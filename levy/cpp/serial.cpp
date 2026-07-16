/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#include "serial.h"

void Serial::Begin(const float timeout_sec)
{
    fd_ = open(port_name_.c_str(), O_RDWR);

    if (fd_ < 0) {
        fprintf(stderr, "Unable to open port %s\n", port_name_.c_str());
        return;
    }

    struct termios tty;

    // Read existing settings from the port
    if(tcgetattr(fd_, &tty) != 0) {
        fprintf(stderr, "Error from tcgetattr\n");
        close(fd_);
        return;
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit (No parity)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field (1 stop bit)
    tty.c_cflag &= ~CSIZE;         // Clear bits-per-byte size
    tty.c_cflag |= CS8;            // 8 data bits
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines

    tty.c_lflag &= ~ICANON;        // Disable canonical mode (raw data mode)
    tty.c_lflag &= ~ECHO;          // Disable echo
    tty.c_lflag &= ~ISIG;          // Disable interpretation of INTR, QUIT, SUSP

    cfsetispeed(&tty, baud_rate_);
    cfsetospeed(&tty, baud_rate_);

    if (timeout_sec > 0) {
        tty.c_cc[VMIN] = 0;   // 0 means read() returns immediately if VTIME expires
        tty.c_cc[VTIME] = timeout_sec * 10; // Timeout in deciseconds
    }

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr\n");
    }
}

void Serial::Flush()
{
    tcflush(fd_, TCOFLUSH);
    tcdrain(fd_);
}

int Serial::Read(void * msg, const int size)
{
    return read(fd_, msg, size);
}

void Serial::Write(const void * msg, const int size)
{
    write(fd_, msg, sizeof(msg));
}

void Serial::Close()
{
    close(fd_);
}

