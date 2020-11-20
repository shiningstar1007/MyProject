#include "StdAfx.h"

#pragma warning(suppress: 6001)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MiniFltLogConnect)
#pragma alloc_text(PAGE, MiniFltLogDisconnect)
#pragma alloc_text(PAGE, MiniFltLogMessage)
#endif

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
