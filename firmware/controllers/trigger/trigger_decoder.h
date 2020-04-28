/**
 * @file	trigger_decoder.h
 *
 * @date Dec 24, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "trigger_structure.h"
#include "engine_configuration.h"
#include "trigger_state_generated.h"

class TriggerState;

struct TriggerStateListener {
#if EFI_SHAFT_POSITION_INPUT
	virtual void OnTriggerStateProperState(efitick_t nowNt) = 0;
	virtual void OnTriggerSyncronization(bool wasSynchronized) = 0;
	virtual void OnTriggerInvalidIndex(int currentIndex) = 0;
	virtual void OnTriggerSynchronizationLost() = 0;
#endif
};

typedef void (*TriggerStateCallback)(TriggerState *);

typedef struct {
	/**
	 * index within trigger revolution, from 0 to trigger event count
	 */
	uint32_t current_index;
	/**
	 * Number of actual events of each channel within current trigger cycle, these
	 * values are used to detect trigger signal errors.
	 * see TriggerWaveform
	 */
	uint32_t eventCount[PWM_PHASE_MAX_WAVE_PER_PWM];
	/**
	 * This array is used to calculate duty cycle of each trigger channel.
	 * Current implementation is a bit funny - it does not really consider if an event
	 * is a rise or a fall, it works based on the event order within synchronization cycle.
	 *
	 * 32 bit value is good enough here, overflows will happen but they would work just fine.
	 */
	uint32_t timeOfPreviousEventNt[PWM_PHASE_MAX_WAVE_PER_PWM];
	/**
	 * Here we accumulate the amount of time this signal was ON within current trigger cycle
	 */
	uint32_t totalTimeNt[PWM_PHASE_MAX_WAVE_PER_PWM];
} current_cycle_state_s;

/**
 * @see TriggerWaveform for trigger wheel shape definition
 */
class TriggerState : public trigger_state_s {
public:
	TriggerState();
	/**
	 * current trigger processing index, between zero and #size
	 */
	int getCurrentIndex() const;
	int getTotalRevolutionCounter() const;
	/**
	 * this is important for crank-based virtual trigger and VVT magic
	 */
	bool isEvenRevolution() const;
	void incrementTotalEventCounter();
	efitime_t getTotalEventCounter() const;

	void decodeTriggerEvent(TriggerWaveform *triggerShape, const TriggerStateCallback triggerCycleCallback,
			TriggerStateListener * triggerStateListener,
			trigger_event_e const signal, efitime_t nowUs DECLARE_CONFIG_PARAMETER_SUFFIX);

	bool validateEventCounters(TriggerWaveform *triggerShape) const;
	void onShaftSynchronization(const TriggerStateCallback triggerCycleCallback,
			efitick_t nowNt, TriggerWaveform *triggerShape);


	bool isValidIndex(TriggerWaveform *triggerShape) const;
	float getTriggerDutyCycle(int index);

	/**
	 * TRUE if we know where we are
	 */
	bool shaft_is_synchronized;
	efitick_t mostRecentSyncTime;
	volatile efitick_t previousShaftEventTimeNt;

	void setTriggerErrorState();

	efitick_t lastDecodingErrorTime;
	// the boolean flag is a performance optimization so that complex comparison is avoided if no error
	bool someSortOfTriggerError;

	/**
	 * current duration at index zero and previous durations are following
	 */
	uint32_t toothDurations[GAP_TRACKING_LENGTH + 1];

	efitick_t toothed_previous_time;

	current_cycle_state_s currentCycle;

	int expectedTotalTime[PWM_PHASE_MAX_WAVE_PER_PWM];

	/**
	 * how many times since ECU reboot we had unexpected number of teeth in trigger cycle
	 */
	uint32_t totalTriggerErrorCounter;
	uint32_t orderingErrorCounter;

	void resetTriggerState();
	void setShaftSynchronized(bool value);

	/**
	 * this is start of real trigger cycle
	 * for virtual double trigger see timeAtVirtualZeroNt
	 */
	efitick_t startOfCycleNt;

	uint32_t findTriggerZeroEventIndex(TriggerWaveform * shape, trigger_config_s const*triggerConfig
			DECLARE_CONFIG_PARAMETER_SUFFIX);

private:
	void resetCurrentCycleState();

	trigger_event_e curSignal;
	trigger_event_e prevSignal;
	int64_t totalEventCountBase;
	uint32_t totalRevolutionCounter;
	bool isFirstEvent;
};

// we only need 90 degrees of events so /4 or maybe even /8 should work?
#define PRE_SYNC_EVENTS (PWM_PHASE_MAX_COUNT / 4)


/**
 * the reason for sub-class is simply to save RAM but not having statistics in the trigger initialization instance
 */
class TriggerStateWithRunningStatistics : public TriggerState {
public:
	TriggerStateWithRunningStatistics();
	float instantRpm = 0;
	/**
	 * timestamp of each trigger wheel tooth
	 */
	uint32_t timeOfLastEvent[PWM_PHASE_MAX_COUNT];

	int spinningEventIndex = 0;
	// todo: change the implementation to reuse 'timeOfLastEvent'
	uint32_t spinningEvents[PRE_SYNC_EVENTS];
	/**
	 * instant RPM calculated at this trigger wheel tooth
	 */
	float instantRpmValue[PWM_PHASE_MAX_COUNT];
	/**
	 * Stores last non-zero instant RPM value to fix early instability
	 */
	float prevInstantRpmValue = 0;
	void movePreSynchTimestamps(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	float calculateInstantRpm(int *prevIndex, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);
#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT
	void runtimeStatistics(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);
#endif
	/**
	 * Update timeOfLastEvent[] on every trigger event - even without synchronization
	 * Needed for early spin-up RPM detection.
	 */
	void setLastEventTimeForInstantRpm(efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);
};

angle_t getEngineCycle(operation_mode_e operationMode);

class Engine;

void initTriggerDecoder(DECLARE_ENGINE_PARAMETER_SIGNATURE);
void initTriggerDecoderLogger(Logging *sharedLogger);

bool isTriggerDecoderError(DECLARE_ENGINE_PARAMETER_SIGNATURE);

void calculateTriggerSynchPoint(TriggerWaveform *shape, TriggerState *state DECLARE_ENGINE_PARAMETER_SUFFIX);

