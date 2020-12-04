# 커널에서 현재 프로세스 전체 경로를 획득하는 방법 by Unundocumented API  

보통 커널에서 현재 프로세스를 구하는 방법을 저는 3가지가 있는걸로 알고 있습니다. (더 있을 수도...)   
- 첫번째는 PEB를 찾아서 프로세스 전체 경로를 획득하는 방법  
- 두번째는 Unundocumented API 를 사용해서 획득하는 방법  
- 세번째는 나중에 커널에서 Export 한 API를 사용하는 방법  

여기서는 Unundocumented API를 이용해서 프로세스 전체 경로를 획득하는 방법에 대해서 기술해보겠습니다.  

먼저 이 API는 추후에 다른 정보를 획득하는데 있어서도 유용하게 써먹을 수 있습니다.  
그럼 먼저 Unundocumented API의 경우는 함수 포인터를 획득해야 합니다.  
그러기 위해서 함수 원형을 알아야 하는데 아래와 같습니다.  

```c
typedef NTSTATUS(*QUERY_INFORMATION_PROCESS) (
	_In_      HANDLE           ProcessHandle,
	_In_      PROCESSINFOCLASS ProcessInformationClass,
	_Out_     PVOID            ProcessInformation,
	_In_      ULONG            ProcessInformationLength,
	_Out_opt_ PULONG           ReturnLength
);
```

위 구조체는 NtQueryInformationProcess를 선언해준거구요.  
이제 선언한 구조체에 함수 포인터를 받아오면 됩니다.  

```c
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

그리고 이제 프로세스 경로를 획득 할 때마다 선언한 ZwQueryInformationProcess API를 사용하면 됩니다.  
그렇다면 프로세스 경로를 획득하기 위해서 호출해야 하는 API는 무엇들이 있는지 보도록 하겠습니다.  
1. 현재 프로세스의 EPROCESS 포인터를 획득해야 합니다.  
2. 그리고 난 다음에 획득한 EPROCESS 포인터의 핸들을 획득하고,  
3. 그 핸들을 가지고 프로세스에 대한 정보를 조회하면 됩니다.  

그럼 위 순서를 코드로 보면 아래와 같습니다.  

```c
NTSTATUS ZwGetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG BufLen = 0;
	HANDLE hProcess = NULL;
	PEPROCESS pProcess = NULL;
	PUNICODE_STRING ProcName;
	PVOID Buffer = NULL;

	PAGED_CODE();

	if (ZwQueryInformationProcess == NULL && InitZwQueryInformationProcess() == STATUS_UNSUCCESSFUL) return 0;

	pProcess = FltGetRequestorProcess(Data);
	if (pProcess) {
		Status = ObOpenObjectByPointer(pProcess, OBJ_KERNEL_HANDLE, NULL, GENERIC_READ, 0, KernelMode, &hProcess);
		if (!NT_SUCCESS(Status)) {
			DbgPrint("ObOpenObjectByPointer failed [0x%X]", Status);
			return Status;
		}

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &BufLen);
		if (Status != STATUS_INFO_LENGTH_MISMATCH) {
			DbgPrint("ZwQueryInformationProcess failed #1[0x%X]", Status);
			ZwClose(hProcess);
			return Status;
		}

		Buffer = MyAllocNonPagedPool(BufLen, &g_NonPagedPoolCnt);
		if (!Buffer) {
			ZwClose(hProcess);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, Buffer, BufLen, &BufLen);
		if (NT_SUCCESS(Status))
		{
			ProcName = (PUNICODE_STRING)Buffer;

			DbgPrint("FullPath[%S]", ProcName->Buffer);
		}
		else DbgPrint("ZwQueryInformationProcess failed #2[0x%X]", Status);

		MyFreeNonPagedPool(Buffer, &g_NonPagedPoolCnt);

		ZwClose(hProcess);
	}
	else DbgPrint("FltGetRequestorProcess failed");

	return Status;
}
```

먼저 FltGetRequestorProcess API를 이용해서 **EPROCESS의 포인터를 획득**하고 난 뒤에  
ObOpenObjectByPointer API를 호출해서 EPROCESS의 핸들을 획득합니다.  
여기서 ObOpenObjectByPointer API를 호출 할 때 반드시 2번째 파라미터  
**HandleAttributes 값은 OBJ_KERNEL_HANDLE** 를 설정해야 합니다.  
**(간혹 이 부분을 0으로 셋팅하고 커널에서 사용하시는분들이 계시더군요..)**  
이 후에 획득한 핸들을 가지고 ZwQueryInformationProcess API를 호출하면 됩니다.  
여기서 2번째 파라미터 **ProcessInformationClass 값은 ProcessImageFileName 로 설정**을 해주시면 됩니다.  
ProcessImageFileName 값은 프로세스 이미지 파일 이름을 획득할 때 설정하는 값입니다.  

추가적으로 ZwQueryInformationProcess API를 호출 할 때 버퍼를 충분히 크게 잡아도 되지만  
리턴 되는 값을 가지고 메모리를 할당해서 재 호출 하는 방법이 좀 더 좋다고 생각하기에  
저는 2번 호출하여 먼저 할당 할 메모리 사이즈를 리턴 받고 이 후 메모리를 할당하고  
다시 호출하여 프로세스 이미지 파일 경로가 저장된 UNICODE_STRING 포인터를 리턴 받았습니다.  
그리고 그 안에 있는 버퍼를 확인하여 프로세스 경로를 획득하였습니다.  
그리고 난 다음에 마지막에는 반드시 ObOpenObjectByPointer API 를 호출해서  
**획득한 핸들을 닫아주기 위해 **ZwClose API를 호출** 해줘야 합니다.  

이것으로 제가 알고 있는 총 3가지의 방법으로 커널에서 현재 실행중인 프로세스의 경로를   
획득하는 방법에 대해서 알아봤습니다.  