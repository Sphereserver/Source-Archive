    // CClient.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    #include "graysvr.h"        // predef header.
    #include "CIRCServer.h"
    #include "cclient.h"
    
    /////////////////////////////////////////////////////////////////
    // -CClient stuff.
    
    CClient::CClient( SOCKET client ) : 
        m_Socket( client )
    {
        // This may be a web connection or Telnet ?
    
        m_ConnectType = CONNECT_UNK;    // don't know what sort of connect this is yet.
        m_Crypt.SetClientVer( g_Serv.m_ClientVersion );
        m_dwCompressXORIndex = 0;
        m_pAccount = NULL;
    
        m_pChar = NULL;
        m_pGMPage = NULL;
    
        m_timeLogin.Init();
        m_timeLastSend =
        m_timeLastEvent = CServTime::GetCurrentTime();
    
        m_bin_PrvMsg = XCMD_QTY;
    
        m_wWalkCount = -1;
        m_iWalkStepCount = 0;
        m_fPaused = false;
    
        m_Targ_Mode = CLIMODE_SETUP_CONNECTING;
        m_tmSetup.m_dwIP = 0;
        m_tmSetup.m_iConnect = 0;
    
        memset( m_Walk_LIFO, 0, sizeof( m_Walk_LIFO )); // Initialize the fast walk killer stuff
        m_Walk_InvalidEchos = 0;
        m_Walk_CodeQty = -1;    // how many valid codes in here ?
    
        m_Env.SetInvalid();
    
        g_Serv.ClientsInc();
        g_Serv.m_Clients.InsertHead( this );
    
        CSocketAddress PeerName = m_Socket.GetPeerName();
        if ( ! PeerName.IsSameIP( g_BackTask.m_RegisterIP ))
        {
                g_Log.Event( LOGM_CLIENTS_LOG, "%x:Client connected  from '%s'." DEBUG_CR,
                        m_Socket.GetSocket(), g_Serv.StatGet( SERV_STAT_CLIENTS ), (LPCTSTR) PeerName.GetAddrStr());
        }
    
    #ifdef _WIN32
        DWORD lVal = 1; // 0 =  block
        int iRet = m_Socket.IOCtlSocket( FIONBIO, &lVal );
        DEBUG_CHECK( iRet==0 );
    #endif
        if ( g_Serv.m_uSizeMsgMax < MAX_BUFFER )
        {
                // MAX_BUFFER for outgoing buffers ?
        }
    
        // disable NAGLE algorythm for data compression/coalescing. Send as fast as we can. we handle packing ourselves.
        BOOL nbool=TRUE;
        m_Socket.SetSockOpt( TCP_NODELAY, &nbool, sizeof(BOOL), IPPROTO_TCP );
    
        DEBUG_CHECK( g_Serv.StatGet( SERV_STAT_CLIENTS ) == g_Serv.m_Clients.GetCount());
    }
    
    CClient::~CClient()
    {
        g_Serv.StatDec( SERV_STAT_CLIENTS );
    
        CSocketAddress PeerName = m_Socket.GetPeerName();
        if ( ! PeerName.IsSameIP( g_BackTask.m_RegisterIP ))
        {
                g_Log.Event( LOGM_CLIENTS_LOG, "%x:Client disconnected " DEBUG_CR, m_Socket.GetSocket(), g_Serv.StatGet(SERV_STA
T_CLIENTS));
        }
    
        if ( m_ConnectType == CONNECT_IRC )
        {
                g_IRCLocalServer.ClientQuit(this);
        }
    
        CharDisconnect();       // am i a char in game ?
        Cmd_GM_PageClear();
    
        CAccount * pAccount = GetAccount();
        if ( pAccount )
        {
                pAccount->OnLogout( this );
                m_pAccount = NULL;              // unattach first.
                if ( pAccount->IsPriv(PRIV_TEMPORARY) || pAccount->GetPrivLevel() >= PLEVEL_QTY ) // kill the temporary account.
                {
                        g_Accounts.Account_Delete( pAccount );
                }
        }
    
        xFlush();
    }
    
    bool CClient::CanInstantLogOut() const
    {
        if ( g_Serv.IsLoading())        // or exiting.
                return( true );
        if ( ! g_Cfg.m_iClientLingerTime )
                return true;
        if ( IsPriv( PRIV_GM ))
                return true;
        if ( m_pChar == NULL )
                return( true );
        if ( m_pChar->IsStatFlag(STATF_DEAD))
                return( true );
        if ( m_pChar->IsStatFlag(STATF_Stone))
                return( false );
    
        const CRegionWorld * pArea = m_pChar->GetRegion();
        if ( pArea == NULL )
                return( true );
        if ( pArea->IsFlag( REGION_FLAG_INSTA_LOGOUT ))
                return( true );
    
    #if 0
        // Camp near by ? have we been standing near it long enough ?
        if ( pArea->IsGuarded() && ! m_pChar->IsStatFlag( STATF_Criminal ))
                return( true );
    #endif
    
        return( false );
    }
    
    void CClient::CharDisconnect()
    {
        // Disconnect the CChar from the client.
        // Even tho the CClient might stay active.
        if ( m_pChar == NULL )
                return;
    
        Announce( false );
        bool fCanInstaLogOut = CanInstantLogOut();
        m_pChar->ClientDetach();        // we are not a client any more.
    
        m_pChar->OnTrigger( CTRIG_LogOut, m_pChar );
    
        // log out immediately ? (test before ClientDetach())
        if ( ! fCanInstaLogOut )
        {
                // become an NPC for a little while
                CItem * pItemChange = CItem::CreateBase( ITEMID_RHAND_POINT_W );
                ASSERT(pItemChange);
                pItemChange->SetType( IT_EQ_CLIENT_LINGER );
                pItemChange->SetTimeout( g_Cfg.m_iClientLingerTime );
                m_pChar->LayerAdd( pItemChange, LAYER_FLAG_ClientLinger );
        }
        else
        {
                // remove me from other clients screens now.
                m_pChar->SetDisconnected();
        }
    
        m_pChar = NULL;
    }
    
    void CClient::SysMessage( LPCTSTR pszMsg, HUE_TYPE wHue ) const // System message (In lower left corner)
    {
        // Diff sorts of clients.
    
        switch (m_ConnectType )
        {
        case CONNECT_TELNET:
                {
                if ( pszMsg == NULL || ISINTRESOURCE(pszMsg))
                        return;
                for ( ; *pszMsg != '\0'; pszMsg++ )
                {
                        if ( *pszMsg == '\n' )  // translate.
                        {
                                (const_cast <CClient*>(this))->xSendReady( "\r", 1 );
                        }
                        (const_cast <CClient*>(this))->xSendReady( pszMsg, 1 );
                }
                }
                return;
        case CONNECT_CRYPT:
        case CONNECT_LOGIN:
        case CONNECT_GAME:
                const_cast <CClient*>(this)->addSysMessage( pszMsg, wHue );
                return;
    
        case CONNECT_HTTP:
                const_cast <CClient*>(this)->m_Targ_Text = pszMsg;
                return;
    
        // else toss it.?
        }
    }
    
    void CClient::Announce( bool fArrive ) const
    {
        ASSERT( GetChar() != NULL );
        ASSERT( GetChar()->m_pPlayer != NULL );
        if ( GetAccount() == NULL || GetChar() == NULL || GetChar()->m_pPlayer == NULL )
                return;
    
        // We have logged in or disconnected.
        // Annouce my arrival or departure.
        if ( g_Cfg.m_fArriveDepartMsg )
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
                                const CRegionBase * pRegion = m_pChar->GetTopPoint().GetRegion( REGION_TYPE_AREA );
                                sMsg.Format( _TEXT( "%s has %s %s." ),
                                        (LPCTSTR) m_pChar->GetName(),
                                        (fArrive)? _TEXT("arrived in") : _TEXT("departed from"),
                                        pRegion ? (LPCTSTR) pRegion->GetName() : (LPCTSTR) g_Serv.GetName());
                        }
                        pClient->SysMessage( sMsg );
                }
        }
    
        // re-Start murder decay if arriving
    
        if ( m_pChar->m_pPlayer->m_wMurders )
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
    
    bool CClient::CanHear( const CObjBaseTemplate * pSrc, TALKMODE_TYPE mode ) const
    {
        // can we hear this text or sound.
    
        if ( ! IsConnectTypePacket())
        {
                if ( m_ConnectType != CONNECT_TELNET )
                        return( false );
                if ( mode == TALKMODE_BROADCAST ) // && GetAccount()
                        return( true );
                return( false );
        }
    
        if ( mode == TALKMODE_BROADCAST || pSrc == NULL )
                return( true );
        if ( m_pChar == NULL )
                return( false );
    
        if ( IsPriv( PRIV_HEARALL ) &&
                pSrc->IsChar() && 
                ( mode == TALKMODE_SYSTEM || mode == TALKMODE_SAY || mode == TALKMODE_WHISPER || mode == TALKMODE_YELL ))
        {
                const CChar * pCharSrc = dynamic_cast <const CChar*> ( pSrc );
                ASSERT(pCharSrc);
                if ( pCharSrc && pCharSrc->IsClient())
                {
                        if ( pCharSrc->GetPrivLevel() <= GetPrivLevel())
                        {
                                return( true );
                        }
                }
                // Else it does not apply.
        }
    
        return( m_pChar->CanHear( pSrc, mode ));
    }
    
    ////////////////////////////////////////////////////
    
    void CClient::addTargetVerb( LPCTSTR pszCmd, LPCTSTR pszArg )
    {
        // Target a verb at some object .
    
        ASSERT(pszCmd);
        GETNONWHITESPACE(pszCmd);
        SKIP_SEPERATORS(pszCmd);
    
        if ( pszCmd == pszArg )
                pszArg = "";
    
        m_Targ_Text.Format( "%s%s%s", pszCmd, ( pszArg && pszCmd ) ? " " : "", pszArg );
        CGString sPrompt;
        sPrompt.Format( "Select object to set/command '%s'", (LPCTSTR) m_Targ_Text );
        addTarget( CLIMODE_TARG_OBJ_SET, sPrompt );
    }
    
    enum CLIR_TYPE
    {
        CLIR_ACCOUNT,
        CLIR_GMPAGEP,
        CLIR_SACCOUNT,
        CLIR_T,
        CLIR_TARG,
        CLIR_TARGET,
        CLIR_TARGPROP,
        CLIR_TARGPRV,
        CLIR_TPROP,
        CLIR_TPRV,
        CLIR_QTY,
    };
    
    LPCTSTR const CClient::sm_szRefKeys =
    {
        "ACCOUNT",
        "GMPAGEP",
        "SACCOUNT",
        "T",
        "TARG",
        "TARGET",
        "TARGPROP",
        "TARGPRV",
        "TPROP",
        "TPRV",
        NULL,
    };
    
    bool CClient::r_GetRefSingle( LPCTSTR & pszKey, CScriptObj * & pRef )
    {
        int i = FindTableHeadSorted( pszKey, sm_szRefKeys, COUNTOF(sm_szRefKeys)-1 );
        if ( i < 0 )
                return( false );
    
        pszKey += strlen( sm_szRefKeys );
        SKIP_SEPERATORS(pszKey);
        switch (i)
        {
        case CLIR_ACCOUNT:
                if ( pszKey != '.' )    // only used as a ref !
                        break;
                pRef = GetAccount();
                return( true );
        case CLIR_GMPAGEP:
                pRef = m_pGMPage;
                return( true );
        case CLIR_SACCOUNT:
                // m_Targ_Text = name of an account.
                pRef = g_Accounts.Account_FindCheck( m_Targ_Text );
                return( true );
        case CLIR_T:
        case CLIR_TARG:
        case CLIR_TARGET:
                pRef = m_Targ_UID.ObjFind();
                return( true );
        case CLIR_TARGPRV:
        case CLIR_TPRV:
                pRef = m_Targ_PrvUID.ObjFind();
                return( true );
        case CLIR_TARGPROP:
        case CLIR_TPROP:
                pRef = m_Prop_UID.ObjFind();
                return( true );
    
        default:
                DEBUG_CHECK(0);
                return( false );
        }
        return( false );
    }
    
    LPCTSTR const CClient::sm_szLoadKeys = // static
    {
        "ALLMOVE",
        "ALLSHOW",
        "CLIENTVER",
        "CLIENTVERSION",
        "CLIVER",
        "DEBUG",
        "DETAIL",
        "GM",
        "HEARALL",
        "LISTEN",
        "PRIVHIDE",
        "PRIVSHOW",
        "TARG",
        "TARGP",
        "TARGPROP",
        "TARGPRV",
        "TARGTXT",
        NULL,
    };
    
    LPCTSTR const CClient::sm_szVerbKeys =      // static
    {
        "ADD",
        "ADDITEM",
        "ADDNPC",
        "ADMIN",
        "ARROWQUEST",
        "BANKSELF",
        "CAST",
        "CHARLIST",
        "CLIENTS",
        "EVERBTARG",
        "EXTRACT",
        "GHOST",
        "GMPAGE",
        "GOTARG",
        "HELP",
        "INFO",
        "INFORMATION",
        "ITEMMENU",
        "LAST",
        "LINK",
        "LOGIN",
        "LOGOUT",
        "MENU",
        "MIDI",
        "MIDILIST",
        "MUSIC",
        "NUDGE",
        "NUKE",
        "NUKECHAR",
        "ONECLICK",
        "PAGE",
        "REPAIR",
        "RESEND",
        "RESYNC",
        "SAVE",
        "SCROLL",
        "SELF",
        "SHOWSKILLS",
        "SKILLMENU",
        "STATIC",
        "SUMMON",
        "SYNC",
        "SYSMESSAGE",
        "TELE",
        "TILE",
        "UNEXTRACT",
        "VERSION",
        "WEBLINK",
        NULL,
    };
    
    void CClient::r_DumpLoadKeysSingle( CTextConsole * pSrc )
    {
        CScriptObj::r_DumpKeys(pSrc,sm_szLoadKeys);
    }
    void CClient::r_DumpVerbKeysSingle( CTextConsole * pSrc )
    {
        CScriptObj::r_DumpKeys(pSrc,sm_szVerbKeys);
    }
    int CClient::r_GetVerbIndex( LPCTSTR pszKey ) // static
    {
        return FindTableSorted( pszKey, sm_szVerbKeys, COUNTOF(sm_szVerbKeys)-1 );
    }
    
    bool CClient::r_WriteValSingle( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
    {
        int index = FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
        switch (index)
        {
        case CC_ALLMOVE:
                sVal.FormatVal( IsPriv( PRIV_ALLMOVE ));
                break;
        case CC_ALLSHOW:
                sVal.FormatVal( IsPriv( PRIV_ALLSHOW ));
                break;
        case CC_CLIENTVER:
        case CC_CLIVER:
        case CC_CLIENTVERSION:
                {
                        TCHAR szVersion;
                        sVal = m_Crypt.WriteClientVer( szVersion );
                }
                break;
        case CC_DEBUG:
                sVal.FormatVal( IsPriv( PRIV_DEBUG ));
                break;
        case CC_DETAIL:
                sVal.FormatVal( IsPriv( PRIV_DETAIL ));
                break;
        case CC_GM:     // toggle your GM status on/off
                sVal.FormatVal( IsPriv( PRIV_GM ));
                break;
        case CC_HEARALL:
    hearall:
                sVal.FormatVal( IsPriv( PRIV_HEARALL ));
                break;
        case CC_LISTEN:
                goto hearall;
        case CC_PRIVHIDE:
                // Hide my priv title.
                sVal.FormatVal( IsPriv( PRIV_PRIV_NOSHOW ));
                break;
        case CC_PRIVSHOW:
                // Show my priv title.
                sVal.FormatVal( ! IsPriv( PRIV_PRIV_NOSHOW ));
                break;
        case CC_TARG:
                sVal.FormatVal( m_Targ_UID );
                break;
        case CC_TARGP:
                sVal = m_Targ_p.WriteUsed();
                break;
        case CC_TARGPROP:
                sVal.FormatVal( m_Prop_UID );
                break;
        case CC_TARGPRV:
                sVal.FormatVal( m_Targ_PrvUID );
                break;
        case CC_TARGTXT:
                sVal = m_Targ_Text;
                break;
        default:
                return( false );
        }
        return( true );
    }
    
    bool CClient::r_LoadValSingle( CScript & s )
    {
        int i = FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
        if ( i < 0 )
                return( false );
    
        CAccountRef pAccount = GetAccount();
        if ( pAccount == NULL )
                return( false );
    
        switch (i)
        {
        case CC_ALLMOVE:
                pAccount->TogPrivFlags( PRIV_ALLMOVE, s.GetArgStr() );
                addReSync();
                break;
        case CC_ALLSHOW:
                addRemoveAll( true, true );
                pAccount->TogPrivFlags( PRIV_ALLSHOW, s.GetArgStr() );
                addReSync();
                break;
        case CC_DEBUG:
                pAccount->TogPrivFlags( PRIV_DEBUG, s.GetArgStr() );
                addRemoveAll( true, false );
                addReSync();
                break;
        case CC_DETAIL:
                pAccount->TogPrivFlags( PRIV_DETAIL, s.GetArgStr() );
                break;
        case CC_GM: // toggle your GM status on/off
                if ( GetPrivLevel() >= PLEVEL_GM )
                {
                        pAccount->TogPrivFlags( PRIV_GM, s.GetArgStr() );
                }
                break;
        case CC_HEARALL:
    scp_hearall:
                pAccount->TogPrivFlags( PRIV_HEARALL, s.GetArgStr() );
                break;
        case CC_LISTEN:
                goto scp_hearall;
        case CC_PRIVHIDE:
                pAccount->TogPrivFlags( PRIV_PRIV_NOSHOW, s.GetArgStr() );
                break;
        case CC_PRIVSHOW:
                // Hide my priv title.
                if ( GetPrivLevel() >= PLEVEL_Counsel )
                {
                        if ( ! s.HasArgs())
                        {
                                pAccount->TogPrivFlags( PRIV_PRIV_NOSHOW, NULL );
                        }
                        else if ( s.GetArgVal() )
                        {
                                pAccount->ClearPrivFlags( PRIV_PRIV_NOSHOW );
                        }
                        else
                        {
                                pAccount->SetPrivFlags( PRIV_PRIV_NOSHOW );
                        }
                }
                break;
    
        case CC_TARG:
                m_Targ_UID = s.GetArgVal();
                break;
        case CC_TARGP:
                m_Targ_p.Read( s.GetArgRaw());
                break;
        case CC_TARGPROP:
                m_Prop_UID = s.GetArgVal();
                break;
        case CC_TARGPRV:
                m_Targ_PrvUID = s.GetArgVal();
                break;
        default:
                DEBUG_CHECK(false);
                return( false );
        }
        return( true );
    }
    
    bool CClient::r_VerbSingle( CScript & s, CTextConsole * pSrc ) // Execute command from script
    {
        // NOTE: This can be called directly from a RES_WEBPAGE script.
        //  So do not assume we are a game client !
        // NOTE: Mostly called from CChar::r_Verb
        // NOTE: Little security here so watch out for dangerous scripts !
    
        ASSERT(pSrc);
        LPCTSTR pszKey = s.GetKey();
    
        if ( s.IsKeyHead( "SET", 3 ))
        {
                ASSERT( m_pChar );
                addTargetVerb( pszKey+3, s.GetArgRaw());
                return( true );
        }
    
        if ( toupper( pszKey ) == 'X' )
        {
                // Target this command verb on some other object.
                ASSERT( m_pChar );
                addTargetVerb( pszKey+1, s.GetArgRaw());
                return( true );
        }
    
        int index = r_GetVerbIndex( s.GetKey());
        if ( index < 0 )
        {
                if ( r_LoadValSingle( s ))
                {
                        CGString sVal;
                        if ( r_WriteValSingle( s.GetKey(), sVal, pSrc ))
                        {
                                SysMessagef( "%s = %s", (LPCTSTR) s.GetKey(), (LPCTSTR) sVal ); // feedback on what we just did.
                                return( true );
                        }
                }
                return( false );
        }
    
        switch (index)
        {
        case CV_ADD:
                goto do_additem;
        case CV_ADDITEM:
    do_additem:
                if ( s.HasArgs())
                {
                        // FindItemName ???
                        TCHAR * pszArgs = s.GetArgStr();
                        RESOURCE_ID rid = g_Cfg.ResourceGetID( RES_ITEMDEF, pszArgs );
    
                        if ( rid.GetResType() == RES_CHARDEF )
                        {
                                m_Targ_PrvUID.InitUID();
                                return Cmd_CreateChar( (CREID_TYPE) rid.GetResIndex(), SPELL_Summon, false );
                        }
    
                        ITEMID_TYPE id = (ITEMID_TYPE) rid.GetResIndex();
                        return Cmd_CreateItem( id );
                }
                else
                {
                        Menu_Setup( g_Cfg.ResourceGetIDType( RES_MENU, _TEXT("MENU_ADDITEM")));
                }
                break;
        case CV_ADDNPC:
                // Add a script NPC
                m_Targ_PrvUID.InitUID();
                return Cmd_CreateChar( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr()), SPELL_Summon, false
 );
        case CV_ADMIN:
    do_admin:
                {
                        int piVal;
                        int iQty = Str_ParseCmds( s.GetArgRaw(), piVal, COUNTOF(piVal));
                        addGumpDialogAdmin( piVal, piVal ); // Open the Admin console
                }
                break;
        case CV_ARROWQUEST:
                {
                        int piVal;
                        int iQty = Str_ParseCmds( s.GetArgRaw(), piVal, COUNTOF(piVal));
                        addArrowQuest( piVal, piVal );
                }
                break;
        case CV_BANKSELF: // open my own bank
                addBankOpen( m_pChar, (LAYER_TYPE) s.GetArgVal());
                break;
        case CV_CAST:
                return Cmd_Skill_Magery( (SPELL_TYPE) g_Cfg.ResourceGetIndexType( RES_SPELL, s.GetArgStr()), dynamic_cast <CObjB
ase *>(pSrc));
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
    
        case CV_EVERBTARG:
                m_Targ_Text = s.GetArgStr();
                addPromptConsole( CLIMODE_PROMPT_TARG_VERB, m_Targ_Text.IsEmpty() ? "Enter the verb" : "Enter the text" );
                break;
    
        case CV_EXTRACT:
                // sort of like EXPORT but for statics.
                // Opposite of the "UNEXTRACT" command
    
                if ( ! s.HasArgs())
                {
                        SysMessage( "Usage: EXTRACT filename.ext code" );
                }
                else
                {
                        TCHAR * ppArgs;
                        Str_ParseCmds( s.GetArgStr(), ppArgs, COUNTOF( ppArgs ));
    
                        m_Targ_Text = ppArgs; // Point at the options, if any
                        m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                        m_tmTile.m_Code = CV_EXTRACT;   // set extract code.
                        m_tmTile.m_id = Exp_GetVal(ppArgs);     // extract id.
                        addTarget( CLIMODE_TARG_TILE, "Select area to Extract", true );
                }
                break;
    
        case CV_UNEXTRACT:
                // Create item from script.
                // Opposite of the "EXTRACT" command
                if ( ! s.HasArgs())
                {
                        SysMessage( "Usage: UNEXTRACT filename.ext code" );
                }
                else
                {
                        TCHAR * ppArgs;
                        Str_ParseCmds( s.GetArgStr(), ppArgs, COUNTOF( ppArgs ));
    
                        m_Targ_Text = ppArgs; // Point at the options, if any
                        m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                        m_tmTile.m_Code = CV_UNEXTRACT; // set extract code.
                        m_tmTile.m_id = Exp_GetVal(ppArgs);     // extract id.
    
                        addTarget( CLIMODE_TARG_UNEXTRACT, "Where to place the extracted multi?", true );
                }
                break;
    
        case CV_GHOST:
                // Leave your body behind as an NPC.
                {
                        // Create the ghost. c_GHOST_WOMAN
                        CChar * pChar = CChar::CreateBasic( CREID_EQUIP_GM_ROBE );
                        if ( pChar == NULL )
                                return( false );
                        pChar->StatFlag_Set( STATF_Insubstantial );
                        pChar->MoveToChar( m_pChar->GetTopPoint());
    
                        // Switch bodies to it.
                        return Cmd_Control( pChar );
                }
                break;
    
        case CV_GMPAGE:
                m_Targ_Text = s.GetArgStr();
                addPromptConsole( CLIMODE_PROMPT_GM_PAGE_TEXT, "Describe your comment or problem" );
                break;
        case CV_GOTARG: // go to my (preselected) target.
                {
                        ASSERT(m_pChar);
                        CObjBase * pObj = m_Targ_UID.ObjFind();
                        if ( pObj != NULL )
                        {
                                CPointMap po = pObj->GetTopLevelObj()->GetTopPoint();
                                CPointMap pnt = po;
                                pnt.MoveN( DIR_W, 3 );
                                WORD wBlockFlags = m_pChar->GetMoveBlockFlags();
                                pnt.m_z = g_World.GetHeightPoint( pnt, wBlockFlags, NULL );     // ??? Get Area
                                m_pChar->m_dirFace = pnt.GetDir( po, m_pChar->m_dirFace ); // Face the player
                                m_pChar->Spell_Teleport( pnt, true, false );
                        }
                }
                break;
        case CV_HELP:
                if ( ! s.HasArgs()) // if no command, display the main help system dialog.
                {
                        Dialog_Setup( CLIMODE_DIALOG, g_Cfg.ResourceGetIDType( RES_DIALOG, IsPriv(PRIV_GM) ? "d_HELPGM" : "d_HEL
P" ), m_pChar );
                }
                else
                {
                        // Below here, we're looking for a command to get help on
                        int index = g_Cfg.m_HelpDefs.FindKey( s.GetArgStr());
                        if ( index >= 0 )
                        {
                                CResourceNamed * pHelpDef = STATIC_CAST <CResourceNamed*>(g_Cfg.m_HelpDefs);
                                ASSERT(pHelpDef);
                                CResourceLock s;
                                if ( pHelpDef->ResourceLock( s ))
                                {
                                        addScrollScript( s, SCROLL_TYPE_TIPS, 0, s.GetArgStr());
                                }
                        }
                }
                break;
        case CV_INFO:
                // We could also get ground tile info.
                addTarget( CLIMODE_TARG_OBJ_INFO, "What would you like info on?", true, false );
                break;
        case CV_INFORMATION:
                SysMessage( g_Serv.GetStatusString( 0x22 ));
                SysMessage( g_Serv.GetStatusString( 0x24 ));
                break;
        case CV_ITEMMENU:
                // cascade to another menu.
                goto do_menu;
        case CV_LAST:
                // Fake Previous target.
                if ( GetTargMode() >= CLIMODE_MOUSE_TYPE )
                {
                        ASSERT(m_pChar);
                        CObjBase * pObj = m_pChar->m_Act_Targ.ObjFind();
                        if ( pObj != NULL )
                        {
                                CEvent Event;
                                CPointMap pt = pObj->GetUnkPoint();
                                Event.Target.m_context = GetTargMode();
                                Event.Target.m_x = pt.m_x;
                                Event.Target.m_y = pt.m_y;
                                Event.Target.m_z = pt.m_z;
                                Event.Target.m_UID = pObj->GetUID();
                                Event.Target.m_id = 0;
                                Event_Target( &Event );
                        }
                        break;
                }
                return( false );
        case CV_LINK:   // link doors
                m_Targ_UID.InitUID();
                addTarget( CLIMODE_TARG_LINK, "Select the item to link." );
                break;
    
        case CV_LOGIN:
                {
                        // Try to login with name and password given.
                        CLogIP * pLogIP = GetLogIP();
                        if ( pLogIP == NULL )
                                return( false );
                        m_pAccount = NULL;      // this is odd ???
                        pLogIP->SetAccount( NULL );
    
                        TCHAR * ppArgs;
                        Str_ParseCmds( s.GetArgStr(), ppArgs, COUNTOF( ppArgs ));
                        LOGIN_ERR_TYPE nCode = LogIn( ppArgs, ppArgs, m_Targ_Text );
    
                        // Associate this with the CLogIP !
                        pLogIP->SetAccount( GetAccount());
                }
                break;
    
        case CV_LOGOUT:
                {
                        // Clear the account and the link to this CLogIP
                        CLogIP * pLogIP = GetLogIP();
                        if ( pLogIP == NULL )
                                return( false );
                        pLogIP->InitTimes();
                }
                break;
    
        case CV_MENU:
    do_menu:
                Menu_Setup( g_Cfg.ResourceGetIDType( RES_MENU, s.GetArgStr()));
                break;
        case CV_MIDI:
                goto do_music;
        case CV_MIDILIST:
    do_music:
                {
                        int piMidi;
                        int iQty = Str_ParseCmds( s.GetArgStr(), piMidi, COUNTOF(piMidi));
                        if ( iQty > 0 )
                        {
                                addMusic( piMidi );
                        }
                }
                break;
        case CV_MUSIC:
                goto do_music;
    
        case CV_NUDGE:
                if ( ! s.HasArgs())
                {
                        SysMessage( "Usage: NUDGE dx dy dz" );
                }
                else
                {
                        m_Targ_Text = s.GetArgRaw();
                        m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                        m_tmTile.m_Code = CV_NUDGE;
                        addTarget( CLIMODE_TARG_TILE, "Select area to Nudge", true );
                }
                break;
    
        case CV_NUKE:
                m_Targ_Text = s.GetArgRaw();
                m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                m_tmTile.m_Code = CV_NUKE;      // set nuke code.
                addTarget( CLIMODE_TARG_TILE, "Select area to Nuke", true );
                break;
        case CV_NUKECHAR:
                m_Targ_Text = s.GetArgRaw();
                m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                m_tmTile.m_Code = CV_NUKECHAR;  // set nuke code.
                addTarget( CLIMODE_TARG_TILE, "Select area to Nuke Chars", true );
                break;
    
        case CV_ONECLICK:
                m_Targ_Text = s.GetArgRaw();
                m_pChar->Skill_Start( (m_Targ_Text.IsEmpty()) ? SKILL_NONE : NPCACT_OneClickCmd );
                break;
    
        case CV_PAGE:
                Cmd_GM_PageCmd( s.GetArgStr());
                break;
        case CV_REPAIR:
                addTarget( CLIMODE_TARG_REPAIR, "What item do you want to repair?" );
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
        case CV_SCROLL:
                // put a scroll up.
                addScrollResource( s.GetArgStr(), SCROLL_TYPE_UPDATES );
                break;
        case CV_SELF:
                // Fake self target.
                if ( GetTargMode() >= CLIMODE_MOUSE_TYPE )
                {
                        ASSERT(m_pChar);
                        CEvent Event;
                        Event.Target.m_context = GetTargMode();
                        CPointMap pt = m_pChar->GetTopPoint();
                        Event.Target.m_x = pt.m_x;
                        Event.Target.m_y = pt.m_y;
                        Event.Target.m_z = pt.m_z;
                        Event.Target.m_UID = m_pChar->GetUID();
                        Event.Target.m_id = 0;
                        Event_Target(&Event);
                        break;
                }
                return( false );
        case CV_SHOWSKILLS:
                addSkillWindow( SKILL_QTY ); // Reload the real skills
                break;
    
        case CV_SKILLMENU:
                // Just put up another menu.
                return Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, s.GetArgStr()));
    
        case CV_STATIC:
                if ( s.HasArgs())
                {
                        Cmd_CreateItem( (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgStr()), true );
                }
                break;
        case CV_SUMMON: // from the spell skill script.
                // m_Targ_PrvUID should already be set.
                return Cmd_CreateChar( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgStr()), SPELL_Summon, true 
);
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
                        SysMessage( "Usage: TILE z-height item1 item2 itemX" );
                }
                else
                {
                        m_Targ_Text = s.GetArgStr(); // Point at the options
                        m_tmTile.m_ptFirst.InitPoint(); // Clear this first
                        m_tmTile.m_Code = CV_TILE;
                        addTarget( CLIMODE_TARG_TILE, "Pick 1st corner:", true );
                }
                break;
        case CV_VERSION:        // "SHOW VERSION"
                SysMessage( g_szServerDescription );
                break;
        case CV_WEBLINK:
                addWebLaunch( s.GetArgStr());
                break;
        default:
                DEBUG_CHECK(0);
                return( false );
        }
    
        return( true );
    }
    
    bool CClient::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
    {
        if ( r_GetRefSingle( pszKey, pRef ))
                return( true );
        return( CScriptObj::r_GetRef( pszKey, pRef ));
    }
    bool CClient::r_Verb( CScript & s, CTextConsole * pSrc )
    {
        if ( r_VerbSingle( s, pSrc ))
                return( true );
        return( CScriptObj::r_Verb( s, pSrc ));
    }
    bool CClient::r_WriteVal( LPCTSTR pszKey, CGString & s, CTextConsole * pSrc )
    {
        if ( r_WriteValSingle( pszKey, s, pSrc ))
                return( true );
        return( CScriptObj::r_WriteVal( pszKey, s, pSrc  ));
    }
    bool CClient::r_LoadVal( CScript & s )
    {
        if ( r_LoadValSingle( s ))
                return( true );
        return( CScriptObj::r_LoadVal( s ));
    }