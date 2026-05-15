#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace Corsairs::Util {

// Усекает строку до `len` байт с многоточием в хвосте. UTF-8 aware: если граница
// усечения попадает внутрь multi-byte codepoint — заменяет continuation-байт
// точкой, чтобы итог не оставил битый codepoint. Возвращает копию.
std::string StringLimit(std::string_view str, std::size_t len);

// Извлекает имя отправителя из чат-строки формата "Name: text" (поддерживает
// префиксы "<From >" и "<To >"). Имя возвращается через out-параметр. Result —
// валидно ли имя (длина <=16, найдено двоеточие до позиции 24).
bool GetNameFormString(std::string_view str, std::string& name);

// Подрезает текст до заданной длины с учётом UTF-8 codepoint границ и языка
// (для English ищет последний пробел в пределах окна). Реализация — в
// Client/Script/Tables/ClientStringLib.cpp (использует LanguageRecordStore).
std::string CutFaceText(std::string& text, std::size_t cutLimitlen);

// Раскрывает короткие маркеры формата #NN в HTML-подобные теги </NN>. Использует
// std::format для безопасной подстановки.
void ChangeParseSymbol(std::string& text, int nMaxCount);

// Разбивает строку на строки фиксированной ширины. Диспетчер: English — слова
// не разрываются (нужен пробел), Chinese/RU — резка по UTF-8 границе. Реализация
// в ClientStringLib.cpp (зависит от GetLanguageString).
std::string StringNewLine(std::string_view input, unsigned int nWidth);

// MBCS-aware вариант (резка по UTF-8 codepoint границе после nWidth байт).
std::string StringNewLineChs(std::string_view input, unsigned int nWidth);

// English-вариант: режет по последнему пробелу в пределах окна nWidth.
std::string StringNewLineEng(std::string_view input, unsigned int nWidth);

// Разбить строку по разделителю. Пробелы и табы вокруг токенов обрезаются.
std::vector<std::string> SplitString(std::string_view str, char delimiter = ',');

// Разбить строку по разделителю в вектор int.
std::vector<int> SplitStringInt(std::string_view str, char delimiter = ',');

// Форматирует число с разделителями групп разрядов (например 1,234,567). Возвращает
// std::string по значению (RVO/move). Раньше отдавал const char* в thread_local
// буфер — приходилось помнить про время жизни.
std::string StringSplitNum(long nNumber, int nCount = 3, char cSplit = ',');
std::string StringSplitNum(std::string_view bigIntStr, int nCount = 3, char cSplit = ',');

// Форматирование языковой строки ({} плейсхолдеры). Замена sprintf для строк из
// GetLanguageString — возвращает std::string по значению, без выходного char-буфера.
template <typename... Args>
std::string FmtLang(std::string_view fmt, Args&&... args) {
	return std::vformat(fmt, std::make_format_args(args...));
}

} // namespace Corsairs::Util
