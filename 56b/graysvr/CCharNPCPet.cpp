//
// CCharNPCPet.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Actions specific to an NPC.
//

#include "graysvr.h"	// predef header.

void CChar::NPC_OnPetCommand( bool fSuccess, CChar * pMaster )
{
	ADDTOCALLSTACK("CChar::NPC_OnPetCommand");
	if ( m_pPlayer )	// players don't respond
		return;

	if (  !g_Cfg.m_sSpeechPet.IsEmpty() )
		return;

	if ( !CanSee( pMaster ) )
		return;

	// i take a command from my master.
	if ( NPC_CanSpeak())
	{
		Speak( fSuccess? g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_SUCCESS ) : g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_FAILURE ) );
	}
	else
	{
		SoundChar( fSuccess? CRESND_RAND1 : CRESND_RAND2 );
	}
}

enum PC_TYPE
{
	PC_ATTACK,
	PC_BOUGHT,
	PC_CASH,
	PC_COME,
	PC_DROP,
	PC_DROP_ALL,
	PC_EQUIP,
	PC_FOLLOW,
	PC_FOLLOW_ME,
	PC_FRIEND,
	PC_GO,
	PC_GUARD,
	PC_GUARD_ME,
	PC_KILL,
	PC_PRICE,
	PC_RELEASE,
	PC_SAMPLES,
	PC_SPEAK,
	PC_STATUS,
	PC_STAY,
	PC_STOCK,
	PC_STOP,
	PC_TRANSFER,
	PC_QTY
};

bool CChar::NPC_OnHearPetCmd( LPCTSTR pszCmd, CChar * pSrc, bool fAllPets )
{
	ADDTOCALLSTACK("CChar::NPC_OnHearPetCmd");
	// This should just be another speech block !!!

	// We own this char (pet or hireling)
	// pObjTarget = the m_ActTarg has been set for them to attack.
	// RETURN:
	//  true = we understand this. tho we may not do what we are told.
	//  false = this is not a command we know.
	//  if ( GetTargMode() == CLIMODE_TARG_PET_CMD ) it needs a target.

	static LPCTSTR const sm_Pet_table[] =
	{
		"ATTACK",
		"BOUGHT",
		"CASH",
		"COME",
		"DROP",	// "GIVE" ?
		"DROP ALL",
		"EQUIP",
		"FOLLOW",
		"FOLLOW ME",
		"FRIEND",
		"GO",
		"GUARD",
		"GUARD ME",
		"KILL",
		"PRICE",	// may have args ?
		"RELEASE",
		"SAMPLES",
		"SPEAK",
		"STATUS",
		"STAY",
		"STOCK",
		"STOP",
		"TRANSFER"
	};

	// Kill me?
	// attack me?

	if ( !pSrc || m_pPlayer || !m_pNPC )
		return false;

	if ( ! NPC_IsOwnedBy( pSrc, true ))
	{
		return( false );	// take no commands
	}

	TALKMODE_TYPE	mode	= TALKMODE_SAY;
	if ( OnTriggerSpeech( true, pszCmd, pSrc, mode) )
		return true;

	PC_TYPE iCmd = (PC_TYPE) FindTableSorted( pszCmd, sm_Pet_table, COUNTOF(sm_Pet_table));
	if ( iCmd < 0 )
	{
		if ( ! strnicmp( pszCmd, sm_Pet_table[PC_PRICE], 5 ))
			iCmd = PC_PRICE;
		else
			return( false );
	}

	CCharBase * pCharDef = Char_GetDef();

	bool fTargAllowGround = false;
	bool fMayBeCrime = false;
	LPCTSTR pTargPrompt = NULL;

	if ( ! pSrc->IsClient())
		return( false );
#ifdef _ALPHASPHERE_PETS
	CClient * pClient;
	CVarDefCont * pTagStorage;
	unsigned short int iFollowerSlotsNeeded;
#endif
	switch ( iCmd )
	{
		case PC_ATTACK:
	do_attack:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_ATT );
			fMayBeCrime = true;
			break;
		case PC_CASH:
			// Drop all we have.
			if ( ! NPC_IsVendor())
				return( false );
			{
				// Give up my cash total.
				CItemContainer * pBank = GetBank();
				if ( pBank )
				{
					unsigned int iWage = pCharDef->GetHireDayWage();
					TCHAR *pszMsg = Str_GetTemp();
					if ( pBank->m_itEqBankBox.m_Check_Amount > iWage )
					{
						sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_GETGOLD_1), pBank->m_itEqBankBox.m_Check_Amount - iWage);
						pSrc->AddGoldToPack( pBank->m_itEqBankBox.m_Check_Amount - iWage );
						pBank->m_itEqBankBox.m_Check_Amount = iWage;
					}
					else
						sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_GETGOLD_2), pBank->m_itEqBankBox.m_Check_Amount);
					Speak(pszMsg);
				}
				return true;
			}
			break;
		case PC_COME:
		case PC_GUARD_ME:
		case PC_FOLLOW_ME:
			m_Act_Targ = pSrc->GetUID();
			Skill_Start( NPCACT_FOLLOW_TARG );
			break;
		case PC_RELEASE:
			Skill_Start( SKILL_NONE );
#ifdef _ALPHASPHERE_PETS
			if (!IsSetEF(EF_Minimize_Triggers) && IsSetEF(EF_PetSlots))
			{
				pTagStorage = this->GetKey("FOLLOWERSLOTS", true);
				iFollowerSlotsNeeded = pTagStorage ? ((unsigned short int)pTagStorage->GetValNum()) : 1;
				// pet's owner is pSrc
				if ( pSrc->m_pPlayer->m_curFollower >= iFollowerSlotsNeeded )
				{
		  			pSrc->m_pPlayer->m_curFollower -= iFollowerSlotsNeeded;
				} else
				{
					pSrc->m_pPlayer->m_curFollower = 0;
				}
				pClient = pSrc->GetClient();
				if (pClient)
					pClient->addCharStatWindow( this->GetUID() );
			}
#endif
			NPC_PetClearOwners();
			SoundChar( CRESND_RAND2 );	// No noise
			return( true );
		case PC_DROP:
			// Drop just the stuff we are carrying. (not wearing)
			// "GIVE" ?
			{
				CItemContainer * pPack = GetPack();
				if ( pPack )
				{
					if ( pPack->GetCount() > 0 )
					{
						pPack->ContentsDump( GetTopPoint(), ATTR_OWNED );
						break;
					}
				}
				if ( NPC_CanSpeak())
				{
					Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_CARRYNOTHING ) );
				}
				return( true );
			}
			break;
		case PC_DROP_ALL:
			DropAll( NULL, ATTR_OWNED );
			break;
		case PC_FOLLOW:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_FOLLOW );
			break;
		case PC_FRIEND:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_FRIEND );
			break;
		case PC_GO:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_GO );
			fTargAllowGround = true;
			break;
		case PC_GUARD:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_GUARD );
			fMayBeCrime = true;
			break;
		case PC_KILL:
			goto do_attack;
		case PC_PRICE:
			if ( ! NPC_IsVendor())
				return( false );
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_SETPRICE );
			break;
		case PC_SPEAK:
			NPC_OnPetCommand( true, pSrc );
			return( true );

		case PC_EQUIP:
			ItemEquipWeapon(false);
			ItemEquipArmor(false);
			break;

		case PC_STATUS:
			if ( NPC_CanSpeak() )
			{
				CItemContainer * pBank = GetBank();
				unsigned int iWage = pCharDef->GetHireDayWage();
				TCHAR *pszMsg = Str_GetTemp();
				if ( NPC_IsVendor() )
				{
					CItemContainer * pCont = GetBank(LAYER_VENDOR_STOCK);
					TCHAR *pszTemp1 = Str_GetTemp();
					TCHAR *pszTemp2 = Str_GetTemp();
					TCHAR *pszTemp3 = Str_GetTemp();
					if ( iWage )
					{
						sprintf(pszTemp1, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_1 ), pBank->m_itEqBankBox.m_Check_Amount);
						sprintf(pszTemp2, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_2 ), pBank->m_itEqBankBox.m_Check_Amount / iWage);
						sprintf(pszTemp3, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_3 ), static_cast<int>(pCont->GetCount()) );
					}
					else
					{
						sprintf(pszTemp1, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_1 ), pBank->m_itEqBankBox.m_Check_Amount );
						sprintf(pszTemp2, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_4 ), pBank->m_itEqBankBox.m_Check_Restock, pBank->GetTimerAdjusted() / 60);
						sprintf(pszTemp3, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_STAT_GOLD_3 ), static_cast<int>(pCont->GetCount()) );
					}
					sprintf(pszMsg, "%s %s %s", pszTemp1, pszTemp2, pszTemp3);
				}
				else if ( iWage )
				{
					sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_DAYS_LEFT), pBank->m_itEqBankBox.m_Check_Amount / iWage);
				}
				Speak(pszMsg);
				return true;
			}
			break;
		case PC_STAY:
		case PC_STOP:
			Skill_Start( NPCACT_STAY );
			break;
		case PC_TRANSFER:
			pTargPrompt = g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_TARG_TRANSFER );
			break;

		case PC_STOCK:
			// Magic restocking container.
			if ( ! NPC_IsVendor())
				return( false );
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_ITEMS_SELL ) );
			pSrc->GetClient()->addBankOpen( this, LAYER_VENDOR_STOCK );
			break;

		case PC_BOUGHT:
			if ( ! NPC_IsVendor())
				return( false );
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_ITEMS_BUY ) );
			pSrc->GetClient()->addBankOpen( this, LAYER_VENDOR_EXTRA );
			break;

		case PC_SAMPLES:
			if ( ! NPC_IsVendor())
				return( false );
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_ITEMS_SAMPLE ) );
			pSrc->GetClient()->addBankOpen( this, LAYER_VENDOR_BUYS );
			break;

		default:
			return( false );
	}

	if ( pTargPrompt )
	{
		pszCmd += strlen( sm_Pet_table[iCmd] );
		GETNONWHITESPACE( pszCmd );

		// I Need a target arg.

		if ( ! pSrc->IsClient())
			return( false );

		pSrc->m_pClient->m_tmPetCmd.m_iCmd = iCmd;
		pSrc->m_pClient->m_tmPetCmd.m_fAllPets = fAllPets;
		pSrc->m_pClient->m_Targ_UID = GetUID();
		pSrc->m_pClient->m_Targ_Text = pszCmd;

		pSrc->m_pClient->addTarget( CLIMODE_TARG_PET_CMD, pTargPrompt, fTargAllowGround, fMayBeCrime );
		return( true );
	}

	// make some sound to confirm we heard it.
	// Make the yes noise.
	NPC_OnPetCommand( true, pSrc );
	return( true );
}

bool CChar::NPC_OnHearPetCmdTarg( int iCmd, CChar * pSrc, CObjBase * pObj, const CPointMap & pt, LPCTSTR pszArgs )
{
	ADDTOCALLSTACK("CChar::NPC_OnHearPetCmdTarg");
	// Pet commands that required a target.

	if ( ! NPC_IsOwnedBy( pSrc ))
	{
		return( false );	// take no commands
	}

	bool fSuccess = false;	// No they won't do it.

	// Could be NULL
	CItem * pItemTarg = dynamic_cast<CItem*>(pObj);
	CChar * pCharTarg = dynamic_cast<CChar*>(pObj);

	switch ( iCmd )
	{
		case PC_GO:
			// Go to the location x,y
			if ( ! pt.IsValidPoint())
				break;
			m_Act_p = pt;
			fSuccess = Skill_Start( NPCACT_GOTO );
			break;

		case PC_GUARD:
			if ( pObj == NULL )
				break;
			m_Act_Targ = pObj->GetUID();
			fSuccess = Skill_Start( NPCACT_GUARD_TARG );
			break;
		case PC_TRANSFER:
			// transfer ownership via the transfer command.
			if ( pCharTarg == NULL )
				break;
			if ( pCharTarg->IsClient() )
			{
	#ifdef _ALPHASPHERE_PETS
				if (!IsSetEF(EF_Minimize_Triggers) && IsSetEF(EF_PetSlots))
				{

					CVarDefCont * pTagStorage = this->GetKey("FOLLOWERSLOTS", true);
					unsigned short int iFollowerSlotsNeeded = pTagStorage ? ((unsigned short int)pTagStorage->GetValNum()) : 1;
					// transferrer is pSrc
					// transferree is pCharTarg
					if ((iFollowerSlotsNeeded + pCharTarg->m_pPlayer->m_curFollower) > pCharTarg->m_pPlayer->m_maxFollower )
					{
						SysMessage( g_Cfg.GetDefaultMsg(DEFMSG_TRANSFER_NO_SLOTS_FREE) );
						break;
					}
		  			pCharTarg->m_pPlayer->m_curFollower += iFollowerSlotsNeeded;
					// send an update packet for the stats
					CClient * pClient = pCharTarg->GetClient();
					if (pClient)
						pClient->addCharStatWindow( this->GetUID() );

					// free a follower slot at the transferrer
					if ( pSrc->m_pPlayer->m_curFollower >= iFollowerSlotsNeeded )
					{
		  				pSrc->m_pPlayer->m_curFollower -= iFollowerSlotsNeeded;
					} else
					{
						pSrc->m_pPlayer->m_curFollower = 0;
					}
					pClient = pSrc->GetClient();
					if (pClient)
						pClient->addCharStatWindow( this->GetUID() );
				}
	#endif
				fSuccess = NPC_PetSetOwner( pCharTarg );
			}
			break;

		case PC_KILL:
		case PC_ATTACK:
			// Attack the target.
			if ( pCharTarg == NULL )
				break;
			// refuse to attack friends.
			if ( NPC_IsOwnedBy( pCharTarg, true ))
			{
				fSuccess = false;	// take no commands
				break;
			}
			fSuccess = pCharTarg->OnAttackedBy( pSrc, 1, true );	// we know who told them to do this.
			if ( fSuccess )
			{
				fSuccess = Fight_Attack( pCharTarg );
			}
			break;

		case PC_FOLLOW:
			if ( pCharTarg == NULL )
				break;
			m_Act_Targ = pCharTarg->GetUID();
			fSuccess = Skill_Start( NPCACT_FOLLOW_TARG );
			break;
		case PC_FRIEND:
			// Not the same as owner,
			if ( pCharTarg == NULL )
				break;
			Memory_AddObjTypes( pCharTarg, MEMORY_FRIEND );
			break;

		case PC_PRICE:	// "PRICE" the vendor item.
			if ( pItemTarg == NULL )
				break;
			if ( ! NPC_IsVendor())
				break;

			// did they name a price
			if ( IsDigit( pszArgs[0] ))
			{
				return NPC_SetVendorPrice( pItemTarg, ATOI(pszArgs) );
			}

			// test if it is pricable.
			if ( ! NPC_SetVendorPrice( pItemTarg, -1 ))
			{
				return false;
			}

			// Now set it's price.
			if ( ! pSrc->IsClient())
				break;
			
			pSrc->m_pClient->addPromptConsole( CLIMODE_PROMPT_VENDOR_PRICE, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_SETPRICE_2 ), pItemTarg->GetUID(), GetUID() );
			return( true );

		default:
			break;
	}

	// Make the yes/no noise.
	NPC_OnPetCommand( fSuccess, pSrc );
	return fSuccess;
}

void CChar::NPC_PetClearOwners()
{
	ADDTOCALLSTACK("CChar::NPC_PetClearOwners");
	if ( NPC_IsVendor() )
	{
		StatFlag_Clear(STATF_INVUL);

		// Drop all the stuff we are trying to sell !.
		CChar * pBoss = NPC_PetGetOwner();
		if ( pBoss )	// Give it all back.
		{
			CItemContainer * pBankV = GetBank();
			CItemContainer * pBankB = pBoss->GetBank();
			pBoss->AddGoldToPack( pBankV->m_itEqBankBox.m_Check_Amount, pBankB );
			pBankV->m_itEqBankBox.m_Check_Amount = 0;

			for ( size_t i = 0; i < COUNTOF(sm_VendorLayers); i++ )
			{
				CItemContainer * pCont = GetBank( sm_VendorLayers[i] );
				if ( !pCont )
					continue;
				CItem	*pItemNext, *pItem = pCont->GetContentHead();
				for ( ; pItem != NULL; pItem = pItemNext )
				{
					pItemNext = pItem->GetNext();
					pBankB->ContentAdd(pItem);
				}
			}
		}
	}

	if ( IsStatFlag(STATF_Ridden) )
	{
		CChar	*pCharRider = Horse_GetMountChar();
		if ( pCharRider )
			pCharRider->Horse_UnMount();
	}

	Memory_ClearTypes(MEMORY_IPET|MEMORY_FRIEND);
}

bool CChar::NPC_PetSetOwner( const CChar * pChar )
{
	ADDTOCALLSTACK("CChar::NPC_PetSetOwner");
	// If previous owner was OWNER_SPAWN then remove it from spawn count
	if ( IsStatFlag( STATF_Spawned ))
	{
		Memory_ClearTypes( MEMORY_ISPAWNED );
	}

	// m_pNPC may not be set yet if this is a conjured creature.

	if ( m_pPlayer || pChar == this || pChar == NULL )
	{
		// Clear all owners ?
		NPC_PetClearOwners();
		return false;
	}
   NPC_PetClearOwners();
	// We get some of the noto of our owner.
	// ??? If I am a pet. I have noto of my master.

	m_ptHome.InitPoint();	// No longer homed.
	Memory_AddObjTypes( pChar, MEMORY_IPET );
   NPC_Act_Idle();
	if ( NPC_IsVendor())
	{
		// Clear my cash total.
		CItemContainer * pBank = GetBank();
		pBank->m_itEqBankBox.m_Check_Amount = 0;
		StatFlag_Set( STATF_INVUL );
	}
	return( true );
}

// Hirelings...

bool CChar::NPC_CheckHirelingStatus()
{
	ADDTOCALLSTACK("CChar::NPC_CheckHirelingStatus");
	//  Am i happy at the moment ?
	//  If not then free myself.
	//
	// RETURN:
	//  true = happy.

	if ( ! IsStatFlag( STATF_Pet ))
		return( true );

	CCharBase * pCharDef = Char_GetDef();
	int iFoodConsumeRate = g_Cfg.m_iRegenRate[STAT_FOOD];

	unsigned int iWage = pCharDef->GetHireDayWage();
	if ( ! iWage || ! iFoodConsumeRate )
		return( true );

	// I am hired for money not for food.
	unsigned int iPeriodWage = IMULDIV( iWage, iFoodConsumeRate, 24 * 60 * g_Cfg.m_iGameMinuteLength );
	if ( iPeriodWage <= 0 )
		iPeriodWage = 1;

	CItemContainer * pBank = GetBank();
	if ( pBank->m_itEqBankBox.m_Check_Amount > iPeriodWage )
	{
		pBank->m_itEqBankBox.m_Check_Amount -= iPeriodWage;
	}
	else
	{
		TCHAR* pszMsg = Str_GetTemp();
		sprintf(pszMsg, g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_WAGE_COST ), iWage);
		Speak(pszMsg);

		CChar * pOwner = NPC_PetGetOwner();
		if ( pOwner )
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_HIRE_TIMEUP ) );

			CItem * pMemory = Memory_AddObjTypes( pOwner, MEMORY_SPEAK );
			if ( pMemory )
				pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_HIRE;

			NPC_PetDesert();
			return false;
		}

		// Some sort of strange bug to get here.
		Memory_ClearTypes( MEMORY_IPET );
		StatFlag_Clear( STATF_Pet );
	}

	return( true );
}

void CChar::NPC_OnHirePayMore( CItem * pGold, bool fHire )
{
	ADDTOCALLSTACK("CChar::NPC_OnHirePayMore");
	// We have been handed money.
	// similar to PC_STATUS

	CCharBase * pCharDef = Char_GetDef();
	unsigned int iWage = pCharDef->GetHireDayWage();
	CItemContainer	*pBank = GetBank();
	if ( !iWage || !pBank )
		return;

	if ( pGold )
	{
		if ( fHire )
		{
			pBank->m_itEqBankBox.m_Check_Amount = 0;	// zero any previous balance.
		}

		pBank->m_itEqBankBox.m_Check_Amount += pGold->GetAmount();
		Sound( pGold->GetDropSound( NULL ));
		pGold->Delete();
	}

	TCHAR *pszMsg = Str_GetTemp();
	sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_HIRE_TIME), pBank->m_itEqBankBox.m_Check_Amount / iWage);
	Speak(pszMsg);
}

bool CChar::NPC_OnHirePay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold )
{
	ADDTOCALLSTACK("CChar::NPC_OnHirePay");
	if ( !m_pNPC || !pCharSrc || !pMemory )
		return false;

	CCharBase * pCharDef = Char_GetDef();
	if ( IsStatFlag( STATF_Pet ))
	{
		if ( ! pMemory->IsMemoryTypes(MEMORY_IPET|MEMORY_FRIEND))
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_EMPLOYED ) );
			return false;
		}
	}
	else
	{
		unsigned int iWage = pCharDef->GetHireDayWage();
		if ( iWage <= 0 )
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_NOT_FOR_HIRE ) );
			return false;
		}
		if ( pGold->GetAmount() < iWage )
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_NOT_ENOUGH ) );
			return false;
		}
		// NOTO_TYPE ?
		if ( pMemory->IsMemoryTypes( MEMORY_FIGHT|MEMORY_HARMEDBY|MEMORY_IRRITATEDBY ))
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_NOT_WORK ) );
			return false;
		}

		// Put all my loot cash away.
		ContentConsume( RESOURCE_ID(RES_TYPEDEF,IT_GOLD), INT_MAX, false, 0 );
		// Mark all my stuff ATTR_OWNED - i won't give it away.
		ContentAttrMod( ATTR_OWNED, true );

		NPC_PetSetOwner( pCharSrc );
	}

	pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_NONE;
	NPC_OnHirePayMore( pGold, true );
	return true;
}

bool CChar::NPC_OnHireHear( CChar * pCharSrc )
{
	ADDTOCALLSTACK("CChar::NPC_OnHireHear");
	if ( !m_pNPC )
		return false;

	CCharBase * pCharDef = Char_GetDef();
	unsigned int iWage = pCharDef->GetHireDayWage();
	if ( ! iWage )
	{
		Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_NOT_FOR_HIRE ) );
		return false;
	}

	CItemMemory * pMemory = Memory_FindObj( pCharSrc );
	if ( pMemory )
	{
		if ( pMemory->IsMemoryTypes(MEMORY_IPET|MEMORY_FRIEND))
		{
			// Next gold i get goes toward hire.
			pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_HIRE;
			NPC_OnHirePayMore( NULL, false );
			return true;
		}
		if ( pMemory->IsMemoryTypes( MEMORY_FIGHT|MEMORY_HARMEDBY|MEMORY_IRRITATEDBY ))
		{
			Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_NOT_WORK ) );
			return false;
		}
	}
	if ( IsStatFlag( STATF_Pet ))
	{
		Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_EMPLOYED ) );
		return false;
	}

	TCHAR *pszMsg = Str_GetTemp();
	sprintf(pszMsg, Calc_GetRandVal(2) ?
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_HIRE_AMNT ) :
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_HIRE_RATE ), iWage );
	Speak(pszMsg);

	pMemory = Memory_AddObjTypes( pCharSrc, MEMORY_SPEAK );
	if ( pMemory )
		pMemory->m_itEqMemory.m_Action = NPC_MEM_ACT_SPEAK_HIRE;
	return true;
}

bool CChar::NPC_SetVendorPrice( CItem * pItem, int iPrice )
{
	ADDTOCALLSTACK("CChar::NPC_SetVendorPrice");
	// player vendors.
	// CLIMODE_PROMPT_VENDOR_PRICE
	// This does not check who is setting the price if if it is valid for them to do so.

	if ( ! NPC_IsVendor())
		return( false );

	if ( pItem == NULL ||
		pItem->GetTopLevelObj() != this ||
		pItem->GetParent() == this )
	{
		Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_INV_ONLY ) );
		return( false );
	}

	CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
	if ( pVendItem == NULL )
	{
		Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_PET_CANTSELL ) );
		return( false );
	}

	if ( iPrice < 0 )	// just a test.
		return( true );

	TCHAR *pszMsg = Str_GetTemp();
	sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_VENDOR_SETPRICE_1), pVendItem->GetName(), iPrice);
	Speak(pszMsg);

	pVendItem->SetPlayerVendorPrice( iPrice );
	return( true );
}

void CChar::NPC_PetDesert()
{
	ADDTOCALLSTACK("CChar::NPC_PetDesert");
	CChar * pCharOwn = NPC_PetGetOwner();

	// Are we a pet at all?
	if ( pCharOwn )
	{
		// How happy are we with being a pet ?
		if ( ! pCharOwn->CanSee(this))
		{
			pCharOwn->SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_DESERTED), GetName());
		}

		if ( !IsSetEF(EF_Minimize_Triggers) )
		{
			if ( OnTrigger( CTRIG_PetDesert, pCharOwn, NULL ) == TRIGRET_RET_TRUE ) return;
		}

		NPC_PetClearOwners();

		TCHAR	*pszMsg = Str_GetTemp();
		sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_NPC_PET_DECIDE_MASTER), GetName());
		Speak(pszMsg);

		// free to do as i wish !
		Skill_Start( SKILL_NONE );
	}
}

