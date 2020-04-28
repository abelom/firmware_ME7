/**
 * @file map.h
 *
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "engine_configuration.h"

void initMapDecoder(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);

/**
 * @return Raw MAP sensor value right now
 */
float getRawMap(DECLARE_ENGINE_PARAMETER_SIGNATURE);
float getBaroPressure(DECLARE_ENGINE_PARAMETER_SIGNATURE);
bool hasBaroSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE);
bool hasMapSensor(DECLARE_ENGINE_PARAMETER_SIGNATURE);

/**
 * @return MAP value averaged within a window of measurement
 */
float getMap(DECLARE_ENGINE_PARAMETER_SIGNATURE);
float getMapByVoltage(float voltage DECLARE_ENGINE_PARAMETER_SUFFIX);
float decodePressure(float voltage, air_pressure_sensor_config_s * mapConfig DECLARE_ENGINE_PARAMETER_SUFFIX);
float validateMap(float mapKPa DECLARE_ENGINE_PARAMETER_SUFFIX);

#define KPA_PER_PSI 6.89475728f

// PSI (relative to atmosphere) to kPa (relative to vacuum)
#define PSI2KPA(psi)  (101.32500411216164f + KPA_PER_PSI * (psi))

#define INHG2KPA(inhg) ((inhg) * 3.386375)
#define KPA2INHG(kpa) ((kpa) / 3.386375)

