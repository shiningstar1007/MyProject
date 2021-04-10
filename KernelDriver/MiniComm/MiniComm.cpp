#include <Windows.h>
#include <fltUser.h>

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