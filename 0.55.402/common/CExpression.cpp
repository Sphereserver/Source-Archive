//
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


bool IsStrEmpty( LPCTSTR pszTest )
{
	if ( !pszTest || !*pszTest )
		return true;

	do
	{
		if ( isspace(*pszTest) )
			continue;
		return false;
	}
	while( *(++pszTest) );

	return true;
}

bool IsStrNumericDec( LPCTSTR pszTest )
{
	if ( !pszTest || !*pszTest )
		return false;

	do
	{
		if ( isdigit( *pszTest ) )
			continue;
		return false;
	}
	while ( *(++pszTest) );

	return true;
}


bool IsStrNumeric( LPCTSTR pszTest )
{
	if ( !pszTest || !*pszTest )
		return false;
	bool	fHex	= false;
	if ( pszTest[0] == '0' )
		fHex	= true;

	do
	{
		if ( isdigit( *pszTest ) )
			continue;
		if ( fHex && tolower(*pszTest) >= 'A' && tolower(*pszTest) <= 'F' )
			continue;
		return false;
	}
	while ( *(++pszTest) );
	return true;
}


bool IsSimpleNumberString( LPCTSTR pszTest )
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
		if ( ch == '/' && pszTest[1] != '/' )
			fMathSep	= true;
		else
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
	for ( ;pszArgs[i]; i++ )
	{
		if ( ! _ISCSYM(pszArgs[i]))
			break;
		if ( i>=EXPRESSION_MAX_KEY_LEN )
			return( NULL );
		szTag[i] = pszArgs[i];
	}

	szTag[i] = '\0';
	return i;
}

/////////////////////////////////////////////////////////////////////////
// -CVarDefArray

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

int CVarDefArray::SetNumNew( LPCTSTR pszName, int iVal )
{
	// This is dangerous , we know it to be a new entry !
	// Not bothering to check for duplication.
	// ASSUME we already did FindKey

	CVarDefBase * pVarNum = new CVarDefNum( pszName, iVal );
	ASSERT(pVarNum);
	return Add( pVarNum );
}

int CVarDefArray::SetNum( LPCTSTR pszName, int iVal, bool fZero )
{
	ASSERT(pszName);
	if ( pszName[0] == '\0' )
		return( -1 );


	if ( fZero && (iVal == 0) )	// but not if empty
	{
		int i = FindKey( pszName );
		if ( i >= 0 )
		{
			DeleteAt(i);
		}
		return( -1 );
	}

	// Create if necessary.
	int i = FindKey( pszName );
	if ( i < 0 )
	{
		// key does not exist so create it.
		return SetNumNew( pszName, iVal );
	}

	// replace existing key.

	CVarDefBase * pVarBase = GetAt(i);
	ASSERT(pVarBase);

	CVarDefNum * pVarNum	= dynamic_cast <CVarDefNum *>( pVarBase );
	if ( pVarNum )
	{
		pVarNum->SetValNum( iVal );
	}
	else
	{
		// Replace the existing.
		// This is dangerous !!!
#ifdef GRAY_SVR
		if ( g_Serv.IsLoading() )
		{
			DEBUG_ERR(( "Replace existing VarStr '%s'\n", pVarBase->GetKey()));
		}
#endif
		SetAt( i, new CVarDefNum( pszName, iVal ));
	}
	return( i );
}

int CVarDefArray::SetStr( LPCTSTR pszName, bool fQuoted, LPCTSTR pszVal, bool fZero )
{
	// ASSUME: This has been clipped of unwanted beginning and trailing spaces.
	ASSERT(pszName);
	if ( pszName[0] == '\0' )
		return( -1 );

	if ( this != &g_Exp.m_VarMsgs )
	{
		if ( pszVal == NULL || pszVal[0] == '\0' )	// but not if empty
		{
			int i = FindKey( pszName );
			if ( i >= 0 )
			{
				DeleteAt(i);
			}
			return( -1 );
		}

		if ( !fQuoted && IsSimpleNumberString(pszVal))
		{
			// Just store the number and not the string.
			return SetNum( pszName, Exp_GetVal( pszVal ));
		}
	}


	// Create if necessary.
	int i = FindKey( pszName );
	if ( i < 0 )
	{
		// key does not exist so create it.
		return Add( new CVarDefStr( pszName, pszVal ) );
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
			DEBUG_ERR(( "Replace existing VarNum '%s' with %s\n", pVarBase->GetKey(), pszVal ));
		}
#endif
		SetAt( i, new CVarDefStr( pszName, pszVal ));
	}
	return( i );
}

CVarDefBase * CVarDefArray::GetKey( LPCTSTR pszKey ) const
{
	if ( pszKey )
	{
		int j = FindKey( pszKey );
		if ( j >= 0 )
		{
			return( GetAt(j));
		}
	}
	return( NULL );
}

LPCTSTR CVarDefArray::GetKeyStr( LPCTSTR pszKey, bool fZero  ) const
{
	CVarDefBase * pVar = GetKey(pszKey);
	if ( pVar == NULL )
		return (_TEXT( (fZero ? "0" : "") ));
	return pVar->GetValStr();
}

CVarDefBase * CVarDefArray::GetParseKey( LPCTSTR & pszArgs ) const
{
	// Skip to the end of the expression name.
	// The name can only be valid.

	TCHAR szTag[ EXPRESSION_MAX_KEY_LEN ];
	int i = GetIdentifierString( szTag, pszArgs );
	int j = FindKey( szTag );
	if ( j >= 0 )
	{
		pszArgs += i;
		return( GetAt(j));
	}

	if ( this == (&g_Exp.m_VarGlobals) )
		return NULL;

	g_pLog->Event( LOGL_TRACE, "WARNING: can't find definition for '%s'!\n", pszArgs );
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
	LPCTSTR		pszVal;
	// Write with the TAG. prefix.
	for ( int i=GetCount(); --i >= 0; )
	{
		CVarDefBase * pVar = GetAt(i);
		ASSERT(pVar);
		CGString sKey;
		sKey.Format( "TAG.%s", pVar->GetKey() );
		pszVal		= pVar->GetValStr();
		CVarDefStr * pVarStr = dynamic_cast <CVarDefStr *>( pVar );
		if ( pVarStr /*isspace(pszVal[0]) || isspace( pszVal[strlen(pszVal)-1] )*/ )
			s.WriteKeyFormat( sKey, "\"%s\"", pszVal  );
		else
			s.WriteKey( sKey, pszVal );
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

void CVarDefArray::ClearKeys()
{
	// Clear the tag array
	for ( int i=GetCount(); --i >= 0;)
	{
		DeleteAt(i);
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

static LPCTSTR const sm_IntrinsicFunctions[INTRINSIC_QTY+1] =
{
	"ID",		// ID(x) = truncate the type portion of an Id
	"ISBIT",	// ISBIT(number,bit) = is the bit set, 1 or 0
	"RAND",		// RAND(x) = flat random
	"RANDBELL",	// RANDBELL(center,variance25)
	"StrAscii",
	"STRCMP",	// STRCMP(str1,str2)
	"STRCMPI",	// STRCMPI(str1,str2)
	"StrIndexOf", // StrIndexOf(string,searchVal,[index]) = find the index of this, -1 = not here.
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

	if ( pszArgs[0] == '0' )	// leading '0' = hex value.
	{
		// A hex value.
		if ( pszArgs[1] == '.' )	// leading 0. means it really is decimal.
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
					if ( ch == '.' && pStart[0] != '0' )	// ok i'm confused. it must be decimal.
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
	else if ( pszArgs[0] == '.' || isdigit(pszArgs[0]))
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
	else if ( ! _ISCSYMF(pszArgs[0]))
	{
		// some sort of math op ?

		switch ( pszArgs[0] )
		{
		case '{':
			pszArgs ++;
			return( GetRange( pszArgs ));
		case '[':
		case '(': // Parse out a sub expression.
			pszArgs ++;
			return( GetVal( pszArgs ));
		case '+':
			pszArgs++;
			break;
		case '-':
			pszArgs++;
			return( -GetSingle( pszArgs ));
		case '~':	// Bitwise not.
			pszArgs++;
			return( ~GetSingle( pszArgs ));
		case '!':	// boolean not.
			pszArgs++;
			if ( pszArgs[0] == '=' )  // odd condition such as (!=x) which is always true of course.
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
			int iLen = strlen(sm_IntrinsicFunctions[iIntrinsic]);
			if ( pszArgs[iLen] == '(' )
			{
			pszArgs += iLen+1;
			TCHAR * pszArgsNext;
			Str_Parse( const_cast<TCHAR*>(pszArgs), &(pszArgsNext), ")" );

			TCHAR * ppCmd[4];
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
				return( RES_GET_INDEX( GetVal( ppCmd[0] )));	// RES_GET_INDEX
			case INTRINSIC_StrIndexOf:
				// 2 or 3 args. ???
				return( -1 );
			case INTRINSIC_STRMATCH:
				if ( iCount < 2 )
					return( 0 );
				return(( Str_Match( ppCmd[0], ppCmd[1] ) == MATCH_VALID ) ? 1 : 0 );
			case INTRINSIC_RANDBELL:
				if ( iCount < 2 )
					return( 0 );
				return( Calc_GetBellCurve( GetVal( ppCmd[0] ), GetVal( ppCmd[1] )));
			case INTRINSIC_ISBIT:
				return( GetVal( ppCmd[0] ) & ( 1 << GetVal( ppCmd[1] )));
#endif
			case INTRINSIC_STRASCII:
				return( ppCmd[0][0] );
			case INTRINSIC_RAND:

				if ( iCount == 2 )
				{
					int		val1	= GetVal( ppCmd[0] );
					int		val2	= GetVal( ppCmd[1] );
					return val1 + Calc_GetRandVal( val2 - val1 );
				}
				else
					return( Calc_GetRandVal( GetVal( ppCmd[0] )));
			case INTRINSIC_STRCMP:
				if ( iCount < 2 )
					return( 1 );
				return( strcmp( ppCmd[0], ppCmd[1] ));
			case INTRINSIC_STRCMPI:
				if ( iCount < 2 )
					return( 1 );
				return( strcmpi( ppCmd[0], ppCmd[1] ));
			case INTRINSIC_STRLEN:
				return( strlen( ppCmd[0] ));
			}
			}
		}

		// Must be a symbol of some sort ?
		long lVal;
		if ( m_VarGlobals.GetParseVal( pszArgs, &lVal ) )
			return( lVal );
		if ( m_VarDefs.GetParseVal( pszArgs, &lVal ) )
			return( lVal );
	}

	// hard end ! Error of some sort.
	TCHAR szTag[ EXPRESSION_MAX_KEY_LEN ];
	int i = GetIdentifierString( szTag, pszArgs );
	pszArgs += i;	// skip it.

	DEBUG_ERR(( "Undefined symbol '%s'\n", szTag ));
	return( 0 );
}

int CExpression::GetValMath( int lVal, LPCTSTR & pExpr )
{
	GETNONWHITESPACE(pExpr);

	// Look for math type operator.
	switch ( pExpr[0] )
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
		if ( pExpr[0] == '|' )	// boolean ?
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
		if ( pExpr[0] == '&' )	// boolean ?
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
	case '%':
		pExpr++;
		{
			long iVal = GetVal( pExpr );
			if ( ! iVal )
			{
				DEBUG_ERR(( "Exp_GetVal Divide by 0\n" ));
				break;
			}
			lVal %= iVal;
		}
		break;
	case '>': // boolean
		pExpr++;
		if ( pExpr[0] == '=' )	// boolean ?
		{
			pExpr++;
			lVal = ( lVal >= GetVal( pExpr ));
		}
		else if ( pExpr[0] == '>' )	// boolean ?
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
		if ( pExpr[0] == '=' )	// boolean ?
		{
			pExpr++;
			lVal = ( lVal <= GetVal( pExpr ));
		}
		else if ( pExpr[0] == '<' )	// boolean ?
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
		if ( pExpr[0] != '=' )
			break; // boolean ! is handled as a single expresion.
		pExpr ++;
		lVal = ( lVal != GetVal( pExpr ));
		break;
	case '=': // boolean
		while ( pExpr[0] == '=' )
			pExpr ++;
		lVal = ( lVal == GetVal( pExpr ));
		break;
	case '^':
		pExpr ++;
		lVal = ( lVal ^ GetVal( pExpr ));
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
		if ( pExpr[0] == '\0' )
			break;
		if ( pExpr[0] == ';' )
			break;	// seperate feild.
		if ( pExpr[0] == ',' )
			break;	// seperate feild.

		piVals[iQty] = GetSingle( pExpr );
		if ( ++iQty >= iMaxQty )
			break;
		if ( pExpr[0] == '-' && iQty == 1 )	// range seperator. (if directly after, I know this is sort of strange)
		{
			pExpr++;	// ??? This is stupid. get rid of this and clean up it's use in the scripts.
			continue;
		}

		GETNONWHITESPACE(pExpr);

		// Look for math type operator.
		switch ( pExpr[0] )
		{
		case ')':  // expression end markers.
		case '}':
		case ']':
			pExpr++;	// consume this and end.
			return( iQty );

		case '+':
		case '*':
		case '/':
		case '%':
		case '<':
		case '>':
		case '|':
		case '&':
			piVals[iQty-1] = GetValMath( piVals[iQty-1], pExpr );
			break;
		}
	}

	return( iQty );
}

int CExpression::GetRange( LPCTSTR & pExpr )
{
	int lVals[64];		// Maximum elements in a list

	int iQty = GetRangeVals( pExpr, lVals, COUNTOF(lVals));

	if (iQty == 0)
	{
		return( 0 );
	}
	if (iQty == 1) // It's just a simple value
	{
		return( lVals[0] );
	}
	if (iQty == 2) // It's just a simple range....pick one in range at random
	{
		return( min( lVals[0], lVals[1] ) + Calc_GetRandVal( abs( lVals[1] - lVals[0] ) + 1 ));
	}

	// I guess it's weighted values
	// First get the total of the weights

	int iTotalWeight = 0;
	int i = 1;
	for ( ; i < iQty; i+=2 )
	{
		if ( ! lVals[i] )	// having a weight of 0 is very strange !
		{
			DEBUG_ERR(( "Weight of 0 in random set?\n" ));	// the whole table should really just be invalid here !
		}
		iTotalWeight += lVals[i];
	}

	// Now roll the dice to see what value to pick
	iTotalWeight = Calc_GetRandVal(iTotalWeight) + 1;
	// Now loop to that value
	i = 1;
	for ( ; i<iQty; i+=2 )
	{
		iTotalWeight -= lVals[i];
		if ( iTotalWeight <= 0 )
			break;
	}

	return( lVals[i-1] );
}

#if 0

int CExpression::GetArgList( LPCTSTR & pExpr )
{
	// Get complex argument list.
}

#endif
