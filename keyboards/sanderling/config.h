// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define SN74X138_ADDRESS_PINS { B3, B2, B1 }

/* ESC */
#define BOOTMAGIC_LITE_ROW 7
#define BOOTMAGIC_LITE_COLUMN 2

#define LED_PIN_ON_STATE 0

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
