#ifndef MPQuaternion_H
#define MPQuaternion_H

#include "MindPowerAPI.h"
#include "lwDirectX.h"

class MPEffVector3;
class MPRadian;

class MPEffQuaternion {
public:
	D3DXQUATERNION _self;

public:
	MPEffQuaternion() : _self(0, 0, 0, 0) {
	}

	MPEffQuaternion(float w, float x, float y, float z)
		: _self(w, x, y, z) {
	}

	MPEffQuaternion(const D3DXQUATERNION& dxQ) : _self(dxQ) {
	}

	MPEffQuaternion(const MPEffQuaternion& q) : _self(q._self) {
	}

	D3DXQUATERNION& GetDXValue() {
		return _self;
	}

	MPEffVector3 operator*(const MPEffVector3& rkVector) const;


	void FromAngleAxis(const MPRadian& rfAngle, const MPEffVector3& rkAxis);
};
#endif
