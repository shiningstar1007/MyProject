#include "stdafx.h"

LONG g_NonPagedPoolCnt = 0;
PVOID MyAllocNonPagedPool(
	_In_ ULONG BufSize,
	_Inout_ PLONG MemCnt
)
{
	PVOID Buffer;

	if (BufSize == 0) return NULL;

	Buffer = ExAllocatePoolWithTag(NonPagedPool, BufSize, TAG_MINIFLT);
	if (Buffer) InterlockedIncrement(MemCnt);

	return Buffer;
}

PVOID MyFreeNonPagedPool(
	_Inout_ PVOID Buffer,
	_Inout_ PLONG MemCnt
)
{
	if (!Buffer) return NULL;

	InterlockedDecrement(MemCnt);
	ExFreePoolWithTag(Buffer, TAG_MINIFLT);

	return NULL;
}

ULONG MyStrNCopy(
	_Out_ PCHAR DestBuf,
	_In_ CONST PCHAR SourceBuf,
	_In_ ULONG MaxLen
)
{
	ULONG SourceLen;

	if (!DestBuf || !SourceBuf) return 0;

	SourceLen = (ULONG)strlen(SourceBuf);
	if (MaxLen <= SourceLen) SourceLen = MaxLen;

	RtlStringCchCopyNA(DestBuf, MaxLen, SourceBuf, SourceLen);

	return SourceLen;
}

ULONG MyStrNCopyW(
	_Out_ PWCHAR DestBuf,
	_In_ CONST PWCHAR SourceBuf,
	_In_ ULONG SourceLen,
	_In_ ULONG MaxLen
)
{
	if (!DestBuf || !SourceBuf) return 0;

	if (SourceLen == -1) SourceLen = (ULONG)wcslen(SourceBuf);

	if (MaxLen <= SourceLen) SourceLen = MaxLen;

	RtlStringCchCopyNW(DestBuf, MaxLen, SourceBuf, SourceLen);

	return SourceLen;
}

ULONG MySNPrintf(
	_Out_opt_ PCHAR DestBuf,
	_In_ ULONG MaxLen,
	_In_opt_ CONST PCHAR FormatStr, ...
)
{
	va_list ap;

	if (!DestBuf || !FormatStr) return 0;

	va_start(ap, FormatStr);
	RtlStringCchVPrintfA(DestBuf, MaxLen, FormatStr, ap);
	va_end(ap);

	return (ULONG)strlen(DestBuf);
}

ULONG MySNPrintfW(
	_Out_opt_ PWCHAR DestBuf,
	_In_ ULONG MaxLen,
	_In_opt_ CONST PWCHAR FormatStr, ...
)
{
	va_list ArgList;

	if (!DestBuf || !FormatStr) return 0;

	va_start(ArgList, FormatStr);
	RtlStringCchVPrintfW(DestBuf, MaxLen, FormatStr, ArgList);
	va_end(ArgList);

	return (ULONG)wcslen(DestBuf);
}

WINDOWS_VERSION g_WinVersion;
VOID GetVersion()
{
	RTL_OSVERSIONINFOW VersionInfo;

	RtlGetVersion(&VersionInfo);

	if (VersionInfo.dwMajorVersion == 6) {
		if (VersionInfo.dwMinorVersion == 0) g_WinVersion = WV_2008;
		else if (VersionInfo.dwMinorVersion == 1) g_WinVersion = WV_2008R2;
		else if (VersionInfo.dwMinorVersion == 2) g_WinVersion = WV_2012;
		else if (VersionInfo.dwMinorVersion == 3) g_WinVersion = WV_2012R2;
	}
	else if (VersionInfo.dwMajorVersion == 10) {
		if (VersionInfo.dwMinorVersion == 0) g_WinVersion = WV_2016;
	}
	else g_WinVersion = WV_UNKNOWN;
}

ULONG g_ProcNameOffset = 0, g_PebOffset = 0;
VOID InitializeProcess()
{
	PEPROCESS pProcess = PsGetCurrentProcess();

	if (!pProcess) return;

	for (g_ProcNameOffset = 0; g_ProcNameOffset < 3 * PAGE_SIZE; g_ProcNameOffset++)
		if (!_strnicmp(SYSNAME, (PCHAR)pProcess + g_ProcNameOffset, strlen(SYSNAME))) break;

	switch (g_ProcNameOffset) {
		case PROC_OFFSET_WS2008:
			g_PebOffset = PEB_OFFSET_WS2008;
			break;
		case PROC_OFFSET_WIN7:
			g_PebOffset = PEB_OFFSET_WIN7;
			break;
		case PROC_OFFSET_WS2008_x64:
			g_PebOffset = PEB_OFFSET_WS2008_x64;
			break;
		case PROC_OFFSET_WS2008_x64_R2:
			g_PebOffset = PEB_OFFSET_WS2008_x64_R2;
			break;
		case PROC_OFFSET_WS2012_x86:
			g_PebOffset = PEB_OFFSET_WS2012_x86;
			break;
		case PROC_OFFSET_WS2012_x64:
			g_PebOffset = PEB_OFFSET_WS2012_x64;
			break;
		case PROC_OFFSET_WS2016_x86:
			g_PebOffset = PEB_OFFSET_WS2016_x86;
			break;
		case PROC_OFFSET_WS2016_x64:
			g_PebOffset = PEB_OFFSET_WS2016_x64;
			break;
		case PROC_OFFSET_WS2019_x86:
			g_PebOffset = PEB_OFFSET_WS2019_x86;
			break;
		case PROC_OFFSET_WS2019_x64:
			g_PebOffset = PEB_OFFSET_WS2016_x64;
			break;
		case PROC_OFFSET_WS2019_x64_2004:
			g_PebOffset = PEB_OFFSET_WS2019_x64;
			break;

		default:
			DbgPrint("Unknown Process Offset");
	}
}

QUERY_INFO_PROCESS ZwQueryInformationProcess = NULL;
NTSTATUS ZwGetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG BufLen = 0;
	HANDLE hProcess = NULL;
	PEPROCESS pProcess = NULL;
	PUNICODE_STRING ProcName;
	PVOID Buffer = NULL;

	PAGED_CODE();

	pProcess = FltGetRequestorProcess(Data);
	if (pProcess) {
		Status = ObOpenObjectByPointer(pProcess, OBJ_KERNEL_HANDLE, NULL, GENERIC_READ, 0, KernelMode, &hProcess);
		if (!NT_SUCCESS(Status)) {
			DbgPrint("[TEST] ObOpenObjectByPointer failed [0x%X]", Status);
			return Status;
		}

		if (!ZwQueryInformationProcess) {
			UNICODE_STRING RoutineName;
			RtlInitUnicodeString(&RoutineName, L"ZwQueryInformationProcess");

			ZwQueryInformationProcess = (QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&RoutineName);

			if (!ZwQueryInformationProcess) {
				ZwClose(hProcess);
				return STATUS_UNSUCCESSFUL;
			}
		}

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &BufLen);
		if (Status != STATUS_INFO_LENGTH_MISMATCH) {
			DbgPrint("[TEST] ZwQueryInformationProcess failed #1[0x%X]", Status);
			ZwClose(hProcess);
			return Status;
		}

		Buffer = MyAllocNonPagedPool(BufLen, &g_NonPagedPoolCnt);
		if (!Buffer) {
			ZwClose(hProcess);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, Buffer, BufLen, &BufLen);
		if (NT_SUCCESS(Status))
		{
			ProcName = (PUNICODE_STRING)Buffer;

			DbgPrint("[TEST] FullPath[%S]", ProcName->Buffer);
		}
		else DbgPrint("[TEST] ZwQueryInformationProcess failed #2[0x%X]", Status);

		MyFreeNonPagedPool(Buffer, &g_NonPagedPoolCnt);

		ZwClose(hProcess);
	}
	else DbgPrint("[TEST] FltGetRequestorProcess failed");

	return Status;
}

NTSTATUS GetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PEPROCESS pProcess = NULL;
	PUNICODE_STRING ProcName = NULL;

	PAGED_CODE();

	pProcess = FltGetRequestorProcess(Data);
	if (pProcess) {
		Status = SeLocateProcessImageName(pProcess, &ProcName); // Windows Vista and later only.
		if (NT_SUCCESS(Status)) {
			DbgPrint("[TEST]ProcName[%S]", ProcName->Buffer);
			ExFreePool(ProcName);
		}
		else DbgPrint("[TEST] SeLocateProcessImageName failed [0x%X]", Status);
	}

	return Status;
}

VOID GetUserName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PACCESS_TOKEN pToken = NULL;
	PTOKEN_USER pUser = NULL;
	SID* SId;
	ULONG NameSize = 0;
	UNICODE_STRING UniName;
	//UNICODE_STRING UniDomain;
	//ULONG DomainSize = 0;
	SID_NAME_USE NameUse;

	pToken = SeQuerySubjectContextToken(&(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext));
	if (pToken) {
		Status = SeQueryInformationToken(pToken, TokenUser, &pUser);

		if (NT_SUCCESS(Status)) {
			SId = (SID*)pUser->User.Sid;
			Status = SecLookupAccountSid(SId, &NameSize, NULL, NULL, NULL, &NameUse);
			if (Status == STATUS_BUFFER_TOO_SMALL) {
				UniName.Length = (USHORT)(NameSize * sizeof(WCHAR));
				UniName.MaximumLength = (USHORT)(NameSize * sizeof(WCHAR));
				UniName.Buffer = MyAllocNonPagedPool(NameSize * sizeof(WCHAR), &g_NonPagedPoolCnt);

				//  UniDomain.Length = DomainSize * sizeof(WCHAR);
				//  UniDomain.MaximumLength = DomainSize * sizeof(WCHAR);
				//  UniDomain.Buffer = MyAllocNonPagedPool(NameSize * sizeof(WCHAR), &g_NonPagedPoolCnt);

				Status = SecLookupAccountSid(SId, &NameSize, &UniName, NULL, NULL, &NameUse);
				if (NT_SUCCESS(Status)) {
					DbgPrint("[TEST] Name[%S]", UniName.Buffer);
				}
				else DbgPrint("[TEST] SecLookupAccountSid failed [0x%X]", Status);

				if (UniName.Buffer) MyFreeNonPagedPool(UniName.Buffer, &g_NonPagedPoolCnt);

				// if (UniDomain.Buffer) MyFreeNonPagedPool(UniDomain.Buffer, &g_NonPagedPoolCnt);
			}
		}

		if (pUser) ExFreePool(pUser);
	}
}

VOID GetGroupName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PACCESS_TOKEN pToken = NULL;
	PTOKEN_GROUPS pGroups = NULL;
	SID* SId;
	ULONG NameSize = 0;
	UNICODE_STRING UniName;
	//UNICODE_STRING UniDomain;
	//ULONG DomainSize = 0;
	SID_NAME_USE NameUse;

	pToken = SeQuerySubjectContextToken(&(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext));
	if (pToken) {
		Status = SeQueryInformationToken(pToken, TokenGroups, &pGroups);

		if (NT_SUCCESS(Status)) {
			SId = (SID*)pGroups->Groups->Sid;
			Status = SecLookupAccountSid(SId, &NameSize, NULL, NULL, NULL, &NameUse);
			if (Status == STATUS_BUFFER_TOO_SMALL) {
				UniName.Length = (USHORT)(NameSize * sizeof(WCHAR));
				UniName.MaximumLength = (USHORT)(NameSize * sizeof(WCHAR));
				UniName.Buffer = MyAllocNonPagedPool(NameSize * sizeof(WCHAR), &g_NonPagedPoolCnt);

				//  UniDomain.Length = DomainSize * sizeof(WCHAR);
				//  UniDomain.MaximumLength = DomainSize * sizeof(WCHAR);
				//  UniDomain.Buffer = MyAllocNonPagedPool(NameSize * sizeof(WCHAR), &g_NonPagedPoolCnt);

				Status = SecLookupAccountSid(SId, &NameSize, &UniName, NULL, NULL, &NameUse);
				if (NT_SUCCESS(Status)) {
					DbgPrint("[TEST] GroupName[%S]", UniName.Buffer);
				}
				else DbgPrint("[TEST] SecLookupAccountSid failed [0x%X]", Status);

				if (UniName.Buffer) MyFreeNonPagedPool(UniName.Buffer, &g_NonPagedPoolCnt);

				// if (UniDomain.Buffer) MyFreeNonPagedPool(UniDomain.Buffer, &g_NonPagedPoolCnt);
			}
		}

		if (pGroups) ExFreePool(pGroups);
	}
}

PVOID MyQueryInformationToken(
	_In_opt_ PACCESS_TOKEN AccessToken,
	_In_ TOKEN_INFORMATION_CLASS TokenInfoClass,
	_In_ ULONG MinSize
)
{
	NTSTATUS Status;
	HANDLE hToken = 0;
	PVOID Buffer = NULL;
	ULONG BufSize = 0;

	if (AccessToken == NULL) return NULL;

	__try {
		Status = ObOpenObjectByPointer(AccessToken, OBJ_KERNEL_HANDLE, NULL, TOKEN_QUERY, NULL, KernelMode,
			&hToken);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Status = STATUS_UNHANDLED_EXCEPTION;
	}

	ObDereferenceObject(AccessToken);
	if (Status != STATUS_SUCCESS) return NULL;

	Status = ZwQueryInformationToken(hToken, TokenInfoClass, NULL, BufSize, &BufSize);
	if (Status == STATUS_BUFFER_TOO_SMALL) {
		if (BufSize < MinSize) BufSize = MinSize;

		Buffer = MyAllocNonPagedPool(BufSize, &g_NonPagedPoolCnt);
		if (Buffer != NULL) {
			Status = ZwQueryInformationToken(hToken, TokenInfoClass, Buffer, BufSize, &BufSize);
			if (Status != STATUS_SUCCESS) {
				MyFreeNonPagedPool(Buffer, &g_NonPagedPoolCnt);
				Buffer = NULL;
			}
		}
	}

	ZwClose(hToken);

	return Buffer;
}

BOOL CheckLocalUser()
{
	BOOLEAN CopyOnOpen, EffectiveOnly, bLocalUser = TRUE;
	SECURITY_IMPERSONATION_LEVEL Level;
	PTOKEN_SOURCE pToKenSource = NULL;
	ULONG MinSize = sizeof(TOKEN_SOURCE);
	PACCESS_TOKEN AccessToken = PsReferenceImpersonationToken(PsGetCurrentThread(),
		&CopyOnOpen, &EffectiveOnly, &Level);

	if (AccessToken == NULL) AccessToken = PsReferencePrimaryToken(PsGetCurrentProcess());

	pToKenSource = (PTOKEN_SOURCE)MyQueryInformationToken(AccessToken, TokenSource, MinSize);
	if (pToKenSource != NULL) {
		bLocalUser = !_strnicmp(pToKenSource->SourceName, "NTLMSSP", 7) ? FALSE : TRUE;
		MyFreeNonPagedPool(pToKenSource, &g_NonPagedPoolCnt);
	}

	return bLocalUser;
}

BOOL CheckLocalUserOnlyCreate(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PACCESS_TOKEN pToken = NULL;
	PTOKEN_SOURCE pSource = NULL;
	BOOL bLocalUser = TRUE;

	pToken = SeQuerySubjectContextToken(&(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext));
	if (pToken) {
		Status = SeQueryInformationToken(pToken, TokenSource, &pSource);

		if (NT_SUCCESS(Status)) {
			bLocalUser = !_strnicmp(pSource->SourceName, "NTLMSSP", 7);
		}

		if (pSource != NULL) ExFreePool(pSource);
	}
}

ULONG GetAction(
	_In_ ACCESS_MASK AccessMask
)
{
	ACCESS_MASK TmpAccessMask;
	ULONG Action = 0;

	if ((AccessMask == FILE_ANY_ACCESS) || (AccessMask == FILE_ALL_ACCESS)) {
		Action = ACT_READ | ACT_WRITE | ACT_TRAVERSE;
	}
	else if (AccessMask & DELETE) Action = ACT_DELETE;
	else {
		TmpAccessMask = FILE_READ_ACCESS | FILE_READ_EA | FILE_READ_ATTRIBUTES | GENERIC_READ;
		if (AccessMask & TmpAccessMask) Action |= ACT_READ;

		TmpAccessMask = FILE_WRITE_ACCESS | FILE_APPEND_DATA | FILE_WRITE_EA |
			FILE_WRITE_ATTRIBUTES | GENERIC_WRITE;
		if (AccessMask & TmpAccessMask) Action |= ACT_WRITE;

		TmpAccessMask = FILE_LIST_DIRECTORY | FILE_TRAVERSE;
		if (AccessMask & TmpAccessMask) Action |= ACT_TRAVERSE;
	}

	return Action;
}

NTSTATUS MiniFltConnect(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
)
{
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	//ClientPort = ClientPort;
	return STATUS_SUCCESS;
}

VOID MiniFltDisconnect(
	_In_opt_ PVOID ConnectionCookie
)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(ConnectionCookie);

	//FltCloseClientPort(Filter, ClientPort);
}

NTSTATUS MiniFltMessage(
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InBufSize) PVOID InBuf,
	_In_ ULONG InBufSize,
	_Out_writes_bytes_to_opt_(OutBufSize, *RetLen) PVOID OutBuf,
	_In_ ULONG OutBufSize,
	_Out_ PULONG RetLen
)
{
	MY_COMMAND Command;
	PWCHAR Data;
	NTSTATUS Status = STATUS_SUCCESS;

	PAGED_CODE();

	UNREFERENCED_PARAMETER(ConnectionCookie);
	UNREFERENCED_PARAMETER(OutBufSize);

	*RetLen = 0;

	if ((InBuf != NULL) && (InBufSize >= (FIELD_OFFSET(COMMAND_DATA, Command) + sizeof(MY_COMMAND)))) {
		__try {
			Command = ((PCOMMAND_DATA)InBuf)->Command;
			Data = (PWCHAR)((PCOMMAND_DATA)InBuf)->Data;
		}
		__except (PsKeExceptionFilter(GetExceptionInformation(), TRUE)) {
			return GetExceptionCode();
		}

		switch (Command) {
		case SET_COMMAND: break;

		case GET_COMMAND: break;
		
		}
	}
	else Status = STATUS_INVALID_PARAMETER;

	return Status;
}

LONG MiniFltExceptionFilter(
	_In_ PEXCEPTION_POINTERS ExceptionPointer,
	_In_ BOOL AccessingUserBuffer
)
{
	NTSTATUS Status = ExceptionPointer->ExceptionRecord->ExceptionCode;

	if (!FsRtlIsNtstatusExpected(Status) && !AccessingUserBuffer) return EXCEPTION_CONTINUE_SEARCH;

	return EXCEPTION_EXECUTE_HANDLER;
}

ULONG MyWideCharToChar(
	_In_opt_ PWCHAR pWBuf,
	_Out_opt_ PCHAR pBuf,
	_In_ ULONG MaxLen
)
{
	ULONG SourceLen;
	UNICODE_STRING UniStr;
	ANSI_STRING AnStr;

	if (!pWBuf || !pBuf) return 0;

	*pBuf = 0;

	RtlInitUnicodeString(&UniStr, pWBuf);
	RtlUnicodeStringToAnsiString(&AnStr, &UniStr, TRUE);

	SourceLen = MyStrNCopy(pBuf, AnStr.Buffer, MaxLen);
	RtlFreeAnsiString(&AnStr);

	return SourceLen;
}

ULONG MyCharToWideChar(
	_In_opt_ PCHAR pBuf,
	_Out_opt_ PWCHAR pWBuf,
	_In_ ULONG MaxLen
)
{
	ULONG SourceLen;
	UNICODE_STRING UniStr;
	ANSI_STRING AnStr;

	if (!pBuf || !pWBuf) return 0;

	*pWBuf = 0;

	RtlInitAnsiString(&AnStr, pBuf);
	RtlAnsiStringToUnicodeString(&UniStr, &AnStr, TRUE);

	SourceLen = MyStrNCopyW(pWBuf, UniStr.Buffer, UniStr.Length / 2, MaxLen);
	RtlFreeUnicodeString(&UniStr);

	return SourceLen;
}

BOOL CheckRecycle(
	_In_opt_ PWCHAR ObjPathW,
	_In_opt_ PCHAR ObjPath
)
{
	BOOL bRet = FALSE;
	PWCHAR TmpPathW = NULL;

	if (!ObjPathW) {
		if (!ObjPath) return FALSE;

		TmpPathW = MyAllocNonPagedPool(NonPagedPool, MAX_KPATH * 2, &g_NonPagedPoolCnt);
		if (TmpPathW) MyCharToWideChar(ObjPath, TmpPathW, MAX_KPATH);
		else return FALSE;

		ObjPathW = TmpPathW;
	}

	if (!_wcsnicmp(ObjPathW, L"$RECYCLE", 8)) bRet = TRUE;

	MyFreeNonPagedPool(TmpPathW, &g_NonPagedPoolCnt);

	return bRet;
}