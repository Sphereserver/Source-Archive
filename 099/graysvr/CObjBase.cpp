    // CObjBase.cpp
    // Copyright Menace Software (www.menasoft.com).
    // base classes.
    //
    #include "graysvr.h"        // predef header.
    
    #if defined(_DEBUG) && defined(GRAY_SVR)    //
    bool CObjBaseTemplate::IsValidContainer() const     // am i really in the container i say i am ?
    {
        if ( IsDisconnected())  // must also be at top level !
        {
                // There are several places a disconnected item can be.
                if ( GetParent() == NULL )
                        return( true );
                if ( IsChar())
                {
                        return( dynamic_cast <CCharsDisconnectList*>( GetParent()) != NULL );
                }
                else
                {
                        return( false );        // not valid for item to be disconnected. (except at create time)
                }
        }
        if ( IsItem())
        {
                if ( dynamic_cast <const CItem*>(this) == NULL )
                        return( false );
        }
        else
        {
                if ( dynamic_cast <const CChar*>(this) == NULL )
                        return( false );
        }
        if ( IsTopLevel())
        {
                if ( IsChar())
                {
                        return( dynamic_cast <CCharsActiveList*>( GetParent()) != NULL );
                }
                else
                {
                        return( dynamic_cast <CItemsList*>( GetParent()) != NULL );
                }
        }
        if ( IsItemEquipped())
        {
                if ( IsChar())
                {
                        return( false );
                }
                else
                {
                        return( dynamic_cast <CChar*>( GetParent()) != NULL );
                }
        }
        if ( IsItemInContainer())
        {
                if ( IsChar())
                {
                        return( false );
                }
                else
                {
                        return( dynamic_cast <CItemContainer*>( GetParent()) != NULL );
                }
        }
        return( false );        // no flags !?
    }
    #endif
    
    bool GetDeltaStr( CPointMap & pt, TCHAR * pszDir )
    {
        TCHAR * ppCmd;
        int iQty = Str_ParseCmds( pszDir, ppCmd, COUNTOF(ppCmd));
        TCHAR chDir = toupper( ppCmd );
        int iTmp = Exp_GetVal( ppCmd );
    
        if ( isdigit( chDir ) || chDir == '-' )
        {
                pt.m_x += Exp_GetVal( ppCmd );
                pt.m_y += iTmp;
                pt.m_z += Exp_GetVal( ppCmd );
        }
        else    // a direction by name.
        {
                if ( iTmp == 0 )
                        iTmp = 1;
                DIR_TYPE eDir = GetDirStr( ppCmd );
                if ( eDir >= DIR_QTY )
                        return( false );
                pt.MoveN( eDir, iTmp );
        }
    
        return( true );
    }
    
    /////////////////////////////////////////////////////////////////
    // -CObjBase stuff
    // Either a player, npc or item.
    
    CObjBase::CObjBase( bool fItem )
    {
        sm_iCount ++;
        m_wHue=HUE_DEFAULT;
        m_timeout.Init();
    
        if ( g_Serv.IsLoading())
        {
                // Don't do this yet if we are loading. UID will be set later.
                // Just say if this is an item or not.
                CObjBaseTemplate::SetUID( fItem ? ( UID_O_DISCONNECT | UID_F_ITEM | UID_O_INDEX_MASK ) : ( UID_O_DISCONNECT | UI
D_O_INDEX_MASK ));
        }
        else
        {
                // Find a free UID slot for this.
                SetUID( UID_CLEAR, fItem );
                ASSERT(IsValidUID());
                SetContainerFlags(UID_O_DISCONNECT);    // it is no place for now
        }
    
        // Put in the idle list by default. (til placed in the world)
        g_World.m_ObjNew.InsertHead( this );
    }
    
    CObjBase::~CObjBase()
    {
        sm_iCount --;
        ASSERT( IsDisconnected());
    
        // free up the UID slot.
        SetUID( UID_UNUSED, false );
    }
    
    bool CObjBase::IsContainer() const
    {
        // Simple test if object is a container.
        return( dynamic_cast <const CContainer*>(this) != NULL );
    }
    
    int CObjBase::IsWeird() const
    {
        int iResultCode = CObjBaseTemplate::IsWeird();
        if ( iResultCode )
        {
                return( iResultCode );
        }
        if ( ! g_Serv.IsLoading())
        {
                if ( GetUID().ObjFind() != this )       // make sure it's linked both ways correctly.
                {
                        return( 0x3201 );
                }
        }
        return( 0 );
    }
    
    void CObjBase::SetUID( DWORD dwIndex, bool fItem )
    {
        // Move the serial number,
        // This is possibly dangerous if conflict arrises.
    
        dwIndex &= UID_O_INDEX_MASK;    // Make sure no flags in here.
        if ( IsValidUID())      // i already have a uid ?
        {
                if ( ! dwIndex )
                        return; // The point was just to make sure it was located.
                // remove the old UID.
                g_World.FreeUID( ((DWORD)GetUID()) & UID_O_INDEX_MASK );
        }
    
        if ( dwIndex != UID_O_INDEX_MASK )      // just wanted to remove it
        {
                dwIndex = g_World.AllocUID( dwIndex, this );
        }
    
        if ( fItem ) dwIndex |= UID_F_ITEM;
    
        CObjBaseTemplate::SetUID( dwIndex );
    }
    
    bool CObjBase::SetNamePool( LPCTSTR pszName )
    {
        ASSERT(pszName);
    
        // Parse out the name from the name pool ?
        if ( pszName == '#' )
        {
                pszName ++;
                TCHAR szTmp;
                strcpy( szTmp, pszName );
    
                TCHAR * ppTitles;
                Str_ParseCmds( szTmp, ppTitles, COUNTOF(ppTitles));
    
                CResourceLock s;
                if ( ! g_Cfg.ResourceLock( s, RES_NAMES, ppTitles ))
                {
    failout:
                        DEBUG_ERR(( "Name pool '%s' could not be found\n", ppTitles ));
                        CObjBase::SetName( ppTitles );
                        return false;
                }
    
                // Pick a random name.
                if ( ! s.ReadKey())
                        goto failout;
                int iCount = Calc_GetRandVal( atoi( s.GetKey())) + 1;
                while ( iCount-- )
                {
                        if ( ! s.ReadKey())
                                goto failout;
                }
    
                return CObjBaseTemplate::SetName( s.GetKey());
        }
    
        // NOTE: Name must be <= MAX_NAME_SIZE
        TCHAR szTmp;
        int len = strlen( pszName );
        if ( len >= MAX_ITEM_NAME_SIZE )
        {
                strcpylen( szTmp, pszName, MAX_ITEM_NAME_SIZE );
                pszName = szTmp;
        }
    
        // Can't be a dupe name with type ?
        LPCTSTR pszTypeName = Base_GetDef()->GetTypeName();
        if ( ! strcmpi( pszTypeName, pszName ))
                pszName = "";
    
        return CObjBaseTemplate::SetName( pszName );
    }
    
    bool CObjBase::MoveNearObj( const CObjBaseTemplate * pObj, int iSteps, WORD wCan )
    {
        ASSERT( pObj );
        if ( pObj->IsDisconnected())    // nothing is "near" a disconnected item.
        {
                // DEBUG_CHECK(! pObj->IsDisconnected() );
                return( false );
        }
    
        pObj = pObj->GetTopLevelObj();
        MoveNear( pObj->GetTopPoint(), iSteps, wCan );
        return( true );
    }
    
    void CObjBase::r_WriteSafe( CScript & s )
    {
        // Write an object with some fault protection.
    
        CGrayUID uid;
        try
        {
                uid = GetUID();
                if ( ! g_Cfg.m_fSaveGarbageCollect )
                {
                        if ( g_World.FixObj( this ))
                                return;
                }
                r_Write(s);
        }
        catch ( CGrayError &e )
        {
                g_Log.CatchEvent( &e, "Write Object 0%x", (DWORD) uid );
        }
        catch (...)     // catch all
        {
                g_Log.CatchEvent( NULL, "Write Object 0%x", (DWORD) uid );
        }
    }
    
    void CObjBase::SetTimeout( int iDelayInTicks )
    {
        // Set delay in TICK_PER_SEC of a sec.
        // -1 = never.
    
    #if 0 // def _DEBUG
        if ( g_Log.IsLogged( LOGL_TRACE ))
        {
                // DEBUG_MSG(( "SetTimeout(%d) for 0%lx\n", iDelay, GetUID()));
                //if ( iDelay > 100*TICK_PER_SEC )
                //{
                //      DEBUG_MSG(( "SetTimeout(%d) for 0%lx LARGE\n", iDelay, GetUID()));
                //}
        }
    #endif
    
        if ( iDelayInTicks < 0 )
        {
                m_timeout.Init();
        }
        else
        {
                m_timeout = CServTime::GetCurrentTime() + iDelayInTicks;
        }
    }
    
    void CObjBase::Sound( SOUND_TYPE id, int iOnce ) const // Play sound effect for player
    {
        // play for everyone near by.
    
        if ( id <= 0 )
                return;
    
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( ! pClient->CanHear( this, TALKMODE_OBJ ))
                        continue;
                pClient->addSound( id, this, iOnce );
        }
    }
    
    void CObjBase::Effect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBase * pSource, BYTE bSpeedSeconds, BYTE bLoop, bool fE
xplode ) const
    {
        // show for everyone near by.
        //
        // bSpeedSeconds
        // bLoop
        // fExplode 
    
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( ! pClient->CanSee( this ))
                        continue;
                pClient->addEffect( motion, id, this, pSource, bSpeedSeconds, bLoop, fExplode );
        }
    }
    
    void CObjBase::Emote( LPCTSTR pText, CClient * pClientExclude, bool fForcePossessive )
    {
        // IF this is not the top level object then it might be possessive ?
    
        // "*You see NAME blah*" or "*You blah*"
        // fPosessive = "*You see NAME's blah*" or "*Your blah*"
    
        CObjBase * pObjTop = STATIC_CAST <CObjBase*>( GetTopLevelObj());
        ASSERT(pObjTop);
    
        CGString sMsgThem;
        CGString sMsgYou;
    
        if ( pObjTop->IsChar())
        {
                // Someone has this equipped.
    
                if ( pObjTop != this )
                {
                        sMsgThem.Format( "*You see %ss %s %s*", (LPCTSTR) pObjTop->GetName(), (LPCTSTR) GetName(), (LPCTSTR) pTe
xt );
                        sMsgYou.Format( "*Your %s %s*", (LPCTSTR) GetName(), (LPCTSTR) pText );
                }
                else if ( fForcePossessive )
                {
                        // ex. "You see joes poor shot ruin an arrow"
                        sMsgThem.Format( "*You see %ss %s*", (LPCTSTR) GetName(), (LPCTSTR) pText );
                        sMsgYou.Format( "*Your %s*", pText );
                }
                else
                {
                        sMsgThem.Format( "*You see %s %s*", (LPCTSTR) GetName(), (LPCTSTR) pText );
                        sMsgYou.Format( "*You %s*", pText );
                }
        }
        else
        {
                // Top level is an item. Article ?
                sMsgThem.Format( "*You see %s %s*", (LPCTSTR) GetName(), (LPCTSTR) pText );
                sMsgYou = sMsgThem;
        }
    
        pObjTop->UpdateObjMessage( sMsgThem, sMsgYou, pClientExclude, HUE_RED, TALKMODE_EMOTE );
    }
    
    void CObjBase::Speak( LPCTSTR pText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font )
    {
        g_World.Speak( this, pText, wHue, mode, font );
    }
    
    void CObjBase::SpeakUTF8( LPCTSTR pText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang )
    {
        // convert UTF8 to UNICODE.
        NCHAR szBuffer;
        int iLen = CvtSystemToNUNICODE( szBuffer, COUNTOF(szBuffer), pText, -1 );
        g_World.SpeakUNICODE( this, szBuffer, wHue, mode, font, lang );
    }
    
    void CObjBase::MoveNear( CPointMap pt, int iSteps, WORD wCan )
    {
        // Move to nearby this other object.
        // Actually move it within +/- iSteps
    
        DIR_TYPE dir = (DIR_TYPE) Calc_GetRandVal( DIR_QTY );
        for (; iSteps > 0 ; iSteps-- )
        {
                // Move to the right or left?
                CPointBase pTest = pt;  // Save this so we can go back to it if we hit a blocking object.
                pt.Move( dir );
                dir = GetDirTurn( dir, Calc_GetRandVal(3)-1 );  // stagger ?
                // Put the item at the correct Z point
                WORD wBlockRet = wCan;
                pt.m_z = g_World.GetHeightPoint( pt, wBlockRet, pt.GetRegion(REGION_TYPE_MULTI));
                if ( wBlockRet &~ wCan )
                {
                        // Hit a block, so go back to the previous valid position
                        pt = pTest;
                        break;  // stopped
                }
        }
    
        MoveTo( pt );
    }
    
    void CObjBase::UpdateObjMessage( LPCTSTR pTextThem, LPCTSTR pTextYou, CClient * pClientExclude, HUE_TYPE wHue, TALKMODE_TYPE
 mode ) const
    {
        // Show everyone a msg coming form this object.
    
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( pClient == pClientExclude )
                        continue;
                if ( ! pClient->CanSee( this ))
                        continue;
    
                pClient->addBark(( pClient->GetChar() == this )? pTextYou : pTextThem, this, wHue, mode, FONT_NORMAL );
        }
    }
    
    void CObjBase::UpdateCanSee( const CCommand * pCmd, int iLen, CClient * pClientExclude ) const
    {
        // Send this update message to everyone who can see this.
        // NOTE: Need not be a top level object. CanSee() will calc that.
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( pClient == pClientExclude )
                        continue;
                if ( ! pClient->CanSee( this ))
                        continue;
                pClient->xSendPkt( pCmd, iLen );
        }
    }
    
    TRIGRET_TYPE CObjBase::OnHearTrigger( CResourceLock & s, LPCTSTR pszCmd, CChar * pSrc )
    {
        // Check all the keys in this script section.
        // look for pattern or partial trigger matches.
        // RETURN:
        //  TRIGRET_ENDIF = no match.
        //  TRIGRET_DEFAULT = found match but it had no RETURN
    
        bool fMatch = false;
    
        while ( s.ReadKeyParse())
        {
                if ( s.IsKeyHead("ON",2))
                {
                        // Look for some key word.
                        _strupr( s.GetArgStr());
                        if ( Str_Match( s.GetArgStr(), pszCmd ) == MATCH_VALID )
                                fMatch = true;
                        continue;
                }
    
                if ( ! fMatch )
                        continue;       // look for the next "ON" section.
    
                CScriptTriggerArgs Args( pszCmd );
                TRIGRET_TYPE iRet = CObjBase::OnTriggerRun( s, TRIGRUN_SECTION_EXEC, pSrc, &Args );
                if ( iRet != TRIGRET_RET_FALSE )
                        return( iRet );
    
                fMatch = false;
        }
    
        return( TRIGRET_ENDIF );        // continue looking.
    }
    
    bool CObjBase::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
    {
        if ( ! strnicmp( pszKey, "SECTOR.", 7 ))
        {
                pszKey += 7;
                pRef = GetTopLevelObj()->GetTopSector();
                return( true );
        }
        if ( ! strnicmp( pszKey, "TYPEDEF.", 8 ))
        {
                pszKey += 8;
                pRef = Base_GetDef();
                return( true );
        }
        if ( ! strnicmp( pszKey, "TOPOBJ.", 7 ))
        {
                pszKey += 7;
                pRef = dynamic_cast <CObjBase*>( GetTopLevelObj());
                return( true );
        }
        return( CScriptObj::r_GetRef( pszKey, pRef ));
    }
    
    enum OBC_TYPE
    {
        OC_CANSEE,
        OC_CANSEELOS,
        OC_COLOR,
        OC_DISTANCE,
        OC_ISCHAR,
        OC_ISITEM,
        OC_MAPPLANE,
        OC_NAME,
        OC_P,
        OC_SERIAL,
        OC_TIMER,
        OC_TIMERD,
        OC_UID,
        OC_WEIGHT,
        OC_Z,
        OC_QTY,
    };
    
    LPCTSTR const CObjBase::sm_szLoadKeys =
    {
        "CanSee",
        "CanSeeLOS",
        "COLOR",
        "Distance",     // distance to the source.
        "IsChar",
        "IsItem",
        "MAPPLANE",
        "NAME",
        "P",
        "SERIAL",       // same as UID
        "TIMER",
        "TIMERD",
        "UID",
        "Weight",
        "Z",
        NULL,
    };
    
    void CObjBase::r_DumpLoadKeys( CTextConsole * pSrc )
    {
        r_DumpKeys(pSrc,sm_szLoadKeys);
        CScriptObj::r_DumpLoadKeys(pSrc);
    }
    
    bool CObjBase::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
    {
        DEBUG_CHECK(pSrc);
    
        int index = FindTableHeadSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
        if ( index < 0 )
        {
                // Is it a function returning a value ? Parse args ?
                index = g_Cfg.m_Functions.FindKey( pszKey );
                if ( index >= 0 )
                {
                        CResourceNamed * pFunction = STATIC_CAST <CResourceNamed *>( g_Cfg.m_Functions );
                        ASSERT(pFunction);
                        CResourceLock sFunction;
                        if ( pFunction->ResourceLock(sFunction))
                        {
                                // CScriptTriggerArgs Args( s.GetArgStr());
                                TRIGRET_TYPE iRet = OnTriggerRun( sFunction, TRIGRUN_SECTION_TRUE, pSrc ); // &Args
                                sVal.FormatVal(iRet);
                        }
                        return( true );
                }
    
                // Just try to default to something reasonable ?
                // Even though we have not really specified it correctly !
    
                // SECTOR. ?
                if ( ! IsDisconnected())
                {
                        if ( GetTopLevelObj()->GetTopSector()->r_WriteVal( pszKey, sVal, pSrc ))
                                return( true );
                }
    
                // WORLD. ?
                if ( g_World.r_WriteVal( pszKey, sVal, pSrc ))
                        return( true );
    
                // TYPEDEF. ?
                if ( Base_GetDef()->r_WriteVal( pszKey, sVal, pSrc ))
                        return( true );
    
    do_default:
                return( CScriptObj::r_WriteVal( pszKey, sVal, pSrc ));
        }
    
        switch (index)
        {
        case OC_CANSEE:
                {
                        CChar * pChar = pSrc->GetChar();
                        if ( pChar == NULL )
                                return( false );
                        sVal.FormatVal( pChar->CanSee( this ));
                }
                break;
        case OC_CANSEELOS:
                {
                        CChar * pChar = pSrc->GetChar();
                        if ( pChar == NULL )
                                return( false );
                        sVal.FormatVal( pChar->CanSeeLOS( this ));
                }
                break;
        case OC_COLOR:
                sVal.FormatHex( GetHue());
                break;
        case OC_DISTANCE:
                {
                        // Distance from source to this.
                        CChar * pChar = pSrc->GetChar();
                        if ( pChar == NULL )
                                return( false );
                        sVal.FormatVal( GetDist( pChar ));
                }
                break;
        case OC_ISCHAR:
                sVal.FormatVal( IsChar());
                break;
        case OC_ISITEM:
                sVal.FormatVal( IsItem());
                break;
        case OC_MAPPLANE:
                sVal.FormatVal( GetUnkPoint().m_mapplane );
                break;
        case OC_NAME:
                sVal = GetName();
                break;
        case OC_P:
                if ( pszKey == '.' )
                {
                        return( GetUnkPoint().r_WriteVal( pszKey+2, sVal ));
                }
                sVal = GetUnkPoint().WriteUsed();
                break;
        case OC_TIMER:
                sVal.FormatVal( GetTimerAdjusted());
                break;
        case OC_UID:
                if ( pszKey == '.' )
                        goto do_default;
        case OC_SERIAL:
                sVal.FormatHex( GetUID());
                break;
        case OC_WEIGHT:
                sVal.FormatVal( GetWeight());
                break;
        case OC_Z:
                sVal.FormatVal( GetUnkZ());
                break;
        default:
                DEBUG_CHECK(0);
                return( false );
        }
        return( true );
    }
    
    bool CObjBase::r_LoadVal( CScript & s )
    {
        // load the basic stuff.
    
        int index = FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 );
        if ( index < 0 )
        {
                return( CScriptObj::r_LoadVal(s));
        }
    
        switch ( index )
        {
        case OC_COLOR:
                if ( ! strcmpi( s.GetArgStr(), "match_shirt" ) ||
                        ! strcmpi( s.GetArgStr(), "match_hair" ))
                {
                        CChar * pChar = dynamic_cast <CChar*>(GetTopLevelObj());
                        if ( pChar )
                        {
                                CItem * pHair = pChar->LayerFind( ! strcmpi( s.GetArgStr()+6, "shirt" ) ? LAYER_SHIRT : LAYER_HA
IR );
                                if ( pHair )
                                {
                                        m_wHue = pHair->GetHue();
                                        break;
                                }
                        }
                        m_wHue = HUE_GRAY;
                        break;
                }
                RemoveFromView();
                m_wHue = (HUE_TYPE) s.GetArgVal();
                Update();
                break;
        case OC_DISTANCE:       // Read only.
                return( false );
        case OC_MAPPLANE:
                // Move to another map plane.
                if ( ! IsTopLevel())
                        return( false );
                {
                        CPointMap pt = GetTopPoint();
                        pt.m_mapplane = s.GetArgVal();
                        MoveTo(pt);
                }
                break;
        case OC_NAME:
                SetName( s.GetArgStr());
                break;
        case OC_P:      // Must set the point via the CItem or CChar methods.
                return( false );
        case OC_TIMER:
                if ( g_World.m_iLoadVersion < 34 )
                {
                        m_timeout.InitTime( s.GetArgVal());
                }
                else
                {
                        SetTimeout( s.GetArgVal() * TICK_PER_SEC );
                }
                break;
        case OC_TIMERD:
                SetTimeout( s.GetArgVal());
                break;
        case OC_UID:
        case OC_SERIAL:
                DEBUG_CHECK( IsDisconnected());
                DEBUG_CHECK( g_Serv.IsLoading());
                DEBUG_CHECK( ! IsValidUID());
                // Don't set container flags through this.
                SetUID( s.GetArgVal(), (dynamic_cast <CItem*>(this)) ? true : false );
                break;
        default:
                DEBUG_CHECK(0);
                return( false );
        }
    
        return( true );
    }
    
    void CObjBase::r_Serialize( CGFile & a )
    {
        // Read and write binary.
    }
    
    void CObjBase::r_Write( CScript & s )
    {
        s.WriteKeyHex( "SERIAL", GetUID());
        if ( IsIndividualName())
                s.WriteKey( "NAME", GetIndividualName());
        if ( m_wHue != HUE_DEFAULT )
                s.WriteKeyHex( "COLOR", GetHue());
        if ( m_timeout.IsTimeValid() )
                s.WriteKeyVal( "TIMER", GetTimerAdjusted());
    }
    
    enum OV_TYPE
    {
        OV_DAMAGE,
        OV_DCLICK,
        OV_DIALOG,
        OV_EDIT,
        OV_EFFECT,
        OV_EMOTE,
        OV_FIX,
        OV_FIXWEIGHT,
        OV_FLIP,
        OV_FOLLOW,
        OV_FOLLOWED,
        OV_HIT,
        OV_INFO,
        OV_INPDLG,
        OV_M,
        OV_MENU,
        OV_MESSAGE,
        OV_MOVE,
        OV_MOVETO,
        OV_MSG,
        OV_NUDGEDOWN,
        OV_NUDGEUP,
        OV_P,   // protect this from use.
        OV_PROPS,
        OV_REMOVE,
        OV_REMOVEFROMVIEW,
        OV_SAY,
        OV_SAYU,
        OV_SAYUA,
        OV_SFX,
        OV_SOUND,
        OV_SPEAK,
        OV_SPEAKU,
        OV_SPEAKUA,
        OV_SPELLEFFECT,
        OV_TARGET,
        OV_TARGETG,
        OV_TRY,
        OV_TRYP,
        OV_TWEAK,
        OV_UID,
        OV_UPDATE,
        OV_UPDATEX,
        OV_USEITEM,
        OV_Z,
        OV_QTY,
    };
    
    LPCTSTR const CObjBase::sm_szVerbKeys =
    {
        "DAMAGE",
        "DCLICK",
        "DIALOG",
        "EDIT",
        "EFFECT",
        "EMOTE",
        "FIX",
        "FIXWEIGHT",
        "FLIP",
        "FOLLOW",
        "FOLLOWED",     // get rid of this
        "HIT",
        "INFO",
        "INPDLG",
        "M",
        "MENU",
        "MESSAGE",
        "MOVE",
        "MOVETO",
        "MSG",
        "NUDGEDOWN",
        "NUDGEUP",
        "P",    // protect this from use.
        "PROPS",
        "REMOVE",
        "REMOVEFROMVIEW",
        "SAY",
        "SAYU",
        "SAYUA",
        "SFX",
        "SOUND",
        "SPEAK",
        "SPEAKU",
        "SPEAKUA",
        "SPELLEFFECT",
        "TARGET",
        "TARGETG",
        "TRY",
        "TRYP",
        "TWEAK",
        "UID",
        "UPDATE",
        "UPDATEX",
        "USEITEM",
        "Z",
        NULL,
    };
    
    void CObjBase::r_DumpVerbKeys( CTextConsole * pSrc )
    {
        r_DumpKeys(pSrc,sm_szVerbKeys);
        CScriptObj::r_DumpVerbKeys(pSrc);
    }
    
    bool CObjBase::r_Verb( CScript & s, CTextConsole * pSrc ) // Execute command from script
    {
        ASSERT(pSrc);
    
        // Is it a macro command ? RES_FUNCTION
        int index = g_Cfg.m_Functions.FindKey( s.GetKey());
        if ( index >= 0 )
        {
                CResourceNamed * pFunction = STATIC_CAST <CResourceNamed *>( g_Cfg.m_Functions );
                ASSERT(pFunction);
                CResourceLock sFunction;
                if ( pFunction->ResourceLock(sFunction))
                {
                        CScriptTriggerArgs Args( s.GetArgStr());
                        TRIGRET_TYPE iRet = OnTriggerRun( sFunction, TRIGRUN_SECTION_TRUE, pSrc, &Args );
                }
                return( true );
        }
    
        index = FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF(sm_szVerbKeys)-1 );
        if ( index < 0 )
        {
                // Not found.
                return( CScriptObj::r_Verb( s, pSrc ));
        }
    
        CChar * pCharSrc = pSrc->GetChar();
        CClient * pClientSrc = (pCharSrc && pCharSrc->IsClient()) ? (pCharSrc->GetClient()) : NULL ;
    
        switch (index)
        {
        case OV_HIT:    //      "Amount,SourceFlags,SourceCharUid" = do me some damage.
                {
                        int piCmd;
                        int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
                        if ( iArgQty < 1 )
                                return( false );
                        if ( iArgQty > 2 )      // Give it a new source char UID
                        {
                                CObjBaseTemplate * pObj = CGrayUID( piCmd ).ObjFind();
                                if ( pObj )
                                {
                                        pObj = pObj->GetTopLevelObj();
                                }
                                pCharSrc = dynamic_cast <CChar*>(pObj);
                        }
                        OnGetHit( piCmd,
                                pCharSrc,
                                ( iArgQty > 1 ) ? piCmd : ( DAMAGE_HIT_BLUNT | DAMAGE_GENERAL ));
                }
                break;
    
        case OV_DAMAGE: //      "Amount,SourceFlags,SourceCharUid" = do me some damage.
                {
                        int piCmd;
                        int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
                        if ( iArgQty < 1 )
                                return( false );
                        if ( iArgQty > 2 )      // Give it a new source char UID
                        {
                                CObjBaseTemplate * pObj = CGrayUID( piCmd ).ObjFind();
                                if ( pObj )
                                {
                                        pObj = pObj->GetTopLevelObj();
                                }
                                pCharSrc = dynamic_cast <CChar*>(pObj);
                        }
                        OnTakeDamage( piCmd,
                                pCharSrc,
                                ( iArgQty > 1 ) ? piCmd : ( DAMAGE_HIT_BLUNT | DAMAGE_GENERAL ));
                }
                break;
    
        case OV_DCLICK:
                if ( ! pCharSrc )
                        return( false );
                return pCharSrc->Use_Obj( this, true );
    
        case OV_EDIT:
                {
                        // Put up a list of items in the container. (if it is a container)
                        if ( pClientSrc == NULL )
                                return( false );
                        pClientSrc->m_Targ_Text = s.GetArgStr();
                        pClientSrc->Cmd_EditItem( this, -1 );
                }
                break;
        case OV_EFFECT: // some visual effect.
                {
                        int piCmd;
                        int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
                        if ( iArgQty < 2 )
                                return( false );
    
                        Effect( (EFFECT_TYPE) piCmd, (ITEMID_TYPE) RES_GET_INDEX(piCmd),
                                pCharSrc,
                                (iArgQty>=3)? piCmd : 5, //BYTE bSpeedSeconds = 5,
                                (iArgQty>=4)? piCmd : 1, //BYTE bLoop = 1,
                                (iArgQty>=5)? piCmd : false //bool fExplode = false
                                );
                }
                break;
        case OV_EMOTE:
                Emote( s.GetArgStr());
                break;
        case OV_FIXWEIGHT:
                //FixWeight();
                break;
        case OV_FLIP:
                Flip( s.GetArgStr());
                break;
        case OV_FOLLOW:
        case OV_FOLLOWED:
                // Follow this item.
                if ( pCharSrc )
                {
                        // I want to follow this person about.
                        pCharSrc->Memory_AddObjTypes( this, MEMORY_FOLLOW );
                }
                break;
        case OV_INPDLG:
                // "INPDLG" verb maxchars
                // else assume it was a property button.
                {
                        if ( pClientSrc == NULL )
                                return( false );
    
                        TCHAR *Arg_ppCmd;               // Maximum parameters in one line
                        int iQty = Str_ParseCmds( s.GetArgStr(), Arg_ppCmd, COUNTOF( Arg_ppCmd ));
    
                        CGString sOrgValue;
                        if ( ! r_WriteVal( Arg_ppCmd, sOrgValue, pSrc ))
                                sOrgValue = ".";
    
                        pClientSrc->m_Targ_Text = Arg_ppCmd;    // The attribute we want to edit.
                        int iMaxLength = atoi(Arg_ppCmd);
    
                        CGString sPrompt;
                        sPrompt.Format( "%s (# = default)", (LPCTSTR) Arg_ppCmd );
                        pClientSrc->addGumpInpVal( true, INPVAL_STYLE_TEXTEDIT,
                                iMaxLength,     sPrompt, sOrgValue, this );
                }
                break;
        case OV_MENU:
                {
                        if ( pClientSrc == NULL )
                                return( false );
                        pClientSrc->Menu_Setup( g_Cfg.ResourceGetIDType( RES_MENU, s.GetArgStr()), this );
                }
                break;
        case OV_MESSAGE:        //put info message (for pSrc client only) over item.
        case OV_MSG:
                {
                        if ( pCharSrc == NULL )
                        {
                                UpdateObjMessage( s.GetArgStr(), s.GetArgStr(), NULL, HUE_TEXT_DEF, IsChar() ? TALKMODE_EMOTE : 
TALKMODE_ITEM );
                        }
                        else
                        {
                                pCharSrc->ObjMessage( s.GetArgStr(), this );
                        }
                }
                break;
        case OV_M:
        case OV_MOVE:
                // move without restriction. east,west,etc. (?up,down,)
                if ( IsTopLevel())
                {
                        CPointMap pt = GetTopPoint();
                        if ( ! GetDeltaStr( pt, s.GetArgStr()))
                                return( false );
                        MoveTo( pt );
                        Update();
                }
                break;
        case OV_NUDGEDOWN:
                if ( IsTopLevel())
                {
                        int zdiff = s.GetArgVal();
                        SetTopZ( GetTopZ() - ( zdiff ? zdiff : 1 ));
                        Update();
                }
                break;
        case OV_NUDGEUP:
                if ( IsTopLevel())
                {
                        int zdiff = s.GetArgVal();
                        SetTopZ( GetTopZ() + ( zdiff ? zdiff : 1 ));
                        Update();
                }
                break;
        case OV_MOVETO:
        case OV_P:
                MoveTo( g_Cfg.GetRegionPoint( s.GetArgStr()));
                break;
    
        case OV_PROPS:
        case OV_TWEAK:
        case OV_INFO:
                if ( ! pClientSrc )
                        return( false );
                return pClientSrc->addGumpDialogProps( GetUID());
    
        case OV_REMOVE: //remove this object now.
                Delete();
                return( true );
        case OV_REMOVEFROMVIEW:
                RemoveFromView();       // remove this item from all clients.
                return( true );
        case OV_SAY:
        case OV_SPEAK:  //speak so everyone can here
                Speak( s.GetArgStr());
                break;
    
        case OV_SAYU:
        case OV_SPEAKU:
                // Speak in unicode from the UTF8 system format.
                SpeakUTF8( s.GetArgStr(), HUE_TEXT_DEF, TALKMODE_SAY, FONT_NORMAL );
                break;
    
        case OV_SAYUA:
        case OV_SPEAKUA:
                // This can have full args. SAYUA Color, Mode, Font, Lang, Text Text
                {
                        TCHAR * pszArgs;
                        int iArgQty = Str_ParseCmds( s.GetArgRaw(), pszArgs, 5 );
                        SpeakUTF8( pszArgs,
                                (HUE_TYPE) ( pszArgs ? Exp_GetVal(pszArgs) : HUE_TEXT_DEF ), 
                                (TALKMODE_TYPE) ( pszArgs ? Exp_GetVal(pszArgs) : TALKMODE_SAY ), 
                                (FONT_TYPE) ( pszArgs ? Exp_GetVal(pszArgs) : FONT_NORMAL ), 
                                CLanguageID(pszArgs));
                }
                break;
    
        case OV_SFX:
        case OV_SOUND:
                {
                        int piCmd;
                        int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
                        Sound( piCmd, ( iArgQty > 1 ) ? piCmd : 1 );
                }
                break;
        case OV_SPELLEFFECT:    // spell, strength, noresist
                {
                        int piCmd;
                        int iArgs = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
                        if ( piCmd == 1 )
                        {
                                pCharSrc = dynamic_cast <CChar*> (this);
                        }
                        OnSpellEffect( (SPELL_TYPE) RES_GET_INDEX( piCmd ), pCharSrc, piCmd, NULL );
                }
                break;
    
        case OV_TARGET:
        case OV_TARGETG:
                {
                        if ( pClientSrc == NULL )
                                return( false );
                        pClientSrc->m_Targ_UID = GetUID();
                        pClientSrc->m_tmUseItem.m_pParent = GetParent();        // Cheat Verify
                        pClientSrc->addTarget( CLIMODE_TARG_USE_ITEM, s.GetArgStr(), index == OV_TARGETG, true );
                }
                break;
    
        case OV_DIALOG:
                {
                        if ( pClientSrc == NULL )
                                return( false );
                        pClientSrc->Dialog_Setup( CLIMODE_DIALOG, g_Cfg.ResourceGetIDType( RES_DIALOG, s.GetArgStr()), this );
                }
                break;
        case OV_TRY:
    do_try:
                // do this verb only if we can touch it.
                {
                        LPCTSTR pszVerb = s.GetArgStr();
                        if ( pSrc->GetPrivLevel() <= PLEVEL_Counsel )
                        {
                                if ( pCharSrc == NULL || ! pCharSrc->CanTouch( this ))
                                {
                                        goto failit;
                                }
                        }
    
                        if ( ! r_Verb( CScript( pszVerb ), pSrc ))
                        {
    failit:
                                pSrc->SysMessagef( "Can't %s object %s", (LPCTSTR) pszVerb, (LPCTSTR) GetName());
                                return( false );
                        }
    
                        // pSrc->SysMessagef( "%s object %s success", pszVerb, GetName());
                }
                return( true );
        case OV_TRYP:
                {
                        int iMinPriv = s.GetArgVal();
    
                        if ( iMinPriv >= PLEVEL_QTY )
                        {
                                pSrc->SysMessagef("The %s property can't be changed.", (LPCTSTR) s.GetArgStr());
                                return( false );
                        }
                        if ( pSrc->GetPrivLevel() < iMinPriv )
                        {
                                pSrc->SysMessagef( "You lack the privilege to change the %s property.", (LPCTSTR) s.GetArgStr())
;
                                return( false );
                        }
                }
                goto do_try;
    
        case OV_UID:
                // block anyone from ever doing this.
                if ( pSrc )
                {
                        pSrc->SysMessage( "Setting the UID this way is not allowed" );
                }
                return( false );
    
        case OV_UPDATE:
                Update();
                break;
        case OV_UPDATEX:
                // Some things like equipped items need to be removed before they can be updated !
                RemoveFromView();
                Update();
                break;
        case OV_USEITEM:
                if ( ! pCharSrc )
                        return( false );
                return pCharSrc->Use_Obj( this, false );
    
        case OV_FIX:
                s.GetArgStr() = '\0';
        case OV_Z:      //      ussually in "SETZ" form
                if ( IsItemEquipped())
                        return( false );
                if ( s.HasArgs())
                {
                        SetUnkZ( s.GetArgVal());
                }
                else if ( IsTopLevel())
                {
                        WORD wBlockFlags = CAN_C_WALK;
                        SetTopZ( g_World.GetHeightPoint( GetTopPoint(), wBlockFlags, GetTopPoint().GetRegion(REGION_TYPE_MULTI))
);
                }
                Update();
                break;
    
        default:
                return( false );
        }
        return( true );
    }
    
    void CObjBase::RemoveFromView( CClient * pClientExclude )
    {
        // Remove this item from all clients.
        // In a destructor this can do funny things.
    
        if ( IsDisconnected())
                return; // not in the world.
    
        CObjBaseTemplate * pObjTop = GetTopLevelObj();
    
        for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
        {
                if ( pClientExclude == pClient )
                        continue;
                CChar * pChar = pClient->GetChar();
                if ( pChar == NULL )
                        continue;
                if ( pChar->GetTopDist( pObjTop ) > UO_MAP_VIEW_RADAR )
                        continue;
                pClient->addObjectRemove( this );
        }
    }
    
    void CObjBase::DeletePrepare()
    {
        // Prepare to delete.
        RemoveFromView();
        RemoveSelf();   // Must remove early or else virtuals will fail.
        ASSERT( GetParent() == NULL );
        ASSERT( IsDisconnected());      // It is no place in the world.
    }
    
    void CObjBase::Delete()
    {
        DeletePrepare();
        ASSERT( IsDisconnected());      // It is no place in the world.
        g_World.m_ObjDelete.InsertHead(this);
    }