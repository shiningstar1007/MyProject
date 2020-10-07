#include "StdAfx.h"

LONG g_ACLNonPagedPoolCnt = 0;

MINICODE ACLEntriesAdd()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;


	return ErrCode;
}

MINICODE ACLEntriesModify()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;


	return ErrCode;
}

MINICODE ACLEntriesRemove()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;


	return ErrCode;
}

MINICODE ACLEntriesList()
{
	MINICODE ErrCode = ERROR_MINI_SUCCESS;


	return ErrCode;
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