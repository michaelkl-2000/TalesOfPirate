#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Util {

// Менеджер фильтров текста (имён персонажей и сообщений чата). Хранит набор
// запрещённых подстрок для нескольких таблиц (NAME_TABLE для имён, DIALOG_TABLE
// для чата) и предоставляет проверку/замену. Все методы статические — класс
// работает как namespace со счётчиком таблиц.
class CTextFilter {
public:
	static constexpr std::size_t eTableMax = 5;

	enum class eFilterTable : std::uint8_t {
		NAME_TABLE   = 0,
		DIALOG_TABLE = 1,
		MAX_TABLE    = eTableMax,
	};
	using enum eFilterTable;

	/*
	 * Warning : не использовать MAX_TABLE — это sentinel-значение. Чтобы расширить
	 * enum выше eTableMax(5), увеличить константу eTableMax.
	 */

	CTextFilter();
	~CTextFilter() = default;

	static bool                 Add(eFilterTable eTable, std::string_view filterText);
	static bool                 IsLegalText(eFilterTable eTable, std::string_view text);
	static bool                 Filter(eFilterTable eTable, std::string& text);
	static bool                 LoadFile(std::string_view fileName, eFilterTable eTable = NAME_TABLE);
	static const std::uint8_t*  GetNowSign(eFilterTable eTable);

private:
	static bool  ReplaceText(std::string& text, std::string_view filterText);
	static bool  CheckLegalText(std::string_view text, std::string_view illegalText);

	static std::vector<std::string> m_FilterTable[eTableMax];
	static std::uint8_t             m_NowSign[eTableMax][8];
};

} // namespace Corsairs::Util
