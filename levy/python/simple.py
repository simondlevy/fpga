#!/usr/bin/env python3

from periphery import Serial

PORT_NAME = "/dev/ttyUSB1"
BAUD_RATE = 4000000
READ_TIMEOUT_SEC =  0.1

def write_byte(serial, byte):
    serial.write(bytes([byte]))

def main():

    serial = Serial(PORT_NAME, BAUD_RATE)

    write_byte(serial, 0xC0)
    write_byte(serial, 0x4A)
    write_byte(serial, 0x6A)
    write_byte(serial, 0x01)
    write_byte(serial, 0x4A)
    write_byte(serial, 0x6A)
    write_byte(serial, 0x01)
    write_byte(serial, 0x4A)
    write_byte(serial, 0x6A)
    write_byte(serial, 0x01)
    write_byte(serial, 0x4A)
    write_byte(serial, 0x6A)
    write_byte(serial, 0x01)
    write_byte(serial, 0x4A)
    write_byte(serial, 0x6A)
    write_byte(serial, 0x01)
    write_byte(serial, 0x80)

    while True:
        b = serial.read(1, READ_TIMEOUT_SEC)
        if len(b) == 0:
            break
        print("x%02X" % ord(b))

main()

