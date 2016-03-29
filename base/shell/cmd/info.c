/*
 *  INFO.C - info internal command.
 *
 *  History:
 *
 *    2015/11/26 (Didier Stevens)
 *        started.
 *
 */

#include <precomp.h>

#ifdef INCLUDE_CMD_INFO

void PrintTime(FILETIME *pftIn)
{
	SYSTEMTIME st;
	_TCHAR szBuffer[256];

	FileTimeToSystemTime(pftIn, &st);
	_sntprintf(szBuffer, 256, TEXT("%04d/%02d/%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	ConOutPrintf(szBuffer);
}

INT cmd_info(LPTSTR param)
{
	HANDLE hFile; // hConsoleOut;
	INT    argc;
	LPTSTR *argv;
	LPTSTR errmsg;

	DWORD dwNeeded;
	PSECURITY_DESCRIPTOR psdSD = NULL;
	LPTSTR		  lpStrSecDesc = NULL;
	ULONG		  ulSecLen;
	DWORD dwFileAttributes;
	FILETIME ftCreation;
	FILETIME ftLastWrite;
	FILETIME ftLastAccess;

	//hConsoleOut=GetStdHandle(STD_OUTPUT_HANDLE);

	if (!_tcsncmp(param, _T("/?"), 2))
	{
		ConOutResPaging(TRUE, STRING_INFO_HELP1);
		return 0;
	}

	if (!*param)
	{
		error_req_param_missing();
		return 1;
	}

	argv = split(param, &argc, TRUE);

	if (1 != argc)
	{
		error_req_param_missing();
		freep(argv);
		return 1;
	}

	nErrorLevel = 0;

	ConOutPrintf(_T("File: %s\n\n"), argv[0]);

	hFile = CreateFile(argv[0], GENERIC_READ, FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("%s - %s"), argv[0], errmsg);
		LocalFree(errmsg);
		nErrorLevel = 1;
		return 1;
	}

	if (GetFileTime(hFile, &ftCreation, &ftLastAccess, &ftLastWrite))
	{
		ConOutPrintf(_T("Creation:    "));
		PrintTime(&ftCreation);
		ConOutPrintf(_T("\n"));
		ConOutPrintf(_T("Last write:  "));
		PrintTime(&ftLastWrite);
		ConOutPrintf(_T("\n"));
		ConOutPrintf(_T("Last access: "));
		PrintTime(&ftLastAccess);
		ConOutPrintf(_T("\n"));
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("GetFileTime: %s"), errmsg);
		LocalFree(errmsg);
		nErrorLevel = 1;
	}
			
	ConOutPrintf(_T("\n"));

	dwFileAttributes = GetFileAttributes(argv[0]);
	ConOutPrintf(_T("Attributes: 0x%02X "), dwFileAttributes);
	if (dwFileAttributes & 0x20)
		ConOutPrintf(_T("A"));
	if (dwFileAttributes & 0x4)
		ConOutPrintf(_T("S"));
	if (dwFileAttributes & 0x2)
		ConOutPrintf(_T("H"));
	if (dwFileAttributes & 0x1)
		ConOutPrintf(_T("R"));
	ConOutPrintf(_T("\n"));

	freep(argv);

	ConOutPrintf(_T("\n"));

	GetKernelObjectSecurity(hFile, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, NULL, 0, &dwNeeded);

	psdSD = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), 0, dwNeeded);
	if (NULL == psdSD)
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("HeapAlloc: %s"), errmsg);
		LocalFree(errmsg);
		nErrorLevel = 1;
		return 1;
	}

	if (!GetKernelObjectSecurity(hFile, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, psdSD, dwNeeded, &dwNeeded))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("GetKernelObjectSecurity: %s"), errmsg);
		LocalFree(errmsg);
		nErrorLevel = 1;
		return 1;
	}

	CloseHandle(hFile);

	if (!ConvertSecurityDescriptorToStringSecurityDescriptor(psdSD, SDDL_REVISION_1, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, &lpStrSecDesc, &ulSecLen))
	{
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errmsg, 0, NULL);
		ConErrPrintf(_T("ConvertSecurityDescriptorToStringSecurityDescriptor: %s"), errmsg);
		LocalFree(errmsg);
		nErrorLevel = 1;
		return 1;
	}

	ConOutPrintf(_T("Security descriptor:\n%s\n"), lpStrSecDesc);

	if (psdSD)
		HeapFree(GetProcessHeap(), 0, psdSD);
	if (lpStrSecDesc)
		LocalFree(lpStrSecDesc);

	return 0;
}

#endif
