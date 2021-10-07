//
// CScript.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CSCRIPT_H
#define _INC_CSCRIPT_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CMemBlock.h"
#include "CArray.h"
#include "CFile.h"


#define GRAY_SCRIPT		".scp"
#define GRAY_BINARY		".bin"

#define SCRIPT_MAX_SECTION_LEN 128

struct CScriptLineContext
{
public:
	long m_lOffset;
	int m_iLineNum;		// for debug purposes if there is an error.
public:
	void Init()
	{
		m_lOffset = -1;
		m_iLineNum = -1;
	}
	CScriptLineContext()
	{
		Init();
	}
	bool IsValid() const
	{
		return( m_lOffset >= 0 );
	}
};

class CScriptKey
{
	// A single line form a script file.
	// This is usually in the form of KEY=ARG
	// Unknown allocation of pointers.
protected:
	TCHAR * m_pszKey;		// The key. (or just start of the line)
	TCHAR * m_pszArg;		// for parsing the last read line.	KEY=ARG or KEY ARG

public:
	bool IsKey( LPCTSTR pszName ) const
	{
		ASSERT(m_pszKey);
		return( ! strcmpi( m_pszKey, pszName ));
	}
	bool IsKeyHead( LPCTSTR pszName, int len ) const
	{
		ASSERT(m_pszKey);
		return( ! strnicmp( m_pszKey, pszName, len ));
	}

	void InitKey();

	// Query the key.
	LPCTSTR GetKey() const
	{
		// Get the key or section name.
		ASSERT(m_pszKey);
		return(m_pszKey);
	}

	// Args passed with the key.
	bool HasArgs() const
	{
		ASSERT(m_pszArg);
		return(( m_pszArg[0] ) ? true : false );
	}
	TCHAR * GetArgRaw() const	// Not need to parse at all.
	{
		ASSERT(m_pszArg);
		return(m_pszArg);
	}

	TCHAR * GetArgStr( bool * fQuoted );	// this could be a quoted string ?
	TCHAR * GetArgStr()
	{
		return GetArgStr( NULL );
	}
	long GetArgVal();
	long GetArgRange();
	DWORD GetArgFlag( DWORD dwStart, DWORD dwMask );

	CScriptKey() :
		m_pszKey( NULL ),
		m_pszArg( NULL )
	{
	}
	CScriptKey( TCHAR * pszKey, TCHAR * pszArg ) :
		m_pszKey( pszKey ),
		m_pszArg( pszArg )
	{
	}
};

class CScriptKeyAlloc : public CScriptKey
{
	// Dynamic allocated script key.
protected:
	CMemLenBlock m_Mem;	// the buffer to hold data read.

protected:
	TCHAR * GetKeyBufferRaw( int iSize );

	bool ParseKey( LPCTSTR pszKey );
	bool ParseKey( LPCTSTR pszKey, LPCTSTR pszArgs );
	int ParseKeyEnd();

public:
	TCHAR * GetKeyBuffer()
	{
		// Get the buffer the key is in. 
		ASSERT(m_Mem.GetData());
		return((TCHAR *) m_Mem.GetData());
	}
	void ParseKeyLate();

	CScriptKeyAlloc()
	{
	}
	virtual ~CScriptKeyAlloc()
	{
	}
};

class CScript : public CFileText, public CScriptKeyAlloc
{
private:
	bool m_fSectionHead;	// File Offset to current section header. [HEADER]
	long m_lSectionData;	// File Offset to current section data, under section header.

public:
	int m_iLineNum;		// for debug purposes if there is an error.
protected:
	void InitBase();

#if 0
	void EndSection();
	void WriteBinLength( DWORD dwLen );
	DWORD ReadBinLength();
	int CompressBin( int iLen );
	int DeCompressBin( int iLen );
#endif

	virtual LONG Seek( long offset = 0, UINT origin = SEEK_SET );

	bool WriteProfileStringDst( CScript * pDst, LPCTSTR pszSection, long lSectionOffset, LPCTSTR pszKey, LPCTSTR pszVal );
	bool WriteProfileStringBase( LPCTSTR pszSection, long lSectionOffset, LPCTSTR pszKey, LPCTSTR pszVal );

public:
	// text only functions:
	virtual bool ReadTextLine( bool fRemoveBlanks );	// looking for a section or reading strangly formated section.
	bool FindTextHeader( LPCTSTR pszName ); // Find a section in the current script

	bool WriteProfileStringOffset( long lSectionOffset, LPCTSTR pszKey, LPCTSTR pszVal )
	{
		return( WriteProfileStringBase( NULL, lSectionOffset, pszKey, pszVal ));
	}
	bool WriteProfileStringSec( LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszVal )
	{
		return( WriteProfileStringBase( pszSection, 0, pszKey, pszVal ));
	}

public:
	virtual bool Open( LPCTSTR szFilename = NULL, UINT Flags = OF_READ|OF_TEXT );
	virtual void Close();
	virtual void CloseForce()
	{
		CScript::Close();
	}

	bool SeekContext( CScriptLineContext LineContext )
	{
		m_iLineNum = LineContext.m_iLineNum;
		return( Seek( LineContext.m_lOffset, SEEK_SET ) == LineContext.m_lOffset );
	}
	CScriptLineContext GetContext() const
	{
		CScriptLineContext LineContext;
		LineContext.m_iLineNum = m_iLineNum;
		LineContext.m_lOffset = GetPosition();
		return( LineContext );
	}

	// Find sections.
	bool FindNextSection();
	virtual bool FindSection( LPCTSTR pszName, UINT uModeFlags ); // Find a section in the current script
	LPCTSTR GetSection() const
	{
		DEBUG_CHECK( (long) GetPosition() == m_lSectionData );
		ASSERT(m_pszKey);
		return( m_pszKey );
	}
	bool IsSectionType( LPCTSTR pszName ) //const
	{
		// Only valid after FindNextSection()
		DEBUG_CHECK( (long) GetPosition() == m_lSectionData );
		return( ! strcmpi( GetKey(), pszName ));
	}

	// Find specific keys in the current section.
	bool FindKey( LPCTSTR pszName ); // Find a key in the current section

	// read the sections keys.
	bool ReadKey( bool fRemoveBlanks = true );
	bool ReadKeyParse();

	// Write stuff out to a script file.
	bool _cdecl WriteSection( LPCTSTR pszSection, ... );
	bool WriteKey( LPCTSTR pszKey, LPCTSTR pszVal );
	void _cdecl WriteKeyFormat( LPCTSTR pszKey, LPCTSTR pszFormat, ... );

	void WriteKeyVal( LPCTSTR pszKey, int dwVal )
	{
		WriteKeyFormat( pszKey, "%d", dwVal );
	}
	void WriteKeyHex( LPCTSTR pszKey, DWORD dwVal )
	{
		WriteKeyFormat( pszKey, "0%x", dwVal );
	}

	CScript();
	CScript( LPCTSTR pszKey );
	CScript( LPCTSTR pszKey, LPCTSTR pszVal );
	virtual ~CScript()
	{
	}
};

#endif // _INC_CSCRIPT_H
