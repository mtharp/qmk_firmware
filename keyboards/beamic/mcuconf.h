// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

// The gpio alt func number corresponding to the timer(s) performing capture
#define TIM345_ALT_FUNC 2

// The frequency at which the pace timer and capture timer(s) tick
#define CAPTURE_FREQUENCY STM32_TIMCLK2

// IRQ priority for pace timer
#define STM32_IRQ_TIM1_UP_PRIORITY 2
#define STM32_IRQ_TIM1_CC_PRIORITY 2
