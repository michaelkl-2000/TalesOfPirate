//
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

namespace Corsairs::Engine::Render {
	enum lwThreadTaskStateType {
		THREADTASKSTATE_INVALID = 0,
		THREADTASKSTATE_WAITING = 1,
		THREADTASKSTATE_RUNNING = 2,
	};

	struct lwThreadTaskInfo {
		lwThreadProc proc;
		void* param;
	};

	class lwThreadPool : public lwIThreadPool {
		LW_STD_DECLARATION();

	private:
		std::vector<std::thread> _threads;
		std::vector<std::atomic<int>> _thread_states; // THREADTASKSTATE_*
		std::deque<lwThreadTaskInfo> _queue_task;
		std::uint32_t _task_seq_size = 0;
		int _priority = 0;

		std::mutex _mu;                  // защищает _queue_task + paused-flag
		std::condition_variable _cv;     // сигнал «есть задача / выход»
		std::atomic<bool> _exit_flag{false};
		std::atomic<bool> _paused{false}; // SetPoolEvent: lock_flag = paused

		void _ThreadProc(std::size_t worker_idx);

	public:
		lwThreadPool();
		~lwThreadPool();

		LW_RESULT Create(DWORD thread_seq_size, DWORD task_seq_size, DWORD suspend_flag);
		LW_RESULT Destroy();
		LW_RESULT RegisterTask(lwThreadProc proc, void* param);
		LW_RESULT RemoveTask(lwThreadProc proc, void* param);
		LW_RESULT FindTask(lwThreadProc proc, void* param);
		LW_RESULT SuspendThread();
		LW_RESULT ResumeThread();
		LW_RESULT SetPriority(int priority);
		LW_RESULT SetPoolEvent(BOOL lock_flag);

		int GetPriority() const {
			return _priority;
		}

		DWORD GetCurrentWaitingTaskNum() const {
			return static_cast<DWORD>(_queue_task.size());
		}

		DWORD GetCurrentRunningTaskNum() const;
		DWORD GetCurrentIdleThreadNum() const;

		DWORD GetThreadId(DWORD id) {
			// std::thread::id не маппится в DWORD напрямую; для legacy-вызовов
			// возвращаем 0. Если кому-то нужен native id — добавить отдельный
			// helper через _threads[id].native_handle().
			return 0;
		}
	};

} // namespace Corsairs::Engine::Render
