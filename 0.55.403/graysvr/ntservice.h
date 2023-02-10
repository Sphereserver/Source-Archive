//
// NTService.h
//

#ifdef _WIN32
#ifndef _INC_NTSERVICE_H
#define _INC_NTSERVICE_H

#include <eh.h> //	exception handling info.

extern class CNTService
{
private:
	bool m_fIsNTService;	// Are we running an NT service ?
	bool m_fDebugService;

	HANDLE m_hServerStopEvent;
	SERVICE_STATUS_HANDLE m_hStatusHandle;
	SERVICE_STATUS m_sStatus;

private:
	int  ServiceStart(DWORD, LPTSTR *);
	void ServiceStop();
	void ServiceStartMain(DWORD dwArgc, LPTSTR *lpszArgv);

	// Our exported API.
	static BOOL WINAPI ControlHandler(DWORD dwCtrlType);
	static void WINAPI service_ctrl(DWORD dwCtrlCode);
	static VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);

public:
	CNTService();

	void ReportEvent( WORD wType, DWORD dwMsgID, LPCTSTR lpszMsg, LPCTSTR lpszArgs = NULL );
	BOOL SetServiceStatus(DWORD, DWORD, DWORD);

	// command line entries.
	void CmdInstallService();
	void CmdRemoveService();
	int  CmdDebugService(int, TCHAR **);
	void CmdMainStart();

	bool OnTick();
	bool IsRunningAsService() const
	{
		return( m_fIsNTService );
	}

	// int InstallService(int argc, TCHAR * argv[]);

} g_Service;

#endif // _INC_NTSERVICE_H
#endif // _WIN32
