/**
 * @file engine_state.h
 * @brief One header which acts as gateway to current engine state
 *
 * @date Dec 20, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "engine_parts.h"
#include "pid.h"
#include "engine_state_generated.h"

#define BRAIN_PIN_COUNT (1 << sizeof(brain_pin_e))

class EngineState : public engine_state2_s {
public:
	EngineState();
	void periodicFastCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	void updateSlowSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	void updateTChargeK(int rpm, float tps DECLARE_ENGINE_PARAMETER_SUFFIX);

#if ! EFI_PROD_CODE
	bool mockPinStates[BRAIN_PIN_COUNT];
#endif

	FuelConsumptionState fuelConsumption;

	efitick_t crankingTime = 0;
	efitick_t timeSinceCranking = 0;

	WarningCodeState warnings;

	/**
	 * speed-density logic, calculated air flow in kg/h for tCharge Air-Interp. method
	 */
	float airFlow = 0;

	float engineNoiseHipLevel = 0;

	float auxValveStart = 0;
	float auxValveEnd = 0;

	// too much copy-paste here, something should be improved :)
	ThermistorMath iatCurve;
	ThermistorMath cltCurve;

	/**
	 * MAP averaging angle start, in relation to 'mapAveragingSchedulingAtIndex' trigger index index
	 */
	angle_t mapAveragingStart[INJECTION_PIN_COUNT];
	angle_t mapAveragingDuration = 0;

	/**
	 * timing advance is angle distance before Top Dead Center (TDP), i.e. "10 degree timing advance" means "happens 10 degrees before TDC"
	 */
	angle_t timingAdvance = 0;
	// fuel-related;
	float fuelCutoffCorrection = 0;
	efitick_t coastingFuelCutStartTime = 0;

	efitick_t timeSinceLastTChargeK;

	float currentRawVE = 0;

	int vssEventCounter = 0;


	/**
	 * pre-calculated value from simple fuel lookup
	 */
	floatms_t baseTableFuel = 0;
	/**
	 * Raw fuel injection duration produced by current fuel algorithm, without any correction
	 */
	floatms_t baseFuel = 0;

	/**
	 * TPS acceleration: extra fuel amount
	 */
	floatms_t tpsAccelEnrich = 0;

	angle_t injectionOffset = 0;

#if EFI_ENABLE_MOCK_ADC
	MockAdcState mockAdcState;
#endif /* EFI_ENABLE_MOCK_ADC */

	multispark_state multispark;
};
