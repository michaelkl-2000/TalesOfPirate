//
#include "stdafx.h"


#include "CoordinateSys.h"

namespace Corsairs::Engine::Render {
	LW_STD_IMPLEMENTATION(CoordinateSys)

	CoordinateSys::CoordinateSys() {
		_active_id = 0;

		for (DWORD i = 0; i < COORD_SEQ_SIZE; i++) {
			lwMatrix44Identity(&_coord_mat_seq[i]);
		}
	}


	LW_RESULT CoordinateSys::SetActiveCoordSys(DWORD id) {
		if (id >= COORD_SEQ_SIZE)
			return LW_RET_FAILED;

		_active_id = id;

		return LW_RET_OK;
	}

	LW_RESULT CoordinateSys::SetCoordSysMatrix(DWORD id, const lwMatrix44* mat) {
		if (id >= COORD_SEQ_SIZE || mat == 0)
			return LW_RET_FAILED;

		_coord_mat_seq[id] = *mat;

		return LW_RET_OK;
	}

	LW_RESULT CoordinateSys::GetCoordSysMatrix(DWORD id, lwMatrix44* mat) {
		if (id >= COORD_SEQ_SIZE || mat == 0)
			return LW_RET_FAILED;

		*mat = _coord_mat_seq[id];

		return LW_RET_OK;
	}

	LW_RESULT CoordinateSys::GetCurCoordSysMatrix(lwMatrix44* mat) {
		*mat = _coord_mat_seq[_active_id];
		return LW_RET_OK;
	}

	LW_RESULT CoordinateSys::GetActiveCoordSys(DWORD* id) {
		*id = _active_id;
		return LW_RET_OK;
	}

} // namespace Corsairs::Engine::Render
