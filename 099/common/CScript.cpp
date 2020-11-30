    // CScript.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    // NOTE: all scripts should be encoded in UTF-8.
    // So they may have full unicode chars inside.
    
    #ifdef GRAY_SVR
    #include "../graysvr/graysvr.h"
    #elif defined(GRAY_MAP)
    #include "../graymap/stdafx.h"
    #include "../graymap/graymap.h"
    #elif defined(GRAY_AGENT)
    #include "../grayagent/stdafx.h"
    #include "../grayagent/grayagent.h"
    #else
    #include "graycom.h"
    #endif
    
    ///////////////////////////////////////////////////////////////
    // -CScriptKey
    
    void CScriptKey::InitKey()
    {
    	m_pszArg = m_pszKey = NULL;
    }
    
    TCHAR * CScriptKey::GetArgStr()	// this could be a quoted string ?
    {
    	ASSERT(m_pszKey);
    
    	TCHAR * pStr = GetArgRaw();
    	if ( *pStr != '"' )
    		return( pStr );
    
    	pStr++;
    	TCHAR * pEnd = strchr( pStr, '"' );
    	if ( pEnd )
    	{
    		*pEnd = '\0';
    	}
    
    	return( pStr );
    }
    
    DWORD CScriptKey::GetArgFlag( DWORD dwStart, DWORD dwMask )
    {
    	// No args = toggle the flag.
    	// 1 = set the flag.
    	// 0 = clear the flag.
    
    	ASSERT(m_pszKey);
    	ASSERT(m_pszArg);
    
    	if ( ! HasArgs())
    		return( dwStart ^ dwMask );
    
    	else if ( GetArgVal())
    		return( dwStart | dwMask );
    
    	else
    		return( dwStart &~ dwMask );
    }
    
    long CScriptKey::GetArgVal()
    {
    	ASSERT(m_pszKey);
    	ASSERT(m_pszArg);
    	return( Exp_GetVal( m_pszArg ));
    }
    
    long CScriptKey::GetArgRange()
    {
    	ASSERT(m_pszKey);
    	ASSERT(m_pszArg);
    #ifndef GRAY_CLIENT
    	return( Exp_GetRange( m_pszArg ));
    #else
    	return( Exp_GetRange( m_pszArg ));
    #endif
    }
    
    ///////////////////////////////////////////////////////////////
    // -CScriptKeyAlloc
    
    TCHAR * CScriptKeyAlloc::GetKeyBufferRaw( int iLen )
    {
    	// iLen = length of the string we want to hold.
    
    	ASSERT( iLen >= 0 );
    
    	if ( iLen > SCRIPT_MAX_LINE_LEN )
    		iLen = SCRIPT_MAX_LINE_LEN;
    	iLen ++;	// add null.
    
    	if ( m_Mem.GetDataLength() < iLen )
    	{
    		m_Mem.Alloc( iLen );
    	}
    
    	m_pszKey = m_pszArg = GetKeyBuffer();
    	m_pszKey = '\0';
    
    	return m_pszKey;
    }
    
    bool CScriptKeyAlloc::ParseKey( LPCTSTR pszKey )
    {
    	// Skip leading white space 
    	if ( ! pszKey )
    	{
    		GetKeyBufferRaw(0);
    		return false;
    	}
    
    	GETNONWHITESPACE( pszKey );
    
    	TCHAR * pBuffer = GetKeyBufferRaw( strlen( pszKey ));
    	ASSERT(pBuffer);
    
    	int iLen = m_Mem.GetDataLength()-1;
    	strncpy( pBuffer, pszKey, iLen );
    	pBuffer = '\0';
    
    	Str_Parse( pBuffer, &m_pszArg, "=, \t(){}" );
    	return( true );
    }
    
    bool CScriptKeyAlloc::ParseKey( LPCTSTR pszKey, LPCTSTR pszVal )
    {
    	ASSERT(pszKey);
    
    	int lenkey = strlen( pszKey );
    	if ( ! lenkey )
    	{
    		return ParseKey(pszVal);
    	}
    
    	ASSERT( lenkey < SCRIPT_MAX_LINE_LEN-2 );
    
    	int lenval = 0;
    	if ( pszVal )
    	{
    		lenval = strlen( pszVal );
    	}
    
    	m_pszKey = GetKeyBufferRaw( lenkey + lenval + 1 );
    
    	strcpy( m_pszKey, pszKey );
    	m_pszArg = m_pszKey + lenkey;
    
    	if ( pszVal )
    	{
    		m_pszArg ++;
    		lenval = m_Mem.GetDataLength()-2;
    		strcpylen( m_pszArg, pszVal, ( lenval - lenkey ) + 1 );	// strcpylen
    	}
    
    	return( true );
    }
    
    int CScriptKeyAlloc::ParseKeyEnd()
    {
    	// Now parse the line for comments and trailing whitespace junk
    	// NOTE: leave leading whitespace for now.
    
    	ASSERT(m_pszKey);
    
    	int len = 0;
    	for ( ; len<SCRIPT_MAX_LINE_LEN; len++ )
    	{
    		TCHAR ch = m_pszKey;
    		if ( ch == '\0' )
    			break;
    		if ( ch == '/' && m_pszKey == '/' )
    		{
    			// Remove comment at end of line.
    			break;
    		}
    	}
    
    	// Remove CR and LF from the end of the line.
    	len = Str_TrimEndWhitespace( m_pszKey, len );
    	if ( len <= 0 )		// fRemoveBlanks &&
    		return 0;
    
    	m_pszKey = '\0';
    	return( len );
    }
    
    void CScriptKeyAlloc::ParseKeyLate()
    {
    	ASSERT(m_pszKey);
    	ParseKeyEnd();
    	GETNONWHITESPACE( m_pszKey );
    	Str_Parse( m_pszKey, &m_pszArg );
    }
    
    ///////////////////////////////////////////////////////////////
    // -CScript
    
    CScript::CScript()
    {
    	InitBase();
    }
    
    CScript::CScript( LPCTSTR pszKey )
    {
    	InitBase();
    	ParseKey(pszKey);
    }
    
    CScript::CScript( LPCTSTR pszKey, LPCTSTR pszVal )
    {
    	InitBase();
    	ParseKey( pszKey, pszVal );
    }
    
    void CScript::InitBase()
    {
    	m_iLineNum = 0;
    	m_fSectionHead = false;
    	m_lSectionData = 0;
    	InitKey();
    }
    
    bool CScript::Open( LPCTSTR pszFilename, UINT wFlags )
    {
    	// If we are in read mode and we have no script file.
    	// ARGS: wFlags = OF_READ, OF_NONCRIT etc
    	// RETURN: true = success.
    
    	InitBase();
    
    	if ( pszFilename == NULL )
    	{
    		pszFilename = GetFilePath();
    	}
    	else
    	{
    		SetFilePath( pszFilename );
    	}
    
    	LPCTSTR pszTitle = GetFileTitle();
    	if ( pszTitle == NULL || pszTitle == '\0' )
    		return( false );
    
    	LPCTSTR pszExt = GetFilesExt( GetFilePath() ); 
    	if ( pszExt == NULL )
    	{
    		TCHAR szTemp;
    		strcpy( szTemp, GetFilePath() );
    		strcat( szTemp, GRAY_SCRIPT );
    		SetFilePath( szTemp );
    		wFlags |= OF_TEXT;
    	}
    
    	if ( ! CFileText::Open( GetFilePath(), wFlags ))
    	{
    		if ( ! ( wFlags & OF_NONCRIT ))
    		{
    			g_pLog->Event( LOGL_WARN, "'%s' not found...\n", (LPCTSTR) GetFilePath() );
    		}
    		return( false );
    	}
    
    	return( true );
    }
    
    bool CScript::ReadTextLine( bool fRemoveBlanks ) // Read a line from the opened script file
    {
    	// ARGS:
    	// fRemoveBlanks = Don't report any blank lines, (just keep reading)
    	//
    
    	ASSERT( ! IsBinaryMode());
    
    	while ( CFileText::ReadString( GetKeyBufferRaw(SCRIPT_MAX_LINE_LEN), SCRIPT_MAX_LINE_LEN ))
    	{
    		m_iLineNum++;
    		if ( fRemoveBlanks )
    		{
    			if ( ParseKeyEnd() <= 0 )
    				continue;
    		}
    		return( true );
    	}
    
    	m_pszKey = '\0';
    	return( false );
    }
    
    bool CScript::FindTextHeader( LPCTSTR pszName ) // Find a section in the current script
    {
    	// RETURN: false = EOF reached.
    	ASSERT(pszName);
    	ASSERT( ! IsBinaryMode());
    
    	SeekToBegin();
    
    	int len = strlen( pszName );
    	ASSERT(len);
    	do
    	{
    		if ( ! ReadTextLine(false))
    		{
    			return( false );
    		}
    		if ( IsKeyHead( "", 5 ))
    		{
    			return( false );
    		}
    	}
    	while ( ! IsKeyHead( pszName, len ));
    	return( true );
    }
    
    LONG CScript::Seek( long offset, UINT origin )
    {
    	// Go to the start of a new section.
    	// RETURN: the new offset in bytes from start of file.
    	if ( offset == 0 && origin == SEEK_SET )
    	{
    		m_iLineNum = 0;	// so we don't have to override SeekToBegin
    	}
    	m_fSectionHead = false;		// unknown , so start at the beginning.
    	m_lSectionData = offset;
    	return( CFileText::Seek(offset,origin));
    }
    
    #if 0
    
    void CScript::WriteBinLength( DWORD dwLen )
    {
    	do
    	{
    		BYTE bLen = dwLen & 0x7F;
    		dwLen >>= 7;
    		if ( dwLen )
    			bLen |= 0x80;
    		Write( &bLen, 1 );
    	} while ( dwLen );
    }
    
    DWORD CScript::ReadBinLength()
    {
    	DWORD dwLen = 0;
    	int i=0;
    	while (true)
    	{
    		BYTE bLen;
    		if ( Read( &bLen, 1 ) != 1 )
    			return( 0xFFFFFFFF );
    		dwLen |= bLen;
    		if ( ! ( bLen & 0x80 ))
    			break;
    		if ( ++i >= 4 )
    			break;
    		dwLen <<= 7;
    	}
    	return( dwLen );
    }
    
    int CScript::DeCompressBin( int iLen )
    {
    #if 0
    	iLen = CClient::xDeCompress( CClient::xCompress_Buffer, (const BYTE*) m_pszKey, iLen );
    	if ( iLen <= 0 )
    		return( -1 );
    	m_pszKey = m_pBuffer;
    	ASSERT(m_pszKey);
    	memcpy( m_pszKey, CClient::xCompress_Buffer, iLen );
    	m_pszKey = '\0';
    #endif
    	return( iLen );
    }
    
    int CScript::CompressBin( int iLen )
    {
    #if 0
    	iLen = CClient::xCompress( CClient::xCompress_Buffer, (const BYTE*) GetPtr(), iLen );
    	if ( iLen <= 0 )
    		return( -1 );
    	m_pszKey = m_pBuffer;
    	ASSERT(m_pszKey);
    	memcpy( m_pszKey, CClient::xCompress_Buffer, iLen );
    	m_pszKey = '\0';
    #endif
    	return( iLen );
    }
    
    void CScript::EndSection()
    {
    	// End any previous section.
    }
    
    #endif
    
    bool CScript::FindNextSection( void )
    {
    	// RETURN: false = EOF.
    
    	if ( m_fSectionHead )	// we have read a section already., (not at the start)
    	{
    		// Start from the previous line. It was the line that ended the last read.
    		m_pszKey = GetKeyBuffer();
    		ASSERT(m_pszKey);
    		m_fSectionHead = false;
    		if ( m_pszKey == ' == ' == ']' )
    		{
    			m_pszKey = '\0';
    			break;
    		}
    	}
    
    	m_lSectionData = GetPosition();
    	if ( IsSectionType( "EOF" ))
    		return( false );
    
    	Str_Parse( m_pszKey, &m_pszArg );
    	return( true );
    }
    
    bool CScript::FindSection( LPCTSTR pszName, UINT uModeFlags )
    {
    	// Find a section in the current script
    	// RETURN: true = success
    
    	ASSERT(pszName);
    	if ( strlen( pszName ) > 32 )
    	{
    		DEBUG_ERR(( "Bad script section name\n" ));
    		return( false );
    	}
    
    	CGString sSec;
    	sSec.Format( "", (LPCTSTR) pszName );
    	if ( FindTextHeader( sSec ))
    	{
    		// Success
    		m_lSectionData = GetPosition();
    		return( true );
    	}
    
    	// Failure Error display. (default)
    
    	if ( ! ( uModeFlags & OF_NONCRIT ))
    	{
    		g_pLog->Event( LOGL_WARN, "Did not find '%s' section '%s'\n", (LPCTSTR) GetFileTitle(), (LPCTSTR) pszName );
    	}
    	return( false );
    }
    
    bool CScript::ReadKey( bool fRemoveBlanks )
    {
    	if ( ! ReadTextLine(fRemoveBlanks))
    		return( false );
    	if ( m_pszKey == ' == ', iLenSecName );
    					if ( fFoundSection && pszKey == NULL )
    					{
    						lOffsetWhiteSpace = 0;
    						continue;	// delete whole section.
    					}
    				}
    
    				lOffsetWhiteSpace = 0;
    			}
    
    			else
    			{
    
    				if ( ! fFoundSection && lSectionOffset )
    				{
    					fFoundSection = ( GetPosition() >= lSectionOffset );
    				}
    
    				if ( fFoundSection )
    				{
    					if ( pszKey == NULL )
    						continue;	// delete whole section.
    					if ( ! strnicmp( pszKey, m_pszKey, strlen( pszKey )))
    					{
    						fFoundKey = true;
    						if ( pszVal == NULL )
    							continue;	// just lose this key
    						pDst->Printf( "%s=%s\n", pszKey, pszVal );
    						continue;	// replace this key.
    					}
    
    					// Is this just white space ?
    					LPCTSTR pNonWhite = m_pszKey;
    					while ( ISWHITESPACE( pNonWhite ))
    						pNonWhite ++;
    					if ( pNonWhite != '\0' )
    					{
    						lOffsetWhiteSpace = 0;
    					}
    					else if ( ! lOffsetWhiteSpace )
    					{
    						lOffsetWhiteSpace = pDst->GetPosition();
    					}
    				}
    			}
    		}
    
    		// Just copy the old stuff.
    		if ( ! pDst->WriteString( m_pszKey ))
    		{
    			DEBUG_ERR(( "Profile Failed to write line\n" ));
    			return false;
    		}
    	}
    
    	if ( ! m_iLineNum )
    	{
    		// Failed to read anything !
    		DEBUG_ERR(( "Profile Failed to read script\n" ));
    		return false;
    	}
    
    	if ( ! fFoundSection && pszSection && pszVal && pszKey )
    	{
    		// Create the section ?
    		if ( pDst->Printf( "\n", pszSection ) <= 0 )
    			goto cantwrite;
    
    		if ( pDst->Printf( "%s=%s\n", pszKey, pszVal ) <= 0 )
    		{
    cantwrite:
    			DEBUG_ERR(( "Profile Failed to write new line\n" ));
    			return false;
    		}
    	}
    
    	return( true );
    }
    
    bool CScript::WriteProfileStringBase( LPCTSTR pszSection, long lSectionOffset, LPCTSTR pszKey, LPCTSTR pszVal )
    {
    	// Assume the script is open in read mode.
    	// Key = NULL = delete the section.
    	// Val = NULL = delete the key.
    	// RETURN:
    	//  true = it worked.
    	//
    
    	if ( IsBinaryMode())
    	{
    		DEBUG_ERR(( "Profile Can't write binary SCP\n" ));
    		return( false );
    	}
    
    	bool fWasOpen = IsFileOpen();
    	if ( ! fWasOpen )
    	{
    		// Might be creating a new file. not normal !
    		if ( ! Open( NULL, OF_READ | OF_TEXT ))
    			return( false );
    	}
    	else
    	{
    		ASSERT( ! IsWriteMode());
    	}
    
    #if 0 // def _WIN32
    	Close();
    
    	if ( pszSection == NULL )
    		return( false );
    
    	// Use Windows version.
    	if ( ! ::WritePrivateProfileString(
    		pszSection,
    		pszKey,
    		pszVal,
    		GetFilePath()))
    	{
    		DEBUG_ERR(( "Profile Can't WritePrivateProfileString\n" ));
    		return( false );
    	}
    
    #else
    
    	// Open the write side of the profile.
    	CScript s;
    	if ( ! s.Open( "tmpout.ini", OF_WRITE | OF_TEXT ))
    	{
    		DEBUG_ERR(( "Profile Can't open tmp.ini\n" ));
    		return( false );
    	}
    
    	if ( ! WriteProfileStringDst( &s, pszSection, lSectionOffset, pszKey, pszVal ))
    		return( false );
    
    	// Close, rename and re-open the file.
    	s.Close();
    	Close();
    	CloseForce();	// some will linger open !
    	if ( remove( GetFilePath()))
    	{
    		DEBUG_ERR(( "Profile remove fail\n" ));
    		return(false);
    	}
    	if ( rename( s.GetFilePath(), GetFilePath()))
    	{
    		DEBUG_ERR(( "Profile rename fail\n" ));
    		return(false);
    	}
    #endif
    
    	if ( fWasOpen )
    	{
    		if ( ! Open())
    		{
    			DEBUG_ERR(( "Profile src reopen fail\n" ));
    			return(false);
    		}
    	}
    	return( true );
    }
    
    void CScript::Close()
    {
    	// EndSection();
    	CFileText::Close();
    }
    
    bool _cdecl CScript::WriteSection( LPCTSTR pszSection, ... )
    {
    	// Write out the section header.
    	va_list vargs;
    	va_start( vargs, pszSection );
    
    	// EndSection();	// End any previous section.
    	Printf( "\n\n" );
    	va_end( vargs );
    
    	return( true );
    }
    
    bool CScript::WriteKey( LPCTSTR pszKey, LPCTSTR pszVal )
    {
    	if ( pszKey == NULL || pszKey == '\0' )
    	{
    		return false;
    	}
    	if ( pszVal == NULL || pszVal == '\0' )
    	{
    		// Books are like this. No real keys.
    		Printf( "%s\n", pszKey );
    	}
    	else
    	{
    		Printf( "%s=%s\n", pszKey, pszVal );
    	}
    	return( true );
    }
    
    void _cdecl CScript::WriteKeyFormat( LPCTSTR pszKey, LPCTSTR pszVal, ... )
    {
    	va_list vargs;
    	va_start( vargs, pszVal );
    	CGString sTmp;
    	sTmp.FormatV( pszVal, vargs );
    	WriteKey( pszKey, sTmp );
    	va_end( vargs );
    }