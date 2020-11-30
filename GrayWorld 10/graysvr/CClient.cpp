//
// CClient.cpp
//
#include "graysvr.h"	// predef header.

// Define my potions.

const CPotionDef Potions[] = 
{
	// Potion Name Color Ingredients Cost Sell Return Effect Skill 

	// "Agility",  Blue 1 Blood Moss 12 9 0.75 +10 DEX (30 sec.) 15.1 
	{ "Agility",			ITEMID_POTION_BLUE,		"Bm", 151 },
	// "Greater Agility", Blue 3 Blood Moss 24 12 0.5 +20 DEX (30 sec.) 35.1 
	{ "Greater Agility",	ITEMID_POTION_BLUE,		"BmBm", 351 },
	// "Cure, Lesser Orange 1 Garlic 9 9 1 Cures Lesser Poison -- 
	{ "Lesser Cure",		ITEMID_POTION_ORANGE,	"Ga", 0 },
	// "Cure Orange 3 Garlic 15 9 0.6 Cures Lesser and Normal Poison 25.1 
	{ "Cure",				ITEMID_POTION_ORANGE,	"GaGa",	251 },
#ifdef COMMENT
	"Cure, Greater Orange 6 Garlic 24 9 0.38 Cures All Poisons 65.1 
	"Explosion, Lesser Purple 3 Sulphurous Ash 15 12 0.8 1-5 damage 5.1 
	"Explosion Purple 5 Sulphurous Ash 21 12 0.57 6-10 damage 35.1 
	"Explosion, Greater Purple 10 Sulphurous Ash 36 12 0.33 11-20 damage 65.1 
	"Heal, Lesser Yellow 1 Ginseng 9 9 1 + 3-10 HP -- 
	"Heal Yellow 3 Ginseng 15 9 0.6 + 6-20 HP 15.1 
	"Heal, Greater Yellow 7 Ginseng 27 9 0.33 + 9-30 HP 55.1 
	"Nightsight Black 1 Spider's Silk 9 9 1 until daylight -- 
	"Poison, Lesser Green 1 Nightshade 9 9 1 * -- 
	"Poison Green 2 Nightshade 12 9 0.75 * 15.1 
	"Poison, Greater Green 4 Nightshade 18 9 0.5 * 55.1 
	"Poison, Deadly Green 8 Nightshade 30 9 0.3 * 90.1 
	"Refresh Red 1 Black Pearl 12 7 0.58 +25% STAM -- 
	"Refresh, Total Red 5 Black Pearl 36 7 0.19 Full STAM 25.1 
	"Strength White 2 Mandrake Root 12 9 0.75 +10 STR (30 sec.) 25.1 
	"Strength, Greater White 5 Mandrake Root 21 9 0.42 +20 STR (30 sec.) 45.1 
#endif
	
};

/////////////////////////////////////////////////////////////////
// -CClient stuff.

CClient :: CClient( SOCKET client ) : CSocket( client )
{
	m_pChar = NULL;

	m_DeCryptInit = false;
	m_EnCrypt = false;
	m_Priv = 0;

	m_bin_len = 0;
	m_bin_pkt = 0;
	m_bout_len = 0;

	m_WalkCount = -1;
	m_fPaused = false;
	m_Targ_Index = 0;

	// ??? Make sure the first server matches the GetSockName here ?

	struct sockaddr_in Name;
	GetPeerName( &Name );

	Log.Event( "%x:Client connected [Total:%i] from %s.\n", 
		GetSocket(), Serv.m_Clients.GetCount()+1, inet_ntoa( Name.sin_addr ));
}

CClient :: ~CClient()
{
	Log.Event( "%x:Client disconnected [Total:%i]\n", GetSocket(), Serv.m_Clients.GetCount() -1 );

	xFlush();

	if ( Serv.m_pAdminClient == this )
	{
		// unlink the admin client.
		Serv.m_pAdminClient = NULL;
	}

	if ( m_pChar != NULL )
	{
		// remove me from other clients screens.
		m_pChar->Remove( this );
		m_pChar->m_pClient = NULL;
		// Should we allow them to become an NPC for a little while ?
		World.m_CharsIdle.InsertAfter( m_pChar );
	}
}

// The "golden" key for (0.0.0.0)
// Client version 1.25.33

static const WORD EnCrypt_Base[256+1] =
{
	0x0002, 0x01f5, 0x0226, 0x0347, 0x0757, 0x0286, 0x03b6, 0x0327,
	0x0e08, 0x0628, 0x0567, 0x0798, 0x19d9, 0x0978, 0x02a6, 0x0577,
	0x0718, 0x05b8, 0x1cc9, 0x0a78, 0x0257, 0x04f7, 0x0668, 0x07d8,
	0x1919, 0x1ce9, 0x03f7, 0x0909, 0x0598, 0x07b8, 0x0918, 0x0c68,
	0x02d6, 0x1869, 0x06f8, 0x0939, 0x1cca, 0x05a8, 0x1aea, 0x1c0a,
	0x1489, 0x14a9, 0x0829, 0x19fa, 0x1719, 0x1209, 0x0e79, 0x1f3a,
	0x14b9, 0x1009, 0x1909, 0x0136, 0x1619, 0x1259, 0x1339, 0x1959,
	0x1739, 0x1ca9, 0x0869, 0x1e99, 0x0db9, 0x1ec9, 0x08b9, 0x0859,
	0x00a5, 0x0968, 0x09c8, 0x1c39, 0x19c9, 0x08f9, 0x18f9, 0x0919,
	0x0879, 0x0c69, 0x1779, 0x0899, 0x0d69, 0x08c9, 0x1ee9, 0x1eb9,
	0x0849, 0x1649, 0x1759, 0x1cd9, 0x05e8, 0x0889, 0x12b9, 0x1729,
	0x10a9, 0x08d9, 0x13a9, 0x11c9, 0x1e1a, 0x1e0a, 0x1879, 0x1dca,
	0x1dfa, 0x0747, 0x19f9, 0x08d8, 0x0e48, 0x0797, 0x0ea9, 0x0e19,
	0x0408, 0x0417, 0x10b9, 0x0b09, 0x06a8, 0x0c18, 0x0717, 0x0787,
	0x0b18, 0x14c9, 0x0437, 0x0768, 0x0667, 0x04d7, 0x08a9, 0x02f6,
	0x0c98, 0x0ce9, 0x1499, 0x1609, 0x1baa, 0x19ea, 0x39fa, 0x0e59,
	0x1949, 0x1849, 0x1269, 0x0307, 0x06c8, 0x1219, 0x1e89, 0x1c1a,
	0x11da, 0x163a, 0x385a, 0x3dba, 0x17da, 0x106a, 0x397a, 0x24ea,
	0x02e7, 0x0988, 0x33ca, 0x32ea, 0x1e9a, 0x0bf9, 0x3dfa, 0x1dda,
	0x32da, 0x2eda, 0x30ba, 0x107a, 0x2e8a, 0x3dea, 0x125a, 0x1e8a,
	0x0e99, 0x1cda, 0x1b5a, 0x1659, 0x232a, 0x2e1a, 0x3aeb, 0x3c6b,
	0x3e2b, 0x205a, 0x29aa, 0x248a, 0x2cda, 0x23ba, 0x3c5b, 0x251a,
	0x2e9a, 0x252a, 0x1ea9, 0x3a0b, 0x391b, 0x23ca, 0x392b, 0x3d5b,
	0x233a, 0x2cca, 0x390b, 0x1bba, 0x3a1b, 0x3c4b, 0x211a, 0x203a,
	0x12a9, 0x231a, 0x3e0b, 0x29ba, 0x3d7b, 0x202a, 0x3adb, 0x213a,
	0x253a, 0x32ca, 0x23da, 0x23fa, 0x32fa, 0x11ca, 0x384a, 0x31ca,
	0x17ca, 0x30aa, 0x2e0a, 0x276a, 0x250a, 0x3e3b, 0x396a, 0x18fa,
	0x204a, 0x206a, 0x230a, 0x265a, 0x212a, 0x23ea, 0x3acb, 0x393b,
	0x3e1b, 0x1dea, 0x3d6b, 0x31da, 0x3e5b, 0x3e4b, 0x207a, 0x3c7b,
	0x277a, 0x3d4b, 0x0c08, 0x162a, 0x3daa, 0x124a, 0x1b4a, 0x264a,
	0x33da, 0x1d1a, 0x1afa, 0x39ea, 0x24fa, 0x373b, 0x249a, 0x372b,
	0x1679, 0x210a, 0x23aa, 0x1b8a, 0x3afb, 0x18ea, 0x2eca, 0x0627,
	0x00d4 // terminator
} ;

void CClient :: xFlush() // Sends buffered data at once
{
	if ( m_bout_len <= 0 ) return;
	if ( ! m_EnCrypt )
	{
		Send( (char*) m_bout.m_Raw, m_bout_len );
		m_bout_len=0;
		return;
	}

	// This acts as a compression alg.
	static BYTE xoutbuffer[MAX_BUFFER];
	int len=0;
	int bitidx=0;	// Offset in output byte

	for ( int i=0; i <= m_bout_len; i++ )
	{
		WORD value = ( i == m_bout_len ) ? EnCrypt_Base[256] : 
			EnCrypt_Base[ m_bout.m_Raw[i] ];
		int nrBits = value & 0xF;
		value >>= 4;
		while ( nrBits-- )
		{
			xoutbuffer[len] <<= 1;
			xoutbuffer[len] |= (value >> nrBits) & 0x1;
			if ( ++bitidx == 8)
			{
				bitidx = 0;
				len++;
			}
		}
	}
	if(bitidx)
	{
		xoutbuffer[len] <<= 8-bitidx;
		len++;
	}

	// DEBUG_MSG(( "%x:Send %d bytes as %d\n", GetSocket(), m_bout_len, len ));

	ASSERT( len <= sizeof(xoutbuffer));

	Send( (char *) xoutbuffer, len );
	m_bout_len=0;
}

void CClient :: xSend( const void *pData, int length ) 
{
	// buffer a packet to client.
	if ( m_bout_len + length > MAX_BUFFER )
	{
		DEBUG_ERR(( "ERROR %x: Client out overflow!\n", GetSocket() ));
		ASSERT(0);
		return;
	}

	memcpy( & ( m_bout.m_Raw[m_bout_len] ), pData, length );
	m_bout_len += length;
}

bool CClient :: xCheckSize( int len )
{
	// Is there enough data from client to process this packet ?
	if ( len > sizeof( CEvent )) return( false );	// junk
	m_bin_pkt = len;
	return( m_bin_len >= len );
}

void CClient :: xProcess( bool fGood )
{
	// Done with the current packet.
	if ( ! fGood )	// toss all.
	{
		DEBUG_ERR(( "ERROR %x:Bad Msg %xh Eat %d bytes\n", GetSocket(), m_bin.Default.m_Cmd, m_bin_len ));
		m_bin_len = 0;
		if ( ! m_EnCrypt )	// tell them about it.
		{
			addLoginErr( LOGIN_ERR_OTHER );
		}
	}
	else
	{
		ASSERT( m_bin_len >= m_bin_pkt );
		memmove( m_bin.m_Raw, &(m_bin.m_Raw[m_bin_pkt]), m_bin_len-m_bin_pkt);
		m_bin_len -= m_bin_pkt;
	}
	m_bin_pkt = 0;
}

void CClient :: xInit_DeCrypt( const BYTE * pDeCryptID )
{
	DEBUG_MSG(( "%x:xInit_DeCrypt %d.%d.%d.%d\n", GetSocket(),
		pDeCryptID[0], pDeCryptID[1], pDeCryptID[2], pDeCryptID[3] ));

	m_DeCryptInit = true;

	DWORD seed = ((DWORD)pDeCryptID[2] << 8) + (DWORD)pDeCryptID[3];
	m_DeCryptMaskLo = (((~seed) << 16) | seed ) ^ 0x1357aaaa;
	seed = ((DWORD)pDeCryptID[0] << 8) + (DWORD)pDeCryptID[1] ;
	m_DeCryptMaskHi = (((~seed) << 16) | seed ) ^ 0xabcd4321;
}

#if 1

void CClient :: xInit_DeCrypt_FindKey( const BYTE * pCryptData, int len )
{
	// Given an encrypted buffer. pCryptData
	// Compare it back agianst what we expected to see.
	// Use this to derive the MasterKey

	// use m_Targ_Index for index into masks.
	if ( m_Targ_Index > 64 ) return;

	// ExpectedData should need to be only 64 bytes ?
	// This is the login packet + 2 bytes from the spy packet.
	// Login the first time with admin + password same = predictable data.
	// Assume 2 bytes from spy packet will also be the same ?
	static BYTE ExpectedData[64] =	
	{
		0x80, 'A',  'D',  'M',  'I',  'N',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'P', 
		'A',  'S',  'S',  'W',  'O',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
		0xa4, 0x03	// 2 bytes from the spy packet.
	};

	// What key would it take to derive the expected data.
	// for 127.0.0.1 = m_DeCryptMaskLo=0xeca9aaab, m_DeCryptMaskHi=0x2b323c21
	for ( int i=0; i<len; i++ )
	{
		// 
		//m_bin.m_Raw[i] = m_bin.m_Raw[i] ^ m_DeCryptMaskLo;
		BYTE MaskLoShouldBe = m_bin.m_Raw[i] ^ ExpectedData[ m_Targ_Index ];

		//DWORD MaskLo = m_DeCryptMaskLo;
		//DWORD MaskHi = m_DeCryptMaskHi;
		//m_DeCryptMaskHi = ((MaskHi >> 1) | (MaskLo << 31)) ^ MASTERKEY_HI;
		//m_DeCryptMaskLo = ((MaskLo >> 1) | (MaskHi << 31)) ^ MASTERKEY_LO;
	}

}

#endif

bool CClient :: xRecvData() // Receive message from client
{
	// High level Rx from Client.
	// RETURN: false = dump the client.

	int iPrev = m_bin_len;
	int count = Receive( (char*) &(m_bin.m_Raw[iPrev]), MAX_BUFFER - iPrev );
	if ( count <= 0 )	// I should always get data here.
	{
		return( false );
	}
	m_bin_len += count;

	if ( ! m_DeCryptInit )
	{
		// Must process the whole thing as one packet right now.
		ASSERT( iPrev == 0 );
		if ( Serv.m_pAdminClient == this )	// remote admin console.
		{
			for ( int i=0; i < m_bin_len; i++ ) 
				Serv.OnConsoleCmd( m_bin.m_Raw[i] );
			m_bin_len = 0;
			return( true );
		}

		if ( m_bin_len < 4 )	// just a ping ?
		{
			if ( m_bin.Default.m_Cmd == 0x21 ) // UOMon ???
			{
				const char * pTemp = World.GetInformation();
				xSend( pTemp, strlen( pTemp )+1 );
			}
			// m_bin_len = 0;
			return( false );
		}

		// Log in admin console 
		if ( m_bin.m_Raw[0] == 0xFF && 
			m_bin.m_Raw[1] == 0xFF && 
			m_bin.m_Raw[2] == 0xFF && 
			m_bin.m_Raw[3] == 0xFF && 
			m_bin_len > 5 )
		{
			m_bin_len = 0;	// eat the buffer.
			if ( Serv.m_pAdminClient != NULL ) 
				return( false );
			if ( ! LogIn( "Administrator", (char *) &m_bin.m_Raw[4] ))
				return( false );
			Serv.m_pAdminClient = this;
			Serv.Printf( "Welcome Remote Admin Console\n" );
			return( true );
		}

		// Assume it's a normal client log in.
		xCheckSize( 4 );
		xInit_DeCrypt( m_bin.m_Raw ); // Init decryption table
		xProcess( true );
	}

	// Decrypt the data.
	// ??? If we miss a packet we are screwed !

	//Log.Event( "Before\n" );
	//Log.Dump( m_bin.m_Raw, m_bin_len );

	for ( int i=iPrev; i<m_bin_len; i++ )
	{
		m_bin.m_Raw[i] ^= m_DeCryptMaskLo;
		DWORD MaskLo = m_DeCryptMaskLo;
		DWORD MaskHi = m_DeCryptMaskHi;
		m_DeCryptMaskHi = ((MaskHi >> 1) | (MaskLo << 31)) ^ MASTERKEY_HI;
		m_DeCryptMaskLo = ((MaskLo >> 1) | (MaskHi << 31)) ^ MASTERKEY_LO;
	}

	//Log.Event( "After\n" );
	//Log.Dump( m_bin.m_Raw, m_bin_len );

	return( true );
}

/////////////////////////////////////////////////////////////////////////
// Push world display data to this client only.

void CClient :: addPause( bool fPause )
{
	// 0 = restart. 1=pause
	if ( this == NULL ) return;
	if ( m_fPaused == fPause ) return;

	if ( ! fPause && m_UpdateStats )
	{
		m_pChar->UpdateStats( (STAT_TYPE) ( m_UpdateStats-1 ) );
	}

	CCommand cmd;
	cmd.Pause.m_Cmd = 0x33;
	cmd.Pause.m_Arg = fPause;
	xSendPkt( &cmd, sizeof( cmd.Pause ));
	m_fPaused = fPause;
}

void CClient :: addLoginErr( LOGIN_ERR_TYPE code )
{
	// code 
	// 0 = no account
	// 1 = account used.
	// 2 = blocked.
	// 3 = no password
	// LOGIN_ERR_OTHER

	DEBUG_ERR(( "ERROR %x:Bad Login %d\n", GetSocket(), code ));

	CCommand cmd;
	cmd.LogBad.m_Cmd = 0x82;
	cmd.LogBad.m_code = code;
	xSendPkt( &cmd, sizeof( cmd.LogBad ));
}

void CClient ::	addOptions()
{
	char Options[21] = "\x69\x00\x05\x01\x00"
						"\x69\x00\x05\x02\x00"
						"\x69\x00\x05\x03\x00"
						"\x55"
						"\x5B\x0C\x13\x03";	// Time
	xSend( Options, 20 );

#ifdef COMMENT
	// set options ? // ??? not needed ?
	cmd.Options.m_Cmd = 0x69;
	cmd.Options.m_len = 5;
	cmd.Options.m_data[0] = 1;
	cmd.Options.m_data[1] = 0;
	xSendPkt( &cmd, 5 );
	xSend( "\x69\x00\x05\x02\x00", 5 );
	xSend( "\x69\x00\x05\x03\x00", 5 );

	xSend( "\x55", 1 );				// ???

	// Send time. (real or game time ??? why ?)
	cmd.Time.m_Cmd = 0x5b;
	cmd.Time.m_hours = ( World.m_Clock_Time / ( 60*60 )) % 24;
	cmd.Time.m_min   = ( World.m_Clock_Time / ( 60 )) % 60;
	cmd.Time.m_sec   = ( World.m_Clock_Time		   ) % 60;
	xSendPkt( &cmd, sizeof( cmd.Time ));
#endif
}

void CClient :: addObjectRemove( CObjBase * pObj )
{
	// Tell the client to remove the item or char
	addPause();
	CCommand cmd;
	cmd.Remove.m_Cmd = 0x1D;
	cmd.Remove.m_UID = pObj->GetUID();
	xSendPkt( &cmd, sizeof( cmd.Remove ));
}

void CClient :: addObjectLight( CObjBase * pObj )		// Object light level.
{
	CCommand cmd;
	cmd.LightPoint.m_Cmd = 0x4e;
	cmd.LightPoint.m_UID = pObj->GetUID();
	cmd.LightPoint.m_level = 0;	// ??? light level.
	xSendPkt( &cmd, sizeof( cmd.Remove ));
}

void CClient :: addItemPut( CItem * pItem ) // Send items (on ground)
{
	ASSERT( pItem->IsTopLevel());	// This doesn't apply.

	CCommand cmd;
	cmd.Put.m_Cmd = 0x1A;
	cmd.Put.m_len = sizeof(cmd.Put);
	cmd.Put.m_UID = pItem->GetUID() | UID_SPEC;	// Enable Piles	
	cmd.Put.m_id = pItem->GetID();
	cmd.Put.m_amount = pItem->m_amount;
	cmd.Put.m_x = pItem->m_p.m_x;
	cmd.Put.m_y = pItem->m_p.m_y | 0xC000; // Enable m_color and m_movable
	cmd.Put.m_z = pItem->m_p.m_z;
	cmd.Put.m_color = pItem->m_color & 0xFFF;	// restrict colors
	cmd.Put.m_movable = 0;

	// Do i have the right to move this object ?
	if ( m_pChar->CanMove( pItem ))
	{
		cmd.Put.m_movable |= 0x20;
	}
	if ( IsPriv(PRIV_DEBUG))
	{
		// just use safe numbers
		cmd.Put.m_id = ITEMID_WorldGem;
		cmd.Put.m_z = m_pChar->m_p.m_z;
		cmd.Put.m_amount = 1;	
	}
	else if (( pItem->m_Attr & ATTR_INVIS ) && ! IsPriv(PRIV_GM))
	{
		cmd.Put.m_movable |= 0x80;	// invis object 
	}

	//If this item has direction facing use it
#ifdef COMMENT
	if( (pItem->m_type == ITEMID_LIGHT          )||
		(pItem->m_type == ITEMID_LIGHT_WEARABLE )||
		(pItem->m_type == ITEMID_GATE          ))
#endif
	if ( pItem->GetID() == ITEMID_CORPSE )
	{
		cmd.Put.m_len = sizeof(cmd.Put)+1;
		cmd.Put.m_x = pItem->m_p.m_x | 0x8000;
		cmd.m_Raw[19]=cmd.m_Raw[18];
		cmd.m_Raw[18]=cmd.m_Raw[17];
		cmd.m_Raw[17]=cmd.m_Raw[16];
		cmd.m_Raw[16]=cmd.m_Raw[15];
		cmd.m_Raw[15]=pItem->m_corpse_dir;
	}

	xSendPkt( &cmd, cmd.Put.m_len );

	if ( IsPriv(PRIV_DEBUG)) return;
	if ( pItem->GetID() == ITEMID_CORPSE )	// cloths on corpse
	{
		// send all the items on the corpse.
		addContents( dynamic_cast <CContainerItem*> (pItem), false );
		// equip the proper items on the corpse. 
		addContents( dynamic_cast <CContainerItem*> (pItem), true );
	}
}

void CClient :: addItemEquip( CItem * pItem )
{
	// Equip a single item on a CChar.
	ASSERT( pItem->IsEquipped());
	ASSERT( ! pItem->GetContainer()->IsItem());
	CCommand cmd;
	cmd.Equip.m_Cmd = 0x2E;
	cmd.Equip.m_UID = pItem->GetUID();
	cmd.Equip.m_id = pItem->GetID();
	cmd.Equip.m_zero7 = 0;
	cmd.Equip.m_layer = pItem->GetEquipLayer();
	cmd.Equip.m_UIDchar = pItem->GetContainerUID();
	cmd.Equip.m_color = pItem->m_color & 0xFFF;
	xSendPkt( &cmd, sizeof( cmd.Equip ));
}

void CClient :: addItemCont( CItem * pItem )
{
	ASSERT( ! pItem->IsTopLevel());
	ASSERT( pItem->GetContainer()->IsItem());

	// Add a single item in a container.
	CCommand cmd;
	cmd.ItemAddCont.m_Cmd = 0x25;
	cmd.ItemAddCont.m_UID = pItem->GetUID();
	cmd.ItemAddCont.m_id = pItem->GetID();
	cmd.ItemAddCont.m_zero7 = 0;
	cmd.ItemAddCont.m_amount = pItem->m_amount;
	cmd.ItemAddCont.m_x = pItem->m_p.m_x;
	cmd.ItemAddCont.m_y = pItem->m_p.m_y;
	cmd.ItemAddCont.m_contUID = pItem->GetContainerUID();
	cmd.ItemAddCont.m_color = pItem->m_color & 0xFFF;	// restrict colors in containers.
	xSendPkt( &cmd, sizeof( cmd.ItemAddCont ));
}

int CClient :: addContents( CContainerItem * pContainer, bool fEquip, bool fShop ) // Send Backpack (with items)
{
	// NOTE: We needed to send the header for this FIRST !!!
	// 1 = equip a corpse
	// 0 = contents.

	ASSERT( pContainer->IsValid());
	CCommand cmd;

	// send all the items in the container.
	int count = 0;
	for ( CItem* pItem=pContainer->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( fEquip )	// list equipped items on a corpse
		{
			ASSERT( pItem->GetEquipLayer() < LAYER_HORSE );
			switch ( pItem->GetEquipLayer() )	// don't put these on a corpse.
			{
			case LAYER_NONE:
			case LAYER_PACK:	// these display strange.
				continue;
			}
			cmd.CorpEquip.items[count].m_layer	= pItem->GetEquipLayer();
			cmd.CorpEquip.items[count].m_UID	= pItem->GetUID();
		}
		else	// Content items
		{
			cmd.Content.items[count].m_UID		= pItem->GetUID();
			cmd.Content.items[count].m_id		= pItem->GetID();
			cmd.Content.items[count].m_zero6	= 0;
			cmd.Content.items[count].m_amount	= pItem->m_amount;
			if ( fShop )
			{
				if ( ! pItem->m_amount ) continue;
				if ( pItem->GetID() == ITEMID_GOLD ) continue;
				cmd.Content.items[count].m_x	= count;
				cmd.Content.items[count].m_y	= count;
			}
			else
			{
				cmd.Content.items[count].m_x	= pItem->m_p.m_x;
				cmd.Content.items[count].m_y	= pItem->m_p.m_y;
			}
			cmd.Content.items[count].m_UIDCont	= pItem->GetContainerUID();	
			cmd.Content.items[count].m_color	= pItem->m_color & 0xFFF;	// restrict colors
		}
		count ++;
	}

	if ( ! count )
	{
		return 0;
	}
	int len;
	if ( ! fEquip )
	{
		len = sizeof( cmd.Content ) - sizeof(cmd.Content.items) + ( count * sizeof(cmd.Content.items[0]));
		cmd.Content.m_Cmd = 0x3c;
		cmd.Content.m_len = len;
		cmd.Content.m_count = count;
	}
	else
	{
		len = sizeof( cmd.CorpEquip ) - sizeof(cmd.CorpEquip.items) + ( count * sizeof(cmd.CorpEquip.items[0]));
		cmd.CorpEquip.m_Cmd = 0x89;
		cmd.CorpEquip.m_len = len;
		cmd.CorpEquip.m_UID = pContainer->GetUID();
	}

	xSendPkt( &cmd, len );
	return( count );
}

void CClient :: addOpenGump( CObjBase * pContainer, WORD gump )
{
	// NOTE: if pContainer has not already been sent to the client 
	//  this will crash client.
	CCommand cmd;
	cmd.Open.m_Cmd = 0x24;
	cmd.Open.m_UID = pContainer->GetUID();
	cmd.Open.m_gump = gump;

	// we automatically get open sound for this,.
	xSendPkt( &cmd, sizeof( cmd.Open ));
}

void CClient :: addContainerSetup( CContainerItem * pContainer ) // Send Backpack (with items)
{
	ASSERT( pContainer->IsValid());
	ASSERT( pContainer->IsItem());

	// open the conatiner with the proper GUMP.
	WORD gump = pContainer->IsContainerID( pContainer->GetID());
	if ( gump == 0 ) return;

	addOpenGump( pContainer, gump );
	addContents( pContainer, false );
}

void CClient :: addDyeOption( CObjBase * pObj )
{
	// Put up the color chart for the client.
	// This results in a Event_Item_Dye message.
	ITEMID_TYPE id;
	if ( pObj->IsItem())
	{
		id = (dynamic_cast <CItem*> (pObj))->GetID();
	}
	else
	{
		// Get the item equiv for the creature.
		id = (dynamic_cast <CChar*> (pObj))->m_pCre->m_trackID;
	}

	CCommand cmd;
	cmd.DyeVat.m_Cmd = 0x95;
	cmd.DyeVat.m_UID = pObj->GetUID();
	cmd.DyeVat.m_zero5 = 0;
	cmd.DyeVat.m_id = id;
	xSendPkt( &cmd, sizeof( cmd.DyeVat ));
}

void CClient :: addLight()
{
	// Global light level.
	CCommand cmd;
	cmd.Light.m_Cmd = 0x4F;
	cmd.Light.m_level = m_pChar->GetLightLevel();
	xSendPkt( &cmd, sizeof( cmd.Light ));
}

void CClient :: addMusic( WORD id )
{
	// Music is ussually appropriate for the region.
	CCommand cmd;
	cmd.PlayMusic.m_Cmd = 0x6d;
	cmd.PlayMusic.m_musicid = id;
	xSendPkt( &cmd, sizeof( cmd.PlayMusic ));
}

void CClient :: addSound( WORD id, const CObjBase * pBase )
{
	CCommand cmd;
	cmd.Sound.m_Cmd = 0x54;
	cmd.Sound.m_unk1 = 1;
	cmd.Sound.m_id = id;
	cmd.Sound.m_zero4 = 0;
	cmd.Sound.m_x = pBase->m_p.m_x;
	cmd.Sound.m_y = pBase->m_p.m_y;
	cmd.Sound.m_z = pBase->m_p.m_z;
	xSendPkt( &cmd, sizeof(cmd.Sound));
}

void CClient :: addItemDragCancel( BYTE type )
{
	// Sound seems to be automatic ???
	// addSound( m_pChar, 0x051 );
	CCommand cmd;
	cmd.DragCancel.m_Cmd = 0x27;
	cmd.DragCancel.m_type = type;
	xSendPkt( &cmd, sizeof( cmd.DragCancel ));
}

void CClient :: addBark( const char * pText, CObjBase * pSrc, WORD color, TALKMODE_TYPE mode, FONT_TYPE font )
{
	CCommand cmd;
	cmd.Speak.m_Cmd = 0x1C;

	if ( mode == TALKMODE_BROADCAST )
	{
		mode = TALKMODE_SYSTEM;
		pSrc = NULL;
	}

	int len = sizeof(cmd.Speak) + strlen(pText);
	cmd.Speak.m_len = len;
	cmd.Speak.m_mode = mode;		// mode = range.
	cmd.Speak.m_color = color;
	cmd.Speak.m_font = font;		// font. 3 = system message just to you !

	if ( pSrc == NULL )
	{
		cmd.Speak.m_UID = 0x01010101;
		strcpy( cmd.Speak.m_name, "System" );
	}
	else
	{
		cmd.Speak.m_UID = pSrc->GetUID();
		strncpy( cmd.Speak.m_name, pSrc->GetName(), sizeof(cmd.Speak.m_name));
		cmd.Speak.m_name[ sizeof(cmd.Speak.m_name)-1 ] = '\0';
	}
	if ( pSrc == NULL || pSrc->IsItem())
	{
		cmd.Speak.m_id = 0x0101;
	}
	else	// char id only.
	{
		cmd.Speak.m_id = ((CChar*)pSrc)->GetID();
	}
	strcpy( cmd.Speak.m_text, pText );
	xSendPkt( &cmd, len );
}

void CClient :: addSysMessage( const char *pMsg) // System message (In lower left corner)
{
	addBark( pMsg, NULL, 0x03B2, TALKMODE_SYSTEM, FONT_NORMAL );
}

void CClient :: addItemMessage( const char *pMsg, CObjBase * pSrc, WORD color ) // The message when an item is clicked
{
	addBark( pMsg, pSrc, color, TALKMODE_ITEM, FONT_NORMAL );
}

void CClient :: addWeather() // Send new weather to player
{
	CCommand cmd;
	cmd.Weather.m_Cmd = 0x65;
	cmd.Weather.m_type = m_pChar->m_p.GetQuadrant()->m_weather;

	// ??? not if we are in doors or in cave ?

	switch ( cmd.Weather.m_type )
	{
	case 1:	// rain
		cmd.Weather.m_ext = 0x4600;
		break;
	case 2:	// snow
		cmd.Weather.m_ext = 0x46EC;
		break;
	default:	// dry
		cmd.Weather.m_ext = 0;
		break;
	}
	xSendPkt( &cmd, sizeof(cmd.Weather));
}

void CClient :: addEffect( BYTE motion, ITEMID_TYPE id, const CObjBase * pDst, const CObjBase * pSrc, BYTE speed, BYTE loop, BYTE explode )
{
	CCommand cmd;
	cmd.Effect.m_Cmd = 0x70;
	cmd.Effect.m_motion = motion;
	cmd.Effect.m_id = id;
	cmd.Effect.m_UID = pDst->GetUID();
	if ( pSrc != NULL )
	{
		cmd.Effect.m_srcx = pSrc->m_p.m_x;
		cmd.Effect.m_srcy = pSrc->m_p.m_y;
		cmd.Effect.m_srcz = pSrc->m_p.m_z;
		cmd.Effect.m_dstx = pSrc->m_p.m_x;
		cmd.Effect.m_dsty = pSrc->m_p.m_y;
		cmd.Effect.m_dstz = pSrc->m_p.m_z;
	}
	cmd.Effect.m_speed = speed;		// 22= 0=very fast, 7=slow.
	cmd.Effect.m_loop = loop;		// 23= 0 is really long.  1 is the shortest., 6 = longer
	cmd.Effect.m_zero24 = 0x300;		// 24=0 unknown
	cmd.Effect.m_OneDir = true;		// 26=1=point in single dir else turn.
	cmd.Effect.m_explode = explode;	// 27=effects that explode on impact.

	switch ( motion )
	{
	case 0:	// a targetted bolt
		cmd.Effect.m_targUID = pDst->GetUID();
		cmd.Effect.m_UID = pSrc->GetUID();	// source
		cmd.Effect.m_dstx = pDst->m_p.m_x;
		cmd.Effect.m_dsty = pDst->m_p.m_y;
		cmd.Effect.m_dstz = pDst->m_p.m_z;
		cmd.Effect.m_OneDir = false;
		cmd.Effect.m_loop = 0;	// Does not apply.
		break;

	case 1:	// lightning bolt.
		break;

	case 2:	// Stay at current xyz ??? not sure about this.
		break;

	case 3:	// effect at single Object.
		break;
	}

	xSendPkt( &cmd, sizeof( cmd.Effect ));
}

void CClient :: addMoveCancel( void )
{
	// Resync back to a previous move.
	CCommand cmd;
	cmd.MoveCancel.m_Cmd = 0x21;
	cmd.MoveCancel.m_count = m_WalkCount;	// sequence # 
	cmd.MoveCancel.m_x = m_pChar->m_p.m_x;
	cmd.MoveCancel.m_y = m_pChar->m_p.m_y;
	cmd.MoveCancel.m_dir = m_pChar->m_dir;
	cmd.MoveCancel.m_z = m_pChar->m_p.m_z;
	xSendPkt( &cmd, sizeof( cmd.MoveCancel ));
}

void CClient :: addCharMove( CChar * pChar )
{
	// This char has just moved on screen. 
	// or changed in a subtle way like "hidden"
	// 

	CCommand cmd;
	cmd.CharMove.m_Cmd = 0x77;
	cmd.CharMove.m_UID = pChar->GetUID();
	cmd.CharMove.m_id = pChar->GetID();
	cmd.CharMove.m_x  = pChar->m_p.m_x;
	cmd.CharMove.m_y  = pChar->m_p.m_y;
	cmd.CharMove.m_z = pChar->m_p.m_z;
	cmd.CharMove.m_dir = pChar->m_dir;	// ??? running ?
	cmd.CharMove.m_color = pChar->m_color;
	cmd.CharMove.m_mode = pChar->GetModeFlag();
	cmd.CharMove.m_noto = pChar->GetNotoFlag();

	if ( IsPriv(PRIV_DEBUG))
	{
		cmd.CharMove.m_id = CREID_MAN;
		cmd.CharMove.m_z = m_pChar->m_p.m_z;
	}

	xSendPkt( &cmd, sizeof(cmd.CharMove));
}

void CClient :: addChar( CChar * pChar )
{
	// Full update about a char. 

	CCommand cmd;
	cmd.Char.m_Cmd = 0x78;
	cmd.Char.m_UID = pChar->GetUID();
	cmd.Char.m_id = pChar->GetID();
	cmd.Char.m_x = pChar->m_p.m_x;
	cmd.Char.m_y = pChar->m_p.m_y;
	cmd.Char.m_z = pChar->m_p.m_z;
	cmd.Char.m_dir = pChar->m_dir | 0x80;	// running
	cmd.Char.m_color = pChar->m_color;
	cmd.Char.m_mode = pChar->GetModeFlag();
	cmd.Char.m_noto = pChar->GetNotoFlag();

	if ( IsPriv(PRIV_DEBUG))
	{
		cmd.Char.m_id = CREID_MAN;
		cmd.Char.m_z = m_pChar->m_p.m_z;
	}

	// extend the current struct for all the equipped items.

	int i=0;
	for ( CItem* pItem=pChar->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->GetEquipLayer() >= LAYER_DRAGGING ) continue;
		cmd.Char.equip[i].m_UID = pItem->GetUID();
		cmd.Char.equip[i].m_id = pItem->GetID() | 0x8000;	// include color.
		cmd.Char.equip[i].m_layer = pItem->GetEquipLayer();
		cmd.Char.equip[i].m_color = pItem->m_color & 0xFFF;	// ??? optional color.
		i++;
	}

	// Not well understood.  It's a serial number.  I set this to my serial number,
	// and all of my messages went to my paperdoll gump instead of my character's 
	// head, when I was a character with serial number 0 0 0 1.
	cmd.Char.equip[i].m_UID = 0;	// terminator.
	int len = ( sizeof( cmd.Char ) - sizeof( cmd.Char.equip )) + ( i * sizeof( cmd.Char.equip[0] )) + sizeof( DWORD );
	cmd.Char.m_len = len;
	xSendPkt( &cmd, len );
}

void CClient :: addCharName( CChar * pChar ) // Singleclick text for a character
{
	// Karma color codes ?

	WORD color;
	if ( pChar->m_Stat[STAT_Karma] < -500 ) color = 0x026;		// Red
	else if ( pChar->m_Stat[STAT_Karma] < -100 ) color = 0x03b2;	// Grey
	else color = 0x63;	// Blue

	char * pTemp = GetTempStr();

	strcpy( pTemp, pChar->GetFameTitle());
	strcat( pTemp, pChar->GetName());

	if ( pChar->m_NPC_Brain ) strcat( pTemp, " [NPC]" );
	if ( pChar->m_StatFlag & STATF_INVUL ) strcat( pTemp, " [invulnerable]" );
	if ( pChar->m_StatFlag & STATF_Freeze ) strcat( pTemp, " [frozen]" );

	if ( IsPriv(PRIV_SHOWUID))
	{
		sprintf( pTemp+strlen(pTemp), " [%lx]", pChar->GetUID() );
	}

	addItemMessage( pTemp, pChar, color );
}

void CClient :: addPlayerStart()
{
	m_pChar->m_pClient = this;
	m_pChar->m_NPC_Brain = NPCBRAIN_NONE;	// Not allowed.

	addPause();

static BYTE Pkt_Start1[20] =
	"\x00\x00\x00\x7F\x00\x00\x00\x00\x00\x07\x80\x09\x60\x00\x00\x00\x00\x00\x00";

	CCommand cmd;
	cmd.Start.m_Cmd = 0x1B;
	cmd.Start.m_UID = m_pChar->GetUID();
	cmd.Start.m_zero5 = 0;
	cmd.Start.m_id = m_pChar->GetID();
	cmd.Start.m_x = m_pChar->m_p.m_x;
	cmd.Start.m_y = m_pChar->m_p.m_y;
	cmd.Start.m_z = m_pChar->m_p.m_z;
	cmd.Start.m_dir = m_pChar->m_dir;
	memcpy( cmd.Start.m_unk18, Pkt_Start1, sizeof( cmd.Start.m_unk18 ));
	cmd.Start.m_mode = m_pChar->GetModeFlag();
	xSendPkt( &cmd, sizeof( cmd.Start ));

	m_pChar->Update();
	addPlayerWarMode();
	addOptions();
	addWeather();
}

void CClient :: addPlayerWarMode()
{
	CCommand cmd;
	cmd.War.m_Cmd = 0x72;
	cmd.War.m_warmode = ( m_pChar->m_StatFlag & STATF_War ) ? 1 : 0;
	cmd.War.m_unk2[0] = 0;
	cmd.War.m_unk2[1] = 0x32;
	cmd.War.m_unk2[2] = 0;
	xSendPkt( &cmd, sizeof( cmd.War ));
}

void CClient :: addWebLaunch( const char *pPage )
{
	 // Direct client to a web page
	addSysMessage( "Launching your web browser. Please wait...");

	CCommand cmd;
	cmd.Web.m_Cmd = 0xA5;
	int len = sizeof(cmd.Web) + strlen(pPage);
	cmd.Web.m_len = len;
	strcpy( cmd.Web.m_page, pPage );
	xSendPkt( &cmd, len );
}

void CClient :: addBookOpen( CItem * pItem )
{
	CCommand cmd;
	cmd.BookOpen.m_Cmd = 0x93;
	cmd.BookOpen.m_UID = pItem->GetUID();
	cmd.BookOpen.m_writable = 0;

	CScript s;
	if ( ! s.Open( GRAY_FILE "book" GRAY_SCRIPT ))
		return;

	CString sSec;
	sSec.Format( "BOOK %li", pItem->m_bookID );
	if ( ! s.FindSec(sSec))
		return;
	do
	{
		if ( ! s.ReadParse()) break;
	}
	while (strcmp(s.m_Line, "PAGES"));
	cmd.BookOpen.m_pages = atoi(s.m_pArg);
	do
	{
		if ( ! s.ReadParse()) break;
	}
	while (strcmp(s.m_Line, "TITLE"));
	strcpy( cmd.BookOpen.m_title, s.m_pArg );
	do
	{
		if ( ! s.ReadParse()) break;
	}
	while (strcmp(s.m_Line, "AUTHOR"));
	strcpy( cmd.BookOpen.m_author, s.m_pArg );

	xSendPkt( &cmd, sizeof( cmd.BookOpen ));
}

void CClient :: SetTargMode( TARGMODE_TYPE targmode, const char *pPrompt )
{
	// ??? Get rid of menu stuff if previous targ mode. 
	if ( m_Targ_Mode == targmode ) return;

	m_Targ_Mode = targmode;
	if ( targmode == TARGMODE_NONE )
	{
		// Just clear the target mode.
		// Cancel any related skill.
		m_pChar->Skill_Cleanup();
		addSysMessage( "Cancelled" );
	}
	else
	{
		m_pChar->SetTimeout( 4*60 );	// suspend the act timer til targetted.
		addSysMessage( pPrompt );
	}
}

void CClient :: addTarget( TARGMODE_TYPE targmode, const char *pPrompt, bool fObject ) // Send targetting cursor to client
{
	SetTargMode( targmode, pPrompt );

	// ??? will this be selective for us ? objects only or chars only ? not on the ground (statics) ?
	CCommand cmd;
	cmd.Target.m_Cmd = 0x6c;
	cmd.Target.m_Req = 1;		// Not sure what this is.
	cmd.Target.m_One = 1;
	cmd.Target.m_code = targmode ;	// 5=my id code for action.
	cmd.Target.m_type = (fObject)?0:1;	// 1=allow xyz, 0=objects only.

	xSendPkt( &cmd, sizeof( cmd.Target ));
}

void CClient :: addTargetMulti( CItem * pDeed )
{
	ITEMID_TYPE iddef;
	const char * pPrompt;
	switch ( pDeed->GetID())
	{
	case ITEMID_DEED1:
	case ITEMID_DEED2:
	default:
		iddef = ITEMID_HOUSE;
		pPrompt = "Structure";
		break;
	case ITEMID_SHIP_PLANS1:
	case ITEMID_SHIP_PLANS2:
		iddef = ITEMID_BOAT_N;
		pPrompt = "Ship";
		break;
	}
	m_Targ_Val = pDeed->m_deed_type;
	if ( m_Targ_Val < ITEMID_BOAT_N || m_Targ_Val >= ITEMID_SCRIPT )
	{
		m_Targ_Val = iddef;
	}

	CString sTmp;
	sTmp.Format( "Where would you like to place the %s?", pPrompt );
	SetTargMode( TARGMODE_USE_ITEM, sTmp );

	CCommand cmd;
	cmd.TargetMulti.m_Cmd = 0x99;
	cmd.TargetMulti.m_Req = 1;		// Not sure what this is.
	cmd.TargetMulti.m_One = 1;
	cmd.TargetMulti.m_code = TARGMODE_USE_ITEM ;	// 5=my id code for action.
	memset( cmd.TargetMulti.m_zero6, 0, sizeof(cmd.TargetMulti.m_zero6));
	cmd.TargetMulti.m_id = m_Targ_Val - ITEMID_BOAT_N;
	memset( cmd.TargetMulti.m_zero20, 0, sizeof(cmd.TargetMulti.m_zero20));

	xSendPkt( &cmd, sizeof( cmd.TargetMulti ));
}

void CClient :: addSkillWindow( SKILL_TYPE skill ) // Opens the skills list
{
	CCommand cmd;
	cmd.Skill.m_Cmd = 0x3A;
	cmd.Skill.m_zero3 = 0;

	int iQty;
	int iSkill;

	if ( skill >= SKILL_QTY )
	{	// all skills
		iSkill = 0;
		iQty = SKILL_QTY;
	}
	else 
	{	// Just one skill update.
		iSkill = skill;
		iQty = 1;
	}

	int len = sizeof(cmd.Skill) + ((iQty-1) * sizeof(cmd.Skill.skills[0])) + sizeof(NWORD);
	cmd.Skill.m_size = len;
	int i = 0;
	for ( i=0; iQty--; i++ )
	{
		cmd.Skill.skills[i].m_index = iSkill+1;
		cmd.Skill.skills[i].m_val = m_pChar->m_Skill[iSkill];
		iSkill ++;
	}

	cmd.Skill.skills[i].m_index = 0;	// terminator.

	xSendPkt( &cmd, len );
}

void CClient :: addPlayerSee( CPoint & pold )
{
	// adjust to my new location.
	// What do I now see here ?

	// ??? is the light level diff here ? ex. not in cave.
	// ??? is the weather diff here ?

	// What new people do i see ?
	CWorldSearch AreaChars( m_pChar->m_p, UO_MAP_VIEW_SIZE );
	while ( 1 )
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL ) break;
		if ( m_pChar == pChar ) continue;	// I saw myself before.
		if ( ! CanSee( pChar )) continue;

		if ( abs(pold.m_x-pChar->m_p.m_x)>UO_MAP_VIEW_SIZE ||
			abs(pold.m_y-pChar->m_p.m_y)>UO_MAP_VIEW_SIZE )
		{
			addChar( pChar );
		}
	}

	// What new things do i see (on the ground) ?
	CWorldSearch AreaItems( m_pChar->m_p, UO_MAP_VIEW_BIG_SIZE );
	while ( 1 )
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL ) break;
		if ( ! CanSee( pItem )) continue;

		if ( abs(pold.m_x-pItem->m_p.m_x)>UO_MAP_VIEW_SIZE ||
			abs(pold.m_y-pItem->m_p.m_y)>UO_MAP_VIEW_SIZE )
		{
			addItemPut( pItem );
		}
	}
}

void CClient :: addPlayerView( CPoint & pold )
{
	// I moved = Change my point of view.

	CCommand cmd;
	cmd.View.m_Cmd = 0x20;
	cmd.View.m_UID = m_pChar->GetUID();
	cmd.View.m_id = m_pChar->GetID();
	cmd.View.m_zero7 = 0;
	cmd.View.m_color = m_pChar->m_color;
	cmd.View.m_mode = m_pChar->GetModeFlag();
	cmd.View.m_x = m_pChar->m_p.m_x;
	cmd.View.m_y = m_pChar->m_p.m_y;
	cmd.View.m_zero15 = 0;
	cmd.View.m_dir = m_pChar->m_dir | 0x80;	// ???
	cmd.View.m_z = m_pChar->m_p.m_z;
	xSendPkt( &cmd, sizeof( cmd.View ));

	if ( pold == m_pChar->m_p ) return;

	addLight();		// Current light level where I am.

	// What can i see here ?
	addPlayerSee( pold );
}

void CClient :: addReSync()
{
	// Reloads the client with all it needs.
	CPoint pold( 0xFFFF, 0xFFFF, 0 );
	addPlayerView( pold );
	addChar( m_pChar );
}

void CClient :: addStatWindow( CObjUID uid ) // Opens the status window
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) return;

	// Poisoned green ???

	CCommand cmd;
	cmd.Status.m_Cmd = 0x11;
	cmd.Status.m_len = sizeof( cmd.Status );
	cmd.Status.m_UID = pChar->GetUID();

	// WARNING: Char names must be <= 30 !!!
	// "kellen the magincia counsel me" 30 char names !!
	strncpy( cmd.Status.m_name, pChar->GetName(), sizeof( cmd.Status.m_name ));
	cmd.Status.m_name[ sizeof( cmd.Status.m_name )-1 ] = '\0';

	cmd.Status.m_health = pChar->m_health;
	cmd.Status.m_maxhealth = pChar->m_Stat[STAT_STR];

	// renamable ?
	if ( m_pChar != pChar &&	// this will have strange effects 
		( IsPriv(PRIV_GM) ||
		pChar->m_owner == m_pChar->GetUID() ))	// my pet.
	{
		cmd.Status.m_perm = 0xFF;
	}
	else
	{
		cmd.Status.m_perm = 0x00;
	}
	cmd.Status.m_ValidStats = 1;	// valid stats ? 
	cmd.Status.m_sex = ( CChar :: IsFemale( pChar->GetID() )) ? 1 : 0;
	cmd.Status.m_str = pChar->m_Stat[STAT_STR] ;
	cmd.Status.m_dex = pChar->m_Stat[STAT_DEX] ;
	cmd.Status.m_int = pChar->m_Stat[STAT_INT] ;
	cmd.Status.m_stam =	pChar->m_stam ;
	cmd.Status.m_maxstam = pChar->m_Stat[STAT_DEX] ;
	cmd.Status.m_mana =	pChar->m_mana ;
	cmd.Status.m_maxmana = pChar->m_Stat[STAT_INT] ; 
	cmd.Status.m_gold = pChar->ContentCount( ITEMID_GOLD );	/// ??? optimize this count is too often.
	cmd.Status.m_armor = pChar->m_defense ;
	cmd.Status.m_weight = pChar->GetTotalWeight() ;

	xSendPkt( &cmd, sizeof( cmd.Status ));
}

void CClient :: addSpellbookOpen( CItem * pBook )
{
	// NOTE: if the spellbook item is not present on the client it will crash.
	// count what spells I have.
	// ??? Mapping is wrong !!!

	int count=0;
	int i=0;
	for ( ;i<SPELL_BOOK_QTY-1;i++ )
		if ( pBook->IsSpellInBook( (SPELL_TYPE) i ))
	{
		count++;
	}

	addOpenGump( pBook, 0xFFFF );
	if (!count) return;

	CCommand cmd;
	int len = sizeof( cmd.Content ) - sizeof(cmd.Content.items) + ( count * sizeof(cmd.Content.items[0]));
	cmd.Content.m_Cmd = 0x3c;
	cmd.Content.m_len = len;
	cmd.Content.m_count = count;
	for (i=0;i<SPELL_BOOK_QTY-1;i++)
		if ( pBook->IsSpellInBook( (SPELL_TYPE) i ))
	{
		cmd.Content.items[i].m_UID = UID_ITEM + UID_FREE + i + 1; // just some unused id.
		cmd.Content.items[i].m_id = Spells[i+1].m_ScrollID; // 0x1F2E;	// scroll id.
		cmd.Content.items[i].m_zero6 = 0;
		cmd.Content.items[i].m_amount = i+1;
		cmd.Content.items[i].m_x = 0x48;	// may not mean anything ?
		cmd.Content.items[i].m_y = 0x7D;
		cmd.Content.items[i].m_UIDCont = pBook->GetUID();
		cmd.Content.items[i].m_color = 0;
	}

	xSendPkt( &cmd, len );
}

void CClient :: addScrollRead( const char * szSec, BYTE type, WORD tip )
{
	CScript s;
	if ( ! s.Open( GRAY_FILE "book" GRAY_SCRIPT ))
		return;
	if ( ! s.FindSec( szSec )) 
		return;

	// measure the page first.
	long pos = s.GetPos();
	int lines=0;
	int length=0;
	while (1)
	{
		if ( ! s.Read1()) break;
		lines++;
		length+=strlen(s.m_Line)+1;
	}

	CCommand cmd;
	length += sizeof( cmd.Scroll ) - sizeof( cmd.Scroll.m_text );

	// now send it.
	s.Seek( pos );

	cmd.Scroll.m_Cmd = 0xA6;
	cmd.Scroll.m_len = length;
	cmd.Scroll.m_type = type;
	cmd.Scroll.m_zero4 = 0;
	cmd.Scroll.m_tip = tip;
	cmd.Scroll.m_lentext = length - (sizeof( cmd.Scroll ) - sizeof( cmd.Scroll.m_text ));

	int i = 0;
	while ( lines -- )
	{
		if ( ! s.Read1()) break;
		strcpy( &cmd.Scroll.m_text[i], s.m_Line );
		i += strlen(s.m_Line);
		cmd.Scroll.m_text[i++] = ' ';
	}

	xSendPkt( &cmd, length );
}

void CClient :: addMap( CObjUID uid )
{
	// ??? which map ?
	CCommand cmd;

// BYTE map1[20] = "\x13\x9D\x04\x44\x05\x74\x06\xC8\x07\x84\x00\xC8\x00\xC8";
static BYTE Pkt_map1[20]=
	"\x13\x9D\x00\x00\x00\x00\x13\xFF\x0F\xA0\x01\x90\x01\x90";

	cmd.Map1.m_Cmd = 0x90;
	cmd.Map1.m_UID = uid;
	memcpy( cmd.Map1.m_unk, Pkt_map1, sizeof( cmd.Map1.m_unk ));
	xSendPkt( &cmd, sizeof( cmd.Map1 ));

static BYTE Pkt_map2[12]=
	"\x05\x00\x00\x00\x00\x00";

	cmd.Map2.m_Cmd = 0x56;
	cmd.Map2.m_UID = uid;
	memcpy( cmd.Map2.m_unk, Pkt_map2, sizeof( cmd.Map2.m_unk ));
	xSendPkt( &cmd, sizeof( cmd.Map2 ));
}

#if	1	// ??? clean this up.
#define PACKWORD(p,w)	(p)[0]=HIBYTE(w);(p)[1]=LOBYTE(w)
#define PACKDWORD(p,d)	(p)[0]=((d)>>24)&0xFF;(p)[1]=((d)>>16)&0xFF;(p)[2]=HIBYTE(d);(p)[3]=LOBYTE(d)
#endif

void CClient :: addItemMenu( CMenuItem * item, int count )
{
	int total = 9 + 1 + item[0].m_text.GetLength() + 1;
	int i = 1;
	for (;i<=count;i++)
	{
		total += 4+1+item[i].m_text.GetLength();
	}

static BYTE Pkt_menu1[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
	PACKWORD( &Pkt_menu1[1], total);
	PACKDWORD( &Pkt_menu1[3], m_pChar->GetUID() );
	PACKWORD( &Pkt_menu1[7], item[0].m_id );
	xSend( Pkt_menu1, 9);

	BYTE lentext = item[0].m_text.GetLength();
	xSend( &lentext, 1);
	xSend( item[0].m_text, lentext);

	lentext=count;
	xSend( &lentext, 1);
	for (i=1;i<=count;i++)
	{
static BYTE Pkt_menu2[6]="\x00\x00\x00\x00\x01";
		PACKWORD( &Pkt_menu2[0], item[i].m_id );
		Pkt_menu2[4] = item[i].m_text.GetLength();
		xSend( Pkt_menu2, 5 );
		xSend( item[i].m_text, item[i].m_text.GetLength() );
	}
}

void CClient :: addScriptMenu( int menuid, TARGMODE_TYPE offset ) // Menus for general purpose
{
	// offset = MENU_ITEMMENU
	CScript s;
	if ( ! s.Open( GRAY_FILE "menu" GRAY_SCRIPT ))
		return;

	CString sSec;
	sSec.Format( ( offset == TARGMODE_MENU_GM ) ? "GMMENU %i" : "ITEMMENU %i", menuid );
	if ( ! s.FindSec(sSec)) 
	{
		return;
	}

	CMenuItem item[32];

	s.Read1();
	if ( offset )
	{
		item[0].m_text.Format( "%i: %s", menuid, s.m_Line );
	}
	else
	{
		item[0].m_text.Format( s.m_Line );
	}
	item[0].m_id = menuid + offset;

	int count=0;
	while(1)
	{
		if ( offset )
		{
			if ( ! s.ReadParse()) break;

			count++;
			ASSERT( count < COUNTOF( item ));

			item[count].m_id = (ITEMID_TYPE) ahextoi(s.m_Line);
			item[count].m_text.Format( s.m_pArg );
		}
		else
		{
			if ( ! s.Read1()) break;
	
			count++;
			ASSERT( count < COUNTOF( item ));

			item[count].m_id = count-1;	// ??? diff !!!
			item[count].m_text.Format( s.m_Line );
		}

		if ( ! s.Read1()) break;
	}

	addItemMenu( item, count );
}

void CClient :: addVendorClose( CChar * pVendor )
{
	// Clear the vendor display.
	CCommand cmd;
	cmd.VendorClose.m_Cmd = 0x3B;
	cmd.VendorClose.m_len = sizeof( cmd.VendorClose );
	cmd.VendorClose.m_UIDVendor = pVendor->GetUID();
	cmd.VendorClose.m_flag = 0;	// 0x00 = no items following, 0x02 - items following 
	xSendPkt( &cmd, sizeof( cmd.VendorClose ));
}

int CClient :: addShopItems( CChar * pVendor, LAYER_TYPE layer )
{
	// Buy the contents of this container
	CContainerItem * pContainer = dynamic_cast <CContainerItem *>(pVendor->LayerFind( layer ));
	if ( pContainer == NULL )	// create the box if it is not here ?
	{
		pContainer = dynamic_cast <CContainerItem *>(ItemCreateScript( ITEMID_BANK_BOX ));
		pContainer->m_Attr |= ATTR_NEWBIE;
		pVendor->LayerAdd( pContainer, layer );
		// return( -1 );
	}
	addContents( pContainer, false, true );

	// add the names and prices for stuff.
	CCommand cmd;
	cmd.OpenBuy.m_Cmd = 0x74;
	cmd.OpenBuy.m_VendorUID = pContainer->GetUID();
	int len = sizeof( cmd.OpenBuy ) - sizeof(cmd.OpenBuy.items);

	CCommand * pCur = &cmd;
	int count = 0;
	for ( CItem* pItem=pContainer->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->m_amount ) continue;
		if ( pItem->GetID() == ITEMID_GOLD ) continue;
		pCur->OpenBuy.items[0].m_price = pItem->GetPrice();
		int lenname = sprintf( pCur->OpenBuy.items[0].m_text, pItem->GetName());
		pCur->OpenBuy.items[0].m_len = lenname + 1;
		lenname += sizeof( cmd.OpenBuy.items[0] );
		len += lenname;
		pCur = (CCommand *)( ((BYTE*) pCur ) + lenname );
		if ( ++count >= 255 ) break;
	}

	cmd.OpenBuy.m_len = len;
	cmd.OpenBuy.m_count = count;
	xSendPkt( &cmd, len );
	return( count );
}

bool CClient :: addShopMenuBuy( CChar * pVendor )
{
	// Try to buy stuff that the vendor has.
	addPause();
	addStatWindow( m_pChar->GetUID());	// Make sure the gold total has been updated.
	addChar( pVendor );			// Send the NPC again to make sure info is current. (OSI does this we might not have to)
	if ( addShopItems( pVendor, LAYER_VENDOR_STOCK ) <= 0 )
		return( false );
	if ( addShopItems( pVendor, LAYER_VENDOR_EXTRA ) < 0 )
		return( false );
	addOpenGump( pVendor, 0x30 );
	return( true );
}

int CClient :: addShopItemsSell( CCommand * pBase, CChar * pVendor, LAYER_TYPE layer )
{
	CContainerItem * pContainer = dynamic_cast <CContainerItem *> (pVendor->LayerFind( layer )); 
	if ( pContainer == NULL )
	{
		pContainer = dynamic_cast <CContainerItem *>( ItemCreateScript( ITEMID_BANK_BOX ));
		pContainer->m_Attr |= ATTR_NEWBIE;
		pVendor->LayerAdd( pContainer, layer );
		// return( 0 );
	}

	int len = pBase->OpenSell.m_len;
	int count = pBase->OpenSell.m_count;
	CCommand * pCur = (CCommand *)( ((BYTE*) pBase ) + len - ( sizeof( pBase->OpenSell ) - sizeof(pBase->OpenSell.items) ));

	for ( CItem* pItem=pContainer->GetContentHead(); pItem!=NULL; pItem=pItem->GetNext())
	{
		// ??? There could be multiple of this type ?
		CItem * pItemSell = m_pChar->ContentFind( pItem->GetID());
		if ( pItemSell == NULL ) continue;
		if ( ! pItemSell->IsSameID( pItem->GetID())) continue;
		if ( pItemSell->m_type != pItem->m_type ) continue;
		if ( pItemSell->m_Attr & ATTR_NEWBIE ) continue;

		pCur->OpenSell.items[0].m_UID = pItemSell->GetUID();
		pCur->OpenSell.items[0].m_id = pItemSell->GetID();
		pCur->OpenSell.items[0].m_color = pItemSell->m_color;
		pCur->OpenSell.items[0].m_amount = pItemSell->m_amount;
		pCur->OpenSell.items[0].m_price = pItemSell->GetPrice();

		int lenname = sprintf( pCur->OpenSell.items[0].m_text, pItemSell->GetName());
		pCur->OpenSell.items[0].m_len = lenname + 1;
		lenname += sizeof( pBase->OpenSell.items[0] );
		len += lenname;
		pCur = (CCommand *)( ((BYTE*) pCur ) + lenname );

		if ( ++count >= 75 ) break;
	}

	pBase->OpenSell.m_len = len;
	pBase->OpenSell.m_count = count;
	return( count );
}

bool CClient :: addShopMenuSell( CChar * pVendor )
{
	// What things do you have in your inventory that the vendor would want ?

	addPause();

	CCommand cmd;
	cmd.OpenSell.m_Cmd = 0x9E;
	cmd.OpenSell.m_UIDVendor = pVendor->GetUID();
	cmd.OpenSell.m_len = sizeof( cmd.OpenSell ) - sizeof(cmd.OpenSell.items);
	cmd.OpenSell.m_count = 0;

	addShopItemsSell( &cmd, pVendor, LAYER_VENDOR_SELL );
	int count = addShopItemsSell( &cmd, pVendor, LAYER_VENDOR_STOCK );

	if ( count == 0 ) 
	{
		pVendor->Speak( "Thou doth posses nothing of interest to me." );
		return( false );
	}

	xSendPkt( &cmd, cmd.OpenSell.m_len );
	return( true );
}

void CClient :: addBankOpen( CChar * pChar, LAYER_TYPE layer )
{
	switch( layer )
	{
	case LAYER_VENDOR_STOCK:
	case LAYER_VENDOR_EXTRA:
	case LAYER_VENDOR_SELL:
	case LAYER_BANKBOX:
		break;
	default:
		layer = LAYER_BANKBOX;
	}

	CContainerItem * pBankBox = dynamic_cast <CContainerItem *>( pChar->LayerFind( layer ));
	if ( pBankBox == NULL )
	{
		// Give them a bank box if not already have one.
		pBankBox = dynamic_cast <CContainerItem *>( ItemCreateScript( ITEMID_BANK_BOX ));
		pBankBox->m_Attr |= ATTR_NEWBIE;
		pChar->LayerAdd( pBankBox, layer );
	}

	// open it up.
	addItemEquip( pBankBox );	// may crash client if we dont do this.
	addContainerSetup( pBankBox );
}

void CClient :: addGumpMenu( int menuid )
{
	CScript s;
	if ( ! s.Open( GRAY_FILE "menu" GRAY_SCRIPT ))
		return;

	CString sSec;
	sSec.Format( "GUMPMENU %i", menuid );
	if ( ! s.FindSec(sSec))
		return;

	int length=21;
	int length2=1;
	while ( 1 )
	{
		if ( ! s.Read1()) break;
		length+=strlen(s.m_Line)+4;
		length2+=strlen(s.m_Line)+4;
	}

	sSec.Format( "GUMPTEXT %i", menuid );
	if ( ! s.FindSec(sSec))
		return;

	length+=3;
	int textlines=0;
	while ( 1 )
	{
		if ( ! s.Read1()) break;
		length+=(strlen(s.m_Line)*2)+2;
		textlines++;
	}

static BYTE Pkt_gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	PACKWORD( &Pkt_gump1[1], length );
	PACKDWORD( &Pkt_gump1[3], m_pChar->GetUID() );
	Pkt_gump1[7]=0;
	Pkt_gump1[8]=0;
	Pkt_gump1[9]=0;
	Pkt_gump1[10]=0x12; // Gump Number
	PACKWORD( &Pkt_gump1[19], length2 );
	xSend( Pkt_gump1, 21);

	sSec.Format( "GUMPMENU %i", menuid);
	if ( ! s.FindSec(sSec))
		return;
	while ( 1 )
	{
		if ( ! s.Read1()) break;
		CString sTmp;
		int len = sTmp.Format( "{ %s }", s.m_Line );
		xSend( sTmp, len );
	}

static BYTE Pkt_gump2[4]="\x00\x00\x00";
	PACKWORD( &Pkt_gump2[1], textlines );
	xSend( Pkt_gump2, 3);

	sSec.Format( "GUMPTEXT %i", menuid);
	if ( ! s.FindSec(sSec))
		return;
	while ( 1 )
	{
		if ( ! s.Read1()) break;

static BYTE Pkt_gump3[3]="\x00\x00";
		int len = strlen(s.m_Line);
		PACKWORD( &Pkt_gump3[0], len);
		xSend( Pkt_gump3, 2);
		Pkt_gump3[0]=0;
		for ( int i=0;i<len;i++)
		{
			Pkt_gump3[1]=s.m_Line[i];
			xSend( Pkt_gump3, 2);
		}
	}
}

////////////////////////////////////////////////////

bool CClient :: CanSee( const CObjBase * pObj ) const
{
	 // Can player see item b
	if ( m_pChar == NULL ) return( false );
	return( m_pChar->CanSee( pObj ));
}

bool CClient :: Can_Snoop_Container( CContainerItem * pItem )
{
	ASSERT( pItem->IsItem());
	ASSERT( pItem->IsContainer());

	CObjBase * pContainer = pItem->GetTopLevelObj();
	if ( pContainer == NULL ||
		pContainer == m_pChar ||
		pContainer->IsItem() || 
		IsPriv(PRIV_GM))
	{
		// this is yours 
		return( true );
	}

	CChar * pChar = (CChar *) pContainer;
	if ( ! m_pChar->Skill_UseQuick( SKILL_SNOOPING, pChar->m_Stat[STAT_DEX] ))
	{
		addSysMessage( "You failed to peek into the container." );
		return( false );
	}

	CString sMsg;
	sMsg.Format( "You notice %s peeking into your pack", m_pChar->GetName() );
	pChar->SysMessage( sMsg );

	// ??? others notice?
	return( true );
}

CPoint CClient :: Script_GetPlace( int i ) // Decode a teleport location number into X/Y/Z
{
	if ( i <= 0 )	// Get location from start list.
	{
		i = -i;
		if ( i > Serv.m_StartCount ) i = 0;
		return( Serv.m_Starts[i].m_p );
	}

	CPoint p;
	p.m_x = -1;	// invalid

	CScript s;
	if ( ! s.Open( GRAY_FILE "map" GRAY_SCRIPT ))
	{
		return( p );
	}

	CString sSec;
	sSec.Format( "LOCATION %i", i );
	if ( s.FindSec( sSec ))
	{
		while ( s.ReadParse()) 
		{
			if ( ! strcmpi( s.m_Line, "P" ))
				p.Read( s.m_pArg );
		}
	}
	return( p );
}

////////////////////////////////////////////////////////
// Targetted GM functions.

bool CClient :: OnTarg_Obj_Set( CObjUID uid )
{
	// Set some attribute of the item.
	// m_Targ_Text = new command and value.

	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
	{
		addSysMessage( "No object specified?" );
		return( false );
	}

	// Parse the command.
	char * pTemp = GetTempStr();
	strcpy( pTemp, m_Targ_Text );
	char * pVal;
	if ( ! Parse( pTemp, &pVal ))
	{
		if ( ! strcmpi( pTemp, "COLOR" ))
		{
			addDyeOption( pObj );
			return true;
		}
		addSysMessage( "Not enough args specified?" );
		return false ;
	}

	// ??? dont allow equipped items to be modified !

	pObj->Remove();	// strangly we need to remove this before color will take.
	if ( ! pObj->LoadVal( pTemp, pVal ))
	{
		addSysMessage( "Invalid Set" );
		return( false );
	}
	pObj->Update();
	return( true );
}

bool CClient :: OnTarg_Obj_Flip( CObjUID uid )
{
	// TARGMODE_FLIP
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) return false;
	pObj->Flip( m_Targ_Text );
	return true;
}

bool CClient :: OnTarg_Obj_Dupe( CObjUID uid )
{
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) return false;

	if ( pObj->IsItem())
	{
		if ( pObj->IsEquipped()) return false; // can't be equipped !!!
		CItem * pItemNew = ItemCreateDupe( dynamic_cast <CItem*> (pObj) );
		pItemNew->Update();
	}
	else
	{
		CChar * pCharOld = dynamic_cast <CChar*> (pObj);
		CChar * pCharNew = CharCreate( pCharOld->GetID() );
		pCharNew->MoveTo( pCharOld->m_p );
	}
	return true;
}

bool CClient :: OnTarg_Obj_Remove( CObjUID uid )
{
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
	{
		CString sTmp;
		sTmp.Format( "Must target a dynamic item %xh.", (DWORD) m_bin.Target.m_UID );
		addSysMessage( sTmp );
		return false;
	}
	if ( ! pObj->IsItem())
	{
		// remove npcs !
		CChar * pChar = dynamic_cast <CChar*> (pObj);
		if ( pChar->m_pClient != NULL )
		{
			addSysMessage( "Can't remove players this this way. Use 'Kick' or 'Kill'" );
			return false;
		}
	}
	delete pObj;
	return true;
}

bool CClient :: OnTarg_Obj_Set_Z( CObjUID uid )
{
	// m_Targ_Val = new z
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL )
		return false;
	bool fBlock;
	pObj->m_p.m_z = (m_Targ_Val==0xFFFF) ? World.GetHeight(pObj->m_p,fBlock) : m_Targ_Val;
	pObj->Update();
	return true;
}

bool CClient :: OnTarg_Char_Kill( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );
	if ( pChar->IsPriv( PRIV_COUNSEL|PRIV_GM|PRIV_Administrator ))
		return( false );
	pChar->Effect( 1, ITEMID_NOTHING, pChar );
	pChar->m_health = 0;
	return true;
}

bool CClient :: OnTarg_Char_Kick( CObjUID uid )
{
	// Kick them out.

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return false;
	if ( pChar->m_pClient == NULL )
		return false;
	if ( IsPriv( PRIV_Administrator )) 
		return false;

	CString sMsg;
	sMsg.Format( "You have been kicked out by %s %s", m_pChar->GetName(), (const char*) m_sAccount );
	pChar->m_pClient->addSysMessage( sMsg );

	CCommand cmd;
	cmd.Kick.m_Cmd = 0x26;
	cmd.Kick.m_unk1 = 0;
	pChar->m_pClient->xSendPkt( &cmd, sizeof( cmd.Kick ));

	// ??? block the account ?
	return true;
}

bool CClient :: OnTarg_Char_Set_Priv( CObjUID uid, const char * pszFlags )
{
	if ( ! IsPriv( PRIV_Administrator ))
		return( false );
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return false;
	if ( pChar->m_pClient == NULL ) 
		return false;

	BYTE Priv = 0;
	if ( pszFlags[0] >= '0' && pszFlags[0] <= '9' )
		Priv = ahextoi( pszFlags );
	else if ( ! strcmpi( pszFlags, "GM" ))
		Priv = PRIV_GM;
	else if ( ! strnicmp( pszFlags, "CO", 2 ))
		Priv = PRIV_COUNSEL;

	if ( Priv == 0 )
	{
		// Revoke GM status.
		// Remove GM Robe
		pChar->m_pClient->m_Priv = 0;
		pChar->ContentConsume( ITEMID_GM_ROBE, 0xFFFF );
	}

	if ( Priv & (PRIV_GM|PRIV_COUNSEL))
	{
		// Give gm robe.
		CItem * pItem = ItemCreateScript( ITEMID_GM_ROBE );
		pItem->m_Attr |= ATTR_NEWBIE | ATTR_MAGIC;

		if ( Priv & PRIV_GM )
		{
			Priv |= PRIV_GM_PAGE;
			pItem->m_color = 0x0021;
		}
		else
		{
			Priv |= PRIV_GM_PAGE;
			pItem->m_color = 0x0003;
		}
		pChar->UnEquipAllItems();
		pChar->ContentAdd( pItem );
	}

	pChar->m_pClient->m_Priv |= Priv;
	pChar->m_StatFlag = 0x00;
	pChar->Update();
	return true;
}

bool CClient :: OnTarg_Char_Bank( CObjUID uid )
{
	// Open the bank account of the person specified. TARGMODE_BANK
	// m_Targ_Index = layer ?
	// m_Targ_Val = create or destroy ?
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) return false;
	addBankOpen( pChar, (LAYER_TYPE) m_Targ_Index );
	return true;
}

bool CClient :: OnTarg_Char_Control( CObjUID uid )
{
	// TARGMODE_CONTROL
	// Possess this creature.
	// Leave my own body behind.

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) return false;

	// Maybe fight for control ?
	if ( pChar->m_pClient != NULL ) return( false );

	m_pChar->m_pClient = NULL;
	m_pChar->m_NPC_Brain = NPCBRAIN_ZOMBIE;
	m_pChar = pChar;
	addPlayerStart();

	return true;
}

bool CClient :: OnTarg_Char_Shrink( CObjUID uid )
{
	// TARGMODE_SHRINK 

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) return false ;
	if ( pChar->m_pClient != NULL ) return( false );

	if ( ! IsPriv( PRIV_GM ))
	{
		// we must own it.
		if ( pChar->m_owner != m_pChar->GetUID()) return( false );
	}

	// turn it to a figurine.
	CItem * pItem = ItemCreateScript( pChar->m_pCre->m_trackID );
	pItem->m_type = ITEM_FIGURINE;
	pItem->m_more1 = pChar->GetID();
	pItem->PutOnGround( pChar->m_p );

	delete pChar;
	return( true );
}

bool CClient :: OnTarg_Item_Add()
{
	// m_Targ_Val = new char id
	CItem * pItem = ItemCreateScript( (ITEMID_TYPE) m_Targ_Val );
	if ( pItem == NULL )
		return( false );
	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	pItem->PutOnGround( p );
	return true;
}

bool CClient :: OnTarg_Item_Door_Link( CObjUID uid )
{
	// TARGMODE_DOOR_LINK 

	CItem * pItem1 = uid.ItemFind();
	if ( pItem1 == NULL || 
		( pItem1->m_type != ITEM_DOOR &&
		pItem1->m_type != ITEM_DOOR_LOCKED &&
		pItem1->m_type != ITEM_SWITCH &&
		pItem1->m_type != ITEM_KEY
		))
	{
		addSysMessage( "Must target a door, switch or key object." );
		return( false );
	}

	if (( pItem1->m_type != ITEM_DOOR ||
		pItem1->m_type != ITEM_DOOR_LOCKED ) &&
		! pItem1->IsDoorID( pItem1->GetID()))
	{
		addSysMessage( "That's not a valid door." );
		return( false );
	}

	CItem * pItem2 = m_Targ_UID.ItemFind();
	if ( pItem2 == NULL )
	{
		m_Targ_UID = uid;
		addTarget( TARGMODE_DOOR_LINK, "What do you want to link it to ?", true );
		return true;
	}

	if ( pItem2 == pItem1 )
	{
		addSysMessage( "That's the same door. Try the other one." );
		return false;
	}

	if ( pItem1->m_type == ITEM_KEY || pItem2->m_type == ITEM_KEY )
	{
		if ( pItem1->m_lockID == 0 )
			pItem1->m_lockID = pItem2->GetUID();
		pItem2->m_lockID = pItem1->m_lockID;
	}
	else
	{
		pItem1->m_Door_Link = pItem2->GetUID();
		pItem2->m_Door_Link = pItem1->GetUID();
	}

	addSysMessage( "These doors are now linked." );
	return true;
}

bool CClient :: OnTarg_Test_Bolt( void )
{
	// BOLT type, id, speed, loop, explode
	// m_Targ_Text = args.
#ifdef COMMENT
		if (Arg_Qty==3)
		{
			m_pChar->Effect( Arg_hex_num(1), (ITEMID_TYPE) Arg_hex_num(2), m_pChar );
		}
		else if (Arg_Qty==2)
		{
			m_pChar->Effect( Arg_hex_num(1), ITEMID_NOTHING, m_pChar );
		} 
		else
		{
			m_pChar->Effect( 1, ITEMID_NOTHING, m_pChar );
		}
#endif
	return true;
}

//-----------------------------------------------------------------------
// Targetted Informational skills

bool CClient :: OnTarg_Skill_Begging( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );

	CString sMsg;
	sMsg.Format( "You grovel at %s's feet", pChar->GetName());
	addSysMessage(sMsg);

	m_pChar->UpdateAnimate( ANIM_BOW );
	return true;
}

bool CClient :: OnTarg_Skill_ItemID( CObjUID uid )
{
	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	CUOMapMeter meter;
	World.GetMapMeter( p, meter );

	CString sMsg;
	sMsg.Format( "Base=%xh", meter.m_wIndex );
	addSysMessage(sMsg);

	if ( m_bin.Target.m_UID == 0 )
	{
		WORD id = m_bin.Target.m_id;
		if ( id )
		{
			// static items have no uid's but we can still use them.
			sMsg.Format( "Item [Static] ID=%x", (WORD) m_bin.Target.m_id );
		}
		else
		{
			// tile info for location.
			sMsg.Format( "No static tile" );
		}
	}
	else if ( uid.IsItem())
	{
		CItem * pItem = uid.ItemFind();
		if ( pItem == NULL ) 
			return( false );

#ifdef COMMENT
		if ( pItem->m_Attr & ( ATTR_MAGIC | ATTR_NEWBIE ) )
		{
			len += sprintf( pTemp+len, "Magic " );
		}

#endif

		sMsg.Format( "Item [Dynamic] Ser=%lx ID=%x Name='%s' Color=%x "
			"Type=%x More=%lx,%lx "
			"Pos=%i,%i,%i Amount=%i",
			(DWORD) uid, pItem->GetID(),
			pItem->GetName(), pItem->m_color,
			pItem->m_type,	pItem->m_more1, pItem->m_more2,
			pItem->m_p.m_x, pItem->m_p.m_y, pItem->m_p.m_z, pItem->m_amount );
	}
	else
	{
		CChar * pChar = uid.CharFind();
		if ( pChar == NULL )
			return( false );

		sMsg.Format( "Ser=%lx ID=%x Name='%s' Skin=%x Account='%s',"
			"Stat=%lx Pos=%i,%i,%i Act=%d, Brain=%d",
			(DWORD) uid, pChar->GetID(),
			pChar->GetName(), pChar->m_color,
			(const char*) pChar->m_sAccount,
			pChar->m_StatFlag,
			pChar->m_p.m_x, pChar->m_p.m_y, pChar->m_p.m_z,
			pChar->m_Act_Skill, pChar->m_NPC_Brain );
	}

	addSysMessage(sMsg);
	return true;
}

bool CClient :: OnTarg_Skill_EvalInt( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) 
	{
		addSysMessage( "That does not appear to be a living being." );
		return( false );
	}

	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	const char * pDesc;
	int iVal = pChar->m_Stat[STAT_INT];

	if (iVal <= 10)
		pDesc = "slightly less intelligent than a rock";
	else if (iVal <= 20)
		pDesc = "fairly stupid";
	else if (iVal <= 30)
		pDesc = "not the brightest";
	else if (iVal <= 40)
		pDesc = "about average";
	else if (iVal <= 50)
		pDesc = "moderately intelligent";
	else if (iVal <= 60)
		pDesc = "very intelligent";
	else if (iVal <= 70)
		pDesc = "extraordinarily intelligent";
	else if (iVal <= 80)
		pDesc = "like a formidable intellect, well beyond the ordinary";
	else if (iVal <=90)
		pDesc = "like a definite genius";
	else if (iVal > 90)
		pDesc = "superhumanly intelligent in a manner you cannot comprehend";

	CString sMsg;
	sMsg.Format( "%s looks %s.", pChar->GetName(), pDesc );
	addSysMessage( sMsg );
	return true;
}

bool CClient :: OnTarg_Skill_AnimalLore( CObjUID uid )
{
	// The creature is a "human" etc..
	// How happy.
	// Well trained.
	// Who owns it ?
	// What it eats.
	
	// Other "lore" type things about a creature ? 
	// ex. liche = the remnants of a powerful wizard 
	
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );

	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );
	
	const char * pHe = pChar->GetPronoun();
	const char * pHis = pChar->GetPossessPronoun();

	CString sTmp;

	// What kind of animal.
	if ( ! pChar->m_sName.IsEmpty())
	{
		sTmp.Format( "%s is a %s", pChar->GetName(), pChar->m_pCre->m_name );
		addItemMessage( sTmp, pChar );
	}

	// Who is master ?
	if ( ! pChar->m_owner.IsValidUID())	
	{   
		sTmp.Format( "%s is %s own master", pHe, pHis );
	}
	else
	{
		CChar * pCharOwner = pChar->m_owner.CharFind();
		sTmp.Format( "%s is owned by %s", pHe, pCharOwner->GetName() );
	}
	addItemMessage( sTmp, pChar );

	if ( pChar->m_StatFlag & STATF_Conjured )
	{
		addItemMessage( "This is a conjured creature", pChar );
	}
	return true;
}

bool CClient :: OnTarg_Skill_ArmsLore( CObjUID uid )
{
	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL )
		return( false );

	switch ( pItem->m_type )
	{
	case ITEM_ARMOR:				// some type of armor. (no real action)
	case ITEM_WEAPON_MACE:			// Can be used for smithing ?
	case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:
	case ITEM_WEAPON_FENCE:
	case ITEM_WEAPON_BOW:
		break;
	default:
		addSysMessage( "That does not appear to be a weapon or armor." );
 		return false;
	}

	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	// Magical effects ?

	// Poisoned ?

	CString sTmp;
	sTmp.Format( "Attack/Defense [%i]", pItem->m_pDef->m_attack );
	addSysMessage(sTmp);
	return true;
}

bool CClient :: OnTarg_Skill_Anatomy( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
	{
		addItemMessage( "That does not appear to be a living being.", pChar );
		return( false );
	}

	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	// Add in error cased on your skill level.

	int val = pChar->m_Stat[STAT_STR];
	const char * pStr;
	if ( val <= 10)
		pStr = "rather feeble";
	else if (val <= 20)
		pStr = "somewhat weak";
	else if (val <= 30)
		pStr = "to be of normal strength";
	else if (val <= 40)
		pStr = "somewhat strong"; 
	else if (val <= 50)
		pStr = "very strong";
	else if (val <= 60)
		pStr = "extremely strong"; 
	else if (val <= 70)
		pStr = "extraordinarily strong";
	else if (val <= 80)
		pStr = "as strong as an ox";
	else if (val <= 90)
		pStr = "like one of the strongest people you have ever seen";
	else if (val > 90)
		pStr = "superhumanly strong"; 

	const char * pDex;
	val = pChar->m_Stat[STAT_DEX];

	if (val <= 10) 
		pDex = "very clumsy";
	else if (val <= 20)
		pDex = "somewhat uncoordinated";
	else if (val <= 30)
		pDex = "moderately dexterous";
	else if (val <= 40)
		pDex = "somewhat agile";
	else if (val <= 50)
		pDex = "very agile";
	else if (val <= 60)
		pDex = "extremely agile";
	else if (val <= 70)
		pDex = "extraordinarily agile";
	else if (val <= 80)
		pDex = "like they move like quicksilver";
	else if (val <= 90)
		pDex = "like one of the fastest people you have ever seen";
	else if (val > 90) 
		pDex =  "superhumanly agile";

	CString sTmp;
	sTmp.Format( "%s looks %s and %s.", pChar->GetName(), pStr, pDex );
	addItemMessage( sTmp, pChar );

	if ( pChar->m_StatFlag & STATF_Conjured )
	{
		addItemMessage( "This is a magical creature", pChar );
	}

	// ??? looks hungry ?
	return true;
}

bool CClient :: OnTarg_Skill_Forensics( CObjUID uid )
{
	// ??? can't target corpses ?

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL || pItem->GetID() != ITEMID_CORPSE )
	{
		addSysMessage( "Forensics must be used on a corpse." );
		return( false );
	}

	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	// STATF_Sleeping 
	// "They are not dead but mearly unconscious"

	CObjUID uidKiller( pItem->m_corpse_killer_UID );
	CChar * pChar = uidKiller.CharFind();
	const char * pName = ( pChar != NULL ) ? pChar->GetName() : NULL;

	char * pTemp = GetTempStr();
	if ( pItem->m_corpse_deathtime )
	{
		int len = sprintf( pTemp, "This is the corpse of %s and it is %d seconds old. ",
			pItem->GetName(), World.m_Clock_Time - pItem->m_corpse_deathtime );
		if ( pName == NULL )
		{
			sprintf( pTemp+len, "You cannot determine who killed it" );
		}
		else
		{
			sprintf( pTemp+len, "It looks to have been killed by %s", pName );
		}
	}
	else
	{
		int len = sprintf( pTemp, "This is the corpse of %s and it is has been carved up. ",
			pItem->GetName() );
		if ( pName == NULL )
		{
			sprintf( pTemp+len, "You cannot determine who killed it." );
		}
		else
		{
			sprintf( pTemp+len, "It looks to have been carved by %s", pName );
		}
	}
	addSysMessage( pTemp );
	return true;
}

bool CClient :: OnTarg_Skill_Tasting( CObjUID uid )
{
	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	return true;
}

////////////////////////////////////////
// Targeted skills and actions.

bool CClient :: OnTarg_Skill_Tame( CObjUID uid )
{
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );

	if ( pChar == m_pChar )
	{
		addSysMessage( "You are now your own master." );
		m_pChar->m_owner.ClearUID();
		return( false );
	}
	
	// Is it tamable ?
	if ( ! IsPriv( PRIV_GM ))
	{
		if ( pChar->GetID() >= CREID_MAN )
		{
			addSysMessage( "This creature is not tamable" );
			return( false );
		}
		if ( pChar->m_owner.IsValidUID())
		{
			addSysMessage( "This creature is already tame." );
			return( false );
		}
	}

	m_pChar->m_Act_Targ = uid;
	return( m_pChar->Skill_Start( 25 ));
}

bool CClient :: OnTarg_Skill_Herd_Pick( CObjUID uid )
{
	// Selected a creature to herd 
	// Move the selected item or char to this location.
	if ( uid.IsItem())
	{
		// ??? hit it ?
		addSysMessage( "Try just picking up inanimate objects" );
		return( false );
	}

	CChar * pChar = uid.CharFind();
	if ( pChar == NULL )
		return( false );

	// special GM version to move to coordinates.
	if ( IsPriv( PRIV_GM | PRIV_COUNSEL ))
	{
		if ( m_pChar->m_Act_p.IsValid() )
		{
			pChar->Spell_Teleport( m_pChar->m_Act_p );
			return( false );
		}
	}
	else
	{
		// Herdable ?
	}

	m_Targ_UID = uid;
	addTarget( TARGMODE_SKILL, "Where do you want them to go ?", false );
	return( true );
}

bool CClient :: OnTarg_Skill_Herd_Move()
{
	// SKILL_HERDING 
	CChar * pChar = m_Targ_UID.CharFind();
	if ( pChar == NULL )
		return( false );

	// How do I make them move fast ? or with proper speed ???
	if ( ! m_pChar->Skill_Start( 5 ))
		return( false );

	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	if ( IsPriv( PRIV_GM ))
	{
		pChar->Spell_Teleport( p );
		return( false );
	}

	//
	// Try to make them walk there.
	//
	pChar->m_Act_p = p;
	pChar->Skill_Setup( NPCACT_GOTO );
	addSysMessage( "The animal goes where it is instructed" );
	return( false );
}

bool CClient :: OnTarg_Skill_Steal( CObjUID uid )
{
	// remove this type of item from char.

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL ) return( false );

	CObjBase * pContainer = pItem->GetTopLevelObj();	// Who has it ?
	if ( pContainer == m_pChar )
	{
		addSysMessage( "Stealing from yourself?" );
		return( false );
	}

	CChar * pCharMark = NULL;
	if ( pContainer != NULL && ! pContainer->IsItem() )
	{
		pCharMark = (CChar *) pContainer;
	}

	if ( ! IsPriv( PRIV_GM ))
	{
		// Can't steal newbie items or a main pack.

		if ( ! pItem->GetContainer()->IsItem() || ( pItem->m_Attr & ATTR_NEWBIE ))
		{
			// Always caught doing this !!!
			addSysMessage( "Naughty !" );
			return( false );
		}
	}

	int iDifficulty = 35;
	if ( pCharMark != NULL )
	{
		iDifficulty = pCharMark->m_Stat[ STAT_DEX ] + pCharMark->m_Skill[ SKILL_STEALING ];
	}

	const char * pszSuccess;
	if ( m_pChar->Skill_Start( iDifficulty ))
	{
		pItem->m_Attr &= ~ATTR_OWNED;	// Now it's mine
		pItem->Remove();
		// Put in my invent.
		m_pChar->GetPack()->ContentAdd( pItem );
		pszSuccess = "stealing";
	}
	else
	{
		pszSuccess = "attempting to steal";
	}

	// ??? noticed ?
	if ( pCharMark != NULL )
	{
		CString sMsg;
		sMsg.Format( "You notice %s %s %s from you.", m_pChar->GetName(), pszSuccess, pItem->GetName());
		pCharMark->SysMessage( sMsg );

		// Alert the guards !!!
	}
	return( false );
}

bool CClient :: OnTarg_Skill_Magery()
{
	// The client player has targeted a spell.

	CObjUID uid( m_bin.Target.m_UID );
	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	return( m_pChar->Spell_Cast( uid, p ));
}

bool CClient :: OnTarg_Skill_Fishing()
{
	if ( m_bin.Target.m_UID != 0 ||
		! CItem::IsWaterID( (ITEMID_TYPE)(WORD) m_bin.Target.m_id ))
	{
		addSysMessage( "You need to be closer to the water to fish!" );
		return( false );
	}

	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );
	m_pChar->m_Act_p = p;
	return( m_pChar->Skill_Start( 5 ));
}

bool CClient :: OnTarg_Skill()
{
	// targetted skill now has it's target.
	// response to TARGMODE_SKILL

	bool fContinue = false;
	switch ( m_pChar->m_Act_Skill )
	{
		// Informational skills.
	case SKILL_ANIMALLORE:	OnTarg_Skill_AnimalLore( m_bin.Target.m_UID ); break;
	case SKILL_ARMSLORE:	OnTarg_Skill_ArmsLore( m_bin.Target.m_UID );  break;
	case SKILL_ANATOMY:		OnTarg_Skill_Anatomy( m_bin.Target.m_UID );  break;
	case SKILL_ITEMID:		OnTarg_Skill_ItemID( m_bin.Target.m_UID );  break;
	case SKILL_EVALINT:		OnTarg_Skill_EvalInt( m_bin.Target.m_UID );  break;
	case SKILL_FORENSICS:	OnTarg_Skill_Forensics( m_bin.Target.m_UID ); break;
	case SKILL_TASTEID:		OnTarg_Skill_Tasting( m_bin.Target.m_UID ); break;

		// Instant response type skills.
	case SKILL_BEGGING:		OnTarg_Skill_Begging( m_bin.Target.m_UID );	break;


		// Delayed response type skills.

	case SKILL_STEALING:	fContinue = OnTarg_Skill_Steal( m_bin.Target.m_UID ); break;

	case SKILL_MAGERY:		fContinue = OnTarg_Skill_Magery();	break;
	case SKILL_FISHING:		fContinue = OnTarg_Skill_Fishing();	break;
	case SKILL_TAMING:		fContinue = OnTarg_Skill_Tame( m_bin.Target.m_UID ); break;
	case SKILL_HERDING:		fContinue = OnTarg_Skill_Herd_Move(); break;

	case SKILL_ENTICEMENT:
	case SKILL_PROVOCATION:

	case SKILL_POISONING:	// 30
		// Use what poison ?
		break;

	case SKILL_TAILORING:
		// Use what material ?
		break;

	case SKILL_LUMBERJACKING:
		// What tree to chop?
		break;

	case SKILL_MINING:
		// Mine where ?
		break;
	}

	if ( ! fContinue )
	{
		m_pChar->Skill_Cleanup();
	}
	return true;
}

bool CClient :: OnTarg_Pet_Command()
{
	// Any pet command requiring a target.
	CChar * pCharPet = m_Targ_UID.CharFind();
	if ( pCharPet == NULL ) 
		return false;

	CObjUID uid( m_bin.Target.m_UID );
	CChar * pCharTarg = uid.CharFind();
	if ( pCharTarg == NULL && m_Targ_Index != 4 ) 
		return false;

	switch ( m_Targ_Index )
	{
	case 0:	// "TRANSFER"
		// transfer ownership via the transfer command.
		pCharPet->m_owner = pCharTarg->GetUID();
		break;
	case 1:	// "KILL" 
	case 2: // "ATTACK"
		// Attack the target.
		pCharPet->Attack( pCharTarg );
		break;
	case 3:	// "FOLLOW"
		break;
	case 4:	// "GO"
		// Go to the location x,y
		break;
	case 5:	// "FRIEND",
	case 6:	// "GUARD",
	case 7:	// "FETCH",
		break;
	}
	return true;
}

//-----------------------------------------------------------------------
// Targetted items with special props.

bool CClient :: OnTarg_Use_Key( CItem * pItem, CItem * pKey )
{
	if ( pItem == NULL || pItem->m_lockID == 0 )
	{
		addSysMessage( "That does not have a lock.");
 		return false;
	}
	if ( pKey == pItem )	// Rename the key.
	{
		// We may rename the key.
		SetTargMode( TARGMODE_NAME_KEY, "What would you like to name the key?" );
 		return false;
	}
	if ( pKey->m_lockID != 0xFFFFFFFF )	// Skeleton key.
	{
		if ( pItem->m_lockID != pKey->m_lockID )
		{
			addSysMessage( "The key does not fit into that lock.");
	 		return false;
		}
	}

	if ( pItem->IsSameID( ITEMID_SIGN_BRASS ))
	{
		// We may rename the sign.
		m_Targ_UID = pItem->GetUID();
		SetTargMode( TARGMODE_NAME_SIGN, "What should the sign say?" );
 		return true;
	}

	switch ( pItem->m_type )
	{
	case ITEM_CONTAINER:
		pItem->m_type=ITEM_CONTAINER_LOCKED;
		addSysMessage( "You lock the container.");
		break;
	case ITEM_CONTAINER_LOCKED:
		pItem->m_type=ITEM_CONTAINER;
		addSysMessage( "You unlock the container.");
		break;
	case ITEM_DOOR:
		pItem->m_type=ITEM_DOOR_LOCKED;
		addSysMessage( "You lock the door.");
		break;
	case ITEM_DOOR_LOCKED:
		pItem->m_type=ITEM_DOOR;
		addSysMessage( "You unlock the door.");
		break;
	default:
		addSysMessage( "That does not have a lock.");
 		return false;
	}

	pItem->Sound( 0x049 );
	return true;
}

bool CClient :: OnTarg_Use_Item()
{
	// TARGMODE_USE_ITEM
	// Targetted an item to be used on something else.
	// Not skill related.
	// m_Targ_UID = what is the used object (uid)
	
	CItem * pItemUse = m_Targ_UID.ItemFind();
	if ( pItemUse == NULL ) 
 		return false;

	CPoint p( m_bin.Target.m_x, m_bin.Target.m_y, m_bin.Target.m_z );

	CChar * pCharTarg = NULL; 
	CItem * pItemTarg = NULL;
	CObjUID uid( m_bin.Target.m_UID );
	CObjBase * pObjTarg = uid.ObjFind();
	if ( pObjTarg != NULL )
	{
		if ( pObjTarg->IsItem())
			pItemTarg = (CItem*) pObjTarg;
		else
			pCharTarg = (CChar*) pObjTarg;
	}

	switch ( pItemUse->GetID() )
	{
	case ITEMID_DYE:
		if (( pItemTarg != NULL && pItemTarg->IsSameID( ITEMID_DYEVAT )) ||
			( pCharTarg != NULL && ( pCharTarg == m_pChar || IsPriv( PRIV_GM ))))	// Change skin color.
		{
			addDyeOption( pObjTarg );
	 		return true;
		}
		addSysMessage( "You can only use this item on a dye vat.");
 		return false;

	case ITEMID_DYEVAT:

		if ( pObjTarg == NULL )
	 		return false;

		if ( pCharTarg != NULL )
		{
			// Dye hair.
			if ( pCharTarg != m_pChar && 
				! IsPriv( PRIV_GM ))	// Change hair color.
		 		return false;
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
			{
				pObjTarg->m_color = pItemUse->m_color;
				pObjTarg->Update();
			}
			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg == NULL ) 
		 		return true;
		}
		else if ( ! IsPriv( PRIV_GM ) &&
			! ( pItemTarg->m_pDef->m_Can & CAN_DYE ) &&
			pItemTarg->m_type != ITEM_CLOTHING )
		{
			addSysMessage( "The dye just drips off this.");
			return false;
		}
		pObjTarg->m_color = pItemUse->m_color;
		pObjTarg->Update();
 		return true;

	case ITEMID_SCISSORS1:
	case ITEMID_SCISSORS2:
		// cut hair as well ?
		if ( pCharTarg != NULL )
		{
			pObjTarg = pCharTarg->LayerFind( LAYER_BEARD );
			if ( pObjTarg != NULL )
				delete pObjTarg;
			pObjTarg = pCharTarg->LayerFind( LAYER_HAIR );
			if ( pObjTarg != NULL )
				delete pObjTarg;
			m_pChar->Sound( 0x248 );	// snip noise.
	 		return true;
		}
		addSysMessage( "Can't use scissors this way" );
 		return false;

	case ITEMID_Shovel:
	case ITEMID_Pickaxe1:
	case ITEMID_Pickaxe2:	// 	SKILL_MINING
	case ITEMID_Pitchfork:
		// Mine at the location.

		break;

	case ITEMID_BANDAGES:	// SKILL_HEALING, or SKILL_VETERINARY
		Cmd_Skill_Heal( pItemUse, pCharTarg );
		break;

	case ITEMID_KEYRING:
		// ??? Use a keyring on self = remove all keys.
		// else it acts as a key.
		break;

	case ITEMID_DEED1:
	case ITEMID_DEED2:
	case ITEMID_SHIP_PLANS1:
	case ITEMID_SHIP_PLANS2:
		// Place the house here. (if possible)
		pItemTarg = ItemCreateScript( m_Targ_Val );
		pItemTarg->PutOnGround( p );
		pItemTarg->CreateMulti( m_pChar );
		delete pItemUse;	// consume the deed.
 		return true;
	}

	switch ( pItemUse->m_type )
	{
	case ITEM_KEY:
		return( OnTarg_Use_Key( pItemTarg, pItemUse ));

	case ITEM_WEAPON_MACE:
	case ITEM_WEAPON_MACE_SHARP:// 22 = war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:		// 23 = 
	case ITEM_WEAPON_FENCE:		// 24 = can't be used to chop trees.
		// Use on tree or corpse to cut.
		// Item to smash ?
		if ( pItemTarg != NULL )
		{
			// Near an anvil ?
			// Near a forge ?
			if ( pItemTarg->GetID() == ITEMID_CORPSE )
			{
				m_pChar->CarveCorpse( dynamic_cast <CContainerItem *>( pItemTarg ));
		 		return true;
			}
		}
		if ( pCharTarg != NULL )
		{
			// Sheep have wool.

		}
 		return true;
	}

	addSysMessage( "The object is strange" );
	return false;
}

/////////////////////////////////////////////

void CClient :: Event_Target()
{
	// If player clicks on something with the targetting cursor
	// Assume addTarget was called before this.

	if ( m_bin.Target.m_code != m_Targ_Mode )
	{
		DEBUG_ERR(( "ERROR %x: Unrequested target info ?\n", GetSocket() ));
		return;
	}
	if ( m_bin.Target.m_x == 0xFFFF && m_bin.Target.m_UID == 0 )
	{
		// canceled
		SetTargMode();
		return;
	}

	TARGMODE_TYPE prevmode = m_Targ_Mode;
	m_Targ_Mode = TARGMODE_NONE;

	bool fSuccess = false;

	switch ( prevmode )
	{
		// GM stuff.

	case TARGMODE_TEST_BOLT:		fSuccess = OnTarg_Test_Bolt();	break;

	case TARGMODE_REMOVE:			fSuccess = OnTarg_Obj_Remove( m_bin.Target.m_UID );  break;
	case TARGMODE_SET:				fSuccess = OnTarg_Obj_Set( m_bin.Target.m_UID ); break;
	case TARGMODE_FLIP:				fSuccess = OnTarg_Obj_Flip( m_bin.Target.m_UID ); break;
	case TARGMODE_DUPE:				fSuccess = OnTarg_Obj_Dupe( m_bin.Target.m_UID ); break;
	case TARGMODE_SET_Z:			fSuccess = OnTarg_Obj_Set_Z( m_bin.Target.m_UID );  break;

	case TARGMODE_ADDITEM:			fSuccess = OnTarg_Item_Add(); break;
	case TARGMODE_DOOR_LINK:		fSuccess = OnTarg_Item_Door_Link( m_bin.Target.m_UID ); break;

	case TARGMODE_KILL:				fSuccess = OnTarg_Char_Kill( m_bin.Target.m_UID );  break;
	case TARGMODE_KICK:			    fSuccess = OnTarg_Char_Kick( m_bin.Target.m_UID );  break;
	case TARGMODE_SET_PRIV:			fSuccess = OnTarg_Char_Set_Priv( m_bin.Target.m_UID, m_Targ_Text );  break;
	case TARGMODE_BANK:				fSuccess = OnTarg_Char_Bank( m_bin.Target.m_UID ); break;
	case TARGMODE_CONTROL:			fSuccess = OnTarg_Char_Control( m_bin.Target.m_UID ); break;

	case TARGMODE_SHRINK:			fSuccess = OnTarg_Char_Shrink( m_bin.Target.m_UID ); break;

		// Player stuff.

	case TARGMODE_SKILL:			fSuccess = OnTarg_Skill(); break;
	case TARGMODE_SKILL_HERD_PICK:	fSuccess = OnTarg_Skill_Herd_Pick( m_bin.Target.m_UID ); break;
	case TARGMODE_USE_ITEM:			fSuccess = OnTarg_Use_Item();  break;
	case TARGMODE_PET_CMD:			fSuccess = OnTarg_Pet_Command(); break;
	}
}

/////////////////////////////////////////////

void CClient :: Cmd_GM_Page( const char * reason ) // Help button (Calls GM Call Menus up)
{
	CString sMsg;
	sMsg.Format( "GM Page from %s [%lx]: %s",
		m_pChar->GetName(), m_pChar->GetUID(), reason );

	bool fFound=0;
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient->IsPriv( PRIV_GM_PAGE )) // found GM
		{
			fFound=1;
			pClient->addSysMessage(sMsg);
		}
	}
	if ( ! fFound) 
	{
		addSysMessage( "There was no Game Master available to take your call.");
		return;
	}

	addSysMessage( "Available Game Masters have been notified of your request.");
	// ??? Add to the GM page list. CGMPage
}

void CClient :: Cmd_CreateItem( int id )
{
	// make an item near by.
	m_Targ_Val = id;
	addTarget( TARGMODE_ADDITEM, "Where would you like to create the item ?", false );
}

static char Txt_Summon[] = "Where would you like to summon the creature ?";

void CClient :: Cmd_CreateChar( int id )
{
	// make a creature near by. (GM or script used only)
	m_pChar->Skill_Setup( SKILL_MAGERY );
	m_pChar->m_Act_Spell = SPELL_Summon;
	m_pChar->m_Act_Arg = id;

	addTarget( TARGMODE_SKILL, Txt_Summon, false );
}

void CClient :: Cmd_Skill_Camping( CItem * pKindling )
{
	if ( pKindling->GetContainer() != NULL )
	{
		addSysMessage( "You can't light the kindling in a container" );
		return;
	}

	if ( ! m_pChar->Skill_UseQuick( SKILL_CAMPING, 10 ))
		return;

	pKindling->SetID( ITEMID_CAMPFIRE );
	pKindling->Update();
}

void CClient :: Cmd_Skill_Magery( SPELL_TYPE iSpell, CObjBase * pSrc )
{
	// start casting a spell. prompt for target.
	// pSrc = you the char.
	// pSrc = magic object is source ?
	// NULL = natural effect.

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Cast Spell %d\n", GetSocket(), iSpell ));
	}

	// Do we have the regs ?

	if ( ! m_pChar->Spell_CanCast( iSpell )) return;

	m_pChar->Skill_Setup( SKILL_MAGERY );
	m_pChar->m_Act_Spell = iSpell;

	const char * pPrompt = "Select Target";
	switch ( iSpell )
	{
	case SPELL_Recall:	
		pPrompt = "Select rune to recall from.";
		break;
	case SPELL_Blade_Spirit:
		pPrompt = Txt_Summon;
		break;
	case SPELL_Summon:
		m_pChar->m_Act_Arg = CREID_GORILLA;	// menu or random ?
		pPrompt = Txt_Summon;
		break;
	case SPELL_Mark:
		pPrompt = "Select rune to mark.";
		break;
	case SPELL_Gate_Travel:	// gate travel
		pPrompt = "Select rune to gate from.";
		break;
	case SPELL_Polymorph:	
		// polymorph creature menu.
		addScriptMenu( 60, TARGMODE_MENU_ITEMS );
		return;
	case SPELL_Earthquake:
		// cast immediately with no targetting.
		m_pChar->Spell_Cast( m_pChar->GetUID(), m_pChar->m_p );
		return;
	case SPELL_Resurrection:	// resurrection
		pPrompt = "Select ghost to resurrect.";
		break;
	case SPELL_Vortex:	// energy vortex.
	case SPELL_Air_Elem:	// air elem
	case SPELL_Daemon:	// Daemon
	case SPELL_Earth_Elem:	// Earth elem
	case SPELL_Fire_Elem:	// Fire elem
	case SPELL_Water_Elem:	// Water elem.
		pPrompt = Txt_Summon;
		break;

		// Necro
	case SPELL_Summon_Undead: // Summon an undead
		pPrompt = Txt_Summon;
		break;
	case SPELL_Animate_Dead: // Corpse to zombie
		pPrompt = "Choose a corpse";
		break;
	case SPELL_Bone_Armor: // Skeleton corpse to bone armor
		pPrompt = "Chose a skeleton";
		break;
	}

	addTarget( TARGMODE_SKILL, pPrompt, Spells[ iSpell ].m_wFlags & ( SPELLFLAG_TARG_OBJ | SPELLFLAG_TARG_CHAR ));
}

bool CClient :: Cmd_Skill_Tracking()
{
	// look around for stuff.

	CMenuItem item[32];	// m_Targ_Menu ???
	int count = 0;

	item[0].m_id = TARGMODE_MENU_SKILL;
	item[0].m_text.Format( "Tracking" );

	CWorldSearch AreaChars( m_pChar->m_p, m_pChar->m_Skill[SKILL_TRACKING]/2 + 10 );
	while ( 1 )
	{
		CChar * pChar = AreaChars.GetChar();
		if ( pChar == NULL ) break;
		if ( m_pChar == pChar ) continue;

		count ++;
		item[count].m_id = 	pChar->m_pCre->m_trackID;
		item[count].m_text.Format( pChar->GetName() );
		item[count].m_context = pChar->GetUID();
		if ( count >= COUNTOF( item )-1 ) break;
	}

	if ( ! count )
	{
		addSysMessage( "You see no signs of creatures" );
		return( false );
	}

	m_Targ_UID.ClearUID();
	m_pChar->Skill_Start( 10 );
	addItemMenu( item, count );
	return( true );
}

bool CClient ::  Cmd_Skill_Inscription( int iLevel )
{
	// Select the scroll type to make.

	CMenuItem item[32];

	// we should already be in inscription skill mode.

	// Do we have the resources still ? ITEMID_SCROLL_BLANK
	if ( ! m_pChar->ContentFind( ITEMID_SCROLL_BLANK ))
	{
		addSysMessage( "You have no blank scrolls" );
		return( false );
	}

	item[0].m_id = TARGMODE_MENU_SKILL;
	const char * pname;
	if ( iLevel ) 
		pname = "Spell Circle %d";
	else
		pname = "Spell Circles" ;
	item[0].m_text.Format( pname, iLevel );

	int count = 0;
	for ( int k = 1; k <= 8; k ++ )
	{
		count ++;
		if ( iLevel )
		{
			int ispell = (iLevel-1)*8 + (k-1);
			item[count].m_id  =	Spells[ ispell ].m_SpellID;
			item[count].m_text = Spells[ispell].m_name;
		}
		else
		{
			item[count].m_id  =	ITEMID_SPELL_CIRCLE1 + count - 1;
			item[count].m_text.Format( "Spell Circle %d", count );
		}
	}

	if ( ! count ) 
	{
		addSysMessage( "You don't know any Spells" );
		return( false );
	}

	addItemMenu( item, count-1 );
	return( true );
}

bool CClient ::  Cmd_Skill_Alchemy()
{
	// Build up a potion type menu.
	// SKILL_ALCHEMY

	// Find bottles to put potions in.
	if ( ! m_pChar->ContentFind( ITEMID_EMPTY_BOTTLE ))
	{
		addSysMessage( "You don't have any bottles to put potions in" );
		return( false );
	}

	CMenuItem item[32];
	item[0].m_id = TARGMODE_MENU_SKILL;
	item[0].m_text = "Potion Formulas";

	int count = 0;
	for ( int k = 0; k < COUNTOF( Potions ); k ++ )
	{
		// Can we make this type ?
		// Have regs ?
		// Have near enough skill ?
		count ++;
		item[count].m_id  =	Potions[k].m_potion;
		item[count].m_text = Potions[k].m_name;
		item[count].m_context = k;
	}

	if ( ! count ) 
	{
		addSysMessage( "You can't make any potions" );
		return( false );
	}

	addItemMenu( item, count-1 );
	return( true );
}

bool CClient ::  Cmd_Skill_Blacksmith( int iLevel )
{
	// Select the blacksmith item type.
	// repair items or make type of items.
	// Must be near an anvil to repair and forge to make.


	return( false );
}

bool CClient ::  Cmd_Skill_Carpentry( int iLevel )
{	
	// Select the carpentry item type.
	// Check for wood as a raw material.
	// SKILL_CARPENTRY

	if ( ! m_pChar->ContentFind( ITEMID_LOGS ))
	{
		addSysMessage( "You don't have any wood" );
		return( false );
	}

	return( false );
}

bool CClient ::  Cmd_Skill_Tinker( int iLevel )
{
	// select the tinker item type.

	return( false );
}

bool CClient ::  Cmd_Skill_Bowcraft( int iLevel )
{
	// select the bow or arrow type.
	return( false );
}

bool CClient ::  Cmd_Skill_Cartography( int iLevel )
{
	// select the map type.

	if ( ! m_pChar->ContentFind( ITEMID_MAP_BLANK ))
	{
		addSysMessage( "You have no blank parchment to draw on" );
		return( false );
	}

	return( false );
}

void CClient :: Cmd_Skill_Heal( CItem * pBandages, CChar * pTarg )
{
	// ITEMID_BANDAGES:	// SKILL_HEALING, or SKILL_VETERINARY

	if ( ! m_pChar->Skill_UseQuick( SKILL_HEALING, 10 ))
		return;
}

void CClient :: Script_MenuCmd( const char * pCmd, const char * pArg ) // Execute command from script
{
	// Choice menus will launch scripts from here.
	if (! strcmp( "GMMENU", pCmd ))
	{
		addScriptMenu( atoi(pArg), TARGMODE_MENU_GM ); 
	}
	else if (! strcmp("ITEMMENU", pCmd))
	{
		// cascade to another menu.
		addScriptMenu( atoi(pArg), TARGMODE_MENU_ITEMS );
	}
	else if (! strcmp("WEBLINK", pCmd))
	{
		addWebLaunch( pArg);
	}
	else if (! strcmp("SYSMESSAGE", pCmd))
	{
		addSysMessage( pArg);
	}
	else if (! strcmp("GMPAGE", pCmd))
	{
		Cmd_GM_Page( pArg );
	}
	else if (! strcmp("VERSION", pCmd))
	{
		addSysMessage( Gray_szDesc);
	}
	else if (! strcmp("ADDITEM", pCmd))
	{
		Cmd_CreateItem( ahextoi(pArg));
	}
	else if (! strcmp("INFORMATION", pCmd))
	{
		addSysMessage( Gray_szDesc );
		addSysMessage( World.GetInformation());
	}
	else if (! strcmp("NPC", pCmd))
	{
		// Add a script NPC
		Cmd_CreateChar( - (int) ahextoi(pArg));
	}
	else if (! strcmp("POLY", pCmd))
	{
		// result of poly spell script choice.
		m_pChar->Spell_Polymorph( (CREID_TYPE) ahextoi(pArg) );
	}
	else if (! strcmp("COLOR", pCmd))
	{
		m_pChar->m_color = m_pChar->m_prev_color = ahextoi(pArg);
		m_pChar->Update();
	}
	else if (! strcmp("GOPLACE", pCmd))
	{
		m_pChar->Spell_Teleport( Script_GetPlace( atoi(pArg)));
	}
}

bool CClient :: Cmd_Pet( const char * pCmd, CChar * pChar )
{
	// We own this char (pet or hireling)
	// RETURN: true = requires targetting mode.

	m_Targ_UID = pChar->GetUID();

static const char * table[] =
{
	"TRANSFER",
	"KILL",
	"ATTACK",
	"FOLLOW",
	"GO",
	"FRIEND",
	"GUARD",
	"FETCH",
	"STOP",
	"STAY",
	"FOLLOW ME",
	"GUARD ME",
	"COME",
	"RELEASE",
	"SPEAK",
	"DROP",	// "GIVE" ?
};

	bool fTargetting = false;

	switch ( m_Targ_Index = FindTable( pCmd, table, COUNTOF(table)))
	{
	case 0:	// "TRANSFER"
		addTarget( TARGMODE_PET_CMD, "Who do you want to transfer to?", true );
		fTargetting = true;
		break;
	case 1:	// "KILL" 
	case 2: // "ATTACK"
		addTarget( TARGMODE_PET_CMD, "Who do you want to attack?", true );
		fTargetting = true;
		break;
	case 3:	// "FOLLOW"
	case 4:	// "GO"
	case 5:	// "FRIEND",
	case 6:	// "GUARD",
	case 7:	// "FETCH" 
		break;
	case 8:	// "STOP" 
	case 9: // "STAY"
		pChar->Skill_Setup( NPCACT_STAY );
		break;
	case 10: // "FOLLOW ME"
	case 11: // "GUARD ME"
	case 12: // "COME"
		pChar->Skill_Setup( NPCACT_FOLLOW_OWN );
		break;
	case 13: // "RELEASE"
		pChar->Skill_Setup( SKILL_NONE );
		pChar->m_owner.ClearUID();
		break;
	case 14:	// "SPEAK",
		break;
	case 15:	// "DROP",
		break;
	default:
		pChar->Hear( pCmd, m_pChar );
		return( false );
	}

	// make some sound to confirm we heard it.
	pChar->SoundChar( ( GetRandVal(2) ) ? CRESND_RAND1 : CRESND_RAND2 );
	return( fTargetting );
}

void CClient :: Cmd_SecureTrading( CChar * pChar, CItem * pItem )
{
	// Begin secure trading with a char. (MAke the initial offer)

	// Is there already a trade window open ?

	// addOpenGump( pBook, 0x866 );

}

/////////////////////////////////
// Events from the Client.

void CClient :: Event_Item_Dye() // Rehue an item
{
	// Result from addDyeOption()
	CObjUID uid( m_bin.DyeVat.m_UID  );
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) return;

	WORD color = m_bin.DyeVat.m_color ;

	if ( ! IsPriv( PRIV_GM ))
	{
		if ( ! pObj->IsItem()) 
			return;
		if ( color<0x0002 || color>0x03E9 )
			color = 0x03E9;
	}
	else
	{
		if ( ! pObj->IsItem()) 
		{
			pObj->Remove();
			color |= 0x8000;
		}
	}

	pObj->m_color = color;
	pObj->Update();
}

void CClient :: Event_Tips( int i) // Tip of the day window
{
	if (i==0) i=1;
	CString sSec;
	sSec.Format( "TIP %i", i );
	addScrollRead( sSec, 0, i );
}

void CClient :: Event_Book_Read( void ) // Book window
{
	CObjUID uid( m_bin.BookPage.m_UID );
	CItem * pItem = uid.ItemFind();	// the book.
	if ( pItem == NULL ) return;

	int page = m_bin.BookPage.m_page;	// page.

	CScript s;
	if ( ! s.Open( GRAY_FILE "book" GRAY_SCRIPT ))
		return;

	CString sSec;
	sSec.Format( "PAGE %d %d", pItem->m_bookID, page );
	if ( ! s.FindSec( sSec )) 
		return;

	// measure the page first.
	long pos=s.GetPos();
	int lines=0;
	int length=0;
	while (1)
	{
		if ( ! s.Read1()) break;
		lines++;
		length+=strlen(s.m_Line)+1;
	}

	CCommand cmd;
	length += sizeof( cmd.BookPage ) - sizeof( cmd.BookPage.m_text );

	// now send it.

	s.Seek( pos );

	cmd.BookPage.m_Cmd = 0x66;
	cmd.BookPage.m_len = length;
	cmd.BookPage.m_UID = pItem->GetUID();
	cmd.BookPage.m_pages = 1;
	cmd.BookPage.m_page = page;
	cmd.BookPage.m_lines = lines;

	int i=0;
	while ( lines-- )
	{
		if ( ! s.Read1()) break;
		strcpy( cmd.BookPage.m_text+i, s.m_Line );
		i += strlen(s.m_Line)+1;
	}

	xSendPkt( &cmd, length );
}

void CClient :: Event_Item_Get() // Client grabs an item
{
	CObjUID uid( m_bin.ItemGet.m_UID );

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Event_Item_Get %d\n", GetSocket(), uid ));
	}

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL ||
		(( pItem->m_Attr & ATTR_MOVE_NEVER ) && ! IsPriv(PRIV_ALLMOVE)) ||
		! m_pChar->CanTouch( pItem ))
	{
		addItemDragCancel(0);
		return;
	}

	if ( pItem->m_Attr & ATTR_OWNED )
	{
		addSysMessage( "That is not yours. You will have to steal the item" );
		addItemDragCancel(0);
		return;
	}

	addPause();
	m_Targ_Mode = TARGMODE_DRAG;
	m_Targ_UID = uid;
	m_Targ_Val = pItem->m_amount;

	if ( pItem->m_pDef->m_Can & CAN_PILE )
	{
		// Did we only pick up part of it ?
		m_Targ_Val = m_bin.ItemGet.m_amount;
	
		// part or all of a pile. Only if pilable !
		if ( m_Targ_Val > pItem->m_amount )
			m_Targ_Val = pItem->m_amount;
		if ( m_Targ_Val < pItem->m_amount )
		{
			// create left over item.
			CItem * pItemNew = ItemCreateDupe( pItem );
			pItemNew->m_amount = pItem->m_amount - m_Targ_Val;
			pItem->SetAmount( m_Targ_Val );
		}
	}

	// do the dragging anim for everyone else to see.
	m_pChar->UpdateDrag( pItem );

	// Pick it up.
	m_pChar->LayerAdd( pItem, LAYER_DRAGGING );
}

void CClient :: Event_Item_Drop() // Item is dropped 
{
	// This started from the Event_Item_Get()

	CObjUID uidItem( m_bin.ItemDrop.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CObjUID uidCont( m_bin.ItemDrop.m_UIDCont );	// dropped on this item.
	CObjBase * pObjCont = uidCont.ObjFind();
	CPoint  p( m_bin.ItemDrop.m_x, m_bin.ItemDrop.m_y, m_bin.ItemDrop.m_z );

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Event_Item_Drop %lx on %lx, x=%d, y=%d\n",
			GetSocket(), uidItem, uidCont, p.m_x, p.m_y ));
	}

	// Are we out of sync ?
	if ( pItem == NULL ||
		m_Targ_Mode != TARGMODE_DRAG ||
		pItem != m_pChar->LayerFind( LAYER_DRAGGING ))
	{
cantdrop:
		// The item was in the LAYER_DRAGGING.
		// Now where is it ??? Put it back where we got it from ?
		addItemDragCancel(5);
		return;
	}

	addPause();

	// What sound will it make ?
	WORD sound = 0x42;	// drop on ground sound

	if ( pObjCont != NULL )	// Put on or in another object
	{
		if ( ! pObjCont->IsItem())	// Drop on a char.
		{
			CChar * pChar = dynamic_cast <CChar*>( pObjCont );
			if ( pChar != m_pChar )
			{
				// Trade window if another PC.
				if ( pChar->m_pClient )
				{
					Cmd_SecureTrading( pChar, pItem );
				}
				else if ( ! pChar->ItemGive( m_pChar, pItem ))
					goto cantdrop;
				return;
			}

			// dropped on myself. Get my Pack.
			pObjCont = m_pChar->GetPack();
		}

		CItem * pItemCont = dynamic_cast <CItem*> ( pObjCont );

		if ( pItemCont->IsContainer())
		{
			CContainerItem * pCont = dynamic_cast <CContainerItem *>( pItemCont );
			if ( ! Can_Snoop_Container( pCont ))
				goto cantdrop;

			if (( pCont->m_Attr & ATTR_MAGIC ) && ! IsPriv( PRIV_GM ))
			{
				// Put stuff in a magic box
				// if ( pItemCont->m_item_spell == SPELL_BOX_RESTOCK )
				addSysMessage( "The item bounces out of the magic container" );
				goto cantdrop;
			}
			if ( pCont->GetCount() >= MAX_ITEMS_CONT-1 )
			{
				addSysMessage( "Too many items in that container" );
				goto cantdrop;
			}
			if ( ! pCont->IsEquipped() && 
				pItem->IsContainer() && 
				pItem->m_pDef->m_weight >= pCont->m_pDef->m_weight )
			{
				// is the container too small ?
				// can't put barrels in barrels.
				addSysMessage( "The container is too small for that" );
				goto cantdrop;
			}
		}
		else
		{
			// dropped on top of a non container.
			// can i pile them ?

			// Still in same container.
			pObjCont = pItemCont->GetContainer();
			if ( pItem->IsStackable( pItemCont ))
			{
				pItem->SetAmount( pItem->m_amount + pItemCont->m_amount );
				p = pItemCont->m_p;
				delete pItemCont;
			}
			else if ( pItemCont->IsSameID( ITEMID_SPELLBOOK ) || pItemCont->IsSameID( ITEMID_SPELLBOOK2 ))
			{
				if ( pItemCont->AddSpellbookScroll( pItem ))
				{
					addSysMessage( "Can't add this to the spellbook" );
				}
				goto cantdrop;
			}
			else if ( pItemCont->IsSameID( ITEMID_KEYRING ))
			{
				// Add keys to key ring.
				addSysMessage( "Can't do key rings yet" );
				goto cantdrop;
			}
			else
			{
				// Just drop on top of the current item.
				p = pItemCont->m_p;
			}
		}
		sound = 0x0057;	// add to inv sound.
	}

	if ( pItem->GetID() >= 0x0EEA && pItem->GetID() <= 0x0EF2 )	// ITEMID_GOLD
	{
		// depends on amount.
		switch ( pItem->m_amount )
		{
		case 1: sound = 0x035; break;
		case 2: sound = 0x032; break;
		case 3:
		case 4:	sound = 0x036; break;
		default: sound = 0x037; break;
		}
	}
	if ( pItem->m_type == ITEM_GEM )
	{
		// ??? gems have a sound as well.
	}

	// do the dragging anim for everyone else to see.

	if ( pObjCont != NULL )
	{
		// in pack or other CContainerItem.
		m_pChar->UpdateDrag( pItem, pObjCont );
		(dynamic_cast <CContainerItem *>(pObjCont))->ContentAdd( pItem, p );
		addSound( sound, m_pChar );
	}
	else
	{	// on ground
		m_pChar->UpdateDrag( pItem, NULL, &p );
		pItem->PutOnGround( p );
		pItem->Sound( sound );
	}
}

void CClient :: Event_Item_Equip() // Item is dropped on paperdoll
{
	// This started from the Event_Item_Get()

	CObjUID uidItem( m_bin.ItemEquip.m_UID );
	CItem * pItem = uidItem.ItemFind();
	CObjUID uidChar( m_bin.ItemEquip.m_UIDChar );
	CChar * pChar = uidChar.CharFind();

	if ( pItem == NULL ||
		pChar == NULL ||
		m_Targ_Mode != TARGMODE_DRAG ||
		m_bin.ItemEquip.m_layer >= LAYER_HORSE ||	// Can't equip this way.
		pItem != m_pChar->LayerFind( LAYER_DRAGGING ))
	{
cantequip:
		addItemDragCancel(5);
		return;
	}

	if ( pChar != m_pChar &&
		! IsPriv( PRIV_GM ) &&
		! ( pChar->m_owner == m_pChar->GetUID() ))
	{
		// trying to equip another char ?
		// can if he works for you. 
		// else just give it to him ?
		goto cantequip;
	}

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Item %xh equipped on layer %i.\n", GetSocket(), pItem->GetID(), m_bin.ItemEquip.m_layer ));
	}
	if ( ! pItem->IsSameID( ITEMID_BACKPACK2 ) && ! pItem->IsSameID( ITEMID_SPELLBOOK ))
	{
		ASSERT( m_bin.ItemEquip.m_layer == pItem->m_pDef->m_layer );
	}

	// ??? strong enough to equip this ?

	pChar->LayerAdd( pItem, (LAYER_TYPE) m_bin.ItemEquip.m_layer );
	addSound( 0x057, pChar );
}

void CClient :: Event_Skill_Use( SKILL_TYPE skill ) // Skill is clicked on the skill list
{
	// All the push button skills come through here.
	// Any "Last skill" macro comes here as well.

	if ( ! m_pChar->Skill_Setup( skill ))
		return;
	if ( Skills[skill].m_targetprompt != NULL )
	{
		// Go instantly into targtting mode.
		addTarget( TARGMODE_SKILL, Skills[skill].m_targetprompt, false );	// true
		return;
	}

	bool fContinue = false;

	// These skills need menus or other strange stuff.
	switch ( skill )
	{
	case SKILL_HIDING:
	case SKILL_SPIRITSPEAK:
	case SKILL_PEACEMAKING:
	case SKILL_DETECTINGHIDDEN:
		// These start/stop automatically.
		return;
	case SKILL_HERDING:
		// make this creature move to this point.
		addTarget( TARGMODE_SKILL_HERD_PICK, "What animal do you want to move?", true );
		return;
	case SKILL_TRACKING:
		fContinue = Cmd_Skill_Tracking();
		break;
	case SKILL_INSCRIPTION:
		// Menu select for spell type.
		fContinue = Cmd_Skill_Inscription( 0 );
		break;
	case SKILL_TINKERING:
		// What items do my skill and materials on hand allow me to make ?
		fContinue = Cmd_Skill_Tinker( 0 );
		break;
	case SKILL_ALCHEMY:	// Alchemy menus.
		// What items do my skill and materials on hand allow me to make ?
		fContinue = Cmd_Skill_Alchemy();
		break;
	case SKILL_BLACKSMITHING:
		fContinue = Cmd_Skill_Blacksmith(0);
		break;
	case SKILL_CARPENTRY:
		fContinue = Cmd_Skill_Carpentry(0);
		break;
	case SKILL_BOWCRAFT:
		fContinue = Cmd_Skill_Bowcraft(0);
		break;
	case SKILL_CARTOGRAPHY:
		fContinue = Cmd_Skill_Cartography(0);
		break;
	default:
		addSysMessage( "That skill has not been implemented yet.");
		break;
	}

	if ( ! fContinue )
	{
		m_pChar->Skill_Cleanup();	// cancel skill.
	}
}

void CClient :: Event_Walking() // Player moves
{
	if ( m_pChar->m_StatFlag & STATF_Freeze )
	{
		addMoveCancel();
		addSysMessage( "You are frozen and can not move.");
		return;
	}
	if ( m_pChar->m_stam <= 0 )
	{
		addMoveCancel();
		addSysMessage( "You are too fatigued to move.");
		return;
	}

	bool fRun = ( m_bin.Walk.m_dir & 0x80 );
	DIR_TYPE dir = (DIR_TYPE)( m_bin.Walk.m_dir & 0x0F );
	CPoint p = m_pChar->m_p;
	CPoint pold = p;
	bool fMove = true;

	if ( dir == m_pChar->m_dir )
	{
		// Move in this dir.
		p.Move( dir, 1 );

		// Check the z height here.
		// The client already knows this but doesn't tell us.
		if ( ! World.CheckValidMove( p ))
		{
			addMoveCancel();
			// addSysMessage( "ouchy" );
			return;
		}

		m_pChar->MoveTo( p );

		// Are we still invis ?
		if ( m_pChar->m_StatFlag & ( STATF_Invisible | STATF_Hidden | STATF_Sleeping ))
		{
			// ??? Wake up if sleeping and this is possible.
			// ??? check hiding again ? unless running ?
			m_pChar->m_StatFlag &=~ ( STATF_Invisible | STATF_Hidden | STATF_Sleeping );
			m_pChar->UpdateMode();
			return;
		}

		// did i step on a telepad, trap, etc ?
		if ( m_pChar->CheckLocation())
		{
			// We stepped on teleporter
			return;
		}
	}
	else
	{
		// Just a change in dir.
		m_pChar->m_dir = dir;
		fMove = false;
	}

	if ( Serv.m_fVerbose && m_bin.Walk.m_count != m_WalkCount+1 )
	{
		DEBUG_MSG(( "%x New Walk Count %d\n", GetSocket(), m_WalkCount ));
	}

	m_WalkCount = m_bin.Walk.m_count;

	// Ack the move. ( if this does not go back we get rubber banding )
	CCommand cmd;
	cmd.WalkAck.m_Cmd = 0x22;
	cmd.WalkAck.m_count = m_bin.Walk.m_count;
	cmd.WalkAck.m_unused = 0;	// seems to be junk ? 0x41
	xSendPkt( &cmd, sizeof( cmd.WalkAck ));

	if ( ! fMove )
	{
		// Show others I have turned !!
		m_pChar->UpdateMode( this );
		return;
	}

	if ( fRun && World.m_Clock_Time != World.m_Clock_PrevTime )
	{
		// Lower stamina if running.
		if ( m_pChar->m_stam > 0 ) m_pChar->m_stam --;
		m_pChar->UpdateStats( STAT_DEX );
	}

	// Who now sees me ?
	m_pChar->UpdateMove( pold, this );
	// What do I now see ?
	addPlayerSee( pold );
}

void CClient :: Event_CombatMode( bool fWar ) // Only for switching to combat mode
{
	// If peacmaking then this doens't work ??
	// Say "you are feeling to peacefull"
    
	if ( fWar )
		m_pChar->m_StatFlag |= STATF_War;
	else
		m_pChar->m_StatFlag &= ~STATF_War;

	if ( m_pChar->m_StatFlag & STATF_DEAD )
	{
		// Manifest the ghost.
		// War mode for ghosts.
		if ( fWar )
			m_pChar->m_StatFlag &= ~STATF_Invisible;
		else
			m_pChar->m_StatFlag |= STATF_Invisible;
	}

	m_pChar->m_Act_Targ.ClearUID();	// no target yet
    addPlayerWarMode();
	m_pChar->UpdateMode( this );
}

void CClient :: Event_Choice() // Choice from GMMenu or Itemmenu received
{
	// ??? Select from a menu. CMenuItem

	WORD menuid = m_bin.Choice.m_menuid;
	WORD select = m_bin.Choice.m_select;

	if ( menuid == TARGMODE_MENU_SKILL )
	{
		// Some skill menu got us here.
		//
		m_pChar->Skill_Done();
		return;
	}

	CScript s;
	if ( ! s.Open( GRAY_FILE "menu" GRAY_SCRIPT ))
		return;

	CString sSec;
	if ( menuid < TARGMODE_MENU_ITEMS ) // GM Menus
	{
		sSec.Format( "GMMENU %i", menuid-TARGMODE_MENU_GM);
	}
	else
	{
		sSec.Format( "ITEMMENU %i", menuid-TARGMODE_MENU_ITEMS);
	}
	if ( ! s.FindSec( sSec )) 
	{
		return;
	}
	s.Read1();

	int i=0;
	while ( 1 )
	{
		if ( ! s.Read1()) break;
		i++;
		if ( ! s.ReadParse()) break;
		if ( i == select )
		{
			Script_MenuCmd( s.m_Line, s.m_pArg );
			return;
		}
	}
}

void CClient :: Event_Command() // Client entered a '/' command like /ADD
{
#define Arg_num(n)		atoi(Arg_ppCmd[(n)])
#define Arg_hex_num(n)	ahextoi(Arg_ppCmd[(n)])

	// Get a spoken command.
	// Copy it to temp buffer.

	char Arg_Cmd[256];
	strncpy( Arg_Cmd, &m_bin.Talk.m_text[1], sizeof(Arg_Cmd));
	Arg_Cmd[sizeof(Arg_Cmd)-1] = '\0';
	if ( Arg_Cmd[0] == '\0' || Arg_Cmd[1] == ' ' ) return;
	strupr( Arg_Cmd );

	char *Arg_ppCmd[16];		// Maximum parameters in one line 
	int Arg_Qty = ParseCmds( Arg_Cmd, Arg_ppCmd, COUNTOF( Arg_ppCmd ));
	
	// Administrator level = debug and server security type stuff.

	if ( IsPriv( PRIV_Administrator )) 
	{

static const char * szCmd_Administrator[] =	// Designer only commands
{
	"SETPRIV",
	"SAVE",
	"SHUTDOWN",
	"DEBUG",
	"IMPORT",
	"GUMPMENU",
	"SEND",
	"BOLT",
};

		switch ( FindTable( Arg_Cmd, szCmd_Administrator, COUNTOF(szCmd_Administrator)))
		{
		case 0:	// "SETPRIV"
			if (Arg_Qty>=2)
			{
				m_Targ_Text = &m_bin.Talk.m_text[9]; // Point at the options, if any
				addTarget( TARGMODE_SET_PRIV, "Select character to change privelege.", true );
				return;
			}
			break;
		case 1: // SAVE
			World.Save();
			return;
		case 2:	// SHUTDOWN
			Serv.Shutdown( (Arg_Qty>=2) ? Arg_num(1) : (10*60*60) );
			return;
		case 3:	// DEBUG
			{
				m_Priv ^= PRIV_DEBUG;
				CString sTmp;
				sTmp.Format( "DEBUG %s", ( m_Priv & PRIV_DEBUG ) ? "ON" : "OFF" );
				addSysMessage( sTmp );
				addReSync();
			}
			return;
		case 4: // IMPORT name	
			if (Arg_Qty>=2)
			{
				if ( ! World.Import( Arg_ppCmd[1] ))
					addSysMessage( "Import failed" );
				addReSync();
				return;
			}
			break;
		case 5:	// GUMPMENU	// ??? Tweak command !?
			addGumpMenu( (Arg_Qty>=2) ? Arg_num(1) : 1 );
			return;
		case 6:	// SEND
			if (Arg_Qty>=2)
			{
				char * pTemp = GetTempStr();
				for ( int i=1;i<Arg_Qty;i++) pTemp[i-1]=Arg_hex_num(i);
				Log.Event( "Sending %d bytes to client.\n", Arg_Qty-1 );
				xSend( pTemp, Arg_Qty-1);
				return;
			}
			break;
		case 7:	// "BOLT" type id speed loop explode
			m_Targ_Text = (Arg_Qty>=2) ? &m_bin.Talk.m_text[6] : "";
			addTarget( TARGMODE_TEST_BOLT, "Select target", false );
			return;
		}
	}

	// GM = full world building capability and power over all chars.
	if ( IsPriv( PRIV_GM|PRIV_Administrator ))
	{
static const char * szCmd_GM[] =	// GM only commands
{
	"XGO",		// x y (z) = Send char to "x y z" coords
	"ADD",		// id = add an item or get a menu if no args
	"REMOVE",
	"SETZ",
	"SET",		// modify any attribute.
	"KILL",
	"KICK",
	"SFX",
	"LIGHT",
	"DRY",
	"RAIN",
	"SNOW",
	"COMMAND",
	"ALLMOVE",
	"ADDNPC",
	"FLIP",
	"DUPE",
	"BANK",
	"RESTOCK",
	"CONTROL",	// Possess
	"LINK",
	"CLIENTS",
	"SHRINK",
	"INVULNERABLE",
	// "HOME",		// Set this creatures home location.
};

		switch ( FindTable( Arg_Cmd, szCmd_GM, COUNTOF(szCmd_GM)))
		{
		case 0:	// XGO x y (z) 
			if (Arg_Qty>=3)
			{
				CPoint p( Arg_num(1), Arg_num(2), Arg_num(3));
				if ( p.IsValid())
				{
					m_pChar->m_Act_p = p;
					Event_Skill_Use( SKILL_HERDING );
					return;
				}
			}
			break;
		case 1:	// ADD
			// Create any item from script.
			if (Arg_Qty==2)
			{
				Cmd_CreateItem( Arg_hex_num(1));
				return;
			}
			addScriptMenu( 1, TARGMODE_MENU_ITEMS );
			return;

		case 2:	// REMOVE
			addTarget( TARGMODE_REMOVE, "Select item to remove.", true );
			return;
		case 3:	// SETZ
			m_Targ_Val = (Arg_Qty>=2) ? Arg_num(1) : 0xFFFF;
			addTarget( TARGMODE_SET_Z, "Select item to reposition.", true );
			return;
		case 4:	// "SET" = set some attribute.
			if ( Arg_Qty>=2 )
			{
				m_Targ_Text = &m_bin.Talk.m_text[5];
				addTarget( TARGMODE_SET, "Select object to set attributes of", true );
				return;
			}
			break;

		case 5:	// KILL
			addTarget( TARGMODE_KILL, "Select character to kill.", true );
			return;

		case 6:	// KICK
			addTarget( TARGMODE_KICK, "Select character to kick.", true );
			return;

		case 7:	// SFX
			if (Arg_Qty==2)
			{
				m_pChar->Sound( Arg_hex_num(1));
				return;
			}
			break;

		case 8:	// LIGHT
			World.SetLight( (Arg_Qty==2) ? Arg_hex_num(1) : 0 );
			return;

		case 9:		// DRY
			m_pChar->m_p.GetQuadrant()->SetWeather( 0 );
			return;

		case 10:	// RAIN
			m_pChar->m_p.GetQuadrant()->SetWeather( 1 );
			return;

		case 11:	// SNOW
			m_pChar->m_p.GetQuadrant()->SetWeather( 2 );
			return;

		case 12:	// COMMAND x args 
			if (Arg_Qty>1)
			{
				Script_MenuCmd( Arg_ppCmd[1], Arg_ppCmd[2] );
				return;
			}
			break;

		case 13:	// ALLMOVE
			{
				m_Priv ^= PRIV_ALLMOVE;
				CString sTmp;
				sTmp.Format( "ALLMOVE %s", ( m_Priv & PRIV_ALLMOVE ) ? "ON" : "OFF" );
				addSysMessage( sTmp );
				addReSync();
			}
			return;

		case 14:	// ADDNPC
			if (Arg_Qty==2)
			{
				Cmd_CreateChar( - (int) Arg_hex_num(1));
				return;
			}
			break;

		case 15:	// "FLIP"
			m_Targ_Text = (Arg_Qty>=2)?&m_bin.Talk.m_text[6]:""; // Point at the options, if any
			addTarget( TARGMODE_FLIP, "What item would you like to flip?", true );
			return;

		case 16:	// "DUPE"
			addTarget( TARGMODE_DUPE, "What item would you like to dupe?", true );
			return;

		case 17:	// "BANK"
			m_Targ_Index = (Arg_Qty>1) ? Arg_hex_num(1) : LAYER_BANKBOX;
			addTarget( TARGMODE_BANK, "Whose bank do you want to open?", true );
			return;

		case 18:	// "RESTOCK" x
			{
			// set restock time of all vendors in quadrant.
			int iTime = (Arg_Qty>1) ? Arg_num(1) : (15*60);
			CChar * pChar = static_cast <CChar*>( m_pChar->m_p.GetQuadrant()->m_Chars.GetHead() );
			for ( ; pChar != NULL; pChar = pChar->GetNext())
			{
				if ( pChar->m_NPC_Brain != NPCBRAIN_VENDOR )
					continue;
				CContainerItem * pCont = dynamic_cast <CContainerItem *>( pChar->LayerFind( LAYER_VENDOR_STOCK ));
				if ( pCont == NULL ) 
					continue;
				pCont->SetTimeout( iTime );
			}
			return;
			}

		case 19:	// "CONTROL",	// Possess
			addTarget( TARGMODE_CONTROL, "Who do you want to control?", true );
			return;
		case 20:	// "LINK" = link doors
			m_Targ_UID.ClearUID();
			addTarget( TARGMODE_DOOR_LINK, "Select the door to link.", true );
			return;
		case 21:	// "CLIENTS"
			Serv.ListClients( this );
			return;
		case 22:	// "SHRINK"
			addTarget( TARGMODE_SHRINK, "Select the creature to shrink", true );
			return;
		case 23:	// "INVULNERABLE
			{
				m_pChar->m_StatFlag ^= STATF_INVUL;
				CString sTmp;
				sTmp.Format( "Invulnerability %s", ( m_pChar->m_StatFlag & STATF_INVUL ) ? "ON" : "OFF" );
				addSysMessage( sTmp );
			}
			return;
		}
	}

	// Counselor commands.

	if ( IsPriv( PRIV_COUNSEL|PRIV_GM|PRIV_Administrator ))
	{

static const char * szCmd_Couns[] =	// Counselor commands
{
	"ACTION",
	"ANIM",
	"GO",
	"GOUID",
	"GOCHAR",
	"GOSOCK",
	"GOCLI",
	"XGO",
	"JAIL",
};

		switch ( FindTable( Arg_Cmd, szCmd_Couns, COUNTOF(szCmd_Couns)))
		{
		case 0:	// ACTION
		case 1:	// "ANIM"
			if (Arg_Qty==2)
			{
				m_pChar->UpdateAnimate( (ANIM_TYPE) Arg_hex_num(1), false );
				return;
			}
			break;
		case 2:	// GO x y z	
			if (Arg_Qty==2) // GO n
			{
				m_pChar->Spell_Teleport( Script_GetPlace( Arg_num(1)));
				return;
			}
			else if (Arg_Qty>=3)	// GO x y (z)
			{
				CPoint p( Arg_num(1), Arg_num(2), Arg_num(3) );
				m_pChar->Spell_Teleport( p );
				return;
			}
			break;
		case 3:	// "GOUID" uid
			if (Arg_Qty==2)
			{
				CObjUID uid( Arg_hex_num(1) );
				CObjBase * pObj = uid.ObjFind();
				if ( pObj != NULL )
				{
					m_pChar->Spell_Teleport( pObj->GetTopLevelObj()->m_p );
					return;
				}
				return;
			}
			break;
		case 4:	// "GOCHAR" uid
			if (Arg_Qty==2)
			{
				int j = Arg_hex_num(1);
				for ( int i=0; i<QUAD_QTY; i++ )
				{
					CChar * pChar = static_cast <CChar*>( World.m_Quads[i].m_Chars.GetHead() );
					for ( ; pChar!=NULL; pChar = pChar->GetNext())
					{
						if ( ! j-- ) 
						{
							m_pChar->Spell_Teleport( pChar->m_p );
							return;
						}
					}
				}
			}
			break;
		case 5:	// GOSOCK sockid
			if (Arg_Qty==2)
			{
				int i = Arg_hex_num(1);
				for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
				{
					if ( pClient->GetSocket() != i ) continue;
					if ( pClient->m_pChar == NULL ) continue;
					m_pChar->Spell_Teleport( pClient->m_pChar->m_p );
					return;
				}
			}
			break;
		case 6:	// GOCLI enum clients
			if (Arg_Qty==2)
			{
				int i = Arg_hex_num(1);
				for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
				{
					if ( pClient->m_pChar == NULL ) continue;
					if ( i-- ) continue;
					m_pChar->Spell_Teleport( pClient->m_pChar->m_p );
					return;
				}
			}
			break;
		case 7:	// XGO n
		case 8:
			// default to jail
			m_pChar->m_Act_p = Script_GetPlace( (Arg_Qty>=2) ? Arg_num(1) : 6 );
			Event_Skill_Use( SKILL_HERDING );
			return;
		}
	}

	// Anyone can use these.
static const char * szCmd_Gen[] =
{
	"ADD",
	"FIX",
	"WHERE",
	"RESEND",
	"SYNC",
	"RESYNC",
	"MUSIC",
	"CAST",
};
	switch ( FindTable( Arg_Cmd, szCmd_Gen, COUNTOF(szCmd_Gen)))
	{
	case 0:	// ADD
		addScriptMenu( 0, TARGMODE_MENU_ITEMS );
		return;
	case 1:	// FIX
		{
			bool fBlock;
			m_pChar->m_p.m_z = (Arg_Qty>=2) ? Arg_num(1) : World.GetHeight( m_pChar->m_p, fBlock );
			m_pChar->Update();
		}
		return;
	case 2:	// WHERE
		{
			m_pChar->Skill_UseQuick( SKILL_CARTOGRAPHY, 10 );
			CString sTmp;
			sTmp.Format( "I am at %i %i (%i)",
				m_pChar->m_p.m_x, m_pChar->m_p.m_y, m_pChar->m_p.m_z );
			addItemMessage(sTmp,m_pChar);
		}
		return;
	case 3:
	case 4:
	case 5: // "RESEND"
		addReSync();
		return;
	case 6:	// "MUSIC"
		{
			// Set music for the region ?
			WORD wMusic = (Arg_Qty>=2) ? Arg_num(1) : 0;
			for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
			{
				pClient->addMusic( wMusic );
			}
		}
		return;

	case 7: // "CAST"
		if (Arg_Qty==2)
		{
			Cmd_Skill_Magery( (SPELL_TYPE) Arg_num(1), m_pChar );
		}
		return;
	}

	addSysMessage( "Not a valid command or format" );
}

void CClient :: Event_Attack()
{
	// d-click in war mode 
	// I am attacking someone.
		
	CObjUID uid( m_bin.Click.m_UID );
	CChar * pChar = uid.CharFind();
	if ( pChar == NULL ) return;
	if ( ! m_pChar->Attack( pChar )) return;	// decline the attack ???

	CCommand cmd;
	cmd.AttackOK.m_Cmd = 0xaa;
	cmd.AttackOK.m_UID = pChar->GetUID();
	xSendPkt( &cmd, sizeof( cmd.AttackOK ));
}

void CClient :: Event_VendorBuy()
{
	// Client wants to Buy item from vendor.	

	CObjUID uidVendor( m_bin.VendorBuy.m_UIDVendor );
	CChar * pVendor = uidVendor.CharFind();
	if ( pVendor == NULL ) return;
	if ( m_bin.VendorBuy.m_flag == 0 ) return;

	// Calculate the total cost of goods.
	int costtotal=0;
	bool fSoldout = false;
	int nItems = (m_bin.VendorBuy.m_len - 8) / sizeof( m_bin.VendorBuy.items[0] );
	int i=0;
	for ( ;i<nItems;i++)
	{
		CObjUID uid( m_bin.VendorBuy.items[i].m_UID );
		CItem * pItem = uid.ItemFind();
		if ( pItem == NULL ) continue;	// ignore it for now.
		if ( m_bin.VendorBuy.items[i].m_amount > pItem->m_amount )
		{
			fSoldout = true;
			continue;
		}
		costtotal += m_bin.VendorBuy.items[i].m_amount * pItem->GetPrice();
	}

	if ( fSoldout )
	{
		pVendor->Speak( "Alas, I no longer have all those goods in stock.  Let me know if there is something else thou wouldst buy." );
		// ??? Update the vendors list and try again.
		return;
	}
	if ( m_pChar->ContentConsume( ITEMID_GOLD, costtotal, true ))
	{
		pVendor->Speak( "Alas, thou dost not possess sufficient gold for this purchase!" );
		return;
	}
	if ( costtotal <= 0 )
	{
		pVendor->Speak( "You have bought nothing. But feel free to browse" );
		return;
	}

	CString sMsg;
	sMsg.Format( 
		"Here you are, %s."
		"That will be %d gold coin%s."
		"I thank thee for thy business.", 
		m_pChar->GetName(), costtotal, (costtotal==1) ? "" : "s" );
	pVendor->Speak( sMsg );

	// Take the gold.
	m_pChar->ContentConsume( ITEMID_GOLD, costtotal );

	CContainerItem * pPack = m_pChar->GetPack();

	// Move the items bought into your pack.
	for ( i=0;i<nItems;i++)
	{
		CObjUID uid( m_bin.VendorBuy.items[i].m_UID );
		CItem * pItem = uid.ItemFind();
		if ( pItem == NULL ) continue;	// ignore it i guess.

		WORD amount = m_bin.VendorBuy.items[i].m_amount;
		pItem->SetAmount( pItem->m_amount - amount );

		if ( amount > 1 && ! ( pItem->m_pDef->m_Can & CAN_PILE ))
		{
			while ( amount -- )
			{
				pPack->ContentAdd( ItemCreateDupe( pItem ));
			}
		}
		else 
		{
			CItem * pItemNew = ItemCreateDupe( pItem );
			pItemNew->SetAmount( amount );
			pPack->ContentAdd( pItemNew );
		}

		if ( pItem->m_amount == 0 &&
			m_bin.VendorBuy.items[i].m_layer == LAYER_VENDOR_EXTRA )
		{
			// we can buy it all.
			// not allowed to delete all from LAYER_VENDOR_STOCK
			delete pItem;
		}
		else 
		{
			pItem->Update();
		}
	}

	// Clear the vendor display.
	addVendorClose(pVendor);
}

void CClient :: Event_VendorSell()
{
	// Selling items to the vendor.
	// Done with the selling action.

	CObjUID uidVendor( m_bin.VendorSell.m_UIDVendor );
	CChar * pVendor = uidVendor.CharFind();
	if ( pVendor == NULL ) return;

	if ( ! m_bin.VendorSell.m_count )
	{
		addVendorClose( pVendor );
		// pVendor->Speak( "You have sold nothing" );
		return;
	}

	CContainerItem * pContStock = dynamic_cast <CContainerItem *>( pVendor->LayerFind( LAYER_VENDOR_STOCK ));
	CContainerItem * pContExtra = dynamic_cast <CContainerItem *>( pVendor->LayerFind( LAYER_VENDOR_EXTRA ));
	if ( pContStock == NULL || pContExtra == NULL )
	{
		addVendorClose( pVendor );
		pVendor->Speak( "Ahh, Guards my goods are gone !!" );
		return;
	}

	// Can the vendor afford this ???

	int iGold = 0;
	for ( int i=0; i<m_bin.VendorSell.m_count; i++ )
	{
		CObjUID uid( m_bin.VendorSell.items[i].m_UID );
		CItem * pItem = uid.ItemFind();
		if ( pItem == NULL ) continue;
		int amount = m_bin.VendorSell.items[i].m_amount;
		if ( pItem->m_amount < amount )
		{
			amount = pItem->m_amount;
		}

		// give them the appropriate amount of gold.
		iGold += pItem->GetPrice() * amount;

		// Take the items from player.
		// Put items in vendor inventory.
		if ( amount >= pItem->m_amount )
		{
			// Transfer all.
			pContExtra->ContentAdd( pItem );
		}
		else
		{
			// Just part of the stack.
			CItem * pItemNew = ItemCreateDupe( pItem );
			pItemNew->SetAmount( amount );
			pContExtra->ContentAdd( pItemNew );
			pItem->SetAmount( pItem->m_amount - amount );
		}
	}

	if ( iGold )
	{
		CString sMsg;
		sMsg.Format( "Here you are, %d gold coin%s."
			"I thank thee for thy business.", 
			iGold, (iGold==1) ? "" : "s" );
		pVendor->Speak( sMsg );

		CItem * pGold = ItemCreateScript( ITEMID_GOLD );
		pGold->SetAmount( iGold );
		m_pChar->GetPack()->ContentAdd( pGold );
	}

	addVendorClose( pVendor );
}

void CClient :: Event_SecureTrade()
{
	// perform the trade.
	switch ( m_bin.SecureTrade.m_action )
	{
	case 0: // Start trade - Never happens, sent out by the server only.
		break;
	case 1: // Cancel trade.  Send each person cancel messages, move items.
		// TradeEnd( m_bin.SecureTrade.m_UID );
		break;
	case 2: // Change check marks.  Possibly conclude trade

#ifdef COMMENT
		cont1=calcItemFromSer(m_bin.SecureTrade.m_UID);
		cont2=calcItemFromSer(items[cont1].moreb1, items[cont1].moreb2, items[cont1].moreb3, items[cont1].moreb4);
		items[cont1].morez=buffer[s][11];
		sendtradestatus(cont1, cont2);
		if (items[cont1].morez && items[cont2].morez)
		{
			dotrade(cont1, cont2);
			TradeEnd( m_bin.SecureTrade.m_UID);
		}
#endif
		break;
	}
}

void CClient :: Event_ExtCmd()
{
	int i = 0;
	// parse the args.
	for ( i = 0; m_bin.ExtCmd.m_name[i] != '\0' && m_bin.ExtCmd.m_name[i] != ' '; i++ )
		;
	m_bin.ExtCmd.m_name[i] = '\0';

	switch ( m_bin.ExtCmd.m_type )
	{

	case 0x43: // 67 = open spell book if we have one.
		{
			CItem * pBook = m_pChar->ContentFind( ITEMID_SPELLBOOK );
			if ( pBook == NULL )
			{
				pBook = m_pChar->ContentFind( ITEMID_SPELLBOOK2 );
				if ( pBook == NULL )
				{
					// Sorry no spellbook found.
					break;
				}
			}
			addItemCont( pBook );
			addSpellbookOpen( pBook );
		}
		break;

	case 0xC7: // Cmd_Animate
		if ( !strcmp( m_bin.ExtCmd.m_name,"bow"))
			m_pChar->UpdateAnimate( ANIM_BOW );
		else if ( ! strcmp( m_bin.ExtCmd.m_name,"salute")) 
			m_pChar->UpdateAnimate( ANIM_SALUTE );
		else 
		{
			DEBUG_ERR(( "ERROR %x:Event Animate '%s'\n", GetSocket(), m_bin.ExtCmd.m_name ));
		}
		break;

	case 0x24:			// Skill
		Event_Skill_Use( (SKILL_TYPE) atoi( m_bin.ExtCmd.m_name ));
		break;

	case 86:	// macro spell.
	case 39:	// cast spell from book.
		Cmd_Skill_Magery( (SPELL_TYPE) atoi( m_bin.ExtCmd.m_name ), m_pChar );
		break;

	case 88: // open door macro ?
		// break;

	default:
		DEBUG_ERR(( "ERR %x:Event Animate unk %d, '%s'\n", GetSocket(), m_bin.ExtCmd.m_type, m_bin.ExtCmd.m_name ));
	}
}

void CClient :: Event_Talk() // PC speech
{
	// Rip out the unprintables first.
	char szText[256];
	int j=0;
	int i = 0;
	for ( i = 0; 1; i++ )
	{
		char ch = m_bin.Talk.m_text[i];
		if ( ch == '|' ) continue;
		if ( ch == '~' ) continue;
		if ( j >= sizeof( szText )-1 )
		{
			ch = 0;
		}
		szText[j++] = ch;
		if ( ch == 0 ) break;
	}

	const char * pszName;
	const char * pszPrefix;
	switch ( m_Targ_Mode )
	{
	case TARGMODE_NAME_RUNE:
		pszName = "Rune";
		pszPrefix = "Rune to:";
		goto setname;

	case TARGMODE_NAME_KEY:
		pszName = "Key";
		pszPrefix = "Key to:";
		goto setname;

	case TARGMODE_NAME_SIGN:
		pszName = "Sign";
		pszPrefix = "";
		goto setname;

setname:
		{
			m_Targ_Mode = TARGMODE_NONE;
			CString sMsg;
			CItem * pItem = m_Targ_UID.ItemFind();
			if ( pItem == NULL || szText[0] == '\0' )
			{
				sMsg.Format( "%s Renaming Canceled" );
			}
			else
			{
				sMsg.Format( "%s%s", pszPrefix, szText );
				pItem->SetName( sMsg );
				sMsg.Format( "%s renamed: %s", pszName, pItem->GetName());
			}
			addSysMessage( sMsg );
		}
		return;
	}

	if ( szText[0] == '\0' ) return;
	if ( szText[0] == '/' ) 
	{
		Event_Command();
		return;
	}

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Say'%s' mode %d\n", GetSocket(), szText, m_bin.Talk.m_mode ));
	}

	m_pChar->Speak( szText, m_bin.Talk.m_color, (TALKMODE_TYPE) m_bin.Talk.m_mode );

	// ??? Allow NPC's to talk to each other in the future.
	// Do hearing here so there is not feedback loop with NPC's talking to each other.

	strupr( szText );

	// Find an NPC that may have heard us.
	CChar * pCharAlt = NULL;
	int iAltDist = UO_MAP_VIEW_SIZE;

	CChar * pChar;
	CWorldSearch AreaChars( m_pChar->m_p, UO_MAP_VIEW_SIZE );
	while ( 1 )
	{
		pChar = AreaChars.GetChar();
		if ( pChar == NULL ) break;
		if ( pChar == m_pChar ) continue;
		int iDist = m_pChar->GetDist( pChar );

		if ( pChar->m_Act_Targ == m_pChar->GetUID()) // already talking to him
		{
			pCharAlt = pChar;
			iAltDist = 0;
		}
		if ( iDist < iAltDist )	// closest guy ?
		{
			pCharAlt = pChar;
			iAltDist = iDist;
		}
		// Named the char ?
		const char * pName = pChar->GetName();
		for ( i=0; pName[i] != '\0' && szText[i] != '\0'; i++ )
		{
			if ( toupper( pName[i] ) != szText[i] )
				break;
		}
		if ( pName[i] == '\0' )		// Found name !
		{
			// remove name.
			while ( szText[i] == ' ' ) i ++;
			break;	// Char name
		}

		// NPC's with special key words ?
		if ( pChar->m_NPC_Brain == NPCBRAIN_BANKER )
		{
			if ( FindStrWord( szText, "BANK" ))
				break;
		}
		else if ( pChar->m_NPC_Brain == NPCBRAIN_GUARD )
		{
			if ( FindStrWord( szText, "GUARD" ))
				break;
		}
		else if ( pChar->m_NPC_Brain == NPCBRAIN_VENDOR )
		{
			if ( FindStrWord( szText, "VENDOR" ))
				break;
		}
	}

	if ( pChar == NULL )
	{
		i = 0;
		pChar = pCharAlt;
		if ( pChar == NULL ) return;	// no one heard it.
	}

	char * pCmd = &szText[i];

	if ( pChar->m_owner == m_pChar->GetUID())
	{
		Cmd_Pet( pCmd, pChar );
	}
	else
	{
		// The char hears you say this.
		pChar->Hear( pCmd, m_pChar );
	}
}

bool CClient :: Event_DeathOption()
{
	if ( m_bin.DeathOpt.m_mode )
	{
		if ( m_bin.DeathOpt.m_mode == 1 ) // res w/penalties ?
		{
			// 
			m_pChar->Spell_Effect( SPELL_Resurrection, NULL, 100 );
			// ??? Clear the menu !!! Not sure how.
			//  "The connection between your sirit and the world is too weak."
			//	"Thou hast drained thyself in thy efforts to reassert thine lifeforce in the physical world."
			//  "No matter how strong your efforts, you cannot reestablish contact with the physical world."
			addOptions();
		}
		else
		{
			// Play as a ghost.
			// "As the mortal pains fade, you become aware of yourself as a spirit."
			addSysMessage( "You are a ghost" );
			addSound( 0x17f, m_pChar );	// Creepy noise.
		}
		return( true );
	}
	if ( ! xCheckSize( sizeof( m_bin.DeathOpt ))) return(false);
	Event_CombatMode( m_bin.DeathOpt.m_manifest );
	return( true );
}

void CClient :: Event_SetName()
{
	// ??? Just assume they have the right to do so ?
	CObjUID uid( m_bin.CharName.m_UID );
	CChar * pChar = uid.CharFind();
	if ( pChar != NULL )
	{
		pChar->SetName( m_bin.CharName.m_name );
	}
}

void CClient :: Event_SingleClick()
{
	CObjUID uid( m_bin.Click.m_UID );

	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Event_SingleClick %lx\n", GetSocket(), (DWORD) uid ));
	}

	CChar * pChar = uid.CharFind();
	if ( pChar != NULL ) 
	{
		addCharName( pChar );
		return;
	}

	CItem * pItem = uid.ItemFind();
	if ( pItem == NULL ) return;

	char * pTemp = GetTempStr();
	int len = 0;

	if ( pItem->m_Attr & ATTR_MAGIC )
	{
		len += sprintf( pTemp+len, "Magic " );
	}
	if ( pItem->m_amount==1 || ! ( pItem->m_pDef->m_Can & CAN_PILE ))
	{
		len += sprintf( pTemp+len, pItem->GetName());
	}
	else
	{
		len += sprintf( pTemp+len, "%i %ss", pItem->m_amount, pItem->GetName() );
	}

	if ( pItem->IsContainer())
	{
		len += sprintf( pTemp+len, " (%d items)", pItem->GetThisContainer()->GetCount());
	}
	if ( IsPriv( PRIV_GM ))
	{
		// Show the restock count ???
		// If GetContainer()-> RESTOCK
		// len += sprintf( pTemp+len, " (%d restock)", pItem->m_p.m_z );
	}
	if ( IsPriv( PRIV_SHOWUID ))
	{
		len += sprintf( pTemp+len, " [%lx]", uid );
	}

	addItemMessage( pTemp, pItem );
}

void CClient :: Event_DoubleClick()
{
	// Try to use the object in some way.

	DWORD dwUID = m_bin.Click.m_UID;
	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Event_DoubleClick %lxh\n", GetSocket(), dwUID ));
	}

	bool fMacro = ( dwUID & UID_SPEC ); // ALTP vs dbl click. no unmount.
	CObjUID uid( dwUID & ( UID_MASK | UID_ITEM ));
	CObjBase * pObj = uid.ObjFind();
	if ( pObj == NULL ) return;

	if ( ! m_pChar->CanTouch( pObj ))
	{
		if ( m_pChar->m_StatFlag & STATF_DEAD )
		{
			addSysMessage( "Your ghostly hand passes through the object." );
		}
		else
		{
			addSysMessage( "You can't reach that." );
		}
		return;
	}

	if ( ! pObj->IsItem() )
	{
		CChar * pChar = (CChar*) pObj;
		if ( pChar->GetID() == CREID_BLADES )
		{
			return;
		}
		if ( pChar == m_pChar && ! fMacro )
		{
			if ( m_pChar->Horse_UnMount())
				return;
		}
		if ( pChar->GetID() < CREID_MAN && pChar->m_pClient == NULL )
		{
			switch ( pChar->GetID() )
			{
			case CREID_HORSE1:
			case CREID_HORSE2:
			case CREID_HORSE3:
			case CREID_HORSE4:
				if ( m_pChar->Horse_Mount( pChar ))
					return;
				return;

			case CREID_HORSE_PACK:
			case CREID_LLAMA_PACK:
				{
					// pack animals open continer.
					// if it is yours ??? else it is snooping ?
					addContainerSetup( pChar->GetPack());
				}
				return;
			default:
				return;
			}
		}

		// open paper doll.
		CCommand cmd;
		cmd.PaperDoll.m_Cmd = 0x88;
		cmd.PaperDoll.m_UID = pChar->GetUID();
		cmd.PaperDoll.m_mode = pChar->GetModeFlag();	// 0=normal, 0x40 = attack

		if ( pChar->m_StatFlag & STATF_Incognito )
		{
			strcpy( cmd.PaperDoll.m_text, pChar->GetName());
		}
		else
		{
			sprintf( cmd.PaperDoll.m_text, "%s, %s", pChar->GetNotoTitle(), pChar->GetTradeTitle());
		}
		xSendPkt( &cmd, sizeof( cmd.PaperDoll ));
		return;
	}

	CItem * pItem = (CItem *) pObj;
	m_Targ_UID = uid;
	m_pChar->UpdateDir( pItem );

	// unique activated items.
	switch ( pItem->GetID() )
	{
	case ITEMID_Bulletin:
		addSysMessage( "Bulletins not working yet." );
		return;

	case ITEMID_SPELLBOOK:
	case ITEMID_SPELLBOOK2:
		addSpellbookOpen( pItem );
		return;

	case ITEMID_DUMMY:
		// Train dummy.
		// ??? use effect instead !!!
		pItem->SetID( ITEMID_FX_DUMMY );
		pItem->SetTimeout( 3 );
		pItem->Sound( 0x033 );
		pItem->Update();
		m_pChar->Skill_UseQuick( m_pChar->GetWeaponSkill(), 30 );
		m_pChar->UpdateAnimate( ANIM_ATTACK_1H_WIDE );
		return;

	case ITEMID_MAP:
		addMap( pItem->GetUID() );
		return;

	case ITEMID_DYE:
		addTarget( TARGMODE_USE_ITEM, "Which dye vat will you use this on?", true );
		return;

	case ITEMID_DYEVAT:
		addTarget( TARGMODE_USE_ITEM, "Select the clothing to use this on.", true );
		return;

	case ITEMID_EMPTY_BOTTLE:
		// If we have a mortar then do alchemy.
		if ( m_pChar->ContentFind( ITEMID_MORTAR ) == NULL ) break;
		Event_Skill_Use( SKILL_ALCHEMY );
		return;

	case ITEMID_HERD_CROOK1:
	case ITEMID_HERD_CROOK2: // Use herding skill.
		m_pChar->m_Act_p.m_x = -1;
		Event_Skill_Use( SKILL_HERDING );
		return;
	case ITEMID_TINKER:	// Tinker tools.
		Event_Skill_Use( SKILL_TINKERING );
		return;
	case ITEMID_MORTAR:	// Alchemy menus.
		Event_Skill_Use( SKILL_ALCHEMY );
		return;
	case ITEMID_FISH_POLE:	// Just be near water ?
		Event_Skill_Use( SKILL_FISHING );
		return;
	case ITEMID_SCROLL_BLANK:
		Event_Skill_Use( SKILL_INSCRIPTION );
		return;
	case ITEMID_SEWINGKIT:	// Sew with materials we have on hand.
		Event_Skill_Use( SKILL_TAILORING );
		return;
	case ITEMID_MAP_BLANK:
		Event_Skill_Use( SKILL_CARTOGRAPHY );
		return;
	case ITEMID_LOCKPICK:
		Event_Skill_Use( SKILL_LOCKPICKING );
		return;
	case ITEMID_SHAFTS:
	case ITEMID_SHAFTS2:
	case ITEMID_SHAFTS3:
	case ITEMID_SHAFTS4:
	case ITEMID_SHAFTS5:
	case ITEMID_FEATHERS:
	case ITEMID_FEATHERS2:
	case ITEMID_FEATHERS3:
	case ITEMID_FEATHERS4:
	case ITEMID_FEATHERS5:
		Event_Skill_Use( SKILL_BOWCRAFT );
		return;

	case ITEMID_KINDLING:
	case ITEMID_KINDLING2:
		// Try to make a fire.
		Cmd_Skill_Camping( pItem );
		return;

	case ITEMID_Shovel:
	case ITEMID_Pickaxe1:
	case ITEMID_Pickaxe2:	// 	SKILL_MINING
	case ITEMID_Pitchfork:
	case ITEMID_BANDAGES:	// SKILL_HEALING, or SKILL_VETERINARY
		{
			CString sTmp;
			sTmp.Format( "Where do you want to use the %s?", pItem->GetName() );
			addTarget( TARGMODE_USE_ITEM, sTmp, false );
		}
		return;

	case ITEMID_KEYRING:
	case ITEMID_SCISSORS1: // cut hair and beard ?	
	case ITEMID_SCISSORS2: // cut up cloth to make bandages ? 
		{
			CString sTmp;
			sTmp.Format( "What do you want to use the %s on?", pItem->GetName() );
			addTarget( TARGMODE_USE_ITEM, sTmp, true );
		}
		return;

	case ITEMID_DEED1:
	case ITEMID_DEED2:
	case ITEMID_SHIP_PLANS1:
	case ITEMID_SHIP_PLANS2:
		addTargetMulti( pItem );
		return;

	case ITEMID_GAME_BACKGAM:	// backgammon board.
	case ITEMID_GAME_BOARD:	// game board.
		if ( pItem->GetTopLevelObj() != pItem )
		{
			addSysMessage( "Can't open game board in a container" );
			return;
		}
		{
			CContainerItem* pBoard = dynamic_cast <CContainerItem *>(pItem);
			pBoard->CreateGamePieces();
			addContainerSetup( pBoard );
		}
		return;
	} 

	// Use types of items.
	switch ( pItem->m_type )
	{
	case ITEM_CONTAINER:
		if ( Can_Snoop_Container( dynamic_cast <CContainerItem *>(pItem)))
			addContainerSetup( dynamic_cast <CContainerItem *>(pItem));
		break;

	case ITEM_SWITCH:
		// open castle gate. (etc)
		m_pChar->Use_Item( pItem );
		break;

	case ITEM_TELEPORT:	// teleport ring or such
		Cmd_Skill_Magery( SPELL_Teleport, pItem );
		break;

	case ITEM_KEY:
		addTarget( TARGMODE_USE_ITEM, "Select item to use the key on.", true );
		break;

	case ITEM_CONTAINER_LOCKED:
		addSysMessage( "This item is locked.");
		break;

	case ITEM_BOOK:
		addBookOpen( pItem );
		break;

	case ITEM_RUNE:
		// name the rune.
		SetTargMode( TARGMODE_NAME_RUNE, "What is the new name of the rune ?" );
		break;

	case ITEM_DOOR:
		m_pChar->Use_Item( pItem );
		break;

	case ITEM_DOOR_LOCKED:
		addSysMessage( "This door is locked.");
		break;

	case ITEM_FOOD:
		// eating anim
		m_pChar->Eat( pItem );
		break;

	case ITEM_DRINK:
	case ITEM_POTION:
		m_pChar->Drink( pItem );
		break;

	case ITEM_WEAPON_MACE:
	case ITEM_WEAPON_MACE_SHARP:// 22 = war axe can be used to cut/chop trees.
	case ITEM_WEAPON_SWORD:		// 23 = 
	case ITEM_WEAPON_FENCE:		// 24 = can't be used to chop trees.
		// 	SKILL_LUMBERJACKING,
		addTarget( TARGMODE_USE_ITEM, "What do you want to use this on?", true );
		return;

	case ITEM_FOOD_RAW:
		Event_Skill_Use( SKILL_COOKING );
		return;

	case ITEM_MUSICAL:
		m_pChar->Sound( pItem->Use_Music( GetRandVal(99) < m_pChar->m_Skill[SKILL_MUSICIANSHIP] ));
		Event_Skill_Use( SKILL_MUSICIANSHIP );
		return;

	case ITEM_TRAP:
		pItem->Use_Trap();
		return;

	case ITEM_CLOCK:
		addItemMessage( pItem->Use_Clock(), pItem );
		return;

	case ITEM_SCROLL:	// activate the scroll.

		return;

	case ITEM_CARPENTRY:
		// Carpentry type tool
		Event_Skill_Use( SKILL_CARPENTRY );
		return;

	case ITEM_SPAWN:
		addSysMessage( "You negate the spawn" );
		pItem->Kill_Spawn_Children();
		return;

	case ITEM_FIGURINE:
		// Create the creature here.
		{
			int id = pItem->m_more1;
			if ( ! id )
			{
				// Find the creature id.

			}
			if ( ! id ) return;	// just normal figure.
			m_pChar->Spell_Summon( id, (pItem->IsTopLevel()) ? pItem->m_p : m_pChar->m_p );
			delete pItem;
		}
		return;

	default:
		addSysMessage( "You can't think of a way to use that item.");
	}
}

//---------------------------------------------------------------------
// Login type stuff.

void CClient :: Setup_Start() // Send character startup stuff to player
{
	DEBUG_MSG(( "%x:Setup_Start acct='%s', char='%s'\n", GetSocket(), (const char*) m_sAccount, (const char*) m_pChar->GetName() ));

	srand( clock()); // Perform randomize

	if ( ! m_sAccount.CompareNoCase( "Administrator" ))	// This account always has privilege
	{
		m_Priv |= PRIV_Administrator;
	}
	m_pChar->MoveTo( m_pChar->m_p );	// ??? Make sure we are in active list.

	addPlayerStart();

	// Get the intro script.
	addSysMessage( Gray_szDesc );
	addScrollRead( ( ! m_sAccount.CompareNoCase( "Guest" )) ? "GUEST" : "MOTD", 2, 0 );

	if ( ! Serv.m_fSecure )
	{
		addBark( "WARNING: The world is running in DEBUG MODE", 
			NULL, 0x03B2, TALKMODE_SYSTEM, FONT_BOLD );
	}

	DEBUG_MSG(( "%x:Setup_Start done\n", GetSocket() ));
}

void CClient :: Setup_CreateDialog() // All the character creation stuff
{
	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Setup_CreateDialog acct='%s'\n", GetSocket(), m_sAccount ));
	}

	m_pChar = CharCreate( CREID_MAN );
	m_pChar->Create( &m_bin );
	m_pChar->m_sAccount = m_sAccount;

	Setup_Start();
}

void CClient :: Setup_Play() // After hitting "Play Character" button
{
	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Setup_Play slot %d\n", GetSocket(), (int) m_bin.Play.m_slot ));
	}

	int j=0;
	for ( CChar * pChar = static_cast <CChar*>( World.m_CharsIdle.GetHead()); pChar != NULL; pChar = pChar->GetNext())
	{
		if ( pChar->m_sAccount.CompareNoCase( m_sAccount ))
			continue;
		if ( j == m_bin.Play.m_slot )
		{
			m_pChar = pChar;
			Setup_Start();
			return;
		}
		j++;
	}
	addLoginErr( LOGIN_ERR_NONE );
}

bool CClient :: LogIn( const char * pName, const char * pPassword )
{
	if ( ! m_sAccount.IsEmpty()) return( true );
	if ( pName[0] == '\0' || strlen( pName ) > MAX_NAME_SIZE ) 
	{
		DEBUG_ERR(( "ERROR: Bad login name format\n" ));
		addLoginErr( LOGIN_ERR_NONE );
		return( false );
	}

	CScript s;
	if ( ! s.Open( GRAY_FILE "acct" GRAY_SCRIPT )) // Open script file
	{
		addLoginErr( LOGIN_ERR_NONE );
		return( false );
	}
	if ( ! s.FindSec( pName ))
	{
		// No account by this name ? So select a guest account.
		// Allow guest accounts ???

#ifdef COMMENT
		if ( ! strnicmp( pClient->m_sAccount, "GUEST", 5 ))
			iGuests ++;
	int iGuests = 0;
	if ( iAccount >= ACCOUNT_GUEST + Serv.m_nGuestsMax )
	{
		addLoginErr( LOGIN_ERR_NONE );
		return( false );
	}

#endif

		addLoginErr( LOGIN_ERR_NONE );
		return( false );
	}
	while(1)
	{
		if ( ! s.ReadParse()) 
		{
			// Cant read the password ?
			addLoginErr( LOGIN_ERR_OTHER );
			return( false );
		}
		if ( ! strcmpi( s.m_Line, "PRIV" ))
		{
			m_Priv = ahextoi( s.m_pArg );
		}
		if ( ! strcmpi( s.m_Line, "PASSWORD" ))
			break;
	}

	if ( ! strcmpi( s.m_pArg, "xxx" ))
	{
		addLoginErr( LOGIN_ERR_BLOCKED );
		return( false );
	}

	// Get the password.
	if ( pPassword[0] == '\0' || strcmpi( pPassword, s.m_pArg ))
	{
		DEBUG_ERR(( "ERR %x: '%s' bad pass '%s' != '%s'\n", GetSocket(), pName, pPassword, s.m_pArg ));
		addLoginErr( LOGIN_ERR_BADPASS );
		return( false );
	}

	if ( Serv.m_nClientsMax <= 1 && strcmpi( pName, "Administrator" ))
	{
		// Allow no one but Administrator on.
		addLoginErr( LOGIN_ERR_BLOCKED );
		return( false );
	}

	// Look for this account already in use.
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->m_sAccount.CompareNoCase( pName ))
		{
			addLoginErr( LOGIN_ERR_USED );
			return( false );
		}
	}

	m_sAccount = pName;
	DEBUG_MSG(( "%x:Login '%s', pass='%s'\n", GetSocket(), pName, pPassword ));
	return( true );
}

void CClient :: Setup_ListFill( void )
{
	// DEBUG_MSG(( "%x:Setup_ListFill\n", GetSocket() ));

	CCommand cmd;
	cmd.CharList.m_Cmd = 0xA9;
	int len = sizeof( cmd.CharList ) - sizeof(cmd.CharList.m_start) + ( Serv.m_StartCount * sizeof(cmd.CharList.m_start[0]));
	cmd.CharList.m_len = len;

	// list available chars for your account.
	cmd.CharList.m_count = 5;	
	int j=0;
	for ( CChar * pChar = static_cast <CChar*> ( World.m_CharsIdle.GetHead()); pChar != NULL; pChar = pChar->GetNext() )
	{
		if ( pChar->m_sAccount.CompareNoCase( m_sAccount ))
			continue;

		strncpy( cmd.CharList.m_char[j].m_name, pChar->GetName(), sizeof( cmd.CharList.m_char[j].m_name ));
		cmd.CharList.m_char[j].m_pass[0] = '\0';
		if ( ++j >= 5 ) break;
	}
	for ( ;j<5;j++)
	{
		cmd.CharList.m_char[j].m_name[0] = '\0';
		cmd.CharList.m_char[j].m_pass[0] = '\0';
	}

	cmd.CharList.m_startcount = Serv.m_StartCount;

	for ( int i=0;i<Serv.m_StartCount;i++)
	{
		cmd.CharList.m_start[i].m_id = i;
		strcpy( cmd.CharList.m_start[i].m_area, Serv.m_Starts[i].m_sArea );
		strcpy( cmd.CharList.m_start[i].m_name, Serv.m_Starts[i].m_sName );
	}

	xSendPkt( &cmd, len );
}

void CClient :: Setup_Delete() // Deletion of character
{
	DEBUG_MSG(( "%x:Setup_Delete slot=%d\n", GetSocket(), m_bin.Delete.m_slot ));

	int j=0;
	for ( CChar * pChar = static_cast <CChar*> ( World.m_CharsIdle.GetHead()); pChar != NULL; pChar = pChar->GetNext() )
	{
		if ( pChar->m_sAccount.CompareNoCase( m_sAccount ))	// this char is mine.
			continue;

		if ( j == m_bin.Delete.m_slot )
		{
			delete pChar;
			// refill the list.
			Setup_ListFill();
			return;
		}
		j++;
	}
}

void CClient :: Setup_List() // Gameserver login and character listing
{
	m_EnCrypt = true;
	if ( ! LogIn( m_bin.List.m_account, m_bin.List.m_password ))
		return;
	Setup_ListFill();
}

void CClient :: Login_Relay() // Relay player to a selected IP
{
	// DEBUG_MSG(( "%x:Login_Relay\n", GetSocket() ));

	CCommand cmd;
	cmd.Relay.m_Cmd = 0x8C;
	DWORD dwAddr = Serv.m_Servers[ m_bin.Relay.m_select ].m_ip.s_addr;
	cmd.Relay.m_ip[3] = ( dwAddr >> 24 ) & 0xFF;
	cmd.Relay.m_ip[2] = ( dwAddr >> 16 ) & 0xFF;
	cmd.Relay.m_ip[1] = ( dwAddr >> 8  ) & 0xFF;
	cmd.Relay.m_ip[0] = ( dwAddr		  ) & 0xFF;
	cmd.Relay.m_port = Serv.m_Servers[ m_bin.Relay.m_select ].m_port;
	cmd.Relay.m_Account = 0x7f000001; // customer account. (don't bother to check this.)

	xSendPkt( &cmd, sizeof(cmd.Relay));
}

void CClient :: Login_ServerList() // Initial login (Login on "loginserver", new format)
{
	if ( Serv.m_fVerbose )
	{
		DEBUG_MSG(( "%x:Login_ServerList\n", GetSocket() ));
	}

	if ( ! LogIn( m_bin.Servers.m_name, m_bin.Servers.m_password ))
		return;

	CCommand cmd;
	cmd.Servers.m_Cmd = 0xA8;
	int len = sizeof(cmd.Servers) - sizeof(cmd.Servers.m_serv) + ( Serv.m_ServerCount * sizeof(cmd.Servers.m_serv[0]));
	cmd.Servers.m_len = len;
	cmd.Servers.m_unk3 = 0xFF;
	cmd.Servers.m_count = Serv.m_ServerCount;

	for ( int i=0;i<Serv.m_ServerCount;i++)
	{
		cmd.Servers.m_serv[i].m_count = i;
		strncpy( cmd.Servers.m_serv[i].m_name, Serv.m_Servers[i].m_sName, sizeof(cmd.Servers.m_serv[i].m_name));
		cmd.Servers.m_serv[i].m_zero32 = 0;
		cmd.Servers.m_serv[i].m_percentfull = (i) ? 0 : Serv.m_Clients.GetCount();
		cmd.Servers.m_serv[i].m_timezone = 5;	// GRAY_TIMEZONE

		DWORD dwAddr = Serv.m_Servers[i].m_ip.s_addr;
		cmd.Servers.m_serv[i].m_ip[3] = ( dwAddr >> 24 ) & 0xFF;
		cmd.Servers.m_serv[i].m_ip[2] = ( dwAddr >> 16 ) & 0xFF;
		cmd.Servers.m_serv[i].m_ip[1] = ( dwAddr >> 8  ) & 0xFF;
		cmd.Servers.m_serv[i].m_ip[0] = ( dwAddr       ) & 0xFF;
	}

	xSendPkt( &cmd, len );
}

//----------------------------------------------------------------------

bool CClient :: DispatchMsg()
{
	// Process any messages we have.

	if ( ! xCheckSize( 1 ))	// just get the command
	{
		return( false );
	}

	if ( ! m_EnCrypt || m_sAccount.IsEmpty() )
	{
		switch ( m_bin.Default.m_Cmd )
		{
		case 0x80: // First Login
			if ( ! xCheckSize( sizeof( m_bin.Servers ))) return(false);
			Login_ServerList();
			break;
		case 0xA0:// Server Select - relay me to the server.
			if ( ! xCheckSize( sizeof( m_bin.Relay ))) return(false);
			Login_Relay();
			break;
		case 0x91: // Second Login to select char
			if ( ! xCheckSize( sizeof( m_bin.List ))) return(false);
			Setup_List();
			break;
		case 0xA4: // Spy not sure what this does.
			if ( ! xCheckSize( sizeof( m_bin.Spy ))) return(false);
			// DEBUG_MSG(( "%x:Spy1\n", GetSocket() ));
			break;
		case 0x72: // Tab = Combat Mode (toss this)
			if ( ! xCheckSize( sizeof( m_bin.War ))) return(false);
			break;
		default:
			return( false );
		}
		return( true );
	}

	/////////////////////////////////////////////////////
	// We should be encrypted below here.

	// Get messages from the client.
	switch ( m_bin.Default.m_Cmd )
	{
	case 0x00: // Character Create
		if ( ! xCheckSize( sizeof( m_bin.Create ))) return(false);
		Setup_CreateDialog();
		return( true );
	case 0x83: // Character Delete
		if ( ! xCheckSize( sizeof( m_bin.Delete ))) return(false);
		Setup_Delete();
		return( true );
	case 0x5D: // Character Select
		if ( ! xCheckSize( sizeof( m_bin.Play ))) return(false);
		Setup_Play();
		return( true );
	case 0x73: // Ping from client
		if ( ! xCheckSize( sizeof( m_bin.Ping ))) return(false);
		xSend( m_bin.m_Raw, sizeof( m_bin.Ping ));	// ping back.
		return( true );
	case 0xA7: // Get Tip
		if ( ! xCheckSize( sizeof( m_bin.Tip ))) return(false);
		Event_Tips( m_bin.Tip.m_index + 1 );
		return( true );
	}

	if ( m_pChar == NULL ) return( false );

	//////////////////////////////////////////////////////
	// We are now playing.

	switch ( m_bin.Default.m_Cmd )
	{
	case 0x02: // Walk
		if ( ! xCheckSize( sizeof( m_bin.Walk ))) return(false);
		Event_Walking();
		break;
	case 0x03: // Speech or at least text was typed.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize(m_bin.Talk.m_len )) return(false);
		Event_Talk();
		break;
	case 0x05: // Attack
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_Attack();
		break;
	case 0x06:// Doubleclick
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_DoubleClick();
		break;
	case 0x07: // Get Item
		if ( ! xCheckSize( sizeof( m_bin.ItemGet ))) return(false);
		Event_Item_Get();
		break;
	case 0x08: // Drop Item
		if ( ! xCheckSize( sizeof( m_bin.ItemDrop ))) return(false);
		Event_Item_Drop();
		break;
	case 0x09: // Singleclick
		if ( ! xCheckSize( sizeof( m_bin.Click ))) return(false);
		Event_SingleClick();
		break;
	case 0x12: // Ext. Command
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.ExtCmd.m_len )) return(false);
		Event_ExtCmd();
		break;
	case 0x13: // Equip Item
		if ( ! xCheckSize( sizeof( m_bin.ItemEquip ))) return(false);
		Event_Item_Equip();
		break;
	case 0x22: // Resync Request
		if ( ! xCheckSize( sizeof( m_bin.ReSyncReq ))) return(false);
		addReSync();
		break;
	case 0x2c:	// DeathOpt (un)Manifest ghost 
		if ( ! xCheckSize(2)) return(false);
		if ( ! Event_DeathOption()) return( false );
		break;
	case 0x34: // Status Request
		if ( ! xCheckSize( sizeof( m_bin.CharStatReq ))) return(false);
		if ( m_bin.CharStatReq.m_type == 4 ) addStatWindow( m_bin.CharStatReq.m_UID );
		if ( m_bin.CharStatReq.m_type == 5 ) addSkillWindow( SKILL_QTY );
		break;
	case 0x3b:	// Buy item from vendor.
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.VendorBuy.m_len )) return(false);
		Event_VendorBuy();
		break;
	case 0x56:	// plot course on map.
		if ( ! xCheckSize( sizeof( m_bin.MapPlot ))) return(false);
		break;
	case 0x66: // Read Book
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.BookPage.m_len )) return(false);
		Event_Book_Read();
		break;
	case 0x69: // Options set
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.Options.m_len )) return(false);
		DEBUG_MSG(( "%x:Set Options len=%d\n", GetSocket(), m_bin.Options.m_len ));
		break;
	case 0x6C: // Targeting
		if ( ! xCheckSize( sizeof( m_bin.Target ))) return(false);
		Event_Target();
		break;
	case 0x6F: // Secure trading
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.SecureTrade.m_len )) return(false);
		break;
	case 0x72: // Combat Mode
		if ( ! xCheckSize( sizeof( m_bin.War ))) return(false);
		Event_CombatMode( m_bin.War.m_warmode );
		break;
	case 0x75: // Rename Character
		if ( ! xCheckSize( sizeof( m_bin.CharName ))) return(false);
		Event_SetName();
		break;
	case 0x7D: // Menu Choice
		if ( ! xCheckSize( sizeof( m_bin.Choice ))) return(false);
		Event_Choice();
		break;
	case 0x95: // Color Select Dialog
		if ( ! xCheckSize( sizeof( m_bin.DyeVat ))) return(false);
		Event_Item_Dye();
		break;
	case 0x9B: // GM Page
		if ( ! xCheckSize( sizeof( m_bin.HelpPage ))) return(false);
		addScriptMenu( 1, TARGMODE_MENU_GM ); 
		break;
	case 0x9F: // Vendor Sell
		if ( ! xCheckSize(3)) return(false);
		if ( ! xCheckSize( m_bin.VendorSell.m_len )) return(false);
		Event_VendorSell();
		break;
	default:
		// clear socket I have no idea what this is.
		return( false );
	}

	return( true );
}

