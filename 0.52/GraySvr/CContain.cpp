//
// CContain.CPP
//
#include "graysvr.h"	// predef header.

//***************************************************************************
// -CContainer

void CContainer::OnWeightChange( int iChange )
{
	// Propagate the weight change up the stack if there is one.
	m_totalweight += iChange;
	DEBUG_CHECK( m_totalweight >= 0 );
}

int CContainer::FixWeight()
{
	// If there is some sort of ASSERT during item add then this is used to fix it.
#if 0
	m_totalweight = 0;

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		CItemContainer * pCont = dynamic_cast <CItemContainer *> (pItem);
		if ( pCont )
		{
			pCont->FixWeight();
			if ( ! pCont->IsWeighed()) 
				continue;	// Bank box doesn't count for wieght.
		}
		m_totalweight += pItem->GetWeight();
	}
#endif

	return( m_totalweight );
}

void CContainer::ContentAddPrivate( CItem * pItem )
{
	// We are adding to a CChar or a CItemContainer
	ASSERT( pItem != NULL );
	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_CHECK( GetCount() <= MAX_ITEMS_CONT );
	}
	CGObList::InsertAfter( pItem );
	OnWeightChange( pItem->GetWeight());
}

void CContainer::OnRemoveOb( CGObListRec* pObRec )	// Override this = called when removed from list.
{
	// remove this object from the container list.
	// Overload the RemoveAt for general lists to come here.
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	ASSERT(pItem);
	CGObList::OnRemoveOb( pItem );
	ASSERT( pItem->GetParent() == NULL );
	pItem->SetContainerFlags(UID_SPEC);	// It is no place for the moment.
	OnWeightChange( -pItem->GetWeight());
}

CItem * CContainer::ContentFindType( ITEM_TYPE iType, DWORD dwArg, int iDecendLevels ) const
{
	// send all the items in the container.
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->m_type == iType )
		{
			if ( pItem->GetAmount() == 0 )
				continue;	// does not count.
			if ( iType == ITEM_MAP )
			{
				if ( ! dwArg )	// look for blank map.
				{
					if ( ! pItem->m_itMap.m_left && ! pItem->m_itMap.m_right )
						break;
				}
				else if ( LOWORD(dwArg) == pItem->m_itMap.m_top && HIWORD(dwArg) == pItem->m_itMap.m_left )
				{
					break;
				}
				continue;
			}
			else if ( iType == ITEM_KEY )
			{
				if ( pItem->IsKeyLockFit( dwArg ))
					break;
				continue;
			}
			break;
		}
		CItemContainer * pCont = dynamic_cast <CItemContainer*>(pItem);
		if ( pCont != NULL && iDecendLevels > 0 )
		{
			if ( ! pCont->IsSearchable())
				continue;
			CItem * pItemInCont = pCont->ContentFindType( iType, dwArg, iDecendLevels-1 );
			if ( pItemInCont )
				return( pItemInCont );
		}
	}
	return( pItem );
}

bool CContainer::ContentFindKeyFor( CItem * pLocked ) const
{
	return( ContentFindType( ITEM_KEY, pLocked->m_itContainer.m_lockUID ) != NULL );
}

CItem * CContainer::ContentFind( ITEMID_TYPE id, int iDecendLevels ) const
{
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->GetID() == id )
			break;
		CItemContainer * pCont = dynamic_cast <CItemContainer*>(pItem);
		if ( pCont != NULL && iDecendLevels > 0 ) // this is a sub-container.
		{
			if ( ! pCont->IsSearchable())
				continue;
			CItem * pItemInCont = pCont->ContentFind(id,iDecendLevels-1);
			if ( pItemInCont != NULL )
				return( pItemInCont );
		}
	}
	return( pItem );
}

CItem* CContainer::ContentFindRandom( void ) const
{
	// returns Pointer of random item, NULL if player carrying none
	return( dynamic_cast <CItem*> ( GetAt( GetRandVal( GetCount()))));
}

void CContainer::WriteContent( CScript & s ) const
{
	const CGObList * pListThis = dynamic_cast <const CGObList *>(this);
	ASSERT(pListThis);

	// Write out all the items in me.
	CItem* pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		ASSERT( pItem->GetParent() == pListThis );
		pItem->WriteTry(s);
	}
}

int CContainer::ContentConsume( ITEMID_TYPE id, int amount, bool fTest, COLOR_TYPE wColor )
{
	// RETURN:
	// 0 = all consumed ok. else number left to be consumed. (still required)

	if ( id == ITEMID_NOTHING )
		return( amount );	// from skills menus.

	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();

		// ITEM_INGOT
		if ( id == ITEMID_INGOT_IRON && pItem->GetDispID() == ITEMID_INGOT_IRON )
		{
			if ( pItem->GetColor() == wColor )
				goto gotit;
			continue;
		}
		// ITEM_CLOTH
		if ( id == ITEMID_CLOTH1 && pItem->m_type == ITEM_CLOTH )
		{
			pItem->ConvertBolttoCloth();
			goto gotit;
		}
		// ITEM_LEATHER
		if ( id == ITEMID_HIDES &&
			( ( pItem->GetID() == ITEMID_HIDES ) ||
			  ( pItem->GetID() == ITEMID_HIDES_2 ) ||
			  ( pItem->GetID() == ITEMID_LEATHER_1 ) ||
			  ( pItem->GetID() == ITEMID_LEATHER_2 )))
		{
			// We should be able to use leather in place of hides.
			goto gotit;
		}
		// ITEM_LOG
		if ( id == ITEMID_LOG_1 && pItem->m_type == ITEM_LOG )
		{
			goto gotit;
		}
		if ( pItem->GetDispID() == id )
		{
			// It at least looks like what we want.
			if ( id == ITEMID_Arrow || id == ITEMID_XBolt )
				goto gotit;
		}
		if ( pItem->GetID() == id )
		{
gotit:
			if ( pItem->GetAmount() > amount )
			{
				// subtract from it's count = part of stack.
				if ( ! fTest )
					pItem->SetAmountUpdate( pItem->GetAmount() - amount );
				return( 0 );
			}
			else
			{
				// Use whole stack.
				amount -= pItem->GetAmount();
				if ( ! fTest )
				{
					pItem->Delete();
					if ( amount <= 0 ) 
						break;
					continue;
				}
			}
		}
		CItemContainer * pCont = dynamic_cast <CItemContainer*> (pItem);
		if ( pCont != NULL )	// this is a sub-container.
		{
			if ( id != ITEMID_GOLD_C1 )
			{
				if ( pCont->GetID() == ITEMID_BANK_BOX )
					continue;
				if ( pCont->GetID() == ITEMID_VENDOR_BOX )
					continue;
			}
			if ( pCont->m_type == ITEM_CONTAINER_LOCKED ) 
				continue;
			amount = pCont->ContentConsume( id, amount, fTest, wColor );
			if ( amount <= 0 )
				break;
		}
	}
	return( amount );
}

int CContainer::ContentCount( ITEMID_TYPE id )
{
	// Calculate total (gold or other items) in this recursed container
	return( INT_MAX - ContentConsume( id, INT_MAX, true ));
}

void CContainer::ContentsDump( const CPointMap & pt )
{
	// Just dump the contents onto the ground.
	DEBUG_CHECK( pt.IsValid());
	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( pItem->IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2)) 
			continue;	// hair and newbie stuff.
		// ??? scatter a little ?
		pItem->MoveToDecay( pt );
	}
}

void CContainer::ContentsTransfer( CItemContainer * pCont, bool fNoNewbie )
{
	// Move all contents to another container. (pCont)
	if ( pCont == NULL ) 
		return;

	CItem* pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( fNoNewbie && pItem->IsAttr( ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2 ))	// keep newbie stuff.
			continue;
		pCont->ContentAdd( pItem );	// add content
	}
}

int CContainer::ResourceConsume( TCHAR * pResource, int amount, bool fTest, COLOR_TYPE wColor )
{
	// RETURN: how many whole objects can be made.
	if ( amount <= 0 ) 
		amount = 1;
	if ( ! fTest && amount > 1 )
	{
		// Test what the max number we can really make is !
		// All resources must be consumed with the same number.
		amount = ResourceConsume( pResource, amount, true, wColor );
	}

	int iQtyMin = INT_MAX;
	while (true)
	{
		int iQty = 0;
		ITEMID_TYPE id = GetResourceEntry( pResource, iQty );
		if ( id == ITEMID_NOTHING )
			break;
		int iQtyMax = ( iQty * amount );
		int iQtyCur = iQtyMax - ContentConsume( id, iQtyMax, fTest, wColor );
		iQtyCur /= iQty;
		if ( iQtyCur < iQtyMin ) 
			iQtyMin = iQtyCur;
	}

	if ( iQtyMin == INT_MAX )
		return( 0 );
	return( iQtyMin );
}

int CContainer::ContentCountAll() const
{
	// RETURN:
	//  A count of all the items in this container and sub contianers.
	int iTotal = 0;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		iTotal++;
		const CItemContainer * pCont = dynamic_cast <const CItemContainer*>(pItem);
		if ( pCont == NULL ) 
			continue;
		// if ( ! pCont->IsSearchable()) continue; // found a sub
		iTotal += pCont->ContentCountAll();
	}
	return( iTotal );
}

//----------------------------------------------------
// -CItemContainer

CItemContainer::CItemContainer( ITEMID_TYPE id, CItemBase * pDef ) :
	CItemVendable( id, pDef )
{
	// DEBUG_CHECK( CItemBase::IsContainerID( id ));
	// m_fTinkerTrapped = false;
}

bool CItemContainer::r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc )
{
	if ( ! strnicmp( pszKey, "FINDID", 6 ))
	{
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFind( (ITEMID_TYPE) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDCONT", 8 ))
	{
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		pRef = GetAt( Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	return( CItem::r_GetRef( pszKey, pRef, pSrc ));
}

bool CItemContainer::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc )
{
	if ( ! strnicmp( pKey, "COUNT", 5 ))
	{
		if ( ! pKey[5] )
		{
			sVal.FormatVal( GetCount());	// count all.
		}
		else
		{
			pKey += 6;
			sVal.FormatVal( ContentCount( (ITEMID_TYPE) Exp_GetVal( pKey )));
		}
		return( true );
	}
	if ( ! strnicmp( pKey, "RESTEST", 7 ))
	{
		pKey += 7;
		sVal.FormatVal( ResourceConsume( (TCHAR*) pKey, 1, true ));
		return( true );
	}
	return( CItem::r_WriteVal( pKey, sVal, pSrc ));
}

void CItemContainer::Trade_Status( bool fCheck )
{
	// Update trade status check boxes to both sides.
	CItemContainer * pPartner = dynamic_cast <CItemContainer *> ( m_uidLink.ItemFind());
	if ( pPartner == NULL )
		return;

	CChar * pChar1 = dynamic_cast <CChar*> (GetParent());
	if ( pChar1 == NULL ) 
		return;
	CChar * pChar2 = dynamic_cast <CChar*> (pPartner->GetParent());
	if ( pChar2 == NULL ) 
		return;

	m_itEqTradeWindow.m_fCheck = fCheck;
	if ( ! fCheck )
	{
		pPartner->m_itEqTradeWindow.m_fCheck = false;
	}

	CCommand cmd;
	cmd.SecureTrade.m_Cmd = XCMD_SecureTrade;
	cmd.SecureTrade.m_len = 0x11;
	cmd.SecureTrade.m_action = 0x02;	// status
	cmd.SecureTrade.m_fname = 0;

	if ( pChar1->IsClient())
	{
		cmd.SecureTrade.m_UID = GetUID();
		cmd.SecureTrade.m_UID1 = m_itEqTradeWindow.m_fCheck;
		cmd.SecureTrade.m_UID2 = pPartner->m_itEqTradeWindow.m_fCheck;
		pChar1->m_pClient->xSendPkt( &cmd, 0x11 );
	}
	if ( pChar2->IsClient())
	{
		cmd.SecureTrade.m_UID = pPartner->GetUID();
		cmd.SecureTrade.m_UID1 = pPartner->m_itEqTradeWindow.m_fCheck;
		cmd.SecureTrade.m_UID2 = m_itEqTradeWindow.m_fCheck;
		pChar2->m_pClient->xSendPkt( &cmd, 0x11 );
	}

	// if both checked then done.
	if ( ! pPartner->m_itEqTradeWindow.m_fCheck || ! m_itEqTradeWindow.m_fCheck )
		return;

	CItem * pItemNext;
	CItem* pItem = GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		pChar2->ItemBounce( pItem );
	}

	pItem = pPartner->GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		pChar1->ItemBounce( pItem );
	}

	// done with trade.
	Delete();
}

void CItemContainer::Trade_Delete()
{
	// Called when object deleted.

	ASSERT( m_type == ITEM_EQ_TRADE_WINDOW );

	CChar * pChar = dynamic_cast <CChar*> (GetParent());
	if ( pChar == NULL ) 
		return;

	if ( pChar->IsClient())
	{
		// Send the cancel trade message.
		CCommand cmd;
		cmd.SecureTrade.m_Cmd = XCMD_SecureTrade;
		cmd.SecureTrade.m_len = 0x11;
		cmd.SecureTrade.m_action = 0x01;
		cmd.SecureTrade.m_UID = GetUID();
		cmd.SecureTrade.m_UID1 = 0;
		cmd.SecureTrade.m_UID2 = 0;
		cmd.SecureTrade.m_fname = 0;
		pChar->m_pClient->xSendPkt( &cmd, 0x11 );
	}

	// Drop items back in my pack.
	CItem * pItemNext;
	for ( CItem* pItem = GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		pChar->ItemBounce( pItem );
	}

	// Kill my trading partner.
	CItemContainer * pPartner = dynamic_cast <CItemContainer *> ( m_uidLink.ItemFind());
	if ( pPartner == NULL ) 
		return;

	m_uidLink.ClearUID();	// unlink.
	pPartner->m_uidLink.ClearUID();
	pPartner->Delete();
}

void CItemContainer::OnWeightChange( int iChange )
{
	CContainer::OnWeightChange( iChange );
	if ( iChange == 0 ) 
		return;	// no change

	// some containers do not add weight to you.
	if ( ! IsWeighed())
		return;

	// Propagate the weight change up the stack if there is one.
	CContainer * pCont = dynamic_cast <CContainer*>(GetParent());
	if ( pCont == NULL ) 
		return;	// on ground.
	pCont->OnWeightChange( iChange );
}

CPointMap CItemContainer::GetRandContainerLoc() const
{
	// Max/Min Container Sizes.

	static const struct // we can probably get this from MUL file some place.
	{
		GUMP_TYPE m_gump;
		WORD m_minx;
		WORD m_miny;
		WORD m_maxx;
		WORD m_maxy;
	} ContSize[] =
	{
		{ GUMP_RESERVED, 40, 50, 100, 100 },		// default.
		{ GUMP_SECURE_TRADE, 1, 1, 66, 26 },
		{ GUMP_CORPSE, 20, 85, 80, 185 },
		{ GUMP_PACK, 44, 65, 142, 150 },	// Open backpack
		{ GUMP_BAG, 29, 34, 93, 119 },		// Leather Bag
		{ GUMP_CHEST_GO_SI, 18, 105, 118, 169 },	// Gold and Silver Chest.
		{ GUMP_CHEST_WO_GO, 18, 105, 118, 169 },	// Wood with gold trim.
		{ GUMP_CHEST_SI, 18, 105, 118, 169 },	// silver chest.
		{ GUMP_BARREL, 33, 36, 98, 139 },		// Barrel
		{ GUMP_CRATE, 20, 10, 126, 91 },	// Wood Crate
		{ GUMP_BASKET_SQ, 19, 47, 138, 114 },	// Square picknick Basket
		{ GUMP_BASKET_RO, 33, 36,  98, 134 }, // Round Basket
		{ GUMP_CABINET_DK, 24, 96, 91, 143 },
		{ GUMP_CABINET_LT, 24, 96, 91, 143 },
		{ GUMP_BOOK_SHELF, 76, 12, 96, 59 },
		{ GUMP_BOX_WOOD, 16, 51, 140, 115 },	// small wood box with a lock
		{ GUMP_BOX_WOOD_OR, 16, 51, 140, 115 }, // Small wood box (ornate)(no lock)
		{ GUMP_BOX_GO_LO, 16, 51, 140, 115 }, // Gold/Brass box with a lock.
		{ GUMP_DRAWER_DK, 16, 17, 110, 85 },
		{ GUMP_DRAWER_LT, 16, 17, 110, 85 },
		{ GUMP_SHIP_HOLD, 46, 74, 152, 175 },

		{ GUMP_GAME_BOARD,	 4, 10, 220, 185 },	// Chess or checker board.
		{ GUMP_GAME_BACKGAM, 4, 10, 220, 185 },
	};

	// Get a random location in the container.
	GUMP_TYPE gump = CItemBase::IsContainerID( GetDispID());
	int i=0;
	for ( ; true; i++ )
	{
		if (i>=COUNTOF(ContSize))
		{
			i=0;
			DEBUG_ERR(( "unknown container gump id %d for 0%x\n", gump, GetDispID()));
			break;
		}
		if ( ContSize[i].m_gump == gump ) 
			break;
	}

	return( CPointMap(
		ContSize[i].m_minx + GetRandVal( ContSize[i].m_maxx - ContSize[i].m_minx ),
		ContSize[i].m_miny + GetRandVal( ContSize[i].m_maxy - ContSize[i].m_miny ),
		0 ));
}

void CItemContainer::ContentAdd( CItem * pItem, CPointMap pt )
{
	// Add to CItemContainer
	if ( pItem == NULL )
		return;
	if ( pItem == this )
		return;	// infinite loop.

	if ( ! g_Serv.IsLoading())
	{
		if ( m_type == ITEM_EQ_TRADE_WINDOW )
		{
			// Drop into a trade window.
			Trade_Status( false );
		}
		else if ( pItem->m_type == ITEM_LIGHT_LIT )
		{
			// Douse the light in a pack ! (if possible)
			pItem->Use_Light();
		}
		else if ( pItem->m_type == ITEM_GAME_BOARD )
		{
			// Can't be put into any sort of a container.
			// delete all it's pieces.
			(dynamic_cast <CItemContainer*> (pItem))->DeleteAll();
		}
	}

	if ( pt.m_x <= 0 || pt.m_y <= 0 ||
		pt.m_x > 512 || pt.m_y > 512 )	// invalid container location ?
	{
		// Try to stack it.
		if ( ! g_Serv.IsLoading() && pItem->m_pDef->IsStackableType())
		{
			CItem * pItemNext;
			for ( CItem* pTry=GetContentHead(); pTry!=NULL; pTry=pItemNext)
			{
				pItemNext = pTry->GetNext();
				pt = pTry->GetContainedPoint();
				if ( pItem->Stack( pTry ))
					goto insertit;
			}
		}
		pt = GetRandContainerLoc();
	}

insertit:
	CContainer::ContentAddPrivate( pItem );
	pItem->SetContainedPoint( pt );

#ifdef _DEBUG
	if ( ! g_Serv.IsLoading())
	{
		ASSERT( pItem->GetID() != ITEMID_MEMORY );
		ASSERT( pItem->m_type != ITEM_EQ_MEMORY_OBJ );
	}
#endif

	switch ( GetID())
	{
	case ITEMID_KEY_RING0: // empty key ring.
	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
		SetKeyRing();
		break;
	case ITEMID_VENDOR_BOX:
		if ( IsEquipped())	// vendor boxes should ALWAYS be equipped !
		{
			pItem->m_Attr |= ATTR_FORSALE;
			pItem->SetContainedLayer( pItem->GetAmount());
			pItem->SetPlayerVendorPrice( 0 );	// unpriced yet.
		}
		break;
	case ITEMID_BEDROLL_O_EW:
	case ITEMID_BEDROLL_O_NS:
		// Close the bedroll
		pItem->SetDispID( ITEMID_BEDROLL_C );
		break;
	}

	pItem->Update();
}

void CItemContainer::ContentAdd( CItem * pItem )
{
	if ( pItem == NULL )
		return;
	if ( pItem->GetParent() == this )
		return;	// already here.
	CPointMap pt;	// invalid point.
	if ( g_Serv.IsLoading())
	{
		pt = pItem->GetUnkPoint();
	}
	ContentAdd( pItem, pt );
}

bool CItemContainer::IsItemInContainer(const CItem * pItem) const
{
	// Checks if a particular item is in a container or one of
	// it's subcontainers.

	while (true)
	{
		if ( pItem == NULL )
			return( false );
		if ( pItem == this )
			return( true );
		pItem = dynamic_cast <CItemContainer *>(pItem->GetParent());
	}
}

void CItemContainer::OnRemoveOb( CGObListRec* pObRec )	// Override this = called when removed from list.
{
	// remove this object from the container list.
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	ASSERT(pItem);
	if ( m_type == ITEM_EQ_TRADE_WINDOW )
	{
		// Remove from a trade window.
		Trade_Status( false );
	}
	if ( GetID() == ITEMID_VENDOR_BOX && IsEquipped())	// vendor boxes should ALWAYS be equipped !
	{
		pItem->m_Attr &= ~ATTR_FORSALE;
		pItem->SetPlayerVendorPrice( 0 );	// the timer can be reused..
	}

	CContainer::OnRemoveOb(pObRec);
	switch ( GetID())
	{
	case ITEMID_KEY_RING0: // empty key ring.
	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
		SetKeyRing();
		break;
	}
}

void CItemContainer::DupeCopy( const CItem * pItem )
{
	// Copy the contents of this item.

	CItemVendable::DupeCopy( pItem );

	const CItemContainer * pContItem = dynamic_cast <const CItemContainer *>(pItem);
	DEBUG_CHECK(pContItem);
	if ( pContItem == NULL )
		return;

	for ( CItem * pContent = pContItem->GetContentHead(); pContent != NULL; pContent = pContent->GetNext())
	{
		ContentAdd( CreateDupeItem( pContent ), pContent->GetContainedPoint());
	}
}

void CItemContainer::MakeKey()
{
	m_type = ITEM_CONTAINER;	
	m_itContainer.m_lockUID = GetUID();
	m_itContainer.m_lock_complexity = 500 + GetRandVal( 600 );

	CItem * pKey = CreateScript( ITEMID_KEY_COPPER );
	pKey->m_itKey.m_lockUID = GetUID();
	ContentAdd( pKey );
}

void CItemContainer::SetKeyRing()
{
	// Look of a key ring depends on how many keys it has in it.
	static const WORD Item_Keyrings[] =
	{
		ITEMID_KEY_RING0, // empty key ring.
		ITEMID_KEY_RING1,
		ITEMID_KEY_RING1,
		ITEMID_KEY_RING3,
		ITEMID_KEY_RING3,
		ITEMID_KEY_RING5,
	};
	WORD id = GetCount();
	id = ( id >= COUNTOF(Item_Keyrings)) ? ITEMID_KEY_RING5 : Item_Keyrings[id];
	if ( id != GetID())
	{
		SetDispID((ITEMID_TYPE)id);
		Update();
	}
}

bool CItemContainer::CanContainerHold( const CItem * pItem, const CChar * pCharMsg ) const
{
	if ( pCharMsg == NULL )
		return( false );
	if ( pCharMsg->IsPriv(PRIV_GM))	// a gm can doing anything.
		return( true );

	if ( GetID() == ITEMID_BANK_BOX )
	{
		// Check if the bankbox will allow this item to be dropped into it.
		// Too many items or too much weight?

		// Check if the item dropped in the bank is a container. If it is
		// we need to calculate the number of items in that too.
		int iItemsInContainer = 0;
		const CItemContainer * pContItem = dynamic_cast <const CItemContainer *>( pItem );
		if ( pContItem )
		{
			iItemsInContainer = pContItem->ContentCountAll();
		}

		// Check the total number of items in the bankbox and the ev.
		// container put into it.
		if (( ContentCountAll() + iItemsInContainer ) > g_Serv.m_iBankIMax )
		{
			pCharMsg->SysMessage( "Your bankbox can't hold more items." );
			return( false );
		}

		// Check the weightlimit on bankboxes.
		if (( GetWeight() + pItem->GetWeight()) > g_Serv.m_iBankWMax )
		{
			pCharMsg->SysMessage( "Your bankbox can't hold more weight." );
			return( false );
		}
	}

	if ( IsAttr(ATTR_MAGIC))
	{
		// Put stuff in a magic box
		pCharMsg->SysMessage( "The item bounces out of the magic container" );
		return( false );
	}

	if ( GetCount() >= MAX_ITEMS_CONT-1 )
	{
		pCharMsg->SysMessage( "Too many items in that container" );
		return( false );
	}

	if ( ! IsEquipped() &&	// does not apply to my pack.
		pItem->IsContainer() &&
		pItem->m_pDef->GetVolume() >= m_pDef->GetVolume())
	{
		// is the container too small ? can't put barrels in barrels.
		pCharMsg->SysMessage( "The container is too small for that" );
		return( false );
	}

	if ( m_type == ITEM_GAME_BOARD )
	{
		if ( pItem->m_type != ITEM_GAME_PIECE )
		{
			pCharMsg->SysMessage( "That is not a game piece" );
			return( false );
		}
	}
	
	switch ( GetID())
	{
	case ITEMID_Bulletin1:
	case ITEMID_Bulletin2:	// This is a trade window or a bboard.
		if ( m_type == ITEM_EQ_TRADE_WINDOW ) 
			break;
		// BBoards are containers but you can't put stuff in them this way.
		return( false );

	case ITEMID_KEY_RING0: // empty key ring.
	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
		if ( pItem->m_type != ITEM_KEY )
		{
			pCharMsg->SysMessage( "This is not a key." );
			return( false );
		}
		if ( ! pItem->m_itKey.m_lockUID )
		{
			pCharMsg->SysMessage( "You can't put empty keys on a keyring." );
			return( false );
		}
		break;

	case ITEMID_VENDOR_BOX:
		if ( pItem->IsTimerSet() && ! pItem->IsAttr(ATTR_DECAY))
		{
			pCharMsg->SysMessage( "That's not a saleable item." );
			return( false );
		}
		break;
	}

	return( true );
}

void CItemContainer::Restock()
{
	// Check for vendor type containers.

	if ( IsAttr(ATTR_MAGIC))
	{
		// Magic restocking container.
	}

	if ( IsEquipped())
	{
		// Part of a vendor.
		CChar * pChar = dynamic_cast <CChar*> (GetParent());
		if ( ! pChar->IsStat( STATF_Pet ))	// Not on a hired vendor.
		switch ( GetEquipLayer())
		{
		case LAYER_VENDOR_STOCK:
			// Magic restock the vendors container.
			{
				CItem* pItemNext;
				CItem* pItem = GetContentHead();
				for ( ; pItem!=NULL; pItem=pItemNext)
				{
					pItemNext = pItem->GetNext();
					if ( ! pItem->GetContainedLayer())
						pItem->SetContainedLayer(1);
					CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
					if ( pVendItem != NULL )
						pVendItem->Restock();
					if ( pItem->GetAmount() >= pItem->GetContainedLayer())
						continue;
					pItem->SetAmount( pItem->GetContainedLayer());	// restock amount
				}
			}
			break;

		case LAYER_VENDOR_EXTRA:
			// clear all this junk periodicallly.
			// sell it back for cash value ?
			DeleteAll();
			break;

		case LAYER_VENDOR_BUYS:
			{
				// Reset what we will buy from players.
				CItem* pItem= GetContentHead();
				for ( ; pItem!=NULL; pItem = pItem->GetNext())
				{
					CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
					if ( pVendItem != NULL )
						pVendItem->Restock();
					pItem->SetContainedLayer(0);
				}
			}
			break;

		case LAYER_BANKBOX:
			// Restock petty cash.
			if ( ! m_itEqBankBox.m_Check_Restock )
				m_itEqBankBox.m_Check_Restock = 10000;
			if ( m_itEqBankBox.m_Check_Amount < m_itEqBankBox.m_Check_Restock )
				m_itEqBankBox.m_Check_Amount = m_itEqBankBox.m_Check_Restock;
			return;
		}
	}

	SetTimeout( GetRestockTimeSeconds() * TICK_PER_SEC );
}

void CItemContainer::Game_Create()
{
	DEBUG_CHECK( GetDispID() == ITEMID_GAME_BOARD ||
		GetDispID() == ITEMID_GAME_BACKGAM ||
		GetDispID() == ITEMID_GAME_BACKGAM_2 );

	ASSERT( m_type == ITEM_GAME_BOARD );
	if ( GetCount())
		return;	// already here.

	static const ITEMID_TYPE Item_ChessPieces[] =
	{
	ITEMID_GAME1_ROOK,		// 42,4
	ITEMID_GAME1_KNIGHT,
	ITEMID_GAME1_BISHOP,
	ITEMID_GAME1_QUEEN,
	ITEMID_GAME1_KING,
	ITEMID_GAME1_BISHOP,
	ITEMID_GAME1_KNIGHT,
	ITEMID_GAME1_ROOK,		// 218,4

	ITEMID_GAME1_PAWN,		// 42,41
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,
	ITEMID_GAME1_PAWN,		// 218, 41

	ITEMID_GAME2_PAWN,		// 44, 167
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,
	ITEMID_GAME2_PAWN,		// 218, 164

	ITEMID_GAME2_ROOK,		// 42, 185
	ITEMID_GAME2_KNIGHT,
	ITEMID_GAME2_BISHOP,
	ITEMID_GAME2_QUEEN,
	ITEMID_GAME2_KING,
	ITEMID_GAME2_BISHOP,
	ITEMID_GAME2_KNIGHT,
	ITEMID_GAME2_ROOK,		// 218, 183
	};

	CPointMap pt;
	bool fChess = ( GetDispID() == ITEMID_GAME_BOARD && m_itGameBoard.m_GameType == 0 );

	static const WORD ChessRow[] =
	{
		5,
		40,
		160,
		184,
	};
	static const WORD CheckerRow[] =
	{
		30,
		55,
		178,
		205,
	};

	for ( int i=0; i<COUNTOF(Item_ChessPieces); i++ )
	{
		// Add all it's pieces. (if not already added)
		CItem * pPiece = CItem::CreateBase( (fChess ) ? Item_ChessPieces[i] :
			(( i >= (2*8)) ? ITEMID_GAME1_CHECKER : ITEMID_GAME2_CHECKER ));
		if ( pPiece == NULL ) break;
		pPiece->m_type = ITEM_GAME_PIECE;
		if ( (i&7) == 0 )
		{
			pt.m_x = 42;
			pt.m_y = (fChess)?ChessRow[ i/8 ]:CheckerRow[ i/8 ];
		}
		else
		{
			pt.m_x += 25;
		}
		ContentAdd( pPiece, pt );
	}
}

bool CItemContainer::r_Verb( CScript &s, CTextConsole * pSrc )
{
	if ( s.IsKey( "EMPTY" ))
	{
		DeleteAll();
		return( true );
	}
	if ( s.IsKey( "DELETE" ) && s.HasArgs())
	{
		// 1 based pages.
		int index = s.GetArgVal();
		if ( index <= GetCount())
		{
			delete GetAt( index-1 );
			return( true );
		}
	}
	return( CItem::r_Verb( s, pSrc ));
}

bool CItemContainer::OnTick()
{
	if ( IsAttr( ATTR_MAGIC ) && m_type == ITEM_CONTAINER )
	{
		// Magic restocking container.
		Restock();
		return true;
	}
	switch ( GetID())
	{
	case ITEMID_BANK_BOX:
	case ITEMID_VENDOR_BOX:
		// Restock this box.
		// Magic restocking container.
		Restock();
		return true;
	}
	return CItem::OnTick();
}

CChar * CItemCorpse::IsCorpseSleeping() const
{
	// Is this corpse really a sleeping person ?
	// CItemCorpse
	ASSERT( GetID() == ITEMID_CORPSE );

	CChar * pCharCorpse = m_uidLink.CharFind();
	if ( pCharCorpse &&
		pCharCorpse->IsStat( STATF_Sleeping ) &&
		! m_itCorpse.m_tDeathTime )
	{
		return( pCharCorpse );
	}
	return( NULL );
}

