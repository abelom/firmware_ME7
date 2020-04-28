/*
 * @file	scaled_channel.h
 * @brief	Scaled channel storage for binary formats
 *
 * Storage of values (floating point, usually) scaled in integer storage, for transmission over the wire.
 * This includes Tunerstudio serial/USB, and CAN.
 *
 * @date Feb 24, 2020
 * @author Matthew Kennedy, (c) 2020
 */

#pragma once

#include <cstdint>

#include "rusefi_generated.h"

// This class lets us transparently store something at a ratio inside an integer type
// Just use it like a float - you can read and write to it, like this:
// scaled_channel<uint8_t, 10> myVar;
// myVar = 2.4f;	// converts to an int, stores 24
// float x = myVar; // converts back to float, returns 2.4f
template <typename T, int mult = 1>
class scaled_channel {
public:
	scaled_channel() : m_value(static_cast<T>(0)) { }
	scaled_channel(float val)
		: m_value(val * mult)
	{
	}

	// Allow reading back out as a float (note: this may be lossy!)
	operator float() const {
		return m_value / (float)mult;
	}

private:
	T m_value;
};

// We need to guarantee that scaled values containing some type are the same size
// as that underlying type.  We rely on the class only having a single member for
// this trick to work.
static_assert(sizeof(scaled_channel<uint8_t>) == 1);
static_assert(sizeof(scaled_channel<uint16_t>) == 2);
static_assert(sizeof(scaled_channel<uint32_t>) == 4);
static_assert(sizeof(scaled_channel<float>) == 4);

// Common scaling options - use these if you can!
using scaled_temperature = scaled_channel<int16_t, PACK_MULT_TEMPERATURE>;	// +-327 deg C at 0.01 deg resolution
using scaled_ms = scaled_channel<int16_t, PACK_MULT_MS>;				// +- 100ms at 0.003ms precision
using scaled_percent = scaled_channel<int16_t, PACK_MULT_PERCENT>;		// +-327% at 0.01% resolution
using scaled_pressure = scaled_channel<uint16_t, PACK_MULT_PRESSURE>;		// 0-2000kPa (~300psi) at 0.03kPa resolution
using scaled_angle = scaled_channel<int16_t, PACK_MULT_ANGLE>;			// +-655 degrees at 0.02 degree resolution
using scaled_voltage = scaled_channel<uint16_t, PACK_MULT_VOLTAGE>;		// 0-65v at 1mV resolution
using scaled_afr = scaled_channel<uint16_t, PACK_MULT_AFR>;			// 0-65afr at 0.001 resolution
