/**
 * @file	trigger_nissan.cpp
 *
 * https://rusefi.com/forum/viewtopic.php?f=3&t=1194&start=150#p27784
 *
 * @date Sep 19, 2015
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "trigger_nissan.h"
#include "trigger_universal.h"

/**
 * 8,2,2,2 Nissan pattern
 */
static void initializeNissanSR20VE_4_optional_360(TriggerWaveform *s, bool with2nd) {
	s->initialize(FOUR_STROKE_CAM_SENSOR);
	s->isSynchronizationNeeded = true;
	s->gapBothDirections = true;
	s->useOnlyPrimaryForSync = true;

	s->tdcPosition = 630;

	float width = 4;
	s->setTriggerSynchronizationGap2(9.67 * 0.75, 16);

	float left = 0;
	float right;

	int total = 360; // 360 on cam, over 720 crank degree

	float base = 180;
	right = base - 4 * width;

	if (with2nd) {
		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
				left, right);
	}

	s->addEvent720(right, T_PRIMARY, TV_RISE);

	left = right;
	right = base;
	if (with2nd) {
		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
				left, right);
	}
	s->addEvent720(right, T_PRIMARY, TV_FALL);

	// was is the the one with 360 opto sensor?

	base += 180;

	left = right;
	right = base - width;
	if (with2nd) {
//		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
//				left, right PASS_ENGINE_PARAMETER_SUFFIX);
	}
	s->addEvent720(right, T_PRIMARY, TV_RISE);

	left = right;
	right = base;
	if (with2nd) {
//		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
//				left, right PASS_ENGINE_PARAMETER_SUFFIX);
	}
	s->addEvent720(right, T_PRIMARY, TV_FALL);

	base += 180;
	left = right;
	right = base - width;
	if (with2nd) {
//		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
//				left, right PASS_ENGINE_PARAMETER_SUFFIX);
	}
	s->addEvent720(right, T_PRIMARY, TV_RISE);

	left = right;
	right = base;
	if (with2nd) {
//		addSkippedToothTriggerEvents(T_SECONDARY, s, total, 0, 0.5, 0, 720,
//				left, right PASS_ENGINE_PARAMETER_SUFFIX);
	}
	s->addEvent720(right, T_PRIMARY, TV_FALL);

	base += 180;
	left = right;
	right = base - width;

	s->addEvent720(right, T_PRIMARY, TV_RISE);
	left = right;
	right = base;
	s->addEvent720(right, T_PRIMARY, TV_FALL);
}


/**
 * Nissan Primera p11 year 1995-2002
 */

void initializeNissanSR20VE_4(TriggerWaveform *s) {
	initializeNissanSR20VE_4_optional_360(s, false);
}

void initializeNissanSR20VE_4_360(TriggerWaveform *s) {
	initializeNissanSR20VE_4_optional_360(s, true);
}
