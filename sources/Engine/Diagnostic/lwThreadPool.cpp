//
#include "stdafx.h"
#include "lwThreadPool.h"

#include <chrono>

namespace Corsairs::Engine::Render {
	LW_STD_IMPLEMENTATION(lwThreadPool)

	void lwThreadPool::_ThreadProc(std::size_t worker_idx) {
		_thread_states[worker_idx].store(THREADTASKSTATE_WAITING, std::memory_order_release);

		while (true) {
			lwThreadTaskInfo task_info{nullptr, nullptr};
			bool have_task = false;

			{
				std::unique_lock lock(_mu);
				_cv.wait(lock, [&] {
					if (_exit_flag.load(std::memory_order_acquire)) {
						return true;
					}
					if (_paused.load(std::memory_order_acquire)) {
						return false;
					}
					return !_queue_task.empty();
				});

				if (_exit_flag.load(std::memory_order_acquire)) {
					break;
				}

				if (!_queue_task.empty()) {
					task_info = _queue_task.front();
					_queue_task.pop_front();
					have_task = true;
				}
			}

			if (have_task) {
				_thread_states[worker_idx].store(THREADTASKSTATE_RUNNING, std::memory_order_release);
				(*task_info.proc)(task_info.param);
				_thread_states[worker_idx].store(THREADTASKSTATE_WAITING, std::memory_order_release);
			}
		}

		_thread_states[worker_idx].store(THREADTASKSTATE_INVALID, std::memory_order_release);
	}

	lwThreadPool::lwThreadPool() = default;

	lwThreadPool::~lwThreadPool() {
		Destroy();
	}

	LW_RESULT lwThreadPool::Create(DWORD thread_seq_size, DWORD task_seq_size, DWORD suspend_flag) {
		if (!_threads.empty()) {
			return LW_RET_FAILED;
		}
		if (thread_seq_size == 0 || task_seq_size == 0) {
			return LW_RET_FAILED;
		}

		_task_seq_size = task_seq_size;
		_exit_flag.store(false, std::memory_order_release);
		_paused.store(suspend_flag != 0, std::memory_order_release);

		// std::atomic нельзя помещать в vector через resize (он требует move/copy);
		// конструируем vector с правильным размером по конструктору, потом инициализируем.
		_thread_states = std::vector<std::atomic<int>>(thread_seq_size);
		for (auto& s : _thread_states) {
			s.store(THREADTASKSTATE_INVALID, std::memory_order_relaxed);
		}

		_threads.reserve(thread_seq_size);
		try {
			for (std::size_t i = 0; i < thread_seq_size; ++i) {
				_threads.emplace_back(&lwThreadPool::_ThreadProc, this, i);
			}
		}
		catch (const std::system_error&) {
			ToLogService("errors", LogLevel::Error,
						 "[{}] std::thread launch failed: thread_seq_size={}, task_seq_size={}, suspend_flag={}",
						 __FUNCTION__, thread_seq_size, task_seq_size, suspend_flag);
			Destroy();
			return LW_RET_FAILED;
		}

		_priority = THREAD_PRIORITY_NORMAL;
		return LW_RET_OK;
	}

	LW_RESULT lwThreadPool::Destroy() {
		if (!_threads.empty()) {
			{
				std::lock_guard lock(_mu);
				_exit_flag.store(true, std::memory_order_release);
				_paused.store(false, std::memory_order_release);
			}
			_cv.notify_all();

			for (auto& t : _threads) {
				if (t.joinable()) {
					t.join();
				}
			}
			_threads.clear();
		}

		_thread_states.clear();
		_queue_task.clear();
		_task_seq_size = 0;
		_priority = 0;
		_exit_flag.store(false, std::memory_order_release);
		_paused.store(false, std::memory_order_release);

		return LW_RET_OK;
	}

	LW_RESULT lwThreadPool::RegisterTask(lwThreadProc proc, void* param) {
		if (proc == nullptr) {
			return LW_RET_FAILED;
		}

		{
			std::lock_guard lock(_mu);
			if (_queue_task.size() >= _task_seq_size) {
				return LW_RET_FAILED;
			}
			_queue_task.push_back(lwThreadTaskInfo{proc, param});
		}
		_cv.notify_one();

		return LW_RET_OK;
	}

	LW_RESULT lwThreadPool::RemoveTask(lwThreadProc proc, void* param) {
		std::lock_guard lock(_mu);
		auto it = std::find_if(_queue_task.begin(), _queue_task.end(),
							   [&](const lwThreadTaskInfo& t) {
								   return t.proc == proc && t.param == param;
							   });
		if (it == _queue_task.end()) {
			return LW_RET_FAILED;
		}
		_queue_task.erase(it);
		return LW_RET_OK;
	}

	LW_RESULT lwThreadPool::FindTask(lwThreadProc proc, void* param) {
		std::lock_guard lock(_mu);
		auto it = std::find_if(_queue_task.begin(), _queue_task.end(),
							   [&](const lwThreadTaskInfo& t) {
								   return t.proc == proc && t.param == param;
							   });
		return (it != _queue_task.end()) ? LW_RET_OK : LW_RET_FAILED;
	}

	LW_RESULT lwThreadPool::SuspendThread() {
		// Legacy stub — оригинал возвращал -1L. Сохраняем поведение для
		// совместимости с lwIThreadPool. Реальный suspend делается через
		// SetPoolEvent(TRUE).
		return -1L;
	}

	LW_RESULT lwThreadPool::ResumeThread() {
		return -1L;
	}

	LW_RESULT lwThreadPool::SetPriority(int priority) {
		if (_priority == priority) {
			return LW_RET_OK;
		}

		// std::thread не имеет кросс-платформенного priority API.
		// На Windows можно через native_handle() + SetThreadPriority.
		for (auto& t : _threads) {
			if (t.joinable()) {
				if (::SetThreadPriority(static_cast<HANDLE>(t.native_handle()), priority) == FALSE) {
					return LW_RET_FAILED;
				}
			}
		}
		_priority = priority;
		return LW_RET_OK;
	}

	LW_RESULT lwThreadPool::SetPoolEvent(BOOL lock_flag) {
		const bool was_paused = _paused.exchange(lock_flag != FALSE, std::memory_order_acq_rel);
		if (was_paused && !lock_flag) {
			// Разбудить ждущих воркеров — может быть несработавший wait.
			_cv.notify_all();
		}
		return LW_RET_OK;
	}

	DWORD lwThreadPool::GetCurrentRunningTaskNum() const {
		DWORD n = 0;
		for (const auto& s : _thread_states) {
			if (s.load(std::memory_order_acquire) == THREADTASKSTATE_RUNNING) {
				n++;
			}
		}
		return n;
	}

	DWORD lwThreadPool::GetCurrentIdleThreadNum() const {
		return static_cast<DWORD>(_thread_states.size()) - GetCurrentRunningTaskNum();
	}

} // namespace Corsairs::Engine::Render
