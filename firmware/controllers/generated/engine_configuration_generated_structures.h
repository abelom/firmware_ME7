// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on gen_config.bat integration\rusefi_config.txt Tue Apr 28 03:31:05 CEST 2020
// by class com.rusefi.output.CHeaderConsumer
// begin
#ifndef CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#define CONTROLLERS_GENERATED_ENGINE_CONFIGURATION_GENERATED_STRUCTURES_H
#include "rusefi_types.h"
// start of stft_cell_cfg_s
struct stft_cell_cfg_s {
	/**
	 * offset 0
	 */
	int8_t maxAdd;
	/**
	 * offset 1
	 */
	int8_t maxRemove;
	/**
	 * offset 2
	 */
	uint16_t timeConstant;
	/** total size 4*/
};

typedef struct stft_cell_cfg_s stft_cell_cfg_s;

// start of stft_s
struct stft_s {
	/**
	 * Below this RPM, the idle region is active
	 * offset 0
	 */
	uint8_t maxIdleRegionRpm;
	/**
	 * Below this engine load, the overrun region is active
	 * offset 1
	 */
	uint8_t maxOverrunLoad;
	/**
	 * Above this engine load, the power region is active
	 * offset 2
	 */
	uint8_t minPowerLoad;
	/**
	 * When close to correct AFR, pause correction. This can improve stability by not changing the adjustment if the error is extremely small, but is not required.
	 * offset 3
	 */
	uint8_t deadband;
	/**
	 * Below this temperature, correction is disabled.
	 * offset 4
	 */
	int8_t minClt;
	/**
	 * Below this AFR, correction is paused
	 * offset 5
	 */
	uint8_t minAfr;
	/**
	 * Above this AFR, correction is paused
	 * offset 6
	 */
	uint8_t maxAfr;
	/**
	 * Delay after starting the engine before beginning closed loop correction.
	 * offset 7
	 */
	uint8_t startupDelay;
	/**
	 * offset 8
	 */
	stft_cell_cfg_s cellCfgs[STFT_CELL_COUNT];
	/** total size 24*/
};

typedef struct stft_s stft_s;

// start of pid_s
struct pid_s {
	/**
	 * offset 0
	 */
	float pFactor;
	/**
	 * offset 4
	 */
	float iFactor;
	/**
	 * offset 8
	 */
	float dFactor;
	/**
	 * Linear addition to PID logic
	 * offset 12
	 */
	int16_t offset;
	/**
	 * PID dTime
	 * offset 14
	 */
	int16_t periodMs;
	/**
	 * Output min value
	 * offset 16
	 */
	int16_t minValue;
	/**
	 * Output max value
	 * offset 18
	 */
	int16_t maxValue;
	/** total size 20*/
};

typedef struct pid_s pid_s;

// start of cranking_parameters_s
struct cranking_parameters_s {
	/**
	 * Base duration of the fuel injection during cranking, this is modified by the multipliers for CLT, IAT, TPS ect, to give the final cranking pulse width.
	 * offset 0
	 */
	float baseFuel;
	/**
	 * This sets the RPM limit below which the ECU will use cranking fuel and ignition logic, typically this is around 350-450rpm. 
	 * set cranking_rpm X
	 * offset 4
	 */
	int16_t rpm;
	/**
	 * need 4 byte alignment
	 * offset 6
	 */
	uint8_t alignmentFill_at_6[2];
	/** total size 8*/
};

typedef struct cranking_parameters_s cranking_parameters_s;

// start of spi_pins
struct spi_pins {
	/**
	 * offset 0
	 */
	brain_pin_e mosiPin;
	/**
	 * offset 1
	 */
	brain_pin_e misoPin;
	/**
	 * offset 2
	 */
	brain_pin_e sckPin;
	/**
	 * need 4 byte alignment
	 * offset 3
	 */
	uint8_t alignmentFill_at_3;
	/** total size 4*/
};

typedef struct spi_pins spi_pins;

// start of air_pressure_sensor_config_s
struct air_pressure_sensor_config_s {
	/**
	 * kPa value at low volts
	 * offset 0
	 */
	float lowValue;
	/**
	 * kPa value at high volts
	 * offset 4
	 */
	float highValue;
	/**
	 * offset 8
	 */
	air_pressure_sensor_type_e type;
	/**
	 * offset 12
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 13
	 */
	uint8_t align[3];
	/** total size 16*/
};

typedef struct air_pressure_sensor_config_s air_pressure_sensor_config_s;

/**
 * @brief MAP averaging configuration

*/
// start of MAP_sensor_config_s
struct MAP_sensor_config_s {
	/**
	 * offset 0
	 */
	float samplingAngleBins[MAP_ANGLE_SIZE];
	/**
	 * @brief MAP averaging sampling start angle, by RPM
	 * offset 32
	 */
	float samplingAngle[MAP_ANGLE_SIZE];
	/**
	 * offset 64
	 */
	float samplingWindowBins[MAP_WINDOW_SIZE];
	/**
	 * @brief MAP averaging angle duration, by RPM
	 * offset 96
	 */
	float samplingWindow[MAP_WINDOW_SIZE];
	/**
	 * offset 128
	 */
	air_pressure_sensor_config_s sensor;
	/** total size 144*/
};

typedef struct MAP_sensor_config_s MAP_sensor_config_s;

/**
 * @brief Thermistor known values

*/
// start of thermistor_conf_s
struct thermistor_conf_s {
	/**
	 * these values are in Celcius
	 * offset 0
	 */
	float tempC_1;
	/**
	 * offset 4
	 */
	float tempC_2;
	/**
	 * offset 8
	 */
	float tempC_3;
	/**
	 * offset 12
	 */
	float resistance_1;
	/**
	 * offset 16
	 */
	float resistance_2;
	/**
	 * offset 20
	 */
	float resistance_3;
	/**
	 * Pull-up resistor value on your board
	 * offset 24
	 */
	float bias_resistor;
	/** total size 28*/
};

typedef struct thermistor_conf_s thermistor_conf_s;

/**
 * @brief Oil pressure sensor interpolation

*/
// start of oil_pressure_config_s
struct oil_pressure_config_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t align[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct oil_pressure_config_s oil_pressure_config_s;

/**
 * @brief Thermistor curve parameters

*/
// start of ThermistorConf
struct ThermistorConf {
	/**
	 * offset 0
	 */
	thermistor_conf_s config;
	/**
	 * offset 28
	 */
	adc_channel_e adcChannel;
	/**
	 * need 4 byte alignment
	 * offset 29
	 */
	uint8_t alignmentFill_at_29[3];
	/** total size 32*/
};

typedef struct ThermistorConf ThermistorConf;

// start of injector_s
struct injector_s {
	/**
	 * This is your injector flow at the fuel pressure used in the vehicle. cc/min, cubic centimetre per minute
	 * By the way, g/s = 0.125997881 * (lb/hr)
	 * g/s = 0.125997881 * (cc/min)/10.5
	 * g/s = 0.0119997981 * cc/min
	 * offset 0
	 */
	float flow;
	/**
	 * set_flat_injector_lag LAG
	 * set_injector_lag VOLTAGE LAG
	 * offset 4
	 */
	float battLagCorrBins[VBAT_INJECTOR_CURVE_SIZE];
	/**
	 * offset 28
	 */
	uint8_t unused17359[8];
	/**
	 * ms delay between injector open and close dead times
	 * offset 36
	 */
	float battLagCorr[VBAT_INJECTOR_CURVE_SIZE];
	/**
	 * offset 60
	 */
	uint8_t unused173598[8];
	/** total size 68*/
};

typedef struct injector_s injector_s;

// start of antilag
struct antilag {
	/**
	 * This level of throttle position (TP(Main)) needs to be exceeded before Anti-Lag becomes active. Anti-Lag will stay active as long as TP(Main) is above this value. Once the throttle position falls below this value, Anti-Lag will remain active until the AL Deactivation Timeout has elapsed.
	 * offset 0
	 */
	int antiLagTpsTreshold;
	/**
	 * Minimum Coolant temperature before AntiLag arms
	 * offset 4
	 */
	int antiLagCoolantTreshold;
	/**
	 * Additional fuel when AntiLag are armed. Should only be used together with retard
	 * offset 8
	 */
	int antiLagExtraFuel;
	/**
	 * This function overrides the current position of the idle speed solenoid, E-Throttle or stepper motor. This override is used to bleed extra air into the engine when Anti-Lag is active. A standard Idle valve does generally not flow enough air for proper Anti-Lag operation, an after market upgrade is often necessary.
	 * offset 12
	 */
	int antilagExtraAir;
	/**
	 * This function specifies the time after the engine speed (RPM) falls below the AL Enable RPM AND the throttle position (TP(Main)) falls below the AL Enable TP that Anti-Lag remains active for. This is the time that the turbo will remain spooled for once stepping off the throttle.
	 * offset 16
	 */
	int antilagTimeout;
	/**
	 * Sets the current Ignition Retard value as the current Ignition Timing.
	 * offset 20
	 */
	int antiLagRetard;
	/**
	 * Sets the Boost Valve Duty when AntiLag are active
	 * offset 24
	 */
	int antiLagBoost;
	/**
	 * This engine speed (RPM) needs to be exceeded before Anti-Lag becomes active. Anti-Lag will stay active as long as the engine speed (RPM) is above this value. Once the engine speed (RPM) falls below this value, Anti-Lag will remain active until the AL Deactivation Timeout has elapsed.
	 * offset 28
	 */
	int antiLagRpmTreshold;
	/**
	 * offset 32
	 */
	antiLagAirSupply_e antiLagAirSupply;
	/**
	 * The Anti-lag system will remain OFF until the system is armed. This means Anti-Lag will not work until the arming condition(s) is met. Various methods can be used including a Digital Input, an Always ON option allowing the system to be armed permanently.
	 * offset 36
	 */
	antiLagActivationMode_e antiLagActivationMode;
	/** total size 40*/
};

typedef struct antilag antilag;

// start of specs_s
struct specs_s {
	/**
	 * Engine displacement, in litres
	 * see also cylindersCount
	 * offset 0
	 */
	float displacement;
	/**
	 * offset 4
	 */
	cylinders_count_t cylindersCount;
	/**
	 * offset 8
	 */
	firing_order_e firingOrder;
	/** total size 12*/
};

typedef struct specs_s specs_s;

/**
 * @brief Trigger wheel(s) configuration

*/
// start of trigger_config_s
struct trigger_config_s {
	/**
	 * set trigger_type X
	 * offset 0
	 */
	trigger_type_e type;
	/**
	offset 4 bit 0 */
	bool todoRemoveMeOneDay0 : 1;
	/**
	offset 4 bit 1 */
	bool todoRemoveMeOneDay1 : 1;
	/**
	 * This option could be used if your second trigger channel is broken
	offset 4 bit 2 */
	bool useOnlyFirstChannel : 1;
	/**
	offset 4 bit 3 */
	bool unusedBit_4_3 : 1;
	/**
	offset 4 bit 4 */
	bool unusedBit_4_4 : 1;
	/**
	offset 4 bit 5 */
	bool unusedBit_4_5 : 1;
	/**
	offset 4 bit 6 */
	bool unusedBit_4_6 : 1;
	/**
	offset 4 bit 7 */
	bool unusedBit_4_7 : 1;
	/**
	offset 4 bit 8 */
	bool unusedBit_4_8 : 1;
	/**
	offset 4 bit 9 */
	bool unusedBit_4_9 : 1;
	/**
	offset 4 bit 10 */
	bool unusedBit_4_10 : 1;
	/**
	offset 4 bit 11 */
	bool unusedBit_4_11 : 1;
	/**
	offset 4 bit 12 */
	bool unusedBit_4_12 : 1;
	/**
	offset 4 bit 13 */
	bool unusedBit_4_13 : 1;
	/**
	offset 4 bit 14 */
	bool unusedBit_4_14 : 1;
	/**
	offset 4 bit 15 */
	bool unusedBit_4_15 : 1;
	/**
	offset 4 bit 16 */
	bool unusedBit_4_16 : 1;
	/**
	offset 4 bit 17 */
	bool unusedBit_4_17 : 1;
	/**
	offset 4 bit 18 */
	bool unusedBit_4_18 : 1;
	/**
	offset 4 bit 19 */
	bool unusedBit_4_19 : 1;
	/**
	offset 4 bit 20 */
	bool unusedBit_4_20 : 1;
	/**
	offset 4 bit 21 */
	bool unusedBit_4_21 : 1;
	/**
	offset 4 bit 22 */
	bool unusedBit_4_22 : 1;
	/**
	offset 4 bit 23 */
	bool unusedBit_4_23 : 1;
	/**
	offset 4 bit 24 */
	bool unusedBit_4_24 : 1;
	/**
	offset 4 bit 25 */
	bool unusedBit_4_25 : 1;
	/**
	offset 4 bit 26 */
	bool unusedBit_4_26 : 1;
	/**
	offset 4 bit 27 */
	bool unusedBit_4_27 : 1;
	/**
	offset 4 bit 28 */
	bool unusedBit_4_28 : 1;
	/**
	offset 4 bit 29 */
	bool unusedBit_4_29 : 1;
	/**
	offset 4 bit 30 */
	bool unusedBit_4_30 : 1;
	/**
	offset 4 bit 31 */
	bool unusedBit_4_31 : 1;
	/**
	 * offset 8
	 */
	int customTotalToothCount;
	/**
	 * offset 12
	 */
	int customSkippedToothCount;
	/** total size 16*/
};

typedef struct trigger_config_s trigger_config_s;

// start of afr_sensor_s
struct afr_sensor_s {
	/**
	 * offset 0
	 */
	adc_channel_e hwChannel;
	/**
	 * offset 1
	 */
	uint8_t alignAf[3];
	/**
	 * offset 4
	 */
	float v1;
	/**
	 * offset 8
	 */
	float value1;
	/**
	 * offset 12
	 */
	float v2;
	/**
	 * offset 16
	 */
	float value2;
	/** total size 20*/
};

typedef struct afr_sensor_s afr_sensor_s;

// start of idle_hardware_s
struct idle_hardware_s {
	/**
	 * offset 0
	 */
	int solenoidFrequency;
	/**
	 * offset 4
	 */
	output_pin_e solenoidPin;
	/**
	 * offset 5
	 */
	brain_pin_e stepperDirectionPin;
	/**
	 * offset 6
	 */
	brain_pin_e stepperStepPin;
	/**
	 * offset 7
	 */
	pin_output_mode_e solenoidPinMode;
	/** total size 8*/
};

typedef struct idle_hardware_s idle_hardware_s;

// start of etb_io
struct etb_io {
	/**
	 * offset 0
	 */
	brain_pin_e directionPin1;
	/**
	 * offset 1
	 */
	brain_pin_e directionPin2;
	/**
	 * offset 2
	 */
	brain_pin_e controlPin1;
	/**
	 * offset 3
	 */
	brain_pin_e disablePin;
	/** total size 4*/
};

typedef struct etb_io etb_io;

// start of gppwm_condition
struct gppwm_condition {
	/**
	 * offset 0
	 */
	float gpPwmConditionValue;
	/**
	 * offset 4
	 */
	gpPwmChannel_e gpPwmChannel;
	/**
	 * offset 8
	 */
	gpPwmBelowAbove_e belowOrAbove;
	/** total size 12*/
};

typedef struct gppwm_condition gppwm_condition;

// start of gppwm_io
struct gppwm_io {
	/**
	 * offset 0
	 */
	output_pin_e gpPwmPin;
	/**
	 * offset 1
	 */
	pin_output_mode_e gpPwmPinMode;
	/**
	 * offset 2
	 */
	switch_input_pin_e gpPwmInputPin;
	/**
	 * need 4 byte alignment
	 * offset 3
	 */
	uint8_t alignmentFill_at_3;
	/** total size 4*/
};

typedef struct gppwm_io gppwm_io;

// start of gppwm
struct gppwm {
	/**
	 * offset 0
	 */
	loadAxis_e gpPwmLoad;
	/**
	 * offset 4
	 */
	int gpPwmFrequency;
	/**
	 * offset 8
	 */
	int unused123;
	/**
	 * offset 12
	 */
	gppwm_condition conditions[4];
	/**
	 * offset 60
	 */
	gppwm_io io;
	/** total size 64*/
};

typedef struct gppwm gppwm;

// start of vvtsettings
struct vvtsettings {
	/**
	 * offset 0
	 */
	vvtLoadAxis_e vvtLoadAxis;
	/**
	 * offset 4
	 */
	float minVvtTemperature;
	/**
	 * offset 8
	 */
	int vvtPwmFrequency;
	/**
	 * offset 12
	 */
	vvtType_e vvtType;
	/**
	 * offset 16
	 */
	pid_s vvtPid;
	/**
	 * offset 36
	 */
	int maxVvtDeviation;
	/**
	 * offset 40
	 */
	output_pin_e vvtControlPin;
	/**
	 * offset 41
	 */
	pin_output_mode_e vvtControlPinMode;
	/**
	 * offset 42
	 */
	uint8_t pad[2];
	/** total size 44*/
};

typedef struct vvtsettings vvtsettings;

// start of engine_configuration_s
struct engine_configuration_s {
	/**
	 * offset 0
	 */
	engine_type_e engineType;
	/**
	 * Engine sniffer would be disabled above this rpm
	 * set engineSnifferRpmThreshold X
	 * offset 4
	 */
	int engineSnifferRpmThreshold;
	/**
	 * offset 8
	 */
	injector_s injector;
	/**
	 * Should trigger emulator push data right into trigger handling logic, eliminating the need for physical jumper wires?
	 * See also triggerSimulatorPins
	 * PS: Funny name, right? :)
	offset 76 bit 0 */
	bool directSelfStimulation : 1;
	/**
	offset 76 bit 1 */
	bool activateAuxPid1 : 1;
	/**
	offset 76 bit 2 */
	bool isVerboseAuxPid1 : 1;
	/**
	offset 76 bit 3 */
	bool activateAuxPid2 : 1;
	/**
	offset 76 bit 4 */
	bool isVerboseAuxPid2 : 1;
	/**
	offset 76 bit 5 */
	bool activateAuxPid3 : 1;
	/**
	offset 76 bit 6 */
	bool isVerboseAuxPid3 : 1;
	/**
	offset 76 bit 7 */
	bool activateAuxPid4 : 1;
	/**
	offset 76 bit 8 */
	bool isVerboseAuxPid4 : 1;
	/**
	 * enable cj125verbose/disable cj125verbose
	offset 76 bit 9 */
	bool isCJ125Verbose : 1;
	/**
	offset 76 bit 10 */
	bool cj125isUaDivided : 1;
	/**
	offset 76 bit 11 */
	bool cj125isLsu49 : 1;
	/**
	offset 76 bit 12 */
	bool etb_use_two_wires : 1;
	/**
	offset 76 bit 13 */
	bool isDoubleSolenoidIdle : 1;
	/**
	offset 76 bit 14 */
	bool showSdCardWarning : 1;
	/**
	 * looks like 3v range should be enough, divider not needed
	offset 76 bit 15 */
	bool cj125isUrDivided : 1;
	/**
	offset 76 bit 16 */
	bool unusedCrapAgain : 1;
	/**
	offset 76 bit 17 */
	bool useTLE8888_cranking_hack : 1;
	/**
	offset 76 bit 18 */
	bool useInstantRpmForIdle : 1;
	/**
	 * If your fuel regulator does not have vacuum line
	offset 76 bit 19 */
	bool absoluteFuelPressure : 1;
	/**
	offset 76 bit 20 */
	bool launchControlEnabled : 1;
	/**
	offset 76 bit 21 */
	bool rollingLaunchEnabled : 1;
	/**
	offset 76 bit 22 */
	bool antiLagEnabled : 1;
	/**
	offset 76 bit 23 */
	bool useRunningMathForCranking : 1;
	/**
	offset 76 bit 24 */
	bool displayLogicLevelsInEngineSniffer : 1;
	/**
	offset 76 bit 25 */
	bool issue_294_26 : 1;
	/**
	offset 76 bit 26 */
	bool issue_294_27 : 1;
	/**
	offset 76 bit 27 */
	bool issue_294_28 : 1;
	/**
	offset 76 bit 28 */
	bool issue_294_29 : 1;
	/**
	offset 76 bit 29 */
	bool issue_294_30 : 1;
	/**
	offset 76 bit 30 */
	bool issue_294_31 : 1;
	/**
	offset 76 bit 31 */
	bool unusedBit_34_31 : 1;
	/**
	 * Closed throttle. todo: extract these two fields into a structure
	 * See also tps1_1AdcChannel
	 * set tps_min X
	 * offset 80
	 */
	int16_t tpsMin;
	/**
	 * Full throttle. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps_max X
	 * offset 82
	 */
	int16_t tpsMax;
	/**
	 * TPS error detection, what TPS % value is unrealistically low
	 * offset 84
	 */
	int16_t tpsErrorDetectionTooLow;
	/**
	 * TPS error detection, what TPS % value is unrealistically high
	 * offset 86
	 */
	int16_t tpsErrorDetectionTooHigh;
	/**
	 * offset 88
	 */
	cranking_parameters_s cranking;
	/**
	 * offset 96
	 */
	float primingSquirtDurationMs;
	/**
	 * Used if useConstantDwellDuringCranking is TRUE
	 * offset 100
	 */
	float ignitionDwellForCrankingMs;
	/**
	 * While cranking (which causes battery voltage to drop) we can calculate dwell time in shaft
	 * degrees, not in absolute time as in running mode.
	 * set cranking_charge_angle X
	 * offset 104
	 */
	float crankingChargeAngle;
	/**
	 * @see hasMapSensor
	 * @see isMapAveragingEnabled
	 * offset 108
	 */
	MAP_sensor_config_s map;
	/**
	 * todo: merge with channel settings, use full-scale Thermistor here!
	 * offset 252
	 */
	ThermistorConf clt;
	/**
	 * offset 284
	 */
	ThermistorConf iat;
	/**
	 * A secondary Rev limit engaged by the driver to help launch the vehicle faster
	 * offset 316
	 */
	int launchRpm;
	/**
	 * offset 320
	 */
	int launchTimingRetard;
	/**
	 * value '6' for 8MHz hw osc
	 * read hip9011 datasheet for details
	 * todo split into two bit fields
	 * offset 324
	 */
	int hip9011PrescalerAndSDO;
	/**
	 * We calculate knock band based of cylinderBore
	 *  Use this to override - kHz knock band override
	 * offset 328
	 */
	float knockBandCustom;
	/**
	 * On single-coil or wasted spark setups you have to lower dwell at high RPM
	 * offset 332
	 */
	float sparkDwellRpmBins[DWELL_CURVE_SIZE];
	/**
	 * offset 364
	 */
	float sparkDwellValues[DWELL_CURVE_SIZE];
	/**
	 * offset 396
	 */
	specs_s specs;
	/**
	 * Cylinder diameter, in mm.
	 * offset 408
	 */
	float cylinderBore;
	/**
	 * Disable sensor sniffer above this rpm
	 * offset 412
	 */
	int sensorSnifferRpmThreshold;
	/**
	 * set rpm_hard_limit X
	 * offset 416
	 */
	int rpmHardLimit;
	/**
	 * This setting controls which fuel quantity control algorithm is used.
	 * See also useTPSAdvanceTable
	 * set algorithm X
	 * offset 420
	 */
	engine_load_mode_e fuelAlgorithm;
	/**
	 * This is the injection strategy during engine start. See Fuel/Injection settings for more detail. It is suggested to use "Simultaneous".
	 * offset 424
	 */
	injection_mode_e crankingInjectionMode;
	/**
	 * This is where the fuel injection type is defined: "Simultaneous" means all injectors will fire together at once. "Sequential" fires the injectors on a per cylinder basis, which requires individually wired injectors. "Batched" will fire the injectors in groups. If your injectors are individually wired you will also need to enable "Two wire batch emulation". 
	 * set injection_mode X
	 * See also twoWireBatchInjection
	 * offset 428
	 */
	injection_mode_e injectionMode;
	/**
	 * this is about deciding when the injector starts it's squirt
	 * See also injectionPhase map
	 * todo: do we need even need this since we have the map anyway?
	 * offset 432
	 */
	angle_t extraInjectionOffset;
	/**
	 * Ignition advance angle used during engine cranking, 5-10 degrees will work as a base setting for most engines.
	 * set cranking_timing_angle X
	 * offset 436
	 */
	angle_t crankingTimingAngle;
	/**
	 * "One Coil" is for use on distributed ignition system. "Individual Coils" is to be used when you have one coil per cylinder (COP or similar). "Wasted" means one coil is driving two spark plugs in two cylinders, with one of the sparks not doing anything since it's happening on the exhaust cycle
	 * set ignition_mode X
	 * offset 440
	 */
	ignition_mode_e ignitionMode;
	/**
	 * this value could be used to offset the whole ignition timing table by a constant
	 * offset 444
	 */
	angle_t ignitionOffset;
	/**
	 * Dynamic uses the timing map to decide the ignition timing, Static timing fixes the timing to the value set below (only use for checking static timing).
	 * offset 448
	 */
	timing_mode_e timingMode;
	/**
	 * This value is the ignition timing used when in 'fixed timing' mode, i.e. constant timing
	 * This mode is useful when adjusting distributor location.
	 * offset 452
	 */
	angle_t fixedModeTiming;
	/**
	 * Angle between Top Dead Center (TDC) and the first trigger event.
	 * Knowing this angle allows us to control timing and other angles in reference to TDC.
	 * set global_trigger_offset_angle X
	 * offset 456
	 */
	angle_t globalTriggerAngleOffset;
	/**
	 * Ratio/coefficient of input voltage dividers on your PCB. For example, use '2' if your board divides 5v into 2.5v. Use '1.66' if your board divides 5v into 3v.
	 * offset 460
	 */
	float analogInputDividerCoefficient;
	/**
	 * This is the ratio of the resistors for the battery voltage, measure the voltage at the battery and then adjust this number until the gauge matches the reading.
	 * offset 464
	 */
	float vbattDividerCoeff;
	/**
	 * Cooling fan turn-on temperature threshold, in Celsius
	 * offset 468
	 */
	float fanOnTemperature;
	/**
	 * Cooling fan turn-off temperature threshold, in Celsius
	 * offset 472
	 */
	float fanOffTemperature;
	/**
	 * This coefficient translates vehicle speed input frequency (in Hz) into vehicle speed, km/h
	 * offset 476
	 */
	float vehicleSpeedCoef;
	/**
	 * set can_mode X
	 * offset 480
	 */
	can_nbc_e canNbcType;
	/**
	 * CANbus thread period, ms
	 * offset 484
	 */
	int canSleepPeriodMs;
	/**
	 * 'Some triggers could be mounted differently. Most well-known triggers imply specific sensor setup. 4 stroke with symmetrical crank' is a pretty special case for example on Miata NB2
	 * See engineCycle
	 * set operation_mode X
	 * offset 488
	 */
	operation_mode_e ambiguousOperationMode;
	/**
	 * offset 492
	 */
	display_mode_e displayMode;
	/**
	 * offset 496
	 */
	log_format_e logFormat;
	/**
	 * offset 500
	 */
	int byFirmwareVersion;
	/**
	 * offset 504
	 */
	int HD44780width;
	/**
	 * offset 508
	 */
	int HD44780height;
	/**
	 * First throttle body, first sensor. See also pedalPositionAdcChannel
	 * offset 512
	 */
	adc_channel_e tps1_1AdcChannel;
	/**
	 * This is the processor input pin that the battery voltage circuit is connected to, if you are unsure of what pin to use, check the schematic that corresponds to your PCB.
	 * offset 513
	 */
	adc_channel_e vbattAdcChannel;
	/**
	 * This is the processor pin that your fuel level sensor in connected to. This is a non standard input so will need to be user defined.
	 * offset 514
	 */
	adc_channel_e fuelLevelSensor;
	/**
	 * Second throttle body position sensor, single channel so far
	 * set_analog_input_pin tps2 X
	 * offset 515
	 */
	adc_channel_e tps2_1AdcChannel;
	/**
	 * offset 516
	 */
	int overrideCrankingIgnition;
	/**
	 * offset 520
	 */
	int sensorChartFrequency;
	/**
	 * offset 524
	 */
	trigger_config_s trigger;
	/**
	 * offset 540
	 */
	spi_device_e hip9011SpiDevice;
	/**
	 * offset 541
	 */
	adc_channel_e high_fuel_pressure_sensor_1;
	/**
	 * offset 542
	 */
	adc_channel_e high_fuel_pressure_sensor_2;
	/**
	 * See hasMafSensor
	 * offset 543
	 */
	adc_channel_e mafAdcChannel;
	/**
	 * set global_fuel_correction X
	 * offset 544
	 */
	float globalFuelCorrection;
	/**
	 * offset 548
	 */
	float adcVcc;
	/**
	 * maximum total number of degrees to subtract from ignition advance
	 * when knocking
	 * offset 552
	 */
	float maxKnockSubDeg;
	/**
	 * Camshaft input could be used either just for engine phase detection if your trigger shape does not include cam sensor as 'primary' channel, or it could be used for Variable Valve timing on one of the camshafts.
	 * TODO #660
	 * offset 556
	 */
	brain_input_pin_e camInputs[CAM_INPUTS_COUNT];
	/**
	 * offset 560
	 */
	afr_sensor_s afr;
	/**
	 * Electronic throttle pedal position input
	 * First channel
	 * See also tps1_1AdcChannel
	 * set_analog_input_pin pps X
	 * offset 580
	 */
	adc_channel_e throttlePedalPositionAdcChannel;
	/**
	 * offset 581
	 */
	brain_pin_e tle6240_cs;
	/**
	 * offset 582
	 */
	pin_output_mode_e tle6240_csPinMode;
	/**
	 * Throttle Pedal not pressed switch - used on some older vehicles like early Mazda Miata
	 * offset 583
	 */
	switch_input_pin_e throttlePedalUpPin;
	/**
	 * @see hasBaroSensor
	 * offset 584
	 */
	air_pressure_sensor_config_s baroSensor;
	/**
	 * offset 600
	 */
	idle_hardware_s idle;
	/**
	 * value between 0 and 100 used in Manual mode
	 * offset 608
	 */
	float manIdlePosition;
	/**
	 * offset 612
	 */
	float mapFrequency0Kpa;
	/**
	 * offset 616
	 */
	float mapFrequency100Kpa;
	/**
	 * Same RPM is used for two ways of producing simulated RPM. See also triggerSimulatorPins (with wires)
	 * See also directSelfStimulation (no wires, bypassing input hardware)
	 * rpm X
	 * offset 620
	 */
	int triggerSimulatorFrequency;
	/**
	 * offset 624
	 */
	output_pin_e injectionPins[INJECTION_PIN_COUNT];
	/**
	 * offset 636
	 */
	output_pin_e ignitionPins[IGNITION_PIN_COUNT];
	/**
	 * offset 648
	 */
	pin_output_mode_e injectionPinMode;
	/**
	 * offset 649
	 */
	pin_output_mode_e ignitionPinMode;
	/**
	 * offset 650
	 */
	brain_pin_e HD44780_rs;
	/**
	 * offset 651
	 */
	brain_pin_e HD44780_e;
	/**
	 * offset 652
	 */
	brain_pin_e HD44780_db4;
	/**
	 * offset 653
	 */
	brain_pin_e HD44780_db5;
	/**
	 * offset 654
	 */
	brain_pin_e HD44780_db6;
	/**
	 * offset 655
	 */
	brain_pin_e HD44780_db7;
	/**
	 * offset 656
	 */
	brain_pin_e gps_rx_pin;
	/**
	 * offset 657
	 */
	brain_pin_e gps_tx_pin;
	/**
	 * offset 658
	 */
	output_pin_e fuelPumpPin;
	/**
	 * offset 659
	 */
	pin_output_mode_e fuelPumpPinMode;
	/**
	 * Check engine light, also malfunction indicator light. Always blinks once on boot.
	 * offset 660
	 */
	output_pin_e malfunctionIndicatorPin;
	/**
	 * offset 661
	 */
	pin_output_mode_e malfunctionIndicatorPinMode;
	/**
	 * offset 662
	 */
	pin_output_mode_e fanPinMode;
	/**
	 * offset 663
	 */
	output_pin_e fanPin;
	/**
	 * some cars have a switch to indicate that clutch pedal is all the way down
	 * offset 664
	 */
	switch_input_pin_e clutchDownPin;
	/**
	 * offset 665
	 */
	output_pin_e alternatorControlPin;
	/**
	 * offset 666
	 */
	pin_output_mode_e alternatorControlPinMode;
	/**
	 * offset 667
	 */
	pin_input_mode_e clutchDownPinMode;
	/**
	 * offset 668
	 */
	brain_pin_e digitalPotentiometerChipSelect[DIGIPOT_COUNT];
	/**
	 * offset 672
	 */
	pin_output_mode_e electronicThrottlePin1Mode;
	/**
	 * offset 673
	 */
	brain_pin_e wboHeaterPin;
	/**
	 * offset 674
	 */
	brain_pin_e cj125CsPin;
	/**
	 * offset 675
	 */
	spi_device_e max31855spiDevice;
	/**
	 * offset 676
	 */
	brain_pin_e debugTriggerSync;
	/**
	 * Digital Potentiometer is used by stock ECU stimulation code
	 * offset 677
	 */
	spi_device_e digitalPotentiometerSpiDevice;
	/**
	 * offset 678
	 */
	brain_pin_e mc33972_cs;
	/**
	 * offset 679
	 */
	pin_output_mode_e mc33972_csPinMode;
	/**
	 * Useful in Research&Development phase
	 * offset 680
	 */
	adc_channel_e auxFastSensor1_adcChannel;
	/**
	 * First throttle body, second sensor.
	 * offset 681
	 */
	adc_channel_e tps1_2AdcChannel;
	/**
	 * Second throttle body, second sensor.
	 * offset 682
	 */
	adc_channel_e tps2_2AdcChannel;
	/**
	 * Electronic throttle pedal position input
	 * Second channel
	 * See also tps1_1AdcChannel
	 * offset 683
	 */
	adc_channel_e throttlePedalPositionSecondAdcChannel;
	/**
	 * offset 684
	 */
	float fuelLevelEmptyTankVoltage;
	/**
	 * offset 688
	 */
	float fuelLevelFullTankVoltage;
	/**
	 * AFR, WBO, EGO - whatever you like to call it
	 * offset 692
	 */
	ego_sensor_e afr_type;
	/**
	 * offset 696
	 */
	float fuelClosedLoopAfrLowThreshold;
	/**
	 * offset 700
	 */
	brain_input_pin_e triggerInputPins[TRIGGER_INPUT_PIN_COUNT];
	/**
	 * offset 703
	 */
	pin_output_mode_e hip9011CsPinMode;
	/**
	 * This implementation produces one pulse per engine cycle. See also dizzySparkOutputPin.
	 * offset 704
	 */
	output_pin_e tachOutputPin;
	/**
	 * offset 705
	 */
	pin_output_mode_e tachOutputPinMode;
	/**
	 * offset 706
	 */
	output_pin_e mainRelayPin;
	/**
	 * offset 707
	 */
	brain_pin_e sdCardCsPin;
	/**
	 * set_can_tx_pin X
	 * offset 708
	 */
	brain_pin_e canTxPin;
	/**
	 * set_can_rx_pin X
	 * offset 709
	 */
	brain_pin_e canRxPin;
	/**
	 * offset 710
	 */
	pin_input_mode_e throttlePedalUpPinMode;
	/**
	 * offset 711
	 */
	brain_pin_e debugTimerCallback;
	/**
	 * offset 712
	 */
	int idleThreadPeriodMs;
	/**
	 * offset 716
	 */
	int consoleLoopPeriodMs;
	/**
	 * offset 720
	 */
	int lcdThreadPeriodMs;
	/**
	 * offset 724
	 */
	int generalPeriodicThreadPeriodMs;
	/**
	 * offset 728
	 */
	uint32_t tunerStudioSerialSpeed;
	/**
	 * offset 732
	 */
	int anUnused4Bytes;
	/**
	 * offset 736
	 */
	brain_pin_e triggerSimulatorPins[TRIGGER_SIMULATOR_PIN_COUNT];
	/**
	 * offset 739
	 */
	pin_output_mode_e triggerSimulatorPinModes[TRIGGER_SIMULATOR_PIN_COUNT];
	/**
	 * Narrow band o2 heater, not used for CJ125. See wboHeaterPin
	 * offset 742
	 */
	output_pin_e o2heaterPin;
	/**
	 * offset 743
	 */
	pin_output_mode_e o2heaterPinModeTodO;
	/**
	offset 744 bit 0 */
	bool is_enabled_spi_1 : 1;
	/**
	offset 744 bit 1 */
	bool is_enabled_spi_2 : 1;
	/**
	offset 744 bit 2 */
	bool is_enabled_spi_3 : 1;
	/**
	offset 744 bit 3 */
	bool isSdCardEnabled : 1;
	/**
	offset 744 bit 4 */
	bool isFastAdcEnabled : 1;
	/**
	offset 744 bit 5 */
	bool isEngineControlEnabled : 1;
	/**
	offset 744 bit 6 */
	bool isHip9011Enabled : 1;
	/**
	offset 744 bit 7 */
	bool isVerboseAlternator : 1;
	/**
	offset 744 bit 8 */
	bool useSerialPort : 1;
	/**
	 * This setting should only be used if you have a stepper motor idle valve and a stepper motor driver installed.
	offset 744 bit 9 */
	bool useStepperIdle : 1;
	/**
	offset 744 bit 10 */
	bool enabledStep1Limiter : 1;
	/**
	offset 744 bit 11 */
	bool useTpicAdvancedMode : 1;
	/**
	offset 744 bit 12 */
	bool useLcdScreen : 1;
	/**
	offset 744 bit 13 */
	bool verboseTLE8888 : 1;
	/**
	offset 744 bit 14 */
	bool enableVerboseCanTx : 1;
	/**
	 *  +This will cause the alternator to be operated in a basic on or off mode, this is the simplest alternator control.
	offset 744 bit 15 */
	bool onOffAlternatorLogic : 1;
	/**
	offset 744 bit 16 */
	bool isCJ125Enabled : 1;
	/**
	 * Use rise or fall signal front
	offset 744 bit 17 */
	bool vvtCamSensorUseRise : 1;
	/**
	 * Useful for individual intakes
	offset 744 bit 18 */
	bool measureMapOnlyInOneCylinder : 1;
	/**
	offset 744 bit 19 */
	bool stepperForceParkingEveryRestart : 1;
	/**
	 * Smarter cranking logic.
	 * See also startOfCrankingPrimingPulse
	offset 744 bit 20 */
	bool isFasterEngineSpinUpEnabled : 1;
	/**
	 * This setting disables fuel injection while the engine is in overrun, this is useful as a fuel saving measure and to prevent back firing.
	offset 744 bit 21 */
	bool coastingFuelCutEnabled : 1;
	/**
	 * This setting allows the ECU to open the IAC during overrun conditions to help reduce engine breaking, this can be helpful for large engines in light weight cars.
	offset 744 bit 22 */
	bool useIacTableForCoasting : 1;
	/**
	offset 744 bit 23 */
	bool useNoiselessTriggerDecoder : 1;
	/**
	offset 744 bit 24 */
	bool useIdleTimingPidControl : 1;
	/**
	offset 744 bit 25 */
	bool useTPSBasedVeTable : 1;
	/**
	offset 744 bit 26 */
	bool is_enabled_spi_4 : 1;
	/**
	offset 744 bit 27 */
	bool pauseEtbControl : 1;
	/**
	offset 744 bit 28 */
	bool alignEngineSnifferAtTDC : 1;
	/**
	 * This setting allows the ETB to act as the idle air control valve and move to regulate the airflow at idle.
	offset 744 bit 29 */
	bool useETBforIdleControl : 1;
	/**
	offset 744 bit 30 */
	bool idleIncrementalPidCic : 1;
	/**
	offset 744 bit 31 */
	bool enableAemXSeries : 1;
	/**
	 * offset 748
	 */
	brain_input_pin_e logicAnalyzerPins[LOGIC_ANALYZER_CHANNEL_COUNT];
	/**
	 * offset 752
	 */
	pin_output_mode_e mainRelayPinMode;
	/**
	 * offset 753
	 */
	brain_pin_e hip9011CsPin;
	/**
	 * offset 754
	 */
	brain_pin_e hip9011IntHoldPin;
	/**
	 * offset 755
	 */
	pin_output_mode_e hip9011IntHoldPinMode;
	/**
	 * offset 756
	 */
	uint32_t verboseCanBaseAddress;
	/**
	 * offset 760
	 */
	uint8_t mc33_hvolt;
	/**
	 * offset 761
	 */
	uint8_t unusedHere[3];
	/**
	 * offset 764
	 */
	pin_output_mode_e gpioPinModes[FSIO_COMMAND_COUNT];
	/**
	 * todo: more comments
	 * offset 780
	 */
	output_pin_e fsioOutputPins[FSIO_COMMAND_COUNT];
	/**
	 * offset 796
	 */
	brain_pin_e max31855_cs[EGT_CHANNEL_COUNT];
	/**
	 * SD card logging period, in milliseconds
	 * offset 804
	 */
	int16_t sdCardPeriodMs;
	/**
	 * offset 806
	 */
	brain_pin_e debugSetTimer;
	/**
	 * offset 807
	 */
	brain_pin_e debugMapAveraging;
	/**
	 * offset 808
	 */
	output_pin_e starterRelayDisablePin;
	/**
	 * On some vehicles we can disable starter once engine is already running
	 * offset 809
	 */
	pin_output_mode_e starterRelayDisableMode;
	/**
	 * Some Subaru and some Mazda use double-solenoid idle air valve
	 * offset 810
	 */
	brain_pin_e secondSolenoidPin;
	/**
	 * See also starterControlPin
	 * offset 811
	 */
	switch_input_pin_e startStopButtonPin;
	/**
	 * offset 812
	 */
	int mapMinBufferLength;
	/**
	 * offset 816
	 */
	int16_t idlePidDeactivationTpsThreshold;
	/**
	 * offset 818
	 */
	int16_t stepperParkingExtraSteps;
	/**
	 * offset 820
	 */
	float unusedAntilagTimeout;
	/**
	 * offset 824
	 */
	int16_t ubusedantiLagRpmTreshold;
	/**
	 * Maximum time to crank starter
	 * offset 826
	 */
	int16_t startCrankingDuration;
	/**
	 * This pin is used for debugging - snap a logic analyzer on it and see if it's ever high
	 * offset 828
	 */
	brain_pin_e triggerErrorPin;
	/**
	 * offset 829
	 */
	pin_output_mode_e triggerErrorPinMode;
	/**
	 * offset 830
	 */
	output_pin_e acRelayPin;
	/**
	 * offset 831
	 */
	pin_output_mode_e acRelayPinMode;
	/**
	 * offset 832
	 */
	fsio_pwm_freq_t fsioFrequency[FSIO_COMMAND_COUNT];
	/**
	 * offset 864
	 */
	fsio_setting_t fsio_setting[FSIO_COMMAND_COUNT];
	/**
	 * offset 928
	 */
	brain_pin_e spi1mosiPin;
	/**
	 * offset 929
	 */
	brain_pin_e spi1misoPin;
	/**
	 * offset 930
	 */
	brain_pin_e spi1sckPin;
	/**
	 * offset 931
	 */
	brain_pin_e spi2mosiPin;
	/**
	 * offset 932
	 */
	brain_pin_e spi2misoPin;
	/**
	 * offset 933
	 */
	brain_pin_e spi2sckPin;
	/**
	 * offset 934
	 */
	brain_pin_e spi3mosiPin;
	/**
	 * offset 935
	 */
	brain_pin_e spi3misoPin;
	/**
	 * offset 936
	 */
	brain_pin_e spi3sckPin;
	/**
	 * Saab Combustion Detection Module knock signal input pin
	 * also known as Saab Ion Sensing Module
	 * offset 937
	 */
	brain_pin_e cdmInputPin;
	/**
	 * offset 938
	 */
	brain_pin_e joystickCenterPin;
	/**
	 * offset 939
	 */
	brain_pin_e joystickAPin;
	/**
	 * offset 940
	 */
	brain_pin_e joystickBPin;
	/**
	 * offset 941
	 */
	brain_pin_e joystickCPin;
	/**
	 * offset 942
	 */
	brain_pin_e joystickDPin;
	/**
	 * offset 943
	 */
	uart_device_e consoleUartDevice;
	/**
	 * offset 944
	 */
	sensor_chart_e sensorChartMode;
	/**
	 * offset 948
	 */
	maf_sensor_type_e mafSensorType;
	/**
	 * todo:not finished
	 * These input pins allow us to pull toggle buttons state
	 * offset 952
	 */
	brain_pin_e fsioDigitalInputs[FSIO_COMMAND_COUNT];
	/**
	 * offset 968
	 */
	brain_input_pin_e vehicleSpeedSensorInputPin;
	/**
	 * Some vehicles have a switch to indicate that clutch pedal is all the way up
	 * offset 969
	 */
	switch_input_pin_e clutchUpPin;
	/**
	 * offset 970
	 */
	brain_input_pin_e frequencyReportingMapInputPin;
	/**
	 * offset 971
	 */
	pin_input_mode_e clutchUpPinMode;
	/**
	 * offset 972
	 */
	uint16_t multisparkMaxRpm;
	/**
	 * offset 974
	 */
	uint8_t multisparkMaxSparkingAngle;
	/**
	 * offset 975
	 */
	uint8_t multisparkMaxExtraSparkCount;
	/**
	offset 976 bit 0 */
	bool todoClutchUpPinInverted : 1;
	/**
	offset 976 bit 1 */
	bool todoClutchDownPinInverted : 1;
	/**
	offset 976 bit 2 */
	bool useHbridges : 1;
	/**
	offset 976 bit 3 */
	bool multisparkEnable : 1;
	/**
	offset 976 bit 4 */
	bool gpPwm1SwitchCondition : 1;
	/**
	offset 976 bit 5 */
	bool gpPwm2SwitchCondition : 1;
	/**
	offset 976 bit 6 */
	bool gpPwm3SwitchCondition : 1;
	/**
	offset 976 bit 7 */
	bool gpPwm4SwitchCondition : 1;
	/**
	offset 976 bit 8 */
	bool isGpPwm1Enabled : 1;
	/**
	offset 976 bit 9 */
	bool isGpPwm2Enabled : 1;
	/**
	offset 976 bit 10 */
	bool isGpPwm3Enabled : 1;
	/**
	offset 976 bit 11 */
	bool isGpPwm4Enabled : 1;
	/**
	offset 976 bit 12 */
	bool useSolenoidIdle : 1;
	/**
	offset 976 bit 13 */
	bool useSecondEtb : 1;
	/**
	offset 976 bit 14 */
	bool enableAntiLagAir : 1;
	/**
	offset 976 bit 15 */
	bool enableAntiLagFuel : 1;
	/**
	offset 976 bit 16 */
	bool syncMode : 1;
	/**
	offset 976 bit 17 */
	bool enableCanVss : 1;
	/**
	offset 976 bit 18 */
	bool useLaunchActivateDelay : 1;
	/**
	offset 976 bit 19 */
	bool unusedBit_251_19 : 1;
	/**
	offset 976 bit 20 */
	bool unusedBit_251_20 : 1;
	/**
	offset 976 bit 21 */
	bool unusedBit_251_21 : 1;
	/**
	offset 976 bit 22 */
	bool unusedBit_251_22 : 1;
	/**
	offset 976 bit 23 */
	bool unusedBit_251_23 : 1;
	/**
	offset 976 bit 24 */
	bool unusedBit_251_24 : 1;
	/**
	offset 976 bit 25 */
	bool unusedBit_251_25 : 1;
	/**
	offset 976 bit 26 */
	bool unusedBit_251_26 : 1;
	/**
	offset 976 bit 27 */
	bool unusedBit_251_27 : 1;
	/**
	offset 976 bit 28 */
	bool unusedBit_251_28 : 1;
	/**
	offset 976 bit 29 */
	bool unusedBit_251_29 : 1;
	/**
	offset 976 bit 30 */
	bool unusedBit_282_30 : 1;
	/**
	offset 976 bit 31 */
	bool unusedBit_282_31 : 1;
	/**
	 * offset 980
	 */
	etb_io etbIo[ETB_COUNT];
	/**
	 * offset 988
	 */
	output_pin_e boostControlPin;
	/**
	 * offset 989
	 */
	pin_output_mode_e boostControlPinMode;
	/**
	 * offset 990
	 */
	switch_input_pin_e antiLagActivatePin;
	/**
	 * offset 991
	 */
	switch_input_pin_e launchActivatePin;
	/**
	 * offset 992
	 */
	pid_s boostPid;
	/**
	 * offset 1012
	 */
	boostType_e boostType;
	/**
	 * offset 1016
	 */
	int boostPwmFrequency;
	/**
	 * offset 1020
	 */
	launchActivationMode_e launchActivationMode;
	/**
	 * offset 1024
	 */
	int unusedMoreCrap;
	/**
	 * Disabled above this speed
	 * offset 1028
	 */
	int launchSpeedTreshold;
	/**
	 * Disabled below this rpm
	 * offset 1032
	 */
	int launchRpmTreshold;
	/**
	 * Range from Launch Rpm for Timing Retard to activate
	 * offset 1036
	 */
	int launchTimingRpmRange;
	/**
	 * Extra Fuel Added
	 * offset 1040
	 */
	int launchFuelAdded;
	/**
	 * Duty Cycle for the Boost Solenoid
	 * offset 1044
	 */
	int launchBoostDuty;
	/**
	 * RPM Range for Hard Cut
	 * offset 1048
	 */
	int hardCutRpmRange;
	/**
	 * offset 1052
	 */
	int launchAdvanceRpmRange;
	/**
	 * offset 1056
	 */
	int launchTpsTreshold;
	/**
	 * Time in Seconds
	 * offset 1060
	 */
	float launchActivateDelay;
	/**
	 * offset 1064
	 */
	vvtsettings vvt;
	/**
	 * offset 1108
	 */
	brain_pin_e cj125ModePin;
	/**
	 * offset 1109
	 */
	pin_output_mode_e cj125ModePinMode;
	/**
	 * offset 1110
	 */
	uint8_t unusedmorecrapagain[2];
	/**
	 * offset 1112
	 */
	stft_s stft;
	/**
	 * offset 1136
	 */
	gppwm gpPwm[GP_PWM_COUNT];
	/**
	 * offset 1392
	 */
	antilag antiLag;
	/**
	 * offset 1432
	 */
	int setEtbDuty;
	/**
	 * offset 1436
	 */
	int unusedAtOldBoardConfigurationEnd[7];
	/**
	offset 1464 bit 0 */
	bool vvtDisplayInverted : 1;
	/**
	 * Enables lambda sensor closed loop feedback for fuelling.
	offset 1464 bit 1 */
	bool fuelClosedLoopCorrectionEnabled : 1;
	/**
	offset 1464 bit 2 */
	bool isVerboseIAC : 1;
	/**
	offset 1464 bit 3 */
	bool isVerboseETB : 1;
	/**
	 * If set to true, will use the specified duration for cranking dwell. If set to false, will use the specified dwell angle. Unless you have a really good reason to, leave this set to true to use duration mode.
	offset 1464 bit 4 */
	bool useConstantDwellDuringCranking : 1;
	/**
	 * This options enables data for 'engine sniffer' tab in console, which comes at some CPU price
	offset 1464 bit 5 */
	bool isEngineChartEnabled : 1;
	/**
	 * Sometimes we have a performance issue while printing error
	offset 1464 bit 6 */
	bool silentTriggerError : 1;
	/**
	offset 1464 bit 7 */
	bool useLinearCltSensor : 1;
	/**
	offset 1464 bit 8 */
	bool canReadEnabled : 1;
	/**
	offset 1464 bit 9 */
	bool canWriteEnabled : 1;
	/**
	offset 1464 bit 10 */
	bool useLinearIatSensor : 1;
	/**
	 * See fsioTimingAdjustment
	offset 1464 bit 11 */
	bool useFSIO16ForTimingAdjustment : 1;
	/**
	offset 1464 bit 12 */
	bool tachPulseDurationAsDutyCycle : 1;
	/**
	 * This enables smart alternator control and activates the extra alternator settings.
	offset 1464 bit 13 */
	bool isAlternatorControlEnabled : 1;
	/**
	 * This setting flips the signal from the primary engine speed sensor.
	offset 1464 bit 14 */
	bool invertPrimaryTriggerSignal : 1;
	/**
	 * This setting flips the signal from the secondary engine speed sensor.
	offset 1464 bit 15 */
	bool invertSecondaryTriggerSignal : 1;
	/**
	offset 1464 bit 16 */
	bool cutFuelOnHardLimit : 1;
	/**
	offset 1464 bit 17 */
	bool cutSparkOnHardLimit : 1;
	/**
	offset 1464 bit 18 */
	bool launchFuelCutEnable : 1;
	/**
	 * This is the Cut Mode normally used
	offset 1464 bit 19 */
	bool launchSparkCutEnable : 1;
	/**
	offset 1464 bit 20 */
	bool hasFrequencyReportingMapSensor : 1;
	/**
	offset 1464 bit 21 */
	bool useFSIO8ForServo1 : 1;
	/**
	offset 1464 bit 22 */
	bool useFSIO9ForServo2 : 1;
	/**
	offset 1464 bit 23 */
	bool useFSIO10ForServo3 : 1;
	/**
	offset 1464 bit 24 */
	bool useFSIO11ForServo4 : 1;
	/**
	offset 1464 bit 25 */
	bool useFSIO12ForServo5 : 1;
	/**
	offset 1464 bit 26 */
	bool useFSIO15ForIdleRpmAdjustment : 1;
	/**
	 * Sometimes we just have to shut the engine down. Use carefully!
	offset 1464 bit 27 */
	bool useFSIO5ForCriticalIssueEngineStop : 1;
	/**
	 * Sometimes we have to miss injection on purpose to attract driver's attention
	offset 1464 bit 28 */
	bool useFSIO4ForSeriousEngineWarning : 1;
	/**
	offset 1464 bit 29 */
	bool useFSIO12ForIdleOffset : 1;
	/**
	offset 1464 bit 30 */
	bool useFSIO13ForIdleMinValue : 1;
	/**
	offset 1464 bit 31 */
	bool useFSIO6ForRevLimiter : 1;
	/**
	 * offset 1468
	 */
	adc_channel_e hipOutputChannel;
	/**
	 * A/C button input handled as analogue input
	 * offset 1469
	 */
	adc_channel_e acSwitchAdc;
	/**
	 * offset 1470
	 */
	adc_channel_e vRefAdcChannel;
	/**
	 * Expected neutral position
	 * offset 1471
	 */
	uint8_t etbNeutralPosition;
	/**
	 * See also idleRpmPid
	 * offset 1472
	 */
	idle_mode_e idleMode;
	/**
	 * Enable fuel injection - This is default off for new projects as a safety feature, set to "true" to enable fuel injection and further injector settings.
	offset 1476 bit 0 */
	bool isInjectionEnabled : 1;
	/**
	 * Enable ignition - This is default off for new projects as a safety feature, set to "true" to enable ignition and further ignition settings.
	offset 1476 bit 1 */
	bool isIgnitionEnabled : 1;
	/**
	 * When enabled if TPS is held above 95% no fuel is injected while cranking to clear excess fuel from the cylinders.
	offset 1476 bit 2 */
	bool isCylinderCleanupEnabled : 1;
	/**
	offset 1476 bit 3 */
	bool secondTriggerChannelEnabled : 1;
	/**
	offset 1476 bit 4 */
	bool enableLaunchBoost : 1;
	/**
	offset 1476 bit 5 */
	bool isMapAveragingEnabled : 1;
	/**
	 * This setting overrides the normal multiplication values that have been set for the idle air control valve during cranking. If this setting is enabled the "IAC multiplier" table in the Cranking settings tab needs to be adjusted appropriately or potentially no IAC opening will occur.
	offset 1476 bit 6 */
	bool overrideCrankingIacSetting : 1;
	/**
	 * This activates a separate ignition timing table for idle conditions, this can help idle stability by using ignition retard and advance either side of the desired idle speed. Extra retard at low idle speeds will prevent stalling and extra advance at high idle speeds can help reduce engine power and slow the idle speed.
	offset 1476 bit 7 */
	bool useSeparateAdvanceForIdle : 1;
	/**
	offset 1476 bit 8 */
	bool isTunerStudioEnabled : 1;
	/**
	offset 1476 bit 9 */
	bool isWaveAnalyzerEnabled : 1;
	/**
	 * This activates a separate fuel table for Idle, this allows fine tuning of the idle fuelling.
	offset 1476 bit 10 */
	bool useSeparateVeForIdle : 1;
	/**
	 * enable trigger_details
	offset 1476 bit 11 */
	bool verboseTriggerSynchDetails : 1;
	/**
	 * Usually if we have no trigger events that means engine is stopped
	 * Unless we are troubleshooting and spinning the engine by hand - this case a longer
	 * delay is needed
	offset 1476 bit 12 */
	bool isManualSpinningMode : 1;
	/**
	 * This is needed if your coils are individually wired and you wish to use batch injection.
	 * enable two_wire_batch_injection
	offset 1476 bit 13 */
	bool twoWireBatchInjection : 1;
	/**
	 * VR sensors are only precise on rising front
	 * enable trigger_only_front
	offset 1476 bit 14 */
	bool useOnlyRisingEdgeForTrigger : 1;
	/**
	 * This is needed if your coils are individually wired (COP) and you wish to use batch ignition (wasted spark).
	offset 1476 bit 15 */
	bool twoWireBatchIgnition : 1;
	/**
	offset 1476 bit 16 */
	bool useFixedBaroCorrFromMap : 1;
	/**
	 * This activates a separate advance table for cranking conditions, this allows cranking advance to be RPM dependant.
	offset 1476 bit 17 */
	bool useSeparateAdvanceForCranking : 1;
	/**
	 * This enables the various ignition corrections during cranking (IAT, CLT, FSIO and PID idle).
	offset 1476 bit 18 */
	bool useAdvanceCorrectionsForCranking : 1;
	/**
	 * This flag allows to use TPS for ignition lookup while in Speed Density Fuel Mode
	offset 1476 bit 19 */
	bool useTPSAdvanceTable : 1;
	/**
	offset 1476 bit 20 */
	bool etbCalibrationOnStart : 1;
	/**
	 * This flag allows to use a special 'PID Multiplier' table (0.0-1.0) to compensate for nonlinear nature of IAC-RPM controller
	offset 1476 bit 21 */
	bool useIacPidMultTable : 1;
	/**
	offset 1476 bit 22 */
	bool isBoostControlEnabled : 1;
	/**
	 * Interpolates the Ignition Retard from 0 to 100% within the RPM Range
	offset 1476 bit 23 */
	bool launchSmoothRetard : 1;
	/**
	offset 1476 bit 24 */
	bool isVvtControlEnabled : 1;
	/**
	offset 1476 bit 25 */
	bool setEtbErrorRpm : 1;
	/**
	offset 1476 bit 26 */
	bool sensorType : 1;
	/**
	offset 1476 bit 27 */
	bool etbPidPaused : 1;
	/**
	offset 1476 bit 28 */
	bool enableAntiLagBoost : 1;
	/**
	offset 1476 bit 29 */
	bool enableAntiLagRetard : 1;
	/**
	offset 1476 bit 30 */
	bool launchDisableBySpeed : 1;
	/**
	offset 1476 bit 31 */
	bool enableLaunchRetard : 1;
	/**
	 * offset 1480
	 */
	uint32_t engineChartSize;
	/**
	 * Relative to the target idle RPM
	 * offset 1484
	 */
	int16_t idlePidRpmUpperLimit;
	/**
	 * This sets the temperature above which no priming pulse is used, The value at -40 is reduced until there is no more priming injection at this temperature.
	 * offset 1486
	 */
	int16_t primeInjFalloffTemperature;
	/**
	 * At what trigger index should some ignition-related math be executed? This is a performance trick to reduce load on synchronization trigger callback.
	 * offset 1488
	 */
	int ignMathCalculateAtIndex;
	/**
	 * offset 1492
	 */
	int16_t acCutoffLowRpm;
	/**
	 * offset 1494
	 */
	int16_t acCutoffHighRpm;
	/**
	 * offset 1496
	 */
	int16_t acIdleRpmBump;
	/**
	 * set warningPeriod X
	 * offset 1498
	 */
	int16_t warningPeriod;
	/**
	 * offset 1500
	 */
	float knockDetectionWindowStart;
	/**
	 * offset 1504
	 */
	float knockDetectionWindowEnd;
	/**
	 * offset 1508
	 */
	float idleStepperReactionTime;
	/**
	 * offset 1512
	 */
	float knockVThreshold;
	/**
	 * offset 1516
	 */
	pin_input_mode_e fsioInputModes[FSIO_COMMAND_COUNT];
	/**
	 * offset 1532
	 */
	int idleStepperTotalSteps;
	/**
	 * TODO: finish this #413
	 * offset 1536
	 */
	float noAccelAfterHardLimitPeriodSecs;
	/**
	 * At what trigger index should some MAP-related math be executed? This is a performance trick to reduce load on synchronization trigger callback.
	 * offset 1540
	 */
	int mapAveragingSchedulingAtIndex;
	/**
	 * offset 1544
	 */
	float baroCorrPressureBins[BARO_CORR_SIZE];
	/**
	 * offset 1560
	 */
	float baroCorrRpmBins[BARO_CORR_SIZE];
	/**
	 * offset 1576
	 */
	baro_corr_table_t baroCorrTable;
	/**
	 * Cranking fuel correction coefficient based on TPS
	 * offset 1640
	 */
	float crankingTpsCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 1672
	 */
	float crankingTpsBins[CRANKING_CURVE_SIZE];
	/**
	 * offset 1704
	 */
	float tachPulseDuractionMs;
	/**
	 * Trigger cycle index at which we start tach pulse (performance consideration)
	 * offset 1708
	 */
	int tachPulseTriggerIndex;
	/**
	 * Length of time the deposited wall fuel takes to dissipate after the start of acceleration. 
	 * offset 1712
	 */
	float wwaeTau;
	/**
	 * offset 1716
	 */
	pid_s alternatorControl;
	/**
	 * offset 1736
	 */
	pid_s etb;
	/**
	 * offset 1756
	 */
	float fuelRailPressure;
	/**
	 * offset 1760
	 */
	float alternator_derivativeFilterLoss;
	/**
	 * offset 1764
	 */
	float alternator_antiwindupFreq;
	/**
	 * Closed throttle#2. todo: extract these two fields into a structure
	 * See also tps2_1AdcChannel
	 * set tps2_min X
	 * offset 1768
	 */
	int16_t tps2Min;
	/**
	 * Full throttle#2. tpsMax value as 10 bit ADC value. Not Voltage!
	 * See also tps1_1AdcChannel
	 * set tps2_max X
	 * offset 1770
	 */
	int16_t tps2Max;
	/**
	 * See also startStopButtonPin
	 * offset 1772
	 */
	output_pin_e starterControlPin;
	/**
	 * offset 1773
	 */
	pin_input_mode_e startStopButtonMode;
	/**
	 * offset 1774
	 */
	brain_pin_e mc33816_flag0;
	/**
	 * offset 1775
	 */
	uint8_t tachPulsePerRev;
	/**
	 * kPa value which is too low to be true
	 * offset 1776
	 */
	float mapErrorDetectionTooLow;
	/**
	 * kPa value which is too high to be true
	 * offset 1780
	 */
	float mapErrorDetectionTooHigh;
	/**
	 * offset 1784
	 */
	uint16_t multisparkSparkDuration;
	/**
	 * offset 1786
	 */
	uint16_t multisparkDwell;
	/**
	 * See cltIdleRpmBins
	 * offset 1788
	 */
	pid_s idleRpmPid;
	/**
	 * 0 = No fuel settling on port walls 1 = All the fuel settling on port walls setting this to 0 disables the wall wetting enrichment. 
	 * offset 1808
	 */
	float wwaeBeta;
	/**
	 * blue LED on discovery by default
	 * offset 1812
	 */
	brain_pin_e communicationLedPin;
	/**
	 * green LED on discovery by default
	 * offset 1813
	 */
	brain_pin_e runningLedPin;
	/**
	 * offset 1814
	 */
	brain_pin_e binarySerialTxPin;
	/**
	 * offset 1815
	 */
	brain_pin_e binarySerialRxPin;
	/**
	 * offset 1816
	 */
	brain_pin_e auxValves[AUX_DIGITAL_VALVE_COUNT];
	/**
	 *  todo: finish pin migration from hard-coded to configurable?
	 * offset 1818
	 */
	brain_pin_e consoleSerialTxPin;
	/**
	 * todo: finish pin migration from hard-coded to configurable?
	 * offset 1819
	 */
	brain_pin_e consoleSerialRxPin;
	/**
	 * Knock sensor output knock detection threshold depending on current RPM
	 * offset 1820
	 */
	float knockNoise[ENGINE_NOISE_CURVE_SIZE];
	/**
	 * offset 1852
	 */
	float knockNoiseRpmBins[ENGINE_NOISE_CURVE_SIZE];
	/**
	 * offset 1884
	 */
	float throttlePedalUpVoltage;
	/**
	 * Pedal in the floor
	 * offset 1888
	 */
	float throttlePedalWOTVoltage;
	/**
	 * on ECU start turn fuel pump on to build fuel pressure
	 * offset 1892
	 */
	int16_t startUpFuelPumpDuration;
	/**
	 * If RPM is close enough let's leave IAC alone, and maybe engage timing PID correction
	 * offset 1894
	 */
	int16_t idlePidRpmDeadZone;
	/**
	 * CLT-based target RPM for automatic idle controller
	 * offset 1896
	 */
	float cltIdleRpmBins[CLT_CURVE_SIZE];
	/**
	 * See idleRpmPid
	 * offset 1960
	 */
	float cltIdleRpm[CLT_CURVE_SIZE];
	/**
	 * This is the target battery voltage the alternator PID control will attempt to maintain
	 * offset 2024
	 */
	float targetVBatt;
	/**
	 * Turns off alternator output above specified TPS, enabling this reduced parasitic drag on the engine at full load.
	 * offset 2028
	 */
	float alternatorOffAboveTps;
	/**
	 * Prime pulse for cold engine, duration in ms
	 * Linear interpolation between -40F/-40C and fallout temperature
	 * 
	 * See also isFasterEngineSpinUpEnabled
	 * set cranking_priming_pulse X
	 * offset 2032
	 */
	float startOfCrankingPrimingPulse;
	/**
	 * This is the duration in cycles that the IAC will take to reach its normal idle position, it can be used to hold the idle higher for a few seconds after cranking to improve startup.
	 * offset 2036
	 */
	int16_t afterCrankingIACtaperDuration;
	/**
	 * Extra IAC, in percent between 0 and 100, tapered between zero and idle deactivation TPS value
	 * offset 2038
	 */
	int16_t iacByTpsTaper;
	/**
	 * set_aux_tx_pin X
	 * offset 2040
	 */
	brain_pin_e auxSerialTxPin;
	/**
	 * offset 2041
	 */
	brain_pin_e warningLedPin;
	/**
	 * set_aux_rx_pin X
	 * offset 2042
	 */
	brain_pin_e auxSerialRxPin;
	/**
	 * offset 2043
	 */
	brain_pin_e LIS302DLCsPin;
	/**
	 * This is the number of engine cycles that the TPS position change can occur over, a longer duration will make the enrichment more active but too long may affect steady state driving, a good default is 30-60 cycles. 
	 * offset 2044
	 */
	int tpsAccelLength;
	/**
	 * Maximum change delta of TPS percentage over the 'length'. Actual TPS change has to be above this value in order for TPS/TPS acceleration to kick in.
	 * offset 2048
	 */
	float tpsAccelEnrichmentThreshold;
	/**
	 * Angle between cam sensor and VVT zero position
	 * set vvt_offset X
	 * offset 2052
	 */
	float vvtOffset;
	/**
	 * offset 2056
	 */
	int engineLoadAccelLength;
	/**
	 * offset 2060
	 */
	float engineLoadDecelEnleanmentThreshold;
	/**
	 * offset 2064
	 */
	float engineLoadDecelEnleanmentMultiplier;
	/**
	 * offset 2068
	 */
	float engineLoadAccelEnrichmentThreshold;
	/**
	 * offset 2072
	 */
	float engineLoadAccelEnrichmentMultiplier;
	/**
	 * offset 2076
	 */
	uint32_t uartConsoleSerialSpeed;
	/**
	 * offset 2080
	 */
	float tpsDecelEnleanmentThreshold;
	/**
	 * offset 2084
	 */
	float tpsDecelEnleanmentMultiplier;
	/**
	 * ExpAverage alpha coefficient
	 * offset 2088
	 */
	float slowAdcAlpha;
	/**
	 * offset 2092
	 */
	debug_mode_e debugMode;
	/**
	 * offset 2096
	 */
	uint32_t auxSerialSpeed;
	/**
	 * offset 2100
	 */
	uint32_t unused_former_warmup_target_afr[8];
	/**
	 * kPa value at which we need to cut fuel and spark, 0 if not enabled
	 * offset 2132
	 */
	float boostCutPressure;
	/**
	 * offset 2136
	 */
	float mapAccelTaperBins[MAP_ACCEL_TAPER];
	/**
	 * offset 2168
	 */
	float mapAccelTaperMult[MAP_ACCEL_TAPER];
	/**
	 * todo: rename to fsioAnalogInputs
	 * offset 2200
	 */
	adc_channel_e fsioAdc[FSIO_ANALOG_INPUT_COUNT];
	/**
	 * Fixed timing, useful for TDC testing
	 * offset 2204
	 */
	float fixedTiming;
	/**
	 * MAP voltage for low point
	 * offset 2208
	 */
	float mapLowValueVoltage;
	/**
	 * MAP voltage for low point
	 * offset 2212
	 */
	float mapHighValueVoltage;
	/**
	 * EGO value correction
	 * offset 2216
	 */
	float egoValueShift;
	/**
	 * offset 2220
	 */
	output_pin_e auxPidPins[AUX_PID_COUNT];
	/**
	 * offset 2224
	 */
	spi_device_e cj125SpiDevice;
	/**
	 * offset 2225
	 */
	pin_output_mode_e cj125CsPinMode;
	/**
	 * This implementation makes a pulse every time one of the coils is charged, using coil dwell for pulse width. See also tachOutputPin
	 * offset 2226
	 */
	output_pin_e dizzySparkOutputPin;
	/**
	 * offset 2227
	 */
	pin_output_mode_e dizzySparkOutputPinMode;
	/**
	 * This is the IAC position during cranking, some engines start better if given more air during cranking to improve cylinder filling.
	 * offset 2228
	 */
	int crankingIACposition;
	/**
	 * offset 2232
	 */
	float tChargeMinRpmMinTps;
	/**
	 * offset 2236
	 */
	float tChargeMinRpmMaxTps;
	/**
	 * offset 2240
	 */
	float tChargeMaxRpmMinTps;
	/**
	 * offset 2244
	 */
	float tChargeMaxRpmMaxTps;
	/**
	 * offset 2248
	 */
	fsio_pwm_freq_t auxPidFrequency[AUX_PID_COUNT];
	/**
	 * offset 2256
	 */
	int alternatorPwmFrequency;
	/**
	 * offset 2260
	 */
	mass_storage_e storageMode;
	/**
	 * Narrow Band WBO Approximation
	 * offset 2264
	 */
	float narrowToWideOxygenBins[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * offset 2296
	 */
	float narrowToWideOxygen[NARROW_BAND_WIDE_BAND_CONVERSION_SIZE];
	/**
	 * set vvt_mode X
	 * offset 2328
	 */
	vvt_mode_e vvtMode;
	/**
	 * offset 2332
	 */
	uint8_t unusedOldBiquad[20];
	/**
	 * CLT-based timing correction
	 * offset 2352
	 */
	float cltTimingBins[CLT_TIMING_CURVE_SIZE];
	/**
	 * offset 2384
	 */
	float cltTimingExtra[CLT_TIMING_CURVE_SIZE];
	/**
	 * offset 2416
	 */
	tle8888_mode_e tle8888mode;
	/**
	 * offset 2417
	 */
	uint8_t unusedSomethingWasHere[3];
	/**
	 * offset 2420
	 */
	float autoTuneCltThreshold;
	/**
	 * offset 2424
	 */
	float autoTuneTpsRocThreshold;
	/**
	 * offset 2428
	 */
	float autoTuneTpsQuietPeriod;
	/**
	 * offset 2432
	 */
	float unused2432;
	/**
	 * Fuel multiplier taper, see also postCrankingDurationSec
	 * offset 2436
	 */
	float postCrankingFactor;
	/**
	 * See also postCrankingFactor
	 * offset 2440
	 */
	float postCrankingDurationSec;
	/**
	 * todo: finish implementation #332
	 * offset 2444
	 */
	ThermistorConf auxTempSensor1;
	/**
	 * todo: finish implementation #332
	 * offset 2476
	 */
	ThermistorConf auxTempSensor2;
	/**
	 * offset 2508
	 */
	int16_t fuelClosedLoopCltThreshold;
	/**
	 * offset 2510
	 */
	int16_t fuelClosedLoopTpsThreshold;
	/**
	 * offset 2512
	 */
	int16_t fuelClosedLoopRpmThreshold;
	/**
	 * offset 2514
	 */
	int16_t etbFreq;
	/**
	 * offset 2516
	 */
	pid_s fuelClosedLoopPid;
	/**
	 * offset 2536
	 */
	float fuelClosedLoopAfrHighThreshold;
	/**
	 * per-cylinder timing correction
	 * offset 2540
	 */
	cfg_float_t_1f timing_offset_cylinder[IGNITION_PIN_COUNT];
	/**
	 * offset 2588
	 */
	float idlePidActivationTime;
	/**
	 * offset 2592
	 */
	spi_device_e sdCardSpiDevice;
	/**
	 * offset 2593
	 */
	uint8_t unusedSpiPadding4[3];
	/**
	 * offset 2596
	 */
	pin_mode_e spi1SckMode;
	/**
	 * Modes count be used for 3v<>5v integration using pull-ups/pull-downs etc.
	 * offset 2597
	 */
	pin_mode_e spi1MosiMode;
	/**
	 * offset 2598
	 */
	pin_mode_e spi1MisoMode;
	/**
	 * offset 2599
	 */
	pin_mode_e spi2SckMode;
	/**
	 * offset 2600
	 */
	pin_mode_e spi2MosiMode;
	/**
	 * offset 2601
	 */
	pin_mode_e spi2MisoMode;
	/**
	 * offset 2602
	 */
	pin_mode_e spi3SckMode;
	/**
	 * offset 2603
	 */
	pin_mode_e spi3MosiMode;
	/**
	 * offset 2604
	 */
	pin_mode_e spi3MisoMode;
	/**
	 * offset 2605
	 */
	pin_output_mode_e stepperEnablePinMode;
	/**
	 * ResetB
	 * offset 2606
	 */
	brain_pin_e mc33816_rstb;
	/**
	 * offset 2607
	 */
	brain_pin_e mc33816_driven;
	/**
	 * Brake pedal switch
	 * offset 2608
	 */
	switch_input_pin_e brakePedalPin;
	/**
	 * lambda input
	 * offset 2609
	 */
	adc_channel_e cj125ua;
	/**
	 * heater input
	 * offset 2610
	 */
	adc_channel_e cj125ur;
	/**
	 * offset 2611
	 */
	pin_input_mode_e brakePedalPinMode;
	/**
	 * offset 2612
	 */
	pid_s auxPid[AUX_PID_COUNT];
	/**
	 * offset 2692
	 */
	oil_pressure_config_s oilPressure;
	/**
	 * offset 2712
	 */
	spi_device_e accelerometerSpiDevice;
	/**
	 * offset 2713
	 */
	uint8_t unusedSpiPadding5[3];
	/**
	 * offset 2716
	 */
	float fsioCurve1Bins[FSIO_CURVE_16];
	/**
	 * offset 2780
	 */
	float fsioCurve1[FSIO_CURVE_16];
	/**
	 * offset 2844
	 */
	float fsioCurve2Bins[FSIO_CURVE_16];
	/**
	 * offset 2908
	 */
	float fsioCurve2[FSIO_CURVE_16];
	/**
	 * offset 2972
	 */
	float fsioCurve3Bins[FSIO_CURVE_8];
	/**
	 * offset 3004
	 */
	float fsioCurve3[FSIO_CURVE_8];
	/**
	 * offset 3036
	 */
	float fsioCurve4Bins[FSIO_CURVE_8];
	/**
	 * offset 3068
	 */
	float fsioCurve4[FSIO_CURVE_8];
	/**
	 * offset 3100
	 */
	uint8_t unusedFlexFuelSensor;
	/**
	 * offset 3101
	 */
	brain_pin_e test557pin;
	/**
	 * offset 3102
	 */
	pin_output_mode_e stepperDirectionPinMode;
	/**
	 * offset 3103
	 */
	adc_channel_e externalKnockSenseAdc;
	/**
	 * offset 3104
	 */
	brain_pin_e stepperEnablePin;
	/**
	 * offset 3105
	 */
	brain_pin_e tle8888_cs;
	/**
	 * offset 3106
	 */
	pin_output_mode_e tle8888_csPinMode;
	/**
	 * offset 3107
	 */
	brain_pin_e mc33816_cs;
	/**
	 * Optional timing advance table for Cranking (see useSeparateAdvanceForCranking)
	 * offset 3108
	 */
	float crankingAdvanceBins[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Cranking (see useSeparateAdvanceForCranking)
	 * offset 3124
	 */
	float crankingAdvance[CRANKING_ADVANCE_CURVE_SIZE];
	/**
	 * todo: more comments
	 * offset 3140
	 */
	brain_pin_e servoOutputPins[SERVO_COUNT];
	/**
	 * This sets the RPM limit above which the fuel cut is deactivated, activating this maintains fuel flow at high RPM to help cool pistons
	 * offset 3148
	 */
	int16_t coastingFuelCutRpmHigh;
	/**
	 * This sets the RPM limit below which the fuel cut is deactivated, this prevents jerking or issues transitioning to idle
	 * offset 3150
	 */
	int16_t coastingFuelCutRpmLow;
	/**
	 * percent between 0 and 100 below which the fuel cut is deactivated, this helps low speed drivability.
	 * offset 3152
	 */
	int16_t coastingFuelCutTps;
	/**
	 * Fuel cutoff is deactivated below this coolant threshold.
	 * offset 3154
	 */
	int16_t coastingFuelCutClt;
	/**
	 * Increases PID reaction for RPM<target by adding extra percent to PID-error
	 * offset 3156
	 */
	int16_t pidExtraForLowRpm;
	/**
	 * MAP value above which fuel injection is re-enabled.
	 * offset 3158
	 */
	int16_t coastingFuelCutMap;
	/**
	 * CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 3160
	 */
	float iacCoastingBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position for coasting (used in Auto-PID Idle mode)
	 * offset 3224
	 */
	float iacCoasting[CLT_CURVE_SIZE];
	/**
	 * offset 3288
	 */
	ignition_tps_table_t ignitionTpsTable;
	/**
	 * offset 3800
	 */
	float ignitionTpsBins[IGN_TPS_COUNT];
	/**
	 * offset 3864
	 */
	float tChargeAirCoefMin;
	/**
	 * offset 3868
	 */
	float tChargeAirCoefMax;
	/**
	 * offset 3872
	 */
	float tChargeAirFlowMax;
	/**
	 * offset 3876
	 */
	float tChargeAirIncrLimit;
	/**
	 * offset 3880
	 */
	float tChargeAirDecrLimit;
	/**
	 * offset 3884
	 */
	tChargeMode_e tChargeMode;
	/**
	 * target TPS value, 0 to 100%
	 * TODO: use int8 data date once we template interpolation method
	 * offset 3888
	 */
	float etbBiasBins[ETB_BIAS_CURVE_LENGTH];
	/**
	 * offset 3912
	 */
	uint8_t unused1735[8];
	/**
	 * PWM bias, 0 to 100%
	 * offset 3920
	 */
	float etbBiasValues[ETB_BIAS_CURVE_LENGTH];
	/**
	 * offset 3944
	 */
	uint8_t unused1733[8];
	/**
	 * offset 3952
	 */
	float hip9011Gain;
	/**
	 * iTerm min value
	 * offset 3956
	 */
	int16_t etb_iTermMin;
	/**
	 * iTerm max value
	 * offset 3958
	 */
	int16_t etb_iTermMax;
	/**
	 * offset 3960
	 */
	float etbDeadband;
	/**
	 * offset 3964
	 */
	uint8_t unused1059[4];
	/**
	 * See useIdleTimingPidControl
	 * offset 3968
	 */
	pid_s idleTimingPid;
	/**
	 * When the current RPM is closer than this value to the target, closed-loop idle timing control is enabled.
	 * offset 3988
	 */
	int16_t idleTimingPidWorkZone;
	/**
	 * If the RPM closer to target than this value, disable timing correction to prevent oscillation
	 * offset 3990
	 */
	int16_t idleTimingPidDeadZone;
	/**
	 * Taper out idle timing control over this range as the engine leaves idle conditions
	 * offset 3992
	 */
	int16_t idlePidFalloffDeltaRpm;
	/**
	 * A delay in cycles between fuel-enrich. portions
	 * offset 3994
	 */
	int16_t tpsAccelFractionPeriod;
	/**
	 * A fraction divisor: 1 or less = entire portion at once, or split into diminishing fractions
	 * offset 3996
	 */
	float tpsAccelFractionDivisor;
	/**
	 * offset 4000
	 */
	spi_device_e tle8888spiDevice;
	/**
	 * offset 4001
	 */
	spi_device_e mc33816spiDevice;
	/**
	 * iTerm min value
	 * offset 4002
	 */
	int16_t idlerpmpid_iTermMin;
	/**
	 * offset 4004
	 */
	spi_device_e tle6240spiDevice;
	/**
	 * offset 4005
	 */
	uint8_t unusedSpiPadding7;
	/**
	 * iTerm max value
	 * offset 4006
	 */
	int16_t idlerpmpid_iTermMax;
	/**
	 * offset 4008
	 */
	spi_device_e mc33972spiDevice;
	/**
	 * offset 4009
	 */
	uint8_t unusedSpiPadding8[3];
	/**
	 *  ETB idle authority
	 * offset 4012
	 */
	float etbIdleThrottleRange;
	/**
	 * offset 4016
	 */
	uint8_t unusuedvref[4];
	/**
	 * offset 4020
	 */
	uint8_t unusuedsw[4];
	/**
	 * offset 4024
	 */
	int alFIn[3];
	/**
	 * Trigger comparator center point voltage
	 * offset 4036
	 */
	uint8_t triggerCompCenterVolt;
	/**
	 * Trigger comparator hysteresis voltage (Min)
	 * offset 4037
	 */
	uint8_t triggerCompHystMin;
	/**
	 * Trigger comparator hysteresis voltage (Max)
	 * offset 4038
	 */
	uint8_t triggerCompHystMax;
	/**
	 * VR-sensor saturation RPM
	 * offset 4039
	 */
	uint8_t triggerCompSensorSatRpm;
	/**
	 * offset 4040
	 */
	pid_s idleRpmPid2;
	/**
	 * offset 4060
	 */
	iac_pid_mult_t iacPidMultTable;
	/**
	 * offset 4124
	 */
	uint8_t iacPidMultLoadBins[IAC_PID_MULT_SIZE];
	/**
	 * offset 4132
	 */
	uint8_t iacPidMultRpmBins[IAC_PID_MULT_SIZE];
	/**
	 * set can_vss X
	 * offset 4140
	 */
	can_vss_nbc_e canVssNbcType;
	/**
	 * offset 4144
	 */
	int mainUnusedEnd[286];
	/** total size 5288*/
};

typedef struct engine_configuration_s engine_configuration_s;

// start of persistent_config_s
struct persistent_config_s {
	/**
	 * offset 0
	 */
	engine_configuration_s engineConfiguration;
	/**
	 * offset 5288
	 */
	gp_pwm_table_t gpPwmTable1;
	/**
	 * offset 5352
	 */
	float gpPwm1LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 5384
	 */
	float gpPwm1RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 5416
	 */
	gp_pwm_table_t gpPwmTable2;
	/**
	 * offset 5480
	 */
	float gpPwm2LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 5512
	 */
	float gpPwm2RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 5544
	 */
	gp_pwm_table_t gpPwmTable3;
	/**
	 * offset 5608
	 */
	float gpPwm3LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 5640
	 */
	float gpPwm3RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 5672
	 */
	gp_pwm_table_t gpPwmTable4;
	/**
	 * offset 5736
	 */
	float gpPwm4LoadBins[GP_PWM_LOAD_COUNT];
	/**
	 * offset 5768
	 */
	float gpPwm4RpmBins[GP_PWM_RPM_COUNT];
	/**
	 * offset 5800
	 */
	fsio_table_8x8_f32t vvtTable;
	/**
	 * offset 6056
	 */
	float vvtLoadBins[VVT_LOAD_COUNT];
	/**
	 * offset 6088
	 */
	float vvtRpmBins[VVT_RPM_COUNT];
	/**
	 * offset 6120
	 */
	float afterstartCoolantBins[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 6152
	 */
	float afterstartHoldTime[AFTERSTART_HOLD_CURVE_SIZE];
	/**
	 * offset 6184
	 */
	float afterstartEnrich[AFTERSTART_ENRICH_CURVE_SIZE];
	/**
	 * offset 6216
	 */
	float afterstartDecayTime[AFTERSTART_DECAY_CURVE_SIZE];
	/**
	 * offset 6248
	 */
	boost_table_t boostTableOpenLoop;
	/**
	 * offset 6312
	 */
	uint8_t boostMapBins[BOOST_LOAD_COUNT];
	/**
	 * offset 6320
	 */
	uint8_t boostRpmBins[BOOST_RPM_COUNT];
	/**
	 * offset 6328
	 */
	boost_table_t boostTableClosedLoop;
	/**
	 * offset 6392
	 */
	uint8_t boostTpsBins[BOOST_LOAD_COUNT];
	/**
	 * offset 6400
	 */
	pedal_to_tps_t pedalToTpsTable;
	/**
	 * offset 6464
	 */
	uint8_t pedalToTpsPedalBins[PEDAL_TO_TPS_SIZE];
	/**
	 * offset 6472
	 */
	uint8_t pedalToTpsRpmBins[PEDAL_TO_TPS_SIZE];
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 6480
	 */
	float cltCrankingCorrBins[CLT_CRANKING_CURVE_SIZE];
	/**
	 * CLT-based cranking position multiplier for simple manual idle controller
	 * offset 6512
	 */
	float cltCrankingCorr[CLT_CRANKING_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 6544
	 */
	float idleAdvanceBins[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional timing advance table for Idle (see useSeparateAdvanceForIdle)
	 * offset 6576
	 */
	float idleAdvance[IDLE_ADVANCE_CURVE_SIZE];
	/**
	 * Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 6608
	 */
	float idleVeBins[IDLE_VE_CURVE_SIZE];
	/**
	 *  Optional VE table for Idle (see useSeparateVEForIdle)
	 * offset 6640
	 */
	float idleVe[IDLE_VE_CURVE_SIZE];
	/**
	 * offset 6672
	 */
	le_formula_t fsioFormulas[FSIO_COMMAND_COUNT];
	/**
	 * offset 9872
	 */
	le_formula_t timingMultiplier;
	/**
	 * offset 10072
	 */
	le_formula_t timingAdditive;
	/**
	 * offset 10272
	 */
	float cltFuelCorrBins[CLT_CURVE_SIZE];
	/**
	 * offset 10336
	 */
	float cltFuelCorr[CLT_CURVE_SIZE];
	/**
	 * offset 10400
	 */
	float iatFuelCorrBins[IAT_CURVE_SIZE];
	/**
	 * offset 10464
	 */
	float iatFuelCorr[IAT_CURVE_SIZE];
	/**
	 * offset 10528
	 */
	float crankingFuelCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 10560
	 */
	float crankingFuelBins[CRANKING_CURVE_SIZE];
	/**
	 * offset 10592
	 */
	float crankingCycleCoef[CRANKING_CURVE_SIZE];
	/**
	 * offset 10624
	 */
	float crankingCycleBins[CRANKING_CURVE_SIZE];
	/**
	 * CLT-based idle position multiplier for simple manual idle controller
	 * offset 10656
	 */
	float cltIdleCorrBins[CLT_CURVE_SIZE];
	/**
	 *  CLT-based idle position multiplier for simple manual idle controller
	 * offset 10720
	 */
	float cltIdleCorr[CLT_CURVE_SIZE];
	/**
	 * offset 10784
	 */
	int unusedMaf[448];
	/**
	 * kg/hour value.
	 * By the way 2.081989116 kg/h = 1 ft3/m
	 * offset 12576
	 */
	float mafDecoding[MAF_DECODING_COUNT];
	/**
	 * offset 12704
	 */
	float mafDecodingBins[MAF_DECODING_COUNT];
	/**
	 * offset 12832
	 */
	angle_table_t ignitionIatCorrTable;
	/**
	 * offset 13856
	 */
	float ignitionIatCorrLoadBins[IGN_LOAD_COUNT];
	/**
	 * offset 13920
	 */
	float ignitionIatCorrRpmBins[IGN_RPM_COUNT];
	/**
	 * offset 13984
	 */
	angle_table_t injectionPhase;
	/**
	 * offset 15008
	 */
	float injPhaseLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 15072
	 */
	float injPhaseRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 15136
	 */
	fuel_table_t fuelTable;
	/**
	 * offset 16160
	 */
	float fuelLoadBins[FUEL_LOAD_COUNT];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 16224
	 */
	float fuelRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 16288
	 */
	ignition_table_t ignitionTable;
	/**
	 * offset 17312
	 */
	float ignitionLoadBins[IGN_LOAD_COUNT];
	/**
	 * offset 17376
	 */
	float ignitionRpmBins[IGN_RPM_COUNT];
	/**
	 * offset 17440
	 */
	ve_table_t veTable;
	/**
	 * offset 18464
	 */
	float veLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 18528
	 */
	float veRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 18592
	 */
	afr_table_t afrTable;
	/**
	 * offset 18848
	 */
	float afrLoadBins[FUEL_LOAD_COUNT];
	/**
	 * offset 18912
	 */
	float afrRpmBins[FUEL_RPM_COUNT];
	/**
	 * offset 18976
	 */
	tps_tps_table_t tpsTpsAccelTable;
	/**
	 * offset 19232
	 */
	float tpsTpsAccelFromRpmBins[TPS_TPS_ACCEL_TABLE];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19264
	 */
	float tpsTpsAccelToRpmBins[TPS_TPS_ACCEL_TABLE];
	/**
	 * offset 19296
	 */
	fsio_table_8x8_f32t fsioTable1;
	/**
	 * offset 19552
	 */
	float fsioTable1LoadBins[FSIO_TABLE_8];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19584
	 */
	float fsioTable1RpmBins[FSIO_TABLE_8];
	/**
	 * offset 19616
	 */
	fsio_table_8x8_u8t fsioTable2;
	/**
	 * offset 19680
	 */
	float fsioTable2LoadBins[FSIO_TABLE_8];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19712
	 */
	float fsioTable2RpmBins[FSIO_TABLE_8];
	/**
	 * offset 19744
	 */
	fsio_table_8x8_u8t fsioTable3;
	/**
	 * offset 19808
	 */
	float fsioTable3LoadBins[FSIO_TABLE_8];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19840
	 */
	float fsioTable3RpmBins[FSIO_TABLE_8];
	/**
	 * offset 19872
	 */
	fsio_table_8x8_u8t fsioTable4;
	/**
	 * offset 19936
	 */
	float fsioTable4LoadBins[FSIO_TABLE_8];
	/**
	 * RPM is float and not integer in order to use unified methods for interpolation
	 * offset 19968
	 */
	float fsioTable4RpmBins[FSIO_TABLE_8];
	/** total size 20000*/
};

typedef struct persistent_config_s persistent_config_s;

#endif
// end
// this section was generated automatically by rusEfi tool ConfigDefinition.jar based on gen_config.bat integration\rusefi_config.txt Tue Apr 28 03:31:05 CEST 2020
