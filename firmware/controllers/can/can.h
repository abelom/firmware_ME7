/**
 * @file	can.h
 *
 * @date Mar 19, 2020
 * @author Matthew Kennedy, (c) 2020
 */

#pragma once

#include "hal.h"

#include "periodic_thread_controller.h"

#define CAN_PEDAL_TPS_OFFSET 2
#define CAN_SENSOR_1_OFFSET 3

class Logging;
class CanSensorBase;

void processCanRxMessage(const CANRxFrame& msg, Logging* logger, efitick_t nowNt);
void registerCanSensor(CanSensorBase& sensor);

class CanWrite final : public PeriodicController<512> {
public:
	CanWrite();
	void PeriodicTask(efitime_t nowNt) override;
};
