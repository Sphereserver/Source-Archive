//
// CgrayMap.H
//

#ifndef _INC_CGRAYMAP_H
#define _INC_CGRAYMAP_H

class CGrayCachedMulItem
{
private:
	time_t m_dwTimeRef;		// When in world.GetTime() was this last referenced.
public:
	void InitCacheTime()
	{
		m_dwTimeRef = 0;
	}
	CGrayCachedMulItem()
	{
		InitCacheTime();
	}
	bool IsValid() const
	{
		return( m_dwTimeRef ? true : false );
	}
	void HitCacheTime();
	int GetCacheAge() const;
};

class CGrayStaticsBlock
{
private:
	int m_iStatics;
	CUOStaticItemRec * m_pStatics;	// dyn alloc array block.
public:
	void LoadStatics( DWORD dwBlockIndex );
public:
	CGrayStaticsBlock()
	{
		m_iStatics = 0;
		m_pStatics = NULL;
	}
	~CGrayStaticsBlock()
	{
		if ( m_pStatics ) delete [] m_pStatics;
	}
	int GetStaticQty() const
	{
		return( m_iStatics );
	}
	const CUOStaticItemRec * GetStatic( int i ) const
	{
		ASSERT( i < m_iStatics );
		return( &m_pStatics[i] );
	}
	bool IsStaticPoint( int i, int xo, int yo ) const
	{
		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
		ASSERT( i < m_iStatics );
		return( m_pStatics[i].m_x == xo && m_pStatics[i].m_y == yo );
	}
};

class CGrayMapBlock : public CMemDynamic,	// Cache this from the MUL files. 8x8 block of the world.
	public CGrayStaticsBlock
#if defined(GRAY_SVR) || defined(GRAY_MAP)
	, public CGrayCachedMulItem
#endif
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	static int sm_iCount;	// count number of loaded blocks.

	CUOMapBlock m_Terrain;

#ifdef GRAY_MAP
	WORD m_wMapColorMode;
	WORD m_MapColor[UO_BLOCK_SIZE*UO_BLOCK_SIZE];
#endif
	
public:
	const CPointMap m_p;	// The upper left corner. (ignore z)

private:
	// NOTE: This will "throw" on failure !
	void Load( int bx, int by );

public:
	CGrayMapBlock( const CPointMap & pt ) :
		m_p( pt )	// The upper left corner.
	{
		sm_iCount++;
		ASSERT( ! UO_BLOCK_OFFSET(pt.m_x));
		ASSERT( ! UO_BLOCK_OFFSET(pt.m_y));
		Load( pt.m_x/UO_BLOCK_SIZE, pt.m_y/UO_BLOCK_SIZE );
	}

	CGrayMapBlock( int bx, int by ) :
		m_p( bx * UO_BLOCK_SIZE, by * UO_BLOCK_SIZE )
	{
		sm_iCount++;
		Load( bx, by );
	}

	~CGrayMapBlock()
	{
		sm_iCount--;
	}

	int GetOffsetX( int x ) const
	{
		// Allow this to go out of bounds.
		// ASSERT( ( x-m_p.m_x) == UO_BLOCK_OFFSET(x));
		return( x - m_p.m_x );
	}
	int GetOffsetY( int y ) const
	{
		return( y - m_p.m_y );
	}

	const CUOMapMeter * GetTerrain( int xo, int yo ) const
	{
		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
		return( &( m_Terrain.m_Meter[ yo*UO_BLOCK_SIZE + xo ] ));
	}
	const CUOMapBlock * GetTerrainBlock() const
	{
		return( &m_Terrain );
	}

#ifdef GRAY_MAP
	static COLOR_TYPE GetMapColorItem( WORD id );
	static WORD GetMapColorMode( bool fMap, bool fStatics, bool fColor, signed char zclip );

	WORD GetMapColor( int xo, int yo ) const
	{
		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
		return( m_MapColor[(yo*UO_BLOCK_SIZE) + xo] );
	}
	void UnloadMapColors()
	{
		m_wMapColorMode = 0;
	}
	void LoadMapColors( WORD wMode );
#endif

};

#ifdef GRAY_MAP
inline WORD CGrayMapBlock::GetMapColorMode( bool fMap, bool fStatics, bool fColor, signed char zclip )
{
	WORD wMapColorMode = (BYTE) zclip;
	if ( fMap ) wMapColorMode |= 0x8000;
	if ( fStatics ) wMapColorMode |= 0x4000;
	if ( fColor ) wMapColorMode |= 0x2000;
	return( wMapColorMode );
}
#endif

struct CGrayItemInfo:public CUOItemTypeRec
{
	CGrayItemInfo( ITEMID_TYPE id );
};

struct CGrayTerrainInfo:public CUOTerrainTypeRec
{
	CGrayTerrainInfo( TERRAIN_TYPE id );
};

class CGrayMulti
#ifdef GRAY_SVR
	: public CGrayCachedMulItem
#endif
{
	// Load all the relivant info for the 
private:
	MULTI_TYPE m_id;
	CUOMultiItemRec * m_pItems;
	int m_iItemQty;
private:
	void Init()
	{
		m_id = MULTI_QTY;
		m_pItems = NULL;
		m_iItemQty = 0;
	}
	void Release()
	{
		if ( m_pItems )
		{
			delete [] m_pItems;
			Init();
		}
	}
public:
	int Load( MULTI_TYPE id );
	CGrayMulti()
	{
		Init();
	}
	CGrayMulti( MULTI_TYPE id )
	{
		Init();
		Load( id );
	}
	MULTI_TYPE GetMultiID() const
	{
		return( m_id );
	}
	int GetItemCount() const
	{
		return( m_iItemQty );
	}
	const CUOMultiItemRec * GetItem( int i ) const
	{
		ASSERT( i<m_iItemQty );
		return( m_pItems+i);
	}
	~CGrayMulti()
	{
		Release();
	}
};

#endif // _INC_CGRAYMAP_H