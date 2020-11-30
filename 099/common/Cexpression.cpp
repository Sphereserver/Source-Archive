    // Cexpression.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #ifdef GRAY_SVR
    #include "../graysvr/graysvr.h"
    #elif defined(GRAY_MAP)
    #include "../graymap/stdafx.h"
    #include "../graymap/graymap.h"
    #elif defined(GRAY_AGENT)
    #include "../grayagent/stdafx.h"
    #include "../grayagent/grayagent.h"
    #else
    #include "graycom.h"
    #endif
    
    DWORD ahextoi( LPCTSTR pszStr ) // Convert hex string to integer
    {
    	// Unfortunatly the library func cant handle the number FFFFFFFF
    	// TCHAR * sstop; return( strtol( s, &sstop, 16 ));
    
    	if ( pszStr == NULL )
    		return( 0 );
    
    	GETNONWHITESPACE( pszStr );
    
    	DWORD val = 0;
    	while (true)
    	{
    		TCHAR ch = *pszStr;
    		if ( isdigit( ch ))
    			ch -= '0';
    		else
    		{
    			ch = toupper(ch);
    			if ( ch > 'F' || ch <'A' )
    				break;
    			ch -= 'A' - 10;
    		}
    		val *= 0x10;
    		val += ch;
    		pszStr ++;
    	}
    	return( val );
    }
    
    static bool IsSimpleNumberString( LPCTSTR pszTest )
    {
    	// is this a string or a simple numeric expression ?
    	// string = 1 2 3, sdf, sdf sdf sdf, 123d, 123 d,
    	// number = 1.0+-\*~|&!%^()2, 0aed, 123
    
    	bool fMathSep = true;	// last non whitespace was a math sep.
    	bool fHextDigitStart = false;
    	bool fWhiteSpace = false;
    
    	for ( ; true; pszTest++ )
    	{
    		TCHAR ch = *pszTest;
    		if ( ! ch )
    		{
    			return( true );
    		}
    		if (( ch >= 'A' && ch <= 'F') || ( ch >= 'a' && ch <= 'f' ))	// isxdigit
    		{
    			if ( ! fHextDigitStart )
    				return( false );
    			fWhiteSpace = false;
    			fMathSep = false;
    			continue;
    		}
    		if ( isspace( ch ))
    		{
    			fHextDigitStart = false;
    			fWhiteSpace = true;
    			continue;
    		}
    		if ( isdigit( ch ))
    		{
    			if ( fWhiteSpace && ! fMathSep )
    				return false;
    			if ( ch == '0' )
    			{
    				fHextDigitStart = true;
    			}
    			fWhiteSpace = false;
    			fMathSep = false;
    			continue;
    		}
    		fMathSep = strchr( _TEXT("+-\\*~|&!%^()"), ch ) ? true : false ;
    		if ( ! fMathSep )
    		{
    			return( false );
    		}
    		fHextDigitStart = false;
    		fWhiteSpace = false;
    	}
    }
    
    static int GetIdentifierString( TCHAR * szTag, LPCTSTR pszArgs )
    {
    	// Copy the identifier (valid char set) out to this buffer.
    	int i=0;
    	for ( ;pszArgs; i++ )
    	{
    		if ( ! _ISCSYM(pszArgs))
    			break;
    		if ( i>=EXPRESSION_MAX_KEY_LEN )
    			return( NULL );
    		szTag = pszArgs;
    	}
    
    	szTag = '\0';
    	return i;
    }
    
    /////////////////////////////////////////////////////////////////////////
    // -CVarDefArray
    
    int CVarDefArray::CompareKey( CAtomRef Key, CVarDefBase * pVar ) const
    {
    	ASSERT( pVar ); 
    	//return( strcmpi( Key, pVar->GetKey()));
    	return( Key.GetIndex() - pVar->GetKey().GetIndex() );
    }
    
    int CVarDefArray::FindValStr( LPCTSTR pVal ) const
    {
    	int iQty = GetCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		const CVarDefBase * pVarBase = GetAt(i);
    		ASSERT( pVarBase );
    		const CVarDefStr * pVarStr = dynamic_cast <const CVarDefStr *>( pVarBase );
    		if ( pVarStr == NULL )
    			continue;
    		if ( ! strcmpi( pVal, pVarStr->GetValStr()))
    			return( i );
    	}
    	return( -1 );
    }
    
    int CVarDefArray::FindValNum( int iVal ) const
    {
    	// Find a value in the array. (Not a name)
    	// just used for testing purposes.
    
    	int iQty = GetCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		const CVarDefBase * pVarBase = GetAt(i);
    		ASSERT( pVarBase );
    		const CVarDefNum * pVarNum = dynamic_cast <const CVarDefNum *>( pVarBase );
    		if ( pVarNum == NULL )
    			continue;
    		int iValCmp = pVarNum->GetValNum();
    		if ( iValCmp == iVal )
    			return( i );
    	}
    	return( -1 );
    }
    
    int CVarDefArray::Add( CVarDefBase * pVar )
    {
    	int i = AddSortKey( pVar, pVar->GetKey());
    	if ( i < 0 )
    	{
    		// NOTE: pVar has ALREADY BEEN DELETED !!!???
    		DEBUG_ERR(( "Duplicate def %s\n", (LPCTSTR) pVar->GetKey()));	// duplicate should not happen.
    	}
    	return( i );
    }
    
    void CVarDefArray::Copy( const CVarDefArray * pArray )
    {
    	if ( this == pArray )
    		return;
    	Empty();
    	int iQty = pArray->GetCount();
    	SetCount( iQty );
    	for ( int i=0; i<iQty; i++ )
    	{
    		SetAt( i, pArray->GetAt(i)->CopySelf() );
    	}
    }
    
    int CVarDefArray::SetNumNew( CAtomRef Key, int iVal )
    {
    	// This is dangerous , we know it to be a new entry !
    	// Not bothering to check for duplication.
    	// ASSUME we already did FindKey
    
    	CVarDefBase * pVarNum = new CVarDefNum( Key, iVal );
    	ASSERT(pVarNum);
    	return Add( pVarNum );
    }
    
    int CVarDefArray::SetNum( CAtomRef Key, int iVal )
    {
    	if ( ! Key.IsValid())
    		return( -1 );
    
    	// Create if necessary.
    	int i = FindKey( Key );
    	if ( i < 0 )
    	{
    		// key does not exist so create it.
    		return SetNumNew( Key, iVal );
    	}
    
    	// replace existing key.
    
    	CVarDefBase * pVarBase = GetAt(i);
    	ASSERT(pVarBase);
    
    	CVarDefNum * pVarNum = dynamic_cast <CVarDefNum *>( pVarBase );
    	if ( pVarNum )
    	{
    		pVarNum->SetValNum( iVal );
    	}
    	else
    	{
    		// Replace the existing.
    		// This is dangerous !!!
    #ifdef GRAY_SVR
    		if ( g_Serv.IsLoading())
    		{
    			DEBUG_ERR(( "Replace existing VarStr '%s'\n", (LPCTSTR) pVarBase->GetKey()));
    		}
    #endif
    		SetAt( i, new CVarDefNum( Key, iVal ));
    	}
    	return( i );
    }
    
    int CVarDefArray::SetStr( CAtomRef Key, LPCTSTR pszVal )
    {
    	// ASSUME: This has been clipped of unwanted beginning and trailing spaces.
    
    	if ( ! Key.IsValid())
    		return( -1 );
    
    	if ( pszVal == NULL || pszVal == '\0' )	// but not if empty
    	{
    		int i = FindKey( Key );
    		if ( i >= 0 )
    		{
    			DeleteAt(i);
    		}
    		return( -1 );
    	}
    
    	if ( IsSimpleNumberString(pszVal))
    	{
    		// Just store the number and not the string.
    		return SetNum( Key, Exp_GetVal( pszVal ));
    	}
    
    	// Create if necessary.
    	int i = FindKey( Key );
    	if ( i < 0 )
    	{
    		// key does not exist so create it.
    		return Add( new CVarDefStr( Key, pszVal ));
    	}
    
    	// replace existing key.
    	CVarDefBase * pVarBase = GetAt(i);
    	ASSERT(pVarBase);
    
    	CVarDefStr * pVarStr = dynamic_cast <CVarDefStr *>( pVarBase );
    	if ( pVarStr )
    	{
    		pVarStr->SetValStr( pszVal );
    	}
    	else
    	{
    		// Replace the existing. (this will kill crash any existing pointers !)
    #ifdef GRAY_SVR
    		if ( g_Serv.IsLoading())
    		{
    			DEBUG_ERR(( "Replace existing VarNum '%s' with %s\n", (LPCTSTR) pVarBase->GetKey(), (LPCTSTR) pszVal ));
    		}
    #endif
    		SetAt( i, new CVarDefStr( Key, pszVal ));
    	}
    	return( i );
    }
    
    CVarDefBase * CVarDefArray::FindKeyPtr( CAtomRef Key ) const
    {
    	if ( Key.IsValid() )
    	{
    		int j = FindKey( Key );
    		if ( j >= 0 )
    		{
    			return( GetAt(j));
    		}
    	}
    	return( NULL );
    }
    
    LPCTSTR CVarDefArray::FindKeyStr( CAtomRef Key ) const
    {
    	CVarDefBase * pVar = FindKeyPtr(Key);
    	if ( pVar == NULL )
    		return( _TEXT(""));
    	return pVar->GetValStr();
    }
    
    CVarDefBase * CVarDefArray::GetParseKey( LPCTSTR & pszArgs ) const
    {
    	// Skip to the end of the expression name.
    	// The name can only be valid.
    
    	TCHAR szTag;
    	int i = GetIdentifierString( szTag, pszArgs );
    	int j = FindKey( szTag );
    	if ( j >= 0 )
    	{
    		pszArgs += i;
    		return( GetAt(j));
    	}
    
    	g_pLog->Event( LOGL_TRACE, "WARNING: can't find definition for '%s'!\n", (LPCTSTR) pszArgs );
    	return NULL;
    }
    
    bool CVarDefArray::GetParseVal( LPCTSTR & pszArgs, long * plVal ) const
    {
    	// Find the key in the defs collection
    
    	CVarDefBase * pVarBase = GetParseKey( pszArgs );
    	if ( pVarBase == NULL )
    		return( false );
    
    	*plVal = pVarBase->GetValNum();
    	return( true );
    }
    
    void CVarDefArray::r_WriteTags( CScript & s )
    {
    	// Write with the TAG. prefix.
    	for ( int i=GetCount(); --i >= 0; )
    	{
    		const CVarDefBase * pVar = GetAt(i);
    		ASSERT(pVar);
    		CGString sKey;
    		sKey.Format( "TAG.%s", (LPCTSTR) pVar->GetKey() );
    		s.WriteKey( sKey, pVar->GetValStr());
    	}
    }
    
    void CVarDefArray::DumpKeys( CTextConsole * pSrc, LPCTSTR pszPrefix )
    {
    	// List out all the keys.
    	ASSERT(pSrc);
    	if ( pszPrefix == NULL )
    		pszPrefix = _TEXT("");
    	int iQty = GetCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		const CVarDefBase * pVar = GetAt(i);
    		pSrc->SysMessagef( "%s%s=%s", (LPCTSTR) pszPrefix, (LPCTSTR) pVar->GetKey(), (LPCTSTR) pVar->GetValStr());
    	}
    }
    
    ///////////////////////////////////////////////////////////////
    // -CExpression
    
    CExpression::CExpression()
    {
    }
    
    CExpression::~CExpression()
    {
    }
    
    enum INTRINSIC_TYPE
    {
    	INTRINSIC_ID = 0,
    	INTRINSIC_ISBIT,
    	INTRINSIC_RAND,
    	INTRINSIC_RANDBELL,
    	INTRINSIC_STRASCII,
    	INTRINSIC_STRCMP,
    	INTRINSIC_STRCMPI,
    	INTRINSIC_StrIndexOf,	
    	INTRINSIC_STRLEN,
    	INTRINSIC_STRMATCH,
    	INTRINSIC_QTY,
    };
    
    #if 0
    // iso646.h defs.
    #define and && 
    #define and_eq &= 
    #define bitand & 
    #define bitor | 
    #define compl ~ 
    #define not ! 
    #define not_eq != 
    #define or || 
    #define or_eq |= 
    #define xor ^ 
    #define xor_eq ^=  
    #endif
    
    static LPCTSTR const sm_IntrinsicFunctions =
    {
    	"ID",		// ID(x) = truncate the type portion of an Id 
    	"ISBIT",	// ISBIT(number,bit) = is the bit set, 1 or 0
    	"RAND",		// RAND(x) = flat random
    	"RANDBELL",	// RANDBELL(center,variance25)
    	"StrAscii",
    	"STRCMP",	// STRCMP(str1,str2)
    	"STRCMPI",	// STRCMPI(str1,str2)
    	"StrIndexOf", // StrIndexOf(string,searchVal,) = find the index of this, -1 = not here.
    	"STRLEN",	// STRLEN(str)
    	"STRMATCH",	// STRMATCH(str,*?pattern)
    	NULL,
    	// Others? SQRT(), SIN(), COS(), TAN(),
    };
    
    int CExpression::GetSingle( LPCTSTR & pszArgs )
    {
    	// Parse just a single expression without any operators or ranges.
    	ASSERT(pszArgs);
    	GETNONWHITESPACE( pszArgs );
    
    	if ( pszArgs == '0' )	// leading '0' = hex value.
    	{
    		// A hex value.
    		if ( pszArgs == '.' )	// leading 0. means it really is decimal.
    		{
    			pszArgs += 2;
    			goto try_dec;
    		}
    
    		LPCTSTR pStart = pszArgs;
    		DWORD val = 0;
    		while ( true )
    		{
    			TCHAR ch = *pszArgs;
    			if ( isdigit( ch ))
    				ch -= '0';
    			else
    			{
    				ch = tolower(ch);
    				if ( ch > 'f' || ch <'a' )
    				{
    					if ( ch == '.' && pStart != '0' )	// ok i'm confused. it must be decimal.
    					{
    						pszArgs = pStart;
    						goto try_dec;
    					}
    					break;
    				}
    				ch -= 'a' - 10;
    			}
    			val *= 0x10;
    			val += ch;
    			pszArgs ++;
    		}
    		return( val );
    	}
    	else if ( pszArgs == '.' || isdigit(pszArgs))
    	{
    		// A decminal number
    try_dec:
    		long iVal = 0;
    		for ( ;true; pszArgs++ )
    		{
    			if ( *pszArgs == '.' )
    				continue;	// just skip this.
    			if ( ! isdigit( *pszArgs ))
    				break;
    			iVal *= 10;
    			iVal += *pszArgs - '0';
    		}
    		return( iVal );
    	}
    	else if ( ! _ISCSYMF(pszArgs))
    	{
    		// some sort of math op ?
    
    		switch ( pszArgs )
    		{
    		case '{':
    			pszArgs ++;
    			return( GetRange( pszArgs ));
    		case ' == '=' )  // odd condition such as (!=x) which is always true of course.
    			{
    				pszArgs++;		// so just skip it. and compare it to 0
    				return( GetSingle( pszArgs ));
    			}
    			return( !GetSingle( pszArgs ));
    		case ';':	// seperate feild.
    		case ',':	// seperate feild.
    		case '\0':
    			return( 0 );
    		}
    	}
    	else
    	{
    		// Symbol or intrinsinc function ?
    
    		INTRINSIC_TYPE iIntrinsic = (INTRINSIC_TYPE) FindTableHeadSorted( pszArgs, sm_IntrinsicFunctions, COUNTOF(sm_IntrinsicFunctions)-1 );
    		if ( iIntrinsic >= 0 )
    		{
    			int iLen = strlen(sm_IntrinsicFunctions);
    			if ( pszArgs == '(' )
    			{
    			pszArgs += iLen+1;
    			TCHAR * pszArgsNext;
    			Str_Parse( const_cast<TCHAR*>(pszArgs), &(pszArgsNext), ")" );
    
    			TCHAR * ppCmd;
    			int iCount = Str_ParseCmds( const_cast<TCHAR*>(pszArgs), ppCmd, COUNTOF(ppCmd), "," );
    			if ( ! iCount )
    			{
    				DEBUG_ERR(( "Bad intrinsic function usage. missing )\n" ));
    				return 0;
    			}
    
    			pszArgs = pszArgsNext;
    
    			switch ( iIntrinsic )
    			{
    #ifdef GRAY_SVR
    			case INTRINSIC_ID:
    				return( RES_GET_INDEX( GetVal( ppCmd )));	// RES_GET_INDEX
    			case INTRINSIC_StrIndexOf:
    				// 2 or 3 args. ???
    				return( -1 );
    			case INTRINSIC_STRMATCH:
    				if ( iCount < 2 )
    					return( 0 );
    				return(( Str_Match( ppCmd, ppCmd ) == MATCH_VALID ) ? 1 : 0 );
    			case INTRINSIC_RANDBELL:
    				if ( iCount < 2 )
    					return( 0 );
    				return( Calc_GetBellCurve( GetVal( ppCmd ), GetVal( ppCmd )));
    			case INTRINSIC_ISBIT:
    				return( GetVal( ppCmd ) & ( 1 << GetVal( ppCmd )));
    #endif
    			case INTRINSIC_STRASCII:
    				return( ppCmd );
    			case INTRINSIC_RAND:
    				return( Calc_GetRandVal( GetVal( ppCmd )));
    			case INTRINSIC_STRCMP:
    				if ( iCount < 2 )
    					return( 1 );
    				return( strcmp( ppCmd, ppCmd ));
    			case INTRINSIC_STRCMPI:
    				if ( iCount < 2 )
    					return( 1 );
    				return( strcmpi( ppCmd, ppCmd ));
    			case INTRINSIC_STRLEN:
    				return( strlen( ppCmd ));
    			}
    			}
    		}
    
    		// Must be a symbol of some sort ?
    		long lVal;
    		if ( m_VarDefs.GetParseVal( pszArgs, &lVal ))
    			return( lVal );
    	}
    
    	// hard end ! Error of some sort.
    	TCHAR szTag;
    	int i = GetIdentifierString( szTag, pszArgs );
    	pszArgs += i;	// skip it.
    
    	DEBUG_ERR(( "Undefined symbol '%s'\n", (LPCTSTR) szTag ));
    	return( 0 );
    }
    
    int CExpression::GetValMath( int lVal, LPCTSTR & pExpr )
    {
    	GETNONWHITESPACE(pExpr);
    
    	// Look for math type operator.
    	switch ( pExpr )
    	{
    	case '\0':
    		break;
    	case ')':  // expression end markers.
    	case '}':
    	case ']':
    		pExpr++;	// consume this.
    		break;
    	case '+':
    		pExpr++;
    		lVal += GetVal( pExpr );
    		break;
    	case '-':
    		pExpr++;
    		lVal -= GetVal( pExpr );
    		break;
    	case '*':
    		pExpr++;
    		lVal *= GetVal( pExpr );
    		break;
    	case '|':
    		pExpr++;
    		if ( pExpr == '|' )	// boolean ?
    		{
    			pExpr++;
    			lVal = ( GetVal( pExpr ) || lVal );
    		}
    		else	// bitwise
    		{
    			lVal |= GetVal( pExpr );
    		}
    		break;
    	case '&':
    		pExpr++;
    		if ( pExpr == '&' )	// boolean ?
    		{
    			pExpr++;
    			lVal = ( GetVal( pExpr ) && lVal );	// tricky stuff here. logical ops must come first or possibly not get processed.
    		}
    		else	// bitwise
    		{
    			lVal &= GetVal( pExpr );
    		}
    		break;
    	case '/':
    		pExpr++;
    		{
    			long iVal = GetVal( pExpr );
    			if ( ! iVal )
    			{
    				DEBUG_ERR(( "Exp_GetVal Divide by 0\n" ));
    				break;
    			}
    			lVal /= iVal;
    		}
    		break;
    	case '>': // boolean
    		pExpr++;
    		if ( pExpr == '=' )	// boolean ?
    		{
    			pExpr++;
    			lVal = ( lVal >= GetVal( pExpr ));
    		}
    		else if ( pExpr == '>' )	// boolean ?
    		{
    			pExpr++;
    			lVal >>= GetVal( pExpr );
    		}
    		else
    		{
    			lVal = ( lVal > GetVal( pExpr ));
    		}
    		break;
    	case '<': // boolean
    		pExpr++;
    		if ( pExpr == '=' )	// boolean ?
    		{
    			pExpr++;
    			lVal = ( lVal <= GetVal( pExpr ));
    		}
    		else if ( pExpr == '<' )	// boolean ?
    		{
    			pExpr++;
    			lVal <<= GetVal( pExpr );
    		}
    		else
    		{
    			lVal = ( lVal < GetVal( pExpr ));
    		}
    		break;
    	case '!':
    		pExpr ++;
    		if ( pExpr != '=' )
    			break; // boolean ! is handled as a single expresion.
    		pExpr ++;
    		lVal = ( lVal != GetVal( pExpr ));
    		break;
    	case '=': // boolean
    		while ( pExpr == '=' )
    			pExpr ++;
    		lVal = ( lVal == GetVal( pExpr ));
    		break;
    	}
    
    	return( lVal );
    }
    
    int CExpression::GetVal( LPCTSTR & pExpr )
    {
    	// Get a value (default decimal) that could also be an expression.
    	// This does not parse beyond a comma !
    	//
    	// These are all the type of expressions and defines we'll see:
    	//
    	//	all_skin_colors				// simple DEF value
    	//	7933 						// simple decimal
    	//  -100.0						// simple negative decimal
    	//  .5							// simple decimal
    	//  0.5							// simple decimal
    	//	073a 						// hex value (leading zero and no .)
    	//
    	//  0 -1					// Subtraction. has a space seperator. (Yes I know I hate this)
    	//	{0-1}						// hyphenated simple range (GET RID OF THIS!)
    	//				complex ranges must be in {}
    	//	{ 3 6}						// simple range
    	//	{ 400 1 401 1 } 				// weighted values (2nd val = 1)
    	//	{ 1102 1148 1 }				// weighted range (3rd val < 10)
    	//	{ animal_colors 1 no_colors 1 } 	// weighted range
    	//	{ red_colors 1 {34 39} 1 }		// same (red_colors expands to a range)
    
    	if ( pExpr == NULL )
    		return( 0 );
    
    	GETNONWHITESPACE( pExpr );
    
    	int lVal = GetSingle( pExpr );
    
    	return GetValMath( lVal, pExpr );
    }
    
    int CExpression::GetRangeVals( LPCTSTR & pExpr, int * piVals, int iMaxQty )
    {
    	// Get a list of values.
    	if ( pExpr == NULL )
    		return( 0 );
    
    	ASSERT(piVals);
    
    	int iQty = 0;
    	while (true)
    	{
    		if ( pExpr == '\0' )
    			break;
    		if ( pExpr == ';' )
    			break;	// seperate feild.
    		if ( pExpr == ',' )
    			break;	// seperate feild.
    
    		piVals = GetSingle( pExpr );
    		if ( ++iQty >= iMaxQty )
    			break;
    		if ( pExpr == '-' && iQty == 1 )	// range seperator. (if directly after, I know this is sort of strange)
    		{
    			pExpr++;	// ??? This is stupid. get rid of this and clean up it's use in the scripts.
    			continue;
    		}
    
    		GETNONWHITESPACE(pExpr);
    
    		// Look for math type operator.
    		switch ( pExpr )
    		{
    		case ')':  // expression end markers.
    		case '}':
    		case ']':
    			pExpr++;	// consume this and end.
    			return( iQty );
    
    		case '+':
    		case '*':
    		case '/':
    		case '<':
    		case '>':
    		case '|':
    		case '&':
    			piVals = GetValMath( piVals, pExpr );
    			break;
    		}
    	}
    
    	return( iQty );
    }
    
    int CExpression::GetRange( LPCTSTR & pExpr )
    {
    	int lVals;		// Maximum elements in a list
    
    	int iQty = GetRangeVals( pExpr, lVals, COUNTOF(lVals));
    
    	if (iQty == 0)
    	{
    		return( 0 );
    	}
    	if (iQty == 1) // It's just a simple value
    	{
    		return( lVals );
    	}
    	if (iQty == 2) // It's just a simple range....pick one in range at random
    	{
    		return( min( lVals, lVals ) + Calc_GetRandVal( abs( lVals - lVals ) + 1 ));
    	}
    
    	// I guess it's weighted values
    	// First get the total of the weights
    
    	int iTotalWeight = 0;
    	int i = 1;
    	for ( ; i < iQty; i+=2 )
    	{
    		if ( ! lVals )	// having a weight of 0 is very strange !
    		{
    			DEBUG_ERR(( "Weight of 0 in random set?\n" ));	// the whole table should really just be invalid here !
    		}
    		iTotalWeight += lVals;
    	}
    
    	// Now roll the dice to see what value to pick
    	iTotalWeight = Calc_GetRandVal(iTotalWeight) + 1;
    	// Now loop to that value
    	i = 1;
    	for ( ; i<iQty; i+=2 )
    	{
    		iTotalWeight -= lVals;
    		if ( iTotalWeight <= 0 )
    			break;
    	}
    
    	return( lVals );
    }
    
    #if 0
    
    int CExpression::GetArgList( LPCTSTR & pExpr )
    {
    	// Get complex argument list.
    }
    
    #endif