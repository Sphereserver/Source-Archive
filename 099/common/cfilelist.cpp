    // CFileList
    //
    
    #if defined(GRAY_SVR) || defined(GRAY_MAP) || defined(GRAY_CLIENT)
    #include "graycom.h"
    #elif defined(_WIN16) || defined(_WIN32)
    #include <windows.h>
    #include <stdio.h>
    #endif
    
    #include "cfilelist.h"
    
    #if defined(_WIN16)
    #include <dos.h>	 	// dos_findfirst
    static struct find_t fileinfo;                         // File control block.
    #elif defined(_WIN32)
    #include <io.h> 		// findfirst
    static struct _finddata_t fileinfo;
    #else	// LINUX
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    static struct dirent * fileinfo;
    static DIR * dirp;
    #endif
    
    // Similar to the MFC CFileFind
    bool CFileList::ReadFileInfo( LPCTSTR pszFilePath, time_t & dwDateChange, DWORD & dwSize ) // static
    {
    #if defined(_WIN16)
    	fileinfo.attrib = _A_NORMAL;
    	if ( _dos_findfirst( pszFilePath, _A_NORMAL, &fileinfo ))
    #elif defined(_WIN32)
    	fileinfo.attrib = _A_NORMAL;
    	long lFind = _findfirst( pszFilePath, &fileinfo );
    	if ( lFind == -1 )
    #else
    	// LINUX
    	struct stat fileStat;
    	if ( stat( pszFilePath, &fileStat) == -1 )
    #endif
    	{
    		DEBUG_ERR(( "Can't open input dir \n", pszFilePath ));
    		return( false );
    	}
    #if defined(_WIN16)
    	dwDateChange = fileinfo.time_write;
    	dwSize = fileinfo.size;
    #elif defined(_WIN32)
    	dwDateChange = fileinfo.time_write;
    	dwSize = fileinfo.size;
    	_findclose( lFind );
    #else
    	// LINUX
    	dwDateChange = fileStat.st_mtime;
    	dwSize = fileStat.st_size;
    #endif
    	return( true );
    }
    
    int CFileList::ReadDir( LPCTSTR pszFileDir )
    {
    	// NOTE: It seems NOT to like the trailing \ alone
    	//  ex. d:\menace\scripts\ is bad ! use d:\menace\scripts\*.*
    
    	TCHAR szFileDir;
    	int len = strcpylen( szFileDir, pszFileDir );
    #ifdef _WIN32
    	if ( len )
    	{
    		len--;
    		if ( szFileDir == '\\' || szFileDir == '/' )
    			strcat( szFileDir, "*.*" );
    	}
    #endif
    
    #if defined(_WIN16)
    	fileinfo. = _A_NORMAL
    	if ( _dos_findfirst( szFileDir, _A_NORMAL, &fileinfo ))
    #elif defined(_WIN32)
    	long lFind = _findfirst( szFileDir, &fileinfo );
    	if ( lFind == -1 )
    #else
    	// LINUX
    	// Need to strip out the *.scp part
    	for ( int i = len; i > 0; i-- )
    	{
    		if ( szFileDir == '/' )
    		{
    			szFileDir = 0x00;
    			break;
    		}
    	}
    		dirp = opendir(szFileDir);
    	if ( dirp == NULL )
    #endif
    	{
    		DEBUG_ERR(( "Unable to open directory %s\n", szFileDir));
    		return( -1 );
    	}
    	do
    	{
    #if defined(_WIN16) || defined(_WIN32)
    		if ( fileinfo.name == '.' )
    			continue;
    		AddHead( fileinfo.name );
    #else
    		fileinfo = readdir(dirp);
    		if ( fileinfo == NULL )
    		{
    			break;
    		}
    		if ( fileinfo->d_name == '.' )
    			continue;
    		char szFilename;
    		sprintf(szFilename, "%s%s", szFileDir, fileinfo->d_name);
    		len = strlen(szFilename);
    		if ( len > 4 && ( strcmpi(&szFilename, ".scp") == 0 ) )
    		AddHead( fileinfo->d_name );
    #endif
    
    #if defined(_WIN16)
    	} while ( ! _dos_findnext( &fileinfo )) ;                       // No more.
    #elif defined(_WIN32)
    	} while ( ! _findnext( lFind, &fileinfo )) ;                       // No more.
    	_findclose( lFind );
    #else
    	// LINUX
    	} while ( fileinfo != NULL );
    #endif
    
    	return( GetCount() );
    }