//
#pragma once

#include <mutex>

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"

namespace Corsairs::Engine::Render {
	// Тонкая обёртка над std::mutex для совместимости со старым API
	// Create()/Destroy()/Lock()/Unlock(). Внутри — std::mutex.
	//
	// Используется в lwResourceMgr (_lock_sysmemtex). После полной миграции
	// callsite на std::mutex напрямую — класс можно удалить.
	//
	// lwEvent/lwSemaphore (WinAPI HANDLE-обёртки) были удалены при миграции
	// lwThreadPool на std::thread + condition_variable.
	class lwCriticalSection {
	private:
		std::mutex _mutex;
		bool _created = false;

	public:
		lwCriticalSection() = default;

		~lwCriticalSection() {
			Destroy();
		}

		lwCriticalSection(const lwCriticalSection&) = delete;
		lwCriticalSection& operator=(const lwCriticalSection&) = delete;

		LW_RESULT Create() {
			_created = true;
			return LW_RET_OK;
		}

		LW_RESULT Destroy() {
			_created = false;
			return LW_RET_OK;
		}

		void Lock() {
			_mutex.lock();
		}

		void Unlock() {
			_mutex.unlock();
		}

		// Совместимость со старым GetState() — был LW_INVALID_INDEX до Create().
		// Возвращает 0 после Create(), LW_INVALID_INDEX иначе.
		DWORD GetState() const {
			return _created ? 0u : LW_INVALID_INDEX;
		}
	};

} // namespace Corsairs::Engine::Render
