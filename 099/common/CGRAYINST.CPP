    // GRAYINST.CPP
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
    
    #else
    #include "graycom.h"
    #include "cgrayinst.h"
    #endif
    
    bool CGrayInstall::FindInstall()
    {
    #ifdef _WIN32
    	// Get the install path from the registry.
    
    	static LPCTSTR m_szKeys =
    	{
    		"Software\\Menasoft\\" GRAY_FILE,
    		"Software\\Origin Worlds Online\\Ultima Online\\1.0", 
    		"Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0", 
    	};
    	
    	HKEY hKey;
    	LONG lRet;
    	for ( int i=0; i<COUNTOF(m_szKeys); i++ )
    	{
    		lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
    			m_szKeys, // address of name of subkey to query
    			0, KEY_READ, &hKey );
    		if ( lRet == ERROR_SUCCESS )
    			break;
    	}
    	if ( lRet != ERROR_SUCCESS )
    	{
    		return( false );
    	}
    
    	TCHAR szValue;
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
    
    bool CGrayInstall::SetMulFile( VERFILE_TYPE i, LPCTSTR pszName )
    {
    	// Set this file to have an individual path.
    	// Close(); // then reopen later ?
    	CGFile * pFile = GetMulFile(i);
    	if ( pFile == NULL )
    		return false;
    	pFile->SetFilePath( pszName );
    	return( true );
    }
    
    bool CGrayInstall::OpenFile( CGFile & file, LPCTSTR pszName, WORD wFlags )
    {
    	ASSERT(pszName);
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
    
    #ifndef GRAY_AGENT
    
    LPCTSTR CGrayInstall::GetBaseFileName( VERFILE_TYPE i ) // static
    {
    	static LPCTSTR const sm_szFileNames =
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
    		"radarcol.mul",	// ? color tranlation form terrain to radar map.
    		"fonts.mul",	//  Fixed size bitmaps style fonts.
    		"palette.mul",	// Contains an 8 bit palette (use unknown)
    		"light.mul",	// light pattern bitmaps.
    		"lightidx.mul", // light pattern bitmaps.
    		"speech.mul",	// > 2.0.5 new versions only.
    		"verdata.mul",
    		NULL,
    		NULL,
    		NULL,
    		NULL,
    		NULL,
    		NULL,
    		"tiledata.mul", // Data about tiles in ART. name and flags, etc
    		"animdata.mul", //
    		"hues.mul",		// the 16 bit color pallete we use for everything.
    	};
    
    	if ( i<0 || i>=VERFILE_QTY )
    		return( NULL );
    	return( sm_szFileNames );
    }
    
    bool CGrayInstall::OpenFile( VERFILE_TYPE i )
    {
    	ASSERT( i < VERFILE_QTY );
    	CGFile * pFile = GetMulFile(i);
    	if ( pFile == NULL )
    		return false;
    	if ( pFile->IsFileOpen())
    		return( true );
    
    	if ( ! pFile->GetFilePath().IsEmpty())
    	{
    		if ( pFile->Open( pFile->GetFilePath(), OF_READ|OF_SHARE_DENY_WRITE ))
    			return true;
    	}
    
    	LPCTSTR pszTitle = GetBaseFileName( (VERFILE_TYPE) i );
    	if ( pszTitle == NULL )
    		return( false );
    
    	return( OpenFile( m_File, pszTitle, OF_READ|OF_SHARE_DENY_WRITE ));
    }
    
    VERFILE_TYPE CGrayInstall::OpenFiles( DWORD dwMask )
    {
    	// Now open all the required files.
    	// REUTRN: VERFILE_QTY = all open success.
    
    	for ( int i=0; i<VERFILE_QTY; i++ )
    	{
    		if ( ! ( dwMask & ( 1 << i )))
    			continue;
    		if ( GetBaseFileName( (VERFILE_TYPE) i ) == NULL )
    			continue;
    
    		if ( ! OpenFile( (VERFILE_TYPE) i ))
    		{
    			if ( i == VERFILE_VERDATA )
    				continue;	// this is optional
    			if ( i == VERFILE_SPEECH )
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
    		if ( ! m_File.IsFileOpen())
    			continue;
    		m_File.Close();
    	}
    }
    
    #if 0
    
    bool CGrayInstall::IsValidMulIndex( VERFILE_TYPE fileindex, DWORD id )
    {
    	// Mul files may vary in size in the future.
    	// Some are hard coded. or limited by scripts.
    
    	switch ( fileindex )
    	{
    	case VERFILE_ANIMIDX:
    	case VERFILE_ANIM:
    		return( true );
    
    	case VERFILE_SOUNDIDX:
    	case VERFILE_SOUND:
    		if ( id >= SOUND_QTY )
    			return( false );
    		return( true );
    
    	case VERFILE_GUMPIDX:
    	case VERFILE_GUMPART:
    		return( true );
    		
    	case VERFILE_MULTIIDX:
    	case VERFILE_MULTI:
    		return( true );
    
    	case VERFILE_SKILLSIDX:
    	case VERFILE_SKILLS:
    		return( true );
    	}
    
    	DEBUG_CHECK(0);
    	return( false );
    }
    
    #endif
    
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
    
    	// Just in the regular Index data.
    
    	LONG lOffset = id * sizeof(CUOIndexRec);
    	if ( m_File.Seek( lOffset, SEEK_SET ) != lOffset )
    	{
    		return( false );
    	}
    	if ( m_File.Read( (void *) &Index, sizeof(CUOIndexRec)) != sizeof(CUOIndexRec))
    	{
    		return( false );
    	}
    	return( Index.HasData());
    }
    
    bool CGrayInstall::ReadMulData( VERFILE_TYPE filedata, const CUOIndexRec & Index, void * pData )
    {
    	// Use CGFile::GetLastError() for error.
    	if ( Index.IsVerData())
    	{
    		filedata = VERFILE_VERDATA;
    	}
    	if ( m_File.Seek( Index.GetFileOffset(), SEEK_SET ) != Index.GetFileOffset())
    	{
    		return( false );
    	}
    	DWORD dwLength = Index.GetBlockLength();
    	if ( m_File.Read( pData, dwLength ) != dwLength )
    	{
    		return( false );
    	}
    	return( true );
    }
    
    #endif	// GRAY_AGENT