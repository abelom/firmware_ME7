/**
 * @file    maf.h
 * @brief
 *
 * by the way 2.081989116 kg/h = 1 ft^3/min
 *
 *
 * @date Nov 15, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"

float getMafVoltage(DECLARE_ENGINE_PARAMETER_SIGNATURE);
bool hasMafSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE);
float getRealMaf(DECLARE_ENGINE_PARAMETER_SIGNATURE);

void setBosch0280218037(persistent_config_s *engineConfiguration);
void setBosch0280218004(persistent_config_s *engineConfiguration);

void setDensoTODO(persistent_config_s *engineConfiguration);
void setMazdaMiataNAMaf(persistent_config_s *e);

