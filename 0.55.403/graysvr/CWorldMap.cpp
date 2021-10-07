//
// CWorldMap.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

//************************
// Natural resources.

CItem * CWorld::CheckNaturalResource( const CPointMap & pt, IT_TYPE Type, bool fTest, CChar * pCharSrc )
{
	// RETURN: 
	//  The resource tracking item.
	//  NULL = nothing here.

	if ( ! pt.IsValidPoint())
		return( NULL );

	// Check/Decrement natural resources at this location.
	// We create an invis object to time out and produce more.
	// RETURN: Quantity they wanted. 0 = none here.

	if ( fTest )	// Is the resource avail at all here ?
	{
		if ( ! g_World.IsItemTypeNear( pt, Type ))
			return( NULL );
	}

	// Find the resource object.
	CItem * pResBit;
	CWorldSearch Area(pt);
	while(true)
	{
		pResBit = Area.GetItem();
		if ( ! pResBit )
			break;
		// NOTE: ??? Not all resource objects are world gems. should they be ?
		// I wanted to make tree stumps etc be the resource block some day.

		if ( pResBit->IsType(Type) && pResBit->GetID() == ITEMID_WorldGem )
			break;
	}

	// If none then create one.
	if ( pResBit )
	{
		return( pResBit );
	}

	// What type of ore is here ?
	// NOTE: This is unrelated to the fact that we might not be skilled enough to find it.
	// Odds of a vein of ore being present are unrelated to my skill level.
	// Odds of my finding it are.
	// RES_REGIONRESOURCE from RES_REGIONTYPE linked to RES_AREA

	CRegionWorld* pRegion = dynamic_cast <CRegionWorld*>( pt.GetRegion( REGION_TYPE_AREA ));
	if ( pRegion == NULL )
	{
		DEBUG_CHECK(pRegion);
		return( NULL );
	}

	int iQty = pRegion->m_Events.GetCount();
	if ( iQty == 0 )
	{
		// just use the background (default) region for this
		CPointMap ptZero(0,0);
		pRegion = dynamic_cast <CRegionWorld*>( ptZero.GetRegion( REGION_TYPE_AREA ));
	}

	// Find RES_REGIONTYPE
	const CRandGroupDef * pResGroup = pRegion->FindNaturalResource( Type );
	if ( pResGroup == NULL )
	{
		return( NULL );
	}

	// Find RES_REGIONRESOURCE
	int	id	= pResGroup->GetRandMemberIndex( pCharSrc );
	CRegionResourceDef *	pOreDef;
	if ( id == -1 )
	{
		pOreDef	= dynamic_cast <CRegionResourceDef *> (g_Cfg.ResourceGetDefByName( RES_REGIONRESOURCE, "mr_nothing" ));
	}
	else
	{
		RESOURCE_ID rid	= pResGroup->GetMemberID( id );
		pOreDef		= dynamic_cast <CRegionResourceDef *>( g_Cfg.ResourceGetDef( rid ));
	}

	if ( pOreDef == NULL )
	{
		return( NULL );
	}

	pResBit = CItem::CreateScript( ITEMID_WorldGem, pCharSrc );
	ASSERT(pResBit);

	pResBit->SetType( Type );
	pResBit->SetAttr( ATTR_INVIS | ATTR_MOVE_NEVER );	// use the newbie flag just to show that it has just been created.
	pResBit->m_itResource.m_rid_res = pOreDef->GetResourceID();

	// Total amount of ore here.
	pResBit->SetAmount( pOreDef->m_Amount.GetRandom());

	pResBit->MoveToDecay( pt, pOreDef->m_iRegenerateTime * TICK_PER_SEC );	// Delete myself in this amount of time.

	CScriptTriggerArgs	Args( 0, 0, pResBit );
	if ( pOreDef->OnTrigger( "@ResourceFound", pCharSrc, &Args ) == TRIGRET_RET_TRUE )
	{
		if ( pResBit->IsDisconnected() )
			return NULL;
		pResBit->SetAmount( 0 );
	}
	return( pResBit );
}

//////////////////////////////////////////////////////////////////
// Map reading and blocking.

bool CWorld::IsItemTypeNear( const CPointMap & pt, IT_TYPE iType, int iDistance )
{
	if ( !pt.IsValidPoint() )
		return false;
	CPointMap ptn = FindItemTypeNearby( pt, iType, iDistance );
	return( ptn.IsValidPoint());
}

CPointMap CWorld::FindItemTypeNearby( const CPointMap & pt, IT_TYPE iType, int iDistance )
{
	// Find the closest item of this type.
	// This does not mean that i can touch it.
	// ARGS:
	//   iDistance = 2d distance to search.

	CPointMap ptFound;
	int iTestDistance;

		
	// Check dynamics first since they are the easiest.
	CWorldSearch Area( pt, iDistance );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			break;
		if ( ! pItem->IsType( iType ) &&
			! pItem->Item_GetDef()->IsType(iType) )
			continue;

		iTestDistance = pt.GetDist(pItem->GetTopPoint());
		if ( iTestDistance > iDistance )
			continue;

		ptFound = pItem->GetTopPoint();
		iDistance = iTestDistance;	// tighten up the search.
		if ( ! iDistance )
			return( ptFound );
	}

	// Check for appropriate terrain type
	CRectMap rect;
	CItemTypeDef	* pTypeDef	= NULL;
	rect.SetRect( pt.m_x - iDistance, pt.m_y - iDistance,
		pt.m_x + iDistance + 1, pt.m_y + iDistance + 1 );

	for (int x = rect.m_left; x < rect.m_right; x++ )
	{
		for ( int y = rect.m_top; y < rect.m_bottom; y++ )
		{
			CPointMap ptTest( x, y, pt.m_z, pt.m_mapplane );
			const CUOMapMeter * pMeter = GetMapMeter( ptTest );
			ASSERT(pMeter);
			ptTest.m_z = pMeter->m_z;

			if ( iType != g_World.GetTerrainItemType( pMeter->m_wTerrainIndex ) )
				continue;

			iTestDistance = pt.GetDist(ptTest);
			if ( iTestDistance > iDistance )
				break;

			ptFound = ptTest;
			iDistance = iTestDistance;	// tighten up the search.
			if ( ! iDistance )
				return( ptFound );
			rect.SetRect( pt.m_x - iDistance, pt.m_y - iDistance,
				pt.m_x + iDistance + 1, pt.m_y + iDistance + 1 );
		}
	}

	// Any statics here? (checks just 1 8x8 block !???)
	const CGrayMapBlock * pMapBlock = GetMapBlock( pt );
	ASSERT( pMapBlock );

	int iQty = pMapBlock->m_Statics.GetStaticQty();
	if ( iQty )  // no static items here.
	{
		int x2=pMapBlock->GetOffsetX(pt.m_x);
		int y2=pMapBlock->GetOffsetY(pt.m_y);

		for ( int i=0; i < iQty; i++ )
		{
			const CUOStaticItemRec * pStatic = pMapBlock->m_Statics.GetStatic( i );

			// inside the range we want ?
			CPointMap ptTest( pStatic->m_x+pMapBlock->m_x, pStatic->m_y+pMapBlock->m_y, pStatic->m_z, pt.m_mapplane );
			iTestDistance = pt.GetDist(ptTest);
			if ( iTestDistance > iDistance )
				continue;

			ITEMID_TYPE idTile = pStatic->GetDispID();

			// Check the script def for the item.
			const CItemBase * pItemDef = CItemBase::FindItemBase( idTile );
			if ( pItemDef == NULL )
				continue;
			if ( ! pItemDef->IsType( iType ))
				continue;
			ptFound = ptTest;
			iDistance = iTestDistance;
			if ( ! iDistance )
				return( ptFound );
		}
	}
	// Parts of multis ?

	return ptFound;
}

//****************************************************

#if 0

static int GetHeightTerrain( const CPointMap & pt, int z, const CGrayMapBlock * pMapBlock )
{
	// Stupid terrain is an average of the stuff around it?!
	for ( int i=1; i<4; i++ )
	{
		DIR_TYPE DirCheck;
		switch ( i )
		{
		case 1:
			DirCheck = DIR_E; 
			break;
		case 2:	
			DirCheck = DIR_SE;
			break;
		case 3:	
			DirCheck = DIR_S;
			break;
		}

		CPointMap ptTest = pt;
		ptTest.Move( DirCheck );

		const CUOMapMeter * pMeter = pMapBlock->GetTerrain( UO_BLOCK_OFFSET(ptTest.m_x), UO_BLOCK_OFFSET(ptTest.m_y));
		ASSERT(pMeter);


	}
}

#endif

void CWorld::GetHeightPoint( const CPointMap & pt, CGrayMapBlockState & block, /*const CRegionBase * pRegion*/ bool fHouseCheck )
{
	// Height of statics at/above given coordinates
	// do gravity here for the z.

	WORD wBlockThis = 0;
	const CGrayMapBlock * pMapBlock = GetMapBlock( pt );
	ASSERT(pMapBlock);

	{
	int iQty = pMapBlock->m_Statics.GetStaticQty();
	if ( iQty )  // no static items here.
	{
		int x2=pMapBlock->GetOffsetX(pt.m_x);
		int y2=pMapBlock->GetOffsetY(pt.m_y);
		for ( int i=0; i<iQty; i++ )
		{
			if ( ! pMapBlock->m_Statics.IsStaticPoint( i, x2, y2 ))
				continue;
			const CUOStaticItemRec * pStatic = pMapBlock->m_Statics.GetStatic( i );
			signed char z = pStatic->m_z;
			if ( ! block.IsUsableZ(z,PLAYER_HEIGHT))
				continue;

			// This static is at the coordinates in question.
			// enough room for me to stand here ?
			wBlockThis = 0;
			signed char zHeight = CItemBase::GetItemHeight( pStatic->GetDispID(), wBlockThis );
			block.CheckTile( wBlockThis, z, zHeight, pStatic->GetDispID() + TERRAIN_QTY );
		}
	}
	}

	// Any multi items here ?
	if ( fHouseCheck )
	{
		CRegionLinks rlinks;
		if ( int iQty = pt.GetRegions( REGION_TYPE_MULTI, rlinks ) )
		{
			for ( int i = 0; i < iQty; i++)
			{
				CRegionBase * pRegion = rlinks.GetAt(i);
				CItem * pItem = pRegion->GetResourceID().ItemFind();
				if ( pItem != NULL )
				{
					const CGrayMulti * pMulti = g_Cfg.GetMultiItemDefs( pItem->GetDispID());
					if ( pMulti )
					{
						int x2 = pt.m_x - pItem->GetTopPoint().m_x;
						int y2 = pt.m_y - pItem->GetTopPoint().m_y;

						int iQty = pMulti->GetItemCount();
						for ( int i=0; iQty--; i++ )
						{
							const CUOMultiItemRec * pMultiItem = pMulti->GetItem(i);
							ASSERT(pMultiItem);

							if ( ! pMultiItem->m_visible )
								continue;
							if ( pMultiItem->m_dx != x2 || pMultiItem->m_dy != y2 )
								continue;

							signed char zitem = pItem->GetTopZ() + pMultiItem->m_dz;
							if ( ! block.IsUsableZ(zitem,PLAYER_HEIGHT))
								continue;

							wBlockThis = 0;
							signed char zHeight = CItemBase::GetItemHeight( pMultiItem->GetDispID(), wBlockThis );
							block.CheckTile( wBlockThis, zitem, zHeight, pMultiItem->GetDispID() + TERRAIN_QTY );
						}
					}
				}
			}
		}
	}

	{
	// Any dynamic items here ?
	// NOTE: This could just be an item that an NPC could just move ?
	CWorldSearch Area( pt );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			break;

		signed char zitem = pItem->GetTopZ();
		if ( ! block.IsUsableZ(zitem,PLAYER_HEIGHT))
			continue;

		// Invis items should not block ???
		CItemBase * pItemDef = pItem->Item_GetDef();
		ASSERT(pItemDef);
		if ( !block.CheckTile( 
			pItemDef->m_Can & ( CAN_I_DOOR | CAN_I_WATER | CAN_I_CLIMB | CAN_I_BLOCK | CAN_I_PLATFORM ),
			zitem, pItemDef->GetHeight(), pItemDef->GetDispID() + TERRAIN_QTY ) )
		{
		}
	}
	}

	// Check Terrain here.
	// Terrain height is screwed. Since it is related to all the terrain around it.

	{
	const CUOMapMeter * pMeter = pMapBlock->GetTerrain( UO_BLOCK_OFFSET(pt.m_x), UO_BLOCK_OFFSET(pt.m_y));
	ASSERT(pMeter);

	if ( block.IsUsableZ(pMeter->m_z,0))
	{
		if ( pMeter->m_wTerrainIndex == TERRAIN_HOLE )
			wBlockThis = 0;
		else if ( pMeter->m_wTerrainIndex == TERRAIN_NULL )	// inter dungeon type.
			wBlockThis = CAN_I_BLOCK;
		else
		{
			CGrayTerrainInfo land( pMeter->m_wTerrainIndex );
			if ( land.m_flags & UFLAG1_WATER )
				wBlockThis = CAN_I_WATER;
			else if ( land.m_flags & UFLAG1_DAMAGE )
				wBlockThis = CAN_I_FIRE;
			else if ( land.m_flags & UFLAG1_BLOCK )
				wBlockThis = CAN_I_BLOCK;
			else
				wBlockThis = CAN_I_PLATFORM;
		}
		block.CheckTile( wBlockThis, pMeter->m_z, 0, pMeter->m_wTerrainIndex );
	}
	}

	if ( block.m_Bottom.m_z == UO_SIZE_MIN_Z )
	{
		block.m_Bottom = block.m_Lowest;
	}

#ifdef _DEBUG
	if ( g_Cfg.m_wDebugFlags & 0x08 )
	{
	CGString sMsg;
	sMsg.Format( "New space from z=%d is over(z=%d,n='%s',b=0%x), under(z=%d,n='%s',b=0%x)\n", 
		block.m_z, 

		block.m_Bottom.m_z,	// the z we would stand on,
		(LPCTSTR) CGrayMapBlockState::GetTileName(block.m_Bottom.m_wTile),
		block.m_Bottom.m_dwBlockFlags, // The bottom item has these blocking flags.

		block.m_Top.m_z,	// the z we would stand on,
		(LPCTSTR) CGrayMapBlockState::GetTileName(block.m_Top.m_wTile),
		block.m_Top.m_dwBlockFlags // The bottom item has these blocking flags.

		);
	DEBUG_MSG(( sMsg ));
	}
#endif
}

signed char CWorld::GetHeightPoint( const CPointBase & pt, WORD & wBlockFlags, /*const CRegionBase * pRegion*/ bool fHouseCheck ) // Height of player who walked to X/Y/OLDZ
{
	// Given our coords at pt including pt.m_z
	// What is the height that gravity would put me at should i step hear ?
	// Assume my head height is PLAYER_HEIGHT/2
	// ARGS:
	//  pt = the point of interest.
	//  pt.m_z = my starting altitude.
	//  wBlockFlags = what we can pass thru. doors, water, walls ?
	//		CAN_C_GHOST	= Moves thru doors etc.	- CAN_I_DOOR						
	//		CAN_C_SWIM = walk thru water - CAN_I_WATER
	//		CAN_C_WALK = it is possible that i can step up. - CAN_I_PLATFORM
	//		CAN_C_PASSWALLS	= walk through all blocking items - CAN_I_BLOCK
	//		CAN_C_FLY  = gravity does not effect me. - CAN_I_CLIMB
	//		CAN_C_FIRE_IMMUNE = i can walk into lava etc. - CAN_I_FIRE
	//  pRegion = possible regional effects. (multi's)
	// RETURN:
	//  pt.m_z = Our new height at pt.m_x,pt.m_y
	//  wBlockFlags = our blocking flags at the given location. CAN_I_WATER,CAN_C_WALK,CAN_FLY,CAN_SPIRIT,
	//    CAN_C_INDOORS = i am covered from the sky
	//

	// ??? NOTE: some creatures should be taller than others !!!

	WORD wCan = wBlockFlags;
	CGrayMapBlockState block( wBlockFlags, pt.m_z, PLAYER_HEIGHT );

	GetHeightPoint( pt, block, fHouseCheck );

	// Pass along my results.
	wBlockFlags = block.m_Bottom.m_dwBlockFlags;
	if ( block.m_Top.m_dwBlockFlags )
		wBlockFlags |= CAN_I_ROOF;	// we are covered by something.

	if (( wBlockFlags & ( CAN_I_CLIMB|CAN_I_PLATFORM) ) && ( wCan & CAN_C_WALK ))
	{
		wBlockFlags &= ~CAN_I_CLIMB;
		wBlockFlags |= CAN_I_PLATFORM;	// not really true but hack it anyhow.
		return( block.m_Bottom.m_z );
	}

	if ( wCan & CAN_C_FLY )
		return( pt.m_z );

	return( block.m_Bottom.m_z );
}

void CWorld::GetHeightPoint_New( const CPointMap & pt, CGrayMapBlockState & block, bool fHouseCheck )
{
	// Height of statics at/above given coordinates
	// do gravity here for the z.

	WORD wBlockThis = 0;
	const CGrayMapBlock * pMapBlock = GetMapBlock( pt );
	ASSERT(pMapBlock);

	{
	int iQty = pMapBlock->m_Statics.GetStaticQty();
	if ( iQty )  // no static items here.
	{
		int x2=pMapBlock->GetOffsetX(pt.m_x);
		int y2=pMapBlock->GetOffsetY(pt.m_y);
		for ( int i=0; i<iQty; i++ )
		{
			if ( ! pMapBlock->m_Statics.IsStaticPoint( i, x2, y2 ))
				continue;
			const CUOStaticItemRec * pStatic = pMapBlock->m_Statics.GetStatic( i );
			signed char z = pStatic->m_z;
			if ( ! block.IsUsableZ(z,PLAYER_HEIGHT))
				continue;
			// This static is at the coordinates in question.
			// enough room for me to stand here ?
			wBlockThis = 0; // item CAN flags returned here
			signed char zHeight = CItemBase::GetItemHeight( pStatic->GetDispID(), wBlockThis );
			block.CheckTile_Item( wBlockThis, z, zHeight, pStatic->GetDispID() + TERRAIN_QTY );
		}
	}
	}

	// Any multi items here ?
	// Check all of them
	if ( fHouseCheck )
	{
		CRegionLinks rlinks;
		if ( int iQty = pt.GetRegions( REGION_TYPE_MULTI, rlinks ) )
		{
			for ( int i = 0; i < iQty; i++)
			{
				CRegionBase * pRegion = rlinks.GetAt(i);
				CItem * pItem = pRegion->GetResourceID().ItemFind();
				if ( pItem != NULL )
				{
					const CGrayMulti * pMulti = g_Cfg.GetMultiItemDefs( pItem->GetDispID());
					if ( pMulti )
					{
						int x2 = pt.m_x - pItem->GetTopPoint().m_x;
						int y2 = pt.m_y - pItem->GetTopPoint().m_y;

						int iQty = pMulti->GetItemCount();
						for ( int i=0; iQty--; i++ )
						{
							const CUOMultiItemRec * pMultiItem = pMulti->GetItem(i);
							ASSERT(pMultiItem);

							if ( ! pMultiItem->m_visible )
								continue;
							if ( pMultiItem->m_dx != x2 || pMultiItem->m_dy != y2 )
								continue;

							signed char	zitem = pItem->GetTopZ() + pMultiItem->m_dz;
							if ( ! block.IsUsableZ(zitem,PLAYER_HEIGHT))
								continue;

							wBlockThis = 0;
							signed char zHeight = CItemBase::GetItemHeight( pMultiItem->GetDispID(), wBlockThis );
							block.CheckTile_Item( wBlockThis, zitem, zHeight, pMultiItem->GetDispID() + TERRAIN_QTY );
						}
					}
				}
			}
		}
	}

	{
	// Any dynamic items here ?
	// NOTE: This could just be an item that an NPC could just move ?
	CWorldSearch Area( pt );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL )
			break;

		signed char zitem = pItem->GetTopZ();
		if ( ! block.IsUsableZ(zitem,PLAYER_HEIGHT))
			continue;

		// Invis items should not block ???
		CItemBase * pItemDef = pItem->Item_GetDef();
		ASSERT(pItemDef);

		if ( !block.CheckTile_Item( 
			pItemDef->m_Can & ( CAN_I_DOOR | CAN_I_WATER | CAN_I_CLIMB | CAN_I_BLOCK | CAN_I_PLATFORM ),
			zitem, pItemDef->GetHeight(), pItemDef->GetDispID() + TERRAIN_QTY ) )
		{
		}
	}
	}

	// Terrain height is screwed. Since it is related to all the terrain around it.

	{
	const CUOMapMeter * pMeter = pMapBlock->GetTerrain( UO_BLOCK_OFFSET(pt.m_x), UO_BLOCK_OFFSET(pt.m_y));
	ASSERT(pMeter);

	if ( block.IsUsableZ( pMeter->m_z,0 ) )
	{
		if ( pMeter->m_wTerrainIndex == TERRAIN_HOLE )
			wBlockThis = 0;
		else if ( pMeter->m_wTerrainIndex == TERRAIN_NULL )	// inter dungeon type.
			wBlockThis = CAN_I_BLOCK;
		else
		{
			CGrayTerrainInfo land( pMeter->m_wTerrainIndex );
			if ( land.m_flags & UFLAG1_WATER )
				wBlockThis = CAN_I_WATER;
			else if ( land.m_flags & UFLAG1_DAMAGE )
				wBlockThis = CAN_I_FIRE;
			else if ( land.m_flags & UFLAG1_BLOCK )
				wBlockThis = CAN_I_BLOCK;
			else
				wBlockThis = CAN_I_PLATFORM;
		}
		block.CheckTile_Terrain( wBlockThis, pMeter->m_z, pMeter->m_wTerrainIndex );
	}
	}

	if ( block.m_Bottom.m_z == UO_SIZE_MIN_Z )
	{
		block.m_Bottom = block.m_Lowest;
	}

#ifdef _DEBUG
	if ( g_Cfg.m_wDebugFlags & 0x08 )
	{
	CGString sMsg;
	sMsg.Format( "New space from z=%d is over(z=%d,n='%s',b=0%x), under(z=%d,n='%s',b=0%x)\n", 
		block.m_z, 

		block.m_Bottom.m_z,	// the z we would stand on,
		(LPCTSTR) CGrayMapBlockState::GetTileName(block.m_Bottom.m_wTile),
		block.m_Bottom.m_dwBlockFlags, // The bottom item has these blocking flags.

		block.m_Top.m_z,	// the z we would stand on,
		(LPCTSTR) CGrayMapBlockState::GetTileName(block.m_Top.m_wTile),
		block.m_Top.m_dwBlockFlags // The bottom item has these blocking flags.

		);
	DEBUG_MSG(( sMsg ));
	}
#endif
}

signed char CWorld::GetHeightPoint_New( const CPointBase & pt, WORD & wBlockFlags, bool fHouseCheck )
{
	WORD wCan = wBlockFlags;
	CGrayMapBlockState block( wBlockFlags, pt.m_z + (PLAYER_HEIGHT / 2), pt.m_z + PLAYER_HEIGHT );

	GetHeightPoint_New( pt, block, fHouseCheck );

	// Pass along my results.
	wBlockFlags = block.m_Bottom.m_dwBlockFlags;
	if ( block.m_Top.m_dwBlockFlags )
		wBlockFlags |= CAN_I_ROOF;	// we are covered by something.
#if 1
	if (( wBlockFlags & ( CAN_I_CLIMB|CAN_I_PLATFORM) ) && ( wCan & CAN_C_WALK ))
	{
		wBlockFlags &= ~CAN_I_CLIMB;
		wBlockFlags |= CAN_I_PLATFORM;	// not really true but hack it anyhow.
		return( block.m_Bottom.m_z );
	}
#endif
	if ( wCan & CAN_C_FLY )
		return( pt.m_z );

	return( block.m_Bottom.m_z );
}


CItemTypeDef *	CWorld::GetTerrainItemTypeDef( DWORD dwTerrainIndex )
{
	CResourceDef *	pRes	= NULL;

	if ( g_World.m_TileTypes.IsValidIndex( dwTerrainIndex ) )
	{
		pRes	= g_World.m_TileTypes[dwTerrainIndex];
	}
		
	if ( !pRes )
	{
		RESOURCE_ID	rid( RES_TYPEDEF, 0 );
		pRes	= g_Cfg.ResourceGetDef( rid );
	}
	ASSERT( pRes );

	CItemTypeDef *	pItemTypeDef	= dynamic_cast <CItemTypeDef*> (pRes);
	ASSERT( pItemTypeDef );

	return( pItemTypeDef );
}


IT_TYPE		CWorld::GetTerrainItemType( DWORD dwTerrainIndex )
{
	CResourceDef *	pRes	= NULL;

	if ( g_World.m_TileTypes.IsValidIndex( dwTerrainIndex ) )
	{
		pRes	= g_World.m_TileTypes[dwTerrainIndex];
	}
		
	if ( !pRes )
		return IT_NORMAL;

	CItemTypeDef *	pItemTypeDef	= dynamic_cast <CItemTypeDef*> (pRes);
	ASSERT( pItemTypeDef );

	return (IT_TYPE) pItemTypeDef->GetItemType();
}
