    // GRAYSRV.CPP.
    // Copyright Menace Software (www.menasoft.com).
    //
    // game/login server for uo client
    // http://www.menasoft.com for more details.
    // I'm liking http://www.cs.umd.edu/users/cml/cstyle/CppCodingStandard.html as a coding standard.
    //
    //  Dennis Robinson, http://www.menasoft.com
    //  Nix, Scripts
    //  Westy, /INFO, Maps
    //  John Davey (Garion)(The Answer Guy) docs.web pages.
    //  Philip Esterle (PAE)(Altiara Dragon) tailoring,weather,moons,LINUX
    //
    // UNKNOWN STATUS:
    //  Will Merkins  (Smeg) LINUX
    //  Mans Sjoberg (Allmight)
    //  Gerald D. Anderson (ROK) mailing list
    //  Kyle Jessup (KRJ) Line of sight.
    //  ZARN Brad Patten,
    //  Keif Mayers (Avernus)
    //  Damian Tedrow , http://www.ayekan.com/awesomedev
    //  Alluvian, Charles Manick Livermore
    //
    // Current:
    //  Casting penalties for plate
    //  Unconscious state.
    //  vendor economy.
    //  NPC AI
    //  Interserver registration.
    //
    // Top Issues:
    //  new mining/smelting system. more than 1 ingot for smelting 1 ore.
    //  weapon table for speed and damage. DEX for speed.
    //
    // Wish list:
    //  chop sound.
    //  inscribe magic weapons, wands from menu
    //  Magery skill degradation for plate and weapons in hand.
    //  Skill sticking. set a skill so it will stick at a value and not degrade.
    //  NPC smith repair.
    //  GrandMaster Created items are better and labled with name of the creator.
    //  Portrait of PC. or profile support
    //  Dex as part of AR ?
    //  Stable limits are per player/account based.
    //
    // Protocol Issues:
    // 2. placing an item like a forge or any other deeded item. (xcept houses and ships, i know how to do those.)
    // 9. grave stone and sign gumps ?
    // 4. Watching someone else drag an item from a container on the ground to their pack. and visa versa.
    // 5. Watching someone else drag an item from a pack to the ground. and visa versa.
    // 6. Adding a person to the party system.
    // x. Looking through all the gump menus for the house.
    // x. casting a gate spell. ok to watch someone else do it.
    // x. Bring up a player profile
    // x. Chop a tree. (i want to get the proper sound)
    //
    // BUGS:
    //   npc sheild item. block npc's from walking here .
    //   bless > 100 will lose stats
    //   account update before savbe ?
    //
    // 2. Hiding does not work right.
    //  You are still attacked by monsters when hidden.
    // 3. Fletching does not work right.
    //  If you have 100 shafts and 100 feathers and you fail to make arrow/bolts you lose everything.
    // 4. Combat is EXTREMELY SLOW.
    // 5. There are some truly Physotic Monsters out there.
    //  For example: There are these archers that look like Orcish Lords that shot 4 - 5 arrows at a time.
    //  So by the time you swing at them once you have 15 - 20 arrows in ya and your TOAST.
    // 6. Targeting after casting.
    //  If you are attacking a monster with a sword and cast a spell you lose your target. You just stand
    //  there while the monster rips you a new asshole until you 2x click and attack him again.
    //
    // ADDNPC from scripts, "CONT" keyword
    
    #ifdef VISUAL_SPHERE
        #include "..\VisualSphere\stdafx.h"
        #include "..\VisualSphere\VisualSphere.h"
        #include "..\VisualSphere\ServerObject.h"
    #else
        #include "graysvr.h"    // predef header.
    #endif
    #include "CIRCServer.h"
    #ifdef _WIN32
    #include "ntservice.h"      // g_Service
    #endif
    
    LPCTSTR const g_Stat_Name = // not sorted obviously.
    {
        "STR",
        "INT",
        "DEX",
        "KARMA",
        "FAME",
    
        // "OSTR",
        // "OINT",
        // "ODEX",
        // "MAXHITS",
        // "MAXMANA",
        // "MAXSTAM",
    };
    
    const CPointMap g_pntLBThrone(1323,1624,0); // This is origin for degree, sextant minute coordinates
    
    LPCTSTR g_szServerDescription =
        GRAY_TITLE " TEST Version " GRAY_VERSION " "
    #ifdef _WIN32
        ""
    #else
    #ifdef _BSD
        ""
    #else
        ""
    #endif
    #endif
    #ifdef _DEBUG
        ""
    #endif
        " by " GRAY_URL;
    
    int CObjBase::sm_iCount = 0;        // UID table.
    
    // game servers stuff.
    CWorld              g_World;        // the world. (we save this stuff)
    CServer             g_Serv; // current state stuff not saved.
    CResource   g_Cfg;
    CMainTask   g_MainTask;
    CBackTask   g_BackTask;
    CGrayInstall g_Install;
    CVerDataMul g_VerData;
    CExpression g_Exp;  // Global script variables.
    CLog                g_Log;
    CEventLog * g_pLog = &g_Log;
    CAccounts   g_Accounts;     // All the player accounts. name sorted CAccount
    
    //////////////////////////////////////////////////////////////////
    // util type stuff.
    
    //
    // Provide these functions globally.
    //
    
    class CGrayAssert : public CGrayError
    {
    protected:
        LPCTSTR const m_pExp;
        LPCTSTR const m_pFile;
        const unsigned m_uLine;
    public:
        CGrayAssert( LOGL_TYPE eSeverity, LPCTSTR pExp, 
                LPCTSTR pFile, unsigned uLine ) :
                CGrayError( eSeverity, 0, _TEXT("Assert")),
                m_pExp(pExp), m_pFile(pFile), m_uLine(uLine)
        {
        }
        virtual ~CGrayAssert()
        {
        }
        virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError, UINT * pnHelpContext )
        {
                sprintf( lpszError, "Assert pri=%d:'%s' file '%s', line %d",
                        m_eSeverity, m_pExp, m_pFile, m_uLine );
                return( true );
        }
    };
    
    #define MEMORY_ALLOC_SIGNATURE      0xBADF00D
    #ifdef _DEBUG
    size_t DEBUG_ValidateAlloc( const void * pThis )
    {
        // make sure the dynamic allocation below is valid.
        pThis = ( const BYTE*) pThis - sizeof(size_t);
        size_t stAllocateBlock = *((size_t*)pThis);
        ASSERT( stAllocateBlock != -1 );
        ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );
        return( stAllocateBlock );
    }
    
    void Debug_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
    {
        g_Log.Event( LOGL_ERROR, "Check Fail:'%s' file '%s', line %d" DEBUG_CR, pExp, pFile, uLine );
    }
    
    void Debug_Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
    {
        throw CGrayAssert( LOGL_CRIT, pExp, pFile, uLine );
    }
    #endif      // _DEBUG
    
    void Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
    {
        // These get left in release code .
        throw CGrayAssert( LOGL_CRIT, pExp, pFile, uLine );
    }
    
    #if defined(_WIN32) && defined(_CONSOLE)
    
    BOOL WINAPI ConsoleHandlerRoutine( DWORD dwCtrlType )
    {
        //  control signal type. CTRL_C_EVENT
        // SetConsoleCtrlHandler
        switch ( dwCtrlType )
        {
        case CTRL_C_EVENT: // A CTRL+c signal was received, either from keyboard input or from a signal generated by the Generat
eConsoleCtrlEvent function.
        case CTRL_BREAK_EVENT: //  A CTRL+BREAK signal was received, either from keyboard input or from a signal generated by Ge
nerateConsoleCtrlEvent.
        case CTRL_CLOSE_EVENT: // A signal that the system sends to all processes attached to a console when the user closes the
 console (either by choosing the Close command from the console window's System menu, or by choosing the End Task command from t
he Task List).
                if ( g_Cfg.m_fSecure && ! g_Serv.IsLoading())   // enable the try code.
                {
    #if 0
                        // this never worked.
                        g_Serv.SysMessage( "Press 'y' to exit safely, 'x' to exit immediatly." );
                        while (true)
                        {
                                int ch = _getch();
                                if ( ch == 0 )
                                        continue;
                                ch = toupper( ch );
                                if ( ch == 'X' )
                                        return( FALSE );
                                if ( ch != 'Y' )
                                        return( TRUE );
                                break;
                        }
    #endif
                        g_Serv.SetExitFlag( 3 );
                        return( TRUE );
                }
                break;
        case CTRL_LOGOFF_EVENT: // A signal that the system sends to all console processes when a user is logging off. This sign
al does not indicate which user is logging off, so no assumptions can be made.
        case CTRL_SHUTDOWN_EVENT: // A signal that the system sends to all console processes when the system is shutting down
                break;
        }
        return FALSE;   // process normally.
    }
    
    #endif      // _CONSOLE
    
    #ifdef _WIN32
    
    class CGrayException : public CGrayError
    {
        // Catch and get details on the system exceptions.
        // NULL pointer access etc.
    public:
        const DWORD m_dwAddress;
    public:
        CGrayException( unsigned int uCode, DWORD dwAddress ) :
                m_dwAddress( dwAddress ),
                CGrayError( LOGL_CRIT, uCode, _TEXT("Exception"))
        {
        }
        virtual ~CGrayException()
        {
        }
        virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError, UINT * pnHelpContext )
        {
                sprintf( lpszError, "Exception code=0%0x, addr=0%0x", 
                        m_hError, m_dwAddress );
                return( true );
        }
    };
    
    extern "C" {
    
    void _cdecl Sphere_Exception_Win32( unsigned int id, struct _EXCEPTION_POINTERS* pData )
    {
        // WIN32 gets an exception.
        // id = 0xc0000094 for divide by zero.
        // STATUS_ACCESS_VIOLATION is 0xC0000005.
    
        DWORD dwCodeStart = (DWORD)(BYTE *) &globalstartsymbol; // sync up to my MAP file.
    #ifdef _DEBUG
        // NOTE: This value is not accurate for some stupid reason.
        dwCodeStart += 0x06d40; // no idea why i have to do this.
    #endif
        //      _asm mov dwCodeStart, CODE
    
        DWORD dwAddr = (DWORD)( pData->ExceptionRecord->ExceptionAddress );
        dwAddr -= dwCodeStart;
    #ifdef _DEBUG
        dwAddr += 0x09d60;      // so it will match the most recent map file.
    #endif
    
        throw( CGrayException( id, dwAddr ));
    }
    
    _CRTIMP void _cdecl _assert( void *pExp, void *pFile, unsigned uLine )
    {
        // trap the system version of this just in case.
        Assert_CheckFail((const char*) pExp, (const char*) pFile, uLine );
    }
    
    int _cdecl _purecall()
    {
        // catch this special type of C++ exception as well.
        Assert_CheckFail( "purecall", "unknown", 1 );
        return 0;
    }
    
    #if 0
    void _cdecl _amsg_exit( int iArg )
    {
        // try to trap some of the other strange exit conditions !!!
        // Some strange stdlib calls use this.
        Assert_CheckFail( "_amsg_exit", "unknown", 1 );
    }
    #endif
    
    }   // extern "C"
    
    #endif      // _WIN32
    
    void * _cdecl operator new( size_t stAllocateBlock )
    {
        stAllocateBlock += sizeof(size_t) + sizeof(DWORD);
    
        void * pThis = malloc( stAllocateBlock );
        ASSERT(pThis);
        *((size_t*)pThis) = stAllocateBlock;
        *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = MEMORY_ALLOC_SIGNATURE;
    
        g_Serv.StatChange(SERV_STAT_MEM, stAllocateBlock );
        g_Serv.StatInc(SERV_STAT_ALLOCS);
        return((BYTE*) pThis + sizeof(size_t));
    }
    
    void * _cdecl operator new( size_t stAllocateBlock )
    {
        stAllocateBlock += sizeof(size_t) + sizeof(DWORD);
    
        void * pThis = malloc( stAllocateBlock );
        ASSERT(pThis);
        *((size_t*)pThis) = stAllocateBlock;
        *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = MEMORY_ALLOC_SIGNATURE;
    
        g_Serv.StatChange(SERV_STAT_MEM, stAllocateBlock );
        g_Serv.StatInc(SERV_STAT_ALLOCS);
        return((BYTE*) pThis + sizeof(size_t));
    }
    
    void _cdecl operator delete( void * pThis )
    {
        if ( pThis == NULL )
        {
                DEBUG_ERR(("delete:NULL" DEBUG_CR ));
                return;
        }
    
        pThis = (BYTE*) pThis - sizeof(size_t);
        size_t stAllocateBlock = *((size_t*)pThis);
        ASSERT( stAllocateBlock != -1 );
        ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );
    
        g_Serv.StatChange(SERV_STAT_MEM, - (long) stAllocateBlock );
        g_Serv.StatDec(SERV_STAT_ALLOCS);
    
        // invalidate the block.
        *((size_t*)pThis) = -1;
        *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = 0;
    
        free( pThis );
    }
    
    void _cdecl operator delete( void * pThis )
    {
        if ( pThis == NULL )
        {
                DEBUG_ERR(("delete:NULL" DEBUG_CR ));
                return;
        }
    
        pThis = (BYTE*) pThis - sizeof(size_t);
        size_t stAllocateBlock = *((size_t*)pThis);
        ASSERT( stAllocateBlock != -1 );
        ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );
    
        g_Serv.StatChange(SERV_STAT_MEM, - (long) stAllocateBlock );
        g_Serv.StatDec(SERV_STAT_ALLOCS);
    
        // invalidate the block.
        *((size_t*)pThis) = -1;
        *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = 0;
    
        free( pThis );
    }
    
    DIR_TYPE GetDirStr( LPCTSTR pszDir )
    {
        int iDir = toupper( pszDir );
        int iDir2 = 0;
        if ( iDir ) iDir2 = toupper( pszDir );
        switch ( iDir )
        {
        case 'E': return( DIR_E );
        case 'W': return( DIR_W );
        case 'N':
                if ( iDir2 == 'E' ) return( DIR_NE );
                if ( iDir2 == 'W' ) return( DIR_NW );
                return( DIR_N );
        case 'S':
                if ( iDir2 == 'E' ) return( DIR_SE );
                if ( iDir2 == 'W' ) return( DIR_SW );
                return( DIR_S );
        }
        return( DIR_QTY );
    }
    
    LPCTSTR GetTimeMinDesc( int minutes )
    {
        if ( minutes < 0 )
        {
                TCHAR * pTime = Str_GetTemp();
                strcpy( pTime, "Hmmm...I can't tell what time it is!");
                return( pTime );
        }
        int minute = minutes % 60;
        int hour = ( minutes / 60 ) % 24;
    
        LPCTSTR pMinDif;
        if (minute<=14) pMinDif = "";
        else if ((minute>=15)&&(minute<=30)) 
                pMinDif = " a quarter past";
        else if ((minute>=30)&&(minute<=45)) 
                pMinDif = " half past";
        else
        {
                pMinDif = " a quarter till";
                hour = ( hour + 1 ) % 24;
        }
    
        static LPCTSTR const sm_ClockHour =
        {
                "midnight",
                "one",
                "two",
                "three",
                "four",
                "five",
                "six",
                "seven",
                "eight",
                "nine",
                "ten",
                "eleven",
                "noon",
        };
    
        LPCTSTR pTail;
        if ( hour == 0 || hour==12 )
                pTail = "";
        else if ( hour > 12 )
        {
                hour -= 12;
                if ((hour>=1)&&(hour<6))
                        pTail = " o'clock in the afternoon";
                else if ((hour>=6)&&(hour<9))
                        pTail = " o'clock in the evening.";
                else
                        pTail = " o'clock at night";
        }
        else
        {
                pTail = " o'clock in the morning";
        }
    
        TCHAR * pTime = Str_GetTemp();
        sprintf( pTime, "%s %s%s.", pMinDif, sm_ClockHour, pTail );
        return( pTime );
    }
    
    int FindStrWord( LPCTSTR pTextSearch, LPCTSTR pszKeyWord )
    {
        // Find the pszKeyWord in the pTextSearch string.
        // Make sure we look for starts of words.
    
        int j=0;
        for ( int i=0; 1; i++ )
        {
                if ( pszKeyWord == '\0' )
                {
                        if ( pTextSearch=='\0' || ISWHITESPACE(pTextSearch))
                                return( i );
                        return( 0 );
                }
                if ( pTextSearch == '\0' )
                        return( 0 );
                if ( !j && i )
                {
                        if ( isalpha( pTextSearch ))    // not start of word ?
                                continue;
                }
                if ( toupper( pTextSearch ) == toupper( pszKeyWord ))
                        j++;
                else
                        j=0;
        }
    }
    
    CGrayThread * CGrayThread::GetCurrentThread() // static 
    {
        DWORD dwThreadID = CThread::GetCurrentThreadId();
        if ( g_MainTask.GetThreadID() == dwThreadID )
        {
                return( &g_MainTask );
        }
        if ( g_BackTask.GetThreadID() == dwThreadID )
        {
                return( &g_BackTask );
        }
        return( NULL );
    }
    
    //*******************************************************************
    // -CMainTask
    
    THREAD_ENTRY_RET _cdecl CMainTask::EntryProc( void * lpThreadParameter ) // static
    {
        // The main message loop.
        g_MainTask.OnCreate();
    #ifdef _WIN32
        _set_se_translator( Sphere_Exception_Win32 );   // must be called for each thread.
    #endif
        while ( ! Sphere_OnTick())
        {
    #ifdef _DEBUG
                DEBUG_CHECK( g_MainTask.SetScriptContext(NULL) == NULL );
                DEBUG_CHECK( g_MainTask.SetObjectContext(NULL) == NULL );
    #endif
        }
        g_MainTask.OnClose();
    }
    
    void CMainTask::CreateThread()
    {
        // AttachInputThread to us if needed ?
        CThread::CreateThread( EntryProc );
    }
    
    void CMainTask::CheckStuckThread()
    {
        // Periodically called to check if the tread is stuck.
    
        static CGTime sm_timeRealPrev = 0;
    
        // Has real time changed ?
        CGTime timeRealCur = CGTime::GetCurrentTime();
        int iTimeDiff = timeRealCur.GetTime() - sm_timeRealPrev.GetTime();
        iTimeDiff = abs( iTimeDiff );
        if ( iTimeDiff < g_Cfg.m_iFreezeRestartTime )
                return;
        sm_timeRealPrev = timeRealCur;
    
        // Has server time changed ?
        CServTime timeCur = CServTime::GetCurrentTime();
        if ( timeCur != m_timePrev )    // Seems ok.
        {
                m_timePrev = timeCur;
                return;
        }
    
        if ( g_Serv.IsValidBusy())      // Server is just busy.
                return;
    
        if ( m_timeRestart == timeCur )
        {
                g_Log.Event( LOGL_FATAL, "Main loop freeze RESTART FAILED!" DEBUG_CR );
                //g_Serv.SetExitFlag( -1 );
                //return;
                //_asm int 3;
        }
    
        if ( m_timeWarn == timeCur )
        {
                // Kill and revive the main process
                g_Log.Event( LOGL_CRIT, "Main loop freeze RESTART!" DEBUG_CR );
    
    #ifndef _DEBUG
                TerminateThread( 0xDEAD );
    
                                // try to restart it.
                g_Log.Event( LOGL_EVENT, "Trying to restart the main loop thread" DEBUG_CR );
                CreateThread();
    #endif
                m_timeRestart = timeCur;
        }
        else
        {
                g_Log.Event( LOGL_WARN, "Main loop frozen ?" DEBUG_CR );
                m_timeWarn = timeCur;
        }
    }
    
    //*******************************************************************
    
    int Sphere_InitServer( int argc, char *argv )
    {
        ASSERT( MAX_BUFFER >= sizeof( CCommand ));
        ASSERT( MAX_BUFFER >= sizeof( CEvent ));
        ASSERT( sizeof( int ) == sizeof( DWORD ));      // make this assumption often.
        ASSERT( sizeof( ITEMID_TYPE ) == sizeof( DWORD ));
        ASSERT( sizeof( WORD ) == 2 );
        ASSERT( sizeof( DWORD ) == 4 );
        ASSERT( sizeof( NWORD ) == 2 );
        ASSERT( sizeof( NDWORD ) == 4 );
        ASSERT(( UO_SIZE_X % SECTOR_SIZE_X ) == 0 );
        ASSERT(( UO_SIZE_Y % SECTOR_SIZE_Y ) == 0 );
        ASSERT( sizeof(CUOItemTypeRec) == 37 ); // byte pack working ?
    
    #ifdef _WIN32
    #if defined(_CONSOLE)
        SetConsoleTitle( GRAY_TITLE " V" GRAY_VERSION );
        SetConsoleCtrlHandler( ConsoleHandlerRoutine, true );
    #endif
        _set_se_translator( Sphere_Exception_Win32 );
    #endif
    
        g_Log.WriteString( DEBUG_CR );          // blank space in log.
        g_Log.Event( LOGM_INIT, "%s" DEBUG_CR
                "Compiled at " __DATE__ " (" __TIME__ ")" DEBUG_CR
                // "NOTE: All saves made with ver .53 are unstable and not supported in the future" DEBUG_CR
                DEBUG_CR,
                g_szServerDescription );
    
        if ( ! g_Serv.Load())
        {
                g_Log.Event( LOGL_FATAL|LOGM_INIT, "The " GRAY_FILE ".INI file is corrupt or missing" DEBUG_CR );
                return( -3 );
        }
    
        if ( argc > 1 )
        {
                // Do special debug type stuff.
                if ( ! g_Serv.CommandLine( argc, argv ))
                {
                        return( -1 );
                }
        }
    
        if ( ! g_World.LoadAll())
        {
                return( -8 );
        }
    
        if ( ! g_Serv.SocketsInit())
        {
                return( -9 );
        }
    
        if ( g_Cfg.m_fUseIRC )
        {
                g_IRCLocalServer.Init();
        }
    
        g_Log.Event( LOGM_INIT, g_Serv.GetStatusString( 0x24 ));
        g_Log.Event( LOGM_INIT, _TEXT("Startup complete. items=%d, chars=%d" DEBUG_CR), g_Serv.StatGet(SERV_STAT_ITEMS), g_Serv.
StatGet(SERV_STAT_CHARS));
    
    #ifdef _WIN32
        g_Log.Event( LOGM_INIT, "Press '?' for console commands" DEBUG_CR );
    #endif
        g_Log.FireEvent( LOGEVENT_Startup );
    
        return( 0 );
    }
    
    void Sphere_ExitServer()
    {
        ASSERT(g_Serv.m_iExitFlag);
        g_Serv.SetServerMode( SERVMODE_Exiting );
    
        g_BackTask.WaitForClose( 15 );
        g_MainTask.WaitForClose( 15 );
    
        g_Serv.SocketsClose();
        g_World.Close();
    
    #if defined(_WIN32) && defined(_CONSOLE)
        SetConsoleCtrlHandler( ConsoleHandlerRoutine, false );
    #endif
    
        if ( g_Serv.m_iExitFlag < 0 )
        {
                g_Log.Event( LOGM_INIT|LOGL_FATAL, "Server terminated by error %d!" DEBUG_CR, g_Serv.m_iExitFlag );
    #if defined(_WIN32) && defined(_CONSOLE)
                g_Serv.SysMessage( "Press any key to exit" );
                while ( _getch() == 0 ) ;
    #endif
        }
        else
        {
                g_Log.Event( LOGM_INIT|LOGL_EVENT, "Server shutdown (code %d) complete!" DEBUG_CR, g_Serv.m_iExitFlag );
        }
    
        g_Log.FireEvent( LOGEVENT_Shutdown );
        g_Log.Close();
    }
    
    int Sphere_OnTick()
    {
        // Give the world (CMainTask) a single tick.
        // RETURN: 0 = everything is fine.
        try
        {
    #ifdef _WIN32
                g_Serv.OnTick_Busy();   // not used in Linux
    #endif
                g_World.OnTick();
                g_Serv.OnTick();
                if ( g_Cfg.m_fUseIRC )
                {
                        g_IRCLocalServer.OnTick();
                }
        }
        catch ( CGrayError &e )
        {
                g_Log.CatchEvent( &e, "Main Loop" );
        }
        catch (...)     // catch all
        {
                g_Log.CatchEvent( NULL, "Main Loop" );
        }
    
        return( g_Serv.m_iExitFlag );
    }
    
    //*****************************************************
    
    static void Sphere_MainMonitorLoop()
    {
        // Just make sure the main loop is alive every so often.
        // This should be the parent thread.
        // try to restart it if it is not.
    
        ASSERT( CThread::GetCurrentThreadId() == g_Serv.m_dwParentThread );
        while ( ! g_Serv.m_iExitFlag )
        {
                if ( g_Cfg.m_iFreezeRestartTime <= 0 )
                {
                        DEBUG_ERR(( "Freeze Restart Time cannot be cleared at run time" DEBUG_CR ));
                        g_Cfg.m_iFreezeRestartTime = 10;
                }
    
    #ifdef _WIN32
                NTWindow_OnTick( g_Cfg.m_iFreezeRestartTime * 1000 );
    #else
                Sleep( g_Cfg.m_iFreezeRestartTime * 1000 );
    #endif
    
                // Don't look for freezing when doing certain things.
                if ( g_Serv.IsLoading() || ! g_Cfg.m_fSecure )
                        continue;
    
                g_MainTask.CheckStuckThread();
                g_BackTask.CheckStuckThread();
        }
    }
    
    //******************************************************
    
    #ifdef _WIN32
    int Sphere_MainEntryPoint( int argc, char *argv )
    #else
    int _cdecl main( int argc, char * argv )
    #endif
    {
        g_Serv.m_iExitFlag = Sphere_InitServer( argc, argv );
    
        if ( ! g_Serv.m_iExitFlag )
        {
                if (
    #ifdef _WIN32
                        GRAY_GetOSInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT &&
    #endif
                        g_Cfg.m_iFreezeRestartTime )
                {
                        g_MainTask.CreateThread();
                        Sphere_MainMonitorLoop();
                }
                else
                {
                        g_MainTask.EntryProc( 0 );
                }
        }
    
        Sphere_ExitServer();
        return( g_Serv.m_iExitFlag );
    }