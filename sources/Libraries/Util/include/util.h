#pragma once

// Макросы освобождения остаются на верхнем уровне: текстовая подстановка
// не подчиняется namespace-у. Все три макроса работают и для голых ptr,
// и для COM-интерфейсов с Release().
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)        do { if ((p) != nullptr) { delete   (p); (p) = nullptr; } } while (0)
#define SAFE_DELETE_ARRAY(p)  do { if ((p) != nullptr) { delete[] (p); (p) = nullptr; } } while (0)
#define SAFE_RELEASE(p)       do { if ((p) != nullptr) { (p)->Release(); (p) = nullptr; } } while (0)
#endif

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

// Подключаем соседний algo.h через "" — search-list начинается с каталога
// этого файла, что обходит case-insensitive коллизию с Client/Net/Transport/Algo.h
// при включении из Client TU. Так все Corsairs::Util-функции из algo (Md5/Base64)
// доступны автоматически через util.h.
#include "algo.h"

namespace Corsairs::Util {

// Парсинг строки в массив std::string по одному или двум разделителям с
// схлопыванием идущих подряд одинаковых разделителей. Возвращает число
// заполненных слотов (не более maxTokens).
//
// Семантика legacy: если заданы оба разделителя и оба встречаются в строке,
// строка сначала бьётся по primarySep, затем фрагменты склеиваются через
// secondarySep и парсятся ещё раз.
int ResolveTextLine(std::string_view text,
                    std::string* tokens,
                    int maxTokens,
                    char primarySep = ' ',
                    char secondarySep = '\t');

// Безопасные обёртки над std::from_chars: пустая строка → 0, без бросков.
float        Str2Float(std::string_view str);
std::int32_t Str2Int(std::string_view str);

// Замер интервала в миллисекундах поверх std::chrono::steady_clock.
// Для нового кода предпочитайте `steady_clock::now()` и `duration_cast` напрямую.
class MPTimer {
public:
    MPTimer();

    void Begin();
    std::uint32_t End();
    std::uint32_t GetTimeCount() const;

private:
    std::chrono::steady_clock::time_point _start;
    std::uint32_t _ms{0};
};

} // namespace Corsairs::Util
