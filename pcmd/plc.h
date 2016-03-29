#ifndef _PLC_H
#define _PLC_H


#define PLC_VERSION "3.02"
#pragma warning(disable: 4996)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Merge sections (that way data and const data can still be modified)
#pragma comment(linker,"/merge:.rdata=.data")

char *plc_fn(char *fn, DWORD *);

#endif
