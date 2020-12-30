# 레지스트리 IO를 콜백 받는 방법

레지스트리의 읽기,쓰기,삭제 등등.. 이런 I/O가 동작할 때  
커널에서 콜백을 받아서 확인 할 수 있는 방법이 있습니다.  
이 기술을 사용하면 레지스트리를 보호 할 수 있는데   
MS에서 제공하는 API를 사용하여 안정성을 보장받는 방법에 대해서 기술해보겠습니다.  

먼저 API 함수는 다음과 같습니다.  

```c
EX_CALLBACK_FUNCTION ExCallbackFunction;

NTSTATUS ExCallbackFunction(
  PVOID CallbackContext,
  PVOID Argument1,
  PVOID Argument2
)
{...}

NTSTATUS CmRegisterCallbackEx(
  PEX_CALLBACK_FUNCTION Function,
  PCUNICODE_STRING         Altitude,
  PVOID                          Driver,
  PVOID                          Context,
  PLARGE_INTEGER             Cookie,
  PVOID                          Reserved
);

NTSTATUS CmUnRegisterCallback(
  LARGE_INTEGER Cookie
);
```

기존에 CmRegisterCallback API가 있었는데 Vista 이후부터는 사용하지 말라고 합니다.  
CmRegisterCallback API에 비해 파라미터로 넘겨줘야 하는게 몇개 더 늘어나긴 했는데  
Altitude, Driver, Reserved 3개가 추가 되었으며 API 원형에 대한 설명은 MSDN에 나와 있으니  
확인해보시면 되겠습니다.  

그러면 위 API를 사용하는 방법은 아래와 같습니다.  

```c
LARGE_INTEGER g_RegisterCookie = { 0 };
NTSTATUS RegisterCallback(
	_In_ PVOID CallbackContext, 
	_In_ PVOID Argument1, 
	_In_ PVOID Argument2
)
{
	REG_NOTIFY_CLASS RegNotifyType = (REG_NOTIFY_CLASS)Argument1;
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(CallbackContext);

	switch (RegNotifyType) {
		case RegNtPreCreateKeyEx: {
			PREG_CREATE_KEY_INFORMATION pRegCreateKey = (PREG_CREATE_KEY_INFORMATION)Argument2;

			if (pRegCreateKey != NULL) {
				Status = DecisionRegCallback(ACTION_KEY_CREATE, pRegCreateKey->RootObject, pRegCreateKey->CompleteName);
			}

			break;
		}
             }

	return Status;
}

VOID StartRegHook(_In_ PDEVICE_OBJECT DeviceObject)
{
	UNICODE_STRING AltitudeString;
	
	if (g_RegisterCookie.QuadPart != 0) return;

	RtlInitUnicodeString(&AltitudeString, L"370071");

	CmRegisterCallbackEx(RegisterCallback, &AltitudeString, 
		DeviceObject->DriverObject, NULL, &g_RegisterCookie, NULL);
}

VOID StopRegHook()
{
	if (g_RegisterCookie.QuadPart == 0) return;

	CmUnRegisterCallback(g_RegisterCookie);
	g_RegisterCookie.QuadPart = 0;
}
```

먼저 CmRegisterCallbackEx API를 호출하여 레지스트리 콜백 등록을 합니다.  
여기서 AltitudeString은 본인이 제작한 드라이버의 Altitude를 넣어주시면 되고   
그리고 가장 중요한 RegisterCallback 함수를 등록해주는데  
RegisterCallback 함수는 직접 정의를 하면 됩니다.  
그리고 g_RegisterCookie는 LARGE_INTEGER 로 선언하여 사용하면 됩니다.  
설명은 MSDN에 나와 있으니 생략하도록 하겠습니다.  

RegisterCallback 함수를 이제 콜백 등록을 하여 레지스트리에 대한  
I/O가 발생 했을 시 RegisterCallback 함수로 콜백이 들어오는데   
여기서 Argument1은 레지스트리에 어떤 I/O가 발생했는지를 확인 할 수 있으며,  
Argument2는 해당 작업에 대한 정보가 전달되어 넘어오게 됩니다.  
따라서 Argument1과 Argument2를 적절한 상황에서 사용하여  
레지스트리에 대한 I/O 제어를 할 수 있습니다.  
