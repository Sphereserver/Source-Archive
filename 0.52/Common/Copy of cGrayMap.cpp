//
// CGrayMap.cpp
//

#if defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"

#elif defined(GRAY_SVR)
#include "../graysvr/graysvr.h"

#elif defined(GRAY_MAP)
#include "../graymap/graymap.h"

#elif defined(GRAY_CLIENT)
#include "../grayclient/grayclient.h"

#else
#include "graycom.h"
#include "graymul.h"
#include "cregion.h"
#include "cgraymap.h"
#endif

//////////////////////////////////////////////////////////////////
// -CGrayStaticsBlock

void CGrayStaticsBlock::LoadStatics( DWORD ulBlockIndex )
{
	// long ulBlockIndex = (bx*UO_BLOCKS_Y + by);
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
			throw CGrayError(LOGL_CRIT, E_FAIL, "CGrayMapBlock: Read fStatics0");
		}
	}
}

//////////////////////////////////////////////////////////////////
// -CGrayMapBlock

int CGrayMapBlock::sm_iCount = 0;

#ifdef GRAY_SVR

void CGrayCachedMulItem::HitCacheTime()
{
	// When in g_World.GetTime() was this last referenced.
	m_dwTimeRef = g_World.GetTime();
}

int CGrayCachedMulItem::GetCacheAge() const
{
	return( g_World.GetTime() - m_dwTimeRef );
}

#elif defined(GRAY_MAP)

void CGrayCachedMulItem::HitCacheTime()
{
	// When in g_World.GetTime() was this last referenced.
	m_dwTimeRef = GetTickCount();
}

int CGrayCachedMulItem::GetCacheAge() const
{
	return( GetTickCount() - m_dwTimeRef );
}

#endif	// GRAY_MAP

void CGrayMapBlock::Load( int bx, int by )
{
	// Read in all the statics data for this block.

#if defined(GRAY_SVR) || defined(GRAY_MAP)
	InitCacheTime();		// This is invalid !
#endif

	ASSERT( bx < UO_BLOCKS_X );
	ASSERT( by < UO_BLOCKS_Y );

	long ulBlockIndex = (bx*UO_BLOCKS_Y + by);

	CFileBin * pFile;
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

	if ( ! pFile->Seek( index.GetFileOffset()))
	{
		memset( &m_Terrain, 0, sizeof( m_Terrain ));
		throw CGrayError(LOGL_CRIT, E_FAIL, "CGrayMapBlock: Seek Ver");
	}
	if ( pFile->Read( &m_Terrain, sizeof(CUOMapBlock)) <= 0 )
	{
		memset( &m_Terrain, 0, sizeof( m_Terrain ));
		throw CGrayError(LOGL_CRIT, E_FAIL, "CGrayMapBlock: Read");
	}

	LoadStatics( ulBlockIndex );

#ifdef GRAY_MAP
	// figure out the map colors here just once when loaded.
	memset( m_MapColor, 0, sizeof(m_MapColor));
#endif

#if defined(GRAY_SVR) || defined(GRAY_MAP)
	HitCacheTime();		// validate.
#endif
}

#ifdef GRAY_MAP

COLOR_TYPE CGrayMapBlock::GetMapColorItem( WORD id ) // static
{
	// Do radar color translation on this terrain or item tile.
	// color index for ITEMID_TYPE id = TERRAIN_QTY + id
	//

	LONG lOffset = id * (LONG) sizeof(WORD);
	if ( ! g_Install.m_File[VERFILE_RADARCOL].Seek( lOffset, SEEK_SET ))
	{
		return( 0 );
	}
	WORD wColor;
	if ( g_Install.m_File[VERFILE_RADARCOL].Read( &wColor, sizeof(wColor)) != sizeof(wColor))
	{
		return( 0 );
	}
	return( wColor );
}

void CGrayMapBlock::LoadMapColors( WORD wMode )
{
	HitCacheTime();		// validate.

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
		int iQty = GetStaticQty();
		for ( int i=0; i<iQty; i++ )
		{
			const CUOStaticItemRec * pStatics = GetStatic(i);

			int n = (pStatics->m_y * UO_BLOCK_SIZE) + pStatics->m_x;
			if ( pStatics->m_z >= zsort[n] && 
				pStatics->m_z <= zclip )
			{
				zsort[n] = pStatics->m_z;
				m_MapColor[n] = TERRAIN_QTY + pStatics->m_wTileID;
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

bool CUOMapMeter::IsTerrainWater() const
{
	switch ( m_wTerrainIndex )
	{
	case TERRAIN_WATER1:
	case TERRAIN_WATER2:
	case TERRAIN_WATER3:
	case TERRAIN_WATER4:
	case TERRAIN_WATER5:
	case TERRAIN_WATER6:
		return( true );
	}
	return( false );
}

bool CUOMapMeter::IsTerrainRock() const
{
	// Is the terrain tile (not item tile) minable ?
	static const WORD Terrain_OreBase[] =
	{
		TERRAIN_ROCK1, TERRAIN_ROCK2,
		0x00ec, 0x00f7,
		0x00fc, 0x0107,
		0x010c, 0x0117,
		0x011e, 0x0129,
		0x0141, 0x0144,
		0x021f, 0x0243,
		0x024a, 0x0257,
		0x0259, 0x026d
	};

	for ( int i=0;i<COUNTOF(Terrain_OreBase);i+=2)
	{
		if ( m_wTerrainIndex >= Terrain_OreBase[i] && 
			m_wTerrainIndex <= Terrain_OreBase[i+1])
			return true;
	}
	return false;
}

bool CUOMapMeter::IsTerrainGrass() const
{
	// Is the terrain tile valid for grazing?
	// This is used with the Hunger AI
	static const WORD Terrain_GrassBase[] =
	{
		0x0003, 0x0006,
		0x007d, 0x008c,
		0x00c0, 0x00db,
		0x00f8, 0x00fb,
		0x015d, 0x0164,
		0x01a4, 0x01a7,
		0x0231, 0x0234,
		0x0239, 0x0243,
		0x0324, 0x032b,
		0x036f, 0x0376,
		0x037b, 0x037e,
		0x03bf, 0x03c6,
		0x03cb, 0x3ce,
		0x0579, 0x0580,
		0x058b, 0x058c,
		0x05e3, 0x0604,
		0x066b, 0x066e,
		0x067d, 0x0684,
		0x0695, 0x069c,
		0x06a1, 0x06c2,
		0x06d2, 0x06d9,
		0x06de, 0x06e1
	};

	for ( int i=0;i<COUNTOF(Terrain_GrassBase);i+=2)
	{
		if ( m_wTerrainIndex >= Terrain_GrassBase[i] &&
			m_wTerrainIndex <= Terrain_GrassBase[i+1] )
			return true;
	}
	return false;
}

