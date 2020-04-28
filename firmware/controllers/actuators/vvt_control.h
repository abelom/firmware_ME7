/*
 * vvt_control.h
 *
 *  Created on: 25. nov. 2019
 *     (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */
#pragma once

#include "engine.h"
#include "periodic_task.h"



void turnVvtControlOn(void);
void startVvtPin(void);
void stopVvtPin(void);
void initBoostCtrl(Logging *sharedLogger);
void setVvtPFactor(float value);
void setVvtIFactor(float value);
void setVvtDFactor(float value);
void setDefaultVvtParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE);
void onConfigurationChangeVvtCallback(engine_configuration_s *previousConfiguration);
void initVvtCtrl(Logging *sharedLogger);
