#include "stdafx.h"

#include <afx.h>
#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


class CService
{
public:
	CService(VOID);
	virtual ~CService(VOID);

private:
	SERVICE_STATUS mServiceStatus;
	SERVICE_STATUS_HANDLE	mServiceStatusHandle;

	TCHAR mServiceName[MAX_PATH];

protected:
	virtual VOID			OnStarted(VOID) = 0;
	virtual VOID			OnStopped(VOID) = 0;

public:
	BOOL Install(LPCTSTR serviceName);
	BOOL Uninstall(LPCTSTR serviceName);

	BOOL Begin(LPTSTR serviceName);
	BOOL End(VOID);

	VOID RunCallback(DWORD argumentCount, LPTSTR* arguments);
	VOID CtrlHandlerCallback(DWORD opCode);
};