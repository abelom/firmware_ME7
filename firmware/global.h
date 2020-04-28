/*
 * @file global.h
 *
 * Global utility header file for firmware
 *
 * Simulator and unit tests have their own version of this header
 *
 * While this header contains 'EXTERN_ENGINE' and 'DECLARE_ENGINE_PARAMETER_SIGNATURE' magic,
 * this header is not allowed to actually include higher-level engine related headers
 *
 * @date May 27, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

// todo: remove this from here and rely on os_access.h. unfortunately hal.h includes ch.h :(
#include <hal.h>
// *** IMPORTANT *** from painful experience we know that common_headers.h has to be included AFTER hal.h
// *** https://github.com/rusefi/rusefi/issues/1007 ***
#include "common_headers.h"

// this is about MISRA not liking 'time.h'. todo: figure out something
#if defined __GNUC__
// GCC
#include <sys/types.h>
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
// IAR
typedef unsigned int time_t;
// todo: what's the IAR option?
#define ALWAYS_INLINE INLINE
#endif

#ifdef __cplusplus
#include "eficonsole.h"
#endif /* __cplusplus */


/* definition to expand macro then apply to pragma message */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

#define CORE_CLOCK STM32_SYSCLK
//#pragma message(VAR_NAME_VALUE(CORE_CLOCK))


/**
 * project-wide default thread stack size
 * See also PORT_INT_REQUIRED_STACK
 * See getRemainingStack()
 * See getMaxUsedStack()
 */
#ifndef UTILITY_THREAD_STACK_SIZE
#define UTILITY_THREAD_STACK_SIZE 400
#endif /* UTILITY_THREAD_STACK_SIZE */

#define EFI_ERROR_CODE 0xffffffff

#if EFI_USE_CCM && defined __GNUC__
#define MAIN_RAM __attribute__((section(".ram0")))
#elif defined __GNUC__
#define MAIN_RAM
#else
#define MAIN_RAM @ ".ram0"
#endif


/**
 * rusEfi is placing some of data structures into CCM memory simply
 * in order to use that memory - no magic about which RAM is faster etc.
 * That said, CCM/TCM could be faster as there will be less bus contention
 * with DMA.
 *
 * Please note that DMA does not work with CCM memory
 */
#if defined(STM32F7XX)
#define CCM_RAM ".ram3"
#define NO_CACHE CCM_OPTIONAL
#else /* defined(STM32F4XX) */
#define CCM_RAM ".ram4"
#define NO_CACHE
#endif /* defined(STM32F4XX) */

#if EFI_USE_CCM
#if defined __GNUC__
#define CCM_OPTIONAL __attribute__((section(CCM_RAM)))
#else // non-gcc
#define CCM_OPTIONAL @ CCM_RAM
#endif
#else /* !EFI_USE_CCM */
#define CCM_OPTIONAL
#endif /* EFI_USE_CCM */

#define getCurrentRemainingStack() getRemainingStack(chThdGetSelfX())


// 168 ticks in microsecond in case of 168MHz 407
#define US_TO_NT_MULTIPLIER (CORE_CLOCK / 1000000)

/**
 * converts efitimeus_t to efitick_t
 */
#define US2NT(us) (((efitime_t)(us))*US_TO_NT_MULTIPLIER)

/**
 * converts efitick_t to efitimeus_t
 */
#define NT2US(nt) ((nt) / US_TO_NT_MULTIPLIER)

#ifdef __cplusplus
extern "C"
{
#endif

bool lockAnyContext(void);
void unlockAnyContext(void);

#ifdef __cplusplus
}
#endif

