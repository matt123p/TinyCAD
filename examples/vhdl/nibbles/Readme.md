## "Nibbles" the 4-bit process which is an example for using VHDL ouput

These files show how to use the VHDL output system built in to TinyCAD.  You must use the VHDL library, which is included in the
directory above this one or you can find the VHDL files on [tinycad.net](https://www.tinycad.net/Search?key=%23vhdl).

## Times

Make the Register File: 
3 hours....

Make the DataPath: 
3 hours.... (i still need to test it)
1 hour (test)
1h45 (flags)

Decoder and running:
2h40

Functional Unit
B, A+B, A-B, A-B, NAND, NOR, SRA, SL

Flags: ZNCV

Formato de Instruções (bits):

Reg
Opcode 	DA/AA 	BA
4	2	2

Im
Opcode 	DA/AA 	IM
4	2	2
(com sinal no add sem sinal no mov)

Jump
Opcode 	Offset
4	4

Instruções:
MOVA, ADD,  SUB, CMP, NAND, NOR, SRA, SL
MOVI, ADDI, LD, ST, B, BZ,  BNZ,  BLU (Branch less than unsigned)

PC 8 bits

BC: NB, NB, NB, NB, B,   BZ,  BNZ,  BLU

WR: 1110 1111 1110 0000

Z = 0111 1111 0100 0000 : ~X3.X1. + ~X3.X2. + ~X2.~X1.X0
N = Z
C = 0111 0011 0100 0000 : ~X3.X1. + ~X2.~X1.X0
O = C

