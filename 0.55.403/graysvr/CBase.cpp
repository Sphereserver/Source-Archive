//
// CBase.cpp
// Copyright Menace Software (www.menasoft.com).
// base classes.
//
#include "graysvr.h"	// predef header.

///////////////////////////////////////////////////////////////
// -CBaseBaseDef

// 

enum OBC_TYPE
{
	OBC_ARMOR,
	OBC_ATT,
	OBC_BASEID,
	OBC_CAN,
	OBC_DAM,
	OBC_DEFNAME,
	OBC_DEFNAME2,
	OBC_HEIGHT,
	OBC_INSTANCES,
	OBC_NAME,
	OBC_RANGE,
	OBC_RANGEH,
	OBC_RANGEL,
	OBC_RESOURCES,
	OBC_TAG,
	OBC_TAG0,
	OBC_TEVENTS,
	OBC_QTY,
};

LPCTSTR const CBaseBaseDef::sm_szLoadKeys[OBC_QTY+1] =
{
	"ARMOR",
	"ATT",
	"BASEID",
	"CAN",
	"DAM",
	"DEFNAME",
	"DEFNAME2",
	"HEIGHT",
	"INSTANCES",
	"NAME",
	"RANGE",
	"RANGEH",
	"RANGEL",
	"RESOURCES",
	"TAG",
	"TAG0",
	"TEVENTS",
	NULL,
};

void CBaseBaseDef::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
}

bool CBaseBaseDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pChar )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pChar));
	bool	fZero	= false;

	switch ( FindTableHeadSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ) )
	{
	case OBC_ARMOR:
	case OBC_ATT:	// from script
	case OBC_DAM:
		sVal.Format( "%d,%d", m_attackBase, m_attackBase+m_attackRange );
		return( true );
	case OBC_BASEID:
		sVal = g_Cfg.ResourceGetName( GetResourceID());
		return( true );
	case OBC_CAN:
		sVal.FormatHex( m_Can );
		return( true );
	case OBC_HEIGHT:
		sVal.FormatVal( GetHeight());
		return( true );
	case OBC_INSTANCES:
		sVal.FormatVal( GetRefInstances());
		return( true );
	case OBC_NAME:
		sVal = GetName();
		return( true );
	case OBC_RESOURCES:		// Print the resources
		{
			pszKey	+= 9;
			if ( *pszKey == '.' )
			{
				bool	fQtyOnly	= false;
				bool	fKeyOnly	= false;
				SKIP_SEPERATORS( pszKey );
				int		index	= Exp_GetVal( pszKey );
				SKIP_SEPERATORS( pszKey );

				if ( !strnicmp( pszKey, "KEY", 3 ))
					fKeyOnly	= true;
				else if ( !strnicmp( pszKey, "VAL", 3 ))
					fQtyOnly	= true;

				TCHAR *pszTmp = Str_GetTemp();
				m_BaseResources.WriteKeys( pszTmp, index, fQtyOnly, fKeyOnly );
				if ( fQtyOnly && pszTmp[0] == '\0' )
					strcpy( pszTmp, "0" );
				sVal = pszTmp;
			}
			else
			{
				TCHAR *pszTmp = Str_GetTemp();
				m_BaseResources.WriteKeys( pszTmp );
				sVal = pszTmp;
			}
		}
		return( true );
	case OBC_RANGE:
		if ( RangeH() == 0 )
			sVal.Format( "%d", RangeH() );
		else
			sVal.Format( "%d,%d", RangeH(), RangeL() );
		return( true );
	case OBC_RANGEH:
		sVal.FormatHex( RangeH() );
		return( true );
	case OBC_RANGEL:
		sVal.FormatHex( RangeL() );
		return( true );
	case OBC_TAG0:
		fZero	= true;
		pszKey++;
	case OBC_TAG:			// "TAG" = get/set a local tag.
		if ( pszKey[3] != '.' )
			return( false );
		pszKey += 4;
		sVal = m_TagDefs.GetKeyStr(pszKey, fZero );
		return( true );
	case OBC_TEVENTS:
		m_TEvents.WriteResourceRefList( sVal );
		break;
	}
	EXC_CATCH("CBaseBaseDef");
	return( false );
}

bool CBaseBaseDef::r_LoadVal( CScript & s )
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	if ( s.IsKeyHead( "TAG.", 4 ))
	{
		bool fQuoted = false;
		m_TagDefs.SetStr( s.GetKey()+4, fQuoted, s.GetArgStr( &fQuoted ), false );
		return( true );
	}
	if ( s.IsKeyHead( "TAG0.", 5 ))
	{
		bool fQuoted = false;
		m_TagDefs.SetStr( s.GetKey()+5, fQuoted, s.GetArgStr( &fQuoted ), true );
		return( true );
	}

	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case OBC_ARMOR:
	case OBC_ATT:	//from script
	case OBC_DAM:
		{
			int piVal[2];
			int iQty = Str_ParseCmds( s.GetArgStr(), piVal, COUNTOF(piVal));
			m_attackBase = piVal[0];
			if ( iQty > 1 )
			{
				m_attackRange = piVal[1] - m_attackBase;
			}
			else
			{
				m_attackRange = 0;
			}
		}
		return( true );
	case OBC_BASEID:
		return( false );
	case OBC_CAN:
		m_Can = s.GetArgVal() | ( m_Can & ( CAN_C_INDOORS|CAN_C_EQUIP|CAN_C_USEHANDS|CAN_C_NONHUMANOID ));
		return( true );

	case OBC_DEFNAME:
	case OBC_DEFNAME2:
		return SetResourceName( s.GetArgStr());
	case OBC_HEIGHT:
		m_Height	= s.GetArgVal();
		return( true );
	case OBC_INSTANCES:
		return( false );
	case OBC_NAME:
		SetTypeName( s.GetArgStr());
		return( true );
	case OBC_RANGE:
		{
			int piVal[2];
			int iQty = Str_ParseCmds( s.GetArgStr(), piVal, COUNTOF(piVal));
			if ( iQty > 1 )
			{
				m_range	 = ((piVal[0] & 0xff) << 8) & 0xff00;
				m_range	|= (piVal[1] & 0xff);
			}
			else
			{
				m_range	= (WORD) piVal[0];
			}
		}
		return( true );
	case OBC_RESOURCES:
		m_BaseResources.Load( s.GetArgStr());
		return( true );
	case OBC_TEVENTS:
		return( m_TEvents.r_LoadVal( s, RES_EVENTS ));
	}
	EXC_CATCH("CBaseBaseDef");
	return( CScriptObj::r_LoadVal(s));
}

void CBaseBaseDef::CopyBasic( const CBaseBaseDef * pBase )
{
	// do not copy CResourceLink

	if ( m_sName.IsEmpty())	// Base type name. should set this itself most times. (don't overwrite it!)
		m_sName = pBase->m_sName;

	m_wDispIndex = pBase->m_wDispIndex;

	m_Height = pBase->m_Height;
	// m_BaseResources = pBase->m_BaseResources;	// items might not want this.
	m_attackBase = pBase->m_attackBase;
	m_attackRange = pBase->m_attackRange;
	m_Can = pBase->m_Can;
}

void CBaseBaseDef::CopyTransfer( CBaseBaseDef * pBase )
{
	CResourceLink::CopyTransfer( pBase );

	m_sName = pBase->m_sName;
	m_BaseResources = pBase->m_BaseResources;

	CopyBasic( pBase );
}

