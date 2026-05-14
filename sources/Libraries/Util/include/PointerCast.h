#pragma once

#include <cstdint>

namespace Corsairs::Util {

template <typename T>
inline T* ToPointer(std::uintptr_t address) {
    return reinterpret_cast<T*>(address);
}

inline std::uintptr_t ToAddress(const void* pointer) {
    return reinterpret_cast<std::uintptr_t>(pointer);
}

} // namespace Corsairs::Util
