//
// CSectorTemplate.cpp
// Copyright Menace Software (www.menasoft.com).
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#else
#include "graycom.h"
#include "graymul.h"
#include "cregion.h"
#endif

#ifdef GRAY_SVR

////////////////////////////////////////////////////////////////////////
// -CCharsActiveList

void CCharsActiveList::OnRemoveOb( CGObListRec * pObRec )
{
	// Override this = called when removed from group.
	CChar * pChar = STATIC_CAST <CChar*>(pObRec);
	ASSERT( pChar );
	DEBUG_CHECK( pChar->IsTopLevel());
	if ( pChar->IsClient())
	{
		ClientDetach();
		m_timeLastClient = CServTime::GetCurrentTime();	// mark time in case it's the last client
	}
	CGObList::OnRemoveOb(pObRec);
	DEBUG_CHECK( pChar->GetParent() == NULL );
	pChar->SetContainerFlags(UID_O_DISCONNECT);
}

void CCharsActiveList::AddCharToSector( CChar * pChar )
{
	ASSERT( pChar );
	// ASSERT( pChar->m_pt.IsValid());
	if ( pChar->IsClient())
	{
		ClientAttach();
	}
	CGObList::InsertHead(pChar);
}

//////////////////////////////////////////////////////////////
// -CItemList

bool CItemsList::sm_fNotAMove = false;

void CItemsList::OnRemoveOb( CGObListRec * pObRec )
{
	// Item is picked up off the ground. (may be put right back down though)
	CItem * pItem = STATIC_CAST <CItem*>(pObRec);
	ASSERT( pItem );
	DEBUG_CHECK( pItem->IsTopLevel());
	DEBUG_CHECK( pItem->GetTopPoint().IsValidPoint());

	if ( ! sm_fNotAMove )
	{
		pItem->OnMoveFrom();	// IT_MULTI, IT_SHIP and IT_COMM_CRYSTAL
	}

	CGObList::OnRemoveOb(pObRec);
	DEBUG_CHECK( pItem->GetParent() == NULL );
	pItem->SetContainerFlags(UID_O_DISCONNECT);	// It is no place for the moment.
}

void CItemsList::AddItemToSector( CItem * pItem )
{
	// Add to top level.
	// Either MoveTo() or SetTimeout is being called.
	ASSERT( pItem );
	CGObList::InsertHead( pItem );
}

#endif

//////////////////////////////////////////////////////////////////
// -CSectorTemplate

int CSectorTemplate::GetIndex() const
{
	CSectorTemplate* pSectorZero = g_World.GetSector(0);
	int i = (((BYTE*)this) - ((BYTE*)pSectorZero));
	i /= sizeof(CSector);
	ASSERT( i>=0 && i < SECTOR_QTY );
	return( i );
}

void CSectorTemplate::CheckMapBlockCache( int iTime )
{
	// Clean out the sectors map cache if it has not been used recently.
	// iTime == 0 = delete all.

	int iQty = m_MapBlockCache.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		CGrayMapBlock * pMapBlock = STATIC_CAST <CGrayMapBlock *>(m_MapBlockCache[i]);
		ASSERT(pMapBlock);
		if ( iTime <= 0 || pMapBlock->m_CacheTime.GetCacheAge() >= iTime )
		{
			m_MapBlockCache.DeleteAt(i);
			i--;
			iQty--;
		}
	}
}

const CGrayMapBlock * CSectorTemplate::GetMapBlock( const CPointMap & pt )
{
	// Get a map block from the cache. load it if not.

	
	ASSERT( pt.IsValidXY());
	CPointMap pntBlock( UO_BLOCK_ALIGN(pt.m_x), UO_BLOCK_ALIGN(pt.m_y));
	ASSERT( m_MapBlockCache.GetCount() <= (UO_BLOCK_SIZE * UO_BLOCK_SIZE));

	CGrayMapBlock * pMapBlock;

	// Find it in cache.
	int i = m_MapBlockCache.FindKey( pntBlock.GetPointSortIndex());
	if ( i >= 0 )
	{
		pMapBlock = STATIC_CAST <CGrayMapBlock *>(m_MapBlockCache[i]);
		ASSERT(pMapBlock);
		pMapBlock->m_CacheTime.HitCacheTime();
		return( pMapBlock );
	}

	// else load it.
	try
	{
		pMapBlock = new CGrayMapBlock( pntBlock );
		ASSERT(pMapBlock);
	}
	catch (CGrayError & e)
	{
		return( NULL );
	}
	catch (...)
	{
		return( NULL );
	}

	// Add it to the cache.
	m_MapBlockCache.AddSortKey( pMapBlock, pntBlock.GetPointSortIndex() );
	return( pMapBlock );
}

bool CSectorTemplate::IsInDungeon() const
{
	// ??? in the future make this part of the region info !
	// What part of the maps are filled with dungeons.
	// Used for light / weather calcs.

	CPointMap pt = GetBasePoint();

	int x1=(pt.m_x-UO_SIZE_X_REAL);
	if ( x1 < 0 )
		return( false );

	x1 /= 256;
	switch ( pt.m_y / 256 )
	{
	case 0:
	case 5:
		return( true );
	case 1:
		if (x1!=0)
			return( true );
		break;
	case 2:
	case 3:
		if (x1<3)
			return( true );
		break;
	case 4:
	case 6:
		if (x1<1)
			return( true );
		break;
	case 7:
		if (x1<2)
			return( true );
		break;
	}
	return( false );
}

CRegionBase * CSectorTemplate::GetRegion( const CPointBase & pt, DWORD dwType ) const
{
	// Does it match the mask of types we care about ?
	// Assume sorted so that the smallest are first.
	//
	// REGION_TYPE_AREA => RES_AREA = World region area only = CRegionWorld
	// REGION_TYPE_ROOM => RES_ROOM = NPC House areas only = CRegionBase.
	// REGION_TYPE_MULTI => RES_WORLDITEM = UID linked types in general = CRegionWorld

	int iQty = m_RegionLinks.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		CRegionBase * pRegion = m_RegionLinks[i];
		ASSERT(pRegion);

#ifdef GRAY_SVR
		ASSERT( pRegion->GetResourceID().IsValidUID());
		if ( pRegion->GetResourceID().IsItem())
		{
			if ( ! ( dwType & REGION_TYPE_MULTI ))
				continue;
		}
		else if ( pRegion->GetResourceID().GetResType() == RES_AREA )
		{
			if ( ! ( dwType & REGION_TYPE_AREA ))
				continue;
		}
		else
		{
			if ( ! ( dwType & REGION_TYPE_ROOM ))
				continue;
		}
#else
		if ( ! pRegion->IsMatchType(dwType))
			continue;
#endif

		if ( ! pRegion->m_pt.IsSameMapPlane( pt.m_mapplane ))
			continue;
		if ( ! pRegion->IsInside2d( pt ))
			continue;
		return( pRegion );
	}
	return( NULL );
}

// Balkon: get regions list (to cicle through intercepted house regions)
int CSectorTemplate::GetRegions( const CPointBase & pt, DWORD dwType, CRegionLinks & rlist ) const
{
	int iQty = m_RegionLinks.GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		CRegionBase * pRegion = m_RegionLinks[i];
		ASSERT(pRegion);

#ifdef GRAY_SVR
		ASSERT( pRegion->GetResourceID().IsValidUID());
		if ( pRegion->GetResourceID().IsItem())
		{
			if ( ! ( dwType & REGION_TYPE_MULTI ))
				continue;
		}
		else if ( pRegion->GetResourceID().GetResType() == RES_AREA )
		{
			if ( ! ( dwType & REGION_TYPE_AREA ))
				continue;
		}
		else
		{
			if ( ! ( dwType & REGION_TYPE_ROOM ))
				continue;
		}
#else
		if ( ! pRegion->IsMatchType(dwType))
			continue;
#endif

		if ( ! pRegion->m_pt.IsSameMapPlane( pt.m_mapplane ))
			continue;
		if ( ! pRegion->IsInside2d( pt ))
			continue;
		rlist.Add( pRegion );
	}
	return( rlist.GetCount() );
}

bool CSectorTemplate::UnLinkRegion( CRegionBase * pRegionOld )
{
	// NOTE: What about unlinking it from all the CChar(s) here ? m_pArea
	ASSERT(pRegionOld);
	return m_RegionLinks.RemovePtr(pRegionOld);
}

bool CSectorTemplate::LinkRegion( CRegionBase * pRegionNew )
{
	// link in a region. may have just moved !
	// Make sure the smaller regions are first in the array !
	// Later added regions from the MAP file should be the smaller ones, 
	//  according to the old rules.

	ASSERT(pRegionNew);
	ASSERT( pRegionNew->IsOverlapped( GetRect()));
	int iQty = m_RegionLinks.GetCount();

#if 0 //def _DEBUG
	if ( iQty >= 2 ) // || ! strcmpi( pRegionNew->GetName(), "Britain Territory" ))
	{
		DEBUG_CHECK( iQty>=1 );
	}
#endif

	for ( int i=0; i<iQty; i++ )
	{
		CRegionBase * pRegion = m_RegionLinks[i];
		ASSERT(pRegion);
		if ( pRegionNew == pRegion )
		{
			DEBUG_ERR(( "region already linked!\n" ));
			return false;
		}

		if ( pRegion->IsOverlapped(pRegionNew))
		{
			// NOTE : We should use IsInside() but my version isn't completely accurate for it's FALSE return
			if ( pRegion->IsEqualRegion( pRegionNew ))
			{
				DEBUG_ERR(( "Conflicting region!\n" ));
				return( false );
			}
			if ( pRegionNew->IsInside(pRegion))	// it is accurate in the TRUE case.
				continue;

			// must insert before this.
			m_RegionLinks.InsertAt( i, pRegionNew );
			return( true );
		}

		DEBUG_CHECK( iQty >= 1 );
	}

	m_RegionLinks.Add( pRegionNew );
	return( true );
}

CTeleport * CSectorTemplate::GetTeleport2d( const CPointMap & pt ) const
{
	// Any teleports here at this point ?

	int i = m_Teleports.FindKey( pt.GetPointSortIndex());
	if ( i < 0 )
		return( NULL );
	return STATIC_CAST <CTeleport *>( m_Teleports[i]);
}

CTeleport * CSectorTemplate::GetTeleport( const CPointMap & pt ) const
{
	// Any teleports here at this point ?

	CTeleport * pTeleport = GetTeleport2d( pt );
	if ( pTeleport == NULL )
		return( NULL );

	int zdiff = pt.m_z - pTeleport->m_z;	
	if ( abs(zdiff) > 5 )
		return( NULL );

	// Check m_mapplane ?
	if ( ! pTeleport->IsSameMapPlane( pt.m_mapplane ))
		return( NULL );

	return( pTeleport );
}

bool CSectorTemplate::AddTeleport( CTeleport * pTeleport )
{
	// NOTE: can't be 2 teleports from the same place !
	// ASSERT( Teleport is actually in this sector !

	int i = m_Teleports.FindKey( pTeleport->GetPointSortIndex());
	if ( i >= 0 )
	{
		DEBUG_ERR(( "Conflicting teleport %s!\n", pTeleport->WriteUsed() ));
		return( false );
	}
	m_Teleports.AddSortKey( pTeleport, pTeleport->GetPointSortIndex());
	return( true );
}

