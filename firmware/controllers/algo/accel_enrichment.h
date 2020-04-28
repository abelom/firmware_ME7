/**
 * @file    accel_enrichment.h
 * @brief   Acceleration enrichment calculator
 *
 * @date Apr 21, 2014
 * @author Dmitry Sidin
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "cyclic_buffer.h"
#include "table_helper.h"
#include "wall_fuel_generated.h"

typedef Map3D<TPS_TPS_ACCEL_TABLE, TPS_TPS_ACCEL_TABLE, float, float> tps_tps_Map3D_t;

/**
 * this object is used for MAP rate-of-change and TPS rate-of-change corrections
 */
class AccelEnrichment {
public:
	AccelEnrichment();
	int getMaxDeltaIndex(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	float getMaxDelta(DECLARE_ENGINE_PARAMETER_SIGNATURE);

	void resetAE();
	void setLength(int length);
	cyclic_buffer<float> cb;
	void onNewValue(float currentValue DECLARE_ENGINE_PARAMETER_SUFFIX);
	int onUpdateInvocationCounter = 0;
};

class LoadAccelEnrichment : public AccelEnrichment {
public:
	/**
	 * @return Extra engine load value for fuel logic calculation
	 */
	float getEngineLoadEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	void onEngineCycle(DECLARE_ENGINE_PARAMETER_SIGNATURE);
};

class TpsAccelEnrichment : public AccelEnrichment {
public:
	/**
	 * @return Extra fuel squirt duration for TPS acceleration
	 */
	floatms_t getTpsEnrichment(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	void onEngineCycleTps(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	void resetFractionValues();
	void resetAE();
private:
	/**
	 * Used for Fractional TPS enrichment. 
	 */
	floatms_t accumulatedValue;
	floatms_t maxExtraPerCycle;
	floatms_t maxExtraPerPeriod;
	floatms_t maxInjectedPerPeriod;
	int cycleCnt;
};

/**
 * Wall wetting, also known as fuel film
 * See https://github.com/rusefi/rusefi/issues/151 for the theory
 */
class WallFuel : public wall_fuel_state {
public:
	/**
	 * @param target desired squirt duration
	 * @return total adjusted fuel squirt duration once wall wetting is taken into effect
	 */
	floatms_t adjust(floatms_t target DECLARE_ENGINE_PARAMETER_SUFFIX);
	floatms_t getWallFuel() const;
	void resetWF();
	int invocationCounter = 0;
};

void initAccelEnrichment(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);

void setEngineLoadAccelLen(int len);
void setEngineLoadAccelThr(float value);
void setEngineLoadAccelMult(float value);

void setTpsAccelThr(float value);
void setTpsDecelThr(float value);
void setTpsDecelMult(float value);
void setTpsAccelLen(int length);

void setDecelThr(float value);
void setDecelMult(float value);

void updateAccelParameters();
