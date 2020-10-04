#ifndef __ACCESSCONTROL_H__
#define __ACCESSCONTROL_H__

#include "CommFunc.h"

typedef enum _TYPE_OBJECT {
	OBJECT_FILE = 0,
	OBJECT_DIR,
	OBJECT_UNKNOWN
} TYPE_OBJECT, * PTYPE_OBJECT;

typedef enum _TYPE_SUBJECT {
	SUBJECT_USER = 0,
	SUBJECT_GROUP,
	SUBJECT_PROC,
	SUBJECT_UNKNOWN
} TYPE_SUBJECT, * PTYPE_SUBJECT;

typedef struct _ACL_SUBJECT ACL_SUBJECT, *PACL_SUBJECT;
struct _ACL_SUBJECT
{
	CHAR SubjectName[MAX_KPATH];
	ULONG Permissions;
	USERSID UserSId;

	PACL_SUBJECT NextSubjectLink;
};

typedef struct _ACL_OBJECT ACL_OBJECT, * PACL_OBJECT;
struct _ACL_OBJECT
{
	CHAR ObjectName[MAX_KPATH];
	ULONG Permissions;


	PACL_SUBJECT SubjectLink;

	PACL_OBJECT NextObjectLink;
};

typedef struct _ACL_ENTRIES  ACL_ENTRIES, *PACL_ENTRIES;
struct _ACL_ENTRIES
{
	CHAR EntriesName[MAX_NAME];


};

extern PACL_OBJECT g_FirstObject, g_LastObject;

extern PACL_SUBJECT g_FirstSubject, g_LastSubject;

#endif