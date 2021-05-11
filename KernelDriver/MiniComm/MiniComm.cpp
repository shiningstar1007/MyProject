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
		MyStrNCpy(UserName, pBuffer, MAX_USER_NAME);
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