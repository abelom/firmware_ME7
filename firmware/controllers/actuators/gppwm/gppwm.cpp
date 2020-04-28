
#include "global.h"
#include "engine.h"

#include "gppwm_channel.h"
#include "pwm_generator_logic.h"

EXTERN_ENGINE;

static GppwmChannel channels[4];
static OutputPin pins[4];
static SimplePwm outputs[4];

static gpPwm1_Map3D_t gpPwmMap1("gpPwm1map", 1);
static gpPwm1_Map3D_t gpPwmMap2("gpPwm2map", 1);
static gpPwm1_Map3D_t gpPwmMap3("gpPwm3map", 1);
static gpPwm1_Map3D_t gpPwmMap4("gpPwm4map", 1);



void initGpPwm(DECLARE_ENGINE_PARAMETER_SIGNATURE) {
	for (size_t i = 0; i < efi::size(channels); i++) {
		auto& cfg = CONFIG(gppwm)[i];

		// If no pin, don't enable this channel.
		if (cfg.pin == GPIO_UNASSIGNED) continue;

		// Determine frequency and whether PWM is enabled
		float freq = cfg.pwmFrequency;
		bool usePwm = freq > 0;

		// Setup pin & pwm
		pins[i].initPin("gp pwm", cfg.pin);
		startSimplePwm(&outputs[i], "gp pwm", &engine->executor, &pins[i], freq, 0);

		// Set up this channel's lookup table
		gpPwmMap1.init(config->gpPwmTable1, config->gpPwm1LoadBins, config->gpPwm1RpmBins);
			gpPwmMap2.init(config->gpPwmTable2, config->gpPwm2LoadBins, config->gpPwm2RpmBins);
			gpPwmMap3.init(config->gpPwmTable3, config->gpPwm3LoadBins, config->gpPwm3RpmBins);
			gpPwmMap4.init(config->gpPwmTable4, config->gpPwm4LoadBins, config->gpPwm4RpmBins);


		// Finally configure the channel
		INJECT_ENGINE_REFERENCE(&channels[i]);
		channels[i].init(usePwm, &outputs[i], &cfg);
	}
}

void updateGppwm() {
	for (size_t i = 0; i < efi::size(channels); i++) {
		channels[i].update();
	}
}
