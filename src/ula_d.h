
#ifndef _C_ula_h
#define _C_ula_h

#include "types.h"

typedef struct {
    BYTE text;
    BYTE flash;
} ULA;

void ula_init();
void ula_display2(BYTE tick);
#endif