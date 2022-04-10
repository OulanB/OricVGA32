
#ifndef _C_tape_h
#define _C_tape_h

#include "types.h"

//
//   header file for tape 
//

typedef struct {
    CHAR typeS[16];			// type of part B C S I R
    CHAR name[256];		    // name of parts (oric side)
    DWORD start;			// header of part (oric side)
    DWORD end;            // end on data
    BYTE autom;           // ?
    BYTE type;           // ?
    BYTE array1;         // ?
    BYTE array2;         // ?
    BYTE H7;             // ?
    BYTE *ptr;            // ptr on data

    BYTE bitImage;		// is bit mode ?
} PART;

typedef struct {
	BYTE atmos;
    CHAR name[1024];								    // name of real tape file
    BYTE loaded;						            // is a real tape loaded ?
    BYTE length;				            // max length of a tape
    // BYTE data[512*1024];
    SDWORD size;								    // size of current tape
} TAPE;

#endif
