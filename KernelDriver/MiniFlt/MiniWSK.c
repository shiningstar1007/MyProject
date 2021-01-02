#include "StdAfx.h"
#include "MiniWSK.h"

WSK_REGISTRATION		g_WskRegistration;
WSK_PROVIDER_NPI		g_WskProvider;
WSK_CLIENT_DISPATCH	g_WskDispatch = { MAKE_WSK_VERSION(1,0), 0, NULL };

enum
{
	DEINITIALIZED,
	DEINITIALIZING,
	INITIALIZING,
	INITIALIZED
};

LONG	g_SocketsState = DEINITIALIZED;

NTSTATUS NTAPI CompletionRoutine(
	__in PDEVICE_OBJECT	DeviceObject,
	__in PIRP Irp,
	__in PKEVENT CompletionEvent
)
{
	ASSERT(CompletionEvent);

	KeSetEvent(CompletionEvent, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS InitWskData(
	__out PIRP* pIrp,
	__out PKEVENT	CompletionEvent
)
{
	ASSERT(pIrp);
	ASSERT(CompletionEvent);

	*pIrp = IoAllocateIrp(1, FALSE);
	if (!*pIrp) {
		KdPrint(("InitWskData(): IoAllocateIrp() failed\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeEvent(CompletionEvent, SynchronizationEvent, FALSE);
	IoSetCompletionRoutine(*pIrp, CompletionRoutine, CompletionEvent, TRUE, TRUE, TRUE);

	return STATUS_SUCCESS;
}

NTSTATUS InitWskBuffer(
	__in  PVOID		Buffer,
	__in  ULONG		BufferSize,
	__out PWSK_BUF	WskBuffer
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	ASSERT(Buffer);
	ASSERT(BufferSize);
	ASSERT(WskBuffer);

	WskBuffer->Offset = 0;
	WskBuffer->Length = BufferSize;

	WskBuffer->Mdl = IoAllocateMdl(Buffer, BufferSize, FALSE, FALSE, NULL);
	if (!WskBuffer->Mdl) {
		KdPrint(("InitWskBuffer(): IoAllocateMdl() failed\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	__try {
		MmProbeAndLockPages(WskBuffer->Mdl, KernelMode, IoWriteAccess);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		KdPrint(("InitWskBuffer(): MmProbeAndLockPages(%p) failed\n", Buffer));
		IoFreeMdl(WskBuffer->Mdl);
		Status = STATUS_ACCESS_VIOLATION;
	}

	return Status;
}

VOID FreeWskBuffer(
	__in PWSK_BUF WskBuffer
)
{
	ASSERT(WskBuffer);

	MmUnlockPages(WskBuffer->Mdl);
	IoFreeMdl(WskBuffer->Mdl);
}

NTSTATUS NTAPI SocketsInit()
{
	WSK_CLIENT_NPI WskClient = { 0 };
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (InterlockedCompareExchange(&g_SocketsState, INITIALIZING, DEINITIALIZED) != DEINITIALIZED)
		return STATUS_ALREADY_REGISTERED;

	WskClient.ClientContext = NULL;
	WskClient.Dispatch = &g_WskDispatch;

	Status = WskRegister(&WskClient, &g_WskRegistration);
	if (!NT_SUCCESS(Status)) {
		KdPrint(("WskRegister() failed with status 0x%08X\n", Status));
		InterlockedExchange(&g_SocketsState, DEINITIALIZED);
		return Status;
	}

	Status = WskCaptureProviderNPI(&g_WskRegistration, WSK_INFINITE_WAIT, &g_WskProvider);
	if (!NT_SUCCESS(Status)) {
		KdPrint(("WskCaptureProviderNPI() failed with status 0x%08X\n", Status));
		WskDeregister(&g_WskRegistration);
		InterlockedExchange(&g_SocketsState, DEINITIALIZED);
		return Status;
	}

	InterlockedExchange(&g_SocketsState, INITIALIZED);

	return STATUS_SUCCESS;
}

VOID NTAPI SocketsDeinit()
{
	if (InterlockedCompareExchange(&g_SocketsState, INITIALIZED, DEINITIALIZING) != INITIALIZED)
		return;

	WskReleaseProviderNPI(&g_WskRegistration);
	WskDeregister(&g_WskRegistration);

	InterlockedExchange(&g_SocketsState, DEINITIALIZED);
}

PWSK_SOCKET NTAPI CreateSocket(
	__in ADDRESS_FAMILY	AddressFamily,
	__in USHORT			SocketType,
	__in ULONG			Protocol,
	__in ULONG			Flags
)
{
	KEVENT CompletionEvent = { 0 };
	PIRP Irp = NULL;
	PWSK_SOCKET WskSocket = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (g_SocketsState != INITIALIZED) return NULL;

	Status = InitWskData(&Irp, &CompletionEvent);
	if (!NT_SUCCESS(Status)) {
		KdPrint(("CreateSocket(): InitWskData() failed with status 0x%08X\n", Status));
		return NULL;
	}

	Status = g_WskProvider.Dispatch->WskSocket(g_WskProvider.Client, AddressFamily,
		SocketType, Protocol, Flags, NULL, NULL, NULL, NULL, NULL, Irp);
	if (Status == STATUS_PENDING) {
		KeWaitForSingleObject(&CompletionEvent, Executive, KernelMode, FALSE, NULL);
		Status = Irp->IoStatus.Status;
	}

	WskSocket = NT_SUCCESS(Status) ? (PWSK_SOCKET)Irp->IoStatus.Information : NULL;

	IoFreeIrp(Irp);

	return (PWSK_SOCKET)WskSocket;
}

NTSTATUS NTAPI CloseSocket(
	__in PWSK_SOCKET WskSocket
)
{
	KEVENT CompletionEvent = { 0 };
	PIRP Irp = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (g_SocketsState != INITIALIZED || !WskSocket) return STATUS_INVALID_PARAMETER;

	Status = InitWskData(&Irp, &CompletionEvent);
	if (!NT_SUCCESS(Status)) {
		KdPrint(("CloseSocket(): InitWskData() failed with status 0x%08X\n", Status));
		return Status;
	}

	Status = ((PWSK_PROVIDER_BASIC_DISPATCH)WskSocket->Dispatch)->WskCloseSocket(WskSocket, Irp);
	if (Status == STATUS_PENDING) {
		KeWaitForSingleObject(&CompletionEvent, Executive, KernelMode, FALSE, NULL);
		Status = Irp->IoStatus.Status;
	}

	IoFreeIrp(Irp);

	return Status;
}