    // CClientDialog.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    // Special gumps for the client.
    
    #include "graysvr.h"        // predef header.
    #include "cclient.h"
    
    static LPCTSTR const sm_pszDialogTags =             // GUMPCTL_QTY is more !
    {
        "resizepic",
        "gumppic",
        "tilepic",
        "text",
        "croppedtext",
        "htmlgump",
        "xmfhtmlgump",
        "button",
        "radio",
        "checkbox",
        "textentry",    // textentry    // 7 = x,y,widthpix,widthchars,wHue,gumpid,startstringindex
        "page",
        "group",
        "nomove",
        "noclose",
        "nodispose",
        NULL,
    };
    
    TRIGRET_TYPE CClient::Dialog_OnButton( RESOURCE_ID_BASE rid, DWORD dwButtonID, CObjBase * pObj, CDialogResponseArgs * pArgs 
)
    {
        // one of the gump dialog buttons was pressed.
        if ( pObj == NULL )             // object is gone ?
                return TRIGRET_ENDIF;
    
        CResourceLock s;
        if ( ! g_Cfg.ResourceLock( s, RESOURCE_ID( RES_DIALOG, rid.GetResIndex(), RES_DIALOG_BUTTON )))
        {
                return TRIGRET_ENDIF;
        }
    
        while ( s.ReadKeyParse())
        {
                if ( ! s.IsKeyHead( "ON", 2 ))
                        continue;
                if ( s.GetArgVal() != dwButtonID )
                        continue;
                return pObj->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, m_pChar, pArgs );
        }
    
        return( TRIGRET_ENDIF );
    }
    
    bool CClient::Dialog_Setup( CLIMODE_TYPE mode, RESOURCE_ID_BASE rid, CObjBase * pObj )
    {
        if ( pObj == NULL )
                return( false );
    
        CResourceLock s;
        if ( ! g_Cfg.ResourceLock( s, rid ))
        {
                return false;
        }
    
        // read the size.
        if ( ! s.ReadKey())
        {
                return( false );
        }
    
        // starting x,y location.
        int iSizes;
        Str_ParseCmds( s.GetKeyBuffer(), iSizes, COUNTOF(iSizes));
        int x = iSizes;
        int y = iSizes;
    
        CGString sControls;
        int iControls = 0;
        while ( s.ReadKey())
        {
                if ( iControls >= COUNTOF(sControls)-1 )
                        break;
    
                // The first part of the key is GUMPCTL_TYPE
                TCHAR * pszCmd = s.GetKeyBuffer();
                GETNONWHITESPACE( pszCmd );
                int iCmd = FindTableHead( pszCmd, sm_pszDialogTags, COUNTOF(sm_pszDialogTags)-1 );
                if ( iCmd < 0 )
                {
                        DEBUG_ERR(( "Bad Gump Dialog Command '%s'\n", pszCmd ));
                        continue;
                }
    
                pObj->ParseText( pszCmd, m_pChar );
                sControls = s.GetKey();
    
                // ? count how much text we will need.
        }
    
        if ( ! g_Cfg.ResourceLock( s, RESOURCE_ID( RES_DIALOG, rid.GetResIndex(), RES_DIALOG_TEXT )))
        {
                return false;
        }
    
        CGString sText;
        int iTexts=0;
        while ( s.ReadKey())
        {
                if ( iTexts >= COUNTOF(sText)-1 )
                        break;
                pObj->ParseText( s.GetKeyBuffer(), m_pChar );
                sText = s.GetKey();
        }
    
        // Now pack it up to send,
        m_tmGumpDialog.m_ResourceID = rid;
    
        addGumpDialog( mode, sControls, iControls, sText, iTexts, x, y, pObj );
        return( true );
    }
    
    void CClient::addGumpDialog( CLIMODE_TYPE mode, const CGString * psControls, int iControls, const CGString * psText, int iTe
xts, int x, int y, CObjBase * pObj )
    {
        // Add a generic GUMP menu.
        // Should return a Event_GumpDialogRet
        // NOTE: These packets can get rather LARGE.
        // x,y = where on the screen ?
    
        if ( pObj == NULL )
                pObj = m_pChar;
        int lengthControls=1;
        int i=0;
        for ( ; i < iControls; i++)
        {
                lengthControls += psControls.GetLength() + 2;
        }
    
        int lengthText = lengthControls + 20 + 3;
        for ( i=0; i < iTexts; i++)
        {
                int lentext2 = psText.GetLength();
                DEBUG_CHECK( lentext2 < MAX_TALK_BUFFER );
                lengthText += (lentext2*2)+2;
        }
    
        // Send the fixed length stuff
        CCommand cmd;
        cmd.GumpDialog.m_Cmd = XCMD_GumpDialog;
        cmd.GumpDialog.m_len = lengthText;
        cmd.GumpDialog.m_UID = pObj->GetUID();
        cmd.GumpDialog.m_context = mode;
        cmd.GumpDialog.m_x = x;
        cmd.GumpDialog.m_y = y;
        cmd.GumpDialog.m_lenCmds = lengthControls;
        xSend( &cmd, 21 );
    
        for ( i=0; i<iControls; i++)
        {
                CGString sTmp;
                sTmp.Format( "{%s}", (LPCTSTR) psControls );
                xSend( sTmp, sTmp.GetLength() );
        }
    
        // Pack up the variable length stuff
        BYTE Pkt_gump2;
        Pkt_gump2 = '\0';
        PACKWORD( &Pkt_gump2, iTexts );
        xSend( Pkt_gump2, 3);
    
        // Pack in UNICODE type format.
        for ( i=0; i < iTexts; i++)
        {
                int len1 = psText.GetLength();
    
                NWORD len2;
                len2 = len1;
                xSend( &len2, sizeof(NWORD));
                if ( len1 )
                {
                        NCHAR szTmp;
                        int len3 = CvtSystemToNUNICODE( szTmp, COUNTOF(szTmp), psText, -1 );
                        xSend( szTmp, len2*sizeof(NCHAR));
                }
        }
    
        m_tmGumpDialog.m_UID = pObj->GetUID();
    
        SetTargMode( mode );
    }
    
    bool CClient::addGumpDialogProps( CGrayUID uid )
    {
        // put up a prop dialog for the object.
        CObjBase * pObj = uid.ObjFind();
        if ( pObj == NULL )
                return false;
        if ( m_pChar == NULL )
                return( false );
        if ( ! m_pChar->CanTouch( pObj ))       // probably a security issue.
                return( false );
    
        m_Prop_UID = m_Targ_UID = uid;
        if ( uid.IsChar())
        {
                addSkillWindow( SKILL_QTY ); // load the targets skills
        }
    
        CGString sName;
        sName.Format( pObj->IsItem() ? "d_ITEMPROP1" : "d_CHARPROP1" );
    
        RESOURCE_ID rid = g_Cfg.ResourceGetIDType( RES_DIALOG, sName );
        if ( ! rid.IsValidUID())
                return false;
    
        Dialog_Setup( CLIMODE_DIALOG, rid, pObj );
        return( true );
    }
    
    struct CClientSortList : public CGPtrSortArray< CClient * >
    {
    public:
        int m_iSortType;
    public:
        virtual int QCompare( int i, CClient * pServ );
        void SortByType( int iType, CChar * pCharSrc );
    };
    
    int CClientSortList::QCompare( int left, CClient * pClient )
    {
        CClient * pClientComp = GetAt(left);
        ASSERT(pClientComp);
        switch(m_iSortType)
        {
        case 0: // socket number
                return( pClientComp->m_Socket.GetSocket() - pClient->m_Socket.GetSocket() );
        case 1:
        default:
                return( strcmpi( pClientComp->GetName(), pClient->GetName())); 
    
        }
    }
    
    void CClientSortList::SortByType( int iType, CChar * pCharSrc )
    {
        // 1 = player name
    
        ASSERT(pCharSrc);
        int iSize = g_Serv.m_Clients.GetCount();
        SetCount( iSize );
    
        CClient * pClient = g_Serv.GetClientHead();
        for ( int i=0; pClient; pClient = pClient->GetNext())
        {
                if ( pClient->GetChar() == NULL )
                        continue;
                if ( ! pCharSrc->CanDisturb( pClient->GetChar()))
                        continue;
                SetAt( i, pClient );
                i++; 
        }
    
        SetCount( i );
        if ( i <= 1 )
                return;
    
        m_iSortType = iType;
        if ( iType < 0 )
                return;
    
        // Now quicksort the list to the needed format.
        QSort();
    }
    
    void CClient::addGumpDialogAdmin( int iAdPage, int iSortType )
    {
        // Alter this routine at your own risk....if anymore
        // bytes get sent to the client, you WILL crash them
        // Take away, but don't add (heh) (actually there's like
        // 100 bytes available, but they get eaten up fast with
        // this gump stuff)
    
        static LPCTSTR const sm_szGumps = // These are on every page and don't change
        {
                "page 0",
                "resizepic 0 0 5120 623 310",
                "resizepic 242 262 5120 170 35",
                "text 230 10 955 0",
        };
    
        CGString sControls;
        int iControls = 0;
    
        while ( iControls<COUNTOF(sm_szGumps))
        {
                sControls = sm_szGumps;
                iControls++;
        }
    
        CGString sText;
        int iTexts=1;
        sText.Format( "Admin Control Panel (%d clients)", g_Serv.m_Clients.GetCount());
    
        static const int sm_iColSpaces = // These are on every page and don't change
        {
                8 + 19,         // Text (sock)
                8 + 73,         // Text (account name)
                8 + 188,        // Text (name)
                8 + 328,        // Text (ip)
                8 + 474         // Text (location)
        };
    
        static LPCTSTR const sm_szColText = // These are on every page and don't change
        {
                "Sock",
                "Account",
                "Name",
                "IP Address",
                "Location"
        };
    
        while ( iTexts<=COUNTOF(sm_szColText) )
        {
                sControls.Format( "text %d 30 955 %d", sm_iColSpaces-2, iTexts );
                iControls++;
    
                sText = sm_szColText;
                iTexts++;
        }
    
        // Count clients to show.
        // Create sorted list.
        CClientSortList ClientList;
        ClientList.SortByType( 1, m_pChar );
    
        // none left to display for this page ?
        int iClient = iAdPage*ADMIN_CLIENTS_PER_PAGE;
        if ( iClient >= ClientList.GetCount())
        {
                iAdPage = 0;    // just go back to first page.
                iClient = 0;
        }
    
        m_tmGumpAdmin.m_iPageNum = iAdPage;
        m_tmGumpAdmin.m_iSortType = iSortType;
    
        int iY = 50;
    
        for ( int i=0; iClient<ClientList.GetCount() && i<ADMIN_CLIENTS_PER_PAGE; iClient++, i++ )
        {
                CClient * pClient = ClientList.GetAt(iClient);
                ASSERT(pClient);
    
                // Sock buttons
                //      X, Y, Down gump, Up gump, pressable, iPage, id
                sControls.Format(
                        "button 12 %i 2362 2361 1 1 %i",
                        iY + 4, // Y
                        901 + i );      // id
    
                for ( int j=0; j<COUNTOF(sm_iColSpaces); j++ )
                {
                        sControls.Format(
                                "text %i %i 955 %i",
                                sm_iColSpaces,
                                iY,
                                iTexts+j );
                }
    
                CSocketAddress PeerName = pClient->m_Socket.GetPeerName();
    
                TCHAR accountName;
                strcpylen( accountName, pClient->GetName(), 12 );
                // max name size for this dialog....otherwise CRASH!!!, not a big enuf buffer :(
    
                CChar * pChar = pClient->GetChar();
                if ( pClient->IsPriv(PRIV_GM) || pClient->GetPrivLevel() >= PLEVEL_Counsel )
                {
                        memmove( accountName+1, accountName, 13 );
                        accountName = ( pChar && pChar->IsDND()) ? '*' : '+';
                }
    
                ASSERT( i<COUNTOF(m_tmGumpAdmin.m_Item));
                if ( pChar != NULL )
                {
                        m_tmGumpAdmin.m_Item = pChar->GetUID();
    
                        TCHAR characterName;
                        strcpy(characterName, pChar->GetName());
                        characterName = 0; // same comment as the accountName...careful with this stuff!
                        sText.Format("%x", pClient->m_Socket.GetSocket());
                        sText = accountName;
                        sText = characterName;
    
                        CPointMap pt = pChar->GetTopPoint();
                        sText.Format( "%s", PeerName.GetAddrStr()) ;
                        sText.Format( "%d,%d,%d ",
                                pt.m_x,
                                pt.m_y,
                                pt.m_z,
                                pt.m_mapplane) ;
                }
                else
                {
                        m_tmGumpAdmin.m_Item = 0;
    
                        sText.Format( "%03x", pClient->m_Socket.GetSocket());
                        sText = accountName;
                        sText = "N/A";
                        sText = PeerName.GetAddrStr();
                        sText = "N/A";
                }
    
                iY += 20; // go down a 'line' on the dialog
        }
    
        if ( m_tmGumpAdmin.m_iPageNum ) // is there a previous ?
        {
                sControls = "button 253 267 5537 5539 1 0 801"; // p
        }
        if ( iClient<ClientList.GetCount())     // is there a next ?
        {
                sControls = "button 385 267 5540 5542 1 0 802"; // n
        }
    
        addGumpDialog( CLIMODE_DIALOG_ADMIN, sControls, iControls, sText, iTexts, 0x05, 0x46 );
    }