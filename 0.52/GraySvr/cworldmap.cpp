//
// CWorldMap.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.

//////////////////////////////////////////////////////////////////
// -CTeleport

CTeleport::CTeleport( TCHAR ** ppCmd, int iArgs )
{
	// Assume valid iArgs >= 5
	m_Src = CPointMap( atoi( ppCmd[0] ), atoi( ppCmd[1] ), atoi( ppCmd[2] ));

	if ( iArgs <= 5 )
	{
		m_Src.m_z = 0;
		ppCmd = &ppCmd[2];
	}
	else
	{
		ppCmd = &ppCmd[3];
	}

	m_Dst = CPointMap( atoi( ppCmd[0] ), atoi( ppCmd[1] ), atoi( ppCmd[2] ));

	if ( ! m_Src.IsValid() || ! m_Dst.IsValid())
	{
		DEBUG_ERR(( "CTeleport bad coords %s\n", m_Src.Write() ));
		return;
	}

	m_Src.GetSector()->AddTeleport( this );
}

const CGrayMulti * CWorld::GetMultiItemDefs( ITEMID_TYPE itemid )
{
	if ( ! CItemBase::IsMultiID(itemid))
		return( NULL );

	MULTI_TYPE id = itemid - ITEMID_MULTI;
	int index = m_MultiDefs.FindKey( id );
	if ( index < 0 )
	{
		index = m_MultiDefs.AddSortKey( new CGrayMulti( id ), id );
	}
	else
	{
		m_MultiDefs[index]->HitCacheTime();
	}
	const CGrayMulti * pMulti = m_MultiDefs[index];
	ASSERT(pMulti);
	return( pMulti );
}

//////////////////////////////////////////////////////////////////
// Map reading and blocking.

CPointMap CWorld::FindItemTypeNearby( const CPointMap & pt, ITEM_TYPE iType, int iDistance )
{
	// Find the closest item of this type.

	CPointMap ptPos;

	// Check dynamics first since they are the easiest.
	CWorldSearch Area( pt, iDistance );
	while (true)
	{
		CItem * pItem = Area.GetItem();
		if ( pItem == NULL ) 
			break;
		if ( pItem->m_pDef->m_type != iType ) 
			continue;

		int iTestDistance = pt.GetDist(pItem->GetTopPoint());
		if ( iTestDistance > iDistance ) 
			continue;

		ptPos = pItem->GetTopPoint();
		iDistance = iTestDistance;	// tighten up the search.
		if ( ! iDistance ) 
			return( ptPos );
	}

	// Check for appropriate terrain type
	CRectMap rect;
	rect.SetRect( pt.m_x - iDistance, pt.m_y - iDistance,
		pt.m_x + iDistance, pt.m_y + iDistance );

	for (int x = rect.m_left; x <= rect.m_right; x++ )
	{
		for ( int y = rect.m_top; y <= rect.m_bottom; y++ )
		{
			CPointMap ptTest( x, y );
			const CUOMapMeter * pMeter = GetMapMeter( ptTest );
			ASSERT(pMeter);
			ptTest.m_z = pMeter->m_z;

			switch (iType)
			{
			case ITEM_ROCK:
				if ( pMeter->IsTerrainRock())
				{
checkterrain:
					int iTestDistance = pt.GetDist(ptTest);
					if ( iTestDistance > iDistance ) 
						break;

					ptPos = ptTest;
					iDistance = iTestDistance;	// tighten up the search.
					if ( ! iDistance ) 
						return( ptPos );

					rect.SetRect( pt.m_x - iDistance, pt.m_y - iDistance,
						pt.m_x + iDistance, pt.m_y + iDistance );
				}
				break;
			case ITEM_WATER:
				if ( pMeter->IsTerrainWater())
					goto checkterrain;
				break;
			case ITEM_GRASS:
				if ( pMeter->IsTerrainGrass())
					goto checkterrain;
				break;
			}
		}
	}

	// Any statics here? (checks just 1 8x8 block !???)
	const CGrayMapBlock * pMapBlock = GetMapBlock( pt );
	ASSERT( pMapBlock );

	if ( pMapBlock->GetStaticQty())  // no static items here.
	{
		int x2=pMapBlock->GetOffsetX(pt.m_x);
		int y2=pMapBlock->GetOffsetY(pt.m_y);

		int iQty = pMapBlock->GetStaticQty();
		for ( int i=0; i < iQty; i++ )
		{
			const CUOStaticItemRec * pStatic = pMapBlock->GetStatic( i );

			// inside the range we want ?
			CPointMap ptTest( pStatic->m_x+pMapBlock->m_p.m_x, pStatic->m_y+pMapBlock->m_p.m_y, pStatic->m_z );
			int iTestDistance = pt.GetDist(ptTest);
			if ( iTestDistance > iDistance ) 
				continue;

			ITEMID_TYPE idTile = (ITEMID_TYPE)pStatic->m_wTileID;
			switch (iType)
			{
			case ITEM_ANVIL:
				if ( idTile == ITEMID_ANVIL1 || idTile == ITEMID_ANVIL2 )
				{
checkstatic:
					ptPos = ptTest;
					iDistance = iTestDistance;
					if ( ! iDistance )
						return( ptPos );
				}
				break;
			case ITEM_FIRE:
				// Any fire type of item.	( for cooking ?)

			case ITEM_FORGE:
				if (CItemBase::IsForgeID(idTile))
					goto checkstatic;
				break;
			case ITEM_TREE:
				if (CItemBase::IsTreeID(idTile))
					goto checkstatic;
				break;
			case ITEM_ROCK:
				if (CItemBase::IsRockID(idTile))
					goto checkstatic;
				break;
			case ITEM_WATER:
				if (CItemBase::IsWaterFishID(idTile))
					goto checkstatic;
				break;
			case ITEM_GRASS:
				if (CItemBase::IsGrassID(idTile))
					goto checkstatic;
				break;
			}
		}
	}

	return ptPos;
}

signed char CWorld::GetHeight( const CPointBase & pt, WORD & wBlockFlags, const CRegionBase * pRegion ) // Height of player who walked to X/Y/OLDZ
{
	// Given our coords at pt.m_z
	// What is the height that gravity would put me at should i step hear ?
	// Assume that it is possible that i can step up.
	// ARGS:
	//  pt = the point of interest.
	//  pt.m_z = my starting altitude.
	//  wBlockFlags = what we can pass thru.
	//  pRegion = possible regional effects. (multi's)
	// RETURN:
	//  pt.m_z = Our new height at pt.m_x,pt.m_y
	//  wBlockFlags = our blocking flags at the given location. CAN_I_WATER,CAN_C_WALK,CAN_FLY,CAN_SPIRIT,
	//

	signed char ztry = -UO_SIZE_Z;
	bool fRoof = false;

	// ??? NOTE: some creatures should be taller than others !!!
	signed char zmyhead = pt.m_z + PLAYER_HEIGHT/2;	// what height is my head at ?

	// Height of statics at/above given coordinates
	// do gravity here for the z.

	WORD wBlockRes = 0;
	const CGrayMapBlock * pMapBlock = GetMapBlock( pt );
	ASSERT(pMapBlock);

	int iQty = pMapBlock->GetStaticQty();
	if ( iQty )  // no static items here.
	{
		int x2=pMapBlock->GetOffsetX(pt.m_x);
		int y2=pMapBlock->GetOffsetY(pt.m_y);
		for ( int i=0; i<iQty; i++ )
		{
			if ( ! pMapBlock->IsStaticPoint( i, x2, y2 ))
				continue;
			const CUOStaticItemRec * pStatic = pMapBlock->GetStatic( i );
			signed char z = pStatic->m_z;
			if ( z > zmyhead )
			{
				fRoof = true;
				continue;	// over my head so ignore it
			}
			if ( z < ( pt.m_z - PLAYER_HEIGHT ))
				continue;	// Don't have to check TOO far down under us.

			// This static is at the coordinates in question.
			// enough room for me to stand here ?
			WORD wBlockThis = 0;
			signed char ztop = z + CItemBase::GetItemHeight( (ITEMID_TYPE) pStatic->m_wTileID, wBlockThis );
			if ( ztop < ztry ) continue;	// under something else. (ignore it)
			if ( ! wBlockThis ) continue;	// doesn't block me in any way
			if ( wBlockThis & CAN_I_PLATFORM )
			{
				if ( wBlockThis == CAN_I_PLATFORM && ztop > zmyhead ) continue; // ignore platforms that i can walk under.
			}
			else
			{
				if ( ztop <= ztry )	continue;	// always take the platform in a tie
			}
			ztry = ztop;	// step on it if we can.
			wBlockRes = wBlockThis;
			if ( ztry >= pt.m_z + PLAYER_PLATFORM_STEP && ( wBlockThis & CAN_I_BLOCK ) && ! ( wBlockThis & CAN_I_CLIMB ))
				goto zfixup;
		}
	}

	// Any multi items here ?
	if ( pRegion != NULL && pRegion->IsMatchType( REGION_TYPE_MULTI ))
	{
		CObjUID uid( pRegion->GetMultiUID());
		CItem * pItem = uid.ItemFind();
		if ( pItem != NULL )
		{
			const CGrayMulti * pMulti = GetMultiItemDefs( pItem->GetDispID());
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
				if ( zitem > zmyhead )
				{
					fRoof = true;
					continue;	// can walk under it. (ignore it)
				}

				WORD wBlockThis;
				signed char ztop = zitem + CItemBase::GetItemHeight( (ITEMID_TYPE) pMultiItem->m_wTileID, wBlockThis );
				if ( ztop < ztry )
					continue;	// under something else. (ignore it)

				if ( ! wBlockThis )
					continue;	// doesn't block me in any way
				if ( wBlockThis & CAN_I_PLATFORM )
				{
					if ( wBlockThis == CAN_I_PLATFORM && ztop > zmyhead )
						continue; // ignore platforms that i can walk under.
				}
				else
				{
					if ( ztop <= ztry )
						continue;	// always take the platform in a tie
				}
				ztry = ztop;	// step on it if we can.
				wBlockRes = wBlockThis;
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
		if ( pItem->GetTopZ() > zmyhead )
		{
			fRoof = true;
			continue;	// can walk under it. (ignore it)
		}

		// Invis items should not block ???

		WORD wBlockThis = pItem->m_pDef->m_Can & ( CAN_I_DOOR | CAN_I_WATER | CAN_I_CLIMB | CAN_I_BLOCK | CAN_I_PLATFORM );
		if (( wBlockThis & CAN_I_DOOR ) &&
			( wBlockFlags & CAN_I_DOOR ))
			continue;	// ghosts pass thru doors.

		signed char ztop = pItem->GetTopZ() + pItem->m_pDef->GetHeight();
		if ( ztop < ztry )
			continue;	// under something else. (ignore it)
		if ( ! wBlockThis )
			continue;	// doesn't block me in any way
		if ( wBlockThis & CAN_I_PLATFORM )
		{
			if ( wBlockThis == CAN_I_PLATFORM && ztop > zmyhead )
				continue; // ignore platforms that i can walk under.
		}
		else
		{
			if ( ztop <= ztry )
				continue;	// always take the platform in a tie
		}
		ztry = ztop;	// step on it if we can.
		wBlockRes = wBlockThis;
	}

	const CUOMapMeter * pMeter = pMapBlock->GetTerrain( UO_BLOCK_OFFSET(pt.m_x), UO_BLOCK_OFFSET(pt.m_y));
	ASSERT(pMeter);
	if ( pMeter->m_z > zmyhead && pMeter->m_wTerrainIndex != TERRAIN_HOLE )
		fRoof = true;

	if ( ztry == -UO_SIZE_Z ||
		( pMeter->m_wTerrainIndex != TERRAIN_HOLE &&
		pMeter->m_z > ztry &&
		pMeter->m_z < zmyhead ))
	{
		ztry = pMeter->m_z;
		if ( pMeter->m_wTerrainIndex == TERRAIN_NULL )	// inter dungeon type.
		{
			if ( ! ( wBlockFlags & CAN_C_PASSWALLS ))
			{
				wBlockRes = CAN_I_BLOCK;
				ztry = UO_SIZE_Z;
			}
		}
		else if ( pMeter->IsTerrainWater())
		{
			// water tiles = we can swim ?
			wBlockRes = CAN_I_WATER;
		}
		else
		{
			CGrayTerrainInfo land( pMeter->m_wTerrainIndex );
			if ( land.m_flags & UFLAG1_BLOCK )
				wBlockRes = CAN_I_BLOCK;
			else
				wBlockRes = 0;
		}
	}
	}

zfixup:

	// Platforms don't count if we are on them (or close to being on them).
	// Climbables are ok always.
	if ((( wBlockRes & CAN_I_PLATFORM ) && pt.m_z + PLAYER_PLATFORM_STEP >= ztry ) ||
		( wBlockRes & CAN_I_CLIMB ))
		wBlockRes &= ~( CAN_I_PLATFORM | CAN_I_BLOCK );
	if ( ! ( wBlockRes & CAN_I_WATER ))
		wBlockRes |= CAN_I_CLIMB;
	if ( fRoof )
		wBlockRes |= CAN_I_ROOF;	// we are covered.

	wBlockFlags = wBlockRes;
	return( ztry );
}

////////////////////////////////////////////////////////////////

void CWorld::UnLoadRegions( CGObList & List ) // static
{
	// Delete regions in a specific sector list. (or the global list)
	CRegionBase * pRegionNext;
	CRegionBase * pRegion = dynamic_cast <CRegionBase *>(List.GetHead());
	for ( ; pRegion; pRegion = pRegionNext )
	{
		pRegionNext = pRegion->GetNext();
		if ( pRegion->IsMatchType( REGION_TYPE_MULTI ))
			continue;
		delete pRegion;
	}
}

void CWorld::UnLoadRegions()
{
	// for resync. delete all the regions.
	// xcept for those that are dynamic.

	for ( int k=0; k<SECTOR_QTY; k++ )
	{
		// kill any refs to the regions we are about to unload.
		CChar * pChar = STATIC_CAST <CChar*> ( m_Sectors[k].m_Chars.GetHead());
		for ( ; pChar != NULL; pChar = pChar->GetNext())
		{
			pChar->m_pArea = NULL;
		}

		// Now unload the regions.
		m_Sectors[k].UnLoadRegions();
	}

	// Now kill all the global regions.
	UnLoadRegions( m_Regions );

	m_MoonGates.Empty();
}

bool CWorld::LoadRegions( CScript & s )
{
	while (s.FindNextSection())
	{
		switch ( toupper( s.GetKey()[0] ))
		{
		case 'R':	// "ROOM"
			{
			CRegionBase * pRegion = new CRegionBase(REGION_TYPE_ROOM, s.GetArgStr());
			pRegion->r_Load( s );
			pRegion->CheckRealized();
			}
			continue;
		case 'A':	// "AREA"
			{
			CRegionWorld * pRegion = new CRegionWorld(REGION_TYPE_AREA, s.GetArgStr());
			pRegion->r_Load( s );
			pRegion->CheckRealized();
			}
			continue;
		case 'T':	// "TELEPORTERS"
			while ( s.ReadKey())
			{
				TCHAR * ppCmd[7];
				int iCount = ParseCmds( s.GetKey(), ppCmd, COUNTOF(ppCmd));
				if ( iCount <= 0 ) 
					continue;	// skip line.
				if ( iCount < 5 )
				{
					DEBUG_ERR(( "Bad Teleporter List in MAP.SCP\n" ));
					continue;
				}
				// Add the teleporter to the CSector.
				new CTeleport( ppCmd, iCount );
			}
			break;
		case 'M':	// "Moongates"
			while ( s.ReadKey())
			{
				m_MoonGates.Add( CPointMap( s.GetKey()));
			}
			break;
		case 'L':	// "LOCATION x"
			break;
		default:
			continue;
		}
	}

	// If this is a resync then we must put all the on-line chars in regions.
	for ( int i=0; i<SECTOR_QTY; i++ )
	{
		CChar * pChar = STATIC_CAST <CChar*> ( m_Sectors[i].m_Chars.GetHead());
		for ( ; pChar != NULL; pChar = pChar->GetNext())
		{
			pChar->MoveToRegionReTest( REGION_TYPE_MULTI | REGION_TYPE_AREA );
		}
	}

	return( true );
}

bool CWorld::LoadRegions()
{
	CScript s;
	if ( ! s.OpenFind( GRAY_FILE "map" ))
		return false;
	if ( ! LoadRegions( s ))
		return( false );
	s.Close();
	if ( s.OpenFind( GRAY_FILE "map2", OF_READ|OF_NONCRIT ))
	{
		LoadRegions( s );
	}
	return( true );
}

CPointMap CWorld::GetRegionPoint( const TCHAR * pCmd ) const // Decode a teleport location number into X/Y/Z
{
	// Might just be a point coord ?

	GETNONWHITESPACE( pCmd );
	if ( pCmd[0] == '-' )	// Get location from start list.
	{
		int i = ( - atoi(pCmd)) - 1;
		if ( ! g_Serv.m_StartDefs.IsValidIndex( i )) i = 0;
		return( g_Serv.m_StartDefs[i]->m_p );
	}

	CPointMap pt;	// invalid point
	if ( isdigit( pCmd[0] ))
	{
		TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
		strcpy( szTemp, pCmd );
		int iCount = pt.Read( szTemp );
		if ( iCount >= 2 )
		{
			return( pt );
		}

		// Enum number region
		int i = atoi(pCmd);
		// Look at the world region if no CSector regions apply.
		CRegionBase * pRegion = STATIC_CAST <CRegionBase*> (m_Regions.GetHead());
		for ( ; pRegion!=NULL; pRegion = pRegion->GetNext())
		{
			if ( ! i-- )
			{
				return( pRegion->m_p );
			}
		}
	}
	else
	{
		// Match the region name
		CRegionBase * pRegion = STATIC_CAST <CRegionBase*> (m_Regions.GetHead());
		for ( ; pRegion!=NULL; pRegion = pRegion->GetNext())
		{
			if ( pRegion->IsMatchName( pCmd ))
			{
				return( pRegion->m_p );
			}
		}
	}
	// no match.
	return( pt );
}

