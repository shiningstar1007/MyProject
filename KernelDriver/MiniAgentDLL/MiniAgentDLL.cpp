#include "MiniAgentDLL.h"

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

typedef struct _USER_INFO_24 {
	BOOL     usri24_internet_identity;
	DWORD    usri24_flags;
	LPWSTR   usri24_internet_provider_name;
	LPWSTR   usri24_internet_principal_name;
	PSID     usri24_user_sid;
} USER_INFO_24, * PUSER_INFO_24, * LPUSER_INFO_24;

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

BOOL CheckWindows8OrGreater()
{
	DWORD major, minor;

	GetWinVer(major, minor);

	return (major >= 6 && minor >= 2) ? TRUE : FALSE;
}

void UserList()
{
	LPUSER_INFO_20 pBuf = NULL, pTempBuf;
	LPUSER_INFO_24 pUserInfo = NULL;
	DWORD dwPreMaxLen = (DWORD)-1, dwEntriesRead = 0;
	DWORD dwTotalEntries = 0, dwResumeHandle = 0, i;
	NET_API_STATUS nStatus;
	CHAR UserName[MAX_PATH];
	ULONG Len = 0, OffSet;

	do {

		Len = OffSet = 0;

		do {
			nStatus = NetUserEnum(NULL, 20, 0, (LPBYTE*)&pBuf, dwPreMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
			if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
				pTempBuf = pBuf;
				if (pTempBuf != NULL) {
					for (i = 0; i < dwEntriesRead; i++) {
						if (!pTempBuf) break;

						if ((pTempBuf->usri20_flags & UF_ACCOUNTDISABLE) != UF_ACCOUNTDISABLE) {
							Len = WideCharToMultiByte(CP_ACP, 0, pTempBuf->usri20_name, -1, UserName, MAX_KPATH, NULL, FALSE);

							if (CheckWindows8OrGreater()) {
								if (NetUserGetInfo(NULL, pTempBuf->usri20_name, 24, (LPBYTE*)&pUserInfo) == NERR_Success) {
									if (pUserInfo->usri24_internet_identity) {
										Len = WideCharToMultiByte(CP_ACP, 0, pUserInfo->usri24_internet_principal_name, -1, UserName, MAX_KPATH, NULL, FALSE);
									}
									else Len = WideCharToMultiByte(CP_ACP, 0, pTempBuf->usri20_name, -1, UserName, MAX_KPATH, NULL, FALSE);

									NetApiBufferFree(pUserInfo);
								}
							}
							else Len = WideCharToMultiByte(CP_ACP, 0, pTempBuf->usri20_name, -1, UserName, MAX_KPATH, NULL, FALSE);

							if (OffSet > 0) CopyLineBuf(&KEParam->StrBuf, &OffSet, SEP_NEWLN, (ULONG)strlen(SEP_NEWLN));

							KECode = CopyLineBuf(&KEParam->StrBuf, &OffSet, UserName, (Len - 1));
							if (KECode != ERR_KE_SUCCESS) break;
						}

						pTempBuf++;
					}
				}
			}

			if (pBuf != NULL) {
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
		} while (KECode == ERR_KE_SUCCESS && nStatus == ERROR_MORE_DATA);

	} while (KECode == ERR_MORE_BUFFER);

	if (pBuf != NULL) NetApiBufferFree(pBuf);

	return KECode;
}