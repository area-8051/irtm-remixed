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
#include "ir-receiver.h"
#include "ir-defs.h"
#include <fifo-buffer.h>

FIFO_BUFFER(irReceiveQ, IR_RECEIVE_QUEUE_ELEMENTS * IR_DATA_SIZE, __idata)

struct {
	uint16_t burstLength;
	uint8_t byteCount;
	uint8_t value;
	uint8_t bitCount;
	IRMessageType messageType;
	uint8_t lastCommand[IR_DATA_SIZE];
} irReceiverState;

bool irReceiver_isIdle() {
	return irReceiverState.messageType == NoMessage;
}

bool irReceiver_isDataReady(uint8_t *destination) {
	return fifoRead(&irReceiveQ, destination, IR_DATA_SIZE);
}

typedef enum {
	ReceivedInvalidPulse = 0xff,
	ReceivedBitLow = 0,
	ReceivedBitHigh = 0x80,
	ReceivedCommandMsgStart = 1,
	ReceivedKeypressMsgStart = 2,
} IRReceivedElement;

// We allow a tolerance of +/- 10% (rounded to closest integer)
// on durations.
#define WITHIN_TOLERANCE(actual, expected) \
	(actual >= (expected - ((expected + 5) / 10)) \
	&& actual <= (expected + ((expected + 5) / 10)))

#define LOWER_BOUND(expected) \
	(expected - ((expected + 5) / 10))

static void irReceiver_reset() {
	irReceiverState.value = 0;
	irReceiverState.bitCount = 8;
	irReceiverState.byteCount = 0;
	irReceiverState.messageType = NoMessage;
}

void irReceiver_initialise() {
	irReceiverState.lastCommand[0] = 0;
	irReceiverState.lastCommand[1] = 0;
	irReceiver_reset();
}

void irReceiver_execute(IREvent *event) {
	if (event->isBurst) {
		// We've measured a burst
		// Save its length.
		irReceiverState.burstLength = event->length;
		
		switch (irReceiverState.messageType) {
		case KeypressMessage:
			// We're in a keypress message, so it's the trailing burst.
			// => Send the command message.
			fifoWrite(&irReceiveQ, irReceiverState.lastCommand, IR_DATA_SIZE);
			irReceiverState.messageType = NoMessage;
			break;
			
		case CommandMessage:
			if (irReceiverState.byteCount == 4) {
				// It's the trailing burst, reception complete.
				irReceiverState.messageType = NoMessage;
			}
			break;
		}
	} else {
		// We've measured a pause
		uint16_t elementDuration = irReceiverState.burstLength + event->length;
		IRReceivedElement receivedElement;
		
		switch (irReceiverState.messageType) {
		case CommandMessage:
			receivedElement = ReceivedInvalidPulse;
			
			if (WITHIN_TOLERANCE(elementDuration, BIT_LOW_PULSES)) {
				// We've read a logical 0
				receivedElement = ReceivedBitLow;
			} else if (WITHIN_TOLERANCE(elementDuration, BIT_HIGH_PULSES)) {
				// We've read a logical 1
				receivedElement = ReceivedBitHigh;
			} else if (WITHIN_TOLERANCE(elementDuration, KEYPRESS_MSG_PULSES)) {
				// We've measured the lead pause of a keypress message
				receivedElement = ReceivedKeypressMsgStart;
			} else if (WITHIN_TOLERANCE(elementDuration, COMMAND_MSG_PULSES)) {
				// We've measured the lead pause of a command message
				receivedElement = ReceivedCommandMsgStart;
			}
			
			switch (receivedElement) {
			case ReceivedBitLow:
			case ReceivedBitHigh:
				irReceiverState.value = (irReceiverState.value >> 1) | receivedElement;
				irReceiverState.bitCount--;
				
				if (irReceiverState.bitCount == 0) {
					// Bytes 1 and 3 are the complements of bytes 0 and 2 
					// respectively, so we can ignore them completely.
					switch (irReceiverState.byteCount) {
					case 0:
						irReceiverState.lastCommand[0] = irReceiverState.value;
						break;
					
					case 2:
						irReceiverState.lastCommand[1] = irReceiverState.value;
						break;
					}
					
					irReceiverState.value = 0;
					irReceiverState.bitCount = 8;
					irReceiverState.byteCount++;
				}
				break;
			
			case ReceivedKeypressMsgStart:
				irReceiverState.messageType = KeypressMessage;
				break;
			
			case ReceivedInvalidPulse:
				// Message format error, reset receiver.
				
				if (irReceiverState.byteCount) {
					// If a command message was partially received,
					// we want to start anew.
					irReceiver_initialise(); 
				} else {
					irReceiver_reset(); 
				}
				break;
			}
			break;
		
		case NoMessage:
			// We're expecting the pause preceding the lead burst,
			// let's check if its duration is consistent.
			if (event->length >= LOWER_BOUND(KEYPRESS_INTERVAL_PULSES)) {
				// Let's assume it's a command message for now 
				// (we need to measure the next pause before
				// we can decide which type of message it is).
				irReceiver_reset();
				irReceiverState.messageType = CommandMessage;
			}
			break;
		}
	}
}
