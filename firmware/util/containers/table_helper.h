/**
 * @file	table_helper.h
 *
 * @date Jul 6, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include <math.h>
#include "error_handling.h"
#include "interpolation.h"
#include "efilib.h"

// popular left edge of CLT-based correction curves
#define CLT_CURVE_RANGE_FROM -40

class ValueProvider3D {
public:
	virtual float getValue(float xRpm, float y) const = 0;
};


/**
 * this helper class brings together 3D table with two 2D axis curves
 */
template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
class Map3D : public ValueProvider3D {
public:
	explicit Map3D(const char*name);
	Map3D(const char*name, float multiplier);
	void init(vType table[RPM_BIN_SIZE][LOAD_BIN_SIZE], const kType loadBins[LOAD_BIN_SIZE], const kType rpmBins[RPM_BIN_SIZE]);
	float getValue(float xRpm, float y) const;
	void setAll(vType value);
	vType *pointers[LOAD_BIN_SIZE];
private:
	void create(const char*name, float multiplier);
	const kType *loadBins = NULL;
	const kType *rpmBins = NULL;
	bool initialized =  false;
	const char *name;
	float multiplier;
};

/*
 * this dead code is a questionable performance optimization idea: instead of division every time
 * we want interpolation for a curve we can pre-calculate A and B and save the division at the cost of more RAM usage
 * Realistically we probably value RAM over CPU at this time and the costs are not justified.
template<int SIZE>
class Table2D {
public:
	Table2D();
	void preCalc(float *bin, float *values);
	float aTable[SIZE];
	float bTable[SIZE];
	float *bin;
};
template<int SIZE>
Table2D<SIZE>::Table2D() {
	bin = NULL;
}

template<int SIZE>
void Table2D<SIZE>::preCalc(float *bin, float *values) {
	this->bin = bin;
	for (int i = 0; i < SIZE - 1; i++) {
		float x1 = bin[i];
		float x2 = bin[i + 1];
		if (x1 == x2) {
			warning(CUSTOM_INTEPOLATE_ERROR_4, "preCalc: Same x1 and x2 in interpolate: %.2f/%.2f", x1, x2);
			return;
		}

		float y1 = values[i];
		float y2 = values[i + 1];

		aTable[i] = INTERPOLATION_A(x1, y1, x2, y2);
		bTable[i] = y1 - aTable[i] * x1;
	}
}
*/

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
void Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::init(vType table[RPM_BIN_SIZE][LOAD_BIN_SIZE],
		const kType loadBins[LOAD_BIN_SIZE],
		const kType rpmBins[RPM_BIN_SIZE]) {
	// this method cannot use logger because it's invoked before everything
	// that's because this method needs to be invoked before initial configuration processing
	// and initial configuration load is done prior to logging initialization

  for (int k = 0; k < LOAD_BIN_SIZE; k++) {
		pointers[k] = table[k];
  }
	initialized = true;
	this->loadBins = loadBins;
	this->rpmBins = rpmBins;
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
float Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::getValue(float xRpm, float y) const {
	efiAssert(CUSTOM_ERR_ASSERT, initialized, "map not initialized", NAN);
	if (cisnan(y)) {
		warning(CUSTOM_PARAM_RANGE, "%s: y is NaN", name);
		return NAN;
	}
	// todo: we have a bit of a mess: in TunerStudio, RPM is X-axis
	return multiplier * interpolate3d<vType, kType>(y, loadBins, LOAD_BIN_SIZE, xRpm, rpmBins, RPM_BIN_SIZE, pointers);
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::Map3D(const char *name) {
	create(name, 1);
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::Map3D(const char *name, float multiplier) {
	create(name, multiplier);
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
void Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::create(const char *name, float multiplier) {
	this->name = name;
	this->multiplier = multiplier;
	memset(&pointers, 0, sizeof(pointers));
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
void Map3D<RPM_BIN_SIZE, LOAD_BIN_SIZE, vType, kType>::setAll(vType value) {
	efiAssertVoid(CUSTOM_ERR_6573, initialized, "map not initialized");
	for (int l = 0; l < LOAD_BIN_SIZE; l++) {
		for (int r = 0; r < RPM_BIN_SIZE; r++) {
			pointers[l][r] = value / multiplier;
		}
	}
}

template<int RPM_BIN_SIZE, int LOAD_BIN_SIZE, typename vType, typename kType>
void copy2DTable(const vType source[LOAD_BIN_SIZE][RPM_BIN_SIZE], vType destination[LOAD_BIN_SIZE][RPM_BIN_SIZE]) {
	for (int k = 0; k < LOAD_BIN_SIZE; k++) {
		for (int rpmIndex = 0; rpmIndex < RPM_BIN_SIZE; rpmIndex++) {
			destination[k][rpmIndex] = source[k][rpmIndex];
		}
	}
}

/**
 * AFR value is packed into uint8_t with a multiplier of 10
 */
#define AFR_STORAGE_MULT 10
/**
 * TPS-based Advance value is packed into int16_t with a multiplier of 100
 */
#define ADVANCE_TPS_STORAGE_MULT 100

typedef Map3D<FUEL_RPM_COUNT, FUEL_LOAD_COUNT, uint8_t, float> afr_Map3D_t;
typedef Map3D<IGN_RPM_COUNT, IGN_LOAD_COUNT, float, float> ign_Map3D_t;
typedef Map3D<IGN_RPM_COUNT, IGN_TPS_COUNT, int16_t, float> ign_tps_Map3D_t;
typedef Map3D<FUEL_RPM_COUNT, FUEL_LOAD_COUNT, float, float> fuel_Map3D_t;
typedef Map3D<BARO_CORR_SIZE, BARO_CORR_SIZE, float, float> baroCorr_Map3D_t;
typedef Map3D<PEDAL_TO_TPS_SIZE, PEDAL_TO_TPS_SIZE, uint8_t, uint8_t> pedal2tps_t;
typedef Map3D<BOOST_RPM_COUNT, BOOST_LOAD_COUNT, uint8_t, uint8_t> boostOpenLoop_Map3D_t;
typedef Map3D<BOOST_RPM_COUNT, BOOST_LOAD_COUNT, uint8_t, uint8_t> boostClosedLoop_Map3D_t;
typedef Map3D<VVT_RPM_COUNT, VVT_LOAD_COUNT, float, float> vvt_Map3D_t;

typedef Map3D<IAC_PID_MULT_SIZE, IAC_PID_MULT_SIZE, uint8_t, uint8_t> iacPidMultiplier_t;
typedef Map3D<GP_PWM_RPM_COUNT, GP_PWM_LOAD_COUNT, uint8_t, float> gpPwm1_Map3D_t;
typedef Map3D<GP_PWM_RPM_COUNT, GP_PWM_LOAD_COUNT, uint8_t, float> gpPwm2_Map3D_t;
typedef Map3D<GP_PWM_RPM_COUNT, GP_PWM_LOAD_COUNT, uint8_t, float> gpPwm3_Map3D_t;
typedef Map3D<GP_PWM_RPM_COUNT, GP_PWM_LOAD_COUNT, uint8_t, float> gpPwm4_Map3D_t;
void setRpmBin(float array[], int size, float idleRpm, float topRpm);

/**
 * @param precision for example '0.1' for one digit fractional part. Default to 0.01, two digits.
 */
template<typename TValue, int TSize>
void setLinearCurve(TValue (&array)[TSize], float from, float to, float precision = 0.01f) {
	for (int i = 0; i < TSize; i++) {
		float value = interpolateMsg("setLinearCurve", 0, from, TSize - 1, to, i);

		/**
		 * rounded values look nicer, also we want to avoid precision mismatch with Tuner Studio
		 */
		array[i] = efiRound(value, precision);
	}
}

template<typename TValue, int TSize>
void setArrayValues(TValue (&array)[TSize], TValue value) {
	for (int i = 0; i < TSize; i++) {
		array[i] = value;
	}
}

void setRpmTableBin(float array[], int size);
