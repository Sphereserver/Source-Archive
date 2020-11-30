//
// CgrayCom.cpp
//

#include "graycom.h"
#include "graymul.h"
#include "grayproto.h"

#ifdef _WIN32 

OSVERSIONINFO g_osInfo;
const OSVERSIONINFO * GRAY_GetOSInfo() 
{
	if ( g_osInfo.dwOSVersionInfoSize != sizeof(g_osInfo) )
	{
		g_osInfo.dwOSVersionInfoSize = sizeof(g_osInfo);
		if ( ! GetVersionEx(&g_osInfo))
		{
			// must be an old version of windows. win95 or win31 ?
			memset( &g_osInfo, 0, sizeof(g_osInfo));
			g_osInfo.dwOSVersionInfoSize = sizeof(g_osInfo);
			g_osInfo.dwPlatformId = VER_PLATFORM_WIN32s;	// probably not right.
		}
	}
	return( &g_osInfo );
}

#endif

int CvtSystemToNUNICODE( NCHAR * pOut, const TCHAR * pInp, int iSizeInWideChars )
{
	//
	// Convert the system default text format UTF8 to UNICODE
	// May be network byte order !
	// RETURN: 
	//  Number of wide chars. not including null.
	//

	int i=0;

#ifdef _WIN32 
	GRAY_GetOSInfo();
	if ( g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ||
		g_osInfo.dwMajorVersion > 4 )
	{
		// Windows 98 or NT
		// NOTE: ? UTF8 only works in Windows NT 4.0 and Windows 2000 ?

		int iLen = MultiByteToWideChar(
			CP_UTF8,         // code page
			0,         // character-type options
			pInp, // address of string to map
			-1,      // number of bytes in string
			(LPWSTR) pOut,  // address of wide-character buffer
			iSizeInWideChars        // size of buffer
			);
		if ( iLen <= 0 )
		{
			return( 0 );
		}

		// flip all the words to network order .
		if ( iLen > iSizeInWideChars )
			iLen = iSizeInWideChars;
		iLen --;	// return included the null.
		for ( ; i<iLen; i++ )
		{
			pOut[i] = *((WCHAR*)&(pOut[i]));
		}
	}
	else
#endif
	{
		// Win95 or Linux
		iSizeInWideChars--;
		for ( ; pInp[i] && i < iSizeInWideChars; i++ )
		{
			pOut[i] = pInp[i];
		}
	}
	pOut[i] = 0;

	return( i );
}

int CvtNUNICODEToSystem( TCHAR * pOut, const NCHAR * pInp, int iSizeInBytes )
{
	// ARGS:
	//  iSizeInBytes = space we have (included null char)
	// RETURN: 
	//  Number of bytes. (not including null)
	// NOTE: 
	//  This need not be a properly terminated string.

	int i=0;
#ifdef _WIN32
	GRAY_GetOSInfo();
	if ( g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT || 
		g_osInfo.dwMajorVersion > 4 )
	{
		// Windows 98 or NT

		if ( iSizeInBytes <= 0 )
			return( 0 );
		iSizeInBytes--;

		// Flip all from network order.
		WCHAR szBuffer[ 1024*8 ];
		for ( ; pInp[i] && i < COUNTOF(szBuffer)-1 && i < iSizeInBytes; i++ )
		{
			szBuffer[i] = pInp[i];	
		}

		// Convert to proper UTF8
		i = WideCharToMultiByte(
			CP_UTF8,         // code page
			0,         // performance and mapping flags
			szBuffer, // address of wide-character string
			i,       // number of characters in string
			pOut,  // address of buffer for new string
			iSizeInBytes+1,      // size of buffer
			NULL,  // address of default for unmappable characters
			NULL  // address of flag set when default char. used
			);
		if ( i < 0 ) 
			return( 0 );
	}
	else
#endif
	{
		// Win95 or linux
		iSizeInBytes--;
		for ( ; pInp[i] && i < iSizeInBytes; i++ )
		{
			pOut[i] = pInp[i];
		}
	}
	pOut[i] = 0;

	return( i );
}

