//
// CGFile.h
// Copyright Menace Software (www.menasoft.com).

#ifndef _INC_CFILE_H
#define _INC_CFILE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cstring.h"

#ifndef OF_WRITE
#define OF_READ             0x00
#define OF_WRITE            0x01
#define OF_READWRITE        0x02
#define OF_SHARE_DENY_NONE	0x00
#endif
#define OF_SORTED			0x1000	// the file is sorted so we can use that for indexing.
#define OF_NONCRIT			0x4000	// just a test.
#define OF_TEXT				0x2000
#define OF_BINARY			0x8000

class CGFile	// try to be compatible with MFC CFile class.
{
private:
	WORD m_wMode;
protected:
	CGString m_sFileName;
protected:
	CGFile()
	{
		m_wMode = 0;
	}

protected:
	void SetMode( WORD uMode ) { m_wMode = uMode; } 	// Change file open mode.
	virtual bool OpenBase( void FAR * pExtra ) = 0;
#ifndef _AFXDLL
	virtual bool OpenBaseCopy( const CGFile & file ) = 0;
	virtual void CloseCopy() = 0;
	virtual bool OpenCopy( const CGFile & file, WORD uMode = OF_READ | OF_SHARE_DENY_NONE );
#endif

public:
	virtual bool IsFileOpen() const = 0;
	virtual bool IsBinaryMode() const = 0;

	const TCHAR * GetFilePath() const	// for compatibility with MFC
	{
		return( m_sFileName);
	}
	const TCHAR * GetFileName() const	// for compatibility with MFC
	{
		return( m_sFileName);
	}
	const TCHAR * GetFileTitle() const;
	virtual bool SetFilePath( const TCHAR * pszName );
	WORD GetMode() const
	{
		return( m_wMode & 0xFF );	// get rid of OF_NONCRIT type flags
	}
	bool IsWriteMode() const
	{
		return(( m_wMode & OF_WRITE ) ? true : false );
	}

	virtual bool Open( const TCHAR * pszName = NULL, WORD uMode = OF_READ | OF_SHARE_DENY_NONE, void FAR * pExtra = NULL );

	virtual bool Close( void ) = 0;
	virtual DWORD GetPosition() const = 0;
	virtual bool Seek( long lOffset, int iOrigin = SEEK_SET ) = 0;
	virtual size_t Read( void FAR * pData, size_t dwLength ) const = 0;
	virtual bool Write( const void FAR * pData, size_t dwLength ) const = 0;

	void SeekToBegin()
	{
		Seek( 0, SEEK_SET );
	}
	DWORD SeekToEnd()
	{
		if ( ! Seek( 0, SEEK_END ))
			return( 0 );
		return( GetPosition());
	}
	DWORD GetLength()
	{
		// Get the size of the file.
		DWORD lPos = GetPosition();	// save current pos.
		DWORD lSize = SeekToEnd();
		Seek( lPos, SEEK_SET );	// restore previous pos.
		return( lSize );
	}
};

class CFileText : public CGFile
{
private:
	FILE * m_pFile;		// the current open script type file.
private:
	const TCHAR * GetModeStr() const;
protected:
	bool OpenBase( void FAR * pExtra );
#ifndef _AFXDLL
	bool OpenBaseCopy( const CGFile & file );
	void CloseCopy()
	{
		m_pFile = NULL;
	}
#endif
public:
	CFileText()
	{
		m_pFile = NULL;
	}
	~CFileText()
	{
		Close();
	}
	bool IsFileOpen() const
	{
		return( m_pFile != NULL );
	}
	bool IsBinaryMode() const { return false; }
	bool IsEOF() const
	{
		if ( ! IsFileOpen()) 
			return( true );
		return(( feof( m_pFile )) ? true : false );
	}

	bool Close();
	virtual bool Seek( long offset = 0, int origin = SEEK_SET )
	{
		// true = success
		if ( ! IsFileOpen())
			return( false );
		if ( offset < 0 ) 
			return( false );
		return( fseek( m_pFile, offset, origin ) == 0 );
	}
	void Flush() const
	{
		if ( ! IsFileOpen()) 
			return;
		ASSERT(m_pFile);
		fflush(m_pFile);
	}
	DWORD GetPosition() const
	{
		// RETURN: -1 = error.
		if ( ! IsFileOpen()) 
			return -1;
		return( ftell(m_pFile));
	}
	size_t Read( void FAR * pBuffer, size_t sizemax ) const
	{
		// This can return: EOF(-1) constant.
		// returns the number of full items actually read
		if ( IsEOF())
			return( 0 );	// LINUX will ASSERT if we read past end.
		return( fread( pBuffer, 1, sizemax, m_pFile ));
	}
	TCHAR * ReadLine( TCHAR FAR * pBuffer, size_t sizemax ) const
	{
		// Read a line of text. NULL = EOF
		if ( IsEOF()) 
			return( NULL );	// LINUX will ASSERT if we read past end.
		return( fgets( pBuffer, sizemax, m_pFile ));
	}
	bool Write( const void FAR * pData, size_t iLen ) const
	{
		// RETURN: 1 = success else fail.
		if ( ! IsFileOpen())
			return( false );
		return( fwrite( pData, iLen, 1, m_pFile ) == 1 );
	}
	bool WriteStr( const TCHAR * pStr )
	{
		// RETURN: < 0 = failed.
		return( Write( pStr, strlen( pStr )));
	}
	size_t VPrintf( const TCHAR * pFormat, va_list args )
	{
		if ( ! IsFileOpen())
			return( -1 );
		TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
		size_t len = vsprintf( szTemp, pFormat, args );
		size_t lenret = fwrite( szTemp, 1, len, m_pFile );
		if ( lenret != len ) 
			return( -1 );
		return( lenret );
	}
	size_t _cdecl Printf( const TCHAR * pFormat, ... )
	{
		va_list args;
		va_start( args, pFormat );
		return( VPrintf( pFormat, args ));
	}
};

#ifdef _WIN32 

#ifndef HFILE_ERROR
#define HFILE_ERROR	-1
#define HFILE unsigned int
#endif	// HFILE_ERROR

#ifdef _AFXDLL

class CFileBin : public CFile
{
public:
	bool IsFileOpen() const
	{
		return( m_hFile!=hFileNull );
	}
	bool Seek( long lOffset = 0, int iOrigin = SEEK_SET )
	{
		return( CFile::Seek( lOffset, iOrigin ) != -1 );
	}
};

#else

class CFileBin : public CGFile
{
private:
	HFILE m_hFile;	// HFILE_ERROR

protected:
	bool OpenBase( void FAR * pExtra )
	{
		OFSTRUCT ofs;
		m_hFile = OpenFile( GetFilePath(), &ofs, GetMode());
		return( IsFileOpen());
	}
#ifndef _AFXDLL
	bool OpenBaseCopy( const CGFile & file )
	{
		const CFileBin * pFile = dynamic_cast <const CFileBin *> ( &file );
		ASSERT(pFile);
		if ( ! pFile )
			return( false );
		m_hFile = pFile->m_hFile;
		return( false );	// not used anyhow.
	}
	void CloseCopy()
	{
		m_hFile = HFILE_ERROR;
	}
#endif

public:
	CFileBin()
	{
		CloseCopy();
	}
	~CFileBin()
	{
		Close();
	}
	bool IsFileOpen() const
	{
		return( m_hFile != HFILE_ERROR );
	}
	bool IsBinaryMode() const { return true; }

	bool Close()
	{
		if ( ! IsFileOpen())
			return( true );
		HFILE hRet = _lclose( m_hFile );
		m_hFile = HFILE_ERROR;
		return( hRet == 0 );
	}
	DWORD GetPosition() const
	{
		return( _llseek( m_hFile, 0, SEEK_CUR ));
	}
	bool Seek( long lOffset = 0, int iOrigin = SEEK_SET )
	{
		// true = success.
		return( _llseek( m_hFile, lOffset, iOrigin ) != HFILE_ERROR );
	}
	size_t Read( void FAR * pData, size_t dwLength ) const
	{
		// RETURN: length of the read data.
		return( _hread( m_hFile, pData, (long) dwLength ));
	}
	bool Write( const void FAR * pData, size_t dwLength ) const
	{
		return( _hwrite( m_hFile, (const char *) pData, (long) dwLength ) == (long) dwLength );
	}
};

#endif	// _AFXDLL

#else	// _WIN32

class CFileBin : public CFileText
{
public:
	bool Open( const TCHAR * pszName = NULL, WORD uMode = OF_READ | OF_SHARE_DENY_NONE, void FAR * pExtra = NULL )
	{
		return( CGFile::Open( pszName, uMode | OF_BINARY, pExtra ));
	}
};

#endif	// ! _WIN32

#ifdef _INC_MMSYSTEM	// not used.

class CMMFile : public CGFile
{
private:
	HMMIO m_hFile;
protected:
	DWORD CreateChunkBase( MMCKINFO * pInfoChk, DWORD dwFlags = 0 ) const
	{
		return( mmioCreateChunk( (HMMIO) m_hFile, pInfoChk, dwFlags ));
	}
	virtual bool OpenBase( void FAR * pExtra = NULL )
	{
		m_hFile = mmioOpen( (LPSTR)(LPCSTR) m_sFileName, (LPMMIOINFO) pExtra, GetMode() );
		return( m_hFile != NULL );
	}
#ifndef _AFXDLL
	bool OpenBaseCopy( const CGFile & file )
	{
		return( Open( m_sFileName ));
	}
	void CloseCopy()
	{
		Close();
	}
#endif

public:
	CMMFile()
	{
		m_hFile = NULL;
	}
	~CMMFile()
	{
		Close();
	}
	bool IsFileOpen() const
	{
		return( m_hFile != NULL );
	}
	bool IsBinaryMode() const { return true; }

	DWORD Descend( MMCKINFO * pInfoChk, const MMCKINFO * pInfoRIFF, DWORD dwFlags ) const
	{
		return( mmioDescend( (HMMIO) m_hFile, pInfoChk, pInfoRIFF, dwFlags ));
	}
	DWORD Ascend( MMCKINFO * pInfo ) const
	{
		return( mmioAscend( (HMMIO) m_hFile, pInfo, 0 ));
	}

	virtual bool Close( void )
	{
		if ( ! IsFileOpen()) 
			return( true );
		MMRESULT mRes = mmioClose( (HMMIO) m_hFile, 0 );
		m_hFile = NULL;
		return( mRes == 0 );
	}
	virtual DWORD GetPosition() const
	{
		return( mmioSeek( (HMMIO) m_hFile, 0, SEEK_CUR ));
	}
	virtual bool Seek( LONG lOffset, int iOrigin )
	{
		return( mmioSeek( (HMMIO) m_hFile, lOffset, iOrigin ) != -1 );
	}
	virtual size_t Read( void FAR * pData, size_t dwLength ) const
	{
		return( mmioRead( (HMMIO) m_hFile, (HPSTR) pData, (LONG) dwLength ));
	}
	virtual bool Write( const void FAR * pData, size_t dwLength ) const
	{
		return( mmioWrite( (HMMIO) m_hFile, (HPSTR) pData, (LONG) dwLength ) == (LONG) dwLength );
	}

	DWORD DescendRIFF( MMCKINFO * pInfo, DWORD fccType ) const
	{
		pInfo->fccType = fccType;
		return( Descend( pInfo, NULL, MMIO_FINDRIFF ));
	}
	DWORD CreateRIFF( MMCKINFO * pInfo, DWORD fccType, DWORD dwFlags = MMIO_CREATERIFF ) const
	{
		pInfo->fccType = fccType;
		return( CreateChunkBase( pInfo, dwFlags ));
	}
	DWORD DescendCHUNK( MMCKINFO * pInfoCHUNK, const MMCKINFO * pInfoRIFF, DWORD ckid ) const
	{
		pInfoCHUNK->ckid = ckid;
		return( Descend( pInfoCHUNK, pInfoRIFF, MMIO_FINDCHUNK ));
	}
	DWORD CreateCHUNK( MMCKINFO * pInfo, DWORD ckid ) const
	{
		pInfo->ckid = ckid ;
		return( CreateChunkBase( pInfo ));
	}
};

#endif	// _INC_MMSYSTEM

extern CGString GetMergedFileName( const TCHAR *pszBase, const TCHAR *pszName );
extern void GetStrippedDirName( TCHAR * pszFilePath );

#endif // _INC_CFILE_H