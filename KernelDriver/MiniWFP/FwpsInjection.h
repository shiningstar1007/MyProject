#ifndef __FWPSINJECTION_H__
#define __FWPSINJECTION_H__

#include <wdf.h>
#include <ntdef.h>

typedef struct _INJECTION_HANDLE
{
  HANDLE *pMACHandle;
  HANDLE *pVSwitchEthernetHandle;
  HANDLE *pForwardHandle;
  HANDLE *pNetworkHandle;
  HANDLE *pTransportHandle;
  HANDLE *pStreamHandle;
} INJECTION_HANDLE, * PINJECTION_HANDLE;

NTSTATUS InjectionHandleCreate(_Outptr_ INJECTION_HANDLE * *pInjectionHandle,
  _In_ ADDRESS_FAMILY AddressFamily,                      /* AF_INET */
  _In_ BOOLEAN IsInbound,                                 /* TRUE */
  _In_ UINT32 Index);                                     /* WFPSAMPLER_INDEX */

_At_(*pInjectionHandle, _Pre_ _Notnull_)
_At_(*pInjectionHandle, _Post_ _Null_ __drv_freesMem(Pool))
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
_Success_(return == STATUS_SUCCESS)
NTSTATUS InjectionHandleRelease(INJECTION_HANDLE** pInjectionHandle);

#endif