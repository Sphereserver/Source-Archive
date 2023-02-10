//
// CChar.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.

bool CChar::IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwAmount )
{
	return IsResourceMatch( rid, dwAmount, 0 );
}


bool CChar::IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwAmount, DWORD dwArgResearch )
{
	// Is the char a match for this test ?
	switch ( rid.GetResType())
	{
	case RES_SKILL:
		// Do i have this skill level at least ?
		// A min skill is required.
		if ( Skill_GetBase((SKILL_TYPE) rid.GetResIndex()) < dwAmount )
			return( false );
		return( true );

	case RES_RACECLASS:
		// Am i this type of char race ?
		break;

	case RES_CHARDEF:
		// Am i this type of char ?
		{
			CCharBase * pCharDef = Char_GetDef();
			ASSERT(pCharDef);
			if ( pCharDef->GetResourceID() == rid )
				return( true );
		}
		break;
	case RES_SPEECH:	// do i have this speech ?
		if ( m_pNPC != NULL )
		{
			if ( m_pNPC->m_Speech.FindResourceID( rid ) >= 0 )
				return( true );
			CCharBase * pCharDef = Char_GetDef();
			ASSERT(pCharDef);
			if ( pCharDef->m_Speech.FindResourceID( rid ) >= 0 )
				return( true );
		}
		break;
	case RES_EVENTS:	// do i have these events ?
		{
			if ( m_OEvents.FindResourceID( rid ) >= 0 )
				return( true );
			if ( m_pNPC != NULL )
			{
				CCharBase * pCharDef = Char_GetDef();
				ASSERT(pCharDef);
				if ( pCharDef->m_TEvents.FindResourceID( rid ) >= 0 )
					return( true );
			}
		}
		break;
	case RES_TYPEDEF:
		// Do i have these in my posession ?
		if ( ! ContentConsume( rid, dwAmount, true, dwArgResearch ) )
		{
			return( true );
		}
		return( false );
	case RES_ITEMDEF:
		// Do i have these in my posession ?
		if ( ! ContentConsume( rid, dwAmount, true ) )
		{
			return( true );
		}
		return( false );

	default:
		// No idea.
		return( false );
	}
	return( false );
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
		if ( ! NPC_IsVendor())
			return( NULL );
 		id = ITEMID_VENDOR_BOX;
		break;

	case LAYER_BANKBOX:
	default:
		id = ITEMID_BANK_BOX;
		layer = LAYER_BANKBOX;
		break;
	}

	CItem * pItemTest = LayerFind( layer );
	CItemContainer * pBankBox = dynamic_cast <CItemContainer *>(pItemTest);
	if ( pBankBox == NULL && ! g_Serv.IsLoading())
	{
		if ( pItemTest )
		{
			DEBUG_ERR(( "Junk in bank box layer %d!\n", layer ));
			DEBUG_CHECK( ! pItemTest->IsWeird());
			pItemTest->Delete();
		}

		// Give them a bank box if not already have one.
		pBankBox = dynamic_cast <CItemContainer *>( CItem::CreateScript( id, this ));
		ASSERT(pBankBox);
		pBankBox->SetAttr(ATTR_NEWBIE | ATTR_MOVE_NEVER);
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

TRIGRET_TYPE CChar::OnCharTrigForLayerLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, LAYER_TYPE layer )
{
	CScriptLineContext StartContext = s.GetContext();
	CScriptLineContext EndContext = StartContext;

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->GetEquipLayer() == layer )
		{
			TRIGRET_TYPE iRet = pItem->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
			EndContext = s.GetContext();
			s.SeekContext( StartContext );
		}
	}
	if ( EndContext.m_lOffset <= StartContext.m_lOffset )
	{
		// just skip to the end.
		TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
	}
	else
	{
		s.SeekContext( EndContext );
	}
	return( TRIGRET_ENDIF );
}

int CChar::GetWeightLoadPercent( int iWeight ) const
{
	// Get a percent of load.
	if ( IsPriv(PRIV_GM))
		return( 1 );
	int	MaxCarry = g_Cfg.Calc_MaxCarryWeight(this);
	if ( !MaxCarry ) return 1000;	// suppose self extra-overloaded
	return( IMULDIV( iWeight, 100, MaxCarry));
}

bool CChar::CanCarry( const CItem * pItem ) const
{
	if ( IsPriv(PRIV_GM))
		return( true );

	int iMaxWeight = g_Cfg.Calc_MaxCarryWeight(this);

	// We are already carrying it ?
	const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
	if ( this == pObjTop )
	{
		if ( GetTotalWeight() > iMaxWeight )
			return( false );
	}
	else
	{
		int iWeight = pItem->GetWeight();
		if ( GetTotalWeight() + iWeight > iMaxWeight )
			return( false );
	}

	return( true );
}

LAYER_TYPE CChar::CanEquipLayer( CItem * pItem, LAYER_TYPE layer, CChar * pCharMsg, bool fTest )
{
	// This takes care of any conflicting items in the slot !
	// NOTE: Do not check to see if i can pick this up or steal this etc.
	// LAYER_NONE = can't equip this .

	ASSERT(pItem);

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	const CItemBase * pItemDef = pItem->Item_GetDef();
	if ( layer >= LAYER_QTY )
	{
		layer = pItemDef->GetEquipLayer();
		if ( pItemDef->IsTypeEquippable() && CItemBase::IsVisibleLayer( layer ))
		{
			if ( ( m_pPlayer || fTest )	// test for PCs only or if requested
			&& pItemDef->m_ttEquippable.m_StrReq
			&& Stat_GetAdjusted(STAT_STR) < pItemDef->m_ttEquippable.m_StrReq )
			{
				SysMessagef( "Not strong enough to equip %s.", (LPCTSTR) pItem->GetName());
				if ( pCharMsg != this )
				{
					pCharMsg->SysMessagef( "Not strong enough to equip %s.", (LPCTSTR) pItem->GetName());
				}
				return LAYER_NONE;	// can't equip stuff.
			}

			if ( pItem->GetParent() == this && pItem->GetEquipLayer() == layer )
			{
				return layer;
			}
		}
	}
	else
	{
		// Not a visible item LAYER_TYPE.
		if ( pItem->GetParent() == this && pItem->GetEquipLayer() == layer )
		{
			return layer;
		}
	}

	CItem * pItemPrev = NULL;
	switch ( layer )
	{
	case LAYER_NONE:
	case LAYER_SPECIAL:
		switch ( pItem->GetType() )
		{
		case IT_EQ_TRADE_WINDOW:
		case IT_EQ_MEMORY_OBJ:
		case IT_EQ_NPC_SCRIPT:
		case IT_EQ_SCRIPT:
			// We can have multiple items of these.
			return LAYER_SPECIAL;
		}
		return( LAYER_NONE );	// not legal !
	case LAYER_HAIR:
		if ( ! pItem->IsType(IT_HAIR))
		{
			goto cantequipthis;
		}
		break;
	case LAYER_BEARD:
		if ( ! pItem->IsType(IT_BEARD))
		{
			goto cantequipthis;
		}
		break;
	case LAYER_PACK:
		if ( ! pItem->IsType(IT_CONTAINER))
		{
			goto cantequipthis;
		}
		break;
	case LAYER_BANKBOX:
		if ( ! pItem->IsType(IT_EQ_BANK_BOX))
		{
			goto cantequipthis;
		}
		break;
	case LAYER_HORSE:
		// Only humans can ride horses !?
		if ( ! pItem->IsType(IT_EQ_HORSE) || ! IsHuman())
		{
			goto cantequipthis;
		}
		break;
	case LAYER_HAND1:
	case LAYER_HAND2:
		if ( ! pItemDef->IsTypeEquippable())
		{
			goto cantequipthis;
		}
		if ( ! pCharDef->Can( CAN_C_USEHANDS ))
		{
			goto cantequipthis;
		}
		if ( layer == LAYER_HAND2 )
		{
			// Is this a 2 handed weapon ? shields and lights are ok
			if ( pItem->IsTypeWeapon() || pItem->IsType(IT_FISH_POLE))
			{
				// Make sure the other hand is not full.
				pItemPrev = LayerFind( LAYER_HAND1 );
			}
		}
		else
		{
			// do i have a 2 handed weapon already equipped ? shields and lights are ok
			pItemPrev = LayerFind( LAYER_HAND2 );
			if ( pItemPrev != NULL )
			{
				if ( ! pItemPrev->IsTypeWeapon() && ! pItemPrev->IsType(IT_FISH_POLE))
				{
					pItemPrev = NULL;	// must be a shield or light.
				}
			}
		}
		break;


	case LAYER_HIDDEN:
		// DEBUG_ERR(( "ALERT: Weird layer %d used for '%s' check this\n", LAYER_HIDDEN, pItem->GetResourceName()));
	case LAYER_COLLAR:
	case LAYER_RING:
	case LAYER_EARS:	// anyone can use these !
	case LAYER_LIGHT:
		break;

	default:
		// Can i equip this with my body type ?
		if ( CItemBase::IsVisibleLayer( layer ))
		{
			if ( ! pCharDef->Can( CAN_C_EQUIP ))
			{
				// some creatures can equip certain special items ??? (orc lord?)
cantequipthis:
				if ( pCharMsg)
				{
					pCharMsg->SysMessage( "You can't equip this." );
				}
				if ( g_Log.IsLogged( LOGL_TRACE ) || g_Serv.IsLoading())
				{
					DEBUG_ERR(( "ContentAdd Creature '%s' can't equip %d, id='%s' '%s'\n", GetResourceName(), layer,  pItem->GetResourceName(), pItem->GetName()));
				}
				return LAYER_NONE;	// can't equip stuff.
			}
		}
		break;
	}


	// Check for objects already in this slot. Deal with it.
	if ( pItemPrev == NULL )
	{
		pItemPrev = LayerFind( layer );
	}
	if ( pItemPrev != NULL )
	{
		switch ( layer )
		{
		case LAYER_PACK:
			// this should not happen.
			// Put it in my main pack.
			// DEBUG_CHECK( layer != LAYER_PACK );
			return LAYER_NONE;
		case LAYER_HORSE:
			// this should not happen.
			DEBUG_CHECK( layer != LAYER_HORSE );
			if ( ! fTest )
			{
				ItemBounce( pItemPrev );
			}
			break;
		case LAYER_DRAGGING:	// drop it.
			if ( ! fTest )
			{
				ItemBounce( pItemPrev );
			}
			break;
		case LAYER_BEARD:
		case LAYER_HAIR:
			if ( ! fTest )
			{
				pItemPrev->Delete();
			}
			break;
		default:
			if ( layer >= LAYER_SPELL_STATS )
			{
				// Magic spell slots just get bumped.
				pItemPrev->Delete();
				break;
			}
			// DEBUG_ERR(( "LayerAdd Layer %d already used\n", layer ));
			// Force the current layer item into the pack ?
			if ( ! CanMove( pItemPrev, true ))
			{
				return LAYER_NONE;
			}
			if ( ! fTest )
			{
				ItemBounce( pItemPrev );
			}
			break;
		}
	}

	return( layer );
}

bool CChar::CheckCorpseCrime( const CItemCorpse *pCorpse, bool fLooting, bool fTest )
{
	// fLooting = looting as apposed to carving.
	// RETURN: true = criminal act !

	if ( pCorpse == NULL )
		return( false );
	CChar * pCharGhost = pCorpse->m_uidLink.CharFind();
	if ( pCharGhost == NULL )
		return( false );
	if ( pCharGhost == this )	// ok but strange to carve up your own corpse.
		return( false );
	if ( ! g_Cfg.m_fLootingIsACrime )
		return( false );

	// It's ok to loot a guild member !
	NOTO_TYPE noto = pCharGhost->Noto_GetFlag( this, false );
	switch (noto)
	{
	case NOTO_NEUTRAL:		// animals etc.
	case NOTO_GUILD_SAME:
	case NOTO_CRIMINAL:
	case NOTO_GUILD_WAR:
	case NOTO_EVIL:
		return( false );	// not a crime.
	}

	if ( ! fTest )
	{
		// Anyone see me do this ?
		CheckCrimeSeen( SKILL_NONE, pCharGhost, pCorpse, fLooting ? "looting" : NULL );
		Noto_Criminal();
	}

	return true; // got flagged
}

CItemCorpse * CChar::FindMyCorpse( int iRadius ) const
{
	// If they are standing on there own corpse then res the corpse !
	CWorldSearch Area( GetTopPoint(), iRadius );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			break;
		if ( ! pItem->IsType( IT_CORPSE ))
			continue;
		CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pItem);
		if ( pCorpse == NULL )
			continue;
		if ( pCorpse->m_uidLink != GetUID())
			continue;
		// skip "Body of " but must have same name
		//if ( strcmp( GetName(), pCorpse->GetName() + 8 ))
		//	continue;
		// not morphed type.
		if ( pCorpse->m_itCorpse.m_BaseID != m_prev_id )
			break;
		return( pCorpse );
	}
	return( NULL );
}

int CChar::GetHealthPercent() const
{
	if ( !Stat_GetAdjusted(STAT_STR) ) return 0;
	return( IMULDIV( Stat_GetVal(STAT_STR), 100, Stat_GetAdjusted(STAT_STR)));
}

bool CChar::IsSwimming() const
{
	// Am i in/over/slightly under the water now ?
	// NOTE: This is a bit more complex because we need to test if we are slightly under water.

	CPointMap ptTop = GetTopPoint();

#if 1
	CPointMap pt = g_World.FindItemTypeNearby( ptTop, IT_WATER );
	if ( ! pt.IsValidPoint())
		return( false );	// no water here.

	int iDistZ = ptTop.m_z - pt.m_z;
	if ( iDistZ < -PLAYER_HEIGHT )
	{
		// standing far under the water some how.
		return( false );
	}
	if ( iDistZ <= 0 )
	{
		// we are in or below the water.
		return( true );
	}

	// Is there a solid surface under us ?
	WORD wBlockFlags = GetMoveBlockFlags();
	int iHeightZ = g_World.GetHeightPoint( ptTop, wBlockFlags, true );
	if ( iHeightZ == pt.m_z )
	{
		return( true );
	}
#else

	WORD wBlockFlags = GetMoveBlockFlags();
	CGrayMapBlockState block( wBlockFlags, ptTop.m_z, PLAYER_HEIGHT );
	g_World.GetHeightPoint( ptTop, block, true );

	if ( ! ( wBlockFlags & CAN_C_FLY ))	// none of this really matters if i could just fly.
	{

	}

#endif

	return( false );
}

NPCBRAIN_TYPE CChar::GetNPCBrain( bool fDefault ) const
{
/*
	if ( !m_pNPC )
		return (NPCBRAIN_TYPE) 0;

	if ( m_pNPC->m_Brain && !fDefault )
		return	m_pNPC->m_Brain;
*/
	// return 1 for animal, 2 for monster, 3 for NPC humans and PCs
	// For tracking and other purposes.

	// handle the exceptions.
	CREID_TYPE id = GetDispID();
	if ( id >= CREID_MAN )
	{
		if ( id == CREID_BLADES || id == CREID_VORTEX )
			return NPCBRAIN_BESERK;
		return( NPCBRAIN_HUMAN );
	}
	if ( id >= CREID_HORSE1 )
		return( NPCBRAIN_ANIMAL );
	switch ( id )
	{
	case CREID_EAGLE:
	case CREID_BIRD:
	case CREID_GORILLA:
	case CREID_Snake:
	case CREID_Dolphin:
	case CREID_Giant_Toad:	// T2A 0x50 = Giant Toad
	case CREID_Bull_Frog:	// T2A 0x51 = Bull Frog
		return NPCBRAIN_ANIMAL;
	}
	return( NPCBRAIN_MONSTER );
}

LPCTSTR CChar::GetPronoun() const
{
	switch ( GetDispID())
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "he" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "she" );
	default:
		return( "it" );
	}
}

LPCTSTR CChar::GetPossessPronoun() const
{
	switch ( GetDispID())
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "his" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "her" );
	default:
		return( "it's" );
	}
}

BYTE CChar::GetModeFlag( bool fTrueSight ) const
{
	BYTE mode = 0;
	if ( IsStatFlag( STATF_Poisoned ))
		mode |= CHARMODE_POISON;
	if ( IsStatFlag( STATF_War ))
		mode |= CHARMODE_WAR;
//	if ( IsStatFlag( STATF_Freeze|STATF_Sleeping|STATF_Hallucinating|STATF_Stone ))
//		mode |= CHARMODE_YELLOW;
	if ( ! fTrueSight && IsStatFlag( STATF_Insubstantial | STATF_Invisible | STATF_Hidden | STATF_Sleeping ))	// if not me, this will not show up !
		mode |= CHARMODE_INVIS;
	return( mode );
}

BYTE CChar::GetLightLevel() const
{
	// Get personal light level.

	if ( IsStatFlag( STATF_DEAD ) || IsPriv(PRIV_DEBUG))	// dead don't need light.
		return( LIGHT_BRIGHT + 1 );
	if ( IsStatFlag( STATF_Sleeping ) && ! IsPriv( PRIV_GM ))	// eyes closed.
		return( LIGHT_DARK/2 );
	if ( IsStatFlag( STATF_NightSight ))
		return( LIGHT_BRIGHT );
	return( GetTopSector()->GetLight());
}

CItem * CChar::GetSpellbook( SPELL_TYPE iSpell ) const
{
	CItem * pBook;

	// Look for the equipped spellbook first.
	if ( ( iSpell > 100 ) && ( iSpell < 117 ) )
		pBook = ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK_NECRO), 0, 1 );
	else if ( ( iSpell > 200 ) && ( iSpell < 211 ) )
		pBook = ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK_PALA), 0, 1 );
	else
		pBook = ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK), 0, 1 );

	if ( pBook != NULL )
		return( pBook );
	// Look in the top level of the pack only.
	const CItemContainer * pPack = GetPack();
	if ( pPack )
	{
		if ( ( iSpell > 100 ) && ( iSpell < 117 ) )
			pBook = pPack->ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK_NECRO), 0, 1 );
		else if ( ( iSpell > 200 ) && ( iSpell < 211 ) )
			pBook = pPack->ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK_PALA), 0, 1 );
		else
			pBook = pPack->ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK), 0, 1 );

		if ( pBook != NULL )
			return( pBook );
	}
	// Sorry no spellbook found.
	return( NULL );
}

int CChar::Food_GetLevelPercent() const
{
	int	max	= Stat_GetMax(STAT_FOOD);
	if ( max == 0 )
		return 100;
	else
		return IMULDIV( Stat_GetVal(STAT_FOOD), 100, max );
}

LPCTSTR CChar::Food_GetLevelMessage( bool fPet, bool fHappy ) const
{
	int	max	= Stat_GetMax(STAT_FOOD);
	if ( max == 0)
		return "unaffected by hunger";
	int index = IMULDIV(  Stat_GetVal(STAT_FOOD), 8, max );

	if ( fPet )
	{
		static LPCTSTR const sm_szPetHunger[] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_1 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_2 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_3 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_4 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_5 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_6 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_7 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_HAPPY_8 ),
		};
		static LPCTSTR const sm_szPetHappy[] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_1 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_2 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_3 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_4 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_5 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_6 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_7 ),
			g_Cfg.GetDefaultMsg( DEFMSG_PET_FOOD_8 ),
		};

		if ( index >= COUNTOF(sm_szPetHunger)-1 )
			index = COUNTOF(sm_szPetHunger)-1;

		return( fHappy ? sm_szPetHappy[index] : sm_szPetHunger[index] );
	}

	static LPCTSTR const sm_szFoodLevel[] =
	{
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_1 ),			// "weak with hunger",
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_2 ),
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_3 ),
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_4 ),	// "a might pekish",
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_5 ),
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_6 ),
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_7 ),
		g_Cfg.GetDefaultMsg( DEFMSG_FOOD_LVL_8 ),
	};

	if ( index >= COUNTOF(sm_szFoodLevel)-1 )
		index = COUNTOF(sm_szFoodLevel)-1;

	return( sm_szFoodLevel[ index ] );
}

int CChar::Food_CanEat( CObjBase * pObj ) const
{
	// Would i want to eat this creature ? hehe
	// would i want to eat some of this item ?
	// 0 = not at all.
	// 10 = only if starving.
	// 20 = needs to be prepared.
	// 50 = not bad.
	// 75 = yummy.
	// 100 = my favorite (i will drop other things to get this).
	//

	CCharBase* pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	int iRet = pCharDef->m_FoodType.FindResourceMatch( pObj );
	if ( iRet >= 0 )
	{
		return( pCharDef->m_FoodType[iRet].GetResQty()); // how bad do i want it ?
	}

	// ???
	return( 0 );
}

static const CValStr sm_SkillTitles[] =
{
	"", INT_MIN,
	"Neophyte", 300,
	"Novice", 400,
	"Apprentice", 500,
	"Journeyman", 600,
	"Expert", 700,
	"Adept", 800,
	"Master", 900,
	"Grandmaster", 980,
	NULL, INT_MAX,
};

LPCTSTR CChar::GetTradeTitle() const // Paperdoll title for character p (2)
{
	if ( ! m_sTitle.IsEmpty())
		return( m_sTitle );

	TCHAR * pTemp = Str_GetTemp();

	CCharBase* pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	// Incognito ?
	// If polymorphed then use the poly name.
	if ( IsStatFlag( STATF_Incognito ) ||
		! IsHuman() ||
		( m_pNPC && pCharDef->GetTypeName() != pCharDef->GetTradeName()))
	{
		if ( ! IsIndividualName())
			return( "" );	// same as type anyhow.
		sprintf( pTemp, "the %s", pCharDef->GetTradeName());
		return( pTemp );
	}

	SKILL_TYPE skill = Skill_GetBest();
	int len = sprintf( pTemp, "%s ", sm_SkillTitles->FindName( Skill_GetBase(skill)));
	sprintf( pTemp+len, g_Cfg.GetSkillDef(skill)->m_sTitle, (pCharDef->IsFemale()) ? "woman" : "man" );
	return( pTemp );
}

bool CChar::CanDisturb( CChar * pChar ) const
{
	// I can see/disturb only players with priv same/less than me.
	if ( pChar == NULL )
		return( false );
	if ( GetPrivLevel() < pChar->GetPrivLevel())
	{
		return( ! pChar->IsDND());
	}
	return( true );
}

bool CChar::CanSeeInContainer( const CItemContainer * pContItem ) const
{
	// This is a container of some sort. Can i see inside it ?

	if ( pContItem == NULL )	// must be a CChar ?
		return( true );

	if ( pContItem->IsSearchable())	// not a bank box or locked box.
		return( true );

	// Not normally searchable.
	// Make some special cases for searchable.

	const CChar * pChar = dynamic_cast <const CChar*> (pContItem->GetTopLevelObj());
	if ( pChar == NULL )
	{
		if ( IsPriv(PRIV_GM))
			return( true );
		return( false );
	}

	if ( ! pChar->NPC_IsOwnedBy( this ))	// player vendor boxes.
		return( false );

	if ( pContItem->IsType(IT_EQ_VENDOR_BOX) ||
		pContItem->IsType(IT_EQ_BANK_BOX))
	{
		// must be in the same position i opened it legitamitly.
		// see addBankOpen

		if ( IsPriv(PRIV_GM))
			return( true );
		if ( pContItem->m_itEqBankBox.m_pntOpen != GetTopPoint() )
		{
			/*
			if ( IsClient())
			{

				g_Log.Event( LOGL_WARN|LOGM_CHEAT,
					"%x:Cheater '%s' is using 3rd party tools to access bank box\n",
					GetClient()->m_Socket.GetSocket(), (LPCTSTR) GetClient()->GetAccount()->GetName());

			}
			*/
			return( false );
		}
	}

	return( true );
}

bool CChar::CanSee( const CObjBaseTemplate * pObj ) const
{
	// Can I see this object ( char or item ) ?
	// Am I blind ?

	if ( pObj == NULL )
		return( false );

	if ( IsDisconnected())
	{
		// This could just be a ridden horse !?? in which case i sort of can see it.
		return( false );
	}

	if ( pObj->IsItem())
	{
		const CItem * pItem = STATIC_CAST <const CItem*>(pObj);
		ASSERT(pItem);
		if ( ! CanSeeItem( pItem ))
			return( false );
		CObjBase * pObjCont = pItem->GetContainer();
		if ( pObjCont != NULL )
		{
			if ( ! CanSeeInContainer( dynamic_cast <const CItemContainer*>(pObjCont) ))
				return( false );
			return( CanSee( pObjCont ));
		}
	}
	else
	{
		const CChar * pChar = STATIC_CAST <const CChar*>(pObj);
		ASSERT(pChar);
		if ( this == pChar )
			return( true );
		if ( pChar->IsStatFlag(STATF_DEAD) && m_pNPC )
		{
			if ( !(   m_pNPC->m_Brain == NPCBRAIN_HEALER
				   || Skill_GetBase( SKILL_SPIRITSPEAK ) >= 1000 ) )
				return( false );
		}
		else if ( pChar->IsStatFlag( STATF_Insubstantial | STATF_Invisible | STATF_Hidden ))
		{
			// Characters can be invisible, but not to GM's (true sight ?)
			// equal level can see each other
			if ( pChar->GetPrivLevel() <= PLEVEL_Player )
			{
				if ( GetPrivLevel() <= pChar->GetPrivLevel())
					return( false );
			}
			else
			{
				if ( GetPrivLevel() < pChar->GetPrivLevel())
					return( false );
			}

		}
		if ( pChar->IsDisconnected() && pChar->IsStatFlag(STATF_Ridden))
		{
			CChar * pCharRider = Horse_GetMountChar();
			if ( pCharRider )
			{
				return( CanSee( pCharRider ));
			}
		}
	}

	if (( pObj->IsTopLevel() || pObj->IsDisconnected()) && IsPriv( PRIV_ALLSHOW ))
	{
		// don't exclude for logged out and diff maps.
		return( GetTopPoint().GetDistBase( pObj->GetTopPoint()) <= pObj->GetVisualRange());
	}

	return( GetDist( pObj ) <= pObj->GetVisualRange());
}

bool CChar::CanSeeLOS( const CPointMap & ptDst, CPointMap * pptBlock, int iMaxDist ) const
{
	// Max distance of iMaxDist
	// Line of sight check
	// NOTE: if not blocked. pptBlock is undefined.
	// ToDo: move repeated blocks to a separate functions?
	// 3D LOS later - real LOS, i.e. we can't shoot through the floor, but can shoot through the hole in it

	if ( IsPriv( PRIV_GM ))
		return( true );

	CPointMap ptSrc = GetTopPoint();

	// ptSrc.m_z += PLAYER_HEIGHT/2;
   	// if ( g_Cfg.IsSetEF( EF_CanSeeLOS )
		// && (ptDst.m_z > ptSrc.m_z + 14 || ptDst.m_z + 14 < ptSrc.m_z) )
      		// return false;

	int iDist = ptSrc.GetDist( ptDst );
	if ( iDist > iMaxDist )
	{
blocked:
		if ( pptBlock != NULL )
			* pptBlock = ptSrc;
		return false; // blocked
	}
	// Walk towards the object. If any spot is too far above our heads
	// then we can not see what's behind it.
/*
   	if ( g_Cfg.IsSetEF( EF_CanSeeLOS ) )
	{
		if ( iDist < 2 && (ptDst.m_z > ptSrc.m_z || ptDst.m_z < ptSrc.m_z) )
	      		return false;
	}

*/

	int iDistTry = 0;
	while ( --iDist >= 0 )
	{
		DIR_TYPE dir = ptSrc.GetDir( ptDst );

		WORD wBlockFlags;
		if ( g_Cfg.IsSetEF( EF_DiagonalLOSCheck )	&& (dir % 2) )		// test only diagonal dirs
		{
			CPointMap	ptTest;
			DIR_TYPE	dirTest1	= (DIR_TYPE) (dir-1);	// get 1st ortogonal
			DIR_TYPE	dirTest2	= (DIR_TYPE) (dir+1);	// get 2nd ortogonal
			if ( dirTest2 == DIR_QTY )		// roll over
				dirTest2	= (DIR_TYPE) 0;

			bool fBlocked = false;
			ptTest = ptSrc;
			ptTest.Move( dirTest1 );
			{
				wBlockFlags = CAN_C_SWIM | CAN_C_WALK | CAN_C_FLY;
				signed char z = g_World.GetHeightPoint( ptTest, wBlockFlags, true );;
				if ( g_Cfg.IsSetEF( EF_CanSeeLOS ) )
				{
					signed char zDiff	= abs( z - ptTest.m_z );
					if ( zDiff > PLAYER_HEIGHT )
						fBlocked = true;
					else
						ptTest.m_z	= z;
				}

				if ( wBlockFlags & ( CAN_I_BLOCK | CAN_I_DOOR ))
					fBlocked = true;
			}
			if ( fBlocked )
			{
				ptTest = ptSrc;
				ptTest.Move( dirTest2 );
				{
					wBlockFlags = CAN_C_SWIM | CAN_C_WALK | CAN_C_FLY;
					signed char z = g_World.GetHeightPoint( ptTest, wBlockFlags, true );;
					if ( g_Cfg.IsSetEF( EF_CanSeeLOS ) )
					{
						signed char zDiff	= abs( z - ptTest.m_z );
						if ( zDiff > PLAYER_HEIGHT )
							goto blocked;
						else
							ptTest.m_z	= z;
					}

					if ( wBlockFlags & ( CAN_I_BLOCK | CAN_I_DOOR ))
					{
						ptSrc = ptTest;
						goto blocked;
					}
				}
			}
		}
		if ( iDist )
		{
			ptSrc.Move( dir );	// NOTE: The dir is very coarse and can change slightly.

			wBlockFlags = CAN_C_SWIM | CAN_C_WALK | CAN_C_FLY;
			signed char z = g_World.GetHeightPoint( ptSrc, wBlockFlags, true );;
			if ( g_Cfg.IsSetEF( EF_CanSeeLOS ) )
			{
				signed char zDiff	= abs( z - ptSrc.m_z );
				if ( zDiff > PLAYER_HEIGHT )
					goto blocked;
				else
					ptSrc.m_z	= z;
			}

			if ( wBlockFlags & ( CAN_I_BLOCK | CAN_I_DOOR ))
				goto blocked;

			if ( iDistTry > iMaxDist )
			{
				// just went too far.
				goto blocked;
			}
			iDistTry ++;
		}
	}

	if ( g_Cfg.IsSetEF( EF_CanSeeLOS ) )
	{
		if ( abs( ptSrc.m_z - ptDst.m_z ) > PLAYER_HEIGHT )
			return false;
	}

	return true; // made it all the way to the object with no obstructions.
}



bool CChar::CanSeeLOS( const CObjBaseTemplate * pObj ) const
{
	if ( ! CanSee( pObj ))
		return( false );
	pObj = pObj->GetTopLevelObj();
	return( CanSeeLOS( pObj->GetTopPoint(), NULL, pObj->GetVisualRange()));
}

bool CChar::CanTouch( const CPointMap & pt ) const
{
	// Can I reach this from where i am.
	// swords, spears, arms length = x units.
	// Use or Grab.
	// Check for blocking objects.
	// It this in a container we can't get to ?

	return( CanSeeLOS( pt, NULL, 6 ));
}

bool CChar::CanTouch( const CObjBase * pObj ) const
{
	// Can I reach this from where i am.
	// swords, spears, arms length = x units.
	// Use or Grab. May be in snooped container.
	// Check for blocking objects.
	// Is this in a container we can't get to ?

	if ( pObj == NULL )
		return( false );

	const CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
	ASSERT(pObjTop);
	int iDist = GetTopDist3D( pObjTop );

	bool fDeathImmune = IsPriv( PRIV_GM );
	if ( pObj->IsItem())	// some objects can be used anytime. (even by the dead.)
	{
		const CItem * pItem = dynamic_cast <const CItem*> (pObj);
		ASSERT( pItem );
		switch ( pItem->GetType())
		{
		case IT_SIGN_GUMP:	// can be seen from a distance.
			return( iDist < pObjTop->GetVisualRange());

		case IT_TELESCOPE:
		case IT_SHRINE:	// We can use shrines when dead !!
			fDeathImmune = true;
			break;
		case IT_SHIP_SIDE:
		case IT_SHIP_SIDE_LOCKED:
		case IT_SHIP_PLANK:
		case IT_ARCHERY_BUTTE:	// use from distance.
			if ( IsStatFlag( STATF_Sleeping | STATF_Freeze | STATF_Stone ))
				break;
			return( GetTopDist3D( pItem->GetTopLevelObj()) <= UO_MAP_VIEW_SIZE );
		}
	}

	// Search up to the top level object.
	while (true)
	{
		const CItem * pItem = dynamic_cast <const CItem*>(pObj);
		if ( pItem == NULL )
			break;

		// What is this inside of ?
		CObjBase * pObjCont = pItem->GetContainer();
		if ( pObjCont == NULL )
			break;	// reached top level.

		pObj = pObjCont;
		if ( ! CanSeeInContainer( dynamic_cast <const CItemContainer*>(pObj) ))
		{
			return( false );
		}
	}

	// We now have the top level object.
	DEBUG_CHECK( pObj->IsTopLevel() || pObj->IsDisconnected());
	DEBUG_CHECK( pObj == pObjTop );

	if ( ! fDeathImmune )
	{
		if ( IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Freeze | STATF_Stone ))
			return( false );
	}

	if ( pObjTop->IsChar())
	{
		const CChar * pChar = dynamic_cast <const CChar*> (pObjTop);
		ASSERT( pChar );
		if ( pChar == this )	// something i am carrying. short cut.
		{
			return( true );
		}
		if ( IsPriv( PRIV_GM ))
		{
			return( GetPrivLevel() >= pChar->GetPrivLevel());
		}
		if ( pChar->IsStatFlag( STATF_DEAD|STATF_Stone ))
			return( false );
	}
	else
	{
		// on the ground or In a container on the ground.
		if ( IsPriv( PRIV_GM ))
			return( true );
	}

	if ( iDist > 3 )	// max touch distance.
		return( false );

	return( CanSeeLOS( pObjTop->GetTopPoint(), NULL, pObjTop->GetVisualRange()));
}

IT_TYPE CChar::CanTouchStatic( CPointMap & pt, ITEMID_TYPE id, CItem * pItem )
{
	// Might be a dynamic or a static.
	// RETURN:
	//  IT_JUNK = too far away.
	//  set pt to the top level point.

	if ( pItem )
	{
		if ( ! CanTouch( pItem ))
			return( IT_JUNK );
		pt = GetTopLevelObj()->GetTopPoint();
		return( pItem->GetType());
	}

	// Its a static !

	CItemBase * pItemDef = CItemBase::FindItemBase(id);
	if ( pItemDef == NULL )
		return( IT_NORMAL );

	if ( ! CanTouch( pt ))
		return( IT_JUNK );

	// Is this static really here ?
	const CGrayMapBlock * pMapBlock = g_World.GetMapBlock( pt );
	ASSERT( pMapBlock );

	int x2=pMapBlock->GetOffsetX(pt.m_x);
	int y2=pMapBlock->GetOffsetY(pt.m_y);

	int iQty = pMapBlock->m_Statics.GetStaticQty();
	for ( int i=0; i < iQty; i++ )
	{
		if ( ! pMapBlock->m_Statics.IsStaticPoint(i,x2,y2))
			continue;
		const CUOStaticItemRec * pStatic = pMapBlock->m_Statics.GetStatic( i );
		if ( id == pStatic->GetDispID() )
			return( pItemDef->GetType() );
	}

	return( IT_NORMAL );
}

bool CChar::CanHear( const CObjBaseTemplate * pSrc, TALKMODE_TYPE mode ) const
{
	// can we hear text or sound. (not necessarily understand it (ghost))
	// Can't hear TALKMODE_SAY through house walls.
	// NOTE: Assume pClient->CanHear() has already been called. (if it matters)

	if ( pSrc == NULL )	// must be broadcast i guess.
		return( true );
	bool	fThrough	= false;;
	int		iHearRange;
	switch ( mode )
	{
	case TALKMODE_YELL:
		if ( g_Cfg.m_iDistanceYell < 0 )
			return true;
		iHearRange = g_Cfg.m_iDistanceYell;
		fThrough	= true;
		break;
	case TALKMODE_BROADCAST:
		return( true );
	case TALKMODE_WHISPER:
		if ( g_Cfg.m_iDistanceWhisper < 0 )
			return true;
		iHearRange = g_Cfg.m_iDistanceWhisper;
		break;
	default:
		if ( g_Cfg.m_iDistanceTalk < 0 )
			return true;
		iHearRange = g_Cfg.m_iDistanceTalk;
		break;
	}

	pSrc = pSrc->GetTopLevelObj();
	int iDist = GetTopDist3D( pSrc );
	if ( iDist > iHearRange )	// too far away
		return( false );

	if ( g_Cfg.IsSetOF( OF_NoHouseMuteSpeech ) )
		return( true );

	if ( fThrough )	// a yell goes through walls..
		return( true );

	// sound can be blocked if in house.
	CRegionWorld * pSrcRegion;
	if ( pSrc->IsChar())
	{
		const CChar* pCharSrc = dynamic_cast <const CChar*> (pSrc);
		ASSERT(pCharSrc);
		pSrcRegion = pCharSrc->GetRegion();
	}
	else
	{
		pSrcRegion = dynamic_cast <CRegionWorld *>( pSrc->GetTopPoint().GetRegion( REGION_TYPE_MULTI|REGION_TYPE_AREA ));
	}

	if ( m_pArea == pSrcRegion )	// same region is always ok.
		return( true );
	if ( pSrcRegion == NULL || m_pArea == NULL ) // should not happen really.
		return( false );
	if ( pSrcRegion->GetResourceID().IsItem() && ! pSrcRegion->IsFlag(REGION_FLAG_SHIP))
		return( false );
	if ( m_pArea->GetResourceID().IsItem() && ! m_pArea->IsFlag(REGION_FLAG_SHIP))
		return( false );

	return( true );
}

bool CChar::CanMove( CItem * pItem, bool fMsg ) const
{
	// Is it possible that i could move this ?
	// NOT: test if need to steal. IsTakeCrime()
	// NOT: test if close enough. CanTouch()

	if ( IsPriv( PRIV_ALLMOVE | PRIV_DEBUG | PRIV_GM ))
		return( true );
	if ( IsStatFlag( STATF_Stone | STATF_Freeze | STATF_Insubstantial | STATF_DEAD | STATF_Sleeping ))
	{
		if ( fMsg )
		{
			SysMessagef( "you can't reach anything in your state." );
		}
		return( false );
	}
	if ( pItem == NULL )
		return( false );

#if 0
	// ??? Trigger to test 'can move'
	if ( ! pItem->OnTrigger( ITRIG_CAN_MOVE ))
	{
		return( false );
	}
#endif

	if ( pItem->IsTopLevel() )
	{
		if ( pItem->IsTopLevelMultiLocked())
		{
			if ( fMsg )
			{
				SysMessage( "It appears to be locked to the structure." );
			}
			return false;
		}
	}
	else	// equipped or in a container.
	{
		if ( pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ) && pItem->IsItemEquipped())
		{
			// "It seems to be stuck where it is"
			//
			if ( fMsg )
			{
				pItem->SetAttr(ATTR_IDENTIFIED);
				SysMessagef( "%s appears to be cursed.", (LPCTSTR) pItem->GetName());
			}
			return false;
		}

		// Can't steal/move newbie items on another cchar. (even if pet)
		if ( pItem->IsAttr( ATTR_NEWBIE|ATTR_BLESSED2|ATTR_CURSED|ATTR_CURSED2 ))
		{
			const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
			if ( pObjTop->IsItem())	// is this a corpse or sleeping person ?
			{
				const CItemCorpse * pCorpse = dynamic_cast <const CItemCorpse *>(pObjTop);
				if ( pCorpse )
				{
					CChar * pChar = pCorpse->IsCorpseSleeping();
					if ( pChar && pChar != this )
					{
						return( false );
					}
				}
			}
			else if ( pObjTop->IsChar() && pObjTop != this )
			{
				if ( ! pItem->IsItemEquipped() ||
					pItem->GetEquipLayer() != LAYER_DRAGGING )
				{
					return( false );
				}
			}
		}

		if ( pItem->IsItemEquipped())
		{
			LAYER_TYPE layer = pItem->GetEquipLayer();
			switch ( layer )
			{
			case LAYER_DRAGGING:
				return( true );
			case LAYER_HAIR:
			case LAYER_BEARD:
			case LAYER_PACK:
				if ( ! IsPriv(PRIV_GM))
					return( false );
				break;
			default:
				if ( ! CItemBase::IsVisibleLayer(layer) && ! IsPriv(PRIV_GM))
				{
					return( false );
				}
			}
		}
	}

	return( pItem->IsMovable());
}

bool CChar::IsTakeCrime( const CItem * pItem, CChar ** ppCharMark ) const
{
	// We are snooping or stealing.
	// Is taking this item a crime ?
	// RETURN:
	//  ppCharMark = The character we are offending.
	//  false = no crime.

	if ( IsPriv(PRIV_GM|PRIV_ALLMOVE))
		return( false );

	CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
	CChar * pCharMark = dynamic_cast <CChar *> (pObjTop);
	if ( ppCharMark != NULL )
	{
		*ppCharMark = pCharMark;
	}

	if ( static_cast <const CObjBase *>(pObjTop) == this )
	{
		// this is yours
		return( false );
	}

	if ( pCharMark == NULL )	// In some (or is) container.
	{
		if ( pItem->IsAttr(ATTR_OWNED) && pItem->m_uidLink != GetUID())
			return( true );

		CItemContainer * pCont = dynamic_cast <CItemContainer *> (pObjTop);
		if (pCont)
		{
			if ( pCont->IsAttr(ATTR_OWNED))
				return( true );

			// On corpse
			// ??? what if the container is locked ?
		}

		const CItemCorpse * pCorpseItem = dynamic_cast <const CItemCorpse *>(pObjTop);
		if ( pCorpseItem )
		{
			// Taking stuff off someones corpse can be a crime !
			return( const_cast <CChar*>(this)->CheckCorpseCrime( pCorpseItem, true, true ));
		}

		return( false );	// i guess it's not a crime.
	}

	if ( pCharMark->NPC_IsOwnedBy( this ))	// He let's you
		return( false );

	// Pack animal has no owner ?
	if ( pCharMark->GetNPCBrain() == NPCBRAIN_ANIMAL &&	// free to take.
		! pCharMark->IsStatFlag( STATF_Pet ) &&
		! pCharMark->m_pPlayer )
	{
		return( false );
	}

	return( true );
}

bool CChar::CanUse( CItem * pItem, bool fMoveOrConsume ) const
{
	// Can the Char use ( CONSUME )  the item where it is ?
	// NOTE: Although this implies that we pick it up and use it.

	if ( ! CanTouch(pItem) )
		return( false );

	if ( fMoveOrConsume )
	{
		if ( ! CanMove(pItem))
			return( false );	// ? using does not imply moving in all cases ! such as reading ?
		if ( IsTakeCrime( pItem ))
			return( false );
	}
	else
	{
		// Just snooping i guess.
		if ( pItem->IsTopLevel())
			return( true );
		// The item is on another character ?
		const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
		if ( pObjTop != ( static_cast <const CObjBaseTemplate *>(this)))
		{
			if ( IsPriv( PRIV_GM ))
				return( true );
			if ( pItem->IsType(IT_CONTAINER))
				return( true );
			if ( pItem->IsType(IT_BOOK))
				return( true );
			return( false );
		}
	}

	return( true );
}

CRegionBase * CChar::CheckValidMove( CPointBase & ptDest, WORD * pwBlockFlags, DIR_TYPE dir ) const
{
	// Is it ok to move here ? is it blocked ?
	// ignore other characters for now.
	// RETURN:
	//  The new region we may be in.
	//  Fill in the proper ptDest.m_z value for this location. (if walking)
	//  pwBlockFlags = what is blocking me. (can be null = don't care)

	if ( g_Cfg.IsSetEF( EF_DiagonalWalkCheck )	&& (dir % 2) )		// test only diagonal dirs
	{
		CPointMap	ptTest;
		DIR_TYPE	dirTest1	= (DIR_TYPE) (dir-1);	// get 1st ortogonal
		DIR_TYPE	dirTest2	= (DIR_TYPE) (dir+1);	// get 2nd ortogonal
		if ( dirTest2 == DIR_QTY )		// roll over
			dirTest2	= (DIR_TYPE) 0;

		ptTest		= GetTopPoint();
		ptTest.Move( dirTest1 );
		if ( !CheckValidMove( ptTest, pwBlockFlags ) )
				return NULL;

		ptTest		= GetTopPoint();
		ptTest.Move( dirTest2 );
		if ( !CheckValidMove( ptTest, pwBlockFlags ) )
				return NULL;

	}

	CRegionBase * pArea = ptDest.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA );
	if ( pArea == NULL )
		return( NULL );

	// In theory the client only lets us go valid places.
	// But in reality the client and server are not really in sync.
	// if ( IsClient()) return( pArea );

	WORD wCan = GetMoveBlockFlags();
	WORD wBlockFlags = wCan;
	signed char z = g_World.GetHeightPoint( ptDest, wBlockFlags, true );
	if ( wCan != 0xFFFF )
	{
		if ( wBlockFlags &~ wCan )
		{
			return( NULL );
		}

		CCharBase* pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		if ( ! pCharDef->Can( CAN_C_FLY ))
		{
			if ( z > ptDest.m_z + PLAYER_HEIGHT )	// Too high to climb.
				return( NULL );
		}
		if ( z >= UO_SIZE_Z )
			return( NULL );
	}

	if ( pwBlockFlags )
	{
		*pwBlockFlags = wBlockFlags;
	}

	ptDest.m_z = z;
	return( pArea );
}

CRegionBase * CChar::CheckValidMove_New( CPointBase & ptDest, WORD * pwBlockFlags, DIR_TYPE dir, signed char * pClimbHeight ) const
{
	// Is it ok to move here ? is it blocked ?
	// ignore other characters for now.
	// RETURN:
	//  The new region we may be in.
	//  Fill in the proper ptDest.m_z value for this location. (if walking)
	//  pwBlockFlags = what is blocking me. (can be null = don't care)

	if ( g_Cfg.IsSetEF( EF_DiagonalWalkCheck )	&& (dir % 2) )		// test only diagonal dirs
	{
		CPointMap	ptTest;
		DIR_TYPE	dirTest1	= (DIR_TYPE) (dir-1);	// get 1st ortogonal
		DIR_TYPE	dirTest2	= (DIR_TYPE) (dir+1);	// get 2nd ortogonal
		if ( dirTest2 == DIR_QTY )		// roll over
			dirTest2	= (DIR_TYPE) 0;

		ptTest		= GetTopPoint();
		ptTest.Move( dirTest1 );
		if ( !CheckValidMove_New( ptTest, pwBlockFlags, DIR_QTY, pClimbHeight ) )
				return NULL;

		ptTest		= GetTopPoint();
		ptTest.Move( dirTest2 );
		if ( !CheckValidMove_New( ptTest, pwBlockFlags, DIR_QTY, pClimbHeight ) )
				return NULL;

	}
	CRegionBase * pArea = ptDest.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA );
	if ( pArea == NULL )
		return( NULL );

	WORD wCan = GetMoveBlockFlags();
	WORD wBlockFlags = wCan;

	if ( wCan & CAN_C_WALK )
		wBlockFlags |= CAN_I_CLIMB; // If we can walk than we can climb. Ignore CAN_C_FLY at all here

	//ptDest.m_z += m_zClimbHeight; // If we are climbing than check z at the top of stairs
	CGrayMapBlockState block( wBlockFlags, ptDest.m_z, ptDest.m_z + m_zClimbHeight + PLAYER_HEIGHT + 3, ptDest.m_z + m_zClimbHeight + 2 );

	g_World.GetHeightPoint_New( ptDest, block, true );

	// Pass along my results.
	wBlockFlags = block.m_Bottom.m_dwBlockFlags;

	if ( block.m_Top.m_dwBlockFlags )
	{
		wBlockFlags |= CAN_I_ROOF;	// we are covered by something.
		if ( block.m_Top.m_z < ptDest.m_z - m_zClimbHeight + (block.m_Top.m_wTile > TERRAIN_QTY ? PLAYER_HEIGHT : PLAYER_HEIGHT/2 ) )
			wBlockFlags |= CAN_I_BLOCK; // we can't fit under this!
	}


	if ( wCan != 0xFFFF )
	{
		// do not check CAN_I_CLIMB
		if ( ( wBlockFlags &~ CAN_I_CLIMB ) &~ wCan )
		{
			return( NULL );
		}
#if 1
		CCharBase* pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		if ( ! pCharDef->Can( CAN_C_FLY ))
		{
			if ( ! ( wBlockFlags & CAN_I_CLIMB ) ) // we can climb anywhere
			{
				if ( block.m_Bottom.m_wTile > TERRAIN_QTY )
				{
					if ( block.m_Bottom.m_z > ptDest.m_z + m_zClimbHeight + 2) // Too high to climb.
						return( NULL );
				}
				else if ( block.m_Bottom.m_z > ptDest.m_z + m_zClimbHeight + PLAYER_HEIGHT + 3)
						return( NULL );
			}
		}
#endif
		if ( block.m_Bottom.m_z >= UO_SIZE_Z )
			return( NULL );
	}

	if ( pwBlockFlags )
	{
		*pwBlockFlags = wBlockFlags;
	}
	if ( ( wBlockFlags & CAN_I_CLIMB ) && ( pClimbHeight ) )
		*pClimbHeight = block.m_zClimbHeight;


	ptDest.m_z = block.m_Bottom.m_z;
	return( pArea );
}

void CChar::FixClimbHeight()
{
	if ( g_Cfg.IsSetEF( EF_WalkCheck ) ) // get correct climb height (or could be stacked)
	{
		CPointBase pt = GetTopPoint();
		CGrayMapBlockState block( CAN_I_CLIMB, pt.m_z , pt.m_z + PLAYER_HEIGHT + 3, pt.m_z + 2 );

		g_World.GetHeightPoint_New( pt, block, true );

		if ( ( block.m_Bottom.m_z == pt.m_z ) && ( block.m_dwBlockFlags & CAN_I_CLIMB ) ) // we are standing on stairs
			m_zClimbHeight = block.m_zClimbHeight;
		else
			m_zClimbHeight = 0;
	}
	m_fClimbUpdated = true;
}