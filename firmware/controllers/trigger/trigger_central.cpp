/*
 * @file	trigger_central.cpp
 * Here we have a bunch of higher-level methods which are not directly related to actual signal decoding
 *
 * @date Feb 23, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "os_access.h"

#include "trigger_central.h"
#include "trigger_decoder.h"
#include "main_trigger_callback.h"
#include "engine_configuration.h"
#include "listener_array.h"
#include "data_buffer.h"
#include "pwm_generator_logic.h"
#include "tooth_logger.h"

#include "settings.h"
#include "engine_math.h"
#include "local_version_holder.h"
#include "trigger_simulator.h"

#include "rpm_calculator.h"
#include "tooth_logger.h"
#include "perf_trace.h"

#if EFI_PROD_CODE
#include "pin_repository.h"
#endif /* EFI_PROD_CODE */

#if EFI_TUNER_STUDIO
#include "tunerstudio.h"
#endif /* EFI_TUNER_STUDIO */

#if EFI_ENGINE_SNIFFER
#include "engine_sniffer.h"
WaveChart waveChart;
#endif /* EFI_ENGINE_SNIFFER */

trigger_central_s::trigger_central_s() : hwEventCounters() {

	static_assert(TRIGGER_TYPE_60_2 == TT_TOOTHED_WHEEL_60_2, "One we will have one source of this magic constant");
	static_assert(TRIGGER_TYPE_36_1 == TT_TOOTHED_WHEEL_36_1, "One we will have one source of this magic constant");



}

TriggerCentral::TriggerCentral() : trigger_central_s() {

	clearCallbacks(&triggerListeneres);
	triggerState.resetTriggerState();
	noiseFilter.resetAccumSignalData();
}

void TriggerNoiseFilter::resetAccumSignalData() {
	memset(lastSignalTimes, 0xff, sizeof(lastSignalTimes));	// = -1
	memset(accumSignalPeriods, 0, sizeof(accumSignalPeriods));
	memset(accumSignalPrevPeriods, 0, sizeof(accumSignalPrevPeriods));
}

int TriggerCentral::getHwEventCounter(int index) const {
	return hwEventCounters[index];
}

#if EFI_SHAFT_POSITION_INPUT

EXTERN_ENGINE;

static Logging *logger;

void TriggerCentral::addEventListener(ShaftPositionListener listener, const char *name, Engine *engine) {
	print("registerCkpListener: %s\r\n", name);
	triggerListeneres.registerCallback((VoidInt)(void*)listener, engine);
}

angle_t TriggerCentral::getVVTPosition() {
	return vvtPosition;
}

/**
 * @brief Adds a trigger event listener
 *
 * Trigger event listener would be invoked on each trigger event. For example, for a 60/2 wheel
 * that would be 116 events: 58 SHAFT_PRIMARY_RISING and 58 SHAFT_PRIMARY_FALLING events.
 */
void addTriggerEventListener(ShaftPositionListener listener, const char *name, Engine *engine) {
	engine->triggerCentral.addEventListener(listener, name, engine);
}

#define miataNb2VVTRatioFrom (8.50 * 0.75)
#define miataNb2VVTRatioTo (14)
#define miataNbIndex (0)

void hwHandleVvtCamSignal(trigger_value_e front, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	TriggerCentral *tc = &engine->triggerCentral;
	if (front == TV_RISE) {
		tc->vvtEventRiseCounter++;
	} else {
		tc->vvtEventFallCounter++;
	}

	if (!CONFIG(displayLogicLevelsInEngineSniffer)) {
		addEngineSnifferEvent(PROTOCOL_VVT_NAME, front == TV_RISE ? PROTOCOL_ES_UP : PROTOCOL_ES_DOWN);
	}


	if (CONFIG(vvtCamSensorUseRise) ^ (front != TV_FALL)) {
		return;
	}

	if (CONFIG(displayLogicLevelsInEngineSniffer)) {
		if (CONFIG(vvtCamSensorUseRise)) {
			// todo: unify TS composite logger code with console Engine Sniffer
			// todo: better API to reduce copy/paste?
#if EFI_TOOTH_LOGGER
			LogTriggerTooth(SHAFT_SECONDARY_RISING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
			LogTriggerTooth(SHAFT_SECONDARY_FALLING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_TOOTH_LOGGER */
			addEngineSnifferEvent(PROTOCOL_VVT_NAME, PROTOCOL_ES_UP);
			addEngineSnifferEvent(PROTOCOL_VVT_NAME, PROTOCOL_ES_DOWN);
		} else {
#if EFI_TOOTH_LOGGER
			LogTriggerTooth(SHAFT_SECONDARY_FALLING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
			LogTriggerTooth(SHAFT_SECONDARY_RISING, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
#endif /* EFI_TOOTH_LOGGER */

			addEngineSnifferEvent(PROTOCOL_VVT_NAME, PROTOCOL_ES_DOWN);
			addEngineSnifferEvent(PROTOCOL_VVT_NAME, PROTOCOL_ES_UP);
		}
	}

	floatus_t oneDegreeUs = engine->rpmCalculator.oneDegreeUs;
	if (cisnan(oneDegreeUs)) {
		// todo: this code branch is slowing NB2 cranking since we require RPM sync for VVT sync!
		// todo: smarter code
		//
		// we are here if we are getting VVT position signals while engine is not running
		// for example if crank position sensor is broken :)
		return;
	}

	tc->vvtCamCounter++;

	if (engineConfiguration->vvtMode == MIATA_NB2) {
		uint32_t currentDuration = nowNt - tc->previousVvtCamTime;
		float ratio = ((float) currentDuration) / tc->previousVvtCamDuration;


		tc->previousVvtCamDuration = currentDuration;
		tc->previousVvtCamTime = nowNt;

		if (engineConfiguration->verboseTriggerSynchDetails) {
			scheduleMsg(logger, "vvt ratio %.2f", ratio);
		}
		if (ratio < miataNb2VVTRatioFrom || ratio > miataNb2VVTRatioTo) {
			return;
		}
		if (engineConfiguration->verboseTriggerSynchDetails) {
			scheduleMsg(logger, "looks good: vvt ratio %.2f", ratio);
		}
		if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
			tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
		}
	}

	tc->vvtSyncTimeNt = nowNt;

	efitick_t offsetNt = nowNt - tc->timeAtVirtualZeroNt;

	angle_t vvtPosition = NT2US(offsetNt) / oneDegreeUs;

	// convert engine cycle angle into trigger cycle angle
	vvtPosition -= tdcPosition();
	fixAngle(vvtPosition, "vvtPosition", CUSTOM_ERR_6558);

	tc->vvtPosition = (engineConfiguration->vvtDisplayInverted ? -vvtPosition : vvtPosition) + engineConfiguration->vvtOffset;

	if (engineConfiguration->vvtMode == VVT_FIRST_HALF) {
		bool isEven = tc->triggerState.isEvenRevolution();
		if (!isEven) {
			/**
			 * we are here if we've detected the cam sensor within the wrong crank phase
			 * let's increase the trigger event counter, that would adjust the state of
			 * virtual crank-based trigger
			 */
			tc->triggerState.incrementTotalEventCounter();
			if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
				tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
			}
		}
	} else if (engineConfiguration->vvtMode == VVT_SECOND_HALF) {
		bool isEven = tc->triggerState.isEvenRevolution();
		if (isEven) {
			// see above comment
			tc->triggerState.incrementTotalEventCounter();
			if (engineConfiguration->debugMode == DBG_VVT) {
#if EFI_TUNER_STUDIO
				tsOutputChannels.debugIntField1++;
#endif /* EFI_TUNER_STUDIO */
			}
		}

	} else if (engineConfiguration->vvtMode == MIATA_NB2) {
		/**
		 * NB2 is a symmetrical crank, there are four phases total
		 */
		while (tc->triggerState.getTotalRevolutionCounter() % 4 != miataNbIndex) {
			tc->triggerState.incrementTotalEventCounter();
		}
	}

}

#if EFI_PROD_CODE || EFI_SIMULATOR

int triggerReentraint = 0;
int maxTriggerReentraint = 0;
uint32_t triggerDuration;
uint32_t triggerMaxDuration = 0;

void hwHandleShaftSignal(trigger_event_e signal, efitick_t timestamp) {
	ScopePerf perf(PE::HandleShaftSignal, static_cast<uint8_t>(signal));

#if EFI_TOOTH_LOGGER
	// Log to the Tunerstudio tooth logger
	// We want to do this before anything else as we
	// actually want to capture any noise/jitter that may be occurring

	bool logLogicState = CONFIG(displayLogicLevelsInEngineSniffer && engineConfiguration->useOnlyRisingEdgeForTrigger);

	if (!logLogicState) {
		// we log physical state even if displayLogicLevelsInEngineSniffer if both fronts are used by decoder
		LogTriggerTooth(signal, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
	}

#endif /* EFI_TOOTH_LOGGER */

	// for effective noise filtering, we need both signal edges, 
	// so we pass them to handleShaftSignal() and defer this test
	if (!CONFIG(useNoiselessTriggerDecoder)) {
		if (!isUsefulSignal(signal PASS_CONFIG_PARAMETER_SUFFIX)) {
			return;
		}
	}

#if EFI_TOOTH_LOGGER
	if (logLogicState) {
		LogTriggerTooth(signal, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
		if (signal == SHAFT_PRIMARY_RISING) {
			LogTriggerTooth(SHAFT_PRIMARY_FALLING, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
		} else {
			LogTriggerTooth(SHAFT_SECONDARY_FALLING, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
		}
	}
#endif /* EFI_TOOTH_LOGGER */

	uint32_t triggerHandlerEntryTime = getTimeNowLowerNt();
	if (triggerReentraint > maxTriggerReentraint)
		maxTriggerReentraint = triggerReentraint;
	triggerReentraint++;

	efiAssertVoid(CUSTOM_ERR_6636, getCurrentRemainingStack() > 128, "lowstck#8");
	engine->triggerCentral.handleShaftSignal(signal, timestamp PASS_ENGINE_PARAMETER_SUFFIX);

	triggerReentraint--;
	triggerDuration = getTimeNowLowerNt() - triggerHandlerEntryTime;
	if (triggerDuration > triggerMaxDuration)
		triggerMaxDuration = triggerDuration;
}
#endif /* EFI_PROD_CODE */

void TriggerCentral::resetCounters() {
	memset(hwEventCounters, 0, sizeof(hwEventCounters));
}

static char shaft_signal_msg_index[15];

static const bool isUpEvent[6] = { false, true, false, true, false, true };
static const char *eventId[6] = { PROTOCOL_CRANK1, PROTOCOL_CRANK1, PROTOCOL_CRANK2, PROTOCOL_CRANK2, PROTOCOL_CRANK3, PROTOCOL_CRANK3 };

static ALWAYS_INLINE void reportEventToWaveChart(trigger_event_e ckpSignalType, int index DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (!ENGINE(isEngineChartEnabled)) { // this is here just as a shortcut so that we avoid engine sniffer as soon as possible
		return; // engineSnifferRpmThreshold is accounted for inside ENGINE(isEngineChartEnabled)
	}


	itoa10(&shaft_signal_msg_index[2], index);
	bool isUp = isUpEvent[(int) ckpSignalType];
	shaft_signal_msg_index[0] = isUp ? 'u' : 'd';

	addEngineSnifferEvent(eventId[(int )ckpSignalType], (char* ) shaft_signal_msg_index);
	if (engineConfiguration->useOnlyRisingEdgeForTrigger) {
		// let's add the opposite event right away
		shaft_signal_msg_index[0] = isUp ? 'd' : 'u';
		addEngineSnifferEvent(eventId[(int )ckpSignalType], (char* ) shaft_signal_msg_index);
	}
}

/**
 * This is used to filter noise spikes (interference) in trigger signal. See 
 * The basic idea is to use not just edges, but the average amount of time the signal stays in '0' or '1'.
 * So we update 'accumulated periods' to track where the signal is. 
 * And then compare between the current period and previous, with some tolerance (allowing for the wheel speed change).
 * @return true if the signal is passed through.
 */
bool TriggerNoiseFilter::noiseFilter(efitick_t nowNt,
		TriggerState * triggerState,
		trigger_event_e signal DECLARE_ENGINE_PARAMETER_SUFFIX) {
	// todo: find a better place for these defs
	static const trigger_event_e opposite[6] = { SHAFT_PRIMARY_RISING, SHAFT_PRIMARY_FALLING, SHAFT_SECONDARY_RISING, SHAFT_SECONDARY_FALLING, 
			SHAFT_3RD_RISING, SHAFT_3RD_FALLING };
	static const trigger_wheel_e triggerIdx[6] = { T_PRIMARY, T_PRIMARY, T_SECONDARY, T_SECONDARY, T_CHANNEL_3, T_CHANNEL_3 };
	// we process all trigger channels independently
	trigger_wheel_e ti = triggerIdx[signal];
	// falling is opposite to rising, and vise versa
	trigger_event_e os = opposite[signal];
	
	// todo: currently only primary channel is filtered, because there are some weird trigger types on other channels
	if (ti != T_PRIMARY)
		return true;
	
	// update period accumulator: for rising signal, we update '0' accumulator, and for falling - '1'
	if (lastSignalTimes[signal] != -1)
		accumSignalPeriods[signal] += nowNt - lastSignalTimes[signal];
	// save current time for this trigger channel
	lastSignalTimes[signal] = nowNt;
	
	// now we want to compare current accumulated period to the stored one 
	efitick_t currentPeriod = accumSignalPeriods[signal];
	// the trick is to compare between different
	efitick_t allowedPeriod = accumSignalPrevPeriods[os];

	// but first check if we're expecting a gap
	bool isGapExpected = TRIGGER_WAVEFORM(isSynchronizationNeeded) && triggerState->shaft_is_synchronized &&
			(triggerState->currentCycle.eventCount[ti] + 1) == TRIGGER_WAVEFORM(expectedEventCount[ti]);
	
	if (isGapExpected) {
		// usually we need to extend the period for gaps, based on the trigger info
		allowedPeriod *= TRIGGER_WAVEFORM(syncRatioAvg);
	}
	
	// also we need some margin for rapidly changing trigger-wheel speed,
	// that's why we expect the period to be no less than 2/3 of the previous period (this is just an empirical 'magic' coef.)
	efitick_t minAllowedPeriod = 2 * allowedPeriod / 3;
	// but no longer than 5/4 of the previous 'normal' period
	efitick_t maxAllowedPeriod = 5 * allowedPeriod / 4;
	
	// above all, check if the signal comes not too early
	if (currentPeriod >= minAllowedPeriod) {
		// now we store this period as a reference for the next time,
		// BUT we store only 'normal' periods, and ignore too long periods (i.e. gaps)
		if (!isGapExpected && (maxAllowedPeriod == 0 || currentPeriod <= maxAllowedPeriod)) {
			accumSignalPrevPeriods[signal] = currentPeriod;
		}
		// reset accumulator
		accumSignalPeriods[signal] = 0;
		return true;
	}
	// all premature or extra-long events are ignored - treated as interference
	return false;
}

void TriggerCentral::handleShaftSignal(trigger_event_e signal, efitick_t timestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	efiAssertVoid(CUSTOM_CONF_NULL, engine!=NULL, "configuration");

	if (triggerShape.shapeDefinitionError) {
		// trigger is broken, we cannot do anything here
		warning(CUSTOM_ERR_UNEXPECTED_SHAFT_EVENT, "Shaft event while trigger is mis-configured");
		// magic value to indicate a problem
		hwEventCounters[0] = 155;
		return;
	}

	// This code gathers some statistics on signals and compares accumulated periods to filter interference
	if (CONFIG(useNoiselessTriggerDecoder)) {
		if (!noiseFilter.noiseFilter(timestamp, &triggerState, signal PASS_ENGINE_PARAMETER_SUFFIX)) {
			return;
		}
		// moved here from hwHandleShaftSignal()
		if (!isUsefulSignal(signal PASS_CONFIG_PARAMETER_SUFFIX)) {
			return;
		}
	}

	engine->onTriggerSignalEvent(timestamp);

	int eventIndex = (int) signal;
	efiAssertVoid(CUSTOM_TRIGGER_EVENT_TYPE, eventIndex >= 0 && eventIndex < HW_EVENT_TYPES, "signal type");
	hwEventCounters[eventIndex]++;


	/**
	 * This invocation changes the state of triggerState
	 */
	triggerState.decodeTriggerEvent(&triggerShape,
			nullptr, engine, signal, timestamp PASS_CONFIG_PARAMETER_SUFFIX);

	/**
	 * If we only have a crank position sensor with four stroke, here we are extending crank revolutions with a 360 degree
	 * cycle into a four stroke, 720 degrees cycle.
	 */
	int triggerIndexForListeners;
	operation_mode_e operationMode = engine->getOperationMode(PASS_ENGINE_PARAMETER_SIGNATURE);
	if (operationMode == FOUR_STROKE_CAM_SENSOR || operationMode == TWO_STROKE) {
		// That's easy - trigger cycle matches engine cycle
		triggerIndexForListeners = triggerState.getCurrentIndex();
	} else {
		int crankDivider = operationMode == FOUR_STROKE_CRANK_SENSOR ? 2 : SYMMETRICAL_CRANK_SENSOR_DIVIDER;

		int crankInternalIndex = triggerState.getTotalRevolutionCounter() % crankDivider;

		triggerIndexForListeners = triggerState.getCurrentIndex() + (crankInternalIndex * getTriggerSize());
	}
	if (triggerIndexForListeners == 0) {
		timeAtVirtualZeroNt = timestamp;
	}
	reportEventToWaveChart(signal, triggerIndexForListeners PASS_ENGINE_PARAMETER_SUFFIX);

	if (!triggerState.shaft_is_synchronized) {
		// we should not propagate event if we do not know where we are
		return;
	}

	if (triggerState.isValidIndex(&ENGINE(triggerCentral.triggerShape))) {
		ScopePerf perf(PE::ShaftPositionListeners);

#if TRIGGER_EXTREME_LOGGING
	scheduleMsg(logger, "trigger %d %d %d", triggerIndexForListeners, getRevolutionCounter(), (int)getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */

		/**
		 * Here we invoke all the listeners - the main engine control logic is inside these listeners
		 */
		for (int i = 0; i < triggerListeneres.currentListenersCount; i++) {
			ShaftPositionListener listener = (ShaftPositionListener) (void*) triggerListeneres.callbacks[i];
			(listener)(signal, triggerIndexForListeners, timestamp PASS_ENGINE_PARAMETER_SUFFIX);
		}

	}
}

EXTERN_ENGINE;

static void triggerShapeInfo(void) {
#if EFI_PROD_CODE || EFI_SIMULATOR
	TriggerWaveform *s = &engine->triggerCentral.triggerShape;
	scheduleMsg(logger, "useRise=%s", boolToString(TRIGGER_WAVEFORM(useRiseEdge)));
	scheduleMsg(logger, "gap from %.2f to %.2f", TRIGGER_WAVEFORM(syncronizationRatioFrom[0]), TRIGGER_WAVEFORM(syncronizationRatioTo[0]));

	for (size_t i = 0; i < s->getSize(); i++) {
		scheduleMsg(logger, "event %d %.2f", i, s->eventAngles[i]);
	}
#endif
}

#if EFI_UNIT_TEST
#include <stdlib.h>

#define TRIGGERS_FILE_NAME "triggers.txt"

/**
 * This is used to generate trigger info which is later used by TriggerImage java class
 * to generate images for documentation
 */
extern bool printTriggerDebug;
void printAllTriggers() {

	FILE * fp = fopen (TRIGGERS_FILE_NAME, "w+");

	fprintf(fp, "# Generated by rusEfi unit test suite\n");
	fprintf(fp, "# This file is used by TriggerImage tool\n");
	fprintf(fp, "# See 'gen_trigger_images.bat'\n");

	//printTriggerDebug = true;
	for (int triggerId = 1; triggerId < TT_UNUSED; triggerId++) {
		trigger_type_e tt = (trigger_type_e) triggerId;

		printf("Exporting %s\r\n", getTrigger_type_e(tt));

		persistent_config_s pc;
		Engine e(&pc);
		Engine *engine = &e;
		persistent_config_s *config = &pc;
		engine_configuration_s *engineConfiguration = &pc.engineConfiguration;
		

		engineConfiguration->trigger.type = tt;
		engineConfiguration->ambiguousOperationMode = FOUR_STROKE_CAM_SENSOR;

		TriggerWaveform *s = &engine->triggerCentral.triggerShape;
		engine->initializeTriggerWaveform(NULL PASS_ENGINE_PARAMETER_SUFFIX);

		if (s->shapeDefinitionError) {
			printf("Trigger error %d\r\n", triggerId);
			exit(-1);
		}

		fprintf(fp, "TRIGGERTYPE %d %d %s %.2f\n", triggerId, s->getLength(), getTrigger_type_e(tt), s->tdcPosition);

		fprintf(fp, "# duty %.2f %.2f\n", s->expectedDutyCycle[0], s->expectedDutyCycle[1]);

		for (int i = 0; i < s->getLength(); i++) {

			int triggerDefinitionCoordinate = (s->getTriggerWaveformSynchPointIndex() + i) % s->getSize();


			fprintf(fp, "event %d %d %.2f\n", i, s->triggerSignals[triggerDefinitionCoordinate], s->eventAngles[i]);
		}

	}
	fclose(fp);
	printf("All triggers exported to %s\n", TRIGGERS_FILE_NAME);
}

#endif


#if EFI_PROD_CODE
extern PwmConfig triggerSignal;
#endif /* #if EFI_PROD_CODE */

extern uint32_t hipLastExecutionCount;
extern uint32_t hwSetTimerDuration;

extern uint32_t maxLockedDuration;
extern uint32_t maxEventCallbackDuration;

extern int perSecondIrqDuration;
extern int perSecondIrqCounter;

#if EFI_PROD_CODE
extern uint32_t maxPrecisionCallbackDuration;
#endif /* EFI_PROD_CODE  */

extern uint32_t maxSchedulingPrecisionLoss;
extern uint32_t *cyccnt;

void resetMaxValues() {
#if EFI_PROD_CODE || EFI_SIMULATOR
	maxEventCallbackDuration = triggerMaxDuration = 0;
#endif /* EFI_PROD_CODE || EFI_SIMULATOR */

	maxSchedulingPrecisionLoss = 0;

#if EFI_CLOCK_LOCKS
	maxLockedDuration = 0;
#endif /* EFI_CLOCK_LOCKS */

#if EFI_PROD_CODE
	maxPrecisionCallbackDuration = 0;
#endif /* EFI_PROD_CODE  */
}

#if HAL_USE_ICU == TRUE
extern int icuRisingCallbackCounter;
extern int icuFallingCallbackCounter;
#endif /* HAL_USE_ICU */

void triggerInfo(void) {
#if EFI_PROD_CODE || EFI_SIMULATOR

	TriggerWaveform *ts = &engine->triggerCentral.triggerShape;


#if (HAL_TRIGGER_USE_PAL == TRUE) && (PAL_USE_CALLBACKS == TRUE)
		scheduleMsg(logger, "trigger PAL mode %d", engine->hwTriggerInputEnabled);
#else

#if HAL_USE_ICU == TRUE
	scheduleMsg(logger, "trigger ICU hw: %d %d %d", icuRisingCallbackCounter, icuFallingCallbackCounter, engine->hwTriggerInputEnabled);
#endif /* HAL_USE_ICU */

#endif /* HAL_TRIGGER_USE_PAL */

	scheduleMsg(logger, "Template %s (%d) trigger %s (%d) useRiseEdge=%s onlyFront=%s useOnlyFirstChannel=%s tdcOffset=%.2f",
			getConfigurationName(engineConfiguration->engineType), engineConfiguration->engineType,
			getTrigger_type_e(engineConfiguration->trigger.type), engineConfiguration->trigger.type,
			boolToString(TRIGGER_WAVEFORM(useRiseEdge)), boolToString(engineConfiguration->useOnlyRisingEdgeForTrigger),
			boolToString(engineConfiguration->trigger.useOnlyFirstChannel), TRIGGER_WAVEFORM(tdcPosition));

	if (engineConfiguration->trigger.type == TT_TOOTHED_WHEEL) {
		scheduleMsg(logger, "total %d/skipped %d", engineConfiguration->trigger.customTotalToothCount,
				engineConfiguration->trigger.customSkippedToothCount);
	}

	scheduleMsg(logger, "trigger#1 event counters up=%d/down=%d", engine->triggerCentral.getHwEventCounter(0),
			engine->triggerCentral.getHwEventCounter(1));

	if (ts->needSecondTriggerInput) {
		scheduleMsg(logger, "trigger#2 event counters up=%d/down=%d", engine->triggerCentral.getHwEventCounter(2),
				engine->triggerCentral.getHwEventCounter(3));
	}
	scheduleMsg(logger, "expected cycle events %d/%d/%d", TRIGGER_WAVEFORM(expectedEventCount[0]),
			TRIGGER_WAVEFORM(expectedEventCount[1]), TRIGGER_WAVEFORM(expectedEventCount[2]));

	scheduleMsg(logger, "trigger type=%d/need2ndChannel=%s", engineConfiguration->trigger.type,
			boolToString(TRIGGER_WAVEFORM(needSecondTriggerInput)));
	scheduleMsg(logger, "expected duty #0=%.2f/#1=%.2f", TRIGGER_WAVEFORM(expectedDutyCycle[0]), TRIGGER_WAVEFORM(expectedDutyCycle[1]));

	scheduleMsg(logger, "synchronizationNeeded=%s/isError=%s/total errors=%d ord_err=%d/total revolutions=%d/self=%s",
			boolToString(ts->isSynchronizationNeeded),
			boolToString(isTriggerDecoderError()), engine->triggerCentral.triggerState.totalTriggerErrorCounter,
			engine->triggerCentral.triggerState.orderingErrorCounter, engine->triggerCentral.triggerState.getTotalRevolutionCounter(),
			boolToString(engine->directSelfStimulation));

	if (TRIGGER_WAVEFORM(isSynchronizationNeeded)) {
		scheduleMsg(logger, "gap from %.2f to %.2f", TRIGGER_WAVEFORM(syncronizationRatioFrom[0]), TRIGGER_WAVEFORM(syncronizationRatioTo[0]));
	}

#endif /* EFI_PROD_CODE || EFI_SIMULATOR */

#if EFI_PROD_CODE
	if (HAVE_CAM_INPUT()) {
		scheduleMsg(logger, "VVT input: %s mode %s", hwPortname(engineConfiguration->camInputs[0]),
				getVvt_mode_e(engineConfiguration->vvtMode));
		scheduleMsg(logger, "VVT event counters: %d/%d", engine->triggerCentral.vvtEventRiseCounter, engine->triggerCentral.vvtEventFallCounter);

	}

	scheduleMsg(logger, "primary trigger input: %s", hwPortname(CONFIG(triggerInputPins)[0]));
	scheduleMsg(logger, "primary trigger simulator: %s %s freq=%d",
			hwPortname(CONFIG(triggerSimulatorPins)[0]),
			getPin_output_mode_e(CONFIG(triggerSimulatorPinModes)[0]),
			CONFIG(triggerSimulatorFrequency));

	if (ts->needSecondTriggerInput) {
		scheduleMsg(logger, "secondary trigger input: %s", hwPortname(CONFIG(triggerInputPins)[1]));
#if EFI_EMULATE_POSITION_SENSORS
		scheduleMsg(logger, "secondary trigger simulator: %s %s phase=%d",
				hwPortname(CONFIG(triggerSimulatorPins)[1]),
				getPin_output_mode_e(CONFIG(triggerSimulatorPinModes)[1]), triggerSignal.safe.phaseIndex);
#endif /* EFI_EMULATE_POSITION_SENSORS */
	}
//	scheduleMsg(logger, "3rd trigger simulator: %s %s", hwPortname(CONFIG(triggerSimulatorPins)[2]),
//			getPin_output_mode_e(CONFIG(triggerSimulatorPinModes)[2]));

	scheduleMsg(logger, "trigger error extra LED: %s %s", hwPortname(CONFIG(triggerErrorPin)),
			getPin_output_mode_e(CONFIG(triggerErrorPinMode)));
	scheduleMsg(logger, "primary logic input: %s", hwPortname(CONFIG(logicAnalyzerPins)[0]));
	scheduleMsg(logger, "secondary logic input: %s", hwPortname(CONFIG(logicAnalyzerPins)[1]));

	scheduleMsg(logger, "maxSchedulingPrecisionLoss=%d", maxSchedulingPrecisionLoss);

#if EFI_CLOCK_LOCKS
	scheduleMsg(logger, "maxLockedDuration=%d / maxTriggerReentraint=%d", maxLockedDuration, maxTriggerReentraint);

	scheduleMsg(logger, "perSecondIrqDuration=%d ticks / perSecondIrqCounter=%d", perSecondIrqDuration, perSecondIrqCounter);
	scheduleMsg(logger, "IRQ CPU utilization %f%%", perSecondIrqDuration / (float)CORE_CLOCK * 100);

#endif /* EFI_CLOCK_LOCKS */

	scheduleMsg(logger, "maxEventCallbackDuration=%d", maxEventCallbackDuration);

#if EFI_HIP_9011
	scheduleMsg(logger, "hipLastExecutionCount=%d", hipLastExecutionCount);
#endif /* EFI_HIP_9011 */
	scheduleMsg(logger, "hwSetTimerDuration=%d", hwSetTimerDuration);

	scheduleMsg(logger, "totalTriggerHandlerMaxTime=%d", triggerMaxDuration);
	scheduleMsg(logger, "maxPrecisionCallbackDuration=%d", maxPrecisionCallbackDuration);
	resetMaxValues();

#endif /* EFI_PROD_CODE */
}

static void resetRunningTriggerCounters() {
#if !EFI_UNIT_TEST
	engine->triggerCentral.resetCounters();
	triggerInfo();
#endif
}

void onConfigurationChangeTriggerCallback(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	bool changed = false;
	for (int i = 0; i < CAM_INPUTS_COUNT; i++) {
		changed |= isConfigurationChanged(camInputs[i]);
	}

	changed |=
		isConfigurationChanged(trigger.type) ||
		isConfigurationChanged(ambiguousOperationMode) ||
		isConfigurationChanged(useOnlyRisingEdgeForTrigger) ||
		isConfigurationChanged(globalTriggerAngleOffset) ||
		isConfigurationChanged(trigger.customTotalToothCount) ||
		isConfigurationChanged(trigger.customSkippedToothCount) ||
		isConfigurationChanged(triggerInputPins[0]) ||
		isConfigurationChanged(triggerInputPins[1]) ||
		isConfigurationChanged(triggerInputPins[2]) ||
		isConfigurationChanged(vvtMode) ||
		isConfigurationChanged(vvtCamSensorUseRise) ||
		isConfigurationChanged(vvtOffset) ||
		isConfigurationChanged(vvtDisplayInverted);
	if (changed) {
		assertEngineReference();

	#if EFI_ENGINE_CONTROL
		ENGINE(initializeTriggerWaveform(logger PASS_ENGINE_PARAMETER_SUFFIX));
		engine->triggerCentral.noiseFilter.resetAccumSignalData();
	#endif
	}
#if EFI_DEFAILED_LOGGING
	scheduleMsg(logger, "isTriggerConfigChanged=%d", engine->isTriggerConfigChanged);
#endif /* EFI_DEFAILED_LOGGING */

	// we do not want to miss two updates in a row
	engine->isTriggerConfigChanged = engine->isTriggerConfigChanged || changed;
}

/**
 * @returns true if configuration just changed, and if that change has affected trigger
 */
bool checkIfTriggerConfigChanged(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	bool result = engine->triggerVersion.isOld(engine->getGlobalConfigurationVersion()) && engine->isTriggerConfigChanged;
	engine->isTriggerConfigChanged = false; // whoever has called the method is supposed to react to changes
	return result;
}

bool isTriggerConfigChanged(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engine->isTriggerConfigChanged;
}

void initTriggerCentral(Logging *sharedLogger) {
	logger = sharedLogger;
	strcpy((char*) shaft_signal_msg_index, "x_");

#if EFI_ENGINE_SNIFFER
	initWaveChart(&waveChart);
#endif /* EFI_ENGINE_SNIFFER */

#if EFI_PROD_CODE || EFI_SIMULATOR
	addConsoleAction(CMD_TRIGGERINFO, triggerInfo);
	addConsoleAction("trigger_shape_info", triggerShapeInfo);
	addConsoleAction("reset_trigger", resetRunningTriggerCounters);
#endif

}

#endif
