/**
 * @file	adc_math.h
 *
 * todo: all this looks to be about internal ADC, merge this file with adc_inputs.h?
 * these macro are also used in simulator so maybe not really merging with adc_inputs.h until simulator get's it's own implementation?
 *
 * @date Mar 18, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "engine_configuration.h"
#define ADC_MAX_VALUE 4095

#define adcToVolts(adc) ((engineConfiguration->adcVcc) / ADC_MAX_VALUE * (adc))

#define voltsToAdc(volts) ((volts) * (ADC_MAX_VALUE / (engineConfiguration->adcVcc)))

float getVoltage(const char *msg, adc_channel_e channel DECLARE_ENGINE_PARAMETER_SUFFIX);

float getVoltageDivided(const char *msg, adc_channel_e channel DECLARE_ENGINE_PARAMETER_SUFFIX);

