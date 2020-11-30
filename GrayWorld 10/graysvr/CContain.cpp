//
// CContain.CPP
//
#include "graysvr.h"	// predef header.

//***************************************************************************
// -CContainer

void CContainer :: WeightChange( int iChange )
{
	// Propagate the weight change up the stack if there is one.
	m_totalweight += iChange;
}

void CContainer :: ContentAdd( CItem * pItem )
{
	// We are adding to a CChar or a CContainerItem
	ASSERT( pItem != NULL );

	if ( pItem->GetID() == ITEMID_GAME_BACKGAM ||
		pItem->GetID() == ITEMID_GAME_BOARD )
	{
		// Can't be put into any sort of a container.
		// delete all it's pieces.
		(dynamic_cast <CContainerItem*> (pItem))->DeleteAll();
	}

	CObList::InsertAfter( pItem );
	WeightChange( pItem->GetWeight());
}

void CContainer :: ContentRemove( CItem * pItem )
{
	// remove this object from the list.
	// Overload the RemoveAt for general lists to come here.
	
	ASSERT( pItem != NULL );
	CObList::RemoveAt( pItem );
	WeightChange( -pItem->GetWeight());
}

CItem * CContainer :: ContentFind( ITEMID_TYPE id )
{
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->IsSameID( id ))
			break;
		CContainer * pCont = pItem->GetThisContainer();
		if ( pCont != NULL ) // this is a sub-container.
		{
			if ( pItem->m_type == ITEM_CONTAINER_LOCKED ) continue;
			CItem * pItemInCont = pCont->ContentFind(id);
			if ( pItemInCont != NULL )
				return( pItemInCont );
		}
	}
	return( pItem );
}

#ifdef COMMENT

CItem* CContainer :: ContentFindRandom( void )
{
	// returns Pointer of random item, NULL if player carrying none
	int number = GetRandVal( GetCount() );
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL && number; pItem=pItem->GetNext())
		number --;
	return(pItem);
}

#endif

void CContainer :: WriteContent( CScript & s ) const
{
	// Write out all the items in me.
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		pItem->Write(s);
	}
}

int CContainer :: ContentCount( ITEMID_TYPE id ) const
{
	// Calculate total (gold or other items) in this container 
	int count=0;

	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->IsSameID( id ))
		{
			count += pItem->m_amount;
		}
		if ( pItem->IsContainer())	// this is a sub-container.
		{
			if ( pItem->GetID() == ITEMID_BANK_BOX && id != ITEMID_GOLD ) continue;
			if ( pItem->m_type == ITEM_CONTAINER_LOCKED ) continue;
			count += (dynamic_cast <CContainerItem*> (pItem))->ContentCount(id);
		}
	}
	return count;
}

int CContainer :: ContentConsume( ITEMID_TYPE id, int amount, bool fTest )
{
	// 0 = all consumed. else number left to consume.
	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( pItem->IsSameID( id ))
		{
			if ( pItem->m_amount > amount )
			{
				// subtract from it's count = part of stack.
				if ( ! fTest )
					pItem->SetAmount( pItem->m_amount - amount );
				return( 0 );
			}
			else
			{
				// Use whole stack.
				amount -= pItem->m_amount;
				if ( ! fTest )
					delete pItem;
			}
		}
		if ( pItem->IsContainer())	// this is a sub-container.
		{
			if ( pItem->GetID() == ITEMID_BANK_BOX && id != ITEMID_GOLD ) continue;
			if ( pItem->m_type == ITEM_CONTAINER_LOCKED ) continue;
			amount = (dynamic_cast <CContainerItem*> (pItem))->ContentConsume( id, amount, fTest );
		}
	}
	return( amount );
}

void CContainer :: ContentsDump( CPoint p )
{
	// Just dump the contents onto the ground.
	CItem * pItemNext;
	for ( CItem* pItem=GetContentHead(); pItem!=NULL; pItem=pItemNext)
	{
		pItemNext = pItem->GetNext();
		if ( pItem->m_Attr & ATTR_NEWBIE ) continue;	// hair and newbie stuff.
		// ??? scatter a little ?
		pItem->PutOnGround( p );
	}
}

//----------------------------------------------------
// -CContainerItem

CContainerItem :: CContainerItem( ITEMID_TYPE id ) : CItem( id )
{
	ASSERT( CItem :: IsContainerID( id ));
	m_type = ITEM_CONTAINER;
	m_fTinkerTrapped = false;
}

void CContainerItem :: WeightChange( int iChange )
{
	CContainer::WeightChange( iChange );
	if ( GetID() == ITEMID_BANK_BOX || iChange == 0 ) return;	// no change for bank boxes

	// Propagate the weight change up the stack if there is one.
	CObList * pList = GetParent();
	if ( pList == NULL ) return;
	CContainer * pCont = pList->GetThisContainer();
	if ( pCont == NULL ) return;	// on ground.
	pCont->WeightChange( iChange );
}

void CContainerItem :: ContentAdd( CItem * pItem, CPoint p )
{
	// Add to CContainerItem
	if ( p.m_x <= 0 || p.m_y <= 0 ||
		p.m_x > 256 || p.m_y > 256 )	// invalid container location ?
	{
		// Try to stack it.
		for ( CItem* pTry=GetContentHead(); pTry!=NULL; pTry=pTry->GetNext())
		{
			if ( pItem->IsStackable( pTry ))
			{
				pTry->SetAmount( pItem->m_amount + pTry->m_amount );
				delete pItem;
				return;
			}
		}

#define MAX_CONT_SIZE 100	// ??? based on container type.
		// ??? corpse type container has strange size.
		// Find a random toss location into the container.
		// Place randomly in the container ???
		p.m_x = 40 + GetRandVal( MAX_CONT_SIZE );
		p.m_y = 60 + GetRandVal( MAX_CONT_SIZE );
	}

	pItem->SetContainerFlags( UID_CONTAINED );
	pItem->m_p = p;
	pItem->m_equip_layer = LAYER_NONE;
	CContainer::ContentAdd( pItem );
	pItem->Update();
}

void CContainerItem :: ContentAdd( CItem * pItem )
{
	CPoint p;	// invalid point.
	if ( World.IsLoading())
	{
		p = pItem->m_p;
	}
	ContentAdd( pItem, p );
}

void CContainerItem :: Restock()
{
	// Assume this is a vendor type container.

	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->m_p.m_z ) pItem->m_p.m_z = 1;
		if ( pItem->m_amount > pItem->m_p.m_z ) continue;
		pItem->SetAmount( pItem->m_p.m_z );	// restock amount
	}
}

void CContainerItem :: CreateGamePieces()
{
	ASSERT( GetID() == ITEMID_GAME_BOARD || GetID() == ITEMID_GAME_BACKGAM );
	if ( GetCount()) return;	// already here.

	m_Attr |= ATTR_INVIS;	// Don't update it yet.

	CPoint p( 0, 0 );
	for ( int i=0; i<32; i++ )
	{
		// Add all it's pieces. (if not already added)
		CItem * pPiece = ItemCreateBase( (ITEMID_TYPE) 0xe12 );
		if ( pPiece == NULL ) break;
		pPiece->m_type = ITEM_GAME_PIECE;
		pPiece->m_more1 = i;

		if ( (i&7) == 0 )
		{
			p.m_x = 5;
			p.m_y += 10;
		}
		else
		{
			p.m_x += 5;
		}
		ContentAdd( pPiece, p );
	}

	m_Attr &= ~ATTR_INVIS;	// update now.
}

void CContainerItem :: OnTick()
{
	switch ( GetID())
	{
	case ITEMID_BANK_BOX:
		// Restock this box.
		Restock();
		return;
	}
	CItem::OnTick();
}
