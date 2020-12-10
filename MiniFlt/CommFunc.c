#include "Stdafx.h"

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

static const int YearLengths[2] = { DAYSPERNORMALYEAR, DAYSPERLEAPYEAR };
static const int MonthLengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

VOID KrnlTimeToSysTime(
	_In_ LONGLONG KrnlTime,
	_Inout_ PSYSTEMTIME SysTime
)
{
	const int* Months;
	int LeapSecondCorrections, SecondsInDay, CurYear, LeapYear, CurMonth, Days;

	SysTime->wMilliseconds = (USHORT)((KrnlTime % TICKSPERSEC) / TICKSPERMSEC);

	KrnlTime = KrnlTime / TICKSPERSEC;

	LeapSecondCorrections = 0;
	Days = (int)(KrnlTime / SECSPERDAY);
	SecondsInDay = (int)(KrnlTime % SECSPERDAY);
	SecondsInDay += LeapSecondCorrections;
	while (SecondsInDay < 0) {
		SecondsInDay += SECSPERDAY;
		Days--;
	}
	while (SecondsInDay >= SECSPERDAY) {
		SecondsInDay -= SECSPERDAY;
		Days++;
	}

	SysTime->wHour = (USHORT)(SecondsInDay / SECSPERHOUR);
	SecondsInDay = SecondsInDay % SECSPERHOUR;

	SysTime->wMinute = (USHORT)(SecondsInDay / SECSPERMIN);
	SysTime->wSecond = (USHORT)(SecondsInDay % SECSPERMIN);
	SysTime->wDayOfWeek = (USHORT)((EPOCHWEEKDAY + Days) % DAYSPERWEEK);

	for (CurYear = EPOCHYEAR; ; CurYear++) {
		LeapYear = (CurYear % 4 == 0 && (CurYear % 100 != 0 || CurYear % 400 == 0)) ? 1 : 0;
		if (Days < YearLengths[LeapYear]) break;

		Days = Days - YearLengths[LeapYear];
	}
	SysTime->wYear = (USHORT)CurYear;

	Months = MonthLengths[LeapYear];
	for (CurMonth = 0; Days >= Months[CurMonth]; CurMonth++) {
		Days = Days - Months[CurMonth];
	}
	SysTime->wMonth = (USHORT)(CurMonth + 1);
	SysTime->wDay = (USHORT)(Days + 1);
}

LARGE_INTEGER GetKernelTime(
	_Inout_ PSYSTEMTIME SysTime
)
{
	LARGE_INTEGER Time, SystemTime;

	KeQuerySystemTime(&SystemTime);
	ExSystemTimeToLocalTime(&SystemTime, &Time);

	if (SysTime != NULL) KrnlTimeToSysTime(Time.QuadPart, SysTime);

	return Time;
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
	RTL_OSVERSIONINFOEXW VersionInfo;

	RtlGetVersion(&VersionInfo);

	if (VersionInfo.dwMajorVersion == 6) {
		if (VersionInfo.dwMinorVersion == 0) g_WinVersion = WV_2008;
		else if (VersionInfo.dwMinorVersion == 1) g_WinVersion = WV_2008R2;
		else if (VersionInfo.dwMinorVersion == 2) g_WinVersion = WV_2012;
		else if (VersionInfo.dwMinorVersion == 3) g_WinVersion = WV_2012R2;
	}
	else if (VersionInfo.dwMajorVersion == 10) {
		if (VersionInfo.dwMinorVersion == 0) {
			if (VersionInfo.dwBuildNumber >= WV_WIN10_20H2) g_WinVersion = WV_WIN10_20H2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_20H1) g_WinVersion = WV_WIN10_20H1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_19H2) g_WinVersion = WV_WIN10_19H2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_19H1) g_WinVersion = WV_WIN10_19H1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS5) g_WinVersion = WV_WIN10_RS5;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS4) g_WinVersion = WV_WIN10_RS4;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS3) g_WinVersion = WV_WIN10_RS3;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS2) g_WinVersion = WV_WIN10_RS2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS1) g_WinVersion = WV_WIN10_RS1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_TH2) g_WinVersion = WV_WIN10_TH2;
			else g_WinVersion = WV_WIN10;
		}
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

VOID GetProcName(
	_In_opt_ PEPROCESS pProcess,
	_Out_opt_ PCHAR ProcName
)
{
	if (!ProcName) return;

	if (pProcess && g_ProcNameOffset > 0) {
		__try {
			MyStrNCopy(ProcName, (PCHAR)pProcess + g_ProcNameOffset, NT_PROCNAMELEN - 1);
			ProcName[NT_PROCNAMELEN - 1] = 0;

			return;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
	}

	MyStrNCopy(ProcName, SYSNAME, NT_PROCNAMELEN);
}

ULONG GetProcessFullPath(
	_In_opt_ PEPROCESS pProcess,
	_Out_opt_ PWCHAR ProcPathW
)
{
	ULONG_PTR ProcAddress = (ULONG_PTR)pProcess;
	ULONG Len = 0;
	CHAR ProcName[NT_PROCNAMELEN];
	PWCHAR TmpPathW = ProcPathW;
	PUNICODE_STRING pUniStr;

	GetProcName(pProcess, ProcName);
	if (!_stricmp(ProcName, SYSNAME)) {
		if (ProcPathW) Len = MySNPrintfW(ProcPathW, MAX_KPATH, L"%s", SYSNAMEW);

		return Len;
	}

	if (!TmpPathW) TmpPathW = MyAllocNonPagedPool(MAX_KPATH * sizeof(WCHAR), &g_NonPagedPoolCnt);

	if (TmpPathW) *TmpPathW = 0;
	else return 0;

	__try {
		do {
			if ((ProcAddress == 0) || (ProcAddress == -1)) break;
			ProcAddress += g_PebOffset;
			if ((ProcAddress = *(ULONG_PTR*)ProcAddress) == 0) break;
			if ((ProcAddress == 0) || (ProcAddress == -1)) break;

			ProcAddress += PARAMETERS_OFFSET;
			if ((ProcAddress = *(ULONG_PTR*)ProcAddress) == 0) break;
			if ((ProcAddress == 0) || (ProcAddress == -1)) break;

			pUniStr = (PUNICODE_STRING)(ProcAddress + IMAGEPATH_OFFSET);
			MyStrNCopyW(TmpPathW, pUniStr->Buffer, (pUniStr->Length / 2), MAX_KPATH);

		} while (FALSE);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Len = 0;
	}

	if (TmpPathW != ProcPathW) MyFreeNonPagedPool(TmpPathW, &g_NonPagedPoolCnt);

	return Len;
}


QUERY_INFORMATION_PROCESS ZwQueryInformationProcess = NULL;
NTSTATUS InitZwQueryInformationProcess()
{
	if (!ZwQueryInformationProcess) {
		UNICODE_STRING RoutineName;
		RtlInitUnicodeString(&RoutineName, L"ZwQueryInformationProcess");

		ZwQueryInformationProcess = (QUERY_INFORMATION_PROCESS)MmGetSystemRoutineAddress(&RoutineName);

		if (!ZwQueryInformationProcess) {
			return STATUS_UNSUCCESSFUL;
		}
	}

	return STATUS_SUCCESS;
}

ULONG GetSessionId()
{
	NTSTATUS Status;
	HANDLE hProcess = NULL;
	CLIENT_ID ClientId = { 0 };
	OBJECT_ATTRIBUTES	ObjAttr = { 0 };
	PROCESS_SESSION_INFORMATION ProcSession;
	ULONG RetByte;

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;

	InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	ClientId.UniqueProcess = PsGetCurrentProcessId();

	Status = ZwOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION, &ObjAttr, &ClientId);
	if (Status != STATUS_SUCCESS || !hProcess) return 0;

	Status = ZwQueryInformationProcess(hProcess, ProcessSessionInformation,
		&ProcSession, sizeof(PROCESS_SESSION_INFORMATION), &RetByte);

	ZwClose(hProcess);

	if (Status == STATUS_SUCCESS) return ProcSession.SessionId;
	else return 0;
}

ULONG ProcessGetIdByHandle(HANDLE hProcess)
{
	NTSTATUS Status;
	PROCESS_BASIC_INFORMATION ProcBasicInfo;
	ULONG Len;

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;

	Status = ZwQueryInformationProcess(hProcess, ProcessBasicInformation,
		&ProcBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), &Len);

	if (NT_SUCCESS(Status)) return (ULONG)ProcBasicInfo.UniqueProcessId;

	return 0;
}

ULONG ProcessGetPathBy(PMINIFLT_INFO MiniFltInfo, HANDLE hProcess, ULONG ProcessId)
{
	PEPROCESS pProcess = NULL;
	BOOL bRet = FALSE;
	ULONG Len = 0;

	if (hProcess) ProcessId = ProcessGetIdByHandle(hProcess);

	if (PsLookupProcessByProcessId(UlongToPtr(ProcessId), &pProcess) == STATUS_SUCCESS) {
		if (pProcess != NULL) {
			KAPC_STATE APCState;

			KeStackAttachProcess((PRKPROCESS)pProcess, &APCState);
			Len = GetProcessFullPath(pProcess, MiniFltInfo->ProcNameW);
			KeUnstackDetachProcess(&APCState);
			ObDereferenceObject(pProcess);
		}
	}

	return Len;
}

ULONG ProcessGetParentPId(ULONG ProcessId)
{
	NTSTATUS Status;
	HANDLE hProcess = NULL;
	OBJECT_ATTRIBUTES	ObjAttr = { 0 };
	CLIENT_ID ClientId = { 0 };
	PROCESS_BASIC_INFORMATION BasicInfo;
	ULONG ParentPId = 0, BufSize = sizeof(PROCESS_BASIC_INFORMATION);

	if (ProcessId == 0) return 0;

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;

	InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	ClientId.UniqueProcess = UlongToHandle(ProcessId);

	Status = ZwOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION, &ObjAttr, &ClientId);
	if (Status != STATUS_SUCCESS || hProcess == NULL) return 0;

	if (ZwQueryInformationProcess(hProcess, ProcessBasicInformation, &BasicInfo,
		BufSize, &BufSize) == STATUS_SUCCESS) {
		ParentPId = (ULONG)BasicInfo.InheritedFromUniqueProcessId;
	}

	ZwClose(hProcess);

	return ParentPId;
}

ULONG ProcessGetParentPath(ULONG ProcessId, PWCHAR ParentPathW)
{
	NTSTATUS Status;
	PEPROCESS pProcess = NULL;
	ULONG ParentPId = ProcessGetParentPId(ProcessId);

	if (ParentPId == 0) return 0;

	Status = PsLookupProcessByProcessId(UlongToPtr(ParentPId), &pProcess);
	if (Status == STATUS_SUCCESS && pProcess != NULL) {
		KAPC_STATE APCState;

		KeStackAttachProcess((PRKPROCESS)pProcess, &APCState);
		GetProcessFullPath(pProcess, ParentPathW);
		KeUnstackDetachProcess(&APCState);
		ObDereferenceObject(pProcess);
	}

	return ParentPId;
}

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

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;

	pProcess = FltGetRequestorProcess(Data);
	if (pProcess) {
		Status = ObOpenObjectByPointer(pProcess, OBJ_KERNEL_HANDLE, NULL, GENERIC_READ, 0, KernelMode, &hProcess);
		if (!NT_SUCCESS(Status)) {
			DbgPrint("[TEST] ObOpenObjectByPointer failed [0x%X]", Status);
			return Status;
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
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS pProcess = NULL;
	PUNICODE_STRING pUniString = NULL;
	PCHAR ProcName = NULL;

	PAGED_CODE();

	pProcess = FltGetRequestorProcess(Data);
	if (pProcess) {
		Status = SeLocateProcessImageName(pProcess, &pUniString); // Windows Vista and later only.
		if (NT_SUCCESS(Status)) {
			ProcName = MyAllocNonPagedPool(pUniString->Length / 2, &g_NonPagedPoolCnt);
			if (ProcName != NULL) {
				MyWideCharToChar(pUniString->Buffer, MiniFltInfo->ProcName, pUniString->Length / 2);
			}
			ExFreePool(pUniString);
		}
		else DbgPrint("SeLocateProcessImageName failed [0x%X]", Status);
	}

	return Status;
}

DEFINE_GUID(GUID_ECP_SRV_OPEN,
	0xbebfaebc,
	0xaabf,
	0x489d,
	0x9d, 0x2c, 0xe9, 0xe3, 0x61, 0x10, 0x28, 0x53);

NTSTATUS GetSharedRemoteIP(
	_In_ PFLT_FILTER Filter,
	_Inout_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status;
	PECP_LIST pEcpList = NULL;
	PSRV_OPEN_ECP_CONTEXT ECPContext = NULL;
	GUID LpcGUID = GUID_ECP_SRV_OPEN;
	ULONG ContextSize, IPAddr;
	PSOCKADDR_IN SockAddr;

	Status = FltGetEcpListFromCallbackData(Filter, Data, &pEcpList);
	if (NT_SUCCESS(Status)) {
		if (pEcpList) {
			Status = FltFindExtraCreateParameter(Filter, pEcpList, &LpcGUID, &ECPContext, &ContextSize);
			if (NT_SUCCESS(Status)) {
				if (ECPContext) {
					DbgPrint("[TEST] FltGetNextExtraCreateParameter Success");
					SockAddr = (PSOCKADDR_IN)ECPContext->SocketAddress;
					IPAddr = SockAddr->sin_addr.S_un.S_addr;
					DbgPrint("[TEST] ECPContext IP[%u.%u.%u.%u],Port[%d]", (UCHAR)IPAddr, (IPAddr >> 8) & 0x00ff, (IPAddr >> 16) & 0x0000ff, (IPAddr >> 24) & 0x000000ff, SockAddr->sin_port);
				}
			}
			else DbgPrint("[TEST] FltGetNextExtraCreateParameter failed [0x%X]", Status);
		}
	}
	else DbgPrint("[TEST] FltGetEcpListFromCallbackData failed [0x%X]", Status);

	return Status;
}

NTSTATUS GetUserName(
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
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
	PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;

	if (Data == NULL) {
		SubjectContext = MyAllocNonPagedPool(sizeof(SECURITY_SUBJECT_CONTEXT), &g_NonPagedPoolCnt);
		if (SubjectContext == NULL) return Status;

		SeCaptureSubjectContext(SubjectContext);
	}
	else {
		SubjectContext = &(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext);
	}

	pToken = SeQuerySubjectContextToken(SubjectContext);
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
					MyWideCharToChar(UniName.Buffer, MiniFltInfo->UserName, NameSize);
				}
				else DbgPrint("[TEST] SecLookupAccountSid failed [0x%X]", Status);

				if (UniName.Buffer) MyFreeNonPagedPool(UniName.Buffer, &g_NonPagedPoolCnt);

				// if (UniDomain.Buffer) MyFreeNonPagedPool(UniDomain.Buffer, &g_NonPagedPoolCnt);
			}
		}

		if (pUser) ExFreePool(pUser);
	}

	if (Data == NULL) {
		SeReleaseSubjectContext(SubjectContext);
		MyFreeNonPagedPool(SubjectContext, &g_NonPagedPoolCnt);
	}

	return Status;
}

NTSTATUS GetGroupName(
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
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
	PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;

	if (Data == NULL) {
		SubjectContext = MyAllocNonPagedPool(sizeof(SECURITY_SUBJECT_CONTEXT), &g_NonPagedPoolCnt);
		if (SubjectContext == NULL) return Status;

		SeCaptureSubjectContext(SubjectContext);
	}
	else {
		SubjectContext = &(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext);
	}

	pToken = SeQuerySubjectContextToken(SubjectContext);
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
					MyWideCharToChar(UniName.Buffer, MiniFltInfo->GroupName, NameSize);
				}
				else DbgPrint("[TEST] SecLookupAccountSid failed [0x%X]", Status);

				if (UniName.Buffer) MyFreeNonPagedPool(UniName.Buffer, &g_NonPagedPoolCnt);

				// if (UniDomain.Buffer) MyFreeNonPagedPool(UniDomain.Buffer, &g_NonPagedPoolCnt);
			}
		}

		if (pGroups) ExFreePool(pGroups);
	}

	if (Data == NULL) {
		SeReleaseSubjectContext(SubjectContext);
		MyFreeNonPagedPool(SubjectContext, &g_NonPagedPoolCnt);
	}

	return Status;
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

VOID GetUserSId(
	_Out_opt_ PUSERSID UserSId
)
{
	PTOKEN_USER pTokenUser = NULL;
	PACCESS_TOKEN AccessToken = PsReferencePrimaryToken(PsGetCurrentProcess());
	ULONG MinSize = sizeof(USERSID) + sizeof(TOKEN_USER);

	if (!AccessToken || !UserSId) return;
	else memset(UserSId, 0, sizeof(USERSID));

	pTokenUser = (PTOKEN_USER)MyQueryInformationToken(AccessToken, TokenUser, MinSize);
	if (pTokenUser) {
		memcpy(UserSId, pTokenUser->User.Sid, sizeof(USERSID));
		MyFreeNonPagedPool(pTokenUser, &g_NonPagedPoolCnt);
	}
}

ULONG GetGroupSId(
	_Out_ PUSERSID GroupSId
)
{
	PTOKEN_GROUPS pTokenGroup = NULL;
	ULONG MinSize = sizeof(USERSID) + sizeof(TOKEN_USER);
	PACCESS_TOKEN AccessToken = PsReferencePrimaryToken(PsGetCurrentProcess());
	ULONG GroupCount = 0;

	if (!AccessToken) return GroupCount;

	pTokenGroup = (PTOKEN_GROUPS)MyQueryInformationToken(AccessToken, TokenGroups, MinSize);
	if (pTokenGroup) {
		if (GroupSId == NULL) {
			GroupSId = MyAllocNonPagedPool(sizeof(USERSID) * pTokenGroup->GroupCount, &g_NonPagedPoolCnt);
			if (GroupSId) {
				GroupCount = pTokenGroup->GroupCount;
				memset(GroupSId, 0, (size_t)(sizeof(USERSID) * GroupCount));
				for (ULONG i = 0; i < pTokenGroup->GroupCount; i++) {
					memcpy(&GroupSId[i], pTokenGroup->Groups[i].Sid, sizeof(USERSID));
				}
			}
		}

		MyFreeNonPagedPool(pTokenGroup, &g_NonPagedPoolCnt);
	}

	return GroupCount;
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

BOOL CheckLocalUserSecSub(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PACCESS_TOKEN pToken = NULL;
	PTOKEN_SOURCE pSource = NULL;
	BOOL bLocalUser = TRUE;
	PSECURITY_SUBJECT_CONTEXT SubjectContext = NULL;
	
	if (Data == NULL) {
		SubjectContext = MyAllocNonPagedPool(sizeof(SECURITY_SUBJECT_CONTEXT), &g_NonPagedPoolCnt);
		if (SubjectContext == NULL) return bLocalUser;

		SeCaptureSubjectContext(SubjectContext);
	}
	else {
		SubjectContext = &(Data->Iopb->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext);
	}

	pToken = SeQuerySubjectContextToken(SubjectContext);
	if (pToken) {
		Status = SeQueryInformationToken(pToken, TokenSource, &pSource);

		if (NT_SUCCESS(Status)) {
			bLocalUser = !_strnicmp(pSource->SourceName, "NTLMSSP", 7);
		}

		if (pSource != NULL) ExFreePool(pSource);
	}

	if (Data == NULL) {
		SeReleaseSubjectContext(SubjectContext);
		MyFreeNonPagedPool(SubjectContext, &g_NonPagedPoolCnt);
	}

	return bLocalUser;
}

ULONG GetAction(
	_In_ ACCESS_MASK AccessMask
)
{
	ACCESS_MASK TmpAccessMask;
	ULONG Action = 0;

	if ((AccessMask == FILE_ANY_ACCESS) || (AccessMask == FILE_ALL_ACCESS)) {
		Action = ACTION_READ | ACTION_WRITE | ACTION_TRAVERSE;
	}
	else if (AccessMask & DELETE) Action = ACTION_DELETE;
	else {
		TmpAccessMask = FILE_READ_ACCESS | FILE_READ_EA | FILE_READ_ATTRIBUTES | GENERIC_READ;
		if (AccessMask & TmpAccessMask) Action |= ACTION_READ;

		TmpAccessMask = FILE_WRITE_ACCESS | FILE_APPEND_DATA | FILE_WRITE_EA |
			FILE_WRITE_ATTRIBUTES | GENERIC_WRITE;
		if (AccessMask & TmpAccessMask) Action |= ACTION_WRITE;

		TmpAccessMask = FILE_LIST_DIRECTORY | FILE_TRAVERSE;
		if (AccessMask & TmpAccessMask) Action |= ACTION_TRAVERSE;
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

	UNREFERENCED_PARAMETER(ClientPort);
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
	UNREFERENCED_PARAMETER(OutBuf);


	*RetLen = 0;

	if ((InBuf != NULL) && (InBufSize >= (FIELD_OFFSET(COMMAND_DATA, Command) + sizeof(MY_COMMAND)))) {
		__try {
			Command = ((PCOMMAND_DATA)InBuf)->Command;
			Data = (PWCHAR)((PCOMMAND_DATA)InBuf)->Data;
		}
		__except (MiniFltExceptionFilter(GetExceptionInformation(), TRUE)) {
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

		TmpPathW = MyAllocNonPagedPool(MAX_KPATH * 2, &g_NonPagedPoolCnt);
		if (TmpPathW) MyCharToWideChar(ObjPath, TmpPathW, MAX_KPATH);
		else return FALSE;

		ObjPathW = TmpPathW;
	}

	if (!_wcsnicmp(ObjPathW, L"$RECYCLE", 8)) bRet = TRUE;

	MyFreeNonPagedPool(TmpPathW, &g_NonPagedPoolCnt);

	return bRet;
}

NTSTATUS ReadRegDWORD(
	_In_ PUNICODE_STRING KeyName,
	_In_ PCWSTR ValName,
	_Out_ PULONG RetVal
)
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES ObjAttr;
	HANDLE RegKey;
	UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(LONG)];
	ULONG RetLen;
	UNICODE_STRING UniValName;

	RtlInitUnicodeString(&UniValName, ValName);

	InitializeObjectAttributes(&ObjAttr, KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenKey(&RegKey, KEY_READ, &ObjAttr);
	if (!NT_SUCCESS(Status)) {
		DbgPrint("ReadRegDWORD : ZwOpenKey failed:0x%X", Status);

		return Status;
	}

	Status = ZwQueryValueKey(RegKey, &UniValName, KeyValuePartialInformation, Buffer, sizeof(Buffer), &RetLen);
	if (NT_SUCCESS(Status)) *RetVal = *((PULONG) & (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));

	ZwClose(RegKey);

	return Status;
}

NTSTATUS ReadRegStringA(
	_In_ PUNICODE_STRING KeyName,
	_In_ PCWSTR ValName,
	_Out_ PCHAR RetVal,
	_In_ ULONG ValLen,
	_Out_ PULONG ActualLen
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES ObjAttr;
	HANDLE RegKey = NULL;
	UNICODE_STRING UniValName, UniVal;
	ANSI_STRING AnsiVal = { 0 };
	PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = NULL;
	ULONG RetLen;
	ULONG Len = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + ValLen * sizeof(WCHAR);

	PartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)MyAllocNonPagedPool(Len, &g_NonPagedPoolCnt);
	if (!PartialInfo) return STATUS_INSUFFICIENT_RESOURCES;

	InitializeObjectAttributes(&ObjAttr, KeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenKey(&RegKey, KEY_READ, &ObjAttr);
	if (!NT_SUCCESS(Status)) {
		DbgPrint("ZwOpenKey failed : 0x%X", Status);
		MyFreeNonPagedPool(PartialInfo, &g_NonPagedPoolCnt);

		return Status;
	}

	RtlInitUnicodeString(&UniValName, ValName);
	Status = ZwQueryValueKey(RegKey, &UniValName, KeyValuePartialInformation, PartialInfo, Len, &RetLen);
	if (NT_SUCCESS(Status)) {
		if ((ValLen * sizeof(WCHAR)) >= PartialInfo->DataLength) {
			*(PWCHAR)(PartialInfo->Data + PartialInfo->DataLength) = UNICODE_NULL;
			RtlInitUnicodeString(&UniVal, (PWCHAR)(PartialInfo->Data));
			RtlUnicodeStringToAnsiString(&AnsiVal, &UniVal, TRUE);

			RtlCopyMemory(RetVal, AnsiVal.Buffer, AnsiVal.Length);
			RetVal[AnsiVal.Length] = ANSI_NULL;

			if (ActualLen != NULL) *ActualLen = AnsiVal.Length;
		}
	}
	else DbgPrint("ZwQueryValueKey failed : 0x%X", Status);

	if (RegKey != NULL) ZwClose(RegKey);

	if (AnsiVal.Buffer != NULL) RtlFreeAnsiString(&AnsiVal);

	if (PartialInfo != NULL) MyFreeNonPagedPool(PartialInfo, &g_NonPagedPoolCnt);

	return Status;
}

NTSTATUS GetRegSubKeyName(
	_In_ PUNICODE_STRING RootKeyName,
	_In_ ULONG Index,
	_In_ PUNICODE_STRING KeyName
)
{
	OBJECT_ATTRIBUTES ObjAttr;
	HANDLE RegKey = NULL;
	PKEY_BASIC_INFORMATION KeyInfo = NULL;
	ULONG Len;
	NTSTATUS Status;

	InitializeObjectAttributes(&ObjAttr, RootKeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenKey(&RegKey, KEY_READ, &ObjAttr);
	if (!NT_SUCCESS(Status)) return Status;

	Len = sizeof(KEY_BASIC_INFORMATION) + (MAX_KPATH * sizeof(WCHAR));
	KeyInfo = MyAllocNonPagedPool(Len, &g_NonPagedPoolCnt);
	if (KeyInfo == NULL) {
		ZwClose(RegKey);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Status = ZwEnumerateKey(RegKey, Index, KeyBasicInformation, KeyInfo, Len, &Len);
	if (!NT_SUCCESS(Status))
	{
		if (Status != STATUS_NO_MORE_ENTRIES) DbgPrint("ZwEnumerateKey failed : 0x%X", Status);
	}
	else {
		size_t cbToCopy = (size_t)KeyInfo->NameLength;

		Status = RtlUnicodeStringCopy(KeyName, RootKeyName);
		if (NT_SUCCESS(Status)) {
			Status = RtlUnicodeStringCatString(KeyName, L"\\");
			if (NT_SUCCESS(Status)) {
				Status = RtlUnicodeStringCbCatStringN(KeyName, KeyInfo->Name, cbToCopy);
			}
			else DbgPrint("RtlUnicodeStringCatString failed : 0x%X", Status);
		}
		else DbgPrint("RtlUnicodeStringCopy failed : 0x%X", Status);
	}

	if (KeyInfo != NULL) MyFreeNonPagedPool(KeyInfo, &g_NonPagedPoolCnt);

	if (RegKey != NULL) ZwClose(RegKey);

	return Status;
}

NTSTATUS PsKeReadRegEnumString(
	_In_ PUNICODE_STRING RootKeyName,
	_In_ PCWSTR ValName
)
{
	NTSTATUS Status;
	UNICODE_STRING SubKeyName;
	OBJECT_ATTRIBUTES ObjAttr;
	ULONG Index = 0;
	PKEY_VALUE_PARTIAL_INFORMATION ValInfo = NULL;
	ULONG ValLen;
	HANDLE Key;

	SubKeyName.Buffer = (PWCHAR)MyAllocNonPagedPool(MAX_KPATH * sizeof(WCHAR), &g_NonPagedPoolCnt);
	SubKeyName.MaximumLength = MAX_KPATH * sizeof(WCHAR);
	if (SubKeyName.Buffer == NULL) return STATUS_INSUFFICIENT_RESOURCES;

	ValLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + (MAX_KPATH * sizeof(WCHAR));
	ValInfo = MyAllocNonPagedPool(ValLen, &g_NonPagedPoolCnt);
	if (ValInfo == NULL) {
		DbgPrint("ValInfo allocation failed");
		MyFreeNonPagedPool(SubKeyName.Buffer, &g_NonPagedPoolCnt);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	for (;;) {
		Key = NULL;

		Status = GetRegSubKeyName(RootKeyName, Index, &SubKeyName);
		if (!NT_SUCCESS(Status)) {
			if (Status == STATUS_NO_MORE_ENTRIES) Status = STATUS_SUCCESS;

			break;
		}

		InitializeObjectAttributes(&ObjAttr, &SubKeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		Status = ZwOpenKey(&Key, KEY_READ, &ObjAttr);
		if (NT_SUCCESS(Status)) {
			UNICODE_STRING UniValName;
			ULONG RetLen;

			RtlInitUnicodeString(&UniValName, ValName);
			Status = ZwQueryValueKey(Key, &UniValName, KeyValuePartialInformation, ValInfo, ValLen, &RetLen);
			if (NT_SUCCESS(Status)) {
				DbgPrint("Data[%S]", (PWCHAR)ValInfo->Data);
			}

			ZwClose(Key);
		}
		else DbgPrint("ZwOpenKey failed 0x:%X", Status);

		Index++;
	}

	if (SubKeyName.Buffer != NULL) MyFreeNonPagedPool(SubKeyName.Buffer, &g_NonPagedPoolCnt);

	if (ValInfo != NULL) MyFreeNonPagedPool(ValInfo, &g_NonPagedPoolCnt);

	return Status;
}