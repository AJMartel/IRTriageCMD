/*
 *  VER.C - ver internal command.
 *
 *
 *  History:
 *
 *    06/30/98 (Rob Lake)
 *        rewrote ver command to accept switches, now ver alone prints
 *        copyright notice only.
 *
 *    27-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 *        added config.h include
 *
 *    30-Jul-1998 (John P Price <linux-guru@gcfl.net>)
 *        added text about where to send bug reports and get updates.
 *
 *    20-Jan-1999 (Eric Kohl)
 *        Unicode and redirection safe!
 *
 *    26-Feb-1999 (Eric Kohl)
 *        New version info and some output changes.
 */

#include <precomp.h>


VOID ShortVersion (VOID)
{
	OSVERSIONINFO VersionInfo;
//	ConOutResPrintf(STRING_CMD_SHELLINFO, _T(KERNEL_RELEASE_STR), _T(KERNEL_VERSION_BUILD_STR));
	VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	memset(VersionInfo.szCSDVersion, 0, sizeof(VersionInfo.szCSDVersion));
	if (GetVersionEx(&VersionInfo))
	{
		ConOutPrintf("Version %d.%d, build %d, platform %d, %s\r\n", 
				VersionInfo.dwMajorVersion,VersionInfo.dwMinorVersion,
				VersionInfo.dwBuildNumber,
				VersionInfo.dwPlatformId,
				VersionInfo.szCSDVersion
		);
	}
}


#ifdef INCLUDE_CMD_VER

/*
 *  display shell version info internal command.
 *
 *
 */
INT cmd_ver (LPTSTR param)
{
	INT i;

  nErrorLevel = 0;


    if (_tcsstr (param, _T("/?")) != NULL)
	{
		ConOutResPaging(TRUE,STRING_VERSION_HELP1);
		return 0;
	}

	ShortVersion();

	/* Basic copyright notice */
	if (param[0] != _T('\0'))
	{
		ConOutPuts(_T("pCmd Script Executor"));
		ConOutPuts (_T("Copyright (C) 1994-1998 Tim Norman and others."));
		ConOutPuts (_T("Copyright (C) 1998-2011") _T(" ReactOS Team")); 
		ConOutPuts (_T("Copyright (C) 2011-2012") _T(" AtBas.org")); 

		for (i = 0; param[i]; i++)
		{
			if (param[i] == _T(' '))
				continue;

			if (param[i] == _T('/'))
			{
				if (param[i + 1] == 0)
				{
					error_invalid_switch (_T(' '));
					return 1;
				}
				continue;
			}

			if (_totupper (param[i]) == _T('W'))
			{
				ConOutResPuts(STRING_VERSION_HELP3);
			}
			else if (_totupper (param[i]) == _T('R'))
			{
				ConOutResPuts(STRING_VERSION_HELP4);
			}
			else if (_totupper (param[i]) == _T('C'))
			{
				ConOutResPuts(STRING_VERSION_HELP6);
				ConOutResPuts(STRING_FREEDOS_DEV);
				ConOutResPuts(STRING_VERSION_HELP7);
                ConOutResPuts(STRING_REACTOS_DEV);
                ConOutPuts("\npCmd written by Hans Harder\nUpdates are available at : http://www.atbas.org\n");
			}
			else
			{
				error_invalid_switch ((TCHAR)_totupper (param[i]));
				return 1;
			}
		}
		ConOutResPuts(STRING_VERSION_HELP5);
	}
	return 0;
}

#endif
