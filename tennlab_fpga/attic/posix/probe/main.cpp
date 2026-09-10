#include "usb_serial.hpp"

static void do_read(neuro::UsbSerial & ser)
{
    const auto avail = ser.Available();
    for (size_t i=0; i<avail; ++i) {
        printf("read:  0x%02X\n", ser.Read(i));
    }
}

int main()
{
    auto ser = neuro::UsbSerial("/dev/ttyUSB1");

    ser.Begin();

    ser.Write(0xc0);
    do_read(ser);
    ser.Write(0xc0);
    do_read(ser);
    ser.Write(0x03);
    ser.Write(0x80);
    do_read(ser);

    printf("\n");

    ser.Write(0xc0);
    do_read(ser);
    ser.Write(0x68);
    ser.Write(0x03);
    ser.Write(0x80);
    do_read(ser);

    printf("\n");

    ser.Write(0xc0);
    do_read(ser);
    ser.Write(0x48);
    ser.Write(0x03);
    ser.Write(0x80);
    do_read(ser);

    printf("\n");

    ser.Write(0xc0);
    do_read(ser);
    ser.Write(0x48);
    ser.Write(0x68);
    ser.Write(0x03);
    ser.Write(0x80);
    do_read(ser);

    ser.Close();

    return 0;
}
