//
// GrayMon.CPP
// Stand alone app that is a remote monitor for GraySvr
// http://www.ayekan.com/awesomedev
// or see my page: http://www.menasoft.com for more details.

#include "..\graysvr\graycom.h"

#define GRAY_TITLE		"GrayMon"
#define GRAY_VERSION	"0.01" 

struct CMonitor : public CSocket
{
public:
	int m_bin_count;
	char m_bin[1024];

	const char * m_szAddress;
	const char * m_szPassword;

public:
	bool SocketsInit();
	void SocketsClose();
	bool SocketsConnect();
	bool SocketsReceive(); // Check for messages from the clients
};

static CMonitor Mon;

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

/////////////////////////////////////////////////////////////////
// -CMonitor

bool CMonitor :: SocketsInit()
{
#ifdef _WIN32
	WSADATA wsaData;
	WORD wVersionRequested;

	wVersionRequested=0x0101;
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err)
	{
		printf("ERROR: Winsock 1.1 not found...\n");
		return( false );
	}
#endif

	if ( ! Create())
	{
		printf("ERROR: Unable to create socket\n");
		return( false );
	}

#ifndef _WIN32
	int on=1;
	int off=0;
	setsockopt(GetSocket(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

#ifdef COMMENT
	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = INADDR_ANY;
	connection.sin_port = htons( GRAY_PORT );
	int bcode = Bind( &connection );
	if (bcode<0)
	{
		printf( "ERROR: Unable to bind socket 1 - Error code: %i\n", bcode );
		return( false );
	}
#endif

	return( true );
}

void CMonitor :: SocketsClose()
{

}

bool CMonitor :: SocketsConnect()
{
	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = inet_addr( m_szAddress );
	connection.sin_port = htons( GRAY_PORT );

	int iRet = Connect( &connection );
	if ( iRet ) 
	{
		printf( "ERROR: code %d while connecting to server\n", iRet );
		return( false );
	}

	// Now log in
	struct fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(GetSocket(), &writefds);
	int nfds = GetSocket();

	timeval Timeout;	// time to wait for data.	
	Timeout.tv_sec=30;
	Timeout.tv_usec=200;	
	iRet = select( nfds+1, NULL, &writefds, NULL, &Timeout );
	if ( iRet <= 0 )
	{
		printf( "ERROR: timeout while connecting to server\n" );
		return( false );
	}
	if ( ! FD_ISSET( GetSocket(), &writefds)) 
	{
		printf( "ERROR: timeout2 while connecting to server\n" );
		return( false );
	}

	m_bin[0] = 0xFF;
	m_bin[1] = 0xFF;
	m_bin[2] = 0xFF;
	m_bin[3] = 0xFF;
	int count = 4;
	strcpy( m_bin+count, m_szPassword );
	count += strlen( m_szPassword ) + 1;
	Send( m_bin, count );

	printf( "server connected\n" );
	return( true );
}

bool CMonitor :: SocketsReceive() // Check for messages from the clients
{
	// What sockets do I want to look at ?
	struct fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(GetSocket(), &readfds);
	int nfds = GetSocket();

	timeval Timeout;	// time to wait for data.	
	Timeout.tv_sec=0;
	Timeout.tv_usec=200;	
	int ret = select( nfds+1, &readfds, NULL, NULL, &Timeout );
	if ( ret <= 0 )
	{
		return( true );
	}

	// Any new data ?
	if ( FD_ISSET( GetSocket(), &readfds)) 
	{
		m_bin_count = Receive( m_bin, sizeof( m_bin ));
		if ( m_bin_count <= 0 )	// I should always get data here.
		{
			return( false );
		}
		m_bin[ m_bin_count ] = '\0';
		printf( m_bin );
	}
	return( true );
}

int main( int argc, char *argv[] )
{
#ifdef _WIN32
	SetConsoleTitle( GRAY_TITLE " V" GRAY_VERSION );
#endif
	printf( GRAY_TITLE " V" GRAY_VERSION 
#ifdef _WIN32
		" for Win32\n"
#else
		" for Linux\n"
#endif
		"Client Version: " GRAY_CLIENT_STR "\n"
		"Compiled at " __DATE__ " (" __TIME__ " " GRAY_TIMEZONE ")\n"
		"Compiled by " GRAY_NAME " <" GRAY_EMAIL ">\n"
		"\n" );

	Mon.m_szAddress  = "24.128.106.85";
	Mon.m_szPassword = "password";	// "password";	
	if ( argc >= 2 ) 
	{
		Mon.m_szAddress  = argv[1];
	}
	if ( argc >= 3 ) 
	{
		Mon.m_szPassword = argv[2];
	}

	if ( ! Mon.SocketsInit())
	{
		printf( "Failed to Initialize\n" );
		printf( "Press a key to exit\n" );
		getch();
		return( -1 );
	}

	if ( ! Mon.SocketsConnect())
	{
		printf( "Failed to connect\n" );
		printf( "Press a key to exit\n" );
		getch();
		return( -1 );
	}

	while ( 1 )
	{
		if ( kbhit())
		{
			// Send the char over to the server.
			char ch = getch();
			if ( ch == 0x1b ) break;	// ESCAPE KEY
			Mon.Send( &ch, 1 );
		}

		// Look for incoming data.
		if ( ! Mon.SocketsReceive())
			break;
	}

	Mon.SocketsClose();

	printf( "Server disconnect\n" );
	printf( "Press a key to exit\n" );
	getch();
	return( 0 );
}

