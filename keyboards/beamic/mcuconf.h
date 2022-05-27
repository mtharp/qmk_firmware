// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

// The gpio alt func number corresponding to the timer(s) performing capture
#define TIM345_ALT_FUNC 2

// The frequency at which the pace timer and capture timer(s) tick. Note that
// all the timers are on APB1 so they tick at the same rate without tweaking
// prescalers.
#define CAPTURE_FREQUENCY STM32_TIMCLK1

// IRQ priority for pace timer
#define STM32_IRQ_TIM4_PRIORITY 2
