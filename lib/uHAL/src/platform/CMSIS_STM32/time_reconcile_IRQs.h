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
// time.h
// Manage the time-keeping peripherals
// NOTES:
//    The names of the various timer IRQs aren't stable between devices so we
//    need to do this to fix them on a case-by-case basis
//
#ifndef _uHAL_PLATFORM_CMSIS_TIME_RECONCILE_IRQs_H
#define _uHAL_PLATFORM_CMSIS_TIME_RECONCILE_IRQs_H


#if defined(TIM1)
# if defined(TIM10)
#  define TIM1_IRQn TIM1_UP_TIM10_IRQn
#  define TIM1_IRQHandler TIM1_UP_TIM10_IRQHandler

#  define TIM10_IRQn TIM1_UP_TIM10_IRQn
#  define TIM10_IRQHandler TIM1_UP_TIM10_IRQHandler

#  define IS_SHARED_TIMER_1_OR_10(_t1_, _t2_) (((_t1_) == TIMER_1 || (_t1_) == TIMER_10) && ((_t2_) == TIMER_1 || (_t2_) == TIMER_10))

# else // defined(TIM10)
#  define TIM1_IRQn TIM1_UP_IRQn
#  define TIM1_IRQHandler TIM1_UP_IRQHandler
# endif

# if defined(TIM9)
#  define TIM9_IRQn TIM1_BRK_TIM9_IRQn
#  define TIM9_IRQHandler TIM1_BRK_TIM9_IRQHandler
# endif

# if defined(TIM11)
#  define TIM11_IRQn TIM1_TRG_COM_TIM11_IRQn
#  define TIM11_IRQHandler TIM1_TRG_COM_TIM11_IRQHandler
# endif
#endif

#if defined(TIM8)
# if defined(TIM13)
#  define TIM8_IRQn TIM8_UP_TIM13_IRQn
#  define TIM8_IRQHandler TIM8_UP_TIM13_IRQHandler

#  define TIM13_IRQn TIM8_UP_TIM13_IRQn
#  define TIM13_IRQHandler TIM8_UP_TIM13_IRQHandler

#  define IS_SHARED_TIMER_8_OR_13(_t1_, _t2_) (((_t1_) == TIMER_8 || (_t1_) == TIMER_13) && ((_t2_) == TIMER_8 || (_t2_) == TIMER_13))

# else // defined(TIM13)
#  define TIM8_IRQn TIM8_UP_IRQn
#  define TIM8_IRQHandler TIM8_UP_IRQHandler
# endif

# if defined(TIM12)
#  define TIM12_IRQn TIM8_BRK_TIM12_IRQn
#  define TIM12_IRQHandler TIM8_BRK_TIM12_IRQHandler
# endif

# if defined(TIM14)
#  define TIM14_IRQn TIM8_TRG_COM_TIM14_IRQn
#  define TIM14_IRQHandler TIM8_TRG_COM_TIM14_IRQHandler
# endif
#endif

#ifndef IS_SHARED_TIMER_8_OR_13
# define IS_SHARED_TIMER_8_OR_13(_t1_, _t2_) (0)
#endif
#ifndef IS_SHARED_TIMER_1_OR_10
# define IS_SHARED_TIMER_1_OR_10(_t1_, _t2_) (0)
#endif
#define TIMERS_SHARE_INTERRUPT(_tt1_, _tt2_) ((_tt1_) == (_tt2_) || IS_SHARED_TIMER_1_OR_10((_tt1_), (_tt2_)) || IS_SHARED_TIMER_8_OR_13((_tt1_), (_tt2_)))


#endif // _uHAL_PLATFORM_CMSIS_TIME_RECONCILE_IRQs_H
