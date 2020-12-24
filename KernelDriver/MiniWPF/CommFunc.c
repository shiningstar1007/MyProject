#include "Stdafx.h"

LONG g_NonPagedPoolCnt = 0;
PVOID MyAllocNonPagedPool(
	_In_ ULONG BufSize,
	_Inout_ PLONG MemCnt
)
{
	PVOID Buffer;

	if (BufSize == 0) return NULL;

	Buffer = ExAllocatePoolWithTag(NonPagedPool, BufSize, TAG_WFPFLT);
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
	ExFreePoolWithTag(Buffer, TAG_WFPFLT);

	return NULL;
}