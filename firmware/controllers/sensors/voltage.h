/**
 * @file    voltage.h
 * @brief
 *
 *
 * @date Nov 15, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "engine_configuration.h"

float getVRef(DECLARE_ENGINE_PARAMETER_SIGNATURE);
float getVBatt(DECLARE_ENGINE_PARAMETER_SIGNATURE);
bool hasVBatt(DECLARE_ENGINE_PARAMETER_SIGNATURE);
