    // CCharSpell.cpp
    //
    
    #include "graysvr.h"        // predef header.
    
    void CChar::Spell_Dispel( int iLevel )
    {
        // ARGS: iLevel = 0-100 level of dispel caster.
        // remove all the spells. NOT if caused by objects worn !!!
        // ATTR_MAGIC && ! ATTR_MOVE_NEVER
    
        CItem* pItemNext;
        CItem* pItem=GetContentHead();
        for ( ; pItem!=NULL; pItem=pItemNext )
        {
                pItemNext = pItem->GetNext();
                if ( iLevel <= 100 && pItem->IsAttr(ATTR_MOVE_NEVER))   // we don't lose this.
                        continue;
                if ( pItem->GetEquipLayer() == LAYER_LIGHT ||
                        ( pItem->GetEquipLayer() >= LAYER_SPELL_STATS &&
                        pItem->GetEquipLayer() <= LAYER_SPELL_Summon ))
                {
                        pItem->Delete();
                }
        }
        Update();
    }
    
    bool CChar::Spell_Teleport( CPointBase ptNew,
        bool fTakePets, bool fCheckAntiMagic,
        ITEMID_TYPE iEffect, SOUND_TYPE iSound )
    {
        // Teleport you to this place.
        // This is sometimes not really a spell at all.
        // ex. ships plank.
        // RETURN: true = it worked.
    
        if ( ! ptNew.IsCharValid())
                return false;
    
        Reveal();
    
        if ( IsPriv(PRIV_JAILED))
        {
                // Must be /PARDONed
    static LPCTSTR const sm_szPunishMsg =
    {
        "You feel the world is punishing you.",
        "You feel you should be ashamed of your actions.",
    };
                SysMessage( sm_szPunishMsg );
                ptNew = g_Cfg.GetRegionPoint( "jail" );
        }
    
        // Is it a valid teleport location that allows this ?
    
        if ( IsPriv( PRIV_GM ))
        {
                fCheckAntiMagic = false;
                if ( iEffect && ! IsStatFlag( STATF_Incognito ) && ! IsPriv( PRIV_PRIV_NOSHOW ))
                {
                        iEffect = g_Cfg.m_iSpell_Teleport_Effect_Staff; // drama
                        iSound = g_Cfg.m_iSpell_Teleport_Sound_Staff;
                }
        }
        else if ( fCheckAntiMagic )
        {
                CRegionBase * pArea = CheckValidMove( ptNew );
                if ( pArea == NULL )
                {
                        SysMessage( "You can't teleport to that spot." );
                        return false;
                }
                if ( pArea->IsFlag( REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_TELEPORT ))
                {
                        SysMessage( "An anti-magic field blocks you" );
                        return false;
                }
        }
    
        if ( IsClient())
        {
                if ( IsStatFlag( STATF_Insubstantial ))
                        iEffect = ITEMID_NOTHING;
                GetClient()->addPause();
        }
    
        if ( GetTopPoint().IsValidPoint())      // Guards might have justbeen created.
        {
        if ( fTakePets )
        {
                // Look for any creatures that might be following me near by.
                CWorldSearch Area( GetTopPoint(), UO_MAP_VIEW_SIGHT );
                while (true)
                {
                        CChar * pChar = Area.GetChar();
                        if ( pChar == NULL )
                                break;
                        if ( pChar == this )
                                continue;
    
                        // Hostiles ???
    
                        // My pets ?
                        if ( pChar->Skill_GetActive() == NPCACT_FOLLOW_TARG &&
                                pChar->m_Act_Targ == GetUID())
                        {
                                CPointMap masterp = GetTopPoint();      // it will modify the val.
                                if ( ! pChar->CanMoveWalkTo( masterp, false ))
                                        continue;
                                pChar->Spell_Teleport( ptNew, fTakePets, fCheckAntiMagic, iEffect, iSound );
                        }
                }
        }
    
        if ( iEffect )  // departing side effect.
        {
                CItem * pItem = CItem::CreateBase( iEffect );
                ASSERT(pItem);
                pItem->MoveToDecay( GetTopPoint(), 2 * TICK_PER_SEC );
        }
        }
    
        CPointMap ptOld = GetTopPoint();
    
        // NOTE: NEVER move to plane MAP_PLANE_ALL this way !!
        if ( ptNew.m_mapplane == MAP_PLANE_ALL )
        {
                ptNew.m_mapplane = ptOld.m_mapplane;
        }
    
        MoveToChar( ptNew );
        UpdateMove( ptOld, NULL, true );
    
        if ( iEffect )
        {
                Sound( iSound );        // 0x01fe
                Effect( EFFECT_OBJ, iEffect, this, 9, 20 );
        }
        return( true );
    }
    
    CChar * CChar::Spell_Summon( CREID_TYPE id, CPointMap pntTarg, bool fSpellSummon )
    {
        if ( id == CREID_INVALID )
                return( NULL );
    
        DEBUG_CHECK( pntTarg.IsValidPoint());
        CChar * pChar;
        if ( ! fSpellSummon )
        {
                // GM creates a char this way. "ADDNPC"
                // More specific npc type from script ?
                pChar = CreateNPC( id );
                if ( pChar == NULL )
                        return( NULL );
                m_Act_Targ = pChar->GetUID();   // for last target stuff.
        }
        else
        {
                // These type summons make me it's master. (for a limited time)
                pChar = CChar::CreateBasic( id );
                if ( pChar == NULL )
                        return( NULL );
                // Time based on magery. Add the flag first so it does not get loot.
                // conjured creates have no loot. (Mark this early)
                CItem * pSpell = pChar->Spell_Effect_Create( SPELL_Summon,
                        LAYER_SPELL_Summon, Skill_GetAdjusted( SKILL_MAGERY ), 0, this );
                ASSERT(pSpell);
                pChar->NPC_LoadScript(false);
    
                // Can't own a beserk type creature.
                ASSERT(pChar->m_pNPC);
                pChar->NPC_PetSetOwner( this );
        }
    
        pChar->MoveToChar( pntTarg );
        pChar->Update();
        pChar->UpdateAnimate( ANIM_CAST_DIR );
        pChar->SoundChar( CRESND_GETHIT );
        return( pChar );
    }
    
    bool CChar::Spell_Recall( CItem * pRune, bool fGate )
    {
        if ( pRune == NULL ||
                ( ! pRune->IsType(IT_RUNE) && ! pRune->IsType(IT_TELEPAD)))
        {
                SysMessage( "That item is not a recall rune." );
                return( false );
        }
        if ( ! pRune->m_itRune.m_pntMark.IsValidPoint())
        {
                SysMessage( "The recall rune is blank." );
                return( false );
        }
        if ( pRune->IsType(IT_RUNE) && pRune->m_itRune.m_Strength <= 0 )
        {
                SysMessage( "The recall rune's magic has faded" );
                if ( ! IsPriv(PRIV_GM))
                {
                        return( false );
                }
        }
    
        if ( fGate )
        {
                // Can't even open the gate ?
                CRegionBase * pArea = pRune->m_itRune.m_pntMark.GetRegion( REGION_TYPE_AREA | REGION_TYPE_MULTI | REGION_TYPE_RO
OM );
                if ( pArea == NULL )
                {
                        return( false );
                }
                if ( pArea->IsFlag( REGION_ANTIMAGIC_ALL | REGION_ANTIMAGIC_GATE | REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC
_RECALL_OUT | REGION_ANTIMAGIC_RECALL_IN ))
                {
                        // Antimagic
                        SysMessage( "Antimagic blocks the gate" );
                        if ( ! IsPriv(PRIV_GM))
                        {
                                return( false );
                        }
                }
    
                const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(SPELL_Gate_Travel);
                ASSERT(pSpellDef);
    
                // Color gates to unguarded regions.
                CItem * pGate = CItem::CreateBase( pSpellDef->m_idEffect );
                ASSERT(pGate);
                pGate->SetType( IT_TELEPAD );
                pGate->SetAttr(ATTR_MAGIC|ATTR_MOVE_NEVER|ATTR_CAN_DECAY);      // why is this movable ?
                pGate->m_itTelepad.m_pntMark = pRune->m_itRune.m_pntMark;
                // pGate->m_light_pattern = LIGHT_LARGE;
                pGate->SetHue(( pArea != NULL && pArea->IsGuarded()) ? HUE_DEFAULT : HUE_RED );
                int iDuration = pSpellDef->m_Duration.GetLinear( 0 );
                pGate->MoveToDecay( GetTopPoint(), iDuration );
                // The open animation.
                pGate->Effect( EFFECT_OBJ, ITEMID_MOONGATE_FX_BLUE, pGate, 2 );
    
                // Far end gate.
                pGate = CItem::CreateDupeItem( pGate );
                ASSERT(pGate);
                pGate->m_itTelepad.m_pntMark = GetTopPoint();
                pGate->SetHue(( m_pArea && m_pArea->IsGuarded()) ? HUE_DEFAULT : HUE_RED );
                pGate->MoveToDecay( pRune->m_itRune.m_pntMark, iDuration );
                pGate->Sound( pSpellDef->m_sound );
                // The open animation.
                pGate->Effect( EFFECT_OBJ, ITEMID_MOONGATE_FX_BLUE, pGate, 2 );
        }
        else
        {
                if ( ! Spell_Teleport( pRune->m_itRune.m_pntMark, false, true, ITEMID_NOTHING ))
                        return( false );
        }
    
        // Wear out the rune.
        if ( pRune->IsType(IT_RUNE))
        {
                if ( ! IsPriv(PRIV_GM))
                {
                        pRune->m_itRune.m_Strength --;
                }
                if ( pRune->m_itRune.m_Strength < 10 )
                {
                        SysMessage( "The recall rune is starting to fade." );
                }
                if ( ! pRune->m_itRune.m_Strength )
                {
                        SysMessage( "The recall rune fades completely." );
                }
        }
    
        return( true );
    }
    
    bool CChar::Spell_Resurrection( int iSkillLossPercent, CItemCorpse * pCorpse )
    {
        // SPELL_Resurrection
        // ARGS: iSkillLossPercent < 0 for a shrine or GM.
    
        if ( ! IsStatFlag( STATF_DEAD ))
                return false;
    
        if ( iSkillLossPercent >= 0 &&
                ! IsPriv(PRIV_GM) &&
                m_pArea &&
                m_pArea->IsFlag( REGION_ANTIMAGIC_ALL | REGION_ANTIMAGIC_RECALL_IN | REGION_ANTIMAGIC_TELEPORT ))
        {
                // Could be a house break in.
                // Check to see if it is.
                if ( m_pArea->GetResourceID().IsItem())
                {
                        SysMessage( "Anti-Magic blocks the Resurrection!" );
                        return false;
                }
        }
    
        SetID( m_prev_id );
        StatFlag_Clear( STATF_DEAD | STATF_Insubstantial );
        SetHue( m_prev_Hue );
        m_StatHealth = IMULDIV( Stat_Get(STAT_STR), g_Cfg.m_iHitpointPercentOnRez, 100 );
    
        if ( m_pPlayer )
        {
                CItem * pRobe = ContentFind( RESOURCE_ID(RES_ITEMDEF,ITEMID_DEATHSHROUD));
                if ( pRobe != NULL )
                {
                        pRobe->RemoveFromView();        // For some reason this does not update without remove first.
                        pRobe->SetID( ITEMID_ROBE );
                        pRobe->SetName( "Resurrection Robe" );
                        pRobe->Update();
                }
    
                if ( pCorpse == NULL )
                {
                        pCorpse = FindMyCorpse();
                }
                if ( pCorpse != NULL )
                {
                        if ( RaiseCorpse( pCorpse ))
                        {
                                SysMessage( "You spirit rejoins your body" );
                                if ( pRobe != NULL )
                                {
                                        pRobe->Delete();
                                        pRobe = NULL;
                                }
                        }
                }
        }
    
        if ( iSkillLossPercent > 0 )
        {
                // Remove some skills / stats as a percent.
                int i=0;
                for ( ; i<SKILL_QTY; i++ )
                {
                        int iVal = Skill_GetBase( (SKILL_TYPE) i );
                        if ( iVal <= 250 )
                                continue;
                        Skill_SetBase( (SKILL_TYPE) i, iVal - IMULDIV( iVal, Calc_GetRandVal( iSkillLossPercent ), 100 ));
                }
                for ( i=STAT_STR; i<STAT_BASE_QTY; i++ )
                {
                        int iVal = Stat_Get( (STAT_TYPE) i );
                        if ( iVal <= 25 )
                                continue;
                        Stat_Set( (STAT_TYPE) i, iVal - IMULDIV( iVal, Calc_GetRandVal( iSkillLossPercent ), 100 ));
                }
        }
    
        Effect( EFFECT_OBJ, ITEMID_FX_HEAL_EFFECT, this, 9, 14 );
        Update();
        return( true );
    }
    
    void CChar::Spell_Effect_Remove( CItem * pSpell )
    {
        // we are removing the spell effect.
        ASSERT(pSpell);
    
        if ( ! pSpell->IsTypeSpellable())       // can this item have a spell effect ?
                return;
        if ( ! pSpell->m_itSpell.m_spell )
                return;
        if ( pSpell->IsType(IT_WAND))   // equipped wands do not confer effect.
                return;
    
        // m_itWeapon, m_itArmor, m_itSpell
    
        SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX( pSpell->m_itSpell.m_spell );
    
        if ( pSpell->IsAttr( ATTR_CURSED | ATTR_CURSED2 ))
        {
                // The spell item was cursed in some way.
                spell = SPELL_Curse;
        }
    
        int iStatEffect = g_Cfg.GetSpellEffect( spell, pSpell->m_itSpell.m_spelllevel );
    
        switch ( spell )
        {
        case SPELL_Clumsy:
                Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) + iStatEffect );
                break;
        case SPELL_Particle_Form:       // 112 // turns you into an immobile, but untargetable particle system for a while.
        case SPELL_Stone:
                StatFlag_Clear( STATF_Stone );
                break;
        case SPELL_Hallucination:
                StatFlag_Clear( STATF_Hallucinating );
                if ( IsClient())
                {
                        m_pClient->addReSync();
                }
        case SPELL_Feeblemind:
                Stat_Set( STAT_INT, Stat_Get(STAT_INT) + iStatEffect );
                break;
        case SPELL_Weaken:
                Stat_Set( STAT_STR, Stat_Get(STAT_STR) + iStatEffect );
                break;
        case SPELL_Agility:
                Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - iStatEffect );
                break;
        case SPELL_Cunning:
                Stat_Set( STAT_INT, Stat_Get(STAT_INT) - iStatEffect );
                break;
        case SPELL_Strength:
                Stat_Set( STAT_STR, Stat_Get(STAT_STR) - iStatEffect );
                break;
        case SPELL_Bless:
                {
                        for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
                        {
                                Stat_Set( (STAT_TYPE) i, m_Stat - iStatEffect );
                        }
                }
                break;
    
        case SPELL_Ale:         // 90 = drunkeness ?
        case SPELL_Wine:        // 91 = mild drunkeness ?
        case SPELL_Liquor:      // 92 = extreme drunkeness ?
        case SPELL_Curse:
        case SPELL_Mass_Curse:
                {
                        for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
                        {
                                Stat_Set( (STAT_TYPE) i, m_Stat + iStatEffect );
                        }
                }
                break;
    
        case SPELL_Reactive_Armor:
                StatFlag_Clear( STATF_Reactive );
                return;
        case SPELL_Night_Sight:
                StatFlag_Clear( STATF_NightSight );
                if ( IsClient())
                {
                        m_pClient->addLight();
                }
                return;
        case SPELL_Magic_Reflect:
                StatFlag_Clear( STATF_Reflection );
                return;
        case SPELL_Poison:
        case SPELL_Poison_Field:
                StatFlag_Clear( STATF_Poisoned );
                UpdateMode();
                break;
        case SPELL_Steelskin:           // 114 // turns your skin into steel, giving a boost to your AR.
        case SPELL_Stoneskin:           // 115 // turns your skin into stone, giving a boost to your AR.
        case SPELL_Protection:
        case SPELL_Arch_Prot:
                m_defense = CalcArmorDefense();
                break;
        case SPELL_Incognito:
                StatFlag_Clear( STATF_Incognito );
                SetName( pSpell->GetName());    // restore your name
                if ( ! IsStatFlag( STATF_Polymorph ))
                {
                        SetHue( m_prev_Hue );
                }
                pSpell->SetName( "" );  // lcear the name from the item (might be a worn item)
                break;
        case SPELL_Invis:
                Reveal(STATF_Invisible);
                return;
        case SPELL_Paralyze:
        case SPELL_Paralyze_Field:
                StatFlag_Clear( STATF_Freeze );
                return;
        case SPELL_Polymorph:
                {
                        DEBUG_CHECK( IsStatFlag( STATF_Polymorph ));
                        //  m_prev_id != GetID()
                        // poly back to orig form.
                        SetID( m_prev_id );
                        // set back to original stats as well.
                        Stat_Set( STAT_STR, Stat_Get(STAT_STR) - pSpell->m_itSpell.m_PolyStr );
                        Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - pSpell->m_itSpell.m_PolyDex );
                        m_StatHealth = min( m_StatHealth, Stat_Get(STAT_STR));
                        m_StatStam = min( m_StatStam, Stat_Get(STAT_DEX));
                        Update();
                        StatFlag_Clear( STATF_Polymorph );
                }
                return;
        case SPELL_Summon:
                // Delete the creature completely.
                // ?? Drop anything it might have had ?
                // m_StatHealth = 0;
                if ( ! g_Serv.IsLoading())
                {
                        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(SPELL_Teleport);
                        ASSERT(pSpellDef);
    
                        CItem * pEffect = CItem::CreateBase( ITEMID_FX_TELE_VANISH );
                        ASSERT(pEffect);
                        pEffect->SetAttr(ATTR_MAGIC|ATTR_MOVE_NEVER|ATTR_CAN_DECAY); // why is this movable ?
                        pEffect->MoveToDecay( GetTopPoint(), 2*TICK_PER_SEC );
                        pEffect->Sound( pSpellDef->m_sound  );
                }
                DEBUG_CHECK( ! m_pPlayer );
                if ( m_pPlayer )        // summoned players ? thats odd.
                        return;
                Delete();
                return;
    
        case SPELL_Chameleon:           // 106 // makes your skin match the colors of whatever is behind you.
        case SPELL_BeastForm:           // 107 // polymorphs you into an animal for a while.
        case SPELL_Monster_Form:        // 108 // polymorphs you into a monster for a while.
                break;
    
        case SPELL_Trance:                      // 111 // temporarily increases your meditation skill.
                Skill_SetBase( SKILL_MEDITATION, Skill_GetBase( SKILL_MEDITATION ) - iStatEffect );
                break;
    
        case SPELL_Shield:                      // 113 // erects a temporary force field around you. Nobody approaching will be 
able to get within 1 tile of you, though you can move close to them if you wish.
                break;
        }
        UpdateStatsFlag();
    }
    
    void CChar::Spell_Effect_Add( CItem * pSpell )
    {
        // Attach the spell effect for a duration.
        // Add effects which are saved in the save file here.
        // Not in LayerAdd
        // NOTE: ATTR_MAGIC without ATTR_MOVE_NEVER is dispellable !
    
        ASSERT(pSpell);
        if ( ! pSpell->IsTypeSpellable())
                return;
        if ( pSpell->IsType(IT_WAND))   // equipped wands do not confer effect.
                return;
    
        SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX(pSpell->m_itSpell.m_spell);
    
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
        if ( pSpell->IsAttr( ATTR_CURSED | ATTR_CURSED2 ))
        {
                // The spell item was cursed in some way.
                spell = SPELL_Curse;
                if ( ! pSpell->IsAttr( ATTR_MAGIC ) || pSpellDef == NULL )
                {
                        pSpell->m_itSpell.m_spell = SPELL_Curse;
                        pSpell->m_itSpell.m_spelllevel = 100 + Calc_GetRandVal(300);
                        pSpell->SetAttr(ATTR_MAGIC);
                }
                pSpellDef = g_Cfg.GetSpellDef(spell);
                pSpell->SetAttr(ATTR_IDENTIFIED);
                SysMessage( "Cursed Magic!" );
        }
    
        if ( pSpellDef == NULL )
                return;
        if ( ! spell )
                return;
    
        int iStatEffect = g_Cfg.GetSpellEffect( spell, pSpell->m_itSpell.m_spelllevel );
    
        switch ( spell )
        {
        case SPELL_Poison:
        case SPELL_Poison_Field:
                StatFlag_Set( STATF_Poisoned );
                UpdateMode();
                break;
        case SPELL_Reactive_Armor:
                StatFlag_Set( STATF_Reactive );
                break;
        case SPELL_Night_Sight:
                StatFlag_Set( STATF_NightSight );
                if ( IsClient())
                {
                        m_pClient->addLight();
                }
                break;
        case SPELL_Clumsy:
                // NOTE: Allow stats to go negative !
                Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) - iStatEffect );
                break;
        case SPELL_Particle_Form:       // 112 // turns you into an immobile, but untargetable particle system for a while.
        case SPELL_Stone:
                StatFlag_Set( STATF_Stone );
                break;
        case SPELL_Hallucination:
                StatFlag_Set( STATF_Hallucinating );
        case SPELL_Feeblemind:
                // NOTE: Allow stats to go negative !
                Stat_Set( STAT_INT, Stat_Get(STAT_INT) - iStatEffect );
                break;
        case SPELL_Weaken:
                // NOTE: Allow stats to go negative !
                Stat_Set( STAT_STR, Stat_Get(STAT_STR) - iStatEffect );
                break;
        case SPELL_Agility:
                Stat_Set( STAT_DEX, Stat_Get(STAT_DEX) + iStatEffect );
                break;
        case SPELL_Cunning:
                Stat_Set( STAT_INT, Stat_Get(STAT_INT) + iStatEffect );
                break;
        case SPELL_Strength:
                Stat_Set( STAT_STR, Stat_Get(STAT_STR) + iStatEffect );
                break;
        case SPELL_Bless:
                {
                        for ( int i=STAT_STR; i<STAT_BASE_QTY; i++ )
                        {
                                Stat_Set( (STAT_TYPE) i, m_Stat + iStatEffect );
                        }
                }
                break;
        case SPELL_Ale:         // 90 = drunkeness ?
        case SPELL_Wine:        // 91 = mild drunkeness ?
        case SPELL_Liquor:      // 92 = extreme drunkeness ?
        case SPELL_Curse:
        case SPELL_Mass_Curse:
                {
                        // NOTE: Allow stats to go negative !
                        int i=STAT_STR;
                        for ( i=STAT_STR; i<STAT_BASE_QTY; i++ )
                        {
                                Stat_Set( (STAT_TYPE) i, Stat_Get((STAT_TYPE)i) - iStatEffect );
                        }
                }
                break;
        case SPELL_Incognito:
                if ( ! IsStatFlag( STATF_Incognito ))
                {
                        CCharBase * pCharDef = Char_GetDef();
                        ASSERT(pCharDef);
                        StatFlag_Set( STATF_Incognito );
                        pSpell->SetName( GetName());    // Give it my name
                        SetName( pCharDef->GetTypeName());      // Give me general name for the type
                        if ( ! IsStatFlag( STATF_Polymorph ) && IsHuman())
                        {
                                SetHue((HUE_UNDERWEAR|HUE_SKIN_LOW) + Calc_GetRandVal(HUE_SKIN_HIGH-HUE_SKIN_LOW));
                        }
                }
                break;
        case SPELL_Magic_Reflect:
                StatFlag_Set( STATF_Reflection );
                break;
        case SPELL_Steelskin:           // 114 // turns your skin into steel, giving a boost to your AR.
        case SPELL_Stoneskin:           // 115 // turns your skin into stone, giving a boost to your AR.
        case SPELL_Protection:
        case SPELL_Arch_Prot:
                m_defense = CalcArmorDefense();
                break;
        case SPELL_Invis:
                StatFlag_Set( STATF_Invisible );
                // m_wHue = HUE_TRANSLUCENT;
                UpdateMove(GetTopPoint());      // Some will be seeing us for the first time !
                break;
        case SPELL_Paralyze:
        case SPELL_Paralyze_Field:
                StatFlag_Set( STATF_Freeze );
                break;
        case SPELL_Polymorph:
                DEBUG_CHECK( ! IsStatFlag( STATF_Polymorph ));  // Should have already been removed.
                StatFlag_Set( STATF_Polymorph );
                break;
        case SPELL_Summon:
                // LAYER_SPELL_Summon
                StatFlag_Set( STATF_Conjured );
                break;
    
        case SPELL_Chameleon:           // 106 // makes your skin match the colors of whatever is behind you.
        case SPELL_BeastForm:           // 107 // polymorphs you into an animal for a while.
        case SPELL_Monster_Form:        // 108 // polymorphs you into a monster for a while.
                break;
    
        case SPELL_Trance:                      // 111 // temporarily increases your meditation skill.
                Skill_SetBase( SKILL_MEDITATION, Skill_GetBase( SKILL_MEDITATION ) + iStatEffect );
                break;
    
        case SPELL_Shield:                      // 113 // erects a temporary force field around you. Nobody approaching will be 
able to get within 1 tile of you, though you can move close to them if you wish.
                break;
    
        default:
                return;
        }
        UpdateStatsFlag();
    }
    
    bool CChar::Spell_Equip_OnTick( CItem * pItem )
    {
        // Spells that have a gradual effect over time.
        // NOTE: These are not necessarily "magical". could be something physical as well.
        // RETURN: false = kill the spell.
    
        ASSERT(pItem);
    
        SPELL_TYPE spell = (SPELL_TYPE) RES_GET_INDEX(pItem->m_itSpell.m_spell);
    
        int iCharges = pItem->m_itSpell.m_spellcharges;
        int iLevel = pItem->m_itSpell.m_spelllevel;
    
        static LPCTSTR const sm_Poison_Message =
        {
                "sickly",
                "very ill",
                "extremely sick",
                "deathly sick",
        };
        static const int sm_iPoisonMax = { 2, 4, 6, 8 };
    
        switch ( spell )
        {
        case SPELL_Ale:         // 90 = drunkeness ?
        case SPELL_Wine:        // 91 = mild drunkeness ?
        case SPELL_Liquor:      // 92 = extreme drunkeness ?
                // Degrades over time.
                // LAYER_FLAG_Drunk
    
                if ( iCharges <= 0 || iLevel <= 0 )
                {
                        return( false );
                }
                if ( iLevel > 50 )
                {
                        Speak( "*Hic*" );
                        if ( iLevel > 200 && Calc_GetRandVal(2))
                        {
                                UpdateAnimate( ANIM_BOW );
                        }
                }
                if ( Calc_GetRandVal(2))
                {
                        Spell_Effect_Remove( pItem );
                        pItem->m_itSpell.m_spelllevel-=10;      // weaken the effect.
                        Spell_Effect_Add( pItem );
                }
    
                // We will have this effect again.
                pItem->SetTimeout( Calc_GetRandVal(10)*TICK_PER_SEC );
                break;
    
        case SPELL_Regenerate:
                if ( iCharges <= 0 || iLevel <= 0 )
                {
                        return( false );
                }
    
                // Gain HP.
                UpdateStatVal( STAT_STR, g_Cfg.GetSpellEffect( spell, iLevel ));
                pItem->SetTimeout( 2*TICK_PER_SEC );
                break;
    
        case SPELL_Hallucination:
    
                if ( iCharges <= 0 || iLevel <= 0 )
                {
                        return( false );
                }
                if ( IsClient())
                {
                        static const SOUND_TYPE sm_sounds = { 0x243, 0x244, 0x245 };
                        m_pClient->addSound( sm_sounds );
                        m_pClient->addReSync();
                }
                // save the effect.
                pItem->SetTimeout( (15+Calc_GetRandVal(15))*TICK_PER_SEC );
                break;
    
        case SPELL_Poison:
                // Both potions and poison spells use this.
                // m_itSpell.m_spelllevel = strength of the poison ! 0-1000
    
                if ( iCharges <= 0 || iLevel < 50 )
                        return( false );
    
                // The poison in your body is having an effect.
    
                if ( iLevel < 200 )     // Lesser
                        iLevel = 0;
                else if ( iLevel < 400 ) // Normal
                        iLevel = 1;
                else if ( iLevel < 800 ) // Greater
                        iLevel = 2;
                else    // Deadly.
                        iLevel = 3;
    
                {
                CGString sMsg;
                sMsg.Format( "looks %s", (LPCTSTR) sm_Poison_Message );
                Emote( sMsg, GetClient() );
                SysMessagef( "You feel %s", (LPCTSTR) sm_Poison_Message );
    
                int iDmg = IMULDIV( Stat_Get(STAT_STR), iLevel * 2, 100 );
                OnTakeDamage( max( sm_iPoisonMax, iDmg ), pItem->m_uidLink.CharFind(), DAMAGE_POISON | DAMAGE_GENERAL );
                }
    
                pItem->m_itSpell.m_spelllevel -= 50;    // gets weaker too.
    
                // g_Cfg.GetSpellEffect( SPELL_Poison,
    
                // We will have this effect again.
                pItem->SetTimeout((5+Calc_GetRandVal(4))*TICK_PER_SEC);
                break;
    
        default:
                return( false );
        }
    
        // Total number of ticks to come back here.
        if ( --pItem->m_itSpell.m_spellcharges )
                return( true );
        return( false );
    }
    
    CItem * CChar::Spell_Effect_Create( SPELL_TYPE spell, LAYER_TYPE layer, int iSkillLevel, int iDuration, CObjBase * pSrc )
    {
        // Attach an effect to the Character.
        //
        // ARGS:
        // spell = SPELL_Invis, etc.
        // layer == LAYER_FLAG_Potion, etc.
        // iSkillLevel = 0-1000 = skill level or other spell specific value.
        // iDuration = TICK_PER_SEC
        //
        // NOTE: 
        //   ATTR_MAGIC without ATTR_MOVE_NEVER is dispellable !
    
        if ( g_Log.IsLogged( LOGL_TRACE ))
        {
                DEBUG_CHECK( iSkillLevel <= 1000 );
        }
    
        CItem * pSpell;
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
    
        pSpell = CItem::CreateBase( pSpellDef ? ( pSpellDef->m_idSpell ) : ITEMID_RHAND_POINT_NW );
        ASSERT(pSpell);
    
        pSpell->SetAttr(ATTR_NEWBIE);
        if ( pSpellDef )
        {
                pSpell->SetAttr(ATTR_MAGIC);    // can it be dispelled ?
        }
        pSpell->SetType(IT_SPELL);
        pSpell->m_itSpell.m_spell = spell;
        pSpell->m_itSpell.m_spelllevel = iSkillLevel;   // 0 - 1000
        pSpell->m_itSpell.m_spellcharges = 1;
    
        if ( iDuration <= 0 )   // use default script duration.
        {
                ASSERT(pSpellDef);
                iDuration = pSpellDef->m_Duration.GetLinear( iSkillLevel );
                if ( iDuration <= 0 )
                        iDuration = 1;
        }
        pSpell->SetDecayTime( iDuration );
    
        if ( pSrc )
        {
                pSpell->m_uidLink = pSrc->GetUID();
        }
    
        LayerAdd( pSpell, layer );      // Remove any competing effect first.
        Spell_Effect_Add( pSpell );
        return( pSpell );
    }
    
    void CChar::Spell_Bolt( CObjBase * pObjTarg, ITEMID_TYPE idBolt, int iSkillLevel )
    {
        // I am casting a bolt spell.
        // ARGS:
        // iSkillLevel = 0-1000
        //
    
        if ( pObjTarg == NULL )
                return;
        pObjTarg->Effect( EFFECT_BOLT, idBolt, this, 5, 1, true );
        // Take damage !
        pObjTarg->OnSpellEffect( m_atMagery.m_Spell, this, iSkillLevel, NULL );
    }
    
    void CChar::Spell_Area( CPointMap pntTarg, int iDist, int iSkillLevel )
    {
        // Effects all creatures in the area. (but not us)
        // ARGS:
        // iSkillLevel = 0-1000
        //
    
        SPELL_TYPE spelltype = m_atMagery.m_Spell;
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spelltype);
        DEBUG_CHECK(pSpellDef);
        if ( pSpellDef == NULL )
                return;
    
        CWorldSearch AreaChar( pntTarg, iDist );
        while (true)
        {
                CChar * pChar = AreaChar.GetChar();
                if ( pChar == NULL )
                        break;
                if ( pChar == this ) // not harm the caster.
                {
                        if ( pSpellDef->IsSpellType( SPELLFLAG_HARM ))
                                continue;
                }
                pChar->OnSpellEffect( spelltype, this, iSkillLevel, NULL );
        }
        CWorldSearch AreaItem( pntTarg, iDist );
        while (true)
        {
                CItem * pItem = AreaItem.GetItem();
                if ( pItem == NULL )
                        break;
                pItem->OnSpellEffect( spelltype, this, iSkillLevel, NULL );
        }
    }
    
    void CChar::Spell_Field( CPointMap pntTarg, ITEMID_TYPE idEW, ITEMID_TYPE idNS, int iSkillLevel )
    {
        // Cast the field spell to here.
        // ARGS:
        // m_atMagery.m_Spell = the spell
        // iSkillLevel = 0-1000
        //
    
        if ( m_pArea && m_pArea->IsGuarded())
        {
                Noto_Criminal();
        }
    
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(m_atMagery.m_Spell);
        ASSERT(pSpellDef);
    
        // get the dir of the field.
        int dx = abs( pntTarg.m_x - GetTopPoint().m_x );
        int dy = abs( pntTarg.m_y - GetTopPoint().m_y );
        ITEMID_TYPE id = ( dx > dy ) ? idNS : idEW;
    
        for ( int i=-3; i<=3; i++ )
        {
                bool fGoodLoc = true;
    
                // Where is this ?
                CPointMap ptg = pntTarg;
                if ( dx > dy )
                        ptg.m_y += i;
                else
                        ptg.m_x += i;
    
                // Check for direct cast on a creature.
                CWorldSearch AreaChar( ptg );
                while (true)
                {
                        CChar * pChar = AreaChar.GetChar();
                        if ( pChar == NULL )
                                break;
                        if ( ! pChar->OnAttackedBy( this, 1, false ))   // they should know they where attacked.
                                return;
                        if ( idEW == ITEMID_STONE_WALL )
                        {
                                fGoodLoc = false;
                                break;
                        }
                }
    
                // Check for direct cast on an item.
                CWorldSearch AreaItem( ptg );
                while (true)
                {
                        CItem * pItem = AreaItem.GetItem();
                        if ( pItem == NULL )
                                break;
                        pItem->OnSpellEffect( m_atMagery.m_Spell, this, iSkillLevel, NULL );
                }
    
                if ( fGoodLoc)
                {
                CItem * pSpell = CItem::CreateScript( id );
                ASSERT(pSpell);
                pSpell->SetType(IT_SPELL);
                pSpell->SetAttr(ATTR_MAGIC);
                pSpell->m_itSpell.m_spell = m_atMagery.m_Spell;
                pSpell->m_itSpell.m_spelllevel = iSkillLevel;
                pSpell->m_itSpell.m_spellcharges = 1;
                pSpell->m_uidLink = GetUID();   // Link it back to you
    
                // Add some random element.
                int iDuration = pSpellDef->m_Duration.GetLinear(iSkillLevel);
    
                pSpell->MoveToDecay( ptg, iDuration + Calc_GetRandVal( iDuration/2 ));
                }
        }
    }
    
    bool CChar::Spell_CanCast( SPELL_TYPE spell, bool fTest, CObjBase * pSrc, bool fFailMsg )
    {
        // ARGS:
        //  pSrc = possible scroll or wand source.
        // Do we have enough mana to start ?
        if ( spell <= SPELL_NONE ||
                pSrc == NULL )
                return( false );
    
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
        if ( pSpellDef == NULL )
                return( false );
        if ( pSpellDef->IsSpellType( SPELLFLAG_DISABLED ))
                return( false );
    
        // if ( ! fTest || m_pNPC )
        {
                if ( ! IsPriv(PRIV_GM) && m_pArea && m_pArea->CheckAntiMagic( spell ))
                {
                        if ( fFailMsg )
                                SysMessage( "An anti-magic field disturbs the spells." );
                        m_Act_Difficulty = -1;  // Give very little credit for failure !
                        return( false );
                }
        }
    
        int wManaUse = pSpellDef->m_wManaUse;
    
        // The magic item must be on your person to use.
        if ( pSrc != this )
        {
                CItem * pItem = dynamic_cast <CItem*> (pSrc);
                if ( pItem == NULL )
                {
                        DEBUG_CHECK( 0 );
                        return( false );        // where did it go ?
                }
                if ( ! pItem->IsAttr( ATTR_MAGIC ))
                {
                        if ( fFailMsg )
                                SysMessage( "This item lacks any enchantment." );
                        return( false );
                }
                CObjBaseTemplate * pObjTop = pSrc->GetTopLevelObj();
                if ( pObjTop != this )
                {
                        if ( fFailMsg )
                                SysMessage( "Magic items must be on your person to activate." );
                        return( false );
                }
                if ( pItem->IsType(IT_WAND))
                {
                        // Must have charges.
                        if ( pItem->m_itWeapon.m_spellcharges <= 0 )
                        {
                                // ??? May explode !!
                                if ( fFailMsg )
                                        SysMessage( "It seems to be out of charges" );
                                return false;
                        }
                        wManaUse = 0;   // magic items need no mana.
                        if ( ! fTest && pItem->m_itWeapon.m_spellcharges != 255 )
                        {
                                pItem->m_itWeapon.m_spellcharges --;
                        }
                }
                else    // Scroll
                {
                        wManaUse /= 2;
                        if ( ! fTest )
                        {
                                 pItem->ConsumeAmount();
                        }
                }
        }
        else
        {
                // Raw cast from spellbook.
    
                if ( IsPriv( PRIV_GM ))
                        return( true );
    
                if ( IsStatFlag( STATF_DEAD|STATF_Sleeping ) ||
                        ! pSpellDef->m_SkillReq.IsResourceMatchAll(this))
                {
                        if ( fFailMsg )
                                SysMessage( "This is beyond your ability." );
                        return( false );
                }
    
                if ( m_pPlayer )
                {
                        // check the spellbook for it.
                        CItem * pBook = ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_SPELLBOOK), spell, 20 );
                        if ( pBook == NULL )
                        {
                                if ( fFailMsg )
                                        SysMessage( "You don't know that spell." );
                                return( false );
                        }
                }
        }
    
        if ( m_StatMana < wManaUse )
        {
                if ( fFailMsg )
                        SysMessage( "You lack sufficient mana for this spell" );
                return( false );
        }
    
        if ( ! fTest && wManaUse )
        {
                // Consume mana.
                if ( m_Act_Difficulty < 0 )     // use diff amount of mana if we fail.
                {
                        wManaUse = wManaUse/2 + Calc_GetRandVal( wManaUse/2 + wManaUse/4 );
                }
                UpdateStatVal( STAT_INT, -wManaUse );
        }
    
        if ( m_pNPC ||  // NPC's don't need regs.
                pSrc != this )  // wands and scrolls have there own reags source.
                return( true );
    
        // Check for regs ?
        if ( g_Cfg.m_fReagentsRequired )
        {
                CItemContainer * pPack = GetPack();
                if ( pPack )
                {
                        const CResourceQtyArray * pRegs = &(pSpellDef->m_Reags);
                        int iMissing = pPack->ResourceConsumePart( pRegs, 1, 100, fTest );
                        if ( iMissing >= 0 )
                        {
                                if ( fFailMsg )
                                {
                                        CResourceDef * pReagDef = g_Cfg.ResourceGetDef( pRegs->GetAt(iMissing).GetResourceID() )
;
                                        SysMessagef( "You lack %s for this spell", pReagDef ? pReagDef->GetName() : "reagents" )
;
                                }
                                return( false );
                        }
                }
        }
        return( true );
    }
    
    bool CChar::Spell_TargCheck()
    {
        // Is the spells target or target pos valid ?
    
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(m_atMagery.m_Spell);
        if ( pSpellDef == NULL )
        {
                DEBUG_ERR(( "Bad Spell %d, uid 0%0x\n", m_atMagery.m_Spell, GetUID()));
                return( false );
        }
    
        CObjBase * pObj = m_Act_Targ.ObjFind();
        CObjBaseTemplate * pObjTop;
        if ( pObj )
        {
                pObjTop = pObj->GetTopLevelObj();
        }
    
        // NOTE: Targeting a field spell directly on a char should not be allowed ?
        if ( pSpellDef->IsSpellType(SPELLFLAG_FIELD))
        {
                if ( m_Act_Targ.IsValidUID() && m_Act_Targ.IsChar())
                {
                        SysMessage( "Can't target fields directly on a character." );
                        return false;
                }
        }
    
        // Need a target.
        if ( pSpellDef->IsSpellType( SPELLFLAG_TARG_OBJ | SPELLFLAG_TARG_CHAR ))
        {
                if ( pObj == NULL )
                {
                        SysMessage( "This spell needs a target object" );
                        return( false );
                }
                if ( ! CanSeeLOS( pObj ))
                {
                        SysMessage( "Target is not in line of sight" );
                        return( false );
                }
                if ( ! IsPriv( PRIV_GM ) && pObjTop != pObj && pObjTop->IsChar() && pObjTop != this )
                {
                        SysMessage( "Can't cast a spell on this where it is" );
                        return( false );
                }
    
    #if 0
                if ( pSpellDef->IsSpellType( SPELLFLAG_TARG_CHAR ))
                {
                        CChar * pChar = dynamic_cast <CChar*> ( pObj );
                        // Need a char target.
                        if ( pChar == NULL )
                        {
                                SysMessage( "This spell has no effect on objects" );
                                return( false );
                        }
                }
    #endif
                m_Act_p = pObjTop->GetTopPoint();
    
    facepoint:
                UpdateDir(m_Act_p);
    
                // Is my target in an anti magic feild.
                CRegionWorld * pArea = dynamic_cast <CRegionWorld *> ( m_Act_p.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
                if ( ! IsPriv(PRIV_GM) && pArea && pArea->CheckAntiMagic( m_atMagery.m_Spell ))
                {
                        SysMessage( "An anti-magic field disturbs the spells." );
                        m_Act_Difficulty = -1;  // Give very little credit for failure !
                        return( false );
                }
        }
        else if ( pSpellDef->IsSpellType( SPELLFLAG_TARG_XYZ ))
        {
                if ( pObj )
                {
                        m_Act_p = pObjTop->GetTopPoint();
                }
                if ( ! CanSeeLOS( m_Act_p, NULL, UO_MAP_VIEW_SIGHT ))
                {
                        SysMessage( "Target is not in line of sight" );
                        return( false );
                }
                goto facepoint;
        }
    
        return( true );
    }
    
    bool CChar::Spell_Unequip( LAYER_TYPE layer )
    {
        CItem * pItemPrev = LayerFind( layer );
        if ( pItemPrev != NULL )
        {
                if ( ! CanMove( pItemPrev ))
                        return( false );
                if ( ! pItemPrev->IsType(IT_SPELLBOOK) && ! pItemPrev->IsType(IT_WAND))
                {
                        ItemBounce( pItemPrev );
                }
        }
        return( true );
    }
    
    bool CChar::Spell_CastDone( void )
    {
        // Spell_CastDone
        // Ready for the spell effect.
        // m_Act_TargPrv = spell was magic item or scroll ?
        // RETURN:
        //  false = fail.
        // ex. magery skill goes up FAR less if we use a scroll or magic device !
        //
    
        if ( ! Spell_TargCheck())
                return( false );
    
        CObjBase * pObj = m_Act_Targ.ObjFind(); // dont always need a target.
        CObjBase * pObjSrc = m_Act_TargPrv.ObjFind();
    
        SPELL_TYPE spell = m_atMagery.m_Spell;
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
        if ( pSpellDef == NULL )
                return( false );
    
        int iSkillLevel;
        if ( pObjSrc != this )
        {
                // Get the strength of the item. IT_SCROLL or IT_WAND
                CItem * pItem = dynamic_cast <CItem*>(pObjSrc);
                if ( pItem == NULL )
                        return( false );
                if ( ! pItem->m_itWeapon.m_spelllevel )
                        iSkillLevel = Calc_GetRandVal( 500 );
                else
                        iSkillLevel = pItem->m_itWeapon.m_spelllevel;
                if ( pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ))
                {
                        // do something bad.
                        spell = SPELL_Curse;
                        pSpellDef = g_Cfg.GetSpellDef(SPELL_Curse);
                        pItem->SetAttr(ATTR_IDENTIFIED);
                        pObj = this;
                        SysMessage( "Cursed Magic!" );
                }
        }
        else
        {
                iSkillLevel = Skill_GetAdjusted( SKILL_MAGERY );
        }
    
        // Consume the reagents/mana/scroll/charge
        if ( ! Spell_CanCast( spell, false, pObjSrc, true ))
                return( false );
    
        switch ( spell )
        {
                // 1st
        case SPELL_Create_Food:
                // Create object. Normally food.
                if ( pObj == NULL )
                {
                        static const ITEMID_TYPE sm_Item_Foods =        // possible foods.
                        {
                                ITEMID_FOOD_BACON,
                                ITEMID_FOOD_SAUSAGE,
                                ITEMID_FOOD_HAM,
                                ITEMID_FOOD_CAKE,
                                ITEMID_FOOD_BREAD,
                        };
    
                        CItem * pItem = CItem::CreateScript( sm_Item_Foods );
                        pItem->SetType( IT_FOOD );      // should already be set .
                        pItem->MoveToCheck( m_Act_p, this );
                }
                break;
    
        case SPELL_Magic_Arrow:
                Spell_Bolt( pObj, ITEMID_FX_MAGIC_ARROW, iSkillLevel );
                break;
    
        case SPELL_Heal:
        case SPELL_Night_Sight:
        case SPELL_Reactive_Armor:
    
        case SPELL_Clumsy:
        case SPELL_Feeblemind:
        case SPELL_Weaken:
    simple_effect:
                if ( pObj == NULL )
                        return( false );
                pObj->OnSpellEffect( spell, this, iSkillLevel, dynamic_cast <CItem*>( pObjSrc ));
                break;
    
                // 2nd
        case SPELL_Agility:
        case SPELL_Cunning:
        case SPELL_Cure:
        case SPELL_Protection:
        case SPELL_Strength:
    
        case SPELL_Harm:
                goto simple_effect;
    
        case SPELL_Magic_Trap:
        case SPELL_Magic_Untrap:
                // Create the trap object and link it to the target. ???
                // A container is diff from door or stationary object
                break;
    
                // 3rd
        case SPELL_Bless:
    
        case SPELL_Poison:
                goto simple_effect;
        case SPELL_Fireball:
                Spell_Bolt( pObj, ITEMID_FX_FIRE_BALL, iSkillLevel );
                break;
    
        case SPELL_Magic_Lock:
        case SPELL_Unlock:
                goto simple_effect;
    
        case SPELL_Telekin:
                // Act as dclick on the object.
                Use_Obj( pObj, false );
                break;
        case SPELL_Teleport:
                Spell_Teleport( m_Act_p );
                break;
        case SPELL_Wall_of_Stone:
                Spell_Field( m_Act_p, ITEMID_STONE_WALL, ITEMID_STONE_WALL, iSkillLevel );
                break;
    
                // 4th
        case SPELL_Arch_Cure:
        case SPELL_Arch_Prot:
        {
                Spell_Area( m_Act_p, 5, iSkillLevel );
                break;
        }
        case SPELL_Great_Heal:
        case SPELL_Curse:
        case SPELL_Lightning:
                goto simple_effect;
        case SPELL_Fire_Field:
                Spell_Field( m_Act_p, ITEMID_FX_FIRE_F_EW, ITEMID_FX_FIRE_F_NS, iSkillLevel );
                break;
    
        case SPELL_Recall:
                if ( ! Spell_Recall( dynamic_cast <CItem*> (pObj), false ))
                        return( false );
                break;
    
                // 5th
    
        case SPELL_Blade_Spirit:
                m_atMagery.m_SummonID = CREID_BLADES;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
    
        case SPELL_Dispel_Field:
                {
                        CItem * pItem = dynamic_cast <CItem*> (pObj);
                        if ( pItem == NULL ||
                                ( ! pItem->IsType(IT_CAMPFIRE) &&
                                ! pItem->IsType(IT_SPELL) &&
                                ! pItem->IsType(IT_FIRE)))
                        {
                                SysMessage( "That is not a field!" );
                                return( false );
                        }
                        pItem->OnSpellEffect( SPELL_Dispel_Field, this, iSkillLevel, NULL );
                        break;
                }
    
        case SPELL_Mind_Blast:
                if ( pObj->IsChar())
                {
                        CChar * pChar = dynamic_cast <CChar*> ( pObj );
                        ASSERT( pChar );
                        int iDiff = ( Stat_Get(STAT_INT) - pChar->Stat_Get(STAT_INT) ) / 2;
                        if ( iDiff < 0 )
                        {
                                pChar = this;   // spell revereses !
                                iDiff = -iDiff;
                        }
                        int iMax = pChar->Stat_Get(STAT_STR) / 4;
                        pChar->OnSpellEffect( spell, this, min( iDiff, iMax ), NULL );
                }
                break;
    
        case SPELL_Magic_Reflect:
    
        case SPELL_Paralyze:
        case SPELL_Incognito:
                goto simple_effect;
    
        case SPELL_Poison_Field:
                Spell_Field( m_Act_p, (ITEMID_TYPE) 0x3915, ITEMID_FX_POISON_F_NS, iSkillLevel );
                break;
    
        case SPELL_Summon:
        summon_effect:
                Spell_Summon( m_atMagery.m_SummonID, m_Act_p, m_atMagery.m_fSummonPet );
                break;
    
                // 6th
    
        case SPELL_Invis:
    
        case SPELL_Dispel:
                goto simple_effect;
    
        case SPELL_Energy_Bolt:
                Spell_Bolt( pObj, ITEMID_FX_ENERGY_BOLT, iSkillLevel );
                break;
    
        case SPELL_Explosion:
                Spell_Area( m_Act_p, 2, iSkillLevel );
                break;
    
        case SPELL_Mark:
                goto simple_effect;
    
        case SPELL_Mass_Curse:
                Spell_Area( m_Act_p, 5, iSkillLevel );
                break;
        case SPELL_Paralyze_Field:
                Spell_Field( m_Act_p, ITEMID_FX_PARA_F_EW, ITEMID_FX_PARA_F_NS, iSkillLevel );
                break;
        case SPELL_Reveal:
                Spell_Area( m_Act_p, UO_MAP_VIEW_SIGHT, iSkillLevel );
                break;
    
                // 7th
    
        case SPELL_Chain_Lightning:
                Spell_Area( m_Act_p, 5, iSkillLevel );
                break;
        case SPELL_Energy_Field:
                Spell_Field( m_Act_p, ITEMID_FX_ENERGY_F_EW, ITEMID_FX_ENERGY_F_NS, iSkillLevel );
                break;
    
        case SPELL_Flame_Strike:
                // Display spell.
                if ( pObj == NULL )
                {
                        CItem * pItem = CItem::CreateBase( ITEMID_FX_FLAMESTRIKE );
                        ASSERT(pItem);
                        pItem->SetType( IT_SPELL );
                        pItem->m_itSpell.m_spell = SPELL_Flame_Strike;
                        pItem->MoveToDecay( m_Act_p, 2*TICK_PER_SEC );
                }
                else
                {
                        pObj->Effect( EFFECT_OBJ, ITEMID_FX_FLAMESTRIKE, pObj, 6, 15 );
                        // Burn person at location.
                        goto simple_effect;
                }
                break;
    
        case SPELL_Gate_Travel:
                if ( ! Spell_Recall( dynamic_cast <CItem*> (pObj), true ))
                        return( false );
                break;
    
        case SPELL_Mana_Drain:
        case SPELL_Mana_Vamp:
                // Take the mana from the target.
                if ( pObj->IsChar() && this != pObj )
                {
                        CChar * pChar = dynamic_cast <CChar*> ( pObj );
                        ASSERT( pChar );
                        if ( ! pChar->IsStatFlag( STATF_Reflection ))
                        {
                                int iMax = pChar->Stat_Get(STAT_INT);
                                int iDiff = Stat_Get(STAT_INT) - iMax;
                                if ( iDiff < 0 )
                                        iDiff = 0;
                                else
                                        iDiff = Calc_GetRandVal( iDiff );
                                iDiff += Calc_GetRandVal( 25 );
                                pChar->OnSpellEffect( spell, this, iDiff, NULL );
                                if ( spell == SPELL_Mana_Vamp )
                                {
                                        // Give some back to me.
                                        UpdateStatVal( STAT_INT, min( iDiff, Stat_Get(STAT_INT)));
                                }
                                break;
                        }
                }
                goto simple_effect;
    
        case SPELL_Mass_Dispel:
                Spell_Area( m_Act_p, 15, iSkillLevel );
                break;
    
        case SPELL_Meteor_Swarm:
                // Multi explosion ??? 0x36b0
                Spell_Area( m_Act_p, 4, iSkillLevel );
                break;
    
        case SPELL_Polymorph:
                // This has a menu select for client.
                if ( GetPrivLevel() < PLEVEL_Seer )
                {
                        if ( pObj != this )
                                return( false );
                }
                goto simple_effect;
    
                // 8th
    
        case SPELL_Earthquake:
                Spell_Area( GetTopPoint(), UO_MAP_VIEW_SIGHT, iSkillLevel );
                break;
    
        case SPELL_Vortex:
                m_atMagery.m_SummonID = CREID_VORTEX;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
    
        case SPELL_Resurrection:
        case SPELL_Light:
                goto simple_effect;
    
        case SPELL_Air_Elem:
                m_atMagery.m_SummonID = CREID_AIR_ELEM;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
        case SPELL_Daemon:
                m_atMagery.m_SummonID = ( Calc_GetRandVal( 2 )) ? CREID_DAEMON_SWORD : CREID_DAEMON;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
        case SPELL_Earth_Elem:
                m_atMagery.m_SummonID = CREID_EARTH_ELEM;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
        case SPELL_Fire_Elem:
                m_atMagery.m_SummonID = CREID_FIRE_ELEM;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
        case SPELL_Water_Elem:
                m_atMagery.m_SummonID = CREID_WATER_ELEM;
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
    
                // Necro
        case SPELL_Summon_Undead:
                switch (Calc_GetRandVal(15))
                {
                case 1:
                        m_atMagery.m_SummonID = CREID_LICH;
                        break;
                case 3:
                case 5:
                case 7:
                case 9:
                        m_atMagery.m_SummonID = CREID_SKELETON;
                        break;
                default:
                        m_atMagery.m_SummonID = CREID_ZOMBIE;
                        break;
                }
                m_atMagery.m_fSummonPet = true;
                goto summon_effect;
    
        case SPELL_Animate_Dead:
                {
                        CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pObj);
                        if ( pCorpse == NULL )
                        {
                                SysMessage( "That is not a corpse!" );
                                return( false );
                        }
                        if ( IsPriv( PRIV_GM ))
                        {
                                m_atMagery.m_SummonID = pCorpse->m_itCorpse.m_BaseID;
                        }
                        else if ( CCharBase::IsHumanID( pCorpse->GetCorpseType()))      // Must be a human corpse ?
                        {
                                m_atMagery.m_SummonID = CREID_ZOMBIE;
                        }
                        else
                        {
                                m_atMagery.m_SummonID = pCorpse->GetCorpseType();
                        }
                        m_atMagery.m_fSummonPet = true;
    
                        if ( ! pCorpse->IsTopLevel())
                        {
                                return( false );
                        }
                        CChar * pChar = Spell_Summon( m_atMagery.m_SummonID, pCorpse->GetTopPoint(), true );
                        ASSERT( pChar );
                        if ( ! pChar->RaiseCorpse( pCorpse ))
                        {
                                SysMessage( "The corpse stirs for a moment then falls!" );
                                pChar->Delete();
                        }
                        break;
                }
    
        case SPELL_Bone_Armor:
                {
                        CItemCorpse * pCorpse = dynamic_cast <CItemCorpse*> (pObj);
                        if ( pCorpse == NULL )
                        {
                                SysMessage( "That is not a corpse!" );
                                return( false );
                        }
                        if ( ! pCorpse->IsTopLevel() ||
                                pCorpse->GetCorpseType() != CREID_SKELETON )    // Must be a skeleton corpse
                        {
                                SysMessage( "The body stirs for a moment" );
                                return( false );
                        }
                        // Dump any stuff on corpse
                        pCorpse->ContentsDump( pCorpse->GetTopPoint());
                        pCorpse->Delete();
    
                        static const ITEMID_TYPE sm_Item_Bone =
                        {
                                ITEMID_BONE_ARMS,
                                ITEMID_BONE_ARMOR,
                                ITEMID_BONE_GLOVES,
                                ITEMID_BONE_HELM,
                                ITEMID_BONE_LEGS,
                        };
    
                        int iGet = 0;
                        for ( int i=0; i<COUNTOF(sm_Item_Bone); i++ )
                        {
                                if ( ! Calc_GetRandVal( 2+iGet ))
                                        break;
                                CItem * pItem = CItem::CreateScript( (ITEMID_TYPE) sm_Item_Bone );
                                pItem->MoveToCheck( m_Act_p, this );
                                iGet++;
                        }
                        if ( ! iGet )
                        {
                                SysMessage( "The bones shatter into dust!" );
                                break;
                        }
                }
                break;
    
        case SPELL_Fire_Bolt:
                Spell_Bolt( pObj, ITEMID_FX_FIRE_BOLT, iSkillLevel );
                break;
    
        case SPELL_Ale:         // 90 = drunkeness ?
        case SPELL_Wine:        // 91 = mild drunkeness ?
        case SPELL_Liquor:      // 92 = extreme drunkeness ?
        case SPELL_Hallucination:
        case SPELL_Stone:
        case SPELL_Shrink:
        case SPELL_Mana:
        case SPELL_Refresh:
        case SPELL_Restore:             // increases both your hit points and your stamina.
        case SPELL_Sustenance:          //  // serves to fill you up. (Remember, healing rate depends on how well fed you are!)
        case SPELL_Forget:                      //  // permanently lowers one skill.
        case SPELL_Gender_Swap:         //  // permanently changes your gender.
        case SPELL_Chameleon:           //  // makes your skin match the colors of whatever is behind you.
        case SPELL_BeastForm:           //  // polymorphs you into an animal for a while.
        case SPELL_Monster_Form:        //  // polymorphs you into a monster for a while.
        case SPELL_Trance:                      //  // temporarily increases your meditation skill.
        case SPELL_Particle_Form:       //  // turns you into an immobile, but untargetable particle system for a while.
        case SPELL_Shield:                      //  // erects a temporary force field around you. Nobody approaching will be abl
e to get within 1 tile of you, though you can move close to them if you wish.
        case SPELL_Steelskin:           //  // turns your skin into steel, giving a boost to your AR.
        case SPELL_Stoneskin:           //  // turns your skin into stone, giving a boost to your AR.
                goto simple_effect;
    
        default:
                // No effect on creatures it seems.
                break;
        }
    
        if ( g_Cfg.m_fHelpingCriminalsIsACrime &&
                pObj != NULL &&
                pObj->IsChar() &&
                pObj != this &&
                pSpellDef->IsSpellType(SPELLFLAG_GOOD))
        {
                CChar * pChar = dynamic_cast <CChar*> ( pObj );
                ASSERT( pChar );
                switch ( pChar->Noto_GetFlag( this, false ))
                {
                case NOTO_CRIMINAL:
                case NOTO_GUILD_WAR:
                case NOTO_EVIL:
                        Noto_Criminal();
                        break;
                }
        }
    
        // Make noise.
        if ( ! IsStatFlag( STATF_Insubstantial ))
        {
                Sound( pSpellDef->m_sound );
        }
        return( true );
    }
    
    void CChar::Spell_CastFail()
    {
        Effect( EFFECT_OBJ, ITEMID_FX_SPELL_FAIL, this, 1, 30 );
        Sound( SOUND_SPELL_FIZZLE );
        if ( g_Cfg.m_fReagentLossFail )
        {
                // consume the regs.
                Spell_CanCast( m_atMagery.m_Spell, false, m_Act_TargPrv.ObjFind(), false );
        }
    }
    
    int CChar::Spell_CastStart()
    {
        // Casting time goes up with difficulty
        // but down with skill, int and dex
        // ARGS:
        //  m_Act_p = location to cast to.
        //  m_atMagery.m_Spell = the spell.
        //  m_Act_TargPrv = the source of the spell.
        //  m_Act_Targ = target for the spell.
        // RETURN:
        //  0-100
        //  -1 = instant failure.
    
        if ( ! Spell_TargCheck())
                return( -1 );
    
        // Animate casting.
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(m_atMagery.m_Spell);
        if ( pSpellDef == NULL )
                return( -1 );
    
        UpdateAnimate(( pSpellDef->IsSpellType( SPELLFLAG_DIR_ANIM )) ? ANIM_CAST_DIR : ANIM_CAST_AREA );
    
        bool fWOP = ( GetPrivLevel() >= PLEVEL_Seer ) ?
                g_Cfg.m_fWordsOfPowerStaff :
                g_Cfg.m_fWordsOfPowerPlayer ;
    
        if ( ! NPC_CanSpeak() || IsStatFlag( STATF_Insubstantial ))
        {
                fWOP = false;
        }
    
        int i = pSpellDef->m_SkillReq.FindResourceType( RES_SKILL );
        if ( i < 0 )
                return( false );
        int iDifficulty = pSpellDef->m_SkillReq.GetResQty() / 10;
    
        CGrayUID uid( m_Act_TargPrv );
        CItem * pItem = uid.ItemFind();
        if ( pItem != NULL )
        {
                if ( pItem->IsType(IT_WAND))
                {
                        // Wand use no words of power. and require no magery.
                        fWOP = false;
                        iDifficulty = 1;
                }
                else
                {
                        // Scroll
                        iDifficulty /= 2;
                }
        }
    
        if ( ! g_Cfg.m_fEquippedCast && fWOP )
        {
                // Attempt to Unequip stuff before casting.
                // Except not wands and spell books !
                if ( ! Spell_Unequip( LAYER_HAND1 ))
                        return( -1 );
                if ( ! Spell_Unequip( LAYER_HAND2 ))
                        return( -1 );
        }
    
        if ( fWOP )
        {
                int len=0;
                TCHAR szTemp;
                int i=0;
                for ( ; true; i++ )
                {
                        TCHAR ch = pSpellDef->m_sRunes;
                        if ( ! ch )
                                break;
                        len += strcpylen( szTemp+len, g_Cfg.GetRune(ch));
                        szTemp=' ';
                }
                if ( i )
                {
                        szTemp = '\0';
                        Speak( szTemp );
                }
        }
    
        CScriptTriggerArgs Args( (int) m_atMagery.m_Spell, iDifficulty, pItem );
        if ( OnTrigger( CTRIG_SpellCast, this, &Args ) == TRIGRET_RET_TRUE )
                return( -1 );
    
        int iWaitTime;
        if ( IsPriv( PRIV_GM ))
                iWaitTime = 1;
        else
                iWaitTime = pSpellDef->m_iCastTime;
    
        SetTimeout( iWaitTime );
    
        return( iDifficulty );
    }
    
    bool CChar::OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel, CItem * pSourceItem )
    {
        // Spell has a direct effect on this char.
        // This should effect noto of source.
        // ARGS:
        //  pSourceItem = the potion, wand, scroll etc. NULL = cast (IT_SPELL)
        //  iSkillLevel = 0-1000 = difficulty. may be slightly larger .
        // RETURN:
        //  false = the spell did not work. (should we get credit ?)
    
        if ( this == NULL )
                return( false );
        ASSERT( ! IsItem());
    
        if ( iSkillLevel <= 0 ) // spell died (fizzled?).
                return( false );
    
        CScriptTriggerArgs Args( (int) spell, iSkillLevel, pSourceItem );
        if ( OnTrigger( CTRIG_SpellEffect, pCharSrc, &Args ) == TRIGRET_RET_TRUE )
                return( false );
    
        const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(spell);
        if ( pSpellDef == NULL )
                return( false );
    
        // Most spells don't work on ghosts.
        if ( IsStatFlag( STATF_DEAD ) && spell != SPELL_Resurrection )
                return false;
    
        bool fResistAttempt = true;
        switch ( spell )        // just strengthen the effect.
        {
        case SPELL_Poison:
        case SPELL_Poison_Field:
                if ( IsStatFlag(STATF_Poisoned))
                {
                        fResistAttempt = false;
                }       // no further effect. don't count resist effect.
                break;
        case SPELL_Paralyze_Field:
        case SPELL_Paralyze:
                if ( IsStatFlag(STATF_Freeze))
                        return false;   // no further effect.
                break;
        }
    
        bool fPotion = ( pSourceItem != NULL && pSourceItem->IsType( IT_POTION ));
        if ( fPotion )
                fResistAttempt = false;
        if ( pCharSrc == this )
                fResistAttempt = false;
    
        if ( pSpellDef->IsSpellType( SPELLFLAG_HARM ))
        {
                // Can't harm yourself directly ?
                if ( pCharSrc == this )
                        return( false );
    
                if ( IsStatFlag( STATF_INVUL ))
                {
                        Effect( EFFECT_OBJ, ITEMID_FX_GLOW, this, 9, 30, false );
                        return false;
                }
    
                if ( ! fPotion && fResistAttempt )
                {
                        if ( pCharSrc != NULL && GetPrivLevel() > PLEVEL_Guest )
                        {
                                if ( pCharSrc->GetPrivLevel() <= PLEVEL_Guest )
                                {
                                        pCharSrc->SysMessage( "The guest curse strikes you." );
                                        goto reflectit;
                                }
                        }
    
                        // Check resistance to magic ?
                        if ( pSpellDef->IsSpellType( SPELLFLAG_RESIST ))
                        {
                        if ( Skill_UseQuick( SKILL_MAGICRESISTANCE, iSkillLevel ))
                        {
                                SysMessage( "You feel yourself resisting magic" );
    
                                // iSkillLevel
                                iSkillLevel /= 2;       // ??? reduce effect of spell.
                        }
    
                        // Check magic reflect.
                        if ( IsStatFlag( STATF_Reflection ))    // reflected.
                        {
                                StatFlag_Clear( STATF_Reflection );
    reflectit:
                                Effect( EFFECT_OBJ, ITEMID_FX_GLOW, this, 9, 30, false );
                                if ( pCharSrc != NULL )
                                {
                                        pCharSrc->OnSpellEffect( spell, NULL, iSkillLevel/2, pSourceItem );
                                }
                                return false;
                        }
                }
                }
    
                if ( ! OnAttackedBy( pCharSrc, 1, false ))
                        return false;
        }
    
        if ( pSpellDef->IsSpellType( SPELLFLAG_FX_TARG ) &&
                pSpellDef->m_idEffect )
        {
                Effect( EFFECT_OBJ, pSpellDef->m_idEffect, this, 0, 15 ); // 9, 14
        }
    
        iSkillLevel = iSkillLevel/2 + Calc_GetRandVal(iSkillLevel/2);   // randomize the effect.
    
        switch ( spell )
        {
    
        case SPELL_Ale:         // 90 = drunkeness ?
        case SPELL_Wine:        // 91 = mild drunkeness ?
        case SPELL_Liquor:      // 92 = extreme drunkeness ?
    
        case SPELL_Clumsy:
        case SPELL_Feeblemind:
        case SPELL_Weaken:
        case SPELL_Agility:
        case SPELL_Cunning:
        case SPELL_Strength:
        case SPELL_Bless:
        case SPELL_Curse:
        case SPELL_Mass_Curse:
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_STATS, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Heal:
        case SPELL_Great_Heal:
                if ( iSkillLevel > 1000 )
                {
                        UpdateStatVal( STAT_STR, g_Cfg.GetSpellEffect( spell, iSkillLevel ), Stat_Get(STAT_STR) + 20 );
                }
                else
                {
                        UpdateStatVal( STAT_STR, g_Cfg.GetSpellEffect( spell, iSkillLevel ));
                }
                break;
    
        case SPELL_Night_Sight:
                Spell_Effect_Create( SPELL_Night_Sight, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Night_Sight, iSkillLevel, 0, p
CharSrc );
                break;
    
        case SPELL_Reactive_Armor:
                Spell_Effect_Create( SPELL_Reactive_Armor, LAYER_SPELL_Reactive, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Magic_Reflect:
                Spell_Effect_Create( SPELL_Magic_Reflect, LAYER_SPELL_Magic_Reflect, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Poison:
        case SPELL_Poison_Field:
                if ( ! fPotion )
                {
                        Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
                }
                SetPoison( iSkillLevel, iSkillLevel/50, pCharSrc );
                break;
    
        case SPELL_Cure:
                SetPoisonCure( iSkillLevel, iSkillLevel > 900 );
                break;
        case SPELL_Arch_Cure:
                SetPoisonCure( iSkillLevel, true );
                break;
    
        case SPELL_Protection:
        case SPELL_Arch_Prot:
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Protection, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Dispel:
        case SPELL_Mass_Dispel:
                // ??? should be difficult to dispel SPELL_Summon creatures
                Spell_Dispel( (pCharSrc != NULL && pCharSrc->IsPriv(PRIV_GM)) ? 150 : 50);
                break;
    
        case SPELL_Reveal:
                if ( ! Reveal())
                        break;
                Effect( EFFECT_OBJ, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
                break;
    
        case SPELL_Invis:
                Spell_Effect_Create( SPELL_Invis, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Invis, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Incognito:
                Spell_Effect_Create( SPELL_Incognito, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Incognito, iSkillLevel, 0, pChar
Src );
                break;
    
        case SPELL_Particle_Form:       // 112 // turns you into an immobile, but untargetable particle system for a while.
        case SPELL_Stone:
        case SPELL_Paralyze_Field:
        case SPELL_Paralyze:
                // Effect( EFFECT_OBJ, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Paralyze, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Mana_Drain:
        case SPELL_Mana_Vamp:
                UpdateStatVal( STAT_INT, -iSkillLevel );
                break;
    
        case SPELL_Harm:
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_POISON | DAMAGE_MAGIC | DAMAGE_GENERA
L );
                break;
        case SPELL_Mind_Blast:
                OnTakeDamage( iSkillLevel, pCharSrc, DAMAGE_POISON | DAMAGE_MAGIC | DAMAGE_GENERAL );
                break;
        case SPELL_Explosion:
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT_BLUNT | DAMAGE_GEN
ERAL );
                break;
        case SPELL_Energy_Bolt:
        case SPELL_Magic_Arrow:
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT_PIERCE );
                break;
        case SPELL_Fireball:
        case SPELL_Fire_Bolt:
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT_BLUNT | DAMAGE_FIR
E );
                break;
        case SPELL_Fire_Field:
        case SPELL_Flame_Strike:
                // Burn whoever is there.
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_MAGIC | DAMAGE_FIRE | DAMAGE_GENERAL 
);
                break;
        case SPELL_Meteor_Swarm:
                Effect( EFFECT_OBJ, ITEMID_FX_EXPLODE_3, this, 9, 6 );
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_MAGIC | DAMAGE_HIT_BLUNT | DAMAGE_FIR
E );
                break;
        case SPELL_Earthquake:
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_HIT_BLUNT | DAMAGE_GENERAL );
                break;
        case SPELL_Lightning:
        case SPELL_Chain_Lightning:
                GetTopSector()->LightFlash();
                Effect( EFFECT_LIGHTNING, ITEMID_NOTHING, pCharSrc );
                OnTakeDamage( g_Cfg.GetSpellEffect( spell, iSkillLevel ), pCharSrc, DAMAGE_ELECTRIC | DAMAGE_GENERAL );
                break;
    
        case SPELL_Resurrection:
                return Spell_Resurrection( (pCharSrc && pCharSrc->IsPriv(PRIV_GM)) ? -1 : 0, NULL );
    
        case SPELL_Light:
                Effect( EFFECT_OBJ, ITEMID_FX_HEAL_EFFECT, this, 9, 6 );
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_LIGHT, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Hallucination:
                {
                CItem * pItem = Spell_Effect_Create( SPELL_Hallucination, LAYER_FLAG_Hallucination, iSkillLevel, 10*TICK_PER_SEC
, pCharSrc );
                pItem->m_itSpell.m_spellcharges = Calc_GetRandVal(30);
                }
                break;
        case SPELL_Polymorph:
                {
                        CREID_TYPE creid = m_atMagery.m_SummonID;
    #define SPELL_MAX_POLY_STAT 150
    
                        CItem * pSpell = Spell_Effect_Create( SPELL_Polymorph, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Polymor
ph, iSkillLevel, 0, pCharSrc );
                        SetID(creid);
    
                        CCharBase * pCharDef = Char_GetDef();
                        ASSERT(pCharDef);
    
                        // set to creature type stats.
                        if ( pCharDef->m_Str )
                        {
                                int iStatPrv = Stat_Get(STAT_STR);
                                int iChange = pCharDef->m_Str - iStatPrv;
                                if ( iChange > SPELL_MAX_POLY_STAT )
                                        iChange = SPELL_MAX_POLY_STAT;
                                if ( iChange < -50 )
                                        iChange = -50;
                                Stat_Set( STAT_STR, iChange + iStatPrv );
                                pSpell->m_itSpell.m_PolyStr = Stat_Get(STAT_STR) - iStatPrv;
                        }
                        else
                        {
                                pSpell->m_itSpell.m_PolyStr = 0;
                        }
                        if ( pCharDef->m_Dex )
                        {
                                int iStatPrv = Stat_Get(STAT_DEX);
                                int iChange = pCharDef->m_Dex - iStatPrv;
                                if ( iChange > SPELL_MAX_POLY_STAT )
                                        iChange = SPELL_MAX_POLY_STAT;
                                if ( iChange < -50 )
                                        iChange = -50;
                                Stat_Set( STAT_DEX, iChange + iStatPrv );
                                pSpell->m_itSpell.m_PolyDex = Stat_Get(STAT_DEX) - iStatPrv;
                        }
                        else
                        {
                                pSpell->m_itSpell.m_PolyDex = 0;
                        }
                        Update();               // show everyone I am now a new type
                }
                break;
    
        case SPELL_Shrink:
                // Getting a pet to drink this is funny.
                if ( m_pPlayer )
                        break;
                if ( fPotion && pSourceItem )
                {
                        pSourceItem->Delete();
                }
                NPC_Shrink();   // this delete's the char !!!
                break;
    
        case SPELL_Mana:
                if ( iSkillLevel > 1000 )
                {
                        UpdateStatVal( STAT_INT, g_Cfg.GetSpellEffect( spell, iSkillLevel ), Stat_Get(STAT_INT) + 20 );
                }
                else
                {
                        UpdateStatVal( STAT_INT, g_Cfg.GetSpellEffect( spell, iSkillLevel ));
                }
                break;
    
        case SPELL_Refresh:
                if ( iSkillLevel > 1000 )
                {
                        UpdateStatVal( STAT_DEX, g_Cfg.GetSpellEffect( spell, iSkillLevel ), Stat_Get(STAT_DEX) + 20 );
                }
                else
                {
                        UpdateStatVal( STAT_DEX, g_Cfg.GetSpellEffect( spell, iSkillLevel ));
                }
                break;
    
        case SPELL_Restore:             // increases both your hit points and your stamina.
                UpdateStatVal( STAT_DEX, g_Cfg.GetSpellEffect( spell, iSkillLevel ));
                UpdateStatVal( STAT_STR, g_Cfg.GetSpellEffect( spell, iSkillLevel ));
                break;
    
        case SPELL_Forget:                      // 109 // permanently lowers one skill.
                {
                        int iSkillLevel = 0;
                        Skill_Degrade(SKILL_QTY);
                }
                break;
        case SPELL_Sustenance:          // 105 // serves to fill you up. (Remember, healing rate depends on how well fed you are
!)
                {
                        CCharBase * pCharDef = Char_GetDef();
                        ASSERT(pCharDef);
                        m_food = pCharDef->m_MaxFood + (pCharDef->m_MaxFood/2);
                }
                break;
        case SPELL_Gender_Swap:         // 110 // permanently changes your gender.
                if ( IsHuman())
                {
                        CCharBase * pCharDef = Char_GetDef();
                        ASSERT(pCharDef);
    
                        SetID( pCharDef->IsFemale() ? CREID_MAN : CREID_WOMAN );
                        m_prev_id = GetID();
                        Update();
                }
                break;
    
        case SPELL_Chameleon:           // 106 // makes your skin match the colors of whatever is behind you.
        case SPELL_BeastForm:           // 107 // polymorphs you into an animal for a while.
        case SPELL_Monster_Form:        // 108 // polymorphs you into a monster for a while.
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Polymorph, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Trance:                      // 111 // temporarily increases your meditation skill.
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_STATS, iSkillLevel, 0, pCharSrc );
                break;
    
        case SPELL_Shield:                      // 113 // erects a temporary force field around you. Nobody approaching will be 
able to get within 1 tile of you, though you can move close to them if you wish.
        case SPELL_Steelskin:           // 114 // turns your skin into steel, giving a boost to your AR.
        case SPELL_Stoneskin:           // 115 // turns your skin into stone, giving a boost to your AR.
                Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_Protection, iSkillLevel, -1, pCharSrc );
                break;
    
        case SPELL_Regenerate:
                // Set number of charges based on effect level.
                //
                {
                        int iDuration = pSpellDef->m_Duration.GetLinear( iSkillLevel );
                        iDuration /= (2*TICK_PER_SEC);
                        if ( iDuration <= 0 )
                                iDuration = 1;
                        CItem * pSpell = Spell_Effect_Create( spell, fPotion ? LAYER_FLAG_Potion : LAYER_SPELL_STATS, iSkillLeve
l, 2*TICK_PER_SEC, pCharSrc );
                        ASSERT(pSpell);
                        pSpell->m_itSpell.m_spellcharges = iDuration;
                }
                break;
    
        default:
                // seems to have no effect.
                break;
        }
        return( true );
    }