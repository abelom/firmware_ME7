/*
 * @file engine_parts.h
 *
 * @date May 27, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "engine_configuration_generated_structures.h"
#include "cyclic_buffer.h"
#include "thermistor_generated.h"

#define MOCK_ADC_SIZE 26

class MockAdcState {
public:
	MockAdcState();
	bool hasMockAdc[MOCK_ADC_SIZE];
	int fakeAdcValues[MOCK_ADC_SIZE];

	void setMockVoltage(int hwChannel, float voltage DECLARE_ENGINE_PARAMETER_SUFFIX);
	int getMockAdcValue(int hwChannel) const;
};

class ThermistorMath : public thermistor_state_s {
public:
	void setConfig(thermistor_conf_s *config);
	void prepareThermistorCurve(thermistor_conf_s *tc);
	float getKelvinTemperatureByResistance(float resistance) const;
	float s_h_a = 0;
	float s_h_b = 0;
	float s_h_c = 0;
	bool isLinear;
private:
	thermistor_conf_s currentConfig = {0,0,0,0,0,0,0};
};

class Accelerometer {
public:
	float x = 0; // G value
	float y = 0;
	float z = 0;
};

class SensorsState {
public:
	SensorsState();
	/**
	 * Performance optimization:
	 * log() function needed for thermistor logic is relatively heavy, to avoid it we have these
	 * pre-calculated values
	 * Access to these two fields is not synchronized in any way - that should work since float read/write are atomic.
	 *
	 * values are in Celsius
	 */
	float iat = NAN;
#if EFI_UNIT_TEST
	float mockClt = NAN;
#endif
	float clt = NAN;

	Accelerometer accelerometer;

	float vBatt = 0;
	float currentAfr;
	/**
	 * that's fuel in tank - just a gauge
	 */
	percent_t fuelTankLevel = 0;
};

class FuelConsumptionState {
public:
	FuelConsumptionState();
	void addData(float durationMs);
	void update(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);
	float perSecondConsumption = 0;
	float perMinuteConsumption = 0;
	float perSecondAccumulator = 0;
	float perMinuteAccumulator = 0;
	efitick_t accumulatedSecondPrevNt;
	efitick_t accumulatedMinutePrevNt;
};

class TransmissionState {
public:
	TransmissionState();
	gear_e gearSelectorPosition;
};

class WarningCodeState {
public:
	WarningCodeState();
	void addWarningCode(obd_code_e code);
	bool isWarningNow(efitimesec_t now, bool forIndicator DECLARE_ENGINE_PARAMETER_SUFFIX) const;
	void clear();
	int warningCounter;
	int lastErrorCode;
	efitimesec_t timeOfPreviousWarning;
	// todo: we need a way to post multiple recent warnings into TS
	cyclic_buffer<int, 8> recentWarnings;
};

class FsioState {
public:
	FsioState();
	float fsioTimingAdjustment = 0;
	float fsioIdleTargetRPMAdjustment = 0;
	float servoValues[SERVO_COUNT];
	float fsioLastValue[FSIO_COMMAND_COUNT];

	float fsioIdleOffset = 0;
	float fsioIdleMinValue = 0;

#if EFI_UNIT_TEST
	float mockFan = 0;
	float mockRpm = 0;
	float mockCrankingRpm = 0;
	float mockTimeSinceBoot = 0;
	int mockAcToggle = 0;
#endif

#if EFI_ENABLE_ENGINE_WARNING
	/**
	 * Shall we purposely miss on some cylinders in order to attract driver's attention to some problem
	 * like getting too hot
	 */
	float isEngineWarning;
#endif /* EFI_ENABLE_ENGINE_WARNING */

#if EFI_ENABLE_CRITICAL_ENGINE_STOP
	/**
	 * Shall we stop engine due to some critical condition in order to save the engine
	 */
	float isCriticalEngineCondition;
#endif /* EFI_ENABLE_CRITICAL_ENGINE_STOP */
};

/**
 * 6 crossing over 50% TPS means pressing and releasing three times
 * TODO: looks like this code is not finished / not used?
 */
#define PUMPS_TO_PRIME 6

class StartupFuelPumping {
public:
	StartupFuelPumping();
	void update(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	bool isTpsAbove50;
	int pumpsCounter;
private:
	void setPumpsCounter(int newValue);
};

struct multispark_state
{
	efitick_t delay;
	efitick_t dwell;
	uint8_t count;
};
