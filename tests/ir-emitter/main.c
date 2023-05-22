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
#include "ir-emitter.h"
#include "ir-defs.h"
#include "pwmctl.h"

struct stExpectedData {
	uint16_t pulses;
	uint8_t pwmState;
};

const struct stExpectedData items[] = {
	// Command message (payload = 00 46) ===============================
	
	// 00 - MESSAGE START BURST
	{ .pulses = MSG_START_PULSES, .pwmState = 1, },
	//      COMMAND MESSAGE PAUSE
	{ .pulses = COMMAND_MSG_PULSES - MSG_START_PULSES, .pwmState = 0, },
	//      bit start burst - BYTE 0 - LSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 05 - data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 10 - bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 15 - data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - byte 0 - MSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - BYTE 1 - LSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 20 - bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 25 - data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 30 - bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - byte 1 - MSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - BYTE 2 - LSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 35 - data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 40 - bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 45 - data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - byte 2 - MSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 50 - bit start burst - BYTE 3 - LSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 55 - data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	// 60 - bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	//      data bit LOW pause
	{ .pulses = BIT_LOW_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      bit start burst - byte 3 - MSB
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	// 65 - data bit HIGH pause
	{ .pulses = BIT_HIGH_PULSES - BIT_START_PULSES, .pwmState = 0, },
	//      MESSAGE END BURST
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
	
	// Keypress message ================================================
	
	//      pause between messages
	{ .pulses = KEYPRESS_INTERVAL_PULSES, .pwmState = 0, },
	//      MESSAGE START BURST
	{ .pulses = MSG_START_PULSES, .pwmState = 1, },
	//      KEYPRESS MESSAGE PAUSE
	{ .pulses = KEYPRESS_MSG_PULSES - MSG_START_PULSES, .pwmState = 0, },
	// 70 - MESSAGE END BURST
	{ .pulses = BIT_START_PULSES, .pwmState = 1, },
};

static uint8_t pwmState = 0;
static uint16_t pwmPulses = 0;

void enablePwmOutput(uint16_t burstPulses) {
	pwmState = 1;
	pwmPulses = burstPulses;
}

void pausePwmOutput(uint16_t pausePulses) {
	pwmState = 0;
	pwmPulses = pausePulses;
}

void disablePwmOutput() {
	pwmState = 0;
	pwmPulses = 0;
}

int main() {
	irEmitter_initialise();
	
	uint8_t data[IR_DATA_SIZE];
	data[0] = 0x00;
	data[1] = 0x46;
	
	irEmitter_accept(data);
	
	size_t item = 0;
	
	while (item < (sizeof(items) / sizeof(struct stExpectedData))) {
		irEmitter_execute(0);
		
		const struct stExpectedData *data = &items[item];
		
		if (pwmPulses != data->pulses) {
			printf("FAILED: item %d, expected %d pulses, actual %d\n", item, data->pulses, pwmPulses);
			exit(0);
		}
		
		if (pwmState != data->pwmState) {
			printf("FAILED: item %d, expected PWM state %hhd, actual %hhd\n", item, data->pwmState, pwmState);
			exit(0);
		}
		
		pwmPulses = 0;
		item++;
	}
	
	printf("PASSED\n");
}
