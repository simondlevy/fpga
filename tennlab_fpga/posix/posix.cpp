/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <string>

#include <processor.hpp>

static const char * kPort = "/dev/ttyUSB1";
static constexpr speed_t kBaudRate = B4000000;
static constexpr size_t kMaxMessageSize = 4096;
static constexpr uint32_t kDefaultTimeoutMsec = 20;

std::string port_;
int fd_;
uint8_t buf_[kMaxMessageSize] = {};
size_t index_;

void neuro::Processor::Connect()
{
    fd_ = open(kPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) {
        fprintf(stderr, "open %s: %s\n", kPort, strerror(errno));
        exit(1);
    }

    struct termios tio;
    if (tcgetattr(fd_, &tio) != 0) {
        perror("tcgetattr");
        close(fd_);
        exit(1);
    }

    cfmakeraw(&tio);
    tio.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
    tio.c_cflag |= CS8 | CLOCAL | CREAD;
    tio.c_cc[VMIN] = 0;  /* poll() does the waiting */
    tio.c_cc[VTIME] = 0;

    if (cfsetispeed(&tio, kBaudRate) != 0 || cfsetospeed(&tio, kBaudRate) != 0) {
        perror("cfsetspeed");
        close(fd_);
        exit(1);
    }
    if (tcsetattr(fd_, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        close(fd_);
        exit(1);
    }

    tcflush(fd_, TCIOFLUSH);
}

void neuro::Processor::Write(const uint8_t byte)
{
    write(fd_, &byte, 1);
}

auto neuro::Processor::Available() -> size_t
{
    size_t got = 0;

    // Read until the line has been quiet for kDefaultTimeoutMsec, or buf is full
    while (got < kMaxMessageSize) {

        struct pollfd pfd = { .fd = fd_, .events = POLLIN, .revents = 0 };

        int r = poll(&pfd, 1, kDefaultTimeoutMsec);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            perror("poll");
            exit(1);
        }
        if (r == 0)
            break;

        size_t n = read(fd_, buf_ + got, kMaxMessageSize - got);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            perror("read");
            exit(1);
        }
        if (n == 0)
            break;

        got += (size_t)n;
    }

    index_ = 0;

    return got;
}

auto neuro::Processor::Read() -> uint8_t
{
    return buf_[index_++];
}
