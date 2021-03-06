//
// CClientTarg.cpp
// Copyright Menace Software (www.menasoft.com).
//
// An item is targetted.

#include "graysvr.h"	// predef header.
#include "CClient.h"

////////////////////////////////////////////////////////
// Targetted GM functions.

bool CClient::OnTarg_Obj_Set( CObjBase * pObj )
{
	// CLIMODE_TARG_OBJ_SET
	// Targettted a command at an CObjBase object
	// ARGS:
	//  m_Targ_Text = new command and value.

	if ( pObj == NULL )
	{
		SysMessage( "No object specified?" );
		return( false );
	}

	// Parse the command.
	CGString sLogMsg;
	sLogMsg.Format( "'%s' commands uid=0%lx (%s) to '%s'", (LPCTSTR) GetName(), (DWORD) pObj->GetUID(), (LPCTSTR) pObj->GetName(), (LPCTSTR) m_Targ_Text );

	// Check priv level for the new verb.
	if ( ! g_Cfg.CanUsePrivVerb( pObj, m_Targ_Text, this ))
	{
		SysMessage( "You lack privilege to do this." );
		g_Log.Event( LOGM_GM_CMDS, "%s=0\n", (LPCTSTR) sLogMsg );
		return( false );
	}

	// Special stuff to check for.
	static LPCTSTR const sm_szSpecialTable[] =
	{
		"COLOR",
		"REMOVE",
		"SHRINK",
	};

	bool fDelete = false;
	CScript sCmd( m_Targ_Text );

	switch ( FindTableHeadSorted( sCmd.GetKey(), sm_szSpecialTable, COUNTOF(sm_szSpecialTable)))
	{
	case 0:	// "COLOR"
		if ( ! sCmd.HasArgs() )
		{
			addDyeOption( pObj );
			return true;
		}
		break;
	case 1: // "REMOVE" = the object is immediatly invalid so return immediatly.
	case 2: // "SHRINK"
		fDelete = true;
		break;
	}

	bool fRet = pObj->r_Verb( sCmd, this );
	if ( ! fRet )
	{
		SysMessage( "Invalid Set" );
	}

	else if ( ! fDelete )
	{
		// BANK command will be harmed by RemoveFromView
		if ( pObj->IsItemEquipped())
		{
			pObj->RemoveFromView();	// strangly we need to remove this before color will take.
			pObj->Update();
		}
	}

	if ( GetPrivLevel() >= g_Cfg.m_iCommandLog )
		g_Log.Event( LOGM_GM_CMDS, "%s=%d\n", (LPCTSTR) sLogMsg, fRet );
	return( fRet );
}


bool CClient::OnTarg_Obj_Function( CObjBase * pObj, const CPointMap & pt, ITEMID_TYPE id )
{
	m_Targ_p	= pt;
	LPCTSTR	pSpace	= strchr( m_Targ_Text, ' ' );
	if ( !pSpace )
		pSpace	= strchr( m_Targ_Text, '\t' );
	if ( pSpace )
		GETNONWHITESPACE( pSpace );
	
	CScriptTriggerArgs	Args( pSpace ? pSpace : "" );
	Args.m_VarsLocal.SetNum( "ID", id, true );
	Args.m_pO1	= pObj;
	m_pChar->r_Call( (LPCTSTR) m_Targ_Text, this, &Args );
	return true;
}


bool CClient::OnTarg_Obj_Info( CObjBase * pObj, const CPointMap & pt, ITEMID_TYPE id )
{
	// CLIMODE_TARG_OBJ_INFO "INFO"

	if ( pObj )
	{
		SetTargMode();
		addGumpDialogProps(pObj->GetUID());
	}
	else
	{
		TCHAR szTemp[ SCRIPT_MAX_LINE_LEN ];
		int len = 0;
		if ( id )
		{
			len = sprintf( szTemp, "[Static z=%d, 0%x=", pt.m_z, id );

			// static items have no uid's but we can still use them.
			CItemBase * pItemDef = CItemBase::FindItemBase(id);
			if ( pItemDef )
			{
				len += sprintf( szTemp+len, "%s->%s], ", pItemDef->GetResourceName(),
					g_Cfg.ResourceGetName( RESOURCE_ID( RES_TYPEDEF, pItemDef->GetType() )));
			}
			else
			{
				len += sprintf( szTemp+len, "NON scripted], " );
			}
		}
		else
		{
			// tile info for location.
			len = strcpylen( szTemp, "[No static tile], " );
		}

		const CUOMapMeter * pMeter = g_World.GetMapMeter( pt );
		ASSERT(pMeter);
		len += sprintf( szTemp+len, "TERRAIN=0%x   TYPE=%s",
			pMeter->m_wTerrainIndex,
			g_World.GetTerrainItemTypeDef( pMeter->m_wTerrainIndex )->GetResourceName() );

		SysMessage(szTemp);
	}

	return true;
}

bool CClient::Cmd_Control( CChar * pChar2 )
{
	// I wish to control pChar2
	// Leave my own body behind.

	if ( pChar2 == NULL )
		return false;
	if ( pChar2->IsDisconnected())
		return( false );	// char is not on-line. (then we should make it so !)

	if ( GetPrivLevel() < pChar2->GetPrivLevel())
		return false;

	ASSERT(m_pChar);
	CChar * pChar1 = m_pChar;

	// Put my newbie equipped items on it.
	CItem* pItemNext;
	for ( CItem* pItem=pChar1->GetContentHead(); pItem!=NULL; pItem=pItemNext )
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
		pChar2->LayerAdd( pItem );	// add content
	}

	// Put my GM pack stuff in it's inventory.
	CItemContainer * pPack1 = pChar1->GetPack();
	if ( pPack1 != NULL )
	{
		CItemContainer * pPack2 = pChar2->GetPackSafe();
		CItem* pItemNext;
		for ( CItem* pItem=pPack1->GetContentHead(); pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( ! pItem->IsAttr(ATTR_MOVE_NEVER))
				continue; // keep newbie stuff.
			pPack2->ContentAdd( pItem );	// add content
		}
	}

	pChar1->ClientDetach();
	m_pChar = NULL;
	CClient * pClient2 = pChar2->GetClient();
	if ( pClient2 )	// controled char is a player/client.
	{
		pChar2->ClientDetach();
		pClient2->m_pChar = NULL;
	}

	CCharPlayer * pPlayer1 = pChar1->m_pPlayer;
	if ( pPlayer1 )
	{
		pPlayer1->GetAccount()->DetachChar(pChar1);
	}
	CCharPlayer * pPlayer2 = pChar2->m_pPlayer;
	if ( pPlayer2 )
	{
		pPlayer2->GetAccount()->DetachChar(pChar2);
	}

	// swap m_pPlayer (if they even are both players.)

	pChar1->m_pPlayer = pPlayer2;
	pChar2->m_pPlayer = pPlayer1;

	ASSERT( pChar1->m_pNPC == NULL );
	pChar1->m_pNPC = pChar2->m_pNPC;	// Turn my old body into a NPC. (if it was)
	pChar2->m_pNPC = NULL;

	if ( pPlayer1 )
	{
		pPlayer1->GetAccount()->AttachChar(pChar2);
	}
	if ( pPlayer2 )
	{
		pPlayer2->GetAccount()->AttachChar(pChar1);
	}
	if ( pClient2 )
	{
		pClient2->addPlayerStart( pChar1 );
	}
	else
	{
		// delete my ghost.
		if ( pChar1->GetID() == CREID_EQUIP_GM_ROBE ||
			pChar1->GetID() == CREID_GHOSTMAN ||
			pChar1->GetID() == CREID_GHOSTWOMAN )	// CREID_EQUIP_GM_ROBE
		{
			pChar1->Delete();
		}
		else
		{
			pChar1->SetTimeout(1);	// must kick start the npc.
		}
	}
	addPlayerStart( pChar2 );
	return true;
}

bool CClient::OnTarg_UnExtract( CObjBase * pObj, const CPointMap & pt )
{
	// CLIMODE_TARG_UNEXTRACT
	// ??? Get rid of this in favor of a more .SCP file type approach.
	// result of the MULTI command.
	// Break a multi out of the multi.txt files and turn it into items.

	if ( ! pt.IsValidPoint())
		return( false );

	CScript s;	// It is not really a valid script type file.
	if ( ! g_Cfg.OpenResourceFind( s, m_Targ_Text ))
		return false;

	CGString sSec;
	sSec.Format( "%i template id", m_tmTile.m_id );
	if ( ! s.FindTextHeader(sSec))
		return false;
	if ( ! s.ReadKey())
		return false; // throw this one away
	if ( ! s.ReadKeyParse())
		return false; // this has the item count

	int iItemCount = atoi(s.GetKey()); // item count
	while (iItemCount > 0)
	{
		if ( ! s.ReadKeyParse())
			return false; // this has the item count

		int piCmd[4];		// Maximum parameters in one line
		int iArgQty = Str_ParseCmds( s.GetArgStr(), piCmd, COUNTOF(piCmd));

		CItem * pItem = CItem::CreateTemplate( (ITEMID_TYPE) atoi(s.GetKey()), NULL, m_pChar );
		if ( pItem == NULL )
			return( false );

		CPointMap ptOffset( piCmd[0], piCmd[1], piCmd[2] );
		ptOffset += pt;
		pItem->MoveTo( ptOffset );
		pItem->Update();
		iItemCount --;
	}

	return true;
}

bool CClient::OnTarg_Item_Add( CObjBase * pObj, const CPointMap & pt )
{
	// CLIMODE_ADDITEM
	// m_tmAdd.m_id = new item id

	if ( ! pt.IsValidPoint())
		return( false );
	ASSERT( m_pChar );

	CItem * pItem = CItem::CreateTemplate( m_tmAdd.m_id, NULL, m_pChar );
	if ( pItem == NULL )
		return( false );
	if ( m_tmAdd.m_fStatic == 1)
	{
		// Lock this item down
		pItem->SetAttr( ATTR_MOVE_NEVER );
	}

	if ( pItem->IsType(IT_MULTI))
	{
		CItem * pItemNew = OnTarg_Use_Multi( pItem->Item_GetDef(), pt, pItem->m_Attr, pItem->GetHue());
		pItem->Delete();
		if ( pItemNew == NULL )
			return( false );
		pItem = pItemNew;
	}
	else
	{
		if ( pObj && 
			pObj->IsItemInContainer() && 
			m_pChar->CanUse( STATIC_CAST<CItem*>(pObj), true ))
		{
			pItem->MoveNearObj( pObj );
		}
		else
		{
			CPointMap ptNew = pt;
			ptNew.m_z ++;
			pItem->MoveToCheck( ptNew, m_pChar );
		}
	}

	m_pChar->m_Act_Targ = pItem->GetUID();	// for last target stuff. (trigger stuff)
	return true;
}

bool CClient::OnTarg_Item_Link( CObjBase * pObj2 )
{
	// CLIMODE_LINK

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
			m_Targ_UID.InitUID();
			addTarget( CLIMODE_TARG_LINK, "First object must be a dynamic item, try again." );
		}
		else
		{
			m_Targ_UID = pObj2->GetUID();
			addTarget( CLIMODE_TARG_LINK, "What do you want to link it to ?" );
		}
		return true;
	}

	if ( pItem2 == pItem1 )
	{
		SysMessage( "That's the same object. Link cancelled." );
		// Break any existing links.
		return false;
	}

	if ( pItem2 && ( pItem1->IsType(IT_KEY) || pItem2->IsType(IT_KEY)))
	{
		// Linking a key to something is a special case.
		if ( ! pItem1->IsType(IT_KEY))
		{
			CItem * pTmp = pItem1;
			pItem1 = pItem2;
			pItem2 = pTmp;
		}
		// pItem1 = the IT_KEY
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

int CClient::Cmd_Extract( CScript * pScript, CRectMap &rect, int & zlowest )
{
	// RETURN: Number of statics here.
	CPointMap ptCtr = rect.GetCenter();

	int iCount = 0;
	for ( int mx = rect.m_left; mx <= rect.m_right; mx++)
	{
		for ( int my = rect.m_top; my <= rect.m_bottom; my++)
		{
			CPointMap ptCur( mx, my );
			const CGrayMapBlock * pBlock = g_World.GetMapBlock( ptCur );
			int iQty = pBlock->m_Statics.GetStaticQty();
			if ( ! iQty )  // no static items here.
				continue;

			int x2=pBlock->GetOffsetX(mx);
			int y2=pBlock->GetOffsetY(my);
			for ( int i=0; i<iQty; i++ )
			{
				if ( ! pBlock->m_Statics.IsStaticPoint( i, x2, y2 ))
					continue;
				const CUOStaticItemRec * pStatic = pBlock->m_Statics.GetStatic(i);
				ASSERT(pStatic);
				iCount ++;
				if ( pScript )
				{
					// This static is at the coordinates in question.
					pScript->Printf( "%i %i %i %i 0\n",
						pStatic->GetDispID(), mx - ptCtr.m_x, my - ptCtr.m_y, pStatic->m_z - zlowest);
				}
				else
				{
					if ( pStatic->m_z < zlowest)
					{
						zlowest = pStatic->m_z;
					}
				}
			}
		}
	}

	// Extract Multi's ??? 


	// Extract dynamics as well.

	int rx = 1 + abs( rect.m_right - rect.m_left ) / 2;
	int ry = 1 + abs( rect.m_bottom - rect.m_top ) / 2;

	CWorldSearch AreaItem( ptCtr, max( rx, ry ));
	while (true)
	{
		CItem * pItem = AreaItem.GetItem();
		if ( pItem == NULL )
			break;
		if ( ! rect.IsInside2d( pItem->GetTopPoint()))
			continue;

		CPointMap pt = pItem->GetTopPoint();
		iCount ++;
		if ( pScript )
		{
			// This static is at the coordinates in question.
			pScript->Printf( "%i %i %i %i 0\n",
				pItem->GetDispID(), pt.m_x - ptCtr.m_x, pt.m_y - ptCtr.m_y, pt.m_z - zlowest );
		}
		else
		{
			if ( pt.m_z < zlowest)
			{
				zlowest = pt.m_z;
			}
		}
	}

	return iCount;
}

bool CClient::OnTarg_Tile( CObjBase * pObj, const CPointMap & pt )
{
	// CLIMODE_TARG_TILE
	// m_tmTile.m_Code = CV_TILE etc
	// CV_NUKE. CV_NUKECHAR, CV_EXTRACT, CV_NUDGE
	//

	ASSERT(m_pChar);

	if ( pObj && ! pObj->IsTopLevel())
		return( false );
	if ( ! pt.IsValidPoint())
		return( false );

	if ( !m_tmTile.m_ptFirst.IsValidPoint())
	{
		m_tmTile.m_ptFirst = pt;
		addTarget( CLIMODE_TARG_TILE, "Pick other corner:", true );
		return true;
	}

	if ( pt == m_tmTile.m_ptFirst && m_tmTile.m_Code != CV_EXTRACT ) // Extract can work with one square
	{
		SysMessage("Thats the same point.");
		addTarget( CLIMODE_TARG_TILE, "Pick other corner:", true );
		return true;
	}

	CRectMap rect;
	rect.SetRect( m_tmTile.m_ptFirst.m_x, m_tmTile.m_ptFirst.m_y, pt.m_x, pt.m_y );
	CPointMap ptCtr = rect.GetCenter();
	ptCtr.m_mapplane = pt.m_mapplane;

	int rx = 1 + abs( rect.m_right - rect.m_left ) / 2;
	int ry = 1 + abs( rect.m_bottom - rect.m_top ) / 2;
	int iRadius = max( rx, ry );

	int iCount = 0;

	switch ( m_tmTile.m_Code )
	{
	case CV_EXTRACT:
		{
		// "EXTRACT" all map statics in the region.
		// First find the lowest Z to use as a base
		// and count the statics
		int zlowest = UO_SIZE_Z;
		iCount = Cmd_Extract( NULL, rect, zlowest );
		if ( iCount )
		{
			CScript s;
			if ( ! s.Open( m_Targ_Text, OF_WRITE|OF_TEXT ))
				return( false );

			// Write a header for this multi in XXX format
			// (i have no idea what most of this means)
			s.Printf("6 version\n");
			s.Printf("%d template id\n", m_tmTile.m_id );
			s.Printf("-1 item version\n");
			s.Printf("%i num components\n", iCount);

			Cmd_Extract( &s, rect, zlowest );
		}

		SysMessagef( "%d Statics Extracted to '%s', id=%d", iCount, (LPCTSTR) m_Targ_Text, m_tmTile.m_id );
		}
		break;

	case CV_NUDGE:
		{
			TCHAR szTmp[512];
			strcpylen( szTmp, m_Targ_Text, sizeof(szTmp));

			int piArgs[3];		// Maximum parameters in one line
			int iArgQty = Str_ParseCmds( szTmp, piArgs, COUNTOF( piArgs ));

			CPointMap ptNudge(piArgs[0],piArgs[1],piArgs[2] );

			CWorldSearch AreaItem( ptCtr, iRadius );
			AreaItem.SetAllShow( IsPriv( PRIV_ALLSHOW ));
			while (true)
			{
				CItem * pItem = AreaItem.GetItem();
				if ( pItem == NULL )
					break;
				if ( ! rect.IsInside2d( pItem->GetTopPoint()))
					continue;
				CPointMap ptMove = pItem->GetTopPoint();
				ptMove += ptNudge;
				pItem->MoveToCheck( ptMove );
				iCount++;
			}

			CWorldSearch AreaChar( ptCtr, iRadius );
			AreaChar.SetAllShow( IsPriv( PRIV_ALLSHOW ));
			while (true)
			{
				CChar* pChar = AreaChar.GetChar();
				if ( pChar == NULL )
					break;
				if ( ! rect.IsInside2d( pChar->GetTopPoint()))
					continue;
				CPointMap ptMove = pChar->GetTopPoint();
				ptMove += ptNudge;
				pChar->m_fClimbUpdated = false; // update climb height
				pChar->MoveToChar( ptMove );
				iCount++;
			}

			SysMessagef( "%d Objects Nudged", iCount );
		}
		break;

	case CV_NUKE:		// NUKE all items in the region.
		{
			CWorldSearch AreaItem( ptCtr, iRadius );
			AreaItem.SetAllShow( IsPriv( PRIV_ALLSHOW ));
			while (true)
			{
				CItem * pItem = AreaItem.GetItem();
				if ( pItem == NULL )
					break;
				if ( ! rect.IsInside2d( pItem->GetTopPoint()))
					continue;

				if ( m_Targ_Text.IsEmpty())
				{
					pItem->Delete();
				}
				else
				{
					CScript script(m_Targ_Text);
					if ( ! pItem->r_Verb( script, this ))
						continue;
				}
				iCount++;
			}
			SysMessagef( "%d Items Nuked!", iCount );
		}
		break;

	case CV_NUKECHAR:
		{
			CWorldSearch AreaChar( ptCtr, iRadius );
			AreaChar.SetAllShow( IsPriv( PRIV_ALLSHOW ));
			while (true)
			{
				CChar* pChar = AreaChar.GetChar();
				if ( pChar == NULL )
					break;
				if ( ! rect.IsInside2d( pChar->GetTopPoint()))
					continue;
				if ( pChar->m_pPlayer )
					continue;
				if ( m_Targ_Text.IsEmpty())
				{
					pChar->Delete();
				}
				else
				{
					CScript script(m_Targ_Text);
					if ( ! pChar->r_Verb( script, this ))
						continue;
				}
				iCount++;
			}
			SysMessagef( "%d Chars Nuked!", iCount );
		}
		break;

	case CV_TILE:
		{
			TCHAR szTmp[256];
			strcpylen( szTmp, m_Targ_Text, sizeof(szTmp));

			int piArgs[16];		// Maximum parameters in one line
			int iArgQty = Str_ParseCmds( szTmp, piArgs, COUNTOF( piArgs ));

			signed char z = piArgs[0];	// z height is the first arg.
			int iArg = 0;
			for ( int mx = rect.m_left; mx <= rect.m_right; mx++)
			{
				for(int my = rect.m_top; my <= rect.m_bottom; my++)
				{
					if ( ++iArg >= iArgQty )
						iArg = 1;
					CItem * pItem = CItem::CreateTemplate( (ITEMID_TYPE) RES_GET_INDEX(piArgs[iArg]), NULL, m_pChar );
					ASSERT(pItem);
					pItem->SetAttr( ATTR_MOVE_NEVER );
					CPointMap ptCur( mx, my, z, pt.m_mapplane );
					pItem->MoveTo( ptCur );
					iCount++;
				}
			}

			SysMessagef( "%d Items Created", iCount );
		}
		break;
	}

	return true;
}

//-----------------------------------------------------------------------
// Targetted Informational skills

int CClient::OnSkill_AnimalLore( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_ANIMALLORE
	// The creature is a "human" etc..
	// How happy.
	// Well trained.
	// Who owns it ?
	// What it eats.

	// Other "lore" type things about a creature ?
	// ex. liche = the remnants of a powerful wizard

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		SysMessageDefault( "non_alive" );
		return( 1 );
	}

	if ( fTest )
	{
		if ( pChar == m_pChar )
			return( 2 );
		if ( pChar->IsHuman())
			return( Calc_GetRandVal(10));
		return Calc_GetRandVal(60);
	}

	LPCTSTR pszHe = pChar->GetPronoun();
	LPCTSTR pszHis = pChar->GetPossessPronoun();

	CGString sTmp;

	// What kind of animal.
	if ( pChar->IsIndividualName())
	{
		sTmp.Format( g_Cfg.GetDefaultMsg( "animallore_result" ), (LPCTSTR) pChar->GetName(),(LPCTSTR) pChar->Char_GetDef()->GetTradeName());
		addObjMessage( sTmp, pChar );
	}

	// Who is master ?
	CChar * pCharOwner = pChar->NPC_PetGetOwner();
	if ( pCharOwner == NULL )
	{
		sTmp.Format( g_Cfg.GetDefaultMsg( "animallore_free" ), (LPCTSTR) pszHe, (LPCTSTR) pszHis );
	}
	else
	{
		sTmp.Format( g_Cfg.GetDefaultMsg( "animallore_master" ), pszHe, ( pCharOwner == m_pChar ) ? "you" : (LPCTSTR) pCharOwner->GetName());
		// How loyal to master ?
	}
	addObjMessage( sTmp, pChar );

	// How well fed ?
	// Food count = 30 minute intervals.
	LPCTSTR pszText;
	int ifood = pChar-> Stat_GetVal(STAT_FOOD);
	if ( ifood > 7 )
		ifood = 7;
	if ( pChar->IsStatFlag( STATF_Conjured ))
	{
		pszText = g_Cfg.GetDefaultMsg( "animallore_conjured" );
	}
	else
	{
		pszText = pChar->Food_GetLevelMessage( pCharOwner ? true : false, true );
	}

	sTmp.Format( g_Cfg.GetDefaultMsg( "animallore_food" ), (LPCTSTR) pszHe, (LPCTSTR) pszText );
	addObjMessage( sTmp, pChar );

	// How hard to tame ???
	int iTameBase = pChar->Skill_GetBase(SKILL_TAMING);
	int iTameMe = m_pChar->Skill_GetAdjusted(SKILL_TAMING);

	// if ( 

	return 0;
}

int CClient::OnSkill_ItemID( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_ITEMID

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
	{
		return( -1 );
	}

	if ( pObj->IsChar())
	{
		CChar * pChar = STATIC_CAST <CChar*>(pObj);
		ASSERT(pChar);
		if ( fTest )
		{
			return( 1 );
		}

		SysMessagef( g_Cfg.GetDefaultMsg( "itemid_result" ), (LPCTSTR) pChar->GetName());
		return( 1 );
	}

	CItem * pItem = STATIC_CAST <CItem*>(pObj);
	ASSERT( pItem );

	if ( fTest )
	{
		if ( pItem->IsAttr( ATTR_IDENTIFIED ))
		{
			// already identified so easier.
			return Calc_GetRandVal(20);
		}
		return Calc_GetRandVal(60);
	}

#ifdef COMMENT
	if ( pItem->IsAttr(ATTR_MAGIC))
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "item_magic" ) );
	}
	else if ( pItem->IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER))
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "item_newbie" ) );
	}
#endif

	pItem->SetAttr(ATTR_IDENTIFIED);

	// ??? Estimate it's worth ?

	CItemVendable * pItemVend = dynamic_cast <CItemVendable *>(pItem);
	if ( pItemVend == NULL )
	{
		SysMessagef( _TEXT( g_Cfg.GetDefaultMsg( "itemid_noval" ) ));
	}
	else
	{
		SysMessagef( _TEXT( g_Cfg.GetDefaultMsg( "itemid_gold" ) ),
			pItemVend->GetVendorPrice(-15), (LPCTSTR) pItemVend->GetNameFull(true));
	}

	// Whats it made of ?

	CItemBase * pItemDef = pItem->Item_GetDef();
	ASSERT(pItemDef);

	if ( iSkillLevel > 40 && pItemDef->m_BaseResources.GetCount())
	{
		TCHAR szTemp[ SCRIPT_MAX_LINE_LEN ];
		int iLen = sprintf( szTemp, g_Cfg.GetDefaultMsg( "itemid_madeof" ) );
		pItemDef->m_BaseResources.WriteNames( szTemp+iLen );
		SysMessage( szTemp );
	}

	// It required what skills to make ?
	// "It requires lots of mining skill"

	return iSkillLevel;
}

int CClient::OnSkill_EvalInt( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_EVALINT
	// iSkillLevel = 0 to 1000
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		SysMessageDefault( "non_alive" );
		return( 1 );
	}

	if ( fTest )
	{
		if ( pChar == m_pChar )
			return( 2 );
		return Calc_GetRandVal(60);
	}

	static LPCTSTR const sm_szIntDesc[] =
	{
		"evalint_int_1",
		"evalint_int_2",
		"evalint_int_3",
		"evalint_int_4",
		"evalint_int_5",
		"evalint_int_6",
		"evalint_int_7",
		"evalint_int_8",
		"evalint_int_9",
		"evalint_int_10",
	};

	int iIntVal = pChar->Stat_GetAdjusted(STAT_INT);
	int iIntEntry = (iIntVal-1) / 10;
	if ( iIntEntry < 0 )
		iIntEntry = 0;
	if ( iIntEntry >= COUNTOF( sm_szIntDesc ))
		iIntEntry = COUNTOF( sm_szIntDesc )-1;

	SysMessagef( g_Cfg.GetDefaultMsg( "evalint_result" ), (LPCTSTR) pChar->GetName(), (LPCTSTR) g_Cfg.GetDefaultMsg( sm_szIntDesc[iIntEntry] ) );

	static LPCTSTR const sm_szMagicDesc[] =
	{
		"evalint_mag_1",
		"evalint_mag_2",
		"evalint_mag_3",
		"evalint_mag_4",
		"evalint_mag_5",
		"evalint_mag_6",
	};

	static LPCTSTR const sm_szManaDesc[] =
	{
		"evalint_man_1",
		"evalint_man_2",
		"evalint_man_3",
		"evalint_man_4",
		"evalint_man_5",	// 100 %
		"evalint_man_6",
	};

	if ( iSkillLevel > 400 )	// magery skill and mana level ?
	{
		int iMagerySkill = pChar->Skill_GetAdjusted(SKILL_MAGERY);
		int iNecroSkill = pChar->Skill_GetAdjusted(SKILL_NECROMANCY);
		int iMagicSkill = max(iMagerySkill,iNecroSkill);

		int iMagicEntry = iMagicSkill / 200;
		if ( iMagicEntry < 0 )
			iMagicEntry = 0;
		if ( iMagicEntry >= COUNTOF(sm_szMagicDesc))
			iMagicEntry = COUNTOF(sm_szMagicDesc)-1;

		int iManaEntry = IMULDIV( pChar->Stat_GetVal(STAT_INT), COUNTOF(sm_szManaDesc)-1, iIntVal );
		if ( iManaEntry < 0 )
			iManaEntry = 0;
		if ( iManaEntry >= COUNTOF(sm_szManaDesc))
			iManaEntry = COUNTOF(sm_szManaDesc)-1;

		SysMessagef( g_Cfg.GetDefaultMsg( "evalint_result_2" ),
			(LPCTSTR) g_Cfg.GetDefaultMsg( sm_szMagicDesc[iMagicEntry] ), (LPCTSTR) g_Cfg.GetDefaultMsg( sm_szManaDesc[iManaEntry] ) );
	}

	return iSkillLevel;
}
static LPCTSTR const sm_szPoisonMessages[] =
{
	"armslore_psn_1",
	"armslore_psn_2",
	"armslore_psn_3",
	"armslore_psn_4",
	"armslore_psn_5",
	"armslore_psn_6",
	"armslore_psn_7",
	"armslore_psn_8",
	"armslore_psn_9",
	"armslore_psn_10",
};

int CClient::OnSkill_ArmsLore( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_ARMSLORE
	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL || ! pItem->IsTypeArmorWeapon())
	{
notweapon:
		SysMessageDefault( "armslore_unable" );
		return( -SKTRIG_QTY );
	}

// MAKE SURE YOU ACCESS THESE AS g_Cfg.GetDefaultMsg( sm_szXXXMessages[i] ) !!!!
static LPCTSTR const sm_szAttackMessages[] =
{
	"armslore_dam_1",
	"armslore_dam_2",
	"armslore_dam_3",
	"armslore_dam_4",
	"armslore_dam_5",
	"armslore_dam_6",
	"armslore_dam_7",
	"armslore_dam_8",
	"armslore_dam_9",
	"armslore_dam_10",
};

static LPCTSTR const sm_szDefenseMessages[] =
{
	"armslore_def_1",
	"armslore_def_2",
	"armslore_def_3",
	"armslore_def_4",
	"armslore_def_5",
	"armslore_def_6",
	"armslore_def_7",
	"armslore_def_8",
	"armslore_def_9",
	"armslore_def_10",
};

	TCHAR szTemp[ SCRIPT_MAX_LINE_LEN ];
	int len = 0;
	bool fWeapon;
	int iHitsCur;
	int iHitsMax;

	if ( fTest )
	{
		return Calc_GetRandVal(60);
	}

	switch ( pItem->GetType() )
	{
	case IT_ARMOR:				// some type of armor. (no real action)
	case IT_SHIELD:
	case IT_ARMOR_LEATHER:
	case IT_CLOTHING:
	case IT_JEWELRY:
		fWeapon = false;
		iHitsCur = pItem->m_itArmor.m_Hits_Cur;
		iHitsMax = pItem->m_itArmor.m_Hits_Max;
		len += sprintf( szTemp+len, g_Cfg.GetDefaultMsg( "armslore_def" ), pItem->Armor_GetDefense());
		break;
	case IT_WEAPON_MACE_CROOK:
	case IT_WEAPON_MACE_PICK:
	case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case IT_WEAPON_MACE_STAFF:
	case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case IT_WEAPON_SWORD:
	case IT_WEAPON_AXE:
	case IT_WEAPON_FENCE:
	case IT_WEAPON_BOW:
	case IT_WEAPON_XBOW:
		fWeapon = true;
		iHitsCur = pItem->m_itWeapon.m_Hits_Cur;
		iHitsMax = pItem->m_itWeapon.m_Hits_Max;
		len += sprintf( szTemp+len, g_Cfg.GetDefaultMsg( "armslore_dam" ), pItem->Weapon_GetAttack());
		break;
	default:
		goto notweapon;
	}

	len += sprintf( szTemp+len, g_Cfg.GetDefaultMsg( "armslore_rep" ), pItem->Armor_GetRepairDesc());

	if ( iHitsCur <= 3 || iHitsMax <= 3 )
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "armslore_rep_0" ) );
	}

	// Magical effects ?
	if ( pItem->IsAttr(ATTR_MAGIC))
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "item_magic" ) );
	}
	else if ( pItem->IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER))
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "item_newbie" ) );
	}

	// Repairable ?
	if ( ! pItem->Armor_IsRepairable())
	{
		len += strcpylen( szTemp+len, g_Cfg.GetDefaultMsg( "item_repair" ) );
	}

	// Poisoned ?
	if ( fWeapon && pItem->m_itWeapon.m_poison_skill )
	{
		int iLevel = IMULDIV( pItem->m_itWeapon.m_poison_skill, COUNTOF(sm_szPoisonMessages), 100 );
		if ( iLevel < 0 )
			iLevel = 0;
		if ( iLevel >= COUNTOF(sm_szPoisonMessages))
			iLevel = COUNTOF(sm_szPoisonMessages)-1;
		len += sprintf( szTemp+len, " %s", g_Cfg.GetDefaultMsg( sm_szPoisonMessages[iLevel]) );
	}

	SysMessage(szTemp);
	return iSkillLevel;
}

int CClient::OnSkill_Anatomy( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_ANATOMY
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		addObjMessage( g_Cfg.GetDefaultMsg( "non_alive" ), pChar );
		return( 1 );
	}

	if ( fTest )
	{
		// based on rareity ?
		if ( pChar == m_pChar ) 
			return( 2 );
		return Calc_GetRandVal(60);
	}

	// Add in error cased on your skill level.

	static LPCTSTR const sm_szStrEval[] =
	{
		"anatomy_str_1",
		"anatomy_str_2",
		"anatomy_str_3",
		"anatomy_str_4",
		"anatomy_str_5",
		"anatomy_str_6",
		"anatomy_str_7",
		"anatomy_str_8",
		"anatomy_str_9",
		"anatomy_str_10",
	};
	static LPCTSTR const sm_szDexEval[] =
	{
		"anatomy_dex_1",
		"anatomy_dex_2",
		"anatomy_dex_3",
		"anatomy_dex_4",
		"anatomy_dex_5",
		"anatomy_dex_6",
		"anatomy_dex_7",
		"anatomy_dex_8",
		"anatomy_dex_9",
		"anatomy_dex_10",
	};

	int iStrVal = pChar->Stat_GetAdjusted(STAT_STR);
	int iStrEntry = (iStrVal-1)/10;
	if ( iStrEntry < 0 )
		iStrEntry = 0;
	if ( iStrEntry >= COUNTOF( sm_szStrEval ))
		iStrEntry = COUNTOF( sm_szStrEval )-1;

	int iDexVal = pChar->Stat_GetAdjusted(STAT_DEX);
	int iDexEntry = (iDexVal-1)/10;
	if ( iDexEntry < 0 )
		iDexEntry = 0;
	if ( iDexEntry >= COUNTOF( sm_szDexEval ))
		iDexEntry = COUNTOF( sm_szDexEval )-1;

	CGString sTmp;
	sTmp.Format( g_Cfg.GetDefaultMsg( "anatomy_result" ), (LPCTSTR) pChar->GetName(),
		g_Cfg.GetDefaultMsg( sm_szStrEval[iStrEntry] ), g_Cfg.GetDefaultMsg( sm_szDexEval[iDexEntry] ) );
	addObjMessage( sTmp, pChar );

	if ( pChar->IsStatFlag( STATF_Conjured ))
	{
		addObjMessage( g_Cfg.GetDefaultMsg( "anatomy_magic" ), pChar );
	}

	// ??? looks hungry ?
	return iSkillLevel;
}

int CClient::OnSkill_Forensics( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_FORENSICS
	// ! weird client issue targetting corpses !

	CItemCorpse * pCorpse = dynamic_cast <CItemCorpse *>( uid.ItemFind());
	if ( pCorpse == NULL )
	{
		SysMessageDefault( "forensics_corpse" );
		return( -SKTRIG_QTY );
	}

	if ( ! m_pChar->CanTouch( pCorpse ))
	{
		SysMessageDefault( "forensics_reach" );
		return( -SKTRIG_QTY );
	}

	if ( fTest )
	{
		if ( pCorpse->m_uidLink == m_pChar->GetUID() )
			return( 2 );
		return Calc_GetRandVal(60);
	}

	CGrayUID uidKiller( pCorpse->m_itCorpse.m_uidKiller );
	CChar * pCharKiller = uidKiller.CharFind();
	LPCTSTR pName = ( pCharKiller != NULL ) ? pCharKiller->GetName() : NULL;

	if ( pCorpse->IsCorpseSleeping())
	{
		// "They are not dead but mearly unconscious"
		SysMessagef( g_Cfg.GetDefaultMsg( "forensics_alive" ), pName ? pName : "It" );
		return 1;
	}

	TCHAR szTemp[ SCRIPT_MAX_LINE_LEN ];
	if ( pCorpse->m_itCorpse.m_timeDeath.IsTimeValid() )
	{
		int len = sprintf( szTemp, g_Cfg.GetDefaultMsg( "forensics_timer" ),
			pCorpse->GetName(), ( - g_World.GetTimeDiff( pCorpse->m_itCorpse.m_timeDeath )) / TICK_PER_SEC );
		if ( pName == NULL )
		{
			strcpy( szTemp+len, g_Cfg.GetDefaultMsg( "forensics_failname" ) );
		}
		else
		{
			sprintf( szTemp+len, g_Cfg.GetDefaultMsg( "forensics_name" ), pName );
		}
	}
	else
	{
		int len = sprintf( szTemp, g_Cfg.GetDefaultMsg( "forensics_carve_1" ),
			pCorpse->GetName());
		if ( pName == NULL )
		{
			strcpy( szTemp+len, g_Cfg.GetDefaultMsg( "forensics_failname" ) );
		}
		else
		{
			sprintf( szTemp+len, g_Cfg.GetDefaultMsg( "forensics_carve_2" ), pName );
		}
	}
	SysMessage( szTemp );
	return iSkillLevel;
}

int CClient::OnSkill_TasteID( CGrayUID uid, int iSkillLevel, bool fTest )
{
	// SKILL_TASTEID
	// Try to taste for poisons I assume.
	// Maybe taste what it is made of for ingredients ?
	// Differntiate potion types ?

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL )
	{
		if ( uid == m_pChar->GetUID())
		{
			SysMessageDefault( "tasteid_self" );
		}
		else
		{
			SysMessageDefault( "tasteid_char" );
		}
		return( -SKTRIG_QTY );
	}

	if ( ! m_pChar->CanUse( pItem, true ))
	{
		SysMessageDefault( "tasteid_unable" );
		return( -SKTRIG_QTY );
	}

	int iPoisonLevel = 0;
	switch ( pItem->GetType())
	{
	case IT_POTION:
		if ( RES_GET_INDEX(pItem->m_itPotion.m_Type) == SPELL_Poison )
		{
			iPoisonLevel = pItem->m_itPotion.m_skillquality;
		}
		break;
	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		iPoisonLevel = pItem->m_itFood.m_poison_skill*10;
		break;
	case IT_WEAPON_MACE_SHARP:
	case IT_WEAPON_SWORD:		// 13 =
	case IT_WEAPON_FENCE:		// 14 = can't be used to chop trees. (make kindling)
	case IT_WEAPON_AXE:
		// pItem->m_itWeapon.m_poison_skill = pPoison->m_itPotion.m_skillquality / 10;
		iPoisonLevel = pItem->m_itWeapon.m_poison_skill*10;
		break;
	default:
		if ( ! fTest )
		{
			SysMessagef( g_Cfg.GetDefaultMsg( "tasteid_result" ), (LPCTSTR) pItem->GetNameFull(false) );
		}
		return 1;
	}

	if ( fTest )
	{
		return Calc_GetRandVal(60);
	}

	if ( iPoisonLevel )
	{
		int iLevel = IMULDIV( iPoisonLevel, COUNTOF(sm_szPoisonMessages), 1000 );
		if ( iLevel < 0 )
			iLevel = 0;
		if ( iLevel >= COUNTOF(sm_szPoisonMessages))
			iLevel = COUNTOF(sm_szPoisonMessages)-1;
		SysMessage( sm_szPoisonMessages[iLevel] );
	}
	else
	{
		SysMessagef( g_Cfg.GetDefaultMsg( "tasteid_result" ), (LPCTSTR) pItem->GetNameFull(false) );
	}

	return iSkillLevel;
}

int CClient::OnSkill_Info( SKILL_TYPE skill, CGrayUID uid, int iSkillLevel, bool fTest )
{
	// Skill timer has expired.
	// RETURN: difficulty credit. 0-100
	//  <0 = immediate failure.
	switch ( skill )
	{
	case SKILL_ANIMALLORE:	return OnSkill_AnimalLore( uid, iSkillLevel, fTest );
	case SKILL_ARMSLORE:	return OnSkill_ArmsLore( uid, iSkillLevel, fTest );
	case SKILL_ANATOMY:		return OnSkill_Anatomy( uid, iSkillLevel, fTest );
	case SKILL_ITEMID:		return OnSkill_ItemID( uid, iSkillLevel, fTest );
	case SKILL_EVALINT:		return OnSkill_EvalInt( uid, iSkillLevel, fTest );
	case SKILL_FORENSICS:	return OnSkill_Forensics( uid, iSkillLevel, fTest );
	case SKILL_TASTEID:		return OnSkill_TasteID( uid, iSkillLevel, fTest );
	}

	DEBUG_CHECK(0);
	return( -SKTRIG_QTY );
}

////////////////////////////////////////
// Targeted skills and actions.

bool CClient::OnTarg_Skill( CObjBase * pObj )
{
	// targetted skill now has it's target.
	// response to CLIMODE_TARG_SKILL
	// from Event_Skill_Use() select button from skill window

	if ( pObj == NULL )
		return( false );

	bool fContinue = false;

	SetTargMode();	// just make sure last targ mode is gone.
	m_Targ_UID = pObj->GetUID();	// keep for 'last target' info.

	// targetting what skill ?
	switch ( m_tmSkillTarg.m_Skill )
	{
		// Delayed response type skills.
	case SKILL_BEGGING:
		m_pChar->UpdateAnimate( ANIM_BOW );

	case SKILL_STEALING:
	case SKILL_TAMING:
	case SKILL_ENTICEMENT:
	case SKILL_STEALTH:

		// Informational skills. (instant return)
	case SKILL_ANIMALLORE:
	case SKILL_ARMSLORE:
	case SKILL_ANATOMY:
	case SKILL_ITEMID:
	case SKILL_EVALINT:
	case SKILL_FORENSICS:
	case SKILL_TASTEID:
		m_pChar->m_Act_Targ = m_Targ_UID;
		return( m_pChar->Skill_Start( m_tmSkillTarg.m_Skill ));

	case SKILL_PROVOCATION:
		if ( ! pObj->IsChar())
		{
			SysMessageDefault( "provoke_unable" );
			return( false );
		}
		addTarget( CLIMODE_TARG_SKILL_PROVOKE, g_Cfg.GetDefaultMsg( "provoke_select" ), false, true );
		break;

	case SKILL_POISONING:
		// We now have to find the poison.
		addTarget( CLIMODE_TARG_SKILL_POISON, g_Cfg.GetDefaultMsg( "poisoning_select_1" ), false, true );
		break;

	default:
		// This is not a targetted skill !
		break;
	}

	return true;
}

bool CClient::OnTarg_Skill_Provoke( CObjBase * pObj )
{
	// CLIMODE_TARG_SKILL_PROVOKE
	if ( pObj == NULL )
		return( false );
	if ( ! pObj->IsChar() )
	{
		SysMessageDefault( "provoke_unable" );
		return( false );
	}
	m_pChar->m_Act_TargPrv = m_Targ_UID;	// provoke him
	m_pChar->m_Act_Targ = pObj->GetUID();				// against him
	return( m_pChar->Skill_Start( SKILL_PROVOCATION ));
}

bool CClient::OnTarg_Skill_Poison( CObjBase * pObj )
{
	// CLIMODE_TARG_SKILL_POISON
	if ( pObj == NULL )
		return( false );
	m_pChar->m_Act_TargPrv = m_Targ_UID;	// poison this
	m_pChar->m_Act_Targ = pObj->GetUID();				// with this poison
	return( m_pChar->Skill_Start( SKILL_POISONING ));
}

bool CClient::OnTarg_Skill_Herd_Dest( CObjBase * pObj, const CPointMap & pt )
{
	// CLIMODE_TARG_SKILL_HERD_DEST

	m_pChar->m_Act_p = pt;
	m_pChar->m_Act_Targ = m_Targ_UID;	// Who to herd?
	m_pChar->m_Act_TargPrv = m_Targ_PrvUID; // crook ?

	return( m_pChar->Skill_Start( SKILL_HERDING ));
}

bool CClient::OnTarg_Skill_Magery( CObjBase * pObj, const CPointMap & pt )
{
	// The client player has targeted a spell.
	// CLIMODE_TARG_SKILL_MAGERY
	
	CSpellDef *		pSpell	= g_Cfg.GetSpellDef( m_tmSkillMagery.m_Spell );

	if ( ! pSpell )		return false;

	if ( pObj )
	{
		if ( !pSpell->IsSpellType( SPELLFLAG_TARG_OBJ ) )
		{
			SysMessageDefault( "magery_4" );
			return true;
		}

		if ( pObj->IsItem() && !pSpell->IsSpellType( SPELLFLAG_TARG_ITEM ) )
		{
			SysMessageDefault( "magery_1" );
			return true;
		}
		if ( pObj->IsChar() && !pSpell->IsSpellType( SPELLFLAG_TARG_CHAR ) )
		{
			SysMessageDefault( "magery_2" );
			return true;
		}
		if (	pObj == (CObjBase*) m_pChar
			&&	pSpell->IsSpellType( SPELLFLAG_TARG_NOSELF )
			&&	!IsPriv(PRIV_GM) )
		{
			SysMessageDefault( "magery_3" );
			return true;
		}
	}

	if ( m_tmSkillMagery.m_Spell == SPELL_Polymorph )
	{
		return Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, "sm_polymorph" ));
	}

	m_pChar->m_atMagery.m_Spell			= m_tmSkillMagery.m_Spell;
	m_pChar->m_atMagery.m_SummonID		= m_tmSkillMagery.m_SummonID;
	m_pChar->m_atMagery.m_fSummonPet	= m_tmSkillMagery.m_fSummonPet;

	m_pChar->m_Act_TargPrv				= m_Targ_PrvUID;	// Source (wand or you?)
	m_pChar->m_Act_Targ					= pObj ? (DWORD) pObj->GetUID() : UID_CLEAR ;
	m_pChar->m_Act_p					= pt;
	m_Targ_p							= pt;

	if ( g_Cfg.IsSetOF( OF_Magic_PreCast ) )
		return( m_pChar->Spell_CastDone() );
	else
		return( m_pChar->Skill_Start( SKILL_MAGERY ));
}

bool CClient::OnTarg_Pet_Command( CObjBase * pObj, const CPointMap & pt )
{
	// CLIMODE_TARG_PET_CMD
	// Any pet command requiring a target.
	// m_Targ_UID = the pet i was instructing.
	// m_tmPetCmd = command index.
	// m_Targ_Text = the args to the command that got us here.

	if ( m_tmPetCmd.m_fAllPets )
	{
		// All the pets that could hear me.
		bool fGhostSpeak = m_pChar->IsSpeakAsGhost();

		CWorldSearch AreaChars( m_pChar->GetTopPoint(), UO_MAP_VIEW_SIGHT );
		while (true)
		{
			CChar * pCharPet = AreaChars.GetChar();
			if ( pCharPet == NULL )
				break;
			if ( pCharPet == m_pChar )
				continue;
			if ( fGhostSpeak && ! pCharPet->CanUnderstandGhost())
				continue;
			if ( ! pCharPet->NPC_IsOwnedBy( GetChar(), true ))
				continue;
			pCharPet->NPC_OnHearPetCmdTarg( m_tmPetCmd.m_iCmd, GetChar(), pObj, pt, m_Targ_Text );
		}
		return( true );
	}
	else
	{
		CChar * pCharPet = m_Targ_UID.CharFind();
		if ( pCharPet == NULL )
			return false;
		return pCharPet->NPC_OnHearPetCmdTarg( m_tmPetCmd.m_iCmd, GetChar(), pObj, pt, m_Targ_Text );
	}
}

bool CClient::OnTarg_Pet_Stable( CChar * pCharPet )
{
	// CLIMODE_PET_STABLE
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
		SysMessageDefault( "npc_stablemaster_select" );
		return( false );
	}

	if ( ! pCharMaster->CanSee(pCharPet))
	{
		pCharMaster->Speak( g_Cfg.GetDefaultMsg( "npc_stablemaster_los" ) );
		return( false );
	}

	// Shink the pet and put it in the bank box of the stable master.
	CItem * pPetItem = pCharPet->Make_Figurine( m_pChar->GetUID());
	if ( pPetItem == NULL )
	{
		pCharMaster->Speak( g_Cfg.GetDefaultMsg( "npc_stablemaster_fail" ) );
		return( false );
	}

	pCharMaster->GetBank()->ContentAdd( pPetItem );

	CGString sMsg;
	sMsg.Format( g_Cfg.GetDefaultMsg( "npc_stablemaster_rem" ),
		(LPCTSTR) pCharMaster->GetName());
	pCharMaster->Speak( sMsg );
	return( true );
}

//-----------------------------------------------------------------------
// Targetted items with special props.

bool CClient::OnTarg_Use_Deed( CItem * pDeed, const CPointMap & pt )
{
	// Place the house/ship here. (if possible)
	// Can the structure go here ?
	// IT_DEED
	//

	if ( ! m_pChar->CanUse(pDeed, true ))
		return( false );

	const CItemBase * pItemDef = CItemBase::FindItemBase( (ITEMID_TYPE) RES_GET_INDEX( pDeed->m_itDeed.m_Type ));

	if ( ! OnTarg_Use_Multi( pItemDef, pt, pDeed->m_Attr, pDeed->GetHue() ))
		return( false );

	pDeed->Delete();	// consume the deed.
	return true;
}

CItem * CClient::OnTarg_Use_Multi( const CItemBase * pItemDef, const CPointMap & pt, WORD wAttr, HUE_TYPE wHue )
{
	// Might be a IT_MULTI or it might not. place it anyhow.

	if ( pItemDef == NULL )
		return( NULL );

	bool fShip = ( pItemDef->IsType(IT_SHIP));	// must be in water.

	const CItemBaseMulti * pMultiDef = dynamic_cast <const CItemBaseMulti *> ( pItemDef );

	// Check water/mountains/etc.
	if ( pMultiDef != NULL && ! (wAttr&ATTR_MAGIC))
	{
		// Check for items in the way and bumpy terrain.

		CGRect rect = pMultiDef->m_rect;
		rect.OffsetRect( pt.m_x, pt.m_y );
		CPointMap ptn = pt;

		int x=rect.m_left;
		for ( ; x <=rect.m_right; x++ )
		{
			ptn.m_x = x;
			int y=rect.m_top;
			for ( ; y<=rect.m_bottom; y++ );
			{
				ptn.m_y = y;

				CRegionBase * pRegion = ptn.GetRegion( REGION_TYPE_MULTI | REGION_TYPE_AREA | REGION_TYPE_ROOM );
				if ( pRegion == NULL || (( pRegion->IsFlag( REGION_FLAG_NOBUILDING ) && ! fShip ) && ! IsPriv(PRIV_GM)))
				{
					SysMessageDefault( "itemuse_multi_fail" );
					if ( ! IsPriv( PRIV_GM ))
						return( NULL );
				}

				WORD wBlockFlags = ( fShip ) ? CAN_C_SWIM : CAN_C_WALK;
				ptn.m_z = g_World.GetHeightPoint( ptn, wBlockFlags, true );
				if ( abs( ptn.m_z - pt.m_z ) > 4 )
				{
					SysMessageDefault( "itemuse_multi_bump" );
					if ( ! IsPriv( PRIV_GM ))
						return( NULL );
				}
				if ( fShip )
				{
					if ( ! ( wBlockFlags & CAN_I_WATER ))
					{
						SysMessageDefault( "itemuse_multi_shipw" );
						if ( ! IsPriv( PRIV_GM ))
							return( NULL );
					}
				}
				else
				{
					if ( wBlockFlags & ( CAN_I_WATER | CAN_I_BLOCK | CAN_I_CLIMB ))
					{
						SysMessageDefault( "itemuse_multi_blocked" );
						if ( ! IsPriv( PRIV_GM ))
							return( NULL );
					}
				}
			}
		}

		// Check for chars in the way.

		CWorldSearch Area( pt, UO_MAP_VIEW_SIZE );
		while (true)
		{
			CChar * pChar = Area.GetChar();
			if ( pChar == NULL )
				break;
			if ( pChar == m_pChar )
				continue;
			if ( ! rect.IsInside2d( pChar->GetTopPoint()))
				continue;

			SysMessagef( g_Cfg.GetDefaultMsg( "itemuse_multi_intway" ), (LPCTSTR) pChar->GetName());
			return( NULL );
		}
	}

	CItem * pItemNew = CItem::CreateTemplate( pItemDef->GetID(), NULL, m_pChar );
	if ( pItemNew == NULL )
	{
		SysMessageDefault( "itemuse_multi_collapse" );
		return( NULL );
	}

	pItemNew->SetAttr( wAttr & ( ATTR_MAGIC | ATTR_INVIS ));
	pItemNew->SetHue( wHue );
	pItemNew->MoveTo( pt );

	CItemMulti * pMultiItem = dynamic_cast <CItemMulti*>(pItemNew);
	if ( pMultiItem )
	{
		pMultiItem->Multi_Create( m_pChar, UID_CLEAR );
	}

	if ( pItemDef->IsType(IT_STONE_GUILD))
	{
		// Now name the guild
		m_Targ_UID = pItemNew->GetUID();
		addPromptConsole( CLIMODE_PROMPT_STONE_NAME, g_Cfg.GetDefaultMsg( "itemuse_guildstone_new" ) );
	}
	else if ( fShip )
	{
		pItemNew->Sound( Calc_GetRandVal(2)? 0x12:0x13 );
	}

	return pItemNew;
}

bool CClient::OnTarg_Use_Item( CObjBase * pObjTarg, CPointMap & pt, ITEMID_TYPE id )
{
	// CLIMODE_TARG_USE_ITEM started from Event_DoubleClick()
	// Targetted an item to be used on some other object (char or item).
	// Solve for the intersection of the items.
	// ARGS:
	//  id = static item id
	//  uid = the target.
	//  m_Targ_UID = what is the used object on the target
	//
	// NOTE:
  	//  Assume we can see the target.
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

	m_Targ_PrvUID = m_Targ_UID;	// used item.
	m_Targ_p = pt;

	ITRIG_TYPE trigtype;
	if ( pObjTarg == NULL )
	{
		m_Targ_UID.ClearUID();
		if ( pt.IsCharValid())
		{
			m_pChar->UpdateDir(pt);
		}
		// CScriptTriggerArgs Args( x, y, z );
		trigtype = ITRIG_TARGON_GROUND;
	}
	else 
	{
		m_Targ_UID = pObjTarg->GetUID();
		m_pChar->UpdateDir(pObjTarg);
		if ( pObjTarg->IsChar() )
		{
			trigtype = ITRIG_TARGON_CHAR;
		}
		else
		{
			trigtype = ITRIG_TARGON_ITEM;
		}
	}

	CScriptTriggerArgs	Args( id, 0, pObjTarg );
	if ( pItemUse->OnTrigger( trigtype, m_pChar, &Args ) == TRIGRET_RET_TRUE )
	{
		return true;
	}

	// NOTE: We have NOT checked to see if the targetted object is within reasonable distance.
	// Call CanUse( pItemTarg )

	// What did i target it on ? this could be null if ground is the target.
	CChar * pCharTarg = dynamic_cast <CChar*>(pObjTarg);
	CItem * pItemTarg = dynamic_cast <CItem*>(pObjTarg);

	switch ( pItemUse->GetType() )
	{
	case IT_COMM_CRYSTAL:
		if ( pItemTarg == NULL )
			return( false );
		if ( ! pItemTarg->IsType(IT_COMM_CRYSTAL))
			return( false );
		pItemUse->m_uidLink = pItemTarg->GetUID();
		pItemUse->Speak( "Linked" );
		return( true );

	case IT_POTION:
		// Use a potion on something else.
		if ( RES_GET_INDEX(pItemUse->m_itPotion.m_Type) == SPELL_Poison )
		{
			// ??? If the item is a poison ask them what they want to use the poison on ?
			// Poisoning or poison self ?
		}
		else if ( RES_GET_INDEX(pItemUse->m_itPotion.m_Type) == SPELL_Explosion )
		{
			// Throw explode potion.
			if ( ! pItemUse->IsItemEquipped() || pItemUse->GetEquipLayer() != LAYER_DRAGGING )
				return( false );

			// Put at destination location.
			CPointMap ptBlock;
			if ( m_pChar->CanSeeLOS( pt, &ptBlock, UO_MAP_VIEW_SIZE ))	// Get the block point.
				ptBlock = pt;
			pItemUse->MoveTo( ptBlock );	// leave the decay as it is.
			pItemUse->Effect( EFFECT_BOLT, pItemUse->GetDispID(), m_pChar, 7, 0, false );
		}
		return( true );

	case IT_MEAT_RAW:
	case IT_FOOD_RAW:
		// Try to put it on some sort of fire.

		switch ( m_pChar->CanTouchStatic( pt, id, pItemTarg ))
		{
		case IT_JUNK:
			SysMessageDefault( "itemuse_foodraw_touch" );
			return( false );
		case IT_FIRE:
		case IT_FORGE:
		case IT_CAMPFIRE:
			// Start cooking skill.
			m_pChar->m_Act_Targ = m_Targ_PrvUID;
			m_pChar->m_Act_p = pt;
			m_pChar->Skill_Start( SKILL_COOKING );
			return( true );
		default:
			if ( pCharTarg == m_pChar && m_pChar->Use_Eat( pItemUse ))
			{
				return( true );
			}
			// static fire ?
			SysMessageDefault( "itemuse_foodraw_use" );
			return( false );
		}
		break;

	case IT_KEY:
		return( m_pChar->Use_Key( pItemUse, pItemTarg ));

	case IT_FORGE:
		// target the ore.
		return( m_pChar->Skill_Mining_Smelt( pItemTarg, pItemUse ));

	case IT_CANNON_MUZZLE:
		// We have targetted the cannon to something.
		if ( ( pItemUse->m_itCannon.m_Load & 3 ) != 3 )
		{
			if ( m_pChar->Use_Cannon_Feed( pItemUse, pItemTarg ))
			{
				pItemTarg->ConsumeAmount();
			}
			return( true );
		}

		// Fire!
		pItemUse->m_itCannon.m_Load = 0;
		pItemUse->Sound( 0x207 );
		pItemUse->Effect( EFFECT_OBJ, ITEMID_FX_TELE_VANISH, pItemUse, 9, 6 );

		// just explode on the ground ?
		if ( pObjTarg != NULL )
		{
			// Check distance and LOS.
#if 0
			// CPointMap ptDst;
			if ( ! pItemUse->CanSeeLOS( pObjTarg ))
				return( true );
#endif

			if ( pItemUse->GetDist( pObjTarg ) > UO_MAP_VIEW_SIZE )
			{
				SysMessageDefault( "itemuse_toofar" );
				return( true );
			}

			// Hit the Target !
			pObjTarg->Sound( 0x207 );
			pObjTarg->Effect( EFFECT_BOLT, ITEMID_Cannon_Ball, pItemUse, 8, 0, true );
			pObjTarg->OnTakeDamage( 80 + Calc_GetRandVal( 150 ), m_pChar, DAMAGE_HIT_BLUNT | DAMAGE_FIRE );
		}
		return( true );

	case IT_WEAPON_MACE_PICK:
		// Mine at the location. (shovel)
		m_pChar->m_Act_p = pt;
		m_pChar->m_Act_TargPrv = m_Targ_PrvUID;
		return( m_pChar->Skill_Start( SKILL_MINING ));

	case IT_WEAPON_MACE_CROOK:
		// SKILL_HERDING
		// Selected a creature to herd
		// Move the selected item or char to this location.
		if ( pCharTarg == NULL )
		{
			// hit it ?
			SysMessageDefault( "itemuse_crook_try" );
			return( false );
		}
		addTarget( CLIMODE_TARG_SKILL_HERD_DEST, "Where do you want them to go ?", true );
		return( true );

	case IT_WEAPON_MACE_SMITH:		// Can be used for smithing.
		// Near a forge ? smithing ?
		if ( pItemTarg == NULL )
			break;
		if ( pItemTarg->IsType(IT_INGOT) )
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

	case IT_CARPENTRY_CHOP:		// Carpentry type tool
	case IT_WEAPON_MACE_SHARP:// 22 = war axe can be used to cut/chop trees.
	case IT_WEAPON_FENCE:		// 24 = can't be used to chop trees.
	case IT_WEAPON_AXE:
	case IT_WEAPON_SWORD:		// 23 =

		// Use sharp weapon on something.
		if ( pCharTarg != NULL )
		{
			// on some person ?
			if ( ! m_pChar->CanTouch(pCharTarg) )
				return( false );
			switch ( pCharTarg->GetID())
			{
			case CREID_Sheep: // Sheep have wool.
				// Get the wool.
				{
					CItem *pWool = CItem::CreateBase( ITEMID_WOOL );
					ASSERT(pWool);
					m_pChar->ItemBounce(pWool);
					pCharTarg->SetID(CREID_Sheep_Sheered);
					// Set wool to regrow.
					pWool = CItem::CreateBase( ITEMID_WOOL );
					ASSERT(pWool);
					pWool->SetTimeout( g_Cfg.m_iWoolGrowthTime );
					pCharTarg->LayerAdd( pWool, LAYER_FLAG_Wool );
					pCharTarg->Update();
				}
				return true;
			case CREID_Sheep_Sheered:
				SysMessageDefault( "itemuse_weapon_wwait" );
				return true;
			default:
				// I suppose this is an attack ?
				break;
			}
			break;
		}


		switch ( m_pChar->CanTouchStatic( pt, id, pItemTarg ))
		{
		case IT_JUNK:
			SysMessageDefault( "itemuse_junk_reach" );
			return false;
		case IT_FOLIAGE:
		case IT_TREE:
			// Just targetted a tree type
			if ( pItemUse->IsType(IT_WEAPON_FENCE) )
			{
				CItem * pResBit = g_World.CheckNaturalResource( pt, 
					(IT_TYPE) GETINTRESOURCE(m_pChar->m_atResource.m_ridType), false, m_pChar );
				if ( pResBit == NULL )
				{
					SysMessageDefault( "lumberjacking_3" );
					return false;
				}
				if ( pResBit->ConsumeAmount(1) == 0 )
				{
					SysMessageDefault( "lumberjacking_4" );
					return false;
				}

				SysMessageDefault( "lumberjacking_5" );

				//m_pChar->UpdateDir( pt );
				m_pChar->UpdateAnimate( ANIM_ATTACK_WEAPON );
				m_pChar->Sound( 0x13e );
				m_pChar->ItemBounce( CItem::CreateScript( ITEMID_KINDLING1, m_pChar ));
				return true;
			}

			m_pChar->m_Act_TargPrv = m_Targ_PrvUID;
			m_pChar->m_Act_Targ = m_Targ_UID;
			m_pChar->m_Act_p = pt;
			return( m_pChar->Skill_Start( SKILL_LUMBERJACKING ));
/*
			SysMessage( "Chopping the tree foliage yields nothing." );
			return( true );
*/
		case IT_LOG:

			if ( ! m_pChar->CanUse( pItemTarg, true ))
			{
				SysMessageDefault( "itemuse_log_unable" );
				return( false );
			}
			if ( pItemUse->IsType(IT_CARPENTRY_CHOP) )
			{
				return Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, "sm_carpentry" ));
			}
			if ( pItemUse->IsSameDispID( ITEMID_DAGGER ))
			{
				// set the target item
				m_Targ_UID = pItemTarg->GetUID();
				return Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, "sm_bowcraft" ) );
			}
			SysMessageDefault( "itemuse_log_use" );
			return false;

		case IT_FISH:
			if ( ! m_pChar->CanUse( pItemTarg, true ))
			{
				SysMessageDefault( "itemuse_fish_unable" );
				return( false );
			}

			// Carve up Fish parts.
			pItemTarg->SetID( ITEMID_FOOD_FISH_RAW );
			pItemTarg->SetAmount( 4 * pItemTarg->GetAmount());
			pItemTarg->Update();
			return true;

		case IT_CORPSE:
			if ( ! m_pChar->CanUse( pItemTarg, false ))
				return( false );
			m_pChar->Use_CarveCorpse( dynamic_cast <CItemCorpse*>( pItemTarg ));
			return true;

		case IT_FRUIT:
		case IT_REAGENT_RAW:
			// turn the fruit into a seed.
			if ( ! m_pChar->CanUse( pItemTarg, true ))
				return( false );
			{
			pItemTarg->SetDispID(ITEMID_COPPER_C1);	// copper coin
			pItemTarg->SetType(IT_SEED);
			CGString sTmp;
			sTmp.Format( "%s seed", pItemTarg->GetName());
			pItemTarg->SetName(sTmp);
			pItemTarg->Update();
			}
			return( true );

		// case IT_FOLIAGE: // trim back ?
//Torfo begin (we can not use static crops)
//		case IT_CROPS:
//			pItemTarg->Plant_CropReset();
//			return( true );
//Torfo end
		default:
			// Item to smash ? furniture ???

			if ( ! m_pChar->CanMove(pItemTarg) )
			{
				SysMessageDefault( "itemuse_weapon_immune" );
				return( false );	// ? using does not imply moving in all cases ! such as reading ?
			}

			// Is breaking this a crime ?
			if ( m_pChar->IsTakeCrime( pItemTarg ))
			{
				SysMessageDefault( "itemuse_steal" );
				return( false );
			}

			if ( pItemTarg )
				pItemTarg->OnTakeDamage( 1, m_pChar, DAMAGE_HIT_BLUNT );
			return( true );
		}
		break;

	case IT_BANDAGE:	// SKILL_HEALING, or SKILL_VETERINARY
		// Use bandages on some creature.
		if ( pCharTarg == NULL )
			return( false );
		m_pChar->m_Act_TargPrv = m_Targ_PrvUID;
		m_pChar->m_Act_Targ = m_Targ_UID;
		return( m_pChar->Skill_Start( (pCharTarg->GetNPCBrain() == NPCBRAIN_ANIMAL) ? SKILL_VETERINARY : SKILL_HEALING ));

	case IT_SEED:
		return m_pChar->Use_Seed( pItemUse, &pt );

	case IT_DEED:
		return( OnTarg_Use_Deed( pItemUse, pt ));

	case IT_WOOL:
	case IT_COTTON:
		// Use on a spinning wheel.
		if ( pItemTarg == NULL )
			break;
		if ( ! pItemTarg->IsType(IT_SPINWHEEL))
			break;
		if ( ! m_pChar->CanUse( pItemTarg, false ))
			return( false );

		pItemTarg->SetAnim( (ITEMID_TYPE)( pItemTarg->GetID() + 1 ), 2*TICK_PER_SEC );
		pItemUse->ConsumeAmount( 1 );

		{
			CItem * pNewItem;
			if ( pItemUse->IsType(IT_WOOL))
			{
				// 1 pile of wool yields three balls of yarn
				SysMessageDefault("itemuse_wool_create");
				pNewItem = CItem::CreateScript( ITEMID_YARN1, m_pChar );
				if ( pNewItem->GetAmount() == 1 )
					pNewItem->SetAmountUpdate( 3 );
			}
			else
			{
				// 1 pile of cotton yields six spools of thread
				SysMessageDefault("itemuse_cotton_create");
				pNewItem = CItem::CreateScript( ITEMID_THREAD1, m_pChar );
				if ( pNewItem->GetAmount() == 1 )
					pNewItem->SetAmountUpdate( 6 );
			}
			m_pChar->ItemBounce( pNewItem );
		}
		return true;

	case IT_KEYRING:
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

		CItem * pKey = NULL;
		bool fLockable = pItemTarg->IsTypeLockable();

		if ( fLockable && pItemTarg->m_itContainer.m_lockUID )
		{
			// try all the keys on the object.
			pKey = pKeyRing->ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_KEY), pItemTarg->m_itContainer.m_lockUID );
		}
		if ( pKey == NULL )
		{
			// are we trying to lock it down ?
			if ( m_pChar->m_pArea->GetResourceID().IsItem())
			{
				pKey = pKeyRing->ContentFind( RESOURCE_ID(RES_TYPEDEF,IT_KEY), m_pChar->m_pArea->GetResourceID() );
				if ( pKey )
				{
					if ( m_pChar->Use_MultiLockDown( pItemTarg ))
					{
						return( true );
					}
				}
			}

			if ( ! fLockable || ! pItemTarg->m_itContainer.m_lockUID )
			{
				SysMessageDefault( "itemuse_key_nolock");
			}
			else
			{
				SysMessageDefault( "itemuse_key_nokey" );
			}
			return false;
		}

		return( m_pChar->Use_Key( pKey, pItemTarg ));
		}

	case IT_SCISSORS:
		// cut hair as well ?
		if ( pCharTarg != NULL )
		{
			// Make this a crime ?
			if ( pCharTarg != m_pChar && ! IsPriv(PRIV_GM))
				return( false );

			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg != NULL )
				pObjTarg->Delete();
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
				pObjTarg->Delete();
			m_pChar->Sound( SOUND_SNIP );	// snip noise.
			return true;
		}
		if ( pItemTarg != NULL )
		{
			if ( ! m_pChar->CanUse( pItemTarg, true ))
				return( false );

			// Cut up cloth.
			int iOutQty = 0;
			ITEMID_TYPE iOutID = ITEMID_BANDAGES1;

			switch ( pItemTarg->GetType())
			{
			case IT_CLOTH_BOLT:
				// Just make cut cloth here !
				pItemTarg->ConvertBolttoCloth();
				m_pChar->Sound( SOUND_SNIP );	// snip noise.
				return( true );
			case IT_CLOTH:
				pItemTarg->ConvertBolttoCloth();
				iOutQty = pItemTarg->GetAmount();
				break;
			case IT_CLOTHING:
				// Cut up for bandages.
				iOutQty = pItemTarg->GetWeight()/WEIGHT_UNITS;
				break;
			case IT_HIDE:
				// IT_LEATHER
				// Cut up the hides and create strips of leather
				iOutID = ITEMID_LEATHER_1;
				iOutQty = pItemTarg->GetAmount();
				break;
			}
			if ( iOutQty )
			{
				CItem * pItemNew = CItem::CreateBase( iOutID );
				ASSERT(pItemNew);
				pItemNew->SetHue( pItemTarg->GetHue());
				pItemNew->SetAmount( iOutQty );
				m_pChar->ItemBounce( pItemNew );
				pItemTarg->Delete();
				m_pChar->Sound( SOUND_SNIP );	// snip noise.
				return( true );
			}
		}
		SysMessageDefault( "itemuse_scissors_use" );
		return false;

	case IT_YARN:
	case IT_THREAD:
		// Use this on a loom.
		// use on a spinning wheel.
		if ( pItemTarg == NULL )
			break;
		if ( ! pItemTarg->IsType( IT_LOOM ))
			break;
		if ( ! m_pChar->CanUse( pItemTarg, false ))
			return( false );

		{
static LPCTSTR const sm_Txt_LoomUse[] =
{
	"itemuse_bolt_1",
	"itemuse_bolt_2",
	"itemuse_bolt_3",
	"itemuse_bolt_4",
	"itemuse_bolt_5",
};

		// pItemTarg->SetAnim( (ITEMID_TYPE)( pItemTarg->GetID() + 1 ), 2*TICK_PER_SEC );

		// Use more1 to record the type of resource last used on this object
		// Use more2 to record the number of resources used so far
		// Check what was used last.
		if ( pItemTarg->m_itLoom.m_ClothID != pItemUse->GetDispID() &&
			pItemTarg->m_itLoom.m_ClothID )
		{
			// throw away what was on here before
			SysMessageDefault("itemuse_loom_remove");
			CItem * pItemCloth = CItem::CreateTemplate( pItemTarg->m_itLoom.m_ClothID, NULL, m_pChar );
			pItemCloth->SetAmount( pItemTarg->m_itLoom.m_ClothQty );
			pItemTarg->m_itLoom.m_ClothQty = 0;
			pItemTarg->m_itLoom.m_ClothID = ITEMID_NOTHING;
			m_pChar->ItemBounce( pItemCloth );
			return true;
		}

		pItemTarg->m_itLoom.m_ClothID = pItemUse->GetDispID();

		int iUsed;
		int iNeed = COUNTOF( sm_Txt_LoomUse )-1;
		int iHave = pItemTarg->m_itLoom.m_ClothQty;
		if ( iHave < iNeed )
		{
			iNeed -= iHave;
			iUsed = pItemUse->ConsumeAmount( iNeed );
		}

		if ( iHave  + iUsed < COUNTOF( sm_Txt_LoomUse )-1 )
		{
			pItemTarg->m_itLoom.m_ClothQty += iUsed;
			SysMessageDefault( sm_Txt_LoomUse[ pItemTarg->m_itLoom.m_ClothQty ] );
		}
		else
		{
			SysMessage( sm_Txt_LoomUse[ COUNTOF( sm_Txt_LoomUse )-1 ] );
			pItemTarg->m_itLoom.m_ClothQty = 0;
			pItemTarg->m_itLoom.m_ClothID = ITEMID_NOTHING;
			m_pChar->ItemBounce( CItem::CreateScript(ITEMID_CLOTH_BOLT1, m_pChar ));
		}
		}
		return true;

	case IT_BANDAGE_BLOOD:
		// Use these on water to clean them.
		switch ( m_pChar->CanTouchStatic( pt, id, pItemTarg ))
		{
		case IT_WATER:
		case IT_WATER_WASH:
			// Make clean.
			pItemUse->SetID( ITEMID_BANDAGES1 );
			pItemUse->Update();
			return( true );
		case IT_JUNK:
			SysMessageDefault( "itemuse_bandage_reach" );
			break;
		default:
			SysMessageDefault( "itemuse_bandage_clean" );
			break;
		}
		return( false );

	case IT_FISH_POLE:
		m_pChar->m_Act_p = pt;
		return( m_pChar->Skill_Start( SKILL_FISHING ));

	case IT_LOCKPICK:
		// Using a lock pick on something.
		if ( pItemTarg== NULL )
			return( false );
		m_pChar->m_Act_Targ = m_Targ_UID;	// the locked item to be picked
		m_pChar->m_Act_TargPrv = m_Targ_PrvUID;	// the pick
		if ( ! m_pChar->CanUse( pItemTarg, false ))
			return( false );
		return( m_pChar->Skill_Start( SKILL_LOCKPICKING ));

	case IT_CANNON_BALL:
		if ( m_pChar->Use_Cannon_Feed( pItemTarg, pItemUse ))
		{
			pItemUse->Delete();
			return( true );
		}
		break;

	case IT_DYE:
		if (( pItemTarg != NULL && pItemTarg->IsType(IT_DYE_VAT)) ||
			( pCharTarg != NULL && ( pCharTarg == m_pChar || IsPriv( PRIV_GM ))))	// Change skin color.
		{
			addDyeOption( pObjTarg );
			return true;
		}
		SysMessageDefault( "itemuse_dye_fail");
		return false;

	case IT_DYE_VAT:
		// Use the dye vat on some object.
		if ( pObjTarg == NULL )
			return false;

		if ( pObjTarg->GetTopLevelObj() != m_pChar &&
			! IsPriv( PRIV_GM ))	// Change hair wHue.
		{
			SysMessageDefault( "itemuse_dye_reach");
			return false;
		}
		if ( pCharTarg != NULL )
		{
			// Dye hair.
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
			{
				pObjTarg->SetHue( pItemUse->GetHue());
				pObjTarg->Update();
			}
			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg == NULL )
				return true;
			// fall through
		}
		else
		{
			if ( ! m_pChar->CanUse( pItemTarg, false ))
				return( false );
			if ( ! IsPriv( PRIV_GM ) &&
				! pItemTarg->Item_GetDef()->Can( CAN_I_DYE ) &&
				! pItemTarg->IsType(IT_CLOTHING) )
			{
				SysMessageDefault( "itemuse_dye_fail");
				return false;
			}
		}
		pObjTarg->SetHue( pItemUse->GetHue());
		pObjTarg->Update();
		return true;

	case IT_PITCHER_EMPTY:
		// Fill it up with water.
		switch ( m_pChar->CanTouchStatic( pt, id, pItemTarg ))
		{
		case IT_JUNK:
			SysMessageDefault( "itemuse_pitcher_reach" );
			return( false );
		case IT_WATER:
		case IT_WATER_WASH:
			pItemUse->SetID( ITEMID_PITCHER_WATER );
			pItemUse->Update();
			return( true );
		default:
			SysMessageDefault( "itemuse_pitcher_fill" );
			return( false );
		}
		break;

	case IT_POTION_EMPTY:
	case IT_MORTAR:
		// Alchemy targeting stuff
		return( Cmd_Skill_Alchemy( pItemTarg ));

	case IT_SEWING_KIT:
		// Use on cloth or hides
		if ( pItemTarg == NULL)
			break;
		if ( ! m_pChar->CanUse( pItemTarg, true ))
			return( false );

		switch ( pItemTarg->GetType())
		{
		case IT_LEATHER:
		case IT_HIDE:
			return( Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, "sm_tailor_leather" )));
		case IT_CLOTH:
		case IT_CLOTH_BOLT:
			pItemTarg->ConvertBolttoCloth();
			return( Cmd_Skill_Menu( g_Cfg.ResourceGetIDType( RES_SKILLMENU, "sm_tailor_cloth" )));
		}

		SysMessageDefault( "itemuse_sewingkit_unable" );
		return (false);
	}

	SysMessageDefault( "itemuse_unable");
	return false;
}

bool CClient::OnTarg_Stone_Recruit( CChar* pChar )
{
	// CLIMODE_TARG_STONE_RECRUIT
	CItemStone * pStone = dynamic_cast <CItemStone*> (m_Targ_UID.ItemFind());
	if ( !pStone )
		return false;
	return( pStone->AddRecruit( pChar, STONEPRIV_CANDIDATE ) != NULL );
}

bool CClient::OnTarg_Party_Add( CChar * pChar )
{
	// CLIMODE_TARG_PARTY_ADD
	// Invite this person to join our party. PARTYMSG_Add

	if ( pChar == NULL )
	{
		SysMessageDefault( "party_select" );
		return( false );
	}
	if (( pChar->m_pParty && pChar->m_pParty == m_pChar->m_pParty ) ||
		( IsPriv(PRIV_GM) && GetPrivLevel() > pChar->GetPrivLevel()))
	{
		// They are forced to join.
		CPartyDef::AcceptEvent( pChar, m_pChar->GetUID());
		return( true );
	}

	if ( ! pChar->IsClient())
	{
		// pets should just join as instructed.
		if ( pChar->NPC_IsOwnedBy( m_pChar ))
		{
			CPartyDef::AcceptEvent( pChar, m_pChar->GetUID());
			return true;
		}
		return( false );
	}

	SysMessagef( g_Cfg.GetDefaultMsg( "party_invite" ), pChar->GetName() );

	pChar->SysMessagef( g_Cfg.GetDefaultMsg( "party_invite_targ" ), (LPCTSTR) m_pChar->GetName());

	CExtData ExtData;
	ExtData.Party_Msg_Rsp.m_code = PARTYMSG_NotoInvited;
	ExtData.Party_Msg_Rsp.m_UID = m_pChar->GetUID();
	pChar->GetClient()->addExtData( EXTDATA_Party_Msg, &ExtData, 9 );

	// Now up to them to decide to accept.
	return( true );
}
