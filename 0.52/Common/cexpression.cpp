//
// Cexpression.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graycom.h"

int GetLog2( UINT iVal )
{
	// This is really log2 + 1
	int i=0;
	for ( ; iVal; i++ ) 
	{
		ASSERT( i < 32 );
		iVal >>= 1 ;
	}
	return( i );
}

int GetRandVal( int iqty )
{
	if ( iqty <= 0 ) 
		return( 0 );
	if ( iqty > RAND_MAX )
	{
		return( IMULDIV( rand(), (DWORD) iqty, RAND_MAX + 1 )) ;
	}
	return( rand() % iqty );
}

int GW_GetBellCurve( int iValDiff, int iVariance )
{
	// Produce a log curve.
	//
	// 50+
	//	 |
	//	 |
	//	 |
	// 25|  +
	//	 | 
	//	 |	   +		   
	//	 |		  +	
	//	0 --+--+--+--+------
	//    iVar				iValDiff
	//
	// ARGS:
	//  iValDiff = Given a value relative to 0
	//		0 = 50.0% chance.
	//  iVariance = the 25.0% point of the bell curve
	// RETURN:
	//  (0-100.0) % chance at this iValDiff.
	//  Chance gets smaller as Diff gets bigger.
	// EXAMPLE:
	//  if ( iValDiff == iVariance ) return( 250 )
	//  if ( iValDiff == 0 ) return( 500 );
	//

	if ( iVariance <= 0 )	// this really should not happen but just in case.
		return( 500 );
	if ( iValDiff < 0 ) iValDiff = -iValDiff;

#ifdef _DEBUG
	int iCount = 32;
#endif

	int iChance = 500;
	while ( iValDiff > iVariance && iChance )
	{
		iValDiff -= iVariance;
		iChance /= 2;	// chance is halved for each Variance period.
#ifdef _DEBUG
		iCount--;
		ASSERT( iCount );
#endif
	}

	return( iChance - IMULDIV( iChance/2, iValDiff, iVariance ));
}

int GW_GetSCurve( int iValDiff, int iVariance )
{
	// ARGS:
	//   iValDiff = Difference between our skill level and difficulty.
	//		positive = high chance, negative = lower chance
	//		0 = 50.0% chance.
	//   iVariance = the 25.0% difference point of the bell curve
	// RETURN: 
	//	 what is the (0-100.0)% chance of success = 0-1000
	// NOTE:
	//   Chance of skill gain is inverse to chance of success.
	//
	int iChance = GW_GetBellCurve( iValDiff, iVariance );
	if ( iValDiff > 0 ) 
		return( 1000 - iChance );
	return( iChance );
}

DWORD ahextoi( const TCHAR * pszStr ) // Convert hex string to integer
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

/////////////////////////////////////////////////////////////////////////
// -CVarDefArray

int CVarDefArray::FindValStr( const TCHAR * pVal ) const
{
	for ( int i=0; i<GetCount(); i++ )
	{
		const CVarDef * pVar = GetAt(i);
		ASSERT( pVar );
		if ( ! strcmpi( pVal, pVar->GetVal()))
			return( i );
	}
	return( -1 );
}

int CVarDefArray::FindValNum( int iVal ) const
{
	for ( int i=0; i<GetCount(); i++ )
	{
		const CVarDef * pVar = GetAt(i);
		ASSERT( pVar );
		const TCHAR * pszVal = pVar->GetVal();
		ASSERT( pszVal );
		if ( ! isdigit( pszVal[0] )) 
			continue;
		int iValCmp = Exp_GetVal( pszVal );
		if ( iValCmp == iVal ) 
			return( i );
	}
	return( -1 );
}

/////////////////////////////////////////////////////////////////////////
// -CExpression

bool CExpression::SetVarDef( const TCHAR * pszName, const TCHAR * pszVal )
{
	int i = m_VarDefs.FindKey( pszName );
	if ( i >= 0 )
	{
		// replace existing key.
		m_VarDefs[i]->SetVal( pszVal );
		return( true );
	}
	else
	{
		// key does not exist so create it.
		m_VarDefs.Add( new CVarDef( pszName, pszVal ));
		return( false );
	}
}

CVarDef * CExpression::GetVarDef( const TCHAR * & pArgs )
{
	// Skip to the end of the expression name.
	// The name can only be valid.

	TCHAR szTag[ 128 ];
	int i=0;
	for ( ;pArgs[i]; i++ )
	{
		if ( ! isalnum(pArgs[i]) && pArgs[i] != '_' )
			break;
		if ( i>=sizeof(szTag)) 
			return( NULL );
		szTag[i] = pArgs[i];
	}

	szTag[i] = '\0';

	int j = m_VarDefs.FindKey( szTag );
	if ( j >= 0 )
	{
		pArgs += i;
		return( m_VarDefs[j] );
	}

	g_pLog->Event( LOGL_TRACE, "WARNING: can't find definition for '%s'!\n", pArgs );
	return NULL;
}

bool CExpression::GetVarDef( const TCHAR * & pArgs, long * plVal )
{
	// Find the key in the defs collection

	CVarDef * pVarDef = GetVarDef( pArgs );
	if ( pVarDef == NULL )
		return( false );

	const TCHAR * pszVal = pVarDef->GetVal();
	*plVal = GetVal( pszVal );	// assume all defs are decimal defaults
	return true;
}

int CExpression::GetSingle( const TCHAR * & pArgs, bool fHexDef )
{
	// Parse just a single expression without any operators or ranges.

	GETNONWHITESPACE( pArgs );
	switch ( pArgs[0] )
	{
	case 'r':
	case 'R':
		if ( ! strnicmp( pArgs, "RAND(", 5 ))
		{
			pArgs += 5;
			return( GetRandVal( GetVal( pArgs, false )));
		}
		break;
	case 's':
	case 'S':

#if 0
		if ( ! strnicmp( pArgs, "STRMATCH(", 9 ))
		{
			pArgs += 9;
			int len1 = 0;
			const TCHAR * pStr1 = pArgs;
			for ( ; pArgs[0] && pArgs[0] != ',' && pArgs[0] != ')'; len1++ ) 
			{
				pArgs ++;
			}
			if ( pArgs[0] == ',' ) { pArgs ++; }
			int len2 = 0;
			const TCHAR * pStr2 = pArgs;
			for ( ; pArgs[0] && pArgs[0] != ')'; len2++ ) 
			{ 
				pArgs ++; 
			}
			return( Text_Match( pStr1, pStr2 ));
		}
#endif

		if ( ! strnicmp( pArgs, "STRCMP(", 7 ))
		{
			pArgs += 7;
			int len1 = 0;
			const TCHAR * pStr1 = pArgs;
			for ( ; pArgs[0] && pArgs[0] != ',' && pArgs[0] != ')'; len1++ ) 
			{
				pArgs ++;
			}
			if ( pArgs[0] == ',' ) { pArgs ++; }
			int len2 = 0;
			const TCHAR * pStr2 = pArgs;
			for ( ; pArgs[0] && pArgs[0] != ')'; len2++ ) 
			{ 
				pArgs ++; 
			}
			return( strncmp( pStr1, pStr2, min( len1, len2 )));
		}
		if ( ! strnicmp( pArgs, "STRLEN(", 7 ))
		{
			pArgs += 7;
			int len = 0;
			for ( ; pArgs[0] && pArgs[0] != ')'; len++ ) 
			{
				pArgs ++;
			}
			return( len );
		}
		break;
	case '{':
	case '[':
	case '(': // Parse out a sub expression.
		pArgs ++;
		return( GetVal( pArgs, false ));
	case '~':	// Bitwise not.
		pArgs++;
		return( ~GetSingle( pArgs, false ));
	case '!':	// boolean not.
		pArgs++;
		return( !GetSingle( pArgs, false )); 
	case '\0':
		return( 0 );
	}

	bool fNeg = ( *pArgs == '-' );
	if ( fNeg )
	{
		pArgs++;
		GETNONWHITESPACE( pArgs );
	}
	else if ( pArgs[0] == '0' )	// leading '0' = hex value.
	{
		if ( pArgs[1] == '.' )	// leading 0. means it really is decimal.
		{
			pArgs += 2;
			goto try_dec;
		}
try_hex:
		const TCHAR * pStart = pArgs;
		DWORD val = 0;
		while ( true )
		{
			TCHAR ch = *pArgs;
			if ( isdigit( ch ))
				ch -= '0';
			else
			{
				ch = tolower(ch);
				if ( ch > 'f' || ch <'a' )
				{
					if ( ch == '.' && pStart[0] != '0' )	// ok i'm confused. it must be decimal.
					{
						pArgs = pStart;
						goto try_dec;
					}
					break;
				}
				ch -= 'a' - 10;
			}
			val *= 0x10;
			val += ch;
			pArgs ++;
		}
		return( val );
	}
	else if ( pArgs[0] == '.' ) 
	{
		fHexDef = false;
	}
	else if ( ! isdigit( pArgs[0] ))
	{
		// is it an expression ?
		long lVal;
		if ( GetVarDef( pArgs, &lVal ))
			return( lVal );
		goto try_hex;	// must be hex i guess ?
	}
	else if ( fHexDef )
	{
		goto try_hex;
	}

try_dec:
	long iVal = 0;
	for ( ;true; pArgs++ )
	{
		if ( *pArgs == '.' )
			continue;	// just skip this.
		if ( ! isdigit( *pArgs ))
			break;
		iVal *= 10;
		iVal += *pArgs - '0';
	}
	if ( fNeg ) 
		iVal = -iVal;
	return( iVal );
}

int CExpression::GetVal( const TCHAR * & pExpr, bool fHexDef )
{
	// Get a value (default decimal) that could also be an range expression.
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
	//	0 1						// simple range
	//	0-1						// hyphenated simple range (GET RID OF THIS!)
	//  0 -1					// Subtraction. has a space seperator. (Yes I know I hate this)
	//				complex ranges must be in {}
	//	{ 400 1 401 1 } 				// weighted values (2nd val = 1)
	//	{ 1102 1148 1 }				// weighted range (3rd val < 10)
	//	{ animal_colors 1 no_colors 1 } 	// weighted range
	//	{ red_colors 1 34 39 1 }		// same (red_colors expands to a range)

	static int iRecurse = 0;	// Make sure we don't recurse too much.
	if ( iRecurse > 32 ) 
	{
		DEBUG_ERR(( "Exp_GetVal Too much recursion!\n" ));
		return( 0 );
	}
	if ( pExpr == NULL ) 
		return( 0 );

	iRecurse ++;

	GETNONWHITESPACE( pExpr );

	long lVals[64];		// Maximum elements in a list
	int iQty = 0;
	while (true)
	{
getmorevals:
		if ( pExpr[0] == '\0' ) 
			break;	
		if ( pExpr[0] == ';' )
			break;	// seperate feild.
		if ( pExpr[0] == ',' )
			break;	// seperate feild.

		lVals[iQty] = GetSingle( pExpr, fHexDef );
		if ( ++iQty >= COUNTOF(lVals)-1 ) 
			break;
		if ( pExpr[0] == '-' && iQty == 1 )	// range seperator. (if directly after, I know this is sort of strange)
		{
			pExpr++;	// ??? This is stupid. get rid of this and clean up it's use in the scripts.
			continue;
		}

		bool fWhitespace = false;
		while (true)
		{
			// Look for math type operator.
			switch ( pExpr[0] )
			{
			case ')':  // expression end markers.
			case '}':
			case ']':
				pExpr++;	// consume this.
				goto nomorevals;
			case '\t':
			case ' ':
				fWhitespace = true;
				pExpr++;
				continue;
			case '+': 
				pExpr++;
				lVals[iQty-1] += GetSingle( pExpr, fHexDef ); 
				break;
			case '-': 
				pExpr++;
				lVals[iQty-1] -= GetSingle( pExpr, fHexDef ); 
				break;
			case '*': 
				pExpr++;
				lVals[iQty-1] *= GetSingle( pExpr, fHexDef ); 
				break;
			case '|': 
				pExpr++;
				if ( pExpr[0] == '|' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] = ( GetSingle( pExpr, fHexDef ) || lVals[iQty-1] );
				}
				else	// bitwise
				{
					lVals[iQty-1] |= GetSingle( pExpr, fHexDef );
				}
				break;
			case '&': 
				pExpr++;
				if ( pExpr[0] == '&' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] = ( GetSingle( pExpr, fHexDef ) && lVals[iQty-1] );	// tricky stuff here. logical ops must come first or possibly not get processed.
				}
				else	// bitwise
				{
					lVals[iQty-1] &= GetSingle( pExpr, fHexDef ); 
				}
				break;
			case '/': 
				pExpr++;
				{
					long iVal = GetSingle( pExpr, fHexDef );
					if ( ! iVal )
					{
						DEBUG_ERR(( "Exp_GetVal Divide by 0\n" ));
						break;
					}
					lVals[iQty-1] /= iVal;
				}
				break;
			case '>': // boolean
				pExpr++;
				if ( pExpr[0] == '=' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] = ( lVals[iQty-1] >= GetSingle( pExpr, fHexDef ));
				}
				else if ( pExpr[0] == '>' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] >>= GetSingle( pExpr, fHexDef );
				}
				else
				{
					lVals[iQty-1] = ( lVals[iQty-1] > GetSingle( pExpr, fHexDef ));
				}
				break;
			case '<': // boolean
				pExpr++;
				if ( pExpr[0] == '=' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] = ( lVals[iQty-1] <= GetSingle( pExpr, fHexDef ));
				}
				else if ( pExpr[0] == '<' )	// boolean ?
				{
					pExpr++;
					lVals[iQty-1] <<= GetSingle( pExpr, fHexDef );
				}
				else
				{
					lVals[iQty-1] = ( lVals[iQty-1] < GetSingle( pExpr, fHexDef )); 
				}
				break;
			case '!':
				pExpr ++;
				if ( pExpr[0] != '=' ) 
					goto nomorevals; // boolean ! is handled as a single expresion.
				pExpr ++;
				lVals[iQty-1] = ( lVals[iQty-1] != GetSingle( pExpr, fHexDef )); 
				break;
			case '=': // boolean
				while ( pExpr[0] == '=' ) 
					pExpr ++;
				lVals[iQty-1] = ( lVals[iQty-1] == GetSingle( pExpr, fHexDef )); 
				break;
			default: 
				if ( fWhitespace )
					goto getmorevals;
				goto nomorevals;
			}
			fWhitespace = false;
		}
	}
nomorevals:
	iRecurse --;

	if (iQty == 0) 
	{
		return( 0 );
	}
	if (iQty == 1) // It's just a simple value
	{
		return( lVals[0] );
	}
	if (iQty == 2) // It's just a simple range....pick one at random
	{
		return( min( lVals[0], lVals[1] ) + GetRandVal( abs( lVals[1] - lVals[0] ) + 1 ));
	}

	// I guess it's weighted values and/or ranges.

	// First get the total of the weights
	int iTotalWeight = 0;
	int iState = 0;
	int i = 0;
	for ( ; i < iQty; i++ )
	{
		switch ( iState )
		{
		case 0:		// Val1
			iState++;
			break;
		case 1:	// Val2 or Weight.
			if ( ! lVals[i] || lVals[i] >= lVals[i-1] )
			{
				// 2nd value of a range.
				iState++;
				break;
			}
		case 2: // Weight.
			DEBUG_CHECK( lVals[i] );
			iTotalWeight += lVals[i];
			iState = 0;
			break;
		}
	}

	// Now roll the dice to see what value to pick
	iTotalWeight = GetRandVal(iTotalWeight) + 1;
	// Now loop to that value
	iState = 0;
	iQty--;
	for ( i=0; i<iQty; i++ )
	{
		switch ( iState )
		{
		case 0:		// Val1
			iState++;
			break;
		case 1:	// Val2 or Weight.
			if ( ! lVals[i] || lVals[i] >= lVals[i-1] )
			{
				// 2nd value of a range.
				iState++;
				break;
			}
		case 2: // Weight.
			DEBUG_CHECK( lVals[i] );
			iTotalWeight -= lVals[i];
			if ( iTotalWeight <= 0 ) 
				goto bailout;
			iState = 0;
			break;
		}
	}

bailout:

	if ( iState == 0 )
		return( lVals[0] );
	if ( iState == 1 ) 
		return( lVals[i-1] );

	return( lVals[i-2] + GetRandVal( lVals[i-1] - lVals[i-2] + 1 ));
}

