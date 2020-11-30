//
// CScript.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CSCRIPT_H
#define _INC_CSCRIPT_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cstring.h"
#include "cfile.h"
#include "carray.h"

class CScriptLink;

class CScript : public CFileText
{
private:
	CGString m_Buffer;		// the buffer to hold data read.

	long m_lSectionHead;	// File Offset to section header.
	long m_lSectionData;	// File Offset to section data, not section header.
	DWORD m_lSectionLength;	// For binary files we know the size in advance.
	int m_iLineNum;		// for debug purposes if there is an error.

public:
	TCHAR * m_pArg;		// for parsing the last read line.

private:
	void EndSection();
	void Init()
	{
		m_iLineNum = 0;
		m_pArg = NULL;
		m_lSectionHead = 0;
		m_lSectionData = 0;
	}
	TCHAR * GetArgNextStr();

	void WriteBinLength( DWORD dwLen );
	DWORD ReadBinLength();
	int CompressBin( int iLen );
	int DeCompressBin( int iLen );

public:
	// text only functions:
	bool FindTextHeader( const TCHAR *pName, CScriptLink *pPrev = NULL, WORD wModeFlags = 0 ); // Find a section in the current script
	bool ReadTextLine( bool fRemoveBlanks );	// looking for a section or reading strangly formated section.
	bool WriteProfileString( CScript * pDst, const TCHAR * pszSection, const TCHAR * pszKey, const TCHAR * pszVal );
	bool WriteProfileString( const TCHAR * pszSection, const TCHAR * pszKey, const TCHAR * pszVal );

public:
	CScript()
	{
		Init();
	}
	CScript( const TCHAR * pKey )
	{
		Init();
		m_Buffer.Copy( pKey );
		m_pArg = m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN);
		GetArgNextStr();
	}
	CScript( const TCHAR * pKey, const TCHAR * pVal );

#ifdef GRAY_SVR
	bool OpenFind( const TCHAR *pszFilename, WORD Flags = OF_READ );
#endif

private:
	bool Seek( long offset = 0, int origin = SEEK_SET );
protected:
	virtual bool OpenCopy( const CScript & s, WORD wFlags = OF_READ );
public:
	virtual bool Open( const TCHAR *szFilename = NULL, WORD Flags = OF_READ );
	virtual bool Close();

	bool SeekPos( long offset )
	{
		// Only rarely would we want to just seek directly in a script.
		return( Seek( offset, SEEK_SET ));
	}
	bool SeekLine( long offset, int iLineNum )
	{
		m_iLineNum = iLineNum;
		return( Seek( offset, SEEK_SET ));
	}

	int GetLineNumber() const
	{
		return( m_iLineNum );
	}
	TCHAR * GetData()
	{
		return( m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN));
	}
	TCHAR * GetKey()
	{
		return( m_Buffer.GetBuffer(MAX_SCRIPT_LINE_LEN));
	}
	TCHAR * GetArgStr() const
	{
		return( m_pArg );
	}
	long GetArgVal();
	long GetArgRange()
	{
		return( GetArgVal());
	}
	DWORD GetArgHex();
	bool HasArgs() const
	{
		return(( m_pArg[0] ) ? true : false );
	}

	// find sections.
	bool FindNextSection();
	virtual bool FindSection( const TCHAR * pName, WORD wModeFlags = 0, CScriptLink *pPrev = NULL ); // Find a section in the current script

	bool IsSectionType( const TCHAR *pName ) //const
	{
		return( ! strcmpi( GetData(), pName ));
	}

	// Find specific keys in the current section.
	bool FindKey( const TCHAR *pName ); // Find a key in the current section 
	bool IsKey( const TCHAR *pName ) //const
	{
		return( ! strcmpi( GetKey(), pName ));
	}
	bool IsKeyHead( const TCHAR *pName, int len ) //const
	{
		return( ! strnicmp( GetKey(), pName, len ));
	}

	// read the sections keys.
	bool ReadKey( bool fRemoveBlanks = true );
	bool ReadKeyParse();

	// Write stuff out to a script file.
	bool _cdecl WriteSection( const TCHAR * pszSection, ... );
	bool WriteKey( const TCHAR * pszKey, const TCHAR * pszVal );
	void _cdecl WriteKeyFormat( const TCHAR * pszKey, const TCHAR * pszFormat, ... );

	void WriteKeyVal( const TCHAR * pszKey, int dwVal )
	{
		WriteKeyFormat( pszKey, "%d", dwVal );
	}
	void WriteKeyHex( const TCHAR * pszKey, DWORD dwVal )
	{
		WriteKeyFormat( pszKey, "0%x", dwVal );
	}
};

#ifndef _AFXDLL

class CScriptLock : public CScript
{
	// Open a script that might already be open.
	// preserve the previous openers offset in the script.
private:
	long m_lPrvOffset;
	const CScript * m_pPrvScriptContext;
private:
	void Init()
	{
		m_lPrvOffset = -1;	// means the script was NOT open.
		m_pPrvScriptContext = NULL;
	}
public:
	CScriptLock()
	{
		Init();
	}
	~CScriptLock()
	{
		Close();
	}
	bool Open( const TCHAR *szFilename = NULL, WORD Flags = OF_READ );
	bool OpenCopy( const CScript & s, WORD uFlags = OF_READ );
	bool Close();

#ifdef GRAY_SVR
	bool FindSection( const TCHAR *pszName, WORD wModeFlags=0, CScriptLink *pPrev=NULL );
#endif		
};

class CScriptLink
{
	// A pre-indexed link into a script file.
private:
	CScript * m_pScript;	// we already found the script.
	long m_lOffset;		// we already found the offset.
	long m_iLineNum;
public:
	void InitLink()
	{
		// Clear the link.
		m_lOffset = -1;	// not yet tested.
		m_iLineNum = -1;
	}
	CScriptLink()
	{
		m_pScript = NULL;
		InitLink();
	}
	bool IsLinked() const	// been loaded from the scripts ?
	{
		if ( m_pScript == NULL ) 
			return( false );
		return( m_lOffset >= 0 ? true : false );
	}
	bool IsBadLinked() const	// tried this but failed b4.
	{
		return( m_lOffset < -1 );
	}
	DWORD GetBadLinkOffset() const
	{
		// No check for valid range.
		return( m_lOffset );
	}
	void SetBadLinkOffset( int iCode )
	{
		ASSERT( iCode < -1 );
		m_lOffset = iCode;
	}
	void SetLinkUnk( CScript * pScriptBase )
	{
		DEBUG_CHECK(pScriptBase);
		m_pScript = pScriptBase;
		InitLink();
	}
	void SetLink( CScript * pScriptBase, CScript & s )
	{
		DEBUG_CHECK(pScriptBase);
		m_pScript = pScriptBase;
		m_lOffset = s.GetPosition();
		DEBUG_CHECK(m_lOffset>=-1);
		m_iLineNum = s.GetLineNumber();
	}
	DWORD GetLinkOffset() const
	{
		DEBUG_CHECK(IsLinked());
		return( m_lOffset );
	}
	void ClearLinkOffset()
	{
		m_lOffset = -1;
	}
	DWORD GetLinkLineNum() const
	{
		DEBUG_CHECK(IsLinked());
		return( m_iLineNum );
	}
	CScript * GetLinkScript() const
	{
		DEBUG_CHECK(m_pScript);
		return( m_pScript );
	}
	bool OpenLinkLock( CScriptLock & s )
	{
		// RETURN: true = found it.
		if ( ! IsLinked())
			return( false );
		if ( ! s.OpenCopy( *m_pScript, OF_READ ))
		{
			SetBadLinkOffset( -2 ); // tried this and failed so don't bother trying again.
			return( false );
		}
		if ( ! s.SeekLine( m_lOffset, m_iLineNum ))
		{
			SetBadLinkOffset( -3 ); // tried this and failed so don't bother trying again.
			return( false );
		}
		return( true );
	}
    const CScriptLink& operator=( const CScriptLink & link )
    {
		m_pScript = link.m_pScript;
		m_lOffset = link.m_lOffset;
		m_iLineNum = link.m_iLineNum;
		return( *this );
    }
};

class CScriptIndexLink : public CScriptLink
{
private:
	const DWORD m_dwIndex;
public:
	CScriptIndexLink( DWORD index, CScript * pScript, CScript & s ) :
		m_dwIndex( index )
	{
		SetLink( pScript, s );
	}
	DWORD GetIndexID() const
	{
		return( m_dwIndex );
	}
};

#if 0

class CScriptWait : public CScriptLink
{
	// A pending script execution.
	// Save the state of the script running. 
private:
	// State stack !
	TRIGRUN_TYPE m_StateStack[ 128 ];
	int m_StateStackIndex;

	CObjUID m_uidBase;	// Array sorted by base object.
	time_t m_dwWakeTime;	// When is the wait over. (in world time) 

	CTextConsole * m_pSrc;	// Source of the script wait in the first place. (How to guarantee reliability ?)

public:

	void RunScript();

};

#endif

#endif	// _AFXDLL

enum TRIGRUN_TYPE
{
	TRIGRUN_SECTION_EXEC,	// Just execute this. (first line already read!)
	TRIGRUN_SECTION_FALSE,	// Just ignore this whole section.
	TRIGRUN_SECTION_TRUE,	// execute this section normally.
	TRIGRUN_SECTION_SINGLE,	// Execute just this line or blocked segment.
};

enum TRIGRET_TYPE	// trigger script returns.
{
	TRIGRET_RET_FALSE = 0,	// default return.
	TRIGRET_RET_TRUE = 1,
	TRIGRET_RET_DEFAULT,	// we just came to the end of the script.
	TRIGRET_ENDIF,
	TRIGRET_ELSE,
	TRIGRET_ELIF_FALSE,
};

enum PLEVEL_TYPE		// Priv levels.
{
	PLEVEL_Guest = 0,		// 0 = This is just a guest account. (cannot PK)
	PLEVEL_Player,			// 1 = Player or NPC.
	PLEVEL_Counsel,			// 2 = Can travel and give advice.
	PLEVEL_Seer,			// 3 = Can make things and NPC's but not directly bother players.
	PLEVEL_GM,				// 4 = GM command clearance
	PLEVEL_Dev,				// 5 = Not bothererd by GM's
	PLEVEL_Admin,			// 6 = Can switch in and out of gm mode. assign GM's
	PLEVEL_Owner,			// 7 = Highest allowed level.
	PLEVEL_QTY,
};

class CTextConsole
{
	// A base class for any class that can act like a console.
	// CClient, CChar, CServer
protected:
	int OnConsoleKey( CGString & sText, TCHAR nChar, bool fEcho );
public:
	virtual PLEVEL_TYPE GetPrivLevel() const = 0;
	virtual const TCHAR * GetName() const = 0;	// ( every object must have at least a type name )

	virtual CChar * GetChar() const;	// are we also a Cchar ? dynamic_cast ?
	virtual void SysMessage( const TCHAR * pszMessage ) const = 0;	// Feed back message.

	void VSysMessage( const TCHAR * pszFormat, va_list args ) const
	{
		TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
		size_t len = vsprintf( szTemp, pszFormat, args );
		SysMessage( szTemp );
	}
	void _cdecl SysMessagef( const TCHAR * pszFormat, ... ) const
	{
		va_list args;
		va_start( args, pszFormat );
		VSysMessage( pszFormat, args );
	}
};

class CScriptObj
{
#define SKIP_SEPERATORS(p)	while ( *(p)=='.' || ISWHITESPACE(*(p))) { (p)++; }

public:
	static int sm_iTrigArg;			// a modifying arg to the current trigger.

	// This object can be scripted. (but might not be)
private:
	TRIGRET_TYPE OnTriggerForLoop( CScript &s, int iType, CTextConsole * pSrc );
protected:
	TRIGRET_TYPE OnTriggerScript( CScript &s, const TCHAR * pszTrigName, CTextConsole * pSrc = NULL, int iArg = 0 );
	virtual bool OnTrigger( const TCHAR * pTrigName, CTextConsole * pSrc, int iArg )
	{
		return( false );
	}
public:
	TRIGRET_TYPE OnTriggerRun( CScript &s, TRIGRUN_TYPE trigger, CTextConsole * pSrc = NULL, int iArg = 0 );

	virtual const TCHAR * GetName() const = 0;	// ( every object must have at least a type name )
	int ParseText( TCHAR * pszResponse, CTextConsole * pSrc, TCHAR chBegin = '<', TCHAR chEnd = '>' );

	virtual bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s )
	{
		return( false );
	}
	virtual bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	virtual bool r_Load( CScript & s )
	{
		while ( s.ReadKeyParse())
		{
			r_LoadVal(s);
		}
		return( true );
	}
	bool r_SetVal( const TCHAR * pszKey, const TCHAR * pszVal )
	{
		CScript s( pszKey, pszVal );
		return( r_LoadVal( s ));
	}
};

#endif // _INC_CSCRIPT_H