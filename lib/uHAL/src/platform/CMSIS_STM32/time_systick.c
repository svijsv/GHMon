// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// time_*_.c
// Manage the system tick timer
// NOTES:
//

//#include "time_private.h"
#include "common.h"
#include "system.h"

#define SYSTICK_PSC_MAX (SysTick_LOAD_RELOAD_Msk + 1U)
enum {
	SYSTICK_CTRL_MASK = (SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk)
};

// System tick count, milliseconds
volatile utime_t G_sys_msticks = 0;

void SysTick_Handler(void) {
	++G_sys_msticks;
	return;
}

//
// Manage the systick timer
void systick_init(void) {
	uint32_t psc;
	uint32_t div;

	G_sys_msticks = 0;

#if (G_freq_HCLK/1000) < SYSTICK_PSC_MAX
	psc = ((G_freq_HCLK) / 1000U);
	div = 0b1U;
#else
	psc = ((G_freq_HCLK / 8U) / 1000U);
	div = 0b0U;
#endif
	uHAL_assert(psc > 0);
	uHAL_assert(SysTick_LOAD_RELOAD_Msk >= (psc - 1U));

	SysTick->LOAD = (psc - 1U);
	NVIC_SetPriority(SysTick_IRQn, SYSTICK_IRQp);
	SysTick->VAL = 0;
	MODIFY_BITS(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk,
		(div  << SysTick_CTRL_CLKSOURCE_Pos) | // Keep 0 for HCLCK/8; set to 1 for HCLK
		(0b1U << SysTick_CTRL_TICKINT_Pos  ) | // Enable the interrupt
		(0b1U << SysTick_CTRL_ENABLE_Pos   ) | // Enable the counter
		0U);

	return;
}
void disable_systick(void) {
	CLEAR_BIT(SysTick->CTRL, SYSTICK_CTRL_MASK);
	while (BITS_ARE_SET(SysTick->CTRL, SYSTICK_CTRL_MASK)) {
		// Nothing to do here
	}
	return;
}
void enable_systick(void) {
	SET_BIT(SysTick->CTRL, SYSTICK_CTRL_MASK);
	while (!BITS_ARE_SET(SysTick->CTRL, SYSTICK_CTRL_MASK)) {
		// Nothing to do here
	}
	return;
}
bool systick_is_enabled(void) {
	return BITS_ARE_SET(SysTick->CTRL, SYSTICK_CTRL_MASK);
}
