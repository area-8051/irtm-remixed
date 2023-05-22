/*
 * SPDX-License-Identifier: BSD-2-Clause
 * 
 * Copyright (c) 2023 Vincent DEFERT. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _PROJECT_DEFS_H
#define _PROJECT_DEFS_H

#ifdef __SDCC
	#define IR_EMITTER_ENABLE_PORT GPIO_PORT5
	#define IR_EMITTER_ENABLE_PIN  GPIO_PIN4
	
	#ifdef BUILD_FOR_STC15W408AS
		#include <STC/15W4xxAS/DIP16.h>

		// == PIN ASSIGNMENTS ============================

		// Function | Name      | Pin DIP16 | Pin SKDIP28
		// ---------+-----------+-----------+-------------
		// UART RX  | P3.0      |    10     |     9
		// UART TX  | P3.1      |    14     |    16
		// IR RX    | CCP0/P1.1 |    16     |     4
		// IR TX    | CCP1/P1.0 |    15     |     3
		// IR EN    | P5.4      |     5     |    11

		#define IR_RECEIVER_CAPTURE    PCA_CHANNEL0
		#define IR_EMITTER_PWM         PCA_CHANNEL1
		#define IR_RECEIVER_PORT       GPIO_PORT1
		#define IR_RECEIVER_PIN        GPIO_PIN1
	#endif // BUILD_FOR_STC15W408AS
	
	#ifdef BUILD_FOR_STC8G1K08A
		#include <STC/8G1KxxA/DIP8.h>

		// == PIN ASSIGNMENTS ============================

		// Function | Name      | Pin 
		// ---------+-----------+-----
		// UART RX  | P3.0      |  5
		// UART TX  | P3.1      |  6
		// IR RX    | CCP1/P3.3 |  8
		// IR TX    | CCP0/P3.2 |  7
		// IR EN    | P5.4      |  1

		#define IR_RECEIVER_CAPTURE    PCA_CHANNEL1
		#define IR_EMITTER_PWM         PCA_CHANNEL0
		#define IR_RECEIVER_PORT       GPIO_PORT3
		#define IR_RECEIVER_PIN        GPIO_PIN3
	#endif // BUILD_FOR_STC8G1K08A
	
	#ifdef BUILD_FOR_STC8G1K17
		#include <STC/8G1Kxx/DIP16.h>

		// == PIN ASSIGNMENTS ============================

		// Function | Name      | Pin 
		// ---------+-----------+-----
		// UART RX  | P3.0      |  9
		// UART TX  | P3.1      |  10
		// IR RX    | CCP1/P1.0 |  1
		// IR TX    | CCP0/P1.1 |  2
		// IR EN    | P5.4      |  5

		#define IR_RECEIVER_CAPTURE    PCA_CHANNEL1
		#define IR_EMITTER_PWM         PCA_CHANNEL0
		#define IR_RECEIVER_PORT       GPIO_PORT1
		#define IR_RECEIVER_PIN        GPIO_PIN0
	#endif // BUILD_FOR_STC8G1K17
#else
	#include <uni-STC/uni-STC.h>
#endif // __SDCC

#define UART_PORT       UART1
#define UART_BAUD_RATE  115200UL
#define UART_PIN_CONFIG 0

#define PCA_PIN_CONFIG  0

// Valid values for PWM_BITS are 6, 7, and 8 (and 10 with an STC8).
#define PWM_BITS 7

// Save a few clock cycles every time the PCA ISR is executed
#define HAL_PCA_SEGMENT __data

// Manage to fit in IDATA + PDATA
#define UART1_RX_BUFFER_SIZE 8
#define UART1_TX_BUFFER_SIZE 8
#define UART1_SEGMENT __idata

// Manage to fit in 8K flash
#define BASIC_GPIO_HAL
#define HAL_PCA_CHANNELS 2

#endif // _PROJECT_DEFS_H
