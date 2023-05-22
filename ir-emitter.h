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
#ifndef _IR_EMITTER_H
#define _IR_EMITTER_H

// 4 bytes are sent/received over IR, but the second and the fourth
// are the complements of the first and third respectively.
#define IR_PACKET_SIZE 4

// Each element is IR_PACKET_SIZE (i.e. 4) bytes
#define IR_TRANSMIT_QUEUE_ELEMENTS 2

typedef enum {
	NotTransmitting,
	TransmissionStarted,
	TransmissionInProgress,
	TransmissionEnded,
} IrEmitterStatus;

bool irEmitter_isReadyForData();
bool irEmitter_isIdle();
void irEmitter_accept(uint8_t *data);
void irEmitter_initialise();
IrEmitterStatus irEmitter_execute(uint16_t pwmPulseCount);

// These functions must be implemented in the host application.
void startBurst(uint16_t pulses);
void startPause(uint16_t pulses);

#endif // _IR_EMITTER_H
