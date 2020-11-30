    // CString.h
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #ifndef _INC_CSTRING_H
    #define _INC_CSTRING_H
    
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    #include "common.h"
    
    #define SCRIPT_MAX_LINE_LEN 4096    // default size.
    
    class CFile;
    
    #ifndef _WIN32
    // No portable UNIX equiv to this.
    extern void _strupr( TCHAR * pszStr );
    extern void _strlwr( TCHAR * pszStr );
    #endif
    
    #ifdef _AFXDLL      // Only need minimal sub-set if using MFC.
    
    struct CGString : public CString
    {
    public:
        bool IsValid() const
        {
                return( true );
        }
        void Copy( LPCTSTR pszText )
        {
                // ...
                ASSERT(0);
        }
        CGString() {}
        CGString( LPCTSTR pszText ) :
                CString( pszText )
        {
        }
        // NOTE: GetBuffer acts very different here !
        void FormatVal( long iVal )
        {
                Format( _TEXT("%d"), iVal );
        }
        void FormatHex( DWORD dwVal )
        {
                Format( _TEXT("0%x"), dwVal );
        }
    
        int   ReadFrom( CFile * pFile, int iLenMax );
        bool  WriteTo( CFile * pFile ) const;
    };
    
    #else       // _AFXDLL
    
    class CGString
    {
    private:
        TCHAR * m_pchData;      // This cannot be just zeroed !
        static const TCHAR sm_Nil;              // Use this instead of null.
        int m_iLength;
    
    private:
        void Init( void )
        {
                m_pchData = (TCHAR*)&sm_Nil;
                m_iLength = 0;
        }
    protected:
    #if 1 // def _DEBUG
        operator const CGString() const
        {
                ASSERT(0);
                return( *this );
        }
        operator CGString()
        {
                ASSERT(0);
                return( *this );
        }
    #endif
        void ReleaseBuffer() {}
    public:
    
        bool IsValid() const
        {
    #if defined(_DEBUG) && defined(DEBUG_VALIDATE_ALLOC)
                if ( m_pchData != &sm_Nil )
                {
                        DEBUG_ValidateAlloc( m_pchData );
                }
    #endif
                return( m_pchData == '\0' );
        }
        TCHAR * GetBuffer( void )
        {
                ASSERT( IsValid());
                return( m_pchData );
        }
        TCHAR * GetBuffer( int iMinLength )
        {
                if ( iMinLength > GetLength())
                        SetLength( iMinLength );
                return( GetBuffer() );
        }
        int SetLength( int iLen );
    
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
    
        TCHAR & ReferenceAt(int nIndex)       // 0 based
        {
                ASSERT( IsValid());
                ASSERT( nIndex < m_iLength );
                return m_pchData;
        }
        TCHAR GetAt(int nIndex) const      // 0 based
        {
                ASSERT( IsValid());
                ASSERT( nIndex <= m_iLength );  // allow to get the null char
                return( m_pchData );
        }
        void SetAt(int nIndex, TCHAR ch)
        {
                ASSERT( IsValid());
                ASSERT( nIndex < m_iLength );
                m_pchData = ch;
        }
        LPCTSTR GetPtr( void ) const
        {
                ASSERT( IsValid());
                return( m_pchData );
        }
        void Copy( LPCTSTR pStr );
        int CopyTo( TCHAR * pStr ) const
        {
                strcpy( pStr, GetPtr() );
                return( GetLength());
        }
        void FormatV( LPCTSTR pStr, va_list args );
        void _cdecl Format( LPCTSTR pStr, ... )
        {
                va_list vargs;
                va_start( vargs, pStr );
                FormatV( pStr, vargs );
                va_end( vargs );
        }
        void FormatVal( long iVal )
        {
                Format( _TEXT("%d"), iVal );
        }
        void FormatHex( DWORD dwVal )
        {
                Format( _TEXT("0%x"), dwVal );
        }
        int Compare( LPCTSTR pStr ) const
        {
                ASSERT( IsValid());
                return( strcmp( GetPtr(), pStr ));
        }
        int CompareNoCase( LPCTSTR pStr ) const
        {
                ASSERT( IsValid());
                return( strcmpi( GetPtr(), pStr ));
        }
        void Add( TCHAR ch );
        void Add( LPCTSTR pszStr );
        void MakeUpper()
        {
                _strupr(m_pchData);
        }
        void MakeLower()
        {
                _strlwr(m_pchData);
        }
    
        int   ReadZ( CFile * pFile, int iLenMax );
        bool  WriteZ( CFile * pFile );
    
        int   ReadFrom( CFile * pFile, int iLenMax );
        bool  WriteTo( CFile * pFile ) const;
    
    #ifdef UNICODE
        void Copy( const char * pStr );
        const CGString& operator=( const char * lpsz )
        {
                Copy( lpsz );
                return( *this );
        }
    #endif
    
        TCHAR operator(int nIndex) const // same as GetAt
        {
                return GetAt( nIndex );
        }
        TCHAR & operator(int nIndex)
        {
                return ReferenceAt(nIndex);
        }
        operator LPCTSTR() const       // as a C string
        {
                return( GetPtr());
        }
        const CGString& operator+=(LPCTSTR psz) // like strcat
        {
                Add(psz);
                return( *this );
        }
        const CGString& operator+=(TCHAR ch)
        {
                Add(ch);
                return( *this );
        }
        const CGString& operator=( LPCTSTR pStr )
        {
            Copy( pStr );
                return( *this );
        }
        const CGString& operator=( const CGString &s )
        {
                Copy( s.GetPtr());
                return( *this );
        }
    
        ~CGString( void )
        {
            Empty();
        }
        CGString()
        {
                Init();
        }
        CGString( LPCTSTR pStr )
        {
            Init();
                Copy( pStr );
        }
        CGString( const CGString &s )
        {
            Init();
                Copy( s.GetPtr());
        }
    };
    
    #endif // ! _AFXDLL
    
    enum MATCH_TYPE                     // match result defines
    {
        MATCH_INVALID = 0,
        MATCH_VALID,            // valid match
        MATCH_END,                      // premature end of pattern string
        MATCH_ABORT,            // premature end of text string
        MATCH_RANGE,            // match failure on  construct
        MATCH_LITERAL,          // match failure on literal match
        MATCH_PATTERN,          // bad pattern
    };
    extern MATCH_TYPE Str_Match( LPCTSTR pPattern, LPCTSTR pText );
    
    extern int strcpylen( TCHAR * pDst, LPCTSTR pSrc );
    extern int strcpylen( TCHAR * pDst, LPCTSTR pSrc, int imaxlen );
    
    extern LPCTSTR Str_GetArticleAndSpace( LPCTSTR pszWords );
    extern TCHAR * Str_GetTemp();
    extern int Str_GetBare( TCHAR * pszOut, LPCTSTR pszInp, int iMaxSize, LPCTSTR pszStrip = NULL );
    extern TCHAR * Str_MakeFiltered( TCHAR * pStr );
    extern void Str_MakeUnFiltered( TCHAR * pStrOut, LPCTSTR pStrIn, int iSizeMax );
    extern TCHAR * Str_GetNonWhitespace( LPCTSTR pStr );
    extern int Str_TrimEndWhitespace( TCHAR * pStr, int len );
    extern TCHAR * Str_TrimWhitespace( TCHAR * pStr );
    extern bool Str_Parse( TCHAR * pLine, TCHAR ** ppArg = NULL, LPCTSTR pSep = NULL );
    extern int Str_ParseCmds( TCHAR * pCmdLine, TCHAR ** ppCmd, int iMax, LPCTSTR pSep = NULL );
    extern int Str_ParseCmds( TCHAR * pCmdLine, int * piCmd, int iMax, LPCTSTR pSep = NULL );
    
    extern int FindTable( LPCTSTR pFind, LPCTSTR const * ppTable, int iCount, int iElemSize = sizeof(LPCTSTR));
    extern int FindTableSorted( LPCTSTR pFind, LPCTSTR const * ppTable, int iCount, int iElemSize = sizeof(LPCTSTR));
    extern int FindTableHead( LPCTSTR pFind, LPCTSTR const * ppTable, int iCount, int iElemSize = sizeof(LPCTSTR));
    extern int FindTableHeadSorted( LPCTSTR pFind, LPCTSTR const * ppTable, int iCount, int iElemSize = sizeof(LPCTSTR));