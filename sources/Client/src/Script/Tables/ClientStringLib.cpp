// Клиентские функции для работы со строками, использующие LanguageRecordStore.
// Перенесены из Common/StringLib.cpp — серверу эти функции не нужны.

namespace Corsairs::Common::Localization {}
using namespace Corsairs::Common::Localization;
#include "StringLib.h"
#include "EncodingUtil.h"

using namespace std;

namespace Corsairs::Util {

string CutFaceText(string& text, size_t cutLimitlen) {
	string retStr = text;
	if (cutLimitlen <= 0 || text.length() <= cutLimitlen) {
		text.clear();
		return retStr;
	}

	if (0 == stricmp(GetLanguageString(0).c_str(), "English")) {
		string temp = text.substr(0, cutLimitlen);
		size_t nPos = temp.find_last_of(" ");
		if (nPos != string::npos && nPos > 8) {
			retStr = text.substr(0, nPos);
			cutLimitlen = nPos;
		}
		else {
			retStr = text.substr(0, cutLimitlen);
		}
	}
	else {
		retStr = text.substr(0, cutLimitlen);
	}

	// UTF-8: если позиция разреза попала внутрь multi-byte codepoint,
	// отступаем назад до начала ближайшего codepoint (starter-байт).
	if (cutLimitlen < text.size()
		&& !Corsairs::Util::IsUtf8StartByte(static_cast<unsigned char>(text[cutLimitlen]))) {
		size_t safe = cutLimitlen;
		while (safe > 0
			&& !Corsairs::Util::IsUtf8StartByte(static_cast<unsigned char>(text[safe]))) {
			--safe;
		}
		retStr = text.substr(0, safe);
	}
	if ((*--retStr.end()) == '#') {
		retStr = retStr.substr(0, retStr.length() - 1);
	}
	else if ((*(retStr.end() - 2)) == '#') {
		retStr = retStr.substr(0, retStr.length() - 2);
	}
	text = text.substr(retStr.length(), text.length() - retStr.length());
	return retStr;
}

std::string StringNewLine(std::string_view input, unsigned int nWidth) {
	if (0 == _stricmp(GetLanguageString(0).c_str(), "english")) {
		return StringNewLineEng(input, nWidth);
	}
	return StringNewLineChs(input, nWidth);
}

} // namespace Corsairs::Util
