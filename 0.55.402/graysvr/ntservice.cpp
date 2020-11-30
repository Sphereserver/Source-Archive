// ntservice.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Stuff for making this application run as an NT service
//
// pae 06/17/1999

#ifdef _WIN32
// Linux should be completely oblivious to this file

#include "graysvr.h"	// predef header.
#include "../common/grayver.h"	// sphere version

#include "ntservice.h"	// Stuff for the NT Service
#include <direct.h>
// #include <messages.h>  // Event Log Message definitions for NT services

// Globals
CNTService g_Service;

CNTService::CNTService()
{
	m_hStatusHandle = NULL;
	m_fDebugService = false;
	m_fIsNTService = false;
}

/////////////////////////////////////////////////////////////////////////////////////

static void ExtractPath( LPTSTR szPath )
{
	// Try to create the registry key containing the working directory for the application
	TCHAR * pszPath = strrchr( szPath, '\\' );
	if ( pszPath )
	{
		*pszPath = '\0';
	}
}

static LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
	// Check CGrayError.
	//	PURPOSE:  copies error message text to a string
	//
	//	PARAMETERS:
	//		lpszBuf - destination buffer
	//		dwSize - size of buffer
	//
	//	RETURN VALUE:
	//		destination buffer

	int nChars = CGrayError::GetSystemErrorMessage( GetLastError(), lpszBuf, dwSize );
	sprintf( lpszBuf+nChars, " (ox%x)", GetLastError());
	return lpszBuf;
}

/////////////////////////////////////////////////////////////////////////////////////

void CNTService::ReportEvent( WORD wType, DWORD dwEventID, LPCTSTR lpszMsg, LPCTSTR lpszArgs )
{
	//	PURPOSE:  Allows any thread to log a message to the NT Event Log
	//
	//	PARAMETERS:
	//		lpszMessage - Description of the event to store in the Event Log
	//		dwEventID - EventID to store in the Event Log
	//		wType - Event Type (error, informational, or warning) EVENTLOG_ERROR_TYPE
	//
	// Open the handle to the Event Log

	g_Log.Event( ( wType == EVENTLOG_INFORMATION_TYPE ) ?
		(LOGL_EVENT|LOGM_INIT) : (LOGL_ERROR|LOGM_INIT),
		_TEXT("%s %s\n"), lpszMsg, lpszArgs );

#if 0
	// Put message in the system event Queue ?
	CGString sMsg;
	sMsg.Format( GRAY_TITLE " %s", g_Serv.GetName() );
	HANDLE hEventSource = RegisterEventSource( NULL, sMsg );

	if (hEventSource != NULL)
	{
		// if we got the handle to the event log, go ahead and write the event
		if (lpszMsg == NULL)
			::ReportEvent(hEventSource, wType, 0, dwEventID, NULL, 0, 0, NULL, NULL);
		else
			::ReportEvent(hEventSource, wType, 0, dwEventID, NULL, 1, 0, (const char **) (&lpszMsg), NULL);
		// Need to close the handle to the event log.
		DeregisterEventSource(hEventSource);
	}
#endif

}

/////////////////////////////////////////////////////////////////////////////////////

bool CNTService::OnTick()
{
	// RETURN: false = exit app.
	if ( ! m_fIsNTService )
		return( true );
	if ( m_sStatus.dwCurrentState != SERVICE_STOP_PENDING )
		return( true );

	// Let the SCM know we aren't ignoring it
	SetServiceStatus( SERVICE_STOP_PENDING, NO_ERROR, 1000 );

	// The service manager wants us to exit.
	g_Serv.SetExitFlag( 4 );
	return( false );
}

/////////////////////////////////////////////////////////////////////////////////////

BOOL CNTService::SetServiceStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint )
{
	//	PURPOSE:  Sets the current status of the service and
	//		reports it to the Service Control Manager
	//
	//	PARAMETERS:
	//		dwCurrentState - the state of the service
	//		dwWin32ExitCode - error code to report
	//		dwWaitHint - worst case estimate to next checkpoint
	//
	//	RETURN VALUE:
	//		TRUE - success
	//		FALSE - failure
	// When debugging, we don't report to the SCM

	if ( m_fDebugService ) 
		return( TRUE );

	if (dwCurrentState == SERVICE_START_PENDING)
		m_sStatus.dwControlsAccepted = 0;
	else
		m_sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	m_sStatus.dwCurrentState = dwCurrentState;
	m_sStatus.dwWin32ExitCode = dwWin32ExitCode;
	m_sStatus.dwWaitHint = dwWaitHint;

	static DWORD dwCheckPoint = 1;
	if (( dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
		m_sStatus.dwCheckPoint = 0;
	else
		m_sStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the Service Control Manager
	BOOL fResult = ::SetServiceStatus( m_hStatusHandle, &m_sStatus );
	if ( !fResult && m_fIsNTService )
	{
		// if fResult is not 0, then an error occurred.  Throw this in the event log.
		char szErr[256];
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, // CSNTSERVER_SERVICE_SETSTATUS_FAILED, 
			"SetServiceStatus", GetLastErrorText(szErr, sizeof(szErr)));
	}

	return (fResult);
}

/////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI CNTService::ControlHandler(DWORD dwCtrlType)	// static
{
	//	PURPOSE:  Handles console control events.  This application only handles
	//		Ctrl-C and Ctrl-Break events.
	//
	//	PARAMETERS:
	//		dwCtrlType - type of control event
	//
	//	RETURN VALUE:
	//		TRUE - handled
	//		FALSE - not handled
	// Determine which Control sequence was entered

	switch(dwCtrlType)
	{
	case CTRL_BREAK_EVENT:		// Use Ctrl+Break or Ctrl+C to simulate
	case CTRL_C_EVENT:			// SERVICE_CONTROL_STOP in debug mode
		g_Service.ServiceStop();
		return (TRUE);
	}

	// Send the current status of the service
	g_Service.SetServiceStatus( g_Service.m_sStatus.dwCurrentState, NO_ERROR, 3000);
	return(FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////

void WINAPI CNTService::service_ctrl(DWORD dwCtrlCode) // static
{
	//
	//	PURPOSE:  This function is called by the SCM whenever
	//		ControlService() is called on this service.  The
	//		SCM does not start the service through this function.
	//
	//	PARAMETERS:
	//		dwCtrlCode - type of control requested
	//
	// Handle the requested control code.

	switch(dwCtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		// Stop the service
		g_Service.ServiceStop();
		break;
	case SERVICE_CONTROL_INTERROGATE:
		// Don't do anything with this control code.
		// We'll report back to the SCM the current status of the service.
		break;
	default:
		// Unhandled control code
		break;
	}
	g_Service.SetServiceStatus( g_Service.m_sStatus.dwCurrentState, NO_ERROR, 0 );
}

/////////////////////////////////////////////////////////////////////////////////////

VOID WINAPI CNTService::service_main(DWORD dwArgc, LPTSTR *lpszArgv) // static
{
	//	PURPOSE:  is called by the SCM, and takes care of some
	//						initialization and calls ServiceStart().
	//	PARAMETERS:
	//		dwArgc - count of arguments
	//		lpszArgv - pointer to an array of arguments
	//
	g_Service.ServiceStartMain( dwArgc, lpszArgv );
}

/////////////////////////////////////////////////////////////////////////////////////

void CNTService::ServiceStartMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	//	PURPOSE:  starts the service. (synchronous)
	ASSERT(m_fIsNTService);

	CGString sTitle;
	sTitle.Format( GRAY_TITLE " V" GRAY_VERSION " - %s", g_Serv.GetName());

	m_hStatusHandle = RegisterServiceCtrlHandler( sTitle, service_ctrl );
	if ( ! m_hStatusHandle )
	{
		// Not much we can do about this.
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "RegisterServiceCtrlHandler failed" );
		return;
	}

	// SERVICE_STATUS members that don't change
	m_sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_sStatus.dwServiceSpecificExitCode = 0;

	// report the status to the service control manager.
	if ( SetServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
		ServiceStart( dwArgc, lpszArgv);
	}

	// try to report the stopped status to the service control manager
	SetServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

/////////////////////////////////////////////////////////////////////////////////////

int CNTService::ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv)
{
	//	PURPOSE:  starts the service. (synchronous)
	//
	//	PARAMETERS:
	//		dwArgc - the number of parameters passed in
	//		lpszArgv - array of parameters
	//

	ASSERT(m_fIsNTService);

	ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, // CSNTSERVER_SERVICE_START_PENDING, 
		"Service start pending." );

	// Service initialization
	// report status to the service control manager.
	if ( ! SetServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000))
		return( -1 );

	int rc = -1;

	// Create the event object.  The control handler function
	// signals this event when it receives a "stop" control code
	m_hServerStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hServerStopEvent == NULL)
		return( -1 );

	if ( ! SetServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000 ))
	{
bailout1:
		CloseHandle(m_hServerStopEvent);
		return( rc );
	}

	// Create the event object used in overlapped i/o
	HANDLE hEvents[2] = {NULL, NULL};
	hEvents[0] = m_hServerStopEvent;
	hEvents[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvents[1] == NULL)
	{
		goto bailout1;
	}

	if ( ! SetServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
bailout2:
		CloseHandle(hEvents[1]);
		goto bailout1;
	}

	// Create a security descriptor that allows anyone to write to the pipe
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (pSD == NULL)
	{
		goto bailout2;
	}

	if ( ! InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
	{
bailout3:
		free(pSD);
		goto bailout2;
	}

	// Add a NULL descriptor ACL to the security descriptor
	if ( ! SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE))
	{
		goto bailout3;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = TRUE;

	if ( ! SetServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000))
	{
		goto bailout3;
	}

	// Set the name of the named pipe this application uses.  We create a named pipe to ensure that
	// only one instance of this application runs on the machine at a time.  If there is an instance
	// running, it will own this pipe, and any further attempts to create the same named pipe will fail.
	char lpszPipeName[80];
	sprintf(lpszPipeName, "\\\\.\\pipe\\" GRAY_FILE "svr\\%s", g_Serv.GetName());

	char szErr[256];

	// Open the named pipe
	HANDLE hPipe = CreateNamedPipe(lpszPipeName, FILE_FLAG_OVERLAPPED |
		PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, 0, 0, 1000, &sa);
	if ( hPipe == INVALID_HANDLE_VALUE )
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, // CSNTSERVER_SERVICE_CREATEPIPE_FAILED, 
			"Can't create named pipe. Check multi-instance?", GetLastErrorText(szErr, sizeof(szErr)));
		goto bailout3;
	}

#if 0
	// Read the WorkingPath
	// NOTE: ??? We do nothing with this.!!
	HKEY hKey;
	CGString sKey;
	sKey.Format( "System\\CurrentControlSet\\Services\\" GRAY_TITLE " - %s\\Parameters", g_Serv.GetName());
	char szGlobalPath[MAX_PATH];
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, sKey, 0, KEY_ALL_ACCESS, &hKey ))
	{
		strcpy(szGlobalPath, "c:\\" GRAY_FILE );
	}
	else
	{
		unsigned long lType;
		unsigned long lSize;
		RegQueryValueEx(hKey, "WorkingPath", 0, &lType, (unsigned char *) &szGlobalPath[0], &lSize);
	}
	if (strcmp(szGlobalPath, "") == 0)
		strcpy(szGlobalPath, "c:\\" GRAY_FILE );
#endif

	if (SetServiceStatus(SERVICE_RUNNING, NO_ERROR, 0))
	{
		rc = Sphere_MainEntryPoint(dwArgc, lpszArgv);
		// Now the application has stopped
		if (rc == 0)
		{
			ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, // CSNTSERVER_SERVICE_STOPPED,
				"service stopped", GetLastErrorText(szErr, sizeof(szErr)));
		}
		else
		{
			char szMessage[80];
			sprintf(szMessage, "%ld.", rc);
			ReportEvent( EVENTLOG_ERROR_TYPE, 0, // CSNTSERVER_SERVICE_STOPPED,
				"service stopped abnormally", szMessage );
		}
	}
	else
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, // CSNTSERVER_SERVICE_START_FAILED,
			"ServiceStart failed." );
	}

	// Cleanup
	CloseHandle(hPipe);
	goto bailout3;
}

/////////////////////////////////////////////////////////////////////////////////////

VOID CNTService::ServiceStop()
{
	//	PURPOSE:  reports that the service is attempting to stop
	//
	//	COMMENTS:
	//		The service stop is actually triggered by the fact that the service_ctrl()
	//		function set the m_sStatus.dwCurrentState == SERVICE_STOP_PENDING, since
	//		that value is checked by the csntServerMain() function.
	//		The m_hServerStopEvent that is set here is not referenced by csntServerMain().

	ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, // CSNTSERVER_SERVICE_STOP_PENDING,
		"Attempting to stop service" );

	m_sStatus.dwCurrentState = SERVICE_STOP_PENDING;

	SetServiceStatus(m_sStatus.dwCurrentState, NO_ERROR, 3000);

	if (m_hServerStopEvent)
	{
		SetEvent(m_hServerStopEvent);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
//	FUNCTION: CmdInstallService()
//
//	PURPOSE:  Installs the service on the local machine
//
//	PARAMETERS:
//		none
//
//	RETURN VALUE:
//		none
//
//	COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////////////
void CNTService::CmdInstallService()
{
	char szPath[_MAX_PATH * 2];
	char szErr[256];

	// Try to determine the name and path of this application.
	if (GetModuleFileName(NULL, szPath, sizeof(szPath)) == 0)
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
			"Install GetModuleFileName", GetLastErrorText(szErr, sizeof(szErr)));
		return;
	}

	// Try to open the Service Control Manager
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( ! schSCManager)
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
			"Install OpenSCManager", GetLastErrorText(szErr, sizeof(szErr)));
		return;
	}

	// Try to create the service
	char szInternalName[MAX_PATH];
	sprintf(szInternalName, GRAY_TITLE " - %s", g_Serv.GetName());

	SC_HANDLE schService = CreateService(
		schSCManager,					// handle of the Service Control Manager
		szInternalName,				// Internal name of the service (used when controlling the service using "net start" or "netsvc")
		szInternalName,			// Display name of the service (displayed in the Control Panel | Services page)
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,				// Start automatically when the OS starts
		SERVICE_ERROR_NORMAL,
		szPath,							// Path and filename of this executable
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);
	if ( ! schService )
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
			"Install CreateService", GetLastErrorText(szErr, sizeof(szErr)));
bailout1:
		CloseServiceHandle(schSCManager);
		return;
	}

	HKEY hKey;
	char szKey[MAX_PATH];

	// Register the application for event logging
	DWORD dwData;
	// Try to create the registry key containing information about this application
	strcpy(szKey, "System\\CurrentControlSet\\Services\\EventLog\\Application\\" GRAY_FILE "svr");

	if (RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey))
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
			"Install RegCreateKey", GetLastErrorText(szErr, sizeof(szErr)));
	}
	else
	{
		// Try to create the registry key containing the name of the EventMessageFile
		//  Replace the name of the exe with the name of the dll in the szPath variable
		if (RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ, (LPBYTE) szPath, strlen(szPath) + 1))
		{
			ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
				"Install RegSetValueEx", GetLastErrorText(szErr, sizeof(szErr)));
		}
		else
		{
			// Try to create the registry key containing the types of errors this application will generate
			dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE;
			if (RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD, (LPBYTE) &dwData, sizeof(DWORD)))
			{
				ReportEvent( EVENTLOG_ERROR_TYPE, 0,
					"Install RegSetValueEx", GetLastErrorText(szErr, sizeof(szErr)));
			}
		}
		// Close the Registry Key handle
		RegCloseKey(hKey);
	}

	// Set the working path for the application
	sprintf(szKey,  "System\\CurrentControlSet\\Services\\" GRAY_TITLE " - %s\\Parameters", g_Serv.GetName());
	if ( RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey))
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0,
			"Install RegCreateKey", GetLastErrorText(szErr, sizeof(szErr)));
bailout2:
		CloseServiceHandle(schService);
		goto bailout1;
	}

	ExtractPath( szPath );

	if (RegSetValueEx(hKey, "WorkingPath", 0, REG_SZ, (const unsigned char *) &szPath[0], strlen(szPath)))
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0,
			"Install RegSetValueEx", GetLastErrorText(szErr, sizeof(szErr)));
	}

	ReportEvent( EVENTLOG_INFORMATION_TYPE, 0,	
		"Install OK", g_Serv.GetName());

	goto bailout2;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//	FUNCTION: CmdRemoveService()
//
//	PURPOSE:  Stops and removes the service
//
//	PARAMETERS:
//		none
//
//	RETURN VALUE:
//		none
//
//	COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////////////
void CNTService::CmdRemoveService()
{
	char szErr[256];

	// Try to open the Service Control Manager
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( ! schSCManager)
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0,
			"Remove OpenSCManager failed", GetLastErrorText(szErr, sizeof(szErr)));
		return;
	}

	// Try to obtain the handle of this service
	char szInternalName[MAX_PATH];
	sprintf( szInternalName, GRAY_TITLE " - %s", g_Serv.GetName());

	SC_HANDLE schService = OpenService(schSCManager, szInternalName, SERVICE_ALL_ACCESS );
	if ( ! schService)
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, 
			"Remove OpenService failed", GetLastErrorText(szErr, sizeof(szErr)));

		// Close the SCM handle
// do_close_scm:
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check to see if the service is started, if so try to stop it.
	if (ControlService(schService, SERVICE_CONTROL_STOP, &m_sStatus))
	{
		ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, // CSNTSERVER_SERVICE_STOP_PENDING,
			"Stopping" );

		Sleep(1000);
		// wait for the service to stop.
		while (QueryServiceStatus(schService, &m_sStatus))
		{
			if (m_sStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				// printf(".");
				Sleep(1000);
			}
			else
				break;
		}

		if (m_sStatus.dwCurrentState == SERVICE_STOPPED)
		{
			ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
				"Stopped" );
		}
		else
		{
			ReportEvent( EVENTLOG_ERROR_TYPE, 0,
				"Failed to Stop" );
		}
	}

	// Remove the service
	if (DeleteService(schService))
	{
		ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
			"Removed OK" );
	}
	else
	{
		ReportEvent( EVENTLOG_ERROR_TYPE, 0,
			"Remove Fail", GetLastErrorText(szErr, sizeof(szErr)));
	}

	// Close the handle to this service
	CloseServiceHandle(schService);

	// Close the SCM handle
	CloseServiceHandle(schSCManager);
}

/////////////////////////////////////////////////////////////////////////////////////

int CNTService::CmdDebugService(int argc, char **argv)
{
	//	PURPOSE:  Runs the service as a console application
	//
	//	PARAMETERS:
	//		argc - The number of command line arguments
	//		argv - String array of command line arguments
	//
	//	RETURN VALUE:
	//		the return value from the real main function

	m_fDebugService = true;
	m_fIsNTService = true;

	DWORD dwArgc;
	LPTSTR *lpszArgv;

#ifdef UNICODE
	lpszArgv = CommandLineToArgvW(GetCommandLineW(), &(dwArgc));
#else
	dwArgc = (DWORD) argc;
	lpszArgv = argv;
#endif

	// Set the Console Control Handler so that the application will
	// respond to Ctrl-C and Ctrl-Break from the console window.
	SetConsoleCtrlHandler( ControlHandler, TRUE );

	return ServiceStart(dwArgc, lpszArgv);
}

/////////////////////////////////////////////////////////////////////////////////////

void CNTService::CmdMainStart()
{
	// probably synchronous.
	// printf("This may take serveral seconds... Please wait.\n");

	m_fIsNTService = true;

	char szTmp[256];
	sprintf( szTmp, GRAY_TITLE " - %s", g_Serv.GetName() );
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ szTmp, (LPSERVICE_MAIN_FUNCTION) service_main },
		{ NULL, NULL },
	};

	if (!StartServiceCtrlDispatcher(dispatchTable))
	{
		char szErr[256];
		ReportEvent( EVENTLOG_ERROR_TYPE, 0, // CSNTSERVER_SERVICE_START_FAILED,
			"StartServiceCtrlDispatcher", GetLastErrorText(szErr, sizeof(szErr)));
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
//	FUNCTION: main()
//
//	PURPOSE:  This is the main function in the application.
//		It parses the command line arguments and attempts to start the service.
//
//	PARAMETERS:
//		argc - Argument count
//		argv - Program arguments
//
//	RETURN VALUE:
//		none
//
//	COMMENTS:
//
/////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONSOLE
int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // command line
  int nCmdShow              // show state, SHOW_HIDE
)
{
	TCHAR * argv[32];
	argv[0] = NULL;
	int argc = Str_ParseCmds( lpCmdLine, &argv[1], COUNTOF(argv)-1, " \t" ) + 1;
#else
int _cdecl main( int argc, char * argv[] )
{
	int nCmdShow = SHOW_RESTORE;             // show state
#endif

	if ( GRAY_GetOSInfo()->dwPlatformId != VER_PLATFORM_WIN32_NT )
	{
		// We are running Win9x - So we are not an NT service.
do_not_nt_service:
		NTWindow_Init( hInstance, lpCmdLine, nCmdShow );
		int iRet = Sphere_MainEntryPoint( argc, argv );
		NTWindow_Exit();
		return( iRet );
	}

	// We need to find out what the server name is....look it up in the .ini file
	if ( ! g_Cfg.LoadIni( true ))
	{
		// Try to determine the name and path of this application.
		// We can't load the ini file if we don't know where it is.
		// When running as a service, we don't know our working path
		// g_Service.ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, "Getting module name." );

		char szPath[_MAX_PATH];
		GetModuleFileName( NULL, szPath, sizeof( szPath ) );
		
		if( szPath[0] == 0x00 )
		{
			// g_Service.ReportEvent( EVENTLOG_ERROR_TYPE, 0, "Couldn't find the application path!" );
			// g_Service.SetServiceStatus( SERVICE_STOPPED, -2, 0 );
			return( -2 );
		}

		ExtractPath( szPath );
		_chdir( szPath );

		if( !g_Cfg.LoadIni( false ) )
		{
			//g_Service.ReportEvent( EVENTLOG_ERROR_TYPE, 0, "Couldn't load ini file", szPath );
			//g_Service.SetServiceStatus( SERVICE_STOPPED, -2, 0 );
			return -2;
		}
	}

	// Am i being started as a service ??? No good way to tell !
	if ( !g_Cfg.m_fUseNTService )
	{
		// We just don't want to be a service.
		goto do_not_nt_service;
	}

	g_Service.SetServiceStatus( SERVICE_START_PENDING, NO_ERROR, 5000 );
	// We need to find out what directory the files are going to be in

	// process the command line arguments...

	if ((argc > 1) && _IS_SWITCH( *argv[1] ))
	{
		if ( ! strcmp((argv[1] + 1), "install"))
		{
			g_Service.ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
				"Installing Service." );
			g_Service.CmdInstallService();
			return 0;
		}
		else if ( ! strcmp((argv[1] + 1), "remove"))
		{
			g_Service.ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
				"Removing Service." );
			g_Service.CmdRemoveService();
			return 0;
		}
		else if ( ! strcmp((argv[1] + 1), "debug"))
		{
			g_Service.ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
				"Debugging Service." );
			goto do_not_nt_service;
			//return g_Service.CmdDebugService(argc, argv);
		}
	}

	// If the argument does not match any of the above parameters, the Service Control Manager (SCM) may
	// be attempting to start the service, so we must call StartServiceCtrlDispatcher.
	//
	// If the SCM isn't trying to start the service, then the service won't start.
	// Print a Usage type message just in case.
	// printf( GRAY_FILE "svr -install			to install the service\n");
	// printf( GRAY_FILE "svr -remove			to remove the service\n");
	// printf( GRAY_FILE "svr -debug <params>	to run as a console application for debugging\n");

	g_Service.ReportEvent( EVENTLOG_INFORMATION_TYPE, 0, 
		"Starting Service." );

	g_Service.CmdMainStart();
	g_Service.SetServiceStatus(SERVICE_STOPPED, -1, 0);
	return -1;
}

#endif

