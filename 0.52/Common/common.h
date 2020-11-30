//
// common.h
// Copyright Menace Software (www.menasoft.com).
// always for __cplusplus
// I try to compile in several different environments.
// 1. DOS command line or windows (_WINDOWS	by compiler or _INC_WINDOWS in windows.h)
// 2. MFC or not MFC  (__AFX_H__ in afx.h or _MFC_VER by compiler)
// 3. 16 bit or 32 bit (_WIN32 defined by compiler)
// 4. LINUX 32 bit
//

#ifndef _INC_COMMON_H
#define _INC_COMMON_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// C standard types.

#ifndef _MAX_PATH			// stdlib.h ?
#define _MAX_PATH   260 	// max. length of full pathname
#endif
#ifndef offsetof			// stddef.h ?
#define offsetof(s,m)   	(int)( (BYTE *)&(((s *)0)->m) - (BYTE *)0 )
#endif
#define IMULDIV(a,b,c) (((a)*(b))/(c))	// the windows version on MulDiv will round !

// These must be made to work for both longs and ints
#ifndef min					// limits.h ?
#define min(x,y)	((x) <? (y))
#define max(x,y)	((x) >? (y))
#endif	// min
#ifndef sign
#define sign(n) (((n) < 0) ? -1 : (((n) > 0) ? 1 : 0))
#endif	// sign

// Windows type standard stuff.

#ifndef ERROR_SUCCESS		// #include <winerror.h>
#define ERROR_SUCCESS 		0	// EZERO ?
#endif
#ifndef UNREFERENCED_PARAMETER	// _WIN32 type thing.
#define UNREFERENCED_PARAMETER(P)          (P)
#endif
#ifndef HKEY_LOCAL_MACHINE	// Not defined in normal 16 bit API
#define HKEY_LOCAL_MACHINE	(( HKEY ) 0x80000002 )
#endif	// HKEY_LOCAL_MACHINE

#ifndef _UNICODE		// _WIN32
#define TCHAR			char
#define LPCTSTR			LPCSTR
#define LPCWSTR			LPCSTR
#endif  // _UNICODE _WIN32
#ifndef _TEXT
#define _TEXT(x)		x
#endif	// _TEXT

#ifndef _INC_WINDOWS	// Did NOT include "windows.h" need these for UNICODE.
#ifdef _WIN32
#define lstrcpy		strcpy
#define lstrlen		strlen
#else
#define lstrcpy		_fstrcpy
#define lstrlen		_fstrlen
#endif
#define wvsprintf	vsprintf
#define wsprintf	sprintf
#else	// _INC_WINDOWS
#define strcpy		lstrcpy		
#define strlen		lstrlen		
#define vsprintf	wvsprintf
#define sprintf		wsprintf	
#endif	// _INC_WINDOWS

#ifdef _WIN32
#define WINCALL         CALLBACK
#define HANDLE_DLGMSG   HANDLE_MSG
#else	// _WIN32
#define WINCALL         CALLBACK _export	// 16 bit 
#define HANDLE_DLGMSG(hwnd, message, fn) case (message): return (UINT) HANDLE_##message((hwnd), (wParam), (lParam), (fn))
#endif	// ! _WIN32

#ifndef TRUE	// Did not include "windows.h"
#define TRUE		    1
#define FALSE		    0

#ifdef _WIN32
#define NEAR
#define FAR
#elif defined( _WIN16 )  // _WIN32
#define NEAR	_near
#define FAR		_far
#endif	// ! _WIN32

#ifndef BYTE	// might be a typedef ?
#define BYTE 		unsigned char	// 8 bits
#define WORD 		unsigned short	// 16 bits
#define DWORD		unsigned long	// 32 bits
#define UINT		unsigned int
#endif	// BYTE

#endif	// TRUE

#ifndef _WIN32	
#define _cdecl
#define bool int
#define true 1
#define false 0
#define TCHAR char
#define LPCSTR const char *
#define LPCTSTR const char *
#define MulDiv	IMULDIV
#endif

#ifndef MAKELONG
#define MAKELONG(low, high) ((long)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)			((BYTE)(((WORD)(w))&0xFF))
#define HIBYTE(w)			((BYTE)(((WORD)(w))>>8))
#endif	// MAKELONG
#ifndef MAKEDWORD
#define MAKEDWORD(low, high) ((DWORD)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif	// MAKEDWORD
// My personal standard tastes.

#ifndef COUNTOF
#define COUNTOF(a) 		(sizeof(a)/sizeof((a)[0]))	// dimensionof() ?
#endif

typedef unsigned int	ERROR_CODE;	// ERROR_SUCCESS
#define _IS_ERROR(s)	((s)!=ERROR_SUCCESS)

#define _BITMASK(b) 	(1<<(b))
#define _ISSET(w,b) 	((w)&_BITMASK(b))
#define _ISCLR(w,b) 	(!_ISSET(w,b))

#ifdef _DEBUG
#define _LOCCALL        // Give local calls better calling conventions
#else
#define _LOCCALL        // __fastcall // Local procedure name modifier.
#endif

#define _WORDCAST(p)	(*((WORD*)(p)))	// Get a word from a pointer of unknown type.

//
// Debug Message Macro.
//
#ifndef DEBUG_MSG

#include <stdarg.h>

#ifndef ASSERT
#define ASSERT
#endif // ASSERT
#ifndef DEBUG_CHECK
#define DEBUG_CHECK
#endif // DEBUG_CHECK

enum LOGL_TYPE	
{
	// critical level.
	LOGL_FATAL	= 1, 	// fatal error ! cannot continue.
	LOGL_CRIT	= 2, 	// critical. might not continue.
	LOGL_ERROR	= 3, 	// non-fatal errors. can continue.
	LOGL_WARN	= 4,	// strange.
	LOGL_EVENT	= 5,	// Misc major events.
	LOGL_TRACE	= 6,	// low level debug trace.
};

extern class CEventLog
{
	// Any text event stream. (destination is independant)
	// May include __LINE__ or __FILE__ macro as well ?

protected:
	virtual int EventStr( WORD wMask, const TCHAR * pszMsg )
	{
#ifdef _WINDOWS
		OutputDebugString( pszMsg );
#endif
		return( 0 );
	}
	virtual int VEvent( WORD wMask, const TCHAR * pszFormat, va_list args )
	{
		// This is currently not overriden but just in case we might want to.
		TCHAR szTemp[ 1024 ];	// assume this magic number is big enough.
		ASSERT( pszFormat && *pszFormat );
		size_t len = vsprintf( szTemp, pszFormat, args );
		if ( ! len ) strcpy( szTemp, pszFormat );
		return( EventStr( wMask, szTemp ));
	}

public:
	int _cdecl Event( WORD wMask, const TCHAR * pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		return( VEvent( wMask, pszFormat, args ));
	}

#define DEBUG_ERR(_x_)		g_pLog->EventError _x_
	int _cdecl EventError( const TCHAR * pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		return( VEvent( LOGL_ERROR, pszFormat, args ));
	}

#ifdef _DEBUG
#define DEBUG_WARN(_x_)		g_pLog->EventWarn _x_
#define DEBUG_MSG(_x_)		g_pLog->EventEvent _x_	
#define DEBUG_TRACE(_x_)	g_pLog->EventTrace _x_	

	int _cdecl EventWarn( const TCHAR * pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		return( VEvent( LOGL_WARN, pszFormat, args ));
	}
	int _cdecl EventEvent( const TCHAR * pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		return( VEvent( LOGL_EVENT, pszFormat, args ));
	}
	int _cdecl EventTrace( const TCHAR * pszFormat, ... )
	{
		va_list args;
		va_start( args, pszFormat );
		return( VEvent( LOGL_TRACE, pszFormat, args ));
	}

#else
#define DEBUG_WARN(_x_)
#define DEBUG_MSG(_x_)				// this allows all the variable args to be removed
#define DEBUG_TRACE(_x_)				// this allows all the variable args to be removed
#endif
} * g_pLog;

#endif	// DEBUG_MSG

#ifndef INT_MAX	// assumes 32 bit !
#define INT_MAX       2147483647    // maximum (signed) int value
#endif

#define _IS_SWITCH(c)    ((c) == '-' || (c) == '/' )	// command line switch.

#endif	// _INC_COMMON_H


