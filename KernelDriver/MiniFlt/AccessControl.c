#include "StdAfx.h"

LONG g_ACLNonPagedPoolCnt = 0;

ONOFF_MODE StrToOnOffMode(
	_In_ PCHAR OnOffModeStr
)
{
	if (!_stricmp(OnOffModeStr, OFM_ON_STR)) return OFM_ON;

	return OFM_OFF;
}

PCHAR OnOffModeToStr(
	_In_ ONOFF_MODE OnOffMode
)
{
	if (OnOffMode == OFM_ON) return OFM_ON_STR;

	return OFM_OFF_STR;
}

TYPE_SUBJECT StrToSubType(
	_In_ PCHAR SubTypeStr
)
{
	if (!_stricmp(SubTypeStr, SUBJECT_USER_STR)) return SUBJECT_USER;
	else if (!_stricmp(SubTypeStr, SUBJECT_GROUP_STR)) return SUBJECT_GROUP;
	else if (!_stricmp(SubTypeStr, SUBJECT_PROC_STR)) return SUBJECT_PROC;

	return SUBJECT_UNKNOWN;
}

PCHAR SubTypeToStr(
	_In_ TYPE_SUBJECT SubType
)
{
	if (SubType == SUBJECT_USER) return SUBJECT_USER_STR;
	if (SubType == SUBJECT_GROUP) return SUBJECT_GROUP_STR;
	else if (SubType == SUBJECT_PROC) return SUBJECT_PROC_STR;

	return NULL;
}

TYPE_OBJECT StrToObjType(
	_In_ PCHAR ObjTypeStr
)
{
	if (!_stricmp(ObjTypeStr, OBJECT_FILE_STR)) return OBJECT_FILE;
	else if (!_stricmp(ObjTypeStr, OBJECT_DIR_STR)) return OBJECT_DIR;

	return OBJECT_UNKNOWN;
}

PCHAR ObjTypeToStr(
	_In_ TYPE_OBJECT ObjType
)
{
	if (ObjType == OBJECT_FILE) return OBJECT_FILE_STR;
	else if (ObjType == OBJECT_DIR) return OBJECT_DIR_STR;

	return "";
}

EFFECT_MODE StrToEffectMode(
	_In_ PCHAR EffectModeStr
)
{
	if (!_stricmp(EffectModeStr, EFT_ALLOW_STR)) return EFT_ALLOW;
	else if (!_stricmp(EffectModeStr, EFT_DENY_STR)) return EFT_DENY;

	return EFT_UNKNOWN;
}

PCHAR EffectModeToStr(
	_In_ EFFECT_MODE EffectMode
)
{
	if (EffectMode == EFT_ALLOW) return EFT_ALLOW_STR;
	else if (EffectMode == EFT_DENY) return EFT_DENY_STR;

	return EFT_UNKNOWN_STR;
}

ULONG StrToRunMode(
	_In_ PCHAR RunModeStr
)
{
	if (!_stricmp(RunModeStr, RUN_TEST_STR)) return RUN_TEST;
	else if (!_stricmp(RunModeStr, RUN_DISABLE_STR)) return RUN_DISABLE;

	return RUN_NORMAL;
}

PCHAR RunModeToStr(
	_In_ RUN_MODE RunMode
)
{
	if (RunMode == RUN_TEST) return RUN_TEST_STR;
	else if (RunMode == RUN_DISABLE) return RUN_DISABLE_STR;

	return RUN_NORMAL_STR;
}

ULONG StrToAction(
	_In_ PCHAR ActionStr
)
{
	PCHAR Value, NextStr = ActionStr;
	ULONG Action = 0;

	if (!ActionStr || !*ActionStr) return 0;

	if (!_stricmp(ActionStr, ACTION_ALL_STR)) return ACTION_ALL;

	while (NextStr) {
		Value = MyStrTok(NextStr, SEP_COMMA, &NextStr, FALSE);
		if (!Value) break;

		if (!_stricmp(Value, ACTION_READ_STR)) Action |= ACTION_READ;
		else if (!_stricmp(Value, ACTION_WRITE_STR)) Action |= ACTION_WRITE;
		else if (!_stricmp(Value, ACTION_TRAVERSE_STR)) Action |= ACTION_TRAVERSE;
		else if (!_stricmp(Value, ACTION_EXECUTE_STR)) Action |= ACTION_EXECUTE;
		else if (!_stricmp(Value, ACTION_DELETE_STR)) Action |= ACTION_DELETE;
		else if (!_stricmp(Value, ACTION_CREATE_STR)) Action |= ACTION_CREATE;
		else if (!_stricmp(Value, ACTION_RENAME_STR)) Action |= ACTION_RENAME;
		else if (!_stricmp(Value, ACTION_KEY_CREATE_STR)) Action |= ACTION_KEY_CREATE;
		else if (!_stricmp(Value, ACTION_KEY_DELETE_STR)) Action |= ACTION_KEY_DELETE;
		else if (!_stricmp(Value, ACTION_VALUE_WRITE_STR)) Action |= ACTION_VALUE_WRITE;
		else if (!_stricmp(Value, ACTION_VALUE_DELETE_STR)) Action |= ACTION_VALUE_DELETE;
	}

	return Action;
}

PCHAR ActionToStr(
	_In_ ULONG Action,
	_Out_ PCHAR ActionStrBuf,
	_In_ ULONG MaxLen
)
{
	ULONG OffSet = 0;

	if (Action == ACTION_ALL) MyStrNCopy(ActionStrBuf, ACTION_ALL_STR, MaxLen);
	else {
		if (Action & ACTION_READ) OffSet = MySNPrintf(ActionStrBuf, MaxLen, "%s,", ACTION_READ_STR);
		if (Action & ACTION_WRITE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_WRITE_STR);
		if (Action & ACTION_TRAVERSE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_TRAVERSE_STR);
		if (Action & ACTION_EXECUTE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_EXECUTE_STR);
		if (Action & ACTION_DELETE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_DELETE_STR);
		if (Action & ACTION_CREATE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_CREATE_STR);
		if (Action & ACTION_RENAME) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_RENAME_STR);
		if (Action & ACTION_KEY_CREATE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_KEY_CREATE_STR);
		if (Action & ACTION_KEY_DELETE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_KEY_DELETE_STR);
		if (Action & ACTION_VALUE_WRITE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_VALUE_WRITE_STR);
		if (Action & ACTION_VALUE_DELETE) OffSet += MySNPrintf(ActionStrBuf + OffSet, MaxLen - OffSet, "%s,", ACTION_VALUE_DELETE_STR);

		if (OffSet == 0) *ActionStrBuf = 0;
		else ActionStrBuf[OffSet - 1] = 0;
	}

	return ActionStrBuf;
}

RUN_MODE g_ACLMode = RUN_NORMAL;
MINICODE ACLSet(
	_In_ RUN_MODE ACLRunMode
)
{
	g_ACLMode = ACLRunMode;

	return ERROR_MINI_SUCCESS;
}

MINICODE ACLMode()
{
	DbgPrint("aclmode=%s", RunModeToStr(g_ACLMode));

	return ERROR_MINI_SUCCESS;
}

PACL_DATA FindData(
	_In_ PVOID Data,
	_In_ PACL_LINK ACLLink,
	_In_ PVOID AnyData
)
{
	PACL_DATA ACLData;
	PSUB_PERM TargetPerm, ComparePerm;

	for (ACLData = ACLLink->FirstData; ACLData; ACLData = ACLData->NextData) {
		if (ACLData->Data == Data) {
			if (AnyData) {
				TargetPerm = (PSUB_PERM)ACLData->AnyData;
				ComparePerm = (PSUB_PERM)AnyData;

				if ((TargetPerm->ProcUser.SubType != ComparePerm->ProcUser.SubType) ||
					(_stricmp(TargetPerm->ProcUser.SubName, ComparePerm->ProcUser.SubName))) continue;
			}

			return ACLData;
		}
	}

	return NULL;
}

PACL_DATA AddDataEx(
	_In_ PVOID Data,
	_Inout_ PACL_LINK ACLLink,
	_In_ PVOID AnyData
)
{
	PACL_DATA ACLData = FindData(Data, ACLLink, AnyData);
	if (ACLData) return ACLData;

	ACLData = (PACL_DATA)MyAllocNonPagedPool(sizeof(ACL_DATA), &g_ACLNonPagedPoolCnt);
	if (!ACLData) return NULL;
	else memset(ACLData, 0, sizeof(ACL_DATA));

	ACLData->Data = Data;
	if (ACLLink->LinkCnt) {
		ACLLink->LastData->NextData = ACLData;
		ACLLink->LastData = ACLData;
	}
	else ACLLink->FirstData = ACLLink->LastData = ACLData;

	ACLLink->LinkCnt++;

	return ACLData;
}

PACL_DATA AddData(
	_In_ PVOID Data,
	_Inout_ PACL_LINK ACLLink
)
{
	return AddDataEx(Data, ACLLink, NULL);
}

PACL_OBJECT g_FirstObject = NULL, g_LastObject = NULL;
PACL_OBJECT SearchObject(PACL_OBJECT Object)
{
	PACL_OBJECT CompareObj;

	for (CompareObj = g_FirstObject; CompareObj; CompareObj = CompareObj->NextObjectLink) {
		if (!_stricmp(CompareObj->ObjectName, Object->ObjectName)) break;
	}

	return CompareObj;
}

MINICODE ACLObjectAdd(PACL_OBJECT Object)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_OBJECT ACLObject;

	ACLObject = SearchObject(Object);
	if (ACLObject != NULL) return ErrCode;

	ACLObject = MyAllocNonPagedPool(sizeof(ACL_OBJECT), &g_ACLNonPagedPoolCnt);
	if (ACLObject == NULL) return ERROR_NOT_ALLOC_BUF;
	else memcpy(ACLObject, Object, sizeof(ACL_OBJECT));

	if (g_FirstObject == NULL) g_FirstObject = ACLObject;
	else g_LastObject->NextObjectLink = ACLObject;

	g_LastObject = ACLObject;

	return ErrCode;
}

MINICODE ACLObjectModify(PACL_OBJECT Object)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_OBJECT ACLObject;

	ACLObject = SearchObject(Object);
	if (ACLObject == NULL) return ERROR_OBJECT_NOT_EXIST;

	return ErrCode;
}

MINICODE ACLObjectRemove(PACL_OBJECT Object)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_OBJECT ACLObject, PreACLObject = NULL;

	for (ACLObject = g_FirstObject; ACLObject; ACLObject = ACLObject->NextObjectLink) {
		if (!_stricmp(ACLObject->ObjectName, Object->ObjectName)) {
			if (g_FirstObject == ACLObject) {
				g_FirstObject = ACLObject->NextObjectLink;
				if (g_LastObject == ACLObject) {
					g_LastObject = ACLObject->NextObjectLink;
				}
			}
			else {
				PreACLObject->NextObjectLink = ACLObject->NextObjectLink;
				if (g_LastObject == ACLObject) {
					g_LastObject = PreACLObject;
				}
			}
		}
		else PreACLObject = ACLObject;
	}

	return ErrCode;
}

MINICODE ACLObjectList()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_OBJECT ACLObject;

	for (ACLObject = g_FirstObject; ACLObject; ACLObject = ACLObject->NextObjectLink) {
		DbgPrint("ObjectName[%s]", ACLObject->ObjectName);
	}

	return ErrCode;
}

VOID ACLObjectClear()
{
	PACL_OBJECT ACLObject, NextObject;

	for (ACLObject = g_FirstObject; ACLObject; ACLObject = NextObject) {
		NextObject = ACLObject->NextObjectLink;

		MyFreeNonPagedPool(ACLObject, &g_ACLNonPagedPoolCnt);
	}

	g_FirstObject = g_LastObject = NULL;
}

PACL_SUBJECT g_FirstSubject = NULL, g_LastSubject = NULL;
PACL_SUBJECT SearchSubject(PACL_SUBJECT Subject)
{
	PACL_SUBJECT CompareSub;

	for (CompareSub = g_FirstSubject; CompareSub; CompareSub = CompareSub->NextSubjectLink) {
		if (!_stricmp(CompareSub->SubjectName, Subject->SubjectName)) break;
	}

	return CompareSub;
}

MINICODE ACLSubjectAdd(PACL_SUBJECT Subject)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_SUBJECT ACLSubject;

	ACLSubject = SearchSubject(Subject);
	if (ACLSubject != NULL) return ERROR_SUBJECT_EXIST;

	ACLSubject = MyAllocNonPagedPool(sizeof(ACL_SUBJECT), &g_ACLNonPagedPoolCnt);
	if (ACLSubject == NULL) return ERROR_NOT_ALLOC_BUF;
	else memcpy(ACLSubject, Subject, sizeof(ACL_SUBJECT));

	if (g_FirstSubject == NULL) g_FirstSubject = ACLSubject;
	else g_LastSubject->NextSubjectLink = ACLSubject;

	g_LastSubject = ACLSubject;


	return ErrCode;
}

MINICODE ACLSubjectModify(PACL_SUBJECT Subject)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_SUBJECT ACLSubject;

	ACLSubject = SearchSubject(Subject);
	if (ACLSubject == NULL) return ERROR_SUBJECT_NOT_EXIST;
	

	return ErrCode;
}

MINICODE ACLSubjectRemove(PACL_SUBJECT Subject)
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_SUBJECT ACLSubject, PreACLSubject = NULL;

	for (ACLSubject = g_FirstSubject; ACLSubject; ACLSubject = ACLSubject->NextSubjectLink) {
		if (!_stricmp(ACLSubject->SubjectName, Subject->SubjectName)) {
			if (g_FirstSubject == ACLSubject) {
				g_FirstSubject = ACLSubject->NextSubjectLink;
				if (g_LastSubject == ACLSubject) {
					g_LastSubject = ACLSubject->NextSubjectLink;
				}
			}
			else {
				PreACLSubject->NextSubjectLink = ACLSubject->NextSubjectLink;
				if (g_LastSubject == ACLSubject) {
					g_LastSubject = PreACLSubject;
				}
			}
		}
		else PreACLSubject = ACLSubject;
	}


	return ErrCode;
}

MINICODE ACLSubjectList()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;
	PACL_SUBJECT ACLSubject;

	for (ACLSubject = g_FirstSubject; ACLSubject; ACLSubject = ACLSubject->NextSubjectLink) {
		DbgPrint("SubjectName[%s]", ACLSubject->SubjectName);
	}


	return ErrCode;
}

VOID ACLSubjectClear()
{
	PACL_SUBJECT ACLSubject, NextSubject;

	for (ACLSubject = g_FirstSubject; ACLSubject; ACLSubject = NextSubject) {
		NextSubject = ACLSubject->NextSubjectLink;

		MyFreeNonPagedPool(ACLSubject, &g_ACLNonPagedPoolCnt);
	}

	g_FirstSubject = g_LastSubject = NULL;
}