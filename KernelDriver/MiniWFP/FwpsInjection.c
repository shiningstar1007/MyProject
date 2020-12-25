#include "StdAfx.h"
#include "FwpsInjection.h"

NTSTATUS InjectionHandleCreate(_Outptr_ INJECTION_HANDLE** pInjectionHandle,
  _In_ ADDRESS_FAMILY AddressFamily,                      /* AF_INET */
  _In_ BOOLEAN IsInbound,                                 /* TRUE */
  _In_ UINT32 Index)                                      /* WFPSAMPLER_INDEX */
{
  NTSTATUS Status = STATUS_SUCCESS;

  __try {

    *pInjectionHandle = (PINJECTION_HANDLE)MyAllocNonPagedPool(sizeof(INJECTION_HANDLE), &g_InjectNPPoolCnt);
    if (*pInjectionHandle == NULL) __leave;
    else RtlSecureZeroMemory(*pInjectionHandle, sizeof(INJECTION_HANDLE));

    Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_STREAM, (*pInjectionHandle)->pStreamHandle);
    if (!NT_SUCCESS(Status)) __leave;

    //Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_NETWORK, (*pInjectionHandle)->pNetworkHandle);
    //if (!NT_SUCCESS(Status)) __leave;

    //Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_TRANSPORT, (*pInjectionHandle)->pTransportHandle);
    //if (!NT_SUCCESS(Status)) __leave;

    //Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_L2, (*pInjectionHandle)->pMACHandle);
    //if (!NT_SUCCESS(Status)) __leave;

    //Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_L2, (*pInjectionHandle)->pVSwitchEthernetHandle);
    //if (!NT_SUCCESS(Status)) __leave;

    //Status = FwpsInjectionHandleCreate(AddressFamily, FWPS_INJECTION_TYPE_FORWARD, (*pInjectionHandle)->pForwardHandle);
    //if (!NT_SUCCESS(Status)) __leave;

  }
  __finally {
    if (!NT_SUCCESS(Status)) {
      if (*pInjectionHandle != NULL) {
        InjectionHandleRelease(pInjectionHandle);
      }
    }
  }

  return Status;
}

_At_(*pInjectionHandle, _Pre_ _Notnull_)
_At_(*pInjectionHandle, _Post_ _Null_ __drv_freesMem(Pool))
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
_Success_(return == STATUS_SUCCESS)
NTSTATUS InjectionHandleRelease(INJECTION_HANDLE** pInjectionHandle)
{
  NTSTATUS Status = STATUS_SUCCESS;

  if (*pInjectionHandle == NULL) return Status;

  if ((*pInjectionHandle)->pStreamHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pStreamHandle);
  }

  if ((*pInjectionHandle)->pNetworkHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pNetworkHandle);
  }

  if ((*pInjectionHandle)->pTransportHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pTransportHandle);
  }

  if ((*pInjectionHandle)->pMACHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pMACHandle);
  }

  if ((*pInjectionHandle)->pVSwitchEthernetHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pVSwitchEthernetHandle);
  }

  if ((*pInjectionHandle)->pForwardHandle) {
    FwpsInjectionHandleDestroy((*pInjectionHandle)->pForwardHandle);
  }

  MyFreeNonPagedPool(*pInjectionHandle, &g_InjectNPPoolCnt);

  return Status;
}
