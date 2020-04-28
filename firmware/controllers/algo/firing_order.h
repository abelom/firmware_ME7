/*
 * @file firing_order.h
 *
 * See also FiringOrderTSLogic.java
 *
 * @date Jul 20, 2016
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "rusefi_enums.h"

#pragma once

typedef enum {
	FO_1 = 0,

	// 2 cylinder
	FO_1_2 = 8,

	// 3 cylinder
	FO_1_2_3 = 10,

	// 4 cylinder
	FO_1_3_4_2 = 1, // typical inline 4
	FO_1_2_4_3 = 2,
	FO_1_3_2_4 = 3, // for example horizontally opposed engine
	FO_1_4_3_2 = 17, // for example VW aircooled boxer engine

	// 5 cylinder
	FO_1_2_4_5_3 = 6,

	// 6 cylinder
	FO_1_5_3_6_2_4 = 4,
	FO_1_4_2_5_3_6 = 7,
	FO_1_2_3_4_5_6 = 9,
	FO_1_6_3_2_5_4 = 13, // EG33

	// 8 cylinder
	FO_1_8_4_3_6_5_7_2 = 5,
	FO_1_8_7_2_6_5_4_3 = 11,
	FO_1_5_4_2_6_3_7_8 = 12,
	FO_1_2_7_8_4_5_6_3 = 19,

	// 10 cylinder
	FO_1_10_9_4_3_6_5_8_7_2 = 14, // dodge and viper ram v10

	// 12 cylinder
	FO_1_7_5_11_3_9_6_12_2_8_4_10 = 15, // bmw M70 & M73 etc
	FO_1_7_4_10_2_8_6_12_3_9_5_11 = 16, // lamborghini, typical rusEfi use-case
	FO_1_12_5_8_3_10_6_7_2_11_4_9 = 18, // VAG W12

	// 16 cylinder
	// todo: 1-14-9-4-7-12-15-6-13-8-3-16-11-2-5-10

	// max used = 19

	Force_4b_firing_order = ENUM_32_BITS,
} firing_order_e;
