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