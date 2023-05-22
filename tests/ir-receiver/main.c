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

#include <stdlib.h>
#include <stdio.h>
#include "project-defs.h"
#include "ir-receiver.h"
#include "fifo-buffer.h"

struct stInjectedData {
	uint8_t pulseMeasured;
	uint16_t pulseLength;
	uint8_t gpioState;
};

const struct stInjectedData items[] = {
	// Command message (payload = 00 46) ===============================
	
	// pause between messages
	{ .pulseMeasured = 1, .pulseLength = 0xffff, .gpioState = 0, },
	// MESSAGE START BURST
	{ .pulseMeasured = 1, .pulseLength = 0x0155, .gpioState = 1, },
	// COMMAND MESSAGE PAUSE
	{ .pulseMeasured = 1, .pulseLength = 0xa9, .gpioState = 0, },
	// bit start burst - BYTE 0 - LSB
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x18, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 0, },
	// bit start burst - byte 0 - MSB
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst - BYTE 1 - LSB
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3e, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3e, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x40, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3e, .gpioState = 0, },
	// bit start burst - byte 1 - MSB
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst - BYTE 2 - LSB
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x18, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst - byte 2 - MSB
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst - BYTE 3 - LSB
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x14, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x40, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x16, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// bit start burst
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	// data bit LOW pause
	{ .pulseMeasured = 1, .pulseLength = 0x13, .gpioState = 0, },
	// bit start burst - byte 3 - MSB
	{ .pulseMeasured = 1, .pulseLength = 0x17, .gpioState = 1, },
	// data bit HIGH pause
	{ .pulseMeasured = 1, .pulseLength = 0x3f, .gpioState = 0, },
	// MESSAGE END BURST
	{ .pulseMeasured = 1, .pulseLength = 0x15, .gpioState = 1, },
	
	// Keypress message ================================================
	
	// pause between messages
	{ .pulseMeasured = 1, .pulseLength = 0x05e6, .gpioState = 0, },
	// MESSAGE START BURST
	{ .pulseMeasured = 1, .pulseLength = 0x0156, .gpioState = 1, },
	// KEYPRESS MESSAGE PAUSE
	{ .pulseMeasured = 1, .pulseLength = 0x53, .gpioState = 0, },
	// MESSAGE END BURST
	{ .pulseMeasured = 1, .pulseLength = 0x18, .gpioState = 1, },
};

static size_t item = 0;

int main() {
	irReceiver_initialise();
	
	while (item < (sizeof(items) / sizeof(struct stInjectedData))) {
		const struct stInjectedData *data = &items[item++];
		irReceiver_execute(data->gpioState, data->pulseLength);
	}
	
	const uint8_t eCount = 2;
	const uint8_t eAddress = 0x00;
	const uint8_t eCommand = 0x46;
	uint8_t data[2];
	
	if (irReceiver_isDataReady(data)) {
		if (data[0] == eAddress && data[1] == eCommand) {
			printf("PASSED\n");
		} else {
			printf("FAILED: expected data %02hhx %02hhx, actual %02hhx %02hhx\n", eAddress, eCommand, data[0], data[1]);
		}
	} else {
		printf("FAILED: expected count %hhd, actual %hhd\n", eCount, 0);
	}
}
