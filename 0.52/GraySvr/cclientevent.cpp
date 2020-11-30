//
// CClientEvent.cpp
// Copyright Menace Software (www.menasoft.com).
//
#include "graysvr.h"	// predef header.

/////////////////////////////////
// Events from the Client.

void CClient::Event_ChatButton(const NCHAR * pszName) // Client's chat button was pressed
{
	// See if they've made a chatname yet
	// m_ChatPersona.SetClient(this);

	if ( GetAccount()->m_sChatName.IsEmpty())
	{
		// No chatname yet, see if the client sent one
		if (pszName[0] == 0) // No name was sent, so ask for a permanent chat system nickname (account based)
		{
			addChatSystemMessage(CHATMSG_GetChatName);
			return;
		}
		// OK, we got a nick, now store it with the account stuff.

		// Make it non unicode
		TCHAR name[ MAX_NAME_SIZE * 2 + 2 ];
		CvtNUNICODEToSystem( name, pszName, sizeof(name));

		if ( ! g_Serv.m_Chats.IsValidName( name, true) ||
			g_Serv.m_Chats.FindChatName(name)) // Check for legal name, duplicates, etc
		{
			addChatSystemMessage(CHATMSG_Error);
			addChatSystemMessage(CHATMSG_GetChatName);
			return;
		}
		m_pAccount->m_sChatName = name;
		// SetChatName( m_pAccount->m_sChatName );
	}

	// Ok, below here we have a chat system nickname
	// Tell the chat system it has a new client using it
	SetChatActive();
}

void CClient::Event_ChatText( const char * pszLang, const NCHAR * pszText, int len) // Text from a client
{
	// Just send it all to the chat system
	g_Serv.m_Chats.Event(this, pszLang, pszText, len);
}

void CClient::Event_MapEdit( CObjUID uid )
{
	CItemMap * pMap = dynamic_cast <CItemMap*> ( uid.ItemFind());
	if ( ! m_pChar->CanTouch( pMap ))	// sanity check.
	{
		SysMessage( "You can't reach it" );
		return;
	}
	if ( pMap->m_itMap.m_fPinsGlued )
	{
		SysMessage( "The pins seem to be glued in place" );
		if ( ! IsPriv(PRIV_GM))
		{
			return;
		}
	}

	// NOTE: while in edit mode, right click canceling of the
	// dialog sends the same msg as
	// request to edit in either mode...strange huh?

	switch (m_bin.MapEdit.m_action)
	{
	case MAP_ADD: // add pin
		if ( pMap->m_Pins.GetCount() > CItemMap::MAX_PINS ) 
			return;	// too many.
		pMap->m_Pins.Add( CMapPinRec( m_bin.MapEdit.m_pin_x, m_bin.MapEdit.m_pin_y ));
		break;
	case MAP_INSERT: // insert between 2 pins
		if ( pMap->m_Pins.GetCount() > CItemMap::MAX_PINS ) 
			return;	// too many.
		pMap->m_Pins.InsertAt( m_bin.MapEdit.m_pin, CMapPinRec( m_bin.MapEdit.m_pin_x, m_bin.MapEdit.m_pin_y ));
		break;
	case MAP_MOVE: // move pin
		{
			if ( m_bin.MapEdit.m_pin >= pMap->m_Pins.GetCount())
			{
				SysMessage( "That's strange... (bad pin)" );
				break;
			}
			pMap->m_Pins[m_bin.MapEdit.m_pin].m_x = m_bin.MapEdit.m_pin_x;
			pMap->m_Pins[m_bin.MapEdit.m_pin].m_y = m_bin.MapEdit.m_pin_y;
			break;
		}
	case MAP_DELETE: // delete pin
		{
			if ( m_bin.MapEdit.m_pin >= pMap->m_Pins.GetCount())
			{
				SysMessage( "That's strange... (bad pin)" );
				break;
			}
			pMap->m_Pins.RemoveAt(m_bin.MapEdit.m_pin);
			break;
		}
		break;
	case MAP_CLEAR: // clear all pins
		pMap->m_Pins.RemoveAll();
		break;
	case MAP_TOGGLE: // edit req/cancel
		addMapMode( pMap, MAP_SENT, ! pMap->m_fPlotMode );
		break;
	}
}

void CClient::Event_Item_Dye( CObjUID uid ) // Rehue an item
{
	// Result from addDyeOption()
	CObjBase * pObj = uid.ObjFind();
	if ( ! m_pChar->CanTouch( pObj ))	// sanity check.
	{
		SysMessage( "You can't reach it" );
		return;
	}
	if ( GetTargMode() != TARGMODE_DYE )
	{
		return;
	}

	ClearTargMode();
	COLOR_TYPE color = m_bin.DyeVat.m_color ;

	if ( ! IsPriv( PRIV_GM ))
	{
		if ( pObj->IsChar())
			return;
		if ( color<COLOR_BLUE_LOW || color>COLOR_DYE_HIGH )
			color = COLOR_DYE_HIGH;
	}
	else
	{
		if ( pObj->IsChar())
		{
			pObj->RemoveFromView();
			color |= COLOR_UNDERWEAR;
		}
	}

	pObj->SetColor( color );
	pObj->Update();
}

void CClient::Event_Tips( WORD i) // Tip of the day window
{
	if (i==0) i=1;
	CGString sSec;
	sSec.Format( "TIP %i", i );
	addScrollFile( sSec, SCROLL_TYPE_TIPS, i );
}

void CClient::Event_Book_Title( CObjUID uid, const TCHAR * pszTitle, const TCHAR * pszAuthor )
{
	// user is changing the books title/author info.

	CItemMessage * pBook = dynamic_cast <CItemMessage *> (uid.ItemFind());
	if ( ! m_pChar->CanTouch( pBook ))	// sanity check.
	{
		SysMessage( "you can't reach it" );
		return;
	}
	if ( ! pBook->IsBookWritable())	// Not blank
		return;

	pBook->SetName( pszTitle );
	pBook->m_sAuthor = pszAuthor;
}

void CClient::Event_Book_Page( CObjUID uid ) // Book window
{
	// Read or write to a book page.

	CItem * pBook = uid.ItemFind();	// the book.
	if ( ! m_pChar->CanSee( pBook )) 
	{
		addObjectRemoveCantSee( uid, "the book" );
		return;
	}

	int iPage = m_bin.BookPage.m_page[0].m_pagenum;	// page.
	DEBUG_CHECK( iPage > 0 );

	if ( m_bin.BookPage.m_page[0].m_lines == 0xFFFF || m_bin.BookPage.m_len <= 0x0d )
	{
		// just a request for pages.
		addBookPage( pBook, iPage );
		return;
	}

	// Trying to write to the book.
	CItemMessage * pText = dynamic_cast <CItemMessage *> (pBook);
	if ( pText == NULL || ! pBook->IsBookWritable()) // not blank ?
		return;

	int iLines = m_bin.BookPage.m_page[0].m_lines;
	DEBUG_CHECK( iLines <= 8 );
	DEBUG_CHECK( m_bin.BookPage.m_pages == 1 );

	if ( ! iLines || iPage <= 0 || iPage > 16 ) 
		return;
	if ( iLines > 8 ) iLines = 8;
	iPage --;

	int len = 0;
	TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
	for ( int i=0; i<iLines; i++ )
	{
		len += strcpylen( szTemp+len, m_bin.BookPage.m_page[0].m_text+len );
		szTemp[len++] = '\t';
	}

	szTemp[--len] = '\0';
	pText->SetPageText( iPage, szTemp );
}

void CClient::Event_Item_Pickup( CObjUID uid, int amount ) // Client grabs an item
{
	// Player/client is picking up an item.

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_Item_Pickup %d\n", GetSocket(), uid ));
	}

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL || pItem->IsWeird())
	{
		addItemDragCancel(0);
		addObjectRemove( uid );
		return;
	}

	// Where is the item coming from ? (just in case we have to toss it back)
	CObjBase * pObjParent = dynamic_cast <CObjBase *>(pItem->GetParent());
	m_Targ_PrvUID = ( pObjParent ) ? (DWORD) pObjParent->GetUID() : UID_CLEAR;
	m_Targ_p = pItem->GetUnkPoint();

	amount = m_pChar->ItemPickup( pItem, amount );
	if ( amount < 0 )
	{
		addItemDragCancel(0);
		return;
	}

	addPause();
	SetTargMode( TARGMODE_DRAG );
	m_Targ_UID = uid;
}

void CClient::Event_Item_Drop() // Item is dropped
{
	// This started from the Event_Item_Pickup()

	CObjUID uidItem( m_bin.ItemDropReq.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CObjUID uidOn( m_bin.ItemDropReq.m_UIDCont );	// dropped on this item.
	CObjBase * pObjOn = uidOn.ObjFind();
	CPointMap  pt( m_bin.ItemDropReq.m_x, m_bin.ItemDropReq.m_y, m_bin.ItemDropReq.m_z );

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_Item_Drop %lx on %lx, x=%d, y=%d\n",
			GetSocket(), uidItem, uidOn, pt.m_x, pt.m_y ));
	}

	// Are we out of sync ?
	if ( pItem == NULL ||
		pItem == pObjOn ||	// silliness.
		GetTargMode() != TARGMODE_DRAG ||
		pItem != m_pChar->LayerFind( LAYER_DRAGGING ))
	{
		addItemDragCancel(5);
		return;
	}

	ClearTargMode();	// done dragging

	addPause();
	
	SOUND_TYPE sound;	// What sound will it make ?

	if ( pObjOn != NULL )	// Put on or in another object
	{
		if ( ! m_pChar->CanTouch( pObjOn ))	// Must also be LOS !
		{
			goto cantdrop;
		}

		if ( pObjOn->IsChar())	// Drop on a chars head.
		{
			CChar * pChar = dynamic_cast <CChar*>( pObjOn );
			if ( pChar != m_pChar )
			{
				if ( ! Cmd_SecureTrade( pChar, pItem ))
					goto cantdrop;
				return;
			}

			// dropped on myself. Get my Pack.
			pObjOn = m_pChar->GetPackSafe();
		}

		// On a container item ?
		CItemContainer * pContItem = dynamic_cast <CItemContainer *>( pObjOn );

		// Is the object on a person ? check the weight.
		CObjBaseTemplate * pObjTop = pObjOn->GetTopLevelObj();
		if ( pObjTop->IsChar())
		{
			CChar * pChar = dynamic_cast <CChar*>( pObjTop );
			if ( ! pChar->NPC_IsOwnedBy( m_pChar ))
			{
				// Slyly dropping item in someone elses pack.
				// or just dropping on their trade window.
				if ( ! Cmd_SecureTrade( pChar, pItem ))
					goto cantdrop;
				return;
			}
			if ( pChar->m_pNPC )
			{
				// newbie items loose newbie status when transfered to NPC
				pItem->m_Attr &= ~ATTR_NEWBIE;
			}
			if ( pChar->GetBank()->IsItemInContainer( pContItem ))
			{
				// Diff Weight restrict for bank box and items in the bank box.
				if ( ! pChar->GetBank()->CanContainerHold( pItem, m_pChar ))
					goto cantdrop;
			}
			else if ( ! pChar->CanCarry( pItem ))
			{
				// SysMessage( "That is too heavy" );
				goto cantdrop;
			}
		}

		if ( pContItem != NULL )
		{
			// Putting it into some sort of container.
			if ( pContItem->m_type == ITEM_TRASH )
			{
				pContItem->Sound( 0x235 ); // a little sound so we know it "ate" it.
				SysMessage( "You trash the item" );
				pItem->Delete();
				return;
			}

			if ( ! pContItem->CanContainerHold( pItem, m_pChar ))
				goto cantdrop;
		}
		else
		{
			// dropped on top of a non container item.
			// can i pile them ?
			// Still in same container.
			CItem * pItemOn = dynamic_cast <CItem*> ( pObjOn );
			pObjOn = pItemOn->GetContainer();
			pt = pItemOn->GetUnkPoint();
			if ( ! pItem->Stack( pItemOn ))
			{
				if ( pItemOn->m_type == ITEM_SPELLBOOK )
				{
					if ( pItemOn->AddSpellbookScroll( pItem ))
					{
						SysMessage( "Can't add this to the spellbook" );
						goto cantdrop;
					}
					addSound( 0x057, pItemOn );	// add to inv sound.
					return;
				}

				// Just drop on top of the current item.
				// Client probably doesn't allow this anyhow.
			}
		}
		sound = 0x057;	// add to inv sound.
	}
	else
	{
		if ( ! m_pChar->CanTouch( pt ))	// Must also be LOS !
		{
	cantdrop:
			// The item was in the LAYER_DRAGGING.
			// Try to bounce it back to where it came from.
			m_pChar->ItemBounce( pItem );
			return;
		}
		sound = 0x042;	// drop on ground sound
	}

	// Game pieces can only be droped on their game boards.
	if ( pItem->m_type == ITEM_GAME_PIECE )
	{
		if ( pObjOn == NULL || m_Targ_PrvUID != pObjOn->GetUID())
		{
			CItemContainer * pGame = dynamic_cast <CItemContainer *>( m_Targ_PrvUID.ItemFind());
			if ( pGame != NULL )
			{
				pGame->ContentAdd( pItem, m_Targ_p );
			}
			else
			{
				DEBUG_CHECK( 0 );
				pItem->Delete();	// Not sure what else to do with it.
			}
			return;
		}
	}

	SOUND_TYPE soundspec = pItem->GetDropSound();
	if ( soundspec ) sound = soundspec;

	// do the dragging anim for everyone else to see.

	if ( pObjOn != NULL )
	{
		// in pack or other CItemContainer.
		m_pChar->UpdateDrag( pItem, pObjOn );
		(dynamic_cast <CItemContainer *>(pObjOn))->ContentAdd( pItem, pt );
		addSound( sound );
	}
	else
	{	// on ground
		if ( g_Serv.m_fFlipDroppedItems &&
			pItem->IsMovableType() &&
			! pItem->m_pDef->IsStackableType())
		{
			// Does this item have a flipped version.
			pItem->SetDispID( pItem->m_pDef->Flip( pItem->GetDispID()));
		}
		m_pChar->UpdateDrag( pItem, NULL, &pt );
		pItem->MoveToDecay( pt );
		pItem->Sound( sound );
	}
}

void CClient::Event_Item_Equip() // Item is dropped on paperdoll
{
	// This started from the Event_Item_Pickup()

	CObjUID uidItem( m_bin.ItemEquipReq.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CObjUID uidChar( m_bin.ItemEquipReq.m_UIDChar );
	CChar * pChar = uidChar.CharFind();
	LAYER_TYPE layer = (LAYER_TYPE)( m_bin.ItemEquipReq.m_layer );

	if ( pItem == NULL ||
		GetTargMode() != TARGMODE_DRAG ||
		pItem != m_pChar->LayerFind( LAYER_DRAGGING ))
	{
		// I have no idea why i got here.
		addItemDragCancel(5);
		return;
	}

	ClearTargMode(); // done dragging.

	if ( pChar == NULL ||
		layer >= LAYER_HORSE )	// Can't equip this way.
	{
	cantequip:
		// The item was in the LAYER_DRAGGING.
		m_pChar->ItemBounce( pItem );	// drop at our feet.
		return;
	}

	if ( layer != pItem->m_pDef->m_layer )
	{
		DEBUG_ERR(( "%x:Item 0%x equipped on strange layer %i.\n", GetSocket(), pItem->GetID(), layer ));
		// goto cantequip;
	}

	if ( ! pChar->NPC_IsOwnedBy( m_pChar ))
	{
		// trying to equip another char ?
		// can if he works for you.
		// else just give it to him ?
		goto cantequip;
	}

	if ( ! pChar->ItemEquip( pItem ))
	{
		if ( pChar != m_pChar )
		{
			SysMessage( "Not strong enough to equip this." );
		}
		goto cantequip;
	}

#ifdef _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Item 0%x equipped on layer %i.\n", GetSocket(), pItem->GetID(), layer ));
	}
#endif
}

void CClient::Event_Skill_Locks()
{
	// Skill lock buttons in the skills window.
	ASSERT( GetChar());
	ASSERT( m_bin.Skill.m_Cmd == XCMD_Skill );		// 0= 0x3A
	ASSERT( GetChar()->m_pPlayer );
	DEBUG_CHECK( ! m_Crypt.GetClientVersion() || m_Crypt.GetClientVersion() >= 12602 );

	int len = m_bin.Skill.m_len;
	len -= 3;
	for ( int i=0; len; i++ )
	{
		SKILL_TYPE index = (SKILL_TYPE)(WORD) m_bin.Skill.skills[i].m_index;
		SKILLLOCK_TYPE state = (SKILLLOCK_TYPE) m_bin.Skill.skills[i].m_lock;

		GetChar()->m_pPlayer->Skill_SetLock( index, state );

		len -= sizeof( m_bin.Skill.skills[0] );
	}
}

void CClient::Event_Skill_Use( SKILL_TYPE skill ) // Skill is clicked on the skill list
{
	// All the push button skills come through here.
	// Any "Last skill" macro comes here as well. (push button only)

	bool fContinue = false;

	if ( m_pChar->Skill_Wait())
		return;

	SetTargMode();
	m_Targ_UID.ClearUID();	// This is a start point for targ more.

	switch ( skill )
	{
	case SKILL_ARMSLORE:
	case SKILL_ITEMID:
	case SKILL_ANATOMY:
	case SKILL_ANIMALLORE:
	case SKILL_EVALINT:
	case SKILL_FORENSICS:
	case SKILL_TASTEID:

	case SKILL_BEGGING:
	case SKILL_STEALING:
	case SKILL_TAMING:
	case SKILL_ENTICEMENT:
	case SKILL_PROVOCATION:
	case SKILL_POISONING:
	case SKILL_RemoveTrap:
		// Go into targtting mode.
		if ( g_Serv.m_SkillDefs[skill]->m_sTargetPrompt.IsEmpty())
		{
			DEBUG_ERR(( "%x: Event_Skill_Use bad skill %d\n", GetSocket(), skill ));
			return;
		}
		m_Targ_Skill = skill;
		addTarget( TARGMODE_SKILL, g_Serv.m_SkillDefs[skill]->m_sTargetPrompt );
		return;

	case SKILL_Stealth:	// How is this supposed to work.
	case SKILL_HIDING:
	case SKILL_SPIRITSPEAK:
	case SKILL_PEACEMAKING:
	case SKILL_DETECTINGHIDDEN:
	case SKILL_MEDITATION:
		// These start/stop automatically.
		m_pChar->Skill_Start(skill);
		return;

	case SKILL_TRACKING:
		Cmd_Skill_Tracking( -1, false );
		break;

	case SKILL_CARTOGRAPHY:
		// Menu select for map type.
		Cmd_Skill_Cartography( 0 );
		break;

	case SKILL_INSCRIPTION:
		// Menu select for spell type.
		Cmd_Skill_Inscription( 0, -1 );
		break;

	default:
		SysMessage( "There is no such skill. Please tell support you saw this message.");
		break;
	}
}

bool CClient::Event_WalkingCheck(DWORD dwEcho)
{
	// look for the walk code, and remove it

	if ( m_Crypt.GetClientVersion() < 12600 )
		return( true );

	if ( ! ( g_Serv.m_wDebugFlags & DEBUGF_WALKCODES ))
		return( true );

	// If the LIFO stack has not been sent, send them out now
	if ( m_Walk_CodeQty < 0 )
	{
		addWalkCode(EXTDATA_WalkCode_Prime,COUNTOF(m_Walk_LIFO));
	}

	// Keep track of echo'd 0's and invalid non 0's
	// (you can get 1 to 4 of these legitimately when you first start walking)
	ASSERT( m_Walk_CodeQty >= 0 );

	for ( int i=0; i<m_Walk_CodeQty; i++ )
	{
		if ( m_Walk_LIFO[i] == dwEcho )
		{
			m_Walk_CodeQty--;
			ASSERT( m_Walk_CodeQty < COUNTOF(m_Walk_LIFO));
			int n = m_Walk_CodeQty-i;
			memmove( m_Walk_LIFO+i, m_Walk_LIFO+i+1, n*sizeof(DWORD));
			// Set this to negative so we know later if we've gotten at least 1 valid echo
			m_Walk_InvalidEchos = -1;
			return( true );
		}
	}

	if ( m_Walk_InvalidEchos < 0 )
	{
		// If we're here, we got at least one valid echo....therefore
		// we should never get another one
		DEBUG_ERR(( "%x: Invalid walk echo (0%x). Invalid after valid.\n", GetSocket(), dwEcho ));
		addPlayerWalkCancel();
		return false;
	}

	// Increment # of invalids received
	if ( ++ m_Walk_InvalidEchos >= COUNTOF(m_Walk_LIFO))
	{
		// The most I ever got was 2 of these, but I've seen 4
		// a couple of times on a real server...we might have to
		// increase this # if it becomes a problem (but I doubt that)
		DEBUG_ERR(( "%x: Invalid walk echo. Too many initial invalid.\n", GetSocket()));
		addPlayerWalkCancel();
		return false;
	}

	// Allow them to walk a bit till we catch up.
	return true;
}

void CClient::Event_Walking( BYTE rawdir, BYTE count, DWORD dwEcho ) // Player moves
{
	// The theory....
	// The client sometimes echos 1 or 2 zeros or invalid echos when you first start
	//	walking (the invalid non zeros happen when you log off and don't exit the
	//	client.exe all the way and then log back in, OSI doesn't clear the stack)

	if ( ! Event_WalkingCheck( dwEcho ))
		return;

	if ( m_pChar->IsStat( STATF_Freeze | STATF_Stone ) &&
		m_pChar->OnFreezeCheck())
	{
		addPlayerWalkCancel();
		return;
	}

	if ( count != (BYTE)( m_WalkCount+1 ))	// && count != 255
	{
		// DEBUG_MSG(( "%x: New Walk Count %d!=%d\n", GetSocket(), count, m_WalkCount ));
		if ( (WORD)(m_WalkCount) == 0xFFFF ) 
			return;	// just playing catch up with last reset. don't cancel again.
		addPlayerWalkCancel();
		return;
	}

	bool fRun = ( rawdir & 0x80 ); // or flying ?

	m_pChar->ModStat( STATF_Fly, fRun );

	DIR_TYPE dir = (DIR_TYPE)( rawdir & 0x0F );
	if ( dir >= DIR_QTY )
	{
		addPlayerWalkCancel();
		return;
	}
	CPointMap pt = m_pChar->GetTopPoint();
	CPointMap ptold = pt;
	bool fMove = true;

	if ( dir == m_pChar->m_face_dir )
	{
		// Move in this dir.
		pt.Move( dir );

		// Check if we have gone indoors.
		bool fRoof = m_pChar->IsStat( STATF_InDoors );

		// Check the z height here.
		// The client already knows this but doesn't tell us.
		if ( ! m_pChar->CanMoveWalkTo( pt ))
		{
			addPlayerWalkCancel();
			return;
		}

		// Are we invis ?
		m_pChar->CheckRevealOnMove();
		m_pChar->MoveTo( pt );

		// Should i update the weather ?
		if ( fRoof != m_pChar->IsStat( STATF_InDoors ))
		{
			addWeather( WEATHER_DEFAULT );
		}

		// did i step on a telepad, trap, etc ?
		if ( m_pChar->CheckLocation())
		{
			// We stepped on teleporter
			return;
		}
	}
	else
	{
		// Just a change in dir.
		m_pChar->m_face_dir = dir;
		fMove = false;
	}

	// Ack the move. ( if this does not go back we get rubber banding )
	m_WalkCount = count;
	CCommand cmd;
	cmd.WalkAck.m_Cmd = XCMD_WalkAck;
	cmd.WalkAck.m_count = m_WalkCount;
	// Not really sure what this does.
	cmd.WalkAck.m_flag = ( m_pChar->IsStat( STATF_Insubstantial | STATF_Invisible | STATF_Hidden | STATF_Sleeping )) ?
		0 : 0x41;
	xSendPkt( &cmd, sizeof( cmd.WalkAck ));

	if ( ! fMove )
	{
		// Show others I have turned !!
		m_pChar->UpdateMode( this );
		return;
	}

	// Who now sees me ?
	m_pChar->UpdateMove( ptold, this );
	// What do I now see ?
	addPlayerSee( ptold );
}

void CClient::Event_CombatMode( bool fWar ) // Only for switching to combat mode
{
	// If peacmaking then this doens't work ??
	// Say "you are feeling too peacefull"

	m_pChar->ModStat( STATF_War, fWar );

	if ( m_pChar->IsStat( STATF_DEAD ))
	{
		// Manifest the ghost.
		// War mode for ghosts.
		m_pChar->ModStat( STATF_Insubstantial, ! fWar );
	}

	m_pChar->Skill_Fail( true );	// clean up current skill.
	m_pChar->m_Act_Targ.ClearUID();	// no target yet
	addPlayerWarMode();
	m_pChar->UpdateMode( this, m_pChar->IsStat( STATF_DEAD ));
}

void CClient::Event_MenuChoice() // Choice from GMMenu or Itemmenu received
{
	// Select from a menu. CMenuItem
	// result of addItemMenu call previous.
	// select = 0 = cancel.

	TARGMODE_TYPE menuid = (TARGMODE_TYPE)(DWORD) m_bin.MenuChoice.m_menuid;
	if ( menuid != GetTargMode() ||
		m_bin.MenuChoice.m_UID != m_pChar->GetUID())
	{
		DEBUG_ERR(( "%x: Menu choice unrequested %d!=%d\n", GetSocket(), menuid, m_Targ_Mode ));
		return;
	}

	ClearTargMode();
	WORD select = m_bin.MenuChoice.m_select;

	// Item Script or GM menu script got us here.
	if ( menuid < TARGMODE_MENU_SKILL )
	{
		Cmd_Script_Menu( menuid, select );
		return;
	}

	switch ( menuid )
	{
	case TARGMODE_MENU_SKILL:
		// Some skill menu got us here.
		if ( select >= COUNTOF(m_tmMenu)) 
			return;
		Cmd_Skill_Menu( m_Targ_Skill, m_tmMenu[0], (select) ? m_tmMenu[select] : 0 );
		return;
	case TARGMODE_MENU_SKILL_TRACK_SETUP:
		// PreTracking menu got us here.
		Cmd_Skill_Tracking( select, false );
		return;
	case TARGMODE_MENU_SKILL_TRACK:
		// Tracking menu got us here. Start tracking the selected creature.
		Cmd_Skill_Tracking( select, true );
		return;
	case TARGMODE_MENU_SKILL_INSCRIPTION:
		Cmd_Skill_Inscription( m_tmMenu[0], select );
		return;
	case TARGMODE_MENU_GM_PAGES:
		// Select a GM page from the menu.
		Cmd_GM_PageSelect( select );
		return;
	case TARGMODE_MENU_EDIT:
		Cmd_EditItem( ((CObjUID)(m_tmMenu[0])).ObjFind(), select );
		return;
	default:
		DEBUG_ERR(( "%x:Unknown Targetting mode for menu %d\n", GetSocket(), menuid ));
		return;
	}
}

void CClient::Event_Command( TCHAR * pszCommand ) // Client entered a console command like /ADD
{
	// Get a spoken /command.
	// Copy it to temp buffer.

	ASSERT(pszCommand);

	CScript s( pszCommand );
	if ( s.GetKey()[0] == '\0' ) 
		return;

	// Administrator level = debug and server security type stuff.
	bool fRet = false;
	int ilevel = PLEVEL_QTY-1;

	if ( GetPrivLevel() >= PLEVEL_Admin )
	{
		fRet = g_Serv.OnServCmd( s.GetKey(), s.GetArgStr(), this );
		if ( fRet )
			goto donehere;
	}

	// Is this command avail for your priv level (or lower) ?
	for ( ; ilevel; ilevel-- )
	{
		if ( FindTableSorted( s.GetKey(), g_Serv.m_PrivCommands[ilevel].GetBasePtr(), g_Serv.m_PrivCommands[ilevel].GetCount()) < 0 )
		{
			// A GM will default to use all commands.
			// xcept those that are specifically named that i can't use.
			if ( ilevel > PLEVEL_GM || GetPrivLevel() < PLEVEL_GM )
				continue;
		}

		if ( ilevel > GetPrivLevel())	// priv is set too high for you.
		{
			SysMessage( "You can't use this command." );
			goto donehere;
		}

		static const TCHAR * szCmd_Redirect[] =		// default to redirect these.
		{
			"DUPE",
			"FORGIVE",
			"JAIL",
			"KICK",
			"KILL",
			"NUDGEDOWN",
			"NUDGEUP",
			"PARDON",
			"REMOVE",
			"SHRINK",
		};
		if ( FindTableSorted( s.GetKey(), szCmd_Redirect, COUNTOF(szCmd_Redirect)) >= 0 )
		{
			// targetted verbs are logged once the target is selected.
			addTargetVerb( pszCommand, "" );
			return;
		}

		fRet = m_pChar->r_Verb( s, m_pChar );
		break;
	}

	if ( ! fRet )
	{
		SysMessage( "Not a valid command or format" );
	}

donehere:
	g_Log.Event( LOGM_GM_CMDS, "%x:'%s' commands '%s'=%d\n", GetSocket(), GetName(), pszCommand, fRet );
}

void CClient::Event_Attack( CObjUID uid )
{
	// d-click in war mode
	// I am attacking someone.

	// ??? Combat music = MIDI = 37,38,39

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) 
		return;

	// Accept or decline the attack.
	CCommand cmd;
	cmd.AttackOK.m_Cmd = XCMD_AttackOK;
	cmd.AttackOK.m_UID = (m_pChar->Attack( pChar )) ? (DWORD) pChar->GetUID() : 0;
	xSendPkt( &cmd, sizeof( cmd.AttackOK ));
}

void CClient::Event_VendorBuy( CObjUID uidVendor )
{
	// Client buying items from the Vendor

	if ( m_bin.VendorBuy.m_flag == 0 )	// just a close command.
		return;

	CChar * pVendor = uidVendor.CharFind();
	if ( ! m_pChar->CanTouch( pVendor ))
	{
		SysMessage( "You can't reach the vendor" );
		return;
	}

	// Calculate the total cost of goods.
	int costtotal=0;
	bool fSoldout = false;
	int nItems = (m_bin.VendorBuy.m_len - 8) / sizeof( m_bin.VendorBuy.items[0] );
	int i=0;
	for ( ;i<nItems;i++)
	{
		CObjUID uid( m_bin.VendorBuy.items[i].m_UID );
		CItemVendable * pItem = dynamic_cast <CItemVendable *> (uid.ItemFind());
		if ( pItem == NULL ) 
			continue;	// ignore it for now.

		// Do cheat/sanity check on goods sold.
		//int iPrice = pItem->GetVendorPrice( true );
		long iPrice = pItem->GetBuyPrice();
		if ( ! iPrice ||
			pItem->GetTopLevelObj() != pVendor ||
			m_bin.VendorBuy.items[i].m_amount > pItem->GetAmount())
		{
			fSoldout = true;
			continue;
		}
		costtotal += m_bin.VendorBuy.items[i].m_amount * iPrice;
	}


	if ( fSoldout )
	{
		pVendor->Speak( "Alas, I don't have all those goods in stock.  Let me know if there is something else thou wouldst buy." );
		// ??? Update the vendors list and try again.
		return;
	}

	bool fBoss = pVendor->NPC_IsOwnedBy( m_pChar );
	if ( ! fBoss )
	{
		if ( m_pChar->ContentConsume( ITEMID_GOLD_C1, costtotal, true ))
		{
			pVendor->Speak( "Alas, thou dost not possess sufficient gold for this purchase!" );
			return;
		}
	}

	if ( costtotal <= 0 )
	{
		pVendor->Speak( "You have bought nothing. But feel free to browse" );
		return;
	}

	CItemContainer * pPack = m_pChar->GetPackSafe();

	// Move the items bought into your pack.
	for ( i=0;i<nItems;i++)
	{
		CObjUID uid( m_bin.VendorBuy.items[i].m_UID );
		CItem * pItem = uid.ItemFind();
		if ( pItem == NULL ) 
			continue;	// ignore it i guess.
		if ( ! pItem->IsValidSaleItem( true )) 
			continue;	// sorry can't buy this !

		WORD amount = m_bin.VendorBuy.items[i].m_amount;
		pItem->SetAmount( pItem->GetAmount() - amount );

		switch ( pItem->m_type )
		{
		case ITEM_FIGURINE:
			// Buying animals is special.
			m_pChar->Use_Figurine( pItem, 2 );
			continue;
		case ITEM_BEARD:
			if ( m_pChar->GetID() != CREID_MAN )
			{
nohair:
				pVendor->Speak( "Sorry not much i can do for you" );
				continue;
			}
		case ITEM_HAIR:
			// Must be added directly. can't exist in pack!
			if ( ! m_pChar->IsHuman())
				goto nohair;
			m_pChar->LayerAdd( CItem::CreateDupeItem( pItem ));
			m_pChar->Sound( 0x248 );	// snip noise.
			pVendor->UpdateAnimate(ANIM_ATTACK_1H_WIDE);
			continue;
		}

		if ( amount > 1 && ! pItem->m_pDef->IsStackableType())
		{
			while ( amount -- )
			{
				CItem * pItemNew = CItem::CreateDupeItem( pItem );
				pItemNew->SetAmount(1);
				pPack->ContentAdd(pItemNew);
			}
		}
		else
		{
			CItem * pItemNew = CItem::CreateDupeItem( pItem );
			pItemNew->SetAmount( amount );
			pPack->ContentAdd( pItemNew );
		}

		if ( pVendor->IsStat( STATF_Pet ) ||
			( pItem->GetAmount() == 0 &&
			m_bin.VendorBuy.items[i].m_layer == LAYER_VENDOR_EXTRA ))
		{
			// we can buy it all.
			// not allowed to delete all from LAYER_VENDOR_STOCK
			pItem->Delete();
		}
		else
		{
			pItem->Update();
		}
	}

	CGString sMsg;
	sMsg.Format(
		fBoss ?
		"Here you are, %s. "
		"That is %d gold coin%s worth of goods." :
		"Here you are, %s. "
		"That will be %d gold coin%s. "
		"I thank thee for thy business.",
			m_pChar->GetName(), costtotal, (costtotal==1) ? "" : "s" );
	pVendor->Speak( sMsg );

	// Take the gold.
	if ( ! fBoss )
	{
		m_pChar->ContentConsume( ITEMID_GOLD_C1, costtotal );

		// Add the gold to the vendors total to play with.
		pVendor->GetBank()->m_itEqBankBox.m_Check_Amount += costtotal;
	}

	// Clear the vendor display.
	addVendorClose(pVendor);
	if ( i ) addSound( 0x057 );	// add to inv sound.
}

void CClient::Event_VendorSell( CObjUID uidVendor )
{
	// Player Selling items to the vendor.
	// Done with the selling action.

	CChar * pVendor = uidVendor.CharFind();
	if ( ! m_pChar->CanTouch( pVendor ))
	{
		SysMessage( "Too far away from the Vendor" );
		return;
	}
	if ( ! m_bin.VendorSell.m_count )
	{
		addVendorClose( pVendor );
		// pVendor->Speak( "You have sold nothing" );
		return;
	}

	CItemContainer * pBank = pVendor->GetBank();
	CItemContainer * pContStock = pVendor->GetBank( LAYER_VENDOR_STOCK );
	CItemContainer * pContBuy = pVendor->GetBank( LAYER_VENDOR_BUYS );
	CItemContainer * pContExtra = pVendor->GetBank( LAYER_VENDOR_EXTRA );
	if ( pBank == NULL || pContStock == NULL || pContExtra == NULL )
	{
		addVendorClose( pVendor );
		pVendor->Speak( "Ahh, Guards my goods are gone !!" );
		return;
	}

	// MAX_ITEMS_CONT ???

	int iGold = 0;
	bool fShortfall = false;

	for ( int i=0; i<m_bin.VendorSell.m_count; i++ )
	{
		CObjUID uid( m_bin.VendorSell.items[i].m_UID );
		CItemVendable * pItem = dynamic_cast <CItemVendable *> (uid.ItemFind());
		if ( pItem == NULL ) 
			continue;

		// Do we still have it ? (cheat check)
		if ( pItem->GetTopLevelObj() != m_pChar ) 
			continue;

		// Find the valid sell item from vendors stuff.
		if ( ! pItem->IsValidSaleItem( false )) 
			continue;

		CItemVendable * pItemSell = dynamic_cast <CItemVendable *> (pContBuy->ContentFind( pItem->GetID()));
		if ( pItemSell == NULL )
		{
			pItemSell = dynamic_cast <CItemVendable *> (pContStock->ContentFind( pItem->GetID()));
			if ( pItemSell == NULL )
				continue;
		}
		if ( pItem->m_type != pItemSell->m_type ) 
			continue;

		// Now how much did i say i wanted to sell ?
		int amount = m_bin.VendorSell.items[i].m_amount;
		if ( pItem->GetAmount() < amount )	// Selling more than i have ?
		{
			amount = pItem->GetAmount();
		}

		long iPrice = pItemSell->GetSellPrice();
		if ( pItem->GetQuality())
			iPrice = IMULDIV( iPrice, pItem->GetQuality(), 100 );
		iPrice *= amount;

		// Can vendor afford this ?
		if ( iPrice > pBank->m_itEqBankBox.m_Check_Amount )
		{
			fShortfall = true;
			break;
		}
		pBank->m_itEqBankBox.m_Check_Amount -= iPrice;

		// give them the appropriate amount of gold.
		iGold += iPrice;

		// Take the items from player.
		// Put items in vendor inventory.
		if ( amount >= pItem->GetAmount())
		{
			// Transfer all.
			pItem->SetBuyPrice( iPrice * 2 );	// so I know what to sell it for later.
			pItem->SetSellPrice( iPrice );
			pContExtra->ContentAdd( pItem );
		}
		else
		{
			// Just part of the stack.
			CItem * pItemNew = CItem::CreateDupeItem( pItem );
			pItemNew->SetAmount( amount );
			pContExtra->ContentAdd( pItemNew );
			pItem->SetAmountUpdate( pItem->GetAmount() - amount );
		}
	}

	if ( iGold )
	{
		CGString sMsg;
		sMsg.Format( "Here you are, %d gold coin%s. "
			"I thank thee for thy business.",
			iGold, (iGold==1) ? "" : "s" );
		pVendor->Speak( sMsg );

		if ( fShortfall )
		{
			pVendor->Speak( "Alas I have run out of money." );
		}

		m_pChar->AddGoldToPack( iGold );
	}
	else
	{
		if ( fShortfall )
		{
			pVendor->Speak( "I cannot afford any more at the moment" );
		}
	}

	addVendorClose( pVendor );
}

void CClient::Event_BBoardRequest( CObjUID uid )
{
	// Answer a request reguarding the BBoard.

	CItemContainer * pBoard = dynamic_cast <CItemContainer *> ( uid.ItemFind());
	if ( ! m_pChar->CanSee( pBoard ))
	{
		addObjectRemoveCantSee( uid, "the board" );
		return;
	}

	switch ( m_bin.BBoard.m_flag )
	{
	case 3:
	case 4:
		// request for message header and/or body.
		if ( m_bin.BBoard.m_len != 0x0c )
		{
			DEBUG_ERR(( "%x:BBoard feed back message bad length %d\n", GetSocket(), (int) m_bin.BBoard.m_len ));
			return;
		}
		if ( ! addBBoardMessage( pBoard, m_bin.BBoard.m_flag, (DWORD)( m_bin.BBoard.m_UIDMsg )))
		{
			// sanity check fails.
			addObjectRemoveCantSee( (DWORD)( m_bin.BBoard.m_UIDMsg ), "the message" );
			return;
		}
		break;
	case 5:
		// Submit a message
		if ( m_bin.BBoard.m_len < 0x0c )
		{
			DEBUG_ERR(( "%x:BBoard feed back message bad length %d\n", GetSocket(), (int) m_bin.BBoard.m_len ));
			return;
		}
		if ( ! m_pChar->CanTouch( pBoard ))
		{
			SysMessage( "You can't reach the bboard" );
			return;
		}
		if ( pBoard->GetCount() > 32 )
		{
			// Role a message off.
			delete pBoard->GetAt(pBoard->GetCount()-1);
		}
		// if pMsgItem then this is a reply to it !
		{
			CItemMessage * pMsgNew = dynamic_cast <CItemMessage *>( CItem::CreateBase( ITEMID_BBOARD_MSG ));
			if ( pMsgNew == NULL )
			{
				DEBUG_ERR(( "%x:BBoard can't create message item\n", GetSocket()));
				return;
			}

			int lenstr = m_bin.BBoard.m_data[0];
			pMsgNew->SetName( (const TCHAR*) &m_bin.BBoard.m_data[1] );
			pMsgNew->m_itBook.m_TimeID = g_World.GetTime() | 0x80000000;
			pMsgNew->m_sAuthor = m_pChar->GetName();
			pMsgNew->m_uidLink = m_pChar->GetUID();	// Link it to you forever.

			int len = 1 + lenstr;
			int lines = m_bin.BBoard.m_data[len++];
			if ( lines > 32 ) lines = 32;	// limit this.

			while ( lines-- )
			{
				lenstr = m_bin.BBoard.m_data[len++];
				pMsgNew->AddPageText( (const TCHAR*) &m_bin.BBoard.m_data[len] );
				len += lenstr;
			}

			pBoard->ContentAdd( pMsgNew );
		}
		break;

	default:
		DEBUG_ERR(( "%x:BBoard unknown flag %d\n", GetSocket(), (int) m_bin.BBoard.m_flag ));
		return;
	}
}

void CClient::Event_SecureTrade( CObjUID uid )
{
	// pressed a button on the secure trade window.

	CItemContainer * pCont = dynamic_cast <CItemContainer *> ( uid.ItemFind());
	if ( pCont == NULL ) 
		return;
	if ( m_pChar != pCont->GetParent()) 
		return;

	// perform the trade.
	switch ( m_bin.SecureTrade.m_action )
	{
	case 1: // Cancel trade.  Send each person cancel messages, move items.
		pCont->Delete();
		return;
	case 2: // Change check marks.  Possibly conclude trade
		if ( m_pChar->GetDist( pCont ) > UO_MAP_VIEW_SIZE )
		{
			// To far away.
			SysMessage( "You are too far away to trade items" );
			return;
		}
		pCont->Trade_Status( m_bin.SecureTrade.m_UID1 );
		return;
	}
}

void CClient::Event_MailMsg( CObjUID uid1, CObjUID uid2 )
{
	// NOTE: How do i protect this from spamming others !!!
	// Drag the mail bag to this clients char.

	CChar * pChar = uid1.CharFind();
	if ( pChar == NULL )
	{
		SysMessage( "Drop mail on other players" );
		return;
	}
	if ( pChar == m_pChar ) // this is normal (for some reason) at startup.
	{
		return;
	}

	CGString sMsg;
	sMsg.Format( "'%s' has dropped mail on you", m_pChar->GetName() );
	pChar->SysMessage( sMsg );
}

void CClient::Event_ToolTip( CObjUID uid )
{
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
		return;

	CGString sStr;
	sStr.Format( "'%s'", pObj->GetName());

	// TODO: Lookup in grayitem.scp for tooltip???
	addToolTip( uid.ObjFind(), sStr );
}

void CClient::Event_ExtData( EXTDATA_TYPE type, int len, BYTE * pData )
{
	// XCMD_ExtData
	DEBUG_MSG(( "%x:XCMD_ExtData t=%d,l=%d\n", GetSocket(), type, len ));

	switch ( type )
	{
	case EXTDATA_Party_UnkFromCli5:
	case EXTDATA_Party_UnkFromCli11:
		// Sent at start up for the party system.
		// break;
	case EXTDATA_Party_Add:
		SysMessage( "Sorry the party system is not working" );
		break;
	case EXTDATA_Arrow_Click:
		SysMessage( "Follow the Arrow" );
		break;
	default:
		SysMessage( "Unknown extended msg." );
		break;
	}

}

void CClient::Event_ExtCmd( EXTCMD_TYPE type, TCHAR * pszName )
{
	// parse the args.
	int i=0;
	for ( ; pszName[i] != '\0' && pszName[i] != ' '; i++ )
		;
	pszName[i] = '\0';

	switch ( type )
	{

	case EXTCMD_OPEN_SPELLBOOK: // 67 = open spell book if we have one.
		{
			CItem * pBook = m_pChar->GetSpellbook();
			if ( pBook == NULL )
			{
				SysMessage( "You have no spellbook" );
				break;
			}
			// Must send proper context info or it will crash tha client.
			// if ( pBook->GetParent() != m_pChar )
			{
				addItem( m_pChar->GetPackSafe());
			}
			addItem( pBook );
			addSpellbookOpen( pBook );
		}
		break;

	case EXTCMD_ANIMATE: // Cmd_Animate
		if ( !strcmpi( pszName,"bow"))
			m_pChar->UpdateAnimate( ANIM_BOW );
		else if ( ! strcmpi( pszName,"salute"))
			m_pChar->UpdateAnimate( ANIM_SALUTE );
		else
		{
			DEBUG_ERR(( "%x:Event Animate '%s'\n", GetSocket(), pszName ));
		}
		break;

	case EXTCMD_SKILL:			// Skill
		Event_Skill_Use( (SKILL_TYPE) atoi( pszName ));
		break;

	case EXTCMD_AUTOTARG:	// bizarre new autotarget mode.
		// "target x y z"
		{
			CObjUID uid( atoi( pszName ));
			CObjBase * pObj = uid.ObjFind();
			if ( pObj )
			{
				DEBUG_ERR(( "%x:Event Autotarget '%s' '%s'\n", GetSocket(), pObj->GetName(), pszName+i+1 ));
			}
			else
			{
				DEBUG_ERR(( "%x:Event Autotarget UNK '%s' '%s'\n", GetSocket(), pszName, pszName+i+1 ));
			}
		}
		break;

	case EXTCMD_CAST_MACRO:	// macro spell.
	case EXTCMD_CAST_BOOK:	// cast spell from book.
		Cmd_Skill_Magery( (SPELL_TYPE) atoi( pszName ), m_pChar );
		break;

	case EXTCMD_DOOR_AUTO: // open door macro = Attempt to open a door around us.
		if ( m_pChar )
		{
			CWorldSearch Area( m_pChar->GetTopPoint(), 4 );
			while(true)
			{
				CItem * pItem = Area.GetItem();
				if ( pItem == NULL ) 
					break;
				switch ( pItem->m_type )
				{
				case ITEM_PORT_LOCKED:	// Can only be trigered.
				case ITEM_PORTCULIS:
				case ITEM_DOOR_LOCKED:
				case ITEM_DOOR:
					m_pChar->Use_Item( pItem, false );
					return;
				}
			}
		}
		break;

	default:
		DEBUG_ERR(( "%x:Event Animate unk %d, '%s'\n", GetSocket(), type, pszName ));
	}
}

void CClient::Event_PromptResp( const TCHAR * pszText, int len )
{
	// result of addPrompt
	TCHAR szText[256];
	if ( len <= 0 )	// cancel
	{
		szText[0] = '\0';
	}
	else
	{
		len = GetBareText( szText, pszText, sizeof(szText), "|~,=[]{|}~" );
	}

	const TCHAR * pszReName = NULL;
	const TCHAR * pszPrefix = NULL;

	TARGMODE_TYPE PrvTargMode = GetTargMode();
	ClearTargMode();

	switch ( PrvTargMode )
	{
	case TARGMODE_GM_PAGE_TEXT:
		// m_Targ_Text
		Cmd_GM_Page( szText );
		return;

	case TARGMODE_VENDOR_PRICE:
		// Setting the vendor price for an item.
		{
			if ( szText[0] == '\0' )	// cancel
				return;
			CChar * pCharVendor = m_Targ_PrvUID.CharFind();
			if ( pCharVendor )
			{
				pCharVendor->NPC_SetVendorPrice( m_Targ_UID.ItemFind(), atoi(szText) );
			}
		}
		return;

	case TARGMODE_NAME_RUNE:
		pszReName = "Rune";
		pszPrefix = "Rune to:";
		break;

	case TARGMODE_NAME_KEY:
		pszReName = "Key";
		pszPrefix = "Key to:";
		break;

	case TARGMODE_NAME_SHIP:
		pszReName = "Ship";
		pszPrefix = "SS ";
		break;

	case TARGMODE_NAME_SIGN:
		pszReName = "Sign";
		pszPrefix = "";
		break;

	case TARGMODE_STONE_NAME:
		pszReName = "Stone";
		pszPrefix = "Stone for the ";
		break;

	case TARGMODE_STONE_SET_ABBREV:
		pszReName = "Abbreviation";
		pszPrefix = "";
		break;

	case TARGMODE_STONE_GRANT_TITLE:
	case TARGMODE_STONE_SET_TITLE:
		pszReName = "Title";
		pszPrefix = "";
		break;

	case TARGMODE_ENTER_TARG_VERB:
		// Send a msg to the pre-tergetted player. "ETARGVERB"
		// m_Targ_UID = the target.
		// m_Targ_Text = the prefix.
		if ( szText[0] != '\0' )
		{
			CObjBase * pObj = m_Targ_UID.ObjFind();
			if ( pObj )
			{
				pObj->r_Verb( CScript( m_Targ_Text, szText ), this );
			}
		}
		return;

	default:
		DEBUG_ERR(( "%x:Unrequested Prompt mode %d\n", GetSocket(), PrvTargMode ));
		return;
	}

	ASSERT(pszReName);

	CGString sMsg;
	CItem * pItem = m_Targ_UID.ItemFind();

	if ( pItem == NULL || szText[0] == '\0' )
	{
		SysMessagef( "%s Renaming Canceled", pszReName );
		return;
	}

	if ( g_Serv.IsObscene( szText ))
	{
		SysMessagef( "%s name %s is not allowed.", pszReName, szText );
		return;
	}

	sMsg.Format( "%s%s", pszPrefix, szText );
	switch (pItem->m_type)
	{
	case ITEM_STONE_GUILD:
	case ITEM_STONE_TOWN:
	case ITEM_STONE_ROOM:
		{
			CItemStone * pStone = dynamic_cast <CItemStone*> ( pItem );
			ASSERT(pStone);
			if ( ! pStone )
				return;
			if ( ! pStone->OnPromptResp( this, PrvTargMode, szText, sMsg ))
				return;
		}
		break;
	default:
		pItem->SetName( sMsg );
		sMsg.Format( "%s renamed: %s", pszReName, pItem->GetName());
		break;
	}

	SysMessage( sMsg );
}

void CClient::Event_Talk_Common( TCHAR * szText ) // PC speech
{
	// ??? Allow NPC's to talk to each other in the future.
	// Do hearing here so there is not feedback loop with NPC's talking to each other.
	if ( g_Serv.IsConsoleCmd( szText[0] ))
	{
		Event_Command( szText+1 );
		return;
	}

	strupr( szText );

	ASSERT( m_pChar );
	ASSERT( m_pChar->m_pPlayer );
	ASSERT( m_pChar->m_pArea );

	if ( m_pChar->m_pArea->IsFlag( REGION_FLAG_SHIP ))
	{
		// Find the ship and ask it if it likes the commands
		CObjUID uid( m_pChar->m_pArea->GetMultiUID());
		CItemMulti * pShipItem = dynamic_cast <CItemMulti *>( uid.ItemFind());
		if ( pShipItem )
		{
			if ( pShipItem->Ship_Command( szText, m_pChar ))
				return;
		}
	}

	// Special declarations

	if ( ! strnicmp( szText, "I resign from my guild", 22 ))
	{
		m_pChar->Guild_Resign(MEMORY_GUILD);
		return;
	}
	if ( ! strnicmp( szText, "I resign from my town", 21 ))
	{
		m_pChar->Guild_Resign(MEMORY_TOWN);
		return;
	}
	if ( ! strnicmp( szText, "I must consider my sins", 23 ))
	{
		const TCHAR * pszMsg;
		switch ( m_pChar->m_pPlayer->m_Murders )
		{
		case 0: pszMsg = "Thy conscience is clear.";
			break;
		case 1:	pszMsg = "Although thou hast committed murder in the past, the guards will still protect you.";
			break;
		case 2: pszMsg = "Your soul is black with guilt. Because thou hast murdered, the guards shall smite thee.";
			break;
		default: pszMsg = "Thou hast done great naughty! I wouldst not show my face in town if I were thee.";
			break;
		}
		SysMessage( pszMsg );
		return;
	}

	// They can't hear u if your dead.
	bool fGhostSpeak = m_pChar->IsSpeakAsGhost();
	if ( ! fGhostSpeak && ( FindStrWord( szText, "GUARD" ) || FindStrWord( szText, "GUARDS" )))
	{
		m_pChar->CallGuards();
	}

	// Find an NPC that may have heard us.
	CChar * pCharAlt = NULL;
	int iAltDist = UO_MAP_VIEW_SIGHT;
	CChar * pChar;
	int i=0;

	CWorldSearch AreaChars( m_pChar->GetTopPoint(), UO_MAP_VIEW_SIGHT );
	while (true)
	{
		pChar = AreaChars.GetChar();
		if ( pChar == NULL ) 
			break;
		if ( pChar == m_pChar ) 
			continue;

		if ( fGhostSpeak && ! pChar->CanUnderstandGhost()) 
			continue;

		bool fNamed = false;
		i = 0;
		if ( ! strnicmp( szText, "PETS", 4 ))
			i = 5;
		else if ( ! strnicmp( szText, "ALL", 3 ))
			i = 4;
		else
		{
			// Named the char specifically ?
			i = pChar->NPC_OnHearName( szText );
			fNamed = true;
		}
		if ( i )
		{
			while ( ISWHITESPACE( szText[i] )) i++;

			if ( pChar->NPC_OnHearPetCmd( szText+i, m_pChar ))
			{
				if ( GetTargMode() == TARGMODE_PET_CMD || fNamed )
					return;
				// The command might apply to other pets.
				continue;
			}
			if ( fNamed ) break;
		}

		// Are we close to the char ?
		int iDist = m_pChar->GetTopDist3D( pChar );

		if ( pChar->GetActiveSkill() == NPCACT_TALK && pChar->m_Act_Targ == m_pChar->GetUID()) // already talking to him
		{
			pCharAlt = pChar;
			iAltDist = 1;
		}
		else if ( pChar->IsClient() && iAltDist >= 2 )	// PC's have higher priority
		{
			pCharAlt = pChar;
			iAltDist = 2;	// high priority
		}
		else if ( iDist < iAltDist )	// closest NPC guy ?
		{
			pCharAlt = pChar;
			iAltDist = iDist;
		}

		// NPC's with special key words ?
		if ( pChar->m_pNPC )
		{
			if ( pChar->m_pNPC->m_Brain == NPCBRAIN_BANKER )
			{
				if ( FindStrWord( szText, "BANK" ))
					break;
			}
			else if ( pChar->m_pNPC->m_Brain == NPCBRAIN_GUARD )
			{
				if ( FindStrWord( szText, "GUARD" ))
					break;
			}
			else if ( pChar->m_pNPC->IsVendor())
			{
				if ( FindStrWord( szText, "VENDOR" ))
					break;
			}
		}
	}

	if ( pChar == NULL )
	{
		i = 0;
		pChar = pCharAlt;
		if ( pChar == NULL )
			return;	// no one heard it.
	}

	// The char hears you say this.
	pChar->NPC_OnHear( &szText[i], m_pChar );
}

void CClient::Event_Talk( const TCHAR * pszText, COLOR_TYPE color, TALKMODE_TYPE mode ) // PC speech
{
	if ( ! m_pAccount )
		return;
	ASSERT( m_pChar );

	// store the language of choice.
	m_pAccount->m_lang[0] = 0;	// default.

	// Rip out the unprintables first.
	TCHAR szText[MAX_TALK_BUFFER];
	int len = GetBareText( szText, pszText, sizeof(szText));
	if ( len <= 0 ) 
		return;
	pszText = szText;

	if ( ! g_Serv.IsConsoleCmd( pszText[0] ))
	{
		if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
		{
			g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says '%s' mode=%d\n", GetSocket(), m_pChar->GetName(), szText, mode );
		}
		m_pChar->Speak( szText, color, mode );	// echo this back.
	}
	else
	{

	}

	Event_Talk_Common( (TCHAR *) pszText );
}

void CClient::Event_TalkUNICODE()
{
	// Get the text in wide bytes.
	// ENU = English
	// FRC = French

	if ( ! m_pAccount )
		return;

	TCHAR szText[MAX_TALK_BUFFER];
	int iLen = CvtNUNICODEToSystem( szText, m_bin.TalkUNICODE.m_utext, sizeof( szText ));
	if ( iLen <= 0 )
		return;

	m_bin.TalkUNICODE.m_utext[ iLen ] = '\0';

	if ( ! strnicmp( m_bin.TalkUNICODE.m_lang, "ENU", 3 ))
	{
		// It's just english anyhow.
		Event_Talk( szText, m_bin.TalkUNICODE.m_color, (TALKMODE_TYPE)( m_bin.TalkUNICODE.m_mode ));
		return;
	}

	// store the language of choice.
	strncpy( m_pAccount->m_lang, m_bin.TalkUNICODE.m_lang, sizeof(m_pAccount->m_lang));
	m_pAccount->m_lang[sizeof(m_pAccount->m_lang)-1] = '\0';

	// Non-english.
	if ( ! g_Serv.IsConsoleCmd( szText[0] ))
	{
		if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
		{
			g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says UNICODE '%s' '%s' mode=%d\n", GetSocket(), m_pChar->GetName(), m_pAccount->m_lang, szText, m_bin.Talk.m_mode );
		}
		m_pChar->SpeakUNICODE( m_bin.TalkUNICODE.m_utext, m_bin.Talk.m_color, (TALKMODE_TYPE) m_bin.Talk.m_mode, m_pAccount->m_lang );
	}

	Event_Talk_Common( szText );
}

bool CClient::Event_DeathOption( BYTE mode )
{
	if ( m_pChar == NULL ) 
		return false;
	if ( mode )
	{
		// Death menu option.
		if ( mode == 1 ) // res w/penalties ?
		{
			// Insta res should go back to the spot of death !
			static const TCHAR * Res_Fail_Msg[] =
			{
				"The connection between your spirit and the world is too weak.",
				"Thou hast drained thyself in thy efforts to reassert thine lifeforce in the physical world.",
				"No matter how strong your efforts, you cannot reestablish contact with the physical world.",
			};
			if ( GetTargMode() != TARGMODE_DEATH )
			{
				SysMessage( Res_Fail_Msg[ GetRandVal( COUNTOF( Res_Fail_Msg )) ] );
			}
			else
			{
				SetTargMode();
				m_pChar->MoveTo( m_Targ_p ); // Insta res takes us back to where we died.
				if ( ! m_pChar->Spell_Resurrection( 10 ))
				{
					SysMessage( Res_Fail_Msg[ GetRandVal( COUNTOF( Res_Fail_Msg )) ] );
				}
			}
		}
		else
		{
			// Play as a ghost.
			// "As the mortal pains fade, you become aware of yourself as a spirit."
			SysMessage( "You are a ghost" );
			addSound( 0x17f );	// Creepy noise.
		}

		addPlayerStart( m_pChar ); // Do practically a full reset (to clear death menu)
		return( true );
	}

	// Toggle manifest mode.
	if ( ! xCheckSize( sizeof( m_bin.DeathMenu ))) 
		return(false);
	Event_CombatMode( m_bin.DeathMenu.m_manifest );
	return( true );
}

void CClient::Event_SetName( CObjUID uid )
{
	// Set the name in the character status window.
	ASSERT( m_pChar );
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) 
		return;

	// Do we have the right to do this ?
	if ( ! pChar->NPC_IsOwnedBy( m_pChar ))
		return;

	if ( g_Serv.IsObscene( m_bin.CharName.m_name ))
		return;

	pChar->SetName( m_bin.CharName.m_name );
}

void CClient::Event_GumpTextIn()
{
	// Text was typed into the gump on the screen.
	// m_bin.GumpText
	// result of addGumpInputBox. GumpInputBox

	TARGMODE_TYPE dialog = (TARGMODE_TYPE) (DWORD) m_bin.GumpText.m_dialogID;

	BYTE parent = m_bin.GumpText.m_parentID; // the original dialog #, shortened to a BYTE
	BYTE button = m_bin.GumpText.m_buttonID;

	BYTE retcode = m_bin.GumpText.m_retcode; // 0=canceled, 1=okayed
	WORD textlen = m_bin.GumpText.m_textlen; // length of text entered
	TCHAR * pszText = m_bin.GumpText.m_text;

	CGString sStr;

	if ( IsPriv( PRIV_DETAIL ))
	{
		SysMessagef( "gumptext parent:%i, button:%i", parent, button );
	}

	if ( GetTargMode() != TARGMODE_GUMP_VAL || dialog != TARGMODE_GUMP_VAL )
	{
		DEBUG_ERR(( "%x:Event_GumpTextIn unexpected input %d %d!=%d\n", GetSocket(), dialog, parent, GetTargMode()));
		SysMessage( "Unexpected text input" );
		return;
	}

	ClearTargMode();

	if (retcode == 1)
	{
		switch ( parent )
		{
		case TARGMODE_GUMP_PROP1: 
		case TARGMODE_GUMP_PROP2:
		case TARGMODE_GUMP_PROP3:
		case TARGMODE_GUMP_PROP4:
		case TARGMODE_GUMP_PROP5:
			{
				// Properties Dialog, page x
				// m_Targ_Text = the verb we are dealing with.
				// m_Prop_UID = object we are after.

				CObjBase * pObj = m_Prop_UID.ObjFind();
				if ( pObj == NULL )
					return;

				if ( IsPriv( PRIV_DETAIL ))
				{
					SysMessagef( "set: %s = %s", (const TCHAR*) m_Targ_Text, pszText );
				}

				if ( ! pObj->r_Verb( CScript( m_Targ_Text, pszText ), m_pChar ))
				{
					SysMessagef( "Invalid set: %s = %s", (const TCHAR*) m_Targ_Text, pszText );
				}
				else
				{
					pObj->RemoveFromView(); // weird client thing
					pObj->Update();
				}
				addGumpPropMenu( (TARGMODE_TYPE) parent );	// put back the client.
			}
			break;
		}
	}
	else
	{
		addGumpPropMenu( (TARGMODE_TYPE) parent );
	}
	return;
}

void CClient::Event_GumpButton()
{
	// A button was pressed in a gump on the screen.
	// possibly multiple check boxes.

	// First let's completely decode this packet
	TARGMODE_TYPE dialog = (TARGMODE_TYPE)(DWORD)( m_bin.GumpButton.m_dialogID );
	CObjUID uid = (DWORD) m_bin.GumpButton.m_UID;
	DWORD dwButtonID = m_bin.GumpButton.m_buttonID;

	if ( dialog != GetTargMode())
	{
		DEBUG_ERR(( "%x: Event_GumpButton unexpected input %d!=%d\n", GetSocket(), dialog, GetTargMode()));
		SysMessage( "Unexpected button input" );
		return;
	}

	CObjBase * pObj = uid.ObjFind();

	DWORD iCheckID[32]; // This should do for most pages...
	DWORD iCheckQty = m_bin.GumpButton.m_checkQty; // this has the total of all checked boxes and radios
	ASSERT( iCheckQty < COUNTOF(iCheckID));
	int i = 0;
	for ( ; i < iCheckQty; i++ ) // Store the returned checked boxes' ids for possible later use
	{
		iCheckID[i] = m_bin.GumpButton.m_checkIds[i];
	}

	// Find out how many textentry boxes we have that returned data
	CEvent * pMsg = (CEvent *)(((BYTE*)(&m_bin))+(iCheckQty-1)*sizeof(m_bin.GumpButton.m_checkIds[0]));
	DWORD iTextQty = pMsg->GumpButton.m_textQty;

	WORD iTextID[96]; // Store textentry boxes' ids in here
	CGString strText[COUNTOF(iTextID)]; // Store the text in here
	ASSERT( iTextQty < COUNTOF(iTextID));

	for ( i = 0; i < iTextQty; i++)
	{
		// Get an id
		iTextID[i] = pMsg->GumpButton.m_texts[0].m_id;

		// Get the length....no need to store this permanently
		int lenstr = pMsg->GumpButton.m_texts[0].m_len;

		TCHAR scratch[256]; // use this as scratch storage
		ASSERT( lenstr < COUNTOF(scratch));

		// Do a loop and "convert" from unicode to normal ascii
		CvtNUNICODEToSystem( scratch, pMsg->GumpButton.m_texts[0].m_utext, lenstr+1 );
		strText[i] = scratch; // Store it here

		lenstr = sizeof(pMsg->GumpButton.m_texts[0]) + ( lenstr - 1 ) * sizeof(NCHAR);
		pMsg = (CEvent *)(((BYTE*)pMsg)+lenstr);
	}

#if 0 // def _DEBUG
	// Dennis, can you leave this in? I use it to track down stuff
	DEBUG_MSG(("uid: %x, dialog: %i, gump: %i\n", (DWORD) uid, dialog, dwButtonID ));
	if (iCheckQty > 0)
	{
		DEBUG_MSG(("%i boxes are checked.\n", iCheckQty));
		for(int i = 0; i < iCheckQty; i++)
		{
			DEBUG_MSG(( "box[%i]: id = %i\n", i, iCheckID[i] ));
		}
	}
#endif

	ClearTargMode();

	switch ( dialog ) // This is the page number
	{
	case TARGMODE_GUMP_PROP1:
	case TARGMODE_GUMP_PROP2:
	case TARGMODE_GUMP_PROP3:
	case TARGMODE_GUMP_PROP4:
	case TARGMODE_GUMP_PROP5:
		switch ( dwButtonID ) // Handle props dialog nav. buttons here
		{
		case 0: // Right click, cancel and discard any changes
			m_Prop_UID.ClearUID(); // no longer in /props dialog
			addSkillWindow( SKILL_QTY ); // Reload the real skills
			return;
		case 900: // Cancel and discard any changes
clear_dialog:
			m_Prop_UID.ClearUID(); // no longer in /props dialog
			addSkillWindow( SKILL_QTY ); // Reload the real skills
			return;
		case 901: // Apply changes and return
			// TODO: apply changes here
			addGumpPropMenu();
			return;
		case 902: // Save and dismiss
			goto clear_dialog;
		}
		break;

	case TARGMODE_GUMP_ADMIN: // Admin console stuff comes here
		{
			switch ( dwButtonID ) // Button presses come here
			{
			case 801: // Previous page
				add_Admin_Dialog( m_adminPage - 1 );
				break;
			case 802: // Next page
				add_Admin_Dialog( m_adminPage + 1 );
				break;
			case 901: // Open up the options page for this client
			case 902:
			case 903:
			case 904:
			case 905:
			case 906:
			case 907:
			case 908:
			case 909:
			case 910:
				dwButtonID -= 901;
				if ( dwButtonID >= COUNTOF( m_tmMenu ))
					return;
				DEBUG_CHECK( dwButtonID < ADMIN_CLIENTS_PER_PAGE );
				m_Targ_UID = m_tmMenu[dwButtonID];
				if ( m_Targ_UID.IsValidUID())
					Cmd_Script_Menu( (TARGMODE_TYPE)( TARGMODE_MENU_ADMIN + 1 ));
				else
					add_Admin_Dialog( 0 );
				break;
			}
		}
		return;

	case TARGMODE_GUMP_GUILD: // Guild/Leige/Townstones stuff comes here
		{
			CItemStone * pStone = dynamic_cast <CItemStone *> ( m_Targ_UID.ItemFind());
			if ( pStone == NULL )
				return;
			if ( pStone->OnGumpButton( this, (STONEGUMP_TYPE) dwButtonID, iCheckID, iCheckQty, strText, iTextID, iTextQty ))
				return;
		}
		break;

	case TARGMODE_GUMP_HAIR_DYE: // Using hair dye
		{
			if (dwButtonID == 0) // They cancelled
				return;
			if (iCheckID == 0) // They didn't pick a color, but they hit OK
				return;
			CItem * pItem = m_Targ_UID.ItemFind();
			if ( pItem == NULL ) 
				return;
			CItem * pFacialHair = m_pChar->LayerFind( LAYER_BEARD );
			CItem * pHeadHair = m_pChar->LayerFind( LAYER_HAIR );
			if (!pFacialHair && !pHeadHair)
			{
				SysMessage("You have no hair to dye!");
				return;
			}
			if (pFacialHair)
				pFacialHair->SetColor( iCheckID[0] + 1 );
			if (pHeadHair)
				pHeadHair->SetColor( iCheckID[0] + 1 );
			m_pChar->Update();
			m_pChar->Use_Consume(pItem);// Consume it
		}
		return;

	case TARGMODE_GUMP_TOME:	// The new spell book.
		// ??? Make this just a table of triggers.
		if (dwButtonID <= 99) // Casting a spell
		{
			// TODO: cast the spell
			break;
		}
		else if (dwButtonID <= 199) // Open a page in spell book
		{
			m_pChar->Sound( 0x55 );
			Gump_Setup( (TARGMODE_TYPE) dwButtonID, pObj );
			return;
		}
		else if (dwButtonID <= 299) // Making an icon
		{
			Gump_Setup( (TARGMODE_TYPE) dwButtonID, pObj  );
			return;
		}
		else if (dwButtonID <= 399) // Casting from icon
		{
			Gump_Setup( (TARGMODE_TYPE)( dwButtonID - 100 ), pObj );
			// TODO: cast the spell
			return;
		}
		break;
	}

	// Lose all the checks and text.
	Gump_Button( dialog, dwButtonID, pObj );
}

bool CClient::Cmd_Use_ItemID( CItem * pItem, ITEMID_TYPE id )
{
	// Items that have a use specific to their id.
	// NOTE: The correct way to do this is the m_type or TYPE feild !
	//

	DEBUG_CHECK( m_Targ_UID == pItem->GetUID());
	m_pChar->m_atCreate.m_Color = 0;	// ??? This makes no sense ?

	switch ( id )
	{
		// Solve for the combination of things.

	case ITEMID_BOTTLE1_DYE:
	case ITEMID_BOTTLE2_DYE:
		if (!m_pChar->LayerFind( LAYER_BEARD ) && !m_pChar->LayerFind( LAYER_HAIR ))
		{
			SysMessage("You have no hair to dye!");
			return true;
		}
		Gump_Setup( TARGMODE_GUMP_HAIR_DYE, m_pChar );
		return true;
	case ITEMID_DYE:
		addTarget( TARGMODE_USE_ITEM, "Which dye vat will you use this on?" );
		return true;
	case ITEMID_DYEVAT:
		addTarget( TARGMODE_USE_ITEM, "Select the object to use this on." );
		return true;
	case ITEMID_EMPTY_BOTTLE:
		addTarget( TARGMODE_USE_ITEM, "What would you like to make the potion out of?" );
		return true;
	case ITEMID_MORTAR:
		// If we have a mortar then do alchemy.
		addTarget( TARGMODE_USE_ITEM, "What reagent you like to make a potion out of?" );
		return true;
	case ITEMID_FISH_POLE1:	// Just be near water ?
	case ITEMID_FISH_POLE2:
		addTarget( TARGMODE_USE_ITEM, "Where would you like to fish?", true );
		return true;

	case ITEMID_Pitchfork:
	case ITEMID_PITCHER:
		{
			CGString sTmp;
			sTmp.Format( "Where do you want to use the %s?", pItem->GetName());
			addTarget( TARGMODE_USE_ITEM, sTmp, true );
		}
		return true;

	case ITEMID_BANDAGES1:	// SKILL_HEALING, or SKILL_VETERINARY
	case ITEMID_BANDAGES2:
	case ITEMID_BANDAGES_BLOODY1:
	case ITEMID_BANDAGES_BLOODY2:
		// Clean the bandages.
	case ITEMID_YARN1:
	case ITEMID_YARN2:
	case ITEMID_YARN3:
	case ITEMID_THREAD1:
	case ITEMID_THREAD2:
		// Use this on a loom.
	case ITEMID_WOOL:
	case ITEMID_WOOL2:
	case ITEMID_COTTON:
	case ITEMID_COTTON_RAW:
		// use on a spinning wheel.
	case ITEMID_LOCKPICK1:
		// Use on a locked thing.
	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
	case ITEMID_SCISSORS1: // cut hair and beard ?
	case ITEMID_SCISSORS2: // cut up cloth to make bandages ?
	case ITEMID_SCISSORS3: // cut hides into strips of leather?
	case ITEMID_SCISSORS4:
	case ITEMID_Cannon_Ball:
		{
			CGString sTmp;
			sTmp.Format( "What do you want to use the %s on?", pItem->GetName());
			addTarget( TARGMODE_USE_ITEM, sTmp );
		}
		return true;

	case ITEMID_DEED1:
	case ITEMID_DEED2:
	case ITEMID_SHIP_PLANS1:
	case ITEMID_SHIP_PLANS2:
		addTargetDeed( pItem );
		return true;

		// Put up menus to better decide what to do.

	case ITEMID_TINKER:	// Tinker tools.
		Cmd_Skill_Menu( SKILL_TINKERING );
		return true;
	case ITEMID_SEWINGKIT:	// Sew with materials we have on hand.
		// Cmd_Skill_Menu( SKILL_TAILORING );
		{
			CGString sTmp;
			sTmp.Format( "What do you want to use the %s on?", pItem->GetName());
			addTarget( TARGMODE_USE_ITEM, sTmp );
		}
		return true;
	case ITEMID_SCROLL1_BLANK:
	case ITEMID_SCROLL2_BLANK:
		Cmd_Skill_Inscription( 0, -1 );
		return true;
	case ITEMID_SHAFTS:
	case ITEMID_SHAFTS2:
	case ITEMID_SHAFTS3:
	case ITEMID_SHAFTS4:
	case ITEMID_SHAFTS5:
	case ITEMID_FEATHERS:
	case ITEMID_FEATHERS2:
	case ITEMID_FEATHERS3:
	case ITEMID_FEATHERS4:
	case ITEMID_FEATHERS5:
		Cmd_Skill_Menu( SKILL_BOWCRAFT, 1 );
		return true;
	}

	return( m_pChar->Use_Item( pItem ));
}

bool CClient::Cmd_Use_Item( CItem * pItem, bool fTestTouch )
{
	if ( pItem == NULL )
		return false;

	if ( fTestTouch )
	{
		// CanTouch handles priv level compares for chars
		if ( ! m_pChar->CanTouch( pItem ))
		{
			SysMessage(( m_pChar->IsStat( STATF_DEAD )) ?
				"Your ghostly hand passes through the object." :
				"You can't reach that." );
			return false;
		}

		// The item is on another character.
		CObjBaseTemplate * pObjTop = pItem->GetTopLevelObj();
		if ( ! pItem->IsTopLevel() &&
			pObjTop != m_pChar &&
			! IsPriv( PRIV_GM ) &&
			pItem->m_type != ITEM_CONTAINER &&
			pItem->m_type != ITEM_BOOK )
		{
			SysMessage( "You can't use this where it is." );
			return false;
		}
	}

	if ( pItem->m_pDef->m_layer != LAYER_NONE &&	// It is equippable.
		pItem->m_type != ITEM_LIGHT_OUT &&
		pItem->m_type != ITEM_LIGHT_LIT &&
		pItem->m_type != ITEM_CONTAINER &&
		pItem->m_type != ITEM_CONTAINER_LOCKED &&
		pItem->m_type != ITEM_SPELLBOOK &&
		! pItem->IsEquipped())
	{
		if ( ! m_pChar->CanMove( pItem ) ||
			! m_pChar->ItemEquip( pItem ))
		{
			SysMessage( "The item should be equipped to use." );
			return false;
		}
	}

	SetTargMode();
	m_Targ_UID = pItem->GetUID();	// probably already set anyhow.
	m_tmUseItem.m_pParent = pItem->GetParent();	// Cheat Verify.

	if ( pItem->OnTrigger( ITRIG_DCLICK, m_pChar ))
	{
		return true;
	}

	// Use types of items. (specific to client)
	switch ( pItem->m_type )
	{
	case ITEM_TRACKER:
		{
			DIR_TYPE dir = (DIR_TYPE) ( DIR_QTY + 1 );	// invalid value.
			if ( ! m_pChar->Skill_Tracking( pItem->m_uidLink, dir ))
			{
				if ( pItem->m_uidLink.IsValidUID())
				{
					SysMessage( "You cannot locate your target" );
				}
				m_Targ_UID = pItem->GetUID();
				addTarget( TARGMODE_LINK, "Who do you want to attune to ?" );
			}
		}
		return true;

	case ITEM_METRONOME:
		pItem->OnTick();
		return true;

	case ITEM_CONTAINER_LOCKED:
		if ( ! m_pChar->GetPackSafe()->ContentFindKeyFor( pItem ))
		{
			SysMessage( "This item is locked.");
			if ( ! IsPriv( PRIV_GM )) 
				return false;
		}

	case ITEM_CORPSE:
	case ITEM_CONTAINER:
		{
			switch ( pItem->GetID())
			{
			case ITEMID_KEY_RING0:
			case ITEMID_KEY_RING1:
			case ITEMID_KEY_RING3:
			case ITEMID_KEY_RING5:
				goto use_default;
			case ITEMID_BANK_BOX:
			case ITEMID_VENDOR_BOX:
				g_Log.Event( LOGL_WARN|LOGM_CHEAT, "%x:Cheater '%s' is using 3rd party tools to open bank box\n", GetSocket(), GetAccount()->GetName());
				return false;
			}
			CItemContainer * pPack = dynamic_cast <CItemContainer *>(pItem);
			if ( ! Check_Snoop_Container( pPack ))
			{
				if ( ! addContainerSetup( pPack ))
					return false;
			}
		}
		return true;

	case ITEM_GAME_BOARD:
		if ( ! pItem->IsTopLevel())
		{
			SysMessage( "Can't open game board in a container" );
			return false;
		}
		{
			CItemContainer* pBoard = dynamic_cast <CItemContainer *>(pItem);
			pBoard->Game_Create();
			addContainerSetup( pBoard );
		}
		return true;

	case ITEM_BBOARD:
		addBulletinBoard( dynamic_cast<CItemContainer *>(pItem));
		return true;

	case ITEM_SIGN_GUMP:
		// Things like grave stones and sign plaques.
		// Need custom gumps.
		addGumpTextDisp( pItem,
			( pItem->m_itSign.m_gump ) ? pItem->m_itSign.m_gump : GUMP_PLAQUE_C,
			pItem->GetName(),
			( pItem->IsIndividualName()) ? pItem->GetName() : NULL );
		return true;

	case ITEM_BOOK:
	case ITEM_MESSAGE:
	case ITEM_EQ_NPC_SCRIPT:
		if ( ! addBookOpen( pItem ))
		{
			SysMessage( "The book apears to be ruined!" );
		}
		return true;

	case ITEM_STONE_GUILD:
	case ITEM_STONE_TOWN:
	case ITEM_STONE_ROOM:
		// Guild and town stones.
		{
			CItemStone * pStone = dynamic_cast<CItemStone*>(pItem);
			if ( pStone == NULL ) 
				break;
			pStone->Use_Item( this );
		}
		return true;

	case ITEM_ADVANCE_GATE:
		// Upgrade the player to the skills of the selected NPC script.
		m_pChar->Use_AdvanceGate( pItem );
		return true;

	case ITEM_REAGENT:
		// Make a potion with this. (The best type we can)
		if ( ! m_pChar->ContentFind(ITEMID_MORTAR))
		{
			SysMessage( "You have no mortar and pestle." );
			return false;
		}
		m_pChar->m_Act_TargPrv.ClearUID();
		Cmd_Skill_Alchemy( pItem );
		return true;

	case ITEM_POTION:
		if ( ! m_pChar->CanMove( pItem ))	// locked down decoration.
		{
			SysMessage( "You can't move the item." );
			return false;
		}
		if ( pItem->m_itPotion.m_Type >= POTION_POISON_LESS &&
			pItem->m_itPotion.m_Type <= POTION_POISON_DEADLY )
		{
			// ask them what they want to use the poison on ?
			// Poisoning or poison self ?
			addTarget( TARGMODE_USE_ITEM, "What do you want to use this on?" );
			return true;
		}
		else if ( pItem->m_itPotion.m_Type >= POTION_EXPLODE_LESS &&
			pItem->m_itPotion.m_Type <= POTION_EXPLODE_GREAT )
		{
			// Throw explode potion.
			if ( ! m_pChar->ItemPickup( pItem, 1 ))
				return true;
			m_tmUseItem.m_pParent = pItem->GetParent();	// Cheat Verify FIX.

			addTarget( TARGMODE_USE_ITEM, "Where do you want to throw the potion?", true );
			// Put the potion in our hand as well. (and set it's timer)
			pItem->m_itPotion.m_tick = 4;	// countdown to explode purple.
			pItem->SetTimeout( 2 * TICK_PER_SEC );
			pItem->m_uidLink = m_pChar->GetUID();
			return true;
		}
		// if ( m_pChar->Use_Drink( pItem ))
		//	return;
		break;

	case ITEM_ANIM_ACTIVE:
		SysMessage( "The item is in use" );
		return false;

	case ITEM_CLOCK:
		addObjMessage( m_pChar->GetTopSector()->GetGameTime(), pItem );
		return true;

	case ITEM_SPAWN_CHAR:
		SysMessage( "You negate the spawn" );
		pItem->Spawn_KillChildren();
		return true;

	case ITEM_SPAWN_ITEM:
		SysMessage( "You trigger the spawn." );
		pItem->Spawn_OnTick( true );
		return true;

	case ITEM_SHRINE:
		if ( m_pChar->Spell_Resurrection( 0 ))
			return true;
		SysMessage( "You have a feeling of holiness" );
		return true;

	case ITEM_SHIP_TILLER:
		// dclick on tiller man.
		pItem->Speak( "Arrg stop that.", 0, TALKMODE_SAY, FONT_NORMAL );
		return true;

		// A menu or such other action ( not instantanious)

	case ITEM_WAND:
	case ITEM_SCROLL:	// activate the scroll.
		return Cmd_Skill_Magery( (SPELL_TYPE) pItem->m_itWeapon.m_spell, pItem );

	case ITEM_RUNE:
		// name the rune.
		if ( ! m_pChar->CanMove( pItem, true ))
		{
			return false;
		}
		addPromptConsole( TARGMODE_NAME_RUNE, "What is the new name of the rune ?" );
		return true;

	case ITEM_CARPENTRY:
		// Carpentry type tool
		Cmd_Skill_Menu( SKILL_CARPENTRY );
		return true;

		// Solve for the combination of this item with another.
	case ITEM_FORGE:
		addTarget( TARGMODE_USE_ITEM, "Select ore to smelt." );
		return true;
	case ITEM_ORE:
		// just use the nearest forge.
		return m_pChar->Skill_MiningSmelt( pItem, NULL );
	case ITEM_INGOT:
		return Cmd_Skill_Smith( pItem );

	case ITEM_KEY:
		addTarget( TARGMODE_USE_ITEM, "Select item to use the key on." );
		return true;
	case ITEM_CARPENTRY_CHOP:
	case ITEM_WEAPON_MACE_STAFF:
	case ITEM_WEAPON_MACE_SMITH:	// Can be used for smithing ?
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:		// 23 =
	case ITEM_WEAPON_FENCE:		// 24 = can't be used to chop trees.
		addTarget( TARGMODE_USE_ITEM, "What do you want to use this on?" );
		return true;

	case ITEM_FOOD_RAW:
		addTarget( TARGMODE_USE_ITEM, "What do you want to cook this on?" );
		return true;
	case ITEM_FISH:
		SysMessage( "Use a knife to cut this up" );
		return true;
	case ITEM_TELESCOPE:
		// Big telescope.
		SysMessage( "Wow you can see the sky!" );
		return true;
	case ITEM_MAP:
		if ( ! pItem->m_itMap.m_right && ! pItem->m_itMap.m_bottom )
		{
			Cmd_Skill_Menu( SKILL_CARTOGRAPHY );
		}
		else if ( pItem->GetTopLevelObj() != m_pChar )	// must be on your person.
		{
			SysMessage( "You must possess the map to get a good look at it." );
		}
		else
		{
			addMap( dynamic_cast <CItemMap*>( pItem ));
		}
		return true;

	case ITEM_CANNON_MUZZLE:
		// Make sure the cannon is loaded.
		if ( ! ( pItem->m_itCannon.m_Load & 1 ))
		{
			addTarget( TARGMODE_USE_ITEM, "The cannon needs powder" );
			return true;
		}
		if ( ! ( pItem->m_itCannon.m_Load & 2 ))
		{
			addTarget( TARGMODE_USE_ITEM, "The cannon needs shot" );
			return true;
		}
		addTarget( TARGMODE_USE_ITEM, "Armed and ready. What is the target?" );
		return true;

	case ITEM_CRYSTAL_BALL:
		// Gaze into the crystal ball.

		return true;

	case ITEM_ITEM_STONE:
		// Give them this item
		m_pChar->ItemBounce( CItem::CreateTemplate( pItem->m_itItemStone.m_ItemID, m_pChar->GetPackSafe()));
		m_pChar->m_pPlayer->m_Plot1 |= pItem->m_itItemStone.m_Plot1;
		return true;

	case ITEM_WEAPON_MACE_CROOK:
		addTarget( TARGMODE_USE_ITEM, "What would you like to herd?" );
		return true;

	case ITEM_WEAPON_MACE_PICK:
		{
		// Mine at the location.
			CGString sTmp;
			sTmp.Format( "Where do you want to use the %s?", pItem->GetName());
			addTarget( TARGMODE_USE_ITEM, sTmp, true );
		}
		return true;

	case ITEM_SPELLBOOK:
		addSpellbookOpen( pItem );
		return true;
	}

use_default:

	// unique activated items.
	if ( Cmd_Use_ItemID( pItem, pItem->m_pDef->GetDispID()))
		return true;
	SysMessage( "You can't think of a way to use that item.");
	return( false );
}

bool CClient::Event_DoubleClick( CObjUID uid, bool fMacro, bool fTestTouch )
{
	// Try to use the object in some way.
	// will trigger a OnTarg_Use_Item() ussually.
	// fMacro = ALTP vs dbl click. no unmount.

	// Allow some static in game objects to have function?
	// Not possible with dclick.

	ASSERT(m_pChar);
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_DoubleClick 0%x\n", GetSocket(), (DWORD) uid ));
	}

	CObjBase * pObj = uid.ObjFind();
	if ( ! m_pChar->CanSee( pObj ))
	{
		addObjectRemoveCantSee( uid, "the target" );
		return false;
	}

	// Face the object we are using/activating.
	SetTargMode();
	m_Targ_UID = uid;
	m_pChar->UpdateDir( pObj );

	if ( pObj->IsItem())
	{
		return Cmd_Use_Item( dynamic_cast <CItem *>(pObj), fTestTouch );
	}

	CChar * pChar = dynamic_cast <CChar*>(pObj);
	if ( ! fMacro )
	{
		if ( pChar == m_pChar )
		{
			if ( m_pChar->Horse_UnMount())
				return true;
		}
		if ( pChar->GetCreatureType() != NPCBRAIN_HUMAN &&
			pChar->m_pNPC )
		{
			if ( m_pChar->Horse_Mount( pChar ))
				return true;
			switch ( pChar->GetID())
			{
			case CREID_VORTEX:
			case CREID_BLADES:
				return false;
			case CREID_HORSE_PACK:
			case CREID_LLAMA_PACK:
				// pack animals open container.
				return Cmd_Use_Item( pChar->GetPackSafe(), fTestTouch );
			default:
				return false;
			}
		}
	}

	// open paper doll.

	CCommand cmd;
	cmd.PaperDoll.m_Cmd = XCMD_PaperDoll;
	cmd.PaperDoll.m_UID = pChar->GetUID();

	if ( pChar->IsStat( STATF_Incognito ))
	{
		strcpy( cmd.PaperDoll.m_text, pChar->GetName());
	}
	else
	{
		int len = 0;
		CItemStone * pStone = pChar->Guild_Find(MEMORY_GUILD);
		if ( pStone )
		{
			CStoneMember * pMember = pStone->GetMember(pChar);
			if ( pMember && pMember->IsAbbrevOn() && pStone->GetAbbrev()[0] )
			{
				len = sprintf( cmd.PaperDoll.m_text, "%s [%s], %s",
					pChar->Noto_GetTitle(), pStone->GetAbbrev(),
					pMember->GetTitle()[0] ? pMember->GetTitle() : pChar->GetTradeTitle());
			}
		}
		if ( ! len )
		{
			sprintf( cmd.PaperDoll.m_text, "%s, %s", pChar->Noto_GetTitle(), pChar->GetTradeTitle());
		}
	}

	cmd.PaperDoll.m_text[ sizeof(cmd.PaperDoll.m_text)-1 ] = '\0';
	cmd.PaperDoll.m_mode = pChar->GetModeFlag();	// 0=normal, 0x40 = attack
	xSendPkt( &cmd, sizeof( cmd.PaperDoll ));
	return( true );
}

void CClient::Event_SingleClick( CObjUID uid )
{
	// the ALLNAMES macro comes thru here.
	ASSERT(m_pChar);
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_SingleClick %lx\n", GetSocket(), (DWORD) uid ));
	}

	CObjBase * pObj = uid.ObjFind();
	if ( ! m_pChar->CanSee( pObj ))
	{
		// ALLNAMES makes this happen as we are running thru an area.
		// So display no msg.
		addObjectRemove( uid );
		return;
	}

	if ( pObj->IsItem())
	{
		addItemName( dynamic_cast <CItem *>(pObj));
		return;
	}

	if ( pObj->IsChar())
	{
		addCharName( dynamic_cast <CChar*>(pObj) );
		return;
	}

	SysMessagef( "Bogus item uid=0%x?", uid );
}

void CClient::Event_Target()
{
	// XCMD_Target
	// If player clicks on something with the targetting cursor
	// Assume addTarget was called before this.
	// NOTE: Make sure they can actually validly trarget this item !

	ASSERT(m_pChar);
	if ( m_bin.Target.m_code != GetTargMode())
	{
		DEBUG_ERR(( "%x: Unrequested target info ?\n", GetSocket()));
		SysMessage( "Unexpected target info" );
		return;
	}
	if ( m_bin.Target.m_x == 0xFFFF && m_bin.Target.m_UID == 0 )
	{
		// canceled
		SetTargMode();
		return;
	}

	CObjUID uid( m_bin.Target.m_UID );
	CPointMap pt( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	ITEMID_TYPE id = (ITEMID_TYPE)(WORD) m_bin.Target.m_id;	// if static tile.

	TARGMODE_TYPE prevmode = GetTargMode();
	ClearTargMode();

	if ( uid.IsValidUID() && ! IsPriv( PRIV_GM ))
	{
		if ( ! m_pChar->CanSee( uid.ObjFind()))
		{
			addObjectRemoveCantSee( uid, "the target" );
			return;
		}
	}

	bool fSuccess = false;

	switch ( prevmode )
	{
		// GM stuff.

	case TARGMODE_OBJ_SET:		fSuccess = OnTarg_Obj_Set( uid ); break;
	case TARGMODE_OBJ_PROPS:	fSuccess = OnTarg_Obj_Props( uid, pt, id );  break;
	case TARGMODE_OBJ_FLIP:		fSuccess = OnTarg_Obj_Flip( uid ); break;

	case TARGMODE_CHAR_PRIVSET:	fSuccess = OnTarg_Char_PrivSet( uid, m_Targ_Text );  break;
	case TARGMODE_CHAR_BANK:	fSuccess = OnTarg_Char_Bank( uid ); break;
	case TARGMODE_CHAR_CONTROL:	fSuccess = OnTarg_Char_Control( uid ); break;

	case TARGMODE_ADDMULTIITEM:	fSuccess = OnTarg_Item_Multi_Add( pt ); break;
	case TARGMODE_ADDITEM:		fSuccess = OnTarg_Item_Add( pt ); break;
	case TARGMODE_LINK:			fSuccess = OnTarg_Item_Link( uid ); break;
	case TARGMODE_TILE:			fSuccess = OnTarg_Tile( pt );  break;

		// Player stuff.

	case TARGMODE_STONE_RECRUIT:	fSuccess = OnTarg_Stone_Recruit( uid );  break;
	case TARGMODE_SKILL:			fSuccess = OnTarg_Skill( uid ); break;
	case TARGMODE_SKILL_MAGERY:     fSuccess = OnTarg_Skill_Magery( uid, pt ); break;
	case TARGMODE_SKILL_HERD_DEST:  fSuccess = OnTarg_Skill_Herd_Dest( pt ); break;
	case TARGMODE_SKILL_POISON:		fSuccess = OnTarg_Skill_Poison( uid ); break;
	case TARGMODE_SKILL_PROVOKE:	fSuccess = OnTarg_Skill_Provoke( uid ); break;

	case TARGMODE_REPAIR:			fSuccess = m_pChar->Use_Repair( uid.ItemFind()); break;
	case TARGMODE_PET_CMD:			fSuccess = OnTarg_Pet_Command( uid, pt ); break;
	case TARGMODE_PET_STABLE:		fSuccess = OnTarg_Pet_Stable( uid.CharFind()); break;

	case TARGMODE_USE_ITEM:			fSuccess = OnTarg_Use_Item( uid, pt, id );  break;
	}
}

//----------------------------------------------------------------------

bool CClient::xDispatchMsg()
{
	// Process any messages we have Received from client.
	// RETURN: false = the message was not formatted correctly (dump the client)

	if ( ! xCheckSize( 1 ))	// just get the command
	{
		return( false );
	}

	m_Time_LastEvent = g_World.GetTime();	// We will always get pinged every couple minutes or so

	// check the packet size first.
	if ( m_bin.Default.m_Cmd >= XCMD_QTY ) // bad packet type ?
		return( false );

#if 0
	if ( Packet_Lengths_20000[m_bin.Default.m_Cmd] >= 0x8000 ) // var length
	{
		if ( ! xCheckSize(3))
			return(false);
		if ( ! xCheckSize(m_bin.Talk.m_len ))
			return(false);
	}
	else
	{
		// NOTE: What about client version differences !
		if ( ! xCheckSize( Packet_Lengths_20000[m_bin.Default.m_Cmd] ))
			return(false);
	}
#endif

	if ( m_bin.Default.m_Cmd == XCMD_Ping )
	{
		// Ping from client. Always ping back.
		if ( ! xCheckSize( sizeof( m_bin.Ping )))
			return(false);
		xSendReady( m_bin.m_Raw, sizeof( m_bin.Ping ));	// ping back.
		return( true );
	}

	if ( ! m_fGameServer || ! m_pAccount )
	{
		// login server or a game server that has not yet logged in.

		switch ( m_bin.Default.m_Cmd )
		{
		case XCMD_ServersReq: // First Login
			if ( ! xCheckSize( sizeof( m_bin.ServersReq ))) return(false);
			Login_ServerList( m_bin.ServersReq.m_name, m_bin.ServersReq.m_password );
			break;
		case XCMD_ServerSelect:// Server Select - relay me to the server.
			if ( ! xCheckSize( sizeof( m_bin.ServerSelect ))) return(false);
			return Login_Relay( m_bin.ServerSelect.m_select );
		case XCMD_CharListReq: // Second Login to select char
			if ( ! xCheckSize( sizeof( m_bin.CharListReq ))) return(false);
			Setup_ListReq( m_bin.CharListReq.m_account, m_bin.CharListReq.m_password );
			break;
		case XCMD_Spy: // Spy not sure what this does. tells us stuff about the clients world.
			if ( ! xCheckSize( sizeof( m_bin.Spy ))) return(false);
			// DEBUG_MSG(( "%x:Spy1\n", GetSocket()));
			break;
		case XCMD_War: // Tab = Combat Mode (toss this)
			if ( ! xCheckSize( sizeof( m_bin.War ))) return(false);
			break;
		default:
			return( false );
		}
		return( true );
	}

	/////////////////////////////////////////////////////
	// We should be encrypted below here.

	// Get messages from the client.
	switch ( m_bin.Default.m_Cmd )
	{
	case XCMD_Create: // Character Create
		if ( m_Crypt.GetClientVersion() >= 12600 )
		{
			if ( ! xCheckSize( sizeof( m_bin.Create ))) return(false);
		}
		else
		{
			if ( ! xCheckSize( sizeof( m_bin.Create_v25 ))) return(false);
		}
		Setup_CreateDialog();
		return( true );
	case XCMD_CharDelete: // Character Delete
		if ( ! xCheckSize( sizeof( m_bin.CharDelete ))) return(false);
		if ( ! Setup_Delete( m_bin.CharDelete.m_slot ))
		{
			addSysMessage( "Character Not Deleted" );
		}
		return( true );
	case XCMD_CharPlay: // Character Select
		if ( ! xCheckSize( sizeof( m_bin.CharPlay ))) return(false);
		if ( ! Setup_Play( m_bin.CharPlay.m_slot ))
		{
			addLoginErr( LOGIN_ERR_NONE );
		}
		return( true );
	case XCMD_TipReq: // Get Tip
		if ( ! xCheckSize( sizeof( m_bin.TipReq ))) return(false);
		Event_Tips( m_bin.TipReq.m_index + 1 );
		return( true );
	}

	// must have a logged in char to use any other messages.
	if ( m_pChar == NULL ) return( false );

	//////////////////////////////////////////////////////
	// We are now playing.

	switch ( m_bin.Default.m_Cmd )
	{
	case XCMD_Walk: // Walk
		if ( m_Crypt.GetClientVersion() >= 12600 )
		{
			if ( ! xCheckSize( sizeof( m_bin.Walk_v26 ))) return(false);
			Event_Walking( m_bin.Walk_v26.m_dir, m_bin.Walk_v26.m_count, m_bin.Walk_v26.m_cryptcode );
		}
		else
		{
			if ( ! xCheckSize( sizeof( m_bin.Walk_v25 ))) return(false);
			Event_Walking( m_bin.Walk_v25.m_dir, m_bin.Walk_v25.m_count, 0 );
		}
		break;
	case XCMD_Talk: // Speech or at least text was typed.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize(m_bin.Talk.m_len )) return(false);
		Event_Talk( m_bin.Talk.m_text, m_bin.Talk.m_color, (TALKMODE_TYPE)( m_bin.Talk.m_mode ));
		break;
	case XCMD_Attack: // Attack
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_Attack( (DWORD) m_bin.Click.m_UID );
		break;
	case XCMD_DClick:// Doubleclick
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_DoubleClick( (DWORD) m_bin.Click.m_UID, ((DWORD)(m_bin.Click.m_UID)) & UID_SPEC, true );
		break;
	case XCMD_ItemPickupReq: // Pick up Item
		if ( ! xCheckSize( sizeof( m_bin.ItemPickupReq ))) return(false);
		Event_Item_Pickup( (DWORD) m_bin.ItemPickupReq.m_UID, m_bin.ItemPickupReq.m_amount );
		break;
	case XCMD_ItemDropReq: // Drop Item
		if ( ! xCheckSize( sizeof( m_bin.ItemDropReq ))) return(false);
		Event_Item_Drop();
		break;
	case XCMD_Click: // Singleclick
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_SingleClick( (DWORD) m_bin.Click.m_UID );
		break;
	case XCMD_ExtCmd: // Ext. Command
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.ExtCmd.m_len )) return(false);
		Event_ExtCmd( (EXTCMD_TYPE) m_bin.ExtCmd.m_type, m_bin.ExtCmd.m_name );
		break;
	case XCMD_ItemEquipReq: // Equip Item
		if ( ! xCheckSize( sizeof( m_bin.ItemEquipReq ))) return(false);
		Event_Item_Equip();
		break;
	case XCMD_WalkAck: // Resync Request
		if ( ! xCheckSize( sizeof( m_bin.WalkAck ))) return(false);
		addReSync();
		break;
	case XCMD_DeathMenu:	// DeathOpt (un)Manifest ghost
		if ( ! xCheckSize(2)) return(false);
		if ( ! Event_DeathOption( m_bin.DeathMenu.m_mode )) return( false );
		break;
	case XCMD_CharStatReq: // Status Request
		if ( ! xCheckSize( sizeof( m_bin.CharStatReq ))) return(false);
		if ( m_bin.CharStatReq.m_type == 4 ) addCharStatWindow( (DWORD) m_bin.CharStatReq.m_UID );
		if ( m_bin.CharStatReq.m_type == 5 ) addSkillWindow( SKILL_QTY );
		break;
	case XCMD_Skill:	// Skill locking.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.Skill.m_len )) return(false);
		Event_Skill_Locks();
		break;
	case XCMD_VendorBuy:	// Buy item from vendor.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.VendorBuy.m_len )) return(false);
		Event_VendorBuy( (DWORD) m_bin.VendorBuy.m_UIDVendor );
		break;
	case XCMD_MapEdit:	// plot course on map.
		if ( ! xCheckSize( sizeof( m_bin.MapEdit ))) return(false);
		Event_MapEdit( (DWORD) m_bin.MapEdit.m_UID );
		break;
	case XCMD_BookPage: // Read/Change Book
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.BookPage.m_len )) return(false);
		Event_Book_Page( (DWORD) m_bin.BookPage.m_UID );
		break;
	case XCMD_Options: // Options set
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.Options.m_len )) return(false);
		DEBUG_MSG(( "%x:XCMD_Options len=%d\n", GetSocket(), m_bin.Options.m_len ));
		break;
	case XCMD_Target: // Targeting
		if ( ! xCheckSize( sizeof( m_bin.Target ))) return(false);
		Event_Target();
		break;
	case XCMD_SecureTrade: // Secure trading
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.SecureTrade.m_len )) return(false);
		Event_SecureTrade( (DWORD) m_bin.SecureTrade.m_UID );
		break;
	case XCMD_BBoard: // BBoard Request.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.BBoard.m_len )) return(false);
		Event_BBoardRequest( (DWORD) m_bin.BBoard.m_UID );
		break;
	case XCMD_War: // Combat Mode
		if ( ! xCheckSize( sizeof( m_bin.War ))) return(false);
		Event_CombatMode( m_bin.War.m_warmode );
		break;
	case XCMD_CharName: // Rename Character(pet)
		if ( ! xCheckSize( sizeof( m_bin.CharName ))) return(false);
		Event_SetName( (DWORD) m_bin.CharName.m_UID );
		break;
	case XCMD_MenuChoice: // Menu Choice
		if ( ! xCheckSize( sizeof( m_bin.MenuChoice ))) return(false);
		Event_MenuChoice();
		break;
	case XCMD_BookOpen:	// Change a books title/author.
		if ( m_Crypt.GetClientVersion() >= 12600 )
		{
			if ( ! xCheckSize( sizeof( m_bin.BookOpen_v26 ))) return(false);
			Event_Book_Title( (DWORD) m_bin.BookOpen_v26.m_UID, m_bin.BookOpen_v26.m_title, m_bin.BookOpen_v26.m_author );
		}
		else
		{
			if ( ! xCheckSize( sizeof( m_bin.BookOpen_v25 ))) return(false);
			Event_Book_Title( (DWORD) m_bin.BookOpen_v25.m_UID, m_bin.BookOpen_v25.m_title, m_bin.BookOpen_v25.m_author );
		}
		break;
	case XCMD_DyeVat: // Color Select Dialog
		if ( ! xCheckSize( sizeof( m_bin.DyeVat ))) return(false);
		Event_Item_Dye( (DWORD) m_bin.DyeVat.m_UID );
		break;
	case XCMD_Prompt: // Response to console prompt.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.Prompt.m_len )) return(false);
		Event_PromptResp( m_bin.Prompt.m_text, m_bin.Prompt.m_len-sizeof(m_bin.Prompt));
		break;
	case XCMD_HelpPage: // GM Page
		if ( ! xCheckSize( sizeof( m_bin.HelpPage ))) return(false);
		Cmd_Script_Menu( TARGMODE_MENU_GM_PAGE );
		break;
	case XCMD_VendorSell: // Vendor Sell
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.VendorSell.m_len )) return(false);
		Event_VendorSell( (DWORD) m_bin.VendorSell.m_UIDVendor );
		break;
	case XCMD_Scroll:	// Scroll Closed
		if ( ! xCheckSize( sizeof( m_bin.Scroll ))) return(false);
		DEBUG_MSG(( "%x:XCMD_Scroll(close) 0%x\n", GetSocket(), m_bin.Scroll.m_scrollID ));
		break;
	case XCMD_GumpText:	// Gump text input
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.GumpText.m_len )) return(false);
		Event_GumpTextIn();
		break;
	case XCMD_TalkUNICODE:	// Talk unicode.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize(m_bin.TalkUNICODE.m_len )) return(false);
		SetPrivFlags( PRIV_T2A );
		Event_TalkUNICODE();
		break;
	case XCMD_GumpButton:	// Gump menu.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.GumpButton.m_len )) return(false);
		Event_GumpButton();
		break;

	case XCMD_ChatText:	// ChatText
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.ChatText.m_len )) return(false);
		SetPrivFlags( PRIV_T2A );
		Event_ChatText( m_bin.ChatText.m_lang, m_bin.ChatText.m_utext, m_bin.ChatText.m_len );
		break;
	case XCMD_Chat: // Chat
		if ( ! xCheckSize( sizeof( m_bin.Chat))) return(false);
		SetPrivFlags( PRIV_T2A );
		Event_ChatButton(m_bin.Chat.m_uname);
		break;
	case XCMD_ToolTipReq:	// Tool Tip
		if ( ! xCheckSize( sizeof( m_bin.ToolTipReq ))) return(false);
		SetPrivFlags( PRIV_T2A );
		Event_ToolTip( (DWORD) m_bin.ToolTipReq.m_UID );
		break;
	case XCMD_CharProfile:	// Get Character Profile.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.CharProfile.m_len )) return(false);
		SetPrivFlags( PRIV_T2A );
		DEBUG_MSG(( "%x:XCMD_CharProfile\n", GetSocket()));
		SysMessage( "Sorry No profile info is available yet" );
		// ??? we should do something with this !
		break;
	case XCMD_MailMsg:	// Some new OSI packet
		if ( ! xCheckSize( sizeof(m_bin.MailMsg))) return(false);
		Event_MailMsg( (DWORD) m_bin.MailMsg.m_uid1, (DWORD) m_bin.MailMsg.m_uid2 );
		break;
	case XCMD_ClientVersion:	// Client Version string packet
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize(m_bin.ClientVersion.m_len)) return(false);
		SetPrivFlags( PRIV_T2A );
		// DEBUG_MSG(( "%x:XCMD_ClientVersion\n", GetSocket()));
		break;
	case XCMD_ExtData:	// Add someone to the party system.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.ExtData.m_len )) return(false);
		Event_ExtData( (EXTDATA_TYPE)(WORD) m_bin.ExtData.m_type, m_bin.ExtData.m_len-5, m_bin.ExtData.m_data );
		break;

	default:
		// clear socket I have no idea what this is.
		return( false );
	}

	// This is the last message we are pretty sure we got correctly.
	m_bin_PrvMsg = (XCMD_TYPE) m_bin.Default.m_Cmd;
	return( true );
}



