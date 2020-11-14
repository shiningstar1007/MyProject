# 현재 프로세스의 부모 프로세스 ID 획득 방법  

현재 프로세스의 부모 프로세스 ID를 구하는 방법에 대해서 기술해보겠습니다.  
먼저 현재 프로세스의 ID를 가지고 부모 프로세스의 핸들을 획득해야 합니다.  
그리고 난 다음 부모 프로세스의 핸들을 가지고 부모 프로세스의 정보를 조회해서   
부모 프로세스의 ID를 획득하면 됩니다.  

여기서 사용해야 할 API는 다음 아래와 같습니다.  
```C   
NTSYSAPI NTSTATUS ZwOpenProcess(  
  PHANDLE            ProcessHandle,  
  ACCESS_MASK        DesiredAccess,  
  POBJECT_ATTRIBUTES ObjectAttributes,  
  PCLIENT_ID         ClientId  
);  

NTSTATUS WINAPI ZwQueryInformationProcess(  
  _In_      HANDLE           ProcessHandle,  
  _In_      PROCESSINFOCLASS ProcessInformationClass,  
  _Out_     PVOID            ProcessInformation,  
  _In_      ULONG            ProcessInformationLength,  
  _Out_opt_ PULONG           ReturnLength  
);  
```

위 2개의 API를 호출해서 현재 실행된 프로세스의 부모 프로세스ID를 손쉽게 획득 할 수 있습니다.    
여기서 ZwQueryInformationProcess API의 경우는 Undocumented API 입니다.  
그래서 따로 해당 API의 주소를 획득해서 사용을 해야 합니다.  
주소를 획득하는 방법은 다음 아래와 같습니다.  

```C  
QUERY_INFORMATION_PROCESS ZwQueryInformationProcess = NULL;  
NTSTATUS InitZwQueryInformationProcess()  
{  
	if (!ZwQueryInformationProcess) {  
		UNICODE_STRING RoutineName;  
		RtlInitUnicodeString(&RoutineName, L"ZwQueryInformationProcess");  

		ZwQueryInformationProcess = (QUERY_INFORMATION_PROCESS)MmGetSystemRoutineAddress(&RoutineName);  

		if (!ZwQueryInformationProcess) {  
			return STATUS_UNSUCCESSFUL;  
		}  
	}  

	return STATUS_SUCCESS;  
}  
```   
위 방법을 통해서 API의 주소를 획득한 뒤 사용하시면 됩니다.  

그렇다면 이제 위에서 설명한 순서대로 먼저 현재 프로세스의 ID를 가지고  
ZwOpenProcess API를 호출해서 부모 프로세스의 핸들을 획득합니다.  
처음 ZwOpenProcess API를 호출 할 때 Object의 속성을 초기화 해줘야 하는데   
여기서 조심해야 할 부분은 커널에서 오픈을 하기 때문에 Attributes 값을   
OBJ_KERNEL_HANDLE로 반드시 설정하고 ZwOpenProcess API 를 호출해야 합니다.  
 
그리고 난 다음에 ZwQueryInformationProcess API를 호출하는데   
PROCESSINFOCLASS는 ProcessBasicInformation 로 조회를 하면 됩니다.  
ProcessBasicInformation는 현재 조회하는 프로세스를 식별하기 위해 시스템에서 사용하는  
고유한 값을 검색, 즉 프로세스 ID 값을 조회 할 때 사용합니다.  

그래서 조회가 성공 하면 넘겨준 PROCESS_BASIC_INFORMATION 구조체에 정보가 담겨오는데  
그 중에서 InheritedFromUniqueProcessId 값을 확인하면 됩니다.  

```C  
ULONG ProcessGetParentPId(ULONG ProcessId)  
{  
	NTSTATUS Status;  
	HANDLE hProcess = NULL;  
	OBJECT_ATTRIBUTES ObjAttr = { 0 };  
	CLIENT_ID ClientId = { 0 };  
	PROCESS_BASIC_INFORMATION BasicInfo;  
	ULONG ParentPId = 0, BufSize = sizeof(PROCESS_BASIC_INFORMATION);  

	if (ProcessId == 0) return 0;  

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;  

	InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);  
	ClientId.UniqueProcess = UlongToHandle(ProcessId);  

	Status = ZwOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION, &ObjAttr, &ClientId);  
	if (Status != STATUS_SUCCESS || hProcess == NULL) return 0;  

	if (ZwQueryInformationProcess(hProcess, ProcessBasicInformation, &BasicInfo,  
		BufSize, &BufSize) == STATUS_SUCCESS) {  
		ParentPId = (ULONG)BasicInfo.InheritedFromUniqueProcessId;  
	}  

	ZwClose(hProcess);  
 
	return ParentPId;  
}  
```