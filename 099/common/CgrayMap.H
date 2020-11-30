    // CgrayMap.H
    //
    
    #ifndef _INC_CGRAYMAP_H
    #define _INC_CGRAYMAP_H
    
    #if defined(GRAY_SVR) || defined(GRAY_MAP)
    
    class CGrayCachedMulItem
    {
    private:
    	CServTime m_timeRef;		// When in world.GetTime() was this last referenced.
    public:
    	void InitCacheTime()
    	{
    		m_timeRef.Init();
    	}
    	CGrayCachedMulItem()
    	{
    		InitCacheTime();
    	}
    	bool IsTimeValid() const
    	{
    		return( m_timeRef.IsTimeValid());
    	}
    	void HitCacheTime()
    	{
    		// When in g_World.GetTime() was this last referenced.
    		m_timeRef = CServTime::GetCurrentTime();
    	}
    	int GetCacheAge() const
    	{
    		// In TICK_PER_SEC or milliseconds
    		return( CServTime::GetCurrentTime() - m_timeRef );
    	}
    };
    
    #endif	// defined(GRAY_SVR) || defined(GRAY_MAP)
    
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
    		if ( m_pStatics ) delete  m_pStatics;
    	}
    	int GetStaticQty() const
    	{
    		return( m_iStatics );
    	}
    	const CUOStaticItemRec * GetStatic( int i ) const
    	{
    		ASSERT( i < m_iStatics );
    		return( &m_pStatics );
    	}
    	bool IsStaticPoint( int i, int xo, int yo ) const
    	{
    		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
    		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
    		ASSERT( i < m_iStatics );
    		return( m_pStatics.m_x == xo && m_pStatics.m_y == yo );
    	}
    };
    
    struct CGrayMapBlocker
    {
    	DWORD m_dwBlockFlags;	// How does this item block ? CAN_I_PLATFORM
    	WORD m_wTile;			// TERRAIN_QTY + id.
    	signed char m_z;		// Top of a solid object. or bottom of non blocking one.
    };
    
    struct CGrayMapBlockState
    {
    	// Go through the list of stuff at this location to decide what is  blocking us and what is not.
    	//  dwBlockFlags = what we can pass thru. doors, water, walls ?
    	//		CAN_C_GHOST	= Moves thru doors etc.	- CAN_I_DOOR = UFLAG4_DOOR						
    	//		CAN_C_SWIM = walk thru water - CAN_I_WATER = UFLAG1_WATER
    	//		CAN_C_WALK = it is possible that i can step up. - CAN_I_PLATFORM = UFLAG2_PLATFORM
    	//		CAN_C_PASSWALLS	= walk through all blcking items - CAN_I_BLOCK = UFLAG1_BLOCK
    	//		CAN_C_FLY  = gravity does not effect me. -  CAN_I_CLIMB = UFLAG2_CLIMBABLE
    	//		CAN_C_FIRE_IMMUNE = i can walk into lava etc. - CAN_I_FIRE = UFLAG1_DAMAGE
    
    	const DWORD m_dwBlockFlags;	// The block flags we can overcome.	
    	const signed char m_z;	// the z we start at. (stay at if we are flying)
    	const int m_iHeight;		// The height we need to stand here.
    
    	CGrayMapBlocker m_Top;		// What would be over my head.
    	CGrayMapBlocker m_Bottom;	// What i would be standing on.
    	CGrayMapBlocker m_Lowest;	// the lowest item we have found.	
    
    public:
    	CGrayMapBlockState( DWORD dwBlockFlags, signed char m_z, int iHeight = PLAYER_HEIGHT );
    	bool IsUsableZ( signed char zBottom, signed char zHeightEstimate ) const
    	{
    		if ( zBottom > m_Top.m_z )	// above something that is already over my head.
    			return( false );
    		// NOTE: Assume multi overlapping items are not normal. so estimates are safe
    		if ( zBottom + zHeightEstimate < m_Bottom.m_z )	// way below my feet
    			return( false );
    		return( true );	
    	}
    	bool CheckTile( DWORD dwItemBlockFlags, signed char zBottom, signed char zheight, WORD wID );
    	static LPCTSTR GetTileName( WORD wID );
    };
    
    class CGrayMapBlock : public CMemDynamic,	// Cache this from the MUL files. 8x8 block of the world.
    	public CPointSort	// The upper left corner. (ignore z) sort by this
    {
    protected:
    	DECLARE_MEM_DYNAMIC;
    private:
    	static int sm_iCount;	// count number of loaded blocks.
    
    	CUOMapBlock m_Terrain;
    
    #if defined(GRAY_MAP)
    	WORD m_wMapColorMode;
    	COLOR_TYPE m_MapColor;
    #endif
    
    public:
    	CGrayStaticsBlock m_Statics;
    #if defined(GRAY_SVR) || defined(GRAY_MAP)
    	CGrayCachedMulItem m_CacheTime;	// keep track of the use time of this item. (client does not care about this)
    #endif
    
    private:
    	// NOTE: This will "throw" on failure !
    	void Load( int bx, int by );
    
    public:
    	CGrayMapBlock( const CPointMap & pt ) :
    		CPointSort( pt )	// The upper left corner.
    	{
    		sm_iCount++;
    		ASSERT( ! UO_BLOCK_OFFSET(pt.m_x));
    		ASSERT( ! UO_BLOCK_OFFSET(pt.m_y));
    		Load( pt.m_x/UO_BLOCK_SIZE, pt.m_y/UO_BLOCK_SIZE );
    	}
    
    	CGrayMapBlock( int bx, int by ) :
    		CPointSort( bx * UO_BLOCK_SIZE, by * UO_BLOCK_SIZE )
    	{
    		sm_iCount++;
    		Load( bx, by );
    	}
    
    	virtual ~CGrayMapBlock()
    	{
    		sm_iCount--;
    	}
    
    	int GetOffsetX( int x ) const
    	{
    		// Allow this to go out of bounds.
    		// ASSERT( ( x-m_pt.m_x) == UO_BLOCK_OFFSET(x));
    		return( x - m_x );
    	}
    	int GetOffsetY( int y ) const
    	{
    		return( y - m_y );
    	}
    
    	const CUOMapMeter * GetTerrain( int xo, int yo ) const
    	{
    		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
    		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
    		return( &( m_Terrain.m_Meter ));
    	}
    	const CUOMapBlock * GetTerrainBlock() const
    	{
    		return( &m_Terrain );
    	}
    
    	static COLOR_TYPE GetMapColorItem( WORD idTile );
    #if defined(GRAY_MAP)
    	static WORD GetMapColorMode( bool fMap, bool fStatics, bool fColor, signed char zclip );
    	COLOR_TYPE GetMapColor( int xo, int yo ) const
    	{
    		ASSERT( xo >= 0 && xo < UO_BLOCK_SIZE );
    		ASSERT( yo >= 0 && yo < UO_BLOCK_SIZE );
    		return( m_MapColor );
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
    			delete  m_pItems;
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
    	virtual ~CGrayMulti()
    	{
    		Release();
    	}
    };
    
#endif // _INC_CGRAYMAP_H': command not understood.