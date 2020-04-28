/**
 * @file efifeatures.h
 *
 * @brief In this header we can configure which firmware modules are used.
 *
 * STM32F7 config is inherited from STM32F4. This file contains only differences between F4 and F7.
 * This is more consistent way to maintain these config 'branches' and add new features.
 *
 * @date Aug 29, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */
 
#include "../stm32f4ems/efifeatures.h"

#pragma once
#undef EFI_GP_PWM
#define EFI_GP_PWM TRUE

#undef EFI_BOOST_CONTROL
#define EFI_BOOST_CONTROL TRUE

#undef EFI_VVT_CONTROL
#define EFI_VVT_CONTROL FALSE

#undef EFI_LAUNCH_CONTROL
#define EFI_LAUNCH_CONTROL TRUE

#undef EFI_CJ125
#define EFI_CJ125 TRUE

#undef EFI_USE_CCM
#define EFI_USE_CCM TRUE

#undef EFI_POTENTIOMETER
#define EFI_POTENTIOMETER FALSE

#undef EFI_MAX_31855
#define EFI_MAX_31855 FALSE

#undef EFI_MCP_3208
#define EFI_MCP_3208 FALSE

#undef EFI_MC33816
#define EFI_MC33816 FALSE

#undef EFI_DENSO_ADC
#define EFI_DENSO_ADC FALSE

#undef EFI_MEMS
#define EFI_MEMS FALSE

#undef EFI_CDM_INTEGRATION
#define EFI_CDM_INTEGRATION FALSE

#undef EFI_PWM_TESTER
#define EFI_PWM_TESTER FALSE

#undef EFI_MC3381
#define EFI_MC33816 FALSE

#undef EFI_ENABLE_CRITICAL_ENGINE_STOP
#define EFI_ENABLE_CRITICAL_ENGINE_STOP FALSE

#undef EFI_ENABLE_ENGINE_WARNING
#define EFI_ENABLE_ENGINE_WARNING FALSE

#undef EFI_LOGIC_ANALYZER
#define EFI_LOGIC_ANALYZER FALSE

#undef EFI_ICU_INPUTS
#define EFI_ICU_INPUTS TRUE

#undef HAL_USE_ICU
#define HAL_USE_ICU FALSE

#undef HAL_TRIGGER_USE_PAL
#define HAL_TRIGGER_USE_PAL TRUE

#undef EFI_VEHICLE_SPEED
#define EFI_VEHICLE_SPEED FALSE

#undef EFI_BLUETOOTH_SETUP
#define EFI_BLUETOOTH_SETUP FALSE

#undef EFI_TUNER_STUDIO_VERBOSE
#define EFI_TUNER_STUDIO_VERBOSE TRUE

#undef EFI_DEFAILED_LOGGING
#define EFI_DEFAILED_LOGGING FALSE

#undef EFI_AUX_PID
#define EFI_AUX_PID FALSE

#undef EFI_POTENTIOMETER
#define EFI_POTENTIOMETER FALSE


#undef EFI_MAX_31855
#define EFI_MAX_31855 FALSE

#undef EFI_MCP_3208
#define EFI_MCP_3208 FALSE

#undef EFI_HIP_9011
#define EFI_HIP_9011 FALSE

#undef EFI_CJ125
#define EFI_CJ125 TRUE

#undef EFI_INCLUDE_ENGINE_PRESETS
#define EFI_INCLUDE_ENGINE_PRESETS FALSE

#ifndef BOARD_TLE6240_COUNT
#define BOARD_TLE6240_COUNT         0
#endif

#ifndef BOARD_MC33972_COUNT
#define BOARD_MC33972_COUNT			0
#endif

#ifndef BOARD_TLE8888_COUNT
#define BOARD_TLE8888_COUNT 	1
#endif




#undef EFI_CAN_SUPPORT
#define EFI_CAN_SUPPORT TRUE

#undef EFI_HD44780_LCD
#define EFI_HD44780_LCD FALSE

#undef EFI_LCD
#define EFI_LCD FALSE

#undef EFI_FSIO
#define EFI_FSIO FALSE

#undef EFI_FILE_LOGGING
#define EFI_FILE_LOGGING FALSE

#undef EFI_USB_SERIAL
#define EFI_USB_SERIAL TRUE

#undef EFI_UART_GPS
#define EFI_UART_GPS FALSE

// todo: start using consoleUartDevice? Not sure
#undef EFI_CONSOLE_SERIAL_DEVICE
#define EFI_CONSOLE_SERIAL_DEVICE (&SD3)

#undef TS_UART_DMA_MODE
#define TS_UART_DMA_MODE FALSE

#undef TS_UART_DEVICE
#define TS_UART_DEVICE (&UARTD3)
#undef TS_SERIAL_DEVICE
#define TS_SERIAL_DEVICE (&SD3)

#if (TS_UART_DMA_MODE || TS_UART_MODE)
#undef EFI_CONSOLE_SERIAL_DEVICE
#endif

#undef EFI_CONSOLE_TX_PORT
#define EFI_CONSOLE_TX_PORT GPIOC

#undef EFI_CONSOLE_TX_PIN
#define EFI_CONSOLE_TX_PIN 10

#undef EFI_CONSOLE_RX_PORT
#define EFI_CONSOLE_RX_PORT GPIOC

#undef EFI_CONSOLE_RX_PIN
#define EFI_CONSOLE_RX_PIN 11

// todo: temporary ignore errors, this is a test config
#define EFI_PRINT_ERRORS_AS_WARNINGS TRUE
