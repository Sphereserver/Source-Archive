//
// GRAYINST.CPP
// Copyright Menace Software (www.menasoft.com).
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"

#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"

#else
#include "graycom.h"
#include "cgrayinst.h"
#endif

bool CGrayInstall::FindInstall()
{
#ifdef _WIN32
	// Get the install path from the registry.
	HKEY hKey;
	LONG lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
		"Software\\Origin Worlds Online\\Ultima Online\\1.0", // address of name of subkey to query 
		0, KEY_READ, 
		&hKey );
	if ( lRet != ERROR_SUCCESS )
		return( false );

	TCHAR szValue[ _MAX_PATH ];
	DWORD lSize = sizeof( szValue );
	DWORD dwType = REG_SZ;
	lRet = RegQueryValueEx( hKey, 
		_TEXT("ExePath"), // address of name of subkey to query 
		NULL, &dwType,
		(BYTE*) szValue, &lSize );

	if ( lRet == ERROR_SUCCESS && dwType == REG_SZ )
	{
		TCHAR * pSlash = strrchr( szValue, '\\' );	// get rid of the client.exe part of the name
		if ( pSlash ) * pSlash = '\0';
		m_sExePath = szValue;
	}

	// ??? Find CDROM install base as well, just in case.
	// uo.cfg CdRomDataPath=e:\uo

	lRet = RegQueryValueEx( hKey, 
		_TEXT("InstCDPath"), // address of name of subkey to query 
		NULL, &dwType,
		(BYTE*) szValue, &lSize );

	if ( lRet == ERROR_SUCCESS && dwType == REG_SZ )
	{
		m_sCDPath = szValue;
	}

	RegCloseKey( hKey );

#else
	// LINUX has no registry so we must have the INI file show us where it is installed.
#endif

	return( true );
}

const TCHAR * CGrayInstall::GetBaseFileName( VERFILE_TYPE i ) // static
{
	static const TCHAR * szFileNames[VERFILE_QTY] = 
	{
		"map0.mul",		// Terrain data
		"staidx0.mul",	// Index into STATICS0
		"statics0.mul", // Static objects on the map
		"artidx.mul",	// Index to ART
		"art.mul",		// Artwork such as ground, objects, etc.
		"anim.idx",
		"anim.mul",		// Animations such as monsters, people, and armor.
		"soundidx.mul", // Index into SOUND
		"sound.mul",	// Sampled sounds 
		"texidx.mul",	// Index into TEXMAPS
		"texmaps.mul",	// Texture map data (the ground).
		"gumpidx.mul",	// Index to GUMPART
		"gumpart.mul",	// Gumps. Stationary controller bitmaps such as windows, buttons, paperdoll pieces, etc.
		"multi.idx",
		"multi.mul",	// Groups of art (houses, castles, etc)
		"skills.idx",
		"skills.mul",
		"radarcol.mul",	// color tranlation form terrain to radar map.
		"fonts.mul",	//  Fixed size bitmaps style fonts. 
		"palette.mul",	// Contains an 8 bit palette (use unknown)
		"light.mul",	// light pattern bitmaps.
		"lightidx.mul", // light pattern bitmaps.
		"hues.mul",		// the 16 bit color pallete we use for everything.
		"verdata.mul",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"tiledata.mul", // Data about tiles in ART. name and flags, etc
		"animdata.mul", //
	};

	if ( i<0 || i>=VERFILE_QTY ) 
		return( NULL );
	return( szFileNames[ i ] );
}

bool CGrayInstall::OpenFile( CFileBin & file, const TCHAR * pszName, WORD wFlags )
{
	if ( ! m_sPreferPath.IsEmpty())
	{
		if ( file.Open( GetPreferPath( pszName ), wFlags ))
			return true;
	}

#ifdef _AFXDLL
	CFileException e;
	if ( file.Open( GetFullExePath( pszName ), wFlags, &e ))
		return true;
	if ( file.Open( GetFullCDPath( pszName ), wFlags, &e ))
		return true;
#else
	if ( file.Open( GetFullExePath( pszName ), wFlags ))
		return true;
	if ( file.Open( GetFullCDPath( pszName ), wFlags ))
		return true;
#endif

	return( false );
}

bool CGrayInstall::SetMulFile( VERFILE_TYPE i, const TCHAR * pszName )
{
	// Close(); // then reopen later ?
	CFileBin * pFile = GetMulFile(i);
	if ( pFile == NULL ) 
		return false;
	pFile->SetFilePath( pszName );
	return( true );
}

bool CGrayInstall::OpenFile( VERFILE_TYPE i )
{
	ASSERT( i < VERFILE_QTY );
	if ( m_File[i].IsFileOpen()) 
		return( true );

	const TCHAR * pszName = GetBaseFileName( (VERFILE_TYPE) i );
	if ( pszName == NULL ) 
		return( false );

	return( OpenFile( m_File[i], pszName, OF_READ|OF_SHARE_DENY_WRITE ));
}

VERFILE_TYPE CGrayInstall::OpenFiles( DWORD dwMask )
{
	// Now open all the required files.
	// REUTRN: VERFILE_QTY = all open success.

	int i;
	for ( i=0; i<VERFILE_QTY; i++ )
	{
		if ( ! ( dwMask & ( 1 << i ))) 
			continue;
		if ( GetBaseFileName( (VERFILE_TYPE) i ) == NULL )
			continue;

		if ( ! OpenFile( (VERFILE_TYPE) i ))
		{
			if ( i == VERFILE_VERDATA )
				continue;	// this is optional
			// Problems ! Can't open the file anyplace !
			break;
		}
	}

	return( (VERFILE_TYPE) i );
}

void CGrayInstall::CloseFiles()
{
	for ( int i=0; i<VERFILE_QTY; i++ )
	{
		m_File[i].Close();
	}
}

bool CGrayInstall::ReadMulIndex( VERFILE_TYPE fileindex, VERFILE_TYPE filedata, DWORD id, CUOIndexRec & Index )
{
	// Read about this data type in one of the index files.
	// RETURN: true = we are ok.
	ASSERT(fileindex<VERFILE_QTY);

	// Is there an index for it in the VerData ?
	if ( g_VerData.FindVerDataBlock( filedata, id, Index ))
	{
		return( true );
	}
	if ( ! m_File[fileindex].Seek( id * sizeof(CUOIndexRec), SEEK_SET ))
	{
		return( false );
	}
	if ( m_File[fileindex].Read( (void *) &Index, sizeof(CUOIndexRec)) != sizeof(CUOIndexRec))
	{
		return( false );
	}
	return( Index.HasData());
}

bool CGrayInstall::ReadMulData( VERFILE_TYPE filedata, const CUOIndexRec & Index, void * pData )
{
	if ( Index.IsVerData())
	{
		filedata = VERFILE_VERDATA;
	}
	if ( ! m_File[filedata].Seek( Index.GetFileOffset(), SEEK_SET ))
	{
		return( false );
	}
	DWORD dwLength = Index.GetBlockLength();
	if ( m_File[filedata].Read( pData, dwLength ) != dwLength )
	{
		return( false );
	}
	return( true );
}

//////////////////////////////////////////////////////////////////////
// -CVerDataMul

void CVerDataMul::Load()
{
	// assume it is in sorted order.
	if ( GetCount())	// already loaded.
	{
		return;
	}

#define g_fVerData g_Install.m_File[VERFILE_VERDATA]

	if ( ! g_fVerData.IsFileOpen())		// T2a might not have this.
		return;

	g_fVerData.SeekToBegin();
	DWORD dwQty;
	if ( g_fVerData.Read( (void *) &dwQty, sizeof(dwQty)) <= 0 )
		throw CGrayError(LOGL_CRIT, -1, "VerData: Read Qty");

	Unload();
	m_Data.SetCount( dwQty );

	if ( g_fVerData.Read( (void *) m_Data.GetBasePtr(), GetCount() * sizeof( CUOVersionBlock )) <= 0 )
		throw CGrayError(LOGL_CRIT, E_FAIL, "VerData: Read");

	// Now sort it for fast searching.
	// Make sure it is sorted.
	for ( int i=0; i<GetCount()-1; i++ )
	{
		if ( GetEntry(i)->GetIndex() > GetEntry(i+1)->GetIndex())
		{
			DEBUG_ERR(( "VerData Array is NOT sorted !\n" ));
			break;
		}
	}
}

bool CVerDataMul::FindVerDataBlock( VERFILE_TYPE type, DWORD id, CUOIndexRec & Index ) const
{
	// Search the verdata.mul for changes to stuff.
	// search for a specific block.
	// assume it is in sorted order.

	int iHigh = GetCount()-1;
	if ( iHigh < 0 )
	{
		return( false );
	}

	DWORD dwIndex = VERDATA_MAKE_INDEX(type,id);
	const CUOVersionBlock *pArray = (const CUOVersionBlock *) m_Data.GetBasePtr();
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		int iCompare = dwIndex - pArray[i].GetIndex();
		if ( iCompare == 0 )
		{
			Index.CopyIndex( &(pArray[i]));
			return( true );
		}
		if ( iCompare > 0 ) 
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( false );
}


