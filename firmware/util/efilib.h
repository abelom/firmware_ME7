/**
 * @file	efilib.h
 *
 * @date Feb 21, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include <stdint.h>

#define _MAX_FILLER 11

// http://en.wikipedia.org/wiki/Endianness

#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

#define SWAP_UINT32(x) ((((x) >> 24) & 0xff) | (((x) << 8) & 0xff0000) | (((x) >> 8) & 0xff00) | (((x) << 24) & 0xff000000))

#define BIT(n) (UINT32_C(1) << (n))

// human-readable IDs start from 1 while computer-readbale indexes start from 0
#define ID2INDEX(id) ((id) - 1)

// number of milliseconds in one period of given frequency (per second)
#define frequency2periodMs(freq) ((1000.0f) / (freq))

// number of microseconds in one period of given frequency (per second)
#define frequency2periodUs(freq) ((1000000.0f) / (freq))

#define ERROR_CODE 311223344

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

const char * boolToString(bool value);

char * efiTrim(char *param);
int mytolower(const char c);
uint32_t efiStrlen(const char *param);
int efiPow10(int param);
bool startsWith(const char *line, const char *prefix);
int indexOf(const char *string, char ch);
float atoff(const char *string);
int atoi(const char *string);

#define UNUSED(x) (void)(x)
  
int absI(int32_t value);
float absF(float value);
/**
 * Rounds value to specified precision.
 * @param precision some pow of 10 value - for example, 100 for two digit precision
 */
float efiRound(float value, float precision);
int maxI(int i1, int i2);
int minI(int i1, int i2);
float maxF(float i1, float i2);
float minF(float i1, float i2);
char* itoa10(char *p, int num);
bool isSameF(float v1, float v2);
float clampF(float min, float clamp, float max);

/**
 * clamps value into the [0, 100] range
 */
#define clampPercentValue(x) (clampF(0, x, 100))

bool strEqualCaseInsensitive(const char *str1, const char *str2);
bool strEqual(const char *str1, const char *str2);

// Currently used by air-interp. tCharge mode (see EngineState::updateTChargeK()).
float limitRateOfChange(float newValue, float oldValue, float incrLimitPerSec, float decrLimitPerSec, float secsPassed);

// @brief Compute e^x using a 4th order taylor expansion centered at x=-1.  Provides
// bogus results outside the range -2 < x < 0.
float expf_taylor(float x);

#ifdef __cplusplus
}

#include <cstddef>

// C++ helpers go here
namespace efi
{
template <typename T, size_t N>
constexpr size_t size(const T(&)[N]) {
    return N;
}
} // namespace efi

/**
 * Copies an array from src to dest.  The lengths of the arrays must match.
 */
template <typename TElement, size_t N>
constexpr void copyArray(TElement (&dest)[N], const TElement (&src)[N]) {
	for (size_t i = 0; i < N; i++) {
		dest[i] = src[i];
	}
}

/**
 * Copies an array from src to the beginning of dst.  If dst is larger
 * than src, then only the elements copied from src will be touched.
 * Any remaining elements at the end will be untouched.
 */
template <typename TElement, size_t NSrc, size_t NDest>
constexpr void copyArrayPartial(TElement (&dest)[NDest], const TElement (&src)[NSrc]) {
	static_assert(NDest >= NSrc, "Source array must be larger than destination.");

	for (size_t i = 0; i < NSrc; i++) {
		dest[i] = src[i];
	}
}

#endif /* __cplusplus */

#if defined(__cplusplus) && defined(__OPTIMIZE__)
#include <type_traits>
// "g++ -O2" version, adds more strict type check and yet no "strict-aliasing" warnings!
#define cisnan(f) ({ \
	static_assert(sizeof(f) == sizeof(int32_t)); \
	union cisnanu_t { std::remove_reference_t<decltype(f)> __f; int32_t __i; } __cisnan_u = { f }; \
	__cisnan_u.__i == 0x7FC00000; \
})
#else
// "g++ -O0" or other C++/C compilers
#define cisnan(f) (*(((int*) (&f))) == 0x7FC00000)
#endif /* __cplusplus && __OPTIMIZE__ */
