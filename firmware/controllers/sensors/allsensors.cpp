/**
 * @file    allsensors.cpp
 * @brief
 *
 *
 * @date Nov 15, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "engine.h"
#include "allsensors.h"

EXTERN_ENGINE;

void initSensors(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	initThermistors(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
	initMapDecoder(sharedLogger PASS_ENGINE_PARAMETER_SUFFIX);
}

bool hasAcToggle(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	return engineConfiguration->acSwitchAdc != EFI_ADC_NONE;
}

// todo: move this somewhere else? maybe.
bool getAcToggle(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	/**
	 * todo: make this flexible
	 *
	 * for now we are looking for a pull-up. High level means input switch is floating (which is OFF position)
	 * low value means input is ground - which means ON.
	 */
	return getVoltageDivided("A/C", engineConfiguration->acSwitchAdc PASS_ENGINE_PARAMETER_SUFFIX) < 2.5;
}
