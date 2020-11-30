//
// CMail.cpp
// Copyright Menace Software (www.menasoft.com).
// EMail interface.
//

#include "graysvr.h"	// predef header.


/////////////////////////////////////////////////////////////////////////////
// CMailSMTP

bool CMailSMTP::SendLine( const TCHAR * pszMessage, int iResponseExpect )
{
    // allocate enough space for the command plus enough to add a \n char
	CGString sMsg;
	sMsg.Format( "%s\r\n", pszMessage);

	m_sResponse.Empty();

    // Send the message
    if ( Send( sMsg, sMsg.GetLength()) == SOCKET_ERROR )
		return( false );

	if ( iResponseExpect < 0 )
		return( true );
	// if there wasn't an error get the response;
    return ReceiveResponse();
}

bool CMailSMTP::ReceiveResponse()
{
    // I only care about the numeric value
	TCHAR szResponse[ 1024 ];
    int numReceived=Receive(szResponse,sizeof(szResponse)-1);
    if ( numReceived >= sizeof(szResponse)-1)
    {
        // I have more data to get,  I will want to flush the rest,
        // but I don't really need it.
    }
    else
    {
        // null terminate the string.
        szResponse[numReceived] = '\0';
		m_sResponse = szResponse;
    }

    switch(szResponse[0])
    {
    case '1':       // not possible, but code for anyways
    case '2':       // positive response
    case '3':       // intermediate response, but go ahead and continue
        return true;
    case '4':
    case '5':
        return false;
    default:
        // if anything other than the above happen, assume a failure
        return false;
    }

	return false;
}

bool CMailSMTP::OpenConnection(const TCHAR * szMailServerIPAddress)
{
    // Create a socket
    bool fStatus=Create();
    if( ! fStatus)
		return( false );

    // connect to the SMTP Service (port 25)
    int iStatus=Connect(szMailServerIPAddress,PORT_SMTP);
    if( iStatus )
		return( false );

    // Check response from the SMTP Service
    fStatus=ReceiveResponse();
    return fStatus;
}

bool CMailSMTP::SendMail( const TCHAR * pszDstMail, const TCHAR * pszSrcMail, CGString * ppszBody, int iBodyLines )
{
	// See RFC 821 for more on this.
	// mail on port 25

	const TCHAR * pszAddr = strrchr( pszDstMail, '@' );
	if ( pszAddr == NULL )	// bad form of address.
		return( false );

	bool fIsOK = OpenConnection( pszAddr+1 );  // open a connection to a mail server
	if ( ! fIsOK )
	{
		return( false );
	}

	static const TCHAR *Lines[]=
	{
		"HELO %s",	// NEEDS domain ARG ?
        "MAIL FROM: <%s>",
        "RCPT TO: <%s>",
        "DATA",
		NULL,
	};

	for ( int i=0;Lines[i] != NULL;i++)
	{
		CGString sMsg;
		switch ( i )
		{
		case 0:	sMsg.Format( Lines[0], g_Serv.m_ip.GetAddrStr()); break;
		case 1: sMsg.Format( Lines[1], pszSrcMail ); break;
		case 2: sMsg.Format( Lines[2], pszDstMail ); break;
		default:
			sMsg = Lines[i];
			break;
		}
		if ( ! SendLine(sMsg))
		{
			return( false );
		}
	}

    //S: Date: 2 Nov 81 22:33:44
    //S: From: John Q. Public <JQP@MIT-AI.ARPA>
    //S: Subject:  The Next Meeting of the Board
    //S: To: Jones@BBN-Vax.ARPA
            
	for (int i=0; iBodyLines--; i++ )
	{
		const TCHAR * pszLine = ppszBody[i];
		if ( pszLine[0] == '.' && 
			pszLine[1] == '\r' )
		{
			pszLine = "..";
		}
		if ( ! SendLine( pszLine, -1 ))
		{
			return( false );
		}
	}
	if ( ! SendLine("."))	// end of the msg.
	{
		return( false );
	}
	if ( ! SendLine("QUIT"))
	{
		return( false );
	}

	// check for an error
	if ( GetLastResponse()[0] != '2' )
	{
		// have an error
		return( false );
	}

	return( true );
}

#if 0

////////////////
// CMailPOP = mail receiver.

class CMailPOP : protected CSocket
{
	// Mail receiver.
public:
	CObArray <CSocket*> m_MailClients;
public:
	bool Init();
	bool Check();
};

bool CMailPOP::Init()
{
	// CSocket
	if ( ! Create())
	{
		// g_Log.Event( LOGL_ERROR, "Unable to create socket\n");
		return( false );
	}

	if ( ! Bind( PORT_SMTP ))
	{
		// Probably already a server running.
		// g_Log.Event( LOGL_ERROR, "Unable to bind listen socket port %d - Error code: %i\n", m_ip.GetPort(), bcode );
		// HRESULT hRes = GetLastError();

		return( false );
	}

	return( Listen());
}

bool CMailPOP::Check()
{
	// Check for connections on the mail port

	struct fd_set readfds;
	FD_ZERO(&readfds);

	FD_SET(GetSocket(), &readfds);
	int nfds = GetSocket();

	for ( int i=0; i<m_MailClients.GetCount(); i++ )
	{
		SOCKET hSocket = m_MailClients[i]->GetSocket();
		FD_SET( hSocket,&readfds);
		if ( hSocket > nfds ) nfds = hSocket;
	}

	timeval Timeout;	// time to wait for data.
	Timeout.tv_sec=0;
	Timeout.tv_usec=100;	// micro seconds = 1/1000000
	int ret = select( nfds+1, &readfds, NULL, NULL, &Timeout );
	if ( ret <= 0 ) return false;

	// Any events from clients ?
	for ( i=0; i<m_MailClients.GetCount(); i++ )
	{
		SOCKET hSocket = m_MailClients[i]->GetSocket();
		if ( FD_ISSET( hSocket, &readfds ))
		{
			TCHAR szData[ 1024 ];
			int count = m_MailClients[i]->Receive( szData, sizeof( szData ));
			if ( count <= 0 )	// I should always get data here.
			{
				// The client broke the connection.
				continue;
			}

			// process the data.
			ret = 123;
		}
		else
		{
			// Check dead socket time ?
		}
	}

	// Any new connections ? what if there are several ???
	if ( FD_ISSET( GetSocket(), &readfds))
	{
		int len = sizeof( struct sockaddr );
		struct sockaddr client_addr;
		SOCKET hSocketClient = accept( GetSocket(), &client_addr, &len );
		if ( hSocketClient < 0 || hSocketClient == INVALID_SOCKET )	// LINUX case is signed ?
		{
			// g_Log.Event( LOGL_ERROR, "Error at client connection!\n");
			return false ;
		}

		CSocket * pNewSocket = new CSocket( hSocketClient );
		//BOOL fSuccess = pNewSocket->Attach( hSocketClient );

		// Create the new client.
		m_MailClients.Add( pNewSocket );
	}

	return( true );
}

#endif
