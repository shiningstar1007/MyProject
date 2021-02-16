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

typedef enum _RUN_MODE {
	RUN_NORMAL = 0,
	RUN_TEST,
	RUN_DISABLE,
} RUN_MODE, * PRUN_MODE;

#define RUN_NORMAL_STR  "normal"
#define RUN_TEST_STR    "test"
#define RUN_DISABLE_STR "disable"

typedef struct _ACL_DATA ACL_DATA, * PACL_DATA;
struct _ACL_DATA {
	PVOID Data;
	PVOID AnyData;

	PACL_DATA NextData;
};

typedef struct _ACL_LINK {
	PACL_DATA FirstData;
	PACL_DATA LastData;

	ULONG LinkCnt;
} ACL_LINK, * PACL_LINK;

typedef struct _PROC_USER PROC_USER, * PPROC_USER;
struct _PROC_USER
{
	TYPE_SUBJECT SubType;
	CHAR SubName[MAX_KPATH];

	USERSID UserSId;
	CHAR UserSIdBuf[MAX_NAME];
};

typedef struct _SUB_PERM SUB_PERM, * PSUB_PERM;
struct _SUB_PERM
{
	EFFECT_MODE Effect;
	ULONG Action;

	EFFECT_MODE DecPerm;
	PROC_USER ProcUser;
};

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