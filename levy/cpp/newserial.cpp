#include <fcntl.h>   
#include <stdio.h>
#include <string.h>
#include <termios.h> 
#include <unistd.h>  
#include <sys/select.h>

#include "newserial.h"

static int fd_;
static fd_set read_fds_;
static struct timeval timeout_;
static uint8_t buf_[256];
static bool did_read_;
static uint8_t available_;
static uint8_t index_;

static const char * kPortName = "/dev/ttyUSB1";

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

    cfsetispeed(&tty, B4000000);
    cfsetospeed(&tty, B4000000);

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr\n");
        return;
    }

    FD_ZERO(&read_fds_);
    FD_SET(fd_, &read_fds_);

    timeout_.tv_sec = 1;  // 1 second
    timeout_.tv_usec = 0; // 0 microseconds
}

void Serial::WriteByte(const uint8_t byte)
{
    write(fd_, &byte, 1);
}

uint8_t Serial::Available()
{
    if (did_read_) {
        return available_;
    }

    int select_res = select(fd_ + 1, &read_fds_, NULL, NULL, &timeout_);

    if (select_res > 0 && FD_ISSET(fd_, &read_fds_)) {

        available_ = read(fd_, buf_, sizeof(buf_) - 1);

        index_ = 0;

        return available_;
    }

    did_read_ = false;

    return 0;
}

uint8_t Serial::ReadOne()
{
    did_read_ = true;
    const auto byte = buf_[index_];
    index_++;
    available_--;
    return byte;
}


void Serial::Read()
{
    int select_res = select(fd_ + 1, &read_fds_, NULL, NULL, &timeout_);

    if (select_res == -1) {
        perror("Select error");
    }

    else if (select_res == 0) {
        printf("Read timed out! No data received.\n");
    }

    else {

        if (FD_ISSET(fd_, &read_fds_)) {

            int n = read(fd_, buf_, sizeof(buf_) - 1);

            for (int k=0; k<n; ++k) {
                printf("x%02X\n", buf_[k]);
            }
        }
    }
}

void Serial::Close()
{
    close(fd_);
}
