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

#ifdef _DEBUG
	if ( m_totalweight < 0 )
	{
		DEBUG_CHECK(( "Trap" ));
	}
#endif

	DEBUG_CHECK( m_totalweight >= 0 );
}

int CContainer::FixWeight()
{
	// If there is some sort of ASSERT during item add then this is used to fix it.
#if 1
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
	ASSERT( pItem->IsValidUID());	// it should be valid at this point.
	if ( pItem->GetParent() == this )
		return;

	if ( g_Log.IsLogged( LOGL_TRACE ))
	{
		DEBUG_CHECK( GetCount() <= MAX_ITEMS_CONT );
	}
	CGObList::InsertHead( pItem );
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
	pItem->SetContainerFlags(UID_O_DISCONNECT);	// It is no place for the moment.
	OnWeightChange( -pItem->GetWeight());
}

void CContainer::r_SerializeContent( CGFile & a ) const
{
	const CGObList * pListThis = STATIC_CAST <const CGObList *>(this);
	ASSERT(pListThis);

	// Write out all the items in me.
	CItem* pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		ASSERT( pItem->GetParent() == pListThis );
		pItem->r_Serialize(a);
	}
}

void CContainer::r_WriteContent( CScript & s ) const
{
	const CGObList * pListThis = STATIC_CAST <const CGObList *>(this);
	ASSERT(pListThis);

	// Write out all the items in me.
	CItem* pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		ASSERT( pItem->GetParent() == pListThis );
		pItem->r_WriteSafe(s);
	}
}

CItem * CContainer::ContentFind( RESOURCE_ID_BASE rid, DWORD dwArg, int iDecendLevels ) const
{
	// send all the items in the container.

	if ( rid.GetResIndex() == 0 )
		return( NULL );

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->IsResourceMatch( rid, dwArg ))
			break;
		if ( iDecendLevels <= 0 )
			continue;
		CItemContainer * pCont = dynamic_cast <CItemContainer*>(pItem);
		if ( pCont != NULL )
		{
			if ( ! pCont->IsSearchable())
				continue;
			CItem * pItemInCont = pCont->ContentFind( rid, dwArg, iDecendLevels-1 );
			if ( pItemInCont )
				return( pItemInCont );
		}
	}
	return( pItem );
}

TRIGRET_TYPE CContainer::OnContTriggerForLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, CScriptLineContext & StartContext, CScriptLineContext & EndContext, RESOURCE_ID_BASE rid, DWORD dwArg, int iDecendLevels )
{
	if ( rid.GetResIndex() != 0 )
	{
		CItem* pItem=GetContentHead();
		for ( ; pItem!=NULL; pItem=pItem->GetNext())
		{
			if ( pItem->IsResourceMatch( rid, dwArg ))
			{
				s.SeekContext( StartContext );
				TRIGRET_TYPE iRet = pItem->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
 				if ( iRet != TRIGRET_ENDIF )
				{
					return( iRet );
				}
				EndContext = s.GetContext();
			}
			if ( iDecendLevels <= 0 )
				continue;
			CItemContainer * pCont = dynamic_cast <CItemContainer*>(pItem);
			if ( pCont != NULL )
			{
				if ( ! pCont->IsSearchable())
					continue;
				CContainer * pContBase = dynamic_cast <CContainer *> (pCont);
				TRIGRET_TYPE iRet = pContBase->OnContTriggerForLoop( s, pSrc, pArgs, pResult, StartContext, EndContext, rid, dwArg, iDecendLevels-1 );
 				if ( iRet != TRIGRET_ENDIF )
				{
					return( iRet );
				}
			}
		}
	}
	if ( EndContext.m_lOffset <= StartContext.m_lOffset )
	{
		CScriptObj * pScript = dynamic_cast <CScriptObj *> (this);
		TRIGRET_TYPE iRet = pScript->OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
	}
	else
	{
		s.SeekContext( EndContext );
	}
	return( TRIGRET_ENDIF );
}

TRIGRET_TYPE CContainer::OnGenericContTriggerForLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, CScriptLineContext & StartContext, CScriptLineContext & EndContext, int iDecendLevels )
{
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		s.SeekContext( StartContext );
		TRIGRET_TYPE iRet = pItem->OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pSrc, pArgs, pResult );
 		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
		EndContext = s.GetContext();
		if ( iDecendLevels <= 0 )
			continue;
		CItemContainer * pCont = dynamic_cast <CItemContainer*>(pItem);
		if ( pCont != NULL )
		{
			if ( ! pCont->IsSearchable())
				continue;
			CContainer * pContBase = dynamic_cast <CContainer *> (pCont);
			TRIGRET_TYPE iRet = pContBase->OnGenericContTriggerForLoop( s, pSrc, pArgs, pResult, StartContext, EndContext, iDecendLevels-1 );
 			if ( iRet != TRIGRET_ENDIF )
			{
				return( iRet );
			}
		}
	}
	if ( EndContext.m_lOffset <= StartContext.m_lOffset )
	{
		CScriptObj * pScript = dynamic_cast <CScriptObj *> (this);
		TRIGRET_TYPE iRet = pScript->OnTriggerRun( s, TRIGRUN_SECTION_FALSE, pSrc, pArgs, pResult );
		if ( iRet != TRIGRET_ENDIF )
		{
			return( iRet );
		}
	}
	else
	{
		s.SeekContext( EndContext );
	}
	return( TRIGRET_ENDIF );
}

bool CContainer::ContentFindKeyFor( CItem * pLocked ) const
{
	// Look for the key that fits this in my possesion.
	DEBUG_CHECK( pLocked->IsTypeLockable());
	return( ContentFind( RESOURCE_ID( RES_TYPEDEF, IT_KEY ), pLocked->m_itContainer.m_lockUID ) != NULL );
}

CItem* CContainer::ContentFindRandom( void ) const
{
	// returns Pointer of random item, NULL if player carrying none
	return( dynamic_cast <CItem*> ( GetAt( Calc_GetRandVal( GetCount()))));
}

int CContainer::ContentConsume( RESOURCE_ID_BASE rid, int amount, bool fTest, DWORD dwArg )
{
	// ARGS:
	//  dwArg = a hack for ores.
	// RETURN:
	//  0 = all consumed ok.
	//  # = number left to be consumed. (still required)

	if ( rid.GetResIndex() == 0 )
		return( amount );	// from skills menus.

	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();

		if ( pItem->IsResourceMatch( rid, dwArg ))
		{
			amount -= pItem->ConsumeAmount( amount, fTest );
			if ( amount <= 0 )
				break;
		}

		CItemContainer * pCont = dynamic_cast <CItemContainer*> (pItem);
		if ( pCont != NULL )	// this is a sub-container.
		{
			if ( rid == RESOURCE_ID(RES_TYPEDEF,IT_GOLD))
			{
				if ( pCont->IsType(IT_CONTAINER_LOCKED))
					continue;
			}
			else
			{
				if ( ! pCont->IsSearchable())
					continue;
			}
			amount = pCont->ContentConsume( rid, amount, fTest, dwArg );
			if ( amount <= 0 )
				break;
		}
	}
	return( amount );
}

int CContainer::ContentCount( RESOURCE_ID_BASE rid, DWORD dwArg )
{
	// Calculate total (gold or other items) in this recursed container
	return( INT_MAX - ContentConsume( rid, INT_MAX, true, dwArg ));
}

void CContainer::ContentAttrMod( WORD wAttr, bool fSet )
{
	// Mark the attr
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( fSet )
		{
			pItem->SetAttr( wAttr );
		}
		else
		{
			pItem->ClrAttr( wAttr );
		}
		CItemContainer * pCont = dynamic_cast <CItemContainer*> (pItem);
		if ( pCont != NULL )	// this is a sub-container.
		{
			pCont->ContentAttrMod( wAttr, fSet );
		}
	}
}

void CContainer::ContentsDump( const CPointMap & pt, WORD wAttrLeave )
{
	// Just dump the contents onto the ground.
	DEBUG_CHECK( pt.IsValidPoint());
	wAttrLeave |= ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2;
	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( pItem->IsAttr(wAttrLeave))
			continue;	// hair and newbie stuff.
		// ??? scatter a little ?
		pItem->MoveToCheck( pt );
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

int CContainer::ResourceConsumePart( const CResourceQtyArray * pResources, int iReplicationQty, int iDamagePercent, bool fTest )
{
	// Consume just some of the resources.
	// ARGS:
	//	pResources = the resources i need to make 1 replication of this end product.
	//  iDamagePercent = 0-100
	// RETURN:
	//  -1 = all needed items where present.
	// index of the item we did not have.

	if ( iDamagePercent <= 0 )
		return( -1 );

	int iMissing = -1;

	int iQtyRes = pResources->GetCount();
	for ( int i=0; i<iQtyRes; i++ )
	{
		int iResQty = pResources->GetAt(i).GetResQty();
		if (iResQty<=0)	// not sure why this would be true
			continue;

		int iQtyTotal = ( iResQty * iReplicationQty );
		if ( iQtyTotal <= 0 )
			continue;
		iQtyTotal = IMULDIV( iQtyTotal, iDamagePercent, 100 );
		if ( iQtyTotal <= 0 )
			continue;

		RESOURCE_ID rid = pResources->GetAt(i).GetResourceID();
		int iRet = ContentConsume( rid, iQtyTotal, fTest );
		if ( iRet )
		{
			iMissing = i;
		}
	}

	return( iMissing );
}

int CContainer::ResourceConsume( const CResourceQtyArray * pResources, int iReplicationQty, bool fTest, DWORD dwArg )
{
	// Consume or test all the required resources.
	// ARGS:
	//	pResources = the resources i need to make 1 replication of this end product.
	// RETURN:
	//  how many whole objects can be made. <= iReplicationQty

	if ( iReplicationQty <= 0 )
		iReplicationQty = 1;
	if ( ! fTest && iReplicationQty > 1 )
	{
		// Test what the max number we can really make is first !
		// All resources must be consumed with the same number.
		iReplicationQty = ResourceConsume( pResources, iReplicationQty, true, dwArg );
	}

	int iQtyMin = INT_MAX;
	for ( int i=0; i<pResources->GetCount(); i++ )
	{
		int iResQty = pResources->GetAt(i).GetResQty();
		if (iResQty<=0)	// not sure why this would be true
			continue;

		int iQtyTotal = ( iResQty * iReplicationQty );
		RESOURCE_ID rid = pResources->GetAt(i).GetResourceID();
		if ( rid.GetResType() == RES_SKILL )
		{
			CChar *	pChar	= dynamic_cast <CChar *> (this);
			if ( !pChar )
				continue;
			if ( pChar->Skill_GetBase( (SKILL_TYPE) rid.GetResIndex() ) < iResQty )
				return 0;
			continue;
		}

		int iQtyCur = iQtyTotal - ContentConsume( rid, iQtyTotal, fTest, dwArg );
		iQtyCur /= iResQty;
		if ( iQtyCur < iQtyMin )
			iQtyMin = iQtyCur;
	}

	if ( iQtyMin == INT_MAX )	// it has no resources ? So i guess we can make it from nothing ?
		return( iReplicationQty );

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

bool CContainer::r_GetRefContainer( LPCTSTR & pszKey, CScriptObj * & pRef )
{
	if ( ! strnicmp( pszKey, "FINDID", 6 ))
	{
		// Get item id from the container.
		pszKey += 6;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFind( g_Cfg.ResourceGetID( RES_ITEMDEF, pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDCONT", 8 ))
	{
		// Get enumerated item from the container.
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		pRef = GetAt( Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
	if ( ! strnicmp( pszKey, "FINDTYPE", 8 ))
	{
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFind( g_Cfg.ResourceGetID( RES_TYPEDEF, pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}

#if 0
	if ( ! strnicmp( pszKey, "FINDUID", 7 ))
	{
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		pRef = ContentFindType( (IT_TYPE) Exp_GetSingle( pszKey ));
		SKIP_SEPERATORS(pszKey);
		return( true );
	}
#endif

	return( false );
}

LPCTSTR const CContainer::sm_szLoadKeys[] = // static
{
	"RESCOUNT",
	"RESTEST",
	"FCOUNT",
	"COUNT",
	NULL,
};

bool CContainer::r_WriteValContainer( LPCTSTR pszKey, CGString & sVal )
{
	EXC_TRY(("r_WriteVal('%s',)", pszKey));
	if ( ! strnicmp( pszKey, sm_szLoadKeys[0], 8 ))
	{
		// RESCOUNT
		pszKey += 8;
		SKIP_SEPERATORS(pszKey);
		if ( ! pszKey[0] )
		{
			sVal.FormatVal( GetCount());	// count all.
		}
		else
		{
			sVal.FormatVal( ContentCount( g_Cfg.ResourceGetID( RES_ITEMDEF, pszKey )));
		}
		return( true );
	}
	if ( ! strnicmp( pszKey, sm_szLoadKeys[1], 7 ))
	{
		// RESTEST
		pszKey += 7;
		SKIP_SEPERATORS(pszKey);
		CResourceQtyArray Resources;
		Resources.Load( pszKey );
		sVal.FormatVal( ResourceConsume( &Resources, 1, true ));
		return( true );
	}

	if ( ! strnicmp( pszKey, sm_szLoadKeys[2], 6 ))
	{
		sVal.FormatVal(ContentCountAll());
		return true;
	}

	if ( ! strnicmp(pszKey, sm_szLoadKeys[3], 5 ))
	{
		int iTotal = 0;

		for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
		{
			iTotal++;
		}
		sVal.FormatVal(iTotal);
		return true;
	}
	EXC_CATCH("CContainer");
	return( false );
}

//----------------------------------------------------
// -CItemContainer

CItemContainer::CItemContainer( ITEMID_TYPE id, CItemBase * pItemDef ) :
	CItemVendable( id, pItemDef )
{
	// DEBUG_CHECK( pItemDef->IsTypeContainer());
	// m_fTinkerTrapped = false;
}

void CItemContainer::r_Serialize( CGFile & a )
{
	CItemVendable::r_Serialize(a);
	r_SerializeContent(a);
}

void CItemContainer::r_Write( CScript & s )
{
	CItemVendable::r_Write(s);
	r_WriteContent(s);
}

bool CItemContainer::r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef )
{
	if ( r_GetRefContainer( pszKey, pRef ))
	{
		return( true );
	}
	return( CItemVendable::r_GetRef( pszKey, pRef ));
}

bool CItemContainer::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	if ( r_WriteValContainer( pszKey, sVal ))
	{
		return( true );
	}
	return( CItemVendable::r_WriteVal( pszKey, sVal, pSrc ));
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
	cmd.SecureTrade.m_action = SECURE_TRADE_CHANGE;	// status
	cmd.SecureTrade.m_fname = 0;

	if ( pChar1->IsClient())
	{
		cmd.SecureTrade.m_UID = GetUID();
		cmd.SecureTrade.m_UID1 = m_itEqTradeWindow.m_fCheck;
		cmd.SecureTrade.m_UID2 = pPartner->m_itEqTradeWindow.m_fCheck;
		pChar1->GetClient()->xSendPkt( &cmd, 0x11 );
	}
	if ( pChar2->IsClient())
	{
		cmd.SecureTrade.m_UID = pPartner->GetUID();
		cmd.SecureTrade.m_UID1 = pPartner->m_itEqTradeWindow.m_fCheck;
		cmd.SecureTrade.m_UID2 = m_itEqTradeWindow.m_fCheck;
		pChar2->GetClient()->xSendPkt( &cmd, 0x11 );
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

	ASSERT( IsType(IT_EQ_TRADE_WINDOW) );

	CChar * pChar = dynamic_cast <CChar*> (GetParent());
	if ( pChar == NULL )
		return;

	if ( pChar->IsClient())
	{
		// Send the cancel trade message.
		CCommand cmd;
		cmd.SecureTrade.m_Cmd = XCMD_SecureTrade;
		cmd.SecureTrade.m_len = 0x11;
		cmd.SecureTrade.m_action = SECURE_TRADE_CLOSE;
		cmd.SecureTrade.m_UID = GetUID();
		cmd.SecureTrade.m_UID1 = 0;
		cmd.SecureTrade.m_UID2 = 0;
		cmd.SecureTrade.m_fname = 0;
		pChar->GetClient()->xSendPkt( &cmd, 0x11 );
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

	m_uidLink.InitUID();	// unlink.
	pPartner->m_uidLink.InitUID();
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
	} sm_ContSize[] =
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

	// ??? pItemDef->m_ttContainer.m_dwMinXY to m_dwMaxXY
	// Get a random location in the container.

	CItemBase * pItemDef = Item_GetDef();
	GUMP_TYPE gump = pItemDef->IsTypeContainer();
	int i=0;
	for ( ; true; i++ )
	{
		if (i>=COUNTOF(sm_ContSize))
		{
			i=0;
			DEBUG_ERR(( "unknown container gump id %d for 0%x\n", gump, GetDispID()));
			break;
		}
		if ( sm_ContSize[i].m_gump == gump )
			break;
	}

	return( CPointMap(
		sm_ContSize[i].m_minx + Calc_GetRandVal( sm_ContSize[i].m_maxx - sm_ContSize[i].m_minx ),
		sm_ContSize[i].m_miny + Calc_GetRandVal( sm_ContSize[i].m_maxy - sm_ContSize[i].m_miny ),
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
		if ( IsType(IT_EQ_TRADE_WINDOW) )
		{
			// Drop into a trade window.
			Trade_Status( false );
		}
		switch ( pItem->GetType())
		{
		case IT_LIGHT_LIT:
			// Douse the light in a pack ! (if possible)
			pItem->Use_Light();
			break;
		case IT_GAME_BOARD:
			// Can't be put into any sort of a container.
			// delete all it's pieces.
			CItemContainer* pCont = dynamic_cast <CItemContainer*> (pItem);
			ASSERT(pCont);
			pCont->DeleteAll();
			break;
		}
	}

	if ( pt.m_x <= 0 || pt.m_y <= 0 ||
		pt.m_x > 512 || pt.m_y > 512 )	// invalid container location ?
	{
		// Try to stack it.
		if ( ! g_Serv.IsLoading() && pItem->Item_GetDef()->IsStackableType())
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
		DEBUG_CHECK( ! pItem->IsType(IT_EQ_MEMORY_OBJ));
	}
#endif

	switch ( GetType())
	{
	case IT_KEYRING: // empty key ring.
		SetKeyRing();
		break;
	case IT_EQ_VENDOR_BOX:
		if ( ! IsItemEquipped())	// vendor boxes should ALWAYS be equipped !
		{
			DEBUG_ERR(("Un-equipped vendor box uid=0%d is bad\n", (DWORD) GetUID()));
			break;
		}
		{
			CItemVendable * pItemVend = dynamic_cast <CItemVendable *>(pItem);
			if ( pItemVend == NULL )
			{
				g_Log.Event( LOGL_WARN, "Vendor non-vendable item: %s\n", (LPCTSTR) pItem->GetResourceName());
				pItem->Delete();
				break;
			}

			pItemVend->SetPlayerVendorPrice( 0 );	// unpriced yet.
			pItemVend->SetContainedLayer( pItem->GetAmount());
		}
		break;
	}

	switch( pItem->GetID())
	{
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

bool CItemContainer::IsItemInside(const CItem * pItem) const
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
	if ( IsType(IT_EQ_TRADE_WINDOW))
	{
		// Remove from a trade window.
		Trade_Status( false );
		m_TagDefs.SetNum("wait1sec", g_World.GetCurrentTime().GetTimeRaw() + 60*TICK_PER_SEC, false);
	}
	if ( IsType(IT_EQ_VENDOR_BOX) && IsItemEquipped())	// vendor boxes should ALWAYS be equipped !
	{
		CItemVendable * pItemVend = dynamic_cast <CItemVendable *>(pItem);
		if ( pItemVend != NULL )
		{
			pItemVend->SetPlayerVendorPrice( 0 );
		}
	}

	CContainer::OnRemoveOb(pObRec);

	if ( IsType(IT_KEYRING))	// key ring.
	{
		SetKeyRing();
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
	DEBUG_CHECK( IsType(IT_CONTAINER_LOCKED));
	SetType(IT_CONTAINER);
	m_itContainer.m_lockUID = GetUID();
	m_itContainer.m_lock_complexity = 500 + Calc_GetRandVal( 600 );

	CItem * pKey = CreateScript( ITEMID_KEY_COPPER );
	ASSERT(pKey);
	pKey->m_itKey.m_lockUID = GetUID();
	ContentAdd( pKey );
}

void CItemContainer::SetKeyRing()
{
	// Look of a key ring depends on how many keys it has in it.
	static const ITEMID_TYPE sm_Item_Keyrings[] =
	{
		ITEMID_KEY_RING0, // empty key ring.
		ITEMID_KEY_RING1,
		ITEMID_KEY_RING1,
		ITEMID_KEY_RING3,
		ITEMID_KEY_RING3,
		ITEMID_KEY_RING5,
	};

	int iQty = GetCount();
	if ( iQty >= COUNTOF(sm_Item_Keyrings))
		iQty = COUNTOF(sm_Item_Keyrings)-1;

	ITEMID_TYPE id = sm_Item_Keyrings[iQty];
	if ( id != GetID())
	{
		SetID(id);	// change the type as well.
		Update();
	}
}

bool CItemContainer::CanContainerHold( const CItem * pItem, const CChar * pCharMsg )
{
	if ( pCharMsg == NULL )
		return( false );
	if ( pCharMsg->IsPriv(PRIV_GM))	// a gm can doing anything.
		return( true );

	if ( IsAttr(ATTR_MAGIC))
	{
		// Put stuff in a magic box
		pCharMsg->SysMessageDefault( DEFMSG_CONT_MAGIC );
		return( false );
	}

	if ( GetCount() >= MAX_ITEMS_CONT-1 )
	{
		pCharMsg->SysMessageDefault( DEFMSG_CONT_FULL );
		return( false );
	}

	if ( ! IsItemEquipped() &&	// does not apply to my pack.
		pItem->IsContainer() &&
		pItem->Item_GetDef()->GetVolume() >= Item_GetDef()->GetVolume())
	{
		// is the container too small ? can't put barrels in barrels.
		pCharMsg->SysMessageDefault( DEFMSG_CONT_TOOSMALL );
		return( false );
	}

	switch ( GetType())
	{
	case IT_EQ_BANK_BOX:
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
		if (( ContentCountAll() + iItemsInContainer ) > g_Cfg.m_iBankIMax )
		{
			pCharMsg->SysMessageDefault( DEFMSG_BVBOX_FULL_ITEMS );
			return( false );
		}

		// Check the weightlimit on bankboxes.
		if (( GetWeight() + pItem->GetWeight()) > g_Cfg.m_iBankWMax )
		{
			pCharMsg->SysMessageDefault( DEFMSG_BVBOX_FULL_WEIGHT );
			return( false );
		}
		}
		break;

	case IT_GAME_BOARD:
		if ( ! pItem->IsType(IT_GAME_PIECE))
		{
			pCharMsg->SysMessageDefault( DEFMSG_ERR_NOTGAMEPIECE );
			return( false );
		}
		break;
	case IT_BBOARD:		// This is a trade window or a bboard.
		return( false );
	case IT_EQ_TRADE_WINDOW:
		// BBoards are containers but you can't put stuff in them this way.
		return( true );

	case IT_KEYRING: // empty key ring.
		if ( ! pItem->IsType(IT_KEY) )
		{
			pCharMsg->SysMessageDefault( DEFMSG_ERR_NOTKEY );
			return( false );
		}
		if ( ! pItem->m_itKey.m_lockUID )
		{
			pCharMsg->SysMessageDefault( DEFMSG_ERR_NOBLANKRING );
			return( false );
		}
		break;

	case IT_EQ_VENDOR_BOX:
		if ( pItem->IsTimerSet() && ! pItem->IsAttr(ATTR_DECAY))
		{
			pCharMsg->SysMessageDefault( DEFMSG_ERR_NOT4SALE );
			return( false );
		}
		break;

	case IT_TRASH_CAN:
		Sound( 0x235 ); // a little sound so we know it "ate" it.
		pCharMsg->SysMessageDefault( DEFMSG_ITEMUSE_TRASHCAN );
		SetTimeout( 15*TICK_PER_SEC );
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

	if ( IsItemEquipped())
	{
		// Part of a vendor.
		CChar * pChar = dynamic_cast <CChar*> (GetParent());
		ASSERT(pChar);
		if ( ! pChar->IsStatFlag( STATF_Pet ))	// Not on a hired vendor.
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
					CItemVendable * pVendItem = dynamic_cast <CItemVendable *> (pItem);
					if ( pVendItem != NULL )
						pVendItem->Restock( true );
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
						pVendItem->Restock( false );
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

void CItemContainer::OnOpenEvent( CChar * pCharOpener, const CObjBaseTemplate * pObjTop )
{
	// The container is being opened. Explode ? etc ?

	ASSERT(pCharOpener);

	if ( IsType(IT_EQ_BANK_BOX) ||
		IsType(IT_EQ_VENDOR_BOX))
	{
		const CChar * pCharTop = dynamic_cast <const CChar *>(pObjTop);
		if ( pCharTop == NULL )
			return;

		int iStones = GetWeight()/WEIGHT_UNITS;
		CGString sMsg;
		if ( pCharTop == pCharOpener )
		{
			sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_BVBOX_OPEN_SELF ), iStones, (LPCTSTR) GetName() );
		}
		else
		{
			sMsg.Format( g_Cfg.GetDefaultMsg( DEFMSG_BVBOX_OPEN_OTHER ), pCharTop->GetPronoun(), iStones, (LPCTSTR) pCharTop->GetPossessPronoun(), (LPCTSTR) GetName() );
		}
		pCharOpener->SysMessage( sMsg );

		// these are special. they can only be opened near the designated opener.
		// see CanTouch
		m_itEqBankBox.m_pntOpen = pCharOpener->GetTopPoint();
	}
}

void CItemContainer::Game_Create()
{
	ASSERT( IsType(IT_GAME_BOARD) );
	if ( GetCount())
		return;	// already here.

	static const ITEMID_TYPE sm_Item_ChessPieces[] =
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
	bool fChess = ( m_itGameBoard.m_GameType == 0 );

	static const WORD sm_ChessRow[] =
	{
		5,
		40,
		160,
		184,
	};
	static const WORD sm_CheckerRow[] =
	{
		30,
		55,
		178,
		205,
	};

	for ( int i=0; i<COUNTOF(sm_Item_ChessPieces); i++ )
	{
		// Add all it's pieces. (if not already added)
		CItem * pPiece = CItem::CreateBase( (fChess ) ? sm_Item_ChessPieces[i] :
			(( i >= (2*8)) ? ITEMID_GAME1_CHECKER : ITEMID_GAME2_CHECKER ));
		if ( pPiece == NULL )
			break;
		pPiece->SetType(IT_GAME_PIECE);
		if ( (i&7) == 0 )
		{
			pt.m_x = 42;
			pt.m_y = (fChess)?sm_ChessRow[ i/8 ]:sm_CheckerRow[ i/8 ];
		}
		else
		{
			pt.m_x += 25;
		}
		ContentAdd( pPiece, pt );
	}
}

enum ICV_TYPE
{
	ICV_DELETE,
	ICV_EMPTY,
	ICV_FIXWEIGHT,
	ICV_OPEN,
	ICV_QTY,
};

LPCTSTR const CItemContainer::sm_szVerbKeys[ICV_QTY+1] =
{
	"DELETE",
	"EMPTY",
	"FIXWEIGHT",
	"OPEN",
	NULL,
};

void CItemContainer::r_DumpVerbKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szVerbKeys);
	CItemVendable::r_DumpVerbKeys(pSrc);
}

bool CItemContainer::r_Verb( CScript &s, CTextConsole * pSrc )
{
	EXC_TRY(("r_Verb('%s %s',%x)", s.GetKey(), s.GetArgStr(), pSrc));
	ASSERT(pSrc);
	switch ( FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF(sm_szVerbKeys)-1 ))
	{
	case ICV_DELETE: // "DELETE"
		if ( s.HasArgs())
		{
			// 1 based pages.
			int index = s.GetArgVal();
			if ( index <= GetCount())
			{
				delete GetAt( index-1 );
				return( true );
			}
		}
		return( false );
	case ICV_EMPTY:	// "EMPTY"
		{
			int		iType	= s.GetArgVal();;
			DeleteAll();
			return( true );
		}
	case ICV_FIXWEIGHT:
		FixWeight();
		return( true );

	case ICV_OPEN: // "OPEN"
		if ( pSrc->GetChar())
		{
			CChar * pChar = pSrc->GetChar();
			if ( pChar->IsClient())
			{
				CClient * pClient = pChar->GetClient();
				ASSERT(pClient);

				if ( s.HasArgs() )
				{
					pClient->addItem( this );
					pClient->addCustomSpellbookOpen( this, s.GetArgVal() );
					return true;
				}
				pClient->addItem( this );	// may crash client if we dont do this.
				pClient->addContainerSetup( this );
				OnOpenEvent( pChar, GetTopLevelObj());
			}
		}
		return( true );
	}
	EXC_CATCH("CItemContainer");
	return( CItemVendable::r_Verb( s, pSrc ));
}

bool CItemContainer::OnTick()
{
	// equipped or not.
	switch ( GetType())
	{
	case IT_TRASH_CAN:
		// Empty it !
		DeleteAll();
		return true;
	case IT_CONTAINER:
		if ( IsAttr(ATTR_MAGIC))
		{
			// Magic restocking container.
			Restock();
			return true;
		}
		break;
	case IT_EQ_BANK_BOX:
	case IT_EQ_VENDOR_BOX:
		// Restock this box.
		// Magic restocking container.
		Restock();
		return true;
	}
	return CItemVendable::OnTick();
}

////////////////////////////////////////////////////
// CItemCorpse

CChar * CItemCorpse::IsCorpseSleeping() const
{
	// Is this corpse really a sleeping person ?
	// CItemCorpse
	ASSERT( IsType(IT_CORPSE));

	CChar * pCharCorpse = m_uidLink.CharFind();
	if ( pCharCorpse &&
		pCharCorpse->IsStatFlag( STATF_Sleeping ) &&
		! m_itCorpse.m_timeDeath.IsTimeValid() )
	{
		return( pCharCorpse );
	}
	return( NULL );
}

