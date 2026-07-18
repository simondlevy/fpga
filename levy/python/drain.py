#!/usr/bin/env python3

from periphery import Serial

PORT_NAME = "/dev/ttyUSB1"
BAUD_RATE = 4000000
READ_TIMEOUT_SEC =  0.1

serial = Serial(PORT_NAME, BAUD_RATE)

while True:
    b = serial.read(1, READ_TIMEOUT_SEC)
    if len(b) == 0:
        break
    print("x%02X" % ord(b[1]))

