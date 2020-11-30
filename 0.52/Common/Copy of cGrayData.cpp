//
// CGrayData.Cpp
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/graymap.h"
#elif defined(GRAY_CLIENT)
#include "../grayclient/grayclient.h"
#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#else
#include "graycom.h"
#include "graymul.h"
#include "cregion.h"
#include "cgraymap.h"
#endif

CGrayItemInfo::CGrayItemInfo( ITEMID_TYPE id )
{
	if ( id >= ITEMID_MULTI )
	{
		// composite type objects.
		m_flags = 0;
		m_weight = 0xFF;
		m_layer = LAYER_NONE;
		m_dwAnim = 0;
		m_height = 0;
		strcpy( m_name, ( id <= ITEMID_SHIP6_W ) ? "ship" : "structure" );
		return;
	}

	VERFILE_TYPE filedata;
	long offset;
	CUOIndexRec Index;
	if ( g_VerData.FindVerDataBlock( VERFILE_TILEDATA, (id+TERRAIN_QTY)/UOTILE_BLOCK_QTY, Index ))
	{
		filedata = VERFILE_VERDATA;
		offset = Index.GetFileOffset() + 4 + (sizeof(CUOItemTypeRec)*(id%UOTILE_BLOCK_QTY));
		ASSERT( Index.GetBlockLength() >= sizeof( CUOItemTypeRec ));
	}
	else
	{
		filedata = VERFILE_TILEDATA;
		offset = UOTILE_TERRAIN_SIZE + 4 + (( id / UOTILE_BLOCK_QTY ) * 4 ) + ( id * sizeof( CUOItemTypeRec ));
	}

	if ( ! g_Install.m_File[filedata].Seek( offset, SEEK_SET ))
	{
		throw CGrayError(LOGL_CRIT, E_FAIL, "CTileItemType.ReadInfo: TileData Seek");
	}

	if ( g_Install.m_File[filedata].Read( static_cast <CUOItemTypeRec *>(this), sizeof(CUOItemTypeRec)) <= 0 )
	{
		throw CGrayError(LOGL_CRIT, E_FAIL, "CTileItemType.ReadInfo: TileData Read");
	}
}

CGrayTerrainInfo::CGrayTerrainInfo( TERRAIN_TYPE id )
{
	ASSERT( id < TERRAIN_QTY );

	VERFILE_TYPE filedata;
	long offset;
	CUOIndexRec Index;
	if ( g_VerData.FindVerDataBlock( VERFILE_TILEDATA, id/UOTILE_BLOCK_QTY, Index ))
	{
		filedata = VERFILE_VERDATA;
		offset = Index.GetFileOffset() + 4 + (sizeof(CUOTerrainTypeRec)*(id%UOTILE_BLOCK_QTY));
		ASSERT( Index.GetBlockLength() >= sizeof( CUOTerrainTypeRec ));
	}
	else
	{
		filedata = VERFILE_TILEDATA;
		offset = 4 + (( id / UOTILE_BLOCK_QTY ) * 4 ) + ( id * sizeof( CUOTerrainTypeRec ));
	}

	if ( ! g_Install.m_File[filedata].Seek( offset, SEEK_SET ))
	{
		throw CGrayError(LOGL_CRIT, E_FAIL, "CTileTerrainType.ReadInfo: TileData Seek");
	}

	if ( g_Install.m_File[filedata].Read(static_cast <CUOTerrainTypeRec *>(this), sizeof(CUOTerrainTypeRec)) <= 0 )
	{
		throw CGrayError(LOGL_CRIT, E_FAIL, "CTileTerrainType.ReadInfo: TileData Read");
	}
}

int CGrayMulti::Load( MULTI_TYPE id )
{
	// Just load the whole thing.
	
	Release();

#ifdef GRAY_SVR
	InitCacheTime();		// This is invalid !
#endif

	if ( ( id < 0 || id > MULTI_QTY ) && ( id != ITEMID_MULTI_EXT_1 && id != ITEMID_MULTI_EXT_2) )
		return( 0 );
	m_id = id;

	CUOIndexRec Index;
	if ( ! g_Install.ReadMulIndex( VERFILE_MULTIIDX, VERFILE_MULTI, id, Index ))
		return( 0 );

	m_iItemQty = Index.GetBlockLength() / sizeof(CUOMultiItemRec);
	m_pItems = new CUOMultiItemRec [ m_iItemQty ];
	ASSERT( m_pItems );

	if ( ! g_Install.ReadMulData( VERFILE_MULTI, Index, (void*) m_pItems ))
	{
		return( 0 );
	}

#ifdef GRAY_SVR
	HitCacheTime();
#endif

	return( m_iItemQty );
}


