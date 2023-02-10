//
// CCharAct.cpp
//

#include "graysvr.h"	// predef header.
#include "CClient.h"
#include "../network/network.h"
#include "../network/send.h"

bool CChar::TeleportToObj( int iType, TCHAR * pszArgs )
{
	ADDTOCALLSTACK("CChar::TeleportToObj");
	// "GONAME", "GOTYPE", "GOCHAR"
	// 0 = object name
	// 1 = char
	// 2 = item type

	DWORD dwUID = m_Act_Targ.GetObjUID() &~ UID_F_ITEM;
	DWORD dwTotal = g_World.GetUIDCount();
	DWORD dwCount = dwTotal-1;

	int iArg = 0;
	if ( iType )
	{
		if ( pszArgs[0] && iType == 1 )
			dwUID = 0;
		iArg = RES_GET_INDEX( Exp_GetVal( pszArgs ));
	}

	while ( dwCount-- )
	{
		if ( ++dwUID >= dwTotal )
		{
			dwUID = 1;
		}
		CObjBase * pObj = g_World.FindUID(dwUID);
		if ( pObj == NULL )
			continue;

		switch ( iType )
		{
			case 0:
				{
					MATCH_TYPE match = Str_Match( pszArgs, pObj->GetName());
					if ( match != MATCH_VALID )
						continue;
				}
				break;
			case 1:	// char
				{
					if ( ! pObj->IsChar())
						continue;
					if ( iArg-- > 0 )
						continue;
				}
				break;
			case 2:	// item type
				{
					if ( ! pObj->IsItem())
						continue;
					CItem * pItem = dynamic_cast <CItem*>(pObj);
					if ( ! pItem->IsType(static_cast<IT_TYPE>(iArg)))
						continue;
				}
				break;
			case 3: // char id
				{
					if ( ! pObj->IsChar())
						continue;
					CChar * pChar = dynamic_cast <CChar*>(pObj);
					if ( pChar->GetID() != iArg )
						continue;
				}
				break;
			case 4:	// item id
				{
					if ( ! pObj->IsItem())
						continue;
					CItem * pItem = dynamic_cast <CItem*>(pObj);
					if ( pItem->GetID() != iArg )
						continue;
				}
				break;
		}

		CObjBaseTemplate * pObjTop = pObj->GetTopLevelObj();
		if ( pObjTop->IsChar())
		{
			if ( ! CanDisturb( dynamic_cast<CChar*>(pObjTop)))
				continue;
		}

		if ( pObjTop == this )
			continue;

		m_Act_Targ = pObj->GetUID();
		Spell_Teleport( pObjTop->GetTopPoint(), true, false );
		return( true );
	}
	return( false );
}

bool CChar::TeleportToCli( int iType, int iArgs )
{
	ADDTOCALLSTACK("CChar::TeleportToCli");
	
	ClientIterator it;
	for (CClient* pClient = it.next(); pClient != NULL; pClient = it.next())
	{
		if ( ! iType )
		{
			if ( pClient->GetSocketID() != iArgs )
				continue;
		}
		CChar * pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;
		if ( ! CanDisturb( pChar ))
			continue;
		if ( iType )
		{
			if ( iArgs-- )
				continue;
		}
		m_Act_Targ = pChar->GetUID();
		Spell_Teleport( pChar->GetTopPoint(), true, false );
		return( true );
	}
	return( false );
}

void CChar::Jail( CTextConsole * pSrc, bool fSet, int iCell )
{
	ADDTOCALLSTACK("CChar::Jail");

	CScriptTriggerArgs Args( fSet? 1 : 0, iCell, 0);

	if ( fSet )	// set the jailed flag.
	{
		if ( OnTrigger( CTRIG_Jailed, pSrc, &Args ) == TRIGRET_RET_TRUE )
			return;

		if ( m_pPlayer )	// allow setting of this to offline chars.
		{
			CAccount *pAccount = m_pPlayer->GetAccount();
			ASSERT(pAccount != NULL);

			pAccount->SetPrivFlags( PRIV_JAILED );
			pAccount->m_TagDefs.SetNum("JailCell", iCell, true);
		}
		if ( IsClient())
		{
			m_pClient->SetPrivFlags( PRIV_JAILED );
		}
		TCHAR szJailName[ 128 ];
		if ( iCell )
		{
			sprintf( szJailName, "jail%d", iCell );
		}
		else
		{
			strcpy( szJailName, "jail" );
		}
		Spell_Teleport( g_Cfg.GetRegionPoint( szJailName ), true, false );
		SysMessageDefault( DEFMSG_JAILED );
	}
	else	// forgive.
	{
		if ( OnTrigger( CTRIG_Jailed, pSrc, &Args ) == TRIGRET_RET_TRUE )
			return;

		if ( IsClient())
		{
			if ( ! m_pClient->IsPriv( PRIV_JAILED ))
				return;
			m_pClient->ClearPrivFlags( PRIV_JAILED );
		}
		if ( m_pPlayer )
		{
			CAccount *pAccount = m_pPlayer->GetAccount();
			ASSERT(pAccount != NULL);

			pAccount->ClearPrivFlags( PRIV_JAILED );
			if ( pAccount->m_TagDefs.GetKey("JailCell") != NULL )
				pAccount->m_TagDefs.DeleteKey("JailCell");
		}
		SysMessageDefault( DEFMSG_FORGIVEN );
	}
}

void CChar::AddGoldToPack( int iAmount, CItemContainer * pPack )
{
	ADDTOCALLSTACK("CChar::AddGoldToPack");
	// A vendor is giving me gold. put it in my pack or other place.

	if ( pPack == NULL )
		pPack = GetPackSafe();

	while ( iAmount > 0 )
	{
		CItem * pGold = CItem::CreateScript( ITEMID_GOLD_C1, this );

		int iGoldStack = minimum( iAmount, USHRT_MAX );
		pGold->SetAmount( iGoldStack );

		Sound( pGold->GetDropSound( pPack ));
		pPack->ContentAdd( pGold );
		iAmount -= iGoldStack;
	}
}

void CChar::LayerAdd( CItem * pItem, LAYER_TYPE layer )
{
	ADDTOCALLSTACK("CChar::LayerAdd");
	// add equipped items.
	// check for item already in that layer ?
	// NOTE: This could be part of the Load as well so it may not truly be being "equipped" at this time.
	// OnTrigger for equip is done by ItemEquip()

	if ( pItem == NULL )
		return;
	if ( pItem->GetParent() == this &&
		pItem->GetEquipLayer() == layer )
	{
		return;
	}

	if ( layer == LAYER_DRAGGING )
	{
		pItem->RemoveSelf();	// remove from where i am before add so UNEQUIP effect takes.
		// NOTE: CanEquipLayer may bounce an item . If it stacks with this we are in trouble !
	}

	if ( g_Serv.IsLoading() == false )
	{
		// This takes care of any conflicting items in the slot !
		layer = CanEquipLayer(pItem, layer, NULL, false);
		if ( layer == LAYER_NONE )
		{
			// we should not allow non-layered stuff to be put here ?
			// Put in pack instead ?
			ItemBounce( pItem );
			return;
		}
	}

	if ( layer == LAYER_SPECIAL )
	{
		if ( pItem->IsType( IT_EQ_TRADE_WINDOW ))
			layer = LAYER_NONE;
	}

	CContainer::ContentAddPrivate( pItem );
	pItem->SetEquipLayer( layer );

	// update flags etc for having equipped this.
	switch ( layer )
	{
		case LAYER_HAND1:
		case LAYER_HAND2:
			// If weapon
			if ( pItem->IsTypeWeapon())
			{
				m_uidWeapon = pItem->GetUID();
				Fight_ResetWeaponSwingTimer();
			}
			else if ( pItem->IsTypeArmor())
			{
				// Shield of some sort.
				m_defense = CalcArmorDefense();
				StatFlag_Set( STATF_HasShield );
				UpdateStatsFlag();
			}
			break;
		case LAYER_SHOES:
		case LAYER_PANTS:
		case LAYER_SHIRT:
		case LAYER_HELM:		// 6
		case LAYER_GLOVES:	// 7
		case LAYER_COLLAR:	// 10 = gorget or necklace.
		case LAYER_HALF_APRON:
		case LAYER_CHEST:	// 13 = armor chest
		case LAYER_TUNIC:	// 17 = jester suit
		case LAYER_ARMS:		// 19 = armor
		case LAYER_CAPE:		// 20 = cape
		case LAYER_ROBE:		// 22 = robe over all.
		case LAYER_SKIRT:
		case LAYER_LEGS:
			// If armor or clothing = change in defense rating.
			m_defense = CalcArmorDefense();
			UpdateStatsFlag();
			break;

			// These effects are not magical. (make them spells !)

		case LAYER_FLAG_Criminal:
			StatFlag_Set( STATF_Criminal );
			return;
		case LAYER_FLAG_SpiritSpeak:
			StatFlag_Set( STATF_SpiritSpeak );
			return;
		case LAYER_FLAG_Stuck:
			StatFlag_Set( STATF_Freeze );
			break;
		default:
			break;
	}

	if ( layer != LAYER_DRAGGING )
	{
		switch ( pItem->GetType())
		{
			case IT_EQ_SCRIPT:	// pure script.
				break;
			case IT_EQ_MEMORY_OBJ:
			{
				CItemMemory *pMemory = dynamic_cast<CItemMemory *>( pItem );
				if (pMemory != NULL)
					Memory_UpdateFlags(pMemory);
				break;
			}
			case IT_EQ_HORSE:
				StatFlag_Set(STATF_OnHorse);
				break;
			case IT_COMM_CRYSTAL:
				StatFlag_Set(STATF_COMM_CRYSTAL);
				break;
			default:
				break;
		}
	}

	pItem->Update();
}

void CChar::OnRemoveOb( CGObListRec* pObRec )	// Override this = called when removed from list.
{
	ADDTOCALLSTACK("CChar::OnRemoveOb");
	// Unequip the item.
	// This may be a delete etc. It can not FAIL !
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	if ( !pItem )
		return;

	LAYER_TYPE layer = pItem->GetEquipLayer();
	if ( layer != LAYER_DRAGGING && ! g_Serv.IsLoading())
	{
		pItem->OnTrigger( ITRIG_UNEQUIP, this );
	}

	CContainer::OnRemoveOb( pObRec );

	// remove equipped items effects
	switch ( layer )
	{
		case LAYER_HAND1:
		case LAYER_HAND2:	// other hand = shield
			if ( pItem->IsTypeWeapon())
			{
				m_uidWeapon.InitUID();
				Fight_ResetWeaponSwingTimer();
			}
			else if ( pItem->IsTypeArmor())
			{
				// Shield
				m_defense = CalcArmorDefense();
				StatFlag_Clear( STATF_HasShield );
				UpdateStatsFlag();
			}
			if (( this->m_Act_SkillCurrent == SKILL_MINING ) || ( this->m_Act_SkillCurrent == SKILL_FISHING ) || ( this->m_Act_SkillCurrent == SKILL_LUMBERJACKING ))
			{
				Skill_Cleanup();
			}
			break;
		case LAYER_SHOES:
		case LAYER_PANTS:
		case LAYER_SHIRT:
		case LAYER_HELM:		// 6
		case LAYER_GLOVES:	// 7
		case LAYER_COLLAR:	// 10 = gorget or necklace.
		case LAYER_CHEST:	// 13 = armor chest
		case LAYER_TUNIC:	// 17 = jester suit
		case LAYER_ARMS:		// 19 = armor
		case LAYER_CAPE:		// 20 = cape
		case LAYER_ROBE:		// 22 = robe over all.
		case LAYER_SKIRT:
		case LAYER_LEGS:
			m_defense = CalcArmorDefense();
			UpdateStatsFlag();
			break;

		case LAYER_FLAG_Criminal:
			StatFlag_Clear( STATF_Criminal );
			break;
		case LAYER_FLAG_SpiritSpeak:
			StatFlag_Clear( STATF_SpiritSpeak );
			break;
		case LAYER_FLAG_Stuck:
			StatFlag_Clear( STATF_Freeze );
			break;
		default:
			break;
	}

	// Items with magic effects.
	if ( layer != LAYER_DRAGGING )
	{
		switch ( pItem->GetType())
		{
			case IT_COMM_CRYSTAL:
				if ( ContentFind( RESOURCE_ID( RES_TYPEDEF,IT_COMM_CRYSTAL ), 0, 0 ) == NULL )
				{
					StatFlag_Clear(STATF_COMM_CRYSTAL);
				}
				break;
			case IT_EQ_HORSE:
				StatFlag_Clear(STATF_OnHorse);
				break;
			case IT_EQ_MEMORY_OBJ:
			{
				// Clear the associated flags.
				CItemMemory *pMemory = dynamic_cast<CItemMemory*>(pItem);
				if (pMemory != NULL)
					Memory_UpdateClearTypes( pMemory, 0xFFFF );
				break;
			}
			default:
				break;
		}

		// If items are magical then remove effect here.
		Spell_Effect_Remove(pItem);
	}
}

void CChar::DropAll( CItemContainer * pCorpse, WORD wAttr )
{
	ADDTOCALLSTACK("CChar::DropAll");
	// shrunk or died. (or sleeping)
	if ( IsStatFlag( STATF_Conjured ))
		return;	// drop nothing.

	CItemContainer * pPack = GetPack();
	if ( pPack != NULL )
	{
		if ( pCorpse == NULL )
		{
			pPack->ContentsDump( GetTopPoint(), wAttr );
		}
		else
		{
			pPack->ContentsTransfer( pCorpse, true );
		}

		//	close inventory gump of course
		pPack->RemoveFromView();
		pPack->Update();
	}

	// transfer equipped items to corpse or your pack (if newbie).
	UnEquipAllItems( pCorpse );
}

void CChar::UnEquipAllItems( CItemContainer * pDest, bool bLeaveHands )
{
	ADDTOCALLSTACK("CChar::UnEquipAllItems");
	// We morphed, sleeping, died or became a GM.
	// Pets can be told to "Drop All"
	// drop item that is up in the air as well.
	// pDest       = Container to place items in
	// bLeaveHands = true to leave items in hands; otherwise, false

	if ( GetCount() <= 0 )
		return;
	CItemContainer * pPack = NULL;

	CItem* pItemNext;
	CItem* pItem = GetContentHead();
	for ( ; pItem != NULL; pItem = pItemNext )
	{
		pItemNext = pItem->GetNext();
		LAYER_TYPE layer = pItem->GetEquipLayer();

		switch ( layer )
		{
			case LAYER_NONE:
				pItem->Delete();	// Get rid of any trades.
				continue;
			case LAYER_FLAG_Poison:
			case LAYER_FLAG_Criminal:
			case LAYER_FLAG_Hallucination:
			case LAYER_FLAG_Potion:
			case LAYER_FLAG_Drunk:
			case LAYER_FLAG_Stuck:
			case LAYER_FLAG_PotionUsed:
				if ( IsStatFlag( STATF_DEAD ))
					pItem->Delete();
				continue;
			case LAYER_PACK:
			case LAYER_HORSE:
				continue;
			case LAYER_HAIR:	// leave this.
			case LAYER_BEARD:
				// Copy hair and beard to corpse.
				if ( pDest == NULL )
					continue;
				if ( pDest->IsType(IT_CORPSE))
				{
					CItem * pDupe = CItem::CreateDupeItem( pItem );
					pDest->ContentAdd( pDupe );	// add content
					// Equip layer only matters on a corpse.
					pDupe->SetContainedLayer( layer );
				}
				continue;
			case LAYER_DRAGGING:
				layer = LAYER_NONE;
				break;
			case LAYER_HAND1:
			case LAYER_HAND2:
				if (bLeaveHands)
					continue;
				break;
			default:
				// can't transfer this to corpse.
				if ( ! CItemBase::IsVisibleLayer( layer ))
					continue;
				break;
		}
		if ( pDest != NULL &&
			! pItem->IsAttr( ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2 ))
		{	// Move item to dest. (corpse ussually)
			pDest->ContentAdd( pItem );
			if ( pDest->IsType(IT_CORPSE))
			{
				// Equip layer only matters on a corpse.
				pItem->SetContainedLayer( layer );
			}
		}
		else
		{	// Move item to chars' pack.
			if ( pPack == NULL )
				pPack = GetPackSafe();
			pPack->ContentAdd( pItem );
		}
	}
}

void CChar::UpdateDrag( CItem * pItem, CObjBase * pCont, CPointMap * ppt )
{
	ADDTOCALLSTACK("CChar::UpdateDrag");
	// Show the world that I am picking up or putting down this object.
	// NOTE: This makes people disapear.

	if (pCont != NULL && pCont->GetTopLevelObj() == this)
		return; // moving to my own backpack
	else if (pCont == NULL && ppt == NULL && pItem->GetTopLevelObj() == this)
		return; // doesn't work for ground objects

	PacketDragAnimation* cmd = new PacketDragAnimation(this, pItem, pCont, ppt);
	UpdateCanSee(cmd, m_pClient);
}


void	CChar::UpdateStatsFlag() const
{
	ADDTOCALLSTACK("CChar::UpdateStatsFlag");
	// Push status change to all who can see us.
	// For Weight, AC, Gold must update all
	// Just flag the stats to be updated later if possible.
	if ( g_Serv.IsLoading() )
		return;

	if ( IsClient() )
		GetClient()->addUpdateStatsFlag();
}

// queue updates

void CChar::UpdateHitsFlag()
{
	ADDTOCALLSTACK("CChar::UpdateHitsFlag");
	if ( g_Serv.IsLoading() )
		return;

	m_fStatusUpdate |= SU_UPDATE_HITS;

	if ( IsClient() )
		GetClient()->addUpdateHitsFlag();
}

void CChar::UpdateModeFlag()
{
	ADDTOCALLSTACK("CChar::UpdateModeFlag");
	if ( g_Serv.IsLoading() )
		return;

	m_fStatusUpdate |= SU_UPDATE_MODE;
}

void CChar::UpdateManaFlag() const
{
	ADDTOCALLSTACK("CChar::UpdateManaFlag");
	if ( g_Serv.IsLoading() )
		return;

	if ( IsClient() )
		GetClient()->addUpdateManaFlag();
}

void CChar::UpdateStamFlag() const
{
	ADDTOCALLSTACK("CChar::UpdateStamFlag");
	if ( g_Serv.IsLoading() )
		return;

	if ( IsClient() )
		GetClient()->addUpdateStamFlag();
}

void CChar::UpdateHitsForOthers() const
{
	ADDTOCALLSTACK("CChar::UpdateHitsForOthers");

	PacketHealthUpdate* cmd = new PacketHealthUpdate(this, false);

	UpdateCanSee(cmd, m_pClient);
}

void CChar::UpdateStatVal( STAT_TYPE type, int iChange, int iLimit )
{
	ADDTOCALLSTACK("CChar::UpdateStatVal");
	int iVal = Stat_GetVal( type );

	if ( iChange )
	{
		if ( ! iLimit )
		{
			iLimit = Stat_GetMax( type );
		}
		if ( iChange < 0 )
		{
			iVal += iChange;
		}
		else if ( iVal > iLimit )
		{
			iVal -= iChange;
			if ( iVal < iLimit ) iVal = iLimit;
		}
		else
		{
			iVal += iChange;
			if ( iVal > iLimit ) iVal = iLimit;
		}
		if ( iVal < 0 ) iVal = 0;
		Stat_SetVal( type, iVal );
	}

	iLimit = Stat_GetMax(type);
	if ( iLimit < 0 )
		iLimit = 0;

	switch ( type )
	{
		case STAT_STR:
			UpdateHitsFlag();
			break;
		case STAT_INT:
			UpdateManaFlag();
			break;
		case STAT_DEX:
			UpdateStamFlag();
		default:
			break;
	}
}



bool CChar::UpdateAnimate( ANIM_TYPE action, bool fTranslate, bool fBackward, BYTE iFrameDelay )
{
	ADDTOCALLSTACK("CChar::UpdateAnimate");
	// NPC or character does a certain Animate
	// Translate the animation based on creature type.
	// ARGS:
	//   fBackward = make the anim go in reverse.
	//   iFrameDelay = in seconds (approx), 0=fastest, 1=slower

	if ( action < 0 || action >= ANIM_QTY )
		return false;
	if ( fBackward && iFrameDelay )	// backwards and delayed just dont work ! = invis
		iFrameDelay = 0;

	if ( fTranslate || IsStatFlag( STATF_OnHorse ))
	{
		CCharBase* pCharDef = Char_GetDef();
		CItem * pWeapon = m_uidWeapon.ItemFind();
		if ( pWeapon != NULL && action == ANIM_ATTACK_WEAPON )
		{
			// action depends on weapon type (skill) and 2 Hand type.
			LAYER_TYPE layer = pWeapon->Item_GetDef()->GetEquipLayer();
			switch ( pWeapon->GetType() )
			{
				case IT_WEAPON_MACE_CROOK:
				case IT_WEAPON_MACE_PICK:
				case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
				case IT_WEAPON_MACE_STAFF:
				case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
					action = ( layer == LAYER_HAND2 ) ?
						ANIM_ATTACK_2H_DOWN :
						ANIM_ATTACK_1H_DOWN;
					break;
				case IT_WEAPON_SWORD:
				case IT_WEAPON_AXE:
					action = ( layer == LAYER_HAND2 ) ?
						ANIM_ATTACK_2H_WIDE :
						ANIM_ATTACK_1H_WIDE;
					break;
				case IT_WEAPON_FENCE:
					action = ( layer == LAYER_HAND2 ) ?
						ANIM_ATTACK_2H_JAB :
						ANIM_ATTACK_1H_JAB;
					break;
				case IT_WEAPON_BOW:
					action = ANIM_ATTACK_BOW;
					break;
				case IT_WEAPON_XBOW:
					action = ANIM_ATTACK_XBOW;
					break;
				default:
					break;
			}
			if (( Calc_GetRandVal( 2 ) ) && ( pWeapon->GetType() != IT_WEAPON_BOW ) && ( pWeapon->GetType() != IT_WEAPON_XBOW ) )
			{
				// add some style to the attacks.
				if ( layer == LAYER_HAND2 )
				{
					action = static_cast<ANIM_TYPE>(ANIM_ATTACK_2H_DOWN + Calc_GetRandVal(3));
				}
				else
				{
					action = static_cast<ANIM_TYPE>(ANIM_ATTACK_1H_WIDE + Calc_GetRandVal(3));
				}
			}
		}

		if ( IsStatFlag( STATF_OnHorse ))	// on horse back.
		{
			// Horse back anims are dif.
			switch ( action )
			{
				case ANIM_WALK_UNARM:
				case ANIM_WALK_ARM:
					action = ANIM_HORSE_RIDE_SLOW;
					break;
				case ANIM_RUN_UNARM:
				case ANIM_RUN_ARMED:
					action = ANIM_HORSE_RIDE_FAST;
					break;
				case ANIM_STAND:
					action = ANIM_HORSE_STAND;
					break;
				case ANIM_FIDGET1:
				case ANIM_FIDGET_YAWN:
					action = ANIM_HORSE_SLAP;
					break;
				case ANIM_STAND_WAR_1H:
				case ANIM_STAND_WAR_2H:
					action = ANIM_HORSE_STAND;
					break;
				case ANIM_ATTACK_1H_WIDE:
				case ANIM_ATTACK_1H_JAB:
				case ANIM_ATTACK_1H_DOWN:
					action = ANIM_HORSE_ATTACK;
					break;
				case ANIM_ATTACK_2H_JAB:
				case ANIM_ATTACK_2H_WIDE:
				case ANIM_ATTACK_2H_DOWN:
					action = ANIM_HORSE_SLAP;
					break;
				case ANIM_WALK_WAR:
					action = ANIM_HORSE_RIDE_SLOW;
					break;
				case ANIM_CAST_DIR:
					action = ANIM_HORSE_ATTACK;
					break;
				case ANIM_CAST_AREA:
					action = ANIM_HORSE_ATTACK_BOW;
					break;
				case ANIM_ATTACK_BOW:
					action = ANIM_HORSE_ATTACK_BOW;
					break;
				case ANIM_ATTACK_XBOW:
					action = ANIM_HORSE_ATTACK_XBOW;
					break;
				case ANIM_GET_HIT:
					action = ANIM_HORSE_SLAP;
					break;
				case ANIM_BLOCK:
					action = ANIM_HORSE_SLAP;
					break;
				case ANIM_ATTACK_UNARM:
					action = ANIM_HORSE_ATTACK;
					break;
				case ANIM_BOW:
				case ANIM_SALUTE:
				case ANIM_EAT:
					action = ANIM_HORSE_ATTACK_XBOW;
					break;
				default:
					action = ANIM_HORSE_STAND;
					break;
			}
		}
		else if (!IsHuman())  //( GetDispID() < CREID_MAN ) Possible fix for anims not being displayed above 400
		{
			// Animals have certain anims. Monsters have others.

			if ( GetDispID() >= CREID_HORSE1 )
			{
				// All animals have all these anims thankfully
				switch ( action )
				{
					case ANIM_WALK_UNARM:
					case ANIM_WALK_ARM:
					case ANIM_WALK_WAR:
						action = ANIM_ANI_WALK;
						break;
					case ANIM_RUN_UNARM:
					case ANIM_RUN_ARMED:
						action = ANIM_ANI_RUN;
						break;
					case ANIM_STAND:
					case ANIM_STAND_WAR_1H:
					case ANIM_STAND_WAR_2H:
					default:
						action = ANIM_ANI_STAND;
						break;

					case ANIM_FIDGET1:
						action = ANIM_ANI_FIDGET1;
						break;
					case ANIM_FIDGET_YAWN:
						action = ANIM_ANI_FIDGET2;
						break;
					case ANIM_CAST_DIR:
						action = ANIM_ANI_ATTACK1;
						break;
					case ANIM_CAST_AREA:
						action = ANIM_ANI_EAT;
						break;
					case ANIM_GET_HIT:
						action = ANIM_ANI_GETHIT;
						break;

					case ANIM_ATTACK_1H_WIDE:
					case ANIM_ATTACK_1H_JAB:
					case ANIM_ATTACK_1H_DOWN:
					case ANIM_ATTACK_2H_DOWN:
					case ANIM_ATTACK_2H_JAB:
					case ANIM_ATTACK_2H_WIDE:
					case ANIM_ATTACK_BOW:
					case ANIM_ATTACK_XBOW:
					case ANIM_ATTACK_UNARM:
						switch ( Calc_GetRandVal(2))
						{
							case 0: action = ANIM_ANI_ATTACK1; break;
							case 1: action = ANIM_ANI_ATTACK2; break;
						}
						break;

					case ANIM_DIE_BACK:
						action = ANIM_ANI_DIE1;
						break;
					case ANIM_DIE_FORWARD:
						action = ANIM_ANI_DIE2;
						break;
					case ANIM_BLOCK:
					case ANIM_BOW:
					case ANIM_SALUTE:
						action = ANIM_ANI_SLEEP;
						break;
					case ANIM_EAT:
						action = ANIM_ANI_EAT;
						break;
				}

				while ( action != ANIM_WALK_UNARM && ! ( pCharDef->m_Anims & (1<<action)))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
						case ANIM_ANI_SLEEP:	// All have this.
							action = ANIM_ANI_EAT;
							break;
						default:
							action = ANIM_WALK_UNARM;
							break;
					}
				}
			}
			else
			{
				// Monsters don't have all the anims.

				switch ( action )
				{
					case ANIM_CAST_DIR:
						action = ANIM_MON_Stomp;
						break;
					case ANIM_CAST_AREA:
						action = ANIM_MON_PILLAGE;
						break;
					case ANIM_DIE_BACK:
						action = ANIM_MON_DIE1;
						break;
					case ANIM_DIE_FORWARD:
						action = ANIM_MON_DIE2;
						break;
					case ANIM_GET_HIT:
						switch ( Calc_GetRandVal(3))
						{
						case 0: action = ANIM_MON_GETHIT; break;
						case 1: action = ANIM_MON_BlockRight; break;
						case 2: action = ANIM_MON_BlockLeft; break;
						}
						break;
					case ANIM_ATTACK_1H_WIDE:
					case ANIM_ATTACK_1H_JAB:
					case ANIM_ATTACK_1H_DOWN:
					case ANIM_ATTACK_2H_DOWN:
					case ANIM_ATTACK_2H_JAB:
					case ANIM_ATTACK_2H_WIDE:
					case ANIM_ATTACK_BOW:
					case ANIM_ATTACK_XBOW:
					case ANIM_ATTACK_UNARM:
						switch ( Calc_GetRandVal(3))
						{
							case 0: action = ANIM_MON_ATTACK1; break;
							case 1: action = ANIM_MON_ATTACK2; break;
							case 2: action = ANIM_MON_ATTACK3; break;
						}
						break;
					default:
						action = ANIM_WALK_UNARM;
						break;
				}
				// NOTE: Available actions depend HEAVILY on creature type !
				// ??? Monsters don't have all anims in common !
				// translate these !
				while ( action != ANIM_WALK_UNARM && ! ( pCharDef->m_Anims & (1<<action)))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
						case ANIM_MON_ATTACK1:	// All have this.
							DEBUG_ERR(( "Anim 0%x This is wrong! Invalid SCP file data.\n", GetDispID()));
							action = ANIM_WALK_UNARM;
							break;

						case ANIM_MON_ATTACK2:	// Dolphins, Eagles don't have this.
						case ANIM_MON_ATTACK3:
							action = ANIM_MON_ATTACK1;	// ALL creatures have at least this attack.
							break;
						case ANIM_MON_Cast2:	// Trolls, Spiders, many others don't have this.
							action = ANIM_MON_BlockRight;	// Birds don't have this !
							break;
						case ANIM_MON_BlockRight:
							action = ANIM_MON_BlockLeft;
							break;
						case ANIM_MON_BlockLeft:
							action = ANIM_MON_GETHIT;
							break;
						case ANIM_MON_GETHIT:
							if ( pCharDef->m_Anims & (1<<ANIM_MON_Cast2))
								action = ANIM_MON_Cast2;
							else
								action = ANIM_WALK_UNARM;
							break;

						case ANIM_MON_Stomp:
							action = ANIM_MON_PILLAGE;
							break;
						case ANIM_MON_PILLAGE:
							action = ANIM_MON_ATTACK3;
							break;
						case ANIM_MON_AttackBow:
						case ANIM_MON_AttackXBow:
							action = ANIM_MON_ATTACK3;
							break;
						case ANIM_MON_AttackThrow:
							action = ANIM_MON_AttackXBow;
							break;

						default:
							DEBUG_ERR(( "Anim Unsupported 0%x for 0%x\n", action, GetDispID()));
							action = ANIM_WALK_UNARM;
							break;
					}
				}
			}
		}
	}

	PacketAction* cmd = new PacketAction(this, action, 1, fBackward, iFrameDelay);
	UpdateCanSee(cmd);
	return( true );
}

void CChar::UpdateMode( CClient * pExcludeClient, bool fFull )
{
	ADDTOCALLSTACK("CChar::UpdateMode");
	// If character status has been changed
	// (Polymorph, war mode or hide), resend him

	// no need to update the mode in the next tick
	if ( pExcludeClient == NULL )
		m_fStatusUpdate &= ~SU_UPDATE_MODE;

	ClientIterator it;
	for (CClient* pClient = it.next(); pClient != NULL; pClient = it.next())
	{
		if ( pExcludeClient == pClient )
			continue;
		if ( pClient->GetChar() == NULL )
			continue;

		if ( ! pClient->CanSee( this ))
		{
			// In the case of "INVIS" used by GM's we must use this.
			if ( GetTopPoint().GetDistSight( pClient->GetChar()->GetTopPoint()) <= UO_MAP_VIEW_SIZE )
			{
				pClient->addObjectRemove( this );
			}
			continue;
		}
// VisRangeCheck
		if ( pClient->GetChar()->GetSight() >= GetTopPoint().GetDistSight( pClient->GetChar()->GetTopPoint()) )
		{
			if ( fFull )
				pClient->addChar(this);
			else
			{
				pClient->addCharMove(this);
				pClient->addHealthBarUpdate(this);
			}
		}
	}
}

void CChar::UpdateSpeedMode()
{
	ADDTOCALLSTACK("CChar::UpdateSpeedMode");
	if ( g_Serv.IsLoading() )
		return;

	if ( !m_pPlayer )
		return;

	if ( IsClient() )
		GetClient()->addSpeedMode( m_pPlayer->m_speedMode );
}

void CChar::UpdateVisualRange()
{
	ADDTOCALLSTACK("CChar::UpdateVisualRange");
	if ( g_Serv.IsLoading() )
		return;

	if ( !m_pPlayer )
		return;

	DEBUG_WARN(("CChar::UpdateVisualRange called, m_iVisualRange is %d\n", m_iVisualRange));

	if ( IsClient() )
		GetClient()->addVisualRange( m_iVisualRange );
}

void CChar::UpdateMove( const CPointMap & pold, CClient * pExcludeClient, bool fFull )
{
	ADDTOCALLSTACK("CChar::UpdateMove");
	// Who now sees this char ?
	// Did they just see him move ?

	// no need to update the mode in the next tick
	if ( pExcludeClient == NULL )
		m_fStatusUpdate &= ~SU_UPDATE_MODE;

	EXC_TRY("UpdateMove");
	EXC_SET("FOR LOOP");
	ClientIterator it;
	for (CClient* pClient = it.next(); pClient != NULL; pClient = it.next())
	{
		if ( pClient == pExcludeClient )
			continue;	// no need to see self move.
		EXC_SET("IF m_pClient");
		if ( pClient == m_pClient && fFull )
		{
			EXC_SET("ADD map");
			// What do i now see ?
			pClient->addMap( (pold.IsValidPoint() ? &pold : NULL), true );
			EXC_SET("AddPlayerView");
			pClient->addPlayerView( pold );
			continue;
		}
		EXC_SET("Char Getting");
		CChar * pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;

		bool fCouldSee = ( pold.GetDistSight( pChar->GetTopPoint()) <= pChar->GetSight() );
		EXC_SET("if cansee");
		if ( ! pClient->CanSee( this ))
		{	// can't see me now.
			if ( fCouldSee )
			{
				EXC_SET("AddObjRem");
				pClient->addObjectRemove( this );
			}
		}
		else if ( fCouldSee )
		{	// They see me move because they can see me now and could see me already
			EXC_SET("AddcharMove");
			pClient->addCharMove( this );
		}
		else
		{	// first time this client has seen me.
			EXC_SET("AddChar");
			pClient->addChar( this );
		}
	}
	EXC_CATCH;
}

void CChar::UpdateDir( DIR_TYPE dir )
{
	ADDTOCALLSTACK("CChar::UpdateDir");
	if ( dir != m_dirFace && dir > DIR_INVALID && dir < DIR_QTY )
	{
		m_dirFace = dir;	// face victim.
		UpdateMove( GetTopPoint(), NULL, true );
	}
}

void CChar::UpdateDir( const CPointMap & pt )
{
	ADDTOCALLSTACK("CChar::UpdateDir");
	// Change in direction.
	UpdateDir( GetTopPoint().GetDir( pt ));
}

void CChar::UpdateDir( const CObjBaseTemplate * pObj )
{
	ADDTOCALLSTACK("CChar::UpdateDir");
	if ( pObj == NULL )
		return;
	pObj = pObj->GetTopLevelObj();
	if ( pObj == this )		// In our own pack.
		return;
	UpdateDir( pObj->GetTopPoint());
}

void CChar::Update( const CClient * pClientExclude ) // If character status has been changed (Polymorph), resend him
{
	ADDTOCALLSTACK("CChar::Update");
	// Or I changed looks.
	// I moved or somebody moved me  ?

	// no need to update the mode in the next tick
	if ( pClientExclude == NULL)
		m_fStatusUpdate &= ~SU_UPDATE_MODE;

	ClientIterator it;
	for (CClient* pClient = it.next(); pClient != NULL; pClient = it.next())
	{
		if ( pClient == pClientExclude )
			continue;
		if ( pClient == m_pClient ) 
		{
			pClient->addReSync();
		}	
		else if ( pClient->CanSee( this ) )
		{
			pClient->addChar( this );
		}
	}
}

void CChar::SoundChar( CRESND_TYPE type )
{
	ADDTOCALLSTACK("CChar::SoundChar");
	if ( !g_Cfg.m_fGenericSounds )
		return;

	SOUND_TYPE id;

	CCharBase* pCharDef = Char_GetDef();
	switch ( GetDispID() )
	{
		case CREID_BLADES:
			id = pCharDef->m_soundbase;
			break;

		case CREID_MAN:
		case CREID_WOMAN:
		case CREID_GHOSTMAN:
		case CREID_GHOSTWOMAN:
		case CREID_ELFMAN:
		case CREID_ELFWOMAN:
		case CREID_ELFGHOSTMAN:
		case CREID_ELFGHOSTWOMAN:
		case CREID_GARGMAN:
		case CREID_GARGWOMAN:
		case CREID_GARGGHOSTMAN:
		case CREID_GARGGHOSTWOMAN:
		{
			id = 0;

			static const SOUND_TYPE sm_Snd_Man_Die[] = { 0x15a, 0x15b, 0x15c, 0x15d };
			static const SOUND_TYPE sm_Snd_Man_Omf[] = { 0x154, 0x155, 0x156, 0x157, 0x158, 0x159 };
			static const SOUND_TYPE sm_Snd_Wom_Die[] = { 0x150, 0x151, 0x152, 0x153 };
			static const SOUND_TYPE sm_Snd_Wom_Omf[] = { 0x14b, 0x14c, 0x14d, 0x14e, 0x14f };

			if ( pCharDef->IsFemale())
			{
				switch ( type )
				{
					case CRESND_GETHIT:
						id = sm_Snd_Wom_Omf[ Calc_GetRandVal( COUNTOF(sm_Snd_Wom_Omf)) ];
						break;
					case CRESND_DIE:
						id = sm_Snd_Wom_Die[ Calc_GetRandVal( COUNTOF(sm_Snd_Wom_Die)) ];
						break;
					default:
						break;
				}
			}
			else
			{
				switch ( type )
				{
					case CRESND_GETHIT:
						id = sm_Snd_Man_Omf[ Calc_GetRandVal( COUNTOF(sm_Snd_Man_Omf)) ];
						break;
					case CRESND_DIE:
						id = sm_Snd_Man_Die[ Calc_GetRandVal( COUNTOF(sm_Snd_Man_Die)) ];
						break;
					default:
						break;
				}
			}
		} break;

		default:
		{
			id = pCharDef->m_soundbase + type;
			switch ( pCharDef->m_soundbase )	// some creatures have no base sounds.
			{
				case 0:
					id = 0;
					break;
				case 128: // old versions
				case 181:
				case 199:
					if ( type <= CRESND_RAND2 )
						id = 0;
					break;
				case 130: // ANIMALS_DEER3
				case 183: // ANIMALS_LLAMA3
				case 201: // ANIMALS_RABBIT3
					if ( type <= CRESND_RAND2 )
						id = 0;
					else
						id -= 2;
					break;
				default:
					break;
			}
		} break;
	}

	if ( type == CRESND_HIT )
	{
		CItem * pWeapon = m_uidWeapon.ItemFind();
		if ( pWeapon != NULL )
		{
			// weapon type strike noise based on type of weapon and how hard hit.
			switch ( pWeapon->GetType() )
			{
				case IT_WEAPON_MACE_CROOK:
				case IT_WEAPON_MACE_PICK:
				case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
				case IT_WEAPON_MACE_STAFF:
					// 0x233 = blunt01 (miss?)
					id = 0x233;
					break;
				case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
					// 0x232 = axe01 swing. (miss?)
					id = 0x232;
					break;
				case IT_WEAPON_SWORD:
				case IT_WEAPON_AXE:
					if ( pWeapon->Item_GetDef()->GetEquipLayer() == LAYER_HAND2 )
					{
						// 0x236 = hvyswrd1 = (heavy strike)
						// 0x237 = hvyswrd4 = (heavy strike)
						id = Calc_GetRandVal( 2 ) ? 0x236 : 0x237;
						break;
					}
				case IT_WEAPON_FENCE:
					// 0x23b = sword1
					// 0x23c = sword7
					id = Calc_GetRandVal( 2 ) ? 0x23b : 0x23c;
					break;
				case IT_WEAPON_BOW:
				case IT_WEAPON_XBOW:
					// 0x234 = xbow ( hit)
					id = 0x234;
					break;
				default:
					break;
			}

			CVarDefCont * pTagStorage = NULL; 
			pTagStorage = pWeapon->GetKey("OVERRIDE.SOUND_HIT", true);
			if ( pTagStorage )
			{
				if ( pTagStorage->GetValNum() )
				{
					id = pTagStorage->GetValNum();
				}
			}

		}
		else if ( id == 0 )
		{
			static const SOUND_TYPE sm_Snd_Hit[] =
			{
				0x135,	//= hit01 = (slap)
				0x137,	//= hit03 = (hit sand)
				0x13b	//= hit07 = (hit slap)
			};
			id = sm_Snd_Hit[ Calc_GetRandVal( COUNTOF( sm_Snd_Hit )) ];
		}
	}

	Sound(id);
}

int CChar::ItemPickup(CItem * pItem, int amount)
{
	ADDTOCALLSTACK("CChar::ItemPickup");
	// Pickup off the ground or remove my own equipment. etc..
	// This item is now "up in the air"
	// RETURN:
	//  amount we can pick up.
	//	-1 = we cannot pick this up.

	if (( amount < 0 ) || !pItem )
		return -1;
	if ( pItem->GetParent() == this && pItem->GetEquipLayer() == LAYER_HORSE )
		return -1;
	if (( pItem->GetParent() == this ) && ( pItem->GetEquipLayer() == LAYER_DRAGGING ))
		return pItem->GetAmount();
	if ( !CanTouch(pItem) || !CanMove(pItem, true) )
		return -1;

	const CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();

	if( IsClient() )
	{
		CClient * client = GetClient();
		const CItem * pItemCont	= dynamic_cast <const CItem*> (pItem->GetParent());

		if ( pItemCont != NULL )
		{
			// Don't allow taking items from the bank unless we opened it here
			if ( pItemCont->IsType( IT_EQ_BANK_BOX ) && ( pItemCont->m_itEqBankBox.m_pntOpen != GetTopPoint() ) )
				return -1;

			// Check sub containers too
			CChar * pCharTop = dynamic_cast<CChar *>(const_cast<CObjBaseTemplate *>(pObjTop));
			if ( pCharTop != NULL )
			{
				bool bItemContIsInsideBankBox = pCharTop->GetBank()->IsItemInside( pItemCont );
				if ( bItemContIsInsideBankBox && ( pCharTop->GetBank()->m_itEqBankBox.m_pntOpen != GetTopPoint() ))
					return -1;
			}

			// protect from ,snoop - disallow picking from not opened containers
			bool isInOpenedContainer = false;
			CClient::OpenedContainerMap_t::iterator itContainerFound = client->m_openedContainers.find( pItemCont->GetUID().GetPrivateUID() );
			if ( itContainerFound != client->m_openedContainers.end() )
			{
				DWORD dwTopContainerUID = (((*itContainerFound).second).first).first;
				DWORD dwTopMostContainerUID = (((*itContainerFound).second).first).second;
				CPointMap ptOpenedContainerPosition = ((*itContainerFound).second).second;

				DWORD dwTopContainerUID_ToCheck = 0;
				if ( pItemCont->GetContainer() )
					dwTopContainerUID_ToCheck = pItemCont->GetContainer()->GetUID().GetPrivateUID();
				else
					dwTopContainerUID_ToCheck = pObjTop->GetUID().GetPrivateUID();

				if ( ( dwTopMostContainerUID == pObjTop->GetUID().GetPrivateUID() ) && ( dwTopContainerUID == dwTopContainerUID_ToCheck ) )
				{
					if ( pCharTop != NULL )
					{
						isInOpenedContainer = true;
						// probably a pickup check from pack if pCharTop != this?
					}
					else
					{
						CItem * pItemTop = dynamic_cast<CItem *>(const_cast<CObjBaseTemplate *>(pObjTop));
						if ( pItemTop && (pItemTop->IsType(IT_SHIP_HOLD) || pItemTop->IsType(IT_SHIP_HOLD_LOCK)) && (pItemTop->GetTopPoint().GetRegion(REGION_TYPE_MULTI) == GetTopPoint().GetRegion(REGION_TYPE_MULTI)) )
						{
							isInOpenedContainer = true;
						}
						else if ( ptOpenedContainerPosition.GetDist( pObjTop->GetTopPoint() ) <= 3 )
						{
							isInOpenedContainer = true;
						}
					}
				}
			}

			if( !isInOpenedContainer )
				return -1;
		}
	}

	const CChar * pChar = dynamic_cast <const CChar*> (pObjTop);

	if ( pChar != this &&
		pItem->IsAttr(ATTR_OWNED) &&
		pItem->m_uidLink != GetUID() &&
		!IsPriv(PRIV_ALLMOVE|PRIV_GM))
	{
		SysMessageDefault(DEFMSG_STEAL);
		return -1;
	}

	const CItemCorpse * pCorpseItem = dynamic_cast <const CItemCorpse *>(pObjTop);
	if ( pCorpseItem )
	{
		// Taking stuff off someones corpse can be a crime !
		if ( CheckCorpseCrime(pCorpseItem, true, false) )
			SysMessageDefault(DEFMSG_GUARDS);
	}

	int iAmountMax = pItem->GetAmount();
	if ( iAmountMax <= 0 )
		return -1;

	if ( !pItem->Item_GetDef()->IsStackableType() )
		amount = iAmountMax;	// it's not stackable, so we must pick up the entire amount
	else
		amount = maximum(1, minimum(amount, iAmountMax));

	int iItemWeight = ( amount == iAmountMax ) ? pItem->GetWeight() : pItem->Item_GetDef()->GetWeight() * amount;

	// Is it too heavy to even drag ?
	bool fDrop = false;
	if ( GetWeightLoadPercent(GetTotalWeight() + iItemWeight) > 300 )
	{
		SysMessageDefault(DEFMSG_HEAVY);
		if (( pChar == this ) && ( pItem->GetParent() == GetPack() ))
		{
			fDrop = true;	// we can always drop it out of own pack !
		}
		return -1;
	}

	ITRIG_TYPE trigger;
	if ( pChar != NULL )
	{
		bool bCanTake = false;
		if (pChar == this) // we can always take our own items
			bCanTake = true;
		else if (pItem->GetContainer() != pChar || g_Cfg.m_fCanUndressPets == true) // our owners can take items from us (with CanUndressPets=true, they can undress us too)
			bCanTake = pChar->NPC_IsOwnedBy(this);
		else  // higher priv players can take items and undress us
			bCanTake = IsPriv(PRIV_GM) && GetPrivLevel() > pChar->GetPrivLevel();

		if (bCanTake == false)
		{
			SysMessageDefault(DEFMSG_STEAL);
			return -1;
		}
		trigger = pItem->IsItemEquipped() ? ITRIG_UNEQUIP : ITRIG_PICKUP_PACK;
	}
	else
	{
		trigger = pItem->IsTopLevel() ? ITRIG_PICKUP_GROUND : ITRIG_PICKUP_PACK;
	}

	if ( trigger == ITRIG_PICKUP_GROUND )
	{
		//	bug with taking static/movenever items -or- catching the spell effects
		if ( IsPriv(PRIV_ALLMOVE|PRIV_GM) ) ;
		else if ( pItem->IsAttr(ATTR_STATIC|ATTR_MOVE_NEVER) || pItem->IsType(IT_SPELL) )
			return -1;
	}

	if ( trigger != ITRIG_UNEQUIP )	// unequip is done later.
	{
		CScriptTriggerArgs Args( amount );
		if ( pItem->OnTrigger( trigger, this, &Args ) == TRIGRET_RET_TRUE )
			return( -1 );
		if ( trigger == ITRIG_PICKUP_PACK )
		{
			CItem * pContItem = dynamic_cast <CItem*> ( pItem->GetContainer() );
			if ( pContItem )
			{
				CScriptTriggerArgs Args1(pItem);
				if ( pContItem->OnTrigger(ITRIG_PICKUP_SELF, this, &Args1) == TRIGRET_RET_TRUE )
					return -1;
			}
		}
	}


	if ( pItem->Item_GetDef()->IsStackableType() && amount )
	{
		// Did we only pick up part of it ?
		// part or all of a pile. Only if pilable !
		if ( amount < iAmountMax )
		{
			// create left over item.
			CItem * pItemNew = pItem->UnStackSplit(amount, this);
			pItemNew->SetTimeout( pItem->GetTimerDAdjusted() ); //since this was commented in DupeCopy
			CScriptTriggerArgs Args2(pItemNew);
			if ( pItem->OnTrigger(ITRIG_PICKUP_STACK, this, &Args2) == TRIGRET_RET_TRUE )
				return -1;

		}
	}

	if ( fDrop )
	{
		ItemDrop(pItem, GetTopPoint());
		return -1;
	}

	// do the dragging anim for everyone else to see.
	UpdateDrag(pItem);

	// Remove the item from other clients view if the item is
	// being taken from the ground by a hidden character to
	// prevent lingering item.
	if ( ( trigger == ITRIG_PICKUP_GROUND ) && (IsStatFlag( STATF_Insubstantial | STATF_Invisible | STATF_Hidden )) )
	{
        pItem->RemoveFromView( m_pClient );
	}

	// Pick it up.
	pItem->SetDecayTime(-1);	// Kill any decay timer.
	LayerAdd( pItem, LAYER_DRAGGING );

	return amount;
}

bool CChar::ItemBounce( CItem * pItem )
{
	ADDTOCALLSTACK("CChar::ItemBounce");
	// We can't put this where we want to
	// So put in my pack if i can. else drop.
	// don't check where this came from !


	if ( pItem == NULL )
		return false;

	CItemContainer * pPack = GetPackSafe();
	if ( pItem->GetParent() == pPack )
		return( true );

	LPCTSTR pszWhere = NULL;
	if ( CanCarry( pItem ) && ( pPack ))// this can happen at load time.
	{
		// if we can carry it
		pszWhere = g_Cfg.GetDefaultMsg( DEFMSG_BOUNCE_PACK );

		pPack->ContentAdd( pItem ); // Add it to pack
		Sound( pItem->GetDropSound( pPack ));
	}
	else
	{
		if ( ! GetTopPoint().IsValidPoint())
		{
			// NPC is being created and has no valid point yet.
			if (pszWhere)
			{
				DEBUG_ERR(( "No pack to place loot item '%s' for NPC '%s'\n", pItem->GetResourceName(), GetResourceName()));
			}
			else
			{
				DEBUG_ERR(( "Loot item %s too heavy for NPC %s\n", pItem->GetResourceName(), GetResourceName()));
			}
			pItem->Delete();
			return false;
		}
		pszWhere = g_Cfg.GetDefaultMsg( DEFMSG_FEET );
		ItemDrop( pItem, GetTopPoint());
	}

	SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_ITEMPLACE ), pItem->GetName(), pszWhere );
	return( true );
}

bool CChar::ItemDrop( CItem * pItem, const CPointMap & pt )
{
	ADDTOCALLSTACK("CChar::ItemDrop");
	// A char actively drops an item on the ground.
	if ( pItem == NULL )
		return( false );

	CItemBase * pItemDef = pItem->Item_GetDef();
	if (( g_Cfg.m_fFlipDroppedItems || pItemDef->Can(CAN_I_FLIP)) &&
		pItem->IsMovableType() &&
		! pItemDef->IsStackableType())
	{
		// Does this item have a flipped version.
		pItem->SetDispID( pItemDef->GetNextFlipID( pItem->GetDispID()));
	}

	return( pItem->MoveToCheck( pt, this ));
}

bool CChar::ItemEquip( CItem * pItem, CChar * pCharMsg )
{
	ADDTOCALLSTACK("CChar::ItemEquip");
	// Equip visible stuff. else throw into our pack.
	// Pay no attention to where this came from.
	// Bounce anything in the slot we want to go to. (if possible)
	// NOTE: This can be used from scripts as well to equip memories etc.
	// ASSUME this is ok for me to use. (movable etc)

	if ( !pItem )
		return false;

	// In theory someone else could be dressing me ?
	if ( !pCharMsg )
		pCharMsg = this;

	if ( pItem->GetParent() == this )
	{
		if ( pItem->GetEquipLayer() != LAYER_DRAGGING ) // already equipped.
			return true;
	}

	if ( !IsSetEF(EF_Minimize_Triggers) )
	{
		int iRet = pItem->OnTrigger(ITRIG_EQUIPTEST, this);

		if ( pItem->IsDeleted() )
			return false;

		if ( iRet == TRIGRET_RET_TRUE )
		{
			if ( pItem->GetEquipLayer() == LAYER_DRAGGING ) // dragging? else just do nothing
			{
				pItem->RemoveSelf();
				ItemBounce(pItem);
			}
			return false;
		}
	}

	// strong enough to equip this . etc ?
	// Move stuff already equipped.
   	if ( pItem->GetAmount() > 1 )
		pItem->UnStackSplit(1, this);
	// remove it from the container so that nothing will be stacked with it if unequipped
	pItem->RemoveSelf();

	LAYER_TYPE layer = CanEquipLayer( pItem, LAYER_QTY, pCharMsg, false );

	if ( layer == LAYER_NONE )
	{
		ItemBounce(pItem);
		return false;
	}

	pItem->SetDecayTime(-1);	// Kill any decay timer.
	LayerAdd(pItem, layer);
	if ( !pItem->IsItemEquipped() )	// Equip failed ? (cursed?) Did it just go into pack ?
		return false;

	if ( pItem->OnTrigger(ITRIG_EQUIP, this) == TRIGRET_RET_TRUE )
	{
		return false;
	}

	if ( !pItem->IsItemEquipped() )	// Equip failed ? (cursed?) Did it just go into pack ?
		return false;

	Spell_Effect_Add(pItem);	// if it has a magic effect.

	if ( CItemBase::IsVisibleLayer(layer) )	// visible layer ?
		Sound(0x057);

	return true;
}

void CChar::EatAnim( LPCTSTR pszName, int iQty )
{
	ADDTOCALLSTACK("CChar::EatAnim");
	Stat_SetVal( STAT_FOOD, Stat_GetVal(STAT_FOOD) + iQty );

	static const SOUND_TYPE sm_EatSounds[] = { 0x03a, 0x03b, 0x03c };

	Sound( sm_EatSounds[ Calc_GetRandVal( COUNTOF(sm_EatSounds)) ] );
	UpdateAnimate( ANIM_EAT );

	TCHAR * pszMsg = Str_GetTemp();
	sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_EATSOME), static_cast<LPCTSTR>(pszName));
	Emote(pszMsg);
}

bool CChar::Reveal( DWORD dwFlags )
{
	ADDTOCALLSTACK("CChar::Reveal");
	// Some outside influence may be revealing us.

	if ( !IsStatFlag(dwFlags) )
		return false;

	if ( IsClient() && GetClient()->m_pHouseDesign )
	{
		// no reveal whilst in house design (unless they somehow got out)
		if ( GetClient()->m_pHouseDesign->GetDesignArea().IsInside2d(GetTopPoint()) )
			return false;

		GetClient()->m_pHouseDesign->EndCustomize(true);
	}

	if (( dwFlags & STATF_Sleeping ) && IsStatFlag( STATF_Sleeping ))
	{
		// Try to wake me up.
		Wake();
	}
	/*bool fInvis = false;
	if (( dwFlags & STATF_Invisible ) && IsStatFlag( STATF_Invisible  ))
	{ 
		fInvis = true;
		//SetHue( m_prev_Hue ); <- don't want to reset to oskin!
	}*/

	StatFlag_Clear(dwFlags);
	if ( IsStatFlag(STATF_Invisible|STATF_Hidden|STATF_Insubstantial|STATF_Sleeping))
		return false;

	/*if ( fInvis )
	{
		RemoveFromView();
		Update();
	}
	else*/
		UpdateMode(NULL, true);

	if ( IsClient() )
	{
		GetClient()->removeBuff( BI_HIDDEN );
	}
	SysMessageDefault(DEFMSG_HIDING_REVEALED);
	return true;
}

void CChar::SpeakUTF8( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang )
{
	ADDTOCALLSTACK("CChar::SpeakUTF8");
	// Ignore the font argument here !

	if ( IsStatFlag(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel && g_Cfg.m_iDistanceYell > 0 )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::SpeakUTF8( pszText, wHue, mode, font, lang );
}
void CChar::SpeakUTF8Ex( const NWORD * pszText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang )
{
	ADDTOCALLSTACK("CChar::SpeakUTF8Ex");
	// Ignore the font argument here !

	if ( IsStatFlag(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel && g_Cfg.m_iDistanceYell > 0 )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	if ( m_pNPC )
	{
		wHue = m_pNPC->m_SpeechHue;
	}
	CObjBase::SpeakUTF8Ex( pszText, wHue, mode, font, lang );
}
void CChar::Speak( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font )
{
	ADDTOCALLSTACK("CChar::Speak");
	// Speak to all clients in the area.
	// Ignore the font argument here !

	if ( IsStatFlag(STATF_Stone))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel && g_Cfg.m_iDistanceYell > 0 )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	if ( m_pNPC )
	{
		wHue = m_pNPC->m_SpeechHue;
	}
	CObjBase::Speak( pszText, wHue, mode, font );
}

CItem * CChar::Make_Figurine( CGrayUID uidOwner, ITEMID_TYPE id )
{
	ADDTOCALLSTACK("CChar::Make_Figurine");
	// Make me into a figurine
	if ( IsDisconnected() || m_pPlayer )
		return NULL;

	CCharBase* pCharDef = Char_GetDef();

	// turn creature into a figurine.
	CItem * pItem = CItem::CreateScript( ( id == ITEMID_NOTHING ) ? pCharDef->m_trackID : id, this );
	if ( !pItem )
		return NULL;

	pItem->SetType(IT_FIGURINE);
	pItem->SetName(GetName());
	pItem->SetHue(GetHue());
	pItem->m_itFigurine.m_ID = GetID();	// Base type of creature.
	pItem->m_itFigurine.m_UID = GetUID();
	pItem->m_uidLink = uidOwner;

	if ( IsStatFlag(STATF_Insubstantial) )
		pItem->SetAttr(ATTR_INVIS);

	SoundChar(CRESND_RAND1);	// Horse winny
	StatFlag_Set(STATF_Ridden);
	Skill_Start(NPCACT_RIDDEN);
	SetDisconnected();
	m_atRidden.m_FigurineUID = pItem->GetUID();

	return pItem;
}

CItem * CChar::NPC_Shrink()
{
	ADDTOCALLSTACK("CChar::NPC_Shrink");
	// This will just kill conjured creatures.
	if ( IsStatFlag(STATF_Conjured) )
	{
		Stat_SetVal(STAT_STR, 0);
		return NULL;
	}

	CItem * pItem = Make_Figurine(UID_CLEAR, ITEMID_NOTHING);
	if ( !pItem )
		return NULL;

	pItem->SetAttr(ATTR_MAGIC);
	pItem->MoveToCheck(GetTopPoint());
	return pItem;
}

CItem * CChar::Horse_GetMountItem() const
{
	ADDTOCALLSTACK("CChar::Horse_GetMountItem");
	// I am a horse.
	// Get my mount object. (attached to my rider)

	if ( ! IsStatFlag( STATF_Ridden ))
		return( NULL );

	CItem * pItem = m_atRidden.m_FigurineUID.ItemFind();

	if ( pItem == NULL )
	{
		CItemMemory* pItemMem = Memory_FindTypes( MEMORY_IPET );

		if ( pItemMem != NULL )
		{
			CChar* pOwner = pItemMem->m_uidLink.CharFind();

			if ( pOwner != NULL )
			{
				CItem* pItemMount = pOwner->LayerFind(LAYER_HORSE);

				if ( pItemMount != NULL && pItemMount->m_itNormal.m_more2 == GetUID() )
				{
					const_cast<CGrayUIDBase&>(m_atRidden.m_FigurineUID) = pItemMount->GetUID();
					pItem = pItemMount;

					DEBUG_ERR(("UID=0%lx, id=0%x '%s', Fixed mount item UID=0%lx, id=0%x '%s'\n",
						(DWORD)GetUID(), GetBaseID(), GetName(), (DWORD)(pItem->GetUID()), pItem->GetBaseID(), pItem->GetName()));
				}
			}
		}
	}

	if ( pItem == NULL ||
		( ! pItem->IsType( IT_FIGURINE ) && ! pItem->IsType( IT_EQ_HORSE )))
	{
		return( NULL );
	}
	return( pItem );
}

CChar * CChar::Horse_GetMountChar() const
{
	ADDTOCALLSTACK("CChar::Horse_GetMountChar");
	CItem * pItem = Horse_GetMountItem();
	if ( pItem == NULL )
		return( NULL );
	return( dynamic_cast <CChar*>( pItem->GetTopLevelObj()));
}

bool CChar::Horse_Mount(CChar *pHorse) // Remove horse char and give player a horse item
{
	ADDTOCALLSTACK("CChar::Horse_Mount");
	// RETURN:
	//  true = done mounting so take no more action.
	//  false = we can't mount this so do something else.

	if ( !CanTouch(pHorse) )
	{
		SysMessageDefault(DEFMSG_MOUNT_DIST);
		return false;
	}

	ITEMID_TYPE id;
	TCHAR * sMountDefname = Str_GetTemp();
	sprintf(sMountDefname, "mount_0x%x", pHorse->GetDispID());
	id = static_cast<ITEMID_TYPE>(g_Exp.m_VarDefs.GetKeyNum(sMountDefname));
	if ( id <= ITEMID_NOTHING )
		return false;

	if ( !IsMountCapable() )
	{
		SysMessageDefault(DEFMSG_MOUNT_UNABLE);
		return false;
	}
	if ( pHorse->m_pPlayer || !pHorse->NPC_IsOwnedBy(this) )
	{
		SysMessageDefault(DEFMSG_MOUNT_DONTOWN);
		return false;
	}

	if ( g_Cfg.m_iMountHeight )
	{
		//is there space for me+mounted horse?
		if (! IsVerticalSpace( GetTopPoint(), true ) )
		{
			SysMessageDefault( DEFMSG_MOUNT_CEILING );
			return false;
		}
	}

	CScriptTriggerArgs Args(pHorse);
   	if ( OnTrigger(CTRIG_Mount, this, &Args) == TRIGRET_RET_TRUE )
		return false;

	Horse_UnMount();	// unmount if already on a horse.

	CItem * pItem = pHorse->Make_Figurine(GetUID(), id);
	if ( !pItem )
		return false;

	pItem->SetType(IT_EQ_HORSE);
	pItem->SetTimeout(TICK_PER_SEC);// The first time we give it immediately a tick, then give the horse a tick everyone once in a while.
	LayerAdd(pItem, LAYER_HORSE);	// equip the horse item

	Update();

	return true;
}

bool CChar::Horse_UnMount() // Get off a horse (Remove horse item and spawn new horse)
{
	ADDTOCALLSTACK("CChar::Horse_UnMount");
	if ( ! IsStatFlag( STATF_OnHorse ))
		return( false );

	CItem * pItem = LayerFind( LAYER_HORSE );
	if ( pItem == NULL || pItem->IsDeleted() )
	{
		StatFlag_Clear( STATF_OnHorse );	// flag got out of sync !
		return( false );
	}

	CChar * pPet = pItem->m_itFigurine.m_UID.CharFind();
	if (pPet != NULL && pPet->IsDisconnected() && !pPet->IsDeleted()) // valid horse for trigger
	{
		CScriptTriggerArgs Args(pPet);
   		if ( OnTrigger(CTRIG_Dismount, this, &Args) == TRIGRET_RET_TRUE )
			return ( false );
	}

	// What creature is the horse item ?
	CChar * pHorse = Use_Figurine( pItem, 0 );
	pItem->Delete();

	if ( ( pHorse->Stat_GetVal( STAT_STR ) <= 0 ) || ( pHorse->IsStatFlag( STATF_DEAD ) ) )
	{
		// Horse is dead!
		pHorse->StatFlag_Clear( STATF_DEAD );
		pHorse->Stat_SetVal( STAT_STR, pHorse->Stat_GetAdjusted( STAT_STR ) );
		pHorse->Death();
	}

	return( true );
}

bool CChar::OnTickEquip( CItem * pItem )
{
	ADDTOCALLSTACK("CChar::OnTickEquip");
	// A timer expired for an item we are carrying.
	// Does it periodically do something ?
	// REUTRN:
	//  false = delete it.

	switch ( pItem->GetEquipLayer())
	{
		case LAYER_FLAG_Wool:
			// This will regen the sheep it was sheered from.
			// Sheared sheep regen wool on a new day.
			if ( GetID() != CREID_Sheep_Sheered )
				return false;

			// Is it a new day ? regen my wool.
			SetID( CREID_Sheep );
			return false;

		case LAYER_FLAG_ClientLinger:
			// remove me from other clients screens.
			SetDisconnected();
			return( false );

		case LAYER_SPECIAL:
			switch ( pItem->GetType())
			{
				case IT_EQ_SCRIPT:	// pure script.
					break;
				case IT_EQ_MEMORY_OBJ:
				{
					CItemMemory *pMemory = dynamic_cast<CItemMemory*>( pItem );
					if (pMemory)
						return Memory_OnTick(pMemory);

					return false;
				}
				default:
					break;
			}
			break;

		case LAYER_FLAG_Stuck:
			// Only allow me to try to damage the web so often
			// Non-magical. held by something.
			// IT_EQ_STUCK
			pItem->SetTimeout( -1 );
			return( true );

		case LAYER_HORSE:
			// Give my horse a tick. (It is still in the game !)
			// NOTE: What if my horse dies (poisoned?)
			{
				// NPCs with just an item equipped is fine
				// but still give ticks just in case
				if ( m_pNPC )
				{
					pItem->SetTimeout( 10 * TICK_PER_SEC );
					return( true );
				}

				CChar * pHorse = pItem->m_itFigurine.m_UID.CharFind();
				if ( pHorse == NULL )
					return( false );

				if ( ( pHorse->Stat_GetVal(STAT_STR) <= 0 ) || ( pHorse->IsStatFlag( STATF_DEAD ) ) )
				{
					DEBUG_ERR(( "Character %s (0%lx) riding dead horse (0%lx) - forcing death on horse\n", GetName(), (DWORD)GetUID(), (DWORD)pHorse->GetUID() ));
					Horse_UnMount();
					pHorse->Delete();
					return( false );
				}

				int iTicks = 10 * TICK_PER_SEC;
				for ( STAT_TYPE i = STAT_STR; i <= STAT_FOOD; i = static_cast<STAT_TYPE>(i + 1) )
				{
					int iRegen = g_Cfg.m_iRegenRate[i];
					if ( i < STAT_FOOD )
					{
						char sRegen[21];
						sprintf(sRegen, "OVERRIDE.REGEN_%d", static_cast<int>(i));
						iRegen -= ( pHorse->GetKeyNum(sRegen, true) * 10 );
					}
					iTicks = minimum(iTicks,iRegen);
				}

				pItem->SetTimeout( iTicks );

				if ( pHorse->Fight_IsActive() )
					pHorse->Fight_ClearAll();

				if ( pHorse->Skill_GetActive() != NPCACT_RIDDEN )
					pHorse->Skill_Start( NPCACT_RIDDEN );

				bool bHorseTick = pHorse->OnTick();
				if ( !bHorseTick )
					pHorse->Delete();

				return( bHorseTick );
			}

		case LAYER_FLAG_Murders:
			// decay the murder count.
			{
				if ( ! m_pPlayer || m_pPlayer->m_wMurders <= 0  )
					return( false );

				CScriptTriggerArgs	args;
				args.m_iN1 = m_pPlayer->m_wMurders-1;
				args.m_iN2 = g_Cfg.m_iMurderDecayTime;
				if ( !IsSetEF(EF_Minimize_Triggers) )
				{
					OnTrigger(CTRIG_MurderDecay, this, &args);
					if ( args.m_iN1 < 0 ) args.m_iN1 = 0;
					if ( args.m_iN2 < 1 ) args.m_iN2 = g_Cfg.m_iMurderDecayTime;
				}
				m_pPlayer->m_wMurders = args.m_iN1;
				if ( m_pPlayer->m_wMurders == 0 ) return( false );
				pItem->SetTimeout(args.m_iN2);	// update it's decay time.
				return( true );
			}

		default:
			break;
	}

	if ( pItem->IsType( IT_SPELL ))
	{
		return Spell_Equip_OnTick(pItem);
	}

	return( pItem->OnTick());
}

bool CChar::SetPoisonCure( int iSkill, bool fExtra )
{
	ADDTOCALLSTACK("CChar::SetPoisonCure");
	UNREFERENCED_PARAMETER(iSkill);
	// Leave the antidote in your body for a while.
	// iSkill = 0-1000

	CItem * pPoison = LayerFind( LAYER_FLAG_Poison );
	if ( pPoison != NULL )
	{
		// Is it successful ???
		pPoison->Delete();
	}
	if ( fExtra )
	{
		pPoison = LayerFind( LAYER_FLAG_Hallucination );
		if ( pPoison != NULL )
		{
			// Is it successful ???
			pPoison->Delete();
		}
	}
	UpdateModeFlag();
	return( true );
}

bool CChar::SetPoison( int iSkill, int iTicks, CChar * pCharSrc )
{
	ADDTOCALLSTACK("CChar::SetPoison");
	// SPELL_Poison
	// iSkill = 0-1000 = how bad the poison is
	// iTicks = how long to last.
	// Physical attack of poisoning.

	if ( IsStatFlag( STATF_Conjured ))
	{
		// conjured creatures cannot be poisoned.
		return false;
	}

	CItem * pPoison;
	if ( IsStatFlag( STATF_Poisoned ))
	{
		// strengthen the poison ?
		pPoison = LayerFind( LAYER_FLAG_Poison );
		if ( pPoison)
		{
			pPoison->m_itSpell.m_spellcharges += iTicks;
		}
		return false;
	}

	SysMessage( g_Cfg.GetDefaultMsg( DEFMSG_JUST_BEEN_POISONED ) );

	// Release if paralyzed ?
	StatFlag_Clear( STATF_Freeze );	// remove paralyze.

	// Might be a physical vs. Magical attack.
	pPoison = Spell_Effect_Create( SPELL_Poison, LAYER_FLAG_Poison, iSkill, (1+Calc_GetRandVal(2))*TICK_PER_SEC, pCharSrc );
	pPoison->m_itSpell.m_spellcharges = iTicks;	// how long to last.
	UpdateStatsFlag();
	return( true );
}

void CChar::Wake()
{
	ADDTOCALLSTACK("CChar::Wake");
	if ( ! IsStatFlag( STATF_Sleeping ))
		return;
	CItemCorpse * pCorpse = FindMyCorpse();
	if ( pCorpse != NULL )
	{
		RaiseCorpse(pCorpse);
		StatFlag_Clear( STATF_Sleeping );
		Update();	// update light levels etc.
	}
	else
	{
		Stat_SetVal( STAT_STR, 0 );		// Death
	}
}

void CChar::SleepStart( bool fFrontFall )
{
	ADDTOCALLSTACK("CChar::SleepStart");
	if ( IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Polymorph ))
		return;

	StatFlag_Set( STATF_Sleeping );
	if ( ! MakeCorpse( fFrontFall ))
	{
		SysMessageDefault( DEFMSG_CANTSLEEP );
		return;
	}

	SetID( m_prev_id );
	StatFlag_Clear( STATF_Hidden );

	Update();
}

bool CChar::MakeCorpse_Fail()
{
	ADDTOCALLSTACK("CChar::MakeCorpse_Fail");
	// Some creatures can never sleep. (not corpse)
	if ( ! IsStatFlag( STATF_DEAD ))
		return( false );
	if ( m_pPlayer )
	{
		StatFlag_Clear( STATF_Conjured );
		Horse_UnMount();
	}
	if ( IsHuman())
		return( false );	// conjured humans just disapear.
	if ( !(m_TagDefs.GetKeyNum("DEATHFLAGS", true) & DEATH_NOCONJUREDEFFECT) )
	{
		CItem * pItem = CItem::CreateScript(ITEMID_FX_SPELL_FAIL, this);
		if ( pItem )
			pItem->MoveToDecay(GetTopPoint(), 2*TICK_PER_SEC);
	}
	return true;
}

CItemCorpse * CChar::MakeCorpse( bool fFrontFall )
{
	ADDTOCALLSTACK("CChar::MakeCorpse");
	// some creatures (Elems) have no corpses.
	// IsStatFlag( STATF_DEAD ) might NOT be set. (sleeping)

	bool fLoot = ! IsStatFlag( STATF_Conjured );
	WORD wFlags = m_TagDefs.GetKeyNum("DEATHFLAGS", true);

	int iDecayTime = -1;	// never default.
	CItemCorpse * pCorpse = NULL;

	if ( !(wFlags & DEATH_NOCORPSE) && fLoot &&
		(( GetDispID() != CREID_WATER_ELEM &&
		  GetDispID() != CREID_AIR_ELEM &&
		  GetDispID() != CREID_FIRE_ELEM &&
		  GetDispID() != CREID_VORTEX &&
		  GetDispID() != CREID_BLADES ) || (wFlags & DEATH_HASCORPSE)) )
	{
		if ( m_pPlayer )
		{
			Horse_UnMount(); // If i'm conjured then my horse goes with me.
		}

		CItem* pItemCorpse = CItem::CreateScript( ITEMID_CORPSE, this );
		pCorpse = dynamic_cast <CItemCorpse *>(pItemCorpse);
		if ( pCorpse == NULL )	// Weird internal error !
		{
			pItemCorpse->Delete();
			if ( ! MakeCorpse_Fail() )
				return( NULL );
		}

		TCHAR *pszMsg = Str_GetTemp();
		sprintf(pszMsg, g_Cfg.GetDefaultMsg( DEFMSG_CORPSE_OF ), static_cast<LPCTSTR>(GetName()));
		pCorpse->SetName(pszMsg);
		pCorpse->SetHue( GetHue());
		pCorpse->SetCorpseType( GetDispID() );
		pCorpse->m_itCorpse.m_BaseID = m_prev_id;	// id the corpse type here !
		pCorpse->m_itCorpse.m_facing_dir = m_dirFace;
		pCorpse->SetAttr(ATTR_INVIS);	// Don't display til ready.

		if ( IsStatFlag( STATF_DEAD ))
		{
			pCorpse->m_itCorpse.m_timeDeath = CServTime::GetCurrentTime();	// death time.
			pCorpse->m_itCorpse.m_uidKiller = m_Act_Targ;
			iDecayTime = (m_pPlayer) ?
				g_Cfg.m_iDecay_CorpsePlayer : g_Cfg.m_iDecay_CorpseNPC;
		}
		else	// Sleeping
		{
			pCorpse->m_itCorpse.m_timeDeath.Init();	// Not dead.
			pCorpse->m_itCorpse.m_uidKiller = GetUID();
			iDecayTime = -1;	// never
		}

		if ( m_pPlayer )	// not being deleted.
			pCorpse->m_uidLink = GetUID();
	}
	else
	{
		// Some creatures can never sleep. (not corpse)
		if ( ! MakeCorpse_Fail() )
			return( NULL );
	}

	// can fall forward.
	DIR_TYPE dir = m_dirFace;
	if ( fFrontFall )
	{
		dir = static_cast<DIR_TYPE>( dir | 0x80 );
		if ( pCorpse )
			pCorpse->m_itCorpse.m_facing_dir = dir;
	}

	// Death anim. default is to fall backwards. lie face up.
	UpdateCanSee(new PacketDeath(this, pCorpse), m_pClient);

	// Move non-newbie contents of the pack to corpse. (before it is displayed)
	if ( fLoot && !(wFlags & DEATH_NOLOOTDROP) && !(wFlags & DEATH_NOCORPSE) )
	{
		DropAll( pCorpse );
	}
	if ( pCorpse )
	{
		pCorpse->ClrAttr(ATTR_INVIS);	// make visible.
		pCorpse->MoveToDecay(GetTopPoint(), iDecayTime);
	}

	return( pCorpse );
}

bool CChar::RaiseCorpse( CItemCorpse * pCorpse )
{
	ADDTOCALLSTACK("CChar::RaiseCorpse");
	// We are creating a char from the current char and the corpse.
	// Move the items from the corpse back onto us.

	// If NPC is disconnected then reconnect them.
	// If the player is off line then don't allow this !!!

	if ( !pCorpse )
		return false;

	if ( pCorpse->GetCount() > 0 )
	{
		CItemContainer * pPack = GetPackSafe();

		CItem* pItemNext;
		for ( CItem * pItem = pCorpse->GetContentHead(); pItem != NULL; pItem = pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( pItem->IsType( IT_HAIR ) ||
				pItem->IsType( IT_BEARD ) ||
				pItem->IsAttr( ATTR_MOVE_NEVER ))
				continue;	// Hair on corpse was copied!
			// Redress if equipped.
			if ( pItem->GetContainedLayer())
				ItemEquip( pItem );	// Equip the item.
			else
				pPack->ContentAdd( pItem );	// Toss into pack.
		}

		// Any items left just dump on the ground.
		pCorpse->ContentsDump( GetTopPoint());
	}

	if ( pCorpse->IsTopLevel() || pCorpse->IsItemInContainer())
	{
		// I should move to where my corpse is just in case.
		m_fClimbUpdated = false; // update climb height
		MoveToChar( pCorpse->GetTopLevelObj()->GetTopPoint());
	}

	// Corpse is now gone. 	// 0x80 = on face.
	Update();
	UpdateDir(static_cast<DIR_TYPE>( pCorpse->m_itCorpse.m_facing_dir &~ 0x80 ));
	UpdateAnimate( ( pCorpse->m_itCorpse.m_facing_dir & 0x80 ) ? ANIM_DIE_FORWARD : ANIM_DIE_BACK, true, true, 2 );

	pCorpse->Delete();

	return( true );
}

bool CChar::Death()
{
	ADDTOCALLSTACK("CChar::Death");
	// RETURN: false = delete

	if ( IsStatFlag(STATF_DEAD|STATF_INVUL) )
		return true;

	if ( m_pNPC )
	{
		//	leave no corpse if for some reason creature dies while mounted
		if ( IsStatFlag(STATF_Ridden) )
			StatFlag_Set(STATF_Conjured);
	}

	if ( OnTrigger(CTRIG_Death, this) == TRIGRET_RET_TRUE )
		return true;

	// I am dead and we need to give credit for the kill to my attacker(s).
	TCHAR * pszKillStr = Str_GetTemp();
	int iKillStrLen = sprintf(pszKillStr, g_Cfg.GetDefaultMsg(DEFMSG_KILLED_BY), (m_pPlayer)?'P':'N', GetNameWithoutIncognito() );
	int iKillers = 0;

	// Look through my memories of who i was fighting. (make sure they knew they where fighting me)
	CChar	* pKiller = NULL;
	CItem	* pItemNext;
	CItem	* pItem;
	CItemMemory * pMemoryKiller = NULL;
	int		killedBy = 0;
	std::map<DWORD,bool> mapKillers;

	for ( pItem = GetContentHead(); pItem; pItem = pItemNext, pMemoryKiller = NULL )
	{
		pItemNext = pItem->GetNext();

		if ( pItem->IsType(IT_EQ_TRADE_WINDOW) )
		{
			CItemContainer* pCont = dynamic_cast <CItemContainer*> (pItem);
			if ( pCont )
			{
				pCont->Trade_Delete();
				continue;
			}
		}

		// Sets OBODY value to BODY if LAYER_Flag_Wool is found on an NPC
		// Fixes issue with woolly sheep giving wool resource when corpse is carved after being shorn.
		if ( (pItem->GetEquipLayer() == LAYER_FLAG_Wool) && (m_pNPC) )
		{
			this->m_prev_id = this->GetID();
		}
#ifdef _ALPHASPHERE
		if ( pItem->IsMemoryTypes(MEMORY_HARMEDBY|MEMORY_AGGREIVED|MEMORY_WAR_TARG) )
#else
		if ( pItem->IsMemoryTypes(MEMORY_HARMEDBY|MEMORY_AGGREIVED) )
#endif
		{
			pMemoryKiller = STATIC_CAST<CItemMemory *>(pItem);
			if ( pMemoryKiller )
			{
				pKiller = pMemoryKiller->m_uidLink.CharFind();
				if ( pKiller )
				{
					DWORD dwKillerUID = pKiller->GetUID();

					if ( !pKiller->m_pPlayer && pKiller->NPC_PetGetOwner() )
					{
						mapKillers[dwKillerUID] = false;
						mapKillers[(DWORD)(pKiller->NPC_PetGetOwner()->GetUID())] = true;
					}
					else
					{
						if ( mapKillers.find(dwKillerUID) == mapKillers.end() ) // if we're already in don't override here
							mapKillers[dwKillerUID] = false;
					}
				}
			}
			Memory_ClearTypes(pMemoryKiller, 0xFFFF);
		}
	}

	pKiller = NULL;
	killedBy = mapKillers.size();
	CScriptTriggerArgs args(this);
	args.m_iN1 = killedBy;

	for ( std::map<DWORD,bool>::iterator itCurrentKiller = mapKillers.begin(); itCurrentKiller != mapKillers.end(); ++itCurrentKiller)
	{
		pKiller = CGrayUID((*itCurrentKiller).first).CharFind();
		if ( pKiller && ( pKiller->OnTrigger(CTRIG_Kill, pKiller, &args) != TRIGRET_RET_TRUE ) )
		{
			pKiller->Noto_Kill(this, (*itCurrentKiller).second, killedBy-1);

			iKillStrLen += sprintf( pszKillStr+iKillStrLen, "%s%c'%s'", iKillers ? ", " : "", 
				(pKiller->m_pPlayer) ? 'P':'N', (pKiller->m_pPlayer) ? pKiller->GetNameWithoutIncognito() : pKiller->GetName() );
			++iKillers;
		}
	}

	//	No aggressor/killer detected. Try detect person last hit me  from the act target
	if ( !pKiller )
	{
		CObjBase * ob = g_World.FindUID(m_Act_Targ);
		if ( ob )
			pKiller = STATIC_CAST <CChar *>(ob);
	}

	// record the kill event for posterity.

	iKillStrLen += sprintf( pszKillStr+iKillStrLen, ( iKillers ) ? ".\n" : "accident.\n" );
	if ( m_pPlayer )
		g_Log.Event(LOGL_EVENT|LOGM_KILLS, "%s", pszKillStr);

	if ( m_pParty )
		m_pParty->SysMessageAll( pszKillStr );

	NPC_PetClearOwners();	// Forgot who owns me. dismount my master if ridden.
	Reveal();
	SoundChar( CRESND_DIE );
	//Spell_Dispel(100);		// Get rid of all spell effects.

	// Only players should loose stats upon death.
	if ( m_pPlayer )
	{
		m_pPlayer->m_wDeaths++;
		if ( !(m_TagDefs.GetKeyNum("DEATHFLAGS", true) & DEATH_NOFAMECHANGE) )
			Noto_Fame( -Stat_GetAdjusted(STAT_FAME)/10 );

		//	experience could go down
		if ( g_Cfg.m_bExperienceSystem && ( g_Cfg.m_iExperienceMode&EXP_MODE_ALLOW_DOWN ))
		{
			ChangeExperience(-(static_cast<int>(m_exp)/10));
		}
	}

	// create the corpse item.
	StatFlag_Set(STATF_DEAD);
	StatFlag_Clear(STATF_Stone|STATF_Freeze|STATF_Hidden|STATF_Sleeping|STATF_Hovering);
	Stat_SetVal(STAT_STR, 0);

	Spell_Dispel(100);		// Get rid of all spell effects. (moved here to prevent double @Destroy trigger)

	//	bugfix: no need to call @DeathCorpse since no corpse is created
	CItemCorpse * pCorpse = MakeCorpse(Calc_GetRandVal(2) != 0);
	if ( pCorpse != NULL )
	{
   		CScriptTriggerArgs Args(pCorpse);
   		OnTrigger(CTRIG_DeathCorpse, this, &Args);
	}

	//	clear list of attackers
	m_lastAttackers.clear();

	if ( m_pPlayer )
	{
		SetHue( HUE_DEFAULT );	// Get all pale.

		LPCTSTR pszGhostName = NULL;
		CCharBase	*pCharDefPrev = CCharBase::FindCharBase( m_prev_id );

		switch ( m_prev_id )
		{
			case CREID_GARGMAN:
			case CREID_GARGWOMAN:
				pszGhostName = ( pCharDefPrev && pCharDefPrev->IsFemale() ? "c_garg_ghost_woman" : "c_garg_ghost_man" );
				break;
			case CREID_ELFMAN:
			case CREID_ELFWOMAN:
				pszGhostName = ( pCharDefPrev && pCharDefPrev->IsFemale() ? "c_elf_ghost_woman" : "c_elf_ghost_man" );
				break;
			default:
				pszGhostName = ( pCharDefPrev && pCharDefPrev->IsFemale() ? "c_ghost_woman" : "c_ghost_man" );
				break;
		}

		ASSERT(pszGhostName != NULL);

		SetID(static_cast<CREID_TYPE>(g_Cfg.ResourceGetIndexType( RES_CHARDEF, pszGhostName )));
		LayerAdd( CItem::CreateScript( ITEMID_DEATHSHROUD, this ));
		Update();		// show everyone I am now a ghost.
		
		
		//remove the characters which i cant see as dead from the screen
		int iDeadCannotSee = g_Cfg.m_fDeadCannotSeeLiving;
		if (iDeadCannotSee)
		{
			CWorldSearch AreaChars(GetTopPoint(), UO_MAP_VIEW_SIZE);
			AreaChars.SetSearchSquare(true);
			DWORD	dSeeChars(0);
			for (;;)
			{
				CChar	*pChar = AreaChars.GetChar();
				if ( !pChar )
					break;
				if (!CanSeeAsDead(pChar))
					GetClient()->addObjectRemove(pChar);						
			}
		}

		// Manifest the ghost War mode for ghosts.
		if ( ! IsStatFlag(STATF_War) )
			StatFlag_Set(STATF_Insubstantial);
	}

	Skill_Cleanup();

	if ( !IsClient() )
	{
		if ( m_pPlayer )
		{
			SetDisconnected();	// Respawn the NPC later
			return true;
		}

		// Makes no sense to link the corpse to something that is not going to be valid.
		if ( pCorpse && pCorpse->m_uidLink == GetUID())
			pCorpse->m_uidLink.InitUID();

		return false;	// delete this
	}
	return true;
}


bool CChar::OnFreezeCheck()
{
	ADDTOCALLSTACK("CChar::OnFreezeCheck");
	// Check if and why are held in place.
	// Can we break free ?
	// RETURN: true = held in place.

	// speed mode '4' prevents movement
	if ( m_pPlayer != NULL && (m_pPlayer->m_speedMode & 0x04) != 0 )
		return true;

	// Do not allow move if TAG.NoMoveTill > SERV.Time,
	// needed for script purposes.
	CVarDefCont* pKey = GetKey("NoMoveTill", false);
	if ( pKey != NULL )
	{
		if ( pKey->GetValNum() > g_World.GetCurrentTime().GetTimeRaw() )
			return true;

		DeleteKey("NoMoveTill");
	}

	// finally, check for STATF_Freeze|STATF_Stone flags
	if ( ! IsStatFlag( STATF_Freeze|STATF_Stone ) )
		return false;

	CItem * pFlag = LayerFind( LAYER_FLAG_Stuck );
	if ( pFlag == NULL )	// stuck for some other reason i guess.
	{
		SysMessage(( IsStatFlag( STATF_Sleeping )) ?
			g_Cfg.GetDefaultMsg( DEFMSG_UNCONSCIOUS ) :
			g_Cfg.GetDefaultMsg( DEFMSG_FROZEN ) );
	}
	else if ( pFlag->IsDeleted() == true )
	{
		// not actually stuck!
		return false;
	}
	else
	{
		// IT_EQ_STUCK
		CItem * pWeb = pFlag->m_uidLink.ItemFind();
		if ( pWeb == NULL ||
			! pWeb->IsTopLevel() ||
			pWeb->GetTopPoint() != GetTopPoint())
		{
			// Maybe we teleported away ?
			pFlag->Delete();
			return false;
		}

		// Only allow me to try to damage it once per sec.
		if ( ! pFlag->IsTimerSet())
			return Use_Obj( pWeb, false );
	}

	return ! IsPriv( PRIV_GM );
}

void CChar::Flip()
{
	ADDTOCALLSTACK("CChar::Flip");
	UpdateDir( GetDirTurn( m_dirFace, 1 ));
}

#ifdef _DIAGONALWALKCHECK_PLAYERWALKONLY
CRegionBase * CChar::CanMoveWalkTo( CPointBase & ptDst, bool fCheckChars, bool fCheckOnly, DIR_TYPE dir, bool fPathFinding, bool bWalkCheck )
#else
CRegionBase * CChar::CanMoveWalkTo( CPointBase & ptDst, bool fCheckChars, bool fCheckOnly, DIR_TYPE dir, bool fPathFinding )
#endif
{
	ADDTOCALLSTACK("CChar::CanMoveWalkTo");

	// For both players and NPC's
	// Walk towards this point as best we can.
	// Affect stamina as if we WILL move !
	// RETURN:
	//  ptDst.m_z = the new z
	//  NULL = failed to walk here.
	if ( IsSetMagicFlags( MAGICF_FREEZEONCAST ) && IsSkillMagic(m_Act_SkillCurrent) )
	{
		const CSpellDef* pSpellDef = g_Cfg.GetSpellDef(m_atMagery.m_Spell);
		if (pSpellDef != NULL && !pSpellDef->IsSpellType(SPELLFLAG_NOFREEZEONCAST))
		{
			// Casting prevents movement with freeze-on-cast enabled.
			return( NULL );
		}
	}

	if ( OnFreezeCheck() )
	{
		// NPC's would call here.
		return( NULL );	// can't move.
	}

	if ( !fCheckOnly && Stat_GetVal(STAT_DEX) <= 0 && ! IsStatFlag( STATF_DEAD ) )
	{
		SysMessageDefault( DEFMSG_FATIGUE );
		return( NULL );
	}

	int iWeightLoadPercent = GetWeightLoadPercent( GetTotalWeight());
	if ( !fCheckOnly && iWeightLoadPercent > 200 )
	{
		SysMessageDefault( DEFMSG_OVERLOAD );
		return( NULL );
	}

	if ( IsClient() && GetClient()->m_pHouseDesign )
	{
		if ( GetClient()->m_pHouseDesign->GetDesignArea().IsInside2d(GetTopPoint()) )
		{
			if ( GetClient()->m_pHouseDesign->GetDesignArea().IsInside2d(ptDst) )
			{
				ptDst.m_z = GetTopZ();
				return ptDst.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA );
			}
			return NULL;
		}

		GetClient()->m_pHouseDesign->EndCustomize(true);
	}

	// ok to go here ? physical blocking objects ?
	WORD wBlockFlags = 0;
	height_t ClimbHeight = 0;
	CRegionBase * pArea = NULL;

	EXC_TRY("CanMoveWalkTo");

	EXC_SET("Check Valid Move");
#ifdef _ELLESSAR_MAP
	pArea = CheckValidMove_New(ptDst, &wBlockFlags, dir, &ClimbHeight, fPathFinding);
#else
	if ( IsSetEF( EF_WalkCheck ) )
	{
		if (IsSetEF( EF_NewPositionChecks ))
		{
			pArea = CheckValidMove_New(ptDst, &wBlockFlags, dir, &ClimbHeight, fPathFinding);

		}
		else
			pArea = CheckValidMove_New(ptDst, &wBlockFlags, dir, &ClimbHeight);
	}
	else
	{
#ifdef _DIAGONALWALKCHECK_PLAYERWALKONLY
		pArea = CheckValidMove(ptDst, &wBlockFlags, dir, bWalkCheck);
#else
		pArea = CheckValidMove(ptDst, &wBlockFlags, dir);
#endif
	}

#endif
	if ( !pArea )
	{
		WARNWALK(("CheckValidMove%s failed\n", ((IsSetEF( EF_WalkCheck )) ? ("_New") : ("")) ));
		return NULL;
	}

	EXC_SET("Char-through walkability");
	if ( fCheckOnly )
	{
		if (( g_Cfg.m_iNpcAi&NPC_AI_PATH ) && fCheckChars )	// fast lookup of being able to go through char there
		{
			if ( !IsStatFlag(STATF_DEAD|STATF_Sleeping|STATF_Insubstantial) )
			{
				CWorldSearch AreaChars(ptDst);
				AreaChars.SetAllShow(true);

				for (;;)
				{
					CChar *pChar = AreaChars.GetChar();
					if ( !pChar )
						break;
					if (( pChar == this ) || (pChar->GetTopZ() != ptDst.m_z) || !ptDst.IsSameMap(pChar->GetTopMap()) )
						continue;

					if ( m_pNPC && !pChar->m_pPlayer )
						return NULL;	// not through non-players
					if ( pChar->IsStatFlag(STATF_DEAD|STATF_Insubstantial) || pChar->IsDisconnected())
						continue;
					if ( m_pNPC && !pChar->IsStatFlag(STATF_Hidden))
						return NULL;

					//	How much stamina to push past ?
					int iStamReq = g_Cfg.Calc_WalkThroughChar(this, pChar);
					//	cannot push the char
					if ( iStamReq < 0 || Stat_GetVal(STAT_DEX) <= iStamReq )
						return NULL;
				}
			}
		}
		return pArea;
	}

	EXC_SET("NPC's will");
	if ( ! m_pPlayer )
	{
		// Does the NPC want to walk here ?
		if ( !NPC_CheckWalkHere( ptDst, pArea, wBlockFlags ) )
			return( NULL );
	}

	/*if (	IsStatFlag(STATF_OnHorse)
		&&	( wBlockFlags & CAN_I_ROOF )
		&&	g_Cfg.m_iMountHeight
		&&	!pArea->IsFlag(REGION_FLAG_UNDERGROUND)
		&&	! IsPriv(PRIV_GM) ) // but ok in dungeons
	{
		SysMessageDefault( DEFMSG_MOUNT_CEILING );
		return( NULL );
	}*/

	EXC_SET("Creature bumping");
	// Bump into other creatures ?
	if ( ! IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Insubstantial ) && fCheckChars )
	{
		CWorldSearch AreaChars( ptDst );
		AreaChars.SetAllShow( true );	// show logged out chars.
		for (;;)
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL )
				break;
			if ( pChar == this )
				continue;
			if ( pChar->GetTopZ() != ptDst.m_z )
				continue;
			if ( ! ptDst.IsSameMap( pChar->GetTopMap()))
				continue;

			// Everyone can walk over dead/insubstantial/disconnected characters
			if ( pChar->IsStatFlag( STATF_DEAD | STATF_Insubstantial ) ||
				pChar->IsDisconnected())
			{
				if ( CanDisturb(pChar) && IsStatFlag(STATF_SpiritSpeak))
				{
					SysMessageDefault( DEFMSG_TINGLING );
				}
				continue;
			}

			if( m_pNPC )
			{
				// NPCs can't bump through other characters, but can walk over someone hidden or NPCs
				// in other cases people with 2uo can easily gain skill disallowing npc to move
				if( pChar->IsStatFlag(STATF_Hidden) || !pChar->IsClient() ) ;
				else
				{
					return NULL;
				}
			}

			// How much stamina to push past ?
			int iStamReq = g_Cfg.Calc_WalkThroughChar(this, pChar);
			TRIGRET_TYPE iRet = TRIGRET_RET_DEFAULT;

			if ( !IsSetEF(EF_Minimize_Triggers) )
			{
				CScriptTriggerArgs Args(iStamReq);
				iRet = pChar->OnTrigger(CTRIG_PersonalSpace, this, &Args);
				iStamReq = Args.m_iN1;

				if ( iRet == TRIGRET_RET_TRUE )
					return NULL;
			}

			TCHAR *pszMsg = Str_GetTemp();
			if ( pChar->IsStatFlag(STATF_Invisible) )
			{
				strcpy(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_INVISIBLE));
				continue;
			}
			else if ( pChar->IsStatFlag(STATF_Hidden) )
			{
				// reveal hidden people ?
				sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_HIDING_STUMBLE), pChar->GetName() );
				pChar->Reveal(STATF_Hidden);
			}
			else if ( pChar->IsStatFlag(STATF_Sleeping) )
			{
				sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_STEPON_BODY), pChar->GetName() );
			}
			else if (( iStamReq < 0 ) || ( Stat_GetVal(STAT_DEX) <= iStamReq ))
			{
				if ( !IsPriv(PRIV_GM) )
				{
					sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_CANTPUSH), pChar->GetName());
					SysMessage(pszMsg);
					return NULL;
				}
			}
			else
			{
				sprintf(pszMsg, g_Cfg.GetDefaultMsg(DEFMSG_PUSH), static_cast<LPCTSTR>(pChar->GetName()));
			}

			if ( iRet != TRIGRET_RET_FALSE )
				SysMessage(pszMsg);

			iStamReq = ( IsPriv(PRIV_GM) ? -10 : (( iStamReq < 0 ) ? 0 : iStamReq) );
			UpdateStatVal(STAT_DEX, -iStamReq);
			break;
		}
	}

	EXC_SET("Stamina penalty");
	// decrease stamina if running or overloaded.
	if ( !IsPriv(PRIV_GM) )
	{
		// We are overloaded. reduce our stamina faster.
		// Running acts like an increased load.
		int iStamReq = g_Cfg.Calc_DropStamWhileMoving( this, iWeightLoadPercent );
		if ( iStamReq )
		{
			// Lower avail stamina.
			UpdateStatVal( STAT_DEX, -iStamReq );
		}
	}

	StatFlag_Mod( STATF_InDoors, ( wBlockFlags & CAN_I_ROOF ) || pArea->IsFlag(REGION_FLAG_UNDERGROUND) );

	if ( wBlockFlags & CAN_I_CLIMB )
		m_zClimbHeight = ClimbHeight;
	else
		m_zClimbHeight = 0;

	EXC_CATCH;
	return( pArea );
}

void CChar::CheckRevealOnMove()
{
	ADDTOCALLSTACK("CChar::CheckRevealOnMove");
	// Are we going to reveal ourselves by moving ?
	if ( IsClient() && GetClient()->m_pHouseDesign )
	{
		if ( GetClient()->m_pHouseDesign->GetDesignArea().IsInside2d(GetTopPoint()) )
			return;

		GetClient()->m_pHouseDesign->EndCustomize(true);
	}

	if ( IsStatFlag(STATF_Invisible|STATF_Hidden|STATF_Sleeping) )
	{
		// Wake up if sleeping and this is possible.
		bool bReveal = false;

		if ( IsStatFlag(STATF_Fly|STATF_Sleeping|STATF_Hovering) || !IsStatFlag(STATF_Hidden) ||
			!Skill_UseQuick(SKILL_STEALTH, Calc_GetRandVal(105)) )
			bReveal = true;

		CScriptTriggerArgs Args(bReveal);	// ARGN1 - reveal?
		OnTrigger(CTRIG_StepStealth, this, &Args);
		bReveal = ( Args.m_iN1 != 0);

		if ( bReveal )
			Reveal();
	}
}

bool CChar::CheckLocation( bool fStanding )
{
	ADDTOCALLSTACK("CChar::CheckLocation");
	// We are at this location
	// what will happen ?
	// RETURN: true = we teleported.

	if ( IsClient() && GetClient()->m_pHouseDesign )
	{
		// stepping on items doesn't trigger anything whilst in design mode
		if ( GetClient()->m_pHouseDesign->GetDesignArea().IsInside2d(GetTopPoint()) )
			return false;

		GetClient()->m_pHouseDesign->EndCustomize(true);
	}

	if ( ! fStanding )
	{
		SKILL_TYPE iSkillActive	= Skill_GetActive();

		if ( g_Cfg.IsSkillFlag( iSkillActive, SKF_IMMOBILE ) )
		{
			Skill_Fail(false);
		}
		else if ( g_Cfg.IsSkillFlag( iSkillActive, SKF_FIGHT ) )
		{
			// Are we using a skill that is effected by motion ?
			m_atFight.m_fMoved	= 1;

			if ( g_Cfg.IsSkillFlag( iSkillActive, SKF_RANGED ) && !IsSetCombatFlags(COMBAT_ARCHERYCANMOVE) && ! IsStatFlag( STATF_ArcherCanMove ) )
			{
				// If we moved and are wielding are in combat and are using a
				// ranged weapon, then reset the weaponswingtimer.
				Fight_ResetWeaponSwingTimer();
			}
		}
		else switch ( iSkillActive )
		{
			case SKILL_MEDITATION:
			case SKILL_MAGERY:
			case SKILL_NECROMANCY:
			case SKILL_CHIVALRY:
			case SKILL_BUSHIDO:
			case SKILL_NINJITSU:
			case SKILL_SPELLWEAVING:
			case SKILL_MYSTICISM:
				// Skill is broken if we move ?
				break;
			case SKILL_HIDING:	// this should become stealth ?
				break;
			case SKILL_FENCING:
			case SKILL_MACEFIGHTING:
			case SKILL_SWORDSMANSHIP:
			case SKILL_WRESTLING:
				m_atFight.m_fMoved	= 1;
				break;
			case SKILL_ARCHERY:
				m_atFight.m_fMoved	= 1;
				if ( !IsSetCombatFlags(COMBAT_ARCHERYCANMOVE) && ! IsStatFlag( STATF_ArcherCanMove ) )
				{
					// If we moved and are wielding are in combat and are using a
					// crossbow/bow kind of weapon, then reset the weaponswingtimer.
					Fight_ResetWeaponSwingTimer();
				}
				break;
			default:
				break;
		}

		// This could get REALLY EXPENSIVE !
		if ( IsSetEF(EF_New_Triggers) && !IsSetEF(EF_Minimize_Triggers) )
			if ( m_pArea->OnRegionTrigger( this, RTRIG_STEP ) == TRIGRET_RET_TRUE )
				return( false );
	}

	bool fStepCancel = false;
	CWorldSearch AreaItems( GetTopPoint());
	for (;;)
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL )
			break;

		int zdiff = pItem->GetTopZ() - GetTopZ();

		int	height = pItem->Item_GetDef()->GetHeight();
		if ( height < 3 )
			height = 3;

		if ( zdiff > height || zdiff < -3 )
			continue;

		CScriptTriggerArgs Args( fStanding? 1 : 0 );
		if ( pItem->OnTrigger( ITRIG_STEP, this , &Args ) == TRIGRET_RET_TRUE )
		{
			fStepCancel	= true;
			continue;
		}

		switch ( pItem->GetType() )
		{
			case IT_SHRINE:
				// Resurrect the ghost
				if ( fStanding )
					continue;
				OnSpellEffect( SPELL_Resurrection, this, 1000, pItem );
				return( false );
			case IT_WEB:
				if ( fStanding )
					continue;
				// Caught in a web.
				if ( Use_Item_Web( pItem ))
					return( true );
				continue;
			// case IT_CAMPFIRE:	// does nothing. standing on burning kindling shouldn't hurt us
			case IT_FIRE:
				// fire object hurts us ?
				// pItem->m_itSpell.m_spelllevel = 0-1000 = heat level.
				{
					int iSkillLevel = pItem->m_itSpell.m_spelllevel/2;
					iSkillLevel = iSkillLevel + Calc_GetRandVal(iSkillLevel);
					if ( IsStatFlag( STATF_Fly ))	// run through fire.
					{
						iSkillLevel /= 2;
					}
					OnTakeDamage( g_Cfg.GetSpellEffect( SPELL_Fire_Field, iSkillLevel ), NULL, DAMAGE_FIRE | DAMAGE_GENERAL );
				}
				Sound( 0x15f ); // Fire noise.
				return( false );
			case IT_SPELL:
				{
					SPELL_TYPE Spell = static_cast<SPELL_TYPE>(RES_GET_INDEX(pItem->m_itSpell.m_spell));
					OnSpellEffect( Spell, pItem->m_uidLink.CharFind(), pItem->m_itSpell.m_spelllevel, pItem );
					const CSpellDef * pSpellDef = g_Cfg.GetSpellDef(Spell);
					if ( pSpellDef )
					{
						Sound( pSpellDef->m_sound);
					}
				}
				return( false );
			case IT_TRAP:
			case IT_TRAP_ACTIVE:
				OnTakeDamage( pItem->Use_Trap(), NULL, DAMAGE_HIT_BLUNT | DAMAGE_GENERAL );
				return( false );
			case IT_SWITCH:
				if ( pItem->m_itSwitch.m_fStep )
				{
					Use_Item( pItem );
				}
				return( false );
			case IT_MOONGATE:
			case IT_TELEPAD:
				if ( fStanding )
					continue;
				Use_MoonGate( pItem );
				return( true );
			case IT_SHIP_PLANK:
				// a plank is a teleporter off the ship.
				if ( ! fStanding && ! IsStatFlag( STATF_Fly|STATF_Hovering ))
				{
					// Find some place to go. (in direction of plank)
					if ( MoveToValidSpot(m_dirFace, g_Cfg.m_iMaxShipPlankTeleport, 1, true) )
					{
						pItem->SetTimeout(5*TICK_PER_SEC);	// autoclose it behind us.
						return true;
					}
				}
				continue;

			case IT_CORPSE:
				if ( m_pNPC && ( g_Cfg.m_iNpcAi&NPC_AI_LOOTING ) )
				{
					//	NPC are likely to loot corpses (but not if they are animals!)
					if ( ( Calc_GetRandVal(150) < Stat_GetAdjusted(STAT_INT) ) || ( m_pNPC->m_Brain != NPCBRAIN_ANIMAL ) )
					{
						if ( m_pArea->IsFlag(REGION_FLAG_GUARDED|REGION_FLAG_SAFE) ) ;
						else if ( IsStatFlag(STATF_Pet) && !IsStatFlag(STATF_Conjured) ) ;
						else
						{
							CItemCorpse	*pCorpse = dynamic_cast <CItemCorpse*>(this);

							if ( pCorpse != NULL && pCorpse->GetCount() > 0 )
							{
								CItem *pItem = pCorpse->GetAt( Calc_GetRandVal(pCorpse->GetCount()) );
								bool bLoot = false;

								if ( !pItem ) ;
								//	animals are looting food only
								else if ( m_pNPC->m_Brain == NPCBRAIN_ANIMAL )
								{
									switch ( pItem->GetType() )
									{
										case IT_FOOD:
										case IT_FOOD_RAW:
										case IT_MEAT_RAW:
										case IT_FRUIT:
											bLoot = true;
										default:
											break;
									}
								}
								else
								{
									if ( pItem->IsAttr(ATTR_NEWBIE) ) ;
									else if (( pItem->GetDispID() == ITEMID_BANDAGES_BLOODY1 ) || ( pItem->GetDispID() == ITEMID_BANDAGES_BLOODY2 )) ;
									else if ( pItem->GetDispID() == ITEMID_EMPTY_BOTTLE ) ;
									else if ( !CanCarry(pItem) ) ;
									else if ( pItem->IsType(IT_CONTAINER) )
									{
										CItemContainer *pItemCont = dynamic_cast <CItemContainer*>(pItem);
										if ( pItemCont->GetCount() > 0 )
											bLoot = true;
									}
									else
										bLoot = true;
								}

								if ( bLoot )
								{
									char *zMsg = Str_GetTemp();
									sprintf(zMsg, g_Cfg.GetDefaultMsg( DEFMSG_LOOT_ITEM_FROM ), pItem->GetName(), pCorpse->GetName());
									Emote(zMsg);
									ItemBounce(pItem);
								}
							}
						}
					}
				}
			default:
				break;
		}
	}

	if ( fStepCancel )
		return false;

	if ( fStanding )
		return false;

	// Check the map teleporters in this CSector. (if any)
	const CPointMap & pt = GetTopPoint();
	CSector *pSector = pt.GetSector();
	if ( !pSector )
		return false;

	const CTeleport * pTel = pSector->GetTeleport(pt);
	if ( pTel )
	{
		if ( !IsClient() && m_pNPC )
		{
			if ( !pTel->bNpc )
				return false;

			// NPCs and gates.
			if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )
			{
				// Guards won't gate into unguarded areas.
				const CRegionWorld * pArea = dynamic_cast <CRegionWorld *> ( pTel->m_ptDst.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
				if ( !pArea || ! pArea->IsGuarded())
					return false;
			}
			else if ( Noto_IsCriminal() )
			{
				// wont teleport to guarded areas.
				const CRegionWorld *pArea = dynamic_cast <CRegionWorld *> ( pTel->m_ptDst.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
				if ( !pArea || pArea->IsGuarded())
					return false;
			}
		}

		Spell_Teleport(pTel->m_ptDst, true, false, ITEMID_NOTHING);
		return true;
	}
	return false;
}

bool CChar::MoveToRegion( CRegionWorld * pNewArea, bool fAllowReject )
{
	ADDTOCALLSTACK("CChar::MoveToRegion");
	// Moving to a new region. or logging out (not in any region)
	// pNewArea == NULL = we are logging out.
	// RETURN:
	//  false = do not allow in this area.
	if ( m_pArea == pNewArea )
		return true;

	if ( ! g_Serv.IsLoading())
	{
		if ( fAllowReject && IsPriv( PRIV_GM ))
		{
			fAllowReject = false;
		}

		// Leaving region trigger. (may not be allowed to leave ?)
		if ( m_pArea )
		{
			if ( m_pArea->OnRegionTrigger( this, RTRIG_EXIT ) == TRIGRET_RET_TRUE )
			{
				if ( pNewArea && fAllowReject )
					return false;
			}

			if ( IsSetEF(EF_New_Triggers) && !IsSetEF(EF_Minimize_Triggers) )
			{
				CScriptTriggerArgs Args(m_pArea);
				if ( OnTrigger(CTRIG_RegionLeave, this, & Args) == TRIGRET_RET_TRUE )
				{
					if ( pNewArea && fAllowReject )
						return false;
				}
			}
		}

		if ( IsClient() && pNewArea )
		{
			if ( pNewArea->IsFlag(REGION_FLAG_ANNOUNCE) && !pNewArea->IsInside2d( GetTopPoint()) )	// new area.
			{
				CVarDefContStr * pVarStr = dynamic_cast <CVarDefContStr *>( pNewArea->m_TagDefs.GetKey("ANNOUNCEMENT"));
				SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_REGION_ENTER ),
					( pVarStr != NULL ) ? static_cast<LPCTSTR>(pVarStr->GetValStr()) : static_cast<LPCTSTR>(pNewArea->GetName()));
			}

			// Is it guarded / safe / non-pvp?
			else if ( m_pArea && !IsStatFlag(STATF_DEAD) )
			{
				bool redNew = ( pNewArea->m_TagDefs.GetKeyNum("RED", true) != 0 );
				bool redOld = ( m_pArea->m_TagDefs.GetKeyNum("RED", true) != 0 );

				if ( pNewArea->IsGuarded() != m_pArea->IsGuarded() )
				{
					if ( pNewArea->IsGuarded() )	// now under the protection
					{
						CVarDefContStr	*pVarStr = dynamic_cast <CVarDefContStr *>( pNewArea->m_TagDefs.GetKey("GUARDOWNER"));
						SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_REGION_GUARDS_1 ),
							( pVarStr != NULL ) ? static_cast<LPCTSTR>(pVarStr->GetValStr()) : g_Cfg.GetDefaultMsg( DEFMSG_REGION_GUARD_ART ) );
					}
					else							// have left the protection
					{
						CVarDefContStr	*pVarStr = dynamic_cast <CVarDefContStr *>( m_pArea->m_TagDefs.GetKey("GUARDOWNER"));
						SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_REGION_GUARDS_2 ),
							( pVarStr != NULL ) ? static_cast<LPCTSTR>(pVarStr->GetValStr()) : g_Cfg.GetDefaultMsg( DEFMSG_REGION_GUARD_ART ) );
					}
				}
				if ( redNew != redOld )
				{
					SysMessagef("You have %s the red region.", ( redNew ? "entered" : "left" ));
				}
				else if ( redNew && ( redNew == redOld ))
				{
					SysMessage("You are still in the red region.");
				}
				if ( pNewArea->IsFlag(REGION_FLAG_NO_PVP) != m_pArea->IsFlag(REGION_FLAG_NO_PVP))
				{
					SysMessageDefault(( pNewArea->IsFlag(REGION_FLAG_NO_PVP)) ? DEFMSG_REGION_PVPSAFE : DEFMSG_REGION_PVPNOT );
				}
				if ( pNewArea->IsFlag(REGION_FLAG_SAFE) != m_pArea->IsFlag(REGION_FLAG_SAFE) )
				{
					SysMessageDefault((pNewArea->IsFlag(REGION_FLAG_SAFE)) ? DEFMSG_REGION_SAFETYGET : DEFMSG_REGION_SAFETYLOSE);
				}
			}
		}

		// Entering region trigger.
		if ( pNewArea )
		{
			if ( pNewArea->OnRegionTrigger( this, RTRIG_ENTER ) == TRIGRET_RET_TRUE )
			{
				if ( m_pArea && fAllowReject )
					return false;
			}
			if ( IsSetEF(EF_New_Triggers) && !IsSetEF(EF_Minimize_Triggers) )
			{
				CScriptTriggerArgs Args(pNewArea);
				if ( OnTrigger(CTRIG_RegionEnter, this, & Args) == TRIGRET_RET_TRUE )
				{
					if ( m_pArea && fAllowReject )
						return false;
				}
			}
		}
	}

	m_pArea = pNewArea;
	return true;
}

bool CChar::MoveToChar( CPointMap pt )
{
	ADDTOCALLSTACK("CChar::MoveToChar");
	// Same as MoveTo
	// This could be us just taking a step or being teleported.
	// Low level: DOES NOT UPDATE DISPLAYS or container flags. (may be offline)
	// This does not check for gravity.
	//

	if ( ! pt.IsCharValid() || !pt.IsValidXY() )
		return false;

	if ( m_pPlayer && ! IsClient())	// moving a logged out client !
	{
		CSector *pSector = pt.GetSector();
		if ( !pSector )
			return false;

		// We cannot put this char in non-disconnect state.
		SetDisconnected();
		pSector->m_Chars_Disconnect.InsertHead(this);
		SetUnkPoint( pt );
		return true;
	}

	// Did we step into a new region ?
	CRegionWorld * pAreaNew = dynamic_cast <CRegionWorld *>( pt.GetRegion( REGION_TYPE_MULTI|REGION_TYPE_AREA ));
	if ( ! MoveToRegion( pAreaNew, true ))
		return false;

	bool fSectorChange	= pt.GetSector()->MoveCharToSector(this);
	bool fMapChange = false;
	CPointMap	prevPt	= GetUnkPoint();

	// Move this item to it's point in the world. (ground or top level)
	SetTopPoint(pt);
	if ( IsClient() && ( prevPt.m_map != pt.m_map ))
		fSectorChange = fMapChange = true;

	if ( fSectorChange && ! g_Serv.IsLoading() )	// there was a change in environment.
	{
		if ( fMapChange )
			GetClient()->addReSync(true);			// a must-have for map change

		CScriptTriggerArgs	Args( prevPt.m_x, prevPt.m_y, prevPt.m_z << 16 | prevPt.m_map );
		OnTrigger(CTRIG_EnvironChange, this, &Args);
	}

	if ( !m_fClimbUpdated )
		FixClimbHeight();

	return true;
}

bool CChar::MoveToValidSpot(DIR_TYPE dir, int iDist, int iDistStart, bool bFromShip)
{
	ADDTOCALLSTACK("CChar::MoveToValidSpot");
	// Move from here to a valid spot.
	// ASSUME "here" is not a valid spot. (even if it really is)

	CPointMap pt = GetTopPoint();
	pt.MoveN( dir, iDistStart );
	pt.m_z += PLAYER_HEIGHT;
	signed char startZ = pt.m_z;

	WORD wCan = GetMoveBlockFlags();	// CAN_C_SWIM
	for ( int i=0; i<iDist; ++i )
	{
		if ( pt.IsValidPoint() )
		{
			// Don't allow boarding of other ships (they may be locked)
			CRegionBase * pRegionBase = pt.GetRegion( REGION_TYPE_MULTI );
			if ( pRegionBase && pRegionBase->IsFlag( REGION_FLAG_SHIP ) )
			{
				pt.Move( dir );
				continue;
			}

			WORD wBlockFlags = wCan;
			// Reset Z back to start Z + PLAYER_HEIGHT so we don't climb buildings
			pt.m_z = startZ;
			// Set new Z so we don't end up floating or underground
			pt.m_z = g_World.GetHeightPoint( pt, wBlockFlags, true );

			// don't allow characters to pass through walls or other blocked
			// paths when they're disembarking from a ship
			if ( bFromShip && (wBlockFlags & CAN_I_BLOCK) && !(wCan & CAN_C_PASSWALLS) && (pt.m_z > startZ) )
			{
				break;
			}

			if ( ! ( wBlockFlags &~ wCan ))
			{
				// we can go here. (maybe)
				if ( Spell_Teleport( pt, true, !bFromShip, ITEMID_NOTHING) )
					return true;
			}
		}
		pt.Move( dir );
	}
	return false;
}

bool CChar::SetPrivLevel(CTextConsole * pSrc, LPCTSTR pszFlags)
{
	ADDTOCALLSTACK("CChar::SetPrivLevel");
	// "PRIVSET"
	// Set this char to be a GM etc. (or take this away)
	// NOTE: They can be off-line at the time.

	if (( pSrc->GetPrivLevel() < PLEVEL_Admin ) ||
		( pSrc->GetPrivLevel() < GetPrivLevel() ) ||
		!pszFlags[0] || !m_pPlayer )
		return false;

	CAccount * pAccount = m_pPlayer->GetAccount();
	PLEVEL_TYPE PrivLevel = CAccount::GetPrivLevelText(pszFlags);

	// Remove Previous GM Robe
	ContentConsume(RESOURCE_ID(RES_ITEMDEF,ITEMID_GM_ROBE), INT_MAX);

	if ( PrivLevel >= PLEVEL_Counsel )
	{
		pAccount->SetPrivFlags(PRIV_GM_PAGE|( PrivLevel >= PLEVEL_GM ? PRIV_GM : 0));

		UnEquipAllItems();

		CItem * pItem = CItem::CreateScript( ITEMID_GM_ROBE, this );
		if ( pItem )
		{
			pItem->SetAttr(ATTR_MOVE_NEVER|ATTR_NEWBIE|ATTR_MAGIC);
			pItem->m_itArmor.m_spelllevel = 1000;
			pItem->SetHue( ( PrivLevel >= PLEVEL_GM ) ? HUE_RED : HUE_BLUE_NAVY );
			ItemEquip(pItem);
		}
	}
	else
	{
		// Revoke GM status.
		pAccount->ClearPrivFlags( PRIV_GM_PAGE|PRIV_GM );
	}

	if ( pAccount->GetPrivLevel() < PLEVEL_Admin && PrivLevel < PLEVEL_Admin )	// can't demote admin this way.
	{
		pAccount->SetPrivLevel( PrivLevel );
	}

	Update();
	return true;
}

TRIGRET_TYPE CChar::OnTrigger( LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs )
{
	ADDTOCALLSTACK("CChar::OnTrigger");
	// Attach some trigger to the cchar. (PC or NPC)
	// RETURN: true = block further action.
	CCharBase* pCharDef = Char_GetDef();
	if ( !pCharDef )
		return TRIGRET_RET_DEFAULT;

	CTRIG_TYPE iAction;
	if ( ISINTRESOURCE(pszTrigName))
	{
		iAction = (CTRIG_TYPE) GETINTRESOURCE(pszTrigName);
		pszTrigName = sm_szTrigName[iAction];
	}
	else
	{
		iAction = (CTRIG_TYPE) FindTableSorted( pszTrigName, sm_szTrigName, COUNTOF(sm_szTrigName)-1 );
	}

	TRIGRET_TYPE iRet = TRIGRET_RET_DEFAULT;

	EXC_TRY("Trigger");
	// 1) Triggers installed on characters, sensitive to actions on all chars
	CChar * pChar = pSrc->GetChar();
	if ( pChar != NULL && this != pChar )
	{
		EXC_SET("chardef");
		// Is there an [EVENT] block call here?
		TemporaryString sCharTrigName;
		sprintf(sCharTrigName, "@char%s", pszTrigName+1);
		int iCharAction = (CTRIG_TYPE) FindTableSorted( sCharTrigName, sm_szTrigName, COUNTOF(sm_szTrigName)-1 );
		//DEBUG_ERR(("sCharTrigName %s  sCharTrigName len %d  iCharAction %d\n",sCharTrigName,strlen(sCharTrigName),iCharAction));

		if ( iCharAction > XTRIG_UNKNOWN && iCharAction < CTRIG_Click )
		{
			CGrayUID uidOldAct = pChar->m_Act_Targ;
			pChar->m_Act_Targ = GetUID();
			iRet = pChar->OnTrigger(static_cast<CTRIG_TYPE>(iCharAction), pSrc, pArgs );
			pChar->m_Act_Targ = uidOldAct;
			if ( iRet == TRIGRET_RET_TRUE )
				return iRet;	// Block further action.
		}
	}

	//	2) EVENTS
	//
	// Go thru the event blocks for the NPC/PC to do events.
	//
	EXC_SET("events");
	size_t origEvents = m_OEvents.GetCount();
	size_t curEvents = origEvents;
	for ( size_t i = 0; i < curEvents; ++i ) // EVENTS (could be modifyed ingame!)
	{
		CResourceLink * pLink = m_OEvents[i];
		if ( !pLink || !pLink->HasTrigger(iAction) )
			continue;
		CResourceLock s;
		if ( !pLink->ResourceLock(s) )
			continue;

		iRet = CScriptObj::OnTriggerScript(s, pszTrigName, pSrc, pArgs);
		if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
			return iRet;

		curEvents = m_OEvents.GetCount();
		if ( curEvents < origEvents ) // the event has been deleted, modify the counter for other trigs to work
		{
			--i;
			origEvents = curEvents;
		}
	}

	if ( m_pNPC != NULL )
	{
		// 3) TEVENTS
		EXC_SET("NPC triggers"); // TEVENTS (constant events of NPCs)
		for ( size_t i = 0; i < pCharDef->m_TEvents.GetCount(); ++i )
		{
			CResourceLink * pLink = pCharDef->m_TEvents[i];
			if ( !pLink || !pLink->HasTrigger(iAction) )
				continue;
			CResourceLock s;
			if ( !pLink->ResourceLock(s) )
				continue;
			iRet = CScriptObj::OnTriggerScript(s, pszTrigName, pSrc, pArgs);
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}

		// 4) EVENTSPET triggers
		EXC_SET("NPC triggers - EVENTSPET"); // EVENTSPET (constant events of NPCs set from sphere.ini)
		for ( size_t i = 0; i < g_Cfg.m_pEventsPetLink.GetCount(); ++i )
		{
			CResourceLink	*pLink = g_Cfg.m_pEventsPetLink[i];
			if ( !pLink || !pLink->HasTrigger(iAction) )
				continue;
			CResourceLock s;
			if ( !pLink->ResourceLock(s) )
				continue;
			iRet = CScriptObj::OnTriggerScript(s, pszTrigName, pSrc, pArgs);
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}
	}

	// 5) CHARDEF triggers
	if ( m_pPlayer == NULL ) //	CHARDEF triggers (based on body type)
	{
		EXC_SET("chardef triggers");
		if ( pCharDef->HasTrigger(iAction) )
		{
			CResourceLock s;
			if ( pCharDef->ResourceLock(s) )
			{
				iRet = CScriptObj::OnTriggerScript(s, pszTrigName, pSrc, pArgs);
				if (( iRet != TRIGRET_RET_FALSE ) && ( iRet != TRIGRET_RET_DEFAULT ))
					return iRet;
			}
		}
	}

	// 6) EVENTSPLAYER triggers
	if ( m_pPlayer != NULL )
	{
		//	EVENTSPLAYER triggers (constant events of players set from sphere.ini)
		EXC_SET("chardef triggers - EVENTSPLAYER");
		for ( size_t i = 0; i < g_Cfg.m_pEventsPlayerLink.GetCount(); ++i )
		{
			CResourceLink	*pLink = g_Cfg.m_pEventsPlayerLink[i];
			if ( !pLink || !pLink->HasTrigger(iAction) )
				continue;
			CResourceLock s;
			if ( !pLink->ResourceLock(s) )
				continue;
			iRet = CScriptObj::OnTriggerScript(s, pszTrigName, pSrc, pArgs);
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}
	}
	EXC_CATCH;

	EXC_DEBUG_START;
	g_Log.EventDebug("trigger '%s' action '%d' [0%lx]\n", pszTrigName, iAction, (DWORD)GetUID());
	EXC_DEBUG_END;
	return iRet;
}

void CChar::OnTickStatusUpdate()
{
	ADDTOCALLSTACK("CChar::OnTickStatusUpdate");

	if ( IsClient() )
		GetClient()->UpdateStats();

	// process m_fStatusUpdate flags
	int iTimeDiff = - g_World.GetTimeDiff( m_timeLastHitsUpdate );
	if ( g_Cfg.m_iHitsUpdateRate && ( iTimeDiff >= g_Cfg.m_iHitsUpdateRate ) ) // update hits for all
	{
		if ( m_fStatusUpdate & SU_UPDATE_HITS )
		{
			UpdateHitsForOthers();
			m_fStatusUpdate &= ~SU_UPDATE_HITS;
		}
		m_timeLastHitsUpdate = CServTime::GetCurrentTime();
	}

	if ( m_fStatusUpdate & SU_UPDATE_MODE )
	{
		UpdateMode();
		m_fStatusUpdate &= ~SU_UPDATE_MODE;
	}

	CObjBase::OnTickStatusUpdate();
}

void CChar::OnTickFood()
{
	ADDTOCALLSTACK("CChar::OnTickFood");
	if ( IsStatFlag(STATF_Conjured) || !Stat_GetMax(STAT_FOOD) )
		return;	// No need for food.

	// This may be money instead of food
	if ( IsStatFlag(STATF_Pet) && !NPC_CheckHirelingStatus() )
		return;

	long	lFood = Stat_GetVal(STAT_FOOD);
   	if ( Stat_GetVal(STAT_FOOD) > 0 ) lFood--;
	else lFood++;

	if ( !IsSetEF(EF_Minimize_Triggers) )
	{
		CScriptTriggerArgs Args(lFood);	// ARGN1 - new food level
		if ( OnTrigger(CTRIG_Hunger, this, &Args) == TRIGRET_RET_TRUE )
			return;
		lFood = Args.m_iN1;
	}

	Stat_SetVal(STAT_FOOD, lFood);

	int  nFoodLevel = Food_GetLevelPercent();
	if ( nFoodLevel < 40 )	// start looking for food at 40%
 	{
		// Tell everyone we look hungry.
		SysMessagef(g_Cfg.GetDefaultMsg(DEFMSG_HUNGER), Food_GetLevelMessage(false, false));
		NPC_OnTickFood(nFoodLevel);
	}
}

bool CChar::OnTick()
{
	ADDTOCALLSTACK("CChar::OnTick");
	TIME_PROFILE_INIT;
	if ( IsSetSpecific )
		TIME_PROFILE_START;
	// Assume this is only called 1 time per sec.
	// Get a timer tick when our timer expires.
	// RETURN: false = delete this.
	EXC_TRY("Tick");

	int iTimeDiff = - g_World.GetTimeDiff(m_timeLastRegen);
	if ( !iTimeDiff )
		return true;

	if ( iTimeDiff >= TICK_PER_SEC )	// don't bother with < 1 sec times.
	{
		// decay equipped items (spells)
		CItem* pItemNext = NULL;
		CItem* pItem = GetContentHead();

		for ( ; pItem != NULL; pItem = pItemNext )
		{
			EXC_TRYSUB("Ticking items");
			pItemNext = pItem->GetNext();

			// always check the validity of the memory objects
			if ( pItem->IsType(IT_EQ_MEMORY_OBJ) && !pItem->m_uidLink.ObjFind() )
			{
				pItem->Delete();
				continue;
			}

			pItem->OnTickStatusUpdate();
			if ( !pItem->IsTimerSet() || !pItem->IsTimerExpired() )
				continue;
			else if ( !OnTickEquip(pItem) )
				pItem->Delete();
			EXC_CATCHSUB("Char");
		}

		EXC_SET("last attackers");
		if ( m_lastAttackers.size() )
		{
			for ( std::vector<LastAttackers>::iterator it = m_lastAttackers.begin(); it != m_lastAttackers.end(); ++it)
			{
				LastAttackers & refAttacker = *it;
				if ( ( ++(refAttacker.elapsed) > g_Cfg.m_iAttackerTimeout ) && ( g_Cfg.m_iAttackerTimeout > 0 ) )
				{
					m_lastAttackers.erase(it);
					break;
				}
			}
		}

		if ( IsClient() )
		{
			// Players have a silly "always run" flag that gets stuck on.
			if ( -g_World.GetTimeDiff(GetClient()->m_timeLastEventWalk) > TICK_PER_SEC )
				StatFlag_Clear(STATF_Fly);

			// Check targeting timeout, if set
			if ( GetClient()->m_Targ_Timeout.IsTimeValid() && g_World.GetTimeDiff(GetClient()->m_Targ_Timeout) <= 0 )
				GetClient()->addTargetCancel();
		}

		// NOTE: Summon flags can kill our hp here. check again.
		if ( Stat_GetVal(STAT_STR) <= 0 )	// We can only die on our own tick.
		{
			m_timeLastRegen = CServTime::GetCurrentTime();
			EXC_SET("death");
			return Death();
		}
		if ( IsStatFlag( STATF_DEAD ) )	// We are dead, don't update anything.
		{
			m_timeLastRegen = CServTime::GetCurrentTime();
			return true;
		}

		for ( STAT_TYPE i = STAT_STR; i <= STAT_FOOD; i = static_cast<STAT_TYPE>(i + 1) )
		{
			EXC_SET(g_Stat_Name[i]);
			m_Stat[i].m_regen += iTimeDiff;

			int iRate = g_Cfg.m_iRegenRate[i];		// in TICK_PER_SEC

			//	No Food - Slow Regen
			if ( (i != STAT_FOOD) && Stat_GetMax(STAT_FOOD) && !Stat_GetVal(STAT_FOOD) )
				iRate *= 2;

			// Regen OVERRIDE
			int mod = 1;
			if ( i < STAT_FOOD )
			{
				char sRegen[21];
				sprintf(sRegen, "OVERRIDE.REGEN_%d", static_cast<int>(i));
				iRate -= ( GetKeyNum(sRegen, true) * 10 );
				sprintf(sRegen, "OVERRIDE.REGENVAL_%d", static_cast<int>(i));
				mod = maximum(mod,GetKeyNum(sRegen, true));
			}

			// Metabolism Bonus
			if ( i == STAT_STR )
			{
				int iRateModifier = 1 + (Stat_GetVal(STAT_DEX)/8);
				iRate += iRate / ((iRateModifier == 0) ? 1 : iRateModifier);
			}

			if ( m_Stat[i].m_regen < iRate )
				continue;

			m_Stat[i].m_regen = 0;
			if ( i == STAT_FOOD )
				OnTickFood();
			else if ( Stat_GetVal(i) != Stat_GetMax(i))
				UpdateStatVal(i, mod);
		}
	}
	else
	{
		// Check this all the time.
		if ( Stat_GetVal(STAT_STR) <= 0 )	// We can only die on our own tick.
		{
			EXC_SET("death");
			return Death();
		}
	}

	if ( IsStatFlag(STATF_DEAD) )
		return true;
	if ( IsDisconnected() )		// mounted horses can still get a tick.
	{
		m_timeLastRegen = CServTime::GetCurrentTime();
		return true;
	}

	if ( IsTimerExpired() && IsTimerSet())
	{
		EXC_SET("timer expired");
		// My turn to do some action.
		switch ( Skill_Done())
		{
			case -SKTRIG_ABORT:	EXC_SET("skill abort"); Skill_Fail(true); break;	// fail with no message or credit.
			case -SKTRIG_FAIL:	EXC_SET("skill fail"); Skill_Fail(false); break;
			case -SKTRIG_QTY:	EXC_SET("skill cleanup"); Skill_Cleanup(); break;
		}

		if ( m_pNPC )		// What to do next ?
		{
			ProfileTask aiTask(PROFILE_NPC_AI);
			EXC_SET("NPC action");
			NPC_OnTickAction();

			//	Some NPC AI actions
			if (( g_Cfg.m_iNpcAi&NPC_AI_FOOD ) && !( g_Cfg.m_iNpcAi&NPC_AI_INTFOOD ))
				NPC_Food();
			if ( g_Cfg.m_iNpcAi&NPC_AI_EXTRA )
				NPC_AI();
		}
	}
	else
	{
		// Hit my current target. (if i'm ready)
		EXC_SET("combat hit try");
		if ( IsStatFlag(STATF_War) )
		{
			if ( Fight_IsActive() )
			{
				if ( m_atFight.m_War_Swing_State == WAR_SWING_READY )
					Fight_HitTry();
			}
			else if ( Skill_GetActive() == SKILL_NONE )
				Fight_AttackNext();
		}
	}

	if ( iTimeDiff >= TICK_PER_SEC )
	{
		// Check location periodically for standing in fire fields, traps, etc.
		EXC_SET("check location");
		CheckLocation(true);
		m_timeLastRegen = CServTime::GetCurrentTime();
	}

	EXC_SET("update stats");
	OnTickStatusUpdate();

	EXC_CATCH;

#ifdef _DEBUG
	EXC_DEBUG_START;
	g_Log.EventDebug("'%s' npc '%d' player '%d' client '%d' [0%lx]\n",
		GetName(), (int)(m_pNPC ? m_pNPC->m_Brain : 0), (int)(m_pPlayer != 0), (int)IsClient(), (DWORD)GetUID());
	EXC_DEBUG_END;
#endif
	if ( IsSetSpecific )
	{
		TIME_PROFILE_END;
		DEBUG_ERR(("CChar::OnTick(%lx) took %lld.%lld to run\n", (DWORD)GetUID(), static_cast<INT64>(TIME_PROFILE_GET_HI), static_cast<INT64>(TIME_PROFILE_GET_LO)));
	}
	return true;
}
