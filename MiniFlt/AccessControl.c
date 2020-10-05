#include "StdAfx.h"

LONG g_ACLNonPagedPoolCnt = 0;

MINICODE ACLEntriesAdd()
{
	MINICODE ErrCode;


	return ErrCode;
}

MINICODE ACLEntriesModify()
{
	MINICODE ErrCode;


	return ErrCode;
}

MINICODE ACLEntriesDelete()
{
	MINICODE ErrCode;


	return ErrCode;
}

MINICODE ACLEntriesList()
{
	MINICODE ErrCode;


	return ErrCode;
}

PACL_OBJECT g_FirstObject = NULL, g_LastObject = NULL;
MINICODE ACLObjectAdd(PACL_OBJECT Object)
{
	MINICODE ErrCode;
	PACL_OBJECT ACLObject = MyAllocNonPagedPool(sizeof(ACL_OBJECT), &g_ACLNonPagedPoolCnt);

	if (ACLObject == NULL) return ErrCode;
	else memcpy(ACLObject, Object, sizeof(ACL_OBJECT));

	if (g_FirstObject == NULL) g_FirstObject = ACLObject;
	else g_LastObject->NextObjectLink = ACLObject;

	g_LastObject = ACLObject;

	return ErrCode;
}

MINICODE ACLObjectModify(PACL_OBJECT Object)
{
	MINICODE ErrCode;


	return ErrCode;
}

MINICODE ACLObjectDelete(PACL_OBJECT Object)
{
	MINICODE ErrCode;
	PACL_OBJECT ACLObject, PreACLObject;

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
	MINICODE ErrCode;
	PACL_OBJECT ACLObject;

	for (ACLObject = g_FirstObject; ACLObject; ACLObject = ACLObject->NextObjectLink) {
		DbgPrint("ObjectName[%s]", ACLObject->ObjectName);
	}

	return ErrCode;
}

PACL_SUBJECT g_FirstSubject = NULL, g_LastSubject = NULL;
MINICODE ACLSubjectAdd(PACL_SUBJECT Subject)
{
	MINICODE ErrCode;
	PACL_SUBJECT ACLSubject = MyAllocNonPagedPool(sizeof(ACL_SUBJECT), &g_ACLNonPagedPoolCnt);

	if (ACLSubject == NULL) return ErrCode;
	else memcpy(ACLSubject, Subject, sizeof(ACL_SUBJECT));

	if (g_FirstSubject == NULL) g_FirstSubject = ACLSubject;
	else g_LastSubject->NextSubjectLink = ACLSubject;

	g_LastSubject = ACLSubject;


	return ErrCode;
}

MINICODE ACLSubjectModify(PACL_SUBJECT Subject)
{
	MINICODE ErrCode;


	return ErrCode;
}

MINICODE ACLSubjectDelete(PACL_SUBJECT Subject)
{
	MINICODE ErrCode;
	PACL_SUBJECT ACLSubject, PreACLSubject;

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
	MINICODE ErrCode;
	PACL_SUBJECT ACLSubject;

	for (ACLSubject = g_FirstSubject; ACLSubject; ACLSubject = ACLSubject->NextSubjectLink) {
		DbgPrint("SubjectName[%s]", ACLSubject->SubjectName);
	}


	return ErrCode;
}