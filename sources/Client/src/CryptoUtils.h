#pragma once

#include <string>
#include <string_view>

// BLAKE2s hash → hex string (used for password hashing)
std::string HashPassword(const std::string& password);

// DPAPI per-user шифрование строки → hex-encoded.
// Пустая plain → пустой результат. Ошибка DPAPI → пустая строка.
// Расшифровка возможна только тем же Windows-юзером на той же машине.
std::string ProtectStringDpapi(std::string_view plain);

// Обратная операция к ProtectStringDpapi. Невалидный hex / невалидный
// blob / другой пользователь → пустая строка (звонящий трактует как
// «сохранённого пароля нет»).
std::string UnprotectStringDpapi(std::string_view hex);
