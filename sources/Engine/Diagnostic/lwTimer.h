//
#pragma once

#include <atomic>
#include <cstdint>
#include <thread>

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

namespace Corsairs::Engine::Render {
	struct lwTimerInfo {
		lwTimerProc proc;
		std::uint32_t interval;
		std::uint32_t last_time;
		std::uint32_t pause;
		std::uint32_t hit_type;
		std::uint32_t hit_add_cnt;
	};

	class lwTimer : public lwITimer {
		LW_STD_DECLARATION()

		enum { TIMER_DEFAULT_SIZE = 4 };

		static std::int64_t __tick_freq;

	private:
		lwTimerInfo* _timer_seq;
		std::uint32_t _timer_seq_size;
		std::uint32_t _timer_tickcount;

	private:
		inline std::int64_t _QueryCounter() {
			LARGE_INTEGER t;
			QueryPerformanceCounter(&t);
			return t.QuadPart;
		}

		inline std::uint32_t _QueryTick() {
			return static_cast<std::uint32_t>(_QueryCounter() * 1000 / __tick_freq);
		}

	public:
		lwTimer();
		~lwTimer();
		LW_RESULT OnTimer();
		LW_RESULT SetTimer(DWORD id, lwTimerProc proc, DWORD interval, DWORD hit_type, DWORD hit_add_cnt);
		LW_RESULT SetTimerInterval(DWORD id, DWORD interval);

		DWORD GetLastInvokeTime(DWORD id) {
			return _timer_seq[id].last_time;
		}

		LW_RESULT Pause(DWORD id, DWORD flag); // id == LW_INVALID_INDEX : all
		LW_RESULT ResetTimer(DWORD id); // id == LW_INVALID_INDEX : all
		LW_RESULT ClearTimer(DWORD id); // id == LW_INVALID_INDEX : all
		LW_RESULT ReallocTimerSeq(DWORD size);

		// NOTE: возвращает DWORD по сигнатуре lwITimer (virtual override).
		// Внутри хранится как std::uint32_t — типы бит-эквивалентны на Win64.
		DWORD GetTimerTickCount() {
			return _timer_tickcount;
		}

		DWORD GetTickCount();
	};

	struct lwTimerThreadInfo {
		enum {
			STATE_INVALID = 0,
			STATE_RUN = 1,
			STATE_EXIT = 2,
		};

		std::uint32_t last_time = 0;
		std::uint32_t interval = 0; // milliseconds
		lwTimerProc proc = nullptr;
		std::uint32_t pause = 0;
		std::thread thread;
		std::atomic<int> thread_state{STATE_INVALID};

		static void __thread_proc(lwTimerThreadInfo* self);
		LW_RESULT Init(lwTimerProc p, std::uint32_t i);
		LW_RESULT Term(std::uint32_t delay);
	};

	class lwTimerThread : public lwITimerThread {
		LW_STD_DECLARATION();

	public:
		static std::int64_t __tick_freq;

		static inline void _QueryFreq() {
			if (__tick_freq == 0) {
				LARGE_INTEGER freq;
				QueryPerformanceFrequency(&freq);
				__tick_freq = freq.QuadPart;
			}
		}

	private:
		lwTimerThreadInfo** _timer_seq;
		std::uint32_t _timer_seq_size;

	private:

	public:
		lwTimerThread();
		~lwTimerThread();

		LW_RESULT AllocateTimerSeq(DWORD size);
		LW_RESULT SetTimer(DWORD id, lwTimerProc proc, DWORD interval);
		LW_RESULT SetTimerInterval(DWORD id, DWORD interval);
		LW_RESULT Pause(DWORD id, DWORD flag); // id == LW_INVALID_INDEX : all
		LW_RESULT ClearTimer(DWORD id, DWORD delay); // id == LW_INVALID_INDEX : all
	};

	__declspec(selectany) std::int64_t lwTimerThread::__tick_freq = 0;

	class lwTimerPeriod : public lwITimerPeriod {
		LW_STD_DECLARATION();

	private:
		static int _period_ref;
		static TIMECAPS _time_caps;

	public:
		static void BeginPeriod() {
			if (_period_ref == 0) {
				timeGetDevCaps(&_time_caps, sizeof(_time_caps));
				timeBeginPeriod(_time_caps.wPeriodMax);
			}
			_period_ref++;
		}

		static void EndPeriod() {
			if (_period_ref > 0) {
				if (--_period_ref == 0) {
					timeEndPeriod(_time_caps.wPeriodMax);
				}
			}
		}

		static int GetPeriodRef() {
			return _period_ref;
		}

	private:
		UINT _timer_id;

	public:
		lwTimerPeriod()
			: _timer_id(0) {
			BeginPeriod();
		}

		~lwTimerPeriod() {
			KillEvent();
			EndPeriod();
		}

		LW_RESULT SetEvent(UINT delay, UINT resolution, LPTIMECALLBACK proc, DWORD_PTR param, UINT event) {
			_timer_id = ::timeSetEvent(delay, resolution, proc, param, event);
			return (_timer_id != NULL) ? LW_RET_OK : LW_RET_FAILED;
		}

		LW_RESULT KillEvent() {
			::timeKillEvent(_timer_id);
			_timer_id = 0;
			return LW_RET_OK;
		}

		UINT GetTimerID() {
			return _timer_id;
		}
	};

	__declspec(selectany) int lwTimerPeriod::_period_ref = 0;
	__declspec(selectany) TIMECAPS lwTimerPeriod::_time_caps;

} // namespace Corsairs::Engine::Render
