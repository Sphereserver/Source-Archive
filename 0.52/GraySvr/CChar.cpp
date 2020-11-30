//
// CChar.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.

static const WORD Item_Hair[] = // types of hair.
{
	ITEMID_HAIR_SHORT,
	ITEMID_HAIR_LONG,
	ITEMID_HAIR_PONYTAIL,
	ITEMID_HAIR_MOHAWK,
	ITEMID_HAIR_PAGEBOY,
	ITEMID_HAIR_BUNS,
	ITEMID_HAIR_AFRO,
	ITEMID_HAIR_RECEDING,
	ITEMID_HAIR_2_PIGTAILS,
	ITEMID_HAIR_TOPKNOT,
};

static const WORD Item_Beards[] = // beard types
{
	ITEMID_BEARD_LONG,
	ITEMID_BEARD_SHORT,
	ITEMID_BEARD_GOATEE,
	ITEMID_BEARD_MOUSTACHE,
	ITEMID_BEARD_SH_M,
	ITEMID_BEARD_LG_M,
	ITEMID_BEARD_GO_M,	// Goatee and mustache.
};

/////////////////////////////////////////////////////////////////
// -CChar

CChar * CChar::CreateBasic( CREID_TYPE baseID ) // static
{
	// Create the "basic" NPC. Not NPC or player yet.
	return( new CChar( baseID ));
}

CChar * CChar::CreateNPC( CREID_TYPE baseID )	// static
{
	// Create an NPC
	CChar * pChar = CreateBasic( baseID );
	pChar->NPC_LoadScript(false);
	return( pChar );
}

CChar::CChar( CREID_TYPE baseID ) : CObjBase( false )
{
	g_Serv.StatInc( SERV_STAT_CHARS );	// Count created CChars.

	m_pArea = NULL;
	m_StatFlag = 0;
	if ( g_World.m_fSaveParity ) SetStat( STATF_SaveParity );	// It will get saved next time.

	m_prev_color = COLOR_DEFAULT;
	m_face_dir = DIR_SE;
	m_fonttype = FONT_NORMAL;

	m_defense = 0;

	int i=0;
	for ( ;i<COUNTOF(m_Stat);i++)
	{
		m_Stat[i] = 0;
	}
	for ( i=0; i<COUNTOF(m_StatVal); i++ )
	{
		m_Stat[i] = m_StatVal[i].m_val = 1;
	}

	for ( i=0;i<SKILL_QTY;i++)
	{
		m_Skill[i] = 0;
	}

	m_time_last_regen = m_time_create = g_World.GetTime();
	m_pDef = NULL;

	m_pClient = NULL;	// is the char a logged in player ?
	m_pPlayer = NULL;	// May even be an off-line player !
	m_pNPC	  = NULL;

	SetID( baseID );
	ASSERT( m_pDef );

	m_prev_id = GetID();
	m_food = m_pDef->m_MaxFood;

	Skill_Cleanup();

	// Put in the idle list by default.
	g_World.m_CharsNew.InsertAfter( this );
	ASSERT( IsDisconnected());
}

CChar::~CChar() // Delete character
{
	DeletePrepare();	// remove me early so virtuals will work.
	if ( IsStat( STATF_Ridden ))
	{
		CItem * pItem = Horse_GetMountItem();
		if ( pItem )
		{
			pItem->m_itFigurine.m_UID.ClearUID();	// unlink it first.
			pItem->Delete();
		}
	}

	if ( IsClient())	// this should never happen.
	{
		DEBUG_CHECK( m_pClient );	// This is very bad.
		delete m_pClient;
	}

	DeleteAll();		// remove me early so virtuals will work.
	if ( m_pNPC != NULL )
	{
		delete m_pNPC;
		m_pNPC = NULL;
	}
	if ( m_pPlayer != NULL )
	{
		// unlink me from my account.
		m_pPlayer->GetAccount()->UnlinkChar( this );
		delete m_pPlayer;
		m_pPlayer = NULL;
	}

	g_Serv.StatDec( SERV_STAT_CHARS );
	if ( m_pDef != NULL )
	{
		m_pDef->DelInstance();
	}
}

void CChar::ClientDetach()
{
	// Client is detaching from this CChar.
	if ( ! IsClient()) 
		return;

	// If this char is on a ITEM_SHIP then we need to stop the ship !
	if ( m_pArea && m_pArea->IsFlag( REGION_FLAG_SHIP ))
	{
		CObjUID uid( m_pArea->GetMultiUID());
		CItemMulti * pShipItem = dynamic_cast <CItemMulti *>( uid.ItemFind());
		if ( pShipItem )
		{
			pShipItem->Ship_Stop();
		}
	}

	CSector * pSector = GetTopSector();
	pSector->ClientDetach( this );
	m_pClient = NULL;
}

void CChar::ClientAttach( CClient * pClient )
{
	if ( GetClient() == pClient ) 
		return;
	DEBUG_CHECK( ! IsClient());
	DEBUG_CHECK( pClient->GetAccount());
	if ( ! SetPlayerAccount( pClient->GetAccount()))	// i now own this char.
		return;
	m_pClient = pClient;
	GetTopSector()->ClientAttach( this );
}

void CChar::SetDisconnected()
{
	if ( IsClient()) 
	{
		delete GetClient();
		return;	// can't do this to active client !
	}
	if ( IsDisconnected()) 
		return;
	DEBUG_CHECK( GetParent());
	RemoveFromView();	// Remove from views.
	// DEBUG_MSG(( "Disconnect '%s'\n", GetName()));
	MoveToRegion(NULL,false);
	GetTopSector()->m_Chars_Disconnect.InsertAfter( this );
	DEBUG_CHECK( IsDisconnected());
}

int CChar::IsWeird() const
{
	// RETURN: invalid code.

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
bailout:
		return( iResultCode );
	}

	if ( IsDisconnected())
	{
		if ( ! GetTopSector()->IsCharDisconnectedIn( this ))
		{
			iResultCode = 0x1102;
			goto bailout;
		}
		if ( m_pNPC )
		{
			if ( IsStat( STATF_Ridden ))
			{
				if ( GetActiveSkill() != NPCACT_RIDDEN )
				{
					iResultCode = 0x1103;
					goto bailout;
				}

				// Make sure we are still linked back to the world.
				CItem * pItem = Horse_GetMountItem();
				if ( pItem == NULL )
				{
					iResultCode = 0x1104;
					goto bailout;
				}
				if ( pItem->m_itFigurine.m_UID != GetUID())
				{
					iResultCode = 0x1105;
					goto bailout;
				}
			}
			else
			{
				if ( ! IsStat( STATF_DEAD ))
				{
					iResultCode = 0x1106;
					goto bailout;
				}
			}
		}
	}

	if ( ! m_pPlayer && ! m_pNPC )
	{
		iResultCode = 0x1107;
		goto bailout;
	}

	if ( ! GetTopPoint().IsValid())
	{
		iResultCode = 0x1108;
		goto bailout;
	}

	return( 0 );
}

int CChar::FixWeirdness()
{
	// Clean up weird flags.
	// fix Weirdness.
	// NOTE: 
	//  Deleting a player char is VERY BAD ! Be careful !
	//
	// RETURN: false = i can't fix this.

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
bailout:
		// Not recoverable - must try to delete the object.
		return( iResultCode );
	}

	// Check range of stats.

	if ( Stat_Get(STAT_STR) <= 0 ) Stat_Set( STAT_STR, 1);
	if ( Stat_Get(STAT_INT) <= 0 ) Stat_Set( STAT_INT, 1);
	if ( Stat_Get(STAT_DEX) <= 0 ) Stat_Set( STAT_DEX, 1);

	// Make sure my flags are good.

	if ( IsStat( STATF_Insubstantial ))
	{
		if ( ! IsStat( STATF_DEAD ) && GetPrivLevel() <= PLEVEL_GM )
		{
			ClearStat( STATF_Insubstantial );
		}
	}
	if ( IsStat( STATF_HasShield ))
	{
		CItem * pShield = LayerFind( LAYER_HAND2 );
		if ( pShield == NULL )
		{
			ClearStat( STATF_HasShield );
		}
	}
	if ( IsStat( STATF_OnHorse ))
	{
		CItem * pHorse = LayerFind( LAYER_HORSE );
		if ( pHorse == NULL )
		{
			ClearStat( STATF_OnHorse );
		}
	}
	if ( IsStat( STATF_Pet ))
	{
		CItemMemory * pMemory = Memory_FindTypes( MEMORY_IPET );
		if ( pMemory == NULL )
		{
			ClearStat( STATF_Pet );
		}
	}
	if ( IsStat( STATF_Conjured ))
	{
		// Must have a conjuration timer !
		if ( ! LayerFind( LAYER_SPELL_Summon ))
		{
			if ( ! m_pPlayer )
			{
				iResultCode = 0x1202;
				goto bailout;
			}
			ClearStat( STATF_Conjured );
		}
	}
	if ( IsStat( STATF_Ridden ))
	{
		// Move the ridden creature to the same location as it's rider.
		if ( m_pPlayer || ! IsDisconnected())
		{
			ClearStat( STATF_Ridden );
		}
		else
		{
			if ( GetActiveSkill() != NPCACT_RIDDEN )
			{
				iResultCode = 0x1203;
				goto bailout;
			}
			CItem * pFigurine = Horse_GetMountItem();
			if ( pFigurine == NULL )
			{
				iResultCode = 0x1204;
				goto bailout;
			}
			CPointMap pt = pFigurine->GetTopLevelObj()->GetTopPoint();
			if ( pt != GetTopPoint())
			{
				MoveTo( pt );
				SetDisconnected();
			}
		}
	}
	if ( IsStat( STATF_Criminal ))
	{
		// make sure we have a criminal flag timer ?
	}

	if ( ! IsIndividualName() && m_pDef->GetTypeName()[0] == '#' )
	{
		SetName( m_pDef->GetTypeName());
	}
	if ( ! CCharBase::IsValidDispID( GetID()) && CCharBase::IsHuman( m_prev_id ))
	{
		// This is strange. (has human body)
		m_prev_id = GetID();
	}

	if ( m_pPlayer )	// Player char.
	{
		ClearStat( STATF_Pet | STATF_Ridden );

		if ( ! g_Serv.m_SkillClassDefs.IsValidIndex( m_pPlayer->m_SkillClass ))
		{
			m_pPlayer->m_SkillClass = 0;
		}

		// Make sure players don't get ridiculous stats.
		if ( GetPrivLevel() <= PLEVEL_Player )
		{
			for ( int i=0; i<SKILL_QTY; i++ )
			{
				int iSkillMax = g_Serv.m_SkillClassDefs[m_pPlayer->m_SkillClass]->m_SkillLevelMax[i];
				if ( m_Skill[i] < 0 ) Skill_SetBase( (SKILL_TYPE)i, 0 );
				if ( m_Skill[i] > iSkillMax * 2 ) Skill_SetBase( (SKILL_TYPE)i, iSkillMax );
			}

			// ??? What if magically enhanced !!!
			if ( IsHuman() &&
				GetPrivLevel() < PLEVEL_Counsel &&
				! IsStat( STATF_Polymorph ))
			{
				for ( int j=STAT_STR; j<STAT_BASE_QTY; j++ )
				{
					int iStatMax = g_Serv.m_SkillClassDefs[m_pPlayer->m_SkillClass]->m_StatMax[j];
					if ( Stat_Get((STAT_TYPE)j) > iStatMax*2 ) 
					{
						Stat_Set((STAT_TYPE)j, iStatMax );
					}
				}
			}
		}
	}
	else
	{
		if ( ! m_pNPC )
		{
			// Make it into an NPC ???
			iResultCode = 0x1205;
			goto bailout;
		}

		if ( ! strcmp( GetName(), "ship" ))
		{
			// version .37 cleanup code.
			iResultCode = 0x1206;
			goto bailout;
		}

		// An NPC. Don't keep track of unused skills.
		for ( int i=0; i<SKILL_QTY; i++ )
		{
			if ( m_Skill[i] && m_Skill[i] <= 10 )
				Skill_SetBase( (SKILL_TYPE)i, 0 );
		}
	}

	if ( g_World.m_iSaveVersion <= 35 )
	{
		if ( IsStat( STATF_Ridden ))
		{
			m_atRidden.m_FigurineUID.ClearUID();
			Skill_Start( NPCACT_RIDDEN );
		}
	}

	if ( GetTimerAdjusted() > 60*60 )
	{
		// unreasonably long for a char?
		SetTimeout(1);	
	}

	FixWeight();

	return( IsWeird());
}

void CChar::Stat_Set( STAT_TYPE i, short iVal )
{
	ASSERT(((WORD)i)<STAT_QTY );

	switch ( i )
	{
	case STAT_STR:
	case STAT_INT:
	case STAT_DEX:
		if ( iVal <= 0 )
		{
			iVal = 1;
		}
		break;
	case STAT_Karma:		// -10000 to 10000
		break;
	case STAT_Fame:
		if ( iVal < 0 )
		{
			DEBUG_ERR(( "ID=0%x,UID=0%x Fame set out of range %d\n", GetBaseID(), GetUID(), iVal ));
			iVal = 0;
		}
		break;
	}
	m_Stat[i] = iVal;
	UpdateStatsFlag();
}

bool CChar::ReadScript( CScript &s, bool fRestock, bool fNewbie )
{
	// If this is a regen they will have a pack already.

	bool fIgnoreAttributes = false;

	CItem * pItem = NULL;
	while ( s.ReadKeyParse())
	{
		bool fNewbieItem = s.IsKey("ITEMNEWBIE");
		if ( s.IsKey("ITEM") || fNewbieItem )
		{
			// Possible loot/equipped item.
			fIgnoreAttributes = true;

			if ( IsStat( STATF_Conjured ) && ! fNewbieItem )
				break; // conjured creates have no loot.

			ITEMID_TYPE iEquipID = (ITEMID_TYPE) s.GetArgVal();

			// Check if they already have the item ? In the case of a regen.
			// This is just to keep newbie items from being duped.
			if ( iEquipID < ITEMID_TEMPLATE && fRestock )
			{
				if ( ContentFind( iEquipID ))
				{
					// We already have this.
					continue;
				}
			}

			pItem = CItem::CreateHeader( iEquipID, s.GetArgStr(), this );
			if ( pItem == NULL )
				continue;

			if ( fNewbie || fNewbieItem )
			{
				pItem->m_Attr |= ATTR_NEWBIE;
			}

			fIgnoreAttributes = false;
			LayerAdd( pItem );
			continue;
		}

		if ( s.IsKey("BUY") || s.IsKey("SELL"))
		{
			fIgnoreAttributes = true;
			if ( fNewbie || fRestock ) 
				continue;

			CItemContainer * pCont = GetBank( s.IsKey("SELL") ? LAYER_VENDOR_STOCK : LAYER_VENDOR_BUYS );
			if ( pCont == NULL ) 
			{
				DEBUG_ERR(( "NPC id=0%x, is not a vendor!\n", GetID()));
				continue;
			}
			pItem = CItem::CreateHeader( ITEMID_QTY, s.GetArgStr(), pCont );
			if ( pItem == NULL ) 
				continue;
			if ( pItem == pCont ) 
				continue;

			pCont->ContentAdd( pItem );
			fIgnoreAttributes = false;
			continue;
		}

		if ( fIgnoreAttributes )	// some item creation failure.
			continue;

		if ( pItem != NULL )
		{
			pItem->r_LoadVal( s );
		}
		else
		{
			if ( fRestock )	// restock does not set the char stuff
				continue;
			if ( fNewbie )	// can't set a clients name like this.
			{
				if ( s.IsKey("NAME"))
					continue;
			}

			TRIGRET_TYPE tret = OnTriggerRun( s, TRIGRUN_SECTION_SINGLE, &g_Serv );
			if ( tret != TRIGRET_RET_DEFAULT )
			{
				return( ! tret );
			}
		}
	}

	return( true );
}

void CChar::CreateNewCharCheck()
{
	// Creating a new char. (Not loading from save file)
	m_prev_id = GetID();
	m_prev_color = GetColor();
	m_StatHealth = Stat_Get(STAT_STR);
	m_StatStam = Stat_Get(STAT_DEX);
	m_StatMana = Stat_Get(STAT_INT);
}

void CChar::NPC_LoadScript( bool fRestock )
{
	// Create an NPC from script.
	if ( ! fRestock )
	{
		SetNPCBrain( GetCreatureType());	// Must have a default brain.
	}

	CScriptLock s;
	if ( m_pDef->ScriptLockBase(s))
	{
		ReadScript( s, fRestock );
	}

	if ( m_pNPC && m_pNPC->IsVendor())
	{
		// Restock it now so it can buy stuff immediately
		NPC_Vendor_Restock( 15*60*TICK_PER_SEC );
	}

	CreateNewCharCheck();
}

void CChar::OnWeightChange( int iChange )
{
	CContainer::OnWeightChange( iChange );
	if ( IsClient())
	{
		UpdateStatsFlag();
	}
}

bool CChar::SetName( const TCHAR * pszName )
{
	return SetNamePool( pszName );
}

void CChar::SetID( CREID_TYPE id )
{
	CCharBase * pDef = CCharBase::FindCharBase( id );
	if ( pDef == NULL )
	{
		DEBUG_ERR(( "Create Invalid Char 0%x\n", id ));
		if ( m_pDef != NULL )
			return;
		pDef = CCharBase::FindCharBase( CREID_MAN );
	}

	if ( pDef == m_pDef )
		return;

	if ( m_pDef != NULL )
	{
		m_pDef->DelInstance();
	}
	m_pDef = pDef;
	m_pDef->AddInstance();

	if ( GetCreatureType() != NPCBRAIN_HUMAN )
	{
		// Transfom to non-human (if they ever where human)
		// can't ride a horse in this form.
		Horse_UnMount();
		UnEquipAllItems(); 		// unequip all items.
	}
}

void CChar::AddGoldToPack( int iAmount, CItemContainer * pPack )
{
	// A vendor is giving me gold. put it in my pack or other place.

	if ( pPack == NULL ) pPack = GetPackSafe();
	while ( iAmount > 0 )
	{
		CItem * pGold = CItem::CreateScript( ITEMID_GOLD_C1 );

		int iGoldStack = min( iAmount, 0xFFFF );
		pGold->SetAmount( iGoldStack );

		Sound( pGold->GetDropSound());
		pPack->ContentAdd( pGold );
		iAmount -= iGoldStack;
	}
}

CItemContainer * CChar::GetBank( LAYER_TYPE layer )
{
	// Get our bank box or vendor box.
	// If we dont have 1 then create it.

	ITEMID_TYPE id;
	switch ( layer )
	{
	case LAYER_PACK:
		id = ITEMID_BACKPACK;
		break;

	case LAYER_VENDOR_STOCK:
	case LAYER_VENDOR_EXTRA:
	case LAYER_VENDOR_BUYS:
		if ( m_pNPC == NULL )
			return( NULL );
		if ( ! m_pNPC->IsVendor())
			return( NULL );
 		id = ITEMID_VENDOR_BOX;
		break;

	case LAYER_BANKBOX:
	default:
		id = ITEMID_BANK_BOX;
		layer = LAYER_BANKBOX;
		break;
	}

	CItemContainer * pBankBox = dynamic_cast <CItemContainer *>( LayerFind( layer ));
	if ( pBankBox == NULL )
	{
		// Give them a bank box if not already have one.
		if ( g_Serv.IsLoading())
			return( NULL );
		pBankBox = dynamic_cast <CItemContainer *>( CItem::CreateScript( id ));
		pBankBox->m_Attr |= ATTR_NEWBIE | ATTR_MOVE_NEVER;
		LayerAdd( pBankBox, layer );
		if ( layer != LAYER_PACK )
		{
			pBankBox->SetRestockTimeSeconds( 15*60 );
			pBankBox->SetTimeout( pBankBox->GetRestockTimeSeconds() * TICK_PER_SEC );	// restock count
		}
	}
	return( pBankBox );
}

CItem * CChar::LayerFind( LAYER_TYPE layer ) const
{
	// Find an item i have equipped.
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->GetEquipLayer() == layer )
			break;
	}
	return( pItem );
}

LAYER_TYPE CChar::CanEquipLayer( CItem * pItem, LAYER_TYPE layer )
{
	// LAYER_NONE = can't equip this .
	ASSERT( m_pDef );
	if ( layer >= LAYER_QTY )
	{
		layer = pItem->m_pDef->m_layer;
	}

	switch ( layer )
	{
	case LAYER_NONE:
	case LAYER_SPECIAL:
		switch ( pItem->m_type )
		{
		case ITEM_EQ_TRADE_WINDOW:
		case ITEM_EQ_MEMORY_OBJ:
		case ITEM_EQ_NPC_SCRIPT:
			// We can have multiple items of these.
			return LAYER_SPECIAL;
		}
		return( LAYER_NONE );	// not legal !

	case LAYER_UNUSED9:
	case LAYER_PACK2:
		DEBUG_ERR(( "ALERT: Weird layer 9 used for 0%x check this\n", pItem->GetID()));
		return( LAYER_NONE );	// not legal !

	case LAYER_RING:
	case LAYER_PACK:
	case LAYER_EARS:	// anyone can use these !
		break;

	case LAYER_HAND1:
	case LAYER_HAND2:
		if ( ! m_pDef->Can( CAN_C_USEHANDS ))
		{
			goto cantequipthis;
		}
		break;

	default:
		// Can i equip this with my body type ?
		if ( ! m_pDef->Can( CAN_C_EQUIP ) &&
			CItemBase::IsVisibleLayer( layer ))
		{
			// some creatures can equip certain special items ??? (orc lord?)
cantequipthis:
			if ( g_Log.IsLogged( LOGL_TRACE ))
			{
				DEBUG_ERR(( "ContentAdd Creature 0%x can't equip %d, id=0%x '%s'\n", GetID(), layer,  pItem->GetID(), pItem->GetName()));
			}
			return LAYER_NONE;	// can't equip stuff.
		}
		break;
	}

	// Check for objects already in this slot.
	CItem * pItemPrev = LayerFind( layer );
	if ( pItemPrev != NULL )
	{
		if ( layer == LAYER_PACK )
		{
			// this should not happen.
			// Put it in my main pack.
			// DEBUG_CHECK( layer != LAYER_PACK );
			return LAYER_NONE;
		}
		if ( layer == LAYER_HORSE )
		{
			// this should not happen.
			DEBUG_CHECK( layer != LAYER_HORSE );
			ItemBounce( pItemPrev );
		}
		else if ( layer == LAYER_DRAGGING )	// drop it.
		{
			ItemBounce( pItemPrev );
		}
		else if ( layer == LAYER_BEARD ||
			layer == LAYER_HAIR ||
			layer >= LAYER_SPELL_STATS )
		{
			// Magic spell slots just get bumped.
			pItemPrev->Delete();
		}
		else
		{
			// DEBUG_ERR(( "LayerAdd Layer %d already used\n", layer ));
			// Force the current layer item into the pack ?
			if ( ! CanMove( pItemPrev, true ))
			{
				return LAYER_NONE;
			}
			ItemBounce( pItemPrev );
		}
	}

	return( layer );
}

void CChar::LayerAdd( CItem * pItem, LAYER_TYPE layer )
{
	// add equipped items.
	// check for item already in that layer ?
	// NOTE: This could be part of the Load as well so it may not truly be being "equipped" at this time.
	// OnTrigger for equip is done by ItemEquip()

	if ( pItem == NULL ) 
		return;
	pItem->RemoveSelf();	// unlink myself from the world so i dont get messed with for now.

	if ( m_pNPC )
	{
		pItem->m_Attr &= ~ATTR_NEWBIE;	// Lose nebie status.
	}

	layer = CanEquipLayer( pItem, layer );
	if ( layer == LAYER_NONE )
	{
		// we should not allow non-layered stuff to be put here ?
		// Put in pack instead ?
		//if ( g_Log.IsLogged( LOGL_TRACE ))
		//{
			// DEBUG_MSG(( "ContentAdd id=0%x '%s', LAYER_NONE is strange\n", pItem->GetID(), pItem->GetName()));
		//}
		ItemBounce( pItem );
		return;
	}

	if ( layer == LAYER_SPECIAL )
	{
		if ( pItem->m_type == ITEM_EQ_TRADE_WINDOW )
			layer = LAYER_NONE;
	}

	CContainer::ContentAddPrivate( pItem );
	pItem->SetEquipLayer( layer );

	switch ( layer )
	{
	case LAYER_HAIR:
	case LAYER_BEARD:
		pItem->m_Attr |= ATTR_NEWBIE|ATTR_MOVE_NEVER;	// should not come off when corpse is cut up.
		break;
	case LAYER_HAND1:
	case LAYER_HAND2:
		// If weapon
		if ( pItem->IsArmorWeapon())
		{
			// Make sure the other hand is not full.
			if ( pItem->IsWeapon())
			{
				if ( layer == LAYER_HAND2 )	// this is a 2 handed weapon.
				{
					CItem * pItemPrev = LayerFind( LAYER_HAND1 );
					if ( pItemPrev != NULL )
					{
						ItemBounce( pItemPrev );
					}
				}
				else
				{
					CItem * pItemPrev = LayerFind( LAYER_HAND2 );
					if ( pItemPrev != NULL && pItemPrev->IsWeapon())
					{	// already have a 2 handed weapon ?
						ItemBounce( pItemPrev );
					}
				}
				m_weapon = pItem->GetUID();
				SetWeaponSwingTimer();
			}
			else
			{
				// Shield of some sort.
				m_defense = CalcArmorDefense();
				SetStat( STATF_HasShield );
			}
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
	case LAYER_HALF_APRON:
	case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
	case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
	case LAYER_SKIRT:
	case LAYER_LEGS:
		// If armor or clothing = change in defense rating.
		m_defense = CalcArmorDefense();
		break;
	case LAYER_PACK:
		DEBUG_CHECK( pItem->GetID() == ITEMID_BACKPACK );
		break;
	case LAYER_HORSE:
		SetStat( STATF_OnHorse );
		break;

	case LAYER_SPECIAL:
		// Can be multiple memories.
		if ( pItem->m_type == ITEM_EQ_MEMORY_OBJ )
		{
			Memory_UpdateFlags( dynamic_cast <CItemMemory *>(pItem) );
		}
		else if ( pItem->m_type == ITEM_EQ_NPC_SCRIPT )
		{
			NPC_Script_OnTick( dynamic_cast <CItemMessage *>(pItem));
		}
		else
		{
			DEBUG_CHECK(0);
		}
		return;

		// These effects are not saved in save file.

	case LAYER_FLAG_Poison:
		SetStat( STATF_Poisoned );
		break;
	case LAYER_FLAG_Criminal:
		SetStat( STATF_Criminal );
		return;
	case LAYER_FLAG_SpiritSpeak:
		SetStat( STATF_SpiritSpeak );
		return;
	case LAYER_FLAG_Drunk:
		SetStat( STATF_Drunk );
		return;
	case LAYER_FLAG_Stuck:
		SetStat( STATF_Freeze );
		break;
	}

	pItem->Update();
}

void CChar::OnRemoveOb( CGObListRec* pObRec )	// Override this = called when removed from list.
{
	// Unequip the item.
	// This may be a delete etc. It can not FAIL !
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	ASSERT(pItem);
	DEBUG_CHECK( pItem->IsEquipped());

	LAYER_TYPE layer = pItem->GetEquipLayer();
	if ( layer != LAYER_DRAGGING && ! g_Serv.IsLoading())
	{
		pItem->OnTrigger( ITRIG_UNEQUIP, this, pItem->GetAmount());
	}

	CContainer::OnRemoveOb( pObRec );

	// remove equipped items effects
	switch ( layer )
	{
	case LAYER_HAND1:
	case LAYER_HAND2:	// other hand = shield

		if ( pItem->IsArmorWeapon())
		{
			if ( pItem->IsWeapon())
			{
				m_weapon.ClearUID();
				SetWeaponSwingTimer();
			}
			else
			{
				// Shield
				m_defense = CalcArmorDefense();
				ClearStat( STATF_HasShield );
			}
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
	case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
	case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
	case LAYER_SKIRT:
	case LAYER_LEGS:
		m_defense = CalcArmorDefense();
		break;
	case LAYER_PACK:
		DEBUG_CHECK( pItem->GetID() == ITEMID_BACKPACK );
		break;
	case LAYER_HORSE:
		ClearStat( STATF_OnHorse );
		break;
	case LAYER_FLAG_Criminal:
		ClearStat( STATF_Criminal );
		break;
	case LAYER_FLAG_SpiritSpeak:
		ClearStat( STATF_SpiritSpeak );
		break;
	case LAYER_FLAG_Drunk:
		ClearStat( STATF_Drunk );
		break;
	case LAYER_FLAG_Stuck:
		ClearStat( STATF_Freeze );
		break;
	case LAYER_SPECIAL:
		if ( pItem->m_type == ITEM_EQ_MEMORY_OBJ )
		{
			// Clear the associated flags.
			Memory_UpdateClearTypes( dynamic_cast<CItemMemory*>(pItem), 0xFFFF );
			return;
		}
		break;
	}

	// Items with magic effects.
	if ( layer != LAYER_DRAGGING )
	{
		// If items are magical then remove effect here.
		Spell_Effect_Remove( pItem );
	}
}

void CChar::DropAll( CItemContainer * pCorpse )
{
	// shrunk or died. (or sleeping)
	if ( IsStat( STATF_Conjured )) 
		return;	// drop nothing.

	CItemContainer * pPack = GetPack();
	if ( pPack != NULL )
	{
		if ( pCorpse == NULL )
		{
			pPack->ContentsDump( GetTopPoint());
		}
		else
		{
			pPack->ContentsTransfer( pCorpse, true );
		}
	}

	// transfer equipped items to corpse or your pack (if newbie).
	UnEquipAllItems( pCorpse );
}

void CChar::UnEquipAllItems( CItemContainer * pDest )
{
	// We morphed, sleeping, died or became a GM.
	// Pets can be told to "Drop All"
	// drop item that is up in the air as well.

	if ( ! GetCount()) 
		return;
	CItemContainer * pPack = NULL;

	CItem* pItemNext;
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		LAYER_TYPE layer = pItem->GetEquipLayer();
		switch ( layer )
		{
		case LAYER_NONE:
			DEBUG_CHECK( pItem->m_type == ITEM_EQ_TRADE_WINDOW );
			pItem->Delete();	// Get rid of any trades.
			continue;
		case LAYER_FLAG_Poison:
		case LAYER_FLAG_Criminal:
		case LAYER_FLAG_Hallucination:
		case LAYER_FLAG_Potion:
		case LAYER_FLAG_Drunk:
		case LAYER_FLAG_Stuck:
		case LAYER_FLAG_PotionUsed:
			if ( IsStat( STATF_DEAD ))
				pItem->Delete();
			continue;
		case LAYER_PACK:
		case LAYER_HORSE:
			continue;
		case LAYER_HAIR:	// leave this.
		case LAYER_BEARD:
			// Copy hair and beard to corpse.
			if ( pDest == NULL )
				continue;
			if ( pDest->GetID() == ITEMID_CORPSE )
			{
				CItem * pDupe = CItem::CreateDupeItem( pItem );
				pDest->ContentAdd( pDupe );	// add content
				// Equip layer only matters on a corpse.
				pDupe->SetContainedLayer( layer );
			}
			continue;
		case LAYER_DRAGGING:
			layer = LAYER_NONE;
			break;
		default:
			// can't transfer this to corpse.
			if ( ! CItemBase::IsVisibleLayer( layer ))
				continue;
			break;
		}
		if ( pDest != NULL &&
			! pItem->IsAttr( ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2 ))
		{	// Move item to dest. (corpse ussually)
			pDest->ContentAdd( pItem );
			if ( pDest->GetID() == ITEMID_CORPSE )
			{
				// Equip layer only matters on a corpse.
				pItem->SetContainedLayer( layer );
			}
		}
		else
		{	// Move item to chars' pack.
			if ( pPack == NULL )
				pPack = GetPackSafe();
			pPack->ContentAdd( pItem );
		}
	}
}

void CChar::CancelAllTrades()
{
	// remove all trade windows. client logged out.
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; )
	{
		CItem* pItemNext = pItem->GetNext();
		if ( pItem->m_type == ITEM_EQ_TRADE_WINDOW )
		{
			pItem->Delete();
		}
		pItem=pItemNext;
	}
}

bool CChar::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	if ( ! strnicmp( pszKey, "ACCOUNT.", 8 ))
	{
		pszKey += 8;
		if ( m_pPlayer == NULL ) 
			return( false );
		pRef = m_pPlayer->GetAccount();
		return( true );
	}
	if ( ! strnicmp( pszKey, "ACT.", 4 ))
	{
		pszKey += 4;
		pRef = m_Act_Targ.ObjFind();
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDEQUIP", 9 ) ||
		 ! strnicmp( pszKey, "FINDLAYER", 9 ))	// Find equipped layers.
	{
		pszKey += 9;
		SKIP_SEPERATORS(pszKey);
		pRef = LayerFind( (LAYER_TYPE) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDTYPE", 8 ))
	{
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFindType( (ITEM_TYPE) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDID", 6 ))
	{
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFind( (ITEMID_TYPE) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "MEMORYFINDTYPE", 14 ))	// FInd a type of memory.
	{
		pszKey += 14;		
		SKIP_SEPERATORS(pszKey);
		pRef = Memory_FindTypes( Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "MEMORYFIND", 10 ))	// Find a memory of a UID
	{
		pszKey += 10;
		SKIP_SEPERATORS(pszKey);
		pRef = Memory_FindObj( (CObjUID) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "REGION.", 7 ))
	{
		pszKey += 7;
		pRef = m_pArea;
		return( true );
	}

	return( CObjBase::r_GetRef( pszKey, pRef, pSrc ));
}

enum
{
	CC_ACCOUNT=0,
	CC_ACT,
	CC_ACTARG1,
	CC_ACTARG2,
	CC_ACTION,
	CC_BODY,
	CC_CREATE,
	CC_DIR,
	CC_EMOTEACT,
	CC_EVENTS,
	CC_FLAGS,
	CC_FONT,
	CC_FOOD,
	CC_HITPOINTS,
	CC_HITS,
	CC_HOME,
	CC_MANA,
	CC_NPC,
	CC_OBODY,
	CC_OSKIN,
	CC_P,
	CC_STAM,
	CC_STAMINA,
	CC_STONE,
	CC_TITLE,
	CC_XBODY,
	CC_XSKIN,
	CC_QTY,
};

const TCHAR * CChar::sm_KeyTable[CC_QTY] =
{
	"ACCOUNT",
	"ACT",
	"ACTARG1",
	"ACTARG2",
	"ACTION",
	"BODY",
	"CREATE",
	"DIR",
	"EMOTEACT",
	"EVENTS",
	"FLAGS",
	"FONT",
	"FOOD",
	"HITPOINTS",
	"HITS",
	"HOME",
	"MANA",
	"NPC",
	"OBODY",
	"OSKIN",
	"P",
	"STAM",
	"STAMINA",
	"STONE",
	"TITLE",
	"XBODY",
	"XSKIN",
};

bool CChar::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	if ( IsClient())
	{
		if ( GetClient()->r_WriteVal( pKey, sVal, pSrc ))
			return( true );
	}

	if ( ! strnicmp( pKey, "KARMA.", 6 ))
	{
		// What do i think of this person.
		pKey += 6;
		SKIP_SEPERATORS(pKey);
		static const CValStr titles[] =
		{
			"WICKED", INT_MIN,				// -10000 to -6001
			"BELLIGERENT", -6000,			// -6000 to -2001
			"NEUTRAL", -2000,				// -2000 to  2000
			"KINDLY", 2001,				// 2001 to  6000
			"GOODHEARTED", 6001,		// 6001 to 10000
			NULL, INT_MAX,
		};

		int iKarma = Stat_Get(STAT_Karma);
		sVal = ( ! strcmpi( pKey, titles->FindName( iKarma ))) ? "1" : "0";
		return( true );
	}
	if ( ! strnicmp( pKey, "FAME.", 5 ))
	{
		// How much respect do i give this person ?
		// Fame is never negative !
		pKey += 5;
		SKIP_SEPERATORS(pKey);

		static const TCHAR * tableFame[] =	// display only type stuff.
		{
			"ANONYMOUS",
			"FAMOUS", 
			"INFAMOUS",	// get rid of this in the future.
			"KNOWN",
			"OUTLAW",
		};

		int iFame = Stat_Get(STAT_Fame);
		int iKarma = Stat_Get(STAT_Karma);

		switch ( FindTableSorted( pKey, tableFame, COUNTOF( tableFame )))
		{
		case 0: // "ANONYMOUS"
			iFame = ( iFame < 2000 );
			break;
		case 1: // "FAMOUS"
			iFame = ( iFame > 6000 ); 
			break;
		case 2: // "INFAMOUS"
			iFame = ( iFame > 6000 && iKarma <= -6000 ); 
			break;
		case 3: // "KNOWN"
			iFame = ( iFame > 2000 ); 
			break;
		case 4: // "OUTLAW"
			iFame = ( iFame > 2000 && iKarma <= -2000 ); 
			break;
		}
		sVal = iFame ? "1" : "0";
		return( true );
	}
	if ( ! strnicmp( pKey, "SKILLCHECK", 10 ))
	{
		// odd way to get skills checking into the triggers.
		pKey += 10;
		SKIP_SEPERATORS(pKey);
		TCHAR * ppArgs[2];
		ParseCmds( (TCHAR*) pKey, ppArgs, COUNTOF( ppArgs ));
		SKILL_TYPE iSkill = g_Serv.FindSkillKey( ppArgs[0] );
		if ( iSkill == SKILL_NONE ) 
			return( false );
		sVal = Skill_CheckSuccess( iSkill, Exp_GetVal( ppArgs[1] )) ?
			"1" : "0";
		return( true );
	}
	if ( ! strnicmp( pKey, "RESTEST", 7 ))
	{
		pKey += 7;
		SKIP_SEPERATORS(pKey);
		sVal.FormatVal( ResourceConsume( (TCHAR*) pKey, 1, true ));
		return( true );
	}
	if ( ! strnicmp( pKey, "SEX", 3 ))
	{
		// <SEX milord/milady>	sep chars are :,/
		pKey += 3;
		SKIP_SEPERATORS(pKey);
		TCHAR * ppArgs[2];
		ParseCmds( (TCHAR*) pKey, ppArgs, COUNTOF(ppArgs), ":,/" );
		sVal = ( CCharBase::IsFemaleID( GetID())) ? ppArgs[1] : ppArgs[0];
		return( true );
	}
	if ( ! strnicmp( pKey, "ISEVENT", 7 ))
	{
		// "ISEVENT"
		pKey += 7;
		SKIP_SEPERATORS(pKey);
		sVal = m_Events.IsFragIn(pKey) ? "1" : "0";
		return true;
	}

	static const TCHAR * table[] =	// display only type stuff.
	{
		"AC",
		"AGE",
		"BANKBALANCE",
		"COUNT",
		"DISPIDDEC",	// for properties dialog.
		"GUILDABBREV",
		"ID",
		"ISMYPET",
		"MEMORY",
		"SKILLTOTAL",
		"TOWNABBREV",
		"WEIGHT",
	};

	CChar * pCharSrc = pSrc->GetChar();
	switch ( FindTableSorted( pKey, table, COUNTOF( table )))
	{
	case 0:	// "AC"
		sVal.FormatVal( m_defense + m_pDef->m_defense );
		return( true );
	case 1:	// "AGE"
		sVal.FormatVal(( g_World.GetTime() - m_time_create ) / TICK_PER_SEC );
		return( true );
	case 2: // "BANKBALANCE"
		sVal.FormatVal( GetBank()->ContentCount(ITEMID_GOLD_C1));
		return true;
	case 3: // "COUNT"	count these sort of items. (Count reagents)
		if ( ! pKey[5] )
		{
			sVal.FormatVal( GetCount());	// count all.
		}
		else
		{
			pKey += 6;
			sVal.FormatVal( ContentCount( (ITEMID_TYPE) Exp_GetVal( pKey )));
		}
		return( true );
	case 4: // "DISPIDDEC"	// for properties dialog.
		sVal.FormatVal( m_pDef->m_trackID );
		return true;
	case 5:	// "GUILDABBREV",
		{
			const TCHAR * pszAbbrev = Guild_Abbrev(MEMORY_GUILD);
			sVal = ( pszAbbrev ) ? pszAbbrev : "";
		}
		return true;
	case 6: // "ID"
		sVal.FormatHex( GetID());
		return true;
	case 7: // "ISMYPET"
		sVal = NPC_IsOwnedBy( pCharSrc, true ) ? "1" : "0";
		return( true );
	case 8: // "MEMORY"
		// What is our memory flags about this pSrc person.
		{
			DWORD dwFlags = 0;
			CItemMemory * pMemory = Memory_FindObj( pCharSrc );
			if ( pMemory != NULL )
			{
				dwFlags = pMemory->GetMemoryTypes();
			}
			sVal.FormatHex( dwFlags );
		}
		return( true );
	case 9:	// "SKILLTOTAL"
		{
			int iTotal = 0;
			for ( int i=0; i<SKILL_QTY; i++ )
			{
				iTotal += Skill_GetBase((SKILL_TYPE)i);
			}
			sVal.FormatVal( iTotal );
		}
		return( true );
	case 10:	// "TOWNABBREV",
		{
			const TCHAR * pszAbbrev = Guild_Abbrev(MEMORY_TOWN);
			sVal = ( pszAbbrev ) ? pszAbbrev : "";
		}
		return true;
	case 11:	// "WEIGHT" - use WEIGHT_UNITS ?
		sVal.FormatVal( GetTotalWeight());
		return( true );
	}

	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case CC_ACCOUNT:
		if ( m_pPlayer == NULL )
			return( false );
		sVal = m_pPlayer->GetAccount()->GetName();
		break;
	case CC_ACT:
		sVal.FormatHex( m_Act_Targ.GetIndex());
		break;
	case CC_ACTARG1:
		sVal.FormatHex( m_atUnk.m_Arg1);
		break;
	case CC_ACTARG2:
		sVal.FormatHex( m_atUnk.m_Arg2 );
		break;
	case CC_ACTION:
		sVal.FormatVal( GetActiveSkill());
		break;
	case CC_BODY:
		sVal.FormatHex( GetDispID());
		break;
	case CC_CREATE:
		sVal.FormatHex( m_time_create );
		break;
	case CC_DIR:
		sVal.FormatVal( m_face_dir );
		break;
	case CC_EMOTEACT:
		sVal.FormatVal( IsStat( STATF_EmoteAction ));
		break;
	case CC_EVENTS:
		m_Events.WriteFragList( sVal );
		break;
	case CC_FLAGS:
		sVal.FormatHex( m_StatFlag );
		break;
	case CC_FONT:
		sVal.FormatVal( m_fonttype );
		break;
	case CC_FOOD:
		sVal.FormatVal( m_food);
		break;
	case CC_HITPOINTS:
		goto hitpoints;
	case CC_HITS:
hitpoints:
		sVal.FormatVal( m_StatHealth );
		break;
	case CC_HOME:
		sVal = m_Home.Write();
		break;
	case CC_MANA:
		sVal.FormatVal( m_StatMana );
		break;
	// case CC_NPC:
	case CC_OBODY:
scp_obody:
		sVal.FormatHex( m_prev_id);
		break;
	case CC_OSKIN:
scp_oskin:
		sVal.FormatHex( m_prev_color);
		break;
	//case CC_P
	case CC_STAM:
stamina:
		sVal.FormatVal( m_StatStam );
		break;
	case CC_STAMINA:
		goto stamina;
	case CC_STONE:
		sVal.FormatVal( IsStat( STATF_Stone ));
		break;
	case CC_TITLE:
		sVal = m_sTitle;
		break;
	case CC_XBODY: // not used anymore.
		goto scp_obody;
	case CC_XSKIN:	// not used anymore.
		goto scp_oskin;

	default:
		if ( m_pPlayer )
		{
			//if ( m_pPlayer->GetAccount()->r_WriteVal( pKey, sVal, pSrc ))
			//	break;
			if ( m_pPlayer->r_WriteVal( pKey, sVal ))
				break;
		}
		else if ( m_pNPC )
		{
			if ( m_pNPC->r_WriteVal( pKey, sVal ))
				break;
		}

		// Special write values.
		int i = g_Serv.FindStatKey( pKey );
		if ( i >= 0 )
		{
			sVal.FormatVal( Stat_Get( (STAT_TYPE) i));
			break;
		}
		i = g_Serv.FindSkillKey( pKey );
		if ( i != SKILL_NONE )
		{
			// Check some skill name.
			short iVal = Skill_GetBase( (SKILL_TYPE) i );
			sVal.Format( "%i.%i", iVal/10, iVal%10 );
			break;
		}
		return( CObjBase::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CChar::r_LoadVal( CScript & s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case CC_ACCOUNT:
		return SetPlayerAccount( s.GetArgStr());
	case CC_ACT:
		m_Act_Targ = s.GetArgVal();
		return true;
	case CC_ACTARG1:
		m_atUnk.m_Arg1 = s.GetArgVal();
		return true;
	case CC_ACTARG2:
		m_atUnk.m_Arg2 = s.GetArgVal();
		return true;
	case CC_ACTION:
		return Skill_Start( (SKILL_TYPE) s.GetArgVal());
	case CC_BODY:
		SetID( (CREID_TYPE) s.GetArgHex());
		return true;
	case CC_CREATE:
		m_time_create = s.GetArgHex();
		return true;
	case CC_DIR:
		m_face_dir = (DIR_TYPE) s.GetArgVal();
		if ( m_face_dir < 0 || m_face_dir >= DIR_QTY ) m_face_dir = DIR_SE;
		return true;
	case CC_EMOTEACT:
		{
			bool fSet = IsStat(STATF_EmoteAction);
			if ( s.HasArgs())
			{
				fSet = s.GetArgVal();
			}
			else
			{
				fSet = ! fSet;
			}
			ModStat(STATF_EmoteAction,fSet);
		}
		return true;
	case CC_EVENTS:
		// ??? Check that it is not already part of m_pDef->m_Events
		return( m_Events.r_LoadVal( s ));
	case CC_FLAGS:
		m_StatFlag = s.GetArgHex();
		ModStat( STATF_SaveParity,  g_World.m_fSaveParity ); // It will get saved next time.
		return true;
	case CC_FONT:
		m_fonttype = (FONT_TYPE)s.GetArgVal();
		if ( m_fonttype < 0 || m_fonttype >= FONT_QTY )
			m_fonttype = FONT_NORMAL;
		return true;
	case CC_FOOD:
		m_food = s.GetArgVal();
		return true;
	case CC_HITPOINTS:
		goto scp_hitpoints;
	case CC_HITS:
scp_hitpoints:
		m_StatHealth = s.GetArgRange();
		UpdateStatsFlag();
		return true;
	case CC_HOME:
		if ( ! s.HasArgs()) 
			m_Home = GetTopPoint();
		else 
			m_Home.Read( s.GetArgStr());
		return true;
	case CC_MANA:
		m_StatMana = s.GetArgRange();
		UpdateStatsFlag();
		return true;
	case CC_NPC:
		SetNPCBrain( (NPCBRAIN_TYPE) s.GetArgVal());
		return true;
	case CC_OBODY:
scp_obody:
		m_prev_id = (CREID_TYPE) s.GetArgHex();
		if ( ! CCharBase::FindCharBase( m_prev_id ))
		{
			DEBUG_ERR(( "OBODY Invalid Char 0%x\n", m_prev_id ));
			m_prev_id = CREID_MAN;
		}
		return true;
	case CC_OSKIN:
scp_oskin:
		m_prev_color = s.GetArgHex();
		return true;
	case CC_P:
		{
			CPointMap pt;
			pt.Read(s.GetArgStr());
			MoveTo(pt);
		}
		return true;
	case CC_STAM:
	case CC_STAMINA:
		m_StatStam = s.GetArgRange();
		UpdateStatsFlag();
		return true;
	case CC_STONE:
		{
			bool fSet;
			bool fChange = IsStat(STATF_Stone);
			if ( s.HasArgs())
			{
				fSet = s.GetArgVal();
				fChange = ( fSet != fChange );
			}
			else
			{
				fSet = ! fChange;
				fChange = true;
			}
			ModStat(STATF_Stone,fSet);
			if ( fChange )
			{
				RemoveFromView();
				Update();
			}
		}
		return true;
	case CC_TITLE:
		m_sTitle = s.GetArgStr();
		return true;
	case CC_XBODY:
		goto scp_obody;
	case CC_XSKIN:
		goto scp_oskin;
	}

	if ( m_pPlayer )
	{
		if ( m_pPlayer->r_LoadVal( s ))
			return( true );
	}
	else if ( m_pNPC )
	{
		if ( s.IsKey("HOMEDIST" ))
		{
			if ( ! m_Home.IsValid()) 
				m_Home = GetTopPoint();
		}
		if ( m_pNPC->r_LoadVal( s ))
			return( true );
	}

	int i = g_Serv.FindSkillKey( s.GetKey());
	if ( i != SKILL_NONE )
	{
		// Check some skill name.
		Skill_SetBase( (SKILL_TYPE) i, s.GetArgRange());
		return true;
	}

	i = g_Serv.FindStatKey( s.GetKey());
	if ( i >= 0 )
	{
		Stat_Set( (STAT_TYPE) i, s.GetArgRange());
		return true;
	}

	return( CObjBase::r_LoadVal( s ));
}

void CChar::r_Write( CScript & s )
{
	// if ( GetPrivLevel() <= PLEVEL_Guest ) return;

	if ( g_World.m_fSaveParity == IsStat(STATF_SaveParity))
	{
		return; // already saved.
	}

	ModStat( STATF_SaveParity, g_World.m_fSaveParity );

	if ( IsWeird()) 
		return;

	s.WriteSection( "WORLDCHAR 0%x", GetID());
	s.WriteKeyHex( "CREATE", m_time_create );

	CObjBase::r_Write( s );
	if ( m_pPlayer )
	{
		m_pPlayer->r_Write(s);
	}
	else if ( m_pNPC )
	{
		m_pNPC->r_Write(s);
	}

	if ( GetTopPoint().IsValid())
		s.WriteKey( "P", GetTopPoint().Write());
	if ( ! m_sTitle.IsEmpty())
		s.WriteKey( "TITLE", m_sTitle );
	if ( m_fonttype != FONT_NORMAL )
		s.WriteKeyVal( "FONT", m_fonttype );
	if ( m_face_dir != DIR_SE )
		s.WriteKeyVal( "DIR", m_face_dir );
	s.WriteKeyHex( "OBODY", m_prev_id );
	s.WriteKeyHex( "OSKIN", m_prev_color );

	m_Events.r_Write( s, "EVENTS" );

	s.WriteKeyHex( "FLAGS", m_StatFlag );
	if ( GetActiveSkill() != SKILL_NONE )
	{
		s.WriteKeyVal( "ACTION", GetActiveSkill());
		if ( m_atUnk.m_Arg1 )
		{
			s.WriteKeyHex( "ACTARG1", m_atUnk.m_Arg1 );
		}
		if ( m_atUnk.m_Arg2 )
		{
			s.WriteKeyHex( "ACTARG2", m_atUnk.m_Arg2 );
		}
	}

	s.WriteKeyVal( "HITPOINTS", m_StatHealth );
	s.WriteKeyVal( "STAMINA", m_StatStam );
	s.WriteKeyVal( "MANA", m_StatMana);
	s.WriteKeyVal( "FOOD", m_food );

	if ( m_Home.IsValid())
	{
		s.WriteKey( "HOME", m_Home.Write());
	}

	int j=0;
	for ( ;j<STAT_QTY;j++)
	{
		if ( ! m_Stat[j] ) 
			continue;
		s.WriteKeyVal( g_Stat_Name[j], m_Stat[j] );
	}
	for ( j=0;j<SKILL_QTY;j++)
	{
		if ( ! m_Skill[j] ) 
			continue;
		s.WriteKeyVal( g_Serv.m_SkillDefs[j]->GetKey(), Skill_GetBase( (SKILL_TYPE) j ));
	}

	WriteContent(s);
}

bool CChar::r_Load( CScript & s ) // Load a character from script
{
	CScriptObj::r_Load(s);

	// Make sure everything is ok.
	if (( m_pPlayer && ! IsClient()) ||
		( m_pNPC && IsStat( STATF_DEAD | STATF_Ridden )))	// dead npc
	{
		DEBUG_CHECK( ! IsClient());
		SetDisconnected();
	}
	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
		DEBUG_ERR(( "Char 0%x Invalid, id=0%x, code=0%x\n", GetUID(), GetID(), iResultCode ));
		Delete();
	}

	return( true );
}

bool CChar::TeleportToObj( int iType, TCHAR * pszArgs )
{
	// "GONAME", "GOTYPE", "GOCHAR"
	// 0 = object name
	// 1 = char
	// 2 = item type

	DWORD dwUID = m_Act_Targ.GetIndex() &~ UID_ITEM;
	DWORD dwTotal = g_World.GetUIDCount();
	DWORD dwCount = dwTotal-1;

	int iArg;
	if ( iType )
	{
		if ( pszArgs[0] && iType == 1 )
			dwUID = 0;
		iArg = Exp_GetVal( pszArgs );
	}
	else
	{
		// strupr( pszArgs );
	}

	while ( dwCount-- )
	{
		if ( ++dwUID >= dwTotal )
		{
			dwUID = 1;
		}
		CObjBase * pObj = g_World.FindUID(dwUID);
		if ( pObj == NULL )
			continue;

		switch ( iType )
		{
		case 0:
			{
			MATCH_TYPE match = Text_Match( pszArgs, pObj->GetName());
			if ( match != MATCH_VALID )
				continue;
			}
			break;
		case 1:	// char
			{
				if ( ! pObj->IsChar()) 
					continue;
				CChar * pChar = dynamic_cast <CChar*>(pObj);
				DEBUG_CHECK( pChar );
				if ( iArg-- > 0 ) 
					continue;
			}
			break;
		case 2:	// item type
			{
				if ( ! pObj->IsItem()) 
					continue;
				CItem * pItem = dynamic_cast <CItem*>(pObj);
				ASSERT( pItem );
				if ( pItem->m_type != iArg )
					continue;
			}
			break;
		case 3: // char id
			{
				if ( ! pObj->IsChar()) 
					continue;
				CChar * pChar = dynamic_cast <CChar*>(pObj);
				DEBUG_CHECK( pChar );
				if ( pChar->GetID() != iArg ) 
					continue;
			}
			break;
		case 4:	// item id
			{
				if ( ! pObj->IsItem()) 
					continue;
				CItem * pItem = dynamic_cast <CItem*>(pObj);
				ASSERT( pItem );
				if ( pItem->GetID() != iArg )
					continue;
			}
			break;
		}

		CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
		if ( pObjTop->IsChar())
		{
			if ( ! CanDisturb( dynamic_cast<CChar*>(pObjTop))) 
				continue;
		}

		if ( pObjTop == this ) 
			continue;

		m_Act_Targ = pObj->GetUID();
		Spell_Teleport( pObjTop->GetTopPoint(), true, false );
		return( true );
	}
	return( false );
}

bool CChar::TeleportToCli( int iType, int iArgs )
{
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! iType )
		{
			if ( pClient->GetSocket() != iArgs ) 
				continue;
		}
		CChar * pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;
		if ( ! CanDisturb( pChar ))
			continue;
		if ( iType )
		{
			if ( iArgs-- ) 
				continue;
		}
		m_Act_Targ = pChar->GetUID();
		Spell_Teleport( pChar->GetTopPoint(), true, false );
		return( true );
	}
	return( false );
}

void CChar::Jail( CTextConsole * pSrc, bool fSet )
{
	if ( fSet )	// set the jailed flag.
	{
		if ( m_pPlayer )	// allow setting of this to offline chars.
		{
			m_pPlayer->GetAccount()->SetPrivFlags( PRIV_JAILED );
		}
		if ( IsClient())
		{
			m_pClient->SetPrivFlags( PRIV_JAILED );
		}
		Spell_Teleport( g_World.GetRegionPoint( "jail" ), true, false );
		SysMessage( "You have been jailed" );
	}
	else	// forgive.
	{
		if ( IsClient())
		{
			if ( ! m_pClient->IsPriv( PRIV_JAILED )) 
				return;
			m_pClient->ClearPrivFlags( PRIV_JAILED );
		}
		if ( m_pPlayer )
		{
			m_pPlayer->GetAccount()->ClearPrivFlags( PRIV_JAILED );
		}
		SysMessage( "You have been forgiven" );
	}
}

enum CV_TYPE
{
	CV_ALLSKILLS,
	CV_ANIM,		// "Bow, "Salute"
	CV_ATTACK,
	CV_BANK,
	CV_BARK,
	CV_BOUNCE,
	CV_BOW,
	CV_CONSUME,
	CV_CRIMINAL,
	CV_DISCONNECT,
	CV_DRAWMAP,
	CV_DROP,
	CV_DUPE,
	CV_EMOTE,
	CV_EQUIP,	// engage the equip triggers.
	CV_FACE,
	CV_FORGIVE,
	CV_GHOST,
	CV_GO,
	CV_GOCHAR,
	CV_GOCHARID,
	CV_GOCLI,
	CV_GOITEMID,
	CV_GONAME,
	CV_GOPLACE,
	CV_GOSOCK,
	CV_GOTYPE,
	CV_GOUID,
	CV_HEAR,
	CV_HUNGRY,
	CV_INVIS,
	CV_INVISIBLE,
	CV_INVUL,
	CV_INVULNERABLE,
	CV_JAIL,
	CV_KILL,
	CV_NEWDUPE,
	CV_NEWITEM,
	CV_NEWNPC,
	CV_PACK,
	CV_PARDON,
	CV_POLY,
	CV_REMOVE,
	CV_RESURRECT,
	CV_SALUTE,	//	salute to player
	CV_SKILL,
	CV_SLEEP,
	CV_SUICIDE,
	CV_SUMMONCAGE,
	CV_SUMMONTO,
	CV_UNDERWEAR,
	CV_UNEQUIP,	// engage the unequip triggers.
	CV_WHERE,
};

bool CChar::r_Verb( CScript &s, CTextConsole * pSrc ) // Execute command from script
{
	static const TCHAR * table[] =
	{
		"ALLSKILLS",
		"ANIM",		// "Bow", "Salute"
		"ATTACK",
		"BANK",
		"BARK",
		"BOUNCE",
		"BOW",
		"CONSUME",
		"CRIMINAL",
		"DISCONNECT",
		"DRAWMAP",
		"DROP",
		"DUPE",
		"EMOTE",
		"EQUIP",	// engage the equip triggers.
		"FACE",
		"FORGIVE",
		"GHOST",
		"GO",
		"GOCHAR",
		"GOCHARID",
		"GOCLI",
		"GOITEMID",
		"GONAME",
		"GOPLACE",
		"GOSOCK",
		"GOTYPE",
		"GOUID",
		"HEAR",
		"HUNGRY",
		"INVIS",
		"INVISIBLE",
		"INVUL",
		"INVULNERABLE",
		"JAIL",
		"KILL",
		"NEWDUPE",
		"NEWITEM",
		"NEWNPC",
		"PACK",
		"PARDON",
		"POLY",
		"REMOVE",
		"RESURRECT",
		"SALUTE",	//	salute to player
		"SKILL",
		"SLEEP",
		"SUICIDE",
		"SUMMONCAGE",
		"SUMMONTO",
		"UNDERWEAR",
		"UNEQUIP",	// engage the unequip triggers.
		"WHERE",
	};

	DEBUG_CHECK( pSrc );
	if ( this != pSrc )
	{
		// Check Priv level for person doing this to me.
		if ( pSrc->GetPrivLevel() < GetPrivLevel())
		{
			pSrc->SysMessage( "Target is more privileged than you\n" );
			return false;
		}
	}

	if ( IsClient())
	{
		if ( m_pClient->r_Verb( s, pSrc )) // Execute command from script
			return( true );
	}

	CChar * pCharSrc = pSrc->GetChar();

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table)))
	{
	case CV_ALLSKILLS:
		{
			int iVal = s.GetArgVal();
			for ( int i=0; i<SKILL_QTY; i++ )
			{
				Skill_SetBase( (SKILL_TYPE)i, iVal );
			}
		}
		break;
	case CV_ANIM:
		// ANIM, ANIM_TYPE action, bool fBackward = false, BYTE iFrameDelay = 1
		{
			int Arg_piCmd[3];		// Maximum parameters in one line
			int Arg_Qty = ParseCmds( s.GetArgStr(), Arg_piCmd, COUNTOF(Arg_piCmd));

			UpdateAnimate( (ANIM_TYPE) Arg_piCmd[0], false,
				( Arg_Qty > 1 )	? Arg_piCmd[1] : false,
				( Arg_Qty > 2 )	? Arg_piCmd[2] : 1 );
		}
		break;
	case CV_ATTACK:
		Attack( pCharSrc );
		break;
	case CV_BANK:
		// I am a banker. (or GM)
		// Open the bank box for the person who asked.
		if ( pCharSrc == NULL || ! pCharSrc->IsClient()) 
			return( false ); // NPC ?
		// DEBUG_CHECK( m_pNPC );
		pCharSrc->m_pClient->addBankOpen( pCharSrc );
		break;
	case CV_BARK:
		SoundChar( (CRESND_TYPE) ( s.HasArgs() ? s.GetArgVal() : ( GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 )));
		break;
	case CV_BOUNCE: // uid
		return ItemBounce( CObjUID( s.GetArgVal()).ItemFind());
	case CV_BOW:
		UpdateDir( pCharSrc );
		UpdateAnimate( ANIM_BOW, false );
		break;

	case CV_CONSUME:
		ResourceConsume( s.GetArgStr(), 1, false );
		break;
	case CV_CRIMINAL:
		Noto_Criminal();
		break;
	case CV_DISCONNECT:
		// Push a player char off line. CLIENTLINGER thing
		if ( IsClient())
		{
			return GetClient()->addKick( pSrc, false );
		}
		SetDisconnected();
		break;
	case CV_DRAWMAP:
		// Use the cartography skill to draw a map.
		// Already did the skill check.
		m_atCartography.m_Dist = s.GetArgVal();
		Skill_Start( SKILL_CARTOGRAPHY, 1 );
		break;
	case CV_DROP:	// uid
		return ItemDrop( CObjUID( s.GetArgVal()).ItemFind());
	case CV_DUPE:	// = dupe a creature !
		CChar::CreateNPC( GetID())->MoveNearObj( this, 1 );
		break;
	case CV_EMOTE:
		Emote( s.GetArgStr());
		break;
	case CV_EQUIP:	// uid
		return ItemEquip( CObjUID( s.GetArgVal()).ItemFind());
	case CV_FACE:	
		UpdateDir( pCharSrc );
		break;
	case CV_FORGIVE:
		goto do_pardon;

	case CV_GHOST:
		// Leave your body behind as an NPC.
		{
			// Create the ghost.
			// Switch bodies to it.
		}
		return( false );
		break;
	case CV_GO:
		goto do_goplace;
	case CV_GOCHAR:	// uid
		return TeleportToObj( 1, s.GetArgStr());
	case CV_GOCHARID:
		return TeleportToObj( 3, s.GetArgStr());
	case CV_GOCLI:	// enum clients
		return TeleportToCli( 1, s.GetArgHex());
	case CV_GOITEMID:
		return TeleportToObj( 4, s.GetArgStr());
	case CV_GONAME:
		return TeleportToObj( 0, s.GetArgStr());
	case CV_GOPLACE:
do_goplace:
		Spell_Teleport( g_World.GetRegionPoint( s.GetArgStr()), true, false );
		break;
	case CV_GOSOCK:	// sockid
		return TeleportToCli( 0, s.GetArgHex());
	case CV_GOTYPE:
		return TeleportToObj( 2, s.GetArgStr());
	case CV_GOUID:	// uid
		if ( s.HasArgs())
		{
			CObjUID uid( s.GetArgHex());
			CObjBaseTemplate * pObj = uid.ObjFind();
			if ( pObj == NULL ) 
				return( false );
			pObj = pObj->GetTopLevelObj();
			Spell_Teleport( pObj->GetTopPoint(), true, false );
			return( true );
		}
		return( false );
	case CV_HEAR:
		// NPC will hear this command but no-one else.
		if ( m_pNPC )
		{
			NPC_OnHear( s.GetArgStr(), pSrc->GetChar());
		}
		else
		{
			SysMessage( s.GetArgStr());
		}
		break;
	case CV_HUNGRY:	// How hungry are we ?
		if ( pCharSrc )
		{
			CGString sMsg;
			sMsg.Format( "%s %s %s",
				( pCharSrc == this ) ? "You" : GetName(),
				( pCharSrc == this ) ? "are" : "looks",
				GetFoodLevelMessage( false, false ));
			pCharSrc->ObjMessage( sMsg, this );
		}
		break;
	case CV_INVIS:
	case CV_INVISIBLE:
		if ( pSrc )
		{
			m_StatFlag ^= STATF_Insubstantial;
			pSrc->SysMessagef( "Invis %s", ( IsStat( STATF_Insubstantial )) ? "ON" : "OFF" );
			UpdateMode( NULL, true );	// invis used by GM bug requires this
		}
		break;
	case CV_INVUL:
	case CV_INVULNERABLE:
		if ( pSrc )
		{
			m_StatFlag ^= STATF_INVUL;
			pSrc->SysMessagef( "Invulnerability %s", ( IsStat( STATF_INVUL )) ? "ON" : "OFF" );
		}
		break;
	case CV_JAIL:	// i am being jailed
		Jail( pSrc, true );
		break;
	case CV_KILL:
		{
		Effect( EFFECT_LIGHTNING, ITEMID_NOTHING, pCharSrc );
		m_StatHealth = 0;
		g_Log.Event( LOGL_EVENT, "'%s' was KILLed by '%s'\n", GetName(), pSrc->GetName());
		}
		break;
	case CV_NEWDUPE:	// uid
		{
			CObjUID uid( s.GetArgVal());
			CObjBase * pObj = uid.ObjFind();
			m_Act_Targ = uid;	// for last target stuff. (trigger stuff)
			return pObj->r_Verb( CScript( "DUPE" ), pSrc );
		}
	case CV_NEWITEM:	// just add an item right here.
		{
			CItem * pItem = CItem::CreateTemplate( (ITEMID_TYPE) s.GetArgHex());
			if ( pItem == NULL )
				return( false );
			if ( ! pItem->MoveToCheck( GetTopPoint()))
				return( false );
			pItem->Update();
			m_Act_Targ = pItem->GetUID();	// for last target stuff. (trigger stuff)
		}
		break;
	case CV_NEWNPC:
		{
			CChar * pChar = CChar::CreateNPC( (CREID_TYPE) s.GetArgHex());
			if ( pChar == NULL )
				return( false );
			pChar->MoveNearObj( this, 1 );
			pChar->Update();
			m_Act_Targ = pChar->GetUID();	// for last target stuff. (trigger stuff)
		}
		break;
	case CV_PACK:
		if ( pCharSrc == NULL || ! pCharSrc->IsClient())
			return( false );
		pCharSrc->m_pClient->addBankOpen( this, LAYER_PACK ); // Send Backpack (with items)
		break;
	case CV_PARDON:
do_pardon:
		Jail( pSrc, false );
		break;

	case CV_POLY:	// result of poly spell script choice.

		m_atMagery.m_Spell = SPELL_Polymorph;
		m_atMagery.m_SummonID = (CREID_TYPE) s.GetArgHex();

		if ( m_pClient != NULL )
		{
			m_Act_Targ = m_pClient->m_Targ_UID;
			m_Act_TargPrv = m_pClient->m_Targ_PrvUID;
		}
		else
		{
			m_Act_Targ = GetUID();
			m_Act_TargPrv = GetUID();
		}
		Skill_Start( SKILL_MAGERY );
		break;
	case CV_REMOVE:	// remove this char from the world instantly.
		if ( m_pPlayer )
		{
			if ( GetPrivLevel() < PLEVEL_Admin || s.GetArgStr()[0] != '1' )
			{
				pSrc->SysMessage( "Can't remove players this way, Try 'kill','kick' or 'remove 1'" );
				return( false );
			}
		}
		Delete();
		break;
	case CV_RESURRECT:
		Spell_Resurrection( (pSrc->GetPrivLevel() >= PLEVEL_GM)?-1:0 );
		break;
	case CV_SALUTE:	//	salute to player
		UpdateDir( pCharSrc );
		UpdateAnimate( ANIM_SALUTE, false );
		break;
	case CV_SKILL:
		Skill_Start( (SKILL_TYPE) s.GetArgVal());
		break;
	case CV_SLEEP:
		SleepStart( s.GetArgVal());
		break;
	case CV_SUICIDE:
		Memory_ClearTypes( MEMORY_FIGHT ); // Clear the list of people who get credit for your death
		UpdateAnimate( ANIM_SALUTE );
		m_StatHealth = 0;
		break;
	case CV_SUMMONCAGE: // i just got summoned
		if ( pCharSrc != NULL )
		{
			// Let's make a cage to put the player in
			CPointMap pt = pCharSrc->GetTopPoint();
			pt.Move( pCharSrc->m_face_dir, 3 );
			CItemMulti * pItem = dynamic_cast <CItemMulti*>( CItem::CreateBase( ITEMID_M_CAGE ));
			if ( pItem == NULL ) 
				return( false );
			pItem->MoveTo(pt);
			pItem->Multi_Create( NULL );
			pItem->SetDecayTime( 10*60*TICK_PER_SEC ); // make the cage vanish after 10 minutes.
			Spell_Teleport( pt, true, false );
			break;
		}
		return( false );
	case CV_SUMMONTO:	// i just got summoned
		if ( pCharSrc != NULL )
		{
			Spell_Teleport( pCharSrc->GetTopPoint(), true, false );
		}
		break;
	case CV_UNDERWEAR:
		if ( ! IsHuman())
			return( false );
		SetColor( GetColor() ^ COLOR_UNDERWEAR );
		RemoveFromView();
		Update();
		break;
	case CV_UNEQUIP:	// uid
		return ItemBounce( CObjUID( s.GetArgVal()).ItemFind());
	case CV_WHERE:
		if ( pCharSrc )
		{
			// pCharSrc->Skill_UseQuick( SKILL_CARTOGRAPHY, 10 );

			CGString sMsg;
			if ( m_pArea )
			{
				if ( m_pArea->IsMatchType( REGION_TYPE_MULTI ))
				{
basicform:
					sMsg.Format( "I am in %s (%s)",
						m_pArea->GetName(), GetTopPoint().Write());
				}
				else
				{
					CRegionBase * pRoom = GetTopPoint().GetRegion( REGION_TYPE_ROOM );
					if ( ! pRoom ) goto basicform;

					sMsg.Format( "I am in %s in %s (%s)",
						m_pArea->GetName(), pRoom->GetName(), GetTopPoint().Write());
				}
			}
			else
			{
				// This should not happen.
				sMsg.Format( "I am at %s.", GetTopPoint().Write());
			}
			pCharSrc->ObjMessage( sMsg, this );
		}
		break;

	default:
		if ( m_pNPC )
		{
			if ( NPC_OnVerb( s, pSrc ))
				return( true );
		}
		if ( m_pPlayer )
		{
			if ( Player_OnVerb( s, pSrc ))
				return( true );
		}
		return( CObjBase::r_Verb( s, pSrc ));
	}
	return( true );
}

void CChar::InitPlayer( CEvent * pBin, CClient * pClient )
{
	// Create a brand new Player char.
	ASSERT(pClient);
	ASSERT(pBin);

	SetID( ( pBin->Create.m_sex == 0 ) ? CREID_MAN : CREID_WOMAN );

	if ( g_Serv.IsObscene(pBin->Create.m_name))	// Is the name unacceptable?
	{
		g_Log.Event( LOGL_WARN, "%x:Unacceptable obscene name '%s' for account '%s'\n", pClient->GetSocket(), pBin->Create.m_name, pClient->GetAccount()->GetName() ); 
		SetName( "unnamed" );
	}
	else
	{
		SetName( pBin->Create.m_name );
	}

	COLOR_TYPE color;
	color = pBin->Create.m_color | COLOR_UNDERWEAR;
	if ( color < (COLOR_UNDERWEAR|COLOR_SKIN_LOW) || color > (COLOR_UNDERWEAR|COLOR_SKIN_HIGH))
	{
		color = COLOR_UNDERWEAR|COLOR_SKIN_LOW;
	}
	SetColor( color );
	m_fonttype = FONT_NORMAL;

	int iStartLoc = pBin->Create.m_startloc-1;
	if ( ! g_Serv.m_StartDefs.IsValidIndex( iStartLoc ))
		iStartLoc = 0;
	m_Home = g_Serv.m_StartDefs[iStartLoc]->m_p;

	if ( ! m_Home.IsValid())
	{
		if ( g_Serv.m_StartDefs.GetCount())
		{
			m_Home = g_Serv.m_StartDefs[0]->m_p;
		}
		DEBUG_ERR(( "Invalid start location for character!\n" ));
	}

	SetUnkPoint( m_Home );	// Don't actaully put me in the world yet.

	// randomize the skills first.
	int i = 0;
	for ( ; i < SKILL_QTY; i++ )
	{
		Skill_SetBase( (SKILL_TYPE)i, GetRandVal( g_Serv.m_iMaxBaseSkill ));
	}

	if ( pBin->Create.m_str + pBin->Create.m_dex + pBin->Create.m_int > 66 )
	{
		// ! Cheater !
		pBin->Create.m_str = 10;
		pBin->Create.m_dex = 10;
		pBin->Create.m_int = 10;

		g_Log.Event( LOGL_WARN|LOGM_CHEAT, "%x:Cheater '%s' is submitting hacked char stats\n", pClient->GetSocket(), pClient->GetAccount()->GetName());
	}
	Stat_Set( STAT_STR, pBin->Create.m_str + 1 );
	Stat_Set( STAT_DEX, pBin->Create.m_dex + 1 );
	Stat_Set( STAT_INT, pBin->Create.m_int + 1 );

	if ( pBin->Create.m_val1 > 50 ||
		pBin->Create.m_val2 > 50 ||
		pBin->Create.m_val3 > 50 ||
		pBin->Create.m_val1 + pBin->Create.m_val2 + pBin->Create.m_val3 > 101 )
	{
		// ! Cheater !
		pBin->Create.m_val1 = 33;
		pBin->Create.m_val2 = 33;
		pBin->Create.m_val3 = 33;

		g_Log.Event( LOGL_WARN|LOGM_CHEAT, "%x:Cheater '%s' is submitting hacked char skills\n", pClient->GetSocket(), pClient->GetAccount()->GetName());
	}

	if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill1))
		Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill1, pBin->Create.m_val1*10 );
	if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill2))
		Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill2, pBin->Create.m_val2*10 );
	if ( IsSkillBase((SKILL_TYPE) pBin->Create.m_skill3))
		Skill_SetBase( (SKILL_TYPE) pBin->Create.m_skill3, pBin->Create.m_val3*10 );

	ITEMID_TYPE id = (ITEMID_TYPE)(WORD) pBin->Create.m_hairid;
	if ( FindID( id, Item_Hair, COUNTOF(Item_Hair)) != -1 )
	{
		CItem * pHair = CItem::CreateScript( id );
		color = pBin->Create.m_haircolor;
		if ( color<COLOR_HAIR_LOW || color > COLOR_HAIR_HIGH )
		{
			color = COLOR_HAIR_LOW;
		}
		pHair->SetColor( color );
		pHair->m_Attr |= ATTR_NEWBIE|ATTR_MOVE_NEVER;
		LayerAdd( pHair );	// add content
	}

	id = (ITEMID_TYPE)(WORD) pBin->Create.m_beardid;
	if ( FindID( id, Item_Beards, COUNTOF(Item_Beards)) != -1 && GetID() == CREID_MAN )
	{
		CItem * pBeard = CItem::CreateScript( id );
		color = pBin->Create.m_beardcolor;
		if ( color < COLOR_HAIR_LOW || color > COLOR_HAIR_HIGH )
		{
			color = COLOR_HAIR_LOW;
		}
		pBeard->SetColor( color );
		pBeard->m_Attr |= ATTR_NEWBIE|ATTR_MOVE_NEVER;
		LayerAdd( pBeard );	// add content
	}

	// Create the bank box.
	CItemContainer * pBankBox = GetBank( LAYER_BANKBOX );
	// Create the pack.
	CItemContainer * pPack = GetPackSafe();

	// Get special equip for the starting skills.
	CScript s;
	if ( ! s.OpenFind( GRAY_FILE "newb" ))
		return;
	for ( i=0; i<4; i++ )
	{
		int skill;
		const TCHAR * pszSkill;
		if (i)
		{
			switch ( i )
			{
			case 1: skill = pBin->Create.m_skill1; break;
			case 2: skill = pBin->Create.m_skill2; break;
			case 3: skill = pBin->Create.m_skill3; break;
			}
			pszSkill = g_Serv.m_SkillDefs[ skill ]->GetKey();
		}
		else
		{
			pszSkill = ( pBin->Create.m_sex == 0 ) ? "MALE_DEFAULT" : "FEMALE_DEFAULT";
		}
		if ( ! s.FindSection( pszSkill ))
		{
			continue;
		}

		ReadScript( s, false, true );
	}

	CreateNewCharCheck();
}

void CChar::UpdateDrag( CItem * pItem, CObjBase * pCont, CPointMap * ppt )
{
	// Show the world that I am picking up or putting down this object.
	// NOTE: This makes people disapear.
	CCommand cmd;
	cmd.DragAnim.m_Cmd = XCMD_DragAnim;
	cmd.DragAnim.m_id = pItem->GetDispID();
	cmd.DragAnim.m_unk3 = 0;
	cmd.DragAnim.m_unk5 = 0;
	cmd.DragAnim.m_unk7 = 0;

	CPointMap ptThis = GetTopPoint();

	if ( pCont != NULL )
	{
		// I'm putting an object in a cont..
		CObjBaseTemplate * pObjTop = pCont->GetTopLevelObj();
		if ( pObjTop == this ) 
			return;	// move stuff in my own pack.

		CPointMap ptTop = pObjTop->GetTopPoint();

		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = ptThis.m_x;
		cmd.DragAnim.m_src_y = ptThis.m_y;
		cmd.DragAnim.m_src_x = ptThis.m_z;
		cmd.DragAnim.m_dstUID = pObjTop->GetUID();
		cmd.DragAnim.m_dst_x = ptTop.m_x;
		cmd.DragAnim.m_dst_y = ptTop.m_y;
		cmd.DragAnim.m_dst_z = ptTop.m_z;
	}
	else if ( ppt != NULL )
	{
		// putting on ground.
		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = ptThis.m_x;
		cmd.DragAnim.m_src_y = ptThis.m_y;
		cmd.DragAnim.m_src_x = ptThis.m_z;
		cmd.DragAnim.m_dstUID = 0;
		cmd.DragAnim.m_dst_x = ppt->m_x;
		cmd.DragAnim.m_dst_y = ppt->m_y;
		cmd.DragAnim.m_dst_z = ppt->m_z;
	}
	else
	{
		// I'm getting an object from where ever it is.

		// ??? Note: this doesn't work for ground objects !
		CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
		if ( pObjTop == this ) 
			return;	// move stuff in my own pack.

		CPointMap ptTop = pObjTop->GetTopPoint();

		cmd.DragAnim.m_srcUID = (pObjTop==pItem) ? 0 : (DWORD) pObjTop->GetUID();
		cmd.DragAnim.m_src_x = ptTop.m_x;
		cmd.DragAnim.m_src_y = ptTop.m_y;
		cmd.DragAnim.m_src_z = ptTop.m_z;
		cmd.DragAnim.m_dstUID = 0; // GetUID();
		cmd.DragAnim.m_dst_x = ptThis.m_x;
		cmd.DragAnim.m_dst_y = ptThis.m_y;
		cmd.DragAnim.m_dst_x = ptThis.m_z;
	}

	UpdateCanSee( &cmd, sizeof(cmd.DragAnim), m_pClient );
}

void CChar::UpdateStats( STAT_TYPE type, int iChange, int iLimit )
{
	if ( iChange )
	{
		ASSERT( ((WORD)type) < COUNTOF(m_StatVal));

		int iVal = m_StatVal[type].m_val;
		if ( ! iLimit ) iLimit = Stat_Get(type);
		if ( iChange < 0 )
		{
			iVal += iChange;
		}
		else if ( iVal > iLimit )
		{
			iVal -= iChange;
			if ( iVal < iLimit ) iVal = iLimit;
		}
		else
		{
			iVal += iChange;
			if ( iVal > iLimit ) iVal = iLimit;
		}
		if ( iVal < 0 ) iVal = 0;
		m_StatVal[type].m_val = iVal;
	}

	CCommand cmd;
	cmd.StatChng.m_Cmd = XCMD_StatChngStr + type - STAT_STR;
	cmd.StatChng.m_UID = GetUID();
	cmd.StatChng.m_max = Stat_Get(type);
	cmd.StatChng.m_val = m_StatVal[type].m_val;

	if ( type == STAT_STR )	// everyone sees my health
	{
		UpdateCanSee( &cmd, sizeof(cmd.StatChng), m_pClient );
	}
	if ( IsClient())
	{
		m_pClient->xSendPkt( &cmd, sizeof(cmd.StatChng));
	}
}

void CChar::UpdateAnimate( ANIM_TYPE action, bool fTranslate, bool fBackward, BYTE iFrameDelay )
{
	// NPC or character does a certain Animate
	// Translate the animation based on creature type.

	if ( fTranslate || IsStat( STATF_OnHorse ))	
	{
		CItem * pWeapon = m_weapon.ItemFind();
		if ( pWeapon != NULL && action == ANIM_ATTACK_WEAPON )
		{
			// action depends on weapon type (skill) and 2 Hand type.
			switch ( pWeapon->m_type )
			{
			case ITEM_WEAPON_MACE_CROOK:
			case ITEM_WEAPON_MACE_PICK:
			case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
			case ITEM_WEAPON_MACE_STAFF:
			case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
				action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
					ANIM_ATTACK_2H_DOWN :
					ANIM_ATTACK_1H_DOWN;
				break;
			case ITEM_WEAPON_SWORD:
				action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
					ANIM_ATTACK_2H_WIDE :
					ANIM_ATTACK_1H_WIDE;
				break;
			case ITEM_WEAPON_FENCE:
				action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
					ANIM_ATTACK_2H_JAB :
					ANIM_ATTACK_1H_JAB;
				break;
			case ITEM_WEAPON_BOW:
				// normal or xbow ?
				action = ( pWeapon->Armor_IsXBow()) ?
					ANIM_ATTACK_XBOW : ANIM_ATTACK_BOW ;
				break;
			}

			// add some style to the attacks.
			if ( pWeapon->m_type != ITEM_WEAPON_BOW && GetRandVal( 2 ))
			{
				if ( pWeapon->m_pDef->m_layer == LAYER_HAND2 )
				{
					action = (ANIM_TYPE)( ANIM_ATTACK_2H_DOWN + GetRandVal(3));
				}
				else
				{
					action = (ANIM_TYPE)( ANIM_ATTACK_1H_WIDE + GetRandVal(3));
				}
			}
		}

		if ( IsStat( STATF_OnHorse ))	// on horse back.
		{
			// Horse back anims are dif.
			switch ( action )
			{
			case ANIM_WALK_UNARM:
			case ANIM_WALK_ARM:
				action = ANIM_HORSE_RIDE_SLOW;
				break;
			case ANIM_RUN_UNARM:
			case ANIM_RUN_ARMED:
				action = ANIM_HORSE_RIDE_FAST;
				break;
			case ANIM_STAND:
				action = ANIM_HORSE_STAND;
				break;
			case ANIM_FIDGET1:
			case ANIM_FIDGET2:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_STAND_WAR_1H:
			case ANIM_STAND_WAR_2H:
				action = ANIM_HORSE_STAND;
				break;
			case ANIM_ATTACK_1H_WIDE:
			case ANIM_ATTACK_1H_JAB:
			case ANIM_ATTACK_1H_DOWN:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_ATTACK_2H_JAB:
			case ANIM_ATTACK_2H_WIDE:
			case ANIM_ATTACK_2H_DOWN:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_WALK_WAR:
				action = ANIM_HORSE_RIDE_SLOW;
				break;
			case ANIM_CAST_DIR:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_CAST_AREA:
				action = ANIM_HORSE_ATTACK_BOW;
				break;
			case ANIM_ATTACK_BOW:
				action = ANIM_HORSE_ATTACK_BOW;
				break;
			case ANIM_ATTACK_XBOW:
				action = ANIM_HORSE_ATTACK_XBOW;
				break;
			case ANIM_GET_HIT:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_TURN:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_ATTACK_UNARM:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_BOW:
			case ANIM_SALUTE:
			case ANIM_EAT:
				action = ANIM_HORSE_ATTACK_XBOW;
				break;
			default:
				action = ANIM_HORSE_STAND;
				break;
			}
		}
		else if ( GetDispID() < CREID_MAN )
		{
			// Animals have certain anims. Monsters have others.

			if ( GetDispID() >= CREID_HORSE1 )
			{
				// All animals have all these anims thankfully
				switch ( action )
				{
				case ANIM_WALK_UNARM:
				case ANIM_WALK_ARM:
				case ANIM_WALK_WAR:
					action = ANIM_ANI_WALK;
					break;
				case ANIM_RUN_UNARM:
				case ANIM_RUN_ARMED:
					action = ANIM_ANI_RUN;
					break;
				case ANIM_STAND:
				case ANIM_STAND_WAR_1H:
				case ANIM_STAND_WAR_2H:
				default:
					action = ANIM_ANI_STAND;
					break;

				case ANIM_FIDGET1:
					action = ANIM_ANI_FIDGET1;
					break;
				case ANIM_FIDGET2:
					action = ANIM_ANI_FIDGET2;
					break;
				case ANIM_CAST_DIR:
					action = ANIM_ANI_ATTACK1;
					break;
				case ANIM_CAST_AREA:
					action = ANIM_ANI_EAT;
					break;
				case ANIM_GET_HIT:
					action = ANIM_ANI_ATTACK3;
					break;

				case ANIM_ATTACK_1H_WIDE:
				case ANIM_ATTACK_1H_JAB:
				case ANIM_ATTACK_1H_DOWN:
				case ANIM_ATTACK_2H_DOWN:
				case ANIM_ATTACK_2H_JAB:
				case ANIM_ATTACK_2H_WIDE:
				case ANIM_ATTACK_BOW:
				case ANIM_ATTACK_XBOW:
				case ANIM_ATTACK_UNARM:
					switch ( GetRandVal(3))
					{
					case 0: action = ANIM_ANI_ATTACK1; break;
					case 1: action = ANIM_ANI_ATTACK2; break;
					case 2: action = ANIM_ANI_ATTACK3; break;
					}
					break;

				case ANIM_DIE_BACK:
					action = ANIM_ANI_DIE1;
					break;
				case ANIM_DIE_FORWARD:
					action = ANIM_ANI_DIE2;
					break;
				case ANIM_TURN:
				case ANIM_BOW:
				case ANIM_SALUTE:
					action = ANIM_ANI_SLEEP;
					break;
				case ANIM_EAT:
					action = ANIM_ANI_EAT;
					break;
				}
				while ( action != ANIM_WALK_UNARM && ! ( m_pDef->m_Anims & (1<<action)))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
					case ANIM_ANI_SLEEP:	// All have this.
						action = ANIM_ANI_EAT;
						break;
					default:
						action = ANIM_WALK_UNARM;
						break;
					}
				}
			}
			else
			{
				// Monsters don't have all the anims.

				switch ( action )
				{
				case ANIM_CAST_DIR:
					action = ANIM_MON_MISC_BREATH;
					break;
				case ANIM_CAST_AREA:
					action = ANIM_MON_MISC_STOMP;
					break;
				case ANIM_DIE_BACK:
					action = ANIM_MON_DIE1;
					break;
				case ANIM_DIE_FORWARD:
					action = ANIM_MON_DIE2;
					break;
				case ANIM_GET_HIT:
					switch ( GetRandVal(4))
					{
					case 0: action = ANIM_MON_GETHIT1; break;
					case 1: action = ANIM_MON_GETHIT2; break;
					case 2: action = ANIM_MON_GETHIT3; break;
					case 3: action = ANIM_MON_STUMBLE; break;
					}
					break;
				case ANIM_ATTACK_1H_WIDE:
				case ANIM_ATTACK_1H_JAB:
				case ANIM_ATTACK_1H_DOWN:
				case ANIM_ATTACK_2H_DOWN:
				case ANIM_ATTACK_2H_JAB:
				case ANIM_ATTACK_2H_WIDE:
				case ANIM_ATTACK_BOW:
				case ANIM_ATTACK_XBOW:
				case ANIM_ATTACK_UNARM:
					switch ( GetRandVal(3))
					{
					case 0: action = ANIM_MON_ATTACK1; break;
					case 1: action = ANIM_MON_ATTACK2; break;
					case 2: action = ANIM_MON_ATTACK3; break;
					}
					break;
				default:
					action = ANIM_WALK_UNARM;
					break;
				}
				// NOTE: Available actions depend HEAVILY on creature type !
				// ??? Monsters don't have all anims in common !
				// translate these !
				while ( action != ANIM_WALK_UNARM && ! ( m_pDef->m_Anims & (1<<action)))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
					case ANIM_MON_ATTACK1:	// All have this.
						DEBUG_ERR(( "Anim 0%x This is wrong! Invalid SCP file data.\n", GetDispID()));
						action = ANIM_WALK_UNARM;
						break;

					case ANIM_MON_ATTACK2:	// Dolphins, Eagles don't have this.
					case ANIM_MON_ATTACK3:
						action = ANIM_MON_ATTACK1;	// ALL creatures have at least this attack.
						break;
					case ANIM_MON_GETHIT1:	// Trolls, Spiders, many others don't have this.
						action = ANIM_MON_GETHIT2;	// Birds don't have this !
						break;
					case ANIM_MON_GETHIT2:
						action = ANIM_MON_GETHIT3;
						break;
					case ANIM_MON_GETHIT3:
						action = ANIM_MON_STUMBLE;
						break;
					case ANIM_MON_STUMBLE:
						if ( m_pDef->m_Anims & (1<<ANIM_MON_GETHIT1))
							action = ANIM_MON_GETHIT1;
						else
							action = ANIM_WALK_UNARM;
						break;

					case ANIM_MON_MISC_BREATH:
						action = ANIM_MON_MISC_STOMP;
						break;
					case ANIM_MON_MISC_STOMP:
						action = ANIM_MON_ATTACK3;
						break;

					default:
						DEBUG_ERR(( "Anim Unsupported 0%x for 0%x\n", action, GetDispID()));
						action = ANIM_WALK_UNARM;
						break;
					}
				}
			}
		}
	}

	if ( action > ANIM_QTY ) 
		return;

	WORD wRepeat = 1;

	CCommand cmd;
	cmd.CharAction.m_Cmd = XCMD_CharAction;
	cmd.CharAction.m_UID = GetUID();
	cmd.CharAction.m_action = action;
	cmd.CharAction.m_zero7 = 0;
	cmd.CharAction.m_dir = m_face_dir;
	cmd.CharAction.m_repeat = wRepeat;		// 1, repeat count. 0=forever.
	cmd.CharAction.m_backward = fBackward ? 1 : 0;	// 0, backwards (0/1)
	cmd.CharAction.m_repflag = ( wRepeat == 1 ) ? 0 : 1;	// 0=dont repeat. 1=repeat
	cmd.CharAction.m_framedelay = iFrameDelay;	// 1, 0=fastest.

	UpdateCanSee( &cmd, sizeof(cmd.CharAction));
}

void CChar::UpdateMode( CClient * pExcludeClient, bool fFull )
{
	// If character status has been changed
	// (Polymorph, war mode or hide), resend him
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pExcludeClient == pClient ) 
			continue;
		if ( ! pClient->CanSee( this ))
		{
			// In the case of "INVIS" used by GM's we must use this.
			if ( GetDist( pClient->GetChar()) <= UO_MAP_VIEW_SIZE )
			{
				pClient->addObjectRemove( this );
			}
			continue;
		}
		if ( pClient->IsPriv( PRIV_DEBUG )) 
			continue;
		if ( fFull )
		{
			pClient->addChar( this );
		}
		else
		{
			pClient->addCharMove( this );
		}
	}
}

void CChar::UpdateMove( CPointMap pold, CClient * pExcludeClient )
{
	// Who now sees this char ?
	// Did they just see him move ?
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pExcludeClient ) 
			continue;	// no need to see self move.
		if ( pClient == m_pClient )
		{
			// What do i now see ?
			pClient->addPlayerView( pold );
			continue;
		}
		CChar * pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;

		bool fCouldSee = ( pold.GetDist( pChar->GetTopPoint()) <= UO_MAP_VIEW_SIZE );

		if ( ! pClient->CanSee( this ))
		{	// can't see me now.
			if ( fCouldSee ) pClient->addObjectRemove( this );
		}
		else if ( fCouldSee )
		{	// They see me move.
			pClient->addCharMove( this );
		}
		else
		{	// first time this client has seen me.
			pClient->addChar( this );
		}
	}
}

void CChar::UpdateDir( DIR_TYPE dir )
{
	if ( dir != m_face_dir && dir < DIR_QTY )
	{
		m_face_dir = dir;	// face victim.
		UpdateMove(GetTopPoint());
	}
}

void CChar::UpdateDir( const CPointMap & pt )
{
	// Change in direction.
	UpdateDir( GetTopPoint().GetDir( pt ));
}

void CChar::UpdateDir( const CObjBaseTemplate * pObj )
{
	if ( pObj == NULL ) 
		return;
	pObj = pObj->GetTopLevelObj();
	if ( pObj == this )		// In our own pack.
		return;
	UpdateDir( pObj->GetTopPoint());
}

void CChar::Update( const CClient * pClientExclude ) // If character status has been changed (Polymorph), resend him
{
	// Or I changed looks.
	// I moved or somebody moved me  ?
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude )
			continue;
		if ( pClient == m_pClient )
		{
			pClient->addReSync();
		}
		else if ( pClient->CanSee( this ))
		{
			pClient->addChar( this );
		}
	}
}

SOUND_TYPE CChar::SoundChar( CRESND_TYPE type )
{
	SOUND_TYPE id;

	if ( GetDispID() == CREID_BLADES )
	{
		id = m_pDef->m_soundbase; // just one sound
	}
	else if ( GetDispID() >= CREID_MAN )
	{
		id = 0;

		static const SOUND_TYPE Snd_Man_Die[] = { 0x15a, 0x15b, 0x15c, 0x15d };
		static const SOUND_TYPE Snd_Man_Omf[] = { 0x154, 0x155, 0x156, 0x157, 0x158, 0x159 };
		static const SOUND_TYPE Snd_Wom_Die[] = { 0x150, 0x151, 0x152, 0x153 };
		static const SOUND_TYPE Snd_Wom_Omf[] = { 0x14b, 0x14c, 0x14d, 0x14e, 0x14f };

		if ( CCharBase::IsFemaleID( GetDispID()))
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = Snd_Wom_Omf[ GetRandVal( COUNTOF(Snd_Wom_Omf)) ];
				break;
			case CRESND_DIE:
				id = Snd_Wom_Die[ GetRandVal( COUNTOF(Snd_Wom_Die)) ];
				break;
			}
		}
		else
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = Snd_Man_Omf[ GetRandVal( COUNTOF(Snd_Man_Omf)) ];
				break;
			case CRESND_DIE:
				id = Snd_Man_Die[ GetRandVal( COUNTOF(Snd_Man_Die)) ];
				break;
			}
		}
	}
	else
	{
		id = m_pDef->m_soundbase + type;
		switch ( m_pDef->m_soundbase )	// some creatures have no base sounds.
		{
		case 128: // old versions
		case 181:
		case 199:
			if ( type <= CRESND_RAND2 ) id = 0;
			break;
		case 130: // ANIMALS_DEER3
		case 183: // ANIMALS_LLAMA3
		case 201: // ANIMALS_RABBIT3
			if ( type <= CRESND_RAND2 ) id = 0;
			else id -= 2;
			break;
		}
	}

	if ( type == CRESND_HIT )
	{
		CItem * pWeapon = m_weapon.ItemFind();
		if ( pWeapon != NULL )
		{
			// weapon type strike noise based on type of weapon and how hard hit.

			switch ( pWeapon->m_type )
			{
			case ITEM_WEAPON_MACE_CROOK:
			case ITEM_WEAPON_MACE_PICK:
			case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
			case ITEM_WEAPON_MACE_STAFF:
				// 0x233 = blunt01 (miss?)
				id = 0x233;
				break;
			case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
				// 0x232 = axe01 swing. (miss?)
				id = 0x232;
				break;
			case ITEM_WEAPON_SWORD:
				if ( pWeapon->m_pDef->m_layer == LAYER_HAND2 )
				{
					// 0x236 = hvyswrd1 = (heavy strike)
					// 0x237 = hvyswrd4 = (heavy strike)
					id = GetRandVal( 2 ) ? 0x236 : 0x237;
					break;
				}
			case ITEM_WEAPON_FENCE:
				// 0x23b = sword1
				// 0x23c = sword7
				id = GetRandVal( 2 ) ? 0x23b : 0x23c;
				break;
			case ITEM_WEAPON_BOW:
				// 0x234 = xbow ( hit)
				id = 0x234;
				break;
			}
		}
		else if ( id == 0 )
		{
			static const SOUND_TYPE Snd_Hit[] =
			{
				0x135, //= hit01 = (slap)
				0x137, //= hit03 = (hit sand)
				0x13b, //= hit07 = (hit slap)
			};
			id = Snd_Hit[ GetRandVal( COUNTOF( Snd_Hit )) ];
		}
	}

	if ( id <= 0 ) return( 0 );
	Sound( id );
	return( id );
}

int CChar::ItemPickup( CItem * pItem, int amount )
{
	// Pickup off the ground or remove my own equipment. etc..
	// This item is now "up in the air"
	// RETURN: amount we can pick up.

	if ( amount < 0 ||
		pItem == NULL ||
		! CanTouch( pItem ) ||
		! CanMove( pItem ))
	{
		return -1;
	}

	const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
	const CChar * pChar = dynamic_cast <const CChar*> (pObjTop);

	if ( pChar != this &&
		pItem->IsAttr(ATTR_OWNED) &&
		pItem->m_uidLink != GetUID() &&
		! IsPriv(PRIV_ALLMOVE|PRIV_GM))
	{
muststeal:
		SysMessage( "That is not yours. You will have to steal the item" );
		return -1;
	}

	const CItemCorpse * pCorpseItem = dynamic_cast <const CItemCorpse *>(pObjTop);
	if ( pCorpseItem )
	{
		// Taking stuff off someones corpse can be a crime !
		CChar * pCharCorpse = pCorpseItem->m_uidLink.CharFind();
		if ( Use_CarveCorpseTest( pCharCorpse, pCorpseItem, true ))
		{
			SysMessage( "Guards can now be called on you." );
		}
	}

	int iAmountMax = pItem->GetAmount();
	if ( iAmountMax <= 0 )
		return( -1 );
	if ( amount <= 0 )
		amount = 1;
	if ( amount > iAmountMax )
		amount = iAmountMax;
	int iItemWeight;
	if ( amount <= 1 )
	{
		iItemWeight = pItem->GetWeight();	// might be a container with stuff in it.
	}
	else
	{
		iItemWeight = ( pItem->m_pDef->GetWeight() * amount );
	}

	// Is it too heavy to even drag ?
	bool fDrop = false;
	if ( GetWeightLoadPercent( GetTotalWeight() + iItemWeight ) > 300 )
	{
		SysMessage("That is too heavy. You can't move that.");
		if ( pChar != this )
		{
			return( -1 );
		}
		fDrop = true;
	}

	ITRIG_TYPE trigger;
	if ( pChar != NULL )
	{
		if ( ! pChar->NPC_IsOwnedBy( this ))
			goto muststeal;
		trigger = pItem->IsEquipped() ? ITRIG_UNEQUIP : ITRIG_PICKUP_PACK;
	}
	else
	{
		trigger = pItem->IsTopLevel() ? ITRIG_PICKUP_GROUND : ITRIG_PICKUP_PACK;
	}

	if ( trigger != ITRIG_UNEQUIP)	// unequip is done at a lower level.
	{
		if ( pItem->OnTrigger( trigger, this, amount ))
			return( -1 );
	}

	if ( pItem->m_pDef->IsStackableType() && amount )
	{
		// Did we only pick up part of it ?
		// part or all of a pile. Only if pilable !
		if ( amount < iAmountMax )
		{
			// create left over item.
			pItem->CreateDupeSplit( amount );
		}
	}
	else
	{
		amount = iAmountMax;
	}

	if ( fDrop )
	{
		pItem->MoveToCheck( GetTopPoint());
		return( -1 );
	}

	// do the dragging anim for everyone else to see.
	UpdateDrag( pItem );

	// Pick it up.
	pItem->SetDecayTime(-1);	// Kill any decay timer.
	LayerAdd( pItem, LAYER_DRAGGING );

	return( amount );
}

bool CChar::ItemBounce( CItem * pItem )
{
	// We can't put this where we want to
	// So put in my pack if i can. else drop.
	// don't check where this came from !

	if ( pItem == NULL )
		return false;

	const TCHAR * pszWhere;
	if ( CanCarry( pItem ))
	{
		// if we can carry it
		CItemContainer * pPack = GetPackSafe();
		if ( pPack == NULL )
			goto dropit;	// this can happen at load time.
		pPack->ContentAdd( pItem ); // Add it to pack
		Sound( 0x057 );
		pszWhere = "in your pack";
	}
	else
	{
dropit:
		if ( ! GetTopPoint().IsValid())
		{
			// NPC is being created and has no valid point yet.
			DEBUG_ERR(( "Loot item 0%x too heavy for NPC 0%x\n", pItem->GetID(), GetID()));
			pItem->Delete();
			return false;
		}
		pszWhere = "at your feet. It is too heavy.";
		ItemDrop( pItem );
	}

	SysMessagef( "You put the %s%s %s.", pItem->GetName(), (pItem->GetAmount() <= 1 ? "" : "s"), pszWhere );
	return( true );
}

bool CChar::ItemDrop( CItem * pItem )
{
	if ( pItem == NULL ) 
		return( false );
	return( pItem->MoveToCheck( GetTopPoint()));
}

bool CChar::ItemEquip( CItem * pItem )
{
	// Pay no attention to where this came from.

	if ( pItem == NULL )
		return( false );
	if ( pItem->GetParent() == this && pItem->GetEquipLayer() != LAYER_DRAGGING )
		return( true );
	if ( ! CanCarry( pItem ))
	{
		SysMessagef( "%s is too heavy.", pItem->GetName());
		return( false );
	}

	// strong enough to equip this ?
	if ( pItem->IsArmorWeapon())
	{
		if ( Stat_Get(STAT_STR) < pItem->m_pDef->m_StrReq )
		{
			SysMessagef( "Not strong enough to equip %s.", pItem->GetName());
			return( false );
		}
	}

	if ( CanEquipLayer( pItem ) == LAYER_NONE )
	{
		SysMessage( "You can't equip this" );
		return( false );
	}

	if ( IsClient())
	{
		GetClient()->addPause();
	}

	if ( pItem->OnTrigger( ITRIG_EQUIP, this, pItem->m_pDef->m_layer ))
	{
		return( false );
	}

	pItem->SetDecayTime(-1);	// Kill any decay timer.

	LayerAdd( pItem );
	if ( ! pItem->IsEquipped())	// Equip failed ? (cursed?) Did it just go into pack ?
		return( false );

	Spell_Effect_Add( pItem );	// if it has a magic effect.
	Sound( 0x057 );
	return( true );
}

void CChar::EatAnim( const TCHAR * pszName, int iQty )
{
	m_food += iQty;

	static const SOUND_TYPE EatSounds[] = { 0x03a, 0x03b, 0x03c };

	Sound( EatSounds[ GetRandVal( COUNTOF(EatSounds)) ] );
	UpdateAnimate( ANIM_EAT );

	CGString sEmoteMessage;
	sEmoteMessage.Format( "eat some %s", pszName );
	Emote(sEmoteMessage);
}

bool CChar::Reveal( DWORD dwFlags )
{
	// Some outside influence may be revealing us.

	if ( ! IsStat( dwFlags ))	// no effect.
		return( false );

	if (( dwFlags & STATF_Sleeping ) && IsStat( STATF_Sleeping ))
	{
		// Try to wake me up.
		Wake();
	}
	bool fInvis = false;
	if (( dwFlags & STATF_Invisible ) && IsStat( STATF_Invisible  ))
	{
		fInvis = true;
		SetColor( m_prev_color );
	}

	ClearStat( dwFlags );
	if ( IsStat( STATF_Invisible | STATF_Hidden | STATF_Insubstantial | STATF_Sleeping ))
		return( false );

	if ( fInvis )
	{
		RemoveFromView();	// just the change in color requires this .
		Update();
	}
	else
	{
		UpdateMode( NULL, true );
	}

	SysMessage( "You have been revealed" );

	if ( GetDispID() == CREID_CORPSER )
	{
		// Comes out from under the ground.
		UpdateAnimate( ANIM_MON_MISC_BREATH, false );
		Sound( 0x221 );
	}

	return( true );
}

void CChar::Emote( const TCHAR * pText, CClient * pClientExclude, bool fPosessive )
{
	// "*You see NAME blah*" or "*You blah*"
	// fPosessive = "*You see NAME's blah*" or "*Your blah*"

	CGString sMsgThem;
	CGString sMsgUs;
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude ) 
			continue;
		if ( ! pClient->CanSee( this )) 
			continue;
		const TCHAR * pEmote;
		if ( pClient == m_pClient )
		{
			sMsgUs.Format( "*%s %s*", fPosessive ? "Your" : "You", pText );
			pEmote = sMsgUs;
		}
		else
		{
			if ( sMsgThem.IsEmpty())
			{
				sMsgThem.Format( "*You see %s%s %s*", GetName(), fPosessive ? "'s" : "", pText );
			}
			pEmote = sMsgThem;
		}
		pClient->addObjMessage( pEmote, this, COLOR_RED );
	}
}

void CChar::SpeakUTF8( const TCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, const char * pszLanguage )
{
	if ( IsStat(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::SpeakUTF8( pText, color, mode, m_fonttype, pszLanguage );
}

void CChar::SpeakUNICODE( const NCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode, const char * pszLanguage )
{
	if ( IsStat(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::SpeakUNICODE( pText, color, mode, m_fonttype, pszLanguage );
}

void CChar::Speak( const TCHAR * pText, COLOR_TYPE color, TALKMODE_TYPE mode )
{
	// Speak to all clients in the area.
	if ( IsStat(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::Speak( pText, color, mode, m_fonttype );
}

CItem * CChar::Make_Figurine( CObjUID uidOwner, ITEMID_TYPE id )
{
	// Make me into a figurine
	if ( IsDisconnected())	// we are already a figurine !
		return( NULL );
	if ( m_pPlayer )
		return( NULL );

	// turn creature into a figurine.
	CItem * pItem = CItem::CreateScript( ( id == ITEMID_NOTHING ) ? m_pDef->m_trackID : id );
	pItem->SetName( GetName());
	pItem->m_type = ITEM_FIGURINE;
	pItem->SetColor( GetColor());
	pItem->m_itFigurine.m_ID = GetID();	// Base type of creature.
	pItem->m_itFigurine.m_UID = GetUID();
	pItem->m_uidLink = uidOwner;

	if ( IsStat( STATF_Insubstantial )) 
	{
		pItem->m_Attr |= ATTR_INVIS;
	}

	SoundChar( CRESND_RAND1 );	// Horse winny
	m_atRidden.m_FigurineUID = pItem->GetUID();
	SetStat( STATF_Ridden );
	Skill_Start( NPCACT_RIDDEN );
	SetDisconnected();

	return( pItem );
}

CItem * CChar::NPC_Shrink()
{
	// This will just kill conjured creatures.
	if ( IsStat( STATF_Conjured ))
	{
		m_StatHealth = 0;
		return( NULL );
	}

	CItem * pItem = Make_Figurine( UID_CLEAR, ITEMID_NOTHING );
	if ( pItem == NULL )
		return( NULL );

	pItem->m_Attr |= ATTR_MAGIC;
	pItem->MoveToCheck( GetTopPoint());
	return( pItem );
}

CItem * CChar::Horse_GetMountItem() const
{
	// I am a horse.
	// Get my mount object. (attached to my rider)

	if ( ! IsStat( STATF_Ridden ))
		return( NULL );

	DEBUG_CHECK( GetActiveSkill() == NPCACT_RIDDEN );
	DEBUG_CHECK( m_pNPC );

	CItem * pItem = m_atRidden.m_FigurineUID.ItemFind();
	if ( pItem == NULL ||
		( pItem->m_type != ITEM_FIGURINE && pItem->m_type != ITEM_EQ_HORSE ))
	{
		return( NULL );
	}

	DEBUG_CHECK( pItem->m_itFigurine.m_UID == GetUID());
	return( pItem );
}

CChar * CChar::Horse_GetMountChar() const
{
	CItem * pItem = Horse_GetMountItem();
	if ( pItem == NULL ) 
		return( NULL );
	return( dynamic_cast <CChar*>( pItem->GetTopLevelObj()));
}

const WORD g_Item_Horse_Mounts[][2] = // extern
{
	ITEMID_M_HORSE1,		CREID_HORSE1,
	ITEMID_M_HORSE2,		CREID_HORSE2,
	ITEMID_M_HORSE3,		CREID_HORSE3,
	ITEMID_M_HORSE4,		CREID_HORSE4,
	ITEMID_M_OSTARD_DES,	CREID_Ostard_Desert,	// t2A
	ITEMID_M_OSTARD_Frenz,	CREID_Ostard_Frenz,		// t2A
	ITEMID_M_OSTARD_For,	CREID_Ostard_Forest,	// t2A
	ITEMID_M_LLAMA,			CREID_Llama,			// t2A
	0,0,
};

bool CChar::Horse_Mount( CChar * pHorse ) // Remove horse char and give player a horse item
{
	// RETURN:
	//  true = done mounting so take no more action.
	//  false = we can't mount this so do something else.
	//

	if ( ! CanTouch( pHorse ))
	{
		SysMessage( "You can't reach the creature." );
		return( false );
	}

	ITEMID_TYPE id;
	for ( int i=0; true; i++ )
	{
		if ( i>=COUNTOF(g_Item_Horse_Mounts))
		{
			return( false );
		}
		if ( pHorse->GetDispID() == g_Item_Horse_Mounts[i][1] )
		{
			id = (ITEMID_TYPE) g_Item_Horse_Mounts[i][0];
			break;
		}
	}

	if ( IsStat( STATF_DEAD ) ||	// can't mount horse if dead!
		! IsHuman())	// only humans can ride horses.
	{
		SysMessage( "You are not physically capable of riding a horse." );
		return( false );
	}
	if ( ! pHorse->NPC_IsOwnedBy( this ) || pHorse->m_pPlayer )
	{
		SysMessage( "You dont own that horse." );
		return( false );
	}

	Horse_UnMount();	// unmount if already on a horse.

	CItem * pItem = pHorse->Make_Figurine( GetUID(), id );
	pItem->m_type = ITEM_EQ_HORSE;
	pItem->SetTimeout( 10*TICK_PER_SEC );	// give the horse a tick everyone once in a while.
	LayerAdd( pItem, LAYER_HORSE );	// equip the horse item

	return( true );
}

bool CChar::Horse_UnMount() // Get off a horse (Remove horse item and spawn new horse)
{
	if ( ! IsStat( STATF_OnHorse )) 
		return( false );

	CItem * pItem = LayerFind( LAYER_HORSE );
	if ( pItem == NULL )
	{
		ClearStat( STATF_OnHorse );	// flag got out of sync !
		return( false );
	}

	// What creature is the horse item ?
	CChar * pHorse = Use_Figurine( pItem, 0 );
	pItem->Delete();
	return( true );
}

bool CChar::OnEquipTick( CItem * pItem )
{
	// A timer expired for an item we are carrying.
	// Does it periodically do something ?
	// REUTRN:
	//  false = delete it.

	switch ( pItem->GetEquipLayer())
	{
	case LAYER_FLAG_Wool:
		// This will regen the sheep it was sheered from.
		// Sheared sheep regen wool on a new day.
		if ( GetID() != CREID_Sheep_Sheered )
			return false;

		// Is it a new day ? regen my wool.
		SetID( CREID_Sheep );
		return false;

	case LAYER_FLAG_Drunk:
		// Drunk
		if ( ! pItem->m_itSpell.m_charges || ! pItem->m_itSpell.m_skilllevel )
		{
			return( false );
		}
		Speak( "*Hic*" );
		if ( GetRandVal(2))
		{
			UpdateAnimate( ANIM_BOW );
		}
		pItem->m_itSpell.m_charges--;
		if ( GetRandVal(2))
		{
			Spell_Effect_Remove( pItem );
			pItem->m_itSpell.m_skilllevel--;	// weaken the effect.
			Spell_Effect_Add( pItem );
		}

		// We will have this effect again.
		pItem->SetTimeout( GetRandVal(10)*TICK_PER_SEC );
		return true;

	case LAYER_FLAG_Hallucination:
		if ( ! pItem->m_itSpell.m_charges || ! pItem->m_itSpell.m_skilllevel )
		{
			return( false );
		}
		pItem->m_itSpell.m_charges --;
		if ( IsClient())
		{
			static const SOUND_TYPE sounds[] = { 0x243, 0x244, 0x245 };
			m_pClient->addSound( sounds[ GetRandVal( COUNTOF( sounds )) ] );
			m_pClient->addReSync();
		}
		// save the effect.
		pItem->SetTimeout( (15+GetRandVal(15))*TICK_PER_SEC );
		return true;

	case LAYER_FLAG_ClientLinger:
		// remove me from other clients screens.
		DEBUG_CHECK( pItem->m_type == ITEM_EQ_CLIENT_LINGER );
		SetDisconnected();
		return( false );

	case LAYER_SPECIAL:
		if ( pItem->m_type == ITEM_EQ_NPC_SCRIPT )
		{
			// Make the next action in the script if it is running.
			NPC_Script_OnTick( dynamic_cast <CItemMessage*>( pItem ));
			return( true );
		}
		if ( pItem->m_type == ITEM_EQ_MEMORY_OBJ )
		{
			return Memory_OnTick( dynamic_cast <CItemMemory*>( pItem ));
		}
		DEBUG_CHECK( 0 );	// should be no other types here.
		break;

	case LAYER_FLAG_Stuck:
		// Keep til we are unstuck.
		pItem->SetTimeout( -1 );
		return( true );

	case LAYER_HORSE:
		// Give my horse a tick. (It is still in the game !)
		{
			CChar * pHorse = pItem->m_itFigurine.m_UID.CharFind();
			if ( pHorse == NULL ) 
				return( false );
			pItem->SetTimeout( 10 * TICK_PER_SEC );
			return( pHorse->OnTick());
		}

	case LAYER_FLAG_Murders:
		// decay the murder count.
		DEBUG_CHECK( m_pPlayer );
		if ( ! m_pPlayer || m_pPlayer->m_Murders <= 1  )
			return( false );
		m_pPlayer->m_Murders--;
		pItem->SetTimeout( g_Serv.m_iMurderDecayTime );	// update it's decay time.
		return( true );
	}

	if ( pItem->m_type == ITEM_SPELL && 
		pItem->m_itSpell.m_spell == SPELL_Poison )
	{
		// Both potions and poison spells use this.
		// m_itSpell.m_skilllevel = strength of the poison ! 0-1000

		int iLevel = pItem->m_itSpell.m_skilllevel;
		if ( ! pItem->m_itSpell.m_charges || iLevel < 50 )
			return( false );

		// The poison in your body is having an effect.

		if ( iLevel < 200 )	// Lesser
			iLevel = 0;
		else if ( iLevel < 400 ) // Normal
			iLevel = 1;
		else if ( iLevel < 800 ) // Greater
			iLevel = 2;
		else	// Deadly.
			iLevel = 3;

		static const TCHAR * Poison_Message[] =
		{
			"sickly",
			"very ill",
			"extremely sick",
			"deathly sick",
		};

		static const int iPoisonMax[] = { 2, 4, 6, 8 };

		CGString sMsg;
		sMsg.Format( "looks %s", Poison_Message[iLevel] );
		Emote( sMsg, m_pClient );
		SysMessagef( "You feel %s", Poison_Message[iLevel] );

		int iDmg = IMULDIV( Stat_Get(STAT_STR), iLevel * 2, 100 );
		OnTakeDamage( max( iPoisonMax[iLevel], iDmg ), NULL, DAMAGE_POISON | DAMAGE_GENERAL );

		pItem->m_itSpell.m_skilllevel -= 50;	// gets weaker too.
		pItem->m_itSpell.m_charges--;

		// We will have this effect again.
		pItem->SetTimeout((5+GetRandVal(4))*TICK_PER_SEC);
		return true;
	}

	return( false );
}

void CChar::SetPoisonCure( int iSkill, bool fExtra )
{
	// Leave the anitdote in your body for a while.
	// iSkill = 0-1000

	CItem * pPoison = LayerFind( LAYER_FLAG_Poison );
	if ( pPoison != NULL )
	{
		// Is it successful ???
		pPoison->Delete();
	}
	if ( fExtra )
	{
		pPoison = LayerFind( LAYER_FLAG_Hallucination );
		if ( pPoison != NULL )
		{
			// Is it successful ???
			pPoison->Delete();
		}
	}
	ClearStat( STATF_Poisoned );
	Update();
}

void CChar::SetPoison( int iSkill, CChar * pCharSrc )
{
	// iSkill = 0-1000
	// Physical attack of poisoning.
	if ( IsStat( STATF_Poisoned | STATF_Conjured ))
		return;
	SysMessage( "You have been poisoned!" );

	// Might be a physical vs. Magical attack.
	CItem * pPoison = Spell_Effect_Create( SPELL_Poison, LAYER_FLAG_Poison, iSkill, 2*60*TICK_PER_SEC, pCharSrc );
	pPoison->m_itSpell.m_charges = iSkill/50;	// how long to last.
	UpdateStatsFlag();
}

void CChar::Wake()
{
	if ( ! IsStat( STATF_Sleeping ))
		return;
	CItemCorpse * pCorpse = FindMyCorpse();
	if ( pCorpse != NULL )
	{
		RaiseCorpse(pCorpse);
		ClearStat( STATF_Sleeping );
		Update();	// update light levels etc.
	}
	else
	{
		m_StatHealth = 0;	// Death
	}
}

void CChar::SleepStart( bool fFrontFall )
{
	if ( IsStat( STATF_DEAD | STATF_Sleeping | STATF_Polymorph ))
		return;

	SetStat( STATF_Sleeping );
	if ( ! MakeCorpse( fFrontFall ))
	{
		SysMessage( "Can't sleep here" );
		return;
	}

	SetID( m_prev_id );
	ClearStat( STATF_Hidden );

	Update();
}

CItemCorpse * CChar::FindMyCorpse() const
{
	// If they are standing on there own corpse then res the corpse !
	CWorldSearch Area( GetTopPoint(), 1 );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( pItem->GetID() != ITEMID_CORPSE )
			continue;

		CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pItem);
		ASSERT(pCorpse);

		// skip "Body of " but must have same name
		if ( strcmp( GetName(), pCorpse->GetName() + 8 ))
			continue;
		// not morphed type.
		if ( pCorpse->m_itCorpse.m_BaseID != m_prev_id )
			break;
		return( pCorpse );
	}
	return( NULL );
}

bool CChar::MakeCorpse( bool fFrontFall )
{
	// some creatures (Elems) have no corpses.
	// IsStat( STATF_DEAD ) might NOT be set. (sleeping)

	bool fLoot = true;

	// Don't create a corpse or loot if I was an NPC killed by a guard.
	CChar * pKiller = m_Act_Targ.CharFind();
	if ( ! m_pPlayer && 
		IsStat( STATF_DEAD ) &&
		pKiller && 
		pKiller->m_pNPC && 
		( pKiller->m_pNPC->m_Brain == NPCBRAIN_GUARD ))
	{
		SetStat( STATF_Conjured );
		fLoot = false;
	}

	CItemCorpse * pCorpse = NULL;

	if ( ! IsStat( STATF_Conjured ) &&
		GetDispID() != CREID_WATER_ELEM &&
		GetDispID() != CREID_AIR_ELEM &&
		GetDispID() != CREID_FIRE_ELEM &&
		GetDispID() != CREID_VORTEX &&
		GetDispID() != CREID_BLADES )
	{
		Horse_UnMount(); // If i'm conjured then my horse goes with me.

		CItem* pItemCorpse = CItem::CreateScript( ITEMID_CORPSE );
		pCorpse = dynamic_cast <CItemCorpse *>(pItemCorpse);
		if ( pCorpse == NULL )
		{
			DEBUG_CHECK(pCorpse);
			pItemCorpse->Delete();
			goto nocorpse;
		}

		CGString sName;
		sName.Format( "Body of %s", GetName());
		pCorpse->SetName( sName );
		pCorpse->SetColor( GetColor());
		pCorpse->SetCorpseType( GetDispID());
		pCorpse->m_itCorpse.m_BaseID = m_prev_id;	// id the corpse type here !
		pCorpse->m_itCorpse.m_facing_dir = m_face_dir;
		pCorpse->m_Attr |= ATTR_INVIS;	// Don't display til ready.

		if ( IsStat( STATF_DEAD ))
		{
			pCorpse->m_itCorpse.m_tDeathTime = g_World.GetTime();	// death time.
			pCorpse->m_itCorpse.m_uidKiller = m_Act_Targ;
			pCorpse->SetDecayTime( (m_pPlayer) ?
				g_Serv.m_iDecay_CorpsePlayer : g_Serv.m_iDecay_CorpseNPC );
		}
		else	// Sleeping
		{
			pCorpse->m_itCorpse.m_tDeathTime = 0;	// Not dead.
			pCorpse->m_itCorpse.m_uidKiller = GetUID();
		}

		if ( IsRespawned())	// not being deleted.
		{
			pCorpse->m_uidLink = GetUID();
		}
	}
	else
	{
nocorpse:
		// Some creatures can never sleep.
		if ( ! IsStat( STATF_DEAD ))
			return( false );
		if ( IsHuman())
			return( true );	// conjured humans just disapear.
	}

	// can fall forward.
	DIR_TYPE dir = m_face_dir;
	if ( fFrontFall )
	{
		dir = (DIR_TYPE) ( dir | 0x80 );
		if ( pCorpse ) pCorpse->m_itCorpse.m_facing_dir = dir;
	}

	// Death anim. default is to fall backwards. lie face up.
	CCommand cmd;
	cmd.CharDeath.m_Cmd = XCMD_CharDeath;
	cmd.CharDeath.m_UID = GetUID();	// 1-4
	cmd.CharDeath.m_UIDCorpse = ( pCorpse == NULL ) ? 0 : (DWORD) pCorpse->GetUID(); // 9-12
	cmd.CharDeath.m_DeathFlight = IsStat( STATF_Fly ) ? 1 : 0; 	// Die in flight ?
	cmd.CharDeath.m_Death2Anim = ( dir & 0x80 ) ? 1 : 0; // Fore/Back Death ?

	UpdateCanSee( &cmd, sizeof( cmd.CharDeath ), m_pClient );

	// Move non-newbie contents of the pack to corpse. (before it is displayed)
	if ( fLoot )
	{
		DropAll( pCorpse );
	}
	if ( pCorpse != NULL )
	{
		pCorpse->m_Attr &= ~ATTR_INVIS;	// make visible.
		pCorpse->MoveTo( GetTopPoint());
	}

	return( true );
}

bool CChar::RaiseCorpse( CItemCorpse * pCorpse )
{
	// We are creating a char from the current char and the corpse.
	// Move the items from the corpse back onto us.
	ASSERT(pCorpse);
	if ( pCorpse->GetCount())
	{
		CItemContainer * pPack = GetPackSafe();

		CItem* pItemNext;
		for ( CItem * pItem = pCorpse->GetContentHead(); pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( pItem->m_type == ITEM_HAIR ||
				pItem->m_type == ITEM_BEARD ||
				pItem->IsAttr( ATTR_MOVE_NEVER ))
				continue;	// Hair on corpse was copied!
			// Redress if equipped.
			if ( pItem->GetContainedLayer())
				ItemEquip( pItem );	// Equip the item.
			else
				pPack->ContentAdd( pItem );	// Toss into pack.
		}

		// Any items left just dump on the ground.
		pCorpse->ContentsDump( GetTopPoint());
	}

	if ( pCorpse->IsTopLevel())
	{
		// I should move to where my corpse is just in case.
		MoveTo( pCorpse->GetTopPoint());
	}

	// Corpse is now gone. 	// 0x80 = on face.
	Update();
	UpdateDir( (DIR_TYPE)( pCorpse->m_itCorpse.m_facing_dir &~ 0x80 ));
	UpdateAnimate( ( pCorpse->m_itCorpse.m_facing_dir & 0x80 ) ? ANIM_DIE_FORWARD : ANIM_DIE_BACK, true, true, 2 );

	pCorpse->Delete();

	return( true );
}

bool CChar::Death()
{
	// RETURN: false = delete

	if ( IsStat( STATF_DEAD|STATF_INVUL )) 
		return true;

	if ( OnTrigger( CTRIG_Death, this ))
		return( true );

	if ( IsClient())	// Prevents crashing ?
	{
		GetClient()->addPause();
	}

	// I am dead and we need to give credit for the kill to my attacker(s).
	TCHAR * pszKillStr = GetTempStr();
	int iKillStrLen = sprintf( pszKillStr, "%c'%s' was killed by ", 
		(m_pNPC)?'N':'P', GetName() );
	int iKillers = 0;

	// Look through my memories of who i was fighting. (make sure they knew they where fighting me)
	CItem* pItemMemNext;
	CItem* pItemMem=GetContentHead();
	for ( ; pItemMem!=NULL; pItemMem=pItemMemNext )
	{
		pItemMemNext = pItemMem->GetNext();
		if ( ! pItemMem->IsMemoryTypes( MEMORY_HARMEDBY | MEMORY_SAWCRIME )) // i was harmed in some way but this person.
			continue;

		CChar * pKiller = pItemMem->m_uidLink.CharFind();
		if ( pKiller != NULL )
		{
			// If I have an owner, they get flagged (ex NPCBRAIN_BESERK)
			if ( pKiller->m_pNPC && pKiller->m_pNPC->m_Brain == NPCBRAIN_BESERK )
			{
				CChar * pOwner = pKiller->NPC_GetOwner();
				if ( pOwner != NULL )
				{
					// DEBUG_ERR(("My owner: %x\n", pOwner->GetUID()));
					pOwner->Noto_Kill( this, pItemMem->IsMemoryTypes( MEMORY_SAWCRIME ), true );
				}
			}

			pKiller->Noto_Kill( this, pItemMem->IsMemoryTypes( MEMORY_SAWCRIME ));

			iKillStrLen += sprintf( pszKillStr+iKillStrLen, "%s%c'%s'", 
				iKillers ? ", " : "",
				(pKiller->m_pNPC)?'N':'P', pKiller->GetName() );
			iKillers ++;
		}
		pItemMem->Delete();	// ??? or just clear the FIGHT ?
	}

	// record the kill event for posterity.

	iKillStrLen += sprintf( pszKillStr+iKillStrLen, ( iKillers ) ? ".\n" : "accident.\n" );
	g_Log.Event( ((m_pNPC) ? LOGL_TRACE :LOGL_EVENT)|LOGM_KILLS, pszKillStr );

	SoundChar( CRESND_DIE );
	Reveal();
	Spell_Dispel(100);		// Get rid of all spell effects.

	// Only players should loose fame upon death.
	if ( m_pPlayer )
	{
		Noto_Fame( -Stat_Get(STAT_Fame)/10 );
	}

	// create the corpse item.
	SetStat( STATF_DEAD );
	ClearStat( STATF_Stone | STATF_Freeze | STATF_Hidden | STATF_Sleeping );

	MakeCorpse( GetRandVal(2));

	m_StatHealth = 0;	// on my way to death.

	if ( m_pPlayer )
	{
		SetColor( COLOR_DEFAULT );	// Get all pale.
		SetID( ( CCharBase::IsFemaleID( m_prev_id )) ? CREID_GHOSTWOMAN : CREID_GHOSTMAN );
		Update();		// show everyone I am now a ghost.
		LayerAdd( CItem::CreateScript( ITEMID_DEATHSHROUD ));

		// Manifest the ghost War mode for ghosts.
		if ( ! IsStat( STATF_War ))
		{
			SetStat( STATF_Insubstantial );
		}
	}

	if ( IsClient())
	{
		// Put up the death menu.
		CCommand cmd;
		cmd.DeathMenu.m_Cmd = XCMD_DeathMenu;
		cmd.DeathMenu.m_shift = 0;
		m_pClient->xSendPkt( &cmd, sizeof( cmd.DeathMenu ));

		// We are now in "death menu mode"
		m_pClient->SetTargMode(TARGMODE_DEATH);
		m_pClient->m_Targ_p = GetTopPoint();	// Insta res takes us back to where we died.
	}
	else if ( IsRespawned())
	{
		NPC_ClearOwners();	// Forgot who owns me.
		SetDisconnected();	// Respawn the NPC later
	}
	else
	{
		return false;	// must delete this now !
	}

	return( true );
}

bool CChar::OnFreezeCheck()
{
	// Check why why are held in place.
	// Can we break free ?
	// RETURN: true = held in place.
	DEBUG_CHECK( IsStat( STATF_Stone | STATF_Freeze ));

	CItem * pFlag = LayerFind( LAYER_FLAG_Stuck );
	if ( pFlag == NULL )	// stuck for some other reason i guess.
	{
		SysMessage(( IsStat( STATF_Sleeping )) ?
			"You are unconscious and can't move." :
			"You are frozen and can not move.");
	}
	else
	{
		if ( pFlag->m_itEqStuck.m_p != GetTopPoint())
		{
			// Maybe we teleported away ?
killit:
			pFlag->Delete();
			return( false );
		}

		CItem * pWeb = pFlag->m_uidLink.ItemFind();
		if ( pWeb == NULL || ! pWeb->IsTopLevel())
			goto killit;
		if ( pWeb->GetTopPoint() != pFlag->m_itEqStuck.m_p )
			goto killit;
		if ( ! pFlag->IsTimerSet())
		{
			return( Use_Item_Web( pWeb ));
		}

		SysMessage( "You are caught in the web.");
	}

	return( ! IsPriv( PRIV_GM ));
}

void CChar::Flip( const TCHAR * pCmd )
{
	m_face_dir = GetDirTurn( m_face_dir, 1 );
	UpdateMove(GetTopPoint());
}

CRegionBase * CChar::CanMoveWalkTo( CPointBase & ptDst, bool fCheckChars )
{
	// For both players and NPC's
	// Walk towards this point as best we can.
	// Affect stamina as if we WILL move !
	// RETURN:
	//  ptDst.m_z = the new z
	//  NULL = failed to walk here.

	if ( IsStat( STATF_Freeze | STATF_Stone ) && OnFreezeCheck())
	{
		// NPC's would call here.
		return( NULL );	// can't move.
	}

	if ( m_StatStam <= 0 && ! IsStat( STATF_DEAD ))
	{
		SysMessage( "You are too fatigued to move." );
		return( NULL );
	}

	int iWeightLoadPercent = GetWeightLoadPercent( GetTotalWeight());
	if ( iWeightLoadPercent > 300 )
	{
		SysMessage( "You are too overloaded to move." );
		return( NULL );
	}

	// ok to go here ? physical blocking objects ?
	WORD wBlockFlags;
	CRegionBase * pArea = CheckValidMove( ptDst, &wBlockFlags );
	if ( pArea == NULL )
		return( NULL );

	if ( m_pNPC )
	{
		// Does the NPC want to walk here ?
		if ( ! NPC_CheckWalkHere( ptDst, pArea ))
			return( NULL );
	}

	// Bump into other creatures ?
	if ( ! IsStat( STATF_DEAD | STATF_Sleeping ) && fCheckChars )
	{
		CWorldSearch AreaChars( ptDst );
		AreaChars.SetInertView( true );	// show logged out chars.
		while ( 1 )
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL ) break;
			if ( pChar == this ) continue;
			if ( pChar->GetTopZ() != ptDst.m_z ) continue;

			if ( m_pNPC ) return( NULL ); // NPc's can't bump thru.

			if ( pChar->IsStat( STATF_DEAD | STATF_Insubstantial ) ||
				pChar->IsDisconnected())
			{
				if ( CanDisturb(pChar) && pChar->IsStat(STATF_SpiritSpeak))
				{
					SysMessage( "You feel a tingling sensation" );
				}
				continue;
			}

			// How much stamina to push past ?
			int iStam = GetLog2( pChar->Stat_Get(STAT_STR)) + 1;

			CGString sMsg;
			if ( pChar->IsStat( STATF_Invisible ))
			{
				sMsg = "You push past something invisible";
				continue;
			}
			else if ( pChar->IsStat( STATF_Hidden ))
			{
				// reveal hidden people ?
				sMsg.Format( "You stumble apon %s hidden.", pChar->GetName());
				pChar->Reveal(STATF_Hidden);
			}
			else if ( pChar->IsStat( STATF_Sleeping ))
			{
				sMsg.Format( "You step on the body of %s.", pChar->GetName());
				// ??? damage.
			}
			else if ( m_StatStam < iStam + 1 )
			{
				sMsg.Format( "You are not strong enough to push %s out of the way.", pChar->GetName());
				if ( ! IsPriv( PRIV_GM ))
				{
					SysMessage( sMsg );
					return NULL;
				}
			}
			else
			{
				// ??? What is the true amount of stamina lost ?
				sMsg.Format( "You shove %s out of the way.", pChar->GetName());
			}

			SysMessage( sMsg );
			if ( IsPriv( PRIV_GM ))
			{
				// client deducts stam for us. make sure this is fixed.
				UpdateStats( STAT_DEX, 10 );
			}
			else
			{
				UpdateStats( STAT_DEX, -iStam );
			}

			pChar->OnTrigger( CTRIG_PersonalSpace, this );
			break;
		}
	}

	// decrease stamina if running or overloaded.

	if ( ! IsPriv(PRIV_GM))
	{
		// We are overloaded. reduce our stamina faster.
		// Running acts like an increased load.
		if ( IsStat( STATF_Fly ))
			iWeightLoadPercent += g_Serv.m_iStamRunningPenalty;
		int iChanceForStamLoss = GW_GetSCurve( iWeightLoadPercent - g_Serv.m_iStaminaLossAtWeight, 10 );
		int iRoll = GetRandVal(1000);
		if ( iRoll <= iChanceForStamLoss )
		{
			// Lower avail stamina.
			UpdateStats( STAT_DEX, -1 );
		}
	}

	ModStat( STATF_InDoors, wBlockFlags & CAN_I_ROOF );
	return( pArea );
}

void CChar::CheckRevealOnMove()
{
	// Are we going to reveal ourselves by moving ?
	if ( IsStat( STATF_Invisible | STATF_Hidden | STATF_Sleeping ))
	{
		// Wake up if sleeping and this is possible.
		if ( IsStat( STATF_Fly ) ||
			! IsStat( STATF_Hidden ) ||
			IsStat( STATF_Sleeping ) ||
			! Skill_UseQuick( SKILL_Stealth, GetRandVal( 105 )))
		{
			// check hiding again ? unless running ?
			Reveal();
		}
	}
}

bool CChar::CheckLocation( bool fStanding )
{
	// We are at this location
	// what will happen ?
	// RETURN: true = we teleported.

	if ( ! fStanding )
	{
		// If we moved and are wielding are in combat and are using a
		// crossbow/bow kind of weapon, then reset the weaponswingtimer.
		if ( GetActiveSkill() == SKILL_ARCHERY )
		{
			SetWeaponSwingTimer();
		}
	}

	CWorldSearch AreaItems( GetTopPoint());
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL )
			break;

		int zdiff = pItem->GetTopZ() - GetTopZ();
		if ( abs(zdiff) > 3 ) 
			continue;

		if ( pItem->OnTrigger( ITRIG_STEP, this, fStanding ))
			continue;

		switch ( pItem->m_type )
		{
		case ITEM_SHRINE:
			// Resurrect the ghost
			if ( fStanding ) 
				continue;
			Spell_Resurrection( 0 );
			return( false );
		case ITEM_WEB:
			if ( fStanding ) 
				continue;
			// Caught in a web.
			if ( Use_Item_Web( pItem ))
				return( true );
			continue;
		case ITEM_DREAM_GATE:
			// Move to another server by name
			if ( fStanding )
				continue;
			if ( ! IsClient())
				continue;
			m_pClient->UseInterWorldGate( pItem );
			return( true );
		case ITEM_FIRE:
			// ??? fire object hurts us ?
			// standing on burning kindling shouldn't hurt us
			if ( pItem->GetDispID() >= ITEMID_CAMPFIRE && pItem->GetDispID() <= ITEMID_EMBERS )
				return ( false );
			OnTakeDamage( 1 + GetRandVal(4), NULL, DAMAGE_FIRE | DAMAGE_GENERAL );
			Sound( 0x15f ); // ??? Fire noise.
			return( false );
		case ITEM_SPELL:
			OnSpellEffect( (SPELL_TYPE) pItem->m_itSpell.m_spell, NULL, 50 );
			Sound( g_Serv.m_SpellDefs[ (SPELL_TYPE) pItem->m_itSpell.m_spell]->m_sound  );
			return( false );
		case ITEM_TRAP:
		case ITEM_TRAP_ACTIVE:
		// case ITEM_TRAP_INACTIVE: // reactive it?
			OnTakeDamage( pItem->Use_Trap(), NULL, DAMAGE_HIT | DAMAGE_GENERAL );
			return( false );
		case ITEM_SWITCH:
			if ( pItem->m_itSwitch.m_fStep )
			{
				Use_Item( pItem );
			}
			return( false );
		case ITEM_MOONGATE:
		case ITEM_TELEPAD:
			if ( fStanding )
				continue;
			Use_MoonGate( pItem );
			return( true );
		case ITEM_SHIP_PLANK:
			// a plank is a teleporter off the ship.
			if ( ! fStanding && ! IsStat( STATF_Fly ))
			{
				// Find some place to go. (in direction of plank)
				if ( MoveToValidSpot( m_face_dir, UO_MAP_VIEW_SIZE, 1 ))
				{
					pItem->SetTimeout( 5*TICK_PER_SEC );	// autoclose it behind us.
					return( true );
				}
			}
			continue;

		case ITEM_ADVANCE_GATE:
			// Upgrade the player to the skills of the selected NPC script.
			if ( fStanding ) 
				continue;
			Use_AdvanceGate( pItem );
			break;
		}
	}

	if ( fStanding )
		return( false );

	// Check the map teleporters in this CSector. (if any)
	CPointMap pt = GetTopPoint();
	CSector * pSector = pt.GetSector();
	ASSERT(pSector);
	const CTeleport * pTel = pSector->GetTeleport( pt );
	if ( pTel )
	{
		Spell_Teleport( pTel->m_Dst, true, false, ITEMID_NOTHING );
		return( true );
	}
	return( false );
}

bool CChar::MoveToRegion( CRegionWorld * pNewArea, bool fAllowReject )
{
	// Moving to a new region. or logging out (not in any region)
	// pNewArea == NULL = we are logging out.
	// RETURN:
	//  false = do not allow in this area.

#ifdef _DEBUG
	if ( pNewArea )
	{
		DEBUG_CHECK( pNewArea->IsValid());
	}
	if ( m_pArea )
	{
		DEBUG_CHECK( m_pArea->IsValid());
	}
#endif

	if ( m_pArea == pNewArea )
		return true;

	if ( fAllowReject && IsPriv( PRIV_GM ))
	{
		fAllowReject = false;
	}

	// Leaving region trigger. (may not be allowed to leave ?)
	if ( m_pArea )
	{
		if ( m_pArea->OnRegionTrigger( this, false ) && pNewArea && fAllowReject )
			return false;
	}

	if ( IsClient() && pNewArea )
	{
		if ( pNewArea->IsFlag( REGION_FLAG_ANNOUNCE ) &&
			! pNewArea->IsInside( GetTopPoint()))
		{
			SysMessagef( ( pNewArea->m_sGuardAnnounce.IsEmpty()) ?
				"You have entered %s" :
				((const TCHAR*) pNewArea->m_sGuardAnnounce ),
				pNewArea->GetName());
		}

		// Is it guarded ?
		else if ( m_pArea != NULL && ! IsStat( STATF_DEAD ))
		{
			if ( pNewArea->IsGuarded() != m_pArea->IsGuarded())
			{
				SysMessagef( ( pNewArea->IsGuarded()) ?
					"You are now under the protection of %s guards" :
					"You have left the protection of %s guards",
					(const TCHAR *) pNewArea->m_sGuardAnnounce );
			}
			if ( pNewArea->IsFlag(REGION_FLAG_SAFE) != m_pArea->IsFlag(REGION_FLAG_SAFE))
			{
				SysMessage( ( pNewArea->IsFlag(REGION_FLAG_SAFE)) ?
					"You have a feeling of complete safety" :
					"You lose your feeling of safety" );
			}
		}

		// Set the new music ?
		if ( pNewArea->m_TrigPeriodic )
		{
			pNewArea->OnRegionTrigger( this, true, true );
		}
	}

	// Entering region trigger.
	if ( pNewArea )
	{
		if ( pNewArea->OnRegionTrigger( this, true ) && m_pArea && fAllowReject )
			return false;
	}

	m_pArea = pNewArea;
	return( true );
}

bool CChar::MoveTo( CPointMap pt )
{
	// This could be us just taking a step or being teleported.
	// Low level: DOES NOT UPDATE DISPLAYS or container flags. (may be offline)
	// This does not check for gravity.
	//

	if ( ! pt.IsValid())
		return false;

	if ( m_pPlayer && ! IsClient())	// moving a logged out client !
	{
		// We cannot put this char in non-disconnect state.
		SetDisconnected();
		pt.GetSector()->m_Chars_Disconnect.InsertAfter( this );
		SetUnkPoint( pt );
		return true;
	}

	// Did we step into a new region ?
	if ( ! MoveToRegion( dynamic_cast <CRegionWorld *>( pt.GetRegion( REGION_TYPE_MULTI|REGION_TYPE_AREA )), true ))
		return false;
	DEBUG_CHECK( m_pArea );

	pt.GetSector()->MoveToSector( this );

	// Move this item to it's point in the world. (ground or top level)
	SetTopPoint( pt );

	ASSERT( ! CObjBase::IsWeird());
	return true;
}

bool CChar::MoveToValidSpot( DIR_TYPE dir, int iDist, int iDistStart )
{
	// Move from here to a valid spot.
	// ASSUME "here" is not a valid spot. (even if it really is)

	CPointMap pt = GetTopPoint();
	pt.Move( dir, iDistStart );

	for ( int i=0; i<iDist; i++ )
	{
		WORD wCan = GetMoveBlockFlags();
		WORD wBlockFlags = wCan;
		signed char z = g_World.GetHeight( pt, wBlockFlags, pt.GetRegion( REGION_TYPE_MULTI ));
		if ( ! ( wBlockFlags &~ wCan ))
		{
			// we can go here. (maybe)
			if ( Spell_Teleport( pt, true, true, ITEMID_NOTHING ))
				return( true );
		}
		pt.Move( dir );
	}
	return( false );
}

const TCHAR * CChar::sm_szTrigName[CTRIG_QTY] =	// static
{
	"@HearGreeting",	// NPC i have been spoken to for the first time. (no memory of previous hearing)
	"@HearUnknown",		// NPC I heard something i don't understand.
	"@SpellCast",		// Char is casting a spell.
	"@SpellEffect",		// A spell just hit me.
	"@SeeWantItem",		// NPC i see something good.
	"@PersonalSpace",	// i just got stepped on.
	"@ReceiveItem",		// I was just handed an item (Not yet checked if i want it)
	"@AcceptItem",		// NPC i've been given an item i like (according to DESIRES)
	"@RefuseItem",		// NPC i've been given an item i don't want.
	"@SeeNewPlayer",	// NPC i see u for the first time. (in 30 minutes) (check memory time)
	"@FightSwing",		// randomly choose to speak while fighting.
	"@FearOfDeath",		// I'm not healthy.
	"@GetHit",			// I just got hit.
	"@Hit",				// I just hit someone. (TARG)
	"@Death",			// I just got killed.
};

bool CChar::OnTrigger( const TCHAR * pszTrigName, CTextConsole * pSrc, int iArg )
{
	// Attach some trigger to the cchar. (PC or NPC)
	// RETURN: true = block further action.

	// 
	// Go thru the event blocks for the NPC/PC to do events.

	for ( int i=0; i<m_Events.GetCount(); i++ )
	{
		CScriptLock s;
		if ( ! m_Events[i]->OpenFrag( s ))
			continue;
		if ( CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, iArg ))
			return true;
	}

	if ( m_pNPC )
	{
		for ( i=0; i<m_pDef->m_Events.GetCount(); i++ )
		{
			CScriptLock s;
			if ( ! m_pDef->m_Events[i]->OpenFrag( s ))
				continue;
			if ( CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, iArg ))
				return true;
		}
	}

	return( false );
}

void CChar::OnFoodTick()
{
    if ( IsStat( STATF_Conjured )) 
		return;	// No need for food.

    if ( IsStat( STATF_Pet ))	// This may be money instead of food 
    {
    	if ( ! NPC_CheckHirelingStatus())
    		return;
    }

	if ( m_pDef->m_MaxFood == 0 )	// No need for food.
  		return;
    if ( m_food > 0 )
	{
		m_food --;
	}
	else
	{
		m_food ++;
	}

	int  nFoodLevel = GetFoodLevelPercent();
	if ( nFoodLevel < 40 )	// start looking for food at 40%
    {
    	// Tell everyone we look hungry.

    	SysMessagef( "You are %s", GetFoodLevelMessage( false, false ));

	   	if ( m_pNPC )
		{
  			NPC_OnFoodTick( nFoodLevel );
		}
    }
}

int CChar::sm_Stat_Val_Regen[] =	// static (NOT CONST)
{
	6*TICK_PER_SEC, // Seconds to heal ONE hp (base before stam and food adjust)
	5*TICK_PER_SEC, // Seconds to heal ONE mn
	3*TICK_PER_SEC, // Seconds to heal ONE stm
	30*60*TICK_PER_SEC,	// Food regen (1 time per 30 minutes)
};

bool CChar::OnTick()
{
	// Assume this is only called 1 time per sec.
	// Get a timer tick when our timer expires.
	// RETURN: false = delete this.

	int iTimeDiff = g_World.GetTime() - m_time_last_regen;
	if ( iTimeDiff == 0 )
		return true;	// ??? I have no idea why this happens
	DEBUG_CHECK( iTimeDiff > 0 );

	if ( IsStat( STATF_DEAD ))
		return true;
	if ( m_StatHealth <= 0 )	// We can only die on our own tick.
	{
		return Death();
	}

	if ( iTimeDiff >= TICK_PER_SEC )	// don't bother with < 1 sec times.
	{
		// decay equipped items (spells)
		CItem* pItemNext;
		CItem* pItem=GetContentHead();
		for ( ; pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( ! pItem->IsTimerSet() )
				continue;
			if ( ! pItem->IsTimerExpired() )
				continue;
			if ( OnEquipTick( pItem ))
				continue;
			if ( ! pItem->OnTick())
			{
				pItem->Delete();
			}
		}

		// NOTE: Summon flags can kill our hp here. check again.
		if ( m_StatHealth <= 0 )	// We can only die on our own tick.
		{
			return Death();
		}

		for ( int i=0; i<COUNTOF(m_StatVal); i++ )
		{
			m_StatVal[i].m_regen += iTimeDiff;
			int iRate = sm_Stat_Val_Regen[i];
			if ( i == STAT_STR )	// HitPoints regen rate is related to food and stam.
			{
				if ( m_pPlayer )
				{
					if ( ! m_food )
						continue; // iRate += iRate/2;	// much slower with no food.
					if ( IsStat(STATF_Fly))
						continue;
				}

				// Fast metabolism bonus ?
				iRate += iRate / ( 2 * ( 1 + m_StatStam / 32 ));
			}

			if ( m_StatVal[i].m_regen < iRate )
				continue;

			m_StatVal[i].m_regen = 0;
			if ( i==3 )
			{
				OnFoodTick();
			}
			else if ( m_StatVal[i].m_val != Stat_Get((STAT_TYPE)i))
			{
				UpdateStats( (STAT_TYPE) i, 1 );
			}
		}
	}

	if ( IsDisconnected())	// mounted horses can still get a tick.
	{
		m_time_last_regen = g_World.GetTime();
		return( true );
	}

	DEBUG_CHECK( IsTopLevel());	// not deleted or off line.

	if ( IsTimerExpired())
	{
		// My turn to do some action.

		SKILL_TYPE iSkillPrv = GetActiveSkill();
		if ( ! Skill_Done())
		{
			Skill_Fail( false );
		}
		if ( m_pNPC )	// What to do next ?
		{
			g_Serv.m_Profile.Start( PROFILE_NPC_AI );
			NPC_OnTickAction();
			g_Serv.m_Profile.Start( PROFILE_CHARS );
		}
		if ( IsTimerExpired())	// Was not reset ?
		{
			// default next tick
			SetTimeout( TICK_PER_SEC/3 + GetRandVal( (150-Stat_Get(STAT_DEX))/2 ) * TICK_PER_SEC / 10 );
		}
	}
	else
	{
		// Hit my current target. (i'm ready)
		switch ( GetActiveSkill())
		{
		case SKILL_ARCHERY:
		case SKILL_FENCING:
		case SKILL_MACEFIGHTING:
		case SKILL_SWORDSMANSHIP:
		case SKILL_WRESTLING:
			if ( m_atFight.m_War_Swing_State == WAR_SWING_READY )
			{
				HitTry();
			}
		}
	}

	if ( iTimeDiff >= TICK_PER_SEC )
	{
		// Check location periodically for standing in fire fields, traps, etc.
		CheckLocation( true );
		m_time_last_regen = g_World.GetTime();
	}

	return( true );
}

