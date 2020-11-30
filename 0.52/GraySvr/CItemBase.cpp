//
// CItemBase.CPP
// Copyright Menace Software (www.menasoft.com).
// define the base types of an item (rather than the instance)
//
#include "graysvr.h"	// predef header.

/////////////////////////////////////////////////////////////////
// -CItemBase

CItemBase::CItemBase( ITEMID_TYPE id, const CUOItemTypeRec &tile ) :
	CBaseBase( id )
{
	m_weight = 0;
	m_type = ITEM_NORMAL;
	m_buyvaluemax = 0;
	m_buyvaluemin = 0;
	m_sellvaluemax = 0;
	m_sellvaluemin = 0;
	m_Material = MATERIAL_OTHER;

	// Just applies to weapons/armor.
	m_layer = LAYER_NONE;
	m_StrReq = 0;

	if ( ! IsValidDispID( id ))
	{
		// There should be an ID= in the scripts later.
		m_wDispIndex = ITEMID_GOLD_C1; // until i hear otherwise from the script file.
	}
	else
	{
		SetBaseDispID( id, tile );
	}
}

CItemBase::~CItemBase()
{
	// These don't really get destroyed til the server is shut down but keep this around anyhow.
}

void CItemBase::DupeCopy( const CItemBase * pBase )
{
	m_weight = pBase->m_weight;
	m_type = pBase->m_type;
	m_buyvaluemax = pBase->m_buyvaluemax;
	m_buyvaluemin = pBase->m_buyvaluemin;
	m_sellvaluemax = pBase->m_sellvaluemax;
	m_sellvaluemin = pBase->m_sellvaluemin;
	m_Material = pBase->m_Material;

	// ???
	// m_flip_id

	// Just applies to weapons/armor.
	m_layer = pBase->m_layer;
	m_StrReq = pBase->m_StrReq;

	CBaseBase::DupeCopy( pBase );
}

TCHAR * CItemBase::GetNamePluralize( const TCHAR * pszNameBase, bool fPluralize )	// static
{
	TCHAR * pszName = GetTempStr();
	int j=0;
	bool fInside = false;
	bool fPlural;
	for ( int i=0; pszNameBase[i]; i++ )
	{
		if ( pszNameBase[i] == '%' )
		{
			fInside = ! fInside;
			fPlural = true;
			continue;
		}
		if ( fInside )
		{
			if ( pszNameBase[i] == '/' )
			{
				fPlural = false;
				continue;
			}
			if ( fPluralize )
			{
				if ( ! fPlural ) continue;
			}
			else
			{
				if ( fPlural ) continue;
			}
		}
		pszName[j++] = pszNameBase[i];
	}
	pszName[j] = '\0';
	return( pszName );
}

bool CItemBase::GetItemData( ITEMID_TYPE id, CUOItemTypeRec * pData ) // static
{
	// Read from g_Install.m_fTileData
	// Get an Item tile def data.
	// Invalid object id ?
	// NOTE: This data should already be read into the m_ItemBase table ???

	if ( ! IsValidDispID(id))
		return( false );

	try
	{
		CGrayItemInfo info( id );
		*pData = *( static_cast <CUOItemTypeRec *>( & info ));
	}
	catch(...)
	{
		return( false );
	}

	// Unused tile I guess. Don't create it.
	if ( ! pData->m_flags &&
		! pData->m_weight &&
		! pData->m_layer &&
		! pData->m_dwUnk6 &&
		! pData->m_dwAnim &&
		! pData->m_wUnk14 &&
		! pData->m_height &&
		! pData->m_name[0]
		)
	{
		if ( id == ITEMID_BBOARD_MSG ) // special
			return( true );
		if ( IsGamePiece( id ))
			return( true );
		if ( IsTrackID(id))	// preserve these
			return( true );
		return( false );
	}

	if ( pData->m_flags & UFLAG2_CLIMBABLE )
		pData->m_height /= 2; // For Stairs+Ladders
	return( true );
}

inline signed char CItemBase::GetItemHeightFlags( const CUOItemTypeRec & tile, WORD & wBlockThis ) // static
{
	// Chairs are marked as blocking for some reason ?
	if ( tile.m_flags & UFLAG4_DOOR ) // door
	{
		wBlockThis = CAN_I_DOOR;
		return( tile.m_height );
	}
	if ( tile.m_flags & UFLAG1_BLOCK )
	{
		if ( tile.m_flags & UFLAG1_WATER )	// water	(IsWaterFishID() ?
		{
			wBlockThis = CAN_I_WATER;
			return( tile.m_height );
		}
		wBlockThis = CAN_I_BLOCK;
	}
	else
	{
		wBlockThis = 0;
		if ( ! ( tile.m_flags & UFLAG2_PLATFORM ))
			return 0;	// have no effective height if it doesn't block.
	}
	if ( tile.m_flags & UFLAG2_PLATFORM )
	{
		wBlockThis |= CAN_I_PLATFORM;
	}
	if ( tile.m_flags & UFLAG2_CLIMBABLE )
	{
		wBlockThis |= CAN_I_CLIMB;
		return( tile.m_height * 2 );
	}
	return( tile.m_height );
}

signed char CItemBase::GetItemHeight( ITEMID_TYPE id, WORD & wBlockThis ) // static
{
	// Get just the height and the blocking flags for the item by id.
	// used for walk block checking.

	CUOItemTypeRec tile;
	if ( ! GetItemData( id, &tile ))
	{
		wBlockThis = 0xFF;
		return( UO_SIZE_Z );
	}
	if ( IsChairID( id ))
	{
		wBlockThis = 0;
		return 0;	// have no effective height if they don't block.
	}
	return( GetItemHeightFlags( tile, wBlockThis ));
}

ITEMID_TYPE CItemBase::Flip( ITEMID_TYPE id ) const
{
	if ( m_flip_id.GetCount())
	{
		ITEMID_TYPE idprev = GetDispID();
		for ( int i=0; true; i++ )
		{
			if ( i>=m_flip_id.GetCount())
			{
				break;
			}
			ITEMID_TYPE idnext = m_flip_id[i];
			if ( idprev == id )	
				return( idnext );
			idprev = idnext;
		}
	}
	return( GetDispID());
}

void CItemBase::SetBaseDispID( ITEMID_TYPE id, const CUOItemTypeRec &tile )
{
	// Set the artwork/display id.
	ASSERT( IsValidDispID(id));
	m_wDispIndex = id;

	// ??? The script should be doing ALL this for us !

	if ( IsContainerID( id )) // UFLAG3_CONTAINER
		m_type = ITEM_CONTAINER;
	else if ( IsDoorID( id )) // UFLAG4_DOOR
		m_type = ITEM_DOOR;
	else if ( IsTrapID( id ))	// UFLAG1_DAMAGE
		m_type = ITEM_TRAP;
	else if ( IsWaterFishID( id )) // UFLAG1_WATER
		m_type = ITEM_WATER;
	else if ( IsChairID( id ))
		m_type = ITEM_CHAIR;
	else if ( IsShipID( id ))
		m_type = ITEM_SHIP;
	else if ( IsMultiID( id ))
		m_type = ITEM_MULTI;
	else if ( IsForgeID( id ))
		m_type = ITEM_FORGE;
	else if ( IsTreeID( id ))
		m_type = ITEM_TREE;
	else if ( IsRockID( id ))
		m_type = ITEM_ROCK;
	else if ( IsBedEWID( id ))
		m_type = ITEM_BED_EW;
	else if ( IsBedNSID( id ))
		m_type = ITEM_BED_NS;
	else if ( IsLightUnlit( id ))
		m_type = ITEM_LIGHT_OUT;
	else if ( IsLightLit( id ))
		m_type = ITEM_LIGHT_LIT;
	else if ( IsTrackID( id ))
		m_type = ITEM_FIGURINE;
	else if ( IsRawFood( id ))
		m_type = ITEM_FOOD_RAW;
	else if ( IsCrop(id) )
		m_type = ITEM_PLANT;
	else if ( IsGamePiece( id ))
		m_type = ITEM_GAME_PIECE;
	else if ( IsGrassID( id ))
		m_type = ITEM_GRASS;
	else
		m_type = ITEM_NORMAL;	// Get from script i guess.

	if (( tile.m_flags & UFLAG1_BLOCK ) && (tile.m_flags & UFLAG1_WATER))
		m_type = ITEM_WATER;
	if ( tile.m_flags & UFLAG4_DOOR )
		m_type = ITEM_DOOR;

	// Stuff read from .mul file.
	// Some items (like hair) have no names !
	// Get rid of the leading spaces in the names.
	TCHAR szName[ sizeof(tile.m_name)+1 ];
	int j=0;
	for ( int i=0; i<sizeof(tile.m_name) && tile.m_name[i]; i++ )
	{
		if ( j==0 && ISWHITESPACE(tile.m_name[i]))
			continue;
		szName[j++] = tile.m_name[i];
	}
	szName[j] = '\0';
	SetTypeName( szName );	// default type name.

	m_Can = 0;
	if ( m_type == ITEM_CHAIR )
	{
		SetHeight( 0 ); // have no effective height if they don't block.
	}
	else
	{
		SetHeight( GetItemHeightFlags( tile, m_Can ));
	}

	if ( m_type == ITEM_DOOR )
	{
		m_Can &= ~CAN_I_BLOCK;
		int type = CItemBase::IsDoorID( id )-1;
		if ( type & DOOR_OPENED )
		{
			m_Can &= ~CAN_I_DOOR;
		}
		else
		{
			m_Can |= CAN_I_DOOR;
		}
	}
	if ( tile.m_flags & UFLAG3_LIGHT )	// this may actually be a moon gate or fire ?
	{
		// m_type = ITEM_LIGHT_LIT;
		m_Can |= CAN_I_LIGHT;
	}
	if (( tile.m_flags & UFLAG2_STACKABLE ) || m_type == ITEM_REAGENT ||
		id == ITEMID_EMPTY_BOTTLE )
		m_Can |= CAN_I_PILE;

	if ( tile.m_flags & UFLAG3_CLOTH )
		m_Material = MATERIAL_CLOTH;
	else if ( tile.m_flags & UFLAG1_WATER )
		m_Material = MATERIAL_WATER;
	else if ( tile.m_flags & UFLAG1_LIQUID )
		m_Material = MATERIAL_LIQUID;

	if ( tile.m_weight == 0xFF ||	// not movable.
		( tile.m_flags & UFLAG1_WATER ))
	{
		// water can't be picked up.
		m_weight = 0xFFFF;
	}
	else
	{
		m_weight = tile.m_weight * WEIGHT_UNITS;
	}

	if ( tile.m_flags & ( UFLAG1_EQUIP | UFLAG3_EQUIP2 ))
	{
		m_layer = (LAYER_TYPE) tile.m_layer;
	}
	else
	{
		m_layer = LAYER_NONE ;
	}
	switch ( id )
	{
	case ITEMID_SPELLBOOK:
	case ITEMID_SPELLBOOK2:
		m_layer = LAYER_HAND1;
		break;
	case ITEMID_BANK_BOX:	// ??? this might be a flipped pack ?
		m_layer = LAYER_BANKBOX;
		break;
	case ITEMID_MOONGATE_RED:
	case ITEMID_MOONGATE_BLUE:
	case ITEMID_MOONGATE_GRAY:
		m_layer = LAYER_NONE;
		break;
	}

	if ( m_layer && ! IsMovableType())
	{
		// How am i supposed to equip something i can't pick up ?
		m_weight = WEIGHT_UNITS;
	}
	if ( m_type == ITEM_EQ_HORSE )
	{
		m_layer = LAYER_HORSE;
	}
}

#ifdef COMMENT

BYTE CItemBase::IsReadGumpID() const // grave stones and such
{

	return( 0x64 ); // plaque
	return( 0x65 ); // grave stone

}

#endif

bool CItemBase::IsGamePiece( ITEMID_TYPE id ) // static
{
	return( id >= ITEMID_GAME1_CHECKER && id <= ITEMID_GAME_HI );
}

bool CItemBase::IsTrackID( ITEMID_TYPE id ) // static
{
	return( id >= ITEMID_TRACK_BEGIN && id <= ITEMID_TRACK_END );
}

bool CItemBase::IsBedEWID( ITEMID_TYPE id )
{
	switch ( id )
	{
		// ???
	}
	return( false );
}

bool CItemBase::IsBedNSID( ITEMID_TYPE id )
{
	switch ( id )
	{
		// ???
	}
	return( false );
}

bool CItemBase::IsShipID( ITEMID_TYPE id )
{
	return( id >= ITEMID_MULTI && id <= ITEMID_SHIP6_W );
}

bool CItemBase::IsMultiID( ITEMID_TYPE id ) // static
{
	// NOTE: Ships are also multi's
	return( id >= ITEMID_MULTI && id < ITEMID_SCRIPT );
}

bool CItemBase::IsTrapID( ITEMID_TYPE id ) // static
{
	// ??? this doesn't hold true always.
	switch ( id )
	{
	case ITEMID_TRAP_FACE1:	// stone face. k
	case ITEMID_TRAP_FACE2: // stone face W k
	case 0x1103: // saw trap. k
	case 0x1108: // spike trap. k
	case 0x1110: // stone face N k 110f = non trap
	case 0x1116: // saw trap. k
	case 0x111b: // spike trap.k
	case 0x1125: // exp shroom. k
	case 0x112b: // dart trap. k
	case 0x112f: // dart trap. k
	case 0x1132: // axe trap. k
	case 0x1139: // gas trap. k
	case 0x113f: // axe trap.
	case 0x1145: // gas trap.
	case 0x114b: // axe trap.
	case 0x1193: // axe
	case 0x119a: // spike
	case 0x11a0: // spike
	case 0x11a6: // gas
	case 0x11ac: // saw
	case 0x11b1: // saw
	case 0x11b6: // boulder.
	case ITEMID_TRAP_CRUMBLEFLOOR: // crumble floor. ok
		return( true );
	}

	return( false );
}

bool CItemBase::IsWaterFishID( ITEMID_TYPE id ) // static
{
	// Assume this means water we can fish in.
	// Not water we can wash in.
	if ( id >= 0x1796 && id <= 0x17b2 )
		return( true );
	if ( id == 0x1559 )
		return( true );
	return( false );
}

bool CItemBase::IsWaterWashID( ITEMID_TYPE id ) // static
{
	if ( id >= ITEMID_WATER_TROUGH_1 && id <= ITEMID_WATER_TROUGH_2	)
		return( true );
	return( IsWaterFishID( id ));
}

int CItemBase::IsDoorID( ITEMID_TYPE id ) // static
{
	static const ITEMID_TYPE Item_DoorBase[] =
	{
		ITEMID_DOOR_SECRET_1,
		ITEMID_DOOR_SECRET_2,
		ITEMID_DOOR_SECRET_3,
		ITEMID_DOOR_SECRET_4,
		ITEMID_DOOR_SECRET_5,
		ITEMID_DOOR_SECRET_6,
		ITEMID_DOOR_METAL_S,
		ITEMID_DOOR_BARRED,
		ITEMID_DOOR_RATTAN,
		ITEMID_DOOR_WOODEN_1,
		ITEMID_DOOR_WOODEN_2,
		ITEMID_DOOR_METAL_L,
		ITEMID_DOOR_WOODEN_3,
		ITEMID_DOOR_WOODEN_4,
		ITEMID_DOOR_IRONGATE_1,
		ITEMID_DOOR_WOODENGATE_1,
		ITEMID_DOOR_IRONGATE_2,
		ITEMID_DOOR_WOODENGATE_2,
		ITEMID_DOOR_BAR_METAL,
	};

	for ( int i=0;i<COUNTOF(Item_DoorBase);i++)
	{
		int did = id - Item_DoorBase[i];
		if ( did >= 0 && did <= 15 ) return( did+1 );
	}
	return( 0 );
}

GUMP_TYPE CItemBase::IsContainerID( ITEMID_TYPE id ) // static
{
	// return the container gump id.

#if 0
	// The items i might be missing form the container list.

0990: 00204000,W01,L00,?00000001,A00000000,?0001,H01,'basket'
09a8: 00204040,W05,L00,?00000000,A00000000,?0005,H03,'metal box'
09a9: 00204040,Wff,L00,?00000001,A00000000,?0006,H03,'small crate'
09aa: 00204040,W04,L00,?00000001,A00000000,?0005,H03,'wooden box'
09ab: 00204040,Wff,L00,?00000001,A00000000,?000f,H05,'metal chest'
09ac: 04204040,W03,L00,?00000001,A00000000,?0003,H03,'bushel'
09ad: 00204000,W01,L00,?00000001,A00000000,?0001,H01,'pitcher of milk'
09b0: 00204000,W01,L0d,?00000001,A00000000,?0001,H01,'belt pouch'
09b1: 00204000,W02,L00,?00000001,A00000000,?0001,H02,'basket'
09b2: 00604002,W03,L15,?00000000,A000001a6,?0008,H01,'backpack'
0a2c: 04204040,Wff,L00,?00000000,A00000000,?0007,H08,'chest of drawers'
0a2d: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a2e: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a2f: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a30: 00204040,Wff,L00,?00000000,A00000000,?0007,H09,'chest of drawers'
0a31: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a32: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a33: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a34: 00204040,Wff,L00,?00000000,A00000000,?0007,H08,'chest of drawers'
0a35: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a36: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a37: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a38: 00204040,Wff,L00,?00000000,A00000000,?0007,H09,'chest of drawers'
0a39: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a3a: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a3b: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a3c: 00204040,Wff,L00,?00000000,A00000000,?0000,H05,'dresser'
0a3d: 00204040,Wff,L00,?00000000,A00000000,?0000,H06,'dresser'
0a3e: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a3f: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a40: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a41: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a42: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a43: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a44: 00204040,Wff,L00,?00000000,A00000000,?0000,H05,'dresser'
0a45: 00204040,Wff,L00,?00000000,A00000000,?0000,H05,'dresser'
0a46: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a47: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a48: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a49: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a4a: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a4b: 00204000,Wff,L00,?00000000,A00000000,?0000,H00,'drawer'
0a4d: 00208040,Wff,L00,?00000000,A00000000,?0006,H0f,'armoire'
0a4f: 00208040,Wff,L00,?00000000,A00000000,?0006,H0f,'armoire'
0a51: 00208040,Wff,L00,?00000000,A00000000,?0006,H0f,'armoire'
0a53: 00208040,Wff,L00,?00000000,A00000000,?0006,H0f,'armoire'
0a97: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a98: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a99: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a9a: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a9b: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a9c: 00204040,Wff,L00,?00000000,A00000000,?0000,H0c,'bookcase'
0a9d: 00204040,Wff,L00,?00000000,A00000000,?0008,H0c,'wooden shelf'
0a9e: 00204040,Wff,L00,?00000000,A00000000,?0008,H0c,'wooden shelf'
0e1c: 00204000,W05,L00,?00000000,A00000000,?0000,H01,'backgammon board'
0e3c: 00204040,Wff,L00,?00000000,A00000000,?0008,H03,'crate'
0e3d: 04204040,Wff,L00,?00000000,A00000000,?0008,H03,'crate'
0e3e: 00204040,Wff,L00,?00000000,A00000000,?0006,H03,'crate'
0e3f: 00204040,Wff,L00,?00000000,A00000000,?0006,H03,'crate'
0e40: 00204040,Wff,L00,?00000000,A00000000,?000f,H04,'metal chest'
0e41: 00204040,Wff,L00,?00000000,A00000000,?000f,H04,'metal chest'
0e42: 00204040,Wff,L00,?00000000,A00000000,?000c,H04,'wooden chest'
0e43: 00204040,Wff,L00,?00000000,A00000000,?000c,H04,'wooden chest'
0e75: 00604002,W03,L15,?00000000,A000001a6,?0008,H01,'backpack'
0e76: 00204000,W02,L00,?00000000,A00000000,?0003,H02,'bag'
0e77: 04204040,W19,L00,?00000000,A00000000,?0005,H05,'barrel'
0e78: 00204000,W02,L00,?00000000,A00000000,?0005,H01,'basin'
0e79: 00200000,W01,L00,?00000000,A00000000,?0003,H01,'pouch'
0e7a: 04204000,W02,L00,?00000000,A00000000,?0003,H01,'picnic basket'
0e7c: 00204040,Wff,L00,?00000000,A00000000,?000f,H05,'metal chest'
0e7d: 00200040,W04,L00,?00000000,A00000000,?0005,H03,'wooden box'
0e7e: 04204040,Wff,L00,?00000000,A00000000,?0005,H02,'crate'
0e7f: 00204040,Wff,L00,?00000000,A00000000,?0005,H03,'keg'
0e80: 00204040,W64,L00,?00000000,A00000000,?0005,H03,'strong box'
0e83: 04208000,W1e,L00,?00000000,A00000000,?0000,H01,'empty tub'
0efa: 04604000,W03,L01,?00000000,A000003d8,?000a,H01,'spellbook'
0fa6: 00204000,W05,L00,?00000000,A00000000,?0000,H01,'game board'
0fad: 00204000,W05,L00,?00000000,A00000000,?0000,H01,'backgammon game'
0fae: 00204040,W19,L00,?00000000,A00000000,?0000,H03,'barrel'
1011: 00204000,W01,L00,?00000000,A00000000,?0005,H01,'key ring'
14e0: 00204000,W03,L00,?00000000,A00000000,?0000,H00,'bucket'
1769: 00204000,W01,L00,?00000000,A00000000,?0000,H01,'key ring'
176a: 00204000,W01,L00,?00000000,A00000000,?0000,H01,'key ring'
176b: 00204000,W02,L00,?00000000,A00000000,?0000,H01,'key ring'
1940: 00204040,Wff,L00,?00000000,A00000000,?0000,H00,'keg'

3e4b: 00204040,Wff,L00,?00000000,A00000000,?0000,H03,'tiller man'
3e4e: 00204040,Wff,L00,?00000000,A00000000,?0000,H03,'tiller man'
3e50: 00204040,Wff,L00,?00000000,A00000000,?0000,H03,'tiller man'
3e55: 00204040,Wff,L00,?00000000,A00000000,?0000,H03,'tiller man'
#endif

	switch ( id )
	{
	case ITEMID_Bulletin1:	// It is a container but will never open this way.
	case ITEMID_Bulletin2:
		return( GUMP_SECURE_TRADE );

	case ITEMID_KEY_RING0:
	case ITEMID_KEY_RING1:
	case ITEMID_KEY_RING3:
	case ITEMID_KEY_RING5:
		return( GUMP_RESERVED );	// it is a container but has not gump.

	case ITEMID_SKELETON_1:
	case ITEMID_SKELETON_2:
	case ITEMID_SKELETON_3:
	case ITEMID_SKELETON_4:
	case ITEMID_SKELETON_5:
	case ITEMID_SKELETON_6:
	case ITEMID_SKELETON_7:
	case ITEMID_SKELETON_8:
	case ITEMID_SKELETON_9:

	case ITEMID_CORPSE_1:	// 'dead orc captain'
	case ITEMID_CORPSE_2:	// 'corpse of orc'
	case ITEMID_CORPSE_3:	// 'corpse of skeleton
	case ITEMID_CORPSE_4:	// 'corpse'
	case ITEMID_CORPSE_5:	// 'corpse'
	case ITEMID_CORPSE_6:	// 'deer corpse'
	case ITEMID_CORPSE_7:	// 'wolf corpse'
	case ITEMID_CORPSE_8:	// 'corpse of rabbit'
	case ITEMID_CORPSE:     return( GUMP_CORPSE );  // Coffin

	case ITEMID_POUCH2:
	case ITEMID_POUCH:
	case ITEMID_BACKPACK:	return( GUMP_PACK );  // 60 = Backpack (52?)

	case ITEMID_BAG:		return( GUMP_BAG );  // Leather Bag

	case ITEMID_BARREL:		 // Barrel
	case ITEMID_BARREL_2:	return( GUMP_BARREL );  // Barrel_2

	case ITEMID_BASKET2:
	case ITEMID_SBASKET:	return( GUMP_BASKET_SQ );  // Square picknick Basket

	case ITEMID_BOX_METAL:	return( GUMP_BOX_WOOD );  // Box/Pouch

	case ITEMID_BASIN:
	case ITEMID_BASKET:
	case 0xe83:				// empty tub
	case ITEMID_RBASKET:	return( GUMP_BASKET_RO );  // Round Basket

	case ITEMID_CHEST3:
	case ITEMID_CHEST4:		return( GUMP_CHEST_WO_GO );	// wood chest ?

	case ITEMID_CHEST_METAL2_1: // 2 color metal chest.
	case ITEMID_CHEST_METAL2_2: return( GUMP_CHEST_GO_SI );

	case ITEMID_BANK_BOX:
	case ITEMID_VENDOR_BOX:
	case ITEMID_CHEST_SILVER:
	case ITEMID_CHEST_SILVER2:
		return( GUMP_CHEST_SI );  // Silver Chest

	case ITEMID_BOX_WOOD1:
	case ITEMID_BOX_WOOD2:	return( GUMP_BOX_WOOD_OR );  // Wooden Box

	case ITEMID_CRATE1:
	case ITEMID_CRATE2:
	case ITEMID_CRATE3:
	case ITEMID_CRATE4:
	case ITEMID_CRATE5:
	case ITEMID_CRATE6:
	case ITEMID_CRATE6_2:
	case ITEMID_CRATE7:	return( GUMP_CRATE );	// Wooden Crate

	case ITEMID_KEG:		return( GUMP_BARREL );  // Keg
	case ITEMID_BRASSBOX:	return( GUMP_BOX_GO_LO );  // Brass Box

	case ITEMID_BOOKSHELF1: // book shelf
	case ITEMID_BOOKSHELF2: // book shelf
	case ITEMID_BOOKSHELF3: // book shelf
	case ITEMID_BOOKSHELF4: // book shelf
	case ITEMID_BOOKSHELF5: // book shelf
	case ITEMID_BOOKSHELF6: // book shelf
	case ITEMID_BOOKSHELF7: // book shelf
	case ITEMID_BOOKSHELF8: // book shelf
		// 69 = book shelf
		return( GUMP_BOOK_SHELF );	// ???

	case 0x0a3e: //'drawer'
	case 0x0a3f: //'drawer'
	case 0x0a40: //'drawer'
	case 0x0a41: //'drawer'
	case 0x0a42: //'drawer'
	case 0x0a43: //'drawer'
	case 0x0a46: //'drawer'
	case 0x0a47: //'drawer'
	case 0x0a48: //'drawer'
	case 0x0a49: //'drawer'
	case 0x0a4a: //'drawer'
	case 0x0a4b: //'drawer'
		return( GUMP_DRAWER_LT );

	case 0x0a2c: // light drawers
	case 0x0a34: // light drawers.
	case 0x0a3c: // light dresser drawers.
	case 0x0a3d: // light dresser drawers.
	case 0x0a44: // light dresser drawers.
	case 0x0a45: // light dresser drawers.

	case 0x0a2d: //'drawer'
	case 0x0a2e: //'drawer'
	case 0x0a2f: //'drawer'
	case 0x0a35: //'drawer'
	case 0x0a36: //'drawer'
	case 0x0a37: //'drawer'
		// 73 = light drawer
		return( GUMP_DRAWER_LT );

	case 0x0a30: // dark drawers.
	case 0x0a38: // dark drawers.

	case 0x0a31: //'drawer'
	case 0x0a32: //'drawer'
	case 0x0a33: //'drawer'
	case 0x0a39: //'drawer'
	case 0x0a3a: //'drawer'
	case 0x0a3b: //'drawer'
		// 64 = dark drawer
		return( GUMP_DRAWER_DK );

	case 0x0a4c: // dark armoire.
	case 0x0a4d: // dark armoire.
	case 0x0a50: // dark armoire.
	case 0x0a51: // dark armoire.
		// 70 = dark armoire.
		return( GUMP_CABINET_DK );

	case 0x0a4e: // light armoire.
	case 0x0a4f: // light armoire.
	case 0x0a52: // light armoire.
	case 0x0a53: // light armoire.
		// 71 = light armoire.
		return( GUMP_CABINET_LT );

	case ITEMID_GAME_BACKGAM:	// backgammon board.
	case ITEMID_GAME_BACKGAM_2:	// backgammon board #2.
		return( GUMP_GAME_BACKGAM );
	case ITEMID_GAME_BOARD:	// game board.
		return( GUMP_GAME_BOARD );

	case ITEMID_SHIP_HATCH_E:
	case ITEMID_SHIP_HATCH_W:
	case ITEMID_SHIP_HATCH_N:
	case ITEMID_SHIP_HATCH_S:
		// 68 = ship hold
		return( GUMP_SHIP_HOLD );
	}

	return( GUMP_NONE );	// guess it's not a container.
}

bool CItemBase::IsForgeID( ITEMID_TYPE id ) // static
{
	if (( id == ITEMID_FORGE_1 ) ||
		( id >= ITEMID_BELLOWS_1 && id <= ITEMID_FORGE_33 ))
		return( true );
	return( false );
}

bool CItemBase::IsChairID( ITEMID_TYPE id ) // static
{
	// Strangely there is not chair flag in the statics.mul file !!!
	// ITEM_CHAIR

	switch ( id )
	{
	case 0x0459: // 'marble bench'
	case 0x045a: // 'marble bench'
	case 0x045b: // 'stone bench'
	case 0x045c: // 'stone bench'
	case 0x0b2c: // 'wooden bench'
	case 0x0b2d: // 'wooden bench'
	case 0x0b2e: // 'wooden chair'
	case 0x0b2f: // 'wooden chair'
	case 0x0b30: // 'wooden chair'
	case 0x0b31: // 'wooden chair'
	case 0x0b32: // 'throne'
	case 0x0b33: // 'throne'
	case 0x0b4e: // 'chair'
	case 0x0b4f: // 'chair'
	case 0x0b50: // 'chair'
	case 0x0b51: // 'chair'
	case 0x0b52: // 'chair'
	case 0x0b53: // 'chair'
	case 0x0b54: // 'chair'
	case 0x0b55: // 'chair'
	case 0x0b56: // 'chair'
	case 0x0b57: // 'chair'
	case 0x0b58: // 'chair'
	case 0x0b59: // 'chair'
	case 0x0b5a: // 'chair'
	case 0x0b5b: // 'chair'
	case 0x0b5c: // 'chair'
	case 0x0b5d: // 'chair'
	case 0x0b5e: // 'foot stool'
	case 0x0b5f: // 'bench'
	case 0x0b60: // 'bench'
	case 0x0b61: // 'bench'
	case 0x0b62: // 'bench'
	case 0x0b63: // 'bench'
	case 0x0b64: // 'bench'
	case 0x0b65: // 'bench'
	case 0x0b66: // 'bench'
	case 0x0b67: // 'bench'
	case 0x0b68: // 'bench'
	case 0x0b69: // 'bench'
	case 0x0b6a: // 'bench'
	case 0x0b91: // 'bench'
	case 0x0b92: // 'bench'
	case 0x0b93: // 'bench'
	case 0x0b94: // 'bench'
	case 0x0c17: // 'covered chair'
	case 0x0c18: // 'covered chair'
	case 0x1049: // 'loom bench'
	case 0x104a: // 'loom bench'
	case 0x1207: // 'stone bench'
	case 0x1208: // 'stone bench'
	case 0x1209: // 'stone bench'
	case 0x120a: // 'stone bench'
	case 0x120b: // 'stone bench'
	case 0x120c: // 'stone bench'
	case 0x1218: // 'stone chair'
	case 0x1219: // 'stone chair'
	case 0x121a: // 'stone chair'
	case 0x121b: // 'stone chair'
	case 0x1526: // 'throne'
	case 0x1527: // 'throne'
	case 0x19f1: // 'woodworker's bench'
	case 0x19f2: // 'woodworker's bench'
	case 0x19f3: // 'woodworker's bench'
	case 0x19f5: // 'woodworker's bench'
	case 0x19f6: // 'woodworker's bench'
	case 0x19f7: // 'woodworker's bench'
	case 0x19f9: // 'cooper's bench'
	case 0x19fa: // 'cooper's bench'
	case 0x19fb: // 'cooper's bench'
	case 0x19fc: // 'cooper's bench'
	case 0x1dc7: // 'sandstone bench'
	case 0x1dc8: // 'sandstone bench'
	case 0x1dc9: // 'sandstone bench'
	case 0x1dca: // 'sandstone bench'
	case 0x1dcb: // 'sandstone bench'
	case 0x1dcc: // 'sandstone bench'
	case 0x1dcd: // 'marble bench'
	case 0x1dce: // 'marble bench'
	case 0x1dcf: // 'marble bench'
	case 0x1dd0: // 'marble bench'
	case 0x1dd1: // 'marble bench'
	case 0x1dd2: // 'marble bench'
	case 0x1e6f: // 'chair'
	case 0x1e78: // 'chair'
	case 0x3dff: // 'bench'
	case 0x3e00: // 'bench'
		return( true );
	}

	return( false );
}

bool CItemBase::IsTreeID( ITEMID_TYPE id )	// static
{
	// Choppable tree
	if (( id >= ITEMID_TREE_LO && id <= ITEMID_TREE_HI ))
		return( true );
	if (( id >= ITEMID_TREE2_LO && id <= ITEMID_TREE2_HI ))
		return( true );
	if (( id >= ITEMID_TREE3_LO && id <= ITEMID_TREE3_HI ))
		return( true );
	if (( id >= ITEMID_TREE4_LO && id <= ITEMID_TREE4_HI ))
		return( true );
	if (( id >= ITEMID_TREE5_LO && id <= ITEMID_TREE5_HI ))
		return( true );
	switch ( id )
	{
	case 0x0d01: return( true );
	}
	return( false );
}

bool CItemBase::IsRockID( ITEMID_TYPE id )	// static
{
	// Minable rock. NOTE: NOT throwable rocks.
	static const ITEMID_TYPE Item_RockBase[] =
	{
		ITEMID_ROCK_1_LO,	ITEMID_ROCK_1_HI,
		ITEMID_ROCK_2_LO,	ITEMID_ROCK_2_HI,
		ITEMID_ROCK_3_LO,	ITEMID_ROCK_3_HI,
		ITEMID_ROCK_4_LO,	ITEMID_ROCK_4_HI,
		ITEMID_ROCK_5_LO,	ITEMID_ROCK_5_HI,
		ITEMID_ROCK_6_LO,	ITEMID_ROCK_6_HI,
	};

	for ( int i=0;i<COUNTOF(Item_RockBase); i+=2)
	{
		if ( id >= Item_RockBase[i] && id <= Item_RockBase[i+1])
			return true;
	}
	return (false);
}

bool CItemBase::IsGrassID( ITEMID_TYPE id ) // static
{
	if (id >= 0x0cac && id <= 0x0cbd)
		return (true);
	if (id == 0x0cc5 || id == 0x0cc6 || id == 0x0d32 || id == 0x0d33)
		return (true);
	return (false);
}

bool CItemBase::IsFruit( ITEMID_TYPE id )	// static
{
	static const WORD Items_Fruit[] =
	{
		ITEMID_PLANT_GRAPE1,
		ITEMID_PLANT_GRAPE2,
		ITEMID_PLANT_GRAPE3,
		ITEMID_PLANT_GRAPE4,
		ITEMID_PLANT_GRAPE5,
		ITEMID_PLANT_GRAPE6,
		ITEMID_PLANT_GRAPE7,
		ITEMID_PLANT_GRAPE8,
		ITEMID_PLANT_GRAPE9,
		ITEMID_PLANT_GRAPE10,
		ITEMID_TREE_APPLE_EMPTY1,
		ITEMID_TREE_APPLE_FULL1,
		ITEMID_TREE_APPLE_FALL1,
		ITEMID_TREE_APPLE_EMPTY2,
		ITEMID_TREE_APPLE_FULL2,
		ITEMID_TREE_APPLE_FALL2,
		ITEMID_TREE_PEACH_EMPTY1,
		ITEMID_TREE_PEACH_FULL1,
		ITEMID_TREE_PEACH_FALL1,
		ITEMID_TREE_PEACH_EMPTY2,
		ITEMID_TREE_PEACH_FULL2,
		ITEMID_TREE_PEACH_FALL2,
		ITEMID_TREE_PEAR_EMPTY1,
		ITEMID_TREE_PEAR_FULL1,
		ITEMID_TREE_PEAR_FALL1,
		ITEMID_TREE_PEAR_EMPTY2,
		ITEMID_TREE_PEAR_FULL2,
		ITEMID_TREE_PEAR_FALL2,
		ITEMID_TREE_BANANA1,
		ITEMID_TREE_BANANA2,
		ITEMID_TREE_BANANA3,
		ITEMID_TREE_COCONUT,
		ITEMID_TREE_DATE
	};

	for ( int i = 0; i<COUNTOF( Items_Fruit ); i++ )
	{
		if ( id == Items_Fruit[i] )
			return true;
	}
	return( false );
}

bool CItemBase::IsCrop( ITEMID_TYPE id ) // static
{
	static const WORD Items_Crop[] =
	{
		ITEMID_SPROUT_NORMAL,
		ITEMID_SPROUT_NORMAL2,
		ITEMID_SPROUT_WHEAT1,
		ITEMID_SPROUT_WHEAT2,
		ITEMID_PLANT_WHEAT1,
		ITEMID_PLANT_WHEAT2,
		ITEMID_PLANT_WHEAT3,
		ITEMID_PLANT_WHEAT4,
		ITEMID_PLANT_WHEAT5,
		ITEMID_PLANT_WHEAT6,
		ITEMID_PLANT_WHEAT7,
		ITEMID_PLANT_WHEAT8,
		ITEMID_PLANT_WHEAT9,
		ITEMID_PLANT_COTTON1,
		ITEMID_PLANT_COTTON2,
		ITEMID_PLANT_COTTON3,
		ITEMID_PLANT_COTTON4,
		ITEMID_PLANT_COTTON5,
		ITEMID_PLANT_COTTON6,
		ITEMID_PLANT_HAY1,
		ITEMID_PLANT_HAY2,
		ITEMID_PLANT_HAY3,
		ITEMID_PLANT_HAY4,
		ITEMID_PLANT_HAY5,
		ITEMID_PLANT_HOPS1,
		ITEMID_PLANT_HOPS2,
		ITEMID_PLANT_HOPS3,
		ITEMID_PLANT_HOPS4,
		ITEMID_PLANT_FLAX1,
		ITEMID_PLANT_FLAX2,
		ITEMID_PLANT_FLAX3,
		ITEMID_PLANT_CORN1,
		ITEMID_PLANT_CORN2,
		ITEMID_PLANT_CARROT,
		ITEMID_PLANT_GARLIC1,
		ITEMID_PLANT_GARLIC2,
		ITEMID_PLANT_GENSENG1,
		ITEMID_PLANT_GENSENG2,
		ITEMID_PLANT_TURNIP1,
		ITEMID_PLANT_TURNIP2,
		ITEMID_PLANT_TURNIP3,
		ITEMID_PLANT_ONION,
		ITEMID_PLANT_MANDRAKE1,
		ITEMID_PLANT_MANDRAKE2,
		ITEMID_PLANT_NIGHTSHADE1,
		ITEMID_PLANT_NIGHTSHADE2,
		ITEMID_PLANT_VINE1,
		ITEMID_PLANT_VINE2,
		ITEMID_PLANT_VINE3,
		ITEMID_FRUIT_LETTUCE1,
		ITEMID_FRUIT_LETTUCE2,
	};
	for ( int i = 0; i<COUNTOF( Items_Crop ); i++ )
	{
		if ( id == Items_Crop[i] )
			return true;
	}
	return IsFruit( id );
}

ITEMID_TYPE CItemBase::IsLightUnlit( ITEMID_TYPE id ) // static
{
	// check if we are an unlit lightsource
	switch( id )
	{
	// wall candle facing right (2=dim)
	case 0x09FB:	// 0x09FD-0x09FF
		return( (ITEMID_TYPE) 0x09FD );

	// wall candle facing left (2=dim)
	case 0x0A00:
		return( (ITEMID_TYPE) 0x0A02 );

	// wall torch facing right // 1= bright
	case 0x0A05:	//	0x0A07-0x0A09
		return( (ITEMID_TYPE) 0x0A07 );

	// wall torch facing left // 1 = bright
	case 0x0A0A:	//	0x0A0C-0x0A0E
		return( (ITEMID_TYPE) 0x0A0C );

	// lantern
	case 0x0A18:
		// 0x0A15-0x0A17
		return( (ITEMID_TYPE) 0x0A15 );

	// lantern
	case 0x0A1D:
		//	0x0A1A-0x0A1C
		return( (ITEMID_TYPE) 0x0A1A );

	case 0x0A25:
		// 0x0A15-0x0A17
		return( (ITEMID_TYPE) 0x0A22 );

	// single candle with stick
	case 0x0A26:
		//	0x0B1A-0x0B1C
		return( (ITEMID_TYPE) 0x0B1A );

	// triple candle with stick // 29=very bright
	case 0x0A27:
		//	0x0B10-0x0B12
		return( (ITEMID_TYPE) 0x0B1D );

	// single candle with no stick // 2 = dim
	case 0x0A28:
		//	0x0A0F-0x0A11
		return( (ITEMID_TYPE) 0x0A0F );

	// triple candle with long stick // 29=vbright
	case 0x0A29:
		//	0x0B26-0x0B28
		return( (ITEMID_TYPE) 0x0B26 );

	// standard street lamp	// 29=vbright
	case 0x0B21:
		//	0x0B22
		return( (ITEMID_TYPE) 0x0B20 );

	// plain street lamp	// 29=vbright
	case 0x0B23:
		//	0x0B24
		return( (ITEMID_TYPE) 0x0B22 );

	// fancy street lamp	// 29=vbright
	case 0x0B25:
		//	0x0B26
		return( (ITEMID_TYPE) 0x0B24 );

	case 0x0f64:	// torch.
	case 0x0f6b:
		// 0x0A12-0x0A14
		return( (ITEMID_TYPE) 0x0a12 );

	case 0x142f:	// candle.
		return( (ITEMID_TYPE) 0x142c );
	case 0x1433:	// candle.
		return( (ITEMID_TYPE) 0x1430 );
	case 0x1437:	// candle.
		return( (ITEMID_TYPE) 0x1434 );

	// brazier
	// 0x???   0x0E31-0x0E33 */

	}

	return((ITEMID_TYPE) 0);
}

ITEMID_TYPE CItemBase::IsLightLit( ITEMID_TYPE id ) // static
{
	// check if we are a lit lightsource (that can be unlit)
	switch ( id )
	{
	// wall candle facing right
	case 0x09FD:
	case 0x09FE:
	case 0x09FF:
		return( (ITEMID_TYPE) 0x09FB );

	// wall candle facing left
	case 0x0A02:
	case 0x0A03:
	case 0x0A04:
		return( (ITEMID_TYPE) 0x0A00 );

	// wall torch facing right
	case 0x0A07:
	case 0x0A08:
	case 0x0A09:
		return( (ITEMID_TYPE) 0x0A05 );

	// wall torch facing left
	case 0x0A0C:
	case 0x0A0D:
	case 0x0A0E:
		return( (ITEMID_TYPE) 0x0A0a);

	// single candle with no stick
	case 0x0A0F:
	case 0x0A10:
	case 0x0A11:
		return( (ITEMID_TYPE) 0x0A28 );

	// lit torches
	case 0xa12:
	case 0xa13:
	case 0xa14:
		return( (ITEMID_TYPE) 0x0f64 );

	// lantern
	case 0x0A15:
	case 0x0A16:
	case 0x0A17:
		return( (ITEMID_TYPE) 0x0A18 );

	// lantern
	case 0x0A1A:
	case 0x0A1B:
	case 0x0A1C:
		return( (ITEMID_TYPE) 0x0A1d );

	case 0x0A22:
	case 0x0A23:
	case 0x0A24:
		return( (ITEMID_TYPE) 0x0A25 );

	// single candle with stick
	case 0x0B1A:
	case 0x0B1B:
	case 0x0B1C:
		return( (ITEMID_TYPE) 0x0A26 );

	// triple candle with stick
	case 0x0B1D:
	case 0x0B1E:
	case 0x0B1F:
		return( (ITEMID_TYPE) 0x0A27 );

	// triple candle with long stick
	case 0x0B26:
	case 0x0B27:
	case 0x0B28:
		return( (ITEMID_TYPE) 0x0A29 );

	// standard street lamp
	case 0x0B20:
		return( (ITEMID_TYPE) 0x0b21 );

	// plain street lamp
	case 0x0B22:
		return( (ITEMID_TYPE) 0x0b23 );

	// fancy street lamp
	case 0x0B24:
		return( (ITEMID_TYPE) 0x0b25 );

	// candles.
	case 0x142c:
	case 0x142d:
	case 0x142e:
		return( (ITEMID_TYPE) 0x142f );
	case 0x1430:
	case 0x1431:
	case 0x1432:
		return( (ITEMID_TYPE) 0x1433 );
	case 0x1434:
	case 0x1435:
	case 0x1436:
		return( (ITEMID_TYPE) 0x1437 );

	// campfire
	case 0x0DE1:
	case 0x0dE2:
//			0x0DE3-0x0DE8
		break;
	}

	return((ITEMID_TYPE) 0);
}

ITEMID_TYPE CItemBase::IsRawFood( ITEMID_TYPE id ) // static
{
	// Return the cooked version of the food.
	// ??? move this to ITEM.SCP 

	static const WORD Items_FoodRaw[][2] =	// convert raw food to cooked food.
	{
		ITEMID_FOOD_MEAT_RAW,	ITEMID_FOOD_MEAT,
		ITEMID_FOOD_PIZZA_RAW,	ITEMID_FOOD_PIZZA,
		ITEMID_FOOD_FISH_RAW,	ITEMID_FOOD_FISH,
		ITEMID_FOOD_PIE_RAW,	ITEMID_FOOD_PIE,
		ITEMID_FOOD_DOUGH_RAW,  ITEMID_FOOD_BREAD,
		ITEMID_FOOD_COOKIE_RAW, ITEMID_FOOD_COOKIES,
		ITEMID_FOOD_EGGS_RAW,	ITEMID_FOOD_EGGS,
		ITEMID_FOOD_BIRD1_RAW,	ITEMID_FOOD_BIRD1,
		ITEMID_FOOD_BIRD2_RAW,	ITEMID_FOOD_BIRD2,
		ITEMID_FOOD_LEG1_RAW,	ITEMID_FOOD_LEG1,
		ITEMID_FOOD_LEG2_RAW,	ITEMID_FOOD_LEG2,
	};
	for ( int i = 0; i<COUNTOF( Items_FoodRaw ); i++ )
	{
		if ( id == Items_FoodRaw[i][0] )
			return( (ITEMID_TYPE) Items_FoodRaw[i][1] );
	}
	return( ITEMID_NOTHING );
}

bool CItemBase::IsSameDispID( ITEMID_TYPE id ) const
{
	// Does this item look like the item we want ?
	// Take into account flipped items, ignore custom items.

	if ( ! IsValidDispID( id ))	// this should really not be here but handle it anyhow.
	{
		return( GetID() == id );
	}

	if ( id == GetDispID())
		return( true );

	for ( int i=0; i<m_flip_id.GetCount(); i ++ )
	{
		if ( m_flip_id[i] == id )
			return( true );
	}
	return( false );
}

enum IC_TYPE
{
	IC_BUYVALUE,
	IC_DEF,			// Get rid of this !
	IC_DUPELIST,
	IC_DYE,
	IC_ID,
	IC_LAYER,
	IC_MATERIAL,
	IC_REPAIR,
	IC_REQSTR,
	IC_SELLVALUE,
	IC_TWOHANDS,
	IC_TYPE,
	IC_WEIGHT,
};

const TCHAR * CItemBase::sm_KeyTable[] =
{
	"BUYVALUE",
	"DEF",			// Get rid of this !
	"DUPELIST",
	"DYE",
	"ID",
	"LAYER",
	"MATERIAL",
	"REPAIR",
	"REQSTR",
	"SELLVALUE",
	"TWOHANDS",
	"TYPE",
	"WEIGHT",
};

bool CItemBase::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pChar )
{
	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case IC_BUYVALUE:
		if ( m_buyvaluemin != m_buyvaluemax )
			sVal.Format( "%d-%d", m_buyvaluemin, m_buyvaluemax );
		else
			sVal.Format( "%d", m_buyvaluemin );
		break;
	case IC_DEF:
		return( false );
	case IC_DUPELIST:
		{
			TCHAR szTemp[ MAX_SCRIPT_LINE_LEN ];
			int iLen = 0;
			*szTemp = '\0';
			for ( int i=0; i<m_flip_id.GetCount(); i++ )
			{
				if ( i ) iLen += strcpylen( szTemp+iLen, "," );
				iLen += sprintf( szTemp+iLen, "0%x", m_flip_id[i] );
			}
			sVal = szTemp;
		}
		break;
	case IC_DYE:
		sVal.FormatHex(( m_Can & CAN_I_DYE ) ? true : false );
		break;
	case IC_ID:
		sVal.FormatHex( GetDispID());
		break;
	case IC_LAYER:
		sVal.FormatVal( m_layer );
		break;
	case IC_MATERIAL:
		// What type of material is it made of ?
		sVal.FormatVal( m_Material );
		break;
	case IC_REPAIR:
		sVal.FormatHex(( m_Can & CAN_I_REPAIR ) ? true : false );
		break;
	case IC_REQSTR:
		sVal.FormatVal( m_StrReq );
		break;
	case IC_SELLVALUE:
		if ( m_sellvaluemin != m_sellvaluemax )
			sVal.Format( "%d-%d", m_sellvaluemin, m_sellvaluemax );
		else
			sVal.Format( "%d", m_sellvaluemin );
		break;
	case IC_TWOHANDS:
		// In some cases the layer is not set right.
		// override the layer here.
		break;
	case IC_TYPE:
		sVal.FormatVal( m_type );
		break;
	case IC_WEIGHT:
		sVal.FormatVal( m_weight / WEIGHT_UNITS );
		break;
	default:
		return( CBaseBase::r_WriteVal( pKey, sVal ));
	}
	return( true );
}

bool CItemBase::r_LoadVal( CScript &s )
{
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case IC_BUYVALUE:
		m_buyvaluemin = Exp_GetSingle( s.m_pArg );
		if ( s.m_pArg[0] )
		{
			s.m_pArg ++;
			m_buyvaluemax = Exp_GetSingle( s.m_pArg );
		}
		else
		{
			m_buyvaluemax = m_buyvaluemin;
		}
		break;
	case IC_DEF:
		{
			// This is the global def for this item.
			CGString sVal;
			sVal.FormatVal( GetBaseID());
			g_Exp.SetVarDef( s.GetArgStr(), sVal );
		}
		break;
	case IC_DUPELIST:
		{
#ifdef _DEBUG
			if ( g_Serv.m_iExitCode == 5 )	// special mode. ignore this
				break;
#endif
			TCHAR * ppArgs[512];
			int iArgQty = ParseCmds( s.GetArgStr(), ppArgs, COUNTOF(ppArgs));
			if ( iArgQty <= 0 )
				return( false );
			for ( int i=0; i<iArgQty; i++ )
			{
				ITEMID_TYPE id = (ITEMID_TYPE) Exp_GetHex( ppArgs[i] );
				if ( ! IsValidDispID( id ))
					continue;
				if ( IsSameDispID(id))
					continue;
				m_flip_id.Add(id);
			}
		}
		break;
	case IC_DYE:
		m_Can |= ( s.GetArgVal()) ? CAN_I_DYE : 0;
		break;
	case IC_ID:
		{
			ITEMID_TYPE id = (ITEMID_TYPE) s.GetArgHex();
			if ( GetID() < ITEMID_MULTI || ! IsValidDispID(id))
			{
				DEBUG_ERR(( "Script setting new id=%x for base type [%x]\n", id, GetID()));
				return( false );
			}

			CItemBase * pDef = FindItemBase( id );	// make sure the base is loaded.
			if ( ! pDef )
			{
				DEBUG_ERR(( "Script setting base base id=%x for [%x]\n", id, GetID()));
				return( false );
			}

			ITEM_TYPE oldtype = m_type;

			// For custom script items.
			CGrayItemInfo tile( id );
			SetBaseDispID( id, tile );	// set all the default stuff.

			// Take type default from the base ID ? if not already set.
			if ( oldtype != ITEM_NORMAL )
			{
				m_type = oldtype;
			}
			else
			{
				m_type = pDef->m_type;
			}
		}
		break;

	case IC_LAYER:
		m_layer = (LAYER_TYPE) s.GetArgVal();	// GetEquipLayer()
		break;
	case IC_MATERIAL:
		// What type of material is it made of ?
		m_Material = (MATERIAL_TYPE) s.GetArgVal();
		break;
	case IC_REPAIR:
		m_Can |= ( s.GetArgVal()) ? CAN_I_REPAIR : 0;
		break;
	case IC_REQSTR:
		m_StrReq = s.GetArgVal();
		break;
	case IC_SELLVALUE:
		m_sellvaluemin = Exp_GetSingle( s.m_pArg );
		if ( s.m_pArg[0] )
		{
			s.m_pArg ++;
			m_sellvaluemax = Exp_GetSingle( s.m_pArg );
		}
		else
		{
			m_sellvaluemax = m_sellvaluemin;
		}
		break;
	case IC_TWOHANDS:
		// In some cases the layer is not set right.
		// override the layer here.
		if ( s.GetArgStr()[0] == '1' || s.GetArgStr()[0] == 'Y' || s.GetArgStr()[0] == 'y' )
		{
			m_layer = LAYER_HAND2;
		}
		break;
	case IC_TYPE:
		m_type = (ITEM_TYPE) s.GetArgVal();
		if ( m_type == ITEM_CONTAINER_LOCKED )
		{
			// At this level it just means to add a key for it.
			m_type = ITEM_CONTAINER;
		}
		break;
	case IC_WEIGHT:
		// Read in the weight but it may not be decimalized correctly
		{
			bool fDecimal = ( strchr( s.GetArgStr(), '.' ) != NULL );
			m_weight = s.GetArgVal();
			if ( ! fDecimal )
			{
				m_weight *= WEIGHT_UNITS;
			}
		}
		break;
	default:
		return( CBaseBase::r_LoadVal( s ));
	}
	return( true );
}

void CItemBase::ReplaceItemBase( CItemBase * pOld, CBaseStub * pNew ) //static
{
	ASSERT(pOld);
	int index = g_Serv.m_ItemBase.FindPtr( pOld );
	ASSERT( index >= 0 );
	ASSERT( pOld->GetInstances() == 0 );
	g_Serv.m_ItemBase.DeleteAt( index );

	ASSERT(pNew);
	g_Serv.m_ItemBase.InsertAt( index, pNew );
}

CItemBase * CItemBase::MakeDupeReplacement( CItemBase * pBase, ITEMID_TYPE iddupe ) // static
{
	ITEMID_TYPE id = pBase->GetID();
	if ( iddupe == id || ! IsValidDispID( iddupe ))
	{
		DEBUG_ERR(( "CItemBase:DUPEITEM weirdness %x==%x\n", id, iddupe ));
		return( pBase );
	}

	CItemBase * pBaseNew = FindItemBase( iddupe );
	if ( pBaseNew == NULL )
	{
		DEBUG_ERR(( "CItemBase:DUPEITEM not exist %x==%x\n", id, iddupe ));
		return( pBase );
	}

	if ( ! pBaseNew->IsSameDispID(id))	// already here ?!
	{
		pBaseNew->m_flip_id.Add(id);
	}

	// create the dupe stub.
	CItemBaseDupe * pBaseDupe = new CItemBaseDupe( id, iddupe );
	ReplaceItemBase( pBase, pBaseDupe );

	return( pBaseNew );
}

CItemBase * CItemBase::SetMultiRegion( CItemBase * pBase, CScript & s, const CUOItemTypeRec &tile ) // static
{
	// "COMPONENT"
	// We must transform this object into a CItemBaseMulti

	ASSERT( pBase );
	if ( pBase->m_type != ITEM_MULTI && pBase->m_type != ITEM_SHIP )
	{
		DEBUG_ERR(( "Components defined for NON-MULTI type 0%x\n", pBase->GetID()));
		return pBase;
	}

	CItemBaseMulti * pMultiBase = dynamic_cast <CItemBaseMulti *>(pBase);
	if ( pMultiBase == NULL )
	{
		pMultiBase = new CItemBaseMulti( pBase, tile );
		ReplaceItemBase( pBase, pMultiBase );
	}

	pMultiBase->SetMultiRegion( s.GetArgStr());
	return( pMultiBase );
}

CBaseBaseArray * CItemBase::GetBaseArray() const
{
	return( &(g_Serv.m_ItemBase));
}

CItemBase * CItemBase::FindItemBase( ITEMID_TYPE id ) // static
{
	// CItemBase
	// is a like item already loaded.

	if ( id <= 0 || id >= ITEMID_QTY )
		return( NULL );

	CItemBase * pBase = NULL;

	// find it (or near it) if already loaded.
	int index = g_Serv.m_ItemBase.FindKey( id );
	if ( index >= 0 )	// found it.
	{
		CBaseStub * pBaseStub = g_Serv.m_ItemBase[index];
		const CItemBaseDupe * pBaseDupe = dynamic_cast <const CItemBaseDupe *>(pBaseStub);
		if ( pBaseDupe )
		{
			return( FindItemBase( pBaseDupe->m_wIndexDupe ));
		}

		pBase = dynamic_cast <CItemBase *>(pBaseStub);
		ASSERT(pBase);
		if ( pBase->IsLoaded())
			return( pBase );
		// I have it but it needs to be loaded.
	}

	// read it in from the script and *.mul files.

	CUOItemTypeRec item;
	if ( IsValidDispID( id ))
	{
		if ( ! CItemBase::GetItemData( id, &item ))
		{
			return( NULL );
		}
	}

	if ( pBase == NULL )
	{
		pBase = new CItemBase( id, item );
		// Insert new in sorted order.
		g_Serv.m_ItemBase.AddSortKey( pBase, id );
	}

	// Find the previous one in the series if any.
	// Find it's script section offset.

	CScriptLock s;
	if ( ! pBase->ScriptLockBase(s))
	{
		if ( IsValidDispID(id))		// it is in the base display set. so allow it.
		{
			return( pBase );
		}

		// must be scripted. not in the artwork set.
		g_Log.Event( LOGL_ERROR, "UN-scripted item 0%0x NOT allowed\n", id );

		if ( pBase->GetInstances() == 0 ) // scripts got swaped out ?!
		{
			bool fCheck = g_Serv.m_ItemBase.DeleteOb( pBase );
			ASSERT( fCheck );
		}
		else
		{
			pBase->UnLoad();
		}
		return( NULL );
	}

	// Read the Script file preliminary.
	while ( s.ReadKeyParse())
	{
		if ( s.IsKey( "DUPEITEM" ))
		{
			return( MakeDupeReplacement( pBase, (ITEMID_TYPE) s.GetArgHex()));
		}
		if ( s.IsKey( "MULTIREGION" ))
		{
			// Upgrade the CItemBase::pBase to the CItemBaseMulti.
			pBase = SetMultiRegion( pBase, s, item );
			continue;
		}
		if ( s.IsKeyHead( "ON", 2 ))	// trigger scripting marks the end
		{
			pBase->m_Can |= CAN_TRIGGER;
			break;
		}
		pBase->r_LoadVal( s );
	}

	return( pBase );
}

////////////////////////////////////////////////////////

CItemBaseMulti::CItemBaseMulti( const CItemBase* pBase, const CUOItemTypeRec &tile ) :
	CItemBase( pBase->GetID(), tile )
{
	m_dwRegionFlags = REGION_FLAG_NODECAY | REGION_ANTIMAGIC_TELEPORT | REGION_ANTIMAGIC_RECALL_IN | REGION_FLAG_NOBUILDING;
	m_rect.Empty();
	// copy the stuff from the pBase 
	DupeCopy(pBase);
}

bool CItemBaseMulti::AddComponent( ITEMID_TYPE id, signed short dx, signed short dy, signed char dz )
{
	m_rect.UnionPoint( dx, dy );
	if ( id > 0 )	// we can add a phantom item just to increase the size.
	{
		if ( id >= ITEMID_MULTI ) 
			return false;

		CMultiComponentItem comp;
		comp.m_id = id;
		comp.m_dx = dx;
		comp.m_dy = dy;
		comp.m_dz = dz;
		m_Components.Add( comp );
	}

	return( true );
}

void CItemBaseMulti::SetMultiRegion( TCHAR * pArgs )
{
	int piArgs[4];
	int iQty = ParseCmds( pArgs, piArgs, COUNTOF(piArgs));
	if ( iQty <= 1 )
		return;
	m_Components.Empty();	// might be after a resync
	m_rect.SetRect( piArgs[0], piArgs[1], piArgs[2], piArgs[3] );
}

bool CItemBaseMulti::AddComponent( TCHAR * pArgs )
{
	int piArgs[4];
	int iQty = ParseCmds( pArgs, piArgs, COUNTOF(piArgs));
	if ( iQty <= 1 )
		return false;
	return AddComponent( (ITEMID_TYPE) piArgs[0], piArgs[1], piArgs[2], piArgs[3] );
}

bool CItemBaseMulti::r_LoadVal( CScript &s )
{
	if ( s.IsKey( "COMPONENT" ))
	{
		return AddComponent( s.GetArgStr());
	}
	if ( s.IsKey( "REGIONFLAGS" ))
	{
		m_dwRegionFlags = s.GetArgVal();
		return( true );
	}
	return( CItemBase::r_LoadVal( s ));
}

