#include "stdafx.h"
#include "MiniAgent.h"

BOOL CService::Install(LPCTSTR serviceName)
{
	TCHAR		ServiceFileName[MAX_PATH] = { 0, };
	SC_HANDLE	ServiceControlManager = NULL;
	SC_HANDLE	ServiceHandle = NULL;

	if (!serviceName)
		return FALSE;

	ServiceFileName[0] = '"';
	GetModuleFileName(NULL, ServiceFileName + 1, MAX_PATH - 1);
	_tcscat(ServiceFileName, _T("\""));

	ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!ServiceControlManager)
		return FALSE;

	ServiceHandle = CreateService(ServiceControlManager,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		ServiceFileName,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (!ServiceHandle)
	{
		CloseServiceHandle(ServiceControlManager);
		return FALSE;
	}

	CloseServiceHandle(ServiceHandle);
	CloseServiceHandle(ServiceControlManager);

	return TRUE;
}