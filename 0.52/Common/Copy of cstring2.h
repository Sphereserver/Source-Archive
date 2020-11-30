//
// CGString.H
// Simple shared usefull base classes.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _INC_CSTRING_H
#define _INC_CSTRING_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "common.h"

////////////////////////////////////////////////////////////////////////

#ifndef PT_REG_STRMAX
#define PT_REG_STRMAX		128
#endif
#ifndef PT_REG_ROOTKEY
#define PT_REG_ROOTKEY		HKEY_LOCAL_MACHINE
#endif

////////////////////////////////////////////////////////////////////////

// #include <shellapi.h>
#ifdef HKEY_CLASSES_ROOT

class CAssocReg	// associate members of some class/structure with entries in the registry.
{
	// LAST = { NULL, 0, CAssocReg :: TYPE_VOID }
public:
	const _TCHAR * m_szKey;	// A single key identifier to be cat to a base key. NULL=last
	UINT	m_offset;	// The offset into the class instance for this item.
	enum
	{
		TYPE_VOID = 0,
		TYPE_BYTE,		// 1 byte.
		TYPE_WORD,		// 2 bytes
		TYPE_UINT,		// Whatever the int size is.
		TYPE_DWORD,		// 4 bytes.
		TYPE_CSTRING,
		TYPE_STRING,	// Assume max size of REG_SIZE
	} m_type;

public:
	operator const _TCHAR *() const
	{
		return( m_szKey );
	}
	// get structure value.
	void * GetPtr( const void * pBaseInst ) const
	{
		return( ((BYTE *)pBaseInst) + m_offset );
	}

public:
	long RegQueryPtr( LPCTSTR pszBaseKey, LPSTR pszValue ) const
	{
		// get value from the registry.
		_TCHAR szKey[ PT_REG_STRMAX ];
		LONG lSize = sizeof(szKey);
		pszValue[0] = '\0';
		return( RegQueryValue( PT_REG_ROOTKEY,
			lstrcat( lstrcpy( szKey, pszBaseKey ), m_szKey ),
			pszValue, &lSize ));
	}
	long RegSetPtr( LPCTSTR pszBaseKey, LPCTSTR pszValue ) const
	{
		// set value to the registry.
		_TCHAR szKey[ PT_REG_STRMAX ];
		return( RegSetValue( PT_REG_ROOTKEY,
			lstrcat( lstrcpy( szKey, pszBaseKey ), m_szKey ),
			REG_SZ, pszValue, lstrlen( pszValue )+1 ));
	}

public:
	long RegQuery( LPCTSTR pszBaseKey, void * pBaseInst ) const;
	long RegSet( LPCTSTR pszBaseKey, const void * pBaseInst ) const;

	long RegQueryAll( LPCTSTR pszBaseKey, void * pBaseInst ) const;
	long RegSetAll( LPCTSTR pszBaseKey, const void * pBaseInst ) const
	{
		long lRet = 0;
		for ( int i=0; this[i].m_szKey != NULL; i++ )
		{
			lRet = this[i].RegSet( pszBaseKey, pBaseInst );
		}
		return( lRet );
	}
};

#endif	// HKEY_CLASSES_ROOT

////////////////////////////////////////////////////////////////////////

class CStringListRec : public CObListRec, public CGString
{
	friend class CStringList;
public:
	CStringListRec * GetNext() const
	{
		return( (CStringListRec *) CObListRec :: GetNext());
	}
	CStringListRec( LPCTSTR pszVal ) : CGString( pszVal )
	{
	}
};

class CStringList : public CObList 	// obviously a list of strings.
{
public:
	CStringListRec * GetHead() const
	{
		return( (CStringListRec *) CObList  :: GetHead() );
	}
	void AddHead( LPCTSTR pszVal )
	{
		InsertAfter( new CStringListRec( pszVal ));
	}
};

////////////////////////////////////////////////////////////////////////

class CAssocStr		// Associate a string with a DWORD value
{
	// LAST = { NULL, 0 },
public:
	DWORD m_dwVal;
	const _TCHAR * m_pszLabel;	// NULL = last.
public:
	DWORD Find( const _TCHAR * pszLabel ) const ;
	const _TCHAR * Find( DWORD dwVal ) const ;
	const _TCHAR * Find( _TCHAR * pszOut, DWORD dwVal, const _TCHAR * pszDef = NULL ) const ;
};

////////////////////////////////////////////////////////////////////////

class CAssocVal
{
	// LAST = { 0, 0 },
public:
	DWORD m_dwValA;
	DWORD m_dwValB;
public:
	BOOL CvtAtoB( DWORD dwValue, DWORD FAR * pdwValue ) const
	{
		for ( int i=0; this[i].m_dwValA || this[i].m_dwValB; i++ )
			if ( this[i].m_dwValA == dwValue )
		{
			*pdwValue = this[i].m_dwValB;
			return( TRUE );
		}
		return( FALSE );
	}
};

extern char szTmpBuffer[256];
extern UINT PT_atou( LPCTSTR pszStr );
extern UINT PT_ahextou( LPCTSTR pszStr );

#endif // _INC_CSTRING_H

