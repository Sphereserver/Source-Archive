//
// graycom.h
// Copyright Menace Software (www.menasoft.com).
// common header file.
//

#ifndef _INC_GRAYCOM_H
#define _INC_GRAYCOM_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#define _DEBUG	// compile a debug version with more verbose comments
#define GRAY_DEF_PORT	2593

#define GRAY_FILE		"sphere"	// file name prefix
#define GRAY_SCRIPT		".scp"
#define GRAY_BINARY		".bin"

//---------------------------SYSTEM DEFINITIONS---------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#ifdef _WIN32
#include <io.h>
#else // _WIN32
#include <sys/io.h>
#endif // ! _WIN32

#ifdef _WIN32
#ifndef STRICT
#define STRICT			// strict conversion of handles and pointers.
#endif	// STRICT

#include <windows.h>
#include <winsock.h>
#include <dos.h>
#include <limits.h>
#include <conio.h>
#include <sys/timeb.h>

#define strcmpi		_strcmpi	// Non ANSI equiv functions ?
#define strnicmp	_strnicmp
#define strupr		_strupr

#if defined(_MSC_VER) && ( _MSC_VER < 1100 )
// MSC Version 5.0 defines these from the ANSI spec. MSC 4.1 does not.
#define bool	int
#define false	0
#define true	1
#endif	// _MSC_VER

#define getclock() clock()	// CLOCKS_PER_SEC

#else	// assume LINUX

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

// Compatibility stuff.
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR    (-1)
#define SOCKET			int
#define HANDLE			DWORD
#define _cdecl
#define LONGLONG		DWORD	// This should be 64 bit ???
#define	TCHAR			char			// a text char.
#define FAR
#define E_FAIL			0x80004005
#define BOOL			unsigned short
#define TCP_NODELAY		0x0001

#define IsBadReadPtr( p, len )		((p) == NULL)
#define IsBadStringPtr( p, len )	((p) == NULL)
#define Sleep(mSec)					usleep( (mSec) * 1000 )	// arg is microseconds = 1/1000000

#define strcmpi		strcasecmp
#define strnicmp	strncasecmp
extern void strupr( TCHAR * pText );	// no lib version of this.

#ifndef INT_MIN	// now in limits.h
#define INT_MIN			(-2147483647) // - 1)
#endif
extern unsigned long int getclock();
#ifdef CLOCKS_PER_SEC
#undef CLOCKS_PER_SEC
#endif
#define CLOCKS_PER_SEC 1000
#endif

#define ISWHITESPACE(ch)		 (isspace(ch)||(ch)==0xa0)	// isspace
#define GETNONWHITESPACE( pStr ) while ( ISWHITESPACE( (pStr)[0] )) { (pStr)++; }

#ifndef _TEXT	// No UNICODE by default.
#define _TEXT
#endif

#include "graybase.h"

//---------------------------PROGRAM DEFINITIONS---------------------------

#include "carray.h"
#include "cstring.h"
#include "cfile.h"
#include "cscript.h"
#include "cexpression.h"

#ifdef _AFXDLL

//class CGSocket : public CSocket
//{
//};

#else

class CGSocket
{
private:
	SOCKET  m_hSocket;	// my winsock connect handle
private:
	void Clear()
	{
		// Transfer the socket someplace else.
		m_hSocket = INVALID_SOCKET;
	}
public:
	CGSocket()
	{
		Clear();
	}
	CGSocket( SOCKET socket )	// accept case.
	{
		m_hSocket = socket;
	}
	static int GetLastError();
	bool IsOpen() const
	{
		return( m_hSocket != INVALID_SOCKET );
	}
	bool Create()
	{
		ASSERT( ! IsOpen());
		m_hSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		return( IsOpen());
	}
	int Bind( struct sockaddr_in * pAddr )
	{
		return( bind( m_hSocket, (struct sockaddr *) pAddr, sizeof(*pAddr)));
	}
	int Bind( WORD uPort )
	{
		struct sockaddr_in connection;
		connection.sin_family = AF_INET;
		connection.sin_addr.s_addr = INADDR_ANY;
		connection.sin_port = htons( uPort );
		return( Bind( &connection ));
	}
	int Listen( int iMaxBacklogConnections = SOMAXCONN )
	{
		return( listen( m_hSocket, iMaxBacklogConnections ));
	}
	int Connect( struct sockaddr_in * pAddr )
	{
		// RETURN: 0 = success, else SOCKET_ERROR
		return( connect( m_hSocket, (struct sockaddr*) pAddr, sizeof(*pAddr)));
	}
	int Connect( struct in_addr ip, WORD wPort )
	{
		struct sockaddr_in connection;
		connection.sin_family = AF_INET;
		connection.sin_addr = ip;
		connection.sin_port = htons( wPort );
		return( Connect( &connection ));
	}
	int Connect( const CSocketAddress & ip )
	{
		return( Connect( ip, ip.GetPort()));
	}
	int Connect( const TCHAR * pszHostName, WORD wPort )
	{
		CSocketAddress ip;
		ip.SetHostStr( pszHostName );
		ip.SetPort( wPort );
		return( Connect( ip ));
	}
	SOCKET Accept( struct sockaddr_in * pAddr, int * plen ) const
	{
		return( accept( m_hSocket, (struct sockaddr*) pAddr, plen ));
	}
	int Send( const void * pData, int len ) const
	{
		// RETURN: length sent
		return( send( m_hSocket, (char*) pData, len, 0 ));
	}
	int Receive( void * pData, int len, int flags = 0 )
	{
		// RETURN: length, <= 0 is closed or error.
		// flags = MSG_PEEK or MSG_OOB
		return( recv( m_hSocket, (char*) pData, len, flags ));
	}
	int GetSockName( struct sockaddr_in * pName ) const
	{
		// Get the address of the near end. (us)
		int len = sizeof( *pName );
		return( getsockname( m_hSocket, (struct sockaddr *) pName, &len ));
	}
	int GetPeerName( struct sockaddr_in * pName ) const
	{
		// Get the address of the far end.
		// RETURN: 0 = success
		int len = sizeof( *pName );
		return( getpeername( m_hSocket, (struct sockaddr *) pName, &len ));
	}
	int SetSockOpt( int nOptionName, const void* optval, int optlen, int nLevel = SOL_SOCKET )
	{
		// level = SOL_SOCKET and IPPROTO_TCP.
		return( setsockopt( m_hSocket, nLevel, nOptionName, (const char FAR *) optval, optlen ));
	}
	SOCKET GetSocket() const
	{
		return( m_hSocket );
	}
	void Close()
	{
		if ( ! IsOpen())
			return;
		shutdown(m_hSocket, 2);
#ifdef _WIN32
		closesocket( m_hSocket );
#else
		close(m_hSocket);		// LINUX i assume. SD_BOTH
#endif
		Clear();
	}
	~CGSocket()
	{
		Close();
	}
};

inline int CGSocket::GetLastError()
{
#ifdef _WIN32
	return( WSAGetLastError() );
#else
	return( h_errno );	// WSAGetLastError()
#endif
}

#endif	// _AFXDLL
#endif	// _INC_GRAYCOM_H
