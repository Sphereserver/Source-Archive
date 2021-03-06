//
// CString2.CPP
// Copyright Menace Software (www.menasoft.com).
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"
#elif defined(GRAY_SVR) || defined(GRAY_CLIENT)
#include "graycom.h"
#else
#include <windows.h>
#include <stdio.h>
#include "CString.h"
#endif

//***************************************************************************
// String global functions.

static int		Str_iTemp=0;
static TCHAR	Str_szTemp[8][SCRIPT_MAX_LINE_LEN];

TCHAR * Str_GetTemp( void )
{
	// Some scratch string space, random uses
	if ( ++Str_iTemp >= 8 )
		Str_iTemp = 0;
	return( Str_szTemp[Str_iTemp] );
}

LPCTSTR Str_GetArticleAndSpace( LPCTSTR pszWord )
{
	// NOTE: This is wrong many times.
	//  ie. some words need no article (plurals) : boots.

	ASSERT(pszWord);
	static const TCHAR sm_Vowels[] = {	'A', 'E', 'I', 'O', 'U'};
	TCHAR chName = toupper(pszWord[0]);
	for (int x = 0; x < sizeof(sm_Vowels); x++)
	{
		if ( chName == sm_Vowels[x])
			return( "an " );
	}
	return( "a " );
}

int Str_TrimEndWhitespace( TCHAR * pStr, int len )
{
	ASSERT( len >= 0 );
	while ( len>0 )
	{
		len --;
		if ( ! ISWHITESPACE( pStr[len] ))
		{
			++len;
			break;
		}
	}
	pStr[len] = '\0';
	return( len );
}

TCHAR * Str_TrimWhitespace( TCHAR * pStr )
{
	GETNONWHITESPACE( pStr );
	Str_TrimEndWhitespace( pStr, strlen(pStr));
	return( pStr );
}

TCHAR * Str_GetNonWhitespace( LPCTSTR pStr )
{
	GETNONWHITESPACE( pStr );
	return( (TCHAR*) pStr );
}

bool Str_Parse( TCHAR * pLine, TCHAR ** ppLine2, LPCTSTR pszSep )
{
	// Parse a list of args. Just get the next arg.
	// similar to strtok()
	// RETURN: true = the second arg is valid.

	if ( pszSep == NULL )	// default sep.
		pszSep = _TEXT("=, \t");

	// skip leading white space.
	TCHAR * pNonWhite = Str_GetNonWhitespace( pLine );
	if ( pNonWhite != pLine )
	{
		memmove( pLine, pNonWhite, strlen( pNonWhite ) + 1 );
	}

	TCHAR ch;
	for ( ;true; pLine++ )
	{
		ch = *pLine;
		if ( ch == '\0' )	// no args i guess.
		{
			if ( ppLine2 != NULL )
			{
				*ppLine2 = pLine;
			}
			return false;
		}
		if ( strchr( pszSep, ch ))
			break;
	}

	*pLine++ = '\0';
	if ( isspace( ch ))	// space seperators might have other seps as well ?
	{
		GETNONWHITESPACE( pLine );
		ch = *pLine;
		if ( ch && strchr( pszSep, ch ))
		{
			pLine++;
		}
	}

	// skip leading white space on args as well.
	if ( ppLine2 != NULL )
	{
		*ppLine2 = Str_TrimWhitespace( pLine );
	}
	return true;
}

int Str_ParseCmds( TCHAR * pszCmdLine, TCHAR ** ppCmd, int iMax, LPCTSTR pszSep )
{
	int iQty = 0;
	if ( pszCmdLine != NULL && pszCmdLine[0] != '\0' )
	{
		ppCmd[0] = pszCmdLine;
		iQty++;
		while ( Str_Parse( ppCmd[iQty-1], &(ppCmd[iQty]), pszSep ))
		{
			if ( ++iQty >= iMax )
				break;
		}
	}
	for ( int j=iQty; j<iMax; j++ )
		ppCmd[j] = NULL;	// terminate if possible.
	return( iQty );
}

#ifdef GRAY_SVR
int Str_ParseCmds( TCHAR * pszCmdLine, int * piCmd, int iMax, LPCTSTR pszSep )
{
	TCHAR * ppTmp[256];
	if ( iMax > COUNTOF(ppTmp))
		iMax = COUNTOF(ppTmp);
	int iQty = Str_ParseCmds( pszCmdLine, ppTmp, iMax, pszSep );
	int i;
	for ( i=0; i<iQty; i++ )
	{
		piCmd[i] = Exp_GetVal(ppTmp[i]);
	}
	for ( ;i<iMax;i++)
	{
		piCmd[i] = 0;
	}
	return( iQty );
}
#endif

static int Str_CmpHeadI( LPCTSTR pszFind, LPCTSTR pszTable )
{
	for ( int i=0; true; i++ )
	{
		TCHAR ch1 = toupper( pszFind[i] );
		TCHAR ch2 = toupper( pszTable[i] );
		if ( ch2 == 0 )
		{
			if ( !isalnum(ch1))
				return 0;
			return( ch1 - ch2 );
		}
		if ( ch1 != ch2 )
		{
			return( ch1 - ch2 );
		}
	}
}

int FindTableHeadSorted( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// Do a binary search (un-cased) on a sorted table.
	// RETURN: -1 = not found

#ifdef _DEBUG
	// make sure the table acually IS sorted !
	for ( int i=0; i<iCount-1; i++ )
	{
		LPCTSTR pszName1 = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		LPCTSTR pszName2 = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( (i+1)*iElemSize )));
		int iCompare = strcmpi( pszName1, pszName2 );
		ASSERT( iCompare < 0 );
	}
#endif

	int iHigh = iCount-1;
	if ( iHigh < 0 )
	{
		return -1;
	}
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		LPCTSTR pszName = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		int iCompare = Str_CmpHeadI( pszFind, pszName );
		if ( iCompare == 0 )
			return( i );
		if ( iCompare > 0 )
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( -1 );
}

int FindTableHead( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	for ( int i=0; i<iCount; i++ )
	{
		int iCompare = Str_CmpHeadI( pszFind, *ppszTable );
		if ( ! iCompare )
			return( i );
		ppszTable = (LPCTSTR const *)((( const BYTE*) ppszTable ) + iElemSize );
	}
	return( -1 );
}

int FindTableSorted( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// Do a binary search (un-cased) on a sorted table.
	// RETURN: -1 = not found

#ifdef _DEBUG
	// make sure the table acually IS sorted !
	for ( int i=0; i<iCount-1; i++ )
	{
		LPCTSTR pszName1 = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		LPCTSTR pszName2 = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( (i+1)*iElemSize )));
		int iCompare = strcmpi( pszName1, pszName2 );
		ASSERT( iCompare < 0 );
	}
#endif

	int iHigh = iCount-1;
	if ( iHigh < 0 )
	{
		return -1;
	}
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		LPCTSTR pszName = *((LPCTSTR const *) ((( const BYTE*) ppszTable ) + ( i*iElemSize )));
		int iCompare = strcmpi( pszFind, pszName );
		if ( iCompare == 0 )
			return( i );
		if ( iCompare > 0 )
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( -1 );
}

int FindTable( LPCTSTR pszFind, LPCTSTR const * ppszTable, int iCount, int iElemSize )
{
	// A non-sorted table.
	for ( int i=0; i<iCount; i++ )
	{
		if ( ! strcmpi( *ppszTable, pszFind ))
			return( i );
		ppszTable = (LPCTSTR const *)((( const BYTE*) ppszTable ) + iElemSize );
	}
	return( -1 );
}

int Str_GetBare( TCHAR * pszOut, LPCTSTR pszInp, int iMaxOutSize, LPCTSTR pszStrip )
{
	// Allow only the basic set of chars. Non UNICODE !
	// That the client can deal with.
	// Basic punctuation and alpha and numbers.
	// RETURN: Output length.

	if ( pszStrip == NULL )
		pszStrip = "{|}~";	// client cant print these.

	//GETNONWHITESPACE( pszInp );	// kill leading white space.

	int j=0;
	for ( int i=0; true; i++ )
	{
		TCHAR ch = pszInp[i];
		if ( ch )
		{
			if ( ch < ' ' || ch >= 127 )
				continue;	// Special format chars.
			int k=0;
			while ( pszStrip[k] && pszStrip[k] != ch )
				k++;
			if ( pszStrip[k] )
				continue;
			if ( j >= iMaxOutSize-1 )
			{
				ch = 0;
			}
		}
		pszOut[j++] = ch;
		if ( ch == 0 )
			break;
	}
	return( j-1 );
}


bool Str_Check( const TCHAR * pszIn )
{
	if ( !pszIn )
		return false;
	if ( strchr( pszIn, 0x0A ) == 0 && strchr( pszIn, 0x0D ) == 0 )
		return false;
	return true;
}


TCHAR * Str_MakeFiltered( TCHAR * pStr )
{
	int len = strlen( pStr );
	for ( int i=0; len; i++, len-- )
	{
		if ( pStr[i] == '\\' )
		{
			switch ( pStr[i+1] )
			{
			case 'b': pStr[i] = '\b'; break;
			case 'n': pStr[i] = '\n'; break;
			case 'r': pStr[i] = '\r'; break;
			case 't': pStr[i] = '\t'; break;
			case '\\': pStr[i] = '\\'; break;
			}
			len --;
			memmove( pStr+i+1, pStr+i+2, len );
		}
	}
	return( pStr );
}

void Str_MakeUnFiltered( TCHAR * pStrOut, LPCTSTR pStrIn, int iSizeMax )
{
	int len = strlen( pStrIn );
	int iIn = 0;
	int iOut = 0;
	for ( ; iOut < iSizeMax && iIn <= len; iIn++, iOut++ )
	{
		TCHAR ch = pStrIn[iIn];
		switch ( ch )
		{
		case '\b': ch = 'b'; break;
		case '\n': ch = 'n'; break;
		case '\r': ch = 'r'; break;
		case '\t': ch = 't'; break;
		case '\\': ch = '\\'; break;
		default:
			pStrOut[iOut] = ch;
			continue;
		}

		pStrOut[iOut++] = '\\';
		pStrOut[iOut] = ch;
	}
}

#if 0

bool Str_Match_Simple( LPCTSTR pPattern , LPCTSTR pText )
{
	// recursive match with backtracking. Will cope with match( "*.c", "a.b.c" )
    while (*pPattern)
	{
		if (*pPattern == '*')
		{
			return ((*pText && Text_Match_Simple(pText+1, pPattern)) // match * against first char
				|| Text_Match_Simple(pText, pPattern+1));	// match * against empty string
		}
		if ((*pPattern != '?' && *pText != *pPattern)	// if literal char doesn't match
			|| !*pText)				// or pText is empty
			break;
		pPattern++;
		pText++;			// advance over matching chars
	}
    return (!*pText) && (!*pPattern);
}

#endif

#define TOLOWER tolower

static MATCH_TYPE Str_Match_After_Star( LPCTSTR pPattern, LPCTSTR pText )
{
	// pass over existing ? and * in pattern
	for ( ; *pPattern == '?' || *pPattern == '*'; pPattern++ )
	{
		// take one char for each ? and +
		if (  *pPattern == '?' &&
			! *pText++ )		// if end of text then no match
			return MATCH_ABORT;
	}

	// if end of pattern we have matched regardless of text left
	if ( !*pPattern )
		return MATCH_VALID;

	// get the next character to match which must be a literal or '['
	TCHAR nextp = TOLOWER( *pPattern );
	MATCH_TYPE match = MATCH_INVALID;

	// Continue until we run out of text or definite result seen
	do
	{
		// a precondition for matching is that the next character
		// in the pattern match the next character in the text or that
		// the next pattern char is the beginning of a range.  Increment
		// text pointer as we go here
		if ( nextp == TOLOWER( *pText ) || nextp == '[' )
		{
			match = Str_Match(pPattern, pText);
			if ( match == MATCH_VALID )
				break;
		}

		// if the end of text is reached then no match
		if ( !*pText++ )
			return MATCH_ABORT;

	} while (
		match != MATCH_ABORT &&
		match != MATCH_PATTERN );

	return match;	// return result
}

MATCH_TYPE Str_Match( LPCTSTR pPattern, LPCTSTR pText )
{
	// case independant

	TCHAR range_start;
	TCHAR range_end;  // start and end in range

	for ( ; *pPattern; pPattern++, pText++ )
	{
		// if this is the end of the text then this is the end of the match
		if (!*pText)
		{
			return ( *pPattern == '*' && *++pPattern == '\0' ) ?
				MATCH_VALID : MATCH_ABORT;
		}
		// determine and react to pattern type
		switch ( *pPattern )
		{
		// single any character match
		case '?':
			break;
		// multiple any character match
		case '*':
			return Str_Match_After_Star( pPattern, pText );
		// [..] construct, single member/exclusion character match
		case '[':
			{
				// move to beginning of range
				pPattern++;
				// check if this is a member match or exclusion match
				bool fInvert = false;             // is this [..] or [!..]
				if ( *pPattern == '!' || *pPattern == '^')
				{
					fInvert = true;
					pPattern++;
				}
				// if closing bracket here or at range start then we have a
				// malformed pattern
				if ( *pPattern == ']' )
					return MATCH_PATTERN;

				bool fMemberMatch = false;       // have I matched the [..] construct?
				while (true)
				{
					// if end of construct then fLoop is done
					if (*pPattern == ']')
					{
						break;
					}

					// matching a '!', '^', '-', '\' or a ']'
					if ( *pPattern == '\\' )
						range_start = range_end = TOLOWER( *++pPattern );
					else
						range_start = range_end = TOLOWER( *pPattern );

					// if end of pattern then bad pattern (Missing ']')
					if (!*pPattern)
						return MATCH_PATTERN;

					// check for range bar
					if (*++pPattern == '-')
					{
						// get the range end
						range_end = TOLOWER( *++pPattern );
						// if end of pattern or construct then bad pattern
						if ( range_end == '\0' || range_end == ']')
							return MATCH_PATTERN;
						// special character range end
						if ( range_end == '\\')
						{
							range_end = TOLOWER( *++pPattern );
							// if end of text then we have a bad pattern
							if (!range_end)
								return MATCH_PATTERN;
						}
						// move just beyond this range
						pPattern++;
					}

					// if the text character is in range then match found.
					// make sure the range letters have the proper
					// relationship to one another before comparison
					TCHAR chText = TOLOWER( *pText );
					if ( range_start < range_end  )
					{
						if ( chText >= range_start && chText <= range_end)
						{
							fMemberMatch = true;
							break;
						}
					}
					else
					{
						if (chText >= range_end && chText <= range_start)
						{
							fMemberMatch = true;
							break;
						}
					}
				}	// while

				// if there was a match in an exclusion set then no match
				// if there was no match in a member set then no match
				if (( fInvert && fMemberMatch ) ||
					 !( fInvert || fMemberMatch ))
					return MATCH_RANGE;

				// if this is not an exclusion then skip the rest of the [...]
				//  construct that already matched.
				if (fMemberMatch)
				{
					while (*pPattern != ']')
					{
						// bad pattern (Missing ']')
						if (!*pPattern)
							return MATCH_PATTERN;
						// skip exact match
						if (*pPattern == '\\')
						{
							pPattern++;
							// if end of text then we have a bad pattern
							if (!*pPattern)
								return MATCH_PATTERN;
						}
						// move to next pattern char
						pPattern++;
					}
				}
			}
			break;

		// must match this character (case independant) ?exactly
		default:
			if ( TOLOWER( *pPattern ) != TOLOWER( *pText ))
                return MATCH_LITERAL;
        }
	}
	// if end of text not reached then the pattern fails
	if ( *pText )
		return MATCH_END;
	else
		return MATCH_VALID;
}

#if 0

UINT Str_ahextou( LPCTSTR pszStr )
{
	// I know this is going to be a hex number.
	UINT val = 0;
	while (true)
	{
		_TCHAR ch = *pszStr;
		if ( ch >= _TEXT('0') && ch <= _TEXT('9'))
			ch -= '0';
		else
		{
			ch |= 0x20;	// toupper()
			if ( ch > 'f' || ch <'a' )
				break;
			ch -= 'a' - 10;
		}
		val *= 0x10;
		val += ch;
		pszStr ++;
	}
	return( val );
}

UINT Str_atou( LPCTSTR pszStr )
{
	// ASSERT( pszStr != NULL );
	UINT val = 0;
	if ( pszStr[0] == '0' && pszStr[1] == 'x' )
	{
		return( PT_ahextou( pszStr+2 ));
	}
	while (true)
	{
		_TCHAR ch = *pszStr;
		if ( ch < _TEXT('0') || ch > _TEXT('9'))
			break;
		val *= 10;
		val += ch - _TEXT('0');
		pszStr ++;
	}
	return( val );
}

#endif

