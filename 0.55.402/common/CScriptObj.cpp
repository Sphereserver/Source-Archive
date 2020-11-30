//
// CScriptObj.cpp
// A scriptable object.
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#else
#include "graycom.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////
// -CScriptTriggerArgs

void CScriptTriggerArgs::Init( LPCTSTR pszStr )
{
	m_pO1			= NULL;

	if ( !pszStr )
		pszStr	= "";
	// raw is left untouched for now - it'll be split the 1st time argv is accessed
	m_s1_raw		= pszStr;
	if ( *pszStr == '"' )
		pszStr++;
	m_s1	= pszStr ;

	// take quote if present.
	char	* str;
	if ( (str = strchr( m_s1.GetBuffer(), '"' )) )
		*str	= '\0';
	
	m_iN1	= 0;
	m_iN2	= 0;
	m_iN3	= 0;

	// attempt to parse this.
	if ( isdigit(*pszStr) || ((*pszStr == '-') && isdigit(*(pszStr+1))) )
	{
		m_iN1 = Exp_GetSingle(pszStr);
		SKIP_ARGSEP( pszStr );
		if ( isdigit(*pszStr) || ((*pszStr == '-') && isdigit(*(pszStr+1))) )
		{
			m_iN2 = Exp_GetSingle(pszStr);
			SKIP_ARGSEP( pszStr );
			if ( isdigit(*pszStr) || ((*pszStr == '-') && isdigit(*(pszStr+1))) )
			{
				m_iN3 = Exp_GetSingle(pszStr);
			}
		}
	}
}



CScriptTriggerArgs::CScriptTriggerArgs( LPCTSTR pszStr )
{
	Init( pszStr );
}



bool CScriptTriggerArgs::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
{
	if ( ! strnicmp( pszKey, "ARGO.", 5 ))	// ARGO.NAME
	{
		pszKey += 5;
		pRef = m_pO1;
		return( true );
	}
	if ( ! strnicmp( pszKey, "ARGO1.", 6 ))
	{
		pszKey += 6;
		pRef = m_pO1;
		return( true );
	}
	return( false );
}


enum AGC_TYPE
{
	AGC_N,	// number
	AGC_N1,	// number
	AGC_N2,
	AGC_N3,
	AGC_PT,
	AGC_S,
	AGC_S1,	// string
	AGC_V,	// array values
	AGC_LVAR,
	AGC_QTY,
};

LPCTSTR const CScriptTriggerArgs::sm_szLoadKeys[AGC_QTY+1] =
{
	"ARGN",	// number
	"ARGN1",	// number
	"ARGN2",
	"ARGN3",
	"ARGPT",
	"ARGS",
	"ARGS1",	// string
	"ARGV",
	"LOCAL",
	NULL,
};


bool CScriptTriggerArgs::r_Verb( CScript & s, CTextConsole * pSrc )
{
	int	index;
	LPCTSTR		pszKey;
	pszKey	= s.GetKey();

	if ( !strnicmp( "LOCAL.", pszKey, 6 ) )
	{
		bool fQuoted = false;
		m_VarsLocal.SetStr( s.GetKey()+6, fQuoted, s.GetArgStr( &fQuoted ), false );
		return( true );
	}

	index = FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
	switch (index)
	{
	case AGC_N:
	case AGC_N1:
		m_iN1	= s.GetArgVal();
		return true;
	case AGC_N2:
		m_iN2	= s.GetArgVal();
		return true;
	case AGC_N3:
		m_iN3	= s.GetArgVal();
		return true;
	case AGC_S:
		Init( s.GetArgStr() );
		return true;
	default:
		return false;
	}
	return false;
}



bool CScriptTriggerArgs::r_LoadVal( CScript & s )
{
	return false;
}

bool CScriptTriggerArgs::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
{
	if ( g_Cfg.IsSetEF( EF_Intrinsic_Locals ) )
	{

		CVarDefBase *	pVar	= m_VarsLocal.GetKey( pszKey );
		if ( pVar )
		{
			sVal	= pVar->GetValStr();
			return true;
		}
	}
	else if ( !strnicmp( "LOCAL.", pszKey, 6 ) )
	{
		pszKey	+= 6;
		sVal	= m_VarsLocal.GetKeyStr( pszKey, true );
		return( true );
	}

	if ( !strnicmp( pszKey, "ARGV", 4 ))
	{
		pszKey+=4;
		SKIP_SEPERATORS(pszKey);
		
		int iQty = m_v.GetCount();
		if ( iQty == 0 )
		{
			// PARSE IT HERE
			TCHAR *		pszArg		= m_s1_raw.GetBuffer();
			TCHAR *		s			= pszArg;
			bool		fQuotes		= false;
			while ( *s )
			{
				if ( isspace(*s ) )	{ s++; continue; }
				
				if ( *s == '"' )	{ s++; fQuotes = true; };

				pszArg	= s;	// arg starts here
				s++;

				while (*s)
				{
					if ( *s == '"' )
					{
						if ( fQuotes )	{	*s	= '\0';	fQuotes = false;	break;	}
						*s = '\0';
						s++;
						fQuotes	= true;	// maintain
						break;
					}
					if ( !fQuotes && (*s == ',') )
					{ *s = '\0'; s++; break; }
					s++;
				}
				m_v.Add( pszArg );
			}
			iQty = m_v.GetCount();
		}	
		
		if ( *pszKey == '\0' )
		{
			sVal.FormatVal(iQty);
			return( true );
		}

		int iNum = Exp_GetSingle( pszKey );
		SKIP_SEPERATORS(pszKey);
		if ( !m_v.IsValidIndex(iNum) )
		{
			sVal.Format( "" );
			return true;
		}
		sVal.Format( m_v.GetAt(iNum) );
		return( true );
	}

	int index = FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
	switch (index)
	{
	case AGC_N:
	case AGC_N1:
		sVal.FormatVal( m_iN1 );
		break;
	case AGC_N2:
		sVal.FormatVal( m_iN2 );
		break;
	case AGC_N3:
		sVal.FormatVal( m_iN3 );
		break;
	case AGC_PT: // "PT",
		sVal.Format( "%d,%d,%d", m_iN1, m_iN2, m_iN3 );
		break;
	case AGC_S:
	case AGC_S1:
		sVal = m_s1;
		break;
	default:
		return( CScriptObj::r_WriteVal( pszKey, sVal, pSrc ));
	}
	return( true );
}

////////////////////////////////////////////////////////////////////////////////////////
// -CScriptObj

bool CScriptObj::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
{
#ifdef GRAY_SVR
	// A key name that just links to another object.

	if (   !strnicmp( pszKey, "SERV.", 5 )
		&& strnicmp( pszKey, "SERV.NEW", 8 ) )
	{
		pszKey += 5;
		pRef = &g_Serv;
		return( true );
	}

	if ( ! strnicmp( pszKey, "UID.", 4 ))
	{
		// Can it be resolved to any uid ref.
		pszKey += 4;
		CGrayUID uid = (DWORD) Exp_GetVal( pszKey );
		SKIP_SEPERATORS(pszKey);
		pRef = uid.ObjFind();
		return( true );
	}

	if ( ! strnicmp( pszKey, "OBJ.", 4 ))
	{
		pszKey += 4;
		pRef	= g_World.m_uidObj.ObjFind();
		return( true );
	}

	if ( ! strnicmp( pszKey, "NEW.", 4 ))
	{
		pszKey += 4;
		pRef	= g_World.m_uidNew.ObjFind();
		return( true );
	}

#endif

	if ( ! strnicmp( pszKey, "I.", 2 ))	// just talking about myself.
	{
		pszKey += 2;
		pRef = this;
		return( true );
	}

	return( false );
}

void CScriptObj::r_DumpKeys( CTextConsole * pSrc, LPCTSTR const * pszKeys, int iElemSize ) // static
{
	for ( int i=0; *pszKeys; i++ )
	{
		pSrc->SysMessage( *pszKeys );
		pSrc->SysMessage( "\n" );
		pszKeys = (LPCTSTR const *)(((BYTE*)pszKeys) + iElemSize );
	}
}

enum SSC_TYPE
{
	SSC_ASC,
	SSC_CATEGORY,
	SSC_CHR,
	SSC_DEF,
	SSC_DEF0,
	SSC_DEFMSG,
	SSC_DESCRIPTION,
	SSC_EVAL,
	SSC_FILELINES,
	SSC_FVAL,
	SSC_HVAL,
	SSC_ISEMPTY,
	SSC_ISNUM,
	SSC_LISTCOL,
	SSC_NEW,
	SSC_OBJ,
	SSC_QVAL,
	SSC_READFILE,
	SSC_SRC,	// This is really a GetRef but it works here.
	SSC_StrArg,
	// SSC_StrChar,	// StrChar( char, string) = Find first instance of this char 
	// SSC_StrCharAt,	// StrcharAt(index, string) = return the char here.
	SSC_StrEat,
	SSC_StrFirstCap,// StrFirstCap(string) = cap the first letter.	
	// SSC_StrLeft,	// strleft(n, string) = get the let n chars of the string
	// SSC_StrMid,		// strmid(n, string) = get the mid n2 chars of the string
	SSC_StrPos,		// StrPos( char, string );
	// SSC_StrRight,	// strright(n, string) = get the right n chars of the string.
	SSC_StrSub,
	SSC_StrToLower,	// strlower(str) = lower case the string
	SSC_StrToUpper,	// strupper(str) = upper case the string
	SSC_SUBSECTION,
	SSC_VALSTR,
	SSC_VAR,
	SSC_VAR0,
	SSC_QTY,
};

LPCTSTR const CScriptObj::sm_szLoadKeys[SSC_QTY+1] =
{
	"ASC",
	"CATEGORY",
	"CHR",
	"DEF",
	"DEF0",
	"DEFMSG",
	"DESCRIPTION",
	"Eval",
	"FILELINES",
	"Fval",
	"Hval",
	"ISEMPTY",
	"ISNUM",
	"Listcol",
	"NEW",
	"OBJ",
	"Qval",
	"READFILE",
	"SRC",
	"StrArg",
	// "StrChar",
	// "StrCharAt",	// charAt(string,index)
	"StrEat",
	"StrFirstCap",
	// "StrLeft",		// strleft(string,n)
	// "StrMid",		// strmid(string,n1,n2)
	"StrPos",
	// "StrRight",		// strright(string,n)
	"StrSub",
	"StrToLower",
	"StrToUpper",
	"SUBSECTION",
	"Valstr",
	"VAR",
	"VAR0",
	NULL,
};


bool	CScriptObj::r_Call( LPCTSTR pszFunction, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * psVal, TRIGRET_TYPE * piRet )
{
	GETNONWHITESPACE( pszFunction );
	int	index;
	{
		int	iCompareRes	= -1;
		index = g_Cfg.m_Functions.FindKeyNear( pszFunction, iCompareRes, true );
		if ( iCompareRes )
			index	= -1;
	}

	if ( index < 0 )
		return false;

	CResourceNamed * pFunction = STATIC_CAST <CResourceNamed *>( g_Cfg.m_Functions[index] );
	ASSERT(pFunction);
	CResourceLock sFunction;
	if ( pFunction->ResourceLock(sFunction) )
	{
		TRIGRET_TYPE iRet =OnTriggerRun( sFunction, TRIGRUN_SECTION_TRUE, pSrc, pArgs,
										g_Cfg.IsSetEF( EF_Scripts_Ret_Strings ) ? psVal : NULL );
		if ( psVal && !g_Cfg.IsSetEF( EF_Scripts_Ret_Strings ) )
				psVal->FormatVal( iRet );
		if ( piRet )
			*piRet	= iRet;
	}
	return( true );
}


bool CScriptObj::r_LoadVal( CScript & s )
{
	// ignore these.
	int index = FindTableHeadSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
	if ( index < 0 )
	{
		DEBUG_ERR(( "Undefined keyword '%s'\n", s.GetKey()));
		return( false );
	}
	
#ifdef GRAY_SVR
	if ( index == SSC_VAR )
	{
		bool fQuoted = false;
		int	i	= g_Exp.m_VarGlobals.SetStr( s.GetKey()+4, fQuoted, s.GetArgStr( &fQuoted ), false );
		return( true );
	}
	if ( index == SSC_VAR0 )
	{
		bool fQuoted = false;
		int	i	= g_Exp.m_VarGlobals.SetStr( s.GetKey()+5, fQuoted, s.GetArgStr( &fQuoted ), true );
		return( true );
	}

	if ( index == SSC_DEFMSG )
	{
		bool fQuoted = false;
		int	i	= g_Exp.m_VarMsgs.SetStr( s.GetKey()+7, fQuoted, s.GetArgStr( &fQuoted ), false );
		return( true );
	}
#endif

	return( true );
}

#define REMOVE_QUOTES( x )			\
{									\
	GETNONWHITESPACE( x );			\
	if ( *x == '"' )	x++;				\
	TCHAR * psX	= strchr( x, '"' );	\
	if ( psX )						\
		*psX	= '\0';				\
}


static void StringFunction( int iFunc, LPCTSTR pszKey, CGString &sVal )
{
	GETNONWHITESPACE(pszKey);
	if ( *pszKey == '(' )
		pszKey++;

	TCHAR * ppCmd[4];
	int iCount = Str_ParseCmds( const_cast<TCHAR *>(pszKey), ppCmd, COUNTOF(ppCmd), ",)" );
	if ( ! iCount )
	{
		DEBUG_ERR(( "Bad string function usage. missing )\n" ));
		return;
	}

	TCHAR * psArg1	= ppCmd[0];

	switch ( iFunc )
	{
	case SSC_CHR:
		sVal.Format( "%c", Exp_GetSingle( ppCmd[0] ) );
		return;
/*
	case SSC_StrChar:	// StrChar( string, char ) = Find first instance of this char 
		return;
	case SSC_StrCharAt:	// StrcharAt(string,index) = return the char here.
		return;
	case SSC_StrFirstCap:// StrFirstCap(string) = cap the first letter.	
		return;
	case SSC_StrLeft:	// strleft(string,n) = get the let n chars of the string
		return;
	case SSC_StrMid:		// strmid(string,n1,n2) = get the mid n2 chars of the string
		return;
	case SSC_StrRight:	// strright(string,n) = get the right n chars of the string.
		return;
*/
	case SSC_StrToLower:	// strlower(str) = lower case the string
		sVal = ppCmd[0];
		sVal.MakeLower();
		return;
	case SSC_StrToUpper:	// strupper(str) = upper case the string
		sVal = ppCmd[0];
		sVal.MakeUpper();
		return;
	}
}

bool CScriptObj::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
{
	CScriptObj * pRef;
	if ( r_GetRef( pszKey, pRef ))
	{
		if ( pRef == NULL )
		{
			// good command but bad link.
			sVal = "0";	// Bad refs always return "0"
			return( true );
		}
		if ( pszKey[0] == '\0' )	// we where just testing the ref.
		{
			CObjBase *	pObj	= dynamic_cast <CObjBase *> (pRef);
			if ( pObj )
				sVal.FormatHex( (DWORD) pObj->GetUID() );
			else
				sVal.FormatVal( 1 );
			return( true );
		}
#if 0 // def GRAY_SVR
		if ( ! g_Cfg.CanUsePrivVerb( this, pszKey, pSrc ))
		{
			return( false );
		}
#endif
		return pRef->r_WriteVal( pszKey, sVal, pSrc );
	}

	int i = FindTableHeadSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
	if ( i < 0 )
	{
		return( false );	// Bad command.
	}

	pszKey += strlen( sm_szLoadKeys[i] );
	SKIP_SEPERATORS(pszKey);
	bool	fZero	= false;

	switch ( i )
	{

#ifdef GRAY_SVR
	case SSC_LISTCOL:
		// Set the alternating color.
		sVal = (CWebPageDef::sm_iListIndex&1) ? "bgcolor=\"#E8E8E8\"" : "";
		return( true );
	case SSC_OBJ:
		sVal.FormatHex( g_World.m_uidObj.ObjFind() ? (DWORD) g_World.m_uidObj : (DWORD) 0 );
		return true;
	case SSC_NEW:
		sVal.FormatHex( g_World.m_uidNew.ObjFind() ? (DWORD) g_World.m_uidNew : (DWORD) 0 );
		return true;
	case SSC_SRC:
		if ( pSrc == NULL )
			pRef	= NULL;
		else
		{
			pRef = pSrc->GetChar();	// if it can be converted .
			if ( ! pRef )
				pRef = dynamic_cast <CScriptObj*> (pSrc);	// if it can be converted .
		}
		if ( ! pRef )
		{
			sVal.FormatVal( 0 );
			return true;
		}
		if ( !*pszKey )
		{
			CObjBase * pObj = dynamic_cast <CObjBase*> (pRef);	// if it can be converted .
			sVal.FormatHex( pObj ? (DWORD) pObj->GetUID() : 0 );
			return true;
		}
		return pRef->r_WriteVal( pszKey, sVal, pSrc );
	case SSC_VAR0:
		fZero	= true;
	case SSC_VAR:
		// "VAR." = get/set a system wide variable.
		{
			CVarDefBase * pVar = g_Exp.m_VarGlobals.GetKey(pszKey);
			if ( pVar )
				sVal	= pVar->GetValStr();
			else if ( fZero )
				sVal	= "0";
		}
		return true;
	case SSC_DEF0:
		fZero	= true;
	case SSC_DEF:
		{
			CVarDefBase * pVar = g_Exp.m_VarDefs.GetKey(pszKey);
			if ( pVar )
				sVal	= pVar->GetValStr();
			else if ( fZero )
				sVal	= "0";
		}
		return( true );
	case SSC_DEFMSG:
		sVal	= g_Cfg.GetDefaultMsg( pszKey );
		return true;
#endif

	case SSC_VALSTR:
	case SSC_EVAL:
		sVal.FormatVal( Exp_GetVal( pszKey ));
		return( true );
	case SSC_FVAL:
		{
		int	iVal		= Exp_GetVal( pszKey );
		sVal.Format( "%i.%i", iVal/10, abs(iVal%10) );
		return true;
		}
	case SSC_HVAL:
		sVal.FormatHex( Exp_GetVal( pszKey ));
		return( true );
	case SSC_QVAL:
		{	// Do a switch ? type statement <QVAL conditional ? option1 : option2>
			TCHAR * ppCmds[3];
			ppCmds[0] = const_cast<TCHAR*>(pszKey);
			Str_Parse( ppCmds[0], &(ppCmds[1]), "?" );
			Str_Parse( ppCmds[1], &(ppCmds[2]), ":" );
			sVal = ppCmds[ Exp_GetVal( ppCmds[0] ) ? 1 : 2 ];
			if ( sVal.IsEmpty())
				sVal = " ";
		}
		return( true );
	case SSC_ISEMPTY:
		sVal.FormatVal( IsStrEmpty( pszKey ) );
		return true;
	case SSC_ISNUM:
		GETNONWHITESPACE( pszKey );
		sVal.FormatVal( IsStrNumeric( pszKey ) );
		return true;
	case SSC_StrPos:
		{
			GETNONWHITESPACE( pszKey );
			int	iPos	= Exp_GetVal( pszKey );
			TCHAR	ch;
			if ( isdigit( *pszKey) && isdigit( *(pszKey+1) ) )
				ch	= (TCHAR) Exp_GetVal( pszKey );
			else
			{
				ch	= *pszKey;
				pszKey++;
			}
			
			GETNONWHITESPACE( pszKey );
			int	iLen	= strlen( pszKey );
			if ( iPos < 0 )
				iPos	= iLen + iPos;
			if ( iPos < 0 )
				iPos	= 0;
			else if ( iPos > iLen )
				iPos	= iLen;

			TCHAR *	pszPos	= strchr( pszKey + iPos, ch );
			if ( !pszPos )
				sVal.FormatVal( -1 );
			else
				sVal.FormatVal( pszPos - pszKey );
		}
		return true;
	case SSC_StrSub:
		{
			int	iPos	= Exp_GetVal( pszKey );
			int	iCnt	= Exp_GetVal( pszKey );
			SKIP_ARGSEP( pszKey );
			GETNONWHITESPACE( pszKey );

			int	iLen	= strlen( pszKey );
			if ( iPos < 0 )
				iPos	= iLen + iPos;
			if ( iPos > iLen || iPos < 0 )
				iPos	= 0;

			if ( iPos + iCnt > iLen || iCnt == 0 )
				iCnt	= iLen - iPos;

			TCHAR	buf	[SCRIPT_MAX_LINE_LEN];

			strncpy( buf, pszKey + iPos, iCnt );
			buf[iCnt] = '\0';
			sVal	= buf;
		}
		return true;
	case SSC_StrArg:
		{
			TCHAR	buf	[SCRIPT_MAX_LINE_LEN];
			GETNONWHITESPACE( pszKey );
			if ( *pszKey == '"' )
				pszKey++;
			int	i	= 0;
			while ( *pszKey && !isspace( *pszKey ) && *pszKey != ',' )
			{
				buf[i]	= *pszKey;
				pszKey++;
				i++;
			}
			buf[i]	= '\0';
			sVal	= buf;
		}
		return true;
	case SSC_StrEat:
		{
			GETNONWHITESPACE( pszKey );
			while ( *pszKey && !isspace( *pszKey ) && *pszKey != ',' )
				pszKey++;
			SKIP_ARGSEP( pszKey );
			sVal	= pszKey;
		}
		return true;
	case SSC_ASC:
		{
			TCHAR	buf	[SCRIPT_MAX_LINE_LEN];
			REMOVE_QUOTES( pszKey );
			sVal.FormatHex( *pszKey );
			sprintf( buf, sVal );
			while ( *(++pszKey) )
			{
				if ( *pszKey == '"' ) break;
				sVal.FormatHex( *pszKey );
				strcat( buf, " " );
				strcat( buf, sVal );
			}
			sVal	= buf;
		}
		return true;

	case SSC_READFILE:
		{
			if ( ! g_Cfg.IsSetOF( OF_FileCommands ) ) 
				return false;

			TCHAR * rfArgs[1];
			FILE * rfFD;

			int line;

			rfArgs[0] = const_cast<TCHAR*>(pszKey);
			Str_Parse( rfArgs[0], &(rfArgs[1]), " " );

			// Remove other junk
			Str_Parse( rfArgs[1], NULL, " " );

			line = atoi( rfArgs[1] );

			if ( rfFD = fopen( rfArgs[0], "r" ))
			{
				if ( line == -1 )
				{
					// First line of the file
					sVal = fgets( "%s", SCRIPT_MAX_LINE_LEN, rfFD );
				}
				else if ( line == 0 )
				{
					// Last line of the file
					while ( ! feof( rfFD ) )
						sVal = fgets( "%s", SCRIPT_MAX_LINE_LEN, rfFD );
				}					
				else
				{
					// Line "line" of the file
					int x;
					for ( x = 1; x <= line; x++ )
						sVal = fgets( "%s", SCRIPT_MAX_LINE_LEN, rfFD );
				}
				fclose(rfFD);
			}
			else
			{
				sVal = "";
			}
		}
		return true;

	case SSC_FILELINES:
		{
			if ( ! g_Cfg.IsSetOF( OF_FileCommands ) )
				return false;
			
			FILE * flFD;
			GETNONWHITESPACE( pszKey );
			if ( flFD = fopen( pszKey, "r" ) )
			{
				int x;
				x = 0;

				while ( ! feof( flFD ) )
				{
					fgets( "%s", SCRIPT_MAX_LINE_LEN, flFD );
					x++;
				}
				sVal.FormatVal( x );
			}
			else
			{
				sVal.FormatVal( 0 );
			}
		}
		return true;

	default:
		StringFunction( i, pszKey, sVal );
		return true;
	}

	return( false );	// Bad command.
}

enum SSV_TYPE
{
	SSV_DUMPKEYS	= 0,
	SSV_DUMPVERBS,
	SSV_NEW,
	SSV_OBJ,
	SSV_SHOW,
	SSV_TRIGGER,
	SSV_QTY,
};

LPCTSTR const CScriptObj::sm_szVerbKeys[SSV_QTY+1] =
{
	_TEXT("DUMPKEYS"),
	_TEXT("DUMPVERBS"),
	_TEXT("NEW"),
	_TEXT("OBJ"),
	_TEXT("SHOW"),
	_TEXT("TRIGGER"),
	NULL,
};


bool CScriptObj::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	int	index;
	LPCTSTR pszKey = s.GetKey();

	ASSERT( pSrc );
	CScriptObj * pRef;

	if ( r_GetRef( pszKey, pRef ))
	{
		if ( pszKey[0] )
		{
			if ( ! pRef )
			{
				return( true );
			}
#if 0 // def GRAY_SVR
			if ( ! g_Cfg.CanUsePrivVerb( this, pszKey, pSrc ))
			{
				return( false );
			}
#endif
			CScript script( pszKey, s.GetArgStr());
			return pRef->r_Verb( script, pSrc );
		}
		// else just fall through. as they seem to be setting the pointer !?
	}

#ifdef GRAY_SVR

	if ( s.IsKeyHead( "SRC.", 4 ))
	{
		pszKey += 4;
		pRef = dynamic_cast <CScriptObj*> (pSrc->GetChar());	// if it can be converted .
		if ( ! pRef )
		{
			pRef = dynamic_cast <CScriptObj*> (pSrc);
			if ( ! pRef )
				return( false );
		}
		CScript script( pszKey, s.GetArgStr());
		return pRef->r_Verb( script, pSrc );
	}

	index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF( sm_szVerbKeys )-1 );
	switch (index)
	{
	case SSV_OBJ:
		g_World.m_uidObj	= s.GetArgVal();
		return( true );
	case SSV_NEW:
		g_World.m_uidNew	= s.GetArgVal();
		return( true );
	case SSV_DUMPKEYS:
		r_DumpLoadKeys( pSrc );
		return( true );
	case SSV_DUMPVERBS:
		r_DumpVerbKeys( pSrc );
		return( true );
	case SSV_SHOW:
		{
		CGString sVal;
		if ( ! r_WriteVal( s.GetArgStr(), sVal, pSrc ))
			return( false );
		CGString sMsg;
		sMsg.Format( "'%s' for '%s' is '%s'\n", (LPCTSTR) s.GetArgStr(), (LPCTSTR) GetName(), (LPCTSTR) sVal );
		pSrc->SysMessage( sMsg );
		}
		return( true );
	case SSV_TRIGGER:
		{
		// This is effectively a goto to an alternate trigger. (for this same object)
		TCHAR * pszVals[2];
		// CScriptTriggerArgs * pArgs = NULL ?
		if ( Str_ParseCmds( s.GetArgStr(), pszVals, COUNTOF(pszVals)))
		{
			TRIGRET_TYPE tRet = OnTrigger( pszVals[0], pSrc, NULL );
			return( true );
		}
		}
		return( false );
	}

#endif

	return r_LoadVal( s );	// default to loading values.
}

bool CScriptObj::r_Load( CScript & s )
{
	while ( s.ReadKeyParse())
	{
		if ( s.IsKeyHead( "ON", 2 ))	// trigger scripting marks the end
			break;
		r_LoadVal(s);
	}
	return( true );
}

int CScriptObj::ParseText( TCHAR * pszResponse, CTextConsole * pSrc, int iFlags, CScriptTriggerArgs * pArgs )
{
	// Take in a line of text that may have fields that can be replaced with operators here.
	// ex. "SPEAK hello there my friend <SRC.NAME> my name is <NAME>"
	// ARGS:
	// iFlags = 2=Allow recusive bracket count. 1=use HTML %% as the delimiters.
	// NOTE:
	//  html will have opening <script language="GRAY_FILE"> and then closing </script>
	// RETURN:
	//  New length of the string.
	//
	// Parsing flags
	LPCTSTR			pszKey; // temporary, set below
	bool			fRes;

	static int sm_iReentrant = 0;
	static bool sm_fBrackets = false;	// allowed to span multi lines.
	if ( ! (iFlags&2))
	{
		// DEBUG_CHECK(!sm_iReentrant);
		sm_fBrackets = false;
	}

	int iBegin = 0;
	TCHAR chBegin = '<';
	TCHAR chEnd = '>';

	bool fHTML = (iFlags&1);
	if ( fHTML )
	{
		chBegin = '%';
		chEnd = '%';
	}

	int i;
	for ( i = 0; pszResponse[i]; i++ )
	{
		TCHAR ch = pszResponse[i];

		if ( ! sm_fBrackets )	// not in brackets
		{
			if ( ch == chBegin )	// found the start !
			{
				 if ( !( isalnum( pszResponse[i+1] ) || pszResponse[i+1] == '<' ) )		// ignore this.
					continue;
				iBegin = i;
				sm_fBrackets = true;
			}
			continue;
		}

		if ( ch == '<' )	// recursive brackets
		{			
			if ( !( isalnum( pszResponse[i+1] ) || pszResponse[i+1] == '<' ) )		// ignore this.
				continue;
			
			if (sm_iReentrant > 32 )
			{
				ASSERT( sm_iReentrant < 32 );
			}
			sm_iReentrant++;
			sm_fBrackets = false;
			int ilen = ParseText( pszResponse+i, pSrc, 2, pArgs );
			sm_fBrackets = true;
			sm_iReentrant--;
			i += ilen;
			// DEBUG_CHECK( ilen < 256 );
			continue;
		}

		if ( ch == chEnd )
		{
			sm_fBrackets = false;
			pszResponse[i] = '\0';

			CGString sVal;
			pszKey		= (LPCTSTR) pszResponse+iBegin+1;
			if ( !strnicmp( pszKey, "CALL", 4 ) && isspace(pszKey[4]) )
			{
				pszKey	+= 4;
				GETNONWHITESPACE( pszKey );
				LPCTSTR		pszArgs	= strchr( pszKey, ' ' );
				if ( pszArgs )
					GETNONWHITESPACE( pszArgs );
				if ( !pszArgs || !*pszArgs )
				{
					fRes	= this->r_Call( pszKey, pSrc, pArgs, &sVal );
				}
				else
				{
					CScriptTriggerArgs	Args( pszArgs );
					if ( pArgs )
						Args.m_VarsLocal	= pArgs->m_VarsLocal;
					fRes	= this->r_Call( pszKey, pSrc, &Args, &sVal );
					if ( pArgs )
						pArgs->m_VarsLocal	= Args.m_VarsLocal;
				}
			}
			else
			{
				if ( !strnicmp( pszKey, "SRC", 3 ) && pszKey[3] == '\0' )
				{
					pszKey	= pszKey;
				}
				if ( !( fRes = r_WriteVal( pszKey, sVal, pSrc ) ) )
				{
					if ( pArgs && pArgs->r_WriteVal( pszKey, sVal, pSrc ) )
						fRes	= true;
				}
			}


			if ( !fRes )
			{
				DEBUG_ERR(( "Can't resolve <%s>\n", pszKey ));
				// Just in case this really is a <= operator ?
				pszResponse[i] = chEnd;
				// continue;	// by Kell
			}

resolved:
			if ( sVal.IsEmpty() && fHTML )
			{
				sVal = "&nbsp";
			}

			int len = sVal.GetLength();
			memmove( pszResponse+iBegin+len, pszResponse+i+1, strlen( pszResponse+i+1 ) + 1 );
			memcpy( pszResponse+iBegin, (LPCTSTR) sVal, len );
			i = iBegin+len-1;

			if ( iFlags&2 )	// just do this one then bail out.
				return( i );
		}
	}

	return( i );
}


#ifdef GRAY_SVR

TRIGRET_TYPE CScriptObj::OnTriggerForLoop( CScript &s, int iType, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult )
{
	// loop from start here to the ENDFOR
	// See WebPageScriptList for dealing with Arrays.

	CScriptLineContext StartContext = s.GetContext();
	CScriptLineContext EndContext = StartContext;

	if ( iType & 8 )		// WHILE
	{
		TCHAR *		pszCond;
		TCHAR		szOrig[ SCRIPT_MAX_LINE_LEN ];
		TCHAR		szTemp[ SCRIPT_MAX_LINE_LEN ];
		int			iWhile	= 0;

		strcpy( szOrig, s.GetArgStr() );
		while(true)
		{
			pArgs->m_VarsLocal.SetNum( "_WHILE", iWhile, false );
			iWhile++;
			strcpy( szTemp, szOrig );
			pszCond	= szTemp;
			ParseText( pszCond, pSrc, 0, pArgs );
			if ( !Exp_GetVal( pszCond ) )
				break;
			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			EndContext = s.GetContext();
			s.SeekContext( StartContext );
		}
	}
	else
		ParseText( s.GetArgStr(), pSrc, 0, pArgs );


	
	if ( iType & 4 )		// FOR
	{
		int			fCountDown		= FALSE;
		int			iMin			= 0;
		int			iMax			= 0;
		int			i;
		TCHAR *		ppArgs[3];
		int			iQty			= Str_ParseCmds( s.GetArgStr(), ppArgs, 3, ", " );
		CGString	sLoopVar	= "_FOR";
		
		switch( iQty )
		{
		case 1:		// FOR x
			iMin	= 1;
			iMax	= Exp_GetSingle( ppArgs[0] );
			break;
		case 2:
			if ( isdigit( *ppArgs[0] ) )
			{
				iMin	= Exp_GetSingle( ppArgs[0] );
				iMax	= Exp_GetSingle( ppArgs[1] );
			}
			else
			{
				iMin		= 1;
				iMax		= Exp_GetSingle( ppArgs[1] );
				sLoopVar	= ppArgs[0];
			}
			break;
		case 3:
			sLoopVar	= ppArgs[0];
			iMin		= Exp_GetSingle( ppArgs[1] );;
			iMax		= Exp_GetSingle( ppArgs[2] );
			break;
		default:
			iMin	= iMax		= 1;
			break;
		}

		if ( iMin > iMax )
			fCountDown	= true;

		if ( fCountDown )
		for ( i = iMin; i >= iMax; --i )
		{
			pArgs->m_VarsLocal.SetNum( sLoopVar, i, false );
			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			EndContext = s.GetContext();
			s.SeekContext( StartContext );
		}
		else
		for ( i = iMin; i <= iMax; ++i )
		{
			pArgs->m_VarsLocal.SetNum( sLoopVar, i, false );
			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			EndContext = s.GetContext();
			s.SeekContext( StartContext );
		}
	}

	if ( (iType & 1) || (iType & 2) )
	{
		int iDist;
		if ( s.HasArgs() )
			iDist = s.GetArgVal();
		else
			iDist = UO_MAP_VIEW_SIZE;

		CObjBaseTemplate * pObj = dynamic_cast <CObjBaseTemplate *>(this);
		if ( pObj == NULL )
		{
			iType = 0;
			DEBUG_ERR(( "FOR Loop trigger on non-world object '%s'\n", GetName()));
		}

		CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
		CPointMap pt = pObjTop->GetTopPoint();
		if ( iType & 1 )		// FORITEM, FOROBJ
		{
			CWorldSearch AreaItems( pt, iDist );
			while(true)
			{
				CItem * pItem = AreaItems.GetItem();
				if ( pItem == NULL )
					break;
				TRIGRET_TYPE iRet = pItem->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
				if ( iRet != TRIGRET_ENDIF )
				{
					return( iRet );
				}
				EndContext = s.GetContext();
				s.SeekContext( StartContext );
			}
		}
		if ( iType & 2 )		// FORCHAR, FOROBJ
		{
			CWorldSearch AreaChars( pt, iDist );
			while(true)
			{
				CChar * pChar = AreaChars.GetChar();
				if ( pChar == NULL )
					break;
				if ( ( iType & 0x10 ) && ( ! pChar->IsClient() ) )	// FORCLIENTS
					continue;
				if ( ( iType & 0x20 ) && ( pChar->m_pPlayer == NULL ) )	// FORPLAYERS
					continue;
				TRIGRET_TYPE iRet = pChar->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
				if ( iRet != TRIGRET_ENDIF )
				{
					return( iRet );
				}
				EndContext = s.GetContext();
				s.SeekContext( StartContext );
			}
		}
	}

	if ( EndContext.m_lOffset <= StartContext.m_lOffset )
	{
		// just skip to the end.
		TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
	}
	else
	{
		s.SeekContext( EndContext );
	}
	return( TRIGRET_ENDIF );
}

#endif

bool CScriptObj::OnTriggerFind( CScript & s, LPCTSTR pszTrigName )
{
	while ( s.ReadKey( false ))
	{
		// Is it a trigger ?
		if ( strnicmp( s.GetKey(), "ON", 2 ))
			continue;

		// Is it the right trigger ?
		s.ParseKeyLate();
		if ( ! strcmpi( s.GetArgRaw(), pszTrigName ))
			return( true );
	}
	return( false );
}

TRIGRET_TYPE CScriptObj::OnTriggerScript( CScript & s, LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
{
	// look for exact trigger matches
	if ( ! OnTriggerFind( s, pszTrigName ))
		return( TRIGRET_RET_DEFAULT );

	PROFILE_TYPE	prvProfileTask	= g_Serv.m_Profile.GetCurrentTask();
	g_Serv.m_Profile.Start( PROFILE_SCRIPTS );

	TRIGRET_TYPE	iRet;
	if ( !pArgs )	// all scripts should have an args block.
	{
		CScriptTriggerArgs	Args( 0, 0, NULL );
		iRet	= OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, &Args );
	}
	else
		iRet	= OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs );

	g_Serv.m_Profile.Start( prvProfileTask );
	return iRet;
}



enum SK_TYPE
{
	SK_BEGIN,
	SK_DORAND,
	SK_DOSWITCH,
	SK_ELIF,
	SK_ELSE,
	SK_ELSEIF,
	SK_END,
	SK_ENDDO,
	SK_ENDFOR,
	SK_ENDIF,
	SK_ENDRAND,
	SK_ENDSWITCH,
	SK_ENDWHILE,
	SK_FOR,
	SK_FORCHARLAYER,
	SK_FORCHARMEMORYTYPE,
	SK_FORCHAR,
	SK_FORCLIENTS,
	SK_FORCONT,
	SK_FORCONTID,		// loop through all items with this ID in the cont
	SK_FORCONTTYPE,
	SK_FORITEM,
	SK_FOROBJ,
	SK_FORPLAYERS,		// not necessary to be online
	SK_IF,
	SK_RETURN,
	SK_WHILE,
	SK_QTY,
};



LPCTSTR const CScriptObj::sm_szScriptKeys[SK_QTY+1] =
{
	_TEXT("BEGIN"),
	_TEXT("DORAND"),
	_TEXT("DOSWITCH"),
	_TEXT("ELIF"),
	_TEXT("ELSE"),
	_TEXT("ELSEIF"),
	_TEXT("END"),
	_TEXT("ENDDO"),
	_TEXT("ENDFOR"),
	_TEXT("ENDIF"),
	_TEXT("ENDRAND"),
	_TEXT("ENDSWITCH"),
	_TEXT("ENDWHILE"),
	_TEXT("FOR"),
	_TEXT("FORCHARLAYER"),
	_TEXT("FORCHARMEMORYTYPE"),
	_TEXT("FORCHARS"),
	_TEXT("FORCLIENTS"),
	_TEXT("FORCONT"),
	_TEXT("FORCONTID"),
	_TEXT("FORCONTTYPE"),
	_TEXT("FORITEMS"),
	_TEXT("FOROBJS"),
	_TEXT("FORPLAYERS"),
	_TEXT("IF"),
	_TEXT("RETURN"),
	_TEXT("WHILE"),
	NULL,
};



TRIGRET_TYPE CScriptObj::OnTriggerRun( CScript &s, TRIGRUN_TYPE trigrun, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult )
{
	// ARGS:
	//	TRIGRUN_SECTION_SINGLE = just this 1 line.
	// RETURN:
	//  TRIGRET_RET_FALSE = 0 = return and continue processing.
	//  TRIGRET_RET_TRUE = 1 = return and handled. (halt further processing)
	//  TRIGRET_RET_DEFAULT = 2 = if process returns nothing specifically.

	// CScriptFileContext set g_Log.m_pObjectContext is the current context (we assume)
	// DEBUGCHECK( this == g_Log.m_pObjectContext );

	bool fSectionFalse = (trigrun == TRIGRUN_SECTION_FALSE || trigrun == TRIGRUN_SINGLE_FALSE);
	if ( trigrun == TRIGRUN_SECTION_EXEC || trigrun == TRIGRUN_SINGLE_EXEC )	// header was already read in.
		goto jump_in;

	while ( s.ReadKeyParse())
	{
		// Hit the end of the next trigger.
		if ( s.IsKeyHead( "ON", 2 ))	// done with this section.
			break;

jump_in:
		SK_TYPE iCmd = (SK_TYPE) FindTableSorted( s.GetKey(), sm_szScriptKeys, COUNTOF( sm_szScriptKeys )-1 );
		TRIGRET_TYPE iRet;

		switch ( iCmd )
		{
		case SK_ENDIF:
		case SK_END:
		case SK_ENDDO:
		case SK_ENDFOR:
		case SK_ENDRAND:
		case SK_ENDSWITCH:
		case SK_ENDWHILE:
			return( TRIGRET_ENDIF );

		case SK_ELIF:
		case SK_ELSEIF:
			return( TRIGRET_ELSEIF );

		case SK_ELSE:
			return( TRIGRET_ELSE );
		}

		if ( fSectionFalse )
		{
			// Ignoring this whole section. don't bother parsing it.
			switch ( iCmd )
			{
			case SK_IF:
				do
				{
					iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
				} while ( iRet == TRIGRET_ELSEIF || iRet == TRIGRET_ELSE );
				break;
			case SK_WHILE:
			case SK_FOR:
			case SK_FORCHARLAYER:
			case SK_FORCHARMEMORYTYPE:
			case SK_FORCHAR:
			case SK_FORCLIENTS:
			case SK_FORCONT:
			case SK_FORCONTID:
			case SK_FORCONTTYPE:
			case SK_FORITEM:
			case SK_FOROBJ:
			case SK_FORPLAYERS:
			case SK_DORAND:
			case SK_DOSWITCH:
			case SK_BEGIN:
				iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
				break;
			}
			if ( trigrun >= TRIGRUN_SINGLE_EXEC )
				return( TRIGRET_RET_DEFAULT );
			continue;	// just ignore it.
		}

		switch ( iCmd )
		{
		case SK_FORITEM:	iRet = OnTriggerForLoop( s, 1, pSrc, pArgs, pResult );			break;
		case SK_FORCHAR:	iRet = OnTriggerForLoop( s, 2, pSrc, pArgs, pResult );			break;
		case SK_FORCLIENTS:	iRet = OnTriggerForLoop( s, 0x12, pSrc, pArgs, pResult );		break;
		case SK_FOROBJ:		iRet = OnTriggerForLoop( s, 3, pSrc, pArgs, pResult );			break;
		case SK_FORPLAYERS:	iRet = OnTriggerForLoop( s, 0x22, pSrc, pArgs, pResult );		break;
		case SK_FOR:		iRet = OnTriggerForLoop( s, 4, pSrc, pArgs, pResult );			break;
		case SK_WHILE:		iRet = OnTriggerForLoop( s, 8, pSrc, pArgs, pResult );			break;
		case SK_FORCHARLAYER:
		case SK_FORCHARMEMORYTYPE:
			{
				CChar * pCharThis = dynamic_cast <CChar *> (this);
				if ( pCharThis )
				{
					if ( s.HasArgs() )
					{
						if ( iCmd == SK_FORCHARLAYER )
							iRet = pCharThis->OnCharTrigForLayerLoop( s, pSrc, pArgs, pResult, (LAYER_TYPE) s.GetArgVal() );
						else
							iRet = pCharThis->OnCharTrigForMemTypeLoop( s, pSrc, pArgs, pResult, s.GetArgVal() );
						break;
					}
				}
			}
		case SK_FORCONT:
			{
				if ( s.HasArgs() )
				{
					TCHAR * ppArgs[2];
					TCHAR * tempPoint;
					TCHAR 	origValue[ SCRIPT_MAX_LINE_LEN ];
					
					int iArgQty = Str_ParseCmds( (TCHAR*) s.GetArgRaw(), ppArgs, COUNTOF(ppArgs), " \t," );
					
					if ( iArgQty >= 1 )
					{
						strcpy(origValue, ppArgs[0]);
						tempPoint = origValue;
						ParseText( tempPoint, pSrc, 0, pArgs );
						
						CGrayUID pCurUid = (DWORD) Exp_GetVal(tempPoint);
						if ( pCurUid.IsValidUID() )
						{
							CObjBase * pObj = pCurUid.ObjFind();
							if ( pObj && pObj->IsContainer() )
							{
								CContainer * pContThis = dynamic_cast <CContainer *> (pObj);
								
								CScriptLineContext StartContext = s.GetContext();
								CScriptLineContext EndContext = StartContext;
								iRet = pContThis->OnGenericContTriggerForLoop( s, pSrc, pArgs, pResult, StartContext, EndContext, ppArgs[1] != NULL ? Exp_GetVal(ppArgs[1]) : 255 );
								break;
							}
						}
					}
				}
			}
		case SK_FORCONTID:
		case SK_FORCONTTYPE:
			{
				CContainer * pCont = dynamic_cast <CContainer *> (this);
				if ( pCont )
				{
					if ( s.HasArgs() )
					{
						LPCTSTR pszKey = s.GetArgRaw();
						SKIP_SEPERATORS(pszKey);
					
						TCHAR * ppArgs[2];
						Str_ParseCmds( (TCHAR*) pszKey, ppArgs, COUNTOF(ppArgs), " \t," );

						CScriptLineContext StartContext = s.GetContext();
						CScriptLineContext EndContext = StartContext;
#ifdef _WIN32
						iRet = pCont->OnContTriggerForLoop( s, pSrc, pArgs, pResult, StartContext, EndContext, g_Cfg.ResourceGetID( ( iCmd == SK_FORCONTID ) ? RES_ITEMDEF : RES_TYPEDEF, ppArgs[0] ), 0, ppArgs[1] != NULL ? Exp_GetVal( ppArgs[1] ) : 255 );
#else
						iRet = pCont->OnContTriggerForLoop( s, pSrc, pArgs, pResult, StartContext, EndContext, g_Cfg.ResourceGetID( ( iCmd == SK_FORCONTID ) ? RES_ITEMDEF : RES_TYPEDEF, (LPCTSTR) ppArgs[0] ), 0, ppArgs[1] != NULL ? Exp_GetVal( ppArgs[1] ) : 255 );
#endif
						break;
					}
				}
			}
		default:
			// Parse out any variables in it. (may act like a verb sometimes?)
			ParseText( s.GetArgRaw(), pSrc, 0, pArgs );
		}

		switch ( iCmd )
		{
#ifdef GRAY_SVR
		case SK_FORITEM:
		case SK_FORCHAR:
		case SK_FORCHARLAYER:
		case SK_FORCHARMEMORYTYPE:
		case SK_FORCLIENTS:
		case SK_FORCONT:
		case SK_FORCONTID:
		case SK_FORCONTTYPE:
		case SK_FOROBJ:
		case SK_FORPLAYERS:
		case SK_FOR:
		case SK_WHILE:
			if ( iRet != TRIGRET_ENDIF )
			{
				if ( iRet > TRIGRET_RET_DEFAULT )
				{
					DEBUG_MSG(( "WARNING: Trigger Bad For Ret %d '%s','%s'\n", iRet, s.GetKey(), s.GetArgStr()));
				}
				return( iRet );
			}
			break;
		case SK_DORAND:	// Do a random line in here.
		case SK_DOSWITCH:
			{
			int iVal = s.GetArgVal();
			if ( iCmd == SK_DORAND )
				iVal = Calc_GetRandVal(iVal);
			for ( ;true; iVal-- )
			{
				iRet = OnTriggerRun( s, (!iVal) ? TRIGRUN_SINGLE_TRUE : TRIGRUN_SINGLE_FALSE, pSrc, pArgs, pResult );
				if ( iRet == TRIGRET_RET_DEFAULT )
					continue;
				if ( iRet == TRIGRET_ENDIF )
					break;
				if ( iRet > TRIGRET_RET_DEFAULT )
				{
					DEBUG_MSG(( "WARNING: Trigger Bad Ret %d '%s','%s'\n", iRet, s.GetKey(), s.GetArgStr()));
				}
				return( iRet );
			}
			}
			break;
#endif
		case SK_RETURN:		// Process the trigger.
			if ( pResult )
			{
				pResult->Copy( s.GetArgStr() );
				return (TRIGRET_TYPE) 1;
			}
			return ( (TRIGRET_TYPE) s.GetArgVal() );
		case SK_IF:
			{
				bool fTrigger = s.GetArgVal() ? true : false;
				bool fBeenTrue = false;
				while (true)
				{
					iRet = OnTriggerRun( s, fTrigger ? TRIGRUN_SECTION_TRUE : TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
					if ( iRet < TRIGRET_ENDIF )
						return( iRet );
					if ( iRet == TRIGRET_ENDIF )
						break;
					fBeenTrue |= fTrigger;
					if ( fBeenTrue )
						fTrigger = false;
					else if ( iRet == TRIGRET_ELSE )
						fTrigger = true;
					else if ( iRet == TRIGRET_ELSEIF )
					{
						ParseText( s.GetArgStr(), pSrc, 0, pArgs );
						fTrigger = s.GetArgVal() ? true : false;
					}
				}
			}
			break;

		case SK_BEGIN:
			// Do this block here.
			{
				iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
				if ( iRet != TRIGRET_ENDIF )
				{
					if ( iRet > TRIGRET_RET_DEFAULT )
					{
						DEBUG_MSG(( "WARNING: Trigger Bad Ret %d '%s','%s'\n", iRet, s.GetKey(), s.GetArgStr()));
					}
					return( iRet );
				}
			}
			break;

		default:
			if ( g_Cfg.IsSetEF( EF_Scripts_Parse_Verbs ) )
			{
				ParseText( s.GetKeyBuffer(), pSrc, 0, pArgs );
			}
			if ( pArgs && pArgs->r_Verb( s, pSrc ) )
				;
			else
			{
				bool	fRes;
				if ( !strcmpi( (char *)s.GetKey(), "call" ) )
				{
					LPCTSTR	pszArgs	= strchr( s.GetArgRaw(), ' ' );
					if ( pszArgs )
						GETNONWHITESPACE( pszArgs );
					if ( !pszArgs || !*pszArgs )
					{
						fRes	= this->r_Call( s.GetArgRaw(), pSrc, pArgs );
					}
					else
					{
						CScriptTriggerArgs	Args( pszArgs );
						if ( pArgs )
							Args.m_VarsLocal	= pArgs->m_VarsLocal;
						fRes	= this->r_Call( s.GetArgRaw(), pSrc, &Args );
						if ( pArgs )
							pArgs->m_VarsLocal	= Args.m_VarsLocal;
					}
				}
				else
					fRes	= r_Verb( s, pSrc );

				if ( !fRes  )
				{
					DEBUG_MSG(( "WARNING: Trigger Bad Verb '%s','%s'\n", s.GetKey(), s.GetArgStr()));
				}
			}
			break;
		}

		if ( trigrun >= TRIGRUN_SINGLE_EXEC )
			return( TRIGRET_RET_DEFAULT );
	}

	return( TRIGRET_RET_DEFAULT );
}

