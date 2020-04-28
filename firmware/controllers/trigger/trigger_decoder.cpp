/**
 * @file	trigger_decoder.cpp
 *
 * @date Dec 24, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 *
 *
 *
 * enable trigger_details
 * DBG_TRIGGER_COUNTERS = 5
 * set debug_mode 5
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

#include "obd_error_codes.h"
#include "trigger_decoder.h"
#include "cyclic_buffer.h"
#include "efi_gpio.h"
#include "engine.h"
#include "engine_math.h"
#include "trigger_central.h"
#include "trigger_simulator.h"
#include "perf_trace.h"

#if EFI_SENSOR_CHART
#include "sensor_chart.h"
#endif

TriggerState::TriggerState() {
	resetTriggerState();
}

void TriggerState::setShaftSynchronized(bool value) {
	if (value) {
		if (!shaft_is_synchronized) {
			// just got synchronized
			mostRecentSyncTime = getTimeNowNt();
		}
	} else {
		// sync loss
		mostRecentSyncTime = 0;
	}
	shaft_is_synchronized = value;
}

void TriggerState::resetTriggerState() {
	setShaftSynchronized(false);
	toothed_previous_time = 0;

	memset(toothDurations, 0, sizeof(toothDurations));

	totalRevolutionCounter = 0;
	totalTriggerErrorCounter = 0;
	orderingErrorCounter = 0;
	// we need this initial to have not_running at first invocation
	previousShaftEventTimeNt = (efitimems_t) -10 * NT_PER_SECOND;
	lastDecodingErrorTime = US2NT(-10000000LL);
	someSortOfTriggerError = false;

	memset(toothDurations, 0, sizeof(toothDurations));
	curSignal = SHAFT_PRIMARY_FALLING;
	prevSignal = SHAFT_PRIMARY_FALLING;
	startOfCycleNt = 0;

	resetCurrentCycleState();
	memset(expectedTotalTime, 0, sizeof(expectedTotalTime));

	totalEventCountBase = 0;
	isFirstEvent = true;
}

void TriggerState::setTriggerErrorState() {
	lastDecodingErrorTime = getTimeNowNt();
	someSortOfTriggerError = true;
}

void TriggerState::resetCurrentCycleState() {
	memset(currentCycle.eventCount, 0, sizeof(currentCycle.eventCount));
	memset(currentCycle.timeOfPreviousEventNt, 0, sizeof(currentCycle.timeOfPreviousEventNt));
	memset(currentCycle.totalTimeNt, 0, sizeof(currentCycle.totalTimeNt));
	currentCycle.current_index = 0;
}

TriggerStateWithRunningStatistics::TriggerStateWithRunningStatistics() :
		//https://en.cppreference.com/w/cpp/language/zero_initialization
		timeOfLastEvent(), instantRpmValue()
		{
}

#if EFI_SHAFT_POSITION_INPUT

EXTERN_ENGINE;

#if ! EFI_PROD_CODE
bool printTriggerDebug = false;
float actualSynchGap;
#endif /* ! EFI_PROD_CODE */

static Logging * logger = nullptr;

/**
 * @return TRUE is something is wrong with trigger decoding
 */
bool isTriggerDecoderError(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engine->triggerErrorDetection.sum(6) > 4;
}

void calculateTriggerSynchPoint(TriggerWaveform *shape, TriggerState *state DECLARE_ENGINE_PARAMETER_SUFFIX) {
#if EFI_PROD_CODE
	efiAssertVoid(CUSTOM_TRIGGER_STACK, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "calc s");
#endif
	trigger_config_s const*triggerConfig = &engineConfiguration->trigger;

	engine->triggerErrorDetection.clear();
	shape->triggerShapeSynchPointIndex = state->findTriggerZeroEventIndex(shape, triggerConfig PASS_CONFIG_PARAMETER_SUFFIX);

	int length = shape->getLength();
	engine->engineCycleEventCount = length;
	efiAssertVoid(CUSTOM_SHAPE_LEN_ZERO, length > 0, "shapeLength=0");
	if (length >= PWM_PHASE_MAX_COUNT) {
		warning(CUSTOM_ERR_TRIGGER_WAVEFORM_TOO_LONG, "Count above %d", length);
		shape->setShapeDefinitionError(true);
		return;
	}

	float firstAngle = shape->getAngle(shape->triggerShapeSynchPointIndex);
	assertAngleRange(shape->triggerShapeSynchPointIndex, "firstAngle", CUSTOM_TRIGGER_SYNC_ANGLE);

	int riseOnlyIndex = 0;

	for (int eventIndex = 0; eventIndex < length; eventIndex++) {
		if (eventIndex == 0) {
			// explicit check for zero to avoid issues where logical zero is not exactly zero due to float nature
			shape->eventAngles[0] = 0;
			// this value would be used in case of front-only
			shape->eventAngles[1] = 0;
			shape->riseOnlyIndexes[0] = 0;
		} else {
			assertAngleRange(shape->triggerShapeSynchPointIndex, "triggerShapeSynchPointIndex", CUSTOM_TRIGGER_SYNC_ANGLE2);
			unsigned int triggerDefinitionCoordinate = (shape->triggerShapeSynchPointIndex + eventIndex) % engine->engineCycleEventCount;
			efiAssertVoid(CUSTOM_TRIGGER_CYCLE, engine->engineCycleEventCount != 0, "zero engineCycleEventCount");
			int triggerDefinitionIndex = triggerDefinitionCoordinate >= shape->privateTriggerDefinitionSize ? triggerDefinitionCoordinate - shape->privateTriggerDefinitionSize : triggerDefinitionCoordinate;
			float angle = shape->getAngle(triggerDefinitionCoordinate) - firstAngle;
			efiAssertVoid(CUSTOM_TRIGGER_CYCLE, !cisnan(angle), "trgSyncNaN");
			fixAngle(angle, "trgSync", CUSTOM_TRIGGER_SYNC_ANGLE_RANGE);
			if (engineConfiguration->useOnlyRisingEdgeForTrigger) {
				if (shape->isRiseEvent[triggerDefinitionIndex]) {
					riseOnlyIndex += 2;
					shape->eventAngles[riseOnlyIndex] = angle;
					shape->eventAngles[riseOnlyIndex + 1] = angle;
				}
			} else {
				shape->eventAngles[eventIndex] = angle;
			}

			shape->riseOnlyIndexes[eventIndex] = riseOnlyIndex;
		}
	}
}

int64_t TriggerState::getTotalEventCounter() const {
	return totalEventCountBase + currentCycle.current_index;
}

int TriggerState::getTotalRevolutionCounter() const {
	return totalRevolutionCounter;
}

void TriggerStateWithRunningStatistics::movePreSynchTimestamps(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	// here we take timestamps of events which happened prior to synchronization and place them
	// at appropriate locations
	for (int i = 0; i < spinningEventIndex;i++) {
		timeOfLastEvent[getTriggerSize() - i] = spinningEvents[i];
	}
}

float TriggerStateWithRunningStatistics::calculateInstantRpm(int *prevIndexOut, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	int current_index = currentCycle.current_index; // local copy so that noone changes the value on us
	timeOfLastEvent[current_index] = nowNt;
	/**
	 * Here we calculate RPM based on last 90 degrees
	 */
	angle_t currentAngle = TRIGGER_WAVEFORM(eventAngles[current_index]);
	// todo: make this '90' depend on cylinder count or trigger shape?
	if (cisnan(currentAngle)) {
		return NOISY_RPM;
	}
	angle_t previousAngle = currentAngle - 90;
	fixAngle(previousAngle, "prevAngle", CUSTOM_ERR_TRIGGER_ANGLE_RANGE);
	// todo: prevIndex should be pre-calculated
	int prevIndex = TRIGGER_WAVEFORM(triggerIndexByAngle[(int)previousAngle]);

	if (prevIndexOut) {
		*prevIndexOut = prevIndex;
	}

	// now let's get precise angle for that event
	angle_t prevIndexAngle = TRIGGER_WAVEFORM(eventAngles[prevIndex]);
	efitick_t time90ago = timeOfLastEvent[prevIndex];
	if (time90ago == 0) {
		return prevInstantRpmValue;
	}
	// we are OK to subtract 32 bit value from more precise 64 bit since the result would 32 bit which is
	// OK for small time differences like this one
	uint32_t time = nowNt - time90ago;
	angle_t angleDiff = currentAngle - prevIndexAngle;
	// todo: angle diff should be pre-calculated
	fixAngle(angleDiff, "angleDiff", CUSTOM_ERR_6561);

	// just for safety
	if (time == 0)
		return prevInstantRpmValue;

	float instantRpm = (60000000.0 / 360 * US_TO_NT_MULTIPLIER) * angleDiff / time;
	instantRpmValue[current_index] = instantRpm;

	// This fixes early RPM instability based on incomplete data
	if (instantRpm < RPM_LOW_THRESHOLD)
		return prevInstantRpmValue;
	prevInstantRpmValue = instantRpm;

	return instantRpm;
}

void TriggerStateWithRunningStatistics::setLastEventTimeForInstantRpm(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (shaft_is_synchronized) {
		return;
	}
	// here we remember tooth timestamps which happen prior to synchronization
	if (spinningEventIndex >= PRE_SYNC_EVENTS) {
		// too many events while trying to find synchronization point
		// todo: better implementation would be to shift here or use cyclic buffer so that we keep last
		// 'PRE_SYNC_EVENTS' events
		return;
	}
	spinningEvents[spinningEventIndex++] = nowNt;
}

void TriggerStateWithRunningStatistics::runtimeStatistics(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX) {
	if (engineConfiguration->debugMode == DBG_INSTANT_RPM) {
		instantRpm = calculateInstantRpm(NULL, nowNt PASS_ENGINE_PARAMETER_SUFFIX);
	}
	if (ENGINE(sensorChartMode) == SC_RPM_ACCEL || ENGINE(sensorChartMode) == SC_DETAILED_RPM) {
		int prevIndex;
		instantRpm = calculateInstantRpm(&prevIndex, nowNt PASS_ENGINE_PARAMETER_SUFFIX);

#if EFI_SENSOR_CHART
		angle_t currentAngle = TRIGGER_WAVEFORM(eventAngles[currentCycle.current_index]);
		if (CONFIG(sensorChartMode) == SC_DETAILED_RPM) {
			scAddData(currentAngle, instantRpm);
		} else {
			scAddData(currentAngle, instantRpm / instantRpmValue[prevIndex]);
		}
#endif /* EFI_SENSOR_CHART */
	}
}

bool TriggerState::isValidIndex(TriggerWaveform *triggerShape) const {
	return currentCycle.current_index < triggerShape->getSize();
}

static trigger_wheel_e eventIndex[6] = { T_PRIMARY, T_PRIMARY, T_SECONDARY, T_SECONDARY, T_CHANNEL_3, T_CHANNEL_3 };
static trigger_value_e eventType[6] = { TV_FALL, TV_RISE, TV_FALL, TV_RISE, TV_FALL, TV_RISE };

#define getCurrentGapDuration(nowNt) \
	(isFirstEvent ? 0 : (nowNt) - toothed_previous_time)

#if EFI_UNIT_TEST
#define PRINT_INC_INDEX 		if (printTriggerDebug) {\
		printf("nextTriggerEvent index=%d\r\n", currentCycle.current_index); \
		}
#else
#define PRINT_INC_INDEX {}
#endif /* EFI_UNIT_TEST */

#define nextTriggerEvent() \
 { \
	uint32_t prevTime = currentCycle.timeOfPreviousEventNt[triggerWheel]; \
	if (prevTime != 0) { \
		/* even event - apply the value*/ \
		currentCycle.totalTimeNt[triggerWheel] += (nowNt - prevTime); \
		currentCycle.timeOfPreviousEventNt[triggerWheel] = 0; \
	} else { \
		/* odd event - start accumulation */ \
		currentCycle.timeOfPreviousEventNt[triggerWheel] = nowNt; \
	} \
	if (engineConfiguration->useOnlyRisingEdgeForTrigger) {currentCycle.current_index++;} \
	currentCycle.current_index++; \
	PRINT_INC_INDEX; \
}

#define considerEventForGap() (!triggerShape->useOnlyPrimaryForSync || isPrimary)

#define needToSkipFall(type) ((!triggerShape->gapBothDirections) && (( triggerShape->useRiseEdge) && (type != TV_RISE)))
#define needToSkipRise(type) ((!triggerShape->gapBothDirections) && ((!triggerShape->useRiseEdge) && (type != TV_FALL)))

int TriggerState::getCurrentIndex() const {
	return currentCycle.current_index;
}

void TriggerCentral::validateCamVvtCounters() {
	// micro-optimized 'totalRevolutionCounter % 256'
	int camVvtValidationIndex = triggerState.getTotalRevolutionCounter() & 0xFF;
	if (camVvtValidationIndex == 0) {
		vvtCamCounter = 0;
	} else if (camVvtValidationIndex == 0xFE && vvtCamCounter < 60) {
		// magic logic: we expect at least 60 CAM/VVT events for each 256 trigger cycles, otherwise throw a code
		warning(OBD_Camshaft_Position_Sensor_Circuit_Range_Performance, "no CAM signals");
	}
}

void TriggerState::incrementTotalEventCounter() {
	totalRevolutionCounter++;
}

bool TriggerState::isEvenRevolution() const {
	return totalRevolutionCounter & 1;
}

bool TriggerState::validateEventCounters(TriggerWaveform *triggerShape) const {
	bool isDecodingError = false;
	for (int i = 0;i < PWM_PHASE_MAX_WAVE_PER_PWM;i++) {
		isDecodingError |= (currentCycle.eventCount[i] != triggerShape->expectedEventCount[i]);
	}


#if EFI_UNIT_TEST
			printf("sync point: isDecodingError=%d\r\n", isDecodingError);
			if (isDecodingError) {
				for (int i = 0;i < PWM_PHASE_MAX_WAVE_PER_PWM;i++) {
					printf("count: cur=%d exp=%d\r\n", currentCycle.eventCount[i],  triggerShape->expectedEventCount[i]);
				}
			}
#endif /* EFI_UNIT_TEST */

	return isDecodingError;
}

void TriggerState::onShaftSynchronization(const TriggerStateCallback triggerCycleCallback,
		efitick_t nowNt, TriggerWaveform *triggerShape) {


	if (triggerCycleCallback) {
		triggerCycleCallback(this);
	}

	startOfCycleNt = nowNt;
	resetCurrentCycleState();
	incrementTotalEventCounter();
	totalEventCountBase += triggerShape->getSize();

#if EFI_UNIT_TEST
	if (printTriggerDebug) {
		printf("onShaftSynchronization index=%d %d\r\n",
				currentCycle.current_index,
				totalRevolutionCounter);
	}
#endif /* EFI_UNIT_TEST */
}

/**
 * @brief Trigger decoding happens here
 * This method is invoked every time we have a fall or rise on one of the trigger sensors.
 * This method changes the state of trigger_state_s data structure according to the trigger event
 * @param signal type of event which just happened
 * @param nowNt current time
 */
void TriggerState::decodeTriggerEvent(TriggerWaveform *triggerShape, const TriggerStateCallback triggerCycleCallback,
		TriggerStateListener * triggerStateListener,
		trigger_event_e const signal, efitick_t nowNt DECLARE_CONFIG_PARAMETER_SUFFIX) {
	ScopePerf perf(PE::DecodeTriggerEvent, static_cast<uint8_t>(signal));
	
	if (nowNt - previousShaftEventTimeNt > NT_PER_SECOND) {
		/**
		 * We are here if there is a time gap between now and previous shaft event - that means the engine is not running.
		 * That means we have lost synchronization since the engine is not running :)
		 */
		setShaftSynchronized(false);
		if (triggerStateListener) {
			triggerStateListener->OnTriggerSynchronizationLost();
		}
	}
	previousShaftEventTimeNt = nowNt;


	bool useOnlyRisingEdgeForTrigger = CONFIG(useOnlyRisingEdgeForTrigger);


	efiAssertVoid(CUSTOM_TRIGGER_UNEXPECTED, signal <= SHAFT_3RD_RISING, "unexpected signal");

	trigger_wheel_e triggerWheel = eventIndex[signal];
	trigger_value_e type = eventType[signal];

	if (!useOnlyRisingEdgeForTrigger && curSignal == prevSignal) {
		orderingErrorCounter++;
	}

	prevSignal = curSignal;
	curSignal = signal;

	currentCycle.eventCount[triggerWheel]++;

	efiAssertVoid(CUSTOM_OBD_93, toothed_previous_time <= nowNt, "toothed_previous_time after nowNt");

	efitick_t currentDurationLong = getCurrentGapDuration(nowNt);

	/**
	 * For performance reasons, we want to work with 32 bit values. If there has been more then
	 * 10 seconds since previous trigger event we do not really care.
	 */
	toothDurations[0] =
			currentDurationLong > 10 * NT_PER_SECOND ? 10 * NT_PER_SECOND : currentDurationLong;

	bool isPrimary = triggerWheel == T_PRIMARY;

	if (needToSkipFall(type) || needToSkipRise(type) || (!considerEventForGap())) {
#if EFI_UNIT_TEST
		if (printTriggerDebug) {
			printf("%s isLessImportant %s now=%d index=%d\r\n",
					getTrigger_type_e(engineConfiguration->trigger.type),
					getTrigger_event_e(signal),
					(int)nowNt,
					currentCycle.current_index);
		}
#endif /* EFI_UNIT_TEST */

		/**
		 * For less important events we simply increment the index.
		 */
		nextTriggerEvent()
		;
	} else {

#if EFI_UNIT_TEST
		if (printTriggerDebug) {
			printf("%s event %s %d\r\n",
					getTrigger_type_e(engineConfiguration->trigger.type),
					getTrigger_event_e(signal),
					nowNt);
		}
#endif /* EFI_UNIT_TEST */

		isFirstEvent = false;
// todo: skip a number of signal from the beginning

#if EFI_PROD_CODE
//	scheduleMsg(&logger, "from %.2f to %.2f %d %d", triggerConfig->syncRatioFrom, triggerConfig->syncRatioTo, toothDurations[0], shaftPositionState->toothDurations[1]);
//	scheduleMsg(&logger, "ratio %.2f", 1.0 * toothDurations[0]/ shaftPositionState->toothDurations[1]);
#else
		if (printTriggerDebug) {
			printf("ratio %.2f: current=%d previous=%d\r\n", 1.0 * toothDurations[0] / toothDurations[1],
					toothDurations[0], toothDurations[1]);
		}
#endif

		bool isSynchronizationPoint;
		bool wasSynchronized = shaft_is_synchronized;

		DISPLAY_STATE(Trigger_State)
		DISPLAY_TEXT(Current_Gap);
		DISPLAY(DISPLAY_FIELD(currentGap));
		DISPLAY_TEXT(EOL);

		DISPLAY_STATE(Trigger_Central)
		DISPLAY(DISPLAY_CONFIG(TRIGGERINPUTPINS1));
		DISPLAY_TEXT("Trigger 1: Fall");
		DISPLAY(DISPLAY_FIELD(HWEVENTCOUNTERS1));
		DISPLAY_TEXT(", Rise");
		DISPLAY(DISPLAY_FIELD(HWEVENTCOUNTERS2));
		DISPLAY_TEXT(EOL);

		DISPLAY(DISPLAY_CONFIG(TRIGGERINPUTPINS2));
		DISPLAY_TEXT("Trigger 2: Fall");
		DISPLAY(DISPLAY_FIELD(HWEVENTCOUNTERS3));
		DISPLAY_TEXT(", Rise");
		DISPLAY(DISPLAY_FIELD(HWEVENTCOUNTERS4));
		DISPLAY_TEXT(EOL);

		DISPLAY_TEXT(VVT_1);
		DISPLAY(DISPLAY_CONFIG(CAMINPUTS1));
		DISPLAY(DISPLAY_FIELD(vvtEventRiseCounter));
		DISPLAY(DISPLAY_FIELD(vvtEventFallCounter));
		DISPLAY(DISPLAY_FIELD(vvtCamCounter));

		if (triggerShape->isSynchronizationNeeded) {

			currentGap = 1.0 * toothDurations[0] / toothDurations[1];

			if (CONFIG(debugMode) == DBG_TRIGGER_COUNTERS) {
#if EFI_TUNER_STUDIO
				tsOutputChannels.debugFloatField6 = currentGap;
				tsOutputChannels.debugIntField3 = currentCycle.current_index;
#endif /* EFI_TUNER_STUDIO */
			}

			bool isSync = true;
			for (int i = 0;i<GAP_TRACKING_LENGTH;i++) {
				bool isGapCondition = cisnan(triggerShape->syncronizationRatioFrom[i]) || (toothDurations[i] > toothDurations[i + 1] * triggerShape->syncronizationRatioFrom[i]
					&& toothDurations[i] < toothDurations[i + 1] * triggerShape->syncronizationRatioTo[i]);

				isSync &= isGapCondition;

			}
			isSynchronizationPoint = isSync;
			if (isSynchronizationPoint) {
				enginePins.debugTriggerSync.setValue(1);
			}


			/**
			 * todo: technically we can afford detailed logging even with 60/2 as long as low RPM
			 * todo: figure out exact threshold as a function of RPM and tooth count?
			 * Open question what is 'triggerShape->getSize()' for 60/2 is it 58 or 58*2 or 58*4?
			 */
			bool silentTriggerError = triggerShape->getSize() > 40 && CONFIG(silentTriggerError);

#if EFI_UNIT_TEST
			actualSynchGap = 1.0 * toothDurations[0] / toothDurations[1];
#endif /* EFI_UNIT_TEST */

#if EFI_PROD_CODE || EFI_SIMULATOR
			if (CONFIG(verboseTriggerSynchDetails) || (someSortOfTriggerError && !silentTriggerError)) {
				for (int i = 0;i<GAP_TRACKING_LENGTH;i++) {
					float ratioFrom = triggerShape->syncronizationRatioFrom[i];
					if (cisnan(ratioFrom)) {
						// we do not track gap at this depth
						continue;
					}

					float gap = 1.0 * toothDurations[i] / toothDurations[i + 1];
					if (cisnan(gap)) {
						scheduleMsg(logger, "index=%d NaN gap, you have noise issues?",
								i);
					} else {
						scheduleMsg(logger, "rpm=%d time=%d index=%d: gap=%.3f expected from %.3f to %.3f error=%s",
							GET_RPM(),
							/* cast is needed to make sure we do not put 64 bit value to stack*/ (int)getTimeNowSeconds(),
							i,
							gap,
							ratioFrom,
							triggerShape->syncronizationRatioTo[i],
							boolToString(someSortOfTriggerError));
					}
				}
			}
#else
			if (printTriggerDebug) {
				float gap = 1.0 * toothDurations[0] / toothDurations[1];
				for (int i = 0;i<GAP_TRACKING_LENGTH;i++) {
					float gap = 1.0 * toothDurations[i] / toothDurations[i + 1];
					print("index=%d: gap=%.2f expected from %.2f to %.2f error=%s\r\n",
							i,
							gap,
							triggerShape->syncronizationRatioFrom[i],
							triggerShape->syncronizationRatioTo[i],
							boolToString(someSortOfTriggerError));
				}
			}


#endif /* EFI_PROD_CODE */
			enginePins.debugTriggerSync.setValue(0);

		} else {
			/**
			 * We are here in case of a wheel without synchronization - we just need to count events,
			 * synchronization point simply happens once we have the right number of events
			 *
			 * in case of noise the counter could be above the expected number of events, that's why 'more or equals' and not just 'equals'
			 */

#if EFI_UNIT_TEST
			if (printTriggerDebug) {
				printf("sync=%d index=%d size=%d\r\n",
					shaft_is_synchronized,
					currentCycle.current_index,
					triggerShape->getSize());
			}
#endif /* EFI_UNIT_TEST */
			unsigned int endOfCycleIndex = triggerShape->getSize() - (CONFIG(useOnlyRisingEdgeForTrigger) ? 2 : 1);


			isSynchronizationPoint = !shaft_is_synchronized || (currentCycle.current_index >= endOfCycleIndex);

#if EFI_UNIT_TEST
			if (printTriggerDebug) {
				printf("isSynchronizationPoint=%d index=%d size=%d\r\n",
						isSynchronizationPoint,
						currentCycle.current_index,
						triggerShape->getSize());
			}
#endif /* EFI_UNIT_TEST */

		}

#if EFI_UNIT_TEST
		if (printTriggerDebug) {
			printf("%s isSynchronizationPoint=%d index=%d %s\r\n",
					getTrigger_type_e(engineConfiguration->trigger.type),
					isSynchronizationPoint, currentCycle.current_index,
					getTrigger_event_e(signal));
		}
#endif /* EFI_UNIT_TEST */

		if (isSynchronizationPoint) {

			if (triggerStateListener) {
				triggerStateListener->OnTriggerSyncronization(wasSynchronized);
			}

			setShaftSynchronized(true);
			// this call would update duty cycle values
			nextTriggerEvent()
			;

			onShaftSynchronization(triggerCycleCallback, nowNt, triggerShape);

		} else {	/* if (!isSynchronizationPoint) */
			nextTriggerEvent()
			;
		}

		for (int i = GAP_TRACKING_LENGTH; i > 0; i--) {
			toothDurations[i] = toothDurations[i - 1];
		}

		toothed_previous_time = nowNt;
	}
	if (!isValidIndex(triggerShape) && triggerStateListener) {
		triggerStateListener->OnTriggerInvalidIndex(currentCycle.current_index);
	}
	if (someSortOfTriggerError) {
		if (getTimeNowNt() - lastDecodingErrorTime > NT_PER_SECOND) {
			someSortOfTriggerError = false;
		}
	}


	// Needed for early instant-RPM detection
	if (triggerStateListener) {
		triggerStateListener->OnTriggerStateProperState(nowNt);
	}
}

static void onFindIndexCallback(TriggerState *state) {
	for (int i = 0; i < PWM_PHASE_MAX_WAVE_PER_PWM; i++) {
		// todo: that's not the best place for this intermediate data storage, fix it!
		state->expectedTotalTime[i] = state->currentCycle.totalTimeNt[i];
	}
}

/**
 * Trigger shape is defined in a way which is convenient for trigger shape definition
 * On the other hand, trigger decoder indexing begins from synchronization event.
 *
 * This function finds the index of synchronization event within TriggerWaveform
 */
uint32_t TriggerState::findTriggerZeroEventIndex(TriggerWaveform * shape,
		trigger_config_s const*triggerConfig DECLARE_CONFIG_PARAMETER_SUFFIX) {
	UNUSED(triggerConfig);
#if EFI_PROD_CODE
	efiAssert(CUSTOM_ERR_ASSERT, getCurrentRemainingStack() > 128, "findPos", -1);
#endif


	resetTriggerState();

	if (shape->shapeDefinitionError) {
		return 0;
	}


	// todo: should this variable be declared 'static' to reduce stack usage?
	TriggerStimulatorHelper helper;

	uint32_t syncIndex = helper.findTriggerSyncPoint(shape, this PASS_CONFIG_PARAMETER_SUFFIX);
	if (syncIndex == EFI_ERROR_CODE) {
		return syncIndex;
	}
	efiAssert(CUSTOM_ERR_ASSERT, getTotalRevolutionCounter() == 1, "findZero_revCounter", EFI_ERROR_CODE);

#if EFI_UNIT_TEST
	if (printTriggerDebug) {
		printf("findTriggerZeroEventIndex: syncIndex located %d!\r\n", syncIndex);
	}
#endif /* EFI_UNIT_TEST */

	/**
	 * Now that we have just located the synch point, we can simulate the whole cycle
	 * in order to calculate expected duty cycle
	 *
	 * todo: add a comment why are we doing '2 * shape->getSize()' here?
	 */

	helper.assertSyncPositionAndSetDutyCycle(onFindIndexCallback, syncIndex, this, shape PASS_CONFIG_PARAMETER_SUFFIX);

	return syncIndex % shape->getSize();
}

void initTriggerDecoderLogger(Logging *sharedLogger) {
	logger = sharedLogger;
}

 void initTriggerDecoder(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
#if EFI_GPIO_HARDWARE
	enginePins.triggerDecoderErrorPin.initPin("trg_err", CONFIG(triggerErrorPin),
			&CONFIG(triggerErrorPinMode));
#endif /* EFI_GPIO_HARDWARE */
}

#endif /* EFI_SHAFT_POSITION_INPUT */
