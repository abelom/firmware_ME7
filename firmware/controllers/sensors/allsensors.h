/**
 * @file    allsensors.h
 * @brief
 *
 * This file should have been called 'sensors.h' but there is some conflict
 * with standard win32 header :(
 *
 *
 * @date Nov 15, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "tps.h"
#include "map.h"
#include "maf.h"
#include "ego.h"
#include "voltage.h"
#include "thermistors.h"
#include "adc_inputs.h"

void initSensors(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);

bool hasAcToggle(DECLARE_ENGINE_PARAMETER_SIGNATURE);
bool getAcToggle(DECLARE_ENGINE_PARAMETER_SIGNATURE);

