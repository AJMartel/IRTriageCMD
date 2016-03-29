/*
 *  PRIVILEGE.C - privilege internal command.
 *
 *  History:
 *
 *    2015/11/27 (Didier Stevens)
 *        started.
 *
 */

#include <precomp.h>

#ifdef INCLUDE_CMD_PRIVILEGE

//Adjust token privileges to enable SE_BACKUP_NAME
BOOL CurrentProcessAdjustToken(void)
{
  HANDLE hToken;
  TOKEN_PRIVILEGES sTP;
  DWORD dwLastError;

  if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
  {
		if (!LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &sTP.Privileges[0].Luid))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		sTP.PrivilegeCount = 1;
		sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		dwLastError = GetLastError();
		CloseHandle(hToken);
		return dwLastError == ERROR_SUCCESS;
  }
	return FALSE;
}

INT cmd_privilege(LPTSTR param)
{
	if (!_tcsncmp(param, _T("/?"), 2))
	{
		ConOutResPaging(TRUE, STRING_PRIVILEGE_HELP1);
		return 0;
	}

	if (*param)
	{
		error_req_param_missing();
		return 1;
	}

	if (CurrentProcessAdjustToken())
		ConOutPrintf(_T("Backup privilege enabled\n"));
	else
		ConOutPrintf(_T("Enabling backup privilege failed\n"));

	return 0;
}

#endif
