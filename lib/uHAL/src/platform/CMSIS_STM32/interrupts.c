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
// interrupts.c
// Manage system IRQs
// NOTES:
//   IRQs associated with a particular peripheral should be handled with the
//   rest of that peripheral
//   This file is for IRQs which aren't handled elsewhere
//
// Core IRQ handlers:
//    Reset_Handler
//    NMI_Handler
//    HardFault_Handler
//    MemManage_Handler
//    BusFault_Handler
//    UsageFault_Handler
//    SVC_Handler
//    DebugMon_Handler
//    PendSV_Handler
//    SysTick_Handler
// 
// STM32F103xB IRQ handlers:
//    WWDG_IRQHandler
//    PVD_IRQHandler
//    TAMPER_IRQHandler
//    RTC_IRQHandler
//    FLASH_IRQHandler
//    RCC_IRQHandler
//    EXTI0_IRQHandler
//    EXTI1_IRQHandler
//    EXTI2_IRQHandler
//    EXTI3_IRQHandler
//    EXTI4_IRQHandler
//    DMA1_Channel1_IRQHandler
//    DMA1_Channel2_IRQHandler
//    DMA1_Channel3_IRQHandler
//    DMA1_Channel4_IRQHandler
//    DMA1_Channel5_IRQHandler
//    DMA1_Channel6_IRQHandler
//    DMA1_Channel7_IRQHandler
//    ADC1_2_IRQHandler
//    USB_HP_CAN1_TX_IRQHandler
//    USB_LP_CAN1_RX0_IRQHandler
//    CAN1_RX1_IRQHandler
//    CAN1_SCE_IRQHandler
//    EXTI9_5_IRQHandler
//    TIM1_BRK_IRQHandler
//    TIM1_UP_IRQHandler
//    TIM1_TRG_COM_IRQHandler
//    TIM1_CC_IRQHandler
//    TIM2_IRQHandler
//    TIM3_IRQHandler
//    TIM4_IRQHandler
//    I2C1_EV_IRQHandler
//    I2C1_ER_IRQHandler
//    I2C2_EV_IRQHandler
//    I2C2_ER_IRQHandler
//    SPI1_IRQHandler
//    SPI2_IRQHandler
//    USART1_IRQHandler
//    USART2_IRQHandler
//    USART3_IRQHandler
//    EXTI15_10_IRQHandler
//    RTC_Alarm_IRQHandler
//    USBWakeUp_IRQHandler
//

#include "interrupts.h"
#include "system.h"


void NMI_Handler(void) {
	// Nothing to do here
}

void HardFault_Handler(void) {
	while (1) {
		// Nothing to do here
	}
}

void MemManage_Handler(void) {
	while (1) {
		// Nothing to do here
	}
}

void BusFault_Handler(void) {
	while (1) {
		// Nothing to do here
	}
}

void UsageFault_Handler(void) {
	while (1) {
		// Nothing to do here
	}
}

void SVC_Handler(void) {
	// Nothing to do here
}

void DebugMon_Handler(void) {
	// Nothing to do here
}

void PendSV_Handler(void) {
	// Nothing to do here
}

/*
void SysTick_Handler(void) {
	// Nothing to do here
}
*/

/*
void EXTI0_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI1_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI2_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI3_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI4_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI9_5_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
void EXTI15_10_IRQHandler(void) {
	EXTI_IRQHandler();
	return;
}
*/
