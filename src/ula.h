
//
//   header file for ula  chipsets 
//

#include "system_d.h"

#undef THIS 
#define THIS System.Ula

static  BYTE __colorsL[] = {
         0b000000,              // noir
         0b110000,				// rouge
         0b001100,				// vert
         0b111100,				// jaune
         0b000011,				// bleu
         0b110011,				// magenta
         0b001111,				// cyan
         0b111111,              // blanc
         0b000000,              // noir
         0b100000,				// rouge
         0b001000,				// vert
         0b101000,				// jaune
         0b000010,				// bleu
         0b100010,				// magenta
         0b001010,				// cyan
         0b101010};             // blanc
static BYTE __colorsR[] = {
         0b111111,              // blanc
         0b001111,				// cyan
         0b110011,				// magenta
         0b000011,				// bleu
         0b111100,				// jaune
         0b001100,				// vert
         0b110000,				// rouge
         0b000000,              // noir
         0b010101,              // blanc
         0b000101,				// cyan
         0b010001,				// magenta
         0b000001,				// bleu
         0b010100,				// jaune
         0b000100,				// vert
         0b010000,				// rouge
         0b000000};             // noir

extern uint8_t _frameBuffer[];

void ulaInit() {
    for (int i=0; i < 16; i++) {
        __colorsL[i] |= (BYTE)(Video.SBits & 0x0FF);
        __colorsR[i] |= (BYTE)(Video.SBits & 0x0FF);
    }
}

void _osdLineDisplay2(BYTE *ram, int ix) {
    BYTE *crt = (BYTE *)(_frameBuffer);

    BYTE line=8;
    BYTE col=0;
    BYTE paper, ink=0;
    BYTE data=0;
    BYTE pixels=0;
    BYTE bpaper=0;
    BYTE bink=0;
    int adrT = 0x0;
    
    for (; line; line--) {
        paper = 8 + (ram[adrT] & 0x07);			// paper at start
        ink = 8 + ((ram[adrT] >> 4) & 0x07);		// ink at start
        for (col = 1; col < 41; col++) {
            data = ram[adrT+col];
            if ((data & 0x60) == 0) {		// control
                if (data & 0x08) {
                } else {
                    if (data & 0x10) {			// paper
                        paper = 8 + (data & 0x07);
                    } else { 					// ink
                        ink = 8 + (data & 0x07);
                    }
                }
                bpaper = (data & 0x80) ? __colorsR[paper] : __colorsL[paper];
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                continue;						// next char
            } else {						// data
                BYTE d = data & 0x7F; 
                if (d < 32) d = 32;
                pixels = charsetData[((d-32) << 3) + (8 - line)];
            }
            if (data & 0x80) {
                bpaper = __colorsR[paper];
                bink = __colorsR[ink];
            } else {
                bpaper = __colorsL[paper];
                bink = __colorsL[ink];
            }
            crt[ix++^2] = (pixels & 0x20) ? bink : bpaper;                       // 0 -> 2       6  0 ->  4  -2  
            crt[ix++^2] = (pixels & 0x10) ? bink : bpaper;                       // 1 -> 3       7  1 ->  5  -1
            crt[ix++^2] = (pixels & 0x08) ? bink : bpaper;                       // 2 -> 0       8  2 -> 10   4 
            crt[ix++^2] = (pixels & 0x04) ? bink : bpaper;                       // 3 -> 1       9  3 -> 11   5
            crt[ix++^2] = (pixels & 0x02) ? bink : bpaper;                       // 4 -> 6      10  4 ->  8   2
            crt[ix++^2] = (pixels & 0x01) ? bink : bpaper;                       // 5 -> 7      11  5 ->  9   3
        }
        ix += (HRES-240);// 16; // 80;
    }
}

void osdTopDisplay2(BOOL erase) {
    BYTE *ram = erase ? System.Osd.noTopBottom : System.Osd.top;
    int ix = (HRES-240)/2;
    _osdLineDisplay2(ram, ix);
}
void osdBottomDisplay2(BOOL erase) {
    BYTE *ram = erase ? System.Osd.noTopBottom : System.Osd.bottom;
    int ix = (HRES-240)/2 + (224+8)*HRES;
    _osdLineDisplay2(ram, ix);
}

void ulaDisplay2(BYTE tick) {
    BYTE *ram = System.ram;
    BYTE *crt = (BYTE *)(_frameBuffer);

    BYTE line=200;
    BYTE col=0;
    BYTE paper, ink=0;
    BYTE flash, dbl, txt, alt=0;
    BYTE lc=0, lcf=0;
    BYTE data=0;
    BYTE pixels=0;
    BYTE bpaper=0;
    BYTE bink=0;
    int addrC, addrCh=0;
    int adrT = 0xBB80;
    int adrH = 0xA000;
    DWORD c1 = 0;
    
    int ix = ((VRES/VDIV-224)/2)*HRES+(HRES-240)/2; // 8*320+40;
    
    txt = THIS.text;

    for (; line; line--, lc++) {
        if (lc & 0x08) {
            adrT += 40;							// inc adr text
            lcf = 4 - lcf;
            lc = 0;
        }
        paper = 0;							// paper at start
        ink = 7;							// ink at start
        dbl = 0;							// dbl mode at start
        alt = 0;							// alternate charset at start
        flash = 0;							// flash mode at start
        addrC = (txt) ? 0xB400 : 0x9800;
        for (col = 0; col < 40; col++, adrH++) {
            if (txt) {
                data = ram[adrT+col];
                if ((data & 0x60) == 0) {		// control
                    if (data & 0x08) {
                        if (data & 0x10) {          // mode
                            txt = 4 - (data & 0x04);
                        } else {                    // attributes
                            alt = data & 0x01;
                            dbl = data & 0x02;
                            flash = (data & 0x04) ? tick : 0;
                        }
                        if (txt) {	// text mode
                            addrC = (alt) ? 0xB800 : 0xB400;
                        } else {	// hires mode, text part
                            addrC = (alt) ? 0x9C00 : 0x9800;
                        }
                    } else {
                        if (data & 0x10) {			// paper
                            paper = (data & 0x07);
                        } else { 					// ink
                            ink = (data & 0x07);
                        }
                    }
                    bpaper = (data & 0x80) ? __colorsR[paper] : __colorsL[paper];
                    crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                    crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                    continue;						// next char
                } else {						// data
                    addrCh = addrC + ((data & 0x7F) << 3) + ((dbl) ? (lc >> 1) + lcf : lc);
                    pixels = ram[addrCh];
                }
            } else {	// hires part
                data = pixels = ram[adrH];
                if ((data & 0x60) == 0) {		// control
                    if (data & 0x08) {
                        if (data & 0x10) {	        // modes
                            txt = 4 - (data & 0x04);
                        } else {					// attributes
                            // alt = ((data & 0x01) == 0) ? 0 : 1;
                            // dbl = ((data & 0x02) == 0) ? 0 : 1;
                            flash = (data & 0x04) ? tick : 0;
                        }
                        if (txt) {	// text mode
                            addrC = (alt) ? 0xB800 : 0xB400;
                        } else {	// hires mode, text part
                            addrC = (alt) ? 0x9C00 : 0x9800;
                        }
                    } else {
                        if (data & 0x10) {			// paper
                            paper = data & 0x07;
                        } else { 					// ink
                            ink = data & 0x07;
                        }
                    }
                    bpaper = (data & 0x80) ? __colorsR[paper] : __colorsL[paper];
                    crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                    crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; 
                    continue;					// next char
                }
            }
            if (data & 0x80) {
                bpaper = __colorsR[paper];
                bink = __colorsR[ink];
            } else {
                bpaper = __colorsL[paper];
                bink = __colorsL[ink];
            }
            if (flash) bink = bpaper;
            c1 = (pixels & 0x20) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 0 -> 2       6  0 ->  4  -2  
            c1 = (pixels & 0x10) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 1 -> 3       7  1 ->  5  -1
            c1 = (pixels & 0x08) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 2 -> 0       8  2 -> 10   4 
            c1 = (pixels & 0x04) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 3 -> 1       9  3 -> 11   5
            c1 = (pixels & 0x02) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 4 -> 6      10  4 ->  8   2
            c1 = (pixels & 0x01) ? bink : bpaper;
            crt[ix++^2] = c1;                       // 5 -> 7      11  5 ->  9   3
        }
        ix += (HRES-240);// 16; // 80;
    }
    for (line = 24; line; line--, lc++) {
        if (lc & 0x08) {
            adrT += 40;							// inc adr text
            lcf = 4 - lcf;
            lc = 0;
        }
        paper = 0;							// paper at start
        ink = 7;							// ink at start
        dbl = 0;							// dbl mode at start
        alt = 0;							// alternate charset at start
        flash = 0;							// flash mode at start
        addrC = (txt) ? 0xB400 : 0x9800;
        for (col = 0; col < 40; col++) {
            data = ram[adrT+col];
            if ((data & 0x60) == 0) {		// control
                if (data & 0x08) {
                    if (data & 0x10) {          // mode
                        txt = 4 - (data & 0x04);
                    } else {                    // attributes
                        alt = data & 0x01;
                        dbl = data & 0x02;
                        flash = (data & 0x04) ? tick : 0;
                    }
                    if (txt) {	// text mode
                        addrC = (alt) ? 0xB800 : 0xB400;
                    } else {	// hires mode, text part
                        addrC = (alt) ? 0x9C00 : 0x9800;
                    }
                } else {
                    if (data & 0x10) {			// paper
                        paper = data & 0x07;
                    } else { 					// ink
                        ink = data & 0x07;
                    }
                }
                bpaper = (data & 0x80) ? __colorsR[paper] : __colorsL[paper];
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                continue;					// next char
            }
            addrCh = addrC + ((data & 0x7F) << 3) + ((dbl) ? (lc >> 1) + lcf : lc);
            pixels = ram[addrCh];
            if (data & 0x80) {
                bpaper = __colorsR[paper];
                bink = __colorsR[ink];
            } else {
                bpaper = __colorsL[paper];
                bink = __colorsL[ink];
            }
            if (flash) bink = bpaper;
            c1 = (pixels & 0x20) ? bink : bpaper;
            crt[ix++^2] = c1;
            c1 = (pixels & 0x10) ? bink : bpaper;
            crt[ix++^2] = c1;
            c1 = (pixels & 0x08) ? bink : bpaper;
            crt[ix++^2] = c1;
            c1 = (pixels & 0x04) ? bink : bpaper;
            crt[ix++^2] = c1;
            c1 = (pixels & 0x02) ? bink : bpaper;
            crt[ix++^2] = c1;
            c1 = (pixels & 0x01) ? bink : bpaper;
            crt[ix++^2] = c1;
        }
        ix += (HRES-240); // 16; // 80;
    }
    THIS.text = txt;
}

void osdDisplay2(BYTE tick) {
    BYTE *ram = System.Osd.screen;
    BYTE *crt = (BYTE *)(_frameBuffer);

    BYTE line=224;
    BYTE col=0;
    BYTE paper, ink=0;
    BYTE flash, dbl;
    BYTE lc=0, lcf=0;
    BYTE data=0;
    BYTE pixels=0;
    BYTE bpaper=0;
    BYTE bink=0;
    int adrT = 0x0;

    int ix = ((VRES/VDIV-224)/2)*HRES+(HRES-240)/2; // 8*320+40;
    
    for (; line; line--, lc++) {
        if (lc & 0x08) {
            adrT += 41;							// inc adr text
            lcf = 4 - lcf;
            lc = 0;
        }
        paper = ram[adrT] & 0x07;			// paper at start
        ink = (ram[adrT] >> 4) & 0x07;		// ink at start
        dbl = 0;							// no dbl mode at start
        flash = 0;							// no flash mode at start
        for (col = 1; col < 41; col++) {
            data = ram[adrT+col];
            if ((data & 0x60) == 0) {		// control
                if (data & 0x08) {
                    dbl = data & 0x02;
                    flash = (data & 0x04) ? tick : 0;
                } else {
                    if (data & 0x10) {			// paper
                        paper = (data & 0x07);
                    } else { 					// ink
                        ink = (data & 0x07);
                    }
                }
                bpaper = (data & 0x80) ? __colorsR[paper] : __colorsL[paper];
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                crt[ix++^2] = bpaper; crt[ix++^2] = bpaper;
                continue;						// next char
            } else {						// data
                data &= 0x7F; 
                if (data < 32) data = 32;
                pixels = charsetData[((data-32) << 3) + ((dbl) ? (lc >> 1) + lcf : lc)];
            }
            if (data & 0x80) {
                bpaper = __colorsR[paper];
                bink = __colorsR[ink];
            } else {
                bpaper = __colorsL[paper];
                bink = __colorsL[ink];
            }
            if (flash) bink = bpaper;
            crt[ix++^2] = (pixels & 0x20) ? bink : bpaper;                       // 0 -> 2       6  0 ->  4  -2  
            crt[ix++^2] = (pixels & 0x10) ? bink : bpaper;                       // 1 -> 3       7  1 ->  5  -1
            crt[ix++^2] = (pixels & 0x08) ? bink : bpaper;                       // 2 -> 0       8  2 -> 10   4 
            crt[ix++^2] = (pixels & 0x04) ? bink : bpaper;                       // 3 -> 1       9  3 -> 11   5
            crt[ix++^2] = (pixels & 0x02) ? bink : bpaper;                       // 4 -> 6      10  4 ->  8   2
            crt[ix++^2] = (pixels & 0x01) ? bink : bpaper;                       // 5 -> 7      11  5 ->  9   3
        }
        ix += (HRES-240);// 16; // 80;
    }
}