# 공유폴더에 접근하는 원격지 IP 구하는 방법

미니필터에서 공유폴더에 접근하는 원격지 IP를 구하는 방법이 있습니다.  
이방법은 Windows 7 부터 사용 가능한 방법입니다.  
만약 이 IP를 알아 낼 수 있으면 공유폴더에 접근하는 사용자를 IP별로 접근제어가 가능할 수 있는데요.  
먼저 필요한 구조체와 API를 살펴보도록 하겠습니다.  

```C
DEFINE_GUID(GUID_ECP_SRV_OPEN,
	0xbebfaebc,
	0xaabf,
	0x489d,
	0x9d, 0x2c, 0xe9, 0xe3, 0x61, 0x10, 0x28, 0x53);

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

typedef struct _ECP_LIST *PECP_LIST;

typedef struct sockaddr_in {
#if ...
  short          sin_family;
#else
  ADDRESS_FAMILY sin_family;
#endif
  USHORT         sin_port;
  IN_ADDR        sin_addr;
  CHAR           sin_zero[8];
} SOCKADDR_IN, *PSOCKADDR_IN;

typedef struct _SRV_OPEN_ECP_CONTEXT {
  PUNICODE_STRING       ShareName;
  PSOCKADDR_STORAGE_NFS SocketAddress;
  BOOLEAN               OplockBlockState;
  BOOLEAN               OplockAppState;
  BOOLEAN               OplockFinalState;
  USHORT                Version;
  SRV_INSTANCE_TYPE     InstanceType;
} SRV_OPEN_ECP_CONTEXT, *PSRV_OPEN_ECP_CONTEXT;

NTSTATUS FLTAPI FltGetEcpListFromCallbackData(
  PFLT_FILTER        Filter,
  PFLT_CALLBACK_DATA CallbackData,
  PECP_LIST          *EcpList
);

NTSTATUS FLTAPI FltFindExtraCreateParameter(
  PFLT_FILTER Filter,
  PECP_LIST   EcpList,
  LPCGUID     EcpType,
  PVOID       *EcpContext,
  ULONG       *EcpContextSize
);
```

일단 이 방법은 PreCreate Callback에서 호출하여 알 수 있는 방법입니다.  
커널 디버깅을 해보면 공유폴더로 접근이 될 때 srv2!SrvCreateFile가 콜스택에 찍히는걸 알 수 있습니다.  
그 srv2 는 SMB 드라이버인데 그녀석이 먼저 호출 되고 난 다음에 로컬에 있는 nt! 녀석이 호출 됩니다.  
(커널 디버깅 내용은 안쓸께요. 그냥 이런거구나 정도로만...)  
그래서 어떤 원격지에서 접근을 하는지를 알 수가 있는데요.  
그 정보는 SRV_OPEN_ECP_CONTEXT 구조체에 정보가 담겨져 옵니다.  
그렇다면 SRV_OPEN_ECP_CONTEXT는 어떻게 구하는지가 관건인데요.   
그건 바로 ECP_LIST에서 찾으실 수 있습니다.   

그렇다면 원격지 IP주소를 알아내기 위해서는 먼저  
ECP_LIST를 찾고, ECP_LIST 내에서 SRV_OPEN_ECP_CONTEXT를 찾으면 되는데   
ECP_LIST를 찾기 위해서는 FltGetEcpListFromCallbackData API를 먼저 호출해 주면 됩니다.  
그리고 ECP_LIST를 찾으면 FltFindExtraCreateParameter API를 호출해서  
SRV_OPEN_ECP_CONTEXT 를 구하면 됩니다.  
그런데 ECP_LIST에서 어떻게 찾는지는 GUID_ECP_SRV_OPEN 로 찾으면 되기 때문에  
GUID_ECP_SRV_OPEN GUID 값을 선언을 해주고 함께 넘겨주면 됩니다.   
(GUID_ECP_SRV_OPEN 은 SRV_OPEN_ECP_CONTEXT structure GUID 입니다.)   
그럼 소스코드로 한번 보도록 하겠습니다.   

```C
NTSTATUS GetSharedRemoteIP(
	_In_ PFLT_FILTER Filter,
	_Inout_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status;
	PECP_LIST pEcpList = NULL;
	PSRV_OPEN_ECP_CONTEXT ECPContext = NULL;
	GUID LpcGUID = GUID_ECP_SRV_OPEN;
	ULONG ContextSize, IPAddr;
	PSOCKADDR_IN SockAddr;

	Status = FltGetEcpListFromCallbackData(Filter, Data, &pEcpList);
	if (NT_SUCCESS(Status)) {
		if (pEcpList) {
			Status = FltFindExtraCreateParameter(Filter, pEcpList, &LpcGUID, &ECPContext, &ContextSize);
			if (NT_SUCCESS(Status)) {
				if (ECPContext) {
					DbgPrint("[TEST] FltGetNextExtraCreateParameter Success");
					SockAddr = (PSOCKADDR_IN)ECPContext->SocketAddress;
					IPAddr = SockAddr->sin_addr.S_un.S_addr;
					DbgPrint("[TEST] ECPContext IP[%u.%u.%u.%u],Port[%d]", (UCHAR)IPAddr, (IPAddr >> 8) & 0x00ff, (IPAddr >> 16) & 0x0000ff, (IPAddr >> 24) & 0x000000ff, SockAddr->sin_port);
				}
			}
			else DbgPrint("[TEST] FltGetNextExtraCreateParameter failed [0x%X]", Status);
		}
	}
	else DbgPrint("[TEST] FltGetEcpListFromCallbackData failed [0x%X]", Status);

	return Status;
}
```

위 설명대로 먼저 ECP_LIST를 구하고 그 다음에 SRV_OPEN_ECP_CONTEXT를 찾으면   
SRV_OPEN_ECP_CONTEXT 내 SocketAddress 를 보면 IP주소가 담겨져 있다는걸 확인하실 수 있습니다.   
추가적으로 SRV_OPEN_ECP_CONTEXT 구조체에는 공유 이름과, 원격지 주소, oplock 상태 정보 등이 저장되어 있습니다.   