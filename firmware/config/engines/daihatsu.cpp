/**
 * @file daihatsu.cpp
 *
 * Daihatsu 3-Cylinder KF-VE
 * set engine_type 34
 *
 * @date Sep 10, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "daihatsu.h"

EXTERN_CONFIG;

void setDaihatsu(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->trigger.type = TT_36_2_2_2;

	engineConfiguration->specs.cylindersCount = 3;
	engineConfiguration->specs.firingOrder = FO_1_2_3;

}
