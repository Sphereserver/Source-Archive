//
// CModVer.CPP
// Menace Software	9/1/97
//
// This requires MFC Library (CString,CFile,CTime,etc)
// Link with VER.LIB
//

#include <afx.h>
#include <afxwin.h>
#include <iostream.h>

#include <ver.h>

#ifdef _WIN32
#include <io.h> 		// findfirst
#else
#include <dos.h>		// dos_findfirst
#endif

#include "common.h"

class CModVer
{
public:
	// Get version info for a single file.
	CString m_sFileName;	// Name.EXT w/o path.

	// All files have this.
	CTime 	m_tCreate;		// File creation date.
	long	m_lSize;

	// Is the module loaded ?
	// If so what is it's foot print ?
	BOOL   	m_fLoaded;

	// VERSION resource. Ignore Language and Copyright
	CString m_sCompany;
	CString m_sProdVersion;
	CString m_sFileVersion;

public:
	int SetName( const char * pszName );
	void Report( CFile * pFileOut );

	CModVer()
	{
		m_lSize = 0;
	}
	CModVer( const char * pszName )
	{
		m_lSize = 0;
		SetName( pszName );
	}
};

static int Mod_iCount = 0;

int CModVer :: SetName( const char * pszFilePath )
{
	if ( pszFilePath == NULL ) return( 1 );
	
	m_sFileName = pszFilePath;

	// RETURN: 0 = success.
	CFile File;
	if ( ! File.Open( m_sFileName, CFile::modeRead ))
		return( 2 );
	CFileStatus rStatus;
	if ( ! File.GetStatus( rStatus ))	// Should never fail !!
	{
		File.Close();
		return( 3 );
	}
	m_tCreate = rStatus.m_mtime;
	m_lSize = rStatus.m_size;
	File.Close();

	DWORD dwHandle;
	DWORD dwSize = GetFileVersionInfoSize( (char*)(const char*) m_sFileName, &dwHandle );
	if ( ! dwSize ) return( 0 );

	void * lpvData = new BYTE [ dwSize ];
	if ( lpvData == NULL ) return( 4 );

	BOOL fRet = GetFileVersionInfo( (char*)(const char*) m_sFileName, dwHandle, dwSize, lpvData );
	if ( fRet )
	{
		void FAR * lpvBlock = NULL;
		UINT uBlockSize;
		VerQueryValue( lpvData, "\\VarFileInfo\\Translation", &lpvBlock, &uBlockSize );
		if ( lpvBlock != NULL )
		{
			DWORD dwVal = *((DWORD FAR*)lpvBlock);

			char szBlockName[ 128 ];
			int iBaseLen = wsprintf( szBlockName, "\\StringFileInfo\\%04x%04x\\",
				LOWORD(dwVal), HIWORD(dwVal) );

			for ( int i=0; i<5; i++ )
			{
				static const char * VerFieldNames[] =
				{
					"CompanyName",
					"FileDescription",
					"FileVersion",
					"ProductName",
					"ProductVersion",
				};

				strcpy( szBlockName+iBaseLen, VerFieldNames[i] );
				if ( ! VerQueryValue( lpvData, szBlockName, &lpvBlock, &uBlockSize ))
					continue;
				switch (i)
				{
				case 0: m_sCompany	   = (char FAR*) lpvBlock; break;
				case 2: m_sFileVersion = (char FAR*) lpvBlock; break;
				case 4: m_sProdVersion = (char FAR*) lpvBlock; break;
				}
			}
		}
	}

	delete [] (BYTE *) lpvData;

	//
	// Is the module loaded ?
	// Vxd,386 else dll,tsp,exe,drv,cpl ?
	//
	return( 0 );
}

void CModVer :: Report( CFile * pFileOut )
{
	CString sVal;
	CString sDate = m_tCreate.Format( "%A, %B %d, %Y %H:%M:%S" );

	Mod_iCount ++;

	if ( ! m_lSize )	// might be a sub-dir.
	{
		sVal.Format( "%s CAN'T OPEN\r\n",
			(const char*) m_sFileName );
	}
	else if ( m_sCompany.IsEmpty())
	{
		sVal.Format( "%s S=%ld '%s' NO VERSION INFO\r\n",
			(const char*) m_sFileName, m_lSize,
			(const char*) sDate );
	}
	else
	{
		// format m_tCreate
		sVal.Format( "%s S=%ld '%s' C='%s' P='%s' F='%s'\r\n",
			(const char*) m_sFileName, m_lSize,
			(const char*) sDate,
			(const char*) m_sCompany, (const char*) m_sProdVersion,
			(const char*) m_sFileVersion );
	}

	pFileOut->Write( (const char*) sVal, sVal.GetLength());
}

static void DirCompare( const char * pszNameBase, CFile * pFileOut )
{
	// Get a directory of files and compare against the list of files that I care about.
	// Dump the results out to a file.

	if ( * pszNameBase == '\0' )
	{
		pFileOut->Write( "\r\n", 2 );
		return;
	}
    
    // remove the path info from the name.
	char szFilePath[ _MAX_PATH ];
	strcpy( szFilePath, pszNameBase );
	char * pFileName = strrchr( szFilePath, '\\' );
	if ( pFileName == NULL )
		pFileName = szFilePath;
	else
		pFileName ++;

#ifdef _WIN32
	struct _finddata_t fileinfo;
	long lFind = _findfirst( pszNameBase, &fileinfo );
	if ( lFind == -1 )
#else
	struct find_t fileinfo;  // File control block.
	if ( _dos_findfirst( pszNameBase, _A_NORMAL, &fileinfo ))
#endif
	{
		CModVer Mod( pszNameBase );
		Mod.Report( pFileOut );
		return;
	}

	do
	{
		//
		// Process the file.
		//
		if ( fileinfo.attrib & _A_SUBDIR ) continue;	// skip sub-dirs.
		strcpy( pFileName, fileinfo.name );
		CModVer Mod( szFilePath );
		// Output results.
		Mod.Report( pFileOut );

#ifdef _WIN32
	} while ( ! _findnext( lFind, &fileinfo )) ; // Any more ?
	_findclose( lFind );
#else
	} while ( ! _dos_findnext( &fileinfo )) ;
#endif
}

void DirCheck( LPCSTR pszFileInput )
{
#if 0
	// What is the name of the output file ?
	// What is the net name for this machine ?
	// HKEY_LOCAL_MACHINE
	// \System\CurrentControlSet\control\ComputerName ComputerName

	char szValue[128];
	LONG lRet = RegQueryValue( HKEY_LOCAL_MACHINE,
		"\System\CurrentControlSet\control\ComputerName",
		szValue, sizeof(szValue));

	OPENFILENAME ofn;
	if ( GetOpenFileName(&ofn))
	{

	}
#endif

	CFile FileOut;
	if ( ! FileOut.Open( "dirout.txt", CFile::modeCreate | CFile::modeWrite ))
		return;

	Mod_iCount = 0;

	// Open the list of files we care about.
	BOOL fLooked = FALSE;
	CStringList sList;
	FILE * pFileInp = NULL;
	if ( pszFileInput != NULL )
	{
		pFileInp = fopen( pszFileInput, "r" );
	}
	if ( pFileInp == NULL )
	{
		DirCompare( "*.*", &FileOut );
	}
	else
	{
		char szData[128];
		while ( fgets( szData, sizeof(szData), pFileInp ) != NULL )
		{
			char * pszBreak = strchr( szData, ' ' );
			if ( pszBreak != NULL )
				* pszBreak = '\0';
			pszBreak = strchr( szData, '\n' );
			if ( pszBreak != NULL )
				* pszBreak = '\0';
			pszBreak = strchr( szData, '\r' );
			if ( pszBreak != NULL )
				* pszBreak = '\0';
			DirCompare( szData, &FileOut );

			// Update status count.
		}
		fclose( pFileInp );
	}

	FileOut.Close();
}

#if 1

int _cdecl main(int argc, char* argv[])
{
	cout << _T("MODVER - List the modules and version info for files") << endl;
	cout << _T("Version 1.0 - Copyright (C) Menace Software 1997") << endl;
	cout << _T("http:\\\\www.menasoft.com") << endl;
	cout << endl;

	if (!AfxWinInit( GetModuleHandle(NULL), NULL, GetCommandLine(), 0))
	{
		cout << _T("MFC Failed to initialize.\n");
		return( 1 );
	}
    
    DirCheck( NULL );
	return( 0 );
}

#endif
