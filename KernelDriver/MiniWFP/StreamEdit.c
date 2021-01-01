#include "StdAfx.h"

void* g_ThreadObj;
KSTART_ROUTINE StreamEditWorker;
STREAM_EDITOR g_StreamEdit;

BOOL CopyStreamData(
  _Inout_ STREAM_EDITOR* StreamEdit,
  const FWPS_STREAM_DATA* StreamData
)
{
  SIZE_T BytesCopied;
  size_t ExistingDataLen = StreamEdit->DataLen;

  NT_ASSERT(StreamEdit->DataOffset == 0);

  if (StreamEdit->BufSize - ExistingDataLen < StreamData->dataLength)
  {
    size_t NewBufferSize = (StreamData->dataLength + ExistingDataLen) * 2;
    void* NewBuffer = MyAllocNonPagedPool(NewBufferSize, &g_StreamEditPoolCnt);

    if (NewBuffer != NULL)
    {
      if (ExistingDataLen > 0)
      {
        NT_ASSERT(StreamEditor->scratchBuffer != NULL);

        RtlZeroMemory(StreamEdit->ScratchBuf, StreamEdit->BufSize);
        RtlCopyMemory(NewBuffer, StreamEdit->ScratchBuf, ExistingDataLen);
      }
    }

    if (StreamEdit->ScratchBuf != NULL)
    {
      MyFreeNonPagedPool(StreamEdit->ScratchBuf, &g_StreamEditPoolCnt);

      StreamEdit->ScratchBuf = NULL;
      StreamEdit->BufSize = 0;
      StreamEdit->DataLen = 0;
    }

    if (NewBuffer != NULL)
    {
      StreamEdit->ScratchBuf = NewBuffer;
      StreamEdit->BufSize = NewBufferSize;
      StreamEdit->DataLen = ExistingDataLen;
    }
    else
    {
      return FALSE;
    }
  }

  FwpsCopyStreamDataToBuffer(StreamData, (BYTE*)StreamEdit->ScratchBuf + StreamEdit->DataLen,
    StreamData->dataLength, &BytesCopied);

  NT_ASSERT(bytesCopied == StreamData->dataLength);
  StreamEdit->DataLen += BytesCopied;

  return TRUE;
}


NTSTATUS StreamCopyDataToFlatBuffer(
  _Inout_ STREAM_EDITOR* StreamEdit,
  _Inout_ NET_BUFFER_LIST* NetBufListChain,
  _In_ size_t TotalDataLen,
  _In_ DWORD StreamFlags
)
{
  NTSTATUS Status = STATUS_SUCCESS;

  FWPS_STREAM_DATA StreamData = { 0 };

  if (TotalDataLen > 0)
  {
    StreamData.netBufferListChain = NetBufListChain;
    StreamData.dataLength = TotalDataLen;
    StreamData.flags = StreamFlags;

    StreamData.dataOffset.netBufferList = NetBufListChain;
    StreamData.dataOffset.netBuffer = NET_BUFFER_LIST_FIRST_NB(StreamData.dataOffset.netBufferList);
    StreamData.dataOffset.mdl = NET_BUFFER_CURRENT_MDL(StreamData.dataOffset.netBuffer);
    StreamData.dataOffset.mdlOffset = NET_BUFFER_CURRENT_MDL_OFFSET(StreamData.dataOffset.netBuffer);

    if (CopyStreamData(StreamEdit, &StreamData) == FALSE) Status = STATUS_NO_MEMORY;
  }

  return Status;
}

NTSTATUS StreamEditData(
  _Inout_ PSTREAM_EDITOR StreamEdit,
  _Inout_ NET_BUFFER_LIST* NetBufListChain,
  _In_ size_t TotalDataLen,
  _In_ DWORD StreamFlags
)
{
  NTSTATUS Status = STATUS_SUCCESS;
  UINT i = 0;
  BOOLEAN StreamModified = FALSE;
  BOOLEAN PotentialMatch = FALSE;
  PBYTE DataStart;

  __try {
    Status = StreamCopyDataToFlatBuffer(StreamEdit, NetBufListChain, TotalDataLen, StreamFlags);
    if (!NT_SUCCESS(Status)) __leave;

    DataStart = (BYTE*)StreamEdit->ScratchBuf + StreamEdit->DataOffSet;

    for (; i < StreamEdit->DataLen; ++i)
    {

    }
 
  }
  __finally 
  {
    if (NetBufListChain != NULL)
    {
      FwpsDiscardClonedStreamData(NetBufListChain, 0, FALSE);
    }

    if (StreamEdit->StreamEditInfo.pNBLEof != NULL)
    {
      FwpsDiscardClonedStreamData(StreamEdit->StreamEditInfo.pNBLEof, 0,FALSE);

      StreamEdit->StreamEditInfo.pNBLEof = NULL;
    }
  }

  return Status;
}

_IRQL_requires_same_
_Function_class_(KSTART_ROUTINE)
void StreamEditWorker(
  _In_ void* StartContext
)
{
  NTSTATUS Status = STATUS_SUCCESS;
  NET_BUFFER_LIST* NetBufListChain = NULL;
  size_t TotalDataLen;
  STREAM_EDITOR* StreamEdit = (PSTREAM_EDITOR)StartContext;
  DWORD StreamFlags;

  for (;;)
  {
    KLOCK_QUEUE_HANDLE LockHandle;

    KeWaitForSingleObject(&StreamEdit->StreamEditInfo.EditEvent, Executive, KernelMode, FALSE, NULL);

    if (StreamEdit->StreamEditInfo.EditState == STREAM_EDIT_ERROR ||
      StreamEdit->StreamEditInfo.EditState == STREAM_EDIT_SHUT_DOWN) break;

    KeAcquireInStackQueuedSpinLock(&StreamEdit->StreamEditInfo.EditLock, &LockHandle);

    NT_ASSERT(StreamEdit->StreamEditInfo.editState == OOB_EDIT_PROCESSING ||
      StreamEdit->StreamEditInfo.editState == OOB_EDIT_BUSY);

    NetBufListChain = StreamEdit->StreamEditInfo.NBLHead;
    TotalDataLen = StreamEdit->StreamEditInfo.TotalDataLen;
    StreamFlags = StreamEdit->StreamEditInfo.StreamFlags;

    StreamEdit->StreamEditInfo.NBLHead = NULL;
    StreamEdit->StreamEditInfo.NBLTail = NULL;
    StreamEdit->StreamEditInfo.TotalDataLen = 0;

    KeReleaseInStackQueuedSpinLock(&LockHandle);

    _Analysis_assume_(NetBufListChain != NULL);
    Status = StreamEditData(StreamEdit, NetBufListChain, TotalDataLen, StreamFlags);

    if (!NT_SUCCESS(Status))
    {
      StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_ERROR;
      break;
    }

    if (StreamEdit->StreamEditInfo.EditState == STREAM_EDIT_BUSY)
    {
      NTSTATUS streamContinueStatus;
      StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_PROCESSING;

      streamContinueStatus = FwpsStreamContinue(
        StreamEdit->StreamEditInfo.FlowId,
        StreamEdit->StreamEditInfo.CalloutId,
        StreamEdit->StreamEditInfo.LayerId,
        StreamEdit->StreamEditInfo.StreamFlags
      );

      if (!NT_SUCCESS(streamContinueStatus))
      {
        StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_ERROR;
        break;
      }
    }

    KeAcquireInStackQueuedSpinLock(&StreamEdit->StreamEditInfo.EditLock, &LockHandle);

    if (StreamEdit->StreamEditInfo.NBLHead == NULL)
    {
      if (StreamEdit->StreamEditInfo.ShuttingDown == FALSE)
      {
        StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_IDLE;
        KeClearEvent(&StreamEdit->StreamEditInfo.EditEvent);
      }
      else
      {
        StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_SHUT_DOWN;
      }
    }

    KeReleaseInStackQueuedSpinLock(&LockHandle);
  }

  PsTerminateSystemThread(Status);
}

NTSTATUS InitStreamEdit(
	_Out_ PSTREAM_EDITOR StreamEdit
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE ThreadHandle;

	KeInitializeSpinLock(&StreamEdit->StreamEditInfo.EditLock);
	KeInitializeEvent(&StreamEdit->StreamEditInfo.EditEvent, NotificationEvent, FALSE);

	StreamEdit->StreamEditInfo.BusyThreshold = 32 * 1024;
	StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_IDLE;

	InitializeListHead(&StreamEdit->StreamEditInfo.OutgoingDataQueue);

	Status = PsCreateSystemThread(&ThreadHandle, THREAD_ALL_ACCESS, NULL, NULL, NULL, StreamEditWorker, &g_StreamEdit);
	if (NT_SUCCESS(Status)) {

		Status = ObReferenceObjectByHandle(ThreadHandle, 0, NULL, KernelMode, &g_ThreadObj, NULL);

		NT_ASSERT(NT_SUCCESS(Status));

		ZwClose(ThreadHandle);
	}

	return Status;
}

void EditShutdown(
	_Out_ PSTREAM_EDITOR StreamEdit
)
{
	KLOCK_QUEUE_HANDLE LockHandle;

	KeAcquireInStackQueuedSpinLock(&StreamEdit->StreamEditInfo.EditLock, &LockHandle);

	StreamEdit->StreamEditInfo.ShuttingDown = TRUE;
	switch (StreamEdit->StreamEditInfo.EditState) {

	case STREAM_EDIT_IDLE: {

		StreamEdit->StreamEditInfo.EditState = STREAM_EDIT_SHUT_DOWN;
		KeSetEvent(&StreamEdit->StreamEditInfo.EditEvent, IO_NO_INCREMENT, FALSE);
		break;
	}

	default:
		break;
	};

	KeReleaseInStackQueuedSpinLock(&LockHandle);

	NT_ASSERT(g_ThreadObj != NULL);

	KeWaitForSingleObject(g_ThreadObj, Executive, KernelMode, FALSE, NULL);

	ObDereferenceObject(g_ThreadObj);
}