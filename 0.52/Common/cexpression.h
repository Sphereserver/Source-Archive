//
// CExpression.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CEXPRSSION_H
#define _INC_CEXPRSSION_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "cstring.h"
#include "carray.h"
#include "cscript.h"

class CVarDef : public CMemDynamic, public CScriptObj	// A variable from GRAYDEFS.SCP or other.
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	const CGString m_sName;	// the key for sorting.
	CGString m_sVal;	// the assigned value.
public:
	const TCHAR * GetName() const
	{
		return( m_sName );
	}
	const TCHAR * GetVal() const
	{
		return( m_sVal );
	}
	void SetVal( const TCHAR * pszVal )
	{
		m_sVal.Copy( pszVal );
	}
	bool r_LoadVal( CScript & s )
	{
		SetVal( s.GetArgStr());
		return( true );
	}
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc = NULL )
	{
		sVal = GetVal();
		return( true );
	}
#if 0
	CVarDef( CScript & s ) :
		m_sName( s.GetKey()),
		m_sVal( s.GetArgStr())
	{
	}
#endif
	CVarDef( const TCHAR * pszName, const TCHAR * pszVal ) :
		m_sName( pszName ),
		m_sVal( pszVal )
	{
	}
	CVarDef( const TCHAR * pszName ) :
		m_sName( pszName )
	{
	}
};

struct CVarDefArray : public CGObSortArray< CVarDef *, const TCHAR *>
{
	// Sorted array
	int CompareKey( const TCHAR * pName, CVarDef * pVar ) const
	{
		ASSERT( pVar );
		return( strcmpi( pName, pVar->GetName()));
	}
	int FindValNum( int iVal ) const;
	int FindValStr( const TCHAR * pVal ) const;
	void Add( CVarDef * pVar )
	{
		if ( AddSortKey( pVar, pVar->GetName()) < 0 )
		{
			// NOTE: pVar has ALREADY BEEN DELETED !!!???
			DEBUG_ERR(( "Duplicate def %s\n", pVar->GetName()));	// duplicate should not happen.
		}
	}
};

extern class CExpression
{
private:
	CVarDef * GetVarDef( const TCHAR * & pArgs );
	bool GetVarDef( const TCHAR * & pArgs, long * plVal );
public:
	CVarDefArray m_VarDefs;	 // Defined variables in sorted order.

	int  GetSingle( const TCHAR * & pArgs, bool fHexDef = false );
	inline int  GetSingle(TCHAR * & pArgs, bool fHexDef = false) {
		return GetSingle(const_cast<const TCHAR*&>(pArgs), fHexDef);
	}

	int  GetVal( const TCHAR * & pArgs, bool fHexDef = false );
	inline int GetVal(TCHAR * & pArgs, bool fHexDef = false) {
		return GetVal(const_cast<const TCHAR * &>(pArgs), fHexDef);
	}

	bool SetVarDef( const TCHAR * pszName, const TCHAR * pszVal );

} g_Exp;

// Numeric formulas
extern int GetRandVal( int iqty );
extern int GetLog2( UINT iVal );
extern int GW_GetSCurve( int iValDiff, int iVariance );
extern int GW_GetBellCurve( int iValDiff, int iVariance );

extern DWORD ahextoi( const TCHAR * pArgs ); // Convert hex string to integer

#define Exp_GetSingle( pa ) g_Exp.GetSingle( pa, false )
#define Exp_GetVal( pa )	g_Exp.GetVal( pa, false )	
#define Exp_GetHex( pa )	g_Exp.GetVal( pa, true )

#endif	// _INC_CEXPRSSION_H