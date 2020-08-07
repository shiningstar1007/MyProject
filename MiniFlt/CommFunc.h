#ifndef __COMMFUNC_H__
#define __COMMFUNC_H__

#include <ntifs.h>

#define TAG_MINIFLT 'SFNM'

#define MAX_KPATH 264 
#define MAX_NAME 64
#define MAX_USER_NAME 48
#define MAX_DATA_BUF 4096
#define BUF_UNIT 32768

typedef unsigned int UINT;
typedef char CHAR, * PCHAR;
typedef unsigned char UCHAR, * PUCHAR;
typedef unsigned short USHORT;
typedef short SHORT, WORD;
typedef unsigned long ULONG, * PULONG, DWORD;
typedef unsigned char BYTE, * PBYTE;
typedef long BOOL, * PBOOL;

typedef enum {
	WV_UNKNOWN = 0,
	WV_2008 = 6002,   // 6.0 Windows Vista, Server 2008
	WV_2008R2 = 7601, // 6.1 Windows 7, Server 2008 R2
	WV_2012 = 9200, 	// 6.2 windows 8, Server 2012
	WV_2012R2 = 9600, // 6.3 windows 8, Server 2012 R2
	WV_2016 = 14393   // 10.0 windows 10, Server 2016
} WINDOWS_VERSION;

extern WINDOWS_VERSION g_WinVersion;

typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
);

#define NT_PROCNAMELEN 16
#define SYSNAMEW L"System"
#define SYSNAME  "System"

#define PROC_OFFSET_WS2008        0x14C
#define PROC_OFFSET_WIN7          0x16C
#define PROC_OFFSET_WS2008_x64    0x238 //Same in Vista x64
#define PROC_OFFSET_WS2008_x64_R2 0x2E0 //Same in Windows 7 x64
#define PROC_OFFSET_WS2012_x86    0x170 //Same in Windows 8, 8.1, 2012R2, 10 x86
#define PROC_OFFSET_WS2012_x64    0x438 //Same in Windows 8, 8.1, 2012R2 (all x64)
#define PROC_OFFSET_WS2016_x86    0x174 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2016_x64    0x448 //same in windows 10 (1511 to 1607)
#define PROC_OFFSET_WS2019_x86    0x17C //same in windows 10 (1607 later)
#define PROC_OFFSET_WS2019_x64    0x450 //same in windows 10 (1607 later)

#define PEB_OFFSET_WS2008        0x188
#define PEB_OFFSET_WIN7          0x1A8
#define PEB_OFFSET_WS2008_x64    0x290 //Same in Vista x64
#define PEB_OFFSET_WS2008_x64_R2 0x338 //Same in Windows 7 x64
#define PEB_OFFSET_WS2012_x86    0x140 //Same in Windows 8, 8.1, 2012R2
#define PEB_OFFSET_WS2012_x64    0x3e8 //Same in Windows 8, 8.1, 2012R2 (all x64)
#define PEB_OFFSET_WS2016_x86    0x144 //same in windows 10 (10 to 1607)
#define PEB_OFFSET_WS2016_x64    0x3F8 //same in windows 10 (all x64)
#define PEB_OFFSET_WS2019_x86    0x14C //same in windows 10 (1607 later)

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
NTSTATUS ZwGetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
);

NTSTATUS GetProcessImageName(
	_In_ PFLT_CALLBACK_DATA Data
);

VOID GetUserName(
	_In_ PFLT_CALLBACK_DATA Data
);

VOID GetGroupName(
	_In_ PFLT_CALLBACK_DATA Data
);

PVOID MyQueryInformationToken(
	_In_opt_ PACCESS_TOKEN AccessToken,
	_In_ TOKEN_INFORMATION_CLASS TokenInfoClass,
	_In_ ULONG MinSize
);

BOOL CheckLocalUser();



#endif