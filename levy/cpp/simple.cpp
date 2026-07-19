#include <stdio.h>

#include "newserial.h"

int main()
{
    Serial::Begin();

    Serial::Write(0xC0);

    /*
    Serial::Write(0x4A);
    Serial::Write(0x6A);
    Serial::Write(0x01);
    Serial::Write(0x4A);
    Serial::Write(0x6A);
    Serial::Write(0x01);
    Serial::Write(0x4A);
    Serial::Write(0x6A);
    Serial::Write(0x01);
    Serial::Write(0x4A);
    Serial::Write(0x6A);
    Serial::Write(0x01);
    Serial::Write(0x4A);
    Serial::Write(0x6A);
    Serial::Write(0x01);
    Serial::Write(0x80);
    */

    while (Serial::Available()) {
        printf("x%02x\n", Serial::Read());
    }

    Serial::Close();

    return 0;
}
