/**
 * @file	nissan_primera.cpp
 *
 * NISSAN_PRIMERA
 * set engine_type 5
 *
 * @date Oct 14, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"

#include "nissan_primera.h"

EXTERN_CONFIG;

void setNissanPrimeraEngineConfiguration(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setDefaultFrankensoConfiguration(PASS_CONFIG_PARAMETER_SIGNATURE);

	engineConfiguration->trigger.type = TT_NISSAN_SR20VE;

	engineConfiguration->ignitionPins[0] = GPIOD_7;
	engineConfiguration->ignitionPins[1] = GPIO_UNASSIGNED;
	engineConfiguration->ignitionPins[2] = GPIOD_6;


	engineConfiguration->auxValves[0] = GPIOE_14;
	engineConfiguration->auxValves[1] = GPIOE_12;
}

void setNissanPrimeraEngineConfiguration_360(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->trigger.type = TT_NISSAN_SR20VE_360;
}
