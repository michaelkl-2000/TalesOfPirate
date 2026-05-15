// SlotMap.h
// Engine-локальный шим над универсальной реализацией в Corsairs::Util.
// Сама структура данных живёт в sources/Libraries/Util/src/Memory/SlotMap.h —
// здесь только алиас в Corsairs::Engine::Render и legacy-typedef'ы void*-пулов.

#pragma once

#include "Memory/SlotMap.h" // Corsairs::Util::SlotMap

namespace Corsairs::Engine::Render {

	template <typename Tp, std::uint32_t Capacity>
	using SlotMap = ::Corsairs::Util::SlotMap<Tp, Capacity>;

	using SlotMapVoidPtr1024 = SlotMap<void*, 1024>;
	using SlotMapVoidPtr2048 = SlotMap<void*, 2048>;
	using SlotMapVoidPtr4096 = SlotMap<void*, 4096>;
	using SlotMapVoidPtr10240 = SlotMap<void*, 10240>;
	using SlotMapVoidPtr40960 = SlotMap<void*, 40960>;

} // namespace Corsairs::Engine::Render
