/*
 * @file config/boards/kinetis/rusefi_hw_enums.h
 *
 * @date Jun 2, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @author andreika <prometheus.pcb@gmail.com>
 */

#ifndef RUSEFI_HW_ENUMS_H_
#define RUSEFI_HW_ENUMS_H_

// todo: migrate/unify with pin_output_mode_e? rename? something is messy here
// this enum is currently only used for SPI pins
typedef enum __attribute__ ((__packed__)) {
// todo: here we have a rare example of stm32-specific enum, todo: make this not stm32 specific?
	PO_DEFAULT = 0,
	PO_OPENDRAIN = 4, // PAL_STM32_OTYPE_OPENDRAIN
	PO_PULLUP = 32, // PAL_STM32_PUDR_PULLUP
	PO_PULLDOWN = 64 // PAL_STM32_PUPDR_PULLDOWN
} pin_mode_e;

/**
 * Hardware pin. This enum is platform-specific.
 */
typedef enum __attribute__ ((__packed__)) {
	GPIO_UNASSIGNED = 0,
	GPIO_INVALID = 1,

	GPIOA_0 = 2,
	GPIOA_1 = 3,
	GPIOA_2 = 4,
	GPIOA_3 = 5,
	GPIOA_4 = 6,
	GPIOA_5 = 7,
	GPIOA_6 = 8,
	GPIOA_7 = 9,
	GPIOA_8 = 10,
	GPIOA_9 = 11,
	GPIOA_10 = 12,
	GPIOA_11 = 13,
	GPIOA_12 = 14,
	GPIOA_13 = 15,
	GPIOA_14 = 16,
	GPIOA_15 = 17,
	GPIOA_16 = 18,
	GPIOA_17 = 19,

	GPIOB_0 = 20,
	GPIOB_1 = 21,
	GPIOB_2 = 22,
	GPIOB_3 = 23,
	GPIOB_4 = 24,
	GPIOB_5 = 25,
	GPIOB_6 = 26,
	GPIOB_7 = 27,
	GPIOB_8 = 28,
	GPIOB_9 = 29,
	GPIOB_10 = 30,
	GPIOB_11 = 31,
	GPIOB_12 = 32,
	GPIOB_13 = 33,
	GPIOB_14 = 34,
	GPIOB_15 = 35,
	GPIOB_16 = 36,
	GPIOB_17 = 37,

	GPIOC_0 = 38,
	GPIOC_1 = 39,
	GPIOC_2 = 40,
	GPIOC_3 = 41,
	GPIOC_4 = 42,
	GPIOC_5 = 43,
	GPIOC_6 = 44,
	GPIOC_7 = 45,
	GPIOC_8 = 46,
	GPIOC_9 = 47,
	GPIOC_10 = 48,
	GPIOC_11 = 49,
	GPIOC_12 = 50,
	GPIOC_13 = 51,
	GPIOC_14 = 52,
	GPIOC_15 = 53,
	GPIOC_16 = 54,
	GPIOC_17 = 55,

	GPIOD_0 = 56,
	GPIOD_1 = 57,
	GPIOD_2 = 58,
	GPIOD_3 = 59,
	GPIOD_4 = 60,
	GPIOD_5 = 61,
	GPIOD_6 = 62,
	GPIOD_7 = 63,
	GPIOD_8 = 64,
	GPIOD_9 = 65,
	GPIOD_10 = 66,
	GPIOD_11 = 67,
	GPIOD_12 = 68,
	GPIOD_13 = 69,
	GPIOD_14 = 70,
	GPIOD_15 = 71,
	GPIOD_16 = 72,
	GPIOD_17 = 73,
	
	GPIOE_0 = 74,
	GPIOE_1 = 75,
	GPIOE_2 = 76,
	GPIOE_3 = 77,
	GPIOE_4 = 78,
	GPIOE_5 = 79,
	GPIOE_6 = 80,
	GPIOE_7 = 81,
	GPIOE_8 = 82,
	GPIOE_9 = 83,
	GPIOE_10 = 84,
	GPIOE_11 = 85,
	GPIOE_12 = 86,
	GPIOE_13 = 87,
	GPIOE_14 = 88,
	GPIOE_15 = 89,
	GPIOE_16 = 90,
	GPIOE_17 = 91,

	// TLE6240 pins go right after on chips
	//#define TLE6240_PIN(n)		((brain_pin_e)((int)BRAIN_PIN_LAST_ONCHIP + 1 + (n)))
	TLE6240_PIN_1 = 130,
	TLE6240_PIN_2 = 131,
	TLE6240_PIN_3 = 132,
	TLE6240_PIN_4 = 133,
	TLE6240_PIN_5 = 134,
	TLE6240_PIN_6 = 135,
	TLE6240_PIN_7 = 136,
	TLE6240_PIN_8 = 137,
	TLE6240_PIN_9 = 138,
	TLE6240_PIN_10 = 139,
	TLE6240_PIN_11 = 140,
	TLE6240_PIN_12 = 141,
	TLE6240_PIN_13 = 142,
	TLE6240_PIN_14 = 143,
	TLE6240_PIN_15 = 144,
	TLE6240_PIN_16 = 145,

} brain_pin_e;

/* Plase keep updating this define */
#define BRAIN_PIN_LAST_ONCHIP	GPIOE_17

/* diagnostic for brain pins
 * can be combination of few bits
 * defined as bit mask */
typedef enum __attribute__ ((__packed__))
{
	PIN_OK = 0,
	PIN_OPEN = 0x01,
	PIN_SHORT_TO_GND = 0x02,
	PIN_SHORT_TO_BAT = 0x04,
	PIN_OVERLOAD =	0x08,
	PIN_DRIVER_OVERTEMP = 0x10,
	PIN_INVALID = 0x80
} brain_pin_diag_e;

typedef enum __attribute__ ((__packed__)) {
	EFI_ADC_0 = 0,
	EFI_ADC_1 = 1,
	EFI_ADC_2 = 2,
	EFI_ADC_3 = 3,
	EFI_ADC_4 = 4,
	EFI_ADC_5 = 5,
	EFI_ADC_6 = 6,
	EFI_ADC_7 = 7,
	EFI_ADC_8 = 8,
	EFI_ADC_9 = 9,
	EFI_ADC_10 = 10,
	EFI_ADC_11 = 11,
	EFI_ADC_12 = 12,
	EFI_ADC_13 = 13,
	EFI_ADC_14 = 14,
	EFI_ADC_15 = 15,

	// todo: bad choice of value since now we have ADC_CHANNEL_SENSOR and could end up with 17 and 18 also
	EFI_ADC_NONE = 16,
	EFI_ADC_ERROR = 17,
#if EFI_UNIT_TEST
	/**
	 * these values are unfortunately visible to BoardReader
	 * and TunerStudio would need these ordinals to fit into field size
	 */
    TEST_MAF_CHANNEL = 18,
    TEST_CLT_CHANNEL = 19,
    TEST_IAT_CHANNEL = 20,
#endif /* EFI_UNIT_TEST */
} adc_channel_e;

#define INCOMPATIBLE_CONFIG_CHANGE EFI_ADC_0

#endif /* RUSEFI_HW_ENUMS_H_ */
