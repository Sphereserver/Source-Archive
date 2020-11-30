//
// CCharNPCPet.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Actions specific to an NPC.
//

#include "graysvr.h"	// predef header.

void CChar::NPC_OnPetCommand( bool fSuccess, CChar * pMaster )
{
	// i take a command from my master.

	if ( NPC_CanSpeak())
	{
		Speak( fSuccess? "Yes Master" : "Sorry" );
	}
	else
	{
		SoundChar( fSuccess? CRESND_RAND1 : CRESND_RAND2 );
	}
}

enum 
{
	PC_ATTACK,
	PC_CASH,
	PC_COME,
	PC_DISMISS,
	PC_DROP,	// "GIVE" ?
	PC_DROP_ALL,
	PC_FETCH,
	PC_FOLLOW,
	PC_FOLLOW_ME,
	PC_FRIEND,
	PC_GO,
	PC_GUARD,
	PC_GUARD_ME,
	PC_KILL,
	PC_PRICE,
	PC_RELEASE,
	PC_SPEAK,
	PC_STATUS,
	PC_STAY,
	PC_STAY_HERE,
	PC_STOP,
	PC_TRANSFER,
};

bool CChar::NPC_OnHearPetCmd( const TCHAR * pszCmd, CChar * pSrc )
{
	// We own this char (pet or hireling)
	// pObjTarget = the m_ActTarg has been set for them to attack.
	// RETURN:
	//  true = we understand this.
	//  false = this is not a command we know.
	//  if ( GetTargMode() == TARGMODE_PET_CMD ) it needs a target.

	static const TCHAR * Pet_table[] =
	{
		"ATTACK",
		"CASH",
		"COME",
		"DISMISS",
		"DROP",	// "GIVE" ?
		"DROP ALL",
		"FETCH",
		"FOLLOW",
		"FOLLOW ME",
		"FRIEND",
		"GO",
		"GUARD",
		"GUARD ME",
		"KILL",
		"PRICE",
		"RELEASE",
		"SPEAK",
		"STATUS",
		"STAY",
		"STAY HERE",
		"STOP",
		"TRANSFER",
	};

	if ( m_pNPC == NULL ||
		! NPC_IsOwnedBy( pSrc ) ||
		m_pPlayer ||
		m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		return( false );	// take no commands
	}

	// Do we have a script command to follow ?
	if ( NPC_Script_Command( pszCmd, false ))
		return( true );

	int iCmd = FindTableHead( pszCmd, Pet_table, COUNTOF(Pet_table));
	if ( iCmd < 0 )
	{
		return( false );
	}

	bool fTargAllowGround = false;
	const TCHAR * pTargPrompt = NULL;

	switch ( iCmd )
	{
	case PC_ATTACK:
do_attack:
		pTargPrompt = "Who do you want to attack?";
		break;
	case PC_CASH:
		// Drop all we have.
		if ( ! m_pNPC->IsVendor())
			return( false );
		{
			// Give up my cash total.
			CItemContainer * pBank = GetBank();
			int iWage = m_pDef->GetHireDayWage();
			CGString sMsg;
			if ( pBank->m_itEqBankBox.m_Check_Amount > iWage )
			{
				sMsg.Format( "Here is %d gold. I will keep 1 days wage on hand. To get any items say 'Inventory'",
					pBank->m_itEqBankBox.m_Check_Amount - iWage );
				pSrc->AddGoldToPack( pBank->m_itEqBankBox.m_Check_Amount - iWage );
				pBank->m_itEqBankBox.m_Check_Amount = iWage;
			}
			else
			{
				sMsg.Format( "I only have %d gold. That is less that a days wage. Tell me 'release' if you want me to leave.",
					pBank->m_itEqBankBox.m_Check_Amount );
			}
			Speak( sMsg );
			return( true );
		}
		break;
	case PC_COME:
do_come:
		m_Act_Targ = pSrc->GetUID();
		Skill_Start( NPCACT_FOLLOW_TARG );
		break;
	case PC_DISMISS:
do_release:
		Skill_Start( SKILL_NONE );
		NPC_ClearOwners();
		SoundChar( CRESND_RAND2 );	// No noise
		return( true );
	case PC_DROP:
		// Drop just the stuff we are carrying. (not wearing)
		// "GIVE" ?
		{
			CItemContainer * pPack = GetPack();
			if ( pPack )
			{
				if ( pPack->GetCount())
				{
					pPack->ContentsDump( GetTopPoint());
					break;
				}
			}
			if ( NPC_CanSpeak())
			{
				Speak( "I'm carrying nothing." );
			}
			return( false );
		}
		break;
	case PC_DROP_ALL:
		DropAll();
		break;
	case PC_FETCH:
		pTargPrompt = "What should they fetch?";
		break;
	case PC_FOLLOW:
		pTargPrompt = "Who should they follow?";
		break;
	case PC_FOLLOW_ME:
		goto do_come;
	case PC_FRIEND:
		pTargPrompt = "Who is their friend?";
		break;
	case PC_GO:
		pTargPrompt = "Where should they go?";
		fTargAllowGround = true;
		break;
	case PC_GUARD:
		pTargPrompt = "What should they guard?";
		break;
	case PC_GUARD_ME:
		goto do_come;
	case PC_KILL:
		goto do_attack;
	case PC_PRICE:
		if ( ! m_pNPC->IsVendor())
			return( false );
		pTargPrompt = "What item would you like to set the price of?";
		break;
	case PC_RELEASE:
		goto do_release;
	case PC_SPEAK:
		SoundChar( CRESND_RAND2 );	// No noise
		return( true );
	case PC_STATUS:
		if ( NPC_CanSpeak())
		{
			CItemContainer * pBank = GetBank();
			int iWage = m_pDef->GetHireDayWage();
			if ( ! iWage ) 
				return( false );	// not sure why i'm a pet .
			CGString sMsg;
			if ( m_pNPC->IsVendor())
			{
				CItemContainer * pCont = GetBank(LAYER_VENDOR_STOCK);
				if ( iWage )
				{
					sMsg.Format( "I have %d gold on hand. "
							 "for which I will work for %d more days. "
							 "I have %d items to sell.",
						pBank->m_itEqBankBox.m_Check_Amount,
						pBank->m_itEqBankBox.m_Check_Amount / iWage,
						pCont->GetCount());
				}
				else
				{
					sMsg.Format( "I have %d gold on hand. "
							 "I restock to %d gold in %d minutes or every %d minutes. "
							 "I have %d items to sell.",
						pBank->m_itEqBankBox.m_Check_Amount,
						pBank->m_itEqBankBox.m_Check_Restock,
						pBank->GetTimerAdjusted() / 60,
						pBank->GetRestockTimeSeconds() / 60,
						pCont->GetCount());
				}
			}
			else
			{
				sMsg.Format( "I have been paid to work for %d more days.",
					pBank->m_itEqBankBox.m_Check_Amount / iWage );
			}
			Speak( sMsg );
			return( true );
		}
		break;
	case PC_STAY:
		goto do_stay;
	case PC_STAY_HERE:
do_stay:
		m_Home = GetTopPoint();
		m_pNPC->m_Home_Dist_Wander = UO_MAP_VIEW_SIGHT;
		Skill_Start( NPCACT_STAY );
		break;
	case PC_STOP:
		goto do_stay;
	case PC_TRANSFER:
		pTargPrompt = "Who do you want to transfer to?";
		break;
	default:
		return( false );
	}

	if ( pTargPrompt )
	{
		pszCmd += strlen( Pet_table[iCmd] );
		GETNONWHITESPACE( pszCmd );

		// I Need a target arg.

		if ( ! pSrc->IsClient())
			return( false );

		pSrc->m_pClient->m_tmPetCmd = iCmd;
		pSrc->m_pClient->m_Targ_UID = GetUID();
		pSrc->m_pClient->m_Targ_Text = pszCmd;

		pSrc->m_pClient->addTarget( TARGMODE_PET_CMD, pTargPrompt, fTargAllowGround );
		return( true );
	}

	// make some sound to confirm we heard it.
	// Make the yes noise.
	NPC_OnPetCommand( true, pSrc );
	return( true );
}

bool CChar::NPC_OnHearPetCmdTarg( int iCmd, CChar * pSrc, CObjUID uid, const CPointMap & pt, const TCHAR * pszArgs )
{
	if ( m_pNPC == NULL ||
		! NPC_IsOwnedBy( pSrc ) ||
		m_pPlayer ||
		m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		return( false );	// take no commands
	}

	bool fSuccess = false;	// No they won't do it.
	CItem * pItemTarg = uid.ItemFind();
	CChar * pCharTarg = uid.CharFind();

	switch ( iCmd )
	{
	case PC_GO:
		// Go to the location x,y
		if ( ! pt.IsValid())
			break;
		m_Act_p = pt;
		fSuccess = Skill_Start( NPCACT_GOTO );
		break;

	case PC_FETCH:
		if ( pItemTarg == NULL )
			break;
		if ( ! CanUse(pItemTarg))
			break;
		m_Act_Targ = pItemTarg->GetUID();
		fSuccess = Skill_Start( NPCACT_GO_FETCH );
		break;

	case PC_GUARD:
		if ( pItemTarg == NULL )
			break;
		// NPC_SetOwner( OWNER_ITEM, pObjTarg );
		fSuccess = false;
		break;
	case PC_TRANSFER:
		// transfer ownership via the transfer command.
		if ( pCharTarg == NULL )
			break;
		fSuccess = NPC_SetOwner( pCharTarg );
		break;

	case PC_KILL:
	case PC_ATTACK:
		// Attack the target.
		if ( pCharTarg == NULL )
			break;
		fSuccess = pCharTarg->OnAttackedBy( pSrc, true );
		if ( fSuccess )
		{
			fSuccess = Attack( pCharTarg );
		}
		break;

	case PC_FOLLOW:
		if ( pCharTarg == NULL )
			break;
		m_Act_Targ = pCharTarg->GetUID();
		fSuccess = Skill_Start( NPCACT_FOLLOW_TARG );
		break;
	case PC_FRIEND:
		if ( pCharTarg == NULL )
			break;
		fSuccess = NPC_SetOwner( pCharTarg );	// OWNER_FRIEND,
		break;

	case PC_PRICE:	// "PRICE" the vendor item.
		if ( pItemTarg == NULL )
			break;
		if ( ! m_pNPC->IsVendor())
			break;

		// did they name a price
		if ( isdigit( pszArgs[0] ))
		{
			return NPC_SetVendorPrice( pItemTarg, atoi(pszArgs));
		}

		// test if it is pricable.
		if ( ! NPC_SetVendorPrice( pItemTarg, -1 ))
		{
			return false;
		}

		// Now set it's price.
		if ( ! pSrc->IsClient())
			break;
		pSrc->m_pClient->m_Targ_PrvUID = GetUID();
		pSrc->m_pClient->m_Targ_UID = uid;
		pSrc->m_pClient->addPromptConsole( TARGMODE_VENDOR_PRICE, "What do you want the price to be?" );
		return( true );
	}

	// Make the yes/no noise.
	NPC_OnPetCommand( fSuccess, pSrc );
	return fSuccess;
}

void CChar::NPC_ClearOwners()
{
	if ( m_pNPC && m_pNPC->IsVendor())
	{
		ClearStat( STATF_INVUL );

		// Drop all the stuff we are trying to sell !.
		CChar * pBoss = NPC_GetOwner();
		if ( pBoss )	// Give it all back.
		{
			CItemContainer * pBankV = GetBank();
			CItemContainer * pBankB = pBoss->GetBank();
			pBoss->AddGoldToPack( pBankV->m_itEqBankBox.m_Check_Amount, pBankB );
			pBankV->m_itEqBankBox.m_Check_Amount = 0;
			NPC_Vendor_Dump( pBankB );
		}
		else
		{
			// Guess i'll just keep it.
		}
	}

	if ( IsStat( STATF_Ridden ))	// boot my rider.
	{
		CChar * pCharRider = Horse_GetMountChar();
		if ( pCharRider )
		{
			pCharRider->Horse_UnMount();
		}
	}

	Memory_ClearTypes( MEMORY_IPET );
}

bool CChar::NPC_SetOwner( const CChar * pChar )
{
	// If previous owner was OWNER_SPAWN then remove it from spawn count
	if ( IsStat( STATF_Spawned ))
	{
		Memory_FindTypes( MEMORY_ISPAWNED )->Delete();
	}

	// m_pNPC may not be set yet if this is a conjured creature.

	if ( m_pPlayer || pChar == this || pChar == NULL )
	{
		// Clear all owners ?
		NPC_ClearOwners();
		return false;
	}

	// We get some of the noto of our owner.
	// ??? If I am a pet. I have noto of my master.

	m_Home.Init();	// No longer homed.
	Memory_AddObjTypes( pChar, MEMORY_IPET );

	if ( m_pNPC && m_pNPC->IsVendor())
	{
		// Clear my cash total.
		CItemContainer * pBank = GetBank();
		pBank->m_itEqBankBox.m_Check_Amount = 0;
		SetStat( STATF_INVUL );
	}
	return( true );
}

// Hirelings...

bool CChar::NPC_CheckHirelingStatus()
{
	//  Am i happy at the moment ?
	//  If not then free myself.
	// 
	// RETURN: 
	//  true = happy.

	if ( ! IsStat( STATF_Pet ))
		return( true );

	int iWage = m_pDef->GetHireDayWage();
	if ( ! iWage || ! sm_Stat_Val_Regen[3] )
		return( true );

	// I am hired for money not for food.
	int iPeriodWage = IMULDIV( iWage, sm_Stat_Val_Regen[3], 24 * 60 * g_Serv.m_iGameMinuteLength );
	if ( iPeriodWage <= 0 ) 
		iPeriodWage = 1;

	CItemContainer * pBank = GetBank();
	if ( pBank->m_itEqBankBox.m_Check_Amount > iPeriodWage )
	{
		pBank->m_itEqBankBox.m_Check_Amount -= iPeriodWage;
	}
	else
	{
		Speak( "I'm sorry but my hire time is up." );
		Speak( "I can continue work for %d gold",
			m_pDef->GetHireDayWage());

		CItem * pMemory = Memory_AddObjTypes( NPC_GetOwner(), MEMORY_SPEAK );
		ASSERT(pMemory);
		pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_HIRE;

		NPC_ClearOwners();
		Skill_Start( SKILL_NONE );
		return( false );
	}

	return( true );
}

void CChar::NPC_OnHirePayMore( CItem * pGold, bool fHire )
{
	// We have been handed money.

	ASSERT( m_pDef->GetHireDayWage());

	CItemContainer * pBank = GetBank();
	if ( fHire )
	{
		pBank->m_itEqBankBox.m_Check_Amount = 0;
	}

	pBank->m_itEqBankBox.m_Check_Amount += pGold->GetAmount();
	Sound( pGold->GetDropSound());

	CGString sMsg;
	sMsg.Format( "I will work for you for %d days", pBank->m_itEqBankBox.m_Check_Amount / m_pDef->GetHireDayWage());
	Speak( sMsg );

	pGold->Delete();
}

bool CChar::NPC_OnHirePay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold )
{
	if ( ! m_pNPC ) 
		return( false );

	int iWage = m_pDef->GetHireDayWage();
	if ( ! iWage || IsStat( STATF_Pet ))
	{
		Speak( "Sorry I am not available for hire." );
		return false;
	}
	if ( pGold->GetAmount() < iWage )
	{
		Speak( "Sorry thats not enough for a days wage." );
		return false;
	}

	pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_NONE;
	NPC_SetOwner( pCharSrc );
	NPC_OnHirePayMore( pGold, true );
	return true;
}

bool CChar::NPC_OnHireHear( CChar * pCharSrc )
{
	if ( ! m_pNPC )
		return( false );

	int iWage = m_pDef->GetHireDayWage();
	if ( ! iWage || IsStat( STATF_Pet ))
	{
		Speak( "Sorry I am not available for hire." );
		return false;
	}

	if ( Memory_FindObjTypes( pCharSrc, MEMORY_FIGHT|MEMORY_AGGREIVED|MEMORY_IRRITATEDBY ))
	{
		Speak( "I will not work for you." );
		return false;
	}

	CGString sMsg;
	sMsg.Format( GetRandVal(2) ?
		"I can be hired for %d gold per day." :
		"I require %d gold per day for hire.", iWage );
	Speak( sMsg );

	CItem * pMemory = Memory_AddObjTypes( pCharSrc, MEMORY_SPEAK );
	pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_HIRE;
	return true;
}

bool CChar::NPC_SetVendorPrice( CItem * pItem, int iPrice )
{
	// TARGMODE_VENDOR_PRICE
	// This does not check who is setting the price if if it is valid for them to do so.

	if ( m_pNPC == NULL ||
		! m_pNPC->IsVendor())
		return( false );
	
	if ( pItem == NULL || 
		pItem->GetTopLevelObj() != this ||
		pItem->GetParent() == this )
	{
		Speak( "You can only price things in my inventory." );
		return( false );
	}

	if ( ! pItem->IsValidNPCSaleItem())
	{
		Speak( "I can't sell this" );
		return( false );
	}

	if ( iPrice < 0 )	// just a test.
		return( true );

	CGString sMsg;
	sMsg.Format( "Setting price of %s to %d", pItem->GetName(), iPrice );
	Speak( sMsg );

	pItem->SetPlayerVendorPrice( iPrice );
	return( true );
}


