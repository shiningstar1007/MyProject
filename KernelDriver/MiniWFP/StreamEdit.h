#ifndef __STREAMEDIT_H__
#define __STREAMEDIT_H__

#include <ntddk.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union

#include <fwpsk.h>

#pragma warning(pop)

#include <fwpmk.h>

typedef enum STREAM_EDIT_STATE_
{
  STREAM_EDIT_IDLE,
  STREAM_EDIT_PROCESSING,
  STREAM_EDIT_BUSY,
  STREAM_EDIT_SHUT_DOWN,
  STREAM_EDIT_ERROR
} STREAM_EDIT_STATE;

typedef struct STREAM_EDITOR_
{
  union
  {
    struct
    {
      STREAM_EDIT_STATE EditState;
      BOOLEAN ShuttingDown;

      KSPIN_LOCK EditLock;
      NET_BUFFER_LIST* NBLHead;
      NET_BUFFER_LIST* NBLTail;
      size_t TotalDataLen;
      BOOLEAN NoMoreData;
      NET_BUFFER_LIST* pNBLEof;
      size_t BusyThreshold;
      UINT64 FlowId;
      UINT32 CalloutId;
      UINT16 LayerId;
      DWORD StreamFlags;
      KEVENT EditEvent;
      LIST_ENTRY OutgoingDataQueue;
    } StreamEditInfo;
  };

  void* ScratchBuf;
  size_t BufSize;
  size_t DataOffSet;
  size_t DataLen;

} STREAM_EDITOR, *PSTREAM_EDITOR;

extern STREAM_EDITOR g_StreamEdit;

#endif