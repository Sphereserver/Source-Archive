//
// CScriptObj.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CSCRIPTOBJ_H
#define _INC_CSCRIPTOBJ_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CScript.h"

class CChar;
class CScriptTriggerArgs;
class CScriptObj;

#if 0

struct CScriptStackFrame
{
	TRIGRUN_TYPE m_runtype;	// what type of stack frame is this ?
};


class CScriptWait : public CScript
{
	// A pending script execution.
	// Save the state of the script running.
private:
	// State stack !
	CScriptStackFrame m_StateStack[ 128 ];
	int m_StateStackIndex;

	// preserve CScriptTriggerArgs also ?
	time_t m_dwWakeTime;	// When is the wait over. (in CWorld time)

	CScriptObj * m_pBase;
	CTextConsole * m_pSrc;	// Source of the script wait in the first place. (How to guarantee reliability ?)

public:
	void RunScript();
};

#endif

enum TRIGRUN_TYPE
{
	TRIGRUN_SECTION_EXEC,	// Just execute this. (first line already read!)
	TRIGRUN_SECTION_TRUE,	// execute this section normally.
	TRIGRUN_SECTION_FALSE,	// Just ignore this whole section.
	TRIGRUN_SINGLE_EXEC,	// Execute just this line or blocked segment. (first line already read!)
	TRIGRUN_SINGLE_TRUE,	// Execute just this line or blocked segment.
	TRIGRUN_SINGLE_FALSE,	// ignore just this line or blocked segment.
};

enum TRIGRET_TYPE	// trigger script returns.
{
	TRIGRET_RET_FALSE = 0,	// default return. (script might not have been handled)
	TRIGRET_RET_TRUE = 1,
	TRIGRET_RET_DEFAULT,	// we just came to the end of the script.
	TRIGRET_ENDIF,
	TRIGRET_ELSE,
	TRIGRET_ELSEIF,
	TRIGRET_QTY,
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
	// A base class for any class that can act like a console and issue commands.
	// CClient, CChar, CServer, CFileConsole
protected:
	int OnConsoleKey( CGString & sText, TCHAR nChar, bool fEcho );
public:
	// What privs do i have ?
	virtual PLEVEL_TYPE GetPrivLevel() const = 0;
	virtual LPCTSTR GetName() const = 0;	// ( every object must have at least a type name )
	virtual CChar * GetChar() const;	// are we also a CChar ? dynamic_cast ?

	virtual bool IsAnsi() const	// not used yet.
	{
		// can we use ANSI escape codes here ? for colors and sounds ?
		return( false );
	}
	virtual void SysMessage( LPCTSTR pszMessage ) const = 0;	// Feed back message.
	int VSysMessage( LPCTSTR pszFormat, va_list args ) const
	{
		TCHAR szTemp[ SCRIPT_MAX_LINE_LEN ];
		size_t ilen = vsprintf( szTemp, pszFormat, args );
		SysMessage( szTemp );
		return( ilen );
	}
	int _cdecl SysMessagef( LPCTSTR pszFormat, ... ) const
	{
		va_list vargs;
		va_start( vargs, pszFormat );
		int iRet = VSysMessage( pszFormat, vargs );
		va_end( vargs );
		return( iRet );
	}
};

class CScriptObj
{
	// This object can be scripted. (but might not be)

#define SKIP_SEPERATORS(p)	while ( *(p)=='.' ) { (p)++; }	// || ISWHITESPACE(*(p))
#define SKIP_ARGSEP(p)	while ( *(p)== ',' || isspace(*p) ){ (p)++; }

	static LPCTSTR const sm_szScriptKeys[];
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];

private:
	TRIGRET_TYPE OnTriggerForLoop( CScript &s, int iType, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult );
public:
	TRIGRET_TYPE OnTriggerScript( CScript &s, LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs = NULL );
	virtual TRIGRET_TYPE OnTrigger( LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs = NULL )
	{
		UNREFERENCED_PARAMETER(pszTrigName);
		UNREFERENCED_PARAMETER(pSrc);
		UNREFERENCED_PARAMETER(pArgs);
		return( TRIGRET_RET_DEFAULT );
	}
	bool OnTriggerFind( CScript & s, LPCTSTR pszTrigName );
	TRIGRET_TYPE OnTriggerRun( CScript &s, TRIGRUN_TYPE trigger, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pReturn = NULL );

	virtual LPCTSTR GetName() const = 0;	// ( every object must have at least a type name )

	// Flags = 1 = html
	int ParseText( TCHAR * pszResponse, CTextConsole * pSrc, int iFlags = 0, CScriptTriggerArgs * pArgs = NULL );

#ifndef GRAY_CLIENT
	static void r_DumpKeys( CTextConsole * pSrc, LPCTSTR const * pszKeys, int iElemSize = sizeof(LPCTSTR) );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc )
	{
		r_DumpKeys(pSrc,sm_szLoadKeys);
	}
	virtual void r_DumpVerbKeys( CTextConsole * pSrc )
	{
		r_DumpKeys(pSrc,sm_szVerbKeys);
	}
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	bool r_Call( LPCTSTR pszFunction, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * psVal = NULL, TRIGRET_TYPE * piRet = NULL );

	bool r_SetVal( LPCTSTR pszKey, LPCTSTR pszVal )
	{
		CScript s( pszKey, pszVal );
		bool result = r_LoadVal( s );
		return result;
	}
#endif
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_Load( CScript & s );

	virtual ~CScriptObj()
	{
	}
};

class CScriptTriggerArgs : public CScriptObj
{
	// All the args an event will need.
	static LPCTSTR const sm_szLoadKeys[];
public:
	int							m_iN1;		// "ARGN" or "ARGN1" = a modifying numeric arg to the current trigger.
	int							m_iN2;		// "ARGN2" = a modifying numeric arg to the current trigger.
	int							m_iN3;		// "ARGN3" = a modifying numeric arg to the current trigger.

	CScriptObj *				m_pO1;		// "ARGO" or "ARGO1" = object 1
											// these can go out of date ! get deleted etc.							

	CGString					m_s1;		// ""ARGS" or "ARGS1" = string 1
	CGString					m_s1_raw;	// RAW, used to build argv in runtime

	CGPtrTypeArray	<LPCTSTR>	m_v;

	CVarDefArray 				m_VarsLocal;

public:
	void Init( LPCTSTR pszStr );

	CScriptTriggerArgs() :
		m_iN1(0),  m_iN2(0), m_iN3(0)
	{
		m_pO1 = NULL;
	}

	CScriptTriggerArgs( LPCTSTR pszStr );

	CScriptTriggerArgs( CScriptObj * pObj ) :
		m_iN1(0),  m_iN2(0), m_iN3(0), m_pO1(pObj)
	{
	}
	CScriptTriggerArgs( int iVal1 ) :
		m_iN1(iVal1),  m_iN2(0), m_iN3(0)
	{
		m_pO1 = NULL;
	}
	CScriptTriggerArgs( int iVal1, int iVal2, int iVal3 = 0 ) :
		m_iN1(iVal1), m_iN2(iVal2), m_iN3(iVal3)
	{
		m_pO1 = NULL;
	}
	CScriptTriggerArgs( int iVal1, int iVal2, CScriptObj * pObj ) :
		m_iN1(iVal1), m_iN2(iVal2), m_iN3(0), m_pO1(pObj)
	{
	}

	bool r_Verb( CScript & s, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s );
	bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc );
	LPCTSTR GetName() const
	{
		return( _TEXT("ARG"));
	}
};

#if 0

class CScriptDebug : public CScriptObj, public CMemDynamic
{
	// Use this for debugging scripts.
	// A general context to track for 

protected:
	DECLARE_MEM_DYNAMIC;
private:
	CTextConsole * m_pSrc;		// This is the party intersted in this event.
	const CScriptObj * m_pObj;	// The object of interest.
	const CScript * m_pScript;	// The script we are interested in.
	long m_iLineNum;	// what line number in the script file is this ?

#define SCRIPTOBJ_BREAK		0x001	// Break when we hit this.
#define SCRIPTOBJ_MSG		0x002	// just send a msg when this happens.

	DWORD m_dwFlags;		


};

#endif

#endif	// _INC_CSCRIPTOBJ_H
