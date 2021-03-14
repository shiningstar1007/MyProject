#ifndef __MINIFLTLOG_H__
#define __MINIFLTLOG_H__

#include "CommFunc.h"
#include "MiniFlt.h"

#define MAX_KPATH 264 
#define MAX_NAME 64
#define MAX_USER_NAME 48
#define MAX_DATA_BUF 4096
#define BUF_UNIT 32768

#define MINI_PORT_NAME     L"\\MiniFltPort"
#define MINI_LOG_PORT_NAME L"\\MiniFltLogPort"

#define RECORD_TYPE_NORMAL                       0x00000000
#define RECORD_TYPE_FILETAG                      0x00000004
#define RECORD_TYPE_FLAG_STATIC                  0x80000000
#define RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE 0x20000000
#define RECORD_TYPE_FLAG_OUT_OF_MEMORY           0x10000000

extern ULONG g_LogAllocCnt;
extern KSPIN_LOCK g_LogSpinLock;

typedef struct _LOG_WORKITEM_CONTEXT {
	MINIFLT_INFO MiniInfo;
} LOG_WORKITEM_CONTEXT, *PLOG_WORKITEM_CONTEXT;

typedef struct _LOG_DATA {
	LARGE_INTEGER Time;
	CHAR DateTime[28];
	CHAR UserName[MAX_NAME];
	CHAR GroupName[MAX_NAME];
	CHAR FileName[MAX_KPATH];
	CHAR ProcName[MAX_KPATH];

	ULONG Action;

	WCHAR UserNameW[MAX_NAME];
	WCHAR GroupNameW[MAX_NAME];
	WCHAR FileNameW[MAX_KPATH];
	WCHAR ProcNameW[MAX_KPATH];
} LOG_DATA, * PLOG_DATA;

typedef struct _LOG_RECORD {
	ULONG Length;             // Length of log record.  This Does not include
	ULONG SequenceNumber;     // space used by other members of RECORD_LIST

	ULONG RecordType;         // The type of log record this is.
	ULONG Category;           // For alignment on IA64

	LOG_DATA Data;
	WCHAR FileNameW[MAX_KPATH]; //  This is a null terminated string

} LOG_RECORD, * PLOG_RECORD;

typedef struct _RECORD_LIST {
	LIST_ENTRY List;
	LOG_RECORD LogRecord;

} RECORD_LIST, *PRECORD_LIST;

typedef struct _MINIFLT_LOG_CONTEXT MINIFLT_LOG_CONTEXT, *PMINIFLT_LOG_CONTEXT;
typedef struct _MINIFLT_LOG_OP_CONTEXT MINIFLT_LOG_OP_CONTEXT, *PMINIFLT_LOG_OP_CONTEXT;
typedef VOID(*PMINIFLT_OP_HANDLER_FN)(_In_ PMINIFLT_LOG_OP_CONTEXT LogContext);

typedef struct _MINIFLT_WORK_QUEUE {
	SLIST_HEADER Head;
	KEVENT Event;
	BOOLEAN Stop;
	PETHREAD Thread;
} MINIFLT_WORK_QUEUE, * PMINIFLT_WORK_QUEUE;

struct _MINIFLT_LOG_OP_CONTEXT {
	SLIST_ENTRY QueueEntry;
	PMINIFLT_OP_HANDLER_FN OpHandle;
	PMINIFLT_LOG_CONTEXT LogContext;
};

struct _MINIFLT_LOG_CONTEXT {
	MINIFLT_INFO MiniInfo;
	PMINIFLT_WORK_QUEUE WorkQueue;
	MINIFLT_LOG_OP_CONTEXT LogOpContext;
};

NTSTATUS MiniFltLogConnect(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
);

VOID MiniFltLogDisconnect(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS MiniFltLogMessage(
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InputBufSize) PVOID InputBuf,
	_In_ ULONG InputBufSize,
	_Out_writes_bytes_to_opt_(OutputBufSize, *RetOutBufLen) PVOID OutputBuf,
	_In_ ULONG OutputBufSize,
	_Out_ PULONG RetOutBufLen
);

PRECORD_LIST AllocateMiniFltLogBuf(
	_Out_ PULONG RecordType
);

VOID FreeMiniFltLogBuf(
	_In_ PVOID Buffer
);

PRECORD_LIST CreateMiniFltLog();

VOID FreeMiniFltLog(
	_In_ PRECORD_LIST Record
);

VOID SetMiniFltRecordName(
	_Inout_ PLOG_RECORD LogRecord,
	_In_opt_ PWCHAR ObjPathW
);

VOID MiniFltLogInsert(
	_In_ PMINIFLT_INFO MiniFltInfo
);

VOID MiniFltLogClear();

NTSTATUS MiniFltLogAsync(
	_In_ PMINIFLT_INFO MiniFltInfo
);

VOID MiniFltLogWorkItemRoutine(
	_In_ PFLT_GENERIC_WORKITEM WorkItem,
	_In_ PVOID FltObject,
	_In_opt_ PVOID Context
);

NTSTATUS MiniFltGetLog(
	_Out_writes_bytes_to_opt_(OutBufLen, *RetOutBufLen) PUCHAR OutBuffer,
	_In_ ULONG OutBufLen,
	_Out_ PULONG RetOutBufLen
);

#define REMAINING_NAME_SPACE(LogRecord) \
	(FLT_ASSERT((LogRecord)->Length >= sizeof(LOG_RECORD)), \
	(USHORT)(MAX_NAME_SPACE - ((LogRecord)->Length - sizeof(LOG_RECORD))))

#define MAX_LOG_RECORD_LENGTH  (RECORD_SIZE - FIELD_OFFSET( RECORD_LIST, LogRecord ))
#define MAX_NAME_SPACE ROUND_TO_SIZE((RECORD_SIZE - sizeof(RECORD_LIST)), sizeof( PVOID ))

#endif
