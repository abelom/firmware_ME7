/*
 * @file mc33816.h
 *
 * @date May 3, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"

const int MAX_SPI_MODE_A_TRANSFER_SIZE = 31;  //max size for register config transfer

enum {
	CODE_RAM1,
	CODE_RAM2,
	DATA_RAM
};
enum {
	REG_MAIN,
	REG_CH1,
	REG_CH2,
	REG_IO,
	REG_DIAG
};

void initMc33816(Logging *logger);

