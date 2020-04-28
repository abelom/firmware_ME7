/*
 * gp_pwm.cpp
 *
 *  Created on: 19. des. 2019
 *       (C) All rights reserved by RUUD BILELEKTRO, NORWAY
 */
#include "global.h"
//#if EFI_GP_PWM

#if EFI_TUNER_STUDIO
#include "tunerstudio_configuration.h"
#endif /* EFI_TUNER_STUDIO */
#include "sensor.h"
#include "engine.h"
#include "thermistors.h"
#include "tps.h"
#include "map.h"
#include "io_pins.h"
#include "engine_configuration.h"
#include "gp_pwm.h"
#include "engine_controller.h"
#include "periodic_task.h"
#include "pin_repository.h"
#include "local_version_holder.h"
#include "pwm_generator_logic.h"
#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif
EXTERN_ENGINE;

#define NO_PIN_PERIOD 500

static Logging *logger;
extern pin_output_mode_e DEFAULT_OUTPUT;


static gpPwm1_Map3D_t gpPwmMap1("gpPwm1map", 1);
static gpPwm1_Map3D_t gpPwmMap2("gpPwm2map", 1);
static gpPwm1_Map3D_t gpPwmMap3("gpPwm3map", 1);
static gpPwm1_Map3D_t gpPwmMap4("gpPwm4map", 1);

static SimplePwm gpPwm1("gpPwm1");
static SimplePwm gpPwm2("gpPwm2");
static SimplePwm gpPwm3("gpPwm3");
static SimplePwm gpPwm4("gpPwm4");


class GpPWMControl: public PeriodicTimerController {
	int getPeriodMs() override {
		return 50;
	}
	void PeriodicTask() override {

		float rpm = GET_RPM_VALUE;
				float map = getMap(PASS_ENGINE_PARAMETER_SIGNATURE);
				auto tps = Sensor::get(SensorType::DriverThrottleIntent);
				float clt = getCoolantTemperatureM(PASS_ENGINE_PARAMETER_SIGNATURE);

				//Conditions

				percent_t duty1 = 0;
				percent_t duty2 = 0;
				percent_t duty3 = 0;
				percent_t duty4 = 0;

				float enable1Value1 = engineConfiguration->gpPwm[0].conditions[0].gpPwmConditionValue;
				float enable1Value2 = engineConfiguration->gpPwm[0].conditions[1].gpPwmConditionValue;
				float enable1Value3 = engineConfiguration->gpPwm[0].conditions[2].gpPwmConditionValue;
				float enable1Value4 = engineConfiguration->gpPwm[0].conditions[3].gpPwmConditionValue;

				float enable2Value1 = engineConfiguration->gpPwm[1].conditions[0].gpPwmConditionValue;
				float enable2Value2 = engineConfiguration->gpPwm[1].conditions[1].gpPwmConditionValue;
				float enable2Value3 = engineConfiguration->gpPwm[1].conditions[2].gpPwmConditionValue;
				float enable2Value4 = engineConfiguration->gpPwm[1].conditions[3].gpPwmConditionValue;

				float enable3Value1 = engineConfiguration->gpPwm[2].conditions[0].gpPwmConditionValue;
				float enable3Value2 = engineConfiguration->gpPwm[2].conditions[1].gpPwmConditionValue;
				float enable3Value3 = engineConfiguration->gpPwm[2].conditions[2].gpPwmConditionValue;
				float enable3Value4 = engineConfiguration->gpPwm[2].conditions[3].gpPwmConditionValue;

				float enable4Value1 = engineConfiguration->gpPwm[3].conditions[0].gpPwmConditionValue;
				float enable4Value2 = engineConfiguration->gpPwm[3].conditions[1].gpPwmConditionValue;
				float enable4Value3 = engineConfiguration->gpPwm[3].conditions[2].gpPwmConditionValue;
				float enable4Value4 = engineConfiguration->gpPwm[3].conditions[3].gpPwmConditionValue;

//Gp Pwm 1 Switch
		if (engineConfiguration->gpPwm[0].gpPwmLoad == LOAD_MAP) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, map);

		} else if (engineConfiguration->gpPwm[0].gpPwmLoad == LOAD_TPS) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, tps.value_or(50));

		} else if (engineConfiguration->gpPwm[0].gpPwmLoad == LOAD_CLT) {
			duty1 = gpPwmMap1.getValue(rpm / RPM_1_BYTE_PACKING_MULT, clt);
		}
		if ((engineConfiguration->gpPwm1SwitchCondition) && (CONFIG(gpPwm[0].io.gpPwmInputPin) != GPIO_UNASSIGNED)) {
			engine->gpPwm1InputPinState = efiReadPin(CONFIG(gpPwm[0].io.gpPwmInputPin));
			if (!engine->gpPwm1InputPinState) {
				duty1 = 0;
			}
		}

if (engineConfiguration->isGpPwm1Enabled) {
//GP Pwm 1 Condition 1
	if ((engineConfiguration->gpPwm[0].conditions[0].gpPwmChannel == MAP)) {
				if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == BELOW) && (enable1Value1 > map)) {
					duty1 = 0;
				}
				if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == ABOVE) && (enable1Value1 < map)) {
					duty1 = 0;
				}
		     }
	if ((engineConfiguration->gpPwm[0].conditions[0].gpPwmChannel == tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == BELOW) && (enable1Value1 > tps.value_or(50))) {
					duty1 = 0;
				}
		if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == ABOVE) && (enable1Value1 < tps.value_or(50))) {
					duty1 = 0;
				}
		   }
	if ((engineConfiguration->gpPwm[0].conditions[0].gpPwmChannel == COOLANT)) {
		if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == BELOW) && (enable1Value1 > clt)) {
					duty1 = 0;
				}
		if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == ABOVE)  && (enable1Value1 < clt)) {
					duty1 = 0;
				}
			}

			//GP Pwm 1 Condition 2
	if ((engineConfiguration->gpPwm[0].conditions[1].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[0].conditions[1].belowOrAbove == BELOW) && (enable1Value2 > map)) {
						duty1 = 0;
					}
		if ((engineConfiguration->gpPwm[0].conditions[1].belowOrAbove == ABOVE) && (enable1Value2 < map)) {
						duty1 = 0;
					}
				}
	if ((engineConfiguration->gpPwm[0].conditions[1].gpPwmChannel ==  tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[0].conditions[1].belowOrAbove == BELOW) && (enable1Value2 > tps.value_or(50))) {
						duty1 = 0;
					}
		if ((engineConfiguration->gpPwm[0].conditions[1].belowOrAbove == ABOVE) && (enable1Value2 < tps.value_or(50))) {
						duty1 = 0;
					}

				}
	if ((engineConfiguration->gpPwm[0].conditions[1].gpPwmChannel ==  COOLANT)) {
		if ((engineConfiguration->gpPwm[0].conditions[0].belowOrAbove == BELOW) && (enable1Value2 > clt)) {
						duty1 = 0;
					}
		if ((engineConfiguration->gpPwm[0].conditions[1].belowOrAbove == ABOVE) && (enable1Value2 > clt)) {
						duty1 = 0;
					}
				}

  //GP Pwm 1 Condition 3

	if ((engineConfiguration->gpPwm[0].conditions[2].gpPwmChannel ==  MAP)) {
		if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == BELOW) && (enable1Value3 > map)) {
			duty1 = 0;
			}
	if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == ABOVE) && (enable1Value3 < map)) {
			duty1 = 0;
			}
		}
	if ((engineConfiguration->gpPwm[0].conditions[2].gpPwmChannel ==  tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == BELOW) && (enable1Value3 > tps.value_or(50))) {
			duty1 = 0;
			}
		if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == ABOVE) && (enable1Value3 < tps.value_or(50))) {
			duty1 = 0;
			}
		}
	if ((engineConfiguration->gpPwm[0].conditions[2].gpPwmChannel  == COOLANT)) {
		if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == BELOW) && (enable1Value3 > clt)) {
			duty1 = 0;
			}
		if ((engineConfiguration->gpPwm[0].conditions[2].belowOrAbove == ABOVE) && (enable1Value3 < clt)) {
			duty1 = 0;
			}
		}

//GP Pwm 1 Condition 4



	if ((engineConfiguration->gpPwm[0].conditions[3].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == BELOW)  && (enable1Value4 > map)) {
			duty1 = 0;
			}
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == ABOVE) && (enable1Value4 < map)) {
			duty1 = 0;
			}
		}
	if ((engineConfiguration->gpPwm[0].conditions[3].gpPwmChannel == tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == BELOW)  && (enable1Value4 > tps.value_or(50))) {
			duty1 = 0;
			}
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == ABOVE) && (enable1Value4 < tps.value_or(50))) {
			duty1 = 0;
			}
		}
	if ((engineConfiguration->gpPwm[0].conditions[3].gpPwmChannel == COOLANT)) {
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == BELOW)  && (enable1Value4 > clt)) {
			duty1 = 0;
			}
		if ((engineConfiguration->gpPwm[0].conditions[3].belowOrAbove == ABOVE) && (enable1Value4 < clt)) {
			duty1 = 0;
			}
		}

			gpPwm1.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty1));

	//GP Pwm 2 Condition 1
	if ((engineConfiguration->gpPwm[1].conditions[0].gpPwmChannel == MAP)) {
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == BELOW) && (enable2Value1 > map)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == ABOVE) && (enable2Value1 < map)) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[0].gpPwmChannel == tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == BELOW) && (enable2Value1 > tps.value_or(50))) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == ABOVE) && (enable2Value1 < tps.value_or(50))) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[0].gpPwmChannel == COOLANT)) {
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == BELOW) && (enable2Value1 > clt)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == ABOVE)  && (enable2Value1 < clt)) {
	duty2 = 0;
	}
	}

	//GP Pwm 1 Condition 2
	if ((engineConfiguration->gpPwm[1].conditions[1].gpPwmChannel == MAP)) {
	if ((engineConfiguration->gpPwm[1].conditions[1].belowOrAbove == BELOW) && (enable2Value2 > map)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[1].belowOrAbove == ABOVE) && (enable2Value2 < map)) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[1].gpPwmChannel ==  tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[1].conditions[1].belowOrAbove == BELOW) && (enable2Value2 > tps.value_or(50))) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[1].belowOrAbove == ABOVE) && (enable2Value2 < tps.value_or(50))) {
	duty2 = 0;
	}

	}
	if ((engineConfiguration->gpPwm[1].conditions[1].gpPwmChannel ==  COOLANT)) {
	if ((engineConfiguration->gpPwm[1].conditions[0].belowOrAbove == BELOW) && (enable2Value2 > clt)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[1].belowOrAbove == ABOVE) && (enable2Value2 > clt)) {
	duty2 = 0;
	}
	}

	//GP Pwm 1 Condition 3

	if ((engineConfiguration->gpPwm[1].conditions[2].gpPwmChannel ==  MAP)) {
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == BELOW) && (enable2Value3 > map)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == ABOVE) && (enable2Value3 < map)) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[2].gpPwmChannel ==  tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == BELOW) && (enable2Value3 > tps.value_or(50))) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == ABOVE) && (enable2Value3 < tps.value_or(50))) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[2].gpPwmChannel  == COOLANT)) {
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == BELOW) && (enable2Value3 > clt)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[2].belowOrAbove == ABOVE) && (enable2Value3 < clt)) {
	duty2 = 0;
	}
	}

	//GP Pwm 2 Condition 4



	if ((engineConfiguration->gpPwm[1].conditions[3].gpPwmChannel == MAP)) {
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == BELOW)  && (enable2Value4 > map)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == ABOVE) && (enable2Value4 < map)) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[3].gpPwmChannel == tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == BELOW)  && (enable2Value4 > tps.value_or(50))) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == ABOVE) && (enable2Value4 < tps.value_or(50))) {
	duty2 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[1].conditions[3].gpPwmChannel == COOLANT)) {
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == BELOW)  && (enable2Value4 > clt)) {
	duty2 = 0;
	}
	if ((engineConfiguration->gpPwm[1].conditions[3].belowOrAbove == ABOVE) && (enable2Value4 < clt)) {
	duty2 = 0;
	}
	}

	gpPwm2.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty2));

	//GP Pwm 3 Condition 1
	if ((engineConfiguration->gpPwm[2].conditions[0].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == BELOW) && (enable3Value1 > map)) {
			duty3 = 0;
		}
		if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == ABOVE) && (enable3Value1 < map)) {
			duty3 = 0;
		}
	 }
	if ((engineConfiguration->gpPwm[2].conditions[0].gpPwmChannel == tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == BELOW) && (enable3Value1 > tps.value_or(50))) {
			duty3 = 0;
		}
	if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == ABOVE) && (enable3Value1 < tps.value_or(50))) {
			duty3 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[2].conditions[0].gpPwmChannel == COOLANT)) {
	if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == BELOW) && (enable3Value1 > clt)) {
			duty3 = 0;
		}
	if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == ABOVE)  && (enable3Value1 < clt)) {
			duty3 = 0;
		}
	}

	//GP Pwm 3 Condition 2
	if ((engineConfiguration->gpPwm[2].conditions[1].gpPwmChannel == MAP)) {
	if ((engineConfiguration->gpPwm[2].conditions[1].belowOrAbove == BELOW) && (enable3Value2 > map)) {
				duty3 = 0;
			}
	if ((engineConfiguration->gpPwm[2].conditions[1].belowOrAbove == ABOVE) && (enable3Value2 < map)) {
				duty3 = 0;
			}
		}
	if ((engineConfiguration->gpPwm[2].conditions[1].gpPwmChannel ==  tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[2].conditions[1].belowOrAbove == BELOW) && (enable3Value2 > tps.value_or(50))) {
				duty3 = 0;
			}
	if ((engineConfiguration->gpPwm[2].conditions[1].belowOrAbove == ABOVE) && (enable3Value2 < tps.value_or(50))) {
				duty3 = 0;
			}

		}
	if ((engineConfiguration->gpPwm[2].conditions[1].gpPwmChannel ==  COOLANT)) {
	if ((engineConfiguration->gpPwm[2].conditions[0].belowOrAbove == BELOW) && (enable3Value2 > clt)) {
				duty3 = 0;
			}
	if ((engineConfiguration->gpPwm[2].conditions[1].belowOrAbove == ABOVE) && (enable3Value2 > clt)) {
				duty3 = 0;
			}
		}

	//GP Pwm 3 Condition 3

		if ((engineConfiguration->gpPwm[2].conditions[2].gpPwmChannel ==  MAP)) {
			if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == BELOW) && (enable3Value3 > map)) {
			duty3 = 0;
			}
	if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == ABOVE) && (enable3Value3 < map)) {
	duty3 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[2].conditions[2].gpPwmChannel ==  tps.value_or(50))) {
	if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == BELOW) && (enable3Value3 > tps.value_or(50))) {
	duty3 = 0;
	}
	if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == ABOVE) && (enable3Value3 < tps.value_or(50))) {
	duty3 = 0;
	}
	}
	if ((engineConfiguration->gpPwm[2].conditions[2].gpPwmChannel  == COOLANT)) {
	if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == BELOW) && (enable3Value3 > clt)) {
	duty3 = 0;
	}
	if ((engineConfiguration->gpPwm[2].conditions[2].belowOrAbove == ABOVE) && (enable3Value3 < clt)) {
	duty3 = 0;
	}
	}

	//GP Pwm 3 Condition 4



	if ((engineConfiguration->gpPwm[2].conditions[3].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == BELOW)  && (enable3Value4 > map)) {
		duty3 = 0;
	}
	if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == ABOVE) && (enable3Value4 < map)) {
	duty3 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[2].conditions[3].gpPwmChannel == tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == BELOW)  && (enable3Value4 > tps.value_or(50))) {
		duty3 = 0;
		}
	if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == ABOVE) && (enable3Value4 < tps.value_or(50))) {
		duty3 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[2].conditions[3].gpPwmChannel == COOLANT)) {
		if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == BELOW)  && (enable3Value4 > clt)) {
		duty3 = 0;
		}
		if ((engineConfiguration->gpPwm[2].conditions[3].belowOrAbove == ABOVE) && (enable3Value4 < clt)) {
		duty3 = 0;
		}
	}

	gpPwm3.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty3));

//GP Pwm 4 Condition 1
	if ((engineConfiguration->gpPwm[3].conditions[0].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == BELOW) && (enable4Value1 > map)) {
			duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == ABOVE) && (enable4Value1 < map)) {
			duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[0].gpPwmChannel == tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == BELOW) && (enable4Value1 > tps.value_or(50))) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == ABOVE) && (enable4Value1 < tps.value_or(50))) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[0].gpPwmChannel == COOLANT)) {
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == BELOW) && (enable4Value1 > clt)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == ABOVE)  && (enable4Value1 < clt)) {
		duty4 = 0;
		}
	}

//GP Pwm 4 Condition 2
	if ((engineConfiguration->gpPwm[3].conditions[1].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[3].conditions[1].belowOrAbove == BELOW) && (enable4Value2 > map)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[1].belowOrAbove == ABOVE) && (enable4Value2 < map)) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[1].gpPwmChannel ==  tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[3].conditions[1].belowOrAbove == BELOW) && (enable4Value2 > tps.value_or(50))) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[1].belowOrAbove == ABOVE) && (enable4Value2 < tps.value_or(50))) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[1].gpPwmChannel ==  COOLANT)) {
		if ((engineConfiguration->gpPwm[3].conditions[0].belowOrAbove == BELOW) && (enable4Value2 > clt)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[1].belowOrAbove == ABOVE) && (enable4Value2 > clt)) {
		duty4 = 0;
		}
	}

  //GP Pwm 4 Condition 3

	if ((engineConfiguration->gpPwm[3].conditions[2].gpPwmChannel ==  MAP)) {
		if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == BELOW) && (enable4Value3 > map)) {
		duty4 = 0;
		}
	if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == ABOVE) && (enable4Value3 < map)) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[2].gpPwmChannel ==  tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == BELOW) && (enable4Value3 > tps.value_or(50))) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == ABOVE) && (enable4Value3 < tps.value_or(50))) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[2].gpPwmChannel  == COOLANT)) {
		if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == BELOW) && (enable4Value3 > clt)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[2].belowOrAbove == ABOVE) && (enable4Value3 < clt)) {
		duty4 = 0;
		}
	}

//GP Pwm 4 Condition 4

	if ((engineConfiguration->gpPwm[3].conditions[3].gpPwmChannel == MAP)) {
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == BELOW)  && (enable4Value4 > map)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == ABOVE) && (enable4Value4 < map)) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[3].gpPwmChannel == tps.value_or(50))) {
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == BELOW)  && (enable4Value4 > tps.value_or(50))) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == ABOVE) && (enable4Value4 < tps.value_or(50))) {
		duty4 = 0;
		}
	}
	if ((engineConfiguration->gpPwm[3].conditions[3].gpPwmChannel == COOLANT)) {
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == BELOW)  && (enable4Value4 > clt)) {
		duty4 = 0;
		}
		if ((engineConfiguration->gpPwm[3].conditions[3].belowOrAbove == ABOVE) && (enable4Value4 < clt)) {
		duty4 = 0;
		}
	}

	gpPwm4.setSimplePwmDutyCycle(PERCENT_TO_DUTY(duty4));


#if EFI_TUNER_STUDIO
		tsOutputChannels.gpPwm1Duty = duty1;
	#endif /* EFI_TUNER_STUDIO */
	}
	}
};

static GpPWMControl GpPWMController;

void setDefaultGpPwmParameters(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	engineConfiguration->gpPwm[0].io.gpPwmPin = GPIO_UNASSIGNED;
	engineConfiguration->gpPwm[1].io.gpPwmPin = GPIO_UNASSIGNED;
	engineConfiguration->gpPwm[2].io.gpPwmPin = GPIO_UNASSIGNED;
	engineConfiguration->gpPwm[3].io.gpPwmPin = GPIO_UNASSIGNED;
	engineConfiguration->gpPwm[0].gpPwmFrequency = 50;
	engineConfiguration->gpPwm[1].gpPwmFrequency = 50;
	engineConfiguration->gpPwm[2].gpPwmFrequency = 50;
	engineConfiguration->gpPwm[3].gpPwmFrequency = 50;

	setLinearCurve(config->gpPwm1LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm1RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm2LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm2RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm3LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm3RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);
	setLinearCurve(config->gpPwm4LoadBins, 0, 100, 1);
	setLinearCurve(config->gpPwm4RpmBins, 0, 8000 / RPM_1_BYTE_PACKING_MULT, 1);


	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable1[loadIndex][rpmIndex] = config->gpPwm1LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable2[loadIndex][rpmIndex] = config->gpPwm2LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable3[loadIndex][rpmIndex] = config->gpPwm3LoadBins[loadIndex];
		}
	}
	for (int loadIndex = 0; loadIndex < GP_PWM_LOAD_COUNT; loadIndex++) {
		for (int rpmIndex = 0; rpmIndex < GP_PWM_RPM_COUNT; rpmIndex++) {
			config->gpPwmTable4[loadIndex][rpmIndex] = config->gpPwm4LoadBins[loadIndex];
		}
	}
}
static void turnGpPwm1On() {
	if (engineConfiguration->gpPwm[0].io.gpPwmPin == GPIO_UNASSIGNED) {
		return;
	}
	startSimplePwmExt(&gpPwm1, "gpPwm1", &engine->executor, CONFIG(gpPwm[0].io.gpPwmPin),
			&enginePins.gp1Pin, engineConfiguration->gpPwm[0].gpPwmFrequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm2On() {
	if (CONFIG(gpPwm[1].io.gpPwmPin) == GPIO_UNASSIGNED) {
	return;
	}
	startSimplePwmExt(&gpPwm2, "gpPwm2", &engine->executor, CONFIG(gpPwm[1].io.gpPwmPin),
			&enginePins.gp2Pin, engineConfiguration->gpPwm[1].gpPwmFrequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm3On() {
	if (CONFIG(gpPwm[2].io.gpPwmPin) == GPIO_UNASSIGNED) {
	return;
	}
	startSimplePwmExt(&gpPwm3, "gpPwm3", &engine->executor, CONFIG(gpPwm[2].io.gpPwmPin),
			&enginePins.gp3Pin, engineConfiguration->gpPwm[2].gpPwmFrequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

static void turnGpPwm4On() {
	if (CONFIG(gpPwm[3].io.gpPwmPin) == GPIO_UNASSIGNED) {
	return;
	}
	startSimplePwmExt(&gpPwm4, "gpPwm4", &engine->executor, CONFIG(gpPwm[3].io.gpPwmPin),
			&enginePins.gp4Pin, engineConfiguration->gpPwm[3].gpPwmFrequency, 0.1f,
			(pwm_gen_callback*) applyPinState);
}

void startGpPwmPins(void) {

	turnGpPwm1On();
	turnGpPwm2On();
	turnGpPwm3On();
	turnGpPwm4On();
}
void stopGpPwmPins(void) {
	brain_pin_markUnused(activeConfiguration.gpPwm[0].io.gpPwmPin);
	brain_pin_markUnused(activeConfiguration.gpPwm[1].io.gpPwmPin);
	brain_pin_markUnused(activeConfiguration.gpPwm[2].io.gpPwmPin);
	brain_pin_markUnused(activeConfiguration.gpPwm[3].io.gpPwmPin);
}

void initGpPwmCtrl(Logging *sharedLogger DECLARE_ENGINE_PARAMETER_SUFFIX) {
	logger = sharedLogger;
	gpPwmMap1.init(config->gpPwmTable1, config->gpPwm1LoadBins, config->gpPwm1RpmBins);
	gpPwmMap2.init(config->gpPwmTable2, config->gpPwm2LoadBins, config->gpPwm2RpmBins);
	gpPwmMap3.init(config->gpPwmTable3, config->gpPwm3LoadBins, config->gpPwm3RpmBins);
	gpPwmMap4.init(config->gpPwmTable4, config->gpPwm4LoadBins, config->gpPwm4RpmBins);
	startGpPwmPins();
	GpPWMController.Start();

}
//#endif
