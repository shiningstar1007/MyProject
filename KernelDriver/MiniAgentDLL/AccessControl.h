#include "stdafx.h"

using std::string;

class AccessControl {

public:
	enum class TYPE_OBJECT {
		OBJECT_FILE = 0,
		OBJECT_DIR,
		OBJECT_UNKNOWN
	};

	const string OBJECT_FILE_STR = "file";
	const string OBJECT_DIR_STR = "dir";

	enum class TYPE_SUBJECT {
		SUBJECT_USER = 0,
		SUBJECT_GROUP,
		SUBJECT_PROC,
		SUBJECT_UNKNOWN
	};

	const string SUBJECT_USER_STR = "user";
	const string SUBJECT_GROUP_STR = "group";
	const string SUBJECT_PROC_STR = "proc";


	TYPE_SUBJECT StrToSubType(string SubTypeStr);
	string SubTypeToStr(TYPE_SUBJECT SubType);

	TYPE_OBJECT StrToObjType(string ObjTypeStr);
	string ObjTypeToStr(_In_ TYPE_OBJECT ObjType);


	enum class EFFECT_MODE {
		EFT_DENY = 0,
		EFT_ALLOW,
		EFT_UNKNOWN
	};

	const string EFT_ALLOW_STR = "allow";
	const string EFT_DENY_STR = "deny";
	const string EFT_UNKNOWN_STR = "unknown";

	EFFECT_MODE StrToEffectMode(_In_ string EffectModeStr);
	string EffectModeToStr(_In_ EFFECT_MODE EffectMode);

	enum class ACL_ACTION {
		ACTION_UNKNOWN = 0,
		ACTION_READ = 0x00000001,
		ACTION_WRITE = 0x00000002,
		ACTION_TRAVERSE = 0x0000000,
		ACTION_DELETE = 0x00000008,
		ACTION_CREATE = 0x00000010,
		ACTION_EXECUTE = 0x00000020,
		ACTION_RENAME = 0x00000040,
		ACTION_KEY_CREATE = 0x00000100,
		ACTION_KEY_DELETE = 0x00000200,
		ACTION_VALUE_WRITE = 0x00000400,
		ACTION_VALUE_DELETE = 0x00000800,
		ACTION_ALL = 0xFFFFFFFF
	};

	const string ACTION_READ_STR = "read";
	const string ACTION_WRITE_STR = "write";
	const string ACTION_TRAVERSE_STR = "traverse";
	const string ACTION_DELETE_STR = "delete";
	const string ACTION_CREATE_STR = "create";
	const string ACTION_EXECUTE_STR = "execute";
	const string ACTION_RENAME_STR = "rename";
	const string ACTION_KEY_CREATE_STR = "keycreate";
	const string ACTION_KEY_DELETE_STR = "keydel";
	const string ACTION_VALUE_WRITE_STR = "valwrite";
	const string ACTION_VALUE_DELETE_STR = "valdel";
	const string ACTION_ALL_STR = "all";

	ACL_ACTION StrToAction(_In_ string ActionStr);
	string ActionToStr(_In_ ULONG Action, _Out_ PCHAR ActionStrBuf, _In_ ULONG MaxLen);
	
};