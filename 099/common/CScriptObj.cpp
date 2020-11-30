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
    
    CScriptTriggerArgs::CScriptTriggerArgs( LPCTSTR pszStr ) :
    	m_s1(pszStr)
    {
    	m_pO1 = NULL;
    
    	// attempt to parse this.
    	if ( isdigit(*pszStr))
    	{
    		m_iN1 = Exp_GetVal(pszStr);
    	}
    }
    
    bool CScriptTriggerArgs::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
    {
    	if ( ! strnicmp( pszKey, "O.", 2 ))	// ARGO.NAME
    	{
    		pszKey += 2;
    do_obj1:
    		pRef = m_pO1;
    		return( true );
    	}
    	if ( ! strnicmp( pszKey, "O1.", 3 ))
    	{
    		pszKey += 3;
    		goto do_obj1;
    	}
    #if 0
    	if ( ! strnicmp( pszKey, "O2.", 3 ))
    	{
    		pszKey += 3;
    		pRef = m_pO2;
    		return( true );
    	}
    #endif
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
    	AGC_QTY,
    };
    
    LPCTSTR const CScriptTriggerArgs::sm_szLoadKeys =
    {
    	"N",	// number
    	"N1",	// number
    	"N2",
    	"N3",
    	"PT",
    	"S",
    	"S1",	// string
    	NULL,
    };
    
    bool CScriptTriggerArgs::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
    {
    	if ( strnicmp( pszKey, "ARG", 3 ))
    		return( false );
    	pszKey += 3;
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
    
    	if ( g_World.r_GetRef( pszKey, pRef ))	// "LASTNEWITEM" 
    		return( true );
    
    	if ( ! strnicmp( pszKey, "SRV.", 4 ))
    	{
    		pszKey += 4;
    		pRef = &g_Serv;
    		return( true );
    	}
    	if ( ! strnicmp( pszKey, "SERV.", 5 ))
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
    	SSC_CATEGORY,
    	SSC_DESCRIPTION,
    	SSC_EVAL,
    	SSC_HVAL,
    	SSC_LISTCOL,
    	SSC_QVAL,
    	SSC_SRC,	// This is really a GetRef but it works here.
    	SSC_StrChar,	// StrChar( string, char ) = Find first instance of this char 
    	SSC_StrCharAt,	// StrcharAt(string,index) = return the char here.
    	SSC_StrFirstCap,// StrFirstCap(string) = cap the first letter.	
    	SSC_StrLeft,	// strleft(string,n) = get the let n chars of the string
    	SSC_StrMid,		// strmid(string,n1,n2) = get the mid n2 chars of the string
    	SSC_StrRight,	// strright(string,n) = get the right n chars of the string.
    	SSC_StrToLower,	// strlower(str) = lower case the string
    	SSC_StrToUpper,	// strupper(str) = upper case the string
    	SSC_SUBSECTION,
    	SSC_VALSTR,
    	SSC_VAR,
    	SSC_QTY,
    };
    
    LPCTSTR const CScriptObj::sm_szLoadKeys =
    {
    	"CATEGORY",
    	"DESCRIPTION",
    	"Eval",
    	"Hval",
    	"Listcol",
    	"Qval",
    	"SRC",
    	"StrChar",
    	"StrCharAt",	// charAt(string,index)
    	"StrFirstCap",
    	"StrLeft",		// strleft(string,n)
    	"StrMid",		// strmid(string,n1,n2)
    	"StrRight",		// strright(string,n)
    	"StrToLower",
    	"StrToUpper",
    	"SUBSECTION",
    	"Valstr",
    	"VAR",
    	NULL,
    };
    
    bool CScriptObj::r_LoadVal( CScript & s )
    {
    	// ignore these.
    	int index = FindTableHeadSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
    	if ( index < 0 )
    	{
    		DEBUG_ERR(( "Undefined keyword '%s'\n", (LPCTSTR) s.GetKey()));
    		return( false );
    	}
    	
    #ifdef GRAY_SVR
    	if ( index == SSC_VAR )
    	{
    		g_Exp.m_VarDefs.SetStr( s.GetKey()+4, s.GetArgStr());
    		return( true );
    	}
    #endif
    
    	return( true );
    }
    
    static void StringFunction( int iFunc, LPCTSTR pszKey, CGString &sVal )
    {
    	GETNONWHITESPACE(pszKey);
    	if ( *pszKey == '(' )
    		pszKey++;
    
    	TCHAR * ppCmd;
    	int iCount = Str_ParseCmds( const_cast<TCHAR *>(pszKey), ppCmd, COUNTOF(ppCmd), ",)" );
    	if ( ! iCount )
    	{
    		DEBUG_ERR(( "Bad string function usage. missing )\n" ));
    		return;
    	}
    
    	switch ( iFunc )
    	{
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
    	case SSC_StrToLower:	// strlower(str) = lower case the string
    		sVal = ppCmd;
    		sVal.MakeLower();
    		return;
    	case SSC_StrToUpper:	// strupper(str) = upper case the string
    		sVal = ppCmd;
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
    		if ( pszKey == '\0' )	// we where just testing the ref.
    		{
    			sVal = "1";
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
    
    	pszKey += strlen( sm_szLoadKeys );
    	SKIP_SEPERATORS(pszKey);
    
    	switch ( i )
    	{
    
    #ifdef GRAY_SVR
    	case SSC_LISTCOL:
    		// Set the alternating color.
    		sVal = (CWebPageDef::sm_iListIndex&1) ? "bgcolor=\"#E8E8E8\"" : "";
    		return( true );
    	case SSC_SRC:
    		if ( pSrc == NULL )
    			return( false );
    		pRef = pSrc->GetChar();	// if it can be converted .
    		if ( ! pRef )
    		{
    			pRef = dynamic_cast <CScriptObj*> (pSrc);	// if it can be converted .
    			if ( ! pRef )
    				return( false );
    		}
    		return pRef->r_WriteVal( pszKey, sVal, pSrc );
    	case SSC_VAR:
    		// "VAR." = get/set a system wide variable.
    		{
    			CVarDefBase * pVar = g_Exp.m_VarDefs.FindKeyPtr(pszKey);
    			if ( pVar )
    			{
    				sVal = pVar->GetValStr();
    			}
    		}
    		return( true );
    #endif
    
    	case SSC_VALSTR:
    	case SSC_EVAL:
    		sVal.FormatVal( Exp_GetVal( pszKey ));
    		return( true );
    
    	case SSC_HVAL:
    		sVal.FormatHex( Exp_GetVal( pszKey ));
    		return( true );
    
    	case SSC_QVAL:
    		{	// Do a switch ? type statement <QVAL conditional ? option1 : option2>
    			TCHAR * ppCmds;
    			ppCmds = const_cast<TCHAR*>(pszKey);
    			Str_Parse( ppCmds, &(ppCmds), "?" );
    			Str_Parse( ppCmds, &(ppCmds), ":" );
    			sVal = ppCmds ) ? 1 : 2 ];
    			if ( sVal.IsEmpty())
    				sVal = " ";
    		}
    		return( true );
    
    	case SSC_StrChar:	// StrChar( string, char ) = Find first instance of this char 
    	case SSC_StrCharAt:	// StrcharAt(string,index) = return the char here.
    	case SSC_StrFirstCap:// StrFirstCap(string) = cap the first letter.	
    	case SSC_StrLeft:	// strleft(string,n) = get the let n chars of the string
    	case SSC_StrMid:		// strmid(string,n1,n2) = get the mid n2 chars of the string
    	case SSC_StrRight:	// strright(string,n) = get the right n chars of the string.
    	case SSC_StrToLower:	// strlower(str) = lower case the string
    	case SSC_StrToUpper:	// strupper(str) = upper case the string
    		StringFunction( i, pszKey, sVal );
    		return( true );
    
    	default:
    		DEBUG_CHECK(0);
    		return( false );
    	}
    
    	return( false );	// Bad command.
    }
    
    enum SSV_TYPE
    {
    	SSV_DUMPKEYS=0,
    	SSV_DUMPVERBS,
    	SSV_SHOW,
    	SSV_TRIGGER,
    	SSV_QTY,
    };
    
    LPCTSTR const CScriptObj::sm_szVerbKeys =
    {
    	_TEXT("DUMPKEYS"),
    	_TEXT("DUMPVERBS"),
    	_TEXT("SHOW"),
    	_TEXT("TRIGGER"),
    	NULL,
    };
    
    bool CScriptObj::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
    {
    	ASSERT( pSrc );
    	LPCTSTR pszKey = s.GetKey();
    	CScriptObj * pRef;
    
    	if ( r_GetRef( pszKey, pRef ))
    	{
    		if ( pszKey )
    		{
    			if ( ! pRef )
    			{
    				return( false );
    			}
    #if 0 // def GRAY_SVR
    			if ( ! g_Cfg.CanUsePrivVerb( this, pszKey, pSrc ))
    			{
    				return( false );
    			}
    #endif
    			return pRef->r_Verb( CScript( pszKey, s.GetArgStr()), pSrc );
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
    		return pRef->r_Verb( CScript( pszKey, s.GetArgStr()), pSrc );
    	}
    
    	int index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF( sm_szVerbKeys )-1 );
    	switch (index)
    	{
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
    		TCHAR * pszVals;
    		// CScriptTriggerArgs * pArgs = NULL ?
    		if ( Str_ParseCmds( s.GetArgStr(), pszVals, COUNTOF(pszVals)))
    		{
    			TRIGRET_TYPE tRet = OnTrigger( pszVals, pSrc, NULL );
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
    	static int sm_iReentrant = 0;
    	static bool sm_fBrackets = false;	// allowed to span multi lines.
    
    	if ( ! (iFlags&2))
    	{
    		DEBUG_CHECK(!sm_iReentrant);
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
    
    	for ( int i = 0; pszResponse; i++ )
    	{
    		TCHAR ch = pszResponse;
    
    		if ( ! sm_fBrackets )	// not in brackets
    		{
    			if ( ch == chBegin )	// found the start !
    			{
    				if ( ! isalpha( pszResponse ))		// ignore this.
    					continue;
    				iBegin = i;
    				sm_fBrackets = true;
    			}
    			continue;
    		}
    
    		if ( ch == '<' )	// recursive brackets
    		{
    			if ( ! isalpha( pszResponse ))		// ignore this.
    				continue;
    			sm_iReentrant++;
    			if (sm_iReentrant > 32 )
    			{
    				ASSERT( sm_iReentrant < 32 );
    			}
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
    			pszResponse = '\0';
    
    			CGString sVal;
    			if ( ! r_WriteVal( pszResponse+iBegin+1, sVal, pSrc ))
    			{
    				if ( pArgs )
    				{
    					if ( pArgs->r_WriteVal( pszResponse+iBegin+1, sVal, pSrc ))
    						goto resolved;
    				}
    				DEBUG_WARN(( "Can't resolve <%s>\n", (LPCTSTR) ( pszResponse+iBegin+1 )));
    				// Just in case this really is a <= operator ?
    				pszResponse = chEnd;
    				continue;
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
    
    TRIGRET_TYPE CScriptObj::OnTriggerForLoop( CScript &s, int iType, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
    {
    	// loop from start here to the ENDFOR
    	// See WebPageScriptList for dealing with Arrays.
    
    	int iDist = s.GetArgVal();
    
    	CObjBaseTemplate * pObj = dynamic_cast <CObjBaseTemplate *>(this);
    	if ( pObj == NULL )
    	{
    		iType = 0;
    		DEBUG_ERR(( "FOR Loop trigger on non-world object '%s'\n", (LPCTSTR) GetName()));
    	}
    
    	CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
    	CPointMap pt = pObjTop->GetTopPoint();
    
    	CScriptLineContext StartContext = s.GetContext();
    	CScriptLineContext EndContext = StartContext;
    
    	if ( iType & 1 )
    	{
    		CWorldSearch AreaItems( pt, iDist );
    		while(true)
    		{
    			CItem * pItem = AreaItems.GetItem();
    			if ( pItem == NULL )
    				break;
    			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc );
    			if ( iRet != TRIGRET_ENDIF )
    			{
    				return( iRet );
    			}
    
    			EndContext = s.GetContext();
    			s.SeekContext( StartContext );
    		}
    	}
    	if ( iType & 2 )
    	{
    		CWorldSearch AreaChars( pt, iDist );
    		while(true)
    		{
    			CChar * pChar = AreaChars.GetChar();
    			if ( pChar == NULL )
    				break;
    			TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc );
    			if ( iRet != TRIGRET_ENDIF )
    			{
    				return( iRet );
    			}
    			EndContext = s.GetContext();
    			s.SeekContext( StartContext );
    		}
    	}
    
    	if ( EndContext.m_lOffset <= StartContext.m_lOffset )
    	{
    		// just skip to the end.
    		TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc );
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
    	// look for exact trigger matches.
    
    	if ( ! OnTriggerFind( s, pszTrigName ))
    		return( TRIGRET_RET_DEFAULT );
    	return OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs );
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
    	SK_FOR,	// "OBJ", "CHAR", "ITEM", "CLIENT", "SERVER"
    	SK_IF,
    	SK_RETURN,
    	SK_QTY,
    };
    
    LPCTSTR const CScriptObj::sm_szScriptKeys =
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
    	_TEXT("FOR"),	// "OBJ", "CHAR", "ITEM", "CLIENT", "SERVER"
    	_TEXT("IF"),
    	_TEXT("RETURN"),
    	NULL,
    };
    
    TRIGRET_TYPE CScriptObj::OnTriggerRun( CScript &s, TRIGRUN_TYPE trigrun, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
    {
    	// ARGS:
    	//	TRIGRUN_SECTION_SINGLE = just this 1 line.
    	// RETURN:
    	//  TRIGRET_RET_FALSE = 0 = return and continue processing.
    	//  TRIGRET_RET_TRUE = 1 = return and handled. (halt further processing)
    	//  TRIGRET_RET_DEFAULT = 2 = if process returns nothing specifically.
    
    	// CScriptFileContext set g_Log.m_pObjectContext is the current context (we assume)
    	// DEBUGCHECK( this == g_Log.m_pObjectContext );
    
    	static int sm_iRecurse = 0;
    	if ( sm_iRecurse > 40 )
    		return TRIGRET_RET_DEFAULT;
    	sm_iRecurse++;
    
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
    			sm_iRecurse--;
    			return( TRIGRET_ENDIF );
    
    		case SK_ELIF:
    		case SK_ELSEIF:
    			sm_iRecurse--;
    			return( TRIGRET_ELSEIF );
    
    		case SK_ELSE:
    			sm_iRecurse--;
    			return( TRIGRET_ELSE );
    		}
    
    		if (fSectionFalse)
    		{
    			// Ignoring this whole section. don't bother parsing it.
    			switch ( iCmd )
    			{
    			case SK_IF:
    				do
    				{
    					iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs );
    				} while ( iRet == TRIGRET_ELSEIF || iRet == TRIGRET_ELSE );
    				break;
    			case SK_FOR:
    			case SK_DORAND:
    			case SK_DOSWITCH:
    			case SK_BEGIN:
    				iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs );
    				break;
    			}
    			if ( trigrun >= TRIGRUN_SINGLE_EXEC )
    			{
    				sm_iRecurse--;
    				return( TRIGRET_RET_DEFAULT );
    			}
    			continue;	// just ignore it.
    		}
    
    		// Parse out any variables in it. (may act like a verb sometimes?)
    		ParseText( s.GetArgStr(), pSrc, 0, pArgs );
    
    		switch ( iCmd )
    		{
    #ifdef GRAY_SVR
    
    		case SK_FOR:
    			{
    			// what type of loop is this ?
    			if ( s.IsKey( "FOROBJS" ))
    			{
    				iRet = OnTriggerForLoop( s, 3, pSrc, pArgs );
    			}
    			else if ( s.IsKey( "FORITEMS" ))	// all items near by
    			{
    				iRet = OnTriggerForLoop( s, 1, pSrc, pArgs );
    			}
    			else if ( s.IsKey( "FORCHARS" ))	// all chars near by
    			{
    				iRet = OnTriggerForLoop( s, 2, pSrc, pArgs );
    			}
    			else
    			{
    				iRet = TRIGRET_ENDIF;	// it's just somethin else.
    			}
    			if ( iRet != TRIGRET_ENDIF )
    			{
    				if ( iRet > TRIGRET_RET_DEFAULT )
    				{
    					DEBUG_MSG(( "WARNING: Trigger Bad For Ret %d '%s','%s'\n", iRet, (LPCTSTR) s.GetKey(), (LPCTSTR) s.GetArgStr()));
    				}
    				sm_iRecurse--;
    				return( iRet );
    			}
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
    				iRet = OnTriggerRun( s, (!iVal) ? TRIGRUN_SINGLE_TRUE : TRIGRUN_SINGLE_FALSE, pSrc, pArgs );
    				if ( iRet == TRIGRET_RET_DEFAULT )
    					continue;
    				if ( iRet == TRIGRET_ENDIF )
    					break;
    				if ( iRet > TRIGRET_RET_DEFAULT )
    				{
    					DEBUG_MSG(( "WARNING: Trigger Bad Ret %d '%s','%s'\n", iRet, (LPCTSTR) s.GetKey(), (LPCTSTR) s.GetArgStr()));
    				}
    				sm_iRecurse--;
    				return( iRet );
    			}
    			}
    			break;
    #endif
    
    		case SK_RETURN:
    			// Process the trigger.
    			sm_iRecurse--;
    			return( (TRIGRET_TYPE) s.GetArgVal());
    
    		case SK_IF:
    			{
    				bool fTrigger = s.GetArgVal() ? true : false;
    				bool fBeenTrue = false;
    				while (true)
    				{
    					iRet = OnTriggerRun( s, fTrigger ? TRIGRUN_SECTION_TRUE : TRIGRUN_SECTION_FALSE, pSrc, pArgs );
    					if ( iRet < TRIGRET_ENDIF )
    					{
    						sm_iRecurse--;
    						return( iRet );
    					}
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
    				iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs );
    				if ( iRet != TRIGRET_ENDIF )
    				{
    					if ( iRet > TRIGRET_RET_DEFAULT )
    					{
    						DEBUG_MSG(( "WARNING: Trigger Bad Ret %d '%s','%s'\n", iRet, (LPCTSTR) s.GetKey(), (LPCTSTR) s.GetArgStr()));
    					}
    					sm_iRecurse--;
    					return( iRet );
    				}
    			}
    			break;
    
    		default:
    			if ( ! r_Verb( s, pSrc ))
    			{
    				DEBUG_MSG(( "WARNING: Trigger Bad Verb '%s','%s'\n", (LPCTSTR) s.GetKey(), (LPCTSTR) s.GetArgStr()));
    
    #if 0 //def _DEBUG
    				bool fRet = r_Verb( s, pSrc );
    				DEBUG_MSG(( "WARNING: Trigger2 Bad Verb '%s','%s'\n", s.GetKey(), s.GetArgStr()));
    #endif
    			}
    			break;
    		}
    
    		if ( trigrun >= TRIGRUN_SINGLE_EXEC )
    			break;
    	}
    
    	sm_iRecurse--;
    	return( TRIGRET_RET_DEFAULT );
    }