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