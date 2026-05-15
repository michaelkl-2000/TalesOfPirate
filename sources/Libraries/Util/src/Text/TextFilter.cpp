//------------------------------------------------------------------------
//	2005.4.28	Arcol	created CTextFilter in Common/Core/CommFunc.cpp
//	2026-05		moved Common/Core/CommFunc → Util/Text/TextFilter, wrapped in
//				Corsairs::Util, использует Util::IsUtf8StartByte вместо локального дубля.
//------------------------------------------------------------------------

#include "TextFilter.h"
#include "EncodingUtil.h"

#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Util {

std::uint8_t        CTextFilter::m_NowSign[eTableMax][8];
std::vector<std::string> CTextFilter::m_FilterTable[eTableMax];
static CTextFilter  g_textFilterBin;

namespace {

inline std::size_t TableIndex(CTextFilter::eFilterTable eTable)
{
	return static_cast<std::size_t>(eTable);
}

} // namespace

CTextFilter::CTextFilter()
{
	// m_NowSign[][] — статический массив, zero-init гарантирован стандартом до
	// dynamic init этого конструктора. Явное обнуление не требуется.
}

bool CTextFilter::Add(const eFilterTable eTable, std::string_view filterText)
{
	if (filterText.empty()) return false;

	const auto idx = TableIndex(eTable);
	m_FilterTable[idx].emplace_back(filterText);

	for (std::size_t i = 0; i < filterText.size(); i++) {
		std::uint8_t j = static_cast<std::uint8_t>(filterText[i]) / 32;
		int          n = (static_cast<int>(i) + j) % 8;
		m_NowSign[idx][n] += static_cast<std::uint8_t>(j + i);
	}
	return true;
}

bool CTextFilter::IsLegalText(const eFilterTable eTable, std::string_view text)
{
	for (const auto& pattern : m_FilterTable[TableIndex(eTable)]) {
		if (!CheckLegalText(text, pattern)) {
			return false;
		}
	}
	return true;
}

bool CTextFilter::Filter(const eFilterTable eTable, std::string& text)
{
	bool ret = false;
	for (const auto& pattern : m_FilterTable[TableIndex(eTable)]) {
		if (ReplaceText(text, pattern)) {
			ret = true;
		}
	}
	return ret;
}

bool CTextFilter::ReplaceText(std::string& text, std::string_view filterText)
{
	if (filterText.empty()) return false;

	bool ret = false;
	const bool leadStart = IsUtf8StartByte(static_cast<unsigned char>(filterText[0]));
	std::size_t pos = text.find(filterText);

	while (pos != std::string::npos) {
		const bool textStart = pos < text.size()
			&& IsUtf8StartByte(static_cast<unsigned char>(text[pos]));
		if (leadStart == textStart) {
			text.replace(pos, filterText.size(), filterText.size(), '*');
			ret = true;
			pos = text.find(filterText, pos + filterText.size());
		}
		else {
			pos = text.find(filterText, pos + 1);
		}
	}
	return ret;
}

bool CTextFilter::CheckLegalText(std::string_view text, std::string_view illegalText)
{
	if (illegalText.empty()) return true;

	const bool leadStart = IsUtf8StartByte(static_cast<unsigned char>(illegalText[0]));
	std::size_t pos = text.find(illegalText);

	while (pos != std::string::npos) {
		const bool textStart = pos < text.size()
			&& IsUtf8StartByte(static_cast<unsigned char>(text[pos]));
		if (leadStart == textStart) {
			return false;
		}
		pos = text.find(illegalText, pos + 1);
	}
	return true;
}

bool CTextFilter::LoadFile(std::string_view fileName, const eFilterTable eTable)
{
	if (fileName.empty()) return false;

	std::ifstream filterTxt(std::string(fileName), std::ios::in);
	if (!filterTxt.is_open()) return false;

	const auto idx = TableIndex(eTable);
	std::string line;
	while (std::getline(filterTxt, line)) {
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		if (!line.empty()) {
			m_FilterTable[idx].emplace_back(std::move(line));
		}
	}
	return true;
}

const std::uint8_t* CTextFilter::GetNowSign(const eFilterTable eTable)
{
	return m_NowSign[TableIndex(eTable)];
}

} // namespace Corsairs::Util
