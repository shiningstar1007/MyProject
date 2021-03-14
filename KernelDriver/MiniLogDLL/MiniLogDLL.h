#include <Windows.h>
#include <fltUser.h>

#define MINI_PORT_NAME     L"\\MiniFltPort"

#define MAX_REG_VALUE_LEN	0x100000

typedef enum _MINI_COMMAND {

} MINI_COMMAND;

typedef struct _COMMAND_MESSAGE {
	MINI_COMMAND Command;
	ULONG Reserved;  // Alignment on IA64
	UCHAR Data[];
} COMMAND_MESSAGE, * PCOMMAND_MESSAGE;

typedef struct _LOG_DATA {
	LARGE_INTEGER Time;
	CHAR DateTime[28];
	CHAR UserName[MAX_NAME];
	CHAR GroupName[MAX_NAME];
	CHAR FileName[MAX_KPATH];
	CHAR ProcName[MAX_KPATH];

	ULONG Action;

	WCHAR UserNameW[MAX_NAME];
	WCHAR GroupNameW[MAX_NAME];
	WCHAR FileNameW[MAX_KPATH];
	WCHAR ProcNameW[MAX_KPATH];
} LOG_DATA, * PLOG_DATA;

typedef struct _LOG_RECORD {
	ULONG Length;             // Length of log record.  This Does not include
	ULONG SequenceNumber;     // space used by other members of RECORD_LIST

	ULONG RecordType;         // The type of log record this is.
	ULONG Category;           // For alignment on IA64

	LOG_DATA Data;
	WCHAR FileNameW[MAX_KPATH]; //  This is a null terminated string

} LOG_RECORD, * PLOG_RECORD;