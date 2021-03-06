

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