# SPDX-License-Identifier: BSD-2-Clause
# 
# Copyright (c) 2023 Vincent DEFERT. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
# 
# 1. Redistributions of source code must retain the above copyright 
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the 
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.

# Prerequisites --------------------------------------------------------
#
# Besides make, his project requires: 
#
# - sdcc
# - stcgal-patched
# - minicom
# - doxygen

# Usage ----------------------------------------------------------------
#
# Build executable in release mode:
#   make
#
# Build executable in debug mode:
#   make BUILD_MODE=debug
#
# Build documentation:
#   make doc
#
# Upload executable to MCU:
#   make upload
#
# Open serial console in new window:
#   make console
#
# Clean project (remove all build files):
#   make clean

# Target MCU settings --------------------------------------------------

TARGET_MCU := STC8G1K08A
STACK_SIZE := 112
# Valid choices are 29184 and 34048.
MCU_FREQ_KHZ := 34048

ifeq ($(TARGET_MCU),STC8G1K17)
	# STC8G1K17-36I-DIP16
	MEMORY_SIZES := \
		--xram-loc 0 \
		--xram-size 1024 \
		--stack-size $(STACK_SIZE) \
		--code-size 16384
	
	HAS_DUAL_DPTR := y
	
	# Code is smaller with the medium model, but faster with the large model.
	MEMORY_MODEL := --model-large
endif

ifeq ($(TARGET_MCU),STC8G1K08A)
	# STC8G1K08A-36I-DIP8
	MEMORY_SIZES := \
		--xram-loc 0 \
		--xram-size 1024 \
		--stack-size $(STACK_SIZE) \
		--code-size 8192
	
	HAS_DUAL_DPTR := y
	
	# Code is smaller with the medium model, but faster with the large model.
	MEMORY_MODEL := --model-large
endif

ifeq ($(TARGET_MCU),STC15W408AS)
	# STC15W408AS-35I-DIP16
	MEMORY_SIZES := \
		--xram-loc 0 \
		--xram-size 256 \
		--stack-size $(STACK_SIZE) \
		--code-size 8192
	
	HAS_DUAL_DPTR := n
	
	# The STC15W408AS doesn't have enough RAM to use the large model.
	MEMORY_MODEL := --model-medium
endif

# Define UNISTC_DIR, HAL_DIR, DRIVER_DIR, and MAKE_DIR -----------------
# TODO: Adjust path to match you installation directory
include /home/vincent/src/git/uni-STC/makefiles/0-directories.mk

# Project settings -----------------------------------------------------
PROJECT_NAME := irtm-remixed

PROJECT_FLAGS := -DBUILD_FOR_$(TARGET_MCU)

SRCS := \
	$(HAL_DIR)/delay.c \
	$(HAL_DIR)/fifo-buffer.c \
	$(HAL_DIR)/gpio-hal.c \
	$(HAL_DIR)/pca-hal.c \
	$(HAL_DIR)/timer-hal.c \
	$(HAL_DIR)/uart-hal.c \
	ir-emitter.c \
	ir-receiver.c \
	main.c

CONSOLE_BAUDRATE := 9600
CONSOLE_PORT := /dev/ttyUSB0

ISP_PORT := /dev/ttyUSB0
STCGAL_OPTIONS := -a -A rts

# Boilerplate rules ----------------------------------------------------
include $(MAKE_DIR)/1-mcu-settings.mk
-include $(DEP_FILE)
include $(MAKE_DIR)/2-mcu-rules.mk
