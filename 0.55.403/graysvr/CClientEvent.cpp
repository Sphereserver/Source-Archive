//
// CClientEvent.cpp
// Copyright Menace Software (www.menasoft.com).
//
#include "graysvr.h"	// predef header.
#include "CClient.h"
#include <wchar.h>

/////////////////////////////////
// Events from the Client.

void CClient::Event_ChatButton(const NCHAR * pszName) // Client's chat button was pressed
{
	// See if they've made a chatname yet
	// m_ChatPersona.SetClient(this);

	if (m_pChar->OnTrigger(CTRIG_UserChatButton, m_pChar) == TRIGRET_RET_TRUE)
		return;

	ASSERT(GetAccount());

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
		TCHAR szChatName[ MAX_NAME_SIZE * 2 + 2 ];
		CvtNUNICODEToSystem( szChatName, sizeof(szChatName), pszName, 128 );

		if ( ! CChat::IsValidName(szChatName, true) ||
			g_Accounts.Account_FindChat(szChatName)) // Check for legal name, duplicates, etc
		{
			addChatSystemMessage(CHATMSG_Error);
			addChatSystemMessage(CHATMSG_GetChatName);
			return;
		}
		GetAccount()->m_sChatName = szChatName;
	}

	// Ok, below here we have a chat system nickname
	// Tell the chat system it has a new client using it
	SetChatActive();
}

void CClient::Event_ChatText( const NCHAR * pszText, int len, CLanguageID lang ) // Text from a client
{
	// Just send it all to the chat system
	g_Serv.m_Chats.EventMsg( this, pszText, len, lang );
}

void CClient::Event_MapEdit( CGrayUID uid, const CEvent * pEvent )
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

	switch (pEvent->MapEdit.m_action)
	{
	case MAP_ADD: // add pin
		{
			if ( pMap->m_Pins.GetCount() > CItemMap::MAX_PINS )
				return;	// too many.
			CMapPinRec pin( pEvent->MapEdit.m_pin_x, pEvent->MapEdit.m_pin_y );
			pMap->m_Pins.Add( pin );
			break;
		}
	case MAP_INSERT: // insert between 2 pins
		{
		if ( pMap->m_Pins.GetCount() > CItemMap::MAX_PINS )
			return;	// too many.
		CMapPinRec pin( pEvent->MapEdit.m_pin_x, pEvent->MapEdit.m_pin_y );
		pMap->m_Pins.InsertAt( pEvent->MapEdit.m_pin, pin);
		break;
		}
	case MAP_MOVE: // move pin
		if ( pEvent->MapEdit.m_pin >= pMap->m_Pins.GetCount())
		{
			SysMessage( "That's strange... (bad pin)" );
			return;
		}
		pMap->m_Pins[pEvent->MapEdit.m_pin].m_x = pEvent->MapEdit.m_pin_x;
		pMap->m_Pins[pEvent->MapEdit.m_pin].m_y = pEvent->MapEdit.m_pin_y;
		break;
	case MAP_DELETE: // delete pin
		{
			if ( pEvent->MapEdit.m_pin >= pMap->m_Pins.GetCount())
			{
				SysMessage( "That's strange... (bad pin)" );
				return;
			}
			pMap->m_Pins.RemoveAt(pEvent->MapEdit.m_pin);
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

void CClient::Event_Item_Dye( CGrayUID uid, HUE_TYPE wHue ) // Rehue an item
{
	// CLIMODE_DYE
	// Result from addDyeOption()
	CObjBase * pObj = uid.ObjFind();
	if ( ! m_pChar->CanTouch( pObj ))	// sanity check.
	{
		SysMessage( "You can't reach it" );
		return;
	}
	if ( GetTargMode() != CLIMODE_DYE )
	{
		return;
	}

	ClearTargMode();

	if ( ! IsPriv( PRIV_GM ))
	{
		if ( pObj->GetBaseID() != 0xFAB )
		{
			g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is attempting to dye something besides a dye tub" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
			return;
		}

		if ( wHue<HUE_BLUE_LOW || wHue>HUE_DYE_HIGH )
			wHue = HUE_DYE_HIGH;
	}
	else
	{
		if ( pObj->IsChar())
		{
			pObj->RemoveFromView();
			wHue |= HUE_UNDERWEAR;
		}
	}

	pObj->SetHue( wHue );
	pObj->Update();
}



void CClient::Event_Tips( WORD i) // Tip of the day window
{
	if (i==0)
		i=1;
	CResourceLock s;
	if ( g_Cfg.ResourceLock( s, RESOURCE_ID( RES_TIP, i )))
	{
		addScrollScript( s, SCROLL_TYPE_TIPS, i );
	}
}



void CClient::Event_Book_Title( CGrayUID uid, LPCTSTR pszTitle, LPCTSTR pszAuthor )
{
	// XCMD_BookOpen
	// user is changing the books title/author info.

	CItemMessage * pBook = dynamic_cast <CItemMessage *> (uid.ItemFind());
	if ( ! m_pChar->CanTouch( pBook ))	// sanity check.
	{
		SysMessage( "you can't reach it" );
		return;
	}
	if ( ! pBook->IsBookWritable())	// Not blank
		return;

	if ( Str_Check( pszTitle ) )
	{
		g_Log.Event( LOGL_WARN | LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked book title" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName() );
		return;
	}

	if ( Str_Check( pszAuthor ) )
	{
		g_Log.Event( LOGL_WARN | LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked book author" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName() );
		return;
	}

	pBook->SetName( pszTitle );
	pBook->m_sAuthor = pszAuthor;
}

void CClient::Event_Book_Page( CGrayUID uid, const CEvent * pEvent ) // Book window
{
	// XCMD_BookPage
	// Read or write to a book page.

	CItem * pBook = uid.ItemFind();	// the book.
	if ( ! m_pChar->CanSee( pBook ))
	{
		addObjectRemoveCantSee( uid, "the book" );
		return;
	}

	int iPage = pEvent->BookPage.m_page[0].m_pagenum;	// page.
	DEBUG_CHECK( iPage > 0 );

	if ( pEvent->BookPage.m_page[0].m_lines == 0xFFFF ||
		pEvent->BookPage.m_len <= 0x0d )
	{
		// just a request for pages.
		addBookPage( pBook, iPage );
		return;
	}

	// Trying to write to the book.
	CItemMessage * pText = dynamic_cast <CItemMessage *> (pBook);
	if ( pText == NULL || ! pBook->IsBookWritable()) // not blank ?
		return;

	int iLines = pEvent->BookPage.m_page[0].m_lines;
	DEBUG_CHECK( iLines <= 8 );
	DEBUG_CHECK( pEvent->BookPage.m_pages == 1 );

	if ( ! iLines || iPage <= 0 || iPage > MAX_BOOK_PAGES )
		return;
	if ( iLines > MAX_BOOK_LINES ) iLines = MAX_BOOK_LINES;
	iPage --;

	int len = 0;
	TCHAR *pszTemp = Str_GetTemp();
	for ( int i=0; i<iLines; i++ )
	{
		len += strcpylen( pszTemp+len, pEvent->BookPage.m_page[0].m_text+len );
		pszTemp[len++] = '\t';
	}

	pszTemp[--len] = '\0';
    if ( Str_Check( pszTemp ) )
    {
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked book page" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}
	pText->SetPageText( iPage, pszTemp );
}



void CClient::Event_Item_Pickup( CGrayUID uid, int amount ) // Client grabs an item
{
	// Player/client is picking up an item.

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_Item_Pickup %d" DEBUG_CR, m_Socket.GetSocket(), uid ));
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
	SetTargMode( CLIMODE_DRAG );
	m_Targ_UID = uid;
}



void CClient::Event_Item_Drop( const CEvent * pEvent ) // Item is dropped
{
	// This started from the Event_Item_Pickup()

	ASSERT( m_pChar );

	CGrayUID uidItem( pEvent->ItemDropReq.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CGrayUID uidOn( pEvent->ItemDropReq.m_UIDCont );	// dropped on this item.
	CObjBase * pObjOn = uidOn.ObjFind();
	CPointMap  pt( pEvent->ItemDropReq.m_x, pEvent->ItemDropReq.m_y, pEvent->ItemDropReq.m_z, m_pChar->GetTopMap() );

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_Item_Drop %lx on %lx, x=%d, y=%d" DEBUG_CR,
			m_Socket.GetSocket(), uidItem, uidOn, pt.m_x, pt.m_y ));
	}

	// Are we out of sync ?
	if ( pItem == NULL ||
		pItem == pObjOn ||	// silliness.
		GetTargMode() != CLIMODE_DRAG ||
		pItem != m_pChar->LayerFind( LAYER_DRAGGING ))
	{
		addItemDragCancel(5);
		return;
	}

	ClearTargMode();	// done dragging

	addPause();

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
			/*
			CScriptTriggerArgs Args( pObjOn );
			if ( pItem->OnTrigger( ITRIG_DROPON_CHAR, m_pChar, &Args ) == TRIGRET_RET_TRUE )
				goto cantdrop;
			*/
			CChar * pChar = dynamic_cast <CChar*>( pObjTop );
			ASSERT(pChar);
			if ( ! pChar->NPC_IsOwnedBy( m_pChar ))
			{
				// Slyly dropping item in someone elses pack.
				// or just dropping on their trade window.
				if ( ! Cmd_SecureTrade( pChar, pItem ))
					goto cantdrop;
				return;
			}
			if ( ! pChar->m_pPlayer )
			{
				// newbie items lose newbie status when transfered to NPC
				pItem->ClrAttr(ATTR_NEWBIE|ATTR_OWNED);
			}
			if ( pChar->GetBank()->IsItemInside( pContItem ))
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

		CScriptTriggerArgs Args( pObjOn );
		if ( pItem->OnTrigger( ITRIG_DROPON_ITEM, m_pChar, &Args ) == TRIGRET_RET_TRUE )
			goto cantdrop;

		CItem * pItemOn = dynamic_cast <CItem*> ( pObjOn );
		if ( pItemOn )
		{
			CScriptTriggerArgs Args( pItem );
			if ( pItemOn->OnTrigger( ITRIG_DROPON_SELF, m_pChar, &Args ) == TRIGRET_RET_TRUE )
				goto cantdrop;
		}

		if ( pContItem != NULL )
		{
			if (	pContItem->IsType( IT_EQ_BANK_BOX )
				&&	pContItem->m_itEqBankBox.m_pntOpen != m_pChar->GetTopPoint() )
			{
				g_Log.Event( LOGL_WARN|LOGM_CHEAT,
					"%x:Cheater '%s' is dropping item in bank box using 3rd party tools to access it\n",
						m_Socket.GetSocket(), (LPCTSTR) GetAccount()->GetName() );
					goto cantdrop;
			}

			// Putting it into some sort of container.
			if ( !pContItem->CanContainerHold( pItem, m_pChar ) )
				goto cantdrop;
		}
		else
		{
			// dropped on top of a non container item.
			// can i pile them ?
			// Still in same container.

			ASSERT(pItemOn);
			pObjOn = pItemOn->GetContainer();
			pt = pItemOn->GetUnkPoint();

			if ( ! pItem->Stack( pItemOn ))
			{
				if ( pItemOn->IsType(IT_SPELLBOOK))
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
	}
	else
	{
		if ( ! m_pChar->CanTouch( pt ))	// Must also be LOS !
		{
	cantdrop:
			// The item was in the LAYER_DRAGGING.
			// Try to bounce it back to where it came from.
			if ( pItem == m_pChar->LayerFind( LAYER_DRAGGING ) )	// if still being dragged
				m_pChar->ItemBounce( pItem );
			return;
		}
	}

	// Game pieces can only be droped on their game boards.
	if ( pItem->IsType(IT_GAME_PIECE))
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

	// do the dragging anim for everyone else to see.

	if ( pObjOn != NULL )
	{
		// in pack or other CItemContainer.
		m_pChar->UpdateDrag( pItem, pObjOn );
		CItemContainer * pContOn = dynamic_cast <CItemContainer *>(pObjOn);
		ASSERT(pContOn);
		pContOn->ContentAdd( pItem, pt );
		addSound( pItem->GetDropSound( pObjOn ));
	}
	else
	{
		// on ground
		m_pChar->UpdateDrag( pItem, NULL, &pt );
		m_pChar->ItemDrop( pItem, pt );
	}
}



void CClient::Event_Item_Equip( const CEvent * pEvent ) // Item is dropped on paperdoll
{
	// This started from the Event_Item_Pickup()

	CGrayUID uidItem( pEvent->ItemEquipReq.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CGrayUID uidChar( pEvent->ItemEquipReq.m_UIDChar );
	CChar * pChar = uidChar.CharFind();
	LAYER_TYPE layer = (LAYER_TYPE)( pEvent->ItemEquipReq.m_layer );

	if ( pItem == NULL ||
		GetTargMode() != CLIMODE_DRAG ||
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
		m_pChar->ItemBounce( pItem );	// put in pack or drop at our feet.
		return;
	}

	if ( ! pChar->NPC_IsOwnedBy( m_pChar ))
	{
		// trying to equip another char ?
		// can if he works for you.
		// else just give it to him ?
		goto cantequip;
	}

	if ( ! pChar->ItemEquip( pItem, m_pChar ))
	{
		goto cantequip;
	}

#ifdef _DEBUG
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Item %s equipped on layer %i." DEBUG_CR, m_Socket.GetSocket(), pItem->GetResourceName(), layer ));
	}
#endif
}



void CClient::Event_Skill_Locks( const CEvent * pEvent )
{
	// Skill lock buttons in the skills window.
	ASSERT( GetChar());
	ASSERT( pEvent->Skill.m_Cmd == XCMD_Skill );		// 0= 0x3A
	ASSERT( GetChar()->m_pPlayer );
	DEBUG_CHECK( IsNoCryptVer(1) || m_Crypt.GetClientVer() >= 0x126020 );

	int len = pEvent->Skill.m_len;
	len -= 3;
	for ( int i=0; len; i++ )
	{
		SKILL_TYPE index = (SKILL_TYPE)(WORD) pEvent->Skill.skills[i].m_index;
		SKILLLOCK_TYPE state = (SKILLLOCK_TYPE) pEvent->Skill.skills[i].m_lock;

		GetChar()->m_pPlayer->Skill_SetLock( index, state );

		len -= sizeof( pEvent->Skill.skills[0] );
	}
}



void CClient::Event_Skill_Use( SKILL_TYPE skill ) // Skill is clicked on the skill list
{
	// All the push button skills come through here.
	// Any "Last skill" macro comes here as well. (push button only)

	bool fContinue = false;

	if ( m_pChar->Skill_Wait(skill) )
		return;

	if ( m_pChar->Skill_OnTrigger( skill, SKTRIG_SELECT ) == TRIGRET_RET_TRUE )
	{
		m_pChar->Skill_Fail( true );	// clean up current skill.
		return;
	}

	SetTargMode();
	m_Targ_UID.InitUID();	// This is a start point for targ more.

	bool fCheckCrime	= false;

	if ( g_Cfg.IsSkillFlag( skill, SKF_SCRIPTED ) )
	{
		if ( !g_Cfg.GetSkillDef(skill)->m_sTargetPrompt.IsEmpty() )
		{
			m_tmSkillTarg.m_Skill = skill;	// targetting what skill ?
			addTarget( CLIMODE_TARG_SKILL, g_Cfg.GetSkillDef(skill)->m_sTargetPrompt, false, fCheckCrime );
			return;
		}
		else
			m_pChar->Skill_Start( skill );
	}
	else switch ( skill )
	{
	case SKILL_ARMSLORE:
	case SKILL_ITEMID:
	case SKILL_ANATOMY:
	case SKILL_ANIMALLORE:
	case SKILL_EVALINT:
	case SKILL_FORENSICS:
	case SKILL_TASTEID:

	case SKILL_BEGGING:
	case SKILL_TAMING:
	case SKILL_REMOVETRAP:
		fCheckCrime = false;

dotargetting:
		// Go into targtting mode.
		if ( g_Cfg.GetSkillDef(skill)->m_sTargetPrompt.IsEmpty() )
		{
			DEBUG_ERR(( "%x: Event_Skill_Use bad skill %d" DEBUG_CR, m_Socket.GetSocket(), skill ));
			return;
		}

		m_tmSkillTarg.m_Skill = skill;	// targetting what skill ?
		addTarget( CLIMODE_TARG_SKILL, g_Cfg.GetSkillDef(skill)->m_sTargetPrompt, false, fCheckCrime );
		return;

	case SKILL_STEALING:
	case SKILL_ENTICEMENT:
	case SKILL_PROVOCATION:
	case SKILL_POISONING:
		// Go into targtting mode.
		fCheckCrime = true;
		goto dotargetting;

	case SKILL_STEALTH:	// How is this supposed to work.
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
		Cmd_Skill_Inscription();
		break;

	default:
		SysMessage( "There is no such skill. Please tell support you saw this message.");
		break;
	}
}



bool CClient::Event_WalkingCheck(DWORD dwEcho)
{
	// look for the walk code, and remove it
	// Client will send 0's if we have not given it any EXTDATA_WalkCode_Prime message.
	// The client will use codes in order.
	// But it will skip codes sometimes. so delete codes that get skipped.

	// RETURN:
	//  true = ok to walk.
	//  false = the client is cheating. I did not send that code.

	if ( ! IsNoCryptVer(1) )
		if ( m_Crypt.GetClientVer() < 0x126000 )
			return( true );

	if ( ! ( g_Cfg.m_wDebugFlags & DEBUGF_WALKCODES ))
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
		if ( m_Walk_LIFO[i] == dwEcho )	// found a good code.
		{
			// Move next to the head.
			i++;
			memmove( m_Walk_LIFO, m_Walk_LIFO+i, (m_Walk_CodeQty-i)*sizeof(DWORD));
			m_Walk_CodeQty -= i;
			// Set this to negative so we know later if we've gotten at least 1 valid echo
			m_Walk_InvalidEchos = -1;
			return( true );
		}
	}

	if ( m_Walk_InvalidEchos < 0 )
	{
		// If we're here, we got at least one valid echo....therefore
		// we should never get another one
		DEBUG_ERR(( "%x: Invalid walk echo (0%x). Invalid after valid." DEBUG_CR, m_Socket.GetSocket(), dwEcho ));
		addPlayerWalkCancel();
		return false;
	}

	// Increment # of invalids received. This is allowed at startup.
	// These "should" always be 0's
	if ( ++ m_Walk_InvalidEchos >= COUNTOF(m_Walk_LIFO))
	{
		// The most I ever got was 2 of these, but I've seen 4
		// a couple of times on a real server...we might have to
		// increase this # if it becomes a problem (but I doubt that)
		DEBUG_ERR(( "%x: Invalid walk echo. Too many initial invalid." DEBUG_CR, m_Socket.GetSocket()));
		addPlayerWalkCancel();
		return false;
	}

	// Allow them to walk a bit till we catch up.
	return true;
}



void CClient::Event_Walking( BYTE rawdir, BYTE count, DWORD dwEcho ) // Player moves
{
	// XCMD_Walk
	// The theory....
	// The client sometimes echos 1 or 2 zeros or invalid echos when you first start
	//	walking (the invalid non zeros happen when you log off and don't exit the
	//	client.exe all the way and then log back in, XXX doesn't clear the stack)

	ASSERT(m_pChar);
	if ( ! Event_WalkingCheck( dwEcho ))
		return;

	if ( m_pChar->IsStatFlag( STATF_Freeze | STATF_Stone ) &&
		m_pChar->OnFreezeCheck())
	{
		addPlayerWalkCancel();
		return;
	}

	if ( count != (BYTE)( m_wWalkCount+1 ))	// && count != 255
	{
		// DEBUG_MSG(( "%x: New Walk Count %d!=%d" DEBUG_CR, m_Socket.GetSocket(), count, m_wWalkCount ));
		if ( (WORD)(m_wWalkCount) == 0xFFFF )
			return;	// just playing catch up with last reset. don't cancel again.
		addPlayerWalkCancel();
		return;
	}

	bool fRun = ( rawdir & 0x80 ); // or flying ?

	m_pChar->StatFlag_Mod( STATF_Fly, fRun );

	DIR_TYPE dir = (DIR_TYPE)( rawdir & 0x0F );
	if ( dir >= DIR_QTY )
	{
		addPlayerWalkCancel();
		return;
	}
	CPointMap pt = m_pChar->GetTopPoint();
	CPointMap ptold = pt;
	bool	fMove = true;
	bool	fUpdate	= false;

	if ( dir == m_pChar->m_dirFace )
	{
		LONGLONG	CurrTime	= GetTickCount();
		m_iWalkStepCount++;
		// Move in this dir.
		if ( ( m_iWalkStepCount % 7 ) == 0 )	// we have taken 8 steps ? direction changes don't count.
		{
			if ( !g_Cfg.m_iWalkBuffer )
			{
			}
			// Client only allows 4 steps of walk ahead.
			else // if ( count )
			{
				int		iTimeDiff	= ((CurrTime - m_timeWalkStep)/10);
				int		iTimeMin	= m_pChar->IsStatFlag( STATF_OnHorse ) ? 70 : 140;

				if ( iTimeDiff > iTimeMin )
				{
					int	iRegen	= ((iTimeDiff - iTimeMin) * g_Cfg.m_iWalkRegen) / 150;
					if ( iRegen > g_Cfg.m_iWalkBuffer )
						iRegen	= g_Cfg.m_iWalkBuffer;
					else if ( iRegen < -((g_Cfg.m_iWalkBuffer * g_Cfg.m_iWalkRegen) / 100) )
						iRegen	= -((g_Cfg.m_iWalkBuffer * g_Cfg.m_iWalkRegen) / 100);
					iTimeDiff	= iTimeMin + iRegen;
				}

				m_iWalkTimeAvg		+= iTimeDiff;

				int	oldAvg	= m_iWalkTimeAvg;
				m_iWalkTimeAvg	-= iTimeMin;

				if ( m_iWalkTimeAvg > g_Cfg.m_iWalkBuffer )
					m_iWalkTimeAvg	= g_Cfg.m_iWalkBuffer;
				else if ( m_iWalkTimeAvg < -g_Cfg.m_iWalkBuffer )
					m_iWalkTimeAvg	= -g_Cfg.m_iWalkBuffer;

				if ( g_Log.IsLogged( LOGL_TRACE ) && IsPriv( PRIV_GM ) )
				{
					// SysMessagef( "Walk trace: %i / %i (%i) :: %i",
					//	iTimeDiff, iTimeMin, oldAvg, m_iWalkTimeAvg );
				}

				if ( m_iWalkTimeAvg < 0 && iTimeDiff >= 0 && ! IsPriv(PRIV_GM) )	// TICK_PER_SEC
				{
					// walking too fast.
					DEBUG_WARN(( "%s (%x): Fast Walk ?" DEBUG_CR,
						GetName(), m_Socket.GetSocket() ));

					TRIGRET_TYPE iAction = m_pChar->OnTrigger( CTRIG_UserExWalkLimit, m_pChar, NULL );

					m_iWalkStepCount--; // eval again next time !

					if ( iAction != TRIGRET_RET_TRUE )
					{
						addPlayerWalkCancel();
						return;
					}
				}
			}	//
			m_timeWalkStep = CurrTime;
		}	// nth step


		pt.Move( dir );

		// Check the z height here.
		// The client already knows this but doesn't tell us.
		if ( ! m_pChar->CanMoveWalkTo( pt, true, false, dir ) )
		{
			addPlayerWalkCancel();
			return;
		}

		// Are we invis ?
		m_pChar->CheckRevealOnMove();
		m_pChar->MoveToChar( pt );

		// Check if we have gone indoors.
		bool fRoof = m_pChar->IsStatFlag( STATF_InDoors );

		// Should i update the weather ?
		if ( fRoof != m_pChar->IsStatFlag( STATF_InDoors ))
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

	if ( dir != m_pChar->m_dirFace )
	{
		// Just a change in dir.
		m_pChar->m_dirFace = dir;
		fMove = false;
	}

	// Ack the move. ( if this does not go back we get rubber banding )
	m_wWalkCount = count;	// m_wWalkCount++
	CCommand cmd;
	cmd.WalkAck.m_Cmd = XCMD_WalkAck;
	cmd.WalkAck.m_count = (BYTE) m_wWalkCount;
	// Not really sure what this does.
	cmd.WalkAck.m_flag = ( m_pChar->IsStatFlag( STATF_Insubstantial | STATF_Invisible | STATF_Hidden | STATF_Sleeping )) ?
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
	// What new stuff do I now see ?
	addPlayerSee( ptold );
}



void CClient::Event_CombatMode( bool fWar ) // Only for switching to combat mode
{
	// If peacmaking then this doens't work ??
	// Say "you are feeling too peacefull"

	CScriptTriggerArgs Args;
	Args.m_iN1 = m_pChar->IsStatFlag(STATF_War) ? 1 : 0;
	if (m_pChar->OnTrigger(CTRIG_UserWarmode, m_pChar, &Args) == TRIGRET_RET_TRUE)
		return;

	m_pChar->StatFlag_Mod( STATF_War, fWar );

	if ( m_pChar->IsStatFlag( STATF_DEAD ))
	{
		// Manifest the ghost.
		// War mode for ghosts.
		m_pChar->StatFlag_Mod( STATF_Insubstantial, ! fWar );
	}

	m_pChar->Skill_Fail( true );	// clean up current skill.
	if ( ! fWar )
	{
		m_pChar->Fight_ClearAll();
	}

	addPlayerWarMode();
	m_pChar->UpdateMode( this, m_pChar->IsStatFlag( STATF_DEAD ));
}



bool CClient::Event_Command( LPCTSTR pszCommand ) // Client entered a console command like /ADD
{
	if ( pszCommand[0] == '\0' )
		return false;
	if ( Str_Check( pszCommand ) )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked speech command" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName() );
		return false;
	}

	// Is this command avail for your priv level (or lower) ?
	bool fRet = false;
	if ( ! g_Cfg.CanUsePrivVerb( this, pszCommand, this ))
	{
		SysMessage( "You can't use this command." );
	}
	else
	{
		// Assume you don't mean yourself !
		static LPCTSTR const sm_szCmd_Redirect[] =		// default to redirect these.
		{
			"BANK",
			"CONTROL",
			"DUPE",
			"FORGIVE",
			"JAIL",
			"KICK",
			"KILL",
			"NUDGEDOWN",
			"NUDGEUP",
			"PARDON",
			"PRIVSET",
			"PROPS",
			"REMOVE",
			"SHRINK",
			"TWEAK",
		};
		if ( FindTableHeadSorted( pszCommand, sm_szCmd_Redirect, COUNTOF(sm_szCmd_Redirect)) >= 0 )
		{
			// targetted verbs are logged once the target is selected.
			addTargetVerb( pszCommand, "" );
		}
		else
		{
			CScript s( pszCommand );
			fRet = m_pChar->r_Verb( s, m_pChar );
			if ( ! fRet )
			{
				SysMessageDefault( DEFMSG_CMD_INVALID );
			}
		}
	}

	if ( GetPrivLevel() >= g_Cfg.m_iCommandLog )
		g_Log.Event( LOGM_GM_CMDS, "%x:'%s' commands '%s'=%d" DEBUG_CR, m_Socket.GetSocket(), (LPCTSTR) GetName(), (LPCTSTR) pszCommand, fRet );
	return( fRet );
}




void CClient::Event_Attack( CGrayUID uid )
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
	cmd.AttackOK.m_UID = (m_pChar->Fight_Attack( pChar )) ? (DWORD) pChar->GetUID() : 0;
	xSendPkt( &cmd, sizeof( cmd.AttackOK ));
}



void CClient::Event_VendorBuy( CGrayUID uidVendor, const CEvent * pEvent )
{
	// Client/Player buying items from the Vendor

	if ( pEvent->VendorBuy.m_flag == 0 )	// just a close command.
		return;

	CChar * pVendor = uidVendor.CharFind();
	if ( ! m_pChar->CanTouch( pVendor ))
	{
		SysMessage( "You can't reach the vendor" );
		return;
	}

	if ( ! pVendor->NPC_IsVendor())	// cheat
		return;

	CItemContainer * pContStock = pVendor->GetBank( LAYER_VENDOR_STOCK );
	CItemContainer * pVendExtra = pVendor->GetBank( LAYER_VENDOR_EXTRA );
	CItemContainer * pPack = m_pChar->GetPackSafe();

	if ( ( ! pContStock ) && ( ! pVendExtra ) ) // someone deleted vendor boxes?
		return;

	CItemBase *	pItemResearch	= pVendor->GetKeyItemBase( "RESEARCH.ITEM" );

	int iConvertFactor = pVendor->NPC_GetVendorMarkup( m_pChar );

	// Calculate the total cost of goods.
	int costtotal=0;
	bool fSoldout = false;
	int nItems = (pEvent->VendorBuy.m_len - 8) / sizeof( pEvent->VendorBuy.m_item[0] );
	int i=0;
	for ( ;i<nItems;i++)
	{
		CGrayUID uid( pEvent->VendorBuy.m_item[i].m_UID );
		CItemVendable * pItem = dynamic_cast <CItemVendable *> (uid.ItemFind());
		if ( pItem == NULL )
			continue;	// ignore it for now.

		// Do cheat/sanity check on goods sold.
		long iPrice = pItem->GetVendorPrice(iConvertFactor);
		if ( ! iPrice ||
			pItem->GetTopLevelObj() != pVendor ||
			pEvent->VendorBuy.m_item[i].m_amount > pItem->GetAmount())
		{
			// fSoldout = true;
			// continue;
			goto soldout; // stop looping!
		}
		if ( pContStock )
		{
			if( ! ( pContStock->IsItemInside( pItem ) ) // item is non in the stock
				&& ( ( ! pVendExtra ) // and vendor has no extras box
				|| ( ! pVendExtra->IsItemInside( pItem ) ) ) // or it doesn't contain the item
			  )
			{
				// we can buy items only from vendor boxes
				goto soldout;  // stop looping!
			}
		}
		else // both pVendExtra and pContStock can't be NULL - we've already checked it before?
		{
			if ( ! pVendExtra->IsItemInside( pItem ) )
			{
				goto soldout;  // stop looping!
			}
		}

		costtotal += pEvent->VendorBuy.m_item[i].m_amount * iPrice;
	}

	if ( fSoldout )
	{
soldout:
		pVendor->Speak( "Alas, I don't have all those goods in stock.  Let me know if there is something else thou wouldst buy." );
		// ??? Update the vendors list and try again.
		return;
	}

	bool fBoss = pVendor->NPC_IsOwnedBy( m_pChar );
	if ( ! fBoss )
	{
		if ( ( g_Cfg.m_fPayFromPackOnly ) ?
				m_pChar->GetPackSafe()->ContentConsume( RESOURCE_ID(RES_TYPEDEF,IT_GOLD), costtotal, true ) :
				m_pChar->ContentConsume( RESOURCE_ID(RES_TYPEDEF,IT_GOLD), costtotal, true )
			)
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


	// Move the items bought into your pack.
	for ( i=0;i<nItems;i++)
	{
		CGrayUID uid( pEvent->VendorBuy.m_item[i].m_UID );
		CItemVendable * pItem = dynamic_cast <CItemVendable *> (uid.ItemFind());
		if ( pItem == NULL )
			continue;	// ignore it i guess.
		if ( ! pItem->IsValidSaleItem( true ))
			continue;	// sorry can't buy this !

		if ( pVendor->IsStatFlag( STATF_Pet ) || ( pVendExtra && pVendExtra->IsItemInside( pItem ) ) )
		{
			// all items from extras box or from player vendors are simply transfered when
			// bought (not duped and deleted). No special actions for IT_FIGURINE here!
			WORD amount = pEvent->VendorBuy.m_item[i].m_amount;

			if 	( pItem->GetType() == IT_BEARD || pItem->GetType() == IT_HAIR )
			{
				if ( ( m_pChar->GetDispID() != CREID_MAN ) || ( ! m_pChar->IsHuman() ) )
				{
					pVendor->Speak( "Sorry not much i can do for you" );
					continue;
				}
				CItem * pItemNew = CItem::CreateDupeItem( pItem );
				m_pChar->LayerAdd(pItemNew);
				pItemNew->SetTimeout( 55000*TICK_PER_SEC );	// set the grow timer.
				pVendor->UpdateAnimate(ANIM_ATTACK_1H_WIDE);
				m_pChar->Sound( SOUND_SNIP );	// snip noise.
				if ( amount < pItem->GetAmount() )
					pItem->SetAmount( pItem->GetAmount() - amount );
				else
					pItem->Delete();
			}
			else if ( pItemResearch )
			{
				CItem * pItemNew = CItem::CreateScript( (ITEMID_TYPE) pItemResearch->GetResourceID().GetResIndex(), m_pChar );
				ASSERT(pItem);
				pItemNew->m_itResearchItem.m_ItemID		= RESOURCE_ID( RES_ITEMDEF, pItem->Item_GetDef()->GetResourceID().GetResIndex() );
				pItemNew->m_itResearchItem.m_iCompleted	= 0;
				pPack->ContentAdd(pItemNew);
				if ( amount < pItem->GetAmount() )
					pItem->SetAmount( pItem->GetAmount() - amount );
				else
					pItem->Delete();
			}
			else if ( amount < pItem->GetAmount() ) // dupe take place here only
			{
				pItem->SetAmount( pItem->GetAmount() - amount );
				if (! pItem->Item_GetDef()->IsStackableType())
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
			}
			else if ( amount > 1 && ! pItem->Item_GetDef()->IsStackableType())
			{
				pItem->SetAmount( pItem->GetAmount() - amount );
				while ( amount -- )
				{
					CItem * pItemNew = CItem::CreateDupeItem( pItem );
					pItemNew->SetAmount(1);
					pPack->ContentAdd(pItemNew);
				}
				pItem->Delete();
			}
			else
			{
				// just transfer item from extras box to backpack
				pItem->RemoveSelf();
				pPack->ContentAdd( pItem );
			}

		}
		else // item is in stock
		{
			WORD amount = pEvent->VendorBuy.m_item[i].m_amount;
			pItem->SetAmount( pItem->GetAmount() - amount );

			switch ( pItem->GetType() )
			{
			case IT_FIGURINE:
				{
				for ( int f = 0; f < amount; f++ )
				{
					// Buying animals is special.
					m_pChar->Use_Figurine( pItem, 2 );
				}
				}
				goto do_consume;
			case IT_BEARD:
				if ( m_pChar->GetDispID() != CREID_MAN )
				{
nohair:
					pVendor->Speak( "Sorry not much i can do for you" );
					continue;
				}
			case IT_HAIR:
				// Must be added directly. can't exist in pack!
				if ( ! m_pChar->IsHuman())
					goto nohair;
				{
				CItem * pItemNew = CItem::CreateDupeItem( pItem );
				m_pChar->LayerAdd(pItemNew);
				pItemNew->SetTimeout( 55000*TICK_PER_SEC );	// set the grow timer.
				pVendor->UpdateAnimate(ANIM_ATTACK_1H_WIDE);
				m_pChar->Sound( SOUND_SNIP );	// snip noise.
				}
				continue;
			}

			if ( pItemResearch )
			{
				CItem * pItemNew = CItem::CreateScript( (ITEMID_TYPE) pItemResearch->GetResourceID().GetResIndex(), m_pChar );
				ASSERT(pItem);
				pItemNew->m_itResearchItem.m_ItemID		= RESOURCE_ID( RES_ITEMDEF, pItem->Item_GetDef()->GetResourceID().GetResIndex() );
				pItemNew->m_itResearchItem.m_iCompleted	= 0;
				pPack->ContentAdd(pItemNew);
			}
			else if ( amount > 1 && ! pItem->Item_GetDef()->IsStackableType())
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

do_consume:
/*
			if ( pItem->GetAmount() == 0 &&
				( pVendor->IsStatFlag( STATF_Pet ) ||
				( pVendExtra && pVendExtra->IsItemInside(pItem))))
			{
				// we can buy it all.
				// not allowed to delete all from LAYER_VENDOR_STOCK
				pItem->Delete();
			}
			else
*/
			{
				pItem->Update();
			}
		}
	}

	//allocate few temp holds
	TCHAR *sMsg = Str_GetTemp();
	TCHAR *pszTemp1 = Str_GetTemp();
	TCHAR *pszTemp2 = Str_GetTemp();
	sprintf(pszTemp1, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_HYARE ), m_pChar->GetName());
	sprintf(pszTemp2, fBoss ? g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_S1 ) : g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_B1 ), costtotal, (costtotal==1) ? "" : g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_CA ));
	sprintf(sMsg, "%s %s %s", pszTemp1, pszTemp2, g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_TY ));
	pVendor->Speak( sMsg );

	// Take the gold.
	if ( ! fBoss )
	{
		int rc = ( g_Cfg.m_fPayFromPackOnly ) ?
			m_pChar->GetPackSafe()->ContentConsume( RESOURCE_ID(RES_TYPEDEF,IT_GOLD), costtotal) :
			m_pChar->ContentConsume( RESOURCE_ID(RES_TYPEDEF,IT_GOLD), costtotal);

		// Add the gold to the vendors total to play with.
		pVendor->GetBank()->m_itEqBankBox.m_Check_Amount += costtotal;
	}

	// Clear the vendor display.
	addVendorClose(pVendor);
	if ( i )
	{
		addSound( 0x057 );	// add to inv sound.
	}
}



void CClient::Event_VendorSell( CGrayUID uidVendor, const CEvent * pEvent )
{
	// Player Selling items to the vendor.
	// Done with the selling action.

	CChar * pVendor = uidVendor.CharFind();
	if ( ! m_pChar->CanTouch( pVendor ))
	{
		SysMessageDefault( DEFMSG_TOOFAR_VENDOR );
		return;
	}
	if ( ! pEvent->VendorSell.m_count )
	{
		addVendorClose( pVendor );
		// pVendor->Speak( "You have sold nothing" );
		return;
	}

	if ( ! pVendor->NPC_IsVendor())	// cheat
		return;

	CItemContainer * pBank = pVendor->GetBank();
	CItemContainer * pContStock = pVendor->GetBank( LAYER_VENDOR_STOCK );
	CItemContainer * pContBuy = pVendor->GetBank( LAYER_VENDOR_BUYS );
	CItemContainer * pContExtra = pVendor->GetBank( LAYER_VENDOR_EXTRA );
	if ( pBank == NULL || pContStock == NULL || pContExtra == NULL )
	{
		addVendorClose( pVendor );
		pVendor->Speak( g_Cfg.GetDefaultMsg( DEFMSG_NCP_VENDOR_GUARDS ) );
		return;
	}

	int iConvertFactor = -pVendor->NPC_GetVendorMarkup( m_pChar );

	// MAX_ITEMS_CONT ???
	int iGold = 0;
	bool fShortfall = false;

	for ( int i=0; i<pEvent->VendorSell.m_count; i++ )
	{
		CGrayUID uid( pEvent->VendorSell.m_item[i].m_UID );
		CItemVendable * pItem = dynamic_cast <CItemVendable *> (uid.ItemFind());
		if ( pItem == NULL )
			continue;

		// Do we still have it ? (cheat check)
		if ( pItem->GetTopLevelObj() != m_pChar )
			continue;

		// Find the valid sell item from vendors stuff.
		CItemVendable * pItemSell = CChar::NPC_FindVendableItem( pItem, pContBuy, pContStock );
		if ( pItemSell == NULL )
			continue;

		// Now how much did i say i wanted to sell ?
		int amount = pEvent->VendorSell.m_item[i].m_amount;
		if ( pItem->GetAmount() < amount )	// Selling more than i have ?
		{
			amount = pItem->GetAmount();
		}

		long iPrice = pItemSell->GetVendorPrice(iConvertFactor) * amount;

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
			pItem->RemoveFromView();
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
		sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_NCP_VENDOR_SELL_TY ),
			iGold, (iGold==1) ? "" : g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_CA ) );
		pVendor->Speak( sMsg );

		if ( fShortfall )
		{
			pVendor->Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_NOMONEY ) );
		}

		m_pChar->AddGoldToPack( iGold );
	}
	else
	{
		if ( fShortfall )
		{
			pVendor->Speak( g_Cfg.GetDefaultMsg( DEFMSG_NPC_VENDOR_CANTAFFORD ) );
		}
	}

	addVendorClose( pVendor );
}



void CClient::Event_BBoardRequest( CGrayUID uid, const CEvent * pEvent )
{
	// Answer a request reguarding the BBoard.
	// addBulletinBoard

	CItemContainer * pBoard = dynamic_cast <CItemContainer *> ( uid.ItemFind());
	if ( ! m_pChar->CanSee( pBoard ))
	{
		addObjectRemoveCantSee( uid, "the board" );
		return;
	}

	ASSERT(pBoard);
	DEBUG_CHECK( pBoard->IsType(IT_BBOARD));
	CGrayUID uidMsg( (DWORD)( pEvent->BBoard.m_UIDMsg ) );

	switch ( pEvent->BBoard.m_flag )
	{
	case BBOARDF_REQ_FULL:
	case BBOARDF_REQ_HEAD:
		// request for message header and/or body.
		if ( pEvent->BBoard.m_len != 0x0c )
		{
			DEBUG_ERR(( "%x:BBoard feed back message bad length %d" DEBUG_CR, m_Socket.GetSocket(), (int) pEvent->BBoard.m_len ));
			return;
		}
		if ( ! addBBoardMessage( pBoard, (BBOARDF_TYPE) pEvent->BBoard.m_flag, uidMsg ))
		{
			// sanity check fails.
			addObjectRemoveCantSee( (DWORD)( pEvent->BBoard.m_UIDMsg ), "the message" );
			return;
		}
		break;
	case BBOARDF_NEW_MSG:
		// Submit a message
		if ( pEvent->BBoard.m_len < 0x0c )
		{
			DEBUG_ERR(( "%x:BBoard feed back message bad length %d" DEBUG_CR, m_Socket.GetSocket(), (int) pEvent->BBoard.m_len ));
			return;
		}
		if ( ! m_pChar->CanTouch( pBoard ))
		{
			SysMessageDefault( DEFMSG_ITEMUSE_BBOARD_REACH );
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
				DEBUG_ERR(( "%x:BBoard can't create message item" DEBUG_CR, m_Socket.GetSocket()));
				return;
			}

			int lenstr = pEvent->BBoard.m_data[0];
			pMsgNew->SetAttr( ATTR_MOVE_NEVER );


			if ( !Str_Check( (LPCTSTR) &pEvent->BBoard.m_data[1] ) )
			   pMsgNew->SetName( (LPCTSTR) &pEvent->BBoard.m_data[1] );
			else
			{
				g_Log.Event( LOGL_WARN|LOGM_CHEAT,
					"%x:Cheater '%s' is submitting hacked BBoard message name" DEBUG_CR,
					this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
				pMsgNew->SetName( "Cheater" );
			}

			pMsgNew->m_itBook.m_Time = CServTime::GetCurrentTime();
			pMsgNew->m_sAuthor = m_pChar->GetName();
			pMsgNew->m_uidLink = m_pChar->GetUID();	// Link it to you forever.

			int len = 1 + lenstr;
			int lines = pEvent->BBoard.m_data[len++];
			if ( lines > 32 ) lines = 32;	// limit this.

			while ( lines-- )
			{
				lenstr = pEvent->BBoard.m_data[len++];
            if ( ! Str_Check( (LPCTSTR) &pEvent->BBoard.m_data[len] ) ) {
				   pMsgNew->AddPageText( (LPCTSTR) &pEvent->BBoard.m_data[len] );
            } else {
               g_Log.Event( LOGL_WARN|LOGM_CHEAT,
	            "%x:Cheater '%s' is submitting hacked BBoard message" DEBUG_CR,
	            this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
            }
				len += lenstr;
			}

			pBoard->ContentAdd( pMsgNew );
		}
		break;

	case BBOARDF_DELETE:
		// remove the msg. (if it is yours?)
		{
			CItemMessage * pMsgItem = dynamic_cast <CItemMessage *> ( uidMsg.ItemFind());
			if ( ! pBoard->IsItemInside( pMsgItem ))
			{
				SysMessageDefault( DEFMSG_ITEMUSE_BBOARD_COR );
				return;
			}
			if ( ! IsPriv(PRIV_GM) && pMsgItem->m_uidLink != m_pChar->GetUID())
			{
				SysMessageDefault( DEFMSG_ITEMUSE_BBOARD_DEL );
				return;
			}

			pMsgItem->Delete();
		}
		break;

	default:
		DEBUG_ERR(( "%x:BBoard unknown flag %d" DEBUG_CR, m_Socket.GetSocket(), (int) pEvent->BBoard.m_flag ));
		return;
	}
}



void CClient::Event_SecureTrade( CGrayUID uid, const CEvent * pEvent )
{
	// pressed a button on the secure trade window.

	CItemContainer * pCont = dynamic_cast <CItemContainer *> ( uid.ItemFind());
	if ( pCont == NULL )
		return;
	if ( m_pChar != pCont->GetParent())
		return;

	// perform the trade.
	switch ( pEvent->SecureTrade.m_action )
	{
	case SECURE_TRADE_CLOSE: // Cancel trade.  Send each person cancel messages, move items.
		pCont->Delete();
		return;
	case SECURE_TRADE_CHANGE: // Change check marks.  Possibly conclude trade
		if ( m_pChar->GetDist( pCont ) > UO_MAP_VIEW_SIZE )
		{
			// To far away.
			SysMessageDefault( DEFMSG_TRADE_TOOFAR );
			return;
		}
		long need2wait(0);
		CVarDefBase *vardef = pCont->GetTagDefs()->GetKey("wait1sec");
		if ( vardef ) need2wait = vardef->GetValNum();
		if ( need2wait > 0 )
		{
			long timerow = g_World.GetCurrentTime().GetTimeRaw();
			if ( need2wait > timerow )
			{
				CGString	sMsg;
				long		seconds = (need2wait-timerow)/TICK_PER_SEC;
				sMsg.Format( _TEXT( g_Cfg.GetDefaultMsg( DEFMSG_TRADE_WAIT ) ), seconds);
				SysMessage(sMsg);
				return;
			}
		}
		pCont->Trade_Status( pEvent->SecureTrade.m_UID1 );
		return;
	}
}



void CClient::Event_Profile( BYTE fWriteMode, CGrayUID uid, const CEvent * pEvent )
{
	// mode = 0 = Get profile, 1 = Set profile

	// DEBUG_MSG(( "%x:XCMD_CharProfile" DEBUG_CR, m_Socket.GetSocket()));

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return;
	SetPrivFlags( PRIV_T2A );
	if ( pChar->OnTrigger( CTRIG_Profile, m_pChar ) == TRIGRET_RET_TRUE )
		return;

	if ( pChar->m_pPlayer == NULL )
		return;

	if ( fWriteMode )
	{
		// write stuff to the profile.
		ASSERT( pEvent->CharProfile.m_Cmd == XCMD_CharProfile );
		if ( m_pChar != pChar )
		{
			if ( ! IsPriv(PRIV_GM))
				return;
			if ( m_pChar->GetPrivLevel() < pChar->GetPrivLevel())
				return;
		}

		const int iSizeAll = sizeof(pEvent->CharProfile);
		const int iSizeTxt = sizeof(pEvent->CharProfile.m_utext);

		int len = pEvent->CharProfile.m_len;
		if ( len <= (sizeof(pEvent->CharProfile)-sizeof(pEvent->CharProfile.m_utext)))
			return;

		int iTextLen = pEvent->CharProfile.m_textlen;
		if ( iTextLen*sizeof(NCHAR) != len - (sizeof(pEvent->CharProfile)-sizeof(pEvent->CharProfile.m_utext)) )
			return;

		// BYTE pEvent->CharProfile.m_unk1;	// 8
		BYTE retcode = pEvent->CharProfile.m_retcode;	// 0=canceled, 1=okayed or something similar???

		TCHAR szLine[SCRIPT_MAX_LINE_LEN-16];
		int iWLen = CvtNUNICODEToSystem( szLine, COUNTOF(szLine), pEvent->CharProfile.m_utext, iTextLen );
      if ( /*Str_Check( szLine )*/ szLine && ( strchr( szLine, 0x0A ) != 0 ) ) {
         g_Log.Event( LOGL_WARN|LOGM_CHEAT,
	      "%x:Cheater '%s' is submitting hacked character profile" DEBUG_CR,
	      this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
         return;
      }
		pChar->m_pPlayer->m_sProfile = szLine;
		return;
	}

	bool fIncognito = m_pChar->IsStatFlag( STATF_Incognito ) && ! IsPriv(PRIV_GM);

	// addCharProfile();
	CCommand cmd;

	cmd.CharProfile.m_Cmd = XCMD_CharProfile;
	cmd.CharProfile.m_UID = uid;

	int len = strcpylen( cmd.CharProfile.m_title, pChar->GetName());
	len++;

	CGString sConstText;
	sConstText.Format( "%s, %s", pChar->Noto_GetTitle(), (LPCTSTR) pChar->GetTradeTitle());

	int iWLen = CvtSystemToNUNICODE(
		(NCHAR *) ( cmd.CharProfile.m_title + len ), 1024,
		sConstText, sConstText.GetLength() );

	len += (iWLen+1)*sizeof(NCHAR);

	LPCTSTR pszProfile = fIncognito ? "" : ((LPCTSTR) pChar->m_pPlayer->m_sProfile );
	iWLen = CvtSystemToNUNICODE(
		(NCHAR *) ( cmd.CharProfile.m_title + len ), SCRIPT_MAX_LINE_LEN-16,
		pszProfile, -1 );

	len += (iWLen+1)*sizeof(NCHAR);
	len += 7;
	cmd.CharProfile.m_len = len;

	xSendPkt( &cmd, len );
}



void CClient::Event_MailMsg( CGrayUID uid1, CGrayUID uid2 )
{
	// NOTE: How do i protect this from spamming others !!!
	// Drag the mail bag to this clients char.

	CChar * pChar = uid1.CharFind();

	if (pChar->OnTrigger(CTRIG_UserMailBag, m_pChar, NULL) == TRIGRET_RET_TRUE)
		return;

	if ( pChar == NULL )
	{
		SysMessageDefault( DEFMSG_MAILBAG_DROP_1 );
		return;
	}
	if ( pChar == m_pChar ) // this is normal (for some reason) at startup.
	{
		return;
	}
	// Might be an NPC ?
	CGString sMsg;
	sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_MAILBAG_DROP_2 ), (LPCTSTR) m_pChar->GetName() );
	pChar->SysMessage( sMsg );
}



void CClient::Event_ToolTip( CGrayUID uid )
{
	SetPrivFlags( PRIV_T2A );

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
		return;

	if ( pObj->OnTrigger( "@ToolTip", m_pChar ) == TRIGRET_RET_TRUE )	// CTRIG_ToolTip, ITRIG_ToolTip
		return;

	CGString sStr;
	sStr.Format( "'%s'", (LPCTSTR) pObj->GetName());
	addToolTip( uid.ObjFind(), sStr );
}



void CClient::Event_ExtData( EXTDATA_TYPE type, const CExtData * pData, int len )
{
	// XCMD_ExtData = 5 bytes of overhead before this.
	// DEBUG_MSG(( "%x:XCMD_ExtData t=%d,l=%d" DEBUG_CR, m_Socket.GetSocket(), type, len ));

	switch ( type )
	{
	case EXTDATA_UnkFromCli5:
		// Sent at start up for the party system ?
		break;
	case EXTDATA_Lang:
		// Tell what lang i use.
		GetAccount()->m_lang.Set( pData->Lang.m_code );
		break;
	case EXTDATA_Party_Msg: // = 6
		// Messages about the party we are in.
		ASSERT(m_pChar);
		switch ( pData->Party_Msg_Opt.m_code )
		{
		case PARTYMSG_Add:
			// request to add a new member. len=5. m_msg = 4x0's
//		   	CScriptTriggerArgs Args( m_pChar );
//		   	OnTrigger( CTRIG_PartyInvite, this, &Args );
			addTarget( CLIMODE_TARG_PARTY_ADD, g_Cfg.GetDefaultMsg( DEFMSG_PARTY_TARG_WHO ), false, false );
			break;
		case PARTYMSG_Disband:
			if ( m_pChar->m_pParty == NULL )
				return;
			m_pChar->m_pParty->Disband( m_pChar->GetUID());
			break;
		case PARTYMSG_Remove:
			// Request to remove this member of the party.
			if ( m_pChar->m_pParty == NULL )
				return;
			if ( len != 5 )
				return;
			m_pChar->m_pParty->RemoveChar( (DWORD) pData->Party_Msg_Rsp.m_UID, m_pChar->GetUID());
			break;
		case PARTYMSG_MsgMember:
			// Message a specific member of my party.
			if ( m_pChar->m_pParty == NULL )
				return;
			if ( len < 6 )
				return;
			m_pChar->m_pParty->MessageMember( (DWORD) pData->Party_Msg_Rsp.m_UID, m_pChar->GetUID(), pData->Party_Msg_Rsp.m_msg, len-1 );
			break;
		case PARTYMSG_Msg:
			// Send this message to the whole party.
			if ( len < 6 )
				return;
			if ( m_pChar->m_pParty == NULL )
			{
				// No Party !
				// We must send a response back to the client for this or it will hang !?
				CPartyDef::MessageClient( this, m_pChar->GetUID(), (const NCHAR*)( pData->Party_Msg_Opt.m_data ), len-1 );
			}
			else
			{
				m_pChar->m_pParty->MessageAll( m_pChar->GetUID(), (const NCHAR*)( pData->Party_Msg_Opt.m_data ), len-1 );
			}
			break;
		case PARTYMSG_Option:
			// set the loot flag.
			if ( m_pChar->m_pParty == NULL )
				return;
			if ( len != 2 )
				return;
			m_pChar->m_pParty->SetLootFlag( m_pChar, pData->Party_Msg_Opt.m_data[0] );
			break;
		case PARTYMSG_Accept:
			// We accept or decline the offer of an invite.
			if ( len != 5 )
				return;
			CPartyDef::AcceptEvent( m_pChar, (DWORD) pData->Party_Msg_Rsp.m_UID );
			break;

		case PARTYMSG_Decline:
			// decline party invite.
			// " You notify %s that you do not wish to join the party"
			CPartyDef::DeclineEvent( m_pChar, (DWORD) pData->Party_Msg_Rsp.m_UID );
			break;

		default:
			SysMessagef( "Unknown party system msg %d", pData->Party_Msg_Rsp.m_code );
			break;
		}
		break;
	case EXTDATA_Arrow_Click:
		SysMessageDefault( DEFMSG_FOLLOW_ARROW );
		break;
	case EXTDATA_StatusClose:
		// The status window has been closed. (need send no more updates)
		// 4 bytes = uid of the char status closed.
		break;

	case EXTDATA_Wrestle_DisArm:	// From Client: Wrestling disarm
	case EXTDATA_Wrestle_Stun:		// From Client: Wrestling stun
		SysMessageDefault( DEFMSG_WRESTLING_NOGO );
		break;

	case EXTDATA_Yawn:
		m_pChar->Emote( "Yawn" );
		m_pChar->UpdateAnimate( ANIM_FIDGET_YAWN );
		break;

	case EXTDATA_Unk15:
		break;

	case EXTDATA_Popup_Request:
		Event_AOSPopupMenu( (DWORD) pData->Popup_Request.m_UID );
		break;

	case EXTDATA_Popup_Select:
		Event_AOSPopupMenu( (DWORD) pData->Popup_Select.m_UID, (WORD) pData->Popup_Select.m_EntryTag );
		break;

	case EXTDATA_NewSpellSelect:
		{
			WORD iSpell = pData->NewSpellSelect.m_SpellId;
			DEBUG_ERR(("Spell selected: %d (%x)\n", iSpell, iSpell));

			Cmd_Skill_Magery( (SPELL_TYPE) iSpell, m_pChar );
		}
		break;

	default:
		SysMessagef( "Unknown extended msg %d.", type );
		break;
	}
}



void CClient::Event_ExtCmd( EXTCMD_TYPE type, const char * pszName )
{
	// parse the args.
	TCHAR szTmp[ MAX_TALK_BUFFER ];
	strcpylen( szTmp, pszName, sizeof(szTmp));


	if ( m_pChar )
	{
		CScriptTriggerArgs	Args( szTmp );
		Args.m_iN1	= type;
		if ( m_pChar->OnTrigger( CTRIG_UserExtCmd, m_pChar, &Args ) == TRIGRET_RET_TRUE )
			return;
		strcpy( szTmp, Args.m_s1 );
	}

	TCHAR * ppArgs[2];
	Str_ParseCmds( szTmp, ppArgs, COUNTOF(ppArgs), " " );

	switch ( type )
	{
	case EXTCMD_OPEN_SPELLBOOK: // 67 = open spell book if we have one.
		{
			CItem * pBook = m_pChar->GetSpellbook();
			if ( pBook == NULL )
			{
				SysMessageDefault( DEFMSG_NOSPELLBOOK );
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
		if ( !strcmpi( ppArgs[0],"bow"))
			m_pChar->UpdateAnimate( ANIM_BOW );
		else if ( ! strcmpi( ppArgs[0],"salute"))
			m_pChar->UpdateAnimate( ANIM_SALUTE );
		else
		{
			DEBUG_ERR(( "%x:Event Animate '%s'" DEBUG_CR, m_Socket.GetSocket(), ppArgs[0] ));
		}
		break;

	case EXTCMD_SKILL:			// Skill
		Event_Skill_Use( (SKILL_TYPE) ATOI( ppArgs[0] ));
		break;

	case EXTCMD_AUTOTARG:	// bizarre new autotarget mode.
		// "target x y z"
		{
			CGrayUID uid( ATOI( ppArgs[0] ));
			CObjBase * pObj = uid.ObjFind();
			if ( pObj )
			{
				DEBUG_ERR(( "%x:Event Autotarget '%s' '%s'" DEBUG_CR, m_Socket.GetSocket(), pObj->GetName(), ppArgs[1] ));
			}
			else
			{
				DEBUG_ERR(( "%x:Event Autotarget UNK '%s' '%s'" DEBUG_CR, m_Socket.GetSocket(), ppArgs[0], ppArgs[1] ));
			}
		}
		break;

	case EXTCMD_CAST_MACRO:	// macro spell.
	case EXTCMD_CAST_BOOK:	// cast spell from book.
		if ( g_Cfg.IsSetOF( OF_Magic_PreCast ) )
		{
			m_tmSkillMagery.m_Spell = (SPELL_TYPE) ATOI( ppArgs[0] );
			m_pChar->m_atMagery.m_Spell = (SPELL_TYPE) ATOI( ppArgs[0] );	// m_atMagery.m_Spell
			m_Targ_UID = m_pChar->GetUID();	// default target.
			m_Targ_PrvUID = m_pChar->GetUID();
			m_pChar->Skill_Start( SKILL_MAGERY );
		}
		else
			Cmd_Skill_Magery( (SPELL_TYPE) ATOI( ppArgs[0] ), m_pChar );
		break;

	case EXTCMD_DOOR_AUTO: // open door macro = Attempt to open a door around us.
		if ( m_pChar && !m_pChar->IsStatFlag( STATF_DEAD ) )
		{
			CWorldSearch Area( m_pChar->GetTopPoint(), 4 );
			while(true)
			{
				CItem * pItem = Area.GetItem();
				if ( pItem == NULL )
					break;
				switch ( pItem->GetType() )
				{
				case IT_PORT_LOCKED:	// Can only be trigered.
				case IT_PORTCULIS:
				case IT_DOOR_LOCKED:
				case IT_DOOR:
					m_pChar->Use_Obj( pItem, false );
					return;
				}
			}
		}
		break;

	// case 107: // seen this but no idea what it does.

	default:
		DEBUG_ERR(( "%x:Event_ExtCmd unk %d, '%s'" DEBUG_CR, m_Socket.GetSocket(), type, pszName ));
	}
}



void CClient::Event_PromptResp( LPCTSTR pszText, int len )
{
	// result of addPrompt
	TCHAR szText[MAX_TALK_BUFFER];

	if ( Str_Check( pszText ) )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked prompt resp" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}

	if ( len <= 0 )	// cancel
	{
		szText[0] = '\0';
	}
	else
	{
		len = Str_GetBare( szText, pszText, sizeof(szText), "|~,=[]{|}~" );
	}

	LPCTSTR pszReName = NULL;
	LPCTSTR pszPrefix = NULL;

	CLIMODE_TYPE PrvTargMode = GetTargMode();
	ClearTargMode();

	switch ( PrvTargMode )
	{
	case CLIMODE_PROMPT_GM_PAGE_TEXT:
		// m_Targ_Text
		Cmd_GM_Page( szText );
		return;

	case CLIMODE_PROMPT_VENDOR_PRICE:
		// Setting the vendor price for an item.
		{
			if ( szText[0] == '\0' )	// cancel
				return;
			CChar * pCharVendor = m_Targ_PrvUID.CharFind();
			if ( pCharVendor )
			{
				pCharVendor->NPC_SetVendorPrice( m_Targ_UID.ItemFind(), ATOI(szText) );
			}
		}
		return;

	case CLIMODE_PROMPT_NAME_RUNE:
		pszReName = "Rune";
		pszPrefix = "Rune to:";
		break;

	case CLIMODE_PROMPT_NAME_KEY:
		pszReName = "Key";
		pszPrefix = "Key to:";
		break;

	case CLIMODE_PROMPT_NAME_SHIP:
		pszReName = "Ship";
		pszPrefix = "SS ";
		break;

	case CLIMODE_PROMPT_NAME_SIGN:
		pszReName = "Sign";
		pszPrefix = "";
		break;

	case CLIMODE_PROMPT_STONE_NAME:
		pszReName = "Stone";
		pszPrefix = "Stone for the ";
		break;

	case CLIMODE_PROMPT_STONE_SET_ABBREV:
		pszReName = "Abbreviation";
		pszPrefix = "";
		break;

	case CLIMODE_PROMPT_STONE_GRANT_TITLE:
	case CLIMODE_PROMPT_STONE_SET_TITLE:
		pszReName = "Title";
		pszPrefix = "";
		break;

	case CLIMODE_PROMPT_TARG_VERB:
		// Send a msg to the pre-tergetted player. "ETARGVERB"
		// m_Targ_UID = the target.
		// m_Targ_Text = the prefix.
		if ( szText[0] != '\0' )
		{
			CObjBase * pObj = m_Targ_UID.ObjFind();
			if ( pObj )
			{
				CScript script( m_Targ_Text, szText );
				pObj->r_Verb( script, this );
			}
		}
		return;

	default:
		// DEBUG_ERR(( "%x:Unrequested Prompt mode %d" DEBUG_CR, m_Socket.GetSocket(), PrvTargMode ));
		SysMessage( "Unexpected prompt info" );
		return;
	}

	ASSERT(pszReName);

	CGString sMsg;
	CItem * pItem = m_Targ_UID.ItemFind();

	if ( pItem == NULL || szText[0] == '\0' )
	{
		SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_RENAME_CANCEL ), pszReName );
		return;
	}

	if ( g_Cfg.IsObscene( szText ))
	{
		SysMessagef( g_Cfg.GetDefaultMsg( DEFMSG_RENAME_WNAME ), pszReName, szText );
		return;
	}

	sMsg.Format( "%s%s", pszPrefix, szText );
	switch (pItem->GetType())
	{
	case IT_STONE_GUILD:
	case IT_STONE_TOWN:
	case IT_STONE_ROOM:
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
		sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_RENAME_SUCCESS ), pszReName, (LPCTSTR) pItem->GetName());
		break;
	}

	SysMessage( sMsg );
}




void CClient::Event_Talk_Common( TCHAR * szText ) // PC speech
{
	// ??? Allow NPC's to talk to each other in the future.
	// Do hearing here so there is not feedback loop with NPC's talking to each other.

	ASSERT( m_pChar );
	ASSERT( m_pChar->m_pPlayer );
	ASSERT( m_pChar->m_pArea );


	/*
	// Special GLOBAL things heard.
	if (    ! strnicmp( szText, "home home home", 14 )
		 && m_pChar->IsStatFlag( STATF_DEAD )
		 && m_pChar->m_ptHome.IsCharValid())
	{
		if ( m_pChar->Spell_Teleport( m_pChar->m_ptHome, false, false ))
			return true;
	}
	*/

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

	static LPCTSTR const sm_szTextMurderer[] =
	{
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_TEXT_MURD_1 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_TEXT_MURD_2 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_TEXT_MURD_3 ),
		g_Cfg.GetDefaultMsg( DEFMSG_NPC_TEXT_MURD_4 ),
	};

	if ( ! strnicmp( szText, "I must consider my sins", 23 ))
	{
		int i = m_pChar->m_pPlayer->m_wMurders;
		if ( i >= COUNTOF(sm_szTextMurderer))
			i = COUNTOF(sm_szTextMurderer)-1;
		SysMessage( sm_szTextMurderer[i] );
		return;
	}

	// Guards are special
	// They can't hear u if your dead.
	bool fGhostSpeak = m_pChar->IsSpeakAsGhost();
	if ( ! fGhostSpeak && ( FindStrWord( szText, "GUARD" ) || FindStrWord( szText, "GUARDS" )))
	{
		m_pChar->CallGuards(NULL);
	}

	// Are we in a region that can hear ?
	if ( m_pChar->m_pArea->GetResourceID().IsItem())
	{
		CItemMulti * pItemMulti = dynamic_cast <CItemMulti *>( m_pChar->m_pArea->GetResourceID().ItemFind());
		if ( pItemMulti )
		{
			pItemMulti->OnHearRegion( szText, m_pChar );
		}
	}

	// Are there items on the ground that might hear u ?
	CSector * pSector = m_pChar->GetTopPoint().GetSector();
	if ( pSector->HasListenItems())
	{
		pSector->OnHearItem( m_pChar, szText );
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

		if ( pChar->IsStatFlag(STATF_COMM_CRYSTAL))
		{
			pChar->OnHearEquip( m_pChar, szText );
		}

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
			while ( ISWHITESPACE( szText[i] ))
				i++;

			if ( pChar->NPC_OnHearPetCmd( szText+i, m_pChar, !fNamed ))
			{
				if ( fNamed )
					return;
				if ( GetTargMode() == CLIMODE_TARG_PET_CMD )
					return;
				// The command might apply to other pets.
				continue;
			}
			if ( fNamed )
				break;
		}

		// Are we close to the char ?
		int iDist = m_pChar->GetTopDist3D( pChar );

		if ( pChar->Skill_GetActive() == NPCACT_TALK &&
			pChar->m_Act_Targ == m_pChar->GetUID()) // already talking to him
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
		}
	}

	if ( pChar == NULL )
	{
		i = 0;
		pChar = pCharAlt;
		if ( pChar == NULL )
			return;	// no one heard it.
	}

	// Change to all upper case for ease of search. ???
	_strupr( szText );

	// The char hears you say this.
	pChar->NPC_OnHear( &szText[i], m_pChar );
}




void CClient::Event_Talk( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode ) // PC speech
{
	if ( GetAccount() == NULL )
		return;
	ASSERT( GetChar() );

	if ( mode < 0 || mode > 9 || mode == 1 || mode == 3 || mode == 4 || mode == 5 || mode == 6 || mode == 7 )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting illegal talk mode" DEBUG_CR,
		this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}

	if ( 2 > wHue || wHue > 0x03e9 )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting illegal talk color" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}

	// store the language of choice.
	GetAccount()->m_lang.Set( NULL );	// default.

	// Rip out the unprintables first.
	TCHAR szText[MAX_TALK_BUFFER];
   TCHAR szTextG[MAX_TALK_BUFFER];
   strncpy( szTextG, pszText, MAX_TALK_BUFFER - 1 );
	int len = Str_GetBare( szText, szTextG, sizeof(szText)-1 );
	if ( len <= 0 )
		return;
	pszText = szText;

	if ( ! ( szText[0] == '.' || (m_pChar->GetID() == 0x3db && szText[0] == '=') ) || ! g_Cfg.CanUsePrivVerb( this, &pszText[1], this ) )
	{
		bool	fCancelSpeech	= false;
		char	z[MAX_TALK_BUFFER];

		if (    !g_Cfg.m_sSpeechSelf.IsEmpty()
			&& m_pChar->OnTriggerSpeech( g_Cfg.m_sSpeechSelf, (TCHAR *) szText, m_pChar, mode ) )
			fCancelSpeech	= true;

		if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
		{
			g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says '%s' mode=%d%s" DEBUG_CR, m_Socket.GetSocket(), m_pChar->GetName(), szText, mode,
				fCancelSpeech ? " (muted)" : "");
		}

		strcpy(z, pszText);
		int chars = strlen(z);
		int capitals = 0;
		int i = 0;
		for ( i = 0; i < chars; i++ )
			if (( z[i] >= 'A' ) && ( z[i] <= 'Z' ))
				capitals++;

		if (( chars > 5 ) && ((( capitals * 100 )/chars) > 75 ))
		{							// 80% of chars are in capital letters. lowercase it
			for ( i = 1; i < chars; i++ )				// instead of the 1st char
				if (( z[i] >= 'A' ) && ( z[i] <= 'Z' )) z[i] += 0x20;
		}

		if ( !fCancelSpeech )
		{
			m_pChar->SpeakUTF8( z,
								wHue,
								(TALKMODE_TYPE) mode,
								m_pChar->m_fonttype,
								GetAccount()->m_lang
								);
			Event_Talk_Common( (TCHAR *) z );
			// m_pChar->Speak( szText, wHue, mode );	// echo this back.
		}

	}
	else
	{
      		Event_Command( &pszText[1] );
	}

}



void CClient::Event_TalkUNICODE( const CEvent * pEvent )
{
	// Get the text in wide bytes.
	// ENU = English
	// FRC = French
	// mode == TALKMODE_SYSTEM if coming from player talking.

	CAccount * pAccount = GetAccount();
	if ( pAccount == NULL )	// this should not happen
		return;
	TALKMODE_TYPE	Mode	= (TALKMODE_TYPE) pEvent->TalkUNICODE.m_mode;
	unsigned char	mMode	= pEvent->TalkUNICODE.m_mode;
	if ( mMode & TALKMODE_TOKENIZED )
		mMode &= ~TALKMODE_TOKENIZED;

	if ( mMode < 0 || mMode > 9 || mMode == 1 || mMode == 3 || mMode == 4 || mMode == 5 || mMode == 6 || mMode == 7 )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting illegal unicode talk mode %x" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName(), mMode);
		return;
	}

	if (	pEvent->TalkUNICODE.m_wHue < 0
	|| pEvent->TalkUNICODE.m_wHue > 0x03e9 )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting illegal unicode talk color" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}

	// store the default language of choice. CLanguageID
	pAccount->m_lang.Set( pEvent->TalkUNICODE.m_lang );

	int iLen = pEvent->TalkUNICODE.m_len - sizeof(pEvent->TalkUNICODE);

	if ( Mode & TALKMODE_TOKENIZED )
	{
		// A really odd client "feature" in v2.0.7 .
		// This is not really UNICODE !!! odd tokenized normal text.
		// skip 3 or more bytes of junk,
		// 00 10 01 = I want the balance
		// 00 10 02 = bank
		// 00 11 71 = buy
		// 00 30 03 00 40 04 = check join member
		// 00 20 02 00 20 = buy bank sdfsdf
		// 00 40 01 00 20 07 00 20 = bank guards balance
		// 00 40 01 00 20 07 00 20 = sdf bank bbb guards ccc balance ddd
		// 00 40 01 00 20 07 00 20 = balance guards bank
		// 00 10 07 = guards sdf sdf
		// 00 30 36 04 f1 61 = stop (this pattern makes no sense)
		// 00 20 07 15 c0 = .add c_h_guard
		// and skill
		Mode = (TALKMODE_TYPE) (((int)Mode) &~TALKMODE_TOKENIZED);
		LPCTSTR pszText = (LPCTSTR)(pEvent->TalkUNICODE.m_utext);

		int i;
		for ( i=0; i<iLen; i++ )
		{
			TCHAR ch = pszText[i];
			if ( ch >= 0x20 )
				break;
			i++;
			ch = pszText[i];
			if ( ((BYTE) ch ) > 0xc0 )
			{
				i++;
				continue;
			}
			ch = pszText[i+1];
			if ( i <= 2 || ch < 0x20 || ch >= 0x80 )
				i++;
		}
		Event_Talk( pszText+i, pEvent->TalkUNICODE.m_wHue, TALKMODE_SYSTEM );
		return;
	}


	TCHAR szText[MAX_TALK_BUFFER];
   	NWORD wszText[MAX_TALK_BUFFER];
	int iLenChars = iLen/sizeof(WCHAR);
	const NWORD *	puText;

	if ( g_Cfg.IsSetEF( EF_UNICODE ) )
	{
		wcsncpy( (wchar_t *)wszText, (wchar_t *)pEvent->TalkUNICODE.m_utext,
				MAX_TALK_BUFFER - 2 );
   		iLen = CvtNUNICODEToSystem( szText, sizeof(szText), wszText, iLenChars );
		puText	= wszText;
	}
	else
	{
		puText	= &pEvent->TalkUNICODE.m_utext[0];
		iLen = CvtNUNICODEToSystem( szText, sizeof(szText),
			       	pEvent->TalkUNICODE.m_utext, iLenChars );
	}

	if ( iLen <= 0 )
		return;

#if 0
	// No double chars ? what about stuff over 0x80 ? Euro characters ?
	if ( iLenChars == iLen )
	{
		// If there is no real unicode in the speech then optimize by processing it as normal ?
		// It's just english anyhow ?
		Event_Talk( szText, pEvent->TalkUNICODE.m_wHue, (TALKMODE_TYPE)( pEvent->TalkUNICODE.m_mode ));
		return;
	}
#endif

	// Non-english.
	if ( ! ( szText[0] == '.' || (m_pChar->GetID() == 0x3db && szText[0] == '=') )
	  || !g_Cfg.CanUsePrivVerb( this, &szText[1], this ) )
	{
		bool	fCancelSpeech	= false;

		if (    !g_Cfg.m_sSpeechSelf.IsEmpty()
			&& m_pChar->OnTriggerSpeech( g_Cfg.m_sSpeechSelf, (TCHAR *) szText, m_pChar, Mode ) )
			fCancelSpeech	= true;

		if ( g_Log.IsLoggedMask( LOGM_PLAYER_SPEAK ))
		{
			g_Log.Event( LOGM_PLAYER_SPEAK, "%x:'%s' Says UNICODE '%s' '%s' mode=%d%s" DEBUG_CR,
				m_Socket.GetSocket(), m_pChar->GetName(), pAccount->m_lang.GetStr(), szText, Mode,
				fCancelSpeech ? " (muted)" : "" );
		}

		if ( g_Cfg.IsSetEF( EF_UNICODE ) )
		{
			int chars = strlen(szText);
			int capitals = 0;
			int i = 0;
			for ( i = 0; i < chars; i++ )
				if (( szText[i] >= 'A' ) && ( szText[i] <= 'Z' ))
					capitals++;

			if (( chars > 5 ) && ((( capitals * 100 )/chars) > 75 ))
			{							// 80% of chars are in capital letters. lowercase it
				for ( i = 1; i < chars; i++ )				// instead of the 1st char
					if (( szText[i] >= 'A' ) && ( szText[i] <= 'Z' )) szText[i] += 0x20;
				CvtSystemToNUNICODE(wszText, iLenChars, szText, chars);
			}
		}

		if ( !fCancelSpeech )
		{
			m_pChar->SpeakUTF8Ex(
				puText,
				pEvent->Talk.m_wHue,
				Mode,
				m_pChar->m_fonttype,
				pAccount->m_lang );
			Event_Talk_Common( (TCHAR*) szText );
		}
	}
	else
	{
		Event_Command( &szText[1] );
	}
}




bool CClient::Event_DeathOption( DEATH_MODE_TYPE mode, const CEvent * pEvent )
{
	if ( m_pChar == NULL )
		return false;
	if ( mode != DEATH_MODE_MANIFEST )
	{
		// Death menu option.
		if ( mode == DEATH_MODE_RES_IMMEDIATE ) // res w/penalties ?
		{
    		g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting resurrection menu packet" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
    			return true;

			// Insta res should go back to the spot of death !
			static LPCTSTR const sm_Res_Fail_Msg[] =
			{
				"The connection between your spirit and the world is too weak.",
				"Thou hast drained thyself in thy efforts to reassert thine lifeforce in the physical world.",
				"No matter how strong your efforts, you cannot reestablish contact with the physical world.",
			};
			if ( GetTargMode() != CLIMODE_DEATH )
			{
				SysMessage( sm_Res_Fail_Msg[ Calc_GetRandVal( COUNTOF( sm_Res_Fail_Msg )) ] );
			}
			else
			{
				SetTargMode();
				m_pChar->MoveToChar( m_Targ_p ); // Insta res takes us back to where we died.
				if ( ! m_pChar->Spell_Resurrection( 10, NULL ))
				{
					SysMessage( sm_Res_Fail_Msg[ Calc_GetRandVal( COUNTOF( sm_Res_Fail_Msg )) ] );
				}
			}
		}
		else // DEATH_MODE_PLAY_GHOST
		{
			// Play as a ghost.
			// "As the mortal pains fade, you become aware of yourself as a spirit."
			SysMessage( "You are a ghost" );
			addSound( 0x17f );	// Creepy noise.
		}

		addPlayerStart( m_pChar ); // Do practically a full reset (to clear death menu)
		return( true );
	}

	// Toggle manifest mode. (this has more data) (anomoly to size rules)
	if ( ! xCheckMsgSize( sizeof( pEvent->DeathMenu )))
		return(false);
	Event_CombatMode( pEvent->DeathMenu.m_manifest );

	return( true );
}



void CClient::Event_SetName( CGrayUID uid, const char * pszCharName )
{
	// Set the name in the character status window.
	ASSERT( m_pChar );
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return;

   if ( Str_Check( pszCharName ) )
   {
      g_Log.Event( LOGL_WARN|LOGM_CHEAT,
			"%x:Cheater '%s' is submitting hacked character name" DEBUG_CR,
			this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
      return;
   }

	// Do we have the right to do this ?
	if ( m_pChar == pChar || ! pChar->NPC_IsOwnedBy( m_pChar, true ))
		return;

	if ( g_Cfg.IsObscene( pszCharName ))
		return;

	CScriptTriggerArgs args;
	args.m_pO1 = pChar;
	args.m_s1 = pszCharName;
	if ( OnTrigger("@Rename", this, &args) == TRIGRET_RET_TRUE ) return;
	pChar->SetName( pszCharName );
}



void CClient::Event_ScrollClose( DWORD dwContext )
{
	// A certain scroll has closed.
	DEBUG_MSG(( "%x:XCMD_Scroll(close) 0%x" DEBUG_CR, m_Socket.GetSocket(), dwContext ));

	// Make a trigger out of this.
}



void CClient::Event_MenuChoice( const CEvent * pEvent ) // Choice from GMMenu or Itemmenu received
{
	// Select from a menu. CMenuItem
	// result of addItemMenu call previous.
	// select = 0 = cancel.

	CGrayUID uidItem( pEvent->MenuChoice.m_UID );
	WORD context = pEvent->MenuChoice.m_context;

	if ( context != GetTargMode() || uidItem != m_tmMenu.m_UID )
	{
		// DEBUG_ERR(( "%x: Menu choice unrequested %d!=%d" DEBUG_CR, m_Socket.GetSocket(), context, m_Targ_Mode ));
		SysMessage( "Unexpected menu info" );
		return;
	}

	ClearTargMode();
	WORD select = pEvent->MenuChoice.m_select;

	// Item Script or GM menu script got us here.
	switch ( context )
	{
	case CLIMODE_MENU:
		// A generic menu from script.
		Menu_OnSelect( m_tmMenu.m_ResourceID, select, uidItem.ObjFind() );
		return;
	case CLIMODE_MENU_SKILL:
		// Some skill menu got us here.
		if ( select >= COUNTOF(m_tmMenu.m_Item))
			return;
		Cmd_Skill_Menu( m_tmMenu.m_ResourceID, (select) ? m_tmMenu.m_Item[select] : 0 );
		return;
	case CLIMODE_MENU_SKILL_TRACK_SETUP:
		// PreTracking menu got us here.
		Cmd_Skill_Tracking( select, false );
		return;
	case CLIMODE_MENU_SKILL_TRACK:
		// Tracking menu got us here. Start tracking the selected creature.
		Cmd_Skill_Tracking( select, true );
		return;

	case CLIMODE_MENU_GM_PAGES:
		// Select a GM page from the menu.
		Cmd_GM_PageSelect( select );
		return;
	case CLIMODE_MENU_EDIT:
		// m_Targ_Text = what are we doing to it ?
		Cmd_EditItem( uidItem.ObjFind(), select );
		return;
	default:
		DEBUG_ERR(( "%x:Unknown Targetting mode for menu %d" DEBUG_CR, m_Socket.GetSocket(), context ));
		return;
	}
}



void CClient::Event_GumpInpValRet( const CEvent * pEvent )
{
	// Text was typed into the gump on the screen.
	// pEvent->GumpInpValRet
	// result of addGumpInputBox. GumpInputBox
	// ARGS:
	// 	m_Targ_UID = pObj->GetUID();
	//  m_Targ_Text = verb

	CGrayUID uidItem( pEvent->GumpInpValRet.m_UID );
	WORD context = pEvent->GumpInpValRet.m_context;	// word context is odd.

	BYTE retcode = pEvent->GumpInpValRet.m_retcode; // 0=canceled, 1=okayed
	WORD textlen = pEvent->GumpInpValRet.m_textlen; // length of text entered
	LPCTSTR pszText = pEvent->GumpInpValRet.m_text;

	TCHAR *		pFix;
	if ( ( pFix = strchr( pszText, '\n' ) ) )
		*pFix	= '\0';
	if ( ( pFix = strchr( pszText, '\r' ) ) )
		*pFix	= '\0';
	if ( ( pFix = strchr( pszText, '\t' ) ) )
		*pFix	= ' ';

	if ( GetTargMode() != CLIMODE_INPVAL || uidItem != m_Targ_UID )
	{
		// DEBUG_ERR(( "%x:Event_GumpInpValRetIn unexpected input %d!=%d" DEBUG_CR, m_Socket.GetSocket(), context, GetTargMode()));
		SysMessage( "Unexpected text input" );
		return;
	}

	ClearTargMode();

	CObjBase * pObj = uidItem.ObjFind();
	if ( pObj == NULL )
		return;

	// take action based on the parent context.
	if (retcode == 1)	// ok
	{
		// Properties Dialog, page x
		// m_Targ_Text = the verb we are dealing with.
		// m_Prop_UID = object we are after.

		CScript script( m_Targ_Text, pszText );
		bool fRet = pObj->r_Verb( script, m_pChar );
		if ( ! fRet )
		{
			SysMessagef( "Invalid set: %s = %s", (LPCTSTR) m_Targ_Text, (LPCTSTR) pszText );
		}
		else
		{
			if ( IsPriv( PRIV_DETAIL ))
			{
				SysMessagef( "Set: %s = %s", (LPCTSTR) m_Targ_Text, (LPCTSTR) pszText );
			}
			pObj->RemoveFromView(); // weird client thing
			pObj->Update();
		}

		g_Log.Event( LOGM_GM_CMDS, "%x:'%s' tweak uid=0%x (%s) to '%s %s'=%d" DEBUG_CR,
			m_Socket.GetSocket(), (LPCTSTR) GetName(),
			(DWORD) pObj->GetUID(), (LPCTSTR) pObj->GetName(),
			(LPCTSTR) m_Targ_Text, (LPCTSTR) pszText, fRet );
	}

	// Dialog_Setup( CLIMODE_DIALOG, m_tmInpVal.m_PrvGumpID, 0, pObj ); // Since we haven't any information from where this come from we can't put back the client.

	// Dialog_Setup( CLIMODE_DIALOG, RESOURCE_ID( RES_DIALOG, context ), 0, pObj ); // put back the client.
}



LPCTSTR const CDialogResponseArgs::sm_szLoadKeys[] =
{
	"ARGCHK",
	"ARGTXT",
	NULL,
};



bool CDialogResponseArgs::r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	if ( ! strnicmp( pszKey, sm_szLoadKeys[0], 6 ))	// ARGCHK
	{
		// CGTypedArray <DWORD,DWORD> m_CheckArray;
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);

		int iQty = m_CheckArray.GetCount();
		if ( pszKey[0] == '\0' )
		{
			sVal.FormatVal(iQty);
			return( true );
		}

		int iNum = Exp_GetSingle( pszKey );
		SKIP_SEPERATORS(pszKey);
		for ( int i=0; i<iQty; i++ )
		{
			if ( iNum == m_CheckArray[i] )
			{
				sVal = "1";
				return( true );
			}
		}
		sVal = "0";
		return( true );
	}
	if ( ! strnicmp( pszKey, sm_szLoadKeys[1], 6 ))	// ARGTXT
	{
		// CGObArray< CDialogResponseString *> m_TextArray;
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);

		int iQty = m_TextArray.GetCount();
		if ( pszKey[0] == '\0' )
		{
			sVal.FormatVal(iQty);
			return( true );
		}

		int iNum = Exp_GetSingle( pszKey );
		SKIP_SEPERATORS(pszKey);

		for ( int i=0; i<m_TextArray.GetCount(); i++ )
		{
			if ( iNum == m_TextArray[i]->m_ID )
			{
				sVal = m_TextArray[i]->m_sText;
				return( true );
			}
		}
		sVal.Empty();
		return( false );
	}
	EXC_CATCH("CDialogResponseArgs");
	return( CScriptTriggerArgs::r_WriteVal( pszKey, sVal, pSrc ));
}




void CClient::Event_GumpDialogRet( const CEvent * pEvent )
{
	// CLIMODE_DIALOG
	// initiated by addGumpDialog()
	// A button was pressed in a gump on the screen.
	// possibly multiple check boxes.

	// First let's completely decode this packet
	CGrayUID	uid		( pEvent->GumpDialogRet.m_UID );
	DWORD	context		= pEvent->GumpDialogRet.m_context;
	DWORD dwButtonID	= pEvent->GumpDialogRet.m_buttonID;

	// relying on the context given by the gump might be a security problem, much like
	// relying on the uid returned.
	// maybe keep a memory for each gump?
/*

	int		currContext	= GetTargMode();
	if ( context != currContext || uid != m_tmGumpDialog.m_UID )
	{
		// DEBUG_ERR(( "%x: Event_GumpDialogRet unexpected input %d!=%d" DEBUG_CR, m_Socket.GetSocket(), context, GetTargMode()));
		SysMessage( "Unexpected button input" );
		return;
	}
*/

	CObjBase * pObj = uid.ObjFind();

	// Virtue button -- Handleing this here because the packet is a little different and causes exceptions somewhere
	if ( ( context == CLIMODE_DIALOG_VIRTUE ) && ( (CObjBase *)m_pChar == pObj ) )
	{
		m_pChar->OnTrigger( CTRIG_UserVirtue, (CTextConsole *) m_pChar, NULL );
		return;
	}

	// Add here gump sanity checks
	bool cancontinue = false;
	if ( m_pChar->Memory_FindTypes(MEMORY_GUMPRECORD) != NULL )
	{
		CItem * pCheckItem = m_pChar->GetContentHead();

		for ( ; pCheckItem!=NULL; pCheckItem=pCheckItem->GetNext())
		{
			if ( ! pCheckItem->IsMemoryTypes(MEMORY_GUMPRECORD))
				continue;

			CItemMemory * pCheckMemory = dynamic_cast <CItemMemory *>( pCheckItem );

			if ( pCheckMemory )
			{
				if ( pCheckMemory->m_itNormal.m_more1 == context && pCheckMemory->m_itNormal.m_more2 == (DWORD)uid )
				{
					// Before deleting memory, if climode is one of these, set the m_Targ_UID
					if (context == CLIMODE_DIALOG_HAIR_DYE || context == CLIMODE_DIALOG_GUILD )
					{
						CVarDefBase * sTempVal = pCheckMemory->GetTagDefs()->GetKey( "targ_uid" );
						if ( sTempVal )
						{
							m_Targ_UID = (DWORD)sTempVal->GetValNum();
						}
						else
						{
							m_Targ_UID = 0;
						}
					}
					// Finish the work
					pCheckMemory->Delete();
					// Ok we called this gump
					cancontinue = true;
					break;
				}
			}
		}
	}

	if ( !cancontinue )
	{
		g_Log.Event( LOGL_WARN|LOGM_CHEAT, "%x:Cheater '%s' is using 3rd party tools to send bad gump packets" DEBUG_CR, this->m_Socket.GetSocket(), (LPCTSTR) this->GetAccount()->GetName());
		return;
	}

	// package up the gump response info.
	CDialogResponseArgs resp;

	DWORD iCheckQty = pEvent->GumpDialogRet.m_checkQty; // this has the total of all checked boxes and radios
	int i = 0;
	for ( ; i < iCheckQty; i++ ) // Store the returned checked boxes' ids for possible later use
	{
		resp.m_CheckArray.Add( pEvent->GumpDialogRet.m_checkIds[i] );
		// DEBUG_MSG(( "box[%i]: id = %i" DEBUG_CR, i, iCheckID[i] ));
	}

	// Find out how many textentry boxes we have that returned data
	CEvent * pMsg = (CEvent *)(((BYTE*)(pEvent))+(iCheckQty-1)*sizeof(pEvent->GumpDialogRet.m_checkIds[0]));
	DWORD iTextQty = pMsg->GumpDialogRet.m_textQty;
	for ( i = 0; i < iTextQty; i++)
	{
		// Get the length....no need to store this permanently
		int lenstr = pMsg->GumpDialogRet.m_texts[0].m_len;

		TCHAR *scratch = Str_GetTemp(); // use this as scratch storage

		// Do a loop and "convert" from unicode to normal ascii
		CvtNUNICODEToSystem( scratch, SCRIPT_MAX_LINE_LEN, pMsg->GumpDialogRet.m_texts[0].m_utext, lenstr );

		TCHAR *		pFix;
		if ( ( pFix = strchr( scratch, '\n' ) ) )
			*pFix	= '\0';
		if ( ( pFix = strchr( scratch, '\r' ) ) )
			*pFix	= '\0';
		if ( ( pFix = strchr( scratch, '\t' ) ) )
			*pFix	= ' ';

		resp.AddText( pMsg->GumpDialogRet.m_texts[0].m_id, scratch );

		lenstr = sizeof(pMsg->GumpDialogRet.m_texts[0]) + ( lenstr - 1 ) * sizeof(NCHAR);
		pMsg = (CEvent *)(((BYTE*)pMsg)+lenstr);
	}

	// ClearTargMode();

	switch ( context ) // This is the page number
	{

	case CLIMODE_DIALOG_ADMIN: // Admin console stuff comes here
		{
			switch ( dwButtonID ) // Button presses come here
			{
			case 801: // Previous page
				addGumpDialogAdmin( m_tmGumpAdmin.m_Page - 1 );
				break;
			case 802: // Next page
				addGumpDialogAdmin( m_tmGumpAdmin.m_Page + 1 );
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
				if ( dwButtonID >= COUNTOF( m_tmGumpAdmin.m_Item ))
					return;
				DEBUG_CHECK( dwButtonID < ADMIN_CLIENTS_PER_PAGE );
				m_Targ_UID = m_tmGumpAdmin.m_Item[dwButtonID];
				if ( m_Targ_UID.IsValidUID())
				{
#if 1
					CScript script( "f_AdminMenu" );
					pObj->r_Verb( script, this );
#else
					Menu_Setup( g_Cfg.ResourceGetIDType( RES_MENU, _TEXT("MENU_ADMIN")));
#endif
				}
				else
					addGumpDialogAdmin( 0 );
				break;
			}
		}
		return;

	case CLIMODE_DIALOG_GUILD: // Guild/Leige/Townstones stuff comes here
		{
			CItemStone * pStone = dynamic_cast <CItemStone *> ( m_Targ_UID.ItemFind());
			if ( pStone == NULL )
				return;
			if ( pStone->OnDialogButton( this, (STONEDISP_TYPE) dwButtonID, resp ))
				return;
		}
		break;

	case CLIMODE_DIALOG_HAIR_DYE: // Using hair dye
		{
			if (dwButtonID == 0) // They cancelled
				return;
			if ( resp.m_CheckArray.GetCount() <= 0 ||
				resp.m_CheckArray[0] == 0) // They didn't pick a wHue, but they hit OK
				return;
			CItem * pItem = m_Targ_UID.ItemFind();
			if ( pItem == NULL )
				return;
			if ( ! pItem->IsType(IT_HAIR_DYE) )
				return;
			if ( ! m_pChar->CanTouch( (CObjBase *)pItem ) )
				return;
			CItem * pFacialHair = m_pChar->LayerFind( LAYER_BEARD );
			CItem * pHeadHair = m_pChar->LayerFind( LAYER_HAIR );
			if (!pFacialHair && !pHeadHair)
			{
				SysMessage("You have no hair to dye!");
				return;
			}
			if (pFacialHair)
				pFacialHair->SetHue( resp.m_CheckArray[0] + 1 );
			if (pHeadHair)
				pHeadHair->SetHue( resp.m_CheckArray[0] + 1 );
			m_pChar->Update();
			pItem->ConsumeAmount();// Consume it
		}
		return;
/*

	case CLIMODE_DIALOG_TOME:	// The new spell book.
		// ??? Make this just a table of triggers.
		if (dwButtonID <= 99) // Casting a spell
		{
			// TODO: cast the spell
			break;
		}
		else if (dwButtonID <= 199) // Open a page in spell book
		{
			m_pChar->Sound( 0x55 );
			Dialog_Setup( CLIMODE_DIALOG_TOME, (CLIMODE_TYPE) dwButtonID, 0, pObj );
			return;
		}
		else if (dwButtonID <= 299) // Making an icon
		{
			Dialog_Setup( CLIMODE_DIALOG_TOME, (CLIMODE_TYPE) dwButtonID, 0, pObj  );
			return;
		}
		else if (dwButtonID <= 399) // Casting from icon
		{
			Dialog_Setup( CLIMODE_DIALOG_TOME, (CLIMODE_TYPE)( dwButtonID - 100 ), 0, pObj );
			// TODO: cast the spell
			return;
		}
		break;
*/
	}

	RESOURCE_ID_BASE	rid	= RESOURCE_ID(RES_DIALOG,context);
	//
	// Call the scripted response. Lose all the checks and text.
	//
	Dialog_OnButton( rid, dwButtonID, pObj, &resp );
}




bool CClient::Event_DoubleClick( CGrayUID uid, bool fMacro, bool fTestTouch, bool fScript )
{
	// Try to use the object in some way.
	// will trigger a OnTarg_Use_Item() ussually.
	// fMacro = ALTP vs dbl click. no unmount.

	// Allow some static in game objects to have function?
	// Not possible with dclick.

	ASSERT(m_pChar);
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_DoubleClick 0%x" DEBUG_CR, m_Socket.GetSocket(), (DWORD) uid ));
	}

	CObjBase * pObj = uid.ObjFind();
	if ( fTestTouch && !m_pChar->CanSee( pObj ) )
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
		return Cmd_Use_Item( dynamic_cast <CItem *>(pObj), fTestTouch, fScript );
	}


	CChar * pChar = dynamic_cast <CChar*>(pObj);

	if ( pChar->OnTrigger( CTRIG_DClick, m_pChar ) == TRIGRET_RET_TRUE )
		return true;

	if ( ! fMacro )
	{
		if ( pChar == m_pChar )
		{
			if ( m_pChar->Horse_UnMount())
				return true;
		}
		if ( pChar->m_pNPC && pChar->GetNPCBrain() != NPCBRAIN_HUMAN )
		{
			if ( m_pChar->Horse_Mount( pChar ))
				return true;
			switch ( pChar->GetID())
			{
			case CREID_HORSE_PACK:
			case CREID_LLAMA_PACK:
				// pack animals open container.
				return Cmd_Use_Item( pChar->GetPackSafe(), fTestTouch );
			default:
				if ( IsPriv(PRIV_GM))
				{
					// snoop the creature.
					return Cmd_Use_Item( pChar->GetPackSafe(), false );
				}
				return false;
			}
		}
	}

	// open paper doll.

	CCommand cmd;
	cmd.PaperDoll.m_Cmd = XCMD_PaperDoll;
	cmd.PaperDoll.m_UID = pChar->GetUID();

	if ( pChar->IsStatFlag( STATF_Incognito ))
	{
		strcpy( cmd.PaperDoll.m_text, pChar->GetName());
	}
	else
	{
		int len = 0;
		CStoneMember * pGuildMember = pChar->Guild_FindMember(MEMORY_GUILD);
		if ( pGuildMember && pGuildMember->IsAbbrevOn() && pGuildMember->GetParentStone()->GetAbbrev()[0] )
		{
			len = sprintf( cmd.PaperDoll.m_text, "%s [%s], %s",
				pChar->Noto_GetTitle(), pGuildMember->GetParentStone()->GetAbbrev(),
				pGuildMember->GetTitle()[0] ? pGuildMember->GetTitle() : pChar->GetTradeTitle());
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




void CClient::Event_SingleClick( CGrayUID uid )
{
	// the ALLNAMES macro comes thru here.
	ASSERT(m_pChar);
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_MSG(( "%x:Event_SingleClick 0%lx" DEBUG_CR, m_Socket.GetSocket(), (DWORD) uid ));
	}

	CObjBase * pObj = uid.ObjFind();
	if ( ! m_pChar->CanSee( pObj ))
	{
		// ALLNAMES makes this happen as we are running thru an area.
		// So display no msg. Do not use (addObjectRemoveCantSee)
		addObjectRemove( uid );
		return;
	}

	ASSERT(pObj);
	if ( m_pChar->Skill_GetActive() == NPCACT_OneClickCmd )
	{
		CScript script( m_Targ_Text );
		bool fRet = pObj->r_Verb( script, m_pChar );
		if ( ! fRet )
		{
			SysMessageDefault( DEFMSG_CMD_INVALID );
		}
		return;
	}

	CScriptTriggerArgs Args( this );
	if ( pObj->OnTrigger( "@Click", m_pChar, &Args ) == TRIGRET_RET_TRUE )	// CTRIG_Click, ITRIG_Click
		return;

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

	SysMessagef( "Bogus item uid=0%x?", (DWORD) uid );
}



void CClient::Event_ClientVersion( const char * pData, int Len )
{
	// XCMD_ClientVersion
	// DEBUG_MSG(( "%x:XCMD_ClientVersion" DEBUG_CR, m_Socket.GetSocket()));
}



void CClient::Event_Spy( const CEvent * pEvent )
{
	// XCMD_Spy: // Spy tells us stuff about the clients computer.
	DEBUG_MSG(( "XCMD_Spy" DEBUG_CR ));
}



void CClient::Event_Target( const CEvent * pEvent )
{
	// XCMD_Target
	// If player clicks on something with the targetting cursor
	// Assume addTarget was called before this.
	// NOTE: Make sure they can actually validly trarget this item !

	ASSERT(m_pChar);
	if ( pEvent->Target.m_context != GetTargMode())
	{
		// DEBUG_ERR(( "%x: Unrequested target info ?" DEBUG_CR, m_Socket.GetSocket()));
		SysMessage( "Unexpected target info" );
		return;
	}
	if ( pEvent->Target.m_x == 0xFFFF && pEvent->Target.m_UID == 0 )
	{
		// canceled
		SetTargMode();
		return;
	}

	CGrayUID uid( pEvent->Target.m_UID );
	CPointMap pt( pEvent->Target.m_x, pEvent->Target.m_y, pEvent->Target.m_z, m_pChar->GetTopMap() );
	ITEMID_TYPE id = (ITEMID_TYPE)(WORD) pEvent->Target.m_id;	// if static tile.

	CLIMODE_TYPE prevmode = GetTargMode();
	ClearTargMode();

	CObjBase * pObj = uid.ObjFind();
	if ( IsPriv( PRIV_GM ))
	{
		if ( uid.IsValidUID() && pObj == NULL )
		{
			addObjectRemoveCantSee( uid, "the target" );
			return;
		}
	}
	else
	{
		if ( uid.IsValidUID())
		{
			if ( ! m_pChar->CanSee(pObj))
			{
				addObjectRemoveCantSee( uid, "the target" );
				return;
			}
		}
		else
		{
			// The point must be valid.
			if ( m_pChar->GetTopDist(pt) > UO_MAP_VIEW_SIZE )
			{
				return;
			}
		}
	}

	if ( pObj )
	{
		// Point inside a container is not really meaningful here.
		pt = pObj->GetTopLevelObj()->GetTopPoint();
	}

	bool fSuccess = false;

	switch ( prevmode )
	{
		// GM stuff.

	case CLIMODE_TARG_OBJ_SET:		fSuccess = OnTarg_Obj_Set( pObj ); break;
	case CLIMODE_TARG_OBJ_INFO:		fSuccess = OnTarg_Obj_Info( pObj, pt, id );  break;
	case CLIMODE_TARG_OBJ_FUNC:		fSuccess = OnTarg_Obj_Function( pObj, pt, id );  break;

	case CLIMODE_TARG_UNEXTRACT:	fSuccess = OnTarg_UnExtract( pObj, pt ); break;
	case CLIMODE_TARG_ADDITEM:		fSuccess = OnTarg_Item_Add( pObj, pt ); break;
	case CLIMODE_TARG_LINK:			fSuccess = OnTarg_Item_Link( pObj ); break;
	case CLIMODE_TARG_TILE:			fSuccess = OnTarg_Tile( pObj, pt );  break;

		// Player stuff.

	case CLIMODE_TARG_SKILL:			fSuccess = OnTarg_Skill( pObj ); break;
	case CLIMODE_TARG_SKILL_MAGERY:     fSuccess = OnTarg_Skill_Magery( pObj, pt ); break;
	case CLIMODE_TARG_SKILL_HERD_DEST:  fSuccess = OnTarg_Skill_Herd_Dest( pObj, pt ); break;
	case CLIMODE_TARG_SKILL_POISON:		fSuccess = OnTarg_Skill_Poison( pObj ); break;
	case CLIMODE_TARG_SKILL_PROVOKE:	fSuccess = OnTarg_Skill_Provoke( pObj ); break;

	case CLIMODE_TARG_REPAIR:			fSuccess = m_pChar->Use_Repair( uid.ItemFind()); break;
	case CLIMODE_TARG_PET_CMD:			fSuccess = OnTarg_Pet_Command( pObj, pt ); break;
	case CLIMODE_TARG_PET_STABLE:		fSuccess = OnTarg_Pet_Stable( uid.CharFind()); break;

	case CLIMODE_TARG_USE_ITEM:			fSuccess = OnTarg_Use_Item( pObj, pt, id );  break;
	case CLIMODE_TARG_STONE_RECRUIT:	fSuccess = OnTarg_Stone_Recruit( uid.CharFind() );  break;
	case CLIMODE_TARG_PARTY_ADD:		fSuccess = OnTarg_Party_Add( uid.CharFind() );  break;
	}
}

//----------------------------------------------------------------------



bool CClient::xCheckMsgSize0( int len )
{
	if ( ! len || len > sizeof( CEvent ))
		return( false );	// junk
	return true;
}


bool CClient::xCheckMsgSize( int len )
{
	if ( !xCheckMsgSize0( len ) )
		return false;
	// Is there enough data from client to process this packet ?
	m_bin_msg_len = len;
	return( m_bin.GetDataQty() >= len );
}

#define RETURN_FALSE() 		\
{				\
	m_bin_ErrMsg = (XCMD_TYPE) pEvent->Default.m_Cmd;	\
	return false;		\
}


bool CClient::xDispatchMsg()
{
	char	zTemp[512];
	zTemp[0] = 0;
	EXC_TRY(("xDispatchMsg"));
	// Process any messages we have Received from client.
	// RETURN:
	//  false = the message was not formatted correctly (dump the client)
	strcpy(zTemp, "checkmsgsize");
	if ( ! xCheckMsgSize( 1 ))	// just get the command
	{
		return false;
	}

	strcpy(zTemp, "crypt.isinit");
	ASSERT( m_Crypt.IsInit());

	strcpy(zTemp, "removedatalock");
	const CEvent * pEvent = (const CEvent *) m_bin.RemoveDataLock();

	// check the packet size first.
	if ( pEvent->Default.m_Cmd >= XCMD_QTY ) // bad packet type ?
	{
		DEBUG_ERR(( "Unimplemented command %d" DEBUG_CR, pEvent->Default.m_Cmd ) );
		RETURN_FALSE();
	}

#if 0
	if ( g_Packet_Lengths[pEvent->Default.m_Cmd] >= 0x8000 ) // var length
	{
		if ( ! xCheckMsgSize(3))
			return(false);
		if ( ! xCheckMsgSize(pEvent->Talk.m_len ))
			return(false);
	}
	else
	{
		// NOTE: What about client version differences !
		if ( ! xCheckMsgSize( g_Packet_Lengths[pEvent->Default.m_Cmd] ))
			return(false);
	}
#endif

	if ( pEvent->Default.m_Cmd == XCMD_Ping )
	{
		strcpy(zTemp, "ping");
		// Ping from client. 0 = request?
		// Client seems not to require a response !
		if ( ! xCheckMsgSize( sizeof( pEvent->Ping )))
			RETURN_FALSE();
		return( true );
	}

	if ( GetConnectType() != CONNECT_GAME || ! GetAccount() )
	{
		strcpy(zTemp, "loginserver/not logged: ");
		// login server or a game server that has not yet logged in.

		switch ( pEvent->Default.m_Cmd )
		{
		case XCMD_ServersReq: // First Login
			strcat(zTemp, "first login");
			if ( ! xCheckMsgSize( sizeof( pEvent->ServersReq )))
				RETURN_FALSE();
			return( Login_ServerList( pEvent->ServersReq.m_acctname, pEvent->ServersReq.m_acctpass ) == LOGIN_SUCCESS );
		case XCMD_ServerSelect:// Server Select - relay me to the server.
			strcat(zTemp, "server select");
			if ( ! xCheckMsgSize( sizeof( pEvent->ServerSelect )))
				RETURN_FALSE();
			return( Login_Relay( pEvent->ServerSelect.m_select ));
		case XCMD_CharListReq: // Second Login to select char
			strcat(zTemp, "char list req");
			if ( ! xCheckMsgSize( sizeof( pEvent->CharListReq )))
				RETURN_FALSE();
			return( Setup_ListReq( pEvent->CharListReq.m_acctname, pEvent->CharListReq.m_acctpass, false ) == LOGIN_SUCCESS );
		case XCMD_Spy: // Spy tells us stuff about the clients computer.
			strcat(zTemp, "spy");
			if ( ! xCheckMsgSize( sizeof( pEvent->Spy )))
			{
				if ( xCheckMsgSize0( sizeof( pEvent->Spy ) ) )
					m_bin_msg_len	=  m_bin.GetDataQty();
				else
					RETURN_FALSE();
			}
			Event_Spy( pEvent );
			return( true );
		case XCMD_War: // Tab = Combat Mode (toss this)
			strcat(zTemp, "war mode");
			if ( ! xCheckMsgSize( sizeof( pEvent->War )))
				RETURN_FALSE();
			return( true );
		}
		return( false );
	}

	/////////////////////////////////////////////////////
	// We should be encrypted below here. CONNECT_GAME

	// Get messages from the client.
	strcpy(zTemp, "game connect: ");
	switch ( pEvent->Default.m_Cmd )
	{
	case XCMD_Create: // Character Create
		strcat(zTemp, "create char");
		if ( m_bin_PrvMsg == XCMD_Walk && m_Crypt.GetClientIgnition() )
		{
			// just eat this. (ignition artifact)
			if ( ! xCheckMsgSize( 4 ))
				RETURN_FALSE();
			return( true );
		}
		if ( m_Crypt.GetClientVer() >= 0x126000 || IsNoCryptVer(1) )
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->Create )))
				RETURN_FALSE();
		}
		else
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->Create_v25 )))
				RETURN_FALSE();
		}
		Setup_CreateDialog( pEvent );
		return( true );
	case XCMD_CharDelete: // Character Delete
		strcat(zTemp, "delete char");
		if ( ! xCheckMsgSize( sizeof( pEvent->CharDelete )))
			RETURN_FALSE();
		addDeleteErr( Setup_Delete( pEvent->CharDelete.m_slot ));
		return( true );
	case XCMD_CharPlay: // Character Select
		strcat(zTemp, "play (select) char");
		if ( ! xCheckMsgSize( sizeof( pEvent->CharPlay )))
			RETURN_FALSE();
		if ( ! Setup_Play( pEvent->CharPlay.m_slot ))
		{
			addLoginErr( LOGIN_ERR_NONE );
		}
		return( true );
	case XCMD_TipReq: // Get Tip
		strcat(zTemp, "get a tip");
		if ( ! xCheckMsgSize( sizeof( pEvent->TipReq )))
			RETURN_FALSE();
		Event_Tips( pEvent->TipReq.m_index + 1 );
		return( true );
	case XCMD_ViewRange:
		strcat(zTemp, "view range");
		if ( ! xCheckMsgSize( 2 ) )
			RETURN_FALSE();
		return true;
	// case XCMD_WalkForce:
	//	if ( ! xCheckMsgSize( 2 ) )
	//		RETURN_FALSE();
	//	return true;
	}

	// must have a logged in char to use any other messages.
	if ( m_pChar == NULL )
	{
		strcat(zTemp, "no char");
		DEBUG_CHECK(( "No char" DEBUG_CR ));
		RETURN_FALSE();
	}

	//////////////////////////////////////////////////////
	// We are now playing.

	switch ( pEvent->Default.m_Cmd )
	{
	case XCMD_Walk: // Walk
		strcat(zTemp, "walking");
		if ( m_Crypt.GetClientVer() >= 0x126000 || IsNoCryptVer(1) )
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->Walk_v26 )))
				RETURN_FALSE();
			Event_Walking( pEvent->Walk_v26.m_dir, pEvent->Walk_v26.m_count, pEvent->Walk_v26.m_cryptcode );
		}
		else
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->Walk_v25 )))
				RETURN_FALSE();
			Event_Walking( pEvent->Walk_v25.m_dir, pEvent->Walk_v25.m_count, 0 );
		}
		break;
	case XCMD_Talk: // Speech or at least text was typed.
		strcat(zTemp, "talking");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize(pEvent->Talk.m_len ))
			RETURN_FALSE();
		Event_Talk( pEvent->Talk.m_text, pEvent->Talk.m_wHue, (TALKMODE_TYPE)( pEvent->Talk.m_mode ));
		break;
	case XCMD_Attack: // Attack
		strcat(zTemp, "attacking");
		if ( ! xCheckMsgSize( sizeof( pEvent->Click )))
			RETURN_FALSE();
		Event_Attack( (DWORD) pEvent->Click.m_UID );
		break;
	case XCMD_DClick:// Doubleclick
		strcat(zTemp, "dclicking");
		if ( ! xCheckMsgSize( sizeof( pEvent->Click )))
			RETURN_FALSE();
		Event_DoubleClick( ((DWORD)(pEvent->Click.m_UID)) &~ UID_F_RESOURCE, ((DWORD)(pEvent->Click.m_UID)) & UID_F_RESOURCE, true );
		break;
	case XCMD_ItemPickupReq: // Pick up Item
		strcat(zTemp, "pickupitem");
		if ( ! xCheckMsgSize( sizeof( pEvent->ItemPickupReq )))
			RETURN_FALSE();
		Event_Item_Pickup( (DWORD) pEvent->ItemPickupReq.m_UID, pEvent->ItemPickupReq.m_amount );
		break;
	case XCMD_ItemDropReq: // Drop Item
		strcat(zTemp, "dropitem");
		if ( ! xCheckMsgSize( sizeof( pEvent->ItemDropReq )))
			RETURN_FALSE();
		Event_Item_Drop(pEvent);
		break;
	case XCMD_Click: // Singleclick
		strcat(zTemp, "singleclick");
		if ( ! xCheckMsgSize( sizeof( pEvent->Click )))
			RETURN_FALSE();
		Event_SingleClick( (DWORD) pEvent->Click.m_UID );
		break;
	case XCMD_ExtCmd: // Ext. Command
		strcat(zTemp, "extended command");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->ExtCmd.m_len ))
			RETURN_FALSE();
		Event_ExtCmd( (EXTCMD_TYPE) pEvent->ExtCmd.m_type, pEvent->ExtCmd.m_name );
		break;
	case XCMD_ItemEquipReq: // Equip Item
		strcat(zTemp, "equip item");
		if ( ! xCheckMsgSize( sizeof( pEvent->ItemEquipReq )))
			RETURN_FALSE();
		Event_Item_Equip(pEvent);
		break;
	case XCMD_WalkAck: // Resync Request
		strcat(zTemp, "resync request");
		if ( ! xCheckMsgSize( sizeof( pEvent->WalkAck )))
			RETURN_FALSE();
		addReSync();
		break;
	case XCMD_DeathMenu:	// DeathOpt (un)Manifest ghost (size anomoly)
		strcat(zTemp, "deathmenu");
		if ( ! xCheckMsgSize(2))
			RETURN_FALSE();
		if ( ! Event_DeathOption( (DEATH_MODE_TYPE) pEvent->DeathMenu.m_mode, pEvent ))
			RETURN_FALSE();
		break;
	case XCMD_CharStatReq: // Status Request
		strcat(zTemp, "status request");
		if ( ! xCheckMsgSize( sizeof( pEvent->CharStatReq )))
			RETURN_FALSE();
		if ( pEvent->CharStatReq.m_type == 4 )
		{
			addCharStatWindow( (DWORD) pEvent->CharStatReq.m_UID, true );
		}
		if ( pEvent->CharStatReq.m_type == 5 )
			addSkillWindow( SKILL_QTY );
		break;
	case XCMD_Skill:	// Skill locking.
		strcat(zTemp, "skill lock");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->Skill.m_len ))
			RETURN_FALSE();
		Event_Skill_Locks(pEvent);
		break;
	case XCMD_VendorBuy:	// Buy item from vendor.
		strcat(zTemp, "vendor buy");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->VendorBuy.m_len ))
			RETURN_FALSE();
		Event_VendorBuy( (DWORD) pEvent->VendorBuy.m_UIDVendor, pEvent );
		break;
	case XCMD_MapEdit:	// plot course on map.
		strcat(zTemp, "plot map course");
		if ( ! xCheckMsgSize( sizeof( pEvent->MapEdit )))
			RETURN_FALSE();
		Event_MapEdit( (DWORD) pEvent->MapEdit.m_UID, pEvent );
		break;
	case XCMD_BookPage: // Read/Change Book
		strcat(zTemp, "read/change book");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->BookPage.m_len ))
			RETURN_FALSE();
		Event_Book_Page( (DWORD) pEvent->BookPage.m_UID, pEvent );
		break;
	case XCMD_Options: // Options set
		strcat(zTemp, "set options");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->Options.m_len ))
			RETURN_FALSE();
		DEBUG_MSG(( "%x:XCMD_Options len=%d" DEBUG_CR, m_Socket.GetSocket(), pEvent->Options.m_len ));
		break;
	case XCMD_Target: // Targeting
		strcat(zTemp, "targeting");
		if ( ! xCheckMsgSize( sizeof( pEvent->Target )))
			RETURN_FALSE();
		Event_Target( pEvent );
		break;
	case XCMD_SecureTrade: // Secure trading
		strcat(zTemp, "secure trading");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->SecureTrade.m_len ))
			RETURN_FALSE();
		Event_SecureTrade( (DWORD) pEvent->SecureTrade.m_UID, pEvent );
		break;
	case XCMD_BBoard: // BBoard Request.
		strcat(zTemp, "bboard");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->BBoard.m_len ))
			RETURN_FALSE();
		Event_BBoardRequest( (DWORD) pEvent->BBoard.m_UID, pEvent );
		break;
	case XCMD_War: // Combat Mode
		strcat(zTemp, "combat mode");
		if ( ! xCheckMsgSize( sizeof( pEvent->War )))
			RETURN_FALSE();
		Event_CombatMode( pEvent->War.m_warmode );
		break;
	case XCMD_CharName: // Rename Character(pet)
		strcat(zTemp, "rename pet");
		if ( ! xCheckMsgSize( sizeof( pEvent->CharName )))
			RETURN_FALSE();
		Event_SetName( (DWORD) pEvent->CharName.m_UID, pEvent->CharName.m_charname );
		break;
	case XCMD_MenuChoice: // Menu Choice
		strcat(zTemp, "menu choice");
		if ( ! xCheckMsgSize( sizeof( pEvent->MenuChoice )))
			RETURN_FALSE();
		Event_MenuChoice(pEvent);
		break;
	case XCMD_BookOpen:	// Change a books title/author.
		strcat(zTemp, "open book");
		if ( m_Crypt.GetClientVer() > 0x126000 || IsNoCryptVer(1) )
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->BookOpen_v26 )))
				RETURN_FALSE();
			Event_Book_Title( (DWORD) pEvent->BookOpen_v26.m_UID, pEvent->BookOpen_v26.m_title, pEvent->BookOpen_v26.m_author );
		}
		else
		{
			if ( ! xCheckMsgSize( sizeof( pEvent->BookOpen_v25 )))
				RETURN_FALSE();
			Event_Book_Title( (DWORD) pEvent->BookOpen_v25.m_UID, pEvent->BookOpen_v25.m_title, pEvent->BookOpen_v25.m_author );
		}
		break;
	case XCMD_DyeVat: // Color Select Dialog
		strcat(zTemp, "color dialog");
		if ( ! xCheckMsgSize( sizeof( pEvent->DyeVat )))
			RETURN_FALSE();
		Event_Item_Dye( (DWORD) pEvent->DyeVat.m_UID, pEvent->DyeVat.m_wHue );
		break;
	case XCMD_Prompt: // Response to console prompt.
		strcat(zTemp, "console responce");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->Prompt.m_len ))
			RETURN_FALSE();
		Event_PromptResp( pEvent->Prompt.m_text, pEvent->Prompt.m_len-sizeof(pEvent->Prompt));
		break;
	case XCMD_HelpPage: // GM Page (i want to page a gm!)
		{
			strcat(zTemp, "gm help page");
			if ( ! xCheckMsgSize( sizeof( pEvent->HelpPage )))
				RETURN_FALSE();
			if ( m_pChar == NULL )
				return( false );
			CScript script( "HelpPage" );
			m_pChar->r_Verb( script, this );
			break;
		}
	case XCMD_VendorSell: // Vendor Sell
		strcat(zTemp, "vendor sell");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->VendorSell.m_len ))
			RETURN_FALSE();
		Event_VendorSell( (DWORD) pEvent->VendorSell.m_UIDVendor, pEvent );
		break;
	case XCMD_Scroll:	// Scroll Closed
		strcat(zTemp, "scroll");
		if ( ! xCheckMsgSize( sizeof( pEvent->Scroll )))
			RETURN_FALSE();
		Event_ScrollClose( (DWORD) pEvent->Scroll.m_context );
		break;
	case XCMD_GumpInpValRet:	// Gump text input
		strcat(zTemp, "gump text input");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->GumpInpValRet.m_len ))
			RETURN_FALSE();
		Event_GumpInpValRet(pEvent);
		break;
	case XCMD_TalkUNICODE:	// Talk unicode.
		strcat(zTemp, "unicode talk");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize(pEvent->TalkUNICODE.m_len ))
			RETURN_FALSE();
		SetPrivFlags( PRIV_T2A );
		Event_TalkUNICODE(pEvent);
		break;
	case XCMD_GumpDialogRet:	// Gump menu.
		strcat(zTemp, "dialog return");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->GumpDialogRet.m_len ))
			RETURN_FALSE();
		Event_GumpDialogRet(pEvent);
		break;

	case XCMD_ChatText:	// ChatText
		strcat(zTemp, "chat text");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->ChatText.m_len ))
			RETURN_FALSE();
		SetPrivFlags( PRIV_T2A );
		Event_ChatText( pEvent->ChatText.m_utext, pEvent->ChatText.m_len, CLanguageID( pEvent->ChatText.m_lang ));
		break;
	case XCMD_Chat: // Chat
		strcat(zTemp, "chatting");
		if ( ! xCheckMsgSize( sizeof( pEvent->Chat)))
			RETURN_FALSE();
		SetPrivFlags( PRIV_T2A );
		Event_ChatButton(pEvent->Chat.m_uname);
		break;
	case XCMD_ToolTipReq:	// Tool Tip
		strcat(zTemp, "tooltip");
		if ( ! xCheckMsgSize( sizeof( pEvent->ToolTipReq )))
			RETURN_FALSE();
		Event_ToolTip( (DWORD) pEvent->ToolTipReq.m_UID );
		break;
	case XCMD_CharProfile:	// Get Character Profile.
		strcat(zTemp, "profile");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->CharProfile.m_len ))
			RETURN_FALSE();
		Event_Profile( pEvent->CharProfile.m_WriteMode, (DWORD) pEvent->CharProfile.m_UID, pEvent );
		break;
	case XCMD_MailMsg:
		strcat(zTemp, "mailmsg");
		if ( ! xCheckMsgSize( sizeof(pEvent->MailMsg)))
			RETURN_FALSE();
		Event_MailMsg( (DWORD) pEvent->MailMsg.m_uid1, (DWORD) pEvent->MailMsg.m_uid2 );
		break;
	case XCMD_ClientVersion:	// Client Version string packet
		strcat(zTemp, "clientversion");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize(pEvent->ClientVersion.m_len))
			RETURN_FALSE();
		SetPrivFlags( PRIV_T2A );
		Event_ClientVersion( pEvent->ClientVersion.m_text, pEvent->ClientVersion.m_len );
		break;
	case XCMD_ExtData:	// Add someone to the party system.
		strcat(zTemp, "ext data (party?)");
		if ( ! xCheckMsgSize(3))
			RETURN_FALSE();
		if ( ! xCheckMsgSize( pEvent->ExtData.m_len ))
			RETURN_FALSE();
		Event_ExtData( (EXTDATA_TYPE)(WORD) pEvent->ExtData.m_type, &(pEvent->ExtData.m_u), pEvent->ExtData.m_len-5 );
		break;

	default:
		strcat(zTemp, "unknown");
		// clear socket I have no idea what this is.
		RETURN_FALSE();
	}

	// This is the last message we are pretty sure we got correctly.
	m_bin_PrvMsg = (XCMD_TYPE) pEvent->Default.m_Cmd;
	EXC_CATCH(zTemp);
	return( true );
}

void CClient::Event_AOSPopupMenu( DWORD uid, WORD EntryTag )
{
	CGrayUID uObj = uid;
	CExtData cmd;

#define MAX_POPUPS 15

#define POPUPFLAG_LOCKED 0x01
#define POPUPFLAG_ARROW 0x02
#define POPUPFLAG_COLOR 0x20

#define POPUP_REQUEST 100
#define POPUP_PAPERDOLL 101
#define POPUP_BACKPACK 102
#define POPUP_BANKBOX 201
#define POPUP_BANKBALANCE 202
#define POPUP_VENDORBUY 301
#define POPUP_VENDORSELL 302
#define POPUP_PETGUARD 401
#define POPUP_PETFOLLOW 402
#define POPUP_PETDROP 403
#define POPUP_PETKILL 404
#define POPUP_PETSTOP 405
#define POPUP_PETSTAY 406
#define POPUP_PETFRIEND 407
#define POPUP_PETTRANSFER 408
#define POPUP_STABLESTABLE 501
#define POPUP_STABLERETRIEVE 502

	struct
	{
		WORD m_EntryTag;
		WORD m_TextID; // This comes from cliloc, the 3006xxx range &~ 3000000
		WORD m_Flags;
		WORD m_Color;
	} Popups[MAX_POPUPS];

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int len = 7; // Size of the header

	if ( uObj.IsItem() )
		CItem *pItem = uObj.ItemFind();
	else if ( uObj.IsChar() )
		CChar *pChar = uObj.CharFind();
	else
		return;

	if ( ! CanSee( uObj.ObjFind() ) )
		return;

	if ( ! EntryTag )
	{
		cmd.Popup_Display.m_unk1 = 1;
		cmd.Popup_Display.m_UID = uid;
	}

	if ( uObj.IsChar() )
	{
		CChar * pChar = uObj.CharFind();

		if ( pChar->IsHuman() )
		{
			Popups[x].m_EntryTag = POPUP_PAPERDOLL;
			Popups[x].m_TextID = 6123; // Open Paperdoll 3006123
			Popups[x].m_Color = 0xFFFF;
			Popups[x++].m_Flags = POPUPFLAG_COLOR;
		}

		if ( pChar == m_pChar )
		{
			Popups[x].m_EntryTag = POPUP_BACKPACK;
			Popups[x].m_TextID = 6145; // Open Backpack 3006145
			Popups[x].m_Color = 0xFFFF;
			Popups[x++].m_Flags = POPUPFLAG_COLOR;
		}

		if ( pChar->m_pNPC )
		{
			switch ( pChar->m_pNPC->m_Brain )
			{
			case NPCBRAIN_BANKER:
				Popups[x].m_EntryTag = POPUP_BANKBOX;
				Popups[x].m_TextID = 6105; // Open Bankbox 3006105
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_BANKBALANCE;
				Popups[x].m_TextID = 6124; // Check Balance	3006124
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;
				break;

			case NPCBRAIN_STABLE:
				Popups[x].m_EntryTag = POPUP_STABLESTABLE;
				Popups[x].m_TextID = 6126; // Stable Pet 3006126
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_STABLERETRIEVE;
				Popups[x].m_TextID = 6127; // Claim All Pets 3006127
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;
			case NPCBRAIN_VENDOR:
				Popups[x].m_EntryTag = POPUP_VENDORBUY;
				Popups[x].m_TextID = 6103; // Buy 3006103
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_VENDORSELL;
				Popups[x].m_TextID = 6104; // Sell 3006104
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;
				break;
			}

			if ( pChar->NPC_IsOwnedBy( m_pChar, false ) )
			{
				Popups[x].m_EntryTag = POPUP_PETGUARD;
				Popups[x].m_TextID = 6107; // Command: Guard 3006107
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETFOLLOW;
				Popups[x].m_TextID = 6108; // Command: Follow 3006108
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETDROP;
				Popups[x].m_TextID = 6109; // Command: Drop 3006109
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETKILL;
				Popups[x].m_TextID = 6111; // Command: Kill 3006111
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETSTOP;
				Popups[x].m_TextID = 6112; // Command: Stop 3006112
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETSTAY;
				Popups[x].m_TextID = 6114; // Command: Stay 3006114
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETFRIEND;
				Popups[x].m_TextID = 6110; // Add Friend 3006110
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				Popups[x].m_EntryTag = POPUP_PETTRANSFER;
				Popups[x].m_TextID = 6113; // Transfer 3006113
				Popups[x].m_Color = 0xFFFF;
				Popups[x++].m_Flags = POPUPFLAG_COLOR;

				switch ( EntryTag )
				{
				case POPUP_PETGUARD:
					pChar->NPC_OnHearPetCmd( "guard", m_pChar, false );
					break;

				case POPUP_PETFOLLOW:
					pChar->NPC_OnHearPetCmd( "follow", m_pChar, false );
					break;

				case POPUP_PETDROP:
					pChar->NPC_OnHearPetCmd( "drop", m_pChar, false );
					break;

				case POPUP_PETKILL:
					pChar->NPC_OnHearPetCmd( "kill", m_pChar, false );
					break;

				case POPUP_PETSTOP:
					pChar->NPC_OnHearPetCmd( "stop", m_pChar, false );
					break;

				case POPUP_PETSTAY:
					pChar->NPC_OnHearPetCmd( "stay", m_pChar, false );
					break;

				case POPUP_PETFRIEND:
					pChar->NPC_OnHearPetCmd( "friend", m_pChar, false );
					break;

				case POPUP_PETTRANSFER:
					pChar->NPC_OnHearPetCmd( "transfer", m_pChar, false );
					break;
				}
			}
		}

		switch ( EntryTag )
		{
		case POPUP_PAPERDOLL:
			m_pChar->Use_Obj( (CObjBase *) pChar, false, false );
			break;

		case POPUP_BACKPACK:
			m_pChar->Use_Obj( (CObjBase *)m_pChar->LayerFind( LAYER_PACK ), false, false );
			break;

		case POPUP_BANKBOX:
			if ( pChar->m_pNPC->m_Brain == NPCBRAIN_BANKER )
				addBankOpen( m_pChar );
			break;

		case POPUP_BANKBALANCE:
			if ( pChar->m_pNPC->m_Brain == NPCBRAIN_BANKER )
				SysMessagef( "You have %d gold piece(s) in your bankbox", m_pChar->GetBank()->ContentCount( RESOURCE_ID(RES_TYPEDEF,IT_GOLD) ) );
			break;

		case POPUP_VENDORBUY:
			if ( pChar->NPC_IsVendor() )
				addShopMenuBuy( pChar );
			break;

		case POPUP_VENDORSELL:
			if ( pChar->NPC_IsVendor() )
				addShopMenuSell( pChar );
			break;

		case POPUP_STABLESTABLE:
			if ( pChar->m_pNPC->m_Brain == NPCBRAIN_STABLE )
				pChar->NPC_OnHear( "stable", m_pChar );
			break;

		case POPUP_STABLERETRIEVE:
			if ( pChar->m_pNPC->m_Brain == NPCBRAIN_STABLE )
				pChar->NPC_OnHear( "retrieve", m_pChar );
			break;
		}
	}

	if ( ( ! EntryTag ) && ( x ) )
	{
		CExtData * pCur = &cmd;

		cmd.Popup_Display.m_NumPopups = x;
		for ( y = 0; Popups[y].m_EntryTag; y++ )
		{
			// Construct the packet

			pCur->Popup_Display.m_List[0].m_EntryTag = Popups[y].m_EntryTag;
			pCur->Popup_Display.m_List[0].m_TextID = Popups[y].m_TextID;
			pCur->Popup_Display.m_List[0].m_Flags = Popups[y].m_Flags;
			pCur->Popup_Display.m_List[0].m_Color = Popups[y].m_Color;

			len += sizeof( pCur->Popup_Display.m_List[0] );
			pCur = (CExtData *)( ((BYTE*) pCur ) + sizeof( pCur->Popup_Display.m_List[0] ));
		}

		addExtData( EXTDATA_Popup_Display, &cmd, len );
	}
}
