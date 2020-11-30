//
// CRegion.H
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CREGION_H
#define _INC_CREGION_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "carray.h"
#include "cscript.h"

class CRegionBase;
class CSector;

#define SECTOR_SIZE_X	64	// dungeons use 256 (smaller 32 ???)
#define SECTOR_SIZE_Y	64
#define SECTOR_COLS	( UO_SIZE_X / SECTOR_SIZE_X )
#define SECTOR_ROWS	( UO_SIZE_Y / SECTOR_SIZE_Y )
#define SECTOR_QTY	( SECTOR_COLS * SECTOR_ROWS )

///////////////////////////////////////////////////////////////////////////

struct CPointBase	// Non initialized 3d point.
{
public:
	signed short m_x;	// equipped items dont need x,y
	signed short m_y;
	signed char m_z; // This might be layer if equipped ? or eqipped on corpse. Not used if in other container.

public:
	void Init()
	{
		m_x = -1;	// invalid location.
		m_y = -1;
		m_z = 0;
	}
	void Zero()
	{
		m_x = 0;	// invalid location.
		m_y = 0;
		m_z = 0;
	}
	int GetDist( const CPointBase pt ) const // Distance between points
	{
		int dx = abs(m_x-pt.m_x);
		int dy = abs(m_y-pt.m_y);
		return( max( dx, dy ));
	}
	int GetDist3D( const CPointBase pt ) const; // 3D Distance between points

	bool IsValid() const
	{
		if ( m_z <= -UO_SIZE_Z || m_z >= UO_SIZE_Z ) return( false );
		if ( m_x < 0 || m_x >= UO_SIZE_X ) return( false );
		if ( m_y < 0 || m_y >= UO_SIZE_Y ) return( false );
		return( true );
	}
	bool IsCharValid() const
	{
		if ( m_z <= -UO_SIZE_Z || m_z >= UO_SIZE_Z ) return( false );
		if ( m_x <= 0 || m_x >= UO_SIZE_X ) return( false );
		if ( m_y <= 0 || m_y >= UO_SIZE_Y ) return( false );
		return( true );
	}

	void Validate()
	{
		if ( m_x < 0 ) m_x = 0;
		if ( m_x >= UO_SIZE_X ) m_x = UO_SIZE_X-1;
		if ( m_y < 0 ) m_y = 0;
		if ( m_y >= UO_SIZE_Y ) m_y = UO_SIZE_Y-1;
	}

	bool IsSame2D( const CPointBase & pt ) const
	{
		return( m_x == pt.m_x && m_y == pt.m_y );
	}
	bool operator == ( const CPointBase & pt ) const
	{
		return( m_x == pt.m_x && m_y == pt.m_y && m_z == pt.m_z );
	}
	bool operator != ( const CPointBase & pt ) const
	{
		return( ! ( *this == pt ));
	}
	const CPointBase operator += ( const CPointBase pt )
	{
		m_x += pt.m_x;
		m_y += pt.m_y;
		m_z += pt.m_z;
		return( * this );
	}
	const CPointBase operator -= ( const CPointBase pt )
	{
		m_x -= pt.m_x;
		m_y -= pt.m_y;
		m_z -= pt.m_z;
		return( * this );
	}
	void Set( int x, int y, int z = 0 )
	{
		m_x = x;
		m_y = y;
		m_z = z;
	}
	int Read( TCHAR * pVal );
	const TCHAR * Write( void ) const;

	void Move( DIR_TYPE dir, int dist = 1 );
	DIR_TYPE GetDir( const CPointBase pt, DIR_TYPE DirDefault = DIR_QTY ) const; // Direction to point pt
	void CalcPath( const CPointBase pSrc, int iSteps );

	CSector * GetSector() const;
	CRegionBase * GetRegion( DWORD dwType ) const;
	bool IsInDungeon() const;
};

struct CPointMap : public CPointBase
{
	// A point in the world (or in a container) (initialized)
	CPointMap() { Init(); }
	CPointMap( WORD x, WORD y, signed char z = 0 ) 
	{
		m_x = x; m_y = y; m_z = z;
	}
	CPointMap & operator = ( const CPointBase pt )
	{
		m_x = pt.m_x;
		m_y = pt.m_y;
		m_z = pt.m_z;
		return( * this );
	}
	CPointMap( CPointBase pt )
	{
		*this = pt;
	}
	CPointMap( TCHAR * pVal )
	{
		Read( pVal );
	}
};

#ifndef _WIN32
struct RECT
{
	int left;
	int top;
	int right;
	int bottom;
};
#endif

struct CGRect			// Basic rectangle. (May not be on the map)
{	// Same order and basic element sizing as _WIN32 RECT
public:
	int m_left;		// West	 x=0 
	int m_top;		// North y=0
	int m_right;	// East
	int m_bottom;	// South
public:
	int GetWidth() const { return( m_right - m_left ); }
	int GetHeight() const { return( m_bottom - m_top ); }

#ifdef _WIN32
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
	CGRect & operator = ( RECT rect )
	{
		m_left = rect.left;		
		m_top = rect.top;		
		m_right = rect.right;	
		m_bottom = rect.bottom;	
		return( * this );
	}
#endif

	bool IsEmpty() const
	{
		return( m_left >= m_right || m_top >= m_bottom );
	}
	void Empty()
	{
		m_left = m_top = 0;	// 0x7ffe
		m_right = m_bottom = 0;
	}

	void UnionPoint( int x, int y )
	{
		// Inflate this rect to include this point.
		if ( x	< m_left	) m_left = x;
		if ( y	< m_top		) m_top = y;
		if ( x	> m_right	) m_right = x;
		if ( y	> m_bottom	) m_bottom = y;
	}

	void UnionRect( const CGRect & rect )
	{
		// Inflate this rect to include both rectangles.
		if ( rect.m_left	< m_left	) m_left = rect.m_left;
		if ( rect.m_top		< m_top		) m_top = rect.m_top;
		if ( rect.m_right	> m_right	) m_right = rect.m_right;
		if ( rect.m_bottom	> m_bottom	) m_bottom = rect.m_bottom;
	}
	bool IntersectRect( const CGRect & rect )
	{
		// Shrink this rectangle to be the overlap of the 2. (if anything)
		if ( rect.m_left	> m_left	) m_left = rect.m_left;
		if ( rect.m_top		> m_top		) m_top = rect.m_top;
		if ( rect.m_right	< m_right	) m_right = rect.m_right;
		if ( rect.m_bottom	< m_bottom	) m_bottom = rect.m_bottom;
		return( ! IsEmpty());
	}
	bool IsInside( const CGRect & rect ) const
	{
		// Is this rect inside the rect specified.
		return( IsInside( rect.m_left, rect.m_top ) && 
			IsInside( rect.m_right, rect.m_bottom ));
	}
	void OffsetRect( int x, int y )
	{
		m_left += x;
		m_top += y;
		m_right += x;
		m_bottom += y;
	}
	bool IsInside( int x, int y ) const
	{
		// inclusive rect.
		return( x >= m_left &&	// West
			y >= m_top &&		// North
			x <= m_right &&		// East
			y <= m_bottom );	// South	
	}
	bool IsInside( const CPointBase & pt ) const
	{
		// inclusive rect.
		return( pt.m_x >= m_left &&	// West
			pt.m_y >= m_top &&		// North
			pt.m_x <= m_right &&		// East
			pt.m_y <= m_bottom );	// South	
	}
	bool IsOverlapped( const CGRect & rect ) const
	{
		// are the 2 rects overlapped at all ?
		CGRect recttest = rect;
		return( recttest.IntersectRect( *this ));
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

	int Read( const TCHAR * pVal );
	const TCHAR * Write( void ) const;

	CPointBase GetCenter() const
	{
		CPointBase pt;
		pt.m_x = ( m_left + m_right ) / 2;
		pt.m_y = ( m_top + m_bottom ) / 2;
		pt.m_z = 0;
		return( pt );
	}
	CPointBase GetDirCorner( DIR_TYPE dir ) const;
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
};

class CGRegion
{
	// A bunch of rectangles forming an area.
private:
	CGTypedArray<CGRect, const CGRect&> m_Rects;
private:
	virtual bool RealizeRegion();
public:
	bool IsRegionEmpty() const
	{
		return( ! m_Rects.GetCount());
	}
	void EmptyRegion()
	{
		m_Rects.Empty();
	}
	int GetRegionRectCount() const
	{
		return( m_Rects.GetCount());
	}
	CGRect & GetRegionRect(int i)
	{
		return( m_Rects.ElementAt(i));
	}
	const CGRect & GetRegionRect(int i) const
	{
		return( m_Rects.ElementAt(i));
	}
	virtual bool AddRegionRect( const CGRect & rect )
	{
		m_Rects.Add( rect );
		return( true );
	}
	void RemoveRegionRect( int i )
	{
		m_Rects.RemoveAt(i);
	}

	bool IsEqualRegion( const CGRegion * pRegionTest ) const
	{
		int iQty = GetRegionRectCount();
		int iQtyTest = pRegionTest->GetRegionRectCount();
		if ( iQty != iQtyTest ) 
			return( false );
		int iEqualCount = 0;
		for ( int j=0; j<iQty; j++ )
		{
			for ( int i=0; i<iQtyTest; i++ )
			{
				// Find dupe rectangles.
				if ( m_Rects[j].IsEqual( pRegionTest->m_Rects[i] ))
				{
					iEqualCount ++;
					break;
				}
			}
		}
		// All same ?
		return( iEqualCount == iQty && iEqualCount == iQtyTest );
	}

	CPointBase GetDirCorner( DIR_TYPE dir = DIR_QTY ) const
	{
		// NOTE: DIR_QTY = center.
		return( m_Rects.ElementAt(0).GetDirCorner(dir));
	}
	bool IsOverlapped( const CGRect & rect ) const
	{
		// Does the region overlap this rectangle.
		for ( int i=0; i<m_Rects.GetCount(); i++ )
		{
			if ( rect.IsOverlapped( m_Rects[i] ))
				return( true );
		}
		return( false );
	}
	bool IsInside( const CPointBase & pt ) const
	{
		for ( int i=0; i<m_Rects.GetCount(); i++ )
		{
			if ( m_Rects[i].IsInside( pt ))
				return( true );
		}
		return( false );
	}
};

enum RegionClass_Type
{
	RegionClass_Unspecified = 0,	// Unspecified.
	RegionClass_Jail,

	RegionClass_Qty,
};

class CRegionBase : public CGObListRec, public CScriptObj, public CGRegion
{
	// Square region of the world of arbitrary size and location.
protected:
	DECLARE_MEM_DYNAMIC;
private:
#define REGION_TYPE_AREA		UID_SPEC		// Area/Continent/Town/City (CRegionWorld) guarded areas 
#define REGION_TYPE_ROOM		UID_EQUIPPED	// Room/Building/House. (static stat.mul type)
#define REGION_TYPE_MULTI		UID_ITEM		// A multi item = house or ship.

	DWORD m_RegionType;	// REGION_TYPE_* or multi UID number
	CGString m_sName;	// Name of the region.

#define REGION_ANTIMAGIC_ALL		0x0001	// All magic banned here.
#define REGION_ANTIMAGIC_RECALL_IN	0x0002	// Teleport,recall in to this, and mark
#define REGION_ANTIMAGIC_RECALL_OUT	0x0004	// can't recall out of here.
#define REGION_ANTIMAGIC_GATE		0x0008
#define REGION_ANTIMAGIC_TELEPORT	0x0010	// Can't teleport into here.
#define REGION_ANTIMAGIC_DAMAGE		0x0020	// just no bad magic here

#define REGION_FLAG_SHIP			0x0040	// This is a ship region. ship commands
#define REGION_FLAG_NOBUILDING		0x0080	// No building in this area
#define REGION_FLAG_GLOBALNAME		0x0100	// Make sure the name is avail globally.
#define REGION_FLAG_ANNOUNCE		0x0200	// Announce to all who enter.
#define REGION_FLAG_INSTA_LOGOUT	0x0400	// Instant Log out is allowed here. (hotel)
#define REGION_FLAG_UNDERGROUND		0x0800	// dungeon type area. (no weather)
#define REGION_FLAG_NODECAY			0x1000	// Things on the ground don't decay here.

#define REGION_FLAG_SAFE			0x2000	// This region is safe from all harm.
#define REGION_FLAG_GUARDED			0x4000
#define REGION_DYNAMIC				0x8000	// Must be saved in the WORLD.SCP file (not multi)

	WORD m_Flags;	

public:
	CPointBase m_p;			// safe point in the region. (for teleporting to)
	RegionClass_Type m_RegionClass;

	static const TCHAR * sm_KeyTable[];

private:
	bool SendSectorsVerb( const TCHAR * pszVerb, const TCHAR * pszArgs, CTextConsole * pSrc ); // distribute to the CSectors

public:
	bool RealizeRegion();
	void SetName( const TCHAR * pszName );
	CRegionBase( DWORD Type, const TCHAR * pszName = NULL )
	{
		m_RegionType = Type;		// REGION_TYPE_*
		m_Flags = 0;
		m_p.Init();
		m_RegionClass = RegionClass_Unspecified;
		if ( pszName ) SetName( pszName );
	}
	const TCHAR * GetName() const
	{
		return( m_sName );
	}
	const CGString & GetNameStr() const
	{
		return( m_sName );
	}

	DWORD GetMultiUID() const
	{
#ifdef DEBUG_CHECK
		DEBUG_CHECK( m_RegionType & UID_MASK );
		DEBUG_CHECK( m_RegionType & REGION_TYPE_MULTI );
#endif
		return( m_RegionType );
	}
	bool IsMatchType( DWORD dwType ) const
	{
		// REGION_TYPE_MULTI = Specific UID's ? or UID in general = CRegionWorld
		// REGION_TYPE_AREA = World region area only = CRegionWorld
		// REGION_TYPE_ROOM = NPC House areas only = CRegionBase.
		if ( dwType & UID_MASK )	// looking for a specific multi ? 
		{
			return( m_RegionType == dwType );
		}
		return(( m_RegionType & dwType ) ? true : false );
	}
	virtual bool IsMatchName( const TCHAR * pszName ) const
	{
		return( ! m_sName.CompareNoCase( pszName ));
	}

	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	virtual void r_Write( CScript &s );

	bool CheckRealized();
	bool AddRegionRect( const CRectMap & rect );
	bool SetRegionRect( const CRectMap & rect )
	{
		EmptyRegion();
		return AddRegionRect( rect );
	}
	bool IsFlag( WORD wFlags ) const
	{	// REGION_FLAG_GUARDED
		return(( m_Flags & wFlags ) ? true : false );
	}
	bool IsGuarded() const
	{
		return( IsFlag( REGION_FLAG_GUARDED ));
	}
	void ModFlags( WORD wFlags, bool fSet = true )
	{
		if ( fSet )
			m_Flags |= wFlags;
		else
			m_Flags &= ~wFlags;
	}
	WORD GetFlags() const
	{
		return( m_Flags );
	}

	CRegionBase* GetNext() const
	{
		return( STATIC_CAST <CRegionBase*>( CGObListRec::GetNext()));
	}
	bool CheckAntiMagic( SPELL_TYPE spell ) const;
	virtual bool IsValid() const
	{
#ifdef _DEBUG
		if ( ! IsValidDynamic())
			return( false );
#endif
		return( m_sName.IsValid());
	}
};

class CRegionWorld : public CRegionBase
{
	// [AREA]
public:
	static const TCHAR * sm_KeyTable[];
protected:
	DECLARE_MEM_DYNAMIC;
public:
	WORD m_TrigEntry;	// trigger [REGION x]	when entered or exited
	WORD m_TrigPeriodic;	// trigger just do this periodically. (only when clients in it.)
	
	CGString m_sGuardAnnounce;	// should have the word "the" in it if needs it.
	CGString m_sLocal;	// Name of the region. (for speech scripts)

public:

	CRegionWorld( DWORD Type, const TCHAR * pszName = NULL ) : CRegionBase( Type, pszName )
	{
		m_TrigEntry = 0;
		m_TrigPeriodic = 0;
	}
	bool IsMatchName( const TCHAR * pszName ) const
	{
		if ( CRegionBase::IsMatchName( pszName )) return( true );
		return( ! m_sLocal.CompareNoCase( pszName ));
	}

	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	virtual void r_Write( CScript &s );
	bool OnRegionTrigger( CTextConsole * pChar, bool fEnter = true, bool fPeriodic = false );

	bool IsValid() const
	{
		return( CRegionBase::IsValid() && m_sGuardAnnounce.IsValid() && m_sLocal.IsValid());
	}
};

class CRegionJail : public CRegionWorld
{
	// This is a special region that is a jail.
public:
	static const TCHAR * sm_KeyTable[];
public:
	CPointMap m_ptJailBanish;	// if it's a jail area, the point to banish people to
	CPointMap m_ptJailPoint;	// Point to this regions jail (if any)
	CPointMap m_ptJailRelease;	// if it's a jail area, the release point
	CPointMap m_ptJailBreak;	// if it's a jail area, the break out point
public:
	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
};

class CTeleport : public CMemDynamic	// The static world teleporters. GRAYMAP.SCP
{
	// Put a build in trigger here ?
protected:
	DECLARE_MEM_DYNAMIC;
public:
#ifdef GRAY_MAP
	CGString m_sName;
#endif
	CPointBase m_Src;
	CPointBase m_Dst;
public:
	CTeleport( TCHAR ** ppCmd, int iArgs );
#ifdef GRAY_MAP
	CTeleport();
	void SetSrc( const CPointMap & pt );
	void SetDst( const CPointMap & pt )
	{
		m_Dst = pt;
	}
#endif
};

class CStartLoc : public CMemDynamic	// The start locations for creating a new char. GRAY.INI
{
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CGString m_sArea;	// Area/City Name = Britain or Occlo
	CGString m_sName;	// Place name = Castle Britannia or Docks
	CPointBase m_p;
public:
	CStartLoc( const TCHAR * pszArea )
	{
		m_sArea = pszArea;
	}
};

class CObjBaseTemplate : public CGObListRec
{
	// A dynamic object of some sort.
private:
	CGString	m_sName;	// unique name for the individual object.
	CPointMap	m_p;		// List is sorted by z.
	CObjUID		m_UID;		// How the server will refer to this. 0 = static item
public:
	// Not saved. Just used for internal sorting.
#ifndef GRAY_SVR
	signed char m_z_sort;		// by what z should this be displayed..
	signed char m_z_top;		// z + height.
#endif

protected:
	void DupeCopy( const CObjBaseTemplate * pObj )
	{
		// NOTE: Never copy m_UID
		m_sName = pObj->m_sName;
		m_p = pObj->m_p;
	}

	void SetPrivateUID( DWORD dwIndex )
	{
		m_UID.SetUID( dwIndex );	// Will have UID_ITEM as well.
	}
	void SetUnkPoint( TCHAR * pszScript )
	{
		m_p.Read( pszScript );
	}
	void SetUnkZ( signed char z )
	{
		m_p.m_z = z;
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
#ifndef GRAY_SVR
		m_z_sort = UO_SIZE_MIN_Z;
		m_z_top = UO_SIZE_MIN_Z;
#endif
	}

	CObjUID GetUID() const		{	return( m_UID ); }
	bool IsItem( void ) const	{	return( m_UID.IsItem()); }
	bool IsChar( void ) const	{	return( m_UID.IsChar()); }
	bool IsInContainer() const	{	return( m_UID.IsInContainer() ); }
	bool IsEquipped() const		{	return( m_UID.IsEquipped() ); }
	bool IsDisconnected() const	{	return( m_UID.IsDisconnected() ); }
	bool IsTopLevel() const		{	return( m_UID.IsTopLevel() ); }
	bool IsValidUID() const		{	return( m_UID.IsValidUID() ); }

#if defined(_DEBUG) && defined(GRAY_SVR)	// CObjBaseTemplate
	bool IsValidContainer() const;	// am i really in the container i say i am ?
#endif

	void SetContainerFlags( DWORD dwFlags = 0 )
	{
		m_UID.SetContainerFlags( dwFlags );
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
		DEBUG_CHECK( IsEquipped());
		return( (LAYER_TYPE) m_p.m_z );
	}
	void SetEquipLayer( LAYER_TYPE layer )
	{
		DEBUG_CHECK( IsItem());
		SetContainerFlags( UID_EQUIPPED );
		m_p.m_x = 0;	// these don't apply.
		m_p.m_y = 0;
		m_p.m_z	= layer; // layer equipped.
	}

	BYTE GetContainedLayer() const
	{
		// used for corpse or Restock count as well in Vendor container.
		DEBUG_CHECK( IsItem());
		DEBUG_CHECK( IsInContainer());
		return( m_p.m_z );
	}
	void SetContainedLayer( BYTE layer )
	{
		// used for corpse or Restock count as well in Vendor container.
		DEBUG_CHECK( IsItem());
		DEBUG_CHECK( IsInContainer());
		m_p.m_z = layer;
	}
	const CPointMap & GetContainedPoint() const
	{
		DEBUG_CHECK( IsItem());
		DEBUG_CHECK( IsInContainer() );
		return( m_p );
	}
	void SetContainedPoint( const CPointMap & pt )
	{
		DEBUG_CHECK( IsItem());
		SetContainerFlags( UID_CONTAINED );
		m_p.m_x = pt.m_x;
		m_p.m_y = pt.m_y;
		m_p.m_z = LAYER_NONE;
	}

	void SetTopPoint( const CPointMap & pt )
	{
		SetContainerFlags(0);
		ASSERT( pt.IsValid() );	// already checked b4.
		m_p = pt;
	}
	const CPointMap & GetTopPoint() const
	{
		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
		return( m_p );
	}
	void SetTopZ( signed char z )
	{
		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
		m_p.m_z = z;
	}
	signed char GetTopZ() const
	{
		DEBUG_CHECK( IsTopLevel() || IsDisconnected());
		return( m_p.m_z );
	}

	void SetUnkPoint( const CPointMap & pt )
	{
		m_p = pt;
	}
	const CPointMap & GetUnkPoint() const
	{
		// don't care where this
		return( m_p );
	}
	signed char GetUnkZ() const
	{
		return( m_p.m_z );
	}

	// Distance and direction

	int GetTopDist( const CObjBaseTemplate * pObj ) const
	{
		// don't check for logged out.
		// Assume both already at top level.
		ASSERT( pObj );
		return( GetTopPoint().GetDist( pObj->GetTopPoint()));
	}

	int GetDist( const CObjBaseTemplate * pObj ) const
	{
		// logged out chars have infinite distance
		// Assume i am already at top level.
		DEBUG_CHECK( IsTopLevel());
		if ( pObj == NULL )
			return( 0xFFFF );
		pObj = pObj->GetTopLevelObj();
		if ( pObj->IsDisconnected()) 
			return( 0xFFFF );
		return( GetTopDist( pObj ));
	}

	int GetTopDist3D( const CObjBaseTemplate * pObj ) const // 3D Distance between points
	{
		// logged out chars have infinite distance
		// Assume both already at top level.
		ASSERT( pObj );
		if ( pObj->IsDisconnected()) 
			return( 0xFFFF );
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
	const TCHAR * GetIndividualName() const
	{
		return( m_sName );
	}
	bool IsIndividualName() const
	{
		return( ! m_sName.IsEmpty());
	}
	virtual const TCHAR * GetName() const
	{
		return( m_sName );
	}
	virtual bool SetName( const TCHAR * pszName )
	{
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

extern const TCHAR * Dirs[DIR_QTY+1];

#endif // _INC_CREGION_H