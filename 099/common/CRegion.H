    // CRegion.H
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #ifndef _INC_CREGION_H
    #define _INC_CREGION_H
    #if _MSC_VER >= 1000
    #pragma once
    #endif // _MSC_VER >= 1000
    
    #include "crect.h"
    
    class CObjBaseTemplate : public CGObListRec
    {
    	// A dynamic object of some sort.
    private:
    	CGrayUID	m_UID;		// How the server will refer to this. 0 = static item
    	CGString	m_sName;	// unique name for the individual object.
    	CPointMap	m_pt;		// List is sorted by m_z_sort.
    public:
    	// Not saved. Just used for internal sorting.
    #if defined(GRAY_CLIENT) || defined(GRAY_MAP)
    	signed char m_z_sort;		// by what z should this be displayed..
    	signed char m_z_top;		// z + height.
    #endif
    
    protected:
    	void DupeCopy( const CObjBaseTemplate * pObj )
    	{
    		// NOTE: Never copy m_UID
    		ASSERT(pObj);
    		m_sName = pObj->m_sName;
    		m_pt = pObj->m_pt;
    	}
    
    	void SetUID( DWORD dwIndex )
    	{
    		// don't set container flags through here.
    		m_UID.SetObjUID( dwIndex );	// Will have UID_F_ITEM as well.
    	}
    	void SetUnkZ( signed char z )
    	{
    		m_pt.m_z = z;
    	}
    
    public:
    
    	CObjBaseTemplate * GetNext() const
    	{
    		return( STATIC_CAST <CObjBaseTemplate*> ( CGObListRec::GetNext()));
    	}
    	CObjBaseTemplate * GetPrev() const
    	{
    		return( STATIC_CAST <CObjBaseTemplate*> ( CGObListRec::GetPrev()));
    	}
    	CObjBaseTemplate()
    	{
    #if defined(GRAY_CLIENT) || defined(GRAY_MAP)
    		m_z_sort = UO_SIZE_MIN_Z;
    		m_z_top = UO_SIZE_MIN_Z;
    #endif
    	}
    
    	CGrayUID GetUID() const			{	return( m_UID ); }
    	bool IsItem( void ) const		{	return( m_UID.IsItem()); }
    	bool IsChar( void ) const		{	return( m_UID.IsChar()); }
    	bool IsItemInContainer() const	{	return( m_UID.IsItemInContainer() ); }
    	bool IsItemEquipped() const		{	return( m_UID.IsItemEquipped() ); }
    	bool IsDisconnected() const		{	return( m_UID.IsObjDisconnected() ); }
    	bool IsTopLevel() const			{	return( m_UID.IsObjTopLevel() ); }
    	bool IsValidUID() const			{	return( m_UID.IsValidUID() ); }
    
    #if defined(_DEBUG) && defined(GRAY_SVR)	// CObjBaseTemplate
    	bool IsValidContainer() const;	// am i really in the container i say i am ?
    #endif
    
    	void SetContainerFlags( DWORD dwFlags = 0 )
    	{
    		m_UID.SetObjContainerFlags( dwFlags );
    #if defined(_DEBUG) && defined(GRAY_SVR)	// CObjBaseTemplate
    		DEBUG_CHECK( IsValidContainer());
    #endif
    	}
    
    	virtual int IsWeird() const
    	{
    #ifdef _DEBUG
    		if ( ! IsValidDynamic())
    		{
    			return( 0x3101 );
    		}
    #endif
    		if ( GetParent() == NULL )
    		{
    			return( 0x3102 );
    		}
    #if defined(_DEBUG) && defined(GRAY_SVR)
    		if ( ! IsValidContainer())
    		{
    			return( 0x3103 );
    		}
    #endif
    		if ( ! IsValidUID())
    		{
    			return( 0x3104 );
    		}
    		return( 0 );
    	}
    
    	virtual CObjBaseTemplate * GetTopLevelObj( void ) const
    		= 0;
    
    #if defined(GRAY_SVR)
    	CSector * GetTopSector() const
    	{
    		// Assume it is top level.
    		return( GetTopPoint().GetSector());
    	}
    #endif
    
    	// Location
    
    	LAYER_TYPE GetEquipLayer() const
    	{
    		DEBUG_CHECK( IsItem());
    		DEBUG_CHECK( IsItemEquipped());
    		return( (LAYER_TYPE) m_pt.m_z );
    	}
    	void SetEquipLayer( LAYER_TYPE layer )
    	{
    		DEBUG_CHECK( IsItem());
    		SetContainerFlags( UID_O_EQUIPPED );
    		m_pt.m_x = 0;	// these don't apply.
    		m_pt.m_y = 0;
    		m_pt.m_z = layer; // layer equipped.
    		m_pt.m_mapplane = 0;
    	}
    
    	BYTE GetContainedLayer() const
    	{
    		// used for corpse or Restock count as well in Vendor container.
    		DEBUG_CHECK( IsItem());
    		DEBUG_CHECK( IsItemInContainer());
    		return( m_pt.m_z );
    	}
    	void SetContainedLayer( BYTE layer )
    	{
    		// used for corpse or Restock count as well in Vendor container.
    		DEBUG_CHECK( IsItem());
    		DEBUG_CHECK( IsItemInContainer());
    		m_pt.m_z = layer;
    	}
    	const CPointMap & GetContainedPoint() const
    	{
    		DEBUG_CHECK( IsItem());
    		DEBUG_CHECK( IsItemInContainer() );
    		return( m_pt );
    	}
    	void SetContainedPoint( const CPointMap & pt )
    	{
    		DEBUG_CHECK( IsItem());
    		SetContainerFlags( UID_O_CONTAINED );
    		m_pt.m_x = pt.m_x;
    		m_pt.m_y = pt.m_y;
    		m_pt.m_z = LAYER_NONE;
    		m_pt.m_mapplane = 0;
    	}
    
    	void SetTopPoint( const CPointMap & pt )
    	{
    		SetContainerFlags(0);
    		ASSERT( pt.IsValidPoint() );	// already checked b4.
    		m_pt = pt;
    	}
    	const CPointMap & GetTopPoint() const
    	{
    		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
    		return( m_pt );
    	}
    	void SetTopZ( signed char z )
    	{
    		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
    		m_pt.m_z = z;
    	}
    	signed char GetTopZ() const
    	{
    		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
    		return( m_pt.m_z );
    	}
    	unsigned char GetTopMap() const
    	{
    		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
    		return( m_pt.m_mapplane );
    	}
    
    	void SetUnkPoint( const CPointMap & pt )
    	{
    		m_pt = pt;
    	}
    	const CPointMap & GetUnkPoint() const
    	{
    		// don't care where this
    		return( m_pt );
    	}
    	signed char GetUnkZ() const
    	{
    		return( m_pt.m_z );
    	}
    
    	// Distance and direction
    	int GetTopDist( const CPointMap & pt ) const
    	{
    		return( GetTopPoint().GetDist( pt ));
    	}
    
    	int GetTopDist( const CObjBaseTemplate * pObj ) const
    	{
    		// don't check for logged out.
    		// Assume both already at top level.
    		ASSERT( pObj );
    		if ( pObj->IsDisconnected())
    			return( SHRT_MAX );
    		return( GetTopPoint().GetDist( pObj->GetTopPoint()));
    	}
    
    	int GetDist( const CObjBaseTemplate * pObj ) const
    	{
    		// logged out chars have infinite distance
    		// Assume i am already at top level.
    		DEBUG_CHECK( IsTopLevel());
    		if ( pObj == NULL )
    			return( SHRT_MAX );
    		pObj = pObj->GetTopLevelObj();
    		if ( pObj->IsDisconnected())
    			return( SHRT_MAX );
    		return( GetTopDist( pObj ));
    	}
    
    	int GetTopDist3D( const CObjBaseTemplate * pObj ) const // 3D Distance between points
    	{
    		// logged out chars have infinite distance
    		// Assume both already at top level.
    		ASSERT( pObj );
    		if ( pObj->IsDisconnected())
    			return( SHRT_MAX );
    		return( GetTopPoint().GetDist3D( pObj->GetTopPoint()));
    	}
    
    	DIR_TYPE GetTopDir( const CObjBaseTemplate * pObj, DIR_TYPE DirDefault = DIR_QTY ) const
    	{
    		ASSERT( pObj );
    		return( GetTopPoint().GetDir( pObj->GetTopPoint(), DirDefault ));
    	}
    
    	DIR_TYPE GetDir( const CObjBaseTemplate * pObj, DIR_TYPE DirDefault = DIR_QTY ) const
    	{
    		ASSERT( pObj );
    		pObj = pObj->GetTopLevelObj();
    		return( GetTopDir( pObj, DirDefault ));
    	}
    
    	virtual int GetVisualRange() const
    	{
    		return( UO_MAP_VIEW_SIZE );
    	}
    
    	// Names
    	LPCTSTR GetIndividualName() const
    	{
    		return( m_sName );
    	}
    	bool IsIndividualName() const
    	{
    		return( ! m_sName.IsEmpty());
    	}
    	virtual LPCTSTR GetName() const
    	{
    		return( m_sName );
    	}
    	virtual bool SetName( LPCTSTR pszName )
    	{
    		// NOTE: Name length <= MAX_NAME_SIZE
    		ASSERT(pszName);
    		m_sName = pszName;
    		return true;
    	}
    	virtual ~CObjBaseTemplate()
    	{
    	}
    };
    
    inline DIR_TYPE GetDirTurn( DIR_TYPE dir, int offset )
    {
    	// Turn in a direction.
    	// +1 = to the right.
    	// -1 = to the left.
    	offset += DIR_QTY + dir;
    	offset %= DIR_QTY;
    	return( (DIR_TYPE) (offset));
    }
    
    #ifndef _WIN32
    struct RECT
    {
    	int left;
    	int top;
    	int right;
    	int bottom;
    };
    struct POINT
    {
    	int x;
    	int y;
    };
    #endif
    
    struct CGRect			// Basic rectangle. (May not be on the map)
    {	// Same order and basic element sizing as _WIN32 RECT
    public:
    	int m_left;		// West	 x=0
    	int m_top;		// North y=0
    	int m_right;	// East	( NON INCLUSIVE !)
    	int m_bottom;	// South ( NON INCLUSIVE !)
    public:
    	int GetWidth() const { return( m_right - m_left ); }
    	int GetHeight() const { return( m_bottom - m_top ); }
    
        operator RECT &()
        {
    		return( *((RECT*)(&m_left)) );
        }
        operator RECT *()
        {
    		return( (::RECT*)(&m_left) );
        }
        operator const RECT &() const
        {
    		return( *((RECT*)(&m_left)) );
        }
        operator const RECT *() const
        {
    		return( (::RECT*)(&m_left) );
        }
    
    	bool IsRectEmpty() const
    	{
    		return( m_left >= m_right || m_top >= m_bottom );
    	}
    	void SetRectEmpty()
    	{
    		m_left = m_top = 0;	// 0x7ffe
    		m_right = m_bottom = 0;
    	}
    	void SetRect( const RECT * pRect )
    	{
    		ASSERT(pRect);
    		m_left = pRect->left;
    		m_top = pRect->top;
    		m_right = pRect->right;
    		m_bottom = pRect->bottom;
    	}
    
    	CGRect & operator = ( const RECT * pRect )
    	{
    		SetRect( pRect );
    		return( * this );
    	}
    	CGRect & operator = ( const RECT & rect )
    	{
    		SetRect( &rect );
    		return( * this );
    	}
    
    	void OffsetRect( int x, int y )
    	{
    		m_left += x;
    		m_top += y;
    		m_right += x;
    		m_bottom += y;
    	}
    	void UnionPoint( int x, int y )
    	{
    		// Inflate this rect to include this point.
    		// NON inclusive rect! 
    		if ( x	< m_left	) m_left = x;
    		if ( y	< m_top		) m_top = y;
    		if ( x	>= m_right	) m_right = x+1;
    		if ( y	>= m_bottom	) m_bottom = y+1;
    	}
    
    	bool IsInsideX( int x ) const
    	{	// non-inclusive
    		return( x >= m_left && x < m_right );
    	}
    	bool IsInsideY( int y ) const
    	{	// non-inclusive
    		return( y >= m_top && y < m_bottom );
    	}
    	bool IsInside( int x, int y ) const
    	{
    		// NON inclusive rect! Is the point in the rectangle ?
    		return( IsInsideX(x) &&	IsInsideY(y));
    	}
    	bool IsInside2d( const CPointBase & pt ) const
    	{
    		// NON inclusive rect! Is the point in the rectangle ?
    		return( IsInside( pt.m_x, pt.m_y ));
    	}
    
    	void InflateRectBoth( int x, int y )
    	{
    		// Normal InflateRect() does not move the top and left. This does.
    		m_left -= x;
    		m_top -= y;
    		m_right	+= x;
    		m_bottom += y;
    	}
    
    	void UnionRect( const CGRect & rect )
    	{
    		// Inflate this rect to include both rectangles.
    		// ASSUME: Normalized rect
    		if ( rect.IsRectEmpty())
    			return;
    		if ( IsRectEmpty())
    		{
    			*this = rect;
    			return;
    		}
    		if ( rect.m_left	< m_left	) m_left = rect.m_left;
    		if ( rect.m_top		< m_top		) m_top = rect.m_top;
    		if ( rect.m_right	> m_right	) m_right = rect.m_right;
    		if ( rect.m_bottom	> m_bottom	) m_bottom = rect.m_bottom;
    	}
    	bool IntersectRect( const CGRect & rect )
    	{
    		// Shrink this rectangle to be the overlap of the 2. (if anything)
    		// ASSUME: Normalized rect
    		// RETURN: 
    		//  true = rectangle is not empty.
    		if ( IsRectEmpty())
    			return( false );
    		if ( rect.IsRectEmpty())
    		{
    			SetRectEmpty();
    			return( false );
    		}
    		if ( rect.m_left	> m_left	) m_left = rect.m_left;
    		if ( rect.m_top		> m_top		) m_top = rect.m_top;
    		if ( rect.m_right	< m_right	) m_right = rect.m_right;
    		if ( rect.m_bottom	< m_bottom	) m_bottom = rect.m_bottom;
    		return( ! IsRectEmpty());
    	}
    	bool IsInside( const CGRect & rect ) const
    	{
    		// Is &rect inside me ?
    		// ASSUME: Normalized rect
    		if ( rect.m_left	< m_left	) 
    			return( false );
    		if ( rect.m_top		< m_top		)
    			return( false );
    		if ( rect.m_right	> m_right	)
    			return( false );
    		if ( rect.m_bottom	> m_bottom	)
    			return( false );
    		return( true );
    	}
    	bool IsOverlapped( const CGRect & rect ) const
    	{
    		// are the 2 rects overlapped at all ?
    		// NON inclusive rect.
    		// ASSUME: Normalized rect
    		if ( rect.m_left	>= m_right	) 
    			return( false );
    		if ( rect.m_top		>= m_bottom	)
    			return( false );
    		if ( rect.m_right	<= m_left	)
    			return( false );
    		if ( rect.m_bottom	<= m_top	)
    			return( false );
    		return( true );
    	}
    	bool IsEqual( const CGRect & rect ) const
    	{
    		return( ! memcmp( &m_left, &(rect.m_left), sizeof(int)*4));
    	}
    	virtual void NormalizeRect()
    	{
    		if ( m_bottom < m_top )
    		{
    			int wtmp = m_bottom;
    			m_bottom = m_top;
    			m_top = wtmp;
    		}
    		if ( m_right < m_left )
    		{
    			int wtmp = m_right;
    			m_right = m_left;
    			m_left = wtmp;
    		}
    	}
    
    	void SetRect( int left, int top, int right, int bottom )
    	{
    		m_left = left;
    		m_top = top;
    		m_right = right;
    		m_bottom = bottom;
    		NormalizeRect();
    	}
    
    	void CGRect::NormalizeRectMax( int cx, int cy )
    	{
    		if ( m_left < 0 )
    			m_left = 0;
    		if ( m_top < 0 )
    			m_top = 0;
    		if ( m_right > cx )
    			m_right = cx;
    		if ( m_bottom > cy )
    			m_bottom = cy;
    	}
    
    	int Read( LPCTSTR pVal );
    	TCHAR * Write( TCHAR * pBuffer ) const
    	{
    		sprintf( pBuffer, _TEXT("%d,%d,%d,%d"),
    			m_left,
    			m_top,
    			m_right,
    			m_bottom );
    		return( pBuffer );
    	}
    	LPCTSTR Write( void ) const;
    
    	CPointBase GetCenter() const
    	{
    		CPointBase pt;
    		pt.m_x = ( m_left + m_right ) / 2;
    		pt.m_y = ( m_top + m_bottom ) / 2;
    		pt.m_z = 0;
    		pt.m_mapplane = 0;
    		return( pt );
    	}
    
    	CPointBase GetRectCorner( DIR_TYPE dir ) const;
    	CSector * GetSector( int i ) const;	// ge all the sectors that make up this rect.
    };
    
    struct CRectMap : public CGRect
    {
    public:
    
    	bool IsValid() const
    	{
    		int iSizeX = GetWidth();
    		if ( iSizeX < 0 || iSizeX > UO_SIZE_X )
    			return( false );
    		int iSizeY = GetHeight();
    		if ( iSizeY < 0 || iSizeY > UO_SIZE_Y )
    			return( false );
    		return( true );
    	}
    
    	void NormalizeRect();
    	void NormalizeRectMax();
    };
    
    class CGRegion
    {
    	// A bunch of rectangles forming an area.
    private:
    	CGRect m_rectUnion;	// The union rectangle.
    	CGTypedArray<CGRect, const CGRect&> m_Rects;
    private:
    	virtual bool RealizeRegion();
    public:
    	bool IsRegionEmpty() const
    	{
    		return( m_rectUnion.IsRectEmpty());
    	}
    	void EmptyRegion()
    	{
    		m_rectUnion.SetRectEmpty();
    		m_Rects.Empty();
    	}
    	int GetRegionRectCount() const;
    	CGRect & GetRegionRect(int i);
    	const CGRect & GetRegionRect(int i) const;
    	virtual bool AddRegionRect( const CGRect & rect );
    	virtual void RemoveRegionRect( int i );
    
    	CPointBase GetRegionCorner( DIR_TYPE dir = DIR_QTY ) const;
    	bool IsInside2d( const CPointBase & pt ) const;
    
    	bool IsOverlapped( const CGRect & rect ) const;
    	bool IsInside( const CGRect & rect ) const;
    
    	bool IsInside( const CGRegion * pRegionIsSmaller ) const;
    	bool IsOverlapped( const CGRegion * pRegionTest ) const;
    	bool IsEqualRegion( const CGRegion * pRegionTest ) const;
    
    	CSector * GetSector( int i ) const	// get all the sectors that make up this rect.
    	{
    		return m_rectUnion.GetSector(i);
    	}
    
    	CGRegion();
    };
    
    #ifdef GRAY_MAP
    class CResourceDef : public CScriptObj
    {
    #define RESOURCE_ID DWORD
    	// dummy this out.
    public:
    	DWORD m_RegionType;	// REGION_TYPE_AREA
    	CGString m_sDefName;
    public:
    	CResourceDef( DWORD rid )
    	{
    		m_RegionType = rid;
    	}
    	DWORD GetResourceID() const
    	{
    		// REGION_TYPE_AREA
    		return( m_RegionType );
    	}
    	bool IsMatchType( DWORD dwID ) const
    	{
    		return(( m_RegionType & dwID ) ? true : false );
    	}
    
    	LPCTSTR GetResourceName() const
    	{
    		return( m_sDefName );
    	}
    	bool SetResourceName( LPCTSTR psDefName )
    	{
    		m_sDefName = psDefName;
    		return( true );
    	}
    };
    #endif
    
    #if defined(GRAY_MAP) || defined(GRAY_SVR)
    
    enum RegionClass_Type
    {
    	// General classification for searching purposes.
    	// Ask NPC for type of area ?
    	RegionClass_Unspecified = 0,	// Unspecified.
    	RegionClass_Jail,
    	// ...
    	RegionClass_Qty,
    };
    
    class CRegionBase : public CResourceDef, public CGRegion
    {
    	// region of the world of arbitrary size and location.
    	// made of (possibly multiple) rectangles.
    	// RES_ROOM or base for RES_AREA
    protected:
    	DECLARE_MEM_DYNAMIC;
    
    private:
    	CGString m_sName;	// Name of the region.
    
    #define REGION_ANTIMAGIC_ALL		0x000001	// All magic banned here.
    #define REGION_ANTIMAGIC_RECALL_IN	0x000002	// Teleport,recall in to this, and mark
    #define REGION_ANTIMAGIC_RECALL_OUT	0x000004	// can't recall out of here.
    #define REGION_ANTIMAGIC_GATE		0x000008
    #define REGION_ANTIMAGIC_TELEPORT	0x000010	// Can't teleport into here.
    #define REGION_ANTIMAGIC_DAMAGE		0x000020	// just no bad magic here
    
    #define REGION_FLAG_SHIP			0x000040	// This is a ship region. ship commands
    #define REGION_FLAG_NOBUILDING		0x000080	// No building in this area
    
    #define REGION_FLAG_ANNOUNCE		0x000200	// Announce to all who enter.
    #define REGION_FLAG_INSTA_LOGOUT	0x000400	// Instant Log out is allowed here. (hotel)
    #define REGION_FLAG_UNDERGROUND		0x000800	// dungeon type area. (no weather)
    #define REGION_FLAG_NODECAY			0x001000	// Things on the ground don't decay here.
    
    #define REGION_FLAG_SAFE			0x002000	// This region is safe from all harm.
    #define REGION_FLAG_GUARDED			0x004000	// try TAG.GUARDOWNER
    #define REGION_FLAG_NO_PVP			0x008000	// Players cannot directly harm each other here.
    #define REGION_FLAG_ARENA			0x010000	// Anything goes. no murder counts or crimes.
    
    	DWORD m_dwFlags;
    
    public:
    	CPointMap m_pt;			// safe point in the region. (for teleporting to)
    	RegionClass_Type m_RegionClass;	// general class of the region.
    	int m_iLinkedSectors;	// just for statistics tracking. How many sectors are linked ?
    
    	static LPCTSTR const sm_szLoadKeys;
    	static LPCTSTR const sm_szVerbKeys;
    
    private:
    	bool SendSectorsVerb( LPCTSTR pszVerb, LPCTSTR pszArgs, CTextConsole * pSrc ); // distribute to the CSectors
    
    public:
    	virtual bool RealizeRegion();
    	void UnRealizeRegion();
    
    	void SetName( LPCTSTR pszName );
    	LPCTSTR GetName() const
    	{
    		return( m_sName );
    	}
    	const CGString & GetNameStr() const
    	{
    		return( m_sName );
    	}
    
    	void r_WriteBase( CScript & s );
    
    	virtual bool r_LoadVal( CScript & s );
    	virtual bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc );
    	virtual void r_WriteBody( CScript & s, LPCTSTR pszPrefix );
    #ifdef GRAY_SVR
    	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
    #endif
    #ifdef GRAY_MAP
    	virtual void r_Write( CScript & s );
    #endif
    
    	bool AddRegionRect( const CRectMap & rect );
    	bool SetRegionRect( const CRectMap & rect )
    	{
    		EmptyRegion();
    		return AddRegionRect( rect );
    	}
    	DWORD GetRegionFlags() const
    	{
    		return( m_dwFlags );
    	}
    	bool IsFlag( DWORD dwFlags ) const
    	{	// REGION_FLAG_GUARDED
    		return(( m_dwFlags & dwFlags ) ? true : false );
    	}
    	bool IsGuarded() const;
    	void SetRegionFlags( DWORD dwFlags )
    	{
    		m_dwFlags |= dwFlags;
    	}
    	void TogRegionFlags( DWORD dwFlags, bool fSet )
    	{
    		if ( fSet )
    			m_dwFlags |= dwFlags;
    		else
    			m_dwFlags &= ~dwFlags;
    	}
    
    	bool CheckAntiMagic( SPELL_TYPE spell ) const;
    	virtual bool IsValid() const
    	{
    #if defined(_DEBUG) && ! defined(GRAY_MAP)
    		if ( ! IsValidDynamic())
    			return( false );
    #endif
    		return( m_sName.IsValid());
    	}
    
    	CRegionBase( RESOURCE_ID rid, LPCTSTR pszName = NULL );
    	virtual ~CRegionBase();
    };
    
    enum RTRIG_TYPE
    {
    	// XTRIG_UNKNOWN	= some named trigger not on this list.
    	RTRIG_CLIPERIODIC=1,		// happens to each client.
    	RTRIG_ENTER,
    	RTRIG_EXIT,
    	RTRIG_REGPERIODIC,	// regional periodic. Happens if just 1 or many clients)
    	RTRIG_STEP,
    	RTRIG_QTY,
    };
    
    class CRandGroupDef;
    
    class CRegionWorld : public CRegionBase
    {
    	// A region with extra tags and properties.
    	//  = RES_AREA
    public:
    	static LPCTSTR const sm_szLoadKeys;
    	static LPCTSTR const sm_szTrigName;
    protected:
    	DECLARE_MEM_DYNAMIC;
    public:
    
    #ifdef GRAY_SVR
    	CResourceRefArray m_Events;	// trigger  when entered or exited RES_REGIONTYPE
    	CVarDefArray m_TagDefs;		// attach extra tags here.
    
    	// Standard extra tags:
    	// "TAG.GUARDOWNER" = should have the word "the" in it if needs it.
    	// "TAG.ANNOUNCEMENT"
    	// "TAG.GUARDID"	= body id for the guards.
    
    public:
    	TRIGRET_TYPE OnRegionTrigger( CTextConsole * pChar, RTRIG_TYPE trig );
    	const CRandGroupDef * FindNaturalResource( int /* IT_TYPE */ type ) const;
    #endif	// GRAY_SVR
    
    public:
    
    	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
    	virtual bool r_LoadVal( CScript & s );
    	virtual bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc );
    	virtual void r_WriteBody( CScript &s, LPCTSTR pszPrefix );
    #ifdef GRAY_MAP
    	virtual void r_Write( CScript & s );
    #endif
    #ifdef GRAY_SVR
    	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
    #endif
    
    	CRegionWorld( RESOURCE_ID rid, LPCTSTR pszName = NULL );
    	virtual ~CRegionWorld();
    };
    
    class CTeleport : public CMemDynamic, public CPointSort	// The static world teleporters. GRAYMAP.SCP
    {
    	// Put a built in trigger here ? can be Array sorted by CPointMap.
    protected:
    	DECLARE_MEM_DYNAMIC;
    public:
    #ifdef GRAY_MAP
    	CString m_sName;
    #endif
    	CPointMap m_ptDst;
    public:
    	CTeleport( const CPointMap & pt ) :	CPointSort(pt)
    	{
    		ASSERT( pt.IsValidPoint());
    		m_ptDst = pt;
    	}
    	CTeleport( TCHAR * pszArgs );
    	virtual ~CTeleport()
    	{
    	}
    #ifdef GRAY_MAP
    	void SetSrc( const CPointMap & ptSrc );
    #endif
    	void SetDst( const CPointMap & pt )
    	{
    		m_ptDst = pt;
    	}
    	bool RealizeTeleport();
    };
    
    class CStartLoc : public CMemDynamic	// The start locations for creating a new char. GRAY.INI
    {
    protected:
    	DECLARE_MEM_DYNAMIC;
    public:
    	CGString m_sArea;	// Area/City Name = Britain or Occlo
    	CGString m_sName;	// Place name = Castle Britannia or Docks
    	CPointMap m_pt;
    public:
    	CStartLoc( LPCTSTR pszArea )
    	{
    		m_sArea = pszArea;
    	}
    };
    
    #endif	// GRAY_SVR || GRAY_MAP
    #endif
