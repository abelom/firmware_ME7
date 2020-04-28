/**
 * @file    expected.h
 * @brief This utility class provides a way for a function to accept or return a value that may be invalid.
 * 
 * For example, suppose there needs to be a solution for prevention of divide by zero.  One could write this function:
 * 
 * expected<int> my_divide(int num, int denom) {
 *     if (denom == 0) return unexpected;
 *     return num / denom;
 * }
 *
 * @date April 18, 2020
 * @author Matthew Kennedy, (c) 2020
 */

#pragma once

struct unexpected_t {};

template <class TValue>
struct expected {
	const bool Valid;
	const TValue Value;

	// Implicit constructor to construct in the invalid state
	constexpr expected(const unexpected_t&) : Valid(false), Value{} {}

	// Implicit constructor to convert from TValue (for valid values, so an expected<T> behaves like a T)
	constexpr expected(TValue validValue)
		: Valid(true)
		, Value(validValue)
	{
	}

	// Implicit conversion operator to bool, so you can do things like if (myResult) { ... }
	constexpr explicit operator bool() const {
		return Valid;
	}

	// Easy default value handling
	constexpr float value_or(TValue valueIfInvalid) const {
		return Valid ? Value : valueIfInvalid;
	}

	bool operator ==(const expected<TValue>& other) const {
		// If validity mismatch, not equal
		if (Valid != other.Valid) {
			return false;
		}
		
		// If both are invalid, they are equal
		if (!Valid && !other.Valid) {
			return true;
		}

		// Both are guaranteed valid - simply compare values
		return Value == other.Value;
	}
};

constexpr unexpected_t unexpected{};
