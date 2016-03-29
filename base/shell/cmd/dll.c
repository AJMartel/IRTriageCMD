/*
 *  DLL.C - dll internal command.
 *
 *  History:
 *
 *    2010/02/06 (Didier Stevens)
 *        started.
 *    2010/02/07: added MemoryLoadLibary
 *    2015/09/18: added /A
 *    2015/09/20: checking of address length
 *
 */

#include <precomp.h>
#include "MemoryModule.h"

#ifdef INCLUDE_CMD_DLL

HMEMORYMODULE CallMemoryLoadLibrary(LPTSTR szDLLFileName)
{
	HANDLE hFile;
	HMEMORYMODULE hmResult;
	LPTSTR errmsg;
	LARGE_INTEGER sLIFileSize;
	LPVOID lpBuffer;
	DWORD dwBytesRead;

	hFile = CreateFile(szDLLFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("CreateFile %s - %s"), szDLLFileName, errmsg);
		LocalFree(errmsg);
		return NULL;
	}

	if (FALSE == GetFileSizeEx(hFile, &sLIFileSize))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("GetFileSizeEx %s - %s"), szDLLFileName, errmsg);
		LocalFree(errmsg);
		CloseHandle(hFile);
		return NULL;
	}

	lpBuffer = HeapAlloc(GetProcessHeap(), 0, sLIFileSize.QuadPart);
	if (NULL == lpBuffer)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("HeapAlloc - %s"), errmsg);
		LocalFree(errmsg);
		CloseHandle(hFile);
		return NULL;
	}

	if (FALSE == ReadFile(hFile, lpBuffer, sLIFileSize.QuadPart, &dwBytesRead, NULL))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("ReadFile %s - %s"), szDLLFileName, errmsg);
		LocalFree(errmsg);
		HeapFree(GetProcessHeap(), 0, lpBuffer);
		CloseHandle(hFile);
		return NULL;
	}

	hmResult = MemoryLoadLibrary(lpBuffer);

	HeapFree(GetProcessHeap(), 0, lpBuffer);
	CloseHandle(hFile);

	return hmResult;
}

LPVOID ParseHexAddress(LPTSTR szAddress)
{
	unsigned int uiAddress = 0;
	unsigned int uiCount = 0;

	while (*szAddress != _T('\0'))
	{
		uiCount++;
		if (*szAddress >= _T('0') && *szAddress <= _T('9'))
		{
			uiAddress = uiAddress << 4;
			uiAddress = uiAddress + *szAddress - _T('0');
			szAddress++;
		}
		else if (tolower(*szAddress) >= _T('a') && tolower(*szAddress) <= _T('f'))
		{
			uiAddress = uiAddress << 4;
			uiAddress = uiAddress + tolower(*szAddress) - _T('a') + 10;
			szAddress++;
		}
		else
			return NULL;
		if (uiCount > 8)
			return NULL;
	}

	return (LPVOID)uiAddress;
}

INT cmd_dll(LPTSTR param)
{
	INT	argc;
	INT i;
	LPTSTR *argv;
	HANDLE hDLL;
	BOOL bMemory = FALSE;
	BOOL bAddress = FALSE;
	BOOL bKeep = FALSE;
	LPVOID lpAddress = NULL;

	if (!_tcsncmp(param, _T("/?"), 2))
	{
		ConOutResPaging(TRUE, STRING_DLL_HELP1);
		return 0;
	}

	if (!*param)
	{
		error_req_param_missing();
		return 1;
	}

	argv = split(param, &argc, TRUE);

	for (i = 0; i < argc; i++)
	{
		if ('/' == argv[i][0])
		{
			if ('\0' != argv[i][2])
				ConErrResPrintf(STRING_DLL_ERROR1, argv[i] + 1);
			else
				switch(argv[i][1])
				{
					case 'M':
					case 'm':
						bMemory = TRUE;
						break;
					case 'A':
					case 'a':
						bAddress = TRUE;
						break;
					case 'K':
					case 'k':
						bKeep = TRUE;
						break;
					default:
						ConErrResPrintf(STRING_DLL_ERROR1, argv[i] + 1);
				}
		}
		else
		{
			if (bMemory)
			{
				hDLL = CallMemoryLoadLibrary(argv[i]);
				ConOutPrintf(_T("0x%08x %s\n"), hDLL, argv[i]);
			}
			else if (bAddress)
			{
				lpAddress = ParseHexAddress(argv[i]);
				if (NULL == lpAddress)
					ConOutPrintf(_T("Wrong address %s\n"), argv[i]);
				else
				{
					hDLL = MemoryLoadLibrary(lpAddress);
					ConOutPrintf(_T("0x%08x %s\n"), hDLL, argv[i]);
				}
			}
			else
			{
				hDLL = LoadLibrary(argv[i]);
				ConOutPrintf(_T("0x%08x %s\n"), hDLL, argv[i]);
				if (0 != hDLL && !bKeep)
					FreeLibrary(hDLL);
			}
		}
	}

	freep(argv);

	return 0;
}

#endif
