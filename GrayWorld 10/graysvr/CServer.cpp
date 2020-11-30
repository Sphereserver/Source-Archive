//
// CServer.cpp
//

#include "graysvr.h"	// predef header.

/////////////////////////////////////////////////////////////////
// -CSocket

void CSocket :: Close()
{
	if ( m_hSocket == INVALID_SOCKET ) return;
#ifdef _WIN32
	closesocket( m_hSocket );
#else
	shutdown(m_hSocket, 2);
	close(m_hSocket);
#endif
	m_hSocket = INVALID_SOCKET;
}

////////////////////////////////////////////////////////////////////////////////////////
// -CServer 

CServer :: CServer()
{
	m_pAdminClient = NULL;
	m_fKeepRun = true;
	m_fVerbose = false;
	m_fSecure = true;
	m_fBroadcast = false;
	m_iSpeakIndex = 0;
	m_error = 0;
	m_nClientsMax = FD_SETSIZE;
	m_nGuestsMax = 0;
	m_StartCount=0;
	m_ServerCount=0;
}

void CServer :: Shutdown( int iTime ) // If shutdown is initialized
{
	if ( iTime == 0 )
	{
		if ( ! m_Clock_Shutdown ) return;
		m_Clock_Shutdown=0;
		World.Broadcast("Shutdown has been interrupted.");
		return;
	}

	m_Clock_Shutdown = World.m_Clock_Time + iTime;

	CString sTmp;
	sTmp.Format( "Server going down in %i seconds.", iTime );
	World.Broadcast( sTmp );
}

bool CServer :: IniRead()
{
	CScript s;
	if ( ! s.Open( GRAY_FILE ".ini" )) // Open script file
	{
		return( false );
	}

static const char * table[] =
{
	"FILES",
	"VERBOSE",
	"GUESTS",
	"CLIENTS",
	"LOG",
	"SECURE",
};

	if ( ! s.FindSec( GRAY_FILE )) return( false );

	while ( 1 )
	{
		if ( ! s.ReadParse()) break;
		switch ( FindTable( s.m_Line, table, COUNTOF( table )))
		{
		case 0: World.m_sBase = s.m_pArg; break;
		case 1: m_fVerbose = atoi( s.m_pArg ); break;
		case 2: m_nGuestsMax = atoi( s.m_pArg ); break;
		case 3: m_nClientsMax = atoi( s.m_pArg ); break;
		case 4: if ( atoi( s.m_pArg )) Log.Open(); break;
		case 5: m_fSecure = atoi( s.m_pArg ); break;
		}
	}

	if ( ! s.FindSec( "SERVERS" )) return( false );

	while ( 1 )
	{
		if ( ! s.Read1()) break;
		m_Servers[m_ServerCount].m_sName = s.m_Line;
		if ( ! s.Read1()) break;
		m_Servers[m_ServerCount].m_ip.s_addr = inet_addr( s.m_Line );
		if ( ! s.Read1()) break;
		m_Servers[m_ServerCount].m_port = atoi(s.m_Line);
		if ( ++m_ServerCount >= COUNTOF( m_Servers )) break;
	}

	if ( ! s.FindSec( "STARTS" )) return( false );

	while ( 1 )
	{
		if ( ! s.Read1()) break;
		m_Starts[m_StartCount].m_sArea = s.m_Line;
		if ( ! s.Read1()) break;
		m_Starts[m_StartCount].m_sName = s.m_Line;
		if ( ! s.Read1()) break;
		m_Starts[m_StartCount].m_p.Read( s.m_Line );
		
		if ( ++m_StartCount >= COUNTOF( m_Starts )) break;
	}

	return( m_ServerCount && m_StartCount );
}

#if defined( _CPPUNWIND ) && defined( _DEBUG )
void CServer :: Assert( void * pExp, void *pFile, unsigned uLine )
{
	if ( m_fSecure )
	{
		DEBUG_ERR(( "ERROR:Assert:'%s' file '%s', line %d\n", pExp, pFile, uLine ));
		throw( 1 );
	}
	// go the normal assert route.
	assert( pExp, pFile, uLine );
}
#endif

void CServer :: Printf( const char * pStr ) const
{
#ifdef _WIN32
	printf( pStr );	// print out locally as well.
#endif
	if ( m_pAdminClient != NULL )
	{
		m_pAdminClient->Send( pStr, strlen( pStr ) + 1 );
	}
}

void CServer :: VPrintf( const char * pFormat, va_list pArgs )  const
{
#ifdef _WIN32
	vprintf( pFormat, pArgs );	// print out locally as well.
#endif
	if ( m_pAdminClient != NULL )
	{
		char * pTemp = GetTempStr();
		size_t len = vsprintf( pTemp, pFormat, pArgs );
		m_pAdminClient->Send( pTemp, len + 1 );
	}
}

void CServer :: ListClients( CClient * pClientCmd ) const
{
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		struct sockaddr_in Name;
		pClient->GetPeerName( &Name );

		CString sMsg;
		if ( pClient->m_pChar != NULL ) 
		{
			sMsg.Format( "%x:Acc='%s', (%s) Char='%s',(%d,%d,%d)\n",
				pClient->GetSocket(), 
				(const char*) pClient->m_sAccount, 
				inet_ntoa( Name.sin_addr ),
				pClient->m_pChar->GetName(), 
				pClient->m_pChar->m_p.m_x, pClient->m_pChar->m_p.m_y, pClient->m_pChar->m_p.m_z );
		}
		else
		{
			sMsg.Format( "%x:Acc='%s', (%s) NOT LOGGED IN\n",
				pClient->GetSocket(), 
				(const char*) pClient->m_sAccount, 
				inet_ntoa( Name.sin_addr ));
		}

		if ( pClientCmd != NULL )
		{
			pClientCmd->addSysMessage( sMsg );
		}
		else
		{
			Printf( sMsg );
		}
	}
}

void CServer :: OnConsoleCmd( char key )
{
	// Broadcast mode.
	if ( m_fBroadcast )
	{
		if ( key == 0x1b )
		{
			m_fBroadcast = false;
			Printf("Broadcast disabled\n" );
			return;
		}
		if ( m_iSpeakIndex >= sizeof( m_szSpeakText )-1 )
		{
			World.Broadcast( m_szSpeakText );
			m_iSpeakIndex = 0;
		}
		m_szSpeakText[m_iSpeakIndex++] = key;
		m_szSpeakText[m_iSpeakIndex] = '\0';
		switch ( key )
		{
		case '\n':
		case '\r':
			Printf( m_szSpeakText );
			Printf( "\n" );
			World.Broadcast( m_szSpeakText );
			m_iSpeakIndex = 0;
			break;
		}
		return;
	}

	switch ( toupper( key ))
	{
	case '?':
		Printf(
			"Available Commands:\n"
			"ESC = go into/out of broadcast mode.\n"
			"X = immediate exit of the server\n"
			"S = Secure mode toggle\n"
			"G = Garbage collection\n"
			"# = Save world\n"
			"L = Toggle log file\n"
			"V = Verbose Mode\n"
			"I = Information\n"
			"C = Clients List\n"
			);
		return;
	case 0x1b:	// ESCAPE KEY
		m_fBroadcast = true;
		m_iSpeakIndex = 0;
		m_szSpeakText[0] = '\0';
		Printf("Broadcast enabled\n" );
		return;
	case 'X':
		if (m_fSecure)
		{
			Printf("NOTE: Secure mode prevents keyboard commands!\n");
		}
		else
		{
			Log.Event( "Immediate Shutdown initialized!\n");
			m_fKeepRun = false;
		}
		break;
	case 'S':
		if (m_fSecure)
		{
			Printf( "Secure mode disabled.\n");
			World.Broadcast( "WARNING: The world is now running in DEBUG MODE" );
			m_fSecure=false;
		}
		else
		{
			Printf( "Secure mode re-enabled.\n");
			World.Broadcast( "The world is now running in NORMAL MODE" );
			m_fSecure=true;
		}
		break;
	case 'G':
		World.GarbageCollection();
		break;
	case '#':
		World.Save();
		break;
	case 'L':
		// Turn the log file on or off.
		if ( Log.IsOpen())
		{
			Log.Close();
			Printf( "Log file closed.\n");
		}
		else
		{
			Log.Open();
			Printf( "Log file opened.\n");
		}
		break;
	case 'V':
		if ( m_fVerbose)
		{
			Log.Event( "Verbose display disabled.\n");
			m_fVerbose=false;
		}
		else
		{
			Log.Event( "Verbose display enabled.\n");
			m_fVerbose=true;
		}
		break;
	case 'I':
		Printf( World.GetInformation());
		break;
	case 'C':
		// List all clients on line.
		ListClients();
		break;
	}
}

void CServer :: SocketsReceive() // Check for messages from the clients
{
	// What sockets do I want to look at ?
	struct fd_set readfds;
	FD_ZERO(&readfds);

	FD_SET(GetSocket(), &readfds);
	int nfds = GetSocket();

	CClient * pClientNext;
	CClient * pClient = GetClientHead();
	for ( ; pClient!=NULL; pClient = pClientNext )
	{
		pClientNext = pClient->GetNext();
		FD_SET(pClient->GetSocket(),&readfds);
		if ( pClient->GetSocket() > nfds ) 
			nfds = pClient->GetSocket();
	}

	timeval Timeout;	// time to wait for data.	
	Timeout.tv_sec=0;
	Timeout.tv_usec=200;	
	int ret = select( nfds+1, &readfds, NULL, NULL, &Timeout );
	if ( ret <= 0 ) return;

	// Any events from clients ?
	for ( pClient = GetClientHead(); pClient!=NULL; pClient = pClientNext )
	{
		pClientNext = pClient->GetNext();
		if ( FD_ISSET( pClient->GetSocket(), &readfds ))
		{
			if ( ! pClient->xRecvData()) 
			{
				delete pClient;
			}
		}
	}

	// Any new connections ?
	if ( FD_ISSET( GetSocket(), &readfds) && m_Clients.GetCount() < m_nClientsMax ) 
	{
		int len = sizeof( struct sockaddr_in );
		struct sockaddr_in client_addr;

		SOCKET client = Accept( &client_addr, &len );
		if ( client < 0 )
		{
			Log.Error("ERROR: Error at client connection!\n");
			m_error=1;
			m_fKeepRun=false;
			return;
		}
		
		m_Clients.InsertAfter( new CClient( client ));
	}
}

void CServer :: SocketsFlush() // Sends ALL buffered data
{
	for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		pClient->addPause( false );	// always turn off pause here if it is on.
		pClient->xFlush();
	}
}

void CServer :: OnTick() // Check all timers
{
#ifdef _WIN32
	if (kbhit())
	{
		OnConsoleCmd( getch());
	}
#endif

	// keep world time always moving forward.
	World.SetTime();

	// Check client for incoming packets.
	SocketsReceive();
	for ( CClient * pClient = GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->m_bin_len )	continue;// process queued up packets in fair order.
#ifdef _CPPUNWIND
		if ( m_fSecure )	// enable the try code.
		{
			try
			{
				pClient->xProcess( pClient->DispatchMsg());
			}
			catch (...)	// catch all
			{
				// clear this clients messages. so it won't do the same bad thing next time.
				pClient->xProcess( false );
				throw( 1 );
			}
		}
		else
#endif
			pClient->xProcess( pClient->DispatchMsg());
	}
	SocketsFlush();

	World.OnTick();

	if ( m_Clock_Shutdown )
	{
		if ( m_Clock_Shutdown <= World.m_Clock_Time ) m_fKeepRun=false;
	}
}

bool CServer :: SocketsInit() // Initialize sockets
{
#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested;

	wVersionRequested=0x0101;
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err)
	{
		Log.Error("ERROR: Winsock 1.1 not found...\n");
		return( false );
	}
	if ( m_nClientsMax > wsaData.iMaxSockets )
		m_nClientsMax = wsaData.iMaxSockets;
#endif

	if ( ! Create())
	{
		Log.Error("ERROR: Unable to create socket\n");
		return( false );
	}

#ifndef _WIN32
	int on=1;
	int off=0;
	setsockopt(GetSocket(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = INADDR_ANY;
	connection.sin_port = htons( m_Servers[0].m_port );
	int bcode = Bind( &connection );
	if (bcode<0)
	{
		Log.Error( "ERROR: Unable to bind socket 1 - Error code: %i\n", bcode );
		return( false );
	}

	// Max number we can deal with.
	if ( m_nClientsMax > FD_SETSIZE ) 
		m_nClientsMax = FD_SETSIZE;

	listen( GetSocket(), m_nClientsMax );	// FD_SETSIZE

	// What are we listing our port as to the world.
	Log.Event( "Server started at %s port %d\n", 
		inet_ntoa( m_Servers[0].m_ip ), m_Servers[0].m_port );
	return( true );
}

void CServer :: SocketsClose() 
{
	m_Clients.DeleteAll();
	Close();
}
