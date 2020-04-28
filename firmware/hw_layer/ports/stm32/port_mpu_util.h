/**
 * @file	stm32_common_mpu_util.h
 * @brief	Low level common STM32 header
 *
 * @date Aug 3, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "device_mpu_util.h"

typedef enum {
	BOR_Level_None = OB_BOR_OFF, // 0x0C=12  Supply voltage ranges from 1.62 to 2.10 V
	BOR_Level_1 = OB_BOR_LEVEL1, // 0x08     Supply voltage ranges from 2.10 to 2.40 V
	BOR_Level_2 = OB_BOR_LEVEL2, // 0x04     Supply voltage ranges from 2.40 to 2.70 V
	BOR_Level_3 = OB_BOR_LEVEL3  // 0x00     Supply voltage ranges from 2.70 to 3.60 V
} BOR_Level_t;

// we are lucky - all CAN pins use the same AF
#define EFI_CAN_RX_AF 9
#define EFI_CAN_TX_AF 9

#ifndef GPIO_AF_TIM1
#define GPIO_AF_TIM1 1
#endif

#ifndef GPIO_AF_TIM2
#define GPIO_AF_TIM2 1
#endif

#ifndef GPIO_AF_TIM3
#define GPIO_AF_TIM3 2
#endif

#ifndef GPIO_AF_TIM4
#define GPIO_AF_TIM4 2
#endif

#ifndef GPIO_AF_TIM5
#define GPIO_AF_TIM5 2
#endif

#ifndef GPIO_AF_TIM8
#define GPIO_AF_TIM8 3
#endif

#ifndef GPIO_AF_TIM9
#define GPIO_AF_TIM9 3
#endif

// F4/F7 have the same ADC peripheral
#ifndef ADC_TwoSamplingDelay_5Cycles
#define ADC_TwoSamplingDelay_5Cycles ((uint32_t)0x00000000)
#endif

#ifndef ADC_TwoSamplingDelay_20Cycles
#define ADC_TwoSamplingDelay_20Cycles ((uint32_t)0x00000F00)
#endif

#ifndef ADC_CR2_SWSTART
#define ADC_CR2_SWSTART ((uint32_t)0x40000000)
#endif
