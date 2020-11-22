#ifndef __MINIFLTLOG_H__
#define __MINIFLTLOG_H__

#include "CommFunc.h"

#define RECORD_TYPE_NORMAL                       0x00000000
#define RECORD_TYPE_FILETAG                      0x00000004
#define RECORD_TYPE_FLAG_STATIC                  0x80000000
#define RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE 0x20000000
#define RECORD_TYPE_FLAG_OUT_OF_MEMORY           0x10000000

#define REMAINING_NAME_SPACE(LogRecord) \
	(FLT_ASSERT((LogRecord)->Length >= sizeof(LOG_RECORD)), \
	(USHORT)(MAX_NAME_SPACE - ((LogRecord)->Length - sizeof(LOG_RECORD))))

#define MAX_LOG_RECORD_LENGTH  (RECORD_SIZE - FIELD_OFFSET( RECORD_LIST, LogRecord ))
#define MAX_NAME_SPACE ROUND_TO_SIZE((RECORD_SIZE - sizeof(RECORD_LIST)), sizeof( PVOID ))

extern ULONG g_LogAllocCnt;

typedef struct _LOG_WORKITEM_CONTEXT {
	MINIFLT_INFO MiniInfo;
} LOG_WORKITEM_CONTEXT, * PLOG_WORKITEM_CONTEXT;

typedef struct _RECORD_LIST {
	LIST_ENTRY List;
	LOG_RECORD LogRecord;

} RECORD_LIST, * PRECORD_LIST;

typedef struct _LOG_DATA {
	LARGE_INTEGER Time;
	CHAR DateTime[28];
	PCHAR UserName[MAX_NAME];
	PCHAR GroupName[MAX_NAME];
	PCHAR FileName[MAX_KPATH];
	PCHAR ProcName[MAX_KPATH];

	ULONG Action;

	PWCHAR UserNameW[MAX_NAME];
	PWCHAR GroupNameW[MAX_NAME];
	PWCHAR FileNameW[MAX_KPATH];
	PWCHAR ProcNameW[MAX_KPATH];
} LOG_DATA, * PLOG_DATA;

typedef struct _LOG_RECORD {
	ULONG Length;             // Length of log record.  This Does not include
	ULONG SequenceNumber;     // space used by other members of RECORD_LIST

	ULONG RecordType;         // The type of log record this is.
	ULONG Category;           // For alignment on IA64

	LOG_DATA Data;
	WCHAR FileNameW[MAX_KPATH]; //  This is a null terminated string

} LOG_RECORD, * PLOG_RECORD;

#endif
