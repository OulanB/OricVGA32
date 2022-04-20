/*
 * stuff for tape operations ...
 *
 * (c)2013-2022 ods
 */

#ifndef _C_tape_h
#define _C_tape_h

#include "types.h"

//
//   header file for tape (i.e. /sd/Oric/tapes/Defense Force (19xx)(Tansoft).tap )
//

typedef struct {
    CHAR typeS[1];			// type of part B C S I R
    CHAR name[16+2];		// name of parts (oric side)
    DWORD start;			// header of part (oric side)
    DWORD end;              // end on data
    BYTE autost;            // auto start
    BYTE type;              // ?
    BYTE array1;            // for saved array
    BYTE array2;            //  "    "     "
    BYTE H7;                // ?
    int ptr;                // ptr on data
    int last;               // last good byte
    BOOL bitImage;		    // is bit mode ?
} PART;

typedef struct {
    CHAR fname[256];	        // full name of real tape file
    CHAR path[256];             // path name ...
    CHAR name[256];             // last part of name
    BOOL loaded;				// is a real tape loaded ?
    int length;				    // max length of a tape 512K ?
    BYTE *data;                 // pointer to real tape data
    int size;					// size of current tape
    BOOL bitTape;               // bit image tape (amnukor ...)
    BYTE bit[20];               // bits for read op
    int ptrBit;                 // index on a bit in a byte for a read
    int ptrBitWr;               // index on a bit in a byte for a write
    int ptr;                    // where is the head (0...length)

    BOOL readWanted;            // current op kind
    BOOL writeWanted;

    BYTE *wrData;               // buffer for writes ops, should be 64K max ?
    int wrPtr;                  // where is the head
    BYTE wrTick;                // ticks counting for write ops
    BYTE wrWaitZero;
    BYTE wrWaitData;
    BYTE wrByte;                // byte to write
    WORD wrRawByte;             // raw byte as 13 bits

    BYTE rawByte;

    BOOL quickRead;             // quick mode ? need patched rom
    BOOL quickWrite;            // quick mode ? need patched rom
    
    BOOL playing;               // is tape playing ?
    BOOL dataChanged;           // is data changed (to save at close)

    BYTE nparts;                // number of parts in the tape (with an OET)
    BYTE part;                  // current part 
    PART parts[32];             // max 32 parts

    char partRamName[16];       // name of part in ram
    SBYTE lastPercent;          // for stats
    BOOL selected;

    BYTE topLine[41+3];           // for status
    BYTE bottomLine[41+3];        // for status
} TAPE;

BOOL tapeLoadSD(const char *name);    // full file name
BOOL tapeLoadSPI(const char *name);    // full file name

BOOL tapeSaveSD(const char *path);
BOOL tapeSaveSPI(const char *path);

// void tapeStartPlay();
// void tapeStopPlay();

BYTE tapeQuickRead();
BYTE tapeQuickSync();
void tapeQuickWrite(BYTE d);

void tapeStartOperationRd();
void tapeStartOperationWr();
BYTE tapeGetFromTape();
void tapeInitOp();
void tapeEndOperation(BOOL read, BOOL write);
void tapeWriteBit(WORD data);
void tapeReset();

BOOL tapeDeleteFromTape();

void tapePrev();
void tapeNext();
void tapeBegin();
void tapeEnd();

void tapeEmptyTape();
void tapeInit();

void _tapeRajInfoTape();

#endif
