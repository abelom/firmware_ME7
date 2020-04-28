/*
 * @file common_headers.h
 *
 * Header file shared between firmware, simulator and unit_tests
 *
 * @date Sep 16, 2018
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include "efifeatures.h"
#include "rusefi_types.h"

#include <math.h>
#include <stdio.h>

#include "error_handling.h"
#include "auto_generated_enums.h"
#include "efilib.h"
#include "efitime.h"

#ifdef __cplusplus
#include "datalogging.h"
#include "loggingcentral.h"
#include "cli_registry.h"
#endif /* __cplusplus */

/**
 * reference to configuration parameter.
 */
#define DISPLAY_CONFIG(x) x
/**
 * The main annotation of live data - reference to dynamic state variable
 * See also 'TS_GET_STRUCT'
 * The advantage of Live Data is that there is no need to copy data into tsOutputChannels structure - Live Data is reading exactly
 * the same data as actual runtime logic. It's also important to have annotations of the View as close to actual implementation as possible
 * to increase the changes of those being in sync.
 *
 * One day Live Data could be replace with a Domain Specific Language
 * One day all Live Data could be provided to TS without data copy to tsOutputChannels - the idea is to have a virtual tsOutputChannels
 * where rusEfi would pull data from different memory ranges depending on the requested offset, more or less virtual pages. This could depend
 * on the TS bugfix described in https://github.com/rusefi/rusefi/issues/794?
 *
 */
#define DISPLAY_FIELD(x) x
/**
 * Advanced tag to access fields within data structures
 * "running.fuel" variable would be referenced as DISPLAY_PREFIX(running).DISPLAY_FIELD(fuel)
 */
#define DISPLAY_PREFIX(x) x

/**
 * we use this 'DISPLAY' macro if value is not used by C++ code in current context and we want to hide it from compiler
 */
#define DISPLAY(x)

#define DISPLAY_TAG(x)
/**
 * Just a human-readable comment. '_' underscore symbols become spaces. EOL is handled as new line.
 */
#define DISPLAY_TEXT(x)
#define DISPLAY_STATE(x)
/**
 * Reference to one of 'output' channels/standard gauges.
 */
#define DISPLAY_SENSOR(x) {}
#define DISPLAY_IF(x) x

#define DECLARE_ENGINE_PTR                                 \
	Engine *engine = nullptr;                              \
	engine_configuration_s *engineConfiguration = nullptr; \
	persistent_config_s *config = nullptr;


#define INJECT_ENGINE_REFERENCE(x)                  \
	(x)->engine = engine;                           \
	(x)->engineConfiguration = engineConfiguration; \
	(x)->config = config;

#define EXPAND_Engine \
	    engine_configuration_s *engineConfiguration = engine->engineConfigurationPtr; \
		persistent_config_s *config = engine->config; \
		(void)engineConfiguration; \
		(void)config; \
		

#ifndef EFI_ACTIVE_CONFIGURATION_IN_FLASH
// We store a special changeable copy of configuration is RAM, so we can just compare them
#define isConfigurationChanged(x) (engineConfiguration->x != activeConfiguration.x)
#else
// We cannot call prepareVoidConfiguration() for activeConfiguration if it's stored in flash,
// so we need to tell the firmware that it's "void" (i.e. zeroed, invalid) by setting a special flag variable,
// and then we consider 'x' as changed if it's just non-zero.
extern bool isActiveConfigurationVoid;
#define isConfigurationChanged(x) ((engineConfiguration->x != activeConfiguration.x) || (isActiveConfigurationVoid && engineConfiguration->x != 0))
#endif /* EFI_ACTIVE_CONFIGURATION_IN_FLASH */

#define isPinOrModeChanged(pin, mode) (isConfigurationChanged(pin) || isConfigurationChanged(mode))
