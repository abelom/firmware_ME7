/**
 * @file    engine_controller.cpp
 * @brief   Controllers package entry point code
 *
 *
 *
 * @date Feb 7, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 * This file is part of rusEfi - see http://rusefi.com
 *
 * rusEfi is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * rusEfi is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "os_access.h"
#include "trigger_central.h"
#include "engine_controller.h"
#include "fsio_core.h"
#include "fsio_impl.h"
#include "idle_thread.h"
#include "advance_map.h"
#include "rpm_calculator.h"
#include "main_trigger_callback.h"
#include "io_pins.h"
#include "flash_main.h"
#include "bench_test.h"
#include "os_util.h"
#include "engine_math.h"
#include "allsensors.h"
#include "electronic_throttle.h"
#include "map_averaging.h"
#include "malfunction_central.h"
#include "malfunction_indicator.h"
#include "speed_density.h"
#include "local_version_holder.h"
#include "alternator_controller.h"
#include "fuel_math.h"
#include "settings.h"
#include "aux_pid.h"
#include "spark_logic.h"
#include "aux_valves.h"
#include "accelerometer.h"
#include "counter64.h"
#include "perf_trace.h"
#include "boost_control.h"
#include "vvt_control.h"
#include "gp_pwm.h"
#include "launch_control.h"
#include "tachometer.h"

#if EFI_SENSOR_CHART
#include "sensor_chart.h"
#endif /* EFI_SENSOR_CHART */

#if EFI_TUNER_STUDIO
#include "tunerstudio.h"
#endif /* EFI_TUNER_STUDIO */

#if EFI_LOGIC_ANALYZER
#include "logic_analyzer.h"
#endif /* EFI_LOGIC_ANALYZER */

#if HAL_USE_ADC
#include "AdcConfiguration.h"
#endif /* HAL_USE_ADC */

#if defined(EFI_BOOTLOADER_INCLUDE_CODE)
#include "bootloader/bootloader.h"
#endif /* EFI_BOOTLOADER_INCLUDE_CODE */

#include "periodic_task.h"


#if ! EFI_UNIT_TEST
#include "init.h"
#endif /* EFI_UNIT_TEST */

#include "adc_inputs.h"
#include "pwm_generator_logic.h"

#if EFI_PROD_CODE
#include "pwm_tester.h"
#include "lcd_controller.h"
#include "pin_repository.h"
#endif /* EFI_PROD_CODE */

#if EFI_CJ125
#include "cj125.h"
#endif /* EFI_CJ125 */

EXTERN_ENGINE;

#if !EFI_UNIT_TEST

static LoggingWithStorage logger("Engine Controller");

/**
 * todo: this should probably become 'static', i.e. private, and propagated around explicitly?
 */
Engine ___engine CCM_OPTIONAL;
Engine * engine = &___engine;

#endif /* EFI_UNIT_TEST */


void initDataStructures(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_ENGINE_CONTROL
	initFuelMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	initTimingMap(PASS_ENGINE_PARAMETER_SIGNATURE);
	initSpeedDensity(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif // EFI_ENGINE_CONTROL
}

static void mostCommonInitEngineController(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if !EFI_UNIT_TEST
	// This is tested independently - don't configure sensors for tests.
	// This lets us selectively mock them for each test.
	initNewSensors(sharedLogger);
#endif /* EFI_UNIT_TEST */

	initSensors(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);

	initAccelEnrichment(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);

#if EFI_FSIO
	initFsioImpl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_FSIO */

#if EFI_IDLE_CONTROL
	startIdleThread(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_IDLE_CONTROL */

#if EFI_ELECTRONIC_THROTTLE_BODY
	initElectronicThrottle(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */

#if EFI_MAP_AVERAGING
	if (engineConfiguration->isMapAveragingEnabled) {
		initMapAveraging(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
	}
#endif /* EFI_MAP_AVERAGING */

#if EFI_VVT_CONTROL
	initVvtCtrl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_VVT_CONTROL */


#if EFI_BOOST_CONTROL
	initBoostCtrl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_BOOST_CONTROL */

#if EFI_LAUNCH_CONTROL
	initLaunchControl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
	initAntiLag(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif

}

#if EFI_ENABLE_MOCK_ADC

static void initMockVoltage(void) {
#if EFI_SIMULATOR
	setMockCltVoltage(2);
	setMockIatVoltage(2);
#endif /* EFI_SIMULATOR */
}

#endif /* EFI_ENABLE_MOCK_ADC */


#if !EFI_UNIT_TEST

static void doPeriodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE);

class PeriodicFastController : public PeriodicTimerController {
	void PeriodicTask() override {
		engine->periodicFastCallback();
	}

	int getPeriodMs() override {
		return FAST_CALLBACK_PERIOD_MS;
	}
};

class PeriodicSlowController : public PeriodicTimerController {
	void PeriodicTask() override {
		doPeriodicSlowCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	int getPeriodMs() override {
		// we need at least protection from zero value while resetting configuration
		int periodMs = maxI(50, CONFIG(generalPeriodicThreadPeriodMs));
		return periodMs;
	}
};

static PeriodicFastController fastController;
static PeriodicSlowController slowController;

class EngineStateBlinkingTask : public PeriodicTimerController {
	int getPeriodMs() override {
		return 50;
	}

	void PeriodicTask() override {
		counter++;
#if EFI_SHAFT_POSITION_INPUT
		bool is_running = ENGINE(rpmCalculator).isRunning(PASS_ENGINE_PARAMETER_SIGNATURE);
#else
		bool is_running = false;
#endif /* EFI_SHAFT_POSITION_INPUT */

		if (is_running) {
			// blink in running mode
			enginePins.runningLedPin.setValue(counter % 2);
		} else {
			int is_cranking = ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE);
			enginePins.runningLedPin.setValue(is_cranking);
		}
	}
private:
	int counter = 0;
};

static EngineStateBlinkingTask engineStateBlinkingTask;

/**
 * number of SysClock ticks in one ms
 */
#define TICKS_IN_MS  (CH_CFG_ST_FREQUENCY / 1000)

// todo: this overflows pretty fast!
efitimems_t currentTimeMillis(void) {
	// todo: migrate to getTimeNowUs? or not?
	return chVTGetSystemTimeX() / TICKS_IN_MS;
}

// todo: this overflows pretty fast!
efitimesec_t getTimeNowSeconds(void) {
	return currentTimeMillis() / 1000;
}

static void resetAccel(void) {
	engine->engineLoadAccelEnrichment.resetAE();
	engine->tpsAccelEnrichment.resetAE();

	for (unsigned int i = 0; i < sizeof(engine->wallFuel) / sizeof(engine->wallFuel[0]); i++)
	{
		engine->wallFuel[i].resetWF();
	}
}

static void doPeriodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT
	efiAssertVoid(CUSTOM_ERR_6661, getCurrentRemainingStack() > 64, "lowStckOnEv");
#if EFI_PROD_CODE
	touchTimeCounter();

	slowStartStopButtonCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_PROD_CODE */

	efitick_t nowNt = getTimeNowNt();

	if (nowNt - engine->triggerCentral.vvtSyncTimeNt >= NT_PER_SECOND) {
		// loss of VVT sync
		engine->triggerCentral.vvtSyncTimeNt = 0;
	}


	/**
	 * Update engine RPM state if needed (check timeouts).
	 */
	bool isSpinning = engine->rpmCalculator.checkIfSpinning(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	if (!isSpinning) {
		engine->rpmCalculator.setStopSpinning(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	if (ENGINE(directSelfStimulation) || engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		/**
		 * rusEfi usually runs on hardware which halts execution while writing to internal flash, so we
		 * postpone writes to until engine is stopped. Writes in case of self-stimulation are fine.
		 *
		 * todo: allow writing if 2nd bank of flash is used
		 */
#if EFI_INTERNAL_FLASH
		writeToFlashIfPending();
#endif /* EFI_INTERNAL_FLASH */
		resetAccel();
	}


	if (!engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		updatePrimeInjectionPulseState(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

	if (engine->versionForConfigurationListeners.isOld(engine->getGlobalConfigurationVersion())) {
		updateAccelParameters();
	}

	engine->periodicSlowCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT */
}

void initPeriodicEvents(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	slowController.Start();
	fastController.Start();
}

char * getPinNameByAdcChannel(const char *msg, adc_channel_e hwChannel, char *buffer) {
#if HAL_USE_ADC
	if (hwChannel == EFI_ADC_NONE) {
		strcpy(buffer, "NONE");
	} else {
		strcpy((char*) buffer, portname(getAdcChannelPort(msg, hwChannel)));
		itoa10(&buffer[2], getAdcChannelPin(hwChannel));
	}
#else
	strcpy(buffer, "NONE");
#endif /* HAL_USE_ADC */
	return (char*) buffer;
}

static char pinNameBuffer[16];

#if HAL_USE_ADC
extern AdcDevice fastAdc;
#endif /* HAL_USE_ADC */

static void printAnalogChannelInfoExt(const char *name, adc_channel_e hwChannel, float adcVoltage,
		float dividerCoeff) {
#if HAL_USE_ADC
	if (hwChannel == EFI_ADC_NONE) {
		scheduleMsg(&logger, "ADC is not assigned for %s", name);
		return;
	}

	if (fastAdc.isHwUsed(hwChannel)) {
		scheduleMsg(&logger, "fast enabled=%s", boolToString(CONFIG(isFastAdcEnabled)));
	}

	float voltage = adcVoltage * dividerCoeff;
	scheduleMsg(&logger, "%s ADC%d %s %s adc=%.2f/input=%.2fv/divider=%.2f", name, hwChannel, getAdcMode(hwChannel),
			getPinNameByAdcChannel(name, hwChannel, pinNameBuffer), adcVoltage, voltage, dividerCoeff);
#endif /* HAL_USE_ADC */
}

static void printAnalogChannelInfo(const char *name, adc_channel_e hwChannel) {
#if HAL_USE_ADC
	printAnalogChannelInfoExt(name, hwChannel, getVoltage("print", hwChannel PASS_ENGINE_PARAMETER_SUFFIX), engineConfiguration->analogInputDividerCoefficient);
#endif /* HAL_USE_ADC */
}

static void printAnalogInfo(void) {
	scheduleMsg(&logger, "analogInputDividerCoefficient: %.2f", engineConfiguration->analogInputDividerCoefficient);

	printAnalogChannelInfo("hip9011", engineConfiguration->hipOutputChannel);
	printAnalogChannelInfo("fuel gauge", engineConfiguration->fuelLevelSensor);
	printAnalogChannelInfo("TPS", engineConfiguration->tps1_1AdcChannel);
	printAnalogChannelInfo("TPS2", engineConfiguration->tps2_1AdcChannel);
	printAnalogChannelInfo("pPS", engineConfiguration->throttlePedalPositionAdcChannel);
	printAnalogChannelInfo("CLT", engineConfiguration->clt.adcChannel);
	printAnalogChannelInfo("IAT", engineConfiguration->iat.adcChannel);
	printAnalogChannelInfo("MAF", engineConfiguration->mafAdcChannel);
	for (int i = 0; i < FSIO_ANALOG_INPUT_COUNT ; i++) {
		adc_channel_e ch = engineConfiguration->fsioAdc[i];
		printAnalogChannelInfo("fsio", ch);
	}

	printAnalogChannelInfo("AFR", engineConfiguration->afr.hwChannel);
	printAnalogChannelInfo("MAP", engineConfiguration->map.sensor.hwChannel);
	printAnalogChannelInfo("BARO", engineConfiguration->baroSensor.hwChannel);
	printAnalogChannelInfo("extKno", engineConfiguration->externalKnockSenseAdc);

	printAnalogChannelInfo("OilP", engineConfiguration->oilPressure.hwChannel);

	printAnalogChannelInfo("CJ UR", engineConfiguration->cj125ur);
	printAnalogChannelInfo("CJ UA", engineConfiguration->cj125ua);

	printAnalogChannelInfo("A/C sw", engineConfiguration->acSwitchAdc);
	printAnalogChannelInfo("HIP9011", engineConfiguration->hipOutputChannel);

	for (int i = 0; i < FSIO_ANALOG_INPUT_COUNT ; i++) {
		printAnalogChannelInfo("FSIO", engineConfiguration->fsioAdc[i]);
	}

	printAnalogChannelInfoExt("Vbatt", engineConfiguration->vbattAdcChannel, getVoltage("vbatt", engineConfiguration->vbattAdcChannel PASS_ENGINE_PARAMETER_SUFFIX),
			engineConfiguration->vbattDividerCoeff);
}

#define isOutOfBounds(offset) ((offset<0) || (offset) >= (int) sizeof(engine_configuration_s))

static void getShort(int offset) {
	if (isOutOfBounds(offset))
		return;
	uint16_t *ptr = (uint16_t *) (&((char *) engineConfiguration)[offset]);
	uint16_t value = *ptr;
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "short%s%d is %d", CONSOLE_DATA_PROTOCOL_TAG, offset, value);
}

static void getByte(int offset) {
	if (isOutOfBounds(offset))
		return;
	uint8_t *ptr = (uint8_t *) (&((char *) engineConfiguration)[offset]);
	uint8_t value = *ptr;
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "byte%s%d is %d", CONSOLE_DATA_PROTOCOL_TAG, offset, value);
}

static void onConfigurationChanged() {
#if EFI_TUNER_STUDIO
	// on start-up rusEfi would read from working copy of TS while
	// we have a lot of console commands which write into real copy of configuration directly
	// we have a bit of a mess here
	syncTunerStudioCopy();
#endif /* EFI_TUNER_STUDIO */
	incrementGlobalConfigurationVersion(PASS_ENGINE_PARAMETER_SIGNATURE);
}

static void setBit(const char *offsetStr, const char *bitStr, const char *valueStr) {
	int offset = atoi(offsetStr);
	if (absI(offset) == absI(ERROR_CODE)) {
		scheduleMsg(&logger, "invalid offset [%s]", offsetStr);
		return;
	}
	if (isOutOfBounds(offset)) {
		return;
	}
	int bit = atoi(bitStr);
	if (absI(bit) == absI(ERROR_CODE)) {
		scheduleMsg(&logger, "invalid bit [%s]", bitStr);
		return;
	}
	int value = atoi(valueStr);
	if (absI(value) == absI(ERROR_CODE)) {
		scheduleMsg(&logger, "invalid value [%s]", valueStr);
		return;
	}
	int *ptr = (int *) (&((char *) engineConfiguration)[offset]);
	*ptr ^= (-value ^ *ptr) & (1 << bit);
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "bit%s%d/%d is %d", CONSOLE_DATA_PROTOCOL_TAG, offset, bit, value);
	onConfigurationChanged();
}

static void setShort(const int offset, const int value) {
	if (isOutOfBounds(offset))
		return;
	uint16_t *ptr = (uint16_t *) (&((char *) engineConfiguration)[offset]);
	*ptr = (uint16_t) value;
	getShort(offset);
	onConfigurationChanged();
}

static void setByte(const int offset, const int value) {
	if (isOutOfBounds(offset))
		return;
	uint8_t *ptr = (uint8_t *) (&((char *) engineConfiguration)[offset]);
	*ptr = (uint8_t) value;
	getByte(offset);
	onConfigurationChanged();
}

static void getBit(int offset, int bit) {
	if (isOutOfBounds(offset))
		return;
	int *ptr = (int *) (&((char *) engineConfiguration)[offset]);
	int value = (*ptr >> bit) & 1;
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "bit%s%d/%d is %d", CONSOLE_DATA_PROTOCOL_TAG, offset, bit, value);
}

static void getInt(int offset) {
	if (isOutOfBounds(offset))
		return;
	int *ptr = (int *) (&((char *) engineConfiguration)[offset]);
	int value = *ptr;
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "int%s%d is %d", CONSOLE_DATA_PROTOCOL_TAG, offset, value);
}

static void setInt(const int offset, const int value) {
	if (isOutOfBounds(offset))
		return;
	int *ptr = (int *) (&((char *) engineConfiguration)[offset]);
	*ptr = value;
	getInt(offset);
	onConfigurationChanged();
}

static void getFloat(int offset) {
	if (isOutOfBounds(offset))
		return;
	float *ptr = (float *) (&((char *) engineConfiguration)[offset]);
	float value = *ptr;
	/**
	 * this response is part of rusEfi console API
	 */
	scheduleMsg(&logger, "float%s%d is %.5f", CONSOLE_DATA_PROTOCOL_TAG, offset, value);
}

static void setFloat(const char *offsetStr, const char *valueStr) {
	int offset = atoi(offsetStr);
	if (absI(offset) == absI(ERROR_CODE)) {
		scheduleMsg(&logger, "invalid offset [%s]", offsetStr);
		return;
	}
	if (isOutOfBounds(offset))
		return;
	float value = atoff(valueStr);
	if (cisnan(value)) {
		scheduleMsg(&logger, "invalid value [%s]", valueStr);
		return;
	}
	float *ptr = (float *) (&((char *) engineConfiguration)[offset]);
	*ptr = value;
	getFloat(offset);
	onConfigurationChanged();
}

static void initConfigActions(void) {
	addConsoleActionSS("set_float", (VoidCharPtrCharPtr) setFloat);
	addConsoleActionII("set_int", (VoidIntInt) setInt);
	addConsoleActionII("set_short", (VoidIntInt) setShort);
	addConsoleActionII("set_byte", (VoidIntInt) setByte);
	addConsoleActionSSS("set_bit", setBit);

	addConsoleActionI("get_float", getFloat);
	addConsoleActionI("get_int", getInt);
	addConsoleActionI("get_short", getShort);
	addConsoleActionI("get_byte", getByte);
	addConsoleActionII("get_bit", getBit);
}

// todo: move this logic somewhere else?
static void getKnockInfo(void) {
	adc_channel_e hwChannel = engineConfiguration->externalKnockSenseAdc;
	scheduleMsg(&logger, "externalKnockSenseAdc on ADC", getPinNameByAdcChannel("knock", hwChannel, pinNameBuffer));

	engine->printKnockState();
}
#endif /* EFI_UNIT_TEST */

// this method is used by real firmware and simulator and unit test
void commonInitEngineController(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	initInterpolation(sharedLogger);

#if EFI_SIMULATOR
	printf("commonInitEngineController\n");
#endif

#if !EFI_UNIT_TEST
	initConfigActions();
#endif /* EFI_UNIT_TEST */

#if EFI_ENGINE_CONTROL
	/**
	 * This has to go after 'enginePins.startPins()' in order to
	 * properly detect un-assigned output pins
	 */
	prepareShapes(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_PROD_CODE && EFI_ENGINE_CONTROL */


#if EFI_ENABLE_MOCK_ADC
	initMockVoltage();
#endif /* EFI_ENABLE_MOCK_ADC */

#if EFI_SENSOR_CHART
	initSensorChart();
#endif /* EFI_SENSOR_CHART */


#if EFI_TUNER_STUDIO
	if (engineConfiguration->isTunerStudioEnabled) {
		startTunerStudioConnectivity();
	}
#endif /* EFI_TUNER_STUDIO */

#if EFI_PROD_CODE || EFI_SIMULATOR
	initSettings();

	if (hasFirmwareError()) {
		return;
	}
#endif

#if !EFI_UNIT_TEST
	// This is tested independently - don't configure sensors for tests.
	// This lets us selectively mock them for each test.
	initNewSensors(sharedLogger);
#endif /* EFI_UNIT_TEST */

	initSensors(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);

	initAccelEnrichment(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);

#if EFI_FSIO
	initFsioImpl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_FSIO */


#if EFI_IDLE_CONTROL
	startIdleThread(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_IDLE_CONTROL */

#if EFI_ELECTRONIC_THROTTLE_BODY
	initElectronicThrottle(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_ELECTRONIC_THROTTLE_BODY */

#if EFI_MAP_AVERAGING
	if (engineConfiguration->isMapAveragingEnabled) {
		initMapAveraging(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
	}
#endif /* EFI_MAP_AVERAGING */

#if EFI_BOOST_CONTROL
	initBoostCtrl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_BOOST_CONTROL */

#if EFI_LAUNCH_CONTROL
	initLaunchControl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif

#if EFI_SHAFT_POSITION_INPUT
	/**
	 * there is an implicit dependency on the fact that 'tachometer' listener is the 1st listener - this case
	 * other listeners can access current RPM value
	 */
	initRpmCalculator(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_SHAFT_POSITION_INPUT */

#if (EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT) || EFI_SIMULATOR || EFI_UNIT_TEST
	if (CONFIG(isEngineControlEnabled)) {
		initAuxValves(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
		/**
		 * This method adds trigger listener which actually schedules ignition
		 */
		initSparkLogic(sharedLogger);
		initMainEventListener(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
	}
#endif /* EFI_ENGINE_CONTROL */

	initTachometer(PASS_ENGINE_PARAMETER_SIGNATURE);
}

#if !EFI_UNIT_TEST

void initEngineContoller(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	addConsoleAction("analoginfo", printAnalogInfo);

#if EFI_PROD_CODE && EFI_ENGINE_CONTROL
	enginePins.startPins();

	initBenchTest(sharedLogger);
#endif /* EFI_PROD_CODE && EFI_ENGINE_CONTROL */

	commonInitEngineController(sharedLogger);

#if EFI_LOGIC_ANALYZER
	if (engineConfiguration->isWaveAnalyzerEnabled) {
		initWaveAnalyzer(sharedLogger);
	}
#endif /* EFI_LOGIC_ANALYZER */

#if EFI_CJ125
	/**
	 * this uses SimplePwm which depends on scheduler, has to be initialized after scheduler
	 */
	initCJ125(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_CJ125 */


	// periodic events need to be initialized after fuel&spark pins to avoid a warning
	initPeriodicEvents(PASS_ENGINE_PARAMETER_SIGNATURE);

	if (hasFirmwareError()) {
		return;
	}

	engineStateBlinkingTask.Start();

#if EFI_PWM_TESTER
	initPwmTester();
#endif /* EFI_PWM_TESTER */

#if EFI_ALTERNATOR_CONTROL
	initAlternatorCtrl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_ALTERNATOR_CONTROL */

#if EFI_AUX_PID
	initAuxPid(sharedLogger);
#endif /* EFI_AUX_PID */

#if EFI_GP_PWM
	initGpPwmCtrl(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
#endif
#if EFI_MALFUNCTION_INDICATOR
	initMalfunctionIndicator();
#endif /* EFI_MALFUNCTION_INDICATOR */

	initEgoAveraging(PASS_ENGINE_PARAMETER_SIGNATURE);

	if (engineConfiguration->externalKnockSenseAdc != EFI_ADC_NONE) {
		addConsoleAction("knockinfo", getKnockInfo);
	}

#if EFI_PROD_CODE
	addConsoleAction("reset_accel", resetAccel);
#endif /* EFI_PROD_CODE */

#if EFI_HD44780_LCD
	initLcdController();
#endif /* EFI_HD44780_LCD */

}

// these two variables are here only to let us know how much RAM is available, also these
// help to notice when RAM usage goes up - if a code change adds to RAM usage these variables would fail
// linking process which is the way to raise the alarm
#ifndef RAM_UNUSED_SIZE
#define RAM_UNUSED_SIZE 16000
#endif
#ifndef CCM_UNUSED_SIZE
#define CCM_UNUSED_SIZE 2900
#endif
static char UNUSED_RAM_SIZE[RAM_UNUSED_SIZE];
static char UNUSED_CCM_SIZE[CCM_UNUSED_SIZE] CCM_OPTIONAL;

/**
 * See also VCS_VERSION
 */
int getRusEfiVersion(void) {
	if (UNUSED_RAM_SIZE[0] != 0)
		return 123; // this is here to make the compiler happy about the unused array
	if (UNUSED_CCM_SIZE[0] * 0 != 0)
		return 3211; // this is here to make the compiler happy about the unused array
#if defined(EFI_BOOTLOADER_INCLUDE_CODE)
	// make bootloader code happy too
	if (initBootloader() != 0)
		return 123;
#endif /* EFI_BOOTLOADER_INCLUDE_CODE */
	return 20200426;
}
#endif /* EFI_UNIT_TEST */
