// SlotMap.h
// Slot-map фиксированной ёмкости с refcount и generation-проверкой ABA.
// Универсальная реализация в Corsairs::Util — общий контейнер для клиента,
// движка и сервера; не зависит от MindPower3D-типов.
//
//   - handle = (generation:12 | slot:20), std::uint32_t. Generation растёт
//     при каждом Register, что отлавливает использование протухшего id.
//   - freelist интрузивный, FIFO (через head/tail): generation растёт
//     равномерно по слотам, минимизируя раннее переиспользование.
//   - AddRef учитывает ref_cnt; Unregister уменьшает счётчик и удаляет
//     объект только при достижении нуля.
//   - GetObj/GetPtr/GetRef валидируют handle (generation + refcount).
//   - Итерация только через ForEach(fn). fn получает (handle, T).
//   - Опциональные GetPtrBySlot/UnregisterBySlot — лукап по чистому
//     slot-индексу без проверки generation. Нужны legacy-кодам, где id
//     хранится как обрезанный int16/int32 и ABA не страшен.
//
// InvalidHandle (0xFFFFFFFF) зарезервирован: у живых handle generation
// ∈ [1..0xFFF], slot < Capacity, поэтому InvalidHandle никогда не совпадёт
// с живым.

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

namespace Corsairs::Util {

	template <typename Tp, std::uint32_t Capacity>
	class SlotMap {
		static_assert(Capacity > 0, "SlotMap capacity must be > 0");
		static_assert(Capacity <= (1u << 20), "SlotMap capacity exceeds 20-bit slot index");

		static constexpr std::uint32_t SLOT_BITS = 20;
		static constexpr std::uint32_t SLOT_MASK = (1u << SLOT_BITS) - 1; // 0x000FFFFF
		static constexpr std::uint32_t GEN_MASK = 0x00000FFFu;             // 12 бит
		static constexpr std::uint32_t FREE_MARK = 0xFFFFFFFFu;

		struct Slot {
			Tp Obj{};
			std::uint32_t NextFree = FREE_MARK;
			std::uint16_t Generation = 0;
			std::uint16_t RefCount = 0;
		};

	public:
		// Числовые коды совпадают с LW_RET_OK / LW_RET_OK_1 / ERR_LWISLOTMAP_*
		// из движка, чтобы Engine-колсайты, сравнивающие результат с
		// `LW_RET_OK_1` или `ERR_LWISLOTMAP_POOL_FULL`, продолжали работать
		// без перекомпиляции таблиц констант.
		static constexpr std::int32_t Ok = 0;
		static constexpr std::int32_t OkLast = 1;
		static constexpr std::int32_t ErrPoolFull = -2;
		static constexpr std::int32_t ErrInvalidHandle = -3;
		static constexpr std::uint32_t InvalidHandle = FREE_MARK;
		static constexpr std::uint32_t SlotMaskValue = SLOT_MASK;

		// Совместимость со старым API.
		static constexpr std::uint32_t POOL_SIZE = Capacity;

		SlotMap()
			: _free_head(0)
			  , _free_tail(Capacity - 1)
			  , _obj_num(0) {
			for (std::uint32_t i = 0; i < Capacity; ++i) {
				_slots[i].NextFree = (i + 1 < Capacity) ? (i + 1) : FREE_MARK;
			}
		}

		~SlotMap() = default;

		// Copy/move разрешены, но **дороги**: внутренний массив _slots размера
		// Capacity копируется/перемещается memberwise. Нужно только для
		// legacy-кейса, когда SlotMap встроен в копируемый value-type
		// (например, CDynMapEntryCell внутри CResidentList): копия делается
		// один раз при инстанциировании и обычно по ещё пустому объекту.
		// В hot-path передавай SlotMap по ссылке.
		SlotMap(const SlotMap&) = default;
		SlotMap& operator=(const SlotMap&) = default;
		SlotMap(SlotMap&&) noexcept = default;
		SlotMap& operator=(SlotMap&&) noexcept = default;

		// Register принимает указатель на любой unsigned 32-битный тип
		// (std::uint32_t, DWORD = unsigned long на Windows x64 и т.п.) —
		// размер должен совпадать. Шаблон нужен, чтобы Engine-колсайты с
		// `DWORD handle; Register(&handle, ...)` оставались валидными.
		template <typename HandleT>
		std::int32_t Register(HandleT* ret_handle, Tp obj) {
			static_assert(std::is_integral_v<HandleT> && std::is_unsigned_v<HandleT>,
			              "Handle must be unsigned integral");
			static_assert(sizeof(HandleT) >= sizeof(std::uint32_t),
			              "Handle must fit std::uint32_t");

			if (_obj_num == Capacity) {
				return ErrPoolFull;
			}

			const std::uint32_t slot = _free_head;
			Slot& s = _slots[slot];
			_free_head = s.NextFree;
			if (_free_head == FREE_MARK) {
				_free_tail = FREE_MARK;
			}

			s.Obj = std::move(obj);
			s.NextFree = FREE_MARK;

			std::uint16_t gen = static_cast<std::uint16_t>((s.Generation + 1) & GEN_MASK);
			if (gen == 0) {
				gen = 1;
			}
			s.Generation = gen;
			s.RefCount = 1;
			++_obj_num;

			*ret_handle = static_cast<HandleT>(_Pack(slot, gen));
			return Ok;
		}

		// Возвращает:
		//   OkLast — последний reference, объект удалён из пула (записан в ret_obj, если ret_obj != nullptr).
		//   Ok     — decrement, объект ещё жив.
		//   Err*   — handle битый.
		template <typename HandleT>
		std::int32_t Unregister(Tp* ret_obj, HandleT handle) {
			std::uint32_t slot;
			Slot* s = _Decode(static_cast<std::uint32_t>(handle), slot);
			if (!s) {
				return ErrInvalidHandle;
			}

			--s->RefCount;
			if (s->RefCount == 0) {
				if (ret_obj) {
					*ret_obj = s->Obj;
				}
				s->Obj = Tp{};
				s->NextFree = FREE_MARK;
				if (_free_tail == FREE_MARK) {
					_free_head = slot;
					_free_tail = slot;
				}
				else {
					_slots[_free_tail].NextFree = slot;
					_free_tail = slot;
				}
				--_obj_num;
				return OkLast;
			}
			return Ok;
		}

		template <typename HandleT>
		std::int32_t AddRef(HandleT handle, std::uint32_t ref_cnt = 1) {
			std::uint32_t slot;
			Slot* s = _Decode(static_cast<std::uint32_t>(handle), slot);
			if (!s) {
				return ErrInvalidHandle;
			}
			const std::uint32_t next = static_cast<std::uint32_t>(s->RefCount) + ref_cnt;
			s->RefCount = (next > 0xFFFFu)
			                  ? static_cast<std::uint16_t>(0xFFFFu)
			                  : static_cast<std::uint16_t>(next);
			return Ok;
		}

		template <typename HandleT>
		std::int32_t GetObj(Tp* ret_obj, HandleT handle) {
			std::uint32_t slot;
			Slot* s = _Decode(static_cast<std::uint32_t>(handle), slot);
			if (!s) {
				return ErrInvalidHandle;
			}
			*ret_obj = s->Obj;
			return Ok;
		}

		// Указатель на хранимое значение. nullptr, если handle битый.
		// Указатель валиден до Unregister/Clear или уничтожения SlotMap'а.
		template <typename HandleT>
		Tp* GetPtr(HandleT handle) {
			std::uint32_t slot;
			Slot* s = _Decode(static_cast<std::uint32_t>(handle), slot);
			return s ? &s->Obj : nullptr;
		}

		template <typename HandleT>
		std::uint32_t GetRef(HandleT handle) {
			std::uint32_t slot;
			Slot* s = _Decode(static_cast<std::uint32_t>(handle), slot);
			if (!s) {
				return 0;
			}
			return s->RefCount;
		}

		// --- legacy API без generation-проверки --------------------------------
		// Используется кодом, который хранит только slot-индекс (обрезанный
		// int16/int32 как сетевой id) и обходится без ABA-защиты. Контракт
		// идентичен бывшему CListArray: «слот живой ⇒ объект есть».

		Tp* GetPtrBySlot(std::uint32_t slot) {
			if (slot >= Capacity) {
				return nullptr;
			}
			Slot& s = _slots[slot];
			return s.RefCount > 0 ? &s.Obj : nullptr;
		}

		bool UnregisterBySlot(std::uint32_t slot) {
			if (slot >= Capacity) {
				return false;
			}
			Slot& s = _slots[slot];
			if (s.RefCount == 0) {
				return false;
			}
			const std::uint32_t handle = _Pack(slot, s.Generation);
			return Unregister(static_cast<Tp*>(nullptr), handle) == OkLast;
		}
		// -----------------------------------------------------------------------

		std::uint32_t GetObjNum() const {
			return _obj_num;
		}

		void Clear() {
			for (std::uint32_t i = 0; i < Capacity; ++i) {
				_slots[i].Obj = Tp{};
				_slots[i].Generation = 0;
				_slots[i].RefCount = 0;
				_slots[i].NextFree = (i + 1 < Capacity) ? (i + 1) : FREE_MARK;
			}
			_free_head = 0;
			_free_tail = Capacity - 1;
			_obj_num = 0;
		}

		// Итерация по живым слотам. fn(handle, T). Если fn возвращает bool,
		// значение false прерывает обход (range-break семантика).
		template <typename Fn>
		void ForEach(Fn&& fn) {
			std::uint32_t left = _obj_num;
			for (std::uint32_t i = 0; i < Capacity && left > 0; ++i) {
				Slot& s = _slots[i];
				if (s.RefCount == 0) {
					continue;
				}
				--left;
				const std::uint32_t handle = _Pack(i, s.Generation);
				if constexpr (std::is_same_v<std::invoke_result_t<Fn, std::uint32_t, Tp>, bool>) {
					if (!fn(handle, s.Obj)) {
						return;
					}
				}
				else {
					fn(handle, s.Obj);
				}
			}
		}

	private:
		static constexpr std::uint32_t _Pack(std::uint32_t slot, std::uint32_t gen) {
			return ((gen & GEN_MASK) << SLOT_BITS) | (slot & SLOT_MASK);
		}

		Slot* _Decode(std::uint32_t handle, std::uint32_t& slot) {
			if (handle == InvalidHandle) {
				return nullptr;
			}
			slot = handle & SLOT_MASK;
			if (slot >= Capacity) {
				return nullptr;
			}
			const std::uint32_t gen = (handle >> SLOT_BITS) & GEN_MASK;
			Slot* s = &_slots[slot];
			if (s->RefCount == 0 || s->Generation != gen) {
				return nullptr;
			}
			return s;
		}

		Slot _slots[Capacity]{};
		std::uint32_t _free_head;
		std::uint32_t _free_tail;
		std::uint32_t _obj_num;
	};

} // namespace Corsairs::Util
