/**
 * @file    interpolation.h
 *
 * @date Oct 17, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include <math.h>
#include "datalogging.h"
#include "efilib.h"
#include "obd_error_codes.h"
#include "error_handling.h"

#ifndef DEBUG_INTERPOLATION
#define DEBUG_INTERPOLATION FALSE
#endif

#define INTERPOLATION_A(x1, y1, x2, y2) ((y1 - y2) / (x1 - x2))

int findIndex(const float array[], int size, float value);
#define findIndexMsg(msg, array, size, value) findIndexMsgExt<float>(msg, array, size, value)
void ensureArrayIsAscending(const char *msg, const float array[], int size);
int findIndex2(const float array[], unsigned size, float value);
float interpolateClamped(float x1, float y1, float x2, float y2, float x);
float interpolateMsg(const char *msg, float x1, float y1, float x2, float y2, float x);

namespace priv {
float interpolate2d(const char *msg, float value, const float bin[], const float values[], int size);
}

template <int TSize>
float interpolate2d(const char *msg, const float value, const float (&bin)[TSize], const float (&values)[TSize]) {
	return priv::interpolate2d(msg, value, bin, values, TSize);
}

int needInterpolationLogging(void);

/** @brief	Binary search
 * @returns	the highest index within sorted array such that array[i] is greater than or equal to the parameter
 * @note If the parameter is smaller than the first element of the array, -1 is returned.
 *
 * See also ensureArrayIsAscending
 */
template<typename kType>
int findIndexMsgExt(const char *msg, const kType array[], int size, kType value) {
	float fvalue = (float)value;
	if (cisnan(fvalue)) {
		firmwareError(ERROR_NAN_FIND_INDEX, "NaN in findIndex%s", msg);
		return 0;
	}

	if (value < array[0])
		return -1;
	int middle;

	int left = 0;
	int right = size;

	// todo: extract binary search as template method?
	while (true) {
#if 0
		// that's an assertion to make sure we do not loop here
		size--;
		efiAssert(CUSTOM_ERR_ASSERT, size > 0, "Unexpected state in binary search", 0);
#endif

		// todo: compare current implementation with
		// http://eigenjoy.com/2011/01/21/worlds-fastest-binary-search/
		// ?
		middle = (left + right) / 2;

//		print("left=%d middle=%d right=%d: %.2f\r\n", left, middle, right, array[middle]);

		if (middle == left)
			break;

		if (middle != 0 && array[middle - 1] > array[middle]) {
#if EFI_UNIT_TEST
			firmwareError(CUSTOM_ERR_6610, "%s: out of order %.2f %.2f", msg, array[middle - 1], array[middle]);
#else
			warning(CUSTOM_ERR_OUT_OF_ORDER, "%s: out of order %.2f %.2f", msg, array[middle - 1], array[middle]);

#endif /* EFI_UNIT_TEST */
		}

		if (value < array[middle]) {
			right = middle;
		} else if (value > array[middle]) {
			left = middle;
		} else {
			break;
		}
	}

	return middle;
}

/**
 * @brief	Two-dimensional table lookup with linear interpolation
 */
template<typename vType, typename kType>
float interpolate3d(float x, const kType xBin[], int xBinSize, float y, const kType yBin[], int yBinSize, const vType* const map[]) {
	if (cisnan(x)) {
		warning(CUSTOM_INTEPOLATE_ERROR_3, "%.2f: x is NaN in interpolate3d", x);
		return NAN;
	}
	if (cisnan(y)) {
		warning(CUSTOM_INTEPOLATE_ERROR_2, "%.2f: y is NaN in interpolate3d", y);
		return NAN;
	}

	int xIndex = findIndexMsgExt<kType>("x", xBin, xBinSize, x);
#if	DEBUG_INTERPOLATION
	if (needInterpolationLogging())
		printf("X index=%d\r\n", xIndex);
#endif /* DEBUG_INTERPOLATION */
	int yIndex = findIndexMsgExt<kType>("y", yBin, yBinSize, y);
	if (xIndex < 0 && yIndex < 0) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("X and Y are smaller than smallest cell in table: %d\r\n", xIndex);
#endif /* DEBUG_INTERPOLATION */
		return map[0][0];
	}

	if (xIndex < 0) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("X is smaller than smallest cell in table: %dr\n", xIndex);
#endif /* DEBUG_INTERPOLATION */
		if (yIndex == yBinSize - 1)
			return map[0][yIndex];
		float keyMin = yBin[yIndex];
		float keyMax = yBin[yIndex + 1];
		float rpmMinValue = map[0][yIndex];
		float rpmMaxValue = map[0][yIndex + 1];

		return interpolateMsg("3d", keyMin, rpmMinValue, keyMax, rpmMaxValue, y);
	}

	if (yIndex < 0) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("Y is smaller than smallest cell in table: %d\r\n", yIndex);
#endif /* DEBUG_INTERPOLATION */
		if (xIndex == xBinSize - 1)
			return map[xIndex][0];
		float key1 = xBin[xIndex];
		float key2 = xBin[xIndex + 1];
		float value1 = map[xIndex][0];
		float value2 = map[xIndex + 1][0];

		return interpolateMsg("out3d", key1, value1, key2, value2, x);
	}

	if (xIndex == xBinSize - 1 && yIndex == yBinSize - 1) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("X and Y are larger than largest cell in table: %d %d\r\n", xIndex, yIndex);
#endif /* DEBUG_INTERPOLATION */
		return map[xBinSize - 1][yBinSize - 1];
	}

	if (xIndex == xBinSize - 1) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("TODO BETTER LOGGING x overflow %d\r\n", yIndex);
#endif /* DEBUG_INTERPOLATION */
		// here yIndex is less than yBinSize - 1, we've checked that condition already

		float key1 = yBin[yIndex];
		float key2 = yBin[yIndex + 1];
		float value1 = map[xIndex][yIndex];
		float value2 = map[xIndex][yIndex + 1];

		return interpolateMsg("out3d", key1, value1, key2, value2, y);
	}

	if (yIndex == yBinSize - 1) {
#if	DEBUG_INTERPOLATION
		if (needInterpolationLogging())
			printf("Y is larger than largest cell in table: %d\r\n", yIndex);
#endif /* DEBUG_INTERPOLATION */
		// here xIndex is less than xBinSize - 1, we've checked that condition already

		float key1 = xBin[xIndex];
		float key2 = xBin[xIndex + 1];
		float value1 = map[xIndex][yIndex];
		float value2 = map[xIndex + 1][yIndex];

		return interpolateMsg("out3d", key1, value1, key2, value2, x);
	}

	/*
	 * first we find the interpolated value for this RPM
	 */
	int rpmMaxIndex = xIndex + 1;

	float xMin = xBin[xIndex];
	float xMax = xBin[xIndex + 1];
	float rpmMinKeyMinValue = map[xIndex][yIndex];
	float rpmMaxKeyMinValue = map[xIndex + 1][yIndex];

	float keyMinValue = interpolateMsg("", xMin, rpmMinKeyMinValue, xMax, rpmMaxKeyMinValue, x);

#if	DEBUG_INTERPOLATION
	if (needInterpolationLogging()) {
		printf("X=%.2f:\r\nrange %.2f - %.2f\r\n", x, xMin, xMax);
		printf("X interpolation range %.2f   %.2f result %.2f\r\n", rpmMinKeyMinValue, rpmMaxKeyMinValue, keyMinValue);
	}
#endif /* DEBUG_INTERPOLATION */

	int keyMaxIndex = yIndex + 1;
	float keyMin = yBin[yIndex];
	float keyMax = yBin[keyMaxIndex];
	float rpmMinKeyMaxValue = map[xIndex][keyMaxIndex];
	float rpmMaxKeyMaxValue = map[rpmMaxIndex][keyMaxIndex];

	float keyMaxValue = interpolateMsg("3d", xMin, rpmMinKeyMaxValue, xMax, rpmMaxKeyMaxValue, x);

#if	DEBUG_INTERPOLATION
	if (needInterpolationLogging()) {
		printf("key=%.2f:\r\nrange %.2f - %.2f\r\n", y, keyMin, keyMax);
		printf("key interpolation range %.2f   %.2f result %.2f\r\n", rpmMinKeyMaxValue, rpmMaxKeyMaxValue, keyMaxValue);

		printf("%f", rpmMinKeyMaxValue);
		printf("%f", rpmMaxKeyMaxValue);
		printf("%f", keyMaxValue);
	}
#endif /* DEBUG_INTERPOLATION */

	return interpolateMsg("3d", keyMin, keyMinValue, keyMax, keyMaxValue, y);
}
void setCurveValue(float bins[], float values[], int size, float key, float value);
void initInterpolation(Logging *sharedLogger);

class FastInterpolation {
public:
	FastInterpolation();
	FastInterpolation(float x1, float y1, float x2, float y2);
	void init(float x1, float y1, float x2, float y2);
	float getValue(float x) const;
private:
	float a, b;
};
