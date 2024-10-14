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
// time_TCA.c
// Manage the TCA peripheral
//
// NOTES:
//

#include "time_TCA.h"

#if IS_TCA0(SYSTICK_TIMER)
# warning "The PWM frequency may be > PWM_FREQUENCY_HZ for TCA0 outputs"
# if IS_TCA0(USCOUNTER_TIMER)
#  error "USCOUNTER_TIMER and SYSTICK_TIMER can't be the same peripheral"
# endif
#endif
#if IS_TCA0(USCOUNTER_TIMER)
# warning "The PWM frequency may be > PWM_FREQUENCY_HZ for TCA0 outputs"
#endif

DEBUG_CPP_MACRO(TCA0_MAX_CNT)
DEBUG_CPP_MACRO(TCA0_DUTY_CYCLE_SCALE)
