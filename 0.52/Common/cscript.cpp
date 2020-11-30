//
// CScript.cpp
// Copyright Menace Software (www.menasoft.com).
//
// NOTE: all scripts should be encoded in UTF-8. 
// So they may have full unicode chars inside.

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#else
#include "graycom.h"
#endif

int CScriptObj::sm_iTrigArg;			// a modifying arg to the current trigger.

///////////////////////////////////////////////////////////////
// -CScript

CScript::CScript( const TCHAR * pszKey, const TCHAR * pVal )
{
	Init();
	int lenkey = strlen( pszKey );
	int lenval = pVal ? strlen( pVal ) : 0;
	ASSERT( lenkey+lenval+1 < MAX_SCRIPT_LINE_LEN );
	m_pArg = m_Buffer.GetBuffer( MAX_SCRIPT_LINE_LEN );	// lenkey+lenval+1
	strcpy( m_pArg, pszKey );
	m_pArg += lenkey+1;
	if ( pVal ) strcpy( m_pArg, pVal );
}

#if defined(GRAY_SVR)

bool CScript::OpenFind( const TCHAR *pszFilename, WORD wFlags )
{
	// search the local dir or full path first.
	if ( Open( pszFilename, wFlags | OF_NONCRIT ))
		return( true );

	const TCHAR * pStart = strrchr( pszFilename, '\\' );
	pStart = ( pStart ) ? ( pStart + 1 ) : pszFilename;

	// search the world save file path.
	CGString sPathName = GetMergedFileName( g_Serv.m_sWorldBaseDir, pStart );

	if ( Open( sPathName, wFlags | OF_NONCRIT ))
		return( true );

	// search the script file path.
	sPathName = GetMergedFileName( g_Serv.m_sSCPBaseDir, pStart );
	return( Open( sPathName, wFlags ));
}

#endif	// GRAY_SVR

bool CScript::OpenCopy( const CScript & s, WORD wFlags )
{
	DEBUG_CHECK( s.IsFileOpen());
	Init();
	if ( ! CGFile::OpenCopy( s, wFlags ))
		return( false );
	m_Buffer.SetLength( MAX_SCRIPT_LINE_LEN );
	return( true );
}

bool CScript::Open( const TCHAR *pszFilename, WORD wFlags )
{
	// If we are in read mode and we have no script file.
	// Serv.m_fSaveBinary
	// RETURN: true = success.

	Init();
	m_Buffer.SetLength( MAX_SCRIPT_LINE_LEN );

	if ( pszFilename == NULL )
		pszFilename = GetFilePath();
	const TCHAR * pStart = strrchr( pszFilename, '\\' );
	if ( pStart == NULL ) 
		pStart = pszFilename;
	const TCHAR * pExt = strrchr( pStart, '.' );
	if ( pExt == NULL )
	{
		// No extension so try binary first.
		TCHAR szTemp[ _MAX_PATH ];
		strcpy( szTemp, pszFilename );
		strcat( szTemp, GRAY_BINARY );
		if ( CFileText::Open( szTemp, wFlags | OF_BINARY ))
			return( true );
		strcpy( szTemp, pszFilename );
		strcat( szTemp, GRAY_SCRIPT );
		SetFilePath( szTemp );
		pszFilename = GetFilePath();
		wFlags |= OF_TEXT;
	}
	
	if ( ! CFileText::Open( pszFilename, wFlags ))
	{
		if ( ! ( wFlags & OF_NONCRIT ))
		{
			g_pLog->Event( LOGL_WARN, "'%s' not found...\n", pszFilename );
		}
		return( false );			
	}
	
	return( true );
}

bool CScript::ReadTextLine( bool fRemoveBlanks ) // Read a line from the opened script file
{
	ASSERT( ! IsBinaryMode());
	while (true)
	{
		TCHAR * pData = m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN);
		ASSERT(pData);
		if ( CFileText::ReadLine( pData, MAX_SCRIPT_LINE_LEN ) == NULL )
		{
			pData[0] = '\0';
			return( false );
		}
		m_iLineNum++;

		// Now parse the line for comments and leading and trailing whitespace junk
		int len = 0;
		for ( ; len<MAX_SCRIPT_LINE_LEN; len++ )
		{
			TCHAR ch = pData[len];
			if ( ch == '\0' ) 
				break;
			if ( ch == '/' && pData[len+1] == '/' ) 
			{
				// Remove comment at end of line.
				break;
			}
		}
		// Remove CR and LF from the end of the line.
		len = TrimEndWhitespace( pData, len );
		if ( fRemoveBlanks && len <= 0 ) 
			continue;
		pData[len] = '\0';
		return( true );
	}
}

bool CScript::FindTextHeader( const TCHAR *pszName, CScriptLink *pPrev, WORD wModeFlags ) // Find a section in the current script
{
	// RETURN: false = EOF reached.
	ASSERT(pszName);
	ASSERT( ! IsBinaryMode());

	if ( pPrev )
	{
		if ( ! SeekLine( pPrev->GetLinkOffset(), pPrev->GetLinkLineNum() ))
			return( false );
	}
	else
	{
		SeekToBegin();
	}

	int len = strlen( pszName );
	ASSERT(len);
	do
	{
		if ( ! ReadTextLine(true))
		{
			return( false );
		}
		if ( IsKeyHead( "[EOF]", 5 ))
		{
			return( false );
		}
		if (( wModeFlags & OF_SORTED ) && ( GetKey()[0] == '[' ))
		{
			if ( strcmpi( pszName, GetKey()) < 0 )
				return( false );	// passed it.
		}
	}
	while ( ! IsKeyHead( pszName, len ));
	return( true );
}

bool CScript::Seek( long offset, int origin )
{
	// Go to the start of a new section.
	// RETURN: 0 = success
	m_lSectionHead = 0;		// unknown
	m_lSectionData = offset;
	return( CFileText::Seek(offset,origin));
}

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
	iLen = CClient::xDeCompress( CClient::xCompress_Buffer, (const BYTE*) m_pArg, iLen );
	if ( iLen <= 0 ) return( -1 );
	m_pArg = GetBuffer( max( iLen+1, MAX_SCRIPT_LINE_LEN ));
	memcpy( m_pArg, CClient::xCompress_Buffer, iLen );
	m_pArg[ iLen ] = '\0';
#endif
	return( iLen );
}

int CScript::CompressBin( int iLen )
{
#if 0
	iLen = CClient::xCompress( CClient::xCompress_Buffer, (const BYTE*) GetPtr(), iLen );
	if ( iLen <= 0 ) return( -1 );
	m_pArg = GetBuffer( max( iLen+1, MAX_SCRIPT_LINE_LEN ));
	memcpy( m_pArg, CClient::xCompress_Buffer, iLen );
	m_pArg[ iLen ] = '\0';
#endif
	return( iLen );
}

bool CScript::FindNextSection( void )
{
	// RETURN: false = EOF.

	if ( IsBinaryMode())
	{
		// Assume we are at the start of a key.
		if ( m_lSectionHead )
		{
			Seek( m_lSectionHead );
		}
		else
		{
			// Find next section header.
			while (true)
			{
				DWORD dwLen = ReadBinLength();
				if ( dwLen == 0xFFFFFFFF )
					return( false );
				if ( dwLen & 0x80000000 )
					break;
				if ( ! CFileText::Seek( dwLen, SEEK_CUR ))	// skip key.
					return( false );
			}
		}

		// Read the section name.
		BYTE bStrLen;
		if ( ! Read( &bStrLen, 1 ))
			return( false );

		m_pArg = m_Buffer.GetBuffer( bStrLen );
		if ( ! Read( m_pArg, bStrLen ))
			return( false );

		int iLen = DeCompressBin( bStrLen );
		if ( iLen <= 0 )
			return( false );
	}
	else
	{
		m_pArg = m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN);
		if ( m_lSectionHead )
		{
			// Start from the previous line. It was the line that ended the last read.
			m_lSectionHead = 0;
			if ( m_pArg[0] == '[' ) 
				goto foundit;
		}

		while (true)
		{
			if ( ! ReadTextLine(true)) 
				return( false );
			if ( m_pArg[0] == '[' )
				break;
		}

foundit:
		// Parse up the section name.
		int len = strlen( m_pArg );
		memmove( &m_pArg[0], &m_pArg[1], len );	len--;
		for ( int i=0; i<len; i++ )
			if ( m_pArg[i] == ']' )
			{
				m_pArg[i] = '\0';
				break;
			}

		if ( IsSectionType( "EOF" )) 
			return( false );
	}

	m_lSectionData = GetPosition();
	Parse( m_pArg, &m_pArg );
	return( true );
}

bool CScript::FindSection( const TCHAR *pszName, WORD wModeFlags, CScriptLink *pPrev )
{
	// Find a section in the current script
	// RETURN: true = success

	ASSERT(pszName);
	if ( strlen( pszName ) > 32 )
	{
		DEBUG_ERR(( "Bad script section name\n" ));
		return( false );
	}

	if ( IsBinaryMode())
	{
		if ( pPrev )
		{
			DEBUG_ERR(( "Binary seek not imp.\n" ));
		}
		else
		{
			SeekToBegin();
		}
	}
	else
	{
		CGString sSec;
		sSec.Format( "[%s]", pszName );
		if ( FindTextHeader( sSec, pPrev, wModeFlags ))
		{
			// Success
			m_lSectionData = GetPosition();
			return( true );
		}
	}

	// Failure Error display. (default)

	if ( ! ( wModeFlags & OF_NONCRIT ))
	{
		g_pLog->Event( LOGL_WARN, "Did not find '%s' section '%s'\n", GetFileTitle(), pszName );
	}
	return( false );
}

bool CScript::ReadKey( bool fRemoveBlanks )
{
	if ( IsBinaryMode()) 
	{
		DWORD dwLen = ReadBinLength();
		if ( dwLen == 0xFFFFFFFF )
			return( false );
		if ( dwLen & 0x80000000 )	// Ran into next section.
		{
			m_lSectionHead = GetPosition();
			m_lSectionLength = dwLen &~ 0x80000000;
			return( false );
		}

		m_pArg = m_Buffer.GetBuffer( dwLen );
		if ( ! Read( m_pArg, dwLen ))
			return( false );

		int iLen = DeCompressBin( dwLen );
		if ( iLen < 0 )
			return( false );

		return( true );
	}
	else
	{
		if ( ! ReadTextLine(fRemoveBlanks))
			return( false );
		m_pArg = m_Buffer.GetBuffer();
		if ( m_pArg[0] == '[' )
		{
			m_lSectionHead = true;
			return( false );
		}
		// ASSERT( m_pArg[0] );
		return( m_pArg[0] != '}' );	// ??? what can we do with this ?
	}
}

bool CScript::ReadKeyParse() // Read line from script
{
	if ( ! ReadKey(true))
	{
		m_pArg = "";
		return( false );	// end of section.
	}
	Parse( m_Buffer.GetBuffer(), &m_pArg );
	return( true );
}

TCHAR * CScript::GetArgNextStr()
{
	Parse( m_pArg, &m_pArg );
	return( m_pArg );
}

long CScript::GetArgVal()
{
	return( Exp_GetVal( m_pArg ));
}

DWORD CScript::GetArgHex()
{
	return( Exp_GetHex( m_pArg ));
}

bool CScript::FindKey( const TCHAR *pszName ) // Find a key in the current section 
{
	if ( strlen( pszName ) > 32 )
	{
		DEBUG_ERR(( "Bad script key name\n" ));
		return( false );
	}
	Seek( m_lSectionData );
	while ( ReadKeyParse())
	{
		if ( IsKey( pszName ))
		{
			m_pArg = TrimWhitespace( m_pArg );
			return true;
		}
	}
	return( false );
}

bool CScript::WriteProfileString( CScript * pDst, const TCHAR * pszSection, const TCHAR * pszKey, const TCHAR * pszVal )
{
	int len = pszSection ? strlen( pszSection ) : 0;
	bool fInSection = false;
	bool fFoundKey = false;

	if ( IsBinaryMode())
	{
		DEBUG_ERR(( "WriteProfileString binary mode\n" ));
		return( false );
	}
	SeekToBegin();

	// Write the header stuff.
	// start writing til we get to the section we want.
	int iLines = 0;
	long lOffsetWhiteSpace = 0;
	while ( 1 )
	{
		TCHAR * pData = m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN);
		if ( CFileText::ReadLine( pData, MAX_SCRIPT_LINE_LEN ) == NULL )
		{
			break;
		}

		iLines++;
		if ( ! fFoundKey )
		{
			// The section we want.

			if ( pData[0] == '[' && pszSection ) 
			{
				bool fNewInSection = ! strnicmp( pszSection, &pData[1], len );
				if ( ! fNewInSection && fInSection )
				{
					if ( ! fFoundKey && pszVal != NULL )
					{
						if ( lOffsetWhiteSpace )	// go back to first whitespace.
						{
							bool fSeek = pDst->Seek( lOffsetWhiteSpace );
							ASSERT( fSeek );
						}
						fFoundKey = true;
						pDst->Printf( "%s=%s\n", pszKey, pszVal );
						if ( lOffsetWhiteSpace )
						{
							pDst->Printf( "\n" );
						}
					}
				}
				fInSection = fNewInSection;
			}
			if ( fInSection )
			{
				if ( pszKey == NULL )
					continue;	// delete whole section.
				if ( ! strnicmp( pszKey, pData, strlen( pszKey )))
				{
					fFoundKey = true;
					if ( pszVal == NULL ) 
						continue;	// just lose this key
					pDst->Printf( "%s=%s\n", pszKey, pszVal );
					continue;	// replace this key.
				}
	
				const TCHAR * pNonWhite = pData;
				while ( ISWHITESPACE( pNonWhite[0] ))
					pNonWhite ++;
				if ( pNonWhite[0] != '\0' )
				{
					lOffsetWhiteSpace = 0;
				}
				else if ( ! lOffsetWhiteSpace )
				{
					lOffsetWhiteSpace = pDst->GetPosition();
				}
			}
		}

		// Just copy the old stuff.
		if ( ! pDst->WriteStr( pData ))
		{
			DEBUG_ERR(( "Profile Failed to write line\n" ));
			return false;
		}
	}

	if ( ! iLines )
	{
		// Failed to read anything !
		DEBUG_ERR(( "Profile Failed to read script\n" ));
		return false;
	}

	if ( ! fFoundKey && pszSection && pszVal && pszKey )
	{
		if ( ! fInSection )	// Create the section ?
		{
			if ( pDst->Printf( "[%s]\n", pszSection ) <= 0 )
				goto cantwrite;
		}
		if ( pDst->Printf( "%s=%s\n", pszKey, pszVal ) <= 0 )
		{
cantwrite:
			DEBUG_ERR(( "Profile Failed to write new line\n" ));
			return false;
		}
	}

	return( true );
}

bool CScript::WriteProfileString( const TCHAR * pszSection, const TCHAR * pszKey, const TCHAR * pszVal )
{
	// Assume the script is open in read mode.
	// Key = NULL = delete the section.
	// Val = NULL = delete the key.
	// RETURN:
	//  true = it worked.
	//

	if ( pszSection == NULL )
		return( false );
	if ( IsBinaryMode()) 
	{
		DEBUG_ERR(( "Profile Can't write binary SCP\n" ));
		return( false );
	}

	if ( ! IsFileOpen())
	{
		// Might be creating a new file. not normal !
		if ( ! Open( NULL, OF_WRITE | OF_TEXT )) 
			return( false );
	}
	else
	{
		ASSERT( ! IsWriteMode());
	}

#if 0 // def _WIN32
	Close();

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
	if ( ! s.Open( "tmpout.ini", OF_WRITE | OF_READ | OF_TEXT ))
	{
		DEBUG_ERR(( "Profile Can't open tmp.ini\n" ));
		return( false );
	}

	if ( ! WriteProfileString( &s, pszSection, pszKey, pszVal ))
		return( false );

	// Close, rename and re-open the file.
	if ( ! s.Close()) 
	{
		DEBUG_ERR(( "Profile dest close fail\n" ));
		return(false);
	}
	if ( ! Close()) 
	{
		DEBUG_ERR(( "Profile src close fail\n" ));
		return(false);
	}
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

	if ( ! Open())
	{
		DEBUG_ERR(( "Profile src reopen fail\n" ));
		return(false);
	}
	return( true );
}

void CScript::EndSection()
{
	// End any previous section.
	if ( IsWriteMode() && IsBinaryMode() && m_lSectionHead )
	{
		// Flush out the section length.
		long lCurrent = GetPosition();
		DWORD dwLen = 0x80000000 + ( lCurrent - m_lSectionHead );
		Seek( m_lSectionHead );
		WriteBinLength( dwLen );
		Seek( lCurrent );
	}
}

bool CScript::Close()
{
	EndSection();
	return CFileText::Close();
}

bool _cdecl CScript::WriteSection( const TCHAR * pszSection, ... )
{
	// Write out the section header.
	va_list args;
	va_start( args, pszSection );

	EndSection();	// End any previous section.

	if ( IsBinaryMode())
	{
		// Lengths are NON-inclusive.
		//
		// DWORD [Full Section length]
		// char  [Section Name]
		// '='
		// BYTE	 [Data]

		m_lSectionHead = GetPosition();
		DWORD dwLen = 0xFFFFFFFF;
		WriteBinLength( dwLen );	// section total length prefix.

		int iLen = m_Buffer.VFormat( pszSection, args );
		iLen = CompressBin( iLen );
		if ( iLen <= 0 || iLen > 128 ) 
			return( false );

		BYTE bLenSection = iLen;
		Write( &bLenSection, 1 );
		Write( m_pArg, bLenSection );
	}
	else
	{
		Printf( "\n[");
		VPrintf( pszSection, args );
		Printf( "]\n" );
	}
	return( true );
}

bool CScript::WriteKey( const TCHAR * pszKey, const TCHAR * pszVal )
{
	if ( pszKey == NULL || pszKey[0] == '\0' )
	{
		return false;
	}
	if ( IsBinaryMode())
	{
		int iLen = m_Buffer.Format( "%s=%s", pszKey, pszVal ? pszVal : "" );

		iLen = CompressBin( iLen );
		if ( iLen <= 0 ) 
			return( false );

		WriteBinLength( iLen );
		Write( m_pArg, iLen );
	}
	else
	{
		if ( pszVal == NULL || pszVal[0] == '\0' )
		{
			// Books are like this. No real keys.
			Printf( "%s\n", pszKey );
		}
		else
		{
			Printf( "%s=%s\n", pszKey, pszVal );
		}
	}
	return( true );
}

void _cdecl CScript::WriteKeyFormat( const TCHAR * pszKey, const TCHAR * pszVal, ... )
{
	va_list args;
	va_start( args, pszVal );
	CGString sTmp;
	sTmp.VFormat( pszVal, args );
	WriteKey( pszKey, sTmp );
}

#ifndef _AFXDLL

//***************************************************************************
//	CScriptLock
//

bool CScriptLock::Open( const TCHAR *pszFilename, WORD uFlags )
{
	if ( ! CScript::Open( pszFilename, uFlags ))
		return( false );

	DEBUG_CHECK( m_lPrvOffset < 0 );

#ifdef GRAY_SVR
	// Assume this is the new context !
	m_pPrvScriptContext = g_Log.SetScriptContext( this );
#endif
	return( true );
}

bool CScriptLock::OpenCopy( const CScript & s, WORD uFlags )
{
	if ( ! s.IsFileOpen())
	{
		return( Open( s.GetFilePath(), uFlags ));
	}

	if ( ! CScript::OpenCopy( s, uFlags ))
		return( false );

	m_lPrvOffset = s.GetPosition();
	DEBUG_CHECK( m_lPrvOffset >= 0 );

#ifdef GRAY_SVR
	// Assume this is the new context !
	m_pPrvScriptContext = g_Log.SetScriptContext( this );
#endif
	return( true );
}

bool CScriptLock::Close()
{
	if ( ! IsFileOpen())
		return false;
#ifdef GRAY_SVR
	// Assume this is the new context !
	g_Log.SetScriptContext( m_pPrvScriptContext );
#endif
	if ( m_lPrvOffset >= 0 )
	{
		SeekPos(m_lPrvOffset);	// no need to set the line number as it should not have changed.
		Init();
		CloseCopy();
	}
	else
	{
		CScript::Close();
	}
	DEBUG_CHECK( ! IsFileOpen());
	return true;
}

#ifdef GRAY_SVR

bool CScriptLock::FindSection( const TCHAR *pszName, WORD wModeFlags, CScriptLink *pPrev )
{
	if ( CScript::FindSection( pszName, wModeFlags|OF_NONCRIT, pPrev ))
		return( true );
	
	if ( ! ( wModeFlags & OF_NONCRIT ))
	{
		g_Log.SetScriptContext( m_pPrvScriptContext );	// This is the fault of the previous section.
		g_pLog->Event( LOGL_WARN, "Did not find '%s' section '%s'\n", GetFileTitle(), pszName );
		g_Log.SetScriptContext( this );
	}
	return( false );
}

#endif // GRAY_SVR

#endif // _AFXDLL

////////////////////////////////////////////////////////////////////////////////////////
// -CScriptObj

int CScriptObj::ParseText( TCHAR * pszResponse, CTextConsole * pSrc, TCHAR chBegin, TCHAR chEnd )
{
	// Take in a line of text that may have fields that can be replaced with operators here.
	// ex. "SPEAK hello there my friend <SRC.NAME> my name is <NAME>"

	// Parsing flags
	bool fBracket = false;	// should this be recursive ?
	int iBegin;

	int i;
	for ( i = 0; pszResponse[i]; i++ )
	{
		TCHAR ch = pszResponse[i];
		if ( ch == chBegin )
		{
			if ( fBracket )	// do a recursive.
			{
				if ( chBegin == chEnd )	// %NAME%
					goto foundend;
				ParseText( pszResponse+i, pSrc, chBegin, chEnd );
			}
			else
			{
				iBegin = i;
				fBracket = true;
			}
		}
		else if ( ch == chEnd && fBracket )
		{
foundend:
			fBracket = false;
			pszResponse[i] = '\0';

			CGString sVal;
			if ( ! r_WriteVal( pszResponse+iBegin+1, sVal, pSrc ))
			{
				if ( chBegin == '%' )
				{
					sVal = "&nbsp";
				}
				else
				{
					sVal.Format( "%c%s%c", chBegin, pszResponse+iBegin+1, chEnd );
				}
			}
			else if ( sVal.IsEmpty() && chBegin == '%' )
			{
				sVal = "&nbsp";
			}

			int len = sVal.GetLength();
			i++;
			memmove( pszResponse+iBegin+len, pszResponse+i, strlen( pszResponse+i ) + 1 );
			memcpy( pszResponse+iBegin, (const TCHAR *) sVal, len );
			i = iBegin+len-1;
		}
	}

	return( i );
}

#ifdef GRAY_SVR

TRIGRET_TYPE CScriptObj::OnTriggerForLoop( CScript &s, int iType, CTextConsole * pSrc )
{
	// loop from start here to the ENDFOR

	int iDist = s.GetArgVal();

	CObjBaseTemplate * pObj = dynamic_cast <CObjBaseTemplate *>(this);
	if ( pObj == NULL )
	{
		iType = 0;
		DEBUG_ERR(( "FOR Loop trigger on non-world object '%s'\n", GetName()));
	}

	CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
	CPointMap pt = pObjTop->GetTopPoint();

	long lStartOffset = s.GetPosition();
	long lStartLine = s.GetLineNumber();
	long lEndOffset = lStartOffset;
	long lEndLine = lStartLine;

	if ( iType & 1 )
	{
		CWorldSearch AreaItems( pt, iDist );
		while(true)
		{
			CItem * pItem = AreaItems.GetItem();
			if ( pItem == NULL )
				break;
			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			lEndOffset = s.GetPosition();
			lEndLine = s.GetLineNumber();
			s.SeekLine( lStartOffset, lStartLine );
		}
	}
	if ( iType & 2 )
	{
		CWorldSearch AreaChars( pt, iDist );
		while(true)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL )
				break;
			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			lEndOffset = s.GetPosition();
			lEndLine = s.GetLineNumber();
			s.SeekLine( lStartOffset, lStartLine );
		}
	}

	if ( lEndOffset <= lStartOffset )
	{
		// just skip to the end.
		TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc );
		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
	}
	else
	{
		s.SeekLine( lEndOffset, lEndLine );
	}
	return( TRIGRET_ENDIF );
}

#endif

TRIGRET_TYPE CScriptObj::OnTriggerRun( CScript &s, TRIGRUN_TYPE trigrun, CTextConsole * pSrc, int iArg )
{
	// ARGS:
	//	TRIGRUN_SECTION_SINGLE = just this 1 line.
	// RETURN:
	//  TRIGRET_RET_FALSE = 0 = return and continue processing.
	//  TRIGRET_RET_TRUE = 1 = return and handled. (halt further processing)
	//  TRIGRET_RET_DEFAULT = 2 = if process returns nothing specifically.

	sm_iTrigArg = iArg;

	int iNestedFalse = 0;

	if ( trigrun == TRIGRUN_SECTION_EXEC || trigrun == TRIGRUN_SECTION_SINGLE )	// header was already read in.
		goto jump_in;

	while ( s.ReadKeyParse())
	{
		// Hit the end of the next trigger.
		if ( s.IsKeyHead( "ON", 2 ))	// done with this section.
			break;

jump_in:
		if ( s.IsKey( "ENDIF" ) || s.IsKey( "END" ) || s.IsKey( "ENDFOR" )) 
		{
			if ( ! iNestedFalse ) 
				return( TRIGRET_ENDIF );
			iNestedFalse --;
			continue;
		}
		else if ( s.IsKey( "ELSE" )) 
		{
			if ( iNestedFalse ) 
				continue;
			return( ( trigrun == TRIGRUN_SECTION_FALSE ) ? TRIGRET_ELSE : TRIGRET_ELIF_FALSE );
		}
		else if ( s.IsKey( "ELIF" ) || s.IsKey( "ELSEIF" ))
		{
			if ( iNestedFalse )
				continue;
			if ( trigrun != TRIGRUN_SECTION_FALSE )
				return( TRIGRET_ELIF_FALSE );
		}
		else if ( trigrun == TRIGRUN_SECTION_FALSE ) 
		{
			// Ignoring this whole section.
			if ( s.IsKey( "IF" ) || s.IsKey( "BEGIN" ))
				iNestedFalse ++;
			continue;
		}

#ifdef GRAY_SVR

		else if ( s.IsKeyHead( "FOR", 3 ))
		{
			TRIGRET_TYPE iRet;
			if ( s.IsKey( "FOROBJS" ))
			{
				iRet = OnTriggerForLoop( s, 3, pSrc );
			}
			else if ( s.IsKey( "FORITEMS" ))	// all items near by
			{
				iRet = OnTriggerForLoop( s, 1, pSrc );
			}
			else if ( s.IsKey( "FORCHARS" ))	// all chars near by
			{
				iRet = OnTriggerForLoop( s, 2, pSrc );
			}
			else
			{
				iRet = TRIGRET_ENDIF;	// it's just somethin else.
			}
			if ( iRet != TRIGRET_ENDIF )
			{
				if ( iRet > TRIGRET_RET_DEFAULT )
				{
					DEBUG_MSG(( "WARNING: Trigger Bad For Ret %d '%s','%s'\n", iRet, s.GetKey(), s.GetArgStr())); 
				}
				return( iRet );
			}
		}

		else if ( s.IsKey( "DORAND" ))	// Do a random line in here.
		{
			int iVal = GetRandVal( s.GetArgVal());
			while (true)
			{
				if ( ! s.ReadKeyParse()) 
					return( TRIGRET_RET_DEFAULT );
				ParseText( s.GetArgStr(), pSrc );
				if ( s.IsKey( "ENDDO" ) || s.IsKey( "ENDRAND" )) 
					break;
				if ( s.IsKey( "BEGIN" ))	// do a section.
				{
					TRIGRET_TYPE iRet = OnTriggerRun( s, (!iVal) ? TRIGRUN_SECTION_TRUE : TRIGRUN_SECTION_FALSE, pSrc, iArg );
					if ( iRet != TRIGRET_ENDIF )
					{
						if ( iRet > TRIGRET_RET_DEFAULT )
						{
							DEBUG_MSG(( "WARNING: Trigger Bad Ret %d '%s','%s'\n", iRet, s.GetKey(), s.GetArgStr())); 
						}
						return( iRet );
					}
				}
				else if ( ! iVal )
				{
					if ( ! r_Verb( s, pSrc )) 
					{ 
						DEBUG_MSG(( "WARNING: Trigger Bad Rand Verb '%s','%s'\n", s.GetKey(), s.GetArgStr())); 
					}
				}
				iVal --;
			}
			continue;
		}
#endif

		// Parse out any variables in it. (may act like a verb sometimes)
		ParseText( s.GetArgStr(), pSrc );

		if ( s.IsKey( "ELIF" ) || s.IsKey( "ELSEIF" ))
		{
			return( s.GetArgVal() ? TRIGRET_ELSE : TRIGRET_ELIF_FALSE );
		}

		// Process the trigger.
		if ( s.IsKey( "RETURN" ))
		{
			return( s.GetArgVal() ? TRIGRET_RET_TRUE : TRIGRET_RET_FALSE );
		}

		if ( s.IsKey( "IF" ))
		{
			bool fTrigger = s.GetArgVal() ? true : false;
			bool fBeenTrue = false;
			while (true)
			{
				TRIGRET_TYPE iRet = OnTriggerRun( s, fTrigger ? TRIGRUN_SECTION_TRUE : TRIGRUN_SECTION_FALSE, pSrc, iArg );
				if ( iRet < TRIGRET_ENDIF ) 
					return( iRet );
				if ( iRet == TRIGRET_ENDIF ) 
					break;
				fBeenTrue |= fTrigger;
				if ( iRet == TRIGRET_ELSE ) 
					fTrigger = ! fBeenTrue;
				else 
					fTrigger = false; 
			}
			continue;
		}

		if ( ! r_Verb( s, pSrc )) 
		{ 
			if ( trigrun == TRIGRUN_SECTION_SINGLE ) 
				break;
			DEBUG_MSG(( "WARNING: Trigger Bad Verb '%s','%s'\n", s.GetKey(), s.GetArgStr())); 
		}

		if ( trigrun == TRIGRUN_SECTION_SINGLE ) 
			break;
	}

	return( TRIGRET_RET_DEFAULT );
}

TRIGRET_TYPE CScriptObj::OnTriggerScript( CScript & s, const TCHAR * pTrigName, CTextConsole * pSrc, int iArg )
{
	// look for exact trigger matches.

	while ( s.ReadKeyParse())
	{
		// Is it the right trigger ?
		if ( ! s.IsKeyHead( "ON", 2 ))
			continue;
		if ( strcmpi( s.GetArgStr(), pTrigName )) 
			continue;
		return OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, iArg );
	}
	return( TRIGRET_RET_FALSE );
}

bool CScriptObj::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
#ifdef GRAY_SVR
	ASSERT( pSrc );

	// A key name that just links to another object.

	if ( ! strnicmp( pszKey, "SRC.", 4 ))
	{
		pszKey += 4;
		pRef = dynamic_cast <CScriptObj*> (pSrc->GetChar());	// if it can be converted .
		return( true );
	}

	if ( ! strnicmp( pszKey, "SERV.", 5 ))
	{
		if ( pSrc->GetPrivLevel() < PLEVEL_Admin )
			return( false );
		pszKey += 5;
		pRef = &g_Serv;
		return( true );
	}

	if ( ! strnicmp( pszKey, "UID.", 4 ))
	{
		// Can it be resolved to any uid ref.
		pszKey += 4;
		CObjUID uid = (DWORD) Exp_GetVal( pszKey );
		SKIP_SEPERATORS(pszKey);
		pRef = uid.ObjFind();
		return( true );
	}
#endif

	if ( ! strnicmp( pszKey, "I.", 2 ))	// just talking about myself.
	{
		pszKey += 2;
		pRef = this;
		return( true );
	}

	if ( ! strnicmp( pszKey, "VAR.", 4 ))
	{
		// "VAR" = get/set a system wide variable.
		pszKey += 4;
		TCHAR * pszName = (TCHAR *) pszKey;
		Parse( pszName, (TCHAR **) &pszKey );
		int i = g_Exp.m_VarDefs.FindKey( pszName );
		if ( i < 0 )	// create it.
		{
			pRef = new CVarDef( pszName );
			g_Exp.m_VarDefs.Add( (CVarDef*) pRef );
		}
		else
		{
			pRef = g_Exp.m_VarDefs[i];
		}
		return( true );
	}

	return( false );
}

bool CScriptObj::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	ASSERT( pSrc );
	
	if ( s.IsKey( "SHOW" ))
	{
		CGString sVal;
		if ( ! r_WriteVal( s.GetArgStr(), sVal, pSrc ))
			return( false );
		CGString sMsg;
		sMsg.Format( "'%s' for '%s' is '%s'\n", s.GetArgStr(), GetName(), (const TCHAR*) sVal );
		pSrc->SysMessage( sMsg );
		return( true );
	}
	else if ( s.IsKey( "TRIGGER" ))
	{
		// This is effectively a goto to an alternate trigger. (for this same object)
		TCHAR * pszVals[2];
		if ( ParseCmds( s.GetArgStr(), pszVals, COUNTOF(pszVals)))
		{
			return( OnTrigger( pszVals[0], pSrc, Exp_GetVal(pszVals[1])) ? TRIGRET_RET_TRUE : TRIGRET_RET_FALSE );
		}
	}

	const TCHAR * pszKey = s.GetKey();
	CScriptObj * pRef;
	if ( r_GetRef( pszKey, pRef, pSrc ))
	{
		if ( ! pRef ) 
			return( false );
		return pRef->r_Verb( CScript( pszKey, s.GetArgStr()), pSrc );
	}

	return r_LoadVal( s );	// default to loading values.
}

bool CScriptObj::r_WriteVal( const TCHAR *pszKey, CGString &sVal, CTextConsole * pSrc )
{
	CScriptObj * pRef;
	if ( r_GetRef( pszKey, pRef, pSrc ))
	{
		if ( pRef == NULL )
		{
			// good command but bad link.
			sVal = "0";	// Bad refs always return "0"
			return( true );
		}
		return pRef->r_WriteVal( pszKey, sVal, pSrc );
	}
#if 0
	if ( ! strcmpi( pszKey, "NAME" ))	// weird order of precedence problem here.
	{
		sVal = GetName();
		return( true );
	}
#endif
	if ( ! strcmpi( pszKey, "TRIGARG" ))
	{
		sVal.FormatVal( sm_iTrigArg );
		return( true );
	}
	if ( ! strnicmp( pszKey, "VALSTR", 6 ))
	{
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);
		sVal.FormatVal( Exp_GetVal( pszKey ));
		return( true );
	}
	return( false );	// Bad command.
}

