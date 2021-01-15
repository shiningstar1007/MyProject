#include "StdAfx.h"

#pragma warning(suppress: 6001)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MiniFltLogConnect)
#pragma alloc_text(PAGE, MiniFltLogDisconnect)
#pragma alloc_text(PAGE, MiniFltLogMessage)
#endif

LOG_RECORD g_LastLog = { 0 };
ULONG g_LogAllocCnt = 0;
KSPIN_LOCK g_LogSpinLock;

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
	PRECORD_LIST LogBuf = NULL;
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
	LogRecord->Length = ROUND_TO_SIZE(((LogRecord->Length + MAX_KPATH) + sizeof(WCHAR)), sizeof(PVOID));

	FLT_ASSERT(LogRecord->Length <= MAX_LOG_RECORD_LENGTH);
}

VOID MiniFltLogInsert(
	_In_ PMINIFLT_INFO MiniFltInfo
)
{
	PRECORD_LIST RecordList = NULL;
	PLOG_RECORD LogRecord = NULL;
	SYSTEMTIME SysTime;
	KLOCK_QUEUE_HANDLE hLockQueue;

	RecordList = CreateMiniFltLog();
	if (RecordList == NULL) return;

	LogRecord = &RecordList->LogRecord;

	LogRecord->Data.Time = GetKernelTime(&SysTime);

	SetMiniFltRecordName(LogRecord, MiniFltInfo->FileNameW);
	MyStrNCopy(LogRecord->Data.FileName, MiniFltInfo->FileName, MAX_KPATH);
	MyStrNCopy(LogRecord->Data.ProcName, MiniFltInfo->ProcName, MAX_KPATH);
	MyStrNCopyW(LogRecord->Data.ProcNameW, MiniFltInfo->ProcNameW, -1, MAX_KPATH);

	if (*MiniFltInfo->UserName) MyStrNCopy(LogRecord->Data.UserName, MiniFltInfo->UserName, MAX_NAME);
/*
	MySNPrintf(LogRecord->Data.DateTime, sizeof(LogRecord->Data.DateTime), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
		SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds);
*/

	KeAcquireInStackQueuedSpinLock(&g_LogSpinLock, &hLockQueue);
	RtlCopyMemory(&g_LastLog, LogRecord, sizeof(LOG_RECORD));
	InsertTailList(&(g_MiniData.LogBufList), &(RecordList->List));
	KeReleaseInStackQueuedSpinLock(&hLockQueue);

}


VOID MiniFltLogClear()
{
	PLIST_ENTRY pList;
	PRECORD_LIST pRecordList;
	KLOCK_QUEUE_HANDLE hLockQueue;

	KeAcquireInStackQueuedSpinLock(&g_LogSpinLock, &hLockQueue);
	while (!IsListEmpty(&g_MiniData.LogBufList)) {
		pList = RemoveHeadList(&g_MiniData.LogBufList);

		pRecordList = CONTAINING_RECORD(pList, RECORD_LIST, List);
		FreeMiniFltLog(pRecordList);
	}
	KeReleaseInStackQueuedSpinLock(&hLockQueue);
}

NTSTATUS MiniFltLogAsync(
	_In_ PMINIFLT_INFO MiniFltInfo
)
{
	PFLT_GENERIC_WORKITEM WorkItem = NULL;
	PLOG_WORKITEM_CONTEXT WorkItemCtx = NULL;
	NTSTATUS Status = STATUS_SUCCESS;

	WorkItem = FltAllocateGenericWorkItem();
	if (WorkItem) {
		WorkItemCtx = MyAllocNonPagedPool(sizeof(LOG_WORKITEM_CONTEXT), &g_LogAllocCnt);
		if (WorkItemCtx) {
			RtlCopyMemory(&WorkItemCtx->MiniInfo, MiniFltInfo, sizeof(MINIFLT_INFO));

			Status = FltQueueGenericWorkItem(WorkItem, (PVOID)MiniFltInfo->FltObjects->Filter, MiniFltLogWorkItemRoutine,
				DelayedWorkQueue, (PVOID)WorkItemCtx);
			if (!NT_SUCCESS(Status)) {
				DbgPrint("MiniFltLogAsync: Failed to Create LogWorkItem\n");
				MyFreeNonPagedPool(WorkItemCtx, &g_LogAllocCnt);

				FltFreeGenericWorkItem(WorkItem);
			}
		}
		else FltFreeGenericWorkItem(WorkItem);
	}

	return Status;
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

	MiniFltLogInsert(&WorkItemCtx->MiniInfo);

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
	PLIST_ENTRY pList;
	ULONG BytesWritten = 0;
	PLOG_RECORD pLogRecord;
	PRECORD_LIST pRecordList;
	BOOL RecordsAvailable = FALSE;
	KLOCK_QUEUE_HANDLE hLockQueue;

	KeAcquireInStackQueuedSpinLock(&g_LogSpinLock, &hLockQueue);
	while (!IsListEmpty(&g_MiniData.LogBufList) && (OutBufLen > 0)) {
		RecordsAvailable = TRUE;
		pList = RemoveHeadList(&g_MiniData.LogBufList);
		pRecordList = CONTAINING_RECORD(pList, RECORD_LIST, List);
		pLogRecord = &pRecordList->LogRecord;

		if (REMAINING_NAME_SPACE(pLogRecord) == MAX_NAME_SPACE) {
			pLogRecord->Length += ROUND_TO_SIZE(sizeof(WCHAR), sizeof(PVOID));
			pLogRecord->FileNameW[0] = 0;
		}

		if (OutBufLen < pLogRecord->Length) {
			InsertHeadList(&g_MiniData.LogBufList, pList);
			break;
		}

		__try {
			RtlCopyMemory(OutBuffer, pLogRecord, pLogRecord->Length);
		}
		__except (MiniFltExceptionFilter(GetExceptionInformation(), TRUE)) {
			InsertHeadList(&g_MiniData.LogBufList, pList);
			KeReleaseInStackQueuedSpinLock(&hLockQueue);

			return GetExceptionCode();
		}

		BytesWritten += pLogRecord->Length;
		OutBufLen -= pLogRecord->Length;
		OutBuffer += pLogRecord->Length;
		FreeMiniFltLogBuf(pRecordList);
	}
	KeReleaseInStackQueuedSpinLock(&hLockQueue);

	if ((BytesWritten == 0) && RecordsAvailable) Status = STATUS_BUFFER_TOO_SMALL;
	else if (BytesWritten > 0) Status = STATUS_SUCCESS;

	*RetOutBufLen = BytesWritten;

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

VOID ThreadWorkEnqueue(
	_In_ PMINIFLT_LOG_OP_CONTEXT LogOpContext, 
	_In_ PMINIFLT_OP_HANDLER_FN OpHandle
) 
{
	PMINIFLT_WORK_QUEUE WorkQueue = LogOpContext->LogContext->WorkQueue;

	LogOpContext->OpHandle = OpHandle;

	if (!InterlockedPushEntrySList(&WorkQueue->Head, &LogOpContext->QueueEntry)) {
		KeSetEvent(&WorkQueue->Event, 0, FALSE);
	}
}

PMINIFLT_LOG_CONTEXT MiniFltLogAllocateContext(
	_In_ PMINIFLT_WORK_QUEUE WorkQueue
) 
{
	PMINIFLT_LOG_CONTEXT LogContext;

	LogContext = MyAllocNonPagedPool(sizeof(MINIFLT_LOG_CONTEXT), &g_LogAllocCnt);

	if (LogContext != NULL) {
		RtlZeroMemory(LogContext, sizeof(MINIFLT_LOG_CONTEXT));

		LogContext->WorkQueue = WorkQueue;
		LogContext->LogOpContext.LogContext = LogContext;

		return LogContext;
	}

	return NULL;
}

VOID MiniFltLogAllocateFree(
	_In_ PMINIFLT_LOG_CONTEXT LogContext
) 
{
	if (LogContext != NULL) MyFreeNonPagedPool(LogContext, &g_LogAllocCnt);
}

VOID MiniFltWorkThread(
	_In_ PVOID Context
)
{
	PMINIFLT_WORK_QUEUE ExWorkQueue = (PMINIFLT_WORK_QUEUE)Context;
	PSLIST_ENTRY pEntry, ListEntry, NextEntry;

	while (TRUE) {
		pEntry = InterlockedFlushSList(&ExWorkQueue->Head);

		if (!pEntry) {
			if (ExWorkQueue->Stop) break;

			KeWaitForSingleObject(&ExWorkQueue->Event, Executive, KernelMode, FALSE, 0);
			continue;
		}

		ListEntry = NULL;
		while (pEntry != NULL) { //LIFO -> FIFO
			NextEntry = pEntry->Next;
			pEntry->Next = ListEntry;
			ListEntry = pEntry;
			pEntry = NextEntry;
		}

		while (ListEntry) {
			PMINIFLT_LOG_OP_CONTEXT WorkContext = CONTAINING_RECORD(ListEntry, MINIFLT_LOG_OP_CONTEXT, QueueEntry);
			PMINIFLT_OP_HANDLER_FN OpHandle = WorkContext->OpHandle;

			ListEntry = ListEntry->Next;

			OpHandle(WorkContext);
		}
	}

	PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS MiniFltStartWorkQueue(
	_In_ PMINIFLT_WORK_QUEUE WorkQueue
) 
{
	NTSTATUS Status;
	HANDLE hThread;

	InitializeSListHead(&WorkQueue->Head);
	KeInitializeEvent(&WorkQueue->Event, SynchronizationEvent, FALSE);
	WorkQueue->Stop = FALSE;

	Status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, MiniFltWorkThread, WorkQueue);

	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	Status = ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, &WorkQueue->Thread, NULL);

	ZwClose(hThread);

	if (!NT_SUCCESS(Status)) {
		WorkQueue->Stop = TRUE;
		KeSetEvent(&WorkQueue->Event, 0, FALSE);
	}

	return Status;
}

VOID MiniFltStopWorkQueue(
	_In_ PMINIFLT_WORK_QUEUE WorkQueue
) 
{
	WorkQueue->Stop = TRUE;
	KeSetEvent(&WorkQueue->Event, 0, FALSE);
	KeWaitForSingleObject(WorkQueue->Thread, Executive, KernelMode, FALSE, NULL);
	ObDereferenceObject(WorkQueue->Thread);

}