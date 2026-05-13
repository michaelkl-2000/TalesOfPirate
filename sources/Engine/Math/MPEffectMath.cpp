#include "stdafx.h"
#include "MPEffectMath.h"

#include <math.h>
#include <limits>
#include "MPEffectAsmMath.h"

//=============================================================================
// MPRadian
//-----------------------------------------------------------------------------
float MPRadian::valueDegrees() const {
	return MPEffectMath::RadiansToDegrees(_fRad);
}

//-------------------------------------------------------------------------
float MPRadian::valueAngleUnits() const {
	return MPEffectMath::RadiansToAngleUnits(_fRad);
}

//=============================================================================
// MPDegree
//-----------------------------------------------------------------------------
float MPDegree::valueRadians() const {
	return MPEffectMath::DegreesToRadians(_fDeg);
}

//-------------------------------------------------------------------------
float MPDegree::valueAngleUnits() const {
	return MPEffectMath::DegreesToAngleUnits(_fDeg);
}

//=============================================================================
// MPEffectMath
//-----------------------------------------------------------------------------
const float MPEffectMath::POS_INFINITY = std::numeric_limits<float>::infinity();
const float MPEffectMath::NEG_INFINITY = -std::numeric_limits<float>::infinity();
const float MPEffectMath::PI = float(4.0 * atan(1.0));
const float MPEffectMath::TWO_PI = float(2.0 * PI);
const float MPEffectMath::HALF_PI = float(0.5 * PI);
const float MPEffectMath::fDeg2Rad = PI / float(180.0);
const float MPEffectMath::fRad2Deg = float(180.0) / PI;

const std::string MPEffectMath::BLANK = std::string("");


int MPEffectMath::_TrigTableSize;
MPEffectMath::AngleUnit MPEffectMath::_sAngleUnit;

float MPEffectMath::_TrigTableFactor;
float* MPEffectMath::_SinTable = 0;
float* MPEffectMath::_TanTable = 0;

//-----------------------------------------------------------------------------
MPEffectMath::MPEffectMath(unsigned int trigTableSize) {
	_sAngleUnit = AU_DEGREE;

	_TrigTableSize = trigTableSize;
	_TrigTableFactor = _TrigTableSize / MPEffectMath::TWO_PI;

	_SinTable = new float[_TrigTableSize];
	_TanTable = new float[_TrigTableSize];

	buildTrigTables();
}

//-----------------------------------------------------------------------------
MPEffectMath::~MPEffectMath() {
	delete[] _SinTable;
	delete[] _TanTable;
}

//-----------------------------------------------------------------------------
void MPEffectMath::buildTrigTables(void) {
	float angle;
	for (int i = 0; i < _TrigTableSize; ++i) {
		angle = MPEffectMath::TWO_PI * i / _TrigTableSize;
		_SinTable[i] = sin(angle);
		_TanTable[i] = tan(angle);
	}
}

//-----------------------------------------------------------------------------
float MPEffectMath::SinTable(float fValue) {
	// Convert range to index values, wrap if required
	int idx;
	if (fValue >= 0) {
		idx = int(fValue * _TrigTableFactor) % _TrigTableSize;
	}
	else {
		idx = _TrigTableSize - (int(-fValue * _TrigTableFactor) % _TrigTableSize) - 1;
	}

	return _SinTable[idx];
}

//-----------------------------------------------------------------------------
float MPEffectMath::TanTable(float fValue) {
	int idx = int(fValue * _TrigTableFactor) % _TrigTableSize;
	return _TanTable[idx];
}

//-----------------------------------------------------------------------------
void MPEffectMath::setAngleUnit(AngleUnit unit) {
	_sAngleUnit = unit;
}

//-----------------------------------------------------------------------------
MPEffectMath::AngleUnit MPEffectMath::getAngleUnit(void) {
	return _sAngleUnit;
}

//-----------------------------------------------------------------------------
float MPEffectMath::AngleUnitsToRadians(float units) {
	if (_sAngleUnit == AU_DEGREE)
		return DegreesToRadians(units);
	else
		return units;
}

//-----------------------------------------------------------------------------
float MPEffectMath::RadiansToAngleUnits(float radians) {
	if (_sAngleUnit == AU_DEGREE)
		return RadiansToDegrees(radians);
	else
		return radians;
}

//-----------------------------------------------------------------------------
float MPEffectMath::AngleUnitsToDegrees(float units) {
	if (_sAngleUnit == AU_DEGREE)
		return units;
	else
		return RadiansToDegrees(units);
}

//-----------------------------------------------------------------------------
float MPEffectMath::DegreesToAngleUnits(float degrees) {
	if (_sAngleUnit == AU_DEGREE)
		return degrees;
	else
		return DegreesToRadians(degrees);
}

//-----------------------------------------------------------------------------
float MPEffectMath::UnitRandom() {
	return asm_rand() / asm_rand_max();
}

//-----------------------------------------------------------------------------
float MPEffectMath::RangeRandom(float fLow, float fHigh) {
	return (fHigh - fLow) * UnitRandom() + fLow;
}

//-----------------------------------------------------------------------------
