//
// CClient.cpp
// Copyright Menace Software (www.menasoft.com).
//
#ifdef VISUAL_SPHERE
	#include "..\Visual Sphere\stdafx.h"
	#include "..\Visual Sphere\Visual Sphere.h"
	#include "..\Visual Sphere\ServerObject.h"
#else
	#include "graysvr.h"	// predef header.
#endif

/////////////////////////////////////////////////////////////////
// -CClient stuff.

CClient::CClient( SOCKET client ) : CGSocket( client )
{
	m_pChar = NULL;
	m_pAccount = NULL;
	m_Crypt.SetClientVersion( g_Serv.m_ClientVersion.GetClientVersion());

	m_fGameServer = false;	// act like a login server first.
	m_pGMPage = NULL;

	m_Time_Login = 0;
	m_Time_LastEvent = g_World.GetTime();

	m_bin_PrvMsg = XCMD_QTY;
	m_bin_len = 0;
	m_bout_len = 0;

	m_WalkCount = -1;
	m_fPaused = false;

	m_Targ_Mode = TARGMODE_SETUP_CONNECT;
	m_Targ_PrvMode = TARGMODE_NONE;
	m_tmSetupConnect = 0;

	memset( m_Walk_LIFO, 0, sizeof( m_Walk_LIFO ));	// Initialize the fast walk killer stuff
	m_Walk_InvalidEchos = 0;
	m_Walk_CodeQty = -1;	// how many valid codes in here ?

	g_Serv.ClientsInc();
	g_Serv.m_Clients.InsertAfter( this );

	struct sockaddr_in Name;
	GetPeerName( &Name );

	if ( Name.sin_addr.s_addr != g_Serv.m_BackTask.m_RegisterIP.s_addr )
	{
		g_Log.Event( LOGM_CLIENTS_LOG, "%x:Client connected [Total:%i] from '%s'.\n",
			GetSocket(), g_Serv.StatGet( SERV_STAT_CLIENTS ), inet_ntoa( Name.sin_addr ));
	}

	DEBUG_CHECK( g_Serv.StatGet( SERV_STAT_CLIENTS ) == g_Serv.m_Clients.GetCount());

	// disable NAGLE algorythm for data compression/coalescing. Send as fast as we can. we handle packing ourselves.
	BOOL nbool=TRUE;
	SetSockOpt( TCP_NODELAY, &nbool, sizeof(BOOL), IPPROTO_TCP );
}

bool CClient::CanInstantLogOut() const
{
	if ( ! g_Serv.m_iClientLingerTime )
		return true;
	if ( IsPriv( PRIV_GM ))
		return true;
	if ( m_pChar == NULL )
		return( true );
	if ( m_pChar->IsStat(STATF_DEAD) || m_pChar->m_pArea == NULL )
		return( true );
	if ( m_pChar->m_pArea->IsFlag( REGION_FLAG_INSTA_LOGOUT ))
		return( true );

	// Camp near by ? have we been standing near it long enough ?
	if ( m_pChar->m_pArea->IsGuarded() && ! m_pChar->IsStat( STATF_Criminal ))
		return( true );
	return( false );
}

void CClient::CharDisconnect()
{
	// Disconnect the char from the client.
	// Even tho the client might stay active.
	if ( m_pChar == NULL ) 
		return;

	Announce( false );
	m_pChar->CancelAllTrades();

	// log out immediately ?
	if ( ! CanInstantLogOut())
	{
		// become an NPC for a little while
		m_pChar->ClientDetach();	// we are not a client any more.
		CItem * pItemChange = CItem::CreateBase( ITEMID_MEMORY );
		pItemChange->m_type = ITEM_EQ_CLIENT_LINGER;
		pItemChange->SetTimeout( g_Serv.m_iClientLingerTime );
		m_pChar->LayerAdd( pItemChange, LAYER_FLAG_ClientLinger );
		return;
	}

	// remove me from other clients screens now.
	m_pChar->ClientDetach();	// we are not a client any more.
	m_pChar->SetDisconnected();
}

CClient::~CClient()
{
	g_Serv.StatDec( SERV_STAT_CLIENTS );

	struct sockaddr_in Name;
	GetPeerName( &Name );
	if ( Name.sin_addr.s_addr != g_Serv.m_BackTask.m_RegisterIP.s_addr )
	{
		g_Log.Event( LOGM_CLIENTS_LOG, "%x:Client disconnected [Total:%i]\n", GetSocket(), g_Serv.StatGet(SERV_STAT_CLIENTS));
	}
#ifdef VISUAL_SPHERE
	g_pServerObject->Fire_ClientDetach(GetSocket());
#endif
	if ( GetTargMode() == TARGMODE_CONSOLE )
	{
		// unlink the admin client.
		g_Serv.m_iAdminClients --;
	}

	CharDisconnect();	// am i a char in game ?
	ClearGMHandle();

	if ( m_pAccount )
	{
		if ( GetPrivLevel() <= PLEVEL_Guest )
		{
			g_Serv.m_nGuestsCur--;	// Free up a guest slot.
		}
		if ( GetPrivLevel() >= PLEVEL_QTY ) // kill the temp account.
		{
			CAccount * pAccount = m_pAccount;
			m_pAccount = NULL;		// unattach first.
			g_World.m_Accounts.DeleteOb( pAccount );
		}
	}

	xFlush();
}

void CClient::SysMessage( const TCHAR * pMsg ) const // System message (In lower left corner)
{
	if ( IsConsole())
	{
		if ( *pMsg != '\0' )
		while (true)
		{
			if ( *pMsg == '\n' )
			{
				((CClient*)this)->xSendReady( "\r", 1 );
			}
			((CClient*)this)->xSendReady( pMsg, 1 );
			if ( *pMsg == '\0' )
				break;
			pMsg++;
		}
	}
	else
	{
		((CClient*)this)->addSysMessage( pMsg );
	}
}

int CClient::Send( const void * pData, int len )
{
	// overload these virtuals for stats purposes.
	g_Serv.m_Profile.Count( PROFILE_DATA_TX, len );
	return( CGSocket::Send( pData, len ));
}

int CClient::Receive( void * pData, int len, int flags )
{
	// overload these virtuals for stats purposes.
	int iLen = CGSocket::Receive( pData, len, flags );
	if ( iLen > 0 )
	{
		g_Serv.m_Profile.Count( PROFILE_DATA_RX, iLen );
	}
	return( iLen );
}

void CClient::Announce( bool fArrive ) const
{
	ASSERT( m_pChar != NULL );
	ASSERT( m_pChar->m_pPlayer != NULL );
	if ( m_pAccount == NULL || m_pChar == NULL || m_pChar->m_pPlayer == NULL ) 
		return;

	// We have logged in or disconnected.
	// Annouce my arrival or departure.
	if ( g_Serv.m_fArriveDepartMsg )
	{
		CGString sMsg;
		for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
		{
			if ( pClient == this ) 
				continue;
			CChar * pChar = pClient->GetChar();
			if ( pChar == NULL )
				continue;
			if ( GetPrivLevel() > pClient->GetPrivLevel())
				continue;
			if ( ! pClient->IsPriv( PRIV_DETAIL|PRIV_HEARALL ))
				continue;
			if ( sMsg.IsEmpty())
			{
				sMsg.Format( _TEXT( "%s has %s %s." ),
					m_pChar->GetName(),
					(fArrive)? _TEXT("arrived in") : _TEXT("departed from"),
					m_pChar->GetTopPoint().GetRegion( REGION_TYPE_AREA )->GetName());
			}
			pClient->SysMessage( sMsg );
		}
	}

	// re-Start murder decay if arriving

	if ( m_pChar->m_pPlayer->m_Murders )
	{
		CItem * pMurders = m_pChar->LayerFind(LAYER_FLAG_Murders);
		if ( pMurders )
		{
			if ( fArrive )
			{
				// If the Memory exists, put it in the loop
				pMurders->SetTimeout( pMurders->m_itEqMurderCount.m_Decay_Balance );
			}
			else
			{
				// Or save decay point if departing and remove from the loop
				pMurders->m_itEqMurderCount.m_Decay_Balance = pMurders->GetTimerAdjusted();
				pMurders->SetTimeout(-1); // turn off the timer til we log in again.
			}
		}
		else if ( fArrive )
		{
			// If not, see if we need one made
			m_pChar->Noto_Murder();
		}
	}

	if ( fArrive )
	{
		// On Connect
		struct sockaddr_in Name;
		GetPeerName( &Name );

		// Get the real world time/date.
		CRealTime thetime;
		thetime.FillRealTime();

		if ( ! m_pAccount->m_Total_Connect_Time )
		{
			// FIRSTIP= first IP used.
			m_pAccount->m_First_IP = Name.sin_addr;
			// FIRSTCONNECTDATE= date/ time of the first connect.
			m_pAccount->m_First_Connect_Date = thetime;
		}

		// LASTIP= last IP used.
		m_pAccount->m_Last_IP = Name.sin_addr;
		// LASTCONNECTDATE= date / time of this connect.
		m_pAccount->m_Last_Connect_Date = thetime;
	}
	else
	{
		// On Disconnect.
		// LASTCONNECTTIME= How long where you connected last time.
		m_pAccount->m_Last_Connect_Time = ( g_World.GetTime() - m_Time_Login ) / ( TICK_PER_SEC * 60 );
		if ( m_pAccount->m_Last_Connect_Time <= 0 ) m_pAccount->m_Last_Connect_Time = 1;
		// TOTALCONNECTTIME= qty of minutes connected total.
		m_pAccount->m_Total_Connect_Time += m_pAccount->m_Last_Connect_Time;
	}

	if ( m_pChar != NULL )
	{
		m_pAccount->m_uidLastChar = m_pChar->GetUID();
	}
}

////////////////////////////////////////////////////

bool CClient::CanSee( const CObjBaseTemplate * pObj ) const
{
	// Can player see item b
	if ( m_pChar == NULL ) 
		return( false );
	return( m_pChar->CanSee( pObj ));
}

bool CClient::Check_Snoop_Container( const CItemContainer * pItem )
{
	// Assume the container is not locked.
	// return: true = snoop or can't open at all.
	//  false = instant open.

	if ( pItem == NULL ) 
		return( true );

	ASSERT( pItem->IsItem());

	if ( ! IsPriv(PRIV_GM))
	switch ( pItem->GetID())
	{
	case ITEMID_SHIP_HATCH_E:
	case ITEMID_SHIP_HATCH_W:
	case ITEMID_SHIP_HATCH_N:
	case ITEMID_SHIP_HATCH_S:
		// Must be on board a ship to open the hatch.
		if ( ! m_pChar->m_pArea->IsMatchType( pItem->m_itContainer.m_lockUID ))
		{
			SysMessage( _TEXT("You can only open the hatch on board the ship"));
			return( true );
		}
		break;
	case ITEMID_BANK_BOX:
		// Some sort of cheater.
		return( false );
	}

	CChar * pCharMark;
	if ( ! m_pChar->IsTakeCrime( pItem, &pCharMark ) || pCharMark == NULL )
		return( false );

	DEBUG_CHECK( ! IsPriv(PRIV_GM));
	if ( m_pChar->Skill_Wait())
		return( true );

	m_pChar->m_Act_Targ = pItem->GetUID();
	m_pChar->Skill_Start( SKILL_SNOOPING );
	return( true );
}

bool CClient::UseInterWorldGate( const CItem * pItemGate )
{
	// Try to connect to the server named (pItem->GetName()).
	// ITEM_DREAM_GATE
#if 0
	// Look for a server by name ?


#endif

	// Same server.
	if ( ! pItemGate->m_itDreamGate.m_Server_Index )
		return false;

	// Instant log off.

	return( Login_Relay( pItemGate->m_itDreamGate.m_Server_Index )); // Relay player to a selected IP
}


/////////////////////////////////////////////

void CClient::Cmd_GM_Page( const TCHAR * pszReason ) // Help button (Calls GM Call Menus up)
{
	// Player pressed the help button.
	// m_Targ_Text = menu desc.
	// TARGMODE_GM_PAGE_TEXT

	if ( pszReason[0] == '\0' )
	{
		SysMessage( "Game Master Page Cancelled.");
		return;
	}

	CGString sMsg;
	sMsg.Format( "GM Page from %s [%lx]: %s\n",
		m_pChar->GetName(), m_pChar->GetUID(), pszReason );
	g_Log.Event( LOGM_GM_PAGE, sMsg );

	bool fFound=false;
	for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient->IsPriv( PRIV_GM_PAGE )) // found GM
		{
			fFound=true;
			pClient->SysMessage(sMsg);
		}
	}
	if ( ! fFound)
	{
		SysMessage( "There is no Game Master available to take your call. Your message has been queued.");
	}
	else
	{
		SysMessage( "Available Game Masters have been notified of your request.");
	}

	sMsg.Format( "There are %d messages queued ahead of you", g_World.m_GMPages.GetCount());
	SysMessage( sMsg );

	// Already have a message in the queue ?
	// Find an existing GM page for this account.
	CGMPage * pPage = dynamic_cast <CGMPage*>( g_World.m_GMPages.GetHead());
	for ( ; pPage!= NULL; pPage = pPage->GetNext())
	{
		if ( ! strcmpi( pPage->GetName(), m_pAccount->GetName()))
			break;
	}
	if ( pPage != NULL )
	{
		SysMessage( "You have an existing page. It has been updated." );
		pPage->SetReason( pszReason );
		pPage->m_lTime = g_World.GetTime();
	}
	else
	{
		// Queue a GM page. (based on account)
		pPage = new CGMPage( m_pAccount->GetName());
		pPage->SetReason( pszReason );	// Description of reason for call.
	}
	pPage->m_p = m_pChar->GetTopPoint();		// Origin Point of call.
}

void CClient::ClearGMHandle()
{
	if ( m_pGMPage )
	{
		m_pGMPage->ClearGMHandler();
		m_pGMPage = NULL;
	}
}

void CClient::Cmd_GM_PageMenu( int iEntryStart )
{
	// Just put up the GM page menu.
	SetPrivFlags( PRIV_GM_PAGE );
	ClearGMHandle();

	CMenuItem item[10];	// only display x at a time.
	ASSERT( COUNTOF(item)<=COUNTOF(m_tmMenu));

	item[0].m_text = "GM Page Menu";
	item[0].m_id = TARGMODE_MENU_GM_PAGES;

	int entry=0;
	int count=0;
	CGMPage * pPage = dynamic_cast <CGMPage*>( g_World.m_GMPages.GetHead());
	for ( ; pPage!= NULL; pPage = pPage->GetNext(), entry++ )
	{
		if ( entry < iEntryStart )
			continue;

		CClient * pGM = pPage->FindGMHandler();	// being handled ?
		if ( pGM != NULL ) 
			continue;

		if ( ++count >= COUNTOF( item )-1 )
		{
			// Add the "MORE" option if there is more than 1 more.
			if ( pPage->GetNext() != NULL )
			{
				item[count].m_id = count-1;
				item[count].m_text.Format( "MORE" );
				m_tmMenu[count] = 0xFFFF0000 | entry;
				break;
			}
		}

		CClient * pClient = pPage->FindAccount()->FindClient();	// logged in ?

		item[count].m_id = count-1;
		item[count].m_text.Format( "%s %s %s",
			pPage->GetName(),
			(pClient==NULL) ? "OFF":"ON ",
			pPage->GetReason());
		m_tmMenu[count] = entry;
	}

	if ( ! count )
	{
		SysMessage( "No GM pages queued" );
		return;
	}
	addItemMenu( item, count );
}

void CClient::Cmd_GM_PageInfo()
{
	// Show the current page.
	// This should be a dialog !!!??? book or scroll.
	ASSERT( m_pGMPage );
	const TCHAR *pszName;
	CClient * pClient = m_pGMPage->FindAccount()->FindClient();
	if ( pClient==NULL )
		pszName = "OFFLINE";
	else if ( pClient->GetChar() == NULL )
		pszName = "LOGIN";
	else
		pszName = pClient->GetChar()->GetName();
	SysMessagef(
		"Current GM /PAGE Account=%s (%s) "
		"Reason='%s' "
		"Time=%ld",
		m_pGMPage->GetName(),
		pszName,
		m_pGMPage->GetReason(),
		m_pGMPage->m_lTime );
}

void CClient::Cmd_GM_PageCmd( const TCHAR * pCmd )
{
	// A GM page command.
	// Put up a list of GM pages pending.

	if ( pCmd == NULL || pCmd[0] == '?' )
	{
		static const TCHAR * pCmds[] =
		{
			"/PAGE on/off\n",
			"/PAGE list = list of pages.\n",
			"/PAGE delete = dispose of this page. Assume it has been handled.\n",
			"/PAGE origin = go to the origin point of the page\n",
			"/PAGE undo/queue = put back in the queue\n",
			"/PAGE current = info on the current selected page.\n",
			"/PAGE go/player = go to the player that made the page. (if they are logged in)\n",
			"/PAGE jail\n",
			"/PAGE ban/kick\n",
			"/PAGE wipe (gm only)"
		};
		// addScroll( pCmds );

		for ( int i=0; i<COUNTOF(pCmds); i++ )
		{
			SysMessage( pCmds[i] );
		}
		return;
	}
	if ( ! strcmpi( pCmd, "ON" ))
	{
		SetPrivFlags( PRIV_GM_PAGE );
		SysMessage( "GM pages on" );
		return;
	}
	if ( ! strcmpi( pCmd, "OFF" ))
	{
		if ( GetPrivLevel() < PLEVEL_Counsel ) return;	// cant turn off.
		ClearPrivFlags( PRIV_GM_PAGE );
		ClearGMHandle();
		SysMessage( "GM pages off" );
		return;
	}
	if ( ! strcmpi( pCmd, "WIPE" ))
	{
		if ( ! IsPriv( PRIV_GM )) return;
		g_World.m_GMPages.DeleteAll();
		return;
	}
	TCHAR bCmd = toupper(*pCmd);
	switch ( bCmd )
	{
	case '?':	// help ?
	case 'H':	// help ?
	case ' ':
		Cmd_GM_PageCmd(NULL);
		return;
	}
	if ( m_pGMPage == NULL )
	{
		Cmd_GM_PageMenu();
		return;
	}
	// We must have a select page for these commands.
	switch ( bCmd )
	{
	case 'L':	// List
		Cmd_GM_PageMenu();
		return;
	case 'D':
		// /PAGE delete = dispose of this page. We assume it has been handled.
		SysMessage( "GM Page deleted" );
		delete m_pGMPage;
		ASSERT( m_pGMPage == NULL );
		return;
	case 'O':
		// /PAGE origin = go to the origin point of the page
		m_pChar->Spell_Teleport( m_pGMPage->m_p, true, false );
		return;
	case 'U':
	case 'Q':
		// /PAGE queue = put back in the queue
		SysMessage( "GM Page re-queued." );
		ClearGMHandle();
		return;
	case 'G':	// /PAGE go
	case 'P':	// /PAGE player = go to the player that made the page. (if they are logged in)
	case 'J':	// /PAGE jail
		break;
	case 'B':
	case 'K':	// /PAGE ban or kick = locks the account of this person and kicks
		{
			CAccount * pAccount = m_pGMPage->FindAccount();
			if ( pAccount )
			{
				pAccount->r_SetVal( "BLOCK", "1" );
				SysMessage( "Account blocked" );
			}
			else
			{
				SysMessage( "Invalid account for page !?" );
			}
		}
		break;
	case 'C':
	default:
		// What am i servicing ?
		Cmd_GM_PageInfo();
		return;
	}

	// Manipulate the character only if logged in.

	CClient * pClient = m_pGMPage->FindAccount()->FindClient();
	if ( pClient == NULL || pClient->GetChar() == NULL )
	{
		SysMessage( "The account is not logged in." );
		if ( bCmd == 'P' )
		{
			m_pChar->Spell_Teleport( m_pGMPage->m_p, true, false );
		}
		return;
	}

	switch ( bCmd )
	{
	case 'G':
	case 'P': // /PAGE player = go to the player that made the page. (if they are logged in)
		m_pChar->Spell_Teleport( pClient->GetChar()->GetTopPoint(), true, false );
		return;
	case 'B':
	case 'K':	// /PAGE ban or kick = locks the account of this person and kicks
		pClient->addKick( m_pChar );
		return;
	case 'J':	// /PAGE jail
		pClient->GetChar()->Jail( this, true );
		return;
	}
}

void CClient::Cmd_GM_PageSelect( int iSelect )
{
	// 0 = cancel.
	// 1 based.

	if ( m_pGMPage != NULL )
	{
		SysMessage( "Current message sent back to the queue" );
		ClearGMHandle();
	}

	if ( iSelect <= 0 || iSelect >= COUNTOF(m_tmMenu))
	{
		return;
	}

	if ( m_tmMenu[iSelect] & 0xFFFF0000 )
	{
		// "MORE" selected
		Cmd_GM_PageMenu( m_tmMenu[iSelect] & 0xFFFF );
		return;
	}

	CGMPage * pPage = dynamic_cast <CGMPage*>( g_World.m_GMPages.GetAt( m_tmMenu[iSelect] ));
	if ( pPage != NULL )
	{
		if ( pPage->FindGMHandler())
		{
			SysMessage( "GM Page already being handled." );
			return;	// someone already has this.
		}

		m_pGMPage = pPage;
		m_pGMPage->SetGMHandler( this );
		Cmd_GM_PageInfo();
		Cmd_GM_PageCmd( "P" );	// go there.
	}
}

void CClient::addTargetVerb( const TCHAR * pCmd, const TCHAR * pArg )
{
	GETNONWHITESPACE( pCmd );
	SKIP_SEPERATORS(pCmd);
	if ( pCmd == pArg ) 
		pCmd = "";
	m_Targ_Text.Format( "%s%s%s", pCmd, pCmd[0] ? " " : "", pArg );
	CGString sPrompt;
	sPrompt.Format( "Select object to set/command '%s'", (const TCHAR*) m_Targ_Text );
	addTarget( TARGMODE_OBJ_SET, sPrompt );
}

bool CClient::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	// "TARGET.BLAH","TARG.BLAH" or "T.BLAH"
	if ( pszKey[0] == 'T' || pszKey[0] == 't' )
	{
		if ( pszKey[1] == '.' )
		{
			pszKey += 2;
rettarg:
			pRef = m_Targ_UID.ObjFind();
			return( true );
		}
		if ( ! strnicmp( pszKey, "TARG.", 5 ))
		{
			pszKey += 5;
			goto rettarg;
		}
		if ( ! strnicmp( pszKey, "TARGET.", 7 ))
		{
			pszKey += 7;
			goto rettarg;
		}

		if ( ! strnicmp( pszKey, "TARGPRV.", 8 ))
		{
			pszKey += 8;
retptarg:
			pRef = m_Targ_PrvUID.ObjFind();
			return( true );
		}
		if ( ! strnicmp( pszKey, "TPRV.", 5 ))
		{
			pszKey += 5;
			goto retptarg;
		}
		if ( ! strnicmp( pszKey, "TARGPROP.", 9 ))
		{
			pszKey += 9;
retproptarg:
			pRef = m_Prop_UID.ObjFind();
			return( true );
		}
		if ( ! strnicmp( pszKey, "TPROP.", 6 ))
		{
			pszKey += 6;
			goto retproptarg;
		}
	}

	return( CScriptObj::r_GetRef( pszKey, pRef, pSrc ));
}

const TCHAR * CClient::sm_KeyTable[] = // static
{
	"ALLMOVE",
	"ALLSHOW",
	"DEBUG",
	"DETAIL",
	"GM",
	"HEARALL",
	"LISTEN",
	"PRIVSHOW",
};

bool CClient::r_WriteVal( const TCHAR * pszKey, CGString & sVal, CTextConsole * pSrc )
{
	if ( m_pAccount == NULL )
		return( false );

	switch ( FindTableSorted( pszKey, sm_KeyTable, COUNTOF(sm_KeyTable)))
	{
	case 0:	// "ALLMOVE"
		sVal.FormatVal( IsPriv( PRIV_ALLMOVE ));
		break;
	case 1: // "ALLSHOW"
		sVal.FormatVal( IsPriv( PRIV_ALLSHOW ));
		break;
	case 2:	// "DEBUG"
		sVal.FormatVal( IsPriv( PRIV_DEBUG ));
		break;
	case 3:	// "DETAIL"
		sVal.FormatVal( IsPriv( PRIV_DETAIL ));
		break;
	case 4:	// "GM" = toggle your GM status on/off
		sVal.FormatVal( IsPriv( PRIV_GM ));
		break;
	case 5:	// "HEARALL"
hearall:
		sVal.FormatVal( IsPriv( PRIV_HEARALL ));
		break;
	case 6:	// "LISTEN"
		goto hearall;
	case 7: // "PRIVSHOW"
		// Hide my priv title.
		sVal.FormatVal( IsPriv( PRIV_PRIV_NOSHOW ));
		break;
	default:
		return( CScriptObj::r_WriteVal( pszKey, sVal, pSrc ));
	}
	return( true );
}

bool CClient::r_LoadVal( CScript & s )
{
	if ( m_pAccount == NULL ) 
		return( false );

	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF(sm_KeyTable)))
	{
	case 0:	// "ALLMOVE"
		m_pAccount->TogPrivFlags( PRIV_ALLMOVE );
		addReSync();
		break;
	case 1: // "ALLSHOW"
		addRemoveAll( true, true );
		m_pAccount->TogPrivFlags( PRIV_ALLSHOW );
		addReSync();
		break;
	case 2:	// "DEBUG"
		m_pAccount->TogPrivFlags( PRIV_DEBUG );
		addRemoveAll( true, false );
		addReSync();
		break;
	case 3:	// "DETAIL"
		m_pAccount->TogPrivFlags( PRIV_DETAIL );
		break;
	case 4:	// "GM" = toggle your GM status on/off
		if ( GetPrivLevel() >= PLEVEL_GM )
		{
			m_pAccount->TogPrivFlags( PRIV_GM );
		}
		break;
	case 5:	// "HEARALL"
scp_hearall:
		m_pAccount->TogPrivFlags( PRIV_HEARALL );
		break;
	case 6:	// "LISTEN"
		goto scp_hearall;
	case 7: // "PRIVSHOW"
		// Hide my priv title.
		if ( GetPrivLevel() >= PLEVEL_Counsel )
		{
			m_pAccount->TogPrivFlags( PRIV_PRIV_NOSHOW );
		}
		break;
	default: 
		return( false );
	}
	return( true );
}

bool CClient::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
{
	// NOTE: ONLY CALL this from CChar::r_Verb !!!
	// Little to no security checking here !!!

	enum
	{
		CV_ADD,
		CV_ADDITEM,
		CV_ADDNPC,
		CV_ADMIN,
		CV_ARROWQUEST,
		CV_BANK,
		CV_BANKSELF,
		CV_CAST,
		CV_CHARLIST,
		CV_CLIENTS,
		CV_CONTROL,
		CV_EVERBTARG,
		CV_EXTRACT,
		CV_FLIP,
		CV_GMMENU,
		CV_GMPAGE,
		CV_GOTARG,
		CV_HELP,
		CV_INFO,
		CV_INFORMATION,
		CV_ITEMMENU,
		CV_LAST,
		CV_LINK,
		CV_MIDI,
		CV_MIDILIST,
		CV_MULTI,
		CV_MUSIC,
		CV_NUKE,
		CV_NUKECHAR,
		CV_PAGE,
		CV_PRIVSET,
		CV_PROPS,
		CV_REPAIR,
		CV_RESEND,
		CV_RESYNC,
		CV_SAVE,
		CV_SELF,
		CV_STATIC,
		CV_SUMMON,
		CV_SYNC,
		CV_SYSMESSAGE,
		CV_TELE,
		CV_TILE,
		CV_TWEAK,
		CV_VERSION,
		CV_WEBLINK,
	};

	static const TCHAR * table[] =
	{
		"ADD",
		"ADDITEM",
		"ADDNPC",
		"ADMIN",
		"ARROWQUEST",
		"BANK",
		"BANKSELF",
		"CAST",
		"CHARLIST",
		"CLIENTS",
		"CONTROL",
		"EVERBTARG",
		"EXTRACT",
		"FLIP",
		"GMMENU",
		"GMPAGE",
		"GOTARG",
		"HELP",
		"INFO",
		"INFORMATION",
		"ITEMMENU",
		"LAST",
		"LINK",
		"MIDI",
		"MIDILIST",
		"MULTI",
		"MUSIC",
		"NUKE",
		"NUKECHAR",
		"PAGE",
		"PRIVSET",
		"PROPS",
		"REPAIR",
		"RESEND",
		"RESYNC",
		"SAVE",
		"SELF",
		"STATIC",
		"SUMMON",
		"SYNC",
		"SYSMESSAGE",
		"TELE",
		"TILE",
		"TWEAK",
		"VERSION",
		"WEBLINK",
	};

	const TCHAR * pszKey = s.GetKey();

	if ( s.IsKeyHead( "SET", 3 ))
	{
		addTargetVerb( pszKey+3, s.GetArgStr());
		return( true );
	}

	if ( toupper( pszKey[0] ) == 'X' )
	{
		// Target this command verb on some other object.
		addTargetVerb( pszKey+1, s.GetArgStr());
		return( true );
	}

	switch ( FindTableSorted( pszKey, table, COUNTOF(table)))
	{
	case CV_ADD:
		goto do_additem;
	case CV_ADDITEM:
do_additem:
		if ( s.HasArgs())
		{
			// FindItemName ???
			return Cmd_CreateItem( s.GetArgHex());
		}
		else
		{
			Cmd_Script_Menu( (TARGMODE_TYPE)( TARGMODE_MENU_ITEMS + 1 ));
		}
		break;
	case CV_ADDNPC:
		// Add a script NPC
		return Cmd_CreateChar( - (int) s.GetArgHex());
	case CV_ADMIN:
do_admin:
		add_Admin_Dialog( s.GetArgVal()); // Open the Admin console	;
		break;
	case CV_ARROWQUEST:
		{
			int piVal[2];
			int iQty = ParseCmds( s.GetArgStr(), piVal, COUNTOF(piVal));
			addArrowQuest( piVal[0], piVal[1] );
		}
		break;
	case CV_BANK:
		m_tmCharBank.m_Layer = (LAYER_TYPE)((s.HasArgs()) ? s.GetArgHex() : LAYER_BANKBOX );
		addTarget( TARGMODE_CHAR_BANK, "Whose bank do you want to open?" );
		break;
	case CV_BANKSELF: // open my own bank
		addBankOpen( m_pChar, (LAYER_TYPE) s.GetArgHex());
		break;
	case CV_CAST:
		return Cmd_Skill_Magery( (SPELL_TYPE) s.GetArgVal(), dynamic_cast <CObjBase *>(pSrc));
	case CV_CHARLIST:
		// ussually just a gm command
		addCharList3();
		break;
	case CV_CLIENTS:
		if ( g_Serv.m_Clients.GetCount() <= ADMIN_CLIENTS_PER_PAGE )
		{
			g_Serv.ListClients( this );
			break;
		}
		goto do_admin;
	case CV_CONTROL: // Possess
		addTarget( TARGMODE_CHAR_CONTROL, "Who do you want to control?" );
		break;
	case CV_EVERBTARG:
		m_Targ_Text = s.GetArgStr();
		addPromptConsole( TARGMODE_ENTER_TARG_VERB, m_Targ_Text.IsEmpty() ? "Enter the verb" : "Enter the text" );
		break;

	case CV_EXTRACT:
		if ( ! s.HasArgs())
		{
			SysMessage( "Usage:" );
			SysMessage( "/EXTRACT filename.ext code" );
			break;
		}
		m_Targ_Text = s.GetArgStr(); // Point at the options, if any
		m_Targ_p.Init(); // Clear this first
		m_tmTile.m_Code = 1;	// set extract code.
		m_tmTile.m_id = 1;	// extract id.
		addTarget( TARGMODE_TILE, "Select area to Extract", true );
		break;
	case CV_FLIP:
		m_Targ_Text = s.GetArgStr(); // Point at the options, if any
		addTarget( TARGMODE_OBJ_FLIP, "What item would you like to flip?" );
		break;
	case CV_GMMENU:
		Cmd_Script_Menu( (TARGMODE_TYPE)( TARGMODE_MENU_GM + s.GetArgVal()));
		break;
	case CV_GMPAGE:
		m_Targ_Text = s.GetArgStr();
		addPromptConsole( TARGMODE_GM_PAGE_TEXT, "Describe your comment or problem" );
		break;
	case CV_GOTARG: // go to my (preselected) target.
		{
			CObjBase * pObj = m_Targ_UID.ObjFind();
			if ( pObj != NULL )
			{
				CPointMap po = pObj->GetTopLevelObj()->GetTopPoint();
				CPointMap pnt = po;
				pnt.Move( DIR_W, 3 );
				WORD wBlockFlags = m_pChar->GetMoveBlockFlags();
				pnt.m_z = g_World.GetHeight( pnt, wBlockFlags, NULL );	// ??? Get Area
				m_pChar->m_face_dir = pnt.GetDir( po, m_pChar->m_face_dir ); // Face the player
				m_pChar->Spell_Teleport( pnt, true, false );
			}
		}
		break;
	case CV_HELP:
		if ( ! s.HasArgs()) // if no command, display the main help system dialog.
		{
			Gump_Setup( TARGMODE_GUMP_HELP_CMD, m_pChar );
		}
		else
		{
			// Below here, we're looking for a command to get help on
			CScript s;
			if ( ! s.OpenFind( GRAY_FILE "help" ))
				break;
			if ( ! s.FindSection( s.GetArgStr()))
			{
				break;
			}
			addScrollFile( s, SCROLL_TYPE_TIPS, 0, s.GetArgStr());
		}
		break;
	case CV_INFO:
		goto do_tweak;
	case CV_INFORMATION:
		SysMessage( g_Serv.GetStatusString( 0x22 ));
		SysMessage( g_Serv.GetStatusString( 0x24 ));
		break;
	case CV_ITEMMENU:
		// cascade to another menu.
		Cmd_Script_Menu( (TARGMODE_TYPE)( TARGMODE_MENU_ITEMS + s.GetArgVal()));
		break;
	case CV_LAST:
		// Fake Previous target.
		if ( GetTargMode() >= TARGMODE_MOUSE_TYPE )
		{
			CObjBase * pObj = m_pChar->m_Act_Targ.ObjFind();
			if ( pObj != NULL )
			{
				CPointMap pt = pObj->GetUnkPoint();
				m_bin.Target.m_code = GetTargMode();
				m_bin.Target.m_x = pt.m_x;
				m_bin.Target.m_y = pt.m_y;
				m_bin.Target.m_z = pt.m_z;
				m_bin.Target.m_UID = pObj->GetUID();
				m_bin.Target.m_id = 0;
				Event_Target();
			}
			break;
		}
		return( false );
	case CV_LINK:	// link doors
		m_Targ_UID.ClearUID();
		addTarget( TARGMODE_LINK, "Select the item to link." );
		break;
	case CV_MIDI:
		goto do_music;
	case CV_MIDILIST:
do_music:
		{
			int piMidi[64];
			int iQty = ParseCmds( s.GetArgStr(), piMidi, COUNTOF(piMidi));
			if ( iQty > 0 )
			{
				addMusic( piMidi[ GetRandVal( iQty ) ] );
			}
		}
		break;
	case CV_MULTI:
		// Create item from script.
		if (s.HasArgs())
		{
			// make an item near by.
			m_tmAdd.m_id = (ITEMID_TYPE) s.GetArgVal();
			addTargetItems( TARGMODE_ADDMULTIITEM, m_tmAdd.m_id );
			break;
		}
		Cmd_Script_Menu( (TARGMODE_TYPE)( TARGMODE_MENU_ITEMS + 1 ));
		break;
	case CV_MUSIC:
		goto do_music;
	case CV_NUKE:
		m_Targ_Text.Empty();
		m_Targ_p.Init(); // Clear this first
		m_tmTile.m_Code = 254;	// set nuke code.
		addTarget( TARGMODE_TILE, "Select area to Nuke", true );
		break;
	case CV_NUKECHAR:
		m_Targ_Text.Empty();
		m_Targ_p.Init(); // Clear this first
		m_tmTile.m_Code = 253;	// set nuke code.
		addTarget( TARGMODE_TILE, "Select area to Nuke Chars", true );
		break;
	case CV_PAGE:
		Cmd_GM_PageCmd( s.GetArgStr());
		break;
	case CV_PRIVSET:
		if ( ! s.HasArgs())
			return( false );
		m_Targ_Text = s.GetArgStr(); // Point at the options, if any
		addTarget( TARGMODE_CHAR_PRIVSET, "Select character to change privelege." );
		break;
	case CV_PROPS:
		goto do_tweak;
	case CV_REPAIR:
		addTarget( TARGMODE_REPAIR, "What item do you want to repair?" );
		break;
	case CV_RESEND:
do_resend:
		addReSync();
		break;
	case CV_RESYNC:
		goto do_resend;
	case CV_SAVE: 
		g_World.Save( s.GetArgVal());
		break;
	case CV_SELF:
		// Fake self target.
		if ( GetTargMode() >= TARGMODE_MOUSE_TYPE )
		{
			m_bin.Target.m_code = GetTargMode();
			CPointMap pt = m_pChar->GetTopPoint();
			m_bin.Target.m_x = pt.m_x;
			m_bin.Target.m_y = pt.m_y;
			m_bin.Target.m_z = pt.m_z;
			m_bin.Target.m_UID = m_pChar->GetUID();
			m_bin.Target.m_id = 0;
			Event_Target();
			break;
		}
		return( false );
	case CV_STATIC:
		if ( s.HasArgs())
		{
			Cmd_CreateItem( s.GetArgHex(), true );
		}
		break;
	case CV_SUMMON:	// from the spell skill script.
		return Cmd_CreateChar( s.GetArgHex());
	case CV_SYNC:
		goto do_resend;
	case CV_SYSMESSAGE:
		SysMessage( s.GetArgStr());
		break;
	case CV_TELE:
		return Cmd_Skill_Magery( SPELL_Teleport, dynamic_cast <CObjBase *>(pSrc));
	case CV_TILE:
		if ( ! s.HasArgs())
		{
			SysMessage( "Usage:" );
			SysMessage( "/TILE z-height item1 item2 itemX" );
			SysMessage( "/TILE command" );
			break;
		}
		m_Targ_Text = s.GetArgStr(); // Point at the options
		m_Targ_p.Init(); // Clear this first
		m_tmTile.m_Code = 0;
		addTarget( TARGMODE_TILE, "Pick 1st corner:", true );
		break;
	case CV_TWEAK:
do_tweak:
		addTarget( TARGMODE_OBJ_PROPS, "What object would you like info on?" );
		break;
	case CV_VERSION:	// "SHOW VERSION"
		SysMessage( g_szServerDescription );
		break;
	case CV_WEBLINK:
		addWebLaunch( s.GetArgStr());
		break;
	default:
		if ( r_LoadVal( s ))
		{
			CGString sVal;
			if ( r_WriteVal( s.GetKey(), sVal, pSrc ))
			{
				SysMessagef( "%s = %s", s.GetKey(), sVal );
				return( true );
			}
		}
		return( CScriptObj::r_Verb( s, pSrc ));
	}
	return( true );
}

void CClient::Menu_Select( const TCHAR * pszSection, int iSelect ) // Menus for general purpose
{
	// A select was made.
	// 0 = cancel.

	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_MENU_2, pszSection ) == NULL )
		return;

	// execute the menu script.

	int i=0;	// 1 based selection.
	while ( s.ReadKeyParse())
	{
		if ( ! s.IsKey( "ON" )) 
			continue;

		i++;
		if ( i < iSelect )
			continue;
		if ( i > iSelect ) 
			break;

		TRIGRET_TYPE iRet = m_pChar->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, m_pChar );
		return;
	}
}

void CClient::Menu_Setup( TARGMODE_TYPE menuid, const TCHAR * pszSection )
{
	// Menus for general purpose
	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_MENU_2, pszSection ) == NULL )
		return;

	s.ReadKey();	// get title for the menu.
	m_pChar->ParseText( s.GetKey(), m_pChar );

	CMenuItem item[MAX_MENU_ITEMS];
	item[0].m_text = s.GetKey();
	item[0].m_id = menuid;	// general context id

	int i=0;
	while (s.ReadKeyParse())
	{
		if ( ! s.IsKey( "ON" ))
			continue;
		if ( ++i >= COUNTOF( item ))
			break;

		TCHAR * pszKey = s.GetArgStr();
		if ( isdigit( pszKey[0] ))
		{
			item[i].m_id = g_Exp.GetSingle(pszKey,true);
		}
		else
		{
			item[i].m_id = i-1;	// ??? diff !!!
		}
		GETNONWHITESPACE( pszKey );
		m_pChar->ParseText( pszKey, m_pChar );
		item[i].m_text = pszKey;
	}

	addItemMenu( item, i );
}

void CClient::Cmd_Script_Menu( TARGMODE_TYPE menuid, int iSelect ) // Menus for general purpose
{
	//
	// iSelect = -1 = 1st setup.
	// iSelect = 0 = cancel
	// iSelect = x = execute the selection.
	//

	if ( ! iSelect ) return;	// cancelled.

	CGString sSec;
	if ( menuid >= TARGMODE_MENU_ITEMS )
	{
		sSec.Format( "ITEMMENU %i", menuid-TARGMODE_MENU_ITEMS);
	}
	else if ( menuid >= TARGMODE_MENU_GUILDSTONE1 ) // Guildstone Menus
	{
		sSec.Format( "GUILDMENU %i", menuid-TARGMODE_MENU_GUILDSTONE1 + 1 );
	}
	else if ( menuid >= TARGMODE_MENU_ADMIN ) // Admin Menus
	{
		sSec.Format( "ADMIN %i", menuid-TARGMODE_MENU_ADMIN);
	}
	else if ( menuid >= TARGMODE_MENU_GM ) // GM Menus
	{
		sSec.Format( "GMMENU %i", menuid-TARGMODE_MENU_GM);
	}
	else
	{
		return;
	}

	if ( iSelect<0)
	{
		Menu_Setup( menuid, sSec );
	}
	else
	{
		Menu_Select( sSec, iSelect );
	}
}

void CClient::Cmd_EditItem( CObjBase * pObj, int iSelect )
{
	// iSelect == -1 = setup.
	if ( pObj == NULL ) 
		return;

	CContainer * pContainer = dynamic_cast <CContainer *> (pObj);
	if ( pContainer == NULL )
	{
		m_Prop_UID = pObj->GetUID();
		addGumpPropMenu();
		return;
	}

	if ( iSelect == 0 )
		return;	// cancelled.

	if ( iSelect > 0 )
	{
		// We selected an item.
		if ( iSelect >= COUNTOF(m_tmMenu))
			return;

		if ( m_Targ_Text.IsEmpty())
		{
			m_Prop_UID = m_tmMenu[iSelect];
			addGumpPropMenu();
		}
		else
		{
			OnTarg_Obj_Set( m_tmMenu[iSelect] );
		}
		return;
	}

	CMenuItem item[MAX_MENU_ITEMS];	// Most as we want to display at one time.

	item[0].m_text.Format( "Contents of %s", pObj->GetName());
	item[0].m_id = TARGMODE_MENU_EDIT;
	m_tmMenu[0] = pObj->GetUID();

	int count=0;
	for ( CItem * pItem = pContainer->GetContentHead(); pItem != NULL; pItem = pItem->GetNext())
	{
		if ( count >= COUNTOF( item )) 
			break;
		m_tmMenu[++count] = pItem->GetUID();
		item[count].m_text = pItem->GetName();
		item[count].m_id = pItem->GetDispID();
	}

	addItemMenu( item, count );
}

bool CClient::Cmd_CreateItem( int id, bool fStatic )
{
	// make an item near by.
	m_tmAdd.m_id = (ITEMID_TYPE) id;
	m_tmAdd.m_fStatic = fStatic;
	return addTargetItems( TARGMODE_ADDITEM, m_tmAdd.m_id );
}

static TCHAR Txt_Summon[] = "Where would you like to summon the creature ?";

bool CClient::Cmd_CreateChar( int id )
{
	// make a creature near by. (GM or script used only)
	// "ADDNPC"
	m_tmSkillMagery.m_Spell = SPELL_Summon;	// m_atMagery.m_Spell
	m_tmSkillMagery.m_SummonID = (CREID_TYPE) id;				// m_atMagery.m_SummonID
	m_Targ_PrvUID = m_pChar->GetUID();
	addTarget( TARGMODE_SKILL_MAGERY, Txt_Summon, true );
	return( true );
}

bool CClient::Cmd_Skill_Magery( SPELL_TYPE iSpell, CObjBase * pSrc )
{
	// start casting a spell. prompt for target.
	// pSrc = you the char.
	// pSrc = magic object is source ?

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Cast Spell %d='%s'\n", GetSocket(), iSpell, g_Serv.m_SpellDefs[iSpell]->GetName()));
	}

	// Do we have the regs ? etc.
	if ( ! m_pChar->Spell_CanCast( iSpell, true, pSrc, true ))
		return false;

	SetTargMode();
	m_tmSkillMagery.m_Spell = iSpell;	// m_atMagery.m_Spell
	m_Targ_UID = m_pChar->GetUID();	// default target.
	m_Targ_PrvUID = pSrc->GetUID();	// source of the spell.

	const TCHAR * pPrompt = "Select Target";
	switch ( iSpell )
	{
	case SPELL_Recall:
		pPrompt = "Select rune to recall from.";
		break;
	case SPELL_Blade_Spirit:
		pPrompt = Txt_Summon;
		break;
	case SPELL_Summon:
		return Cmd_Skill_Menu( SKILL_MAGERY, SPELL_Summon );
	case SPELL_Mark:
		pPrompt = "Select rune to mark.";
		break;
	case SPELL_Gate_Travel:	// gate travel
		pPrompt = "Select rune to gate from.";
		break;
	case SPELL_Polymorph:
		// polymorph creature menu.
		if ( GetPrivLevel() >= PLEVEL_Seer )
		{
			pPrompt = "Select creature to polymorph.";
			break;
		}
		return Cmd_Skill_Menu( SKILL_MAGERY, SPELL_Polymorph );
	case SPELL_Earthquake:
		// cast immediately with no targetting.
		return m_pChar->Spell_Cast( SPELL_Earthquake, m_pChar->GetUID(), m_pChar->GetTopPoint());
	case SPELL_Resurrection:
		pPrompt = "Select ghost to resurrect.";
		break;
	case SPELL_Vortex:
	case SPELL_Air_Elem:
	case SPELL_Daemon:
	case SPELL_Earth_Elem:
	case SPELL_Fire_Elem:
	case SPELL_Water_Elem:
		pPrompt = Txt_Summon;
		break;

		// Necro spells
	case SPELL_Summon_Undead: // Summon an undead
		pPrompt = Txt_Summon;
		break;
	case SPELL_Animate_Dead: // Corpse to zombie
		pPrompt = "Choose a corpse";
		break;
	case SPELL_Bone_Armor: // Skeleton corpse to bone armor
		pPrompt = "Chose a skeleton";
		break;
	}

	addTarget( TARGMODE_SKILL_MAGERY, pPrompt, ! g_Serv.m_SpellDefs[ iSpell ]->IsSpellType( SPELLFLAG_TARG_OBJ | SPELLFLAG_TARG_CHAR ));
	return( true );
}

bool CClient::Cmd_Skill_Tracking( int track_sel, bool fExec )
{
	// look around for stuff.

	if ( track_sel < 0 )
	{
		// Initial pre-track setup.

		if ( m_pChar->m_pArea->IsFlag( REGION_FLAG_SHIP ) &&
			! m_pChar->ContentFind( ITEMID_SPYGLASS_1) &&
			! m_pChar->ContentFind( ITEMID_SPYGLASS_2))
		{
			SysMessage( "You need a Spyglass to use tracking here." );
			return( false );
		}

		// Tacking (unlike other skills) is used during menu setup.
		m_pChar->Skill_Fail( true );	// Kill previous skill.

		CMenuItem item[6];
		item[0].m_id = TARGMODE_MENU_SKILL_TRACK_SETUP;
		item[0].m_text = "Tracking";
		item[1].m_id = ITEMID_TRACK_HORSE;
		item[1].m_text = "Animals";
		item[2].m_id = ITEMID_TRACK_OGRE;
		item[2].m_text = "Monsters";
		item[3].m_id = ITEMID_TRACK_MAN;
		item[3].m_text = "Humans";
		item[4].m_id = ITEMID_TRACK_MAN_NAKED;
		item[4].m_text = "Players";
		item[5].m_id = ITEMID_TRACK_WISP;
		item[5].m_text = "Anything that moves";

		m_tmMenu[0] = 0;
		addItemMenu( item, 5 );
		return( true );
	}

	if ( track_sel ) // Not Cancelled
	{
		ASSERT( ((WORD)track_sel) < COUNTOF( m_tmMenu ));
		if ( fExec )
		{
			// Tracking menu got us here. Start tracking the selected creature.
			m_pChar->SetTimeout( 1*TICK_PER_SEC );
			m_pChar->m_Act_Targ = m_tmMenu[track_sel]; // selected UID
			m_pChar->Skill_Start( SKILL_TRACKING );
			return true;
		}

		static const NPCBRAIN_TYPE Track_Brain[] =
		{
			NPCBRAIN_QTY,	// not used here.
			NPCBRAIN_ANIMAL,
			NPCBRAIN_MONSTER,
			NPCBRAIN_HUMAN,
			NPCBRAIN_NONE,	// players
			NPCBRAIN_QTY,	// anything.
		};

		if ( track_sel >= COUNTOF(Track_Brain))
			track_sel = COUNTOF(Track_Brain)-1;
		NPCBRAIN_TYPE track_type = Track_Brain[ track_sel ];

		CMenuItem item[ min( MAX_MENU_ITEMS, COUNTOF( m_tmMenu )) ];
		int count = 0;

		item[0].m_id = TARGMODE_MENU_SKILL_TRACK;
		item[0].m_text = "Tracking";
		m_tmMenu[0] = track_sel;

		CWorldSearch AreaChars( m_pChar->GetTopPoint(), m_pChar->Skill_GetBase(SKILL_TRACKING)/20 + 10 );
		while (true)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL ) 
				break;
			if ( m_pChar == pChar )
				continue;
			if ( ! m_pChar->CanDisturb( pChar )) 
				continue;

			NPCBRAIN_TYPE basic_type = pChar->GetCreatureType();
			if (( track_type == basic_type ) ||
				( track_type == NPCBRAIN_NONE && basic_type == NPCBRAIN_HUMAN ) ||
				( track_type == NPCBRAIN_QTY ))
			{
				// Can't track polymorphed players this way.
				if ( ! pChar->m_pDef->Can( CAN_C_WALK ))	// never moves or just swims.
					continue;
				count ++;
				item[count].m_id = pChar->m_pDef->m_trackID;
				item[count].m_text = pChar->GetName();
				m_tmMenu[count] = pChar->GetUID();
				if ( count >= COUNTOF( item )-1 ) 
					break;
			}
		}

		if ( count )
		{
			// Some credit for trying.
			m_pChar->Skill_UseQuick( SKILL_TRACKING, 20 + GetRandVal( 30 ));
			addItemMenu( item, count );
			return( true );
		}
		else
		{
			// Some credit for trying.
			m_pChar->Skill_UseQuick( SKILL_TRACKING, 10 + GetRandVal( 30 ));
		}
	}

	// Tracking failed or was cancelled .

	static const TCHAR * Track_FailMsg[] =
	{
		"Tracking Cancelled",
		"You see no signs of animals to track.",
		"You see no signs of monsters to track.",
		"You see no signs of humans to track.",
		"You see no signs of players to track.",
		"You see no signs to track."
	};

	SysMessage( Track_FailMsg[track_sel] );
	return( false );
}

bool CClient::Cmd_Skill_Smith( CItem * pIngots )
{
	if ( pIngots == NULL || pIngots->m_type != ITEM_INGOT )
	{
		SysMessage( "You need ingots for smithing." );
		return( false );
	}
	ASSERT( m_Targ_UID == pIngots->GetUID());
	if ( pIngots->GetTopLevelObj() != m_pChar )
	{
		SysMessage( "The ingots must be on your person" );
		return( false );
	}

	// must have hammer equipped.
	CItem * pHammer = m_pChar->LayerFind( LAYER_HAND1 );
	if ( pHammer == NULL || pHammer->m_type != ITEM_WEAPON_MACE_SMITH )
	{
		SysMessage( "You must weild a smith hammer of some sort." );
		return( false );
	}

	// Select the blacksmith item type.
	// repair items or make type of items.
	if ( ! g_World.IsItemTypeNear( m_pChar->GetTopPoint(), ITEM_FORGE, 3 ))
	{
		SysMessage( "You must be near a forge to smith ingots" );
		return( false );
	}

	// See if we can even use this type of ingot.
	const COreDef * pOre = g_Serv.GetOreColor( pIngots->GetColor());
	if ( pOre )
	{
		if ( m_pChar->Skill_GetAdjusted( SKILL_MINING ) < pOre->m_iMinSkill )
		{
			SysMessage("You lack the skill to use this type of ingot.");
			return( false );
		}
		// Only get the color if the ingot is a colored iron ingot
		if ( pIngots->GetID() == ITEMID_INGOT_IRON )
			m_pChar->m_atCreate.m_Color = pOre->m_wColor;
		else
			m_pChar->m_atCreate.m_Color = 0;
	}
	else
	{
		m_pChar->m_atCreate.m_Color = 0;
	}

	// Select the blacksmith item type.
	// repair items or make type of items.
	return( Cmd_Skill_Menu( SKILL_BLACKSMITHING ));
}

bool CClient::Cmd_Skill_Menu( SKILL_TYPE iSkill, int iOffset, int iSelect )
{
	// Build the skill menu for the curent active skill.
	// m_Targ_UID = the object used to get us here.
	// iSelect = -1 = 1st setup.
	// iSelect = 0 = cancel
	// iSelect = x = execute the selection.
	// RETURN: false = fail/cancel the skill.

	if ( iSelect == 0 || ! g_Serv.m_SkillDefs.IsValidIndex( iSkill ))
		return( false );

	int iDifficulty = 0;

	// Find section.
	CGString sSec;
	if ( iOffset )
		sSec.Format( "%s %i", g_Serv.m_SkillDefs[ iSkill ]->GetKey(), iOffset );
	else
		sSec = g_Serv.m_SkillDefs[ iSkill ]->GetKey();
	CScriptLock s;
	if ( g_Serv.ScriptLock( s, SCPFILE_SKILL_2, sSec ) == NULL )
	{
		return false;
	}

	// Get title line
	if ( ! s.ReadKey())
		return( false );

	CMenuItem item[ min( COUNTOF( m_tmMenu ), MAX_MENU_ITEMS ) ];
	if ( iSelect < 0 )
	{
		item[0].m_id = TARGMODE_MENU_SKILL;
		item[0].m_text = s.GetKey();
		m_Targ_Skill = iSkill;	// setting up this skill.
		m_tmMenu[0] = iOffset;
	}
	else
	{
		DEBUG_CHECK( m_Targ_Skill == iSkill );
	}

	int iQtyMin = 1;
	bool fSkip = false;	// skip this if we lack resources or skill.
	bool fSuccess = false;
	int index = 0;
	int count = 0;
	while ( s.ReadKeyParse())
	{
		if ( s.IsKeyHead( "ON", 2 ))
		{
			// a new option to look at.
			fSkip = false;
			index ++;
			if ( iSelect < 0 )
			{
				TCHAR * ppCmd[2];
				ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd));
				count ++;
				item[count].m_id = Exp_GetHex( ppCmd[0] );
				item[count].m_text = ppCmd[1];
				m_tmMenu[count] = index;
				if ( count >= COUNTOF( item )-1 )
					break;
			}
			else
			{
				if ( index > iSelect )	// we are done.
					break;
			}
			continue;
		}

		if ( fSkip ) // we have decided we cant do this option.
			continue;	
		if ( iSelect > 0 && index != iSelect ) // only interested in the selected option
			continue; 

		if ( s.IsKey("REPLICATE"))
		{
			// For arrows/bolts, how many do they want ?
			if ( s.GetArgVal())
			{
				// Set the quantity that they want to make.
				CItem * pItem = m_Targ_UID.ItemFind();
				if ( pItem != NULL ) iQtyMin = pItem->GetAmount();
			}
			else
			{
				iQtyMin = 1;
			}
		}

		// A resource is required. Do we have it ?
		// should we save this to be consumed later ??? m_Act_Text = s.GetArg()
		//
		if ( s.IsKey("RESOURCE") || s.IsKey("RESTEST"))
		{
			bool fTest = ( !(index == iSelect) || s.IsKey("RESTEST"));
			int iQtyMinTry = m_pChar->ResourceConsume( s.GetArgStr(), iQtyMin, fTest, m_pChar->m_atCreate.m_Color );
			if ( ! iQtyMinTry )	// Can't make any of these.
			{
				count--;
				fSkip = true;
			}
			else
			{
				iQtyMin = iQtyMinTry;
			}
			continue;
		}

		// Check for a skill required.
		if ( s.IsKey("TEST"))
		{
			iDifficulty = m_pChar->Skill_TestScript( s.GetArgStr());
			if ( iDifficulty < 0 )
			{
				count--;
				fSkip = true;
			}
			continue;
		}

		// select to execute any entries here ?
		if ( index == iSelect )
		{
			fSuccess = true;
			if ( s.IsKey("SKILLMENU"))	// ""???
			{
				// Just put up another menu.
				return Cmd_Skill_Menu( iSkill, s.GetArgVal());
			}
			if ( s.IsKey("MAKEITEM"))
			{
				// Create the item via the skill.
				TCHAR * ppCmd[2];
				ParseCmds( s.GetArgStr(), ppCmd, COUNTOF(ppCmd));
				m_pChar->m_Act_Targ = m_Targ_UID;
				m_pChar->m_atCreate.m_ItemID = (ITEMID_TYPE) Exp_GetHex( ppCmd[0] );
				int iMult = Exp_GetVal( ppCmd[1] );
				if ( iMult <= 0 ) iMult = 1;
				m_pChar->m_atCreate.m_Amount = iQtyMin * iMult;

				return m_pChar->Skill_Start( iSkill, iDifficulty );
			}

			// Some other command verb like repair
			// Execute command from script
			if ( m_pChar->OnTriggerRun( s, TRIGRUN_SECTION_SINGLE, m_pChar ) == TRIGRET_RET_TRUE )
			{
				return( false );	
			}
		}
	}

	if ( iSelect > 0 )	// seems our resources disappeared.
	{
		if ( ! fSuccess )
		{
			SysMessage( "You can't make that now." );
		}
		return( fSuccess );
	}
	else
	{
		if ( ! count )
		{
			SysMessage( "You can't make anything with what you have." );
			return( false );
		}
	}

	addItemMenu( item, count );
	return( true );
}

bool CClient::Cmd_Skill_Inscription( int iLevel, int iSelect )
{
	// Select the scroll type to make.
	// iSelect = -1 = 1st setup.
	// iSelect = 0 = cancel
	// iSelect = x = execute the selection.
	// we should already be in inscription skill mode.

	if ( iSelect == 0 ) 
		return( false );	// cancel.

	CItem * pBlank = m_pChar->ContentFind( ITEMID_SCROLL2_BLANK );
	if ( pBlank == NULL )
	{
		SysMessage( "You have no blank scrolls" );
		return( false );
	}

	if ( iSelect > 0 )
	{
		// Select an entry.
		if ( iSelect >= COUNTOF( m_tmMenu ))
			return( false );
		if ( ! m_tmMenu[0] )
		{
			// Just put up the next level menu.
			return( Cmd_Skill_Inscription( m_tmMenu[iSelect], -1 ));
		}

		// Try to create the scroll.
		m_pChar->m_Act_Targ = pBlank->GetUID();
		m_pChar->m_atMagery.m_Spell = (SPELL_TYPE) m_tmMenu[iSelect];
		return( m_pChar->Skill_Start( SKILL_INSCRIPTION ));
	}

	// Set up the menu for the level.
	const TCHAR * pname;
	if ( iLevel > 0 )
	{
		pname = "Spell Circle %d";
	}
	else
	{
		pname = "Spell Circles" ;
	}
	CMenuItem item[9];
	item[0].m_id = TARGMODE_MENU_SKILL_INSCRIPTION;
	item[0].m_text.Format( pname, iLevel );
	m_tmMenu[0] = iLevel;

	int count = 0;
	for ( int k = 1; k <= 8; k ++ )
	{
		ASSERT( count < COUNTOF(item) && count < COUNTOF(m_tmMenu));
		if ( iLevel )
		{
			// Have reags/mana for spell ?
			SPELL_TYPE ispell = (SPELL_TYPE)( (iLevel-1)*8 + k );
			if ( ! m_pChar->Spell_CanCast( ispell, true, m_pChar, false ))
				continue;
			count ++;
			item[count].m_id  =	g_Serv.m_SpellDefs[ispell]->m_SpellID;
			item[count].m_text = g_Serv.m_SpellDefs[ispell]->GetName();
			m_tmMenu[count] = ispell;
		}
		else
		{
			// can we cast anything in this circle ?
			int spells = 0;
			for ( int n=0; n<8; n++ )
			{
				if ( m_pChar->Spell_CanCast( (SPELL_TYPE) ((k-1)*8 + n), true, m_pChar, false ))
					spells ++;
			}
			if ( ! spells ) 
				continue;

			int icircle = ITEMID_SPELL_CIRCLE1 + k - 1;
			count ++;
			item[count].m_id  =	icircle;
			item[count].m_text.Format( "Spell Circle %d", k );
			m_tmMenu[count] = k;
		}
	}

	if ( ! count )
	{
		SysMessage( "You don't know any spells" );
		return( false );
	}

	addItemMenu( item, count );
	return( true );
}

bool CClient::Cmd_Skill_Alchemy( CItem * pReag )
{
	// SKILL_ALCHEMY

	if ( pReag == NULL ) 
		return( false );

	if ( ! m_pChar->CanUse( pReag ))
		return( false );

	if ( pReag->m_type != ITEM_REAGENT )
	{
		// That is not a reagent.
		SysMessage( "That is not a reagent." );
		return( false );
	}
	if ( ! m_pChar->m_Act_TargPrv.IsValidUID())
	{
		// Find bottles to put potions in.
		if ( ! m_pChar->ContentFind(ITEMID_EMPTY_BOTTLE))
		{
			SysMessage( "You have no bottles for your potion." );
			return( false );
		}
	}

	// Put up a menu to decide formula ?
#ifdef COMMENT
	int iCount = Cmd_Skill_Menu( SKILL_ALCHEMY, iOffsetforReagentType );
	if ( ! iCount )
	{
		SysMessage( "You can't make any potions" );
		return( false );
	}

	addItemMenu( item, count-1 );
#endif

	m_pChar->m_Act_Targ = pReag->GetUID();
	return( m_pChar->Skill_Start( SKILL_ALCHEMY ));
}

bool CClient::Cmd_Skill_Cartography( int iLevel )
{
	// select the map type.

	if ( m_pChar->Skill_Wait())
		return( false );

	if ( ! m_pChar->ContentFindType( ITEM_MAP, 0 ))
	{
		SysMessage( "You have no blank parchment to draw on" );
		return( false );
	}

	return( Cmd_Skill_Menu( SKILL_CARTOGRAPHY ));
}

bool CClient::Cmd_SecureTrade( CChar * pChar, CItem * pItem )
{
	// Begin secure trading with a char. (Make the initial offer)

	// Trade window only if another PC.
	if ( ! pChar->IsClient())
	{
		return( pChar->NPC_OnItemGive( m_pChar, pItem ));
	}

	// Is there already a trade window open for this client ?
	CItem * pItemCont = m_pChar->GetContentHead();
	for ( ; pItemCont != NULL; pItemCont = pItemCont->GetNext())
	{
		if ( pItemCont->m_type != ITEM_EQ_TRADE_WINDOW )
			continue; // found it
		CItem * pItemPartner = pItemCont->m_uidLink.ItemFind(); // counterpart trade window.
		if ( pItemPartner == NULL ) 
			continue;
		CChar * pCharPartner = dynamic_cast <CChar*>( pItemPartner->GetParent());
		if ( pCharPartner != pChar )
			continue;
		CItemContainer * pCont = dynamic_cast <CItemContainer *>( pItemCont );
		ASSERT(pCont);
		pCont->ContentAdd( pItem );
		return( true );
	}

	// Open a new one.
	CItemContainer* pCont1 = dynamic_cast <CItemContainer*> (CItem::CreateBase( ITEMID_Bulletin1 ));
	CItemContainer* pCont2 = dynamic_cast <CItemContainer*> (CItem::CreateBase( ITEMID_Bulletin1 ));

	pCont1->m_type = ITEM_EQ_TRADE_WINDOW;
	pCont1->m_itEqTradeWindow.m_fCheck = false;
	pCont1->m_uidLink = pCont2->GetUID();

	pCont2->m_type = ITEM_EQ_TRADE_WINDOW;
	pCont2->m_itEqTradeWindow.m_fCheck = false;
	pCont2->m_uidLink = pCont1->GetUID();

	m_pChar->LayerAdd( pCont1, LAYER_SPECIAL );
	pChar->LayerAdd( pCont2, LAYER_SPECIAL );

	CCommand cmd;
	int len = sizeof(cmd.SecureTrade);

	cmd.SecureTrade.m_Cmd = XCMD_SecureTrade;
	cmd.SecureTrade.m_len = len;
	cmd.SecureTrade.m_action = 0x00;	// init
	cmd.SecureTrade.m_UID = pChar->GetUID();
	cmd.SecureTrade.m_UID1 = pCont1->GetUID();
	cmd.SecureTrade.m_UID2 = pCont2->GetUID();
	cmd.SecureTrade.m_fname = 1;
	strcpy( cmd.SecureTrade.m_name, pChar->GetName());
	xSendPkt( &cmd, len );

	cmd.SecureTrade.m_UID = m_pChar->GetUID();
	cmd.SecureTrade.m_UID1 = pCont2->GetUID();
	cmd.SecureTrade.m_UID2 = pCont1->GetUID();
	strcpy( cmd.SecureTrade.m_name, m_pChar->GetName());
	pChar->m_pClient->xSendPkt( &cmd, len );

	CPointMap pt( 30, 30, 9 );
	pCont1->ContentAdd( pItem, pt );
	return( true );
}

