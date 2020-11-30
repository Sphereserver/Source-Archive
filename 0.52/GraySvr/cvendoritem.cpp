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
	m_buyprice = 0;
	m_sellprice = 0;
	m_quality = 0;
	m_bBuyFixed = false;
	m_bSellFixed = false;
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

	m_buyprice = pVendItem->m_buyprice;
	m_sellprice = pVendItem->m_sellprice;
	m_quality = pVendItem->m_quality;
	m_bBuyFixed = pVendItem->m_bBuyFixed;
	m_bSellFixed = pVendItem->m_bSellFixed;
}

void CItemVendable::SetBuyPrice(DWORD dwPrice)
{
	if ( dwPrice != 0 )
	{
		// fixing the buy price on this CItemVendable
		m_buyprice = dwPrice;
		m_bBuyFixed = true;
	}
	else
	{
		// Unfixing the buy price on this CItemVendable
		// Leave the buy price as is for now
		m_bBuyFixed = false;
	}
}

void CItemVendable::SetSellPrice(DWORD dwPrice)
{
	if ( dwPrice != 0 )
	{
		// fixing the sell price on this CItemVendable
		m_sellprice = dwPrice;
		m_bSellFixed = true;
	}
	else
	{
		// Unfixing the sell price on this CItemVendable
		// Leave the sell price as is for now
		m_bSellFixed = false;
	}
}

void CItemVendable::Restock()
{
	if ( ! m_bBuyFixed )
	{
		DWORD dwRange = m_pDef->m_buyvaluemax - m_pDef->m_buyvaluemin;
		if ( dwRange <= 0 )
			m_buyprice = m_pDef->m_buyvaluemin;
		else
			m_buyprice = m_pDef->m_buyvaluemin + ( rand() % ( dwRange + 1 ));
	}
	if ( ! m_bSellFixed )
	{
		DWORD dwRange = m_pDef->m_sellvaluemax - m_pDef->m_sellvaluemin;
		if ( dwRange <= 0 )
			m_sellprice = m_pDef->m_sellvaluemin;
		else
			m_sellprice = m_pDef->m_sellvaluemin + ( rand() % ( dwRange + 1 ));
	}
}

const TCHAR * CItemVendable::sm_KeyTable[] =
{
	"BUYPRICE",
	"QUALITY",
	"SELLPRICE",
};

bool CItemVendable::r_WriteVal(const TCHAR *pKey, CGString &sVal, CTextConsole *pSrc)
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// BUYPRICE
		if ( m_bBuyFixed )
			sVal.FormatVal( GetBuyPrice());
		else
			sVal.FormatVal( 0 );
		return true;
	case 1:	// QUALITY
		sVal.FormatVal( GetQuality());
		return true;
	case 2:	// SELLPRICE
		if ( m_bSellFixed )
			sVal.FormatVal( GetSellPrice());
		else
			sVal.FormatVal( 0 );
		return true;
	}
	return CItem::r_WriteVal( pKey, sVal, pSrc );
}

bool CItemVendable::r_LoadVal(CScript &s)
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case 0:	// BUYPRICE
		SetBuyPrice( s.GetArgVal());
		return true;
	case 1:	// QUALITY
		SetQuality( s.GetArgVal());
		return true;
	case 2:	// SELLPRICE
		SetSellPrice( s.GetArgVal());
		return true;
	}
	return ( CItem::r_LoadVal(s));
}

void CItemVendable::r_Write(CScript &s)
{
	CItem::r_Write(s);
	if ( m_bBuyFixed )
		s.WriteKeyVal( "BUYPRICE", GetBuyPrice());
	if ( m_bSellFixed )
		s.WriteKeyVal( "SELLPRICE", GetSellPrice());
	if ( GetQuality() != 0 )
		s.WriteKeyVal( "QUALITY", GetQuality());
	return;
}

DWORD CItemVendable::GetBuyPrice() const
{
	if ( GetPlayerVendorPrice() )
		return GetPlayerVendorPrice();
	else
		return m_buyprice;
}

DWORD CItemVendable::GetSellPrice() const
{
	if ( GetPlayerVendorPrice() )
		return GetPlayerVendorPrice();
	else
		return m_sellprice;
}

bool CItemVendable::IsValidNPCSaleItem() const
{
	// This item is in an NPC's vendor box.
	if ( m_sellprice == 0 )
		return( CItem::IsValidNPCSaleItem());
	else
		return( CItem::IsValidSaleItem( true ));
}