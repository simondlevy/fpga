from bitstruct import pack
from enum import Enum, IntEnum, auto

class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()

def printspk(fmtstr, idx, chg):
    print([('x%02X' % c) for c in pack(fmtstr, DispatchOpcode.SPK, idx, chg)])

## XOR:
##_to_fpga.spk_fmt_str:  u2u1s2 = opc_width, idx_width, charge_width
##_to_fpga.cmd_fmt_str:  u2u6 , opc_width, operand_width
##_from_fpga.spk_fmt_str:  u2 = opc_width
##_from_fpga.cmd_fmt_str:  u2u6 = opc_width,operand_width

printspk('u2u1s2', 1, 1)

## Dronepong:
##_to_fpga.spk_fmt_str:  u2u1s7
##_to_fpga.cmd_fmt_str:  u2u14
##_from_fpga.spk_fmt_str:  u2u1
##_from_fpga.cmd_fmt_str:  u2u6

printspk('u2u1s7', 0, 63)








