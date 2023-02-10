//
// CCharFight.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Fight/Criminal actions/Noto.
//

#include "graysvr.h"	// predef header.
#include "CClient.h"

CItemStone * CChar::Guild_Find( MEMORY_TYPE MemType ) const
{
	// Get my guild stone for my guild. even if i'm just a STONEPRIV_CANDIDATE ?
	// ARGS:
	//  MemType == MEMORY_GUILD or MEMORY_TOWN
	if ( ! m_pPlayer )
		return( NULL );
	CItemMemory * pMyGMem = Memory_FindTypes( MemType );
	if ( ! pMyGMem )
		return( NULL );
	CItemStone * pMyStone = dynamic_cast <CItemStone*>( pMyGMem->m_uidLink.ItemFind());
	if ( pMyStone == NULL )
	{
		// Some sort of mislink ! fix it.
		const_cast <CChar*>(this)->Memory_ClearTypes( MemType ); 	// Make them forget they were ever in this guild....again!
		return( NULL );
	}
	return( pMyStone );
}

CStoneMember * CChar::Guild_FindMember( MEMORY_TYPE MemType ) const
{
	// Get my member record for my guild.
	CItemStone * pMyStone = Guild_Find(MemType);
	if ( pMyStone == NULL )
		return( NULL );
	CStoneMember * pMember = pMyStone->GetMember( this );
	if ( pMember == NULL )
	{
		// Some sort of mislink ! fix it.
		const_cast <CChar*>(this)->Memory_ClearTypes( MemType ); 	// Make them forget they were ever in this guild....again!
		return( NULL );
	}
	return( pMember );
}

void CChar::Guild_Resign( MEMORY_TYPE MemType )
{
	// response to "I resign from my guild" or "town"
	// Are we in an active battle ?

	if ( IsStatFlag( STATF_DEAD ))
		return;

	CStoneMember * pMember = Guild_FindMember(MemType);
	if ( pMember == NULL )
		return ;

	if ( pMember->IsPrivMember())
	{
		CItemMemory * pMemFight = Memory_FindTypes( MEMORY_FIGHT );
		if ( pMemFight )
		{
			CItemStone * pMyStone = pMember->GetParentStone();
			ASSERT(pMyStone);
			SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_GUILDRESIGN ), (LPCTSTR) pMyStone->GetTypeName() );
		}
	}

	delete pMember;
}

LPCTSTR CChar::Guild_Abbrev( MEMORY_TYPE MemType ) const
{
	// Get my guild abbrev if i have chosen to turn it on.
	CStoneMember * pMember = Guild_FindMember(MemType);
	if ( pMember == NULL )
		return( NULL );
	if ( ! pMember->IsAbbrevOn())
		return( NULL );
	CItemStone * pMyStone = pMember->GetParentStone();
	if ( pMyStone == NULL ||
		! pMyStone->GetAbbrev()[0] )
		return( NULL );
	return( pMyStone->GetAbbrev());
}

LPCTSTR CChar::Guild_AbbrevBracket( MEMORY_TYPE MemType ) const
{
	// Get my [guild abbrev] if i have chosen to turn it on.
	LPCTSTR pszAbbrev = Guild_Abbrev(MemType);
	if ( pszAbbrev == NULL )
		return( NULL );
	TCHAR * pszTemp = Str_GetTemp();
	sprintf( pszTemp, " [%s]", pszAbbrev );
	return( pszTemp );
}

//*****************************************************************

bool CChar::Noto_IsMurderer() const
{
	return( m_pPlayer && m_pPlayer->m_wMurders > g_Cfg.m_iMurderMinCount );
}

bool CChar::Noto_IsEvil() const
{
	// animals and humans given more leeway.
	if ( Noto_IsMurderer())
		return( true );
	int iKarma = Stat_GetAdjusted(STAT_KARMA);
	switch ( GetNPCBrain() )
	{
	case NPCBRAIN_UNDEAD:
	case NPCBRAIN_MONSTER:
		return( iKarma< 0 );	// I am evil ?
	case NPCBRAIN_BESERK:
		return( true );
	case NPCBRAIN_ANIMAL:
		return( iKarma<= -800 );
	}
	if ( m_pPlayer )
	{
		return( iKarma<g_Cfg.m_iPlayerKarmaEvil );
	}
	return( iKarma <= -3000 );
}

bool CChar::Noto_IsNeutral() const
{
	// Should neutrality change in guarded areas ?
	int iKarma = Stat_GetAdjusted(STAT_KARMA);
	switch ( GetNPCBrain() )
	{
	case NPCBRAIN_MONSTER:
	case NPCBRAIN_BESERK:
		return( iKarma<= 0 );
	case NPCBRAIN_ANIMAL:
		return( iKarma<= 100 );
	}
	if ( m_pPlayer )
	{
		return( iKarma<g_Cfg.m_iPlayerKarmaNeutral );
	}
	return( iKarma<0 );
}

NOTO_TYPE CChar::Noto_GetFlag( const CChar * pCharViewer, bool fAllowIncog ) const
{
	// What is this char to the viewer ?
	// This allows the noto attack check in the client.
	// NOTO_GOOD = it is criminal to attack me.

	if ( fAllowIncog && IsStatFlag( STATF_Incognito ))
	{
		return NOTO_NEUTRAL;
	}

	// Are we in the same party ?
	if ( this != pCharViewer && m_pParty && m_pParty == pCharViewer->m_pParty )
	{
		if ( m_pParty->GetLootFlag(this))
		{
			return(NOTO_GUILD_SAME);
		}
	}

	if ( Noto_IsEvil())
	{
		return( NOTO_EVIL );
	}

	if ( this != pCharViewer ) // Am I checking myself?
	{
		// Check the guild stuff
		CItemStone * pMyTown = Guild_Find(MEMORY_TOWN);
		CItemStone * pMyGuild = Guild_Find(MEMORY_GUILD);
		if ( pMyGuild || pMyTown )
		{
			CItemStone * pViewerGuild = pCharViewer->Guild_Find(MEMORY_GUILD);
			CItemStone * pViewerTown = pCharViewer->Guild_Find(MEMORY_TOWN);
			// Are we both in a guild?
			if ( pViewerGuild || pViewerTown )
			{
				if ( pMyGuild && pMyGuild->IsPrivMember(this))
				{
					if ( pViewerGuild && pViewerGuild->IsPrivMember(pCharViewer))
					{
						if ( pViewerGuild == pMyGuild ) // Same guild?
							return NOTO_GUILD_SAME; // return green
						if ( pMyGuild->IsSameAlignType( pViewerGuild ))
							return NOTO_GUILD_SAME;
						// Are we in different guilds but at war? (not actually a crime right?)
						if ( pMyGuild->IsAtWarWith(pViewerGuild))
							return NOTO_GUILD_WAR; // return orange
					}
					if ( pMyGuild->IsAtWarWith(pViewerTown))
						return NOTO_GUILD_WAR; // return orange
				}
				if ( pMyTown && pMyTown->IsPrivMember(this))
				{
					if ( pViewerGuild && pViewerGuild->IsPrivMember(pCharViewer))
					{
						if ( pMyTown->IsAtWarWith(pViewerGuild))
							return NOTO_GUILD_WAR; // return orange
					}
					if ( pMyTown->IsAtWarWith(pViewerTown))
						return NOTO_GUILD_WAR; // return orange
				}
			}
		}
	}

	if ( IsStatFlag( STATF_Criminal ))	// criminal to everyone.
		return( NOTO_CRIMINAL );

	if ( this != pCharViewer ) // Am I checking myself?
	{
		if ( NPC_IsOwnedBy( pCharViewer, false ))	// All pets are neutral to their owners.
			return( NOTO_NEUTRAL );

		// If they saw me commit a crime or I am their aggressor then
		// criminal to just them.
		CItemMemory * pMemory = pCharViewer->Memory_FindObjTypes( this, MEMORY_SAWCRIME | MEMORY_AGGREIVED );
		if ( pMemory != NULL )
		{
			return( NOTO_CRIMINAL );
		}
	}

	if ( m_pArea && m_pArea->IsFlag(REGION_FLAG_ARENA))
	{
		// everyone is neutral here.
		return( NOTO_NEUTRAL );
	}
	if ( Noto_IsNeutral())
	{
		return( NOTO_NEUTRAL );
	}
	return( NOTO_GOOD );
}

HUE_TYPE CChar::Noto_GetHue( const CChar * pCharViewer, bool fIncog ) const
{
	// What is this char to the viewer ?
	// Represent as a text Hue.
	CVarDefBase *	sVal	= m_TagDefs.GetKey( "NAME.HUE" );
	if ( sVal )
		return  sVal->GetValNum();

	switch ( Noto_GetFlag( pCharViewer, fIncog ))
	{
	case NOTO_GOOD:			return HUE_BLUE_LIGHT;	// Blue
	case NOTO_GUILD_SAME:	return 0x0044;	// Green (same guild)
	case NOTO_NEUTRAL:		return 0x03b2;	// Grey 1 (someone that can be attacked)
	case NOTO_CRIMINAL:		return 0x03b2;	// Grey 2 (criminal)
	case NOTO_GUILD_WAR:	return HUE_ORANGE;	// Orange (enemy guild)
	case NOTO_EVIL:			return 0x0026;	// Red
	}

	return HUE_TEXT_DEF;	// ?Grey
}



LPCTSTR CChar::Noto_GetFameTitle() const
{
	if ( IsStatFlag( STATF_Incognito ))
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
	if ( Stat_GetAdjusted(STAT_FAME) <= 9900 )
		return( "" );

	CCharBase * pCharDef = Char_GetDef();
	if ( pCharDef->IsFemale())
		return( "Lady " );
	return( "Lord " );
}

int CChar::Noto_GetLevel() const
{
	// Paperdoll title for character
	// This is so we can inform user of change in title !

	static const int sm_KarmaLevel[] =
	{ 9900, 5000, 1000, 500, 100, -100, -500, -1000, -5000, -9900 };

	int i=0;
	int iKarma = Stat_GetAdjusted(STAT_KARMA);
	for ( ; i<COUNTOF( sm_KarmaLevel ) && iKarma < sm_KarmaLevel[i]; i++ )
		;

	static const WORD sm_FameLevel[] =
	{ 500, 1000, 5000, 9900 };

	int j =0;
	int iFame = Stat_GetAdjusted(STAT_FAME);
	for ( ; j<COUNTOF( sm_FameLevel ) && iFame > sm_FameLevel[j]; j++ )
		;

	return( ( i * 5 ) + j );
}

LPCTSTR CChar::Noto_GetTitle() const
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

	LPCTSTR pTitle;

	// Murderer ?
	if ( Noto_IsMurderer())
	{
		pTitle = "Murderer";
	}
	else if ( IsStatFlag( STATF_Criminal ))
	{
		pTitle = "Criminal";
	}
	else
	{
		pTitle = g_Cfg.GetNotoTitle( Noto_GetLevel() );
	}

	TCHAR * pTemp = Str_GetTemp();
	sprintf( pTemp, "%s%s%s%s%s",
		(pTitle[0]) ? "The " : "",
		pTitle,
		(pTitle[0]) ? " " : "",
		Noto_GetFameTitle(),
		GetName());

	return( pTemp );
}

void CChar::Noto_Murder()
{
	// I am a murderer (it seems) (update my murder decay item)
	if ( Noto_IsMurderer())
	{
		SysMessageDefault( DEFMSG_MURDERER );
	}
	if ( m_pPlayer && m_pPlayer->m_wMurders )
	{
		CItem * pFlag = Spell_Effect_Create( SPELL_NONE, LAYER_FLAG_Murders, 0, g_Cfg.m_iMurderDecayTime, NULL );
	}
}

void CChar::Noto_Criminal()
{
	// I am a criminal and the guards will be on my ass.
	if ( IsPriv(PRIV_GM))
		return;
	if ( !IsStatFlag( STATF_Criminal) ) SysMessageDefault( DEFMSG_CRIMINAL );
	CItem * pFlag = Spell_Effect_Create( SPELL_NONE, LAYER_FLAG_Criminal, 0, g_Cfg.m_iCriminalTimer, NULL );
}

void CChar::Noto_ChangeDeltaMsg( int iDelta, LPCTSTR pszType )
{
	if ( iDelta == 0 )
		return;

#define NOTO_FACTOR 300

	static LPCTSTR const sm_DegreeTable[] =
	{
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_1 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_2 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_3 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_4 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_5 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_6 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_7 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_8 ),		// 300 = huge
	};

	int iDegree = abs(iDelta) / ( NOTO_FACTOR / ( COUNTOF(sm_DegreeTable) - 1));
	if (iDegree > COUNTOF(sm_DegreeTable) - 1)
		iDegree = COUNTOF(sm_DegreeTable) - 1;

	SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_0 ),
		( iDelta < 0 ) ? g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_LOST ) : g_Cfg.GetDefaultMsg( DEFMSG_NOTO_CHANGE_GAIN ),
		 sm_DegreeTable[iDegree], pszType );
}

void CChar::Noto_ChangeNewMsg( int iPrvLevel )
{
	if ( iPrvLevel != Noto_GetLevel())
	{
		// reached a new title level ?
		SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_NOTO_GETTITLE ), (LPCTSTR) Noto_GetTitle());
	}
}

void CChar::Noto_Fame( int iFameChange )
{
	// Fame should only go down on death, time or cowardice ?

	if ( ! iFameChange )
		return;

	int iFame = Stat_GetAdjusted(STAT_FAME);
	ASSERT( iFame >= 0 );

	iFameChange = g_Cfg.Calc_FameScale( iFame, iFameChange );
	if ( ! iFameChange )
		return;

	iFame += iFameChange;
	if ( iFame < 0 )
		iFame = 0;
	if ( iFame > 10000)
		iFame = 10000; // Maximum reached

	Noto_ChangeDeltaMsg( iFame - Stat_GetAdjusted(STAT_FAME), "fame" );
	Stat_SetBase(STAT_FAME,iFame);
}

void CChar::Noto_Karma( int iKarmaChange, int iBottom )
{
	// iBottom is a variable where you control at what point
	// the loss for this action stop (as in stealing shouldnt
	// take you to dread ). iBottom def. to -10000 if you leave
	// it out.

	if ( ! iKarmaChange )
		return;

	int	iKarma = Stat_GetAdjusted(STAT_KARMA);

	iKarmaChange = g_Cfg.Calc_KarmaScale( iKarma, iKarmaChange );
	if ( ! iKarmaChange )
		return;

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

	Noto_ChangeDeltaMsg( iKarma - Stat_GetAdjusted(STAT_KARMA), "karma" );
	Stat_SetBase(STAT_KARMA,iKarma);
}

void CChar::Noto_KarmaChangeMessage( int iKarmaChange, int iLimit )
{
	// Change your title ?
	int iPrvLevel = Noto_GetLevel();
	Noto_Karma( iKarmaChange, iLimit );
	Noto_ChangeNewMsg( iPrvLevel );
}

void CChar::Noto_Kill( CChar * pKill, bool fPetKill )
{
	// I participated in killing pKill CChar. (called from Death())
	// I Get some fame/karma. (maybe)

	ASSERT(pKill);

	// What was there noto to me ?
	NOTO_TYPE NotoThem = pKill->Noto_GetFlag( this, false );

	// Fight is over now that i have won. (if i was fighting at all )
	// ie. Magery cast might not be a "fight"
	Fight_Clear( pKill );
	if ( pKill == this )
	{
		DEBUG_CHECK( pKill != this );
		return;
	}

	if ( m_pPlayer == NULL )
	{
		ASSERT( m_pNPC );

		// I am a guard ?
		if ( ! pKill->m_pPlayer &&
			m_pNPC->m_Brain == NPCBRAIN_GUARD )
		{
			// Don't create a corpse or loot if NPC killed by a guard.
			// No corpse and no loot !
			pKill->StatFlag_Set( STATF_Conjured );
			return;
		}

		// Check to see if anything is on the corpse I just killed
		Skill_Start( NPCACT_LOOKING );	// look around for loot.
		// If an NPC kills an NPC then it doesn't count.
		if ( pKill->m_pNPC )
			return;
	}
	else if ( NotoThem < NOTO_NEUTRAL && NotoThem != NOTO_GUILD_SAME )
	{
		ASSERT( m_pPlayer );
		// I'm a murderer !
		if ( ! IsPriv(PRIV_GM))
		{
			CScriptTriggerArgs args;
			args.m_iN1 = m_pPlayer->m_wMurders+1;
			args.m_iN1 = true;
			OnTrigger(CTRIG_MurderMark, this, &args);
			if ( args.m_iN1 < 0 ) args.m_iN1 = 0;
			m_pPlayer->m_wMurders = args.m_iN1;
			if ( args.m_iN2 ) Noto_Criminal();
			Noto_Murder();
		}
	}
   if ( NotoThem != NOTO_GUILD_SAME ) { //no fame/karma gain for killing guildmates
	   // Store current notolevel before changes, used to check if notlvl is changed.
	   int iPrvLevel = Noto_GetLevel();
	   int iFameChange = g_Cfg.Calc_FameKill( pKill );
	   int iKarmaChange = g_Cfg.Calc_KarmaKill( pKill, NotoThem );

	   Noto_Karma( iKarmaChange );
	   if ( ! fPetKill )	// no real fame for letting your pets do the work !
	   {
		   Noto_Fame( iFameChange );
	   }

	   Noto_ChangeNewMsg( iPrvLevel ); // Inform on any notlvl changes.
   }
}

//***************************************************************
// Memory this char has about something in the world.

bool CChar::Memory_UpdateFlags( CItemMemory * pMemory )
{
	// Reset the check timer based on the type of memory.

	ASSERT(pMemory);
	ASSERT(pMemory->IsType(IT_EQ_MEMORY_OBJ));

	WORD wMemTypes = pMemory->GetMemoryTypes();

	if ( ! wMemTypes )	// No memories here anymore so kill it.
	{
		return false;
	}

	int iCheckTime;
	if ( wMemTypes & MEMORY_ISPAWNED )
	{
		StatFlag_Set( STATF_Spawned );
	}
	else if ( wMemTypes & MEMORY_IPET )
	{
		StatFlag_Set( STATF_Pet );
	}

	if ( wMemTypes & MEMORY_FOLLOW )
		iCheckTime = TICK_PER_SEC;
	else if ( wMemTypes & MEMORY_FIGHT )	// update more often to check for retreat.
		iCheckTime = 30*TICK_PER_SEC;
	else if ( wMemTypes & MEMORY_GUMPRECORD )	// update more often to check for retreat.
		iCheckTime = -1; // 60*TICK_PER_SEC;
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

	WORD wPrvMemTypes = pMemory->GetMemoryTypes();
	bool fMore = pMemory->SetMemoryTypes( wPrvMemTypes &~ MemTypes );

	MemTypes &= wPrvMemTypes;	// Which actually got turned off ?

	if ( MemTypes & MEMORY_ISPAWNED )
	{
		StatFlag_Clear( STATF_Spawned );
		// I am a memory link to another object.
		CItem * pSpawn = pMemory->m_uidLink.ItemFind();
		if ( pSpawn != NULL &&
			pSpawn->IsType(IT_SPAWN_CHAR) &&
			pSpawn->m_itSpawnChar.m_current )
		{
			pSpawn->m_itSpawnChar.m_current --;
		}
	}
	if ( MemTypes & MEMORY_IPET )
	{
		// Am i still a pet of some sort ?
		if ( Memory_FindTypes( MEMORY_IPET ) == NULL )
		{
			StatFlag_Clear( STATF_Pet );
		}
	}

#ifdef _DEBUG
	// Must be deleted.
	if ( ! fMore && MemTypes && g_Log.IsLogged( LOGL_TRACE ))
	{
		CObjBase * pObj = pMemory->m_uidLink.ObjFind();
		LPCSTR pName = ( pObj != NULL ) ? pObj->GetName() : "unk";
		DEBUG_MSG(( "Memory delete from '%s' for '%s', type 0%x" DEBUG_CR, (LPCTSTR) GetName(), pName, MemTypes ));
	}
#endif

	return Memory_UpdateFlags( pMemory );
}

void CChar::Memory_AddTypes( CItemMemory * pMemory, WORD MemTypes )
{
	ASSERT(pMemory);
	DEBUG_CHECK(MemTypes);
	pMemory->SetMemoryTypes( pMemory->GetMemoryTypes() | MemTypes );
	pMemory->m_itEqMemory.m_pt = GetTopPoint();	// Where did the fight start ?
	pMemory->m_itEqMemory.m_timeStart = CServTime::GetCurrentTime();
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

CItemMemory * CChar::Memory_CreateObj( CGrayUID uid, WORD MemTypes )
{
	// Create a memory about this object.
	// NOTE: Does not check if object already has a memory.!!!
	//  Assume it does not !

#ifdef _DEBUG
	DEBUG_CHECK( Memory_FindObj( uid ) == NULL );
#endif

	DEBUG_CHECK( MemTypes );
	if (( MemTypes & MEMORY_IPET ) && uid.IsItem())
	{
		MemTypes = MEMORY_ISPAWNED;
	}

	CItemMemory * pMemory = dynamic_cast <CItemMemory *>(CItem::CreateBase( ITEMID_MEMORY ));
	if ( pMemory == NULL )
		return( NULL );

	pMemory->SetType(IT_EQ_MEMORY_OBJ);
	pMemory->SetAttr(ATTR_NEWBIE);
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

CItemMemory * CChar::Memory_FindObj( CGrayUID uid ) const
{
	// Do I have a memory / link for this object ?
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsType(IT_EQ_MEMORY_OBJ))
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

TRIGRET_TYPE CChar::OnCharTrigForMemTypeLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, WORD wMemType )
{
	CScriptLineContext StartContext = s.GetContext();
	CScriptLineContext EndContext = StartContext;

	if ( wMemType )
	{
		CItem* pItem=GetContentHead();
		for ( ; pItem!=NULL; pItem=pItem->GetNext())
		{
			if ( ! pItem->IsMemoryTypes(wMemType))
				continue;
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

CItemMemory * CChar::Memory_AddObjTypes( CGrayUID uid, WORD MemTypes )
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

	CObjBase * pObj = pMemory->m_uidLink.ObjFind();
	if ( pObj == NULL )
		return( false );

	if ( pMemory->IsMemoryTypes( MEMORY_FOLLOW ))
	{
		// I am following. (GM Feature)
		CChar * pChar = pMemory->m_uidLink.CharFind();
		if ( pChar == NULL )
			return( false );

		if ( IsStatFlag( STATF_War ) || IsDisconnected())
		{
			return Memory_ClearTypes( pMemory, MEMORY_FOLLOW );
		}

		CPointMap ptold = GetTopPoint();
		CPointMap ptnew = pChar->GetTopPoint();
		UpdateDir( ptnew );
		MoveToChar( ptnew );
		UpdateMove( ptold, NULL, true );
		return( true );
	}

	if ( pMemory->IsMemoryTypes( MEMORY_FIGHT ))
	{
		// Is the fight still valid ?
		return Memory_Fight_OnTick( pMemory );
	}

	if ( pMemory->IsMemoryTypes( MEMORY_GUMPRECORD ) )
		return( ! IsDisconnected() );

	if ( pMemory->IsMemoryTypes( MEMORY_IPET | MEMORY_GUARD | MEMORY_ISPAWNED | MEMORY_GUILD | MEMORY_TOWN ))
		return( true );	// never go away.

	return( false );	// kill it?.
}

//////////////////////////////////////////////////////////////////////////////

void CChar::OnNoticeCrime( CChar * pCriminal, const CChar * pCharMark )
{
	// I noticed a crime.
	ASSERT(pCriminal);
	if ( pCriminal == this )
		return;

	// NPCBRAIN_BESERK creatures cause criminal fault on the part of their masters.
	if ( pCriminal->m_pNPC && pCriminal->m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		CChar * pOwner = pCriminal->NPC_PetGetOwner();
		if ( pOwner != NULL && pOwner != this )
		{
			OnNoticeCrime( pOwner, pCharMark );
		}
	}

	if ( m_pPlayer )
	{
		// I have the option of attacking the criminal. or calling the guards.
		Memory_AddObjTypes( pCriminal, MEMORY_SAWCRIME );
		return;
	}

	// NPC's can take other actions.

	ASSERT(m_pNPC);
	bool fMyMaster = NPC_IsOwnedBy( pCriminal );

	if ( this != pCharMark )	// it's not me.
	{
		// Thieves and beggars don't care.
		if ( m_pNPC->m_Brain == NPCBRAIN_THIEF || m_pNPC->m_Brain == NPCBRAIN_BEGGAR )
			return;
		if ( fMyMaster )	// I won't rat you out.
			return;
		// Or if the target is evil ?

		// Or if I am evil.
	}
	else
	{
		// I being the victim can retaliate.
		Memory_AddObjTypes( pCriminal, MEMORY_SAWCRIME );
		OnHarmedBy( pCriminal, 1 );
	}

	// Alert the guards !!!?
	if ( ! NPC_CanSpeak())
		return;	// I can't talk anyhow.

	pCriminal->Noto_Criminal();

	if ( GetNPCBrain() != NPCBRAIN_HUMAN )
	{
		// Good monsters don't call for guards outside guarded areas.
		if ( ! m_pArea || ! m_pArea->IsGuarded())
			return;
	}

	if ( GetNPCBrain() != NPCBRAIN_GUARD )
	{
		Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_GENERIC_CRIM ) );
	}

	// Find a guard.
	CallGuards( pCriminal );
}

bool CChar::CheckCrimeSeen( SKILL_TYPE SkillToSee, CChar * pCharMark, const CObjBase * pItem, LPCTSTR pAction )
{
	// I am commiting a crime.
	// Did others see me commit or try to commit the crime.
	//  SkillToSee = NONE = everyone can notice this.
	// RETURN:
	//  true = somebody saw me.

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

		bool fYour = ( pCharMark == pChar );

		CScriptTriggerArgs Args( pAction );
		Args.m_iN1	= SkillToSee;
		Args.m_iN2	= pItem ? (DWORD) pItem->GetUID() : 0;
		Args.m_pO1	= pCharMark;

		int	iRet = pChar->OnTrigger( CTRIG_SeeCrime, this, &Args );

		if ( iRet == TRIGRET_RET_TRUE )
			continue;
		else if ( iRet == TRIGRET_RET_DEFAULT )
		{
			if ( ! g_Cfg.Calc_CrimeSeen( this, pChar, SkillToSee, fYour ))
				continue;
		}

		CGString sMsg;
		if ( pAction != NULL )
		{
			if ( pCharMark == NULL )
			{
				sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_YOUNOTICE_1 ), (LPCTSTR) GetName(), pAction, (LPCTSTR) pItem->GetName());
			}
			else
			{
				sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_YOUNOTICE_2 ),
					(LPCTSTR) GetName(), pAction, fYour ? g_Cfg.GetDefaultMsg( DEFMSG_YOUNOTICE_YOUR ) : (LPCTSTR) pCharMark->GetName(),
					fYour ? "" : g_Cfg.GetDefaultMsg( DEFMSG_YOUNOTICE_S ),
					(LPCTSTR) pItem->GetName());
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
			if ( ! Calc_GetRandVal( g_Cfg.m_iSnoopCriminal ))
			{
				pChar->OnNoticeCrime( this, pCharMark );
			}
			if ( pChar->m_pNPC )
			{
				pChar->NPC_OnNoticeSnoop( this, pCharMark );
			}
		}
		else
		{
			pChar->OnNoticeCrime( this, pCharMark );
		}
	}
	return( fSeen );
}

bool CChar::Skill_Snoop_Check( const CItemContainer * pItem )
{
	// Assume the container is not locked.
	// return: true = snoop or can't open at all.
	//  false = instant open.

	if ( pItem == NULL )
		return( true );

	ASSERT( pItem->IsItem());

	if ( ! IsPriv(PRIV_GM))
	switch ( pItem->GetType())
	{
	case IT_SHIP_HOLD_LOCK:
	case IT_SHIP_HOLD:
		// Must be on board a ship to open the hatch.
		ASSERT(m_pArea);
		if ( m_pArea->GetResourceID() != pItem->m_uidLink )
		{
			SysMessage( g_Cfg.GetDefaultMsg( DEFMSG_ITEMUSE_HATCH_FAIL )); // removed the _TEXT() here
			return( true );
		}
		break;
	case IT_EQ_BANK_BOX:
		// Some sort of cheater.
		return( false );
	}

	CChar * pCharMark;
	if ( ! IsTakeCrime( pItem, &pCharMark ) || pCharMark == NULL )
		return( false );

	DEBUG_CHECK( ! IsPriv(PRIV_GM));
	if ( Skill_Wait(SKILL_SNOOPING))
		return( true );

	m_Act_Targ = pItem->GetUID();
	Skill_Start( SKILL_SNOOPING );
	return( true );
}

int CChar::Skill_Snooping( SKTRIG_TYPE stage )
{
	// SKILL_SNOOPING
	// m_Act_Targ = object to snoop into.
	// RETURN:
	// -SKTRIG_QTY = no chance. and not a crime
	// -SKTRIG_FAIL = no chance and caught.
	// 0-100 = difficulty = percent chance of failure.

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	// Assume the container is not locked.
	CItemContainer * pCont = dynamic_cast <CItemContainer *>(m_Act_Targ.ItemFind());
	if ( pCont == NULL )
	{
		return( -SKTRIG_QTY );
	}

	CChar * pCharMark;
	if ( ! IsTakeCrime( pCont, &pCharMark ) || pCharMark == NULL )
	{
		// Not a crime really.
		return( 0 );
	}

	DEBUG_CHECK( ! IsPriv(PRIV_GM));
	if ( ! CanTouch( pCont ))
	{
		SysMessageDefault( DEFMSG_SNOOPING_REACH );
		return( -SKTRIG_QTY );
	}

	if ( GetTopDist3D( pCharMark ) > 2 )
	{
		SysMessageDefault( DEFMSG_SNOOPING_MARK );
		return( -SKTRIG_QTY );
	}

	PLEVEL_TYPE plevel = GetPrivLevel();
	bool fForceFail = ( plevel < pCharMark->GetPrivLevel());
	if ( stage == SKTRIG_START )
	{
		if ( fForceFail )
			return( -SKTRIG_FAIL );

		if ( plevel >= PLEVEL_Counsel && plevel > pCharMark->GetPrivLevel())	// i'm higher priv.
			return( 0 );

		// return the difficulty.

		return( pCharMark->Stat_GetAdjusted(STAT_DEX));
	}

	if ( fForceFail )
	{
		stage = SKTRIG_FAIL;
	}

	// did anyone see this ?

	if ( CheckCrimeSeen( SKILL_SNOOPING, pCharMark, pCont, (stage == SKTRIG_FAIL)? g_Cfg.GetDefaultMsg( DEFMSG_SNOOPING_YOUR ) : g_Cfg.GetDefaultMsg( DEFMSG_SNOOPING_SOMEONE ) ))
	{
		Noto_KarmaChangeMessage( -10, -500 );
	}

	//
	// View the container.
	//
	if ( stage == SKTRIG_SUCCESS )
	{
		if ( IsClient())
		{
			m_pClient->addContainerSetup( pCont );
		}
	}
	return( 0 );
}

int CChar::Skill_Stealing( SKTRIG_TYPE stage )
{
	// m_Act_Targ = object to steal.
	// RETURN:
	// -SKTRIG_QTY = no chance. and not a crime
	// -SKTRIG_FAIL = no chance and caught.
	// 0-100 = difficulty = percent chance of failure.
	//

	if ( stage == SKTRIG_STROKE )
	{
		return 0;
	}

	CItem * pItem = m_Act_Targ.ItemFind();
	CChar * pCharMark = NULL;
	if ( pItem == NULL )	// on a chars head ? = random steal.
	{
		pCharMark = m_Act_Targ.CharFind();
		if ( pCharMark == NULL )
		{
			SysMessageDefault( DEFMSG_STEALING_NOTHING );
			return( -SKTRIG_QTY );
		}
		CItemContainer * pPack = pCharMark->GetPack();
		if ( pPack == NULL )
		{
cantsteal:
			SysMessageDefault( DEFMSG_STEALING_EMPTY );
			return( -SKTRIG_QTY );
		}
		pItem = pPack->ContentFindRandom();
		if ( pItem == NULL )
		{
			goto cantsteal;
		}
		m_Act_Targ = pItem->GetUID();
	}

	// Special cases.
	CContainer * pContainer = dynamic_cast <CContainer *> (pItem->GetContainer());
	if ( pContainer )
	{
		CItemCorpse * pCorpse = dynamic_cast <CItemCorpse *> (pContainer);
		if ( pCorpse )
		{
			SysMessageDefault( DEFMSG_STEALING_CORPSE );
			return( -SKTRIG_ABORT );
		}
	}
   CItem * pCItem = dynamic_cast <CItem *> (pItem->GetContainer());
   if ( pCItem )
   {
	   if ( pCItem->GetType() == IT_GAME_BOARD )
	   {
		   SysMessageDefault( DEFMSG_STEALING_GAMEBOARD );
		   return( -SKTRIG_ABORT );
	   }
	   if ( pCItem->GetType() == IT_EQ_TRADE_WINDOW )
	   {
		   SysMessageDefault( DEFMSG_STEALING_TRADE );
		   return( -SKTRIG_ABORT );
	   }
   }
	CItemCorpse * pCorpse = dynamic_cast <CItemCorpse *> (pItem);
	if ( pCorpse )
	{
		SysMessageDefault( DEFMSG_STEALING_CORPSE );
		return( -SKTRIG_ABORT );
	}
	if ( pItem->IsType(IT_TRAIN_PICKPOCKET))
	{
		SysMessageDefault( DEFMSG_STEALING_PICKPOCKET );
		return -SKTRIG_QTY;
	}
	if ( pItem->IsType( IT_GAME_PIECE ))
	{
		return -SKTRIG_QTY;
	}
	if ( ! CanTouch( pItem ))
	{
		SysMessageDefault( DEFMSG_STEALING_REACH );
		return( -SKTRIG_ABORT );
	}
	if ( ! CanMove( pItem ) ||
		! CanCarry( pItem ))
	{
		SysMessageDefault( DEFMSG_STEALING_HEAVY );
		return( -SKTRIG_ABORT );
	}
	if ( ! IsTakeCrime( pItem, & pCharMark ))
	{
		SysMessageDefault( DEFMSG_STEALING_NONEED );

		// Just pick it up ?

		return( -SKTRIG_QTY );
	}
	if ( m_pArea->IsFlag(REGION_FLAG_SAFE))
	{
		SysMessageDefault( DEFMSG_STEALING_STOP );
		return( -SKTRIG_QTY );
	}

	Reveal();	// If we take an item off the ground we are revealed.

	bool fGround = false;
	if ( pCharMark != NULL )
	{
		if ( GetTopDist3D( pCharMark ) > 2 )
		{
			SysMessageDefault( DEFMSG_STEALING_MARK );
			return -SKTRIG_QTY;
		}
		if ( m_pArea->IsFlag(REGION_FLAG_NO_PVP) && pCharMark->m_pPlayer && ! IsPriv(PRIV_GM))
		{
			SysMessageDefault( DEFMSG_STEALING_SAFE );
			return( -1 );
		}
		if ( GetPrivLevel() < pCharMark->GetPrivLevel())
		{
			return -SKTRIG_FAIL;
		}
		if ( stage == SKTRIG_START )
		{
			return g_Cfg.Calc_StealingItem( this, pItem, pCharMark );
		}
	}
	else
	{
		// stealing off the ground should always succeed.
		// it's just a matter of getting caught.
		if ( stage == SKTRIG_START )
		{
			return 1;	// town stuff on the ground is too easy.
		}
		fGround = true;
	}

	// Deliver the goods.

	if ( stage == SKTRIG_SUCCESS || fGround )
	{
		pItem->ClrAttr(ATTR_OWNED);	// Now it's mine
		CItemContainer * pPack = GetPack();
		if ( pItem->GetParent() != pPack && pPack )
		{
			pItem->RemoveFromView();
			// Put in my invent.
			pPack->ContentAdd( pItem );
		}
	}

	if ( m_Act_Difficulty == 0 )
		return( 0 );	// Too easy to be bad. hehe

	// You should only be able to go down to -1000 karma by stealing.
	if ( CheckCrimeSeen( SKILL_STEALING, pCharMark, pItem, (stage == SKTRIG_FAIL)? g_Cfg.GetDefaultMsg( DEFMSG_STEALING_YOUR ) : g_Cfg.GetDefaultMsg( DEFMSG_STEALING_SOMEONE ) ))
	{
		Noto_KarmaChangeMessage( -100, -1000 );
	}
	return( 0 );
}



void CChar::CallGuards( CChar * pCriminal )
{
	// I just yelled for guards.
	if ( ! m_pArea )
		return;
	bool	bSearchedGuard = false;
	CChar	*pGuard = NULL;
	CGrayUID	PrevGuardTarg;

	// Is there anything for guards to see ?
	if ( pCriminal == NULL )
	{
		bSearchedGuard = true;
		CWorldSearch AreaCrime( GetTopPoint(), UO_MAP_VIEW_SIGHT );
		while (true)
		{
			CChar * pChar = AreaCrime.GetChar();
			if ( pChar == NULL ) break;

			//	scan for guards also ;) it will speed up the execution a bit
			if ( !pGuard && !pChar->m_pPlayer )
			{
				ASSERT(pChar->m_pNPC);
				if ( pChar->m_pNPC->m_Brain == NPCBRAIN_GUARD )
				{
					if ( pChar->IsStatFlag( STATF_War ))
					{									// he is attacking to the same char. so, free him
						if ( PrevGuardTarg.IsValidUID() && PrevGuardTarg == pChar->m_Act_Targ ) pGuard = pChar;
					}
					else pGuard = pChar;
				}
			}

			//	mark person a criminal if seen him criming
			// Only players call guards this way. NPC's flag criminal instantly.
			if ( m_pPlayer && Memory_FindObjTypes( pChar, MEMORY_SAWCRIME ) ) pChar->Noto_Criminal();
			if ( !pChar->Noto_IsCriminal()) continue;

			// can't respond to guardcalls if the criminal is outside a guarded zone.
			if ( !pChar->m_pArea->IsGuarded()) continue;

			pCriminal = pChar;
		}
		if ( pCriminal == NULL ) return;
	}

	// Am i in guarded zone ?
	// Guards can't respond if you are outside.
	ASSERT( pCriminal->GetTopPoint().IsValidPoint() );
	if ( ! pCriminal->m_pArea->IsGuarded())
		return;

	if ( OnTrigger( CTRIG_CallGuards, pCriminal ) == TRIGRET_RET_TRUE ) return;

	if ( !bSearchedGuard )
	{
		PrevGuardTarg = UID_UNUSED;
		// Is there a free guard near by ?
		CWorldSearch AreaGuard( GetTopPoint(), UO_MAP_VIEW_RADAR );
		while (true)
		{
			pGuard = AreaGuard.GetChar();
			if ( !pGuard || pGuard->m_pPlayer )  break;

			ASSERT(pGuard->m_pNPC);
			if ( pGuard->m_pNPC->m_Brain != NPCBRAIN_GUARD ) continue;
			if ( pGuard->IsStatFlag( STATF_War ))
			{
				if ( PrevGuardTarg.IsValidUID() && PrevGuardTarg == pGuard->m_Act_Targ )	// take him off this case.
					break;
				PrevGuardTarg = pGuard->m_Act_Targ;
				continue;	// busy.
			}
			break;	// we have a free guard.
		}
	}

	if ( pGuard == NULL )
	{
		// Spawn a new guard.
		RESOURCE_ID rid;
		CVarDefBase * pVar = m_pArea->m_TagDefs.GetKey("GUARDID");
		if ( pVar ) rid = RESOURCE_ID( RES_CHARDEF, pVar->GetValNum());
		else rid = g_Cfg.ResourceGetIDType( RES_CHARDEF, "GUARDS" );
		if ( ! rid.IsValidUID()) return;

		pGuard = CChar::CreateNPC( (CREID_TYPE) rid.GetResIndex() );
		if ( pGuard == NULL ) return;

		// Spawned guards should go away after x minutes.
		CItem * pSpell = pGuard->Spell_Effect_Create( SPELL_Summon, LAYER_SPELL_Summon, 1000, g_Cfg.m_iGuardLingerTime );
		ASSERT(pSpell);
		// pSpell->ClrAttr(ATTR_MAGIC);	// cannot be dispelled.
		pGuard->Spell_Teleport( pCriminal->GetTopPoint(), false, false );
	}
	pGuard->NPC_LookAtCharGuard(pCriminal);
}



void CChar::OnHarmedBy( CChar * pCharSrc, int iHarmQty )
{
	// i notice a Crime or attack against me ..
	// Actual harm has taken place.
	// Attack back.


	bool fFightActive = Fight_IsActive();
	Memory_AddObjTypes( pCharSrc, MEMORY_HARMEDBY );

	if ( fFightActive && m_Act_Targ.CharFind())
	{
		// In war mode already
		if ( m_pPlayer )
			return;
		if ( Calc_GetRandVal( 10 ))
			return;
		// NPC will Change targets.
	}

	bool fMyMaster = NPC_IsOwnedBy( pCharSrc );
	if ( fMyMaster )
	{
		// I Quit !
		NPC_PetDesert();
	}

	// I will Auto-Defend myself.
	Fight_Attack( pCharSrc );
	if ( ! fFightActive )	// auto defend puts us in war mode.
	{
		UpdateMode();
	}
}

bool CChar::OnAttackedBy( CChar * pCharSrc, int iHarmQty, bool fCommandPet )
{
	// We have been attacked in some way by this CChar.
	// Might not actually be doing any real damage. (yet)
	//
	// They may have just commanded their pet to attack me.
	// Cast a bad spell at me.
	// Fired projectile at me.
	// Attempted to provoke me ?
	//
	// RETURN: true = ok.
	//  false = we are immune to this char ! (or they to us)

	if ( pCharSrc == NULL )
		return true;	// field spell ?
	if ( pCharSrc == this )
		return true;	// self induced

	pCharSrc->Reveal();	// fix invis exploit

	if ( pCharSrc->IsStatFlag( STATF_INVUL ) && ! pCharSrc->IsPriv( PRIV_GM ))
	{
		// Can't do any damage either.
		pCharSrc->SysMessageDefault( DEFMSG_MAGIC_BLOCK );
		return( false );
	}

	// Am i already attacking the source anyhow
	if ( Fight_IsActive() && m_Act_Targ == pCharSrc->GetUID())
		return true;

	Memory_AddObjTypes( pCharSrc, MEMORY_HARMEDBY|MEMORY_IRRITATEDBY );

	// Are they a criminal for it ? Is attacking me a crime ?
	if ( Noto_GetFlag(pCharSrc) == NOTO_GOOD )
	{
		if ( IsClient())
		{
			// I decide if this is a crime.
			OnNoticeCrime( pCharSrc, this );
		}
		else
		{
			// If it is a pet then this a crime others can report.
			pCharSrc->CheckCrimeSeen( SKILL_NONE, this, NULL, NULL );
		}
	}

	if ( ! fCommandPet )
	{
		// possibly retaliate. (auto defend)
		OnHarmedBy( pCharSrc, iHarmQty );
	}

	return( true );
}

static const LAYER_TYPE sm_ArmorLayerHead[] = { LAYER_HELM, LAYER_NONE };		// ARMOR_HEAD,
static const LAYER_TYPE sm_ArmorLayerNeck[] = { LAYER_COLLAR, LAYER_NONE };		// ARMOR_NECK,
static const LAYER_TYPE sm_ArmorLayerBack[] = { LAYER_SHIRT, LAYER_CHEST, LAYER_TUNIC, LAYER_CAPE, LAYER_ROBE, LAYER_NONE };	// ARMOR_BACK,
static const LAYER_TYPE sm_ArmorLayerChest[] = { LAYER_SHIRT, LAYER_CHEST, LAYER_TUNIC, LAYER_ROBE, LAYER_NONE };		// ARMOR_CHEST
static const LAYER_TYPE sm_ArmorLayerArms[] = { LAYER_ARMS, LAYER_CAPE, LAYER_ROBE, LAYER_NONE };		// ARMOR_ARMS,
static const LAYER_TYPE sm_ArmorLayerHands[] = { LAYER_GLOVES,	LAYER_NONE };	// ARMOR_HANDS
static const LAYER_TYPE sm_ArmorLayerLegs[] = { LAYER_PANTS, LAYER_SKIRT, LAYER_HALF_APRON, LAYER_ROBE, LAYER_LEGS, LAYER_NONE };	// ARMOR_LEGS,
static const LAYER_TYPE sm_ArmorLayerFeet[] = { LAYER_SHOES, LAYER_LEGS, LAYER_NONE };	// ARMOR_FEET,

struct CArmorLayerType
{
	WORD m_wCoverage;	// Percentage of humanoid body area
	const LAYER_TYPE * m_pLayers;
};

static const CArmorLayerType sm_ArmorLayers[ARMOR_QTY] =	// layers covering the armor zone.
{
	{ 10,	sm_ArmorLayerHead },		// ARMOR_HEAD,
	{ 5,	sm_ArmorLayerNeck },		// ARMOR_NECK,
	{ 10,	sm_ArmorLayerBack },		// ARMOR_BACK,
	{ 30,	sm_ArmorLayerChest },	// ARMOR_CHEST
	{ 10,	sm_ArmorLayerArms },		// ARMOR_ARMS,
	{ 10,	sm_ArmorLayerHands },	// ARMOR_HANDS
	{ 20,	sm_ArmorLayerLegs },		// ARMOR_LEGS,
	{ 5,	sm_ArmorLayerFeet },		// ARMOR_FEET,
};

int CChar::CalcArmorDefense( void ) const
{
	// When armor is added or subtracted check this.
	// This is the general AC number printed.
	// Tho not really used to compute damage.

	int iDefenseTotal = 0;
	int iArmorCount = 0;
	WORD ArmorRegionMax[ARMOR_QTY];
	memset( ArmorRegionMax, 0, sizeof(ArmorRegionMax));
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		int iDefense = pItem->Armor_GetDefense();

		// IsTypeSpellable() ? ! IT_WAND
		if (( pItem->IsType(IT_SPELL) || pItem->IsTypeArmor()) &&
			pItem->m_itSpell.m_spell )
		{
			SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX(pItem->m_itSpell.m_spell);
			switch (spell)
			{
			case SPELL_Steelskin:		// turns your skin into steel, giving a boost to your AR.
			case SPELL_Stoneskin:		// turns your skin into stone, giving a boost to your AR.
			case SPELL_Protection:
			case SPELL_Arch_Prot:
				// Effect of protection spells.
				iDefenseTotal += g_Cfg.GetSpellEffect(spell, pItem->m_itSpell.m_spelllevel ) * 100;
				break;
			}
		}

		// reverse of sm_ArmorLayers
		switch ( pItem->GetEquipLayer())
		{
		case LAYER_HELM:		// 6
			ArmorRegionMax[ ARMOR_HEAD ] = max( ArmorRegionMax[ ARMOR_HEAD ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_COLLAR:	// 10 = gorget or necklace.
			ArmorRegionMax[ ARMOR_NECK ] = max( ArmorRegionMax[ ARMOR_NECK ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_SHIRT:
		case LAYER_CHEST:	// 13 = armor chest
		case LAYER_TUNIC:	// 17 = jester suit
			ArmorRegionMax[ ARMOR_CHEST ] = max( ArmorRegionMax[ ARMOR_CHEST ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_ARMS:		// 19 = armor
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_PANTS:
		case LAYER_SKIRT:
		case LAYER_HALF_APRON:
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_SHOES:
			ArmorRegionMax[ ARMOR_FEET ] = max( ArmorRegionMax[ ARMOR_FEET ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_GLOVES:	// 7
			ArmorRegionMax[ ARMOR_HANDS ] = max( ArmorRegionMax[ ARMOR_HANDS ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_CAPE:		// 20 = cape
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_ROBE:		// 22 = robe over all.
			ArmorRegionMax[ ARMOR_CHEST ] = max( ArmorRegionMax[ ARMOR_CHEST ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_BACK ] = max( ArmorRegionMax[ ARMOR_BACK ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_ARMS ] = max( ArmorRegionMax[ ARMOR_ARMS ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense ) + pItem->m_ModAr;
			break;
		case LAYER_LEGS:
			ArmorRegionMax[ ARMOR_LEGS ] = max( ArmorRegionMax[ ARMOR_LEGS ], iDefense ) + pItem->m_ModAr;
			ArmorRegionMax[ ARMOR_FEET ] = max( ArmorRegionMax[ ARMOR_FEET ], iDefense ) + pItem->m_ModAr;
			break;

		case LAYER_HAND2:
			// Shield effect.
			if ( pItem->IsType( IT_SHIELD ))
			{
				iDefenseTotal += iDefense * ( Skill_GetAdjusted(SKILL_PARRYING) / 10 );
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
			iDefenseTotal += sm_ArmorLayers[i].m_wCoverage * ArmorRegionMax[i];
		}
	}

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	signed int iQty = ( iDefenseTotal / 100 ) + m_ModAr;
	if ( iQty < 0 ) iQty = 0;
	return( iQty );
}

int CChar::OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType )
{
	// Someone hit us.
	// uType =
	//	DAMAGE_GOD		 0x01	// Nothing can block this.
	//	DAMAGE_HIT_BLUNT 0x02	// Physical hit of some sort.
	//	DAMAGE_MAGIC	 0x04	// Magic blast of some sort. (we can be immune to magic to some extent)
	//	DAMAGE_POISON	 0x08	// Or biological of some sort ? (HARM spell)
	//	DAMAGE_FIRE		 0x10	// Fire damage of course.  (Some creatures are immune to fire)
	//	DAMAGE_ELECTRIC  0x20	// lightning.
	//  DAMAGE_DRAIN
	//	DAMAGE_GENERAL	 0x80	// All over damage. As apposed to hitting just one point.
	//
	// RETURN: damage done.
	//  -1 = already dead = invalid target. 0 = no damage. INT_MAX = killed.

	if ( iDmg <= 0 )
		return( 0 );
	if ( pSrc == NULL )
		pSrc = this;

	CScriptTriggerArgs Args( iDmg, uType, NULL );
	if ( OnTrigger( CTRIG_GetHit, pSrc, &Args ) == TRIGRET_RET_TRUE )
		return( 0 );
	iDmg	= Args.m_iN1;
	uType	= Args.m_iN2;

	if ( uType & ( DAMAGE_ELECTRIC | DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH | DAMAGE_FIRE | DAMAGE_MAGIC ))
	{
		StatFlag_Clear( STATF_Freeze );	// remove paralyze.
	}

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( uType & ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH ))
	{
		// A physical blow of some sort.
		// Try to allow the armor or shield to take some damage.
		Reveal();

		// Check for reactive armor.
		if ( IsStatFlag( STATF_Reactive ) && ! ( uType & DAMAGE_GOD ))
		{
			// reflect some damage back.
			if ( pSrc && GetTopDist3D( pSrc ) <= 2 )
			{
				int iSkillVal;
				// Spell strength is NOT the same as MAGERY !!!???
				iSkillVal = Skill_GetAdjusted(SKILL_MAGERY);
				int iEffect = g_Cfg.GetSpellEffect( SPELL_Reactive_Armor, iSkillVal );
				int iRefDam = Calc_GetRandVal( IMULDIV( iDmg, iEffect, 1000 ));
				iDmg -= iRefDam;
				pSrc->OnTakeDamage( iRefDam, this, uType );
				pSrc->Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 9, 6 );
			}
		}

		// absorbed by armor ?
		if ( ! ( uType & DAMAGE_GENERAL ))
		{
			iDmg	= OnTakeDamageHitPoint( iDmg, pSrc, uType );

			if ( !g_Cfg.IsSetOF( OF_Magic_IgnoreAR ) )
			{
				int		iDef;
				iDef	= Calc_GetRandVal( pCharDef->m_defense );
				if ( uType & DAMAGE_MAGIC )
					iDef	/= 2;
				iDmg -= iDef;
			}
		}
		else if ( ! ( uType & DAMAGE_GOD ))
		{
			if ( !g_Cfg.IsSetOF( OF_Magic_IgnoreAR ) )
			{
				// general overall damage.
				iDmg -= Calc_GetRandVal( m_defense + pCharDef->m_defense );
				// ??? take some random damage to my equipped items.
			}
		}
	}


	if ( IsStatFlag( STATF_INVUL ))
	{
effect_bounce:
		if ( iDmg )
		{
			Effect( EFFECT_OBJ, ITEMID_FX_GLOW, this, 9, 30, false );
		}
		iDmg = 0;
	}
	else if ( ! ( uType & DAMAGE_GOD ))
	{
		if ( m_pArea )
		{
			if ( m_pArea->IsFlag(REGION_FLAG_SAFE))
				goto effect_bounce;
			if ( m_pArea->IsFlag(REGION_FLAG_NO_PVP) && m_pPlayer && pSrc && pSrc->m_pPlayer )
				goto effect_bounce;
		}
		if ( IsStatFlag(STATF_Stone))	// can't hurt us anyhow.
		{
			goto effect_bounce;
		}
	}

	if ( Stat_GetVal(STAT_STR) <= 0 )	// Already dead.
		return( -1 );

	if ( uType & DAMAGE_FIRE )
	{
		if ( pCharDef->Can(CAN_C_FIRE_IMMUNE)) // immune to the fire part.
		{
			// If there is any other sort of damage then dish it out as well.
			if ( ! ( uType & ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH | DAMAGE_POISON | DAMAGE_ELECTRIC )))
				return( 0 );	// No effect.
			iDmg /= 2;
		}
	}

	// defend myself. (even though it may not have hurt me.)
	if ( ! OnAttackedBy( pSrc, iDmg, false ))
		return( 0 );

	// Did it hurt ?
	if ( iDmg <= 0 )
		return( 0 );

	// Make blood depending on hit damage. assuming the creature has blood
	ITEMID_TYPE id = ITEMID_NOTHING;
	if ( pCharDef->m_wBloodHue != (HUE_TYPE)-1 )
	{
		if ( iDmg > 10 )
		{
			id = (ITEMID_TYPE)( ITEMID_BLOOD1 + Calc_GetRandVal(ITEMID_BLOOD6-ITEMID_BLOOD1));
		}
		else if ( Calc_GetRandVal( iDmg ) > 5 )
		{
			id = ITEMID_BLOOD_SPLAT;	// minor hit.
		}
		if ( id &&
			! IsStatFlag( STATF_Conjured ) &&
			( uType & ( DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH )))	// A blow of some sort.
		{
			CItem * pBlood = CItem::CreateBase( id );
			ASSERT(pBlood);
			pBlood->SetHue( pCharDef->m_wBloodHue );
			pBlood->MoveToDecay( GetTopPoint(), 7*TICK_PER_SEC );
		}
	}

	UpdateStatVal( STAT_STR, -iDmg );
	// always send updates to src
	if ( pSrc->IsClient() )
		pSrc->GetClient()->addHitsUpdate( GetUID() );

	if ( g_Cfg.IsSetEF( EF_AgeOfShadows ) )
	{
		// The AOS/LBR damage counter
		// I'm not sure whether to show this to everyone or just the person hitting/being hit
		// How does OSI do this?
		CExtData cmdData;
		cmdData.Damage.m_dmg = iDmg;
		cmdData.Damage.m_unk1 = 0;
		cmdData.Damage.m_UID = GetUID();

		if (IsClient())
			m_pClient->addExtData(EXTDATA_Damage, &cmdData, sizeof( cmdData.Damage ) );
		if (pSrc->IsClient())
			pSrc->m_pClient->addExtData(EXTDATA_Damage, &cmdData, sizeof( cmdData.Damage ) );
	}

	if ( Stat_GetVal(STAT_STR) <= 0 )
	{
		// We will die from this...make sure the killer is set correctly...if we don't do this, the person we are currently
		// attacking will get credit for killing us.
		// Killed by a guard looks here !
		if ( pSrc )
		{
			m_Act_Targ = pSrc->GetUID();
		}
		return( -1 );	// INT_MAX ?
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
	// ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH )
	// Point strike type damage.
	// Deflect some damage with shield or weapon ?

	ASSERT( ! ( uType & DAMAGE_GENERAL ));

	if ( IsStatFlag(STATF_HasShield) &&
		(uType & ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH )) &&
		! (uType & (DAMAGE_GOD|DAMAGE_ELECTRIC)))
	{
		CItem * pShield = LayerFind( LAYER_HAND2 );
		if ( pShield != NULL && Skill_UseQuick( SKILL_PARRYING, Calc_GetRandVal((pSrc!=NULL) ? (pSrc->Skill_GetBase(SKILL_TACTICS)/10) : 100 )))
		{
			// Damage the shield.
			// Let through some damage.
			int iDefense = Calc_GetRandVal( pShield->Armor_GetDefense() / 2 );
			if ( pShield->OnTakeDamage( min( iDmg, iDefense ), pSrc, uType ))
			{
				SysMessageDefault( DEFMSG_COMBAT_PARRY );
			}
			iDmg -= iDefense; // damage absorbed by shield
		}
	}

	// Assumes humanoid type body. Orcs, Headless, trolls, humans etc.
	// ??? If not humanoid ??
	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( pCharDef->Can( CAN_C_NONHUMANOID ))
	{
		// ??? we may want some sort of message ?
		return( iDmg );
	}

	// Where was the hit ?
	int iHitRoll = Calc_GetRandVal( 100 ); // determine area of body hit
	BODYPART_TYPE iHitArea=ARMOR_HEAD;
	while ( iHitArea<ARMOR_QTY-1 )
	{
		iHitRoll -= sm_ArmorLayers[iHitArea].m_wCoverage;
		if ( iHitRoll < 0 )
			break;
		iHitArea = (BODYPART_TYPE)( iHitArea + 1 );
	}

	if ( (uType & ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH |DAMAGE_FIRE|DAMAGE_ELECTRIC)) &&
		! (uType & (DAMAGE_GENERAL|DAMAGE_GOD)))
	{

		static LPCTSTR const sm_Hit_Head1[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD1O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD2O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD3S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD3O ),
		};
		static LPCTSTR const sm_Hit_Head2[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD4S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD4O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD5S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD5O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD6S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HEAD6O ),
		};
		static LPCTSTR const sm_Hit_Chest1[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST1O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST2O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST3S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST3O ),
		};
		static LPCTSTR const sm_Hit_Chest2[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST4S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST4O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST5S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST5O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST6S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_CHEST6O ),
		};
		static LPCTSTR const sm_Hit_Arm[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM1O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM2O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM3S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_ARM3O ),
		};
		static LPCTSTR const sm_Hit_Legs[][2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS1O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS2O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS3S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_LEGS3O ),
		};
		static LPCTSTR const sm_Hit_Hands[][2] =	// later include exclusion of left hand if have shield
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND1O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND2O ),
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND3S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_HAND3O ),
		};
		static LPCTSTR const sm_Hit_Neck1[2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_NECK1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_NECK1O ),
		};
		static LPCTSTR const sm_Hit_Neck2[2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_NECK2S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_NECK2O ),
		};
		static LPCTSTR const sm_Hit_Back[2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_BACK1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_BACK1O ),
		};
		static LPCTSTR const sm_Hit_Feet[2] =
		{
			g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_FEET1S ), g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_HIT_FEET1O ),
		};

		int iMsg = Calc_GetRandVal(3);
		LPCTSTR const * ppMsg;
		switch ( iHitArea )
		{
		case ARMOR_HEAD:
			ppMsg = (iDmg>10) ? sm_Hit_Head2[iMsg] : sm_Hit_Head1[iMsg];
			break;
		case ARMOR_NECK:
			ppMsg = (iDmg>10) ? sm_Hit_Neck2 : sm_Hit_Neck1;
			break;
		case ARMOR_BACK:
			ppMsg = sm_Hit_Back;
			break;
		case ARMOR_CHEST:
			ppMsg = (iDmg>10) ? sm_Hit_Chest2[iMsg] : sm_Hit_Chest1[iMsg];
			break;
		case ARMOR_ARMS:
			ppMsg = sm_Hit_Arm[iMsg];
			break;
		case ARMOR_HANDS:
			ppMsg = sm_Hit_Hands[iMsg];
			break;
		case ARMOR_LEGS:
			ppMsg = sm_Hit_Legs[iMsg];
			break;
		case ARMOR_FEET:
			ppMsg = sm_Hit_Feet;
			break;
		default:
			ASSERT(0);
			break;
		}

		if ( pSrc != this )
		{
			if ( IsPriv(PRIV_DETAIL))
			{
				SysMessagef( ppMsg[0], ( pSrc == NULL ) ? "It" : (LPCTSTR) pSrc->GetName());
			}
			if ( pSrc != NULL && pSrc->IsPriv(PRIV_DETAIL))
			{
				pSrc->SysMessagef( ppMsg[1], (LPCTSTR) GetName());
			}
		}
	}

	// Do damage to my armor. (what if it is empty ?)

	int iMaxCoverage = 0;	// coverage at the hit zone.

	CItem * pArmorNext;
	for ( CItem * pArmor = GetContentHead(); pArmor != NULL; pArmor = pArmorNext )
	{
		pArmorNext = pArmor->GetNext();

		if ( pArmor->IsType( IT_SPELL ) && pArmor->m_itSpell.m_spell )
		{
			SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX(pArmor->m_itSpell.m_spell);
			switch ( spell )
			{
			case SPELL_Steelskin:		// turns your skin into steel, giving a boost to your AR.
			case SPELL_Stoneskin:		// turns your skin into stone, giving a boost to your AR.
			case SPELL_Arch_Prot:
			case SPELL_Protection:
				// Effect of protection spells are general.
				iMaxCoverage = max( iMaxCoverage, g_Cfg.GetSpellEffect( spell, pArmor->m_itSpell.m_spelllevel ));
				continue;
			}
		}

		LAYER_TYPE layer = pArmor->GetEquipLayer();
		if ( ! CItemBase::IsVisibleLayer( layer ))
			continue;

		const CArmorLayerType * pArmorLayer = &sm_ArmorLayers[iHitArea];

		for ( int i=0; pArmorLayer->m_pLayers[i] != LAYER_NONE; i++ ) // layers covering the armor zone.
		{
			if ( pArmorLayer->m_pLayers[i] == layer )
			{
				// This piece of armor takes damage.
				iMaxCoverage = max( iMaxCoverage, pArmor->Armor_GetDefense());
				pArmor->OnTakeDamage( iDmg, pSrc, uType );
				break;
			}
		}
	}

	// iDmg = ( iDmg * GW_GetSCurve( iMaxCoverage - iDmg, 10 )) / 100;
	iDmg -= Calc_GetRandVal( iMaxCoverage );
	return( iDmg );
}

//*******************************************************************************
// Fight specific memories.

void CChar::Memory_Fight_Retreat( CChar * pTarg, CItemMemory * pFight )
{
	// The fight is over because somebody ran away.
	if ( pTarg == NULL || pTarg->IsStatFlag( STATF_DEAD ))
		return;

	ASSERT(pFight);
	int iMyDistFromBattle = GetTopPoint().GetDist( pFight->m_itEqMemory.m_pt );
	int iHisDistFromBattle = pTarg->GetTopPoint().GetDist( pFight->m_itEqMemory.m_pt );

	bool fCowardice = ( iMyDistFromBattle > iHisDistFromBattle );

	if ( fCowardice && ! pFight->IsMemoryTypes( MEMORY_IAGGRESSOR ))
	{
		// cowardice is ok if i was attacked.
		return;
	}

	SysMessagef( fCowardice ?
		g_Cfg.GetDefaultMsg( DEFMSG_COWARD_1 ) :
		g_Cfg.GetDefaultMsg( DEFMSG_COWARD_2 ), (LPCTSTR) pTarg->GetName());

	// Lose some fame.
	if ( fCowardice )
	{
		Noto_Fame( -1 );
	}
}

bool CChar::Memory_Fight_OnTick( CItemMemory * pMemory )
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
		DEBUG_MSG(( "OnTick '%s' Memory of Fighting '%s'" DEBUG_CR, (LPCTSTR) GetName(), (LPCTSTR) pTarg->GetName()));
	}
#endif

	if ( GetDist(pTarg) > UO_MAP_VIEW_RADAR )
	{
		Memory_Fight_Retreat( pTarg, pMemory );
clearit:
		Memory_ClearTypes( pMemory, MEMORY_FIGHT|MEMORY_IAGGRESSOR );
		return( true );
	}

	int iTimeDiff = - g_World.GetTimeDiff( pMemory->m_itEqMemory.m_timeStart );
	//DEBUG_CHECK( iTime >= 0 );

	// If am fully healthy then it's not much of a fight.
	if ( iTimeDiff > 60*60*TICK_PER_SEC )
		goto clearit;
	if ( pTarg->GetHealthPercent() >= 100 && iTimeDiff > 2*60*TICK_PER_SEC )
		goto clearit;

	pMemory->SetTimeout(20*TICK_PER_SEC);
	return( true );	// reschedule it.
}

void CChar::Memory_Fight_Start( const CChar * pTarg )
{
	// I am attacking this creature.
	// i might be the aggressor or just retaliating.
	// This is just the "Intent" to fight. Maybe No damage done yet.

	ASSERT(pTarg);
	if ( Fight_IsActive() && m_Act_Targ == pTarg->GetUID())
	{
		// quick check that things are ok.
		return;
	}
	DEBUG_CHECK( pTarg != this );

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
				MemTypes = MEMORY_HARMEDBY;
			else if ( pTargMemory->IsMemoryTypes( MEMORY_HARMEDBY|MEMORY_SAWCRIME|MEMORY_AGGREIVED ))
				MemTypes = MEMORY_IAGGRESSOR;
			else
				MemTypes = 0;
		}
		else
		{
			// Hmm, I must have started this i guess.
			MemTypes = MEMORY_IAGGRESSOR;
		}
		pMemory = Memory_CreateObj( pTarg, MEMORY_FIGHT|MEMORY_WAR_TARG|MemTypes );
	}
	else
	{
		// I have a memory of them.
		bool fMemPrvType = pMemory->IsMemoryTypes(MEMORY_WAR_TARG);
		if ( fMemPrvType )
			return;
		if ( pMemory->IsMemoryTypes(MEMORY_HARMEDBY|MEMORY_SAWCRIME|MEMORY_AGGREIVED))
		{
			// I am defending myself rightly.
			MemTypes = 0;
		}
		else
		{
			MemTypes = MEMORY_IAGGRESSOR;
		}
		// Update the fights status
		Memory_AddTypes( pMemory, MEMORY_FIGHT|MEMORY_WAR_TARG|MemTypes );
	}

#ifdef _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "Fight_Start '%s' attacks '%s', type 0%x" DEBUG_CR, (LPCTSTR) GetName(), (LPCTSTR) pTarg->GetName(), MemTypes ));
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
		GetClient()->xSendPkt( &cmd, sizeof( cmd.Fight ));
	}
	else
	{
		if ( m_pNPC && m_pNPC->m_Brain == NPCBRAIN_BESERK ) // it will attack everything.
			return;
	}

	if ( GetTopSector()->GetCharComplexity() < 7 )
	{
		// too busy for this.
		CGString sMsgThem;
		sMsgThem.Format( g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_ATTACKO ), (LPCTSTR) GetName(), (LPCTSTR) pTarg->GetName());

		// Don't bother telling me who i just attacked.
		UpdateObjMessage( sMsgThem, NULL, pTarg->GetClient(), HUE_RED, TALKMODE_EMOTE );
	}

	if ( pTarg->IsClient() && pTarg->CanSee(this))
	{
		CGString sMsgYou;
		sMsgYou.Format( g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_ATTACKS ), (LPCTSTR) GetName());
		pTarg->GetClient()->addObjMessage( sMsgYou, this, HUE_RED );
	}
}

//********************************************************

SKILL_TYPE CChar::Fight_GetWeaponSkill() const
{
	// What sort of weapon am i using?
	CItem * pWeapon = m_uidWeapon.ItemFind();
	if ( pWeapon == NULL )
		return( SKILL_WRESTLING );
	return( pWeapon->Weapon_GetSkill());
}

bool CChar::Fight_IsActive() const
{
	// Am i in an active fight mode ?
	if ( ! IsStatFlag(STATF_War))
		return( false );

	SKILL_TYPE iSkillActive		= Skill_GetActive();
	switch ( iSkillActive )
	{
	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
	case SKILL_WRESTLING:
		return( true );
	}

	if ( iSkillActive == Fight_GetWeaponSkill() )
		return true;

	return g_Cfg.IsSkillFlag( iSkillActive, SKF_FIGHT );
}

int CChar::Fight_CalcDamage( CItem * pWeapon, SKILL_TYPE skill ) const
{
	// STR bonus on close combat weapons.
	// and DEX bonus on archery weapons
	// A random value of the bonus is added (0-10%)

	if ( m_pNPC &&
		m_pNPC->m_Brain == NPCBRAIN_GUARD &&
		g_Cfg.m_fGuardsInstantKill )
	{
		return( 20000 );	// swing made.
	}

	int iDmg = 1;
	int iDmgAdj;
	if ( skill != SKILL_ARCHERY )
		iDmgAdj = Calc_GetRandVal( Stat_GetAdjusted(STAT_STR));
	else
		iDmgAdj = Calc_GetRandVal( Stat_GetAdjusted(STAT_DEX));
	iDmg += iDmgAdj / 10;

	CCharBase * pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( pWeapon != NULL )
	{
		iDmg += pWeapon->Weapon_GetAttack();
	}
	else
	{
		// Pure wrestling damage.
		// Base type attack for our body. claws/etc
		iDmg += pCharDef->m_attackBase + Calc_GetRandVal( pCharDef->m_attackRange );
	}

	return( iDmg );
}

void CChar::Fight_ResetWeaponSwingTimer()
{
	if ( Fight_IsActive())
	{
		// The target or the weapon might have changed.
		// So restart the skill
		Skill_Start( Fight_GetWeaponSkill() );
	}
}

int CChar::Fight_GetWeaponSwingTimer()
{
	// We have just equipped the weapon or gone into War mode.
	// Set the swing timer for the weapon or on us for .
	//   m_atFight.m_War_Swing_State = WAR_SWING_EQUIPPING or WAR_SWING_SWINGING;
	// RETURN:
	//  tenths of a sec. TICK_PER_SEC

	return( g_Cfg.Calc_CombatAttackSpeed( this, m_uidWeapon.ItemFind()));
}

void CChar::Fight_ClearAll()
{
	// clear all my active targets. Toggle out of war mode.
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsMemoryTypes(MEMORY_WAR_TARG))
			continue;
		Memory_ClearTypes( STATIC_CAST <CItemMemory *>(pItem), MEMORY_WAR_TARG|MEMORY_IAGGRESSOR );
	}

	// Our target is gone.
	StatFlag_Clear( STATF_War );
	Skill_Start( SKILL_NONE );
	m_Act_Targ.InitUID();
	UpdateMode();
}

CChar * CChar::Fight_FindBestTarget()
{
	// If i am an NPC with no more targets then drop out of war mode.
	// RETURN:
	//  number of targets.

	// DEBUG_CHECK( IsStatFlag( STATF_War ));

	SKILL_TYPE skillWeapon = Fight_GetWeaponSkill();
	int iClosest = INT_MAX;	// closest
	CChar * pChar = NULL;
	CChar * pClosest = NULL;

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsMemoryTypes(MEMORY_WAR_TARG))
			continue;
		pChar = pItem->m_uidLink.CharFind();
		int iDist = GetDist(pChar);

		if ( skillWeapon == SKILL_ARCHERY )
		{
			// archery dist is different.
			if ( iDist < g_Cfg.m_iArcheryMinDist || iDist > g_Cfg.m_iArcheryMaxDist )
				continue;
			if ( m_Act_Targ == pChar->GetUID())	// alternate.
				continue;
			return( pChar );
		}
		else if ( iDist < iClosest )
		{
			// ??? in the npc case. can i actually reach this target ?
			pClosest = pChar;
			iClosest = iDist;
		}
	}

	if ( pClosest )
		return( pClosest );
	return( pChar );
}

bool CChar::Fight_Clear( const CChar * pChar )
{
	// I no longer want to attack this char.
	// ASSUME because they are dead.

	if (pChar!=NULL)
	{
		//DEBUG_CHECK( IsStatFlag( STATF_War ));

		CItemMemory * pFight = Memory_FindObj( pChar ); // My memory of the fight.
		if ( pFight != NULL )
		{
			Memory_ClearTypes( pFight, MEMORY_FIGHT|MEMORY_WAR_TARG|MEMORY_IAGGRESSOR );
		}
	}

	// Go to my next target.
	pChar = Fight_FindBestTarget();
	if ( pChar == NULL )
	{
		// If i am an NPC with no more targets then drop out of war mode.
		if ( m_pPlayer == NULL )
		{
			Fight_ClearAll();
		}
	}
	else
	{
		Fight_Attack(pChar);	// attack my next best target instead.
	}

	return( pChar != NULL );	// I did not know about this ?
}

bool CChar::Fight_Stunned( DIR_TYPE dir )
{
	// I have been hit and stunned.
	// Get thrown back if possible.

#if 0

	CAnMoveTo()

#endif

	return( false );
}

bool CChar::Fight_Attack( const CChar * pCharTarg )
{
	// We want to attack some one.
	// But they won't notice til we actually hit them.
	// This is just my intent.
	// RETURN:
	//  true = new attack is accepted.

	if ( pCharTarg == NULL ||
		pCharTarg == this ||
		! CanSee(pCharTarg) ||
		pCharTarg->IsStatFlag( STATF_DEAD ) ||
		pCharTarg->IsDisconnected() ||
		IsStatFlag( STATF_DEAD ))
	{
		// Not a valid target.
		Fight_Clear( pCharTarg );
		return( false );
	}


	if (m_Act_Targ != pCharTarg->GetUID())
		if (OnTrigger(CTRIG_Attack, (CTextConsole *)pCharTarg) == TRIGRET_RET_TRUE)
			return false;

	if ( GetPrivLevel() <= PLEVEL_Guest &&
		pCharTarg->m_pPlayer &&
		pCharTarg->GetPrivLevel() > PLEVEL_Guest )
	{
		SysMessageDefault( DEFMSG_GUEST );
		Fight_Clear( pCharTarg );
		return( false );
	}

	// Record the start of the fight.
	Memory_Fight_Start( pCharTarg );

	// I am attacking. (or defending)
	StatFlag_Set( STATF_War );


	// Skill interruption ?
	SKILL_TYPE	skillWeapon	= Fight_GetWeaponSkill();
	SKILL_TYPE	skillActive	= Skill_GetActive();

	if ( skillActive == skillWeapon && m_Act_Targ == pCharTarg->GetUID() )
		return true;

	const CSpellDef *	pSpellDef;
	if (   skillActive == SKILL_MAGERY
		&& (pSpellDef = g_Cfg.GetSpellDef(m_atMagery.m_Spell)) )
	{
		int iInterrupt = pSpellDef->m_Interrupt.GetLinear(
			       			Skill_GetBase(SKILL_MAGERY) );
		if ( Calc_GetRandVal( 1000 ) >= iInterrupt )
			return true;
	}
	m_Act_Targ = pCharTarg->GetUID();
	Skill_Start( skillWeapon );

	return( true );
}

bool CChar::Fight_AttackNext()
{
	return Fight_Attack( Fight_FindBestTarget());
}

void CChar::Fight_HitTry()
{
	// A timer has expired so try to take a hit.
	// I am ready to swing or already swinging.
	// but i might not be close enough.
	// RETURN:
	//  false = no swing taken
	//  true = continue fighting

	ASSERT( Fight_IsActive());
	ASSERT( m_atFight.m_War_Swing_State == WAR_SWING_READY || m_atFight.m_War_Swing_State == WAR_SWING_SWINGING );

	CChar * pCharTarg = m_Act_Targ.CharFind();
	if ( pCharTarg == NULL )
	{
		// Might be dead ? Clear this.
		// move to my next target.
		Fight_AttackNext();
		return;
	}

	// Try to hit my target. I'm ready.
	switch ( Fight_Hit( pCharTarg ))
	{
	case WAR_SWING_INVALID:	// target is invalid.
		Fight_Clear( pCharTarg );
		Fight_AttackNext();
		return;
	case WAR_SWING_EQUIPPING:
		// Assume I want to continue hitting
		// Swing again. (start swing delay)
		// same target.
		{
		SKILL_TYPE skill = Skill_GetActive();
		Skill_Cleanup();	// Smooth transition = not a cancel of skill.
		Skill_Start( skill );
		}
		return;
	case WAR_SWING_READY:	// probably too far away. can't take my swing right now.
		// Try for a diff target ?
		Fight_AttackNext();
		return;
	case WAR_SWING_SWINGING:	// must come back here again to complete.
		return;
	}

	ASSERT(0);
}


int		CChar::CalcFightRange( CItem * pWeapon, CItemBase * pWeaponDef )
{
	if ( pWeapon )
		pWeaponDef	= pWeapon->Item_GetDef();

	if ( !pWeaponDef )
		return Char_GetDef()->RangeL();

	return Char_GetDef()->RangeL() + pWeaponDef->RangeL() - 1;
}


WAR_SWING_TYPE CChar::Fight_Hit( CChar * pCharTarg )
{
	// Attempt to hit our target.
	// pCharTarg = the target.
	//  NOT - WAR_SWING_EQUIPPING = 0,	// we are recoiling our weapon.
	//  WAR_SWING_READY,			// we can swing at any time.
	//  WAR_SWING_SWINGING,			// we are swinging our weapon.
	// RETURN:
	//  WAR_SWING_INVALID = target is invalid
	//  WAR_SWING_EQUIPPING = swing made.
	//  WAR_SWING_READY = can't take my swing right now. but i'm ready
	//  WAR_SWING_SWINGING = taking my swing now.

	if ( pCharTarg == NULL )
		return( WAR_SWING_INVALID );

	if ( IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Freeze | STATF_Stone ) ||
		pCharTarg->IsStatFlag( STATF_DEAD ))
		return( WAR_SWING_INVALID );

	int dist = GetTopDist3D( pCharTarg );
	if ( dist > UO_MAP_VIEW_RADAR )
		return( WAR_SWING_INVALID );

	CItem * pWeapon			= m_uidWeapon.ItemFind();
	CItem * pAmmo			= NULL;
	CItemBase * pWeaponDef	= NULL;
	if ( pWeapon )
		pWeaponDef			= pWeapon->Item_GetDef();

	SKILL_TYPE skill = Skill_GetActive();
	if ( skill == SKILL_ARCHERY )
	{
		// Archery type skill.
		ASSERT( pWeapon );
		DEBUG_CHECK( pWeapon->IsItemEquipped());
		ASSERT( pWeaponDef );
		DEBUG_CHECK( pWeaponDef->IsType(IT_WEAPON_BOW) || pWeaponDef->IsType(IT_WEAPON_XBOW));

		int	iMinDist	= pWeaponDef->RangeH();
		int	iMaxDist	= pWeaponDef->RangeL();

		if ( !iMaxDist || (iMinDist == 0 && iMaxDist == 1) )
			iMaxDist	= g_Cfg.m_iArcheryMaxDist;
		if ( !iMinDist )
			iMinDist	= g_Cfg.m_iArcheryMinDist;

		if ( dist > iMaxDist )
			return( WAR_SWING_READY );	// can't hit now.

		if ( ! CanSeeLOS( pCharTarg ))
			return( WAR_SWING_READY );

		if ( IsStatFlag( STATF_HasShield ))	// this should never happen.
		{
			SysMessageDefault( DEFMSG_ITEMUSE_BOW_SHIELD );
			return( WAR_SWING_EQUIPPING );
		}

		Reveal();
		UpdateDir( pCharTarg );

		if ( dist < iMinDist )
		{
			// ??? the bow is acting like a (poor) blunt weapon at this range?
			SysMessageDefault( DEFMSG_COMBAT_ARCH_TOOCLOSE );
			int iTime = Fight_GetWeaponSwingTimer();
			UpdateAnimate( ANIM_ATTACK_1H_WIDE, false, false, iTime/TICK_PER_SEC );
			return( WAR_SWING_EQUIPPING );
		}

		// Consume the bolts/arrows
		ITEMID_TYPE AmmoID = (ITEMID_TYPE) pWeaponDef->m_ttWeaponBow.m_idAmmo.GetResIndex();
		if ( AmmoID )
		{
			pAmmo = ContentFind( pWeaponDef->m_ttWeaponBow.m_idAmmo );
			if ( m_pPlayer && pAmmo == NULL )
			{
				SysMessageDefault( DEFMSG_COMBAT_ARCH_NOAMMO );
				Skill_Start( SKILL_NONE );
				return( WAR_SWING_INVALID );
			}
		}

		if ( m_atFight.m_War_Swing_State == WAR_SWING_READY )
		{
			// just start the bow animation.
			int iTime = Fight_GetWeaponSwingTimer();
			CScriptTriggerArgs	Args( iTime, 0, pWeapon );
			if ( OnTrigger( CTRIG_HitTry, pCharTarg, &Args ) == TRIGRET_RET_TRUE )
				return( WAR_SWING_READY );
			iTime = Args.m_iN1;

			m_atFight.m_War_Swing_State = WAR_SWING_SWINGING;
			m_atFight.m_fMoved	= 0;
			SetTimeout( iTime * 3 / 4 );	// try again sooner.
			UpdateAnimate( ANIM_ATTACK_WEAPON, true, false, iTime/TICK_PER_SEC );
			return( WAR_SWING_SWINGING );
		}

		// now use the ammo
		if ( pAmmo )
		{
			pAmmo->UnStackSplit( 1, this );
			pAmmo->Delete();	// Delet eby default.
		}

		pCharTarg->Effect( EFFECT_BOLT, (ITEMID_TYPE) pWeaponDef->m_ttWeaponBow.m_idAmmoX.GetResIndex(), this, 16, 0, false );
	}
	else
	{
		int	iMinDist	= pWeaponDef ? pWeaponDef->RangeH() : 0;
		int	iMaxDist	= CalcFightRange( NULL, pWeaponDef );

		// A hand weapon of some sort.
		// if ( ! CanTouch( pCharTarg )) return( 2 );
		if ( m_atFight.m_War_Swing_State == WAR_SWING_READY )
		{
			if ( dist < iMinDist )
				return( WAR_SWING_READY );	// can't hit now.

			if ( dist > iMaxDist )
				return( WAR_SWING_READY );	// can't hit now.

			if ( ! CanSeeLOS( pCharTarg ) )
				return( WAR_SWING_READY );

			Reveal();
			UpdateDir( pCharTarg );

			// We are swinging.
			int iTime = Fight_GetWeaponSwingTimer();
			CScriptTriggerArgs Args( iTime, 0, pWeapon );
			if ( OnTrigger( CTRIG_HitTry, pCharTarg, &Args ) == TRIGRET_RET_TRUE )
				return( WAR_SWING_READY );
			iTime = Args.m_iN1;

			m_atFight.m_War_Swing_State = WAR_SWING_SWINGING;
			m_atFight.m_fMoved	= 0;

			SetTimeout( iTime/2 );	// try again sooner.
			UpdateAnimate( ANIM_ATTACK_WEAPON, true, false, iTime/TICK_PER_SEC );
			return( WAR_SWING_SWINGING );
		}
		// UpdateAnimate( ANIM_ATTACK_WEAPON, true, true );	// recoil anim

		if ( dist < iMinDist )
			return( WAR_SWING_READY );	// can't hit now.

		if ( dist > iMaxDist+1 )
			return( WAR_SWING_READY );	// can't hit now.

		if ( ! CanSeeLOS( pCharTarg ) )
			return( WAR_SWING_READY );

		Reveal();
		UpdateDir( pCharTarg );
	}

	ASSERT( m_atFight.m_War_Swing_State != WAR_SWING_READY );

	// We made our swing. so we must recoil.
	m_atFight.m_War_Swing_State = WAR_SWING_EQUIPPING;
	m_atFight.m_fMoved	= 0;
	int iTime = Fight_GetWeaponSwingTimer();
	SetTimeout( iTime/2 );	// try again sooner.

	// Use up some stamina to hit ?
	int iWeaponWeight = (( pWeapon ) ? ( pWeapon->GetWeight()/WEIGHT_UNITS + 1 ) : 1 );
	if ( ! Calc_GetRandVal( 10 + iWeaponWeight ))
	{
		UpdateStatVal( STAT_DEX, -1 );
	}

	// Check if we hit something;
	if ( m_Act_Difficulty < 0 )
	{
		CScriptTriggerArgs	Args ( 0, 0, pWeapon );
		if ( OnTrigger( CTRIG_HitMiss, pCharTarg, &Args ) == TRIGRET_RET_TRUE )
			return( WAR_SWING_EQUIPPING );
	}

	if ( m_Act_Difficulty < 0 )		// if not changed within trigger
	{
		// We missed. (miss noise)
		if ( skill == SKILL_ARCHERY )
		{
			// 0x223 = bolt miss or dart ?
			// do some thing with the arrow.
			Sound( Calc_GetRandVal(2) ? 0x233 : 0x238 );
			// Sometime arrows should be lost/broken when we miss

			if ( pAmmo && Calc_GetRandVal(5))
			{
				// int pAmmo->OnTakeDamage( 2, pSrc, DAMAGE_HIT_BLUNT )
				pAmmo->MoveToCheck( pCharTarg->GetTopPoint(), this );
			}
			return( WAR_SWING_EQUIPPING );	// Made our full swing. (tho we missed)
		}

		if ( skill == SKILL_WRESTLING )
		{
			if ( SoundChar( Calc_GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 ))
				return( WAR_SWING_EQUIPPING );
		}

		static const SOUND_TYPE sm_Snd_Miss[] =
		{
			0x238, // = swish01
			0x239, // = swish02
			0x23a, // = swish03
		};

		Sound( sm_Snd_Miss[ Calc_GetRandVal( COUNTOF( sm_Snd_Miss )) ] );

		if ( IsPriv(PRIV_DETAIL))
		{
			SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_MISSS ), (LPCTSTR) pCharTarg->GetName());
		}
		if ( pCharTarg->IsPriv(PRIV_DETAIL))
		{
			pCharTarg->SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_COMBAT_MISSO ), (LPCTSTR) GetName());
		}

		return( WAR_SWING_EQUIPPING );	// Made our full swing. (tho we missed)
	}

	// BAD BAD Healing fix.. Cant think of something else -- Radiant
	if (pCharTarg->m_Act_SkillCurrent == SKILL_HEALING)
	{
		pCharTarg->SysMessageDefault( DEFMSG_HEALING_INTERRUPT );
		pCharTarg->Skill_Cleanup();
	}

	// We hit...
	// Calculate base damage.
	int	iDmg = Fight_CalcDamage( pWeapon, skill );

	//if ( iDmg ) // use OnTakeDamage below instead.
	//	pCharTarg->OnHarmedBy( this, iDmg );

	CScriptTriggerArgs	Args( iDmg, 0, pWeapon );
	if ( OnTrigger( CTRIG_Hit, pCharTarg, &Args ) == TRIGRET_RET_TRUE )
		return( WAR_SWING_EQUIPPING );
	iDmg	= Args.m_iN1;

	if ( skill == SKILL_ARCHERY )
	{
		// There's a chance that the arrow will stick in the target
		if ( pAmmo && !Calc_GetRandVal(2))
		{
			// int pAmmo->OnTakeDamage( 2+iDmg, pCharTarg, DAMAGE_HIT_BLUNT )

			pCharTarg->ItemBounce( pAmmo );
		}
	}

	// Raise skill
	Skill_UseQuick( SKILL_TACTICS, pCharTarg->Skill_GetBase(SKILL_TACTICS)/10 );

	// Hit noise. based on weapon type.
	SoundChar( CRESND_HIT );

	if ( pWeapon != NULL )
	{
		// poison weapon ?
		if ( pWeapon->m_itWeapon.m_poison_skill &&
			Calc_GetRandVal( 100 ) < pWeapon->m_itWeapon.m_poison_skill )
		{
			// Poison delivered.
			int iPoisonDeliver = Calc_GetRandVal(pWeapon->m_itWeapon.m_poison_skill);

			pCharTarg->SetPoison( 10*iPoisonDeliver, iPoisonDeliver/5, this );

			// Diminish the poison on the weapon.
			pWeapon->m_itWeapon.m_poison_skill -= iPoisonDeliver / 2;
		}

		// damage the weapon ?
		pWeapon->OnTakeDamage( iDmg/4, pCharTarg );
	}
	else
	{
		// Base type attack for our body. claws/etc
		// intrinsic attacks ?
		// Poisonous bite/sting ?
		if ( m_pNPC &&
			m_pNPC->m_Brain == NPCBRAIN_MONSTER &&
			Skill_GetBase(SKILL_POISONING) > 300 &&
			Calc_GetRandVal( 1000 ) < Skill_GetBase(SKILL_POISONING))
		{
			// Poison delivered.
			int iSkill = Skill_GetAdjusted( SKILL_POISONING );
			pCharTarg->SetPoison( Calc_GetRandVal(iSkill), Calc_GetRandVal(iSkill/50), this );
		}
	}

	// Took my swing. Do Damage !
	iDmg = pCharTarg->OnTakeDamage( iDmg, this, ( DAMAGE_HIT_BLUNT | DAMAGE_HIT_PIERCE | DAMAGE_HIT_SLASH ));
	if ( iDmg > 0 )
	{
		// Is we do no damage we get no experience!
		Skill_Experience( skill, m_Act_Difficulty );	// Get experience for it.
	}

	return( WAR_SWING_EQUIPPING );	// Made our full swing.
}

