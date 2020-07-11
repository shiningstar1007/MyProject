#include "MiniFlt.h"

PFLT_FILTER g_hFilter;

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

VOID
MiniFltContextCleanup (
    _In_ PFLT_CONTEXT Context,
    _In_ FLT_CONTEXT_TYPE ContextType
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

VOID
MiniFltContextCleanup (
  _In_ PFLT_CONTEXT Context,
  _In_ FLT_CONTEXT_TYPE ContextType
  )
{
  UNREFERENCED_PARAMETER(Context);
  UNREFERENCED_PARAMETER(ContextType);
  
  PAGED_CODE();
}

NTSTATUS
MiniFltInstanceSetup(
  _In_ PCFLT_RELATED_OBJECTS FltObjects,
  _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
  _In_ DEVICE_TYPE VolumeDeviceType,
  _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
  UNREFERENCED_PARAMETER(FltObjects);
  UNREFERENCED_PARAMETER(Flags);
  UNREFERENCED_PARAMETER(VolumeDeviceType);
  UNREFERENCED_PARAMETER(VolumeFilesystemType);
  NTSTATUS Status = STATUS_SUCCESS;

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