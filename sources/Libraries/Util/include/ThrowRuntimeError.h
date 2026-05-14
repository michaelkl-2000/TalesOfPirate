#pragma once

#include <format>
#include <source_location>
#include <stdexcept>
#include <string_view>

// Replacement for the legacy `THROW_EXCP(excpXxx, desc)` macro:
// бросает `std::runtime_error`, к описанию автоматически приклеивается
// `file:line` через `std::source_location`. Никто в проекте не ловил
// конкретные подтипы `excp*` — иерархия схлопнута.

namespace Corsairs::Util {

[[noreturn]] inline void ThrowRuntimeError(
	std::string_view desc,
	const std::source_location& loc = std::source_location::current())
{
	throw std::runtime_error(std::format("{}:{} {}", loc.file_name(), loc.line(), desc));
}

} // namespace Corsairs::Util
