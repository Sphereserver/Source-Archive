//
// CItem.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

LPCTSTR const CItem::sm_szTrigName[ITRIG_QTY+1] =	// static
{
	_TEXT("@AAAUNUSED"),
	"@Click",
	"@Create",
	"@DAMAGE",				// I have been damaged in some way
	"@DCLICK",				// I have been dclicked.
	"@Destroy",
	"@DropOn_Char",			// I have been dropped on this char
	"@DropOn_Ground",		// I have been dropped on the ground here
	"@DropOn_Item",			// I have been dropped on this item
	"@DropOn_Self",			// An item has been dropped here
	"@EQUIP",		// I have been unequipped
    "@EQUIPTEST",
	"@PICKUP_GROUND",	// I was picked up off the ground.
	"@PICKUP_PACK",	// picked up from inside some container.
	"@PICKUP_SELF", // picked up from here
	"@SpellEffect",		// cast some spell on me.
	"@STACKON",			// Something was stacked on top of me.
	"@STEP",			// I have been walked on.
	"@TARGON_CANCEL",
	"@TARGON_CHAR",
	"@TARGON_GROUND",
	"@TARGON_ITEM",	// I am being combined with an item
	"@TIMER",		// My timer has expired.
	"@ToolTip",
	"@UNEQUIP",		// i have been unequipped (or try to unequip)
	NULL,
};

/////////////////////////////////////////////////////////////////
// -CItem

CItem::CItem( ITEMID_TYPE id, CItemBase * pItemDef ) : CObjBase( true )
{
	ASSERT( pItemDef );

	g_Serv.StatInc(SERV_STAT_ITEMS);
	m_Attr = 0;
	m_amount = 1;

	m_itNormal.m_more1 = 0;
	m_itNormal.m_more2 = 0;
	m_itNormal.m_morep.ZeroPoint();

	SetBase( pItemDef );
	SetDispID( id );

	g_World.m_uidLastNewItem = GetUID();	// for script access.
	ASSERT( IsDisconnected());
}

CItem::~CItem()
{
	DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	if ( ! g_Serv.IsLoading())
	switch ( m_type )
	{
	case IT_SPAWN_CHAR:
		Spawn_KillChildren();
		break;
	case IT_FIGURINE:
	case IT_EQ_HORSE:
		{	// remove the ridden or linked char.
			CChar * pHorse = m_itFigurine.m_UID.CharFind();
			if ( pHorse && pHorse->IsDisconnected() && ! pHorse->m_pPlayer )
			{
				DEBUG_CHECK( pHorse->IsStatFlag( STATF_Ridden ));
				DEBUG_CHECK( pHorse->Skill_GetActive() == NPCACT_RIDDEN );
				pHorse->m_atRidden.m_FigurineUID.InitUID();
				pHorse->Delete();
				
			}
		}
		break;
	}
	g_Serv.StatDec(SERV_STAT_ITEMS);
}

CItem * CItem::CreateBase( ITEMID_TYPE id )	// static
{
	// All CItem creates come thru here.
	// NOTE: This is a free floating item.
	//  If not put on ground or in container after this = Memory leak !

	ITEMID_TYPE idErrorMsg = ITEMID_NOTHING;
	CItemBase * pItemDef = CItemBase::FindItemBase( id );
	if ( pItemDef == NULL )
	{
		idErrorMsg = id;
		id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, "DEFAULTITEM" );
		if ( id <= 0 )
		{
			id = ITEMID_GOLD_C1;
		}
		pItemDef = CItemBase::FindItemBase( id );
		ASSERT(pItemDef);
	}

	CItem * pItem;
	switch ( pItemDef->GetType() )
	{
	case IT_MAP:
	case IT_MAP_BLANK:
		pItem = new CItemMap( id, pItemDef );
		break;
	case IT_COMM_CRYSTAL:
		pItem = new CItemCommCrystal( id, pItemDef );
		break;
	case IT_GAME_BOARD:
	case IT_BBOARD:
	case IT_TRASH_CAN:
	case IT_CONTAINER_LOCKED:
	case IT_CONTAINER:
	case IT_EQ_TRADE_WINDOW:
	case IT_EQ_VENDOR_BOX:
	case IT_EQ_BANK_BOX:
	case IT_KEYRING:
	case IT_SHIP_HOLD_LOCK:
	case IT_SHIP_HOLD:
		pItem = new CItemContainer( id, pItemDef );
		break;
	case IT_CORPSE:
		pItem = new CItemCorpse( id, pItemDef );
		break;
	case IT_MESSAGE:
	case IT_BOOK:
	case IT_EQ_NPC_SCRIPT:
		// A message for a bboard or book text.
		pItem = new CItemMessage( id, pItemDef );
		break;
	case IT_STONE_GUILD:
	case IT_STONE_TOWN:
	case IT_STONE_ROOM:
		pItem = new CItemStone( id, pItemDef );
		break;
	case IT_MULTI:
	case IT_SHIP:
		pItem = new CItemMulti( id, pItemDef );
		break;
	case IT_EQ_MEMORY_OBJ:
		pItem = new CItemMemory( id, pItemDef );
		break;
	case IT_EQ_SCRIPT:	// pure script.with TAG(s) support
	case IT_SCRIPT:
		pItem = new CItemScript( id, pItemDef );
		break;
	case IT_DREAM_GATE:
		pItem = new CItemServerGate( id, pItemDef );
		break;

	default:
		if ( pItemDef->GetMakeValue(0))
			pItem = new CItemVendable( id, pItemDef );
		else
			pItem = new CItem( id, pItemDef );
		break;
	}

	ASSERT( pItem );
	if ( idErrorMsg && idErrorMsg != -1 )
	{
		DEBUG_ERR(( "CreateBase invalid item 0%x" DEBUG_CR, idErrorMsg ));
	}
	return( pItem );
}

CItem * CItem::CreateDupeItem( const CItem * pItem, CChar * pSrc, bool fSetNew )	// static
{
	// Dupe this item.
	if ( pItem == NULL )
		return( NULL );
	CItem * pItemNew = CreateBase( pItem->GetID());
	ASSERT(pItemNew);
	pItemNew->DupeCopy( pItem );

	if ( pSrc )
		pSrc->m_Act_Targ = pItemNew->GetUID();

	if ( fSetNew )
		g_World.m_uidNew = pItemNew->GetUID();

	return( pItemNew );
}

CItem * CItem::CreateScript( ITEMID_TYPE id, CChar * pSrc ) // static
{
	// Create item from the script id.

	CItem * pItem = CreateBase( id );
	ASSERT( pItem );

	// default stuff for the item.
	switch ( pItem->GetType())
	{
	case IT_CONTAINER_LOCKED:
		{
			// At this level it just means to add a key for it.
			CItemContainer * pCont = dynamic_cast <CItemContainer *> ( pItem );
			ASSERT(pCont);
			pCont->MakeKey();
		}
		break;
	}

	// call the ON=@Create trigger
	CItemBase * pItemDef = pItem->Item_GetDef();
	ASSERT( pItemDef );

	if ( pItemDef->HasTrigger( ITRIG_Create ))
	{
		CResourceLock s;
		if ( pItemDef->ResourceLock(s))
		{
			if ( pSrc )
				pItem->OnTriggerScript( s, sm_szTrigName[ITRIG_Create], pSrc );
			else
				pItem->OnTriggerScript( s, sm_szTrigName[ITRIG_Create], &g_Serv );
/*
			if ( pItem->OnTriggerFind( s, sm_szTrigName[ITRIG_Create] ))
			{
				pItem->CScriptObj::r_Load(s);
			}
*/
		}
	}

	return( pItem );
}

CItem * CItem::CreateHeader( TCHAR * pArg, CObjBase * pCont, bool fDupeCheck, CChar * pSrc )
{
	// Just read info on a single item carryed by a CChar.
	// ITEM=#id,#amount,R#chance

	RESOURCE_ID rid = g_Cfg.ResourceGetID( RES_ITEMDEF, (LPCTSTR&)pArg );
	if ( ! rid.IsValidUID())
		return( NULL );
	if ( rid.GetResType() != RES_ITEMDEF && rid.GetResType() != RES_TEMPLATE )
		return( NULL );
	if ( rid.GetResIndex() == 0 )
		return( NULL );

	int amount = 1;
	if ( Str_Parse( pArg, &pArg ))
	{
		if ( pArg[0] != 'R' )
		{
			amount = Exp_GetVal( pArg );
			Str_Parse( pArg, &pArg );
		}
		if ( pArg[0] == 'R' )
		{
			// 1 in x chance of creating this.
			if ( Calc_GetRandVal( ATOI( pArg+1 )))
				return( NULL );	// don't create it
		}
	}

	if ( amount == 0 )
		return( NULL );

	ITEMID_TYPE id = (ITEMID_TYPE) rid.GetResIndex();

	if ( fDupeCheck && rid.GetResType() == RES_ITEMDEF && pCont )
	{
		// Check if they already have the item ? In the case of a regen.
		// This is just to keep NEWBIE items from being duped.
		CContainer * pContBase = dynamic_cast <CContainer *> ( pCont );
		ASSERT(pContBase);
		if ( pContBase->ContentFind( rid ))
		{
			// We already have this.
			return( NULL );
		}
	}

	CItem * pItem = CItem::CreateTemplate( id, pCont, pSrc );
	if ( pItem != NULL )
	{
		// Is the item movable ?
		if ( ! pItem->IsMovableType() && pCont && pCont->IsItem())
		{
			DEBUG_ERR(( "Script Error: 0%x item is not movable type, cont=0%x" DEBUG_CR, id, (DWORD)(pCont->GetUID()) ));
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

LPCTSTR const CItem::sm_szTemplateTable[ITC_QTY+1] =
{
	"BUY",
	"CONTAINER",
	"FULLINTERP",
	"ITEM",
	"ITEMNEWBIE",
	"NEWBIESWAP",
	"SELL",
	NULL,
};

CItem * CItem::CreateTemplate( ITEMID_TYPE id, CObjBase * pCont, CChar * pSrc )	// static
{
	// Create an item or a template.
	// A template is a collection of items. (not a single item)
	// But we can create (in this case) only the container item.
	// and return that.
	// ARGS:
	//  pCont = container item or char to put the new template or item in..
	// NOTE:
	//  This can possibly be reentrant under the current scheme.

	if ( id < ITEMID_TEMPLATE )
	{
		CItem * pItem = CreateScript( id, pSrc );
		if ( pCont )
		{
			CContainer * pContBase = dynamic_cast <CContainer *> ( pCont );
			ASSERT(pContBase);
			pContBase->ContentAdd(pItem);
		}
		return( pItem );
	}

	CResourceLock s;
	if ( ! g_Cfg.ResourceLock( s, RESOURCE_ID( RES_TEMPLATE, id )))
		return NULL;

	return( ReadTemplate( s, pCont ));
}

CItem * CItem::ReadTemplate( CResourceLock & s, CObjBase * pCont ) // static
{
	CChar * pVendor = NULL;
	CItemContainer * pVendorSell = NULL;
	CItemContainer * pVendorBuy = NULL;
	if ( pCont != NULL )
	{
		pVendor = dynamic_cast <CChar*>( pCont->GetTopLevelObj());
		if ( pVendor != NULL && pVendor->NPC_IsVendor())
		{
			pVendorSell = pVendor->GetBank( LAYER_VENDOR_STOCK );
			pVendorBuy = pVendor->GetBank( LAYER_VENDOR_BUYS );
		}
	}

	bool fItemAttrib = false;
	CItem * pNewTopCont = NULL;
	CItem * pItem = NULL;
	while ( s.ReadKeyParse())
	{
		if ( s.IsKeyHead( "ON", 2 ))
			break;

		int index = FindTableSorted( s.GetKey(), sm_szTemplateTable, COUNTOF( sm_szTemplateTable )-1 );
		switch (index)
		{
		case ITC_BUY: // "BUY"
		case ITC_SELL: // "SELL"
			fItemAttrib = false;
			if (pVendorBuy != NULL)
			{
				pItem = CItem::CreateHeader( s.GetArgRaw(), (index==ITC_SELL)?pVendorSell:pVendorBuy, false );
				if ( pItem == NULL )
				{
					continue;
				}
				if ( pItem->IsItemInContainer())
				{
					fItemAttrib = true;
					pItem->SetContainedLayer( pItem->GetAmount());	// set the Restock amount.
				}
			}
			continue;

		case ITC_CONTAINER:
			fItemAttrib = false;
			{
				pItem = CItem::CreateHeader( s.GetArgRaw(), pCont, false, pVendor );
				if ( pItem == NULL )
				{
					continue;
				}
				pCont = dynamic_cast <CItemContainer *> ( pItem );
				if ( pCont == NULL )
				{
					DEBUG_ERR(( "CreateTemplate CContainer %s is not a container" DEBUG_CR, pItem->GetResourceName() ));
				}
				else
				{
					fItemAttrib = true;
					if ( ! pNewTopCont )
						pNewTopCont = pItem;
				}
			}
			continue;

		case ITC_ITEM:
		case ITC_ITEMNEWBIE:
			fItemAttrib = false;
			if ( pCont == NULL && pItem != NULL )
			{
				// Don't create anymore items til we have some place to put them !
				continue;
			}
			pItem = CItem::CreateHeader( s.GetArgRaw(), pCont, false, pVendor );
			if ( pItem != NULL )
			{
				fItemAttrib = true;
			}
			continue;
		}

		if ( pItem != NULL && fItemAttrib )
		{
			pItem->r_LoadVal( s );
		}
	}

	if ( pNewTopCont )
	{
		return pNewTopCont;
	}
	return( pItem );
}

bool CItem::IsTopLevelMultiLocked() const
{
	// Is this item locked to the structure ?
	ASSERT( IsTopLevel());
	if ( ! m_uidLink.IsValidUID())	// linked to something.
		return( false );
	if ( IsType(IT_KEY))	// keys cannot be locked down. 
		return( false );
	const CRegionBase * pArea = GetTopPoint().GetRegion( REGION_TYPE_MULTI );
	if ( pArea == NULL )
		return( false );
	if ( pArea->GetResourceID() == m_uidLink )
		return( true );
	return( false );
}

bool CItem::IsMovableType() const
{
	if ( IsAttr( ATTR_MOVE_ALWAYS ))	// this overrides other flags.
		return( true );
	if ( IsAttr( ATTR_MOVE_NEVER | ATTR_STATIC | ATTR_INVIS ))
		return( false );
	const CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);
	if ( ! pItemDef->IsMovableType())
		return( false );
	return( true );
}

bool CItem::IsMovable() const
{
	if ( ! IsTopLevel() && ! IsAttr( ATTR_MOVE_NEVER ))
	{
		// If it's in my pack (event though not normally movable) thats ok.
		return( true );
	}
	return( IsMovableType());
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
	CItemBase * pItemDef = Item_GetDef();
	if ( pItemDef == NULL )
	{
		iResultCode = 0x2102;
		goto bailout;
	}
	if ( pItemDef->GetID() == 0 )
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
		if ( ! GetTopPoint().IsValidPoint())
		{
			iResultCode = 0x2105;
			goto bailout;
		}
		return( 0 );
	}

	// The container must be valid.
	return( GetContainer()->IsWeird());
}

static int GetTypeCode( IT_TYPE type )
{
	return( 0 );
}

CItem * CItem::SetType( IT_TYPE type )
{
	// We need to check this to make sure dynamic_cast won't be broken !
	if ( GetTypeCode(m_type) != GetTypeCode(type))	// we cannot allow this.
		return this;
	m_type = type;
	return( this );
}

int CItem::FixWeirdness()
{
	// Check for weirdness and fix it if possible.
	// RETURN: false = i can't fix this.

	if ( IsType(IT_EQ_MEMORY_OBJ) && ! IsValidUID())
	{
		SetUID( UID_CLEAR, true );	// some cases we don't get our UID because we are created during load.
	}

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
bailout:
		return( iResultCode );
	}

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	CChar * pChar;
	if ( IsItemEquipped())
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

	// Do version Conversion First !

	if ( g_World.m_iLoadVersion <= 32 )
	{
		switch ( GetType() )
		{
		case IT_EQ_HORSE:  // m_Horse_UID
			m_itFigurine.m_UID.SetPrivateUID( m_itNormal.m_more1 );
			m_itNormal.m_more1 = 0;
			break;
		case IT_EQ_MEMORY_OBJ: // m_Memory_Obj_UID
		case IT_SHIP:		 // m_Ship_Tiller_UID
			m_uidLink.SetPrivateUID( m_itNormal.m_more1 );
			m_itNormal.m_more1 = 0;
			break;
		case IT_EQ_TRADE_WINDOW:	// m_Trade_Partner_UID
		case IT_DOOR:
		case IT_DOOR_OPEN:
		case IT_DOOR_LOCKED: // m_Door_Link_UID
			m_uidLink.SetPrivateUID( m_itNormal.m_more2 );
			m_itNormal.m_more2 = 0;
			break;
		}
	}
	if ( g_World.m_iLoadVersion <= 35 )
	{
		switch ( m_type )
		{
		case IT_EQ_HORSE:  // m_Horse_UID
			m_itFigurine.m_UID = m_uidLink;
			m_itFigurine.m_ID = CREID_HORSE1;
			m_uidLink.InitUID();
			break;
		}
	}
	if ( g_World.m_iLoadVersion <= 40 )
	{
		if ( IsType(IT_OLD_CASHIERS_CHECK))
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
	if ( g_World.m_iLoadVersion <= 42 )
	{
		if ( IsType(IT_FIGURINE) && IsAttr(ATTR_INVIS) && IsTopLevel())
		{
			m_type = IT_SPAWN_CHAR;
		}
	}
	if ( g_World.m_iLoadVersion <= 46 )
	{
		if ( GetID() == ITEMID_Bulletin1 || GetID() == ITEMID_Bulletin2 )
		{
			m_type = IT_BBOARD;
		}
		else if ( GetID() == ITEMID_SPELLBOOK )
		{
			m_type = IT_SPELLBOOK;
		}
		else if ( GetID() == ITEMID_CORPSE )
		{
			m_type = IT_CORPSE;
		}
		if ( IsType(IT_RUNE) && m_itRune.m_pntMark.IsValidPoint())
		{
			// Fix a rune that was never given a strength.
			if ( m_itRune.m_Strength == 0 )
				m_itRune.m_Strength = 100;
		}
	}
	if ( g_World.m_iLoadVersion < 52 )
	{
		if ( IsType(IT_EQ_VENDOR_BOX) || IsType(IT_EQ_BANK_BOX) )
		{
			// SetRestockTimeSeconds()
			SetHueAlt( m_itEqBankBox.m_Check_Restock / TICK_PER_SEC );
		}
	}

	if ( g_World.m_iLoadVersion < 53 )
	{
		// Types get screwed pretty bad. just re-assign to the proper type.
		if ( pItemDef->IsTypeEquippable() &&
			GetType() != pItemDef->GetType() &&
			GetType() < IT_TRIGGER )
		{
			SetType( pItemDef->GetType());
		}

		if ( IsType(IT_COIN))
		{
			SetType(IT_GOLD);
		}

		if ( IsType(IT_POTION))
		{
			// Translate the old potion types to spells.
			static const SPELL_TYPE sm_OldPotions[] =
			{
			SPELL_NONE, // POTION_WATER = 100,	// No effect.
			SPELL_Agility, // POTION_AGILITY,			// 1
			SPELL_Agility, // POTION_AGILITY_GREAT,
			SPELL_Cure, // POTION_CURE_LESS,
			SPELL_Cure, // POTION_CURE,
			SPELL_Cure, // POTION_CURE_GREAT,		// 5
			SPELL_Explosion, // POTION_EXPLODE_LESS,	// 6 = purple
			SPELL_Explosion, // POTION_EXPLODE,
			SPELL_Explosion, // POTION_EXPLODE_GREAT,
			SPELL_Heal, // POTION_HEAL_LESS,		// 9 = yellow
			SPELL_Great_Heal, // POTION_HEAL,			// 10
			SPELL_Great_Heal, // POTION_HEAL_GREAT,
			SPELL_Night_Sight, // POTION_NIGHTSIGHT,		// 12
			SPELL_Poison, // POTION_POISON_LESS,
			SPELL_Poison, // POTION_POISON,			// 14
			SPELL_Poison, // POTION_POISON_GREAT,
			SPELL_Poison, // POTION_POISON_DEADLY,	// 16
			SPELL_Refresh, // POTION_REFRESH,
			SPELL_Refresh, // POTION_REFRESH_TOTAL,	// 18
			SPELL_Strength, // POTION_STR,
			SPELL_Strength, // POTION_STR_GREAT,		// 20
			SPELL_NONE, // POTION_FLOAT_LESS,		// 21
			SPELL_NONE, // POTION_FLOAT,
			SPELL_NONE, // POTION_FLOAT_GREAT,
			SPELL_Shrink, // POTION_SHRINK,		// 0x18 = 24
			SPELL_Invis, // POTION_INVISIBLE,
			SPELL_Mana, // POTION_MANA,			// 26
			SPELL_Mana, // POTION_MANA_TOTAL,
			SPELL_NONE, // POTION_LAVA,			// 28
			SPELL_BeastForm, // POTION_BEASTFORM,
			SPELL_NONE, // POTION_CHAMELEON_SKIN,	// 130
			SPELL_Weaken, // POTION_Weakness,	// 131
			SPELL_Cunning, // POTION_Cleverness,			// 132
			SPELL_Cunning, // POTION_Cleverness_Great,	// 133
			SPELL_Clumsy, // POTION_Clumsiness,	// 34
			SPELL_Feeblemind, // POTION_Confusion,
			SPELL_Forget, // POTION_Forget_Less,	// 36
			SPELL_Forget, // POTION_Forget,
			SPELL_Forget, // POTION_Forget_Great,
			SPELL_Gender_Swap,	// POTION_Gender_Swap,	// 139
			SPELL_Trance,		// POTION_Trance_Less,	// 140
			SPELL_Trance,		// POTION_Trance,	// 141
			SPELL_Trance,		// POTION_Trance_Great,	// 142
			SPELL_Restore,		// POTION_Restore,	// 143
			SPELL_Restore,		// POTION_Restore_Great,
			SPELL_Sustenance,	// POTION_Sustenance,
			SPELL_Sustenance,	// POTION_Sustenance_Great,
			SPELL_Monster_Form, // POTION_Monster_Form,	// 147
			SPELL_Particle_Form, // POTION_Particle_Form,
			SPELL_Shield, // POTION_Shield,	// 149
			SPELL_Steelskin, // POTION_Steelskin,	// 150
			SPELL_Stoneskin, // POTION_Stoneskin,	// 151
			};

			if ( m_itPotion.m_Type > COUNTOF(sm_OldPotions))
				m_itPotion.m_Type = SPELL_NONE;
			else
				m_itPotion.m_Type = sm_OldPotions[m_itPotion.m_Type];
		}

		else if ( IsTypeArmorWeapon())
		{
			// scale the magiclevel values.
			// move the poison.

			int itmp = m_itWeapon.m_spellcharges;
			m_itWeapon.m_spellcharges = m_itWeapon.m_poison_skill;
			m_itWeapon.m_poison_skill = itmp / 10;

			if ( IsAttr( ATTR_MAGIC ))
			{
				m_itArmor.m_spelllevel = m_itArmor.m_spelllevel * 100;	// morey=level of the spell. (0-1000)
				if ( m_itArmor.m_spelllevel > 1000 )
				{
					m_itArmor.m_spelllevel = 1000;
				}
			}
		}

		if ( IsTypeLit() && m_itLight.m_pattern == 0 )
		{
			m_itLight.m_pattern = LIGHT_LARGE;
		}
	}

	if ( g_World.m_iLoadVersion < 54 )
	{
		if ( GetID() == ITEMID_GOLD_C1 && IsType(IT_COIN) )
		{
			SetType(IT_GOLD);
		}
	}

	if ( g_World.m_iLoadVersion < 55 )
	{
		switch ( GetType())
		{
		case IT_BOOK:
		case IT_MESSAGE:
			if ( m_itBook.m_ResID.GetPrivateUID() & 0x80000000 )
			{
				m_itBook.m_Time.InitTime( m_itBook.m_ResID.GetPrivateUID() & ~0x80000000 );
				m_itBook.m_ResID.ClearUID();
			}
			else
			{
				m_itBook.m_ResID.SetPrivateUID( m_itBook.m_ResID.GetPrivateUID() | UID_F_RESOURCE );
				m_itBook.m_Time.Init();
			}
		}
	}

	// Make sure the link is valid.

	if ( m_uidLink.IsValidUID())
	{
		// Make sure all my memories are valid !
		if ( m_uidLink == GetUID() || m_uidLink.ObjFind() == NULL )
		{
			if ( m_type == IT_EQ_MEMORY_OBJ )
			{
				return( false );	// get rid of it.	(this is not an ERROR per se)
			}
			if ( IsAttr(ATTR_STOLEN))
			{
				// The item has been laundered.
				m_uidLink.InitUID();
			}
			else
			{
				DEBUG_ERR(( "'%s' Bad Link to 0%lx" DEBUG_CR, (LPCTSTR) GetName(), (DWORD) m_uidLink ));
				m_uidLink.InitUID();
				if ( g_World.m_iLoadVersion > 32 )
				{
					iResultCode = 0x2205;
					goto bailout;	// get rid of it.
				}
			}
		}
	}

	// Check Amount - Only stackable items should set amount ?

	if ( GetAmount() != 1 &&
		! IsType( IT_SPAWN_CHAR ) &&
		! IsType( IT_SPAWN_ITEM ) &&
		! IsStackableException())
	{
		// This should only exist in vendors restock boxes & spawns.
		if ( ! GetAmount())
		{
			SetAmount( 1 );
		}
		// stacks of these should not exist.
		else if ( ! pItemDef->IsStackableType() &&
			! IsType( IT_CORPSE ) &&
			! IsType( IT_EQ_MEMORY_OBJ ))
		{
			SetAmount( 1 );
		}
	}

	// Check Attributes

	if ( IsAttr( ATTR_CURSED|ATTR_CURSED2 ) && IsAttr(ATTR_BLESSED|ATTR_BLESSED2 ))
	{
		// This makes no sense.
		ClrAttr( ATTR_CURSED|ATTR_CURSED2|ATTR_BLESSED|ATTR_BLESSED2 );
	}

	if ( IsMovableType())
	{
		if ( pItemDef->IsType(IT_WATER) || pItemDef->Can( CAN_I_WATER ))
		{
			DEBUG_MSG(( "Movable liquid 0%x?" DEBUG_CR, GetID()));
			SetAttr(ATTR_MOVE_NEVER);
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
				if ( IsAttr( ATTR_MAGIC ) && IsAttr( ATTR_NEWBIE ))
					break;	// special
				if ( ! pChar->IsStatFlag( STATF_DEAD ))
				{
					iResultCode = 0x2207;
					goto bailout;	// get rid of it.
				}
			}
			else
			{
				// Only GM/counsel can have robe.
				// Not a GM til read *ACCT.SCP
				if ( pChar->GetPrivLevel() < PLEVEL_Counsel )
				{
					iResultCode = 0x2208;
					goto bailout;	// get rid of it.
				}
			}
		}
		break;

	case ITEMID_VENDOR_BOX:
		if ( ! IsItemEquipped())
		{
			SetID(ITEMID_BACKPACK);
		}
		else if ( GetEquipLayer() == LAYER_BANKBOX )
		{
			SetID(ITEMID_BANK_BOX);
		}
		else
		{
			SetType(IT_EQ_VENDOR_BOX);
		}
		break;

	case ITEMID_BANK_BOX:
		// These should only be bank boxes and that is it !
		if ( ! IsItemEquipped())
		{
			SetID(ITEMID_BACKPACK);
		}
		else if ( GetEquipLayer() != LAYER_BANKBOX )
		{
			SetID(ITEMID_VENDOR_BOX);
		}
		else
		{
			SetType( IT_EQ_BANK_BOX );
		}
		break;

	case ITEMID_MEMORY:
		// Memory or other flag in my head.
		// Should not exist except equipped.
		if ( ! IsItemEquipped())
		{
			iResultCode = 0x2222;
			goto bailout;	// get rid of it.
		}
		break;
	}

	switch ( GetType() )
	{
	case IT_EQ_TRADE_WINDOW:
		// Should not exist except equipped.
		if ( ! IsItemEquipped() ||
			GetEquipLayer() != LAYER_NONE ||
			pChar->m_pPlayer == NULL )
		{
			iResultCode = 0x2220;
			goto bailout;	// get rid of it.
		}
		break;

	case IT_EQ_CLIENT_LINGER:
		// Should not exist except equipped.
		if ( ! IsItemEquipped() ||
			GetEquipLayer() != LAYER_FLAG_ClientLinger ||
			pChar->m_pPlayer == NULL )
		{
			iResultCode = 0x2221;
			goto bailout;	// get rid of it.
		}
		break;

	case IT_EQ_MEMORY_OBJ:
		{
			// Should not exist except equipped.
			CItemMemory * pItemTemp = dynamic_cast <CItemMemory*>(this);
			if ( pItemTemp == NULL )
			{
				iResultCode = 0x2222;
				goto bailout;	// get rid of it.
			}
			else
			{
				// Gump memory shouldn't be on logged off players
				if ( pItemTemp->GetMemoryTypes() & MEMORY_GUMPRECORD ) 
				{
					if ( pChar && pChar->IsDisconnected() )
					{
						iResultCode = 0x2222;
						goto bailout;	// get rid of it.
					}
				}
			}
		}
		break;

	case IT_EQ_HORSE:
		// These should only exist eqipped.
		if ( ! IsItemEquipped() || GetEquipLayer() != LAYER_HORSE )
		{
			iResultCode = 0x2226;
			goto bailout;	// get rid of it.
		}
		SetTimeout( 10*TICK_PER_SEC );
		break;

	case IT_HAIR:
	case IT_BEARD:	// 62 = just for grouping purposes.
		// Hair should only be on person or equipped. (can get lost in pack)
		// Hair should only be on person or on corpse.
		if ( ! IsItemEquipped())
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
			SetAttr( ATTR_MOVE_NEVER );
			if ( GetEquipLayer() != LAYER_HAIR && GetEquipLayer() != LAYER_BEARD )
			{
				iResultCode = 0x2228;
				goto bailout;	// get rid of it.
			}
		}
		break;

	case IT_GAME_PIECE:
		// This should only be in a game.
		if ( ! IsItemInContainer())
		{
			iResultCode = 0x2229;
			goto bailout;	// get rid of it.
		}
		break;

	case IT_KEY:
		// blank unlinked keys.
		if ( m_itKey.m_lockUID && ! IsValidLockUID())
		{
			DEBUG_MSG(( "Clear 0%x invalid key to 0%x" DEBUG_CR, GetUID(), (DWORD)( m_itKey.m_lockUID)));
			m_itKey.m_lockUID.ClearUID();
		}
		break;

	case IT_SPAWN_CHAR:
	case IT_SPAWN_ITEM:
		Spawn_FixDef();
		Spawn_SetTrackID();
		// gah, I hate it when they do stuff in my stead - Kell
		/*
		if ( ! IsTimerSet())
		{
			Spawn_OnTick( false );
		}
		*/
		break;

	case IT_CONTAINER_LOCKED:
	case IT_SHIP_HOLD_LOCK:
	case IT_DOOR_LOCKED:
		// Doors and containers must have a lock complexity set.
		if ( ! m_itContainer.m_lock_complexity )
		{
			m_itContainer.m_lock_complexity = 500 + Calc_GetRandVal( 600 );
		}
		break;

	case IT_POTION:
		if ( m_itPotion.m_skillquality == 0 ) // store bought ?
		{
			m_itPotion.m_skillquality = Calc_GetRandVal(950);
		}
		break;
	case IT_MAP_BLANK:
		if ( m_itNormal.m_more1 || m_itNormal.m_more2 )
		{
			SetType( IT_MAP );
		}
		break;

	default:
		if ( GetType() > IT_QTY )
		{
			if ( GetType() < IT_TRIGGER )
			{
				m_type = pItemDef->GetType();
			}
			else
			{
				// test of for real ?
				// OnTrigger
			}
		}
	}

	if ( IsItemEquipped())
	{
		ASSERT( pChar );

		switch ( GetEquipLayer())
		{
		case LAYER_NONE:
			// Only Trade windows should be equipped this way..
			if ( m_type != IT_EQ_TRADE_WINDOW )
			{
				iResultCode = 0x2230;
				goto bailout;	// get rid of it.
			}
			break;
		case LAYER_SPECIAL:
			switch ( GetType())
			{
			case IT_EQ_MEMORY_OBJ:
			case IT_EQ_NPC_SCRIPT:
			case IT_EQ_SCRIPT:	// pure script.
				break;
			default:
				iResultCode = 0x2231;
				goto bailout;	// get rid of it.
			}
			break;
		case LAYER_VENDOR_STOCK:
		case LAYER_VENDOR_EXTRA:
		case LAYER_VENDOR_BUYS:
			if ( pChar->m_pPlayer )	// players never need carry these,
				return( false );
			SetAttr(ATTR_MOVE_NEVER);
			break;

		case LAYER_PACK:
		case LAYER_BANKBOX:
			SetAttr(ATTR_MOVE_NEVER);
			break;

#if 0
		case LAYER_PACK2:
			// unused.
			{
				iResultCode = 0x2232;
				goto bailout;	// get rid of it.
			}
#endif

		case LAYER_HORSE:
			if ( m_type != IT_EQ_HORSE )
			{
				iResultCode = 0x2233;
				goto bailout;	// get rid of it.
			}
			SetAttr(ATTR_MOVE_NEVER);
			pChar->StatFlag_Set( STATF_OnHorse );
			break;
		case LAYER_FLAG_ClientLinger:
			if ( m_type != IT_EQ_CLIENT_LINGER )
			{
				iResultCode = 0x2234;
				goto bailout;	// get rid of it.
			}
			break;

		case LAYER_FLAG_Murders:
			if ( ! pChar->m_pPlayer ||
				pChar->m_pPlayer->m_wMurders <= 0 )
			{
				iResultCode = 0x2235;
				goto bailout;	// get rid of it.
			}
			break;
		}
	}

	else if ( IsTopLevel())
	{
		if ( IsAttr(ATTR_DECAY) && ! IsTimerSet())
		{
			iResultCode = 0x2236;
			goto bailout;	// get rid of it.
		}

		if ( GetTimerAdjusted() > 90*24*60*60 )
		{
			// unreasonably long for a top level item ?
			DEBUG_MSG(( "Adjust unreasonable timer length?" ));
			SetTimeout(60*60);
		}
	}

	// is m_BaseDef just set bad ?
	return( IsWeird());
}

CItem * CItem::UnStackSplit( int amount, CChar * pCharSrc )
{
	// Set this item to have this amount.
	// leave another pile behind.
	// can be 0 size if on vendor.
	// ARGS:
	//  amount = the amount that i want to set this pile to
	// RETURN: 
	//  The newly created item.

	if ( amount >= GetAmount() )
		return( NULL );

	ASSERT( amount <= GetAmount());
	CItem * pItemNew = CreateDupeItem( this );
	pItemNew->SetAmount( GetAmount() - amount );
	SetAmountUpdate( amount );

	if ( ! pItemNew->MoveNearObj( this ))
	{
		if ( pCharSrc )
		{
			pCharSrc->ItemBounce( pItemNew );
		}
		else
		{
			// No place to put this item !
			pItemNew->Delete();
		}
	}

	return( pItemNew );
}

bool CItem::IsSameType( const CObjBase * pObj ) const
{
	const CItem * pItem = dynamic_cast <const CItem*> ( pObj );
	if ( pItem == NULL )
		return( false );

	if ( GetID() != pItem->GetID() )
		return( false );
	if ( GetHue() != pItem->GetHue())
		return( false );
	if ( m_type != pItem->m_type )
		return( false );
	if ( m_uidLink != pItem->m_uidLink )
		return( false );

	if ( m_itNormal.m_more1 != pItem->m_itNormal.m_more1 )
		return( false );
	if ( m_itNormal.m_more2 != pItem->m_itNormal.m_more2 )
		return( false );
	if ( m_itNormal.m_morep != pItem->m_itNormal.m_morep )
		return( false );

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
	if ( ! pCont->IsType(IT_EQ_VENDOR_BOX) && ! pCont->IsAttr(ATTR_MAGIC))
		return( false );
	return( true );
}

bool CItem::IsStackable( const CItem * pItem ) const
{
	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	if ( ! pItemDef->IsStackableType())
	{
		// Vendor boxes can stack normally un-stackable stuff.
		if ( ! IsStackableException())
			return( false );
	}

	// try object rotations ex. left and right hand hides ?
	if ( ! IsSameType( pItem ))
		return( false );

	// total should not add up to > 64K !!!
	if ( pItem->GetAmount() > USHRT_MAX - GetAmount())
		return( false );

	return( true );
}

bool CItem::Stack( CItem * pItem )
{
	// RETURN:
	//  true = the item got stacked. (it is gone)
	//  false = the item will not stack. (do somethjing else with it)
	//

	if ( pItem == this )
		return( true );
	if ( ! IsStackable( pItem ))
		return( false );

	// Lost newbie status.
	if ( IsAttr( ATTR_NEWBIE ) != pItem->IsAttr( ATTR_NEWBIE ))
	{
		ClrAttr(ATTR_NEWBIE);
	}

	SetAmount( pItem->GetAmount() + GetAmount());
	pItem->Delete();
	return( true );
}

int CItem::GetDecayTime() const
{
	// Return time in seconds that it will take to decay this item.
	// -1 = never decays.

	switch ( GetType())
	{
	case IT_FOLIAGE:
	case IT_CROPS: // Crops "decay" as they grow
		return( g_World.GetTimeDiff( g_World.GetNextNewMoon(true)) + ( Calc_GetRandVal(20) * g_Cfg.m_iGameMinuteLength ));
	case IT_MULTI:
	case IT_SHIP:
		// very long decay updated as people use it.
		return( 14*24*60*60*TICK_PER_SEC );	// days to rot a structure.
	case IT_TRASH_CAN:
		// Empties every n seconds.
		return( 15*TICK_PER_SEC );
	}

	if ( ! IsAttr( ATTR_CAN_DECAY ))
	{
		if ( IsAttr(ATTR_MOVE_NEVER | ATTR_MOVE_ALWAYS | ATTR_OWNED | ATTR_INVIS | ATTR_STATIC ))
			return( -1 );
		if ( ! IsMovableType())
			return( -1 );
	}
	if ( IsAttr(ATTR_MAGIC))
	{
		// Magic items rot slower.
		return( 4*g_Cfg.m_iDecay_Item );
	}
	if ( IsAttr(ATTR_NEWBIE))
	{
		// Newbie items should rot faster.
		return( 5*60*TICK_PER_SEC );
	}
	return( g_Cfg.m_iDecay_Item );
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

	DEBUG_CHECK( pSector->IsItemInSector(this));

	CItemsList::sm_fNotAMove = true;
	pSector->MoveItemToSector( this, iDelay >= 0 );
	CItemsList::sm_fNotAMove = false;
	SetContainerFlags(0);
}

void CItem::SetDecayTime( int iTime )
{
	// iTime = 0 = set default. (TICK_PER_SEC of a sec)
	// -1 = set none. (clear it)

	if ( IsTimerSet() && ! IsAttr(ATTR_DECAY))
	{
		return;	// already a timer here. let it expire on it's own
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
		SetAttr(ATTR_DECAY);
	}
	else
	{
		ClrAttr(ATTR_DECAY);
	}
}

SOUND_TYPE CItem::GetDropSound( const CObjBase * pObjOn ) const
{
	// Get a special drop sound for the item.
	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	switch ( pItemDef->GetType())
	{
	case IT_COIN:
	case IT_GOLD:
		// depends on amount.
		switch ( GetAmount())
		{
		case 1: return( 0x02e1 );
		case 2: return( 0x02e2 );
		case 3: return( 0x02e3 );
		case 4:	return( 0x02e5 );
		}
		return( 0x02e6 );
	case IT_GEM:
		return(( GetID() > ITEMID_GEMS ) ? 0x034 : 0x032 );  // Small vs Large Gems
	case IT_INGOT:  // Any Ingot
		if ( pObjOn == NULL )
		{
			return( 0x033 );
		}
		break;
	}

	// normal drop sound for what dropped in/on.
	return( pObjOn ? 0x057 : 0x042 );
}

bool CItem::MoveTo( CPointMap pt ) // Put item on the ground here.
{
	// Move this item to it's point in the world. (ground/top level)
	// NOTE: Do NOT do the decay timer here.

	if ( ! pt.IsValidPoint())
		return false;

#ifdef _DEBUG
	if ( pt.m_x < 128 && pt.m_y < 128 )
	{
		DEBUG_CHECK( pt.IsValidPoint());
	}
#endif

	CSector * pSector = pt.GetSector();
	ASSERT( pSector );
	pSector->MoveItemToSector( this, IsTimerSet());

	// Is this area too complex ?
	if ( ! g_Serv.IsLoading())
	{
		int iCount = pSector->GetItemComplexity();
		if ( iCount > g_Cfg.m_iMaxSectorComplexity )
		{
			DEBUG_ERR(( "Warning: %d items at %s,too complex!" DEBUG_CR, iCount, pt.WriteUsed()));
		}
	}

	SetTopPoint( pt );
	ASSERT( IsTopLevel());	// on the ground.

#ifdef _DEBUG
	int iRetWeird = CObjBase::IsWeird();
	if ( iRetWeird )
	{
		DEBUG_CHECK( ! iRetWeird );
	}
#endif

	Update();
	return( true );
}

bool CItem::MoveToCheck( const CPointMap & pt, CChar * pCharMover )
{
	// Make noise and try to pile it and such.

	ASSERT( pt.IsValidPoint());
	CPointMap	ptNewPlace = pt;

	int iItemCount = 0;

	// Look for pileable ? count how many items are here.
	CItem * pItem = NULL;
	CWorldSearch AreaItems( pt );
	DWORD	dMyZ = pt.m_z;
	while (true)
	{
		pItem = AreaItems.GetItem();
		if ( pItem == NULL ) break;
		iItemCount ++;
		// ITRIG_STACKON
		if ( Stack( pItem ))
		{
			dMyZ = 100;
			break;
		}
		if ( pItem->GetTopPoint().m_z > dMyZ ) dMyZ = pItem->GetTopPoint().m_z + 1;
	}
	// one floor. needs some configuration on that
	if ( dMyZ - pt.m_z < 12 ) ptNewPlace.m_z = dMyZ;

	// Set the decay timer for this if not in a house or such.
	int iDecayTime = GetDecayTime();
	if ( iDecayTime > 0 )
	{
		// In a place where it will decay ?
		const CRegionBase * pRegion = pt.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA | REGION_TYPE_ROOM );
		if ( pRegion != NULL && pRegion->IsFlag( REGION_FLAG_NODECAY ))	// No decay here in my house/boat
			iDecayTime = -1;
	}

	MoveToDecay(ptNewPlace, iDecayTime);

	if ( iItemCount > g_Cfg.m_iMaxItemComplexity )
	{
		// Too many items here !!! ???
		Speak( "Too many items here!" );
		if ( iItemCount > g_Cfg.m_iMaxItemComplexity + g_Cfg.m_iMaxItemComplexity/2 )
		{
			Speak( "The ground collapses!" );
			Delete();
		}
		// attempt to reject the move.
		return( false );
	}

	// iItemCount - ITRIG_DROPON_ITEM

	OnTrigger( ITRIG_DROPON_GROUND, pCharMover );
	Sound( GetDropSound(NULL));

	return( true );
}

bool CItem::MoveNearObj( const CObjBaseTemplate * pObj, int iSteps, WORD wCan )
{
	// Put in the same container as another item.
	ASSERT(pObj);
	CItemContainer * pPack = (dynamic_cast <CItemContainer*> (pObj->GetParent()));
	if ( pPack != NULL )
	{
		// Put in same container (make sure they don't get re-combined)
		pPack->ContentAdd( this, pObj->GetContainedPoint());
		return( true );
	}
	else 
	{
		// Equipped or on the ground so put on ground nearby.
		return CObjBase::MoveNearObj( pObj, iSteps, wCan );
	}
}

LPCTSTR CItem::GetName() const
{
	// allow some items to go unnamed (just use type name).

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	LPCTSTR pszNameBase;
	if ( IsIndividualName())
	{
		pszNameBase = CObjBase::GetName();
	}
	else if ( pItemDef->HasTypeName())
	{

		pszNameBase	= NULL;
		if ( g_Cfg.IsSetOF( OF_Items_AutoName ) )
		{
			if ( IsType( IT_SCROLL ) || IsType( IT_POTION ) )
			{
				CSpellDef * pSpell	= g_Cfg.GetSpellDef( (SPELL_TYPE) m_itSpell.m_spell );
				if ( pSpell )
					pszNameBase	= pSpell->GetName();
			}
		}
		if ( !pszNameBase )			// Just use it's type name instead.
			pszNameBase = pItemDef->GetTypeName();
	}
	else
	{
		// Get the name of the link.
		const CObjBase * pObj = m_uidLink.ObjFind();
		if ( pObj && pObj != this )
		{
			return( pObj->GetName());
		}
		return( _TEXT("unnamed") );
	}

	// Watch for names that should be pluralized.
	// Get rid of the % in the names.
	return( CItemBase::GetNamePluralize( pszNameBase, m_amount != 1 && ! IsType(IT_CORPSE) ));
}

LPCTSTR CItem::GetNameFull( bool fIdentified ) const
{
	// Should be LPCTSTR
	// Get a full descriptive name. Prefixing and postfixing.

	int len = 0;
	TCHAR * pTemp = Str_GetTemp();

	LPCTSTR pszTitle = NULL;
	LPCTSTR pszName = GetName();

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	bool fSingular = (GetAmount()==1 || IsType(IT_CORPSE));
	if (fSingular) // m_corpse_DispID is m_amount
	{
		if ( ! IsIndividualName())
		{
			len += strcpylen( pTemp+len, pItemDef->GetArticleAndSpace());
		}
	}
	else
	{
		pszTitle = "%i ";
		len += sprintf( pTemp+len, pszTitle, GetAmount());
	}

	if ( fIdentified && IsAttr(ATTR_CURSED|ATTR_CURSED2|ATTR_BLESSED|ATTR_BLESSED2|ATTR_MAGIC))
	{
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
			if ( fSingular )
			{
				len = strcpylen( pTemp, Str_GetArticleAndSpace(pszTitle));
			}
			len += strcpylen( pTemp+len, pszTitle );
		}

		if ( IsAttr(ATTR_MAGIC))
		{
			if ( m_itWeapon.m_spelllevel && IsTypeArmorWeapon() && ! IsType(IT_WAND))
			{
				// A weapon, (Not a wand)
				if ( ! pszTitle )
				{
					pszTitle = "a ";
					len = strcpylen( pTemp, pszTitle );
				}
				len += sprintf( pTemp+len, "%c%d ", ( m_itWeapon.m_spelllevel<0 ) ? '-':'+', abs( g_Cfg.GetSpellEffect( SPELL_Enchant, m_itWeapon.m_spelllevel )));
			}
			else
			{
				// Don't put "magic" in front of "magic key"
				if ( strnicmp( pszName, "MAGIC", 5 ))
				{
					if ( ! pszTitle )
					{
						pszTitle = "a ";
						len = strcpylen( pTemp, pszTitle );
					}
					len += strcpylen( pTemp+len, "Magic " );
				}
			}
		}
	}

	// Prefix the name
	switch ( m_type )
	{
	case IT_ADVANCE_GATE:
		{
			CREID_TYPE id = (CREID_TYPE) RES_GET_INDEX( m_itAdvanceGate.m_Type );
			CCharBase * pCharDef = CCharBase::FindCharBase( id );
			strcpy( pTemp+len, (pCharDef==NULL)? "Blank Advance Gate" : pCharDef->GetTradeName());
		}
		return ( pTemp );
	case IT_STONE_GUILD:
		len += strcpylen( pTemp+len, "Guildstone for " );
		break;
	case IT_STONE_TOWN:
		len += strcpylen( pTemp+len, "Town of " );
		break;
	case IT_EQ_MEMORY_OBJ:
		len += strcpylen( pTemp+len, "Memory of " );
		break;
	case IT_SPAWN_CHAR:
		if ( ! IsIndividualName())
			len += strcpylen( pTemp+len, "Spawn " );
		break;

	case IT_KEY:
		if ( ! m_itKey.m_lockUID.IsValidUID())
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		break;
	case IT_RUNE:
		if ( ! m_itRune.m_pntMark.IsCharValid())
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		else if ( ! m_itRune.m_Strength )
		{
			len += strcpylen( pTemp+len, "Faded " );
		}
		break;

	case IT_TELEPAD:
		if ( ! m_itTelepad.m_pntMark.IsValidPoint())
		{
			len += strcpylen( pTemp+len, "Blank " );
		}
		break;
	}

	len += strcpylen( pTemp+len, pszName );

	// Suffix the name.

	if ( fIdentified &&
		IsAttr(ATTR_MAGIC) &&
		IsTypeArmorWeapon())	// wand is also a weapon.
	{
		SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX( m_itWeapon.m_spell );
		if ( spell )
		{
			const CSpellDef * pSpellDef = g_Cfg.GetSpellDef( spell );
			if ( pSpellDef )
			{
				len += sprintf( pTemp+len, " of %s", pSpellDef->GetName());
				if (m_itWeapon.m_spellcharges)
				{
					len += sprintf( pTemp+len, " (%d charges)", m_itWeapon.m_spellcharges );
				}
			}
		}
	}

	switch ( m_type )
	{
	case IT_LOOM:
		if ( m_itLoom.m_ClothQty )
		{
			ITEMID_TYPE AmmoID = (ITEMID_TYPE) RES_GET_INDEX( m_itLoom.m_ClothID );
			const CItemBase * pAmmoDef = CItemBase::FindItemBase(AmmoID);
			if ( pAmmoDef )
			{
				len += sprintf( pTemp+len, " (%d %ss)", m_itLoom.m_ClothQty, pAmmoDef->GetName());
			}
		}
		break;

	case IT_ARCHERY_BUTTE:
		if ( m_itArcheryButte.m_AmmoCount )
		{
			ITEMID_TYPE AmmoID = (ITEMID_TYPE) RES_GET_INDEX( m_itArcheryButte.m_AmmoType );
			const CItemBase * pAmmoDef = CItemBase::FindItemBase(AmmoID);
			if ( pAmmoDef )
			{
				len += sprintf( pTemp+len, " %d %ss", m_itArcheryButte.m_AmmoCount, pAmmoDef->GetName());
			}
		}
		break;

	case IT_STONE_TOWN:
		{
			const CItemStone * pStone = dynamic_cast <const CItemStone*>(this);
			ASSERT(pStone);
			len += sprintf( pTemp+len, " (pop:%d)", pStone->GetCount());
		}
		break;
	case IT_MEAT_RAW:
	case IT_LEATHER:
	case IT_HIDE:
	case IT_FEATHER:
	case IT_FUR:
	case IT_WOOL:
	case IT_BLOOD:
	case IT_BONE:
		if ( fIdentified )
		{
			CREID_TYPE id = (CREID_TYPE) RES_GET_INDEX( m_itSkin.m_creid );
			if ( id)
			{
				CCharBase * pCharDef = CCharBase::FindCharBase( id );
				if ( pCharDef)
				{
					len += sprintf( pTemp+len, " (%s)", pCharDef->GetTradeName());
				}
			}
		}
		break;

	case IT_LIGHT_LIT:
	case IT_LIGHT_OUT:
		// how many charges ?
		if ( m_itLight.m_charges != USHRT_MAX )
		{
			if ( pItemDef->m_ttEquippable.m_Light_ID.GetResIndex() )
			{
				len += sprintf( pTemp+len, " (%d charges)", m_itLight.m_charges );
			}
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

bool CItem::SetName( LPCTSTR pszName )
{
	// Can't be a dupe name with type name ?
	ASSERT(pszName);
	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	LPCTSTR pszTypeName = pItemDef->GetTypeName();
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

bool CItem::SetBase( CItemBase * pItemDef )
{
	// Total change of type. (possibly dangerous)

	CItemBase * pItemOldDef = Item_GetDef();

	if ( pItemDef == NULL || pItemDef == pItemOldDef )
		return false;

	// This could be a weight change for my parent !!!
	CContainer * pParentCont = dynamic_cast <CContainer*> (GetParent());
	int iWeightOld = 0;

	if ( pItemOldDef)
	{
		if ( pParentCont )
		{
			iWeightOld = GetWeight();
		}
	}

	m_BaseRef.SetRef(pItemDef);

	if (pParentCont)
	{
		ASSERT( IsItemEquipped() || IsItemInContainer());
		pParentCont->OnWeightChange( GetWeight() - iWeightOld );
	}

	m_type = pItemDef->GetType();	// might change the type.
	return( true );
}

bool CItem::SetBaseID( ITEMID_TYPE id )
{
	// Converting the type of an existing item is possibly risky.
	// Creating invalid objects of any sort can crash clients.
	// User created items can be overwritten if we do this twice !
	CItemBase * pItemDef = CItemBase::FindItemBase( id );
	if ( pItemDef == NULL )
	{
		DEBUG_ERR(( "SetBaseID 0%x invalid item uid=0%x" DEBUG_CR,
		id, (DWORD) GetUID()));
		return false;
	}
	SetBase( pItemDef );	// set new m_type etc
	return( true );
}

bool CItem::SetID( ITEMID_TYPE id )
{
	if ( ! IsSameDispID( id ))
	{
		if ( ! SetBaseID( id ))
			return( false );
	}
	SetDispID( id );
	return( true );
}

bool CItem::SetDispID( ITEMID_TYPE id )
{
	// Just change what this item looks like.
	// do not change it's basic type.

	if ( id == GetDispID())
		return true;	// no need to do this again or overwrite user created item types.

	if ( CItemBase::IsValidDispID(id) && id < ITEMID_MULTI )
	{
		m_wDispIndex = id;
	}
	else
	{
		CItemBase * pItemDef = Item_GetDef();
		ASSERT(pItemDef);
		m_wDispIndex = pItemDef->GetDispID();
		ASSERT( CItemBase::IsValidDispID((ITEMID_TYPE)m_wDispIndex));
	}
	return( true );
}

void CItem::SetAmount( int amount )
{
	// propagate the weight change.
	// Setting to 0 might be legal if we are deleteing it ?

	int oldamount = GetAmount();
	if ( oldamount == amount )
		return;

	DEBUG_CHECK( amount >= 0 && amount <= USHRT_MAX );
	m_amount = amount;

	// sometimes the diff graphics for the types are not in the client.
	if ( IsType(IT_ORE))
	{
		static const ITEMID_TYPE sm_Item_Ore[] =
		{
			ITEMID_ORE_1, ITEMID_ORE_1, ITEMID_ORE_2, ITEMID_ORE_3
		};
		SetDispID( ( GetAmount() >= COUNTOF(sm_Item_Ore)) ? ITEMID_ORE_4 : sm_Item_Ore[GetAmount()] );
	}

#if 0
	// Problems with the replicate flag in the client (causes the Amount box to pop up on drag)
	bool fMirror = false;
	int iDispAmount = 0;
	switch ( m_pDef->GetDispID() )
	{
	case ITEMID_FEATHERS1:	// 1, 3, 7
		if ( amount == 2 )
		{
			SetDispID( Calc_GetRandVal(2) ? ITEMID_FEATHERS2a : ITEMID_FEATHERS2b );
			break;
		}
	case ITEMID_SHAFTS1:	// 1, 3, 15
		iDispAmount = 3;
		break;
	case ITEMID_LUMBER1:	// 1, 3, 7, 1, 3, 7
	case ITEMID_LOG_1:		// 1, 3, 20, 1, 3, 20
	case ITEMID_INGOT_COPPER:	// 1, 3, 6, 1, 3, 6
	case ITEMID_INGOT_GOLD:		// 1, 3, 6, 1, 3, 6
	case ITEMID_INGOT_IRON:		// 1, 3, 6, 1, 3, 6
	case ITEMID_INGOT_SILVER:	// 1, 3, 6, 1, 3, 6
		fMirror = true;
		iDispAmount = 3;
		break;
	case ITEMID_Arrow:		// 1, 5, 20, 1
	case ITEMID_XBolt:		// 1, 5, 20, 1
		iDispAmount = 5;
		break;
	}
	if ( iDispAmount )
	{
		WORD wDispIndex = m_pDef->GetDispID();
		if ( amount >= iDispAmount )
		{
			if ( amount > iDispAmount + 2 )
				wDispIndex = wDispIndex + 2;
			else
				wDispIndex = wDispIndex + 1;
		}
		if ( fMirror && Calc_GetRandVal(2))
		{
			wDispIndex = wDispIndex + 3;
		}
		SetDispID((ITEMID_TYPE)wDispIndex);
	}
#endif

	CContainer * pParentCont = dynamic_cast <CContainer*> (GetParent());
	if (pParentCont)
	{
		ASSERT( IsItemEquipped() || IsItemInContainer());
		CItemBase * pItemDef = Item_GetDef();
		ASSERT(pItemDef);
		pParentCont->OnWeightChange(( amount - oldamount ) * pItemDef->GetWeight());
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
	s.Printf( "SERIAL %d\n", (DWORD) GetUID());
	s.Printf( "NAME %s\n", GetName());
	s.Printf( "ID %d\n", GetDispID());
	s.Printf( "X %d\n", GetTopPoint().m_x );
	s.Printf( "Y %d\n", GetTopPoint().m_y );
	s.Printf( "Z %d\n", GetTopZ());
	s.Printf( "CONT %d\n", -1 );
	s.Printf( "TYPE %d\n", m_type );
	s.Printf( "AMOUNT %d\n", m_amount );
	s.Printf( "COLOR %d\n", GetHue());
	//ATT 5
	//VALUE 1
	s.Printf( "}\n\n" );
}

void CItem::r_WriteMore1( CGString & sVal )
{
	// do special processing to represent this.
	switch ( GetType())
	{
	case IT_SPAWN_CHAR:
		sVal = g_Cfg.ResourceGetName( m_itSpawnChar.m_CharID );
		return;
	case IT_RESEARCH_ITEM:
		sVal = g_Cfg.ResourceGetName( m_itResearchItem.m_ItemID );
		return;
	case IT_SPAWN_ITEM:
		sVal = g_Cfg.ResourceGetName( m_itSpawnItem.m_ItemID );
		return;
	case IT_TREE:
	case IT_GRASS:
	case IT_ROCK:
	case IT_WATER:
		sVal = g_Cfg.ResourceGetName( m_itResource.m_rid_res );
		return;

	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_ITEMDEF, m_itFood.m_cook_id ));
		return;

	case IT_TRAP:
	case IT_TRAP_ACTIVE:
	case IT_TRAP_INACTIVE:
	case IT_ANIM_ACTIVE:
	case IT_SWITCH:
	case IT_DEED:
	case IT_LOOM:
	case IT_ARCHERY_BUTTE:
	case IT_ITEM_STONE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_ITEMDEF, m_itNormal.m_more1 ));
		return;

	case IT_FIGURINE:
	case IT_EQ_HORSE:
	case IT_ADVANCE_GATE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, m_itNormal.m_more1 ));
		return;

	case IT_POTION:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_SPELL, m_itPotion.m_Type ));
		return;
	}

	sVal.FormatHex( m_itNormal.m_more1 );
}

void CItem::r_WriteMore2( CGString & sVal )
{
	// do special processing to represent this.
	switch ( GetType())
	{
	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, m_itFood.m_MeatType ));
		return;

	case IT_CROPS:
	case IT_FOLIAGE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_ITEMDEF, m_itCrop.m_ReapFruitID ));
		return;

	case IT_LEATHER:
	case IT_HIDE:
	case IT_FEATHER:
	case IT_FUR:
	case IT_WOOL:
	case IT_BLOOD:
	case IT_BONE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_CHARDEF, m_itNormal.m_more2 ));
		return;

	case IT_ANIM_ACTIVE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_TYPEDEF, m_itAnim.m_PrevType ));
		return;
	}
	sVal.FormatHex( m_itNormal.m_more2 );
}

void CItem::r_Serialize( CGFile & a )
{
	// Read and write binary.
	CObjBase::r_Serialize(a);

}

void CItem::r_Write( CScript & s )
{
	DEBUG_CHECK( ! IsWeird());

	s.WriteSection( "WORLDITEM %s", GetResourceName());

	CObjBase::r_Write( s );

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	if ( GetDispID() != GetID())	// the item is flipped.
	{
		s.WriteKey( "DISPID", g_Cfg.ResourceGetName( RESOURCE_ID( RES_ITEMDEF, GetDispID() )));
	}
	if ( GetAmount() != 1 )
		s.WriteKeyVal( "AMOUNT", GetAmount());
	if ( ! pItemDef->IsType(m_type))
		s.WriteKey( "TYPE", g_Cfg.ResourceGetName( RESOURCE_ID( RES_TYPEDEF, m_type )));
	if ( m_uidLink.IsValidUID())
		s.WriteKeyHex( "LINK", m_uidLink );
	if ( m_Attr )
		s.WriteKeyHex( "ATTR", m_Attr );

	if ( m_itNormal.m_more1 )
	{
		CGString sVal;
		r_WriteMore1(sVal);
		s.WriteKey( "MORE1", sVal );
	}
	if ( m_itNormal.m_more2 )
	{
		CGString sVal;
		r_WriteMore2(sVal);
		s.WriteKey( "MORE2", sVal );
	}
	if ( m_itNormal.m_morep.m_x || m_itNormal.m_morep.m_y || m_itNormal.m_morep.m_z || m_itNormal.m_morep.m_mapplane )
	{
		s.WriteKey( "MOREP", m_itNormal.m_morep.WriteUsed());
	}

	CObjBase * pCont = GetContainer();
	if ( pCont != NULL )
	{
		if ( pCont->IsChar())
		{
			if ( GetEquipLayer() >= LAYER_HORSE )
			{
				s.WriteKeyVal( "LAYER", GetEquipLayer());
			}
		}
		s.WriteKeyHex( "CONT", pCont->GetUID());
		if ( pCont->IsItem())
		{
			// Where in the container?
			s.WriteKey( "P", GetContainedPoint().WriteUsed());
		}
	}
	else
	{
		s.WriteKey( "P", GetTopPoint().WriteUsed());
	}
}

bool CItem::LoadSetContainer( CGrayUID uid, LAYER_TYPE layer )
{
	// Set the CItem in a container of some sort.
	// Used mostly during load.
	// "CONT" IC_CONT
	// NOTE: We don't have a valid point in the container yet probably. get that later.

#ifdef _DEBUG
//if ( GetID() == 0x1711 )
//{
//		DEBUG_MSG(("boots" DEBUG_CR));
//	}
#endif

	CObjBase * pObjCont = uid.ObjFind();
	if ( pObjCont == NULL )
	{
		DEBUG_ERR(( "Invalid container 0%lx" DEBUG_CR, (DWORD) uid ));
		return( false );	// not valid object.
	}

	if ( pObjCont->IsItem())
	{
		// layer is not used here of course.

		CItemContainer * pCont = dynamic_cast <CItemContainer *> (pObjCont);
		if ( pCont != NULL )
		{
#if 0
			if ( pCont->GetID() == 0x09b2 )
			{
				DEBUG_CHECK("sdfsdf" DEBUG_CR);
			}
#endif
			pCont->ContentAdd( this );
			return( true );
		}
	}
	else
	{
		CChar * pChar = dynamic_cast <CChar *> (pObjCont);
		if ( pChar != NULL )
		{
			// equip the item
			CItemBase * pItemDef = Item_GetDef();
			ASSERT(pItemDef);
			if ( ! layer ) layer = pItemDef->GetEquipLayer();

#if 0
			if ( layer == LAYER_BANKBOX )
			{
				DEBUG_CHECK("sdfsdf" DEBUG_CR);
			}
#endif

			pChar->LayerAdd( this, layer );
			return( true );
		}
	}

	DEBUG_ERR(( "Non container uid=0%lx,id=0%x" DEBUG_CR, (DWORD) uid, pObjCont->GetBaseID() ));
	return( false );	// not a container.
}

enum IC_TYPE
{
	IC_ADDSPELL,
	IC_AMOUNT,
	IC_ATTR,
	IC_CONT,
	IC_DISPID,
	IC_DISPIDDEC,
	IC_FRUIT,
	IC_HITPOINTS,
	IC_HITS,
	IC_ID,
	IC_LAYER,
	IC_LINK,
	IC_MAXHITS,
	IC_MORE,
	IC_MORE1,
	IC_MORE1h,
	IC_MORE1l,
	IC_MORE2,
	IC_MORE2h,
	IC_MORE2l,
	IC_MORE3,
	IC_MOREM,
	IC_MOREP,
	IC_MOREX,
	IC_MOREY,
	IC_MOREZ,
	IC_MOVABLE,
	IC_P,
	IC_TYPE,
	IC_QTY,
};

LPCTSTR const CItem::sm_szLoadKeys[IC_QTY+1] =
{
	"ADDSPELL",
	"AMOUNT",
	"ATTR",
	"CONT",
	"DISPID",
	"DISPIDDEC",
	"FRUIT",
	"HITPOINTS",	// for weapons
	"HITS",
	"ID",
	"LAYER",
	"LINK",
	"MAXHITS",
	"MORE",
	"MORE1",
	"more1h",
	"more1l",
	"MORE2",
	"more2h",
	"more2l",
	"MORE3",
	"MOREM",
	"MOREP",
	"MOREX",
	"MOREY",
	"MOREZ",
	"MOVABLE",
	"P",
	"TYPE",
	NULL,
};

void CItem::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CObjBase::r_DumpLoadKeys(pSrc);
}
void CItem::r_DumpVerbKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szVerbKeys);
	CObjBase::r_DumpVerbKeys(pSrc);
}

bool CItem::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
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
	if ( ! strnicmp( pszKey, "REGION.", 7 ))
	{
		pszKey += 7;
		pRef = GetTopLevelObj()->GetTopPoint().GetRegion(REGION_TYPE_MULTI |REGION_TYPE_AREA);
		return( true );
	}
	return( CObjBase::r_GetRef( pszKey, pRef ));
}

bool CItem::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	int index;
	if ( !strnicmp( "ADDSPELL", pszKey, 8 ) )
		index	= IC_ADDSPELL;
	else
		index	= FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );

	switch ( index )
	{
	case IC_ADDSPELL:
		pszKey	+= 8;
		SKIP_SEPERATORS( pszKey );
		sVal.FormatVal( IsSpellInBook( (SPELL_TYPE) RES_GET_INDEX(Exp_GetVal( pszKey )) ) );
		break;
	case IC_AMOUNT:
		sVal.FormatVal( GetAmount());
		break;
	case IC_ATTR:
		sVal.FormatHex( m_Attr );
		break;
	case IC_CONT:
		{
			CObjBase * pCont = GetContainer();
			sVal.FormatHex( pCont ? ((DWORD) pCont->GetUID() ) : 0 );
		}
		break;
	case IC_DISPID:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_ITEMDEF, GetDispID()));
		break;
	case IC_DISPIDDEC:
		{
			int iVal = GetDispID();
	 		if ( IsType(IT_COIN)) // Fix money piles
			{
				CItemBase * pItemDef = Item_GetDef();
				ASSERT(pItemDef);

				iVal = pItemDef->GetDispID();
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
	case IC_HITS:
		sVal.FormatVal(LOWORD(m_itNormal.m_more1));
		break;
	case IC_HITPOINTS:
		if ( ! IsTypeArmorWeapon())
			return( false );
		sVal.FormatVal( m_itArmor.m_Hits_Cur );
		break;
	case IC_ID:
		goto dodefault;
	case IC_LAYER:
		if ( IsItemEquipped())
		{
			sVal.FormatVal( GetEquipLayer() );
			break;
		}
		goto dodefault;
	case IC_LINK:
		sVal.FormatHex( m_uidLink );
		break;
	case IC_MAXHITS:
		sVal.FormatVal(HIWORD(m_itNormal.m_more1));
		break;
	case IC_MORE:
	case IC_FRUIT:
		sVal.FormatVal( m_itNormal.m_more1 );
		break;
	case IC_MORE1:
		r_WriteMore1(sVal);
		break;
	case IC_MORE1h:
		sVal.FormatVal( HIWORD( m_itNormal.m_more1 ));
		break;
	case IC_MORE1l:
		sVal.FormatVal( LOWORD( m_itNormal.m_more1 ));
		break;
	case IC_MORE2:
		r_WriteMore2(sVal);
		break;
	case IC_MORE2h:
		sVal.FormatVal( HIWORD( m_itNormal.m_more2 ));
		break;
	case IC_MORE2l:
		sVal.FormatVal( LOWORD( m_itNormal.m_more2 ));
		break;
	case IC_MORE3:
		sVal.FormatHex( *((DWORD*) (&m_itNormal.m_morep)) );
		break;
	case IC_MOREM:
		sVal.FormatVal( m_itNormal.m_morep.m_mapplane );
		break;
	case IC_MOREP:
		sVal = m_itNormal.m_morep.WriteUsed();
		break;
	case IC_MOREX:
		sVal.FormatVal( m_itNormal.m_morep.m_x );
		break;
	case IC_MOREY:
		sVal.FormatVal( m_itNormal.m_morep.m_y );
		break;
	case IC_MOREZ:
		sVal.FormatVal( m_itNormal.m_morep.m_z );
		break;
	case IC_P:
		goto dodefault;
	case IC_TYPE:
		sVal = g_Cfg.ResourceGetName( RESOURCE_ID( RES_TYPEDEF, m_type ));
		break;
	default:
dodefault:
		return( CObjBase::r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CItem");
	return( true );
}

bool CItem::r_LoadVal( CScript & s ) // Load an item Script
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case IC_ADDSPELL:
		// Add this spell to the i_spellbook.
		if ( AddSpellbookSpell( (SPELL_TYPE) RES_GET_INDEX( s.GetArgVal()), false ))
			return( false );
		return( true );
	case IC_AMOUNT:
		SetAmount( s.GetArgVal());
		return true;
	case IC_ATTR:
		m_Attr = s.GetArgVal();
		return true;
	case IC_CONT:	// needs special processing.
		// Loading or import.
		return LoadSetContainer( s.GetArgVal(), (LAYER_TYPE) GetUnkZ());
	case IC_DISPID:
	case IC_DISPIDDEC:
		return SetDispID((ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr()));
	case IC_HITS:
		m_itNormal.m_more1 = MAKEDWORD(s.GetArgVal(), HIWORD(m_itNormal.m_more1));
		break;
	case IC_HITPOINTS:
		if ( ! IsTypeArmorWeapon())
		{
			DEBUG_ERR(( "Item:Hitpoints assigned for non-weapon %s" DEBUG_CR, GetResourceName()));
			return( false );
		}
		m_itArmor.m_Hits_Cur = m_itArmor.m_Hits_Max = s.GetArgVal();
		return true;
	case IC_ID:
		return SetID((ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr()));
	case IC_LAYER:
		// used only during load.
		if ( ! IsDisconnected() && ! IsItemInContainer() && ! IsItemEquipped())
		{
			return( false );
		}
		SetUnkZ( s.GetArgVal()); // GetEquipLayer()
		return true;
	case IC_LINK:
		m_uidLink = s.GetArgVal();
		return true;

	case IC_FRUIT:	// m_more1
		DEBUG_CHECK( IsType(IT_CROPS) || IsType(IT_FOLIAGE));
		m_itCrop.m_ReapFruitID = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr());
		return true;
	case IC_MAXHITS:
		m_itNormal.m_more1 = MAKEDWORD(LOWORD(m_itNormal.m_more1), s.GetArgVal());
		break;
	case IC_MORE:
	case IC_MORE1:
		m_itNormal.m_more1 = s.GetArgVal();
		if ( !g_Serv.IsLoading( )
			&& ( IsType( IT_SPAWN_CHAR ) || IsType( IT_SPAWN_ITEM ) ) )
		{
			Spawn_FixDef();
			Spawn_SetTrackID();
			RemoveFromView();
			Update();
		}
		return true;
	case IC_MORE1h:
		m_itNormal.m_more1 = MAKEDWORD( LOWORD(m_itNormal.m_more1), s.GetArgVal());
		break;
	case IC_MORE1l:
		m_itNormal.m_more1 = MAKEDWORD( s.GetArgVal(), HIWORD(m_itNormal.m_more1));
		break;
	case IC_MORE2:
		m_itNormal.m_more2 = s.GetArgVal();
		return true;
	case IC_MORE2h:
		m_itNormal.m_more2 = MAKEDWORD( LOWORD(m_itNormal.m_more2), s.GetArgVal());
		break;
	case IC_MORE2l:
		m_itNormal.m_more2 = MAKEDWORD( s.GetArgVal(), HIWORD(m_itNormal.m_more2));
		break;
	case IC_MOREM:
		m_itNormal.m_morep.m_mapplane = s.GetArgVal();
		break;
	case IC_MOREP:
		m_itNormal.m_morep = g_Cfg.GetRegionPoint( s.GetArgStr());
		return true;
	case IC_MOREX:
		m_itNormal.m_morep.m_x = s.GetArgVal();
		return true;
	case IC_MOREY:
		m_itNormal.m_morep.m_y = s.GetArgVal();
		return true;
	case IC_MOREZ:
		m_itNormal.m_morep.m_z = s.GetArgVal();
		return true;
	case IC_MOVABLE:
		switch (s.GetArgVal())
		{
		case -1:
			ClrAttr(ATTR_MOVE_NEVER);
			ClrAttr(ATTR_MOVE_ALWAYS);
			break;
		case 0:
			SetAttr(ATTR_MOVE_NEVER);
			ClrAttr(ATTR_MOVE_ALWAYS);
			break;
		case 1:
		case 2:
			ClrAttr(ATTR_MOVE_NEVER);
			SetAttr(ATTR_MOVE_ALWAYS);
			break;
		}
		return true;
	case IC_P:
		// Loading or import ONLY ! others use the r_Verb
		if ( ! IsDisconnected() && ! IsItemInContainer())
		{
			DEBUG_CHECK( IsDisconnected() || IsItemInContainer());
			return( false );
		}
		else
		{
			// Will be placed in the world later.
			CPointMap pt;
			pt.Read( s.GetArgStr());
			SetUnkPoint(pt);
		}
		return true;
	case IC_TYPE:
		SetType( (IT_TYPE) g_Cfg.ResourceGetIndexType( RES_TYPEDEF, s.GetArgStr()));
		return true;
	default:
		return( CObjBase::r_LoadVal( s ));
	}
	EXC_CATCH("CItem");
	return( true );
}

bool CItem::r_Load( CScript & s ) // Load an item from script
{
	CScriptObj::r_Load( s );
	if ( GetContainer() == NULL )	// Place into the world.
	{
		if ( GetTopPoint().IsCharValid())
		{
			MoveTo( GetTopPoint());
		}
	}

	int iResultCode = CObjBase::IsWeird();
	if ( iResultCode )
	{
		DEBUG_ERR(( "Item 0%x Invalid, id=%s, code=0%x" DEBUG_CR,
		(DWORD) GetUID(), GetResourceName(), iResultCode ));
		Delete();
		return( true );
	}

	return( true );
}

enum CIV_TYPE
{
	CIV_BOUNCE,
	CIV_CONSUME,
	CIV_DECAY,
	CIV_DROP,
	CIV_DUPE,
	CIV_EQUIP,
	CIV_UNEQUIP,
	CIV_USE,
	CIV_QTY,
};

LPCTSTR const CItem::sm_szVerbKeys[CIV_QTY+1] =
{
	"BOUNCE",
	"CONSUME",
	"DECAY",
	"DROP",
	"DUPE",
	"EQUIP",	// engage the equip triggers.
	"UNEQUIP",	// engage the unequip triggers.
	"USE",
	NULL,
};

bool CItem::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	EXC_TRY(("r_Verb('%s %s',%x)", s.GetKey(), s.GetArgStr(), pSrc));
	ASSERT(pSrc);

	int index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF( sm_szVerbKeys )-1 );
	if ( index < 0 )
	{
		return( CObjBase::r_Verb( s, pSrc ));
	}

	CChar * pCharSrc = pSrc->GetChar();

	switch ( index )
	{
	case CIV_BOUNCE:
		if ( ! pCharSrc )
			return( false );
		pCharSrc->ItemBounce( this );
		break;
	case CIV_CONSUME:
		ConsumeAmount( s.HasArgs() ? s.GetArgVal() : 1 );
		break;
	case CIV_DECAY:
		SetDecayTime( s.GetArgVal());
		break;
	case CIV_DROP:
		{
			CObjBaseTemplate * pObjTop = GetTopLevelObj();
			MoveToCheck( pObjTop->GetTopPoint(), pSrc->GetChar());
		}
		return( true );
	case CIV_DUPE:
		{
			int iCount = s.GetArgVal();
			if ( iCount <= 0 ) 
				iCount = 1;
			if ( iCount > g_Cfg.m_iMaxItemComplexity )
				iCount = g_Cfg.m_iMaxItemComplexity;
			while ( iCount-- )
			{
				CItem::CreateDupeItem( this, dynamic_cast <CChar *> (pSrc), true )->MoveNearObj( this, 1 );
			}
		}
		break;
	case CIV_EQUIP:
		if ( ! pCharSrc )
			return( false );
		return pCharSrc->ItemEquip( this );
	case CIV_UNEQUIP:
		if ( ! pCharSrc )
			return( false );
		RemoveSelf();
		pCharSrc->ItemBounce(this);
		break;
	case CIV_USE:
		if ( ! pCharSrc )
			return( false );
		pCharSrc->Use_Obj( this, s.HasArgs() ? s.GetArgVal() : true, true );
		break;
	default:
		DEBUG_CHECK(0);
		return( false );
	}
	EXC_CATCH("CItem");
	return( true );
}

TRIGRET_TYPE CItem::OnTrigger( LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
{
	if ( pSrc == NULL )
	{
		pSrc = &g_Serv;
	}

	ITRIG_TYPE iAction;
	if ( ISINTRESOURCE(pszTrigName))
	{
		iAction = (ITRIG_TYPE) GETINTRESOURCE(pszTrigName);
		pszTrigName = sm_szTrigName[iAction];
	}
	else
	{
		iAction = (ITRIG_TYPE) FindTableSorted( pszTrigName, sm_szTrigName, COUNTOF(sm_szTrigName)-1 );
	}

	ASSERT( iAction < ITRIG_QTY );
	TRIGRET_TYPE		iRet = TRIGRET_RET_DEFAULT;

	// Is there trigger code in the script file ?
	// RETURN:
	//   false = continue default process normally.
	//   true = don't allow this.  (halt further processing)
	char	inLocalBlock[64];
	inLocalBlock[0] = 0;
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
	try
	{
#endif
		CItemBase *			pItemDef	= Item_GetDef();
		ASSERT( pItemDef );

		CResourceLink *		pResourceLink;
		int					i;


		// 1) Triggers installed on character, sensitive to actions on all items
		strcpy(inLocalBlock, "Char Triggers");
		CChar * pChar = pSrc->GetChar();
		if ( pChar )
		{
			// Is there an [EVENT] block call here?
			if ( iAction > XTRIG_UNKNOWN )
			{
				CGrayUID uidOldAct = pChar->m_Act_Targ;
				pChar->m_Act_Targ = GetUID();
				// iRet = pChar->OnTrigger( (CTRIG_TYPE)( (CTRIG_itemClick-CTRIG_Click)+iAction ),  pSrc, pArgs );
				iRet = pChar->OnTrigger( (CTRIG_TYPE)( (CTRIG_itemClick-1)+iAction ),  pSrc, pArgs );
				pChar->m_Act_Targ = uidOldAct;
				if ( iRet == TRIGRET_RET_TRUE )
				{
					return( iRet );	// Block further action.
				}
			}
		}


		// 2) EVENTS on the item
		strcpy(inLocalBlock, "Item Triggers");
		for ( i=0; i < m_OEvents.GetCount(); i++ )
		{
			// RES_EVENTS
			CResourceLink * pLink = m_OEvents[i];
			ASSERT(pLink);
			if ( ! pLink->HasTrigger( iAction ))
				continue;
			CResourceLock s;
			if ( ! pLink->ResourceLock( s ))
				continue;
			TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, pArgs );
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}


		// 3) TEVENTS on the item
		strcpy(inLocalBlock, "Item TEvents");
		for ( i=0; i < pItemDef->m_TEvents.GetCount(); i++ )
		{
			CResourceLink * pLink = pItemDef->m_TEvents[i];
			ASSERT(pLink);
			if ( ! pLink->HasTrigger( iAction ))
				continue;
			CResourceLock s;
			if ( ! pLink->ResourceLock( s ))
				continue;
			TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, pArgs );
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}


		// 4) TYPEDEF			// if ( m_type >= IT_TRIGGER )
		strcpy(inLocalBlock, "Typedef Triggers");
		{
			// It has an assigned trigger type.
			pResourceLink = dynamic_cast <CResourceLink *>( g_Cfg.ResourceGetDef( RESOURCE_ID( RES_TYPEDEF, GetType() )));
			if ( pResourceLink == NULL )
			{
				if ( pChar )
				{
					DEBUG_ERR(( "0%x '%s' has unhandled [TYPEDEF %d] for 0%x '%s'" DEBUG_CR, (DWORD) GetUID(), GetName(), GetType(), (DWORD) pChar->GetUID(), pChar->GetName()));
				}
				else
				{
					DEBUG_ERR(( "0%x '%s' has unhandled [TYPEDEF %d]" DEBUG_CR, (DWORD) GetUID(), GetName(), GetType() ));
				}
				m_type = Item_GetDef()->GetType();
				return( TRIGRET_RET_DEFAULT );
			}

			if ( pResourceLink->HasTrigger( iAction ))
			{
				CResourceLock s;
				if ( pResourceLink->ResourceLock(s))
				{
					iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, pArgs );
					if ( iRet == TRIGRET_RET_TRUE )
					{
						return( iRet );	// Block further action.
					}
				}
			}
		}


		// Look up the trigger in the RES_ITEMDEF. (default)
		strcpy(inLocalBlock, "Itemdef Triggers");
		pResourceLink = Base_GetDef();
		ASSERT(pResourceLink);
		if ( pResourceLink->HasTrigger( iAction ))
		{
			CResourceLock s;
			if ( pResourceLink->ResourceLock(s))
			{
				iRet = CScriptObj::OnTriggerScript( s, pszTrigName, pSrc, pArgs );
			}
		}
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
	}
	catch ( CGrayError &e )
	{
		g_Log.CatchEvent( &e, "Item Trigger in %s (%s) (UID=0%lx)", pszTrigName, inLocalBlock, (DWORD) GetUID());
	}
	catch (...)	// catch all
	{
		g_Log.CatchEvent(NULL, "Item Trigger in %s (%s) (UID=0%lx)", pszTrigName, inLocalBlock, (DWORD) GetUID());
	}
#endif

	return( iRet );
}

void CItem::DupeCopy( const CItem * pItem )
{
	// Dupe this item.

	CObjBase::DupeCopy( pItem );

	m_wDispIndex = pItem->m_wDispIndex;
	SetBase( pItem->Item_GetDef() );
	m_type = pItem->m_type;
	m_amount = pItem->m_amount;
	m_Attr  = pItem->m_Attr;
	m_uidLink = pItem->m_uidLink;

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
	m_type = IT_ANIM_ACTIVE;
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

		if ( IsItemEquipped())
		{
			if ( GetEquipLayer() == LAYER_DRAGGING )
			{
				pClient->addObjectRemove( this );
				continue;
			}
			if ( GetEquipLayer() >= LAYER_SPECIAL )	// nobody cares about this stuff.
				return;
		}
		else if ( IsItemInContainer())
		{
			CItemContainer* pCont = dynamic_cast <CItemContainer*> (GetParent());
			ASSERT(pCont);
			if ( pCont->IsAttr(ATTR_INVIS))
			{
				// used to temporary build corpses.
				pClient->addObjectRemove( this );
				continue;
			}
		}
		pClient->addItem( this );
	}
}

bool CItem::IsValidLockLink( CItem * pItemLock ) const
{
	// IT_KEY
	if ( pItemLock == NULL )
	{
		return( false );
	}
	if ( pItemLock->IsTypeLockable())
	{
		return( pItemLock->m_itContainer.m_lockUID == m_itKey.m_lockUID );
	}
	if ( CItemBase::IsID_Multi( pItemLock->GetDispID()))
	{
		// Is it linked to the item it is locking. (should be)
		// We can't match the lock uid this way tho.
		// Houses may have multiple doors. and multiple locks
		DEBUG_CHECK( pItemLock->IsType(IT_MULTI) || pItemLock->IsType(IT_SHIP));
		return( true );
	}
	// not connected to anything i recognize.
	return( false );
}

bool CItem::IsValidLockUID() const
{
	// IT_KEY
	// Keys must:
	//  1. have m_lockUID == UID of the container.
	//  2. m_uidLink == UID of the multi.

	DEBUG_CHECK( IsType(IT_KEY));
	if ( ! m_itKey.m_lockUID.IsValidUID() )	// blank
		return( false );

	// or we are a key to a multi.
	// we can be linked to the multi.
	if ( IsValidLockLink( m_itKey.m_lockUID.ItemFind()))
		return( true );
	if ( IsValidLockLink( m_uidLink.ItemFind()))
		return( true );

	return( false );
}

void CItem::ConvertBolttoCloth()
{
	if ( ! IsSameDispID( ITEMID_CLOTH_BOLT1 ))
		return;
	// Bolts of cloth are treated as 50 single pieces of cloth
	// Convert it to 50 pieces of cloth, then consume the amount we want later
	SetID( ITEMID_CLOTH1 );
	SetAmountUpdate( 50 * GetAmount());
}

int CItem::ConsumeAmount( int iQty, bool fTest )
{
	// Eat or drink specific item. delete it when gone.
	// return: the amount we used.
	if ( this == NULL )	// can use none if there is nothing? or can we use all?
		return iQty;

	int iQtyMax = GetAmount();
	if ( iQty < iQtyMax )
	{
		if ( ! fTest )
		{
			SetAmountUpdate( iQtyMax - iQty );
		}
		return( iQty );
	}

	if ( ! fTest )
	{
		SetAmount( 0 );	// let there be 0 amount here til decay.
		if ( ! IsTopLevel() || ! IsAttr( ATTR_INVIS ))	// don't delete resource locators.
		{
			Delete();
		}
	}

	return( iQtyMax );
}

SPELL_TYPE CItem::GetScrollSpell() const
{
	// Given a scroll type. what spell is this ?
	for ( int i=SPELL_Clumsy; true; i++ )
	{
		const CSpellDef * pSpellDef = g_Cfg.GetSpellDef( (SPELL_TYPE) i );
		if ( pSpellDef == NULL )
			break;
		if ( GetID() == pSpellDef->m_idScroll )
			return((SPELL_TYPE) i );
	}
	return( SPELL_NONE );
}

bool CItem::IsSpellInBook( SPELL_TYPE spell ) const
{
	//DEBUG_CHECK( IsType(IT_SPELLBOOK));

	int iSpell = spell;

	if (( spell > 100 ) && ( spell < 117 ))
	{
		if ( ! IsType(IT_SPELLBOOK_NECRO))
			return( false );

		iSpell = spell - NSPELL_Start;
	}
	else if (( spell > 200 ) && ( spell < 211 ))
	{
		if ( ! IsType(IT_SPELLBOOK_PALA))
			return( false );

		iSpell = spell - PSPELL_Start;
	}
	else
		iSpell = spell - SPELL_Clumsy;

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

int CItem::AddSpellbookSpell( SPELL_TYPE spell, bool fUpdate )
{
	// Add  this scroll to the spellbook.
	// 0 = added.
	// 1 = already here.
	// 2 = not a scroll i know about.

	if ( ! IsType( IT_SPELLBOOK ))
		return( 3 );
	if ( spell == SPELL_NONE )
		return( 2 );
	const CSpellDef * pSpellDef = g_Cfg.GetSpellDef( spell );
	if ( pSpellDef == NULL )
		return( 2 );

	if ( IsSpellInBook(spell))
	{
		return( 1 );	// already here.
	}

	// Update the spellbook
	if ( fUpdate)
	{
		CCommand cmd;
		cmd.ContAdd.m_Cmd = XCMD_ContAdd;
		cmd.ContAdd.m_UID = UID_F_ITEM + UID_O_INDEX_FREE + spell;
		cmd.ContAdd.m_id = pSpellDef->m_idScroll;
		cmd.ContAdd.m_zero7 = 0;
		cmd.ContAdd.m_amount = spell;
		cmd.ContAdd.m_x = 0x48;
		cmd.ContAdd.m_y = 0x7D;
		cmd.ContAdd.m_UIDCont = GetUID();
		cmd.ContAdd.m_wHue = HUE_DEFAULT;
		UpdateCanSee( &cmd, sizeof( cmd.ContAdd ));
	}

	// Now add to the spellbook bitmask.
	int i = spell;
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

	return( 0 );
}

int CItem::AddSpellbookScroll( CItem * pScroll )
{
	// Add  this scroll to the spellbook.
	// 0 = added.
	// 1 = already here.
	// 2 = not a scroll i know about.

	ASSERT(pScroll);
	int iRet = AddSpellbookSpell( pScroll->GetScrollSpell(), true );
	if ( iRet )
		return( iRet );
	pScroll->Delete();		// gone.
	return( 0 );
}

void CItem::Flip( LPCTSTR pCmd )
{
	// Equivelant rotational objects.
	// These objects are all the same except they are rotated.

	if ( IsTypeLit())	// m_pDef->Can( CAN_I_LIGHT ) ?
	{
		if ( ++m_itLight.m_pattern >= LIGHT_QTY )
			m_itLight.m_pattern = 0;
		Update();
		return;
	}

	// Doors are easy.
	if ( IsType( IT_DOOR ) || IsType( IT_DOOR_LOCKED ) || IsType(IT_DOOR_OPEN))
	{
		ITEMID_TYPE id = GetDispID();
		int doordir = CItemBase::IsID_Door( id )-1;
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

	if ( IsType( IT_CORPSE ))
	{
		m_itCorpse.m_facing_dir = GetDirTurn((DIR_TYPE) m_itCorpse.m_facing_dir, 1 );
		Update();
		return;
	}

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	// Try to rotate the object.
	ITEMID_TYPE id = pItemDef->GetNextFlipID( GetDispID());
	if ( id != GetDispID())
	{
		SetDispID( id );
		Update();
	}
}

bool CItem::Use_Portculis()
{
	// Set to the new z location.
	DEBUG_CHECK( m_type == IT_PORTCULIS || m_type == IT_PORT_LOCKED );

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
	DEBUG_CHECK( IsType(IT_MUSICAL) );
	const CItemBase * pItemDef = Item_GetDef();
	return( fWell ? ( pItemDef->m_ttMusical.m_iSoundGood ) : ( pItemDef->m_ttMusical.m_iSoundBad ));
}

bool CItem::IsDoorOpen() const
{
	DEBUG_CHECK( IsType(IT_DOOR) || IsType(IT_DOOR_LOCKED) || IsType(IT_DOOR_OPEN));
	return( CItemBase::IsID_DoorOpen( GetDispID()));
}

bool CItem::Use_Door( bool fJustOpen )
{
	// don't call this directly but call CChar::Use_Item() instead.
	// don't check to see if it is locked here
	// RETURN:
	//  true = open

	DEBUG_CHECK( IsType(IT_DOOR) || IsType(IT_DOOR_LOCKED) || IsType(IT_DOOR_OPEN));
	ITEMID_TYPE id = GetDispID();
	int doordir = CItemBase::IsID_Door( id )-1;
	DEBUG_CHECK( doordir >= 0 );
	if ( doordir < 0 || ! IsTopLevel())
		return( false );

	id = (ITEMID_TYPE) ( id - doordir );
	IT_TYPE typelock = m_type;

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
		DEBUG_MSG(( "Door: %d,%d" DEBUG_CR, id, doordir ));
	}

	SetDispID((ITEMID_TYPE) ( id + doordir ));
	// SetType( typelock );	// preserve the fact that it was locked.
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

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	if ( pItemDef->Can( CAN_I_REPAIR ))
		return( true );

	switch ( m_type )
	{
	case IT_CLOTHING:
	case IT_ARMOR_LEATHER:
		return( false );	// Not this way anyhow.
	case IT_SHIELD:
	case IT_ARMOR:				// some type of armor. (no real action)
		// ??? Bone armor etc is not !
		break;
	case IT_WEAPON_MACE_CROOK:
	case IT_WEAPON_MACE_PICK:
	case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case IT_WEAPON_MACE_STAFF:
	case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case IT_WEAPON_SWORD:
	case IT_WEAPON_FENCE:
	case IT_WEAPON_AXE:
		break;

	case IT_WEAPON_BOW:
		// wood Bows are not repairable !
		return( false );
	case IT_WEAPON_XBOW:
		return( true );
	default:
		return( false );
	}

	DEBUG_CHECK( IsTypeArmorWeapon());

	return( true );
}

int CItem::Armor_GetDefense() const
{
	// Get the defensive value of the armor. plus magic modifiers.
	// Subtract for low hitpoints.
	if ( ! IsTypeArmor())
		return 0;

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	int iVal = pItemDef->m_attackBase ;
	int iRepair = Armor_GetRepairPercent();
	iVal = IMULDIV( iVal, iRepair, 100 );

	// Add any magical effect.
	if ( IsAttr(ATTR_MAGIC) ) // && spell == SPELL_Enchant || SPELL_Curse
	{
		iRepair = g_Cfg.GetSpellEffect( SPELL_Enchant, m_itArmor.m_spelllevel );
		iVal += iRepair;	// may even be a negative effect ?
		if ( iVal < 0 )
			iVal = 0;
	}
	return( iVal );
}

int CItem::Weapon_GetAttack() const
{
	// Get the base attack for the weapon plus magic modifiers.
	// Subtract for low hitpoints.
	if ( ! IsTypeWeapon())	// anything can act as a weapon.
		return 1;

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	int iVal = pItemDef->m_attackBase + Calc_GetRandVal(pItemDef->m_attackRange );
	iVal = IMULDIV( iVal, Armor_GetRepairPercent(), 100 );
	if ( IsAttr(ATTR_MAGIC) && ! IsType(IT_WAND))
	{
		iVal += g_Cfg.GetSpellEffect( SPELL_Enchant, m_itArmor.m_spelllevel );
		if ( iVal < 0 )
			iVal = 0;
	}
	return( iVal );
}

SKILL_TYPE CItem::Weapon_GetSkill() const
{
	// assuming this is a weapon. What skill does it apply to.

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	if ( pItemDef->m_iSkill >= 0 && pItemDef->m_iSkill < MAX_SKILL )
		return (SKILL_TYPE) pItemDef->m_iSkill;

	switch ( pItemDef->GetType() )
	{
	case IT_WEAPON_MACE_CROOK:
	case IT_WEAPON_MACE_PICK:
	case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case IT_WEAPON_MACE_STAFF:
	case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
		return( SKILL_MACEFIGHTING );
	case IT_WEAPON_SWORD:
	case IT_WEAPON_AXE:
		return( SKILL_SWORDSMANSHIP );
	case IT_WEAPON_FENCE:
		return( SKILL_FENCING );
	case IT_WEAPON_BOW:
	case IT_WEAPON_XBOW:
		return( SKILL_ARCHERY );
	}
	return( SKILL_WRESTLING );
}

LPCTSTR CItem::Use_SpyGlass( CChar * pUser ) const
{
	// IT_SPY_GLASS
	TCHAR * pResult = Str_GetTemp();
	strcpy( pResult, "The lens are broken." );

/*
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

		if ( ptLand.IsValidPoint())
			sSearch.Format( "You see land to the %s. ", (LPCTSTR) CPointBase::sm_szDirs[ ptCoords.GetDir(ptLand) ] );
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
			pItem->m_type == IT_SHIP )
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
			sSearch.Format("You can see a %s to the %s. ", (LPCTSTR) pBoatSighted->GetName(), CPointBase::sm_szDirs[dir] );
		else
			sSearch.Format("You see many ships, one is to the %s. ", (LPCTSTR) CPointBase::sm_szDirs[dir] );
		strcat( pResult, sSearch);
	}

	if (iItemSighted) // Report item sightings, also boats beyond the boat visibility range in the radar screen
	{
		int iDist = ptCoords.GetDist(pItemSighted->GetTopPoint());
		DIR_TYPE dir = ptCoords.GetDir(pItemSighted->GetTopPoint());
		if (iItemSighted == 1)
		{
			if ( iDist > UO_MAP_VIEW_RADAR) // if beyond ship visibility in the radar window, don't be specific
				sSearch.Format("You can see something floating in the water to the %s. ", (LPCTSTR) CPointBase::sm_szDirs[ dir ] );
			else
				sSearch.Format("You can see %s to the %s.", (LPCTSTR) pItemSighted->GetNameFull(false), (LPCTSTR) CPointBase::sm_szDirs[ dir ] );
		}
		else
		{
			if ( iDist > UO_MAP_VIEW_RADAR) // if beyond ship visibility in the radar window, don't be specific
				sSearch.Format("You can see many things, one is to the %s. ", CPointBase::sm_szDirs[ dir ] );
			else
				sSearch.Format("You can see many things, one is %s to the %s. ", (LPCTSTR) pItemSighted->GetNameFull(false), (LPCTSTR) CPointBase::sm_szDirs[ dir ] );
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
		if ( pChar == NULL )
			break;
		if ( pChar == pUser )
			continue;
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
			sSearch.Format("You see a creature to the %s", CPointBase::sm_szDirs[dir] );
		else
			sSearch.Format("You can see many creatures, one is to the %s.", CPointBase::sm_szDirs[dir] );
		strcat( pResult, sSearch);
	}
	*/
	return pResult;
}

LPCTSTR CItem::Use_Sextant( CPointMap pntCoords ) const
{
	// IT_SEXTANT
	// Conversion from map square to degrees, minutes
	long lLat =  (pntCoords.m_y - g_pntLBThrone.m_y) * 360 * 60 / UO_SIZE_Y;
	long lLong = (pntCoords.m_x - g_pntLBThrone.m_x) * 360 * 60 / UO_SIZE_X_REAL;
	int iLatDeg =  lLat / 60;
	int iLatMin  = lLat % 60;
	int iLongDeg = lLong / 60;
	int iLongMin = lLong % 60;

	TCHAR * pTemp = Str_GetTemp();
	sprintf( pTemp, "%io%i'%s, %io%i'%s",
		abs(iLatDeg),  abs(iLatMin),  (lLat <= 0) ? "N" : "S",
		abs(iLongDeg), abs(iLongMin), (lLong >= 0) ? "E" : "W");
	return pTemp;
}

bool CItem::Use_Light()
{
	ASSERT( IsType(IT_LIGHT_OUT) || IsType(IT_LIGHT_LIT) );

	if ( IsType(IT_LIGHT_OUT) && IsItemInContainer())
		return( false );

	ITEMID_TYPE id = (ITEMID_TYPE) Item_GetDef()->m_ttEquippable.m_Light_ID.GetResIndex();
	if ( id == ITEMID_NOTHING )
		return( false );

	SetID( id );	// this will set the new m_typez

	if ( IsType(IT_LIGHT_LIT))
	{
		SetTimeout( 10*60*TICK_PER_SEC );
		if ( ! m_itLight.m_charges )
			m_itLight.m_charges = 20;
	}

	Sound( 0x226 );
	RemoveFromView();
	Update();
	return( true );
}

int CItem::Use_LockPick( CChar * pCharSrc, bool fTest, bool fFail )
{
	// This is the locked item.
	// Char is using a lock pick on it.
	// SKILL_LOCKPICKING
	// SKILL_MAGERY
	// RETURN:
	//  -1 not possible.
	//  0 = trivial
	//  difficulty of unlocking this (0 100)
	//

	if ( pCharSrc == NULL )
		return -1;

	if ( ! IsTypeLocked())
	{
		pCharSrc->SysMessage( "You cannot unlock that!" );
		return -1;
	}

	CChar * pCharTop = dynamic_cast <CChar*>( GetTopLevelObj());
	if ( pCharTop && pCharTop != pCharSrc )
	{
		pCharSrc->SysMessage( "You cannot unlock that where it is!" );
		return -1;
	}

	// If we have the key allow unlock using spell.
	if ( pCharSrc->ContentFindKeyFor( this ))
	{
		pCharSrc->SysMessage( "You have the key for this" );
		return( 0 );
	}

	if ( IsType(IT_DOOR_LOCKED) && g_Cfg.m_iMagicUnlockDoor != -1 )
	{
		if ( g_Cfg.m_iMagicUnlockDoor == 0 )
		{
			pCharSrc->SysMessage( "This door can only be unlocked with a key." );
			return -1;
		}

		// you can get flagged criminal for this action.
		if ( ! fTest )
		{
			pCharSrc->CheckCrimeSeen( SKILL_SNOOPING, NULL, this, "picking the locked" );
		}

		if ( Calc_GetRandVal( g_Cfg.m_iMagicUnlockDoor ))
		{
			return 10000;	// plain impossible.
		}
	}

	// If we don't have a key, we still have a *tiny* chance.
	// This is because players could have lockable chests too and we don't want it to be too easy
	if ( ! fTest )
	{
		if ( fFail )
		{
			if ( IsType(IT_DOOR_LOCKED))
			{
				pCharSrc->SysMessage( "You failed to unlock the door.");
			}
			else
			{
				pCharSrc->SysMessage( "You failed to unlock the container.");
			}
			return( -1 );
		}

		// unlock it.
		pCharSrc->Use_KeyChange( this );
	}

	return( m_itContainer.m_lock_complexity / 10 );
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

void CItem::SetTrapState( IT_TYPE state, ITEMID_TYPE id, int iTimeSec )
{
	ASSERT( IsType(IT_TRAP) || IsType(IT_TRAP_ACTIVE) || IsType(IT_TRAP_INACTIVE) );
	ASSERT( state == IT_TRAP || state == IT_TRAP_ACTIVE || state == IT_TRAP_INACTIVE );

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
	else if ( iTimeSec > 0 && iTimeSec < USHRT_MAX )
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

	SetType( state );
	SetTimeout( iTimeSec );
	Update();
}

int CItem::Use_Trap()
{
	// We activated a trap somehow.
	// We might not be close to it tho.
	// RETURN:
	//   The amount of damage.

	ASSERT( m_type == IT_TRAP || m_type == IT_TRAP_ACTIVE );
	if ( m_type == IT_TRAP )
	{
		SetTrapState( IT_TRAP_ACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wAnimSec );
	}

	if ( ! m_itTrap.m_Damage ) m_itTrap.m_Damage = 2;
	return( m_itTrap.m_Damage );	// base damage done.
}

bool CItem::SetMagicLock( CChar * pCharSrc, int iSkillLevel )
{
	if ( pCharSrc == NULL )
		return false;

	if ( IsTypeLocked())
	{
		pCharSrc->SysMessage( "It is already locked" );
		return false;
	}
	if ( ! IsTypeLockable())
	{
		// Maybe lock items to the ground ?
		pCharSrc->SysMessage( "It is not really a lockable type object." );
		return false;
	}
   if ( IsAttr( ATTR_OWNED ) )
	{
		pCharSrc->SysMessage( "You may not lock this item" );
		return false;
	}
	switch ( m_type )
	{
	// Have to check for keys here or people would be
	// locking up dungeon chests.
	case IT_CONTAINER:
		if ( pCharSrc->ContentFindKeyFor( this ))
		{
			m_type=IT_CONTAINER_LOCKED;
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
		case IT_DOOR:
		case IT_DOOR_OPEN:
			if ( pCharSrc->ContentFindKeyFor( this ))
			{
				m_type=IT_DOOR_LOCKED;
				pCharSrc->SysMessage( "You lock the door.");
			}
			else
			{
				pCharSrc->SysMessage( "You don't have the key to this door.");
				return false;
			}
			break;
		case IT_SHIP_HOLD:
			if ( pCharSrc->ContentFindKeyFor( this ))
			{
				m_type=IT_SHIP_HOLD_LOCK;
				pCharSrc->SysMessage( "You lock the hold.");
			}
			else
			{
				pCharSrc->SysMessage( "You don't have the key to the hold." );
				return false;
			}
			break;
		case IT_SHIP_SIDE:
			m_type=IT_SHIP_SIDE_LOCKED;
			pCharSrc->SysMessage( "You lock the ship.");
			break;
		default:
			pCharSrc->SysMessage( "You cannot lock that!" );
			return false;
		}

	return( true );
}

int CItem::GetBlessCurseLevel() const
{
	// RETURN: -3 to +3 as a bless / curse level.
	// NOTE: assume both are not set.
	int iLevel = 0;
	if ( IsAttr(ATTR_BLESSED))
		iLevel += 1;
	if ( IsAttr(ATTR_BLESSED2))
		iLevel += 2;
	if ( IsAttr(ATTR_CURSED))
		iLevel -= 1;
	if ( IsAttr(ATTR_CURSED2))
		iLevel -= 2;
	return( iLevel );
}

bool CItem::SetBlessCurse( int iSkillLevel, CChar * pCharSrc )
{
	// ARGS:
	//  iSkillLevel = 0-1000
	// NOTE:
	//  only gods (scripts) and gms can make level +-3.

	int iBlessCurseLevel = GetBlessCurseLevel();
	if ( ! iSkillLevel && ! iBlessCurseLevel )
		return false;

	iBlessCurseLevel = abs( iBlessCurseLevel );

	bool fCurse = ( iSkillLevel < 0 );
	iSkillLevel = abs( iSkillLevel );

	LPCTSTR pszColor = "neutral";
	bool fSpellableType = IsTypeSpellable();	// IT_ARMOR ? IT_SPELL ?

	DEBUG_CHECK(pCharSrc);
	if ( pCharSrc && ! IsMovable() && ! pCharSrc->IsPriv(PRIV_GM))
	{
		return false;
	}

	if ( fCurse )	// curse
	{
		pszColor = "darkling";
		if ( IsAttr(ATTR_BLESSED|ATTR_BLESSED2))
		{
			if ( ! fSpellableType || iSkillLevel > m_itSpell.m_spelllevel )
			{
				ClrAttr(ATTR_BLESSED);
			}
		}
		else
		{
			SetAttr(ATTR_CURSED);
		}
	}
	else	// bless
	{
		pszColor = "brilliant";
		if ( IsAttr(ATTR_CURSED|ATTR_CURSED2))	// it was cursed.
		{
			if ( ! fSpellableType || iSkillLevel > m_itSpell.m_spelllevel )
			{
				ClrAttr(ATTR_CURSED);

				// remove the curse spell. (must not be worn!)
				if ( ! IsItemEquipped() &&
					fSpellableType &&
					IsAttr( ATTR_MAGIC ) &&
					RES_GET_INDEX(m_itSpell.m_spell) == SPELL_Curse )
				{
					m_itSpell.m_spelllevel = 0;
					ClrAttr(ATTR_MAGIC);
				}
			}
		}
		else
		{
			SetAttr(ATTR_BLESSED);
		}
	}

	CGString sMsg;
	sMsg.Format( "show%s a %s glow.",
		IsItemEquipped() ? "" : "s",
		pszColor );

	Emote( sMsg );
	return( true );
}

bool CItem::OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel, CItem * pSourceItem )
{
	//
	// A spell is cast on this item.
	// ARGS:
	//  iSkillLevel = 0-1000 = difficulty. may be slightly larger . how advanced is this spell (might be from a wand)
	//

	CScriptTriggerArgs Args( (int) spell, iSkillLevel, pSourceItem );
	TRIGRET_TYPE iRet	= OnTrigger( ITRIG_SPELLEFFECT, pCharSrc, &Args );
	spell		= (SPELL_TYPE) Args.m_iN1;
	const CSpellDef * pSpellDef = g_Cfg.GetSpellDef( spell );

	switch ( iRet )
	{
	case TRIGRET_RET_TRUE:		return false;
	case TRIGRET_RET_FALSE:		if ( pSpellDef && pSpellDef->IsSpellType( SPELLFLAG_SCRIPTED ) ) return true;
	}
	
	iRet	= Spell_OnTrigger( spell, SPTRIG_EFFECT, pCharSrc, &Args );
	spell		= (SPELL_TYPE) Args.m_iN1;
	iSkillLevel	= Args.m_iN2;
	pSpellDef = g_Cfg.GetSpellDef( spell );

	switch ( iRet )
	{
	case TRIGRET_RET_TRUE:		return false;
	case TRIGRET_RET_FALSE:		if ( pSpellDef && pSpellDef->IsSpellType( SPELLFLAG_SCRIPTED ) ) return true;
	}

	if ( IsType(IT_WAND) )	// try to recharge the wand.
	{
		if ( ! m_itWeapon.m_spell || RES_GET_INDEX(m_itWeapon.m_spell) == spell )
		{
			SetAttr(ATTR_MAGIC);
			if ( ! m_itWeapon.m_spell || ( pCharSrc && pCharSrc->IsPriv( PRIV_GM )))
			{
				m_itWeapon.m_spell = spell;
				m_itWeapon.m_spelllevel = iSkillLevel;
				m_itWeapon.m_spellcharges = 0;
			}

			// Spells chance of blowing up the wand is based on total energy level.

			int ispellbase = pSpellDef->m_wManaUse;
			int iChanceToExplode = Calc_GetBellCurve( m_itWeapon.m_spellcharges+3+ispellbase/10, 2 ) / 2;
			if ( ! Calc_GetRandVal(iChanceToExplode))
			{
				// Overcharge will explode !
				Emote( "overcharged wand explodes" );
				g_World.Explode( pCharSrc, GetTopLevelObj()->GetTopPoint(),
					3, 2 + (ispellbase+iSkillLevel)/20,
					DAMAGE_MAGIC | DAMAGE_GENERAL | DAMAGE_HIT_BLUNT );
				Delete();
				return false;
			}
			else
			{
				m_itWeapon.m_spellcharges++;
			}
		}
	}

	if ( pCharSrc->GetRegion()->CheckAntiMagic(spell) )
	{
		pCharSrc->SysMessage("An anti-magic field disturbs this spell.");
		return false;
	}

	WORD uDamage = 0;
	switch ( spell )
	{
	case SPELL_Dispel_Field:
	case SPELL_Dispel:
	case SPELL_Mass_Dispel:
		switch ( GetType())
		{
		case IT_CAMPFIRE:
		case IT_FIRE:
		case IT_SPELL:
			// ??? compare the strength of the spells ?
			{
			if ( IsTopLevel())
			{
				CItem * pItem = CItem::CreateScript( ITEMID_FX_SPELL_FAIL, pCharSrc );
				pItem->MoveToDecay( GetTopPoint(), 2*TICK_PER_SEC );
			}
			Delete();
			}
			break;
		}
		break;

	case SPELL_Curse:
		// Is the item blessed ?
		// What is the chance of holding the curse ?
		SetBlessCurse( -iSkillLevel, pCharSrc );
		return true;
	case SPELL_Bless:
		// Is the item cursed ?
		// What is the chance of holding the blessing ?
		SetBlessCurse( iSkillLevel, pCharSrc );
		return true;
	case SPELL_Incognito:
		ClrAttr(ATTR_IDENTIFIED);
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
		if ( ! SetMagicLock( pCharSrc, iSkillLevel ))
			return( false );
		break;

	case SPELL_Unlock:
		{
			int iDifficulty = Use_LockPick( pCharSrc, true, false );
			if ( iDifficulty < 0 )
				return( false );
			bool fSuccess = g_Cfg.Calc_SkillCheck( iSkillLevel, iDifficulty );
			Use_LockPick( pCharSrc, false, ! fSuccess );
			return fSuccess;
		}

	case SPELL_Mark:
		if ( pCharSrc == NULL )
			return false;

		if ( ! pCharSrc->IsPriv(PRIV_GM))
		{
			if ( ! IsType(IT_RUNE) && ! IsType(IT_TELEPAD) )
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

		m_itRune.m_pntMark = pCharSrc->GetTopPoint();
		if ( IsType(IT_RUNE) )
		{
			m_itRune.m_Strength = pSpellDef->m_Effect.GetLinear( iSkillLevel );
			SetName( pCharSrc->m_pArea->GetName() );
		}
		break;

	case SPELL_Resurrection:
		// Should be a corpse .
		if ( IsType( IT_CORPSE ))
		{
			CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*>(this);
			ASSERT(pCorpse);

			// If the corpse is attached to a player or other disconnected person then yank them back.
			CChar * pChar = pCorpse->m_uidLink.CharFind();
			if ( pChar )
			{
				pChar->Spell_Resurrection( (pCharSrc && pCharSrc->IsPriv(PRIV_GM)) ? -1 : 0, pCorpse );
			}
			else
			{
				// Just an NPC corpse I guess.
				pChar = CChar::CreateBasic(m_itCorpse.m_BaseID);
				if ( pChar == NULL )
					return( false );
				pChar->NPC_LoadScript(false);
				pChar->RaiseCorpse(pCorpse);
			}
		}
		break;
	}

	// ??? Potions should explode when hit (etc..)
	if ( pSpellDef->IsSpellType( SPELLFLAG_HARM ))
	{
		OnTakeDamage( 1, pCharSrc, DAMAGE_HIT_BLUNT | DAMAGE_MAGIC | uDamage );
	}

	return( true );
}

int CItem::Armor_GetRepairPercent() const
{

	if ( !m_itArmor.m_Hits_Max || ( m_itArmor.m_Hits_Max < m_itArmor.m_Hits_Cur )) return( 100 );
 	return( IMULDIV( m_itArmor.m_Hits_Cur, 100, m_itArmor.m_Hits_Max ));
}

LPCTSTR CItem::Armor_GetRepairDesc() const
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
	// RETURN:
	//  Amount of damage done.
	//  INT_MAX = destroyed !!!
	//  -1 = invalid target ?

	if ( iDmg <= 0 )
		return( 0 );

	CScriptTriggerArgs Args( iDmg, (int) uType );
	if ( OnTrigger( ITRIG_DAMAGE, pSrc, &Args ) == TRIGRET_RET_TRUE )
	{
		return( 0 );
	}

	switch ( GetType())
	{
	case IT_CLOTHING:
		if ( ( uType & DAMAGE_FIRE ) &&
			! IsAttr( ATTR_MAGIC|ATTR_NEWBIE|ATTR_MOVE_NEVER ))
		{
			// normal cloth takes special damage from fire.
			goto forcedamage;
		}
		break;

	case IT_WEAPON_ARROW:
	case IT_WEAPON_BOLT:
		if ( iDmg == 1 )
		{
			// Miss - They will ussually survive.
			if ( Calc_GetRandVal(5))
				return( 0 );
		}
		else
		{
			// Must have hit.
			if ( ! Calc_GetRandVal(3))
				return( 1 );
		}
		Delete();
		return INT_MAX;

	case IT_POTION:
		if ( uType & ( DAMAGE_HIT_BLUNT|DAMAGE_HIT_PIERCE|DAMAGE_HIT_SLASH|DAMAGE_GOD|DAMAGE_MAGIC|DAMAGE_FIRE ))
		{
			if ( RES_GET_INDEX(m_itPotion.m_Type) == SPELL_Explosion )
			{
				g_World.Explode( pSrc, GetTopLevelObj()->GetTopPoint(), 3,
					g_Cfg.GetSpellEffect( SPELL_Explosion, m_itPotion.m_skillquality ),
					DAMAGE_GENERAL | DAMAGE_HIT_BLUNT );
				Delete();
				return( INT_MAX );
			}
		}
		return( 1 );

	case IT_WEB:
		if ( ! ( uType & (DAMAGE_FIRE|DAMAGE_HIT_BLUNT|DAMAGE_HIT_SLASH|DAMAGE_GOD)))
		{
			if ( pSrc ) pSrc->SysMessage( "You have no effect on the web" );
			return( 0 );
		}

		iDmg = Calc_GetRandVal( iDmg ) + 1;
		if ( iDmg > m_itWeb.m_Hits_Cur || ( uType & DAMAGE_FIRE ))
		{
			if ( pSrc ) pSrc->SysMessage( "You destroy the web" );
			if ( Calc_GetRandVal( 2 ) || ( uType & DAMAGE_FIRE ))
			{
				Delete();
				return( INT_MAX );
			}
			SetID( ITEMID_REAG_SS );
			Update();
			return( 2 );
		}

		if ( pSrc ) pSrc->SysMessage( "You weaken the web" );
		m_itWeb.m_Hits_Cur -= iDmg;
		return( 1 );
	}

	// Break armor etc..
	if ( IsTypeArmorWeapon() && ( uType & ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH | DAMAGE_GOD|DAMAGE_MAGIC|DAMAGE_FIRE )))
	{
		// Server option to slow the rate ???
		if ( Calc_GetRandVal( m_itArmor.m_Hits_Max * 16 ) > iDmg )
		{
			return 1;
		}

forcedamage:

		// describe the damage to the item.

		CChar * pChar = dynamic_cast <CChar*> ( GetTopLevelObj());

		if ( m_itArmor.m_Hits_Cur <= 1 )
		{
			m_itArmor.m_Hits_Cur = 0;
			Emote( "is destroyed" );
			Delete();
			return( INT_MAX );
		}

		m_itArmor.m_Hits_Cur --;

		if ( pSrc )	// tell hitter they scored !
		{
			CGString sMsg;
			if ( pChar && pChar != pSrc )
			{
				sMsg.Format( "You damage %s's %s", (LPCTSTR) pChar->GetName(), (LPCTSTR) GetName());
			}
			else
			{
				sMsg.Format( "You damage the %s", (LPCTSTR) GetName());
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
					sMsg.Format( "Your %s is damaged and looks %s", (LPCTSTR) GetName(), (LPCTSTR) Armor_GetRepairDesc());
				}
			}
			if ( sMsg.IsEmpty())
			{
				sMsg.Format( "Your %s may have been damaged", (LPCTSTR) GetName());
			}
			pChar->SysMessage( sMsg );
		}
		return( 2 );
	}

	// don't know how to calc damage for this.
	return( 0 );
}

bool CItem::OnExplosion()
{
	// IT_EXPLOSION
	// Async explosion.
	// RETURN: true = done. (delete the animation)

	ASSERT( IsTopLevel());
	ASSERT( m_type == IT_EXPLOSION );

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
	SetDecayTime( 3 * TICK_PER_SEC );
	return( false );	// don't delete it yet.
}

bool CItem::IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwArg )
{
	// Check for all the matching special cases.
	// ARGS:
	//  dwArg = specific key or map .

	if ( GetAmount() == 0 )
		return( false );	// does not count for any matches.

	CItemBase * pItemDef = Item_GetDef();
	ASSERT(pItemDef);

	if ( rid == pItemDef->GetResourceID())
		return( true );

	RES_TYPE restype = rid.GetResType();
	int index = rid.GetResIndex();

	switch ( restype )
	{
	case RES_TYPEDEF:
		if ( ! IsType( (IT_TYPE) index ))
			return( false );

		if ( dwArg )
		{
			switch ( index )
			{
			case IT_RESEARCH_ITEM:
				{
					if ( LOWORD(m_itResearchItem.m_ItemID) != (ITEMID_TYPE) dwArg )
						return false;
					if ( m_itResearchItem.m_iCompleted < 1000 )
						return false;
				}
				break;
			case IT_MAP:
				// matching maps.
				if ( LOWORD(dwArg) != m_itMap.m_top ||
					HIWORD(dwArg) != m_itMap.m_left )
				{
					return( false );
				}
				break;
			case IT_KEY:
				if ( ! IsKeyLockFit( dwArg ))
					return( false );
				break;
			}
		}
		return( true );

	case RES_ITEMDEF:

		if ( GetID() == index )
			return true;

		if ( g_Cfg.IsSetEF( EF_Item_Strict_Comparison ) )
			return false;

		switch ( index )
		{
		case ITEMID_CLOTH1:
			if ( IsType(IT_CLOTH) || IsType(IT_CLOTH_BOLT))
			{
				ConvertBolttoCloth();
				return( true );
			}
			break;
		case ITEMID_LEATHER_1:
		case ITEMID_HIDES:
			if ( IsType( IT_HIDE ) || IsType( IT_LEATHER ))
			{
				// We should be able to use leather in place of hides.
				return( true );
			}
			break;
		case ITEMID_LOG_1:
		case ITEMID_LUMBER1:
			if ( IsType(IT_LOG) || IsType(IT_LUMBER))
			{
				return( true );
			}
			break;
		case ITEMID_Arrow:
			if ( IsType(IT_WEAPON_ARROW))
			{
				return( true );
			}
			break;
		case ITEMID_XBolt:
			if ( IsType(IT_WEAPON_BOLT))
			{
				return( true );
			}
			break;
		}
		break;
	}

	return( false );
}

bool CItem::OnTick()
{
	// Timer expired. Time to do something.
	// RETURN: false = delete it.

	SetTimeout(-1);
	TRIGRET_TYPE iRet = OnTrigger( ITRIG_TIMER, &g_Serv );
	if ( iRet == TRIGRET_RET_TRUE )
		return true;

	switch ( m_type )
	{
	case IT_LIGHT_LIT:
		// use up the charges that this has .
		if ( m_itLight.m_charges > 1 )
		{
			if ( m_itLight.m_charges == USHRT_MAX )
			{
				return true;
			}
			m_itLight.m_charges --;
			SetTimeout( 10*60*TICK_PER_SEC );
		}
		else
		{
			// Torches should just go away but lanterns etc should not.
			Emote( "burn out" );
			CItemBase * pItemDef = Item_GetDef();
			ITEMID_TYPE id = (ITEMID_TYPE) pItemDef->m_ttEquippable.m_Light_Burnout.GetResIndex();
			if ( ! id )	// burn out and be gone.
			{
				return( false );
			}
			if ( id == GetID())
			{
				// It really has infinite charges I guess.
				m_itLight.m_charges = USHRT_MAX;
			}
			else
			{
				// Transform to the next shape.
				m_itLight.m_charges = 0;
				SetID(id);
			}
			Update();
		}
		return true;

	case IT_SHIP_PLANK:
		Ship_Plank( false );
		return true;

	case IT_EXPLOSION:
		if ( OnExplosion())
			break;
		return true;

	case IT_METRONOME:
		Speak( "Tick" );
		SetTimeout( 5*TICK_PER_SEC );
		return true;

	case IT_TRAP_ACTIVE:
		SetTrapState( IT_TRAP_INACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wAnimSec );
		return( true );

	case IT_TRAP_INACTIVE:
		// Set inactive til someone triggers it again.
		if ( m_itTrap.m_fPeriodic )
		{
			SetTrapState( IT_TRAP_ACTIVE, m_itTrap.m_AnimID, m_itTrap.m_wResetSec );
		}
		else
		{
			SetTrapState( IT_TRAP, GetDispID(), -1 );
		}
		return true;

	case IT_ANIM_ACTIVE:
		// reset the anim
		SetDispID( m_itAnim.m_PrevID );
		m_type = m_itAnim.m_PrevType;
		SetTimeout( -1 );
		Update();
		return true;

	case IT_DOOR:
	case IT_DOOR_OPEN:
	case IT_DOOR_LOCKED:	// Temporarily opened locked door.
		// Doors should close.
		Use_Door( false );
		return true;

	case IT_WAND:
		// Magic devices.
		// Will regen over time ??
#if 0
		if ( IsAttr(ATTR_MAGIC ))
		{
			if ( m_itWeapon.m_charges < m_itWeapon.m_skilllevel/10 )
				m_itWeapon.m_charges++;
			SetTimeout( 30*60*TICK_PER_SEC );
			return true;
		}
#endif

		break;

	case IT_POTION:
		// This is a explode potion ?
		if ( RES_GET_INDEX(m_itPotion.m_Type) == SPELL_Explosion )
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
				CObjBase* pObj = STATIC_CAST <CObjBase*>(GetTopLevelObj());
				ASSERT(pObj);
				pObj->Speak( sMsg );
				SetTimeout( TICK_PER_SEC );
			}
			return true;
		}
		break;

	case IT_SPAWN_CHAR:
		// Spawn a creature if we are under count.
	case IT_SPAWN_ITEM:
		// Spawn an item.
		Spawn_OnTick( true );
		return true;

	case IT_CROPS:
	case IT_FOLIAGE:
		if ( Plant_OnTick())
			return true;
		break;

	case IT_BEE_HIVE:
		// Regenerate honey count
		if ( m_itBeeHive.m_honeycount < 5 )
			m_itBeeHive.m_honeycount++;
		SetTimeout( 15*60*TICK_PER_SEC );
		return true;

	case IT_CAMPFIRE:
		if ( GetID() == ITEMID_EMBERS )
			break;
		SetID( ITEMID_EMBERS );
		SetDecayTime( 2*60*TICK_PER_SEC );
		Update();
		return true;

	case IT_SIGN_GUMP:	// signs never decay
		return( true );
	}

#if 0
	if ( IsAttr(ATTR_MOVE_NEVER) && ! IsAttr(ATTR_DECAY|ATTR_CAN_DECAY))
	{
		// Just a resource tracking hit on an existing dynamic object.
		// IT_WATER || IT_ROCK || IT_TREE || IT_GRASS = World bits normally DO decay
		SetTimeout( -1 );
		SetAmount( 0 );	// regen resources ?
		return true;
	}
#endif

	if ( IsAttr(ATTR_DECAY ))
	{
		if ( g_Log.IsLogged( LOGL_TRACE ))
		{
			DEBUG_MSG(( "Time to delete item '%s'" DEBUG_CR, GetName()));
		}
		// Spells should expire and clean themselves up.
		// Ground items rot... etc
		return false;	// delete it.
	}

	// NOTE: scripts might come here to default delete items we no longer want ?
	if ( iRet == TRIGRET_RET_FALSE )
		return false;

	DEBUG_ERR(( "Timer expired without DECAY flag '%s'?" DEBUG_CR, GetName()));
	return( true );
}

