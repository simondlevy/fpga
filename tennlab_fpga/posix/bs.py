#!/usr/bin/python3
from bitstruct import pack
from enum import Enum, IntEnum, auto

class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()

def printspk(fmtstr, idx, chg):
    print('AS %d %d : ' % (idx, chg), end='')
    packed = pack(fmtstr, DispatchOpcode.SPK, idx, chg)
    print([('x%02X' % c) for c in packed], end=' | ')
    print([(f'{c:08b}') for c in packed])

printspk('u2u1s7', 0, 63)
printspk('u2u1s7', 1, 63)
