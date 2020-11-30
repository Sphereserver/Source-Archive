//
// graycom.h
// common header file.
//

//#define _DEBUG	// compile a debug version with more verbose comments
//#define _WIN32	// This is the NT version else Linux version.

#define GRAY_NAME		"Westy"	// Information about the person who compiled this (Put your info here!)
#define GRAY_EMAIL		"westypeter@hotmail.com"
#define GRAY_TIMEZONE	"EST"	// Your timezone (who cares ?)
#define GRAY_CLIENT_STR	"1.25.35"	// current client we have the code for.
#define GRAY_CLIENT_VER 35

#if GRAY_CLIENT_VER == 35
#define MASTERKEY_HI 0x383477BC
#define MASTERKEY_LO 0x02345CC6
#endif
#if GRAY_CLIENT_VER == 34
#define MASTERKEY_HI 0x38ECEDAC	// 1
#define MASTERKEY_LO 0x025720C6	// 2
#endif
#if GRAY_CLIENT_VER == 33
#define MASTERKEY_HI 0x38A5679C
#define MASTERKEY_LO 0x02767CC6
#endif
#if GRAY_CLIENT_VER == 32
#define MASTERKEY_HI 0x389DE58C
#define MASTERKEY_LO 0x026950C6
#endif
#if GRAY_CLIENT_VER == 31
#define MASTERKEY_HI 0x395A647C
#define MASTERKEY_LO 0x0297BCC6
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock.h>
#include <io.h>
#include <dos.h>
#include <limits.h>
#include <conio.h>

#if defined(_MSC_VER) && ( _MSC_VER < 1100 )
// MSC Version 5.0 defines these from the ANSI spec. MSC 4.1 does not.
#define bool int
#define false 0
#define true 1
#endif	// _MSC_VER

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
//#include <linux/tcp.h>
//#include <linux/net.h>
//#include <linux/in.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Compatibility stuff.
#define strcmpi		strcasecmp
#define strnicmp	strncasecmp

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR    (-1)
#define SOCKET		int
#define IsBadReadPtr( p, len )	((p) == NULL)
#define IsBadStringPtr( p, len )	((p) == NULL)

extern void strupr( char * pText );
#endif

//---------------------------SYSTEM DEFINITIONS---------------------------

#ifndef BYTE
#define BYTE 		unsigned char	// 8 bits
#define WORD 		unsigned short	// 16 bits
#define DWORD		unsigned long	// 32 bits
#define UINT		unsigned int
#endif	// BYTE

#ifndef MAKELONG
#define MAKELONG(low, high) ((long)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#define LOBYTE(w)	((BYTE)(((WORD)(w))&0xFF))
#define HIBYTE(w)	((BYTE)(((WORD)(w))>>8))
#endif	// MAKELONG

#ifndef _MAX_PATH			// stdlib.h ?
#define _MAX_PATH   260 	// max. length of full pathname
#endif	// _MAX_PATH
#ifndef offsetof			// stddef.h ?
#define offsetof(s,m)   (size_t)( (char *)&(((s *)0)->m) - (char *)0 )
#endif	// offsetof
#ifndef COUNTOF
#define COUNTOF(a) 		(sizeof(a)/sizeof(a[0]))	// dimensionof() ?
#endif	// COUNTOF
#ifndef min					// limits.h ?
#define min(x,y)	((x) <? (y))
#define max(x,y)	((x) >? (y))
#endif	// min

//---------------------------PROGRAM DEFINITIONS---------------------------

#define GRAY_FILE		"gray"
#define GRAY_SCRIPT		".scp"
#define GRAY_PORT		2593	// ??? arbitrary ?

class CSocket
{
private:
	SOCKET  m_hSocket;	// my winsock connect handle

public:
	CSocket()
	{
		m_hSocket = INVALID_SOCKET;
	}
	CSocket( SOCKET socket )	// accept case.
	{
		m_hSocket = socket;
	}
	void Clear()
	{
		// Transfer the socket someplace else.
		m_hSocket = INVALID_SOCKET;
	}
	bool Create()
	{
		m_hSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		return( m_hSocket != INVALID_SOCKET );
	}
	int Bind( struct sockaddr_in * pAddr )
	{
		return( bind( m_hSocket, (struct sockaddr *) pAddr, sizeof(*pAddr)));
	}
	int Listen( int iMaxBacklogConnections )
	{
		return( listen( m_hSocket, iMaxBacklogConnections ));
	}
	int Connect( struct sockaddr_in * pAddr )
	{
		return( connect( m_hSocket, (struct sockaddr*) pAddr, sizeof(*pAddr)));
	}
	SOCKET Accept( struct sockaddr_in * pAddr, int * plen ) const
	{
		return( accept( m_hSocket, (struct sockaddr*) pAddr, plen ));
	}
	int Send( const char * pData, int len ) const
	{
		return( send( m_hSocket, pData, len, 0 ));
	}
	int Receive( char * pData, int len )
	{
		return( recv( m_hSocket, pData, len, 0 ));
	}
	int GetSockName( struct sockaddr_in * pName ) const
	{
		int len = sizeof( *pName );
		return( getsockname( m_hSocket, (struct sockaddr *) pName, &len ));
	}
	int GetPeerName( struct sockaddr_in * pName ) const
	{
		int len = sizeof( *pName );
		return( getpeername( m_hSocket, (struct sockaddr *) pName, &len ));
	}
	int GetSocket() const
	{
		return( m_hSocket );
	}
	void Close();
	~CSocket()
	{
		Close();
	}
};

