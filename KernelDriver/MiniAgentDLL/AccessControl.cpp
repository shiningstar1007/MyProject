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
};
