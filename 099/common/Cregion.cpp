    // Cregion.cpp
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
    #include "cregion.h"
    #endif
    
    //************************************************************************
    // -CTeleport
    
    CTeleport::CTeleport( TCHAR * pszArgs )
    {
    	// RES_TELEPORT
    	// Assume valid iArgs >= 5
    
    	TCHAR * ppCmds;
    	int iArgs = Str_ParseCmds( pszArgs, ppCmds, COUNTOF( ppCmds ), "=" );
    	if ( iArgs < 2 )
    	{
    		DEBUG_ERR(( "Bad CTeleport Def\n" ));
    		return;
    	}
    
    	Read( ppCmds );
    	m_ptDst.Read( ppCmds );
    
    #ifdef GRAY_MAP
    	if ( iArgs >= 3 )
    	{
    		m_sName = ppCmds;
    	}
    #endif
    }
    
    #ifdef GRAY_MAP
    
    void CTeleport::SetSrc( const CPointMap & pt )
    {
    	CSector * pSector = GetSector();
    	ASSERT(pSector);
    	int i = pSector->m_Teleports.FindPtr(this);
    	ASSERT( i>=0 );
    	pSector->m_Teleports = NULL;	// we are just moving it.
    	pSector->m_Teleports.DeleteAt(i);
    	*(this) = pt;
    
    	RealizeTeleport();
    }
    
    #endif
    
    bool CTeleport::RealizeTeleport()
    {
    	if ( ! IsCharValid() || ! m_ptDst.IsCharValid())
    	{
    		DEBUG_ERR(( "CTeleport bad coords %s\n", WriteUsed() ));
    		return false;
    	}
    	CSector * pSector = GetSector();
    	ASSERT(pSector);
    	return pSector->AddTeleport( this );
    }
    
    //*************************************************************************
    // -CGRegion
    
    CGRegion::CGRegion()
    {
    	m_rectUnion.SetRectEmpty();
    }
    
    bool CGRegion::RealizeRegion()
    {
    	if ( IsRegionEmpty())
    		return( false );
    	return( true );
    }
    
    int CGRegion::GetRegionRectCount() const
    {
    	// How many rectangles in this region ?
    	int iQty = m_Rects.GetCount();
    	if ( ! iQty )
    	{
    		if ( ! IsRegionEmpty())
    			return 1;
    	}
    	return( iQty );
    }
    
    CGRect & CGRegion::GetRegionRect(int i)
    {
    	// Get a particular rectangle.
    	int iQty = m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		return m_rectUnion;
    	}
    	return( m_Rects.ElementAt(i));
    }
    
    const CGRect & CGRegion::GetRegionRect(int i) const
    {
    	int iQty = m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		return m_rectUnion;
    	}
    	return( m_Rects.ElementAt(i));
    }
    
    CPointBase CGRegion::GetRegionCorner( DIR_TYPE dir ) const
    {
    	// NOTE: DIR_QTY = center.
    	return( m_rectUnion.GetRectCorner(dir));
    }
    
    bool CGRegion::IsInside2d( const CPointBase & pt ) const
    {
    	if ( ! m_rectUnion.IsInside2d( pt ))
    		return( false );
    	int iQty = m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		return( true );
    	}
    	for ( int i=0; i<iQty; i++ )
    	{
    		if ( m_Rects.IsInside2d( pt ))
    			return( true );
    	}
    	return( false );
    }
    
    bool CGRegion::AddRegionRect( const CGRect & rect )
    {
    	ASSERT(!rect.IsRectEmpty());
    	int iQty = m_Rects.GetCount();
    	if ( ! iQty && IsRegionEmpty())
    	{
    		m_rectUnion = rect;
    	}
    	else
    	{
    		// Make sure it is not inside or equal to a previous rect !
    		for ( int j=0; j<iQty; j++ )
    		{
    			if ( rect.IsInside( m_Rects ))
    				return( true );
    		}
    
    		if ( ! iQty )
    		{
    			if ( rect.IsInside( m_rectUnion ))
    				return( true );
    			m_Rects.Add( m_rectUnion );
    		}
    
    		m_Rects.Add( rect );
    		m_rectUnion.UnionRect( rect );
    	}
    	return( true );
    }
    
    void CGRegion::RemoveRegionRect( int i )
    {
    	// Only used in GRAY_MAP really.
    	// Get rid of this rectangle.
    	m_rectUnion.SetRectEmpty();
    
    	int iQty = m_Rects.GetCount();
    	if ( iQty < 0 || i >= iQty )	// should never happen.
    	{
    		DEBUG_ERR(("Bad rect!\n"));
    		return;
    	}
    	if ( iQty == 0 )	// nothing here now.
    		return;
    
    	m_Rects.RemoveAt(i);
    	iQty --;
    	// re-compute the union rect.
    	for ( i=0; i<iQty; i++ )
    	{
    		m_rectUnion.UnionRect( m_Rects );
    	}
    
    	if ( iQty == 1 )
    	{
    		m_Rects.RemoveAt(0);
    	}
    }
    
    bool CGRegion::IsOverlapped( const CGRect & rect ) const
    {
    	// Does the region overlap this rectangle.
    	if ( ! m_rectUnion.IsOverlapped( rect ))
    		return( false );
    	int iQty = m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		return( true );
    	}
    	for ( int i=0; i<iQty; i++ )
    	{
    		if ( rect.IsOverlapped( m_Rects ))
    			return( true );
    	}
    	return( false );
    }
    
    bool CGRegion::IsInside( const CGRect & rect ) const
    {
    	// NOTE: This is NOT 100% true !!
    
    	if ( ! m_rectUnion.IsInside( rect ))
    		return( false );
    
    	int iQty = m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		return( true );
    	}
    
    	for ( int i=0; i<iQty; i++ )
    	{
    		if ( m_Rects.IsInside( rect ))
    			return( true );
    	}
    
    	return( false );
    }
    
    bool CGRegion::IsInside( const CGRegion * pRegionTest ) const
    {
    	// This is a rather hard test to make.
    	// Is the pRegionTest completely inside this region ?
    
    	if ( ! m_rectUnion.IsInside( pRegionTest->m_rectUnion ))
    		return( false );
    
    	int iQtyTest = pRegionTest->m_Rects.GetCount();
    	for ( int j=0; j<iQtyTest; j++ )
    	{
    		if ( ! IsInside( pRegionTest->m_Rects ))
    			return( false );
    	}
    
    	return( true );
    }
    
    bool CGRegion::IsOverlapped( const CGRegion * pRegionTest ) const
    {
    	// Does the region overlap this rectangle.
    	if ( ! m_rectUnion.IsOverlapped( pRegionTest->m_rectUnion ))
    		return( false );
    	int iQty = m_Rects.GetCount();
    	int iQtyTest = pRegionTest->m_Rects.GetCount();
    	if ( iQty == 0 )
    	{
    		if ( iQtyTest == 0 )
    			return( true );
    		return( pRegionTest->IsOverlapped(m_rectUnion));
    	}
    	if ( iQtyTest == 0 )
    	{
    		return( IsOverlapped(pRegionTest->m_rectUnion));
    	}
    	for ( int j=0; j<iQty; j++ )
    	{
    		for ( int i=0; i<iQtyTest; i++ )
    		{
    			if ( m_Rects.IsOverlapped( pRegionTest->m_Rects ))
    				return( true );
    		}
    	}
    	return( false );
    }
    
    bool CGRegion::IsEqualRegion( const CGRegion * pRegionTest ) const
    {
    	// Find dupe rectangles.
    	if ( ! m_rectUnion.IsEqual( pRegionTest->m_rectUnion ))
    		return( false );
    
    	int iQty = m_Rects.GetCount();
    	int iQtyTest = pRegionTest->m_Rects.GetCount();
    	if ( iQty != iQtyTest )
    		return( false );
    
    	for ( int j=0; j<iQty; j++ )
    	{
    		for ( int i=0; true; i++ )
    		{
    			if ( i>=iQtyTest )
    				return( false );
    			if ( m_Rects.IsEqual( pRegionTest->m_Rects ))
    				break;
    		}
    	}
    	return( true );
    }
    
    //*************************************************************************
    // -CRegionBase
    
    CRegionBase::CRegionBase( RESOURCE_ID rid, LPCTSTR pszName ) :
    	CResourceDef( rid )
    {
    	m_dwFlags = 0;
    	m_RegionClass = RegionClass_Unspecified;
    	m_iLinkedSectors = 0;
    	if ( pszName )
    		SetName( pszName );
    }
    
    CRegionBase::~CRegionBase()
    {
    	// RemoveSelf();
    	UnRealizeRegion();
    }
    
    void CRegionBase::UnRealizeRegion()
    {
    	// remove myself from the world.
    	// used in the case of a ship where the region will move.
    
    	for ( int i=0; true; i++ )
    	{
    		CSector * pSector = GetSector(i);
    		if ( pSector == NULL )
    			break;
    		// Does the rect overlap ?
    		if ( ! IsOverlapped( pSector->GetRect()))
    			continue;
    		if ( pSector->UnLinkRegion( this ))
    			m_iLinkedSectors--;
    	}
    
    	ASSERT( ! m_iLinkedSectors );
    }
    
    bool CRegionBase::RealizeRegion()
    {
    	// Link the region to the world.
    	// RETURN: false = not valid.
    
    	if ( IsRegionEmpty())
    	{
    		return( false );
    	}
    
    	if ( ! m_pt.IsValidPoint())
    	{
    		m_pt = GetRegionCorner( DIR_QTY );	// center
    	}
    
    #if defined(GRAY_SVR) || defined(GRAY_MAP)
    
    	// Attach to all sectors that i overlap.
    	ASSERT( m_iLinkedSectors == 0 );
    	for ( int i=0; true; i++ )
    	{
    		CSector * pSector = GetSector(i);
    		if ( pSector == NULL )
    			break;
    		// Does the rect overlap ?
    		if ( ! IsOverlapped( pSector->GetRect()))
    			continue;
    
    		// Add it to the sector list. 
    		if ( ! pSector->LinkRegion( this ))
    			return( false );
    
    		m_iLinkedSectors++;
    	}
    		
    	ASSERT(m_iLinkedSectors);
    
    #endif
    
    	return( true );
    }
    
    bool CRegionBase::AddRegionRect( const CRectMap & rect )
    {
    	// Add an included rectangle to this region.
    	if ( ! rect.IsValid())
    	{
    		return( false );
    	}
    	if ( ! CGRegion::AddRegionRect( rect ))
    		return( false );
    
    	// Need to call RealizeRegion now.?
    	return( true );
    }
    
    void CRegionBase::SetName( LPCTSTR pszName )
    {
    #if defined(GRAY_SVR)
    	if ( pszName == NULL || pszName == '%' )
    	{
    		m_sName = g_Serv.GetName();
    	}
    	else
    #endif
    	{
    		m_sName = pszName;
    	}
    }
    
    enum RC_TYPE
    {
    	RC_ANNOUNCE,
    	RC_ARENA,
    	RC_BUILDABLE,
    	RC_CLASS,
    	RC_COLDCHANCE,
    	RC_FLAGS,
    	RC_GATE,
    	RC_GUARDED,
    	RC_MAGIC,
    	RC_MAGICDAMAGE,
    	RC_MAPPLANE,
    	RC_MARK,		// recall in teleport as well.
    	RC_NAME,
    	RC_NOBUILD,
    	RC_NODECAY,
    	RC_NOPVP,
    	RC_P,
    	RC_RAINCHANCE,
    	RC_RECALL,	// recall out
    	RC_RECALLIN,
    	RC_RECALLOUT,
    	RC_RECT,
    	RC_SAFE,
    	RC_TYPE_,
    	RC_UID,
    	RC_UNDERGROUND,
    	RC_QTY,
    };
    
    LPCTSTR const CRegionBase::sm_szLoadKeys =	// static (Sorted)
    {
    	_TEXT("ANNOUNCE"),
    	_TEXT("ARENA"),
    	_TEXT("BUILDABLE"),
    	_TEXT("CLASS"),
    	_TEXT("COLDCHANCE"),
    	_TEXT("FLAGS"),
    	_TEXT("GATE"),
    	_TEXT("GUARDED"),
    	_TEXT("MAGIC"),
    	_TEXT("MAGICDAMAGE"),
    	_TEXT("MAPPLANE"),
    	_TEXT("MARK"),		// recall in teleport as well.
    	_TEXT("NAME"),
    	_TEXT("NOBUILD"),
    	_TEXT("NODECAY"),
    	_TEXT("NOPVP"),
    	_TEXT("P"),
    	_TEXT("RAINCHANCE"),
    	_TEXT("RECALL"),	// recall out
    	_TEXT("RECALLIN"),
    	_TEXT("RECALLOUT"),
    	_TEXT("RECT"),
    	_TEXT("SAFE"),
    	_TEXT("TYPE"),
    	_TEXT("UID"),
    	_TEXT("UNDERGROUND"),
    	NULL,
    };
    
    bool CRegionBase::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
    {
    	RC_TYPE index = (RC_TYPE) FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
    	if ( index < 0 )
    	{
    		return( CScriptObj::r_WriteVal( pszKey, sVal, pSrc ));
    	}
    
    	switch ( index )
    	{
    	case RC_ANNOUNCE:
    		sVal.FormatVal( IsFlag(REGION_FLAG_ANNOUNCE));
    		break;
    	case RC_ARENA:
    		sVal.FormatVal( IsFlag(REGION_FLAG_ARENA));
    		break;
    	case RC_BUILDABLE:
    		sVal.FormatVal( ! IsFlag(REGION_FLAG_NOBUILDING));
    		break;
    	case RC_CLASS:
    		sVal.FormatVal( m_RegionClass );
    		break;
    	case RC_COLDCHANCE:
    		return( false );
    	case RC_FLAGS:
    		sVal.FormatHex( GetRegionFlags() );
    		break;
    	case RC_GATE:
    		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_GATE));
    		break;
    	case RC_GUARDED:
    		sVal.FormatVal( IsFlag(REGION_FLAG_GUARDED));
    		break;
    	case RC_MAGIC:
    		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_ALL));
    		break;
    	case RC_MAGICDAMAGE:
    		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_DAMAGE));
    		break;
    	case RC_MAPPLANE:
    		sVal.FormatVal( m_pt.m_mapplane );
    		break;
    	case RC_MARK:
    	case RC_RECALLIN:
    		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_RECALL_IN));
    		break;
    	case RC_NAME:
    		// The previous name was really the DEFNAME ???
    		sVal = GetName();
    		break;
    	case RC_NOBUILD:
    		sVal.FormatVal( IsFlag(REGION_FLAG_NOBUILDING));
    		break;
    	case RC_NODECAY:
    		sVal.FormatVal( IsFlag(REGION_FLAG_NODECAY));
    		break;
    	case RC_NOPVP:
    		sVal.FormatVal( IsFlag(REGION_FLAG_NO_PVP));
    		break;
    	case RC_P:
    		sVal = m_pt.WriteUsed();
    		break;
    	case RC_RAINCHANCE:
    		return( false );
    	case RC_RECALL:
    	case RC_RECALLOUT:
    		sVal.FormatVal( ! IsFlag(REGION_ANTIMAGIC_RECALL_OUT));
    		break;
    	case RC_RECT:
    		return( false );
    	case RC_SAFE:
    		sVal.FormatVal( IsFlag(REGION_FLAG_SAFE));
    		break;
    	case RC_TYPE_:
    	case RC_UID:
    		sVal.FormatHex( GetResourceID() );
    		break;
    	case RC_UNDERGROUND:
    		sVal.FormatVal( IsFlag(REGION_FLAG_UNDERGROUND));
    		break;
    	default:
    		DEBUG_CHECK(0);
    		return( false );
    	}
    
    	return( true );
    }
    
    bool CRegionBase::r_LoadVal( CScript & s )
    {
    	RC_TYPE index = (RC_TYPE) FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
    	if ( index < 0 )
    	{
    		// return( CScriptObj::r_LoadVal( s ));
    		return( false );
    	}
    	switch ( index )
    	{
    	case RC_ANNOUNCE:
    		TogRegionFlags( REGION_FLAG_ANNOUNCE, ( ! s.HasArgs()) || s.GetArgVal());
    		break;
    	case RC_ARENA:
    		TogRegionFlags( REGION_FLAG_ARENA, s.GetArgVal());
    		break;
    	case RC_BUILDABLE:
    		TogRegionFlags( REGION_FLAG_NOBUILDING, ! s.GetArgVal());
    		break;
    	case RC_CLASS:
    		m_RegionClass = (RegionClass_Type) s.GetArgVal();
    		break;
    #ifdef GRAY_SVR
    	case RC_COLDCHANCE:
    		SendSectorsVerb( s.GetKey(), s.GetArgStr(), &g_Serv );
    		break;
    #endif
    	case RC_FLAGS:
    		m_dwFlags = ( s.GetArgVal() &~REGION_FLAG_SHIP ) | ( m_dwFlags & REGION_FLAG_SHIP );
    		break;
    	case RC_GATE:
    		TogRegionFlags( REGION_ANTIMAGIC_GATE, ! s.GetArgVal());
    		break;
    	case RC_GUARDED:
    		TogRegionFlags( REGION_FLAG_GUARDED, ( ! s.HasArgs()) || s.GetArgVal());
    		break;
    	case RC_MAGIC:
    		TogRegionFlags( REGION_ANTIMAGIC_ALL, ! s.GetArgVal());
    		break;
    	case RC_MAGICDAMAGE:
    		TogRegionFlags( REGION_ANTIMAGIC_DAMAGE, ! s.GetArgVal());
    		break;
    	case RC_MAPPLANE:
    		m_pt.m_mapplane = s.GetArgVal();
    		break;
    	case RC_MARK:
    	case RC_RECALLIN:
    		TogRegionFlags( REGION_ANTIMAGIC_RECALL_IN, ! s.GetArgVal());
    		break;
    	case RC_NAME:
    		SetName( s.GetArgStr());
    		break;
    	case RC_NODECAY:
    		TogRegionFlags( REGION_FLAG_NODECAY, s.GetArgVal());
    		break;
    	case RC_NOBUILD:
    		TogRegionFlags( REGION_FLAG_NOBUILDING, s.GetArgVal());
    		break;
    	case RC_NOPVP:
    		TogRegionFlags( REGION_FLAG_NO_PVP, s.GetArgVal());
    		break;
    	case RC_P:
    		m_pt.Read(s.GetArgStr());
    		break;
    #ifdef GRAY_SVR
    	case RC_RAINCHANCE:
    		SendSectorsVerb( s.GetKey(), s.GetArgStr(), &g_Serv );
    		break;
    #endif
    	case RC_RECALL:
    	case RC_RECALLOUT:
    		TogRegionFlags( REGION_ANTIMAGIC_RECALL_OUT, ! s.GetArgVal());
    		break;
    	case RC_RECT:
    		{
    			CRectMap rect;
    			rect.Read(s.GetArgStr());
    			return( AddRegionRect( rect ));
    		}
    	case RC_SAFE:
    		TogRegionFlags( REGION_FLAG_SAFE, s.GetArgVal());
    		break;
    	case RC_TYPE_:
    		return( false );
    	case RC_UNDERGROUND:
    		TogRegionFlags( REGION_FLAG_UNDERGROUND, s.GetArgVal());
    		break;
    	default:
    		DEBUG_CHECK(0);
    		return( false );
    	}
    	return( true );
    }
    
    void CRegionBase::r_WriteBody( CScript & s, LPCTSTR pszPrefix )
    {
    	CGString sName;
    	if ( m_RegionClass != RegionClass_Unspecified )
    	{
    		sName.Format( _TEXT("%sCLASS"), (LPCTSTR) pszPrefix );
    		s.WriteKeyVal( sName, m_RegionClass);
    	}
    	if ( GetRegionFlags())
    	{
    		sName.Format( _TEXT("%sFLAGS"), (LPCTSTR) pszPrefix );
    		s.WriteKeyHex( sName, GetRegionFlags());
    	}
    }
    
    #ifdef GRAY_MAP
    
    void CRegionBase::r_WriteBase( CScript &s )
    {
    	if ( GetResourceName() && GetResourceName() )
    	{
    		s.WriteKey( _TEXT("DEFNAME"), GetResourceName() );
    	}
    	if ( m_pt.IsValidPoint())
    	{
    		s.WriteKey( _TEXT("P"), m_pt.WriteUsed());
    	}
    	else if ( m_pt.m_mapplane )
    	{
    		s.WriteKeyVal( _TEXT("MAPPLANE"), m_pt.m_mapplane );
    	}
    	int iQty = GetRegionRectCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		s.WriteKey( _TEXT("RECT"), GetRegionRect(i).Write());
    	}
    }
    
    void CRegionBase::r_Write( CScript &s )
    {
    	s.WriteSection( "ROOM %s", GetName());
    	r_WriteBase( s );
    	r_WriteBody( s, "" );
    }
    
    #endif
    
    bool CRegionBase::IsGuarded() const
    {
    	// Safe areas do not need guards.
    	return( IsFlag( REGION_FLAG_GUARDED ) && ! IsFlag( REGION_FLAG_SAFE ));
    }
    
    bool CRegionBase::CheckAntiMagic( SPELL_TYPE spell ) const
    {
    	// return: true = blocked.
    	if ( ! IsFlag( REGION_FLAG_SHIP |
    		REGION_ANTIMAGIC_ALL |
    		REGION_ANTIMAGIC_RECALL_IN |
    		REGION_ANTIMAGIC_RECALL_OUT |
    		REGION_ANTIMAGIC_GATE |
    		REGION_ANTIMAGIC_TELEPORT |
    		REGION_ANTIMAGIC_DAMAGE ))	// no effects on magic anyhow.
    		return( false );
    
    	if ( IsFlag( REGION_ANTIMAGIC_ALL ))
    		return( true );
    
    	if ( IsFlag( REGION_ANTIMAGIC_RECALL_IN | REGION_FLAG_SHIP ))
    	{
    		if ( spell == SPELL_Mark || spell == SPELL_Gate_Travel )
    			return( true );
    	}
    	if ( IsFlag( REGION_ANTIMAGIC_RECALL_OUT ))
    	{
    		if ( spell == SPELL_Recall || spell == SPELL_Gate_Travel || spell == SPELL_Mark )
    			return( true );
    	}
    	if ( IsFlag( REGION_ANTIMAGIC_GATE ))
    	{
    		if ( spell == SPELL_Gate_Travel )
    			return( true );
    	}
    	if ( IsFlag( REGION_ANTIMAGIC_TELEPORT ))
    	{
    		if ( spell == SPELL_Teleport )
    			return( true );
    	}
    #ifdef GRAY_SVR
    	if ( IsFlag( REGION_ANTIMAGIC_DAMAGE ))
    	{
    		const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
    		ASSERT(pSpellDef);
    		if ( pSpellDef->IsSpellType( SPELLFLAG_HARM ))
    			return( true );
    	}
    #endif
    	return( false );
    }
    
    LPCTSTR const CRegionBase::sm_szVerbKeys =
    {
    	_TEXT("ALLCLIENTS"),
    	_TEXT("CLIENTS"),
    	_TEXT("SECTORS"),
    	NULL,
    };
    
    #ifdef GRAY_SVR	// server only.
    
    bool CRegionBase::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
    {
    	int index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF( sm_szVerbKeys )-1 );
    	switch (index)
    	{
    	case 0:	// ALLCLIENTS
    	case 1:	// CLIENTS
    	{
    		for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
    		{
    			CChar * pChar = pClient->GetChar();
    			if ( pChar == NULL )
    				continue;
    			if ( pChar->GetRegion() != this )
    				continue;
    			pChar->r_Verb( CScript( s.GetArgStr()), pSrc );
    		}
    	}
    	case 2:	// SECTORS
    	{
    		TCHAR * pszArg;
    		Str_Parse( s.GetArgStr(), &pszArg );
    		return SendSectorsVerb( s.GetArgStr(), pszArg, pSrc );
    	}
    	}
    	return( CScriptObj::r_Verb( s, pSrc ));
    }
    
    bool CRegionBase::SendSectorsVerb( LPCTSTR pszVerb, LPCTSTR pszArgs, CTextConsole * pSrc )
    {
    	// Send a command to all the CSectors in this region.
    
    	bool fRet = false;
    	for ( int i=0; true; i++ )
    	{
    		CSector * pSector = GetSector(i);
    		if ( pSector == NULL )
    			break;
    		// Does the rect overlap ?
    		if ( IsOverlapped( pSector->GetRect()))
    		{
    			fRet |= pSector->r_Verb( CScript( pszVerb, pszArgs ), pSrc );
    		}
    	}
    	return( fRet );
    }
    
    #endif // GRAY_SVR
    
    //*************************************************************************
    // -CRegionWorld
    
    CRegionWorld::CRegionWorld( RESOURCE_ID rid, LPCTSTR pszName ) :
    	CRegionBase( rid, pszName )
    {
    }
    
    CRegionWorld::~CRegionWorld()
    {
    }
    
    #if defined(GRAY_SVR) || defined(GRAY_MAP)
    
    enum RWC_TYPE
    {
    	RWC_DEFNAME,
    	RWC_EVENTS,
    	RWC_RESOURCES,
    	RWC_TAG,
    	RWC_QTY,
    };
    
    LPCTSTR const CRegionWorld::sm_szLoadKeys =	// static
    {
    	_TEXT("DEFNAME"),
    	_TEXT("EVENTS"),
    	_TEXT("RESOURCES"),
    	_TEXT("TAG"),
    	NULL,
    };
    
    bool CRegionWorld::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
    {
    	// if ( ! strnicmp( pszKey, "SECTOR.", 7 ))
    	return( CRegionBase::r_GetRef( pszKey, pRef ));
    }
    
    bool CRegionWorld::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
    {
    	switch ( FindTableHeadSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
    	{
    	case RWC_DEFNAME: // "DEFNAME" = for the speech system.
    		sVal = GetResourceName();
    		break;
    #ifdef GRAY_SVR
    	case RWC_RESOURCES:
    	case RWC_EVENTS:
    		m_Events.WriteResourceRefList( sVal );
    		break;
    	case RWC_TAG:
    		// "TAG" = get/set a local tag.
    		if ( pszKey != '.' )
    			return( false );
    		pszKey += 4;
    		sVal = m_TagDefs.FindKeyStr(pszKey);
    		return( true );
    #endif
    	default:
    		return( CRegionBase::r_WriteVal( pszKey, sVal, pSrc ));
    	}
    	return( true );
    }
    
    bool CRegionWorld::r_LoadVal( CScript &s )
    {
    	// Load the values for the region from script.
    	switch ( FindTableHeadSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
    	{
    	case RWC_DEFNAME: // "DEFNAME" = for the speech system.
    		return SetResourceName( s.GetArgStr());
    #ifdef GRAY_SVR
    	case RWC_RESOURCES:
    	case RWC_EVENTS:
    		return( m_Events.r_LoadVal( s, RES_REGIONTYPE ));
    	case RWC_TAG:
    		// Load any extra TAGS here.
    		m_TagDefs.SetStr( s.GetKey()+4, s.GetArgStr());
    		return( true );
    #endif
    	default:
    		return( CRegionBase::r_LoadVal( s ));
    	}
    
    	return( true );
    }
    
    void CRegionWorld::r_WriteBody( CScript &s, LPCTSTR pszPrefix )
    {
    	CRegionBase::r_WriteBody( s, pszPrefix );
    
    #ifdef GRAY_SVR
    	CGString sName;
    
    	if ( m_Events.GetCount())
    	{
    		CGString sVal;
    		m_Events.WriteResourceRefList( sVal );
    		sName.Format( _TEXT("%sEVENTS"), (LPCTSTR) pszPrefix ); 
    		s.WriteKey( sName, sVal );
    	}
    
    	// Write out any extra TAGS here.
    	for ( int j=0; j<m_TagDefs.GetCount(); j++ )
    	{
    		sName.Format( _TEXT("%sTAG.%s"), (LPCTSTR) pszPrefix, (LPCTSTR)( m_TagDefs->GetKey()));
    		s.WriteKey( sName, m_TagDefs->GetValStr());
    	}
    #endif
    
    }
    
    #ifdef GRAY_MAP 
    
    void CRegionWorld::r_Write( CScript &s )
    {
    	s.WriteSection( "AREA %s", GetName());
    	r_WriteBase( s );
    	r_WriteBody( s, "" );
    }
    
    #endif	// GRAY_MAP
    #endif
    
    #ifdef GRAY_SVR	// server only.
    
    bool CRegionWorld::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
    {
    	if ( s.IsKey( "TAGLIST" ))
    	{
    		m_TagDefs.DumpKeys( pSrc, "TAG." );
    		return( true );
    	}
    	return( CRegionBase::r_Verb( s, pSrc ));
    }
    
    LPCTSTR const CRegionWorld::sm_szTrigName =	// static
    {
    	_TEXT("@AAAUNUSED"),
    	_TEXT("@CLIPERIODIC"),
    	_TEXT("@ENTER"),
    	_TEXT("@EXIT"),
    	_TEXT("@REGPERIODIC"),
    	_TEXT("@STEP"),
    };
    
    TRIGRET_TYPE CRegionWorld::OnRegionTrigger( CTextConsole * pSrc, RTRIG_TYPE iAction )
    {
    	// RETURN: true = halt prodcessing (don't allow in this region
    	// 
    	// ONTRIGGER=ENTER
    	// ONTRIGGER=EXIT
    
    	int iQty = m_Events.GetCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		CResourceLink * pLink = m_Events;
    		ASSERT(pLink);
    		if ( pLink->GetResType() != RES_REGIONTYPE )
    			continue;
    		if ( ! pLink->HasTrigger( iAction ))
    			continue;
    		CResourceLock s;
    		if ( ! pLink->ResourceLock( s ))
    			continue;
    		TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, sm_szTrigName, pSrc );
    		if ( iRet != TRIGRET_RET_FALSE )
    			return iRet;
    	}
    
    	return( TRIGRET_RET_DEFAULT );
    }
    
    const CRandGroupDef * CRegionWorld::FindNaturalResource( int /*IT_TYPE*/ type ) const
    {
    	// Find the natural resources assinged to this region.
    	// ARGS: type = IT_TYPE
    
    	int iQty = m_Events.GetCount();
    	for ( int i=0; i<iQty; i++ )
    	{
    		CResourceLink * pLink = m_Events;
    		ASSERT(pLink);
    		if ( pLink->GetResType() != RES_REGIONTYPE )
    			continue;
    		if ( pLink->GetResPage() == type )
    		{
    			return( dynamic_cast <const CRandGroupDef *>( pLink ));
    		}
    	}
    	return( NULL );
    }
    
    #endif	// GRAY_SVR