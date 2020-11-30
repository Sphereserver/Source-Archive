//
// CBackTask.cpp
// Copyright Menace Software (www.menasoft.com).
// Background task stuff.
//

#include "graysvr.h"	// predef header.

///////////////////////////////////////////////////////////////////
// -CBackTask

bool CBackTask::RegisterServer()
{
	// NOTE: This is a synchronous function that could take a while.
	// Register myself with the central login server.
	bool fFirst = ( ! m_timeNextRegister.IsTimeValid() || ! Calc_GetRandVal( 20 ));
	m_timeNextRegister = CServTime::GetCurrentTime() + 60*60*TICK_PER_SEC;

	// don't register to myself.
	if ( ! g_Cfg.m_sMainLogServerDir.IsEmpty() )
	{
		m_sRegisterResult = _TEXT( "This is the list server" );
		return( false );
	}

	// Find the address for the main login server and register.
	LPCTSTR pszName = g_Cfg.m_sRegisterServer;
	if ( pszName[0] == '\0' )
	{
		m_sRegisterResult = _TEXT( "No list server configured" );
		return false;
	}

	if ( fFirst || ! m_RegisterIP.IsValidAddr())
	{
		// Only do the host lookup one time.
		bool fRet = m_RegisterIP.SetHostPortStr( pszName );	// GRAY_MAIN_SERVER
		if ( ! fRet )	// can't resolve the address.
		{
			// Might just try the address we already have ?
			m_sRegisterResult.Format( _TEXT( "Can't resolve list server DNS entry. Code=%d"), CGSocket::GetLastError() );
			m_RegisterIP.SetAddrIP( 0 );
			return false;
		}
	}

	// Tell the registration server that we exist.

	CGSocket sock;
	if ( ! sock.Create())
	{
		m_sRegisterResult = _TEXT( "Failed Create Socket" );
		return( false );
	}
	if ( sock.Connect( m_RegisterIP ))
	{
		m_sRegisterResult.Format( _TEXT( "Failed Connect Code %d"), sock.GetLastError());
		m_RegisterIP.SetAddrIP( 0 );	// try to look it up again later.
		return false;
	}

	// ? need to wait a certain amount of time for the connection to setle out here.

	// got a connection. Send reg command.
	TCHAR szData[ SCRIPT_MAX_LINE_LEN ];
	memset( szData, 0xFF, 4 );	// special registration header.
	szData[4] = 0;
	int iLen = strcpylen( &szData[5], g_Serv.GetStatusStringRegister(fFirst), sizeof( szData )-5);
	iLen += 5;
	int iLenRet = sock.Send( szData, iLen );
	if ( iLenRet != iLen )
	{
		// WSAENOTSOCK
		m_sRegisterResult.Format( _TEXT( "Failed Send Code %d"), sock.GetLastError());
		DEBUG_ERR(( "%d, can't send to reg server '%s'" DEBUG_CR, sock.GetLastError(), pszName ));
		return false;
	}

	m_sRegisterResult.Format( _TEXT( "OK" ));

#if 0
	// Get something back from the list server ?
	// get back the REGRES_TYPE code.
#endif

	// reset the max clients.
	g_Serv.ClientsResetAvg();
	return( true );
}

void CBackTask::PollServers()
{
	// Poll all the servers in my list to see if they are alive.
	// knock them off the list if they have not been alive for a while.

	m_timeNextPoll = CServTime::GetCurrentTime() + g_Cfg.m_iPollServers;

	int iTotalAccounts = 0;
	int iTotalClients = 0;
	for ( int i=0; ! g_Serv.m_iExitFlag; i++ )
	{
		CServerRef pServ = g_Cfg.Server_GetDef(i);
		if ( pServ == NULL )
			break;

		if ( g_Cfg.m_sMainLogServerDir.IsEmpty())	// the main server does not do this because too many people use dial ups .
		{
			if ( pServ->PollStatus())
				continue;
		}
		if ( ! pServ->IsValidStatus())
		{
			g_Log.Event( LOGL_EVENT|LOGM_ACCOUNTS, "Dropping Server '%s', ip=%s, age=%d, from the list." DEBUG_CR, 
				pServ->GetName(), (LPCTSTR) pServ->m_ip.GetAddrStr(), pServ->GetAgeHours());
			g_Cfg.m_Servers.DeleteOb(pServ);
			i--;
			continue;
		}

		pServ->QueueCharToServer();	// Check if the mover task is running ? or needs to.

		if ( pServ->GetTimeSinceLastValid() <= (TICK_PER_SEC * 60 * 60 * 8))
		{
			// Only count this if it has responded recently.
			DEBUG_CHECK(pServ->GetClientsAvg()>=0);
			iTotalClients += pServ->GetClientsAvg();
			iTotalAccounts += pServ->StatGet(SERV_STAT_ACCOUNTS);
		}
	}

	m_iTotalPolledClients = iTotalClients;
	m_iTotalPolledAccounts = iTotalAccounts;
}

void CBackTask::SendOutgoingMail()
{
	// Attempt to send the outgoing mail now.

	m_timeNextMail = CServTime::GetCurrentTime() + ( 60 * TICK_PER_SEC );

	// look thru all the accounts for outgoing mail.
	// ??? This is SERIOSLY asking for multi-thread problems !

	for ( int i=0; true; i++ )
	{
		CAccountRef pAccount = g_Accounts.Account_Get(i);
		if ( pAccount == NULL )
			break;
		pAccount->SendAllOutgoingMail();
	}
}

void CBackTask::EntryTask()
{
	// Do all the background stuff we don't want to bog down the game server thread.
	// None of this is time critical.

	OnCreate();
	while ( g_Cfg.CanRunBackTask() )
	{
		try
		{
			// register the server every few hours or so.
			if ( ! g_Cfg.m_sRegisterServer.IsEmpty() &&
				CServTime::GetCurrentTime() >= m_timeNextRegister )
			{
				//g_Log.Event(LOGL_EVENT, "Attempting to register server" DEBUG_CR);
				RegisterServer();
			}

			// ping the monitored servers.
			if ( g_Cfg.m_iPollServers &&
				CServTime::GetCurrentTime() >= m_timeNextPoll )
			{
				//g_Log.Event(LOGL_EVENT, "Attempting to poll servers" DEBUG_CR);
				PollServers();
			}

			// Check for outgoing mail.
			if ( CServTime::GetCurrentTime() >= m_timeNextMail )
			{
				//g_Log.Event(LOGL_EVENT, "Attempting to send mail" DEBUG_CR);
				SendOutgoingMail();
			}
		}

		catch ( CGrayError &e )
		{
			g_Log.CatchEvent( &e, "Back Task" );
		}
		catch (...)	// catch all
		{
			g_Log.CatchEvent( NULL, "Back Task" );
		}

		// ? watch for incoming connections and data ???.
		Sleep( 10 * 1000 );
	}

	// Tell the world we are closed. CloseHandle() required ?
	g_Log.Event(LOGL_EVENT|LOGM_INIT, "Closing background threads" DEBUG_CR);
	OnClose();
}

THREAD_ENTRY_RET _cdecl CBackTask::EntryProc( void * lpThreadParameter ) // static
{
	g_BackTask.EntryTask();
}

void CBackTask::CreateThread()
{
	// beginthread() and endthread() are the portable versions of this !

	if ( !g_Cfg.CanRunBackTask() )
	{
		// don't bother with the background stuff.
		return;
	}
	g_Log.Event(LOGL_EVENT, "Attempting to start background thread" DEBUG_CR);
	CThread::CreateThread( EntryProc );
}

void CBackTask::CheckStuckThread()
{
	// Periodically called to check if the thread is stuck.

	if ( IsActive())
		return;

	CreateThread();
}


