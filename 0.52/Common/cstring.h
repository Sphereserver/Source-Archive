//
// CString.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CSTRING_H
#define _INC_CSTRING_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "common.h"

#define MAX_SCRIPT_LINE_LEN 4096	// default size.

#ifdef _AFXDLL	// don't need this if using MFC.

struct CGString : public CString
{
public:
	bool IsValid() const
	{
		return( true );
	}
	void Copy( const TCHAR * pszText )
	{
		// ...	
	}
	CGString() {}
	CGString( const TCHAR * pszText ) :
		CString( pszText )
	{
	}
	// GetBuffer acts very dfferent here !
};

#else	// _AFXDLL

class CGFile;

class CGString 
{
private:
    TCHAR * m_pchData;	// This cannot be just zeroed !
	static const TCHAR sm_Nil;		// Use this instead of null.
    int m_iLength;

private:
    void Init( void )
    {
    	m_pchData = (TCHAR*)&sm_Nil;
    	m_iLength = 0;
	}    
protected:
#ifdef _DEBUG
    operator const CGString() const
    {
		return( *this );
	}
    operator CGString()
    {
		return( *this );
	}
#endif
	void ReleaseBuffer() {}
public:
   
	TCHAR * GetBuffer( void )
	{
		ASSERT( IsValid());
		return( m_pchData );
	}
    TCHAR * GetBuffer( int iMinLength )
	{
		if ( iMinLength > GetLength()) SetLength( iMinLength );
		return( GetBuffer() );
	}
    int SetLength( int iLen );
	CGString()
	{
		Init();
	}
    CGString( const TCHAR * pStr )
    {
	    Init();
    	Copy( pStr );
    }

    int GetLength() const
    {
		ASSERT( IsValid());
        return( m_iLength );
    }
    bool IsEmpty() const
    {
		ASSERT( IsValid());
    	return( ! m_iLength );
   	}
    void Empty();
	bool IsValid() const
	{
#if defined(_DEBUG) && defined(DEBUG_VALIDATE_ALLOC)
		if ( m_pchData != &sm_Nil )
		{
			DEBUG_ValidateAlloc( m_pchData );
		}
#endif
		return( m_pchData[m_iLength] == '\0' );
	}

	TCHAR GetAt(int nIndex) const      // 0 based
	{
		ASSERT( IsValid());
		ASSERT( nIndex <= m_iLength );	// allow to get the null char
		return( m_pchData[nIndex] );
	}
	TCHAR operator[](int nIndex) const // same as GetAt
	{
		return GetAt( nIndex );
	}
	void SetAt(int nIndex, TCHAR ch)
	{
		ASSERT( IsValid());
		ASSERT( nIndex < m_iLength );
		m_pchData[nIndex] = ch;
	}
	TCHAR & operator[](int nIndex)
	{
		ASSERT( nIndex < m_iLength );
		ASSERT( IsValid());
		return m_pchData[nIndex];
	}
	const TCHAR * GetPtr( void ) const
	{
		ASSERT( IsValid());
		return( m_pchData );
	}
    operator const TCHAR*() const       // as a C string
    {
		return( GetPtr());
    }
    void Copy( const TCHAR * pStr );
	const CGString& operator+=(const TCHAR* psz);	// like strcat
	const CGString& operator+=(TCHAR ch)
	{
		int iLen = GetLength();
		SetLength(iLen+1);
		SetAt( iLen, ch );
		return( *this );
	}
    ~CGString( void )
    {
        Empty();
    }
    int CopyTo( TCHAR * pStr ) const
    {
    	strcpy( pStr, GetPtr() );
    	return( GetLength());
	}
    const CGString& operator=( const CGString &s )
    {
		Copy( s.GetPtr());
		return( *this );
    }
    const CGString& operator=( const TCHAR * pStr )
    {
        Copy( pStr );
		return( *this );
    }
	int VFormat( const TCHAR * pStr, va_list args );
	int _cdecl Format( const TCHAR * pStr, ... )
	{
		va_list args;
		va_start( args, pStr );
		return( VFormat( pStr, args ));
	}
	int FormatVal( long iVal )
	{
		return Format( "%d", iVal );
	}
	int FormatHex( DWORD dwVal )
	{
		return Format( "0%x", dwVal );
	}
	int Compare( const TCHAR * pStr ) const
	{
		ASSERT( IsValid());
		return( strcmp( GetPtr(), pStr ));
	}
	int CompareNoCase( const TCHAR * pStr ) const
	{
		ASSERT( IsValid());
		return( strcmpi( GetPtr(), pStr ));
	}
	void MakeUpper()
	{
		strupr(m_pchData);
	}

	int   ReadZ( CGFile * pFile, int iLenMax );
	bool  WriteZ( CGFile * pFile );

	int   ReadFrom( CGFile * pFile, int iLenMax );
	bool  WriteTo( CGFile * pFile );

#ifdef UNICODE
	void Copy( const char * pStr );
	const CGString& operator=( const char * lpsz )
	{
		Copy( lpsz );
		return( *this );
	}
#endif
};

#endif // ! _AFXDLL

enum MATCH_TYPE			// match result defines
{
	MATCH_INVALID = 0,
	MATCH_VALID,		// valid match
	MATCH_END,			// premature end of pattern string
	MATCH_ABORT,		// premature end of text string
	MATCH_RANGE,		// match failure on [..] construct
	MATCH_LITERAL,		// match failure on literal match
	MATCH_PATTERN,		// bad pattern
};
extern MATCH_TYPE Text_Match( const TCHAR *pPattern, const TCHAR * pText );

extern TCHAR * GetTempStr();
extern int GetBareText( TCHAR * pszOut, const TCHAR * pszInp, int iMaxSize, const TCHAR * pszStrip = NULL );
extern TCHAR * MakeFilteredStr( TCHAR * pStr );
extern int strcpylen( TCHAR * pDst, const TCHAR * pSrc );
extern int strcpylen( TCHAR * pDst, const TCHAR * pSrc, int imaxlen );
extern TCHAR * GetNonWhitespace( const TCHAR * pStr );
extern int TrimEndWhitespace( TCHAR * pStr, int len );
extern TCHAR * TrimWhitespace( TCHAR * pStr );
extern bool Parse( TCHAR * pLine, TCHAR ** ppArg = NULL, const TCHAR *pSep = NULL );
extern int ParseCmds( TCHAR * pCmdLine, TCHAR ** ppCmd, int iMax, const TCHAR * pSep = NULL );
extern int ParseCmds( TCHAR * pCmdLine, int * piCmd, int iMax, const TCHAR * pSep = NULL );
extern int FindTable( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize = sizeof(const TCHAR *));
extern int FindTableHead( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize = sizeof(const TCHAR *));
extern int FindTableSorted( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize = sizeof(const TCHAR *));

#endif // _INC_CSTRING_H