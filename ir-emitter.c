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
#include "ir-emitter.h"
#include "ir-defs.h"
#include <fifo-buffer.h>

typedef enum {
	Idle = 0,
	LeadBurst,
	LeadPause,
	DataBitBurst,
	DataBitPause,
	TrailingBurst,
	IntervalPause,
} IREmitterStatus;

FIFO_BUFFER(irTransmitQ, IR_TRANSMIT_QUEUE_ELEMENTS * IR_PACKET_SIZE, __idata)

struct {
	IREmitterStatus status;
	uint16_t pulses;
	uint8_t value;
	uint8_t byteCount;
	uint8_t bitCount;
	IRMessageType messageType;
	// Number of times to repeat the KeypressMessage after the first one.
	uint8_t repeatCount;
	uint8_t data[IR_PACKET_SIZE];
} irEmitterState;

bool irEmitter_isReadyForData() {
	return fifoBytesFree(&irTransmitQ) >= IR_PACKET_SIZE;
}

bool irEmitter_isIdle() {
	return irEmitterState.status == Idle;
}

void irEmitter_accept(uint8_t *data) {
	uint8_t packet[IR_PACKET_SIZE];
	
	packet[0] = data[0];
	packet[1] = ~packet[0];
	packet[2] = data[1];
	packet[3] = ~packet[2];
	fifoWrite(&irTransmitQ, packet, IR_PACKET_SIZE);
}

void irEmitter_initialise() {
	irEmitterState.status = Idle;
	irEmitterState.pulses = 0;
	irEmitterState.repeatCount = 0;
	irEmitterState.messageType = NoMessage;
}

IrEmitterStatus irEmitter_execute(uint16_t pwmPulseCount) {
	IrEmitterStatus rc = TransmissionInProgress;
	
	if (irEmitterState.messageType == NoMessage) {
		// Do we have something to send?
		if (fifoRead(&irTransmitQ, irEmitterState.data, IR_PACKET_SIZE)) {
			irEmitterState.byteCount = IR_PACKET_SIZE;
			irEmitterState.messageType = CommandMessage;
			irEmitterState.status = LeadBurst;
			startBurst(MSG_START_PULSES);
			rc = TransmissionStarted;
		} else {
			rc = NotTransmitting;
		}
	} else if (pwmPulseCount == 0) {
		// Count down finished, let's see what to do next.
		switch (irEmitterState.status) {
		case LeadBurst:
			// Lead burst complete, start appropriate pause.
			irEmitterState.status = LeadPause;
			
			switch (irEmitterState.messageType) {
			case CommandMessage:
				startPause(COMMAND_MSG_PULSES - MSG_START_PULSES);
				break;
			
			case KeypressMessage:
				startPause(KEYPRESS_MSG_PULSES - MSG_START_PULSES);
				break;
			}
			break;
			
		case LeadPause:
			// Lead pause complete.
			switch (irEmitterState.messageType) {
			case CommandMessage:
				// Start sending data bits.
				irEmitterState.bitCount = 0;
				irEmitterState.status = DataBitBurst;
				startBurst(BIT_START_PULSES);
				break;
			
			case KeypressMessage:
				// Keypress messages have no data bits,
				// continue with trailing burst.
				irEmitterState.status = TrailingBurst;
				startBurst(BIT_START_PULSES);
				break;
			}
			break;
		
		case DataBitBurst:
			// Data bit burst complete, start appropriate pause.
			if (irEmitterState.bitCount == 0 && irEmitterState.byteCount > 0) {
				irEmitterState.value = irEmitterState.data[IR_PACKET_SIZE - irEmitterState.byteCount];
				irEmitterState.byteCount--;
				irEmitterState.bitCount = 8;
			}
			
			uint8_t bitValue = irEmitterState.value & 1;
			irEmitterState.value = irEmitterState.value >> 1;
			irEmitterState.bitCount--;
			irEmitterState.status = DataBitPause;
			startPause((bitValue ? BIT_HIGH_PULSES : BIT_LOW_PULSES) - BIT_START_PULSES);
			break;
		
		case DataBitPause:
			// Data bit pause complete.
			if (irEmitterState.bitCount == 0 && irEmitterState.byteCount == 0) {
				// All data bits have been sent, continue with trailing burst.
				irEmitterState.status = TrailingBurst;
			} else {
				// Otherwise, start next data bit.
				irEmitterState.status = DataBitBurst;
			}
			
			startBurst(BIT_START_PULSES);
			break;
		
		case TrailingBurst:
			// Trailing burst complete.
			switch (irEmitterState.messageType) {
			case CommandMessage:
				// CommandMessage is finished, wait before 
				// starting KeypressMessage.
				irEmitterState.messageType = KeypressMessage;
				irEmitterState.status = IntervalPause;
				startPause(KEYPRESS_INTERVAL_PULSES);
				break;
			
			case KeypressMessage:
				// Do we need to repeat the KeypressMessage?
				if (irEmitterState.repeatCount) {
					// Yes: wait before starting the next one.
					irEmitterState.repeatCount--;
					irEmitterState.status = IntervalPause;
					startPause(AUTOREPEAT_INTERVAL_PULSES);
				} else {
					// No: we're all done!
					// Disable IR output completely
					startPause(0);
					// Make emitter ready for the next transmission.
					irEmitter_initialise();
					// And tell caller transmission is complete.
					rc = TransmissionEnded;
				}
				break;
			}
			break;
		
		case IntervalPause:
			// Interval finished, start new message (messageType
			// is already correctly set).
			irEmitterState.status = LeadBurst;
			startBurst(MSG_START_PULSES);
			break;
		}
	}
	
	return rc;
}
