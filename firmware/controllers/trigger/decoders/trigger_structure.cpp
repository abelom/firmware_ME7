/**
 * @file	trigger_structure.cpp
 *
 * @date Jan 20, 2014
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
#include "engine.h"
#include "trigger_bmw.h"
#include "trigger_chrysler.h"
#include "trigger_gm.h"
#include "trigger_nissan.h"
#include "trigger_mazda.h"
#include "trigger_misc.h"
#include "trigger_mitsubishi.h"
#include "trigger_subaru.h"
#include "trigger_structure.h"
#include "trigger_toyota.h"
#include "trigger_renix.h"
#include "trigger_rover.h"
#include "trigger_honda.h"
#include "trigger_vw.h"
#include "trigger_universal.h"

#if EFI_SENSOR_CHART
#include "sensor_chart.h"
#endif /* EFI_SENSOR_CHART */

#include "engine_configuration.h"
		extern persistent_config_container_s persistentState;

EXTERN_ENGINE;

void event_trigger_position_s::setAngle(angle_t angle DECLARE_ENGINE_PARAMETER_SUFFIX) {
	TRIGGER_WAVEFORM(findTriggerPosition(this, angle PASS_CONFIG_PARAM(engineConfiguration->globalTriggerAngleOffset)));
}

trigger_shape_helper::trigger_shape_helper() {
	memset(&pinStates, 0, sizeof(pinStates));
	for (int channelIndex = 0; channelIndex < TRIGGER_CHANNEL_COUNT; channelIndex++) {
		channels[channelIndex].init(pinStates[channelIndex]);
	}
}

TriggerWaveform::TriggerWaveform() :
		wave(switchTimesBuffer, NULL) {
	initialize(OM_NONE);
	wave.channels = h.channels;

	memset(triggerIndexByAngle, 0, sizeof(triggerIndexByAngle));
}

void TriggerWaveform::initialize(operation_mode_e operationMode) {
	isSynchronizationNeeded = true; // that's default value
	bothFrontsRequired = false;
	needSecondTriggerInput = false;
	shapeWithoutTdc = false;
	memset(expectedDutyCycle, 0, sizeof(expectedDutyCycle));
	memset(eventAngles, 0, sizeof(eventAngles));
//	memset(triggerIndexByAngle, 0, sizeof(triggerIndexByAngle));

	setTriggerSynchronizationGap(2);
	for (int gapIndex = 1; gapIndex < GAP_TRACKING_LENGTH ; gapIndex++) {
		// NaN means do not use this gap ratio
		setTriggerSynchronizationGap3(gapIndex, NAN, 100000);
	}

	tdcPosition = 0;
	shapeDefinitionError = useOnlyPrimaryForSync = false;
	useRiseEdge = true;
	gapBothDirections = false;

	invertOnAdd = false;

	this->operationMode = operationMode;
	privateTriggerDefinitionSize = 0;
	triggerShapeSynchPointIndex = 0;
	memset(initialState, 0, sizeof(initialState));
	memset(switchTimesBuffer, 0, sizeof(switchTimesBuffer));
	memset(expectedEventCount, 0, sizeof(expectedEventCount));
	wave.reset();
	previousAngle = 0;
	memset(riseOnlyIndexes, 0, sizeof(riseOnlyIndexes));
	memset(isRiseEvent, 0, sizeof(isRiseEvent));
#if EFI_UNIT_TEST
	memset(&triggerSignals, 0, sizeof(triggerSignals));
#endif
}

size_t TriggerWaveform::getSize() const {
	return privateTriggerDefinitionSize;
}

int TriggerWaveform::getTriggerWaveformSynchPointIndex() const {
	return triggerShapeSynchPointIndex;
}

/**
 * physical primary trigger duration
 */
angle_t TriggerWaveform::getCycleDuration() const {
	switch (operationMode) {
	case FOUR_STROKE_THREE_TIMES_CRANK_SENSOR:
		return 120;
	case FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR:
		return 180;
	case FOUR_STROKE_CRANK_SENSOR:
	case TWO_STROKE:
		return 360;
	default:
		return 720;
	}
}

/**
 * Trigger event count equals engine cycle event count if we have a cam sensor.
 * Two trigger cycles make one engine cycle in case of a four stroke engine If we only have a cranksensor.
 */
size_t TriggerWaveform::getLength() const {
	/**
	 * 4 for FOUR_STROKE_SYMMETRICAL_CRANK_SENSOR
	 * 2 for FOUR_STROKE_CRANK_SENSOR
	 * 1 otherwise
	 */
	int multiplier = getEngineCycle(operationMode) / getCycleDuration();
	return multiplier * getSize();
}

angle_t TriggerWaveform::getAngle(int index) const {
	// todo: why is this check here? looks like the code below could be used universally
	if (operationMode == FOUR_STROKE_CAM_SENSOR) {
		return getSwitchAngle(index);
	}
	/**
	 * FOUR_STROKE_CRANK_SENSOR magic:
	 * We have two crank shaft revolutions for each engine cycle
	 * See also trigger_central.cpp
	 * See also getEngineCycleEventCount()
	 */
	efiAssert(CUSTOM_ERR_ASSERT, privateTriggerDefinitionSize != 0, "shapeSize=0", NAN);
	int crankCycle = index / privateTriggerDefinitionSize;
	int remainder = index % privateTriggerDefinitionSize;

	return getCycleDuration() * crankCycle + getSwitchAngle(remainder);
}

void TriggerWaveform::addEventClamped(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const stateParam, float filterLeft, float filterRight) {
	if (angle > filterLeft && angle < filterRight) {
#if EFI_UNIT_TEST
//		printf("addEventClamped %f %s\r\n", angle, getTrigger_value_e(stateParam));
#endif /* EFI_UNIT_TEST */
		addEvent(angle / getEngineCycle(operationMode), channelIndex, stateParam);
	}
}

operation_mode_e TriggerWaveform::getOperationMode() const {
	return operationMode;
}

#if EFI_UNIT_TEST
extern bool printTriggerDebug;
#endif

void TriggerWaveform::calculateExpectedEventCounts(bool useOnlyRisingEdgeForTrigger) {
	UNUSED(useOnlyRisingEdgeForTrigger);

	bool isSingleToothOnPrimaryChannel = useOnlyRisingEdgeForTrigger ? expectedEventCount[0] == 1 : expectedEventCount[0] == 2;
	// todo: next step would be to set 'isSynchronizationNeeded' automatically based on the logic we have here
	if (!shapeWithoutTdc && isSingleToothOnPrimaryChannel != !isSynchronizationNeeded) {
		firmwareError(ERROR_TRIGGER_DRAMA, "trigger constraint violation");
	}

// todo: move the following logic from below here
	//	if (!useOnlyRisingEdgeForTrigger || stateParam == TV_RISE) {
//		expectedEventCount[channelIndex]++;
//	}

}

void TriggerWaveform::addEvent720(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const state) {
	addEvent(angle / 720, channelIndex, state);
}

void TriggerWaveform::addEventAngle(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const state) {
	addEvent(angle / getCycleDuration(), channelIndex, state);
}

void TriggerWaveform::addEvent(angle_t angle, trigger_wheel_e const channelIndex, trigger_value_e const stateParam) {
	efiAssertVoid(CUSTOM_OMODE_UNDEF, operationMode != OM_NONE, "operationMode not set");

	if (channelIndex == T_SECONDARY) {
		needSecondTriggerInput = true;
	}

#if EFI_UNIT_TEST
	if (printTriggerDebug) {
		printf("addEvent2 %.2f i=%d r/f=%d\r\n", angle, channelIndex, stateParam);
	}
#endif

	trigger_value_e state;
	if (invertOnAdd) {
		state = (stateParam == TV_FALL) ? TV_RISE : TV_FALL;
	} else {
		state = stateParam;
	}

#if EFI_UNIT_TEST
	int signal = channelIndex * 1000 + stateParam;
	triggerSignals[privateTriggerDefinitionSize] = signal;
#endif


	// todo: the whole 'useOnlyRisingEdgeForTrigger' parameter and logic should not be here
	// todo: see calculateExpectedEventCounts
	// related calculation should be done once trigger is initialized outside of trigger shape scope
	if (!useOnlyRisingEdgeForTriggerTemp || stateParam == TV_RISE) {
		expectedEventCount[channelIndex]++;
	}

	efiAssertVoid(CUSTOM_ERR_6599, angle > 0, "angle should be positive");
	if (privateTriggerDefinitionSize > 0) {
		if (angle <= previousAngle) {
			warning(CUSTOM_ERR_TRG_ANGLE_ORDER, "invalid angle order: new=%.2f and prev=%.2f, size=%d", angle, previousAngle, privateTriggerDefinitionSize);
			setShapeDefinitionError(true);
			return;
		}
	}
	previousAngle = angle;
	if (privateTriggerDefinitionSize == 0) {
		privateTriggerDefinitionSize = 1;
		for (int i = 0; i < PWM_PHASE_MAX_WAVE_PER_PWM; i++) {
			SingleChannelStateSequence *wave = &this->wave.channels[i];

			if (wave->pinStates == nullptr) {
				warning(CUSTOM_ERR_STATE_NULL, "wave pinStates is NULL");
				setShapeDefinitionError(true);
				return;
			}
			wave->setState(/* switchIndex */ 0, /* value */ initialState[i]);
		}

		isRiseEvent[0] = TV_RISE == stateParam;
		wave.setSwitchTime(0, angle);
		wave.channels[channelIndex].setState(/* channelIndex */ 0, /* value */ state);
		return;
	}

	int exactMatch = wave.findAngleMatch(angle, privateTriggerDefinitionSize);
	if (exactMatch != (int)EFI_ERROR_CODE) {
		warning(CUSTOM_ERR_SAME_ANGLE, "same angle: not supported");
		setShapeDefinitionError(true);
		return;
	}

	int index = wave.findInsertionAngle(angle, privateTriggerDefinitionSize);

	/**
	 * todo: it would be nice to be able to provide trigger angles without sorting them externally
	 * The idea here is to shift existing data - including handling high vs low state of the signals
	 */
	// todo: does this logic actually work? I think it does not! due to broken state handling
/*
	for (int i = size - 1; i >= index; i--) {
		for (int j = 0; j < PWM_PHASE_MAX_WAVE_PER_PWM; j++) {
			wave.waves[j].pinStates[i + 1] = wave.getChannelState(j, index);
		}
		wave.setSwitchTime(i + 1, wave.getSwitchTime(i));
	}
*/
	isRiseEvent[index] = TV_RISE == stateParam;

	if ((unsigned)index != privateTriggerDefinitionSize) {
		firmwareError(ERROR_TRIGGER_DRAMA, "are we ever here?");
	}

	privateTriggerDefinitionSize++;

	for (int i = 0; i < PWM_PHASE_MAX_WAVE_PER_PWM; i++) {
		pin_state_t value = wave.getChannelState(/* channelIndex */i, index - 1);
		wave.channels[i].setState(index, value);
	}
	wave.setSwitchTime(index, angle);
	wave.channels[channelIndex].setState(index, state);
}

angle_t TriggerWaveform::getSwitchAngle(int index) const {
	return getCycleDuration() * wave.getSwitchTime(index);
}

void setToothedWheelConfiguration(TriggerWaveform *s, int total, int skipped,
		operation_mode_e operationMode) {
#if EFI_ENGINE_CONTROL

	s->useRiseEdge = true;

	initializeSkippedToothTriggerWaveformExt(s, total, skipped,
			operationMode);
#endif
}

void TriggerWaveform::setTriggerSynchronizationGap2(float syncRatioFrom, float syncRatioTo) {
	setTriggerSynchronizationGap3(/*gapIndex*/0, syncRatioFrom, syncRatioTo);
}

void TriggerWaveform::setTriggerSynchronizationGap3(int gapIndex, float syncRatioFrom, float syncRatioTo) {
	isSynchronizationNeeded = true;
	this->syncronizationRatioFrom[gapIndex] = syncRatioFrom;
	this->syncronizationRatioTo[gapIndex] = syncRatioTo;
	if (gapIndex == 0) {
		// we have a special case here - only sync with one gap has this feature
		this->syncRatioAvg = (int)efiRound((syncRatioFrom + syncRatioTo) * 0.5f, 1.0f);
	}

#if EFI_UNIT_TEST
	if (printTriggerDebug) {
		printf("setTriggerSynchronizationGap3 %d %.2f %.2f\r\n", gapIndex, syncRatioFrom, syncRatioTo);
	}
#endif /* EFI_UNIT_TEST */

}

/**
 * this method is only used on initialization
 */
int TriggerWaveform::findAngleIndex(float target) const {
	int engineCycleEventCount = getLength();

	efiAssert(CUSTOM_ERR_ASSERT, engineCycleEventCount > 0, "engineCycleEventCount", 0);

	uint32_t left = 0;
	uint32_t right = engineCycleEventCount - 1;

	/**
	 * Let's find the last trigger angle which is less or equal to the desired angle
	 * todo: extract binary search as template method?
	 */
    while (left <= right) {
        int middle = (left + right) / 2;
		angle_t eventAngle = eventAngles[middle];

        if (eventAngle < target) {
            left = middle + 1;
        } else if (eventAngle > target) {
            right = middle - 1;
        } else {
            // Values are equal
            return middle;             // Key found
        }
    }
    return left - 1;
}

void TriggerWaveform::setShapeDefinitionError(bool value) {
	shapeDefinitionError = value;
}

void TriggerWaveform::findTriggerPosition(event_trigger_position_s *position,
		angle_t angle DEFINE_CONFIG_PARAM(angle_t, globalTriggerAngleOffset)) {
	efiAssertVoid(CUSTOM_ERR_6574, !cisnan(angle), "findAngle#1");
	assertAngleRange(angle, "findAngle#a1", CUSTOM_ERR_6545);

	efiAssertVoid(CUSTOM_ERR_6575, !cisnan(tdcPosition), "tdcPos#1")
	assertAngleRange(tdcPosition, "tdcPos#a1", CUSTOM_UNEXPECTED_TDC_ANGLE);

	efiAssertVoid(CUSTOM_ERR_6576, !cisnan(CONFIG_PARAM(globalTriggerAngleOffset)), "tdcPos#2")
	assertAngleRange(CONFIG_PARAM(globalTriggerAngleOffset), "tdcPos#a2", CUSTOM_INVALID_GLOBAL_OFFSET);

	// convert engine cycle angle into trigger cycle angle
	angle += tdcPosition + CONFIG_PARAM(globalTriggerAngleOffset);
	efiAssertVoid(CUSTOM_ERR_6577, !cisnan(angle), "findAngle#2");
	fixAngle2(angle, "addFuel#2", CUSTOM_ERR_6555, getEngineCycle(operationMode));

	int triggerEventIndex = triggerIndexByAngle[(int)angle];
	angle_t triggerEventAngle = eventAngles[triggerEventIndex];
	if (angle < triggerEventAngle) {
		warning(CUSTOM_OBD_ANGLE_CONSTRAINT_VIOLATION, "angle constraint violation in findTriggerPosition(): %.2f/%.2f", angle, triggerEventAngle);
		return;
	}

	position->triggerEventIndex = triggerEventIndex;
	position->angleOffsetFromTriggerEvent = angle - triggerEventAngle;
}

void TriggerWaveform::prepareShape() {
#if EFI_ENGINE_CONTROL && EFI_SHAFT_POSITION_INPUT
	int engineCycleInt = (int) getEngineCycle(operationMode);
	for (int angle = 0; angle < engineCycleInt; angle++) {
		int triggerShapeIndex = findAngleIndex(angle);
		if (useOnlyRisingEdgeForTriggerTemp) {
			// we need even index for front_only mode - so if odd indexes are rounded down
			triggerShapeIndex = triggerShapeIndex & 0xFFFFFFFE;
		}
		triggerIndexByAngle[angle] = triggerShapeIndex;
	}
#endif
}

void TriggerWaveform::setTriggerSynchronizationGap(float syncRatio) {
	setTriggerSynchronizationGap3(/*gapIndex*/0, syncRatio * 0.75f, syncRatio * 1.25f);
}

void TriggerWaveform::setSecondTriggerSynchronizationGap2(float syncRatioFrom, float syncRatioTo) {
	setTriggerSynchronizationGap3(/*gapIndex*/1, syncRatioFrom, syncRatioTo);
}

void TriggerWaveform::setThirdTriggerSynchronizationGap(float syncRatio) {
	setTriggerSynchronizationGap3(/*gapIndex*/2, syncRatio * 0.75f, syncRatio * 1.25f);
}

void TriggerWaveform::setSecondTriggerSynchronizationGap(float syncRatio) {
	setTriggerSynchronizationGap3(/*gapIndex*/1, syncRatio * 0.75f, syncRatio * 1.25f);
}


/**
 * External logger is needed because at this point our logger is not yet initialized
 */
void TriggerWaveform::initializeTriggerWaveform(Logging *logger, operation_mode_e ambiguousOperationMode, bool useOnlyRisingEdgeForTrigger, const trigger_config_s *triggerConfig) {

#if EFI_PROD_CODE
	efiAssertVoid(CUSTOM_ERR_6641, getCurrentRemainingStack() > EXPECTED_REMAINING_STACK, "init t");
	scheduleMsg(logger, "initializeTriggerWaveform(%s/%d)", getTrigger_type_e(triggerConfig->type), (int) triggerConfig->type);
#endif

	shapeDefinitionError = false;

	this->useOnlyRisingEdgeForTriggerTemp = useOnlyRisingEdgeForTrigger;

	switch (triggerConfig->type) {

	case TT_TOOTHED_WHEEL:
		initializeSkippedToothTriggerWaveformExt(this, triggerConfig->customTotalToothCount,
				triggerConfig->customSkippedToothCount, ambiguousOperationMode);
		break;

	case TT_MAZDA_MIATA_NA:
		initializeMazdaMiataNaShape(this);
		break;

	case TT_MAZDA_MIATA_NB1:
		initializeMazdaMiataNb1Shape(this);
		break;

	case TT_MAZDA_MIATA_VVT_TEST:
		initializeMazdaMiataVVtTestShape(this);
		break;

	case TT_MAZDA_Z5:
		initialize_Mazda_Engine_z5_Shape(this);
		break;

	case TT_MIATA_NB2_VVT_CAM:
		initializeMazdaMiataVVtCamShape(this);
		break;

	case TT_RENIX_66_2_2_2:
		initializeRenix66_2_2(this);
		break;

	case TT_RENIX_44_2_2:
		initializeRenix44_2_2(this);
		break;

	case TT_MIATA_VVT:
		initializeMazdaMiataNb2Crank(this);
		break;

	case TT_DODGE_NEON_1995:
		configureNeon1995TriggerWaveform(this);
		break;

	case TT_DODGE_NEON_1995_ONLY_CRANK:
		configureNeon1995TriggerWaveformOnlyCrank(this);
		break;

	case TT_DODGE_STRATUS:
		configureDodgeStratusTriggerWaveform(this);
		break;

	case TT_DODGE_NEON_2003_CAM:
		configureNeon2003TriggerWaveformCam(this);
		break;

	case TT_DODGE_NEON_2003_CRANK:
		configureNeon2003TriggerWaveformCam(this);
//		configureNeon2003TriggerWaveformCrank(triggerShape);
		break;

	case TT_FORD_ASPIRE:
		configureFordAspireTriggerWaveform(this);
		break;

	case TT_GM_7X:
		configureGmTriggerWaveform(this);
		break;

	case TT_MAZDA_DOHC_1_4:
		configureMazdaProtegeLx(this);
		break;

	case TT_ONE_PLUS_ONE:
		configureOnePlusOne(this);
		break;

	case TT_3_1_CAM:
		configure3_1_cam(this);
		break;

	case TT_ONE_PLUS_TOOTHED_WHEEL_60_2:
		configureOnePlus60_2(this);
		break;

	case TT_ONE:
		setToothedWheelConfiguration(this, 1, 0, ambiguousOperationMode);
		break;

	case TT_MAZDA_SOHC_4:
		configureMazdaProtegeSOHC(this);
		break;

	case TT_MINI_COOPER_R50:
		configureMiniCooperTriggerWaveform(this);
		break;

	case TT_TOOTHED_WHEEL_60_2:
		setToothedWheelConfiguration(this, 60, 2, ambiguousOperationMode);
		break;

	case TT_60_2_VW:
		setVwConfiguration(this);
		break;

	case TT_TOOTHED_WHEEL_36_1:
		setToothedWheelConfiguration(this, 36, 1, ambiguousOperationMode);
		break;

	case TT_BOSCH_QUICK_START:
		configureQuickStartSenderWheel(this);
		break;

	case TT_HONDA_K_12_1:
		configureHondaK_12_1(this);
		break;

	case TT_HONDA_4_24_1:
		configureHonda_1_4_24(this, true, true, T_CHANNEL_3, T_PRIMARY, 0);
		shapeWithoutTdc = true;
		break;

	case TT_HONDA_4_24:
		configureHonda_1_4_24(this, false, true, T_NONE, T_PRIMARY, 0);
		shapeWithoutTdc = true;
		break;

	case TT_HONDA_1_24:
		configureHonda_1_4_24(this, true, false, T_PRIMARY, T_NONE, 10);
		break;

	case TT_HONDA_ACCORD_1_24_SHIFTED:
		configureHondaAccordShifted(this);
		break;

	case TT_HONDA_1_4_24:
		configureHondaAccordCDDip(this);
		break;

	case TT_HONDA_CBR_600:
		configureHondaCbr600(this);
		break;

	case TT_HONDA_CBR_600_CUSTOM:
		configureHondaCbr600custom(this);
		break;

	case TT_MITSUBISHI:
		initializeMitsubishi4g18(this);
		break;

	case TT_DODGE_RAM:
		initDodgeRam(this);
		break;

	case TT_JEEP_4_CYL:
		initJeep_XJ_4cyl_2500(this);
		break;

	case TT_JEEP_18_2_2_2:
		initJeep18_2_2_2(this);
		break;

	case TT_SUBARU_7_6:
		initializeSubaru7_6(this);
		break;

	case TT_36_2_2_2:
		initialize36_2_2_2(this);
		break;

	case TT_2JZ_3_34:
		initialize2jzGE3_34(this);
		break;

	case TT_2JZ_1_12:
		initialize2jzGE1_12(this);
		break;

	case TT_NISSAN_SR20VE:
		initializeNissanSR20VE_4(this);
		break;

	case TT_NISSAN_SR20VE_360:
		initializeNissanSR20VE_4_360(this);
		break;

	case TT_ROVER_K:
		initializeRoverK(this);
		break;

	case TT_FIAT_IAW_P8:
		configureFiatIAQ_P8(this);
		break;

	case TT_GM_LS_24:
		initGmLS24(this);
		break;

	default:
		setShapeDefinitionError(true);
		warning(CUSTOM_ERR_NO_SHAPE, "initializeTriggerWaveform() not implemented: %d", triggerConfig->type);
	}
	/**
	 * Feb 2019 suggestion: it would be an improvement to remove 'expectedEventCount' logic from 'addEvent'
	 * and move it here, after all events were added.
	 */
	calculateExpectedEventCounts(useOnlyRisingEdgeForTrigger);
	version++;

	if (!shapeDefinitionError) {
		wave.checkSwitchTimes(getSize());
	}

}
