#!/usr/bin/python3

kUseDronepong = True
kSpk = 1
kOpcodeWidth = 2                                                                              
kIndexWidth = 1                                                                               
kChargeWidth = 7 if kUseDronepong else 2                                                          
kOperandWidth = 14 if kUseDronepong else 6

idx = 1
chg = 63 if kUseDronepong else 1

bits = ((kSpk<<kOperandWidth) +
        (idx<<(kOperandWidth-kIndexWidth)) +
        (chg << (kOperandWidth-kChargeWidth-1)))


print('x%0X' % bits)

