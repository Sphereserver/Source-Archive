//
// CCharSkill.cpp
// Copyright Menace Software (www.menasoft.com).
//  CChar is either an NPC or a Player.
//

#include "graysvr.h"	// predef header.

extern "C"
{
	// for some reason, including the math.h file gave an error, so I'll declare it here.
	double _cdecl log10(double);
}

//----------------------------------------------------------------------

void CChar::Action_StartSpecial( CREID_TYPE id )
{
	// Take the special creature action.
	// lay egg, breath weapon (fire, lightning, acid, code, paralyze),
	//  create web, fire patch, fire ball,
	// steal, teleport, level drain, absorb magic, curse items,
	// rust items, stealing, charge, hiding, grab, regenerate, play dead.
	// Water = put out fire !

	UpdateAnimate( ANIM_CAST_AREA );

	switch ( id )
	{
	case CREID_FIRE_ELEM:
		// leave a fire patch.
		{
			CItem * pItem = CItem::CreateBase( GetRandVal(2) ? ITEMID_FX_FIRE_F_EW : ITEMID_FX_FIRE_F_NS );
			pItem->m_type = ITEM_FIRE;
			pItem->m_itSpell.m_spell = SPELL_Fire_Field;
			pItem->m_itSpell.m_skilllevel = 50 + GetRandVal(100);
			pItem->m_itSpell.m_charges = 1;
			pItem->m_uidLink = GetUID();	// Link it back to you
			pItem->SetDecayTime( 30*TICK_PER_SEC + GetRandVal(60*TICK_PER_SEC));
			pItem->MoveTo( GetTopPoint());
		}
		break;

	case CREID_GIANT_SPIDER:
		// Leave a web patch.
		{
			static const WORD Webs[] =
			{
				ITEMID_WEB1_1,
				ITEMID_WEB1_1+1,
				ITEMID_WEB1_1+2,
				ITEMID_WEB1_4,
			};
			CItem * pItem = CItem::CreateScript( (ITEMID_TYPE) Webs[ GetRandVal( COUNTOF(Webs))] );
			pItem->m_type = ITEM_WEB;
			pItem->MoveToCheck( GetTopPoint());
			pItem->SetDecayTime( 3*60*60*TICK_PER_SEC );
		}
		break;

	default:
		SysMessage( "You have no special abilities" );
		return;	// No special ability.
	}

	// loss of stamina for a bit.
	UpdateStats( STAT_DEX, -( 5 + GetRandVal(5)));	// The cost in stam.
}

//----------------------------------------------------------------------
// Skills

SKILL_TYPE CChar::Skill_GetBest() const // Which skill is the highest for character p
{
	SKILL_TYPE best_skill = SKILL_QTY;
	int best_value=-1;
	for ( int i=0;i<SKILL_QTY;i++)
	{
		int iVal = Skill_GetBase( (SKILL_TYPE)i );
		if ( iVal > best_value )
		{
			best_skill = (SKILL_TYPE) i;
			best_value = iVal;
		}
	}
	return best_skill;
}

int CChar::Skill_Linearity( SKILL_TYPE skill, int iLow, int iHigh ) const
{
	int iSkillVal = Skill_GetAdjusted( skill );
	iHigh -= iLow;
	return( iLow + IMULDIV( iHigh, iSkillVal, 1000 ));
}

short CChar::Skill_GetAdjusted( SKILL_TYPE skill ) const
{
	// Get the skill adjusted for str,dex,int

	// m_SkillStat is used to figure out how much
	// of the total bonus comes from the stats
	// so if it's 80, then 20% (100% - 80%) comes from
	// the stat (str,int,dex) bonuses

	// example:

	// These are the cchar's stats:
	// m_Skill[x] = 50.0
	// m_Stat[str] = 50, m_Stat[int] = 30, m_Stat[dex] = 20

	// these are the skill "defs":
	// m_SkillStat = 80
	// m_StatBonus[str] = 50
	// m_StatBonus[int] = 50
	// m_StatBonus[dex] = 0

	// Pure bonus is:
	// 50% of str (25) + 50% of int (15) = 40

	// Percent of pure bonus to apply to raw skill is
	// 20% = 100% - m_SkillStat = 100 - 80

	// adjusted bonus is: 8 (40 * 0.2)

	// so the effective skill is 50 (the raw) + 8 (the bonus)
	// which is 58 in total.

	ASSERT( IsSkillBase( skill ));
	int iPureBonus =
		( g_Serv.m_SkillDefs[skill]->m_StatBonus[STAT_STR] * m_Stat[STAT_STR] ) +
		( g_Serv.m_SkillDefs[skill]->m_StatBonus[STAT_INT] * m_Stat[STAT_INT] ) +
		( g_Serv.m_SkillDefs[skill]->m_StatBonus[STAT_DEX] * m_Stat[STAT_DEX] );

	int iAdjSkill = IMULDIV( 100 - g_Serv.m_SkillDefs[skill]->m_SkillStat, iPureBonus, 10000 );
	return( Skill_GetBase( (SKILL_TYPE) skill ) + iAdjSkill );
}

void CChar::Skill_SetBase( SKILL_TYPE skill, short wValue )
{
	ASSERT( IsSkillBase(skill));
	if ( wValue < 0 ) wValue = 0;
	m_Skill[skill] = wValue;
	if ( IsClient())
	{
		// Update the skills list
		m_pClient->addSkillWindow( skill );
	}
}

void CChar::Skill_Experience( SKILL_TYPE skill, int difficulty )
{
	// Give the char credit for using the skill.
	// More credit for the more difficult. or none if too easy
	// 
	// ARGS: 
	//  difficulty = skill target from 0-100
	//

	if ( ! IsSkillBase(skill))
		return;
	if ( m_pArea && m_pArea->IsFlag( REGION_FLAG_SAFE ))	// skills don't advance in safe areas.
		return;

	int iSkillVal = Skill_GetBase( skill );
	if ( difficulty < 0 )
	{
		// failure. Give a little experience for failure at low levels.
		if ( iSkillVal < 300 )
		{
			difficulty = (( min( -difficulty, iSkillVal/10 )) / 2 ) - 8;
		}
		else
		{
			difficulty = 0;
		}
	}
	if ( difficulty > 100 ) difficulty = 100;

	int iSkillMax;	// max advance for this skill
	if ( m_pPlayer )
	{
		iSkillMax = g_Serv.m_SkillClassDefs[m_pPlayer->m_SkillClass]->m_SkillLevelMax[skill];
		if ( m_pPlayer->Skill_GetLock(skill) >= SKILLLOCK_DOWN )
		{
			if ( iSkillVal > iSkillMax ) iSkillMax = iSkillVal;
		}
	}
	else
	{
		iSkillMax = 1000;
	}

	if ( iSkillVal < iSkillMax ) // Enough is enough
	{
		// ADV_RATE=2000,500,25 for ANATOMY (easy)
		// ADV_RATE=8000,2000,100 for alchemy (hard)
		// assume 100 = a 1 for 1 gain.
		// ex: 8000 = we must use it 80 times to gain .1
		// Higher the number = the less probable to advance.
		// Extrapolate a place in the range.

		int iSkillAdj = iSkillVal + ( iSkillVal - ( difficulty * 10 ));
		int iChance = g_Serv.m_SkillDefs[skill]->m_Adv.GetChance( iSkillAdj );
		if ( iChance <= 0 ) 
			return; // less than no chance ?
		int iRoll = GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Serv.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "%s=%d.%d Difficult=%d Gain Chance=%d.%d%% Roll=%d%%",
				g_Serv.m_SkillDefs[skill]->GetKey(),
				iSkillVal/10,(iSkillVal)%10,
				difficulty, iChance/10, iChance%10, iRoll/10 );
		}
#endif

		if ( iRoll <= iChance )
		{
			Skill_SetBase( skill, iSkillVal + 1 );
		}
	}

	////////////////////////////////////////////////////////
	// Dish out any stat gains

	int iSumOfStats = 0;

	// Stat effects are unrelated to advance in skill !
	for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
	{
		// Can't gain STR or DEX if morphed.
		if ( IsStat( STATF_Polymorph ) && i != STAT_INT ) 
			continue;

		int iStatVal = Stat_Get((STAT_TYPE)i);
		iSumOfStats += iStatVal;

		int iStatMax;
		if ( m_pPlayer )
		{
			iStatMax = g_Serv.m_SkillClassDefs[m_pPlayer->m_SkillClass]->m_StatMax[i];
		}
		else
		{
			iStatMax = 100;
		}
		if ( iStatVal >= iStatMax )
			continue;	// nothing grows past this. (even for NPC's)

		// You will tend toward these stat vals if you use this skill a lot.
		int iStatTarg = g_Serv.m_SkillDefs[ skill ]->m_Stat[i];
		if ( iStatVal >= iStatTarg ) 
			continue;		// you've got higher stats than this skill is good for.

		// ??? Building stats should consume food !!

		difficulty = IMULDIV( iStatVal, 1000, iStatTarg );
		int iChance = g_Serv.m_StatAdv[i].GetChance( difficulty );

		// adjust the chance by the percent of this that the skill uses.
		iChance = ( iChance * g_Serv.m_SkillDefs[skill]->m_StatBonus[i] * g_Serv.m_SkillDefs[skill]->m_SkillStat ) / 10000;

		int iRoll = GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Serv.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "%s Difficult=%d Gain Chance=%d.%d%% Roll=%d%%",
				g_Stat_Name[i], difficulty, iChance/10, iChance%10, iRoll/10 );
		}
#endif

		if ( iRoll <= iChance )
		{
			Stat_Set( (STAT_TYPE)i, iStatVal+1 );
			break;
		}
	}

	// Check for stats degrade.

	if ( m_pPlayer &&
		iSumOfStats > g_Serv.m_iAvgSumOfStats &&
		! IsStat( STATF_Polymorph ) &&
		! IsPriv( PRIV_GM ))
	{
		// We are at a point where our skills can degrade a bit.
		// In theory magical enhancements make us lazy !
		// int iMaxSumOfStats = m_pDef->m_MaxSumOfStats; // depends on creature type ?!

		if ( iSumOfStats > g_Serv.m_iMaxSumOfStats )
		{
			DEBUG_MSG(( "0%x '%s' Exceeds stat max %d\n", GetUID(), GetName(), iSumOfStats ));
		}

		int iChanceForLoss = GW_GetSCurve( g_Serv.m_iMaxSumOfStats - iSumOfStats, ( g_Serv.m_iMaxSumOfStats - g_Serv.m_iAvgSumOfStats ) / 4 );
		int iRoll = GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Serv.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "Loss Diff=%d Chance=%d.%d%% Roll=%d%%",
				g_Serv.m_iMaxSumOfStats - iSumOfStats,
				iChanceForLoss/10, iChanceForLoss%10, iRoll/10 );
		}
#endif

		if ( iRoll < iChanceForLoss )
		{
			// Find the stat that was used least recently and degrade it.
			int imin = STAT_STR;
			int iminval = INT_MAX;
			for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
			{
				if ( iminval > g_Serv.m_SkillDefs[skill]->m_StatBonus[i] )
				{
					imin = i;
					iminval = g_Serv.m_SkillDefs[skill]->m_StatBonus[i];
				}
			}

			int iStatVal = m_Stat[imin];
			if ( iStatVal > 10 )
			{
				Stat_Set( (STAT_TYPE)imin, iStatVal-1 );
			}
		}
	}
}

bool CChar::Skill_CheckSuccess( SKILL_TYPE skill, int difficulty )
{
	// PURPOSE:
	//  Check a skill for success or fail.
	//  DO NOT give experience here.
	// ARGS:
	//  difficulty = 0-100 = The point at which the equiv skill level has a 50% chance of success.
	// RETURN: 
	//	true = success in skill.
	//

	if ( ! IsSkillBase(skill) || IsPriv( PRIV_GM ))
		return( true );

	int iSkillVal = Skill_GetAdjusted(skill);
	int iChanceForSuccess = GW_GetSCurve( iSkillVal - ( difficulty * 10 ), SKILL_VARIANCE );
	int iRoll = GetRandVal(1000);

#ifdef _DEBUG
	// Print out the skillvalues for now for debugging purposes
	if ( IsPriv( PRIV_DETAIL ) &&
		GetPrivLevel() >= PLEVEL_GM &&
		( g_Serv.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
	{
		SysMessagef( "%s=%d.%d Difficult=%d Success Chance=%d%% Roll=%d%%",
			g_Serv.m_SkillDefs[skill]->GetKey(),
			iSkillVal/10,(iSkillVal)%10,
			difficulty, iChanceForSuccess/10, iRoll/10 );
	}
#endif

	return( iRoll <= iChanceForSuccess  );
}

int CChar::Skill_TestScript( TCHAR * pszCmd )
{
	// RETURN: Difficulty 0-100%
	//  -1 = failed.

	TCHAR * ppCmd[2];
	ParseCmds( pszCmd, ppCmd, COUNTOF( ppCmd ));

	SKILL_TYPE iSkillReq = g_Serv.FindSkillKey( ppCmd[0] );
	if ( iSkillReq == SKILL_NONE )
	{
		DEBUG_ERR(( "Skill script ERROR '%s' bad 'TEST' skill '%s'\n", GetName(), ppCmd[0] ));
		return( -1 );
	}

	// A min skill is required.
	int iDifficulty = Exp_GetVal( ppCmd[1] );
	if ( Skill_GetBase(iSkillReq) < iDifficulty - 100 )	// give 5 point leeway.
		return( -1 );

	return( iDifficulty / 10 );
}

bool CChar::Skill_UseQuick( SKILL_TYPE skill, int difficulty )
{
	// ARGS:
	//  difficulty = 0-100
	// Use a skill instantly. No wait at all.
	// No interference with other skills.
	if ( ! Skill_CheckSuccess( skill, difficulty ))
	{
		Skill_Experience( skill, -difficulty );
		return( false );
	}
	Skill_Experience( skill, difficulty );
	return( true );
}

void CChar::Skill_Cleanup( void )
{
	// We are done with the skill.
	// We may have succeeded, failed, or cancelled.
	m_Act_Difficulty = 0;
	m_Act_SkillCurrent = SKILL_NONE;
	SetTimeout( TICK_PER_SEC );		// we should get a brain tick next time.
}

void CChar::Skill_Fail( bool fCancel )
{
	// We still get some credit for having tried.

	SKILL_TYPE sk = GetActiveSkill();
	if ( sk == SKILL_NONE )
		return;
	DEBUG_CHECK( IsSkillBase(sk) || IsSkillNPC(sk));
	if ( ! IsSkillBase(sk) )
		return;

	if ( m_Act_Difficulty > 0 )
	{
		m_Act_Difficulty = - m_Act_Difficulty;
	}

	const TCHAR * pMsg = g_Serv.m_SkillDefs[ sk ]->m_sFailMsg;
	switch ( sk )
	{
	case SKILL_ALCHEMY:
		Emote( "toss the failed mixture from the mortar" );
		// Sound( 0x031 );	// drinking ???
		Skill_Cleanup();
		return;
	case SKILL_ANATOMY:
	case SKILL_ANIMALLORE:
	case SKILL_ITEMID:
	case SKILL_ARMSLORE:
	case SKILL_PARRYING:
	case SKILL_BEGGING:
	case SKILL_BLACKSMITHING:
	case SKILL_BOWCRAFT:
	case SKILL_PEACEMAKING:
	case SKILL_CAMPING:
	case SKILL_CARPENTRY:
	case SKILL_CARTOGRAPHY:
		break;
	case SKILL_COOKING:
		// Cooking m_Act_Targ food object. done.
		// Burn food
		Use_Consume( m_Act_Targ.ItemFind());
		break;
	case SKILL_DETECTINGHIDDEN:
	case SKILL_ENTICEMENT:
	case SKILL_EVALINT:
		break;
	case SKILL_VETERINARY:
	case SKILL_HEALING:
		{
			// ruin the bandages. = m_Act_TargPrv
			CItem * pBandage = m_Act_TargPrv.ItemFind();
			if ( pBandage == NULL ) 
				break;
			if ( pBandage->m_type != ITEM_BANDAGE ) 
				break;
			Use_Consume( pBandage );
		}
		break;
	case SKILL_FISHING:
	case SKILL_FORENSICS:
	case SKILL_HERDING:	// 20
	case SKILL_HIDING:
	case SKILL_Stealth:
		break;
	case SKILL_PROVOCATION:
		{
			// Might just attack you !
			CChar * pCharProv = m_Act_TargPrv.CharFind();
			if ( pCharProv == NULL )
			{
				break;
			}
			pCharProv->Attack( this );
		}
		break;
	case SKILL_INSCRIPTION:
		// Ruin the scroll.
		{
			CItem * pScroll = m_Act_Targ.ItemFind();
			if ( pScroll != NULL && pScroll->IsSameDispID( ITEMID_SCROLL2_BLANK ))
			{
				Use_Consume( pScroll );
			}
			if ( g_Serv.m_fReagentLossFail )
			{
				Spell_CanCast( m_atMagery.m_Spell, false, this, false );
			}
		}
		break;

	case SKILL_LOCKPICKING:
		// Possibly break the pick ?
		// m_Act_TargPrv = the lock pick
		{
			CItem * pPick = m_Act_TargPrv.ItemFind();
			if ( pPick != NULL &&
				pPick->IsSameDispID( ITEMID_LOCKPICK1 ) &&	// check for item getting lost somehow.
				!GetRandVal(3))
			{
				SysMessage( "You broke your pick!" );
				Use_Consume( pPick );
			}
		}
		break;
	case SKILL_MAGERY:
		Effect( EFFECT_OBJ, ITEMID_FX_SPELL_FAIL, this, 1, 30 );
		Sound( SOUND_SPELL_FIZZLE );
		if ( g_Serv.m_fReagentLossFail )
		{
			// consume the regs.
			Spell_CanCast( m_atMagery.m_Spell, false, m_Act_TargPrv.ObjFind(), false );
		}
		break;
	case SKILL_MAGICRESISTANCE:
	case SKILL_TACTICS:
		break;
	case SKILL_SNOOPING:
		// Did people see us try ?
		Skill_Snoop( false, true );
		break;
	case SKILL_MUSICIANSHIP:
		break;
	case SKILL_POISONING:	// 30
		// m_Act_Targ = poison.
		// m_Act_TargPrv = weapon/food
		break;
	case SKILL_SPIRITSPEAK:
		break;
	case SKILL_STEALING:
		// Did people see us try ?
		Skill_Steal( false, true );
		break;
	case SKILL_TAILORING:
		break;
	case SKILL_TAMING:
		break;
	case SKILL_TASTEID:
		break;
	case SKILL_TINKERING:
		break;
	case SKILL_TRACKING: // This skill didn't fail, it just ended/went out of range, etc...
		ObjMessage( "You have lost your quary.",this); // say this instead of the failure message
		Skill_Cleanup();
		return;

	case SKILL_RemoveTrap:
		// Trigger the trap ?
		{
			CItem * pTrap = m_Act_Targ.ItemFind();
			if ( ! CanTouch(pTrap) || pTrap->m_type != ITEM_TRAP )
			{
				fCancel = true;
				break;
			}
			Use_Item( pTrap );
		}
		break;

	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:

	case SKILL_LUMBERJACKING:
	case SKILL_MINING:
		break;
	}
	if ( pMsg != NULL ) SysMessage( pMsg );

	// Get some experience for failure ?
	if ( ! fCancel )
	{
		Skill_Experience( sk, m_Act_Difficulty );
	}
	Skill_Cleanup();
}

const TCHAR * CChar::Skill_GetName( bool fUse ) const
{
	// Name the current skill we are doing.

	SKILL_TYPE skill = GetActiveSkill();
	if ( skill <= SKILL_NONE )
	{
		return( "Idling" );
	}
	if ( IsSkillBase(skill))
	{
		if ( ! fUse )
		{
			return( g_Serv.m_SkillDefs[skill]->GetKey());
		}

		TCHAR * pszText = GetTempStr();
		sprintf( pszText, _TEXT("use %s"), g_Serv.m_SkillDefs[skill]->GetKey());
		return( pszText );
	}

	switch ( skill )
	{
	case NPCACT_FOLLOW_TARG: return( "Following" );
	case NPCACT_STAY: return( "Staying" );
	case NPCACT_GOTO: return( "GoingTo" );
	case NPCACT_WANDER: return( "Wandering" );
	case NPCACT_FLEE: return( "Fleeing" );
	case NPCACT_TALK: return( "Talking" );
	case NPCACT_TALK_FOLLOW: return( "TalkFollow" );
	case NPCACT_GUARD_TARG: return( "Guarding" );
	case NPCACT_GO_HOME: return( "GoingHome" );
	case NPCACT_GO_FETCH: return( "Fetching" );
	case NPCACT_BREATH: return( "Breathing" );
	case NPCACT_EATING:	return( "Eating" );
	case NPCACT_LOOTING: return( "Looting" );
	case NPCACT_THROWING: return( "Throwing" );
	}

	return( "Thinking" );
}

void CChar::Skill_SetTimeout()
{
	SKILL_TYPE skill = GetActiveSkill();
	ASSERT( IsSkillBase(skill));
	SetTimeout( g_Serv.m_SkillDefs[skill]->m_wDelay );
}

bool CChar::Skill_MiningSmelt( CItem * pItemOre, CItem * pItemTarg )
{
	// pItemTarg = forge or another pile of ore.
	// RETURN: true = success.
	if ( pItemOre == NULL || pItemOre->m_type != ITEM_ORE )
	{
		SysMessage( "You need to target the ore you want to smelt" );
		return( false );
	}

	if ( pItemTarg != NULL &&
		pItemTarg->m_type == ITEM_ORE )
	{
		// combine piles.
		if ( pItemTarg->GetColor() != pItemOre->GetColor())
			return( false );
		if ( pItemTarg == pItemOre )
			return( false );
		pItemTarg->SetAmountUpdate( pItemOre->GetAmount() + pItemTarg->GetAmount());
		pItemOre->Delete();
		return( true );
	}

#define SKILL_SMELT_FORGE_DIST 3

	if ( pItemTarg != NULL && pItemTarg->IsTopLevel() &&
		pItemTarg->m_type == ITEM_FORGE )
	{
		m_Act_p = pItemTarg->GetTopPoint();
		if ( GetDist( pItemTarg ) > SKILL_SMELT_FORGE_DIST )
		{
			SysMessage( "You are too far away from the forge." );
			return( false );
		}
	}
	else
	{
		m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), ITEM_FORGE, SKILL_SMELT_FORGE_DIST );
		if ( ! m_Act_p.IsValid())
		{
			SysMessage( "You must be near a forge to smith ingots" );
			return( false );
		}
	}

	// The ore is on the ground
	if ( GetDist( pItemOre ) > 1 )
	{
		SysMessage( "You are too far away from the ore." );
		return( false );
	}

	const COreDef * pOre = g_Serv.GetOreColor( pItemOre->GetColor());
	if ( pOre && Skill_GetAdjusted( SKILL_MINING ) < pOre->m_iMinSkill )
	{
		SysMessage("You lack the skill to smelt this ore");
		return false;
	}

	// Make sure we are near the forge
	UpdateDir( m_Act_p );

	// try to make ingots.
	if ( ! Skill_UseQuick( SKILL_MINING, GetRandVal(70)))
	{
		SysMessage( "You smelt the ore but are left with nothing useful." );
		// Lose up to half the resources.
		Use_Consume( pItemOre, GetRandVal( pItemOre->GetAmount() / 2 ) + 1 );
		return( false );
	}

	// Payoff - What do i get ?
	if ( pOre == NULL )
	{
		ITEMID_TYPE id;
		switch ( pItemOre->GetColor())
		{
		case COLOR_YELLOW:
		case COLOR_GOLDMETAL: id = ITEMID_INGOT_GOLD; break;
		case COLOR_GRAY:	  id = ITEMID_INGOT_SILVER; break;
		case COLOR_ORANGE_DARK: id = ITEMID_INGOT_COPPER; break;
		default: id = ITEMID_INGOT_IRON; break;
		}
		CItem * pIngots = CItem::CreateScript( id );
		pIngots->SetAmount( pItemOre->GetAmount());
		Use_Consume( pItemOre, pItemOre->GetAmount());
		ItemBounce( pIngots );
	}
	else
	{
		// The new way.
		// This is the one
		CItem * pIngots = CItem::CreateScript( pOre->m_wIngotItem );
		if ( pOre->m_wIngotItem == ITEMID_INGOT_IRON )
		{
			// Either this is really iron, or there isn't an ingot defined for this guy
			CGString sName;
			sName.Format( "%s ingot", pOre->GetName());
			pIngots->SetName( sName );
			pIngots->SetColor( pOre->m_wColor );
		}

		pIngots->SetAmount( pItemOre->GetAmount());
		Use_Consume( pItemOre, pItemOre->GetAmount());
		ItemBounce( pIngots );
	}

	return true;
}

bool CChar::Skill_Mining()
{
	// SKILL_MINING
	// m_Act_p = the point we want to mine at.
	// resource check  to ITEM_ORE. How much can we get ?
	int iSkillVal = Skill_GetBase(SKILL_MINING);

	CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_ROCK, false );
	if ( ! pResBit )	// nothing here.
		return false;

	const COreDef * pOre = NULL;
	if ( pResBit->IsAttr( ATTR_NEWBIE ))
	{
		pOre = g_Serv.GetOreEntry( iSkillVal );
		if ( pOre == NULL ) 
			return false;
		pResBit->SetAmount( GetRandVal( pOre->m_iMaxAmount - pOre->m_iMinAmount ) + pOre->m_iMinAmount );
		pResBit->m_Attr &= ~ATTR_NEWBIE;
		pResBit->SetColor( pOre->m_wColor );
	}
	else
	{
		// Find the ore type based on color.
		pOre = g_Serv.GetOreColor( pResBit->GetColor());
		if ( pOre == NULL )
			return false;
	}

	int iAmount = Use_Consume( pResBit, 2 + GetRandVal(iSkillVal/200+1));	// amount i want.

	CItem * pItem = CItem::CreateScript( ITEMID_ORE_1 );
	pItem->SetAmount( iAmount );

	// The new way
	pItem->SetColor( pOre->m_wColor );
	CGString sName;
	sName.Format( "%s ore", pOre->GetName());
	pItem->SetName( sName );

	// Perhaps we should test for gems etc aswell?

	ItemBounce( pItem );
	return( true );
}

bool CChar::Skill_Tracking( CObjUID uidTarg, DIR_TYPE & dirPrv, int iDistMax )
{
	CObjBase * pObj = uidTarg.ObjFind();
	if ( pObj == NULL )
	{
		return false;
	}

	CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
	if ( pObjTop == NULL )
	{
		return( false );
	}

	int dist = GetTopDist3D( pObjTop );	// disconnect = 0xFFFF
	if ( dist > iDistMax )
	{
		return false;
	}

	DIR_TYPE dir = GetDir( pObjTop );
	if (( dirPrv != dir ) || ! GetRandVal(10))
	{
		dirPrv = dir;
		CGString sMsgDist;
		if ( dist )
		{
			const TCHAR * pszDist;
			if ( dist < 16 ) // Closing in message?
				pszDist = " near";
			else if ( dist < 32 )
				pszDist = "";
			else if ( dist < 100 )
				pszDist = " far";
			else
				pszDist = " very far";
			sMsgDist.Format( _TEXT( "%s to the %s" ), pszDist, Dirs[ dir ] );
		}
		else
		{
			sMsgDist = _TEXT(" here");
		}

		CGString sMsg;
		sMsg.Format( "%s is%s%s", pObj->GetName(),
			pObjTop->IsDisconnected() ? " disconnected" : "",
			sMsgDist );
		ObjMessage( sMsg, this );
	}

	return true;		// keep the skill active.
}

bool CChar::Skill_Done()
{
	// We just finished using a skill. ASYNC timer expired.
	// m_Act_Skill = the skill.
	// Consume resources that have not already been consumed.
	// Confer the benefits of the skill.
	// calc skill gain based on this.
	// RETURN: Did we succeed or fail ?
	//  if False then we must print the fail msg.
	//  true = print nothing.

	SKILL_TYPE skill = GetActiveSkill();
	if ( skill == SKILL_NONE )
		return true;

	if ( IsClient())	// purely informational
	{
		int fSuccess = GetClient()->OnSkill_Done( skill, m_Act_Targ );
		if ( ! fSuccess )	// put up it's own fail msg.
		{
			goto cleanup;
		}
	}

	// multi stroke tried stuff here first.
	// or stuff that never really fails.
	switch ( skill )
	{
	case SKILL_ALCHEMY:
		//  OK, we know potion being attempted and the bottle
		//  it's going in....do a loop for each reagent
		if ( ! g_Serv.m_PotionDefs.IsValidIndex( m_atAlchemy.m_Potion ))
		{
			DEBUG_ERR(( "Alchemy bad potion type\n" ));
			return( false );
		}
		if ( m_atAlchemy.m_Stroke_Count < g_Serv.m_PotionDefs[m_atAlchemy.m_Potion]->m_RegReq )
		{
			CItem * pReg = m_Act_Targ.ItemFind();
			if ( pReg == NULL)	// prevent exploit
			{
				SysMessage( "Hmmm, you lack the reagents for this potion." );
				return false;
			}

			// Keep trying and grinding
			if ( GetTopSector()->GetComplexity() < 5 )
			{
				CGString sSpeak;
				sSpeak.Format(( m_atAlchemy.m_Stroke_Count == 0 ) ?
					"start grinding some %s in the mortar" :
					"add more %s and continue grinding", pReg->GetName());
				Emote( sSpeak );
			}

			Use_Consume( pReg );
			Sound( 0x242 );
			m_atAlchemy.m_Stroke_Count ++;
			Skill_SetTimeout();
			return true;	// keep active.
		}
		break;

	case SKILL_BEGGING:
		if ( m_pNPC ) 
			return true;	// Keep it active.
		break;

	case SKILL_BOWCRAFT:
		UpdateAnimate( ANIM_SALUTE );
		Sound( 0x055 );
		break;

	case SKILL_BLACKSMITHING:
		// m_atCreate.m_ItemID = create this item
		Sound( 0x02a );

		m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), ITEM_FORGE, 3 );
		if ( ! m_Act_p.IsValid())
		{
			SysMessage( "You must be near a forge to smith" );
			return false;
		}

		UpdateDir( m_Act_p );	// toward the forge
		if ( m_atCreate.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atCreate.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_WEAPON );	// ANIM_ATTACK_1H_DOWN
			Skill_SetTimeout();
			return true;	// keep active.
		}
		break;

	case SKILL_CARPENTRY:
		// m_atCreate.m_ItemID = create this item
		Sound( 0x023d );
		if ( m_atCreate.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atCreate.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_WEAPON );
			Skill_SetTimeout();
			return true;	// keep active.
		}
		break;

	case SKILL_DETECTINGHIDDEN:
		// Look around for who is hiding.
		// Detect them based on skill diff.
		// ??? Hidden objects ?
		{
			CWorldSearch Area( GetTopPoint(), ( Skill_GetAdjusted(SKILL_DETECTINGHIDDEN) / 8 ) + 1 );
			bool fFound = false;
			while (true)
			{
				CChar * pChar = Area.GetChar();
				if ( pChar == NULL ) 
					break;
				if ( pChar == this ) 
					continue;
				if ( ! pChar->IsStat( STATF_Invisible | STATF_Hidden ))
					continue;
				// Try to detect them.
				if ( pChar->IsStat( STATF_Hidden ))
				{
					// If there hiding skill is much better than our detect then stay hidden
				}
				pChar->Reveal();
				SysMessagef( "You find %s", pChar->GetName());
				fFound = true;
			}
			if ( ! fFound )
			{
				return false;
			}
		}
		break;

	case SKILL_LUMBERJACKING:
		// Play the chopping sound
		if ( ! CanTouch(m_Act_p) || GetTopPoint().GetDist3D( m_Act_p ) > 3 )
		{
			SysMessage( "That's too far away to chop." );
			return( false );
		}
		Sound( 0x13e );	// 0135, 013e, 148, 14a
		UpdateDir( m_Act_p );
		if ( m_atCreate.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atCreate.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_WEAPON );
			Skill_SetTimeout();
			return true;	// keep active.
		}
		break;

	case SKILL_MINING:
		// Pick a "mining" type sound
		if ( GetTopPoint().GetDist( m_Act_p ) > 2 )
		{
			SysMessage("That is too far away." );
			return( false );
		}
		Sound( ( GetRandVal(2)) ? 0x125 : 0x126 );
		UpdateDir( m_Act_p );
		if ( m_atCreate.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atCreate.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_1H_DOWN );
			Skill_SetTimeout();
			return true;	// keep active.
		}
		break;

	case SKILL_MUSICIANSHIP:
		break;

	case SKILL_PEACEMAKING:
		// make peace if possible. depends on who is listening/fighting.
		{
			CWorldSearch Area( GetTopPoint(), ( Skill_GetAdjusted(SKILL_PEACEMAKING) / 8 ) + 1 );
			while (true)
			{
				CChar * pChar = Area.GetChar();
				if ( pChar == NULL ) 
					return( false );
				if ( pChar == this ) 
					continue;
				break;
			}
		}
		break;

	case SKILL_TAMING:
		{
			static const TCHAR * szTameSpeak[] =
			{
				"I won't hurt you.",
				"I always wanted a %s like you",
				"Good %s",
				"Here %s",
			};
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
			{
				return( false );
			}
			if ( GetTopDist3D( pChar ) > 10 )
			{
				SysMessage( "You are too far away" );
				return( false );
			}
			UpdateDir( pChar );
			CGString sSpeak;

			// Create the memory of being tamed to prevent lame macroers
			if ( pChar->Memory_FindObjTypes( this, MEMORY_SPEAK ))
			{
				// See if I tamed it before
				// I did, no skill to tame it again
				sSpeak.Format( "The %s remembers you and accepts you once more as it's master.", pChar->GetName());
				ObjMessage( sSpeak, pChar );

				pChar->NPC_SetOwner( this );
				pChar->m_food = 50;	// this is good for something.
				pChar->m_Act_Targ = GetUID();
				pChar->Skill_Start( NPCACT_FOLLOW_TARG );
				goto cleanup;
			}

			sSpeak.Format( szTameSpeak[ GetRandVal( COUNTOF( szTameSpeak )) ], pChar->GetName());
			Speak( sSpeak );

			if ( ! IsPriv( PRIV_GM ))
			{
				if ( m_atTaming.m_Stroke_Count )
				{
					// Keep trying and updating the animation
					m_atTaming.m_Stroke_Count --;
					Skill_SetTimeout();
					return true;
				}
			}
		}
		break;

	case SKILL_TRACKING:
		if ( ! Skill_Tracking( m_Act_Targ, m_atTracking.m_PrvDir, Skill_GetAdjusted(SKILL_TRACKING)/20 + 10 ))
			return( false );
		Skill_SetTimeout();		// next update.
		return( true );	// keep it active.

	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:
		// Hit or miss my current target.
		if ( IsStat( STATF_War ))
		{
			if ( m_atFight.m_War_Swing_State <= WAR_SWING_EQUIPPING ||
				m_atFight.m_War_Swing_State > WAR_SWING_SWINGING )
				m_atFight.m_War_Swing_State = WAR_SWING_READY;  // Waited my recoil time. So I'm ready.
			if ( HitTry())
				return true;	// Stay in the skill till we hit.
		}
		break;
	}

	if ( m_Act_Difficulty < 0 )
	{
		// Was Bound to fail. But we had to wait for the timer anyhow.
		return false;
	}

	// Success for the skill.
	switch ( skill )
	{
	case SKILL_ALCHEMY:
		{
			// Consume the targeted bottle
			CItem * pItem = m_Act_TargPrv.ItemFind();
			if ( pItem == NULL || pItem->GetID() != ITEMID_EMPTY_BOTTLE ) // Prevent another exploit
			{
				pItem = ContentFind( ITEMID_EMPTY_BOTTLE );
				if ( pItem == NULL )
				{
					SysMessage( "Hmmm, your bottle disappeared." );
					return false ;
				}
			}
			Use_Consume( pItem );

			if ( ! g_Serv.m_PotionDefs.IsValidIndex(m_atAlchemy.m_Potion))
				return( false );

			// Create the potion, set various properties,
			// put in pack
			pItem = CItem::CreateScript( g_Serv.m_PotionDefs[m_atAlchemy.m_Potion]->m_potionID );
			ASSERT( pItem );

			CGString sName;
			sName.Format( "%s potion", g_Serv.m_PotionDefs[m_atAlchemy.m_Potion]->GetName());
			pItem->SetName(sName);
			pItem->m_itPotion.m_Type = m_atAlchemy.m_Potion;
			pItem->m_itPotion.m_skillquality = Skill_GetAdjusted( SKILL_ALCHEMY );
			pItem->SetColor( g_Serv.m_PotionDefs[m_atAlchemy.m_Potion]->m_color );
			ItemBounce( pItem );

			Emote( "pour the completed potion into a bottle" );
			Sound( 0x240 );	// pouring noise.
		}
		break;

	case SKILL_CARTOGRAPHY:
		// Selected a map type and now we are making it.
		// m_Act_Cartography_Dist = the map distance.
		{
			// Find the blank map to write on first.
			CItem * pItem = ContentFindType( ITEM_MAP, 0 );
			if ( pItem == NULL )
			{
				SysMessage( "You have no blank parchment to draw on" );
				return( false );
			}
			CPointMap pnt = GetTopPoint();
			if ( pnt.m_x >= UO_SIZE_X_REAL )
			{
				SysMessage( "You can't seem to figure out your surroundings." );
				return( false );
			}
			Use_Consume( pItem, 1 );
			// Get a valid region.
			CRectMap rect;
			rect.SetRect( pnt.m_x - m_atCartography.m_Dist,
				pnt.m_y - m_atCartography.m_Dist,
				pnt.m_x + m_atCartography.m_Dist,
				pnt.m_y + m_atCartography.m_Dist );

			// Now create the map
			pItem = CItem::CreateScript( ITEMID_MAP );
			pItem->m_itMap.m_top = rect.m_top;
			pItem->m_itMap.m_left = rect.m_left;
			pItem->m_itMap.m_bottom = rect.m_bottom;
			pItem->m_itMap.m_right = rect.m_right;
			ItemBounce( pItem );
		}
		break;

	case SKILL_BOWCRAFT:
	case SKILL_BLACKSMITHING:
	case SKILL_CARPENTRY:
	case SKILL_TINKERING:
makeit:
		// Confer the new item.
		// m_atCreate.m_ItemID = new item
		// m_atCreate.m_Amount = amount of said item.
		// m_atCreate.m_Color = color of the new item
		{
			CItemVendable * pItem = dynamic_cast <CItemVendable *> (CItem::CreateTemplate( m_atCreate.m_ItemID ));
			if ( pItem == NULL )
				return( false );
			if ( m_atCreate.m_Amount != 1 )
			{
				pItem->SetAmount( m_atCreate.m_Amount ); // Set the quantity if we are making bolts, arrows or shafts
			}
			else
			{
				// Only set the quality on single items.
				int quality = m_Skill[skill] * 2 / 10;	// default value for quality.
				// Quality depends on the skill of the craftsman, and a random chance.
				// minimum quality is 1, maximum quality is 200.  100 is average.
				// How much variance?  This is the difference in quality levels from
				// what I can normally make.
				int variance = 2 - (int) log10( ( rand() % 250 ) + 1); // this should result in a value between 0 and 2.
				// Determine if lower or higher quality
				if ( rand() % 2 )
				{
					// Better than I can normally make
				}
				else
				{
					// Worse than I can normally make
					variance = -(variance);
				}
				// The quality levels are as follows:
				// 1 - 25 Shoddy
				// 26 - 50 Poor
				// 51 - 75 Below Average
				// 76 - 125 Average
				// 125 - 150 Above Average
				// 151 - 175 Excellent
				// 175 - 200 Superior
				// Determine which range I'm in
				int qualityBase;
				if ( quality < 25 )
					qualityBase = 0;
				else if ( quality < 50 )
					qualityBase = 1;
				else if ( quality < 75 )
					qualityBase = 2;
				else if ( quality < 125 )
					qualityBase = 3;
				else if ( quality < 150 )
					qualityBase = 4;
				else if ( quality < 175 )
					qualityBase = 5;
				else
					qualityBase = 6;
				qualityBase += variance;
				if ( qualityBase < 0 )
					qualityBase = 0;
				if ( qualityBase > 6 )
					qualityBase = 6;
				CGString makeMessage;
				switch ( qualityBase )
				{
				case 0:
					// Shoddy quality
					makeMessage.Format("Due to your poor skill, the item is of shoddy quality");
					quality = ( rand() % 25 ) + 1;
					break;
				case 1:
					// Poor quality
					makeMessage.Format("You were barely able to make this item.  It is of poor quality");
					quality = ( rand() % 25 ) + 26;
					break;
				case 2:
					// Below average quality
					makeMessage.Format("You make the item, but it is of below average quality");
					quality = ( rand() % 25 ) + 51;
					break;
				case 3:
					// Average quality
					quality = ( rand() % 50 ) + 76;
					break;
				case 4:
					// Above average quality
					makeMessage.Format("The item is of above average quality");
					quality = ( rand() % 25 ) + 126;
					break;
				case 5:
					// Excellent quality
					makeMessage.Format("The item is of excellent quality");
					quality = ( rand() % 25 ) + 151;
					break;
				case 6:
					// Superior quality
					makeMessage.Format("Due to your exceptional skill, the item is of superior quality");
					quality = ( rand() % 25 ) + 176;
					break;
				default:
					// How'd we get here?
					quality = 1000;
					break;
				}
				pItem->SetQuality(quality);
				if ( Skill_GetBase(skill) > 999 && ( quality > 175 ))
				{
					// A GM made this, and it is of high quality
					CGString csNewName;
					csNewName.Format( "%s crafted by %s", pItem->GetName(), this->GetName());
					pItem->SetName(csNewName);
				}
				if ( makeMessage != "" )
					SysMessage( makeMessage );
			}
			if ( m_atCreate.m_Color )
			{
				pItem->SetColor( m_atCreate.m_Color );
			}
			pItem->m_Attr |= ATTR_MOVE_ALWAYS | ATTR_CAN_DECAY;	// Any made item is movable.
			ItemBounce(pItem);
		}
		break;

	case SKILL_COOKING:
		// Cooking m_Act_Targ food object. done.
		{
			CItem * pItem = m_Act_Targ.ItemFind();
			if ( pItem == NULL || pItem->m_type != ITEM_FOOD_RAW )
			{
				return false;
			}
			// Convert uncooked food to cooked food.
			ITEMID_TYPE id = pItem->Use_Cook();
			if ( ! id )
				return( false );

			SysMessage( "Mmm, smells good" );
			Use_Consume( pItem );
			ItemBounce( CItem::CreateTemplate( id  ));
		}
		break;

	case SKILL_FISHING:
		{
			static const ITEMID_TYPE Fish[] = { ITEMID_FISH_1, ITEMID_FISH_2, ITEMID_FISH_3, ITEMID_FISH_4 };

			// Create the little splash effect.
			Sound( 0x027 );
			CItem * pItemFX = CItem::CreateBase( ITEMID_FX_SPLASH );
			pItemFX->SetDecayTime( 3*TICK_PER_SEC );
			pItemFX->m_type = ITEM_DRINK;	// can't fish here.
			pItemFX->MoveTo( m_Act_p );

			// last minute resource check
			CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_WATER, false );
			if ( pResBit == NULL )
				return( false );
			if ( pResBit->GetAmount() == 0 )
				return( false );
			Use_Consume( pResBit, 1 );

			CItem * pFish = CItem::CreateScript( Fish[ GetRandVal( COUNTOF( Fish )) ] );
			pFish->MoveToCheck( GetTopPoint());	// put at my feet.

			SysMessage( "You pull out a nice fish!" );
		}
		break;

	case SKILL_VETERINARY:
	case SKILL_HEALING:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			CItem * pBandage = m_Act_TargPrv.ItemFind();
			if ( pChar == NULL ||
				! CanTouch( pChar ) ||
				pBandage == NULL ||
				pBandage->m_type != ITEM_BANDAGE )
			{
				return( false );
			}
			Use_Consume( pBandage );
			CItem * pBloodyBandage = CItem::CreateScript(GetRandVal(2) ? ITEMID_BANDAGES_BLOODY1 : ITEMID_BANDAGES_BLOODY2 );
			ItemBounce(pBloodyBandage);
			pChar->UpdateStats( STAT_STR, GetRandVal( 10 + Skill_GetAdjusted(skill) / 20 ));
		}
		break;

	case SKILL_HERDING:
		{
			//
			// Try to make them walk there.
			//
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
			{
				return false;
			}
			CItem * pCrook = m_Act_TargPrv.ItemFind();
			if ( IsPriv( PRIV_GM ) && pCrook && pCrook->IsAttr( ATTR_MAGIC ))
			{
				pChar->Spell_Teleport( m_Act_p, true, false );
			}
			else
			{
				pChar->m_Act_p = m_Act_p;
				pChar->Skill_Start( NPCACT_GOTO );
			}
			ObjMessage( "The animal goes where it is instructed", pChar );
		}
		break;

	case SKILL_Stealth:
	case SKILL_HIDING:
		if ( IsStat( STATF_Hidden ))
		{
			Reveal( STATF_Hidden );
			return( false );
		}

		ObjMessage( "You have hidden yourself well", this );
		SetStat( STATF_Hidden );
		UpdateMode();
		break;

	case SKILL_INSCRIPTION:
		// make the scroll
		{
			// Consume the reagents.
			if ( ! Spell_CanCast( m_atMagery.m_Spell, false, this, true ))
			{
				return false ;
			}
			CItem * pItem = m_Act_Targ.ItemFind();
			if ( pItem == NULL )
			{
				SysMessage( "The scroll is gone" );
				return false;
			}
			Use_Consume( pItem );

			pItem = CItem::CreateBase( g_Serv.m_SpellDefs[ m_atMagery.m_Spell ]->m_ScrollID );
			pItem->m_Attr |= ATTR_MAGIC;
			pItem->m_type = ITEM_SCROLL;	// should already be set.
			pItem->m_itWeapon.m_spell = m_atMagery.m_Spell;
			// ??? Do scrolls have the skill level of the caster or the inscriber ?
			pItem->m_itWeapon.m_skilllevel = Skill_GetAdjusted(SKILL_MAGERY)/10;
			pItem->m_itWeapon.m_charges = 1;

			ItemBounce( pItem );
		}
		break;

	case SKILL_LOCKPICKING:
		{
			// m_Act_Targ = the lock.
			// m_Act_TargPrv = the pick
			CItem * pLock = m_Act_Targ.ItemFind();
			CItem * pPick = m_Act_TargPrv.ItemFind();
			if ( pLock == NULL || pPick == NULL ) 
				return( false );
			if ( pPick->GetTopLevelObj() != this )	// the pick is gone !
				return( false );
			if ( ! CanTouch( pLock ))	// we moved too far from the lock.
				return( false );
			if ( ! pLock->Use_LockPick( this ))
			{
				return( false );	
			}
		}
		break;

	case SKILL_LUMBERJACKING:
		{
			// resource check
			CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_TREE, false );
			int iAmount = Use_Consume( pResBit, 4 + GetRandVal( Skill_GetBase(SKILL_LUMBERJACKING)/100+1 ));
			if ( ! iAmount )
				return( false );

			CItem * pItem = CItem::CreateScript( ITEMID_LOG_1 );
			pItem->SetName("log"); // fix the name!
			pItem->SetAmount( iAmount );
			ItemBounce( pItem );
		}
		break;

	case SKILL_MAGERY:
		// ex. magery skill goes up FAR less if we use a scroll or magic device !
		if ( ! Spell_Done())
		{
			return false;
		}
		break;

	case SKILL_MINING:
		// Test the chance of precious ore.
		if ( ! Skill_Mining())
			return( false );
		break;

	case SKILL_POISONING:	// 30
		// Act_TargPrv = poison this
		// Act_Targ = with this poison.
		{
			Sound( 0x247 );	// powdering.
			CItem * pItem = m_Act_TargPrv.ItemFind();
			CItem * pPoison = m_Act_Targ.ItemFind();
			if ( pItem == NULL || pPoison == NULL ||
				pPoison->GetID() != ITEMID_POTION_GREEN )
			{
				return false;
			}
			switch ( pItem->m_type )
			{
			case ITEM_FOOD:
			case ITEM_FOOD_RAW:
				pItem->m_itFood.m_poison_skill = pPoison->m_itPotion.m_skillquality;
				break;
			case ITEM_WEAPON_MACE_SHARP:
			case ITEM_WEAPON_SWORD:		// 13 =
			case ITEM_WEAPON_FENCE:		// 14 = can't be used to chop trees. (make kindling)
				pItem->m_itWeapon.m_poison_skill = pPoison->m_itPotion.m_skillquality;
				break;
			default:
				SysMessage( "You can only poison food or piercing weapons." );
				return false;
			}
			// skill + quality of the poison.
			SysMessage( "You apply the poison." );
			Use_Consume( pPoison );
		}
		break;

	case SKILL_PROVOCATION:
		// m_Act_TargPrv = provoke this person
		// m_Act_Targ = against this person.
		{
			CChar * pCharProv = m_Act_TargPrv.CharFind();
			CChar * pCharTarg = m_Act_Targ.CharFind();

			// If no provoker, then we fail (naturally!)
			if ( pCharProv == NULL || pCharProv == this )
			{
				SysMessage( "You are really upset about this" );
				return false;
			}

			// If out of range or something, then we might get attacked ourselves.
			if ( pCharProv->Stat_Get(STAT_Karma) >= 10000 )
			{
				// They are just too good for this.
				pCharProv->Emote( "looks peaceful" );
				return false;
			}
			pCharProv->Emote( "looks furious" );

			// If no target then skill fails
			if ( pCharTarg == NULL )
			{
				return false;
			}

			// He realizes that you are the real bad guy.
			if ( ! pCharTarg->OnAttackedBy( this, true ))
				return( false );

			if ( pCharProv->m_pPlayer )	// Players have some recourse against provoker.
			{
				pCharProv->Memory_AddObjTypes( this, MEMORY_AGGREIVED );	// MEMORY_IRRITATEDBY
			}

			// If out of range we might get attacked ourself.
			if ( pCharProv->GetTopDist3D( pCharTarg ) > UO_MAP_VIEW_SIGHT ||
				pCharProv->GetTopDist3D( this ) > UO_MAP_VIEW_SIGHT )
			{
				// Check that only "evil" monsters attack provoker back
				if ( pCharProv->IsEvil())
				{
					pCharProv->Attack( this );
				}
				return false;
			}

			// If we are provoking against a "good" PC/NPC and the provoked
			// NPC/PC is good, we are flagged criminal for it and guards
			// are called.
			if ( pCharProv->GetNotoFlag(this)==NOTO_GOOD )
			{
				// lose some karma for this.
				CheckCrimeSeen( SKILL_NONE, NULL, pCharProv, "provoking" );
				return false;
			}

			// If we provoke upon a good char we should go criminal for it
			// but skill still succeed.
			if ( pCharTarg->GetNotoFlag(this)==NOTO_GOOD )
			{
				CheckCrimeSeen( SKILL_NONE, NULL, pCharTarg, "provoking" );
			}

			pCharProv->Attack( pCharTarg ); // Make the actual provoking.
		}
		break;

	case SKILL_SNOOPING:
		if ( Skill_Snoop( false, false ) < 0 )
		{
			return false;
		}
		break;

	case SKILL_SPIRITSPEAK:
		if ( IsStat( STATF_SpiritSpeak ))
			return( false );
		SysMessage( "You establish a connection to the netherworld." );
		Sound( 0x24a );
		Spell_Effect_Create( SPELL_NONE, LAYER_FLAG_SpiritSpeak, 1, 4*60*TICK_PER_SEC, this );
		break;

	case SKILL_STEALING:
		if ( Skill_Steal( false, false ) < 0 )
		{
			return false;
		}
		break;

	case SKILL_TAILORING:
		Sound( 0x248 );	// snip noise
		goto makeit;

	case SKILL_TAMING:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
				return( false );
			pChar->NPC_SetOwner( this );
			pChar->m_food = 50;	// this is good for something.
			pChar->m_Act_Targ = GetUID();
			pChar->Skill_Start( NPCACT_FOLLOW_TARG );
			SysMessage( "It seems to accept you as master" );

			// Create the memory of being tamed to prevent lame macroers
			CItemMemory * pMemory = pChar->Memory_AddObjTypes( this, MEMORY_SPEAK );
			pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_TAMED;
		}
		break;

	case SKILL_MEDITATION:
		// Try to regen your mana even faster than normal.
		if ( m_StatMana >= Stat_Get(STAT_INT))
			return( false );
		UpdateStats( STAT_INT, 1 );
		// next update. (depends on skill)
		SetTimeout( TICK_PER_SEC +
			(((100-Skill_GetAdjusted(SKILL_MEDITATION)) * TICK_PER_SEC ) / 10 )
			);

		// Set a new possibility for failure ?
		// iDifficulty = GetRandVal(100);
		if ( m_atTaming.m_Stroke_Count )	// give experience only first time.
			return( true );
		m_atTaming.m_Stroke_Count++;
		break;

	case SKILL_RemoveTrap:
		// Is it a trap ?
		// disable it.
		{
			CItem * pTrap = m_Act_Targ.ItemFind();
			if ( ! CanTouch(pTrap) || pTrap->m_type != ITEM_TRAP )
			{
				return( false );
			}
			pTrap->SetTrapState( ITEM_TRAP_INACTIVE, ITEMID_NOTHING, 5*60 );
		}
		break;

	case NPCACT_BREATH:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL ) 
				return false;
			CPointMap pntTarg = pChar->GetTopPoint();
			UpdateDir( pntTarg );
			CPointMap pntMe = GetTopPoint();
			if ( pntMe.GetDist( pntTarg ) > UO_MAP_VIEW_SIGHT )
			{
				pntTarg.CalcPath( pntMe, UO_MAP_VIEW_SIGHT );
			}
			Sound( 0x227 );
			int iDamage = m_StatStam/4 + GetRandVal( m_StatStam/4 );
			g_World.Explode( this, pntTarg, 3, iDamage, DAMAGE_FIRE | DAMAGE_GENERAL );
		}
		break;

	case NPCACT_THROWING:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL ) 
				return false;
			CPointMap pntTarg = pChar->GetTopPoint();
			UpdateDir( pntTarg );
			CPointMap pntMe = GetTopPoint();
			if ( pntMe.GetDist( pntTarg ) > UO_MAP_VIEW_SIGHT )
			{
				pntTarg.CalcPath( pntMe, UO_MAP_VIEW_SIGHT );
			}
			SoundChar( CRESND_GETHIT );

			// a rock or a boulder ?
			ITEMID_TYPE id;
			int iDamage;
			if ( ! GetRandVal( 3 ))
			{
				iDamage = m_StatStam/4 + GetRandVal( m_StatStam/4 );
				id = (ITEMID_TYPE)( ITEMID_ROCK_B_LO + GetRandVal(ITEMID_ROCK_B_HI-ITEMID_ROCK_B_LO));
			}
			else
			{
				iDamage = 2 + GetRandVal( m_StatStam/4 );
				id = (ITEMID_TYPE)( ITEMID_ROCK_2_LO + GetRandVal(ITEMID_ROCK_2_HI-ITEMID_ROCK_2_LO));
			}

			CItem * pRock = CItem::CreateScript(id);
			ASSERT(pRock);
			pRock->m_Attr |= ATTR_CAN_DECAY;
			// pRock->MoveNear( pntTarg, GetRandVal(2));
			pRock->MoveToDecay(pntTarg);
			pRock->Effect( EFFECT_BOLT, id, this );

			// did it hit ?
			if ( ! GetRandVal( pChar->GetTopPoint().GetDist( m_Act_p )))
			{
				pChar->OnTakeDamage( iDamage, this, DAMAGE_HIT );
			}
		}
		break;

	default:
		if ( ! IsSkillBase( skill )) 
			return true;	// keep these active.
		break;
	}

	// Success = Advance the skill
	Skill_Experience( skill, m_Act_Difficulty );
cleanup:
	Skill_Cleanup();
	return( true );
}

bool CChar::Skill_Wait()
{
	// What is we are in combat mode ?
	if ( IsStat( STATF_DEAD | STATF_Sleeping | STATF_Freeze | STATF_Stone ))
	{
		SysMessage( "You can't do much in your current state." );
		return( true );
	}
	if ( GetActiveSkill() == SKILL_NONE )
	{
		Reveal();
		return( false );
	}
	SysMessage( IsStat( STATF_War ) ?
		"You are preoccupied with thoughts of battle." :
		"You must wait to perform another action" );
	return ( true );
}

bool CChar::Skill_Start( SKILL_TYPE sk, int iDifficulty )
{
	// We have all the info we need to do the skill. (targeting etc)
	// Set up how long we have to wait before we get the desired results from this skill.
	// Set up any animations/sounds in the mean time.
	// Calc if we will succeed or fail.
	// ARGS:
	//  iDifficulty = 0-100
	// RETURN: 
	//  false = failed outright with no wait. "You have no chance of taming this"

	if ( g_Serv.IsLoading())
	{
		if ( sk != SKILL_NONE &&
			! IsSkillBase(sk) && 
			! IsSkillNPC(sk))
		{
			DEBUG_ERR(( "UID:0%x Bad Skill %d for '%s'\n", GetUID(), sk, GetName()));
			return( false );
		}
		m_Act_SkillCurrent = sk;
		return( true );
	}

	WORD wWaitTime = 0;	// in seconds
	if ( GetActiveSkill() != SKILL_NONE )
	{
		Skill_Fail( true );	// Fail previous skill unfinished.
	}

	ASSERT( sk == SKILL_NONE || IsSkillBase(sk) || IsSkillNPC(sk));

	// Some skill can start right away. Need no targetting.
	switch ( sk )
	{
	case SKILL_NONE:
		// Set to no skill used.
		iDifficulty = 0;
		break;

	case SKILL_ANIMALLORE:
	case SKILL_ARMSLORE:
	case SKILL_ANATOMY:
	case SKILL_ITEMID:
	case SKILL_EVALINT:
	case SKILL_FORENSICS:
	case SKILL_TASTEID:
		wWaitTime = g_Serv.m_SkillDefs[sk]->m_wDelay;
		iDifficulty = GetRandVal(50);
		break;

	case SKILL_ALCHEMY: // See if skill allows a potion made out of targ'd reagent
		// m_Act_Targ = the reagent.
		// m_Act_TargPrv = bottle (maybe).
		// m_atAlchemy.m_Potion = potion we are making.
		{
			// Sound( 0x243 );
			CItem * pReag = m_Act_Targ.ItemFind();
			if ( pReag == NULL || pReag->m_type != ITEM_REAGENT)
			{
				SysMessage( "That is not a magical reagent." );
				return( false );
			}

			m_atAlchemy.m_Potion = POTION_WATER;
			bool fFound = false;
			bool fNotEnuf = false;
			for ( int i=0; i < g_Serv.m_PotionDefs.GetCount(); i++ ) // Find 1st potion made from this reagent
			{
				if ( pReag->GetID() != g_Serv.m_PotionDefs[i]->m_RegID )
					continue;
				fFound = true;
				if ( Skill_GetAdjusted(SKILL_ALCHEMY) < g_Serv.m_PotionDefs[i]->m_SkillReq )
					continue;
				if ( pReag->GetAmount() >= g_Serv.m_PotionDefs[i]->m_RegReq )
				{
					iDifficulty = g_Serv.m_PotionDefs[i]->m_SkillReq / 10;
					m_atAlchemy.m_Potion = (POTION_TYPE) i;
					fNotEnuf = false;
				}
				else if (m_atAlchemy.m_Potion == POTION_WATER)
				{
					fNotEnuf = true;
				}
			}
			// Tell us if we can't make it
			if ( ! fFound )
			{
				// Nonstandard reagent message
				SysMessage( "You don't know how to make anything from that....(yet!)");
				return false;
			}
			if (m_atAlchemy.m_Potion == 0)
			{
				if (fNotEnuf)
					SysMessage( "You don't have enough to make anything with that.");
				else
					SysMessage( "You are not good enough to make anything out of that.");
				return false;
			}
			m_atAlchemy.m_Stroke_Count = 0; // counts up.
			wWaitTime = 1;
		}
		break;

	case SKILL_BEGGING:
		// m_Act_Targ = Our begging target..
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
				return( false );

			SysMessagef("You grovel at %s's feet", pChar->GetName());
			iDifficulty = pChar->Stat_Get(STAT_INT);
			wWaitTime = g_Serv.m_SkillDefs[sk]->m_wDelay;
		}
		break;

	case SKILL_BLACKSMITHING:
		// m_Act_p = the anvil.
		// m_atCreate.m_ItemID = the item to make.
		// m_Act_Targ = the hammer.

		m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), ITEM_FORGE, 3 );
		if ( ! m_Act_p.IsValid())
		{
			SysMessage( "You must be near a forge to smith" );
			return( false );
		}
		wWaitTime = 1;
		m_atCreate.m_Stroke_Count = GetRandVal( 4 ) + 2;
		break;

	case SKILL_BOWCRAFT:
		// Might be based on how many arrows to make ???
		UpdateAnimate( ANIM_SALUTE );
		Sound( 0x055 );
		wWaitTime = 1*TICK_PER_SEC;
		m_atCreate.m_Stroke_Count = GetRandVal( 2 ) + 1;
		break;

	case SKILL_CARPENTRY:
		// m_atCreate.m_ItemID = create this item
		Sound( 0x23d );
		wWaitTime = 1*TICK_PER_SEC;
		m_atCreate.m_Stroke_Count = GetRandVal( 3 ) + 2;
		break;

	case SKILL_CARTOGRAPHY:
		// Selected a map type and now we are making it.
		// m_Act_Cartography_Dist = distance to map
		Sound( 0x249 );
		wWaitTime = g_Serv.m_SkillDefs[sk]->m_wDelay;
		break;

	case SKILL_COOKING:
		// m_Act_Targ = food object to cook.
		wWaitTime = 2*TICK_PER_SEC;
		iDifficulty = GetRandVal( 50 );
		break;

	case SKILL_DETECTINGHIDDEN:
		// Based on who is hiding ?
		iDifficulty = 10;
		wWaitTime = 1*TICK_PER_SEC;
		break;

	case SKILL_ENTICEMENT:
		// Just keep playing and trying to allure them til we can't
		// Must have a musical instrument.
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
				return( false );
			iDifficulty = Use_PlayMusic( NULL, GetRandVal(55));
			if ( iDifficulty < -1 )	// no instrument fail immediate
				return( false );
			if ( ! iDifficulty )
			{
				iDifficulty = pChar->Stat_Get(STAT_INT);
			}
			wWaitTime = 2*TICK_PER_SEC;
		}
		break;
	case SKILL_FISHING:
		{
			// m_Act_p = where to fish.
			// NOTE: don't check LOS else you can't fish off boats.
			// Check that we dont stand too far away
			// Make sure we aren't in a house
			CRegionBase * pRegion = GetTopPoint().GetRegion( REGION_TYPE_MULTI );
			if ( pRegion && ! pRegion->IsFlag( REGION_FLAG_SHIP ) )
			{
				SysMessage("You can't fish from where you are standing.");
				return( false );
			}

			if ( GetTopPoint().GetDist( m_Act_p ) > 6 )	// cast works for long distances.
			{
				SysMessage("That is too far away." );
				return( false );
			}
			// resource check
			CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_WATER );
			if ( pResBit == NULL )
			{
				SysMessage( "Try fishing in water." );
				return( false );
			}
			if ( pResBit->GetAmount() == 0 )
			{
				SysMessage( "There are no fish here." );
				return( false );
			}

			UpdateAnimate( ANIM_ATTACK_2H_DOWN );
			Sound( 0x027 );
			CItem * pItem = CItem::CreateBase( ITEMID_FX_SPLASH );
			pItem->SetDecayTime( 1*TICK_PER_SEC );
			pItem->m_type = ITEM_DRINK;	// can't fish here.
			pItem->MoveTo( m_Act_p );
			wWaitTime = 2*TICK_PER_SEC;
			iDifficulty = GetRandVal(100);
		}
		break;

	case SKILL_Stealth:
	case SKILL_HIDING:
		// Skill required varies with terrain and situation ?
		// if we are carrying a light source then this should not work.
		{
			CItem * pItem = GetContentHead();
			for ( ; pItem; pItem = pItem->GetNext())
			{
				if ( ! CItemBase::IsVisibleLayer( pItem->GetEquipLayer()))
					continue;
				if ( pItem->m_pDef->Can( CAN_I_LIGHT ))
				{
					SysMessage( "You are too well lit to hide" );
					return( false );
				}
			}
		}
		iDifficulty = GetRandVal(70);
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_HERDING:
		// m_Act_Targ = move this creature.
		// m_Act_p = move to here.
		{
			// How do I make them move fast ? or with proper speed ???
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
				return( false );
			UpdateAnimate( ANIM_ATTACK_WEAPON );
			iDifficulty = pChar->Stat_Get(STAT_INT);
			wWaitTime = 1*TICK_PER_SEC;
		}
		break;

	case SKILL_INSCRIPTION:
		// Can we even attempt to make this scroll ?
		if ( ! Spell_CanCast( m_atMagery.m_Spell, true, this, true ))
			return( false );
		Sound( 0x249 );
		iDifficulty = Spell_GetBaseDifficulty( m_atMagery.m_Spell );
		wWaitTime = g_Serv.m_SkillDefs[sk]->m_wDelay;
		break;

	case SKILL_LOCKPICKING:
		// m_Act_Targ = the item to be picked.
		// m_Act_TargPrv = The pick.
		{
			CItem * pLock = m_Act_Targ.ItemFind();
			CItem * pPick = m_Act_TargPrv.ItemFind();

			if ( pLock == NULL ||
				( pLock->m_type != ITEM_CONTAINER_LOCKED &&
				pLock->m_type != ITEM_DOOR_LOCKED ))
			{
				SysMessage( "Use the lock pick on a locked item" );
				return false;
			}
			if ( ! CanTouch( pLock ))
			{
				SysMessage( "You can't reach that." );
				return false;
			}
			iDifficulty = pLock->m_itContainer.m_lock_complexity/10;
			wWaitTime = 3*TICK_PER_SEC;
		}
		return false;

	case SKILL_LUMBERJACKING:
		{
			if ( m_Act_p.m_x == 0xFFFF )
			{
				SysMessage( "Try chopping a tree!" );
				return( false );
			}
			// 3D distance check and LOS
			if ( ! CanTouch(m_Act_p) || GetTopPoint().GetDist3D( m_Act_p ) > 3 )
			{
				SysMessage( "That's too far away to chop." );
				return( false );
			}
			// resource check
			CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_TREE );
			if ( pResBit == NULL )
			{
				SysMessage( "Try chopping a tree." );
				return( false );
			}
			if ( pResBit->GetAmount() == 0 )
			{
				SysMessage( "There are no logs left here to chop." );
				return( false );
			}
			wWaitTime = 1;
			iDifficulty = GetRandVal(85);
			m_atCreate.m_Stroke_Count = GetRandVal( 5 ) + 2;
		}
		break;

	case SKILL_MAGERY:
		// Casting time goes up with difficulty
		// but down with skill, int and dex
		// m_atMagery.m_Spell = spell to cast.
		// m_Act_p = location to cast to.
		// m_Act_TargPrv = source of spell.
		// m_Act_Targ = dest of spell.

		iDifficulty = Spell_StartCast();
		if ( iDifficulty < 0 )
			return( false );

		if ( IsPriv( PRIV_GM ))
			wWaitTime = 1;
		else
			wWaitTime = ((iDifficulty / 25 ) + 1 )*TICK_PER_SEC;
		break;

	case SKILL_MINING:
		{
			if ( m_Act_p.m_x == 0xFFFF )
			{
				SysMessage( "Try mining in rock!" );
				return( false );
			}
			if ( GetTopPoint().GetDist( m_Act_p ) > 2 )
			{
				SysMessage("That is too far away." );
				return( false );
			}
			// Verify so we have a line of sight.
			if ( ! CanSeeLOS( m_Act_p, NULL, 2 ))
			{
				SysMessage("You have no line of sight to that location");
				return( false );
			}

			// resource check
			CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, ITEM_ROCK );
			if ( pResBit == NULL )
			{
				SysMessage( "Try mining in rock." );
				return( false );
			}
			if ( pResBit->GetAmount() == 0 )
			{
				SysMessage( "There is no ore here to mine." );
				return( false );
			}
			wWaitTime = 1;
			iDifficulty = GetRandVal(100);
			m_atCreate.m_Stroke_Count = GetRandVal( 5 ) + 2;
		}
		break;

	case SKILL_MUSICIANSHIP:
		iDifficulty = Use_PlayMusic( m_Act_Targ.ItemFind(), GetRandVal(90));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( false );
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_PEACEMAKING:
		// Find musical inst first.

		// Basic skill check.
		iDifficulty = Use_PlayMusic( NULL, GetRandVal(40));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( false );

		if ( ! iDifficulty )
		{
			iDifficulty = GetRandVal(40);	// Depend on evil of the creatures here.
		}

		// Who is fighting around us ? determines difficulty.
		wWaitTime = 1*TICK_PER_SEC;
		break;

	case SKILL_POISONING:
		// m_Act_Targ = poison.
		// m_Act_TargPrv = weapon/food
		iDifficulty = GetRandVal( 60 );
		wWaitTime = 1*TICK_PER_SEC;
		break;
	case SKILL_PROVOCATION:
		// m_Act_TargPrv = provoke this person
		// m_Act_Targ = against this person.
		{
		CChar * pCharProv = m_Act_TargPrv.CharFind();
		CChar * pCharTarg = m_Act_Targ.CharFind();
		if ( pCharProv == NULL || pCharTarg == NULL )
			return( false );

		iDifficulty = Use_PlayMusic( NULL, GetRandVal(40));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( false );
		if ( ! iDifficulty )
		{
			iDifficulty = pCharProv->Stat_Get(STAT_INT);	// Depend on evil of the creature.
		}

		wWaitTime = 3*TICK_PER_SEC;
		}
		break;

	case SKILL_SNOOPING:
		// m_Act_Targ = the item to snoop.
		iDifficulty = Skill_Snoop( true, false );
		if ( iDifficulty == -1 )
			return( false );
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_SPIRITSPEAK:
		// difficulty based on spirits near ?
		iDifficulty = 10;
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_STEALING:
		// m_Act_Targ = the item to steal.
		iDifficulty = Skill_Steal( true, false );
		if ( iDifficulty == -1 )
			return( false );
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_TAILORING:
	case SKILL_TINKERING:
		// m_atCreate.m_ItemID = item to create
		wWaitTime = 2*TICK_PER_SEC;
		break;

	case SKILL_TAMING:
		// m_Act_Targ = creature to tame.
		// Check the min required skill for this creature.
		// Related to INT ?
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
				return( false );
			if ( pChar == this )
			{
				SysMessage( "You are your own master." );
				return( false );
			}
			if ( pChar->m_pNPC == NULL )
			{
				SysMessage( "You can't tame a player." );
				return( false );
			}
			if ( ! IsPriv( PRIV_GM )) // if its a gm doing it, just check that its not
			{
				// Is it tamable ?
				if ( pChar->IsStat( STATF_Pet ))
				{
					SysMessagef( "%s is already tame.", pChar->GetName());
					return( false );
				}
				// Too smart or not an animal.
				if ( pChar->m_pNPC->m_Brain != NPCBRAIN_ANIMAL ||
					pChar->GetCreatureType() != NPCBRAIN_ANIMAL )
				{
					SysMessagef( "%s cannot be tamed.", pChar->GetName());
					return( false );
				}
			}
			iDifficulty = max( pChar->Stat_Get(STAT_INT), pChar->Stat_Get(STAT_STR));
			iDifficulty = max( iDifficulty, pChar->Skill_GetBase(SKILL_TAMING)/10);

			if ( pChar->Memory_FindObjTypes( this, MEMORY_FIGHT|MEMORY_AGGREIVED|MEMORY_IRRITATEDBY ))
			{
				// I've attacked it b4 ?
				iDifficulty += 50;
			}

			wWaitTime = 1*TICK_PER_SEC;
			m_atTaming.m_Stroke_Count = GetRandVal( 4 ) + 2;
		}
		break;

	case SKILL_TRACKING:
		// Already checked difficulty earlier.
		iDifficulty = 0;
		wWaitTime = 1*TICK_PER_SEC;
		break;

	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:
		// When do we get our next shot?
		{
			m_atFight.m_War_Swing_State = WAR_SWING_EQUIPPING;
			iDifficulty = GetTargetHitDifficulty(sk);
			wWaitTime = SetWeaponSwingTimer();
		}
		break;

	case SKILL_VETERINARY:
	case SKILL_HEALING:
		// should depend on the severity of the wounds ?
		// m_Act_TargPrv = bandages.
		// m_Act_Targ = heal target.
		iDifficulty = GetRandVal(80);
		{
			CItem * pBandage = m_Act_TargPrv.ItemFind();
			if ( pBandage == NULL ||
				pBandage->m_type != ITEM_BANDAGE )
				return( false );
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL )
			{
				SysMessage("Try healing a living creature.");
				return(false);
			}
			if ( ! CanTouch(pChar))
			{
				SysMessage("You must be able to reach the target");
				return(false);
			}
			if ( pChar->m_StatHealth >= pChar->Stat_Get(STAT_STR) )
			{
				SysMessage("Your target is already fully healed");	
				return( false );
			}
			wWaitTime = 2*TICK_PER_SEC;
		}
		break;

	case SKILL_RemoveTrap:
		// Is it a trap ?
		// How difficult ?
		{
			CItem * pTrap = m_Act_Targ.ItemFind();
			if ( pTrap == NULL || pTrap->m_type != ITEM_TRAP )
			{
				SysMessage( "You should use this skill to disable traps" );
				return( false );
			}
			iDifficulty = GetRandVal(95);
			wWaitTime = 2*TICK_PER_SEC;
		}
		break;

	case SKILL_MEDITATION:
		// Might depend on creatures in the area ?
		if ( m_StatMana >= Stat_Get(STAT_INT))
		{
			SysMessage( "You are at peace." );
			return( false );
		}

		m_atTaming.m_Stroke_Count = 0;
		wWaitTime = 2*TICK_PER_SEC;
		iDifficulty = GetRandVal(100);
		break;

	case NPCACT_BREATH:
		// A Dragon I assume.
		UpdateStats( STAT_DEX, -10 );
		UpdateAnimate( ANIM_MON_MISC_BREATH, false );
		wWaitTime = 3*TICK_PER_SEC;
		iDifficulty = 0;
		break;

	case NPCACT_THROWING:
		{
		CChar * pChar = m_Act_Targ.CharFind();
		if ( pChar == NULL )
			return false;
		m_Act_p = pChar->GetTopPoint();
		UpdateStats( STAT_DEX, -( 4 + GetRandVal(6)));
		UpdateAnimate( ANIM_MON_MISC_BREATH );
		wWaitTime = 1*TICK_PER_SEC;
		iDifficulty = 0;
		}
		break;

	default:
		// Might just be an NPC mode.
		if ( IsSkillBase( sk ))
		{
			SysMessage( "Skill not implemented" );
			return( false );
		}
		iDifficulty = 0;
	}

	if ( iDifficulty > 0 )
	{
		if ( ! Skill_CheckSuccess( sk, iDifficulty ))
			iDifficulty = - iDifficulty; // will result in Failure.
	}

	// instant fail ?
	if ( ! wWaitTime && iDifficulty < 0 )
		return( false );

	// Starting success anyhow.
	switch ( sk)
	{
	case SKILL_MEDITATION:
		{
			SysMessage( "You enter a meditative trance." );
			Sound( 0xf9 );
		}
		break;
	}

	m_Act_SkillCurrent = sk;	// Start using a skill.
	m_Act_Difficulty = iDifficulty;

	// How long before complete skill.
	SetTimeout( wWaitTime );

	// emote the action i am taking.
	if ( g_Serv.m_wDebugFlags & DEBUGF_NPC_EMOTE )
	{
		Emote( Skill_GetName(true));
	}
	else if ( IsStat(STATF_EmoteAction))	// ??? Just to owners ?
	{
		Emote( Skill_GetName(true));
	}

	return( true );
}

//----------------------------------------------------------------------
// Spells

int CChar::Spell_GetBaseDifficulty( SPELL_TYPE spell ) // static
{
	// RETURN 0-100.
	if ( spell > SPELL_Water_Elem )
	{
		// Necro spells.
		return( 20 );
	}
	return( (((spell-SPELL_Clumsy)/8) + 1 ) * 10 );	// inscript
}

void CChar::Spell_Dispel( int iLevel )
{
	// ARGS: iLevel = 0-100 level of dispel caster.
	// remove all the spells. NOT if caused by objects worn !!!
	CItem* pItemNext;
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( iLevel <= 100 && pItem->IsAttr(ATTR_MOVE_NEVER))	// we don't lose this.
			continue;
		if ( pItem->GetEquipLayer() >= LAYER_SPELL_STATS &&
			pItem->GetEquipLayer() <= LAYER_SPELL_Summon )
		{
			pItem->Delete();
		}
	}
	Update();
}

bool CChar::Spell_Teleport( CPointBase ptNew,
	bool fTakePets, bool fCheckAntiMagic,
	ITEMID_TYPE iEffect, SOUND_TYPE iSound )
{
	// Teleport you to this place.
	// This is sometimes not really a spell at all.
	// ex. ships plank.
	// RETURN: true = it worked.

	if ( ! ptNew.IsValid()) 
		return false;

	Reveal();

	if ( IsPriv(PRIV_JAILED))
	{
		// Must be /PARDONed
static const TCHAR * szPunishMsg[] =
{
	"You feel the world is punishing you.",
	"You feel you should be ashamed of your actions.",
};
		SysMessage( szPunishMsg[ GetRandVal( COUNTOF( szPunishMsg )) ] );
		ptNew = g_World.GetRegionPoint( "jail" );
	}

	// Is it a valid teleport location that allows this ?

	if ( IsPriv( PRIV_GM ))
	{
		fCheckAntiMagic = false;
		if ( iEffect && ! IsStat( STATF_Incognito ) && ! IsPriv( PRIV_PRIV_NOSHOW ))
		{
			iEffect = g_Serv.m_iSpell_Teleport_Effect_Staff;	// drama
			iSound = g_Serv.m_iSpell_Teleport_Sound_Staff;
		}
	}
	else if ( fCheckAntiMagic )
	{
		CRegionBase * pArea = CheckValidMove( ptNew );
		if ( pArea == NULL )
		{
			SysMessage( "You can't teleport to that spot." );
			return false;
		}
		if ( pArea->IsFlag( REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_TELEPORT ))
		{
			SysMessage( "An anti-magic field blocks you" );
			return false;
		}
	}

	if ( IsClient())
	{
		if ( IsStat( STATF_Insubstantial )) iEffect = ITEMID_NOTHING;
		GetClient()->addPause();
	}

	if ( GetTopPoint().IsValid())	// Guards might have justbeen created.
	{
	if ( fTakePets )
	{
		// Look for any creatures that might be following me near by.
		CWorldSearch Area( GetTopPoint(), UO_MAP_VIEW_SIGHT );
		while (true)
		{
			CChar * pChar = Area.GetChar();
			if ( pChar == NULL )
				break;
			if ( pChar == this )
				continue;

			// Hostiles ???

			// My pets ?
			if ( pChar->GetActiveSkill() == NPCACT_FOLLOW_TARG &&
				pChar->m_Act_Targ == GetUID())
			{
				CPointMap masterp = GetTopPoint();	// it will modify the val.
				if ( ! pChar->CanMoveWalkTo( masterp, false ))
					continue;
				pChar->Spell_Teleport( ptNew, fTakePets, fCheckAntiMagic, iEffect, iSound );
			}
		}
	}

	if ( iEffect )	// departing side effect.
	{
		CItem * pItem = CItem::CreateBase( iEffect );
		pItem->SetDecayTime( 2 * TICK_PER_SEC );
		pItem->MoveTo( GetTopPoint());
	}
	}

	CPointMap pold = GetTopPoint();
	MoveTo( ptNew );
	UpdateMove( pold );

	if ( iEffect )
	{
		Sound( iSound );	// 0x01fe
		Effect( EFFECT_OBJ, iEffect, this, 9, 20 );
	}
	return( true );
}

CChar * CChar::Spell_Summon( CREID_TYPE id, CPointMap pntTarg )
{
	if ( id == CREID_INVALID ) 
		return( NULL );
	DEBUG_CHECK( pntTarg.IsValid());
	CChar * pChar;
	if ( id < 0 )
	{
		// GM creates a char this way. "ADDNPC"
		// More specific npc type from script ?
		pChar = CreateNPC( (CREID_TYPE) - (int) id );
		if ( pChar == NULL )
			return( NULL );
		m_Act_Targ = pChar->GetUID();	// for last target stuff.
	}
	else
	{
		// These type summons make me it's master. (for a limited time)
		pChar = CChar::CreateBasic( id );
		if ( pChar == NULL )
			return( NULL );
		// Time based on magery. Add the flag first so it does not get loot.
		// conjured creates have no loot. (Mark this early)
		CItem * pSpell = pChar->Spell_Effect_Create( SPELL_Summon,
			LAYER_SPELL_Summon, 10,
			( 3*60 + Skill_GetAdjusted( SKILL_MAGERY ) / 10 )*TICK_PER_SEC,
			this );
		pChar->NPC_LoadScript(false);
		if ( id != CREID_VORTEX && id != CREID_BLADES )	// any BESERK ??
		{
			pChar->NPC_SetOwner( this );
		}
	}

	pChar->MoveTo( pntTarg );
	pChar->Update();
	pChar->UpdateAnimate( ANIM_CAST_DIR );
	pChar->SoundChar( CRESND_GETHIT );
	return( pChar );
}

bool CChar::Spell_Recall( CItem * pRune, bool fGate )
{
	if ( pRune == NULL ||
		( pRune->m_type != ITEM_RUNE && pRune->m_type != ITEM_TELEPAD ))
	{
		SysMessage( "That item is not a recall rune." );
		return( false );
	}
	if ( ! pRune->m_itRune.m_mark_p.IsValid())
	{
		SysMessage( "The recall rune is blank." );
		return( false );
	}
	if ( pRune->m_type == ITEM_RUNE && pRune->m_itRune.m_Strength <= 0 )
	{
		SysMessage( "The recall rune's magic has faded" );
		return( false );
	}

	if ( fGate )
	{
		// Can't even open the gate ?
		CRegionBase * pArea = pRune->m_itRune.m_mark_p.GetRegion( REGION_TYPE_AREA | REGION_TYPE_MULTI | REGION_TYPE_ROOM );
		if ( pArea == NULL || pArea->IsFlag( REGION_ANTIMAGIC_ALL | REGION_ANTIMAGIC_GATE | REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_RECALL_OUT | REGION_ANTIMAGIC_RECALL_IN ))
		{
			// Antimagic
			SysMessage( "Antimagic blocks the gate" );
			return( false );
		}

		// Color gates to unguarded regions.
		CItem * pGate = CItem::CreateBase( ITEMID_MOONGATE_BLUE );
		pGate->m_type = ITEM_TELEPAD;
		pGate->m_Attr |= ATTR_MOVE_NEVER|ATTR_CAN_DECAY;	// why is this movable ?
		pGate->m_itTelepad.m_mark_p = pRune->m_itRune.m_mark_p;
		// pGate->m_light_pattern = LIGHT_LARGE;
		pGate->SetColor(( pArea != NULL && pArea->IsGuarded()) ? COLOR_DEFAULT : COLOR_RED );
		pGate->SetDecayTime( 30*TICK_PER_SEC );
		pGate->MoveTo( GetTopPoint());
		pGate->Effect( EFFECT_OBJ, ITEMID_MOONGATE_FX_BLUE, pGate, 1, 80, false );

		// Far end gate.
		pGate = CItem::CreateDupeItem( pGate );
		pGate->m_itTelepad.m_mark_p = GetTopPoint();
		pGate->SetColor(( m_pArea && m_pArea->IsGuarded()) ? COLOR_DEFAULT : COLOR_RED );
		pGate->SetDecayTime( 30*TICK_PER_SEC );
		pGate->MoveTo( pRune->m_itRune.m_mark_p );
		pGate->Sound( g_Serv.m_SpellDefs[ SPELL_Gate_Travel ]->m_sound );
		pGate->Effect( EFFECT_OBJ, ITEMID_MOONGATE_FX_BLUE, pGate, 7, 80, false );
	}
	else
	{
		if ( ! Spell_Teleport( pRune->m_itRune.m_mark_p, false, true, ITEMID_NOTHING ))
			return( false );
	}

	// Wear out the rune.
	if ( pRune->m_type == ITEM_RUNE )
	{
		pRune->m_itRune.m_Strength --;
		if ( pRune->m_itRune.m_Strength < 10 )
		{
			SysMessage( "The recall rune is starting to fade." );
		}
		if ( ! pRune->m_itRune.m_Strength )
		{
			SysMessage( "The recall rune fades completely." );
		}
	}

	return( true );
}

bool CChar::Spell_Resurrection( int iSkillLossPercent )
{
	// ARGS: iSkillLossPercent < 0 for a shrine or GM.

	if ( ! IsStat( STATF_DEAD )) 
		return false;

	if ( iSkillLossPercent >= 0 &&
		! IsPriv(PRIV_GM) &&
		m_pArea &&
		m_pArea->IsFlag( REGION_ANTIMAGIC_ALL | REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_TELEPORT ))
	{
		// Could be a house break in.
		// Check to see if it is.
		if ( m_pArea->GetMultiUID() != 0 )
		{
			SysMessage( "Anti-Magic blocks the Resurrection!" );
			return false;
		}
	}

	SetID( m_prev_id );
	ClearStat( STATF_DEAD | STATF_Insubstantial );
	SetColor( m_prev_color );
	m_StatHealth = IMULDIV( Stat_Get(STAT_STR), g_Serv.m_iHitpointPercentOnRez, 100 );

	if ( m_pPlayer )
	{
		CItem * pRobe = ContentFind( ITEMID_DEATHSHROUD );
		if ( pRobe != NULL )
		{
			pRobe->RemoveFromView();	// For some reason this does not update without remove first.
			pRobe->SetDispID( ITEMID_ROBE );
			pRobe->SetName( "Resurrection Robe" );
			pRobe->Update();
		}

		CItemCorpse * pCorpse = FindMyCorpse();
		if ( pCorpse != NULL )
		{
			if ( RaiseCorpse( pCorpse ))
			{
				SysMessage( "You spirit rejoins your body" );
				if ( pRobe != NULL )
				{
					pRobe->Delete();
					pRobe = NULL;
				}
			}
		}
	}

	if ( iSkillLossPercent > 0 )
	{
		// Remove some skills / stats as a percent.
		int i=0;
		for ( ; i<SKILL_QTY; i++ )
		{
			int iVal = Skill_GetBase( (SKILL_TYPE) i );
			if ( iVal <= 250 )
				continue;
			Skill_SetBase( (SKILL_TYPE) i, iVal - IMULDIV( iVal, GetRandVal( iSkillLossPercent ), 100 ));
		}
		for ( i=STAT_STR; i<STAT_BASE_QTY; i++ )
		{
			int iVal = Stat_Get( (STAT_TYPE) i );
			if ( iVal <= 25 )
				continue;
			Stat_Set( (STAT_TYPE) i, iVal - IMULDIV( iVal, GetRandVal( iSkillLossPercent ), 100 ));
		}
	}

	Effect( EFFECT_OBJ, ITEMID_FX_HEAL_EFFECT, this, 9, 14 );
	Update();
	return( true );
}

void CChar::Spell_Effect_Remove( CItem * pSpell )
{
	// we are removing the spell effect.
	if ( ! pSpell->IsAttr( ATTR_MAGIC )) 
		return;
	if ( pSpell->m_type == ITEM_WAND )	// equipped wands do not confer effect.
		return;

	// m_itWeapon, m_itArmor, m_itSpell

	switch ( pSpell->m_itSpell.m_spell )
	{
	case SPELL_Clumsy:
		Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Stone:
		ClearStat( STATF_Stone );
		break;
	case SPELL_Hallucination:
		ClearStat( STATF_Hallucinating );
		if ( IsClient())
		{
			m_pClient->addReSync();
		}
	case SPELL_Feeblemind:
		Stat_Set( STAT_INT, Stat_Get(STAT_INT) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Weaken:
		Stat_Set( STAT_STR, Stat_Get(STAT_STR) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Agility:
		Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Cunning:
		Stat_Set( STAT_INT, Stat_Get(STAT_INT) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Strength:
		Stat_Set( STAT_STR, Stat_Get(STAT_STR) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Bless:
		{
			for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
			{
				Stat_Set( (STAT_TYPE) i, m_Stat[i] - pSpell->m_itSpell.m_skilllevel );
			}
		}
		break;
	case SPELL_Curse:
	case SPELL_Mass_Curse:
		{
			for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
			{
				Stat_Set( (STAT_TYPE) i, m_Stat[i] + pSpell->m_itSpell.m_skilllevel );
			}
		}
		break;

	case SPELL_Reactive_Armor:
		ClearStat( STATF_Reactive );
		return;
	case SPELL_Night_Sight:
		ClearStat( STATF_NightSight );
		if ( IsClient())
		{
			m_pClient->addLight();
		}
		return;
	case SPELL_Magic_Reflect:
		ClearStat( STATF_Reflection );
		return;
	case SPELL_Poison:
	case SPELL_Poison_Field:
		ClearStat( STATF_Poisoned );
		break;
	case SPELL_Protection:
	case SPELL_Arch_Prot:
		m_defense = CalcArmorDefense();
		break;
	case SPELL_Incognito:
		ClearStat( STATF_Incognito );
		SetName( pSpell->GetName());	// restore your name
		if ( ! IsStat( STATF_Polymorph ))
		{
			SetColor( m_prev_color );
		}
		pSpell->SetName( "" );	// lcear the name from the item (might be a worn item)
		break;
	case SPELL_Invis:
		Reveal(STATF_Invisible);
		return;
	case SPELL_Paralyze:
	case SPELL_Paralyze_Field:
		ClearStat( STATF_Freeze );
		return;
	case SPELL_Polymorph:
		{
			DEBUG_CHECK( IsStat( STATF_Polymorph ));
			//  m_prev_id != GetID()
			// poly back to orig form.
			SetID( m_prev_id );
			// set back to original stats as well.
			Stat_Set( STAT_STR, Stat_Get(STAT_STR) - pSpell->m_itSpell.m_PolyStr );
			Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - pSpell->m_itSpell.m_PolyDex );
			m_StatHealth = min( m_StatHealth, Stat_Get(STAT_STR));
			m_StatStam = min( m_StatStam, Stat_Get(STAT_DEX));
			Update();
			ClearStat( STATF_Polymorph );
		}
		return;
	case SPELL_Summon:
		// Delete the creature completely.
		// ?? Drop anything it might have had ?
		// m_StatHealth = 0;
		if ( ! g_Serv.IsLoading())
		{
			CItem * pEffect = CItem::CreateBase( ITEMID_FX_TELE_VANISH );
			pEffect->m_Attr |= ATTR_MOVE_NEVER|ATTR_CAN_DECAY; // why is this movable ?
			pEffect->SetDecayTime( 2*TICK_PER_SEC );
			pEffect->MoveTo( GetTopPoint());
			pEffect->Sound( g_Serv.m_SpellDefs[SPELL_Teleport]->m_sound  );
		}
		if ( m_pPlayer ) 
			return;
		Delete();
		return;
	}
	UpdateStatsFlag();
}

void CChar::Spell_Effect_Add( CItem * pSpell )
{
	// Attach the spell effect for a duration.
	// Add effects which are saved in the save file here.
	// Not in LayerAdd

	if ( pSpell->IsAttr( ATTR_CURSED | ATTR_CURSED2 ))
	{
		if ( ! pSpell->IsAttr( ATTR_MAGIC ))
		{
			pSpell->m_itSpell.m_spell = SPELL_Curse;
			pSpell->m_itSpell.m_skilllevel = 1;
			pSpell->m_Attr |= ATTR_MAGIC;
		}
		if ( ! g_Serv.m_SpellDefs[pSpell->m_itSpell.m_spell]->IsSpellType( SPELLFLAG_HARM ))
		{
			pSpell->m_itSpell.m_spell = SPELL_Curse;
		}
		pSpell->m_Attr |= ATTR_IDENTIFIED;
		SysMessage( "Cursed Magic!" );
	}

	if ( ! pSpell->IsAttr( ATTR_MAGIC ))
		return;
	if ( pSpell->m_type == ITEM_WAND )	// equipped wands do not confer effect.
		return;

	switch ( pSpell->m_itSpell.m_spell )
	{
	case SPELL_Reactive_Armor:
		SetStat( STATF_Reactive );
		break;
	case SPELL_Night_Sight:
		SetStat( STATF_NightSight );
		if ( IsClient())
		{
			m_pClient->addLight();
		}
		break;
	case SPELL_Clumsy:
		if ( pSpell->m_itSpell.m_skilllevel >= Stat_Get(STAT_DEX))
			pSpell->m_itSpell.m_skilllevel = Stat_Get(STAT_DEX) - 1;
		Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Stone:
		SetStat( STATF_Stone );
		break;
	case SPELL_Hallucination:
		SetStat( STATF_Hallucinating );
	case SPELL_Feeblemind:
		if ( pSpell->m_itSpell.m_skilllevel >= Stat_Get(STAT_INT) )
			pSpell->m_itSpell.m_skilllevel = Stat_Get(STAT_INT) - 1;
		Stat_Set( STAT_INT, Stat_Get(STAT_INT) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Weaken:
		if ( pSpell->m_itSpell.m_skilllevel >= Stat_Get(STAT_STR) )
			pSpell->m_itSpell.m_skilllevel = Stat_Get(STAT_STR) - 1;
		Stat_Set( STAT_STR, Stat_Get(STAT_STR) - pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Agility:
		Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Cunning:
		Stat_Set( STAT_INT, Stat_Get(STAT_INT) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Strength:
		Stat_Set( STAT_STR, Stat_Get(STAT_STR) + pSpell->m_itSpell.m_skilllevel );
		break;
	case SPELL_Bless:
		{
			for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
			{
				Stat_Set( (STAT_TYPE) i, m_Stat[i] + pSpell->m_itSpell.m_skilllevel );
			}
		}
		break;
	case SPELL_Curse:
	case SPELL_Mass_Curse:
		{
			int i=STAT_STR;
			for ( ; i<STAT_BASE_QTY; i++ )
			{
				if ( pSpell->m_itSpell.m_skilllevel >= Stat_Get((STAT_TYPE)i) )
					pSpell->m_itSpell.m_skilllevel = Stat_Get((STAT_TYPE)i) - 1;
			}
			for ( i=STAT_STR; i<STAT_BASE_QTY; i++ )
			{
				Stat_Set( (STAT_TYPE) i, Stat_Get((STAT_TYPE)i) - pSpell->m_itSpell.m_skilllevel );
			}
		}
		break;
	case SPELL_Incognito:
		if ( ! IsStat( STATF_Incognito ))
		{
			SetStat( STATF_Incognito );
			pSpell->SetName( GetName());	// Give it my name
			SetName( m_pDef->GetTypeName());	// Give me general name for the type
			if ( ! IsStat( STATF_Polymorph ) && IsHuman())
			{
				SetColor((COLOR_UNDERWEAR|COLOR_SKIN_LOW) + GetRandVal(COLOR_SKIN_HIGH-COLOR_SKIN_LOW));
			}
		}
		break;
	case SPELL_Magic_Reflect:
		SetStat( STATF_Reflection );
		break;
	case SPELL_Protection:
	case SPELL_Arch_Prot:
		m_defense = CalcArmorDefense();
		break;
	case SPELL_Invis:
		SetStat( STATF_Invisible );
		// m_color = COLOR_TRANSLUCENT;
		UpdateMove(GetTopPoint());
		break;
	case SPELL_Paralyze:
	case SPELL_Paralyze_Field:
		SetStat( STATF_Freeze );
		break;
	case SPELL_Polymorph:
		DEBUG_CHECK( ! IsStat( STATF_Polymorph ));	// Should have already been removed.
		SetStat( STATF_Polymorph );
		break;
	case SPELL_Summon:
		// LAYER_SPELL_Summon
		SetStat( STATF_Conjured );
		break;
	default:
		return;
	}
	UpdateStatsFlag();
}

CItem * CChar::Spell_Effect_Create( SPELL_TYPE spell, LAYER_TYPE layer, int iLevel, int iDuration, CObjBase * pSrc )
{
	// ARGS:
	// spell = SPELL_Invis, etc.
	// layer == LAYER_FLAG_Potion, etc.
	// iLevel = 0-1000 = skill level or other spell specific value.
	// 

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_CHECK( iLevel <= 1000 );
	}
	CItem * pSpell = CItem::CreateBase( g_Serv.m_SpellDefs[ spell ]->m_SpellID );
	pSpell->m_type = ITEM_SPELL;
	pSpell->m_Attr |= ATTR_MAGIC | ATTR_NEWBIE;
	pSpell->m_itSpell.m_spell = spell;
	pSpell->m_itSpell.m_skilllevel = iLevel;	// 0 - 1000
	pSpell->m_itSpell.m_charges = 1;
	pSpell->SetDecayTime( iDuration );
	if ( pSrc )
	{
		pSpell->m_uidLink = pSrc->GetUID();
	}
	LayerAdd( pSpell, layer );	// Remove any competing effect first.
	Spell_Effect_Add( pSpell );
	return( pSpell );
}

int CChar::Spell_Linearity( SPELL_TYPE spell, int iSkillVal ) const
{
	int iLow = g_Serv.m_SpellDefs[spell]->m_wEffectLo;
	int iHigh = g_Serv.m_SpellDefs[spell]->m_wEffectHi;
	iHigh = ( iHigh - iLow ) / 2;
	iHigh = iHigh + GetRandVal( iHigh );	// Half random effect.
	return( iLow + IMULDIV( iHigh, iSkillVal, 1000 ));
}

void CChar::Spell_Bolt( CObjBase * pObjTarg, ITEMID_TYPE idBolt, int iSkill )
{
	// I am casting a bolt spell.
	// ARGS:
	// iSkill = 0-1000
	//

	if ( pObjTarg == NULL )
		return;
	pObjTarg->Effect( EFFECT_BOLT, idBolt, this, 5, 1, true );
	// Take damage !
	pObjTarg->OnSpellEffect( m_atMagery.m_Spell, this, iSkill );
}

void CChar::Spell_Area( CPointMap pntTarg, int iDist, int iSkill )
{
	// Effects all creatures in the area. (but not us)
	// ARGS:
	// iSkill = 0-1000
	//
	CWorldSearch Area( pntTarg, iDist );
	while (true)
	{
		CChar * pChar = Area.GetChar();
		if ( pChar == NULL )
			break;
		if ( pChar == this ) // not harm the caster.
		{
			if ( g_Serv.m_SpellDefs[m_atMagery.m_Spell]->IsSpellType( SPELLFLAG_HARM ))
				continue;
		}
		pChar->OnSpellEffect( m_atMagery.m_Spell, this, iSkill );
	}
}

void CChar::Spell_Field( CPointMap pntTarg, ITEMID_TYPE idEW, ITEMID_TYPE idNS, int iSkill )
{
	// Cast the field spell to here.
	// ARGS:
	// iSkill = 0-1000
	//

	// get the dir of the field.
	int dx = abs( pntTarg.m_x - GetTopPoint().m_x );
	int dy = abs( pntTarg.m_y - GetTopPoint().m_y );
	ITEMID_TYPE id = ( dx > dy ) ? idNS : idEW;

	for ( int i=-3; i<=3; i++ )
	{
		bool fGoodLoc = true;

		// Where is this ?
		CPointMap pg = pntTarg;
		if ( dx > dy )
			pg.m_y += i;
		else
			pg.m_x += i;

		// Check for direct cast on a creature.
		CWorldSearch AreaChar( pg );
		while (true)
		{
			CChar * pChar = AreaChar.GetChar();
			if ( pChar == NULL )
				break;
			if ( ! pChar->OnAttackedBy( this, false ))
				return;
			if ( idEW == ITEMID_STONE_WALL )
			{
				fGoodLoc = false;
				break;
			}
		}

		// Check for direct cast on an item.
		CWorldSearch AreaItem( pg );
		while (true)
		{
			CItem * pItem = AreaItem.GetItem();
			if ( pItem == NULL ) 
				break;
			pItem->OnSpellEffect( m_atMagery.m_Spell, this, iSkill );
		}

		if ( fGoodLoc)
		{
		CItem * pSpell = CItem::CreateBase( id );
		pSpell->m_type = ITEM_SPELL;
		pSpell->m_Attr |= ATTR_MAGIC;
		pSpell->m_itSpell.m_spell = m_atMagery.m_Spell;
		pSpell->m_itSpell.m_skilllevel = 15 + iSkill/25;
		pSpell->m_itSpell.m_charges = 1;
		pSpell->m_uidLink = GetUID();	// Link it back to you
		pSpell->SetDecayTime( pSpell->m_itSpell.m_skilllevel*TICK_PER_SEC + GetRandVal(60*TICK_PER_SEC) );
		pSpell->MoveTo( pg );
		}
	}

}

static const BYTE Spell_ManaReq[10] =
{
	4, 6, 9, 11, 14, 20, 40, 50, // Main spell book.
	15, 40,	// Necro spells.
};

bool CChar::Spell_CanCast( SPELL_TYPE spell, bool fTest, CObjBase * pSrc, bool fFailMsg )
{
	// ARGS:
	//  pSrc = possible scroll or wand source.
	// Do we have enough mana to start ?
	if ( spell <= SPELL_NONE ||
		pSrc == NULL ||
		! g_Serv.m_SpellDefs.IsValidIndex(spell) ||
		g_Serv.m_SpellDefs[spell]->IsSpellType( SPELLFLAG_DISABLED ))
		return( false );

	// if ( ! fTest || m_pNPC )
	{
		if ( ! IsPriv(PRIV_GM) && m_pArea && m_pArea->CheckAntiMagic( spell ))
		{
			if ( fFailMsg )
				SysMessage( "An anti-magic field disturbs the spells." );
			m_Act_Difficulty = -1;	// Give very little credit for failure !
			return( false );
		}
	}

	int mana = Spell_ManaReq[ (spell-SPELL_Clumsy)/8 ];

	// The magic item must be on your person to use.
	if ( pSrc != this )
	{
		CItem * pItem = dynamic_cast <CItem*> (pSrc);
		if ( pItem == NULL )
		{
			DEBUG_CHECK( 0 );
			return( false );	// where did it go ?
		}
		if ( ! pItem->IsAttr( ATTR_MAGIC ))
		{
			if ( fFailMsg )
				SysMessage( "This item lacks any enchantment." );
			return( false );
		}
		CObjBaseTemplate * pObjTop = pSrc->GetTopLevelObj();
		if ( pObjTop != this )
		{
			if ( fFailMsg )
				SysMessage( "Magic items must be on your person to activate." );
			return( false );
		}
		if ( pItem->m_type == ITEM_WAND )
		{
			// Must have charges.
			if ( pItem->m_itWeapon.m_charges <= 0 )
			{
				// ??? May explode !!
				if ( fFailMsg )
					SysMessage( "It seems to be out of charges" );
				return false;
			}
			mana = 0;	// magic items need no mana.
			if ( ! fTest && pItem->m_itWeapon.m_charges != 255 )
			{
				pItem->m_itWeapon.m_charges --;
			}
		}
		else	// Scroll
		{
			mana /= 2;
			if ( ! fTest )
			{
				Use_Consume( pItem );
			}
		}
	}
	else
	{
		// Raw cast from spellbook.

		if ( IsPriv( PRIV_GM )) 
			return( true );

		if ( IsStat( STATF_DEAD|STATF_Sleeping ) ||
			Spell_GetBaseDifficulty( spell ) > Skill_GetAdjusted( SKILL_MAGERY ))
		{
			if ( fFailMsg )
				SysMessage( "This is beyond your ability." );
			return( false );
		}

		if ( m_pPlayer )
		{
			// check the spellbook for it.
			CItem * pBook = GetSpellbook();
			if ( pBook == NULL )
			{
				if ( fFailMsg )
					SysMessage( "You don't have a spellbook handy." );
				return( false );
			}
			if ( ! pBook->IsSpellInBook( spell ))
			{
				if ( fFailMsg )
					SysMessage( "The spell is not in your spellbook." );
				return( false );
			}
		}
	}

	if ( m_StatMana < mana )
	{
		if ( fFailMsg )
			SysMessage( "You lack sufficient mana for this spell" );
		return( false );
	}

	if ( ! fTest && mana )
	{
		// Consume mana.
		if ( m_Act_Difficulty < 0 )	// use diff amount of mana if we fail.
		{
			mana = mana/2 + GetRandVal( mana/2 + mana/4 );
		}
		UpdateStats( STAT_INT, -mana );
	}

	if ( m_pNPC ||	// NPC's don't need regs.
		pSrc != this )	// wands and scrolls have there own reags source.
		return( true );

	// Check for regs ?
	if ( g_Serv.m_fReagentsRequired )
	{
		CItemContainer * pPack = GetPack();
		if ( pPack )
		{
			const TCHAR * pszRegs = g_Serv.m_SpellDefs[spell]->m_sReags;
			while ( pszRegs[0] )
			{
				if ( *pszRegs == ',' ) pszRegs ++;
				const TCHAR * pRegPrv = pszRegs;
				if ( pPack->ContentConsume( GetResourceItem(const_cast<TCHAR * &>(pszRegs)), 1, fTest ))
				{
					if ( fFailMsg )
					{
						SysMessagef( "You lack reagents for this spell (%s)", pRegPrv );
					}
					return( false );
				}
				if ( pRegPrv == pszRegs ) break;	// should not happen.
			}
		}
	}
	return( true );
}

bool CChar::Spell_TargCheck()
{
	// Is the spells target or target pos valid ?

	if ( m_atMagery.m_Spell <= SPELL_NONE ||
		! g_Serv.m_SpellDefs.IsValidIndex( m_atMagery.m_Spell ))
	{
		DEBUG_ERR(( "Bad Spell %d, uid 0%0x\n", m_atMagery.m_Spell, GetUID()));
		return( false );
	}

	CObjBase * pObj = m_Act_Targ.ObjFind();
	CObjBaseTemplate * pObjTop;
	if ( pObj )
	{
		pObjTop = pObj->GetTopLevelObj();
	}

	// Need a target.
	const CSpellDef * pDef = g_Serv.m_SpellDefs[m_atMagery.m_Spell];
	if ( pDef->IsSpellType( SPELLFLAG_TARG_OBJ | SPELLFLAG_TARG_CHAR ))
	{
		if ( pObj == NULL )
		{
			SysMessage( "This spell needs a target object" );
			return( false );
		}
		if ( ! CanSeeLOS( pObj ))
		{
			SysMessage( "Target is not in line of sight" );
			return( false );
		}
		if ( ! IsPriv( PRIV_GM ) && pObjTop != pObj && pObjTop->IsChar() && pObjTop != this )
		{
			SysMessage( "Can't cast a spell on this where it is" );
			return( false );
		}

#if 0
		if ( pDef->IsSpellType( SPELLFLAG_TARG_CHAR ))
		{
			CChar * pChar = dynamic_cast <CChar*> ( pObj );
			// Need a char target.
			if ( pChar == NULL )
			{
				SysMessage( "This spell has no effect on objects" );
				return( false );
			}
		}
#endif
		m_Act_p = pObjTop->GetTopPoint();

facepoint:
		UpdateDir(m_Act_p);

		// Is my target in an anti magic feild.
		CRegionWorld * pArea = dynamic_cast <CRegionWorld *> ( m_Act_p.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
		if ( ! IsPriv(PRIV_GM) && pArea && pArea->CheckAntiMagic( m_atMagery.m_Spell ))
		{
			SysMessage( "An anti-magic field disturbs the spells." );
			m_Act_Difficulty = -1;	// Give very little credit for failure !
			return( false );
		}
	}
	else if ( pDef->IsSpellType( SPELLFLAG_TARG_XYZ ))
	{
		if ( pObj )
		{
			m_Act_p = pObjTop->GetTopPoint();
		}
		if ( ! CanSeeLOS( m_Act_p, NULL, UO_MAP_VIEW_SIGHT ))
		{
			SysMessage( "Target is not in line of sight" );
			return( false );
		}
		goto facepoint;
	}

	return( true );
}

bool CChar::Spell_Done( void )
{
	// Ready for the spell effect.
	// m_Act_TargPrv = spell was magic item or scroll ?
	// RETURN: 
	//  false = fail.
	//

	if ( ! Spell_TargCheck())
		return( false );

	CObjBase * pObj = m_Act_Targ.ObjFind();	// dont always need a target.
	CObjBase * pObjSrc = m_Act_TargPrv.ObjFind();

	int iSkill;
	if ( pObjSrc != this )
	{
		// Get the strength of the item. ITEM_SCROLL or ITEM_WAND
		CItem * pItem = dynamic_cast <CItem*>(pObjSrc);
		if ( pItem == NULL )
			return( false );
		if ( ! pItem->m_itWeapon.m_skilllevel )
			iSkill = GetRandVal( 500 );
		else
			iSkill = pItem->m_itWeapon.m_skilllevel * 10;
		if ( pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ))
		{
			// do something bad.
			if ( ! g_Serv.m_SpellDefs[m_atMagery.m_Spell]->IsSpellType( SPELLFLAG_HARM ))
			{
				m_atMagery.m_Spell = SPELL_Curse;
			}
			pItem->m_Attr |= ATTR_IDENTIFIED;
			pObj = this;
			SysMessage( "Cursed Magic!" );
		}
	}
	else
	{
		iSkill = Skill_GetAdjusted( SKILL_MAGERY );
	}

	// Consume the reagents/mana/scroll/charge
	if ( ! Spell_CanCast( m_atMagery.m_Spell, false, pObjSrc, true ))
		return( false );

	switch ( m_atMagery.m_Spell )
	{
		// 1st
	case SPELL_Create_Food:
		// Create object. Normally food.
		if ( pObj == NULL )
		{
			static const ITEMID_TYPE Item_Foods[] =	// possible foods.
			{
				ITEMID_FOOD_BACON,
				ITEMID_FOOD_SAUSAGE,
				ITEMID_FOOD_HAM,
				ITEMID_FOOD_CAKE,
				ITEMID_FOOD_BREAD,
			};

			CItem * pItem = CItem::CreateScript( Item_Foods[ GetRandVal( COUNTOF( Item_Foods )) ] );
			pItem->m_type = ITEM_FOOD;	// should already be set .
			pItem->MoveToCheck( m_Act_p );
		}
		break;

	case SPELL_Magic_Arrow:
		Spell_Bolt( pObj, ITEMID_FX_MAGIC_ARROW, iSkill );
		break;

	case SPELL_Heal:
	case SPELL_Night_Sight:
	case SPELL_Reactive_Armor:

	case SPELL_Clumsy:
	case SPELL_Feeblemind:
	case SPELL_Weaken:
		goto simple_effect;

		// 2nd
	case SPELL_Agility:
	case SPELL_Cunning:
	case SPELL_Cure:
	case SPELL_Protection:
	case SPELL_Strength:

	case SPELL_Harm:
simple_effect:
		if ( pObj == NULL ) 
			return( false );
		pObj->OnSpellEffect( m_atMagery.m_Spell, this, iSkill );
		break;

	case SPELL_Magic_Trap:
	case SPELL_Magic_Untrap:
		// Create the trap object and link it to the target. ???
		// A container is diff from door or stationary object
		break;

		// 3rd
	case SPELL_Bless:

	case SPELL_Poison:
		goto simple_effect;
	case SPELL_Fireball:
		Spell_Bolt( pObj, ITEMID_FX_FIRE_BALL, iSkill );
		break;

	case SPELL_Magic_Lock:
	case SPELL_Unlock:
		goto simple_effect;

	case SPELL_Telekin:
		// Act as dclick on the object.
		Use_Obj( pObj, false );
		break;
	case SPELL_Teleport:
		Spell_Teleport( m_Act_p );
		break;
	case SPELL_Wall_of_Stone:
		Spell_Field( m_Act_p, ITEMID_STONE_WALL, ITEMID_STONE_WALL, iSkill );
		break;

		// 4th
	case SPELL_Arch_Cure:
	case SPELL_Arch_Prot:
	{
		Spell_Area( m_Act_p, 5, iSkill );
		break;
	}
	case SPELL_Great_Heal:

	case SPELL_Curse:
	case SPELL_Lightning:
		goto simple_effect;
	case SPELL_Fire_Field:
		Spell_Field( m_Act_p, ITEMID_FX_FIRE_F_EW, ITEMID_FX_FIRE_F_NS, iSkill );
		break;

	case SPELL_Recall:
		if ( ! Spell_Recall( dynamic_cast <CItem*> (pObj), false ))
			return( false );
		break;

		// 5th

	case SPELL_Blade_Spirit:
		m_atMagery.m_SummonID = CREID_BLADES;
		goto summon_effect;

	case SPELL_Dispel_Field:
		{
			CItem * pItem = dynamic_cast <CItem*> (pObj);
			if ( pItem == NULL || ( pItem->m_type != ITEM_FIRE && pItem->m_type != ITEM_SPELL ))
			{
				SysMessage( "That is not a field!" );
				return( false );
			}
			pItem->Delete();
			break;
		}

	case SPELL_Mind_Blast:
		if ( pObj->IsChar())
		{
			CChar * pChar = dynamic_cast <CChar*> ( pObj );
			ASSERT( pChar );
			int iDiff = ( Stat_Get(STAT_INT) - pChar->Stat_Get(STAT_INT) ) / 2;
			if ( iDiff < 0 )
			{
				pChar = this;	// spell revereses !
				iDiff = -iDiff;
			}
			int iMax = pChar->Stat_Get(STAT_STR) / 4;
			pChar->OnSpellEffect( m_atMagery.m_Spell, this, min( iDiff, iMax ));
		}
		break;

	case SPELL_Magic_Reflect:

	case SPELL_Paralyze:
	case SPELL_Incognito:
		goto simple_effect;

	case SPELL_Poison_Field:
		Spell_Field( m_Act_p, ITEMID_FX_POISON_F_EW, ITEMID_FX_POISON_F_NS, iSkill );
		break;

	case SPELL_Summon:
	summon_effect:
		Spell_Summon( m_atMagery.m_SummonID, m_Act_p );
		break;

		// 6th

	case SPELL_Invis:

	case SPELL_Dispel:
		goto simple_effect;

	case SPELL_Energy_Bolt:
		Spell_Bolt( pObj, ITEMID_FX_ENERGY_BOLT, iSkill );
		break;

	case SPELL_Explosion:
		Spell_Area( m_Act_p, 2, iSkill );
		break;

	case SPELL_Mark:
		goto simple_effect;

	case SPELL_Mass_Curse:
		Spell_Area( m_Act_p, 5, iSkill );
		break;
	case SPELL_Paralyze_Field:
		Spell_Field( m_Act_p, ITEMID_FX_PARA_F_EW, ITEMID_FX_PARA_F_NS, iSkill );
		break;
	case SPELL_Reveal:
		Spell_Area( m_Act_p, UO_MAP_VIEW_SIGHT, iSkill );
		break;

		// 7th

	case SPELL_Chain_Lightning:
		Spell_Area( m_Act_p, 5, iSkill );
		break;
	case SPELL_Energy_Field:
		Spell_Field( m_Act_p, ITEMID_FX_ENERGY_F_EW, ITEMID_FX_ENERGY_F_NS, iSkill );
		break;

	case SPELL_Flame_Strike:
		// Display spell.
		if ( pObj == NULL )
		{
			CItem * pItem = CItem::CreateBase( ITEMID_FX_FLAMESTRIKE );
			pItem->m_type = ITEM_SPELL;
			pItem->m_itSpell.m_spell = SPELL_Flame_Strike;
			pItem->SetDecayTime( 2*TICK_PER_SEC );
			pItem->MoveTo( m_Act_p );
		}
		else
		{
			pObj->Effect( EFFECT_OBJ, ITEMID_FX_FLAMESTRIKE, pObj, 6, 15 );
			// Burn person at location.
			goto simple_effect;
		}
		break;

	case SPELL_Gate_Travel:
		if ( ! Spell_Recall( dynamic_cast <CItem*> (pObj), true ))
			return( false );
		break;

	case SPELL_Mana_Drain:
	case SPELL_Mana_Vamp:
		// Take the mana from the target.
		if ( pObj->IsChar() && this != pObj )
		{
			CChar * pChar = dynamic_cast <CChar*> ( pObj );
			ASSERT( pChar );
			if ( ! pChar->IsStat( STATF_Reflection ))
			{
				int iMax = pChar->Stat_Get(STAT_INT);
				int iDiff = Stat_Get(STAT_INT) - iMax;
				if ( iDiff < 0 ) iDiff = 0;
				else iDiff = GetRandVal( iDiff );
				iDiff += GetRandVal( 25 );
				pChar->OnSpellEffect( m_atMagery.m_Spell, this, iDiff );
				if ( m_atMagery.m_Spell == SPELL_Mana_Vamp )
				{
					// Give some back to me.
					UpdateStats( STAT_INT, min( iDiff, Stat_Get(STAT_INT)));
				}
				break;
			}
		}
		goto simple_effect;

	case SPELL_Mass_Dispel:
		Spell_Area( m_Act_p, 15, iSkill );
		break;

	case SPELL_Meteor_Swarm:
		// Multi explosion ??? 0x36b0
		Spell_Area( m_Act_p, 4, iSkill );
		break;

	case SPELL_Polymorph:
		// This has a menu select for client.
		if ( GetPrivLevel() < PLEVEL_Seer )
		{
			if ( pObj != this ) 
				return( false );
		}
		goto simple_effect;

		// 8th

	case SPELL_Earthquake:
		Spell_Area( GetTopPoint(), UO_MAP_VIEW_SIGHT, iSkill );
		break;

	case SPELL_Vortex:
		m_atMagery.m_SummonID = CREID_VORTEX;
		goto summon_effect;

	case SPELL_Resurrection:
	case SPELL_Light:
		goto simple_effect;

	case SPELL_Air_Elem:
		m_atMagery.m_SummonID = CREID_AIR_ELEM;
		goto summon_effect;
	case SPELL_Daemon:
		m_atMagery.m_SummonID = ( GetRandVal( 2 )) ? CREID_DAEMON_SWORD : CREID_DAEMON;
		goto summon_effect;
	case SPELL_Earth_Elem:
		m_atMagery.m_SummonID = CREID_EARTH_ELEM;
		goto summon_effect;
	case SPELL_Fire_Elem:
		m_atMagery.m_SummonID = CREID_FIRE_ELEM;
		goto summon_effect;
	case SPELL_Water_Elem:
		m_atMagery.m_SummonID = CREID_WATER_ELEM;
		goto summon_effect;

		// Necro
	case SPELL_Summon_Undead:
		switch (GetRandVal(15))
		{
		case 1:
			m_atMagery.m_SummonID = CREID_LICH;
			break;
		case 3:
		case 5:
		case 7:
		case 9:
			m_atMagery.m_SummonID = CREID_SKELETON;
			break;
		default:
			m_atMagery.m_SummonID = CREID_ZOMBIE;
			break;
		}
		goto summon_effect;

	case SPELL_Animate_Dead:
		{
			CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pObj);
			if ( pCorpse == NULL )
			{
				SysMessage( "That is not a corpse!" );
				return( false );
			}
			if ( IsPriv( PRIV_GM ))
			{
				m_atMagery.m_SummonID = pCorpse->m_itCorpse.m_BaseID;
			}
			else if ( CCharBase::IsHuman( pCorpse->GetCorpseType())) 	// Must be a human corpse ?
			{
				m_atMagery.m_SummonID = CREID_ZOMBIE;
			}
			else
			{
				m_atMagery.m_SummonID = pCorpse->GetCorpseType();
			}

			if ( ! pCorpse->IsTopLevel())
			{
				return( false );
			}
			CChar * pChar = Spell_Summon( m_atMagery.m_SummonID, pCorpse->GetTopPoint());
			ASSERT( pChar );
			if ( ! pChar->RaiseCorpse( pCorpse ))
			{
				SysMessage( "The corpse stirs for a moment then falls!" );
				pChar->Delete();
			}
			break;
		}

	case SPELL_Bone_Armor:
		{
			CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pObj);
			if ( pCorpse == NULL )
			{
				SysMessage( "That is not a corpse!" );
				return( false );
			}
			if ( ! pCorpse->IsTopLevel() || 
				pCorpse->GetCorpseType() != CREID_SKELETON ) 	// Must be a skeleton corpse
			{
				SysMessage( "The body stirs for a moment" );
				return( false );
			}
			// Dump any stuff on corpse
			pCorpse->ContentsDump( pCorpse->GetTopPoint());
			pCorpse->Delete();

			static const WORD Item_Bone[] =
			{
				ITEMID_BONE_ARMS,
				ITEMID_BONE_ARMOR,
				ITEMID_BONE_GLOVES,
				ITEMID_BONE_HELM,
				ITEMID_BONE_LEGS,
			};

			int iGet = 0;
			for ( int i=0; i<COUNTOF(Item_Bone); i++ )
			{
				if ( ! GetRandVal( 2+iGet )) 
					break;
				CItem * pItem = CItem::CreateScript( (ITEMID_TYPE) Item_Bone[i] );
				pItem->MoveToCheck( m_Act_p );
				iGet++;
			}
			if ( ! iGet )
			{
				SysMessage( "The bones shatter into dust!" );
				break;
			}
		}
		break;

	case SPELL_Fire_Bolt:
		Spell_Bolt( pObj, ITEMID_FX_FIRE_BOLT, iSkill );
		break;
	case SPELL_Hallucination:
		goto simple_effect;
	case SPELL_Stone:
		goto simple_effect;
	}

	if ( g_Serv.m_fHelpingCriminalsIsACrime &&
		pObj != NULL && 
		pObj->IsChar() &&
		pObj != this &&
		g_Serv.m_SpellDefs[m_atMagery.m_Spell]->IsSpellType(SPELLFLAG_GOOD))
	{
		CChar * pChar = dynamic_cast <CChar*> ( pObj );
		ASSERT( pChar );
		switch ( pChar->GetNotoFlag( this, false ))
		{
		case NOTO_CRIMINAL:
		case NOTO_GUILD_WAR:
		case NOTO_EVIL:
			Noto_Criminal();
			break;
		}
	}

	// Make noise.
	if ( ! IsStat( STATF_Insubstantial ))
	{
		Sound( g_Serv.m_SpellDefs[ m_atMagery.m_Spell ]->m_sound );
	}
	return( true );
}

int CChar::Spell_StartCast()
{
	// m_atMagery.m_Spell = the spell.
	// m_Act_TargPrv = the source of the spell.
	// RETURN: -1 = failure.

	if ( ! Spell_TargCheck())
		return( -1 );

	// Animate casting.
	ASSERT( g_Serv.m_SpellDefs.IsValidIndex(m_atMagery.m_Spell) );
	if ( ! g_Serv.m_SpellDefs.IsValidIndex(m_atMagery.m_Spell))
		return( -1 );

	UpdateAnimate(( g_Serv.m_SpellDefs[m_atMagery.m_Spell]->IsSpellType( SPELLFLAG_DIR_ANIM )) ? ANIM_CAST_DIR : ANIM_CAST_AREA );

	bool fWOP = ( GetPrivLevel() >= PLEVEL_Seer ) ?
		g_Serv.m_fWordsOfPowerStaff :
		g_Serv.m_fWordsOfPowerPlayer ;

	if ( ! NPC_CanSpeak() || IsStat( STATF_Insubstantial ))
	{
		fWOP = false;
	}

	int iDifficulty;
	CObjUID uid( m_Act_TargPrv );
	CItem * pItem = uid.ItemFind();
	if ( pItem != NULL )
	{
		if ( pItem->m_type == ITEM_WAND )
		{
			// Wand use no words of power.
			fWOP = false;
			iDifficulty = Spell_GetBaseDifficulty( m_atMagery.m_Spell ) / 10;
		}
		else
		{
			// Scroll
			iDifficulty = Spell_GetBaseDifficulty( m_atMagery.m_Spell ) / 2;
		}
	}
	else
	{
		iDifficulty = Spell_GetBaseDifficulty( m_atMagery.m_Spell );
	}

	if ( ! g_Serv.m_fEquippedCast && fWOP )
	{
		// Attempt to Unequip stuff before casting.
		// Except not wands !
		CItem * pItemPrev = LayerFind( LAYER_HAND1 );
		if ( pItemPrev != NULL )
		{
			if ( ! CanMove( pItemPrev ))
				return( -1 );
			ItemBounce( pItemPrev );
		}
		pItemPrev = LayerFind( LAYER_HAND2 );
		if ( pItemPrev != NULL )
		{
			if ( ! CanMove( pItemPrev ))
				return( -1 );
			ItemBounce( pItemPrev );
		}
	}

	if ( fWOP )
	{
		int len=0;
		TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
		int i=0;
		for ( ; true; i++ )
		{
			TCHAR ch = g_Serv.m_SpellDefs[m_atMagery.m_Spell]->m_sRunes[i];
			if ( ! ch )
				break;
			ch = toupper(ch) - 'A';
			if ( ! g_Serv.m_Runes.IsValidIndex(ch))
				continue;
			len += strcpylen( szTemp+len, g_Serv.m_Runes[ ch ] );
			szTemp[len++]=' ';
		}
		szTemp[len] = '\0';
		Speak( szTemp );
	}

	if ( OnTrigger( CTRIG_SpellCast, this, m_atMagery.m_Spell ))
		return( -1 );

	return( iDifficulty );
}

bool CChar::Spell_Cast( SPELL_TYPE spell, CObjUID uid, CPointMap pntTarg )
{
	// The char (player or npc) is casting a spell.
	// Begin casting the spell

	Reveal();

	m_atMagery.m_Spell = spell;
	m_Act_Targ = uid;
	m_Act_TargPrv = GetUID();	// I'm casting this directly.
	m_Act_p = pntTarg;

	// Calculate the difficulty
	return( Skill_Start( SKILL_MAGERY ));
}

bool CChar::OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkill )
{
	// Spell has a direct effect on this char.
	// This should effect noto of source.
	// iSkill = 0-1000 = difficulty. may be slightly larger .

	if ( this == NULL )
		return false;
	if ( IsItem())
		return false;
	if ( iSkill <= 0 )	// spell died (fizzled?).
		return false;

	if ( OnTrigger( CTRIG_SpellEffect, pCharSrc, spell ))
		return( false );

	// Most spells don't work on ghosts.
	if ( IsStat( STATF_DEAD ) && spell != SPELL_Resurrection )
		return false;

	switch ( spell )
	{
	case SPELL_Poison:
	case SPELL_Poison_Field:
		if ( IsStat(STATF_Poisoned))
			return false;	// no further effect.
		break;
	case SPELL_Paralyze_Field:
	case SPELL_Paralyze:
		if ( IsStat(STATF_Freeze))
			return false;	// no further effect.
		break;
	}

	if ( g_Serv.m_SpellDefs[spell]->IsSpellType( SPELLFLAG_HARM ))
	{
		if ( IsStat( STATF_INVUL ))
			return false;
		// Check resistance to magic ?
		if ( Skill_UseQuick( SKILL_MAGICRESISTANCE, Spell_GetBaseDifficulty( spell )))
		{
			SysMessage( "You feel yourself resisting magic" );
			iSkill /= 2;	// ??? reduce effect of spell.
		}

		if ( pCharSrc != NULL && GetPrivLevel() > PLEVEL_Guest )
		{
			if ( pCharSrc->GetPrivLevel() <= PLEVEL_Guest )
			{
				pCharSrc->SysMessage( "The guest curse strikes you." );
				goto reflectit;
			}
		}

		// Check magic reflect.
		if ( IsStat( STATF_Reflection ))	// reflected.
		{
			ClearStat( STATF_Reflection );
reflectit:
			Effect( EFFECT_OBJ, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
			if ( pCharSrc != NULL )
			{
				pCharSrc->OnSpellEffect( spell, NULL, iSkill/2 );
			}
			return false;
		}
		if ( ! OnAttackedBy( pCharSrc, false ))
			return false;
	}

	if ( g_Serv.m_SpellDefs[spell]->IsSpellType( SPELLFLAG_FX_TARG ) &&
		g_Serv.m_SpellDefs[spell]->m_wEffectID )
	{
		Effect( EFFECT_OBJ, g_Serv.m_SpellDefs[spell]->m_wEffectID, this, 0, 15 ); // 9, 14
	}

	switch ( spell )
	{

	case SPELL_Clumsy:
	case SPELL_Feeblemind:
	case SPELL_Weaken:
	case SPELL_Agility:
	case SPELL_Cunning:
	case SPELL_Strength:
	case SPELL_Bless:
	case SPELL_Curse:
	case SPELL_Mass_Curse:
		Spell_Effect_Create( spell, LAYER_SPELL_STATS, Spell_Linearity( spell, iSkill ), 3*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Heal:
	case SPELL_Great_Heal:
		UpdateStats( STAT_STR, Spell_Linearity( spell, iSkill ));
		break;

	case SPELL_Night_Sight:
		Spell_Effect_Create( SPELL_Night_Sight, LAYER_SPELL_Night_Sight, iSkill/10, 60*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Reactive_Armor:
		Spell_Effect_Create( SPELL_Reactive_Armor, LAYER_SPELL_Reactive, iSkill/10, 5*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Magic_Reflect:
		Spell_Effect_Create( SPELL_Magic_Reflect, LAYER_SPELL_Magic_Reflect, iSkill/10, 3*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Poison:
	case SPELL_Poison_Field:
		Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		SetPoison( Spell_Linearity( spell, iSkill ), pCharSrc );
		break;

	case SPELL_Cure:
		SetPoisonCure( iSkill, false );
		break;
	case SPELL_Arch_Cure:
		SetPoisonCure( iSkill, true );
		break;

	case SPELL_Protection:
	case SPELL_Arch_Prot:
		Spell_Effect_Create( SPELL_Protection, LAYER_SPELL_Protection, Spell_Linearity( spell, iSkill ), 5*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Dispel:
	case SPELL_Mass_Dispel:
		// ??? should be difficult to dispel SPELL_Summon creatures
		Spell_Dispel( (pCharSrc != NULL && pCharSrc->IsPriv(PRIV_GM)) ? 150 : 50);
		break;

	case SPELL_Reveal:
		if ( ! Reveal()) 
			break;
		Effect( EFFECT_OBJ, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		break;

	case SPELL_Invis:
		Spell_Effect_Create( SPELL_Invis, LAYER_SPELL_Invis, iSkill/10, (2+iSkill/200)*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Incognito:
		Spell_Effect_Create( SPELL_Incognito, LAYER_SPELL_Incognito, iSkill/10, 120*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Stone:
	case SPELL_Paralyze_Field:
	case SPELL_Paralyze:
		// Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		Spell_Effect_Create( spell, LAYER_SPELL_Paralyze, iSkill/10, 2*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Mana_Drain:
	case SPELL_Mana_Vamp:
		UpdateStats( STAT_INT, -iSkill );
		break;

	case SPELL_Harm:
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_POISON | DAMAGE_MAGIC | DAMAGE_GENERAL );
		break;
	case SPELL_Mind_Blast:
		OnTakeDamage( iSkill, pCharSrc, DAMAGE_POISON | DAMAGE_MAGIC | DAMAGE_GENERAL );
		break;
	case SPELL_Explosion:
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT | DAMAGE_GENERAL );
		break;
	case SPELL_Energy_Bolt:
	case SPELL_Magic_Arrow:
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT );
		break;
	case SPELL_Fireball:
	case SPELL_Fire_Bolt:
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT | DAMAGE_FIRE );
		break;
	case SPELL_Fire_Field:
	case SPELL_Flame_Strike:
		// Burn whoever is there.
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_MAGIC | DAMAGE_FIRE | DAMAGE_GENERAL );
		break;
	case SPELL_Meteor_Swarm:
		Effect( EFFECT_OBJ, ITEMID_FX_EXPLODE_3, this, 9, 6 );
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT | DAMAGE_FIRE );
		break;
	case SPELL_Earthquake:
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_HIT | DAMAGE_GENERAL );
		break;
	case SPELL_Lightning:
	case SPELL_Chain_Lightning:
		GetTopSector()->LightFlash();
		Effect( EFFECT_LIGHTNING, ITEMID_NOTHING, pCharSrc );
		OnTakeDamage( Spell_Linearity( spell, iSkill ), pCharSrc, DAMAGE_ELECTRIC | DAMAGE_GENERAL );
		break;

	case SPELL_Resurrection:
		return Spell_Resurrection( (pCharSrc && pCharSrc->IsPriv(PRIV_GM)) ? -1 : 0 );

	case SPELL_Light:
		Effect( EFFECT_OBJ, ITEMID_FX_HEAL_EFFECT, this, 9, 6 );
		Spell_Effect_Create( spell, LAYER_SPELL_Night_Sight, iSkill/10, 60*60*TICK_PER_SEC, pCharSrc );
		break;

	case SPELL_Hallucination:
		{
		CItem * pItem = Spell_Effect_Create( spell, LAYER_FLAG_Hallucination, Spell_Linearity( spell, iSkill ), 10*TICK_PER_SEC, pCharSrc );
		pItem->m_itSpell.m_charges = GetRandVal(30);
		}
		break;
	case SPELL_Polymorph:
		{

#define SPELL_MAX_POLY_STAT 150

			CItem * pSpell = Spell_Effect_Create( SPELL_Polymorph, LAYER_SPELL_Polymorph, 10, 20*60*TICK_PER_SEC, pCharSrc );
			SetID( m_atMagery.m_SummonID );

			// set to creature type stats.
			if ( m_pDef->m_Str )
			{
				int iStatPrv = Stat_Get(STAT_STR);
				int iChange = m_pDef->m_Str - iStatPrv;
				if ( iChange > SPELL_MAX_POLY_STAT )
					iChange = SPELL_MAX_POLY_STAT;
				if ( iChange < -50 )
					iChange = -50;
				Stat_Set( STAT_STR, iChange + iStatPrv );
				pSpell->m_itSpell.m_PolyStr = Stat_Get(STAT_STR) - iStatPrv;
			}
			else
			{
				pSpell->m_itSpell.m_PolyStr = 0;
			}
			if ( m_pDef->m_Dex )
			{
				int iStatPrv = Stat_Get(STAT_DEX);
				int iChange = m_pDef->m_Dex - iStatPrv;
				if ( iChange > SPELL_MAX_POLY_STAT )
					iChange = SPELL_MAX_POLY_STAT;
				if ( iChange < -50 )
					iChange = -50;
				Stat_Set( STAT_DEX, iChange + iStatPrv );
				pSpell->m_itSpell.m_PolyDex = Stat_Get(STAT_DEX) - iStatPrv;
			}
			else
			{
				pSpell->m_itSpell.m_PolyDex = 0;
			}
			Update();		// show everyone I am now a new type
		}
		break;
	}
	return( true );
}

