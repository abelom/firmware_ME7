/*
 * @file launch_control.cpp
 *
 *  @date 10. sep. 2019
 *      (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */

#include "engine.h"

#if EFI_LAUNCH_CONTROL

#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */

#include "boost_control.h"
#include "vehicle_speed.h"
#include "launch_control.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "allsensors.h"
#include "sensor.h"
#include "engine_math.h"
#include "efi_gpio.h"
#include "advance_map.h"
#include "engine_state.h"
#include "advance_map.h"

static Logging *logger;
extern TunerStudioOutputChannels tsOutputChannels;

EXTERN_ENGINE;


static bool getAntilagSwitchCondition(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	switch (engineConfiguration->antiLag.antiLagActivationMode) {
	case SWITCH_INPUT_ANTILAG:
		if (CONFIG(antiLagActivatePin) != GPIO_UNASSIGNED) {
			engine->antiLagPinState = efiReadPin(CONFIG(antiLagActivatePin));
			}
			return engine->antiLagPinState;
	//case ALWAYS_ON_ANTILAG:
	default:
	return true;
	}
}
static bool getActivateSwitchCondition(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	switch (engineConfiguration->launchActivationMode) {
	case SWITCH_INPUT_LAUNCH:
		if (CONFIG(launchActivatePin) != GPIO_UNASSIGNED) {
			engine->launchActivatePinState = efiReadPin(CONFIG(launchActivatePin));
		}
		return engine->launchActivatePinState;

	case CLUTCH_INPUT_LAUNCH:
		if (CONFIG(clutchDownPin) != GPIO_UNASSIGNED) {
			engine->clutchDownState = efiReadPin(CONFIG(clutchDownPin));
		}
		return engine->clutchDownState;
	default:
		// ALWAYS_ACTIVE_LAUNCH
		return true;
	}
}

class LaunchControl : public PeriodicTimerController{
    efitick_t launchTimer;
    int getPeriodMs() override{
        return 50;
    }
    void PeriodicTask() override{
    	if (!CONFIG(launchControlEnabled)) {
    				return;
    			}

        int rpm = GET_RPM_VALUE;
        auto tps = Sensor::get(SensorType::DriverThrottleIntent);
        int tpstreshold = (CONFIG(launchTpsTreshold));
        float timeDelay = (CONFIG(launchActivateDelay));
        int launchRpm = (CONFIG(launchRpm));
        engine->activateSwitchCondition = getActivateSwitchCondition(PASS_ENGINE_PARAMETER_SIGNATURE);
        engine->rpmCondition = (rpm > launchRpm);
        engine->tpsCondition = (tps.Value > tpstreshold);
        bool speedCondition = (getVehicleSpeed() < CONFIG(launchSpeedTreshold)) || !engineConfiguration->launchDisableBySpeed;
        bool combinedConditions = speedCondition && engine->activateSwitchCondition && engine->rpmCondition && engine->tpsCondition;
        if (engineConfiguration->debugMode == DBG_LAUNCH){
#if EFI_TUNER_STUDIO
            tsOutputChannels.launchTpsCond = engine->tpsCondition;
            tsOutputChannels.launchSwitchCond = engine->activateSwitchCondition;
            tsOutputChannels.debugIntField1 = engine->rpmCondition;
            tsOutputChannels.debugIntField2 = engine->tpsCondition;
            tsOutputChannels.debugIntField3 = speedCondition;
            tsOutputChannels.debugIntField4 = engine->activateSwitchCondition;
            tsOutputChannels.debugIntField5 = combinedConditions;
            tsOutputChannels.debugFloatField1 = engine->launchActivatePinState;
            tsOutputChannels.debugFloatField2 = engine->isLaunchCondition;
            tsOutputChannels.debugFloatField3 = engine->clutchDownState;
            tsOutputChannels.debugFloatField4 = engineConfiguration->launchControlEnabled;
#endif /* EFI_TUNER_STUDIO */
        }
        engine->isLaunchCondition = false;
        if(combinedConditions){
            if(!engineConfiguration->useLaunchActivateDelay){
                engine->isLaunchCondition = true;
            } else if(getTimeNowNt() - launchTimer > MS2NT(timeDelay * 1000)){
                engine->isLaunchCondition = true; // ...enable launch!
            }
        } else {
            launchTimer = getTimeNowNt();
        }
        if (engineConfiguration->debugMode == DBG_LAUNCH){
#if EFI_TUNER_STUDIO
            tsOutputChannels.debugFloatField2 = engine->isLaunchCondition * 10;
            tsOutputChannels.debugFloatField3 = combinedConditions * 10;
#endif /* EFI_TUNER_STUDIO */
        }
    }
};


class AntiLag: public PeriodicTimerController {
	efitick_t antiLagTimerTimer;

	DECLARE_ENGINE_PTR;

	int getPeriodMs() override {
		return 50;
	}

	void PeriodicTask() override {
		if (!CONFIG(antiLagEnabled)) {
			return;
		}
		int rpm = GET_RPM_VALUE;
		auto tps = Sensor::get(SensorType::DriverThrottleIntent);
		float clt = getCoolantTemperatureM(PASS_ENGINE_PARAMETER_SIGNATURE);

		int antiLagRpm = engineConfiguration->antiLag.antiLagRpmTreshold;
		int antiLagTps = engineConfiguration->antiLag.antiLagTpsTreshold;
		int antiLagCoolant = engineConfiguration->antiLag.antiLagCoolantTreshold;
		float timeOut = engineConfiguration->antiLag.antilagTimeout;

        bool tpsCondition = (antiLagTps < tps.Value); //AL Stays active above this TPS
		bool rpmCondition = (antiLagRpm < rpm); //AL Stays active above this RPM
		bool switchCondition = getAntilagSwitchCondition(PASS_ENGINE_PARAMETER_SIGNATURE);
		bool coolantCondition = (antiLagCoolant < clt);


		bool antiLagConditions = rpmCondition && switchCondition && tpsCondition && coolantCondition;
		//Antilag Deactivation Timeout routine
		if (!antiLagConditions) {
			antiLagTimerTimer = getTimeNowNt();
			engine->isAntilagCondition = false;
			} else {
			// If conditions are met...
			if ((getTimeNowNt() - antiLagTimerTimer < MS2NT(timeOut * 1000))) {
				engine->isAntilagCondition = true;           // ...enable antilag until timeout!
			}
		}
	}
};
static LaunchControl Launch;

void setDefaultLaunchParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->launchRpm = 4000;    // Rpm to trigger Launch condition
	engineConfiguration->launchTimingRetard = 10; // retard in absolute degrees ATDC
	engineConfiguration->launchTimingRpmRange = 500; // Rpm above Launch triggered for full retard
	engineConfiguration->launchSparkCutEnable = true;
	engineConfiguration->launchFuelCutEnable = false;
	engineConfiguration->hardCutRpmRange = 500; //Rpm above Launch triggered +(if retard enabled) launchTimingRpmRange to hard cut
	engineConfiguration->launchSpeedTreshold = 10; //maximum speed allowed before disable launch
	engineConfiguration->launchFuelAdded = 10; // Extra fuel in % when launch are triggered
	engineConfiguration->launchBoostDuty = 70; // boost valve duty cycle at launch
	engineConfiguration->launchActivateDelay = 3; // Delay in seconds for launch to kick in
	engineConfiguration->enableLaunchRetard = true;
	engineConfiguration->enableLaunchBoost = true;
	engineConfiguration->launchSmoothRetard = true; //interpolates the advance linear from launchrpm to fully retarded at launchtimingrpmrange
}
angle_t getLaunchRetard(int rpm, float engineLoad DECLARE_ENGINE_PARAMETER_SUFFIX) {
		float retard;
		float launchAngle = -(CONFIG(launchTimingRetard));
		int launchAdvanceRpmRange = engineConfiguration->launchTimingRpmRange;
		int launchRpm = engineConfiguration->launchRpm;
		angle_t currentTiming = getAdvance(rpm, engineLoad PASS_ENGINE_PARAMETER_SUFFIX);
	if ((engineConfiguration->enableLaunchRetard) && (engineConfiguration->launchSmoothRetard)) {
			// interpolate timing from rpm at launch triggered to full retard at launch launchRpm + launchTimingRpmRange
		retard = interpolateClamped(launchRpm, currentTiming, (launchRpm + launchAdvanceRpmRange), launchAngle, rpm);
	}
	if ((engineConfiguration->enableLaunchRetard) && (!engineConfiguration->launchSmoothRetard)) {
		retard = launchAngle;
	}
	return retard;
}
void applyLaunchControlLimiting(bool *limitedSpark, bool *limitedFuel DECLARE_ENGINE_PARAMETER_SUFFIX) {
	int retardThresholdRpm = CONFIG(launchRpm) +
		(CONFIG(enableLaunchRetard) ? CONFIG(launchAdvanceRpmRange) : 0) +
		CONFIG(hardCutRpmRange);

	if (retardThresholdRpm > GET_RPM_VALUE) {
		*limitedSpark = engine->isLaunchCondition && engineConfiguration->launchSparkCutEnable;
		*limitedFuel = engine->isLaunchCondition && engineConfiguration->launchFuelCutEnable;
	}
}

void initLaunchControl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	Launch.Start();
}
static AntiLag Als;

void setDefaultAntiLagParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->antiLag.antiLagTpsTreshold = 60;
	engineConfiguration->antiLag.antilagTimeout = 3;
	engineConfiguration->antiLag.antiLagRpmTreshold = 3000;
	engineConfiguration->antiLag.antiLagExtraFuel = 10;
}



void initAntiLag(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	Als.Start();
}
#endif /* EFI_LAUNCH_CONTROL */


