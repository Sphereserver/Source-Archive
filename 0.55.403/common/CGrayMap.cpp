//
// CGrayMap.cpp
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"

#elif defined(GRAY_SVR)
#include "../graysvr/graysvr.h"

#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"

#elif defined(GRAY_CLIENT)
#include "../grayclient/grayclient.h"

#elif defined(GRAY_PATCH)
#include "../graypatch/stdafx.h"
#include "../graypatch/graypatch.h"

#else
#include "graycom.h"
#include "graymul.h"
#include "CRegion.h"
#include "CGrayMap.h"
#endif

//////////////////////////////////////////////////////////////////
// -CGrayMapBlockState

#ifndef CAN_I_BLOCK
#define CAN_I_BLOCK		UFLAG1_BLOCK
#define CAN_I_PLATFORM	UFLAG2_PLATFORM
#define CAN_I_CLIMB		UFLAG2_CLIMBABLE
#define CAN_I_DOOR		UFLAG4_DOOR
#endif

CGrayMapBlockState::CGrayMapBlockState( DWORD dwBlockFlags, signed char z, int iHeight ) :
	m_dwBlockFlags(dwBlockFlags),	m_z(z), m_iHeight(iHeight), m_zClimb(0)
{
	// m_z = PLAYER_HEIGHT
	m_Top.m_dwBlockFlags = 0;
	m_Top.m_wTile = 0;
	m_Top.m_z = UO_SIZE_Z;	// the z of the item that would be over our head.

	m_Bottom.m_dwBlockFlags = CAN_I_BLOCK; // The bottom item has these blocking flags.
	m_Bottom.m_wTile = 0;
	m_Bottom.m_z = UO_SIZE_MIN_Z;	// the z we would stand on,

	m_Lowest.m_dwBlockFlags = CAN_I_BLOCK; 
	m_Lowest.m_wTile = 0;
	m_Lowest.m_z = UO_SIZE_Z;

	m_zClimbHeight = 0;
}

CGrayMapBlockState::CGrayMapBlockState( DWORD dwBlockFlags, signed char z, int iHeight, signed char zClimb ) :
	m_dwBlockFlags(dwBlockFlags),	m_z(z), m_iHeight(iHeight), m_zClimb(zClimb)
{
	m_Top.m_dwBlockFlags = 0;
	m_Top.m_wTile = 0;
	m_Top.m_z = UO_SIZE_Z;	// the z of the item that would be over our head.

	m_Bottom.m_dwBlockFlags = CAN_I_BLOCK; // The bottom item has these blocking flags.
	m_Bottom.m_wTile = 0;
	m_Bottom.m_z = UO_SIZE_MIN_Z;	// the z we would stand on,

	m_Lowest.m_dwBlockFlags = CAN_I_BLOCK; 
	m_Lowest.m_wTile = 0;
	m_Lowest.m_z = UO_SIZE_Z;

	m_zClimbHeight = 0;
}

LPCTSTR CGrayMapBlockState::GetTileName( WORD wID )	// static
{
	if ( wID == 0 )
	{
		return( "<null>" );
	}
	TCHAR * pStr = Str_GetTemp();
	if ( wID < TERRAIN_QTY )
	{
		CGrayTerrainInfo land( wID );
		strcpy( pStr, land.m_name );
	}
	else
	{
		wID -= TERRAIN_QTY;
		CGrayItemInfo item( (ITEMID_TYPE) wID );
		strcpy( pStr, item.m_name );
	}
	return( pStr );
}

bool CGrayMapBlockState::CheckTile( DWORD wItemBlockFlags, signed char zBottom, signed char zHeight, WORD wID )
{
	// RETURN:
	//  true = continue processing

	signed char zTop = zBottom;
	if ( wItemBlockFlags & CAN_I_CLIMB )
		zTop += ( zHeight / 2 );	// standing position is half way up climbable items.
	else
		zTop += zHeight;

	if ( zTop < m_Bottom.m_z )	// below something i can already step on.
		return true;

	if ( ! wItemBlockFlags )	// no effect.
		return( true );

	// If this item does not block me at all then i guess it just does not count.
	if ( ! ( wItemBlockFlags &~ m_dwBlockFlags ))
	{	// this does not block me.
		if ( ! ( wItemBlockFlags & CAN_I_CLIMB ))
		{
			if ( wItemBlockFlags & CAN_I_PLATFORM )	// i can always walk on the platform.
			{
				zBottom = zTop;	// or i could walk under it.
			}
			else
			{
				zTop = zBottom;
			}
			// zHeight = 0;	// no effective height.
		}
	}

	if ( zTop < m_Lowest.m_z )
	{
		m_Lowest.m_dwBlockFlags = wItemBlockFlags;
		m_Lowest.m_wTile = wID;
		m_Lowest.m_z = zTop;
	}

	// if i can't fit under this anyhow. it is something below me. (potentially)
	if ( zBottom < ( m_z + m_iHeight/2 ))
	{
		// This is the new item ( that would be ) under me.
		// NOTE: Platform items should take precendence over non-platforms.
		if ( zTop >= m_Bottom.m_z )
		{
			if ( zTop == m_Bottom.m_z )
			{
				if ( m_Bottom.m_dwBlockFlags & CAN_I_PLATFORM )
					return( true );
			}
			m_Bottom.m_dwBlockFlags = wItemBlockFlags;
			m_Bottom.m_wTile = wID;
			m_Bottom.m_z = zTop;
		}
	}
	else
	{
		// I could potentially fit under this. ( it would be above me )
		if ( zBottom <= m_Top.m_z )
		{
			m_Top.m_dwBlockFlags = wItemBlockFlags;
			m_Top.m_wTile = wID;
			m_Top.m_z = zBottom;
		}
	}

	return true;
}

bool CGrayMapBlockState::CheckTile_Item( DWORD wItemBlockFlags, signed char zBottom, signed char zHeight, WORD wID )
{
	// RETURN:
	//  true = continue processing

	if ( ! wItemBlockFlags )	// no effect.
		return( true );

	signed char zTop = zBottom;
	if ( wItemBlockFlags & CAN_I_CLIMB )
		zTop += ( zHeight / 2 );
	else
		zTop += zHeight;
	if ( zTop < m_Bottom.m_z )	// below something i can already step on.
		return true;

	if ( zTop < m_Lowest.m_z )
	{
		m_Lowest.m_dwBlockFlags = wItemBlockFlags;
		m_Lowest.m_wTile = wID;
		m_Lowest.m_z = zTop;
	}

	if ( ! ( wItemBlockFlags &~ m_dwBlockFlags ))
	{	// this does not block me.
		if ( ! ( wItemBlockFlags & ( CAN_I_CLIMB | CAN_I_PLATFORM ) ) )
		{
			return true;
		}
	}

	if ( ( zBottom <= m_zClimb ) )
	{
		if ( zTop >= m_Bottom.m_z )
		{
			if ( zTop == m_Bottom.m_z )
			{
				if ( ! ( wItemBlockFlags & CAN_I_CLIMB ) ) // climbable items have the highest priority
					if ( m_Bottom.m_dwBlockFlags & CAN_I_PLATFORM ) //than items with CAN_I_PLATFORM
						return ( true );
			}
			m_Bottom.m_dwBlockFlags = wItemBlockFlags;
			m_Bottom.m_wTile = wID;
			m_Bottom.m_z = zTop;

			if ( wItemBlockFlags & CAN_I_CLIMB ) // return climb height
				m_zClimbHeight = ( zHeight / 2 );
			else
				m_zClimbHeight = 0;
		}
	}
	else
	{
		// I could potentially fit under this. ( it would be above me )
		if ( zBottom < m_Top.m_z )
		{
			m_Top.m_dwBlockFlags = wItemBlockFlags;
			m_Top.m_wTile = wID;
			m_Top.m_z = zBottom;
		}
	}
	return true;

}

bool CGrayMapBlockState::CheckTile_Terrain( DWORD wItemBlockFlags, signed char z, WORD wID )
{
	// RETURN:
	//  true = continue processing

	if ( ! wItemBlockFlags )	// no effect.
		return( true );

	if ( z < m_Bottom.m_z )	// below something i can already step on.
		return true;

	if ( z < m_Lowest.m_z )
	{
		m_Lowest.m_dwBlockFlags = wItemBlockFlags;
		m_Lowest.m_wTile = wID;
		m_Lowest.m_z = z;
	}

	if	(  z <= m_iHeight )
	{
		if ( z >= m_Bottom.m_z )
		{
			if ( z == m_Bottom.m_z )
			{
				if ( m_Bottom.m_dwBlockFlags & CAN_I_CLIMB ) // climbable items have the highest priority
					return ( true );
				if ( ( m_Bottom.m_dwBlockFlags & CAN_I_PLATFORM ) && (!( wItemBlockFlags & CAN_I_PLATFORM )) )
					return ( true );
			} else if ( z > m_z + PLAYER_HEIGHT/2 ) 
				if ( (m_Bottom.m_dwBlockFlags & (CAN_I_PLATFORM|CAN_I_CLIMB)) && (z >= m_Bottom.m_z + PLAYER_HEIGHT/2) ) // we can walk under it
					goto settop;

			m_Bottom.m_dwBlockFlags = wItemBlockFlags;
			m_Bottom.m_wTile = wID;
			m_Bottom.m_z = z;
			m_zClimbHeight = 0;
		}
	}
	else
	{
		// I could potentially fit under this. ( it would be above me )
settop:
		if ( z < m_Top.m_z )
		{
			m_Top.m_dwBlockFlags = wItemBlockFlags;
			m_Top.m_wTile = wID;
			m_Top.m_z = z;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////
// -CGrayStaticsBlock

void CGrayStaticsBlock::LoadStatics( DWORD ulBlockIndex )
{
	// long ulBlockIndex = (bx*(UO_SIZE_Y/UO_BLOCK_SIZE) + by);
	// NOTE: What is index.m_wVal3 and index.m_wVal4 in VERFILE_STAIDX ?
	ASSERT( m_iStatics <= 0 );

	CUOIndexRec index;
	if ( g_Install.ReadMulIndex( VERFILE_STAIDX, VERFILE_STATICS, ulBlockIndex, index ))
	{
		m_iStatics = index.GetBlockLength() / sizeof( CUOStaticItemRec );
		ASSERT( m_iStatics );
		m_pStatics = new CUOStaticItemRec[ m_iStatics ];
		ASSERT( m_pStatics );
		if ( ! g_Install.ReadMulData( VERFILE_STATICS, index, m_pStatics ))
		{
			throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CGrayMapBlock: Read fStatics0");
		}
	}
}

//////////////////////////////////////////////////////////////////
// -CGrayMapBlock

int CGrayMapBlock::sm_iCount = 0;

void CGrayMapBlock::Load( int bx, int by )
{
	// Read in all the statics data for this block.

#if defined(GRAY_SVR) || defined(GRAY_MAP)
	m_CacheTime.InitCacheTime();		// This is invalid !
#endif

	ASSERT( bx < (UO_SIZE_X/UO_BLOCK_SIZE) );
	ASSERT( by < (UO_SIZE_Y/UO_BLOCK_SIZE) );

	long ulBlockIndex = (bx*(UO_SIZE_Y/UO_BLOCK_SIZE) + by);

	CGFile * pFile;
	CUOIndexRec index;
	if ( g_VerData.FindVerDataBlock( VERFILE_MAP, ulBlockIndex, index ))
	{
		pFile = &(g_Install.m_File[VERFILE_VERDATA]);
	}
	else
	{
		index.SetupIndex( ulBlockIndex * sizeof(CUOMapBlock), sizeof(CUOMapBlock));
		pFile = &(g_Install.m_File[VERFILE_MAP]);
	}

	if ( pFile->Seek( index.GetFileOffset(), SEEK_SET ) != index.GetFileOffset() )
	{
		memset( &m_Terrain, 0, sizeof( m_Terrain ));
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CGrayMapBlock: Seek Ver");
	}
	if ( pFile->Read( &m_Terrain, sizeof(CUOMapBlock)) <= 0 )
	{
		memset( &m_Terrain, 0, sizeof( m_Terrain ));
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CGrayMapBlock: Read");
	}

	m_Statics.LoadStatics( ulBlockIndex );

#ifdef GRAY_MAP
	// figure out the map colors here just once when loaded.
	memset( m_MapColor, 0, sizeof(m_MapColor));
#endif

#if defined(GRAY_SVR) || defined(GRAY_MAP)
	m_CacheTime.HitCacheTime();		// validate.
#endif
}

COLOR_TYPE CGrayMapBlock::GetMapColorItem( WORD idTile ) // static
{
	// ??? Cache this !
	// Do radar color translation on this terrain or item tile.
	// NOTE: this is HOrridly SLOW !
	// Do radar color translation on this terrain or item tile.
	// color index for ITEMID_TYPE id = TERRAIN_QTY + id
	//

	LONG lOffset = idTile * (LONG) sizeof(WORD);
	if ( g_Install.m_File[VERFILE_RADARCOL].Seek( lOffset, SEEK_SET ) != lOffset )
	{
		return( 0 );
	}
	COLOR_TYPE wColor;
	if ( g_Install.m_File[VERFILE_RADARCOL].Read( &wColor, sizeof(wColor)) != sizeof(wColor))
	{
		return( 0 );
	}
	return( wColor );
}

#if defined(GRAY_MAP)

void CGrayMapBlock::LoadMapColors( WORD wMode )
{
	m_CacheTime.HitCacheTime();		// validate.

	if ( m_wMapColorMode == wMode )	// we already have a valid color map here.
		return;
	m_wMapColorMode = wMode;

	signed char zclip = (signed char)(BYTE)(wMode);
	signed char zsort[UO_BLOCK_SIZE*UO_BLOCK_SIZE];

	if ( wMode & 0x8000 )	// map terrain base.
	{
		int n=0;
		for ( int y2=0; y2<UO_BLOCK_SIZE; y2++ )
		{
			for ( int x2=0; x2<UO_BLOCK_SIZE; x2++, n++ )
			{
				const CUOMapMeter * pMapMeter = GetTerrain(x2,y2);

				// Get the base map blocks.
				int z = pMapMeter->m_z;
				if ( z <= zclip )
				{
					zsort[n] = z;
					m_MapColor[n] = pMapMeter->m_wTerrainIndex;
				}
				else
				{
					zsort[n] = UO_SIZE_MIN_Z;
					m_MapColor[n] = 0;
				}
			}
		}
	}
	else
	{
		for ( int n=0; n<UO_BLOCK_SIZE*UO_BLOCK_SIZE; n++ )
		{
			zsort[n] = UO_SIZE_MIN_Z;
			m_MapColor[n] = 0;
		}
	}

	if ( wMode & 0x4000 )	// statics
	{
		int iQty = m_Statics.GetStaticQty();
		for ( int i=0; i<iQty; i++ )
		{
			const CUOStaticItemRec * pStatics = m_Statics.GetStatic(i);

			int n = (pStatics->m_y * UO_BLOCK_SIZE) + pStatics->m_x;
			if ( pStatics->m_z >= zsort[n] &&
				pStatics->m_z <= zclip )
			{
				zsort[n] = pStatics->m_z;
				m_MapColor[n] = TERRAIN_QTY + pStatics->GetDispID();
			}
		}
	}

	if ( wMode & 0x2000 )	// map color translation
	{
		// Do the color translation for the block.
		for ( int n=0; n<UO_BLOCK_SIZE*UO_BLOCK_SIZE; n++ )
		{
			m_MapColor[n] = GetMapColorItem( m_MapColor[n] );
		}
	}
}

#endif	// GRAY_MAP
