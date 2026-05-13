#include "Stdafx.h"
#include "MPCamera.h"


MPCameraNOLEECH::MPCameraNOLEECH() {
	_fRoll = 0.0f;

	InitPosition(0.0f, 100.0f, 100.0f, 0.0f, 0.0f, 0.0f);
	strCameraInfo = std::format("camera : ({:7.2f} {:7.2f} {:7.2f})__({:7.2f} {:7.2f} {:7.2f})",
								_EyePos.x, _EyePos.y, _EyePos.z,
								_RefPos.x, _RefPos.y, _RefPos.z);
}

MPCameraNOLEECH::~MPCameraNOLEECH() {
}

VOID MPCameraNOLEECH::Move(DWORD dwMoveType) {
	float fStep = 0.2f;
	D3DXVECTOR3 Move(0, 0, 0);

	switch (dwMoveType) {
	case MOVE_LEFT: {
		auto v = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		auto v2 = (_RefPos - _EyePos);
		D3DXVec3Cross(&Move, &v2, &v);
		D3DXVec3Normalize(&Move, &Move);
		Move *= fStep;
		_RefPos += Move;
		_EyePos += Move;
		break;
	}
	case MOVE_RIGHT: {
		auto v = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		auto v2 = (_RefPos - _EyePos);
		D3DXVec3Cross(&Move, &v2, &v);
		D3DXVec3Normalize(&Move, &Move);
		Move *= fStep;
		_RefPos -= Move;
		_EyePos -= Move;
		break;
	}
	case MOVE_FORWARD: {
		auto v = (_RefPos - _EyePos);
		D3DXVec3Normalize(&Move, &v);
		Move *= fStep;
		_RefPos += Move;
		_EyePos += Move;
		break;
	}
	case MOVE_BACKWARD: {
		auto v = (_RefPos - _EyePos);
		D3DXVec3Normalize(&Move, &v);
		Move *= fStep;
		_RefPos -= Move;
		_EyePos -= Move;
		break;
	}
	case MOVE_UP: {
		Move = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		Move *= fStep;
		_RefPos += Move;
		_EyePos += Move;
		break;
	}
	case MOVE_DOWN: {
		Move = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		Move *= fStep;
		_RefPos -= Move;
		_EyePos -= Move;
		break;
	}
	case ROTATE_VEER: {
		break;
	}
	case ROTATE_REVERSE: {
		break;
	}
	case ROLL1: {
		_fRoll += 5.0f;
		break;
	}
	case ROLL2: {
		_fRoll -= 5.0f;
		break;
	}
	}
	strCameraInfo = std::format("camera : ({:7.2f} {:7.2f} {:7.2f})__({:7.2f} {:7.2f} {:7.2f})",
								_EyePos.x, _EyePos.y, _EyePos.z,
								_RefPos.x, _RefPos.y, _RefPos.z);
}

// , Hang
void MPCameraNOLEECH::MoveForward(float fStep, BOOL bHang) {
	D3DXVECTOR3 Move(0, 0, 0);
	if (bHang) {
		auto v = (D3DXVECTOR3(_RefPos.x, _RefPos.y, 0.0f) - D3DXVECTOR3(_EyePos.x, _EyePos.y, 0.0f));
		D3DXVec3Normalize(&Move, &v);
	}
	else {
		auto v = (_RefPos - _EyePos);
		D3DXVec3Normalize(&Move, &v);
	}
	Move *= fStep;
	_RefPos += Move;
	_EyePos += Move;
}

// , Hang
void MPCameraNOLEECH::MoveRight(float fStep, BOOL bHang) {
	D3DXVECTOR3 Move(0, 0, 0);

	auto v = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	auto v2 = (_RefPos - _EyePos);
	D3DXVec3Cross(&Move, &v2, &v);
	D3DXVec3Normalize(&Move, &Move);
	Move *= fStep;
	if (bHang) {
		Move.z = 0;
	}
	_RefPos += Move;
	_EyePos += Move;
}

void MPCameraNOLEECH::Turn(float fStep, D3DXVECTOR3* pFocusVec) {
	D3DXVECTOR3 Move(0, 0, 0);

	if (!pFocusVec) {
		const D3DXVECTOR3 v[] = {
			_RefPos - _EyePos,
			D3DXVECTOR3(0.0f, 0.0f, 1.0f)
		};
		D3DXVec3Cross(&Move, &v[0], &v[1]);
		D3DXVec3Normalize(&Move, &Move);
		Move *= fStep;
		_RefPos += Move;
	}
	else {
		_RefPos = *pFocusVec;
		D3DXVECTOR3 vv = _EyePos - *pFocusVec;
		D3DXVECTOR3 const v[] = {
			D3DXVECTOR3(vv.x, vv.y, 0.0f),
			D3DXVECTOR3(0.0f, 0.0f, 1.0f)
		};
		D3DXVec3Normalize(&vv, &vv);
		D3DXVec3Cross(&Move, &v[0], &v[1]);
		D3DXVec3Normalize(&Move, &Move);
		Move *= fStep;
		_EyePos += Move;
	}
}

void MPCameraNOLEECH::FrameMove(DWORD dwTailTime) {
}

//lemon add@2004.8.4////////////////////////////////////////////////////////////////////////
