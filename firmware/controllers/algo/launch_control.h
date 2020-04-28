/*
 * @file launch_control.h
 *
 * @date 10. sep. 2019
 * (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#pragma once

#include "engine.h"

void initLaunchControl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);
void initAntiLag(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);
void setDefaultAntiLagParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
angle_t getLaunchRetard(int rpm, float engineLoad DECLARE_ENGINE_PARAMETER_SUFFIX);
