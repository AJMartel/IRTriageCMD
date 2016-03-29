#pragma once

/*
 *   Add extra hooks for more plugins
 */
#define CMDHOOK_LOADFILE	1
#define CMDHOOK_COMMAND		2
#define CMDHOOK_EXECUTE		3

typedef PCHAR    (__cdecl *tCMDHOOK_LoadFile)(char *fn    , DWORD *siz);
typedef INT      (__cdecl *tCMDHOOK_Command )(LPTSTR First, LPTSTR Rest);
typedef INT      (__cdecl *tCMDHOOK_Execute )(LPTSTR First, LPTSTR Rest);

extern tCMDHOOK_LoadFile	pCustomLoadFile;
extern tCMDHOOK_Command		pCustomCommand;
extern tCMDHOOK_Execute		pCustomExecute;
