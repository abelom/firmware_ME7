/**
 * @file	trigger_structure.h
 *
 * @date Dec 22, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "state_sequence.h"
#include "globalaccess.h"
#include "engine_configuration_generated_structures.h"

#define FOUR_STROKE_ENGINE_CYCLE 720

#if EFI_ENABLE_ASSERTS
#define assertAngleRange(angle, msg, code) if (angle > 10000000 || angle < -10000000) { firmwareError(code, "angle range %s %.2f", msg, angle);angle = 0;}
#else
#define assertAngleRange(angle, msg, code) {}
#endif

/**
 * @brief Shifts angle into the [0..720) range for four stroke and [0..360) for two stroke
 * I guess this implementation would be faster than 'angle % engineCycle'
 */
#define fixAngle2(angle, msg, code, engineCycle)			   	    	    \
	{																		\
   	    if (cisnan(angle)) {                                                \
		   firmwareError(CUSTOM_ERR_ANGLE, "aNaN%s", msg);                  \
		   angle = 0;                                                       \
	    }                                                                   \
		assertAngleRange(angle, msg, code);	   					            \
		float engineCycleDurationLocalCopy = engineCycle;	                \
		/* todo: split this method into 'fixAngleUp' and 'fixAngleDown'*/   \
		/*       as a performance optimization?*/                           \
		while (angle < 0)                       							\
			angle += engineCycleDurationLocalCopy;   						\
			/* todo: would 'if' work as good as 'while'? */                 \
		while (angle >= engineCycleDurationLocalCopy)						\
			angle -= engineCycleDurationLocalCopy;   						\
	}

/**
 * This structure defines an angle position in relation to specific tooth within trigger shape
 */
class event_trigger_position_s {
public:
	size_t triggerEventIndex = 0;

	angle_t angleOffsetFromTriggerEvent = 0;

	void setAngle(angle_t angle DECLARE_ENGINE_PARAMETER_SUFFIX);
};

#define TRIGGER_CHANNEL_COUNT 3

class trigger_shape_helper {
public:
	trigger_shape_helper();

	SingleChannelStateSequence channels[TRIGGER_CHANNEL_COUNT];
private:
	pin_state_t pinStates[TRIGGER_CHANNEL_COUNT][PWM_PHASE_MAX_COUNT];
};

class Engine;
class TriggerState;

#define GAP_TRACKING_LENGTH 4

/**
 * @brief Trigger shape has all the fields needed to describe and decode trigger signal.
 * @see TriggerState for trigger decoder state which works based on this trigger shape model
 */
class TriggerWaveform {
public:
	TriggerWaveform();
	void initializeTriggerWaveform(Logging *logger, operation_mode_e ambiguousOperationMode,
			bool useOnlyRisingEdgeForTrigger, const trigger_config_s *triggerConfig);
	void findTriggerPosition(event_trigger_position_s *position,
			angle_t angle DEFINE_CONFIG_PARAM(angle_t, globalTriggerAngleOffset));
	void setShapeDefinitionError(bool value);

	/**
	 * Simplest trigger shape does not require any synchronization - for example if there is only
	 * one primary channel tooth each raising (or falling depending on configuration) front would synchronize
	 */
	bool isSynchronizationNeeded;
	/**
	 * special case for triggers which do not provide exact TDC location
	 * For example pick-up in distributor with mechanical ignition firing order control.
	 */
	bool shapeWithoutTdc = false;
	/**
	 * this flag tells us if we should ignore events on second input channel
	 * that's the way to ignore noise from the disconnected wire
	 */
	bool needSecondTriggerInput;
	/**
	 * true value here means that we do not have a valid trigger configuration
	 */
	bool shapeDefinitionError;

	/**
	 * https://github.com/rusefi/rusefi/issues/898
	 * User can choose for example Miata trigger which is not compatible with useOnlyRisingEdgeForTrigger option
	 * Such contradictory configuration causes a very hard to identify issue and for the sake of usability it's better to
	 * just crash with a very visible fatal error
	 *
	 * One day a nicer implementation could be simply ignoring 'useOnlyRisingEdgeForTrigger' in case of 'bothFrontsRequired'
	 */
	bool bothFrontsRequired;

	/**
	 * this variable is incremented after each trigger shape redefinition
	 * See also
	 */
	int version = 0;

	/**
	 * duty cycle for each individual trigger channel
	 */
	float expectedDutyCycle[PWM_PHASE_MAX_WAVE_PER_PWM];

	/**
	 * These angles are in event coordinates - with synchronization point located at angle zero.
	 * These values are pre-calculated for performance reasons.
	 */
	angle_t eventAngles[PWM_PHASE_MAX_COUNT];
	/**
	 * this cache allows us to find a close-enough (with one degree precision) trigger wheel index by
	 * given angle with fast constant speed. That's a performance optimization for event scheduling.
	 */
	int triggerIndexByAngle[720];


	/**
	 * Depending on trigger shape, we use betweeb one and three previous gap ranges to detect synchronizaiton.
	 *
	 * Usually second or third gap is not needed, but some crazy triggers like 36-2-2-2 require two consecutive
	 * gaps ratios to sync
	 */

	float syncronizationRatioFrom[GAP_TRACKING_LENGTH];
	float syncronizationRatioTo[GAP_TRACKING_LENGTH];


	/**
	 * used by NoiselessTriggerDecoder (See TriggerCentral::handleShaftSignal())
	 */
	int syncRatioAvg;


	/**
	 * Trigger indexes within trigger cycle are counted from synchronization point, and all
	 * engine processes are defined in angles from TDC.
	 *
	 * That's the angle distance from trigger event #0 and actual engine TDC
	 *
	 * see also globalTriggerAngleOffset
	 */
	angle_t tdcPosition;

	/**
	 * In case of a multi-channel trigger, do we want to sync based on primary channel only?
	 * See also gapBothDirections
	 */
	bool useOnlyPrimaryForSync;
	/**
	 * Should we use falls or rises for gap ratio detection?
	 */
	bool useRiseEdge;
	/**
	 * This is about selecting signal edges within particular trigger channels.
	 * Should we measure gaps with both fall and rise signal edges?
	 * See also useOnlyPrimaryForSync
	 */
	bool gapBothDirections;

	void calculateExpectedEventCounts(bool useOnlyRisingEdgeForTrigger);

	/**
	 * This is used for signal validation
	 */
	uint32_t expectedEventCount[PWM_PHASE_MAX_WAVE_PER_PWM];

#if EFI_UNIT_TEST
	/**
	 * These signals are used for trigger export only
	 */
	int triggerSignals[PWM_PHASE_MAX_COUNT];
#endif

	MultiChannelStateSequence wave;

	// todo: add a runtime validation which would verify that this field was set properly
	// todo: maybe even automate this flag calculation?
	pin_state_t initialState[PWM_PHASE_MAX_WAVE_PER_PWM];

	bool isRiseEvent[PWM_PHASE_MAX_COUNT];
	/**
	 * this table translates trigger definition index into 'front-only' index. This translation is not so trivial
	 * in case of a multi-channel signal with overlapping waves, for example Ford Aspire/Mitsubishi
	 */
	int riseOnlyIndexes[PWM_PHASE_MAX_COUNT];

	/**
	 * This is a pretty questionable option which is considered by 'addEvent' method
	 */
	bool invertOnAdd;
	/**
	 * Total count of shaft events per CAM or CRANK shaft revolution.
	 * TODO this should be migrated to CRANKshaft revolution, this would go together
	 * this variable is public for performance reasons (I want to avoid costs of method if it's not inlined)
	 * but name is supposed to hint at the fact that decoders should not be assigning to it
	 * Please use "getTriggerSize()" macro or "getSize()" method to read this value
	 */
	unsigned int privateTriggerDefinitionSize;

	bool useOnlyRisingEdgeForTriggerTemp;

	/* 0..1 angle range */
	void addEvent(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const state);
	/* 0..720 angle range
	 * Deprecated!
	 */
	void addEvent720(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const state);

	/**
	 * This version of 'addEvent...' family considers the angle duration of operationMode in this trigger
	 * For example, 0..180 for FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR
	 */
	void addEventAngle(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const state);

	/* 0..720 angle range
	 * Deprecated?
	 */
	void addEventClamped(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const stateParam, float filterLeft, float filterRight);
	operation_mode_e getOperationMode() const;

	void initialize(operation_mode_e operationMode);
	void setTriggerSynchronizationGap(float syncRatio);
	void setTriggerSynchronizationGap3(int index, float syncRatioFrom, float syncRatioTo);
	void setTriggerSynchronizationGap2(float syncRatioFrom, float syncRatioTo);
	void setSecondTriggerSynchronizationGap(float syncRatio);
	void setSecondTriggerSynchronizationGap2(float syncRatioFrom, float syncRatioTo);
	void setThirdTriggerSynchronizationGap(float syncRatio);
	/**
	 * this one is per CRANKshaft revolution
	 */
	size_t getLength() const;
	size_t getSize() const;

	int getTriggerWaveformSynchPointIndex() const;
	void prepareShape();

	/**
	 * This private method should only be used to prepare the array of pre-calculated values
	 * See eventAngles array
	 */
	angle_t getAngle(int phaseIndex) const;

	angle_t getCycleDuration() const;

	/**
	 * index of synchronization event within TriggerWaveform
	 * See findTriggerZeroEventIndex()
	 */
	int triggerShapeSynchPointIndex;
private:
	trigger_shape_helper h;

	int findAngleIndex(float angle) const;

	/**
	 * Working buffer for 'wave' instance
	 * Values are in the 0..1 range
	 */
	float switchTimesBuffer[PWM_PHASE_MAX_COUNT];
	/**
	 * These angles are in trigger DESCRIPTION coordinates - i.e. the way you add events while declaring trigger shape
	 */
	angle_t getSwitchAngle(int index) const;

	/**
	 * This variable is used to confirm that events are added in the right order.
	 * todo: this variable is probably not needed, could be reimplemented by accessing by index
	 */
	angle_t previousAngle;
	/**
	 * this is part of performance optimization
	 */
	operation_mode_e operationMode;
};

void setToothedWheelConfiguration(TriggerWaveform *s, int total, int skipped, operation_mode_e operationMode);

#define TRIGGER_WAVEFORM(x) ENGINE(triggerCentral.triggerShape.x)

#define getTriggerSize() TRIGGER_WAVEFORM(privateTriggerDefinitionSize)
