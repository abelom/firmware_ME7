/**
 * @file	stm32_pins.cpp
 * @brief	STM32-compatible GPIO code
 *
 * @date Jun 02, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "engine.h"
#include "efi_gpio.h"
#include "pin_repository.h"
#include "io_pins.h"
#include "smart_gpio.h"

#if EFI_GPIO_HARDWARE

#define PORT_SIZE 16

static ioportid_t ports[] = {GPIOA,
		GPIOB,
		GPIOC,
		GPIOD,
#if STM32_HAS_GPIOE
		GPIOE,
#else
		nullptr,
#endif /* STM32_HAS_GPIOE */
#if STM32_HAS_GPIOF
		GPIOF,
#else
		nullptr,
#endif /* STM32_HAS_GPIOF */
#if STM32_HAS_GPIOG
		GPIOG,
#else
		nullptr,
#endif /* STM32_HAS_GPIOG */
#if STM32_HAS_GPIOH
		GPIOH,
#else
		nullptr,
#endif /* STM32_HAS_GPIOH */
};

#define PIN_REPO_SIZE (sizeof(ports) / sizeof(ports[0])) * PORT_SIZE
// todo: move this into PinRepository class
static const char *PIN_USED[PIN_REPO_SIZE + BOARD_EXT_PINREPOPINS];


extern ioportid_t PORTS[];

/**
 * @deprecated - use hwPortname() instead
 */
const char *portname(ioportid_t GPIOx) {
	if (GPIOx == GPIOA)
		return "PA";
	if (GPIOx == GPIOB)
		return "PB";
	if (GPIOx == GPIOC)
		return "PC";
	if (GPIOx == GPIOD)
		return "PD";
#if STM32_HAS_GPIOE
	if (GPIOx == GPIOE)
		return "PE";
#endif /* STM32_HAS_GPIOE */
#if STM32_HAS_GPIOF
	if (GPIOx == GPIOF)
		return "PF";
#endif /* STM32_HAS_GPIOF */
#if STM32_HAS_GPIOG
	if (GPIOx == GPIOG)
		return "PG";
#endif /* STM32_HAS_GPIOG */
#if STM32_HAS_GPIOH
	if (GPIOx == GPIOH)
		return "PH";
#endif /* STM32_HAS_GPIOH */
	return "unknown";
}

static int getPortIndex(ioportid_t port) {
	efiAssert(CUSTOM_ERR_ASSERT, port != NULL, "null port", -1);
	if (port == GPIOA)
		return 0;
	if (port == GPIOB)
		return 1;
	if (port == GPIOC)
		return 2;
	if (port == GPIOD)
		return 3;
#if STM32_HAS_GPIOE
	if (port == GPIOE)
		return 4;
#endif /* STM32_HAS_GPIOE */
#if STM32_HAS_GPIOF
	if (port == GPIOF)
		return 5;
#endif /* STM32_HAS_GPIOF */
#if STM32_HAS_GPIOG
	if (port == GPIOG)
		return 6;
#endif /* STM32_HAS_GPIOG */
#if STM32_HAS_GPIOH
	if (port == GPIOH)
		return 7;
#endif /* STM32_HAS_GPIOH */
	firmwareError(CUSTOM_ERR_UNKNOWN_PORT, "unknown port");
	return -1;
}

ioportid_t getBrainPort(brain_pin_e brainPin) {
	return ports[(brainPin - GPIOA_0) / PORT_SIZE];
}

int getBrainPinIndex(brain_pin_e brainPin) {
	return (brainPin - GPIOA_0) % PORT_SIZE;
}

int getBrainIndex(ioportid_t port, ioportmask_t pin) {
	int portIndex = getPortIndex(port);
	return portIndex * PORT_SIZE + pin;
}

ioportid_t getHwPort(const char *msg, brain_pin_e brainPin) {
	if (brainPin == GPIO_UNASSIGNED || brainPin == GPIO_INVALID)
		return GPIO_NULL;
	if (brainPin < GPIOA_0 || brainPin > BRAIN_PIN_LAST_ONCHIP) {
		firmwareError(CUSTOM_ERR_INVALID_PIN, "%s: Invalid brain_pin_e: %d", msg, brainPin);
		return GPIO_NULL;
	}
	return PORTS[(brainPin - GPIOA_0) / PORT_SIZE];
}

/**
 * this method returns the numeric part of pin name. For instance, for PC13 this would return '13'
 */
ioportmask_t getHwPin(const char *msg, brain_pin_e brainPin)
{
	if (brainPin == GPIO_UNASSIGNED || brainPin == GPIO_INVALID)
			return EFI_ERROR_CODE;

	if (brain_pin_is_onchip(brainPin))
		return getBrainPinIndex(brainPin);

	firmwareError(CUSTOM_ERR_INVALID_PIN, "%s: Invalid on-chip brain_pin_e: %d", msg, brainPin);
	return EFI_ERROR_CODE;
}

/**
 * Parse string representation of physical pin into brain_pin_e ordinal.
 *
 * @return GPIO_UNASSIGNED for "none", GPIO_INVALID for invalid entry
 */
brain_pin_e parseBrainPin(const char *str) {
	if (strEqual(str, "none"))
		return GPIO_UNASSIGNED;
	// todo: create method toLowerCase?
	if (str[0] != 'p' && str[0] != 'P') {
		return GPIO_INVALID;
	}
	char port = str[1];
	brain_pin_e basePin;
	if (port >= 'a' && port <= 'z') {
		basePin = (brain_pin_e) ((int) GPIOA_0 + PORT_SIZE * (port - 'a'));
	} else if (port >= 'A' && port <= 'Z') {
		basePin = (brain_pin_e) ((int) GPIOA_0 + PORT_SIZE * (port - 'A'));
	} else {
		return GPIO_INVALID;
	}
	const char *pinStr = str + 2;
	int pin = atoi(pinStr);
	return (brain_pin_e)(basePin + pin);
}

unsigned int getNumBrainPins(void) {
	return PIN_REPO_SIZE;
}

void initBrainUsedPins(void) {
	memset(PIN_USED, 0, sizeof(PIN_USED));
}

const char* & getBrainUsedPin(unsigned int idx) {
	return PIN_USED[idx];
}

#endif /* EFI_GPIO_HARDWARE */
