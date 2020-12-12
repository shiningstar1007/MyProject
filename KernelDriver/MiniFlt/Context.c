#include "StdAfx.h"

VOID MiniFltContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
)
{
	PMINI_FLT_CONTEXT MiniFltContext = (PMINI_FLT_CONTEXT)Context;
	PCHAR ContextName = "";

	PAGED_CODE();

	if (ContextType == FLT_FILE_CONTEXT) ContextName = "file";
	else if (ContextType == FLT_STREAMHANDLE_CONTEXT) ContextName = "streamhandle";

	DbgPrint("%s: Cleaning up %s context for FileName = %s (FileContext = %p)",
		__FUNCTION__, ContextName, MiniFltContext->ObjPath, MiniFltContext);
}

NTSTATUS FindMiniFltContext(
	_In_ FLT_CONTEXT_TYPE ContextType,
	_In_ PFLT_CALLBACK_DATA Data,
	_Outptr_ PMINI_FLT_CONTEXT* pMiniFltContext
)
{
	NTSTATUS Status = STATUS_NOT_FOUND;
	PMINI_FLT_CONTEXT MiniFltContext = NULL;

	PAGED_CODE();

	if (ContextType == FLT_STREAMHANDLE_CONTEXT) {
		Status = FltGetStreamHandleContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, &MiniFltContext);
	}
	else if (ContextType == FLT_FILE_CONTEXT) {
		Status = FltGetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject, &MiniFltContext);
	}

	if (Status == STATUS_NOT_FOUND) *pMiniFltContext = NULL;
	else *pMiniFltContext = MiniFltContext;

	return Status;
}

NTSTATUS CreateMiniFltContext(
	_In_ FLT_CONTEXT_TYPE ContextType,
	_In_ PFLT_CALLBACK_DATA Data,
	_In_ PFLT_FILTER hFilter,
	_Outptr_ PMINI_FLT_CONTEXT* pMiniFltContext
)
{
	NTSTATUS Status = STATUS_NOT_FOUND;
	PMINI_FLT_CONTEXT MiniFltContext, OldMiniFltContext = NULL;

	PAGED_CODE();

	*pMiniFltContext = NULL;

	Status = FltAllocateContext(hFilter, ContextType, sizeof(MINI_FLT_CONTEXT), NonPagedPool, &MiniFltContext);
	if (!NT_SUCCESS(Status)) {
		DbgPrint("Failed to allocate file context with status 0x%x \n", Status);
		return Status;
	}
	RtlZeroMemory(MiniFltContext, sizeof(MINI_FLT_CONTEXT));

	if (ContextType == FLT_STREAMHANDLE_CONTEXT) {
		Status = FltSetStreamHandleContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject,
			FLT_SET_CONTEXT_KEEP_IF_EXISTS, MiniFltContext, &OldMiniFltContext);
	}
	else if (ContextType == FLT_FILE_CONTEXT) {
		Status = FltSetFileContext(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject,
			FLT_SET_CONTEXT_REPLACE_IF_EXISTS, MiniFltContext, &OldMiniFltContext);
	}

	if (!NT_SUCCESS(Status)) {
		DbgPrint("Failed to set file context with status 0x%x. (FileObject = %p, Instance = %p, rq = %d)\n",
			Status, Data->Iopb->TargetFileObject, Data->Iopb->TargetInstance, Data->Iopb->MajorFunction);

		FltReleaseContext(MiniFltContext);

		if (Status != STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
			DbgPrint("Failed to set file context with status 0x%x != STATUS_FLT_CONTEXT_ALREADY_DEFINED. (FileObject = %p, Instance = %p)\n",
				Status, Data->Iopb->TargetFileObject, Data->Iopb->TargetInstance);

			return Status;
		}

		DbgPrint("Race: File context already defined. Retaining old file context %p (FileObject = %p, Instance = %p)\n",
			OldMiniFltContext, Data->Iopb->TargetFileObject, Data->Iopb->TargetInstance);

		MiniFltContext = OldMiniFltContext;
		Status = STATUS_SUCCESS;
	}

	*pMiniFltContext = MiniFltContext;

	return Status;
}