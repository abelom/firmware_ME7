/*
 * @file	tunerstudio_configuration.h
 * @brief	Tuner Studio connectivity configuration
 *
 * In this file the configuration of TunerStudio is defined
 *
 * @date Oct 22, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "rusefi_types.h"
#include "scaled_channel.h"
#include "tunerstudio_debug_struct.h"

typedef struct {
	uint16_t values[EGT_CHANNEL_COUNT];
} egt_values_s;


enum class TsCalMode : uint8_t {
	None = 0,
	Tps1Max = 1,
	Tps1Min = 2,
	EtbKp = 3,
	EtbKi = 4,
	EtbKd = 5,
};

/**
 * At the moment rusEfi does NOT have any code generation around TS output channels, three locations have to be changed manually
 * 1) this TunerStudioOutputChannels firmware version of the structure
 * 2) '[OutputChannels]' block in rusefi.input
 * 3) com.rusefi.core.Sensor enum in rusEfi console source code
 *
 * please be aware that 'float' (F32) type requires TunerStudio version 2.6 and later
 */
typedef struct {
	/* see also [OutputChannels] in rusefi.input */

	/**
	 * Yes, I do not really enjoy packing bits into integers but we simply have too many boolean flags and I cannot
	 * water 4 bytes per traffic - I want gauges to work as fast as possible
	 */
	unsigned int hasSdCard : 1; // bit 0, 72
	unsigned int isIgnitionEnabled : 1; // bit 1
	unsigned int isInjectionEnabled : 1; // bit 2
	unsigned int isCylinderCleanupEnabled : 1; // bit 3
	unsigned int isCylinderCleanupActivated : 1; // bit 4
	unsigned int isFuelPumpOn : 1; // bit 5
	unsigned int isFanOn : 1; // bit 6
	unsigned int isO2HeaterOn : 1; // bit 7
	unsigned int checkEngine : 1; // bit 8
	unsigned int needBurn : 1; // bit 9
	unsigned int secondTriggerChannelEnabled : 1; // bit 10
	unsigned int clutchUpState : 1; // bit 11
	unsigned int clutchDownState : 1; // bit 12
	unsigned int knockEverIndicator : 1; // bit 13
	unsigned int knockNowIndicator : 1; // bit 14
	unsigned int brakePedalState : 1; // bit 15. 0 - not pressed, 1 = pressed
	unsigned int toothLogReady : 1; // bit 16
	unsigned int acSwitchState : 1; // bit 17. 0 - not pressed, 1 = pressed
	unsigned int isTpsError : 1; // bit 18
	unsigned int isCltError : 1; // bit 19
	unsigned int isMapError : 1; // bit 20
	unsigned int isIatError : 1; // bit 21
	unsigned int isAcSwitchEngaged : 1; // bit 22
	unsigned int isTriggerError : 1; // bit 23
	unsigned int hasCriticalError : 1; // bit 24
	unsigned int isWarnNow : 1; // bit 25
	unsigned int isPedalError : 1; // bit 26
	unsigned int isKnockChipOk : 1; // bit 27
	unsigned int rpmHardCut : 1; // bit 28
	unsigned int launchTpsCond : 1; // bit 29
    unsigned int launchSwitchCond : 1; // bit 30
    unsigned int unused : 1; // bit 30
	// RPM, vss
	scaled_channel<uint16_t> rpm;   // 4
	scaled_percent rpmAcceleration; // 6
	scaled_percent speedToRpmRatio; // 8
	scaled_channel<uint8_t> vehicleSpeedKph; // 10
	
	// temperatures
	scaled_channel<int8_t> internalMcuTemperature; // offset 11
	scaled_temperature coolantTemperature;   // offset 12
	scaled_temperature intakeAirTemperature; // offset 14
	scaled_temperature auxTemp1;             // offset 16
	scaled_temperature auxTemp2;             // offset 18

	// throttle, pedal
	scaled_percent throttlePosition;    // 20
	scaled_percent pedalPosition;       // 22
	uint16_t tpsADC;                    // 24

	// air flow/mass measurment
	scaled_voltage massAirFlowVoltage; // 26
	scaled_channel<uint16_t, 100> massAirFlow; // 28
	scaled_pressure manifoldAirPressure; // 30
	scaled_pressure baroPressure; // 32

	scaled_afr airFuelRatio; // 34
	scaled_channel<uint16_t, 100> engineLoad; // 36

	// misc sensors
	scaled_voltage vBatt; // 38
	scaled_pressure oilPressure; // 40
	scaled_angle vvtPosition; // 42

	// Fuel math
	scaled_channel<uint16_t, 1000> chargeAirMass; // 44
	scaled_ms crankingFuelMs; // 46
	scaled_afr currentTargetAfr; // 48
	// This is the raw value we take from the fuel map or base fuel algorithm, before the corrections
	scaled_ms fuelBase; // 50
	// Total fuel with CLT, IAT and TPS acceleration without injector lag corrections per cycle, as pulse per cycle
	scaled_ms fuelRunning; // 52
	// Actual last injection time - including all compensation and injection mode
	scaled_ms actualLastInjection; // 54
	scaled_channel<uint8_t, 2> injectorDutyCycle; // 56
	scaled_channel<uint8_t, 2> veValue; // 57
	scaled_angle injectionOffset; // 58
	scaled_temperature tCharge; // 60

	// Corrections
	scaled_ms injectorLagMs; // 62
	scaled_percent iatCorrection; // 64
	scaled_percent cltCorrection; // 66
	scaled_percent baroCorrection; // 68
	scaled_ms fuelPidCorrection; // 70

	// Wall model AE
	scaled_ms wallFuelAmount; // 72
	scaled_channel<int16_t, 1000> wallFuelCorrection; // 74
	
	// TPS/load AE
	scaled_percent engineLoadDelta; // 76
	scaled_percent deltaTps; // 78
	scaled_percent engineLoadAccelExtra; // 80
	scaled_ms tpsAccelFuel; // 82

	// Ignition
	scaled_angle ignitionAdvance; // 84
	scaled_ms sparkDwell; // 86
	scaled_percent coilDutyCycle; // 88

	// Idle & ETB
	scaled_percent idlePosition; // 90
	scaled_percent etbTarget; // 92
	scaled_percent etb1DutyCycle; // 94
	scaled_percent etb1Error; // 96

	// Fuel system
	scaled_percent fuelTankLevel; // 98
	float fuelConsumptionPerHour; // 100

	// Knock
	uint32_t knockCount; // 104
	float knockLevel; // 108

	// Mode, firmware, protocol, run time
	uint32_t timeSeconds; // 112
	uint32_t engineMode; // 116
	uint32_t firmwareVersion; // 120
	uint32_t tsConfigVersion; // 124
//+100
	scaled_percent boostDuty; //128
	scaled_percent vvtDuty; //130
	float tpsAdcMin; //132
	float tpsAdcMax; //136
	int16_t unused1[3]; // 140 142 144
	scaled_percent gpPwm1Duty; //146
	scaled_percent heaterDuty; //148
	int16_t wideBandState;     //150
	int16_t wideBandError;     //152
	int16_t wideBandIdent;   // 154

	int16_t unused2[4];        //156 158 160 162
	uint32_t multiSparks; //164
	scaled_percent etbDirectDutyCycle; //168
	int16_t unused3[25];   //168 + 56
	float calibrationValue;	// 128
	TsCalMode calibrationMode; // 132
	uint8_t padding[3]; // 133-135

//End
	// Errors
	int totalTriggerErrorCounter; // 228
	int orderingErrorCounter; // 232
	int16_t warningCounter; // 236
	int16_t lastErrorCode; // 238
	int16_t recentErrorCodes[8]; // 240

	// Debug
	float debugFloatField1; // 256
	float debugFloatField2;
	float debugFloatField3;
	float debugFloatField4;
	float debugFloatField5;
	float debugFloatField6;
	float debugFloatField7;
	int debugIntField1;
	int debugIntField2;
	int debugIntField3;
	int16_t debugIntField4;
	int16_t debugIntField5; // 298

	// accelerometer
	int16_t accelerationX; // 300
	int16_t accelerationY; // 302

	// EGT
	egt_values_s egtValues; // 304
	scaled_percent throttle2Position;    // 320

	scaled_voltage rawTps1Primary;		// 222
	scaled_voltage rawPpsPrimary;		// 224
	scaled_voltage rawClt;				// 226
	scaled_voltage rawIat;				// 228
	scaled_voltage rawOilPressure;		// 230

	uint8_t unusedAtTheEnd[8]; // we have some unused bytes to allow compatible TS changes

	// Temporary - will remove soon
	TsDebugChannels* getDebugChannels() {
		return reinterpret_cast<TsDebugChannels*>(&debugFloatField1);
	}

	/* see also [OutputChannels] in rusefi.input */
} TunerStudioOutputChannels;

extern TunerStudioOutputChannels tsOutputChannels;
