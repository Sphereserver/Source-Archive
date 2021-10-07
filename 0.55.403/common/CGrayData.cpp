//
// CGrayData.Cpp
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#elif defined(GRAY_CLIENT)
#include "../grayclient/grayclient.h"
#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"
#elif defined(GRAY_PATCH)
#include "../graypatch/stdafx.h"
#include "../graypatch/graypatch.h"
#else
#include "graycom.h"
#include "graymul.h"
#include "CRegion.h"
#include "CGrayMap.h"
#endif

//////////////////////////////////////////////////////////////////////
// -CVerDataMul

#if 0

int CVerDataMul::QCompare( void * pLeft, void * pRight ) // static
{
	const CUOVersionBlock* pVerRight = (const CUOVersionBlock*) pRight;
	const CUOVersionBlock* pVerLeft = (const CUOVersionBlock*) pLeft;
	ASSERT(pVerRight!=pVerLeft);
	DWORD dwValLeft = pVerLeft->GetIndex();
	DWORD dwValRight = pVerRight->GetIndex();
	if ( dwValLeft > dwValRight )
		return( 1 );
	if ( dwValLeft==dwValRight)
		return( 0 );
	return( -1 );
}

#else

int CVerDataMul::QCompare( int left, DWORD dwRefIndex ) const
{
	DWORD dwIndex2 = GetEntry(left)->GetIndex();
	return( dwIndex2 - dwRefIndex );
}

void CVerDataMul::QSort( int left, int right )
{
	static int iReentrant=0;
	ASSERT( left < right );
	int j = left;
	int i = right;

	DWORD dwRefIndex = GetEntry( (left+right) / 2 )->GetIndex();

	do
	{
		while ( QCompare( j, dwRefIndex ) < 0 ) j++;
		while ( QCompare( i, dwRefIndex ) > 0 ) i--;

		if ( i >= j )
		{
			if ( i != j )
			{
				CUOVersionBlock Tmp = m_Data.GetAt(i);
				CUOVersionBlock block = m_Data.GetAt(j);
				m_Data.SetAt( i, block );
				m_Data.SetAt( j, Tmp );
			}
			i--;
			j++;
		}

	} while (j <= i);

	iReentrant++;
	if (left < i)  QSort(left,i);
	if (j < right) QSort(j,right);
	iReentrant--;
}

#endif

void CVerDataMul::Load( CGFile & file )
{
	// assume it is in sorted order.
	if ( GetCount())	// already loaded.
	{
		return;
	}

// #define g_fVerData g_Install.m_File[VERFILE_VERDATA]

	if ( ! file.IsFileOpen())		// T2a might not have this.
		return;

	file.SeekToBegin();
	DWORD dwQty;
	if ( file.Read( (void *) &dwQty, sizeof(dwQty)) <= 0 )
	{
		throw CGrayError( LOGL_CRIT, CGFile::GetLastError(), "VerData: Read Qty");
	}

	Unload();
	m_Data.SetCount( dwQty );

	if ( file.Read( (void *) m_Data.GetBasePtr(), dwQty * sizeof( CUOVersionBlock )) <= 0 )
	{
		throw CGrayError( LOGL_CRIT, CGFile::GetLastError(), "VerData: Read");
	}

	if ( ! dwQty )
		return;

	// Now sort it for fast searching.
	// Make sure it is sorted.
#if 0
	CQSortArray array( m_Data.GetBasePtr(), dwQty, sizeof(CUOVersionBlock), QCompare );
	array.QSort();
#else
	QSort( 0, dwQty-1 );
#endif

#ifdef _DEBUG
	for ( DWORD i=0; i<dwQty-1; i++ )
	{
		DWORD dwIndex1 = GetEntry(i)->GetIndex();
		DWORD dwIndex2 = GetEntry(i+1)->GetIndex();
		if ( dwIndex1 > dwIndex2 )
		{
			DEBUG_ERR(( "VerData Array is NOT sorted !\n" ));
			throw CGrayError( LOGL_CRIT, -1, "VerData: NOT Sorted!");
			break;
		}
	}
#endif

}

bool CVerDataMul::FindVerDataBlock( VERFILE_TYPE type, DWORD id, CUOIndexRec & Index ) const
{
	// Search the verdata.mul for changes to stuff.
	// search for a specific block.
	// assume it is in sorted order.

	int iHigh = GetCount()-1;
	if ( iHigh < 0 )
	{
		return( false );
	}

	DWORD dwIndex = VERDATA_MAKE_INDEX(type,id);
	const CUOVersionBlock *pArray = (const CUOVersionBlock *) m_Data.GetBasePtr();
	int iLow = 0;
	while ( iLow <= iHigh )
	{
		int i = (iHigh+iLow)/2;
		DWORD dwIndex2 = pArray[i].GetIndex();
		int iCompare = dwIndex - dwIndex2;
		if ( iCompare == 0 )
		{
			Index.CopyIndex( &(pArray[i]));
			return( true );
		}
		if ( iCompare > 0 )
		{
			iLow = i+1;
		}
		else
		{
			iHigh = i-1;
		}
	}
	return( false );
}

//*********************************************8
// -CGrayItemInfo

CGrayItemInfo::CGrayItemInfo( ITEMID_TYPE id )
{
	if ( id >= ITEMID_MULTI )
	{
		// composite/multi type objects.
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

	if ( g_Install.m_File[filedata].Seek( offset, SEEK_SET ) != offset )
	{
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CTileItemType.ReadInfo: TileData Seek");
	}

	if ( g_Install.m_File[filedata].Read( static_cast <CUOItemTypeRec *>(this), sizeof(CUOItemTypeRec)) <= 0 )
	{
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CTileItemType.ReadInfo: TileData Read");
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

	if ( g_Install.m_File[filedata].Seek( offset, SEEK_SET ) != offset )
	{
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CTileTerrainType.ReadInfo: TileData Seek");
	}

	if ( g_Install.m_File[filedata].Read(static_cast <CUOTerrainTypeRec *>(this), sizeof(CUOTerrainTypeRec)) <= 0 )
	{
		throw CGrayError(LOGL_CRIT, CGFile::GetLastError(), "CTileTerrainType.ReadInfo: TileData Read");
	}
}

#ifndef GRAY_AGENT

int CGrayMulti::Load( MULTI_TYPE id )
{
	// Just load the whole thing.

	Release();

#ifdef GRAY_SVR
	InitCacheTime();		// This is invalid !
#endif

	if ( id < 0 || id > MULTI_QTY )
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

#endif
