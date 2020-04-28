

#include "global.h"
#include "engine.h"
#include "engine_math.h"
#include "allsensors.h"
#include "fsio_impl.h"
#include "engine_configuration.h"
EXTERN_CONFIG;
EXTERN_ENGINE;
#undef SERIAL_SPEED
#define SERIAL_SPEED 115200

//Assembled 2

static const float injectorLagBins[VBAT_INJECTOR_CURVE_SIZE] = {
        6.0,         8.0,        10.0,
        12.0,         14.0,        15.0
};
static const float injectorLagCorrection[VBAT_INJECTOR_CURVE_SIZE] = {
        4.0 ,        3.0 ,        2.0 ,
        1.5 ,                1.25 ,        1.20
};

static const float dwellBins[DWELL_CURVE_SIZE] = {
        0,         1000,        2000,        3000,
        4000,        5000,  6000,        7000
};
static const float dwellValues[DWELL_CURVE_SIZE] = {
        5.0 ,        4.5 ,        4.0 ,        4.0,
        4.0 ,        4.0,        4.0 ,        3.0
};

static const fuel_table_t veTable = {
		         {42.0, 41.6, 42.2, 43.4, 44.6, 46.0, 47.5, 49.0, 50.5, 52.0, 53.5, 55.0, 56.5, 58.0, 59.5, 61.0, },
		         {44.8, 45.3, 45.8, 46.8, 47.9, 49.1, 50.4, 51.7, 53.0, 54.2, 55.5, 56.8, 58.1, 59.4, 60.6, 61.9, },
		         {48.6, 49.09, 49.5, 50.3, 51.2, 52.2, 53.3, 54.4, 55.4, 56.5, 57.5, 58.6, 59.7, 60.7, 61.8, 62.8,},
		         {52.5, 52.8, 53.2, 53.8, 54.5, 55.3, 56.2, 57.0, 57.9, 58.7, 59.6, 60.4, 61.2, 62.1, 62.9, 63.8, },
		         {56.3, 56.6, 56.8, 57.3, 57.8, 58.4, 59.0, 59.7, 60.3, 60.9, 61.6, 62.2, 62.8, 63.5, 64.1, 64.7, },
		         {60.1, 60.3, 60.5, 60.8, 61.1, 61.5, 61.9, 62.3, 62.8, 63.2, 63.6, 64.0, 64.4, 64.8, 65.2, 65.6, },
		         {64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 64.0, 65.0, 65.0, 65.4, 65.6, 65.8, 66.0, 66.2, 66.4, 66.6, },
		         {68.0, 71.0, 73.0, 72.0, 72.0, 71.0, 70.0, 71.0, 73.0, 74.0, 76.0, 77.0, 77.0, 76.0, 76.0, 76.0, },
		         {71.0, 73.0, 73.0, 72.0, 72.0, 71.0, 70.0, 71.0, 73.0, 74.0, 76.0, 77.0, 77.0, 76.0, 76.0, 76.0, },
		         {74.0, 76.0, 75.0, 75.0, 76.0, 76.0, 75.0, 75.0, 76.0, 77.0, 78.0, 79.0, 79.0, 79.0, 78.0, 78.0, },
		         {75.0, 76.0, 77.0, 80.0, 81.0, 82.0, 83.0, 82.0, 83.0, 83.0, 84.0, 84.0, 84.0, 83.0, 83.0, 83.0, },
		         {75.0, 76.0, 77.0, 81.0, 83.0, 85.0, 87.0, 87.0, 88.0, 88.0, 87.0, 87.0, 87.0, 85.0, 84.0, 84.0, },
		         {75.0, 76.0, 77.0, 81.0, 84.0, 87.0, 89.0, 90.0, 91.0, 91.0, 89.0, 89.0, 89.0, 86.0, 84.0, 84.0, },
		         {75.0, 76.0, 77.0, 81.0, 84.0, 87.0, 90.0, 91.0, 93.0, 93.0, 92.0, 91.0, 91.0, 87.0, 84.0, 84.0, },
				 {75.0, 76.0, 77.0, 81.0, 84.0, 87.0, 90.0, 91.0, 93.0, 93.0, 92.0, 91.0, 91.0, 87.0, 84.0, 84.0, },
		         {75.0, 76.0, 77.0, 81.0, 84.0, 87.0, 89.0, 91.0, 93.0, 93.0, 92.0, 91.0, 91.0, 87.0, 84.0, 84.0, }
};

const float veRpmBins[FUEL_RPM_COUNT] = {
		700.0, 820.0, 950.0, 1100.0,
		1300.0, 1550.0, 1800.0, 2150.0,
		2500.0, 3000.0, 3500.0, 4150.0,
		4900.0, 5800.0, 6800.0, 8000.0
};

const float veLoadBins[FUEL_LOAD_COUNT] = {
		10, 20, 40, 60,
		70, 80, 100, 120,
		140, 160, 180, 200.0,
		220.0, 240.0, 260.0, 280.0
};
static const ignition_table_t ignitionTimingTable = {
		{24.00,	14.20,	18.10,	22.00,	25.90,	27.00,	35.00,	35.78,	36.56,	37.33,	38.11,	38.89,	39.67,	40.44,	41.22,	42.00,	},
		{24.00,	14.00,	17.90,	21.70,	25.60,	29.40,	34.25,	35.01,	35.78,	36.54,	37.31,	38.07,	38.83,	39.60,	40.36,	41.13,	},
		{24.00,	13.90,	17.70,	21.50,	25.30,	29.00,	33.50,	34.25,	35.00,	35.75,	36.50,	37.25,	38.00,	38.75,	39.50,	40.25,	},
		{24.00,	13.70,	17.40,	21.20,	24.90,	28.60,	32.75,	33.49,	34.22,	34.96,	35.69,	36.43,	37.17,	37.90,	38.64,	39.38,	},
		{24.00,	13.50,	17.10,	20.80,	24.40,	28.00,	32.00,	32.72,	33.44,	34.17,	34.89,	35.61,	36.33,	37.06,	37.78,	38.50,	},
		{19.00,	13.30,	16.90,	20.50,	24.10,	27.60,	31.25,	31.96,	32.67,	33.38,	34.08,	34.79,	35.50,	36.21,	36.92,	37.63,	},
		{16.00,	13.20,	16.70,	20.20,	23.70,	27.20,	30.50,	31.19,	31.89,	32.58,	33.28,	33.97,	34.67,	35.36,	36.06,	36.75,	},
		{8.00,	8.00,	9.00,	11.00,	17.00,	25.00,	29.00,	29.67,	30.33,	31.00,	31.67,	32.33,	33.00,	33.67,	34.33,	35.00,	},
		{5.00,	5.00,	7.00,	8.00,	11.00,	16.00,	18.00,	19.00,	19.00,	20.70,	21.60,	22.45,	23.30,	24.15,	25.00,	26.00,	},
		{4.00,	4.00,	5.81,	7.04,	6.00,	9.83,	11.37,	12.92,	14.47,	16.01,	19.27,	20.08,	20.89,	21.69,	22.50,	19.00,	},
		{4.00,	4.00,	4.00,	6.31,	4.70,	8.75,	10.10,	11.45,	12.81,	14.16,	16.94,	17.71,	18.47,	19.24,	20.00,	16.00,	},
		{3.00,	3.00,	4.64,	5.57,	4.10,	7.66,	8.83,	9.99,	11.15,	12.31,	14.61,	15.34,	16.06,	16.78,	17.50,	14.00,	},
		{3.00,	3.00,	4.06,	3.00,	3.60,	6.58,	7.55,	8.52,	9.49,	10.46,	12.29,	12.96,	13.64,	14.32,	15.00,	12.00,	},
		{2.00,	3.00,	3.00,	3.00,	3.30,	5.50,	6.28,	7.05,	7.83,	8.61,	9.96,	10.59,	11.23,	11.86,	12.50,	10.00,  },
		{2.00,	3.00,	3.00,	3.00,	3.30,	4.42,	5.00,	5.59,	6.17,	6.76,	7.63,	8.22,	8.81,	9.41,	10.00,	9.00,   },
		{2.00,	3.00,	3.00,	3.00,	3.30,	3.34,	3.73,	4.12,	4.51,	4.91,	5.30,	5.85,	6.40,	6.95,	7.50,	8.00,   }

};
const float ignitionRpmBins[IGN_LOAD_COUNT] = {
		700.0, 820.0, 950.0, 1100.0,
		1300.0, 1550.0, 1800.0, 2150.0,
		2500.0, 3000.0, 3500.0, 4150.0,
		4900.0, 5800.0, 6800.0, 8000.0
};

const float ignitionLoadBins[IGN_LOAD_COUNT] = {
		10, 20, 40, 60,
		70, 80, 100, 120,
		140, 160, 180, 200.0,
		220.0, 240.0, 260.0, 280.0
};
const float crankingCycleBins[] = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8
	};
const float crankingCycleValues[] = {
		1.80,
		1.10,
		1.05,
		1,
		1,
		1,
		1,
		1
	};

const float afterstartEnrich[] = {
		1.8,
		1.6,
		1.4,
		1.0,
		1.09,
		1.08,
		1.06,
		1.05
	};



static const float gpPwmCoolantFanLoad[GP_PWM_LOAD_COUNT] = {0.0, 40.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0};

float const gpPwmCoolantFan[GP_PWM_LOAD_COUNT][GP_PWM_RPM_COUNT] = {
		{15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000,},
		{15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000,},
		{15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000, 15.000,},
		{20.000, 20.000, 20.000, 20.000, 20.000, 20.000, 20.000, 20.000,},
		{70.000, 70.000, 70.000, 70.000, 70.000, 70.000, 70.000, 70.000,},
		{90.000, 90.000, 90.000, 90.000, 90.000, 90.000, 90.000, 90.000,},
		{100.000, 100.000, 100.000,	100.000, 100.000, 100.000, 100.000, 100.000,},
		{100.000, 100.000, 100.000,	100.000, 100.000, 100.000, 100.000,	100.000,},
		};

void setDefaultMaps(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	memcpy(engineConfiguration->sparkDwellRpmBins, dwellBins, sizeof(dwellBins));
	memcpy(engineConfiguration->sparkDwellValues, dwellValues, sizeof(dwellValues));
	memcpy(engineConfiguration->injector.battLagCorr, injectorLagCorrection, sizeof(injectorLagCorrection));
	memcpy(engineConfiguration->injector.battLagCorrBins, injectorLagBins, sizeof(injectorLagBins));


	memcpy(config->veRpmBins, veRpmBins, sizeof(veRpmBins));
	memcpy(config->veLoadBins, veLoadBins, sizeof(veLoadBins));
	copyFuelTable(veTable, config->veTable);
	memcpy(config->ignitionRpmBins, ignitionRpmBins, sizeof(ignitionRpmBins));
	memcpy(config->ignitionLoadBins, ignitionLoadBins, sizeof(ignitionLoadBins));
	copyTimingTable(ignitionTimingTable, config->ignitionTable);
	memcpy(config->crankingCycleBins, crankingCycleBins, sizeof(crankingCycleBins));
	memcpy(config->crankingCycleCoef, crankingCycleValues, sizeof(crankingCycleValues));
	memcpy(config->afterstartEnrich, afterstartEnrich, sizeof(afterstartEnrich));



	//memcpy(config->gpPwm1LoadBins, gpPwmCoolantFanLoad, sizeof(gpPwmCoolantFanLoad));
	// todo: there should be a better way?
//	setLinearCurve(config->gpPwm1RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
//	setLinearCurve(config->gpPwm1LoadBins, 0, 130, 1);

//	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
//		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
//			config->gpPwmTable1[loadIndex][rpmIndex] = gpPwmCoolantFan[loadIndex][rpmIndex];
//		}
//	}
}

void setPinConfigurationOverrides(void) {
}

void setSerialConfigurationOverrides(void) {
	engineConfiguration->useSerialPort = true;
	engineConfiguration->binarySerialTxPin = GPIOC_10;
	engineConfiguration->binarySerialRxPin = GPIOC_11;
	engineConfiguration->consoleSerialTxPin = GPIOC_10;
	engineConfiguration->consoleSerialRxPin = GPIOC_11;

}
static void setLedPins() {
	engineConfiguration->communicationLedPin = GPIO_UNASSIGNED; // d23 = blue
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;		// d22 = green
	engineConfiguration->triggerErrorPin = GPIO_UNASSIGNED;		// d27 = orange
}


static void setInjectorPins() {
	engineConfiguration->injectionPinMode = OM_DEFAULT;
	engineConfiguration->injectionPins[0] = GPIOE_2; // #1
	engineConfiguration->injectionPins[1] = GPIOB_9; // #2
	engineConfiguration->injectionPins[2] = GPIOE_1; // #3
	engineConfiguration->injectionPins[3] = GPIOE_0; // #4
	engineConfiguration->injectionPins[4] = GPIOE_5; // #5
	engineConfiguration->injectionPins[5] = GPIOE_4; // #6
	engineConfiguration->injectionPins[6] = GPIOB_8; // #7
	engineConfiguration->injectionPins[7] = GPIOB_7; // #8

	// Disable remainder
	for (int i = 8; i < INJECTION_PIN_COUNT; i++) {
		engineConfiguration->injectionPins[i] = GPIO_UNASSIGNED;
	}
}

static void setIgnitionPins() {
	engineConfiguration->ignitionPinMode = OM_INVERTED;
	engineConfiguration->ignitionPins[0] = GPIOD_2; // #1
	engineConfiguration->ignitionPins[1] = GPIOD_1; // #2
	engineConfiguration->ignitionPins[2] = GPIOD_4; // #3
	engineConfiguration->ignitionPins[3] = GPIOD_0; // #4
	engineConfiguration->ignitionPins[4] = GPIOD_3; // #5
	engineConfiguration->ignitionPins[5] = GPIOD_6; // #6
	engineConfiguration->ignitionPins[6] = GPIOC_12; // #7
	engineConfiguration->ignitionPins[7] = GPIOC_9; // #8

	// disable remainder
	for (int i = 8; i < IGNITION_PIN_COUNT; i++) {
		engineConfiguration->ignitionPins[i] = GPIO_UNASSIGNED;
	}
}

static void setupEtb() {
	engineConfiguration->etb_use_two_wires = true;
	engineConfiguration->etb.pFactor = 12;
	engineConfiguration->etb.iFactor = 15;
	engineConfiguration->etb.dFactor = 0;
	engineConfiguration->etb.offset = 0;
	engineConfiguration->etbFreq = 500;
	engineConfiguration->useETBforIdleControl = true;
	engineConfiguration->etbCalibrationOnStart = false;
}
static void setDefaultOutputs() {
	engineConfiguration->etbIo[0].directionPin1 = GPIOG_9;
	engineConfiguration->etbIo[0].directionPin2 = GPIOD_7;
	engineConfiguration->wboHeaterPin = GPIOE_9;
	engineConfiguration->cj125ModePin = GPIOE_12;
	engineConfiguration->triggerInputPins[0] = GPIOC_6;
	engineConfiguration->camInputs[0] = GPIOA_8;
	engineConfiguration->boostControlPin = GPIOE_3;

	engineConfiguration->clutchDownPin = GPIOE_7;
	engineConfiguration->brakePedalPin = GPIOF_15;
    engineConfiguration->triggerInputPins[0] = GPIOC_6;
	engineConfiguration->fuelPumpPin = GPIOG_3;
	engineConfiguration->mainRelayPin = GPIO_UNASSIGNED;
	engineConfiguration->idle.solenoidPin = GPIO_UNASSIGNED;
	engineConfiguration->fanPin = GPIO_UNASSIGNED;
	engineConfiguration->triggerInputPins[1] = GPIO_UNASSIGNED;

}

static void setupDefaultSensorInputs() {
	engineConfiguration->vbattAdcChannel = EFI_ADC_0;
	engineConfiguration->cj125ur = EFI_ADC_1;
	engineConfiguration->cj125ua = EFI_ADC_2;
	engineConfiguration->throttlePedalPositionAdcChannel = EFI_ADC_5;
	engineConfiguration->mafAdcChannel = EFI_ADC_7;
	engineConfiguration->iat.adcChannel = EFI_ADC_8;
	engineConfiguration->map.sensor.hwChannel = EFI_ADC_12;
	engineConfiguration->clt.adcChannel = EFI_ADC_13;
	engineConfiguration->tps1_1AdcChannel = EFI_ADC_15;
	engineConfiguration->tps1_2AdcChannel = EFI_ADC_9;
	engineConfiguration->tps2_1AdcChannel = EFI_ADC_NONE;
	engineConfiguration->tps2_2AdcChannel = EFI_ADC_NONE;
	engineConfiguration->throttlePedalPositionSecondAdcChannel = EFI_ADC_4;
	engineConfiguration->afr.hwChannel = EFI_ADC_NONE;
}


static void setupVbatt() {
	engineConfiguration->vbattDividerCoeff = 8.166666f;
	engineConfiguration->analogInputDividerCoefficient = 2.5f / 1.5f;
	engineConfiguration->adcVcc = 3.00f;
}

static void spiSetup() {
	engineConfiguration->is_enabled_spi_1 = true;
	engineConfiguration->tle8888spiDevice = SPI_DEVICE_1;
	engineConfiguration->tle8888_cs = GPIOD_5;

	engineConfiguration->spi1mosiPin = GPIOB_5;
	engineConfiguration->spi1misoPin = GPIOB_4;
	engineConfiguration->spi1sckPin = GPIOB_3;

	engineConfiguration->is_enabled_spi_2 = true;
	engineConfiguration->cj125SpiDevice = SPI_DEVICE_2;
	engineConfiguration->cj125CsPin = GPIOE_13;

    engineConfiguration->spi2mosiPin = GPIOB_15;
	engineConfiguration->spi2misoPin = GPIOB_14;
	engineConfiguration->spi2sckPin = GPIOB_13;

	engineConfiguration->is_enabled_spi_3 = false;
}

static void setupWideband() {
	engineConfiguration->cj125ModePinMode = OM_DEFAULT;
	engineConfiguration->cj125CsPinMode = OM_DEFAULT;
	engineConfiguration->isCJ125Enabled = true;

	engineConfiguration->cj125isUaDivided = true;
	engineConfiguration->cj125isUrDivided = true;
}

static void setEngineDefaults() {
	engineConfiguration->engineType = MINIMAL_PINS;
	setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);
	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
	engineConfiguration->trigger.type = TT_60_2_VW;
	engineConfiguration->useOnlyRisingEdgeForTrigger = true;

	engineConfiguration->specs.cylindersCount = 4;
	engineConfiguration->specs.firingOrder = FO_1_3_4_2;
	engineConfiguration->injector.flow = 205;
	engineConfiguration->specs.displacement = 1.795;
	engineConfiguration->globalTriggerAngleOffset = 114;

	engineConfiguration->ignitionMode = IM_INDIVIDUAL_COILS;
	engineConfiguration->crankingInjectionMode = IM_SEQUENTIAL;
	engineConfiguration->injectionMode = IM_SEQUENTIAL;
	engineConfiguration->useRunningMathForCranking =true;
	engineConfiguration->isCylinderCleanupEnabled = true;
	engineConfiguration->rpmHardLimit = 8000;
	engineConfiguration->cranking.baseFuel = 5;
	engineConfiguration->tpsMin = 118;
	engineConfiguration->tpsMax = 1000;
	engineConfiguration->tpsErrorDetectionTooLow = -10; // -10% open
	engineConfiguration->tpsErrorDetectionTooHigh = 110; // 110% open

	engineConfiguration->oilPressure.v1 = 0.5f;
	engineConfiguration->oilPressure.v2 = 4.5f;
	engineConfiguration->oilPressure.value1 = 0;
	engineConfiguration->oilPressure.value2 = 689.476f;	// 100psi = 689.476kPa

	engineConfiguration->multisparkEnable = true;
	engineConfiguration->multisparkMaxRpm =1200;
	engineConfiguration->multisparkMaxSparkingAngle = 45;
	engineConfiguration->multisparkDwell = 3;
	engineConfiguration->multisparkSparkDuration = 1;
	engineConfiguration->multisparkMaxExtraSparkCount = 6;

	engineConfiguration->isFastAdcEnabled = true;
		// Map Sensor

	engineConfiguration->map.sensor.type = MT_CUSTOM;
	engineConfiguration->map.sensor.lowValue = 0;
	engineConfiguration->map.sensor.highValue = 230;
	engineConfiguration->mapLowValueVoltage = 0;
	engineConfiguration->mapHighValueVoltage = 5;

	engineConfiguration->idlePidRpmUpperLimit = 500;
	engineConfiguration->idlePidRpmDeadZone = 50;
	engineConfiguration->idleRpmPid.periodMs = 10;
	engineConfiguration->idleRpmPid.pFactor = 0.05;
	engineConfiguration->idleRpmPid.iFactor = 0.002;

	engineConfiguration->idleRpmPid.minValue = 0.1;
	engineConfiguration->idleRpmPid.maxValue = 99;

	engineConfiguration->idlerpmpid_iTermMin = -40;
	engineConfiguration->idlerpmpid_iTermMax = 40;
	engineConfiguration->idlePidDeactivationTpsThreshold = 2;
	engineConfiguration->idle.solenoidFrequency = 200;


	engineConfiguration->clt.config.bias_resistor = 2700;
	engineConfiguration->iat.config.bias_resistor = 2700;

	engineConfiguration->silentTriggerError = true;
	engineConfiguration->primingSquirtDurationMs = 5;
	engineConfiguration->cranking.rpm = 400;
	engineConfiguration->startOfCrankingPrimingPulse = 40;
	engineConfiguration->primeInjFalloffTemperature = 90;

	engineConfiguration->useConstantDwellDuringCranking = false;
	engineConfiguration->ignitionDwellForCrankingMs = 5;
	engineConfiguration->useSeparateAdvanceForCranking = false;
	engineConfiguration->crankingTimingAngle = 7;

	engineConfiguration->manIdlePosition = 70;
	engineConfiguration->crankingIACposition = 70;
	engineConfiguration->etbIdleThrottleRange = 10;

	engineConfiguration->boostControlPinMode = OM_DEFAULT;
	engineConfiguration->isBoostControlEnabled = true;
	engineConfiguration->boostPwmFrequency = 55;
	engineConfiguration->boostPid.offset = 0;
	engineConfiguration->boostPid.pFactor = 0.5;
	engineConfiguration->boostPid.iFactor = 0.3;
	engineConfiguration->boostPid.periodMs = 100;
	engineConfiguration->boostPid.maxValue = 99;
	engineConfiguration->boostPid.minValue = -99;

}
void setLaunchParameters() {
	engineConfiguration->launchRpm = 4000;        // Rpm to trigger Launch condition
	engineConfiguration->launchTimingRetard = 10;  // retard in absolute degrees ATDC
	engineConfiguration->launchTimingRpmRange = 500;   // Rpm above Launch triggered for full retard
	engineConfiguration->launchSparkCutEnable = true;
	engineConfiguration->launchFuelCutEnable = false;
	engineConfiguration->hardCutRpmRange = 500;       //Rpm above Launch triggered +(if retard enabled) launchTimingRpmRange to hard cut
	engineConfiguration->launchSpeedTreshold = 10;   //maximum speed allowed before disable launch
	engineConfiguration->launchFuelAdded = 10;       // Extra fuel in % when launch are triggered
	engineConfiguration->launchBoostDuty = 70;       // boost valve duty cycle at launch
	engineConfiguration->launchActivateDelay = 3;    // Delay in seconds for launch to kick in
	engineConfiguration->enableLaunchRetard = true;
	engineConfiguration->enableLaunchBoost= true;
	engineConfiguration->launchSmoothRetard = true; //interpolates the advance linear from launchrpm to fully retarded at launchtimingrpmrange

}

void setengineConfigurationOverrides(void) {
	//CAN Settings

	engineConfiguration->canNbcType = CAN_BUS_NBC_VAG;
	engineConfiguration->canReadEnabled = true;
	engineConfiguration->canWriteEnabled = true;
	engineConfiguration->canTxPin = GPIOB_6;
	engineConfiguration->canRxPin = GPIOB_12;



	//ETB Settings
}
	void setBoardConfigurationOverrides(void) {

	setLedPins();
	setInjectorPins();
	setIgnitionPins();
	setupVbatt();
	spiSetup();
	setupEtb();
	setupDefaultSensorInputs();
	setDefaultOutputs();
	setupWideband();
	setEngineDefaults();
	setengineConfigurationOverrides();
	setDefaultMaps(PASS_CONFIG_PARAMETER_SIGNATURE);
	setLaunchParameters();




	// NOT USED JUNK
	engineConfiguration->isEngineChartEnabled = false;
	engineConfiguration->useLcdScreen = false;
	engineConfiguration->etbIo[1].directionPin1 = GPIO_UNASSIGNED;
	engineConfiguration->etbIo[1].directionPin2 = GPIO_UNASSIGNED;
	engineConfiguration->isHip9011Enabled = false;
	engineConfiguration->dizzySparkOutputPin = GPIO_UNASSIGNED;
	engineConfiguration->externalKnockSenseAdc = EFI_ADC_NONE;
	engineConfiguration->displayMode = DM_NONE;
	for (int i = 0; i < DIGIPOT_COUNT; i++) {
		engineConfiguration->digitalPotentiometerChipSelect[i] = GPIO_UNASSIGNED;
	}
	engineConfiguration->triggerSimulatorPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPins[2] = GPIO_UNASSIGNED;
	engineConfiguration->triggerSimulatorPinModes[1] = OM_DEFAULT;
	engineConfiguration->triggerSimulatorPinModes[2] = OM_DEFAULT;
	engineConfiguration->digitalPotentiometerSpiDevice = SPI_NONE;
	engineConfiguration->max31855spiDevice = SPI_NONE;
	engineConfiguration->warningLedPin = GPIO_UNASSIGNED;
	engineConfiguration->runningLedPin = GPIO_UNASSIGNED;
	engineConfiguration->useStepperIdle = false;
	engineConfiguration->idle.stepperDirectionPin = GPIO_UNASSIGNED;
	engineConfiguration->idle.stepperStepPin = GPIO_UNASSIGNED;
	engineConfiguration->stepperEnablePin = GPIO_UNASSIGNED;
	engineConfiguration->stepperEnablePinMode = OM_DEFAULT;
	engineConfiguration->injectionPins[8] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[9] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[10] = GPIO_UNASSIGNED;
	engineConfiguration->injectionPins[11] = GPIO_UNASSIGNED;
	engineConfiguration->tachOutputPin = GPIO_UNASSIGNED;
	engineConfiguration->HD44780_rs = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_e = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db4 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db5 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db6 = GPIO_UNASSIGNED;
		engineConfiguration->HD44780_db7 = GPIO_UNASSIGNED;
		engineConfiguration->o2heaterPin = GPIO_UNASSIGNED;
		engineConfiguration->digitalPotentiometerChipSelect[0] = GPIO_UNASSIGNED;
		engineConfiguration->digitalPotentiometerChipSelect[1] = GPIO_UNASSIGNED;
		engineConfiguration->digitalPotentiometerChipSelect[2] = GPIO_UNASSIGNED;
}
void setSdCardConfigurationOverrides(void) {
}

void setAdcChannelOverrides(void) {
}

