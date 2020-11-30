//
// CItem.cpp
//

#include "graysvr.h"	// predef header.

#ifdef COMMENT
const CWeaponBase Item_Weapons[] =	// All weapons have a left and right flip.
{

0df0: 02,F40,40,00,W06,L02,?00000004,A00000269,?0008,H00	// 'black staff'
0e81: 02,F40,40,00,W04,L02,?00000020,A0000026d,?0000,H00	// 'shepherd's crook'
0e85: 02,F40,40,00,W0b,L01,?0000001c,A0000027b,?0009,H00	// 'pickaxe'
0e87: 02,F40,40,04,W0b,L02,?0000001d,A0000027c,?0007,H00	// 'pitchfork'
0e89: 02,F40,40,04,W04,L02,?0000001e,A00000288,?0006,H00	// 'quarter staff'
0ec2: 02,F00,40,04,W02,L01,?00000019,A0000026b,?0004,H00	// 'cleaver'
0ec4: 02,F00,40,00,W01,L01,?00000021,A0000027e,?0005,H00	// 'skinning knife'
0f43: 02,F40,40,00,W04,L02,?00000172,A00000267,?0006,H00	// 'hatchet'
0f45: 02,F00,40,00,W08,L02,?00000174,A00000265,?000f,H00	// 'executioner's axe'
0f47: 02,F00,40,00,W04,L01,?00000002,A00000267,?000c,H00	// 'battle axe'
0f49: 02,F80,40,04,W04,L02,?00000174,A0000028d,?000f,H00	// 'axe'
0f4b: 02,F40,40,00,W08,L02,?00000026,A0000027a,?0009,H00	// 'double axe'
0f4d: 02,F00,40,00,W07,L02,?00000001,A00000266,?001e,H00	// 'bardiche'
0f4f: 02,F40,40,00,W07,L01,?00000009,A0000028b,?0014,H00	// 'crossbow'
0f51: 02,F40,40,00,W01,L01,?0000000c,A0000026e,?000a,H00	// 'dagger'
0f5c: 02,F40,40,04,W0e,L01,?00000017,A00000277,?0009,H00	// 'mace'
0f5e: 02,F40,40,04,W06,L01,?00000013,A0000026a,?000f,H00	// 'broadsword'
0f60: 02,F40,40,00,W07,L01,?00000014,A0000026a,?0017,H00	// 'longsword'
0f62: 02,F40,40,04,W07,L02,?00000024,A00000281,?000b,H00	// 'spear'
0fb4: 02,F40,40,00,W0a,L01,?00000022,A00000280,?0006,H00	// 'sledge hammer'
102a: 02,F40,00,00,W02,L01,?00000001,A00000000,?0005,H00	// 'hammer'
13af: 02,F40,40,00,W08,L01,?00000173,A00000284,?000d,H00	// 'war axe'
13b1: 02,F40,40,00,W06,L01,?00000005,A00000289,?0011,H00	// 'bow'
13b3: 02,F40,40,04,W09,L01,?00000008,A0000026c,?0006,H00	// 'club'
13b5: 02,F40,40,00,W05,L01,?0000001f,A0000027d,?000d,H00	// 'scimitar'
13b7: 02,F40,40,04,W01,L01,?00000014,A0000026a,?000c,H00	// 'long sword'
13b9: 02,F40,40,04,W06,L01,?00000016,A00000283,?000f,H00	// 'viking sword'
13e3: 02,F40,40,00,W08,L01,?00000022,A00000280,?0008,H00	// 'smith's hammer'
13f4: 02,F40,40,04,W03,L02,?00000020,A0000026d,?0007,H00	// 'crook'
13f6: 02,F40,40,00,W01,L01,?00000007,A00000275,?0005,H00	// 'butcher knife'
13f8: 02,F40,40,00,W03,L02,?0000000e,A00000274,?0005,H00	// 'gnarled staff'
13fa: 02,F40,40,04,W06,L02,?00000003,A00000263,?0009,H00	// 'large battle axe'
13fc: 02,F40,40,04,W09,L01,?0000000a,A00000268,?0023,H00	// 'heavy crossbow'
13fe: 02,F40,40,04,W06,L01,?00000011,A00000273,?000a,H00	// 'katana'
1400: 02,F40,40,04,W02,L01,?00000012,A00000276,?0009,H00	// 'kryss'
1402: 02,F40,40,00,W04,L02,?00000023,A0000027f,?0007,H00	// 'short spear'
1404: 02,F40,40,00,W09,L01,?00000027,A00000285,?0008,H00	// 'war fork'
1406: 02,F40,40,00,W11,L01,?00000029,A00000282,?000b,H00	// 'war mace'
1438: 02,F40,40,00,W0a,L01,?00000028,A00000286,?0008,H00	// 'war hammer'
143a: 02,F40,40,00,W0a,L01,?00000018,A00000279,?0007,H00	// 'maul'
143c: 02,F40,40,00,W09,L01,?00000010,A00000271,?0007,H00	// 'hammer pick'
143e: 02,F40,40,00,W10,L02,?0000000f,A00000270,?000f,H00	// 'halberd'
1440: 02,F40,40,00,W08,L01,?0000000b,A0000026f,?0006,H00	// 'cutlass'
1442: 02,F40,40,00,W08,L02,?00000025,A00000264,?0007,H00	// 'two handed axe'
166e: 02,F00,00,00,W01,L00,?00000000,A00000000,?000a,H00	// 'whip'

};
#endif	

/////////////////////////////////////////////////////////////////
// -CItemBase

#ifdef COMMENT

BYTE CItem :: IsReadGumpID() const // grave stones and such
{

	return( 0x64 ); // plaque
	return( 0x65 ); // grave stone

}

#endif

int CItem :: IsMusicalID( ITEMID_TYPE id )
{
static const WORD Item_Musical[] = 
{
	0x0e9c,
	0x0e9D,
	0x0e9E,
	0x0eB1,
	0x0eB2,
	0x0eB3,
	0x0eB4,
};
	return( FindID( id, Item_Musical, COUNTOF( Item_Musical )) + 1 );
}

bool CItem :: IsTrapID( ITEMID_TYPE id )
{
	switch ( id )
	{
	case 0x10f5:	// stone face. k
	case 0x10fc: // stone face W k
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
	case 0x11c0: // crumble floor. ok
		return( true );
	}

	return( false );
}

bool CItem :: IsWaterID( ITEMID_TYPE id )
{
	if ( id >= 0x1796 && id <= 0x17b2 )
		return( true );
	if ( id == 0x1559 )
		return( true );
	return( false );
}

int CItem :: IsDoorID( ITEMID_TYPE id )
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
	ITEMID_DOOR_WOODENGATE_1,
	ITEMID_DOOR_IRONGATE,
	ITEMID_DOOR_WOODENGATE_2,
};

	for ( int i=0;i<COUNTOF(Item_DoorBase);i++)
	{
		int did = id - Item_DoorBase[i];
		if ( did >= 0 && did <= 15 ) return( did+1 );
	}
	return( 0 );
}

WORD CItem :: IsContainerID( ITEMID_TYPE id )
{
	// return the container gump id.

	switch ( id )
	{
	case ITEMID_CORPSE:     return( 0x09 );  // Coffin

	case ITEMID_BACKPACK2:
	case ITEMID_BACKPACK:	return( 0x3C );  // 60 = Backpack (52?)
	case ITEMID_BAG:		return( 0x3D );  // Leather Bag
	case ITEMID_BARREL:		return( 0x3E );  // Barrel
	case ITEMID_BASIN:
	case ITEMID_RBASKET:	return( 0x41 );  // Round Basket
	case ITEMID_POUCH:		return( 0x40 );  // Box/Pouch
	case ITEMID_SBASKET:	return( 0x3F );  // Square picknick Basket

	case ITEMID_CHEST1:
	case ITEMID_CHEST2:	
	case ITEMID_CHEST3:		return( 0x49 );	// wood chest ?
	case ITEMID_CHEST4:		return( 0x42 ); // 2 color metal chest.

	case ITEMID_Bulletin:
	case ITEMID_BANK_BOX:
	case ITEMID_CHEST_METAL:
	case ITEMID_CHEST_METAL2: return( 0x4A );  // Silver Chest
	case ITEMID_WOODBOX:	return( 0x43 );  // Wooden Box

	case ITEMID_CRATE1:
	case ITEMID_CRATE2:
	case ITEMID_CRATE3:
	case ITEMID_CRATE4:
	case ITEMID_CRATE:		return( 0x44 );	// Wooden Crate
	case ITEMID_KEG:		return( 0x3E );  // Keg
	case ITEMID_BRASSBOX:	return( 0x4B );  // Brass Box

	case 0x0a97: // book shelf
	case 0x0a98: // book shelf
	case 0x0a99: // book shelf
	case 0x0a9a: // book shelf 
	case 0x0a9b: // book shelf
	case 0x0a9c: // book shelf
	case 0x0a9d: // book shelf 
	case 0x0a9e: // book shelf
		// 69 = book shelf
		return( 0x4d );	// ???

	case 0x0a2c: // light drawers
	case 0x0a34: // light drawers.
	case 0x0a3c: // light dresser drawers.
	case 0x0a3d: // light dresser drawers.	
	case 0x0a44: // light dresser drawers.
	case 0x0a45: // light dresser drawers.
		// 73 = light drawer
		return( 0x51 );

	case 0x0a30: // dark drawers.
	case 0x0a38: // dark drawers.
		// 64 = dark drawer
		return( 0x48 );

	case 0x0a4c: // dark armoire.
	case 0x0a4d: // dark armoire.
	case 0x0a50: // dark armoire.
	case 0x0a51: // dark armoire.
		// 70 = dark armoire.
		return( 0x4e );

	case 0x0a4e: // light armoire.
	case 0x0a4f: // light armoire.
	case 0x0a52: // light armoire.
	case 0x0a53: // light armoire.
		// 71 = light armoire.
		return( 0x4f );

	case ITEMID_GAME_BACKGAM:	// backgammon board.
		return( 0x92e );

	case ITEMID_GAME_BOARD:	// game board.
		return( 0x91a );

	case 0x3e93:
		// 68 = ship hold
		return( 0x4c );
	}

	return( 0 );	// guess it's not a container.
}

CItemBase * FindItemBase( int id )
{
	// ItemBase
	// is a like item already loaded.

	// ??? optimize this
	CItemBase * pBase = static_cast <CItemBase*>( ItemBase.GetHead());
	for ( ; pBase!=NULL; pBase = static_cast <CItemBase*>( pBase->GetNext()))
	{
		// is it like a flipped version of this ?
		if ( pBase->IsSameID( id ))
			return( pBase );
	}

	// Else read it in from the script and *.mul files.

	CUOTileItemBlock item;
 	if ( ! World.GetItemData( (ITEMID_TYPE) id, &item ))
	{
		if ( id < ITEMID_TRACK_BEGIN || id > ITEMID_TRACK_END )	// preserve these
			return( NULL );
	}

	pBase = new CItemBase( id, item );
	ItemBase.InsertAfter( pBase );
	return( pBase );
}

CItemBase :: CItemBase( int id, CUOTileItemBlock &item )
{
	m_id = id;
	m_flip_id = NULL;

	// Script read stuff.
	// pBase->m_disp_id = id;	// Base display id	
	if ( CItem::IsContainerID( (ITEMID_TYPE) id )) 
		m_type = ITEM_CONTAINER;
	else if ( CItem::IsDoorID( (ITEMID_TYPE) id ))
		m_type = ITEM_DOOR;
	else if ( CItem::IsMusicalID( (ITEMID_TYPE) id ))
		m_type = ITEM_MUSICAL;
	else if ( CItem::IsTrapID( (ITEMID_TYPE) id ))
		m_type = ITEM_TRAP;
	else if ( CItem::IsWaterID( (ITEMID_TYPE) id ))
		m_type = ITEM_WATER;
	else
		m_type = ITEM_NORMAL;	// Get from script i guess.
	m_buyvalue = 0;
	m_sellvalue = 0;

	// Stuff read from .mul file.
	// Some items (like hair) have no names !
	m_sName = item.m_name;	// default type name.
	m_Can = ( item.m_flag2 & 0x08 ) ? CAN_PILE : 0;
	m_weight = item.m_weight; // ! movable = ( item.m_weight == 0xFF )
	m_layer = ( item.m_type == 0x02 || item.m_type == 0x00 ) ? (LAYER_TYPE) item.m_layer : LAYER_NONE ;
	if ( id == ITEMID_SPELLBOOK || id == ITEMID_SPELLBOOK2 )
		m_layer = LAYER_HAND1;
	if ( id == ITEMID_BACKPACK2 )
		m_layer = LAYER_PACK;

	// m_height;

	m_attack = 0;
}

void CItemBase :: LoadSameID( int id )
{
	// add this to the "same id" list.
	// The current pItem is a dupe.

	if ( id == m_id ) return;

	ITEMID_TYPE * pOld = m_flip_id;
	int iCount = 0;
	if ( pOld != NULL )
	{
		for ( ; pOld[iCount] != ITEMID_NOTHING; iCount ++ )
		{
			if ( pOld[iCount] == id )
				return;
		}
	}

	// re-alloc new array for dupe item.
	//
	m_flip_id = new ITEMID_TYPE [ iCount+2 ];
	memcpy( m_flip_id, pOld, sizeof(ITEMID_TYPE)*iCount );
	m_flip_id[iCount++] = (ITEMID_TYPE) id;
	m_flip_id[iCount] = ITEMID_NOTHING;
	if ( pOld != NULL ) delete [] pOld;
}

bool CItemBase :: IsSameID( int id ) const
{
	// Is this a like item
	if ( id == m_id ) return( true );
	if ( m_flip_id == NULL ) return( false );
	for ( int i=0; m_flip_id[i] != ITEMID_NOTHING; i ++ )
	{
		if ( m_flip_id[i] == (ITEMID_TYPE) id )
			return( true );
	}
	return( false );
}

/////////////////////////////////////////////////////////////////
// -CItem

CItem :: CItem( ITEMID_TYPE id ) : CObjBase( id, true )
{
	sm_iCount++;
	SetID( id );	// virtuals not called by constructurs.

	m_type = m_pDef->m_type;
	// m_layer = m_pDef->m_layer;
	m_Attr = 0;
	m_amount = 1;
	m_more1 = 0;
	m_more2 = 0;
}

CItem :: ~CItem()
{
	RemoveSelf();	// Must remove early or else virtuals will fail.
	if ( m_type == ITEM_SPAWN )
	{
		Kill_Spawn_Children();
	}
	sm_iCount--;
}

CItem * ItemCreateBase( ITEMID_TYPE id )
{
	// NOTE: This is a free floating item. 
	//  If not put on ground or in container after this = Memory leak !
	CItem * pItem;
	if ( CItem :: IsContainerID( id ))
	{
		pItem = new CContainerItem( id );
	}
	else
	{
		pItem = new CItem( id );
	}
	return( pItem );
}

CItem * ItemCreateDupe( CItem * pItem )
{
	// Dupe this item.
	CItem * pItemNew = ItemCreateBase( (ITEMID_TYPE) pItem->GetID() );
	pItemNew->Dupe( pItem );
	return( pItemNew );
}

CItem * ItemCreateScript( int id )
{
	// Create item from the script id.

	// default item.
	CItem * pItem = ItemCreateBase( (ITEMID_TYPE) id );

	CString sSec;
	sSec.Format( "ITEM %04X", id );
	if ( World.m_scrItems.FindSec(sSec))
	{
		while ( 1 )
		{
			if ( ! World.m_scrItems.ReadParse()) break;
			pItem->LoadVal( World.m_scrItems.m_Line, World.m_scrItems.m_pArg );
		}
	}
	return( pItem );
}

void CItem :: SetID( ITEMID_TYPE id )
{
	// Creating invalid objects of any sort can crash clients.
	if ( id < 0 || id >= ITEMID_QTY ) id = ITEMID_GOLD;

	m_pDef = FindItemBase( id );
	if ( m_pDef == NULL )
	{
		DEBUG_ERR(( "ERROR: Creating invalid object %xh\n", id ));
		id = ITEMID_GOLD;
		m_pDef = FindItemBase( id );
	}

	SetPrivateID( id );
}

CObjBase * CItem :: GetContainer() const
{
	// What is this CItem contained in ?
	// Container could be a CChar or CContainerItem
	CObList * pList = GetParent();
	if ( pList == NULL ) return( NULL ); // should only happen on loading.
	CContainer * pCont = pList->GetThisContainer();
	if ( pCont == NULL ) return( NULL ); // on ground ?
	return( pCont->GetObject() );
}

bool CItem :: LoadSetContainer( CObjUID uid, LAYER_TYPE layer )
{
	// Set the CItem in a continer of some sort.
	// Used only during load.
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) 
	{
		DEBUG_ERR(( "Invalid container %lxh\n", (DWORD) uid ));
		return( false );	// not valid object.
	}
	CContainer * pCont = pObj->GetThisContainer();
	if ( pCont == NULL )
	{
		DEBUG_ERR(( "Non container %lxh\n", (DWORD) uid ));
		return( false );	// not a container.
	}
	if ( ! pObj->IsItem())
		(dynamic_cast <CChar*> (pObj))->LayerAdd( this, layer ? layer : m_pDef->m_layer );
	else
		pCont->ContentAdd( this );

	return( true );
}

CObjBase * CItem :: GetTopLevelObj( void ) const
{
	CObjBase * pObj = GetContainer();
	if ( pObj == NULL ) return( (CObjBase *) this );	// Get rid of const.
	return( pObj->GetTopLevelObj());
}

bool CItem :: IsValid() const
{
	// Does item i have a "link to reality"? 
	// (Is the container it is in still there)
	//
	if ( ! CObjBase::IsValid()) 
		return( false );

	if ( m_pDef == NULL ) return( false );
	if ( m_pDef->m_id == 0 )
	{
		return( false );
	}

	if ( IsTopLevel()) 
	{
		// no container = must be at valid point.
		return( m_p.IsValid());
	}

	// The container must be valid.
	return( GetContainer()->IsValid());
}

bool CItem :: IsSameType( CObjBase * pObj ) const
{
	if ( ! pObj->IsItem()) return( false );

	CItem * pItem = dynamic_cast <CItem*> ( pObj );
	if ( ! IsSameID( pItem->GetID())) return( false );
	if ( m_type != pItem->m_type ) return( false );
	if ( m_color != pItem->m_color ) return( false );

	return( true );
}

bool CItem :: IsStackable( CItem * pItem ) const
{
	// try object rotations ??? left and right hand hides ?
	if ( ! ( m_pDef->m_Can & CAN_PILE )) return( false );
	if ( ! IsSameType( pItem )) return( false );

	// total should not add up to > 64K !!!
	if ( pItem->m_amount > 0xFFFF - m_amount )
		return( false );

	return( true );
}

int CItem :: GetPrice( void ) const
{
	// Price for a single item. (Do not factor in amount)
	int iPrice = 0;
	if ( m_pDef->m_buyvalue ) iPrice = m_pDef->m_buyvalue;
	if ( m_pDef->m_sellvalue ) iPrice = m_pDef->m_sellvalue;
	if ( ! iPrice ) iPrice = 2;

	// ??? factor in magic and quality values.

	return( iPrice );
}

const char * CItem :: GetName() const
{	// allow some items to go unnamed (just use type name).

	const char * pName = CObjBase :: GetName();

	if ( pName[0] != '\0' )	
		return( pName );

	// Just use it's type name instead.
	if ( m_pDef->m_sName.IsEmpty())
		return( "unnamed" );

	return( m_pDef->m_sName );
}

void CItem :: SetAmount( int amount )
{
	// propagate the weight change. 
	int oldamount = m_amount;
	m_amount = amount;
	if ( ! IsTopLevel()) 
	{
		GetParent()->GetThisContainer()->WeightChange( ( amount - oldamount ) * m_pDef->m_weight );
	}
}

void CItem :: Write( CScript & s ) const
{
	s.Printf( "[WORLDITEM %x]\n", GetID() );
	CObjBase :: Write( s );

	if ( m_amount != 1 )
		s.Printf( "AMOUNT=%i\n", m_amount);
	if ( m_type )
		s.Printf( "TYPE=%i\n", m_type);
	if ( m_more1 )
		s.Printf( "MORE=%lx\n", m_more1 );
	if ( m_more2 )
		s.Printf( "MORE2=%lx\n", m_more2 );
	if ( m_magic_p.m_x )
		s.Printf( "MOREP=%s\n", m_magic_p.Write());
	if ( m_Attr )
		s.Printf( "ATTR=%x\n", m_Attr );

	CObjBase * pCont = GetContainer();
	if ( pCont != NULL )
	{
		if ( ! pCont->IsItem())
		{
			if ( GetEquipLayer() >= LAYER_HORSE )
				s.Printf( "LAYER=%i\n", GetEquipLayer() );
		}
		else
		{
			s.Printf( "P=%s\n", m_p.Write() );
		}
		s.Printf( "CONT=%x\n", GetContainerUID());
	}
	else
	{
		s.Printf( "P=%s\n", m_p.Write() );
	}

	s.Printf( "\n");
}

bool CItem :: LoadVal( const char * pKey, char * pVal ) // Load an item Script
{
static const char * table[] = 
{
	"ID",
	"AMOUNT",
	"TYPE",
	"MORE",
	"MORE2",
	"MOREP",
	"ATTR",
	"LAYER",
	"P",
	"CONT",

	"DUPEITEM",
	"DAM",
	"ARMOR",
	"BUYVALUE",
	"SELLVALUE",
	"HITPOINTS",
	"SKILL",
	"SPEED",
};

	switch ( FindTable( pKey, table, COUNTOF( table )))
	{
	case 0:	// "ID"
		SetID( (ITEMID_TYPE) ahextoi(pVal));
		return true;
	case 1:	// "AMOUNT"
		SetAmount( atoi(pVal));
		return true;
	case 2: // "TYPE"
		m_type = (ITEM_TYPE) atoi(pVal);
		return true;
	case 3:	// "MORE"
		m_more1 = ahextoi(pVal);
		return true;
	case 4:	// "MORE2"
		m_more2 = ahextoi(pVal);
		return true;
	case 5: // "MOREP"
		m_magic_p.Read(pVal);
		return true;
	case 6: // "ATTR"
		m_Attr = ahextoi(pVal);
		return true;
	case 7: // "LAYER"
		m_equip_layer = (LAYER_TYPE) atoi(pVal);	// GetEquipLayer()
		return true;
	case 8: // "P"
		m_p.Read( pVal );
		return true;
	case 9:	// "CONT" needs special processing.
		LoadSetContainer( ahextoi(pVal), (LAYER_TYPE) m_equip_layer );
		return true;
#ifdef COMMENT
	case 10:	// "DUPEITEM"
		if ( GetID() == m_pDef->m_id )
		{
			delete m_pDef;	// junk this one.
			m_pDef = FindItemBase( ahextoi(pVal) );
			ASSERT( m_pDef != NULL );
			m_pDef->LoadSameID( GetID() );
		}
		return( true );
#endif
	case 11:  // "DAM",
	case 12:	// "ARMOR"
		m_pDef->m_attack = atoi( pVal );
		return true;
	case 13: // "BUYVALUE",
		m_pDef->m_buyvalue = GetRangeVal( pVal );
		return true;
	case 14: // "SELLVALUE",
		m_pDef->m_sellvalue = GetRangeVal( pVal );
		return true;
	case 15: // "HITPOINTS",
		m_hitpoints_max = m_hitpoints = GetRangeVal( pVal );
		return true;
	case 16: // "SKILL",
		switch ( toupper( pVal[0] ))
		{
		case 'S': m_pDef->m_type = m_type = ITEM_WEAPON_SWORD; return true;
		case 'F': m_pDef->m_type = m_type = ITEM_WEAPON_FENCE; return true;
		case 'M': m_pDef->m_type = m_type = ITEM_WEAPON_MACE; return true;
		case 'A': m_pDef->m_type = m_type = ITEM_WEAPON_BOW; return true;
		}
		break;
	case 17: // "SPEED",
		break;
	}

	return( CObjBase :: LoadVal( pKey, pVal ));
}

void CItem :: Load( CScript & s ) // Load an item from script
{
	while ( 1 )
	{
		if ( ! s.ReadParse()) break;

		if ( ! strcmp(s.m_Line, "SERIAL"))
		{
			SetPrivateUID( ahextoi(s.m_pArg), true );
		}
		else
		{
			LoadVal( s.m_Line, s.m_pArg );
		}
	}
	if ( m_UID == UID_UNUSED )
	{
		SetPrivateUID( 0, true );
	}
	if ( GetContainer() == NULL )
	{
		MoveTo( m_p );
	}
}

void CItem :: Dupe( CItem * pItem )
{
	// Dupe this item.

	SetPrivateID( pItem->GetID());
	m_sName = pItem->m_sName;
	m_p = pItem->m_p;
	m_color  = pItem->m_color;

	m_type = pItem->m_type;		
	m_amount = pItem->m_amount;	
	m_timeout = pItem->m_timeout;

	m_more1 = pItem->m_more1;		
	m_more2 = pItem->m_more2;
	m_magic_p = pItem->m_magic_p;	
	m_Attr  = pItem->m_Attr;

	// Copy the contents of this item.
	if ( IsContainer())
	{
		CContainer * pContainer = GetThisContainer();
		for ( CItem * pContent = pContainer->GetContentHead(); pContent != NULL; pContent = pContent->GetNext())
		{
			pContainer->ContentAdd( ItemCreateDupe( pContent ));
		}
	}

	// Put in the same container as well.
	CObjBase * pCont = pItem->GetContainer();
	if ( pCont != NULL )
	{
		// Put in same container
		pCont->GetThisContainer()->ContentAdd(this);
	}
	else
	{
		PutOnGround( pItem->m_p );
	}
}

void CItem :: OnTick()
{
	// Timer expired. Time to do something.

	switch ( m_type )
	{
	case ITEM_SPAWN:
		// Spawn a creature if we are under count.
		SetTimeout( 60 );	// check agian later
		if ( ! IsTopLevel()) return;
		if ( m_amount >= m_spawn_max )
			return;
		{
			CREID_TYPE id = CREID_ORC;
			switch ( m_spawn_type )	// The class of spawn.
			{
			case 0x1001: // undead;
			case 0x1002: // elementals
			case 0x1003: // dungeon vermin.
			case 0x1004: // woodsy.
			case 0x1005: // water
				break;
			default:
				// ???
				id = (CREID_TYPE) m_spawn_type;
				break;
			}
			m_amount ++;
			CChar * pChar = CharCreateScript( id );
			pChar->m_owner = GetUID();
			pChar->MoveTo( m_p );
			pChar->Update();
		}
		return;
	case ITEM_TRAP_ACTIVE:
		Use_Trap();	// reset the trap.
		return;
	case ITEM_DOOR:
		// Doors should close.
		Use_Door( false );
		return;
	}

	switch ( GetID())
	{
	case ITEMID_FX_DUMMY:	// probly a much better way to do this.
		// reset to starting image.
		SetTimeout( -1 );
		SetID( (ITEMID_TYPE)( GetID()-1 ));
		Update();
		return;
	}

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "Time to delete item '%s'\n", GetName()));
	}

	// Spells should expire and clean themselves up.
	// Ground items rot... etc

	delete this;	// yeah yeah i know you should never do this.
}

void CItem :: Update( CClient * pClientExclude )
{
	// Send this new item to all that can see it.

	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude ) continue;
		if ( ! pClient->CanSee( this )) continue;

		CObjBase * pObjCont = GetContainer();
		if ( pObjCont == NULL )					
			pClient->addItemPut( this );	// on ground
		else if ( ! pObjCont->IsItem())
		{
			if ( GetEquipLayer() < LAYER_DRAGGING ) 
				pClient->addItemEquip( this );	// equipped
			else
				pClient->addObjectRemove( this );
		}
		else 
		{
			// used to temporary build corpses.
			if ( ! ( ( dynamic_cast <CItem*> (pObjCont))->m_Attr & ATTR_INVIS ))
				pClient->addItemCont( this );	// in container.
			else
				pClient->addObjectRemove( this );
		}
	}
}

bool CItem :: IsSpellInBook( SPELL_TYPE spell ) const
{
	if ( spell < 32 )
		return( m_spells1 & (1<<spell));
	else
		return( m_spells2 & (1<<(spell-32)));
}

int CItem :: AddSpellbookScroll( CItem * pItem )
{
	// Add  this scroll to the spellbook.

	for ( int i=0; i<SPELL_BOOK_QTY; i++ )
	{
		if ( pItem->GetID() != Spells[i].m_ScrollID )
			continue;
		if ( IsSpellInBook((SPELL_TYPE)i))
		{
			return( 1 );	// already here.
		}
		// Update the spellbook ???
		// addItemCont()
		if ( i < 32 )
			m_spells1 |= ( 1<<(i));
		else
			m_spells2 |= ( 1<<(i-32));
		delete pItem;		// gone.
		return( 0 );
	}
	return( 2 );
}

void CItem :: CreateMulti( CChar * pChar )
{
	// Create house or boat extra stuff.
	// NOTE: This can only be done after the house is given location.

	ASSERT( GetID() >= ITEMID_BOAT_N && GetID() < ITEMID_SCRIPT );

	int id = GetID();
	if ( id < ITEMID_HOUSE )
	{
		// Dont do boats yet.
		return;
	}

	CItem * pDoor;	// Main Door or chest.
	CPoint pd = m_p;
	CPoint ps = m_p;	// place the sign.

	switch ( id )
	{
	case ITEMID_HOUSE:	// 0x4064 = 4K+100
	case 0x4065:
	case ITEMID_STONEHOUSE:	// 0x4066 = 4K+100
	case 0x4067:
	case 0x4068:
	case 0x4069:
	case 0x406a:
	case 0x406b:
	case 0x406c:
	case 0x406d:
	case 0x406e:
	case 0x406f:
		// 5 x 5 spaces inside
		// Main Door
		pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd.m_y += 3;
		pd.m_z += 7;

		// Place the sign.
		ps.m_x += 3;
		ps.m_y += 5;
		ps.m_z += 15;
		break;

	case ITEMID_TENT_BLUE:
	case ITEMID_TENT_BLUE+1:
	case ITEMID_TENT_GREEN:		// 0x72
	case ITEMID_TENT_GREEN+1:	// 0x73

	    pDoor = ItemCreateScript( ITEMID_CHEST4 );
	    pd.m_y -= 1;
	    pd.m_z += 7;

		// No sign
		ps.m_x = 0;
		break;

	case ITEMID_3ROOM:
	case ITEMID_3ROOM+1:

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd.m_x -= 1;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1_2 );
		pd = m_p;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd = m_p;
	    pd.m_x -= 3;
	    pd.m_y -= 1;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd = m_p;
	    pd.m_x += 3;
	    pd.m_y -= 1;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 3;
	    ps.m_y += 7;
	    ps.m_z += 5;
		break;

	case ITEMID_2STORY_STUKO:
	case ITEMID_2STORY_STUKO+1:
	case ITEMID_2STORY_SAND:
	case ITEMID_2STORY_SAND+1:

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd.m_x -= 3;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1_2 );
		pd = m_p;
		pd.m_x -= 2;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd = m_p;
		pd.m_x -= 2;
	    pd.m_z += 27;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd = m_p;
		pd.m_x -= 3;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 1;
	    ps.m_y += 7;
	    ps.m_z += 5;
		break;

	case ITEMID_TOWER:
	case ITEMID_TOWER+1:

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_3 );
		pd.m_x += 1;
	    pd.m_y += 4;
	    pd.m_z += 47;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_3 );
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y += 4;
	    pd.m_z += 27;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S );
		pd = m_p;
		pd.m_x += 3;
	    pd.m_y -= 2;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// other Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_2 );
		pDoor->m_type = ITEM_DOOR_LOCKED;
		pDoor->m_lockID = GetUID(); 
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S );
		pd = m_p;
	    pd.m_y += 6;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 4;
	    ps.m_y += 7;
	    ps.m_z += 5;
		break;

	case ITEMID_KEEP:
	case ITEMID_KEEP+1:

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_2 );
		pDoor->m_type = ITEM_DOOR_LOCKED;
		pDoor->m_lockID = GetUID(); 
		pd.m_x += 1;
	    pd.m_y += 10;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S );
		pd = m_p;
	    pd.m_y += 10;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 5;
	    ps.m_y += 12;
	    ps.m_z += 15;
		break;

	case ITEMID_CASTLE:
	case ITEMID_CASTLE+1:

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S );
	    pd.m_y += 5;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_2 );
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y += 5;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// 2nd front door
	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S_2 );
		pDoor->m_type = ITEM_DOOR_LOCKED;
		pDoor->m_lockID = GetUID(); 
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y += 15;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_METAL_S );
		pd = m_p;
	    pd.m_y += 15;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 5;
	    ps.m_y += 17;
	    ps.m_z += 15;
		break;

	case ITEMID_LARGESHOP:

	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd.m_x += 4;
	    pd.m_y -= 1;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		pDoor = ItemCreateScript( (ITEMID_TYPE)( 0x6ad ));
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y += 4;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

	    pDoor = ItemCreateScript( (ITEMID_TYPE)( 0x6ad ));
		pd = m_p;
		pd.m_x += 1;
	    pd.m_y -= 4;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Other Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1_2 );
		pDoor->m_type = ITEM_DOOR_LOCKED;
		pDoor->m_lockID = GetUID(); 
		pd = m_p;
		pd.m_x -= 3;
	    pd.m_y += 6;
	    pd.m_z += 7;
		pDoor->PutOnGround( pd );

		// Front door.
	    pDoor = ItemCreateScript( ITEMID_DOOR_WOODEN_1 );
		pd = m_p;
		pd.m_x -= 4;
	    pd.m_y += 6;
	    pd.m_z += 7;

		// Place the sign.
		ps.m_x += 1;
	    ps.m_y += 8;
	    ps.m_z += 15;
		break;
	}

	if ( ps.m_x )
	{
	    CItem * pSign = ItemCreateScript( ITEMID_SIGN_BRASS );
		pSign->m_lockID = GetUID();
		pSign->PutOnGround( ps );
	}

	// Set the key id for the main door.
	pDoor->m_type = ITEM_DOOR_LOCKED;
	pDoor->m_lockID = GetUID(); 
	pDoor->PutOnGround( pd );

	// Create the key to the door.
	CItem * pKey = ItemCreateScript( ITEMID_KEY_COPPER );
	pKey->m_type = ITEM_KEY;
	pKey->m_lockID = GetUID(); 

	// Put in your pack 
	CContainerItem * pPack = pChar->GetPack();
	pPack->ContentAdd( pKey );

	// Put dupe key in the bank.
	pKey = ItemCreateDupe( pKey );
	CContainerItem * pBank = dynamic_cast <CContainerItem *>( pChar->LayerFind( LAYER_BANKBOX ));
	if ( pBank != NULL )
	{
		pBank->ContentAdd( pKey );
		pChar->SysMessage( "The duplicate key is in your bank account" );
	}
	else
	{
		pPack->ContentAdd( pKey );
	}
}

void CItem :: Flip( const char * pCmd )
{
	// Equivelant rotational objects.
	// These objects are all the same except they are rotated.

	// Doors are easy.
	if ( m_type == ITEM_DOOR )
	{
		int i = IsDoorID( GetID());

#ifdef COMMENT
	if ( ! strcmpi( pTemp, "ALIGN" ))
			((CItem*) pObj)->Flip(DOOR_NORTHSOUTH);
		else if ( ! strcmpi( pTemp, "HAND" ))
			((CItem*) pObj)->Flip(DOOR_RIGHTLEFT);
		else if ( ! strcmpi( pTemp, "SWING" ))
			((CItem*) pObj)->Flip(DOOR_INOUT);
		else
			((CItem*) pObj)->Flip(DOOR_OPENCLOSE);
#endif

		i = ( i >= 16 ) ? -15 : 1;
		SetID( (ITEMID_TYPE)( GetID() + i ));
		Update();
		return;
	}

	// Try to rotate the object.
#ifdef COMMENT
	int id = 0;
	if ( m_flip_id != NULL )
	{
		int idprev = m_pDef->m_id;
		for ( int i=0; 1; i++ )
		{
			id = m_pDef->m_flip_id[i];
			if ( id == 0 ) break;
			if ( idprev == GetID())	break;
			idprev = id;
		}
	}
	if ( id == 0 ) id = m_pDef->m_id;
	if ( id != GetID() )
	{
		SetID( (ITEMID_TYPE) id );
		Update();
	}
#else
static const WORD item_flip[] = // (ITEMID_TYPE) 
{
	// ??? this table is way incomplete.
	0x02, 0x05, 0x00,	// ank
	0x03, 0x04, 0x00,	// ank
	0x3ae, 0x3af, 0x3b0, 0,	// pier

	ITEMID_SCISSORS1, ITEMID_SCISSORS2, 0,
	ITEMID_CRATE1, ITEMID_CRATE2, 0,
	ITEMID_CRATE3, ITEMID_CRATE4, 0,

	0x6f5, 0x6f6, 0,	// Portculis.
	0x6fd, 0x6fe, 0x6ff, 0x700, 0, // tile roof.

	0x839, 0x83B, 0x841, 0x843, 0, // Wooden Gate 1
	0x84C, 0x84E, 0x854, 0x856, 0, // Iron Gate
	0x866, 0x868, 0x86E, 0x870, 0, // Wooden Gate 2

	0x0a2c, 0x0a34, 0, // light drawers.
	0x0a30, 0x0a38, 0, // dark drawers.

	0x0a97, 0x0a99, 0, // book case.
	0x0a98, 0x0a9a, 0, // book case.
	0x0a9b, 0x0a9c, 0, // book case.
	0x0a9d, 0x0a9e, 0, // book case.

	0x0b2c, 0x0b2d, 0, // wood bench.
	0x0b2e, 0x0b2f, 0x0b30, 0xb31, 0, // chair.
	0x0b32, 0x0b33, 0, // throne.
	0x0b4e, 0x0b4f, 0x0b50, 0x0e51, 0, 
	0x0b52, 0x0b53, 0x0b54, 0x0e55, 0, 
	0x0b56, 0x0b57, 0x0b58, 0x0e59, 0, 
	0x0b5a, 0x0b5b, 0x0b5c, 0x0e5d, 0, // chair

	0x0faf, 0x0fb0, 0,	// anvil
	0x0fb4, 0x0fb5, 0,  // sledge hammer.

	0x100c, 0x100d, 0x0f36, 0, // hay
	0x1022, 0x1023, 0, // fletching.
	0x1024, 0x1025, 0, // arrow shafts.

	0x102a, 0x102b, 0, // hammer
	0x103a, 0x1046, 0, // open flour
	0x1039, 0x1045, 0, // flour sack

	0x1049, 0x104a, 0, // benches.
	0x104b, 0x104c, 0, // clocks
	0x104d, 0x104e, 0, // clock frame
	0x1057, 0x1058, 0, // sextant.

	0x1078, 0x1079, 0, // hides

	0x1165, 0x1166, 0, // graves.

	0x1183, 0x1184, 0, // graves

	0x11bc, 0x11bd, 0x11be, 0x11bf, 0,	// dung ramp
	0x11d6, 0x11d9, 0, // bamboo table.
	0x11d7, 0x11da, 0, // bamboo table.
	0x11d8, 0x11db, 0, // bamboo table.

	0x1218, 0x1219, 0x121a, 0x121b, 0, // stone chair.
	0x1243, 0x1244, 0, // wall carving

	0x13ad, 0x13ae, 0, // pillows.
	0x13af, 0x13b0, 0, // war axe

	0x13e7, 0x13e8, 0, // ring armor.
	0x13ec, 0x13ed, 0, // ring tunic.
	0x1410, 0x1417, 0, // plate arms.
	0x1411, 0x141a, 0, // plate legs.
	0x1415, 0x1416, 0, // plate chest.

	0x144f, 0x1454, 0, // bone chest.
	0x14eb, 0x14ec, 0, // map

	0x14f3, 0x14f4, 0, // model ship.
	0x14f7, 0x14f9, 0, // archor.
	0x14ef, 0x14f0, 0, // deed
	0x14f1, 0x14f2,	0, // ship deed.
	0x14f8, 0x14fa, 0, // rope.

	0x1721, 0x1722, 0, // bananas.
	0x1b74, 0x1b75, 0, // metal kite shield.
	0x1b76, 0x1b77, 0, // heater
	0x1b78, 0x1b79, 0, // wood kite.
	0,
};
	for ( int i=0; item_flip[i]; i++ )
	{
		int iStart = i;
		for ( ; item_flip[i]; i++ )
		{
			if ( GetID() == item_flip[i] )
			{
				SetID( (ITEMID_TYPE)( ( item_flip[i+1] ) ? item_flip[i+1] : item_flip[iStart] ));
				Update();
				return;
			}
		}
	}
#endif

}

WORD CItem :: Use_Music( bool fWell )
{
	WORD wSoundWell = 0;
	WORD wSoundPoor = 0;

	switch ( IsMusicalID( GetID())) 
	{
	case 1:
		wSoundWell = 0x0038;
		wSoundPoor = 0x0039;
		break;
	case 2:
	case 3:
		wSoundWell = 0x0052;
		wSoundPoor = 0x0053;
		break;
	case 4:
	case 5:
		wSoundWell = 0x0045;
		wSoundPoor = 0x0046;
		break;
	case 6:
	case 7:
		wSoundWell = 0x004C;
		wSoundPoor = 0x004D;
		break;
	}
	return( fWell ? wSoundWell : wSoundPoor );
}

bool CItem :: Use_Door( bool fJustOpen )
{
	// RETURN: true = open
	ASSERT( m_type == ITEM_DOOR );
	WORD id = GetID();
	int type = IsDoorID( GetID());
	ASSERT( type );
	if ( ! type ) return( false );
	--type;

	bool fClosing = ( type & 1 );
	if ( fJustOpen && fClosing ) return( true );	// links just open

	switch ( type )
	{
	case 0:
		m_p.m_x--;
		m_p.m_y++;
		id++;
		break;
	case 1:
		m_p.m_x++;
		m_p.m_y--;
		id--;
		break;
	case 2:
		m_p.m_x++;
		m_p.m_y++;
		id++;
		break;
	case 3:
		m_p.m_x--;
		m_p.m_y--;
		id--;
		break;
	case 4:
		m_p.m_x--;
		id++;
		break;
	case 5:
		m_p.m_x++;
		id--;
		break;
	case 6:
		m_p.m_x++;
		m_p.m_y--;
		id++;
		break;
	case 7:
		m_p.m_x--;
		m_p.m_y++;
		id--;
		break;
	case 8:
		m_p.m_x++;
		m_p.m_y++;
		id++;
		break;
	case 9:
		m_p.m_x--;
		m_p.m_y--;
		id--;
		break;
	case 10:
		m_p.m_x++;
		m_p.m_y--;
		id++;
		break;
	case 11:
		m_p.m_x--;
		m_p.m_y++;
		id--;
		break;
	case 12:
		id++;
		break;
	case 13:
		id--;
		break;
	case 14:
		m_p.m_y--;
		id++;
		break;
	case 15:
		m_p.m_y++;
		id--;
		break;
	}

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "Door: %d,%d\n", GetID(), type ));
	}

	SetID( (ITEMID_TYPE) id );
	Update();
	Sound( fClosing ? 0x00f1 : 0x00ea );
	// Auto close the door in n seconds.
	SetTimeout( fClosing ? -1 : 60 );
	return( ! fClosing );
}

const char * CItem :: Use_Clock()
{
	// Get the time in text.

	WORD minutes = World.GetDayTimeMinutes();
	WORD minute = minutes % 60;
	WORD hour = ( minutes / 60 ) % 12;

	const char * pMinDif;
	if ((minute>=0)&&(minute<=14)) pMinDif = "";
	else if ((minute>=15)&&(minute<=30)) pMinDif = " a quarter past";
	else if ((minute>=30)&&(minute<=45)) pMinDif = " half past";
	else 
	{
		pMinDif = " a quarter till";
		hour = ( hour + 1 ) % 12;
	}

static const char * ClockHour[] = 
{
	"noon",
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine",
	"ten",
	"eleven",
	"midnight"
};

	const char * pTale;
	if (hour==12) pTale = "";
	else if ( minutes > 12*60 )
	{
		if ((hour>=1)&&(hour<6)) pTale = " o'clock in the afternoon";
		else if ((hour>=6)&&(hour<9)) pTale = " o'clock in the evening.";
		else pTale = " o'clock at night";
	}
	else
	{
		pTale = " o'clock in the morning";
	}
 
	char * pTime = GetTempStr();
	sprintf( pTime, "It is%s %s%s.", pMinDif, ClockHour[hour], pTale );
	return( pTime );
}

SKILL_TYPE CItem :: GetWeaponSkill() const
{
	// assuming this is a weapon. What skill does it apply to.

	switch ( m_type )
	{
	case ITEM_WEAPON_MACE:			// Can be used for smithing ?
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
		return( SKILL_MACEFIGHTING );
	case ITEM_WEAPON_SWORD:
		return( SKILL_SWORDSMANSHIP );
	case ITEM_WEAPON_FENCE:
		return( SKILL_FENCING );
	case ITEM_WEAPON_BOW:
		return( SKILL_ARCHERY );
	}
	return( SKILL_WRESTLING );
}

int CItem :: Use_Trap()
{
	// ??? use effect instead !!!
	if ( m_type == ITEM_TRAP_ACTIVE ) 
	{
		SetID( (ITEMID_TYPE)( GetID()-1 ));
		SetTimeout( -1 );
		m_type = ITEM_TRAP;
	}
	else
	{
		SetID( (ITEMID_TYPE)( GetID() + 1 ));
		SetTimeout( 3 );
		m_type = ITEM_TRAP_ACTIVE;
	}
	Update();

	return( 2 );	// base damage done.
}

void CItem :: Kill_Spawn_Children()
{
	// kill all creatures spawned from this !
	ASSERT( m_type == ITEM_SPAWN );
	for ( int i=0; i<QUAD_QTY; i++ )
	{
		CChar * pCharNext;
		CChar * pChar = static_cast <CChar*>( World.m_Quads[i].m_Chars.GetHead());
		for ( ; pChar!=NULL; pChar = pCharNext )
		{
			pCharNext = pChar->GetNext();
			if ( pChar->m_owner == GetUID())
			{
				delete pChar;
			}
		}
	}
}

