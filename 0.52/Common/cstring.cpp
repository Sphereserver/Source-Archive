//
// CGString.CPP
// Copyright Menace Software (www.menasoft.com).
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#else
#include "graycom.h"
#endif

//***************************************************************************
// String global functions.

static int		iTempStr=0;
static TCHAR	szTempStr[8][MAX_SCRIPT_LINE_LEN];

#ifndef _WIN32
void strupr( TCHAR * pText )
{
	// No portable UNIX equiv to this.
	for ( ;pText[0] != '\0'; pText++ )
	{
		pText[0] = toupper( pText[0] );
	}
}
#endif

int strcpylen( TCHAR * pDst, const TCHAR * pSrc )
{
	strcpy( pDst, pSrc );
	return( strlen( pDst ));
}

int strcpylen( TCHAR * pDst, const TCHAR * pSrc, int imaxlen )
{
	ASSERT( imaxlen );
	strncpy( pDst, pSrc, imaxlen );
	pDst[imaxlen-1] = '\0';
	return( strlen( pDst ));
}

TCHAR * GetTempStr( void )
{
	// Some scratch string space, random uses
	if ( ++iTempStr >= 8 ) 
		iTempStr = 0;
	return( szTempStr[iTempStr] );
}

int TrimEndWhitespace( TCHAR * pStr, int len )
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

TCHAR * TrimWhitespace( TCHAR * pStr )
{
	GETNONWHITESPACE( pStr );
	TrimEndWhitespace( pStr, strlen(pStr));
	return( pStr );
}

TCHAR * GetNonWhitespace( const TCHAR * pStr )
{
	GETNONWHITESPACE( pStr );
	return( (TCHAR*) pStr );
}

bool Parse( TCHAR * pLine, TCHAR ** ppLine2, const TCHAR * pSep )
{
	// Parse a list of args. Just get the next arg.
	// similar to strtok()
	// RETURN: true = the second arg is valid.

	if ( pSep == NULL )
		pSep = "=, \t";

	TCHAR * pNonWhite = GetNonWhitespace( pLine );
	if ( pNonWhite != pLine )
	{
		memmove( pLine, pNonWhite, strlen( pNonWhite ) + 1 );
	}

	int i=0;
	for ( ;true; i++ )
	{
		if ( pLine[i] == '\0' )	// no args i guess.
		{
			if ( ppLine2 != NULL )
			{
				*ppLine2 = &pLine[i];
			}
			return false;
		}
		for ( int j=0; pSep[j]; j++ )
			if ( pLine[i] == pSep[j] )
				goto breakout;
	}

breakout:
	pLine[i++] = '\0';

	// skip leading white space.
	if ( ppLine2 != NULL )
	{
		*ppLine2 = TrimWhitespace( &pLine[i] );
	}
	return true;
}

int ParseCmds( TCHAR * pCmdLine, TCHAR ** ppCmd, int iMax, const TCHAR * pSep )
{
	int iQty = 0;
	if ( pCmdLine != NULL && pCmdLine[0] != '\0' )
	{
		ppCmd[0] = pCmdLine;
		iQty++;
		while ( Parse( ppCmd[iQty-1], &(ppCmd[iQty]), pSep ))
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
int ParseCmds( TCHAR * pCmdLine, int * piCmd, int iMax, const TCHAR * pSep )
{
	TCHAR * ppTmp[256];
	if ( iMax > COUNTOF(ppTmp))
		iMax = COUNTOF(ppTmp);
	int iQty = ParseCmds( pCmdLine, ppTmp, iMax, pSep );
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

int FindTableHead( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize )
{
	for ( int i=0; i<iCount; i++ )
	{
		if ( ! strnicmp( pFind, *ppTable, strlen(*ppTable)))
			return( i );
		ppTable = (const TCHAR * const *)((( const BYTE*) ppTable ) + iElemSize );
	}
	return( -1 );
}

int FindTableSorted( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize )
{
	// Do a binary search (un-cased) on a sorted table.

//#ifdef COMMENT
#ifdef _DEBUG
	// make sure the table acually IS sorted !
	for ( int i=0; i<iCount-1; i++ )
	{
		const TCHAR * pszName1 = *((const TCHAR * const *) ((( const BYTE*) ppTable ) + ( i*iElemSize )));
		const TCHAR * pszName2 = *((const TCHAR * const *) ((( const BYTE*) ppTable ) + ( (i+1)*iElemSize )));
		int iCompare = strcmpi( pszName1, pszName2 );
		ASSERT( iCompare < 0 );
	}
#endif
//#endif

	int iHigh = iCount-1;
	if ( iHigh < 0 )
	{
		return -1;
	}
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		const TCHAR * pszName = *((const TCHAR * const *) ((( const BYTE*) ppTable ) + ( i*iElemSize )));
		int iCompare = strcmpi( pFind, pszName );
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

int FindTable( const TCHAR * pFind, const TCHAR * const * ppTable, int iCount, int iElemSize )
{
	// A non-sorted table.
	for ( int i=0; i<iCount; i++ )
	{
		if ( ! strcmpi( *ppTable, pFind ))
			return( i );
		ppTable = (const TCHAR * const *)((( const BYTE*) ppTable ) + iElemSize );
	}
	return( -1 );
}

int GetBareText( TCHAR * pszOut, const TCHAR * pszInp, int iMaxSize, const TCHAR * pszStrip )
{
	// Allow only the basic set of chars. Non UNICODE !
	// That the client can deal with.
	// Basic punctuation and alpha and numbers.
	// RETURN: Output length.

	if ( pszStrip == NULL ) 
		pszStrip = "{|}~";	// client cant print these.

	GETNONWHITESPACE( pszInp );	// kill leading white space.

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
			if ( j >= iMaxSize-1 )
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

TCHAR * MakeFilteredStr( TCHAR * pStr )
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

#if 0

bool Text_Match_Simple( const TCHAR *pPattern , const TCHAR *pText )
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

static MATCH_TYPE Text_Match_After_Star( const TCHAR *pPattern, const TCHAR *pText )
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
			match = Text_Match(pPattern, pText);
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

MATCH_TYPE Text_Match( const TCHAR *pPattern, const TCHAR * pText )
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
			return Text_Match_After_Star( pPattern, pText );
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

#ifndef _AFXDLL

//***************************************************************************
// -CGString

const TCHAR CGString::sm_Nil = '\0'; // Use this instead of null.

void CGString::Empty()
{
	if ( m_pchData != &sm_Nil )
	{
		delete [] m_pchData;
		Init();
	}
}

int CGString::SetLength( int iNewLength )
{
	ASSERT( IsValid());
	if ( ! iNewLength )
	{
		Empty();
		return( 0 );
	}

	TCHAR * pNewData = new TCHAR [ iNewLength + 1 ];
	ASSERT( pNewData );
	strncpy( pNewData, m_pchData, iNewLength );
	pNewData[ iNewLength ] = '\0';	// validation.

	Empty();

	m_pchData = pNewData;
	m_iLength = iNewLength;
	return( m_iLength );
}

void CGString::Copy( const TCHAR * pStr )
{
	// DEBUG_MSG(( "CGString:Copy" ));
	if ( pStr != m_pchData )    // Same string.
	{
		Empty();
		if ( pStr != NULL )
		{
			if ( SetLength( strlen( pStr )))
			{
				strcpy( m_pchData, pStr );
			}
		}
	}
}

int CGString::VFormat( const TCHAR * pStr, va_list args )
{
	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	vsprintf( szTemp, pStr, args );
	Copy( szTemp );
	return( GetLength());
}

const CGString& CGString::operator+=(const TCHAR* psz)	// like strcat
{
	ASSERT( psz );
	int iLenCat = strlen( psz ); 
	if ( iLenCat )
	{
		int iLenOld = GetLength();
		SetLength(iLenOld+iLenCat);
		strcpy( m_pchData+iLenOld, psz );
	}
	return( *this );
}

#ifdef UNICODE
const CGString& CGString::Copy( const char * pszStr )
{
	// Get a string that is not in UNICODE
	Empty();
	if ( pszStr != NULL )
	{
		int nBaseLen = lstrlen(pszStr);
		m_iLength = nBaseLen * sizeof(_TCHAR);
		if ( m_iLength )
		{
			m_pchData = new _TCHAR [ m_iLength + 1 ];
			if (0 == MultiByteToWideChar(
				CP_ACP,					// code page
				MB_COMPOSITE | MB_ERR_INVALID_CHARS,// character translations
				pszStr,					// address of string to map
				nBaseLen,				// size of that non-UNICODE string
				m_pchData,				// address of wide-character buffer
				m_iLength				// size of buffer
				))
			{	// -- don't much care about the error, just is there --
				Init();
			}
		}
	}
	return *this;
}
#endif

#endif // _AFXDLL

#if 0	// _WINDOWS

int CGString::Load( int id )
{
	char szText[ _MAX_PATH ];
	int iLen = LoadString( SP_App.m_hInstance, id, szText, sizeof( szText ));
	Copy( szText );
	return( iLen );
}

CGString::CGString( int id )
{
	m_pchData = &Str_Nil;
	m_iLength = 0;
	Load( id );
}

#endif

//***************************************************************************
// -CValStr

const TCHAR * CValStr::FindName( int iVal ) const
{
	int i=0;
	for ( ; this[i].m_pszName; i++ )
	{
		if ( iVal < this[i+1].m_iVal )
			return( this[i].m_pszName );
	}
	return( this[i-1].m_pszName );
}

//***************************************************************************

#if 0

UINT PT_ahextou( LPCTSTR pszStr )
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

UINT PT_atou( LPCTSTR pszStr )
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

//***************************************************************************

const _TCHAR * CAssocStr::Find( DWORD dwVal ) const
{
	for ( int i=0; this[i].m_pszLabel != NULL; i++ )
	{
		if ( this[i].m_dwVal == dwVal ) 
			break;
	}
	return( this[i].m_pszLabel );
}

const _TCHAR * CAssocStr::Find( _TCHAR * pszOut, DWORD dwVal, const _TCHAR * pszDef ) const
{
	//
	// PURPOSE:
	//  Find a description string for the index (wValue) provided.
	//  Pak the result string into (pszOut).
	//  if no value found then use the (pszDef) string and sprintf to make default
	//

	const _TCHAR * pszVal = Find( dwVal );
	if ( pszVal == NULL )
	{
		pszVal = pszDef;
	}
	if ( pszVal != NULL )
	{
		if ( pszOut != NULL )
		{
			wsprintf( pszOut, pszVal, dwVal );   // Not here so use default.
			return( pszOut );
		}
	}
	return( pszVal );
}

#endif

