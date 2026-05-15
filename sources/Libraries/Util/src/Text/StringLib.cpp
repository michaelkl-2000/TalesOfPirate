//------------------------------------------------------------------------
//	2005.3.25	Arcol	create this file
//	2005.4.7	Arcol	add function : bool GetNameFormString(const string &str,string &name)
//						add function : string CutFaceText(string &text,size_t cutLimitlen)
//	2005.4.19	Arcol	add function : void ChangeParseSymbol(string &text,int nMaxCount)
//	2005.4.28	Arcol	remove all filter system function and create a filter manager class in the common lib
//	2026-05		moved Common/Core/StringLib → Util/Text/StringLib, wrapped in Corsairs::Util,
//				removed mbstring.h (UTF-8 aware), fixed static-buffer UB in StringSplitNum.
//	2026-05		removed all char* from public API: input → std::string_view,
//				output → std::string by value. Callers больше не управляют сырыми буферами.
//------------------------------------------------------------------------

#include "StringLib.h"
#include "EncodingUtil.h"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <format>
#include <string>
#include <string_view>

namespace Corsairs::Util {

std::string StringLimit(std::string_view str, std::size_t len) {
	if (len <= 3 || len >= str.length()) {
		return std::string(str);
	}

	std::string result(str.data(), len);
	result[len - 2] = result[len - 1] = '.';

	// Если позиция len-3 — continuation-байт UTF-8, граница троеточия попала
	// внутрь codepoint: заменяем continuation-байт точкой, чтобы итог не оставил
	// битый codepoint.
	if (!IsUtf8StartByte(static_cast<unsigned char>(result[len - 3]))) {
		result[len - 3] = '.';
	}
	return result;
}

bool GetNameFormString(std::string_view str, std::string& name) {
	std::size_t n = str.find(':');
	if (n > 24) {
		return false;
	}
	name.assign(str.substr(0, n));
	while (n && name[--n] == 0x20) {
		name.erase(n);
	}
	if (name.starts_with("<From ") && name.length() > 7) {
		name = name.substr(6, name.length() - 7);
	}
	else if (name.starts_with("<To ") && name.length() > 5) {
		name = name.substr(4, name.length() - 5);
	}
	return name.length() <= 16;
}

void ChangeParseSymbol(std::string& text, int nMaxCount) {
	for (int i = 0; i < nMaxCount; i++) {
		const std::string src = std::format("#{:02d}", i);
		const std::string rpl = std::format("</{:02d}>", i);
		auto nPos = text.find(src);
		while (nPos != std::string::npos) {
			text.replace(nPos, src.length(), rpl);
			nPos = text.find(src, nPos + rpl.length());
		}
	}
}

std::string StringNewLineChs(std::string_view input, unsigned int nWidth) {
	if (nWidth == 0 || input.empty()) {
		return std::string(input);
	}

	std::string result;
	result.reserve(input.size() + input.size() / nWidth);

	std::size_t pos = 0;
	while (pos < input.size()) {
		const std::size_t chunk = std::min<std::size_t>(nWidth, input.size() - pos);
		result.append(input.substr(pos, chunk));
		pos += chunk;

		// Если граница ширины попала внутрь UTF-8 codepoint (continuation-байт на
		// позиции pos) — добираем continuation-байт следом. Сохраняет старую
		// семантику MBCS-aware копирования без mbstring.h.
		if (pos < input.size()
			&& !IsUtf8StartByte(static_cast<unsigned char>(input[pos]))) {
			result.push_back(input[pos]);
			++pos;
		}
		if (pos < input.size()) {
			result.push_back('\n');
		}
	}
	return result;
}

std::string StringNewLineEng(std::string_view input, unsigned int nWidth) {
	if (nWidth == 0) {
		return std::string{};
	}

	std::string result;
	result.reserve(input.size() + input.size() / nWidth);

	std::size_t spacePos = std::string::npos;
	unsigned int linePos = 0;

	for (std::size_t i = 0; i < input.size(); ++i) {
		const char ch = input[i];
		if (ch == ' ') {
			spacePos = result.size();
		}

		if (linePos >= nWidth) {
			// Откат к последнему пробелу в текущей строке (если был); чтобы не
			// разрывать слова посередине.
			if (spacePos != std::string::npos && spacePos > result.size() - linePos) {
				const std::size_t cut = result.size() - spacePos;
				i      -= cut;
				result.resize(spacePos);
				spacePos = std::string::npos;
			}
			result.push_back('\n');
			linePos = 0;
		}
		else {
			result.push_back(ch);
			++linePos;
		}
	}

	return result;
}

namespace {

// Общая логика разделения числа на группы разрядов. Возвращает std::string по
// значению — RVO/move делают это дешёвым. Старый static char[256] давал гонку
// между потоками сервера; thread_local-промежуточная версия требовала помнить
// про время жизни возвращаемого указателя.
std::string StringSplitNumImpl(std::string_view input, int nCount, char cSplit) {
	const int nLen  = static_cast<int>(input.length());
	const int nLoop = (nLen - 1) / nCount;
	if (nLoop <= 0) {
		return std::string(input);
	}

	std::string result;
	result.resize(nLen + nLoop);
	for (int i = 0; i < nLoop; i++) {
		const int nStart  = nLen - nCount * (i + 1);
		const int nTarget = nStart + nLoop - i;
		std::memcpy(&result[nTarget], input.data() + nStart, nCount);
		result[nTarget - 1] = cSplit;
	}
	// Левый фрагмент (до первого разделителя) — копируем как есть.
	const int firstChunk = nLen - nCount * nLoop;
	std::memcpy(result.data(), input.data(), firstChunk);

	return result;
}

} // namespace

std::string StringSplitNum(std::string_view bigIntStr, int nCount, char cSplit) {
	return StringSplitNumImpl(bigIntStr, nCount, cSplit);
}

std::string StringSplitNum(long nNumber, int nCount, char cSplit) {
	char tmp[32];
	const auto [ptr, ec] = std::to_chars(tmp, tmp + sizeof(tmp), nNumber);
	return StringSplitNumImpl(std::string_view(tmp, ptr - tmp), nCount, cSplit);
}

std::vector<std::string> SplitString(std::string_view str, char delimiter) {
	std::vector<std::string> result;
	std::size_t start = 0;
	while (start <= str.size()) {
		auto end = str.find(delimiter, start);
		if (end == std::string_view::npos) {
			end = str.size();
		}

		auto token = str.substr(start, end - start);
		while (!token.empty() && (token.front() == ' ' || token.front() == '\t')) {
			token.remove_prefix(1);
		}
		while (!token.empty() && (token.back() == ' ' || token.back() == '\t')) {
			token.remove_suffix(1);
		}

		if (!token.empty()) {
			result.emplace_back(token);
		}

		start = end + 1;
	}
	return result;
}

std::vector<int> SplitStringInt(std::string_view str, char delimiter) {
	auto tokens = SplitString(str, delimiter);
	std::vector<int> result;
	result.reserve(tokens.size());
	for (const auto& t : tokens) {
		result.push_back(std::stoi(t));
	}
	return result;
}

} // namespace Corsairs::Util
