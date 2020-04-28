/*
 * @file maf2map.h
 *
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @date Jan 20, 2018
 */

#pragma once

#include "table_helper.h"

#define ASIZE 16

typedef Map3D<ASIZE, ASIZE, float, float> maf2map_Map3D_t;

void initMaf2Map();
float estimateMapByRpmAndMaf(int rpm, float maf);
