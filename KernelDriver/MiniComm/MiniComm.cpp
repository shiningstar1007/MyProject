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