// cvendoritem.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Implementation file for the CItemVendable class
//
// Initial version by Catharsis.  11/20/1999
//

#include "graysvr.h"

CItemVendable::CItemVendable( ITEMID_TYPE id, CItemBase * pDef ) : CItem( id, pDef )
{
	// Constructor
	m_price = 0;
	m_quality = 0;
}

CItemVendable::~CItemVendable()
{
	// Nothing really to do...no dynamic memory has been allocated.
	DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
}

void CItemVendable::DupeCopy( const CItem * pItem )
{
	CItem::DupeCopy( pItem );

	const CItemVendable * pVendItem = dynamic_cast <const CItemVendable *>(pItem);
	DEBUG_CHECK(pVendItem);
	if ( pVendItem == NULL )
		return;

	m_price = pVendItem->m_price;
	m_quality = pVendItem->m_quality;
}

enum IVC_TYPE
{
	IVC_BUYPRICE,
	IVC_PRICE,
	IVC_QUALITY,
	IVC_SELLPRICE,
	IVC_QTY,
};

LPCTSTR const CItemVendable::sm_szLoadKeys[IVC_QTY+1] =
{
	"BUYPRICE",		// ignore this.
	"PRICE",
	"QUALITY",
	"SELLPRICE",	// ignore this.
	NULL,
};

bool CItemVendable::r_WriteVal(LPCTSTR pszKey, CGString &sVal, CTextConsole *pSrc)
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case IVC_PRICE:	// PRICE
		sVal.FormatVal( m_price );
		return true;
	case IVC_QUALITY:	// QUALITY
		sVal.FormatVal( GetQuality());
		return true;
	default:
		return CItem::r_WriteVal( pszKey, sVal, pSrc );
	}
	EXC_CATCH("CItemVendable");
	return false;
}

void CItemVendable::r_DumpLoadKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szLoadKeys);
	CItem::r_DumpLoadKeys(pSrc);
}
void CItemVendable::r_DumpVerbKeys( CTextConsole * pSrc )
{
	r_DumpKeys(pSrc,sm_szVerbKeys);
	CItem::r_DumpVerbKeys(pSrc);
}

bool CItemVendable::r_LoadVal(CScript &s)
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case IVC_BUYPRICE:
	case IVC_SELLPRICE:
	case IVC_PRICE:	// PRICE
		m_price = s.GetArgVal();
		return true;
	case IVC_QUALITY:	// QUALITY
		SetQuality( s.GetArgVal());
		return true;
	}
	EXC_CATCH("CItemVendable");
	return( CItem::r_LoadVal(s));
}

void CItemVendable::r_Serialize( CGFile & a )
{
	// Read and write binary.
	CItem::r_Serialize(a);

}

void CItemVendable::r_Write(CScript &s)
{
	CItem::r_Write(s);
	if ( GetQuality() != 0 )
	{
		s.WriteKeyVal( "QUALITY", GetQuality());
	}

	// am i on a vendor right now ?
	if ( m_price > 0 )
	{
		s.WriteKeyVal( "PRICE", m_price );
	}
	return;
}

void CItemVendable::Restock( bool fSellToPlayers )
{
	// This is on a non-pet vendor.
	// allow prices to fluctuate randomly (per vendor) but hold the values for a bit.

	ASSERT( IsItemInContainer());
	if ( m_price < 0 )
		m_price = 0;	// signal to recalc this later.

	if ( fSellToPlayers )
	{
		// restock to my full amount.

		int iAmountRestock = GetContainedLayer();
		if ( ! iAmountRestock )
		{
			SetContainedLayer(1);
			iAmountRestock = 1;
		}
		if ( GetAmount() < iAmountRestock )
		{
			SetAmount( iAmountRestock );	// restock amount
		}
	}
	else
	{
		// Clear the amount i have bought from players.
		// GetAmount() is the max that i will buy in the next period.
		SetContainedLayer(0);
	}
}

void CItemVendable::SetPlayerVendorPrice( LONG lPrice )
{
	// This can only be inside a vendor container.
	// DEBUG_CHECK( g_World.IsLoading() || IsItemInContainer() );

	if ( lPrice < 0 )
		lPrice = 0;
	m_price = lPrice;
}

LONG CItemVendable::GetBasePrice() const
{
	// Get the price that the player set on his vendor
	//if ( m_price < 0 ) return 0;
	return( m_price );
}

LONG CItemVendable::GetVendorPrice( int iConvertFactor )
{
	// Player is buying/selling from a vendor.
	// ASSUME this item is on the vendor !
	// Consider: (if not on a player vendor)
	//  Quality of the item.
	//  rareity of the item.
	// ARGS:
	// iConvertFactor will consider:
	//  Vendors Karma.
	//  Players Karma
	// -100 = player selling to the vendor (lower price)
	// +100 = vendor selling to player (higher price)

	int iPrice = m_price;
	if ( iPrice <= 0 )	// set on player vendor.
	{
		if ( iPrice == 0 )	// set a new randomized price for the item
		{
			CItemBase * pItemDef;
			if ( IsType( IT_DEED ))
			{
				// Deeds just represent the item they are deeding.
				pItemDef = CItemBase::FindItemBase((ITEMID_TYPE) RES_GET_INDEX(m_itDeed.m_Type));
				if ( pItemDef == NULL )
					return( 1 );
			}
			else
			{
				pItemDef = Item_GetDef();
			}
			iPrice = pItemDef->GetMakeValue(GetQuality());
			m_price = -iPrice;
		}
		else
		{
			iPrice = -iPrice;
		}
	}

	iPrice += IMULDIV( iPrice, iConvertFactor, 100 );
	return iPrice;
}

bool CItemVendable::IsValidSaleItem( bool fBuyFromVendor ) const
{
	// Can this individual item be sold or bought ?
	if ( ! IsMovableType())
	{
		DEBUG_ERR(( "Vendor uid=0%lx selling unmovable item %s='%s'\n", (DWORD) GetTopLevelObj()->GetUID(), GetResourceName(), GetName()));
		return( false );
	}
	if ( ! fBuyFromVendor )
	{
		// cannot sell these to a vendor.
		if ( IsAttr(ATTR_NEWBIE|ATTR_MOVE_NEVER))
			return( false );	// spellbooks !
	}
	if ( IsType(IT_COIN))
		return( false );
	return( true );
}

bool CItemVendable::IsValidNPCSaleItem() const
{
	// This item is in an NPC's vendor box.
	// Is it a valid item that NPC's should be selling ?

	CItemBase * pItemDef = Item_GetDef();

	if ( m_price <= 0 && pItemDef->GetMakeValue(0) <= 0 )
	{
		DEBUG_ERR(( "Vendor uid=0%lx selling unpriced item %s='%s'\n", (DWORD) GetTopLevelObj()->GetUID(), GetResourceName(), GetName()));
		return( false );
	}

	if ( ! IsValidSaleItem( true ))
	{
		DEBUG_ERR(( "Vendor uid=0%lx selling bad item %s='%s'\n", (DWORD) GetTopLevelObj()->GetUID(), GetResourceName(), GetName()));
		return( false );
	}

	return( true );
}

