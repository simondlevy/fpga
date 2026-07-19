#include <stdio.h>

#include "newserial.h"

int main()
{
    Serial::Begin();

    Serial::WriteByte(0xC0);
    Serial::WriteByte(0x4A);
    Serial::WriteByte(0x6A);
    Serial::WriteByte(0x01);
    Serial::WriteByte(0x4A);
    Serial::WriteByte(0x6A);
    Serial::WriteByte(0x01);
    Serial::WriteByte(0x4A);
    Serial::WriteByte(0x6A);
    Serial::WriteByte(0x01);
    Serial::WriteByte(0x4A);
    Serial::WriteByte(0x6A);
    Serial::WriteByte(0x01);
    Serial::WriteByte(0x4A);
    Serial::WriteByte(0x6A);
    Serial::WriteByte(0x01);
    Serial::WriteByte(0x80);

    while (Serial::Available()) {
        printf("x%02x\n", Serial::ReadOne());
    }

    Serial::Close();

    return 0;
}
