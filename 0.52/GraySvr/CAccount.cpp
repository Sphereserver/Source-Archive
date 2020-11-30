//
// CAccount.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

void globalstartsymbol()	// put this here as just the starting offset.
{
}

//////////////////////////////////////////////////////////////////////

bool CWorld::LoadAccounts( bool fChanges, bool fClearChanges )
{
	// Load the accounts file. (at start up)

	const TCHAR * pszBaseName;
	if ( fChanges )
	{
		pszBaseName = GRAY_FILE "acct"; // Open script file
	}
	else
	{
		pszBaseName = GRAY_FILE "accu";
	}

	if ( g_Serv.m_sAcctBaseDir.IsEmpty())
	{
		g_Serv.m_sAcctBaseDir = g_Serv.m_sWorldBaseDir;
	}

	CGString sLoadName;
	sLoadName.Format( _TEXT("%s%s"), (const TCHAR*) g_Serv.m_sAcctBaseDir, pszBaseName );

	CScript s;
	if ( ! s.Open( sLoadName ))
	{
		if ( ! fChanges )
		{
			if ( LoadAccounts( true ))	// if we have changes then we are ok.
				return( true );
			g_Log.Event( LOGL_FATAL|LOGM_INIT, "Can't open account file '%s'\n", s.GetFilePath());
		}
		return false;
	}

	if ( fClearChanges )
	{
		ASSERT( fChanges );
		// empty the changes file.
		s.Close();
		s.Open( NULL, OF_WRITE );
		s.WriteStr( "// Accounts are periodically moved to the " GRAY_FILE "accu" GRAY_SCRIPT " file.\n"
			"// All account changes should be made here.\n"
			"// Use the /ACCOUNT UPDATE command to force accounts to update.\n"
			);
		return true;
	}

	int iAccounts = 0;

	while (s.FindNextSection())
	{
		CAccount * pAccount = AccountFind( s.GetKey());
		if ( pAccount )
		{
			if ( ! fChanges )
			{
				g_Log.Event( LOGL_ERROR|LOGM_INIT, _TEXT("Account '%s': duplicate name\n"), pAccount->GetName());
			}
		}
		else
		{
			TCHAR szName[ MAX_ACCOUNT_NAME_SIZE ];
			if ( ! CAccount::NameStrip( szName, s.GetKey()))
				continue;
			pAccount = new CAccount( szName );
			ASSERT(pAccount);
			iAccounts ++;
		}
		pAccount->r_Load( s );
	}

	if ( ! fChanges )
	{
		LoadAccounts( true );
	}

	return( true );
}

bool CWorld::SaveAccounts()
{
	// Look for changes FIRST.
	LoadAccounts( true );

	CGString sArchive;
	GetBackupName( sArchive, 'a' );

	// remove previous archive of same name
	remove( sArchive );

	// rename previous save to archive name.
	CGString sSaveName;
	sSaveName.Format( "%s" GRAY_FILE "accu%s",
		(const TCHAR*) g_Serv.m_sAcctBaseDir,
		g_Serv.m_fSaveBinary ? GRAY_BINARY : GRAY_SCRIPT );

	if ( rename( sSaveName, sArchive ))
	{
		// May not exist if this is the first time.
		g_Log.Event( LOGL_ERROR, "Account Save Rename to '%s' FAILED\n", (const TCHAR*) sArchive );
	}

	CScript s;
	if ( ! s.Open( sSaveName, OF_WRITE|((g_Serv.m_fSaveBinary)?OF_BINARY:OF_TEXT)))
	{
		return( false );
	}

	s.Printf( "\\\\ " GRAY_TITLE " %s accounts file\n"
		"\\\\ NOTE: This file cannot be edited while the server is running.\n"
		"\\\\ Any file changes must be made to " GRAY_FILE "accu" GRAY_SCRIPT ". This is read in at save time.\n",
		g_Serv.GetName());

	int i=0;
	for ( ; i<m_Accounts.GetCount(); i++ )
	{
		m_Accounts[i]->r_Write(s);
	}

	LoadAccounts( true, true );	// clear the change file now.
	return( true );
}

CAccount * CWorld::AccountFind( const TCHAR * pszName, bool fAutoCreate, bool fGuest )
{
	TCHAR szName[ MAX_ACCOUNT_NAME_SIZE ];
	if ( ! CAccount::NameStrip( szName, pszName ))
		return( NULL );

	int i = m_Accounts.FindKey( pszName );
	if ( i >= 0 )
	{
		return( m_Accounts[i] );
	}

	if ( fAutoCreate )	// Create if not found.
	{
		return new CAccount( pszName, fGuest );
	}

	return( NULL );
}

bool CWorld::OnAccountCmd( TCHAR * pszArgs, CTextConsole * pSrc )
{
	TCHAR * ppCmd[3];
	int iQty = ParseCmds( pszArgs, ppCmd, COUNTOF( ppCmd ));

	const TCHAR *pszAccount = ppCmd[0];
	const TCHAR *pVerb = ppCmd[1];
	pszArgs = ppCmd[2];

	// Modify the accounts on line. "ACCOUNT"

	if ( iQty <= 0 ||
		pszAccount == NULL ||
		pszAccount[0] == '\0' ||
		pszAccount[0] == '?' ||
		! strcmpi( pszAccount, "HELP" ))
	{
		static const TCHAR * pCmds[] =
		{
			// "/ACCOUNT LIST\n",
			"/ACCOUNT UPDATE\n",
			"/ACCOUNT UNUSED days [command]\n",
			"/ACCOUNT name ADD password",
			"/ACCOUNT name BLOCK 0/1\n",
			"/ACCOUNT name JAIL 0/1\n",
			"/ACCOUNT name DELETE = delete all chars and the account\n",
			"/ACCOUNT name PLEVEL x = set account priv level\n",
			// "/ACCOUNT name DELETE x = delete this char",
			// "/ACCOUNT name SUMMON x = force this char to be on line and here.",
			// "/ACCOUNT name DISCONNECT = log out a summoned char.",
		};
		for ( int i=0; i<COUNTOF(pCmds); i++ )
		{
			pSrc->SysMessage( pCmds[i] );
		}
		return true;
	}
	if ( ! strcmpi( pszAccount, "UPDATE" ))
	{
		SaveAccounts();
		return true;
	}
	if ( ! strcmpi( pszAccount, "UNUSED" ))
	{
		// do something to all the unused accounts.
		int iDaysTest = Exp_GetVal( pVerb );
		CRealTime date;
		date.FillRealTime();
		int iDaysCur = date.GetDaysTotal();

		iQty = ParseCmds( pszArgs, ppCmd, 2 );

		int iCount = 0;
		for ( int i=0; i<m_Accounts.GetCount(); i++ )
		{
			CAccount * pAccount = m_Accounts[i];
			int iDaysAcc = pAccount->m_Last_Connect_Date.GetDaysTotal();
			if ( ! iDaysAcc )
			{
				// account has never been used ? (get create date instead)
				iDaysAcc = pAccount->m_First_Connect_Date.GetDaysTotal();
			}

			int iDaysDiff = iDaysCur - iDaysAcc;
			if ( iDaysDiff <= iDaysTest )
				continue;
			// Account has not been used in a while.

			iCount ++;
			if ( iQty <= 0 || ppCmd[0][0] == '\0' )
			{
				// just list stuff about the account.
				pAccount->r_Verb( CScript( "SHOW LASTCONNECTDATE" ), pSrc );
				continue;
			}

			// can't delete unused priv accounts this way.
			if ( ! strcmpi( ppCmd[0], "DELETE" ) && pAccount->GetPrivLevel() > PLEVEL_Player )
			{
				pSrc->SysMessagef( "Can't Delete PrivLevel %d Account '%s' for %d days of non-use.\n",
					pAccount->GetPrivLevel(), pAccount->GetName(), iDaysDiff );
				continue;
			}

			pAccount->r_Verb( CScript( ppCmd[0], ppCmd[1] ), pSrc );
		}

		pSrc->SysMessagef( "%d of %d accounts unused for %d days", iCount, m_Accounts.GetCount(), iDaysTest );
		return true;
	}

#if 0
	if ( ! strcmpi( pszAccount, "LIST" ))
	{
		// List all the accounts. 10 at a time.
		int iPage = pVerb ? atoi(pVerb):0;

		return true;
	}
#endif

	bool fAdd = ( pVerb != NULL && ! strcmpi( pVerb, "ADD" ));

	// Must be a valid account ?
	CAccount * pAccount = AccountFind( pszAccount );
	if ( pAccount == NULL )
	{
		if ( ! fAdd )
		{
			pSrc->SysMessagef( "Account '%s' does not exist\n", pszAccount );
			return true;
		}
		pAccount = new CAccount(pszAccount);
		ASSERT(pAccount);
		pVerb = "PASSWORD";
	}
	else
	{
		if ( fAdd )
		{
			pSrc->SysMessagef( "Account '%s' already exists\n", pszAccount );
			return false;
		}
	}

	if ( pVerb == NULL )
	{
		// just list stuff about the account.
		pVerb = "SHOW";
		pszArgs = "PLEVEL";
	}

	return pAccount->r_Verb( CScript( pVerb, pszArgs ), pSrc );
}

//////////////////////////////////////////////////////////////////////////
// -CRealTime

bool CRealTime::Read( TCHAR * pszVal )
{
	TCHAR * ppCmds[7];
	int iQty = ParseCmds( pszVal, ppCmds, COUNTOF(ppCmds), "/,: \t" );

	m_Year = 0;	// invalid by default.

	if ( isdigit( ppCmds[0][0] ))
	{
		// new format is "1999/8/1 14:30:18"
		if ( iQty < 6 )
		{
			return( false );
		}
		m_Year = atoi( ppCmds[0] ) - 1900;
		m_Month = atoi( ppCmds[1] ) - 1;
		m_Day = atoi( ppCmds[2] );
		m_Hour = atoi( ppCmds[3] );
		m_Min = atoi( ppCmds[4] );
		m_Sec = atoi( ppCmds[5] );
	}
	else
	{
		// old format is "Tue Mar 30 14:30:18 1999"
		if ( iQty < 7 )
		{
			return( false );
		}

		switch ( toupper( ppCmds[1][0] ))
		{
		case 'J':
			if ( toupper( ppCmds[1][1] ) == 'A' )
				m_Month = 0;  // january.
			else if ( toupper( ppCmds[1][2] ) == 'N' )
				m_Month = 5; // june
			else
				m_Month = 6; // july
			break;
		case 'F': m_Month = 1; break; // february
		case 'M':
			if ( toupper( ppCmds[1][2] ) == 'R' )
				m_Month = 2; // march
			else
				m_Month = 4; // may
			break;
		case 'A':
			if ( toupper( ppCmds[1][1] ) == 'P' )
				m_Month = 3; // april
			else
				m_Month = 7; // august
			break;
		case 'S': m_Month = 8; break; // september
		case 'O': m_Month = 9; break; // october
		case 'N': m_Month = 10; break; // november
		case 'D': m_Month = 11; break; // december
		default: m_Month = 0; break;
		}

		m_Day = atoi( ppCmds[2] );
		m_Hour = atoi( ppCmds[3] );
		m_Min = atoi( ppCmds[4] );
		m_Sec = atoi( ppCmds[5] );
		m_Year = atoi( ppCmds[6] ) - 1900;
	}

	return( true );
}

const TCHAR * CRealTime::Write( void ) const
{
	TCHAR * pTemp = GetTempStr();
	sprintf( pTemp, "%d/%d/%d %02d:%02d:%02d", 1900+m_Year, m_Month+1, m_Day, m_Hour, m_Min, m_Sec );
	return( pTemp );
}

void CRealTime::FillRealTime()	// fill in with real time
{
	time_t SysTime;
	time( & SysTime );
	struct tm * pTime = localtime(& SysTime);
	ASSERT(pTime);

	m_Year = pTime->tm_year;	// years since 1900
	m_Month = pTime->tm_mon;	// months since January - [0,11]
	m_Day = pTime->tm_mday;		// day of the month - [1,31]
	m_Hour = pTime->tm_hour;	// hours since midnight - [0,23]
	m_Min = pTime->tm_min;		// minutes after the hour - [0,59]
	m_Sec = pTime->tm_sec;		// seconds after the minute - [0,59]
}

//////////////////////////////////////////////////////////////////////
// -CAccount

int CAccount::NameStrip( TCHAR * pszNameOut, const TCHAR * pszNameInp ) // static
{
	// allow just basic chars. No spaces, only numbers, letters and underbar.
	int len = GetBareText( pszNameOut, pszNameInp, MAX_ACCOUNT_NAME_SIZE, " !\"#$%&()*,/:;<=>?@[\\]^{|}~" ); // "'`-.+" // seems people are using these.
	if ( g_Serv.IsObscene( pszNameOut ))
	{
		DEBUG_ERR(( "Obscene account '%s' ignored.\n", pszNameInp ));
		return( 0 );
	}
	return len;
}

PLEVEL_TYPE CAccount::GetPrivLevelText( const TCHAR * pszFlags ) // static
{
	if ( ! strnicmp( pszFlags, "GM", 2 ))
		return PLEVEL_GM;
	else if ( ! strnicmp( pszFlags, "CO", 2 ))
		return PLEVEL_Counsel;

	PLEVEL_TYPE level = (PLEVEL_TYPE) Exp_GetVal( pszFlags );
	if ( level >= PLEVEL_QTY ) 
		level = PLEVEL_Owner;
	return( level );
}

CAccount::CAccount( const TCHAR * pszName, bool fGuest )
{
	// Make sure the name is in valid format.
	// Assume the pszName has been stripped of all just !

	m_sName = pszName;

	ASSERT( ! m_sName.IsEmpty());
	ASSERT( ! ISWHITESPACE( m_sName[0] ));

	if ( ! strnicmp( m_sName, "GUEST", 5 ) || fGuest )
	{
		SetPrivLevel( PLEVEL_Guest );
	}
	else
	{
		SetPrivLevel( PLEVEL_Player );
	}

	m_lang[0] = 0;
	m_PrivFlags = PRIV_DETAIL;	// Set on by default for everyone.
	m_Total_Connect_Time = 0;
	m_First_IP.s_addr = 0;
	m_Last_IP.s_addr = 0;	// last ip i logged in from.
	m_Last_Connect_Time = 0;
	m_iEmailFailures = 0;

	// Add myself to the list.
	g_World.m_Accounts.AddSortKey(this,pszName);
}

CAccount::~CAccount()
{
	// We should go track down and delete all the chars and clients that use this account !

	CClient * pClient = FindClient();
	if ( pClient )
	{
		delete pClient;	// we have no choice but to kick them.
	}

	// Now track down all my disconnected chars !
	// ? CAccountMailSched
	if ( ! g_Serv.IsLoading())
	{
		for ( int k=0; k<SECTOR_QTY; k++ )
		{
			CChar * pCharNext;
			CChar * pChar = STATIC_CAST <CChar*> ( g_World.m_Sectors[k].m_Chars_Disconnect.GetHead());
			for ( ; pChar != NULL; pChar = pCharNext )
			{
				pCharNext = pChar->GetNext();
				if ( ! IsMyAccountChar( pChar ))
					continue;
				pChar->Delete();
			}
		}
	}
}

CClient * CAccount::FindClient( const CClient * pExclude ) const
{
	// Is the account logged in.
	if ( this == NULL )
		return( NULL );	// this might be possible.
	CClient * pClient = g_Serv.GetClientHead();
	for ( ; pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pExclude )
			continue;
		if ( this == pClient->GetAccount())
			break;
	}
	return( pClient );
}

bool CAccount::IsMyAccountChar( const CChar * pChar ) const
{
	// this char is mine ?
	if ( pChar == NULL )
		return( false );
	if ( pChar->m_pPlayer == NULL )
		return( false );
	return(	pChar->m_pPlayer->GetAccount() == this );
}

void CAccount::UnlinkChar( CChar * pChar )
{
	// unlink the CChar from this CAccount.
	ASSERT( pChar );
	if ( m_uidLastChar == pChar->GetUID())
	{
		m_uidLastChar.ClearUID();
	}
}

void CAccount::LinkChar( CChar * pChar )
{
	// link the char to this account.
	ASSERT( pChar );
}

void CAccount::CheckStart()
{
	if ( ! m_sName.CompareNoCase( "Administrator" ))	// This account always has privilege
	{
		SetPrivLevel( PLEVEL_Admin );
	}
	if ( GetPrivLevel() >= PLEVEL_Counsel )	// ON by default.
	{
		m_PrivFlags |= PRIV_GM_PAGE;
	}
}

bool CAccount::CheckBlockedEmail( const TCHAR * pszEmail) // static
{
	CScript s;
	if ( ! s.OpenFind( GRAY_FILE ".ini" )) // Open script file
	{
		g_Log.Event( LOGL_ERROR, "Can't open " GRAY_FILE ".ini for [BLOCKEMAIL]" );
		return( false );
	}

	if ( ! s.FindSection( _TEXT("BLOCKEMAIL")))
	{
		// g_Log.Event( LOGL_ERROR, "Can't find [BLOCKEMAIL] section in " GRAY_FILE ".ini.\n" );
		return( false );
	}

	int len1 = strlen( pszEmail );
	// Read in and compare the sections.
	while ( s.ReadKey())
	{
		int len2 = strlen( s.GetKey());
		if ( len2 < len1 ) 
			continue;
		if ( ! strcmpi( pszEmail + len1 - len2, s.GetKey()))
		{
			// Bad email host detected.
			return( false );
		}
	}
	return( true );
}

bool CAccount::SetEmailAddress( const TCHAR * pszEmail, bool fBlockHost )
{
	if ( pszEmail == NULL ) 
		return( false );
	GETNONWHITESPACE( pszEmail );	// Skip leading spaces.
	if ( ! g_Serv.IsValidEmailAddressFormat( pszEmail ))
		return( false );

	// Make sure it is a valid email server !
	if ( fBlockHost && CheckBlockedEmail(pszEmail))
	{
		return( false );
	}

	m_sEMail = pszEmail;
	m_iEmailFailures = 0;
	ClearPrivFlags( PRIV_EMAIL_VALID );	// we have not tried it yet.
	return( true );
}

bool CAccount::ScheduleEmailMessage( WORD iEmailMessage )
{
	// ??? Note this might have some thread conflict issues.
	// Queued in forground thread. Dequeued in background thread.
	if ( m_sEMail.IsEmpty())
		return( false );
	if ( m_iEmailFailures > 5 )
		return( false );

	// Make sure this msg is not already scheduled.
	for ( int i=0; i<m_EMailSchedule.GetCount(); i++ )
	{
		if ( m_EMailSchedule[i] == iEmailMessage )
			return( false );
	}

	m_EMailSchedule.Add(iEmailMessage);
	return( true );
}

const TCHAR * CAccount::sm_KeyTable[] = // static
{
	"ACCOUNT",
	"BLOCK",
	"CHATNAME",
	"COMMENT",
	"EMAIL",
	"EMAILFAIL",
	"EMAILMSG",
	"FIRSTCONNECTDATE",
	"FIRSTIP",
	"GUEST",
	"JAIL",
	"LANG",
	"LASTCHARUID",
	"LASTCONNECTDATE",
	"LASTCONNECTTIME",
	"LASTIP",
	"LEVEL",
	"PASSWORD",
	"PLEVEL",
	"PRIV",
	"T2A",
	"TOTALCONNECTTIME",
};

bool CAccount::r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc )
{
	if ( ! pSrc ||
		pSrc->GetPrivLevel() < PLEVEL_GM )
	{
		return( false );
	}

	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0: // "ACCOUNT"
		sVal = m_sName;
		break;
	case 1:	// "BLOCK"
		sVal.FormatVal( IsPriv( PRIV_BLOCKED ));
		break;
	case 2: // "CHATNAME"
		sVal = m_sChatName;
		break;
	case 3: // "COMMENT",
		sVal = m_sComment;
		break;
	case 4: // "EMAIL",
		sVal = m_sEMail;
		break;
	case 5: // "EMAILFAIL",
		sVal.FormatVal( m_iEmailFailures );
		break;
	case 6: // "EMAILMSG",
		sVal.FormatVal( (m_EMailSchedule.GetCount()) ? (m_EMailSchedule[0]) : 0 );
		break;
	case 7: //	"FIRSTCONNECTDATE",
		sVal = m_First_Connect_Date.Write();
		break;
	case 8: //	"FIRSTIP",
		sVal = inet_ntoa(m_First_IP);
		break;
	case 9: // "GUEST"
		sVal.FormatVal( GetPrivLevel() == PLEVEL_Guest );
		break;
	case 10: // "JAIL"
		sVal.FormatVal( IsPriv( PRIV_JAILED ));
		break;
	case 11: // "LANG"
		sVal = m_lang;
		break;
	case 12: // "LASTCHARUID"
		sVal.FormatHex( m_uidLastChar );
		break;
	case 13: // "LASTCONNECTDATE",
		sVal = m_Last_Connect_Date.Write();
		break;
	case 14: // "LASTCONNECTTIME",
		sVal.FormatVal( m_Last_Connect_Time );
		break;
	case 15: // "LASTIP",
		sVal = inet_ntoa(m_Last_IP);
		break;
	case 16: // "LEVEL"
plevel:
		sVal.FormatVal( m_PrivLevel );
		break;
	case 17: // "PASSWORD",
		if ( pSrc->GetPrivLevel() < PLEVEL_Admin )
		{
			return( false );	
		}
		sVal = GetPassword();
		break;
	case 18: // "PLEVEL"
		goto plevel;
	case 19: // "PRIV"
		sVal.FormatHex( m_PrivFlags );
		break;
	case 20: //	"T2A",
		sVal.FormatVal( IsPriv( PRIV_T2A ));
		break;
	case 21: // "TOTALCONNECTTIME",
		sVal.FormatVal( m_Total_Connect_Time );
		break;

	default:
		return( CScriptObj::r_WriteVal( pKey, sVal, pSrc ));
	}
	return( true );
}

bool CAccount::r_LoadVal( CScript & s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// "ACCOUNT" // can't be set this way.
		return( false );
	case 1: // "BLOCK"
		if ( ! s.HasArgs() || s.GetArgVal())
		{
			SetPrivFlags( PRIV_BLOCKED );
		}
		else
		{
			ClearPrivFlags( PRIV_BLOCKED );
		}
		break;
	case 2: // "CHATNAME",
		m_sChatName = s.GetArgStr();
		break;
	case 3: // "COMMENT",
		m_sComment = s.GetArgStr();
		break;
	case 4: // "EMAIL",
		m_sEMail = s.GetArgStr();
		break;
	case 5: // "EMAILFAIL",
		m_iEmailFailures = s.GetArgVal();
		break;
	case 6: //	"EMAILMSG",
		ScheduleEmailMessage( s.GetArgVal());
		break;
	case 7: //	"FIRSTCONNECTDATE",
		m_First_Connect_Date.Read( s.GetArgStr());
		break;
	case 8: //	"FIRSTIP",
		m_First_IP.s_addr = inet_addr( s.GetArgStr());
		break;
	case 9: // "GUEST"
		if ( ! s.HasArgs() || s.GetArgVal())
		{
			SetPrivLevel( PLEVEL_Guest );
		}
		else
		{
			if ( GetPrivLevel() == PLEVEL_Guest ) SetPrivLevel( PLEVEL_Player );
		}
		break;
	case 10: // "JAIL"
		if ( ! s.HasArgs() || s.GetArgVal())
		{
			SetPrivFlags( PRIV_JAILED );
		}
		else
		{
			ClearPrivFlags( PRIV_JAILED );
		}
		break;
	case 11:	// "LANG"
		strncpy( m_lang, s.GetArgStr(), sizeof( m_lang ));
		if ( ! isalnum(m_lang[0]))
			m_lang[0] = '\0';
		else
			m_lang[sizeof(m_lang)-1] = '\0';
		break;
	case 12: // "LASTCHARUID"
		m_uidLastChar = s.GetArgHex();
		break;
	case 13: // "LASTCONNECTDATE",
		m_Last_Connect_Date.Read( s.GetArgStr());
		break;
	case 14:	// "LASTCONNECTTIME"
		// Previous total amount of time in game
		m_Last_Connect_Time = s.GetArgVal();
		break;
	case 15: // "LASTIP",
		m_Last_IP.s_addr = inet_addr( s.GetArgStr());
		break;
	case 16: // "LEVEL"
plevel:
		SetPrivLevel( GetPrivLevelText( s.GetArgStr()));
		break;
	case 17:	// "PASSWORD"
		SetPassword( s.GetArgStr());
		break;
	case 18: // "PLEVEL"
		goto plevel;
	case 19: // "PRIV"
		// Priv flags.
		m_PrivFlags = s.GetArgHex();
		break;
	case 20: //	"T2A",
		if ( ! s.HasArgs() || s.GetArgVal())
		{
			SetPrivFlags( PRIV_T2A );
		}
		else
		{
			ClearPrivFlags( PRIV_T2A );
		}
		break;
	case 21:	// "TOTALCONNECTTIME"
		// Previous total amount of time in game
		m_Total_Connect_Time = s.GetArgVal();
		break;
	default:
		return( false );
	}
	return( true );
}

void CAccount::r_Write( CScript & s )
{
	if ( GetPrivLevel() >= PLEVEL_QTY ) 
		return;

	s.WriteSection( m_sName );
	if ( GetPrivLevel() != PLEVEL_Player )
	{
		s.WriteKeyVal( "PLEVEL", GetPrivLevel());
	}
	if ( m_PrivFlags != PRIV_DETAIL )
	{
		s.WriteKeyHex( "PRIV", m_PrivFlags &~( PRIV_BLOCKED | PRIV_JAILED ));
	}
	if ( IsPriv( PRIV_JAILED ))
	{
		s.WriteKeyVal( "JAIL", 1 );
	}
	if ( IsPriv( PRIV_BLOCKED ))
	{
		s.WriteKeyVal( "BLOCK", 1 );
	}
	if ( ! m_sPassword.IsEmpty())
	{
		s.WriteKey( "PASSWORD", GetPassword() );
	}
	if ( ! m_sComment.IsEmpty())	// Some GM comments about me.
	{
		s.WriteKey( "COMMENT", m_sComment );
	}
	if ( m_Total_Connect_Time )
	{
		s.WriteKeyVal( "TOTALCONNECTTIME", m_Total_Connect_Time );
	}
	if ( m_Last_Connect_Time )
	{
		s.WriteKeyVal( "LASTCONNECTTIME", m_Last_Connect_Time );
	}
	if ( m_uidLastChar.IsValidUID())
	{
		s.WriteKeyHex( "LASTCHARUID", m_uidLastChar );
	}
	if ( ! m_sEMail.IsEmpty())
	{
		s.WriteKey( "EMAIL", m_sEMail );
	}
	if ( m_iEmailFailures )
	{
		s.WriteKeyVal( "EMAILFAIL", m_iEmailFailures );
	}
	// write out any scheduled email message s.
	if ( m_EMailSchedule.GetCount())
	{
		for ( int i=0; i<m_EMailSchedule.GetCount(); i++ )
		{
			s.WriteKeyVal( "EMAILMSG", m_EMailSchedule[i] );
		}
	}
	if ( m_First_Connect_Date.IsValid())
	{
		s.WriteKey( "FIRSTCONNECTDATE", m_First_Connect_Date.Write());
	}
	if ( m_First_IP.s_addr )
	{
		s.WriteKey( "FIRSTIP", inet_ntoa(m_First_IP));
	}

	if ( m_Last_Connect_Date.IsValid())
	{
		s.WriteKey( "LASTCONNECTDATE", m_Last_Connect_Date.Write());
	}
	if ( m_Last_IP.s_addr )
	{
		s.WriteKey( "LASTIP", inet_ntoa(m_Last_IP));
	}
	if ( ! m_sChatName.IsEmpty())
	{
		s.WriteKey( "CHATNAME", m_sChatName );
	}
	if ( m_lang[0] )
	{
		s.WriteKey( "LANG", m_lang );
	}
}

bool CAccount::r_Verb( CScript &s, CTextConsole * pSrc )
{
	if ( ! pSrc ||
		pSrc->GetPrivLevel() < PLEVEL_Admin )
	{
		return( false );
	}
	if ( s.IsKeyHead( "SUMMON", 6 ))
	{
		// move all the offline chars here ?
	}

	static const TCHAR * pszKeyVerbs[] = 
	{
		"DELETE",
		"EMAIL",
		"EMAILMSG",
	};

	switch ( FindTableSorted( s.GetKey(), pszKeyVerbs, COUNTOF( pszKeyVerbs )))
	{
	case 0: // "DELETE"
		// delete the account and all it's chars
		if ( pSrc->GetChar())
		{
			CClient * pClient = pSrc->GetChar()->GetClient();
			if ( pClient == NULL )	// NPC's have no business doing this.
				return( false );
			if ( pClient->GetAccount() == this ) // deleting myself at this point is bad.
				return( false );
		}
		g_World.m_Accounts.DeleteOb( this );
		return( true );

	case 1: // "EMAIL"
		// Enter the email address , but filtered.
		if ( s.GetArgStr()[0] )
		{
			if ( ! SetEmailAddress( s.GetArgStr(), true ))
			{
				pSrc->SysMessagef( "Email address '%s' is not allowed.", s.GetArgStr());
				return false;
			}
		}
		pSrc->SysMessagef( "Email for '%s' is '%s'.",
			GetName(), (const TCHAR*) m_sEMail );
		return( true );

	case 2:	// "EMAILMSG"
		// Schedule one of the email messages to be delivered.
		if ( ! ScheduleEmailMessage( s.GetArgVal()))
		{
			return( false );
		}
		return( true );
	}

	return( CScriptObj::r_Verb( s, pSrc ));
}

bool CAccount::SendOutgoingMail( int iMsg )
{
	CGString sSec;
	sSec.Format( "EMAILMSG %d", iMsg );

	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_BOOK_2, sSec ) == NULL )
	{
		// This is an odd problem.
		// we should just move on to the next msg if any ?
		return false;
	}

	CGString pStrings[512];

	int lines=0;
	while (s.ReadKey(false))
	{
		// Parse out any variables in it. (may act like a verb sometimes)
		ParseText( s.GetArgStr(), &g_Serv );
		if ( lines >= COUNTOF(pStrings)-1 ) 
			break;
		pStrings[lines++] = s.GetArgStr();
	}

	CMailSMTP smtp;
	if ( ! smtp.SendMail( m_sEMail, g_Serv.m_sEMail, pStrings, lines ))
	{
		DEBUG_ERR(( "Failed to send mail to '%s', reason '%s'\n", GetName(), smtp.GetLastResponse() ));
		return( false );
	}
	
	return( true );
}

void CAccount::SendAllOutgoingMail()
{
	// NOTE: This must run on background thread.

	while ( m_EMailSchedule.GetCount())
	{
		if ( m_iEmailFailures > 5 )
		{
			return;
		}
		if ( g_Serv.m_fResyncPause )	// don't do this now. multi thread problems.
			return;
		int iMsg = m_EMailSchedule[0];
		if ( ! SendOutgoingMail( iMsg ))
		{
			m_iEmailFailures ++;
			return;	// try again later.
		}

		// we are done with this msg.
		m_EMailSchedule.RemoveAt(0);
		m_iEmailFailures = 0;
	}
}

