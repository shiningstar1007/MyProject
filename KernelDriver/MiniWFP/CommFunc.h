#ifndef __COMMFUNC_H__
#define __COMMFUNC_H__

#define TAG_WFPFLT 'PFWM'

extern LONG g_NonPagedPoolCnt;
extern LONG g_InjectNPPoolCnt;
PVOID MyAllocNonPagedPool(
	_In_ ULONG BufSize,
	_Inout_ PLONG MemCnt
);

PVOID MyFreeNonPagedPool(
	_Inout_ PVOID Buffer,
	_Inout_ PLONG MemCnt
);

#endif

