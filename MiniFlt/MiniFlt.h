#ifndef __MINIFLT_H__
#define __MINIFLT_H__

//  Enabled warnings

#pragma warning(error:4100)     //  Enable-Unreferenced formal parameter
#pragma warning(error:4101)     //  Enable-Unreferenced local variable
#pragma warning(error:4061)     //  Eenable-missing enumeration in switch statement
#pragma warning(error:4505)     //  Enable-identify dead functions

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

#define TAG_CONTEXT       'FMTC'

typedef enum {
	DVT_FIXED = 0,
	DVT_REMOVABLE,
	DVT_NETWORK,
	DVT_UNKNOWN
} DRIVE_TYPE, * PDRIVE_TYPE;

typedef struct _VOLUME_CONTEXT {
	UNICODE_STRING Name;
	ULONG SectorSize;

	FLT_FILESYSTEM_TYPE FSType;
	DRIVE_TYPE DrvType;
} VOLUME_CONTEXT, * PVOLUME_CONTEXT;


#endif __MINIFLT_H__

