    // CGString.CPP
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #if defined(GRAY_MAKER)
    #include "../graymaker/stdafx.h"
    #include "../graymaker/graymaker.h"
    #elif defined(GRAY_MAP)
    #include "../graymap/stdafx.h"
    #include "../graymap/graymap.h"
    #elif defined(GRAY_AGENT)
    #include "../grayagent/stdafx.h"
    #include "../grayagent/grayagent.h"
    #elif defined(GRAY_SVR) || defined(GRAY_CLIENT)
    #include "graycom.h"
    #else
    #define STRICT
    #include <windows.h>
    #include "cstring.h"
    #endif
    
    #ifndef _WIN32
    void _strupr( TCHAR * pszStr )
    {
    	// No portable UNIX/LINUX equiv to this.
    	for ( ;pszStr != '\0'; pszStr++ )
    	{
    		*pszStr = toupper( *pszStr );
    	}
    }
    
    void _strlwr( TCHAR * pszStr )
    {
    	// No portable UNIX/LINUX equiv to this.
    	for ( ;pszStr != '\0'; pszStr++ )
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
    	pDst = '\0';	// always terminate.
    	return( strlen( pDst ));
    }
    
    #ifndef _AFXDLL
    
    //***************************************************************************
    // -CGString
    
    const TCHAR CGString::sm_Nil = '\0'; // Use this instead of null.
    
    void CGString::Empty()
    {
    	if ( m_pchData != &sm_Nil && m_pchData != NULL )	// certain off instances where it could be NULL. arrays
    	{
    		delete  m_pchData;
    		Init();
    	}
    }
    
    int CGString::SetLength( int iNewLength )
    {
    	ASSERT( IsValid());
    	ASSERT(iNewLength>=0);
    	if ( ! iNewLength )
    	{
    		Empty();
    		return( 0 );
    	}
    
    	TCHAR * pNewData = new TCHAR ;
    	ASSERT( pNewData );
    
    	int iMinLength = min(iNewLength,m_iLength);	// just set it shorter.
    	strncpy( pNewData, m_pchData, iMinLength );
    	pNewData = '\0';	// validation.
    	pNewData = '\0';
    
    	Empty();
    
    	m_pchData = pNewData;
    	m_iLength = iNewLength;
    	return( m_iLength );
    }
    
    void CGString::Copy( LPCTSTR pszStr )
    {
    	// DEBUG_MSG(( "CGString:Copy" ));
    	if ( pszStr != m_pchData )    // Same string.
    	{
    		Empty();
    		if ( pszStr != NULL )
    		{
    			if ( SetLength( strlen( pszStr )))
    			{
    				strcpy( m_pchData, pszStr );
    			}
    		}
    	}
    }
    
    void CGString::FormatV( LPCTSTR pszFormat, va_list args )
    {
    	TCHAR szTemp;
    	vsprintf( szTemp, pszFormat, args );
    	Copy( szTemp );
    }
    
    void CGString::Add( TCHAR ch )
    {
    	int iLen = GetLength();
    	SetLength(iLen+1);
    	SetAt( iLen, ch );
    }
    
    void CGString::Add( LPCTSTR pszStr )
    {
    	ASSERT( pszStr );
    	int iLenCat = strlen( pszStr );
    	if ( iLenCat )
    	{
    		int iLenOld = GetLength();
    		SetLength(iLenOld+iLenCat);
    		strcpy( m_pchData+iLenOld, pszStr );
    	}
    }
    
    #ifdef UNICODE
    const CGString& CGString::Copy( const char * pszStr )
    {
    	// Get a string that is not in UNICODE
    	Empty();
    	if ( pszStr != NULL )
    	{
    		int nBaseLen = lstrlen(pszStr);
    		m_iLength = nBaseLen * sizeof(_TCHAR);
    		if ( m_iLength )
    		{
    			m_pchData = new _TCHAR ;
    			if (0 == MultiByteToWideChar(
    				CP_ACP,					// code page
    				MB_COMPOSITE | MB_ERR_INVALID_CHARS,// character translations
    				pszStr,					// address of string to map
    				nBaseLen,				// size of that non-UNICODE string
    				m_pchData,				// address of wide-character buffer
    				m_iLength				// size of buffer
    				))
    			{	// -- don't much care about the error, just is there --
    				Init();
    			}
    		}
    	}
    	return *this;
    }
    #endif
    
    #endif // _AFXDLL
    
    #if 0	// _WINDOWS
    
    int CGString::Load( int id )
    {
    	char szText;
    	int iLen = LoadString( SP_App.m_hInstance, id, szText, sizeof( szText ));
    	Copy( szText );
    	return( iLen );
    }
    
    CGString::CGString( int id )
    {
    	m_pchData = &Str_Nil;
    	m_iLength = 0;
    	Load( id );
    }
    
    #endif