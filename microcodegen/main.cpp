#include <iostream>
#include <fstream>

using namespace std;
/*

UNUSED       UNUSED       UNUSED     UNUSED ||
UNUSED       FEN          SQSH0      SQSH1 ||
SQSH2        BCASCADEMOD  BCLKMOD    BLOAD ||
BINMOD       BENDEF       ALUCIN     ACASCADEMOD ||
ACLKMOD      ALOAD        AINMOD     AENDEF ||
PCCASCADEMOD PCCLKMOD     PCLOAD     PCINMOD  ||
PCENDEF      RAMLOAD      MARENDEF   MARINMOD  ||
MARLOAD      MARCLKMOD    MARCASCADE IRLOAD


*/

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


/// WARNING: You can't have more than one component enabled
#define NONE 0
#define IRENO  (    0 |     0 | SQSH0)
#define RAMENO (    0 | SQSH1 |     0)
#define MARENO (    0 | SQSH1 | SQSH0)
#define PCENO  (SQSH2 |     0 |     0)
#define AENO   (SQSH2 |     0 | SQSH0)
#define ALUENO (SQSH2 | SQSH1 |     0)
#define BENO   (SQSH2 | SQSH1 | SQSH0)


#define HIGHBIT 0b10000

#define TOA   0x1
#define TOB   0x2
#define SUM   0x3
#define SUB   0x4
#define FROMA 0x5
#define FROMB 0x6
#define JMP   0x7
#define INCA  0x8
#define INCB  0x9
#define DECA  0xA
#define DECB  0xB
#define NEGA  0xC
#define NEGB  0xD
#define JMPC  0xE
#define JMPZ  0xF

/// TODO: Make JMP instructions jump to value of pointer instead of immediate
/// for better felxibility, also maybe do the same for FROMs


uint32_t rom[(1<<5) - 1];
int main(){
    /// There are 4 microsteps in an instruction
    /// Step 0 and 1 are handled by combinatorial logic to help reduce rom size, since they are constant
    /// Steps 2 and 3 are programmed here

    ofstream f("rom.bin", ios::out | ios::binary);
    /// Load data from address from immediate to A
    /// A.K.A: A = *(immediate)
    rom[TOA]            = MARLOAD | IRENO;
    rom[TOA|HIGHBIT]    = RAMENO  | ALOAD;

    /// Load data from address from immediate to B
    /// A.K.A: B = *(immediate)
    rom[TOB]            = MARLOAD | IRENO;
    rom[TOB|HIGHBIT]    = RAMENO  | BLOAD;

    /// Essentially A += B, saving flags
    rom[SUM]            = ALUENO  | ALOAD | FEN;
    rom[SUM|HIGHBIT]    = 0;

    /// Essentially A -= B, saving flags, and inverting B
    rom[SUB]            = BENDEF | BLOAD | BINMOD; // INVB
    rom[SUB|HIGHBIT]    = ALUENO | ALOAD | ALUCIN | FEN; // Add A to B, enabling Carry in

    /// Save value of A to address in immediate
    /// A.K.A: *(immediate) = A
    rom[FROMA]          = IRENO  | MARLOAD;
    rom[FROMA|HIGHBIT]  = AENO   | RAMLOAD;

    /// Save value of B to address in immediate
    /// A.K.A; *(immediate) = B
    rom[FROMB]          = IRENO  | MARLOAD;
    rom[FROMB|HIGHBIT]  = BENO   | RAMLOAD;

    /// Go to address at address from immediate
    /// A.K.A: goto immediate;
    rom[JMP]            = IRENO  | MARLOAD;
    rom[JMP|HIGHBIT]    = RAMENO | PCLOAD;

    /// A += 1, saving flags
    rom[INCA]           = AENDEF | AINMOD | ACLKMOD;
    rom[INCA|HIGHBIT]   = AENDEF | AINMOD | ACLKMOD | ALOAD | FEN;

    /// B += 1, saving flags
    rom[INCB]           = BENDEF | BINMOD | BCLKMOD;
    rom[INCB|HIGHBIT]   = BENDEF | BINMOD | BCLKMOD | BLOAD | FEN;

    /// A -= 1, saving flags
    rom[DECA]           = AENDEF | AINMOD | ACLKMOD | ACASCADEMOD;
    rom[DECA|HIGHBIT]   = AENDEF | AINMOD | ACLKMOD | ACASCADEMOD | ALOAD | FEN;

    /// B -= 1, saving flags
    rom[DECB]           = BENDEF | BINMOD | BCLKMOD | BCASCADEMOD;
    rom[DECB|HIGHBIT]   = BENDEF | BINMOD | BCLKMOD | BCASCADEMOD | BLOAD | FEN;

    /// A = -A, saving flags
    rom[NEGA]           = AENDEF | ALOAD | AINMOD;
    rom[NEGA|HIGHBIT]   = AENDEF | ALOAD | AINMOD | ACLKMOD | FEN;

    /// B = -B, saving flags
    rom[NEGB]           = BENDEF | BLOAD | BINMOD;
    rom[NEGB|HIGHBIT]   = BENDEF | BLOAD | BINMOD | BCLKMOD | FEN;

    /// If zero or carry flags are actually set that is handled by external combinatorial logic to help dramatiaclly reduce rom size
    /// So because of that external logic, we kind of have to set this to 0

    /// if(alu == 0) goto immediate;
    rom[JMPZ]           = 0; // IRENO  | MARLOAD
    rom[JMPZ|HIGHBIT]   = 0; // RAMENO | PCLOAD

    /// if(alu_carray == 1) goto immediate;
    rom[JMPC]           = 0; // IRENO  | MARLOAD
    rom[JMPC|HIGHBIT]   = 0; // RAMENO | PCLOAD

    f.write((char*)rom, sizeof(uint32_t)*((1<<5)));
}
