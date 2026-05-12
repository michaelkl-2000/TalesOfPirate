#include "stdafx.h"

#include "MPEffQuaternion.h"

#include "MPEffectMath.h"
#include "MPEffVector3.h"
//-----------------------------------------------------------------------------
void MPEffQuaternion::FromAngleAxis(const MPRadian& rfAngle, const MPEffVector3& rkAxis) {
	// :  

	MPRadian fHalfAngle(rfAngle * 0.5);
	float fSin = MPEffectMath::Sin(fHalfAngle);
	_self.w = MPEffectMath::Cos(fHalfAngle);
	_self.x = fSin * rkAxis._self.x;
	_self.y = fSin * rkAxis._self.y;
	_self.z = fSin * rkAxis._self.z;
}

//-----------------------------------------------------------------------------
MPEffVector3 MPEffQuaternion::operator*(const MPEffVector3& v) const {
	// nVidia SDK 
	MPEffVector3 uv, uuv;
	MPEffVector3 qvec(_self.x, _self.y, _self.z);
	uv = qvec.crossProduct(v);
	uuv = qvec.crossProduct(uv);
	uv *= (2.0f * _self.w);
	uuv *= 2.0f;

	return v + uv + uuv;
}
