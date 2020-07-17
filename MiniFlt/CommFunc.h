#ifndef __COMMFUNC_H__
#define __COMMFUNC_H__

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

#endif