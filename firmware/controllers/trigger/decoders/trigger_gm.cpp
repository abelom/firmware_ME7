/**
 * @file	trigger_gm.cpp
 *
 * @date Mar 28, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "trigger_gm.h"

void configureGmTriggerWaveform(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR);

	// all angles are x2 here - so, 5 degree width is 10
	float w = 10;

	float m = CRANK_MODE_MULTIPLIER;

	s->addEvent720(m * 60 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 60, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 120 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 120.0, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 180 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 180, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 240 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 240.0, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 300 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 300.0, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 350 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 350.0, T_PRIMARY, TV_FALL);

	s->addEvent720(m * 360 - w, T_PRIMARY, TV_RISE);
	s->addEvent720(m * 360.0, T_PRIMARY, TV_FALL);

	s->setTriggerSynchronizationGap(6);
}

static int gm_tooth_pair(float startAngle, bool isShortLong, TriggerWaveform* s, int mult)
{
	int window = (isShortLong ? 5 : 10) * mult;
	int end = startAngle + mult * 15;

	s->addEvent720(startAngle + window, T_PRIMARY, TV_RISE);
	s->addEvent720(end, T_PRIMARY, TV_FALL);

	return end;
}

/**
 * TT_GM_LS_24
 * https://www.mediafire.com/?40mfgeoe4ctti
 * http://www.ls1gto.com/forums/archive/index.php/t-190549.htm
 * http://www.ls2.com/forums/showthread.php/834483-LS-Timing-Reluctor-Wheels-Explained
 *
 *
 * based on data in https://rusefi.com/forum/viewtopic.php?f=3&t=936&p=30303#p30285
 */
void initGmLS24(TriggerWaveform *s) {
	s->initialize(FOUR_STROKE_CRANK_SENSOR);

	/* 
	 * Okay, here's how this magic works:
	 * The GM 24x crank wheel has 48 edges.  There is
	 * a falling edge every 15 degrees (1/24 revolution).
	 * After every falling edge, a rising edge occurs either
	 * 5 or 10 (= 15 - 5) degrees later.  The code 0x0A33BE
	 * encodes the pattern of which type of gap occurs in the
	 * pattern.  Starting from the LSB, each bit left is the 
	 * next gap in sequence as the crank turns.  A 0 indicates
	 * long-short, while a 1 indicates short-long.
	 * 
	 * The first few bits read are 0xE (LSB first!) = 0 - 1 - 1 - 1, so the pattern
	 * looks like this:
	 * ___     _   ___   ___   ___
	 *    |___| |_|   |_|   |_|   |_ etc
	 * 
	 *    |  0  |  1  |  1  |  1  |
	 * 
	 *     ___ = 10 degrees, _ = 5 deg
	 * 
	 * There is a falling edge at angle=0=720, and this is position
	 * is #1 (and #6) TDC.  If there's a falling edge on the cam
	 * sensor, it's #1 end compression stroke (fire this plug!)
	 * and #6 end exhaust stroke.  If rising, it's exhaust #1,
	 * compression #6.
	 */

	uint32_t code = 0x0A33BE;

	int angle = 0;
	
	for(int i = 0; i < 24; i++)
	{
		bool bit = code & 0x000001;
		code = code >> 1;

		angle = gm_tooth_pair(angle, bit, s, CRANK_MODE_MULTIPLIER);
	}

	s->useOnlyPrimaryForSync = true;

	// This is tooth #20, at 310 degrees ATDC #1
	s->setTriggerSynchronizationGap(2.0f);
	s->setSecondTriggerSynchronizationGap(0.5f);
	s->setThirdTriggerSynchronizationGap(2.0f);
}

