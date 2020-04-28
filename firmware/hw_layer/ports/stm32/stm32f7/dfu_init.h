/*
 * dfu_init.h
 *
 * @date Aug 3, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#ifndef HW_LAYER_PORTS_STM32_STM32F7_DFU_INIT_H_
#define HW_LAYER_PORTS_STM32_STM32F7_DFU_INIT_H_

// looks like this is System Control Block Base Address
LDR R1, =0xE000ED00
LDR R0, =0x1FF00000
// so this must be Vector Table Offset Register?
STR R0, [R1, #8]
LDR SP, [R0, #0]
LDR R0, [R0, #4]

#endif /* HW_LAYER_PORTS_STM32_STM32F7_DFU_INIT_H_ */
