#include "StdAfx.h"

#pragma warning(suppress: 6001)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MiniFltLogConnect)
#pragma alloc_text(PAGE, MiniFltLogDisconnect)
#pragma alloc_text(PAGE, MiniFltLogMessage)
#endif

LOG_RECORD g_LastLog = { 0 };
ULONG g_LogAllocCnt = 0;

PRECORD_LIST AllocateMiniFltLogBuf(
	_Out_ PULONG RecordType
)
{
	PVOID Buffer;
	ULONG RecType = RECORD_TYPE_NORMAL;

	if (g_MiniData.LogBufCnt < g_MiniData.MaxLogBufCnt) {
		InterlockedIncrement(&g_MiniData.LogBufCnt);
		Buffer = ExAllocateFromNPagedLookasideList(&g_MiniData.LogBufLookaside);

		if (Buffer == NULL) {
			InterlockedDecrement(&g_MiniData.LogBufCnt);
			RecType = RECORD_TYPE_FLAG_OUT_OF_MEMORY;
		}
	}
	else {
		RecType = RECORD_TYPE_FLAG_EXCEED_MEMORY_ALLOWANCE;
		Buffer = NULL;
	}

	*RecordType = RecType;

	return Buffer;
}

VOID FreeMiniFltLogBuf(
	_In_ PVOID Buffer
)
{
	ExFreeToNPagedLookasideList(&g_MiniData.LogBufLookaside, Buffer);
	InterlockedDecrement(&g_MiniData.LogBufCnt);
}

PRECORD_LIST CreateMiniFltLog()
{
	PRECORD_LIST LogBuf;
	ULONG InitRecordType;

	LogBuf = AllocateMiniFltLogBuf(&InitRecordType);
	if (LogBuf == NULL) {
		if (!InterlockedExchange(&g_MiniData.StaticBufInUse, TRUE)) {

			LogBuf = (PRECORD_LIST)g_MiniData.OutOfMemoryBuf;
			InitRecordType |= RECORD_TYPE_FLAG_STATIC;
		}
	}

	if (LogBuf) {
		LogBuf->LogRecord.RecordType = InitRecordType;
		LogBuf->LogRecord.Length = sizeof(LOG_RECORD);
		LogBuf->LogRecord.SequenceNumber = InterlockedIncrement(&g_MiniData.LogSequenceNumber);
		RtlZeroMemory(&LogBuf->LogRecord.Data, sizeof(LOG_DATA));
	}

	return LogBuf;
}

VOID FreeMiniFltLog(
	_In_ PRECORD_LIST Record
)
{
	if (FlagOn(Record->LogRecord.RecordType, RECORD_TYPE_FLAG_STATIC)) {
		FLT_ASSERT(g_MiniData.StaticBufInUse);
		g_MiniData.StaticBufInUse = FALSE;
	}
	else FreeMiniFltLogBuf(Record);
}

VOID SetMiniFltRecordName(
	_Inout_ PLOG_RECORD LogRecord,
	_In_opt_ PWCHAR ObjPathW
)
{
	FLT_ASSERT(ObjPathW != NULL);

	MyStrNCopyW(LogRecord->FileNameW, ObjPathW, -1, MAX_KPATH);
	LogRecord->Length = ROUND_TO_SIZE((LogRecord->Length + MAX_KPATH + sizeof(WCHAR)), sizeof(PVOID));

	FLT_ASSERT(LogRecord->Length <= MAX_LOG_RECORD_LENGTH);
}

VOID MiniFltLog(
	_In_ PMINIFLT_INFO MiniFltInfo
)
{
	PRECORD_LIST RecordList;
	PLOG_RECORD LogRecord;
	//SYSTEMTIME SysTime;

	RecordList = CreateMiniFltLog();
	if (!RecordList) return;

	LogRecord = &RecordList->LogRecord;

	//LogRecord->Data.Time = MiniFltInfo->Time;
	//KrnlTimeToSysTime(LogRecord->Data.Time.QuadPart, &SysTime);

	SetMiniFltRecordName(LogRecord, MiniFltInfo->FileNameW);
	MyStrNCopy(LogRecord->Data.FileName, MiniFltInfo->FileName, MAX_KPATH);
	MyStrNCopy(LogRecord->Data.ProcName, MiniFltInfo->ProcName, MAX_KPATH);
	MyStrNCopyW(LogRecord->Data.ProcNameW, MiniFltInfo->ProcNameW, -1, MAX_KPATH);

	if (*MiniFltInfo->UserName) MyStrNCopy(LogRecord->Data.UserName, MiniFltInfo->UserName, MAX_NAME);
/*
	MySNPrintf(LogRecord->Data.DateTime, sizeof(LogRecord->Data.DateTime), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
		SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds);
*/

	memcpy(&g_LastLog, LogRecord, sizeof(LOG_RECORD));
	InsertTailList(&g_MiniData.LogBufList, &RecordList->List);
}

VOID MiniFltLogWorkItemRoutine(
	_In_ PFLT_GENERIC_WORKITEM WorkItem,
	_In_ PVOID FltObject,
	_In_opt_ PVOID Context
)
{
	PLOG_WORKITEM_CONTEXT WorkItemCtx = (PLOG_WORKITEM_CONTEXT)Context;

	UNREFERENCED_PARAMETER(WorkItem);
	UNREFERENCED_PARAMETER(FltObject);

	if (!WorkItemCtx) return;

	MiniFltLog(&WorkItemCtx->MiniInfo);

	MyFreeNonPagedPool(WorkItemCtx, &g_LogAllocCnt);

	FltFreeGenericWorkItem(WorkItem);
}

NTSTATUS MiniFltGetLog(
	_Out_writes_bytes_to_opt_(OutBufLen, *RetOutBufLen) PUCHAR OutBuffer,
	_In_ ULONG OutBufLen,
	_Out_ PULONG RetOutBufLen
)
{
	NTSTATUS Status = STATUS_NO_MORE_ENTRIES;

	return Status;
}

NTSTATUS MiniFltLogConnect(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
)
{
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	FLT_ASSERT(g_MiniData.ClientLogPort == NULL);
	g_MiniData.ClientLogPort = ClientPort;

	return STATUS_SUCCESS;
}

VOID MiniFltLogDisconnect(
	_In_opt_ PVOID ConnectionCookie
)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(ConnectionCookie);

	FltCloseClientPort(g_MiniData.hFilter, &g_MiniData.ClientLogPort);
}

BOOL IsAligned(
	_In_ PVOID buffer
)
{
#if defined(_WIN64)
	if (IoIs32bitProcess(NULL)) {
		if (!IS_ALIGNED(buffer, sizeof(ULONG))) return FALSE;;
	}
	else {
#endif

		if (!IS_ALIGNED(buffer, sizeof(PVOID))) return FALSE;

#if defined(_WIN64)
	}
#endif

	return TRUE;
}

NTSTATUS MiniFltLogMessage(
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InputBufSize) PVOID InputBuf,
	_In_ ULONG InputBufSize,
	_Out_writes_bytes_to_opt_(OutputBufSize, *RetOutBufLen) PVOID OutputBuf,
	_In_ ULONG OutputBufSize,
	_Out_ PULONG RetOutBufLen
)
{
	MINIFLT_COMMAND Command;
	NTSTATUS Status;

	PAGED_CODE();
	UNREFERENCED_PARAMETER(ConnectionCookie);

	if ((InputBuf != NULL) && (InputBufSize >= (FIELD_OFFSET(COMMAND_MESSAGE, Command) + sizeof(MINIFLT_COMMAND)))) {
		__try {
			Command = ((PCOMMAND_MESSAGE)InputBuf)->Command;
		}
		__except (MiniFltExceptionFilter(GetExceptionInformation(), TRUE)) {
			return GetExceptionCode();
		}

		switch (Command) {
		case GET_LOG:
			if ((OutputBuf == NULL) || (OutputBufSize == 0)) {
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if (!IsAligned(OutputBuf)) {
				Status = STATUS_DATATYPE_MISALIGNMENT;
				break;
			}

			Status = MiniFltGetLog(OutputBuf, OutputBufSize, RetOutBufLen);
			break;

		default:
			Status = STATUS_INVALID_PARAMETER;
			break;
		}
	}
	else Status = STATUS_INVALID_PARAMETER;

	return Status;
}
