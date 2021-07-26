#ifndef __MINICOMM_H__
#define __MINICOMM_H__

#include <Windows.h>
#include <fltUser.h>
#include <strsafe.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <lm.h>
#include <psapi.h>
#include <conio.h>
#include <sys/stat.h>
#include <WtsApi32.h>
#include <Aclapi.h>
#include <Dsgetdc.h>

typedef struct _MINI_THREAD {
	HANDLE Handle;
	BOOL Terminated;
	DWORD ThreadId;
	DWORD(WINAPI* ThreadProc)(PVOID);
	PVOID Param;
} MINI_THREAD, * PMINI_THREAD;

typedef struct _FILE_MAP {
	HANDLE hMutex;
	HANDLE hFileMap;
	HANDLE hWaitEvent;
	CHAR MutexName[32];
	CHAR ReadEventName[32];
	CHAR WriteEventName[32];
	CHAR WaitEventName[32];
	CHAR FileMapName[32];

	MINI_THREAD FileMapThread;
} FILE_MAP, * PFILE_MAP;

#endif