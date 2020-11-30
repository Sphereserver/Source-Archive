    // CServerDef.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #include "graysvr.h"        // predef header.
    
    //////////////////////////////////////////////////////////////////////
    // -CServerDef
    
    CServerDef::CServerDef( LPCTSTR pszName, CSocketAddressIP dwIP ) :
        m_ip( dwIP, GRAY_DEF_PORT )     // SOCKET_LOCAL_ADDRESS
    {
        // Statistics.
        memset( m_dwStat, 0, sizeof( m_dwStat ));       // THIS MUST BE FIRST !
    
        SetName( pszName );
        m_timeLastPoll.Init();
        m_timeLastValid.Init();
        m_timeCreate = CServTime::GetCurrentTime();
        m_iClientsAvg = 0;
    
        // Set default time zone from UTC
        m_TimeZone = _timezone / (60*60);       // Greenwich mean time.
        m_eAccApp = ACCAPP_Unspecified;
    }
    
    void CServerDef::SetName( LPCTSTR pszName )
    {
        if ( ! pszName )
                return;
    
        // No HTML tags using <> either.
        TCHAR szName;
        int len = Str_GetBare( szName, pszName, sizeof(szName), "<>/\"\\" );
        if ( ! len )
                return;
    
        // allow just basic chars. No spaces, only numbers, letters and underbar.
        if ( g_Cfg.IsObscene( szName ))
        {
                DEBUG_ERR(( "Obscene server '%s' ignored.\n", szName ));
                return;
        }
    
        m_sName = szName;
    }
    
    void CServerDef::SetValidTime()
    {
        m_timeLastValid = CServTime::GetCurrentTime();
    }
    
    int CServerDef::GetTimeSinceLastValid() const
    {
        return( - g_World.GetTimeDiff( m_timeLastValid ));
    }
    
    void CServerDef::SetPollTime()
    {
        m_timeLastPoll = CServTime::GetCurrentTime();
    }
    
    int CServerDef::GetTimeSinceLastPoll() const
    {
        return( - g_World.GetTimeDiff( m_timeLastPoll ));
    }
    
    void CServerDef::ClientsResetAvg()
    {
        // Match the avg closer to the real current count.
    
    #if 0
        int iClientsNew = StatGet(SERV_STAT_CLIENTS);
        if ( iClientsNew < m_iClientsAvg )
        {
                iClientsNew += ( m_iClientsAvg - iClientsNew ) / 2;
        }
        m_iClientsAvg = iClientsNew;
    #endif
    
    }
    
    void CServerDef::addToServersList( CCommand & cmd, int index, int j ) const
    {
        // Add myself to the server list.
        cmd.ServerList.m_serv.m_count = index;
    
        //pad zeros to length.
        strcpylen( cmd.ServerList.m_serv.m_servname, GetName(), sizeof(cmd.ServerList.m_serv.m_servname));
    
        cmd.ServerList.m_serv.m_zero32 = 0;
        cmd.ServerList.m_serv.m_percentfull = StatGet(SERV_STAT_CLIENTS);
        cmd.ServerList.m_serv.m_timezone = m_TimeZone;  // GRAY_TIMEZONE
    
        DWORD dwAddr = m_ip.GetAddrIP();
        cmd.ServerList.m_serv.m_ip = ( dwAddr >> 24 ) & 0xFF;
        cmd.ServerList.m_serv.m_ip = ( dwAddr >> 16 ) & 0xFF;
        cmd.ServerList.m_serv.m_ip = ( dwAddr >> 8  ) & 0xFF;
        cmd.ServerList.m_serv.m_ip = ( dwAddr       ) & 0xFF;
    }
    
    enum SC_TYPE
    {
        SC_ACCAPP,
        SC_ACCAPPS,
        SC_ACCOUNTS,
        SC_ADMIN,
        SC_ADMINEMAIL,  //      m_sEMail
        SC_AGE,
        SC_ALLOCS,
        SC_CHARS,
        SC_CLIENTS,
        SC_CLIENTSAVG,
        SC_CLIENTVER,
        SC_CLIENTVERSION,
        SC_CLIVER,
        SC_CREATE,
        SC_EMAIL,
        SC_EMAILLINK,
        SC_IP,
        SC_ITEMS,
        SC_LANG,
        SC_LASTPOLLTIME,
        SC_LASTVALIDDATE,
        SC_LASTVALIDTIME,
        SC_MEM,
        SC_NAME,
        SC_NOTES,
        SC_PORT,
        SC_REGPASS,
        SC_SERVIP,
        SC_SERVNAME,
        SC_SERVPORT,
        SC_STATACCOUNTS,
        SC_STATCLIENTS,
        SC_STATITEMS,
        SC_STATMEMORY,
        SC_STATNPCS,
        SC_STATUS,
        SC_TIMEZONE,
        SC_TZ,
        SC_URL,                 // m_sURL
        SC_URLLINK,
        SC_VALID,
        SC_VER,
        SC_VERSION,
        SC_QTY,
    };
    
    LPCTSTR const CServerDef::sm_szLoadKeys =   // static
    {
        "ACCAPP",
        "ACCAPPS",
        "ACCOUNTS",
        "ADMIN",
        "ADMINEMAIL",   //      m_sEMail
        "AGE",
        "ALLOCS",
        "CHARS",
        "CLIENTS",
        "CLIENTSAVG",
        "CLIENTVER",
        "CLIENTVERSION",
        "CLIVER",
        "CREATE",
        "EMAIL",
        "EMAILLINK",
        "IP",
        "ITEMS",
        "LANG",
        "LASTPOLLTIME",
        "LASTVALIDDATE",
        "LASTVALIDTIME",
        "MEM",
        "NAME",
        "NOTES",
        "PORT",
        "REGPASS",
        "SERVIP",
        "SERVNAME",
        "SERVPORT",
        "STATACCOUNTS",
        "STATCLIENTS",
        "STATITEMS",
        "STATMEMORY",
        "STATNPCS",
        "STATUS",
        "TIMEZONE",
        "TZ",
        "URL",                  // m_sURL
        "URLLINK",
        "VALID",
        "VER",
        "VERSION",
        NULL,
    };
    
    static LPCTSTR const sm_AccAppTable =
    {
        "Closed",               // Closed. Not accepting more.
        "EmailApp",             // Must send email to apply.
        "Free",                 // Anyone can just log in and create a full account.
        "GuestAuto",    // You get to be a guest and are automatically sent email with u're new password.
        "GuestTrial",   // You get to be a guest til u're accepted for full by an Admin.
        "Other",                // specified but other ?
        "Unspecified",  // Not specified.
        "WebApp",               // Must fill in a web form and wait for email response
        "WebAuto",              // Must fill in a web form and automatically have access
    };
    
    void CServerDef::r_DumpLoadKeys( CTextConsole * pSrc )
    {
        r_DumpKeys(pSrc,sm_szLoadKeys);
        CScriptObj::r_DumpLoadKeys(pSrc);
    }
    
    bool CServerDef::r_LoadVal( CScript & s )
    {
        switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
        {
        case SC_ACCAPP:
        case SC_ACCAPPS:
                // Treat it as a value or a string.
                if ( isdigit( s.GetArgStr() ))
                {
                        m_eAccApp = (ACCAPP_TYPE) s.GetArgVal();
                }
                else
                {
                        // Treat it as a string. "Manual","Automatic","Guest"
                        m_eAccApp = (ACCAPP_TYPE) FindTableSorted(  s.GetArgStr(), sm_AccAppTable, COUNTOF( sm_AccAppTable ));
                }
                if ( m_eAccApp < 0 || m_eAccApp >= ACCAPP_QTY )
                        m_eAccApp = ACCAPP_Unspecified;
                break;
        case SC_ALLOCS:
                SetStat( SERV_STAT_ALLOCS, s.GetArgVal());
                break;
        case SC_AGE:
                break;
        case SC_CLIENTSAVG:
                {
                m_iClientsAvg = s.GetArgVal();
                if ( m_iClientsAvg < 0 )
                        m_iClientsAvg = 0;
                if ( m_iClientsAvg > FD_SETSIZE )       // Number is bugged !
                        m_iClientsAvg = FD_SETSIZE;
                }
                break;
        case SC_CLIENTVERSION:
        case SC_CLIVER:
                m_ClientVersion.SetClientVer( s.GetArgStr());
                break;
        case SC_CREATE:
                m_timeCreate = CServTime::GetCurrentTime() + ( s.GetArgVal() * TICK_PER_SEC );
                break;
        case SC_ADMIN:
        case SC_ADMINEMAIL:
        case SC_EMAIL:
        case SC_EMAILLINK:
                if ( this != &g_Serv &&
                        ! g_Serv.m_sEMail.IsEmpty() &&
                        strstr( s.GetArgStr(), g_Serv.m_sEMail ))
                        return( false );
                if ( ! g_Cfg.IsValidEmailAddressFormat( s.GetArgStr()))
                        return( false );
                if ( g_Cfg.IsObscene( s.GetArgStr()))   // Is the name unacceptable?
                        return( false );
                m_sEMail = s.GetArgStr();
                break;
        case SC_LANG:
                {
                        TCHAR szLang;
                        int len = Str_GetBare( szLang, s.GetArgStr(), sizeof(szLang), "<>/\"\\" );
                        if ( g_Cfg.IsObscene(szLang))   // Is the name unacceptable?
                                return( false );
                        m_sLang = szLang;
                }
                break;
        case SC_LASTPOLLTIME:
                m_timeLastPoll = CServTime::GetCurrentTime() + ( s.GetArgVal() * TICK_PER_SEC );
                break;
        case SC_LASTVALIDDATE:
                m_dateLastValid.Read( s.GetArgStr());
                break;
        case SC_LASTVALIDTIME:
                {
                        int iVal = s.GetArgVal() * TICK_PER_SEC;
                        if ( iVal < 0 )
                                m_timeLastValid = CServTime::GetCurrentTime() + iVal;
                        else
                                m_timeLastValid = CServTime::GetCurrentTime() - iVal;
                }
                break;
        case SC_NOTES:
                // Make sure this is not too long !
                // make sure there are no bad HTML tags in here ?
                {
                        TCHAR szTmp;
                        int len = Str_GetBare( szTmp, s.GetArgStr(), COUNTOF(szTmp), "<>/" );   // no tags
                        if ( g_Cfg.IsObscene( szTmp ))  // Is the name unacceptable?
                                return( false );
                        m_sNotes = szTmp;
                }
                break;
        case SC_REGPASS:
                m_sRegisterPassword = s.GetArgStr();
                break;
        case SC_IP:
        case SC_SERVIP:
                m_ip.SetHostPortStr( s.GetArgStr());
                break;
    
        case SC_NAME:
        case SC_SERVNAME:
                SetName( s.GetArgStr());
                break;
        case SC_PORT:
        case SC_SERVPORT:
                m_ip.SetPort( s.GetArgVal());
                break;
    
        case SC_ACCOUNTS:
        case SC_STATACCOUNTS:
                SetStat( SERV_STAT_ACCOUNTS, s.GetArgVal());
                break;
    
        case SC_CLIENTS:
        case SC_STATCLIENTS:
                {
                        int iClients = s.GetArgVal();
                        if ( iClients < 0 )
                                return( false );        // invalid
                        if ( iClients > FD_SETSIZE )    // Number is bugged !
                                return( false );
                        SetStat( SERV_STAT_CLIENTS, iClients );
                        if ( iClients > m_iClientsAvg )
                                m_iClientsAvg = StatGet(SERV_STAT_CLIENTS);
                }
                break;
        case SC_ITEMS:
        case SC_STATITEMS:
                SetStat( SERV_STAT_ITEMS, s.GetArgVal());
                break;
        case SC_MEM:
        case SC_STATMEMORY:
                SetStat( SERV_STAT_MEM, s.GetArgVal() * 1024 );
                break;
        case SC_CHARS:
        case SC_STATNPCS:
                SetStat( SERV_STAT_CHARS, s.GetArgVal());
                break;
        case SC_STATUS:
                return( ParseStatus( s.GetArgStr(), true ));
        case SC_TIMEZONE:
        case SC_TZ:
                m_TimeZone = s.GetArgVal();
                break;
        case SC_URL:
        case SC_URLLINK:
                // It is a basically valid URL ?
                if ( this != &g_Serv )
                {
                        if (! g_Serv.m_sURL.IsEmpty() &&
                                strstr( s.GetArgStr(), g_Serv.m_sURL ))
                                return( false );
                }
                if ( g_Cfg.m_sMainLogServerDir.IsEmpty() || this != &g_Serv )
                {
                        if ( ! strnicmp( s.GetArgStr(), "www.menasoft.com", 15 ))
                                return( false );
                        if ( ! strnicmp( s.GetArgStr(), "www.sphereserver.com", 19 ))
                                return( false );
                }
                if ( ! strchr( s.GetArgStr(), '.' ))
                        return( false );
                if ( g_Cfg.IsObscene( s.GetArgStr()))   // Is the name unacceptable?
                        return( false );
                m_sURL = s.GetArgStr();
                break;
    
        case SC_VALID:
                // Force it to be valid. (not sure why i'd want to do this)
                SetValidTime();
                m_dateLastValid = CGTime::GetCurrentTime();
                m_timeLastPoll = (s.GetArgVal())? m_timeLastValid : CServTime::GetCurrentTime() ;
                break;
    
        case SC_VER:
        case SC_VERSION:
                {
                        TCHAR szVer;
                        int len = Str_GetBare( szVer, s.GetArgStr(), sizeof(szVer), "<>/\"\\" );
                        if ( g_Cfg.IsObscene(szVer))    // Is the name unacceptable?
                                return( false );
                        m_sServVersion = szVer;
                }
                break;
    
        default:
                return( CScriptObj::r_LoadVal(s));
        }
        return( true );
    }
    
    bool CServerDef::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
    {
        switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
        {
        case SC_ACCAPP:
                sVal.FormatVal( m_eAccApp );
                break;
        case SC_ACCAPPS:
                // enum string
                ASSERT( m_eAccApp >= 0 && m_eAccApp < ACCAPP_QTY );
                sVal = sm_AccAppTable;
                break;
        case SC_ADMINEMAIL:
        case SC_ADMIN:
        case SC_EMAIL:
                sVal = m_sEMail;
                break;
        case SC_AGE:
                // display the age in days.
                sVal.FormatVal( GetAgeHours()/24 );
                break;
        case SC_ALLOCS:
                sVal.FormatVal( StatGet( SERV_STAT_ALLOCS ));
                break;
        case SC_CLIENTSAVG:
                sVal.FormatVal( GetClientsAvg());
                break;
        case SC_CLIENTVERSION:
        case SC_CLIENTVER:
        case SC_CLIVER:
                {
                        TCHAR szVersion;
                        sVal = m_ClientVersion.WriteClientVer( szVersion );
                }
                break;
        case SC_CREATE:
                sVal.FormatVal( g_World.GetTimeDiff(m_timeCreate) / TICK_PER_SEC );
                break;
        case SC_EMAILLINK:
                if ( m_sEMail.IsEmpty())
                {
                        sVal.Empty();
                        break;
                }
                sVal.Format( "<a href=\"mailto:%s\">%s</a>", (LPCTSTR) m_sEMail, (LPCTSTR) m_sEMail );
                break;
    
        case SC_LANG:
                sVal = m_sLang;
                break;
    
        case SC_LASTPOLLTIME:
                sVal.FormatVal( m_timeLastPoll.IsTimeValid() ? ( g_World.GetTimeDiff(m_timeLastPoll) / TICK_PER_SEC ) : -1 );
                break;
        case SC_LASTVALIDDATE:
                //if ( m_dateLastValid.IsValid())
                        //sVal = m_dateLastValid.Write();
    
                if ( m_timeLastValid.IsTimeValid() )
                        sVal.FormatVal( GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 ));
                else
                        sVal = "NA";
                break;
        case SC_LASTVALIDTIME:
                // How many seconds ago.
                sVal.FormatVal( m_timeLastValid.IsTimeValid() ? ( GetTimeSinceLastValid() / TICK_PER_SEC ) : -1 );
                break;
        case SC_NOTES:
                sVal = m_sNotes;
                break;
        case SC_REGPASS:
                if ( pSrc == NULL || pSrc->GetPrivLevel() < PLEVEL_Admin )
                        return( false );
                sVal = m_sRegisterPassword;
                break;
        case SC_IP:
        case SC_SERVIP:
                sVal = m_ip.GetAddrStr();
                break;
        case SC_NAME:
        case SC_SERVNAME:
                sVal = GetName();       // What the name should be. Fill in from ping.
                break;
        case SC_PORT:
        case SC_SERVPORT:
                sVal.FormatVal( m_ip.GetPort());
                break;
        case SC_ACCOUNTS:
        case SC_STATACCOUNTS:
                sVal.FormatVal( StatGet( SERV_STAT_ACCOUNTS ));
                break;
        case SC_CLIENTS:
        case SC_STATCLIENTS:
                sVal.FormatVal( StatGet( SERV_STAT_CLIENTS ));
                break;
        case SC_ITEMS:
        case SC_STATITEMS:
                sVal.FormatVal( StatGet( SERV_STAT_ITEMS ));
                break;
        case SC_MEM:
        case SC_STATMEMORY:
                sVal.FormatVal( StatGet( SERV_STAT_MEM )/1024 );
                break;
        case SC_CHARS:
        case SC_STATNPCS:
                sVal.FormatVal( StatGet( SERV_STAT_CHARS ));
                break;
        case SC_STATUS:
                sVal = m_sStatus;
                break;
        case SC_TIMEZONE:
        case SC_TZ:
                sVal.FormatVal( m_TimeZone );
                break;
        case SC_URL:
                sVal = m_sURL;
                break;
        case SC_URLLINK:
                // try to make a link of it.
                if ( m_sURL.IsEmpty())
                {
                        sVal = GetName();
                        break;
                }
                sVal.Format( "<a href=\"http://%s\">%s</a>", (LPCTSTR) m_sURL, (LPCTSTR) GetName() );
                break;
        case SC_VALID:
                sVal.FormatVal( IsConnected());
                break;
        case SC_VER:
        case SC_VERSION:
                sVal = m_sServVersion;
                break;
        default:
                return( CScriptObj::r_WriteVal( pszKey, sVal, pSrc ));
        }
        return( true );
    }
    
    void CServerDef::r_WriteData( CScript & s )
    {
        if ( ! m_ip.IsLocalAddr())
        {
                s.WriteKey( "IP", m_ip.GetAddrStr());
        }
        if ( m_ip.GetPort() != GRAY_DEF_PORT )
        {
                s.WriteKeyVal( "PORT", m_ip.GetPort());
        }
        if ( m_TimeZone != ( _timezone / (60*60)))
        {
                s.WriteKeyVal( "TZ", m_TimeZone );
        }
        if ( ! m_sURL.IsEmpty())
        {
                s.WriteKey( "URL", m_sURL );
        }
        if ( ! m_sEMail.IsEmpty())
        {
                s.WriteKey( "EMAIL", m_sEMail );
        }
        if ( ! m_sRegisterPassword.IsEmpty())
        {
                s.WriteKey( "REGPASS", m_sRegisterPassword );
        }
        if ( ! m_sNotes.IsEmpty())
        {
                s.WriteKey( "NOTES", m_sNotes );
        }
        if ( ! m_sLang.IsEmpty())
        {
                s.WriteKey( "LANG", m_sLang );
        }
        if ( m_eAccApp != ACCAPP_Unspecified )
        {
                ASSERT( m_eAccApp >= 0 && m_eAccApp < ACCAPP_QTY );
                s.WriteKey( "ACCAPPS", sm_AccAppTable );
        }
    
        // Statistical stuff about other server.
        if ( this != &g_Serv )
        {
                s.WriteKeyVal( "CREATE", g_World.GetTimeDiff(m_timeCreate) / TICK_PER_SEC );
                s.WriteKeyVal( "LASTVALIDTIME", GetTimeSinceLastValid() / TICK_PER_SEC );
                s.WriteKey( "LASTVALIDDATE", m_dateLastValid.Format(NULL) );
                if ( StatGet(SERV_STAT_ACCOUNTS))
                {
                        s.WriteKeyVal( "ACCOUNTS", StatGet(SERV_STAT_ACCOUNTS) );
                }
                if ( m_iClientsAvg )
                {
                        s.WriteKeyVal( "CLIENTSAVG", m_iClientsAvg );
                }
                if ( ! m_sServVersion.IsEmpty())
                {
                        s.WriteKey( "VER", m_sServVersion );
                }
        }
    }
    
    void CServerDef::r_WriteCreated( CScript & s )
    {
        if ( ! m_timeCreate.IsTimeValid() )
                return;
        s.WriteSection( "SERVER %s", GetName() );
        r_WriteData(s);
    }
    
    bool CServerDef::ParseStatus( LPCTSTR pszStatus, bool fStore )
    {
        // Take the status string we get from the server and interpret it.
        // fStore = set the Status msg.
    
        ASSERT( this != &g_Serv );
        m_dateLastValid = CGTime::GetCurrentTime();
        SetValidTime(); // this server seems to be alive.
    
        TCHAR bBareData;
        int len = Str_GetBare( bBareData, pszStatus, sizeof(bBareData)-1 );
        if ( ! len )
        {
                return false;
        }
    
        CScriptFileContext ScriptContext( &g_Cfg.m_scpIni ); // for error reporting.
    
        // Parse the data we get. Older versions did not have , delimiters
        TCHAR * pData = bBareData;
        while ( pData )
        {
                TCHAR * pEquals = strchr( pData, '=' );
                if ( pEquals == NULL )
                        break;
    
                // Start of argument.
                *pEquals = '\0';
                pEquals++;
    
                TCHAR * pszKey = strrchr( pData, ' ' ); // find start of key. reversed
                if ( pszKey == NULL )
                        pszKey = pData;
                else
                        pszKey++;
    
                if ( pEquals == '"' )
                {
                        pEquals++;
                        pData = strchr( pEquals, '"' );
                        if ( pData == NULL )
                                break;
                        *pData = '\0';
                        pData++;
                        if ( pData == ',' )
                                pData ++;
                }
                else
                {
                        pData = strchr( pEquals, ',' );
                        if ( pData )
                        {
                                TCHAR * pEnd = pData;
                                pData ++;
                                while ( ISWHITESPACE( pEnd ))
                                        pEnd--;
                                *pEnd = '\0';
                        }
                }
    
                r_SetVal( pszKey, pEquals );
        }
    
        if ( fStore )
        {
                m_sStatus = "OK";
        }
    
        return( true );
    }
    
    int CServerDef::GetAgeHours() const
    {
        // This is just the amount of time it has been listed.
        return(( - g_World.GetTimeDiff( m_timeCreate )) / ( TICK_PER_SEC * 60 * 60 ));
    }
    
    bool CServerDef::IsValidStatus() const
    {
        // Should this server be listed at all ?
        // Drop it from the list ?
    
        if ( this == &g_Serv )  // we are always a valid server.
                return( true );
        if ( ! m_timeCreate.IsTimeValid())
                return( true ); // Never delete this. it was defined in the *.INI file
        if ( ! m_timeLastValid.IsTimeValid() && ! m_timeLastPoll.IsTimeValid())
                return( true ); // it was defined in the *.INI file. but has not updated yet
    
        // Give the old reliable servers a break.
        DWORD dwAgeHours = GetAgeHours();
    
        // How long has it been down ?
        DWORD dwInvalidHours = GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 * 60 );
    
        return( dwInvalidHours <= (7*24) + ( dwAgeHours/24 ) * 6 );
    }
    
    void CServerDef::SetStatusFail( LPCTSTR pszTrying )
    {
        ASSERT(pszTrying);
        m_sStatus.Format( "Failed: %s: down for %d minutes",
                pszTrying,
                GetTimeSinceLastValid() / ( TICK_PER_SEC * 60 ));
    }
    
    bool CServerDef::PollStatus()
    {
        // Poll for the status of this server.
        // Try to ping the server to find out what it's status is.
        // NOTE: 
        //   This is a synchronous function that could take a while. do in new thread.
        // RETURN: 
        //  false = failed to respond
    
        ASSERT( this != &g_Serv );
    
        bool fWasUpToDate = IsConnected();
        SetPollTime();          // record that i tried to connect.
    
        CGSocket sock;
        if ( ! sock.Create())
        {
                SetStatusFail( "Socket" );
                return( false );
        }
        if ( sock.Connect( m_ip ))
        {
                SetStatusFail( "Connect" );
                return( false );
        }
    
        char bData = 0x21;      // GetStatusString arg data.
        if ( fWasUpToDate && Calc_GetRandVal( 16 ))
        {
                // just ask it for stats. don't bother with header info.
                bData = 0x23;
        }
    
        // got a connection
        // Ask for the data.
        if ( sock.Send( &bData, 1 ) != 1 )
        {
                SetStatusFail( "Send" );
                return( false );
        }
    
        // Wait for some sort of response.
        struct fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock.GetSocket(), &readfds);
    
        timeval Timeout;        // time to wait for data.
        Timeout.tv_sec=10;      // wait for 10 seconds for a response.
        Timeout.tv_usec=0;
        int ret = select( sock.GetSocket()+1, &readfds, NULL, NULL, &Timeout );
        if ( ret <= 0 )
        {
                SetStatusFail( "No Response" );
                return( false );
        }
    
        // Any events from clients ?
        if ( ! FD_ISSET( sock.GetSocket(), &readfds ))
        {
                SetStatusFail( "Timeout" );
                return( false );
        }
    
        // No idea how much data i really have here.
        char bRetData;
        int len = sock.Receive( bRetData, sizeof( bRetData ) - 1 );
        if ( len <= 0 )
        {
                SetStatusFail( "Receive" );
                return( false );
        }
    
        CThreadLockRef lock( this );
        if ( ! ParseStatus( bRetData, bData == 0x21 ))
        {
                SetStatusFail( "Bad Status Data" );
                return( false );
        }
    
        return( true );
    }
    
    bool CServerDef::QueueCharToServer( CChar * pChar )
    {
        // Move this char to this server.
        // Or at least try to.
        // RETURN: 
        //  false = this server was not responding last i checked.
    
        // Thread lock myself.
        CThreadLockRef lock( this );
        if ( pChar )
        {
                // Does not matter if we are already queued.
                // Set up this char to be moved to the remote server.
                m_MoveChars.AttachChar( pChar );
    
                // we should freeze the char or something to hold them from doing bad stuff in the mean time.
        }
        else
        {
                // anything in the queue now ?
                if ( ! m_MoveChars.GetCharCount())
                        return false;
        }
    
        // Make sure the mover thread is started.
        if ( m_TaskCharMove.IsActive())
                return true;
        if ( g_Serv.IsLoading())        // can't do this just now.
                return true;
        m_TaskCharMove.CreateThread( MoveCharEntryProc, this );
        return true;
    }
    
    int CServerDef::MoveCharToServer( CGSocket &sock, CChar * pChar )
    {
        // Delete the char off this server ? or just hold it till later ?
        // RETURN: 
        //  0 = ok,
        //  1 = char was rejected.
        //  2 = failure of some sort.
    
    #if 0
        if ( pChar->IsClient())
        {
                // Send a message to the game thread to tell the client to move to it's new server !
                // If they have logged off or don't get it. no big deal.
    
                pChar->GetClient()->addRelay( this );
        }
    
    #endif
    
        return( 0 );
    }
    
    bool CServerDef::MoveCharsToServer()
    {
        // Move any queued chars to the server.
        // This is done on a seperate thread.
    
        ASSERT( this != &g_Serv );
    
        CThreadLockRef lock( this );
        m_TaskCharMove.OnCreate();
        ASSERT(m_MoveChars.GetCharCount());
    
        SetPollTime();  // record that i tried to connect.
    
        CGSocket sock;
        if ( ! sock.Create())
        {
                SetStatusFail( "Socket" );
                return( false );
        }
    
        lock.ReleaseRef();      // release the ref while we are waiting to connect.
        if ( sock.Connect( m_ip ))
        {
                SetStatusFail( "Connect" );
                return( false );
        }
    
        // Now log into the remote server.
    
        lock.SetRef(this);
    
        int iRet = 0;
    
        while ( ! g_Serv.IsLoading() && m_MoveChars.GetCharCount())
        {
                CChar * pChar = m_MoveChars.GetChar(0).CharFind();
                if ( pChar != NULL )
                {
                        iRet = MoveCharToServer( sock, pChar );
                        if ( iRet )
                        {
                                // not responding i would assume ?
                                // or the char was just not accepted?
                                break;
                        }
                }
                m_MoveChars.DetachChar(0);
        }
    
        if ( iRet <= 1 )
        {
                m_dateLastValid = CGTime::GetCurrentTime();
                SetValidTime(); // this server seems to be alive.
                m_sStatus = "OK";
        }
    
        m_TaskCharMove.OnClose();
        return( true );
    }
    
    THREAD_ENTRY_RET _cdecl CServerDef::MoveCharEntryProc( void * lpThreadParameter ) // static
    {
        CServerDef * pServ = (CServerDef *) lpThreadParameter;
        pServ->MoveCharsToServer();
    }