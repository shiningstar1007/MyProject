#include "StdAfx.h"

NPAGED_LOOKASIDE_LIST g_MiniFltLookaside;
NPAGED_LOOKASIDE_LIST g_MiniSwapLookaside;

MINI_GLOBAL_DATA g_MiniData;

NTSTATUS InitializeCommPort()
{
	NTSTATUS Status;
	PSECURITY_DESCRIPTOR SecDes;
	OBJECT_ATTRIBUTES ObjAttr;
	UNICODE_STRING UniPortName;

	Status = FltBuildDefaultSecurityDescriptor(&SecDes, FLT_PORT_ALL_ACCESS);
	if (!NT_SUCCESS(Status)) return Status;

	RtlInitUnicodeString(&UniPortName, MINI_PORT_NAME);
	InitializeObjectAttributes(&ObjAttr, &UniPortName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, SecDes);

	Status = FltCreateCommunicationPort(g_MiniData.hFilter, &g_MiniData.ServerPort, &ObjAttr, NULL,
		MiniFltConnect, MiniFltDisconnect, MiniFltMessage, 1);

	if (NT_SUCCESS(Status)) {
		RtlInitUnicodeString(&UniPortName, MINI_LOG_PORT_NAME);
		InitializeObjectAttributes(&ObjAttr, &UniPortName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, SecDes);

		Status = FltCreateCommunicationPort(g_MiniData.hFilter, &g_MiniData.ServerLogPort, &ObjAttr, NULL,
			MiniFltLogConnect, MiniFltLogDisconnect, MiniFltLogMessage, 1);
	}

	FltFreeSecurityDescriptor(SecDes);

	return Status;
}

VOID FinalizeCommPort()
{
	if (g_MiniData.ServerPort != NULL) {

		FltCloseCommunicationPort(g_MiniData.ServerPort);
	}

	if (g_MiniData.ServerLogPort != NULL) {

		FltCloseCommunicationPort(g_MiniData.ServerLogPort);
	}
}

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

		{ IRP_MJ_READ, 
			0, 
			MiniFltPreRead,
			MiniFltPostRead },

    { IRP_MJ_CLEANUP,
      FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,
      MiniFltPreCleanup,
      NULL },

		{ IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION, 
			0, 
			MiniFltPreFileMapping, 
			NULL },

		{ IRP_MJ_FILE_SYSTEM_CONTROL,
			0, 
			MiniFltPreFsControl,
			NULL },

		{ IRP_MJ_SET_SECURITY, 
			0, 
			MiniFltPreSetSecurity, 
			NULL },

		{ IRP_MJ_SET_INFORMATION, 
			FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO, 
			MiniFltPreSetInformation, 
			NULL },
		
		{ IRP_MJ_MDL_READ, 
			0, 
			MiniFltPreMDLReadBuffers, 
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

NTSTATUS InitializeData(
	_In_ PUNICODE_STRING RegistryPath
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	RtlZeroMemory(&g_MiniData, sizeof(MINI_GLOBAL_DATA));

	InitializeListHead(&g_MiniData.LogBufList);

	ExInitializeNPagedLookasideList(&g_MiniFltLookaside, NULL, NULL, 0, sizeof(MINIFLT_INFO), TAG_MINIFLT, 0);
	ExInitializeNPagedLookasideList(&g_MiniSwapLookaside, NULL, NULL, 0, sizeof(MINI_SWAP_CONTEXT), TAG_MINIFLT, 0);

	KeInitializeSpinLock(&g_LogSpinLock);
	KeInitializeSpinLock(&g_MiniData.SpinLock);

	g_MiniData.DriverObject = NULL;
	g_MiniData.LogSequenceNumber = 0;
	g_MiniData.MaxLogBufCnt = MAX_RECORDS_ALLOCATE;
	g_MiniData.LogBufCnt = 0;
	g_MiniData.bFirstInitLoad = FALSE;
	g_MiniData.bFirstLoadReg = FALSE;

	g_MiniData.RegistryPath.Buffer = (PWCHAR)MyAllocNonPagedPool(RegistryPath->MaximumLength, &g_NonPagedPoolCnt);
	if (g_MiniData.RegistryPath.Buffer == NULL) DbgPrint("%s:RegistryPath allocation failed", __FUNCTION__);
	else {
		g_MiniData.RegistryPath.MaximumLength = RegistryPath->MaximumLength;
		RtlCopyUnicodeString(&g_MiniData.RegistryPath, RegistryPath);
	}

	return Status;
}

NTSTATUS FinalizeDriver(
	_In_ NTSTATUS Status
)
{
	ExDeleteNPagedLookasideList(&g_MiniFltLookaside);
	ExDeleteNPagedLookasideList(&g_MiniSwapLookaside);

	if (g_MiniData.RegistryPath.Buffer != NULL) MyFreeNonPagedPool(g_MiniData.RegistryPath.Buffer, &g_NonPagedPoolCnt);

	if (g_MiniData.ServerPort != NULL) FltCloseCommunicationPort(g_MiniData.ServerPort);

	if (g_MiniData.ServerLogPort != NULL) FltCloseCommunicationPort(g_MiniData.ServerLogPort);

	if (g_MiniData.hFilter != NULL) FltUnregisterFilter(g_MiniData.hFilter);


	return Status;
}

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ExInitializeDriverRuntime( DrvRtPoolNxOptIn );

		InitializeData(RegistryPath);
		InitializeProcess();
    
    Status = FltRegisterFilter(DriverObject, &FilterRegistration, &g_MiniData.hFilter);
    if (!NT_SUCCESS(Status)) return FinalizeDriver(Status);

		Status = FltStartFiltering(g_MiniData.hFilter);
    if (!NT_SUCCESS(Status)) return FinalizeDriver(Status);

    return Status;
}

NTSTATUS
MiniFltUnload (
  _In_ FLT_FILTER_UNLOAD_FLAGS Flags
  )
{
  UNREFERENCED_PARAMETER(Flags);

  PAGED_CODE();

  return FinalizeDriver(STATUS_SUCCESS);
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

	if (!g_MiniData.bFirstInitLoad) {
		Status = FltGetDeviceObject(FltObjects->Volume, &DeviceObj);
		if (NT_SUCCESS(Status)) {
			g_MiniData.DriverObject = DeviceObj->DriverObject;

			ObDereferenceObject(DeviceObj);
			DeviceObj = NULL;
			g_MiniData.bFirstInitLoad = TRUE;
		}
	}

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

VOID GetAlternateDataStream(PCFLT_RELATED_OBJECTS FltObjects)
{
	NTSTATUS Status;
	PFILE_STREAM_INFORMATION pStreamInfo;
	PBYTE InfoBlock;
	WCHAR StreamName[MAX_KPATH] = { 0 };
	BOOLEAN bDirectory = FALSE;

	Status = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &bDirectory);
	if (NT_SUCCESS(Status) && bDirectory) return;

	InfoBlock = (PBYTE)MyAllocNonPagedPool((1024 * 64), &g_NonPagedPoolCnt);
	
	if (!InfoBlock) return;
	else memset(InfoBlock, 0, (1024 * 64));

	pStreamInfo = (PFILE_STREAM_INFORMATION)InfoBlock;
	pStreamInfo->StreamNameLength = 0;

	Status = FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, InfoBlock,
		(1024 * 64), FileStreamInformation, NULL);

	if (NT_SUCCESS(Status)) {
		for (;;) {
			if (pStreamInfo->StreamNameLength == 0) break;

			memcpy(StreamName, pStreamInfo->StreamName, pStreamInfo->StreamNameLength);
			DbgPrint("StreamName=[%S]", StreamName);

			if (pStreamInfo->NextEntryOffset == 0) break;
			else {
				pStreamInfo = (PFILE_STREAM_INFORMATION)((PBYTE)pStreamInfo + pStreamInfo->NextEntryOffset);
			}
		}
	}
	else DbgPrint("FileStreamInformation failed[0x%X]", Status);

	MyFreeNonPagedPool(InfoBlock, &g_NonPagedPoolCnt);
}

ULONG GetVolumeId(
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
	PFILE_FS_VOLUME_INFORMATION pFsVolumeInfo;
	ULONG BufSize = sizeof(PFILE_FS_VOLUME_INFORMATION) + 200, VolumeId;

	pFsVolumeInfo = (PFILE_FS_VOLUME_INFORMATION)MyAllocNonPagedPool(BufSize, &g_NonPagedPoolCnt);
	if (pFsVolumeInfo == NULL) return 0;

	memset(pFsVolumeInfo, 0, BufSize);
	FltQueryVolumeInformationFile(FltObjects->Instance, FltObjects->FileObject, pFsVolumeInfo, BufSize, FileFsVolumeInformation, NULL);
	VolumeId = pFsVolumeInfo->VolumeSerialNumber;
	MyFreeNonPagedPool(pFsVolumeInfo, &g_NonPagedPoolCnt);

	return VolumeId;
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

PCHAR GetFileNameFromFileObject(
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
	NTSTATUS Status;
	PCHAR FileName = NULL;
	ULONG BufSize = sizeof(FILE_NAME_INFORMATION) + (MAX_KPATH * sizeof(WCHAR));
	PFILE_NAME_INFORMATION FileNameInfo = (PFILE_NAME_INFORMATION)MyAllocNonPagedPool(BufSize, &g_NonPagedPoolCnt);

	if (FileNameInfo == NULL) return NULL;

	memset(FileNameInfo, 0, BufSize);
	Status = FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, FileNameInfo,
		BufSize, FileNameInformation, NULL);

	if (NT_SUCCESS(Status)) {
		FileName = MyAllocNonPagedPool(MAX_KPATH, &g_NonPagedPoolCnt);
		if (FileName != NULL) {
			MyWideCharToChar(FileNameInfo->FileName, FileName, MAX_KPATH);
		}
	}

	return FileName;
}

PMINIFLT_INFO GetMiniFltInfo(
	_In_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ PCHAR CallFuncName
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PVOLUME_CONTEXT	pVolContext = NULL;
	PFLT_FILE_NAME_INFORMATION pFullNameInfo = NULL;
	PCHAR FileName = NULL;
	PMINIFLT_INFO MiniFltInfo = NULL;

	if (KeGetCurrentIrql() > APC_LEVEL) return NULL;

	Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &pVolContext);
	if (!NT_SUCCESS(Status)) {
		DbgPrint("%s FltGetVolumeContext Fail (Status=0x%X)", CallFuncName, Status);

		return NULL;
	}

	if (pVolContext->VolumeName.Length / 2 >= (USHORT)strlen("X:") && pVolContext->VolumeName.Buffer[1] == L':') {
		if (pVolContext->DriveType != DRIVE_NETWORK)
		{
			Status = FltGetFileNameInformation(Data,
				FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &pFullNameInfo);
			if (NT_SUCCESS(Status))
			{
				Status = FltParseFileNameInformation(pFullNameInfo);
				if (NT_SUCCESS(Status))
				{
					FileName = MakeFileName(pVolContext, pFullNameInfo);
				}
				else {
					DbgPrint("%s FltParseFileNameInformation Fail (DriveType=%d)(Status=0x%X)",
						CallFuncName, pVolContext->DriveType, Status);
				}
			}
		}
	}
	else if (pVolContext->DriveType == DRIVE_NETWORK && FltObjects->FileObject &&
		FltObjects->FileObject->FileName.Length / 2 >= 4) {
		WCHAR *pSep, FileNameW[MAX_KPATH] = { 0 }, DrvName[3] = { 0 };

		MyStrNCopyW(FileNameW, FltObjects->FileObject->FileName.Buffer, FltObjects->FileObject->FileName.Length / 2, MAX_KPATH);
		//2008: \;LanmanRedirector\;Z:0000000000028ab3\192.168.150.202\D$\desktop.ini
		//2012: \;Z:0000000000027a36\192.168.150.202\Enc
		pSep = wcschr(FileNameW, L':');
		if (pSep && pSep - FileNameW > 2) {
			MyStrNCopyW(DrvName, pSep - 1, -1, 3); //Copy Z:
		}
		Status = FltGetFileNameInformation(Data,
			FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &pFullNameInfo);
		if (NT_SUCCESS(Status)) {
			Status = FltParseFileNameInformation(pFullNameInfo);
			if (NT_SUCCESS(Status)) FileName = MakeFileName(pVolContext, pFullNameInfo);
			else {
				DbgPrint("%s FltParseFileNameInformation Fail (DriveType=%d)(Status=0x%X)",
					CallFuncName, pVolContext->DriveType, Status);
			}
		}
	}

	if (FileName == NULL) {
		FileName = MakeFileNameByFileObj(pVolContext, FltObjects->FileObject);
		if (FileName == NULL) {
			DbgPrint("%s MakeFilePathByFileObj failed (DriveType=%d)", CallFuncName, pVolContext->DriveType);
		}
		else FileName = GetFileNameFromFileObject(FltObjects);
	}

	if (pFullNameInfo != NULL) FltReleaseFileNameInformation(pFullNameInfo);

	if (pVolContext != NULL) FltReleaseContext(pVolContext);

	if (FileName != NULL) {
		MiniFltInfo = ExAllocateFromNPagedLookasideList(&g_MiniFltLookaside);
		if (MiniFltInfo != NULL) {
			MyStrNCopy(MiniFltInfo->FileName, FileName, MAX_KPATH);
		}

		MyFreeNonPagedPool(FileName, &g_NonPagedPoolCnt);
	}

	return MiniFltInfo;
}

PCHAR MakeFileName(
	_In_ PVOLUME_CONTEXT pVolContext,
	_In_ PFLT_FILE_NAME_INFORMATION NameInfo
)
{
	ULONG Len = 0;
	WCHAR FileNameW[MAX_KPATH] = { 0 };
	PCHAR FileName = MyAllocNonPagedPool(MAX_KPATH, &g_NonPagedPoolCnt);

	if (!FileName) return NULL;

	memset(FileName, 0, MAX_KPATH);

	Len += MyStrNCopyW(FileNameW + Len, pVolContext->VolumeName.Buffer,
		pVolContext->VolumeName.Length / 2, MAX_KPATH - Len);

	if (!NameInfo->ParentDir.Buffer || NameInfo->ParentDir.Length > 260) {
		Len += MyStrNCopyW(FileNameW + Len, L"\\", 1, MAX_KPATH - Len);
	}
	else {
		Len += MyStrNCopyW(FileNameW + Len, NameInfo->ParentDir.Buffer,
			NameInfo->ParentDir.Length / 2, MAX_KPATH - Len);
		Len += MyStrNCopyW(FileNameW + Len, NameInfo->FinalComponent.Buffer,
			NameInfo->FinalComponent.Length / 2, MAX_KPATH - Len);
	}

	MyWideCharToChar(FileNameW, FileName, MAX_KPATH);

	return FileName;
}

PCHAR MakeFileNameByFileObj(
	_In_ PVOLUME_CONTEXT pVolContext,
	_In_ PFILE_OBJECT FileObject
)
{
	WCHAR FileNameW[MAX_KPATH], * pBuf, * pDrive;
	PCHAR FileName;
	ULONG Len = 0;

	if (!FileObject || !FileObject->FileName.Buffer || FileObject->FileName.Length == 0) return NULL;

	FileName = MyAllocNonPagedPool(MAX_KPATH, &g_NonPagedPoolCnt);
	if (!FileName) return NULL;

	MyStrNCopyW(FileNameW, FileObject->FileName.Buffer, FileObject->FileName.Length / 2, MAX_KPATH);
	if (pVolContext->DriveType == DRIVE_NETWORK) {
		//2012: \;Z:0000000000027a36\192.168.150.202\TestDir
		pBuf = wcschr(FileNameW, L':');
		if (pBuf && (pBuf + 1)) {
			pBuf = wcschr(pBuf, L'\\');
			if (pBuf && (pBuf + 1)) {
				Len = MySNPrintfW(FileNameW, MAX_KPATH, L"\\"); // prefix '\'
				MyStrNCopyW(FileNameW + Len, pBuf, -1, MAX_KPATH - Len);
			}
		}
		else {
			Len = MySNPrintfW(FileNameW, MAX_KPATH, L"\\"); // prefix '\'
			MyStrNCopyW(FileNameW + Len, FileNameW, -1, MAX_KPATH - Len);
		}
	}
	else {
		if (!_wcsicmp(FileObject->FileName.Buffer, L"\\Device")) {
			pBuf = wcschr(FileNameW, L'\\');
			if (pBuf && (pBuf + 1)) {
				pBuf = wcschr(pBuf + 1, L'\\');
				if (pBuf) {
					pDrive = wcschr(pBuf, L':');
					if (!pDrive) Len = MyStrNCopyW(FileNameW, pVolContext->VolumeName.Buffer, pVolContext->VolumeName.Length / 2, MAX_KPATH);

					MyStrNCopyW(FileNameW + Len, pBuf, -1, MAX_KPATH - Len);
				}
			}
		}
		else {
			if (pVolContext->VolumeName.Length / 2 >= (USHORT)strlen("X:") && pVolContext->VolumeName.Buffer[1] == L':') {
				pDrive = wcschr(FileNameW, L':');
				if (!pDrive) {
					Len = MyStrNCopyW(FileNameW, pVolContext->VolumeName.Buffer, pVolContext->VolumeName.Length / 2, MAX_KPATH);
					if (Len > 2) Len += MySNPrintfW(FileNameW + Len, MAX_KPATH - Len, L"\\");
				}

				MyStrNCopyW(FileNameW + Len, FileNameW, -1, MAX_KPATH);
			}
		}
	}

	if (FileNameW[0] != 0) {
		MyWideCharToChar(FileNameW, FileName, MAX_KPATH);
	}
	else {
		MyFreeNonPagedPool(FileName, &g_NonPagedPoolCnt);
		FileName = NULL;
	}

	return FileName;
}

PMINIFLT_INFO GetNewMiniFltInfo(
	_In_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects
)
{
	NTSTATUS Status;
	PVOLUME_CONTEXT	pVolContext = NULL;
	PFILE_RENAME_INFORMATION RenameInfo;
	PFLT_FILE_NAME_INFORMATION NameInfo = NULL;
	PCHAR NewFilePath = NULL;
	PMINIFLT_INFO MiniFltInfo = NULL;

	PAGED_CODE();

	if (KeGetCurrentIrql() > APC_LEVEL) return NULL;

	Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &pVolContext);
	if (!NT_SUCCESS(Status)) return NULL;

	if (pVolContext->VolumeName.Length / 2 >= (USHORT)strlen("X:") && pVolContext->VolumeName.Buffer[1] == L':') {
		RenameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
		Status = FltGetDestinationFileNameInformation(Data->Iopb->TargetInstance, Data->Iopb->TargetFileObject,
			RenameInfo->RootDirectory, RenameInfo->FileName, RenameInfo->FileNameLength,
			FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &NameInfo);
		if (NT_SUCCESS(Status)) {
			Status = FltParseFileNameInformation(NameInfo);
			if (NT_SUCCESS(Status)) NewFilePath = MakeFileName(pVolContext, NameInfo);
			else DbgPrint("NewFilePath FltParseFileNameInformation Fail (Status=0x%X)", Status);
		}
		else DbgPrint("NewFilePath FltGetFileNameInformation Fail (Status=0x%X)", Status);

		if (NameInfo) FltReleaseFileNameInformation(NameInfo);
	}

	if (pVolContext != NULL) FltReleaseContext(pVolContext);

	if (NewFilePath != NULL) {
		MiniFltInfo = ExAllocateFromNPagedLookasideList(&g_MiniFltLookaside);
		if (MiniFltInfo != NULL) {
			MyStrNCopy(MiniFltInfo->FileName, NewFilePath, MAX_KPATH);
		}

		MyFreeNonPagedPool(NewFilePath, &g_NonPagedPoolCnt);
	}

	return MiniFltInfo;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
  FLT_PREOP_CALLBACK_STATUS Status = FLT_PREOP_SYNCHRONIZE;
	NTSTATUS NtStatus;
	ULONG Action, Options = Data->Iopb->Parameters.Create.Options;
	PMINIFLT_INFO MiniFltInfo;
	UNREFERENCED_PARAMETER(CompletionContext);

	PAGED_CODE();

	if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE) || FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN))
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	Action = GetAction(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess);
	if (!(Options & FILE_DELETE_ON_CLOSE) || Action != ACTION_DELETE) return Status;

	MiniFltInfo = GetMiniFltInfo(Data, FltObjects, "MiniFltPreCreate");
	if (MiniFltInfo != NULL) {
		NtStatus = GetUserName(Data, MiniFltInfo);
		NtStatus = GetProcessImageName(Data, MiniFltInfo);

		ExFreeToNPagedLookasideList(&g_MiniFltLookaside, MiniFltInfo);
	}

  return Status;
}

FLT_POSTOP_CALLBACK_STATUS MiniFltPostCreate(
  _Inout_ PFLT_CALLBACK_DATA Data,
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _Inout_opt_ PVOID CbdContext,
  _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	PAGED_CODE();
	FLT_PREOP_CALLBACK_STATUS Status = FLT_POSTOP_FINISHED_PROCESSING;
	NTSTATUS NtStatus;
	ULONG Action, Disp = (Data->Iopb->Parameters.Create.Options >> 24) & 0xFF;
	PMINIFLT_INFO MiniFltInfo = (PMINIFLT_INFO)CbdContext;
	PMINI_FLT_CONTEXT MiniContext = NULL;

	if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING) || !NT_SUCCESS(Data->IoStatus.Status)) return Status;

	if (MiniFltInfo == NULL) MiniFltInfo = GetMiniFltInfo(Data, FltObjects, "MiniFltPostCreate");

	if (MiniFltInfo != NULL) {
		NtStatus = GetUserName(Data, MiniFltInfo);
		NtStatus = GetProcessImageName(Data, MiniFltInfo);

		Action = GetAction(Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess);

		__try {
			Status = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
			if (!NT_SUCCESS(Status)) {
				Status = CreateMiniFltContext(FLT_STREAM_CONTEXT, Data, g_MiniData.hFilter, &MiniContext, MiniFltInfo);
				if (!NT_SUCCESS(Status)) {
					DbgPrint("PostCreate: Failed to Create Stream Context (FileObject = %p)", FltObjects->FileObject);
				}
			}

			if (MiniContext != NULL) {

				FltReleaseContext(MiniContext);
				MiniContext = NULL;
			}

			Status = CreateMiniFltContext(FLT_STREAMHANDLE_CONTEXT, Data, g_MiniData.hFilter, &MiniContext, MiniFltInfo);
			if (!NT_SUCCESS(Status)) {
				DbgPrint("PostCreate: Failed to Create StreamHandle Context (FileObject = %p)", FltObjects->FileObject);
			}
		}
		__finally {
			if (MiniContext != NULL) {

				FltReleaseContext(MiniContext);
			}
		}

		if (Data->IoStatus.Information == FILE_CREATE) {
			FILE_DISPOSITION_INFORMATION DeleteInfo;

			DeleteInfo.DeleteFile = TRUE;
			FltSetInformationFile(FltObjects->Instance, FltObjects->FileObject, &DeleteInfo,
				sizeof(FILE_DISPOSITION_INFORMATION), FileDispositionInformation);
		}

		FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);
		Data->IoStatus.Status = STATUS_ACCESS_DENIED;
		Data->IoStatus.Information = 0;
	}

	if (MiniFltInfo != NULL) ExFreeToNPagedLookasideList(&g_MiniFltLookaside, MiniFltInfo);

	return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreRead(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	NTSTATUS Status;
	PFLT_IO_PARAMETER_BLOCK pIopb = Data->Iopb;
	FLT_PREOP_CALLBACK_STATUS RetStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PVOID NewBuf = NULL;
	PMDL NewMdl = NULL;
	PVOLUME_CONTEXT VolContext = NULL;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	PMINI_SWAP_CONTEXT SwapContext = NULL;
	ULONG ReadLen = pIopb->Parameters.Read.Length;

	__try {
		if (ReadLen == 0) __leave;

		Status = FltGetStreamHandleContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (!NT_SUCCESS(Status)) {

			MiniContext = NULL;
			Status = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
			if (!NT_SUCCESS(Status)) {

				MiniContext = NULL;
				__leave;
			}
		}

		Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolContext);
		if (!NT_SUCCESS(Status)) __leave;

		if (FlagOn(IRP_NOCACHE, pIopb->IrpFlags)) ReadLen = (ULONG)ROUND_TO_SIZE(ReadLen, VolContext->SectorSize);

		NewBuf = FltAllocatePoolAlignedWithTag(FltObjects->Instance, NonPagedPool, (SIZE_T)ReadLen, TAG_MINIFLT);
		if (NewBuf == NULL) __leave;

		if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION)) {
			NewMdl = IoAllocateMdl(NewBuf, ReadLen, FALSE, FALSE, NULL);
			if (NewMdl == NULL) __leave;

			MmBuildMdlForNonPagedPool(NewMdl);
		}

		SwapContext = ExAllocateFromNPagedLookasideList(&g_MiniSwapLookaside);
		if (SwapContext == NULL) __leave;

		pIopb->Parameters.Read.ReadBuffer = NewBuf;
		pIopb->Parameters.Read.MdlAddress = NewMdl;
		FltSetCallbackDataDirty(Data);

		SwapContext->SwappedBuffer = NewBuf;
		SwapContext->VolContext = VolContext;
		SwapContext->MiniContext = MiniContext;

		*CompletionContext = SwapContext;

		RetStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}
	__finally {

		if (RetStatus != FLT_PREOP_SUCCESS_WITH_CALLBACK) {

			if (NewBuf != NULL) FltFreePoolAlignedWithTag(FltObjects->Instance, NewBuf, TAG_MINIFLT);
			
			if (NewMdl != NULL) IoFreeMdl(NewMdl);

			if (VolContext != NULL) FltReleaseContext(VolContext);

			if (MiniContext != NULL) FltReleaseContext(MiniContext);
		}
	}

	return RetStatus;
}

FLT_POSTOP_CALLBACK_STATUS MiniFltPostRead(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	PFLT_IO_PARAMETER_BLOCK pIopb = Data->Iopb;
	FLT_POSTOP_CALLBACK_STATUS RetStatus = FLT_POSTOP_FINISHED_PROCESSING;
	PMINI_SWAP_CONTEXT SwapContext = CompletionContext;
	PVOID OrgBuf = NULL;
	BOOL bFreeBuf = TRUE;

	FLT_ASSERT(!FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING));

	__try {
		if (!NT_SUCCESS(Data->IoStatus.Status) || (Data->IoStatus.Information == 0)) __leave;

		if (pIopb->Parameters.Read.MdlAddress != NULL) {

			FLT_ASSERT(((PMDL)pIopb->Parameters.Read.MdlAddress)->Next == NULL);

			OrgBuf = MmGetSystemAddressForMdlSafe(pIopb->Parameters.Read.MdlAddress, NormalPagePriority | MdlMappingNoExecute);

			if (OrgBuf == NULL) {
				Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
				Data->IoStatus.Information = 0;
				__leave;
			}
		}
		else if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) || FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION)) {
			OrgBuf = pIopb->Parameters.Read.ReadBuffer;
		}
		else {
			if (FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, MiniFltPostReadWhenSafe, &RetStatus)) {
				bFreeBuf = FALSE;
			}
			else {
				Data->IoStatus.Status = STATUS_UNSUCCESSFUL;
				Data->IoStatus.Information = 0;
			}

			__leave;
		}

		__try {
			RtlCopyMemory(OrgBuf, SwapContext->SwappedBuffer, Data->IoStatus.Information);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {

			Data->IoStatus.Status = GetExceptionCode();
			Data->IoStatus.Information = 0;
		}
	}
	__finally {

		if (bFreeBuf) {

			FltFreePoolAlignedWithTag(FltObjects->Instance, SwapContext->SwappedBuffer, TAG_MINIFLT);
			FltReleaseContext(SwapContext->MiniContext);
			FltReleaseContext(SwapContext->VolContext);

			ExFreeToNPagedLookasideList(&g_MiniSwapLookaside, SwapContext);
		}
	}

	return RetStatus;
}

FLT_POSTOP_CALLBACK_STATUS MiniFltPostReadWhenSafe(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	PFLT_IO_PARAMETER_BLOCK pIopb = Data->Iopb;
	PMINI_SWAP_CONTEXT SwapContext = CompletionContext;
	PVOID OrgBuf = NULL;
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	FLT_ASSERT(Data->IoStatus.Information != 0);

	Status = FltLockUserBuffer(Data);

	if (!NT_SUCCESS(Status)) {
		Data->IoStatus.Status = Status;
		Data->IoStatus.Information = 0;
	}
	else {
		OrgBuf = MmGetSystemAddressForMdlSafe(pIopb->Parameters.Read.MdlAddress, NormalPagePriority);
		if (OrgBuf == NULL) {
			Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			Data->IoStatus.Information = 0;
		}
		else {
			RtlCopyMemory(OrgBuf, SwapContext->SwappedBuffer, Data->IoStatus.Information);
		}
	}

	FltFreePoolAlignedWithTag(FltObjects->Instance, SwapContext->SwappedBuffer, TAG_MINIFLT);
	FltReleaseContext(SwapContext->MiniContext);
	FltReleaseContext(SwapContext->VolContext);

	ExFreeToNPagedLookasideList(&g_MiniSwapLookaside, SwapContext);

	return Status;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	PFLT_IO_PARAMETER_BLOCK pIopb = Data->Iopb;
	FLT_PREOP_CALLBACK_STATUS RetValue = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PVOID NewBuf = NULL;
	PMDL NewMdl = NULL;
	PVOLUME_CONTEXT VolContext = NULL;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	PMINI_SWAP_CONTEXT SwapContext = NULL;
	PVOID OrgBuf = NULL;
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG WriteLen = Data->Iopb->Parameters.Write.Length;

	__try {
		if (WriteLen == 0) leave;

		Status = FltGetStreamHandleContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (!NT_SUCCESS(Status)) {

			MiniContext = NULL;
			Status = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
			if (!NT_SUCCESS(Status)) {

				MiniContext = NULL;
				__leave;
			}
		}

		Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolContext);
		if (!NT_SUCCESS(Status)) {
			__leave;
		}

		if (FlagOn(IRP_NOCACHE, pIopb->IrpFlags)) {
			WriteLen = (ULONG)ROUND_TO_SIZE(WriteLen, VolContext->SectorSize);
		}

		NewBuf = FltAllocatePoolAlignedWithTag(FltObjects->Instance, NonPagedPool, (SIZE_T)WriteLen, TAG_MINIFLT);
		if (NewBuf == NULL) {
			__leave;
		}

		if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION)) {
			NewMdl = IoAllocateMdl(NewBuf, WriteLen, FALSE, FALSE, NULL);

			if (NewMdl == NULL) {
				__leave;
			}

			MmBuildMdlForNonPagedPool(NewMdl);
		}

		if (pIopb->Parameters.Write.MdlAddress != NULL) {
			FLT_ASSERT(((PMDL)pIopb->Parameters.Write.MdlAddress)->Next == NULL);

			OrgBuf = MmGetSystemAddressForMdlSafe(pIopb->Parameters.Write.MdlAddress, NormalPagePriority | MdlMappingNoExecute);

			if (OrgBuf == NULL) {
				Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
				Data->IoStatus.Information = 0;
				RetValue = FLT_PREOP_COMPLETE;
				__leave;
			}
		}
		else {
			OrgBuf = pIopb->Parameters.Write.WriteBuffer;
		}

		__try {
			RtlCopyMemory(NewBuf, OrgBuf, WriteLen);
		} 
		__except(EXCEPTION_EXECUTE_HANDLER) {

			Data->IoStatus.Status = GetExceptionCode();
			Data->IoStatus.Information = 0;
			RetValue = FLT_PREOP_COMPLETE;
			__leave;
		}

		SwapContext = ExAllocateFromNPagedLookasideList(&g_MiniSwapLookaside);
		if (SwapContext == NULL) __leave;

		pIopb->Parameters.Write.WriteBuffer = NewBuf;
		pIopb->Parameters.Write.MdlAddress = NewMdl;
		FltSetCallbackDataDirty(Data);

		SwapContext->SwappedBuffer = NewBuf;
		SwapContext->VolContext = VolContext;
		*CompletionContext = SwapContext;

		RetValue = FLT_PREOP_SUCCESS_WITH_CALLBACK;
	}
	__finally {
		if (RetValue != FLT_PREOP_SUCCESS_WITH_CALLBACK) {

			if (NewBuf != NULL) FltFreePoolAlignedWithTag(FltObjects->Instance, NewBuf, TAG_MINIFLT);

			if (NewMdl != NULL) IoFreeMdl(NewMdl);

			if (VolContext != NULL) FltReleaseContext(VolContext);
		}

		if (MiniContext != NULL) FltReleaseContext(MiniContext);
	}

	return RetValue;
}

FLT_POSTOP_CALLBACK_STATUS MiniFltPostWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	PMINI_SWAP_CONTEXT SwapContext = CompletionContext;
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(Data);

	FltFreePoolAlignedWithTag(FltObjects->Instance, SwapContext->SwappedBuffer, TAG_MINIFLT);
	FltReleaseContext(SwapContext->VolContext);

	ExFreeToNPagedLookasideList(&g_MiniSwapLookaside, SwapContext);

	return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreCleanup(
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

FLT_PREOP_CALLBACK_STATUS MiniFltPreSetSecurity(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS Status = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PMINIFLT_INFO MiniFltInfo = NULL;

	UNREFERENCED_PARAMETER(CompletionContext);

	PAGED_CODE();
	//write operation

	MiniFltInfo = GetMiniFltInfo(Data, FltObjects, "MiniFltPreSetSecurity");
	if (!MiniFltInfo) return Status;

	if (MiniFltInfo != NULL) ExFreeToNPagedLookasideList(&g_MiniFltLookaside, MiniFltInfo);

	return Status;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreSetInformation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS RetStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	NTSTATUS Status = STATUS_SUCCESS;
	PMINIFLT_INFO MiniFltInfo = NULL, NewMiniFltInfo = NULL;
	PVOLUME_CONTEXT VolumeContext = NULL;
	UNREFERENCED_PARAMETER(CompletionContext);

	PAGED_CODE();

	switch (Data->Iopb->Parameters.SetFileInformation.FileInformationClass) {
	case FileRenameInformation: // rename
	case FileRenameInformationEx:
		Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolumeContext);
		if (NT_SUCCESS(Status) && VolumeContext->DriveType == DRIVE_FIXED) {
			MiniFltInfo = GetMiniFltInfo(Data, FltObjects, "PsKePreSetInformation");
			NewMiniFltInfo = GetNewMiniFltInfo(Data, FltObjects);
			Status = GetUserName(Data, MiniFltInfo);
			Status = GetProcessImageName(Data, MiniFltInfo);
		}

		break;

	case FileDispositionInformation: // delete
	case FileDispositionInformationEx:
		MiniFltInfo = GetMiniFltInfo(Data, FltObjects, "PsKePreSetInformation");
		Status = GetUserName(Data, MiniFltInfo);
		Status = GetProcessImageName(Data, MiniFltInfo);

		break;
	}

	if (MiniFltInfo != NULL) ExFreeToNPagedLookasideList(&g_MiniFltLookaside, MiniFltInfo);

	if (NewMiniFltInfo != NULL) ExFreeToNPagedLookasideList(&g_MiniFltLookaside, NewMiniFltInfo);

	if (VolumeContext != NULL) FltReleaseContext(VolumeContext);

	return RetStatus;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreFileMapping(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	NTSTATUS Status;
	FS_FILTER_SECTION_SYNC_TYPE SyncType = Data->Iopb->Parameters.AcquireForSectionSynchronization.SyncType;
	ULONG Option = (PAGE_READONLY | PAGE_READWRITE);
	ULONG PageProtection = Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection;
	PVOLUME_CONTEXT VolumeContext = NULL;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	UNREFERENCED_PARAMETER(CompletionContext);

	PAGED_CODE();

	if (SyncType == SyncTypeCreateSection) {
		Status = FltGetStreamHandleContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (NT_SUCCESS(Status)) {
			Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolumeContext);
			if (NT_SUCCESS(Status)) {

				if (VolumeContext->DriveType == DRIVE_NETWORK) {

				}
				else {

				}
			}
		}

		if (MiniContext != NULL) FltReleaseContext(MiniContext);

		if (VolumeContext != NULL) FltReleaseContext(VolumeContext);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreFsControl(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS RetStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PVOLUME_CONTEXT VolumeContext = NULL;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(CompletionContext);
	PAGED_CODE();

	__try {
		Status = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (NT_SUCCESS(Status)) {
			Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolumeContext);
			if (!NT_SUCCESS(Status)) {

				__leave;
			}

			if ((VolumeContext->DriveType == DRIVE_NETWORK) &&
				(Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode == IOCTL_LMR_DISABLE_LOCAL_BUFFERING)) {

				Data->IoStatus.Status = STATUS_NOT_SUPPORTED;
				RetStatus = FLT_PREOP_COMPLETE;
				DbgPrint("%s: IOCTL_LMR_DISABLE_LOCAL_BUFFERING", __FUNCTION__);
			}
		}
	}
	__finally {

		if (MiniContext != NULL) FltReleaseContext(MiniContext);

		if (VolumeContext != NULL) FltReleaseContext(VolumeContext);
	}

	return RetStatus;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreDeviceControl(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS RetStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PVOLUME_CONTEXT VolumeContext = NULL;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(CompletionContext);
	PAGED_CODE();

	__try {

		Status = FltGetFileContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (NT_SUCCESS(Status)) {

			Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolumeContext);
			if (NT_SUCCESS(Status) && (VolumeContext->DriveType == DRIVE_NETWORK)) {

				if ((Data->Iopb->Parameters.DeviceIoControl.Common.IoControlCode == IOCTL_QUERY_REMOTE_SERVER_NAME) ||
					(Data->Iopb->Parameters.DeviceIoControl.Common.IoControlCode == IOCTL_COPYCHUNK)) {

					Data->IoStatus.Status = STATUS_NOT_SUPPORTED;
					RetStatus = FLT_PREOP_COMPLETE;
				}
			}
		}
	}
	__finally {

		if (MiniContext != NULL) FltReleaseContext(MiniContext);

		if (VolumeContext != NULL) FltReleaseContext(VolumeContext);
	}

	return RetStatus;
}

FLT_PREOP_CALLBACK_STATUS MiniFltPreMDLReadBuffers(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS RetStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	PMINI_FLT_CONTEXT MiniContext = NULL;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(CompletionContext);

	__try {

		if (!FltSupportsStreamHandleContexts(FltObjects->FileObject)) {

			__leave;
		}

		Status = FltGetStreamHandleContext(FltObjects->Instance, FltObjects->FileObject, &MiniContext);
		if (!NT_SUCCESS(Status)) {

			MiniContext = NULL;
			__leave;
		}

		if (FLT_IS_FASTIO_OPERATION(Data)) {

			RetStatus = FLT_PREOP_DISALLOW_FASTIO;
			__leave;
		}
	}
	__finally {

		if (MiniContext != NULL) {

			FltReleaseContext(MiniContext);
		}
	}

	return RetStatus;
}
