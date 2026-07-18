#include <fcntl.h>   
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h> 
#include <unistd.h>  
#include <sys/select.h>

static const char * kPortName = "/dev/ttyUSB1";
static const int kBaudRate = 4000000;
static const float kReadTimeoutSec =  0.1;

static void WriteByte(const int fd, const uint8_t byte)
{
    const uint8_t msg[1] = {byte};
    write(fd, msg, 1);
}

int main()
{
    const int fd = open(kPortName, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "Unable to open port %s\n", kPortName);
        return 1;
    }

    struct termios tty = {};

    // Read existing settings from the port
    if(tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error from tcgetattr\n");
        close(fd);
        return 1;
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

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr\n");
        return 1;
    }

    fd_set read_fds;
    struct timeval timeout;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    timeout.tv_sec = 1;  // 1 second
    timeout.tv_usec = 0; // 0 microseconds

    WriteByte(fd, 0xC0);
    WriteByte(fd, 0x4A);
    WriteByte(fd, 0x6A);
    WriteByte(fd, 0x01);
    WriteByte(fd, 0x4A);
    WriteByte(fd, 0x6A);
    WriteByte(fd, 0x01);
    WriteByte(fd, 0x4A);
    WriteByte(fd, 0x6A);
    WriteByte(fd, 0x01);
    WriteByte(fd, 0x4A);
    WriteByte(fd, 0x6A);
    WriteByte(fd, 0x01);
    WriteByte(fd, 0x4A);
    WriteByte(fd, 0x6A);
    WriteByte(fd, 0x01);
    WriteByte(fd, 0x80);

    int select_res = select(fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_res == -1) {
        perror("Select error");
    }
    
    else if (select_res == 0) {
        printf("Read timed out! No data received.\n");
    }
    
    else {

        if (FD_ISSET(fd, &read_fds)) {

            uint8_t buf[256] = {};

            int n = read(fd, buf, sizeof(buf) - 1);

            for (int k=0; k<n; ++k) {
                printf("x%02X\n", buf[k]);
            }
        }
    }

    close(fd);

    return 0;
}
