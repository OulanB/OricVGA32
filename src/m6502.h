
#include "system_d.h"

#include "m6502_d.h"

#undef THIS
#define THIS System.Cpu

typedef union {
    WORD w;
    BYTE b[2];
} MEM;

static const CHAR *__des[] = {
//     0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
    "BRK", "ORA", "k02", "slo",	"nop", "ORA", "ASL", "slo", "PHP", "ORA", "ASL", "anc", "nop", "ORA", "ASL", "slo", // 0x
    "BPL", "ORA", "k12", "slo",	"nop", "ORA", "ASL", "slo",	"CLC", "ORA", "nop", "slo", "nop", "ORA", "ASL", "slo", // 1x
    "JSR", "AND", "k22", "rla", "BIT", "AND", "ROL", "rla", "PLP", "AND", "ROL", "anc", "BIT", "AND", "LSR", "rla", // 2x
    "BMI", "AND", "kil", "rla", "nop", "AND", "ROL", "rla", "SEC", "AND", "nop", "rla", "nop", "AND", "ROL", "rla", // 3x
    "RTI", "EOR", "kil", "sre", "nop", "EOR", "LSR", "sre", "PHA", "EOR", "LSR", "alr", "JMP", "EOR", "LSR", "sre", // 4x
    "BVC", "EOR", "kil", "sre", "nop", "EOR", "LSR", "sre", "CLI", "EOR", "nop", "sre", "nop", "EOR", "LSR", "sre", // 5x
    "RTS", "ADC", "kil", "rra", "nop", "ADC", "ROR", "rra", "PLA", "ADC", "ROR", "arr", "JMP", "ADC", "ROR", "rra", // 6x
    "BVS", "ADC", "kil", "rra", "nop", "ADC", "ROR", "rra", "SEI", "ADC", "nop", "rra", "nop", "ADC", "ROR", "rra", // 7x
    "nop", "STA", "nop", "sax", "STY", "STA", "STX", "sax", "DEY", "nop", "TXA", "xaa", "STY", "STA", "STX", "sax", // 8x
    "BCC", "STA", "kil", "sha", "STY", "STA", "STX", "sax", "TYA", "STA", "TXS", "tas", "shy", "STA", "shx", "ahx", // 9x
    "LDY", "LDA", "LDX", "lax", "LDY", "LDA", "LDX", "lax", "TAY", "LDA", "TAX", "lax", "LDY", "LDA", "LDX", "lax", // Ax
    "BCS", "LDA", "kil", "lax", "LDY", "LDA", "LDX", "lax", "CLV", "LDA", "TSX", "las", "LDY", "LDA", "LDX", "lax", // Bx
    "CPY", "CMP", "nop", "dcp", "CPY", "CMP", "DEC", "dcp", "INY", "CMP", "DEX", "axs", "CPY", "CMP", "DEC", "dcp", // Cx
    "BNE", "CMP", "kil", "dcp", "nop", "CMP", "DEC", "dcp", "CLD", "CMP", "nop", "dcp", "nop", "CMP", "DEC", "dcp", // Dx
    "CPX", "SBC", "nop", "isc", "CPX", "SBC", "INC", "isc", "INX", "SBC", "NOP", "sbc", "CPX", "SBC", "INC", "isc", // Ex
    "BEQ", "SBC", "kil", "isc", "nop", "SBC", "INC", "isc", "SED", "SBC", "nop", "isc", "nop", "SBC", "INC", "isc"  // Fx
//     0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
    };

static const BYTE __desc[] = {
//   0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
     5,     6,    14,     6,     3,     3,     3,     3,     5,     1,     4,     1,     2,     2,     2,     2,    // 0x - 0 : illegal
    11,     7,    14,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9,    // 1x - 1 : immediate
     2,     6,    14,     6,     3,     3,     3,     3,     5,     1,     4,     1,     2,     2,     2,     2,    // 2x - 2 : absolute
    11,     7,     0,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9,    // 3x - 3 : zero page
     5,     6,     0,     6,     3,     3,     3,     3,     5,     1,     4,     1,     2,     2,     2,     2,    // 4x - 4 : accumulator
    11,     7,     0,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9,    // 5x - 5 : implied
     5,     6,     0,     6,     3,     3,     3,     3,     5,     1,     4,     1,    12,     2,     2,     2,    // 6x - 6 : (,X)
    11,     7,     0,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9,    // 7x - 7 : (),Y
     1,     6,     1,     6,     3,     3,     3,     3,     5,     1,     5,     1,     2,     2,     2,     2,    // 8x - 8 : zp,X
    11,     7,     0,     7,     8,     8,    13,    13,     5,    10,     5,    10,     9,     9,    10,    10,    // 9x - 9 : abs,X
     1,     6,     1,     6,     3,     3,     3,     3,     5,     1,     5,     1,     2,     2,     2,     2,    // Ax -10 : abs,Y
    11,     7,     0,     7,     8,     8,    13,    13,     5,    10,     5,    10,     9,     9,    10,    10,    // Bx -11 : rel
     1,     6,     1,     6,     3,     3,     3,     3,     5,     1,     5,     1,     2,     2,     2,     2,    // Cx -12 : (ind)
    11,     7,     0,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9,    // Dx -13 : zp,Y
     1,     6,     1,     6,     3,     3,     3,     3,     5,     1,     5,     1,     2,     2,     2,     2,    // Ex -14 : patch
    11,     7,     0,     7,     8,     8,     8,     8,     5,    10,     5,    10,     9,     9,     9,     9     // Fx -15 :
//   0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
    };

static const SWORD __dcyc[] = {
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
    7,     6,    20,     8,     3,     3,     5,     5,     3,     2,     2,     2,     2,     4,     6,     6,    // 0x
    2,     5,    20,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7,    // 1x
    6,     6,    20,     8,     3,     3,     5,     5,     4,     2,     2,     2,     4,     4,     6,     6,    // 2x
    2,     5,     0,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7,    // 3x
    6,     6,     0,     8,     3,     3,     5,     5,     3,     2,     2,     2,     3,     4,     6,     6,    // 4x
    2,     5,     0,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7,    // 5x
    6,     6,     0,     8,     3,     3,     5,     5,     4,     2,     2,     2,     5,     4,     6,     6,    // 6x
    2,     5,     0,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7,    // 7x
    2,     6,     2,     6,     3,     3,     3,     3,     2,     2,     2,     2,     4,     4,     4,     4,    // 8x
    2,     5,     0,     6,     4,     4,     4,     4,     2,     4,     2,     5,     5,     5,     5,     5,    // 9x
    2,     6,     2,     6,     3,     3,     3,     3,     2,     2,     2,     2,     4,     4,     4,     4,    // Ax
    2,     5,     0,     5,     4,     4,     4,     4,     2,     4,     2,     4,     4,     4,     4,     4,    // Bx
    2,     6,     2,     8,     3,     3,     5,     5,     2,     2,     2,     2,     4,     4,     6,     6,    // Cx
    2,     5,     0,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7,    // Dx
    2,     6,     2,     8,     3,     3,     5,     5,     2,     2,     2,     2,     4,     4,     6,     6,    // Ex
    2,     5,     0,     8,     4,     4,     6,     6,     2,     4,     2,     7,     4,     4,     7,     7     // Fx
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
    };

static CHAR __buff1[128];
static CHAR __buff2[128];
static const CHAR __h[] = "0123456789ABCDEF";

static CHAR *_apd(CHAR *ad, const CHAR *str) {
    while(*str) {
        *ad++ = *str++;
    }
    return ad;
}
static CHAR *_apd2(CHAR *ad, BYTE n) {
    *ad++ = __h[n>>4]; *ad++ = __h[n & 0x0F];
    return ad;
}
static CHAR *_apd4(CHAR *ad, WORD n) {
    *ad++ = __h[n>>12]; *ad++ = __h[(n>>8) & 0x0F];
    *ad++ = __h[(n>>4) & 0x0F]; *ad++ = __h[n & 0x0F];
    return ad;
}

CHAR *_desass(WORD addr) {
    CHAR *ea = __buff1;
    CHAR *op = __buff2;
    WORD jr = 0x0000;			// address for jump
    BYTE i = 0x00;             // data 
    WORD start = addr;
    BYTE opc = systemPeek(addr++);
    ea = _apd(ea, __des[opc]); *ea++ = ' ';
    switch(__desc[opc]) {
        case 1:				// immediate
            ea = _apd(ea, "#$"); ea = _apd2(ea, systemPeek(addr++));
            break;
        case 2:				// absolute
            *ea++ = '$'; ea = _apd4(ea, systemDeek(addr)); addr += 2;
            break;
        case 3:				// zero page
            *ea++ = '$'; ea = _apd2(ea, systemPeek(addr++));
            break;
        case 4:				// accumulator
            *ea++ = 'A';
            break;
        case 5:				// implied
            //*ea++ = '';
            break;
        case 6:				// (,X)
            ea = _apd(ea, "($"); ea = _apd2(ea, systemPeek(addr++)); ea = _apd(ea, ",X)");
            break;
        case 7:				// (zp),Y
            ea = _apd(ea, "($"); ea = _apd2(ea, systemPeek(addr++)); ea = _apd(ea, "),Y");
            break;
        case 8:				// zp, X
            *ea++ = '$'; ea = _apd2(ea, systemPeek(addr++)); ea = _apd(ea, ",X");
            break;
        case 9:				// abs, X
            *ea++ = '$'; ea = _apd4(ea, systemDeek(addr)); ea = _apd(ea, ",X"); addr += 2;
            break;
        case 10:			// abs, Y
            *ea++ = '$'; ea = _apd4(ea, systemDeek(addr)); ea = _apd(ea, ",Y"); addr += 2;
            break;
        case 11:			// relative
            i = systemPeek(addr++);
            if (i > 0x7F) {
                jr = addr + i - 0x100;
                *ea++ = '-'; ea = _apd2(ea, 0x100-i); ea = _apd(ea, " ($"); ea = _apd4(ea, jr); *ea++ = ')';
            } else {
                jr = addr + i;
                *ea++ = '+'; ea = _apd2(ea, i); ea = _apd(ea, " ($"); ea = _apd4(ea, jr); *ea++ = ')';
            }
            break;
        case 12:			// (indirect)
            ea = _apd(ea, "($"); ea = _apd4(ea, systemDeek(addr)); *ea++ =')'; addr += 2;
            break;
        case 13:			// zp, Y
            *ea++ = '$'; ea = _apd2(ea, systemPeek(addr++)); ea = _apd(ea, ",Y");
            break;
        case 14:			// for patches
            ea = _apd(ea, "patches");
            break;
        default:
            ea = _apd(ea, "????");
            break;
    }
    *ea = 0x00;
    op = _apd4(op, start); *op++ = ' ';
    i = 0;
    while(start < addr) {
        op = _apd2(op, systemPeek(start++)); *op++ = ' ';
        i++;
    }
    while (i < 3) {
        op = _apd(op, "   ");
        i++;
    }
    op = _apd(op, "    "); op = _apd(op, __buff1);
    *op = 0x00;
    return __buff2;
} 

void m6502Desass(BYTE val) {
    THIS.desassOk = val;
}
    
static void _pushR(BYTE data) {
    systemWrite(THIS.S.w, data);
    THIS.S.b[0]--; 
}

static BYTE _popR() {
    THIS.S.b[0]++; 
    return systemRead(THIS.S.w);
}

void m6502Reset() {
    THIS.A = 0x00;
    THIS.X = 0x00;
    THIS.Y = 0x00;
    THIS.S.w = 0x1FF;
    THIS.IRQ = 0;
    THIS.NMI = 0;
    THIS.RESET = 1;
    THIS.C = 0;
    THIS.Z = 0;
    THIS.I = 1;
    THIS.D = 0;
    // THIS.B = 0;
    THIS.V = 0;
    THIS.N = 0;
    ioReset();
}

static inline void _addAcc(BYTE b) {		// add into acc
    if (THIS.D) {
        BYTE d, e, U, V;
        THIS.Z = ((THIS.A + b + THIS.C) & 0xFF) ? 0 : 1;
        U = THIS.A >> 7;
        V = b >> 7;
        d = (THIS.A & 0x0F) + (b & 0x0F) + THIS.C;
        if (d > 9) d += 6;
        e = (THIS.A >> 4) + (b >> 4) + ((d > 15) ? 1 : 0);
        if (e > 9) e += 6;
        THIS.A = ((e & 0xF) << 4) | (d & 0xF);
        THIS.N = THIS.A >> 7;
        THIS.V = 1 - ((U ^ THIS.N)  & (U ^ V));
        THIS.C = (e > 15) ? 1 : 0;
    } else {
        MEM d;
        d.w = THIS.A + b + THIS.C;
        THIS.V = ((THIS.A ^ b) & (THIS.A ^ d.b[0])) >> 7;
        THIS.C = d.b[1];
        THIS.A = d.b[0];
        THIS.Z = (THIS.A) ? 0 : 1;
        THIS.N = THIS.A >> 7;
    }
}

static inline void _subAcc(BYTE b) {		// sub into acc
    if (THIS.D) {
        BYTE d, e, U, V;
        THIS.Z = ((THIS.A - b - (1 - THIS.C)) & 0xFF) ? 0 : 1;
        U = THIS.A >> 7;
        V = b >> 7;
        d = (THIS.A & 0x0F) - (b & 0x0F) - (1 - THIS.C);
        if (d & 0x10) d -= 6;
        e = (THIS.A >> 4) - (b >> 4) - ((d & 0x10) >> 4);
        if (e & 0x10) e -= 6;
        THIS.A = ((e & 0xF) << 4) | (d & 0xF);
        // WORD acc = ((WORD)(cpu->A) - (WORD)(b) - (WORD)(cpu->P.C ? 0 : 1));
        THIS.N = THIS.A >> 7;
        THIS.V = 1 - ((U ^ THIS.N) & (U ^ V));
        THIS.C = (e > 15) ? 0 : 1;
    } else {
        MEM d;
        d.w = THIS.A - b - (1 - THIS.C);
        THIS.V = ((THIS.A ^ b) & (THIS.A ^ d.b[0])) >> 7;
        THIS.C = (d.b[1]) ? 0 : 1;
        THIS.A = d.b[0];
        THIS.Z = (THIS.A) ? 0 : 1;
        THIS.N = THIS.A >> 7;
    }
}

static inline void _doOpOne(BYTE op) {
    MEM a = { 0x0000 };    // EA of opcode
    MEM b = { 0x0000 };    // tmp address
    BYTE n = 0x00;      // data read at EA
    // BYTE i = 0x00;      // tmp data
    BYTE c = 0x00;      // carry
    THIS.dcycles += __dcyc[op];
    switch(__desc[op]) {
        case 1:				// immediate
            a.w = THIS.PC.w++;
            break;
        case 2:				// absolute
            a.b[0] = systemRead(THIS.PC.w++); a.b[1] = systemRead(THIS.PC.w++);
            break;
        case 3:				// zero page
            a.b[0] = systemRead(THIS.PC.w++);
            break;
        case 4:				// accumulator
            break;
        case 5:				// implied
            break;
        case 6:				// (,X)
            b.b[0] = systemRead(THIS.PC.w++) + THIS.X;
            a.b[0] = systemRead(b.w); b.b[0]++;                    // don't cross page boundary  
            a.b[1] = systemRead(b.w);			
            break;
        case 7:				// (zp),Y
            b.b[0] = systemRead(THIS.PC.w++);
            a.w = systemRead(b.w) + THIS.Y; b.b[0]++;              // don't cross page boundary
            if (a.b[0] == 0) THIS.dcycles++;
            a.b[1] += systemRead(b.w);                               
            break;
        case 8:				// zp, X
            a.b[0] = systemRead(THIS.PC.w++) + THIS.X;             // don't cross page boundary
            break;
        case 9:				// abs, X
            a.w = systemRead(THIS.PC.w++) + THIS.X; if (a.b[1]) THIS.dcycles++;
            a.b[1] += systemRead(THIS.PC.w++);
            break;
        case 10:			// abs, Y
            a.w = systemRead(THIS.PC.w++) + THIS.Y; if (a.b[1])  THIS.dcycles++;
            a.b[1] += systemRead(THIS.PC.w++);
            break;
        case 11:			// relative
            b.b[0] = systemRead(THIS.PC.w++);
            a.w = THIS.PC.w + b.w;
            if (b.b[0] > 0x7F) a.w -= 0x100;
            break;
        case 12:			// (indirect)
            b.b[0] = systemRead(THIS.PC.w++); b.b[1] = systemRead(THIS.PC.w++);
            a.b[0] = systemRead(b.w); b.b[0]++;						// don't cross page boundary
            a.b[1] = systemRead(b.w);
            break;
        case 13:			// zp, Y
            a.b[0] = systemRead(THIS.PC.w++) + THIS.Y;			    // don't cross page boundary ??
            break;
        case 14:			// for patches
            break;
        default:
            // fprintf(stdout, "unknow address mode\n");
            break;
    }
    switch(op) {
		case 0x02:					// read uint8_t from tape
            if (System.quickTape) {
                THIS.C = 0;
                THIS.A = tapeQuickRead();
                THIS.Z = (THIS.A == 0x00);
                THIS.N = (THIS.A & 0x80) != 0;
                System.ram[0x002F] = THIS.A;
                THIS.PC.b[0] = _popR();
                THIS.PC.b[1] = _popR();
                THIS.PC.w++;
            } else {
                // should lock the cpu
            }
            break;
        case 0x12:					// get sync from tape
            if (System.quickTape) {
                THIS.A = tapeQuickSync();
//                THIS.A = 0x16;
                THIS.C = (THIS.A & 0x80) != 0;
                THIS.A = THIS.A << 1;
                THIS.Z = (THIS.A == 0x00);
                THIS.N = (THIS.A & 0x80) != 0;
                System.ram[0x2F] = THIS.A;
                THIS.PC.b[0] = _popR();
                THIS.PC.b[1] = _popR();
                THIS.PC.w++;
            } else {
                // should lock the cpu
            }
            break;
        case 0x22:					// write uint8_t to tape
            if (System.quickTape) {
                tapeQuickWrite(THIS.A);
                System.ram[0x002F] = THIS.A;
                THIS.PC.b[0] = _popR();
                THIS.PC.b[1] = _popR();
                THIS.PC.w++;
            } else {
                // shoudl lock the cpu
            }
            break; 

        case 0x69:					// ADC imm
        case 0x6D:					// ADC abs
        case 0x65:					// ADC zp
        case 0x61:					// ADC (,X)
        case 0x71:					// ADC (),Y
        case 0x75:					// ADC zp,X
        case 0x7D:					// ADC abs,X
        case 0x79:					// ADC abs,Y
            _addAcc(systemRead(a.w)); 
            break;

        case 0x29:					// AND imm					2
        case 0x2D:					// AND abs					4
        case 0x25:					// AND zp					3
        case 0x21:					// AND (,X)					6
        case 0x31:					// AND (),Y					6 (+1)
        case 0x35:					// AND zp,X					4
        case 0x3D:					// AND abs,X				4 (+1)
        case 0x39:					// AND abs,Y				4 (+1)
            THIS.A &= systemRead(a.w);
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        case 0x0A:					// ASL A
            THIS.C = THIS.A >> 7;
            THIS.A = (THIS.A << 1) & 0xFF;
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;
        case 0x0E:					// ASL abs
        case 0x06:					// ASL zp
        case 0x16:					// ASL zp,X
        case 0x1E:					// ASL abs,X
            n = systemRead(a.w);
            THIS.C = n >> 7;
            n = (n << 1) & 0xFF;
            THIS.Z = (n) ? 0 : 1;
            THIS.N = n >> 7;
            systemWrite(a.w, n);
            break;

        case 0x90:					// BCC rel
            if (THIS.C == 0) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0xB0:					// BCS rel
            if (THIS.C) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0xF0:					// BEQ rel
            if (THIS.Z) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0x30:					// BMI rel
            if (THIS.N) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0xD0:					// BNE rel
            if (THIS.Z == 0) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0x10:					// BPL rel
            if (THIS.N == 0) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0x50:					// BVC rel
            if (THIS.V == 0) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;
        case 0x70:					// BVS rel
            if (THIS.V) {THIS.PC.w = a.w; THIS.dcycles++;}
            break;

        case 0x2C:					// BIT abs
        case 0x24:					// BIT zp
            n = systemRead(a.w);
            THIS.Z = (THIS.A & n) ? 0 : 1;
            THIS.V = (n >> 6) & 1;
            THIS.N = n >> 7;
            break;

        case 0x00:					// BRK
            THIS.PC.w++; 
            _pushR(THIS.PC.b[1]);
            _pushR(THIS.PC.b[0]);
            // _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.B << 4) | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
            _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | 0x10 | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
            THIS.PC.w = (systemRead(0xFFFF) << 8) | systemRead(0xFFFE);
            THIS.I = 1;
            THIS.dcycles += 7;
            break;

        case 0x18:					// CLC
            THIS.C = 0;
            break;
        case 0xD8:					// CLD
            THIS.D = 0;
            break;
        case 0x58:					// CLI
            THIS.I = 0;
            break;
        case 0xB8:					// CLV
            THIS.V = 0;
            break;

        case 0xC9:					// CMP imm
        case 0xCD:					// CMP abs
        case 0xC5:					// CMP zp
        case 0xC1:					// CMP (,X)
        case 0xD1:					// CMP (),Y
        case 0xD5:					// CMP zp,X
        case 0xDD:					// CMP abs,X
        case 0xD9:					// CMP abs,Y
            n = systemRead(a.w);
            THIS.C = (n <= THIS.A) ? 1 : 0;
            THIS.Z = (THIS.A - n) ? 0 : 1;
            THIS.N = ((THIS.A - n) >> 7) & 1;
            break;

        case 0xE0:					// CPX imm
        case 0xEC:					// CPX abs
        case 0xE4:					// CPX zp
            n = systemRead(a.w);
            THIS.C = (n <= THIS.X) ? 1 : 0;
            THIS.Z = (THIS.X - n) ? 0 : 1;
            THIS.N = ((THIS.X - n) >> 7) & 1;
            break;

        case 0xC0:					// CPY imm
        case 0xCC:					// CPY abs
        case 0xC4:					// CPY zp
            n = systemRead(a.w);
            THIS.C = (n <= THIS.Y) ? 1 : 0;
            THIS.Z = (THIS.Y - n) ? 0 : 1;
            THIS.N = ((THIS.Y - n) >> 7) & 1;
            break;

        case 0xCE:					// DEC abs
        case 0xC6:					// DEC zp
        case 0xD6:					// DEC zp,X
        case 0xDE:					// DEC abs,X
            n = systemRead(a.w) - 1;
            THIS.Z = (n) ? 0 : 1;
            THIS.N = n >> 7;
            systemWrite(a.w, n);
            break;
        case 0xCA:					// DEX
            THIS.X--;
            THIS.Z = (THIS.X) ? 0 : 1;
            THIS.N = THIS.X >> 7;
            break;
        case 0x88:					// DEY
            THIS.Y--;
            THIS.Z = (THIS.Y) ? 0 : 1;
            THIS.N = THIS.Y >> 7;
            break;

        case 0x49:					// EOR imm
        case 0x4D:					// EOR abs
        case 0x45:					// EOR zp
        case 0x41:					// EOR (,X)
        case 0x51:					// EOR (),Y
        case 0x55:					// EOR zp,X
        case 0x5D:					// EOR abs,X
        case 0x59:					// EOR abs,Y
            THIS.A ^= systemRead(a.w);
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        case 0xEE:					// INC abs
        case 0xE6:					// INC zp
        case 0xF6:					// INC zp,X
        case 0xFE:					// INC abs,X
            n = systemRead(a.w) + 1;
            THIS.Z = (n) ? 0 : 1;
            THIS.N = n >> 7;
            systemWrite(a.w, n);
            break;
        case 0xE8:					// INX
            THIS.X++;
            THIS.Z = (THIS.X) ? 0 : 1;
            THIS.N = THIS.X >> 7;
            break;
        case 0xC8:					// INY
            THIS.Y++;
            THIS.Z = (THIS.Y) ? 0 : 1;
            THIS.N = THIS.Y >> 7;
            break;

        case 0x20:					// JSR abs
            THIS.PC.w--;
            _pushR(THIS.PC.b[1]);
            _pushR(THIS.PC.b[0]);
        case 0x4C:					// JMP abs
        case 0x6C:					// JMP ind
            THIS.PC.w = a.w;
            break;

        case 0xA9:					// LDA imm
        case 0xAD:					// LDA abs
        case 0xA5:					// LDA zp
        case 0xA1:					// LDA (,X)
        case 0xB1:					// LDA (),Y
        case 0xB5:					// LDA zp,X
        case 0xBD:					// LDA abs,X
        case 0xB9:					// LDA abs,Y
            THIS.A = systemRead(a.w);
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        case 0xA2:					// LDX imm
        case 0xAE:					// LDX abs
        case 0xA6:					// LDX zp
        case 0xBE:					// LDX abs,Y
        case 0xB6:					// LDX zp,Y
            THIS.X = systemRead(a.w);
            THIS.Z = (THIS.X) ? 0 : 1;
            THIS.N = THIS.X >> 7;
            break;

        case 0xA0:					// LDY imm
        case 0xAC:					// LDY abs
        case 0xA4:					// LDY zp
        case 0xBC:					// LDY abs,X
        case 0xB4:					// LDY zp,X
            THIS.Y = systemRead(a.w);
            THIS.Z = (THIS.Y) ? 0 : 1;
            THIS.N = THIS.Y >> 7;
            break;

        case 0x4A:					// LSR A
            THIS.C = THIS.A & 1;
            THIS.A >>= 1;
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = 0;
            break;
        case 0x4E:					// LSR abs
        case 0x46:					// LSR zp
        case 0x56:					// LSR zp,X
        case 0x5E:					// LSR abs,X
            n = systemRead(a.w);
            THIS.C = n & 1;
            n >>= 1;
            THIS.Z = (n) ? 0 : 1;
            THIS.N = 0;
            systemWrite(a.w, n);
            break;

        case 0xEA:					// NOP
            break;

        case 0x09:					// ORA imm
        case 0x0D:					// ORA abs
        case 0x05:					// ORA zp
        case 0x01:					// ORA (,X)
        case 0x11:					// ORA (),Y
        case 0x15:					// ORA zp,X
        case 0x1D:					// ORA abs,X
        case 0x19:					// ORA abs,Y
            THIS.A |= systemRead(a.w);
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        case 0x48:					// PHA
            _pushR(THIS.A);
            break;

        case 0x08:					// PHP
            _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | 0x10 | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
            // _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.B << 4) | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
            break;

        case 0x68:					// PLA
            THIS.A = _popR();
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        case 0x28:					// PLP
            n = _popR();
            THIS.N = n >> 7;
            THIS.V = (n >> 6) & 1;
            // THIS.B = (n >> 4) & 1;
            THIS.D = (n >> 3) & 1;
            THIS.I = (n >> 2) & 1;
            THIS.Z = (n >> 1) & 1;
            THIS.C = n & 1;
            break;

        case 0x2A:					// ROL A
            c = THIS.A >> 7;
            THIS.A = (THIS.A << 1) | THIS.C;
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            THIS.C = c;
            break;
        case 0x2E:					// ROL abs
        case 0x26:					// ROL zp
        case 0x36:					// ROL zp,X
        case 0x3E:					// ROL abs,X
            n = systemRead(a.w);
            c = n >> 7;
            n = (n << 1) | THIS.C;
            THIS.Z = (n) ? 0 : 1;
            THIS.N = n >> 7;
            THIS.C = c;
            systemWrite(a.w, n);
            break;

        case 0x6A:					// ROR A
            THIS.N = THIS.C;
            THIS.C = THIS.A & 1;
            THIS.A = (THIS.A >> 1) | (THIS.N << 7);
            THIS.Z = (THIS.A) ? 0 : 1;
            break;
        case 0x6E:					// ROR abs
        case 0x66:					// ROR zp
        case 0x76:					// ROR zp,X
        case 0x7E:					// ROR abs,X
            n = systemRead(a.w);
            THIS.N = THIS.C;
            THIS.C = n & 1;
            n = (n >> 1) | (THIS.N << 7);
            THIS.Z = (n) ? 0 : 1;
            systemWrite(a.w, n);
            break;

        case 0x40:					// RTI
            n = _popR();
            THIS.N = n >> 7;
            THIS.V = (n >> 6) & 1;
            // THIS.B = (n >> 4) & 1;
            THIS.D = (n >> 3) & 1;
            THIS.I = (n >> 2) & 1;
            THIS.Z = (n >> 1) & 1;
            THIS.C = n & 1;
            THIS.PC.b[0] = _popR();
            THIS.PC.b[1] = _popR();
            break;

        case 0x60:					// RTS
            THIS.PC.b[0] = _popR();
            THIS.PC.b[1] = _popR();
            THIS.PC.w++;
            break;

        case 0xE9:					// SBC imm
        case 0xED:					// SBC abs
        case 0xE5:					// SBC zp
        case 0xE1:					// SBC (,X)
        case 0xF1:					// SBC (),Y
        case 0xF5:					// SBC zp,X
        case 0xFD:					// SBC abs,X
        case 0xF9:					// SBC abs,Y
            _subAcc(systemRead(a.w));
            break;

        case 0x38:					// SEC
            THIS.C = 1;
            break;

        case 0xF8:					// SED
            THIS.D = 1;
            break;

        case 0x78:					// SEI
            THIS.I = 1;
            break;

        case 0x8D:					// STA abs
        case 0x85:					// STA zp
        case 0x81:					// STA (,X)
        case 0x91:					// STA (),Y
        case 0x95:					// STA zp,X
        case 0x9D:					// STA abs,X
        case 0x99:					// STA abs,Y
            systemWrite(a.w, THIS.A);
            break;

        case 0x8E:					// STX abs
        case 0x86:					// STX zp
        case 0x96:					// STX zp,Y
            systemWrite(a.w, THIS.X);
            break;

        case 0x8C:					// STY abs
        case 0x84:					// STY zp
        case 0x94:					// STY zp,X
            systemWrite(a.w, THIS.Y);
            break;

        case 0xAA:					// TAX
            THIS.X = THIS.A;
            THIS.Z = (THIS.X) ? 0 : 1;
            THIS.N = THIS.X >> 7;
            break;
        case 0xA8:					// TAY
            THIS.Y = THIS.A;
            THIS.Z = (THIS.Y) ? 0 : 1;
            THIS.N = THIS.Y >> 7;
            break;
        case 0xBA:					// TSX
            THIS.X = THIS.S.b[0];
            THIS.Z = (THIS.X) ? 0 : 1;
            THIS.N = THIS.X >> 7;
            break;
        case 0x8A:					// TXA
            THIS.A = THIS.X;
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;
        case 0x9A:					// TXS
            THIS.S.b[0] = THIS.X;
            break;
        case 0x98:					// TYA
            THIS.A = THIS.Y;
            THIS.Z = (THIS.A) ? 0 : 1;
            THIS.N = THIS.A >> 7;
            break;

        default:
            Serial.printf("Illegal opcode\n%s\n", _desass(THIS.PC.w-1));
            //__android_log_print(ANDROID_LOG_ERROR, "jni", "%04X : %02X unknown opcode", _pc-1, __op);
            //__desass(_pc-1);
            break;
    }
}

int m6502DoOps(SDWORD cycles) {
    int opscount = 0;
    CHAR *desass_t;
    THIS.cycles += cycles;
    if (THIS.NMI) {
        THIS.NMI = 0;
        _pushR(THIS.PC.b[1]);
        _pushR(THIS.PC.b[0]);
        // _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.B << 4) | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
        _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
        THIS.PC.w = (systemRead(0xFFFB) << 8) | systemRead(0xFFFA);
        THIS.I = 1;
        THIS.cycles -= 7;
        System.Psg.cyclesA += 7;
    } else if (THIS.RESET) {
        THIS.RESET = 0;
        THIS.PC.w = (systemRead(0xFFFD) << 8) | systemRead(0xFFFC);
        THIS.I = 1;
        //THIS.cycles -= 7;
        System.Psg.cyclesA += 7;
    }
    while (THIS.cycles > 0) {
        THIS.dcycles = 0;
        if (THIS.IRQ) {			// one is demanded
            if (THIS.I == 0) {		// can interrupt
                // console.log("irq seen");
                // THIS.desassOk = 1;
                System.Io.irqVia = 0;
                _pushR(THIS.PC.b[1]);
                _pushR(THIS.PC.b[0]);
                // _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.B << 4) | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
                _pushR((THIS.N << 7) | (THIS.V << 6) | 0x20 | (THIS.D << 3) | (THIS.I << 2) | (THIS.Z << 1) | THIS.C);
                THIS.PC.w = (systemRead(0xFFFF) << 8) | systemRead(0xFFFE);
                THIS.I = 1;
                THIS.dcycles += 7;
            }
            opscount++;
        }
        if (THIS.desassOk) {
            desass_t = _desass(THIS.PC.w);
        }
        // if (THIS.PC.w == 0xF8AC) THIS.desassOk = 1;
        _doOpOne(systemRead(THIS.PC.w++));
        opscount++;
        if (THIS.desassOk) {
            log_d("%s", desass_t);
            // console.log(__desass_t+''.padStart(40-__desass_t.length, ' ') +
            // ' A:' + THIS.A.toString(16).padStart(2,'0') +
            // ' X:' + THIS.X.toString(16).padStart(2,'0') +
            // ' Y:' + THIS.Y.toString(16).padStart(2,'0') +
            // ' S:' + THIS.S.toString(16).padStart(4,'0'));
            THIS.desassCounter++;
            if (THIS.desassCounter == 3000) {
                THIS.desassOk = 0;
            }
        }
        ioDoTimer(THIS.dcycles);
        THIS.cycles -= THIS.dcycles;
        System.Psg.cyclesA += THIS.dcycles;
    }
    if (System.Psg.audioEnable == 1) ym2149DoAudio();
    return opscount;
}
