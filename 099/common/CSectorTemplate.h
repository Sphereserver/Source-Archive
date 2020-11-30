    // CSectorTemplate.h
    //
    #ifndef _INC_CSECTOR_H
    #define _INC_CSECTOR_H
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    class CItemsDisconnectList : public CGObList
    {
    	// Use this just for validation purposes.
    };
    
    class CCharsDisconnectList : public CGObList
    {
    	// Use this just for validation purposes.
    };
    
    class CCharsActiveList : public CGObList
    {
    #ifdef GRAY_SVR
    private:
    	int	   m_iClients;			// How many clients in this sector now?
    public:
    	DWORD  m_dwMapPlaneClients;	// What planes are we active on.
    	CServTime m_timeLastClient;	// age the sector based on last client here.
    protected:
    	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
    public:
    	int HasClients() const { return( m_iClients ); }
    	void ClientAttach()
    	{
    		m_iClients++;
    	}
    	void ClientDetach()
    	{
    		DEBUG_CHECK(m_iClients>0);
    		m_iClients--;
    	}
    	void SetWakeStatus( int iMapPlane );
    	bool IsAwakePlane( int iMapPlane ) const
    	{
    		return( m_dwMapPlaneClients & ( 1 << ( iMapPlane & 0x3f )));
    	}
    	void AddCharToSector( CChar * pChar );
    	CCharsActiveList()
    	{
    		m_timeLastClient.Init();
    		m_iClients = 0;
    		m_dwMapPlaneClients = 0;
    	}
    #endif
    };
    
    class CItemsList : public CGObList
    {
    #ifdef GRAY_SVR
    	// Top level list of items.
    	// ??? Sort by point ?
    public:
    	static bool sm_fNotAMove;	// hack flag to prevent items from bouncing around too much.
    protected:
    	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
    public:
    	void AddItemToSector( CItem * pItem );
    #endif
    };
    
    class CObPointSortArray : public CGObSortArray< CPointSort*, long >
    {
    	// Find a point fast.
    	int CompareKey( long id, CPointSort* pBase ) const
    	{
    		ASSERT( pBase );
    		return( id - pBase->GetPointSortIndex());
    	}
    };
    
    class CSectorTemplate	// square region of the world.
    {
    private:
    	CObPointSortArray m_MapBlockCache;	// CGrayMapBlock Cache Map Stuff. Max of 8*8=64 items in here. from MAP0.MUL file.
    public:
    	CObPointSortArray m_Teleports;	// CTeleport array
    	CGPtrTypeArray<CRegionBase*> m_RegionLinks;		// CRegionBase(s) in this CSector.
    public:
    	// Search for items and chars in a region must check 4 quandrants around location.
    	CCharsActiveList m_Chars_Active;		// CChar(s) in this CSector.
    	CCharsDisconnectList m_Chars_Disconnect;	// Idle player characters. Dead NPC's and ridden horses.
    	CItemsList m_Items_Timer;	// CItem(s) in this CSector that need timers.
    	CItemsList m_Items_Inert;	// CItem(s) in this CSector. (no timer required)
    public:
    
    	// Location map units.
    	int GetIndex() const;
    	CPointMap GetBasePoint() const
    	{
    		// What is the coord base of this sector. upper left point.
    		int index = GetIndex();
    		ASSERT( index >= 0 && index < SECTOR_QTY );
    		CPointMap pt(( index % SECTOR_COLS ) * SECTOR_SIZE_X, ( index / SECTOR_COLS ) * SECTOR_SIZE_Y );
    		return( pt );
    	}
    	CPointMap GetMidPoint() const
    	{
    		CPointMap pt = GetBasePoint();
    		pt.m_x += SECTOR_SIZE_X/2;	// East
    		pt.m_y += SECTOR_SIZE_Y/2;	// South
    		return( pt );
    	}
    	CRectMap GetRect() const
    	{
    		// Get a rectangle for the sector.
    		CPointMap pt = GetBasePoint();
    		CRectMap rect;
    		rect.m_left = pt.m_x;
    		rect.m_top = pt.m_y;
    		rect.m_right = pt.m_x + SECTOR_SIZE_X;	// East
    		rect.m_bottom = pt.m_y + SECTOR_SIZE_Y;	// South
    		return( rect );
    	}
    	bool IsInDungeon() const;
    
    	void CheckMapBlockCache( int iTime );
    	const CGrayMapBlock * GetMapBlock( const CPointMap & pt );
    
    	// CRegionBase
    	CRegionBase * GetRegion( const CPointBase & pt, DWORD dwType ) const;
    	bool UnLinkRegion( CRegionBase * pRegionOld );
    	bool LinkRegion( CRegionBase * pRegionNew );
    
    	// CTeleport(s) in the region.
    	CTeleport * GetTeleport( const CPointMap & pt ) const;
    	CTeleport * GetTeleport2d( const CPointMap & pt ) const;
    	bool AddTeleport( CTeleport * pTeleport );
    };
    
    
#endif // _INC_CSECTOR_H