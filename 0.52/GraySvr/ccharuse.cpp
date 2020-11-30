//
// CCharUse.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.

static WORD Item_Blood[] =	// ??? not used
{
	0x1cf2, ITEMID_BLOOD1, ITEMID_BLOOD6,
};

int CChar::Use_Consume( CItem * pItem, int iQty )
{
	// Eat or drink specific item. delete it when gone.
	if ( pItem == NULL )
		return 0;
	int iQtyMax = pItem->GetAmount();
	if ( iQty >= iQtyMax )
	{
		iQty = iQtyMax;
		pItem->SetAmount( 0 );	// let there be 0 amount here til decay.
		if ( ! pItem->IsAttr( ATTR_INVIS ))	// don't delete resource locators.
		{
			pItem->Delete();
		}
	}
	else
	{
		pItem->SetAmountUpdate( iQtyMax - iQty );
	}
	return( iQty );
}

bool CChar::Use_CarveCorpseTest( CChar *pCharCorpse, const CItemCorpse *pCorpse, bool fLooting )
{
	// RETURN: true = criminal act !

	if ( pCharCorpse == NULL )
		return( false );
	if ( pCharCorpse == this )	// ok but strange to carve up your own corpse.
		return( false );
	if ( ! g_Serv.m_fLootingIsACrime )
		return( false );

	// It's ok to loot a guild member !
	NOTO_TYPE noto = pCharCorpse->GetNotoFlag( this, false );
	switch (noto)
	{
	case NOTO_GUILD_SAME:
	case NOTO_CRIMINAL:
	case NOTO_GUILD_WAR:
	case NOTO_EVIL:
		return( false );
	}

	if ( fLooting )
		CheckCrimeSeen( SKILL_NONE, pCharCorpse, pCorpse, "looting" );
	else
		CheckCrimeSeen( SKILL_NONE, pCharCorpse, pCorpse, NULL );

	Noto_Criminal();
	return true; // got flagged
}

void CChar::Use_CarveCorpse( CItemCorpse * pCorpse )
{
	DEBUG_CHECK( pCorpse->GetID() == ITEMID_CORPSE );

	UpdateAnimate( ANIM_BOW );

	CChar * pChar = pCorpse->m_uidLink.CharFind();
	if ( pChar && pChar->IsStat( STATF_Sleeping ))
	{
		// They are violently awakened.
		SysMessage( "That is not a corpse!" );
		pChar->SysMessage( "You are violently awakened" );
		pChar->OnTakeDamage( 1 + GetRandVal(10), this, DAMAGE_HIT|DAMAGE_GENERAL );
		return;
	}

	CPointMap pnt = pCorpse->GetTopLevelObj()->GetTopPoint();

	bool fHumanCorpse = CCharBase::IsHuman( pCorpse->GetCorpseType());
	if ( fHumanCorpse )
	{
		// dismember the corpse.
		static const WORD Item_BodyParts[] = // ITEMID_TYPE
		{
			0x1ce1,
			0x1cdd,
			0x1ce5,
			0x1ce2,
			0x1cec,
			0x1ced,
			0x1cf1
		};
		// If it's a player corpse, put the stuff in one of these and (mabey)
		// flag them a criminal

		CItem * pParts = CItem::CreateBase( (ITEMID_TYPE) Item_Blood[ GetRandVal( COUNTOF( Item_Blood )) ] );
		pParts->SetDecayTime( 60*TICK_PER_SEC );
		pParts->MoveTo( pCorpse->GetTopPoint());

		for ( int i=0; i < COUNTOF( Item_BodyParts ); i++ )
		{
			pParts = CItem::CreateScript( (ITEMID_TYPE) Item_BodyParts[i] );
			pParts->SetDecayTime( 5*60*TICK_PER_SEC );
			if ( pChar )
			{
				CGString sName;
				sName.Format( "%s of %s", pParts->GetName(), pChar->GetName());
				pParts->SetName( sName );
				pParts->m_uidLink = pChar->GetUID();
			}
			pParts->MoveTo( pnt );
		}

		if ( pChar && pChar->m_pPlayer && pChar != this )
		{
			// Until the corse inherits NPC criminal status, only do
			// other players
			if ( Use_CarveCorpseTest( pChar, pCorpse, false ))
			{
				SysMessage("Guards can now be called on you.");
			}

		static const WORD Item_Skeletons[] =
		{
			ITEMID_SKELETON_1,
			ITEMID_SKELETON_2,
			ITEMID_SKELETON_3,
			ITEMID_SKELETON_4,
			ITEMID_SKELETON_5,
			ITEMID_SKELETON_6,
			ITEMID_SKELETON_7,
			ITEMID_SKELETON_8,
			ITEMID_SKELETON_9,
		};
			// Transfer contents to the skeleton
			CItemContainer * pSkeleton = dynamic_cast <CItemContainer*> (CItem::CreateBase( (ITEMID_TYPE) Item_Skeletons[ GetRandVal( COUNTOF( Item_Skeletons )) ] ));
			ASSERT(pSkeleton);
			CGString sName;
			sName.Format( "%s of %s", pSkeleton->GetName(), pChar->GetName());
			pSkeleton->SetName( sName );
			pSkeleton->MoveTo( pnt );
			pSkeleton->m_Attr |= ATTR_MOVE_NEVER;
			pCorpse->ContentsTransfer( pSkeleton, true );
			pSkeleton->SetDecayTime( 5*60*TICK_PER_SEC );
		}
		else
		{
			// Dump any stuff on corpse.
			pCorpse->ContentsDump( pnt );
		}
	}

	// based on corpse type.
	const CCharBase * pCre = CCharBase::FindCharBase( (CREID_TYPE) pCorpse->m_itCorpse.m_BaseID );
	if ( pCre == NULL ||
		! pCorpse->m_itCorpse.m_tDeathTime ||
		pCre->m_sCorpseResources.IsEmpty())
	{
		SysMessage( "You carve the corpse but find nothing usefull." );
		pCorpse->m_itCorpse.m_tDeathTime = 0;
		pCorpse->m_itCorpse.m_uidKiller = GetUID();
		return;
	}

	pCorpse->m_itCorpse.m_tDeathTime = 0;	// been carved.
	pCorpse->m_itCorpse.m_uidKiller = GetUID();	// by you.

	TCHAR szDesc[ MAX_SCRIPT_LINE_LEN ];
	strcpy( szDesc, pCre->m_sCorpseResources );
	TCHAR * pDesc = szDesc;
	const TCHAR * pszMsg = NULL;

	while (true)
	{
		int iQty = 0;
		ITEMID_TYPE id = GetResourceEntry( pDesc, iQty );
		if ( id == ITEMID_NOTHING ) 
			break;

		if ( pszMsg == NULL ) switch ( id )
		{
		case ITEMID_FOOD_MEAT_RAW:	// meat
			pszMsg = "You carve away some meat.";
			break;
		case ITEMID_FEATHERS3:	// feathers.
			pszMsg = "You pluck the bird and get some feathers.";
			break;
		case ITEMID_HIDES:	// hides
			pszMsg = "You skin the corpse and get the hides.";
			break;
		case ITEMID_FURS:	// fur
			pszMsg = "You skin the corpse and get some fur.";
			break;
		case ITEMID_WOOL:	// wool
			pszMsg = "You skin the corpse and get some unspun wool.";
			break;
		}

		CItem * pPart = CItem::CreateTemplate( id );
 		if ( id == ITEMID_FOOD_MEAT_RAW )
			pPart->m_itFoodRaw.m_MeatType = pCorpse->m_itCorpse.m_BaseID;
		if ( iQty > 1 ) 
			pPart->SetAmount( iQty );
		if ( !fHumanCorpse )
			pCorpse->ContentAdd( pPart );
		else
			pPart->MoveTo( pnt );
	}

	if ( pszMsg != NULL )
	{
		SysMessage( pszMsg );
	}

	CItem * pBlood = CItem::CreateBase( ITEMID_BLOOD1 );
	pBlood->SetDecayTime( 60*TICK_PER_SEC );
	pBlood->MoveTo( pnt );

	if ( fHumanCorpse )
		pCorpse->Delete();
}

void CChar::Use_AdvanceGate( CItem * pItem )
{
	// Have we already used one ?
	DEBUG_CHECK( pItem->m_type == ITEM_ADVANCE_GATE );

	if ( ! IsClient()) 
		return;

	if ( pItem->m_itAdvanceGate.m_Type )
	{
		CCharBase * pDef = CCharBase::FindCharBase( pItem->m_itAdvanceGate.m_Type );
		if ( pDef == NULL )
		{
bailout:
			SysMessage( "The advance gate is not configured." );
			return;
		}
		CScriptLock s;
		if ( pDef->ScriptLockBase(s))
		{
			bool fRet = ReadScript( s, false, true );
			if ( ! fRet ) 
				goto bailout;
		}
	}

	m_pPlayer->m_Plot1 |= pItem->m_itAdvanceGate.m_Plot1;

	RemoveFromView();
	Update();
	Effect( EFFECT_OBJ, ITEMID_FX_ADVANCE_EFFECT, this, 9, 30 );
}

void CChar::Use_MoonGate( CItem * pItem )
{
	CPointMap ptTeleport = pItem->m_itTelepad.m_mark_p;
	if ( pItem->m_type == ITEM_MOONGATE )
	{
		// What gate are we at ?
		int iCount = g_World.m_MoonGates.GetCount();
		int i=0;
		for ( ;true; i++ )
		{
			if ( i>=iCount) 
				return;
			int iDist = GetTopPoint().GetDist( g_World.m_MoonGates[i] );
			if ( iDist <= UO_MAP_VIEW_SIZE )
				break;
		}

		// Set it's current destination based on the moon phases.
		int iPhaseJump = ( g_World.GetMoonPhase( false ) - g_World.GetMoonPhase( true )) % iCount;
		// The % operator actually can produce negative numbers...Check that.
		if (iPhaseJump < 0)
			iPhaseJump += iCount;

		ptTeleport = g_World.m_MoonGates[ (i+ iPhaseJump) % iCount ];
	}

	if ( m_pNPC )
	{
		// NPCs and gates.
		if ( pItem->m_itTelepad.m_fPlayerOnly )
		{
			return;
		}
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )
		{
			// Guards won't gate into unguarded areas.
			CRegionWorld * pArea = dynamic_cast <CRegionWorld *> ( ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( pArea == NULL || ! pArea->IsGuarded())
				return;
		}
		if ( IsEvil())
		{
			// wont teleport to guarded areas.
			CRegionWorld * pArea = dynamic_cast <CRegionWorld *> ( ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( pArea == NULL || pArea->IsGuarded())
				return;
		}
	}

	// Teleport me. and take a step.
	Spell_Teleport( ptTeleport, true,
		(pItem->IsTimerSet())?true:false,
		(pItem->m_itTelepad.m_fQuiet) ? ITEMID_NOTHING : ITEMID_FX_TELE_VANISH );
}

bool CChar::Use_Campfire( CItem * pKindling )
{
	if ( pKindling->IsInContainer())
	{
		SysMessage( "You can't light the kindling in a container" );
		return false ;
	}

	if ( ! Skill_UseQuick( SKILL_CAMPING, GetRandVal(30)))
	{
		SysMessage( "You fail to ignite the fire." );
		return false;
	}

	pKindling->SetDispID( ITEMID_CAMPFIRE );
	pKindling->m_itLight.m_pattern = LIGHT_LARGE;
	pKindling->m_Attr |= ATTR_MOVE_NEVER | ATTR_CAN_DECAY;
	pKindling->SetTimeout( (4+pKindling->GetAmount())*60*TICK_PER_SEC );
	pKindling->SetAmount(1);	// All kindling is set to one fire.
	pKindling->Update();
	pKindling->Sound( 0x226 );
	return( true );
}

bool CChar::Use_Cannon_Feed( CItem * pCannon, CItem * pFeed )
{
	if ( pCannon != NULL &&
		pCannon->m_type == ITEM_CANNON_MUZZLE &&
		pFeed != NULL )
	{
		if ( pFeed->GetID() == ITEMID_REAG_SA )
		{
			if ( pCannon->m_itCannon.m_Load & 1 )
			{
				SysMessage( "The cannon already has powder." );
				return( false );
			}
			pCannon->m_itCannon.m_Load |= 1;
			SysMessage( "Powder loaded." );
			return( true );
		}

		if ( pFeed->IsSameDispID( ITEMID_Cannon_Ball ))
		{
			if ( pCannon->m_itCannon.m_Load & 2 )
			{
				SysMessage( "The cannon already has shot." );
				return( false );
			}
			pCannon->m_itCannon.m_Load |= 2;
			SysMessage( "Shot loaded." );
			return( true );
		}
	}

	SysMessage( "Feed shot and powder into the cannon muzzle." );
	return( false );
}

bool CChar::Use_Plant( CItem * pItem )
{
	ASSERT( pItem->m_type == ITEM_PLANT );
	// Pick cotton/hay/etc...
	if ( pItem->IsAttr(ATTR_INVIS))
		return( false );

	DWORD iNextTick = g_World.GetNextNewMoon(true) - ( (g_World.GetTime() + GetRandVal(20) ) * g_Serv.m_iGameMinuteLength * TICK_PER_SEC );

	ITEMID_TYPE id = ITEMID_NOTHING;
	const TCHAR * pszName;

	switch ( pItem->GetID())
	{
	case ITEMID_SPROUT_NORMAL:
	case ITEMID_SPROUT_NORMAL2:
		SysMessage( "None of the crops are ripe enough." );
		return true;
	case ITEMID_SPROUT_WHEAT1:
	case ITEMID_SPROUT_WHEAT2:
		SysMessage( "None of the wheat is ripe enough." );
		return true;
	case ITEMID_PLANT_CORN2:
		SysMessage( "The corn isn't ripe yet." );
		return true;
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

	ITEMID_TYPE iReapID = pItem->m_itPlant.m_Reap_ID;
	if ( id != ITEMID_NOTHING )
	{
		pItem->SetDispID(id);
		pItem->m_itPlant.m_Reap_ID = iReapID;
		pItem->SetName(pszName);
	}

	pItem->SetTimeout( iNextTick );
	pItem->RemoveFromView();	// remove from most screens.
	pItem->SetColor( COLOR_RED_DARK );	// Indicate to GM's that it is growing.
	pItem->m_Attr |= ATTR_INVIS;	// regrown invis.

	UpdateAnimate( ANIM_BOW );
	Sound( 0x13e );

	if ( iReapID )
	{
		CItem * pItemFruit = CItem::CreateScript( iReapID );
		switch ( pItemFruit->GetID())
		{
		case ITEMID_FRUIT_COTTON:
		case ITEMID_FRUIT_HAY1:
		case ITEMID_FRUIT_HAY2:
		case ITEMID_FRUIT_HAY3:
		case ITEMID_FRUIT_WHEAT:
		case ITEMID_FRUIT_HOPS:
		case ITEMID_FRUIT_FLAX1:
		case ITEMID_FRUIT_FLAX2:
			pItemFruit->m_type = ITEM_NORMAL;		// What should this be?
			break;
		case ITEMID_FRUIT_MANDRAKE_ROOT1:
		case ITEMID_FRUIT_MANDRAKE_ROOT2:
		case ITEMID_FRUIT_NIGHTSHADE1:
		case ITEMID_FRUIT_NIGHTSHADE2:
		case ITEMID_FRUIT_GARLIC1:
		case ITEMID_FRUIT_GARLIC2:
		case ITEMID_FRUIT_GENSENG1:
		case ITEMID_FRUIT_GENSENG2:
			pItemFruit->m_type = ITEM_REAGENT_RAW;
			break;
		default:
			pItemFruit->m_type = ITEM_FOOD;
			break;
		}
		switch ( pItemFruit->GetID())
		{
		case ITEMID_COTTON_RAW:
			SysMessage( ( ! GetRandVal(10)) ?
				"Reach down, Turn around, Pick a bail of cotton." :
				"You reach down and pick some cotton.");
			break;
		case ITEMID_HAY:
			SysMessage( "You bundle some hay" );
			break;
		default:
			{
				CGString sMsg;
				sMsg.Format( "You pick some %s", pItemFruit->GetName());
				SysMessage( sMsg );
			}
			break;
		}
		ItemBounce( pItemFruit );
	}

	return true;
}

bool CChar::Use_TrainingDummy( CItem * pItem)
{
	SKILL_TYPE skill = GetWeaponSkill();
	if ( skill == SKILL_ARCHERY ) // We do not allow archerytraining on dummys.
	{
		SysMessage( "You cannot train archery on this." );
		return( true );
	}
	if ( Skill_GetBase(skill) > 300 )
	{
		SysMessage( "You can learn alot from a dummy, but you've already learned it all." );
		return( true );
	}
	if ( ! pItem->IsTopLevel())
	{
baddumy:
		SysMessage( "You must be standing in front of or behind the dummy to use it." );
		return( true );
	}

	// Check location
	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	if ( pItem->GetID() == ITEMID_DUMMY1 )
	{
		if ( ! ( ! dx && abs(dy) < 2 ))
			goto baddumy;
	}
	else
	{
		if ( ! ( ! dy && abs(dx) < 2 ))
			goto baddumy;
	}

	pItem->Sound( 0x033 );
	pItem->SetAnim( (ITEMID_TYPE) ( pItem->GetID() + 1 ), 3*TICK_PER_SEC );
	Skill_UseQuick( skill, GetRandVal(30)) ;
	UpdateAnimate( ANIM_ATTACK_WEAPON );
	return( true );
}

bool CChar::Use_PickPocketDip( CItem *pItem)
{
	// Train dummy.
	if ( Skill_GetBase(SKILL_STEALING) > 300 )
	{
		SysMessage( "Try practicing on real people." );
		return( true );
	}
	if ( !pItem->IsTopLevel())
	{
badpickpocket:
		SysMessage( "You must be standing in front of or behind the dip to use it." );
		return( true );
	}
	
	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	bool fNS = ( pItem->GetDispID() == ITEMID_PICKPOCKET_NS || 
		pItem->GetDispID() == ITEMID_PICKPOCKET_NS2 );
	if ( fNS )
	{
		if ( ! ( ! dx && abs(dy) < 2 ))
			goto badpickpocket;
	}
	else
	{
		if ( ! ( ! dy && abs(dx) < 2 ))
			goto badpickpocket;
	}
	pItem->Sound( 0x033 );
	pItem->SetAnim( fNS ? ITEMID_PICKPOCKET_NS_FX : ITEMID_PICKPOCKET_EW_FX, 3*TICK_PER_SEC );

	Skill_UseQuick( SKILL_STEALING, GetRandVal(30));
	UpdateAnimate( ANIM_ATTACK_WEAPON );
	return( true );
}

bool CChar::Use_ArcheryButte( CItem * pButte )
{
	// If we are standing right next to the butte,
	// retrieve the arrows and bolts
	DEBUG_CHECK( pButte->m_type == ITEM_ARCHERY_BUTTE );

	if ( GetDist( pButte ) < 2 )
	{
		if ( pButte->m_itArcheryButte.m_AmmoCount == 0 )
		{
			SysMessage( "The target is empty.");
			return ( true );
		}

		CGString sMsg;
		sMsg.Format( "remove the %s%s from the target",
			( pButte->m_itArcheryButte.m_AmmoType == ITEMID_Arrow ) ?
			"arrow" : "bolt",
			(pButte->m_itArcheryButte.m_AmmoCount == 1 ) ? "" : "s" );
		Emote( sMsg );

		CItem *pRemovedAmmo =  CItem::CreateBase( pButte->m_itArcheryButte.m_AmmoType );
		pRemovedAmmo->SetAmount( pButte->m_itArcheryButte.m_AmmoCount );
		ItemBounce( pRemovedAmmo );

		// Clear the target
		pButte->m_itArcheryButte.m_AmmoType = ITEMID_NOTHING;
		pButte->m_itArcheryButte.m_AmmoCount = 0;
		return ( true );
	}

	// We are not standing right next to the target...

	// We have to be using the archery skill on this
	SKILL_TYPE skill = GetWeaponSkill();
	if ( skill != SKILL_ARCHERY )
	{
		SysMessage( "You can only train archery on this." );
		return ( true );
	}
	if ( Skill_GetBase(SKILL_ARCHERY)>300 )
	{
		SysMessage( "You can't learn any more by using an archery butte." );
		return ( true );
	}

	// Make sure we have some ammo

	CItem * pWeapon = m_weapon.ItemFind();
	ITEMID_TYPE AmmoID = pWeapon->Armor_IsXBow() ? ITEMID_XBolt : ITEMID_Arrow;

	// If there is a different ammo type on the butte currently,
	// tell us to remove the current type first.
	if ( (pButte->m_itArcheryButte.m_AmmoType != ITEMID_NOTHING) && (pButte->m_itArcheryButte.m_AmmoType != AmmoID))
	{
		SysMessage( "You need to remove what is on the target before you can train here." );
		return ( true );
	}

	// We need to be correctly aligned with the target before we can use it
	// For the south facing butte, we need to have the save x value and a y value of butte.y + 6
	// For the east facing butte, we need to have the same y value and an x value of butte.x + 6
	if ( ! pButte->IsTopLevel())
	{
badalign:
		SysMessage("You need to be correctly aligned with the butte in order to use it.");
		return ( true );
	}

	int targDistX = GetTopPoint().m_x - pButte->GetTopPoint().m_x;
	int targDistY = GetTopPoint().m_y - pButte->GetTopPoint().m_y;

	if (pButte->GetID() == ITEMID_ARCHERYBUTTE_S)
	{
		if ( ! ( targDistX == 0 && targDistY > 3 && targDistY < 7 ))
			goto badalign;
	}
	else
	{
		if ( ! ( targDistY == 0 && targDistX > 3 && targDistX < 7 ))
			goto badalign;
	}

	// Is anything in the way?
	if ( !CanSeeLOS(pButte))
	{
		SysMessage("The target is currently blocked.");
		return ( true );
	}

	if ( m_pPlayer && ContentConsume( AmmoID ))
	{
		SysMessage( "You have no ammunition" );
		return( true );
	}

	// OK...go ahead and fire at the target
	// Check the skill
	bool fSuccess = Skill_UseQuick( skill, GetRandVal(30));
	UpdateAnimate( ANIM_ATTACK_WEAPON );

	pButte->Effect( EFFECT_BOLT, (AmmoID == ITEMID_Arrow) ? ITEMID_ArrowX : ITEMID_XBoltX, this, 16, 0, false );
	pButte->Sound( 0x224 );

static const TCHAR * Txt_ArcheryButte_Failure[] =
{
	"shot is well off target.",
	"shot is wide of the mark.",
	"shot misses terribly.",
	"shot nearly misses the archery butte."
};

static const TCHAR * Txt_ArcheryButte_Success[] =
{
	"hit the outer ring.",
	"hit the middle ring.",
	"hit the inner ring.",
	"hit the bullseye!."
};

	// Did we destroy the ammo?
	const TCHAR * pszAmmoName = ( AmmoID == ITEMID_Arrow ) ? "arrow" : "bolt";
	if (!fSuccess)
	{
		// Small chance of destroying the ammo
		if ( !GetRandVal(10))
		{
			CGString sMessage;
			sMessage.Format( "poor shot destroys the %s.", pszAmmoName );
			Emote( sMessage, NULL, true );
			return ( true );
		}

		Emote( Txt_ArcheryButte_Failure[ GetRandVal(COUNTOF(Txt_ArcheryButte_Failure)) ], NULL, true );
	}
	else
	{
		// Very small chance of destroying another arrow
		if ( !GetRandVal(50) && pButte->m_itArcheryButte.m_AmmoCount )
		{
			CGString sMessage;
			sMessage.Format("shot splits another %s.", pszAmmoName );
			Emote( sMessage, NULL, true );
			return ( true );
		}

		Emote( Txt_ArcheryButte_Success[ GetRandVal(COUNTOF(Txt_ArcheryButte_Failure)) ], NULL, false );
	}
	// Update the target
	pButte->m_itArcheryButte.m_AmmoType = AmmoID;
	pButte->m_itArcheryButte.m_AmmoCount++;
	return( true );
}

bool CChar::Use_Item_Web( CItem * pItemWeb )
{
	// RETURN: true = held in place.
	//  false = walk thru it.
	ASSERT( pItemWeb );
	DEBUG_CHECK( pItemWeb->m_type == ITEM_WEB );

	if ( GetDispID() == CREID_GIANT_SPIDER ||
		IsPriv(PRIV_GM) ||
		! pItemWeb->IsTopLevel() ||
		IsStat(STATF_DEAD|STATF_Insubstantial))
		return( false );	// just walks thru it.

	// Try to break it.
	int iStr = pItemWeb->m_itWeb.m_Hits_Cur;
	if ( iStr == 0 )
	{
		iStr = pItemWeb->m_itWeb.m_Hits_Cur = 60 + GetRandVal( 250 );
	}

	CItem * pFlag = LayerFind( LAYER_FLAG_Stuck );

	// Since broken webs become spider silk, we should get out of here now if we aren't in a web.
	if (pItemWeb->m_type != ITEM_WEB)
	{
		if (pFlag) pFlag->Delete();
		return( false );
	}

	if ( pFlag )
	{
		if ( pFlag->IsTimerSet()) 
			return( true );
	}

	int iDmg = pItemWeb->OnTakeDamage( Stat_Get(STAT_STR), this );

	if ( iDmg >= 0 && GetTopPoint() != pItemWeb->GetTopPoint())
		return( false ); // at a distance.

	// Stuck in it still.
	if ( pFlag == NULL )
	{
		if ( iDmg < 0 )
			return( false );
		// First time message.
		pFlag = CItem::CreateBase( ITEMID_WEB1_1 );
		pFlag->m_type = ITEM_EQ_STUCK;
		pFlag->m_itEqStuck.m_p = GetTopPoint();
		pFlag->m_uidLink = pItemWeb->GetUID();
		LayerAdd( pFlag, LAYER_FLAG_Stuck );
	}
	else if ( iDmg < 0 )
	{
		pFlag->Delete();
		return( false );
	}

	pFlag->SetTimeout( TICK_PER_SEC );	// Don't check it too often.
	return( true );
}

int CChar::Use_PlayMusic( CItem * pInstrument, int iDifficultyToPlay )
{
	// iDifficulty = 0-100
	// SKILL_ENTICEMENT, SKILL_MUSICIANSHIP,
	// RETURN:
	//  Difficulty.
	//  -2 = can't play. no instrument.
	//

	if ( pInstrument == NULL )
	{
		pInstrument = ContentFindType( ITEM_MUSICAL );
		if ( pInstrument == NULL )
		{
			SysMessage( "You have no musical instrument available" );
			return( -2 );
		}
	}

	bool fSuccess = Skill_UseQuick( SKILL_MUSICIANSHIP, iDifficultyToPlay );
	Sound( pInstrument->Use_Music( fSuccess ));
	if ( ! fSuccess )
	{
		SysMessage( "You play poorly." );
		return( -1 );	// Impossible.
	}
	return( 0 );	// success
}

bool CChar::Use_Repair( CItem * pItemArmor )
{
	// Attempt to repair the item.
	// If it is repairable.

	if ( pItemArmor == NULL || ! pItemArmor->Armor_IsRepairable())
	{
		SysMessage( "The item is not repairable" );
		return( false );
	}

	if ( pItemArmor->IsEquipped())
	{
		SysMessage( "Can't repair an item being worn !" );
		return( false );
	}
	if ( ! CanUse(pItemArmor))
	{
		SysMessage( "Can't repair this where it is !" );
		return( false );
	}

	if ( ! Skill_UseQuick( SKILL_ARMSLORE, GetRandVal(30)))
	{
		SysMessage( "You have trouble figuring out the item." );
		return( false );
	}

	if ( pItemArmor->m_itArmor.m_Hits_Cur >= pItemArmor->m_itArmor.m_Hits_Max )
	{
		SysMessage( "The item is already in full repair." );
		return( false );
	}

	m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), ITEM_ANVIL, 3 );
	if ( ! m_Act_p.IsValid())
	{
		SysMessage( "You must be near an anvil to repair" );
		return( false );
	}

	UpdateDir( m_Act_p );
	UpdateAnimate( ANIM_ATTACK_WEAPON );

	if ( ! Skill_UseQuick( SKILL_BLACKSMITHING, pItemArmor->m_itArmor.m_Hits_Max + 2*( pItemArmor->m_itArmor.m_Hits_Max - pItemArmor->m_itArmor.m_Hits_Cur )))
	{
		const TCHAR * pszText;
		if ( ! GetRandVal(6))
		{
			pszText = "really damage";
			pItemArmor->m_itArmor.m_Hits_Max --;
			pItemArmor->m_itArmor.m_Hits_Cur --;
		}
		else if ( ! GetRandVal(3))
		{
			pszText = "slightly damage";
			pItemArmor->m_itArmor.m_Hits_Cur --;
		}
		else
		{
			pszText = "fail to repair";
		}
		if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
		{
			pszText = "destroy";
		}

		CGString sMsg;
		sMsg.Format( "%s the %s", pszText, GetName());
		Emote( sMsg );

		if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
			pItemArmor->Delete();
		return( false );
	}

	pItemArmor->m_itArmor.m_Hits_Max = pItemArmor->m_itArmor.m_Hits_Cur;
	SysMessage( "You repair the item" );
	return( true );
}

void CChar::Use_EatQty( CItem * pFood, int iQty )
{
	if ( iQty <= 0 ) 
		return;

	if ( iQty > pFood->GetAmount())
		iQty = pFood->GetAmount();
	int iValue = pFood->m_pDef->GetVolume(); // some food should have more value than other !
	if ( ! iValue ) 
		iValue = 1;

	// Was the food poisoned ?
	// Did it have special (magical?) properties ?
	UpdateDir( pFood );
	Use_Consume( pFood, iQty );

	EatAnim( pFood->GetName(), iQty * iValue );
}

bool CChar::Use_Eat( CItem * pItemFood, int iQty )
{
	// What we can eat should depend on body type.
	// How much we can eat should depend on body size and current fullness.
	//
	// ??? monsters should be able to eat corpses / raw meat
	// ITEM_FOOD or ITEM_FOOD_RAW

	if ( ! CanMove( pItemFood ))
	{
		SysMessage( "You can't move the item." );
		return false;
	}
	if ( m_pDef->m_MaxFood == 0 )
	{
		SysMessage( "You are not capable of eating." );
		return false;
	}

	// Is this edible by me ?
	if ( ! CanEat( pItemFood ))
	{
		SysMessage( "You can't really eat this." );
		return( false );
	}

	const TCHAR * pMsg;
	int index = IMULDIV( m_food, 6, m_pDef->m_MaxFood );

	switch ( index )
	{
	case 0:
		pMsg = "You eat the food, but are still extremely hungry.";
		break;
	case 1:
		pMsg = "After eating the food, you feel much less hungry.";
		break;
	case 2:
	case 3:
		pMsg = "You eat the food, and begin to feel more satiated.";
		break;
	case 4:
		pMsg = "You are nearly stuffed, but manage to eat the food.";
		break;
	case 5:
		pMsg = "You feel quite full after consuming the food.";
		break;
	case 6:
	default:
		pMsg = "You are simply too full to eat any more!";
		break;
	}

	SysMessage( pMsg );
	if ( m_food >= m_pDef->m_MaxFood )
		return false;

	Use_EatQty( pItemFood, iQty );
	return( true );
}

bool CChar::Use_Drink( CItem * pItem )
{
	ITEMID_TYPE idbottle = ITEMID_NOTHING;

	if ( ! CanMove( pItem ))
	{
		SysMessage( "You can't move the item." );
		return false;
	}
	if ( pItem->GetID() == ITEMID_EMPTY_BOTTLE )
	{
		SysMessage( "It's empty." );
		return false;
	}

	static const WORD DrinkSounds[] = { 0x030, 0x031 };
	Sound( DrinkSounds[ GetRandVal( COUNTOF(DrinkSounds)) ] );
	UpdateAnimate( ANIM_EAT );

	if ( pItem->m_type == ITEM_BOOZE )
	{
		// Beer wine and liquor.
		int iStrength = 0;

		// Create ITEMID_PITCHER if drink a pitcher.
		// GLASS or MUG or Bottle ?

		// Get you Drunk, but check to see if we already are drunk
		CItem *pDrunkLayer = LayerFind( LAYER_FLAG_Drunk );
		if ( pDrunkLayer == NULL )
		{
			CItem * pSpell = Spell_Effect_Create( SPELL_Curse, LAYER_FLAG_Drunk, GetRandVal(10) + 1, 15*TICK_PER_SEC, this );
			pSpell->m_itSpell.m_charges = 10;	// how long to last.
		}
		else
		{
			// lengthen/strengthen the effect
			Spell_Effect_Remove( pDrunkLayer );
			pDrunkLayer->m_itSpell.m_charges += 10;
			pDrunkLayer->m_itSpell.m_skilllevel++;
			Spell_Effect_Add( pDrunkLayer );
		}
	}

	if ( pItem->m_type == ITEM_POTION )
	{
		// Time limit on using potions.
		if ( LayerFind( LAYER_FLAG_PotionUsed ))
		{
			SysMessage( "You can't drink another potion yet" );
			return false;
		}

		idbottle = ITEMID_EMPTY_BOTTLE;
		int iSkillQuality = pItem->m_itPotion.m_skillquality;

		switch ( pItem->m_itPotion.m_Type )
		{
		case POTION_WATER:	// No effect.
			break;
		case POTION_FLOAT_LESS:
		case POTION_FLOAT:
		case POTION_FLOAT_GREAT:
			break;
		case POTION_SHRINK:
			// Getting a pet to drink this is funny.
			if ( m_pPlayer ) 
				break;
			pItem->Delete();
			NPC_Shrink();	// this delete's the char !!!
			return true;
		case POTION_INVISIBLE:
			Spell_Effect_Create( SPELL_Invis, LAYER_FLAG_Potion, iSkillQuality, 120*TICK_PER_SEC, this );
			break;
		case POTION_MANA:
			UpdateStats( STAT_INT, 10+(iSkillQuality/100));
			break;
		case POTION_MANA_TOTAL:
			UpdateStats( STAT_INT, 20+(iSkillQuality/50), Stat_Get(STAT_INT) + 20 );
			break;
		case POTION_LAVA:
			break;

			// Normal and Nonstandard magic above...not below

		case POTION_AGILITY:
			Spell_Effect_Create( SPELL_Agility, LAYER_FLAG_Potion, 10 + (iSkillQuality/200), 120*TICK_PER_SEC, this );
			break;
		case POTION_AGILITY_GREAT:
			Spell_Effect_Create( SPELL_Agility, LAYER_FLAG_Potion, 20 + (iSkillQuality/100), 120*TICK_PER_SEC, this );
			break;
		case POTION_CURE_LESS:
		case POTION_CURE:
		case POTION_CURE_GREAT:
			SetPoisonCure( iSkillQuality, pItem->m_itPotion.m_Type == POTION_CURE_GREAT );
			break;
		case POTION_EXPLODE_LESS:
		case POTION_EXPLODE:
		case POTION_EXPLODE_GREAT:
			// Explode on you !
			g_World.Explode( this, GetTopPoint(), 1, 10+GetRandVal(iSkillQuality/40), DAMAGE_HIT|DAMAGE_GENERAL );
			idbottle = ITEMID_NOTHING;
			break;
		case POTION_HEAL_LESS:
			UpdateStats( STAT_STR, 3+GetRandVal(5)+(iSkillQuality/200));
			break;
		case POTION_HEAL:
			UpdateStats( STAT_STR, 13+GetRandVal(5)+(iSkillQuality/200));
			break;
		case POTION_HEAL_GREAT:
			UpdateStats( STAT_STR, 15+GetRandVal(9)+(iSkillQuality/100), m_Stat[STAT_STR] + 20 );
			break;
		case POTION_NIGHTSIGHT:
			Spell_Effect_Create( SPELL_Night_Sight, LAYER_FLAG_Potion, iSkillQuality, 2*60*60*TICK_PER_SEC, this );
			break;
		case POTION_POISON_LESS:
		case POTION_POISON:
		case POTION_POISON_GREAT:
		case POTION_POISON_DEADLY:
			// Poison me !
			SetPoison( iSkillQuality, this );
			break;
		case POTION_REFRESH:
			UpdateStats(STAT_DEX, 20+GetRandVal(15)+(iSkillQuality/200));
			break;
		case POTION_REFRESH_TOTAL:
			UpdateStats(STAT_DEX, 50+GetRandVal(20)+(iSkillQuality/100), Stat_Get(STAT_DEX) + 20 );
			break;
		case POTION_STR:
			Spell_Effect_Create( SPELL_Strength, LAYER_FLAG_Potion, 10+iSkillQuality/200, 120*TICK_PER_SEC, this );
			break;
		case POTION_STR_GREAT:
			Spell_Effect_Create( SPELL_Strength, LAYER_FLAG_Potion, 20+iSkillQuality/100, 120*TICK_PER_SEC, this );
			break;
		}

		// Give me the marker that i've used a potion.
		Spell_Effect_Create( SPELL_NONE, LAYER_FLAG_PotionUsed, iSkillQuality, 15*TICK_PER_SEC, this );
	}

	Use_Consume( pItem );

	// Create the empty bottle ?
	if ( idbottle != ITEMID_NOTHING )
	{
		ItemBounce( CItem::CreateScript( idbottle ));
	}
	return( true );
}

CChar * CChar::Use_Figurine( CItem * pItem, int iPaces )
{
	// NOTE: The figurine is NOT destroyed.

	DEBUG_CHECK( pItem->m_type == ITEM_FIGURINE || pItem->m_type == ITEM_EQ_HORSE );

	if ( pItem->m_uidLink.IsValidUID() &&
		pItem->m_uidLink.IsChar() &&
		pItem->m_uidLink != GetUID() &&
		! IsPriv( PRIV_GM ))
	{
		SysMessage( "This figurine is not yours" );
		return( NULL );
	}

	CChar * pPet = pItem->m_itFigurine.m_UID.CharFind();
	if ( pPet != NULL && pPet->IsDisconnected())
	{
		// Pull the creature out of idle space.
		pPet->ClearStat( STATF_Ridden );
	}
	else
	{
		CREID_TYPE id = pItem->m_itFigurine.m_ID;
		if ( ! id )
		{
			id = CCharBaseDisp::FindCharTrack( pItem->GetDispID());
			if ( ! id )
			{
				DEBUG_ERR(( "FIGURINE id 0%x, no creature\n", pItem->GetDispID()));
				return NULL;
			}
		}

		// I guess we need to create a new one ? (support old versions.)
		pPet = CreateNPC( id );
		pPet->SetName( pItem->GetName());
		if ( pItem->GetColor())
		{
			pPet->m_prev_color = pItem->GetColor();
			pPet->SetColor( pItem->GetColor());
		}
	}

	pItem->m_itFigurine.m_UID.ClearUID();

	if ( ! iPaces )
	{
		pPet->m_face_dir = m_face_dir;	// getting off ridden horse.
	}
	if ( ! pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ))
	{
		pPet->NPC_SetOwner( this );
	}
	pPet->MoveNear( pItem->GetTopLevelObj()->GetTopPoint(), iPaces );
	pPet->Update();

	if ( pItem->IsAttr( ATTR_MAGIC ))
	{
		pPet->UpdateAnimate( ANIM_CAST_DIR );
		pPet->SoundChar( CRESND_GETHIT );
	}
	else
	{
		pPet->SoundChar( CRESND_RAND1 );	// Horse winny
	}

	return pPet;
}

bool CChar::Use_Item( CItem * pItem, bool fLink )
{
	// An NPC could use these as well.
	// don't check distance here.
	// Could be a switch or something.
	// ARGS:
	//   fLink = this is the result of a linked action.
	// RETURN: 
	//   true = it activated.

	if ( pItem == NULL )
		return( false );

	bool fAction = true;
	switch ( pItem->GetID())
	{
	case ITEMID_SPYGLASS_1:
	case ITEMID_SPYGLASS_2:
		{
		// Spyglass will now tell you the moon phases...
		static const TCHAR * sPhases[8] =
		{
			"new moon",
			"waxing crescent",
			"first quarter",
			"waxing gibbous",
			"full moon",
			"waning gibbous",
			"third quarter",
			"waning crescent"
		};
		SysMessagef( "Trammel is in the %s phase.", sPhases[ g_World.GetMoonPhase(false) ]);
		SysMessagef( "Felucca is in the %s phase.", sPhases[ g_World.GetMoonPhase(true) ]);
		}
		if ( m_pArea && m_pArea->IsFlag( REGION_FLAG_SHIP ))
		{
			ObjMessage( pItem->Use_SpyGlass(this), this );
		}
		return true;

	case ITEMID_SEXTANT_1:
	case ITEMID_SEXTANT_2:
		if ( GetTopPoint().m_x > UO_SIZE_X_REAL)
		{
			ObjMessage( "I cannot tell where I'm at.", this );
		}
		else
		{
			CGString sTmp;
			sTmp.Format( "I am in %s, %s", m_pArea->GetName(), pItem->Use_Sextant(GetTopPoint()));
			ObjMessage( sTmp, this );
		}
		return true;

	case ITEMID_KINDLING1:
	case ITEMID_KINDLING2:
		return Use_Campfire( pItem );

	case ITEMID_DUMMY1:
	case ITEMID_DUMMY2:
		// Train dummy.
		return Use_TrainingDummy(pItem);

	case ITEMID_PICKPOCKET_NS:
	case ITEMID_PICKPOCKET_EW:
	case ITEMID_PICKPOCKET_NS2:
	case ITEMID_PICKPOCKET_EW2:
		return Use_PickPocketDip(pItem);

	case ITEMID_ARCHERYBUTTE_S:
	case ITEMID_ARCHERYBUTTE_E:
		// Archery Butte
		return Use_ArcheryButte(pItem);

	case ITEMID_LOOM1:
	case ITEMID_LOOM2:
		//pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICK_PER_SEC );
		SysMessage("Use thread or yarn on looms");
		return true;

	case ITEMID_SPINWHEEL1:
	case ITEMID_SPINWHEEL2:
		// Just make them spin.
		pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICK_PER_SEC );
		SysMessage("Use wool or cotton on spinning wheels");
		return true;

	case ITEMID_BELLOWS_1:
	case ITEMID_BELLOWS_5:
	case ITEMID_BELLOWS_9:
	case ITEMID_BELLOWS_13:
		// This is supposed to make the nearby fire pit hotter.
		pItem->Sound( 0x02b );
		pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICK_PER_SEC );
		return true;

	case ITEMID_BEEHIVE:
		// Get honey from it.
		{
			ITEMID_TYPE id = ITEMID_NOTHING;
			if ( ! pItem->m_itBeeHive.m_honeycount )
			{
				SysMessage( "The hive appears to be unproductive" );
			}
			else switch ( GetRandVal(3))
			{
			case 1:
				id = ITEMID_JAR_HONEY;
				break;
			case 2:
				id = ITEMID_BEE_WAX;
				break;
			}
			if ( id )
			{
				ItemBounce( CItem::CreateScript( id ));
				pItem->m_itBeeHive.m_honeycount --;
			}
			else
			{
				// ouch got stung. Poison bee sting ?
				SysMessage( "Ouch!  Bee sting!" );
				OnTakeDamage( GetRandVal(5), this, DAMAGE_POISON|DAMAGE_GENERAL );
			}
			pItem->SetTimeout( 15*60*TICK_PER_SEC );
		}
		return true;

	case ITEMID_BEDROLL_C:
		if ( ! pItem->IsTopLevel())
		{
putonground:
			SysMessage( "Put this on the ground to open it." );
			return( true );
		}
		pItem->SetDispID( GetRandVal(2) ? ITEMID_BEDROLL_O_EW : ITEMID_BEDROLL_O_NS );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_C_NS:
		if ( ! pItem->IsTopLevel()) 
			goto putonground;
		pItem->SetDispID( ITEMID_BEDROLL_O_NS );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_C_EW:
		if ( ! pItem->IsTopLevel()) 
			goto putonground;
		pItem->SetDispID( ITEMID_BEDROLL_O_EW );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_O_EW:
	case ITEMID_BEDROLL_O_NS:
		pItem->SetDispID( ITEMID_BEDROLL_C );
		pItem->Update();
		return true;
	}

	switch ( pItem->m_type )
	{
	case ITEM_MUSICAL:
		if ( ! Skill_Wait())
		{
			m_Act_Targ = pItem->GetUID();
			Skill_Start( SKILL_MUSICIANSHIP );
		}
		break;

	case ITEM_PLANT:
		fAction = Use_Plant( pItem );
		break;

	case ITEM_FIGURINE:
		// Create the creature here.
		fAction = ( Use_Figurine( pItem, 0 ) != NULL );
		if ( fAction )
		{
			pItem->Delete();
		}
		break;

	case ITEM_TRAP:
	case ITEM_TRAP_ACTIVE:
		// Explode or active the trap. (plus any linked traps.)
		if ( CanTouch( pItem->GetTopLevelObj()->GetTopPoint()))
		{
			OnTakeDamage( pItem->Use_Trap(), NULL, DAMAGE_HIT | DAMAGE_GENERAL );
		}
		else
		{
			pItem->Use_Trap();
		}
		break;

	case ITEM_SWITCH:
		// Switches can be linked to gates and doors and such.
		// Flip the switch graphic.
		pItem->SetSwitchState();
		break;

	case ITEM_PORT_LOCKED:	// Can only be trigered.
		if ( ! fLink && ! IsPriv(PRIV_GM))
		{
			SysMessage( "You can't move the gate." );
			return( true );
		}

	case ITEM_PORTCULIS:
		// Open a metal gate via a trigger of some sort.
		pItem->Use_Portculis();
		break;

	case ITEM_DOOR_LOCKED:
		if ( ! ContentFindKeyFor( pItem ))
		{
			SysMessage( "This door is locked.");
			if ( ! pItem->IsTopLevel())
				return( false );
			if ( pItem->IsAttr(ATTR_MAGIC))
			{
				// Show it's magic face.
				ITEMID_TYPE id = ( GetDispID() & DOOR_NORTHSOUTH ) ? ITEMID_DOOR_MAGIC_SI_NS : ITEMID_DOOR_MAGIC_SI_EW;
				CItem * pFace = CItem::CreateBase( id );
				pFace->SetDecayTime( 4*TICK_PER_SEC );
				pFace->MoveTo( pItem->GetTopPoint());
			}
			if ( ! IsPriv( PRIV_GM ))
				return true;
		}

	case ITEM_DOOR:
		{
			bool fOpen = pItem->Use_Door( fLink );
			if ( fLink || ! fOpen ) // Don't link if we are just closing the door.
				return true;
		}
		break;

	case ITEM_SHIP_PLANK:
		// If on the ship close it. if not teleport to it.
		if ( m_pArea->IsFlag( REGION_FLAG_SHIP ) &&
			m_pArea->IsMatchType( pItem->m_itContainer.m_lockUID ))
		{
			// Close it.
			// If on the ship close it. if not teleport to it.
			return( pItem->Ship_Plank( false ));
		}
		else if ( pItem->IsTopLevel())
		{
			// Move to it.
			CPointMap pntTarg = pItem->GetTopPoint();
			pntTarg.m_z ++;
			Spell_Teleport( pntTarg, true, false, ITEMID_NOTHING );
		}
		return true;

	case ITEM_SHIP_SIDE_LOCKED:
		if ( ! ContentFindKeyFor( pItem ))
		{
			SysMessage( "That is locked" );
			return true;
		}

	case ITEM_SHIP_SIDE:
		// Open it if we have the key or are already on the ship
		pItem->Ship_Plank( true );
		break;

	case ITEM_FOOD:
	case ITEM_FOOD_RAW:
		return Use_Eat( pItem );

	case ITEM_POTION:
	case ITEM_DRINK:
	case ITEM_BOOZE:
		return Use_Drink( pItem );

	case ITEM_LIGHT_OUT: // Can the light be lit ?
	case ITEM_LIGHT_LIT: // Can the light be doused ?
		fAction = pItem->Use_Light();
		break;

	case ITEM_CLOTHING:
	case ITEM_ARMOR:
	case ITEM_WEAPON_MACE_CROOK:
	case ITEM_WEAPON_MACE_PICK:
	case ITEM_WEAPON_MACE_SMITH:
	case ITEM_WEAPON_MACE_SHARP:
	case ITEM_WEAPON_SWORD:
	case ITEM_WEAPON_FENCE:
	case ITEM_WEAPON_BOW:
	case ITEM_WEAPON_MACE_STAFF:
		DEBUG_CHECK( pItem->IsArmorWeapon());
		return ItemEquip( pItem );

	case ITEM_WEB:
		// try to get out of web.
		return( Use_Item_Web( pItem ));

	default:
		fAction = false;
	}

	// Try to follow the link as well. (if it has one)
	CItem * pLinkItem = pItem->m_uidLink.ItemFind();
	if ( pLinkItem != NULL && pLinkItem != pItem )
	{
		static CItem * pItemFirst = NULL;	// watch out for loops.
		static int iCount = 0;
		if ( ! fLink )
		{
			pItemFirst = pItem;
			iCount = 0;
		}
		else
		{
			if ( pItemFirst == pItem )
				return true;	// kill the loop.
			if ( ++iCount > 64 )
			{
				return( true );
			}
		}
		fAction |= Use_Item( pLinkItem, true );
	}

	return( fAction );
}

bool CChar::Use_Obj( CObjBase * pObj, bool fTestTouch )
{
	if ( pObj == NULL )
		return( false );
	if ( IsClient())
	{
		return GetClient()->Event_DoubleClick( pObj->GetUID(), false, fTestTouch );
	}
	else
	{
		return Use_Item( dynamic_cast <CItem*>(pObj), fTestTouch );
	}
}

