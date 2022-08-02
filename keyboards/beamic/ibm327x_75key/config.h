// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define SOLENOID_PIN D15
#define SOLENOID_PIN_ACTIVE_LOW 1
#define HAPTIC_ENABLE_STATUS_LED D12
#define HAPTIC_OFF_IN_LOW_POWER 1
#define NO_HAPTIC_MOD 1

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT
