#include <fcntl.h>   
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h> 
#include <unistd.h>  
#include <sys/select.h>

static const char * kPortName = "/dev/ttyUSB1";

class Serial {

    public:

        void Begin()
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

        void WriteByte(const uint8_t byte)
        {
            const uint8_t msg[1] = {byte};
            write(fd_, &byte, 1);
        }

        void Read()
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

                    uint8_t buf[256] = {};

                    int n = read(fd_, buf, sizeof(buf) - 1);

                    for (int k=0; k<n; ++k) {
                        printf("x%02X\n", buf[k]);
                    }
                }
            }
        }

        void Close()
        {
            close(fd_);
        }

    private:

        int fd_;
        fd_set read_fds_;
        struct timeval timeout_;
}; 


int main()
{
    Serial serial;

    serial.Begin();

    serial.WriteByte(0xC0);
    serial.WriteByte(0x4A);
    serial.WriteByte(0x6A);
    serial.WriteByte(0x01);
    serial.WriteByte(0x4A);
    serial.WriteByte(0x6A);
    serial.WriteByte(0x01);
    serial.WriteByte(0x4A);
    serial.WriteByte(0x6A);
    serial.WriteByte(0x01);
    serial.WriteByte(0x4A);
    serial.WriteByte(0x6A);
    serial.WriteByte(0x01);
    serial.WriteByte(0x4A);
    serial.WriteByte(0x6A);
    serial.WriteByte(0x01);
    serial.WriteByte(0x80);

    serial.Read();

    serial.Close();

    return 0;
}
