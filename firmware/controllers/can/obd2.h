/*
 * @file obd2.h
 *
 * @date Jun 9, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"

#define OBD_TEST_REQUEST 0x7DF

#define OBD_TEST_RESPONSE 0x7E8

#define OBD_CURRENT_DATA 1
#define OBD_STORED_DIAGNOSTIC_TROUBLE_CODES 3
#define OBD_PENDING_DIAGNOSTIC_TROUBLE_CODES 7

#define PID_SUPPORTED_PIDS_REQUEST_01_20 0x00
#define PID_MONITOR_STATUS 0x01
#define PID_FUEL_SYSTEM_STATUS 0x03
#define PID_ENGINE_LOAD 0x04
#define PID_COOLANT_TEMP 0x05
#define PID_FUEL_PRESSURE 0x0A
#define PID_INTAKE_MAP 0x0B
#define PID_RPM 0x0C
#define PID_SPEED 0x0D
#define PID_TIMING_ADVANCE 0x0E
#define PID_INTAKE_TEMP 0x0F
#define PID_INTAKE_MAF 0x10
#define PID_THROTTLE 0x11

#define PID_SUPPORTED_PIDS_REQUEST_21_40 0x20

#define PID_SUPPORTED_PIDS_REQUEST_41_60 0x40
#define PID_FUEL_RATE 0x5E

#if HAL_USE_CAN
void obdOnCanPacketRx(const CANRxFrame& rx);
#endif /* HAL_USE_CAN */

