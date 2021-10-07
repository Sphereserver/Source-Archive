//
// Crect.cpp
// Copyright Menace Software (www.menasoft.com).
// Common for client and server.
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#else
#include "graycom.h"
#include "graymul.h"
#include "CRect.h"
#endif

//*************************************************************************
// -CPointBase

LPCTSTR const CPointBase::sm_szDirs[DIR_QTY+1] =
{
	_TEXT("North"),
	_TEXT("North East"),
	_TEXT("East"),
	_TEXT("South East"),
	_TEXT("South"),
	_TEXT("South West"),
	_TEXT("West"),
	_TEXT("North West"),
	_TEXT("Here"),
};

const int CPointBase::sm_Moves[DIR_QTY+1][2] =
{
	 0, -1, // DIR_N
	 1, -1, // DIR_NE
	 1,  0, // DIR_E
	 1,  1, // DIR_SE
	 0,  1, // DIR_S
	-1,  1, // DIR_SW
	-1,  0, // DIR_W
	-1, -1, // DIR_NW
 	 0,	 0,	// DIR_QTY = here.
};


enum PT_TYPE
{
	PT_ISNEARTYPE,
	PT_M,
	PT_MAPPLANE,
	PT_TERRAIN,
	PT_TYPE,
	PT_X,
	PT_Y,
	PT_Z,
	PT_QTY,
};


LPCTSTR const CPointBase::sm_szLoadKeys[PT_QTY+1] =
{
	"ISNEARTYPE",
	"M",
	"MAPPLANE",
	"TERRAIN",
	"TYPE",
	"X",
	"Y",
	"Z",
	NULL,
};

bool CPointBase::r_WriteVal( LPCTSTR pszKey, CGString & sVal ) const
{
	if ( !strnicmp( pszKey, "STATICS", 7 ) )
	{
		pszKey	+= 7;
		const CGrayMapBlock * pBlock	= g_World.GetMapBlock( *(this) );
		if ( !pBlock ) return false;

		if ( *pszKey == '\0' )
		{
			int		iStaticQty	= 0;
			for ( int i = 0; i <  pBlock->m_Statics.GetStaticQty(); i++ )
			{
				const CUOStaticItemRec * pStatic = pBlock->m_Statics.GetStatic( i );
				CPointMap ptTest( pStatic->m_x+pBlock->m_x, pStatic->m_y+pBlock->m_y, pStatic->m_z, this->m_mapplane );
				if ( this->GetDist( ptTest ) > 0 )
					continue;
				iStaticQty++;
			}

			sVal.FormatVal( iStaticQty );
			return true;
		}

		SKIP_SEPERATORS( pszKey );

		const CUOStaticItemRec * pStatic	= NULL;


		int		iStatic	= 0;
		int		type	= 0;
		
		if ( !strnicmp( pszKey, "FINDID", 6 ) )
		{
			pszKey	+= 6;
			SKIP_SEPERATORS( pszKey );
			iStatic	= Exp_GetVal( pszKey );
			type	= RES_GET_TYPE( iStatic );
			if ( type == 0 )
				type	= RES_ITEMDEF;
			SKIP_SEPERATORS( pszKey );
		}
		else
		{
			iStatic	= Exp_GetVal( pszKey );
			type	= RES_GET_TYPE( iStatic );
		}
		
		if ( type == RES_ITEMDEF )
		{
			CItemBase * pItemDef	= CItemBase::FindItemBase( (ITEMID_TYPE) RES_GET_INDEX(iStatic) );
			if ( !pItemDef )
			{
				sVal.FormatVal( 0 );
				return false;
			}
			for ( int i = 0; i < pBlock->m_Statics.GetStaticQty(); pStatic	= NULL, i++ )
			{
				pStatic = pBlock->m_Statics.GetStatic( i );
				CPointMap ptTest( pStatic->m_x+pBlock->m_x, pStatic->m_y+pBlock->m_y, pStatic->m_z, this->m_mapplane );
				if ( this->GetDist( ptTest ) > 0 )
					continue;
				if ( pStatic->GetDispID() == pItemDef->GetDispID() )
					break;
			}
		}
		else
		{
			for ( int i = 0; i <  pBlock->m_Statics.GetStaticQty(); pStatic	= NULL, i++ )
			{
				pStatic = pBlock->m_Statics.GetStatic( i );
				CPointMap ptTest( pStatic->m_x+pBlock->m_x, pStatic->m_y+pBlock->m_y, pStatic->m_z, this->m_mapplane );
				if ( this->GetDist( ptTest ) > 0 )
					continue;
				if ( iStatic == 0 )
					break;
				iStatic--;
			}
		}

		if ( !pStatic )
		{
			/*
			SKIP_SEPERATORS( pszKey );
			if ( !*pszKey )
				pszKey	= "ID";

			if ( !strnicmp( pszKey, "ID", 2 ) )
			{
				sVal.FormatHex( 0 );
				return true;
			}

			if ( iStaticDbg < 0 )
			else
			DEBUG_ERR(("Non-existing static %d at MAP point %d,%d  mapplane %d." DEBUG_CR, iStaticDbg,
						m_x, m_y, m_mapplane ));

			return false;
			*/
			sVal.FormatHex( 0 );
			return true;
		}

		SKIP_SEPERATORS( pszKey );
		if ( !*pszKey )
			pszKey	= "ID";

		ITEMID_TYPE idTile = pStatic->GetDispID();

		if ( !strnicmp( pszKey, "ID", 2 ) )
		{
			sVal.FormatHex( idTile );
			return true;
		}

		// Check the script def for the item.
		CItemBase * pItemDef = CItemBase::FindItemBase( idTile );
		if ( pItemDef == NULL )
		{

			DEBUG_ERR(("Must have ITEMDEF section for item ID %x" DEBUG_CR, idTile ));
			return false;
		}

		return pItemDef->r_WriteVal( pszKey, sVal, &g_Serv );
	}
	
	int		index = FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
	if ( index < 0 )
		return false;

	switch ( index )
	{
	case PT_M:
	case PT_MAPPLANE:
		sVal.FormatVal(m_mapplane);
		break;
	case PT_X:
		sVal.FormatVal(m_x);
		break;
	case PT_Y:
		sVal.FormatVal(m_y);
		break;
	case PT_Z:
		sVal.FormatVal(m_z);
		break;
	case PT_ISNEARTYPE:
	{
		int		iType		= Exp_GetVal( pszKey );
		int		iDistance;
		SKIP_SEPERATORS( pszKey );
		if ( !*pszKey )
			iDistance	= 0;
		else
			iDistance	= Exp_GetVal( pszKey );
		sVal.FormatVal( g_World.IsItemTypeNear(
				*this,
				(IT_TYPE) GETINTRESOURCE( iType ),
				iDistance ) );
	}
	default:
		{
			const CGrayMapBlock * pBlock	= g_World.GetMapBlock( *(this) );
			const CUOMapMeter * pMeter = g_World.GetMapMeter( (*this) );
			if ( pMeter ) switch( index )
			{
			case PT_TYPE:
				{
					CItemTypeDef *	pTypeDef	= g_World.GetTerrainItemTypeDef( pMeter->m_wTerrainIndex );
					sVal.Format( pTypeDef ? pTypeDef->GetResourceName() : "" );
				}
				return true;	
			case PT_TERRAIN:
				sVal.FormatHex( pMeter->m_wTerrainIndex );
				return true;
			}
		}

		return( true );
	}

	return( true );
}

bool CPointBase::r_LoadVal( LPCTSTR pszKey, LPCTSTR pszArgs )
{
	int index = FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
	if ( index <= 0 )
	{
		return( false );
	}
	int iVal = Exp_GetVal(pszArgs);
	switch (index)
	{
	case 0: m_mapplane = iVal; break;
	case 1: m_x = iVal; break;
	case 2: m_y = iVal; break;
	case 3: m_z = iVal; break;
	}
	return( true );
}

int CPointBase::GetDist( const CPointBase & pt ) const // Distance between points
{
	// Get the basic 2d distance.
	if ( ! IsSameMapPlane( pt.m_mapplane ))	// as far apart as possible
		return( SHRT_MAX );
	return( GetDistBase( pt ));
}

int CPointBase::GetDist3D( const CPointBase & pt ) const // Distance between points
{
	// OK, 1 unit of Z is not the same (real life) distance as 1
	// unit of X (or Y)
	int dist = GetDist(pt);

	// Get the deltas and correct the Z for height first
	int dz = GetDistZAdj(pt); // Take player height into consideration
	return( max( dist, dz ));
}

DIR_TYPE CPointBase::GetDir( const CPointBase & pt, DIR_TYPE DirDefault ) const // Direction to point pt
{
	// Get the 2D direction between points.
	int dx = (m_x-pt.m_x);
	int dy = (m_y-pt.m_y);

	int ax = abs(dx);
	int ay = abs(dy);

	if ( ay > ax )
	{
		if ( ! ax )
		{
			return(( dy > 0 ) ? DIR_N : DIR_S );
		}
		int slope = ay / ax;
		if ( slope > 2 )
			return(( dy > 0 ) ? DIR_N : DIR_S );
		if ( dx > 0 )	// westish
		{
			return(( dy > 0 ) ? DIR_NW : DIR_SW );
		}
		return(( dy > 0 ) ? DIR_NE : DIR_SE );
	}
	else
	{
		if ( ! ay )
		{
			if ( ! dx )
				return( DirDefault );	// here ?
			return(( dx > 0 ) ? DIR_W : DIR_E );
		}
		int slope = ax / ay;
		if ( slope > 2 )
			return(( dx > 0 ) ? DIR_W : DIR_E );
		if ( dy > 0 )
		{
			return(( dx > 0 ) ? DIR_NW : DIR_NE );
		}
		return(( dx > 0 ) ? DIR_SW : DIR_SE );
	}
}

int CPointBase::StepLinePath( const CPointBase & ptSrc, int iSteps )
{
	// Take x steps toward this point.
	int dx = m_x - ptSrc.m_x;
	int dy = m_y - ptSrc.m_y;
	int iDist2D = GetDist( ptSrc );
	if ( ! iDist2D )
		return 0;

	m_x = ptSrc.m_x + IMULDIV( iSteps, dx, iDist2D );
	m_y = ptSrc.m_y + IMULDIV( iSteps, dy, iDist2D );
	return( iDist2D );
}

LPCTSTR CPointBase::WriteUsed( void ) const
{
	return( WriteUsed( Str_GetTemp()));
}

int CPointBase ::Read( TCHAR * pszVal )
{
	// parse reading the point
	// NOTE: do not use = as a seperator here !
	m_z = 0;
	m_mapplane = 0;
	TCHAR * ppVal[4];
	int iArgs = Str_ParseCmds( pszVal, ppVal, COUNTOF( ppVal ), " ,\t" );
	switch ( iArgs )
	{
	default:
	case 4:	// m_mapplane
		if ( isdigit(ppVal[3][0]))
		{
			m_mapplane = atoi(ppVal[3]);
		}
	case 3: // m_z
		if ( isdigit(ppVal[2][0]) || ppVal[2][0] == '-' )
		{
			m_z = atoi(ppVal[2]);
		}
	case 2:
		m_y = atoi(ppVal[1]);
	case 1:
		m_x = atoi(ppVal[0]);
	case 0:
		break;
	}
	return( iArgs );
}

void CPointBase::MoveWrap( DIR_TYPE dir )
{
	// Check for world wrap.
	// CPointBase ptOld = *this;
	Move(dir);
	if (m_y < 0)
		m_y += UO_SIZE_Y;
	if (m_x < 0)
		m_x += UO_SIZE_X_REAL;
	if (m_y >= UO_SIZE_Y)
		m_y -= UO_SIZE_Y;
	if (m_x >= UO_SIZE_X)
		m_x -= UO_SIZE_X;
	if (m_x == UO_SIZE_X_REAL)
		m_x = 0;
}

#if defined(GRAY_SVR) || defined(GRAY_MAP)	// server only.

CSector * CPointBase::GetSector() const
{
	if ( !IsValidXY() )
	{
		ASSERT(	m_x >= 0 && m_x < UO_SIZE_X );
		ASSERT(	m_y >= 0 && m_y < UO_SIZE_Y );
	}
	// Get the world Sector we are in.
	return( g_World.GetSector( (( m_y / SECTOR_SIZE_Y ) * SECTOR_COLS ) + ( m_x / SECTOR_SIZE_X ) ));
}

CRegionBase * CPointBase::GetRegion( DWORD dwType ) const
{
	// What region in the current CSector am i in ?
	// We only need to update this every 8 or so steps ?
	// REGION_TYPE_AREA

	if ( ! IsValidPoint())
	{
		return NULL;
	}

	CSector * pSector = GetSector();
	ASSERT(pSector);
	return( pSector->GetRegion( *this, dwType ));
}

int CPointBase::GetRegions( DWORD dwType, CRegionLinks & rlinks ) const
{
	if ( ! IsValidPoint())
	{
		return NULL;
	}

	CSector * pSector = GetSector();
	ASSERT(pSector);
	return( pSector->GetRegions( *this, dwType, rlinks ));
}

//*************************************************************************
// -CGRect

#if 0
int CGRect::Read( TCHAR * pszBuffer )
{
	LPCTSTR pSeps = _TEXT(" ,\t\n");
	TCHAR *pToken = strtok( pszBuffer, pSeps );
	for ( int i=0; i<4 && pToken != NULL; i++ )
	{
		int iVal = atoi( pToken );
		switch ( i )
		{
		case 0: m_left = iVal; break;
		case 1: m_top = iVal; break;
		case 2: m_right = iVal; break;
		case 3: m_bottom = iVal; break;
		}
		pToken = strtok( NULL, pSeps );
	}
	NormalizeRect();
	return( i );
}
#endif

int CGRect::Read( LPCTSTR pszVal )
{
	// parse reading the rectangle
	TCHAR *pszTemp = Str_GetTemp();
	strcpy( pszTemp, pszVal );
	TCHAR * ppVal[4];
	int i = Str_ParseCmds( pszTemp, ppVal, COUNTOF( ppVal ), _TEXT(" ,\t"));
	switch (i)
	{
	case 4: m_bottom = atoi(ppVal[3]);
	case 3: m_right = atoi(ppVal[2]);
	case 2: m_top =	atoi(ppVal[1]);
	case 1: m_left = atoi(ppVal[0]);
	}
	NormalizeRect();
	return( i );
}

LPCTSTR CGRect::Write( void ) const
{
	return( Write( Str_GetTemp()));
}

CPointBase CGRect::GetRectCorner( DIR_TYPE dir ) const
{
	// Get the point if a directional corner of the CRectMap.
	CPointBase pt;
	pt.m_z = 0;	// NOTE: remember this is a nonsense value.
	pt.m_mapplane = 0;
	switch ( dir )
	{
	case DIR_N:
		pt.m_x = ( m_left + m_right ) / 2;
		pt.m_y = m_top;
		break;
	case DIR_NE:
		pt.m_x = m_right;
		pt.m_y = m_top;
		break;
	case DIR_E:
		pt.m_x = m_right;
		pt.m_y = ( m_top + m_bottom ) / 2;
		break;
	case DIR_SE:
		pt.m_x = m_right;
		pt.m_y = m_bottom;
		break;
	case DIR_S:
		pt.m_x = ( m_left + m_right ) / 2;
		pt.m_y = m_bottom;
		break;
	case DIR_SW:
		pt.m_x = m_left;
		pt.m_y = m_bottom;
		break;
	case DIR_W:
		pt.m_x = m_left;
		pt.m_y = ( m_top + m_bottom ) / 2;
		break;
	case DIR_NW:
		pt.m_x = m_left;
		pt.m_y = m_top;
		break;
	case DIR_QTY:
		pt = GetCenter();
		break;
	}
	return( pt );
}

CSector * CGRect::GetSector( int i ) const	// ge all the sectors that make up this rect.
{
	// get all the CSector(s) that overlap this rect.
	// RETURN: NULL = no more

	// Align new rect.
	CRectMap rect;
	rect.m_left = m_left &~ (SECTOR_SIZE_X-1);
	rect.m_right = ( m_right | (SECTOR_SIZE_X-1)) + 1;
	rect.m_top = m_top &~ (SECTOR_SIZE_Y-1);
	rect.m_bottom = ( m_bottom | (SECTOR_SIZE_Y-1)) + 1;
	rect.NormalizeRectMax();

	int width = (rect.GetWidth()) / SECTOR_SIZE_X;
	ASSERT(width<=SECTOR_COLS);
	int height = (rect.GetHeight()) / SECTOR_SIZE_Y;
	ASSERT(height<=SECTOR_ROWS);

	int iBase = (( rect.m_top / SECTOR_SIZE_Y ) * SECTOR_COLS ) + ( rect.m_left / SECTOR_SIZE_X );

	if ( i >= ( height * width ))
	{
		if ( ! i )
		{
			return( g_World.GetSector(iBase) );
		}
		return( NULL );
	}

	int indexoffset = (( i / width ) * SECTOR_COLS ) + ( i % width );

	return( g_World.GetSector(iBase+indexoffset) );
}

//*************************************************************************
// -CRectMap

void CRectMap::NormalizeRect()
{
	CGRect::NormalizeRect();
	NormalizeRectMax();
}

void CRectMap::NormalizeRectMax()
{
	CGRect::NormalizeRectMax( UO_SIZE_X, UO_SIZE_Y );
}

#endif
