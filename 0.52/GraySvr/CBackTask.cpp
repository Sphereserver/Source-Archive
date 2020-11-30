//
// CBackTask.cpp
// Copyright Menace Software (www.menasoft.com).
// Background task stuff.
//

#include "graysvr.h"	// predef header.

#ifdef _WIN32
#include <process.h>    // _beginthread, _endthread
#else
#include <pthread.h>	// pthread_create
#endif

//////////////////////////////////////////////////////////////////////
// -CServRef

CServRef::CServRef( const TCHAR * pszName, DWORD dwIP )
{
	// Statistics.
	memset( m_dwStat, 0, sizeof( m_dwStat ));	// THIS MUST BE FIRST !

	SetName( pszName );
	m_ip.SetAddr( dwIP );	// SOCKET_LOCAL_ADDRESS
	m_ip.SetPort( GRAY_DEF_PORT );
	m_LastPollTime = 0;
	m_LastValidTime = 0;
	m_CreateTime = g_World.GetTime();
	m_iClientsAvg = 0;

	m_TimeZone = 0;	// Greenwich mean time.
	m_eAccApp = ACCAPP_Unspecified;
}

void CServRef::SetName( const TCHAR * pszName )
{
	if ( ! pszName )
		return;

	// No HTML tags using <> either.
	TCHAR szName[ 2*MAX_ACCOUNT_NAME_SIZE ];
	int len = GetBareText( szName, pszName, sizeof(szName), "<>/\"\\" );
	if ( ! len )
		return;

	// allow just basic chars. No spaces, only numbers, letters and underbar.
	if ( g_Serv.IsObscene( szName ))
	{
		DEBUG_ERR(( "Obscene server '%s' ignored.\n", szName ));
		return;
	}

	m_sName = szName;
}

int CServRef::GetTimeSinceLastValid() const
{
	return( g_World.GetTime() - m_LastValidTime );
}

void CServRef::ClientsResetAvg()
{
	// Match the avg closer to the real current count.

#if 0
	int iClientsNew = StatGet(SERV_STAT_CLIENTS);
	if ( iClientsNew < m_iClientsAvg )
	{
		iClientsNew += ( m_iClientsAvg - iClientsNew ) / 2;
	}
	m_iClientsAvg = iClientsNew;
#endif

}

void CServRef::addToServersList( CCommand & cmd, int index, int j ) const
{
	cmd.ServerList.m_serv[j].m_count = index;

	strncpy( cmd.ServerList.m_serv[j].m_name, GetName(), sizeof(cmd.ServerList.m_serv[j].m_name));
	cmd.ServerList.m_serv[j].m_name[ sizeof(cmd.ServerList.m_serv[j].m_name)-1 ] ='\0';

	cmd.ServerList.m_serv[j].m_zero32 = 0;
	cmd.ServerList.m_serv[j].m_percentfull = StatGet(SERV_STAT_CLIENTS);
	cmd.ServerList.m_serv[j].m_timezone = m_TimeZone;	// GRAY_TIMEZONE

	DWORD dwAddr = m_ip.GetAddr();
	cmd.ServerList.m_serv[j].m_ip[3] = ( dwAddr >> 24 ) & 0xFF;
	cmd.ServerList.m_serv[j].m_ip[2] = ( dwAddr >> 16 ) & 0xFF;
	cmd.ServerList.m_serv[j].m_ip[1] = ( dwAddr >> 8  ) & 0xFF;
	cmd.ServerList.m_serv[j].m_ip[0] = ( dwAddr       ) & 0xFF;
}

enum SC_TYPE
{
	SC_ACCAPP,
	SC_ACCAPPS,
	SC_ADMIN,
	SC_ADMINEMAIL,	// 	m_sEMail
	SC_AGE,
	SC_ALLOCS,
	SC_CHARS,
	SC_CLIENTS,
	SC_CLIENTSAVG,
	SC_CREATE,
	SC_EMAIL,
	SC_EMAILLINK,
	SC_IP,
	SC_ITEMS,
	SC_LANG,
	SC_LASTPOLLTIME,
	SC_LASTVALIDDATE,
	SC_LASTVALIDTIME,
	SC_MEM,
	SC_NAME,
	SC_NOTES,
	SC_PORT,
	SC_REGPASS,
	SC_SERVIP,
	SC_SERVLISTCOL,
	SC_SERVNAME,
	SC_SERVPORT,
	SC_STATCLIENTS,
	SC_STATITEMS,
	SC_STATMEMORY,
	SC_STATNPCS,
	SC_STATUS,
	SC_TIMEZONE,
	SC_TZ,
	SC_URL,			// m_sURL
	SC_URLLINK,
	SC_VALID,
	SC_VER,
	SC_VERSION,
};

const TCHAR * CServRef::sm_KeyTable[] =	// static
{
	"ACCAPP",
	"ACCAPPS",
	"ADMIN",
	"ADMINEMAIL",	// 	m_sEMail
	"AGE",
	"ALLOCS",
	"CHARS",
	"CLIENTS",
	"CLIENTSAVG",
	"CREATE",
	"EMAIL",
	"EMAILLINK",
	"IP",
	"ITEMS",
	"LANG",
	"LASTPOLLTIME",
	"LASTVALIDDATE",
	"LASTVALIDTIME",
	"MEM",
	"NAME",
	"NOTES",
	"PORT",
	"REGPASS",
	"SERVIP",
	"SERVLISTCOL",
	"SERVNAME",
	"SERVPORT",
	"STATCLIENTS",
	"STATITEMS",
	"STATMEMORY",
	"STATNPCS",
	"STATUS",
	"TIMEZONE",
	"TZ",
	"URL",			// m_sURL
	"URLLINK",
	"VALID",
	"VER",
	"VERSION",
};
 
static const TCHAR * AccAppTable[ ACCAPP_QTY ] =
{
	"Closed",		// Closed. Not accepting more.
	"EmailApp",		// Must send email to apply.
	"Free",			// Anyone can just log in and create a full account.
	"GuestAuto",	// You get to be a guest and are automatically sent email with u're new password.
	"GuestTrial",	// You get to be a guest til u're accepted for full by an Admin.
	"Other",		// specified but other ?
	"Unspecified",	// Not specified.
	"WebApp",		// Must fill in a web form and wait for email response
	"WebAuto",		// Must fill in a web form and automatically have access
};

bool CServRef::r_LoadVal( CScript & s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case SC_ACCAPP:
	case SC_ACCAPPS:
		// Treat it as a value or a string.
		if ( isdigit( s.GetArgStr()[0] ))
		{
			m_eAccApp = (ACCAPP_TYPE) s.GetArgVal();
		}
		else
		{
			// Treat it as a string. "Manual","Automatic","Guest"
			m_eAccApp = (ACCAPP_TYPE) FindTableSorted(  s.GetArgStr(), AccAppTable, COUNTOF( AccAppTable ));
		}
		if ( m_eAccApp < 0 || m_eAccApp >= ACCAPP_QTY )
			m_eAccApp = ACCAPP_Unspecified;
		break;
	case SC_ADMIN:
		goto get_email;
	case SC_ADMINEMAIL:
		goto get_email;
	case SC_ALLOCS:
		SetStat( SERV_STAT_ALLOCS, s.GetArgVal());
		break;
	case SC_CHARS:
		goto get_npcs;
	case SC_CLIENTS:
		goto get_clients;
	case SC_CLIENTSAVG:
		{
		m_iClientsAvg = s.GetArgVal();
		if ( m_iClientsAvg < 0 )
			m_iClientsAvg = 0;
		if ( m_iClientsAvg > FD_SETSIZE )	// Number is bugged !
			m_iClientsAvg = FD_SETSIZE;
		}
		break;
	case SC_CREATE:
		m_CreateTime = g_World.GetTime() + ( s.GetArgVal() * TICK_PER_SEC );
		break;
	case SC_EMAIL:
		goto get_email;
	case SC_EMAILLINK:
get_email:
		if ( ! g_Serv.m_sEMail.IsEmpty() &&
			strstr( s.GetArgStr(), g_Serv.m_sEMail ))
			return( false );
		if ( ! g_Serv.IsValidEmailAddressFormat( s.GetArgStr()))
			return( false );
		if ( g_Serv.IsObscene( s.GetArgStr()))	// Is the name unacceptable?
			return( false );
		m_sEMail = s.GetArgStr();
		break;
	case SC_IP:
		goto get_ip;
	case SC_ITEMS:
		goto get_items;
	case SC_LANG:
		{
			TCHAR szLang[ 32 ];
			int len = GetBareText( szLang, s.GetArgStr(), sizeof(szLang), "<>/\"\\" );
			if ( g_Serv.IsObscene(szLang))	// Is the name unacceptable?
				return( false );
			m_sLang = szLang;
		}
		break;
	case SC_LASTPOLLTIME:
		m_LastPollTime = g_World.GetTime() + ( s.GetArgVal() * TICK_PER_SEC );
		break;
	case SC_LASTVALIDDATE:
		m_LastValidDate.Read( s.GetArgStr());
		break;
	case SC_LASTVALIDTIME:
		{
			int iVal = s.GetArgVal() * TICK_PER_SEC;
			if ( iVal < 0 )
				m_LastValidTime = g_World.GetTime() + iVal;
			else
				m_LastValidTime = g_World.GetTime() - iVal;
		}
		break;
	case SC_MEM:
		goto get_mem;
	case SC_NAME:
		goto get_name;
	case SC_NOTES:
		// Make sure this is not too long !
		// make sure there are no bad HTML tags in here ?
		{
			TCHAR szTmp[256];
			int len = GetBareText( szTmp, s.GetArgStr(), COUNTOF(szTmp), "<>/" );	// no tags 
			if ( g_Serv.IsObscene( szTmp ))	// Is the name unacceptable?
				return( false );
			m_sNotes = szTmp;
		}
		break;
	case SC_PORT:
		goto get_port;
	case SC_REGPASS:
		m_sRegisterPassword = s.GetArgStr();
		break;
	case SC_SERVIP:
get_ip:
		m_ip.SetHostPortStr( s.GetArgStr());
		break;

	case SC_SERVNAME:
get_name:
		SetName( s.GetArgStr());
		break;
	case SC_SERVPORT:
get_port:
		m_ip.SetPort( s.GetArgVal());
		break;

	case SC_STATCLIENTS:
get_clients:
		{
			int iClients = s.GetArgVal();
			if ( iClients < 0 ) 
				return( false );	// invalid
			if ( iClients > FD_SETSIZE )	// Number is bugged !
				return( false );
			SetStat( SERV_STAT_CLIENTS, iClients );
			if ( iClients > m_iClientsAvg )
				m_iClientsAvg = StatGet(SERV_STAT_CLIENTS);
		}
		break;
	case SC_STATITEMS:
get_items:
		SetStat( SERV_STAT_ITEMS, s.GetArgVal());
		break;
	case SC_STATMEMORY:
get_mem:
		SetStat( SERV_STAT_MEM, s.GetArgVal() * 1024 );
		break;
	case SC_STATNPCS:
get_npcs:
		SetStat( SERV_STAT_CHARS, s.GetArgVal());
		break;
	case SC_STATUS:
		return( ParseStatus( s.GetArgStr(), true ));
	case SC_TIMEZONE:
get_tz:
		m_TimeZone = s.GetArgVal();
		break;
	case SC_TZ:
		goto get_tz;

	case SC_URL:
get_url:
		// It is a basically valid URL ?
		{
		if ( ! g_Serv.m_sURL.IsEmpty() && 
			strstr( s.GetArgStr(), g_Serv.m_sURL ))
			return( false );
		if ( ! strcmpi( s.GetArgStr(), "www.menasoft.com" ))
			return( false );
		if ( ! strchr( s.GetArgStr(), '.' ))
			return( false );
		if ( g_Serv.IsObscene( s.GetArgStr()))	// Is the name unacceptable?
			return( false );
		m_sURL = s.GetArgStr();
		}
		break;
	case SC_URLLINK:
		// try to make a link of it.
		goto get_url;

	case SC_VALID:
		// Force it to be valid.
		m_LastPollTime = (s.GetArgVal())? m_LastValidTime : 1 ;
		break;

	case SC_VER:
		goto get_version;
	case SC_VERSION:
get_version:
		m_sVersion = s.GetArgStr();
		break;

	default:
		return( CScriptObj::r_LoadVal(s));
	}
	return( true );
}

bool CServRef::r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case SC_ACCAPP:
		sVal.FormatVal( m_eAccApp );
		break;
	case SC_ACCAPPS:
		// enum string
		if ( m_eAccApp < 0 || m_eAccApp >= ACCAPP_QTY )
			m_eAccApp = ACCAPP_Unspecified;
		sVal = AccAppTable[ m_eAccApp ];
		break;
	case SC_ADMIN:
		goto get_email;
	case SC_ADMINEMAIL:
get_email:
		sVal = m_sEMail;
		break;
	case SC_AGE:
		// display the age in days.
		sVal.FormatVal( GetAgeHours()/24 );
		break;
	case SC_ALLOCS:
		sVal.FormatVal( StatGet( SERV_STAT_ALLOCS ));
		break;
	case SC_CHARS:
		goto get_npcs;
	case SC_CLIENTS:
		goto get_clients;
	case SC_CLIENTSAVG:
		sVal.FormatVal( GetClientsAvg());
		break;
	case SC_CREATE:
		sVal.FormatVal(( m_CreateTime - g_World.GetTime()) / TICK_PER_SEC );
		break;
	case SC_EMAIL:
		goto get_email;
	case SC_EMAILLINK:
		if ( m_sEMail.IsEmpty())
		{
			sVal.Empty();
			break;
		}
		sVal.Format( "<a href=\"mailto:%s\">%s</a>", (const TCHAR*) m_sEMail, (const TCHAR *) m_sEMail );
		break;
	case SC_IP:
		goto get_ip;
	case SC_ITEMS:
		goto get_items;

	case SC_LANG:
		sVal = m_sLang;
		break;

	case SC_LASTPOLLTIME:
		sVal.FormatVal( m_LastPollTime ? (( m_LastPollTime - g_World.GetTime()) / TICK_PER_SEC ) : -1 );
		break;
	case SC_LASTVALIDDATE:
		//if ( m_LastValidDate.IsValid())
			//sVal = m_LastValidDate.Write();

		if ( m_LastValidTime )
			sVal.FormatVal( GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 ));
		else
			sVal = "NA";
		break;
	case SC_LASTVALIDTIME:
		// How many seconds ago.
		sVal.FormatVal( m_LastValidTime ? ( GetTimeSinceLastValid() / TICK_PER_SEC ) : -1 );
		break;
	case SC_MEM:
		goto get_mem;
	case SC_NAME:
		goto get_name;
	case SC_NOTES:
		sVal = m_sNotes;
		break;
	case SC_PORT:
		goto get_port;
	case SC_REGPASS:
		if ( pSrc == NULL || pSrc->GetPrivLevel() < PLEVEL_Admin )
			return( false );
		sVal = m_sRegisterPassword;
		break;
	case SC_SERVIP:
get_ip:
		sVal = m_ip.GetAddrStr();
		break;
	case SC_SERVLISTCOL:
		// Set the alternating color.
		sVal = (CWebPageDef::sm_iServListIndex&1) ? "bgcolor=\"#E8E8E8\"" : "";
		break;
	case SC_SERVNAME:
get_name:
		sVal = GetName();	// What the name should be. Fill in from ping.
		break;
	case SC_SERVPORT:
get_port:
		sVal.FormatVal( m_ip.GetPort());
		break;
	case SC_STATCLIENTS:
get_clients:
		sVal.FormatVal( StatGet( SERV_STAT_CLIENTS ));
		break;
	case SC_STATITEMS:
get_items:
		sVal.FormatVal( StatGet( SERV_STAT_ITEMS ));
		break;
	case SC_STATMEMORY:
get_mem:
		sVal.FormatVal( StatGet( SERV_STAT_MEM )/1024 );
		break;
	case SC_STATNPCS:
get_npcs:
		sVal.FormatVal( StatGet( SERV_STAT_CHARS ));
		break;
	case SC_STATUS:
		sVal = m_sStatus;
		break;
	case SC_TIMEZONE:
get_tz:
		sVal.FormatVal( m_TimeZone );
		break;
	case SC_TZ:
		goto get_tz;
	case SC_URL:
		sVal = m_sURL;
		break;
	case SC_URLLINK:
		// try to make a link of it.
		if ( m_sURL.IsEmpty())
		{
			sVal = GetName();
			break;
		}
		sVal.Format( "<a href=\"http://%s\">%s</a>", (const TCHAR*) m_sURL, GetName() );
		break;
	case SC_VALID:
		sVal.FormatVal( IsConnected());
		break;
	case SC_VER:
		goto get_version;
	case SC_VERSION:
get_version:
		sVal = m_sVersion;
		break;
	default:
		return( CScriptObj::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

void CServRef::WriteCreated( CScript & s )
{
	// Write this only if it was created dynamically.
	if ( ! m_CreateTime )
		return;

	s.WriteSection( "SERVER %s", GetName() );

	s.WriteKey( "IP", m_ip.GetAddrStr());
	s.WriteKeyVal( "PORT", m_ip.GetPort());
	s.WriteKeyVal( "TZ", m_TimeZone );
	s.WriteKeyVal( "CREATE", ( m_CreateTime - g_World.GetTime()) / TICK_PER_SEC );
	s.WriteKeyVal( "LASTVALIDTIME", GetTimeSinceLastValid() / TICK_PER_SEC );
	s.WriteKey( "LASTVALIDDATE", m_LastValidDate.Write() );
	s.WriteKeyVal( "CLIENTSAVG", m_iClientsAvg );

	if ( ! m_sVersion.IsEmpty())
	{
		s.WriteKey( "VER", m_sVersion );
	}
	if ( ! m_sURL.IsEmpty())
	{
		s.WriteKey( "URL", m_sURL );
	}
	if ( ! m_sEMail.IsEmpty())
	{
		s.WriteKey( "EMAIL", m_sEMail );
	}
	if ( ! m_sRegisterPassword.IsEmpty())
	{
		s.WriteKey( "REGPASS", m_sRegisterPassword );
	}
	if ( ! m_sNotes.IsEmpty())
	{
		s.WriteKey( "NOTES", m_sNotes );
	}
	if ( ! m_sLang.IsEmpty())
	{
		s.WriteKey( "LANG", m_sLang );
	}
	if ( m_eAccApp )
	{
		s.WriteKeyVal( "ACCAPP", m_eAccApp );
	}
}

bool CServRef::ParseStatus( const TCHAR * pszStatus, bool fStore )
{
	// Take the status string we get from the server and interpret it.
	// fStore = set the Status msg.

	ASSERT( this != &g_Serv );
	m_LastValidDate.FillRealTime();
	m_LastValidTime = g_World.GetTime();

	TCHAR bBareData[ MAX_SCRIPT_LINE_LEN ];
	int len = GetBareText( bBareData, pszStatus, sizeof(bBareData)-1 );
	if ( ! len )
	{
		return false;
	}

	// Parse the data we get. Older versions did not have , delimiters
	TCHAR * pData = bBareData;
	while ( pData )
	{
		TCHAR * pEquals = strchr( pData, '=' );
		if ( pEquals == NULL ) 
			break;

		*pEquals = '\0';
		pEquals++;

		TCHAR * pKey = strrchr( pData, ' ' );
		if ( pKey == NULL )
			pKey = pData;
		else
			pKey++;

		pData = strchr( pEquals, ',' );
		if ( pData )
		{
			TCHAR * pEnd = pData;
			pData ++;
			while ( ISWHITESPACE( pEnd[-1] )) 
				pEnd--;
			*pEnd = '\0';
		}

		r_SetVal( pKey, pEquals );
	}

	if ( fStore )
	{
		m_sStatus = "OK";
	}

	return( true );
}

int CServRef::GetAgeHours() const
{
	// This is just the amount of time it has been listed.
	return(( g_World.GetTime() - m_CreateTime ) / ( TICK_PER_SEC * 60 * 60 ));
}

bool CServRef::IsValidStatus() const
{
	if ( ! m_CreateTime ) 
		return( true ); // Never delete this.
	if ( ! m_LastValidTime )
		return( true );	// it was defined in the *.INI file. but has not updated.

	// Give the old reliable servers a break.
	DWORD dwAge = GetAgeHours();

	// How long has it been down ?
	DWORD dwInvalidHours = GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 * 60 );

	return( dwInvalidHours <= 36 + ( dwAge/24 ) * 2 );
}

bool CServRef::PollStatus()
{
	// NOTE: This is a synchronous function that could take a while.
	// RETURN: 0 = failed to respond
	ASSERT( this != &g_Serv );

	// Try to ping the server to find out what it's status is.

	const TCHAR * pszTrying;

	CGSocket sock;
	if ( ! sock.Create())
	{
		pszTrying = "Socket";
failout:
		m_LastPollTime = g_World.GetTime();
		m_sStatus.Format( "Failed: %s: down for %d minutes",
			pszTrying,
			GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 ));
		return( false );
	}

	if ( sock.Connect( m_ip ))
	{
		pszTrying = "Connect";
		goto failout;
	}

	char bData = 0x21;	// GetStatusString arg data.
	if ( IsConnected() && GetRandVal( 16 ))
	{
		// just ask it for stats. don't bother with header info.
		bData = 0x23;	// try this command as well.
	}

	// got a connection
	// Ask for the data.
	if ( sock.Send( &bData, 1 ) != 1 )
	{
		pszTrying = "Send";
		goto failout;
	}

	// Wait for some sort of response.
	struct fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock.GetSocket(), &readfds);

	timeval Timeout;	// time to wait for data.
	Timeout.tv_sec=10;
	Timeout.tv_usec=200;
	int ret = select( sock.GetSocket()+1, &readfds, NULL, NULL, &Timeout );
	if ( ret <= 0 )
	{
		pszTrying = "No Response";
		goto failout;
	}

	// Any events from clients ?
	if ( ! FD_ISSET( sock.GetSocket(), &readfds ))
	{
		pszTrying = "Timeout";
		goto failout;
	}

	// How long do we block for is the issue here.
	//
	char bRetData[ MAX_SCRIPT_LINE_LEN ];
	int len = sock.Receive( bRetData, sizeof( bRetData ) - 1 );
	if ( len <= 0 )
	{
		pszTrying = "Receive";
		goto failout;
	}

	if ( ! ParseStatus( bRetData, bData == 0x21 ))
	{
		pszTrying = "Bad Status Data";
		goto failout;
	}

	m_LastPollTime = g_World.GetTime();
	return( true );
}

/////////////
// -CThread

void CThread::CreateThread( void ( _cdecl * pEntryProc )(void *) )
{
	if ( IsActive())
		return;	// Already active.

#ifdef _WIN32
	m_hThread = _beginthread( pEntryProc, 0, this );
	if ( m_hThread == -1 )
	{
		m_hThread = NULL;
	}
#else
	//pthread_t thread;
	//pthread_attr_t attr;
	//int pthread_create( &thread, &attr, EntryProc, this );

#endif
}

bool CThread::TerminateThread( DWORD dwExitCode )
{
	if ( ! IsActive()) 
		return true;
#ifdef _WIN32
	if ( m_ThreadID == GetCurrentThreadId())
	{
		_endthread();	// , dwExitCode
	}
	else
	{
		BOOL fRet = ::TerminateThread( (HANDLE)m_hThread, dwExitCode );
		if ( fRet )
		{
			CloseHandle( (HANDLE) m_hThread );
		}
		else
		{
			DEBUG_CHECK( fRet );
		}
	}
#else

#endif
	OnClose();
	return( true );
}

void CThread::WaitForClose( int iSec )
{
	// wait for another thread to close.
	int icount = iSec*10;
	while ( IsActive() && icount -- )
	{
		Sleep(100);
	}
}

void CThread::OnClose()
{
	m_hThread = NULL;
}

//////////////////////////////////
// -CServer

void CServer::EnterCriticalSection()
{
	// Some data can be shared amoung multiple tasks.
	// Protection against multiple access.
}

void CServer::LeaveCriticalSection()
{

}

///////////////////////////////////////////////////////////////////
// -CBackTask

bool CBackTask::RegisterServer()
{
	// NOTE: This is a synchronous function that could take a while.
	// Register myself with the central login server.
	bool fFirst = ( m_dwNextRegisterTime == 0 || ! GetRandVal( 20 ));
	m_dwNextRegisterTime = g_World.GetTime() + 60*60*TICK_PER_SEC;

	// don't register to myself.
	if ( g_Serv.m_fMainLogServer ) 
		return( false );

	// Find the address for the main login server and register.
	const TCHAR * pszName = g_Serv.m_sRegisterServer;
	if ( pszName[0] == '\0' || pszName[0] == '0' )
		return false;

	if ( fFirst || ! m_RegisterIP.GetAddr())
	{
		bool fRet = m_RegisterIP.SetHostPortStr( pszName );	// GRAY_MAIN_SERVER
		if ( ! fRet )	// can't resolve the address.
		{
			// Might just try the adddress we already have ?
			m_RegisterIP.SetAddr( 0 );
			return false;
		}
	}

	// Tell the registration server that we exist.

	CGSocket sock;
	if ( ! sock.Create())
	{
		return( false );
	}
	if ( sock.Connect( m_RegisterIP ))
	{
		m_RegisterIP.SetAddr( 0 );	// try to look it up again later.
		return false;
	}

	// ? need to wait a certain amount of time for the connection to setle out here.

	// got a connection. Send reg command.
	TCHAR szData[ MAX_SCRIPT_LINE_LEN ];
	memset( szData, 0xFF, 4 );	// special registration header.
	szData[4] = 0;
	int iLen = strcpylen( &szData[5], g_Serv.GetStatusStringRegister(fFirst), sizeof( szData )-6);
	iLen += 5;
	int iLenRet = sock.Send( szData, iLen );
	if ( iLenRet != iLen )
	{
		// WSAENOTSOCK 
		DEBUG_ERR(( "%d, can't send to reg server '%s'\n", sock.GetLastError(), pszName ));
		return false;
	}

	// reset the max clients.
	g_Serv.ClientsResetAvg();
	return( true );
}

void CBackTask::PollServers()
{
	m_dwNextPollTime = g_World.GetTime() + g_Serv.m_iPollServers;

	int iTotalClients = 0;
	for ( int i=0; i<g_Serv.m_Servers.GetCount() && ! g_Serv.m_wExitFlag; i++ )
	{
		CServRef * pServ = g_Serv.m_Servers[i];
		if ( pServ == NULL )
			continue;

		if ( ! g_Serv.m_fMainLogServer )	// the main server does not do this because too many people use dial ups .
		{
			if ( pServ->PollStatus())
				continue;
		}
		if ( ! pServ->IsValidStatus())
		{
			g_Serv.m_Servers.RemoveAt(i);
			i--;
		}
		else
		{
			if ( pServ->GetTimeSinceLastValid() > (TICK_PER_SEC*60*60*24) )
				continue;
			iTotalClients += pServ->GetClientsAvg();
		}
	}

	m_iTotalPolledClients = iTotalClients;
}

void CBackTask::SendOutgoingMail()
{
	// Attempt to send the outgoing mail now.

	m_dwNextMailTime = g_World.GetTime() + 60 * TICK_PER_SEC;

	// look thru all the accounts for outgoing mail.
	// ??? This is SERIOSLY asking for multi-thread problems !

	for ( int i=0; i<g_World.m_Accounts.GetCount(); i ++ )
	{
		CAccount * pAccount = g_World.m_Accounts[i];
		ASSERT(pAccount);
		pAccount->SendAllOutgoingMail();
	}
}

void _cdecl CBackTask::EntryProc( void * lpThreadParameter ) // static
{
	// Do all the background stuff we don't want to bog down the game server thread.

	g_Serv.m_BackTask.OnCreate();
	while ( g_Serv.CanRunBackTask())
	{
		try
		{
			// register the server every few hours or so.
			if ( ! g_Serv.m_sRegisterServer.IsEmpty() &&
				g_World.GetTime() >= g_Serv.m_BackTask.m_dwNextRegisterTime )
			{
				g_Serv.m_BackTask.RegisterServer();
			}

			// ping the monitored servers.
			if ( g_Serv.m_iPollServers &&
				g_World.GetTime() >= g_Serv.m_BackTask.m_dwNextPollTime )
			{
				g_Serv.m_BackTask.PollServers();
			}

			// Check for outgoing mail.
			if ( g_World.GetTime() >= g_Serv.m_BackTask.m_dwNextMailTime )
			{
				g_Serv.m_BackTask.SendOutgoingMail();
			}
		}

		catch (...)	// catch all
		{
			DEBUG_ERR(( "BackTask FAULT\n" ));
		}

		// ? watch for incoming connections and data ???.
		Sleep( 15 * 1000 );
	}

	// Tell the world we are closed. CloseHandle() required ?
	g_Serv.m_BackTask.OnClose();
}

void CBackTask::CreateThread()
{
	// beginthread() and endthread() are the portable versions of this !

	if ( ! g_Serv.CanRunBackTask())
	{
		// don't bother with the background stuff.
		return;
	}
	CThread::CreateThread( EntryProc );
}

