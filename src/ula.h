
//
//   header file for ula  chipsets 
//

#include "ula_d.h"

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
         0b111111};             // blanc
static BYTE __colorsR[] = {
         0b111111,              // blanc
         0b001111,				// cyan
         0b110011,				// magenta
         0b000011,				// bleu
         0b111100,				// jaune
         0b001100,				// vert
         0b110000,				// rouge
         0b000000};             // noir

extern uint8_t _frameBuffer[];

void ula_init() {
    for (int i=0; i < 8; i++) {
        __colorsL[i] |= (BYTE)(Video.SBits & 0x0FF);
        __colorsR[i] |= (BYTE)(Video.SBits & 0x0FF);
    }
}

void ula_display2(BYTE tick) {
    BYTE *ram = System.ram;
    BYTE *crt = (BYTE *)(_frameBuffer);

    BYTE line=200;
    BYTE col=0;
    BYTE paper, ink=0;
    BYTE flash, dbl, txt, alt=0;
    BYTE lc=0, lcf=0;
    BYTE data=0;
    BYTE pixels=0;
    DWORD bpaper=0;
    DWORD bink=0;
    WORD addrC, addrCh=0;
    WORD adrT = 0xBB80;
    WORD adrH = 0xA000;
    DWORD c1 = 0;
    
    DWORD ix = ((VRES/VDIV-224)/2)*HRES+(HRES-240)/2; // 8*320+40;
    
    txt = THIS.text;
    addrC = (txt) ? 0xB400 : 0x9800;

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
