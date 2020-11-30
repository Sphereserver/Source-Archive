//
// CCharNPCStatus.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Test things to judge what an NPC might be thinking. (want to do)
// But take no actions here.
//

#include "graysvr.h"	// predef header.

CREID_TYPE CChar::NPC_GetAllyGroupType(CREID_TYPE idTest)	// static
{
	switch ( idTest )
	{
	case CREID_MAN:
	case CREID_WOMAN:
	case CREID_GHOSTMAN:
	case CREID_GHOSTWOMAN:
		return( CREID_MAN );
	case CREID_ETTIN:
	case CREID_ETTIN_AXE:
		return( CREID_ETTIN );
	case CREID_ORC_LORD:
	case CREID_ORC:
	case CREID_ORC_CLUB:
		return( CREID_ORC );
	case CREID_DAEMON:
	case CREID_DAEMON_SWORD:
		return( CREID_DAEMON );
	case CREID_DRAGON_GREY:
	case CREID_DRAGON_RED:
	case CREID_DRAKE_GREY:
	case CREID_DRAKE_RED:
		return( CREID_DRAGON_GREY );
	case CREID_LIZMAN:
	case CREID_LIZMAN_SPEAR:
	case CREID_LIZMAN_MACE:
		return( CREID_LIZMAN );
	case CREID_RATMAN:
	case CREID_RATMAN_CLUB:
	case CREID_RATMAN_SWORD:
		return( CREID_RATMAN );
	case CREID_SKELETON:
	case CREID_SKEL_AXE:
	case CREID_SKEL_SW_SH:
		return( CREID_SKELETON );
	case CREID_TROLL_SWORD:
	case CREID_TROLL:
	case CREID_TROLL_MACE:
		return( CREID_TROLL );
	case CREID_Tera_Warrior:
	case CREID_Tera_Drone:
	case CREID_Tera_Matriarch:
		return( CREID_Tera_Drone );
	case CREID_Ophid_Mage:
	case CREID_Ophid_Warrior:
	case CREID_Ophid_Queen:
		return( CREID_Ophid_Warrior );
	case CREID_HORSE1:
	case CREID_HORSE4:
	case CREID_HORSE2:
	case CREID_HORSE3:
	case CREID_HORSE_PACK:
		return( CREID_HORSE1 );
	case CREID_BrownBear:
	case CREID_GrizzlyBear:
	case CREID_PolarBear:
		return( CREID_BrownBear );
	case CREID_Cow_BW:
	case CREID_Cow2:
	case CREID_Bull_Brown:
	case CREID_Bull2:
		return( CREID_Bull_Brown );
	case CREID_Ostard_Desert:
	case CREID_Ostard_Frenz:
	case CREID_Ostard_Forest:
		return( CREID_Ostard_Forest );
	case CREID_Sheep:
	case CREID_Sheep_Sheered:
		return( CREID_Sheep );
	case CREID_Hart:
	case CREID_Deer:
		return( CREID_Deer );
	case CREID_Pig:
	case CREID_Boar:
		return( CREID_Pig );
	case CREID_Llama:
	case CREID_LLAMA_PACK:
		return( CREID_Llama );
	}
	return( idTest );
}

int CChar::NPC_OnHearName( const TCHAR * pszText ) const
{
	// Did I just hear my name ?
	// RETURN: 
	//  index to skip past the name.

	const TCHAR * pszName = GetName();

	int i = FindStrWord( pszText, pszName );
	if ( i )
		return( i );

	// Named the chars type ? (must come first !)
	pszName = m_pDef->GetTradeName();
	for ( i=0; pszText[i] != '\0'; i++ )
	{
		if ( pszName[i] == '\0' )
		{
			// found name.
			while ( ISWHITESPACE( pszText[i] )) 
				i ++;
			return( i );	// Char name found
		}
		if ( toupper( pszName[i] ) != toupper( pszText[i] ))	// not the name.
			break;
	}

	return( 0 );
}

bool CChar::NPC_CanSpeak() const
{
	if ( m_pNPC == NULL )	// all players can speak.
		return( true );
	return( m_pNPC->m_Speech.GetCount() || m_pDef->m_Speech.GetCount() );
}

bool CChar::NPC_FightMayCast() const
{
	// This NPC could cast spells if they wanted to ?
	// check mana and anti-magic
#define NPC_MAGERY_MIN_CAST 300	// Min magery to cast.

	int iSkillVal = Skill_GetBase(SKILL_MAGERY);
	if ( iSkillVal < NPC_MAGERY_MIN_CAST )
		return( false );
	if ( m_pArea->IsFlag( REGION_ANTIMAGIC_DAMAGE | REGION_FLAG_SAFE ))	// can't cast here.
		return false;
	if ( m_StatMana < 5 )
		return( false );

	return( true );
}

bool CChar::NPC_IsSpawnedBy( const CItem * pItem ) const
{
	if ( ! IsStat( STATF_Spawned ))	// shortcut - i'm not a spawned.
		return( false );

	return( Memory_FindObjTypes( pItem, MEMORY_ISPAWNED ) != NULL );
}

bool CChar::NPC_IsOwnedBy( const CChar * pChar, bool fAllowGM ) const
{
	// fAllowGM = consider GM's to be owners of all NPC's

	if ( this == pChar )
		return( true );

	if ( fAllowGM && pChar->IsPriv( PRIV_GM ))
		return( pChar->GetPrivLevel() > GetPrivLevel());

	if ( ! IsStat( STATF_Pet ) || m_pPlayer )	// shortcut - i'm not a pet.
		return( false );

	return( Memory_FindObjTypes( pChar, MEMORY_IPET ) != NULL );
}

CChar * CChar::NPC_GetOwner() const
{
	// Assume i am a pet. Get my first (primary) owner.
	if ( ! IsStat( STATF_Pet )) 
		return( NULL );

	CItemMemory * pMemory = Memory_FindTypes( MEMORY_IPET );
	if ( pMemory == NULL )
	{
		// ClearStat( STATF_Pet );
		DEBUG_CHECK(0);
		return( NULL );
	}
	return( pMemory->m_uidLink.CharFind());
}

int CChar::NPC_GetTrainMax( SKILL_TYPE Skill ) const
{
	// What is the max I can train to ?
	int iMax = IMULDIV( g_Serv.m_iTrainSkillPercent, Skill_GetBase(Skill), 100 );
	if ( iMax > g_Serv.m_iTrainSkillMax )
		return( g_Serv.m_iTrainSkillMax );
	return( iMax );
}

bool CChar::NPC_CheckWalkHere( const CPointBase & pt, const CRegionBase * pArea ) const
{
	// Does the NPC want to walk here ? step on this item ?

	ASSERT( m_pNPC );	// not an NPC

	if ( m_pArea != NULL ) // most decisions are based on area.
	{
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD && ! IsStat( STATF_War ))
		{
			// Guards will want to stay in guard range.
			if ( m_pArea->IsGuarded() && ! pArea->IsGuarded())
			{
				return( false );
			}
		}

		if ( IsEvil() && Stat_Get(STAT_INT) > 20 )
		{
			if ( ! m_pArea->IsGuarded() && pArea->IsGuarded())		// too smart for this.
			{
				return( false );
			}
		}
	}

	// Is there a nasty object here that will hurt us ?
	CWorldSearch AreaItems( pt );
	while (true)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL )
			break;

		int zdiff = pItem->GetTopZ() - pt.m_z;
		if ( abs(zdiff) > 3 )
			continue;

		int iIntToAvoid = 10;	// how intelligent do i have to be to avoid this.
		switch ( pItem->m_type )
		{
		case ITEM_SHRINE:
		case ITEM_DREAM_GATE:
		case ITEM_ADVANCE_GATE:
			// always avoid.
			return( false );

		case ITEM_WEB:
			if ( GetDispID() == CREID_GIANT_SPIDER )
				continue;
			iIntToAvoid = 80;
			goto try_avoid;
		case ITEM_FIRE: // fire object hurts us ?
			if ( m_pDef->Can(CAN_C_FIRE_IMMUNE))	// i like fire.
				continue;
#if 0
			// standing on burning kindling shouldn't hurt us ?
			if ( pItem->GetDispID() >= ITEMID_CAMPFIRE && 
				pItem->GetDispID() < ITEMID_EMBERS )
				continue;
#endif
			iIntToAvoid = 20;	// most creatures recognize fire as bad.
			goto try_avoid;
		case ITEM_SPELL:
			iIntToAvoid = 150;
			goto try_avoid;
		case ITEM_TRAP:
			iIntToAvoid = 150;
			goto try_avoid;
		case ITEM_TRAP_ACTIVE:
			iIntToAvoid = 50;
			goto try_avoid;
		case ITEM_MOONGATE:
		case ITEM_TELEPAD:
try_avoid:
			if ( GetRandVal( Stat_Get(STAT_INT)) > GetRandVal( iIntToAvoid ))
				return( false );
			break;
		}
	}

	return( true );
}

////////////////////////////////////////////////////////////////////
// This stuff is still questionable.

int CChar::NPC_WantThisItem( CItem * pItem ) const
{
	//  This should be the ULTIMATE place to check if the NPC wants this in any way.
	//  May not want to use it but rather just put it in my pack.
	//
	// NOTE: 
	//  Don't check if i can see this or i can reach it.
	//  Also take into consideration that some items such as:
	// ex. i want to eat fruit off a fruit tree.
	// ex. i want raw food that may be cooked etc.
	// ex. I want a corpse that i can cut up and eat the parts.
	// ex. I want a chest to loot.
	// RETURN:
	//  0-100 percent = how bad do we want it ?

	if ( ! CanMove( pItem, false ))
	{
		// Some items like fruit trees, chests and corpses might still be useful.
		return( false );
	}

	bool bWantGold = false;
	if ( strstr( m_pDef->m_sDesires, "GOLD" ) != 0 )
		bWantGold = true;

	switch ( pItem->GetDispID())
	{
	case ITEMID_FOOD_MEAT_RAW:
		// I want this if I eat meat
		if ( strstr( m_pDef->m_sFoodType, "MEAT" ))
			return true;
		if ( strstr( m_pDef->m_sFoodType, "ANY" ))
			return true;
		break;
	case ITEMID_HIDES:
		// I want this if I eat leather
		if ( strstr( m_pDef->m_sDesires, "LEATHER" ))
			return true;
		if ( strstr( m_pDef->m_sFoodType, "LEATHER" ))
			return true;
		break;
	}
	switch ( pItem->m_type )
	{
	case ITEM_ARMOR:
	case ITEM_CLOTHING:
		break;

	case ITEM_AROCK:
		if ( strstr( m_pDef->m_sDesires, "ROCKS" ))
			return true;
		break;
	case ITEM_FOOD:
		// I want this if I eat ANY food type
		if (strstr( m_pDef->m_sFoodType, "ANY" ))
			return true;
		break;
	case ITEM_COIN:
		if ( bWantGold )
			return true;
	}

	// anything else to check for?
	if ( strstr( m_pDef->m_sFoodType, "ANY") ||
		strstr( m_pDef->m_sFoodType, "FRUIT"))
	{
		if ( pItem->m_pDef->IsFruit( pItem->GetDispID()))
			return true;
	}

	// If I'm smart and I want gold, I'll pick up valuables too (can be sold)
	if ( bWantGold &&
		Stat_Get(STAT_INT) > 50 &&
		pItem->GetBasePrice( false ) > 30 )
		return true;

	// I guess I don't want it
	return 0;
}

int CChar::NPC_WantToUseThisItem( const CItem * pItem ) const
{
	// Does the NPC want to use the item now ?
	// This may be reviewing items i have in my pack.
	//
	// ex. armor is better than what i'm wearing ?
	// ex. str potion in battle.
	// ex. heal potion in battle.
	// ex. food when i'm hungry.

	return( 0 );
}

int CChar::NPC_GetHostilityLevelToward( CChar * pCharTarg ) const
{
	// What is my general hostility level toward this type of creature ?
	//
	// based on:
	//  npc vs player, (evil npc's don't like players regurdless of align, xcept in town)
	//  karma (we are of different alignments)
	//  creature body type. (allie groups)
	//  hunger, (they could be food)
	//  memories of this creature.
	//
	// DO NOT consider:
	//   strength, he is far stronger or waeker than me. 
	//	 health, i may be near death.
	//   location (guarded area), (xcept in the case that evil people like other evils in town)
	//   loot, etc.
	//
	// RETURN: 
	//   100 = extreme hatred.
	//   0 = neutral.
	//   -100 = love them
	//

	if ( pCharTarg == NULL )
		return( 0 );
	if ( pCharTarg->IsStat( STATF_DEAD | STATF_INVUL | STATF_Stone ))
		return( 0 );

	ASSERT(m_pNPC);

	int iKarma = Stat_Get(STAT_Karma);

	int iHostility = 0;

	if ( IsEvil() &&	// i am evil.
		! m_pArea->IsGuarded() &&	// we are not in an evil town.
		pCharTarg->m_pPlayer )	// my target is a player.
	{
		// If i'm evil i give no benefit to players with bad karma.
		// I hate all players.
		// Unless i'm in a guarded area. then they are cool.
		iHostility = 51;
	}
	else if ( m_pNPC->m_Brain == NPCBRAIN_BESERK )	// i'm beserk.
	{
		// beserks just hate everyone all the time.
		iHostility = 100;
	}
	else if ( pCharTarg->m_pNPC &&	// my target is an NPC
		pCharTarg->m_pNPC->m_Brain != NPCBRAIN_BESERK &&	// ok to hate beserks.
		! g_Serv.m_fMonsterFight )		// monsters are not supposed to fight other monsters !
	{
		iHostility = -50;
		goto domemorybase;	// set this low in case we are defending ourselves. but not attack for hunger.
	}
	else
	{
		// base hostillity on karma diff.

		if ( iKarma < 0 )
		{
			// I'm evil.
			iHostility += ( - iKarma ) / 1024;
		}
		else if ( pCharTarg->IsEvil())
		{
			// I'm good and my target is evil.
			iHostility += ( iKarma ) / 1024;
		}

	}

	// Based on just creature type.

	if ( pCharTarg->m_pNPC )
	{
		// Human NPC's will attack humans .
		if ( GetDispID() == pCharTarg->GetDispID())
		{
			// I will never attack those of my own kind...even if starving
			iHostility -= 100;
		}
		else if ( NPC_GetAllyGroupType( GetDispID()) == NPC_GetAllyGroupType(pCharTarg->GetDispID()))
		{
			iHostility -= 50;
		}
		else if ( pCharTarg->m_pNPC->m_Brain == m_pNPC->m_Brain )	// My basic kind
		{
			// Won't attack other monsters. (unless very hungry)
			iHostility -= 30;
		}
	}
	else
	{
		// Not immediately hostile if looks the same as me.
		if ( ! IsHuman() && NPC_GetAllyGroupType( GetDispID()) == NPC_GetAllyGroupType(pCharTarg->GetDispID()))
		{
			iHostility -= 51;
		}
	}

	// How hungry am I? Could this creature be food ? 
	// some creatures are not appetising.
	{
	int iFoodLevel = GetFoodLevelPercent();
	if ( iFoodLevel < 50 && CanEat( pCharTarg ) > iFoodLevel )
	{
		if ( iFoodLevel < 10 )
		{
			iHostility += 20;
		}
		if ( iFoodLevel < 1 )
		{
			iHostility += 20;
		}
	}
	}

domemorybase:
	// I have been attacked/angered by this creature before ?
	CItemMemory * pMemory = Memory_FindObjTypes( pCharTarg, MEMORY_FIGHT|MEMORY_AGGREIVED |MEMORY_IRRITATEDBY|MEMORY_SAWCRIME );
	if ( pMemory )
	{
		iHostility += 50;
	}

	return( iHostility );
}

int CChar::NPC_GetAttackContinueMotivation( CChar * pChar, int iMotivation ) const
{
	// I have seen fit to attack them.
	// How much do i want to continue an existing fight ? cowardice ?
	// ARGS:
	//  iMotivation = My base motivation toward this creature.
	//
	// RETURN:
	// -101 = ? dead meat. (run away)
	// 
	// 0 = I'm have no interest.
	// 50 = even match.
	// 100 = he's a push over.

	ASSERT( m_pNPC );

	if ( m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		// Less interested the further away they are.
		return( iMotivation + 80 - GetDist( pChar ));
	}

	// Undead are fearless.
	if ( m_pNPC->m_Brain == NPCBRAIN_UNDEAD	||
		m_pNPC->m_Brain == NPCBRAIN_GUARD ||
		m_pNPC->m_Brain == NPCBRAIN_CONJURED )
		iMotivation += 90;

	// Try to stay on one target.
	if ( IsStat( STATF_War ) && m_Act_Targ == pChar->GetUID())
		iMotivation += 8;

	// Less interested the further away they are.
	iMotivation -= GetDist( pChar );

	if ( ! g_Serv.m_fMonsterFear )
	{
		return( iMotivation );
	}

	// I'm just plain stronger.
	iMotivation += ( Stat_Get(STAT_STR) - pChar->Stat_Get(STAT_STR));

	// I'm healthy.
	int iTmp = GetHealthPercent() - pChar->GetHealthPercent();
	if ( iTmp < -50 ) 
		iMotivation -= 50;
	else if ( iTmp > 50 ) 
		iMotivation += 50;

	// I'm smart and therefore more cowardly. (if injured)
	iMotivation -= Stat_Get(STAT_INT) / 16;

	return( iMotivation );
}

int CChar::NPC_GetAttackMotivation( CChar * pChar, int iMotivation ) const
{
	// Some sort of monster.
	// Am I stronger than he is ? Should I continue fighting ?
	// Take into consideration AC, health, skills, etc..
	// RETURN:
	// <-1 = dead meat. (run away)
	// 0 = I'm have no interest.
	// 50 = even match.
	// 100 = he's a push over.

	ASSERT( m_pNPC );
	if ( m_StatHealth <= 0 ) 
		return( -1 );	// I'm dead.
	if ( pChar == NULL ) 
		return( 0 );

	// Is the target interesting ?
	if ( pChar->m_pArea->IsFlag( REGION_FLAG_SAFE ))	// universal
		return( 0 );

	// If the area is guarded then think better of this.
	if ( pChar->m_pArea->IsGuarded() && m_pNPC->m_Brain != NPCBRAIN_GUARD )		// too smart for this.
	{
		iMotivation -= Stat_Get(STAT_INT) / 20;
	}

	// Owned by or is one of my kind ?
	CChar * pCharOwn = pChar->NPC_GetOwner();
	if ( pCharOwn == NULL )
	{
		pCharOwn = pChar;
	}

	iMotivation += NPC_GetHostilityLevelToward( pCharOwn );

	if ( iMotivation > 0 )
	{
		// Am i injured etc ?
		iMotivation = NPC_GetAttackContinueMotivation( pChar, iMotivation );
	}

#ifdef _DEBUG
	if ( g_Serv.m_wDebugFlags & DEBUGF_MOTIVATION )
	{
		DEBUG_MSG(( "NPC_GetAttackMotivation '%s' for '%s' is %d\n", GetName(), pChar->GetName(), iMotivation ));
	}
#endif
	return( iMotivation );
}

