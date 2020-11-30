    // CAssoc.cpp
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
    #include <windows.h>
    #include <stdio.h>
    #include "cstring.h"
    #endif
    #include "cassoc.h"
    
    //***************************************************************************
    // -CValStr
    
    LPCTSTR CValStr::FindName( int iVal ) const
    {
    	int i=0;
    	for ( ; this.m_pszName; i++ )
    	{
    		if ( iVal < this.m_iVal )
    			return( this.m_pszName );
    	}
    	return( this.m_pszName );
    }
    
    //***************************************************************************
    // -CElementDef
    // Describe the elements of a structure/class
    
    const int CElementDef::sm_Lengths =
    {
    	0,	// ELEM_VOID:
    	-1,	// ELEM_CSTRING,
    	-1, // ELEM_STRING,	// Assume max size of REG_SIZE
    	sizeof(bool),	// ELEM_BOOL
    	sizeof(BYTE), // ELEM_BYTE,		// 1 byte.
    	sizeof(BYTE), // ELEM_MASK_BYTE,	// bits in a BYTE
    	sizeof(WORD), // ELEM_WORD,		// 2 bytes
    	sizeof(WORD), // ELEM_MASK_WORD,	// bits in a WORD
    	sizeof(int),  // ELEM_INT,		// Whatever the int size is. 4 i assume
    	sizeof(int),  // ELEM_MASK_INT,
    	sizeof(DWORD), // ELEM_DWORD,		// 4 bytes.
    	sizeof(DWORD), // ELEM_MASK_DWORD,	// bits in a DWORD
    };
    
    int CElementDef::CompareVal( const void * pBase1, const void * pBase2 ) const
    {
    	ASSERT(m_type!=ELEM_VOID);
    	ASSERT(m_offset>=0);
    	switch ( m_type )
    	{
    	case ELEM_STRING:
    		return( strcmpi( (TCHAR*) GetValPtr(pBase1), (TCHAR*) GetValPtr(pBase2) ));
    	case ELEM_CSTRING:
    		return( ((CGString*) GetValPtr(pBase1))->CompareNoCase( *((CGString*) GetValPtr(pBase2)) ));
    	}
    	return( CompareMem( pBase1, pBase2 ));
    }
    
    bool CElementDef::SetValStr( void * pBase, LPCTSTR pszVal ) const
    {
    	// Set the element value as a string.
    	DWORD dwVal = 0;
    	ASSERT(m_offset>=0);
    	void * pValPtr = GetValPtr(pBase);
    	switch ( m_type )
    	{
    	case ELEM_VOID:
    		DEBUG_CHECK(0);
    		return( false );
    	case ELEM_STRING:
    		strcpylen( (TCHAR*) pValPtr, pszVal, GetValLength()-1 );
    		return( true );
    	case ELEM_CSTRING:
    		*((CGString*)pValPtr) = pszVal;
    		return true;
    	case ELEM_BOOL:
    	case ELEM_BYTE:
    	case ELEM_WORD:
    	case ELEM_INT: // signed ?
    	case ELEM_DWORD:
    		dwVal = Exp_GetVal( pszVal );
    		memcpy( pValPtr, &dwVal, GetValLength());
    		return true;
    	case ELEM_MASK_BYTE:	// bits in a BYTE
    	case ELEM_MASK_WORD:	// bits in a WORD
    	case ELEM_MASK_INT:
    	case ELEM_MASK_DWORD:	// bits in a DWORD
    		DEBUG_CHECK(0);
    		return( false );
    	}
    
    	DEBUG_CHECK(0);
    	return( false );
    }
    
    bool CElementDef::GetValStr( const void * pBase, CGString & sVal ) const
    {
    	// Get the element value as a string.
    
    	DWORD dwVal = 0;
    	ASSERT(m_offset>=0);
    	void * pValPtr = GetValPtr(pBase);
    	switch ( m_type )
    	{
    	case ELEM_VOID:
    		DEBUG_CHECK(0);
    		return( false );
    	case ELEM_STRING:
    		sVal = (TCHAR*) pValPtr;
    		return( true );
    	case ELEM_CSTRING:
    		sVal = *((CGString*)pValPtr);
    		return true;
    	case ELEM_BOOL:
    	case ELEM_BYTE:
    	case ELEM_WORD:
    	case ELEM_INT: // signed ?
    	case ELEM_DWORD:
    		memcpy( &dwVal, pValPtr, GetValLength());
    		sVal.Format( _TEXT("%u"), dwVal );
    		return true;
    	case ELEM_MASK_BYTE:	// bits in a BYTE
    	case ELEM_MASK_WORD:	// bits in a WORD
    	case ELEM_MASK_INT:
    	case ELEM_MASK_DWORD:	// bits in a DWORD
    		DEBUG_CHECK(0);
    		return( false );
    	}
    
    	DEBUG_CHECK(0);
    	return( false );
    }
    
    void CElementDef::Serialize( CGFile & file, void * pBase )
    {
    	// Move the element. Read or Write.
    	ASSERT(m_type!=ELEM_VOID);
    
    	ASSERT(m_offset>=0);
    	void * pValPtr = GetValPtr(pBase);
    
    	if ( file.IsWriteMode())
    	{
    		WORD iLength;
    		switch ( m_type )
    		{
    		case ELEM_STRING:
    			{
    				const TCHAR * pStr = (LPCTSTR) pValPtr;
    				iLength = strlen(pStr);
    				file.Write( &iLength, sizeof(iLength));
    				file.Write( pStr, iLength );
    			}
    			return;
    		case ELEM_CSTRING:
    			{
    				const CGString* pStr = (const CGString*) pValPtr;
    				bool fRet = pStr->WriteTo( &file );
    			}
    			return;
    		}
    
    		iLength = GetValLength();
    		ASSERT( iLength > 0 );
    		file.Write( pValPtr, iLength );
    	}
    	else
    	{
    		WORD iLength;
    		switch ( m_type )
    		{
    		case ELEM_STRING:	// unknown max size !? assume its ok.
    			{
    				TCHAR * pStr = (TCHAR*) pValPtr;
    				file.Read( &iLength, sizeof(iLength));
    				file.Read( pStr, iLength );
    			}
    			return;
    		case ELEM_CSTRING:
    			{
    				CGString* pStr = (CGString*) pValPtr;
    				iLength = pStr->ReadFrom( &file, 0x7fff );
    			}
    			return;
    		}
    
    		iLength = GetValLength();
    		ASSERT( iLength > 0 );
    		file.Read( pValPtr, iLength );
    	}
    }
    
    //***************************************************************************
    
    #if 0
    
    const _TCHAR * CAssocStr::Find( DWORD dwVal ) const
    {
    	for ( int i=0; this.m_pszLabel != NULL; i++ )
    	{
    		if ( this.m_dwVal == dwVal )
    			break;
    	}
    	return( this.m_pszLabel );
    }
    
    const _TCHAR * CAssocStr::Find( _TCHAR * pszOut, DWORD dwVal, const _TCHAR * pszDef ) const
    {
    	//
    	// PURPOSE:
    	//  Find a description string for the index (wValue) provided.
    	//  Pak the result string into (pszOut).
    	//  if no value found then use the (pszDef) string and sprintf to make default
    	//
    
    	const _TCHAR * pszVal = Find( dwVal );
    	if ( pszVal == NULL )
    	{
    		pszVal = pszDef;
    	}
    	if ( pszVal != NULL )
    	{
    		if ( pszOut != NULL )
    		{
    			wsprintf( pszOut, pszVal, dwVal );   // Not here so use default.
    			return( pszOut );
    		}
    	}
    	return( pszVal );
    }
    
    #endif