#ifndef __MINIWPF_H__
#define __MINIWPF_H__

#include <ntifs.h>
#include <wdf.h>
#include <stdlib.h>
#include <ntstrsafe.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union

#include <fwpsk.h>

#pragma warning(pop)

#include <fwpmk.h>

#define MAX_KPATH 264

typedef struct _IPV4ADDR
{
	ULONG IPv4Addr[4];
	WCHAR RemoteIp[16];
}IPV4ADDR, * PIPV4ADDR;


#endif