#ifndef __ACCESSCONTROL_H__
#define __ACCESSCONTROL_H__

#include "CommFunc.h"

typedef enum _TYPE_OBJECT {
	OBJECT_FILE = 0,
	OBJECT_DIR,
	OBJECT_UNKNOWN
} TYPE_OBJECT, * PTYPE_OBJECT;

#define OBJECT_FILE_STR "file"
#define OBJECT_DIR_STR  "dir"

typedef enum _TYPE_SUBJECT {
	SUBJECT_USER = 0,
	SUBJECT_GROUP,
	SUBJECT_PROC,
	SUBJECT_UNKNOWN
} TYPE_SUBJECT, * PTYPE_SUBJECT;

#define SUBJECT_USER_STR  "user"
#define SUBJECT_GROUP_STR "group"
#define SUBJECT_PROC_STR  "proc"

typedef enum _EFFECT_MODE {
	EFT_DENY = 0,
	EFT_ALLOW,
	EFT_UNKNOWN
} EFFECT_MODE, * PEFFECT_MODE;

#define EFT_ALLOW_STR   "allow"
#define EFT_DENY_STR    "deny"
#define EFT_UNKNOWN_STR "unknown"

typedef enum _ONOFF_MODE {
	OFM_ON = 0,
	OFM_OFF,
} ONOFF_MODE, * PONOFF_MODE;

#define OFM_ON_STR  "on"
#define OFM_OFF_STR "off"

typedef struct _ACL_OBJECT ACL_OBJECT, * PACL_OBJECT;
typedef struct _ACL_SUBJECT ACL_SUBJECT, *PACL_SUBJECT;

struct _ACL_SUBJECT
{
	CHAR SubjectName[MAX_KPATH];
	ULONG Permissions;
	USERSID UserSId;

	PACL_OBJECT ObjectLink;

	PACL_SUBJECT NextSubjectLink;
};

struct _ACL_OBJECT
{
	CHAR ObjectName[MAX_KPATH];
	ULONG Permissions;


	PACL_SUBJECT SubjectLink;

	PACL_OBJECT NextObjectLink;
};

extern PACL_OBJECT g_FirstObject, g_LastObject;

extern PACL_SUBJECT g_FirstSubject, g_LastSubject;

#endif