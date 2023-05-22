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
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

// Build with: gcc -o serial-tool serial-tool.c

/**
 * Sets the state of the DTR# pin.
 * 
 * serialPort: file descriptor
 * level: non-zero => set pin to logical "1", zero => set pin to logical "0"
 */
void setDTR(int serialPort, int level) {
	int ioLineMask = TIOCM_DTR;
	ioctl(serialPort, level ? TIOCMBIC : TIOCMBIS, &ioLineMask);
}

/**
 * Sets the state of the RTS# pin.
 * 
 * serialPort: file descriptor
 * level: non-zero => set pin to logical "1", zero => set pin to logical "0"
 */
void setRTS(int serialPort, int level) {
	int ioLineMask = TIOCM_RTS;
	ioctl(serialPort, level ? TIOCMBIC : TIOCMBIS, &ioLineMask);
}

int setCommunicationParameters(int serialPort, unsigned long baudRate, unsigned int readMinChars, unsigned int readTimeOut) {
	struct termios tty;
	
	if (tcgetattr(serialPort, &tty) < 0) {
		return -1;
	}
	
	cfmakeraw(&tty);
	
	// Enable receiver
	tty.c_cflag |= CREAD;
	// Ignore modem control lines
	tty.c_cflag |= CLOCAL;
	// Only one stop bit
	tty.c_cflag &= ~CSTOPB;

	tty.c_cc[VMIN] = readMinChars;
	// Note: readTimeOut is in tenths of second.
	tty.c_cc[VTIME] = readTimeOut;
	
	speed_t speed = B9600;
	
	switch (baudRate) {
		case 50:	speed = B50;	break;
		case 75:	speed = B75;	break;
		case 110:	speed = B110;	break;
		case 134:	speed = B134;	break;
		case 150:	speed = B150;	break;
		case 200:	speed = B200;	break;
		case 300:	speed = B300;	break;
		case 600:	speed = B600;	break;
		case 1200:	speed = B1200;	break;
		case 1800:	speed = B1800;	break;
		case 2400:	speed = B2400;	break;
		case 4800:	speed = B4800;	break;
		case 9600:	speed = B9600;	break;
		case 19200:	speed = B19200;	break;
		case 38400:	speed = B38400;	break;
		case 57600:	speed = B57600;	break;
#ifdef B76800
		case 76800:	speed = B76800;	break;
		case 153600:	speed = B153600;	break;
		case 307200:	speed = B307200;	break;
		case 614400:	speed = B614400;	break;
#endif
		case 115200:	speed = B115200;	break;
		case 230400:	speed = B230400;	break;
		case 460800:	speed = B460800;	break;
		case 500000:	speed = B500000;	break;
		case 576000:	speed = B576000;	break;
		case 921600:	speed = B921600;	break;
		case 1000000:	speed = B1000000;	break;
		case 1152000:	speed = B1152000;	break;
		case 1500000:	speed = B1500000;	break;
		case 2000000:	speed = B2000000;	break;
#ifdef B2500000
		case 2500000:	speed = B2500000;	break;
		case 3000000:	speed = B3000000;	break;
		case 3500000:	speed = B3500000;	break;
		case 4000000:	speed = B4000000;	break;
#endif
	}
	
	cfsetispeed(&tty, speed);
	cfsetospeed(&tty, speed);

	tcsetattr(serialPort, TCSANOW, &tty);
	
	tcflush(serialPort, TCIOFLUSH);
	
	setDTR(serialPort, 1);
	setRTS(serialPort, 1);
	
	return 0;
}

volatile int ctrlC = 0;

void handleCtrlC(int sigNum) {
	ctrlC = 1;
}

#define BYTES_PER_LINE 16

void main(int argc, char **argv) {
	if (argc < 4) {
		fprintf(stderr, "Usage: %s <device> <speed> <byte>...\n", argv[0]);
		fprintf(stderr, "<device> can be specified without the leading '/dev/' directory.\n");
		fprintf(stderr, "<byte> can be specified either as an 8-bit integer using the C syntax, or\n");
		fprintf(stderr, "as a string enclosed in double quotes.\n");
		fprintf(stderr, "Note 1: when invoking %s from the command line, these double-quotes\n", argv[0]);
		fprintf(stderr, "must be escaped, e.g.: %s ttyUSB0 9600 \"\\\"some string\\\"\".\n", argv[0]);
		fprintf(stderr, "Note 2: if the first value of <byte> is the string \"blocking\", the\n");
		fprintf(stderr, "read following the write(s) will block until at least one character\n");
		fprintf(stderr, "is read, e.g. %s ttyUSB0 9600 blocking.\n", argv[0]);
		exit(1);
	}
	
	int curArg = 1;
	char devicePath[256];
	
	if (argv[curArg][0] == '/') {
		strncpy(devicePath, argv[curArg], sizeof(devicePath) - 1);
		devicePath[sizeof(devicePath) - 1] = '\0';
	} else {
		strcpy(devicePath, "/dev/");
		strncat(devicePath, argv[1], sizeof(devicePath) - 1 - strlen(devicePath));
	}
	
	curArg++;
	errno = 0;
	unsigned long baudRate = strtoul(argv[curArg], (char **) NULL, 10);
	
	if (errno == EINVAL || errno == ERANGE) {
		fprintf(stderr, "FATAL: invalid baud rate value: %s\n", argv[curArg]);
		exit(1);
	}
	
	curArg++;
	int blockingRead = strcasecmp("blocking", argv[curArg]) ? 0 : 1;
	
	if (blockingRead) {
		curArg++;
	}
	
	int serialPort = open(devicePath, O_RDWR);
	
	if (serialPort < 0) {
		fprintf(stderr, "FATAL: can't open %s\n", devicePath);
		exit(1);
	}
	
	/*
	 * After the message has been sent, we'll wait for a response
	 * and hexdump all bytes received (if any). The timeout value
	 * is 5 tenths of second.
	 */
	if (setCommunicationParameters(serialPort, baudRate, blockingRead, blockingRead ? 0 : 5) < 0) {
		fprintf(stderr, "FATAL: %s is not a terminal\n", devicePath);
		exit(1);
	}
	
	for (int i = curArg; i < argc; i++) {
		if (argv[i][0] == '"') {
			for (int j = 1; argv[i][j] != '\0' && argv[i][j] != '"'; j++) {
				write(serialPort, &argv[i][j], 1);
			}
		} else {
			errno = 0;
			unsigned long number = strtoul(argv[i], (char **) NULL, 0);
			
			if (errno == EINVAL || errno == ERANGE) {
				fprintf(stderr, "FATAL: invalid number: %s\n", argv[i]);
				exit(1);
			}
			
			unsigned char byte = number & 0xff;
			write(serialPort, &byte, 1);
		}
	}
	
	// Installing a signal handler for SIGINT will allow
	// interrupting a blocking read().
	struct sigaction sigint_handler = {
		.sa_handler = handleCtrlC,
	};
	sigaction(SIGINT, &sigint_handler, 0);
	
	// Suppress echo of Ctrl-C on stdin.
	struct termios stdinTty;
	tcgetattr(0, &stdinTty);
	stdinTty.c_lflag &= ~ECHOCTL;
	tcsetattr(0, TCSANOW, &stdinTty);
	
	// Disable output buffering so the data bytes are printed
	// as they're received.
    setvbuf(stdout, NULL, _IONBF, 0);
	
	int rc;
	int linesPrinted = 0;
	int bytesPrinted = 0;
	unsigned char str[BYTES_PER_LINE + 1];
	unsigned char c;
	
	while ((!ctrlC) && (rc = read(serialPort, &c, 1)) > 0) {
		if (bytesPrinted == BYTES_PER_LINE) {
			str[bytesPrinted] = '\0';
			printf(" \"%s\"\n", str);
			bytesPrinted = 0;
			linesPrinted++;
		}
		
		if (bytesPrinted == 0) {
			printf("%04x:", linesPrinted * BYTES_PER_LINE);
		}
		
		printf(" %02hhx", c);
		str[bytesPrinted] = isprint(c) ? c : '.';
		bytesPrinted++;
	}
	
	if (bytesPrinted) {
		for (int i = bytesPrinted; i < BYTES_PER_LINE; i++) {
			printf("   ");
		}
		
		str[bytesPrinted] = '\0';
		printf(" \"%s\"\n", str);
	}
	
	close(serialPort);
}
