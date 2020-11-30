//
// CFile.cpp
// Copyright Menace Software (www.menasoft.com).
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"

#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"
 
#elif defined(GRAY_SVR) || defined(GRAY_MAP) || defined(GRAY_CLIENT)
#include "graycom.h"

#else
#include <windows.h>
#include <stdio.h>
#include "cfile.h"
#endif

void GetStrippedDirName( TCHAR * pszFilePath )
{
	int len = strlen( pszFilePath );
	while ( len > 0 )
	{
		if ( pszFilePath[len] == ':' )
			break;
		if ( pszFilePath[len] == '\\' || 
			pszFilePath[len] == '/' ) // Might be LINUX
		{
			break;
		}
		pszFilePath[len] = '\0';
		len --;
	}
}

CGString GetMergedFileName( const TCHAR *pszBase, const TCHAR *pszName )
{
	TCHAR szFilePath[ _MAX_PATH ];
	if ( pszBase[0] )
	{
		strcpy( szFilePath, pszBase );
		int len = strlen( szFilePath );
		if ( len && szFilePath[ len-1 ] != '\\' && 
			szFilePath[ len-1 ] != '/' )	// Might be LINUX
		{
			strcat( szFilePath, "\\" );
		}
	}
	else
	{
		szFilePath[0] = '\0';
	}
	if ( pszName )
	{
		strcat( szFilePath, pszName );
	}
	return( (CGString) szFilePath );
}

//***************************************************************************
// -CGFile

const TCHAR * CGFile::GetFileTitle() const
{
	const TCHAR * pszName = m_sFileName;
	int len = m_sFileName.GetLength();
	while ( len>0 )
	{
		len--;
		if ( pszName[len] == '\\' || pszName[len] == '/' )
		{
			len++;
			break;
		}
	}
	return( pszName + len );
}

bool CGFile::SetFilePath( const TCHAR * pszName )
{
	if ( pszName == NULL )
		return false;
	if ( ! m_sFileName.CompareNoCase( pszName ))
		return( true );
	bool fIsOpen = IsFileOpen();
	Close();
	m_sFileName = pszName;
	if ( fIsOpen )
	{
		return( Open( NULL, GetMode()));	// open it back up.
	}
	return( true );
}

bool CGFile::Open( const TCHAR *pszFilename, WORD wModeFlags, void FAR * pExtra )
{
	// RETURN: true = success.
	// OF_BINARY | OF_WRITE
	if ( pszFilename == NULL )
	{
		if ( IsFileOpen())
			return( true );
	}
	else
	{
		Close();	// Make sure it's closed first.
	}

	if ( pszFilename == NULL )
		pszFilename = GetFilePath();
	else 
		m_sFileName = pszFilename;

	SetMode( wModeFlags );
	return( OpenBase( pExtra ));
}

#ifndef _AFXDLL

bool CGFile::OpenCopy( const CGFile & file, WORD wModeFlags )
{
	Close();	// Make sure it's closed first.
	m_sFileName	= file.m_sFileName;
	ASSERT( file.IsFileOpen());
	SetMode( wModeFlags );
	return( OpenBaseCopy( file ));
}

#endif

//***************************************************************************
// -CFileText

bool CFileText::Close()
{
	if ( ! IsFileOpen())
		return true;
	if ( IsWriteMode())
	{
		fflush(m_pFile);
	}
	bool fSuccess = ( fclose( m_pFile ) == 0 );
	DEBUG_CHECK( fSuccess );
	m_pFile = NULL;
	return( fSuccess );
}

const TCHAR * CFileText::GetModeStr() const
{
	if ( IsBinaryMode())
		return ( IsWriteMode()) ? "wb" : "rb";
	if ( GetMode() & OF_READWRITE )
		return "a";
	return ( IsWriteMode()) ? "w" : "r";
}

bool CFileText::OpenBase( void FAR * pszExtra )
{
	// Open a file.
	m_pFile = fopen( m_sFileName, GetModeStr());
	if ( m_pFile == NULL )
	{
		return( false );
	}
	return( true );
}

#ifndef _AFXDLL

bool CFileText::OpenBaseCopy( const CGFile & file )
{
	const CFileText * pFile = dynamic_cast <const CFileText *> ( &file );
	ASSERT(pFile);
	if ( ! pFile ) 
		return( false );
	ASSERT( pFile->IsFileOpen());
#if 0
	// Dupe the existing file handle.
	int hFile = _fileno( pFile->m_pFile );
	const TCHAR * pszMode = GetModeStr();
	m_pFile = _fdopen( hFile, pszMode );
	if ( m_pFile == NULL )
	{
		return( false );
	}
#else
	// Just copy the file pointer.
	m_pFile = pFile->m_pFile;
#endif
	return( true );
}

//
// CGString file support
//

int CGString::ReadZ( CGFile * pFile, int iLenMax )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Read in a new string from an open MMSYSTEM file.
	// ARGS:
	//  hmmio = the open file.
	//  iLen = The length of the string to read. NOT THE NULL !
	// RETURN:
	//  <= 0 = error or no valid string.
	//  length of the string.
	//@------------------------------------------------------------------------

	if ( ! SetLength( iLenMax )) 
		return( -1 );

	if ( pFile->Read( m_pchData, iLenMax ) != iLenMax )
		return( -1 );

	//
	// Make sure it is null terminated.
	//
	m_pchData[ iLenMax ] = '\0';
	return( iLenMax );
}

bool CGString::WriteZ( CGFile * pFile )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Write a string AND NULL out to the file.
	// ARGS:
	//  hmmio = the open file.
	// NOTE:
	//  Standard RIFF strings are NULL terminated !
	// RETURN:
	//  string length. -1 = error.
	//@------------------------------------------------------------------------

	if ( ! pFile->Write( m_pchData, GetLength()+1 ))
		return( false );
	return( true );
}

int CGString::ReadFrom( CGFile * pFile, int iLenMax )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Read in a new string from an open MMSYSTEM file.
	// ARGS:
	//  hmmio = the open file.
	//  iLen = The length of the string to read. NOT THE NULL !
	// RETURN:
	//  <= 0 = error or no valid string.
	//  length of the string.
	//@------------------------------------------------------------------------

	WORD wLength = 0;
	if ( pFile->Read( &wLength, 2 ) != 2 )
		return( -1 );

	if ( ! SetLength( wLength )) 
		return( -1 );

	if ( pFile->Read( m_pchData, wLength ) != wLength )
		return( -1 );

	//
	// Make sure it is null terminated.
	//
	m_pchData[ wLength ] = '\0';
	return( wLength );
}

bool CGString::WriteTo( CGFile * pFile )
{
	//@------------------------------------------------------------------------
	// PURPOSE:
	//  Write a string AND NULL out to the file.
	// ARGS:
	//  hmmio = the open file.
	// NOTE:
	//  Standard RIFF strings are NULL terminated !
	// RETURN:
	//  string length. -1 = error.
	//@------------------------------------------------------------------------

	WORD wLength = GetLength();
	if ( ! pFile->Write( &wLength, 2 ))
		return( false );
	if ( ! pFile->Write( m_pchData, wLength ))
		return( false );
	return( true );
}

#endif	// _AFXDLL

