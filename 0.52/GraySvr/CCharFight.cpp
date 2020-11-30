//
// CCharFight.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Fight/Criminal actions/Noto.
//

#include "graysvr.h"	// predef header.

CItemStone * CChar::Guild_Find( MEMORY_TYPE MemType ) const
{
	CItemMemory * pMyGMem = Memory_FindTypes( MemType );
	if ( ! pMyGMem )
		return( NULL );
	CItemStone * pMyStone = dynamic_cast <CItemStone*>( pMyGMem->m_uidLink.ItemFind());
	if ( pMyStone == NULL )
	{
		((CChar*)this)->Memory_ClearTypes( MemType ); 	// Make them forget they were ever in this guild....again!
		return( NULL );
	}
	return( pMyStone );
}

CStoneMember * CChar::Guild_FindMember( MEMORY_TYPE MemType  ) const
{
	CItemStone * pMyStone = Guild_Find(MemType);
	if ( pMyStone == NULL )
		return( NULL );
	CStoneMember * pMember = pMyStone->GetMember( this );
	if ( pMember == NULL )
	{
		((CChar*)this)->Memory_ClearTypes( MemType ); 	// Make them forget they were ever in this guild....again!
		return( NULL );
	}
	return( pMember );
}

void CChar::Guild_Resign( MEMORY_TYPE MemType )
{
	// response to "I resign from my guild"
	CStoneMember * pMember = Guild_FindMember(MemType);
	if ( pMember == NULL )
		return ;
	delete pMember;
}

const TCHAR * CChar::Guild_AbbrevBracket( MEMORY_TYPE MemType ) const
{
	const TCHAR * pszAbbrev = Guild_Abbrev(MemType);
	if ( pszAbbrev == NULL )
		return( NULL );
	TCHAR * pszTemp = GetTempStr();
	sprintf( pszTemp, " [%s]", pszAbbrev );
	return( pszTemp );
}

const TCHAR * CChar::Guild_Abbrev( MEMORY_TYPE MemType ) const
{
	CStoneMember * pMember = Guild_FindMember(MemType);
	if ( pMember == NULL )
		return( NULL );
	if ( ! pMember->IsAbbrevOn())
		return( NULL );
	CItemStone * pMyStone = dynamic_cast <CItemStone *>( pMember->GetParent());
	if ( pMyStone == NULL || 
		! pMyStone->GetAbbrev()[0] )
		return( NULL );
	return( pMyStone->GetAbbrev());
}

NOTO_TYPE CChar::GetNotoFlag( const CChar * pCharViewer, bool fAllowIncog ) const
{
	// This allows the noto attack check in the client.

	if ( fAllowIncog && IsStat( STATF_Incognito ))
	{
		return NOTO_NEUTRAL;
	}

	if ( IsEvil())
		return( NOTO_EVIL );

	if ( this != pCharViewer ) // Am I checking myself?
	{
		// Check the guild stuff
		CItemStone * pMyTown = Guild_Find(MEMORY_TOWN);
		CItemStone * pMyGuild = Guild_Find(MEMORY_GUILD);
		if ( pMyGuild || pMyTown )
		{
			CItemStone * pTheirGuild = pCharViewer->Guild_Find(MEMORY_GUILD);
			CItemStone * pTheirTown = pCharViewer->Guild_Find(MEMORY_TOWN);
			// Are we both in a guild?
			if ( pTheirGuild || pTheirTown )
			{
				if ( pMyGuild )
				{
					if ( pTheirGuild == pMyGuild ) // Same guild?
						return NOTO_GUILD_SAME; // return green
					if ( pMyGuild->IsSameAlignType( pTheirGuild ))
						return NOTO_GUILD_SAME;
					// Are we at in different guilds but at war? (not actually a crime right?)
					if ( pMyGuild->IsAtWarWith(pTheirGuild))
						return NOTO_GUILD_WAR; // return orange
					if ( pMyGuild->IsAtWarWith(pTheirTown))
						return NOTO_GUILD_WAR; // return orange
				}
				if ( pMyTown)
				{
					if ( pMyTown->IsAtWarWith(pTheirGuild))
						return NOTO_GUILD_WAR; // return orange
					if ( pMyTown->IsAtWarWith(pTheirTown))
						return NOTO_GUILD_WAR; // return orange
				}
			}
		}
	}

	if ( IsStat( STATF_Criminal ))	// criminal to everyone.
		return( NOTO_CRIMINAL );

	if ( this != pCharViewer ) // Am I checking myself?
	{
		if ( NPC_IsOwnedBy( pCharViewer, false ))	// All pets are neutral to their owners.
			return( NOTO_NEUTRAL );

		// If they saw me commit a crime or I am their aggressor then
		// criminal to just them.
		CItemMemory * pMemory = pCharViewer->Memory_FindObjTypes( this, MEMORY_SAWCRIME );
		if ( pMemory != NULL )
		{
			return( NOTO_CRIMINAL );
		}
	}

	if ( IsNeutral())
	{
		return( NOTO_NEUTRAL );
	}
	return( NOTO_GOOD );
}

bool CChar::IsEvil() const
{
	// animals and humans given more leeway.
	if ( IsMurderer()) 
		return( true );
	int iKarma = Stat_Get(STAT_Karma);
	switch ( GetCreatureType())
	{
	case NPCBRAIN_MONSTER:
		return( iKarma< 0 );	// I am evil ?
	case NPCBRAIN_BESERK:
		return( true );
	case NPCBRAIN_ANIMAL:
		return( iKarma<= -800 );
	}
	if ( m_pPlayer )
	{
		return( iKarma<g_Serv.m_iPlayerKarmaEvil );
	}
	return( iKarma<= -3000 );
}

bool CChar::IsNeutral() const
{
	// Should neutrality change in guarded areas ?
	int iKarma = Stat_Get(STAT_Karma);
	switch ( GetCreatureType())
	{
	case NPCBRAIN_MONSTER:
	case NPCBRAIN_BESERK:
		return( iKarma<= 0 );
	case NPCBRAIN_ANIMAL:
		return( iKarma<= 100 );
	}
	if ( m_pPlayer )
	{
		return( iKarma<g_Serv.m_iPlayerKarmaNeutral );
	}
	return( iKarma<0 );
}

const TCHAR * CChar::GetFameTitle() const
{
	if ( IsStat( STATF_Incognito )) 
		return( "" );
	if ( ! IsPriv( PRIV_PRIV_NOSHOW ))
	{
		if ( IsPriv( PRIV_GM ))
			return( "GM " );
		switch ( GetPrivLevel())
		{
		case PLEVEL_Seer: return( "Seer " );
		case PLEVEL_Counsel: return( "Counselor " );
		}
	}
	// ??? Mayor ?
	if ( Stat_Get(STAT_Fame) <= 9900 )
		return( "" );
	if ( CCharBase::IsFemaleID( GetDispID())) 
		return( "Lady " );
	return( "Lord " );
}

int CChar::Noto_GetLevel() const
{
	// Paperdoll title for character
	// This is so we can inform user of change in title !

	static const int KarmaLevel[] =
	{ 9900, 5000, 1000, 500, 100, -100, -500, -1000, -5000, -9900 };

	int i=0;
	for ( ; i<COUNTOF( KarmaLevel ) && Stat_Get(STAT_Karma) < KarmaLevel[i]; i++ )
		;

	static const WORD FameLevel[] =
	{ 500, 1000, 5000, 9900 };

	int j =0;
	for ( ; j<COUNTOF( FameLevel ) && Stat_Get(STAT_Fame) > FameLevel[j]; j++ )
		;

	return( ( i * 5 ) + j );
}

const TCHAR * CChar::Noto_GetTitle() const
{
	// Paperdoll title for character

#if 0
	// Murderer titles.
30 Destroyer of Life
50 Impaler
90 Slaughterer of Innocents
125+ was Evil Lord, Name, enemy of all living

#endif


	// Paperdoll title for character

	const TCHAR * pTitle;

	// Murderer ?
	if ( IsMurderer())
	{
		pTitle = "murderer";
	}
	else if ( IsStat( STATF_Criminal ))
	{
		pTitle = "criminal";
	}
	else
	{
		int iLevel = Noto_GetLevel();
		if ( ! g_Serv.m_NotoTitles.IsValidIndex(iLevel))
		{
			pTitle = "";
		}
		else
		{
			pTitle = g_Serv.m_NotoTitles[ iLevel ];
		}
	}

	TCHAR * pTemp = GetTempStr();
	sprintf( pTemp, "%s%s%s%s%s",
		(pTitle[0]) ? "The " : "",
		pTitle,
		(pTitle[0]) ? " " : "",
		GetFameTitle(),
		GetName());

	return( pTemp );
}

void CChar::Noto_Murder()
{
	// I am a murderer (it seems) (update my murder decay item)
	if ( IsPriv(PRIV_GM))
		return;
	if ( ! IsMurderer())
		return;

	// NOTE: This should not be a ITEMID_MEMORY really !

	SysMessage( "Murderer!" );
	CItem * pFlag = CItem::CreateBase( ITEMID_MEMORY );
	pFlag->m_Attr |= ATTR_NEWBIE;
	pFlag->SetDecayTime( g_Serv.m_iMurderDecayTime );
	LayerAdd( pFlag, LAYER_FLAG_Murders );
}

void CChar::Noto_Criminal()
{
	// I am a criminal and the guards will be on my ass.
	if ( IsPriv(PRIV_GM))
		return;

	// NOTE: This should not be a ITEMID_MEMORY really !

	SysMessage( "Criminal!" );
	CItem * pFlag = CItem::CreateBase( ITEMID_MEMORY );
	pFlag->m_Attr |= ATTR_NEWBIE;
	pFlag->SetDecayTime( g_Serv.m_iCriminalTimer );
	LayerAdd( pFlag, LAYER_FLAG_Criminal );
}

void CChar::Noto_ChangeDeltaMsg( int iDelta, const TCHAR * pszType )
{
	if ( iDelta == 0 )
		return;

#define NOTO_FACTOR 300
	static const TCHAR * sm_DegreeTable[] =
	{
		"a bit of",
		"a small amount of",
		"a little",
		"some",
		"a moderate amount of",
		"alot of",
		"large amounts of",
		"huge amounts of",
	};

	int iDegree = abs(iDelta) / ( 10000 / NOTO_FACTOR / ( COUNTOF(sm_DegreeTable) - 1));
	if (iDegree > COUNTOF(sm_DegreeTable) - 1)
		iDegree = COUNTOF(sm_DegreeTable) - 1;

	SysMessagef( "You have %s %s %s.",
		( iDelta < 0 ) ? "lost" : "gained",
		sm_DegreeTable[iDegree], pszType );
}

void CChar::Noto_ChangeNewMsg( int iPrvLevel )
{
	if ( iPrvLevel != Noto_GetLevel())
	{
		// reached a new title level ?
		SysMessagef( "You are now %s", Noto_GetTitle());
	}
}

void CChar::Noto_Fame( int iFameChange )
{
	// Fame should only go down on death, time or cowardice ?

	if ( ! iFameChange ) 
		return;
	int iFame = Stat_Get(STAT_Fame) + iFameChange;
	if ( iFame < 0 ) iFame = 0;
	if ( iFame > 10000) iFame = 10000; // Maximum reached
	Noto_ChangeDeltaMsg( iFame - Stat_Get(STAT_Fame), "fame" );
	Stat_Set(STAT_Fame,iFame);
}

void CChar::Noto_Karma( int iKarmaChange, int iBottom )
{
	// iBottom is a variable where you control at what point
	// the loss for this action stop (as in stealing shouldnt
	// take you to dread ). iBottom def. to -10000 if you leave
	// it out.

	if ( ! iKarmaChange )
		return;
	int	iKarma = Stat_Get(STAT_Karma);
	if ( iKarma > 0 )
	{
		// Your a good guy. Harder to be a good guy.
		if ( iKarmaChange < 0 )
			iKarmaChange *= 2;	// counts worse against you.
		else
			iKarmaChange /= 2;	// counts less for you.
	}
	else
	{
		// Your a bad guy.
	}

	// If we are going to loose karma and are already below bottom
	// then return.
	if (( iKarma <= iBottom ) && ( iKarmaChange < 0 ))
		return;

	iKarma += iKarmaChange;
	if ( iKarmaChange < 0 )
	{
		if ( iKarma < iBottom )
			iKarma = iBottom;
	}
	else
	{
		if ( iKarma > 10000 )
			iKarma = 10000;
	}

	Noto_ChangeDeltaMsg( iKarma - Stat_Get(STAT_Karma), "karma" );
	Stat_Set(STAT_Karma,iKarma);
}

void CChar::Noto_KarmaChangeMessage( int iKarmaChange, int iLimit )
{
	// Change your title ?
	int iPrvLevel = Noto_GetLevel();
	Noto_Karma( iKarmaChange, iLimit );
	Noto_ChangeNewMsg( iPrvLevel );
}

/////////////////////////////////////////////////////////////////////////////
// Memory this char has about something in the world.

bool CChar::Memory_UpdateFlags( CItemMemory * pMemory )
{
	// Reset the check timer based on the type of memory.

	ASSERT(pMemory);
	ASSERT(pMemory->m_type == ITEM_EQ_MEMORY_OBJ );

	WORD wMemTypes = pMemory->GetMemoryTypes();

	if ( ! wMemTypes )	// No memories here anymore so kill it.
	{
		return false;
	}

	int iCheckTime;
	if ( wMemTypes & MEMORY_ISPAWNED )
	{
		SetStat( STATF_Spawned );
	}
	else if ( wMemTypes & MEMORY_IPET )
	{
		SetStat( STATF_Pet );
	}

	if ( wMemTypes & MEMORY_FOLLOW )
		iCheckTime = TICK_PER_SEC;
	else if ( wMemTypes & MEMORY_FIGHT )	// update more often to check for retreat.
		iCheckTime = 30*TICK_PER_SEC;
	else if ( wMemTypes & ( MEMORY_IPET | MEMORY_GUARD | MEMORY_ISPAWNED | MEMORY_GUILD | MEMORY_TOWN ))
		iCheckTime = -1;	// never go away.
	else if ( m_pNPC )	// MEMORY_SPEAK
		iCheckTime = 5*60*TICK_PER_SEC;
	else
	{
		DEBUG_CHECK( m_pPlayer );
		iCheckTime = 20*60*TICK_PER_SEC;
	}
	DEBUG_CHECK(iCheckTime);
	pMemory->SetTimeout( iCheckTime );	// update it's decay time.
	return( true );
}

bool CChar::Memory_UpdateClearTypes( CItemMemory * pMemory, WORD MemTypes )
{
	// Just clear these flags but do not delete the memory.
	// RETURN: true = still useful memory.
	ASSERT(pMemory);
	bool fMore = pMemory->SetMemoryTypes( pMemory->GetMemoryTypes() &~ MemTypes );

	if ( MemTypes & MEMORY_ISPAWNED )
	{
		ClearStat( STATF_Spawned );
		// I am a memory link to another object.
		CItem * pSpawn = pMemory->m_uidLink.ItemFind();
		if ( pSpawn != NULL && 
			pSpawn->m_type == ITEM_SPAWN_CHAR && 
			pSpawn->m_itSpawnChar.m_current )
		{
			pSpawn->m_itSpawnChar.m_current --;
		}
	}
	if ( MemTypes & MEMORY_IPET )
	{
		// Am i still a pet of some sort ?
		if ( Memory_FindTypes( MEMORY_IPET ) == NULL )
			ClearStat( STATF_Pet );
	}

#ifdef _DEBUG
	// Must be deleted.
	if ( ! fMore && MemTypes && g_Log.IsLogged( LOGL_TRACE ))
	{
		CObjBase * pObj = pMemory->m_uidLink.ObjFind();
		LPCSTR pName = ( pObj != NULL ) ? pObj->GetName() : "unk";
		DEBUG_MSG(( "Memory delete from '%s' for '%s', type 0%x\n", GetName(), pName, MemTypes ));
	}
#endif

	return Memory_UpdateFlags( pMemory );
}

void CChar::Memory_AddTypes( CItemMemory * pMemory, WORD MemTypes )
{
	ASSERT(pMemory);
	DEBUG_CHECK(MemTypes);
	pMemory->SetMemoryTypes( pMemory->GetMemoryTypes() | MemTypes );
	pMemory->m_itEqMemory.m_p = GetTopPoint();	// Where did the fight start ?
	pMemory->m_itEqMemory.m_StartTime = g_World.GetTime();
	Memory_UpdateFlags( pMemory );
}

bool CChar::Memory_ClearTypes( CItemMemory * pMemory, WORD MemTypes )
{
	// Clear this memory object of this type.
	ASSERT(pMemory);
	DEBUG_CHECK(MemTypes);
	if ( Memory_UpdateClearTypes( pMemory, MemTypes ))
		return( true );
	pMemory->Delete();
	return( false );
}

CItemMemory * CChar::Memory_CreateObj( CObjUID uid, WORD MemTypes )
{
	// Create a memory about this object.
	// NOTE: Does not check if object already has a memory.
	//  Assume it does not !

#ifdef _DEBUG
	DEBUG_CHECK( Memory_FindObj( uid ) == NULL );
#endif

	DEBUG_CHECK( MemTypes );
	if (( MemTypes & MEMORY_IPET ) && uid.IsItem())
	{
		MemTypes = MEMORY_ISPAWNED;
	}
	CItem * pItem = CItem::CreateBase( ITEMID_MEMORY );
	ASSERT(pItem);
	CItemMemory * pMemory = dynamic_cast <CItemMemory *>(pItem);
	ASSERT(pMemory);
	if ( pMemory == NULL )
		return( FALSE );
	pMemory->m_Attr |= ATTR_NEWBIE;
	pMemory->m_type = ITEM_EQ_MEMORY_OBJ;
	pMemory->m_uidLink = uid;
	Memory_AddTypes( pMemory, MemTypes );
	LayerAdd( pMemory, LAYER_SPECIAL );
	return( pMemory );
}

void CChar::Memory_ClearTypes( WORD MemTypes )
{
	// Remove all the memories of this type.
	DEBUG_CHECK(MemTypes);
	CItem* pItemNext;
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( ! pItem->IsMemoryTypes(MemTypes))
			continue;
		CItemMemory * pMemory = dynamic_cast <CItemMemory *>(pItem);
		if ( pMemory == NULL )
			continue;
		Memory_ClearTypes( pMemory, MemTypes );
	}
}

CItemMemory * CChar::Memory_FindObj( CObjUID uid ) const
{
	// Do I have a memory / link for this object ?
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsType(ITEM_EQ_MEMORY_OBJ))
			continue;
		if ( pItem->m_uidLink != uid )
			continue;
		return( dynamic_cast <CItemMemory *>( pItem ));
	}
	return( NULL );
}

CItemMemory * CChar::Memory_FindTypes( WORD MemTypes ) const
{
	// Do we have a certain type of memory.
	// Just find the first one.
	if ( ! MemTypes )
		return( NULL );
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsMemoryTypes(MemTypes))
			continue;
		return( dynamic_cast <CItemMemory *>( pItem ));
	}
	return( NULL );
}

CItemMemory * CChar::Memory_AddObjTypes( CObjUID uid, WORD MemTypes )
{
	DEBUG_CHECK(MemTypes);
	CItemMemory * pMemory = Memory_FindObj( uid );
	if ( pMemory == NULL )
	{
		return Memory_CreateObj( uid, MemTypes );
	}
	Memory_AddTypes( pMemory, MemTypes );
	return( pMemory );
}

bool CChar::Memory_OnTick( CItemMemory * pMemory )
{
	// NOTE: Do not return true unless u update the timer !
	// RETURN: false = done with this memory.
	ASSERT(pMemory);
	if ( pMemory->IsMemoryTypes( MEMORY_FOLLOW ))
	{
		// I am following. (GM Feature)
		CChar * pChar = pMemory->m_uidLink.CharFind();
		if ( pChar && ! IsStat( STATF_War ) && ! IsDisconnected())
		{
			CPointMap ptold = GetTopPoint();
			CPointMap ptnew = pChar->GetTopPoint();
			UpdateDir( ptnew );
			MoveTo( ptnew );
			UpdateMove( ptold );
		}
		else
		{
			if ( ! Memory_UpdateClearTypes( pMemory, MEMORY_FOLLOW ))
				return( false );
		}
		return( true );
	}

	if ( pMemory->IsMemoryTypes( MEMORY_FIGHT ))
	{
		// Is the fight still valid ?
		return Fight_OnTick( pMemory );
	}

	return( false );	// kill it.
}

//////////////////////////////////////////////////////////////////////////////
// Fight memories.

void CChar::Fight_Retreat( CChar * pTarg, CItemMemory * pFight )
{
	// The fight is over.
	// Somebody ran away.
	if ( pTarg == NULL || pTarg->IsStat( STATF_DEAD ))
		return;
	ASSERT(pFight);
	bool fCowardice = ( GetTopPoint().GetDist( pFight->m_itEqMemory.m_p ) > pTarg->GetTopPoint().GetDist( pFight->m_itEqMemory.m_p ));

	if ( fCowardice && ! pFight->IsMemoryTypes( MEMORY_IAGGRESSOR ))
		return;

	SysMessagef( fCowardice ?
		"You have retreated from the battle with %s" :
		"%s has retreated from the battle.", pTarg->GetName());

	// Lose some fame.
	if ( fCowardice )
	{
		Noto_Fame( -1 );
	}
}

bool CChar::Fight_OnTick( CItemMemory * pMemory )
{
	// Check on the status of the fight.
	// return: false = delete the memory completely.
	//  true = skip it.
	ASSERT(pMemory);
	CChar * pTarg = pMemory->m_uidLink.CharFind();
	if ( pTarg == NULL ) 
		return( false );	// They are gone for some reason ?

#ifdef _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "OnTick '%s' Memory of Fighting '%s'\n", GetName(), pTarg->GetName()));
	}
#endif

	if ( GetDist(pTarg) > UO_MAP_VIEW_RADAR )
	{
		Fight_Retreat( pTarg, pMemory );
clearit:
		Memory_ClearTypes( pMemory, MEMORY_FIGHT );
		return( true );
	}

	int iTime = g_World.GetTime() - pMemory->m_itEqMemory.m_StartTime;
	//DEBUG_CHECK( iTime >= 0 );

	// If am fully healthy then it's not much of a fight.
	if ( iTime > 60*60*TICK_PER_SEC )
		goto clearit;
	if ( pTarg->GetHealthPercent() >= 100 && iTime > 2*60*TICK_PER_SEC )
		goto clearit;

	pMemory->SetTimeout(20*TICK_PER_SEC);
	return( true );	// reschedule it.
}

void CChar::Fight_Start( const CChar * pTarg )
{
	// I am attacking this creature.
	// i might be the the aggressor or just retaliating.
	//

	ASSERT(pTarg);
	if ( IsStat( STATF_War ) && m_Act_Targ == pTarg->GetUID())
		return;

	WORD MemTypes;
	CItemMemory * pTargMemory = NULL;
	CItemMemory * pMemory = Memory_FindObj( pTarg );
	if ( pMemory == NULL )
	{
		// I have no memory of them yet.
		// There was no fight. Am I the aggressor ?
		CItem * pTargMemory = pTarg->Memory_FindObj( this );
		if ( pTargMemory != NULL )	// My target remembers me.
		{
			if ( pTargMemory->IsMemoryTypes( MEMORY_IAGGRESSOR ))
				MemTypes = MEMORY_FIGHT|MEMORY_AGGREIVED;
			else if ( pTargMemory->IsMemoryTypes( MEMORY_HARMEDBY|MEMORY_SAWCRIME ))
				MemTypes = MEMORY_FIGHT|MEMORY_IAGGRESSOR;
			else
				MemTypes = MEMORY_FIGHT;
		}
		else
		{
			MemTypes = MEMORY_FIGHT|MEMORY_IAGGRESSOR;
		}
		pMemory = Memory_CreateObj( pTarg, MemTypes );
	}
	else
	{
		// I have a memory of them.
		bool fMemPrvType = pMemory->IsMemoryTypes(MEMORY_FIGHT);
		if ( pMemory->IsMemoryTypes(MEMORY_AGGREIVED|MEMORY_SAWCRIME))
		{
			MemTypes = MEMORY_FIGHT;
		}
		else
		{
			MemTypes = MEMORY_FIGHT|MEMORY_IAGGRESSOR;
		}
		// Update the fights status
		Memory_AddTypes( pMemory, MemTypes );
		if ( fMemPrvType ) 
			return;
	}

#ifdef _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "Fight_Start '%s' attacks '%s', type 0%x\n", GetName(), pTarg->GetName(), MemTypes ));
	}
#endif

	if ( IsClient())
	{
		// This may be a useless command. How do i say the fight is over ?
		// This causes the funny turn to the target during combat !
		CCommand cmd;
		cmd.Fight.m_Cmd = XCMD_Fight;
		cmd.Fight.m_dir = 0; // GetDirFlag();
		cmd.Fight.m_AttackerUID = GetUID();
		cmd.Fight.m_AttackedUID = pTarg->GetUID();
		m_pClient->xSendPkt( &cmd, sizeof( cmd.Fight ));
	}
	else
	{
		if ( m_pNPC && m_pNPC->m_Brain == NPCBRAIN_BESERK ) // it will attack everything.
			return;
	}

	if ( GetTopSector()->GetComplexity() > 7 )
		return;	// too busy for this.

	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == m_pClient ) 
			continue;	// we know we are fighting.
		if ( ! pClient->CanSee( this )) 
			continue;

		CGString sMsg;
		sMsg.Format( "*You see %s attacking %s*", GetName(), ( pClient->GetChar() == pTarg ) ? "you" : pTarg->GetName());
		pClient->addObjMessage( sMsg, this, COLOR_RED );
	}
}

void CChar::Noto_Kill( CChar * pKill, bool fAggressiveKill, bool fPetKill )
{
	// I participated in killing pKill CChar
	// I Get some fame/karma.
	// ARGS:
	//  fAggressiveKill = i was the agressor here it seems. (or i commited a crime) 

	ASSERT(pKill);

	// Fight is over now that i have won.
	CItemMemory * pFight = Memory_FindObj( pKill ); // My memory of the fight.
	if ( pFight == NULL )	// I did not know about this.
		return;

	fAggressiveKill |= pFight->IsMemoryTypes( MEMORY_AGGREIVED | MEMORY_SAWCRIME );
	pFight->Delete();

	if ( m_pNPC )
	{
		// Check to see if anything is on the corpse I just killed
		// Clear my WAR flag
		Attack( NULL );
		// m_Act_Targ = ???;
		Skill_Start( NPCACT_LOOTING );
		// If an NPC kills an NPC then it doesn't count.
		if ( pKill->m_pNPC )
			return;
	}

	// Store current notolevel before changes, used to check if notlvl is changed.
	int iPrvLevel = Noto_GetLevel();
	int iKarmaChange = pKill->Stat_Get(STAT_Karma);

	if ( pKill->IsStat( STATF_Criminal ) || fAggressiveKill )
	{
		// No bad karma for killing a criminal or my aggressor.
		if ( iKarmaChange >= 0 ) 
			return;
		fAggressiveKill = true;
	}

	// Check if the victim is a PC, then higher gain/loss.
	if ( pKill->m_pPlayer )
	{
		if ( m_pPlayer && ! fAggressiveKill && ! pKill->IsEvil())
		{
			m_pPlayer->m_Murders++; // Only 'award' kills if victim is a player
			Noto_Criminal();
			Noto_Murder();
		}

		// If killing a 'good' PC we should always loose atleast
		// 500 karma
		if ( iKarmaChange>=0 && iKarmaChange<500 )
			iKarmaChange = 500;
		Noto_Karma( -(iKarmaChange/10));

		if ( ! fPetKill )
		{
			Noto_Fame( pKill->Stat_Get(STAT_Fame) / 10 );
		}
	}
	// Or is it was a NPC, less gain/loss.
	else
	{
		// Always loose atleast 20 karma if you kill a 'good' NPC
		if ( iKarmaChange<100 && iKarmaChange>0)
		{
			Noto_Karma( -20 );
		}
		else
		{
			// Should be harder to gain karma than to loose it.
			if ( iKarmaChange < 0)
			{
				Noto_Karma( - (iKarmaChange/NOTO_FACTOR));
			}
			else
			{
				// Not as harsh penalty as with player chars.
				Noto_Karma( - (iKarmaChange/20));
			}
		}
		if ( ! fPetKill )
		{
			Noto_Fame( pKill->Stat_Get(STAT_Fame) / NOTO_FACTOR );
		}
	}

	Noto_ChangeNewMsg( iPrvLevel ); // Inform on any notlvl changes.
}

void CChar::OnNoticeCrime( CChar * pCriminal, const CChar * pCharMark )
{
	// I noticed a crime.
	ASSERT(pCriminal);
	if ( m_pPlayer )
	{
		// I have the option of attacking the criminal. or calling the guards.
		Memory_AddObjTypes( pCriminal, MEMORY_SAWCRIME );
		return;
	}

	if ( this != pCharMark )	// it's not me.
	{
		// Thieves and beggars don't care.
		if ( m_pNPC->m_Brain == NPCBRAIN_THEIF || m_pNPC->m_Brain == NPCBRAIN_BEGGAR )
			return;
		if ( NPC_IsOwnedBy( pCriminal ))	// I won't rat you out.
			return;
		// Or if the target is evil ?

		// Or if I am evil.
	}
	else
	{
		// I being the victim can retaliate.
		Memory_AddObjTypes( pCriminal, MEMORY_SAWCRIME );
		OnOffendedBy( pCriminal );
	}

	// Alert the guards !!!?
	if ( ! NPC_CanSpeak()) 
		return;	// I can't talk anyhow.

	pCriminal->Noto_Criminal();

	if ( GetCreatureType() != NPCBRAIN_HUMAN )
	{
		// Good monsters don't call for guards outside guarded areas.
		if ( ! m_pArea || ! m_pArea->IsGuarded()) 
			return;
	}

	if ( m_pNPC->m_Brain != NPCBRAIN_GUARD )
	{
		Speak( "Help! Guards a Criminal!" );
	}

	// Find a guard.
	CallGuards();
}

bool CChar::CheckCrimeSeen( SKILL_TYPE SkillToSee, const CChar * pCharMark, const CObjBase * pItem, const TCHAR * pAction )
{
	// I am commiting a crime.
	// Did others see me commit or try to commit the crime.
	bool fSeen = false;

	// Who notices ?
	CWorldSearch AreaChars( GetTopPoint(), UO_MAP_VIEW_SIGHT );
	while (true)
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL ) 
			break;
		if ( this == pChar ) 
			continue;	// I saw myself before.
		if ( ! pChar->CanSeeLOS( this )) 
			continue;
		if ( pChar->GetPrivLevel() < GetPrivLevel()) 
			continue;

		CGString sMsg;
		bool fYour = ( pCharMark == pChar );
		int chanceSee = ( pChar->Stat_Get(STAT_DEX) + pChar->Stat_Get(STAT_INT)) * 50;
		if ( SkillToSee != SKILL_NONE )
		{
			chanceSee = 1000+(pChar->Skill_GetBase(SkillToSee)-Skill_GetBase(SkillToSee));
		}
		else
		{
			chanceSee += 400;
		}

		// the targets chance of seeing.
		if ( pChar->GetPrivLevel() > GetPrivLevel())
		{
			chanceSee = 1001;	// always seen.
		}
		else if ( fYour )
		{
			// Up by 30 % if it's me.
			chanceSee += chanceSee/3;
			if ( chanceSee < 50 ) // always atleast 5% chance.
				chanceSee=50;
		}

		// the bystanders chance of seeing.
		if ( chanceSee < 10 ) // always atleast 1% chance.
			chanceSee=10;

		if ( GetRandVal(1000) > chanceSee )
			continue;

		if ( pAction != NULL )
		{
			if ( pCharMark == NULL )
			{
				sMsg.Format( "You notice %s %s %s.", GetName(), pAction, pItem->GetName());
			}
			else
			{
				sMsg.Format( "You notice %s %s %s%s %s",
					GetName(), pAction, fYour ? "your" : pCharMark->GetName(),
					fYour ? "" : "'s",
					pItem->GetName());
			}
			pChar->ObjMessage( sMsg, this );
		}

		// If a GM sees you it it not a crime.
		if ( pChar->GetPrivLevel() > GetPrivLevel()) 
			continue;
		fSeen = true;

		// They are not a criminal til someone calls the guards !!!
		if ( SkillToSee == SKILL_SNOOPING )
		{
			// Off chance of being a criminal. (hehe)
			if ( ! GetRandVal( g_Serv.m_iSnoopCriminal ))
			{
				pChar->OnNoticeCrime( this, pCharMark );
			}

			if ( fYour && pChar->m_pNPC )
			{
				// start making them angry at you.
static const TCHAR * szTextSnoop[] =
{
	"Get thee hence, busybody!",
	"What do you think your doing ?",
	"Ack, away with you!",
	"Beware or I'll call the guards!",
};
				if ( pChar->NPC_CanSpeak())
				{
					pChar->Speak( szTextSnoop[ GetRandVal( COUNTOF( szTextSnoop )) ] );
				}
				if ( ! GetRandVal(4))
				{
					pChar->m_Act_Targ = GetUID();
					pChar->Skill_Start( NPCACT_FLEE );
				}
			}
		}
		else
		{
			pChar->OnNoticeCrime( this, pCharMark );
		}
	}
	return( fSeen );
}

int CChar::Skill_Snoop( bool fTest, bool fFail )
{
	// SKILL_SNOOPING
	// m_Act_Targ = object to snoop into.
	// RETURN:
	// -1 = no chance. and not a crime
	// -2 = no chance and caught.
	// 0-100 = difficulty = percent chance of failure.

	// Assume the container is not locked.
	CItemContainer * pCont = dynamic_cast <CItemContainer *>(m_Act_Targ.ItemFind());
	if ( pCont == NULL ) 
		return( -1 );

	CChar * pCharMark;
	if ( ! IsTakeCrime( pCont, &pCharMark ) || pCharMark == NULL )
		return( 0 );

	DEBUG_CHECK( ! IsPriv(PRIV_GM));
	if ( ! CanTouch( pCont ))
	{
		SysMessage( "You can't reach it." );
		return( -1 );
	}
	if ( GetTopDist3D( pCharMark ) > 2 )
	{
		SysMessage( "Your mark is too far away." );
		return( -1 );
	}

	bool fForceFail = ( GetPrivLevel() < pCharMark->GetPrivLevel());
	if ( fTest )
	{
		if ( fForceFail )
			return( -2 );
		if ( Skill_Wait())
			return( -1 );
		if ( GetPrivLevel() > pCharMark->GetPrivLevel())
			return( 0 );
		return( pCharMark->Stat_Get(STAT_DEX));
	}

	if ( fForceFail ) 
		fFail = true;
	if ( CheckCrimeSeen( SKILL_SNOOPING, pCharMark, pCont, (fFail)? "attempting to peek into" : "peeking into" ))
	{
		Noto_KarmaChangeMessage( -10, -500 );
	}

	//
	// View the container.
	//
	if ( IsClient() && ! fFail )
	{
		m_pClient->addContainerSetup( pCont );
	}
	return( 0 );
}

int CChar::Skill_Steal( bool fTest, bool fFail )
{
	// m_Act_Targ = object to steal.
	// RETURN:
	// -1 = no chance. and not a crime
	// -2 = no chance and caught.
	// 0-100 = difficulty = percent chance of failure.
	//

	CItem * pItem = m_Act_Targ.ItemFind();
	CChar * pCharMark = NULL;
	if ( pItem == NULL )	// on a chars head ? = random steal.
	{
		pCharMark = m_Act_Targ.CharFind();
		if ( pCharMark == NULL )
			return( -1 );
		CItemContainer * pPack = pCharMark->GetPack();
		if ( pPack == NULL )
		{
cantsteal:
			SysMessage( "They have nothing to steal" );
			return( -1 );
		}
		pItem = pPack->ContentFindRandom();
		if ( pItem == NULL )
		{
			goto cantsteal;
		}
	}

	Reveal();	// If we take an item off the ground we are revealed.

	int iDifficulty = 350;
	if ( ! IsTakeCrime( pItem, & pCharMark ))
	{
		if ( fTest ) return( 0 );
		SysMessage( "No need to steal this" );
		iDifficulty = 0;
	}
	else
	{
		if ( pCharMark != NULL )
		{
			if ( GetTopDist3D( pCharMark ) > 2 )
			{
				SysMessage( "Your mark is too far away." );
				return( -1 );
			}
			int iDexMark = pCharMark->Stat_Get(STAT_DEX);
			iDifficulty = iDexMark +
				pCharMark->Skill_GetAdjusted( SKILL_STEALING )/5 +
				IMULDIV( pItem->GetWeight(), 4, WEIGHT_UNITS );
			if ( pItem->IsEquipped())
			{
				// This is REALLY HARD to do.
				iDifficulty += iDexMark/2 + pCharMark->Stat_Get(STAT_INT);
			}
		}
	}

	if ( pItem->IsEquipped() && pItem->GetEquipLayer() == LAYER_PACK )
	{
		SysMessage( "You can't steal peoples backpacks." );
		return( -1 );
	}
	if ( ! CanTouch( pItem ))
	{
		SysMessage( "You can't reach it." );
		return( -1 );
	}
	if ( ! CanMove( pItem ) ||
		! CanCarry( pItem ))
	{
		SysMessage( "That is too heavy." );
		return( -1 );
	}

	// Special case.
	if ( pItem->m_type == ITEM_GAME_PIECE )
		return -1;

	// ??? heavier items should be more difficult.

	if ( pCharMark != NULL )
	{
		if ( GetPrivLevel() < pCharMark->GetPrivLevel())
			iDifficulty = -2;
	}

	if ( fTest ) return( iDifficulty );
	if ( iDifficulty < 0 ) fFail = true;

	if ( ! fFail )
	{
		pItem->m_Attr &= ~ATTR_OWNED;	// Now it's mine
		CItemContainer * pPack = GetPack();
		if ( pItem->GetParent() != pPack && pPack )
		{
			pItem->RemoveFromView();
			// Put in my invent.
			pPack->ContentAdd( pItem );
		}
	}

	if ( iDifficulty == 0 )
		return( 0 );	// Too easy to be bad. hehe

	// You should only be able to go down to -1000 karma by stealing.
	if ( CheckCrimeSeen( SKILL_STEALING, pCharMark, pItem, (fFail)? "attempting to steal" : "stealing" ))
	{
		Noto_KarmaChangeMessage( -100, -1000 );
	}
	return( 0 );
}

SKILL_TYPE CChar::GetWeaponSkill() const
{
	CItem * pWeapon = m_weapon.ItemFind();
	if ( pWeapon == NULL )
		return( SKILL_WRESTLING );
	return( pWeapon->Weapon_GetSkill());
}

int CChar::SetWeaponSwingTimer()
{
	// We have just equipped the weapon or gone into War mode.
	// Set the swing timer for the weapon or on us for .
	// RETURN: tenths of a sec.

	// Base speed is just your DEX range=40 to 0
	int iWaitTime = IMULDIV( 100 - Stat_Get(STAT_DEX), 40, 100 );
	if ( iWaitTime < 0 )	// no-one needs to be this fast.
		iWaitTime = 5;
	else
		iWaitTime += 5;

	// Speed of the weapon as well effected by strength (minor).

	SKILL_TYPE iSkill;
	CItem * pWeapon = m_weapon.ItemFind();
	if ( pWeapon != NULL )
	{
		int iWeaponWait = ( pWeapon->GetWeight() * 10 ) / ( 4 * WEIGHT_UNITS );	// tenths of a stone.
		if ( pWeapon->GetEquipLayer() == LAYER_HAND2 )	// 2 handed
		{
			iWeaponWait += iWaitTime/2;
		}
		iWaitTime += iWeaponWait;
		iSkill = pWeapon->Weapon_GetSkill();
	}
	else
	{
		iWaitTime += 2;
		iSkill = SKILL_WRESTLING;
	}

	switch ( GetActiveSkill())
	{
	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:
		if ( iSkill != GetActiveSkill())
		{
			Skill_Start( iSkill );
		}
		SetTimeout( iWaitTime * TICK_PER_SEC / 10 );
	}
	return( iWaitTime );
}

int CChar::GetTargetHitDifficulty( SKILL_TYPE skill ) const
{
	// What is our chance of hitting our target?
	// This will be compared to our current weapon skill level.
	// There should always be a bit of a chance. (even if we suck)
	// Adjust the value for our experience curve level.
	// RETURN: 0-100 difficulty.

	CChar * pCharTarg = m_Act_Targ.CharFind();
	if ( pCharTarg == NULL )
	{
		return( GetRandVal(31)); // must be a training dummy
	}

	// Frozen targets should be easy.
	if ( pCharTarg->IsStat( STATF_Sleeping | STATF_Freeze | STATF_Stone ))
		return( 20 );

	int iSkillVal = Skill_GetAdjusted( skill );

	// Offensive value mostly based on your skill and TACTICS.
	// 0 - 1000
	int iSkillAttack = ( iSkillVal + Skill_GetAdjusted( SKILL_TACTICS )) / 2;
	// int iSkillAttack = ( iSkillVal * 3 + Skill_GetAdjusted( SKILL_TACTICS )) / 4;

	// Defensive value mostly based on your tactics value and random DEX,
	// 0 - 1000
	int iSkillDefend = pCharTarg->Skill_GetAdjusted( SKILL_TACTICS );

	// Make it easier to hit people havin a bow or crossbow due to the fact that its
	// not a very "mobile" weapon, nor is it fast to change position while in
	// a fight etc. Just use 90% of the statvalue when defending so its easier
	// to hit than defend == more fun in combat.

	if ( pCharTarg->GetWeaponSkill() == SKILL_ARCHERY &&
		skill != SKILL_ARCHERY )
		// The defender uses ranged weapon and the attacker is not.
		// Make just a bit easier to hit.
		iSkillDefend = ( iSkillDefend + pCharTarg->m_StatStam*9 ) / 2;
	else
		// The defender is using a nonranged, or they both use bows.
		iSkillDefend = ( iSkillDefend + pCharTarg->m_StatStam*10 ) / 2;

	int iDiff = ( iSkillAttack - iSkillDefend ) / 5;

	iDiff = ( iSkillVal - iDiff ) / 10;
	if ( iDiff < 0 ) iDiff = 0;	// just means it's very easy.
	else if ( iDiff > 100 ) iDiff = 100;	// just means it's very hard.
	return( iDiff );
}

void CChar::CallGuards()
{
	// I just yelled for guards.

	// Am i in guarded zone ?
	if ( ! m_pArea )
		return;
	if ( ! m_pArea->IsGuarded())
		return;

	// Is there a free guard near by ?
	CObjUID PrevGuardTarg;
	CChar * pGuard = NULL;
	CWorldSearch AreaGuard( GetTopPoint(), UO_MAP_VIEW_RADAR );
	while (true)
	{
		pGuard = AreaGuard.GetChar();
		if ( pGuard == NULL )
		{
			break;
		}
		if ( pGuard->m_pNPC == NULL || pGuard->m_pNPC->m_Brain != NPCBRAIN_GUARD )
			continue;
		if ( pGuard->IsStat( STATF_War ))
		{
			if ( PrevGuardTarg == pGuard->m_Act_Targ )	// take him off this case.
				break;
			PrevGuardTarg = pGuard->m_Act_Targ;
			continue;	// busy.
		}
		break;
	}

	// Is there anything for him to see ?
	CWorldSearch AreaCrime( GetTopPoint(), UO_MAP_VIEW_SIGHT );
	while (true)
	{
		CChar * pChar = AreaCrime.GetChar();
		if ( pChar == NULL ) 
			return;

		// Never respond to guardcalls if the criminal is outside a guarded zone.
		if ( ! pChar->m_pArea->IsGuarded()) 
			continue;

		// never respond if a criminal can't be found.
		if ( Memory_FindObjTypes( pChar, MEMORY_SAWCRIME ))
		{
			pChar->Noto_Criminal();
		}
		else if ( ! pChar->IsCriminal())
			continue;

		if ( pGuard == NULL )
		{
			// Spawn a new guard.
			pGuard = CChar::CreateNPC( NPCID_Man_Guard );

			// Spawned guards should go away after x minutes.
			pGuard->Spell_Effect_Create( SPELL_Summon, LAYER_SPELL_Summon, 30, g_Serv.m_iGuardLingerTime );

			ASSERT(pChar->GetTopPoint().IsValid());
			pGuard->Spell_Teleport( pChar->GetTopPoint(), false, false );
		}

		if ( pGuard->NPC_LookAtCharGuard( pChar )) 
			return;
	}
}

bool CChar::Attack( const CChar * pCharTarg )
{
	// We want to attack some one.
	// But they won't notice til we actually hit them.
	// This is just my intent.
	// RETURN:
	//  true = new attack is accepted.

	if ( pCharTarg == NULL ||
		pCharTarg == this ||
		pCharTarg->IsStat( STATF_DEAD ) ||
		pCharTarg->IsDisconnected() ||
		IsStat( STATF_DEAD ))
	{
		// Our target is gone.
		m_Act_Targ.ClearUID();
		if ( m_pPlayer ) 
			return( false );

		// Turn war mode off if this is an NPC.
		ClearStat( STATF_War );
		Skill_Start( SKILL_NONE );
		UpdateMode();
		return( false );
	}

	if ( GetPrivLevel() <= PLEVEL_Guest &&
		pCharTarg->m_pPlayer &&
		pCharTarg->GetPrivLevel() > PLEVEL_Guest )
	{
		SysMessage( "Your guest curse prevents you from taking this action" );
		return( false );
	}

	// Record the start of the fight.
	Fight_Start( pCharTarg );

	// I am attacking. (or defending)
	SetStat( STATF_War );
	bool fTargChange = ( m_Act_Targ != pCharTarg->GetUID());
	m_Act_Targ = pCharTarg->GetUID();
	UpdateDir( pCharTarg );

	// Skill interruption ?
	SKILL_TYPE WeaponSkill = GetWeaponSkill();
	if ( GetActiveSkill() != WeaponSkill || fTargChange )
	{
		Skill_Start( WeaponSkill );
	}

	return( true );
}

void CChar::OnOffendedBy( CChar * pCharSrc )
{
	// i notice a Crime or attack against me ..
	// Attack back.

	if ( IsStat( STATF_War ) && m_Act_Targ.CharFind())
	{
		// In war mode with a valid target.
		if ( m_pPlayer )
			return;
		if ( GetRandVal( 10 ))
			return;
		// NPC will Change targets.
	}

	// I will Auto-Defend myself.
	Attack( pCharSrc );
}

bool CChar::OnAttackedBy( CChar * pCharSrc, bool fCommandPet )
{
	// We have been attacked in some way by this CChar.
	// They may have just commanded their pet to attack me.
	// Cast a bad spell at me.
	// Hit me with projectile.
	// Might not actually be doing any real damage.
	//
	// RETURN: true = ok.
	//  false = we are immune to this char ! (or they to us)

	if ( pCharSrc == NULL ) 
		return true;	// field spell ?
	if ( pCharSrc == this ) 
		return true;	// self induced

	pCharSrc->Reveal();	// fix invis exploit

	if ( pCharSrc->IsStat( STATF_INVUL ) && ! pCharSrc->IsPriv( PRIV_GM ))
	{
		// Can't do any damage either.
		pCharSrc->SysMessage( "The attack is magically blocked" );
		return( false );
	}

	// Am i already attacking the source anyhow
	if ( IsStat( STATF_War ) && m_Act_Targ == pCharSrc->GetUID())
		return true;

	// Make sure everyone knows they attacked first. (if it applies)
	pCharSrc->Fight_Start( this );

	// Are they a criminal for it ? Is attacking me a crime ?

	if ( GetNotoFlag(pCharSrc) == NOTO_GOOD )
	{
		OnNoticeCrime( pCharSrc, this );
		// If it is a pet then this a crime others can report.
		if ( ! NPC_CanSpeak())
		{
			pCharSrc->CheckCrimeSeen( SKILL_NONE, this, NULL, NULL );
		}
	}
	else
	{
		Memory_AddObjTypes( pCharSrc, MEMORY_HARMEDBY );
	}

	if ( ! fCommandPet )
	{
		OnOffendedBy( pCharSrc );
	}

	return( true );
}

enum ARMOR_TYPE
{
	ARMOR_HEAD,
	ARMOR_NECK,
	ARMOR_BACK,
	ARMOR_CHEST,
	ARMOR_ARMS,
	ARMOR_HANDS,
	ARMOR_LEGS,
	ARMOR_FEET,
	ARMOR_QTY,
};

static const WORD ArmorCoverage[ARMOR_QTY] =	// Percentage of body area
{
	10,		// ARMOR_HEAD,
	5,		// ARMOR_NECK,
	10,		// ARMOR_BACK,
	30,		// ARMOR_CHEST
	10,		// ARMOR_ARMS,
	10,		// ARMOR_HANDS
	20,		// ARMOR_LEGS,
	5,		// ARMOR_FEET,
};

static const LAYER_TYPE ArmorLayerHead[] = { LAYER_HELM, LAYER_NONE };		// ARMOR_HEAD,
static const LAYER_TYPE ArmorLayerNeck[] = { LAYER_COLLAR, LAYER_NONE };		// ARMOR_NECK,
static const LAYER_TYPE ArmorLayerBack[] = { LAYER_SHIRT, LAYER_CHEST, LAYER_TUNIC, LAYER_CAPE, LAYER_ROBE, LAYER_NONE };	// ARMOR_BACK,
static const LAYER_TYPE ArmorLayerChest[] = { LAYER_SHIRT, LAYER_CHEST, LAYER_TUNIC, LAYER_ROBE, LAYER_NONE };		// ARMOR_CHEST
static const LAYER_TYPE ArmorLayerArms[] = { LAYER_ARMS, LAYER_CAPE, LAYER_ROBE, LAYER_NONE };		// ARMOR_ARMS,
static const LAYER_TYPE ArmorLayerHands[] = { LAYER_GLOVES,	LAYER_NONE };	// ARMOR_HANDS
static const LAYER_TYPE ArmorLayerLegs[] = { LAYER_PANTS, LAYER_SKIRT, LAYER_HALF_APRON, LAYER_ROBE, LAYER_LEGS, LAYER_NONE };	// ARMOR_LEGS,
static const LAYER_TYPE ArmorLayerFeet[] = { LAYER_SHOES, LAYER_LEGS, LAYER_NONE };	// ARMOR_FEET,

static const LAYER_TYPE * ArmorLayers[ARMOR_QTY] =	// layers covering the armor zone.
{
	ArmorLayerHead,		// ARMOR_HEAD,
	ArmorLayerNeck,		// ARMOR_NECK,
	ArmorLayerBack,		// ARMOR_BACK,
	ArmorLayerChest,	// ARMOR_CHEST
	ArmorLayerArms,		// ARMOR_ARMS,
	ArmorLayerHands,	// ARMOR_HANDS
	ArmorLayerLegs,		// ARMOR_LEGS,
	ArmorLayerFeet,		// ARMOR_FEET,
};

int CChar::CalcArmorDefense( void )
{
	// When armor is added or subtracted check this.
	int iDefenseTotal = 0;
	int iArmorCount = 0;
	WORD ArmorRegionMax[ARMOR_QTY];
	memset( ArmorRegionMax, 0, sizeof(ArmorRegionMax));
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		int iDefense = pItem->Armor_GetDefense();

		if (( pItem->m_type == ITEM_SPELL || pItem->IsArmor()) &&
			pItem->m_itSpell.m_spell == SPELL_Protection )
		{
			// Effect of protection spells.
			iDefenseTotal += ( pItem->m_itSpell.m_skilllevel * 100 );
		}

		switch ( pItem->GetEquipLayer())
		{
		case LAYER_HELM:		// 6
			ArmorRegionMax[ ARMOR_HEAD ] = max( ArmorRegionMax[ ARMOR_HEAD ], iDefense );
			break;
		case LAYER_COLLAR:	// 10 = gorget or necklace.
			ArmorRegionMax[ ARMOR_NECK ] = max( ArmorRegionMax[ ARMOR_NECK ], iDefense );
			break;
		case LAYER_SHIRT:
		case LAYER_CHEST:	// 13 = armor chest
		case LAYER_TUNIC:	// 17 = jester suit
			ArmorRegionMax[ ARMOR_CHEST ] = max( ArmorRegionMax[ ARMOR_CHEST ], iDefense );
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense );
			break;
		case LAYER_ARMS:		// 19 = armor
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense );
			break;
		case LAYER_PANTS:
		case LAYER_SKIRT:
		case LAYER_HALF_APRON:
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense );
			break;
		case LAYER_SHOES:
			ArmorRegionMax[ ARMOR_FEET ] = max( ArmorRegionMax[ ARMOR_FEET ], iDefense );
			break;
		case LAYER_GLOVES:	// 7
			ArmorRegionMax[ ARMOR_HANDS ] = max( ArmorRegionMax[ ARMOR_HANDS ], iDefense );
			break;
		case LAYER_CAPE:		// 20 = cape
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense );
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense );
			break;
		case LAYER_ROBE:		// 22 = robe over all.
			ArmorRegionMax[ ARMOR_CHEST ] = max( ArmorRegionMax[ ARMOR_CHEST ], iDefense );
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense );
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense );
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense );
			break;
		case LAYER_LEGS:
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense );
			ArmorRegionMax[ ARMOR_FEET ] = max( ArmorRegionMax[ ARMOR_FEET ], iDefense );
			break;

		case LAYER_HAND2:
			// Shield effect.
			if ( pItem->m_type == ITEM_ARMOR )
			{
				iDefenseTotal += iDefense * Skill_GetAdjusted(SKILL_PARRYING) / 10;
			}
			continue;

		default:
			continue;
		}

		iArmorCount ++;
	}
	if ( iArmorCount )
	{
		for ( int i=0; i<ARMOR_QTY; i++ )
		{
			iDefenseTotal += ArmorCoverage[i] * ArmorRegionMax[i];
		}
	}
	return( iDefenseTotal / 100 );
}

int CChar::OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType )
{
	// Someone hit us.
	// uType =
	//	DAMAGE_GOD		0x01	// Nothing can block this.
	//	DAMAGE_HIT		0x02	// Physical hit of some sort.
	//	DAMAGE_MAGIC	0x04	// Magic blast of some sort. (we can be immune to magic to some extent)
	//	DAMAGE_POISON	0x08	// Or biological of some sort ? (HARM spell)
	//	DAMAGE_FIRE		0x10	// Fire damage of course.  (Some creatures are immune to fire)
	//	DAMAGE_ELECTRIC 0x20	// lightning.
	//  DAMAGE_DRAIN
	//	DAMAGE_GENERAL	0x80	// All over damage. As apposed to hitting just one point.
	//
	// RETURN: damage done. -1 = dead. 0 = no damage.

	if ( iDmg <= 0 ) 
		return( 0 );

	if ( uType & ( DAMAGE_ELECTRIC | DAMAGE_HIT | DAMAGE_FIRE | DAMAGE_MAGIC ))
	{
		ClearStat( STATF_Freeze );	// remove paralyze.
	}
	if ( uType & DAMAGE_HIT )
	{
		// A physical blow of some sort.
		// Try to allow the armor or shield to take some damage.
		Reveal();

		// Check for reactive armor.
		if ( IsStat( STATF_Reactive ) && ! ( uType & DAMAGE_GOD ))
		{
			// reflect some damage back.
			if ( pSrc != NULL && GetTopDist3D( pSrc ) <= 2 )
			{
				int iRefDam = iDmg / 2;
				int iMaxRefDam = Skill_GetAdjusted(SKILL_MAGERY)/75;
				iRefDam = min( iRefDam, iMaxRefDam );
				iDmg -= iRefDam;
				pSrc->OnTakeDamage( iRefDam, this, uType );
				pSrc->Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 9, 6 );
			}
		}

		// absorbed by armor ?
		if ( ! ( uType & DAMAGE_GENERAL ))
		{
			iDmg = OnTakeDamageHitPoint( iDmg, pSrc, uType );
			iDmg -= GetRandVal( m_pDef->m_defense );
		}
		else if ( ! ( uType & DAMAGE_GOD ))
		{
			// general overall damage.
			iDmg -= GetRandVal( m_defense + m_pDef->m_defense );
			// ??? take some random damage to my equipped items.

		}
	}

	if ( IsStat( STATF_INVUL ))
		return( 0 );
	if ( ! ( uType & DAMAGE_GOD ) && m_pArea &&
		( m_pArea->IsFlag( REGION_FLAG_SAFE ) || IsStat(STATF_Stone)))	// can't hurt us anyhow.
		return( 0 );
	if ( m_StatHealth <= 0 )	// Already dead.
		return( -1 );

	if ( uType & DAMAGE_FIRE )
	{
		if ( m_pDef->Can(CAN_C_FIRE_IMMUNE)) // immune to the fire part.
		{
			// If there is any other sort of damage then dish it out as well.
			if ( ! ( uType & ( DAMAGE_HIT | DAMAGE_POISON | DAMAGE_ELECTRIC )))
				return( 0 );	// No effect.
			iDmg /= 2;
		}
	}
	if ( iDmg <= 0 )
		return( 0 );

	// defend myself.
	if ( ! OnAttackedBy( pSrc, false ))
		return( 0 );

	// Make blood depending on hit damage.
	// ??? assuming the creature has blood ?
	ITEMID_TYPE id = ITEMID_NOTHING;
	if ( iDmg > 10 )
	{
		id = (ITEMID_TYPE)( ITEMID_BLOOD1 + GetRandVal(ITEMID_BLOOD6-ITEMID_BLOOD1));
	}
	else if ( GetRandVal( iDmg ) > 5 )
	{
		id = ITEMID_BLOOD_SPLAT;	// minor hit.
	}
	if ( id && ! IsStat( STATF_Conjured ) && ( uType & DAMAGE_HIT ))	// A blow of some sort.
	{
		CItem * pBlood = CItem::CreateBase( id );
		if ( m_pNPC && m_pNPC->m_Brain == NPCBRAIN_UNDEAD )
		{
			pBlood->SetColor( COLOR_GREEN_DARK );
		}
		pBlood->SetDecayTime( 7*TICK_PER_SEC );
		pBlood->MoveTo( GetTopPoint());
	}

	UpdateStats( STAT_STR, -iDmg );
	if ( m_StatHealth <= 0 )
	{
		// We will die from this...make sure the killer is set correctly...if we don't do this, the person we are currently
		// attacking will get credit for killing us.
		if ( pSrc && m_Act_Targ != pSrc->GetUID())
		{
			m_Act_TargPrv = m_Act_Targ;
			m_Act_Targ = pSrc->GetUID();
		}
		return( -1 );
	}

	SoundChar( CRESND_GETHIT );
	if ( m_atFight.m_War_Swing_State != WAR_SWING_SWINGING )	// Not interrupt my swing.
	{
		UpdateAnimate( ANIM_GET_HIT );
	}
	return( iDmg );
}

int CChar::OnTakeDamageHitPoint( int iDmg, CChar * pSrc, DAMAGE_TYPE uType )
{
	// Point strike type damage.
	// Deflect some damage with shield or weapon ?

	ASSERT( ! ( uType & DAMAGE_GENERAL ));

	if ( IsStat(STATF_HasShield) &&
		(uType & DAMAGE_HIT) &&
		! (uType & (DAMAGE_GOD|DAMAGE_ELECTRIC)))
	{
		CItem * pShield = LayerFind( LAYER_HAND2 );
		if ( pShield != NULL && Skill_UseQuick( SKILL_PARRYING, GetRandVal((pSrc!=NULL) ? (pSrc->Skill_GetBase(SKILL_TACTICS)/10) : 100 )))
		{
			// Damage the shield.
			// Let through some damage.
			int iDefense = GetRandVal( pShield->Armor_GetDefense() / 2 );
			if ( pShield->OnTakeDamage( min( iDmg, iDefense ), pSrc, uType ))
			{
				SysMessage( "You parry the blow" );
			}
			iDmg -= iDefense; // damage absorbed by shield
		}
	}

	// Assumes humanoid type body. Orcs, Headless, trolls, humans etc.
	// ??? If not humanoid ??
	if ( m_pDef->Can( CAN_C_NONHUMANOID ))
	{
		// ??? we may want some sort of message ?
		return( iDmg );
	}

	// Where was the hit ?
	int iHitRoll = GetRandVal( 100 ); // determine area of body hit
	int iHitArea=0;
	for ( ; iHitArea<ARMOR_QTY-1; iHitArea++ )
	{
		iHitRoll -= ArmorCoverage[iHitArea];
		if ( iHitRoll < 0 ) break;
	}

	if ( (uType & (DAMAGE_HIT|DAMAGE_FIRE|DAMAGE_ELECTRIC)) && ! (uType & (DAMAGE_GENERAL|DAMAGE_GOD)))
	{

		static const TCHAR * Hit_Head1[][2] =
		{
			"hit you straight in the face!",	"You hit %s straight in the face!",
			"hits you in the head!",			"You hit %s in the head!",
			"hit you square in the jaw!",		"You hit %s square in the jaw!",
		};
		static const TCHAR * Hit_Head2[][2] =
		{
			"scores a stunning blow to your head!",		"You score a stunning blow to %ss head!",
			"smashes a blow across your face!",			"You smash a blow across %ss face!",
			"scores a terrible hit to your temple!",	"You score a terrible hit to %ss temple!",
		};
		static const TCHAR * Hit_Chest1[][2] =
		{
			"hits your Chest!",					"You hit %ss Chest!",
			"lands a blow to your stomach!",	"You land a blow to %ss stomach!",
			"hits you in the ribs!",			"You hit %s in the ribs!",
		};
		static const TCHAR * Hit_Chest2[][2] =
		{
			"lands a terrible blow to your chest!",	"You land a terrible blow to %ss chest!",
			"knocks the wind out of you!",			"You knock the wind out of %s!",
			"smashed you in the rib cage!",			"You smash %s in the rib cage!",
		};
		static const TCHAR * Hit_Arm[][2] =
		{
			"hits your left arm!",		"You hit %ss left arm!",
			"hits your right arm!",		"You hit %ss right arm!",
			"hits your right arm!",		"You hit %ss right arm!",
		};
		static const TCHAR * Hit_Legs[][2] =
		{
			"hits your left thigh!",	"You hit %ss left thigh!",
			"hits your right thigh!",	"You hit %ss right thigh!",
			"hits you in the groin!",	"You hit %s in the groin!",
		};
		static const TCHAR * Hit_Hands[][2] =	// later include exclusion of left hand if have shield
		{
			"hits your left hand!",		"You hit %ss left hand!",
			"hits your right hand!",	"You hit %ss right hand!",
			"hits your right hand!",	"You hit %ss right hand!",
		};
		static const TCHAR * Hit_Neck1[2] =
		{
			"hits you in the throat!",		"You hit %s in the throat!",
		};
		static const TCHAR * Hit_Neck2[2] =
		{
			"smashes you in the throat!",	"You smash %s in the throat!",
		};
		static const TCHAR * Hit_Back[2] =
		{
			"scores a hit to your back!",	"You score a hit to %ss back!",
		};
		static const TCHAR * Hit_Feet[2] =
		{
			"hits your foot!",				"You hit %s foot!",
		};

		int iMsg = GetRandVal(3);
		const TCHAR ** ppMsg;
		switch ( iHitArea )
		{
		case ARMOR_HEAD:
			ppMsg = (iDmg>10) ? Hit_Head2[iMsg] : Hit_Head1[iMsg] ;
			break;
		case ARMOR_NECK:
			ppMsg = (iDmg>10) ? Hit_Neck2 : Hit_Neck1 ;
			break;
		case ARMOR_BACK:
			ppMsg = Hit_Back;
			break;
		case ARMOR_CHEST:
			ppMsg = (iDmg>10) ? Hit_Chest2[iMsg] : Hit_Chest1[iMsg] ;
			break;
		case ARMOR_ARMS:
			ppMsg = Hit_Arm[iMsg];
			break;
		case ARMOR_HANDS:
			ppMsg = Hit_Hands[iMsg];
			break;
		case ARMOR_LEGS:
			ppMsg = Hit_Legs[iMsg];
			break;
		case ARMOR_FEET:
			ppMsg = Hit_Feet;
			break;
		default:
			ASSERT(0);
			break;
		}

		if ( pSrc != this )
		{
			if ( IsPriv(PRIV_DETAIL))
			{
				SysMessagef( "%s %s", ( pSrc == NULL ) ? "It" : pSrc->GetName(), ppMsg[0] );
			}
			if ( pSrc != NULL && pSrc->IsPriv(PRIV_DETAIL))
			{
				pSrc->SysMessagef( ppMsg[1], GetName());
			}
		}
	}

	// Do damage to my armor. (what if it is empty ?)

	int iMaxCoverage = 0;	// coverage at the hit zone.

	CItem * pArmorNext;
	for ( CItem * pArmor = GetContentHead(); pArmor != NULL; pArmor = pArmorNext )
	{
		pArmorNext = pArmor->GetNext();

		if ( pArmor->m_type == ITEM_SPELL &&
			pArmor->m_itSpell.m_spell == SPELL_Protection )
		{
			// Effect of protection spells.
			iMaxCoverage = max( iMaxCoverage, pArmor->m_itSpell.m_skilllevel );
			continue;
		}

		LAYER_TYPE layer = pArmor->GetEquipLayer();
		if ( ! CItemBase::IsVisibleLayer( layer )) 
			continue;

		for ( int i=0; ArmorLayers[iHitArea][i] != LAYER_NONE; i++ ) // layers covering the armor zone.
		{
			if ( ArmorLayers[iHitArea][i] == layer )
			{
				iMaxCoverage = max( iMaxCoverage, pArmor->Armor_GetDefense());
				pArmor->OnTakeDamage( iDmg, pSrc, uType );
				break;
			}
		}
	}

	// iDmg = ( iDmg * GW_GetSCurve( iMaxCoverage - iDmg, 10 )) / 100;
	iDmg -= GetRandVal( iMaxCoverage );
	return( iDmg );
}

bool CChar::HitTry()
{
	ASSERT( IsStat( STATF_War ));

	SKILL_TYPE sk = GetActiveSkill();
	ASSERT( sk == SKILL_ARCHERY ||
		sk == SKILL_FENCING ||
		sk == SKILL_MACEFIGHTING ||
		sk == SKILL_SWORDSMANSHIP ||
		sk == SKILL_WRESTLING );
	ASSERT( m_atFight.m_War_Swing_State >= WAR_SWING_READY );

	// Try to hit my target. I'm ready.
	// RETURN:
	//  true = try to hit again.
	//  false = invalid target. (fall out of war mode)

	if ( ! m_Act_Targ.IsValidUID())
		return( false );

	CChar * pCharTarg = m_Act_Targ.CharFind();
	if ( pCharTarg != NULL )
	{
		int iRet = Hit( pCharTarg );
		if ( iRet == 2 )	// probably too far away
			return( true );

		if ( iRet )	// Took my swing
		{
			// Get experience for it.
			Skill_Experience( sk, m_Act_Difficulty );
			// Assume I want to continue hitting
			// Swing again. (start swing delay)
			Skill_Start( GetWeaponSkill());
			return( true );
		}
	}

	// Might be dead ? Clear this.
	Attack( NULL );
	return( false );
}

int CChar::Hit( CChar * pCharTarg )
{
	// Attempt to hit our target.
	// pCharTarg = the target.
	// RETURN:
	//  0 = target is invalid.
	//  1 = took my swing.
	//  2 = can't take my swing right now.

	if ( pCharTarg->IsStat( STATF_DEAD ))
		return( 0 );

	int dist = GetTopDist3D( pCharTarg );
	if ( dist > UO_MAP_VIEW_RADAR )
		return( 0 );

	CItem * pWeapon = m_weapon.ItemFind();
	ITEMID_TYPE AmmoID;
	if ( GetActiveSkill() == SKILL_ARCHERY )
	{
		if ( dist <= 1 )
		{
			// ??? the bow is acting like a (poor) blunt weapon at this range.
		}

		if ( dist > 10 )
			return( 2 );	// can't hit now.
		if ( ! CanSeeLOS( pCharTarg ))
			return( 2 );

		if ( IsStat( STATF_HasShield ))
		{
			SysMessage( "Your shield prevents you from using your bow correctly." );
			return( 0 );
		}

		// Consume the bolts/arrows
		AmmoID = pWeapon->Armor_IsXBow() ? ITEMID_XBolt : ITEMID_Arrow;
		if ( m_pPlayer && ContentConsume( AmmoID ))
		{
			SysMessage( "You have no ammunition" );
			Skill_Start( SKILL_NONE );
			return( 0 );
		}

		Reveal();
		UpdateDir( pCharTarg );
		UpdateAnimate( ANIM_ATTACK_WEAPON );
		pCharTarg->Effect( EFFECT_BOLT, ( AmmoID == ITEMID_Arrow ) ? ITEMID_ArrowX : ITEMID_XBoltX, this, 16, 0, false );
	}
	else
	{
		// if ( ! CanTouch( pCharTarg )) return( 2 );
		if ( m_atFight.m_War_Swing_State == WAR_SWING_READY )
		{
			if ( dist > 1 )
				return( 2 );	// can't hit now.

			Reveal();
			UpdateDir( pCharTarg );
			UpdateAnimate( ANIM_ATTACK_WEAPON );

			m_atFight.m_War_Swing_State = WAR_SWING_SWINGING;
			SetTimeout(1*TICK_PER_SEC);	// try again sooner.
			return( 2 );
		}

		// done with the swing.
		Reveal();
		// UpdateAnimate( ANIM_ATTACK_WEAPON, true, true );	// recoil anim
		if ( dist > 2 )
			return( 2 );	// can't hit now.
	}

	// We made our swing. so we must recoil.
	m_atFight.m_War_Swing_State = WAR_SWING_EQUIPPING;

	if ( ! pCharTarg->OnAttackedBy( this, false ))
		return( 0 );

	// Use up some stamina to hit ?
	int iWeaponWeight = (( pWeapon ) ? ( pWeapon->GetWeight()/WEIGHT_UNITS + 1 ) : 1 );
	if ( ! GetRandVal( 10 + iWeaponWeight ))
	{
		UpdateStats( STAT_DEX, -1 );
	}

	// if GUARD instakill is activated, no skillcheck is needed and we do maximum damage.
	if ( m_pNPC &&
		m_pNPC->m_Brain == NPCBRAIN_GUARD &&
		g_Serv.m_fGuardsInstantKill )
	{
		return( pCharTarg->OnTakeDamage( pCharTarg->m_StatHealth*2, this, DAMAGE_HIT | DAMAGE_GOD ) ? 1 : 0 );
	}

	// Check if we hit something;
	if ( m_Act_Difficulty < 0 )
	{
		// We missed. (miss noise)
		if ( GetActiveSkill() == SKILL_ARCHERY )
		{
			// 0x223 = bolt miss or dart ?
			// do some thing with the arrow.
			// AmmoID
			Sound( GetRandVal(2) ? 0x233 : 0x238 );
			// Sometime arrows should be lost when we miss
			if ( GetRandVal(3))
			{
				CItem::CreateBase( AmmoID )->MoveToCheck(pCharTarg->GetTopPoint());
			}
			return( 1 );
		}
		if ( GetActiveSkill() == SKILL_WRESTLING )
		{
			if ( SoundChar( GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 ))
				return( 1 );
		}

		static const WORD Snd_Miss[] =
		{
			0x238, // = swish01
			0x239, // = swish02
			0x23a, // = swish03
		};

		Sound( Snd_Miss[ GetRandVal( COUNTOF( Snd_Miss )) ] );

		if ( IsPriv(PRIV_DETAIL))
		{
			SysMessagef( "You Miss %s", pCharTarg->GetName());
		}
		if ( pCharTarg->IsPriv(PRIV_DETAIL))
		{
			pCharTarg->SysMessagef( "%s missed you.", GetName());
		}
		return( 1 );
	}

	// We hit...
	if ( GetActiveSkill() == SKILL_ARCHERY )
	{
		// There's a chance that the arrow will stick in the target
		if ( !GetRandVal(4))
		{
			CItem * pAmmo = CItem::CreateBase( AmmoID );
			pCharTarg->ItemBounce( pAmmo );
		}
	}

	// Raise skill
	Skill_UseQuick( SKILL_TACTICS, pCharTarg->Skill_GetBase(SKILL_TACTICS)/10 );

	// Hit noise. based on weapon type.
	SoundChar( CRESND_HIT );

	// Calculate base damage.
	int	iDmg = 1;

	// STR bonus on close combat weapons.
	// and DEX bonus on archery weapons
	// A random value of the bonus is added (0-10%)
	int iDmgAdj;
	if ( GetWeaponSkill() != SKILL_ARCHERY )
		iDmgAdj = GetRandVal( Stat_Get(STAT_STR));
	else
		iDmgAdj = GetRandVal( Stat_Get(STAT_DEX));
	iDmg += iDmgAdj / 10;

	if ( pWeapon != NULL )
	{
		iDmg += pWeapon->Weapon_GetAttack();

		// poison weapon ?
		if ( pWeapon->m_itWeapon.m_poison_skill &&
			GetRandVal( 100 ) < pWeapon->m_itWeapon.m_poison_skill )
		{
			// Poison delivered.
			pCharTarg->SetPoison( GetRandVal(pWeapon->m_itWeapon.m_poison_skill), this );
		}

		// damage the weapon ?
		pWeapon->OnTakeDamage( iDmg/4, pCharTarg );
	}
	else
	{
		// Pure wrestling damage.
		// Base type attack for our body. claws/etc
		iDmg += m_pDef->m_attackBase + GetRandVal( m_pDef->m_attackRange );

		// intrinsic attacks ?
		// Poisonous bite/sting ?
		if ( m_pNPC &&
			m_pNPC->m_Brain == NPCBRAIN_MONSTER &&
			Skill_GetBase(SKILL_POISONING) > 300 &&
			GetRandVal( 1000 ) < Skill_GetBase(SKILL_POISONING))
		{
			// Poison delivered.
			pCharTarg->SetPoison( GetRandVal( Skill_GetAdjusted( SKILL_POISONING )), this );
		}
	}

	return( pCharTarg->OnTakeDamage( iDmg, this, DAMAGE_HIT ) ? 1 : 0 );
}

