/*
 * boost_control.h
 *
 *  Created on: 18. aug. 2019
 *     (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */
#pragma once

#include "engine.h"
#include "periodic_task.h"

void startBoostPin(void);
void stopBoostPin(void);
void initBoostCtrl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);
void setBoostPFactor(float p);
void setBoostIFactor(float i);
void setBoostDFactor(float d);
void setDefaultBoostParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void showBoostInfo(void);
void onConfigurationChangeBoostCallback(engine_configuration_s *previousConfiguration);
