// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on integration/thermistor_state.txt Wed Apr 08 00:41:39 CEST 2020
// by class com.rusefi.output.CHeaderConsumer
// begin
#ifndef CONTROLLERS_GENERATED_THERMISTOR_GENERATED_H
#define CONTROLLERS_GENERATED_THERMISTOR_GENERATED_H
#include "rusefi_types.h"
// start of thermistor_state_s
struct thermistor_state_s {
	/**
	 * offset 0
	 */
	float resistance = (float)0;
	/**
	 * offset 4
	 */
	float voltageMCU = (float)0;
	/**
	 * offset 8
	 */
	float voltageBoard = (float)0;
	/** total size 12*/
};

typedef struct thermistor_state_s thermistor_state_s;

#endif
// end
// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on integration/thermistor_state.txt Wed Apr 08 00:41:39 CEST 2020
