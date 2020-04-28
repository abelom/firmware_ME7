/**
 * @file boards/Prometheus/efifeatures.h
 *
 * @brief In this header we can override efifeatures.h.
 *
 * @date Apr 22, 2017
 * @author andreika, (c) 2017
 */

#include "../../stm32f4ems/efifeatures.h"

#pragma once

#undef EFI_RTC
#define EFI_RTC FALSE

#undef EFI_POTENTIOMETER
#define EFI_POTENTIOMETER FALSE

#undef EFI_MAX_31855
#define EFI_MAX_31855 FALSE

#undef EFI_HD44780_LCD
#define EFI_HD44780_LCD FALSE

#undef EFI_LCD
#define EFI_LCD FALSE

#undef EFI_USB_SERIAL
#define EFI_USB_SERIAL FALSE

#undef MMC_CARD_SPI
#define MMC_CARD_SPI SPID1

#define ADC_CHANNEL_VREF ADC_CHANNEL_IN14

#undef EFI_CONSOLE_SERIAL_DEVICE
#undef EFI_CONSOLE_UART_DEVICE

#undef EFI_UART_GPS
#define EFI_UART_GPS FALSE

//!!!!!!!!!!!!!!!!!
#undef TS_UART_DMA_MODE
#define TS_UART_DMA_MODE TRUE

#undef TS_UART_DEVICE
#define TS_UART_DEVICE (&UARTD4)

#undef TS_SERIAL_DEVICE

#undef EFI_BLUETOOTH_SETUP
#define EFI_BLUETOOTH_SETUP TRUE

#undef SERIAL_SPEED
#define SERIAL_SPEED 115200

#ifndef BOARD_TLE6240_COUNT
#define BOARD_TLE6240_COUNT         0
#endif

#ifndef BOARD_MC33972_COUNT
#define BOARD_MC33972_COUNT			0
#endif

#ifndef BOARD_TLE8888_COUNT
#define BOARD_TLE8888_COUNT 	0
#endif


#undef EFI_CONSOLE_TX_PORT
#define EFI_CONSOLE_TX_PORT GPIOA
#undef EFI_CONSOLE_TX_PIN
#define EFI_CONSOLE_TX_PIN 0
#undef EFI_CONSOLE_RX_PORT
#define EFI_CONSOLE_RX_PORT GPIOA
#undef EFI_CONSOLE_RX_PIN
#define EFI_CONSOLE_RX_PIN 1
#undef EFI_CONSOLE_AF
#define EFI_CONSOLE_AF 8

#undef TS_SERIAL_AF
#define TS_SERIAL_AF 8

#undef LED_ERROR_BRAIN_PIN
#define LED_ERROR_BRAIN_PIN GPIOA_13

#undef CONSOLE_MODE_SWITCH_PORT
#undef CONFIG_RESET_SWITCH_PORT

//!!!!!!!!!!!!!!!
//#undef EFI_INTERNAL_ADC
//#define EFI_INTERNAL_ADC FALSE

#undef ADC_VCC
#define ADC_VCC 3.275f

#undef EFI_MAIN_RELAY_CONTROL
#define EFI_MAIN_RELAY_CONTROL TRUE

#undef EFI_MEMS
#define EFI_MEMS FALSE

#undef EFI_IDLE_PID_CIC
#define EFI_IDLE_PID_CIC TRUE

#define RPM_LOW_THRESHOLD 8				// RPM=8 is an empirical lower sensitivity threshold of MAX9926 for 60-2
#define NO_RPM_EVENTS_TIMEOUT_SECS 5	// (RPM < 12)

#define EFI_PRINT_ERRORS_AS_WARNINGS TRUE

#define EFI_NARROW_EGO_AVERAGING TRUE

