    // CClientGMPage.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    #include "graysvr.h"        // predef header.
    #include "CIRCServer.h"
    #include "cclient.h"
    
    /////////////////////////////////////////////
    
    void CClient::Cmd_GM_Page( LPCTSTR pszReason ) // Help button (Calls GM Call Menus up)
    {
        // Player pressed the help button.
        // m_Targ_Text = menu desc.
        // CLIMODE_PROMPT_GM_PAGE_TEXT
    
        if ( pszReason == '\0' )
        {
                SysMessage( "Game Master Page Cancelled.");
                return;
        }
    
        CGString sMsg;
        sMsg.Format( "GM Page from %s : %s" DEBUG_CR,
                (LPCTSTR) m_pChar->GetName(), m_pChar->GetUID(), (LPCTSTR) pszReason );
        g_Log.Event( LOGM_GM_PAGE, (LPCTSTR) sMsg );
    
        bool fFound=false;
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( pClient->GetChar() && pClient->IsPriv( PRIV_GM_PAGE )) // found GM
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
        CGMPage * pPage = STATIC_CAST <CGMPage*>( g_World.m_GMPages.GetHead());
        for ( ; pPage!= NULL; pPage = pPage->GetNext())
        {
                if ( ! strcmpi( pPage->GetName(), GetAccount()->GetName()))
                        break;
        }
        if ( pPage != NULL )
        {
                SysMessage( "You have an existing page. It has been updated." );
                pPage->SetReason( pszReason );
                pPage->m_timePage = CServTime::GetCurrentTime();
        }
        else
        {
                // Queue a GM page. (based on account)
                pPage = new CGMPage( GetAccount()->GetName());
                pPage->SetReason( pszReason );  // Description of reason for call.
        }
        pPage->m_ptOrigin = m_pChar->GetTopPoint();             // Origin Point of call.
    }
    
    void CClient::Cmd_GM_PageClear()
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
        Cmd_GM_PageClear();
    
        CMenuItem item; // only display x at a time.
        ASSERT( COUNTOF(item)<=COUNTOF(m_tmMenu.m_Item));
    
        item.m_sText = "GM Page Menu";
    
        int entry=0;
        int count=0;
        CGMPage * pPage = STATIC_CAST <CGMPage*>( g_World.m_GMPages.GetHead());
        for ( ; pPage!= NULL; pPage = pPage->GetNext(), entry++ )
        {
                if ( entry < iEntryStart )
                        continue;
    
                CClient * pGM = pPage->FindGMHandler(); // being handled ?
                if ( pGM != NULL )
                        continue;
    
                if ( ++count >= COUNTOF( item )-1 )
                {
                        // Add the "MORE" option if there is more than 1 more.
                        if ( pPage->GetNext() != NULL )
                        {
                                item.m_id = count-1;
                                item.m_sText.Format( "MORE" );
                                m_tmMenu.m_Item = 0xFFFF0000 | entry;
                                break;
                        }
                }
    
                CClient * pClient = pPage->FindAccount()->FindClient(); // logged in ?
    
                item.m_id = count-1;
                item.m_sText.Format( "%s %s %s",
                        (LPCTSTR) pPage->GetName(),
                        (pClient==NULL) ? "OFF":"ON ",
                        (LPCTSTR) pPage->GetReason());
                m_tmMenu.m_Item = entry;
        }
    
        if ( ! count )
        {
                SysMessage( "No GM pages queued. Use /PAGE ?" );
                return;
        }
        addItemMenu( CLIMODE_MENU_GM_PAGES, item, count );
    }
    
    void CClient::Cmd_GM_PageInfo()
    {
        // Show the current page.
        // This should be a dialog !!!??? book or scroll.
        ASSERT( m_pGMPage );
    
        SysMessagef(
                "Current GM /PAGE Account=%s (%s) "
                "Reason='%s' "
                "Time=%ld",
                (LPCTSTR)m_pGMPage->GetName(),
                (LPCTSTR)m_pGMPage->GetAccountStatus(),
                (LPCTSTR)m_pGMPage->GetReason(),
                m_pGMPage->GetAge());
    }
    
    enum GPV_TYPE
    {
        GPV_B,
        GPV_BAN,
        GPV_C,
        GPV_CURRENT,
        GPV_D,
        GPV_DELETE,
        GPV_G,  // /PAGE go
        GPV_GO,
        GPV_H,  // help ?
        GPV_HELP,
        GPV_J,  // /PAGE jail
        GPV_JAIL,
        GPV_K,  // /PAGE ban or kick = locks the account of this person and kicks
        GPV_KICK,       // /PAGE ban or kick = locks the account of this person and kicks
        GPV_L,  // List
        GPV_LIST,
        GPV_O,
        GPV_OFF,
        GPV_ON,
        GPV_ORIGIN,
        GPV_P,  // /PAGE player = go to the player that made the page. (if they are logged in)
        GPV_PLAYER,
        GPV_Q,
        GPV_QUEUE,
        GPV_U,
        GPV_UNDO,
        GPV_WIPE,
        GPV_QTY,
    };
    
    static LPCTSTR const sm_pszGMPageVerbs =
    {
        "B",
        "BAN",
        "C",
        "CURRENT",
        "D",
        "DELETE",
        "G",    // /PAGE go
        "GO",
        "H",    // help ?
        "HELP",
        "J",    // /PAGE jail
        "JAIL",
        "K",    // /PAGE ban or kick = locks the account of this person and kicks
        "KICK", // /PAGE ban or kick = locks the account of this person and kicks
        "L",    // List
        "LIST",
        "O",
        "OFF",
        "ON",
        "ORIGIN",
        "P",    // /PAGE player = go to the player that made the page. (if they are logged in)
        "PLAYER",
        "Q",
        "QUEUE",
        "U",
        "UNDO",
        "WIPE",
    };
    
    static LPCTSTR const sm_pszGMPageVerbsHelp =
    {
        "/PAGE on/off" DEBUG_CR,
        "/PAGE list = list of pages." DEBUG_CR,
        "/PAGE delete = dispose of this page. Assume it has been handled." DEBUG_CR,
        "/PAGE origin = go to the origin point of the page" DEBUG_CR,
        "/PAGE undo/queue = put back in the queue" DEBUG_CR,
        "/PAGE current = info on the current selected page." DEBUG_CR,
        "/PAGE go/player = go to the player that made the page. (if they are logged in)" DEBUG_CR,
        "/PAGE jail" DEBUG_CR,
        "/PAGE ban/kick" DEBUG_CR,
        "/PAGE wipe (gm only)"
    };
    
    void CClient::Cmd_GM_PageCmd( LPCTSTR pszCmd )
    {
        // A GM page command.
        // Put up a list of GM pages pending.
    
        if ( pszCmd == NULL || pszCmd == '?' )
        {
                // addScrollText( pCmds );
                for ( int i=0; i<COUNTOF(sm_pszGMPageVerbsHelp); i++ )
                {
                        SysMessage( sm_pszGMPageVerbsHelp );
                }
                return;
        }
        if ( pszCmd == '\0' )
        {
                if ( m_pGMPage )
                        Cmd_GM_PageInfo();
                else
                        Cmd_GM_PageMenu();
                return;
        }
    
        int index = FindTableHeadSorted( pszCmd, sm_pszGMPageVerbs, COUNTOF(sm_pszGMPageVerbs) );
        if ( index < 0 )
        {
                // some other verb ?
    #if 0
                if ( m_pGMPage )
                {
                        r_Verb( sdfsdf );
                }
    #endif
                Cmd_GM_PageCmd(NULL);
                return;
        }
    
        switch ( index )
        {
        case GPV_OFF:
                if ( GetPrivLevel() < PLEVEL_Counsel )
                        return; // cant turn off.
                ClearPrivFlags( PRIV_GM_PAGE );
                Cmd_GM_PageClear();
                SysMessage( "GM pages off" );
                return;
        case GPV_ON:
                SetPrivFlags( PRIV_GM_PAGE );
                SysMessage( "GM pages on" );
                return;
        case GPV_WIPE:
                if ( ! IsPriv( PRIV_GM ))
                        return;
                g_World.m_GMPages.DeleteAll();
                return;
        case GPV_H:     // help ?
        case GPV_HELP:
                Cmd_GM_PageCmd(NULL);
                return;
        }
    
        if ( m_pGMPage == NULL )
        {
                // No gm page has been selected yet.
                Cmd_GM_PageMenu();
                return;
        }
    
        // We must have a select page for these commands.
        switch ( index )
        {
        case GPV_L:     // List
        case GPV_LIST:
                Cmd_GM_PageMenu();
                return;
        case GPV_D:
        case GPV_DELETE:
                // /PAGE delete = dispose of this page. We assume it has been handled.
                SysMessage( "GM Page deleted" );
                delete m_pGMPage;
                ASSERT( m_pGMPage == NULL );
                return;
        case GPV_O:
        case GPV_ORIGIN:
                // /PAGE origin = go to the origin point of the page
                m_pChar->Spell_Teleport( m_pGMPage->m_ptOrigin, true, false );
                return;
        case GPV_U:
        case GPV_UNDO:
        case GPV_Q:
        case GPV_QUEUE:
                // /PAGE queue = put back in the queue
                SysMessage( "GM Page re-queued." );
                Cmd_GM_PageClear();
                return;
        case GPV_B:
        case GPV_BAN:
        case GPV_K:     // /PAGE ban or kick = locks the account of this person and kicks
        case GPV_KICK:  // /PAGE ban or kick = locks the account of this person and kicks
                // This should work even if they are not logged in.
                {
                        CAccountRef pAccount = m_pGMPage->FindAccount();
                        if ( pAccount )
                        {
                                if ( ! pAccount->Kick( this, true ))
                                        return;
                        }
                        else
                        {
                                SysMessage( "Invalid account for page !?" );
                        }
                }
                break;
        case GPV_C:
        case GPV_CURRENT:
                // What am i servicing ?
                Cmd_GM_PageInfo();
                return;
        }
    
        // Manipulate the character only if logged in.
    
        CClient * pClient = m_pGMPage->FindAccount()->FindClient();
        if ( pClient == NULL || pClient->GetChar() == NULL )
        {
                SysMessage( "The account is not logged in." );
                if ( index == GPV_P || index == GPV_PLAYER )
                {
                        m_pChar->Spell_Teleport( m_pGMPage->m_ptOrigin, true, false );
                }
                return;
        }
    
        switch ( index )
        {
        case GPV_G:
        case GPV_GO: // /PAGE player = go to the player that made the page. (if they are logged in)
        case GPV_P:     // /PAGE player = go to the player that made the page. (if they are logged in)
        case GPV_PLAYER:
                m_pChar->Spell_Teleport( pClient->GetChar()->GetTopPoint(), true, false );
                return;
        case GPV_B:
        case GPV_BAN:
        case GPV_K:     // /PAGE ban or kick = locks the account of this person and kicks
        case GPV_KICK:  // /PAGE ban or kick = locks the account of this person and kicks
                pClient->addKick( m_pChar );
                return;
        case GPV_J:     // /PAGE jail
        case GPV_JAIL:
                pClient->GetChar()->Jail( this, true, 0 );
                return;
        default:
                DEBUG_CHECK(0);
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
                Cmd_GM_PageClear();
        }
    
        if ( iSelect <= 0 || iSelect >= COUNTOF(m_tmMenu.m_Item))
        {
                return;
        }
    
        if ( m_tmMenu.m_Item & 0xFFFF0000 )
        {
                // "MORE" selected
                Cmd_GM_PageMenu( m_tmMenu.m_Item & 0xFFFF );
                return;
        }
    
        CGMPage * pPage = STATIC_CAST <CGMPage*>( g_World.m_GMPages.GetAt( m_tmMenu.m_Item ));
        if ( pPage != NULL )
        {
                if ( pPage->FindGMHandler())
                {
                        SysMessage( "GM Page already being handled." );
                        return; // someone already has this.
                }
    
                m_pGMPage = pPage;
                m_pGMPage->SetGMHandler( this );
                Cmd_GM_PageInfo();
                Cmd_GM_PageCmd( "P" );  // go there.
        }
    }