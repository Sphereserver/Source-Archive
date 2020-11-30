    // CResourceTest.cpp
    // Copyright Menace Software (www.menasoft.com).
    //
    
    #include "graysvr.h"        // predef header.
    
    #include <signal.h>
    
    bool ResourceWriteLink( int id, CResourceLink * pLink, LPCTSTR pszKey, LPCTSTR pszVal )
    {
        // Write to an alternate file that we are not reading from !
        ASSERT(pLink);
        CResourceScript * pResFile = pLink->GetLinkFile();
        ASSERT(pResFile);
    
        TCHAR szSectionName;
    
        CGString sTmpName = CGFile::GetMergedFileName( "d:\\menace\\scripts\\tmp", pResFile->GetFileTitle() );
        CScript s;
        if ( ! s.Open( sTmpName, OF_NONCRIT ))
        {
                // Copy it to the Tmp dir if we can't find it !
                if ( ! pResFile->CopyFileTo( sTmpName ))
                {
    bailout:
                        DEBUG_ERR(( "Can't find item 0%x file '%s'" DEBUG_CR, id, (LPCTSTR) sTmpName ));
                        return( false );
                }
                if ( ! s.Open( sTmpName ))
                {
                        goto bailout;
                }
        }
    
        sprintf( szSectionName, "ITEMDEF 0%x", id );
        if ( ! s.FindSection( szSectionName, 0 ))
        {
                sprintf( szSectionName, "ITEMDEF %s", pLink->GetResourceName() );
                if ( ! s.FindSection( szSectionName, 0 ))
                {
                        DEBUG_ERR(( "Can't find item 0%x name '%s' in file '%s'" DEBUG_CR, id, (LPCTSTR) pLink->GetResourceName(
), (LPCTSTR) sTmpName ));
                        return( false );
                }
        }
    
        ASSERT(pszKey);
        // is it already set this way ?
        if ( s.FindKey( pszKey )) // Find a key in the current section
        {
                if ( pszVal && ! strcmp( pszVal, s.GetArgRaw()))
                        return( true );
        }
        else
        {
                if ( pszVal == NULL )
                        return( true );
        }
    
        s.WriteProfileStringSec( szSectionName, pszKey, pszVal );
        return( true );
    }
    
    static int sm_iResourceChanges;
    
    bool CResource::ResourceTestSkills()
    {
        // '8'
        // Move the RESOURCE= and the TEST for skills in SPHERESKILL.SCP to proper ITEMDEF entries.
        // RES_SKILLMENU
        //CARTOGRAPHY
        //BOWCRAFT
        //BLACKSMITHING
        //CARPENTRY
        //TAILORING
        //TINKERING
    
        sm_iResourceChanges = 0;
    
        for ( int i=0; i<COUNTOF(m_ResHash.m_Array); i++ )
        for ( int j=0; j<m_ResHash.m_Array.GetCount(); j++ )
        {
                CResourceLink* pResSkill = dynamic_cast <CResourceLink*>(m_ResHash.m_Array);
                if ( pResSkill == NULL )
                        continue;
                if ( pResSkill->GetResType() != RES_SKILLMENU )
                        continue;
    
                CResourceLock s;
                if ( ! pResSkill->ResourceLock(s))
                {
                        return( false );
                }
    
                // any place "MAKEITEM" is used.
    
                CGString sResources;
                CGString sSkillMake;
                while ( s.ReadKeyParse())
                {
                        if ( s.IsKeyHead( "ON", 2 ))
                        {
                                sResources.Empty();
                                continue;
                        }
                        if ( s.IsKey( "TEST" ))
                        {
                                sSkillMake = s.GetArgRaw();
                                continue;
                        }
                        if ( s.IsKey( "RESOURCES" ))
                        {
                                sResources = s.GetArgRaw();
                                continue;
                        }
                        if ( s.IsKey( "MAKEITEM" ) && ! sResources.IsEmpty())
                        {
                                // this should be last.
                                // Write out what we learned here !
                                ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgRaw() );
                                CItemBase * pItemDef = CItemBase::FindItemBase( (ITEMID_TYPE) id );
    
                                if ( pItemDef == NULL || id <= 0)
                                {
                                        DEBUG_ERR(( "Cant MAKEITEM '%s'" DEBUG_CR, (LPCTSTR) s.GetArgRaw()));
                                        continue;
                                }
    
                                if ( pItemDef->m_BaseResources.GetCount())
                                {
                                        // we already have resources !
                                        // DEBUG_ERR(( "Resources conflict for '%s'" DEBUG_CR, (LPCTSTR) pItemDef->GetResourceNa
me()));
                                        CResourceQtyArray SimpResources;
                                        SimpResources.Load( sResources );
                                        if ( ! ( SimpResources == pItemDef->m_BaseResources ))
                                        {
                                                ResourceWriteLink( id, pItemDef, "RESOURCES", sResources );
                                                pItemDef->r_WriteVal( "RESOURCES", sResources, NULL );
                                                ResourceWriteLink( id, pItemDef, "RESOURCES2", sResources );
                                        }
                                }
                                else
                                {
                                        ResourceWriteLink( id, pItemDef, "RESOURCES", sResources );
                                }
    
                                // Just overwrite the skill to make.
    #if 0
                                if ( pItemDef->m_SkillMake.GetCount())
                                {
                                        CResourceQtyArray SkillMake;
                                        SkillMake.Load( sSkillMake );
                                        if ( ! ( SkillMake == pItemDef->m_SkillMake ))
                                        {
                                                DEBUG_ERR(( "SkillMake conflict for '%s'" DEBUG_CR, (LPCTSTR) pItemDef->GetResou
rceName()));
                                        }
                                }
                                else
    #endif
                                {
                                        while ( true )
                                        {
                                                TCHAR * pEquals = strchr( (TCHAR*)(LPCTSTR)sSkillMake, '=' );
                                                if ( pEquals == NULL )
                                                        break;
                                                *pEquals = ' ';
                                        }
                                        ResourceWriteLink( id, pItemDef, "SKILLMAKE", sSkillMake );
                                }
    
                                continue;
                        }
                }
    
                s.Close();
        }
    
        return( true );
    }
    
    static void ResourceTestSortMove( CGString * sLines, int iTo, int iFrom )
    {
        // NOTE : size we are just moving something we do not need to know the total size.
        if ( iFrom == iTo )
                return;
    
        sm_iResourceChanges++;
    
        ASSERT(iFrom>=0);
        ASSERT(iTo>=0);
        BYTE Tmp;
        memcpy( Tmp, &sLines, sizeof(CGString));
        if ( iFrom < iTo )
        {
                memmove( &sLines, &sLines, sizeof(CGString)*((iTo-iFrom)));
                iTo--;
        }
        else
        {
                memmove( &sLines, &sLines, sizeof(CGString)*((iFrom-iTo)));
        }
        memcpy( &sLines, Tmp, sizeof(CGString));
    }
    
    static int ResourceTestSortSection( LPCTSTR psLastHead, CGString * sLines, int iLines )
    {
        static LPCTSTR const sm_szCharKeysDef = // CCharBase
        {
                "DEFNAME",
                "DEFNAME2",
                "NAME",
                "ID",
                "CAN",
                "ICON",
                "ANIM",
                "SOUND",
                "BLOODCOLOR",
                "SHELTER",
                "AVERSIONS",
                "ATT",
                "ARMOR",
                "RESOURCES",
                "DESIRES",
                "FOODTYPE",
                "HIREDAYWAGE",
                "JOB",
                "MAXFOOD",
        //      "DEX",
        //      "STR",
                "TEVENTS",
                "TSPEECH",
                "DAM",
                "ARMOR",
    
                "CATEGORY",
                "DESCRIPTION",
                "SUBSECTION",
        };
    
        static LPCTSTR const sm_szCharKeysCreate =
        {
                "STR",
                "DEX",
                "INT",
                "COLOR",
                "BODY",
                "FLAGS",
    
                "ALCHEMY",
                "ANATOMY",
                "ANIMALLORE",
                "ITEMID",
                "ARMSLORE",
                "PARRYING",
                "BEGGING",
                "BLACKSMITHING",
                "BOWCRAFT",
                "PEACEMAKING",
                "CAMPING",
                "CARPENTRY",
                "CARTOGRAPHY",
                "COOKING",
                "DETECTINGHIDDEN",
                "ENTICEMENT",
                "EVALUATINGINTEL",
                "HEALING",
                "FISHING",
                "FORENSICS",
                "HERDING",
                "HIDING",
                "PROVOCATION",
                "INSCRIPTION",
                "LOCKPICKING",
                "MAGERY",
                "MAGICRESISTANCE",
                "TACTICS",
                "SNOOPING",
                "MUSICIANSHIP",
                "POISONING",
                "ARCHERY",
                "SPIRITSPEAK",
                "STEALING",
                "TAILORING",
                "TAMING",
                "TASTEID",
                "TINKERING",
                "TRACKING",
                "VETERINARY",
                "SWORDSMANSHIP",
                "MACEFIGHTING",
                "FENCING",
                "WRESTLING",
                "LUMBERJACKING",
                "MINING",
                "MEDITATION",
                "STEALTH",
                "REMOVETRAP",
                "NECROMANCY",
    
                "NPC",
                "FAME",
                "KARMA",
                "NEED",
    
                "SPEECH",
                "EVENTS",
                "TIMER",
                "TIMED",
        };
    
        static LPCTSTR const sm_szCharKeysEnd =
        {
                "CONTAINER",
                "ITEM",
                "ITEMNEWBIE",
                "ON",
                "ONTRIGGER",
        };
    
        static LPCTSTR const sm_szItemKeysDef =
        {
                "DEFNAME",
                "DEFNAME2",
                "NAME",
                "ID",
                "TYPE",
                "VALUE",
    
        "TDATA1",
        "TDATA2",
        "TDATA3",
        "TDATA4",
    
        "FLIP",
        "DYE",
        "LAYER",
        "PILE",
        "REPAIR",
        "REPLICATE",
        "REQSTR",
        "SKILL",
        "SKILLMAKE",
    //  "SPEED",
        "TWOHANDS",
        "WEIGHT",
        "DUPELIST",
        "DUPEITEM",
                "RESOURCES",
                "RESOURCES2",
                "DAM",
                "ARMOR",
    
                "MULTIREGION",
                "COMPONENT",
    
                "CATEGORY",
                "DESCRIPTION",
                "SUBSECTION",
        };
    
        static LPCTSTR const sm_szItemKeysCreate =
        {
                "COLOR",
        "AMOUNT",
        "ATTR",
    //  "CONT",
    //  "DISPID",
    //  "DISPIDDEC",
        "FRUIT",
        "HITPOINTS",    // for weapons
    //  "ID",
        "LAYER",
    //  "LINK",
        "MORE",
        "MORE1",
        "MORE2",
        "MOREP",
        "MOREX",
        "MOREY",
        "MOREZ",
        "TIMER",
        "TIMED",
    //  "P",
        };
    
        static LPCTSTR const sm_szItemKeysEnd =
        {
                "ON",
                "ONTRIGGER",
        };
    
        static LPCTSTR const sm_szItemKeysDupe =
        {
                "DUPEITEM",
                "CATEGORY",
                "DESCRIPTION",
                "SUBSECTION",
                "DEFNAME",
                "DEFNAME2",
        };
    
        static LPCTSTR const sm_szKeysDelete =
        {
                "SELLVALUE",
                "MATERIAL",
                "NOINDEX",
                // "SIMPRESOURCES",
        };
    
        if ( iLines == 0 )
                return 0;
    
        // what type of resource is this ?
        LPCTSTR const* pszKeysDef;
        int iKeysDef;
        LPCTSTR const* pszKeysCreate;
        int iKeysCreate;
        LPCTSTR const* pszKeysEnd;
        int iKeysEnd;
        RES_TYPE restype;
    
        if ( ! strnicmp( psLastHead, " != '/' )
                        break;
        }
    
        int iLineCreate = -1;
        int iLineEnd = iLineStart;
    
        bool fItemDupe = false;
    
        static LPCTSTR const sm_szMultiValidKey =
        {
                "COMPONENT",
                "DEFNAME2",
                "TSPEECH",
                "TEVENT",
                "SPEECH",
                "EVENT",
                "ON",
        };
    
        // Find the end and ON=@Create
        for ( ; iLineEnd < iLines; iLineEnd++ )
        {
                LPCTSTR pszTag = sLines;
                GETNONWHITESPACE(pszTag);
    
                // First look for multiple instances of this .
                if ( pszTag && pszTag != '/' )
                for ( int j=iLineStart; j<iLineEnd; j++ )
                {
                        LPCTSTR pszTest = sLines;
                        GETNONWHITESPACE(pszTest);
    
                        if ( ! strcmpi( pszTest, pszTag ))
                        {
                                DEBUG_ERR(( "Multi Instance of line %s" DEBUG_CR, (LPCTSTR) pszTag ));
                                break;
                        }
    
                        for ( int k=0; true; k++ )
                        {
                                bool fEnd1 = ! _ISCSYM( pszTag );
                                bool fEnd2 = ! _ISCSYM( pszTest );
                                if ( fEnd1 && fEnd2 )
                                {
                                        // NOTE: It's ok to duplicate some keys.
                                        if ( FindTableHead( pszTag, sm_szMultiValidKey, COUNTOF(sm_szMultiValidKey)) >= 0 )
                                                break;
    
                                        DEBUG_ERR(( "Multi Instance of key %s" DEBUG_CR, (LPCTSTR) pszTag ));
                                        break;
                                }
                                if ( toupper( pszTag ) != toupper( pszTest ))
                                        break;  // no match
                        }
                }
    
                if ( ! strnicmp( pszTag, "ON=@Create", 10 ))
                {
                        iLineCreate = iLineEnd;
                        continue;
                }
    
                if ( ! strnicmp( pszTag, "DEFNAME=", 8 ))
                {
                        if ( strnicmp( pszTag+8, ( restype == RES_ITEMDEF ) ? "i_" : "c_", 2 ))
                        {
                                DEBUG_WARN(( "DEFNAME '%s' does not match convention" DEBUG_CR, (LPCTSTR) (pszTag+8) ));
                        }
                        _strlwr( (TCHAR*)( pszTag+8 ));
    
                        if ( iLineStart == iLineEnd )   // fine where it is.
                                continue;
    
                        if ( iLineEnd && ! strnicmp( sLines, "DEFNAME=", 8 ))
                        {
                                // make this DEFNAME2 !
                                DEBUG_WARN(( "Multi DEFNAMEs" DEBUG_CR ));
                                continue;
                        }
    
                        // put this at the top
                        ResourceTestSortMove( sLines, iLineStart, iLineEnd );
                        continue;
                }
    
                if ( ! strnicmp( pszTag, "DUPEITEM=", 9 ))
                {
                        fItemDupe = true;
                        continue;
                }
    
                int j = FindTableHead( pszTag, pszKeysEnd, iKeysEnd );
                if ( j >= 0 )
                        break;
    
                j = FindTableHead( pszTag, sm_szKeysDelete, COUNTOF(sm_szKeysDelete));
                if ( j >= 0 )
                {
                        ResourceTestSortMove( sLines, iLines, iLineEnd );
                        iLineEnd--;
                        iLines--;
                        continue;
                }
        }
    
        // Now sort the main defs section
    
        for ( int i=iLineStart; i<iLineEnd; i++ )
        {
                // where does this line go ?
                if ( iLineCreate >= 0 && i >= iLineCreate )     // done.
                {
                        break;
                }
    
                LPCTSTR pszTag = sLines;
                if ( pszTag == '\n' )
                        continue;
                if ( pszTag == '/' )
                        continue;
    
                GETNONWHITESPACE(pszTag);
    
                if ( fItemDupe )        // this is a DUPEITEM
                {
                        int j = FindTableHead( pszTag, sm_szItemKeysDupe, COUNTOF(sm_szItemKeysDupe));
                        if ( j < 0 )
                        {
                                DEBUG_ERR(( "Needless duplication key '%s'" DEBUG_CR, (LPCTSTR) pszTag ));
    #if 0
                                ResourceTestSortMove( sLines, iLines, i );
                                i--;
                                iLineEnd--;
                                iLines--;
    #endif
                                continue;
                        }
                }
    
                int j = FindTableHead( pszTag, pszKeysDef, iKeysDef );
                if ( j >= 0 )
                {
                        continue;
                }
    
                j = FindTableHead( pszTag, pszKeysCreate, iKeysCreate );
                if ( j >= 0 )
                {
                        // must move this down to the create section.
    
                        if ( iLineCreate < 0 )
                        {
                                // there is none. so we must create it.
                                sLines = "ON=@Create\n";
                                ResourceTestSortMove( sLines, iLineEnd, iLines );
                                iLineCreate = iLineEnd;
                                iLines++;
                        }
    
                        ResourceTestSortMove( sLines, iLineCreate+1, i );
                        iLineCreate--;
                        i--;
                        continue;
                }
    
                DEBUG_ERR(( "Unknown key '%s'" DEBUG_CR, (LPCTSTR) pszTag ));
    
    #if 0
                        ResourceTestSortMove( sLines, iLines, i );
                        i--;
                        iLineEnd--;
                        iLines--;
    #endif
    
                continue;
        }
    
        // Now look at stuff after the ON=@Create
    
        for ( ++i; i<iLineEnd; i++ )
        {
                ASSERT( iLineCreate >= 0 );
    
                LPCTSTR pszTag = sLines;
    
                if ( pszTag == '\n' )
                        continue;
                if ( pszTag == '/' )
                        continue;
                GETNONWHITESPACE(pszTag);
    
                int j = FindTableHead( pszTag, pszKeysCreate, iKeysCreate );
                if ( j >= 0 )
                {
                        continue;
                }
    
                j = FindTableHead( pszTag, pszKeysDef, iKeysDef );
                if ( j >= 0 )
                {
                        ResourceTestSortMove( sLines, iLineCreate-1, i );
                        continue;
                }
    
                DEBUG_ERR(( "Unknown key '%s'" DEBUG_CR, (LPCTSTR) pszTag ));
    #if 0
                        ResourceTestSortMove( sLines, iLines, i );
                        i--;
                        iLineEnd--;
                        iLines--;
    #endif
                continue;
        }
    
        // now look for strange blanks.
    
        return( iLines );
    }
    
    bool CResource::ResourceTestSort( LPCTSTR pszFilename )
    {
        // '7' = test sort
    
        ASSERT(pszFilename);
        if ( pszFilename == '*' )
        {
                // do all the scripts.
                int i=0;
                for ( ; true; i++ )
                {
                        CResourceScript* pResFile = GetResourceFile( i );
                        if ( pResFile == NULL )
                                break;
                        ResourceTestSort( pResFile->GetFilePath());
                }
    
                return( true );
        }
    
        CScript sFileInp;
        CScript * pFileInp;
    
        CResourceScript * pResFile = FindResourceFile( pszFilename );
        if ( pResFile == NULL )
        {
                sFileInp.SetFilePath( pszFilename );
                pFileInp = &sFileInp;
        }
        else
        {
                pResFile->Close();
                pFileInp = pResFile;
        }
    
        if ( ! pFileInp->Open( NULL, OF_READ ))
        {
                DEBUG_ERR(( "Can't load resource script '%s'" DEBUG_CR, (LPCTSTR) pFileInp->GetFilePath() ));
                return false;
        }
    
        CGString sOutName;
        sOutName = "tmpsort.scp";
        CScript sFileOut;
        if ( ! sFileOut.Open( sOutName, OF_WRITE ))
        {
                DEBUG_ERR(( "Can't open resource output script '%s'" DEBUG_CR, (LPCTSTR) sOutName ));
                return( false );
        }
    
        CScriptFileContext context( pFileInp ); // set this as the error context.
        long lSizeInp = pFileInp->GetLength();
    
        CGString sLastHead;
        bool fInSection = false;
        int iTotalLines = 0;
        int iLines=0;
        CGString sLines;
    
        sm_iResourceChanges = 0;
        while ( true )
        {
                if ( ! pFileInp->ReadTextLine( false ))
                {
                        break;
                }
    
                LPCTSTR pszKey = pFileInp->GetKey();
    
                iTotalLines++;
                if ( ! ( iTotalLines & 0x3f ))
                {
                        g_Serv.PrintPercent( pFileInp->GetPosition(), lSizeInp );
                }
    
                if ( ! fInSection )
                {
                        sFileOut.WriteString(pszKey);
    
                        if ( pszKey == ' == ' == '\0' || sLines == '\n' ))
                                {
                                        iLines--;
                                }
                                // Now write out all the lines in the section.
                                for ( int i=0; i<iLines; i++ )
                                {
                                        sFileOut.WriteString(sLines);
                                }
                                sFileOut.WriteString("\n");     // spacer to the next section.
                                iLines=0;
                        }
    
                        sLastHead = pszKey;
                        sFileOut.WriteString(pszKey);
                }
                else
                {
                        if ( iLines >= COUNTOF(sLines)-1 )
                        {
                                DEBUG_ERR(( "Section '%s' too large, %d changes" DEBUG_CR, (LPCTSTR) sLastHead, sm_iResourceChan
ges ));
                                return( false );
                        }
                        sLines = pszKey;
                        iLines++;
                }
        }
    
        DEBUG_MSG(( "Sort resource '%s', %d changes" DEBUG_CR, pFileInp->GetFileTitle(), sm_iResourceChanges ));
    
        // delete the old and rename to the new !
        sFileOut.Close();
        pFileInp->Close();
        if ( pResFile )
        {
                pResFile->CloseForce();
        }
    
        if ( sm_iResourceChanges )
        {
                remove( pFileInp->GetFilePath());
    
                int iRet = rename( sFileOut.GetFilePath(), pFileInp->GetFilePath() );
                if ( iRet )
                {
                        DEBUG_WARN(( "rename of %s to %s failed = 0%x!" DEBUG_CR, (LPCTSTR) sFileOut.GetFilePath(), (LPCTSTR) pF
ileInp->GetFilePath(), iRet ));
                }
        }
        else
        {
                // remove( sFileOut.GetFilePath());
        }
    
        return( true );
    }
    
    bool CResource::ResourceTestItemDupes()
    {
        // '5'
        // xref the DUPEITEM= stuff to match DUPELIST=aa,bb,cc,etc stuff
    
        // read them all in first.
        int i=1;
        for ( ; i<ITEMID_MULTI; i++ )   // ITEMID_MULTI
        {
                if ( !( i%0x1ff ))
                {
                        g_Serv.PrintPercent( i, 2*ITEMID_MULTI );
                }
    
                CItemBase * pItemDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
                if ( pItemDef == NULL )
                {
                        // Is it a defined item ?
                        CUOItemTypeRec item;
                        if ( ! CItemBase::GetItemData((ITEMID_TYPE) i, &item ))
                                continue;
    
                        // Seems we have a new item on our hands.
                        g_Log.Event( LOGL_EVENT, " // %s //New" DEBUG_CR, i, (LPCTSTR) item.m_name );
                        continue;
                }
        }
    
        // Now do XREF stuff. write out the differences.
        for ( i=1; i<ITEMID_MULTI; i++ )
        {
                if ( !( i%0x1ff ))
                {
                        g_Serv.PrintPercent( i+ITEMID_MULTI, 2*ITEMID_MULTI );
                }
    
                CItemBase * pItemDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
                if ( pItemDef == NULL )
                        continue;
                if ( pItemDef->GetResourceID().GetResIndex() != i )     // this is just a DUPEITEM
                        continue;
    
                CGString sVal;
                if ( ! pItemDef->r_WriteVal( "DUPELIST", sVal ))
                {
                        ASSERT(0);
                        continue;
                }
    
                ResourceWriteLink( i, pItemDef, "DUPELIST", ( sVal.IsEmpty() ) ? NULL : ((LPCTSTR) sVal ));
    
                CUOItemTypeRec item;
                if ( ! CItemBase::GetItemData((ITEMID_TYPE) i, &item ))
                {
                        g_Log.Event( LOGL_EVENT, " // Undefined in tiledata.mul" DEBUG_CR, i );
                }
    
                IT_TYPE type = CItemBase::GetTypeBase( (ITEMID_TYPE) i, item );
                if ( type != IT_NORMAL && ! pItemDef->IsType(type) )
                {
                        if ( pItemDef->IsType(IT_NORMAL) )
                        {
                                ResourceWriteLink( i, pItemDef, "TYPE", ResourceGetName( RESOURCE_ID( RES_TYPEDEF, type )));
                        }
                        else
                        {
                                g_Log.Event( LOGL_EVENT, " // Type mismatch %s != %s ?" DEBUG_CR, i,
                                        (LPCTSTR) ResourceGetName( RESOURCE_ID( RES_TYPEDEF, pItemDef->GetType() )),
                                        (LPCTSTR) ResourceGetName( RESOURCE_ID( RES_TYPEDEF, type )));
                        }
                }
        }
    
        return( true );
    }
    
    bool CResource::ResourceTestItemMuls()
    {
        // Xref the RES_ITEMDEF blocks with the items database to make sure we have defined all.
        int i = 0;
        for ( ; i < ITEMID_MULTI; i++ )
        {
                if ( !( i%0x1ff ))
                {
                        g_Serv.PrintPercent( i, ITEMID_MULTI );
                }
    
                CUOItemTypeRec item;
                if ( ! CItemBase::GetItemData((ITEMID_TYPE) i, &item ))
                        continue;
    
                CItemBase * pItemDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
                if ( pItemDef == NULL )
                {
                        // Seems we have a new item on our hands.
                        // Write it to the file !
                        g_Log.Event( LOGL_EVENT, " // %s //New" DEBUG_CR, i, (LPCTSTR) item.m_name );
                        continue;
                }
    
        }
        return( true );
    }
    
    bool CResource::ResourceTestCharAnims()
    {
        // xref the ANIM= lines in RES_CHARDEF with ANIM.IDX
    
        if ( ! g_Install.OpenFile( VERFILE_ANIMIDX ))
        {
                return( false );
        }
    
        for ( int id = 0; id < CREID_QTY; id ++ )
        {
                if ( id >= CREID_MAN )
                {
                        // must already have an entry in the RES_CHARDEF. (don't get the clothing etc)
    
                        continue;
                }
    
                // Does it have an entry in "ANIM.IDX" ?
    
                int index;
                int animqty;
                if ( id < CREID_HORSE1 )
                {
                        // Monsters and high detail critters.
                        // 22 actions per char.
                        index = ( id * ANIM_QTY_MON * 5 );
                        animqty = ANIM_QTY_MON;
                }
                else if ( id < CREID_MAN )
                {
                        // Animals and low detail critters.
                        // 13 actions per char.
                        index = 22000 + (( id - CREID_HORSE1 ) * ANIM_QTY_ANI * 5 );
                        animqty = ANIM_QTY_ANI;
                }
                else
                {
                        // Human and equip
                        // 35 actions per char.
                        index = 35000 + (( id - CREID_MAN ) * ANIM_QTY_MAN * 5 );
                        animqty = ANIM_QTY_MAN;
                }
    
                // It MUST have a walk entry !
    
                DWORD dwAnim = 0;       // mask of valid anims.
                for ( int anim = 0; anim < animqty; anim ++, index += 5 )
                {
                        CUOIndexRec Index;
                        if ( ! g_Install.ReadMulIndex( VERFILE_ANIMIDX, VERFILE_ANIM, index, Index ))
                        {
                                if ( anim == 0 ) break; // skip this.
                                continue;
                        }
    
                        dwAnim |= ( 1L<<anim );
                }
    
                if ( dwAnim )
                {
                        // report the valid animations.
                        CGString sVal;
                        sVal.FormatHex( dwAnim );
                        CGString sSec;
                        sSec.Format( "%04X", id );
    
                        CCharBase * pCharDef = CCharBase::FindCharBase((CREID_TYPE) id );
                        if ( pCharDef == NULL )
                        {
                                continue;
                        }
    
                        CResourceScript * pResFile = pCharDef->GetLinkFile();
                        ASSERT(pResFile);
    
                        pResFile->CloseForce();
                        pResFile->WriteProfileStringOffset( pCharDef->GetLinkOffset(), "ANIM", sVal );
                        pResFile->CloseForce();
                }
        }
        return( true );
    }
    
    #if 0
    void CResource::ResourceTestMatchItems()
    {
        // resolve all the defs for the item names.
        // Match up DEFS.SCP with the ITEM.SCP;
    
        CScript s;
        s.SetFilePath( GRAY_FILE "ITEM" GRAY_SCRIPT );
    
        int i=1;
        for ( ; i<ITEMID_MULTI; i++ )   // ITEMID_MULTI
        {
                if ( !( i%0x1ff ))
                        PrintPercent( i, ITEMID_MULTI );
                CItemBase * pItemDef = CItemBase::FindItemBase((ITEMID_TYPE) i );
                if ( pItemDef == NULL )
                        continue;
    
                if ( pItemDef->GetResourceID() != i )
                        continue;
    
                int j = g_Exp.m_VarDefs.FindValNum( i );
                if ( j<0 )
                {
                        for ( int k=0; pItemDef->GetFlippedID(k); k++ )
                        {
                                j = g_Exp.m_VarDefs.FindValNum( pItemDef->GetFlippedID(k));
                                if ( j>=0) break;
                        }
                }
                if ( j>= 0 )
                {
                        s.WriteProfileStringOffset( sdfsdf, "DEFNAME", g_Exp.m_VarDefs->GetName());
                        g_Exp.m_VarDefs.DeleteAt(j);
                }
    
                j = g_Exp.m_VarDefs.FindValNum( i );
                if ( j<0 )
                {
                        for ( int k=0; pItemDef->GetFlippedID(k); k++ )
                        {
                                j = g_Exp.m_VarDefs.FindValNum( pItemDef->GetFlippedID(k));
                                if ( j>=0) break;
                        }
                }
                if ( j>= 0 )
                {
                        s.WriteProfileStringOffset( dfsdf, "DEFNAME2", g_Exp.m_VarDefs->GetName());
                        g_Exp.m_VarDefs.DeleteAt(j);
                }
    
        }
    
        // What DEFS are left ?
        for ( i=0; i< g_Exp.m_VarDefs.GetCount(); i++ )
        {
                g_Log.Event( LOGL_EVENT, "Unused DEF = %s" DEBUG_CR, (LPCTSTR) g_Exp.m_VarDefs->GetName());
        }
    }
    
    void CResource::ResourceTestMatchChars()
    {
        // resolve all the defs as char names.
        // Match up DEFS.SCP with the CHAR.SCP;
    
        CScript s;
        s.SetFilePath( GRAY_FILE "CHAR" GRAY_SCRIPT );
    
        int i=1;
        for ( ; i<CREID_QTY; i++ )      // ITEMID_MULTI
        {
                if ( !( i%0x07 ))
                {
                        PrintPercent( i, CREID_QTY );
                }
    
                CCharBase * pCharDef = CCharBase::FindCharBase((CREID_TYPE) i );
                if ( pCharDef == NULL )
                        continue;
    
                ASSERT( pCharDef->GetResourceID() == i );
    
                int j = g_Exp.m_VarDefs.FindValNum( i );
                if ( j>= 0 )
                {
                        s.WriteProfileStringOffset( sSsdfsdfec, "DEFNAME", g_Exp.m_VarDefs->GetName());
                        g_Exp.m_VarDefs.DeleteAt(j);
                }
    
                j = g_Exp.m_VarDefs.FindValNum( i );
                if ( j>= 0 )
                {
                        s.WriteProfileStringOffset( sSsfsdfec, "DEFNAME2", g_Exp.m_VarDefs->GetName());
                        g_Exp.m_VarDefs.DeleteAt(j);
                }
        }
    
        // What DEFS are left ?
        for ( i=0; i< g_Exp.m_VarDefs.GetCount(); i++ )
        {
                g_Log.Event( LOGL_EVENT, "Unused DEF = %s" DEBUG_CR, (LPCTSTR) g_Exp.m_VarDefs->GetName());
        }
    }
    
    #endif      // 0
    
        static LPCTSTR const sm_szSkillMenuKeys =
        {
                "ON",
                "ONOPTION",
                "TEST",
                "RESOURCES",
                "RESTEST",
                "POLY",
                "SUMMON",
                "DRAWMAP",
                "REPLICATE",
                "MAKEITEM",
                "SKILLMENU",
                "REPAIR",
        };
    
    bool CResource::ResourceTest( LPCTSTR pszFilename )
    {
        // 'T' = Just run a test on this resource file.
        ASSERT(pszFilename);
        if ( pszFilename == '*' )
        {
                // do all the scripts.
                int i=0;
                for ( ; true; i++ )
                {
                        CResourceScript* pResFile = GetResourceFile( i );
                        if ( pResFile == NULL )
                                break;
                        ResourceTest( pResFile->GetFilePath());
                }
    
                return( true );
        }
    
        CResourceScript * pResFile = FindResourceFile( pszFilename );
        if ( pResFile == NULL )
        {
                pResFile = LoadResourcesAdd( pszFilename );
                if ( pResFile == NULL )
                {
                        DEBUG_ERR(( "Can't load resource script '%s'" DEBUG_CR, (LPCTSTR) pszFilename ));
                        return false;
                }
        }
    
        // NOW test all the stuff that is CResourceLink based !
        CScriptFileContext context( pResFile ); // set this for error reporting.
    
        CREID_TYPE idchar = (CREID_TYPE) ResourceGetIndexType( RES_CHARDEF, "c_h_provis" );
        CChar * pCharProvis = CChar::CreateNPC( idchar );
        if ( pCharProvis == NULL )
        {
                DEBUG_ERR(( "DEFAULTCHAR is not valid %d!" DEBUG_CR, idchar ));
        }
    
        pCharProvis->MoveToChar( CPointMap( 1, 1 ));
    
        CItemContainer * pItemCont = dynamic_cast <CItemContainer *>( CItem::CreateTemplate( ITEMID_BACKPACK, pCharProvis ));
        if ( pItemCont == NULL )
        {
                DEBUG_ERR(( "Backpack item 0%x is not a container ?" DEBUG_CR, ITEMID_BACKPACK ));
        }
    
        CItem * pItemGold = CItem::CreateTemplate( ITEMID_GOLD_C1 );
        ASSERT(pItemGold);
        pItemGold->MoveTo( CPointMap( 1, 1 ));
    
        // Now look for all the elements in it.
    
        for ( int i=0; i<COUNTOF(m_ResHash.m_Array); i++ )
        for ( int j=0; j<m_ResHash.m_Array.GetCount(); j++ )
        {
                CResourceLink* pLink = dynamic_cast <CResourceLink*>(m_ResHash.m_Array);
                if ( pLink == NULL )
                        continue;
                if ( pLink->GetLinkFile() != pResFile )
                        continue;
    
                CResourceLock s;
                if ( ! pLink->ResourceLock(s))
                {
                        continue;
                }
    
                RES_TYPE restype = pLink->GetResType();
                switch ( restype )
                {
                case RES_CHARDEF:
                        {
                                CChar * pCharNew = CChar::CreateNPC( (CREID_TYPE) pLink->GetResourceID().GetResIndex() );
                                pCharNew->Delete();
                        }
                        break;
    
                case RES_ITEMDEF:
                        {
                                CItem * pItemNew = CItem::CreateScript( (ITEMID_TYPE) pLink->GetResourceID().GetResIndex() );
                                pItemNew->MoveTo( CPointMap( 1, 1 ));
    
                                CItemBase * pItemNewDef = pItemNew->Item_GetDef();
                                ASSERT(pItemNewDef);
    
    #if 0
                                if ( pItemNewDef->m_BaseResources.GetCount() && pItemNewDef->GetBaseValueMin() > 0 )
                                {
                                        DEBUG_ERR(( "%s: Odd to have both VALUE and RESOURCES" DEBUG_CR, (LPCTSTR) pItemNewDef->
GetResourceName() ));
                                }
    #endif
    
                                // Excercise all the possible triggers.
                                for ( int j=0; j<ITRIG_QTY; j++ )
                                {
                                        // pLink
                                        pItemNew->OnTrigger( (ITRIG_TYPE) j, pCharProvis );
                                }
    
                                pItemNew->Delete();
                                delete pItemNew;
                        }
                        break;
    
                case RES_TYPEDEF:
                        {
                                pItemGold->SetType( (IT_TYPE) pLink->GetResourceID().GetResIndex() );
    
                                // Excercise all the possible triggers.
                                for ( int j=0; j<ITRIG_QTY; j++ )
                                {
                                        pItemGold->OnTrigger( (ITRIG_TYPE) j, pCharProvis );
                                }
                        }
                        break;
                case RES_MENU:
                        // read in all the options to make sure they are valid.
                        {
                                int iOnHeaders = 0;
                                for ( int k=0; s.ReadKeyParse(); k++ )
                                {
                                        if ( ! k )
                                                continue;
    
                                        if ( s.IsKeyHead( "ON", 2 ))
                                        {
                                                iOnHeaders ++;
                                                CMenuItem menuitem;
                                                if ( ! menuitem.ParseLine( s.GetArgRaw(), pCharProvis, pCharProvis ))
                                                {
                                                }
                                                continue;
                                        }
    
                                        if ( ! iOnHeaders )
                                                continue;
    
                                        if ( s.IsKey( "ADDITEM" ) || s.IsKey( "ADD" ))
                                        {
                                                ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArg
Raw() );
                                                if ( id <= 1)   // pCharDef == NULL ||
                                                {
                                                        DEBUG_ERR(( "Cant resolve %s=%s" DEBUG_CR, (LPCTSTR) s.GetKey(), (LPCTST
R) s.GetArgRaw()));
                                                        continue;
                                                }
                                        }
    
                                        if ( s.IsKey( "ADDNPC" ))
                                        {
                                                CREID_TYPE id = (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CHARDEF, s.GetArgRa
w() );
                                                if ( id < 1)    // pCharDef == NULL ||
                                                {
                                                        DEBUG_ERR(( "Cant resolve %s=%s" DEBUG_CR, (LPCTSTR) s.GetKey(), (LPCTST
R) s.GetArgRaw()));
                                                        continue;
                                                }
                                        }
    
                                        if ( s.IsKey( "GO" ))
                                        {
                                                CPointMap pt = GetRegionPoint( s.GetArgRaw());
                                                if ( ! pt.IsValidPoint())
                                                {
                                                        DEBUG_ERR(( "Cant resolve %s=%s" DEBUG_CR, (LPCTSTR) s.GetKey(), (LPCTST
R) s.GetArgRaw()));
                                                        continue;
                                                }
                                        }
    
                                        // we should try to check other stuff as well.
                                }
                        }
                        break;
                case RES_EMAILMSG:
                        break;
                case RES_SPEECH:
                        break;
                case RES_NAMES:
                        break;
                case RES_EVENTS:
                        // trigger all the events on the cchar ?
                        {
                                // Excercise all the possible triggers.
                                ASSERT(CTRIG_QTY<=64);
                                for ( int j=0; j<CTRIG_QTY; j++ )
                                {
                                        if ( ! pLink->HasTrigger( j ))
                                                continue;
    
                                        CResourceLock s;
                                        if ( ! pLink->ResourceLock( s ))
                                                continue;
    
                                        TRIGRET_TYPE iRet = pCharProvis->OnTriggerScript( s, CChar::sm_szTrigName, &g_Serv, NULL
 );
                                }
                        }
                        break;
                case RES_PLOTITEM:
                        break;
                case RES_REGIONTYPE:
                        {
                                // Excercise all the possible triggers.
    
                                CPointMap pt( 1,1,1 );
                                CRegionWorld * pRegion = dynamic_cast <CRegionWorld *>( pt.GetRegion(REGION_TYPE_AREA));
                                ASSERT(pRegion);
    
                                for ( int j=0; j<RTRIG_QTY; j++ )
                                {
                                        if ( ! pLink->HasTrigger( j ))
                                                continue;
    
                                        CResourceLock s;
                                        if ( ! pLink->ResourceLock( s ))
                                                continue;
    
                                        TRIGRET_TYPE iRet = pRegion->OnTriggerScript( s, CRegionWorld::sm_szTrigName, pCharProvi
s, NULL );
                                }
                        }
                        break;
                case RES_SCROLL:
                        break;
                case RES_TIP:
                        break;
                case RES_CRYSTALBALL:
                        break;
                case RES_BLOCKEMAIL:
                        break;
                case RES_BOOK:
                        break;
                case RES_NEWBIE:
                        pCharProvis->ReadScript( s );
                        pItemCont->DeleteAll();
                        break;
                case RES_DIALOG:
                        // Excercise all the triggers.
                        break;
                case RES_TEMPLATE:
                        {
                                CItem * pItemNew = CItem::CreateTemplate( (ITEMID_TYPE) pLink->GetResourceID().GetResIndex(), pI
temCont );
                                pItemCont->DeleteAll();
                        }
                        break;
                case RES_SKILLMENU:     // - try making all the items.
                        {
                        int iOnHeaders = 0;
                        int iTestCount = 0;
                        for ( int k=0; s.ReadKeyParse(); k++ )
                        {
                                if ( ! k )
                                        continue;
    
                                int j = FindTableHead( s.GetKey(), sm_szSkillMenuKeys, COUNTOF(sm_szSkillMenuKeys));
                                if ( j < 0 )
                                {
                                        DEBUG_ERR(( "Odd key '%s'" DEBUG_CR, (LPCTSTR) s.GetKey() ));
                                        continue;
                                }
    
                                if ( j <= 1 )
                                        iTestCount = 0;
    
                                if ( s.IsKey( "TEST" ))
                                {
                                        if ( iTestCount )
                                        {
                                                DEBUG_WARN(( "Multiple TEST=, combine these on a line!" DEBUG_CR ));
                                        }
                                        iTestCount++;
                                        continue;
                                }
                                if ( s.IsKeyHead( "ON", 2 ))
                                {
                                        iOnHeaders++;
                                        CMenuItem menuitem;
                                        if ( ! menuitem.ParseLine( s.GetArgRaw(), pCharProvis, pCharProvis ))
                                        {
                                        }
                                        continue;
                                }
                                if ( s.IsKey( "MAKEITEM" ))
                                {
                                        ITEMID_TYPE id = (ITEMID_TYPE) g_Cfg.ResourceGetIndexType( RES_ITEMDEF, s.GetArgRaw() );
                                        if ( id <= 1)   // pCharDef == NULL ||
                                        {
                                                DEBUG_ERR(( "Cant resolve MAKEITEM '%s'" DEBUG_CR, (LPCTSTR) s.GetArgRaw()));
                                                continue;
                                        }
    
                                        CItemBase * pItemDef = CItemBase::FindItemBase( (ITEMID_TYPE) id );
                                        if ( pItemDef == NULL )
                                        {
                                                DEBUG_ERR(( "Cant MAKEITEM '%s'" DEBUG_CR, (LPCTSTR) s.GetArgRaw()));
                                                continue;
                                        }
    
                                        if ( pItemDef->GetID() != id )
                                        {
                                                DEBUG_ERR(( "MAKEITEM '%s' is a DUPEITEM!" DEBUG_CR, (LPCTSTR) s.GetArgRaw()));
                                                continue;
                                        }
    
                                        continue;
                                }
                        }
                        }
                        break;
                default:
                        ASSERT(0);
                        break;
                }
        }
    
        //RES_FUNCTION:
    
        //RES_HELP:
    
        return( true );
    }