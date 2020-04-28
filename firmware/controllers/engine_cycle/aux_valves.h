/*
 * @file aux_valves.h
 *
 * @date Nov 25, 2017
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "engine.h"

void initAuxValves(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX);
void updateAuxValves(DECLARE_ENGINE_PARAMETER_SIGNATURE);
void plainPinTurnOn(AuxActor *current);
void plainPinTurnOff(NamedOutputPin *output);
