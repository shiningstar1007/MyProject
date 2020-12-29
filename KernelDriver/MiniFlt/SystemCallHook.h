#ifndef __SYSTEMCALLHOOK_H__
#define __SYSTEMCALLHOOK_H__

#include "CommFunc.h"

extern PVOID g_RegHandle;
OB_PREOP_CALLBACK_STATUS ObPreCallBack(
	_In_ PVOID RegContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION ObPreOperInfo
);

POB_POST_OPERATION_CALLBACK ObPostCallBack(
	_In_ PVOID RegContext,
	_Inout_ POB_POST_OPERATION_INFORMATION OperInfo
);

NTSTATUS RegisterCallbackProcess();
VOID UnRegisterCallbackProcess();

#define MAX_REGPATH 1024

typedef struct _REG_ROOTKEYW {
	WCHAR KRootName[MAX_KPATH];
	WCHAR URootName[32];
	ULONG KRootLen;
} REG_ROOTKEYW, * PREG_ROOTKEYW;

typedef struct _REG_LINKKEYW {
	WCHAR OriName[MAX_KPATH];
	WCHAR LinkName[MAX_KPATH];
	ULONG OriLen;
} REG_LINKKEYW, * PREG_LINKKEYW;

typedef struct _REG_CTRLKEYW {
	WCHAR KeyName[MAX_KPATH];
	ULONG NameLen;
} REG_CTRLKEYW, * PREG_CTRLKEYW;

extern LARGE_INTEGER g_RegisterCookie;
VOID StartRegHook(_In_ PDEVICE_OBJECT DeviceObject);
VOID StopRegHook();

#endif