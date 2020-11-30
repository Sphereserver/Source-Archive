//
// CClientLog.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Login and low level stuff for the client.
//

#include "graysvr.h"	// predef header.

BYTE CClient::xCompress_Buffer[MAX_BUFFER];	// static
CCompressTree CClient::sm_xComp;

/////////////////////////////////////////////////////////////////
// -CClient stuff.

int CClient::xDeCompress( BYTE * pOutput, const BYTE * pInput, int inplen ) // static
{
	if ( ! sm_xComp.IsLoaded())
	{
		if ( ! sm_xComp.Load()) 
			return( -1 );
	}
	return( sm_xComp.Decode( pOutput, pInput, inplen ));
}

int CClient::xCompress( BYTE * pOutput, const BYTE * pInput, int inplen ) // static
{
	// The game server will compress the outgoing data to the clients.
	return( sm_xComp.Encode( pOutput, pInput, inplen ));
}

void CClient::xFlush() // Sends buffered data at once
{
	if ( m_bout_len <= 0 ) 
		return;
	if ( ! m_fGameServer )	// acting as a login server to this client.
	{
		Send( m_bout.m_Raw, m_bout_len );
		m_bout_len=0;
		return;
	}

#ifdef GRAY_GAME_SERVER
	// Only the game server does this.
	// This acts as a compression alg.

	int len = xCompress( xCompress_Buffer, m_bout.m_Raw, m_bout_len );
	ASSERT( len <= sizeof(xCompress_Buffer));

	// DEBUG_MSG(( "%x:Send %d bytes as %d\n", GetSocket(), m_bout_len, len ));

	Send( xCompress_Buffer, len );
	m_bout_len=0;
#endif

}

void CClient::xSend( const void *pData, int length )
{
	// buffer a packet to client.
	if ( length > MAX_BUFFER )
	{
		DEBUG_ERR(( "%x:Client out TO BIG %d!\n", GetSocket(), length ));
		ASSERT(0);
		return;
	}
	if ( m_bout_len + length > MAX_BUFFER )
	{
		DEBUG_ERR(( "%x:Client out overflow %d+%d!\n", GetSocket(), m_bout_len, length ));
		ASSERT(0);
		return;
	}
	ASSERT(length);
	memcpy( & ( m_bout.m_Raw[m_bout_len] ), pData, length );
	m_bout_len += length;
}

void CClient::xSendReady( const void *pData, int length ) // We could send the packet now if we wanted to but wait til we have more.
{
	// We could send the packet now if we wanted to but wait til we have more.
	if ( m_bout_len + length >= MAX_BUFFER )
	{
		xFlush();
	}
	xSend( pData, length );
	if ( m_bout_len >= MAX_BUFFER / 2 )	// send only if we have a bunch.
	{
		xFlush();
	}
}

bool CClient::xCheckSize( int len )
{
	// Is there enough data from client to process this packet ?
	if ( ! len || len > sizeof( CEvent ))
		return( false );	// junk
	m_bin_pkt = len;
	return( m_bin_len >= len );
}

void CClient::xProcess( bool fGood )
{
	// Done with the current packet.
	// m_bin_pkt = size of the current packet we are processing.

	if ( ! fGood || ! m_bin_pkt )	// toss all.
	{
		DEBUG_ERR(( "%x:Bad Msg 0%x Eat %d bytes, prv=0%x\n", GetSocket(), m_bin.Default.m_Cmd, m_bin_len, m_bin_PrvMsg ));
		m_bin_len = 0;
		if ( ! m_fGameServer )	// tell them about it.
		{
			addLoginErr( LOGIN_ERR_OTHER );
		}
	}
	else
	{
		ASSERT( m_bin_len >= m_bin_pkt );
		memmove( m_bin.m_Raw, &(m_bin.m_Raw[m_bin_pkt]), m_bin_len-m_bin_pkt);
		m_bin_len -= m_bin_pkt;
	}
	m_bin_pkt = 0;
}

#ifdef COMMENT

void CClient::xInit_DeCrypt_FindKey( const BYTE * pCryptData, int len )
{
	// Given an encrypted buffer. pCryptData
	// Compare it back against what we expected to see.
	// Use this to derive the MasterKey

	// use m_tmSetupConnect for index into masks.
	int n = m_tmSetupConnect;
	if ( n > 64 )
	{
		ASSERT(0);	// done.
		return;
	}

	// CalibrateData should need to be only 64 bytes ?
	// This is the login packet + 2 bytes from the spy packet.
	// Login the first time with admin + password same = predictable data.
	// Assume 2 bytes from spy packet will also be the same ?
	static BYTE CalibrateData[64] =
	{
		0x80, 'A',  'D',  'M',  'I',  'N',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'P',
		'A',  'S',  'S',  'W',  'O',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
		0xa4, 0x03	// 2 bytes from the spy packet.
	};

	// What key would it take to derive the expected data.
	// for 127.0.0.1 = m_RxCryptMaskLo=0xeca9aaab, m_RxCryptMaskHi=0x2b323c21

	static DWORD dwMasterKeyLo = 0;
	static DWORD dwMasterKeyHi = 0;

	for ( int i=0; i<len && n < 64; i++, n++ )
	{
		//m_bin.m_Raw[i] ^= m_RxCryptMaskLo;
		//DWORD MaskLo = m_RxCryptMaskLo;
		//DWORD MaskHi = m_RxCryptMaskHi;
		//m_RxCryptMaskHi = ((MaskHi >> 1) | (MaskLo << 31)) ^ MASTERKEY_HI;
		//m_RxCryptMaskLo = ((MaskLo >> 1) | (MaskHi << 31)) ^ MASTERKEY_LO;

		if ( n & 7 ) continue;

		BYTE bVal = m_bin.m_Raw[i] ^ CalibrateData[ m_tmSetupConnect ];

		if ( n == 0 )
		{
			ASSERT( bVal == 0xab );
		}

		if ( n<32 )
			dwMasterKeyLo |= bVal << (n);
		else
			dwMasterKeyHi |= bVal << (n-32);
	}

	m_tmSetupConnect = n;
	if ( n < 64 ) 
		return;	// need more.
}

#endif

bool CClient::IsBlockedIP() const
{
	struct sockaddr_in Name;
	if ( GetPeerName( &Name ))	// can't get ip ? why ?
		return( true );
	return( g_Serv.CheckLogIPBlocked( Name.sin_addr, false ));
}

bool CClient::OnAutoServerRegisterRx( BYTE * pData, int len )
{
	// RETURN: false = dump the client.

	if ( len <= 1  || len > MAX_SCRIPT_LINE_LEN ) 
		return( false );	// bad format.
	pData[len] = '\0';
	pData ++;

	// Server registration message.
	// Check to see if this server is already here.
	struct sockaddr_in Name;
	if ( GetPeerName( &Name ))
		return( false );

	// Create a new entry for this. (so we can get it's name etc.)
	CServRef * pServNew = new CServRef( NULL, Name.sin_addr.s_addr );
	ASSERT( pServNew );
	pServNew->ParseStatus( (char*)pData, true );

	if ( pServNew->GetName()[0] == '\0' )
	{
		// There is no name here ?
		// Might we try to match by IP address ?

		{
		for ( int i=0; i<g_Serv.m_Servers.GetCount(); i++ )
		{
			CServRef * pServTest = g_Serv.m_Servers[i];
			if ( pServTest == NULL ) 
				continue;
			if ( Name.sin_addr.s_addr != pServTest->m_ip.GetAddr())
				continue;
			pServTest->ParseStatus( (char*)pData, true );
			goto bailout;
		}
		}

		g_Log.Event( LOGL_WARN, "%x:Bad reg info, No server name '%s'\n", GetSocket(), (char*)pData ); 
bailout:
		delete pServNew;
		return( false );
	}

	// Look up it's name.
	int index = g_Serv.m_Servers.FindKey( pServNew->GetName());
	if ( index < 0 )
	{
		// only the main login server will add automatically.
		if ( ! g_Serv.m_fMainLogServer ) 
			return( false );

		// No server by this name is here. So add it.
		g_Serv.m_Servers.AddSortKey( pServNew, pServNew->GetName());
		return( false );
	}

	CServRef * pServName = g_Serv.m_Servers[index];
	ASSERT( pServName );
	if ( ! pServName->IsSame( pServNew ) && 
		Name.sin_addr.s_addr != pServName->m_ip.GetAddr())
	{
		g_Log.Event( LOGL_WARN, "%x:Bad regpass '%s'!='%s' for server '%s'\n", GetSocket(), (const TCHAR*) pServNew->m_sRegisterPassword, (const TCHAR*) pServName->m_sRegisterPassword, pServName->GetName());
		goto bailout;
	}

	// It looks like our IP just changed thats all. RegPass checks out. 
	pServName->ParseStatus( (char*)pData, true );
	return( false );
}

bool CClient::OnConsoleLogin()
{
	ASSERT( GetTargMode() == TARGMODE_CONSOLE_LOGIN );

	CGString sMsg;
	if ( LogIn( _TEXT("RemoteAdmin"), m_Targ_Text, sMsg ) != LOGIN_SUCCESS )
	{
		if ( LogIn( _TEXT("Administrator"), m_Targ_Text, sMsg ) != LOGIN_SUCCESS )
		{
			if ( ! sMsg.IsEmpty())
			{
				SysMessage( sMsg );
				SysMessage( "\n" );
			}
			return( false );
		}
	}

	if ( GetPrivLevel() < PLEVEL_Admin )
	{
		SysMessage( "Sorry you don't have admin level access\n" );
		return( false );
	}

	struct sockaddr_in Name;
	if ( GetPeerName( &Name ))
		return( false );

	g_Serv.m_iAdminClients++;
	m_Targ_Mode = TARGMODE_CONSOLE;
	m_Targ_Text.Empty();

	SysMessagef( "Welcome to the Remote Admin Console '%s','%s'\n", GetName(), inet_ntoa( Name.sin_addr ) );
	return( true );
}

bool CClient::OnConsoleRx( BYTE * pData, int len )
{
	// A special console version of the client. (Not game protocol)
	ASSERT( len );
	ASSERT( IsConsole());

	while ( len -- )
	{
		int iRet = OnConsoleKey( m_Targ_Text, *pData++, GetTargMode() == TARGMODE_CONSOLE );
		if ( ! iRet )
			return( false );
		if ( iRet == 2 )
		{
			if ( GetTargMode() == TARGMODE_CONSOLE_LOGIN )
			{
				iRet = OnConsoleLogin();
			}
			else
			{
				ASSERT( GetTargMode() == TARGMODE_CONSOLE );
				iRet = g_Serv.OnConsoleCmd( m_Targ_Text, this );
			}
			if ( ! iRet ) 
				return( false );
		}
	}

	return( true );
}

bool CClient::OnPingRx( BYTE * pData, int len )
{
	// UOMon should work like this.

	ASSERT( len < 4 );
	if ( len > 1 )	// this is not a ping. don't know what it is.
		return( false );

	if ( pData[0] == ' ' )
	{
		// enter into remote admin mode. (look for password).
		m_Targ_Mode = TARGMODE_CONSOLE_LOGIN;
		SysMessagef( "Welcome to %s Admin Telnet\n", g_Serv.GetName());
		SysMessage( "Password?:\n" );
		return( true );
	}

	if ( pData[0] == '?' )
	{
		// Only our central login server or a peer server may do this !
		// g_Serv.m_BackTask.m_RegisterIP or g_Serv.m_Servers
		// Poll us for data.
		m_Targ_Mode = TARGMODE_PEER_SERVER_REQUEST;
		const char * pszMsg = GRAY_TITLE "\n";
		xSendReady( pszMsg, strlen( pszMsg )+1 );
		return( true );
	}

	// Answer the ping and drop.
	const char * pTemp = g_Serv.GetStatusString( pData[0] );
	xSendReady( pTemp, strlen( pTemp )+1 );
	return( false );
}

bool CClient::xRecvData() // Receive message from client
{
	// High level Rx from Client.
	// RETURN: false = dump the client.

	int iPrev = m_bin_len;
	int count = Receive( &(m_bin.m_Raw[iPrev]), MAX_BUFFER - iPrev );
	if ( count <= 0 )	// I should always get data here.
	{
		return( false ); // this means that the client is gone.
	}
	m_bin_len += count;

	if ( ! m_Crypt.IsInit())
	{
		// Must process the whole thing as one packet right now.
		ASSERT( iPrev == 0 );
		ASSERT( count == m_bin_len );

		if ( m_bin_len > 5 && m_bin.m_CryptHeader == 0xFFFFFFFF )
		{
			// special inter-server type message.
			if ( IsBlockedIP())
				return( false );

			if ( m_bin.m_Raw[4] == 0 )
			{
				// Server Registration message
				bool fRet = OnAutoServerRegisterRx( &m_bin.m_Raw[4], m_bin_len-4 );
				m_bin_len = 0;	// eat the buffer.
				return( fRet );
			}
		}

#ifdef GRAY_GAME_SERVER

		if ( GetTargMode() == TARGMODE_PEER_SERVER_REQUEST )
		{
			// Answer a question for this peer server.
			bool fSuccess = false;
			if ( m_bin_len <= 128 )
			{
				m_bin.m_Raw[m_bin_len] = '\0';
				TCHAR * pszCmd = TrimWhitespace( (char*) m_bin.m_Raw );
				CGString sVal;
				fSuccess = g_Serv.r_WriteVal( pszCmd, sVal, this );
				if ( fSuccess )
				{
					xSendReady( sVal, sVal.GetLength()+1 );
				}
			}
			m_bin_len = 0;	// eat the buffer.
			return( true );	// don't dump the connection.
		}

		if ( IsConsole())
		{
			// We already logged in or are in the process of logging in.
			bool fRet = OnConsoleRx( m_bin.m_Raw, m_bin_len );
			m_bin_len = 0;	// eat the buffer.
			return( fRet );
		}

		if ( m_bin_len < 4 )	// just a ping for server info.
		{
			if ( IsBlockedIP())
				return( false );
			bool fRet = OnPingRx( m_bin.m_Raw, m_bin_len );
			m_bin_len = 0;	// eat the buffer.
			return( fRet );
		}
#endif
		// Assume it's a normal client log in.
		xCheckSize( sizeof( m_bin.m_CryptHeader ));

		// DEBUG_MSG(( "%x:CCrypt:Init %d.%d.%d.%d\n", GetSocket(), pDeCryptID[0], pDeCryptID[1], pDeCryptID[2], pDeCryptID[3] ));
		bool fGame = false;
		if ( count == 66 ) // SERVER_Login 1.26.0
		{
			m_Crypt.Init( m_bin.m_Raw, SERVER_Login ); // Init decryption table
		}
		else if ( count == 69 )	// Auto-registering server sending us info.
		{
			m_Crypt.Init( m_bin.m_Raw, SERVER_Game ); // Init decryption table
			fGame = true;
		}
		else	// probably this is a login server.
		{
			m_Crypt.Init( m_bin.m_Raw ); // Init decryption table
		}

		if ( IsBlockedIP())
		{
			addLoginErr( LOGIN_ERR_BLOCKED );
			return( false );
		}

		xProcess( true );
	}

	// Decrypt the data.
	// TCP = no missed packets ! If we miss a packet we are screwed !

	// g_Log.Event( LOGL_TRACE, "Before\n" );
	// g_Log.Dump( m_bin.m_Raw, m_bin_len );

	m_Crypt.Decrypt( m_bin.m_Raw+iPrev, m_bin.m_Raw+iPrev, m_bin_len-iPrev );

	// g_Log.Event( LOGL_TRACE, "After\n" );
	// g_Log.Dump( m_bin.m_Raw, m_bin_len );

	return( true );
}

//---------------------------------------------------------------------
// Push world display data to this client only.

bool CClient::addLoginErr( LOGIN_ERR_TYPE code )
{
	// code
	// 0 = no account
	// 1 = account used.
	// 2 = blocked.
	// 3 = no password
	// LOGIN_ERR_OTHER

	if ( code == LOGIN_SUCCESS ) 
		return true;

	DEBUG_ERR(( "%x:Bad Login %d\n", GetSocket(), code ));

	CCommand cmd;
	cmd.LogBad.m_Cmd = XCMD_LogBad;
	cmd.LogBad.m_code = code;
	xSendPkt( &cmd, sizeof( cmd.LogBad ));
	xFlush();
	return( false );
}

void CClient::addSysMessage( const char *pMsg) // System message (In lower left corner)
{
	addBark( pMsg, NULL, COLOR_TEXT_DEF, TALKMODE_SYSTEM, FONT_NORMAL );
}

void CClient::addWebLaunch( const char *pPage )
{
	// Direct client to a web page
	addSysMessage( "Launching your web browser. Please wait...");

	CCommand cmd;
	cmd.Web.m_Cmd = XCMD_Web;
	int len = sizeof(cmd.Web) + strlen(pPage);
	cmd.Web.m_len = len;
	strcpy( cmd.Web.m_page, pPage );
	xSendPkt( &cmd, len );
}

///////////////////////////////////////////////////////////////
// Login server.

bool CClient::Login_Relay( int iRelay ) // Relay player to a selected IP
{
	// DEBUG_MSG(( "%x:Login_Relay\n", GetSocket() ));
	// iRelay = 0 = this local server.

	if ( GetTargMode() != TARGMODE_SETUP_SERVERS )
		return false;

	if ( m_Crypt.GetClientVersion() >= 12600 )
	{
		// Must be 1 based index for some reason.
		iRelay --;
	}

	CServRef * pServ;
	if ( iRelay <= 0 )
	{
		pServ = &g_Serv;
	}
	else
	{
		iRelay --;
		if ( ! g_Serv.m_Servers.IsValidIndex( iRelay ))
		{
			DEBUG_ERR(( "%x:Login_Relay BAD index! %d\n", GetSocket(), iRelay ));
			return( false );
		}
		pServ = g_Serv.m_Servers[ iRelay ];
	}

	DWORD dwAddr = pServ->m_ip.GetAddr();

	if ( dwAddr == 0 || dwAddr == SOCKET_LOCAL_ADDRESS )	// local server address not yet filled in.
	{
		struct sockaddr_in Name;
		GetSockName( &Name );
		dwAddr = Name.sin_addr.s_addr;
		DEBUG_MSG(( "%x:Login_Relay to %s\n", GetSocket(), inet_ntoa( Name.sin_addr ) ));
	}

	struct sockaddr_in PeerName;
	GetPeerName( &PeerName );
	if ( PeerName.sin_addr.s_addr == dwAddr )	// weird problem with client relaying back to self.
	{
		DEBUG_MSG(( "%x:Login_Relay loopback to server %s\n", GetSocket(), inet_ntoa( PeerName.sin_addr ) ));
		dwAddr = SOCKET_LOCAL_ADDRESS;
	}

	CCommand cmd;
	cmd.Relay.m_Cmd = XCMD_Relay;
	cmd.Relay.m_ip[3] = ( dwAddr >> 24 ) & 0xFF;
	cmd.Relay.m_ip[2] = ( dwAddr >> 16 ) & 0xFF;
	cmd.Relay.m_ip[1] = ( dwAddr >> 8  ) & 0xFF;
	cmd.Relay.m_ip[0] = ( dwAddr	   ) & 0xFF;
	cmd.Relay.m_port = pServ->m_ip.GetPort();
	cmd.Relay.m_Account = 0x7f000001; // customer account. (don't bother to check this.)

	xSendPkt( &cmd, sizeof(cmd.Relay));

	m_Targ_Mode = TARGMODE_SETUP_RELAY;

	// just in case they are on the same machine, change over to the new game encrypt
	m_Crypt.Init( cmd.Relay.m_ip, SERVER_Game ); // Init decryption table
	return( true );
}

void CClient::Login_ServerList( char * pszAccount, char * pszPassword ) // Initial login (Login on "loginserver", new format)
{
	// If the messages are garbled make sure they are terminated to correct length.
	pszAccount[ sizeof( m_bin.ServersReq.m_name ) -1 ] = '\0';
	pszPassword[ sizeof( m_bin.ServersReq.m_password ) -1 ] = '\0';

	// Make sure the first server matches the GetSockName here
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Login_ServerList to '%s','%s'\n", GetSocket(), pszAccount, pszPassword ));
	}

	// don't bother logging in yet.
	// Give the server list to everyone.
	// if ( ! addLoginErr( LogIn( pszAccount, pszPassword )))
	// { return; }

	CCommand cmd;
	cmd.ServerList.m_Cmd = XCMD_ServerList;

	int indexoffset = 1;
	if ( m_Crypt.GetClientVersion() >= 12600 )
	{
		indexoffset = 2;
	}

	// always list myself first here.
	g_Serv.addToServersList( cmd, indexoffset-1, 0 );

	int j = 1;
	for ( int i=0; i<g_Serv.m_Servers.GetCount() && j < MAX_SERVERS; i++ )
	{
		CServRef * pServ = g_Serv.m_Servers[i];
		if ( pServ == NULL )
			continue;
		if ( g_Serv.m_iPollServers && ! pServ->IsConnected())
			continue;
		pServ->addToServersList( cmd, i+indexoffset, j );
		j++;
	}

	int len = sizeof(cmd.ServerList) - sizeof(cmd.ServerList.m_serv) + ( j * sizeof(cmd.ServerList.m_serv[0]));
	cmd.ServerList.m_len = len;
	cmd.ServerList.m_count = j;
	cmd.ServerList.m_unk3 = 0xFF;
	xSendPkt( &cmd, len );

	m_Targ_Mode = TARGMODE_SETUP_SERVERS;
}

