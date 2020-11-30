//
// CCharNPC.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Actions specific to an NPC.
//

#include "graysvr.h"	// predef header.

bool CChar::SetPlayerAccount( CAccount * pAccount )
{
	// Set up the char as a Player.
	if ( pAccount == NULL )
	{
		DEBUG_ERR(( "SetPlayerAccount '%s' NULL\n", GetName()));
		return false;
	}
	if ( m_pNPC != NULL )
	{
		delete m_pNPC;
		m_pNPC = NULL;
	}
	if ( m_pPlayer != NULL )
	{
		if ( m_pPlayer->GetAccount() == pAccount )
			return( true );
		DEBUG_ERR(( "SetPlayerAccount '%s' already set '%s' != '%s' !\n", GetName(), m_pPlayer->GetAccount()->GetName(), pAccount->GetName()));
		return( false );
	}
	m_pPlayer = new CCharPlayer( pAccount );
	return( true );
}

bool CChar::SetPlayerAccount( const TCHAR * pszAccount )
{
	CAccount * pAccount = g_World.AccountFind( pszAccount );
	if ( pAccount == NULL )
	{
		DEBUG_ERR(( "SetPlayerAccount '%s' can't find '%s'!\n", GetName(), pszAccount ));
		return false;
	}
	return( SetPlayerAccount( pAccount ));
}

void CChar::SetNPCBrain( NPCBRAIN_TYPE NPCBrain )
{
	// Set up the char as an NPC
	if ( NPCBrain == NPCBRAIN_NONE || IsClient())
	{
		DEBUG_ERR(( "SetNPCBrain NULL\n" ));
		return;
	}
	if ( m_pPlayer != NULL )
	{
		delete m_pPlayer;
		m_pPlayer = NULL;
	}
	if ( m_pNPC == NULL )
	{
		m_pNPC = new CCharNPC( NPCBrain );
	}
	else
	{
		m_pNPC->m_Brain = NPCBrain;
	}
}

//////////////////////////
// -CCharPlayer

const TCHAR * CCharPlayer::sm_KeyTable[] =
{
	"ACCOUNT",
	"KILLS",
	"PLOT1",
	"PLOT2",
	"SKILLCLASS",
};

CCharPlayer::CCharPlayer( CAccount * pAccount ) :
	m_pAccount( pAccount )
{
	ASSERT(pAccount);
	m_Murders = 0;
	m_Plot1 = 0;
	m_Plot2 = 0;
	m_SkillClass = 0;
	memset( m_SkillLock, 0, sizeof( m_SkillLock ));
}

SKILL_TYPE CCharPlayer::Skill_GetLockType( const TCHAR * pKey ) const
{
	// only players can have skill locks.

	TCHAR szTmpKey[128];
	strcpy( szTmpKey, pKey );

	TCHAR * ppArgs[2];
	int i = ParseCmds( szTmpKey, ppArgs, COUNTOF(ppArgs), ".[]" );
	if ( i <= 0 )
		return( SKILL_NONE );

	if ( isdigit( ppArgs[1][0] ))
	{
		i = atoi(ppArgs[1]);
	}
	else
	{
		i = g_Serv.FindSkillKey( ppArgs[1] );
	}
	if ( i >= SKILL_QTY )
		return( SKILL_NONE );
	return( (SKILL_TYPE) i );
}

bool CCharPlayer::r_WriteVal( const TCHAR * pszKey, CGString & sVal )
{
	ASSERT( GetAccount());

	if ( ! strnicmp( pszKey, "SkillLock", 9 ))
	{
		// "SkillLock[alchemy]"
		SKILL_TYPE skill = Skill_GetLockType( pszKey );
		if ( skill <= SKILL_NONE )
			return( false );
		sVal.FormatVal( Skill_GetLock( skill ));
		return( true );
	}

	switch ( FindTableSorted( pszKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "ACCOUNT",
		sVal = m_pAccount->GetName();
		return( true );
	case 1:	// "KILLS",
		sVal.FormatVal( m_Murders );
		return( true );
	case 2:	// "PLOT1",
		sVal.FormatHex( m_Plot1 );
		return( true );
	case 3:	// "PLOT2",
		sVal.FormatHex( m_Plot2 );
		return( true );
	case 4: // "SKILLCLASS"
		sVal.FormatVal( m_SkillClass );
		return( true );
	}
	return( false );
}

bool CCharPlayer::r_LoadVal( CScript &s )
{
	if ( ! strnicmp( s.GetKey(), "SkillLock", 9 ))
	{
		const TCHAR * sm_States[] =
		{
			"UP",
			"DOWN",
			"LOCK",
		};

		SKILL_TYPE skill = Skill_GetLockType( s.GetKey());
		if ( skill <= SKILL_NONE )
			return( false );
		int bState;
		if ( isdigit( s.GetArgStr()[0] ))
		{
			bState = atoi( s.GetArgStr());
		}
		else
		{
			bState = FindTable( s.GetArgStr(), sm_States, COUNTOF( sm_States ));
		}
		if ( bState < SKILLLOCK_UP || bState > SKILLLOCK_LOCK )
			return( false );
		Skill_SetLock( skill, (SKILLLOCK_TYPE) bState );
		return true;
	}

	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 1:	// "KILLS",
		m_Murders = s.GetArgVal();
		return true;
	case 2:	// "PLOT1",
		m_Plot1 = s.GetArgHex();
		return true;
	case 3:	// "PLOT2",
		m_Plot2 = s.GetArgHex();
		return true;
	case 4:	// "SKILLCLASS"
		{
		int i = s.GetArgVal();
		if ( i < 0 || i >= g_Serv.m_SkillClassDefs.GetCount())
			return( false );
		m_SkillClass = i;
		}
		return true;
	}
	return( false );
}

void CCharPlayer::r_Write( CScript & s ) const
{
	ASSERT(m_pAccount);
	s.WriteKey( "ACCOUNT", m_pAccount->GetName());

	if ( m_Murders )
		s.WriteKeyVal( "KILLS", m_Murders );
	if ( m_Plot1 )
		s.WriteKeyHex( "PLOT1", m_Plot1 );
	if ( m_Plot2 )
		s.WriteKeyHex( "PLOT2", m_Plot2 );
	if ( m_SkillClass )
		s.WriteKeyVal( "SKILLCLASS", m_SkillClass );

	for ( int j=0;j<SKILL_QTY;j++)	// Don't write all lock states!
	{
		if ( ! m_SkillLock[j] )
			continue;
		TCHAR szTemp[64];
#if 1
		sprintf( szTemp, "SkillLock[%d]", j );	// smaller storage space.
		s.WriteKeyVal( szTemp, m_SkillLock[j] );
#else
		sprintf( szTemp, "SkillLock[%s]", g_Serv.m_SkillDefs[j]->GetKey());
		s.WriteKey( szTemp, (( m_SkillLock[j] == 2 ) ? "Lock" :
			(( m_SkillLock[j] == 1) ? "Down" : "Up")));
#endif
	}
}

bool CChar::Player_OnVerb( CScript &s, CTextConsole * pSrc ) // Execute command from script
{
	ASSERT( m_pPlayer );

	static const TCHAR * table[] =
	{
		"EMAIL",
		"KICK",
		"PASSWORD",
	};

	switch ( FindTableSorted( s.GetKey(), table, COUNTOF(table)))
	{
	case 0:	// "EMAIL"
		// Sets the email for the players account.
		m_pPlayer->GetAccount()->r_Verb( s, pSrc );
		break;
	case 1: // "KICK" = kick and block the account
		if ( ! IsClient()) 
			return false;
		return( m_pClient->addKick( pSrc ));
	case 2:	// "PASSWORD"
		// Set/Clear the password
		if ( pSrc != this )
		{
			if ( pSrc->GetPrivLevel() <= GetPrivLevel() ||
				pSrc->GetPrivLevel() < PLEVEL_GM )
			{
				pSrc->SysMessage( "You are not privileged to do this." );
				return( false );
			}
		}
		if ( ! s.HasArgs())
		{
			m_pPlayer->GetAccount()->ClearPassword();
			SysMessage( "Password has been cleared." );
			SysMessage( "Log out, then back in to set the new password." );
			g_Log.Event( LOGM_ACCOUNTS|LOGL_EVENT, "Account '%s', password cleared", m_pPlayer->GetAccount()->GetName());
		}
		else
		{
			m_pPlayer->GetAccount()->SetPassword( s.GetArgStr());
			SysMessage( "Password has been set" );
			g_Log.Event( LOGM_ACCOUNTS|LOGL_EVENT, "Account '%s', password set to '%s'", m_pPlayer->GetAccount()->GetName(), s.GetArgStr());
		}
		break;

	default:
		return( false );
	}

	return( true );
}

//////////////////////////
// -CCharNPC

CCharNPC::CCharNPC( NPCBRAIN_TYPE NPCBrain )
{
	m_Brain = NPCBrain;
	m_Home_Dist_Wander = 0xFFFF;	// as far as i want.
	m_Act_Motivation = 0;
}

const TCHAR * CCharNPC::sm_KeyTable[] =
{
	"ACTPRI",
	"BRAIN",
	"DESIRES",
	"HOMEDIST",
	"KNOWLEDGE",
	"MOTIVES",
	"NEED",
	"NPC",
	"SPEECH",
};

bool CCharNPC::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ACTPRI"
		m_Act_Motivation = s.GetArgVal();
		break;
	case 1:// "BRAIN",
scp_brain:
		m_Brain = (NPCBRAIN_TYPE) s.GetArgVal();
		break;
	case 2: // "DESIRES",
scp_desires:
		// return( m_Motives.r_LoadVal( s ));
		return( false );
	case 3:	// "HOMEDIST"
		m_Home_Dist_Wander = s.GetArgVal();
		break;
	case 4:// "KNOWLEDGE",
scp_speech:
		// ??? Check that it is not already part of m_pDef->m_Speech
		return( m_Speech.r_LoadVal( s ));
	case 5: // "MOTIVES"
		goto scp_desires;
	case 6: // "NEED"
		m_sNeed = s.GetArgStr();
		break;
	case 7:// "NPC",
		goto scp_brain;
	case 8:// "SPEECH",
		goto scp_speech;
	default:
		return(false );
	}
	return( true );
}

bool CCharNPC::r_WriteVal( const TCHAR * pKey, CGString & sVal )
{
	if ( ! strcmpi( pKey, "HINT" ))	
	{
		// Some sort of hint about the area.	// ???
		sVal = "don't take any wooden nickles";
		return( true );
	}

	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:  // "ACTPRI"
		sVal.FormatVal( m_Act_Motivation );
		break;
	case 1:// "BRAIN",
scp_brain:
		sVal.FormatVal( m_Brain );
		break;
	case 2: // "DESIRES"
		// Write out all the motive blocks and their priorities.
scp_desires:
		// m_Motives.WriteFragList( sVal );
		return( false );
	case 3:	// "HOMEDIST"
		sVal.FormatVal( m_Home_Dist_Wander );
		break;
	case 4:// "KNOWLEDGE",
		// Write out all the speech blocks.
scp_speech:
		m_Speech.WriteFragList( sVal );
		break;
	case 5: // "MOTIVES"
		goto scp_desires;
	case 6: // "NEED"
		sVal = m_sNeed;
		break;
	case 7:// "NPC",
		goto scp_brain;
	case 8:// "SPEECH",
		goto scp_speech;
	default:
		return( false );
	}

	return( true );
}

void CCharNPC::r_Write( CScript & s ) const
{
	// This says we are an NPC.
	s.WriteKeyVal( "NPC", m_Brain );

	if ( m_Home_Dist_Wander != 0xFFFF )
		s.WriteKeyVal( "HOMEDIST", m_Home_Dist_Wander );
	if ( m_Act_Motivation )
		s.WriteKeyHex( "ACTPRI", m_Act_Motivation );

	m_Speech.r_Write( s, "SPEECH" );

	if ( ! m_sNeed.IsEmpty())
	{
		s.WriteKey( "NEED", m_sNeed );	// Write out my inventory here !!!
	}
}



