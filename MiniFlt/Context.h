#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "CommFunc.h"

typedef struct _MINI_FLT_CONTEXT {
	CHAR ObjPath[MAX_KPATH];
	CHAR ProcPath[MAX_KPATH];
	CHAR UserName[MAX_NAME];
} MINI_FLT_CONTEXT, * PMINI_FLT_CONTEXT;

VOID MiniFltContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
);

NTSTATUS FindMiniFltContext(
	_In_ FLT_CONTEXT_TYPE ContextType,
	_In_ PFLT_CALLBACK_DATA Data,
	_Outptr_ PMINI_FLT_CONTEXT* pMiniFltContext
);

NTSTATUS CreateMiniFltContext(
	_In_ FLT_CONTEXT_TYPE ContextType,
	_In_ PFLT_CALLBACK_DATA Data,
	_In_ PFLT_FILTER hFilter,
	_Outptr_ PMINI_FLT_CONTEXT* pMiniFltContext
);

#endif