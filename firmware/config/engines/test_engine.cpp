/**
 * @file	test_engine.cpp
 *
 * set engine_type 26
 * VVT
 * set engine_type 45
 *
 * See also custom_engine.cpp
 *
 * @date Nov 14, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "engine_configuration.h"
#include "test_engine.h"
#include "engine_math.h"

EXTERN_CONFIG;

void setTestEngineConfiguration(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setDefaultFrankensoConfiguration(PASS_CONFIG_PARAMETER_SIGNATURE);

	setOperationMode(engineConfiguration, FOUR_STROKE_CAM_SENSOR);
	engineConfiguration->trigger.type = TT_ONE_PLUS_ONE;

//	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
//	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL;
//	trigger_config_s *triggerConfig = &engineConfiguration->trigger;
//	triggerConfig->customTotalToothCount = 60;
//	triggerConfig->customSkippedToothCount = 0;
	engineConfiguration->useOnlyRisingEdgeForTrigger = false;

	engineConfiguration->mafAdcChannel = EFI_ADC_1;
	engineConfiguration->vbattAdcChannel = EFI_ADC_NONE;

	setWholeIatCorrTimingTable(0 PASS_CONFIG_PARAMETER_SUFFIX);

	engineConfiguration->ignitionMode = IM_ONE_COIL;
	setConstantDwell(3 PASS_CONFIG_PARAMETER_SUFFIX); // 50% duty cycle @ 5000 rpm

	engineConfiguration->malfunctionIndicatorPin = GPIO_UNASSIGNED;

	engineConfiguration->ignitionPins[0] = GPIOC_7; // #1
	engineConfiguration->ignitionPins[1] = GPIO_UNASSIGNED; // #2
	engineConfiguration->ignitionPins[2] = GPIO_UNASSIGNED; // #3
	engineConfiguration->ignitionPins[3] = GPIO_UNASSIGNED; // #4
	engineConfiguration->ignitionPins[4] = GPIO_UNASSIGNED; // #5
	engineConfiguration->ignitionPins[5] = GPIO_UNASSIGNED; // #6

	engineConfiguration->logicAnalyzerPins[0] = GPIO_UNASSIGNED;
	engineConfiguration->logicAnalyzerPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->logicAnalyzerPins[2] = GPIO_UNASSIGNED;
	engineConfiguration->logicAnalyzerPins[3] = GPIO_UNASSIGNED;
}

void setTestVVTEngineConfiguration(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setDefaultFrankensoConfiguration(PASS_CONFIG_PARAMETER_SIGNATURE);
	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);

	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL;
	engineConfiguration->trigger.customTotalToothCount = 3;
	engineConfiguration->trigger.customSkippedToothCount = 1;

	// set algorithm 3
	setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);

	engineConfiguration->triggerInputPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->camInputs[0] = GPIOA_5;

	// set global_trigger_offset_angle 0
	engineConfiguration->globalTriggerAngleOffset = 0;

	engineConfiguration->vvtMode = VVT_SECOND_HALF;
	engineConfiguration->debugMode = DBG_VVT;
}

#if EFI_UNIT_TEST
void setTestEngineIssue366both(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setTestEngineConfiguration(PASS_CONFIG_PARAMETER_SIGNATURE);


	engineConfiguration->useOnlyRisingEdgeForTrigger = false;
	engineConfiguration->trigger.customTotalToothCount = 2;
	engineConfiguration->trigger.customSkippedToothCount = 1;

	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL;

}

void setTestEngineIssue366rise(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setTestEngineIssue366both(PASS_CONFIG_PARAMETER_SIGNATURE);


	engineConfiguration->useOnlyRisingEdgeForTrigger = true;
}
#endif /* EFI_UNIT_TEST */
