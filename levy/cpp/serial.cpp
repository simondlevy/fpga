#include <fcntl.h>   
#include <stdio.h>
#include <string.h>
#include <termios.h> 
#include <unistd.h>  
#include <sys/select.h>

#include "serial.h"

static const char * kPortName = "/dev/ttyUSB1";
static const speed_t kBaudRate = B4000000;
static const size_t kMaxBuf = 256;

static int fd_;
static fd_set read_fds_;
static struct timeval timeout_;
static uint8_t buf_[kMaxBuf];
static bool did_read_;
static uint8_t available_;
static uint8_t index_;

void Serial::Begin()
{
    fd_ = open(kPortName, O_RDWR);

    if (fd_ < 0) {
        fprintf(stderr, "Unable to open port %s\n", kPortName);
        return;
    }

    struct termios tty = {};

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

    cfsetispeed(&tty, kBaudRate);
    cfsetospeed(&tty, kBaudRate);

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr\n");
        return;
    }

    FD_ZERO(&read_fds_);
    FD_SET(fd_, &read_fds_);

    timeout_.tv_sec = 1;  // 1 second
    timeout_.tv_usec = 0; // 0 microseconds
}

uint32_t Serial::GetBaudRate()
{
    return 4000000;
}

void Serial::Write(const uint8_t byte)
{
    auto n = write(fd_, &byte, 1);
    (void)n;
}

void Serial::ReadBuffer()
{
    int select_res = select(fd_ + 1, &read_fds_, NULL, NULL, &timeout_);

    if (select_res > 0 && FD_ISSET(fd_, &read_fds_)) {

        printf("reading %ld bytes\n", read(fd_, buf_, sizeof(buf_) - 1));
    }
}

 
uint8_t Serial::Available()
{

    if (did_read_) {
        printf("available=%d\n", available_);
        return available_;
    }

    int select_res = select(fd_ + 1, &read_fds_, NULL, NULL, &timeout_);

    if (select_res > 0 && FD_ISSET(fd_, &read_fds_)) {

        printf("reading buffer\n");

        available_ = read(fd_, buf_, sizeof(buf_) - 1);

        index_ = 0;

        return available_;
    }

    return 0;
}

uint8_t Serial::Read()
{
    did_read_ = true;
    const auto byte = buf_[index_];
    index_++;
    available_--;
    return byte;
}

void Serial::Close()
{
    close(fd_);
}
