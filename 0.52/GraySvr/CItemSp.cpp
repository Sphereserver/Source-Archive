//
// CItemSp.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

/////////////////////////////////////////////////////////////////////////////

int CItem::Spawn_GetName( TCHAR * pszOut ) const
{
	int id;
	const TCHAR * pszName = "?";

	if ( m_type == ITEM_SPAWN_ITEM )
	{
		id = m_itSpawnItem.m_ItemID;
		CItemBase * pDef = CItemBase::FindItemBase( m_itSpawnItem.m_ItemID );
		if ( pDef )
		{
			pszName = pDef->GetTypeName();
		}
	}
	else
	{
		DEBUG_CHECK( m_type == ITEM_SPAWN_CHAR );

		// Name the spawn type.
		id = m_itSpawnChar.m_CharID;

		CCharBase * pDef = CCharBase::FindCharBase( m_itSpawnChar.m_CharID );
		if ( pDef != NULL )
		{	
			pszName = pDef->GetTradeName();
		}

		else if ( g_Serv.m_SpawnGroupDefs.IsValidIndex( id - SPAWNTYPE_START ))
		{
			pszName = g_Serv.m_SpawnGroupDefs[ id - SPAWNTYPE_START ]->GetName();
			if ( pszName[0] == '\0' ) 
				pszName = "?";
		}
	}

	return sprintf( pszOut, " (%x=%s)", id, pszName );
}

void CItem::Spawn_OnTick( bool fExec )
{
	int iMinutes;
	if ( m_itSpawnChar.m_TimeHiMin <= 0 )
	{
		iMinutes = GetRandVal(30) + 1;
	}
	else
	{
		iMinutes = min( m_itSpawnChar.m_TimeHiMin, m_itSpawnChar.m_TimeLoMin ) + GetRandVal( abs( m_itSpawnChar.m_TimeHiMin - m_itSpawnChar.m_TimeLoMin ));
	}

	if ( iMinutes <= 0 ) iMinutes = 1;
	SetTimeout( iMinutes * 60 * TICK_PER_SEC );	// set time to check again.

	if ( ! fExec )
		return;

	if ( m_type == ITEM_SPAWN_ITEM )
	{
		// Count how many items are here already.
		// This could be in a container.
		int iCount = 0;
		CItemContainer * pCont = dynamic_cast <CItemContainer *>( GetParent());
		if ( pCont != NULL )
		{
			iCount = pCont->ContentCount( m_itSpawnItem.m_ItemID );
		}
		else
		{
			// If is equipped this will produce the item where you are standing.
			CPointMap pt = GetTopLevelObj()->GetTopPoint();
			CWorldSearch AreaItems( pt, m_itSpawnItem.m_DistMax );
			while (true)
			{
				CItem * pItem = AreaItems.GetItem();
				if ( pItem == NULL )
					break;
				if ( pItem->m_type == ITEM_SPAWN_ITEM )
					continue;
				if ( pItem->IsAttr( ATTR_INVIS ))
					continue;
				if ( pItem->GetID() != m_itSpawnItem.m_ItemID )
					continue;
				// if ( pItem->m_uidLink != GetUID()) continue;
				iCount += pItem->GetAmount();
			}
		}
		if ( iCount >= GetAmount())
			return;
		// ??? What if I want to spawn multiple items in the area ?
		CItem * pItem = CreateTemplate( m_itSpawnItem.m_ItemID );
		if ( pItem )
		{
			pItem->m_Attr |= ( m_Attr & ( ATTR_OWNED | ATTR_MOVE_ALWAYS ));
			if ( pItem->m_pDef->IsStackableType())
			{
				int amount = m_itSpawnItem.m_pile;
				if ( amount == 0 || amount > GetAmount())
					amount = GetAmount();
				pItem->SetAmount( GetRandVal(amount) + 1 );
			}
			// pItem->m_uidLink = GetUID();	// This might be dangerous ?
			pItem->SetDecayTime( g_Serv.m_iDecay_Item );	// It will decay eventually to be replaced later.
			pItem->MoveNearObj( this, m_itSpawnItem.m_DistMax );
		}
		return;
	}

	ASSERT( m_type == ITEM_SPAWN_CHAR );

	if ( ! IsTopLevel())
		return;	// creatures can only be top level.
	if ( m_itSpawnChar.m_current >= GetAmount())
		return;

	CREID_TYPE id = m_itSpawnChar.m_CharID;
	if ( g_Serv.m_SpawnGroupDefs.IsValidIndex( id - SPAWNTYPE_START ))
	{
		const CRandGroupDef * pSpawnGroup = g_Serv.m_SpawnGroupDefs[id - SPAWNTYPE_START];
		int i = pSpawnGroup->GetRandMemberIndex();
		if ( i >= 0 )
		{
			id = (CREID_TYPE) pSpawnGroup->GetMemberVal(i);
		}
	}

	// Kind of wastfull but check it for validness.
	if ( CCharBase::FindCharBase( id ) == NULL )
	{
		DEBUG_ERR(( "Bad Spawn point uid=0%lx, id=%0x\n", GetUID(), id ));
		return;
	}

	if ( GetTopSector()->GetComplexity() > g_Serv.m_iMaxComplexity )
	{
		DEBUG_MSG(( "Spawn point uid=0%lx too complex (%d)\n", GetUID(), g_Serv.m_iMaxComplexity ));
		return;
	}

	CChar * pChar = CChar::CreateNPC( id );
	if ( pChar == NULL )
		return;
	m_itSpawnChar.m_current ++;
	pChar->Memory_AddObjTypes( this, MEMORY_ISPAWNED );
	// Move to spot "near" the spawn item.
	pChar->MoveNearObj( this, m_itSpawnChar.m_DistMax );
	if ( m_itSpawnChar.m_DistMax )
	{
		pChar->m_Home = GetTopPoint();
		pChar->m_pNPC->m_Home_Dist_Wander = m_itSpawnChar.m_DistMax;
	}
	pChar->Update();
}

void CItem::Spawn_KillChildren()
{
	// kill all creatures spawned from this !
	DEBUG_CHECK( m_type == ITEM_SPAWN_CHAR );
	for ( int i=0; i<SECTOR_QTY; i++ )
	{
		CChar * pCharNext;
		CChar * pChar = STATIC_CAST <CChar*>( g_World.m_Sectors[i].m_Chars.GetHead());
		for ( ; pChar!=NULL; pChar = pCharNext )
		{
			pCharNext = pChar->GetNext();
			if ( pChar->NPC_IsSpawnedBy( this ))
			{
				pChar->Delete();
			}
		}
	}
	m_itSpawnChar.m_current = 0;	// Should not be necassary
	Spawn_OnTick( false );
}

CCharBase * CItem::Spawn_SetTrackID()
{
	DEBUG_CHECK( m_type == ITEM_SPAWN_CHAR );

	CCharBase * pDef = NULL;
	if ( m_itSpawnChar.m_CharID < SPAWNTYPE_START )
	{
		pDef = CCharBase::FindCharBase( m_itSpawnChar.m_CharID );
		if ( pDef == NULL )
		{
			m_itSpawnChar.m_CharID = CREID_WISP;
		}
	}
	if ( IsAttr(ATTR_INVIS))	// They must want it to look like this.
	{
		m_id = ( pDef == NULL ) ? ITEMID_TRACK_WISP : pDef->m_trackID;
		SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is invis.
	}
	return( pDef );
}

/////////////////////////////////////////////////////////////////////////////

const CItemBaseMulti * CItemMulti::Multi_GetDef( ITEMID_TYPE id ) // static
{
	return( dynamic_cast <const CItemBaseMulti *> ( CItemBase::FindItemBase(id)));
}

bool CItemMulti::MultiAddRegion()
{
	// Add/move a region for the multi so we know when we are in it.

	DEBUG_CHECK( m_type == ITEM_MULTI || m_type == ITEM_SHIP );
	ASSERT( IsTopLevel());

	CPointMap pt = GetTopPoint();
	if ( ! pt.IsValid())
	{
		DEBUG_CHECK( pt.IsValid());
		return false;
	}

	const CItemBaseMulti * pMulti = Multi_GetDef();
	if ( pMulti == NULL )
	{
		DEBUG_ERR(( "Bad Multi type 0%x, uid=0%x\n", GetID(), GetUID()));
		return false;
	}

	bool fPrvValid = m_itShip.m_last_p.IsValid();
	if ( ! fPrvValid )	// this will happen when first placed.
	{
		m_itShip.m_last_p = pt;
	}

	// Get Background region.
	CRegionWorld * pRegionBack = dynamic_cast <CRegionWorld*> (pt.GetRegion( REGION_TYPE_AREA ));
	ASSERT( pRegionBack );

	// Get previous region.
	CRegionWorld * pRegion = dynamic_cast <CRegionWorld*>( m_itShip.m_last_p.GetRegion( GetUID()));
	if ( pRegion == NULL )	// create new region if need be.
	{
		if ( fPrvValid )
		{
			// DEBUG_ERR(( "Multi invalid region ? uid=0%x\n", GetUID()));
		}
		pRegion = new CRegionWorld( GetUID());
	}
	ASSERT( pRegionBack != pRegion );

	m_itShip.m_last_p = pt;	// save this in case we moved

	CRectMap rect;
	*(static_cast<CGRect*>(&rect)) = pMulti->m_rect;
	rect.OffsetRect( pt.m_x, pt.m_y );

	DWORD dwFlags;
	if ( m_type == ITEM_SHIP )
	{
		dwFlags = REGION_FLAG_SHIP;
	}
	else
	{
		// Houses get some of the attribs of the land around it.
		dwFlags = pRegionBack->GetFlags() &~ REGION_FLAG_GLOBALNAME;
	}
	dwFlags |= REGION_DYNAMIC | pMulti->m_dwRegionFlags;
	pRegion->ModFlags( dwFlags );

	CGString sName;
	sName.Format( "%s (%s)", pRegionBack->GetName(), GetName());
	pRegion->SetName( sName );
	pRegion->m_p = pt;
	pRegion->SetRegionRect( rect );
	return( true );
}

void CItemMulti::MultiRemoveRegion()
{
	DEBUG_CHECK( m_type == ITEM_MULTI || m_type == ITEM_SHIP );
	CRegionWorld * pRegion = dynamic_cast <CRegionWorld*>( m_itShip.m_last_p.GetRegion( GetUID()));
	if ( pRegion == NULL )
	{
		return;
	}

	// find all creatures in the region and remove this from them.
	CWorldSearch Area( m_itShip.m_last_p, UO_MAP_VIEW_SIZE );
	while (true)
	{
		CChar * pChar = Area.GetChar();
		if ( pChar == NULL )
			break;
		if ( pChar->m_pArea != pRegion )
			continue;
		pChar->MoveToRegionReTest( REGION_TYPE_AREA );
	}

	delete pRegion;
}

bool CItemMulti::Multi_CreateComponent( ITEMID_TYPE id, int dx, int dy, int dz )
{
	CItem * pItem = CreateTemplate( id );
	ASSERT(pItem);

	CPointMap pt = GetTopPoint();
	pt.m_x += dx;
	pt.m_y += dy;
	pt.m_z += dz;

	pItem->m_Attr |= ATTR_MOVE_NEVER | (m_Attr&(ATTR_MAGIC|ATTR_INVIS));
	pItem->SetColor( GetColor());
	pItem->m_uidLink = GetUID();	// lock it down with the house.

	bool fLocked = false;
	switch ( pItem->m_type )
	{
	case ITEM_KEY:	// hmm this probly does not happen.
		pItem->m_uidLink.ClearUID();	// don't lock this down.
		break;
	case ITEM_DOOR:
		pItem->m_type = ITEM_DOOR_LOCKED;
		fLocked = true;
		break;
	case ITEM_CONTAINER:
		// This includes ship holds.
		pItem->m_type = ITEM_CONTAINER_LOCKED;
		fLocked = true;
		break;
	case ITEM_SHIP_SIDE:
		pItem->m_type = ITEM_SHIP_SIDE_LOCKED;
		fLocked = true;
		break;
	}

	if ( fLocked )
	{
		pItem->m_itContainer.m_lockUID = GetUID();	// Set the key id for the door/key/sign.
		pItem->m_itContainer.m_lock_complexity = 10000;	// never pickable.
	}

	pItem->MoveTo( pt );
	return( fLocked );
}

void CItemMulti::Multi_Create( CChar * pChar )
{
	// Create house or Ship extra stuff.
	// NOTE: This can only be done after the house is given location.

	const CItemBaseMulti * pMulti = Multi_GetDef();
	if ( pMulti == NULL ||
		! IsTopLevel())
		return;

	// We are top level.

	// ??? SetTimeout( GetDecayTime()); house decay ?

	bool fNeedKey = false;
	for ( int i=0; i<pMulti->m_Components.GetCount(); i++ )
	{
		fNeedKey |= Multi_CreateComponent( (ITEMID_TYPE) pMulti->m_Components[i].m_id, 
			pMulti->m_Components[i].m_dx,
			pMulti->m_Components[i].m_dy,
			pMulti->m_Components[i].m_dz );
	}

#if 0
	const CGrayMulti * pMulti = g_World.GetMultiItemDefs( GetDispID());
	if ( pMulti )
	{
		int iQty = pMulti->GetItemCount();
		for ( int i=0; iQty--; i++ )
		{
			const CUOMultiItemRec * pMultiItem = pMulti->GetItem(i);
			ASSERT(pMultiItem);
			if ( pMultiItem->m_visible )	// client side.
				continue;
			fNeedKey |= Multi_CreateComponent( (ITEMID_TYPE) pMultiItem->m_wTileID, 
				pMultiItem->m_dx,
				pMultiItem->m_dy,
				pMultiItem->m_dz );
		}
	}
#endif

	if ( ! fNeedKey ) 
		return;

	// Create the key to the door.
	ITEMID_TYPE id = IsAttr(ATTR_MAGIC) ? ITEMID_KEY_MAGIC : ITEMID_KEY_COPPER ;
	CItem * pKey = CreateScript(id);
	pKey->m_type = ITEM_KEY;
	pKey->m_Attr |= (m_Attr&ATTR_MAGIC);
	pKey->m_itKey.m_lockUID = GetUID();
	// pKey->m_uidLink = GetUID();	// don't lock it down !

	if ( pChar != NULL )
	{
		m_itShip.m_UIDCreator = pChar->GetUID();

		if ( g_Serv.m_fAutoNewbieKeys )
			pKey->m_Attr |= ATTR_NEWBIE;
		// Put in your pack
		pChar->GetPackSafe()->ContentAdd( pKey );

		// Put dupe key in the bank.
		pKey = CreateDupeItem( pKey );
		pChar->GetBank()->ContentAdd( pKey );
		pChar->SysMessage( "The duplicate key is in your bank account" );
	}
	else
	{
		// Just put the key on the front step ?
		DEBUG_CHECK( 0 );
	}
}

bool CItemMulti::Multi_IsPartOf( const CItem * pItem ) const
{
	// Assume it is in my area test already.
	ASSERT( pItem );
	if ( pItem == this )
		return( true );
	if ( pItem->m_uidLink == GetUID())
		return( true );
	return( pItem->m_itContainer.m_lockUID == GetUID());	// this might just be the key on the ground ?
}

CItem * CItemMulti::Multi_FindItem( ITEM_TYPE type )
{
	// Find a prt of this multi nearby.
	if ( ! IsTopLevel())
		return( NULL );

	CWorldSearch Area( GetTopPoint(), UO_MAP_VIEW_SIZE );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			return( NULL );
		if ( ! Multi_IsPartOf( pItem ))
			continue;
		if ( pItem->IsType( type ))
			return( pItem );
	}
}

CItemMulti::~CItemMulti()
{
	DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	// NOTE: ??? This is dangerous to iterators. The "next" item may no longer be valid !
	Multi_Delete();
}

bool CItemMulti::OnTick()
{
	if ( m_type == ITEM_SHIP )
	{
		// Ships move on their tick.
		if ( Ship_OnMoveTick())
			return true;
	}

	return true;
}

bool CItemMulti::MoveTo( CPointMap pt ) // Put item on the ground here.
{
	// Move this item to it's point in the world. (ground/top level)
	if ( ! CItem::MoveTo(pt))
		return false;

	// Multis need special region info to track when u are inside them.
	// Add new region info.
	MultiAddRegion();
	return( true );
}

void CItemMulti::Multi_Delete()
{
	// Attempt to remove all the accessory junk.
	// NOTE: assume we have already been removed from Top Level

	if ( ! m_itShip.m_last_p.IsValid()) 
		return;

	CWorldSearch Area( m_itShip.m_last_p, UO_MAP_VIEW_SIZE );	// largest area.
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			break;
		if ( pItem == this )	// this gets deleted seperately.
			continue;
		if ( ! Multi_IsPartOf( pItem ))
			continue;
		pItem->Delete();	// delete the key id for the door/key/sign.
	}

	MultiRemoveRegion();
}

#ifdef COMMENT

bool CItemMulti::Multi_DeedConvert( CChar * pChar )
{
	// Turn a multi back into a deed.
	// If it has chests with stuff inside then refuse to do so ?
	// This item specifically will morph into a deed. (so keys will not change!)

	Multi_Delete
	return( false );
}

#endif

bool CItem::Ship_Plank( bool fOpen )
{
	// This item is the ships plank.
	DEBUG_CHECK( m_type == ITEM_SHIP_PLANK || m_type == ITEM_SHIP_SIDE || m_type == ITEM_SHIP_SIDE_LOCKED );

	static const WORD Ship_Planks[] =
	{
		ITEMID_SHIP_PLANK_E_O,	ITEMID_SHIP_PLANK_E_C,
		ITEMID_SHIP_PLANK_E2_O,	ITEMID_SHIP_PLANK_E2_C,
		ITEMID_SHIP_PLANK_W_O,	ITEMID_SHIP_PLANK_W_C,
		ITEMID_SHIP_PLANK_S_O,	ITEMID_SHIP_PLANK_S_C,
		ITEMID_SHIP_PLANK_S2_O,	ITEMID_SHIP_PLANK_S2_C,
		ITEMID_SHIP_PLANK_N_O,	ITEMID_SHIP_PLANK_N_C,
	};

	for ( int i=0; i<COUNTOF(Ship_Planks); i+=2 )
	{
		if ( GetID() == Ship_Planks[i] ) // it is open
		{
			if ( ! fOpen )	// i wanted to close it.
			{
				SetDispID( (ITEMID_TYPE) Ship_Planks[i+1]);
				Update();
			}
			return true;
		}
		if ( GetID() == Ship_Planks[i+1] ) // it is closed.
		{
			if ( fOpen )	// i wanted to open it.
			{
				SetDispID( (ITEMID_TYPE) Ship_Planks[i]);
				Update();
			}
			return true;
		}
	}
	return( false );
}

void CItemMulti::Ship_Stop( void )
{
	m_itShip.m_fSail = false;
	SetTimeout( -1 );
}

bool CItemMulti::Ship_SetMoveDir( DIR_TYPE dir )
{
	// Set the direction we will move next time we get a tick.
	int iSpeed = 1;
	if ( m_itShip.m_DirMove == dir && m_itShip.m_fSail )
	{
		if ( m_itShip.m_DirFace == m_itShip.m_DirMove &&
			m_itShip.m_fSail == 1 )
		{
			iSpeed = 2;
		}
		else return( false );
	}

	if ( ! IsAttr(ATTR_MAGIC ))	// make sound.
	{
		if ( ! GetRandVal(10))
		{
			Sound( GetRandVal(2)?0x12:0x13 );
		}
	}

	m_itShip.m_DirMove = dir;
	m_itShip.m_fSail = iSpeed;
	GetTopSector()->SetSectorWakeStatus();	// may get here b4 my client does.
	SetTimeout(( m_itShip.m_fSail == 1 ) ? 1*TICK_PER_SEC : (TICK_PER_SEC/3));
	return( true );
}

#define MAX_MULTI_LIST_OBJS 128

int CItemMulti::Ship_ListObjs( CObjBase ** ppObjList, const CRegionWorld * pArea )
{
	// List all the objects in the structure.
	// Move the ship and everything on the deck
	// If too much stuff. then some will fall overboard. hehe.

	if ( ! IsTopLevel())
		return 0;

	int iCount = 0;

	CWorldSearch AreaChar( GetTopPoint(), UO_MAP_VIEW_SIZE );
	while ( iCount < MAX_MULTI_LIST_OBJS )
	{
		CChar * pChar = AreaChar.GetChar();
		if ( pChar == NULL )
			break;
		if ( pChar->IsClient())
		{
			pChar->GetClient()->addPause();	// get rid of flicker. for anyone even seeing this.
		}
		if ( ! pArea->IsInside( pChar->GetTopPoint()))
			continue;
		int zdiff = pChar->GetTopZ() - GetTopZ();
		if ( abs( zdiff ) > 3 )
			continue;
		ppObjList[iCount++] = pChar;
	}
	CWorldSearch AreaItem( GetTopPoint(), 8 );	// Biggest ship < 8
	while ( iCount < MAX_MULTI_LIST_OBJS )
	{
		CItem * pItem = AreaItem.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( ! Multi_IsPartOf( pItem ))
		{
			if ( ! pArea->IsInside( pItem->GetTopPoint()))
				continue;
			if ( ! pItem->IsMovable())
				continue;
			int zdiff = pItem->GetTopZ() - GetTopZ();
			if ( abs( zdiff ) > 3 )
				continue;
		}
		ppObjList[iCount++] = pItem;
	}
	return( iCount );
}

bool CItemMulti::Ship_MoveDelta( CPointBase pdelta, const CRegionWorld * pRegion )
{
	int znew = GetTopZ() + pdelta.m_z;
	if ( pdelta.m_z > 0 )
	{
		if ( znew >= (UO_SIZE_Z - PLAYER_HEIGHT )-1 )
			return( false );
	}
	else if ( pdelta.m_z < 0 )
	{
		if ( znew <= (UO_SIZE_MIN_Z + 3 ))
			return( false );
	}

	// Move the ship and everything on the deck
	CObjBase * ppObjs[MAX_MULTI_LIST_OBJS+1];
	int iCount = Ship_ListObjs( ppObjs, pRegion );
	for ( int i=0; i <iCount; i++ )
	{
		CObjBase * pObj = ppObjs[i];
		ASSERT(pObj);
		CPointMap pt = pObj->GetTopPoint();
		pt += pdelta;
		if ( ! pt.IsValid())  // boat goes out of bounds !
		{
			DEBUG_ERR(( "Ship uid=0%x out of bounds\n", GetUID()));
			continue;
		}
		pObj->MoveTo( pt );
		if ( pObj->IsChar())
		{
			pObj->RemoveFromView(); // Get rid of the blink/walk
			pObj->Update();
		}
	}

	return( true );
}

bool CItemMulti::Ship_CanMoveTo( const CPointMap & pt ) const
{
	// Can we move to the new location ? all water type ?
	if ( IsAttr(ATTR_MAGIC ))
		return( true );

	// Run into other ships ? assume my region has been "de-realized"
	CRegionBase * pRegionOther = pt.GetRegion( REGION_TYPE_MULTI );

	WORD wBlockFlags = CAN_I_WATER;
	signed char z = g_World.GetHeight( pt, wBlockFlags, pRegionOther );
	if ( ! ( wBlockFlags & CAN_I_WATER ))
	{
		return( false );
	}

	return( true );
}

bool CItemMulti::Ship_OnMoveTick( void )
{
	// We just got a move tick.
	// RETURN: false = delete the boat.

	if ( ! m_itShip.m_fSail )	// decay the ship instead ???
		return( true );

	CRegionWorld * pRegion = dynamic_cast <CRegionWorld*>( m_itShip.m_last_p.GetRegion( GetUID()));
	if ( pRegion == NULL )
	{
		DEBUG_ERR(( "Ship bad region\n" ));
		return( true );
	}

	// Calculate the leading point.
	DIR_TYPE dir = (DIR_TYPE) m_itShip.m_DirMove;
	CPointMap pt = pRegion->GetDirCorner(dir);
	pt.Move( dir );
	pt.m_z = GetTopZ();

	if ( ! pt.IsValid() ||
		( pt.m_x <= UO_SIZE_X_REAL && pt.m_x >= UO_SIZE_X_REAL ))
	{
		// Circle the globe
		// Fall off edge of world ?
		Ship_Stop();
		CItem * pTiller = Ship_GetTiller();
		pTiller->Speak( "Turbulent waters Cap'n", 0, TALKMODE_SAY, FONT_NORMAL );
		return( true );
	}

	// We should check all relevant corners.
	pRegion->RemoveSelf();	// don't interfere with the search for other ship regions.

	if ( ! Ship_CanMoveTo( pt ))
	{
cantmove:
		pRegion->RealizeRegion();
		Ship_Stop();
		CItem * pTiller = Ship_GetTiller();
		pTiller->Speak( "We've stopped Cap'n", 0, TALKMODE_SAY, FONT_NORMAL );
		return( true );
	}

	pt = pRegion->GetDirCorner(GetDirTurn(dir,-1));
	pt.Move( dir );
	pt.m_z = GetTopZ();
	if ( ! Ship_CanMoveTo( pt ))
		goto cantmove;

	pt = pRegion->GetDirCorner(GetDirTurn(dir,+1));
	pt.Move( dir );
	pt.m_z = GetTopZ();
	if ( ! Ship_CanMoveTo( pt ))
		goto cantmove;

	pRegion->RealizeRegion();

	pt.Zero();
	pt.Move( dir );
	Ship_MoveDelta( pt, pRegion);

	// Move again
	GetTopSector()->SetSectorWakeStatus();	// may get here b4 my client does.
	SetTimeout(( m_itShip.m_fSail == 1 ) ? 1*TICK_PER_SEC : (TICK_PER_SEC/2));
	return( true );
}

static const DIR_TYPE Ship_FaceDir[] =
{
	DIR_N,
	DIR_E,
	DIR_S,
	DIR_W,
};

void CItemMulti::Ship_Face( DIR_TYPE dir, CRegionWorld * pArea )
{
	// Change the direction of the ship.

	ASSERT( IsTopLevel());

	int i=0;
	for ( ; i<COUNTOF(Ship_FaceDir)-1; i++ )
	{
		if ( dir == Ship_FaceDir[i] )
			break;
	}

	int iFaceOffset = Ship_GetFaceOffset();
	ITEMID_TYPE idnew = (ITEMID_TYPE) ( GetID() - iFaceOffset + i );
	const CItemBaseMulti * pMultiNew = Multi_GetDef( idnew );
	if ( pMultiNew == NULL ) 
		return;
	const CItemBaseMulti * pMulti = Multi_GetDef();

	int iTurn = dir - Ship_FaceDir[ iFaceOffset ];

	// Reorient everything on the deck
	CObjBase * ppObjs[MAX_MULTI_LIST_OBJS+1];
	int iCount = Ship_ListObjs( ppObjs, pArea );
	for ( i=0; i<iCount; i++ )
	{
		CObjBase * pObj = ppObjs[i];
		CPointMap pt = pObj->GetTopPoint();

		if ( pObj->IsItem())
		{
			CItem * pItem = STATIC_CAST <CItem*> (pObj);
			ASSERT( pItem );
			if ( pItem == this )
			{
				pItem->SetDispID(idnew);
				pItem->Update();
				continue;
			}

			// Is this a ship component ? transform it.
			if ( Multi_IsPartOf( pItem ))
			{
				int xdiff = pt.m_x - GetTopPoint().m_x ;
				int ydiff = pt.m_y - GetTopPoint().m_y;
				for ( int i=0; i<pMulti->m_Components.GetCount(); i++ )
				{
					if ( xdiff != pMulti->m_Components[i].m_dx ||
						ydiff != pMulti->m_Components[i].m_dy )
						continue;
					pItem->SetDispID( pMultiNew->m_Components[i].m_id );
					pt = GetTopPoint();
					pt.m_x += pMultiNew->m_Components[i].m_dx;
					pt.m_y += pMultiNew->m_Components[i].m_dy;
					pt.m_z += pMultiNew->m_Components[i].m_dz;
					pItem->MoveTo( pt );
					pItem = NULL;	// signal to continue
					break;
				}
				if ( ! pItem ) continue;
			}
		}

		// -2,6 = left.
		// +2,-6 = right.
		// +-4 = flip around

		int iTmp;
		int xdiff = GetTopPoint().m_x - pt.m_x;
		int ydiff = GetTopPoint().m_y - pt.m_y;
		switch ( iTurn )
		{
		case 2: // right
		case (2-DIR_QTY):
			iTmp = xdiff;
			xdiff = ydiff;
			ydiff = -iTmp;
			break;
		case -2: // left.
		case (DIR_QTY-2):
			iTmp = xdiff;
			xdiff = -ydiff;
			ydiff = iTmp;
			break;
		default: // u turn.
			xdiff = -xdiff;
			ydiff = -ydiff;
			break;
		}
		pt.m_x = GetTopPoint().m_x + xdiff;
		pt.m_y = GetTopPoint().m_y + ydiff;
		pObj->MoveTo( pt );

		if ( pObj->IsChar())
		{
			// Turn creatures as well.
			CChar * pChar = STATIC_CAST <CChar*> (pObj);
			pChar->m_face_dir = GetDirTurn( pChar->m_face_dir, iTurn );
			pChar->m_pArea = pArea;	// This can get screwed if we change zones.
			pChar->RemoveFromView();
			pChar->Update();
		}
	}

	m_itShip.m_DirFace = dir;

	// Adjust the region to be the new area. (don't add/remove region)
	MultiAddRegion();
}

CItem * CItemMulti::Ship_GetTiller()
{
	CItem * pTiller = m_uidLink.ItemFind();
	if ( pTiller == NULL )
	{
		pTiller = Multi_FindItem( ITEM_SHIP_TILLER );
		if ( pTiller == NULL )
			return( this );
		m_uidLink = pTiller->GetUID();
	}
	return( pTiller );
}

bool CItemMulti::Ship_Command( const TCHAR * pszCmd, CChar * pChar )
{
	// Speaking in this ships region.
	// return: true = command for the ship.

	static const TCHAR * szShipCmds[] =
	{
		"Furl sail",	// Stop
		"Stop",			// Stops current ship movement.
		"Turn Left",
		"Port",			// Turn Left
		"Turn Right",
		"Starboard",	// Turn Right
		"Left",			// Move ship in desired direction.
		"Drift Left",
		"Right",		// Move ship in desired direction.
		"Drift Right",
		"Back",			// Move ship backwards
		"Backward",		// Move ship backwards
		"Backwards",	// Move ship backwards
		"Forward",
		"Foreward",		// Moves ship forward.
		"Unfurl sail",	// Moves ship forward.
		"Forward left",
		"forward right",
		"backward left",
		"back left",
		"backward right",
		"back right",
		"Raise Anchor",
		"Drop Anchor",
		"Turn around",	// Turns ship around and proceeds.
		"Come about",	// Turns ship around and proceeds.
		"Up",
		"Down",
		"Land",

		//"One (direction*)", " (Direction*), one" Moves ship one tile in desired direction and stops.
		//"Set name xxxxx" Changes Ship's name, where "xxxxx"=name.
		//"Slow (direction*)" Moves ship slowly in desired direction (see below for possible directions).
		//* Directions:

	};

	if ( ! pChar || IsAttr(ATTR_MOVE_NEVER) || ! IsTopLevel())
		return false;

	// Only key holders can command the ship ???
	// if ( pChar->ContentFindKeyFor( pItem ))

	int iCmd=0;
	for ( ; true; iCmd++ )
	{
		if ( iCmd >= COUNTOF(szShipCmds))
			return( false );
		if ( ! strnicmp( szShipCmds[iCmd], pszCmd, strlen(szShipCmds[iCmd])))
		{
			break;
		}
	}

	// Find the tiller man object.
	CItem * pTiller = Ship_GetTiller();
	ASSERT( pTiller != NULL );

	// Get current facing dir.
	CRegionWorld * pArea = pChar->m_pArea;
	DEBUG_CHECK(pArea);
	DIR_TYPE DirFace = Ship_FaceDir[ Ship_GetFaceOffset() ];
	int DirMoveChange;
	const TCHAR * pszSpeak = NULL;
	switch ( iCmd )
	{
	case 0: // "Furl sail"
	case 1: // "Stop" Stops current ship movement.
		if ( ! m_itShip.m_fSail )
			return( false );
		Ship_Stop();
		break;
	case 2: // "Port",
	case 3: // "Turn Left",
		DirMoveChange = -2;
doturn:
		if ( m_itShip.m_fAnchored )
		{
anchored:
			pszSpeak = "The anchor is down <SEX Sir/Mam>!";
			break;
		}
		m_itShip.m_DirMove = GetDirTurn( DirFace, DirMoveChange );
		Ship_Face( (DIR_TYPE) m_itShip.m_DirMove, pArea );
		break;
	case 4: // "Turn Right",
	case 5: // "Starboard",	// Turn Right
		DirMoveChange = 2;
		goto doturn;
	case 6: // "Left",
	case 7: // "Drift Left",
		DirMoveChange = -2;
dodirmovechange:
		if ( m_itShip.m_fAnchored )
			goto anchored;
		if ( ! Ship_SetMoveDir( GetDirTurn( DirFace, DirMoveChange )))
			return( false );
		break;
	case 8: // "Right",
	case 9: // "Drift Right",
		DirMoveChange = 2;
		goto dodirmovechange;
	case 10: // "Back",			// Move ship backwards
	case 11: // "Backward",		// Move ship backwards
	case 12: // "Backwards",	// Move ship backwards
		DirMoveChange = 4;
		goto dodirmovechange;
	case 13: // "Forward"
	case 14: // "Foreward",		// Moves ship forward.
	case 15: // "Unfurl sail",	// Moves ship forward.
		DirMoveChange = 0;
		goto dodirmovechange;
	case 16: // "Forward left",
		DirMoveChange = -1;
		goto dodirmovechange;
	case 17: // "forward right",
		DirMoveChange = 1;
		goto dodirmovechange;
	case 18: // "backward left",
	case 20: // "back left",
		DirMoveChange = -3;
		goto dodirmovechange;
	case 19: // "backward right",
	case 21: // "back right",
		DirMoveChange = 3;
		goto dodirmovechange;
	case 22: // "Raise Anchor",
		if ( ! m_itShip.m_fAnchored )
		{
			pszSpeak = "The anchor is already up <SEX Sir/Mam>";
			break;
		}
		m_itShip.m_fAnchored = false;
		break;
	case 23: // "Drop Anchor",
		if ( m_itShip.m_fAnchored )
		{
			pszSpeak = "The anchor is already down <SEX Sir/Mam>";
			break;
		}
		m_itShip.m_fAnchored = true;
		Ship_Stop();
		break;
	case 24: //	"Turn around",	// Turns ship around and proceeds.
	case 25: // "Come about",	// Turns ship around and proceeds.
		DirMoveChange = 4;
		goto doturn;
	case 26: // "Up"
		{
			if ( ! IsAttr(ATTR_MAGIC ))
				return( false );

			CPointMap pt;
			pt.m_z = PLAYER_HEIGHT;
			if ( Ship_MoveDelta( pt, pArea ))
			{
				pszSpeak = "As you command <SEX Sir/Mam>";
			}
			else
			{
				pszSpeak = "Can't do that <SEX Sir/Mam>";
			}
		}
		break;
	case 27: // "Down"
		{
			if ( ! IsAttr(ATTR_MAGIC ))
				return( false );
			CPointMap pt;
			pt.m_z = -PLAYER_HEIGHT;
			if ( Ship_MoveDelta( pt, pArea ))
			{
				pszSpeak = "As you command <SEX Sir/Mam>";
			}
			else
			{
				pszSpeak = "Can't do that <SEX Sir/Mam>";
			}
		}
		break;
	case 28: // "Land"
		{
			if ( ! IsAttr(ATTR_MAGIC ))
				return( false );
			signed char zold = GetTopZ();
			CPointMap pt = GetTopPoint();
			pt.m_z = zold;
			SetTopZ( -UO_SIZE_Z );	// bottom of the world where i won't get in the way.
			WORD wBlockFlags = CAN_I_WATER;
			signed char z = g_World.GetHeight( pt, wBlockFlags );
			SetTopZ( zold );	// restore z for now.
			pt.Init();
			pt.m_z = z - zold;
			if ( pt.m_z )
			{
				Ship_MoveDelta( pt, pArea );
				pszSpeak = "As you command <SEX Sir/Mam>";
			}
			else
			{
				pszSpeak = "We have landed <SEX Sir/Mam>";
			}
		}
		break;
	default:
		return( false );
	}

	if ( pszSpeak == NULL )
	{
		const TCHAR * pszAye[] =
		{
			"Aye",
			"Aye Cap'n",
			"Aye <SEX Sir/Mam>",
		};
		pszSpeak = pszAye[ GetRandVal( COUNTOF( pszAye )) ];
	}

	TCHAR szText[ 256 ];
	strcpy( szText, pszSpeak );
	pChar->ParseText( szText, &g_Serv );
	pTiller->Speak( szText, 0, TALKMODE_SAY, FONT_NORMAL );
	return( true );
}

#ifdef COMMENT
void CItemMulti::Ship_Dock( CChar * pChar )
{
	// Attempt to dock this "ship". If it is a ship.
	// Must not have things on the deck or in the hold.
	ASSERT( pChar != NULL );

	CItem * pItemShip;
	if ( m_type != ITEM_SHIP )
	{

	}

	if ( m_type != ITEM_SHIP )

		pChar->GetPackSafe()->ContentAdd( this );
}
#endif

/////////////////////////////////////////////////////////////////////////////

bool CItem::Plant_OnTick()
{
	ASSERT( m_type == ITEM_PLANT );
	// If it is in a container, kill it.
	if ( !IsTopLevel())
	{
		return false;
	}

	// Make sure the darn thing isn't moveable
	m_Attr |= ATTR_MOVE_NEVER;

	// Go to next step for crops
	static const WORD Items_Trees[] =
	{
		ITEMID_TREE_COCONUT,
		ITEMID_TREE_DATE,
		ITEMID_TREE_BANANA1,
		ITEMID_TREE_BANANA2,
		ITEMID_TREE_BANANA3,
		ITEMID_TREE_APPLE_EMPTY1,
		ITEMID_TREE_APPLE_FULL1,
		ITEMID_TREE_APPLE_FALL1,
		ITEMID_TREE_APPLE_EMPTY2,
		ITEMID_TREE_APPLE_FULL2,
		ITEMID_TREE_APPLE_FALL2,
		ITEMID_TREE_PEACH_EMPTY1,
		ITEMID_TREE_PEACH_FULL1,
		ITEMID_TREE_PEACH_FALL1,
		ITEMID_TREE_PEACH_EMPTY2,
		ITEMID_TREE_PEACH_FULL2,
		ITEMID_TREE_PEACH_FALL2,
		ITEMID_TREE_PEAR_EMPTY1,
		ITEMID_TREE_PEAR_FULL1,
		ITEMID_TREE_PEAR_FALL1,
		ITEMID_TREE_PEAR_EMPTY2,
		ITEMID_TREE_PEAR_FULL2,
		ITEMID_TREE_PEAR_FALL2,
	};

	// Figure out the next time crops will "mature"....use FELUCCA for now
	DWORD iNextTick = g_World.GetNextNewMoon(true) - ( (g_World.GetTime() + GetRandVal(20) ) * g_Serv.m_iGameMinuteLength * TICK_PER_SEC );

	ITEMID_TYPE iTemp = m_itPlant.m_Reap_ID; // Don't "forget" what we are
	ITEMID_TYPE iID = GetDispID();

	if ( FindID( iID, Items_Trees, COUNTOF(Items_Trees)) >= 0 )
	{
		// Do all tree stuff here
		switch ( iID ) // Go to next "season"
		{
		case ITEMID_TREE_COCONUT:
		case ITEMID_TREE_DATE:
			// DropFruit(); // Not harvested? Drop the fruit then too
			SetTimeout( iNextTick );
			Update();
			return true;
		case ITEMID_TREE_BANANA2:
			// DropFruit(); // Not harvested? Drop the fruit then too
			SetDispID(ITEMID_TREE_BANANA3);
			m_itPlant.m_Reap_ID = iTemp;
			break;
		case ITEMID_TREE_BANANA3:
			SetDispID(ITEMID_TREE_BANANA2);
			m_itPlant.m_Reap_ID = iTemp;
			break;
		case ITEMID_TREE_APPLE_FULL1: // Was summer, go to fall
		case ITEMID_TREE_APPLE_FULL2:
		case ITEMID_TREE_PEAR_FULL1:
		case ITEMID_TREE_PEAR_FULL2:
		case ITEMID_TREE_PEACH_FULL1:
		case ITEMID_TREE_PEACH_FULL2:
			// DropLeaves(); // Drop some leaves
			// DropFruit(); // Not harvested? Drop the fruit then too
			SetDispID((ITEMID_TYPE)(GetID() + 1));
			m_itPlant.m_Reap_ID = iTemp;
			break;
		case ITEMID_TREE_APPLE_FALL1: // Was fall go to winter
		case ITEMID_TREE_APPLE_FALL2:
		case ITEMID_TREE_PEAR_FALL1:
		case ITEMID_TREE_PEAR_FALL2:
		case ITEMID_TREE_PEACH_FALL1:
		case ITEMID_TREE_PEACH_FALL2:
			//DropLeaves(); // Drop more leave
			SetDispID((ITEMID_TYPE)(GetID() - 2));
			m_itPlant.m_Reap_ID = iTemp;
			RemoveFromView();	// remove from most screens.
			SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is growing.
			m_Attr |= ATTR_INVIS;	// regrown invis.
			break;
		case ITEMID_TREE_APPLE_EMPTY1: // Was winter (or spring), go to spring (or summer)
		case ITEMID_TREE_APPLE_EMPTY2:
		case ITEMID_TREE_PEAR_EMPTY1:
		case ITEMID_TREE_PEAR_EMPTY2:
		case ITEMID_TREE_PEACH_EMPTY1:
		case ITEMID_TREE_PEACH_EMPTY2:
			m_itPlant.m_Reap_ID = iTemp;
			if (GetColor() != COLOR_DEFAULT) // Was winter, go to spring
			{
				SetColor( COLOR_DEFAULT );
				m_Attr &= ~ATTR_INVIS;
			}
			else // Was spring, go to summer
				SetDispID((ITEMID_TYPE)(GetID() + 1));
			break;
		}

		switch ( m_itPlant.m_Reap_ID ) // Fix the names, they keep forgetting what they are (dumb osi)
		{
		case ITEMID_FRUIT_DATE1:
			SetName("Date Palm");
			break;
		case ITEMID_FRUIT_COCONUT1:
			SetName("Coconut Palm");
			break;
		case ITEMID_FRUIT_APPLE:
			SetName("Apple Tree");
			break;
		case ITEMID_FRUIT_PEAR:
			SetName("Pear Tree");
			break;
		case ITEMID_FRUIT_PEACH1:
		case ITEMID_FRUIT_PEACH2:
			SetName("Peach Tree");
			break;
		case ITEMID_FRUIT_BANANA1:
		case ITEMID_FRUIT_BANANA2:
			SetName("Banana Tree");
			break;
		case ITEMID_FRUIT_LIME:
			SetName("Lime Tree");
			break;
		case ITEMID_FRUIT_LEMON:
			SetName("Lemon Tree");
			break;
		}
		SetTimeout( iNextTick );
		Update();
		return true;
	}

	// No tree stuff below here
	if ( IsAttr(ATTR_INVIS)) // If it's invis, take care of it here and return
	{
		SetColor( COLOR_DEFAULT );
		m_Attr &= ~ATTR_INVIS;
		SetTimeout( iNextTick );
		Update();
		return true;
	}

	switch ( iID )
	{
	case ITEMID_SPROUT_NORMAL: // Normal advances to normal2
		SetDispID(ITEMID_SPROUT_NORMAL2);
		m_itPlant.m_Reap_ID = iTemp;
		break;
	case ITEMID_SPROUT_WHEAT1: // Stage 1
		SetDispID(ITEMID_SPROUT_WHEAT2);
		m_itPlant.m_Reap_ID = iTemp;
		SetName("Wheat Sprout");
		break;
	case ITEMID_SPROUT_NORMAL2:
	case ITEMID_PLANT_CORN2: // Corn has an extra stage of growth from the other "grains"...so come here twice
		{
			switch ( m_itPlant.m_Reap_ID )
			{
			case ITEMID_FRUIT_LETTUCE1: // Lettuce has no plant form, just fruit
			case ITEMID_FRUIT_LETTUCE2:
				{
					static const WORD Items_Lettuce[] =
					{
						ITEMID_FRUIT_LETTUCE1,
						ITEMID_FRUIT_LETTUCE2,
					};
					SetDispID((ITEMID_TYPE) Items_Lettuce[ GetRandVal( COUNTOF( Items_Lettuce ))]);
					SetName("Lettuce Plant");
					m_itPlant.m_Reap_ID = iTemp;
					m_type = ITEM_PLANT;
					SetTimeout( iNextTick );
					Update();
					return true;
				}
				break;
			case ITEMID_FRUIT_CABBAGE1: // Cabbage has no plant form, just fruit
			case ITEMID_FRUIT_CABBAGE2:
				{
					static const WORD Items_Cabbage[] =
					{
						ITEMID_FRUIT_CABBAGE1,
						ITEMID_FRUIT_CABBAGE2,
					};
					SetDispID((ITEMID_TYPE) Items_Cabbage[ GetRandVal( COUNTOF( Items_Cabbage ))]);
					SetName("Cabbage Plant");
					m_itPlant.m_Reap_ID = iTemp;
					m_type = ITEM_PLANT;
					SetTimeout( iNextTick );
					Update();
					return true;
				}
				break;
			case ITEMID_FRUIT_HAY1:
			case ITEMID_FRUIT_HAY2:
			case ITEMID_FRUIT_HAY3:
				{
					static const WORD Items_Hay[] =
					{
						ITEMID_PLANT_HAY1,
						ITEMID_PLANT_HAY2,
						ITEMID_PLANT_HAY3,
						ITEMID_PLANT_HAY4,
						ITEMID_PLANT_HAY5,
					};
					SetDispID((ITEMID_TYPE) Items_Hay[ GetRandVal( COUNTOF( Items_Hay ))]);
					SetName("Hay Plant");
				}
				break;
			case ITEMID_FRUIT_COTTON:
				{
					static const WORD Items_Cotton[] =
					{
						ITEMID_PLANT_COTTON1,
						ITEMID_PLANT_COTTON2,
						ITEMID_PLANT_COTTON3,
						ITEMID_PLANT_COTTON4,
						ITEMID_PLANT_COTTON5,
						ITEMID_PLANT_COTTON6,
					};
					SetDispID((ITEMID_TYPE) Items_Cotton[ GetRandVal( COUNTOF( Items_Cotton ))]);
					SetName("Cotton Plant");
				}
				break;
			case ITEMID_FRUIT_HOPS:
				{
					static const WORD Items_Hops[] =
					{
						ITEMID_PLANT_HOPS1,
						ITEMID_PLANT_HOPS2,
						ITEMID_PLANT_HOPS3,
						ITEMID_PLANT_HOPS4,
					};
					SetDispID((ITEMID_TYPE) Items_Hops[ GetRandVal( COUNTOF( Items_Hops ))]);
					SetName("Hops Plant");
				}
				break;
			case ITEMID_FRUIT_FLAX1:
			case ITEMID_FRUIT_FLAX2:
				{
					static const WORD Items_Flax[] =
					{
						ITEMID_PLANT_FLAX1,
						ITEMID_PLANT_FLAX2,
						ITEMID_PLANT_FLAX3,
					};
					SetDispID((ITEMID_TYPE) Items_Flax[ GetRandVal( COUNTOF( Items_Flax ))]);
					SetName("Flax Plant");
				}
				break;
			case ITEMID_FRUIT_CORN1:
			case ITEMID_FRUIT_CORN2:
			case ITEMID_FRUIT_CORN3:
			case ITEMID_FRUIT_CORN4:
				{
					if (GetID() == ITEMID_PLANT_CORN2)
						SetDispID(ITEMID_PLANT_CORN1);
					else
					{
						SetDispID(ITEMID_PLANT_CORN2);
						SetName("Corn Stalk");
					}
				}
				break;
			case ITEMID_FRUIT_CARROT1:
			case ITEMID_FRUIT_CARROT2:
				{
					SetDispID(ITEMID_PLANT_CARROT);
					SetName("Carrot Plant");
				}
				break;
			case ITEMID_FRUIT_GARLIC1: // North/South
				{
					SetDispID(ITEMID_PLANT_GARLIC1);
					SetName("Garlic Plant");
				}
				break;
			case ITEMID_FRUIT_GARLIC2: // East/West
				{
					SetDispID(ITEMID_PLANT_GARLIC2);
					SetName("Garlic Plant");
				}
				break;
			case ITEMID_FRUIT_GENSENG1:
			case ITEMID_FRUIT_GENSENG2:
				{
					static const WORD Items_Genseng[] =
					{
						ITEMID_PLANT_GENSENG1,
						ITEMID_PLANT_GENSENG2,
					};
					SetDispID((ITEMID_TYPE) Items_Genseng[ GetRandVal( COUNTOF( Items_Genseng ))]);
					SetName("Genseng Plant");
				}
				break;
			case ITEMID_FRUIT_TURNIP1:
			case ITEMID_FRUIT_TURNIP2:
				{
					static const WORD Items_Turnip[] =
					{
						ITEMID_PLANT_TURNIP1,
						ITEMID_PLANT_TURNIP2,
					};
					SetDispID((ITEMID_TYPE) Items_Turnip[ GetRandVal( COUNTOF( Items_Turnip ))]);
					SetName("Turnip Plant");
				}
				break;
			case ITEMID_FRUIT_ONIONS1:
			case ITEMID_FRUIT_ONIONS2:
				{
					SetDispID(ITEMID_PLANT_ONION);
					SetName("Onion Plant");
				}
				break;
			case ITEMID_FRUIT_MANDRAKE_ROOT1:
			case ITEMID_FRUIT_MANDRAKE_ROOT2:
				{
					static const WORD Items_Mandrake_Root[] =
					{
						ITEMID_PLANT_MANDRAKE1,
						ITEMID_PLANT_MANDRAKE2,
					};
					SetDispID((ITEMID_TYPE) Items_Mandrake_Root[ GetRandVal( COUNTOF( Items_Mandrake_Root ))]);
					SetName("Mandrake Root Plant");
				}
				break;
			case ITEMID_FRUIT_NIGHTSHADE1:
			case ITEMID_FRUIT_NIGHTSHADE2:
				{
					static const WORD Items_Nightshade[] =
					{
						ITEMID_PLANT_NIGHTSHADE1,
						ITEMID_PLANT_NIGHTSHADE2,
					};
					SetDispID((ITEMID_TYPE) Items_Nightshade[ GetRandVal( COUNTOF( Items_Nightshade ))]);
					SetName("Nightshade Plant");
				}
				break;
			}
		}
		break;
	case ITEMID_SPROUT_WHEAT2: // Stage 2
		{
			static const WORD Items_Wheat[] =
			{
				ITEMID_PLANT_WHEAT1,
				ITEMID_PLANT_WHEAT2,
				ITEMID_PLANT_WHEAT3,
				ITEMID_PLANT_WHEAT4,
				ITEMID_PLANT_WHEAT5,
				ITEMID_PLANT_WHEAT6,
				ITEMID_PLANT_WHEAT7,
			};
			SetDispID((ITEMID_TYPE) Items_Wheat[ GetRandVal( COUNTOF( Items_Wheat ))]);
			SetName("Wheat Plant");
		}
		break;
	case ITEMID_FRUIT_LETTUCE1: // Wasn't harvested....replant it
	case ITEMID_FRUIT_LETTUCE2:
	case ITEMID_FRUIT_CABBAGE1:
	case ITEMID_FRUIT_CABBAGE2:
	case ITEMID_PLANT_HAY1:
	case ITEMID_PLANT_HAY2:
	case ITEMID_PLANT_HAY3:
	case ITEMID_PLANT_HAY4:
	case ITEMID_PLANT_HAY5:
	case ITEMID_PLANT_COTTON1:
	case ITEMID_PLANT_COTTON2:
	case ITEMID_PLANT_COTTON3:
	case ITEMID_PLANT_COTTON4:
	case ITEMID_PLANT_COTTON5:
	case ITEMID_PLANT_COTTON6:
	case ITEMID_PLANT_HOPS1:
	case ITEMID_PLANT_HOPS2:
	case ITEMID_PLANT_HOPS3:
	case ITEMID_PLANT_HOPS4:
	case ITEMID_PLANT_FLAX1:
	case ITEMID_PLANT_FLAX2:
	case ITEMID_PLANT_FLAX3:
	case ITEMID_PLANT_CORN1:
	case ITEMID_PLANT_CARROT:
	case ITEMID_PLANT_GARLIC1:
	case ITEMID_PLANT_GARLIC2:
	case ITEMID_PLANT_GENSENG1:
	case ITEMID_PLANT_GENSENG2:
	case ITEMID_PLANT_TURNIP1:
	case ITEMID_PLANT_TURNIP2:
	case ITEMID_PLANT_ONION:
	case ITEMID_PLANT_MANDRAKE1:
	case ITEMID_PLANT_MANDRAKE2:
	case ITEMID_PLANT_NIGHTSHADE1:
	case ITEMID_PLANT_NIGHTSHADE2:
		SetDispID( ITEMID_SPROUT_NORMAL );
		RemoveFromView();	// remove from most screens.
		SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is growing.
		m_Attr |= ATTR_INVIS;	// regrown invis.
		break;
	case ITEMID_PLANT_WHEAT1:
	case ITEMID_PLANT_WHEAT2:
	case ITEMID_PLANT_WHEAT3:
	case ITEMID_PLANT_WHEAT4:
	case ITEMID_PLANT_WHEAT5:
	case ITEMID_PLANT_WHEAT6:
	case ITEMID_PLANT_WHEAT7:
		SetDispID( ITEMID_SPROUT_WHEAT1 );
		RemoveFromView();	// remove from most screens.
		SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is growing.
		m_Attr |= ATTR_INVIS;	// regrown invis.
		break;
	default:
		SetColor( COLOR_DEFAULT );
		m_Attr &= ~ATTR_INVIS;
		SetTimeout( -1 );
		Update();
		break;
	}
	m_itPlant.m_Reap_ID = iTemp;
	SetTimeout( iNextTick );
	Update();
	return true;
}

void CItem::Plant_CropReset()
{
	// Animals will eat crops before they are ripe, so we need a way to reset them prematurely
	if ( m_pDef->m_type != ITEM_PLANT )
	{
		// This isn't a crop, and since it just got eaten, we should delete it
		Delete();
		return;
	}

	DWORD iNextTick = g_World.GetNextNewMoon(true) - g_World.GetTime() + GetRandVal(20);
	ITEMID_TYPE id = ITEMID_NOTHING;
	const TCHAR * pszName;

	switch ( GetID())
	{
	case ITEMID_SPROUT_WHEAT1:
	case ITEMID_SPROUT_WHEAT2:
	case ITEMID_PLANT_WHEAT1:
	case ITEMID_PLANT_WHEAT2:
	case ITEMID_PLANT_WHEAT3:
	case ITEMID_PLANT_WHEAT4:
	case ITEMID_PLANT_WHEAT5:
	case ITEMID_PLANT_WHEAT6:
	case ITEMID_PLANT_WHEAT7:
		id = ITEMID_SPROUT_WHEAT1;
		pszName = "Wheat Sprout";
		break;
	case ITEMID_SPROUT_NORMAL:
	case ITEMID_SPROUT_NORMAL2:
	case ITEMID_PLANT_COTTON1:
	case ITEMID_PLANT_COTTON2:
	case ITEMID_PLANT_COTTON3:
	case ITEMID_PLANT_COTTON4:
	case ITEMID_PLANT_COTTON5:
	case ITEMID_PLANT_COTTON6:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Cotton Sprout";
		break;
	case ITEMID_PLANT_HAY1:
	case ITEMID_PLANT_HAY2:
	case ITEMID_PLANT_HAY3:
	case ITEMID_PLANT_HAY4:
	case ITEMID_PLANT_HAY5:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Hay Sprout";
		break;
	case ITEMID_PLANT_HOPS1:
	case ITEMID_PLANT_HOPS2:
	case ITEMID_PLANT_HOPS3:
	case ITEMID_PLANT_HOPS4:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Hops Sprout";
		break;
	case ITEMID_PLANT_FLAX1:
	case ITEMID_PLANT_FLAX2:
	case ITEMID_PLANT_FLAX3:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Flax Sprout";
		break;
	case ITEMID_PLANT_CORN1:
	case ITEMID_PLANT_CORN2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Corn Sprout";
		break;
	case ITEMID_PLANT_CARROT:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Carrot Sprout";
		break;
	case ITEMID_PLANT_GARLIC1:
	case ITEMID_PLANT_GARLIC2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Garlic Sprout";
		break;
	case ITEMID_PLANT_GENSENG1:
	case ITEMID_PLANT_GENSENG2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Genseng Sprout";
		break;
	case ITEMID_PLANT_TURNIP1:
	case ITEMID_PLANT_TURNIP2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Turnip Sprout";
		break;
	case ITEMID_PLANT_ONION:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Onion Sprout";
		break;
	case ITEMID_PLANT_MANDRAKE1:
	case ITEMID_PLANT_MANDRAKE2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Mandrake Sprout";
		break;
	case ITEMID_PLANT_NIGHTSHADE1:
	case ITEMID_PLANT_NIGHTSHADE2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Nightshade Sprout";
		break;
	case ITEMID_FRUIT_LETTUCE1:
	case ITEMID_FRUIT_LETTUCE2:
		id = ITEMID_SPROUT_NORMAL;
		pszName = "Lettuce Sprout";
		break;
	case ITEMID_FRUIT_CABBAGE1:
	case ITEMID_FRUIT_CABBAGE2:
		break;
	default:
		break;
	}

	ITEMID_TYPE iReapID = m_itPlant.m_Reap_ID;
	if ( id != ITEMID_NOTHING )
	{
		SetDispID(id);
		m_itPlant.m_Reap_ID = iReapID;
		SetName(pszName);
	}

	SetTimeout( iNextTick );
	RemoveFromView();	// remove from most screens.
	SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is growing.
	m_Attr |= ATTR_INVIS;	// regrown invis.
}

/////////////////////////////////////////////////////////////////////////////
// -CItemMap

bool CItemMap::r_LoadVal( CScript & s ) // Load an item Script
{
	if ( s.IsKeyHead( "PIN", 3 ))
	{
		CPointMap pntTemp;
		pntTemp.Read( s.GetArgStr());
		m_Pins.Add( CMapPinRec( pntTemp.m_x, pntTemp.m_y ));
	}
	return( CItem::r_LoadVal( s ));
}

bool CItemMap::r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc )
{
	if ( ! strnicmp( pKey, "PIN", 3 ))
	{
		pKey += 3;
		int i = Exp_GetVal(pKey) - 1;
		if ( i >= 0 && i < m_Pins.GetCount())
		{
			sVal.Format( "%i,%i", m_Pins[i].m_x, m_Pins[i].m_y );
			return( true );
		}
	}
	return( CItemVendable::r_WriteVal( pKey, sVal, pSrc ));
}

void CItemMap::r_Write( CScript & s )
{
	CItem::r_Write( s );
	for ( int i=0; i<m_Pins.GetCount(); i++ )
	{
		s.WriteKeyFormat( "PIN", "%i,%i", m_Pins[i].m_x, m_Pins[i].m_y );
	}
}

void CItemMap::DupeCopy( const CItem * pItem )
{
	CItemVendable::DupeCopy(pItem);

	const CItemMap * pMapItem = dynamic_cast <const CItemMap *>(pItem);
	DEBUG_CHECK(pMapItem);
	if ( pMapItem == NULL )
		return;

	for ( int i=0; i< pMapItem->m_Pins.GetCount(); i++ )
	{
		m_Pins.Add( pMapItem->m_Pins[i] );
	}
}

/////////////////////////////////////////////////////////////////////////////
// -CItemMessage

void CItemMessage::r_Write( CScript & s )
{
	CItem::r_Write( s );

	s.WriteKey( "AUTHOR", m_sAuthor );

	// Store the message body lines.
	for ( int i=0; i<GetPageCount(); i++ )
	{
		CGString sKey;
		sKey.Format( "BODY%d", i );
		const TCHAR * pszText = GetPageText(i);
		s.WriteKey( sKey, ( pszText ) ?  pszText : "" );
	}
}

bool CItemMessage::r_LoadVal( CScript &s )
{
	// Load the message body for a book or a bboard message.
	if ( s.IsKey( "AUTHOR" ))
	{
		if ( s.GetArgStr()[0] != '0' )
		{
			m_sAuthor = s.GetArgStr();
		}
		return( true );
	}
	if ( s.IsKeyHead( "BODY", 4 ))
	{
		AddPageText( s.GetArgStr());
		return( true );
	}
	return( CItem::r_LoadVal( s ));
}

bool CItemMessage::r_Verb( CScript & s, CTextConsole * pSrc )
{
	if ( s.IsKey( "ERASE" ))
	{
		// 1 based pages.
		int iPage = ( s.GetArgStr()[0] && toupper( s.GetArgStr()[0] ) != 'A' ) ? s.GetArgVal() : 0;
		if ( ! iPage )
		{
			m_sBodyLines.RemoveAll();
			return( true );
		}
		else if ( iPage <= m_sBodyLines.GetCount())
		{
			m_sBodyLines.RemoveAt( iPage-1 );
			return( true );
		}
	}
	if ( s.IsKeyHead( "PAGE", 4 ))
	{
		SetPageText( atoi( s.GetKey() + 4 )-1, s.GetArgStr());
		return( true );
	}

	return( CItem::r_Verb( s, pSrc ));
}

void CItemMessage::DupeCopy( const CItem * pItem )
{
	CItemVendable::DupeCopy( pItem );

	const CItemMessage * pMsgItem = dynamic_cast <const CItemMessage *>(pItem);
	DEBUG_CHECK(pMsgItem);
	if ( pMsgItem == NULL ) 
		return;

	m_sAuthor = pMsgItem->m_sAuthor;
	for ( int i=0; i<pMsgItem->GetPageCount(); i++ )
	{
		SetPageText( i, pMsgItem->GetPageText(i));
	}
}

bool CItemMessage::LoadSystemPages()
{
	static const TCHAR * szBookCommands[] = 
	{
		"AUTHOR",
		"PAGES",
		"TITLE",
	};

	DEBUG_CHECK( IsBookSystem());

	CGString sSec;
	sSec.Format( "BOOK %li", m_itBook.m_TimeID );

	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_BOOK_2, sSec ) == NULL )
		return false;

	int iPages = -1;
	while ( s.ReadKeyParse())
	{
		switch ( FindTableSorted( s.GetKey(), szBookCommands, COUNTOF( szBookCommands )))
		{
		case 0: // "AUTHOR"
			m_sAuthor = s.GetArgStr();
			break;
		case 1:	// "PAGES"
			iPages = s.GetArgVal();
			break;
		case 2:	// "TITLE"
			SetName( s.GetArgStr());	// Make sure the book is named.
			break;
		}
	}

	if ( iPages > 128 || iPages < 0 )
		return( false );

	TCHAR szTemp[ 16*1024 ];
	for ( int iPage=1; iPage<=iPages; iPage++ )
	{
		sSec.Format( _TEXT("BOOK %d %d"), m_itBook.m_TimeID, iPage );
		if ( ! s.FindSection( sSec ))
		{
			break;
		}

		int iLen = 0;
		while (s.ReadKey())
		{
			int iLenStr = strlen( s.GetKey());
			if ( iLen + iLenStr >= sizeof( szTemp )) 
				break;
			iLen += strcpylen( szTemp+iLen, s.GetKey());
			szTemp[ iLen++ ] = '\t';
		}
		szTemp[ --iLen ] = '\0';

		AddPageText( szTemp );
	}

	return( true );
}

/////////////////////////////////////////////////////////////////////////////
// -CItemMemory

int CItemMemory::FixWeirdness()
{
	int iResultCode = CItem::FixWeirdness();
	if ( iResultCode )
	{
bailout:
		return( iResultCode );
	}

	if ( ! IsEquipped() || 
		GetEquipLayer() != LAYER_SPECIAL ||
		! GetMemoryTypes())	// has to be a memory of some sort.
	{
		iResultCode = 0x4222;
		goto bailout;	// get rid of it.
	}

	CChar * pChar = dynamic_cast <CChar*>( GetParent());
	if ( pChar == NULL )
	{
		iResultCode = 0x4223;
		goto bailout;	// get rid of it.
	}

	// If it is my guild make sure i am linked to it correctly !
	if ( IsMemoryTypes(MEMORY_GUILD|MEMORY_TOWN))
	{
		CItemStone * pStone = pChar->Guild_Find((MEMORY_TYPE) GetMemoryTypes());
		if ( pStone == NULL ||
			pStone->GetUID() != m_uidLink )
		{
			iResultCode = 0x4224;
			goto bailout;	// get rid of it.
		}
		if ( ! pStone->GetMember( pChar ))
		{
			iResultCode = 0x4225;
			goto bailout;	// get rid of it.
		}
	}

	if ( g_World.m_iSaveVersion < 49 )
	{
		if ( IsMemoryTypes( OLDMEMORY_MURDERDECAY ))
		{
			pChar->Noto_Murder();
			iResultCode = 0x4227;
			goto bailout;	// get rid of it.
		}
	}

	return( 0 );
}

