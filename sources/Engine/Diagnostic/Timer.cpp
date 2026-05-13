//
#include "stdafx.h"
#include "Timer.h"

#include <chrono>
#include <thread>

namespace Corsairs::Engine::Render {
	// Timer
	LW_STD_IMPLEMENTATION(Timer)

	LONGLONG Timer::__tick_freq = 0;

	Timer::Timer() {
		_timer_tickcount = 0;
		_timer_seq_size = TIMER_DEFAULT_SIZE;
		_timer_seq = LW_NEW(TimerInfo[_timer_seq_size]);
		memset(_timer_seq, 0, sizeof(TimerInfo) * _timer_seq_size);

		if (__tick_freq == 0) {
			LARGE_INTEGER timefreq;
			QueryPerformanceFrequency(&timefreq);
			__tick_freq = timefreq.QuadPart;
		}
	}

	Timer::~Timer() {
		LW_DELETE_A(_timer_seq);
	}

	LW_RESULT Timer::ReallocTimerSeq(DWORD size) {
		TimerInfo* new_seq = LW_NEW(TimerInfo[size]);
		memcpy(new_seq, _timer_seq, sizeof(TimerInfo) * (size > _timer_seq_size ? _timer_seq_size : size));
		LW_DELETE_A(_timer_seq);
		_timer_seq = new_seq;
		_timer_seq_size = size;

		return LW_RET_OK;
	}

	LW_RESULT Timer::SetTimerInterval(DWORD id, DWORD interval) {
		LW_RESULT ret = LW_RET_FAILED;

		if (id >= _timer_seq_size)
			goto __ret;
		{
			TimerInfo* info = &_timer_seq[id];

			if (info->proc == 0)
				goto __ret;

			info->interval = interval;
		}
		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT Timer::SetTimer(DWORD id, TimerProc proc, DWORD interval, DWORD hit_type, DWORD hit_add_cnt) {
		LW_RESULT ret = LW_RET_FAILED;

		if (proc == 0 || id >= _timer_seq_size)
			goto __ret;

		{
			TimerInfo* info = &_timer_seq[id];

			if (info->proc)
				goto __ret;

			info->proc = proc;
			info->interval = interval;
			info->hit_type = hit_type;
			info->hit_add_cnt = hit_add_cnt;
			info->pause = 0;

			info->last_time = _QueryTick();
		}
		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT Timer::ClearTimer(DWORD id) {
		LW_RESULT ret = LW_RET_FAILED;

		if (id == LW_INVALID_INDEX) {
			memset(_timer_seq, 0, sizeof(TimerInfo) * _timer_seq_size);
			_timer_seq_size = 0;
		}
		else {
			if (id >= _timer_seq_size)
				goto __ret;

			TimerInfo* info = &_timer_seq[id];

			if (info->proc == 0)
				goto __ret;

			info->proc = 0;
			info->interval = 0;
			info->last_time = 0;
			info->pause = 0;
			info->hit_type = 0;
			info->hit_add_cnt = 0;
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT Timer::ResetTimer(DWORD id) {
		LW_RESULT ret = LW_RET_FAILED;

		if (id == LW_INVALID_INDEX) {
			for (DWORD i = 0; i < _timer_seq_size; i++) {
				if (_timer_seq[i].proc) {
					_timer_seq[i].last_time = _QueryTick();
				}
			}
		}
		else {
			if (id >= _timer_seq_size)
				goto __ret;

			TimerInfo* info = &_timer_seq[id];

			if (info->proc == 0)
				goto __ret;

			info->last_time = _QueryTick();
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT Timer::OnTimer() {
		_timer_tickcount = _QueryTick();

		TimerInfo* info;

		for (DWORD i = 0; i < _timer_seq_size; i++) {
			info = &_timer_seq[i];

			if (info->proc == 0 || info->pause == 1)
				continue;

			DWORD t = _timer_tickcount - info->last_time;

			if (t >= info->interval) {
				(*info->proc)(_timer_tickcount);

				if (info->hit_type == TIMER_HIT_FILTER) {
					info->last_time = _timer_tickcount;
				}
				else {
					info->last_time += info->interval;


					for (DWORD i = 0; (_timer_tickcount - info->last_time >= info->interval) && (i < info->hit_add_cnt);
						 i++) {
						(*info->proc)(_timer_tickcount);
						info->last_time += info->interval;
					}
				}
			}
		}

		return LW_RET_OK;
	}

	LW_RESULT Timer::Pause(DWORD id, DWORD flag) {
		LW_RESULT ret = LW_RET_FAILED;

		if (id == LW_INVALID_INDEX) {
			for (DWORD i = 0; i < _timer_seq_size; i++) {
				if (_timer_seq[id].proc) {
					_timer_seq[id].pause = flag;
				}
			}
		}
		else {
			if (id >= _timer_seq_size)
				goto __ret;

			if (_timer_seq[id].proc == 0)
				goto __ret;

			_timer_seq[id].pause = flag;
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	DWORD Timer::GetTickCount() {
		return _QueryTick();
	}

	// TimerThread (std::thread-based)
	void TimerThreadInfo::__thread_proc(TimerThreadInfo* self) {
		while (self->thread_state.load(std::memory_order_acquire) == STATE_RUN) {
			if (!self->pause) {
				LARGE_INTEGER t;
				QueryPerformanceCounter(&t);

				std::uint32_t cur_t = static_cast<std::uint32_t>(t.QuadPart * 1000 / TimerThread::__tick_freq);

				if ((cur_t - self->last_time) >= self->interval) {
					self->last_time = cur_t;
					(*self->proc)(cur_t);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		self->thread_state.store(STATE_INVALID, std::memory_order_release);
	}

	LW_RESULT TimerThreadInfo::Init(TimerProc p, std::uint32_t i) {
		if (thread.joinable()) {
			// Уже запущен — не пересоздаём
			return LW_RET_FAILED;
		}

		proc = p;
		interval = i;
		pause = 0;
		last_time = 0;
		thread_state.store(STATE_RUN, std::memory_order_release);

		try {
			thread = std::thread(&TimerThreadInfo::__thread_proc, this);
		}
		catch (const std::system_error&) {
			thread_state.store(STATE_INVALID, std::memory_order_release);
			return LW_RET_FAILED;
		}

		return LW_RET_OK;
	}

	LW_RESULT TimerThreadInfo::Term(std::uint32_t delay) {
		thread_state.store(STATE_EXIT, std::memory_order_release);

		// Ждём пока __thread_proc выйдет из цикла и установит STATE_INVALID.
		// Лимит __term_lmt итераций сохранён для совместимости — если поток
		// зависнет, используем detach() вместо TerminateThread (последний
		// небезопасен — оставляет ресурсы в неопределённом состоянии).
		const std::uint32_t __term_lmt = 10;
		std::uint32_t term_cntt = 0;
		while (thread_state.load(std::memory_order_acquire) != STATE_INVALID) {
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			if (++term_cntt == __term_lmt) {
				if (thread.joinable()) {
					thread.detach();
				}
				thread_state.store(STATE_INVALID, std::memory_order_release);
				return LW_RET_OK;
			}
		}

		if (thread.joinable()) {
			thread.join();
		}

		return LW_RET_OK;
	}

	// TimerThread
	LW_STD_IMPLEMENTATION(TimerThread)

	TimerThread::TimerThread()
		: _timer_seq(0), _timer_seq_size(0) {
	}

	TimerThread::~TimerThread() {
		ClearTimer(LW_INVALID_INDEX, 1);
	}

	LW_RESULT TimerThread::AllocateTimerSeq(DWORD size) {
		_QueryFreq();

		TimerThreadInfo** new_seq = LW_NEW(TimerThreadInfo*[size]);
		memset(new_seq, 0, sizeof(TimerThreadInfo*) * size);
		memcpy(new_seq, _timer_seq, sizeof(TimerThreadInfo*) * (size > _timer_seq_size ? _timer_seq_size : size));
		LW_DELETE_A(_timer_seq);
		_timer_seq = new_seq;
		_timer_seq_size = size;

		return LW_RET_OK;
	}

	LW_RESULT TimerThread::SetTimer(DWORD id, TimerProc proc, DWORD interval) {
		LW_RESULT ret = LW_RET_FAILED;

		if ((id >= _timer_seq_size) || _timer_seq[id])
			goto __ret;
		{
			TimerThreadInfo* tti = LW_NEW(TimerThreadInfo);

			if (LW_RESULT r = tti->Init(proc, interval); LW_FAILED(r)) {
				ToLogService("errors", LogLevel::Error,
							 "[{}] tti->Init failed: id={}, interval={}, ret={}",
							 __FUNCTION__, id, interval, static_cast<long long>(r));
				goto __ret;
			}

			_timer_seq[id] = tti;
		}
		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT TimerThread::SetTimerInterval(DWORD id, DWORD interval) {
		LW_RESULT ret = LW_RET_FAILED;

		if ((id >= _timer_seq_size) || (_timer_seq[id] == 0))
			goto __ret;

		_timer_seq[id]->interval = interval;

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	LW_RESULT TimerThread::Pause(DWORD id, DWORD flag) {
		for (DWORD i = 0; i < _timer_seq_size; i++) {
			if (_timer_seq[i] == 0)
				continue;

			if (id == LW_INVALID_INDEX || id == i) {
				_timer_seq[i]->pause = flag;
			}
		}

		return LW_RET_OK;
	}

	LW_RESULT TimerThread::ClearTimer(DWORD id, DWORD delay) {
		LW_RESULT ret = LW_RET_FAILED;

		for (DWORD i = 0; i < _timer_seq_size; i++) {
			if (_timer_seq[i] == 0)
				continue;

			if (id == LW_INVALID_INDEX || id == i) {
				if (LW_RESULT r = _timer_seq[i]->Term(delay); LW_FAILED(r)) {
					ToLogService("errors", LogLevel::Error,
								 "[{}] _timer_seq[i]->Term failed: i={}, delay={}, ret={}",
								 __FUNCTION__, i, delay, static_cast<long long>(r));
					goto __ret;
				}

				LW_DELETE(_timer_seq[i]);
				_timer_seq[i] = 0;
			}
		}

		ret = LW_RET_OK;
	__ret:
		return ret;
	}

	// TimerPeriod
	LW_STD_IMPLEMENTATION(TimerPeriod)

} // namespace Corsairs::Engine::Render
