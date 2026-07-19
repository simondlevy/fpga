#include "newserial.h"

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
