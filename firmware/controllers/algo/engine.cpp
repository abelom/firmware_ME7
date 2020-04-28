/**
 * @file	engine.cpp
 *
 *
 * This might be a http://en.wikipedia.org/wiki/God_object but that's best way I can
 * express myself in C/C++. I am open for suggestions :)
 *
 * @date May 21, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "engine.h"
#include "allsensors.h"
#include "efi_gpio.h"
#include "trigger_central.h"
#include "fuel_math.h"
#include "engine_math.h"
#include "advance_map.h"
#include "speed_density.h"
#include "advance_map.h"
#include "os_util.h"
#include "settings.h"
#include "aux_valves.h"
#include "map_averaging.h"
#include "fsio_impl.h"
#include "perf_trace.h"
#include "sensor.h"

#if EFI_PROD_CODE
#include "bench_test.h"
#else
#define isRunningBenchTest() true
#endif /* EFI_PROD_CODE */

#if (BOARD_TLE8888_COUNT > 0)
#include "gpio/tle8888.h"
#endif

static TriggerState initState CCM_OPTIONAL;

LoggingWithStorage engineLogger("engine");

EXTERN_ENGINE;

#if EFI_ENGINE_SNIFFER
#include "engine_sniffer.h"
extern int waveChartUsedSize;
extern WaveChart waveChart;
#endif /* EFI_ENGINE_SNIFFER */

FsioState::FsioState() {
#if EFI_ENABLE_ENGINE_WARNING
	isEngineWarning = FALSE;
#endif
#if EFI_ENABLE_CRITICAL_ENGINE_STOP
	isCriticalEngineCondition = FALSE;
#endif
}

void Engine::resetEngineSnifferIfInTestMode() {
#if EFI_ENGINE_SNIFFER
	if (isTestMode) {
		// TODO: what is the exact reasoning for the exact engine sniffer pause time I wonder
		waveChart.pauseEngineSnifferUntilNt = getTimeNowNt() + MS2NT(300);
		waveChart.reset();
	}
#endif /* EFI_ENGINE_SNIFFER */
}

void Engine::initializeTriggerWaveform(Logging *logger DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT
	// we have a confusing threading model so some synchronization would not hurt
	bool alreadyLocked = lockAnyContext();

	TRIGGER_WAVEFORM(initializeTriggerWaveform(logger,
			engineConfiguration->ambiguousOperationMode,
			engineConfiguration->useOnlyRisingEdgeForTrigger, &engineConfiguration->trigger));

	if (TRIGGER_WAVEFORM(bothFrontsRequired) && engineConfiguration->useOnlyRisingEdgeForTrigger) {
#if EFI_PROD_CODE || EFI_SIMULATOR
		firmwareError(CUSTOM_ERR_BOTH_FRONTS_REQUIRED, "trigger: both fronts required");
#else
		warning(CUSTOM_ERR_BOTH_FRONTS_REQUIRED, "trigger: both fronts required");
#endif
	}


	if (!TRIGGER_WAVEFORM(shapeDefinitionError)) {
		/**
	 	 * this instance is used only to initialize 'this' TriggerWaveform instance
	 	 * #192 BUG real hardware trigger events could be coming even while we are initializing trigger
	 	 */
		initState.resetTriggerState();
		calculateTriggerSynchPoint(&ENGINE(triggerCentral.triggerShape),
				&initState PASS_ENGINE_PARAMETER_SUFFIX);

		if (engine->triggerCentral.triggerShape.getSize() == 0) {
			firmwareError(CUSTOM_ERR_TRIGGER_ZERO, "triggerShape size is zero");
		}
		engine->engineCycleEventCount = TRIGGER_WAVEFORM(getLength());
	}

	if (!alreadyLocked) {
		unlockAnyContext();
	}

	if (!TRIGGER_WAVEFORM(shapeDefinitionError)) {
		prepareOutputSignals(PASS_ENGINE_PARAMETER_SIGNATURE);
	}
#endif /* EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT */
}

static void cylinderCleanupControl(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_ENGINE_CONTROL
	bool newValue;
	if (engineConfiguration->isCylinderCleanupEnabled) {
		newValue = !engine->rpmCalculator.isRunning(PASS_ENGINE_PARAMETER_SIGNATURE) && Sensor::get(SensorType::DriverThrottleIntent).value_or(0) > CLEANUP_MODE_TPS;
	} else {
		newValue = false;
	}
	if (newValue != engine->isCylinderCleanupMode) {
		engine->isCylinderCleanupMode = newValue;
		scheduleMsg(&engineLogger, "isCylinderCleanupMode %s", boolToString(newValue));
	}
#endif
}

static efitick_t tle8888CrankingResetTime = 0;

void Engine::periodicSlowCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ScopePerf perf(PE::EnginePeriodicSlowCallback);
	
	watchdog();
	updateSlowSensors(PASS_ENGINE_PARAMETER_SIGNATURE);
	checkShutdown();

#if EFI_FSIO
	runFsio(PASS_ENGINE_PARAMETER_SIGNATURE);
#else
	runHardcodedFsio(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif /* EFI_FSIO */

	cylinderCleanupControl(PASS_ENGINE_PARAMETER_SIGNATURE);

#if (BOARD_TLE8888_COUNT > 0)
	if (CONFIG(useTLE8888_cranking_hack) && ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		efitick_t nowNt = getTimeNowNt();
		if (nowNt - tle8888CrankingResetTime > MS2NT(300)) {
			requestTLE8888initialization();
			// let's reset TLE8888 every 300ms while cranking since that's the best we can do to deal with undervoltage reset
			// PS: oh yes, it's a horrible design! Please suggest something better!
			tle8888CrankingResetTime = nowNt;
		}
	}
#endif

	slowCallBackWasInvoked = TRUE;
}


#if (BOARD_TLE8888_COUNT > 0)
extern float vBattForTle8888;
#endif /* BOARD_TLE8888_COUNT */

/**
 * We are executing these heavy (logarithm) methods from outside the trigger callbacks for performance reasons.
 * See also periodicFastCallback
 */
void Engine::updateSlowSensors(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_ENGINE_CONTROL
	int rpm = GET_RPM();
	isEngineChartEnabled = CONFIG(isEngineChartEnabled) && rpm < CONFIG(engineSnifferRpmThreshold);
	sensorChartMode = rpm < CONFIG(sensorSnifferRpmThreshold) ? CONFIG(sensorChartMode) : SC_OFF;

	engineState.updateSlowSensors(PASS_ENGINE_PARAMETER_SIGNATURE);

	// todo: move this logic somewhere to sensors folder?
	if (CONFIG(fuelLevelSensor) != EFI_ADC_NONE) {
		float fuelLevelVoltage = getVoltageDivided("fuel", engineConfiguration->fuelLevelSensor PASS_ENGINE_PARAMETER_SUFFIX);
		sensors.fuelTankLevel = interpolateMsg("fgauge", CONFIG(fuelLevelEmptyTankVoltage), 0,
				CONFIG(fuelLevelFullTankVoltage), 100,
				fuelLevelVoltage);
	}
	sensors.vBatt = hasVBatt(PASS_ENGINE_PARAMETER_SIGNATURE) ? getVBatt(PASS_ENGINE_PARAMETER_SIGNATURE) : 12;

#if (BOARD_TLE8888_COUNT > 0)
	// nasty value injection into C driver which would not be able to access Engine class
	vBattForTle8888 = sensors.vBatt;
#endif /* BOARD_TLE8888_COUNT */

	engineState.running.injectorLag = getInjectorLag(sensors.vBatt PASS_ENGINE_PARAMETER_SUFFIX);
#endif
}

void Engine::onTriggerSignalEvent(efitick_t nowNt) {
	isSpinning = true;
	lastTriggerToothEventTimeNt = nowNt;
}

Engine::Engine() {
	reset();
}

Engine::Engine(persistent_config_s *config) {
	setConfig(config);
	reset();
}

/**
 * @see scheduleStopEngine()
 * @return true if there is a reason to stop engine
 */
bool Engine::needToStopEngine(efitick_t nowNt) const {
	return stopEngineRequestTimeNt != 0 &&
			nowNt - stopEngineRequestTimeNt	< 3 * NT_PER_SECOND;
}

int Engine::getGlobalConfigurationVersion(void) const {
	return globalConfigurationVersion;
}

void Engine::reset() {
	/**
	 * it's important for fixAngle() that engineCycle field never has zero
	 */
	engineCycle = getEngineCycle(FOUR_STROKE_CRANK_SENSOR);
	memset(&ignitionPin, 0, sizeof(ignitionPin));
}


/**
 * Here we have a bunch of stuff which should invoked after configuration change
 * so that we can prepare some helper structures
 */
void Engine::preCalculate(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if HAL_USE_ADC
	adcToVoltageInputDividerCoefficient = adcToVolts(1) * engineConfiguration->analogInputDividerCoefficient;
#else
	adcToVoltageInputDividerCoefficient = engineConfigurationPtr->analogInputDividerCoefficient;
#endif
}

#if EFI_SHAFT_POSITION_INPUT
void Engine::OnTriggerStateDecodingError() {
	Engine *engine = this;
	EXPAND_Engine;

	warning(CUSTOM_SYNC_COUNT_MISMATCH, "trigger not happy current %d/%d/%d expected %d/%d/%d",
			triggerCentral.triggerState.currentCycle.eventCount[0],
			triggerCentral.triggerState.currentCycle.eventCount[1],
			triggerCentral.triggerState.currentCycle.eventCount[2],
			TRIGGER_WAVEFORM(expectedEventCount[0]),
			TRIGGER_WAVEFORM(expectedEventCount[1]),
			TRIGGER_WAVEFORM(expectedEventCount[2]));
	triggerCentral.triggerState.setTriggerErrorState();


	triggerCentral.triggerState.totalTriggerErrorCounter++;
	if (CONFIG(verboseTriggerSynchDetails) || (triggerCentral.triggerState.someSortOfTriggerError && !CONFIG(silentTriggerError))) {
#if EFI_PROD_CODE
		scheduleMsg(&engineLogger, "error: synchronizationPoint @ index %d expected %d/%d/%d got %d/%d/%d",
				triggerCentral.triggerState.currentCycle.current_index,
				TRIGGER_WAVEFORM(expectedEventCount[0]),
				TRIGGER_WAVEFORM(expectedEventCount[1]),
				TRIGGER_WAVEFORM(expectedEventCount[2]),
				triggerCentral.triggerState.currentCycle.eventCount[0],
				triggerCentral.triggerState.currentCycle.eventCount[1],
				triggerCentral.triggerState.currentCycle.eventCount[2]);
#endif /* EFI_PROD_CODE */
	}

}

void Engine::OnTriggerStateProperState(efitick_t nowNt) {
	Engine *engine = this;
	EXPAND_Engine;

	triggerCentral.triggerState.runtimeStatistics(nowNt PASS_ENGINE_PARAMETER_SUFFIX);

	rpmCalculator.setSpinningUp(nowNt PASS_ENGINE_PARAMETER_SUFFIX);
}

void Engine::OnTriggerSynchronizationLost() {
	Engine *engine = this;
	EXPAND_Engine;

	// Needed for early instant-RPM detection
	engine->rpmCalculator.setStopSpinning(PASS_ENGINE_PARAMETER_SIGNATURE);
}

void Engine::OnTriggerInvalidIndex(int currentIndex) {
	Engine *engine = this;
	EXPAND_Engine;
	// let's not show a warning if we are just starting to spin
	if (GET_RPM_VALUE != 0) {
		warning(CUSTOM_SYNC_ERROR, "sync error: index #%d above total size %d", currentIndex, triggerCentral.triggerShape.getSize());
		triggerCentral.triggerState.setTriggerErrorState();
	}
}

void Engine::OnTriggerSyncronization(bool wasSynchronized) {
	// We only care about trigger shape once we have synchronized trigger. Anything could happen
	// during first revolution and it's fine
	if (wasSynchronized) {
		Engine *engine = this;
		EXPAND_Engine;

		/**
	 	 * We can check if things are fine by comparing the number of events in a cycle with the expected number of event.
	 	 */
		bool isDecodingError = triggerCentral.triggerState.validateEventCounters(&triggerCentral.triggerShape);

		enginePins.triggerDecoderErrorPin.setValue(isDecodingError);

		// 'triggerStateListener is not null' means we are running a real engine and now just preparing trigger shape
		// that's a bit of a hack, a sweet OOP solution would be a real callback or at least 'needDecodingErrorLogic' method?
		if (isDecodingError) {
			OnTriggerStateDecodingError();
		}

		engine->triggerErrorDetection.add(isDecodingError);

		if (isTriggerDecoderError(PASS_ENGINE_PARAMETER_SIGNATURE)) {
			warning(CUSTOM_OBD_TRG_DECODING, "trigger decoding issue. expected %d/%d/%d got %d/%d/%d",
					TRIGGER_WAVEFORM(expectedEventCount[0]), TRIGGER_WAVEFORM(expectedEventCount[1]),
					TRIGGER_WAVEFORM(expectedEventCount[2]),
					triggerCentral.triggerState.currentCycle.eventCount[0],
					triggerCentral.triggerState.currentCycle.eventCount[1],
					triggerCentral.triggerState.currentCycle.eventCount[2]);
		}
	}

}
#endif

void Engine::setConfig(persistent_config_s *config) {
	this->config = config;
	engineConfigurationPtr = &config->engineConfiguration;
	memset(config, 0, sizeof(persistent_config_s));
}

void Engine::printKnockState(void) {
	scheduleMsg(&engineLogger, "knock now=%s/ever=%s", boolToString(knockNow), boolToString(knockEver));
}

void Engine::knockLogic(float knockVolts DECLARE_ENGINE_PARAMETER_SUFFIX) {
	this->knockVolts = knockVolts;
    knockNow = knockVolts > engineConfiguration->knockVThreshold;
    /**
     * KnockCount is directly proportional to the degrees of ignition
     * advance removed
     * ex: degrees to subtract = knockCount;
     */

    /**
     * TODO use knockLevel as a factor for amount of ignition advance
     * to remove
     * Perhaps allow the user to set a multiplier
     * ex: degrees to subtract = knockCount + (knockLevel * X)
     * X = user configurable multiplier
     */
    if (knockNow) {
        knockEver = true;
        timeOfLastKnockEvent = getTimeNowUs();
        if (knockCount < engineConfiguration->maxKnockSubDeg)
            knockCount++;
    } else if (knockCount >= 1) {
        knockCount--;
	} else {
        knockCount = 0;
    }
}

void Engine::watchdog() {
#if EFI_ENGINE_CONTROL
	if (isRunningPwmTest)
		return;
	if (!isSpinning) {
		if (!isRunningBenchTest() && enginePins.stopPins()) {
			// todo: make this a firmwareError assuming functional tests would run
			warning(CUSTOM_ERR_2ND_WATCHDOG, "Some pins were turned off by 2nd pass watchdog");
		}
		return;
	}
	efitick_t nowNt = getTimeNowNt();
// note that we are ignoring the number of tooth here - we
// check for duration between tooth as if we only have one tooth per revolution which is not the case
#define REVOLUTION_TIME_HIGH_THRESHOLD (60 * 1000000LL / RPM_LOW_THRESHOLD)
	/**
	 * todo: better watch dog implementation should be implemented - see
	 * http://sourceforge.net/p/rusefi/tickets/96/
	 *
	 * note that the result of this subtraction could be negative, that would happen if
	 * we have a trigger event between the time we've invoked 'getTimeNow' and here
	 */
	efitick_t timeSinceLastTriggerEvent = nowNt - lastTriggerToothEventTimeNt;
	if (timeSinceLastTriggerEvent < US2NT(REVOLUTION_TIME_HIGH_THRESHOLD)) {
		return;
	}
	isSpinning = false;
	ignitionEvents.isReady = false;
#if EFI_PROD_CODE || EFI_SIMULATOR
	scheduleMsg(&engineLogger, "engine has STOPPED");
	scheduleMsg(&engineLogger, "templog engine has STOPPED [%x][%x] [%x][%x] %d",
			(int)(nowNt >> 32), (int)nowNt,
			(int)(lastTriggerToothEventTimeNt >> 32), (int)lastTriggerToothEventTimeNt,
			(int)timeSinceLastTriggerEvent
			);
	triggerInfo();
#endif

	enginePins.stopPins();
#endif
}

void Engine::checkShutdown() {
#if EFI_MAIN_RELAY_CONTROL
	int rpm = rpmCalculator.getRpm();

	/**
	 * Something is weird here: "below 5.0 volts on battery" what is it about? Is this about
	 * Frankenso powering everything while driver has already turned ignition off? or what is this condition about?
	 */
	const float vBattThreshold = 5.0f;
	if (isValidRpm(rpm) && sensors.vBatt < vBattThreshold && stopEngineRequestTimeNt == 0) {
		scheduleStopEngine();
		// todo: add stepper motor parking
	}
#endif /* EFI_MAIN_RELAY_CONTROL */
}

bool Engine::isInShutdownMode() const {
#if EFI_MAIN_RELAY_CONTROL
	if (stopEngineRequestTimeNt == 0)	// the shutdown procedure is not started
		return false;
	
	const efitick_t engineStopWaitTimeoutNt = 5LL * 1000000LL;
	// The engine is still spinning! Give it some time to stop (but wait no more than 5 secs)
	if (isSpinning && (getTimeNowNt() - stopEngineRequestTimeNt) < US2NT(engineStopWaitTimeoutNt))
		return true;
	// todo: add checks for stepper motor parking
#endif /* EFI_MAIN_RELAY_CONTROL */
	return false;
}

injection_mode_e Engine::getCurrentInjectionMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return rpmCalculator.isCranking(PASS_ENGINE_PARAMETER_SIGNATURE) ? CONFIG(crankingInjectionMode) : CONFIG(injectionMode);
}

// see also in TunerStudio project '[doesTriggerImplyOperationMode] tag
static bool doesTriggerImplyOperationMode(trigger_type_e type) {
	return type != TT_TOOTHED_WHEEL
			&& type != TT_ONE
			&& type != TT_ONE_PLUS_ONE
			&& type != TT_3_1_CAM
			&& type != TT_TOOTHED_WHEEL_60_2
			&& type != TT_TOOTHED_WHEEL_36_1;
}

operation_mode_e Engine::getOperationMode(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	/**
	 * here we ignore user-provided setting for well known triggers.
	 * For instance for Miata NA, there is no reason to allow user to set FOUR_STROKE_CRANK_SENSOR
	 */
	return doesTriggerImplyOperationMode(engineConfiguration->trigger.type) ? triggerCentral.triggerShape.getOperationMode() : engineConfiguration->ambiguousOperationMode;
}

/**
 * The idea of this method is to execute all heavy calculations in a lower-priority thread,
 * so that trigger event handler/IO scheduler tasks are faster.
 */
void Engine::periodicFastCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	ScopePerf pc(PE::EnginePeriodicFastCallback);

#if EFI_MAP_AVERAGING
	refreshMapAveragingPreCalc(PASS_ENGINE_PARAMETER_SIGNATURE);
#endif

	engineState.periodicFastCallback(PASS_ENGINE_PARAMETER_SIGNATURE);

#if EFI_ENGINE_CONTROL
	int rpm = GET_RPM();

	ENGINE(injectionDuration) = getInjectionDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX);
#endif
}

void doScheduleStopEngine(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	engine->stopEngineRequestTimeNt = getTimeNowNt();
	// let's close injectors or else if these happen to be open right now
	enginePins.stopPins();
}

void action_s::execute() {
	efiAssertVoid(CUSTOM_ERR_ASSERT, callback != NULL, "callback==null1");
	callback(param);
}

schfunc_t action_s::getCallback() const {
	return callback;
}

void * action_s::getArgument() const {
	return param;
}

