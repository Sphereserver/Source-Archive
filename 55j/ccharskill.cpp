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
			CItem * pItem = CItem::CreateScript( Calc_GetRandVal(2) ? ITEMID_FX_FIRE_F_EW : ITEMID_FX_FIRE_F_NS );
			ASSERT(pItem);
			pItem->SetType( IT_FIRE );
			pItem->m_itSpell.m_spell = SPELL_Fire_Field;
			pItem->m_itSpell.m_spelllevel = 100 + Calc_GetRandVal(500);
			pItem->m_itSpell.m_spellcharges = 1;
			pItem->m_uidLink = GetUID();	// Link it back to you
			pItem->MoveToDecay( GetTopPoint(), 30*TICK_PER_SEC + Calc_GetRandVal(60*TICK_PER_SEC));
		}
		break;

	case CREID_GIANT_SPIDER:
		// Leave a web patch.
		{
			static const WORD sm_Webs[] =
			{
				ITEMID_WEB1_1,
				ITEMID_WEB1_1+1,
				ITEMID_WEB1_1+2,
				ITEMID_WEB1_4,
			};
			CItem * pItem = CItem::CreateScript( (ITEMID_TYPE) sm_Webs[ Calc_GetRandVal( COUNTOF(sm_Webs))] );
			pItem->SetType(IT_WEB);
			pItem->MoveToCheck( GetTopPoint(), this );
			pItem->SetDecayTime( 3*60*60*TICK_PER_SEC );
		}
		break;

	default:
		SysMessage( "You have no special abilities" );
		return;	// No special ability.
	}

	// loss of stamina for a bit.
	UpdateStatVal( STAT_DEX, -( 5 + Calc_GetRandVal(5)));	// The cost in stam.
}

void CChar::Stat_Set( STAT_TYPE i, short iVal )
{
	ASSERT(((WORD)i)<STAT_QTY );

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	switch ( i )
	{
	case STAT_STR:
		if ( ! pCharDef->m_Str )
			pCharDef->m_Str = iVal;
		goto check_neg;
	case STAT_DEX:
		if ( ! pCharDef->m_Dex )
			pCharDef->m_Dex = iVal;
		goto check_neg;
	case STAT_INT:
check_neg:
		if ( iVal <= 0 )
		{
			// Setting stats negative might be acceptable on a temporary basis.
			// iVal = 1;
		}
		break;
	case STAT_Karma:		// -10000 to 10000
		break;
	case STAT_Fame:
		if ( iVal < 0 )
		{
			DEBUG_ERR(( "ID=0%x,UID=0%x Fame set out of range %d" DEBUG_CR, GetBaseID(), GetUID(), iVal ));
			iVal = 0;
		}
		break;
	}
	m_Stat[i] = iVal;
	UpdateStatsFlag();
}

short CChar::Stat_GetMax( STAT_TYPE i ) const
{
	if ( m_pPlayer )
	{
		const CSkillClassDef* pSkillClass = m_pPlayer->GetSkillClass();
		ASSERT(pSkillClass);
		if ( i == STAT_QTY )
		{
			return pSkillClass->m_StatSumMax;
		}
		ASSERT( i<STAT_BASE_QTY );
		return pSkillClass->m_StatMax[i];
	}
	else
	{
		if ( i == STAT_QTY )
		{
			return 300;
		}
		return 100;	// just can't grow past this.
	}
}

//----------------------------------------------------------------------
// Skills

SKILL_TYPE CChar::Skill_GetBest( int iRank ) const // Which skill is the highest for character p
{
	// Get the top n best skills.

	if ( iRank < 0 || iRank >= SKILL_QTY ) 
		iRank = 0;

	DWORD * pdwSkills = new DWORD [iRank+1];
	ASSERT(pdwSkills);
	memset( pdwSkills, 0, (iRank+1) * sizeof(DWORD));

	DWORD dwSkillTmp;
	for ( int i=0;i<SKILL_QTY;i++)
	{
		dwSkillTmp = MAKEDWORD( i, Skill_GetBase( (SKILL_TYPE)i ));
		for ( int j=0; j<=iRank; j++ )
		{
			if ( dwSkillTmp > pdwSkills[j] )
			{
				memmove( pdwSkills+j+1, pdwSkills+j, iRank-j );
				pdwSkills[j] = dwSkillTmp;
				break;
			}
		}
	}

	dwSkillTmp = pdwSkills[ iRank ];
	delete [] pdwSkills;
	return( (SKILL_TYPE) LOWORD( dwSkillTmp ));
}

short CChar::Skill_GetAdjusted( SKILL_TYPE skill ) const
{
	// Get the skill adjusted for str,dex,int = 0-1000

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
	const CSkillDef * pSkillDef = g_Cfg.GetSkillDef( skill );
	ASSERT(pSkillDef);
	int iPureBonus =
		( pSkillDef->m_StatBonus[STAT_STR] * m_Stat[STAT_STR] ) +
		( pSkillDef->m_StatBonus[STAT_INT] * m_Stat[STAT_INT] ) +
		( pSkillDef->m_StatBonus[STAT_DEX] * m_Stat[STAT_DEX] );

	int iAdjSkill = IMULDIV( pSkillDef->m_StatPercent, iPureBonus, 10000 );

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

int CChar::Skill_GetMax( SKILL_TYPE skill ) const
{
	// What is my max potential in this skill ?
	if ( m_pPlayer )
	{
		const CSkillClassDef* pSkillClass = m_pPlayer->GetSkillClass();
		ASSERT(pSkillClass);

		if ( skill == SKILL_QTY )
		{
			return pSkillClass->m_SkillSumMax;
		}

		ASSERT( IsSkillBase(skill) );

		int iSkillMax = pSkillClass->m_SkillLevelMax[skill];
		if ( m_pPlayer->Skill_GetLock(skill) >= SKILLLOCK_DOWN )
		{
			int iSkillLevel = Skill_GetBase(skill);
			if ( iSkillLevel < iSkillMax )
				iSkillMax = iSkillLevel;
		}

		return( iSkillMax );
	}
	else
	{
		if ( skill == SKILL_QTY )
		{
			return 500 * SKILL_QTY;
		}
		return 1000;
	}
}

bool CChar::Skill_Degrade( SKILL_TYPE skillused )
{
	// Degrade skills that are over the cap !
	// RETURN:
	//  false = give no credit for ne skil use.
	//  true = give credit. ok

	if ( ! m_pPlayer )
		return( true );

	int iSkillSum = 0;
	int iSkillSumMax = Skill_GetMax( SKILL_QTY );

	int i = 0;
	for ( i=0; i<SKILL_QTY; i++ )
	{
		iSkillSum += Skill_GetBase( (SKILL_TYPE) i );
	}

	// Check for stats degrade first !

	if ( iSkillSum < iSkillSumMax )
		return( true );

	// degrade a random skill
	// NOTE: Take skills over the class cap first ???

	int skillrand = Calc_GetRandVal(SKILL_QTY);
	for ( i=0; true; i++ )
	{
		if ( i >= SKILL_QTY )
		{
			// If we cannot decrease a skill then give no more credit !
			return( false );
		}
		if ( skillrand >= SKILL_QTY )
			skillrand = 0;
		if ( skillrand == skillused )	// never degrade the skill i just used !
			continue;
		if ( m_pPlayer->Skill_GetLock( (SKILL_TYPE) skillrand ) != SKILLLOCK_DOWN )
			continue;
		int iSkillLevel = Skill_GetBase( (SKILL_TYPE) skillrand );
		if ( ! iSkillLevel )
			continue;

		// reduce the skill.
		Skill_SetBase( (SKILL_TYPE) skillrand, iSkillLevel - 1 );
		return( true );
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

	difficulty *= 10;

	int iSkillLevel = Skill_GetBase( skill );
	if ( difficulty < 0 )
	{
		// failure. Give a little experience for failure at low levels.
		if ( iSkillLevel < 300 )
		{
			difficulty = (( min( -difficulty, iSkillLevel )) / 2 ) - 8;
		}
		else
		{
			difficulty = 0;
		}
	}
	if ( difficulty > 1000 )
		difficulty = 1000;

	if ( ! Skill_Degrade(skill))
	{
		// If we cannot decrease a skill then give no more credit !
		difficulty = 0;
	}

	const CSkillDef * pSkillDef = g_Cfg.GetSkillDef(skill);
	ASSERT(pSkillDef);

	int iSkillMax = Skill_GetMax(skill);	// max advance for this skill.

	if ( iSkillLevel < iSkillMax && difficulty ) // Are we in position to gain skill ?
	{
		// ex. ADV_RATE=2000,500,25 for ANATOMY (easy)
		// ex. ADV_RATE=8000,2000,100 for alchemy (hard)
		// assume 100 = a 1 for 1 gain.
		// ex: 8000 = we must use it 80 times to gain .1
		// Higher the number = the less probable to advance.
		// Extrapolate a place in the range.

		// give a bonus or a penalty if the task was too hard or too easy.
		int iSkillAdj = iSkillLevel + ( iSkillLevel - difficulty );

		int iChance = pSkillDef->m_AdvRate.GetChancePercent( iSkillAdj );
		if ( iChance <= 0 )
			return; // less than no chance ?

		int iRoll = Calc_GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Cfg.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "%s=%d.%d Difficult=%d Gain Chance=%d.%d%% Roll=%d%%",
				(LPCTSTR) pSkillDef->GetKey(),
				iSkillLevel/10,(iSkillLevel)%10,
				difficulty/10, iChance/10, iChance%10, iRoll/10 );
		}
#endif

		if ( iRoll <= iChance )
		{
			Skill_SetBase( skill, iSkillLevel + 1 );
		}
	}

	////////////////////////////////////////////////////////
	// Dish out any stat gains - even for failures.

	int iStatSum = 0;

	// Stat effects are unrelated to advance in skill !
	for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
	{
		// Can't gain STR or DEX if morphed.
		if ( IsStatFlag( STATF_Polymorph ) && i != STAT_INT )
			continue;

		int iStatVal = Stat_Get((STAT_TYPE)i);
		if ( iStatVal <= 0 )	// some odd condition
			continue;
		iStatSum += iStatVal;

		int iStatMax = Stat_GetMax((STAT_TYPE)i);
		if ( iStatVal >= iStatMax )
			continue;	// nothing grows past this. (even for NPC's)

		// You will tend toward these stat vals if you use this skill a lot.
		int iStatTarg = pSkillDef->m_Stat[i];
		if ( iStatVal >= iStatTarg )
			continue;		// you've got higher stats than this skill is good for.

		// ??? Building stats should consume food !!

		difficulty = IMULDIV( iStatVal, 1000, iStatTarg );
		int iChance = g_Cfg.m_StatAdv[i].GetChancePercent( difficulty );

		// adjust the chance by the percent of this that the skill uses.
		iChance = ( iChance * pSkillDef->m_StatBonus[i] * pSkillDef->m_StatPercent ) / 10000;

		int iRoll = Calc_GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Cfg.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "%s Difficult=%d Gain Chance=%d.%d%% Roll=%d%%",
				(LPCTSTR) g_Stat_Name[i], difficulty, iChance/10, iChance%10, iRoll/10 );
		}
#endif

		if ( iRoll <= iChance )
		{
			Stat_Set( (STAT_TYPE)i, iStatVal+1 );
			break;
		}
	}

	// Check for stats degrade.
	int iStatSumAvg = Stat_GetMax( STAT_QTY );

	if ( m_pPlayer &&
		iStatSum > iStatSumAvg &&
		! IsStatFlag( STATF_Polymorph ) &&
		! IsPriv( PRIV_GM ))
	{
		// We are at a point where our skills can degrade a bit.
		// In theory magical enhancements make us lazy !

		int iStatSumMax = iStatSumAvg + iStatSumAvg/4;
		if ( iStatSum > iStatSumMax )
		{
			DEBUG_MSG(( "0%x '%s' Exceeds stat max %d" DEBUG_CR, GetUID(), (LPCTSTR) GetName(), iStatSum ));
		}

		int iChanceForLoss = Calc_GetSCurve( iStatSumMax - iStatSum, ( iStatSumMax - iStatSumAvg ) / 4 );
		int iRoll = Calc_GetRandVal(1000);

#ifdef _DEBUG
		if ( IsPriv( PRIV_DETAIL ) &&
			GetPrivLevel() >= PLEVEL_GM &&
			( g_Cfg.m_wDebugFlags & DEBUGF_ADVANCE_STATS ))
		{
			SysMessagef( "Loss Diff=%d Chance=%d.%d%% Roll=%d%%",
				iStatSumMax - iStatSum,
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
				if ( iminval > pSkillDef->m_StatBonus[i] )
				{
					imin = i;
					iminval = pSkillDef->m_StatBonus[i];
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

bool CChar::Skill_CheckSuccess( SKILL_TYPE skill, int difficulty ) const
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

	return( g_Cfg.Calc_SkillCheck( Skill_GetAdjusted(skill), difficulty ));
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
	SetTimeout( m_pPlayer ? -1 : TICK_PER_SEC ); // we should get a brain tick next time.
}

LPCTSTR CChar::Skill_GetName( bool fUse ) const
{
	// Name the current skill we are doing.

	SKILL_TYPE skill = Skill_GetActive();
	if ( skill <= SKILL_NONE )
	{
		return( "Idling" );
	}
	if ( IsSkillBase(skill))
	{
		if ( ! fUse )
		{
			return( g_Cfg.GetSkillKey(skill));
		}

		TCHAR * pszText = Str_GetTemp();
		sprintf( pszText, _TEXT("use %s"), g_Cfg.GetSkillKey(skill));
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
	case NPCACT_LOOKING: return( "Looking" );
	case NPCACT_TRAINING: return( "Training" );
	case NPCACT_Napping: return( "Napping" );
	case NPCACT_UNConscious: return( "Unconscious" );
	case NPCACT_Sleeping: return( "Sleeping" );
	case NPCACT_Healing: return( "Healing" );
	case NPCACT_ScriptBook: return( "Marching" );
	}

	return( "Thinking" );
}

void CChar::Skill_SetTimeout()
{
	SKILL_TYPE skill = Skill_GetActive();
	ASSERT( IsSkillBase(skill));
	int iSkillLevel = Skill_GetBase(skill);
	int iDelay = g_Cfg.GetSkillDef(skill)->m_Delay.GetLinear( iSkillLevel );
	SetTimeout(iDelay);
}

bool CChar::Skill_MakeItem_Success()
{
	// deliver the goods.

	CItemVendable * pItem = dynamic_cast <CItemVendable *> (CItem::CreateTemplate( m_atCreate.m_ItemID ));
	if ( pItem == NULL )
		return( false );

	int iSkillLevel = Skill_GetBase( Skill_GetActive());	// primary skill value.

	if ( m_atCreate.m_Amount != 1 )
	{
		// Some item with the REPLICATE flag ?
		pItem->SetAmount( m_atCreate.m_Amount ); // Set the quantity if we are making bolts, arrows or shafts
	}
	else if ( pItem->IsType( IT_SCROLL ))
	{
		// scrolls have the skill level of the inscriber ?
		pItem->m_itSpell.m_spelllevel = iSkillLevel;
	}
	else if ( pItem->IsType( IT_POTION ))
	{
		// Create the potion, set various properties,
		// put in pack
		Emote( "pour the completed potion into a bottle" );
		Sound( 0x240 );	// pouring noise.
	}
	else
	{
		// Only set the quality on single items.
		int quality = IMULDIV( iSkillLevel, 2, 10 );	// default value for quality.
		// Quality depends on the skill of the craftsman, and a random chance.
		// minimum quality is 1, maximum quality is 200.  100 is average.
		// How much variance?  This is the difference in quality levels from
		// what I can normally make.
		int variance = 2 - (int) log10( Calc_GetRandVal( 250 ) + 1); // this should result in a value between 0 and 2.
		// Determine if lower or higher quality
		if ( Calc_GetRandVal( 2 ))
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

		CGString sMakeMsg;
		switch ( qualityBase )
		{
		case 0:
			// Shoddy quality
			sMakeMsg.Format("Due to your poor skill, the item is of shoddy quality");
			quality = Calc_GetRandVal( 25 ) + 1;
			break;
		case 1:
			// Poor quality
			sMakeMsg.Format("You were barely able to make this item.  It is of poor quality");
			quality = Calc_GetRandVal( 25 ) + 26;
			break;
		case 2:
			// Below average quality
			sMakeMsg.Format("You make the item, but it is of below average quality");
			quality = Calc_GetRandVal( 25 ) + 51;
			break;
		case 3:
			// Average quality
			quality = Calc_GetRandVal( 50 ) + 76;
			break;
		case 4:
			// Above average quality
			sMakeMsg.Format("The item is of above average quality");
			quality = Calc_GetRandVal( 25 ) + 126;
			break;
		case 5:
			// Excellent quality
			sMakeMsg.Format("The item is of excellent quality");
			quality = Calc_GetRandVal( 25 ) + 151;
			break;
		case 6:
			// Superior quality
			sMakeMsg.Format("Due to your exceptional skill, the item is of superior quality");
			quality = Calc_GetRandVal( 25 ) + 176;
			break;
		default:
			// How'd we get here?
			quality = 1000;
			break;
		}
		pItem->SetQuality(quality);
		if ( iSkillLevel > 999 && ( quality > 175 ))
		{
			// A GM made this, and it is of high quality
			CGString csNewName;
			csNewName.Format( "%s crafted by %s", (LPCTSTR) pItem->GetName(), (LPCTSTR) GetName());
			pItem->SetName(csNewName);
		}
		if ( ! sMakeMsg.IsEmpty())
			SysMessage( sMakeMsg );
	}

	pItem->SetAttr(ATTR_MOVE_ALWAYS | ATTR_CAN_DECAY);	// Any made item is movable.
	ItemBounce(pItem);
	return( true );
}

bool CChar::Skill_MakeItem( ITEMID_TYPE id, CGrayUID uidTarg, SKTRIG_TYPE stage )
{
	// "MAKEITEM"
	//
	// SKILL_ALCHEMY
	// SKILL_BLACKSMITHING
	// SKILL_BOWCRAFT
	// SKILL_CARPENTRY
	// SKILL_INSCRIPTION
	// SKILL_TAILORING:
	// SKILL_TINKERING,
	//
	// Confer the new item.
	// Test for consumable items.
	// Fail = do a partial consume of the resources.
	//
	// ARGS:
	//  uidTarg = item targetted to try to make this . (this item should be used to make somehow)
	// Skill_GetActive()
	//
	// RETURN:
	//   true = success.
	//

	if ( id <= 0 )
		return( true );

	CItemBase * pItemDef = CItemBase::FindItemBase( id );
	if ( pItemDef == NULL )
		return( false );

	CItem * pItemTarg = uidTarg.ItemFind();
	if ( pItemTarg && stage == SKTRIG_SELECT )
	{
		if ( pItemDef->m_SkillMake.FindResourceMatch( pItemTarg ) < 0 &&
			pItemDef->m_BaseResources.FindResourceMatch( pItemTarg ) < 0 )
		{
			// Not intersect with the specified item
			return( false );
		}
	}

	int iReplicationQty = 1;
	if ( pItemDef->Can( CAN_I_REPLICATE ))
	{
		// For arrows/bolts, how many do they want ?
		// Set the quantity that they want to make.
		if ( pItemTarg != NULL )
		{
			iReplicationQty = pItemTarg->GetAmount();
		}
	}

	// Test the hypothetical required skills and tools
	if ( ! pItemDef->m_SkillMake.IsResourceMatchAll( this ))
	{
		return( false );
	}

	// test or consume the needed resources.
	if ( stage == SKTRIG_FAIL )
	{
		// If fail only consume part of them.
		ResourceConsumePart( &(pItemDef->m_BaseResources), iReplicationQty, Calc_GetRandVal( 50 ));
		return( false );
	}

	iReplicationQty = ResourceConsume( &(pItemDef->m_BaseResources), iReplicationQty, stage != SKTRIG_SUCCESS );
	if ( ! iReplicationQty )
	{
		return( false );
	}

	if ( stage == SKTRIG_START )
	{
		// Start the skill.
		// Find the primary skill required.

		int i = pItemDef->m_SkillMake.FindResourceType( RES_SKILL );
		if ( i < 0 )
			return( false );

		CResourceQty RetMainSkill = pItemDef->m_SkillMake[i];
	
		m_Act_Targ = uidTarg;	// targetted item to start the make process.
		m_atCreate.m_ItemID = id;
		m_atCreate.m_Amount = iReplicationQty;

		return Skill_Start( (SKILL_TYPE) RetMainSkill.GetResIndex(), RetMainSkill.GetResQty() / 10 );
	}

	if ( stage == SKTRIG_SUCCESS )
	{
		return( Skill_MakeItem_Success() );
	}

	return( true );
}

int CChar::Skill_NaturalResource_Setup( CItem * pResBit )
{
	// RETURN: skill difficulty 
	//  0-100
	ASSERT(pResBit);

	// Find the ore type located here based on color.
	const CRegionResourceDef * pOreDef = dynamic_cast <const CRegionResourceDef *>( g_Cfg.ResourceGetDef( pResBit->m_itResource.m_rid_res ));
	if ( pOreDef == NULL )
	{
		return( -1 );
	}

	return( pOreDef->m_Skill.GetRandom() / 10 );
}

CItem * CChar::Skill_NaturalResource_Create( CItem * pResBit, SKILL_TYPE skill )
{
	// Create some natural resource item.
	// skill = Effects qty of items returned.
	// SKILL_MINING
	// SKILL_FISHING
	// SKILL_LUMBERJACKING

	ASSERT(pResBit);

	// Find the ore type located here based on color.
	const CRegionResourceDef * pOreDef = dynamic_cast <const CRegionResourceDef *>( g_Cfg.ResourceGetDef( pResBit->m_itResource.m_rid_res ));
	if ( pOreDef == NULL )
	{
		return( NULL );
	}

	// Skill effects how much of the ore i can get all at once.

	ITEMID_TYPE id = (ITEMID_TYPE) RES_GET_INDEX( pOreDef->m_ReapItem );
	if ( id == ITEMID_NOTHING )
	{
		// I intended for there to be nothing here.
		return( NULL );
	}

	// Reap amount is semi-random

	int iAmount = pOreDef->m_ReapAmount.GetRandomLinear( Skill_GetBase(skill));
	if ( ! iAmount)
	{
		iAmount = 1;
	}

	CItem * pItem = CItem::CreateScript( id );
	ASSERT(pItem);

	iAmount = pResBit->ConsumeAmount( iAmount );	// amount i used up.
	if ( iAmount <= 0 )
	{
		pItem->Delete();
		return( NULL );
	}

	pItem->SetAmount( iAmount );
	return( pItem );
}

bool CChar::Skill_Mining_Smelt( CItem * pItemOre, CItem * pItemTarg )
{
	// SKILL_MINING
	// pItemTarg = forge or another pile of ore.
	// RETURN: true = success.
	if ( pItemOre == NULL || pItemOre == pItemTarg )
	{
		SysMessage( "You need to target the ore you want to smelt" );
		return( false );
	}

	// The ore is on the ground
	if ( ! CanUse( pItemOre, true ))
	{
		SysMessagef( "You can't use the %s where it is.", (LPCTSTR) pItemOre->GetName() );
		return( false );
	}

	if ( pItemOre->IsType( IT_ORE ) &&
		pItemTarg != NULL &&
		pItemTarg->IsType( IT_ORE ))
	{
		// combine piles.
		if ( pItemTarg == pItemOre )
			return( false );
		if ( pItemTarg->GetID() != pItemOre->GetID())
			return( false );
		pItemTarg->SetAmountUpdate( pItemOre->GetAmount() + pItemTarg->GetAmount());
		pItemOre->Delete();
		return( true );
	}

#define SKILL_SMELT_FORGE_DIST 3

	if ( pItemTarg != NULL && pItemTarg->IsTopLevel() &&
		pItemTarg->IsType( IT_FORGE ))
	{
		m_Act_p = pItemTarg->GetTopPoint();
	}
	else
	{
		m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), IT_FORGE, SKILL_SMELT_FORGE_DIST );
	}

	if ( ! m_Act_p.IsValidPoint() || ! CanTouch(m_Act_p))
	{
		SysMessage( "You must be near a forge to smelt" );
		return( false );
	}

	const CItemBase * pOreDef = pItemOre->Item_GetDef();
	if ( pOreDef->IsType( IT_INGOT ))
	{
		SysMessage( "Use a smith hammer to make items from ingots." );
		return false;
	}

	// Fire effect ?
	CItem * pItemEffect = CItem::CreateBase(ITEMID_FIRE);
	ASSERT(pItemEffect);
	CPointMap pt = m_Act_p;
	pt.m_z += 8;	// on top of the forge.
	pItemEffect->SetAttr( ATTR_MOVE_NEVER );
	pItemEffect->MoveToDecay( pt, TICK_PER_SEC );
	Sound( 0x2b );

	UpdateDir( m_Act_p );
	if ( pItemOre->IsAttr(ATTR_MAGIC|ATTR_BLESSED|ATTR_BLESSED2|ATTR_MOVE_ALWAYS))	// not magic items
	{
		SysMessage( "The fire is not hot enough to melt this." );
		return false;
	}

	CGString sEmote;
	sEmote.Format( "smelt %s", (LPCTSTR) pItemOre->GetName());
	Emote(sEmote);

	int iMiningSkill = Skill_GetAdjusted(SKILL_MINING);
	int iOreQty = pItemOre->GetAmount();
	const CItemBase * pIngotDef = NULL;
	int iIngotQty = 0;

	if ( pOreDef->IsType( IT_ORE ))
	{
		ITEMID_TYPE idIngot = (ITEMID_TYPE) RES_GET_INDEX( pOreDef->m_ttOre.m_IngotID );
		pIngotDef = CItemBase::FindItemBase(idIngot);
		iIngotQty = 1;	// ingots per ore.
	}
	else
	{
		// Smelting something like armor etc.
		// find the ingot type resources.
		for ( int i=0; i<pOreDef->m_BaseResources.GetCount(); i++ )
		{
			RESOURCE_ID rid = pOreDef->m_BaseResources[i].GetResourceID();
			if ( rid.GetResType() != RES_ITEMDEF )
				continue;

			const CItemBase * pBaseDef = CItemBase::FindItemBase( (ITEMID_TYPE) rid.GetResIndex());
			if ( pBaseDef == NULL )
				continue;

			if ( pBaseDef->IsType( IT_GEM ))
			{
				// bounce the gems out of this.
				CItem * pGem = CItem::CreateScript( pBaseDef->GetID() );
				ASSERT(pGem);
				pGem->SetAmount( iOreQty * pBaseDef->m_BaseResources[i].GetResQty() );
				ItemBounce( pGem );
				continue;
			}
			if ( pBaseDef->IsType( IT_INGOT ))
			{
				if ( iMiningSkill < pBaseDef->m_ttIngot.m_iSkillMin )
				{
					SysMessagef("You lack the skill to smelt %s", (LPCTSTR) pBaseDef->GetName());
					continue;
				}
				pIngotDef = pBaseDef;
				iIngotQty = pOreDef->m_BaseResources[i].GetResQty();
			}
		}
	}

	if ( pIngotDef == NULL ||
		! pIngotDef->IsType(IT_INGOT))
	{
		SysMessage( "It is consumed in the fire." );
		pItemOre->ConsumeAmount( iOreQty );
		return true;
	}

	iIngotQty *= iOreQty;	// max amount

	int iSkillRange = pIngotDef->m_ttIngot.m_iSkillMax - pIngotDef->m_ttIngot.m_iSkillMin;
	int iDifficulty = Calc_GetRandVal(iSkillRange);
	//iIngotQty = IMULDIV( iIngotQty, iDifficulty, iSkillRange );
	//if ( iIngotQty <= 0 )
	//	iIngotQty = 1;

	// try to make ingots.
	iDifficulty = ( pIngotDef->m_ttIngot.m_iSkillMin + iDifficulty ) / 10;
	if ( ! iIngotQty || ! Skill_UseQuick( SKILL_MINING, iDifficulty ))
	{
		SysMessagef( "You smelt the %s but are left with nothing useful.", (LPCTSTR) pItemOre->GetName() );
		// Lose up to half the resources.
		pItemOre->ConsumeAmount( Calc_GetRandVal( pItemOre->GetAmount() / 2 ) + 1 );
		return( false );
	}

	// Payoff - What do i get ?
	// This is the one
	CItem * pIngots = CItem::CreateScript( pIngotDef->GetID() );
	if ( pIngots == NULL )
	{
		// Either this is really iron, or there isn't an ingot defined for this guy
		SysMessage( "You smelt the ore but are left with nothing useful." );
		return( false );
	}

	// give some random loss factor.
	pIngots->SetAmount( iIngotQty );
	pItemOre->ConsumeAmount( pItemOre->GetAmount());
	ItemBounce( pIngots );
	return true;
}

bool CChar::Skill_Tracking( CGrayUID uidTarg, DIR_TYPE & dirPrv, int iDistMax )
{
	// SKILL_TRACKING
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

	int dist = GetTopDist3D( pObjTop );	// disconnect = SHRT_MAX
	if ( dist > iDistMax )
	{
		return false;
	}

	DIR_TYPE dir = GetDir( pObjTop );
	if (( dirPrv != dir ) || ! Calc_GetRandVal(10))
	{
		dirPrv = dir;
		CGString sMsgDist;
		if ( dist )
		{
			LPCTSTR pszDist;
			if ( dist < 16 ) // Closing in message?
				pszDist = " near";
			else if ( dist < 32 )
				pszDist = "";
			else if ( dist < 100 )
				pszDist = " far";
			else
				pszDist = " very far";
			sMsgDist.Format( _TEXT( "%s to the %s" ), pszDist, (LPCTSTR) CPointBase::sm_szDirs[ dir ] );
		}
		else
		{
			sMsgDist = _TEXT(" here");
		}

		CGString sMsg;
		sMsg.Format( "%s is%s%s", (LPCTSTR) pObj->GetName(),
			pObjTop->IsDisconnected() ? " disconnected" : "",
			(LPCTSTR) sMsgDist );
		ObjMessage( sMsg, this );
	}

	return true;		// keep the skill active.
}

//************************************
// Skill handlers.

int CChar::Skill_Tracking( SKTRIG_TYPE stage )
{
	// SKILL_TRACKING
	// m_Act_Targ = what am i tracking ?
	// m_atTracking.m_PrvDir = the previous dir it was in.
	//

	if ( stage == SKTRIG_START )
	{
		// Already checked difficulty earlier.
		return( 0 );
	}
	if ( stage == SKTRIG_FAIL )
	{
		// This skill didn't fail, it just ended/went out of range, etc...
		ObjMessage( "You have lost your quary.", this ); // say this instead of the failure message
		return( -SKTRIG_ABORT );
	}
	if ( stage == SKTRIG_STROKE )
	{
		int iSkillLevel = Skill_GetAdjusted(SKILL_TRACKING);
		if ( ! Skill_Tracking( m_Act_Targ, m_atTracking.m_PrvDir, iSkillLevel/20 + 10 ))
			return( -SKTRIG_ABORT );
		Skill_SetTimeout();		// next update.
		return( -SKTRIG_STROKE );	// keep it active.
	}

	return( -SKTRIG_ABORT );
}

int CChar::Skill_Alchemy( SKTRIG_TYPE stage )
{
	// SKILL_ALCHEMY
	// m_atCreate.m_ItemID = potion we are making.
	// We consume resources on each stroke.
	// This was start in Skill_MakeItem()

	const CItemBase * pPotionDef = CItemBase::FindItemBase( m_atCreate.m_ItemID );
	if ( pPotionDef == NULL )
	{
		SysMessage( "You have no clue how to make this potion." );
		return -SKTRIG_ABORT;
	}

	if ( stage == SKTRIG_START )
	{
		// See if skill allows a potion made out of targ'd reagent		// Sound( 0x243 );
		m_atCreate.m_Stroke_Count = 0; // counts up.
		return( m_Act_Difficulty );
	}
	if ( stage == SKTRIG_FAIL )
	{
		// resources have already been consumed.
		Emote( "toss the failed mixture from the mortar" );
		return( 0 );	// normal failure 
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		// Resources have already been consumed.
		// Now deliver the goods.
		Skill_MakeItem_Success();
		return( 0 );
	}

	ASSERT( stage == SKTRIG_STROKE );
	if ( stage != SKTRIG_STROKE )
		return( -SKTRIG_QTY );

	if ( m_atCreate.m_Stroke_Count >= pPotionDef->m_BaseResources.GetCount())
	{
		// done.
		return 0;
	}

	// Keep trying and grinding
	//  OK, we know potion being attempted and the bottle
	//  it's going in....do a loop for each reagent

	CResourceQty item = pPotionDef->m_BaseResources[m_atCreate.m_Stroke_Count];
	RESOURCE_ID rid = item.GetResourceID();

	CItemBase * pReagDef = dynamic_cast <CItemBase*>( g_Cfg.ResourceGetDef( rid ));
	if ( pReagDef == NULL )
	{
		return -SKTRIG_ABORT;
	}

	if ( pReagDef->IsType(IT_POTION_EMPTY) && m_Act_Difficulty < 0 ) // going to fail anyhow.
	{
		// NOTE: Assume the bottle is ALWAYS LAST !
		// Don't consume the bottle.
		return -SKTRIG_ABORT;
	}

	if ( ContentConsume( rid, item.GetResQty()))
	{
		SysMessagef( "Hmmm, you lack %s for this potion.", (LPCTSTR) pReagDef->GetName() );
		return -SKTRIG_ABORT;
	}

	if ( GetTopSector()->GetCharComplexity() < 5 && pReagDef->IsType(IT_REAGENT))
	{
		CGString sSpeak;
		sSpeak.Format(( m_atCreate.m_Stroke_Count == 0 ) ?
			"start grinding some %s in the mortar" :
			"add %s and continue grinding", (LPCTSTR) pReagDef->GetName());
		Emote( sSpeak );
	}

	Sound( 0x242 );
	m_atCreate.m_Stroke_Count ++;
	Skill_SetTimeout();
	return -SKTRIG_STROKE;	// keep active.
}

int CChar::Skill_Mining( SKTRIG_TYPE stage )
{
	// SKILL_MINING
	// m_Act_p = the point we want to mine at.
	// m_Act_TargPrv = Shovel
	//
	// Test the chance of precious ore.
	// resource check  to IT_ORE. How much can we get ?
	// RETURN:
	//  Difficulty 0-100

	if ( m_Act_p.m_x == 0xFFFF )
	{
		SysMessage( "Try mining in rock!" );
		return( -SKTRIG_QTY );
	}

	// Verify so we have a line of sight.
	if ( ! CanSeeLOS( m_Act_p, NULL, 2 ))
	{
		if ( GetTopPoint().GetDist( m_Act_p ) > 2 )
		{
			SysMessage("That is too far away." );
		}
		else
		{
			SysMessage("You have no line of sight to that location");
		}
		return( -SKTRIG_QTY );
	}

	// resource check
	CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, IT_ROCK, stage == SKTRIG_START );
	if ( pResBit == NULL )
	{
		SysMessage( "Try mining in rock." );
		return( -SKTRIG_QTY );
	}
	if ( pResBit->GetAmount() == 0 )
	{
		SysMessage( "There is no ore here to mine." );
		return( -SKTRIG_QTY );
	}

	CItem * pShovel = m_Act_TargPrv.ItemFind();
	if ( pShovel == NULL )
	{
		SysMessage( "You must use a shovel or pick." );
		return( -SKTRIG_ABORT );
	}

	if ( stage == SKTRIG_FAIL )
		return 0;

	if ( stage == SKTRIG_START )
	{
		m_atResource.m_Stroke_Count = Calc_GetRandVal( 5 ) + 2;

		pShovel->OnTakeDamage( 1, this, DAMAGE_HIT_BLUNT );

		return( Skill_NaturalResource_Setup( pResBit ));
	}

	if ( stage == SKTRIG_STROKE )
	{
		// Pick a "mining" type sound
		Sound( ( Calc_GetRandVal(2)) ? 0x125 : 0x126 );
		UpdateDir( m_Act_p );

		if ( m_atResource.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atResource.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_1H_DOWN );
			Skill_SetTimeout();
			return( -SKTRIG_STROKE );	// keep active.
		}

		return( 0 );
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	CItem * pItem = Skill_NaturalResource_Create( pResBit, SKILL_MINING );
	if ( pItem == NULL )
	{
		SysMessage( "There is no ore here to mine." );
		return( -SKTRIG_FAIL );
	}

	ItemBounce( pItem );
	return( 0 );
}

int CChar::Skill_Fishing( SKTRIG_TYPE stage )
{
	// m_Act_p = where to fish.
	// NOTE: don't check LOS else you can't fish off boats.
	// Check that we dont stand too far away
	// Make sure we aren't in a house
	// RETURN:
	//   difficulty = 0-100

	CRegionBase * pRegion = GetTopPoint().GetRegion( REGION_TYPE_MULTI );
	if ( pRegion && ! pRegion->IsFlag( REGION_FLAG_SHIP ) )
	{
		// We are in a house ?
		SysMessage("You can't fish from where you are standing.");
		return( -SKTRIG_QTY );
	}

	if ( GetTopPoint().GetDist( m_Act_p ) > 6 )	// cast works for long distances.
	{
		SysMessage("That is too far away." );
		return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}

	// resource check
	CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, IT_WATER, stage == SKTRIG_START );
	if ( pResBit == NULL )
	{
		SysMessage( "Try fishing in water." );
		return( -SKTRIG_QTY );
	}
	if ( pResBit->GetAmount() == 0 )
	{
		SysMessage( "There are no fish here." );
		return( -SKTRIG_QTY );
	}

	Sound( 0x027 );

	// Create the little splash effect.
	CItem * pItemFX = CItem::CreateBase( ITEMID_FX_SPLASH );
	ASSERT(pItemFX);
	pItemFX->SetType(IT_WATER_WASH);	// can't fish here.

	if ( stage == SKTRIG_START )
	{
		pItemFX->MoveToDecay( m_Act_p, 1*TICK_PER_SEC );

		UpdateAnimate( ANIM_ATTACK_2H_DOWN );
		return( Skill_NaturalResource_Setup( pResBit ));
	}

	if ( stage == SKTRIG_SUCCESS )
	{
		pItemFX->MoveToDecay( m_Act_p, 3*TICK_PER_SEC );

		CItem * pFish = Skill_NaturalResource_Create( pResBit, SKILL_FISHING );
		if ( pFish == NULL )
		{
			return( -SKTRIG_ABORT );
		}

		SysMessagef( "You pull out a %s!", (LPCTSTR) pFish->GetName() );
		pFish->MoveToCheck( GetTopPoint(), this );	// put at my feet.
		return( 0 );
	}

	ASSERT(0);
	return( -SKTRIG_QTY);
}

int CChar::Skill_Lumberjack( SKTRIG_TYPE stage )
{
	// RETURN:
	//   difficulty = 0-100

	if ( m_Act_p.m_x == 0xFFFF )
	{
		SysMessage( "Try chopping a tree." );
		return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}

	// 3D distance check and LOS
	if ( ! CanTouch(m_Act_p) || GetTopPoint().GetDist3D( m_Act_p ) > 3 )
	{
		if ( GetTopPoint().GetDist( m_Act_p ) > 3 )
		{
			SysMessage("That is too far away." );
		}
		else
		{
			SysMessage("You have no line of sight to that location");
		}
		return( -SKTRIG_QTY );
	}

	// resource check
	CItem * pResBit = g_World.CheckNaturalResource( m_Act_p, IT_TREE, stage == SKTRIG_START );
	if ( pResBit == NULL )
	{
		SysMessage( "Try chopping a tree." );
		return( -SKTRIG_QTY );
	}
	if ( pResBit->GetAmount() == 0 )
	{
		SysMessage( "There are no logs here to chop." );
		return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_START )
	{
		m_atResource.m_Stroke_Count = Calc_GetRandVal( 5 ) + 2;
		return( Skill_NaturalResource_Setup( pResBit ));
	}

	if ( stage == SKTRIG_STROKE )
	{
		Sound( 0x13e );	// 0135, 013e, 148, 14a
		UpdateDir( m_Act_p );
		if ( m_atResource.m_Stroke_Count )
		{
			// Keep trying and updating the animation
			m_atResource.m_Stroke_Count --;
			UpdateAnimate( ANIM_ATTACK_WEAPON );
			Skill_SetTimeout();
			return( -SKTRIG_STROKE );	// keep active.
		}
		return 0;
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	// resource check

	CItem * pItem = Skill_NaturalResource_Create( pResBit, SKILL_LUMBERJACKING );
	if ( pItem == NULL )
		return( -SKTRIG_FAIL );

	ItemBounce( pItem );
	return( 0 );
}

int CChar::Skill_DetectHidden( SKTRIG_TYPE stage )
{
	// SKILL_DETECTINGHIDDEN
	// Look around for who is hiding.
	// Detect them based on skill diff.
	// ??? Hidden objects ?

	if ( stage == SKTRIG_START )
	{
		// Based on who is hiding ?
		return( 10 );
	}
	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage != SKTRIG_SUCCESS )
	{
		ASSERT(0);
		return( -SKTRIG_QTY );
	}

	int iRadius = ( Skill_GetAdjusted(SKILL_DETECTINGHIDDEN) / 8 ) + 1;
	CWorldSearch Area( GetTopPoint(), iRadius );
	bool fFound = false;
	while (true)
	{
		CChar * pChar = Area.GetChar();
		if ( pChar == NULL )
			break;
		if ( pChar == this )
			continue;
		if ( ! pChar->IsStatFlag( STATF_Invisible | STATF_Hidden ))
			continue;
		// Try to detect them.
		if ( pChar->IsStatFlag( STATF_Hidden ))
		{
			// If there hiding skill is much better than our detect then stay hidden
		}
		pChar->Reveal();
		SysMessagef( "You find %s", (LPCTSTR) pChar->GetName());
		fFound = true;
	}

	if ( ! fFound )
	{
		return( -SKTRIG_FAIL );
	}

	return( 0 );
}

int CChar::Skill_Cartography( SKTRIG_TYPE stage )
{
	// Selected a map type and now we are making it.
	// m_Act_Cartography_Dist = the map distance.
	// Find the blank map to write on first.

	if ( stage == SKTRIG_STROKE )
		return 0;

	CPointMap pnt = GetTopPoint();
	if ( pnt.m_x >= UO_SIZE_X_REAL )	// maps don't work out here !
	{
		SysMessage( "You can't seem to figure out your surroundings." );
		return( -SKTRIG_QTY );
	}

	CItem * pItem = ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_MAP_BLANK), 0 );
	if ( pItem == NULL )
	{
		SysMessage( "You have no blank parchment to draw on" );
		return( -SKTRIG_QTY );
	}

	if ( ! CanUse( pItem, true ))
	{
		SysMessagef( "You can't use the %s where it is.", (LPCTSTR) pItem->GetName() );
		return( false );
	}

	m_Act_Targ = pItem->GetUID();

	if ( stage == SKTRIG_START )
	{
		Sound( 0x249 );

		// difficulty related to m_atCartography.m_Dist ???

		return( Calc_GetRandVal(100) );
	}
	if ( stage == SKTRIG_FAIL )
	{
		// consume the map sometimes ?
		// pItem->ConsumeAmount( 1 );
		return 0;
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		pItem->ConsumeAmount( 1 );

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
		return( 0 );
	}

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Musicianship( SKTRIG_TYPE stage )
{
	// m_Act_Targ = the intrument i targetted to play.

	if ( stage == SKTRIG_STROKE )
		return 0;
	if ( stage == SKTRIG_START )
	{
		// no instrument fail immediate
		return Use_PlayMusic( m_Act_Targ.ItemFind(), Calc_GetRandVal(90));;
	}

	return( 0 );
}

int CChar::Skill_Peacemaking( SKTRIG_TYPE stage )
{
	// try to make all those listening peacable.
	// General area effect.
	// make peace if possible. depends on who is listening/fighting.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_START )
	{
		// Find musical inst first.

		// Basic skill check.
		int iDifficulty = Use_PlayMusic( NULL, Calc_GetRandVal(40));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( -SKTRIG_FAIL );

		if ( ! iDifficulty )
		{
			iDifficulty = Calc_GetRandVal(40);	// Depend on evil of the creatures here.
		}

		// Who is fighting around us ? determines difficulty.
		return( iDifficulty );
	}

	if ( stage == SKTRIG_FAIL || stage == SKTRIG_SUCCESS )
	{
		// Failure just irritates.

		int iRadius = ( Skill_GetAdjusted(SKILL_PEACEMAKING) / 8 ) + 1;
		CWorldSearch Area( GetTopPoint(), iRadius );
		while (true)
		{
			CChar * pChar = Area.GetChar();
			if ( pChar == NULL )
				return( -SKTRIG_FAIL );
			if ( pChar == this )
				continue;
			break;
		}
		return 0;
	}

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Enticement( SKTRIG_TYPE stage )
{
	// m_Act_Targ = my target
	// Just keep playing and trying to allure them til we can't
	// Must have a musical instrument.

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		return( -SKTRIG_QTY );
	}
	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		// Walk to me ?
		return 0;
	}
	if ( stage == SKTRIG_START )
	{
		// Base music diff, (whole thing won't work if this fails)
		int iDifficulty = Use_PlayMusic( NULL, Calc_GetRandVal(55));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( -SKTRIG_QTY );

		m_atMusician.m_iMusicDifficulty = iDifficulty;

		// Based on the STAT_INT and will of the target.
		if ( ! iDifficulty )
		{
			return pChar->Stat_Get(STAT_INT);
		}

		return( iDifficulty );
	}

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Provocation( SKTRIG_TYPE stage )
{
	// m_Act_TargPrv = provoke this person
	// m_Act_Targ = against this person.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CChar * pCharProv = m_Act_TargPrv.CharFind();
	CChar * pCharTarg = m_Act_Targ.CharFind();

	// If no provoker, then we fail (naturally!)
	if ( pCharProv == NULL || pCharProv == this )
	{
		SysMessage( "You are really upset about this" );
		return -SKTRIG_QTY;
	}

	if ( stage == SKTRIG_FAIL )
	{
		// Might just attack you !
		pCharProv->Fight_Attack( this );
		return( 0 );
	}

	if ( stage == SKTRIG_START )
	{
		int iDifficulty = Use_PlayMusic( NULL, Calc_GetRandVal(40));
		if ( iDifficulty < -1 )	// no instrument fail immediate
			return( false );
		if ( ! iDifficulty )
		{
			iDifficulty = pCharProv->Stat_Get(STAT_INT);	// Depend on evil of the creature.
		}

		return( iDifficulty );
	}

	if ( stage != SKTRIG_SUCCESS )
	{
		ASSERT(0);
		return -SKTRIG_QTY;
	}

	if ( pCharProv->IsClient() )
	{
		CheckCrimeSeen( SKILL_NONE, NULL, pCharProv, "provoking" );
		return -SKTRIG_FAIL;
	}

	// If out of range or something, then we might get attacked ourselves.
	if ( pCharProv->Stat_Get(STAT_Karma) >= 10000 )
	{
		// They are just too good for this.
		pCharProv->Emote( "looks peaceful" );
		return -SKTRIG_ABORT;
	}

	pCharProv->Emote( "looks furious" );

	// If no target then skill fails
	if ( pCharTarg == NULL )
	{
		return -SKTRIG_FAIL;
	}

	// He realizes that you are the real bad guy as well.
	if ( ! pCharTarg->OnAttackedBy( this, 1, true ))
	{
		return -SKTRIG_ABORT;
	}

	pCharProv->Memory_AddObjTypes( this, MEMORY_AGGREIVED|MEMORY_IRRITATEDBY );

	// If out of range we might get attacked ourself.
	if ( pCharProv->GetTopDist3D( pCharTarg ) > UO_MAP_VIEW_SIGHT ||
		pCharProv->GetTopDist3D( this ) > UO_MAP_VIEW_SIGHT )
	{
		// Check that only "evil" monsters attack provoker back
		if ( pCharProv->Noto_IsEvil())
		{
			pCharProv->Fight_Attack( this );
		}
		return -SKTRIG_ABORT;
	}

	// If we are provoking against a "good" PC/NPC and the provoked
	// NPC/PC is good, we are flagged criminal for it and guards
	// are called.
	if ( pCharProv->Noto_GetFlag(this)==NOTO_GOOD )
	{
		// lose some karma for this.
		CheckCrimeSeen( SKILL_NONE, NULL, pCharProv, "provoking" );
		return -SKTRIG_ABORT;
	}

	// If we provoke upon a good char we should go criminal for it
	// but skill still succeed.
	if ( pCharTarg->Noto_GetFlag(this)==NOTO_GOOD )
	{
		CheckCrimeSeen( SKILL_NONE, NULL, pCharTarg, "provoking" );
	}

	pCharProv->Fight_Attack( pCharTarg ); // Make the actual provoking.
	return( 0 );
}

int CChar::Skill_Poisoning( SKTRIG_TYPE stage )
{
	// Act_TargPrv = poison this weapon/food
	// Act_Targ = with this poison.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CItem * pPoison = m_Act_Targ.ItemFind();
	if ( pPoison == NULL ||
		! pPoison->IsType(IT_POTION))
	{
		return( -SKTRIG_ABORT );
	}

	if ( stage == SKTRIG_START )
	{
		return Calc_GetRandVal( 60 );
	}
	if ( stage == SKTRIG_FAIL )
	{
		// Lose the poison sometimes ?
		return( 0 );
	}

	if ( RES_GET_INDEX(pPoison->m_itPotion.m_Type) != SPELL_Poison )
	{
		return( -SKTRIG_ABORT );
	}

	CItem * pItem = m_Act_TargPrv.ItemFind();
	if ( pItem == NULL )
	{
		return( -SKTRIG_QTY );
	}

	if ( stage != SKTRIG_SUCCESS )
	{
		ASSERT(0);
		return( -SKTRIG_ABORT );
	}

	Sound( 0x247 );	// powdering.

	switch ( pItem->GetType() )
	{
	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		pItem->m_itFood.m_poison_skill = pPoison->m_itPotion.m_skillquality / 10;
		break;
	case IT_WEAPON_MACE_SHARP:
	case IT_WEAPON_SWORD:		// 13 =
	case IT_WEAPON_FENCE:		// 14 = can't be used to chop trees. (make kindling)
		pItem->m_itWeapon.m_poison_skill = pPoison->m_itPotion.m_skillquality / 10;
		break;
	default:
		SysMessage( "You can only poison food or piercing weapons." );
		return( -SKTRIG_QTY );
	}
	// skill + quality of the poison.
	SysMessage( "You apply the poison." );
	pPoison->ConsumeAmount();
	return( 0 );
}

int CChar::Skill_Cooking( SKTRIG_TYPE stage )
{
	// SKILL_COOKING
	// m_Act_Targ = food object to cook.
	// m_Act_p = my fire.
	// How hard to cook is this ?

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CItem * pFoodRaw = m_Act_Targ.ItemFind();
	if ( pFoodRaw == NULL )
	{
		return( -SKTRIG_QTY );
	}
	if ( ! pFoodRaw->IsType( IT_FOOD_RAW ) && ! pFoodRaw->IsType( IT_MEAT_RAW ))
	{
		return( -SKTRIG_ABORT );
	}

	if ( stage == SKTRIG_START )
	{
		return Calc_GetRandVal( 50 );
	}

	// Convert uncooked food to cooked food.
	ITEMID_TYPE id = (ITEMID_TYPE) RES_GET_INDEX( pFoodRaw->m_itFood.m_cook_id );
	if ( ! id )
	{
		id = (ITEMID_TYPE) pFoodRaw->Item_GetDef()->m_ttFoodRaw.m_cook_id.GetResIndex();
		if ( ! id )	// does not cook into anything.
		{
			return( -SKTRIG_QTY );
		}
	}

	CItem * pFoodCooked = NULL;
	if ( stage == SKTRIG_SUCCESS )
	{
		pFoodCooked = CItem::CreateTemplate( id  );
		if ( pFoodCooked )
		{
			SysMessage( "Mmm, smells good" );
			pFoodCooked->m_itFood.m_MeatType = pFoodRaw->m_itFood.m_MeatType;
			ItemBounce(pFoodCooked);
		}
	}
	else	// SKTRIG_FAIL
	{
		// Burn food
	}

	pFoodRaw->ConsumeAmount();

	if ( pFoodCooked == NULL )
	{
		return( -SKTRIG_QTY );
	}

	return( 0 );
}

int CChar::Skill_Taming( SKTRIG_TYPE stage )
{
	// m_Act_Targ = creature to tame.
	// Check the min required skill for this creature.
	// Related to INT ?

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		return( -SKTRIG_QTY );
	}
	if ( pChar == this )
	{
		SysMessage( "You are your own master." );
		return( -SKTRIG_QTY );
	}
	if ( pChar->m_pPlayer )
	{
		SysMessage( "You can't tame them." );
		return( -SKTRIG_QTY );
	}
	if ( GetTopDist3D( pChar ) > 10 )
	{
		SysMessage( "You are too far away" );
		return -SKTRIG_QTY;
	}

	UpdateDir( pChar );

	ASSERT( pChar->m_pNPC );

	int iTameBase = pChar->Skill_GetBase(SKILL_TAMING);
	if ( ! IsPriv( PRIV_GM )) // if its a gm doing it, just check that its not
	{
		// Is it tamable ?
		if ( pChar->IsStatFlag( STATF_Pet ))
		{
			SysMessagef( "%s is already tame.", (LPCTSTR) pChar->GetName());
			return( -SKTRIG_QTY );
		}

		// Too smart or not an animal.
		if ( ! iTameBase || pChar->Skill_GetBase(SKILL_ANIMALLORE))
		{
			SysMessagef( "%s cannot be tamed.", (LPCTSTR) pChar->GetName());
			return( -SKTRIG_QTY );
		}
	}

	if ( stage == SKTRIG_START )
	{
		int iDifficulty = iTameBase/10;

		if ( pChar->Memory_FindObjTypes( this, MEMORY_FIGHT|MEMORY_HARMEDBY|MEMORY_IRRITATEDBY|MEMORY_AGGREIVED ))
		{
			// I've attacked it b4 ?
			iDifficulty += 50;
		}

		m_atTaming.m_Stroke_Count = Calc_GetRandVal( 4 ) + 2;
		return( iDifficulty );
	}

	if ( stage == SKTRIG_FAIL )
	{
		// chance of being attacked ?
		return( 0 );
	}

	if ( stage == SKTRIG_STROKE )
	{
		static LPCTSTR const sm_szTameSpeak[] =
		{
			"I won't hurt you.",
			"I always wanted a %s like you",
			"Good %s",
			"Here %s",
		};

		if ( IsPriv( PRIV_GM ))
			return( 0 );
		if ( m_atTaming.m_Stroke_Count <= 0 )
			return( 0 );

		CGString sSpeak;
		sSpeak.Format( sm_szTameSpeak[ Calc_GetRandVal( COUNTOF( sm_szTameSpeak )) ], (LPCTSTR) pChar->GetName());
		Speak( sSpeak );

		// Keep trying and updating the animation
		m_atTaming.m_Stroke_Count --;
		Skill_SetTimeout();
		return -SKTRIG_STROKE;
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	// Create the memory of being tamed to prevent lame macroers
	CItemMemory * pMemory = pChar->Memory_FindObjTypes( this, MEMORY_SPEAK );
	if ( pMemory && pMemory->m_itEqMemory.m_Action == NPC_MEM_ACT_TAMED)
	{
		// See if I tamed it before
		// I did, no skill to tame it again
		CGString sSpeak;
		sSpeak.Format( "The %s remembers you and accepts you once more as it's master.", (LPCTSTR) pChar->GetName());
		ObjMessage( sSpeak, pChar );

		pChar->NPC_PetSetOwner( this );
		pChar->m_food = 50;	// this is good for something.
		pChar->m_Act_Targ = GetUID();
		pChar->Skill_Start( NPCACT_FOLLOW_TARG );
		return -SKTRIG_QTY;	// no credit for this.
	}

	pChar->NPC_PetSetOwner( this );
	pChar->m_food = 50;	// this is good for something.
	pChar->m_Act_Targ = GetUID();
	pChar->Skill_Start( NPCACT_FOLLOW_TARG );
	SysMessage( "It seems to accept you as master" );

	// Create the memory of being tamed to prevent lame macroers
	pMemory = pChar->Memory_AddObjTypes( this, MEMORY_SPEAK );
	ASSERT(pMemory);
	pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_TAMED;
	return( 0 );
}

int CChar::Skill_Lockpicking( SKTRIG_TYPE stage )
{
	// m_Act_Targ = the item to be picked.
	// m_Act_TargPrv = The pick.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CItem * pPick = m_Act_TargPrv.ItemFind();
	if ( pPick == NULL || ! pPick->IsType( IT_LOCKPICK ))
	{
		SysMessage( "You need a lock pick." );
		return -SKTRIG_QTY;
	}

	CItem * pLock = m_Act_Targ.ItemFind();
	if ( pLock == NULL )
	{
		SysMessage( "Use the lock pick on a lockable item." );
		return -SKTRIG_QTY;
	}

	if ( pPick->GetTopLevelObj() != this )	// the pick is gone !
	{
		SysMessage( "Your pick must be on your person." );
		return -SKTRIG_QTY;
	}

	if ( stage == SKTRIG_FAIL )
	{
		// Damage my pick
		pPick->OnTakeDamage( 1, this, DAMAGE_HIT_BLUNT );
		return( 0 );
	}

	if ( ! CanTouch( pLock ))	// we moved too far from the lock.
	{
		SysMessage( "You can't reach that." );
		return -SKTRIG_QTY;
	}

	if (  stage == SKTRIG_START )
	{
		return( pLock->Use_LockPick( this, true, false ));
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	if ( pLock->Use_LockPick( this, false, false ) < 0 )
	{
		return -SKTRIG_FAIL;
	}
	return 0;
}

int CChar::Skill_Hiding( SKTRIG_TYPE stage )
{
	// SKILL_Stealth = move while already hidden !
	// SKILL_Hiding
	// Skill required varies with terrain and situation ?
	// if we are carrying a light source then this should not work.

#if 0
	// We shoud just stay in HIDING skill. ?
#else
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_FAIL )
	{
		Reveal( STATF_Hidden );
		return 0;
	}

	if ( stage == SKTRIG_SUCCESS )
	{
		if ( IsStatFlag( STATF_Hidden ))
		{
			// I was already hidden ? so un-hide.
			Reveal( STATF_Hidden );
			return( -SKTRIG_ABORT );
		}

		ObjMessage( "You have hidden yourself well", this );
		StatFlag_Set( STATF_Hidden );
		UpdateMode();
		return( 0 );
	}

	if ( stage == SKTRIG_START )
	{
		// Make sure i am not carrying a light ?

		CItem * pItem = GetContentHead();
		for ( ; pItem; pItem = pItem->GetNext())
		{
			if ( ! CItemBase::IsVisibleLayer( pItem->GetEquipLayer()))
				continue;
			if ( pItem->Item_GetDef()->Can( CAN_I_LIGHT ))
			{
				SysMessage( "You are too well lit to hide" );
				return( -SKTRIG_QTY );
			}
		}

		return Calc_GetRandVal(70);
	}
#endif

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Herding( SKTRIG_TYPE stage )
{
	// m_Act_Targ = move this creature.
	// m_Act_p = move to here.
	// How do I make them move fast ? or with proper speed ???

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		SysMessage( "You lost your target!" );
		return( -SKTRIG_QTY );
	}
	CItem * pCrook = m_Act_TargPrv.ItemFind();
	if ( pCrook == NULL )
	{
		SysMessage( "You lost your crook!" );
		return( -SKTRIG_QTY );
	}

	// special GM version to move to coordinates.
	if ( ! IsPriv( PRIV_GM ))
	{
		// Herdable ?
		if ( pChar->m_pPlayer || 
			pChar->m_pNPC == NULL || 
			pChar->m_pNPC->m_Brain != NPCBRAIN_ANIMAL )
		{
			SysMessage( "They look somewhat annoyed" );
			return( -SKTRIG_QTY );
		}
	}
	else
	{
		if ( GetPrivLevel() < pChar->GetPrivLevel())
			return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_START )
	{
		UpdateAnimate( ANIM_ATTACK_WEAPON );
		int iIntVal = pChar->Stat_Get(STAT_INT) / 2;
		return( iIntVal + Calc_GetRandVal(iIntVal));
	}
	if ( stage == SKTRIG_FAIL )
	{
		// Irritate the animal.
		return 0;
	}

	//
	// Try to make them walk there.

	ASSERT( stage == SKTRIG_SUCCESS );

	if ( IsPriv( PRIV_GM ))
	{
		pChar->Spell_Teleport( m_Act_p, true, false );
	}
	else
	{
		pChar->m_Act_p = m_Act_p;
		pChar->Skill_Start( NPCACT_GOTO );
	}

	ObjMessage( "The animal goes where it is instructed", pChar );
	return( 0 );
}

int CChar::Skill_SpiritSpeak( SKTRIG_TYPE stage )
{
	if ( stage == SKTRIG_FAIL )
	{
		// bring ghosts ? hehe
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_START )
	{
		// difficulty based on spirits near ?
		return( Calc_GetRandVal( 90 ));
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		if ( IsStatFlag( STATF_SpiritSpeak ))
			return( -SKTRIG_ABORT );
		SysMessage( "You establish a connection to the netherworld." );
		Sound( 0x24a );
		Spell_Effect_Create( SPELL_NONE, LAYER_FLAG_SpiritSpeak, 1, 4*60*TICK_PER_SEC, this );
		return( 0 );
	}

	ASSERT(0);
	return( -SKTRIG_ABORT );
}

int CChar::Skill_Meditation( SKTRIG_TYPE stage )
{
	// SKILL_MEDITATION
	// Try to regen your mana even faster than normal.
	// Give experience only when we max out.

	if ( stage == SKTRIG_FAIL || stage == SKTRIG_ABORT )
	{
		return 0;
	}

	if ( stage == SKTRIG_START )
	{
		if ( m_StatMana >= Stat_Get(STAT_INT))
		{
			SysMessage( "You are at peace." );
			return( -SKTRIG_QTY );
		}
		m_atTaming.m_Stroke_Count = 0;

		SysMessage( "You attempt a meditative trance." );

		return Calc_GetRandVal(100);	// how hard to get started ?
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		if ( m_StatMana >= Stat_Get(STAT_INT))
		{
			SysMessage( "You are at peace." );
			return( 0 );	// only give skill credit now.
		}

		if ( m_atTaming.m_Stroke_Count == 0 )
		{
			Sound( 0x0f9 );
		}
		m_atTaming.m_Stroke_Count++;

		UpdateStatVal( STAT_INT, 1 );

		// next update. (depends on skill)
		Skill_SetTimeout();

		// Set a new possibility for failure ?
		// iDifficulty = Calc_GetRandVal(100);
		return( -SKTRIG_STROKE );
	}

	DEBUG_CHECK(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Healing( SKTRIG_TYPE stage )
{
	// SKILL_VETERINARY:
	// SKILL_HEALING
	// m_Act_TargPrv = bandages.
	// m_Act_Targ = heal target.
	//
	// should depend on the severity of the wounds ?
	// should be just a fast regen over time ?
	// RETURN:
	//  = -3 = failure.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CItem * pBandage = m_Act_TargPrv.ItemFind();
	if ( pBandage == NULL )
	{
		SysMessage("Where are your bandages?");
		return( -SKTRIG_QTY );
	}
	if ( ! pBandage->IsType(IT_BANDAGE))
	{
		SysMessage("Use a bandage");
		return( -SKTRIG_QTY );
	}

	CObjBase * pObj = m_Act_Targ.ObjFind();
	if ( ! CanTouch(pObj))
	{
		SysMessage("You must be able to reach the target");
		return( -SKTRIG_QTY );
	}

	CItemCorpse * pCorpse;	// resurrect by corpse.
	CChar * pChar;
	if ( pObj->IsItem())
	{
		// Corpse ?
		pCorpse = dynamic_cast <CItemCorpse*>(pObj);
		if ( pCorpse == NULL )
		{
			SysMessage("Try healing a creature.");
			return( -SKTRIG_QTY );
		}

		pChar = pCorpse->m_uidLink.CharFind();
	}
	else
	{
		pCorpse = NULL;
		pChar = m_Act_Targ.CharFind();
	}

	if ( pChar == NULL )
	{
		SysMessage( "This creature is beyond help." );
		return( -SKTRIG_QTY );
	}

	if ( GetDist(pObj) > 2 )
	{
		SysMessagef( "You are too far away to apply bandages on %s", (LPCTSTR) pObj->GetName());
		if ( pChar != this )
		{
			pChar->SysMessagef( "%s is attempting to apply bandages to %s, but they are too far away!", 
				(LPCTSTR) GetName(), (LPCTSTR) ( pCorpse ? ( pCorpse->GetName()) : "you" ));
		}
		return( -SKTRIG_QTY );
	}

	if ( pCorpse )
	{
		if ( ! pCorpse->IsTopLevel())
		{
			SysMessage( "Put the corpse on the ground" );
			return( -SKTRIG_QTY );
		}
		CRegionBase * pRegion = pCorpse->GetTopPoint().GetRegion(REGION_TYPE_AREA|REGION_TYPE_MULTI);
		if ( pRegion == NULL )
		{
			return( -SKTRIG_QTY );
		}
		if ( pRegion->IsFlag( REGION_ANTIMAGIC_ALL | REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_TELEPORT ))
		{
			SysMessage( "Your resurrection attempt is blocked by antimagic." );
			if ( pChar != this )
			{
				pChar->SysMessagef( "%s is attempting to apply bandages to %s, but they are blocked by antimagic!", 
					(LPCTSTR) GetName(), (LPCTSTR) pCorpse->GetName() );
			}
			return( -SKTRIG_QTY );
		}
	}
	else if ( pChar->IsStatFlag(STATF_DEAD))
	{
		SysMessage( "You can't heal a ghost! Try healing their corpse." );
		return( -SKTRIG_QTY );
	}

	if ( ! pChar->IsStatFlag( STATF_Poisoned|STATF_DEAD ) && pChar->m_StatHealth >= pChar->Stat_Get(STAT_STR) )
	{
		if ( pChar == this )
		{
			SysMessagef( "You are healthy" );
		}
		else
		{
			SysMessagef( "%s does not require you to heal or cure them!", (LPCTSTR) pChar->GetName());
		}
		return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_FAIL )
	{
		// just consume the bandage on fail and give some credit for trying.
		pBandage->ConsumeAmount();

		if ( pChar != this )
		{
			pChar->SysMessagef( "%s is attempting to apply bandages to %s, but has failed", 
				(LPCTSTR) GetName(), (LPCTSTR) pCorpse->GetName() );
		}

		// Harm the creature ?
		return( -SKTRIG_FAIL );
	}

	if ( stage == SKTRIG_START )
	{
		if ( pChar != this )
		{
			CGString sMsg;
			sMsg.Format( "apply bandages to %s", pChar->GetName());
			Emote( sMsg );
		}
		else
		{
			Emote( "apply bandages to self" );
		}
		if ( pCorpse )
		{
			// resurrect.
			return( 85 + Calc_GetRandVal(25));
		}
		if ( pChar->IsStatFlag( STATF_Poisoned ))
		{
			// level of the poison ?
			return( 50 + Calc_GetRandVal(50));
		}
		return( Calc_GetRandVal(80));
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	pBandage->ConsumeAmount();

	CItem * pBloodyBandage = CItem::CreateScript(Calc_GetRandVal(2) ? ITEMID_BANDAGES_BLOODY1 : ITEMID_BANDAGES_BLOODY2 );
	ItemBounce(pBloodyBandage);

	CSkillDef* pSkillDef = g_Cfg.GetSkillDef(Skill_GetActive());
	ASSERT(pSkillDef);
	int iSkillLevel = Skill_GetAdjusted( Skill_GetActive());

	if ( pCorpse )
	{
		CGString sText;
		sText.Format( "Resurrect %s", pChar->GetName());
		pChar->Spell_Resurrection( 0, pCorpse );
		return 0;
	}
	if ( pChar->IsStatFlag( STATF_Poisoned ))
	{
		if ( ! SetPoisonCure( iSkillLevel, true ))
			return( -1 );

		SysMessagef( "You cure %s of poisons!", (LPCTSTR) (pChar == this) ? "yourself" : ( pChar->GetName()));
		if ( pChar != this )
		{
			pChar->SysMessagef( "%s has cured you of poisons!", (LPCTSTR) GetName());
		}
		return( 0 );
	}

	// LAYER_FLAG_Bandage
	pChar->UpdateStatVal( STAT_STR, pSkillDef->m_Effect.GetLinear(iSkillLevel));
	return( 0 );
}

int CChar::Skill_RemoveTrap( SKTRIG_TYPE stage )
{
	// m_Act_Targ = trap
	// Is it a trap ?

	if ( stage == SKTRIG_STROKE )
	{
		return 0;		
	}
	CItem * pTrap = m_Act_Targ.ItemFind();
	if ( pTrap == NULL || ! pTrap->IsType(IT_TRAP))
	{
		SysMessage( "You should use this skill to disable traps" );
		return( -SKTRIG_QTY );
	}
	if ( ! CanTouch(pTrap))
	{
		SysMessage( "You can't reach it." );
		return( -SKTRIG_QTY );
	}
	if ( stage == SKTRIG_START )
	{
		// How difficult ?
		return Calc_GetRandVal(95);
	}
	if ( stage == SKTRIG_FAIL )
	{
		Use_Item( pTrap );	// set it off ?
		return 0;
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		// disable it.
		pTrap->SetTrapState( IT_TRAP_INACTIVE, ITEMID_NOTHING, 5*60 );
		return 0;
	}
	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Begging( SKTRIG_TYPE stage )
{
	// m_Act_Targ = Our begging target..

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL || pChar == this )
	{
		return( -SKTRIG_QTY );
	}

	if ( stage == SKTRIG_START )
	{
		SysMessagef("You grovel at %s's feet", (LPCTSTR) pChar->GetName());
		return( pChar->Stat_Get(STAT_INT));
	}
	if ( stage == SKTRIG_STROKE )
	{
		if ( m_pNPC )
			return -SKTRIG_STROKE;	// Keep it active.
		return( 0 );
	}
	if ( stage == SKTRIG_FAIL )
	{
		// Might they do something bad ?
		return( 0 );
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		// Now what ? Not sure how to make begging successful.
		// Give something from my inventory ?

		return( 0 );
	}

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Magery( SKTRIG_TYPE stage )
{
	// SKILL_MAGERY
	//  m_Act_p = location to cast to.
	//  m_Act_TargPrv = the source of the spell.
	//  m_Act_Targ = target for the spell.
	//  m_atMagery.m_Spell = the spell.
	
	if ( stage == SKTRIG_STROKE )
	{
		return( 0 );
	}
	if ( stage == SKTRIG_FAIL )
	{
		Spell_CastFail();
		return( 0 );
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		if ( ! Spell_CastDone())
		{
			return( -SKTRIG_ABORT );
		}
		return( 0 );
	}
	if ( stage == SKTRIG_START )
	{
		// NOTE: this should call SetTimeout();
		return Spell_CastStart();
	}

	ASSERT(0);
	return( -SKTRIG_ABORT );
}

int CChar::Skill_Fighting( SKTRIG_TYPE stage )
{
	// SKILL_ARCHERY
	// m_Act_Targ = attack target.

	if ( stage == SKTRIG_START )
	{
		// When do we get our next shot?

		DEBUG_CHECK( IsStatFlag( STATF_War ));
		
		m_atFight.m_War_Swing_State = WAR_SWING_EQUIPPING;

		int iDifficulty = g_Cfg.Calc_CombatChanceToHit( this, Skill_GetActive(), m_Act_Targ.CharFind(), m_uidWeapon.ItemFind());

		// Set the swing timer.
		int iWaitTime = Fight_GetWeaponSwingTimer()/2;	// start the anim immediately.
		if ( Skill_GetActive() == SKILL_ARCHERY )	// anim is funny for archery
			iWaitTime /= 2;
		SetTimeout( iWaitTime );

		return( iDifficulty );
	}

	if ( stage == SKTRIG_STROKE )
	{
		// Hit or miss my current target.
		if ( ! IsStatFlag( STATF_War ))
			return -SKTRIG_ABORT;

		if ( m_atFight.m_War_Swing_State != WAR_SWING_SWINGING )
		{
			m_atFight.m_War_Swing_State = WAR_SWING_READY;  // Waited my recoil time. So I'm ready.
		}

		Fight_HitTry();	// this cleans up itself.
		return -SKTRIG_STROKE;	// Stay in the skill till we hit.
	}
	
	return -SKTRIG_QTY;
}

int CChar::Skill_MakeItem( SKTRIG_TYPE stage )
{
	// SKILL_BLACKSMITHING:
	// SKILL_BOWCRAFT:
	// SKILL_CARPENTRY:
	// SKILL_INSCRIPTION:
	// SKILL_TAILORING:
	// SKILL_TINKERING:
	//
	// m_Act_Targ = the item we want to be part of this process.
	// m_atCreate.m_ItemID = new item we are making
	// m_atCreate.m_Amount = amount of said item.

	if ( stage == SKTRIG_START )
	{
		return m_Act_Difficulty;	// keep the already set difficulty
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		if ( ! Skill_MakeItem( m_atCreate.m_ItemID, m_Act_Targ, SKTRIG_SUCCESS ))
			return( -SKTRIG_ABORT );
		return 0;
	}
	if ( stage == SKTRIG_FAIL )
	{
		Skill_MakeItem( m_atCreate.m_ItemID, m_Act_Targ, SKTRIG_FAIL );
		return( 0 );	
	}
	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Tailoring( SKTRIG_TYPE stage )
{
	if ( stage == SKTRIG_SUCCESS )
	{
		Sound( SOUND_SNIP );	// snip noise
	}

	return( Skill_MakeItem( stage ));
}

int CChar::Skill_Inscription( SKTRIG_TYPE stage )
{
	if ( stage == SKTRIG_START )
	{
		// Can we even attempt to make this scroll ?
		// m_atCreate.m_ItemID = create this item
		Sound( 0x249 );
	}

	return( Skill_MakeItem( stage ));
}

int CChar::Skill_Bowcraft( SKTRIG_TYPE stage )
{
	// SKILL_BOWCRAFT
	// m_Act_Targ = the item we want to be part of this process.
	// m_atCreate.m_ItemID = new item we are making
	// m_atCreate.m_Amount = amount of said item.

	Sound( 0x055 );
	UpdateAnimate( ANIM_SALUTE );

	if ( stage == SKTRIG_START )
	{
		// Might be based on how many arrows to make ???
		m_atCreate.m_Stroke_Count = Calc_GetRandVal( 2 ) + 1;
	}

	return( Skill_MakeItem( stage ));
}

int CChar::Skill_Blacksmith( SKTRIG_TYPE stage )
{
	// m_atCreate.m_ItemID = create this item
	// m_Act_p = the anvil.
	// m_Act_Targ = the hammer.

	m_Act_p = g_World.FindItemTypeNearby( GetTopPoint(), IT_FORGE, 3 );
	if ( ! m_Act_p.IsValidPoint())
	{
		SysMessage( "You must be near a forge to smith" );
		return( -SKTRIG_QTY );
	}

	UpdateDir( m_Act_p );	// toward the forge

	if ( stage == SKTRIG_START )
	{
		m_atCreate.m_Stroke_Count = Calc_GetRandVal( 4 ) + 2;
	}

	if ( stage == SKTRIG_STROKE )
	{
		Sound( 0x02a );
		if ( m_atCreate.m_Stroke_Count <= 0 )
			return 0;

		// Keep trying and updating the animation
		m_atCreate.m_Stroke_Count --;
		UpdateAnimate( ANIM_ATTACK_WEAPON );	// ANIM_ATTACK_1H_DOWN
		Skill_SetTimeout();
		return( -SKTRIG_STROKE );	// keep active.
	}

	return( Skill_MakeItem( stage ));
}

int CChar::Skill_Carpentry( SKTRIG_TYPE stage )
{
	// m_Act_Targ = the item we want to be part of this process.
	// m_atCreate.m_ItemID = new item we are making
	// m_atCreate.m_Amount = amount of said item.

	Sound( 0x23d );

	if ( stage == SKTRIG_START )
	{
		// m_atCreate.m_ItemID = create this item
		m_atCreate.m_Stroke_Count = Calc_GetRandVal( 3 ) + 2;
	}

	if ( stage == SKTRIG_STROKE )
	{
		if ( m_atCreate.m_Stroke_Count <= 0 )
			return 0;

		// Keep trying and updating the animation
		m_atCreate.m_Stroke_Count --;
		UpdateAnimate( ANIM_ATTACK_WEAPON );
		Skill_SetTimeout();
		return( -SKTRIG_STROKE );	// keep active.
	}

	return( Skill_MakeItem( stage ));
}

int CChar::Skill_Information( SKTRIG_TYPE stage )
{
	// SKILL_ANIMALLORE:
	// SKILL_ARMSLORE:
	// SKILL_ANATOMY:
	// SKILL_ITEMID:
	// SKILL_EVALINT:
	// SKILL_FORENSICS:
	// SKILL_TASTEID:
	// Difficulty should depend on the target item !!!??
	// m_Act_Targ = target.

	if ( ! IsClient())	// purely informational
		return( -SKTRIG_QTY );

	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	SKILL_TYPE skill = Skill_GetActive();
	int iSkillLevel = Skill_GetAdjusted(skill);

	if ( stage == SKTRIG_START )
	{
		return GetClient()->OnSkill_Info( skill, m_Act_Targ, iSkillLevel, true );
	}
	if ( stage == SKTRIG_SUCCESS )
	{
		return GetClient()->OnSkill_Info( skill, m_Act_Targ, iSkillLevel, false );
	}

	ASSERT(0);
	return( -SKTRIG_QTY );
}

int CChar::Skill_Act_Napping( SKTRIG_TYPE stage )
{
	// NPCACT_Napping:
	// we are taking a small nap. keep napping til we wake. (or move)
	// AFK command

	if ( stage == SKTRIG_START )
	{
		// we are taking a small nap.
		SetTimeout( 2*TICK_PER_SEC );
		return( 0 );
	}

	if ( stage == SKTRIG_STROKE )
	{
		if ( m_Act_p != GetTopPoint())
			return( -SKTRIG_QTY );	// we moved.
		SetTimeout( 8*TICK_PER_SEC );
		Speak( "z", HUE_WHITE, TALKMODE_WHISPER );
		return -SKTRIG_STROKE;	// Stay in the skill till we hit.
	}

	return( -SKTRIG_QTY );	// something odd 
}

int CChar::Skill_Act_Breath( SKTRIG_TYPE stage )
{
	// NPCACT_BREATH
	// A Dragon I assume.
	// m_Act_Targ = my target.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_FAIL )
	{
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		return -SKTRIG_QTY;
	}

	m_Act_p = pChar->GetTopPoint();
	UpdateDir( m_Act_p );

	if ( stage == SKTRIG_START )
	{
		UpdateStatVal( STAT_DEX, -10 );
		UpdateAnimate( ANIM_MON_Stomp, false );
		SetTimeout( 3*TICK_PER_SEC );
		return 0;
	}

	ASSERT( stage == SKTRIG_SUCCESS );

	CPointMap pntMe = GetTopPoint();
	if ( pntMe.GetDist( m_Act_p ) > UO_MAP_VIEW_SIGHT )
	{
		m_Act_p.StepLinePath( pntMe, UO_MAP_VIEW_SIGHT );
	}

	Sound( 0x227 );
	int iDamage = m_StatStam/4 + Calc_GetRandVal( m_StatStam/4 );
	g_World.Explode( this, m_Act_p, 3, iDamage, DAMAGE_FIRE | DAMAGE_GENERAL );
	return( 0 );
}

int CChar::Skill_Act_Looting( SKTRIG_TYPE stage )
{
	// NPCACT_LOOTING
	// m_Act_Targ = the item i want.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage == SKTRIG_START )
	{
		if ( m_atLooting.m_iDistCurrent == 0 )
		{
			CScriptTriggerArgs Args( m_Act_Targ.ItemFind());
			if ( OnTrigger( CTRIG_NPCSeeWantItem, this, &Args ) == TRIGRET_RET_TRUE )
				return( false );
		}
		SetTimeout( 1*TICK_PER_SEC );
		return 0;
	}

	return( -SKTRIG_QTY );
}

int CChar::Skill_Act_Throwing( SKTRIG_TYPE stage )
{
	// NPCACT_THROWING
	// m_Act_Targ = my target.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		return( -SKTRIG_QTY );
	}

	m_Act_p = pChar->GetTopPoint();
	UpdateDir( m_Act_p );

	if ( stage == SKTRIG_START )
	{
		UpdateStatVal( STAT_DEX, -( 4 + Calc_GetRandVal(6)));
		UpdateAnimate( ANIM_MON_Stomp );
		return 0;
	}

	if ( stage != SKTRIG_SUCCESS )
	{
		return( -SKTRIG_QTY );
	}

	CPointMap pntMe = GetTopPoint();
	if ( pntMe.GetDist( m_Act_p ) > UO_MAP_VIEW_SIGHT )
	{
		m_Act_p.StepLinePath( pntMe, UO_MAP_VIEW_SIGHT );
	}
	SoundChar( CRESND_GETHIT );

	// a rock or a boulder ?
	ITEMID_TYPE id;
	int iDamage;
	if ( ! Calc_GetRandVal( 3 ))
	{
		iDamage = m_StatStam/4 + Calc_GetRandVal( m_StatStam/4 );
		id = (ITEMID_TYPE)( ITEMID_ROCK_B_LO + Calc_GetRandVal(ITEMID_ROCK_B_HI-ITEMID_ROCK_B_LO));
	}
	else
	{
		iDamage = 2 + Calc_GetRandVal( m_StatStam/4 );
		id = (ITEMID_TYPE)( ITEMID_ROCK_2_LO + Calc_GetRandVal(ITEMID_ROCK_2_HI-ITEMID_ROCK_2_LO));
	}

	CItem * pRock = CItem::CreateScript(id);
	ASSERT(pRock);
	pRock->SetAttr(ATTR_CAN_DECAY);
	// pRock->MoveNear( m_Act_p, Calc_GetRandVal(2));
	pRock->MoveToCheck( m_Act_p, this );
	pRock->Effect( EFFECT_BOLT, id, this );

	// did it hit ?
	if ( ! Calc_GetRandVal( pChar->GetTopPoint().GetDist( m_Act_p )))
	{
		pChar->OnTakeDamage( iDamage, this, DAMAGE_HIT_BLUNT );
	}

	return( 0 );
}

int CChar::Skill_Act_Training( SKTRIG_TYPE stage )
{
	// NPCACT_TRAINING
	// finished some traing maneuver.

	if ( stage == SKTRIG_START )
	{
		SetTimeout( 1*TICK_PER_SEC );
		return 0;
	}
	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}
	if ( stage != SKTRIG_SUCCESS )
	{
		return( -SKTRIG_QTY );
	}

	if ( m_Act_TargPrv == m_uidWeapon )
	{
		CItem * pItem = m_Act_Targ.ItemFind();
		if ( pItem )
		{
			switch ( pItem->GetType())
			{
			case IT_TRAIN_DUMMY:	// Train dummy.
				Use_Train_Dummy(pItem, false);
				break;
			case IT_TRAIN_PICKPOCKET:
				Use_Train_PickPocketDip(pItem, false);
				break;
			case IT_ARCHERY_BUTTE:	// Archery Butte
				Use_Train_ArcheryButte(pItem, false);
				break;
			}
		}
	}

	return 0;
}

//************************************
// General skill stuff.

int CChar::Skill_Stage( SKTRIG_TYPE stage )
{
	switch ( Skill_GetActive())
	{
	case SKILL_NONE:	// idling.
		return 0;
	case SKILL_ALCHEMY:
		return Skill_Alchemy(stage);
	case SKILL_ANATOMY:
	case SKILL_ANIMALLORE:
	case SKILL_ITEMID:
	case SKILL_ARMSLORE:
		return Skill_Information(stage);
	case SKILL_PARRYING:
		return 0;
	case SKILL_BEGGING:
		return Skill_Begging(stage);
	case SKILL_BLACKSMITHING:
		return Skill_Blacksmith(stage);
	case SKILL_BOWCRAFT:
		return Skill_Bowcraft(stage);
	case SKILL_PEACEMAKING:
		return Skill_Peacemaking(stage);
	case SKILL_CAMPING:
		return 0;
	case SKILL_CARPENTRY:
		return Skill_Carpentry(stage);
	case SKILL_CARTOGRAPHY:
		return Skill_Cartography(stage);
	case SKILL_COOKING:
		return Skill_Cooking(stage);
	case SKILL_DETECTINGHIDDEN:
		return Skill_DetectHidden(stage);
	case SKILL_ENTICEMENT:
		return Skill_Enticement(stage);
	case SKILL_EVALINT:
		return Skill_Information(stage);
	case SKILL_HEALING:
		return Skill_Healing(stage);
	case SKILL_FISHING:
		return Skill_Fishing(stage);
	case SKILL_FORENSICS:
		return Skill_Information(stage);
	case SKILL_HERDING:
		return Skill_Herding(stage);
	case SKILL_HIDING:
		return Skill_Hiding(stage);
	case SKILL_PROVOCATION:
		return Skill_Provocation(stage);
	case SKILL_INSCRIPTION:
		return Skill_Inscription(stage);
	case SKILL_LOCKPICKING:
		return Skill_Lockpicking(stage);
	case SKILL_MAGERY:
		return Skill_Magery(stage);
	case SKILL_MAGICRESISTANCE:
		return 0;
	case SKILL_TACTICS:
		return 0;
	case SKILL_SNOOPING:
		return Skill_Snooping(stage);
	case SKILL_MUSICIANSHIP:
		return Skill_Musicianship(stage);
	case SKILL_POISONING:	// 30
		return Skill_Poisoning(stage);
	case SKILL_ARCHERY:
		return Skill_Fighting(stage);
	case SKILL_SPIRITSPEAK:
		return Skill_SpiritSpeak(stage);
	case SKILL_STEALING:
		return Skill_Stealing(stage);
	case SKILL_TAILORING:
		return Skill_Tailoring(stage);
	case SKILL_TAMING:
		return Skill_Taming(stage);
	case SKILL_TASTEID:
		return Skill_Information(stage);
	case SKILL_TINKERING:
		return Skill_MakeItem(stage);
	case SKILL_TRACKING:
		return Skill_Tracking(stage);
	case SKILL_VETERINARY:
		return Skill_Healing(stage);
	case SKILL_SWORDSMANSHIP:
	case SKILL_MACEFIGHTING:
	case SKILL_FENCING:
	case SKILL_WRESTLING:
		return Skill_Fighting(stage);
	case SKILL_LUMBERJACKING:
		return Skill_Lumberjack(stage);
	case SKILL_MINING:
		return Skill_Mining(stage);
	case SKILL_MEDITATION:
		return Skill_Meditation(stage);
	case SKILL_Stealth:
		return Skill_Hiding(stage);
	case SKILL_RemoveTrap:
		return Skill_RemoveTrap(stage);
	case SKILL_NECROMANCY:
		return Skill_Magery(stage);

	case NPCACT_BREATH:
		return Skill_Act_Breath(stage);
	case NPCACT_LOOTING:
		return Skill_Act_Looting(stage);
	case NPCACT_THROWING:
		return Skill_Act_Throwing(stage);
	case NPCACT_TRAINING:
		return Skill_Act_Training(stage);
	case NPCACT_Napping:
		return Skill_Act_Napping(stage);

	default:
		if ( ! IsSkillBase(Skill_GetActive()))
		{
			if ( stage == SKTRIG_STROKE )
				return( -SKTRIG_STROKE ); // keep these active. (NPC modes)
			return 0;	
		}
	}

	SysMessage( "Skill not implemented!" );
	return -SKTRIG_QTY;	
}

void CChar::Skill_Fail( bool fCancel )
{
	// This is the normal skill check failure. 
	// Other types of failure don't come here.
	// 
	// ARGS:
	//	fCancel = no credt.
	//  else We still get some credit for having tried.

	SKILL_TYPE skill = Skill_GetActive();
	if ( skill == SKILL_NONE )
		return;

	DEBUG_CHECK( IsSkillBase(skill) || IsSkillNPC(skill));
	if ( ! IsSkillBase(skill))
	{
		Skill_Cleanup();
		return;
	}

	if ( m_Act_Difficulty > 0 )
	{
		m_Act_Difficulty = - m_Act_Difficulty;
	}

	TRIGRET_TYPE iRet = OnTrigger( CTRIG_SkillFail, this );
	if ( iRet == TRIGRET_RET_TRUE )
	{
		Skill_Cleanup();
		return;
	}

	CSkillDef* pSkillDef = g_Cfg.GetSkillDef(skill);
	ASSERT(pSkillDef);
	if ( pSkillDef->HasTrigger( SKTRIG_FAIL ))
	{
		// RES_SKILL
		CResourceLock s;
		if ( pSkillDef->ResourceLock( s ))
		{
			TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, CSkillDef::sm_szTrigName[SKTRIG_FAIL], this );
			if ( iRet == TRIGRET_RET_TRUE  )
				goto do_cleanup;
		}
	}

	if ( Skill_Stage( SKTRIG_FAIL ) >= 0 )
	{
		// Get some experience for failure ?
		Skill_Experience( skill, m_Act_Difficulty );
	}

do_cleanup:
	Skill_Cleanup();
}

int CChar::Skill_Done()
{
	// We just finished using a skill. ASYNC timer expired.
	// m_Act_Skill = the skill.
	// Consume resources that have not already been consumed.
	// Confer the benefits of the skill.
	// calc skill gain based on this.
	//
	// RETURN: Did we succeed or fail ?
	//   0 = success
	//	 -SKTRIG_STROKE = stay in skill. (stroke)
	//   -SKTRIG_FAIL = we must print the fail msg. (credit for trying)
	//   -SKTRIG_ABORT = we must print the fail msg. (But get no credit, canceled )
	//   -SKTRIG_QTY = special failure. clean up the skill but say nothing. (no credit)

	SKILL_TYPE skill = Skill_GetActive();
	if ( skill == SKILL_NONE )	// we should not be coming here (timer should not have expired)
		return -SKTRIG_QTY;

	// multi stroke tried stuff here first.
	// or stuff that never really fails.
	int iRet = Skill_Stage(SKTRIG_STROKE);
	if ( iRet < 0 )
		return( iRet );
	if ( m_Act_Difficulty < 0 )
	{
		// Was Bound to fail. But we had to wait for the timer anyhow.
		return -SKTRIG_FAIL;
	}

	// Success for the skill.
	iRet = Skill_Stage(SKTRIG_SUCCESS);
	if ( iRet < 0 )
		return iRet;

	// Success = Advance the skill
	Skill_Experience( skill, m_Act_Difficulty );
	Skill_Cleanup();
	return( -SKTRIG_SUCCESS );
}

bool CChar::Skill_Wait( SKILL_TYPE skilltry )
{
	// Some sort of push button skill.
	// We want to do some new skill. Can we ?
	// If this is the same skill then tell them to wait.

	if ( IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Freeze | STATF_Stone ))
	{
		SysMessage( "You can't do much in your current state." );
		return( true );
	}

	SKILL_TYPE skill = Skill_GetActive();
	if ( skill == SKILL_NONE )	// not currently doing anything.
	{
		Reveal();
		return( false );
	}

	// What if we are in combat mode ?
	if ( IsStatFlag( STATF_War ))
	{
		SysMessage( "You are preoccupied with thoughts of battle." );
		return( true );
	}

	// Passive skills just cancel.
	// SKILL_SPIRITSPEAK ?
	if ( skilltry != skill )
	{
		if ( skill == SKILL_MEDITATION ||
			skill == SKILL_HIDING ||
			skill == SKILL_Stealth )
		{
			Skill_Fail( true );
			return( false );
		}
	}

	SysMessage( "You must wait to perform another action" );
	return ( true );
}

bool CChar::Skill_Start( SKILL_TYPE skill, int iDifficulty )
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
		if ( skill != SKILL_NONE &&
			! IsSkillBase(skill) &&
			! IsSkillNPC(skill))
		{
			DEBUG_ERR(( "UID:0%x Bad Skill %d for '%s'" DEBUG_CR, GetUID(), skill, GetName()));
			return( false );
		}
		m_Act_SkillCurrent = skill;
		return( true );
	}

	if ( Skill_GetActive() != SKILL_NONE )
	{
		Skill_Fail( true );	// Fail previous skill unfinished. (with NO skill gain!)
	}

	if ( skill != SKILL_NONE )
	{
		m_Act_SkillCurrent = skill;	// Start using a skill.
		m_Act_Difficulty = iDifficulty;

		TRIGRET_TYPE iTrigRet = OnTrigger( CTRIG_SkillStart, this );
		if ( iTrigRet == TRIGRET_RET_TRUE )
		{
			// goto do_cleanup;
		}

		ASSERT( IsSkillBase(skill) || IsSkillNPC(skill));

		// Some skill can start right away. Need no targetting.
		// 0-100 scale of Difficulty 
		m_Act_Difficulty = Skill_Stage(SKTRIG_START);
		if ( m_Act_Difficulty < 0 )
		{
			Skill_Cleanup();
			return(false);
		}

		if ( IsSkillBase(skill))
		{
			const CSkillDef* pSkillDef = g_Cfg.GetSkillDef(skill);
			ASSERT(pSkillDef);
			int iWaitTime = pSkillDef->m_Delay.GetLinear( Skill_GetBase(skill) );
			if ( iWaitTime )
			{
				// How long before complete skill.
				SetTimeout( iWaitTime );
			}
		}
		if ( IsTimerExpired())
		{
			// the skill should have set it's own delay!?
			SetTimeout( 1 );
		}
		if ( m_Act_Difficulty > 0 )
		{
			if ( ! Skill_CheckSuccess( skill, m_Act_Difficulty ))
				m_Act_Difficulty = - m_Act_Difficulty; // will result in Failure ?
		}
	}

	// emote the action i am taking.
	if (( g_Cfg.m_wDebugFlags & DEBUGF_NPC_EMOTE ) || IsStatFlag(STATF_EmoteAction))
	{
		Emote( Skill_GetName(true));
	}

	return( true );
}

