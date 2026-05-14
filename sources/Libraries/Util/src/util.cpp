#include "util.h"

#include <charconv>
#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace Corsairs::Util {

namespace {

// Split sv по одному разделителю, схлопывая идущие подряд разделители
// в один (поведение legacy Util_ResolveTextLine1 + Util_StringSkip).
int SplitCollapsing(std::string_view sv, std::string* tokens,
                    int maxTokens, char sep) {
    int n = 0;
    std::size_t pos = 0;
    while (n < maxTokens) {
        const std::size_t next = sv.find(sep, pos);
        if (next == std::string_view::npos) {
            tokens[n++] = std::string(sv.substr(pos));
            break;
        }
        tokens[n++] = std::string(sv.substr(pos, next - pos));
        // Пропустить идущие подряд разделители того же типа.
        const std::size_t skipTo = sv.find_first_not_of(sep, next + 1);
        if (skipTo == std::string_view::npos) {
            break;
        }
        pos = skipTo;
    }
    return n;
}

} // namespace

int ResolveTextLine(std::string_view text,
                    std::string* tokens,
                    int maxTokens,
                    char primarySep,
                    char secondarySep) {
    if (tokens == nullptr || maxTokens <= 0 || text.empty()) {
        return 0;
    }

    if ((primarySep == '\0' && secondarySep == '\0') || text.size() == 1) {
        tokens[0] = std::string(text);
        return 1;
    }

    if (primarySep != '\0' && secondarySep != '\0' && primarySep != secondarySep) {
        const bool hasPrimary   = text.find(primarySep)   != std::string_view::npos;
        const bool hasSecondary = text.find(secondarySep) != std::string_view::npos;

        if (!hasPrimary && !hasSecondary) {
            tokens[0] = std::string(text);
            return 1;
        }
        if (!hasPrimary) {
            return SplitCollapsing(text, tokens, maxTokens, secondarySep);
        }
        if (!hasSecondary) {
            return SplitCollapsing(text, tokens, maxTokens, primarySep);
        }

        // Семантика legacy: сначала split по primarySep, потом каждый фрагмент
        // склеить через secondarySep и распарсить ещё раз — эквивалентно сплиту
        // по primarySep, затем по secondarySep внутри каждого фрагмента.
        std::string tmp[1024];
        const int n1 = SplitCollapsing(text, tmp, maxTokens, primarySep);
        std::string joined;
        for (int i = 0; i < n1; ++i) {
            joined += tmp[i];
            joined += secondarySep;
        }
        return SplitCollapsing(std::string_view(joined), tokens, maxTokens, secondarySep);
    }

    const char sep = primarySep != '\0' ? primarySep : secondarySep;
    return SplitCollapsing(text, tokens, maxTokens, sep);
}

float Str2Float(std::string_view str) {
    float result = 0.0f;
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

std::int32_t Str2Int(std::string_view str) {
    std::int32_t result = 0;
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

MPTimer::MPTimer()
    : _start{std::chrono::steady_clock::now()}, _ms{0} {
}

void MPTimer::Begin() {
    _start = std::chrono::steady_clock::now();
}

std::uint32_t MPTimer::End() {
    const auto delta = std::chrono::steady_clock::now() - _start;
    _ms = static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(delta).count());
    return _ms;
}

std::uint32_t MPTimer::GetTimeCount() const {
    return _ms;
}

} // namespace Corsairs::Util
