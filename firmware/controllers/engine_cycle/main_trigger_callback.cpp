/**
 * @file    main_trigger_callback.cpp
 * @brief   Main logic is here!
 *
 * See http://rusefi.com/docs/html/
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

#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT

#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */
#include "main_trigger_callback.h"
#include "efi_gpio.h"
#include "engine_math.h"
#include "trigger_central.h"
#include "spark_logic.h"
#include "rpm_calculator.h"
#include "engine_configuration.h"
#include "interpolation.h"
#include "advance_map.h"
#include "allsensors.h"
#include "cyclic_buffer.h"
#include "fuel_math.h"
#include "cdm_ion_sense.h"
#include "engine_controller.h"
#include "efi_gpio.h"
#if EFI_PROD_CODE
#include "os_util.h"
#endif /* EFI_PROD_CODE */
#include "local_version_holder.h"
#include "event_queue.h"
#include "engine.h"
#include "perf_trace.h"
#include "sensor.h"

#include "backup_ram.h"

EXTERN_ENGINE;
extern TunerStudioOutputChannels tsOutputChannels;
static const char *prevOutputName = nullptr;

static InjectionEvent primeInjEvent;

static Logging *logger;
#if ! EFI_UNIT_TEST
static Pid fuelPid(&persistentState.persistentConfiguration.engineConfiguration.fuelClosedLoopPid);
#endif

// todo: figure out if this even helps?
//#if defined __GNUC__
//#define RAM_METHOD_PREFIX __attribute__((section(".ram")))
//#else
//#define RAM_METHOD_PREFIX
//#endif

void startSimultaniousInjection(Engine *engine) {
	for (int i = 0; i < engine->engineConfigurationPtr->specs.cylindersCount; i++) {
		enginePins.injectors[i].setHigh();
	}
}

static void endSimultaniousInjectionOnlyTogglePins(Engine *engine) {
	for (int i = 0; i < engine->engineConfigurationPtr->specs.cylindersCount; i++) {
		enginePins.injectors[i].setLow();
	}
}

void endSimultaniousInjection(InjectionEvent *event) {
#if EFI_UNIT_TEST
	Engine *engine = event->engine;
	EXPAND_Engine;
#endif
	event->isScheduled = false;

	endSimultaniousInjectionOnlyTogglePins(engine);
	engine->injectionEvents.addFuelEventsForCylinder(event->ownIndex PASS_ENGINE_PARAMETER_SUFFIX);
}

static inline void turnInjectionPinHigh(InjectorOutputPin *output) {
	output->overlappingCounter++;

#if FUEL_MATH_EXTREME_LOGGING
	printf("turnInjectionPinHigh %s %d %d\r\n", output->name, output->overlappingCounter, (int)getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */

	if (output->overlappingCounter > 1) {
//		/**
//		 * #299
//		 * this is another kind of overlap which happens in case of a small duty cycle after a large duty cycle
//		 */
#if FUEL_MATH_EXTREME_LOGGING
		printf("overlapping, no need to touch pin %s %d\r\n", output->name, (int)getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */
	} else {
#if FUEL_MATH_EXTREME_LOGGING
		const char * w = output->currentLogicValue == true ? "err" : "";
//	scheduleMsg(&sharedLogger, "^ %spin=%s eventIndex %d %d", w, output->name,
//			getRevolutionCounter(), getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */

		output->setHigh();
	}
}

void turnInjectionPinHigh(InjectionEvent *event) {
	for (int i = 0;i < MAX_WIRES_COUNT;i++) {
		InjectorOutputPin *output = event->outputs[i];

		if (output) {
			turnInjectionPinHigh(output);
		}
	}
}

static inline void turnInjectionPinLow(InjectorOutputPin *output) {
#if FUEL_MATH_EXTREME_LOGGING
	printf("turnInjectionPinLow %s %d %d\r\n", output->name, output->overlappingCounter, (int)getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */


#if FUEL_MATH_EXTREME_LOGGING
		const char * w = output->currentLogicValue == false ? "err" : "";

//	scheduleMsg(&sharedLogger, "- %spin=%s eventIndex %d %d", w, output->name,
//			getRevolutionCounter(), getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */

		output->overlappingCounter--;
		if (output->overlappingCounter > 0) {
#if FUEL_MATH_EXTREME_LOGGING
			printf("was overlapping, no need to touch pin %s %d\r\n", output->name, (int)getTimeNowUs());
#endif /* FUEL_MATH_EXTREME_LOGGING */
		} else {
			output->setLow();
		}

}

void turnInjectionPinLow(InjectionEvent *event) {
	event->isScheduled = false;
	for (int i = 0;i<MAX_WIRES_COUNT;i++) {
		InjectorOutputPin *output = event->outputs[i];
		if (output != NULL) {
			turnInjectionPinLow(output);
		}
	}
#if EFI_UNIT_TEST
	Engine *engine = event->engine;
	EXPAND_Engine;
#endif
	ENGINE(injectionEvents.addFuelEventsForCylinder(event->ownIndex PASS_ENGINE_PARAMETER_SUFFIX));
}

static ALWAYS_INLINE void handleFuelInjectionEvent(int injEventIndex, InjectionEvent *event,
		int rpm, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {

	/**
	 * todo: this is a bit tricky with batched injection. is it? Does the same
	 * wetting coefficient works the same way for any injection mode, or is something
	 * x2 or /2?
	 */

	size_t injectorIndex = event->outputs[0]->injectorIndex;
	const floatms_t injectionDuration = ENGINE(wallFuel[injectorIndex]).adjust(ENGINE(injectionDuration) PASS_ENGINE_PARAMETER_SUFFIX);
#if EFI_PRINTF_FUEL_DETAILS
	printf("fuel injectionDuration=%.2f adjusted=%.2f\t\n", ENGINE(injectionDuration), injectionDuration);
#endif /*EFI_PRINTF_FUEL_DETAILS */

	bool isCranking = ENGINE(rpmCalculator).isCranking(PASS_ENGINE_PARAMETER_SIGNATURE);
	/**
	 * todo: pre-calculate 'numberOfInjections'
	 * see also injectorDutyCycle
	 */
	if (!isCranking && injectionDuration * getNumberOfInjections(engineConfiguration->injectionMode PASS_ENGINE_PARAMETER_SUFFIX) > getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX)) {
		warning(CUSTOM_TOO_LONG_FUEL_INJECTION, "Too long fuel injection %.2fms", injectionDuration);
	} else if (isCranking && injectionDuration * getNumberOfInjections(engineConfiguration->crankingInjectionMode PASS_ENGINE_PARAMETER_SUFFIX) > getEngineCycleDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX)) {
		warning(CUSTOM_TOO_LONG_CRANKING_FUEL_INJECTION, "Too long cranking fuel injection %.2fms", injectionDuration);
	}

	// Store 'pure' injection duration (w/o injector lag) for fuel rate calc.
	engine->engineState.fuelConsumption.addData(injectionDuration - ENGINE(engineState.running.injectorLag));
	
	ENGINE(actualLastInjection) = injectionDuration;
	if (cisnan(injectionDuration)) {
		warning(CUSTOM_OBD_NAN_INJECTION, "NaN injection pulse");
		return;
	}
	if (injectionDuration < 0) {
		warning(CUSTOM_OBD_NEG_INJECTION, "Negative injection pulse %.2f", injectionDuration);
		return;
	}

	// If somebody commanded an impossibly short injection, do nothing.
	// Durations under 50us-ish aren't safe for the scheduler
	// as their order may be swapped, resulting in a stuck open injector
	// see https://github.com/rusefi/rusefi/pull/596 for more details
	if (injectionDuration < 0.050f)
	{
		return;
	}

	floatus_t durationUs = MS2US(injectionDuration);


	// we are ignoring low RPM in order not to handle "engine was stopped to engine now running" transition
	if (rpm > 2 * engineConfiguration->cranking.rpm) {
		const char *outputName = event->outputs[0]->name;
		if (prevOutputName == outputName
				&& engineConfiguration->injectionMode != IM_SIMULTANEOUS
				&& engineConfiguration->injectionMode != IM_SINGLE_POINT) {
			warning(CUSTOM_OBD_SKIPPED_FUEL, "looks like skipped fuel event %d %s", getRevolutionCounter(), outputName);
		}
		prevOutputName = outputName;
	}

#if EFI_UNIT_TEST || EFI_SIMULATOR || EFI_PRINTF_FUEL_DETAILS
	InjectorOutputPin *output = event->outputs[0];
	printf("fuelout %s duration %d total=%d\t\n", output->name, (int)durationUs,
			(int)MS2US(getCrankshaftRevolutionTimeMs(GET_RPM_VALUE)));
#endif /*EFI_PRINTF_FUEL_DETAILS */

	if (event->isScheduled) {
#if EFI_UNIT_TEST || EFI_SIMULATOR
	printf("still used1 %s %d\r\n", output->name, (int)getTimeNowUs());
#endif /* EFI_UNIT_TEST || EFI_SIMULATOR */
		return; // this InjectionEvent is still needed for an extremely long injection scheduled previously
	}

	event->isScheduled = true;

	action_s startAction, endAction;
	// We use different callbacks based on whether we're running sequential mode or not - everything else is the same
	if (event->isSimultanious) {
		startAction = { &startSimultaniousInjection, engine };
		endAction = { &endSimultaniousInjection, event };
	} else {
		// sequential or batch
		startAction = { &turnInjectionPinHigh, event };
		endAction = { &turnInjectionPinLow, event };
	}

	efitick_t startTime = scheduleByAngle(&event->signalTimerUp, nowNt, event->injectionStart.angleOffsetFromTriggerEvent, startAction PASS_ENGINE_PARAMETER_SUFFIX);
	efitick_t turnOffTime = startTime + US2NT((int)durationUs);
	engine->executor.scheduleByTimestampNt(&event->endOfInjectionEvent, turnOffTime, endAction);

#if EFI_UNIT_TEST
		printf("scheduling injection angle=%.2f/delay=%.2f injectionDuration=%.2f\r\n", event->injectionStart.angleOffsetFromTriggerEvent, NT2US(startTime - nowNt), injectionDuration);
#endif
#if EFI_DEFAILED_LOGGING
	scheduleMsg(logger, "handleFuel pin=%s eventIndex %d duration=%.2fms %d", event->outputs[0]->name,
			injEventIndex,
			injectionDuration,
			getRevolutionCounter());
	scheduleMsg(logger, "handleFuel pin=%s delay=%.2f %d", event->outputs[0]->name, NT2US(startTime - nowNt),
			getRevolutionCounter());
#endif /* EFI_DEFAILED_LOGGING */
}

static void fuelClosedLoopCorrection(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if ! EFI_UNIT_TEST
	if (GET_RPM_VALUE < CONFIG(fuelClosedLoopRpmThreshold) ||
			Sensor::get(SensorType::Clt).value_or(0) < CONFIG(fuelClosedLoopCltThreshold) ||
			Sensor::get(SensorType::Tps1).value_or(100) > CONFIG(fuelClosedLoopTpsThreshold) ||
			ENGINE(sensors.currentAfr) < CONFIG(fuelClosedLoopAfrLowThreshold) ||
			ENGINE(sensors.currentAfr) > engineConfiguration->fuelClosedLoopAfrHighThreshold) {
		engine->engineState.running.pidCorrection = 0;
		fuelPid.reset();
		return;
	}

	engine->engineState.running.pidCorrection = fuelPid.getOutput(ENGINE(engineState.targetAFR), ENGINE(sensors.currentAfr), NOT_TIME_BASED_PID);
	if (engineConfiguration->debugMode == DBG_FUEL_PID_CORRECTION) {
#if EFI_TUNER_STUDIO
		tsOutputChannels.debugFloatField1 = engine->engineState.running.pidCorrection;
		fuelPid.postState(&tsOutputChannels);
#endif /* EFI_TUNER_STUDIO */
	}

#endif
}


static ALWAYS_INLINE void handleFuel(const bool limitedFuel, uint32_t trgEventIndex, int rpm, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::HandleFuel);
	
	efiAssertVoid(CUSTOM_STACK_6627, getCurrentRemainingStack() > 128, "lowstck#3");
	efiAssertVoid(CUSTOM_ERR_6628, trgEventIndex < engine->engineCycleEventCount, "handleFuel/event index");

	if (!isInjectionEnabled(PASS_ENGINE_PARAMETER_SIGNATURE) || limitedFuel) {
		return;
	}
	if (ENGINE(isCylinderCleanupMode)) {
		return;
	}


	/**
	 * Ignition events are defined by addFuelEvents() according to selected
	 * fueling strategy
	 */
	FuelSchedule *fs = &ENGINE(injectionEvents);
	if (!fs->isReady) {
		fs->addFuelEvents(PASS_ENGINE_PARAMETER_SIGNATURE);
	}

#if FUEL_MATH_EXTREME_LOGGING
	scheduleMsg(logger, "handleFuel ind=%d %d", trgEventIndex, getRevolutionCounter());
#endif /* FUEL_MATH_EXTREME_LOGGING */

	ENGINE(tpsAccelEnrichment.onNewValue(Sensor::get(SensorType::Tps1).value_or(0) PASS_ENGINE_PARAMETER_SUFFIX));
	if (trgEventIndex == 0) {
		ENGINE(tpsAccelEnrichment.onEngineCycleTps(PASS_ENGINE_PARAMETER_SIGNATURE));
		ENGINE(engineLoadAccelEnrichment.onEngineCycle(PASS_ENGINE_PARAMETER_SIGNATURE));
	}

	for (int injEventIndex = 0; injEventIndex < CONFIG(specs.cylindersCount); injEventIndex++) {
		InjectionEvent *event = &fs->elements[injEventIndex];
		uint32_t eventIndex = event->injectionStart.triggerEventIndex;
// right after trigger change we are still using old & invalid fuel schedule. good news is we do not change trigger on the fly in real life
//		efiAssertVoid(CUSTOM_ERR_ASSERT_VOID, eventIndex < ENGINE(triggerShape.getLength()), "handleFuel/event sch index");
		if (eventIndex != trgEventIndex) {
			continue;
		}
		handleFuelInjectionEvent(injEventIndex, event, rpm, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	}
}

#if EFI_PROD_CODE
/**
 * this field is used as an Expression in IAR debugger
 */
uint32_t *cyccnt = (uint32_t*) &DWT->CYCCNT;
#endif

/**
 * This is the main trigger event handler.
 * Both injection and ignition are controlled from this method.
 */
static void mainTriggerCallback(trigger_event_e ckpSignalType, uint32_t trgEventIndex, efitick_t edgeTimestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::MainTriggerCallback);

	(void) ckpSignalType;


	if (engineConfiguration->vvtMode == MIATA_NB2 && engine->triggerCentral.vvtSyncTimeNt == 0) {
		// this is a bit spaghetti code for sure
		// do not spark & do not fuel until we have VVT sync. NB2 is a special case
		// due to symmetrical crank wheel and we need to make sure no spark happens out of sync
		return;
	}

	if (hasFirmwareError()) {
		/**
		 * In case on a major error we should not process any more events.
		 * TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		 */
		return;
	}
	efiAssertVoid(CUSTOM_STACK_6629, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "lowstck#2a");

#if EFI_CDM_INTEGRATION
	if (trgEventIndex == 0 && CONFIG(cdmInputPin) != GPIO_UNASSIGNED) {
		int cdmKnockValue = getCurrentCdmValue(engine->triggerCentral.triggerState.getTotalRevolutionCounter());
		engine->knockLogic(cdmKnockValue);
	}
#endif /* EFI_CDM_INTEGRATION */

	if (trgEventIndex >= ENGINE(engineCycleEventCount)) {
		/**
		 * this could happen in case of a trigger error, just exit silently since the trigger error is supposed to be handled already
		 * todo: should this check be somewhere higher so that no trigger listeners are invoked with noise?
		 */
		return;
	}

	int rpm = GET_RPM_VALUE;
	if (rpm == 0) {
		// this happens while we just start cranking
		// todo: check for 'trigger->is_synchnonized?'
		// TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		return;
	}
	if (rpm == NOISY_RPM) {
		warning(OBD_Crankshaft_Position_Sensor_A_Circuit_Malfunction, "noisy trigger");
		// TODO: add 'pin shutdown' invocation somewhere - coils might be still open here!
		return;
	}
	bool limitedSpark = rpm > CONFIG(rpmHardLimit);
	bool limitedFuel = rpm > CONFIG(rpmHardLimit);

	if (CONFIG(boostCutPressure) !=0) {
		if (getMap(PASS_ENGINE_PARAMETER_SIGNATURE) > CONFIG(boostCutPressure)) {
			limitedSpark = true;
			limitedFuel = true;
		}
	}

	if (limitedSpark || limitedFuel) {
    engine->rpmCutIndicator = true;
	}

#if EFI_TUNER_STUDIO
	tsOutputChannels.rpmHardCut = engine->rpmCutIndicator;
#endif

	if (trgEventIndex == 0) {
		if (HAVE_CAM_INPUT()) {
			engine->triggerCentral.validateCamVvtCounters();
		}

		if (checkIfTriggerConfigChanged(PASS_ENGINE_PARAMETER_SIGNATURE)) {
			engine->ignitionEvents.isReady = false; // we need to rebuild complete ignition schedule
			engine->injectionEvents.isReady = false;
			// moved 'triggerIndexByAngle' into trigger initialization (why was it invoked from here if it's only about trigger shape & optimization?)
			// see initializeTriggerWaveform() -> prepareOutputSignals(PASS_ENGINE_PARAMETER_SIGNATURE)

			// we need this to apply new 'triggerIndexByAngle' values
			engine->periodicFastCallback(PASS_ENGINE_PARAMETER_SIGNATURE);
		}

		if (CONFIG(fuelClosedLoopCorrectionEnabled)) {
			fuelClosedLoopCorrection(PASS_ENGINE_PARAMETER_SIGNATURE);
		}
	}

	efiAssertVoid(CUSTOM_IGN_MATH_STATE, !CONFIG(useOnlyRisingEdgeForTrigger) || CONFIG(ignMathCalculateAtIndex) % 2 == 0, "invalid ignMathCalculateAtIndex");

	if (trgEventIndex == (uint32_t)CONFIG(ignMathCalculateAtIndex)) {
		if (CONFIG(externalKnockSenseAdc) != EFI_ADC_NONE) {
			float externalKnockValue = getVoltageDivided("knock", engineConfiguration->externalKnockSenseAdc PASS_ENGINE_PARAMETER_SUFFIX);
			engine->knockLogic(externalKnockValue PASS_ENGINE_PARAMETER_SUFFIX);
		}
	}


	/**
	 * For fuel we schedule start of injection based on trigger angle, and then inject for
	 * specified duration of time
	 */
	handleFuel(limitedFuel, trgEventIndex, rpm, edgeTimestamp PASS_ENGINE_PARAMETER_SUFFIX);
	/**
	 * For spark we schedule both start of coil charge and actual spark based on trigger angle
	 */
	onTriggerEventSparkLogic(limitedSpark, trgEventIndex, rpm, edgeTimestamp PASS_ENGINE_PARAMETER_SUFFIX);
}

// Check if the engine is not stopped or cylinder cleanup is activated
static bool isPrimeInjectionPulseSkipped(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	if (!engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE))
		return true;
	return CONFIG(isCylinderCleanupEnabled) && (Sensor::get(SensorType::Tps1).value_or(0) > CLEANUP_MODE_TPS);
}

/**
 * Prime injection pulse, mainly needed for mono-injectors or long intake manifolds.
 * See testStartOfCrankingPrimingPulse()
 */
void startPrimeInjectionPulse(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// First, we need a protection against 'fake' ignition switch on and off (i.e. no engine started), to avoid repeated prime pulses.
	// So we check and update the ignition switch counter in non-volatile backup-RAM
#if EFI_PROD_CODE
	uint32_t ignSwitchCounter = backupRamLoad(BACKUP_IGNITION_SWITCH_COUNTER);
#else /* EFI_PROD_CODE */
	uint32_t ignSwitchCounter = 0;
#endif /* EFI_PROD_CODE */

	// if we're just toying with the ignition switch, give it another chance eventually...
	if (ignSwitchCounter > 10)
		ignSwitchCounter = 0;
	// If we're going to skip this pulse, then save the counter as 0.
	// That's because we'll definitely need the prime pulse next time (either due to the cylinder cleanup or the engine spinning)
	if (isPrimeInjectionPulseSkipped(PASS_ENGINE_PARAMETER_SIGNATURE))
		ignSwitchCounter = -1;
	// start prime injection if this is a 'fresh start'
	if (ignSwitchCounter == 0) {
		// fill-in the prime event struct
#if EFI_UNIT_TEST
		primeInjEvent.engine = engine;
#endif /* EFI_UNIT_TEST */
		primeInjEvent.ownIndex = 0;
		primeInjEvent.isSimultanious = true;

		scheduling_s *sDown = &ENGINE(injectionEvents.elements[0]).endOfInjectionEvent;
		// When the engine is hot, basically we don't need prime inj.pulse, so we use an interpolation over temperature (falloff).
		// If 'primeInjFalloffTemperature' is not specified (by default), we have a prime pulse deactivation at zero celsius degrees, which is okay.
		const float maxPrimeInjAtTemperature = -40.0f;	// at this temperature the pulse is maximal.
		floatms_t pulseLength = interpolateClamped(maxPrimeInjAtTemperature, CONFIG(startOfCrankingPrimingPulse),
			CONFIG(primeInjFalloffTemperature), 0.0f, Sensor::get(SensorType::Clt).value_or(70));
		if (pulseLength > 0) {
			startSimultaniousInjection(engine);
			efitimeus_t turnOffDelayUs = (efitimeus_t)efiRound(MS2US(pulseLength), 1.0f);
			engine->executor.scheduleForLater(sDown, turnOffDelayUs, { &endSimultaniousInjectionOnlyTogglePins, engine });
		}
	}
#if EFI_PROD_CODE
	// we'll reset it later when the engine starts
	backupRamSave(BACKUP_IGNITION_SWITCH_COUNTER, ignSwitchCounter + 1);
#endif /* EFI_PROD_CODE */
}

void updatePrimeInjectionPulseState(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_PROD_CODE
	static bool counterWasReset = false;
	if (counterWasReset)
		return;

	if (!engine->rpmCalculator.isStopped(PASS_ENGINE_PARAMETER_SIGNATURE)) {
		backupRamSave(BACKUP_IGNITION_SWITCH_COUNTER, 0);
		counterWasReset = true;
	}
#endif /* EFI_PROD_CODE */
}

#if EFI_ENGINE_SNIFFER
#include "engine_sniffer.h"
#endif

static void showMainInfo(Engine *engine) {
#if EFI_PROD_CODE
	int rpm = GET_RPM();
	float el = getEngineLoadT(PASS_ENGINE_PARAMETER_SIGNATURE);
	scheduleMsg(logger, "rpm %d engine_load %.2f", rpm, el);
	scheduleMsg(logger, "fuel %.2fms timing %.2f", getInjectionDuration(rpm PASS_ENGINE_PARAMETER_SUFFIX), engine->engineState.timingAdvance);
#endif /* EFI_PROD_CODE */
}

void initMainEventListener(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	efiAssertVoid(CUSTOM_ERR_6631, engine!=NULL, "null engine");

#if EFI_PROD_CODE
	addConsoleActionP("maininfo", (VoidPtr) showMainInfo, engine);

	printMsg(logger, "initMainLoop: %d", currentTimeMillis());
	if (!isInjectionEnabled(PASS_ENGINE_PARAMETER_SIGNATURE))
		printMsg(logger, "!!!!!!!!!!!!!!!!!!! injection disabled");
#endif

	addTriggerEventListener(mainTriggerCallback, "main loop", engine);

    // We start prime injection pulse at the early init stage - don't wait for the engine to start spinning!
    if (CONFIG(startOfCrankingPrimingPulse) > 0)
    	startPrimeInjectionPulse(PASS_ENGINE_PARAMETER_SIGNATURE);

}

#endif /* EFI_ENGINE_CONTROL */
