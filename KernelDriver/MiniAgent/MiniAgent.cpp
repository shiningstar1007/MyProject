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

BOOL CService::Uninstall(LPCTSTR serviceName)
{
	SC_HANDLE ServiceControlManager = NULL;
	SC_HANDLE ServiceHandle = NULL;

	if (!serviceName)
		return FALSE;

	ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!ServiceControlManager)
		return FALSE;

	ServiceHandle = OpenService(ServiceControlManager, serviceName, DELETE);
	if (!ServiceHandle)
	{
		CloseServiceHandle(ServiceControlManager);
		return FALSE;
	}

	if (!DeleteService(ServiceHandle))
	{
		CloseServiceHandle(ServiceHandle);
		CloseServiceHandle(ServiceControlManager);

		return FALSE;
	}

	CloseServiceHandle(ServiceHandle);
	CloseServiceHandle(ServiceControlManager);

	return TRUE;
}

BOOL CService::Begin(LPTSTR serviceName)
{
	if (!serviceName)
		return FALSE;

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{serviceName,	::RunCallback},
		{NULL,			NULL}
	};

	_tcscpy(mServiceName, serviceName);

	OnStarted();

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		_tprintf(_T("## Debug console mode ##\n"));
		getchar();
	}

	return TRUE;
}

BOOL CService::End(VOID)
{
	OnStopped();

	return TRUE;
}

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("심각한 오류: MFC를 초기화하지 못했습니다.\n"));
			nRetCode = 1;
		}
		else
		{
			CUpdateService upServ;
			if (argc == 2) {
				if (_tcscmp(argv[1], _T("-i")) == 0) {
					upServ.Install(_T("MiniAgent"));
				}
				else if (_tcscmp(argv[1], _T("-u")) == 0) {
					upServ.Uninstall(_T("MiniAgent"));
				}
				return true;
			}
			upServ.main();
		}
	}
	else
	{
		_tprintf(_T("심각한 오류: GetModuleHandle 실패\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
