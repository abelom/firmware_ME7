/**
 * @file	speed_density.h
 *
 * @date May 29, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#define gramm_second_to_cc_minute(gs) ((gs) / 0.0119997981)
#define cc_minute_to_gramm_second(ccm) ((ccm) * 0.0119997981)

temperature_t getTCharge(int rpm, float tps DECLARE_ENGINE_PARAMETER_SUFFIX);
float getCylinderAirMass(float volumetricEfficiency, float MAP, float tempK DECLARE_ENGINE_PARAMETER_SUFFIX);
float sdMath(float airMass, float AFR DECLARE_ENGINE_PARAMETER_SUFFIX);

void setDefaultVETable(DECLARE_ENGINE_PARAMETER_SIGNATURE);
void initSpeedDensity(DECLARE_ENGINE_PARAMETER_SIGNATURE);
floatms_t getSpeedDensityFuel(float map DECLARE_ENGINE_PARAMETER_SUFFIX);
