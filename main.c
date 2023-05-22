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
#include "project-defs.h"
#include <gpio-hal.h>
#include <pca-hal.h>
#include <uart-hal.h>
#include <timer-hal.h>
#include <fifo-buffer.h>
#include "ir-emitter.h"
#include "ir-receiver.h"
#include "ir-defs.h"
#include <delay.h>

/*
 * == DEFINITIONS ======================================================
 */

// With an n-bit PWM, F_PCA_COUNTER must be 2^n * F_CARRIER.
#define F_PCA_COUNTER  (F_CARRIER << PWM_BITS)

// *** IMPORTANT *******************************************************
// Don't forget to check F_CPU is an integer multiple of F_PCA_COUNTER 
// in order to guarantee the accuracy of F_CARRIER.
// *********************************************************************

// (The duty cycle is Tuptime/Tcarrier, i.e. F_CARRIER / F_UPTIME, 
// but we want to calculate the counter value representing Tuptime, 
// so we need to multiply the duty cycle by the counter value 
// representing Tcarrier, i.e. F_CARRIER * 2^PWM_BITS / F_UPTIME, 
// giving finally F_PCA_COUNTER / F_UPTIME).

#define PWM_DUTY_CYCLE ((uint16_t) (F_PCA_COUNTER / F_UPTIME))

/*
 * == IMPLEMENTATION ===================================================
 */

// IMPORTANT: we have to keep the PWM channel running permanently in 
// order to get its interrupts - otherwise, we wouldn't be able to 
// count the duration of bursts and pauses.

// However, the PWM function of the PCA doesn't allow to force the PWM
// output to a fixed state, so we need a separate pin to enable/disable 
// IR emission. We use P5_4 for this.

static GpioConfig irReceiverPin = GPIO_PIN_CONFIG(
	IR_RECEIVER_PORT, 
	IR_RECEIVER_PIN, 
	GPIO_BIDIRECTIONAL_MODE
);

static GpioConfig pwmEnablePin = GPIO_PIN_CONFIG(
	IR_EMITTER_ENABLE_PORT, 
	IR_EMITTER_ENABLE_PIN, 
	GPIO_BIDIRECTIONAL_MODE
);

static volatile uint16_t pwmPulseCount = 0;

// Each element is IR_EVENT_SIZE (i.e. 3) bytes
#define IR_EVENT_QUEUE_ELEMENTS 16
#define IR_EVENT_SIZE sizeof(IREvent)

FIFO_BUFFER(receiverEventQ, IR_EVENT_QUEUE_ELEMENTS * IR_EVENT_SIZE, /* use default segment */)

// @see ir-emitter.h
void startBurst(uint16_t pulses) {
	gpioWrite(&pwmEnablePin, 1);
	pwmPulseCount = pulses;
}

// @see ir-emitter.h
void startPause(uint16_t pulses) {
	gpioWrite(&pwmEnablePin, 0);
	pwmPulseCount = pulses;
}

// @see pca-hal.h
void pcaOnInterrupt(PCA_Channel channel, uint16_t HAL_PCA_SEGMENT pulseLength) {
	IREvent event;
	
	switch (channel) {
	case IR_RECEIVER_CAPTURE:
		event.isBurst = gpioRead(&irReceiverPin);
		event.length = pulseLength;
		fifoWrite(&receiverEventQ, &event, IR_EVENT_SIZE);
		break;
	
	case IR_EMITTER_PWM:
		if (pwmPulseCount) {
			pwmPulseCount--;
			
			if (!pwmPulseCount) {
				gpioWrite(&pwmEnablePin, 0);
			}
		}
		break;
	}
}

void main() {
	INIT_EXTENDED_SFR()
	
	// Initialise GPIO -------------------------------------------------
	gpioConfigure(&pwmEnablePin);
	gpioWrite(&pwmEnablePin, 0);

	// Initialise application state ------------------------------------
	irEmitter_initialise();
	irReceiver_initialise();
	
	// Enable interrupts -----------------------------------------------
	EA = 1;

	// Initialise UART -------------------------------------------------
	uartInitialise(
		UART_PORT, 
		UART_BAUD_RATE, 
		UART_USE_OWN_TIMER, 
		UART_8N1, 
		UART_PIN_CONFIG
	);
	
	// Initialise PCA --------------------------------------------------
	startTimer(
		TIMER0, 
		frequencyToSysclkDivisor(F_PCA_COUNTER), 
		DISABLE_OUTPUT, 
		DISABLE_INTERRUPT, 
		FREE_RUNNING
	);
	
	pcaConfigureInput(IR_RECEIVER_CAPTURE, GPIO_BIDIRECTIONAL_MODE);
	pcaConfigureOutput(IR_EMITTER_PWM, GPIO_BIDIRECTIONAL_MODE);
	
	pcaStartCounter(
		PCA_TIMER0, 
		FREE_RUNNING, 
		ENABLE_INTERRUPT, 
		PCA_PIN_CONFIG
	);
	
	pcaStartCapture(
		IR_RECEIVER_CAPTURE, 
		PCA_EDGE_BOTH, 
		PCA_CONTINUOUS, 
		PWM_BITS
	);
	
	pcaStartPwm(
		IR_EMITTER_PWM, 
		MAKE_PCA_PWM_BITS(PWM_BITS), 
		PCA_EDGE_FALLING, 
		PWM_DUTY_CYCLE
	);
	
	// Main loop -------------------------------------------------------
	
	while (1) {
		uint8_t data[IR_DATA_SIZE];
		
		// We don't want to receive what is being transmitted.
		if (irEmitter_isIdle()) {
			IREvent event;
			
			if (fifoRead(&receiverEventQ, &event, IR_EVENT_SIZE)) {
				irReceiver_execute(&event);
			}
		}

		// We don't want to disturb a reception in progress.
		if (irReceiver_isIdle()) {
			// Handle message from host
			if (irEmitter_isReadyForData()) {
				if (uartGetBlock(UART_PORT, data, IR_DATA_SIZE, NON_BLOCKING)) {
					irEmitter_accept(data);
				}
			}
			
			if (irEmitter_execute(pwmPulseCount) == TransmissionEnded) {
				// Transmission just finished, discard any event 
				// received so far.
				fifoClear(&receiverEventQ);
			}
		}
		
		// Send message to host
		if (uartTransmitBufferHasBytesFree(UART_PORT, IR_DATA_SIZE)) {
			if (irReceiver_isDataReady(data)) {
				uartSendBlock(UART_PORT, data, IR_DATA_SIZE, BLOCKING);
			}
		}
	}
}
