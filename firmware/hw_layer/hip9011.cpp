/**
 * @file	HIP9011.cpp
 * @brief	HIP9011/TPIC8101 driver
 *
 * Jan 2017 status:
 *  1) seems to be kind of working - reacts to parameter changes and does produce variable output
 *  2) only one (first) channel is currently used
 *  3) engine control does not yet react to knock since very little actual testing - no engine runs with proven knock yet
 *
 *
 * http://rusefi.com/forum/viewtopic.php?f=4&t=400
 * http://rusefi.com/forum/viewtopic.php?f=5&t=778
 *
 *	pin1	VDD
 *	pin2	GND
 *
 *	pin8	Chip Select - CS
 *	pin11	Slave Data Out - MISO
 *	pin12	Slave Data In - MOSI
 *	pin13	SPI clock - SCLK
 *
 *
 * http://www.ti.com/lit/ds/symlink/tpic8101.pdf
 * http://www.intersil.com/content/dam/Intersil/documents/hip9/hip9011.pdf
 * http://www.intersil.com/content/dam/Intersil/documents/an97/an9770.pdf
 * http://e2e.ti.com/cfs-file/__key/telligent-evolution-components-attachments/00-26-01-00-00-42-36-40/TPIC8101-Training.pdf
 *
 * max SPI frequency: 5MHz max
 *
 * @date Nov 27, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @Spilly
 */

#include "global.h"
#include "engine.h"
#include "settings.h"
#include "hardware.h"
#include "rpm_calculator.h"
#include "trigger_central.h"
#include "hip9011_logic.h"
#include "hip9011_lookup.h"
#include "hip9011.h"
#include "adc_inputs.h"
#include "perf_trace.h"

#include "engine_controller.h"

#if EFI_PROD_CODE
#include "pin_repository.h"
#include "mpu_util.h"
#endif

#if EFI_HIP_9011

static NamedOutputPin intHold(PROTOCOL_HIP_NAME);

extern uint32_t lastExecutionCount;

uint32_t hipLastExecutionCount;


class Hip9011Hardware : public Hip9011HardwareInterface {
	void sendSyncCommand(unsigned char command) override;
	void sendCommand(unsigned char command) override;
};

static Hip9011Hardware hardware;

static float hipValueMax = 0;

HIP9011 instance(&hardware);

static unsigned char tx_buff[1];
static unsigned char rx_buff[1];
static char pinNameBuffer[16];

static scheduling_s startTimer[2];
static scheduling_s endTimer[2];

static Logging *logger;

// SPI_CR1_BR_1 // 5MHz
// SPI_CR1_CPHA Clock Phase
// todo: nicer method which would mention SPI speed explicitly?

#if EFI_PROD_CODE
static SPIConfig hipSpiCfg = {
	.circular = false,
	.end_cb = NULL,
	.ssport = NULL,
	.sspad = 0,
	.cr1 =
		SPI_CR1_MSTR |
		SPI_CR1_CPHA |
		//SPI_CR1_BR_1 // 5MHz
		SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2 |
		SPI_CR1_8BIT_MODE,
	.cr2 =
		SPI_CR2_8BIT_MODE
};
#endif /* EFI_PROD_CODE */

static void checkResponse(void) {
	if (tx_buff[0] == rx_buff[0]) {
		instance.correctResponsesCount++;
	} else {
		instance.invalidHip9011ResponsesCount++;
	}
}

// this macro is only used on startup
#define SPI_SYNCHRONOUS(value) \
	spiSelect(driver); \
	tx_buff[0] = value; \
	spiExchange(driver, 1, tx_buff, rx_buff); \
	spiUnselect(driver); \
	checkResponse();


static SPIDriver *driver;

void Hip9011Hardware::sendSyncCommand(unsigned char command) {
	SPI_SYNCHRONOUS(command);
	chThdSleepMilliseconds(10);
}

void Hip9011Hardware::sendCommand(unsigned char command) {
	tx_buff[0] = command;

	spiSelectI(driver);
	spiStartExchangeI(driver, 1, tx_buff, rx_buff);
}

EXTERN_ENGINE;

static char hipPinNameBuffer[16];

static void showHipInfo(void) {
	if (!CONFIG(isHip9011Enabled)) {
		scheduleMsg(logger, "hip9011 driver not active");
		return;
	}

	printSpiState(logger, engineConfiguration);
	scheduleMsg(logger, "enabled=%s state=%s bore=%.2fmm freq=%.2fkHz PaSDO=%d",
			boolToString(CONFIG(isHip9011Enabled)),
			getHip_state_e(instance.state),
			engineConfiguration->cylinderBore, getHIP9011Band(PASS_HIP_PARAMS),
			engineConfiguration->hip9011PrescalerAndSDO);

	char *outputName = getPinNameByAdcChannel("hip", engineConfiguration->hipOutputChannel, hipPinNameBuffer);

	scheduleMsg(logger, "band_index=%d gain %.2f/index=%d output=%s", instance.currentBandIndex, engineConfiguration->hip9011Gain, instance.currentGainIndex,
			outputName);
	scheduleMsg(logger, "integrator index=%d knockVThreshold=%.2f knockCount=%d maxKnockSubDeg=%.2f",
	            instance.currentIntergratorIndex, engineConfiguration->knockVThreshold,
	            engine->knockCount, engineConfiguration->maxKnockSubDeg);

	const char * msg = instance.invalidHip9011ResponsesCount > 0 ? "NOT GOOD" : "ok";
	scheduleMsg(logger, "spi=%s IntHold@%s/%d response count=%d incorrect response=%d %s",
			getSpi_device_e(engineConfiguration->hip9011SpiDevice),
			hwPortname(CONFIG(hip9011IntHoldPin)),
			CONFIG(hip9011IntHoldPinMode),
			instance.correctResponsesCount, instance.invalidHip9011ResponsesCount,
			msg);
	scheduleMsg(logger, "CS@%s updateCount=%d", hwPortname(CONFIG(hip9011CsPin)), instance.settingUpdateCount);

#if EFI_PROD_CODE
	scheduleMsg(logger, "hip %.2fv/last=%.2f@%s/max=%.2f adv=%d",
			engine->knockVolts,
			getVoltage("hipinfo", engineConfiguration->hipOutputChannel),
			getPinNameByAdcChannel("hip", engineConfiguration->hipOutputChannel, pinNameBuffer),
			hipValueMax,
			CONFIG(useTpicAdvancedMode));
	printSpiConfig(logger, "hip9011", CONFIG(hip9011SpiDevice));
#endif /* EFI_PROD_CODE */

	scheduleMsg(logger, "start %.2f end %.2f", engineConfiguration->knockDetectionWindowStart,
			engineConfiguration->knockDetectionWindowEnd);

	hipValueMax = 0;
	engine->printKnockState();
}

void setHip9011FrankensoPinout(void) {
	/**
	 * SPI on PB13/14/15
	 */
	//	CONFIG(hip9011CsPin) = GPIOD_0; // rev 0.1

	CONFIG(isHip9011Enabled) = true;
	engineConfiguration->hip9011PrescalerAndSDO = _8MHZ_PRESCALER; // 8MHz chip
	CONFIG(is_enabled_spi_2) = true;
	// todo: convert this to rusEfi, hardware-independent enum
#if EFI_PROD_CODE
#ifdef EFI_HIP_CS_PIN
	CONFIG(hip9011CsPin) = EFI_HIP_CS_PIN;
#else
	CONFIG(hip9011CsPin) = GPIOB_0; // rev 0.4
#endif
	CONFIG(hip9011CsPinMode) = OM_OPENDRAIN;

	CONFIG(hip9011IntHoldPin) = GPIOB_11;
	CONFIG(hip9011IntHoldPinMode) = OM_OPENDRAIN;

	engineConfiguration->spi2SckMode = PO_OPENDRAIN; // 4
	engineConfiguration->spi2MosiMode = PO_OPENDRAIN; // 4
	engineConfiguration->spi2MisoMode = PO_PULLUP; // 32
#endif /* EFI_PROD_CODE */

	engineConfiguration->hip9011Gain = 1;
	engineConfiguration->knockVThreshold = 4;
	engineConfiguration->maxKnockSubDeg = 20;


	if (!CONFIG(useTpicAdvancedMode)) {
	    engineConfiguration->hipOutputChannel = EFI_ADC_10; // PC0
	}
}

static void startIntegration(void *) {
	if (instance.state == READY_TO_INTEGRATE) {
		/**
		 * SPI communication is only allowed while not integrating, so we postpone the exchange
		 * until we are done integrating
		 */
		instance.state = IS_INTEGRATING;
		intHold.setHigh();
	}
}

static void endIntegration(void *) {
	/**
	 * isIntegrating could be 'false' if an SPI command was pending thus we did not integrate during this
	 * engine cycle
	 */
	if (instance.state == IS_INTEGRATING) {
		intHold.setLow();
		instance.state = WAITING_FOR_ADC_TO_SKIP;
	}
}

/**
 * Shaft Position callback used to start or finish HIP integration
 */
static void intHoldCallback(trigger_event_e ckpEventType, uint32_t index, efitick_t edgeTimestamp DECLARE_ENGINE_PARAMETER_SUFFIX) {
	(void)ckpEventType;
	// this callback is invoked on interrupt thread
	if (index != 0)
		return;

	ScopePerf perf(PE::Hip9011IntHoldCallback);

	int rpm = GET_RPM_VALUE;
	if (!isValidRpm(rpm))
		return;

	int structIndex = getRevolutionCounter() % 2;
	// todo: schedule this based on closest trigger event, same as ignition works
	scheduleByAngle(&startTimer[structIndex], edgeTimestamp, engineConfiguration->knockDetectionWindowStart,
			&startIntegration);
#if EFI_PROD_CODE
	hipLastExecutionCount = lastExecutionCount;
#endif /* EFI_PROD_CODE */
	scheduleByAngle(&endTimer[structIndex], edgeTimestamp, engineConfiguration->knockDetectionWindowEnd,
			&endIntegration);
}

void setMaxKnockSubDeg(int value) {
    engineConfiguration->maxKnockSubDeg = value;
    showHipInfo();
}

void setKnockThresh(float value) {
    engineConfiguration->knockVThreshold = value;
    showHipInfo();
}

void setPrescalerAndSDO(int value) {
	engineConfiguration->hip9011PrescalerAndSDO = value;
}

void setHipBand(float value) {
	engineConfiguration->knockBandCustom = value;
	showHipInfo();
}

void setHipGain(float value) {
	engineConfiguration->hip9011Gain = value;
	showHipInfo();
}

/**
 * this is the end of the non-synchronous exchange
 */
static void endOfSpiExchange(SPIDriver *spip) {
	(void)spip;
	spiUnselectI(driver);
	instance.state = READY_TO_INTEGRATE;
	checkResponse();
}

void hipAdcCallback(adcsample_t adcValue) {
	if (instance.state == WAITING_FOR_ADC_TO_SKIP) {
		instance.state = WAITING_FOR_RESULT_ADC;
	} else if (instance.state == WAITING_FOR_RESULT_ADC) {
		engine->knockVolts = adcValue * engine->adcToVoltageInputDividerCoefficient;
		hipValueMax = maxF(engine->knockVolts, hipValueMax);
		engine->knockLogic(engine->knockVolts);

		instance.handleValue(GET_RPM_VALUE DEFINE_PARAM_SUFFIX(PASS_HIP_PARAMS));

	}
}

static void hipStartupCode(void) {
//	D[4:1] = 0000 : 4 MHz
//	D[4:1] = 0001 : 5 MHz
//	D[4:1] = 0010 : 6 MHz
//	D[4:1] = 0011 ; 8 MHz
//	D[4:1] = 0100 ; 10 MHz
//	D[4:1] = 0101 ; 12 MHz
//	D[4:1] = 0110 : 16 MHz
//	D[4:1] = 0111 : 20 MHz
//	D[4:1] = 1000 : 24 MHz


// 0 for 4MHz
// 6 for 8 MHz
	instance.currentPrescaler = engineConfiguration->hip9011PrescalerAndSDO;
	instance.hardware->sendSyncCommand(SET_PRESCALER_CMD + instance.currentPrescaler);


	// '0' for channel #1
	instance.hardware->sendSyncCommand(SET_CHANNEL_CMD + 0);

	// band index depends on cylinder bore
	instance.hardware->sendSyncCommand(SET_BAND_PASS_CMD + instance.currentBandIndex);


	if (instance.correctResponsesCount == 0) {
		warning(CUSTOM_OBD_KNOCK_PROCESSOR, "TPIC/HIP does not respond");
	}

	if (CONFIG(useTpicAdvancedMode)) {
		// enable advanced mode for digital integrator output
		instance.hardware->sendSyncCommand(SET_ADVANCED_MODE);
	}

	/**
	 * Let's restart SPI to switch it from synchronous mode into
	 * asynchronous mode
	 */
	spiStop(driver);
#if EFI_PROD_CODE
	hipSpiCfg.end_cb = endOfSpiExchange;
#endif
	spiStart(driver, &hipSpiCfg);
	instance.state = READY_TO_INTEGRATE;
}

static THD_WORKING_AREA(hipTreadStack, UTILITY_THREAD_STACK_SIZE);

static msg_t hipThread(void *arg) {
	(void)arg;
	chRegSetThreadName("hip9011 init");

	// some time to let the hardware start
	enginePins.hipCs.setValue(true);
	chThdSleepMilliseconds(100);
	enginePins.hipCs.setValue(false);
	chThdSleepMilliseconds(100);
	enginePins.hipCs.setValue(true);

	while (true) {
		chThdSleepMilliseconds(100);

		if (instance.needToInit) {
			hipStartupCode();
			instance.needToInit = false;
		}
	}
	return -1;
}

void stopHip9001_pins() {
#if EFI_PROD_CODE
	brain_pin_markUnused(activeConfiguration.hip9011IntHoldPin);
	brain_pin_markUnused(activeConfiguration.hip9011CsPin);
#endif /* EFI_PROD_CODE */
}

void startHip9001_pins() {
	intHold.initPin("hip int/hold", CONFIG(hip9011IntHoldPin), &CONFIG(hip9011IntHoldPinMode));
	enginePins.hipCs.initPin("hip CS", CONFIG(hip9011CsPin), &CONFIG(hip9011CsPinMode));
}

void initHip9011(Logging *sharedLogger) {
	logger = sharedLogger;
	addConsoleAction("hipinfo", showHipInfo);
	if (!CONFIG(isHip9011Enabled))
		return;


	instance.setAngleWindowWidth();

#if EFI_PROD_CODE
	driver = getSpiDevice(engineConfiguration->hip9011SpiDevice);
	if (driver == NULL) {
		// error already reported
		return;
	}

	hipSpiCfg.ssport = getHwPort("hip", CONFIG(hip9011CsPin));
	hipSpiCfg.sspad = getHwPin("hip", CONFIG(hip9011CsPin));
#endif /* EFI_PROD_CODE */

	startHip9001_pins();

	scheduleMsg(logger, "Starting HIP9011/TPIC8101 driver");
	spiStart(driver, &hipSpiCfg);

	instance.currentBandIndex = getBandIndex();

	/**
	 * this engine cycle callback would be scheduling actual integration start and end callbacks
	 */
	addTriggerEventListener(&intHoldCallback, "DD int/hold", engine);

	// MISO PB14
//	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(EFI_SPI2_AF) | PAL_STM32_PUDR_PULLUP);
	// MOSI PB15
//	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(EFI_SPI2_AF) | PAL_STM32_OTYPE_OPENDRAIN);

	addConsoleActionF("set_gain", setHipGain);
	addConsoleActionF("set_band", setHipBand);
	addConsoleActionI("set_hip_prescalerandsdo", setPrescalerAndSDO);
    addConsoleActionF("set_knock_threshold", setKnockThresh);
    addConsoleActionI("set_max_knock_sub_deg", setMaxKnockSubDeg);
	chThdCreateStatic(hipTreadStack, sizeof(hipTreadStack), NORMALPRIO, (tfunc_t)(void*) hipThread, NULL);
}

#endif /* EFI_HIP_9011 */
