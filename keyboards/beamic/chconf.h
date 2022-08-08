// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// TIM2, the only 32-bit timer, is needed for other purposes so ChibiOS must use
// a 16-bit timer instead.
// #define CH_CFG_ST_RESOLUTION 16

// TODO: TIM15,16,17 don't work for some reason but those are the only ones
// available. Figure out why, but until then use core systick
#define CH_CFG_ST_FREQUENCY 1000
#define CH_CFG_ST_TIMEDELTA 0

#define CH_DBG_ENABLE_ASSERTS TRUE
#define CH_DBG_ENABLE_STACK_CHECK TRUE
#define CH_DBG_ENABLE_CHECKS   TRUE

#include_next <chconf.h>
