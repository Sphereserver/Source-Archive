//
// CItem.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

const TCHAR * CItem::sm_szTrigName[ITRIG_QTY] =	// static
{
	"DCLICK",		// I have been dclicked.
	"STEP",			// I have been walked on.
	"TIMER",		// My timer has expired.
	"DAMAGE",		// I have been damaged in some way
	"SPELL",		// cast some spell on me.
	"HEAR",			// I hear this said ? NOT USED !

	"EQUIP",		// I have been unequipped
	"UNEQUIP",		// i have been unequipped (or try to unequip)

	"PICKUP_GROUND",
	"PICKUP_PACK",	// picked up from inside some container.

	"DROPON_ITEM",		// I have been dropped on this item
	"DROPON_CHAR",		// I have been dropped on this char
	"DROPON_GROUND",		// I have been dropped on the ground here

	"TARGON_ITEM",	// I am being combined with an item
	"TARGON_CHAR",
	"TARGON_GROUND",

	//"CREATE",
	//"DESTROY",
};

/////////////////////////////////////////////////////////////////
// -CItem

CItem::CItem( ITEMID_TYPE id, CItemBase * pDef ) : CObjBase( true )
{
	ASSERT( pDef );

	g_Serv.StatInc(SERV_STAT_ITEMS);
	m_Attr = 0;
	m_amount = 1;

	m_itNormal.m_more1 = 0;
	m_itNormal.m_more2 = 0;
	m_itNormal.m_morep.Zero();

	m_pDef = NULL;
	SetBase( pDef );

	// Set the default display id.
	if ( CItemBase::IsValidDispID(id))
	{
		m_id = id;
	}
	else
	{
		m_id = m_pDef->GetDispID();
	}

	// Set up some type based defaults.

	if ( m_type == ITEM_LIGHT_LIT || m_pDef->Can( CAN_I_LIGHT )) 
	{
		m_itLight.m_pattern = LIGHT_LARGE;	// default.
	}
	if ( m_type == ITEM_SCROLL )
	{
		m_Attr |= ATTR_MAGIC;	// Magic by default.
		m_itWeapon.m_spell = GetScrollSpell();
	}
	switch ( m_id )
	{
	case ITEMID_KEY_MAGIC:
	case ITEMID_SPELLBOOK:	// these are always newbie items.
		m_Attr |= ATTR_NEWBIE | ATTR_MAGIC;	// Magic by default.
		break;
	}
	// Put this item in limbo.
	g_World.m_ItemsNew.InsertAfter( this );
	ASSERT( IsDisconnected());
}

CItem::~CItem()
{
	DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	switch ( m_type )
	{
	case ITEM_SPAWN_CHAR:
		Spawn_KillChildren();
		break;
	case ITEM_FIGURINE:
	case ITEM_EQ_HORSE:
		{	// remove the ridden or linked char.
			CChar * pHorse = m_itFigurine.m_UID.CharFind();
			if ( pHorse && pHorse->IsDisconnected() && pHorse->m_pNPC )
			{
				DEBUG_CHECK( pHorse->IsStat( STATF_Ridden ));
				DEBUG_CHECK( pHorse->GetActiveSkill() == NPCACT_RIDDEN );
				pHorse->m_atRidden.m_FigurineUID.ClearUID();
				pHorse->Delete();
			}
		}
		break;
	}
	if ( m_pDef != NULL )
	{
		m_pDef->DelInstance();
	}
	g_Serv.StatDec(SERV_STAT_ITEMS);
}

CItem * CItem::CreateBase( ITEMID_TYPE id )	// static
{
	// All CItem creates come thru here.
	// NOTE: This is a free floating item.
	//  If not put on ground or in container after this = Memory leak !
	ITEMID_TYPE idErrorMsg = ITEMID_NOTHING;
	CItemBase * pDef = CItemBase::FindItemBase( id );
	if ( pDef == NULL )
	{
		idErrorMsg = id;
		id = ITEMID_GOLD_C1;
		pDef = CItemBase::FindItemBase( id );
	}
	CItem * pItem;
	switch ( pDef->m_type )
	{
	case ITEM_MAP:
		pItem = new CItemMap( id, pDef );
		break;
	case ITEM_GAME_BOARD:
	case ITEM_BBOARD:
	case ITEM_CONTAINER_LOCKED:
	case ITEM_CONTAINER:
	case ITEM_EQ_TRADE_WINDOW:
	case ITEM_EQ_VENDOR_BOX:
	case ITEM_EQ_BANK_BOX:
		pItem = new CItemContainer( id, pDef );
		break;
	case ITEM_CORPSE:
		pItem = new CItemCorpse( id, pDef );
		break;
	case ITEM_MESSAGE:
	case ITEM_BOOK:
	case ITEM_EQ_NPC_SCRIPT:
		// A message for a bboard or book text.
		pItem = new CItemMessage( id, pDef );
		break;
	case ITEM_STONE_GUILD:
	case ITEM_STONE_TOWN:
	case ITEM_STONE_ROOM:
		pItem = new CItemStone( id, pDef );
		break;
	case ITEM_MULTI:
	case ITEM_SHIP:
		pItem = new CItemMulti( id, pDef );
		break;

	case ITEM_EQ_MEMORY_OBJ:
		pItem = new CItemMemory( id, pDef );
		break;

	default:
		if ( pDef->m_buyvaluemin || pDef->m_sellvaluemin )
			pItem = new CItemVendable( id, pDef );
		else
			pItem = new CItem( id, pDef );
		break;
	}
	ASSERT( pItem );
	if ( idErrorMsg )
	{
		DEBUG_ERR(( "CreateBase invalid item 0%x, UID=0%x\n", idErrorMsg, pItem->GetUID()));
	}
	return( pItem );
}

CItem * CItem::CreateDupeItem( const CItem * pItem )	// static
{
	// Dupe this item.
	if ( pItem == NULL ) 
		return( NULL );
	CItem * pItemNew = CreateBase( pItem->GetID());
	pItemNew->DupeCopy( pItem );
	return( pItemNew );
}

CItem * CItem::CreateScript( ITEMID_TYPE id ) // static
{
	// Create item from the script id.

	CItem * pItem = CreateBase( id );
	ASSERT( pItem );
	ASSERT( pItem->m_pDef );

	CScriptLock s;
	if ( pItem->m_pDef->ScriptLockBase(s))
	{
		while ( s.ReadKeyParse())
		{
			if ( s.IsKeyHead( "ON", 2 ))
				break;
			if ( s.IsKeyHead( "ITEM", 4 ))	// sub items for multi's
				break;
			pItem->r_LoadVal( s );
		}
	}

	if ( pItem->m_type == ITEM_CONTAINER_LOCKED )
	{
		// At this level it just means to add a key for it.
		dynamic_cast <CItemContainer *> ( pItem ) ->MakeKey();
	}

	return( pItem );
}

CItem * CItem::CreateHeader( ITEMID_TYPE id, TCHAR * pArg, CObjBase * pCont )
{
	// Just read info on a single item carryed by the CChar.
	// ITEM=#id,#amount,R#chance

	if ( id == ITEMID_QTY )
	{
		id = (ITEMID_TYPE) Exp_GetHex( pArg );
	}

	if ( id == ITEMID_NOTHING ) 
		return( NULL );

	int amount = 1;
	if ( Parse( pArg, &pArg ))
	{
		if ( pArg[0] != 'R' )
		{
			amount = Exp_GetVal( pArg );
			Parse( pArg, &pArg );
		}
		if ( pArg[0] == 'R' )
		{
			// 1 in x chance of creating this.
			if ( GetRandVal( atoi( pArg+1 )))
				return( NULL );	// don't create it
		}
	}

	if ( amount == 0 ) return( NULL );

	// Is the item movable ?

	CItem * pItem = CItem::CreateTemplate( id, pCont );
	if ( pItem != NULL )
	{
		if ( ! pItem->IsMovableType())
		{
			DEBUG_ERR(( "Script Error: 0%x item is not movable type, cont=0%x\n", id, pCont ? ((DWORD)(pCont->GetUID())) : 0 ));
			pItem->Delete();
			return( NULL );
		}
		if ( amount != 1 )
		{
			pItem->SetAmount( amount );
		}
	}
	return( pItem );
}

const TCHAR * CItem::sm_TemplateTable[] =
{
	"BUY",
	"CONTAINER",
	"ITEM",
	"SELL",
};

CItem * CItem::CreateTemplate( ITEMID_TYPE id, CObjBase * pCont )	// static
{
	// A template is a collection of items. (not a single item)
	// NOTE: This can possibly be reentrant under the current scheme. Fix this.

	if ( id < ITEMID_TEMPLATE )
	{
		return CreateScript( id );
	}

	// ??? Change this to hex and use the pre-index for speed.
	// Fool the index system into looking for the hex number.
	CGString sSec;
	sSec.FormatVal( id );

	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_TEMPLATE_2, sSec ) == NULL )
		return NULL;

	CChar * pVendor;
	CItemContainer * pVendorSell = NULL;
	CItemContainer * pVendorBuy = NULL;
	if ( pCont != NULL )
	{
		pVendor = dynamic_cast <CChar*>( pCont->GetTopLevelObj());
		if ( pVendor != NULL && pVendor->m_pNPC && pVendor->m_pNPC->IsVendor())
		{
			pVendorSell = pVendor->GetBank( LAYER_VENDOR_STOCK );
			pVendorBuy = pVendor->GetBank( LAYER_VENDOR_BUYS );
		}
	}

	bool fNewContainer = false;
	bool fIgnoreAttrib = false;
	CItem * pItem = NULL;
	while ( s.ReadKeyParse())
	{
		switch ( FindTableSorted( s.GetKey(), sm_TemplateTable, COUNTOF( sm_TemplateTable )))
		{
		case 0: // "BUY"
			if (pVendorBuy != NULL)
			{
				pItem = CItem::CreateHeader( ITEMID_QTY, s.GetArgStr(), pCont );
				CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
				if ( pItem == NULL )
				{
					fIgnoreAttrib = true;
					continue;
				}
				if ( pVendItem == NULL )
				{
					g_Log.Event( LOGL_WARN, "Vendor buying non-vendable item: %s\n", pItem->GetName());
					pItem->Delete();
					continue;
				}
				pVendorBuy->ContentAdd( pItem );
				fIgnoreAttrib = false;
			}
			break;
		case 1:	// "CONTAINER"
			goto scpitem;
		case 2: // "ITEM"
scpitem:
			{
			pItem = CItem::CreateHeader( ITEMID_QTY, s.GetArgStr(), pCont );
			if ( pItem == NULL )
			{
				fIgnoreAttrib = true;
				continue;
			}
			fIgnoreAttrib = false;
			if ( pCont != NULL)
			{
				dynamic_cast<CContainer*>(pCont)->ContentAdd( pItem );
			}
			if ( s.IsKey( "CONTAINER" ))
			{
				fNewContainer = true;
				pCont = dynamic_cast <CItemContainer *> ( pItem );
				if ( pCont == NULL )
				{
					DEBUG_ERR(( "CreateTemplate CContainer %0x is not a container\n", id ));
				}
			}
			}
			continue;
		case 3: // "SELL"
			if (pVendorSell != NULL)
			{
				fIgnoreAttrib = true;
				pItem = CItem::CreateHeader( ITEMID_QTY, s.GetArgStr(), pCont );
				CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
				if ( pItem == NULL )
				{
					continue;
				}
				if ( pVendItem == NULL )
				{
					g_Log.Event( LOGL_WARN, "Vendor selling non-vendable item: %s\n", pItem->GetName());
					pItem->Delete();
					continue;
				}
				pVendorSell->ContentAdd( pItem );
				fIgnoreAttrib = false;
			}
			break;
		}

		if ( pItem != NULL && ! fIgnoreAttrib )
		{
			pItem->r_LoadVal( s );
		}
	}

	if ( fNewContainer )
	{
		CItemContainer * pContItem = dynamic_cast<CItemContainer*>(pCont);
		if ( pContItem != NULL )
			return pContItem;
	}
	return( pItem );
}

int CItem::IsWeird() const
{
	// Does item i have a "link to reality"?
	// (Is the container it is in still there)
	// RETURN: 0 = success ok
	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
bailout:
		return( iResultCode );
	}
	if ( m_pDef == NULL )
	{
		iResultCode = 0x2102;
		goto bailout;
	}
	if ( m_pDef->GetID() == 0 )
	{
		iResultCode = 0x2103;
		goto bailout;
	}
	if ( IsDisconnected())	// Should be connected to the world.
	{
		iResultCode = 0x2104;
		goto bailout;
	}
	if ( IsTopLevel())
	{
		// no container = must be at valid point.
		if ( ! GetTopPoint().IsValid())
		{
			iResultCode = 0x2105;
			goto bailout;
		}
		return( 0 );
	}

	// The container must be valid.
	return( GetContainer()->IsWeird());
}

bool CItem::IsValidLockUID() const
{
	// ITEM_KEY
	if (( m_itKey.m_lockUID & UID_MASK ) == UID_MASK )
		return( true );
	if ( m_itKey.m_lockUID == UID_CLEAR )	// blank
		return( false );	

	CItem * pItemLock = m_itKey.m_lockUID.ItemFind();
	if ( pItemLock == NULL )
		return( false );

	// only multi's do not have to reciprocate the lock uid.
	if ( ! CItemBase::IsMultiID( pItemLock->GetDispID()) &&
		pItemLock->m_itContainer.m_lockUID != m_itKey.m_lockUID )
		return( false );
	
	return( true );
}

int CItem::FixWeirdness()
{
	// Check for weirdness and fix it if possible.
	// RETURN: false = i can't fix this.

	if ( m_type == ITEM_EQ_MEMORY_OBJ && ! IsValidUID())
	{
		SetPrivateUID( 0, true );	// some cases we don't get our UID because we are created during load.
	}

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
bailout:
		return( iResultCode );
	}

	CChar * pChar;
	if ( IsEquipped())
	{
		pChar = dynamic_cast <CChar*>( GetParent());
		if ( ! pChar )
		{
			iResultCode = 0x2202;
			goto bailout;
		}
	}
	else
	{
		pChar = NULL;
	}

	// Only stackable items should set amount ?

	if ( GetAmount() != 1 &&
		m_type != ITEM_SPAWN_CHAR &&
		m_type != ITEM_SPAWN_ITEM &&
		! IsStackableException())
	{
		// This should only exist in vendors restock boxes & spawns.
		if ( ! GetAmount())
		{
			SetAmount( 1 );
		}
		// stacks of these should not exist.
		else if ( ! m_pDef->IsStackableType() &&
			GetID() != ITEMID_CORPSE &&
			m_type != ITEM_EQ_MEMORY_OBJ )
		{
			SetAmount( 1 );
		}
	}

	if ( g_World.m_iSaveVersion <= 32 )
	{
		switch ( m_type )
		{
		case ITEM_EQ_HORSE:  // m_Horse_UID
			m_itFigurine.m_UID.SetUID( m_itNormal.m_more1 );
			m_itNormal.m_more1 = 0;
			break;
		case ITEM_EQ_MEMORY_OBJ: // m_Memory_Obj_UID
		case ITEM_SHIP:		 // m_Ship_Tiller_UID
			m_uidLink.SetUID( m_itNormal.m_more1 );
			m_itNormal.m_more1 = 0;
			break;
		case ITEM_EQ_TRADE_WINDOW:	// m_Trade_Partner_UID
		case ITEM_DOOR:
		case ITEM_DOOR_LOCKED: // m_Door_Link_UID
			m_uidLink.SetUID( m_itNormal.m_more2 );
			m_itNormal.m_more2 = 0;
			break;
		}
	}
	if ( g_World.m_iSaveVersion <= 35 )
	{
		switch ( m_type )
		{
		case ITEM_EQ_HORSE:  // m_Horse_UID
			m_itFigurine.m_UID = m_uidLink;
			m_itFigurine.m_ID = CREID_HORSE1;
			m_uidLink.ClearUID();
			break;
		}
	}
	if ( g_World.m_iSaveVersion <= 40 )
	{
		if ( m_type == ITEM_OLD_CASHIERS_CHECK )
		{
			CItemContainer * pBank = dynamic_cast <CItemContainer *>( GetParent());
			if ( pBank && pBank->GetEquipLayer() == LAYER_BANKBOX )
			{
				pBank->m_itEqBankBox.m_Check_Amount = m_itEqBankBox.m_Check_Amount;
				pBank->m_itEqBankBox.m_Check_Restock = m_itEqBankBox.m_Check_Restock;

				iResultCode = 0x2203;	// this is really ok.
				goto bailout;	// get rid of it.
			}
		}
	}
	if ( g_World.m_iSaveVersion <= 42 )
	{
		if ( m_type == ITEM_FIGURINE && IsAttr(ATTR_INVIS) && IsTopLevel())
		{
			m_type = ITEM_SPAWN_CHAR;
		}
	}
	if ( g_World.m_iSaveVersion <= 46 )
	{
		if ( GetID() == ITEMID_Bulletin1 || GetID() == ITEMID_Bulletin2 )
		{
			m_type = ITEM_BBOARD;
		}
		else if ( GetID() == ITEMID_SPELLBOOK )
		{
			m_type = ITEM_SPELLBOOK;
		}
		else if ( GetID() == ITEMID_CORPSE )
		{
			m_type = ITEM_CORPSE;
		}
		if ( m_type == ITEM_RUNE && m_itRune.m_mark_p.IsValid())
		{
			// Fix a rune that was never given a strength.
			if ( m_itRune.m_Strength == 0 ) 
				m_itRune.m_Strength = 100;
		}
	}
	if ( g_World.m_iSaveVersion < 52 )
	{
		if ( m_type == ITEM_EQ_VENDOR_BOX || m_type == ITEM_EQ_BANK_BOX )
		{
			// SetRestockTimeSeconds()
			SetColorAlt( m_itEqBankBox.m_Check_Restock / TICK_PER_SEC );
		}
	}

	if ( m_uidLink.IsValidUID())
	{
		// Make sure the link is valid.
		// Make sure all my memories are valid !
		if ( m_uidLink == GetUID() || m_uidLink.ObjFind() == NULL )
		{
			if ( m_type == ITEM_EQ_MEMORY_OBJ )
			{
				return( false );	// get rid of it.	(this is not an ERROR per se)
			}
			if ( IsAttr(ATTR_STOLEN))
			{
				// The item has been laundered.
				m_uidLink.ClearUID();
			}
			else
			{
				DEBUG_ERR(( "'%s' Bad Link to 0%lx\n", GetName(), (DWORD) m_uidLink ));
				m_uidLink.ClearUID();
				if ( g_World.m_iSaveVersion > 32 )
				{
					iResultCode = 0x2205;
					goto bailout;	// get rid of it.
				}
			}
		}
	}

	switch ( GetID())
	{
	case ITEMID_SPELLBOOK2:	// weird client bug with these.
		SetDispID( ITEMID_SPELLBOOK );
		break;

	case ITEMID_DEATHSHROUD:	// be on a dead person only.
	case ITEMID_GM_ROBE:
		{
			CChar * pChar = dynamic_cast<CChar*>( GetTopLevelObj());
			if ( pChar == NULL )
			{
				iResultCode = 0x2206;
				goto bailout;	// get rid of it.
			}
			if ( GetID() == ITEMID_DEATHSHROUD )
			{
				if ( IsAttr( ATTR_MAGIC ) && IsAttr( ATTR_NEWBIE )) break;	// special
				if ( ! pChar->IsStat( STATF_DEAD ))
				{
					iResultCode = 0x2207;
					goto bailout;	// get rid of it.
				}
			}
			else
			{
				// Only GM/counsel can have robe.
				// Not a GM til read ACCT.SCP
				if ( pChar->GetPrivLevel() < PLEVEL_Counsel )
				{
					iResultCode = 0x2208;
					goto bailout;	// get rid of it.
				}
			}
		}
		break;

	case ITEMID_VENDOR_BOX:
		if ( ! IsEquipped())
		{
			SetDispID(ITEMID_BACKPACK);
		}
		else if ( GetEquipLayer() == LAYER_BANKBOX )
		{
			SetDispID(ITEMID_BANK_BOX);
			m_type = ITEM_EQ_BANK_BOX;
		}
		else
		{
			m_type = ITEM_EQ_VENDOR_BOX;
		}
		break;

	case ITEMID_BANK_BOX:
		// These should only be bank boxes and that is it !
		if ( ! IsEquipped())
		{
			SetDispID(ITEMID_BACKPACK);
		}
		else if ( GetEquipLayer() != LAYER_BANKBOX )
		{
			SetDispID(ITEMID_VENDOR_BOX);
			m_type = ITEM_EQ_VENDOR_BOX;
		}
		else
		{
			m_type = ITEM_EQ_BANK_BOX;
		}
		break;

	case ITEMID_MEMORY:
		// Memory or other flag in my head.
		// Should not exist except equipped.
		m_type = ITEM_EQ_MEMORY_OBJ;
		break;
	}

	switch ( m_type )
	{
	case ITEM_EQ_TRADE_WINDOW:
		// Should not exist except equipped.
		if ( ! IsEquipped() || 
			GetEquipLayer() != LAYER_NONE ||
			pChar->m_pPlayer == NULL )
		{
			iResultCode = 0x2220;
			goto bailout;	// get rid of it.
		}
		break;

	case ITEM_EQ_CLIENT_LINGER:
		// Should not exist except equipped.
		if ( ! IsEquipped() || 
			GetEquipLayer() != LAYER_FLAG_ClientLinger ||
			pChar->m_pPlayer == NULL )
		{
			iResultCode = 0x2221;
			goto bailout;	// get rid of it.
		}
		break;

	case ITEM_EQ_MEMORY_OBJ:
		// Should not exist except equipped.
		if ( dynamic_cast <CItemMemory*>(this) == NULL )
		{
			iResultCode = 0x2222;
			goto bailout;	// get rid of it.
		}
		break;

	case ITEM_EQ_HORSE:
		// These should only exist eqipped.
		if ( ! IsEquipped() || GetEquipLayer() != LAYER_HORSE )
		{
			iResultCode = 0x2226;
			goto bailout;	// get rid of it.
		}
		SetTimeout( 10*TICK_PER_SEC );
		break;

	case ITEM_HAIR:
	case ITEM_BEARD:	// 62 = just for grouping purposes.
		// Hair should only be on person or equipped. (can get lost in pack)
		// Hair should only be on person or on corpse.
		if ( ! IsEquipped())
		{
			CItemContainer * pCont = dynamic_cast <CItemContainer*> (GetParent());
			if ( pCont == NULL ||
				( pCont->GetID() != ITEMID_CORPSE && pCont->GetID() != ITEMID_VENDOR_BOX ))
			{
				iResultCode = 0x2227;
				goto bailout;	// get rid of it.
			}
		}
		else
		{
			if ( GetEquipLayer() != LAYER_HAIR && GetEquipLayer() != LAYER_BEARD )
			{
				iResultCode = 0x2228;
				goto bailout;	// get rid of it.
			}
		}
		break;

	case ITEM_GAME_PIECE:
		// This should only be in a game.
		if ( ! IsInContainer())
		{
			iResultCode = 0x2229;
			goto bailout;	// get rid of it.
		}
		break;

	case ITEM_KEY:
		// blank unlinked keys.
		if ( ! IsValidLockUID())
		{
			m_itKey.m_lockUID.SetUID( UID_CLEAR );
		}
		break;

	case ITEM_SPAWN_CHAR:
		Spawn_SetTrackID();

	case ITEM_SPAWN_ITEM:
		if ( ! IsTimerSet())
		{
			Spawn_OnTick( false );
		}
		break;

	case ITEM_PLANT:
		if ( ! m_itPlant.m_Reap_ID )
		{
			if ( GetID() >= ITEMID_PLANT_HAY1 && GetID() <= ITEMID_PLANT_HAY4 )
				m_itPlant.m_Reap_ID = ITEMID_HAY;
			else
				m_itPlant.m_Reap_ID = ITEMID_COTTON_RAW;
		}
		break;

	case ITEM_DREAM_GATE:
		if ( ! g_Serv.m_Servers.IsValidIndex( m_itDreamGate.m_Server_Index))
		{
			m_itDreamGate.m_Server_Index = 0;
		}
		break;

	case ITEM_CONTAINER_LOCKED:
	case ITEM_DOOR_LOCKED:
		// Doors and containers must have a lock complexity set.
		if ( ! m_itContainer.m_lock_complexity )
		{
			m_itContainer.m_lock_complexity = 500 + GetRandVal( 600 );
		}
		break;

	case ITEM_POTION:
		if ( m_itPotion.m_skillquality == 0 ) // store bought ?
		{
			m_itPotion.m_skillquality = GetRandVal(950);
		}
		break;
	}

	if ( IsEquipped())
	{
		ASSERT( pChar );

		switch ( GetEquipLayer())
		{
		case LAYER_NONE:
			// Only Trade windows should be equipped this way..
			if ( m_type != ITEM_EQ_TRADE_WINDOW )
			{
				iResultCode = 0x2230;
				goto bailout;	// get rid of it.
			}
			break;
		case LAYER_SPECIAL:
			if ( m_type != ITEM_EQ_MEMORY_OBJ && m_type != ITEM_EQ_NPC_SCRIPT )
			{
				iResultCode = 0x2231;
				goto bailout;	// get rid of it.
			}
			break;
		case LAYER_VENDOR_STOCK:
		case LAYER_VENDOR_EXTRA:
		case LAYER_VENDOR_BUYS:
			if ( pChar->m_pPlayer )	// players never need carry these,
				return( false );
			m_Attr |= ATTR_MOVE_NEVER;
			break;

		case LAYER_PACK:
		case LAYER_BANKBOX:
			m_Attr |= ATTR_MOVE_NEVER;
			break;
		case LAYER_UNUSED9:
		case LAYER_PACK2:
			// unused.
			{
				iResultCode = 0x2232;
				goto bailout;	// get rid of it.
			}

		case LAYER_HORSE:
			if ( m_type != ITEM_EQ_HORSE )
			{
				iResultCode = 0x2233;
				goto bailout;	// get rid of it.
			}
			m_Attr |= ATTR_MOVE_NEVER;
			pChar->SetStat( STATF_OnHorse );
			break;
		case LAYER_FLAG_ClientLinger:
			if ( m_type != ITEM_EQ_CLIENT_LINGER )
			{
				iResultCode = 0x2234;
				goto bailout;	// get rid of it.
			}
			break;

		case LAYER_FLAG_Murders:
			if ( ! pChar->m_pPlayer || pChar->m_pPlayer->m_Murders <= 0 )
			{
				iResultCode = 0x2235;
				goto bailout;	// get rid of it.
			}
			break;
		}
	}

	else if ( IsTopLevel())
	{
		if ( GetTimerAdjusted() > 2*60*60 )
		{
			// unreasonably long for a top level item ?
			SetTimeout(1);	
		}
	}

	// is m_pDef just set bad ?
	return( IsWeird());
}

CItem * CItem::CreateDupeSplit( int amount )
{
	// Set this item to have this amount.
	// leave another pile behind.
	// can be 0 size if on vendor.
	ASSERT( amount <= GetAmount());
	CItem * pItemNew = CreateDupeItem( this );
	pItemNew->SetAmount( GetAmount() - amount );
	pItemNew->MoveNearObj( this );
	SetAmountUpdate( amount );
	return( pItemNew );
}

bool CItem::IsSameType( const CObjBase * pObj ) const
{
	const CItem * pItem = dynamic_cast <const CItem*> ( pObj );
	if ( pItem == NULL ) return( false );

	if ( GetColor() != pItem->GetColor()) return( false );

	if ( m_pDef != pItem->m_pDef ) return( false );
	if ( m_type != pItem->m_type ) return( false );

	if ( m_itNormal.m_more1 != pItem->m_itNormal.m_more1 ) return( false );
	if ( m_itNormal.m_more2 != pItem->m_itNormal.m_more2 ) return( false );

	return( true );
}

bool CItem::IsStackableException() const
{
	// IS this normally unstackable type item now stackable ?
	// NOTE: This means the m_amount can be = 0

	if ( IsTopLevel() && IsAttr( ATTR_INVIS ))
		return( true );	// resource tracker.

	// In a vendors box ?
	const CItemContainer * pCont = dynamic_cast <const CItemContainer*>( GetParent());
	if ( pCont == NULL )
		return( false );
	if ( pCont->GetID() != ITEMID_VENDOR_BOX && ! pCont->IsAttr(ATTR_MAGIC))
		return( false );
	return( true );
}

bool CItem::IsStackable( const CItem * pItem ) const
{
	if ( ! m_pDef->IsStackableType())
	{
		// Vendor boxes can stack normally un-stackable stuff.
		if ( ! IsStackableException())
			return( false );
	}

	// try object rotations ex. left and right hand hides ?
	if ( ! IsSameType( pItem ))
		return( false );

	// total should not add up to > 64K !!!
	if ( pItem->GetAmount() > 0xFFFF - GetAmount())
		return( false );

	return( true );
}

bool CItem::Stack( CItem * pItem )
{
	// RETURN:
	//  true = the item got stacked. (it is gone)
	//  false = the item will not stack. (do somethjing else with it)
	//

	if ( ! IsStackable( pItem ))
		return( false );

	// Lost newbie status.
	if ( IsAttr( ATTR_NEWBIE ) != pItem->IsAttr( ATTR_NEWBIE ))
	{
		m_Attr &= ~ATTR_NEWBIE;
	}

	SetAmount( pItem->GetAmount() + GetAmount());
	pItem->Delete();
	return( true );
}

int CItem::GetDecayTime() const
{
	// Return time in seconds that it will take to decay this item.
	// -1 = never decays.

	if ( m_type == ITEM_PLANT ) // Crops "decay" as they grow
	{
		return( g_World.GetNextNewMoon(true) - g_World.GetTime() + GetRandVal(20));
	}

	if ( ! IsAttr( ATTR_CAN_DECAY ))
	{
		if ( IsAttr(ATTR_MOVE_NEVER | ATTR_MOVE_ALWAYS | ATTR_OWNED | ATTR_INVIS | ATTR_STATIC ))
			return( -1 );
		if ( ! IsMovableType())
			return( -1 );
	}

	//if ( ! m_pDef->m_buyvalue && ! m_pDef->m_sellvalue )
	//	return( -1 );
	if ( m_type == ITEM_PLANT ) // Crops "decay" as they grow
	{
		return( g_World.GetNextNewMoon(true) - g_World.GetTime() + GetRandVal(20));
	}
	if ( m_type == ITEM_MULTI || m_type == ITEM_SHIP )
	{
		return( 14*24*60*60*TICK_PER_SEC );	// days to rot a structure.
	}
	if ( IsAttr(ATTR_MAGIC))
	{
		// Magic items rot slower.
		return( 4*g_Serv.m_iDecay_Item );
	}
	if ( IsAttr(ATTR_NEWBIE))
	{
		// Newbie items should rot faster.
		return( 5*60*TICK_PER_SEC );
	}
	return( g_Serv.m_iDecay_Item );
}

void CItem::SetTimeout( int iDelay )
{
	// PURPOSE:
	//  Set delay in TICK_PER_SEC of a sec.
	//  -1 = never.
	// NOTE: 
	//  It may be a decay timer or it might be a trigger timer

	CObjBase::SetTimeout( iDelay );

	// Items on the ground must be put in sector list correctly.
	if ( ! IsTopLevel())
		return;

	CSector * pSector = GetTopSector();
	ASSERT( pSector );
	DEBUG_CHECK( GetParent() == &(pSector->m_Items_Inert) || GetParent() == &(pSector->m_Items_Timer));

	// remove from previous list and put in new.
	if ( iDelay < 0 )
	{
		pSector->m_Items_Inert.AddToSector( this );
	}
	else
	{
		pSector->m_Items_Timer.AddToSector( this );
	}
	SetContainerFlags(0);
}

void CItem::SetDecayTime( int iTime )
{
	// iTime = 0 = set default. (TICK_PER_SEC of a sec)
	// -1 = set none. (clear it)

	if ( IsTimerSet() && ! IsAttr(ATTR_DECAY))
	{
		return;	// already a timer here. let it expire
	}
	if ( ! iTime )
	{
		if ( IsTopLevel())
		{
			iTime = GetDecayTime();
		}
		else
		{
			iTime = -1;
		}
	}
	SetTimeout( iTime );
	if ( iTime != -1 )
	{
		m_Attr |= ATTR_DECAY;
	}
	else
	{
		m_Attr &= ~ATTR_DECAY;
	}
}

SOUND_TYPE CItem::GetDropSound( void ) const
{
	// Get a special drop sound for the item.

	if ( m_pDef->m_type == ITEM_COIN )	// ITEMID_GOLD
	{
		// depends on amount.
		switch ( GetAmount())
		{
		case 1: return( 0x035 );
		case 2: return( 0x032 );
		case 3:
		case 4:	return( 0x036 );
		}
		return( 0x037 );
	}
	if ( m_pDef->m_type == ITEM_GEM )
	{
		return(( GetID() > ITEMID_GEMS ) ? 0x034 : 0x032 );  // Small vs Large Gems
	}
	//if ( m_pDef->m_type == ITEM_INGOT )  // Any Ingot
	//{
	//	return( 0x033 );
	//}

	// normal drop sound for what dropped in.
	return( 0 );
}

bool CItem::MoveTo( CPointMap pt ) // Put item on the ground here.
{
	// Move this item to it's point in the world. (ground/top level)
	if ( ! pt.IsValid())
		return false;

	CSector * pSector = pt.GetSector();
	ASSERT( pSector );
	if ( IsTimerSet())	// possibly a decay time.
	{
		pSector->m_Items_Timer.AddToSector( this ); // remove from previous spot and put in new.
	}
	else
	{
		pSector->m_Items_Inert.AddToSector( this );
	}

	// Is this area too complex ?
	if ( ! g_Serv.IsLoading())
	{
		int iCount = pSector->m_Items_Timer.GetCount() + pSector->m_Items_Inert.GetCount();
		if ( iCount > 1024 )
		{
			DEBUG_ERR(( "Warning: %d items at %s,too complex!\n", iCount, pt.Write()));
		}
	}

	SetTopPoint( pt );
	ASSERT( IsTopLevel());	// on the ground.
	ASSERT( ! CObjBase::IsWeird());

	Update();
	return( true );
}

void CItem::MoveToDecay( CPointMap pt )
{
	// Set the decay timer for this if not in a house or such.
	int iDecayTime = GetDecayTime();
	if ( iDecayTime > 0 )
	{
		// In a place where it will decay ?
		CRegionBase * pRegion = pt.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA | REGION_TYPE_ROOM );
		if ( pRegion != NULL && pRegion->IsFlag( REGION_FLAG_NODECAY ))	// No decay here in my house/boat
			iDecayTime = -1;
	}
	SetDecayTime( iDecayTime );
	MoveTo( pt );
}

bool CItem::MoveToCheck( CPointMap pt )
{
	// Make noise and try to pile it and such.

	ASSERT( pt.IsValid());

	int iCount = 0;

	// Look for pileable ? count how many items are here.
	CWorldSearch AreaItems( pt );
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL )
			break;
		iCount ++;
		if ( Stack( pItem ))
			break;
	}

	WORD sound = GetDropSound();
	if ( !sound ) sound = 0x042;	// drop on ground sound = default
	Sound( sound );

	MoveToDecay( pt );

	// g_Serv.m_iMaxComplexity
	if ( iCount > 25 )
	{
		// Too many items here !!! ???
		Speak( "Too many items here!" );
		if ( iCount > 32 )
		{
			Speak( "The ground collapses!" );
			Delete();
		}
		// attempt to reject the move.
		return( false );
	}

	return( true );
}

void CItem::MoveNearObj( const CObjBaseTemplate * pObj, int iSteps, WORD wCan )
{
	// Put in the same container as another item.
	CItemContainer * pPack = (dynamic_cast <CItemContainer*> (pObj->GetParent()));
	if ( pPack != NULL )
	{
		// Put in same container (make sure they don't get re-combined)
		pPack->ContentAdd( this, pObj->GetContainedPoint());
	}
	else
	{
		// Equipped or on the ground so put on ground nearby.
		CObjBase::MoveNearObj( pObj, iSteps, wCan );
	}
}

int CItem::GetVendorPrice( bool fBuyFromVendor ) const
{
	// Get the price on the specfic vendor.
	// RETURN: 0 = invalid sale item.

	CItemContainer * pCont = dynamic_cast <CItemContainer *>(GetParent());
	if ( pCont == NULL )
		return( 0 );
	if ( pCont->GetID() != ITEMID_VENDOR_BOX || ! pCont->IsEquipped())
		return( 0 );
	if ( IsAttr( ATTR_FORSALE ) && GetPlayerVendorPrice() )
	{
		return( GetPlayerVendorPrice() );
	}
	return( GetBasePrice( fBuyFromVendor ));
}

int CItem::GetBasePrice( bool fBuyFromVendor ) const
{
	// Estimate base price for a single item. (Do not factor in amount)
	// RETURN: 0 = some error. or unpriced.

	int iPrice = 0;
	if ( fBuyFromVendor )
	{
		iPrice = m_pDef->m_buyvaluemin;
		if ( ! iPrice && m_pDef->m_sellvaluemin ) iPrice = m_pDef->m_sellvaluemin * 2;
	}
	else
	{
		iPrice = m_pDef->m_sellvaluemin;
		if ( ! iPrice && m_pDef->m_buyvaluemin) iPrice = m_pDef->m_buyvaluemin / 2;
	}

	// ??? factor in magic and quality values.

	return( iPrice );
}

bool CItem::IsValidNPCSaleItem() const
{
	// This item is in an NPC's vendor box.
	// Is it a valid item that NPC's should be selling ?

	if ( ! GetVendorPrice( true ))
	{
		DEBUG_ERR(( "Vendor uid=0%lx selling unpriced item 0%x='%s'\n", GetTopLevelObj()->GetUID(), GetID(), GetName()));
		return( false );
	}
	return( IsValidSaleItem( true ));
}

bool CItem::IsValidSaleItem( bool fBuyFromVendor ) const
{
	// Can this individual item be sold or bought ?
	if ( ! IsMovableType())
	{
		DEBUG_ERR(( "Vendor uid=0%lx selling unmovable item 0%x='%s'\n", GetTopLevelObj()->GetUID(), GetID(), GetName()));
		return( false );
	}
	if ( ! fBuyFromVendor )
	{
		if ( IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER)) 
			return( false );	// spellbooks !
	}
	if ( m_type == ITEM_COIN )
		return( false );
	return( true );
}

const TCHAR * CItem::GetName() const
{
	// allow some items to go unnamed (just use type name).
	const TCHAR * pszNameBase;
	if ( IsIndividualName())
	{
		pszNameBase = CObjBase::GetName();
	}
	else if ( m_pDef->HasTypeName())
	{
		// Just use it's type name instead.
		pszNameBase = m_pDef->GetTypeName();
	}
	else
	{
		// Get the name of the link.
		const CObjBase * pObj = m_uidLink.ObjFind();
		if ( pObj && pObj != this )
		{
			return( pObj->GetName());
		}
		return( "unnamed" );
	}

	// Watch for names that should be pluralized.
	// Get rid of the % in the names.
	return( CItemBase::GetNamePluralize( pszNameBase, m_amount != 1 ));
}

const TCHAR * CItem::GetArticleAndSpace() const
{
	// get the ame article and a space if needed.
	// Yes I know you can get this from. UFLAG2_A or UFLAG2_AN

	const TCHAR * pName = CObjBase::GetName();
	// if ( pName[0] != '\0' ) return( "the " );

	static const TCHAR Vowels[] = {	'A', 'E', 'I', 'O', 'U'};
	TCHAR chName = toupper(GetName()[0]);
	for (int x = 0; x < sizeof(Vowels); x++)
	{
		if ( chName == Vowels[x])
			return( "an " );
	}
	return( "a " );
}

TCHAR * CItem::GetNameFull( bool fIdentified ) const
{
	const TCHAR * pszName = GetName();
	int len = 0;
	TCHAR * pTemp = GetTempStr();
	bool fPlural;
	if ( GetAmount()==1 || GetID() == ITEMID_CORPSE ) // m_corpse_DispID is m_amount
	{
		if ( ! IsIndividualName())
		{
			len += strcpylen( pTemp+len, GetArticleAndSpace());
		}
		fPlural = false;
	}
	else
	{
		len += sprintf( pTemp+len, "%i ", GetAmount());
		fPlural = true;
	}

	if ( fIdentified && IsAttr(ATTR_CURSED|ATTR_CURSED2|ATTR_BLESSED|ATTR_BLESSED2|ATTR_MAGIC))
	{
		const TCHAR * pszTitle;
		switch ( m_Attr & ( ATTR_CURSED|ATTR_CURSED2|ATTR_BLESSED|ATTR_BLESSED2))
		{
		case (ATTR_CURSED|ATTR_CURSED2):
			pszTitle = "Unholy ";
			goto usetitle;
		case ATTR_CURSED2:
			pszTitle = "Damned ";
			goto usetitle;
		case ATTR_CURSED:
			pszTitle = "Cursed ";
			goto usetitle;
		case (ATTR_BLESSED|ATTR_BLESSED2):
			pszTitle = "Holy ";
			goto usetitle;
		case ATTR_BLESSED2:
			pszTitle = "Sacred ";
			goto usetitle;
		case ATTR_BLESSED:
			pszTitle = "Blessed ";
usetitle:
			len += strcpylen( pTemp+len, pszTitle );
		}

		if ( IsAttr(ATTR_MAGIC))
		{
			if ( m_itWeapon.m_skilllevel && IsArmorWeapon() && m_type != ITEM_WAND )
			{
				// A weapon, (Not a wand)
				len += sprintf( pTemp+len, "%c%d ", ( m_itWeapon.m_skilllevel<0 ) ? '-':'+', abs( m_itWeapon.m_skilllevel ));
			}
			else
			{
				// Don't put "magic" in front of "magic key"
				if ( strnicmp( pszName, "MAGIC", 5 ))
				{
					len += strcpylen( pTemp+len, "magic " );

				}
			}
		}
	}

	// Prefix the name
	switch ( m_type )
	{
	case ITEM_ADVANCE_GATE:
		{
			CCharBase * pDef = CCharBase::FindCharBase( m_itAdvanceGate.m_Type );
			strcpy( pTemp+len, (pDef==NULL)? "Blank Advance Gate" : pDef->GetTradeName());
		}
		return ( pTemp );

	case ITEM_STONE_GUILD:
		len += strcpylen( pTemp+len, "Guildstone for " );
		break;
	case ITEM_STONE_TOWN:
		len += strcpylen( pTemp+len, "Town of " );
		break;
	case ITEM_EQ_MEMORY_OBJ:
		len += strcpylen( pTemp+len, "Memory of " );
		break;
	case ITEM_SPAWN_CHAR:
		if ( ! IsIndividualName()) 
			len += strcpylen( pTemp+len, "Spawn " );
		break;

	case ITEM_KEY:
		if ( m_itKey.m_lockUID == UID_CLEAR )
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		break;
	case ITEM_RUNE:
		if ( ! m_itRune.m_mark_p.IsValid())
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		if ( ! m_itRune.m_Strength )
		{
			len += strcpylen( pTemp+len, "Faded " );
		}
		break;

	case ITEM_TELEPAD:
		if ( ! m_itTelepad.m_mark_p.IsValid())
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		break;
	}

	len += strcpylen( pTemp+len, pszName );

	// Suffix the name.

	if ( fIdentified &&
		IsAttr(ATTR_MAGIC) &&
		IsArmorWeapon())	// wand is also a weapon.
	{
		if ( g_Serv.m_SpellDefs.IsValidIndex( m_itWeapon.m_spell ))
		{
			len += sprintf( pTemp+len, " of %s", g_Serv.m_SpellDefs[ m_itWeapon.m_spell ]->GetName());
		}
		if (m_itWeapon.m_charges)
		{
			len += sprintf( pTemp+len, " (%d charges)", m_itWeapon.m_charges );
		}
	}

	switch ( m_type )
	{

	case ITEM_ARCHERY_BUTTE:
		len += sprintf( pTemp+len, " %d %ss", m_itArcheryButte.m_AmmoCount, ( m_itArcheryButte.m_AmmoType == ITEMID_Arrow ) ? "arrow" : "bolt" );
		break;

	case ITEM_STONE_TOWN:
		{
			const CItemStone * pStone = dynamic_cast <const CItemStone*>(this);
			ASSERT(pStone);
			len += sprintf( pTemp+len, " (pop:%d)", pStone->GetCount());
		}
		break;
	}

	if ( IsAttr(ATTR_STOLEN))
	{
		// Who is it stolen from ?
		CChar * pChar = m_uidLink.CharFind();
		if ( pChar )
		{
			len += sprintf( pTemp+len, " (stolen from %s)", pChar->GetName());
		}
		else
		{
			len += sprintf( pTemp+len, " (stolen)" );
		}
	}

	return( pTemp );
}

bool CItem::SetName( const TCHAR * pszName )
{
	// Can't be a dupe name with type ?
	ASSERT(pszName);

	const TCHAR * pszTypeName = m_pDef->GetTypeName();
	if ( ! strnicmp( pszName, "a ", 2 ))
	{
		if ( ! strcmpi( pszTypeName, pszName+2 ))
			pszName = "";
	}
	else if ( ! strnicmp( pszName, "an ", 3 ))
	{
		if ( ! strcmpi( pszTypeName, pszName+3 ))
			pszName = "";
	}
	return SetNamePool( pszName );
}

bool CItem::SetBase( CItemBase * pDef )
{
	// Total change of type. (possibly dangerous)
	if ( pDef == NULL || pDef == m_pDef ) 
		return false;

	// This could be a weight change for my parent !!!
	CContainer * pParentCont = dynamic_cast <CContainer*> (GetParent());
	int iWeightOld = 0;

	if ( m_pDef)
	{
		if ( pParentCont )
		{
			iWeightOld = GetWeight();
		}
		m_pDef->DelInstance();
	}

	m_pDef = pDef;
	m_pDef->AddInstance();

	if (pParentCont)
	{
		ASSERT( IsEquipped() || IsInContainer());
		pParentCont->OnWeightChange( GetWeight() - iWeightOld );
	}

	m_type = m_pDef->m_type;	// might change the type.
	return( true );
}

bool CItem::SetBaseID( ITEMID_TYPE id )
{
	CItemBase * pDef = CItemBase::FindItemBase( id );
	if ( pDef == NULL )
	{
		DEBUG_ERR(( "SetDispID 0%x invalid item uid=0%x\n", id, GetUID()));
		return false;
	}
	SetBase( pDef );	// set new m_type etc
	return( true );
}

bool CItem::SetDispID( ITEMID_TYPE id )
{
	// Converting the type of an existing item is possibly risky.
	// Creating invalid objects of any sort can crash clients.
	// User created items can be overwritten if we do this twice !
	if ( id == GetDispID())
		return true;	// no need to do this again or overwrite user created item types.

	if ( ! IsSameDispID( id ))
	{
		if ( ! SetBaseID( id ))
			return( false );
	}

	if ( CItemBase::IsValidDispID(id))
	{
		m_id = id;
	}
	else
	{
		m_id = m_pDef->GetDispID();
	}
	return( true );
}

void CItem::SetAmount( int amount )
{
	// propagate the weight change.
	int oldamount = GetAmount();
	if ( oldamount == amount )
		return;
	m_amount = amount;

	// sometimes the diff graphics for the types are not in the client.
	if ( m_type == ITEM_ORE )
	{
		static const ITEMID_TYPE Item_Ore[] = 
		{ 
			ITEMID_ORE_1, ITEMID_ORE_1, ITEMID_ORE_2, ITEMID_ORE_3
		};
		m_id = ( GetAmount() >= COUNTOF(Item_Ore)) ? ITEMID_ORE_4 : Item_Ore[GetAmount()];
	}

	CContainer * pParentCont = dynamic_cast <CContainer*> (GetParent());
	if (pParentCont)
	{
		ASSERT( IsEquipped() || IsInContainer());
		pParentCont->OnWeightChange(( amount - oldamount ) * m_pDef->GetWeight());
	}
}

void CItem::SetAmountUpdate( int amount )
{
	int oldamount = GetAmount();
	SetAmount( amount );
	if ( oldamount < 5 || amount < 5 )	// beyond this make no visual diff.
	{
		// Strange client thing. You have to remove before it will update this.
		RemoveFromView();
	}
	Update();
}

void CItem::WriteUOX( CScript & s, int index ) 
{
	DEBUG_CHECK( IsTopLevel());

	s.Printf( "SECTION WORLDITEM %d\n", index );
	s.Printf( "{\n" );
	s.Printf( "SERIAL %d\n", GetUID());
	s.Printf( "NAME %s\n", GetName());
	s.Printf( "ID %d\n", GetDispID());
	s.Printf( "X %d\n", GetTopPoint().m_x );
	s.Printf( "Y %d\n", GetTopPoint().m_y );
	s.Printf( "Z %d\n", GetTopZ());
	s.Printf( "CONT %d\n", -1 );
	s.Printf( "TYPE %d\n", m_type );
	s.Printf( "AMOUNT %d\n", m_amount );
	s.Printf( "COLOR %d\n", GetColor());
	//ATT 5
	//VALUE 1
	s.Printf( "}\n\n" );
}

void CItem::r_Write( CScript & s )
{
	DEBUG_CHECK( ! IsWeird());

	s.WriteSection( "WORLDITEM 0%x", GetID());

#if 0 // def _DEBUG
	if ( GetID() == 0x0e85 )
	{
		ASSERT( GetID() == 0x0e85 );
	}
#endif

	long lPrice;
	if ( IsAttr( ATTR_FORSALE ))
	{
		lPrice = GetPlayerVendorPrice();
		SetPlayerVendorPrice( 0 );	// Fake out the timer for now.
	}

	CObjBase::r_Write( s );

	bool fWeird = false;	// spawn points need this.
	if ( GetDispID() != GetID())
	{
		fWeird = true;	// Find out why we need this to keep from messing up spawns !
		s.WriteKeyHex( "ID", GetDispID());
	}
	if ( GetAmount() != 1 )
		s.WriteKeyVal( "AMOUNT", GetAmount());
	if ( m_type != m_pDef->m_type || fWeird )
		s.WriteKeyVal( "TYPE", m_type );
	if ( m_uidLink.IsValidUID())
		s.WriteKeyHex( "LINK", m_uidLink );
	if ( m_Attr )
		s.WriteKeyHex( "ATTR", m_Attr );

	if ( m_itNormal.m_more1 )
		s.WriteKeyHex( "MORE1", m_itNormal.m_more1 );
	if ( m_itNormal.m_more2 )
		s.WriteKeyHex( "MORE2", m_itNormal.m_more2 );
	if ( m_itNormal.m_morep.m_x || m_itNormal.m_morep.m_y || m_itNormal.m_morep.m_z )
		s.WriteKey( "MOREP", m_itNormal.m_morep.Write());

	CObjBase * pCont = GetContainer();
	if ( pCont != NULL )
	{
		if ( pCont->IsChar())
		{
			if ( GetEquipLayer() >= LAYER_HORSE )
				s.WriteKeyVal( "LAYER", GetEquipLayer());
		}
		s.WriteKeyHex( "CONT", pCont->GetUID());
		if ( pCont->IsItem())
		{
			// Where in the container?
			s.WriteKey( "P", GetContainedPoint().Write());
		}
	}
	else
	{
		s.WriteKey( "P", GetTopPoint().Write());
	}

	// Price must be after container.
	if ( IsAttr( ATTR_FORSALE ) && lPrice )
	{
		s.WriteKeyVal( "PRICE", lPrice );
		SetPlayerVendorPrice( lPrice );
	}
}

bool CItem::LoadSetContainer( CObjUID uid, LAYER_TYPE layer )
{
	// Set the CItem in a container of some sort.
	// Used only during load.
	// "CONT"

	CObjBase * pObjCont = uid.ObjFind();
	if ( pObjCont == NULL )
	{
		DEBUG_ERR(( "Invalid container 0%lx\n", (DWORD) uid ));
		return( false );	// not valid object.
	}

	WORD id;
	if ( pObjCont->IsItem())
	{
		CItemContainer * pCont = dynamic_cast <CItemContainer *> (pObjCont);
		if ( pCont != NULL )
		{
			// Is this in a vendor's bankbox?
			if ( pCont->GetID() == ITEMID_VENDOR_BOX )
			{
				CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (this);
				if ( pVendItem != NULL )
					pVendItem->Restock();
			}
			pCont->ContentAdd( this );
			return( true );
		}
		id = pCont->GetID();
	}
	else
	{
		CChar * pChar = dynamic_cast <CChar *> (pObjCont);
		if ( pChar != NULL )
		{
			// equip the item
			pChar->LayerAdd( this, layer ? layer : m_pDef->m_layer );
			return( true );
		}
		id = 0;
	}

	DEBUG_ERR(( "Non container uid=0%lx,id=0%x\n", (DWORD) uid, id ));
	return( false );	// not a container.
}

const TCHAR * CItem::sm_KeyTable[] =
{
	"AMOUNT",
	"ATTR",
	"CONT",
	"DISPID",
	"DISPIDDEC",
	"FRUIT",
	"HITPOINTS",	// for weapons
	"ID",
	"LAYER",
	"LINK",
	"MORE",
	"MORE1",
	"MORE2",
	"MOREP",
	"MOREX",
	"MOREY",
	"MOREZ",
	"P",
	"PRICE",
	"TYPE",
};

bool CItem::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	if ( ! strnicmp( pszKey, "LINK.", 5 ))
	{
		pszKey += 5;
		pRef = m_uidLink.ObjFind();
		return( true );
	}
	if ( ! strnicmp( pszKey, "CONT.", 5 ))
	{
		pszKey += 5;
		pRef = GetContainer();
		return( true );
	}
	return( CObjBase::r_GetRef( pszKey, pRef, pSrc ));
}

bool CItem::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "AMOUNT"
		sVal.FormatVal( GetAmount());
		break;
	case 1: // "ATTR"
		sVal.FormatHex( m_Attr );
		break;
	case 2:	// "CONT"
		return( false );
	case 3: // "DISPID"
		sVal.FormatHex( GetDispID());
		break;
	case 4: // "DISPIDDEC"
		{
			int iVal = GetDispID();
	 		if ( m_type == ITEM_COIN ) // Fix money piles
			{
				iVal = m_pDef->GetDispID();
				if ( GetAmount() < 2 )
					iVal = iVal;
				else if ( GetAmount() < 6)
					iVal = iVal + 1;
				else
					iVal = iVal + 2;
			}
			sVal.FormatVal( iVal );
		}
		break;
	case 5: // "FRUIT"
		goto dodefault;
	case 6: // "HITPOINTS"
		if ( ! IsArmorWeapon()) 
			return( false );
		sVal.FormatVal( m_itArmor.m_Hits_Cur );
		break;
	case 7:	// "ID"
		goto dodefault;
	case 8:	// "LAYER"
		goto dodefault;
	case 9: // "LINK"
		sVal.FormatHex( m_uidLink );
		break;
	case 10:	// "MORE"
		goto scp_more1;
	case 11:	// "MORE1"
scp_more1:
		sVal.FormatHex( m_itNormal.m_more1 );
		break;
	case 12:	// "MORE2"
		sVal.FormatHex( m_itNormal.m_more2 );
		break;
	case 13: // "MOREP"
		sVal = m_itNormal.m_morep.Write();
		break;
	case 14: // "MOREX",
		sVal.FormatVal( m_itNormal.m_morep.m_x );
		break;
	case 15: // "MOREY",
		sVal.FormatVal( m_itNormal.m_morep.m_y );
		break;
	case 16: // "MOREZ",
		sVal.FormatVal( m_itNormal.m_morep.m_z );
		break;
	case 17: // "P"
		goto dodefault;
	case 18: // "PRICE"
		if ( ! IsAttr( ATTR_FORSALE )) 
			return( false );
		sVal.FormatVal( GetPlayerVendorPrice() );
		break;
	case 19: // "TYPE"
		sVal.FormatVal( m_type );
		break;
	default:
dodefault:
		if ( IsAttr(ATTR_FORSALE) && !strcmpi(pKey,"TIMER"))
		{
			sVal.FormatVal( GetPlayerVendorPrice() );
			break;
			// return( false );
		}
		if ( m_type == ITEM_DREAM_GATE && g_Serv.m_Servers.IsValidIndex( m_itDreamGate.m_Server_Index ))
		{
			if ( g_Serv.m_Servers[m_itDreamGate.m_Server_Index]->r_WriteVal( pKey, sVal, pSrc ))
				return( true );
		}
		return( CObjBase::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CItem::r_LoadVal( CScript & s ) // Load an item Script
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "AMOUNT"
		SetAmount( s.GetArgVal());
		return true;
	case 1: // "ATTR"
		m_Attr = s.GetArgHex();
		return true;
	case 2:	// "CONT" needs special processing.
		// Loading or import.
		DEBUG_CHECK( IsDisconnected());
		if ( ! IsDisconnected()) 
		{
			return( false );
		}
		return LoadSetContainer( s.GetArgHex(), (LAYER_TYPE) GetUnkZ());
	case 3: // "DISPID"
		return SetDispID((ITEMID_TYPE) s.GetArgHex());
	case 4: // "DISPIDDEC"
		return SetDispID((ITEMID_TYPE) s.GetArgVal());
	case 5:	// "FRUIT" = m_more1
		m_itPlant.m_Reap_ID = (ITEMID_TYPE) s.GetArgHex();
		return true;
	case 6:	// "HITPOINTS" = from ITEMS.SCP only
		if ( IsArmorWeapon())
		{
			m_itArmor.m_Hits_Cur = m_itArmor.m_Hits_Max = s.GetArgVal();
			return true;
		}
		else
		{
			DEBUG_ERR(( "Item:Hitpoints assigned for non-weapon 0%x\n", GetID()));
		}
		break;
	case 7:	// "ID"
		return SetDispID((ITEMID_TYPE) s.GetArgHex());
	case 8: // "LAYER"
		// used only during load.
		if ( ! IsDisconnected() && ! IsInContainer()) 
		{
			return( false );
		}
		SetUnkZ( s.GetArgVal()); // GetEquipLayer()
		return true;
	case 9: // "LINK"
		m_uidLink = s.GetArgHex();
		return true;

	case 10:	// "MORE"
		goto scp_more1;
	case 11:	// "MORE1"
scp_more1:
		m_itNormal.m_more1 = s.GetArgHex();	
		return true;
	case 12:	// "MORE2"
		m_itNormal.m_more2 = s.GetArgHex();
		return true;
	case 13: // "MOREP"
		m_itNormal.m_morep.Read(s.GetArgStr());
		return true;
	case 14: // "MOREX",
		m_itNormal.m_morep.m_x = s.GetArgVal();
		return true;
	case 15: // "MOREY",
		m_itNormal.m_morep.m_y = s.GetArgVal();
		return true;
	case 16: // "MOREZ",
		m_itNormal.m_morep.m_z = s.GetArgVal();
		return true;
	case 17: // "P"
		// Loading or import.
		if ( ! IsDisconnected() && ! IsInContainer()) 
		{
			DEBUG_CHECK( IsDisconnected() || IsInContainer());
			return( false );
		}
		SetUnkPoint( s.GetArgStr());
		return true;

	case 18:	// "PRICE"
		if ( IsAttr( ATTR_FORSALE ))
		{
			SetPlayerVendorPrice( s.GetArgVal());
		}
		return true;
	case 19: // "TYPE"
		m_type = (ITEM_TYPE) s.GetArgVal();
		return true;
	}

	return( CObjBase::r_LoadVal( s ));
}

bool CItem::r_Load( CScript & s ) // Load an item from script
{
	CScriptObj::r_Load( s );
	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
killit:
		DEBUG_ERR(( "Item 0%x Invalid, id=0%x, code=0%x\n", GetUID(), GetID(), iResultCode ));
		Delete();
		return( true );
	}

	if ( GetContainer() == NULL )	// Place into the world.
	{
		if ( ! GetTopPoint().IsValid())
			goto killit;
		MoveTo( GetTopPoint());
	}
	return( true );
}

bool CItem::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	static const TCHAR * table[] =
	{
		"BOUNCE",
		"DROP",
		"DUPE",
		"EQUIP",	// engage the equip triggers.
		"UNEQUIP",	// engage the unequip triggers.
	};

	CChar * pCharSrc = pSrc->GetChar();

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF( table )))
	{
	case 0:	// "BOUNCE"
		if ( ! pCharSrc ) 
			return( false );
		pCharSrc->ItemBounce( this );
		break;
	case 1: // "DROP"
		if ( ! pCharSrc ) 
			return( false );
		MoveTo( GetTopLevelObj()->GetTopPoint());
		return( true );
	case 2:	// "DUPE"
		{
			int iCount = s.GetArgVal();
			if ( ! iCount ) iCount = 1;
			while ( iCount-- )
			{
				CItem::CreateDupeItem( this )->MoveNearObj( this, 1 );
			}
		}
		break;
	case 3: // "EQUIP"
		if ( ! pCharSrc ) 
			return( false );
		pCharSrc->LayerAdd( this, (s.HasArgs()) ? ((LAYER_TYPE) s.GetArgVal()) : m_pDef->m_layer );
		break;
	case 4: // "UNEQUIP"
		if ( ! pCharSrc ) 
			return( false );
		RemoveSelf();
		pCharSrc->ItemBounce(this);
		break;
	default:
		if ( m_type == ITEM_DREAM_GATE &&
			g_Serv.m_Servers.IsValidIndex( m_itDreamGate.m_Server_Index ))
		{
			if ( g_Serv.m_Servers[m_itDreamGate.m_Server_Index]->r_Verb( s, pSrc ))
				break;
		}
		return( CObjBase::r_Verb( s, pSrc ));
	}
	return( true );
}

void CItem::ConvertBolttoCloth()
{
	if ( ! IsSameDispID( ITEMID_CLOTH_BOLT1 ))
		return;
	// Bolts of cloth are treated as 50 single pieces of cloth
	// Convert it to 50 pieces of cloth, then consume the amount we want later
	SetDispID( ITEMID_CLOTH1 );
	SetAmountUpdate( 50 * GetAmount());
}

bool CItem::OnTrigger( const TCHAR * pszTrigName, CTextConsole * pSrc, int iArg )
{
	// Is there trigger code in the script file ?
	// RETURN:
	//   false = continue default process normally.
	//   true = don't allow this.  (halt further processing)

	if ( pSrc == NULL ) 
	{
		pSrc = &g_Serv;
	}

	// Is the CChar sesative to actions on all items ?

	CChar * pChar = pSrc->GetChar();
	if ( pChar && pChar->m_pPlayer && pChar->m_pPlayer->m_Plot1 )
	{
		// Check all the plot macros that are in effect for the source char.

		DWORD dwPlot = pChar->m_pPlayer->m_Plot1;
		for ( int i=0; dwPlot; i++, dwPlot >>= 1 )
		{
			ASSERT( i < 32 );
			if ( ! ( dwPlot & 1 ))
				continue;
			CScriptLock s;
			if ( ! g_Serv.ScriptLockPlot( s, i ))
			{
				DEBUG_ERR(( "0%x '%s' has unhandled [PLOTITEM %d]\n", pChar->GetUID(), pChar->GetName(), i ));
				continue;
			}
			TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, iArg );
			if ( iRet == TRIGRET_RET_TRUE )
			{
				return( true );	// Block further action.
			}
		}
	}

	// Do special stuff based on item type.

	CScriptLock s;
	if ( m_type >= ITEM_TRIGGER )
	{
		// It has an assigned trigger type.
		if ( ! g_Serv.ScriptLockTrig( s, m_type ))
		{
			DEBUG_ERR(( "0%x '%s' has unhandled [TRIG %d]\n", pChar->GetUID(), pChar->GetName(), m_type ));
			return( false );
		}
	}
	else
	{
		// Look up the trigger in the GRAYITEM.SCP file.
		if ( ! m_pDef->Can( CAN_TRIGGER ))
			return( false );
		if ( ! m_pDef->ScriptLockBase(s))
			return( false );
	}

	bool fRet = ( CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, iArg ) == TRIGRET_RET_TRUE );

	if ( ! fRet )
	{
		int iResultCode = IsWeird();
		if ( iResultCode )
		{
			DEBUG_WARN(("CItem:OnTrigger processing invalid item 0%x, code=0%x\n", GetID(), iResultCode ));
			// return( true );
		}
	}

	return( fRet );
}

void CItem::DupeCopy( const CItem * pItem )
{
	// Dupe this item.

	CObjBase::DupeCopy( pItem );

	m_id = pItem->m_id;
	SetBase( pItem->m_pDef );
	m_type = pItem->m_type;
	m_amount = pItem->m_amount;
	m_Attr  = pItem->m_Attr;

	m_itNormal.m_more1 = pItem->m_itNormal.m_more1;
	m_itNormal.m_more2 = pItem->m_itNormal.m_more2;
	m_itNormal.m_morep = pItem->m_itNormal.m_morep;
}

void CItem::SetAnim( ITEMID_TYPE id, int iTime )
{
	// Set this to an active anim that will revert to old form when done.
	// ??? use addEffect instead !!!
	m_itAnim.m_PrevID = GetID(); // save old id.
	m_itAnim.m_PrevType = m_type;
	SetDispID( id );
	m_type = ITEM_ANIM_ACTIVE;
	SetTimeout( iTime );
	Update();
}

void CItem::Update( const CClient * pClientExclude )
{
	// Send this new item to all that can see it.

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude )
			continue;
		if ( ! pClient->CanSee( this )) 
			continue;

		if ( IsEquipped())
		{
			if ( GetEquipLayer() == LAYER_DRAGGING )
			{
				pClient->addObjectRemove( this );
				continue;
			}
			if ( GetEquipLayer() >= LAYER_SPECIAL )	// nobody cares about this stuff.
				return;
		}
		else if ( IsInContainer())
		{
			if ( dynamic_cast <CItemContainer*> (GetParent())->IsAttr(ATTR_INVIS))
			{
				// used to temporary build corpses.
				pClient->addObjectRemove( this );
				continue;
			}
		}
		pClient->addItem( this );
	}
}

SPELL_TYPE CItem::GetScrollSpell() const
{
	for ( int i=1; i<g_Serv.m_SpellDefs.GetCount(); i++ )
	{
		if ( GetID() == g_Serv.m_SpellDefs[i]->m_ScrollID )
			return((SPELL_TYPE) i );
	}
	return( SPELL_NONE );
}

bool CItem::IsSpellInBook( SPELL_TYPE spell ) const
{
	DEBUG_CHECK(m_type == ITEM_SPELLBOOK);
	int iSpell = spell - SPELL_Clumsy;
	if ( iSpell < 32 )
		return( m_itSpellbook.m_spells1 & (1<<iSpell));
	iSpell -= 32;
	if ( iSpell < 32 )
		return( m_itSpellbook.m_spells2 & (1<<iSpell));
	iSpell -= 32;
	if ( iSpell < 32 )
		return( m_itSpellbook.m_spells3 & (1<<iSpell));
	return( false );
}

int CItem::AddSpellbookScroll( CItem * pItem )
{
	// Add  this scroll to the spellbook.
	// 0 = added.
	// 1 = already here.
	// 2 = not a scroll i know about.

	ASSERT(pItem);
	SPELL_TYPE iSpell = pItem->GetScrollSpell();
	if ( iSpell == SPELL_NONE )
		return( 2 );

	if ( IsSpellInBook(iSpell))
	{
		return( 1 );	// already here.
	}

	// Update the spellbook
	CCommand cmd;
	cmd.ContAdd.m_Cmd = XCMD_ContAdd;
	cmd.ContAdd.m_UID = UID_ITEM + UID_FREE + iSpell;
	cmd.ContAdd.m_id = pItem->GetDispID();
	cmd.ContAdd.m_zero7 = 0;
	cmd.ContAdd.m_amount = iSpell;
	cmd.ContAdd.m_x = 0x48;
	cmd.ContAdd.m_y = 0x7D;
	cmd.ContAdd.m_UIDCont = GetUID();
	cmd.ContAdd.m_color = COLOR_DEFAULT;
	UpdateCanSee( &cmd, sizeof( cmd.ContAdd ));

	// Now add to the spellbook bitmask.
	int i = iSpell;
	i-=SPELL_Clumsy;
	if ( i < 32 )
	{
		m_itSpellbook.m_spells1 |= (1<<i);
	}
	else
	{
		i -= 32;
		if ( i < 32 )
		{
			m_itSpellbook.m_spells2 |= (1<<i);
		}
		else
		{
			i -= 32;
			m_itSpellbook.m_spells3 |= (1<<i);
		}
	}

	pItem->Delete();		// gone.
	return( 0 );
}

void CItem::Flip( const TCHAR * pCmd )
{
	// Equivelant rotational objects.
	// These objects are all the same except they are rotated.

	// Doors are easy.
	if ( m_type == ITEM_DOOR || m_type == ITEM_DOOR_LOCKED )
	{
		ITEMID_TYPE id = GetDispID();
		int doordir = CItemBase::IsDoorID( id )-1;
		int iNewID = id - doordir;	// get base type.
		if ( pCmd )
		{
			if ( ! strcmpi( pCmd, "ALIGN" ))
				iNewID += doordir ^ DOOR_NORTHSOUTH;
			else if ( ! strcmpi( pCmd, "HAND" ))
				iNewID += doordir ^ DOOR_RIGHTLEFT;
			else if ( ! strcmpi( pCmd, "SWING" ))
				iNewID += doordir ^ DOOR_INOUT;
			else
				iNewID += (( doordir &~DOOR_OPENED ) + 2 ) % 16; // next closed door type.
		}
		else
		{
			iNewID += (( doordir &~DOOR_OPENED ) + 2 ) % 16; // next closed door type.
		}
		SetDispID((ITEMID_TYPE) iNewID );
		Update();
		return;
	}

	if ( m_pDef->Can( CAN_I_LIGHT ) ||
		m_pDef->m_type == ITEM_LIGHT_LIT )
	{
		if ( ++m_itLight.m_pattern >= LIGHT_QTY )
			m_itLight.m_pattern = 0;
		Update();
		return;
	}

	if ( GetID() == ITEMID_CORPSE )
	{
		m_itCorpse.m_facing_dir = GetDirTurn((DIR_TYPE) m_itCorpse.m_facing_dir, 1 );
		Update();
		return;
	}

	// Try to rotate the object.
	ITEMID_TYPE id = m_pDef->Flip( GetDispID());
	if ( id != GetDispID())
	{
		SetDispID( id );
		Update();
	}
}

bool CItem::Use_Portculis()
{
	// Set to the new z location.
	DEBUG_CHECK( m_type == ITEM_PORTCULIS || m_type == ITEM_PORT_LOCKED );

	if ( ! IsTopLevel())
		return false;

	CPointMap pt = GetTopPoint();
	if ( pt.m_z == m_itPortculis.m_z1 )
		pt.m_z = m_itPortculis.m_z2;
	else
		pt.m_z = m_itPortculis.m_z1;

	if ( pt.m_z == GetTopZ())
		return false;

	MoveTo( pt );
	Update();
	Sound( 0x21d );
	return( true );
}

SOUND_TYPE CItem::Use_Music( bool fWell ) const
{
	DEBUG_CHECK( m_type == ITEM_MUSICAL );

	SOUND_TYPE wSoundWell = 0;
	SOUND_TYPE wSoundPoor = 0;

	switch ( GetID())
	{
	case ITEMID_MUSIC_DRUM:
		wSoundWell = 0x0038;
		wSoundPoor = 0x0039;
		break;
	case ITEMID_MUSIC_TAMB1:
	case ITEMID_MUSIC_TAMB2:
		wSoundWell = 0x0052;
		wSoundPoor = 0x0053;
		break;
	case ITEMID_MUSIC_HARP_S:
	case ITEMID_MUSIC_HARP_L:
		wSoundWell = 0x0045;
		wSoundPoor = 0x0046;
		break;
	case ITEMID_MUSIC_LUTE1:
	case ITEMID_MUSIC_LUTE2:
		wSoundWell = 0x004C;
		wSoundPoor = 0x004D;
		break;
	}
	return( fWell ? wSoundWell : wSoundPoor );
}

bool CItem::IsDoorOpen() const
{
	DEBUG_CHECK( m_type == ITEM_DOOR || m_type == ITEM_DOOR_LOCKED );

	ITEMID_TYPE id = GetDispID();
  	int doordir = CItemBase::IsDoorID(id)-1;
    DEBUG_CHECK( doordir >= 0 );
    if ( doordir < 0 )
		return false;
    if ( doordir & DOOR_OPENED )
		return true;	
	return( false );
}

bool CItem::Use_Door( bool fJustOpen )
{
	// don't call this directly but call CChar::Use_Item() instead.
	// don't check to see if it is locked here
	// RETURN: 
	//  true = open
	DEBUG_CHECK( m_type == ITEM_DOOR || m_type == ITEM_DOOR_LOCKED );
	ITEMID_TYPE id = GetDispID();
	int doordir = CItemBase::IsDoorID( id )-1;
	DEBUG_CHECK( doordir >= 0 );
	if ( doordir < 0 || ! IsTopLevel())
		return( false );

	id = (ITEMID_TYPE ) ( id - doordir );
	ITEM_TYPE typelock = m_type;

	bool fClosing = ( doordir & DOOR_OPENED );	// currently open
	if ( fJustOpen && fClosing )
		return( true );	// links just open

	CPointMap pt = GetTopPoint();
	switch ( doordir )
	{
	case 0:
		pt.m_x--;
		pt.m_y++;
		doordir++;
		break;
	case DOOR_OPENED:
		pt.m_x++;
		pt.m_y--;
		doordir--;
		break;
	case DOOR_RIGHTLEFT:
		pt.m_x++;
		pt.m_y++;
		doordir++;
		break;
	case (DOOR_RIGHTLEFT+DOOR_OPENED):
		pt.m_x--;
		pt.m_y--;
		doordir--;
		break;
	case DOOR_INOUT:
		pt.m_x--;
		doordir++;
		break;
	case (DOOR_INOUT+DOOR_OPENED):
		pt.m_x++;
		doordir--;
		break;
	case (DOOR_INOUT|DOOR_RIGHTLEFT):
		pt.m_x++;
		pt.m_y--;
		doordir++;
		break;
	case (DOOR_INOUT|DOOR_RIGHTLEFT|DOOR_OPENED):
		pt.m_x--;
		pt.m_y++;
		doordir--;
		break;
	case 8:
		pt.m_x++;
		pt.m_y++;
		doordir++;
		break;
	case 9:
		pt.m_x--;
		pt.m_y--;
		doordir--;
		break;
	case 10:
		pt.m_x++;
		pt.m_y--;
		doordir++;
		break;
	case 11:
		pt.m_x--;
		pt.m_y++;
		doordir--;
		break;
	case 12:
		doordir++;
		break;
	case 13:
		doordir--;
		break;
	case 14:
		pt.m_y--;
		doordir++;
		break;
	case 15:
		pt.m_y++;
		doordir--;
		break;
	}

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "Door: %d,%d\n", id, doordir ));
	}

	SetDispID((ITEMID_TYPE) ( id + doordir ));
	m_type = typelock ;	// preserve the fact that it was locked.
	MoveTo(pt);

	switch ( id )
	{
	case ITEMID_DOOR_SECRET_1:
	case ITEMID_DOOR_SECRET_2:
	case ITEMID_DOOR_SECRET_3:
	case ITEMID_DOOR_SECRET_4:
	case ITEMID_DOOR_SECRET_5:
	case ITEMID_DOOR_SECRET_6:
		Sound( fClosing ? 0x002e : 0x002f );
		break;
	case ITEMID_DOOR_METAL_S:
	case ITEMID_DOOR_BARRED:
	case ITEMID_DOOR_METAL_L:
	case ITEMID_DOOR_IRONGATE_1:
	case ITEMID_DOOR_IRONGATE_2:
		Sound( fClosing ? 0x00f3 : 0x00eb );
		break;
	default:
		Sound( fClosing ? 0x00f1 : 0x00ea );
		break;
	}

	// Auto close the door in n seconds.
	SetTimeout( fClosing ? -1 : 60*TICK_PER_SEC );
	return( ! fClosing );
}

bool CItem::Armor_IsRepairable( void ) const
{
	// We might want to check based on skills:
	// SKILL_BLACKSMITHING (armor)
	// SKILL_BOWERY (xbows)
	// SKILL_TAILORING (leather)
	//

	if ( m_pDef->Can( CAN_I_REPAIR ))
		return( true );
	if ( ! IsArmorWeapon())
		return( false );

	switch ( m_type )
	{
	case ITEM_CLOTHING:
		return( false );	// Not this way anyhow.
	case ITEM_ARMOR:				// some type of armor. (no real action)
		// ??? Bone armor etc is not !
		break;

	case ITEM_WEAPON_MACE_CROOK:
	case ITEM_WEAPON_MACE_PICK:
	case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case ITEM_WEAPON_MACE_STAFF:
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:
	case ITEM_WEAPON_FENCE:
		return( true );

	case ITEM_WEAPON_BOW:
		// Bows are not !
		return( Armor_IsXBow());

	default:
		return( false );
	}

	return( true );
}

int CItem::Armor_GetDefense() const
{
	// Get the defensive value of the armor. plus magic modifiers.
	// Subtract for low hitpoints.
	if ( ! IsArmor())
		return 1;
	int iVal = m_pDef->m_attackBase ;
	iVal = IMULDIV( iVal, Armor_GetRepairPercent(), 100 );
	if ( IsAttr(ATTR_MAGIC))
	{
		iVal += m_itArmor.m_skilllevel;
	}
	return( iVal );
}

int CItem::Weapon_GetAttack() const
{
	// Get the base attack for the weapon plus magic modifiers.
	// Subtract for low hitpoints.
	if ( ! IsWeapon())	// anything can act as a weapon.
		return 1;
	int iVal = m_pDef->m_attackBase + GetRandVal(m_pDef->m_attackRange );
	iVal = IMULDIV( iVal, Armor_GetRepairPercent(), 100 );
	if ( IsAttr(ATTR_MAGIC) && m_type != ITEM_WAND )
	{
		iVal += m_itWeapon.m_skilllevel;
	}
	return( iVal );
}

SKILL_TYPE CItem::Weapon_GetSkill() const
{
	// assuming this is a weapon. What skill does it apply to.

	switch ( m_pDef->m_type )
	{
	case ITEM_WEAPON_MACE_CROOK:
	case ITEM_WEAPON_MACE_PICK:
	case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case ITEM_WEAPON_MACE_STAFF:
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
		return( SKILL_MACEFIGHTING );
	case ITEM_WEAPON_SWORD:
		return( SKILL_SWORDSMANSHIP );
	case ITEM_WEAPON_FENCE:
		return( SKILL_FENCING );
	case ITEM_WEAPON_BOW:
		return( SKILL_ARCHERY );
	}
	return( SKILL_WRESTLING );
}

const TCHAR * CItem::Use_SpyGlass( CChar * pUser ) const
{
	TCHAR * pResult = GetTempStr();
	// Assume we are in water now ?

	CPointMap ptCoords = pUser->GetTopPoint();

#define BASE_SIGHT 26 // 32 (UO_MAP_VIEW_RADAR) is the edge of the radar circle (for the most part)
	float rWeather = (float) ptCoords.GetSector()->GetWeather();
	float rLight = (float) ptCoords.GetSector()->GetLight();
	CGString sSearch;

	// Weather bonus
	float rWeatherSight = rWeather == 0.0 ? (0.25 * BASE_SIGHT) : 0.0;
	// Light level bonus
	float rLightSight = (1.0 - (rLight / 25.0)) * BASE_SIGHT * 0.25;
	int iVisibility = (int) (BASE_SIGHT + rWeatherSight + rLightSight);

	// Check for the nearest land, only check every 4th square for speed
	const CUOMapMeter * pMeter = g_World.GetMapMeter( ptCoords ); // Are we at sea?
	ASSERT(pMeter);
	if ( pMeter->IsTerrainWater())
	{
		// Look for land if at sea
		CPointMap ptLand;
		for (int x = ptCoords.m_x - iVisibility; x <= ptCoords.m_x + iVisibility; x=x+2)
			for (int y = ptCoords.m_y - iVisibility; y <= ptCoords.m_y + iVisibility; y=y+2)
		{
			CPointMap ptCur(x,y,ptCoords.m_z);
			pMeter = g_World.GetMapMeter( ptCur );
			ASSERT(pMeter);
			if ( pMeter->IsTerrainWater())
				break;
			if (ptCoords.GetDist(ptCur) < ptCoords.GetDist(ptLand))
				ptLand = ptCur;
			break;
		}

		if ( ptLand.IsValid())
			sSearch.Format( "You see land to the %s. ", Dirs[ ptCoords.GetDir(ptLand) ] );
		else if (rLight > 3)
			sSearch = "There isn't enough sun light to see land! ";
		else if (rWeather != 0)
			sSearch = "The weather is too rough to make out any land! ";
		else
			sSearch = "You can't see any land. ";
		strcpy( pResult, sSearch );
	}
	else
	{
		pResult[0] = '\0';
	}

	// Check for interesting items, like boats, carpets, etc.., ignore our stuff
	CItem * pItemSighted = NULL;
	CItem * pBoatSighted = NULL;
	int iItemSighted = 0;
	int iBoatSighted = 0;
	CWorldSearch ItemsArea( ptCoords, iVisibility );
	while (true)
	{
		CItem * pItem = ItemsArea.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( pItem == this ) 
			continue;

		int iDist = ptCoords.GetDist(pItem->GetTopPoint());
		if ( iDist > iVisibility ) // See if it's beyond the "horizon"
			continue;
		if ( iDist <= 8 ) // spyglasses are fuzzy up close.
			continue;

		// TODO: Is this item part of a boat or another multi?
		// if so skip it
		// Track boats separately from other items
		if ( iDist < UO_MAP_VIEW_RADAR && // if it's visible in the radar window as a boat, report it
			pItem->m_type == ITEM_SHIP )
		{
			iBoatSighted ++; // Keep a tally of how many we see
			if (iDist < ptCoords.GetDist(pBoatSighted->GetTopPoint())) // Only find closer items to us
			{
				pBoatSighted = pItem;
			}
		}
		else
		{
			iItemSighted ++; // Keep a tally of how much we see
			if (iDist < ptCoords.GetDist(pItemSighted->GetTopPoint())) // Only find the closest item to us, give boats a preference
			{
				pItemSighted = pItem;
			}
		}
	}
	if (iBoatSighted) // Report boat sightings
	{
		DIR_TYPE dir = ptCoords.GetDir(pBoatSighted->GetTopPoint());
		if (iBoatSighted == 1)
			sSearch.Format("You can see a %s to the %s. ", pBoatSighted->GetName(), Dirs[dir] );
		else
			sSearch.Format("You see many ships, one is to the %s. ", Dirs[dir] );
		strcat( pResult, sSearch);
	}

	if (iItemSighted) // Report item sightings, also boats beyond the boat visibility range in the radar screen
	{
		int iDist = ptCoords.GetDist(pItemSighted->GetTopPoint());
		DIR_TYPE dir = ptCoords.GetDir(pItemSighted->GetTopPoint());
		if (iItemSighted == 1)
		{
			if ( iDist > UO_MAP_VIEW_RADAR) // if beyond ship visibility in the radar window, don't be specific
				sSearch.Format("You can see something floating in the water to the %s. ", Dirs[ dir ] );
			else
				sSearch.Format("You can see %s%s to the %s.", pItemSighted->GetArticleAndSpace(), pItemSighted->GetName(), Dirs[ dir ] );
		}
		else
		{
			if ( iDist > UO_MAP_VIEW_RADAR) // if beyond ship visibility in the radar window, don't be specific
				sSearch.Format("You can see many things, one is to the %s. ", Dirs[ dir ] );
			else
				sSearch.Format("You can see many things, one is %s%s to the %s. ", pItemSighted->GetArticleAndSpace(), pItemSighted->GetName(), Dirs[ dir ] );
		}
		strcat( pResult, sSearch);
	}

	// Check for creatures
	CChar * pCharSighted = NULL;
	int iCharSighted = 0;
	CWorldSearch AreaChar( ptCoords, iVisibility );
	while (true)
	{
		CChar * pChar = AreaChar.GetChar();
		if ( pChar == NULL ) break;
		if ( pChar == pUser ) continue;
		if ( pChar->m_pArea->IsFlag(REGION_FLAG_SHIP))
			continue; // skip creatures on ships, etc.
		int iDist = ptCoords.GetDist(pChar->GetTopPoint());
		if ( iDist > iVisibility ) // Can we see it?
			continue;
		iCharSighted ++;
		if ( iDist < ptCoords.GetDist(pCharSighted->GetTopPoint())) // Only find the closest char to us
		{
			pCharSighted = pChar;
		}
	}

	if (iCharSighted > 0) // Report creature sightings, don't be too specific (that's what tracking is for)
	{
		DIR_TYPE dir =  ptCoords.GetDir(pCharSighted->GetTopPoint());

		if (iCharSighted == 1)
			sSearch.Format("You see a creature to the %s", Dirs [dir] );
		else
			sSearch.Format("You can see many creatures, one is to the %s.", Dirs[dir] );
		strcat( pResult, sSearch);
	}
	return pResult;
}

const TCHAR * CItem::Use_Sextant( CPointMap pntCoords ) const
{
	// Conversion from map square to degrees, minutes
	long lLat =  (pntCoords.m_y - g_pntLBThrone.m_y) * 360 * 60 / UO_SIZE_Y;
	long lLong = (pntCoords.m_x - g_pntLBThrone.m_x) * 360 * 60 / UO_SIZE_X_REAL;
	int iLatDeg =  lLat / 60;
	int iLatMin  = lLat % 60;
	int iLongDeg = lLong / 60;
	int iLongMin = lLong % 60;

	TCHAR * pTemp = GetTempStr();
	sprintf( pTemp, "%io%i'%s, %io%i'%s",
		abs(iLatDeg),  abs(iLatMin),  (lLat <= 0) ? "N" : "S",
		abs(iLongDeg), abs(iLongMin), (lLong >= 0) ? "E" : "W");
	return pTemp;
}

bool CItem::Use_Light()
{
	ASSERT( m_type == ITEM_LIGHT_OUT || m_type == ITEM_LIGHT_LIT );

	if ( m_type == ITEM_LIGHT_OUT && IsInContainer())
		return( false );

	ITEMID_TYPE id = m_itLight.m_Light_ID;
	if ( id == ITEMID_NOTHING )
	{
		id = CItemBase::IsLightUnlit( GetDispID());
		if ( id == ITEMID_NOTHING )
		{
			id = CItemBase::IsLightLit( GetDispID());
			if ( id == ITEMID_NOTHING )
				return( false );
		}
	}

	SetDispID( id );	// this will set the new m_typez

	if ( m_type == ITEM_LIGHT_LIT )
	{
		SetTimeout( 10*60*TICK_PER_SEC );
		if ( ! m_itLight.m_charges )
			m_itLight.m_charges = 20;
	}

	Sound( 0x226 );
	Update();
	return( true );
}

bool CItem::Use_LockPick( CChar * pCharSrc )
{
	// This is the locked item.
	// SKILL_LOCKPICKING
	// SKILL_MAGERY
	if ( pCharSrc == NULL ) 
		return false;

	if ( m_type != ITEM_CONTAINER_LOCKED &&
		m_type != ITEM_DOOR_LOCKED )
	{
		pCharSrc->SysMessage( "You cannot unlock that!" );
		return false;
	}

	CChar * pCharTop = dynamic_cast <CChar*>( GetTopLevelObj());
	if ( pCharTop && pCharTop != pCharSrc )
	{
		pCharSrc->SysMessage( "You cannot unlock that where it is!" );
		return false;
	}

	// If we have the key allow unlock using spell.
	if ( pCharSrc->ContentFindKeyFor( this ))
	{
		if ( m_type == ITEM_DOOR_LOCKED )
		{
			m_type=ITEM_DOOR;
			pCharSrc->SysMessage( "You unlock the door.");
		}
		else
		{
			m_type=ITEM_CONTAINER;
			pCharSrc->SysMessage( "You unlock the container.");
		}
		return( true );
	}

	if ( m_type == ITEM_DOOR_LOCKED )
	{
		if ( g_Serv.m_iMagicUnlockDoor == 0 )
		{
			pCharSrc->SysMessage( "This door can only be unlocked with a key." );
			return false;
		}

		// you can get flagged criminal for this action.
		pCharSrc->CheckCrimeSeen( SKILL_SNOOPING, NULL, this, "picking the locked" );

		if ( GetRandVal( g_Serv.m_iMagicUnlockDoor ))
			goto do_failure;
	}

	// If we don't have a key, we still have a *tiny* chance.
	// This is because players could have lockable chests too and we don't want it to be too easy

do_failure:
	if ( m_type == ITEM_DOOR_LOCKED )
	{
		pCharSrc->SysMessage( "You failed to unlock the door.");
	}
	else
	{
		pCharSrc->SysMessage( "You failed to unlock the container.");
	}
	return false;
}

void CItem::SetSwitchState()
{
	if ( m_itSwitch.m_SwitchID )
	{
		ITEMID_TYPE id = m_itSwitch.m_SwitchID;
		m_itSwitch.m_SwitchID = GetDispID();
		SetDispID( id );
	}
}

void CItem::SetTrapState( ITEM_TYPE state, ITEMID_TYPE id, int iTimeSec )
{
	ASSERT( m_type == ITEM_TRAP || m_type == ITEM_TRAP_ACTIVE || m_type == ITEM_TRAP_INACTIVE );
	ASSERT( state == ITEM_TRAP || state == ITEM_TRAP_ACTIVE || state == ITEM_TRAP_INACTIVE );

	if ( ! id ) 
	{
		id = m_itTrap.m_AnimID;
		if ( ! id )
		{
			id = (ITEMID_TYPE)( GetDispID() + 1 );
		}
	}
	if ( ! iTimeSec )
	{
		iTimeSec = 3*TICK_PER_SEC;
	}
	else if ( iTimeSec > 0 && iTimeSec < 0xffff )
	{
		iTimeSec *= TICK_PER_SEC;
	}
	else
	{
		iTimeSec = -1;
	}

	if ( id != GetDispID())
	{
		m_itTrap.m_AnimID = GetDispID(); // save old id.
		SetDispID( id );
	}
	m_type = state;

	SetTimeout( iTimeSec );
	Update();
}

int CItem::Use_Trap()
{
	// We activated a trap somehow.
	// We might not be close to it tho.
	// RETURN:
	//   The amount of damage.

	ASSERT( m_type == ITEM_TRAP || m_type == ITEM_TRAP_ACTIVE );
	if ( m_type == ITEM_TRAP )
	{
		SetTrapState( ITEM_TRAP_ACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wAnimSec );
	}

	if ( ! m_itTrap.m_Damage ) m_itTrap.m_Damage = 2;
	return( m_itTrap.m_Damage );	// base damage done.
}

void CItem::Emote( const TCHAR * pText, CChar * pCharSrc )
{
	CChar * pChar = dynamic_cast <CChar*>( GetTopLevelObj());
	if ( pChar )
	{
		pChar->Emote( pText, NULL, true );
	}
	else if ( pCharSrc )
	{
		pCharSrc->SysMessage( pText );
	}

	// Else emote it to all who can see.
	Speak( pText );
}

void CItem::SetBlessCurse( int iLevel, CChar * pCharSrc )
{
	if ( ! iLevel ) 
		return;

	const TCHAR * pszColor;
	if ( iLevel < 0 )	// curse
	{
		iLevel = - iLevel;
		pszColor = "darkling";
		if ( IsAttr(ATTR_BLESSED|ATTR_BLESSED2))
		{
			if ( iLevel > m_itSpell.m_skilllevel )
			{
				m_Attr &= ~ATTR_BLESSED;
			}
		}
		else
		{
			m_Attr |= ATTR_CURSED;
		}
	}
	else	// bless
	{
		pszColor = "brilliant";
		if ( IsAttr(ATTR_CURSED|ATTR_CURSED2))	// it was cursed.
		{
			if ( iLevel > m_itSpell.m_skilllevel )
			{
				m_Attr &= ~ATTR_CURSED;

				// remove the curse spell. (must not be worn!)
				if ( ! IsEquipped() &&
					IsAttr( ATTR_MAGIC ) &&
					m_itSpell.m_spell == SPELL_Curse )
				{
					m_itSpell.m_skilllevel = 0;
					m_Attr &= ~ATTR_MAGIC;
				}
			}
		}
		else
		{
			m_Attr |= ATTR_BLESSED;
		}
	}

	CGString sMsg;
	sMsg.Format( "%s show%s a %s glow.",
		GetName(),
		IsEquipped() ? "" : "s",
		pszColor );

	Emote( sMsg, pCharSrc );
}

bool CItem::OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iLevel )
{
	//
	// A spell is cast on this item.
	// ARGS:
	//  iLevel = 0-1000 = difficulty. may be slightly larger . how advanced is this spell (might be from a wand)
	//

	if ( OnTrigger( ITRIG_SPELL, pCharSrc, spell ))
		return false;

	if ( m_type == ITEM_WAND )	// try to recharge the wand.
	{
		if ( ! m_itWeapon.m_spell || m_itWeapon.m_spell == spell )
		{
			m_Attr |= ATTR_MAGIC;
			if ( ! m_itWeapon.m_spell || ( pCharSrc && pCharSrc->IsPriv( PRIV_GM )))
			{
				m_itWeapon.m_spell = spell;
				m_itWeapon.m_skilllevel = iLevel/10;	// max charges.
				m_itWeapon.m_charges = 0;
			}

			int ispellbase = CChar::Spell_GetBaseDifficulty(spell);
			if ( ! GetRandVal( GW_GetBellCurve( m_itWeapon.m_charges+3+ispellbase/10, 2 ) / 2 ))
			{
				// Overcharge will explode !
				Emote( "the overcharged wand explodes" );
				g_World.Explode( pCharSrc, GetTopLevelObj()->GetTopPoint(),
					3, 2 + (ispellbase+iLevel)/20,
					DAMAGE_MAGIC | DAMAGE_GENERAL | DAMAGE_HIT );
				Delete();
				return false;
			}
			else
			{
				m_itWeapon.m_charges++;
			}
		}
	}

	WORD uDamage = 0;
	switch ( spell )
	{
	case SPELL_Curse:
		// Is the item blessed ?
		// What is the chance of holding the curse ?
		SetBlessCurse( -iLevel, pCharSrc );
		return true;
	case SPELL_Bless:
		// Is the item cursed ?
		// What is the chance of holding the blessing ?
		SetBlessCurse( iLevel, pCharSrc );
		return true;
	case SPELL_Incognito:
		m_Attr &= ~ATTR_IDENTIFIED;
		return true;
	case SPELL_Lightning:
		Effect( EFFECT_LIGHTNING, ITEMID_NOTHING, pCharSrc );
		break;
	case SPELL_Explosion:
	case SPELL_Fireball:
	case SPELL_Fire_Bolt:
	case SPELL_Fire_Field:
	case SPELL_Flame_Strike:
	case SPELL_Meteor_Swarm:
		uDamage = DAMAGE_FIRE;
		break;

	case SPELL_Magic_Lock:

		if ( pCharSrc == NULL ) 
			return false;

		switch ( m_type )
		{
		// Have to check for keys here or people would be
		// locking up dungeon chests.
		case ITEM_CONTAINER:
			if ( pCharSrc->ContentFindKeyFor( this ))
			{
				m_type=ITEM_CONTAINER_LOCKED;
				pCharSrc->SysMessage( "You lock the container.");
			}
			else
			{
				pCharSrc->SysMessage( "You don't have the key to this container." );
				return false;
			}
			break;
		// Have to check for keys here or people would be
		// locking all the doors in towns
		case ITEM_DOOR:
			if ( pCharSrc->ContentFindKeyFor( this ))
			{
				m_type=ITEM_DOOR_LOCKED;
				pCharSrc->SysMessage( "You lock the door.");
			}
			else
			{
				pCharSrc->SysMessage( "You don't have the key to this door.");
				return false;
			}
			break;
		case ITEM_SHIP_SIDE:
			m_type=ITEM_SHIP_SIDE_LOCKED;
			pCharSrc->SysMessage( "You lock the ship.");
			break;
		default:
			pCharSrc->SysMessage( "You cannot lock that!" );
			return false;
		}
		break;

	case SPELL_Unlock:
		if ( pCharSrc == NULL )
			return false;
		if ( pCharSrc == NULL )
			return false;
		if ( m_type != ITEM_CONTAINER_LOCKED && m_type != ITEM_DOOR_LOCKED )
			return( false );
		if ( ! pCharSrc->Skill_CheckSuccess( SKILL_MAGERY, IMULDIV( m_itContainer.m_lock_complexity, 130, 100 )))
			return( false );
		return Use_LockPick( pCharSrc );

	case SPELL_Mark:
		if ( pCharSrc == NULL )
			return false;

		if ( ! pCharSrc->IsPriv(PRIV_GM))
		{
			if ( m_type != ITEM_RUNE && m_type != ITEM_TELEPAD )
			{
				pCharSrc->SysMessage( "That item is not a recall rune." );
				return false;
			}
			if ( GetTopLevelObj() != pCharSrc )
			{
				// Prevent people from remarking GM teleport pads if they can't pick it up.
				pCharSrc->SysMessage( "The rune must be on your person to mark." );
				return false;
			}
		}

		m_itRune.m_mark_p = pCharSrc->GetTopPoint();
		if ( m_type == ITEM_RUNE )
		{
			m_itRune.m_Strength = pCharSrc->Skill_GetBase(SKILL_MAGERY) / 10;
			CGString sName;
			sName.Format( "Rune to:%s", pCharSrc->m_pArea->GetName());
			SetName( sName );
		}
		break;
	}

	// ??? Potions should explode when hit (etc..)
	if ( g_Serv.m_SpellDefs[spell]->IsSpellType( SPELLFLAG_HARM ))
	{
		OnTakeDamage( 1, pCharSrc, DAMAGE_HIT | DAMAGE_MAGIC | uDamage );
	}

	return( true );
}

int CItem::Armor_GetRepairPercent() const
{
	if ( ! m_itArmor.m_Hits_Max )
		return( 100 );
	return( IMULDIV( m_itArmor.m_Hits_Cur, 100, m_itArmor.m_Hits_Max ));
}

const TCHAR * CItem::Armor_GetRepairDesc() const
{
	if ( m_itArmor.m_Hits_Cur > m_itArmor.m_Hits_Max )
	{
		return "absolutely flawless";
	}
	else if ( m_itArmor.m_Hits_Cur == m_itArmor.m_Hits_Max )
	{
		return  "in full repair";
	}
	else if ( m_itArmor.m_Hits_Cur > m_itArmor.m_Hits_Max / 2 )
	{
		return  "a bit worn";
	}
	else if ( m_itArmor.m_Hits_Cur > m_itArmor.m_Hits_Max / 3 )
	{
		return  "well worn";
	}
	else if ( m_itArmor.m_Hits_Cur > 3 )
	{
		return  "badly damaged";
	}
	else
	{
		return  "about to fall apart";
	}
}

int CItem::OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType )
{
	// Break stuff.
	// Explode potions. freeze and break ? scrolls get wet ?
	//
	// pSrc = The source of the damage. May not be the person wearing the item.
	//
	// RETURN: -1 = destroyed !!!

	if ( iDmg <= 0 ) return( 0 );

	if ( OnTrigger( ITRIG_DAMAGE, pSrc, iDmg ))
	{
		return( 0 );
	}

	const TCHAR * pszMsg = NULL;

	if ( m_pDef->m_Material == MATERIAL_CLOTH &&
		( uType & DAMAGE_FIRE ) &&
		! IsAttr( ATTR_MAGIC ))
	{
		// normal cloth takes special damage from fire.
		goto forcedamage;
	}

	// Break armor etc..
	if ( IsArmorWeapon() && ( uType & ( DAMAGE_HIT|DAMAGE_GOD|DAMAGE_MAGIC|DAMAGE_FIRE )))
	{
		// Server option to slow the rate ???
		if ( GetRandVal( m_itArmor.m_Hits_Max * 16 ) > iDmg )
		{
			return 1;
		}

forcedamage:

		// describe the damage to the item.

		CChar * pChar = dynamic_cast <CChar*> ( GetTopLevelObj());

		if ( m_itArmor.m_Hits_Cur <= 1 )
		{
			m_itArmor.m_Hits_Cur = 0;
			if ( pChar )
			{
				CGString sMsg;
				sMsg.Format( "%s is destroyed", GetName());
				pChar->Emote( sMsg, NULL, true );
			}

			Delete();
			return( -1 );
		}

		m_itArmor.m_Hits_Cur --;

		if ( pSrc )	// tell hitter they scored !
		{
			CGString sMsg;
			if ( pChar && pChar != pSrc )
			{
				sMsg.Format( "You damage %s's %s", pChar->GetName(), GetName());
			}
			else
			{
				sMsg.Format( "You damage the %s", GetName());
			}
			pSrc->SysMessage( sMsg );
		}
		if ( pChar && pChar != pSrc )
		{
			// Tell target they got damaged.
			CGString sMsg;
			if ( m_itArmor.m_Hits_Cur < m_itArmor.m_Hits_Max / 2 )
			{
				int iPercent = Armor_GetRepairPercent();
				if ( pChar->Skill_GetAdjusted( SKILL_ARMSLORE ) / 10 > iPercent )
				{
					sMsg.Format( "Your %s is damaged and looks %s", GetName(), Armor_GetRepairDesc());
				}
			}
			if ( sMsg.IsEmpty())
			{
				sMsg.Format( "Your %s may have been damaged", GetName());
			}
			pChar->SysMessage( sMsg );
		}
		return( 2 );
	}

	if ( m_type == ITEM_POTION && ( uType & ( DAMAGE_HIT|DAMAGE_GOD|DAMAGE_MAGIC|DAMAGE_FIRE )))
	{
		if ( m_itPotion.m_Type >= POTION_EXPLODE_LESS && m_itPotion.m_Type <= POTION_EXPLODE_GREAT )
		{
			g_World.Explode( pSrc, GetTopLevelObj()->GetTopPoint(), 3, 10+GetRandVal(m_itPotion.m_skillquality/40), DAMAGE_GENERAL | DAMAGE_HIT );
			Delete();
			return( -1 );
		}
	}

	if ( m_type == ITEM_WEB )
	{
		if ( ! ( uType & (DAMAGE_FIRE|DAMAGE_HIT|DAMAGE_GOD)))
		{
			if ( pSrc ) pSrc->SysMessage( "You have no effect on the web" );
			return( 0 );
		}

		iDmg = GetRandVal( iDmg ) + 1;
		if ( iDmg > m_itWeb.m_Hits_Cur || ( uType & DAMAGE_FIRE ))
		{
			if ( pSrc ) pSrc->SysMessage( "You destroy the web" );
			if ( GetRandVal( 2 ) || ( uType & DAMAGE_FIRE ))
			{
				Delete();
				return( -1 );
			}
			SetDispID( ITEMID_REAG_SS );
			Update();
			return( 2 );
		}

		if ( pSrc ) pSrc->SysMessage( "You weaken the web" );
		m_itWeb.m_Hits_Cur -= iDmg;
		return( 1 );
	}

	// don't know how to calc damage for this.
	return( 0 );
}

bool CItem::OnExplosion()
{
	// Async explosion.
	// RETURN: true = done. (delete the animation)

	ASSERT( IsTopLevel());
	ASSERT( m_type == ITEM_EXPLOSION );

	if ( ! m_itExplode.m_wFlags ) 
		return( true );

	CChar * pSrc = m_uidLink.CharFind();

	CWorldSearch AreaChars( GetTopPoint(), m_itExplode.m_iDist );
	while (true)
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL ) 
			break;
		if ( GetTopDist3D( pChar ) > m_itExplode.m_iDist )
			continue;
		pChar->Effect( EFFECT_OBJ, ITEMID_FX_EXPLODE_1, pSrc, 9, 6 );
		pChar->OnTakeDamage( m_itExplode.m_iDamage, pSrc, m_itExplode.m_wFlags );
	}

	// Damage objects near by.
	CWorldSearch AreaItems( GetTopPoint(), m_itExplode.m_iDist );
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( pItem == this ) 
			continue;
		if ( GetTopDist3D( pItem ) > m_itExplode.m_iDist )
			continue;
		pItem->OnTakeDamage( m_itExplode.m_iDamage, pSrc, m_itExplode.m_wFlags );
	}

	m_itExplode.m_wFlags = 0;
	SetTimeout( 3 * TICK_PER_SEC );
	return( false );	// don't delete it yet.
}

bool CItem::OnTick()
{
	// Timer expired. Time to do something.
	// RETURN: false = delete it.

	if ( OnTrigger( ITRIG_TIMER, &g_Serv, 0 ))
		return true;

	switch ( m_type )
	{
	case ITEM_LIGHT_LIT:
		// use up the charges that this has .
		if ( m_itLight.m_charges )
		{
			m_itLight.m_charges --;
			SetTimeout( 10*60*TICK_PER_SEC );
			return true;
		}
		break;

	case ITEM_SHIP_PLANK:
		Ship_Plank( false );
		return true;

	case ITEM_EXPLOSION:
		if ( OnExplosion())
			break;
		return true;

	case ITEM_METRONOME:
		Speak( "Tick" );
		SetTimeout( 5*TICK_PER_SEC );
		return true;

	case ITEM_TRAP_ACTIVE:
		SetTrapState( ITEM_TRAP_INACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wAnimSec );
		return( true );

	case ITEM_TRAP_INACTIVE:
		// Set inactive til someone triggers it again.
		if ( m_itTrap.m_fPeriodic )
		{
			SetTrapState( ITEM_TRAP_ACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wResetSec );
		}
		else
		{
			SetTrapState( ITEM_TRAP, GetDispID(), -1 );
		}
		return true;

	case ITEM_ANIM_ACTIVE:
		// reset the anim
		SetDispID( m_itAnim.m_PrevID );
		m_type = m_itAnim.m_PrevType;
		SetTimeout( -1 );
		Update();
		return true;

	case ITEM_DOOR:
	case ITEM_DOOR_LOCKED:	// Temporarily opened locked door.
		// Doors should close.
		Use_Door( false );
		return true;

	case ITEM_WAND:
		// Magic devices.
		// Will regen over time ?
		if ( IsAttr(ATTR_MAGIC ))
		{
			if ( m_itWeapon.m_charges < m_itWeapon.m_skilllevel/10 )
				m_itWeapon.m_charges++;
			SetTimeout( 30*60*TICK_PER_SEC );
			return true;
		}
		break;

	case ITEM_POTION:
		// This is a explode potion ?
		if ( m_itPotion.m_Type >= POTION_EXPLODE_LESS && m_itPotion.m_Type <= POTION_EXPLODE_GREAT )
		{
			// count down the timer. ??
			if ( m_itPotion.m_tick <= 1 )
			{
				// Set it off.
				OnTakeDamage( 1, m_uidLink.CharFind(), DAMAGE_GENERAL | DAMAGE_FIRE );
			}
			else
			{
				m_itPotion.m_tick --;
				CGString sMsg;
				sMsg.FormatVal( m_itPotion.m_tick );
				STATIC_CAST <CObjBase*>(GetTopLevelObj())->Speak( sMsg );
				SetTimeout( 2 * TICK_PER_SEC );
			}
		}
		return true;

	case ITEM_SPAWN_CHAR:
		// Spawn a creature if we are under count.
	case ITEM_SPAWN_ITEM:
		// Spawn an item.
		Spawn_OnTick( true );
		return true;

	case ITEM_PLANT:
		if ( Plant_OnTick())
			return true;
		break;
	}

	switch ( GetID())
	{
	case ITEMID_BEEHIVE:
		// Regenerate honey count
		if ( m_itBeeHive.m_honeycount < 5 ) m_itBeeHive.m_honeycount++;
		SetTimeout( 15*60*TICK_PER_SEC );
		return true;

	case ITEMID_CAMPFIRE:
		SetDispID( ITEMID_EMBERS );
		SetDecayTime( 2*60*TICK_PER_SEC );
		Update();
		return true;
	}

	if ( IsAttr(ATTR_MOVE_NEVER) && ! IsAttr(ATTR_DECAY|ATTR_CAN_DECAY))
	{
		// Just a resource tracking hit on an existing dynamic object.
		// ITEM_WATER || ITEM_ROCK || ITEM_TREE || ITEM_GRASS
		SetTimeout( -1 );
		SetAmount( 0 );	// regen resources ?
		return true;
	}

	// if ( IsAttr(ATTR_DECAY ))
	{
		if ( g_Log.IsLogged( LOGL_TRACE ))
		{
			DEBUG_MSG(( "Time to delete item '%s'\n", GetName()));
		}

		// Spells should expire and clean themselves up.
		// Ground items rot... etc
		return false;	// delete it.
	}

	// DEBUG_ERR(( "Time to do unknown '%s'\n", GetName()));
	return( true );
}

