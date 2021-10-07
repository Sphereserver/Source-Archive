//
// CGString.CPP
// Copyright Menace Software (www.menasoft.com).
//

#include "graycom.h"

#ifndef _WIN32
void _strupr( TCHAR * pszStr )
{
	// No portable UNIX/LINUX equiv to this.
	for ( ;pszStr[0] != '\0'; pszStr++ )
	{
		*pszStr = toupper( *pszStr );
	}
}

void _strlwr( TCHAR * pszStr )
{
	// No portable UNIX/LINUX equiv to this.
	for ( ;pszStr[0] != '\0'; pszStr++ )
	{
		*pszStr = tolower( *pszStr );
	}
}
#endif

int strcpylen( TCHAR * pDst, LPCTSTR pSrc )
{
	strcpy( pDst, pSrc );
	return( strlen( pDst ));
}

int strcpylen( TCHAR * pDst, LPCTSTR pSrc, int iMaxSize )
{
	// it does NOT include the iMaxSize element! (just like memcpy)
	// so iMaxSize=sizeof() is ok !
	ASSERT( iMaxSize );
	strncpy( pDst, pSrc, iMaxSize-1 );
	pDst[iMaxSize-1] = '\0';	// always terminate.
	return( strlen( pDst ));
}

//***************************************************************************
// -CGString

void CGString::Empty(bool bTotal)
{
	if ( bTotal )
	{
		if ( m_iMaxLength && m_pchData )
		{
			delete []m_pchData;
			m_pchData = NULL;
			m_iMaxLength = 0;
		}
	}
	else m_iLength = 0;
}

int CGString::SetLength( int iNewLength )
{
	if ( iNewLength >= m_iMaxLength )
	{
		m_iMaxLength = iNewLength+64;				// allow grow, and always expand only
		TCHAR	*pNewData = new TCHAR[m_iMaxLength+1];
		ASSERT(pNewData);

		int iMinLength = min(iNewLength, m_iLength);
		strncpy(pNewData, m_pchData, iMinLength);

		if ( m_pchData ) delete []m_pchData;
		m_pchData = pNewData;
	}
	m_iLength = iNewLength;
	m_pchData[m_iLength] = 0;
	return m_iLength;
}

void CGString::Copy( LPCTSTR pszStr )
{
	if (( pszStr != m_pchData ) && pszStr )
	{
		SetLength(strlen(pszStr));
		strcpy(m_pchData, pszStr);
	}
}

void CGString::FormatV(LPCTSTR pszFormat, va_list args)
{
	TCHAR	*pszTemp = Str_GetTemp();
	vsprintf(pszTemp, pszFormat, args);
	Copy(pszTemp);
}

void CGString::Add( TCHAR ch )
{
	int iLen = m_iLength;
	SetLength(iLen+1);
	SetAt(iLen, ch);
}

void CGString::Add( LPCTSTR pszStr )
{
	int iLenCat = strlen(pszStr);
	if ( iLenCat )
	{
		SetLength(iLenCat + m_iLength);
		strcat(m_pchData, pszStr);
	}
}

CGString::~CGString()
{
	Empty(true);
}

CGString::CGString()
{
	Init();
}

CGString::CGString(LPCTSTR pStr)
{
	m_iMaxLength = m_iLength = 0;
	m_pchData = NULL;
	Copy(pStr);
}

CGString::CGString(const CGString &s)
{
	m_iMaxLength = m_iLength = 0;
	m_pchData = NULL;
	Copy(s.GetPtr());
}

bool CGString::IsValid() const
{
	if ( !m_iMaxLength ) return false;
	return( m_pchData[m_iLength] == '\0' );
}
TCHAR * CGString::GetBuffer( void )
{
	return (m_pchData);
}
TCHAR * CGString::GetBuffer(int iMinLength)
{
	if ( iMinLength > m_iMaxLength ) SetLength(iMinLength);
	return( GetBuffer() );
}
//int CGString::SetLength( int iLen );

int CGString::GetLength() const
{
    return (m_iLength);
}
bool CGString::IsEmpty() const
{
    return( !m_iLength );
}
TCHAR & CGString::ReferenceAt(int nIndex)       // 0 based
{
	ASSERT( nIndex < m_iLength );
	return m_pchData[nIndex];
}
TCHAR CGString::GetAt(int nIndex) const      // 0 based
{
	ASSERT( nIndex <= m_iLength );	// allow to get the null char
	return( m_pchData[nIndex] );
}
void CGString::SetAt(int nIndex, TCHAR ch)
{
	ASSERT( nIndex < m_iLength );
	m_pchData[nIndex] = ch;
	if ( !ch ) m_iLength = strlen(m_pchData);	// \0 inserted. line truncated
}
LPCTSTR CGString::GetPtr( void ) const
{
	return( m_pchData );
}

//void CGString::Copy( LPCTSTR pStr );

int CGString::CopyTo( TCHAR * pStr ) const
{
    strcpy(pStr, m_pchData);
    return m_iLength;
}
void _cdecl CGString::Format( LPCTSTR pStr, ... )
{
	va_list vargs;
	va_start( vargs, pStr );
	FormatV( pStr, vargs );
	va_end( vargs );
}
void CGString::FormatVal( long iVal )
{
	Format("%d", iVal);
}
void CGString::FormatHex( DWORD dwVal )
{
	Format("0%x", dwVal);
}
int CGString::Compare( LPCTSTR pStr ) const
{
	return (strcmp(m_pchData, pStr));
}
int CGString::CompareNoCase( LPCTSTR pStr ) const
{
	return (strcmpi(m_pchData, pStr));
}

void CGString::Init( void )
{
	m_iMaxLength = 128;
	m_iLength = 0;
	m_pchData = new TCHAR[m_iMaxLength+1];
	m_pchData[m_iLength] = 0;
}
