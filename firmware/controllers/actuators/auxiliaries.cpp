/*
 * auxiliaries.cpp
 *
 *  Created on: 10. mar. 2020
 *      (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */
#include "global.h"
#if EFI_AUXILIARIES

#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */
#include "auxiliaries.h"
#include "engine.h"
#include "allsensors.h"
#include "thermistors.h"
#include "tps.h"
#include "map.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "gp_pwm.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "pwm_generator.h"
#include "local_version_holder.h"
#include "pwm_generator_logic.h"
#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif
EXTERN_ENGINE;

#define NO_PIN_PERIOD 500


extern pin_output_mode_e DEFAULT_OUTPUT;

class Auxiliaries: public PeriodicTimerController {
	int getPeriodMs() override {
		return 50;
	}

	void PeriodicTask() override {

		if (CONFIG(mainRelayPin) != GPIO_UNASSIGNED) {
			enginePins.mainRelay.setValue((getTimeNowSeconds() < 2) || (getVBatt(PASS_ENGINE_PARAMETER_SIGNATURE) > 5) || engine->isInShutdownMode());
		}
		// see STARTER_RELAY_LOGIC
		if (CONFIG(starterRelayPin) != GPIO_UNASSIGNED) {
			enginePins.starterRelay.setValue(engine->rpmCalculator.getRpm() < engineConfiguration->cranking.rpm);
		}
		// see FAN_CONTROL_LOGIC
		if (CONFIG(fanPin) != GPIO_UNASSIGNED) {
			enginePins.fanRelay.setValue((enginePins.fanRelay.getLogicValue() && (getCoolantTemperature() > engineConfiguration->fanOffTemperature)) ||
				(getCoolantTemperature() > engineConfiguration->fanOnTemperature) || engine->isCltBroken);
		}
		// see AC_RELAY_LOGIC
		if (CONFIG(acRelayPin) != GPIO_UNASSIGNED) {
			enginePins.acRelay.setValue(getAcToggle(PASS_ENGINE_PARAMETER_SIGNATURE) && engine->rpmCalculator.getRpm() > 850);
		}
		// see FUEL_PUMP_LOGIC
		if (CONFIG(fuelPumpPin) != GPIO_UNASSIGNED) {
			enginePins.fuelPumpRelay.setValue((getTimeNowSeconds() < engineConfiguration->startUpFuelPumpDuration) || (engine->rpmCalculator.getRpm() > 0));
		}

		enginePins.o2heater.setValue(engine->rpmCalculator.isRunning(PASS_ENGINE_PARAMETER_SIGNATURE));
	}
};

static Auxiliaries AuxControl;


void stopAuxPins(void) {
	brain_pin_markUnused(activeConfiguration.mainRelayPin);
	brain_pin_markUnused(activeConfiguration.starterRelayPin);
	brain_pin_markUnused(activeConfiguration.fanPin);
	brain_pin_markUnused(activeConfiguration.acRelayPin);
	brain_pin_markUnused(activeConfiguration.fuelPumpPin);
}

void initAuxiliaries(DECLARE_ENGINE_PARAMETER_SIGNATURE) {


	stopAuxPins();
	AuxControl.Start();
}
#endif

