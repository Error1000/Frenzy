#include <iostream>
#include <fstream>

using namespace std;
/*

UNUSED        UNUSED       UNUSED     ACNTRLLINEMOD
BCNTRLLINEMOD FEN          SQSH0      SQSH1
SQSH2         BCASCADEMOD  BCLKMOD    BLOAD
BINMOD        BENDEF       ALUCIN     ACASCADEMOD
ACLKMOD       ALOAD        AINMOD     AENDEF
PCCASCADEMOD  PCCLKMOD     PCLOAD     PCINMOD
PCENDEF       RAMLOAD      MARENDEF   MARINMOD
MARLOAD       MARCLKMOD    MARCASCADE IRLOAD


*/

#define ALUAND          0b00100000000000000000000000000000
#define ACNTRLLINEMOD   0b00010000000000000000000000000000
#define BCNTRLLINEMOD   0b00001000000000000000000000000000

#define FEN             0b00000100000000000000000000000000

#define SQSH0           0b00000010000000000000000000000000
#define SQSH1           0b00000001000000000000000000000000
#define SQSH2           0b00000000100000000000000000000000

#define BCASCADEMOD     0b00000000010000000000000000000000
#define BCLKMOD         0b00000000001000000000000000000000
#define BLOAD           0b00000000000100000000000000000000
#define BINMOD          0b00000000000010000000000000000000
#define BENDEF          0b00000000000001000000000000000000

#define ALUCIN          0b00000000000000100000000000000000

#define ACASCADEMOD     0b00000000000000010000000000000000
#define ACLKMOD         0b00000000000000001000000000000000
#define ALOAD           0b00000000000000000100000000000000
#define AINMOD          0b00000000000000000010000000000000
#define AENDEF          0b00000000000000000001000000000000

#define PCCASCADEMOD    0b00000000000000000000100000000000
#define PCCLKMOD        0b00000000000000000000010000000000
#define PCLOAD          0b00000000000000000000001000000000
#define PCINMOD         0b00000000000000000000000100000000
#define PCENDEF         0b00000000000000000000000010000000

#define RAMLOAD         0b00000000000000000000000001000000

#define MARENDEF        0b00000000000000000000000000100000
#define MARINMOD        0b00000000000000000000000000010000
#define MARLOAD         0b00000000000000000000000000001000
#define MARCLKMOD       0b00000000000000000000000000000100
#define MARCASCADE      0b00000000000000000000000000000010

#define IRLOAD          0b00000000000000000000000000000001



/// WARNING: You can't have more than one of the following "control lines" as they are "compressed"
#define NONE   (    0 |     0 |     0)
#define IRENO  (    0 |     0 | SQSH0)
#define RAMENO (    0 | SQSH1 |     0)
#define MARENO (    0 | SQSH1 | SQSH0)
#define PCENO  (SQSH2 |     0 |     0)
#define AENO   (SQSH2 |     0 | SQSH0)
#define ALUENO (SQSH2 | SQSH1 |     0)
#define BENO   (SQSH2 | SQSH1 | SQSH0)

/// N.B. FEN is currently the only component that can be enabled while other components are enabled ( mostly because it's not connected to the bus )
/// The flags register is connected to the ALU not to A or B, so when a FEN happens it's actually looking at the ouput of the ALU even though the ALU may be unrelated to
/// that instruction: for ex. CHNGA will do a FEN


/// A cycle always consists of 4 steps, at least for now
/// STEP_1 and STEP_2 are handled in hardware
/// Right now they do the following control lines:
/// STEP_1: MARLOAD | PCENO  | PCENDEF | PCINMOD | PCCLKMOD
/// STEP_2: RAMENO  | IRLOAD | PCENDEF | PCINMOD | PCCLKMOD | PCLOAD
/// Basically STEP1 prepares the PC for incrementing while copying the current value of the PC to MAR
/// STEP_2 Takes the value from RAM at the location pointed to by MAR and loads it into IR, oh and also it finishes adding one to PC by enabling PCLOAD
/// The control lines for STEP_3 and STEP_4 are then the output of the ROM which takes as address the first 4 bits of the IR, and a single bit indicating wheter this is STEP_3 or STEP_4, for a total of 5 address bits
/// Except what the ROM says about STEP_3 and STEP_4 can be overriden for JMPZ and JMPC by combinatorial logic which takes as input the contents of the Flags Register, a.k.a the ZF and CF
/// And for CHNG-type instructions, again the control lines for the register may be overriden by combinatorial logic which takes as input the last 4 bits of the bus

#define STEP_3 0b00000
#define STEP_4 0b10000

#define NOOP  0x0
#define TOA   0x1
#define TOB   0x2
#define SUM   0x3
#define SUB   0x4
#define FROMA 0x5
#define FROMB 0x6
#define JMP   0x7
#define CHNGA 0x8
#define CHNGB 0x9
#define AND   0xA
/*
DEC, INC, INV, SET0, SETFF, SETLB0, SETLB1, NEG
0     1   2    3     4      5        6      7

Normally: 1. no LOAD, 2. LOAD
and these:
CASCADEMOD, CLKMOD, DEF, INMOD
SET0:   0, 0, 0, 0
SETFF:  0, 0, 1, 0
INV:    0, 0, 1, 1
SETLB1: 0, 1, 1, 0 // CLKMOD is on, and DEF = 1, hopefully OR1 without sidefects
INC:    0, 1, 1, 1
SETLB0: 1, 1, 0, 0 // CLKMOD = 1, but no INMOD, so this basically means all ops only affect the first bit
DEC:    1, 1, 1, 1

Except for NEG:
where 1. no CLKMOD, 2. CLKMOD
DEF, LOAD, INMOD
*/

// 3 free instructions

#define JMPC  0xE
#define JMPZ  0xF


uint32_t rom[(1<<5) - 1];
int main(){
    /// There are 4 microsteps in an instruction
    /// Step 1 and 2 are handled by combinatorial logic to help reduce rom size, since they are constant
    /// Steps 3 and 4 are programmed here

    ofstream f("rom.bin", ios::out | ios::binary);

    rom[NOOP|STEP_3]   = 0;
    rom[NOOP|STEP_4]   = 0;

    /// Load data from address from immediate to A
    /// A.K.A: A = *(immediate)
    rom[TOA|STEP_3]    = MARLOAD | IRENO;
    rom[TOA|STEP_4]    = RAMENO  | ALOAD;

    /// Load data from address from immediate to B
    /// A.K.A: B = *(immediate)
    rom[TOB|STEP_3]    = MARLOAD | IRENO;
    rom[TOB|STEP_4]    = RAMENO  | BLOAD;

    /// Essentially A += B, saving flags
    rom[SUM|STEP_3]    = ALUENO  | ALOAD | FEN;
    rom[SUM|STEP_4]    = 0;

    /// Essentially A -= B, saving flags, and inverting B
    rom[SUB|STEP_3]    = BENDEF | BLOAD | BINMOD; // INVB
    rom[SUB|STEP_4]    = ALUENO | ALOAD | ALUCIN | FEN; // Add A to B, enabling Carry in

    /// Save value of A to address in immediate
    /// A.K.A: *(immediate) = A
    rom[FROMA|STEP_3]  = IRENO  | MARLOAD;
    rom[FROMA|STEP_4]  = AENO   | RAMLOAD;

    /// Save value of B to address in immediate
    /// A.K.A; *(immediate) = B
    rom[FROMB|STEP_3]  = IRENO  | MARLOAD;
    rom[FROMB|STEP_4]  = BENO   | RAMLOAD;

    /// Go to address at address that's in the next byte to JMP in memory
    /// A.K.A: goto immediate;
    rom[JMP|STEP_3]    = PCENO  | MARLOAD;
    rom[JMP|STEP_4]    = RAMENO | PCLOAD;

    /// Take action on A based on immediate
    /*
        Current possibilities for immediate:
        DEC(0):    A -= 1;
        INC(1):    A += 1;
        INV(2):    A ^= 0xFF;
        SET0(3):   A = 0;
        SEFF(4):   A = 0xFF;
        SETLB0(5): A &= 0b11111110;
        SETLB1(6): A |= 0b00000001;
        NEG(7):    A = -B;

    */
    rom[CHNGA|STEP_3]  = IRENO  | ACNTRLLINEMOD;
    rom[CHNGA|STEP_4]  = IRENO  | ACNTRLLINEMOD | FEN;

    /// Take action on B based on immediate
    /*
        Current possibilities for immediate:
        DEC(0):    B -= 1;
        INC(1):    B += 1;
        INV(2):    B ^= 0xFF;
        SET0(3):   B = 0;
        SEFF(4):   B = 0xFF;
        SETLB0(5): B &= 0b11111110;
        SETLB1(6): B |= 0b00000001;
        NEG(7):    B = -B;

    */
    rom[CHNGB|STEP_3]  = IRENO  | BCNTRLLINEMOD;
    rom[CHNGB|STEP_4]  = IRENO  | BCNTRLLINEMOD | FEN;


    rom[AND|STEP_3]    = ALUAND | ALUENO | ALOAD | FEN;
    rom[AND|STEP_4]    = 0;
    /// If zero or carry flags are actually set that is handled by external combinatorial logic to help dramatiaclly reduce rom size
    /// So because of that external logic, we kind of have to set this to 0

    /// SINCE JMP is 2 bytes we need to do another add again for the CPU, if we didn't jump
    /// if(alu == 0) goto immediate;
    rom[JMPZ|STEP_3]   = PCENDEF | PCINMOD | PCCLKMOD;          // PCENO  | MARLOAD
    rom[JMPZ|STEP_4]   = PCENDEF | PCINMOD | PCCLKMOD | PCLOAD; // RAMENO | PCLOAD

    /// if(alu_carray == 1) goto immediate;
    rom[JMPC|STEP_3]   = PCENDEF | PCINMOD | PCCLKMOD;          // PCENO  | MARLOAD
    rom[JMPC|STEP_4]   = PCENDEF | PCINMOD | PCCLKMOD | PCLOAD; // RAMENO | PCLOAD

    f.write((char*)rom, sizeof(uint32_t)*((1<<5)));
}
