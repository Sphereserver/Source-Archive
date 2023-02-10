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

//---------------------------SYSTEM DEFINITIONS---------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#ifdef _WIN32

#ifndef STRICT
#define STRICT			// strict conversion of handles and pointers.
#endif	// STRICT

#include <io.h>
#include <windows.h>
#include <winsock.h>
#include <dos.h>
#include <limits.h>
#include <conio.h>
#include <sys/timeb.h>

#define strcmpi		_strcmpi	// Non ANSI equiv functions ?
#define strnicmp	_strnicmp

#if defined(_MSC_VER) && ( _MSC_VER < 1100 )
// MSC Version 5.0 defines these from the ANSI spec. MSC 4.1 does not.
#define bool	int
#define false	0
#define true	1
#endif	// _MSC_VER

extern const OSVERSIONINFO * GRAY_GetOSInfo();

#else	// _WIN32 else assume LINUX

#include <sys/types.h>
#include <sys/timeb.h>

#define HANDLE			DWORD
#define _cdecl
#define LONG			DWORD
#define LONGLONG		DWORD	// This should be 64 bit ???
#define WCHAR			unsigned short
#define FAR
#define E_FAIL			0x80004005
#define BOOL			unsigned short
#ifdef _BSD
int getTimezone();
#define _timezone		getTimezone()
#else
#define _timezone		timezone
#endif
#define PUINT			unsigned int *
#define LPTSTR			LPCTSTR

#define IsBadReadPtr( p, len )		((p) == NULL)
#define IsBadStringPtr( p, len )	((p) == NULL)
#define Sleep(mSec)					usleep( (mSec) * 1000 )	// arg is microseconds = 1/1000000

#define strcmpi		strcasecmp
#define strnicmp	strncasecmp
#define _vsnprintf	vsnprintf

#ifndef INT_MIN	// now in limits.h
#define INT_MIN			(-2147483647) // - 1)
#define INT_MAX       2147483647    /* maximum (signed) int value */
#endif
#ifndef SHRT_MIN
#define SHRT_MIN    (-32768)        /* minimum (signed) short value */
#define SHRT_MAX      32767         /* maximum (signed) short value */
#define USHRT_MAX	0xffff
#endif
#endif // !_WIN32

#ifndef _TEXT	// No UNICODE by default.
#define _TEXT
#endif

// #define ASSERT(exp)			(void)( (exp) || (Assert_CheckFail(#exp, __FILE__, __LINE__), 0) )
// extern void Assert_CheckFail( const char * pExp, const char *pFile, unsigned uLine );

#ifdef _DEBUG
#ifdef GRAY_SVR
#ifndef ASSERT
extern void Assert_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define ASSERT(exp)			(void)( (exp) || (Assert_CheckFail(#exp, __FILE__, __LINE__), 0) )
#endif	// ASSERT
#else	// GRAY_SVR
#ifndef ASSERT
#define ASSERT			assert
#endif
#endif	// ! GRAY_SVR
#ifndef DEBUG_CHECK
extern void Debug_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define DEBUG_CHECK(exp)	(void)( (exp) || (Debug_CheckFail(#exp, __FILE__, __LINE__), 0) )
#endif	// DEBUG_CHECK
#ifndef STATIC_CAST
#define STATIC_CAST dynamic_cast	// NOTE: we should ASSERT != NULL here ?
#endif

#else	// _DEBUG

#ifndef ASSERT
#ifndef _WIN32
// In linux, if we get an access violation, an exception isn't thrown.  Instead, we get a SIG_SEGV, and
// the process cores.  The following code takes care of this for us
extern void Assert_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define ASSERT(exp)			(void)( (exp) || (Assert_CheckFail(#exp, __FILE__, __LINE__), 0) )
#else
#define ASSERT(exp)
#endif
#endif	// ASSERT

#ifndef DEBUG_CHECK
#define DEBUG_CHECK(exp)
#endif

#ifndef STATIC_CAST
#define STATIC_CAST static_cast
#endif

#endif	// ! _DEBUG

#ifdef _WIN32
	#define ATOI atoi
	#define LTOA ltoa
#else
	int ATOI( const char * str );
	char * LTOA(long value, char *string, int radix);
#endif

// Macro for fast NoCrypt Client version check
#define IsNoCryptVer( value ) 	( !m_Crypt.GetClientVer() && g_Cfg.m_iUsenocrypt >= value )

#include "common.h"
#include "CSocket.h"
#include "CThread.h"
#include "CCrypt.h"
#include "CArray.h"
#include "CString.h"
#include "CFile.h"
#include "CScript.h"

class CTextConsole; // swapped these two includes, so need to declare this here
#include "CExpression.h"
#include "CScriptObj.h"

class CObjBase;
class CChar;
class CItem;
class CResourceDef;

#ifdef _AFXDLL	// using MFC.
class CGrayError : public CException
#else
#define CException CGrayError
class CGrayError
#endif
{
	// we can throw this structure to produce an error.
	// similar to CFileException and CException
#ifdef _AFXDLL	
	DECLARE_DYNAMIC(CGrayError)
#endif
public:
	LOGL_TYPE m_eSeverity;	// const
	DWORD m_hError;	// HRESULT S_OK, "winerror.h" code. 0x20000000 = start of custom codes.
	LPCTSTR m_pszDescription;
public:
	CGrayError( LOGL_TYPE eSev, DWORD hErr, LPCTSTR pszDescription );
	CGrayError( const CGrayError& e );	// copy contstructor needed.
	virtual ~CGrayError();

	LOGL_TYPE GetSeverity() const { return m_eSeverity; }
	DWORD GetErrorCode() const { return m_hError; }
	LPCTSTR GetDescription() const { return m_pszDescription; }

	static int GetSystemErrorMessage( DWORD dwError, LPTSTR lpszError, UINT nMaxError );

	virtual BOOL GetErrorMessage( LPSTR lpszError, UINT nMaxError,	PUINT pnHelpContext = NULL );
};

struct CGrayUIDBase		// A unique system serial id. 4 bytes long
{
	// This is a ref to a game object. It may or may not be valid.
	// The top few bits are just flags.
#define UID_CLEAR			0
#define UID_UNUSED			0xFFFFFFFF	// 0 = not used as well.

#define UID_F_RESOURCE		0x80000000	// ALSO: pileable or special macro flag passed to client.
#define UID_F_ITEM			0x40000000	// CItem as apposed to CChar based

#define UID_O_EQUIPPED		0x20000000	// This item is equipped.
#define UID_O_CONTAINED		0x10000000	// This item is inside another container
#define UID_O_DISCONNECT	0x30000000	// Not attached yet.
#define UID_O_INDEX_MASK	0x0FFFFFFF	// lose the upper bits.
#define UID_O_INDEX_FREE	0x01000000	// Spellbook needs unused UID's ?

protected:
	DWORD m_dwInternalVal;
public:

	bool IsValidUID( void ) const
	{
		return( m_dwInternalVal && ( m_dwInternalVal & UID_O_INDEX_MASK ) != UID_O_INDEX_MASK );
	}
	void InitUID( void )
	{
		m_dwInternalVal = UID_UNUSED;
	}
	void ClearUID( void )
	{
		m_dwInternalVal = UID_CLEAR;
	}

	bool IsResource( void ) const
	{
		if ( m_dwInternalVal & UID_F_RESOURCE )
			return( IsValidUID() );
		return( false );
	}
	bool IsItem( void ) const	// Item vs. Char
	{
		if (( m_dwInternalVal & (UID_F_ITEM|UID_F_RESOURCE)) == UID_F_ITEM )
			return( true );	// might be static in client ?
		return( false );
	}
	bool IsChar( void ) const	// Item vs. Char
	{
		if (( m_dwInternalVal & (UID_F_ITEM|UID_F_RESOURCE)) == 0 )
			return( IsValidUID());
		return( false );
	}

	bool IsObjDisconnected( void ) const	// Not in the game world for some reason.
	{
		ASSERT( ! IsResource());
		if (( m_dwInternalVal & (UID_F_RESOURCE|UID_O_DISCONNECT)) == UID_O_DISCONNECT )
			return( true );
		return( false );
	}
	bool IsObjTopLevel( void ) const	// on the ground in the world.
	{
		ASSERT( ! IsResource());
		if (( m_dwInternalVal & (UID_F_RESOURCE|UID_O_DISCONNECT)) == 0 )
			return( true );	// might be static in client ?
		return( false );
	}

	bool IsItemEquipped( void ) const
	{
		ASSERT( ! IsResource());
		if (( m_dwInternalVal & (UID_F_RESOURCE|UID_F_ITEM|UID_O_DISCONNECT)) == (UID_F_ITEM|UID_O_EQUIPPED))
			return( IsValidUID() );
		return( false );
	}
	bool IsItemInContainer( void ) const
	{
		ASSERT( ! IsResource());
		if (( m_dwInternalVal & (UID_F_RESOURCE|UID_F_ITEM|UID_O_DISCONNECT)) == (UID_F_ITEM|UID_O_CONTAINED))
			return( IsValidUID() );
		return( false );
	}

	void SetObjContainerFlags( DWORD dwFlags = 0 )
	{
		ASSERT( ! ( dwFlags & (UID_F_RESOURCE|UID_O_INDEX_MASK|UID_F_ITEM)));
		m_dwInternalVal = ( m_dwInternalVal & (UID_O_INDEX_MASK|UID_F_ITEM )) | dwFlags;
	}

	void SetPrivateUID( DWORD dwVal )
	{
		m_dwInternalVal = dwVal;
	}
	DWORD GetPrivateUID() const
	{
		return m_dwInternalVal;
	}

	DWORD GetObjUID() const
	{
		return( m_dwInternalVal & (UID_O_INDEX_MASK|UID_F_ITEM) );
	}
	void SetObjUID( DWORD dwVal )
	{
		// can be set to -1 by the client.
		m_dwInternalVal = ( dwVal & (UID_O_INDEX_MASK|UID_F_ITEM)) | UID_O_DISCONNECT;
	}

	bool operator == ( DWORD index ) const
	{
		return( GetObjUID() == index );
	}
	bool operator != ( DWORD index ) const
	{
		return( GetObjUID() != index );
	}
    operator DWORD () const
    {
		return( GetObjUID());
    }

	CObjBase * ObjFind() const;
	CItem * ItemFind() const; // Does item still exist or has it been deleted
	CChar * CharFind() const; // Does character still exist
	CResourceDef * ResourceFind() const;
};

struct CGrayUID : public CGrayUIDBase
{
	CGrayUID()
	{
		InitUID();
	}
	CGrayUID( DWORD dw )
	{
		SetPrivateUID( dw );
	}
};

#ifndef GRAY_SVR
class CServTime
{
	// The game server time. High accuracy. May role over.
#undef GetCurrentTime
#define	TICK_PER_SEC 1000
public:
	DWORD m_dwPrivateTime;
public:
	void Init()
	{
		m_dwPrivateTime = 0;
	}
	bool IsTimeValid() const
	{
		return( m_dwPrivateTime != 0 );
	}
	int operator-( CServTime time ) const
	{
		return(m_dwPrivateTime-time.m_dwPrivateTime);
	}
	static CServTime GetCurrentTime();
};
inline CServTime CServTime::GetCurrentTime()	// static
{
	CServTime timeVar;
	timeVar.m_dwPrivateTime = GetTickCount();
	return( timeVar );
}
#endif	// ! GRAY_SVR

#ifdef WM_USER
#define WM_GRAY_CLIENT_COMMAND	(WM_USER+123)	// command the client to do something.
#endif

#endif	// _INC_GRAYCOM_H
