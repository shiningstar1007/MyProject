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