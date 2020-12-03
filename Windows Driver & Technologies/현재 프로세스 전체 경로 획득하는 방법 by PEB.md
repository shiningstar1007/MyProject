# 커널에서 현재 프로세스 전체 경로를 획득하는 방법 by PEB  

보통 커널에서 현재 프로세스를 구하는 방법을 저는 3가지가 있는걸로 알고 있습니다. (더 있을 수도...)   
- 첫번째는 PEB를 찾아서 프로세스 전체 경로를 획득하는 방법  
- 두번째는 Unundocumented API 를 사용해서 획득하는 방법  
- 세번째는 나중에 커널에서 Export 한 API를 사용하는 방법  

여기서는 PEB를 찾아서 프로세스 전체 경로를 획득하는 방법에 대해서 기술해보겠습니다.  

이 PEB를 찾는거에 앞서 먼저 큰 단점이 하나 있는데 바로  
OS버전마다 EPROCESS 구조체가 바뀔 수 있다는 점입니다.  
이 부분은 정말 사람을 귀찮게 만들지만 간혹 PEB를 통해서 프로세스 경로를  
획득해야 하는 경우가 있기 때문에 기술을 해보도록 하겠습니다.  

이거는 완전 노가다입니다. 각각의 OffSet을 계산해서 직접 메모리에 접근하면 됩니다.  
먼저 프로세스 OffSet을 구해야 하는데 처음 드라이버가 로드 될 때 OffSet을 구해야 합니다.  
해당 OS의 현재 EPROCESS 구조체 사이즈를 확인 하기 위해서 구분하는 방법 ImageFileName로 찾으면 됩니다.  
최초 드라이버가 로드 될 때에는 현재 프로세스가 system 이기 때문에 당연히  
ImageFileName에는 "system" 이 저장되어 있을거니 해당 부분을 찾아서 ProcNameOffSet을 구합니다.  
추가적으로 ImageFileName의 OffSet은 OS별로 미리 계산해서 define 해둬야 하며,  
OS별로 미리 계산해 둔 ImageFileName OffSet과 매칭되는 PEB OffSet도 미리 계산해서 define 해야 합니다.﻿  
그리고 찾은 ImageFileName의 OffSet 값을 가지고 해당 OffSet의 PEB OffSet을 획득 하면 됩니다.  

```c
#define PROC_OFFSET_WS2016_x86      0x174 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2016_x64      0x448 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2019_x86      0x17C //same in windows 10 (1607 later)
#define PROC_OFFSET_WS2019_x64      0x450 //same in windows 10 (1607 later)
#define PROC_OFFSET_WS2019_x64_2004 0x5A8 //same in windows 10 (2004 later)

#define PEB_OFFSET_WS2016_x86    0x144 //same in windows 10 (10 to 1607)
#define PEB_OFFSET_WS2016_x64    0x3F8 //same in windows 10 (all x64)
#define PEB_OFFSET_WS2019_x86    0x14C //same in windows 10 (1607 later)
#define PEB_OFFSET_WS2019_x64    0x550 //same in windows 10 (2004 later)
```
```c
PEPROCESS pProcess = PsGetCurrentProcess(); 
for (g_ProcNameOffset = 0; g_ProcNameOffset < PAGE_SIZE; g_ProcNameOffset++)
  if (!_strnicmp("system", (PCHAR)pProcess + g_ProcNameOffset, strlen("system"))) break;
  
  switch (g_ProcNameOffset) {
		case PROC_OFFSET_WS2016_x86:
			g_PebOffset = PEB_OFFSET_WS2016_x86;
			break;  
		case PROC_OFFSET_WS2016_x64:
			g_PebOffset = PEB_OFFSET_WS2016_x64;
			break;  
		case PROC_OFFSET_WS2019_x86:  
			g_PebOffset = PEB_OFFSET_WS2019_x86;  
			break;  
		case PROC_OFFSET_WS2019_x64:  
			g_PebOffset = PEB_OFFSET_WS2016_x64;  
			break;  
		case PROC_OFFSET_WS2019_x64_2004:  
			g_PebOffset = PEB_OFFSET_WS2019_x64;  
			break;  

		default:  
			DbgPrint("Unknown Process Offset");  
	}  
```  

이렇게 먼저 PEB OffSet 값을 획득하고 이 후에 프로세스 경로를 획득 할 때마다  
이 PEB OffSet을 가지고 EPROCESS에 접근하여 PEB에 접근하면 됩니다.  
그런데.. 여기서 또 끝이 아닙니다. PEB 구조체 내부에는 PRTL_USER_PROCESS_PARAMATERS 구조체에 접근해야 합니다.  
그나마 다행인것은 아직까지 PEB 구조체가 변경된 적은 없었습니다.  

그래서 PRTL_USER_PROCESS_PARAMATERS 구조체에 접근 하면 됩니다.  
그런데.. 여기서 이제 마지막 ImagePathName 에 접근하여  
현재 UserLevel에서 실행 된 프로세스의 경로를 획득하면 됩니다.  

```c  
ULONG_PTR ProcAddress = (ULONG_PTR)pProcess;  
PUNICODE_STRING pUniStr;  

ProcAddress += EPROCESS_PEB_OFFSET;  
ProcAddress += PRTL_USER_PROCESS_PARAMATERS_OFFSET;  
if ((ProcAddress = *(ULONG_PTR*)ProcAddress) != 0) {  
  pUniStr = (PUNICODE_STRING)(ProcAddress + IMAGE_PATH_NAME_OFFSET);  
}  
```  

따라서 정리하자면..  
1. 각 OS 버전에 따른 EPROCESS의 ImageFileName OffSet과 PEB OffSet을 정의한다.  
2. 드라이버 로드 시 현재 OS의 PEB OffSet을 구한다.  
3. 획득 하려는 프로세스의 EPROCESS에서 앞서 구한 PEB OffSet을 계산해서 접근 한다.  
4. PRTL_USER_PROCESS_PARAMATERS OffSet을 계산해서 접근한다.  
5. ImagePathName에 접근하여 현재 실행 된 프로세스의 경로를 획득한다.  