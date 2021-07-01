#include <Windows.h>
#include <fltUser.h>
#include <strsafe.h>

#define WIN_MAJOR_MAX	    10    // Windows 10
#define WIN_MAJOR_MIN	    5    // Windows 2000 or XP

BOOL GetWinVer(DWORD& dwMajor, DWORD& dwMinor)
{
	DWORD dwTypeMask;
	DWORDLONG dwlConditionMask = 0;
	OSVERSIONINFOEX ovi;

	BOOL ret = FALSE;

	dwMajor = 0;
	dwMinor = 0;

	for (int mjr = WIN_MAJOR_MAX; mjr >= WIN_MAJOR_MIN; mjr--)
	{
		for (int mnr = 0; mnr <= 3; mnr++)
		{
			memset(&ovi, 0, sizeof(OSVERSIONINFOEX));

			ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			ovi.dwMajorVersion = mjr;
			ovi.dwMinorVersion = mnr;

			dwlConditionMask = 0;
			VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
			VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);

			dwTypeMask = VER_MAJORVERSION | VER_MINORVERSION;

			if (TRUE == VerifyVersionInfo(&ovi, dwTypeMask, dwlConditionMask))
			{
				dwMajor = mjr;
				dwMinor = mnr;

				ret = TRUE;
				break;
			}
		}
	}

	return ret;
}

HRESULT SendMessageDriver(MINI_COMMAND Cmd, PBYTE InByte, ULONG InSize, PBYTE OutBuf, ULONG OutSize, PULONG Result)
{
	HRESULT hResult = S_OK;
	HANDLE MiniPort = INVALID_HANDLE_VALUE;
	PCOMMAND_MESSAGE CmdMsg = NULL;
	PCHAR Buffer = NULL;
	DWORD ByteResult = 0, BufSize = 0;

	__try {
		hResult = FilterConnectCommunicationPort(MINI_PORT_NAME, 0, NULL, 0, NULL, &MiniPort);
		if (IS_ERROR(hResult)) {

			__leave;
		}

		if (OutBuf) {
			Buffer = (PCHAR)LocalAlloc(LMEM_FIXED, MAX_REG_VALUE_LEN);
			if (!Buffer) {
				hResult = HRESULT_FROM_WIN32(GetLastError());

				__leave;
			}
			else memset(Buffer, 0, MAX_REG_VALUE_LEN);

			BufSize = MAX_REG_VALUE_LEN;
		}


		CmdMsg = (PCOMMAND_MESSAGE)LocalAlloc(LMEM_FIXED, sizeof(COMMAND_MESSAGE) + InSize);
		if (!CmdMsg) __leave;
		else memset(CmdMsg, 0, sizeof(COMMAND_MESSAGE) + InSize);

		CmdMsg->Command = Cmd;
		if (InByte) memcpy(CmdMsg->Data, InByte, InSize);

		hResult = FilterSendMessage(MiniPort, CmdMsg, sizeof(COMMAND_MESSAGE) + InSize, Buffer, BufSize, &ByteResult);
		if (IS_ERROR(hResult)) __leave;

		if (OutBuf && ByteResult > 0) {
			ByteResult = min(OutSize, ByteResult);
			memcpy(OutBuf, Buffer, ByteResult);

			if (Result) *Result = ByteResult;
		}

	}
	__finally
	{
		if (CmdMsg) LocalFree(CmdMsg);

		if (Buffer) LocalFree(Buffer);

		if (MiniPort != INVALID_HANDLE_VALUE) CloseHandle(MiniPort);
	}

	return hResult;
}

BOOL SetStrBuffer(PSTR_BUF StrBuf)
{
	ULONG BufSize;

	if (!StrBuf) return FALSE;

	if (StrBuf->BufSize == 0) BufSize = BUF_UNIT;
	else if (StrBuf->BufSize < BUF_UNIT) BufSize = StrBuf->BufSize + BUF_UNIT;
	else BufSize = StrBuf->BufSize * 2;

	StrBuf->BufSize = 0;
	if (StrBuf->Buffer) free(StrBuf->Buffer);

	StrBuf->Buffer = (PCHAR)malloc(BufSize);
	if (!StrBuf->Buffer) return FALSE;

	StrBuf->BufSize = BufSize;
	memset(StrBuf->Buffer, 0, StrBuf->BufSize);

	return TRUE;
}

VOID FreeStrBuffer(PSTR_BUF StrBuf)
{
	if (!StrBuf) return;

	if (StrBuf->Buffer) {
		free(StrBuf->Buffer);
		StrBuf->Buffer = NULL;
	}
	StrBuf->BufSize = 0;
}

PCHAR FileTimeToStr(PFILETIME FileTime, PCHAR TimeStr, PCHAR TimeNumOnly)
{
	SYSTEMTIME SysTime;
	FILETIME LocalTime;

	FileTimeToLocalFileTime(FileTime, &LocalTime);
	FileTimeToSystemTime(&LocalTime, &SysTime);

	if (TimeStr) {
		MySNPrintf(TimeStr, 32, "\"%04u-%02u-%02u %02u:%02u:%02u\"", SysTime.wYear, SysTime.wMonth,
			SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	}

	if (TimeNumOnly) {
		MySNPrintf(TimeNumOnly, 32, "%04u%02u%02u%02u%02u%02u", SysTime.wYear, SysTime.wMonth,
			SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	}

	return TimeStr;
}

PCHAR MyStrNCat(PCHAR DestBuf, PCHAR SourceBuf, ULONG MaxLen)
{
	ULONG DestLen;

	if (!DestBuf || !SourceBuf) return "";

	DestLen = (ULONG)strlen(DestBuf);
	MyStrNCopy(DestBuf + DestLen, SourceBuf, MaxLen - DestLen);

	return DestBuf;
}

PWCHAR MyStrNCatW(PWCHAR DestBuf, PWCHAR SourceBuf, ULONG MaxLen)
{
	ULONG DestLen;

	if (!DestBuf || !SourceBuf) return L"";

	DestLen = (ULONG)wcslen(DestBuf);
	MyStrNCopyW(DestBuf + DestLen, SourceBuf, -1, MaxLen - DestLen);

	return DestBuf;
}

PCHAR Trim(PCHAR SourceStr)
{
	if (!SourceStr) return NULL;

	while (*SourceStr) {
		if (*SourceStr != '\n' && isspace((int)*SourceStr)) SourceStr++;
		else break;
	}

	return SourceStr;
}

PCHAR MyStrTok(PCHAR SourceStr, PCHAR SepChars, PCHAR* NextStr, BOOL UseQuota)
{
	PCHAR TargetChr = NULL, NextChr;
	BOOL DelQuota = FALSE;

	if (!UseQuota && strcmp(SepChars, SEP_NEWLN)) SourceStr = Trim(SourceStr);

	if (!SourceStr || (!UseQuota && !*SourceStr)) {
		if (NextStr) *NextStr = NULL;

		return NULL;
	}

	if (SepChars) {
		if (UseQuota && *SourceStr == CHR_QUOTA) {
			TargetChr = ++SourceStr;
			while (*TargetChr) {
				NextChr = TargetChr + 1;
				if (*TargetChr == CHR_QUOTA) {
					if (*NextChr == CHR_QUOTA) TargetChr++;
					else if (!*NextChr || *NextChr == *SepChars) {
						*TargetChr++ = 0;
						DelQuota = TRUE;
						break;
					}
				}

				TargetChr++;
			}
		}
		else {
			while (*SepChars) {
				if (TargetChr = strchr(SourceStr, *SepChars++)) break;
			}
		}

		if (TargetChr && *TargetChr) *TargetChr++ = 0;
		else TargetChr = NULL;

		if (UseQuota && DelQuota) QuotaDelete(SourceStr);
	}

	if (NextStr) *NextStr = TargetChr;

	return SourceStr;
}

PCHAR QuotaAdd(PCHAR SourceStr, PCHAR TargetStr)
{
	PCHAR SourceBuf = NULL, Source, Target;

	if (!SourceStr || !*SourceStr) return SourceStr;

	if (TargetStr) {
		Target = TargetStr;
		Source = SourceStr;
	}
	else {
		ULONG SourceLen = (ULONG)strlen(SourceStr) + 1;

		Target = TargetStr = SourceStr;
		SourceBuf = (PCHAR)malloc(SourceLen);
		if (SourceBuf) {
			MyStrNCopy(SourceBuf, SourceStr, SourceLen);
			Source = SourceBuf;
		}
		else return SourceStr;
	}

	*Target++ = CHR_QUOTA;
	for (; *Source; Source++, Target++) {
		if (*Source == CHR_QUOTA) *Target++ = *Source;

		*Target = *Source;
	}
	*Target++ = CHR_QUOTA;
	*Target = 0;

	if (SourceBuf) free(SourceBuf);

	return TargetStr;
}

PCHAR QuotaDelete(PCHAR SourceStr)
{
	PCHAR SourceBuf, SourceChr, TargetStr = SourceStr;
	ULONG SourceLen;

	if (!SourceStr || !*SourceStr) return SourceStr;

	SourceLen = (ULONG)strlen(SourceStr) + 1;
	SourceBuf = (PCHAR)malloc(SourceLen);
	if (SourceBuf) {
		MyStrNCopy(SourceBuf, SourceStr, SourceLen);
		for (SourceChr = SourceBuf; *SourceChr; SourceChr++, TargetStr++) {
			if (*SourceChr == CHR_QUOTA && *(SourceChr + 1) == CHR_QUOTA) SourceChr++;

			*TargetStr = *SourceChr;
		}
		*TargetStr = 0;

		free(SourceBuf);
	}

	return SourceStr;
}

ULONG MyStrNCopy(PCHAR DestBuf, CONST PCHAR SourceBuf, ULONG MaxLen)
{
	ULONG SourceLen;

	if (!DestBuf || !SourceBuf) return 0;

	SourceLen = (ULONG)strlen(SourceBuf);
	if (MaxLen <= SourceLen) SourceLen = MaxLen;

	StringCchCopyNA(DestBuf, MaxLen, SourceBuf, SourceLen);

	return SourceLen;
}


ULONG MyStrNCopyW(PWCHAR DestBuf, CONST PWCHAR SourceBuf, ULONG SourceLen, ULONG MaxLen)
{
	if (!DestBuf || !SourceBuf) return 0;

	if (SourceLen == -1) SourceLen = (ULONG)wcslen(SourceBuf);

	if (MaxLen <= SourceLen) SourceLen = MaxLen;

	StringCchCopyNW(DestBuf, MaxLen, SourceBuf, SourceLen);

	return SourceLen;
}

ULONG MySNPrintf(PCHAR DestBuf, ULONG MaxLen, CONST PCHAR FormatStr, ...)
{
	va_list ap;

	if (!DestBuf || !FormatStr) return 0;

	va_start(ap, FormatStr);
	StringCchVPrintfA(DestBuf, MaxLen, FormatStr, ap);
	va_end(ap);

	return (ULONG)strlen(DestBuf);
}

ULONG MySNPrintfW(PWCHAR DestBuf, ULONG MaxLen, CONST PWCHAR FormatStr, ...)
{
	va_list ArgList;

	if (!DestBuf || !FormatStr) return 0;

	va_start(ArgList, FormatStr);
	StringCchVPrintfW(DestBuf, MaxLen, FormatStr, ArgList);
	va_end(ArgList);

	return (ULONG)wcslen(DestBuf);
}

ULONG MyMemCopy(PVOID BufTo, PVOID BufFrom, ULONG BufSize)
{
	if (!BufTo || !BufFrom) return 0;

	memcpy(BufTo, BufFrom, BufSize);

	return BufSize;
}

ULONG CopyLineBuf(PSTR_BUF StrBuf, PULONG Offset, PCHAR LineBuf, ULONG Len)
{
	if (*Offset + Len >= StrBuf->BufSize - 1) return -1;
	else memcpy(StrBuf->Buffer + *Offset, LineBuf, Len + 1);

	*Offset += Len;

	return 0;
}

ULONGLONG GetObjKey(TYPE_OBJECT ObjType, PCHAR ObjPath)
{
	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION FileInfo;
	ULONGLONG ObjKey = 0;

	if (!ObjPath || !*ObjPath) return 0;

	if (ObjType == OBJECT_FILE) {
		hFile = CreateFileA(ObjPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			if (GetFileInformationByHandle(hFile, &FileInfo)) {
				ObjKey = ((ULONGLONG)FileInfo.nFileIndexHigh << 32) | (ULONGLONG)FileInfo.nFileIndexLow;
			}
			if (hFile) CloseHandle(hFile);
		}
	}

	return ObjKey;
}

BOOL GetUserSId(PCHAR UserName, PUSERSID UserSId)
{
	CHAR Domain[256] = { 0 };
	DWORD cbSId = sizeof(USERSID), cbDomain = sizeof(Domain);
	SID_NAME_USE peUse;
	BOOL bRet;

	if (!UserName || !*UserName || !UserSId) return FALSE;

	memset(UserSId, 0, sizeof(USERSID));
	bRet = LookupAccountNameA(NULL, UserName, UserSId, &cbSId, Domain, &cbDomain, &peUse);

	return bRet;
}

PCHAR GetLocalAccount(PCHAR UserName)
{
	LPUSER_INFO_20 pBuf = NULL, pTempBuf;
	LPUSER_INFO_24 pUserInfo = NULL;
	DWORD dwPreMaxLen = (DWORD)-1, dwEntriesRead = 0;
	DWORD dwTotalEntries = 0, dwResumeHandle = 0, i;
	NET_API_STATUS nStatus;
	WCHAR CompareName[MAX_KPATH];

	if (!UserName || !*UserName) return NULL;

	MultiByteToWideChar(CP_ACP, 0, UserName, -1, CompareName, MAX_KPATH);

	nStatus = NetUserEnum(NULL, 20, 0, (LPBYTE*)&pBuf, dwPreMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
		pTempBuf = pBuf;
		if (pTempBuf != NULL) {
			for (i = 0; i < dwEntriesRead; i++) {
				if (!pTempBuf) break;

				if ((pTempBuf->usri20_flags & UF_ACCOUNTDISABLE) != UF_ACCOUNTDISABLE) {
					if (NetUserGetInfo(NULL, pTempBuf->usri20_name, 24, (LPBYTE*)&pUserInfo) == NERR_Success) {
						if (pUserInfo->usri24_internet_identity) {
							if (!_wcsicmp(CompareName, pUserInfo->usri24_internet_principal_name)) {
								WideCharToMultiByte(CP_ACP, 0, pTempBuf->usri20_name, -1, UserName, MAX_KPATH, NULL, FALSE);
							}
						}
						NetApiBufferFree(pUserInfo);
					}
				}
				pTempBuf++;
			}
		}
	}

	if (pBuf != NULL) {
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}

	return UserName;
}

ULONG GetGroupList(PKEPARAM KEParam)
{
	PLOCALGROUP_INFO_0 pGroups = NULL, pTmpGroup;
	DWORD dwEntriesRead = 0, dwTotalEntries = 0, i;
	NET_API_STATUS nStatus;
	CHAR GroupName[MAX_KPATH];
	ULONG Len = 0, OffSet, ErrCode;

	do {
		Len = OffSet = 0;

		do {
			nStatus = NetLocalGroupEnum(NULL, 0, (LPBYTE*)&pGroups, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);
			if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
				pTmpGroup = pGroups;
				if (pTmpGroup != NULL) {
					for (i = 0; i < dwEntriesRead; i++) {
						if (!pTmpGroup) break;

						Len = WideCharToMultiByte(CP_ACP, 0, pTmpGroup->lgrpi0_name, -1, GroupName, MAX_KPATH, NULL, FALSE);

						if (OffSet > 0) CopyLineBuf(&KEParam->StrBuf, &OffSet, SEP_NEWLN, (ULONG)strlen(SEP_NEWLN));

						ErrCode = CopyLineBuf(&KEParam->StrBuf, &OffSet, GroupName, (Len - 1));
						if (ErrCode == -1) break;

						pTmpGroup++;
					}
				}
			}

			if (pGroups != NULL) {
				NetApiBufferFree(pGroups);
				pGroups = NULL;
			}
		} while (ErrCode != -1 && nStatus == ERROR_MORE_DATA);

	} while (ErrCode != -1);

	if (pGroups != NULL) NetApiBufferFree(pGroups);

	KEParam->StrBuf.BufSize = OffSet;

	return 0;
}

BOOL SetStrBuffer(PGET_STR GetStr)
{
	ULONG BufSize;

	if (!GetStr) return FALSE;

	if (GetStr->BufSize == 0) BufSize = BUF_UNIT;
	else if (GetStr->BufSize < BUF_UNIT) BufSize = GetStr->BufSize + BUF_UNIT;
	else BufSize = GetStr->BufSize * 2;

	GetStr->BufSize = 0;
	if (GetStr->Buffer) free(GetStr->Buffer);

	GetStr->Buffer = (PCHAR)malloc(BufSize);
	if (!GetStr->Buffer) return FALSE;

	GetStr->BufSize = BufSize;
	memset(GetStr->Buffer, 0, GetStr->BufSize);

	return TRUE;
}

VOID FreeStrBuffer(PGET_STR GetStr)
{
	if (!GetStr) return;

	if (GetStr->Buffer) {
		free(GetStr->Buffer);
		GetStr->Buffer = NULL;
	}
	GetStr->BufSize = 0;
}

NTSTATUS(WINAPI* _ZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG) = NULL;
NTSTATUS(WINAPI* _ZwQueryInformationProcess)(HANDLE, UINT, PVOID, ULONG, PULONG) = NULL;

HMODULE SetNAPIAddr()
{
	CHAR SysDllPath[MAX_KPATH];
	HMODULE hNtDll = LoadLibrary(GetSystemFilePath(SysDllPath, "ntdll.dll"));

	if (hNtDll) {
		*(FARPROC*)&_ZwQuerySystemInformation = GetProcAddress(hNtDll, "ZwQuerySystemInformation");
		if (!_ZwQuerySystemInformation) {
			pWrite("GetProcAddress ZwQuerySystemInformation Error %u", GetLastError());
			return FALSE;
		}

		*(FARPROC*)&_ZwQueryInformationProcess = GetProcAddress(hNtDll, "ZwQueryInformationProcess");
		if (!_ZwQueryInformationProcess)
			pWrite("GetProcAddress ZwQueryInformationProcess Error %u", GetLastError());
	}
	else pWrite("LoadLibrary Error %u", GetLastError());

	return hNtDll;
}

ULONG GetParentProcessId(ULONG ProcessId)
{
	ULONG ParentId = ProcessId;
	PROCESS_BASIC_INFORMATION ProcInfo = { 0 };
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

	if (!hProcess || !_ZwQueryInformationProcess) return ParentId;

	if (_ZwQueryInformationProcess(hProcess, ProcessBasicInformation, &ProcInfo,
		sizeof(PROCESS_BASIC_INFORMATION), NULL) == 0)
		ParentId = (ULONG)ProcInfo.InheritedFromUniqueProcessId;

	CloseHandle(hProcess);

	return ParentId;
}

ULONG FindProcessPath(PCHAR ProcPath, PULONG SessionId)
{
	return FindProcessPathEx(ProcPath, SessionId, NULL);
}

ULONG FindProcessPathEx(PCHAR ProcPath, PULONG SessionId, PCHAR CreateTime)
{
	HMODULE hNtDll = NULL;
	CHAR TmpPath[MAX_KPATH];
	PVOID ProcListBuf;
	PSYSTEM_PROCESSES pProcess;
	ULONG ProcessId = 0, TmpSessionId;

	if (!_ZwQuerySystemInformation) hNtDll = SetNAPIAddr();

	ProcListBuf = GetProcessList();
	if (!ProcListBuf) return 0;

	for (pProcess = (PSYSTEM_PROCESSES)ProcListBuf; pProcess;) {
		if (GetProcessPath((ULONG)pProcess->ProcessId, TmpPath, FALSE) &&
			!_stricmp(TmpPath, ProcPath)) {
			if (SessionId) ProcessIdToSessionId((ULONG)pProcess->ProcessId, &TmpSessionId);

			if (!SessionId || *SessionId == TmpSessionId) {
				ProcessId = (ULONG)pProcess->ProcessId;

				if (CreateTime) {
					FILETIME FileTime;

					FileTime.dwHighDateTime = pProcess->CreateTime.HighPart;
					FileTime.dwLowDateTime = pProcess->CreateTime.LowPart;
					FileTimeToStr(&FileTime, CreateTime, NULL);
				}
				break;
			}
		}

		if (pProcess->NextEntryDelta == 0) break;
		else pProcess = (PSYSTEM_PROCESSES)((LPBYTE)pProcess + pProcess->NextEntryDelta);
	} //for (pProcess = (PSYSTEM_PROCESSES)ProcListBuf; pProcess;) {

	free(ProcListBuf);
	if (hNtDll) FreeLibrary(hNtDll);

	return ProcessId;
}

VOID GetSessionUser(ULONG SessionId, PCHAR UserName)
{
	ULONG RetSize = 0;
	PCHAR pBuffer = NULL;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSUserName,
		&pBuffer, &RetSize) && pBuffer) {
		MyStrNCopy(UserName, pBuffer, MAX_USER_NAME);
		WTSFreeMemory(pBuffer);
	}
	else *UserName = 0;
}

VOID GetSessionIP(ULONG SessionId, PCHAR IPStr, PCHAR DefIP)
{
	ULONG RetSize = 0;
	PWTS_CLIENT_ADDRESS pClientIP = NULL;

	*IPStr = 0;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSClientAddress,
		(PCHAR*)&pClientIP, &RetSize) && pClientIP) {
		if (pClientIP->AddressFamily == AF_INET) {
			MySNPrintf(IPStr, MAX_IP_LEN, "%u.%u.%u.%u", pClientIP->Address[2], pClientIP->Address[3],
				pClientIP->Address[4], pClientIP->Address[5]);
		}
		WTSFreeMemory(pClientIP);
	}
	if (*IPStr) return;

	if (DefIP && *DefIP) MyStrNCopy(IPStr, DefIP, MAX_IP_LEN);

}

VOID GetClientHostName(ULONG SessionId, PCHAR ClientHostName)
{
	ULONG RetSize = 0;
	PCHAR pBuffer = NULL;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSClientName,
		&pBuffer, &RetSize) && pBuffer) {
		MyStrNCpy(ClientHostName, pBuffer, MAX_HOST_LEN);
		WTSFreeMemory(pBuffer);
	}
	else *ClientHostName = 0;
}

ULONG GetDiskFreeMB(PCHAR DirPath, PULONG TotalMB)
{
	ULARGE_INTEGER FreeByte = { 0 }, TotalByte = { 0 };

	GetDiskFreeSpaceEx(DirPath, NULL, &TotalByte, &FreeByte);

	if (TotalMB) *TotalMB = (ULONG)(TotalByte.QuadPart / MB1);

	return (ULONG)(FreeByte.QuadPart / MB1);
}

time_t FileTimeToSecond(PFILETIME FileTime)
{
	SYSTEMTIME SysTime;
	FILETIME LocalTime;
	struct tm DateTime;

	FileTimeToLocalFileTime(FileTime, &LocalTime);
	FileTimeToSystemTime(&LocalTime, &SysTime);

	DateTime.tm_year = SysTime.wYear - 1900;
	DateTime.tm_mon = SysTime.wMonth - 1;
	DateTime.tm_mday = SysTime.wDay;
	DateTime.tm_hour = SysTime.wHour;
	DateTime.tm_min = SysTime.wMinute;
	DateTime.tm_sec = SysTime.wSecond;

	return mktime(&DateTime);
}

BOOLEAN(WINAPI* _WinStationQueryInformationW)(
	HANDLE, ULONG, WINSTATIONINFOCLASS, PVOID, ULONG, PULONG) = NULL;

time_t GetTSTimes(ULONG SessionId, PCHAR LogonTime, PCHAR ConnectTime, PCHAR ConnectNumOnly)
{
	WINSTATIONINFORMATIONW WinStation = { 0 };
	ULONG RetLen = 0;
	time_t LastInput = 0;
	CHAR SysDllPath[MAX_KPATH];
	HMODULE hWinSta = LoadLibrary(GetSystemFilePath(SysDllPath, "WINSTA.DLL"));

	if (LogonTime) *LogonTime = 0;
	if (ConnectTime) *ConnectTime = 0;
	if (ConnectNumOnly) *ConnectNumOnly = 0;

	if (!hWinSta) return 0;

	*(FARPROC*)&_WinStationQueryInformationW = GetProcAddress(hWinSta,
		"WinStationQueryInformationW");

	if (_WinStationQueryInformationW && _WinStationQueryInformationW(NULL, SessionId,
		WinStationInformation, &WinStation, sizeof(WinStation), &RetLen)) {
		LastInput = FileTimeToSecond(&WinStation.LastInputTime);
		if (LastInput > 0 && LogonTime)
			FileTimeToStr(&WinStation.LogonTime, LogonTime, NULL);

		if (ConnectTime || ConnectNumOnly)
			FileTimeToStr(&WinStation.ConnectTime, ConnectTime, ConnectNumOnly);
	}

	FreeLibrary(hWinSta);

	return LastInput;
}

ULONG GetScvVerStr(PCHAR Buffer, PCHAR FilePath, ULONG MaxLen)
{
	PCHAR pValue = NULL, VerStr;
	PBYTE pVerInfo = NULL;
	ULONG Len = 0, VerSize, i, LangCnt;
	UINT VerLen = 0;
	CHAR InfoString[MAX_KPATH];

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	VerSize = GetFileVersionInfoSize(FilePath, &VerSize);
	if (VerSize > 0) pVerInfo = (PBYTE)malloc(VerSize);

	if (!pVerInfo) return 0;

	if (GetFileVersionInfo(FilePath, 0, VerSize, pVerInfo)) {
		if (VerQueryValue(pVerInfo, "\\VarFileInfo\\Translation", (PVOID*)&lpTranslate, &VerLen)) {
			LangCnt = VerLen / sizeof(struct LANGANDCODEPAGE);
			for (i = 0; i < LangCnt && Len == 0; i++) {
				MySNPrintf(InfoString, MAX_NAME, "\\StringFileInfo\\%04x%04x\\ProductVersion",
					lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
				if (!VerQueryValue(pVerInfo, InfoString, (PVOID*)&pValue, &VerLen)) continue;

				pValue[VerLen] = 0;
				for (VerStr = strtok(pValue, " ,"); VerStr;) {
					Len += MySNPrintf(Buffer + Len, MaxLen - Len, "%s", VerStr);
					VerStr = strtok(NULL, " ,");
					if (VerStr) Len += MySNPrintf(Buffer + Len, MaxLen - Len, ".");
				}
			}
		}
	}
	free(pVerInfo);

	return Len;
}

ULONG GetOSVerFromFile(PCHAR SysFilePath, PULONG MinorVersion, PULONG BuildNumber)
{
	ULONG MajorVersion = 5;
	CHAR OSVerStr[MAX_KPATH] = { 0 }, * Minor, * Build;

	if (MinorVersion) *MinorVersion = 0;
	if (BuildNumber) *BuildNumber = 0;

	if (GetScvVerStr(OSVerStr, SysFilePath, MAX_KPATH)) {
		MajorVersion = atoi(OSVerStr);

		Minor = strchr(OSVerStr, '.');
		if (Minor) Minor++;

		if (MinorVersion && Minor) *MinorVersion = atoi(Minor);

		if (BuildNumber) {
			Build = strchr(Minor, '.');
			if (Build) *BuildNumber = atoi(Build + 1);
		}
	}
	else pWriteF("Failed to get Version String from %s", SysFilePath);

	return MajorVersion;
}

BOOL ImpersonateUser(PCHAR UserName, PCHAR UserPwd)
{
	HANDLE hToken;
	BOOL bImpersonated = FALSE;


	if (LogonUserA(NULL, UserName, UserPwd, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)) {
		bImpersonated = ImpersonateLoggedOnUser(hToken);
		if (bImpersonated)
			pWriteF("Impersonated as %s", UserName);

		CloseHandle(hToken);
	}
	else pWriteF("LogonUser Failed %d", GetLastError());

	return bImpersonated;
}

VOID GetProcessUserName(HANDLE hProcess, PCHAR UserName)
{
	HANDLE hToken;
	CHAR UserBuf[256];
	DWORD dwRet = 0, dwName = 32;

	if (!hProcess || !UserName) return;

	UserName[0] = 0;

	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) return;

	if (GetTokenInformation(hToken, TokenUser, UserBuf, sizeof(UserBuf), &dwRet)) {
		CHAR szDomain[256];
		DWORD dwDomain = sizeof(szDomain);
		SID_NAME_USE peUse;
		PTOKEN_USER pUser = (PTOKEN_USER)UserBuf;

		LookupAccountSid(NULL, pUser->User.Sid, UserName, &dwName, szDomain, &dwDomain, &peUse);
	}

	CloseHandle(hToken);
}

VOID KillProcesses(PCHAR InstallPath)
{
	CHAR ProcPath[MAX_PATH];
	ULONG PathLen = (ULONG)strlen(InstallPath), CurPID = GetCurrentProcessId();
	HANDLE hProcess;
	HMODULE hNtDll = NULL;
	PVOID ProcListBuf;
	PSYSTEM_PROCESSES pProcess;

	if (!_ZwQuerySystemInformation) hNtDll = SetNAPIAddr();

	ProcListBuf = GetProcessList();
	if (!ProcListBuf) return;

	for (pProcess = (PSYSTEM_PROCESSES)ProcListBuf; pProcess;) {
		if (CurPID != (ULONG)pProcess->ProcessId &&
			GetProcessPath((ULONG)pProcess->ProcessId, ProcPath, FALSE) &&
			!_strnicmp(InstallPath, ProcPath, PathLen)) {
			pWriteF("KillProcesses Find[%s]", ProcPath);

			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (ULONG)pProcess->ProcessId);
			if (hProcess) {
				pWriteF("KillProcesses Find and Open Success[%s]", ProcPath);
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
		}

		if (pProcess->NextEntryDelta == 0) break;
		else pProcess = (PSYSTEM_PROCESSES)((LPBYTE)pProcess + pProcess->NextEntryDelta);
	}

	free(ProcListBuf);
	if (hNtDll) FreeLibrary(hNtDll);
}

VOID KillProcess(PCHAR ProcPath)
{
	ULONG ProcessId;
	HANDLE hProcess;

	ProcessId = FindProcessPath(ProcPath, NULL);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	if (hProcess) {
		pWriteF("KillProcess Find and Open Success[%s]", ProcPath);
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}
}

BOOL GetHostIPs(PHOST_IP HostIP)
{
	struct hostent* pHost = NULL;
	struct in_addr SockAddr;
	ULONG i;

	pHost = gethostbyname(HostIP->HostName);

	if (pHost && pHost->h_addrtype == AF_INET) {
		for (i = 0; pHost->h_addr_list[i] && i < MAX_HOST_IP; i++) {
			memcpy(&SockAddr, pHost->h_addr_list[i], pHost->h_length);
			HostIP->IPs[i] = ntohl(SockAddr.S_un.S_addr);
		}
		HostIP->IPCnt = i;

		return TRUE;
	}
	else {
		pWrite("Fail to Get HostIPs");

		return FALSE;
	}
}

BOOL CheckExistSession(ULONG SessionId)
{
	PWTS_SESSION_INFO pSessionInfo = NULL, TmpInfo;
	ULONG i, SessionCnt = 0;
	BOOL bRet = FALSE;

	if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &SessionCnt))
		return FALSE;

	for (i = 0, TmpInfo = pSessionInfo; i < SessionCnt && TmpInfo && !bRet; i++, TmpInfo++) {
		//if (TmpInfo->State != WTSActive) continue;

		if (TmpInfo->SessionId == SessionId) bRet = TRUE;
	}

	if (pSessionInfo) WTSFreeMemory(pSessionInfo);

	return bRet;
}

BOOL CheckActiveSession(ULONG SessionId)
{
	PWTS_SESSION_INFO pSessionInfo = NULL, TmpInfo;
	ULONG i, SessionCnt = 0;
	BOOL bRet = FALSE;

	if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &SessionCnt))
		return FALSE;

	for (i = 0, TmpInfo = pSessionInfo; i < SessionCnt && TmpInfo && !bRet; i++, TmpInfo++) {
		if (TmpInfo->State != WTSActive) continue;

		if (TmpInfo->SessionId == SessionId) bRet = TRUE;
	}

	if (pSessionInfo) WTSFreeMemory(pSessionInfo);

	return bRet;
}

VOID GetSessionUser(ULONG SessionId, PCHAR UserName)
{
	ULONG RetSize = 0;
	PCHAR pBuffer = NULL;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSUserName,
		&pBuffer, &RetSize) && pBuffer) {
		MyStrNCpy(UserName, pBuffer, MAX_USER_NAME);
		WTSFreeMemory(pBuffer);
	}
	else *UserName = 0;
}

VOID GetSessionDomain(ULONG SessionId, PCHAR DomainName)
{
	ULONG RetSize = 0;
	PCHAR pBuffer = NULL;

	*DomainName = 0;
	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSDomainName,
		&pBuffer, &RetSize) && pBuffer) {
		if (RetSize > 1)
			MySNPrintf(DomainName, MAX_PATH, "%s\\", pBuffer);

		WTSFreeMemory(pBuffer);
	}
}

VOID GetSessionIP(ULONG SessionId, PCHAR IPStr, PCHAR DefIP)
{
	ULONG RetSize = 0;
	PWTS_CLIENT_ADDRESS pClientIP = NULL;

	*IPStr = 0;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSClientAddress,
		(PCHAR*)&pClientIP, &RetSize) && pClientIP) {
		if (pClientIP->AddressFamily == AF_INET) {
			MySNPrintf(IPStr, MAX_IP_LEN, "%u.%u.%u.%u", pClientIP->Address[2], pClientIP->Address[3],
				pClientIP->Address[4], pClientIP->Address[5]);
		}
		WTSFreeMemory(pClientIP);
	}
	if (*IPStr) return;

	if (DefIP && *DefIP) MyStrNCopy(IPStr, DefIP, MAX_IP_LEN);

}

VOID GetClientHostName(ULONG SessionId, PCHAR ClientHostName)
{
	ULONG RetSize = 0;
	PCHAR pBuffer = NULL;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, SessionId, WTSClientName,
		&pBuffer, &RetSize) && pBuffer) {
		MyStrNCopy(ClientHostName, pBuffer, MAX_HOST_LEN);
		WTSFreeMemory(pBuffer);
	}
	else *ClientHostName = 0;
}

PVOID GetProcessList()
{
	NTSTATUS Status;
	ULONG cbBuffer = 0x8000;
	PVOID ProcListBuf = NULL;

	while (_ZwQuerySystemInformation) {
		ProcListBuf = malloc(cbBuffer);
		if (!ProcListBuf) return NULL;

		Status = _ZwQuerySystemInformation(SystemProcessesAndThreadsInformation,
			ProcListBuf, cbBuffer, NULL);
		if (Status) {
			free(ProcListBuf);
			if (Status == STATUS_INFO_LENGTH_MISMATCH) cbBuffer *= 2;
			else return NULL;
		}
		else break;
	}

	return ProcListBuf;
}

ULONG FindProcessPathEx(PCHAR ProcPath, PULONG SessionId, PCHAR CreateTime)
{
	HMODULE hNtDll = NULL;
	CHAR TmpPath[MAX_KPATH];
	PVOID ProcListBuf;
	PSYSTEM_PROCESSES pProcess;
	ULONG ProcessId = 0, TmpSessionId;

	if (!_ZwQuerySystemInformation) hNtDll = SetNAPIAddr();

	ProcListBuf = GetProcessList();
	if (!ProcListBuf) return 0;

	for (pProcess = (PSYSTEM_PROCESSES)ProcListBuf; pProcess;) {
		if (GetProcessPath((ULONG)pProcess->ProcessId, TmpPath, FALSE) &&
			!_stricmp(TmpPath, ProcPath)) {
			if (SessionId) ProcessIdToSessionId((ULONG)pProcess->ProcessId, &TmpSessionId);

			if (!SessionId || *SessionId == TmpSessionId) {
				ProcessId = (ULONG)pProcess->ProcessId;

				if (CreateTime) {
					FILETIME FileTime;

					FileTime.dwHighDateTime = pProcess->CreateTime.HighPart;
					FileTime.dwLowDateTime = pProcess->CreateTime.LowPart;
					FileTimeToStr(&FileTime, CreateTime, NULL);
				}
				break;
			}
		}

		if (pProcess->NextEntryDelta == 0) break;
		else pProcess = (PSYSTEM_PROCESSES)((LPBYTE)pProcess + pProcess->NextEntryDelta);
	} //for (pProcess = (PSYSTEM_PROCESSES)ProcListBuf; pProcess;) {

	free(ProcListBuf);
	if (hNtDll) FreeLibrary(hNtDll);

	return ProcessId;
}

BOOL GetProcessPath(ULONG ProcessId, PCHAR ProcPath, BOOL bAddBit)
{
	HANDLE hProc;
	CHAR FullPath[MAX_KPATH], * EnvName, * ProcName;
	BOOL bIsWowProc = FALSE;

	*ProcPath = 0;
	if (ProcessId == 0) return FALSE;

	hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessId);
	if (!hProc) return FALSE;

	if (GetModuleFileNameEx(hProc, NULL, FullPath, MAX_KPATH) > 0) {
		if (!strncmp(FullPath, "\\??\\", 4)) EnvName = FullPath + 4;
		else EnvName = FullPath;

		if (*EnvName == '\\') { //\SystemRoot\smss.exe
			ProcName = strchr(EnvName + 1, '\\');
			if (ProcName) *ProcName++ = 0;

			MySNPrintf(ProcPath, MAX_PATH, "%s\\%s", getenv(EnvName + 1), ProcName);
		}
		else MyStrNCopy(ProcPath, EnvName, MAX_PATH);

		if (bAddBit) {

			if (bIsWowProc) MyStrNCat(ProcPath, " *32", MAX_PATH);
		}
	}
	CloseHandle(hProc);

	return (*ProcPath);
}

BOOL NTServiceInstall(PCHAR SvcName, PCHAR DispName, PCHAR SvcPath,
	DWORD dwStartType, PCHAR Description, BOOL bAddAction, PCHAR Dependencies)
{
	SC_HANDLE hSCM, hSrv;
	BOOL Ret = FALSE;
	CHAR MsgStr[128];

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) return FALSE;

	hSrv = OpenService(hSCM, SvcName, SERVICE_ALL_ACCESS);
	if (hSrv);//ServiceReInstall(SvcName, SvcPath, dwStartType);
	else {
		hSrv = CreateService(hSCM, SvcName, DispName, SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS, dwStartType, SERVICE_ERROR_NORMAL, SvcPath,
			NULL, NULL, Dependencies, NULL, NULL);
		if (hSrv) {
			typedef BOOL(__stdcall* CSC2)(SC_HANDLE, DWORD, LPVOID);
			CHAR SysDllPath[MAX_KPATH];
			HINSTANCE hInstance = LoadLibrary(GetSystemFilePath(SysDllPath, "advapi32.dll"));

			if (hInstance) {
				CSC2 pProcAddr = (CSC2)GetProcAddress(hInstance, "ChangeServiceConfig2A");

				if (pProcAddr) {
					SERVICE_DESCRIPTION sdBuf;

					sdBuf.lpDescription = Description;
					(pProcAddr)(hSrv, SERVICE_CONFIG_DESCRIPTION, &sdBuf);

					if (bAddAction) {
						SERVICE_FAILURE_ACTIONS sfaBuf = { 0 };
						SC_ACTION Action[3];

						memset(Action, 0, sizeof(SC_ACTION) * 3);
						Action[0].Type = SC_ACTION_RESTART;
						Action[0].Delay = 1000 * 60;

						sfaBuf.cActions = 3;
						sfaBuf.lpsaActions = Action;
						(pProcAddr)(hSrv, SERVICE_CONFIG_FAILURE_ACTIONS, &sfaBuf);
					}
				}

				FreeLibrary(hInstance);
			}
		}
	}
	if (hSrv) {
		Ret = TRUE;
		CloseServiceHandle(hSrv);
	}
	else {
		MySNPrintf(MsgStr, sizeof(MsgStr), "Failed to install %s", SvcName);
		MessageBox(NULL, MsgStr, "Fail", MB_OK | MB_ICONERROR);
	}

	CloseServiceHandle(hSCM);

	return Ret;
}

BOOL NTServiceUninstall(PCHAR SvcName, DWORD dwControl)
{
	SC_HANDLE hSCM, hSrv;
	SERVICE_STATUS SvcStat = { 0 };
	BOOL Ret = TRUE;

	NTServiceStop(SvcName, dwControl, TRUE);

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) return FALSE;

	hSrv = OpenService(hSCM, SvcName, SERVICE_ALL_ACCESS);
	if (hSrv) {
		Ret = DeleteService(hSrv);
		CloseServiceHandle(hSrv);
	}

	CloseServiceHandle(hSCM);

	return Ret;
}

BOOL NTServiceSetStart(PCHAR SvcName, ULONG StartType)
{
	HKEY hKey;
	CHAR SvcRegPath[MAX_PATH];

	MySNPrintf(SvcRegPath, MAX_PATH, "%s\\%s", REG_SERVICE, SvcName);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SvcRegPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return FALSE;

	RegSetValueEx(hKey, "Start", 0, REG_DWORD, (unsigned char*)&StartType, sizeof(DWORD));
	RegCloseKey(hKey);

	return TRUE;
}

ULONG NTServiceGetStart(PCHAR SvcName)
{
	SC_HANDLE hSCM, hSrv;
	CHAR Buffer[2048] = { 0 };
	LPQUERY_SERVICE_CONFIG SvcConfig = (LPQUERY_SERVICE_CONFIG)Buffer;
	ULONG BytesNeeded = 0, StartType = -1;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!hSCM) return SERVICE_STOPPED;

	hSrv = OpenService(hSCM, SvcName, SERVICE_QUERY_CONFIG);
	if (!hSrv) {
		CloseServiceHandle(hSCM);
		return -1;
	}

	if (QueryServiceConfig(hSrv, SvcConfig, sizeof(Buffer), &BytesNeeded))
		StartType = SvcConfig->dwStartType;

	CloseServiceHandle(hSrv);
	CloseServiceHandle(hSCM);

	return StartType;
}

BOOL NTServiceStart(PCHAR SvcName, BOOL bWaitDone, PCHAR Params)
{
	SC_HANDLE hSCM, hSrv;
	SERVICE_STATUS SvcStat = { 0 };
	BOOL Ret = FALSE;
	ULONG WaitSec = 0;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) return FALSE;

	hSrv = OpenService(hSCM, SvcName, SERVICE_ALL_ACCESS);
	if (!hSrv) {
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	QueryServiceStatus(hSrv, &SvcStat);
	if (SvcStat.dwCurrentState == SERVICE_STOPPED) {
		if (Params) {
			PCHAR ArgVal[1];

			ArgVal[0] = Params;
			StartService(hSrv, 1, (LPCSTR*)ArgVal);
		}
		else StartService(hSrv, 0, NULL);

		if (bWaitDone) {
			for (; WaitSec < ONEMINUTE && SvcStat.dwCurrentState != SERVICE_RUNNING; WaitSec++) {
				Sleep(1000);
				QueryServiceStatus(hSrv, &SvcStat);
				if (SvcStat.dwCurrentState == SERVICE_STOPPED) break;
			}
		}
		else Sleep(500);
	}

	CloseServiceHandle(hSrv);
	CloseServiceHandle(hSCM);

	return TRUE;
}

BOOL NTServiceStop(PCHAR SvcName, DWORD dwControl, BOOL bWaitDone)
{
	SC_HANDLE hSCM, hSrv;
	SERVICE_STATUS SvcStat = { 0 };
	BOOL Ret = FALSE;
	ULONG WaitSec = 0;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) return FALSE;

	hSrv = OpenService(hSCM, SvcName, SERVICE_ALL_ACCESS);
	if (!hSrv) {
		CloseServiceHandle(hSCM);
		return FALSE;
	}

	QueryServiceStatus(hSrv, &SvcStat);
	if (SvcStat.dwCurrentState != SERVICE_STOPPED) {
		if (dwControl == SERVICE_CONTROL_ALLOW_STOP) {
			Ret = ControlService(hSrv, dwControl, &SvcStat);
		}

		Ret = ControlService(hSrv, SERVICE_CONTROL_STOP, &SvcStat);
		if (bWaitDone) {
			for (; WaitSec < ONEMINUTE && SvcStat.dwCurrentState != SERVICE_STOPPED; WaitSec++) {
				Sleep(1000);
				QueryServiceStatus(hSrv, &SvcStat);
			}
		}
		else Sleep(500);
	}
	else Ret = TRUE;

	CloseServiceHandle(hSrv);
	CloseServiceHandle(hSCM);

	return Ret;
}

ULONG NTServiceStatus(PCHAR SvcName)
{
	SC_HANDLE hSCM, hSrv;
	SERVICE_STATUS SvcStat = { 0 };

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!hSCM) return SERVICE_STOPPED;

	hSrv = OpenService(hSCM, SvcName, SERVICE_QUERY_STATUS);
	if (!hSrv) {
		CloseServiceHandle(hSCM);
		return SERVICE_STOPPED;
	}

	QueryServiceStatus(hSrv, &SvcStat);

	CloseServiceHandle(hSrv);
	CloseServiceHandle(hSCM);

	return SvcStat.dwCurrentState;
}

LPENUM_SERVICE_STATUS_PROCESS NTServiceList(PULONG SvcReturned)
{
	ULONG ErrCode = -1;
	SC_HANDLE hSCM;
	LPENUM_SERVICE_STATUS_PROCESS Services = NULL;
	ULONG ByteNeeded = 0;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) return NULL;

	EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
		NULL, 0, &ByteNeeded, SvcReturned, NULL, NULL);
	if (GetLastError() == ERROR_MORE_DATA && ByteNeeded > 0) {
		Services = (LPENUM_SERVICE_STATUS_PROCESS)malloc(ByteNeeded);
		if (Services) {
			if (EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
				(LPBYTE)Services, ByteNeeded, &ByteNeeded, SvcReturned, NULL, NULL))
				ErrCode = 0;
		}
	}

	CloseServiceHandle(hSCM);

	if (ErrCode == 0) return Services;
	else if (Services) free(Services);

	return NULL;
}

BOOL CheckUserName(PCHAR UserName)
{
	NET_API_STATUS nStatus;
	PUSER_INFO_0 UserInfo0 = NULL;
	WCHAR UserNameW[MAX_USER_NAME] = { 0 };

	if (!UserName || !*UserName) return FALSE;

	if (strlen(UserName) >= MAX_USER_NAME) return FALSE;

	MultiByteToWideChar(CP_ACP, 0, UserName, (int)strlen(UserName) + 1, UserNameW,
		MAX_USER_NAME - 1);
	nStatus = NetUserGetInfo(NULL, UserNameW, 0, (LPBYTE*)&UserInfo0);
	if (UserInfo0) NetApiBufferFree(UserInfo0);

	return (nStatus == NERR_Success);
}

BOOL CheckGroupName(PCHAR GroupName)
{
	NET_API_STATUS nStatus;
	PLOCALGROUP_INFO_1 GroupInfo1 = NULL;
	WCHAR GroupNameW[MAX_USER_NAME] = { 0 };

	if (!GroupName || !*GroupName) return FALSE;

	if (strlen(GroupName) >= MAX_USER_NAME) return FALSE;

	MultiByteToWideChar(CP_ACP, 0, GroupName, (int)strlen(GroupName) + 1, GroupNameW,
		MAX_USER_NAME - 1);
	nStatus = NetLocalGroupGetInfo(NULL, GroupNameW, 1, (LPBYTE*)&GroupInfo1);
	if (GroupInfo1) NetApiBufferFree(GroupInfo1);

	return (nStatus == NERR_Success);
}

BOOL CheckGroupMember(PWCHAR DomainNameW, PWCHAR UserNameW, PCHAR GroupStr, GROUP_TYPE GroupType)
{
	TOSERR ErrCode = ERR_AGT_SUCCESS;
	CHAR GroupName[MAX_PATH * 2];
	PGROUP_USERS_INFO_0 pGroups = NULL, TmpGroup;
	ULONG PreMax = -1, EntriesRead = 0, TotalEntries = 0, GroupCnt, TmpLen;
	NET_API_STATUS nStatus;

	PCHAR Group, NextGroup, TmpStr;
	BOOL bFound = FALSE;

	TmpLen = (ULONG)strlen(GroupStr) + 1;
	TmpStr = (PCHAR)malloc(TmpLen);
	if (!TmpStr) return FALSE;

	do {
		if (GroupType == GRT_LOCAL) {
			nStatus = NetUserGetLocalGroups(DomainNameW, UserNameW, 0,
				LG_INCLUDE_INDIRECT, (LPBYTE*)&pGroups, PreMax, &EntriesRead, &TotalEntries);
		}
		else {
			nStatus = NetUserGetGroups(DomainNameW, UserNameW, 0, (LPBYTE*)&pGroups, PreMax,
				&EntriesRead, &TotalEntries);
		}
		if (nStatus == NERR_UserNotFound) ErrCode = ERR_USER_NOT_FOUND;
		else if ((nStatus != NERR_Success) && (nStatus != ERROR_MORE_DATA)) ErrCode = nStatus;

		MyStrNCopy(TmpStr, GroupStr, TmpLen);
		NextGroup = TmpStr;
		while (NextGroup && ErrCode == ERR_AGT_SUCCESS && !bFound) {
			Group = MyStrTok(NextGroup, SEP_COMMA, &NextGroup, FALSE);
			if (!Group) break;

			for (GroupCnt = 0, TmpGroup = pGroups; GroupCnt < EntriesRead && TmpGroup; GroupCnt++, TmpGroup++) {
				memset(GroupName, 0, sizeof(GroupName));
				WideCharToMultiByte(CP_ACP, 0, TmpGroup->grui0_name, -1, GroupName, MAX_PATH - 1, NULL, FALSE);

				if (!_stricmp(Group, GroupName)) {
					bFound = TRUE;
					break;
				}
			}
		}

		if (pGroups) {
			NetApiBufferFree(pGroups);
			pGroups = NULL;
		}
	} while (nStatus == ERROR_MORE_DATA && ErrCode == ERR_AGT_SUCCESS);

	return bFound;
}

VOID GetProcessUserName(HANDLE hProcess, PCHAR UserName)
{
	HANDLE hToken;
	CHAR UserBuf[256];
	DWORD dwRet = 0, dwName = 32;

	if (!hProcess || !UserName) return;

	UserName[0] = 0;

	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) return;

	if (GetTokenInformation(hToken, TokenUser, UserBuf, sizeof(UserBuf), &dwRet)) {
		CHAR szDomain[256];
		DWORD dwDomain = sizeof(szDomain);
		SID_NAME_USE peUse;
		PTOKEN_USER pUser = (PTOKEN_USER)UserBuf;

		LookupAccountSid(NULL, pUser->User.Sid, UserName, &dwName, szDomain, &dwDomain, &peUse);
	}

	CloseHandle(hToken);
}

BOOL StartProcessSession(PCHAR Command, ULONG SessionId, BOOL bWait)
{
	HANDLE hToken;
	STARTUPINFO StartInfo = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	CHAR SysDllPath[MAX_KPATH];
	HINSTANCE hWtsApi = LoadLibrary(GetSystemFilePath(SysDllPath, "WtsApi32.dll"));
	BOOL bRetValue = FALSE;

	if (!hWtsApi) return bRetValue;

	*(FARPROC*)&_WTSQueryUserToken = GetProcAddress(hWtsApi, "WTSQueryUserToken");

	if (_WTSQueryUserToken && _WTSQueryUserToken(SessionId, &hToken)) {
		StartInfo.cb = sizeof(STARTUPINFO);
		if (CreateProcessAsUser(hToken, NULL, AddQuots(Command), NULL, NULL, FALSE, 0, NULL, NULL,
			&StartInfo, &ProcInfo)) {
			if (bWait) WaitForInputIdle(ProcInfo.hProcess, 10000);

			CloseHandle(ProcInfo.hProcess);
			CloseHandle(ProcInfo.hThread);

			bRetValue = TRUE;
		}

		CloseHandle(hToken);
	}

	FreeLibrary(hWtsApi);

	return bRetValue;
}

DWORD WINAPI RestartProcess(LPVOID Param)
{
	PRESTART_PROC Restart = (PRESTART_PROC)Param;
	CHAR ProcPath[MAX_KPATH] = { 0 }, AgentDir[MAX_KPATH];
	ULONG ProcessId, i;
	HANDLE hProcess;

	if (!Restart) return 0;

	//Kill Process
	MyStrNCopy(ProcPath, Restart->FilePath, MAX_KPATH);

	if (*ProcPath) {
		ProcessId = FindProcessPathEx(ProcPath, &Restart->SessionId, NULL);
		if (ProcessId) {
			hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);
			if (hProcess) {
				if (TerminateProcess(hProcess, 0)) {
					for (i = 0; i < CONN_TIMEOUT; i++) {
						Sleep(1000);
						if (!FindProcessPathEx(ProcPath, &Restart->SessionId, NULL)) break;
					}
				}

				CloseHandle(hProcess);
			}
		}
		//Create Process
		StartProcessSession(ProcPath, Restart->SessionId, TRUE);
	}

	free(Restart);

	return 0;
}

BOOL GetSessionIPByHostName(ULONG SessionId, PCHAR IPStr)
{
	CHAR HostName[MAX_NAME] = { 0 };
	WSADATA WSAData;

	struct addrinfo* pAddrInfo = NULL, * pAddr = NULL;
	struct addrinfo Hints;
	struct sockaddr_in* pClientIP;
	DWORD dwRetval;

	GetClientHostName(SessionId, HostName);
	if (!*HostName) return FALSE;

	WSAStartup(MAKEWORD(2, 2), &WSAData);

	ZeroMemory(&Hints, sizeof(Hints));
	Hints.ai_family = AF_UNSPEC;
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_protocol = IPPROTO_TCP;

	dwRetval = getaddrinfo(HostName, "0", &Hints, &pAddrInfo);
	if (dwRetval) {
		pWriteF("getaddrinfo failed with error: %d", dwRetval);
		WSACleanup();

		return FALSE;
	}

	for (pAddr = pAddrInfo; pAddr != NULL; pAddr = pAddr->ai_next) {
		switch (pAddr->ai_family) {
		case AF_INET:
			pClientIP = (struct sockaddr_in*)pAddr->ai_addr;
			MySNPrintf(IPStr, MAX_IP_LEN, "%s", inet_ntoa(pClientIP->sin_addr));
			break;

		default:
			break;
		}
	}

	freeaddrinfo(pAddrInfo);
	WSACleanup();

	return *IPStr ? TRUE : FALSE;
}

BOOL GetSessionNetList(PCHAR NetListStr, ULONG MaxNetCnt)
{
	ULONG Len = 0;
	PCHAR pTemp;
	CHAR TmpName[300] = { 0, }, CmdLine[320] = { 0, }, LineBuf[1024] = { 0, };
	STARTUPINFO StartInfo = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	ULONG TickCount = GetTickCount(), nLimitTime = 8, WaitRet;
	FILE* fp;
	HKEY hKey;
	ULONG DataLen, Port = 3389;
	ULONG NetCnt = 0;

	pTemp = getenv("TEMP");
	if (pTemp) MySNPrintf(TmpName, sizeof(TmpName), "%s\\scvnetlist", pTemp);
	else MySNPrintf(TmpName, sizeof(TmpName), "C:\\scvnetlist");

	//netstat
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp", &hKey) == ERROR_SUCCESS) {
		DataLen = sizeof(ULONG);
		RegQueryValueEx(hKey, "PortNumber", NULL, NULL, (PBYTE)&Port, &DataLen);
		RegCloseKey(hKey);
	}

	pWriteF("GetSessionNetList netstat[%s], port[%u]", TmpName, Port);
	MySNPrintf(CmdLine, sizeof(CmdLine),
		"cmd /c netstat -an | findstr %u | findstr ESTABLISHED > %s 2>&1", Port, TmpName);

	ZeroMemory(&StartInfo, sizeof(StartInfo));
	ZeroMemory(&ProcInfo, sizeof(ProcInfo));
	StartInfo.cb = sizeof(StartInfo);
	if (!CreateProcess(NULL, CmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL,
		&StartInfo, &ProcInfo))
		return FALSE;

	CloseHandle(ProcInfo.hThread);
	WaitRet = WaitForSingleObject(ProcInfo.hProcess, nLimitTime * 1000);
	if (WaitRet != WAIT_OBJECT_0) {
		TerminateProcess(ProcInfo.hProcess, 0);
		DeleteFile(TmpName);
		CloseHandle(ProcInfo.hProcess);
		return FALSE;
	}
	CloseHandle(ProcInfo.hProcess);

	fp = fopen(TmpName, "rt");
	if (!fp) return FALSE;

	while (!feof(fp)) {
		memset(LineBuf, 0x00, sizeof(LineBuf));
		fgets(LineBuf, sizeof(LineBuf), fp);

		if (!strstr(LineBuf, "TCP ")) continue;

		//TCP    192.168.152.11:3389    192.168.150.211:1201   ESTABLISHED
		if (NetCnt >= MaxNetCnt) {
			pWriteF("GetSessionNetList Overflow, MaxNetCnt[%d]", MaxNetCnt);
			break;
		}

		if (Len > 0) Len += MySNPrintf(NetListStr + Len, (MAX_NET_LEN + 1) * MaxNetCnt - Len, ",");

		Len += MySNPrintf(NetListStr + Len, (MAX_NET_LEN + 1) * MaxNetCnt - Len, "%s",
			Trim(LineBuf, FALSE));
		NetCnt++;
	}

	*(NetListStr + Len) = '\0';

	fclose(fp);
	DeleteFile(TmpName);

	return (Len > 0);
}

BOOL AddToACL(PACL* pACL, const PCHAR AccountName, ULONG AccessOption)
{
	CHAR  SIdBuf[MAX_PATH] = { 0 };
	DWORD SIdLen = MAX_PATH;
	CHAR  Domain[MAX_PATH] = { 0 };
	DWORD DomainLen = MAX_PATH;
	SID_NAME_USE SNU;
	PACL TmpACL = NULL;
	EXPLICIT_ACCESS ExAcc = { 0 };

	if (!LookupAccountName(NULL, AccountName, (PSID)SIdBuf, &SIdLen, Domain, &DomainLen, &SNU))
		return FALSE;

	ExAcc.grfAccessPermissions = AccessOption;
	ExAcc.grfAccessMode = SET_ACCESS;
	ExAcc.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ExAcc.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ExAcc.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ExAcc.Trustee.ptstrName = (LPTSTR)(PSID)SIdBuf;

	if (SetEntriesInAcl(1, &ExAcc, *pACL, &TmpACL) != ERROR_SUCCESS)
		return FALSE;

	LocalFree(*pACL);
	*pACL = TmpACL;

	return TRUE;
}

BOOL ChangeFileSecurity(PCHAR FilePath)
{
	BOOL bRet = FALSE;
	BYTE SDBuffer[4096] = { 0 };
	ULONG SDLength = 4096;
	PSECURITY_DESCRIPTOR SD = (PSECURITY_DESCRIPTOR)SDBuffer;
	PACL pACL;
	ULONG AccessOption = GENERIC_ALL | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;

	if (!InitializeSecurityDescriptor(SD, SECURITY_DESCRIPTOR_REVISION)) return FALSE;

	pACL = (PACL)LocalAlloc(LMEM_FIXED, sizeof(ACL));
	if (!pACL) return FALSE;

	if (InitializeAcl(pACL, MAX_PATH, ACL_REVISION)) {
		AddToACL(&pACL, "Everyone", AccessOption);

		if (SetSecurityDescriptorDacl(SD, TRUE, pACL, TRUE)) {
			bRet = SetFileSecurity(FilePath, DACL_SECURITY_INFORMATION, SD);
		}
	}

	LocalFree(pACL);

	return bRet;
}

time_t GetBootTime()
{
	CHAR FilePath[MAX_KPATH];
	struct _stat Stat = { 0 };

	GetWindowsDirectory(FilePath, MAX_KPATH);
	MyStrNCopy(FilePath + 3, "pagefile.sys", MAX_KPATH);
	_stat(FilePath, &Stat);

	return Stat.st_mtime;
}

ULONG AddChar(PCHAR SourceStr, PCHAR TargetStr, CHAR AChar, ULONG TermLen, ULONG MaxLen)
{
	PCHAR Source;
	ULONG SLen, TLen = 0;

	if (!SourceStr || !TargetStr) return 0;

	for (SLen = 0, Source = SourceStr; *Source && TLen < (MaxLen - 2); Source++, SLen++) {
		if (SLen > 0 && (SLen % TermLen == 0)) *(TargetStr + TLen++) = AChar;

		*(TargetStr + TLen++) = *Source;
	}
	*(TargetStr + TLen) = 0;

	return TLen;
}

VOID ReplaceChar(PCHAR SourceStr, CHAR OldChar, CHAR NewChar)
{
	PCHAR Source;

	for (Source = SourceStr; Source && *Source; Source++) {
		if (*Source == OldChar) *Source = NewChar;
	}
}

ULONG RemoveChar(PCHAR SourceStr, PCHAR TargetStr, CHAR RemChar, ULONG MaxLen)
{
	PCHAR Source;
	ULONG Len = 0;

	if (!SourceStr || !TargetStr) return 0;

	for (Source = SourceStr; *Source && Len < (MaxLen - 1); Source++) {
		if (*Source == RemChar) continue;
		else *(TargetStr + Len++) = *Source;
	}
	*(TargetStr + Len) = 0;

	return Len;
}

VOID SetSecAttr(PSECURITY_ATTRIBUTES pSecAttr, PSECURITY_DESCRIPTOR pSecDesc)
{
	InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(pSecDesc, TRUE, NULL, FALSE);

	pSecAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
	pSecAttr->lpSecurityDescriptor = pSecDesc;
	pSecAttr->bInheritHandle = TRUE;
}

BOOL AdjustPrivForProc(PCHAR PrivName, BOOL bEnable)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TokenPriv;
	BOOL Ret = FALSE;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return FALSE;

	LookupPrivilegeValue(NULL, PrivName, &TokenPriv.Privileges[0].Luid);
	TokenPriv.PrivilegeCount = 1;
	if (bEnable) TokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else TokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;

	Ret = AdjustTokenPrivileges(hToken, FALSE, &TokenPriv, 0, NULL, 0);

	CloseHandle(hToken);

	if (TokenPriv.Privileges[0].Attributes == 0) return FALSE;
	else return Ret;
}

BOOL MiniThreadCreate(PMINI_THREAD MiniThread, PVOID Parameter)
{
	if (MiniThread->ThreadHandle == TRUE) return TRUE;

	MiniThread->Parameter = Parameter;
	MiniThread->ThreadHandle = CreateThread(NULL, 0, MiniThread->StartAddress, (PVOID)MiniThread, 0,
		&MiniThread->ThreadId);

	return (MiniThread->ThreadHandle != NULL);
}

VOID MiniThreadTerminate(PMINI_THREAD MiniThread, BOOL bWaitThread)
{
	if (!MiniThread->ThreadHandle) return;

	MiniThread->Terminated = TRUE;
	if (bWaitThread == TRUE) WaitForSingleObject(MiniThread->ThreadHandle, INFINITE);
}

ULONG hexstr2num(PCHAR hexa)
{
	char* p = NULL;
	char	digit = 0;
	int		len = 0, i, j;
	ULONG sq16, result;

	if (!hexa || !*hexa) return 0;

	if (hexa[0] == '0' && (hexa[1] == 'x' || hexa[1] == 'X')) p = hexa + 2;
	else p = hexa;

	len = (int)strlen(p);
	result = 0;
	for (i = 0; i < len; i++, p++) {
		sq16 = 1;
		for (j = 0; j < len - i - 1; j++)
			sq16 = sq16 * 16;

		if (*p >= 'a' && *p <= 'f') digit = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F') digit = *p - 'A' + 10;
		else digit = *p - '0';

		result += (sq16 * digit);
	}

	return result;
}

VOID GenRandStr(PCHAR RandStr, ULONG StrLen)
{
	ULONG i, Special;
	BOOL bSpecial = FALSE, bLower = FALSE, bUpper = FALSE, bDigit = FALSE;

	if (StrLen < 4) return;

	srand((unsigned)time(NULL));

	RandStr[0] = (rand() % 26) + 'A';
	RandStr[1] = (rand() % 26) + 'a';
	RandStr[2] = (rand() % 10) + '0';

	Special = rand() % 4;
	if (Special == 0) RandStr[3] = (rand() % 16) + ' ';
	else if (Special == 1) RandStr[3] = (rand() % 7) + ':';
	else if (Special == 2) RandStr[3] = (rand() % 6) + '[';
	else RandStr[3] = (rand() % 4) + '{';

	for (i = 4; i < StrLen; i++) {
		RandStr[i] = (rand() % 95) + ' ';
	}
	RandStr[i] = 0;
}