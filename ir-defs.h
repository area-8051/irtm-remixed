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
#ifndef _IR_DEFS_H
#define _IR_DEFS_H

// The NEC IR protocol uses a carrier frequency of 38 kHz,
// i.e. a period of 26.315789 microseconds.
#define F_CARRIER 38000UL

// Each carrier pulse is high for 8.77us. We artificially convert this
// into a "pseudo-frequency" of 114 kHz (= 1 / 8.77us) to simplify the
// calculation of the PWM threshold value.
#define F_UPTIME 114000UL

// See ./documentation/ for protocol details.

//  513 = 0x0201
#define COMMAND_MSG_PULSES 513
//  425 = 0x01a9
#define KEYPRESS_MSG_PULSES 425
//   43 = 0x002b
#define BIT_LOW_PULSES 43
//   86 = 0x0056
#define BIT_HIGH_PULSES 86

// The lead burst of a message lasts 9ms, i.e. 342 pulses
//  342 = 0x0156
#define MSG_START_PULSES 342

//   22 = 0x0016
// The bit start burst lasts 579us, i.e. 22 pulses
#define BIT_START_PULSES 22

// Interval between the command message and the first keypress message.
// 1510 = 0x05e6
#define KEYPRESS_INTERVAL_PULSES 1510

// Interval between repeated keypress messages.
// 3636 = 0x0e36
#define AUTOREPEAT_INTERVAL_PULSES 3636

typedef enum {
	NoMessage = 0,
	CommandMessage,
	KeypressMessage,
} IRMessageType;

// From our application's standpoint, an IR payload is only 2 bytes.
#define IR_DATA_SIZE 2

#endif // _IR_DEFS_H
