# 현재 동작중인 OS 버전을 확인하는 방법  

OS 버전에 따라서 접근해야 하는 방법이 조금씩 달라지다 보니  
드라이버가 처음 로드 될 때 현재 OS 버전을 획득해야 하는 경우가 있습니다.  
그런 경우에 해당 API를 호출하여 OS 버전을 획득 하는 방법에 대해서 작성해보겠습니다.  
현재 OS는 20H2 까지 나와있어서 Windows 2008부터 Windows 2012R2 그리고  
Windows 10의 경우는 세부 버전까지 한번에 정리를 해보도록 하겠습니다.  
먼저 호출하는 API는 간단하게 1개 이며, 이거는 드라이버가 처음 로드 될 때  
1번 호출을 하여 정보를 전역 변수로 가지고 있으면 편하다는 개인적인 생각입니다.  
그럼 사용해야 할 API와 Struct는 아래와 같습니다.  

```c
NTSYSAPI NTSTATUS RtlGetVersion(
  PRTL_OSVERSIONINFOW lpVersionInformation
);

typedef struct _OSVERSIONINFOEXW {
  ULONG  dwOSVersionInfoSize;
  ULONG  dwMajorVersion;
  ULONG  dwMinorVersion;
  ULONG  dwBuildNumber;
  ULONG  dwPlatformId;
  WCHAR  szCSDVersion[128];
  USHORT wServicePackMajor;
  USHORT wServicePackMinor;
  USHORT wSuiteMask;
  UCHAR  wProductType;
  UCHAR  wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;
```

OSVERSIONINFOEXW 변수를 선언 하고 RtlGetVersion API를 호출하면 끝이 납니다.  
하지만 세부적으로 확인하기 위해서는 OSVERSIONINFOEXW 내에 있는   
dwMajorVersion, dwMinorVersion, dwBuildNumber 3가지를 확인하면 됩니다.  
먼저 Windows 2008, Vista의 경우는 dwMajorVersion이 6이고, dwMinorVersion은 0 입니다.  
이런식으로 확인을 하면 되는데 구분하는 방법은 아래 코드와 같습니다.  

```c
WINDOWS_VERSION g_WinVersion;
VOID GetVersion()
{
	RTL_OSVERSIONINFOEXW VersionInfo;

	RtlGetVersion(&VersionInfo);

	if (VersionInfo.dwMajorVersion == 6) {
		if (VersionInfo.dwMinorVersion == 0) g_WinVersion = WV_2008;
		else if (VersionInfo.dwMinorVersion == 1) g_WinVersion = WV_2008R2;
		else if (VersionInfo.dwMinorVersion == 2) g_WinVersion = WV_2012;
		else if (VersionInfo.dwMinorVersion == 3) g_WinVersion = WV_2012R2;
	}
	else if (VersionInfo.dwMajorVersion == 10) {
		if (VersionInfo.dwMinorVersion == 0) {
			if (VersionInfo.dwBuildNumber >= WV_WIN10_20H2) g_WinVersion = WV_WIN10_20H2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_20H1) g_WinVersion = WV_WIN10_20H1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_19H2) g_WinVersion = WV_WIN10_19H2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_19H1) g_WinVersion = WV_WIN10_19H1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS5) g_WinVersion = WV_WIN10_RS5;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS4) g_WinVersion = WV_WIN10_RS4;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS3) g_WinVersion = WV_WIN10_RS3;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS2) g_WinVersion = WV_WIN10_RS2;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_RS1) g_WinVersion = WV_WIN10_RS1;
			else if (VersionInfo.dwBuildNumber >= WV_WIN10_TH2) g_WinVersion = WV_WIN10_TH2;
			else g_WinVersion = WV_WIN10;
		}
	}
	else g_WinVersion = WV_UNKNOWN;
}
```

위에서 작성한 코드처럼 세부적으로 구분을 하면 됩니다.  
그런데 여기서 Windows 10의 경우는 더이상 dwMinorVersion버전이 올라가지 않습니다.  
그래서 구분을 하는 방법이 dwBuildNumber로 구분을 해야 합니다.  
따라서 dwBuildNumber는 아래와 같습니다.  

```c
typedef enum {
	WV_UNKNOWN = 0,
	WV_2008 = 6002,        // 6.0 Windows Vista, Server 2008
	WV_2008R2 = 7601,      // 6.1 Windows 7, Server 2008 R2
	WV_2012 = 9200, 	     // 6.2 windows 8, Server 2012
	WV_2012R2 = 9600,      // 6.3 windows 8, Server 2012 R2
	WV_WIN10 = 10240,      // Threshold 1: Version 1507, Build 10240.
	WV_WIN10_TH2 = 10586,  // Threshold 2: Version 1511, Build 10586.
	WV_WIN10_RS1 = 14393,  // Redstone 1: Version 1607, Build 14393.
	WV_WIN10_RS2 = 15063,  // Redstone 2: Version 1703, Build 15063.
	WV_WIN10_RS3 = 16299,  // Redstone 3: Version 1709, Build 16299.
	WV_WIN10_RS4 = 17134,  // Redstone 4: Version 1803, Build 17134.
	WV_WIN10_RS5 = 17763,  // Redstone 5: Version 1809, Build 17763.
	WV_WIN10_19H1 = 18362, // 19H1: Version 1903, Build 18362.
	WV_WIN10_19H2 = 18363, // 19H2: Version 1909, Build 18363.
	WV_WIN10_20H1 = 19041, // 20H1: Version 2004, Build 19041.
	WV_WIN10_20H2 = 19042, // 20H2: Version 2009, Build 19042.
} WINDOWS_VERSION;
```

이렇게 WINDOWS_VERSION enum을 선언하고  
dwMajorVersion, dwMinorVersion, dwBuildNumber 3가지를  
확인하여 현재 OS의 버전을 확인하고 이 후 필요한 상황에 맞게   
OS 버전을 참고하여 활용을 하면 되겠습니다.  
