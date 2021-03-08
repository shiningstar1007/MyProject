#include <Windows.h>
#include <string>
using namespace std;


class AccessControl {

public :
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


	TYPE_SUBJECT StrToSubType(
		string SubTypeStr
	)
	{
		if (!SubTypeStr.compare(SUBJECT_USER_STR)) return TYPE_SUBJECT::SUBJECT_USER;
		else if (!SubTypeStr.compare(SUBJECT_GROUP_STR)) return TYPE_SUBJECT::SUBJECT_GROUP;
		else if (!SubTypeStr.compare(SUBJECT_PROC_STR)) return TYPE_SUBJECT::SUBJECT_PROC;

		return TYPE_SUBJECT::SUBJECT_UNKNOWN;
	}

	string SubTypeToStr(
		TYPE_SUBJECT SubType
	)
	{
		if (SubType == TYPE_SUBJECT::SUBJECT_USER) return SUBJECT_USER_STR;
		if (SubType == TYPE_SUBJECT::SUBJECT_GROUP) return SUBJECT_GROUP_STR;
		else if (SubType == TYPE_SUBJECT::SUBJECT_PROC) return SUBJECT_PROC_STR;

		return "";
	}

	TYPE_OBJECT StrToObjType(
		string ObjTypeStr
	)
	{
		if (!ObjTypeStr.compare(OBJECT_FILE_STR)) return TYPE_OBJECT::OBJECT_FILE;
		else if (!ObjTypeStr.compare(OBJECT_DIR_STR)) return TYPE_OBJECT::OBJECT_DIR;

		return TYPE_OBJECT::OBJECT_UNKNOWN;
	}

	string ObjTypeToStr(
		_In_ TYPE_OBJECT ObjType
	)
	{
		if (ObjType == TYPE_OBJECT::OBJECT_FILE) return OBJECT_FILE_STR;
		else if (ObjType == TYPE_OBJECT::OBJECT_DIR) return OBJECT_DIR_STR;

		return "";
	}
};
