#include "stdafx.h"

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
	ExFreePool(Buffer);

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
		default:
			DbgPrint("Unknown Process Offset");
	}
}

NTSTATUS GetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status;
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