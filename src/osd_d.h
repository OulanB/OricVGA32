/*
 * on screen display
 */

#ifndef _C_osd_h
#define _C_osd_h

#include "types.h"

#define OSD_SDFAT 1
#define OSD_SPIFAT 2

typedef struct OSDFILE {
    char name[256];
    BOOL dir;
    int size;
} OSDFILE;

typedef struct OSD {
    BYTE ink;
    BYTE paper;
    BYTE locX;
    BYTE locY;
    BOOL display;           // osd or ula display ?
    BOOL topDisplay;
    BOOL bottomDisplay;
    BYTE screen[28*41];     // for osd display  + one byte for paper/color at start of line

    BYTE *top;              // pointer to top-1 line
    BYTE *bottom;           // pointer to bottom+1 line
    BYTE noTopBottom[41+3]; // empty line to erase 

    BYTE bufScan[32];       // circular arrat of ascii keys
    int bufScanLow;         //   index low (to read)
    int bufScanHigh;        //   index high (to write)

    File fdRoot;            // fd for root entry in directory listing
    File fdIn;              // fd input
    File fdOut;             // fd output
    
    BYTE fileSys;           // SD or SPI ?
    BOOL mount;             // mounted
    char tapePath[256];
    char tmpName[256];  
    int  maxfiles;          // max number of files in a dir
    int  nfiles;            // number of files
    BOOL tooMuchFiles;      // too much files in a dir

    int  topFile;           // index of first displayed file
    int  selectFile;        // selected file 0..nfiles
    int  selectLine;        // OSD_YTOP .. OSD_YBOTTOM
    int  i;                 // index
    int  j;                 // index
    int  y;                 // index
    OSDFILE *files;         // array of files

    char message1[42];
    char message2[42];
    char message3[42];
    char saveMsg1[42];
    char saveMsg2[42];
    char saveMsg3[42];
    char saveMsg4[42];
    WORD retYesState;
    WORD retNoState;

    BYTE tmpNameLen;
    BYTE tmpCounter;

    WORD prevState;
    WORD state;             // main state
//    WORD substate;          // sub state for key loop
//    WORD modestate;         // sub state for mode loop
    WORD mode;              // 0: tape; 1: disk; 2: CPU; 3: MEM
    BOOL refresh;           // refresh the mode display

} OSD;

void osdPutScancode(BYTE code);

void osdInit(char *tapeRoot);
void _osdCls(BYTE paper, BYTE ink);
void _ovlScroll();
void _osdPrint(const char *text);
void _osdPrintAt(BYTE x, BYTE y, const char *text);

void osdFsm();

#endif