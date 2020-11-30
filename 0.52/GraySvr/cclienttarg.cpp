//
// CClientTarg.cpp
// Copyright Menace Software (www.menasoft.com).
//
// An item is targetted.

#include "graysvr.h"	// predef header.

////////////////////////////////////////////////////////
// Targetted GM functions.

bool CClient::OnTarg_Obj_Set( CObjUID uid )
{
	// Targettted a command at an CObjBase object
	// m_Targ_Text = new command and value.
	// TARGMODE_OBJ_SET

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
	{
		SysMessage( "No object specified?" );
		return( false );
	}

	// Parse the command.
	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	strcpy( szTemp, m_Targ_Text );
	TCHAR * pArg;
	Parse( szTemp, &pArg );

	// Special stuff to check for.
	static const TCHAR * table[] =
	{
		"ACCOUNT",
		"COLOR",
		"REMOVE",
		"SHRINK",
	};

	bool fDelete = false;

	switch ( FindTableSorted( szTemp, table, COUNTOF(table)))
	{
	case 0: // "ACCOUNT"
		if ( GetPrivLevel() < PLEVEL_Admin )
		{
			SysMessage( "Invalid Priv Level" );
			return false ;
		}
		break;
	case 1:	// "COLOR"
		if ( ! pArg[0] )
		{
			addDyeOption( pObj );
			return true;
		}
		break;
	case 2: // "REMOVE" = the object is immediatly invalid so return immediatly.
	case 3: // "SHRINK"
		fDelete = true;
		break;
	}

	CGString sLogMsg;
	sLogMsg.Format( "'%s' commands uid=0%lx (%s) to '%s'", GetName(), pObj->GetUID(), pObj->GetName(), (const TCHAR*) m_Targ_Text );

	bool fRet = pObj->r_Verb( CScript( szTemp, pArg ), this );
	if ( ! fRet )
	{
//failed:
		SysMessage( "Invalid Set" );
	}
	else if ( ! fDelete )
	{
		pObj->RemoveFromView();	// strangly we need to remove this before color will take.
		pObj->Update();
	}

	g_Log.Event( LOGM_GM_CMDS, "%s=%d\n", (const TCHAR*) sLogMsg, fRet );

	return( fRet );
}

bool CClient::OnTarg_Obj_Flip( CObjUID uid )
{
	// TARGMODE_OBJ_FLIP
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) 
		return false;
	if ( ! IsPriv( PRIV_GM ))
	{
		CItem * pItem = dynamic_cast <CItem*>(pObj);
		if ( ! m_pChar->CanUse( pItem ))
		{
			return( false );
		}
	}
	pObj->Flip( m_Targ_Text );
	return true;
}

bool CClient::OnTarg_Obj_Props( CObjUID uid, const CPointMap & pt, ITEMID_TYPE id )
{
	// TARGMODE_OBJ_PROPS "INFO" "PROPS"
	// PROPS command on some object.

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )	
	{
		TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
		int len = 0;

		if ( id )
		{
			// static items have no uid's but we can still use them.
			len = sprintf( szTemp, "[Static %x] ", id );
		}
		else
		{
			// tile info for location.
			len = strcpylen( szTemp, "[No static tile] " );
		}

		const CUOMapMeter * pMeter = g_World.GetMapMeter( pt );
		ASSERT(pMeter);
		len += sprintf( szTemp+len, "TERRAIN=%x", pMeter->m_wTerrainIndex );

		SysMessage(szTemp);
	}
	else
	{
		SetTargMode();
		m_Prop_UID = m_Targ_UID = uid;
		if ( uid.IsChar())
		{
			addSkillWindow( SKILL_QTY ); // load the targets skills
		}
		addGumpPropMenu();
	}

	return true;
}

bool CClient::OnTarg_Char_PrivSet( CObjUID uid, const TCHAR * pszFlags )
{
	// TARGMODE_CHAR_PRIVSET
	if ( GetPrivLevel() < PLEVEL_Admin )	// Only an admin can do this.
		return( false );
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL || ! pChar->m_pPlayer )
		return false;

	CAccount * pAccount = pChar->m_pPlayer->GetAccount();
	ASSERT(pAccount);

	PLEVEL_TYPE PrivLevel = pAccount->GetPrivLevelText( pszFlags );

	// Remove Previous GM Robe
	pChar->ContentConsume( ITEMID_GM_ROBE, 0xFFFF );

	if ( PrivLevel >= PLEVEL_Counsel )
	{
		// Give gm robe.
		CItem * pItem = CItem::CreateScript( ITEMID_GM_ROBE );
		pItem->m_Attr |= ATTR_MOVE_NEVER | ATTR_NEWBIE | ATTR_MAGIC;
		pItem->m_itArmor.m_skilllevel = 30;

		pAccount->SetPrivFlags( PRIV_GM_PAGE );
		if ( PrivLevel >= PLEVEL_GM )
		{
			pAccount->SetPrivFlags( PRIV_GM );
			pItem->SetColor( COLOR_RED );
		}
		else
		{
			pItem->SetColor( COLOR_BLUE_NAVY );
		}
		pChar->UnEquipAllItems();
		pChar->ItemEquip( pItem );
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

	pChar->Update();
	return true;
}

bool CClient::OnTarg_Char_Bank( CObjUID uid )
{
	// Open the bank account of the person specified. TARGMODE_CHAR_BANK
	// m_tmCharBank.m_Layer = layer ?
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return false;
	addBankOpen( pChar, m_tmCharBank.m_Layer );
	return true;
}

bool CClient::OnTarg_Char_Control( CObjUID uid )
{
	// TARGMODE_CHAR_CONTROL
	// Possess this creature.
	// Leave my own body behind.

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return false;
	if ( pChar->IsDisconnected())
		return( false );	// char is not on-line.

	if ( GetPrivLevel() < pChar->GetPrivLevel())
		return false;

	// Put my newbie equipped items on it.
	CItem* pItemNext;
	for ( CItem* pItem=m_pChar->GetContentHead(); pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( ! pItem->IsAttr(ATTR_MOVE_NEVER)) 
			continue; // keep GM stuff.
		if ( ! CItemBase::IsVisibleLayer( pItem->GetEquipLayer())) 
			continue;
		switch ( pItem->GetEquipLayer())
		{
		case LAYER_BEARD:
		case LAYER_HAIR:
		case LAYER_PACK:
			continue;
		}
		pChar->LayerAdd( pItem );	// add content
	}

	// Put my GM pack stuff in it's inventory.
	CItemContainer * pPack = m_pChar->GetPack();
	if ( pPack != NULL )
	{
		CItem* pItemNext;
		for ( CItem* pItem=pPack->GetContentHead(); pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( ! pItem->IsAttr(ATTR_MOVE_NEVER))
				continue; // keep newbie stuff.
			pChar->GetPackSafe()->ContentAdd( pItem );	// add content
		}
	}

	m_pChar->ClientDetach();

	if ( pChar->IsClient())
	{
		// Maybe fight for control ? swap bodies
		CClient * pClient = pChar->m_pClient;
		pChar->ClientDetach();

		CCharPlayer * pPlayer = pChar->m_pPlayer;
		pChar->m_pPlayer = m_pChar->m_pPlayer;
		m_pChar->m_pPlayer = pPlayer;

		pClient->addPlayerStart( m_pChar );
	}
	else
	{
		m_pChar->m_pNPC = pChar->m_pNPC;	// Turn my old body into a NPC.
		pChar->m_pPlayer = m_pChar->m_pPlayer;
		m_pChar->m_pPlayer = NULL;
		pChar->m_pNPC = NULL;

		// delete my ghost.
		if ( m_pChar->GetID() == CREID_GHOSTMAN || m_pChar->GetID() == CREID_GHOSTWOMAN )
		{
			m_pChar->Delete();
		}
	}

	addPlayerStart( pChar );
	return true;
}

bool CClient::OnTarg_Item_Multi_Add( const CPointMap & pt )
{
	// TARGMODE_ADDMULTIITEM
	// ??? Get rid of this in favor of a more .SCP file type approach.

	// Break a multi out of the multi.txt files and turn it into items.
	if ( ! pt.IsValid()) 
		return( false );

	CScript s;	// It is not really a valid script type file.
	if ( ! s.OpenFind( "multi.txt" ))
		return false;

	CGString sSec;
	sSec.Format( "%i template id", m_tmAdd.m_id );
	if ( ! s.FindTextHeader(sSec))
		return false;
	if ( ! s.ReadKey())
		return false; // throw this one away
	if ( ! s.ReadKeyParse())
		return false; // this has the item count

	int iItemCount = atoi(s.GetKey()); // item count
	CPointMap ptCur;
	while (iItemCount > 0)
	{
		if ( ! s.ReadKeyParse())
			return false; // this has the item count

		int piCmd[4];		// Maximum parameters in one line
		int Arg_Qty = ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));
		int dx = piCmd[0];
		int dy = piCmd[1];
		int dz = piCmd[2];

		CItem * pItem = CItem::CreateTemplate( (ITEMID_TYPE) atoi(s.GetKey()));
		if ( pItem == NULL )
			return( false );
		ptCur.m_x = pt.m_x + dx;
		ptCur.m_y = pt.m_y + dy;
		ptCur.m_z = pt.m_z + dz;
		pItem->MoveTo( ptCur );
		iItemCount -= 1;
	}

	return true;
}

bool CClient::OnTarg_Item_Add( const CPointMap & pt )
{
	// TARGMODE_ADDITEM
	// m_tmAdd.m_id = new item id

	if ( ! pt.IsValid()) 
		return( false );

	CItem * pItem = CItem::CreateTemplate( m_tmAdd.m_id );
	if ( pItem == NULL )
		return( false );
	if ( m_tmAdd.m_fStatic == 1)
	{
		// Lock this item down
		pItem->m_Attr |= ATTR_MOVE_NEVER;
	}

	CPointMap ptNew = pt;
	ptNew.m_z ++;
	pItem->MoveToCheck( ptNew );
	m_pChar->m_Act_Targ = pItem->GetUID();	// for last target stuff. (trigger stuff)
	return true;
}

bool CClient::OnTarg_Item_Link( CObjUID uid )
{
	// TARGMODE_LINK

	CObjBase * pObj2 = uid.ObjFind();
	if ( pObj2 == NULL )
	{
		SysMessage( "Must select a dynamic object." );
		return( false );
	}

	CItem * pItem2 = dynamic_cast <CItem*>(pObj2);
	CItem * pItem1 = m_Targ_UID.ItemFind();
	if ( pItem1 == NULL )
	{
		if ( pItem2 == NULL )
		{
			m_Targ_UID.ClearUID();
			addTarget( TARGMODE_LINK, "First object must be a dynamic item, try again." );
		}
		else
		{
			m_Targ_UID = uid;
			addTarget( TARGMODE_LINK, "What do you want to link it to ?" );
		}
		return true;
	}

	if ( pItem2 == pItem1 )
	{
		SysMessage( "That's the same object. Link cancelled." );
		// Break any existing links.
		return false;
	}

	if ( pItem2 && ( pItem1->m_type == ITEM_KEY || pItem2->m_type == ITEM_KEY ))
	{
		// Setting a key to something is a special case.
		if ( pItem1->m_type != ITEM_KEY )
		{
			CItem * pTmp = pItem1;
			pItem1 = pItem2;
			pItem2 = pTmp;
		}
		// pItem1 = the ITEM_KEY
		if ( pItem2->m_itContainer.m_lockUID )
		{
			pItem1->m_itKey.m_lockUID = pItem2->m_itContainer.m_lockUID;
		}
		else if ( pItem1->m_itKey.m_lockUID )
		{
			pItem2->m_itContainer.m_lockUID = pItem1->m_itKey.m_lockUID;
		}
		else
		{
			pItem1->m_itKey.m_lockUID = pItem2->m_itContainer.m_lockUID = pItem2->GetUID();
		}
	}
	else
	{
		pItem1->m_uidLink = pObj2->GetUID();
		if ( pItem2 && ! pItem2->m_uidLink.IsValidUID())
		{
			pItem2->m_uidLink = pItem1->GetUID();
		}
	}

	SysMessage( "These items are now linked." );
	return true;
}

bool CClient::OnTarg_Tile( const CPointMap & pt )
{
	// TARGMODE_TILE
	// m_tmTile.m_Code = 1 = tile.
	// m_tmTile.m_Code = 254 = "NUKE" or  !
	// m_tmTile.m_Code = 253 = "NUKECHAR"

	if ( !m_Targ_p.IsValid())
	{
		m_Targ_p = pt;
		addTarget( TARGMODE_TILE, "Pick other corner:", true );
		return true;
	}

	if ( pt == m_Targ_p && m_tmTile.m_Code != 1) // Extract can work with one squa)
	{
		SysMessage("Thats the same point.");
		addTarget( TARGMODE_TILE, "Pick other corner:", true );
		return true;
	}

	if ( m_tmTile.m_Code == 1 )
	{
		// "EXTRACT" all map statics in the region.
		CPointMap pLT( min(m_Targ_p.m_x, pt.m_x), min(m_Targ_p.m_y, pt.m_y));
		CPointMap pRB( max(m_Targ_p.m_x, pt.m_x), max(m_Targ_p.m_y, pt.m_y));
		CRectMap rctReg;
		rctReg.SetRect( pLT.m_x, pLT.m_y, pRB.m_x, pRB.m_y );
		CPointMap pntCtr = rctReg.GetCenter();
		CScript s;
		if ( ! s.Open( m_Targ_Text, OF_WRITE|OF_TEXT ))
			return( false );

		// First find the lowest Z to use as a base
		// and count the statics
		int lz = UO_SIZE_Z;
		int iCounter = 0;
		CPointMap ptCur;
		int mx = pLT.m_x;
		for(; mx <= pRB.m_x; mx++)
		{
			for ( int my = pLT.m_y; my <= pRB.m_y; my++)
			{
				ptCur.m_x = mx;
				ptCur.m_y = my;
				const CGrayMapBlock * pBlock = g_World.GetMapBlock( ptCur );
				if ( pBlock->GetStaticQty())  // no static items here.
				{
					int x2=pBlock->GetOffsetX(ptCur.m_x);
					int y2=pBlock->GetOffsetY(ptCur.m_y);
					for ( int i=0; i<pBlock->GetStaticQty(); i++ )
					{
						if ( ! pBlock->IsStaticPoint( i, x2, y2 )) continue;
						const CUOStaticItemRec * pStatic = pBlock->GetStatic(i);

						// This static is at the coordinates in question.
						iCounter ++;
						if ( pStatic->m_z < lz)
							lz = pStatic->m_z;
					}
				}
			}
		}
		// Write a header for this multi in OSI format
		// (i have no idea what most of this means)
		s.Printf("6 version\n");
		s.Printf("%d template id\n", m_tmTile.m_id );
		s.Printf("-1 item version\n");
		s.Printf("%i num components\n", iCounter);

		for(mx = pLT.m_x; mx <= pRB.m_x; mx++)
		{
			for(int my = pLT.m_y; my <= pRB.m_y; my++)
			{
				ptCur.m_x = mx;
				ptCur.m_y = my;
				const CGrayMapBlock * pBlock = g_World.GetMapBlock( ptCur );
				if ( pBlock->GetStaticQty())  // no static items here.
				{
					int x2=pBlock->GetOffsetX(ptCur.m_x);
					int y2=pBlock->GetOffsetY(ptCur.m_y);
					for ( int i=0; i<pBlock->GetStaticQty(); i++ )
					{
						if ( ! pBlock->IsStaticPoint( i, x2, y2 )) continue;
						const CUOStaticItemRec * pStatic = pBlock->GetStatic(i);
						// This static is at the coordinates in question.
						s.Printf( "%i %i %i %i 0\n",
							pStatic->m_wTileID, mx - pntCtr.m_x, my - pntCtr.m_y, pStatic->m_z - lz);
					}
				}
			}
		}
		SysMessage( "Region Statics Extracted!" );
		return true;
	}

	if ( m_tmTile.m_Code == 254 || m_tmTile.m_Code == 253 )
	{
		// NUKE all items in the region.
		CRectMap rect;
		rect.SetRect( m_Targ_p.m_x, m_Targ_p.m_y, pt.m_x, pt.m_y );
		CPointMap pt = rect.GetCenter();
		int rx = abs( rect.m_right - rect.m_left ) / 2;
		int ry = abs( rect.m_bottom - rect.m_top ) / 2;

		if ( m_tmTile.m_Code == 254 )
		{
			CWorldSearch AreaItem( pt, max( rx, ry ));
			while (true)
			{
				CItem * pItem = AreaItem.GetItem();
				if ( pItem == NULL )
					break;
				if ( ! rect.IsInside( pItem->GetTopPoint()))
					continue;
				pItem->Delete();
			}
		}
		else
		{
			CWorldSearch AreaChar( pt, max( rx, ry ));
			while (true)
			{
				CChar* pChar = AreaChar.GetChar();
				if ( pChar == NULL )
					break;
				if ( ! rect.IsInside( pChar->GetTopPoint()))
					continue;
				if ( pChar->m_pPlayer ) continue;
				pChar->Delete();
			}
		}

		SysMessage( "Region Nuked!" );
		return true;
	}

	TCHAR Arg_Cmd[256];
	strncpy( Arg_Cmd, m_Targ_Text, sizeof(Arg_Cmd));
	Arg_Cmd[sizeof(Arg_Cmd)-1] = '\0';
	TCHAR *Arg_ppCmd[16];		// Maximum parameters in one line
	int Arg_Qty = ParseCmds( Arg_Cmd, Arg_ppCmd, COUNTOF( Arg_ppCmd ));

	signed char z = atoi(Arg_ppCmd[0]);	// z height is the first arg.
	int iArg = 0;
	for ( int x = min(m_Targ_p.m_x,pt.m_x) ; x <= max(m_Targ_p.m_x,pt.m_x); x++)
	{
		for ( int y = min(m_Targ_p.m_y,pt.m_y) ; y <= max(m_Targ_p.m_y,pt.m_y); y++)
		{
			if ( ++iArg >= Arg_Qty ) 
				iArg = 1;
			CItem * pItem = CItem::CreateTemplate( (ITEMID_TYPE) ahextoi(Arg_ppCmd[iArg]));
			pItem->m_Attr |= ATTR_MOVE_NEVER;
			CPointMap point( x, y, z );
			pItem->MoveTo( point );
		}
	}
	return true;
}

//-----------------------------------------------------------------------
// Targetted Informational skills

bool CClient::OnSkill_ItemID( CObjUID uid )
{
	// SKILL_ITEMID

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
		return( false );

	if ( pObj->IsChar())
	{
		CChar * pChar = STATIC_CAST <CChar*>(pObj);
		SysMessagef("That appears to be '%s'.", pChar->GetName());
		return( true );
	}

	CItem * pItem = STATIC_CAST <CItem*>(pObj);
	ASSERT( pItem );

	if ( pItem->IsAttr( ATTR_IDENTIFIED ))
	{
		// already identified so easier.
	}

	if ( ! m_pChar->Skill_UseQuick( SKILL_ITEMID, GetRandVal(50)))
		return( false );

#ifdef COMMENT
	if ( pItem->IsAttr(ATTR_MAGIC | ATTR_NEWBIE | ATTR_MOVE_NEVER))
	{
		len += strcpylen( pTemp+len, "magic " );
	}
#endif

	pItem->m_Attr |= ATTR_IDENTIFIED;

	// ??? Estimate it's worth ?

	SysMessagef( _TEXT( "Item '%s' appears to be worth from %d to %d" ),
		pItem->GetNameFull(true), pItem->GetBasePrice( false ), pItem->GetBasePrice( true ));
	return true;
}

bool CClient::OnSkill_EvalInt( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		SysMessage( "That does not appear to be a living being." );
		return( false );
	}

	if ( ! m_pChar->Skill_UseQuick( SKILL_EVALINT, GetRandVal(60)))
		return( false );

	const TCHAR * pDesc;
	int iVal = pChar->Stat_Get(STAT_INT);

	if (iVal <= 10)
		pDesc = "slightly less intelligent than a rock";
	else if (iVal <= 20)
		pDesc = "fairly stupid";
	else if (iVal <= 30)
		pDesc = "not the brightest";
	else if (iVal <= 40)
		pDesc = "about average";
	else if (iVal <= 50)
		pDesc = "moderately intelligent";
	else if (iVal <= 60)
		pDesc = "very intelligent";
	else if (iVal <= 70)
		pDesc = "extraordinarily intelligent";
	else if (iVal <= 80)
		pDesc = "like a formidable intellect, well beyond the ordinary";
	else if (iVal <= 90)
		pDesc = "like a definite genius";
	else if (iVal > 90)
		pDesc = "superhumanly intelligent in a manner you cannot comprehend";

	SysMessagef( "%s looks %s.", pChar->GetName(), pDesc );
	return true;
}

bool CClient::OnSkill_AnimalLore( CObjUID uid )
{
	// The creature is a "human" etc..
	// How happy.
	// Well trained.
	// Who owns it ?
	// What it eats.

	// Other "lore" type things about a creature ?
	// ex. liche = the remnants of a powerful wizard

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );

	const TCHAR * pHe = pChar->GetPronoun();
	const TCHAR * pHis = pChar->GetPossessPronoun();

	CGString sTmp;

	// What kind of animal.
	if ( pChar->IsIndividualName())
	{
		sTmp.Format( "%s is a %s", pChar->GetName(), pChar->m_pDef->GetTradeName());
		addObjMessage( sTmp, pChar );
	}

	// Who is master ?
	CChar * pCharOwner = pChar->NPC_GetOwner();
	if ( pCharOwner == NULL )
	{
		sTmp.Format( "%s is %s own master", pHe, pHis );
	}
	else
	{
		sTmp.Format( "%s is loyal to %s", pHe, ( pCharOwner == m_pChar ) ? "you" : (const TCHAR*) pCharOwner->GetName());
		// How loyal to master ?
	}
	addObjMessage( sTmp, pChar );

	// How well fed ?
	// Food count = 30 minute intervals.
	const TCHAR * pszFood;
	int ifood = pChar->m_food;
	if ( ifood > 7 ) 
		ifood = 7;
	if ( pChar->IsStat( STATF_Conjured ))
	{
		pszFood = "like a conjured creature";
	}
	else
	{
		pszFood = pChar->GetFoodLevelMessage( pCharOwner ? true : false, true );
	}
	sTmp.Format( "%s looks %s.", pHe, pszFood );
	addObjMessage( sTmp, pChar );
	return true;
}

bool CClient::OnSkill_ArmsLore( CObjUID uid )
{
	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL || ! pItem->IsArmorWeapon())
	{
notweapon:
		SysMessage( "That does not appear to be a weapon or armor." );
		return( false );
	}

	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	int len = 0;

static const TCHAR * szAttackMessages[] =
{
	"That doesn't look dangerous at all.",
	"That might hurt someone a little bit.",
	"That does small amount of damage.",
	"That does fair amount of damage.",
	"That does considerable damage.",
	"That is a dangerous weapon.",
	"That weapon does a large amount of damage.",
	"That weapon does a huge amount of damage.",
	"That weapon is deadly.",
	"That weapon is extremely deadly.",
};

static const TCHAR * szDefenseMessages[] =
{
	"That offers no protection.",
	"That might protect a little bit.",
	"That offers a little protection.",
	"That offers fair protection.",
	"That offers considerable protection.",
	"That is good protection.",
	"That is very good protection.",
	"That is excellent protection.",
	"That is nearly the best protection.",
	"That makes you almost invincible.",
};

	bool fWeapon;
	int iHitsCur;
	int iHitsMax;

	switch ( pItem->m_type )
	{
	case ITEM_ARMOR:				// some type of armor. (no real action)
	case ITEM_CLOTHING:
		fWeapon = false;
		iHitsCur = pItem->m_itArmor.m_Hits_Cur;
		iHitsMax = pItem->m_itArmor.m_Hits_Max;
		len += sprintf( szTemp+len, "Defense [%i].", pItem->Armor_GetDefense());
		break;
	case ITEM_WEAPON_MACE_CROOK:
	case ITEM_WEAPON_MACE_PICK:
	case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case ITEM_WEAPON_MACE_STAFF:
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:
	case ITEM_WEAPON_FENCE:
	case ITEM_WEAPON_BOW:
		fWeapon = true;
		iHitsCur = pItem->m_itWeapon.m_Hits_Cur;
		iHitsMax = pItem->m_itWeapon.m_Hits_Max;
		len += sprintf( szTemp+len, "Attack [%i].", pItem->Weapon_GetAttack());
		break;
	default:
		goto notweapon;
	}

	len += sprintf( szTemp+len, "This item is %s.", pItem->Armor_GetRepairDesc());

	if ( iHitsCur <= 3 || iHitsMax <= 3 )
	{
		len += strcpylen( szTemp+len, " It looks quite fragile." );
	}

	// Magical effects ?
	if ( pItem->IsAttr(ATTR_MAGIC))
	{
		len += strcpylen( szTemp+len, " It is magical." );
	}
	else if ( pItem->IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER))
	{
		len += strcpylen( szTemp+len, " It is has a slight magical aura." );
	}

	// Repairable ?
	if ( pItem->Armor_IsRepairable())
	{
		len += strcpylen( szTemp+len, " It is repairable." );
	}
	else
	{
		len += strcpylen( szTemp+len, " It is not repairable." );
	}

	// Poisoned ?
	if ( fWeapon && pItem->m_itWeapon.m_poison_skill )
	{

static const TCHAR * szPoisonMessages[] =
{
	"You see no poison.",
	"You detect a tiny amount of poison.",
	"You find a small amount of poison.",
	"You find a little poison.",
	"You find some poison.",
	"There is poison on that.",
	"You find a lot of poison.",
	"You find a large amount of poison.",
	"You find a huge amount of poison.",
	"You see enough poison to kill almost anything.",
};

		len += strcpylen( szTemp+len, " It appears to be poisoned." );
	}

	SysMessage(szTemp);
	return true;
}

bool CClient::OnSkill_Anatomy( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		addObjMessage( "That does not appear to be a living being.", pChar );
		return( false );
	}

	// Add in error cased on your skill level.

	int val = pChar->Stat_Get(STAT_STR);
	const TCHAR * pStr;
	if ( val <= 10)
		pStr = "rather feeble";
	else if (val <= 20)
		pStr = "somewhat weak";
	else if (val <= 30)
		pStr = "to be of normal strength";
	else if (val <= 40)
		pStr = "somewhat strong";
	else if (val <= 50)
		pStr = "very strong";
	else if (val <= 60)
		pStr = "extremely strong";
	else if (val <= 70)
		pStr = "extraordinarily strong";
	else if (val <= 80)
		pStr = "as strong as an ox";
	else if (val <= 90)
		pStr = "like one of the strongest people you have ever seen";
	else if (val > 90)
		pStr = "superhumanly strong";

	const TCHAR * pDex;
	val = pChar->Stat_Get(STAT_DEX);

	if (val <= 10)
		pDex = "very clumsy";
	else if (val <= 20)
		pDex = "somewhat uncoordinated";
	else if (val <= 30)
		pDex = "moderately dexterous";
	else if (val <= 40)
		pDex = "somewhat agile";
	else if (val <= 50)
		pDex = "very agile";
	else if (val <= 60)
		pDex = "extremely agile";
	else if (val <= 70)
		pDex = "extraordinarily agile";
	else if (val <= 80)
		pDex = "like they move like quicksilver";
	else if (val <= 90)
		pDex = "like one of the fastest people you have ever seen";
	else if (val > 90)
		pDex = "superhumanly agile";

	CGString sTmp;
	sTmp.Format( "%s looks %s and %s.", pChar->GetName(), pStr, pDex );
	addObjMessage( sTmp, pChar );

	if ( pChar->IsStat( STATF_Conjured ))
	{
		addObjMessage( "This is a magical creature", pChar );
	}

	// ??? looks hungry ?
	return true;
}

bool CClient::OnSkill_Forensics( CObjUID uid )
{
	// ! weird client issue targetting corpses !

	CItemCorpse * pCorpse = dynamic_cast <CItemCorpse *>( uid.ItemFind());
	if ( pCorpse == NULL )
	{
		SysMessage( "Forensics must be used on a corpse." );
		return( false );
	}

	// STATF_Sleeping
	// "They are not dead but mearly unconscious"

	CObjUID uidKiller( pCorpse->m_itCorpse.m_uidKiller );
	CChar * pCharKiller = uidKiller.CharFind();
	const TCHAR * pName = ( pCharKiller != NULL ) ? pCharKiller->GetName() : NULL;

	if ( pCorpse->IsCorpseSleeping())
	{
		SysMessagef( "%s is unconscious but alive.", pName ? pName : "It" );
		return true;
	}

	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	if ( pCorpse->m_itCorpse.m_tDeathTime )
	{
		int len = sprintf( szTemp, "This is %s and it is %d seconds old. ",
			pCorpse->GetName(), ( g_World.GetTime() - pCorpse->m_itCorpse.m_tDeathTime ) / TICK_PER_SEC );
		if ( pName == NULL )
		{
			strcpy( szTemp+len, "You cannot determine who killed it" );
		}
		else
		{
			sprintf( szTemp+len, "It looks to have been killed by %s", pName );
		}
	}
	else
	{
		int len = sprintf( szTemp, "This is the corpse of %s and it is has been carved up. ",
			pCorpse->GetName());
		if ( pName == NULL )
		{
			strcpy( szTemp+len, "You cannot determine who killed it." );
		}
		else
		{
			sprintf( szTemp+len, "It looks to have been carved by %s", pName );
		}
	}
	SysMessage( szTemp );
	return true;
}

bool CClient::OnSkill_TasteID( CObjUID uid )
{
	// Try to taste for poisons I assume.
	// Maybe taste what it is made of for ingredients ?
	// Differntiate potion types ?

	return( true );
}

int CClient::OnSkill_Done( SKILL_TYPE skill, CObjUID uid )
{
	// Skill timer has expired.
	switch ( skill )
	{
	case SKILL_ANIMALLORE:	return OnSkill_AnimalLore( uid );
	case SKILL_ARMSLORE:	return OnSkill_ArmsLore( uid );
	case SKILL_ANATOMY:		return OnSkill_Anatomy( uid );
	case SKILL_ITEMID:		return OnSkill_ItemID( uid );
	case SKILL_EVALINT:		return OnSkill_EvalInt( uid );
	case SKILL_FORENSICS:	return OnSkill_Forensics( uid );
	case SKILL_TASTEID:		return OnSkill_TasteID( uid );
	}
	return( 3 );
}

////////////////////////////////////////
// Targeted skills and actions.

bool CClient::OnTarg_Skill( CObjUID uid )
{
	// targetted skill now has it's target.
	// response to TARGMODE_SKILL
	// from Event_Skill_Use() select button from skill window

	bool fContinue = false;

	SetTargMode();	// just make sure last targ mode is gone.
	m_Targ_UID = uid;	// keep for 'last target' info.

	switch ( m_Targ_Skill )
	{
		// Delayed response type skills.
	case SKILL_BEGGING:
		m_pChar->UpdateAnimate( ANIM_BOW );

	case SKILL_STEALING:
	case SKILL_TAMING:
	case SKILL_ENTICEMENT:
	case SKILL_RemoveTrap:

		// Informational skills. (instant return)
	case SKILL_ANIMALLORE:
	case SKILL_ARMSLORE:
	case SKILL_ANATOMY:
	case SKILL_ITEMID:
	case SKILL_EVALINT:
	case SKILL_FORENSICS:
	case SKILL_TASTEID:
		m_pChar->m_Act_Targ = uid;
		return( m_pChar->Skill_Start( m_Targ_Skill ));

	case SKILL_PROVOCATION:
		if ( uid.IsItem())
		{
			SysMessage( "You can only provoke living creatures." );
			return( false );
		}
		addTarget( TARGMODE_SKILL_PROVOKE, "Who do you want to provoke them against?" );
		break;

	case SKILL_POISONING:
		// We now have to find the poison.
		addTarget( TARGMODE_SKILL_POISON, "What poison do you want to use?" );
		break;

	default:
		// This is not a targetted skill !
		break;
	}

	return true;
}

bool CClient::OnTarg_Skill_Provoke( CObjUID uid )
{
	// TARGMODE_SKILL_PROVOKE
	m_pChar->m_Act_TargPrv = m_Targ_UID;	// provoke him
	m_pChar->m_Act_Targ = uid;				// against him
	return( m_pChar->Skill_Start( SKILL_PROVOCATION ));
}

bool CClient::OnTarg_Skill_Poison( CObjUID uid )
{
	// TARGMODE_SKILL_POISON
	m_pChar->m_Act_TargPrv = m_Targ_UID;	// poison this
	m_pChar->m_Act_Targ = uid;				// with this poison
	return( m_pChar->Skill_Start( SKILL_POISONING ));
}

bool CClient::OnTarg_Skill_Herd_Dest( const CPointMap & pt )
{
	// TARGMODE_SKILL_HERD_DEST
	m_pChar->m_Act_p = pt;
	m_pChar->m_Act_Targ = m_Targ_UID;	// Who to herd?
	m_pChar->m_Act_TargPrv = m_Targ_PrvUID; // crook ?
	return( m_pChar->Skill_Start( SKILL_HERDING ));
}

bool CClient::OnTarg_Skill_Magery( CObjUID uid, const CPointMap & pt )
{
	// The client player has targeted a spell.
	// TARGMODE_SKILL_MAGERY

	if ( m_tmSkillMagery.m_Spell == SPELL_Polymorph )
	{
		return Cmd_Skill_Menu( SKILL_MAGERY, SPELL_Polymorph );
	}

	m_pChar->m_atMagery.m_Spell = m_tmSkillMagery.m_Spell;
	m_pChar->m_atMagery.m_SummonID  = m_tmSkillMagery.m_SummonID;
	m_pChar->m_Act_TargPrv = m_Targ_PrvUID;	// Source (wand or you?)
	m_pChar->m_Act_Targ = uid;
	m_pChar->m_Act_p = pt;

	return( m_pChar->Skill_Start( SKILL_MAGERY ));
}

bool CClient::OnTarg_Pet_Command( CObjUID uid, const CPointMap & pt )
{
	// Any pet command requiring a target.
	// TARGMODE_PET_CMD
	// m_Targ_UID = the pet i was instructing.
	// m_tmPetCmd = command index.
	// m_Targ_Text = the args to the command that got us here.

	CChar * pCharPet = m_Targ_UID.CharFind();
	if ( pCharPet == NULL )
		return false;
	return pCharPet->NPC_OnHearPetCmdTarg( m_tmPetCmd, GetChar(), uid, pt, m_Targ_Text );
}

bool CClient::OnTarg_Pet_Stable( CChar * pCharPet )
{
	// TARGMODE_PET_STABLE
	// NOTE: You are only allowed to stable x creatures here.
	// m_Targ_PrvUID = stable master.

	CChar * pCharMaster = m_Targ_PrvUID.CharFind();
	if ( pCharMaster == NULL ) 
		return( false );

	if ( pCharPet == NULL ||
		! pCharPet->NPC_IsOwnedBy( m_pChar ) ||
		pCharPet->m_pPlayer ||
		pCharMaster == pCharPet )
	{
		SysMessage( "Select a pet to stable" );
		return( false );
	}

	if ( ! pCharMaster->CanSee(pCharPet))
	{
		pCharMaster->Speak( "I can't seem to see your pet" );
		return( false );
	}
	
	// Shink the pet and put it in the bank box of the stable master.
	CItem * pPetItem = pCharPet->Make_Figurine( m_pChar->GetUID());
	if ( pPetItem == NULL )
	{
		pCharMaster->Speak( "It doesn't seem to want to go." );
		return( false );
	}

	pCharMaster->GetBank()->ContentAdd( pPetItem );

	CGString sMsg;
	sMsg.Format( "Please remember my name is %s. Tell me to 'Retrieve' when you want your pet back.",
		pCharMaster->GetName());
	pCharMaster->Speak( sMsg );
	return( true );
}

//-----------------------------------------------------------------------
// Targetted items with special props.

bool CClient::OnTarg_Use_Key( CItem * pKey, CItem * pItemTarg )
{
	ASSERT(pKey);
	ASSERT(pKey->m_type == ITEM_KEY);
	if ( pItemTarg == NULL )
	{
		SysMessage( "Use a key on a locked item or another key to copy." );
		return false;
	}

	if ( pKey != pItemTarg && pItemTarg->m_type == ITEM_KEY )
	{
		// We are trying to copy a key ?
		if ( ! pKey->m_itKey.m_lockUID && ! pItemTarg->m_itKey.m_lockUID )
		{
			SysMessage( "Both keys are blank." );
			return false;
		}
		if ( pItemTarg->m_itKey.m_lockUID && pKey->m_itKey.m_lockUID )
		{
			SysMessage( "To copy keys get a blank key" );
			return false;
		}

		// Need tinkering tools ???

		if ( ! m_pChar->Skill_UseQuick( SKILL_TINKERING, 30+GetRandVal(40)))
		{
			SysMessage( "You fail to copy the key." );
			return( false );
		}
		if ( pItemTarg->m_itKey.m_lockUID )
		{
			pKey->m_itKey.m_lockUID = pItemTarg->m_itKey.m_lockUID;
		}
		else
		{
			pItemTarg->m_itKey.m_lockUID = pKey->m_itKey.m_lockUID;
		}
		return( true );
	}

	if ( ! pKey->m_itKey.m_lockUID )
	{
		SysMessage( "The key is a blank." );
		return false;
	}
	if ( pKey == pItemTarg )	// Rename the key.
	{
		// We may rename the key.
		addPromptConsole( TARGMODE_NAME_KEY, "What would you like to name the key?" );
		return false;
	}

	if ( m_pChar->m_pArea->IsMatchType( REGION_TYPE_MULTI ) &&
		m_pChar->m_pArea->IsMatchType( pKey->m_itKey.m_lockUID ) &&
		pItemTarg->IsMovableType())
	{
		// I am in my house ?
		if ( ! pItemTarg->m_uidLink.IsValidUID())
		{
			// If we are in a house then lock down the item.
			pItemTarg->m_uidLink = (DWORD) pKey->m_itKey.m_lockUID;
			SysMessage( "The item has been locked down." );
			return( true );
		}
		if ( pItemTarg->m_uidLink == pKey->m_itKey.m_lockUID )
		{
			pItemTarg->m_uidLink.ClearUID();
			SysMessage( "The item has been un-locked from the structure." );
			return( true );
		}
	}

	if ( ! pItemTarg->m_itKey.m_lockUID )
	{
		SysMessage( "That does not have a lock.");
		return false;
	}
	if ( ! pKey->IsKeyLockFit( pItemTarg->m_itContainer.m_lockUID ))
	{
		SysMessage( "The key does not fit into that lock.");
		return false;
	}

	if ( pItemTarg->GetID() == ITEMID_SIGN_BRASS_1 )
	{
		// We may rename the sign.
		m_Targ_UID = pItemTarg->GetUID();
		addPromptConsole( TARGMODE_NAME_SIGN, "What should the sign say?" );
		return true;
	}

	switch ( pItemTarg->m_type )
	{
	case ITEM_CONTAINER:
		pItemTarg->m_type=ITEM_CONTAINER_LOCKED;
		SysMessage( "You lock the container.");
		break;
	case ITEM_CONTAINER_LOCKED:
		pItemTarg->m_type=ITEM_CONTAINER;
		SysMessage( "You unlock the container.");
		break;
	case ITEM_DOOR:
		pItemTarg->m_type=ITEM_DOOR_LOCKED;
		SysMessage( "You lock the door.");
		break;
	case ITEM_DOOR_LOCKED:
		pItemTarg->m_type=ITEM_DOOR;
		SysMessage( "You unlock the door.");
		break;
	case ITEM_SHIP_TILLER:
		m_Targ_UID = pItemTarg->GetUID();
		addPromptConsole( TARGMODE_NAME_SHIP, "What would you like to name the ship?" );
		return true;
	case ITEM_SHIP_PLANK:
		pItemTarg->Ship_Plank( false );	// just close it.
		// Then fall thru and lock it.
	case ITEM_SHIP_SIDE:
		pItemTarg->m_type=ITEM_SHIP_SIDE_LOCKED;
		SysMessage( "You lock the ship.");
		break;
	case ITEM_SHIP_SIDE_LOCKED:
		pItemTarg->m_type=ITEM_SHIP_SIDE;
		SysMessage( "You unlock the ship.");
		break;
	default:
		SysMessage( "That does not have a lock.");
		return false;
	}

	pItemTarg->Sound( 0x049 );
	return true;
}

bool CClient::OnTarg_Use_Deed( CItem * pDeed, const CPointMap & pt )
{
	// Place the house/ship here. (if possible)
	// Can the structure go here ?
	// ITEMID_DEED1:
	// ITEMID_SHIP_PLANS1:
	//

	ITEMID_TYPE iddeed = pDeed->m_itDeed.m_Type;
	bool fShip = CItemBase::IsShipID( iddeed );	// must be in water.

	const CItemBaseMulti * pMultiDef = CItemMulti::Multi_GetDef( iddeed );

	// Check water/mountains/etc.
	if ( pMultiDef != NULL && ! pDeed->IsAttr(ATTR_MAGIC))
	{
		// Check for items in the way and bumpy terrain.

		CGRect rect = pMultiDef->m_rect;

		int x=rect.m_left;
		for ( ; x <=rect.m_right; x++ )
		{
			int y=rect.m_top;
			for ( ; y<=rect.m_bottom; y++ );
			{
				CPointMap ptn = pt;
				ptn.m_x += x;
				ptn.m_y += y;

				CRegionBase * pRegion = pt.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA | REGION_TYPE_ROOM );
				if ( pRegion == NULL || (( pRegion->IsFlag( REGION_FLAG_NOBUILDING ) && ! fShip ) && ! IsPriv(PRIV_GM)))
				{
					SysMessage( "No building is allowed here." );
					if ( ! IsPriv( PRIV_GM ))
						return false;
				}

				WORD wBlockFlags = ( fShip ) ? CAN_C_SWIM : CAN_C_WALK;
				ptn.m_z = g_World.GetHeight( ptn, wBlockFlags, pRegion );
				if ( abs( ptn.m_z - pt.m_z ) > 4 )
				{
					SysMessage( "Terrain is too bumpy to place structure here." );
					if ( ! IsPriv( PRIV_GM )) 
						return( false );
				}
				if ( fShip )
				{
					if ( ! ( wBlockFlags & CAN_I_WATER ))
					{
						SysMessage( "The ship would not be completely in the water." );
						if ( ! IsPriv( PRIV_GM )) 
							return( false );
					}
				}
				else
				{
					if ( wBlockFlags & ( CAN_I_WATER | CAN_I_BLOCK | CAN_I_PLATFORM ))
					{
						SysMessage( "The structure is blocked." );
						if ( ! IsPriv( PRIV_GM )) 
							return( false );
					}
				}
			}
		}

		// Check for chars in the way.
		rect.OffsetRect( pt.m_x, pt.m_y );

		CWorldSearch Area( pt, UO_MAP_VIEW_SIZE );
		while (true)
		{
			CChar * pChar = Area.GetChar();
			if ( pChar == NULL ) 
				break;
			if ( pChar == m_pChar ) 
				continue;
			if ( ! rect.IsInside( pChar->GetTopPoint()))
				continue;

			SysMessagef( "%s is in your way.", pChar->GetName());
			return( false );
		}
	}

	CItem * pItemNew = CItem::CreateTemplate( iddeed );
	if ( pItemNew == NULL )
	{
		// ??? just add the item ? pDeed->m_itDeed.m_Type ?
		SysMessage( "The structure colapses." );
		return( false );
	}

	pItemNew->m_Attr |= ( pDeed->m_Attr & ( ATTR_MAGIC | ATTR_INVIS ));
	pItemNew->SetColor( pDeed->GetColor());
	pItemNew->MoveTo( pt );

	CItemMulti * pMultiItem = dynamic_cast <CItemMulti*>(pItemNew);
	if ( pMultiItem )
	{
		pMultiItem->Multi_Create( m_pChar );
	}
	if ( iddeed == ITEMID_GUILDSTONE1 ||
		iddeed == ITEMID_GUILDSTONE2 )
	{
		// Now name the guild
		m_Targ_UID = pItemNew->GetUID();
		addPromptConsole( TARGMODE_STONE_NAME, "What is the new name?" );
	}
	else if ( fShip )
	{
		pItemNew->Sound( GetRandVal(2)? 0x12:0x13 );
	}

	pDeed->Delete();	// consume the deed.
	return true;
}

bool CClient::OnTarg_Use_Item( CObjUID uid, const CPointMap & pt, ITEMID_TYPE id )
{
	// TARGMODE_USE_ITEM started from Event_DoubleClick()
	// Targetted an item to be used on some other object (char or item).
	// Solve for the intersection of the items.
	// ARGS:
	//  id = static item id
	//  uid = the target.
	//  m_Targ_UID = what is the used object on the target
	//
	// RETURN: 
	//  true = success.

	CItem * pItemUse = m_Targ_UID.ItemFind();
	if ( pItemUse == NULL )
	{
		SysMessage( "Targetted item is gone?" );
		return false;
	}

	if ( pItemUse->GetParent() != m_tmUseItem.m_pParent )
	{
		// Watch out for cheating.
		// Is the source item still in the same place as it was.
		SysMessage( "Targetted item moved?" );
		return false;
	}

	CObjBase * pObjTarg = uid.ObjFind();	// this could be null if ground is the target.
	CChar * pCharTarg = dynamic_cast <CChar*>(pObjTarg);
	CItem * pItemTarg = dynamic_cast <CItem*>(pObjTarg);

	// m_Targ_p = pt;
	m_Targ_PrvUID = m_Targ_UID;
	m_Targ_UID = uid;

	ITRIG_TYPE trigtype;
	if ( pObjTarg == NULL )
	{
		trigtype = ITRIG_TARGON_GROUND;
	}
	else if ( pCharTarg )
	{
		trigtype = ITRIG_TARGON_CHAR;
	}
	else
	{
		trigtype = ITRIG_TARGON_ITEM;
	}
	if ( pItemUse->OnTrigger( trigtype, m_pChar ))
	{
		return true;
	}

	switch ( pItemUse->m_type )
	{
	case ITEM_POTION:
		// Use a potion on something else.
		if ( pItemUse->m_itPotion.m_Type >= POTION_POISON_LESS &&
			pItemUse->m_itPotion.m_Type <= POTION_POISON_DEADLY )
		{
			// ??? If the item is a poison ask them what they want to use the poison on ?
			// Poisoning or poison self ?
		}
		else if ( pItemUse->m_itPotion.m_Type >= POTION_EXPLODE_LESS && 
			pItemUse->m_itPotion.m_Type <= POTION_EXPLODE_GREAT )
		{
			// Throw explode potion.
			if ( ! pItemUse->IsEquipped() || pItemUse->GetEquipLayer() != LAYER_DRAGGING )
				return( false );

			// Put at destination location.
			CPointMap ptBlock;
			if ( m_pChar->CanSeeLOS( pt, &ptBlock, UO_MAP_VIEW_SIZE ))	// Get the block point.
				ptBlock = pt;
			pItemUse->MoveTo( ptBlock );
			pItemUse->Effect( EFFECT_BOLT, pItemUse->GetDispID(), m_pChar, 7, 0, false );
		}
		return( true );

	case ITEM_FOOD_RAW:
		// Try to put it on some sort of fire.
		if ( ! CItemBase::IsForgeID( id ))
		{
			if ( pCharTarg == m_pChar && m_pChar->Use_Eat( pItemUse ))
			{
				return( true );
			}
			if ( pItemTarg == NULL )
			{
				// static fire ?
				SysMessage( "You must cook food on some sort of fire" );
				return( false );
			}
			if ( pItemTarg->m_type != ITEM_FIRE )
			{
				SysMessage( "You must cook food on some sort of fire" );
				return( false );
			}
		}

		// Start cooking skill.
		m_pChar->m_Act_Targ = pItemUse->GetUID();
		m_pChar->Skill_Start( SKILL_COOKING );
		return( true );

	case ITEM_KEY:
		return( OnTarg_Use_Key( pItemUse, pItemTarg ));

	case ITEM_FORGE:
		// target the ore.
		return( m_pChar->Skill_MiningSmelt( pItemTarg, pItemUse ));

	case ITEM_WEAPON_MACE_SMITH:		// Can be used for smithing.
		// Near a forge ? smithing ?
		if ( pItemTarg == NULL )
			break;
		if ( pItemTarg->m_type == ITEM_INGOT )
		{
			return Cmd_Skill_Smith( pItemTarg );
		}
		else if ( pItemTarg->Armor_IsRepairable())
		{
			// Near an anvil ? repair ?
			if ( m_pChar->Use_Repair( pItemTarg ))
				return( true );
		}
		break;

	case ITEM_CANNON_MUZZLE:
		// We have targetted the connon to something.
		if ( ( pItemUse->m_itCannon.m_Load & 3 ) != 3 )
		{
			if ( m_pChar->Use_Cannon_Feed( pItemUse, pItemTarg ))
				m_pChar->Use_Consume( pItemTarg );
			return( true );
		}

		// Fire!
		pItemUse->m_itCannon.m_Load = 0;
		pItemUse->Sound( 0x207 );
		pItemUse->Effect( EFFECT_OBJ, ITEMID_FX_TELE_VANISH, pItemUse, 9, 6 );

		if ( pObjTarg != NULL ) 
		{
			// Check distance and LOS.
			if ( pItemUse->GetDist( pObjTarg ) > UO_MAP_VIEW_SIZE )
			{
				SysMessage( "Target is too far away" );
				return( true );
			}

			// Hit the Target !
			pObjTarg->Sound( 0x207 );
			pObjTarg->Effect( EFFECT_BOLT, ITEMID_Cannon_Ball, pItemUse, 8, 0, true );
			pObjTarg->OnTakeDamage( 80 + GetRandVal( 150 ), m_pChar, DAMAGE_HIT | DAMAGE_FIRE );
		}
		return( true );

	case ITEM_WEAPON_MACE_PICK:
		// Mine at the location.
		m_pChar->m_Act_p = pt;
		return( m_pChar->Skill_Start( SKILL_MINING ));

	case ITEM_WEAPON_MACE_CROOK:
		// SKILL_HERDING
		// Selected a creature to herd
		// Move the selected item or char to this location.
		if ( pCharTarg == NULL )
		{
			// hit it ?
			SysMessage( "Try just picking up inanimate objects" );
			return( false );
		}
		// special GM version to move to coordinates.
		if ( GetPrivLevel() < PLEVEL_Seer )
		{
			// Herdable ?
			if ( pCharTarg->m_pNPC == NULL || pCharTarg->m_pNPC->m_Brain != NPCBRAIN_ANIMAL )
			{
				SysMessage( "They look somewhat annoyed" );
				return( false );
			}
		}
		m_Targ_PrvUID = pItemUse->GetUID();
		m_Targ_UID = uid;
		addTarget( TARGMODE_SKILL_HERD_DEST, "Where do you want them to go ?", true );
		return( true );

	case ITEM_CARPENTRY_CHOP:		// Carpentry type tool
	case ITEM_WEAPON_MACE_SHARP:// 22 = war axe can be used to cut/chop trees.
	case ITEM_WEAPON_FENCE:		// 24 = can't be used to chop trees.
	case ITEM_WEAPON_SWORD:		// 23 =
		// Use sharp weapon on something.

		if ( CItemBase::IsTreeID( id ))
		{
			// Just targetted a tree type
			if ( pItemUse->m_type == ITEM_WEAPON_FENCE )
			{
				CItem * pResBit = g_World.CheckNaturalResource( pt, ITEM_TREE, false );
				if ( pResBit == NULL )
				{
					SysMessage( "This is not a tree." );
					return false;
				}
				if ( pResBit->GetAmount() == 0 )
				{
					SysMessage( "There is no wood left to harvest." );
					return false;
				}

				SysMessage( "You hack at the tree and produce some kindling." );
				pResBit->SetAmount( pResBit->GetAmount() - 1 );

				m_pChar->UpdateDir( pt );
				m_pChar->UpdateAnimate( ANIM_ATTACK_WEAPON );
				m_pChar->Sound( 0x13e );
				m_pChar->ItemBounce( CItem::CreateScript(ITEMID_KINDLING1));
				return true;
			}

			m_pChar->m_Act_TargPrv = pItemUse->GetUID();
			m_pChar->m_Act_Targ = uid;
			m_pChar->m_Act_p = pt;
			return( m_pChar->Skill_Start( SKILL_LUMBERJACKING ));
		}

		if ( pItemUse->m_type == ITEM_CARPENTRY_CHOP )
		{
			if ( pItemTarg == NULL || pItemTarg->m_type != ITEM_LOG )
				break;
			return Cmd_Skill_Menu( SKILL_CARPENTRY );
		}

		if ( pItemTarg != NULL )
		{
			if ( ! m_pChar->CanTouch(pItemTarg) || m_pChar->IsTakeCrime( pItemTarg ))
				return( false );

			// Make daggers, etc. work on logs
			if ( pItemTarg->m_type == ITEM_LOG )
			{
				if ( pItemUse->IsSameDispID( ITEMID_DAGGER ))
				{
					// set the target item
					m_Targ_UID = pItemTarg->GetUID();
					return Cmd_Skill_Menu( SKILL_BOWCRAFT );
				}
				else
				{
					SysMessage( "Use a dagger on wood to create bows or shafts." );
					return false;
				}
			}
			switch ( pItemTarg->GetID())
			{
			case ITEMID_CORPSE:
				m_pChar->Use_CarveCorpse( dynamic_cast <CItemCorpse*>( pItemTarg ));
				return true;
			case ITEMID_FISH_1:
			case ITEMID_FISH_2:
			case ITEMID_FISH_3:
			case ITEMID_FISH_4:
				// Carve up Fish parts.
				pItemTarg->SetDispID( ITEMID_FOOD_FISH_RAW );
				pItemTarg->SetAmount( 4 * pItemTarg->GetAmount());
				pItemTarg->Update();
				return true;
			}

			// Item to smash ? furniture ???
		}
		if ( pCharTarg != NULL )
		{
			// Sheep have wool.
			switch ( pCharTarg->GetID())
			{
			case CREID_Sheep:
				// Get the wool.
				{
					m_pChar->ItemBounce( CItem::CreateBase( ITEMID_WOOL ));
					pCharTarg->SetID(CREID_Sheep_Sheered);
					// Set wool to regrow.
					CItem * pWool = CItem::CreateBase( ITEMID_WOOL );
					pWool->SetTimeout( 30*60*TICK_PER_SEC );
					pCharTarg->LayerAdd( pWool, LAYER_FLAG_Wool );
					pCharTarg->Update();
				}
				return true;
			case CREID_Sheep_Sheered:
				SysMessage( "You must wait for the wool to grow back" );
				return true;
			}
		}
		break;

	case ITEM_BANDAGE:	// SKILL_HEALING, or SKILL_VETERINARY
		// Use bandages on some creature.
		if ( pCharTarg == NULL )
			return( false );
		m_pChar->m_Act_TargPrv = pItemUse->GetUID();
		m_pChar->m_Act_Targ = uid;
		return( m_pChar->Skill_Start( (pCharTarg->GetCreatureType() == NPCBRAIN_ANIMAL) ? SKILL_VETERINARY : SKILL_HEALING ));
	}

	switch ( pItemUse->GetDispID())
	{
	case ITEMID_LOCKPICK1:
		// Using a lock pick on something.
		m_pChar->m_Act_Targ = m_Targ_UID;	// the locked item to be picked
		m_pChar->m_Act_TargPrv = m_Targ_PrvUID;	// the pick
		return( m_pChar->Skill_Start( SKILL_LOCKPICKING ));

	case ITEMID_DYE:
		if (( pItemTarg != NULL && pItemTarg->GetID() == ITEMID_DYEVAT ) ||
			( pCharTarg != NULL && ( pCharTarg == m_pChar || IsPriv( PRIV_GM ))))	// Change skin color.
		{
			addDyeOption( pObjTarg );
			return true;
		}
		SysMessage( "You can only use this item on a dye vat.");
		return false;

	case ITEMID_DYEVAT:
		// Use the dye vat on some object.
		if ( pObjTarg == NULL )
			return false;

		if ( pObjTarg->GetTopLevelObj() != m_pChar &&
			! IsPriv( PRIV_GM ))	// Change hair color.
		{
			SysMessage( "You must have the item in your backpack or on your person.");
			return false;
		}
		if ( pCharTarg != NULL )
		{
			// Dye hair.
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
			{
				pObjTarg->SetColor( pItemUse->GetColor());
				pObjTarg->Update();
			}
			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg == NULL )
				return true;
			// fall through
		}
		else
		{
			if ( ! IsPriv( PRIV_GM ) &&
				! pItemTarg->m_pDef->Can( CAN_I_DYE ) &&
				pItemTarg->m_type != ITEM_CLOTHING )
			{
				SysMessage( "The dye just drips off this.");
				return false;
			}
		}
		pObjTarg->SetColor( pItemUse->GetColor());
		pObjTarg->Update();
		return true;

	case ITEMID_EMPTY_BOTTLE:
		// Alchemy targeting stuff
		// Pick a reagent.
		if ( ! m_pChar->ContentFind(ITEMID_MORTAR))
		{
			SysMessage( "You have no mortar and pestle." );
			return( false );
		}
		m_pChar->m_Act_TargPrv = pItemUse->GetUID();
		return( Cmd_Skill_Alchemy( pItemTarg ));

	case ITEMID_MORTAR:
		// Alchemy targeting stuff
		// Pick a reagent.
		m_pChar->m_Act_TargPrv.ClearUID();
		return( Cmd_Skill_Alchemy( pItemTarg ));

	case ITEMID_SCISSORS1:
	case ITEMID_SCISSORS2:
	case ITEMID_SCISSORS3:
	case ITEMID_SCISSORS4:
		// cut hair as well ?
		if ( pCharTarg != NULL )
		{
			if ( pCharTarg != m_pChar && ! IsPriv(PRIV_GM))
				return( false );
			// Make this a crime ?
			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg != NULL )
				pObjTarg->Delete();
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
				pObjTarg->Delete();
			m_pChar->Sound( 0x248 );	// snip noise.
			return true;
		}
		if ( pItemTarg != NULL )
		{
			if ( ! m_pChar->CanUse( pItemTarg ))
				return( false );
			// Cut up cloth.
			int iCloth = 0;
			if ( pItemTarg->m_type == ITEM_CLOTH )
			{
				pItemTarg->ConvertBolttoCloth();
				iCloth = pItemTarg->GetAmount();
			}
			if ( pItemTarg->m_type == ITEM_CLOTHING )
			{
				// Cut up for bandages.
				iCloth = pItemTarg->GetWeight()/WEIGHT_UNITS;
			}
			if ( iCloth )
			{
				CItem * pItemNew = CItem::CreateBase( ITEMID_BANDAGES1 );
				pItemNew->SetColor( pItemTarg->GetColor());
				pItemNew->SetAmount( iCloth );
				m_pChar->ItemBounce( pItemNew );
				pItemTarg->Delete();
				m_pChar->Sound( 0x248 );	// snip noise.
				return( true );
			}
			if ( ( pItemTarg->GetID() == ITEMID_HIDES ) || ( pItemTarg->GetID() == ITEMID_HIDES_2 ))
			{
				// Cut up the hides and create strips of leather
				CItem * pItemNew = CItem::CreateBase( ITEMID_LEATHER_1 );
				pItemNew->SetAmount( pItemTarg->GetAmount());
				pItemTarg->Delete();
				m_pChar->ItemBounce( pItemNew );
				m_pChar->Sound( 0x248 );	// snip noise.
				return( true );
			}
		}
		SysMessage( "Use scissors on hair or cloth to cut" );
		return false;

	case ITEMID_FISH_POLE1:
	case ITEMID_FISH_POLE2:
		m_pChar->m_Act_p = pt;
		return( m_pChar->Skill_Start( SKILL_FISHING ));

	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
		// it acts as a key.
		{
		if ( pItemTarg == NULL )
			return( false );
		CItemContainer* pKeyRing = dynamic_cast <CItemContainer*>(pItemUse);
		if ( pKeyRing == NULL )
			return( false );

		if ( pItemTarg == pItemUse )
		{
			// Use a keyring on self = remove all keys.
			pKeyRing->ContentsTransfer( m_pChar->GetPack(), false );
			return( true );
		}

		// try all the keys on the object.
		if ( ! pItemTarg->m_itContainer.m_lockUID )
		{
			SysMessage( "That does not have a lock.");
			return false;
		}

		CItem * pKey = pKeyRing->ContentFindType( ITEM_KEY, pItemTarg->m_itContainer.m_lockUID );
		if ( ! pKey )
		{
			SysMessage( "You don't have a key for this." );
			return( false );
		}

		return( OnTarg_Use_Key( pKey, pItemTarg ));
		}

	case ITEMID_DEED1:
	case ITEMID_DEED2:
	case ITEMID_SHIP_PLANS1:
	case ITEMID_SHIP_PLANS2:
		return( OnTarg_Use_Deed( pItemUse, pt ));

	case ITEMID_YARN1:
	case ITEMID_YARN2:
	case ITEMID_YARN3:
	case ITEMID_THREAD1:
	case ITEMID_THREAD2:
		// Use this on a loom.
		if ( pItemTarg == NULL ) 
			break;
		if ( ! ( pItemTarg->IsSameDispID(ITEMID_LOOM1) ||
			pItemTarg->IsSameDispID(ITEMID_LOOM2)) )
			break;

		{
static const TCHAR * Txt_LoomUse[] =
{
	"You start a new bolt of cloth.",
	"The bolt of cloth needs a good deal more.",
	"The bolt of cloth needs a little more.",
	"The bolt of cloth is nearly finished.",
	"The bolt of cloth is finished.",
};

		// pItemTarg->SetAnim( (ITEMID_TYPE)( pItemTarg->GetID() + 1 ), 2*TICK_PER_SEC );

		// Use more1 to record the type of resource last used on this object
		// Use more2 to record the number of resources used so far
		// Check what was used last.
		if ( pItemTarg->m_itLoom.m_ClothID != pItemUse->GetDispID() && 
			pItemTarg->m_itLoom.m_ClothID )
		{
			// throw away what was on here before
			SysMessage("You remove the previously uncompleted weave.");
			CItem * pItemCloth = CItem::CreateTemplate( pItemTarg->m_itLoom.m_ClothID );
			pItemCloth->SetAmount( pItemTarg->m_itLoom.m_ClothQty );
			pItemTarg->m_itLoom.m_ClothQty = 0;
			pItemTarg->m_itLoom.m_ClothID = ITEMID_NOTHING;
			m_pChar->ItemBounce( pItemCloth );
			return true;
		}

		pItemTarg->m_itLoom.m_ClothID = pItemUse->GetDispID();
		m_pChar->Use_Consume( pItemUse, 1 );

		if ( pItemTarg->m_itLoom.m_ClothQty < COUNTOF( Txt_LoomUse )-1 )
		{
			SysMessage( Txt_LoomUse[ pItemTarg->m_itLoom.m_ClothQty ] );
			pItemTarg->m_itLoom.m_ClothQty ++;
		}
		else
		{
			SysMessage( Txt_LoomUse[ COUNTOF( Txt_LoomUse )-1 ] );
			pItemTarg->m_itLoom.m_ClothQty = 0;
			pItemTarg->m_itLoom.m_ClothID = ITEMID_NOTHING;
			m_pChar->ItemBounce( CItem::CreateScript(ITEMID_CLOTH_BOLT1));
		}
		}
		return true;

	case ITEMID_WOOL:
	case ITEMID_WOOL2:
	case ITEMID_COTTON:
	case ITEMID_COTTON_RAW:
		// Use on a spinning wheel.
		if ( pItemTarg == NULL )
			break;
		if ( ! pItemTarg->IsSameDispID(ITEMID_SPINWHEEL1) &&
			! pItemTarg->IsSameDispID(ITEMID_SPINWHEEL2))
			break;
		pItemTarg->SetAnim( (ITEMID_TYPE)( pItemTarg->GetID() + 1 ), 2*TICK_PER_SEC );
		m_pChar->Use_Consume( pItemUse, 1 );

		switch( pItemUse->GetDispID())
		{
		// 1 pile of wool yields three balls of yarn
		case ITEMID_WOOL:
		case ITEMID_WOOL2:
			{
			SysMessage("You create some yarn.");
			CItem * pNewItem = CItem::CreateScript(ITEMID_YARN1);
			pNewItem->SetAmountUpdate( 3 );
			m_pChar->ItemBounce( pNewItem );
			}
			return true;
		// 1 pile of cotton yields six spools of thread
		case ITEMID_COTTON:
		case ITEMID_COTTON_RAW:
			{
			SysMessage("You create some thread.");
			CItem * pNewItem = CItem::CreateScript(ITEMID_THREAD1);
			pNewItem->SetAmountUpdate( 6 );
			m_pChar->ItemBounce( pNewItem );
			}
			return true;
		}
		break;

	case ITEMID_SEWINGKIT:
		// Use on cloth or hides
		if ( pItemTarg == NULL) 
			break;
		if ( ! m_pChar->CanUse( pItemTarg ))
			return( false );
		if ( ( pItemTarg->GetID() == ITEMID_HIDES ) ||
			( pItemTarg->GetID() == ITEMID_HIDES_2 ) ||
			( pItemTarg->GetID() == ITEMID_LEATHER_1 ) ||
			( pItemTarg->GetID() == ITEMID_LEATHER_2 ))
			return( Cmd_Skill_Menu( SKILL_TAILORING, 24 ));
		if ( pItemTarg->m_type == ITEM_CLOTH )
			return( Cmd_Skill_Menu( SKILL_TAILORING, 30 ));
		SysMessage( "You can't use a sewing kit on that." );
		return (false);

	case ITEMID_BANDAGES_BLOODY1:
	case ITEMID_BANDAGES_BLOODY2:
		// Use these on water to clean them.
		if ( CItemBase::IsWaterWashID( id ))
		{
			// Make clean.
			pItemUse->SetDispID( ITEMID_BANDAGES1 );
			return( true );
		}
		SysMessage( "clean bloody bandages in water" );
		return( false );

	case ITEMID_PITCHER:
		// Fill it up with water.
		if ( CItemBase::IsWaterWashID( id ))
		{
			pItemUse->SetDispID( ITEMID_PITCHER_WATER );
			return( true );
		}
		SysMessage( "fill pitcher with some liquid." );
		break;

	case ITEMID_Cannon_Ball:
		if ( m_pChar->Use_Cannon_Feed( pItemTarg, pItemUse ))
		{
			pItemUse->Delete();
			return( true );
		}
		break;
	}

	SysMessage( "You can't think of a way to use that item.");
	return false;
}

bool CClient::OnTarg_Stone_Recruit( CObjUID uid )
{
	CItemStone * pStone = dynamic_cast <CItemStone*> (m_Targ_UID.ItemFind());
	if ( !pStone )
		return false;
	return( pStone->AddRecruit(uid.CharFind()) != NULL );
}

