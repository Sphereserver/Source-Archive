//
// CCharNPCAct.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Actions specific to an NPC.
//

#include "graysvr.h"	// predef header.

//////////////////////////
// CChar

bool CChar::NPC_OnVerb( CScript &s, CTextConsole * pSrc ) // Execute command from script
{
	// Stuff that only NPC's do.
	ASSERT( m_pNPC );

	static const TCHAR * table[] =
	{
		"BUY",
		"BYE",
		"FLEE",
		"HIRE",
		"LEAVE",
		"PETRETRIEVE",
		"PETSTABLE",
		"R",
		"RESTOCK",
		"RUN",
		"SCRIPT",
		"SELL",
		"SHRINK",
		"TRAIN",
		"W",
		"WALK",
	};

	CChar * pCharSrc = pSrc->GetChar();

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table)))
	{
	case 0: // "BUY"
		// Open up the buy dialog.
		if ( pCharSrc == NULL || ! pCharSrc->IsClient())
			return( false );
		if ( ! pCharSrc->GetClient()->addShopMenuBuy( this ))
		{
			Speak( "Sorry I have no goods to sell" );
		}
		break;
	case 1: // "BYE"
		Skill_Start( SKILL_NONE );
		m_Act_Targ.ClearUID();
		break;
	case 2: // "FLEE",
do_leave:
		// Short amount of fleeing.
		Skill_Start( NPCACT_FLEE );
		break;
	case 3: // "HIRE"
		return NPC_OnHireHear( pCharSrc);
	case 4: // "LEAVE",
		goto do_leave;
	case 5: // "PETRETRIEVE"
		return( NPC_StablePetRetrieve( pCharSrc ));
	case 6: // "PETSTABLE"
		return( NPC_StablePetSelect( pCharSrc ));
	case 7: // "R",
		goto do_run;
	case 8: // "RESTOCK" = individual restock command.
		return NPC_Vendor_Restock( s.GetArgVal());
	case 9: //	"RUN",
do_run:
		m_Act_p = GetTopPoint();
		m_Act_p.Move( GetDirStr( s.GetArgStr()));
		NPC_WalkToPoint( true );
		break;
	case 10:	// "SCRIPT"
		// Give the NPC a script book !
		return NPC_Script_Command( s.GetArgStr(), true );
	case 11: // "SELL"
		// Open up the sell dialog.
		if ( pCharSrc == NULL || ! pCharSrc->IsClient()) 
			return( false );
		if ( ! pCharSrc->m_pClient->addShopMenuSell( this ))
		{
			Speak( "You have nothing I'm interested in" );
		}
		break;
	case 12:	// "SHRINK"
		// we must own it.
		if ( ! NPC_IsOwnedBy( pCharSrc )) 
			return( false );
		return( NPC_Shrink() != NULL );
	case 13: // "TRAIN"
		return( NPC_OnTrainHear( pCharSrc, s.GetArgStr()));
	case 14: //	"W",
		goto do_walk;
	case 15: //	"WALK",
do_walk:
		m_Act_p = GetTopPoint();
		m_Act_p.Move( GetDirStr( s.GetArgStr()));
		NPC_WalkToPoint( false );
		break;
	default:
		return( false );
	}
	return( true );
}

const LAYER_TYPE CChar::sm_VendorLayers[] = // static
{
	LAYER_VENDOR_STOCK, LAYER_VENDOR_EXTRA, LAYER_VENDOR_BUYS, LAYER_BANKBOX,
};

bool CChar::NPC_Vendor_Dump( CItemContainer * pBank )
{
	// Dump the contents of my vendor boxes into this container.

	if ( m_pNPC == NULL ) 
		return false;
	if ( ! m_pNPC->IsVendor()) 
		return( false );

	for ( int i=0; i<3; i++ )
	{
		CItemContainer * pCont = GetBank( sm_VendorLayers[i] );
		if ( pCont == NULL )
			return( false );
		CItem * pItemNext;
		CItem * pItem = pCont->GetContentHead();
		for ( ; pItem != NULL; pItem = pItemNext )
		{
			pItemNext = pItem->GetNext();
			pBank->ContentAdd( pItem );
		}
	}

	return( true );
}

bool CChar::NPC_Vendor_Restock( int iTimeSec )
{
	if ( m_pNPC == NULL ) 
		return false;

	if ( IsStat( STATF_Spawned ))
	{
		// kill all spawned creatures.
		Delete();
		return( true );
	}

	// Not a player vendor.
	if ( ! IsStat( STATF_Pet ))
	{
		if ( m_pNPC->IsVendor())
		{
			for ( int i=0; i<COUNTOF(sm_VendorLayers); i++ )
			{
				CItemContainer * pCont = GetBank( sm_VendorLayers[i] );
				if ( pCont == NULL )
					return( false );
				if ( iTimeSec )
				{
					pCont->SetRestockTimeSeconds( iTimeSec );
				}
				pCont->Restock();
			}
		}

		if ( m_Home.IsValid() && ! IsStat( STATF_Freeze | STATF_Stone ))
		{
			// send them back to their "home" and re-equip them.
#if 0
			NPC_LoadScript(true);
#else
			if ( !m_pNPC->IsVendor())
				NPC_LoadScript(false);	// if we do this to vendors, we fall into an infinite loop
#endif
			MoveNear( m_Home, 5 );
		}
	}

	return( true );
}

bool CChar::NPC_StablePetSelect( CChar * pCharPlayer )
{
	// I am a stable master.
	// I will stable a pet for the player.

	if ( pCharPlayer == NULL )
		return( false );
	if ( ! pCharPlayer->IsClient())
		return( false );

	// Might have too many pets already ?

	int iCount = 0;
	CItemContainer * pBank = GetBank();
	if ( pBank->GetCount() >= MAX_ITEMS_CONT )
	{
		Speak( "I'm sorry the stables are full" );
		return( false );
	}

	CItem* pItem = pBank->GetContentHead();
	for ( ; pItem!=NULL ; pItem = pItem->GetNext())
	{
		if ( pItem->m_type == ITEM_FIGURINE && pItem->m_uidLink == pCharPlayer->GetUID())
			iCount++;
	}

	if ( iCount > 10 )
	{
		Speak( "I'm sorry you have too many pets stabled here already. Tell me to 'retrieve' if you want them." );
		return( false );
	}

	pCharPlayer->m_pClient->m_Targ_PrvUID = GetUID();
	pCharPlayer->m_pClient->addTarget( TARGMODE_PET_STABLE, "What pet would you like to stable here" );

	return( true );
}

bool CChar::NPC_StablePetRetrieve( CChar * pCharPlayer )
{
	// Get pets for this person from my inventory.
	// May want to put up a menu ???

	if ( m_pNPC == NULL )
		return( false );
	if ( m_pNPC->m_Brain != NPCBRAIN_STABLE )
		return( false );

	int iCount = 0;
	CItem* pItem = GetBank()->GetContentHead();
	while ( pItem!=NULL )
	{
		CItem * pItemNext = pItem->GetNext();
		if ( pItem->m_type == ITEM_FIGURINE && pItem->m_uidLink == pCharPlayer->GetUID())
		{
			if ( pCharPlayer->Use_Figurine( pItem, 2 ))
			{
				pItem->Delete();
			}
			iCount++;
		}
		pItem = pItemNext;
	}

	if ( ! iCount )
	{
		Speak( "Sorry I have no stabled animals for you" );
		return( false );
	}
	else
	{
		Speak( "Treat them well" );
	}

	return( true );
}

void CChar::NPC_ActStart_SpeakTo( CChar * pSrc )
{
	// My new action is that i am speaking to this person.
	m_Act_Targ = pSrc->GetUID();
	Skill_Start( NPCACT_TALK );
	SetTimeout( 60*TICK_PER_SEC );
	UpdateDir( pSrc );
}

bool CChar::NPC_OnHearTrigger( CScriptLock & s, const TCHAR * pCmd, CChar * pSrc )
{
	// Check all the keys in this script section.
	// look for pattern or partial trigger matches.

	bool fMatch = false;

	while ( s.ReadKeyParse())
	{
		if ( s.IsKeyHead("ON",2))
		{
			// Look for some key word.
			strupr( s.GetArgStr());
			if ( Text_Match( s.GetArgStr(), pCmd ) == MATCH_VALID )
				fMatch = true;
			continue;
		}

		if ( ! fMatch )
			continue;	// look for the next "ON" section.

		TRIGRET_TYPE iRet = CObjBase::OnTriggerRun( s, TRIGRUN_SECTION_EXEC, pSrc );
		if ( iRet != TRIGRET_RET_FALSE )
		{
			return( true );	// we are done processing.
		}

		fMatch = false;
	}

	return( false );	// continue looking.
}

void CChar::NPC_OnHear( const TCHAR * pCmd, CChar * pSrc )
{
	// This CChar has heard you say something.
	ASSERT( pSrc );
	if ( m_pNPC == NULL ) 
		return;

	// Pets always have a basic set of actions.
	if ( NPC_OnHearPetCmd( pCmd, pSrc ))
		return;

	if ( ! NPC_CanSpeak())	// can they speak ?
		return;
	if ( GetActiveSkill() == SKILL_BEGGING ) // busy begging. (hack)
		return;

	////////////////////

	// Take some default action
	// too busy to talk ?

	bool fMyPet = NPC_IsOwnedBy( pSrc, true );

#if 1
	if ( IsStat( STATF_War ) && ! fMyPet )
	{
		if ( GetCreatureType() == NPCBRAIN_HUMAN )
		{
			if ( Memory_FindObjTypes( pSrc, MEMORY_FIGHT|MEMORY_AGGREIVED ))
			{
				static const TCHAR * Speak_War[] =
				{
					"You will not talk your way out of this",
					"Be quiet and fight scoundrel" ,
				};
				if ( GetRandVal( 5 )) 
					return;
				Speak( Speak_War[ GetRandVal( COUNTOF( Speak_War )) ] );
				return;
			}
			Speak( "I am too busy fighting to speak with thee" );
		}
		return;
	}
#endif

	// Was NPC talking to someone else ?
	if (( GetActiveSkill() == NPCACT_TALK || GetActiveSkill() == NPCACT_TALK_FOLLOW )
		&& m_Act_Targ != pSrc->GetUID())
	{
		CChar * pCharOld = m_Act_Targ.CharFind();
		if ( pCharOld != NULL && GetTopDist3D( pCharOld ) < UO_MAP_VIEW_SIGHT )
		{
			CGString sMsg;
			sMsg.Format( "Excuse me %s, but %s wants to speak to me",
				pCharOld->GetName(), pSrc->GetName());
			Speak( sMsg );
		}
	}

	// You are the new speaking target.
	if ( ! IsStat( STATF_Pet ))
	{
		NPC_ActStart_SpeakTo( pSrc );

		// Put this ahead of the speech scripts because of conflicts ??
		if ( FindStrWord( pCmd, "Train" ) || FindStrWord( pCmd, "Teach" ))
		{
			if ( NPC_OnTrainHear( pSrc, pCmd ))
				return;
		}
	}

	// Do the scripts want me to take some action based on this speech.

	for ( int i=0; i<m_pNPC->m_Speech.GetCount(); i++ )
	{
		CScriptLock s;
		if ( ! m_pNPC->m_Speech[i]->OpenFrag( s ))
			continue;
		if ( NPC_OnHearTrigger( s, pCmd, pSrc ))
			return;
	}
	for ( i=0; i<m_pDef->m_Speech.GetCount(); i++ )
	{
		CScriptLock s;
		if ( ! m_pDef->m_Speech[i]->OpenFrag( s ))
			continue;
		if ( NPC_OnHearTrigger( s, pCmd, pSrc ))
			return;
	}

	// hard code some default reactions.
	CGString sDefault;

	if ( m_pNPC->m_Brain == NPCBRAIN_HEALER )
	{
		if ( NPC_LookAtChar( pSrc, 1 ))
			return;
	}
	if ( ! IsStat( STATF_Pet ) &&
		m_pDef->GetHireDayWage() &&
		FindStrWord( pCmd, "Hire" ))
	{
		if ( NPC_OnHireHear( pSrc ))
			return;
	}
	if ( FindStrWord( pCmd, "Bye" ) || FindStrWord( pCmd, "GoodBye" ))
	{
		r_Verb( CScript( "BYE" ), pSrc );
		return;
	}
	if ( m_pNPC->m_Brain == NPCBRAIN_BANKER && FindStrWord( pCmd, "Bank" ))
	{
		r_Verb( CScript( "BANK" ), pSrc );
		return;
	}
	if ( m_pNPC->m_Brain == NPCBRAIN_STABLE )
	{
		if ( FindStrWord( pCmd, "Retrieve" ))
		{
			r_Verb( CScript( "PETRETRIEVE" ), pSrc );
			return;
		}
		if ( FindStrWord( pCmd, "Stable" ))
		{
			r_Verb( CScript( "PETSTABLE" ), pSrc );
			return;
		}
	}
	if ( FindStrWord( pCmd, "Move" ))
	{
		r_Verb( CScript( "LEAVE" ), pSrc );
		return;
	}
	if ( m_pNPC->IsVendor())
	{
		if ( FindStrWord( pCmd, "Buy" ))
		{
			r_Verb( CScript( "BUY" ), pSrc );
			return;
		}
		if ( FindStrWord( pCmd, "Sell" ))
		{
			r_Verb( CScript( "SELL" ), pSrc );
			return;
		}
		if ( fMyPet )
		{
			if ( FindStrWord(pCmd,"STOCK") || FindStrWord(pCmd,"INVENTORY"))
			{
				// Magic restocking container.
				Speak( "Put items you want me to sell in here." );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_STOCK );
				return;
			}
			if (FindStrWord(pCmd,"BOUGHT") || FindStrWord(pCmd,"RECIEVED"))
			{
				Speak( "This contains the items I have bought." );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_EXTRA );
				return;
			}
			if (FindStrWord(pCmd,"SAMPLES"))
			{
				Speak( "Put sample items like you want me to purchase in here" );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_BUYS );
				return;
			}
		}
	}
	if ( m_pNPC->m_Brain == NPCBRAIN_BANKER )
	{
		if ( FindStrWord( pCmd, "Balance" ))	// "BANKBALANCE"
		{
			sDefault.Format( "You have %d gold in your account.", pSrc->GetBank()->ContentCount(ITEMID_GOLD_C1));
			Speak( sDefault );
			return;
		}
	}

	// default stuff that all NPC's know..

	if ( FindStrWord( pCmd, "Where" ))	// "REGION.NAME"
	{
		sDefault.Format( "You are in %s", pSrc->m_pArea->GetName());
		Speak( sDefault );
		return;
	}
	if ( FindStrWord( pCmd, "Time" ))	// "LOCALTIME"
	{
		Speak( GetTopSector()->GetGameTime());
		return;
	}

	// can't figure you out.

	++ m_atTalk.m_HearUnknown;
	if ( OnTrigger( CTRIG_HearUnknown, pSrc ))
		return;

	// Come up with some other default.
	static const TCHAR * Speak_NoUnderstand[] =
	{
		"Huh?",
		"I don't understand thee",
		"I'm sorry but that's not anything I know about",
	};
	sDefault = Speak_NoUnderstand[ GetRandVal( COUNTOF( Speak_NoUnderstand )) ];

	if ( m_atTalk.m_HearUnknown > 4 ) 
	{
		Skill_Start( SKILL_NONE ); // say good by
	}
	if ( pCmd[0] )
	{
		Speak( sDefault );
	}
}

#if 0

bool CChar::OnItemAcquire( CItem * pItem )
{
	// I now have this object.
	// What do i want to use it for ?
	// I can beneift from items that i find.

	// Equip weapons and armor, (if better than what we have)

	// drink (beneficial) potions (etc) only if in war mode.


}

#endif

int CChar::NPC_OnTrainCheck( CChar * pCharSrc, SKILL_TYPE Skill )
{
	// Can we train in this skill ?
	// RETURN: Amount of skill we can train.
	//

	if ( ! IsSkillBase( Skill ))
	{
		Speak( "I know nothing about that" );
		return( 0 );
	}

	int iSkillSrcVal = pCharSrc->Skill_GetBase(Skill);
	int iSkillVal = Skill_GetBase(Skill);
	int iTrainCost = NPC_GetTrainMax( Skill ) - iSkillSrcVal;

	const TCHAR * pszMsg;
	if ( iSkillVal <= 0 )
	{
		pszMsg = "I know nothing about %s";
	}
	else if ( iSkillSrcVal > iSkillVal )
	{
		pszMsg = "You know more about %s than I do";
	}
	else if ( iTrainCost <= 0 )
	{
		pszMsg = "You already know as much as I can teach of %s";
	}
	else
	{
		return( iTrainCost );
	}

	CGString sMsg;
	sMsg.Format( pszMsg, g_Serv.m_SkillDefs[Skill]->GetKey());
	Speak( sMsg );
	return( 0 );
}

bool CChar::NPC_OnTrainPay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold )
{
	ASSERT( pMemory );

	SKILL_TYPE skill = (SKILL_TYPE)( pMemory->m_itEqMemory.m_Skill );
	if ( ! IsSkillBase( skill ))
	{
		Speak( "I can't remember what I was supposed to teach you" );
		return( false );
	}

	int iTrainCost = NPC_OnTrainCheck( pCharSrc, skill );
	if ( iTrainCost <= 0 )
		return false;

	Speak( "Let me show you something of how this is done" );

	// Consume as much money as we can train for.
	ASSERT( pGold );
	if ( pGold->GetAmount() < iTrainCost )
	{
		iTrainCost = pGold->GetAmount();
	}
	else if ( pGold->GetAmount() == iTrainCost )
	{
		Speak( "That is all I can teach." );
		pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_NONE;
	}
	else
	{
		Speak( "I wish i could teach more but I cannot." );
		pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_NONE;

		// Give change back.
		pGold->CreateDupeSplit( iTrainCost );
	}

	GetPackSafe()->ContentAdd( pGold );	// take my cash.

	// Give credit for training.
	pCharSrc->Skill_SetBase( skill, pCharSrc->Skill_GetBase(skill) + iTrainCost );
	return( true );
}

bool CChar::NPC_OnTrainHear( CChar * pCharSrc, const TCHAR * pCmd )
{
	// We are asking for training ?

	if ( ! m_pNPC )
		return( false );
	if ( ! m_pNPC->IsVendor() &&
		m_pNPC->m_Brain != NPCBRAIN_HUMAN )
	{
		return false;
	}

	if ( Memory_FindObjTypes( pCharSrc, MEMORY_FIGHT|MEMORY_AGGREIVED|MEMORY_IRRITATEDBY ))
	{
		Speak( "I would never train the likes of you." );
		return false;
	}

	// Did they mention a skill name i recognize ?

	int i=SKILL_NONE+1;
	for ( ; i<SKILL_QTY; i++ )
	{
		if ( ! FindStrWord( pCmd, g_Serv.m_SkillDefs[i]->GetKey()))
			continue;

		// Can we train in this ?
		int iTrainCost = NPC_OnTrainCheck( pCharSrc, (SKILL_TYPE) i );
		if ( iTrainCost <= 0 ) 
			return true;

		CGString sMsg;
		sMsg.Format( "For %d gold I will train you in all I know of %s. For less gold I will teach you less",
			iTrainCost, g_Serv.m_SkillDefs[i]->GetKey());

		Speak( sMsg );
		CItem * pMemory = Memory_AddObjTypes( pCharSrc, MEMORY_SPEAK );
		pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_TRAIN;
		pMemory->m_itEqMemory.m_Skill = i;
		return true;
	}

	// What can he teach me about ?
	// Just tell them what we can teach them or set up a memory to train.

	TCHAR szMsg[ MAX_SCRIPT_LINE_LEN ];
	strcpy( szMsg, "For a fee I can teach thee of " );

	const TCHAR * pPrvSkill = NULL;

	int iCount = 0;
	for ( i=SKILL_NONE+1; i<SKILL_QTY; i++ )
	{
		int iDiff = NPC_GetTrainMax( (SKILL_TYPE)i ) - pCharSrc->Skill_GetBase( (SKILL_TYPE) i);
		if ( iDiff <= 0 )
			continue;

		if ( iCount > 6 )
		{
			pPrvSkill = "more";
			break;
		}
		if ( iCount > 1 )
		{
			strcat( szMsg, ", " );
		}
		if ( pPrvSkill )
		{
			strcat( szMsg, pPrvSkill );
		}
		pPrvSkill = g_Serv.m_SkillDefs[i]->GetKey();
		iCount ++;
	}

	if ( iCount == 0 )
	{
		Speak( "There is nothing that I can teach you." );
		return true;
	}
	if ( iCount > 1 )
	{
		strcat( szMsg, " and " );
	}

	strcat( szMsg, pPrvSkill );
	strcat( szMsg, "." );
	Speak( szMsg );
	return( true );
}

bool CChar::NPC_Act_Begging( CChar * pChar )
{
	// SKILL_BEGGING
	bool fSpeak;

	if ( pChar )
	{
		// Is this a proper target for begging ?
		if ( pChar == this ||
			pChar->m_pNPC ||	// Don't beg from NPC's or the PC you just begged from
			pChar->GetUID() == m_Act_TargPrv )	// Already targetting this person.
			return( false );

		if ( pChar->GetUID() == m_Act_Targ && GetActiveSkill() == SKILL_BEGGING )
			return( true );

		Skill_Start( SKILL_BEGGING );
		m_Act_Targ = pChar->GetUID();
		fSpeak = true;
	}
	else
	{
		// We are already begging.
		pChar = m_Act_Targ.CharFind();
		if ( pChar == NULL )
		{
	bailout:
			m_Act_Targ.ClearUID();
			Skill_Start( SKILL_NONE );
			return( false );
		}

		if ( ! CanSee( pChar ))	// can we still see them ?
		{
			if ( GetRandVal(75) > Stat_Get(STAT_INT)) // Dumb beggars think I'm gone
			{
				goto bailout;
			}
		}

		// Time to beg ?
		fSpeak = ! GetRandVal(6);
	}

	SetTimeout( 2*TICK_PER_SEC );

	static const TCHAR * szSpeakBeggar[] =
	{
		"Could thou spare a few coins?",
		"I have a family to feed, think of the children.",
		"Can you spare any gold?",
		"I have children to feed.",
		"I haven't eaten in days.",
		"Please? Just a few coins.",
	};

	UpdateDir( pChar );	// face PC
	if ( fSpeak )
	{
		Speak( szSpeakBeggar[ GetRandVal( COUNTOF( szSpeakBeggar )) ] );
	}

	if ( ! GetRandVal( ( 10100 - pChar->Stat_Get(STAT_Fame)) / 50 ))
	{
		UpdateAnimate( ANIM_BOW );
		return( true );
	}

	return( NPC_Act_Follow( false, 2 ));
}

#if 0

void CChar::NPC_MoveBlockingObject( CPointMap pt )
{
	// we have been blocked from walking to this point.
	// try to move the offending object.

}

#endif

int CChar::NPC_WalkToPoint( bool fRun )
{
	// Move toward my target .
	//
	// RETURN:
	//  0 = we are here.
	//  1 = took the step.
	//  2 = can't take this step right now. (obstacle)

	CPointMap pt = GetTopPoint();
	DIR_TYPE Dir = pt.GetDir( m_Act_p );
	if ( Dir >= DIR_QTY )
		return( 0 );	// we are here.
	pt.Move( Dir );

	if ( ! CanMoveWalkTo(pt))
	{
		// try to step around it ?
		int iDiff = 0;
		int iRand = GetRandVal( 100 );
		if ( iRand < 30 )	// do nothing.
			return( 2 );
		if ( iRand < 35 ) iDiff = 4;	// 5
		else if ( iRand < 40 ) iDiff = 3;	// 10
		else if ( iRand < 65 ) iDiff = 2;
		else iDiff = 1;
		if ( iRand & 1 ) iDiff = -iDiff;
		pt = GetTopPoint();
		Dir = GetDirTurn( Dir, iDiff );
		pt.Move( Dir );
		if ( ! CanMoveWalkTo(pt))
		{
			// ??? Some object in my way that i could move ?
			// Is there something in the way ?
			// try to move it.
			// NPC_MoveBlockingObject(pt);
			return( 2 );
		}
	}


	// ??? Make sure we are not facing a wall.
	m_face_dir = Dir;	// Face this direction.
	if ( fRun && ( ! m_pDef->Can(CAN_C_RUN|CAN_C_FLY) || m_StatStam <= 1 ))
		fRun = false;

	ModStat( STATF_Fly, fRun );

	CPointMap pold = GetTopPoint();
	CheckRevealOnMove();
	MoveTo( pt );
	UpdateMove( pold );
	CheckLocation( false );	// Look for teleports etc.

	// How fast can they move.
	if ( fRun )
	{
		SetTimeout( TICK_PER_SEC/4 + GetRandVal( (100-m_Stat[STAT_DEX])/4 ) * TICK_PER_SEC / 10 );
	}
	else
	{
		SetTimeout( TICK_PER_SEC/2 + GetRandVal( (150-m_Stat[STAT_DEX])/2 ) * TICK_PER_SEC / 10 );
	}

	return( 1 );
}

bool CChar::NPC_MotivationCheck( int iMotivation )
{
	// Am i currently doing something more important ?
	if ( iMotivation < m_pNPC->m_Act_Motivation )
		return( false );
	m_pNPC->m_Act_Motivation = iMotivation;
	return( true );
}

bool CChar::NPC_Script_Command( const TCHAR * pCmd, bool fSystemCheck )
{
	// Does the NPC have a script command by this name ?
	// Execute this command.

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->m_type != ITEM_EQ_NPC_SCRIPT )
			continue;
		if ( strcmpi( pCmd, pItem->GetName()))
			continue;
		NPC_Script_OnTick( dynamic_cast <CItemMessage *>( pItem ), true );
		return true;
	}

	if ( ! fSystemCheck )
		return( false );

	// Assume there is a book id in *BOOK?.SCP
	// Create the book for it.
	CItemMessage *pBook = dynamic_cast <CItemMessage *>( CItem::CreateScript(ITEMID_BOOK1));
	ASSERT(pBook);
	pBook->m_type = ITEM_EQ_NPC_SCRIPT;
	pBook->m_Attr |= ATTR_CAN_DECAY;
	// Does the book id exist in scripts.
	pBook->m_itNPCScript.m_TimeID = Exp_GetVal(pCmd);
	if ( ! pBook->LoadSystemPages())
	{
		pBook->Delete();
		return( false );
	}

	LayerAdd( pBook, LAYER_SPECIAL );
	m_Act_Targ = pBook->GetUID();	// for last target stuff. (trigger stuff)
	NPC_Script_OnTick( pBook );
	return( true );
}

void CChar::NPC_Script_OnTick( CItemMessage * pScriptItem, bool fForce )
{
	// ITEM_EQ_NPC_SCRIPT
	if ( m_pNPC == NULL || pScriptItem == NULL ) 
		return;

	// Default re-eval time.
	pScriptItem->SetTimeout( 5 * 60 * TICK_PER_SEC );

	// Is the script running ?
	if ( ! fForce )
	{
		if ( ! IsStat( STATF_Script_Play ))
		{
			// Should it be running (check time)
			// 24 hour clock per day. ex. 20:01 = 2001 (decimal)
			DWORD dwTOD = GetTopSector()->GetLocalTime() % (24*60);	// Time in minutes.
			DWORD dwStart = pScriptItem->m_itNPCScript.m_TimeStart;
			DWORD dwStop = pScriptItem->m_itNPCScript.m_TimeStop;
			bool fPlayTime;
			if ( dwStart < dwStop )
			{
				fPlayTime = ( dwTOD >= dwStart && dwTOD <= dwStop );
			}
			else // Overnight time.
			{
				fPlayTime = ( dwTOD >= dwStart || dwTOD <= dwStop );
			}
			if ( ! fPlayTime )
			{
				// Check again for the start time.
				// Not exact calculation just in case locale changes ?
				return;
			}

			fForce = true;
		}
		else
		{
			// Is the script that is playing us ?
			if ( ! pScriptItem->m_itNPCScript.m_offset )
				return;
		}
	}

	if ( fForce )
	{
		// Time to play. (continue from last)
		SetStat( STATF_Script_Play );
		Skill_Cleanup();

		if ( pScriptItem->IsBookSystem())
		{
			// Need to load the book.
			if ( ! pScriptItem->LoadSystemPages())
				return;
		}
	}

	// Did the script get superceded by something more important ?
	if ( m_pNPC->m_Act_Motivation > pScriptItem->m_itNPCScript.m_iPriorityLevel )
		return;
	m_pNPC->m_Act_Motivation = pScriptItem->m_itNPCScript.m_iPriorityLevel;

	int iPage = pScriptItem->m_itNPCScript.m_page;
	int iOffset = pScriptItem->m_itNPCScript.m_offset;

	// Sample actions.
	// "GO=123,23,3"			// teleport directly here.
	// "TARG=123,23,3;ACT=goto"	// try to walk to this place.
	// "S=Hello, Welcome to my show!;T=30;M=W;M=W;M=E;M=E;S=Done;ANIM=Bow"

	const TCHAR * pszPage = pScriptItem->GetPageText( iPage );
	if ( pszPage == NULL )
	{
		// Done playing.
play_stop:
		ClearStat( STATF_Script_Play );
		pScriptItem->m_itNPCScript.m_page = 0;
		pScriptItem->m_itNPCScript.m_offset = 0;

		if ( pScriptItem->IsBookSystem())
		{
			// unload the book.
			pScriptItem->UnLoadSystemPages();
		}
		if ( pScriptItem->IsAttr(ATTR_CAN_DECAY))
		{
			pScriptItem->Delete();
		}
		else if ( pScriptItem->GetScriptTimeout() )
		{
			pScriptItem->SetTimeout( pScriptItem->GetScriptTimeout() );
		}
		return;
	}

	// Get the script command
	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	TCHAR * pszVerb = szTemp;
	int len = 0;

restart_read:
	while (true)
	{
		TCHAR ch = pszPage[ iOffset ];
		if ( ch )
		{
			iOffset++;
			if ( ch == '\n' || ch == '\r' || ch == '\t' ) 
				continue;	// ignore these.
			if ( ch == ';' )
			{
				break;	// end of command marker.
			}
			pszVerb[len++] = ch;
		}
		else
		{
			pszPage = pScriptItem->GetPageText( ++iPage );
			if ( pszPage == NULL || pszPage[0] == '\0' )
			{
				if ( ! len ) goto play_stop;
				break;
			}
			iOffset = 0;
		}
	}

	pszVerb[len] = '\0';
	pScriptItem->m_itNPCScript.m_page = iPage;
	pScriptItem->m_itNPCScript.m_offset = iOffset;

	// Execute the action.
	if ( len )
	{
		// Set the default time interval.
		if ( pszVerb[0] == 'T' && pszVerb[1] == '=' )
		{
			pszVerb += 2;
			pScriptItem->SetScriptTimeout( Exp_GetVal(pszVerb)); // store the last time interval here.
			len = 0;
			goto restart_read;
		}
		if ( ! r_Verb( CScript( pszVerb ), this ))
		{
			DEBUG_MSG(( "Bad Book Script verb '%s'\n", pszVerb ));
		}
	}

	// When to check here again.
	pScriptItem->SetTimeout( pScriptItem->GetScriptTimeout());
}

bool CChar::NPC_LookAtCharGuard( CChar * pChar )
{
	// Does the guard hate the target ?

	if ( pChar->IsStat( STATF_INVUL|STATF_DEAD )) 
		return( false );
	if ( ! pChar->IsCriminal()) 
		return false;

	static const TCHAR * szSpeakGuardJeer[] =
	{
		"%s thou art a cowardly swine.",
		"You shall get whats coming to you %s",
		"Evildoers shay pay %s!",
		"Beware of me foul %s",
		"Get thee gone foul %s",
	};

	if ( ! pChar->m_pArea->IsGuarded())
	{
		// At least jeer at the criminal.
		if ( GetRandVal(5)) 
			return( false );
		CGString sMsg;
		sMsg.Format( szSpeakGuardJeer[ GetRandVal( COUNTOF( szSpeakGuardJeer )) ], pChar->GetName());
		Speak( sMsg );
		UpdateDir( pChar );
		return false;
	}

	static const TCHAR * szSpeakGuardStrike[] =
	{
		"Thou shalt regret thine actions, swine!",
		"Death to all evil!",
		"Take this vile criminal!",
		"Evildoers shay pay!",
		"Take this swine!",
	};

	if ( GetTopDist3D( pChar ) > 1 )
	{
		Spell_Teleport( pChar->GetTopPoint(), false, false );
		// If we got intant kill guards enabled, allow the guards
		// to take a swing directly after the teleport.
		if ( g_Serv.m_fGuardsInstantKill )
		{
			Hit( pChar );
		}
	}
	if ( ! IsStat( STATF_War ) || m_Act_Targ != pChar->GetUID())
	{
		Speak( szSpeakGuardStrike[ GetRandVal( COUNTOF( szSpeakGuardStrike )) ] );
		Attack( pChar );
	}
	return( true );
}

bool CChar::NPC_LookAtCharMonster( CChar * pChar )
{
	// return:
	//   true = take new action.
	//   false = continue with any previous action.
	//  motivation level =
	//  0 = not at all.
	//  100 = definitly.
	//

	int iFoodLevel = GetFoodLevelPercent();

	// Attacks those not of my kind.
	if ( ! IsCriminal() && iFoodLevel > 40 )		// I am not evil ?
	{
		return NPC_LookAtCharHuman( pChar );
	}

	// Attack if i am stronger.
	// or i'm just stupid.
	int iActMotivation = NPC_GetAttackMotivation( pChar );
	if ( iActMotivation <= 0 )
		return( false );
	if ( IsStat(STATF_War) && m_Act_Targ == pChar->GetUID())	// same targ.
		return( false );
	if ( iActMotivation < m_pNPC->m_Act_Motivation )
		return( false );

	int iDist = GetTopDist3D( pChar );
	if ( IsStat( STATF_Hidden ) &&
		! NPC_FightMayCast() &&
		iDist > 1 )
		return false;	// element of suprise.

	Attack( pChar );
	m_pNPC->m_Act_Motivation = iActMotivation;
	return true;
}

bool CChar::NPC_LookAtCharHuman( CChar * pChar )
{
	if ( pChar->IsStat( STATF_DEAD ))
		return false;

	if ( IsEvil())		// I am evil.
	{
		// Attack others if we are evil.
		return( NPC_LookAtCharMonster( pChar ));
	}

	if ( ! pChar->IsCriminal())	// not interesting.
		return( false );

	// Yell for guard if we see someone evil.
	if ( NPC_CanSpeak() &&
		m_pArea->IsGuarded() &&
		! GetRandVal( 3 ))
	{
		Speak( pChar->IsStat( STATF_Criminal) ?
			"Help! Guards a Criminal!" :
			"Help! Guards a Monster!" );
		// Find a guard.
		CallGuards();
		if ( IsStat( STATF_War ))
			return( false );

		// run away like a coward.
		m_Act_Targ = pChar->GetUID();
		Skill_Start( NPCACT_FLEE );
		m_pNPC->m_Act_Motivation = 80;
		return true;
	}

	// Attack an evil creature ?

	return( false );
}

bool CChar::NPC_LookAtCharHealer( CChar * pChar )
{
	static const TCHAR * szHealerRefuseEvils[] =
	{
		"Thou hast committed too many unholy acts to be resurrected",
		"Thou hast strayed from the virtues and I shall not resurrect you.",
		"Thy soul is too dark for me to resurrect.",
	};
	static const TCHAR * szHealerRefuseCriminals[] =
	{
		"Thou art a criminal, I shall not resurrect you.",
		"Because of your lawless ways, I shall not help you.",
		"It wouldn't be right to help someone like you.",
	};
	static const TCHAR * szHealerRefuseGoods[] =
	{
		"You are good, why would I help you.",
		"Change thy virtuous ways, and I may consider resurrecting you.",
		"Muhaaahaa, You have got to be kidding, I shall not resurrect such as thee.",
	};
	static const TCHAR * szHealer[] =
	{
		"Thou art dead, but 'tis within my power to resurrect thee.  Live!",
		"Allow me to resurrect thee ghost.  Thy time of true death has not yet come.",
		"Perhaps thou shouldst be more careful.  Here, I shall resurrect thee.",
		"Live again, ghost!  Thy time in this world is not yet done.",
		"I shall attempt to resurrect thee.",
	};

	if ( ! pChar->IsStat( STATF_DEAD ))
		return false;

	const TCHAR * pszRefuseMsg;

	int iDist = GetDist( pChar );
	if ( pChar->IsStat( STATF_Insubstantial ))
	{
		pszRefuseMsg = "I can't see you clearly ghost. Manifest for me to heal thee.";
refuse_to_rez:
		if ( GetRandVal(5) || iDist > 3 )
			return false;
		Speak( pszRefuseMsg );
		return true;
	}

	if ( iDist > 3 )
	{
		if ( GetRandVal(5))
			return false;
		Speak( "You are too far away ghost. Come closer." );
		return( true );
	}

	if ( !IsStat( STATF_Criminal ) && pChar->IsStat( STATF_Criminal ))
	{
		pszRefuseMsg = szHealerRefuseCriminals[ GetRandVal( COUNTOF( szHealerRefuseCriminals )) ];
		goto refuse_to_rez;
	}

	if ( ( !IsNeutral() && !IsEvil()) &&
		( pChar->IsNeutral() || pChar->IsEvil()) )
	{
		pszRefuseMsg = szHealerRefuseEvils[ GetRandVal( COUNTOF( szHealerRefuseEvils )) ];
		goto refuse_to_rez;
	}
 
	if ( ( IsNeutral() || IsEvil() ) &&
		(  !pChar->IsNeutral() && !pChar->IsEvil() ) )
	{
		pszRefuseMsg = szHealerRefuseGoods[ GetRandVal( COUNTOF( szHealerRefuseGoods )) ];
		goto refuse_to_rez;
	}

	// Did u attack me ?
	if ( Memory_FindObjTypes( pChar, MEMORY_FIGHT|MEMORY_AGGREIVED|MEMORY_IRRITATEDBY ))
	{
		pszRefuseMsg = szHealerRefuseCriminals[ GetRandVal( COUNTOF( szHealerRefuseCriminals )) ];
		goto refuse_to_rez;
	}

	// Attempt to res.
	Speak( szHealer[ GetRandVal( COUNTOF( szHealer )) ] );
	UpdateAnimate( ANIM_CAST_AREA );
	if ( ! pChar->OnSpellEffect( SPELL_Resurrection, this, 100 ))
	{
		if ( GetRandVal(2))
			Speak( "I'm sorry but i cannot resurrect you" );
		else
			Speak( "Ah! My magic is failing!" );
	}

	return true;
}

bool CChar::NPC_LookAtItem( CItem * pItem, int iDist )
{
	// I might want to go pickup this item ?
	if ( ! CanSee( pItem ))
		return( false );

	if ( NPC_WantThisItem( pItem ) || pItem->IsContainer())
	{
		// Looks for unconscious or sleeping people.
		// NPCACT_LOOTING
		// NPCACT_EATING
		// Don't do this if we've already looted this item
		if ( Memory_FindObj( pItem ) == NULL )
		{
			m_Act_Targ = pItem->GetUID();
			Skill_Start( NPCACT_LOOTING );
			return true;
		}

		// If i'm a vendor. is it something i would sell/buy ?
	}

	// Am i hungry and this is food ?

	// check for doors we can open.
	if ( Stat_Get(STAT_INT) > 20 &&
		m_pDef->Can(CAN_C_USEHANDS) &&
		pItem->m_type == ITEM_DOOR &&	// not locked.
		GetDist( pItem ) <= 1 &&
		CanTouch( pItem ) &&
		!GetRandVal(2))
	{
		// Is it opened or closed?
		if ( pItem->IsDoorOpen())
			return( false );

		// The door is closed.
		UpdateDir( pItem );
		if ( ! Use_Item( pItem ))	// try to open it.
			return( false );

		// Walk through it
		CPointMap pt = GetTopPoint();
		pt.Move( GetDir( pItem ), 2 );
		if ( CanMoveWalkTo( pt ))
		{
			m_Act_p = pt;
			NPC_WalkToPoint();
			return true;
		}
	}

	return( false );
}

bool CChar::NPC_LookAtChar( CChar * pChar, int iDist )
{
	// I see a char.
	// Do I want to do something to this char (more that what i'm already doing ?)
	// RETURN:
	//   true = yes i do want to take a new action.
	//

	ASSERT( pChar );
	ASSERT( pChar != this );

	if ( ! CanSeeLOS( pChar )) 
		return( false );

	if ( NPC_IsOwnedBy( pChar, false ))
	{
		// pets should protect there owners unless told otherwise.
		if ( pChar->IsStat( STATF_War ))
		{
			if ( Attack( pChar->m_Act_Targ.CharFind()))
				return true;
		}
		if ( m_pNPC->m_Brain != NPCBRAIN_BESERK )
		{
			// follow my owner again. (Default action)
			m_Act_Targ = pChar->GetUID();
			m_atFollowTarg.m_DistMin = 1;
			m_atFollowTarg.m_DistMax = 6;
			m_pNPC->m_Act_Motivation = 50;
			Skill_Start( NPCACT_FOLLOW_TARG );
			return true;
		}
	}

	else
	{
		// initiate a conversation ?
		if ( ! IsStat( STATF_War ) &&
			( GetActiveSkill() == SKILL_NONE || GetActiveSkill() == NPCACT_WANDER ) && // I'm idle
			pChar->m_pPlayer &&
			NPC_CanSpeak() &&
			! Memory_FindObjTypes( pChar, MEMORY_SPEAK ))
		{
			if ( ! OnTrigger( CTRIG_SeeNewPlayer, pChar ))
			{
				// record that we attempted to speak to them.
				CItemMemory * pMemory = Memory_AddObjTypes( pChar, MEMORY_SPEAK );
				pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_FIRSTSPEAK;
				// m_Act_Hear_Unknown = 0;
			}
		}
	}

	switch ( m_pNPC->m_Brain )	// my type of brain
	{
	case NPCBRAIN_GUARD:
		// Guards should look around for criminals or nasty creatures.
		if ( NPC_LookAtCharGuard( pChar ))
			return true;
		break;

	case NPCBRAIN_BEGGAR:
		if ( NPC_Act_Begging( pChar ))
			return true;
		if ( NPC_LookAtCharHuman( pChar ))
			return( true );
		break;

	case NPCBRAIN_MONSTER:
	case NPCBRAIN_UNDEAD:
	case NPCBRAIN_DRAGON:
		if ( NPC_LookAtCharMonster( pChar ))
			return( true );
		break;

	case NPCBRAIN_BESERK:
		// Blades or EV.
		// ??? Attack everyone you touch !
		if ( iDist <= 1 )
		{
			Hit( pChar );
		}
		if ( IsStat( STATF_War )) // Is this a better target than my last ?
		{
			CChar * pCharTarg = m_Act_Targ.CharFind();
			if ( pCharTarg != NULL )
			{
				if ( iDist >= GetTopDist3D( pCharTarg ))
					break;
			}
		}
		// if ( ! NPC_MotivationCheck( 50 )) continue;
		Attack( pChar );
		break;

	case NPCBRAIN_HEALER:
		// Healers should look around for ghosts.
		if ( NPC_LookAtCharHealer( pChar ))
			return( true );
		if ( NPC_LookAtCharHuman( pChar ))
			return( true );
		break;

	case NPCBRAIN_CRIER:
	case NPCBRAIN_BANKER:
	case NPCBRAIN_VENDOR:
	case NPCBRAIN_STABLE:
	case NPCBRAIN_ANIMAL:
	case NPCBRAIN_HUMAN:
	case NPCBRAIN_THEIF:
		if ( NPC_LookAtCharHuman( pChar ))
			return( true );
		break;
	}

	return( false );
}

bool CChar::NPC_LookAround()
{
	// Take a look around for other people/chars.
	// We may be doing something already. So check current action motivation level.
	// RETURN: 
	//   true = found something better to do.

	if ( m_pNPC->m_Brain == NPCBRAIN_BESERK || ! GetRandVal(5))
	{
		// Make some random noise
		//
		SoundChar( GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 );
	}

	int iRange = UO_MAP_VIEW_SIGHT;
	int iRangeBlur = iRange;

	// If I can't move don't look to far.
	if ( ! m_pDef->Can( CAN_C_WALK | CAN_C_FLY | CAN_C_SWIM ) ||
		IsStat( STATF_Stone | STATF_Freeze ))
	{
		// I'm NOT mobile.
		if ( ! NPC_FightMayCast())	// And i have no distance attack.
		{
			iRange = 2;
			iRangeBlur = 2;
		}
	}
	else
	{
		// I'm mobile.
		if ( ! NPC_CheckWalkHere( GetTopPoint(), m_pArea ))
		{
			// I should move. Someone lit a fire under me.
			m_Act_p = GetTopPoint();
			m_Act_p.Move( (DIR_TYPE) GetRandVal( DIR_QTY ));
			NPC_WalkToPoint( true );
			return( true );
		}

		if ( Stat_Get(STAT_INT) < 50 ) iRangeBlur = iRange/2;
	}

#if 0
	// Lower the number of chars we look at if complex.
	if ( GetTopPoint().GetSector()->GetComplexity() > g_Serv.m_iMaxComplexity / 4 )
		iRange /= 4;
#endif

	CWorldSearch Area( GetTopPoint(), iRange );
	while (true)
	{
		CChar * pChar = Area.GetChar();
		if ( pChar == NULL ) 
			break;
		if ( pChar == this )	// just myself.
			continue;

		int iDist = GetTopDist3D( pChar );
		if ( iDist > iRangeBlur && ! pChar->IsStat(STATF_Fly))
		{
			if ( GetRandVal(iDist)) 
				continue;	// can't see them.
		}
		if ( NPC_LookAtChar( pChar, iDist ))
			return( true );
	}

	// Check the ground for good stuff.
	if ( Stat_Get(STAT_INT) > 10 &&
		! IsSkillBase( GetActiveSkill()) &&
		! GetRandVal( 3 ))
	{
		CWorldSearch Area( GetTopPoint(), iRange );
		while (true)
		{
			CItem * pItem = Area.GetItem();
			if ( pItem == NULL )
				break;
			int iDist = GetTopDist3D( pItem );
			if ( iDist > iRangeBlur )
			{
				if ( GetRandVal(iDist)) 
					continue;	// can't see them.
			}
			if ( NPC_LookAtItem( pItem, iDist ))
				return( true );
		}
	}

	// Move stuff that is in our way ? (chests etc.)

	return m_pNPC->m_Act_Motivation; // Is this a better target than my last ?
}

void CChar::NPC_Act_Wander()
{
	// NPCACT_WANDER
	// Stop wandering and re-eval frequently

	if ( ! GetRandVal( 7 + ( m_StatStam / 30 )))
	{
		// Stop wandering ?
		Skill_Start( SKILL_NONE );
		return;
	}

	if ( GetRandVal( 2 ))
	{
		if ( NPC_LookAround())
			return;
	}

	// Staggering Walk around.
	m_Act_p = GetTopPoint();
	m_Act_p.Move( GetDirTurn( m_face_dir, 1 - GetRandVal(3)));

	if ( m_pNPC->m_Home_Dist_Wander && m_Home.IsValid())
	{
		if ( m_Act_p.GetDist( m_Home ) > m_pNPC->m_Home_Dist_Wander )
		{
			Skill_Start( NPCACT_GO_HOME );
			return;
		}
	}

	NPC_WalkToPoint();
}

bool CChar::NPC_Act_Follow( bool fFlee, int maxDistance, bool forceDistance )
{
	// Follow our target or owner. (m_Act_Targ) we may be fighting.
	// false = can't follow any more.

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL )
	{
		// free to do as i wish !
		Skill_Start( SKILL_NONE );
		return( false );
	}

	// Have to be able to see target to follow.
	if ( CanSee( pChar ))
	{
		if ( fFlee )
		{
			m_Act_p = GetTopPoint();
			m_Act_p.Move( GetDirTurn( GetDir( pChar ), 4 + 1 - GetRandVal(3)));
			NPC_WalkToPoint( GetDist( pChar ) < GetRandVal(10));
			return( true );
		}
		m_Act_p = pChar->GetTopPoint();
	}
	else
	{
		// Monster may get confused because he can't see you.
		// There is a chance they could forget about you if hidden for a while.
		if ( ! GetRandVal( 1 + (( 100 - Stat_Get(STAT_INT)) / 20 )))
			return( false );
		if ( fFlee ) return( false );
	}

	int dist = GetTopPoint().GetDist( m_Act_p );
	if ( forceDistance )
	{
		if ( dist < maxDistance )
		{
			// start moving away
			m_Act_p = GetTopPoint();
			m_Act_p.Move( GetDirTurn( GetDir( pChar ), 4 + 1 - GetRandVal(3)));
		}
	}
	else
	{
		// just too far away.
		if ( dist <= maxDistance/*1*/ )
			return( true );
		if ( dist > UO_MAP_VIEW_RADAR*2 )
			return( false );
	}

	NPC_WalkToPoint( IsStat( STATF_War ) ? true : ( dist > 3 ));
	return( true );
}

bool CChar::NPC_FightCast( CChar * pChar )
{
	// cast a spell if i can ?
	// RETURN: 
	//  false = revert to melee type fighting.

	ASSERT( pChar );
	if ( ! NPC_FightMayCast())
		return( false );

	int iDist = GetTopDist3D( pChar );
	if ( iDist > ((UO_MAP_VIEW_SIGHT*3)/4))
		return( false );

	if ( iDist <= 1 &&
		Skill_GetBase(SKILL_TACTICS) > 200 &&
		! GetRandVal(2))
	{
		// Within striking distance.
		// Stand and fight for a bit.
		return( false );
	}

	// KRJ
	// A creature with a greater amount of mana will have a greater
	// chance of casting
	int iStatInt = Stat_Get(STAT_INT);
	int iSkillVal = Skill_GetBase(SKILL_MAGERY);
	int iChance = iSkillVal +
		(( m_StatMana >= ( iStatInt / 2 )) ? m_StatMana : ( iStatInt - m_StatMana ));
	if ( GetRandVal( iChance ) < 400 )
	{
		// we failed this test, but we could be casting next time
		// back off from the target a bit
		if ( m_StatMana > ( iStatInt / 3 ) && GetRandVal( iStatInt ))
		{
			if ( iDist < 4 || iDist > 8  )	// Here is fine?
			{
				NPC_Act_Follow( false, GetRandVal( 3 ) + 2, true );
			}
			return( true );
		}
		return( false );
	}

	// select proper spell.
	// defensive spells ???
	int imaxspell = min(( iSkillVal / 12 ) * 8, SPELL_BASE_QTY );

	// does the creature have a spellbook.
	CItem * pSpellbook = GetSpellbook();
	SPELL_TYPE spell;

	for ( int i = GetRandVal( imaxspell ); 1; i++ )
	{
		if ( i >= imaxspell )	// didn't find a spell.
			return( false );

		spell = (SPELL_TYPE) i;
		const CSpellDef * pDef = g_Serv.m_SpellDefs[i];
		if ( pSpellbook )
		{
			if ( ! pSpellbook->IsSpellInBook(spell))
				continue;

			if ( ! pDef->IsSpellType( SPELLFLAG_HARM ))
			{
				// Heal, enhance myself, help friends or dispel conjured creatures.
			}
		}
		else
		{
			if ( ! pDef->IsSpellType( SPELLFLAG_HARM ))
				continue;

			// less chance for berserker spells
			if ( pDef->IsSpellType( SPELLFLAG_SUMMON ) && GetRandVal( 2 ))
				continue;

			// less chance for field spells as well
			if ( pDef->IsSpellType( SPELLFLAG_FIELD ) && GetRandVal( 4 ))
				continue;
		}

		if ( ! Spell_CanCast( spell, true, this, false ))
			continue;

		break;	// I like this spell.
	}

	// KRJ - give us some distance
	// if the opponent is using melee
	// the bigger the disadvantage we have in hitpoints, the further we will go
	if ( m_StatMana > iStatInt / 3 && GetRandVal ( iStatInt << 1 ))
	{
		if ( iDist < 4 || iDist > 8  )	// Here is fine?
		{
			NPC_Act_Follow( false, 5, true );
		}
	}
	else
	{
		NPC_Act_Follow();
	}

	return( Spell_Cast( spell, m_Act_Targ, pChar->GetTopPoint()));
}

void CChar::NPC_Act_Fight()
{
	// I am in an attack mode.
	DEBUG_CHECK( IsTopLevel());
	ASSERT( m_pNPC );

	// Review our targets periodically.
	if ( ! IsStat(STATF_Pet) || m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		int iObservant = ( 130 - Stat_Get(STAT_INT)) / 20;
		if ( ! GetRandVal( 2 + max( 0, iObservant )))
		{
			if ( NPC_LookAround())
				return;
		}
	}

	CChar * pChar = m_Act_Targ.CharFind();
	if ( pChar == NULL ) 
		return;
	int iDist = GetDist( pChar );

	if ( m_pNPC->m_Brain == NPCBRAIN_GUARD &&
		m_atFight.m_War_Swing_State == WAR_SWING_READY &&
		! GetRandVal(3))
	{
		// If a guard is ever too far away (missed a chance to swing)
		// Teleport me closer.
		NPC_LookAtChar( pChar, iDist );
	}

	// If i'm horribly damaged and smart enough to know it.
	if ( ! IsStat(STATF_Pet) && NPC_GetAttackMotivation( pChar ) < 0 )
	{
		Skill_Start( NPCACT_FLEE );	// Run away!
		return;
	}

	// Can only do that with full stamina !
	if ( m_StatStam >= Stat_Get(STAT_DEX))
	{
		// If I am a dragon maybe I will breath fire.
		if ( m_pNPC->m_Brain == NPCBRAIN_DRAGON &&
			iDist >= 1 &&
			iDist <= 8 &&
			CanSeeLOS( pChar ))
		{
			UpdateDir( pChar );
			Skill_Start( NPCACT_BREATH );
			return;
		}

		// If I am a giant. i can throw stones.

		if (( GetDispID() == CREID_OGRE ||
			GetDispID() == CREID_ETTIN ||
			GetDispID() == CREID_Cyclops ) &&
			iDist >= 2 &&
			iDist <= 9 &&
			CanSeeLOS( pChar ) &&
			ContentFindType( ITEM_AROCK, 0, 2 ))
		{
			UpdateDir( pChar );
			Skill_Start( NPCACT_THROWING );
			return;
		}
	}

	// Maybe i'll cast a spell if I can. if so maintain a distance.
	if ( NPC_FightCast( pChar ))
		return;

	// Move in for melee type combat.
	NPC_Act_Follow();
}

void CChar::NPC_Act_GoHome()
{
	// NPCACT_GO_HOME
	// If our home is not valid then

	if ( ! GetRandVal( 2 ))
	{
		if ( NPC_LookAround())
			return;
	}

	if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )
	{
		// Had to change this guards were still roaming the forests
		// this goes hand in hand with the change that guards arent
		// called if the criminal makes it outside guarded territory.

		CRegionBase * pArea = m_Home.GetRegion( REGION_TYPE_AREA );
		if ( pArea && pArea->IsGuarded())
		{
			if ( ! m_pArea->IsGuarded())
			{
				if ( Spell_Teleport( m_Home, false, false ))
				{
					Skill_Start( SKILL_NONE );
					return;
				}
			}
		}
		else
		{
			g_Log.Event( LOGL_WARN, "Guard 0%lx '%s' has no guard post (%s)!\n", GetUID(), GetName(), GetTopPoint().Write());

			// If we arent conjured and still got no valid home
			// then set our status to conjured and take our life.
			if ( ! IsStat(STATF_Conjured))
			{
				SetStat( STATF_Conjured );
				m_StatHealth = -1;
				return;
			}
		}

		// else we are conjured and probably a timer started already.
	}

	ASSERT( m_Home.IsValid());// NOT PRE-CHGECKED FOR GUARDS.

   	m_Act_p = m_Home;
   	if ( ! NPC_WalkToPoint()) // get there
   	{
   		Skill_Start( SKILL_NONE );
		return;
	}

	if ( ! GetRandVal(40)) // give up...
	{
		// Some NPCs should be able to just teleport back home...
		switch( this->m_pNPC->m_Brain )
		{
		case NPCBRAIN_VENDOR:
		case NPCBRAIN_STABLE:
		case NPCBRAIN_BANKER:
		case NPCBRAIN_HEALER:
		case NPCBRAIN_CRIER:
			// if ( ! GetRandVal(100)) // give up...
			{
				Spell_Teleport( m_Home, true, false );
			}
		default:
			Skill_Start( SKILL_NONE );
			break;
		}
	}
}

void CChar::NPC_LootMemory( CItem * pItem )
{
	// Create a memory of this item.
	// I have already looked at it.
	CItem * pMemory = Memory_AddObjTypes( pItem, MEMORY_SPEAK );
	ASSERT(pMemory);
	pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_IGNORE;

	// If the item is set to decay.
	if ( pItem->IsTimerSet() && ! pItem->IsTimerExpired())
	{
		pMemory->SetTimeout( pItem->GetTimerDiff());	// We'll forget about this once the item is gone
	}
}

bool CChar::NPC_LootContainer( CItemContainer * pContainer )
{
	// Go through the pack and see if there is anything I want there...
	CItem * pNext;
	CItem * pLoot = pContainer->GetContentHead();
	for ( ; pLoot != NULL; pLoot = pNext )
	{
		pNext = pLoot->GetNext();
		// Have I checked this one already?
		if ( Memory_FindObj( pLoot ))
			continue;

		if ( pLoot->IsContainer())
		{
			// Loot it as well
			if ( ! NPC_LootContainer( dynamic_cast <CItemContainer *> (pLoot)))
			{
				// Not finished with it
				return false;
			}
		}

		if ( ! NPC_WantThisItem( pLoot ))
			continue;

		// How much can I carry
		if ( CanCarry( pLoot ))
		{
			ItemEquip( pLoot );
		}
		else
		{
			// Can't carry the whole stack...how much can I carry?
			NPC_LootMemory( pLoot );
		}

		// I can only pick up one thing at a time, so come back here on my next tick
		SetTimeout( 1 );	// Don't go through things so fast.
		return false;
	}

	// I've gone through everything here...remember that we've looted this container
	NPC_LootMemory( pContainer );
	return true;
}

void CChar::NPC_Act_Looting()
{
	// We have seen something good that we want. checking it out now.
	// We just killed something, so we should see if it has anything interesting on it.
	// Find the corpse first

	CItem * pWantItem = NULL;
	CWorldSearch AreaItems( GetTopPoint(), 10 );
  	while (true)
  	{
		CItem * pItem = AreaItems.GetItem();
		if (pItem == NULL)
			break;

		// Can I even see it?
		if ( ! CanSee( pItem ))
		{
			continue;
		}
		// Do I want this?
		if ( ! NPC_WantThisItem( pItem ))
		{
			continue;
		}

		if ( pItem->IsContainer() && pItem->GetDispID() != ITEMID_CORPSE )
		{
			continue;	// I only loot from corpses
		}

		// Have I already searched this one?
		CItemMemory * pMemory = Memory_FindObj( pItem );
		if ( pMemory != NULL )
		{
			continue;
		}

		if ( pItem->GetDispID() == ITEMID_CORPSE )
		{
			// Did I kill this one?
			if ( pItem->m_itCorpse.m_uidKiller != GetUID())
			{
				// Wasn't me...less chance of actually looting it.
				if ( GetRandVal( 4 ))
				{
					NPC_LootMemory( pItem );
					continue;
				}
			}
		}

		// Is it right here?
		if ( GetDist( pItem ) > 1 )	// move toward it.
		{
			if ( pWantItem == NULL )
				pWantItem = pItem;
			else if ( GetDist( pWantItem ) > GetDist( pItem ))
				pWantItem = pItem;
			continue;
		}

		// Can i reach the object that i want ?
		if ( ! CanTouch( pItem ))
		{
			continue;
		}

		// I can reach it
		CItemCorpse * pCorpse = dynamic_cast <CItemCorpse *> ( pItem );
		if ( pCorpse )
		{
			if ( ! NPC_LootContainer( pCorpse ))
				return;
			if ( m_pNPC->m_Brain == NPCBRAIN_MONSTER ||
				m_pNPC->m_Brain == NPCBRAIN_ANIMAL ||
				m_pNPC->m_Brain == NPCBRAIN_DRAGON )
			{
				// Only do this if it has a resource type we want...
				if ( pCorpse->m_itCorpse.m_tDeathTime )
				{
					Use_CarveCorpse( pCorpse );
					return;
				}
			}
		}
		else
		{
			// This must be some type of food.
			if ( CanCarry( pItem ))
			{
				ItemBounce( pItem );
				return;
			}

			NPC_LootMemory( pItem );
			return;
		}
	}

	if ( pWantItem != NULL )
	{
		// Go there
		m_Act_p = pWantItem->GetTopPoint();
		NPC_WalkToPoint();
		return;
	}

	// Done looting
	// We might be looting this becuase we are hungry...
	// What was I doing before this?

	Skill_Start( SKILL_NONE );
}

void CChar::NPC_Act_Idle()
{
	// Free to do as we please. decide what we want to do next.
	// Idle NPC's should try to take some action.

	if ( IsStat( STATF_Script_Play ))
	{
		// Just wait for the next command.
		return;
	}

	m_pNPC->m_Act_Motivation = 0;	// we have no motivation to do anything.

	// Look around for things to do.
	if ( NPC_LookAround())
		return;

	// ---------- If we found nothing else to do. do this. -----------

	if ( IsStat( STATF_War ))
	{
		if ( Attack( m_Act_Targ.CharFind()))
			return;
	}

	// If guards are found outside guarded territories, do the following.
	if ( m_pNPC->m_Brain == NPCBRAIN_GUARD && ! m_pArea->IsGuarded())
	{
		Skill_Start( NPCACT_GO_HOME );
		return;
	}

	// Specific creature actions.
	if ( m_StatStam >= Stat_Get(STAT_DEX) && ! GetRandVal( 3 ))
	{
		switch ( GetDispID())
		{
		case CREID_FIRE_ELEM:
			if ( ! g_World.IsItemTypeNear( GetTopPoint(), ITEM_FIRE ))
			{
				Action_StartSpecial( CREID_FIRE_ELEM );
			}
			return;
		case CREID_GIANT_SPIDER:
			if ( ! g_World.IsItemTypeNear( GetTopPoint(), ITEM_WEB ))
			{
				Action_StartSpecial( CREID_GIANT_SPIDER );
			}
			return;
		}
	}

	if ( m_Home.IsValid() && ! GetRandVal( 20 ))
	{
		// Periodically head home.
		Skill_Start( NPCACT_GO_HOME );
		return;
	}

	if ( Skill_GetBase(SKILL_HIDING) > 30 &&
		! GetRandVal( 15 - Skill_GetBase(SKILL_HIDING)/100) &&
		! m_pArea->IsGuarded())
	{
		// Just hide here.
		if ( IsStat( STATF_Hidden )) 
			return;
		Skill_Start( SKILL_HIDING );
		return;
	}
	if ( GetRandVal( 100 - Stat_Get(STAT_DEX)) < 25 )
	{
		// dex determines how jumpy they are.
		// Decide to wander about ?
		Skill_Start( NPCACT_WANDER );
		return;
	}

	// just stand here for a bit.
	SetTimeout( GetRandVal(3*TICK_PER_SEC));
}

bool CChar::NPC_OnItemGive( CChar * pCharSrc, CItem * pItem )
{
	// Someone (Player) is giving me an item.
	// RETURN: true = accepted.

	ASSERT( pCharSrc );

	if ( m_pNPC == NULL )
		return( false );	// must just be an offline player.

	if ( OnTrigger( CTRIG_ReceiveItem, pCharSrc, pItem->GetDispID() ))
		return( true );

	if ( NPC_IsOwnedBy( pCharSrc ))
	{
		// Giving something to my own pet.
		if ( pCharSrc->IsPriv( PRIV_GM ))
		{
			return( ItemEquip( pItem ) );
		}

		// Take stuff from my master.
		if ( m_pNPC->IsVendor())
		{
			if ( pItem->GetID() == ITEMID_GOLD_C1 )
			{
				Speak( "I will keep this money in account." );
				NPC_OnHirePayMore(pItem);
			}
			else
			{
				Speak( "I will attempt to sell this item for you." );
				GetBank(LAYER_VENDOR_STOCK)->ContentAdd( pItem );
			}
			return true;
		}

		// Human hireling
		if ( NPC_CanSpeak())
		{
			if ( pItem->m_type == ITEM_FOOD )
			{
				if ( NPC_WantThisItem( pItem ) &&
					Use_Eat( pItem ))
				{
					Speak( "Mmm. Thank you." );
					return( true );
				}
				else
				{
					Speak( "No thank you." );
					return( false );
				}
			}
			if ( ! CanCarry( pItem ))
			{
				Speak( "I'm too weak to carry that." );
				return( false );
			}
			if ( pItem->GetID() == ITEMID_GOLD_C1 )
			{
				// Pay me for my work ? (hirelings)
				Speak( "Thank thee." );
				// NPC_OnHirePayMore(pItem);
				// return;
			}

			if ( Use_Item( pItem ))
				return true;

			Speak( "Tell me to drop this when you want it back." );
			GetPackSafe()->ContentAdd( pItem );
			return( true );
		}

		switch ( pItem->m_type )
		{
		case ITEM_POTION:
		case ITEM_DRINK:
		case ITEM_BOOZE:
			if ( Use_Item( pItem ))
				return true;
		}
	}

	// Does the NPC know you ?
	CItemMemory * pMemory = Memory_FindObj( pCharSrc );
	if ( pMemory )
	{
		// Am i being paid for something ?
		if ( pItem->GetID() == ITEMID_GOLD_C1 )
		{
			if ( pMemory->m_itEqMemory.m_Action == NPC_MEM_ACT_SPEAK_TRAIN )
			{
				return( NPC_OnTrainPay( pCharSrc, pMemory, pItem ));
			}
			if ( pMemory->m_itEqMemory.m_Action == NPC_MEM_ACT_SPEAK_HIRE )
			{
				return( NPC_OnHirePay( pCharSrc, pMemory, pItem ));
			}
		}
	}

	if ( NPC_WantThisItem( pItem ))
	{

	if ( pItem->GetID() == ITEMID_GOLD_C1 )
	{
		if ( m_pNPC->m_Brain == NPCBRAIN_BANKER )
		{
			// I shall put this item in your bank account.
			if ( pItem->GetID() == ITEMID_GOLD_C1 )
			{
				CGString sMsg;
				sMsg.Format( "I shall deposit %d gold in your account", pItem->GetAmount());
				Speak( sMsg );
				pCharSrc->GetBank()->ContentAdd( pItem );
				return( true );
			}
		}

			if ( NPC_CanSpeak())
			{
				Speak( "Gold is always welcome. thank thee." );
			}
			ItemEquip( pItem );
			pItem->Update();
			return( true );
		}
	}

	// The NPC might want it ?
	switch ( m_pNPC->m_Brain )
	{
	case NPCBRAIN_NONE:
	case NPCBRAIN_BESERK:
		break;

	case NPCBRAIN_GUARD:
	case NPCBRAIN_CRIER:
		Speak( "I cannot be bribed. be gone." );
		return( false );

	case NPCBRAIN_HUMAN:
		Speak( "I don't want this thank you" );
		return( false );

	case NPCBRAIN_BANKER:
		// I shall put this item in your bank account.
		Speak( "I don't want this thank you" );
		return( false );

	case NPCBRAIN_VENDOR:
	case NPCBRAIN_STABLE:
	case NPCBRAIN_HEALER:
		Speak( "To trade with me, please say 'vendor sell'" );
		// ??? I would offer you xxx for this ?
		return( false );

	case NPCBRAIN_UNDEAD:
		// They only want corpse parts.
		break;

	case NPCBRAIN_MONSTER:
		// Monster may have a weird view of food.
		break;

	case NPCBRAIN_DRAGON:
	case NPCBRAIN_ANIMAL:
		// Might want food ?
		if ( pItem->m_type == ITEM_FOOD
			|| pItem->m_type == ITEM_FOOD_RAW )
		{
			// ??? May know it is poisoned ?
			// ??? May not be hungry
			if ( Use_Eat( pItem )) 
				return( true );
		}
		break;

	case NPCBRAIN_BEGGAR:
	case NPCBRAIN_THEIF:
		if ( pItem->m_type == ITEM_FOOD && 
			Use_Eat( pItem ))
		{
			Speak( "Mmm thank you. I was very hungry." );
			return( true );
		}
		if ( ! CanCarry( pItem ))
		{
			Speak( "I'm too weak to carry that." );
			return( false );
		}
		if ( pItem->GetID() == ITEMID_GOLD_C1 ||
			pItem->m_type == ITEM_FOOD )
			Speak( "Thank thee! Now I can feed my children!" );
		else
			Speak( "I'll sell this for gold! Thank thee!" );
		ItemEquip( pItem );
		pItem->Update();
		if (m_Act_Targ == pCharSrc->GetUID())
		{
			Speak( "If only others were as kind as you." );
			m_Act_TargPrv = m_Act_Targ;
			m_Act_Targ.ClearUID();
			Skill_Start( SKILL_NONE );
		}
		return( true );
	}

	pCharSrc->m_pClient->addObjMessage( "They don't appear to want the item", this );
	return( false );
}

bool CChar::NPC_OnFoodTick( int nFoodLevel )
{
	// Check Food usage.
	// Are we hungry enough to take some new action ?
	// RETURN: true = we have taken an action.

   	if ( IsStat( STATF_Pet ))
   	{
   		CGString sMsg;
   		sMsg.Format( "looks %s", GetFoodLevelMessage( true, false ));
   		Emote( sMsg, GetClient());
   		SoundChar( CRESND_RAND2 );

		if ( nFoodLevel <= 0 )
		{
			// How happy are we with being a pet ?
			CChar * pCharOwn = NPC_GetOwner();
			if ( pCharOwn && ! pCharOwn->CanSee(this))
			{
				pCharOwn->SysMessagef( "You sense that %s has deserted you.", GetName());
			}

			NPC_ClearOwners();

			CGString sMsg;
			sMsg.Format( "%s decides they are better off as their own master.", GetName());
			Speak( sMsg );

			// free to do as i wish !
			Skill_Start( SKILL_NONE );
			return true;
		}
   	}

	return( false );	// This code appears to cause serious problems ! cut it out for now.

#if 0
	SoundChar( CRESND_RAND2 );

	switch ( GetActiveSkill())
	{
	case NPCACT_EATING:
  		return false;	// We are already looking for food;
 	case NPCACT_WANDER:
  	case NPCACT_TALK:
  	case NPCACT_TALK_FOLLOW:
  	case NPCACT_GO_HOME:
  	case NPCACT_UNUSED:
  		// OK to eat now
  		break;
  	case NPCACT_GOTO:
  	case NPCACT_GUARD_TARG:
  	case NPCACT_GO_FETCH:
  	case NPCACT_FOLLOW_TARG:
  	case NPCACT_STAY:
  		if (nFoodLevel > 25)
  			return false;	// Only if very hungry
  		break;
  	case NPCACT_FLEE:
  	case NPCACT_BREATH:
  	case NPCACT_LOOTING:
  		if (nFoodLevel > 12)
  			return false; // Only if starving...some things are more important
  		break;
  	case NPCACT_RIDDEN:
  		return false;	// I guess I can't do this now....
  	default:
  		return false ;	// I'm doing something else (perhaps fighting?)
	}

	if ( IsStat( STATF_Stone | STATF_Freeze | STATF_DEAD | STATF_Sleeping ))
		return false;

  	m_Act_TargPrv = m_Act_Targ;
  	Skill_Start( NPCACT_EATING );

  	NPC_Food_Search();
	return( true );
#endif

}

void CChar::NPC_OnTickAction()
{
	// Our action timer has expired.
	// last skill or task might be complete ?
	// What action should we take now ? 

	switch ( GetActiveSkill())
	{
	case SKILL_NONE:
		// We should try to do something new.
		NPC_Act_Idle();
		return;

	case SKILL_BEGGING:
		NPC_Act_Begging( NULL );
		return;

	case SKILL_Stealth:
	case SKILL_HIDING:
		// We are currently hidden.
		if ( NPC_LookAround()) 
			return;
		// just remain hidden unless we find something new to do.
		if ( GetRandVal( Skill_GetBase(SKILL_HIDING))) 
			return;
		NPC_Act_Idle();
		return;

	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:
		// If we are fighting . Periodically review our targets.
		NPC_Act_Fight();
		break;

	case NPCACT_FOLLOW_TARG:
		// continue to follow our target.
		NPC_Act_Follow();
		break;
	case NPCACT_STAY:
		// Just stay here til told to do otherwise.
		break;
	case NPCACT_GOTO:
		// Still trying to get to this point.
		switch ( NPC_WalkToPoint())
		{
		case 0:
			// We reached our destination
			NPC_Act_Idle();	// look for something new to do.
			break;
		case 1:
			// Took a step....keep trying to get there.
			break;
		case 2:
			// Give it up...
			// Go directly there...
			if ( m_Act_p.IsValid() && IsHuman() && !IsStat( STATF_Freeze|STATF_Stone ))
				Spell_Teleport( m_Act_p, true, false);
			else
				NPC_Act_Idle();	// look for something new to do.
			break;
		}
		break;
	case NPCACT_WANDER:
		// just wander aimlessly. (but within bounds)
		NPC_Act_Wander();
		break;
	case NPCACT_FLEE:
		// I should move faster this way.
		// ??? turn to strike if they are close.
		if ( ! GetRandVal( 10 ))	// I'm done running.
		{
			Skill_Start( SKILL_NONE );
			return;
		}
		NPC_Act_Follow( true );
		break;

	case NPCACT_TALK:
	case NPCACT_TALK_FOLLOW:
		// Got bored just talking to you.
		if ( NPC_CanSpeak())
		{
			static const TCHAR * szText[] =
			{
				"Well it was nice speaking to you %s but i must go about my business",
				"Nice speaking to you %s",
			};
			CChar * pChar = m_Act_Targ.CharFind();
			const TCHAR * pszName = ( pChar == NULL ) ? "" : pChar->GetName();
			CGString sMsg;
			sMsg.Format( szText[ GetRandVal( COUNTOF( szText )) ], pszName );
			Speak( sMsg );
		}
		NPC_Act_Idle();	// look for something new to do.
		break;

	case NPCACT_GO_HOME:
		NPC_Act_GoHome();
		break;

	case NPCACT_EATING:
		NPC_Act_Eating();
		break;

	case NPCACT_LOOTING:
		NPC_Act_Looting();
		break;

	default:
		if ( ! IsSkillBase( GetActiveSkill()))	// unassigned skill ? that's weird
		{
			Skill_Start( SKILL_NONE );
		}
	}
}
