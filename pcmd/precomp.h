#ifndef __CMD_PRECOMP_H
#define __CMD_PRECOMP_H

#ifdef _MSC_VER
#pragma warning ( disable : 4103 ) /* use #pragma pack to change alignment */
#undef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif /*_MSC_VER */

#include <stdlib.h>
#include <malloc.h>

#define WIN32_NO_STATUS
#include <windows.h>
#include <winnt.h>
#include <shellapi.h>

#include <tchar.h>
#include <direct.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#define NTOS_MODE_USER
//#include <ndk/ntndk.h>

#include "resource.h"

#include "cmd.h"
#include "config.h"
#include "batch.h"
#ifdef CMD_CUSTOM_PLUGINS
#include "cmdhooks.h"
#endif

//#include <wine/debug.h>
//WINE_DEFAULT_DEBUG_CHANNEL(cmd);

#ifdef CMD_TRACE_DYNAMIC
void			cmd_trace(char *szFormat, ...);
#define TRACE	cmd_trace
#else
#define TRACE	while(0) {}
#endif

#ifdef UNICODE
#define debugstr_aw debugstr_w
#else
#define debugstr_aw (char *)
#endif

#endif /* __CMD_PRECOMP_H */
