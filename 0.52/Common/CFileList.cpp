//
// CFileList
//

#ifdef _WIN32

#if defined(GRAY_SVR) || defined(GRAY_MAP) || defined(GRAY_CLIENT)
#include "graycom.h"
#else
#include <windows.h>
#include <stdio.h>
#endif

#include "cassoc.h"

#ifdef _WIN32
#include <io.h> 		// findfirst
#elif defined(_WIN16)
#include <dos.h>	 	// dos_findfirst
#endif

int CFileList::ReadDir( const TCHAR * pszFilePath )
{
#ifdef _WIN32
	static struct _finddata_t fileinfo;
	long lFind = _findfirst( pszFilePath, &fileinfo );
	if ( lFind == -1 )
#else
	static struct find_t fileinfo;                         // File control block.
	if ( _dos_findfirst( pszFilePath, _A_NORMAL, &fileinfo ))
#endif
	{
		// printf( "Can't open input file [%s]\n", (PCSTR) pszFilePath );
		return( -1 );
	}
	do
	{
		if ( fileinfo.name[0] == '.' )
			continue;
		AddHead( fileinfo.name );
#ifdef _WIN32
	} while ( ! _findnext( lFind, &fileinfo )) ;                       // No more.
	_findclose( lFind );
#else
	} while ( ! _dos_findnext( &fileinfo )) ;                       // No more.
#endif

	return( GetCount() );
}

#endif	// _WIN32
