#ifndef __COMMFUNC_H__
#define __COMMFUNC_H__

#include <ntifs.h>
#include <ws2def.h>
#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

#include "MiniFlt.h"

#define TAG_MINIFLT 'SFNM'

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

extern WINDOWS_VERSION g_WinVersion;

#define PROCESS_QUERY_INFORMATION 0x00000400

typedef NTSTATUS(*QUERY_INFORMATION_PROCESS) (
	_In_      HANDLE           ProcessHandle,
	_In_      PROCESSINFOCLASS ProcessInformationClass,
	_Out_     PVOID            ProcessInformation,
	_In_      ULONG            ProcessInformationLength,
	_Out_opt_ PULONG           ReturnLength
);

#define NT_PROCNAMELEN 16
#define SYSNAMEW L"System"
#define SYSNAME  "System"

#define PROC_OFFSET_WS2008					0x14C
#define PROC_OFFSET_WIN7						0x16C
#define PROC_OFFSET_WS2008_x64			0x238 //Same in Vista x64
#define PROC_OFFSET_WS2008_x64_R2		0x2E0 //Same in Windows 7 x64
#define PROC_OFFSET_WS2012_x86			0x170 //Same in Windows 8, 8.1, 2012R2, 10 x86
#define PROC_OFFSET_WS2012_x64			0x438 //Same in Windows 8, 8.1, 2012R2 (all x64)
#define PROC_OFFSET_WS2016_x86			0x174 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2016_x64			0x448 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2019_x86			0x17C //same in windows 10 (1607 later)
#define PROC_OFFSET_WS2019_x64			0x450 //same in windows 10 (1607 later)
#define PROC_OFFSET_WS2019_x64_2004 0x5A8 //same in windows 10 (2004 later)

#define PEB_OFFSET_WS2008        0x188
#define PEB_OFFSET_WIN7          0x1A8
#define PEB_OFFSET_WS2008_x64    0x290 //Same in Vista x64
#define PEB_OFFSET_WS2008_x64_R2 0x338 //Same in Windows 7 x64
#define PEB_OFFSET_WS2012_x86    0x140 //Same in Windows 8, 8.1, 2012R2
#define PEB_OFFSET_WS2012_x64    0x3e8 //Same in Windows 8, 8.1, 2012R2 (all x64)
#define PEB_OFFSET_WS2016_x86    0x144 //same in windows 10 (10 to 1607)
#define PEB_OFFSET_WS2016_x64    0x3F8 //same in windows 10 (all x64)
#define PEB_OFFSET_WS2019_x86    0x14C //same in windows 10 (1607 later)
#define PEB_OFFSET_WS2019_x64    0x550 //same in windows 10 (2004 later)

#ifdef _WIN64
#define PARAMETERS_OFFSET    0x020
#define IMAGEPATH_OFFSET     0x060
#define COMMANDLINE_OFFSET   0x070
#define STATION_OFFSET			 0x210
#define DESKTOP_OFFSET       0x0C0
#else
#define PARAMETERS_OFFSET    0x010
#define IMAGEPATH_OFFSET     0x038
#define COMMANDLINE_OFFSET   0x040
#define STATION_OFFSET       0x1D4
#define DESKTOP_OFFSET       0x078
#endif

typedef unsigned int UINT;
typedef char CHAR, * PCHAR;
typedef unsigned char UCHAR, * PUCHAR;
typedef unsigned short USHORT;
typedef short SHORT, WORD;
typedef unsigned long ULONG, * PULONG, DWORD;
typedef unsigned char BYTE, * PBYTE;
typedef long BOOL, * PBOOL;

#define TICKSPERMIN       600000000
#define TICKSPERSEC       10000000
#define TICKSPERMSEC      10000
#define SECSPERDAY        86400
#define SECSPERHOUR       3600
#define SECSPERMIN        60
#define MINSPERHOUR       60
#define HOURSPERDAY       24
#define EPOCHWEEKDAY      1
#define DAYSPERWEEK       7
#define EPOCHYEAR         1601
#define DAYSPERNORMALYEAR 365
#define DAYSPERLEAPYEAR   366
#define MONSPERYEAR       12

typedef struct _SYSTEMTIME {
	USHORT wYear;
	USHORT wMonth;
	USHORT wDayOfWeek;
	USHORT wDay;
	USHORT wHour;
	USHORT wMinute;
	USHORT wSecond;
	USHORT wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;

#define ACTION_READ         0x00000001
#define ACTION_WRITE        0x00000002
#define ACTION_TRAVERSE     0x00000004
#define ACTION_DELETE       0x00000008
#define ACTION_CREATE       0x00000010
#define ACTION_EXECUTE      0x00000020
#define ACTION_KEY_CREATE   0x00000100
#define ACTION_KEY_DELETE   0x00000200
#define ACTION_VALUE_WRITE  0x00000400
#define ACTION_VALUE_DELETE 0x00000800
#define ACTION_ALL          0xFFFFFFFF

typedef struct _USERSID {
	BYTE                    Revision;
	BYTE                    SubAuthorityCount;
	SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
	DWORD                    SubAuthority[5];
} USERSID, * PUSERSID;

typedef enum _MY_COMMAND {
	SET_COMMAND,
	GET_COMMAND
} MY_COMMAND;

typedef struct _COMMAND_DATA {
	MY_COMMAND Command;
	ULONG Reserved;  // Alignment on IA64
	PUCHAR Data;
} COMMAND_DATA, * PCOMMAND_DATA;

extern LONG g_NonPagedPoolCnt;
PVOID MyAllocNonPagedPool(
	_In_ ULONG BufSize,
	_Inout_ PLONG MemCnt
);

PVOID MyFreeNonPagedPool(
	_Inout_ PVOID Buffer,
	_Inout_ PLONG MemCnt
);

ULONG MyStrNCopy(
	_Out_ PCHAR DestBuf,
	_In_ CONST PCHAR SourceBuf,
	_In_ ULONG MaxLen
);

VOID KrnlTimeToSysTime(
	_In_ LONGLONG KrnlTime,
	_Inout_ PSYSTEMTIME SysTime
);

LARGE_INTEGER GetKernelTime(
	_Inout_ PSYSTEMTIME SysTime
);

ULONG MyStrNCopyW(
	_Out_ PWCHAR DestBuf,
	_In_ CONST PWCHAR SourceBuf,
	_In_ ULONG SourceLen,
	_In_ ULONG MaxLen
);

ULONG MySNPrintf(
	_Out_opt_ PCHAR DestBuf,
	_In_ ULONG MaxLen,
	_In_opt_ CONST PCHAR FormatStr, ...
);

ULONG MySNPrintfW(
	_Out_opt_ PWCHAR DestBuf,
	_In_ ULONG MaxLen,
	_In_opt_ CONST PWCHAR FormatStr, ...
);

extern ULONG g_ProcNameOffset, g_PebOffset;
VOID GetVersion();
VOID InitializeProcess();

VOID GetProcName(
	_In_opt_ PEPROCESS pProcess,
	_Out_opt_ PCHAR ProcName
);

ULONG GetProcessFullPath(
	_In_opt_ PEPROCESS pProcess,
	_Out_opt_ PWCHAR ProcPathW
);

NTSTATUS ZwGetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
);

NTSTATUS GetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
);

NTSTATUS GetUserName(
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
);

NTSTATUS GetGroupName(
	_In_ PFLT_CALLBACK_DATA Data,
	_Inout_ PMINIFLT_INFO MiniFltInfo
);

PVOID MyQueryInformationToken(
	_In_opt_ PACCESS_TOKEN AccessToken,
	_In_ TOKEN_INFORMATION_CLASS TokenClass,
	_In_ ULONG MinSize
);

VOID GetUserSId(
	_Out_opt_ PUSERSID UserSId
);

ULONG GetGroupSId(
	_Out_ PUSERSID GroupSId
);

BOOL CheckLocalUser();

ULONG GetAction(
	_In_ ACCESS_MASK AccessMask
);

NTSTATUS MiniFltConnect(
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Flt_ConnectionCookie_Outptr_ PVOID* ConnectionCookie
);

VOID MiniFltDisconnect(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS MiniFltMessage(
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InBufSize) PVOID InBuf,
	_In_ ULONG InBufSize,
	_Out_writes_bytes_to_opt_(OutBufSize, *RetLen) PVOID OutBuf,
	_In_ ULONG OutBufSize,
	_Out_ PULONG RetLen
);

LONG MiniFltExceptionFilter(
	_In_ PEXCEPTION_POINTERS ExceptionPointer,
	_In_ BOOL AccessingUserBuffer
);

ULONG MyWideCharToChar(
	_In_opt_ PWCHAR pWBuf,
	_Out_opt_ PCHAR pBuf,
	_In_ ULONG MaxLen
);

ULONG MyCharToWideChar(
	_In_opt_ PCHAR pBuf,
	_Out_opt_ PWCHAR pWBuf,
	_In_ ULONG MaxLen
);

BOOL CheckRecycle(
	_In_opt_ PWCHAR ObjPathW,
	_In_opt_ PCHAR ObjPath
);


#endif