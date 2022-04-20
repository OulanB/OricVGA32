
#ifndef _C_ula_h
#define _C_ula_h

#include "types.h"

typedef struct {
    BYTE text;
    BYTE flash;
} ULA;

void ulaInit();
void ulaDisplay2(BYTE tick);
void osdDisplay2(BYTE tick);
void osdTopDisplay2(BOOL erase);
void osdBottomDisplay2(BOOL erase);
#endif