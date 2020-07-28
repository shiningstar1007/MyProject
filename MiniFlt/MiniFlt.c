#include "StdAfx.h"

PFLT_FILTER g_hFilter;
MYMINIFLT_DATA g_MyMiniFlt;

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
MiniFltUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
MiniFltInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

NTSTATUS
MiniFltInstanceQueryTeardown(
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
MiniFltPreCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
MiniFltPostCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Inout_opt_ PVOID CbdContext,
  _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
MiniFltPreCleanup(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, MiniFltUnload)
#pragma alloc_text(PAGE, MiniFltContextCleanup)
#pragma alloc_text(PAGE, MiniFltInstanceSetup)
#endif


FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      MiniFltPreCreate,
      MiniFltPostCreate },

    { IRP_MJ_CLEANUP,
      FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,
      MiniFltPreCleanup,
      NULL },

    { IRP_MJ_OPERATION_END }
};

const FLT_CONTEXT_REGISTRATION ContextRegistration[] = {
    { FLT_VOLUME_CONTEXT, 
    0, 
    MiniFltContextCleanup, 
    sizeof(VOLUME_CONTEXT), 
    TAG_CONTEXT },

    { FLT_CONTEXT_END }
};

FLT_REGISTRATION FilterRegistration = {
    sizeof( FLT_REGISTRATION ),                     //  Size
    FLT_REGISTRATION_VERSION,                       //  Version
    0,                                              //  Flags
    ContextRegistration,                            //  Context
    Callbacks,                                      //  Operation callbacks
    MiniFltUnload,                                  //  Filters unload routine
    MiniFltInstanceSetup,                           //  InstanceSetup routine
    MiniFltInstanceQueryTeardown,                   //  InstanceQueryTeardown routine
    NULL,                                           //  InstanceTeardownStart routine
    NULL,                                           //  InstanceTeardownComplete routine
    NULL, NULL, NULL                                //  Unused naming support callbacks
};

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;

    ExInitializeDriverRuntime( DrvRtPoolNxOptIn );

    UNREFERENCED_PARAMETER( RegistryPath );



    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &g_hFilter);

    if (!NT_SUCCESS( status )) {

        return status;
    }

    status = FltStartFiltering(g_hFilter);

    if (!NT_SUCCESS( status )) {

        FltUnregisterFilter(g_hFilter);
    }

    return status;
}

NTSTATUS
MiniFltUnload (
  _In_ FLT_FILTER_UNLOAD_FLAGS Flags
  )
{
  UNREFERENCED_PARAMETER(Flags);

  PAGED_CODE();


  FltUnregisterFilter(g_hFilter);

  return STATUS_SUCCESS;
}

NTSTATUS
MiniFltInstanceSetup(
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
  _In_ DEVICE_TYPE VolumeDeviceType,
  _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PDEVICE_OBJECT DeviceObj = NULL;
	PVOLUME_CONTEXT pVolumeContext = NULL;
	ULONG RetLen, VolumePropLen = sizeof(FLT_VOLUME_PROPERTIES) + 512;
	PFLT_VOLUME_PROPERTIES VolumeProp = NULL;
	PUNICODE_STRING WorkingName;
	USHORT BufSize;
	PCHAR DeviceType = "", FileSystem = "", Flag = "";

	PAGED_CODE();

	if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)
		DeviceType = "FILE_DEVICE_DISK_FILE_SYSTEM";
	else if (VolumeDeviceType == FILE_DEVICE_CD_ROM_FILE_SYSTEM)
		DeviceType = "FILE_DEVICE_CD_ROM_FILE_SYSTEM";
	else if (VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM)
		DeviceType = "FILE_DEVICE_NETWORK_FILE_SYSTEM";

	if (VolumeFilesystemType == FLT_FSTYPE_NTFS)
		FileSystem = "FLT_FSTYPE_NTFS";
	else if (VolumeFilesystemType == FLT_FSTYPE_FAT)
		FileSystem = "FLT_FSTYPE_FAT";
	else if (VolumeFilesystemType == FLT_FSTYPE_CDFS)
		FileSystem = "FLT_FSTYPE_CDFS";
	else if (VolumeFilesystemType == FLT_FSTYPE_NFS)
		FileSystem = "FLT_FSTYPE_NFS";
	else if (VolumeFilesystemType == FLT_FSTYPE_REFS)
		FileSystem = "FLT_FSTYPE_REFS";

	if (FlagOn(Flags, FLTFL_INSTANCE_SETUP_AUTOMATIC_ATTACHMENT)) {
		Flag = "FLTFL_INSTANCE_SETUP_AUTOMATIC_ATTACHMENT";
	}
	else if (FlagOn(Flags, FLTFL_INSTANCE_SETUP_MANUAL_ATTACHMENT))
		Flag = "FLTFL_INSTANCE_SETUP_MANUAL_ATTACHMENT";

	if (*FileSystem) {
		DbgPrint("DeviceType:%s Filesystem:%s Flag:%s Volume:%p Instance:%p\n",
			DeviceType, FileSystem, Flag, FltObjects->Volume, FltObjects->Instance);
	}
	else {
    DbgPrint("DeviceType:%s Filesystem:%u Flag:%s Volume:%p Instance:%p\n",
			DeviceType, VolumeFilesystemType, Flag, FltObjects->Volume, FltObjects->Instance);
	}

  VolumeProp = MyAllocNonPagedPool(VolumePropLen, &g_NonPagedPoolCnt);
	if (!VolumeProp) return Status;

	__try {
		Status = FltAllocateContext(FltObjects->Filter, FLT_VOLUME_CONTEXT, sizeof(VOLUME_CONTEXT),
			NonPagedPool, &pVolumeContext);
		if (!NT_SUCCESS(Status)) __leave;

		Status = FltGetVolumeProperties(FltObjects->Volume, VolumeProp, VolumePropLen, &RetLen);
		if (!NT_SUCCESS(Status)) __leave;

		memset(pVolumeContext, 0, sizeof(VOLUME_CONTEXT));

		pVolumeContext->FileSystemType = VolumeFilesystemType;
		if (VolumeProp->DeviceCharacteristics & FILE_REMOVABLE_MEDIA)
			pVolumeContext->DriveType = DRIVE_REMOVABLE;

		ASSERT((VolProp->SectorSize == 0) || (VolProp->SectorSize >= MIN_SECTOR_SIZE));

		pVolumeContext->SectorSize = max(VolumeProp->SectorSize, MIN_SECTOR_SIZE);
		pVolumeContext->VolumeName.Buffer = NULL;

		Status = FltGetDiskDeviceObject(FltObjects->Volume, &DeviceObj);
		if (NT_SUCCESS(Status)) {
#pragma prefast(suppress:__WARNING_USE_OTHER_FUNCTION, "Used to maintain compatability with Win 2k")
			Status = IoVolumeDeviceToDosName(DeviceObj, &pVolumeContext->VolumeName);
		}

		if (NT_SUCCESS(Status)) {
			DbgPrint("DOS Volume Name: %wZ (%s)\n", &pVolumeContext->VolumeName,
				pVolumeContext->DriveType == DRIVE_REMOVABLE ? "DRIVE_REMOVABLE" : "DRIVE_FIXED");
		}
		else {
			ASSERT(pVolContext->Name.Buffer == NULL);

			if (VolumeProp->RealDeviceName.Length > 0) WorkingName = &VolumeProp->RealDeviceName;
			else if (VolumeProp->FileSystemDeviceName.Length > 0) WorkingName = &VolumeProp->FileSystemDeviceName;
			else {
        DbgPrint("No Name. Don't attach.\n");
				Status = STATUS_FLT_DO_NOT_ATTACH;
				__leave;
			}

			BufSize = WorkingName->Length + sizeof(WCHAR);

			// Now allocate a buffer to hold this name
			pVolumeContext->VolumeName.Buffer = MyAllocNonPagedPool(BufSize, &g_NonPagedPoolCnt);
			if (pVolumeContext->VolumeName.Buffer == NULL) {
        DbgPrint("Memory allocation failed.\n");
				Status = STATUS_INSUFFICIENT_RESOURCES;
				__leave;
			}

			pVolumeContext->VolumeName.Length = 0;
			pVolumeContext->VolumeName.MaximumLength = BufSize;
			RtlCopyUnicodeString(&pVolumeContext->VolumeName, WorkingName);

      if (!_wcsnicmp(pVolumeContext->VolumeName.Buffer, L"\\Device\\Mup", pVolumeContext->VolumeName.Length / 2) ||
        !_wcsnicmp(pVolumeContext->VolumeName.Buffer, L"\\Device\\LanmanRedirector", pVolumeContext->VolumeName.Length / 2) ||
        !_wcsnicmp(pVolumeContext->VolumeName.Buffer, L"\\FileSystem\\MRxSmb", pVolumeContext->VolumeName.Length / 2)) {
        pVolumeContext->DriveType = DRIVE_NETWORK;
      }

			RtlAppendUnicodeToString(&pVolumeContext->VolumeName, L":");

			if (pVolumeContext->DriveType == DRIVE_FIXED)
        DbgPrint("NT Volume Name: %wZ (DRIVE_FIXED)\n", &pVolumeContext->VolumeName);
      else if (pVolumeContext->DriveType == DRIVE_NETWORK)
        DbgPrint("NT Volume Name: %wZ (DRIVE_NETWORK)\n", &pVolumeContext->VolumeName);
			else 
        DbgPrint("NT Volume Name: %wZ (DRIVE_REMOVABLE)\n", &pVolumeContext->VolumeName);

		}

		Status = FltSetVolumeContext(FltObjects->Volume, FLT_SET_CONTEXT_KEEP_IF_EXISTS, pVolumeContext, NULL);
		if (Status == STATUS_FLT_CONTEXT_ALREADY_DEFINED) Status = STATUS_SUCCESS;
	}
	__finally {
		if (pVolumeContext != NULL) {

			FltReleaseContext(pVolumeContext);
		}

		if (DeviceObj != NULL) {

			ObDereferenceObject(DeviceObj);
		}

    MyFreeNonPagedPool(VolumeProp, &g_NonPagedPoolCnt);
	}

	return Status;
}

NTSTATUS
MiniFltInstanceQueryTeardown (
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
  )
{
  UNREFERENCED_PARAMETER(FltObjects);
  UNREFERENCED_PARAMETER(Flags);

  PAGED_CODE();

  return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS
MiniFltPreCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
  UNREFERENCED_PARAMETER(Data);
  UNREFERENCED_PARAMETER(FltObjects);
  UNREFERENCED_PARAMETER(CompletionContext);
  FLT_PREOP_CALLBACK_STATUS Status = FLT_PREOP_SYNCHRONIZE;

  return Status;
}

FLT_POSTOP_CALLBACK_STATUS
MiniFltPostCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Inout_opt_ PVOID CbdContext,
  _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
  UNREFERENCED_PARAMETER(Data);
  UNREFERENCED_PARAMETER(FltObjects);
  UNREFERENCED_PARAMETER(CbdContext);
  UNREFERENCED_PARAMETER(Flags);

  return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS
MiniFltPreCleanup(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
  UNREFERENCED_PARAMETER(Data);
  UNREFERENCED_PARAMETER(FltObjects);
  UNREFERENCED_PARAMETER(CompletionContext);

  PAGED_CODE();

  return FLT_PREOP_SUCCESS_NO_CALLBACK;
}