//
// CChar.cpp
//  CChar is either an NPC or a Player.
//
#include "graysvr.h"	// predef header.

static CCharBase CharBase[] =	// ??? const 
{
	{ 1, "Air Elemental",	CREID_AIR_ELEM,		0, ITEMID_TRACK_ELEM_AIR,   0x107, 0x108, 0x109, 0x10a, 0x10b,	NULL, 0, 0, 0 },	
	{ 1, "Earth Elemental",	CREID_EARTH_ELEM,	0, ITEMID_TRACK_ELEM_EARTH, 0x10c, 0x10d, 0x10e, 0x10f, 0x110,	NULL, 0, 0, 0  },
	{ 1, "Fire Elemental",	CREID_FIRE_ELEM,	0, ITEMID_TRACK_ELEM_FIRE,  0x111, 0x112, 0x113, 0x114, 0x115,	NULL, 0, 0, 0  },	
	{ 1, "Water Elemental",	CREID_WATER_ELEM,	0, ITEMID_TRACK_ELEM_WATER, 0x116, 0x117, 0x118, 0x119, 0x11a,	NULL, 0, 0, 0  },

	{ 1, "Ogre",			CREID_OGRE,			0, (ITEMID_TYPE) 0x20cb, 0x1ab, 0x1ac, 0x1ad, 0x1ae, 0x1af,		"M2", 0, 0, 0  },

	{ 1, "Ettin",			CREID_ETTIN,		0, (ITEMID_TYPE) 0x20c8, 0x16f, 0x170, 0x171, 0x172, 0x173,		"M5", 0, 0, 0  },
	{ 1, "Ettin",			CREID_ETTIN_AXE,	0, (ITEMID_TYPE) 0x20c8, 0x16f, 0x170, 0x171, 0x172, 0x173,		"M6", 0, 0, 0 },

	{ 1, "Lizardman",		CREID_LIZMAN,		0, (ITEMID_TYPE) 0x20ca, 0x1a1, 0x1a2, 0x1a3, 0x1a4, 0x1a5,		"M2" },
	{ 1, "Lizardman",		CREID_LIZMAN_SPEAR,	0, (ITEMID_TYPE) 0x20ca, 0x1a1, 0x1a2, 0x1a3, 0x1a4, 0x1a5,		"M2" },	
	{ 1, "Lizardman",		CREID_LIZMAN_MACE,	0, (ITEMID_TYPE) 0x20ca, 0x1a1, 0x1a2, 0x1a3, 0x1a4, 0x1a5,		"M2" },

	{ 1, "Daemon",			CREID_DAEMON,		0, (ITEMID_TYPE) 0x20D3, 0x165, 0x166, 0x167, 0x169, 0x169 },
	{ 1, "Daemon Knight",	CREID_DAEMON_SWORD,	0, (ITEMID_TYPE) 0x2104, 0x165, 0x166, 0x167, 0x169, 0x169 },

	{ 1, "Orc",				CREID_ORC,			0, (ITEMID_TYPE) 0x20E0, 0x1b0, 0x1b1, 0x1b2, 0x1b3, 0x1b4 },
	{ 1, "Orc",				CREID_ORC_CLUB,		0, (ITEMID_TYPE) 0x20E0, 0x1b0, 0x1b1, 0x1b2, 0x1b3, 0x1b4 },	
	{ 1, "Orc Lord",		CREID_ORC_LORD,		0, (ITEMID_TYPE) 0x20E0, 0x1b0, 0x1b1, 0x1b2, 0x1b3, 0x1b4 },

	{ 1, "Ratman",			CREID_RATMAN,		0, (ITEMID_TYPE) 0x20e3, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b9 },
	{ 1, "Ratman",			CREID_RATMAN_CLUB,	0, (ITEMID_TYPE) 0x20e3, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b9 },
	{ 1, "Ratman",			CREID_RATMAN_SWORD,	0, (ITEMID_TYPE) 0x20e3, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b9 },	
	{ 1, "Rat",				CREID_Rat,			0, (ITEMID_TYPE) 0x2123, 0x0cc, 0x0cd, 0x0ce, 0x0cf, 0x0d0 },
	{ 1, "Giant Rat",		CREID_GiantRat,		0, (ITEMID_TYPE) 0x20D0, 0x188, 0x189, 0x18a, 0x18b, 0x18c },	

	{ 1, "Troll Captain",	CREID_TROLL_SWORD,	0, (ITEMID_TYPE) 0x20cc, 0x1cd, 0x1ce, 0x1cf, 0x1d0, 0x1d1 },
	{ 1, "Troll",			CREID_TROLL,		0, (ITEMID_TYPE) 0x20cc, 0x1cd, 0x1ce, 0x1cf, 0x1d0, 0x1d1 },
	{ 1, "Troll Seargent",	CREID_TROLL_MACE,	0, (ITEMID_TYPE) 0x20cc, 0x1cd, 0x1ce, 0x1cf, 0x1d0, 0x1d1 },

	{ 1, "Liche",			CREID_LICH,			0, (ITEMID_TYPE) 0x20f8, 0x19c, 0x19d, 0x19e, 0x19f, 0x1a0 },
	{ 1, "Spectre",			CREID_SPECTRE,		0, (ITEMID_TYPE) 0x2109, 0x17e, 0x17f, 0x180, 0x181, 0x182 },
	{ 1, "Zombie",			CREID_ZOMBIE,		0, (ITEMID_TYPE) 0x20ec, 0x1d7, 0x1d8, 0x1d9, 0x1da, 0x1db },	

	{ 1, "Skeleton",		CREID_SKELETON,     0, (ITEMID_TYPE) 0x20e7, 0x1c3, 0x1c4, 0x1c5, 0x1c6, 0x1c7 },
	{ 1, "Skeletal Guard",	CREID_SKEL_AXE,		0, (ITEMID_TYPE) 0x20e7, 0x1c3, 0x1c4, 0x1c5, 0x1c6, 0x1c7 },	
	{ 1, "Skeletal Knight",	CREID_SKEL_SW_SH,	0, (ITEMID_TYPE) 0x20e7, 0x1c3, 0x1c4, 0x1c5, 0x1c6, 0x1c7 },

	{ 1, "Dragon",			CREID_DRAGON_GREY,	0, (ITEMID_TYPE) 0x20d6, 0x16a, 0x16b, 0x16c, 0x16d, 0x16e },
	{ 1, "Dragon",			CREID_DRAGON_RED,	0, (ITEMID_TYPE) 0x20d6, 0x16a, 0x16b, 0x16c, 0x16d, 0x16e },
	{ 1, "Drake",			CREID_DRAKE_GREY,	0, (ITEMID_TYPE) 0x20d6, 0x16a, 0x16b, 0x16c, 0x16d, 0x16e },
	{ 1, "Drake",			CREID_DRAKE_RED,	0, (ITEMID_TYPE) 0x20d6, 0x16a, 0x16b, 0x16c, 0x16d, 0x16e },	

	{ 1, "Giant Snake",		CREID_GIANT_SNAKE,	0, (ITEMID_TYPE) 0x20fe, 0x0db, 0x0dc, 0x0dd, 0x0de, 0x0df },
	{ 1, "Snake",			CREID_Snake,		0, (ITEMID_TYPE) 0x20fc, 0x0db, 0x0dc, 0x0dd, 0x0de, 0x0df },
	{ 1, "Sea Serpant",		CREID_SEA_SERP,		0, (ITEMID_TYPE) 0x20fe, 0x1bf, 0x1c0, 0x1c1, 0x1c2, 0x1c3 },	

	{ 1, "Eagle",			CREID_EAGLE,		0, (ITEMID_TYPE) 0x211d, 0x08f, 0x090, 0x091, 0x092, 0x093, "F36,?1,9b9" },
	{ 1, "Chicken",			CREID_Chicken,		0, (ITEMID_TYPE) 0x20D1, 0x06e, 0x06f, 0x070, 0x071, 0x072, "F25,?1,9b9" },
	{ 1, "Crow",			CREID_BIRD,			0, (ITEMID_TYPE) 0x20ee, 0x07d, 0x07e, 0x07f, 0x080, 0x081, "F25,M1" },
	{ 1, "Harpy",			CREID_HARPY,		0, (ITEMID_TYPE) 0x20dc, 0x192, 0x193, 0x194, 0x195, 0x196, "F36,M2" },	
	{ 1, "Mongbat",			CREID_MONGBAT,		0, (ITEMID_TYPE) 0x20f9, 0x1a6, 0x1a7, 0x1a8, 0x1a9, 0x1aa, "H2,M2,?2,f78" },

	{ 1, "Corpser",			CREID_CORPSER,		0, (ITEMID_TYPE) 0x20d2, 0x161, 0x000, 0x162, 0x163, 0x164 },
	{ 1, "Reaper",			CREID_REAPER,		0, (ITEMID_TYPE) 0x20fa, 0x1ba, 0x1bb, 0x1bc, 0x1bd, 0x1be },

	{ 1, "Brown Bear",		CREID_BrownBear,	0, (ITEMID_TYPE) 0x20CF, 0x05f, 0x060, 0x061, 0x062, 0x063 },
	{ 1, "Polar Bear",		CREID_PolarBear,	0, (ITEMID_TYPE) 0x20E1, 0x05f, 0x060, 0x061, 0x062, 0x063 },
	{ 1, "Grizzly Bear",	CREID_GrizzlyBear,	0, (ITEMID_TYPE) 0x211e, 0x0a3, 0x0a4, 0x0a5, 0x0a6, 0x0a7 },

	{ 1, "Hart",			CREID_Hart,			0, (ITEMID_TYPE) 0x20D4, 0x000, 0x000, 0x082, 0x083, 0x084, "H15,M5" },
	{ 1, "Deer",			CREID_Deer,			0, (ITEMID_TYPE) 0x20d4, 0x000, 0x000, 0x082, 0x083, 0x084, "H9,M5" },	

	{ 1, "Pig",				CREID_Pig,			0, (ITEMID_TYPE) 0x2101, 0x0c4, 0x0c5, 0x0c6, 0x0c7, 0x0c8, "H5,M3" },	
	{ 1, "Boar",			CREID_Boar,			0, (ITEMID_TYPE) 0x2101, 0x0c4, 0x0c5, 0x0c6, 0x0c7, 0x0c8, "H8,M5" },

	{ 1, "Horse",			CREID_HORSE1,		0, (ITEMID_TYPE) 0x2120, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, "F8,M5" },
	{ 1, "Horse",			CREID_HORSE2,		0, (ITEMID_TYPE) 0x211F, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, "F8,M5" },
	{ 1, "Horse",			CREID_HORSE3,		0, (ITEMID_TYPE) 0x2124, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, "F8,M5" },
	{ 1, "Horse",			CREID_HORSE4,		0, (ITEMID_TYPE) 0x2121, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, "F8,M5" },
	{ 1, "Pack Horse",		CREID_HORSE_PACK,	0, (ITEMID_TYPE) 0x2120, 0x0a8, 0x0a9, 0x0aa, 0x0ab, 0x0ac, "F8,M5" },

	{ 1, "Llama",			CREID_Llama,		0, (ITEMID_TYPE) 0x20f6, 0x000, 0x000, 0x0b7, 0x0b8, 0x0b9 },	
	{ 1, "Pack Llama",		CREID_LLAMA_PACK,	0, (ITEMID_TYPE) 0x20f6, 0x000, 0x000, 0x0b7, 0x0b8, 0x0b9 }, 

	{ 1, "Sheep",			CREID_Sheep,		0, (ITEMID_TYPE) 0x20EB, 0x0d6, 0x0d7, 0x0d8, 0x0d9, 0x0da, "W15,M4,H3" },
	{ 1, "Sheared Sheep",	CREID_Lamb,			0, (ITEMID_TYPE) 0x20E6, 0x0d6, 0x0d7, 0x0d8, 0x0d9, 0x0da, "M4,H3" },
	{ 1, "Goat",			CREID_Goat,			0, (ITEMID_TYPE) 0x2108, 0x099, 0x09a, 0x09b, 0x09c, 0x09d, "M4,H3" },

	{ 1, "Dog",				CREID_Dog,			0, (ITEMID_TYPE) 0x211c, 0x085, 0x086, 0x087, 0x088, 0x089 },
	{ 1, "Wolf",			CREID_Wolf,			0, (ITEMID_TYPE) 0x20D5, 0x0e5, 0x0e6, 0x0e7, 0x0e8, 0x0e9 },

	{ 1, "Cat",				CREID_Cat,			0, (ITEMID_TYPE) 0x211b, 0x069, 0x06a, 0x06b, 0x06c, 0x06d },
	{ 1, "Panther",			CREID_Panther,		0, (ITEMID_TYPE) 0x2119, 0x0ba, 0x0bb, 0x0bc, 0x0bd, 0x0be },

	{ 1, "Cow",				CREID_Cow_BW,		0, (ITEMID_TYPE) 0x2103, 0x078, 0x079, 0x07a, 0x07b, 0x07c },
	{ 1, "Cow",				CREID_Cow2,			0, (ITEMID_TYPE) 0x2103, 0x078, 0x079, 0x07a, 0x07b, 0x07c },
	{ 1, "Bull",			CREID_Bull_Brown,	0, (ITEMID_TYPE) 0x20f0, 0x064, 0x065, 0x066, 0x067, 0x068 },
	{ 1, "Bull",			CREID_Bull2,		0, (ITEMID_TYPE) 0x20f0, 0x064, 0x065, 0x066, 0x067, 0x068 },

	{ 1, "Alligator",		CREID_Alligator,	0, (ITEMID_TYPE) 0x20DA, 0x05a, 0x05b, 0x05c, 0x05d, 0x05e },	
	{ 1, "Dolphin",			CREID_Dolphin,		0, (ITEMID_TYPE) 0x20f1, 0x08a, 0x08b, 0x08c, 0x08d, 0x08e },
	{ 1, "Gorilla",			CREID_GORILLA,		0, (ITEMID_TYPE) 0x20c9, 0x09e, 0x09f, 0x0a0, 0x0a1, 0x0a2 },
	{ 1, "Rabbit",			CREID_Rabbit,		0, (ITEMID_TYPE) 0x20E2, 0x000, 0x000, 0x0c9, 0x0ca, 0x0cb }, // 2125
	{ 1, "Walrus",			CREID_Walrus,		0, (ITEMID_TYPE) 0x20ff, 0x0e0, 0x0e1, 0x0e2, 0x0e3, 0x0e4 },

	{ 1, "Giant Spider",	CREID_GIANT_SPIDER,	0, (ITEMID_TYPE) 0x20fd, 0x183, 0x184, 0x185, 0x186, 0x187 },
	{ 1, "Giant Scorpion",	CREID_SCORP,		0, (ITEMID_TYPE) 0x20e4, 0x18d, 0x18e, 0x18f, 0x190, 0x191 },
	{ 1, "Slime",			CREID_SLIME,		0, (ITEMID_TYPE) 0x20e8, 0x1c8, 0x1c9, 0x1ca, 0x1cb, 0x1cc },
	{ 1, "Wisp",			CREID_WISP,			0, (ITEMID_TYPE) 0x2100, 0x1d2, 0x1d3, 0x1d4, 0x1d5, 0x1d6 },
	{ 1, "Gargoyle",		CREID_GARG,			0, (ITEMID_TYPE) 0x20d9, 0x174, 0x175, 0x176, 0x177, 0x178 },
	{ 1, "Gazer",			CREID_GAZER,		0, (ITEMID_TYPE) 0x20f4, 0x179, 0x17a, 0x17b, 0x17c, 0x17d },
	{ 1, "Headless",		CREID_HEADLESS,		0, (ITEMID_TYPE) 0x210a, 0x197, 0x198, 0x199, 0x19a, 0x19b },	

	// all here are humanish

	{ 1, "Blade Spirits",	CREID_BLADES,		0, ITEMID_FX_BLADES, 0x200, 0x200, 0x200, 0x200, 0x200 },	// odd that it is in human space ?

	{ 1, "Man",				CREID_MAN,			0, ITEMID_TRACK_MAN		},
	{ 1, "Woman",			CREID_WOMAN,		0, ITEMID_TRACK_WOMAN	},	
	{ 1, "Boy",				CREID_CHILD_MB,		0, ITEMID_TRACK_MAN		},	
	{ 1, "Boy",				CREID_CHILD_MD,		0, ITEMID_TRACK_MAN		},	
	{ 1, "Girl",			CREID_CHILD_FD,		0, ITEMID_TRACK_WOMAN	},	
	{ 1, "Toddler",			CREID_CHILD_FB,		0, ITEMID_TRACK_WOMAN	},	

	// not to be picked randomly
	{ 1, "Ghost",			CREID_GHOSTMAN,		0, ITEMID_TRACK_MAN		},
	{ 1, "Ghost",			CREID_GHOSTWOMAN,	0, ITEMID_TRACK_WOMAN	},
	{ 1, "Invisible Man",	CREID_INVISIBLE,	0, ITEMID_TRACK_MAN		},	// Big foot sounds ??

#ifdef COMMENT
	// 0x400 = stuff not directly linked to graphic.
	// More sounds. diff colors ?
	{ 1, "Cougar",			CREID_Panther,		0, 0x2119, 0x073, 0x074, 0x075, 0x076, 0x077 },		// black
	{ 1, "Lion",			CREID_Panther,		0, 0x2119, 0x1b2, 0x1b3, 0x1b4, 0x1b5, 0x1b6 },		// yellow
	{ 1, "Bird",			CREID_BIRD,			0, 0x20ee, 0x017, 0x018, 0x018, 0x018, 0x018 },
	{ 1, "Jungle bird",		CREID_BIRD,			0, 0x20ee, 0x0ad, 0x0ae, 0x0af, 0x0b0, 0x0b1	}, 
	{ 1, "Parrot",			CREID_BIRD,			0, 0x20ee, 0x0bf, 0x0c0, 0x0c1, 0x0c2, 0x0c3 },
	{ 1, "Raven",			CREID_BIRD,			0, 0x20ee, 0x0d1, 0x0d2, 0x0d3, 0x0d4, 0x0d5 },	// black

//	{ 1, "Ogre Lord",		CREID_OGRE,			0, (ITEMID_TYPE) 0x20cb, 0x1ab, 0x1ac, 0x1ad, 0x1ae, 0x1af },
//	{ 1, "Ogre Magi",		CREID_OGRE,			0, (ITEMID_TYPE) 0x20cb, 0x1ab, 0x1ac, 0x1ad, 0x1ae, 0x1af },	// blue skin
#endif

};

const WORD Item_Hair[] = // types of hair.
{
	ITEMID_HAIR_SHORT,
	ITEMID_HAIR_LONG,
	ITEMID_HAIR_PONY,
	ITEMID_HAIR_MOHAWK,
	ITEMID_HAIR_PAGE,
	ITEMID_HAIR_CURL,
	ITEMID_HAIR_7,
	ITEMID_HAIR_RECEDE,
	ITEMID_HAIR_2TAILS,
	ITEMID_HAIR_TOPKNOT,
};

const WORD Item_Beards[] = // beard types
{
	ITEMID_BEARD_LONG,
	ITEMID_BEARD_SHORT,
	ITEMID_BEARD_GOATEE,
	ITEMID_BEARD_MOUSTACHE,
	ITEMID_BEARD_SH_M,
	ITEMID_BEARD_LG_M,
	ITEMID_BEARD_GO_M,	// Goatee and mustache.
};

/////////////////////////////////////////////////////////////////
// -CCharBase

static const CCharBase * FindBase( CREID_TYPE id )
{
	int i=0;
	for ( ; i<COUNTOF( CharBase ); i++ )
	{
		if ( CharBase[i].m_id == id )
		{
			return( &CharBase[i] );
		}
	}
	return( NULL );
}

/////////////////////////////////////////////////////////////////
// -CChar

CChar * CharCreate( CREID_TYPE id )
{
	CChar * pChar = new CChar( id );
	World.m_CharsIdle.InsertAfter( pChar ); // Put in the idle list by default.
	return( pChar );
}

CChar * CharCreateScript( int id )
{
	CChar * pChar = CharCreate( (CREID_TYPE) id );
	pChar->CreateScript( id );
	return( pChar );
}

CChar :: CChar( CREID_TYPE id ) : CObjBase( id, false )
{
	sm_iCount++;

	m_pClient = NULL;	// Not logged in.
	m_pRegion = NULL;

	m_StatFlag = 0;

	m_color = m_prev_color = 0;
	m_dir = DIR_SE;
	m_fonttype = FONT_NORMAL;

	m_attack = 0;
	m_defense = 0;

	int i=0;
	for ( ;i<STAT_QTY;i++) m_Stat[i]=0;
	m_Stat[STAT_Fame]=10;
	m_Stat[STAT_Karma]=10;
	m_health = m_Stat[STAT_STR]=50;
	m_stam = m_Stat[STAT_DEX]=50;
	m_mana = m_Stat[STAT_INT]=50;
	m_food = 30;

	for ( i=0;i<SKILL_QTY;i++) m_Skill[i]=10;

	m_NPC_Brain = NPCBRAIN_NONE;
	m_NPC_Speech = 0;

	m_regen_stam = 0;
	m_regen_mana = 0;
	m_regen_health = 0;
	m_regen_food = 0;

	m_pHome = NULL;

	SetID( id );	// virtuals not called by constructurs.
	m_prev_id = (CREID_TYPE) GetID();
	Skill_Cleanup();
}

CChar :: ~CChar() // Delete character
{
	sm_iCount--;
	if ( m_owner.IsValidUID() && m_owner.IsItem())
	{
		// Unlink from the spawn object.
		CItem * pSpawn = m_owner.ItemFind();
		if ( pSpawn != NULL && 
			pSpawn->m_type == ITEM_SPAWN && 
			pSpawn->m_amount )
		{
			pSpawn->m_amount --;
		}
	}
	ASSERT( m_pClient == NULL );	// This is very bad.
}

bool CChar :: IsValid() const
{
	if ( ! CObjBase :: IsValid()) return( false );
	if ( ! m_Stat[ STAT_DEX ] ) return( false );
	if ( ! m_Stat[ STAT_STR ] ) return( false );
	if ( ! m_Stat[ STAT_INT ] ) return( false );

	if ( m_NPC_Brain == NPCBRAIN_NONE )
	{
		if ( m_sAccount.IsEmpty()) return( false );
	}
	else
	{
		if ( ! m_sAccount.IsEmpty()) return( false );
	}
	return( m_p.IsValid());
}

void CChar :: WeightChange( int iChange )
{
	CContainer::WeightChange( iChange );
	if ( m_pClient != NULL )
	{
		UpdateStats();
	}
}

void CChar :: SetName( const char * pName )
{
	// Parse out the name from the name pool ?
	// NOTE: Name must be <= MAX_NAME_SIZE

	if ( pName[0] == '#' )
	{
		pName ++;
		char * pPool = GetTempStr();
		int i = 0;
		for ( i=0; pName[i] != ' ' && pName[i] != '\0'; i++ )
			pPool[i] = pName[i];
		pPool[i] = '\0';
		const char * pTitle = pName+i;
		const char * pTitle2 = pTitle;
		while ( *pTitle2 == ' ' ) pTitle2 ++;
		m_sName = pTitle2;
	
		CScript s;
		if ( ! s.Open( GRAY_FILE "name" GRAY_SCRIPT )) 
			return;
		if ( ! s.FindSec( pPool ))
			return;
		// Pick a random name.
		if ( ! s.ReadLine())
			return;
		int iCount = GetRandVal( atoi( s.m_Line ));
		while ( iCount-- )
		{
			if ( ! s.ReadLine())
				return;
		}

		m_sTitle = pTitle;
		CObjBase::SetName( s.m_Line );
		return;
	}

	int len = strlen( pName );
	if ( len >= MAX_NAME_SIZE )
	{
		char * pTemp = GetTempStr();
		strncpy( pTemp, pName, MAX_NAME_SIZE );
		pTemp[ MAX_NAME_SIZE ] = '\0';
		pName = pTemp;
	}
	CObjBase::SetName( pName );
}

bool CChar :: IsFemale( CREID_TYPE id )
{

static const WORD Females[] =
{
	CREID_HARPY,
	CREID_Chicken,
	CREID_Deer,
	CREID_Cow_BW,
	CREID_Cow2,
	CREID_WOMAN,
	CREID_GHOSTWOMAN,
	CREID_CHILD_FD,
};

	return( FindID( id, Females, COUNTOF( Females )) != -1 );
}

const char * CChar :: GetPronoun() const
{
	switch ( GetID() )
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "he" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "she" );
	default:
		return( "it" );
	}
}

const char * CChar :: GetPossessPronoun() const
{
	switch ( GetID() )
	{
	case CREID_CHILD_MB:
	case CREID_CHILD_MD:
	case CREID_MAN:
	case CREID_GHOSTMAN:
		return( "his" );
	case CREID_CHILD_FB:
	case CREID_CHILD_FD:
	case CREID_WOMAN:
	case CREID_GHOSTWOMAN:
		return( "her" );
	default:
		return( "it's" );
	}
}

const char * CChar :: GetName() const
{
	// override this base function.
	// allow some creatures to go unnamed.
	const char * pName = CObjBase :: GetName();

	if ( pName[0] == '\0' )	
	{
		// Just use it's type name instead.
		return( m_pCre->m_name );
	}

	return( pName );
}

void CChar :: SetID( CREID_TYPE id )
{
	if ( id < 0 || id >= CREID_QTY ) id = CREID_MAN;

	m_pCre = FindBase( id );
	if ( m_pCre == NULL )
	{
		DEBUG_ERR(( "ERROR: Create Invalid Char %xh\n", id ));
		id = CREID_MAN;
		m_pCre = FindBase( id );
	}
	if ( m_pCre->m_id < CREID_MAN )
	{
		// Transfom to non-human !
		// can't ride a horse in this form.
		Horse_UnMount();
		// unequip all items. 
		UnEquipAllItems();
	}
	SetPrivateID( id );
}

bool CChar :: IsInDungeon() const
{
	// What part of the maps are filled with dungeons.
	if ( m_p.m_x < UO_SIZE_X_REAL )
		return( false );

	int x1=(m_p.m_x-UO_SIZE_X_REAL)/256;
	switch ( m_p.m_y / 256 )
	{
	case 0:
	case 5:
		return( true );
	case 1:
		if (x1!=0) return( true );
		break;
	case 2:
	case 3:
		if (x1<3) return( true );
		break;
	case 4:
	case 6:
		if (x1<1) return( true );
		break;
	case 7:
		if (x1<2) return( true );
		break;
	}
	return( false );
}

int CChar :: GetLightLevel() const
{
	if ( m_StatFlag & STATF_DEAD )
		return( 0 );
	if ( m_StatFlag & STATF_Sleeping )	// eyes closed.
		return( 13 );
	if ( m_StatFlag & STATF_NightSight ) // dead don't need light.
		return( 0 );
	if ( IsInDungeon())
		return( 15 );
	return( World.m_globallight );
}

CContainerItem * CChar :: GetPack( void )
{
	CContainerItem *pPack = dynamic_cast <CContainerItem*>( LayerFind( LAYER_PACK ));
	if ( pPack == NULL )
	{
		// add one if needed.
		if ( World.IsLoading()) return( NULL );
		pPack = dynamic_cast <CContainerItem*>( ItemCreateScript( ITEMID_BACKPACK ));
		pPack->m_Attr |= ATTR_NEWBIE;
		LayerAdd( pPack, LAYER_PACK );
	}
	return( pPack );
}

CItem * CChar :: LayerFind( LAYER_TYPE layer )
{
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( pItem->GetEquipLayer() == layer )
			break;
	}
	return( pItem );
}

void CChar :: LayerAdd( CItem * pItem, LAYER_TYPE layer )
{
	// add equipped items.
	// check for item already in that layer ?

	CItem * pItemPrev = LayerFind( layer );
	if ( pItemPrev != NULL )
	{
		if ( layer == LAYER_DRAGGING )	// drop it.
		{
			pItemPrev->PutOnGround( m_p );
		}
		else if ( layer >= LAYER_SPELL_STATS )
		{
			// Magic spell slots just get bumped.
			delete pItemPrev;
		}
		else
		{
			DEBUG_ERR(( "LayerAdd Layer %d already used\n", layer ));
			// Force the current layer item into the pack ?
			CContainerItem * pPack = GetPack();
			if ( pPack != NULL )	// if loading this could be NULL
			{
				pPack->ContentAdd( pItemPrev );
			}
		}
	}

	if ( GetID() < CREID_MAN && 
		layer != LAYER_NONE && 
		layer != LAYER_PACK && 
		layer <= LAYER_HORSE )
	{
		// some creatures can equip certain items ???
		DEBUG_ERR(( "ContentAdd id=%xh '%s', Creature can't equip %d\n", pItem->GetID(), pItem->GetName(), layer ));
		layer = LAYER_NONE;	// can't equip stuff.
	}

	if ( layer == LAYER_NONE )
	{
		// we should not allow non-layered stuff to be put here ?
		// Put in pack instead ?
		DEBUG_ERR(( "ContentAdd id=%xh '%s', LAYER_NONE is strange\n", pItem->GetID(), pItem->GetName() ));
		CContainerItem * pPack = GetPack();
		if ( pPack != NULL )	// if loading this could be NULL
		{
			pPack->ContentAdd( pItem );
			return;
		}
	}

	CContainer::ContentAdd( pItem );
	pItem->SetContainerFlags( UID_EQUIPPED );
	pItem->m_p.m_x = 0;	// these don't apply.
	pItem->m_p.m_y = 0;
	pItem->m_equip_layer = layer;	// actually is equipped. // GetEquipLayer()

	switch ( layer )
	{
	case LAYER_UNUSED9:
		DEBUG_ERR(( "ALERT: Weird layer 9 used for %xh check this\n", pItem->GetID() ));
		ASSERT( 0 );
		break;

	case LAYER_HAND1:
	case LAYER_HAND2:
		// If weapon
		switch ( pItem->m_type )
		{
		case ITEM_WEAPON_MACE:			// Can be used for smithing ?
		case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
		case ITEM_WEAPON_SWORD:
		case ITEM_WEAPON_FENCE:
		case ITEM_WEAPON_BOW:
			m_weapon = pItem->GetUID();
			m_attack += pItem->m_pDef->m_attack;
			SetWeaponSwingTimer( true );
			break;
		case ITEM_ARMOR:
			// Shield ? or 2 handed weapon ?
			break;
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
    case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
    case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
    case LAYER_SKIRT:
    case LAYER_LEGS:
		// If armor or clothing
		m_defense += pItem->m_pDef->m_attack;
		break;
	case LAYER_PACK2:
		layer = LAYER_PACK;
	case LAYER_PACK:
		ASSERT( pItem->IsSameID( ITEMID_BACKPACK ) || pItem->IsSameID( ITEMID_BACKPACK2 ));
		break;
	case LAYER_HORSE:
		m_StatFlag |= STATF_OnHorse;
		break;
	}

	if ( ! World.IsLoading() && ( pItem->m_Attr & ATTR_MAGIC ))
	{
		// If items are magical then add the effect.
		// Spell_Effect_Add( pItem );
	}

	pItem->Update();
}

void CChar :: ContentRemove( CItem * pItem )
{
	//ASSERT( pItem->IsEquipped());
	CContainer::ContentRemove( pItem );

	// remove equipped items.
	switch ( pItem->GetEquipLayer() )
	{
	case LAYER_HAND1:
	case LAYER_HAND2:	// other hand = shield
		switch ( pItem->m_type )
		{
		case ITEM_WEAPON_MACE:			// Can be used for smithing ?
		case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
		case ITEM_WEAPON_SWORD:
		case ITEM_WEAPON_FENCE:
		case ITEM_WEAPON_BOW:
			m_weapon = UID_UNUSED;
			m_attack -= pItem->m_pDef->m_attack;
			SetWeaponSwingTimer( true );
			break;
		case ITEM_ARMOR:
			// Shield ? or 2 handed weapon ?
			break;
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
    case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
    case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
    case LAYER_SKIRT:
    case LAYER_LEGS:
		m_defense -= pItem->m_pDef->m_attack;
		break;
	case LAYER_PACK:
		ASSERT( pItem->IsSameID( ITEMID_BACKPACK ) || pItem->IsSameID( ITEMID_BACKPACK2 ));
		break;
	case LAYER_HORSE:
		m_StatFlag &= ~STATF_OnHorse;
		break;
	}

	// Items with magic effects.
	if ( pItem->m_Attr & ATTR_MAGIC )
	{
		// If items are magical then remove effect here.
		Spell_Effect_Remove( pItem );
	}
}

void CChar :: UnEquipAllItems( CContainerItem * pDest )
{
	if ( ! GetCount()) return;
	CContainerItem * pPack = GetPack();

	CItem* pItemNext; 
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( pItem->GetEquipLayer() >= LAYER_HORSE ) continue;
		switch ( pItem->GetEquipLayer() )
		{
		case LAYER_PACK:
		case LAYER_HAIR:
		case LAYER_BEARD:
			continue;
		}
		if ( pDest != NULL && ! ( pItem->m_Attr & ATTR_NEWBIE ))
		{	// Move item to dest. (corpse ussually)
			pDest->ContentAdd( pItem );
			if ( pDest->GetID() == ITEMID_CORPSE )
			{
				// Equip layer only matters on a corpse.
				pItem->m_equip_layer = pItem->m_pDef->m_layer;
			}
		}
		else 
		{	// Move item to pack.
			pPack->ContentAdd( pItem );
		}
	}
}

void CChar :: Write( CScript & s ) const
{
	s.Printf( "[WORLDCHAR %x]\n", GetID() );

	if ( ! m_sAccount.IsEmpty())
		s.Printf( "ACCOUNT=%s\n", m_sAccount );

	CObjBase :: Write( s );

	if ( m_p.IsValid())
		s.Printf( "P=%s\n", m_p.Write() );
	if ( ! m_sTitle.IsEmpty())
		s.Printf( "TITLE=%s\n", m_sTitle );
	if ( m_fonttype != FONT_NORMAL )
		s.Printf( "FONT=%i\n", m_fonttype);
	if ( m_dir != DIR_SE )
		s.Printf( "DIR=%i\n", m_dir );
	s.Printf( "XBODY=%x\n", m_prev_id );
	s.Printf( "XSKIN=%x\n", m_prev_color );

	s.Printf( "FLAGS=%x\n", m_StatFlag );
	if ( m_Act_Skill != SKILL_NONE )
		s.Printf( "ACTION=%d\n", m_Act_Skill );

	s.Printf( "HITPOINTS=%i\n", m_health);
	s.Printf( "STAMINA=%i\n", m_stam);
	s.Printf( "MANA=%i\n", m_mana);

	if ( m_NPC_Brain )
		s.Printf( "NPC=%i\n", m_NPC_Brain );
	if ( m_NPC_Speech )
		s.Printf( "SPEECH=%x\n", m_NPC_Speech );
	if ( m_owner.IsValidUID())
		s.Printf( "OWN=%x\n", (DWORD) m_owner );

	int j=0;
	for ( ;j<STAT_QTY;j++)
	{
		s.Printf( "%s=%i\n", Stat_Name[j], m_Stat[j] );
	}
	for ( j=0;j<SKILL_QTY;j++)
	{
		s.Printf( "SKILL%i=%i\n", j, m_Skill[j]);
	}
	s.Printf( "\n");

	// Write out my inventory here !!!
	WriteContent(s);
}

bool CChar :: LoadVal( const char * pKey, char * pVal )
{
	static const char * table[] =
	{
		"ACCOUNT",
		"P",

		"DIR",
		"ID",
		"XBODY",
		"XSKIN",
		"???",

		"FONT",
		"FLAGS",
		"ACTION",
		"TITLE",

		"HITPOINTS",
		"STAMINA",
		"MANA",

		"OWN",
		"NPC",
		"SPEECH",

		"ATT",
		"DEF",

	};

	switch ( FindTable( pKey, table, COUNTOF( table )))
	{
	case 0:	// "ACCOUNT",
		m_sAccount = pVal;
		return true;
	case 1: // "P"
		m_p.Read(pVal);
		MoveTo(m_p); 
		return true;
	case 2:	// "DIR",
		m_dir = (DIR_TYPE) atoi(pVal);
		return true;
	case 3:	// "ID"
		SetID( (CREID_TYPE) ahextoi(pVal));
		return true;
	case 4:	// "XBODY",
		m_prev_id = (CREID_TYPE) ahextoi(pVal);
		return true;
	case 5:	// "XSKIN",
		m_prev_color = ahextoi(pVal);
		return true;

	case 6:	// "???"
		return false;

	case 7:	// "FONT",
		m_fonttype=(FONT_TYPE)atoi(pVal);
		return true;
	case 8:	// "FLAGS",
		m_StatFlag = ahextoi(pVal);
		return true;
	case 9:	// "ACTION",
		m_Act_Skill = (SKILL_TYPE) atoi(pVal);
		return true;
	case 10:	// "TITLE"
		m_sTitle = pVal;
		return true;
	case 11:	// "HITPOINTS",
		m_health = atoi(pVal);
		return true;
	case 12: // "STAMINA",
		m_stam=atoi(pVal);
		return true;
	case 13:// "MANA",
		m_mana=atoi(pVal);
		return true;
	case 14:// "OWN",
		m_owner.SetUID( ahextoi(pVal));
		return true;
	case 15:// "NPC",
		m_NPC_Brain = (NPCBRAIN_TYPE) atoi(pVal);
		return true;
	case 16:// "SPEECH",
		m_NPC_Speech = ahextoi(pVal);
		return true;

	case 17:	// "ATT" from script
		((CCharBase *)m_pCre)->m_attack = GetRangeVal( pVal );
		return true;
	case 18:	// "DEF"
		((CCharBase *)m_pCre)->m_defense = GetRangeVal( pVal );
		return true;
	}

	if ( ! strnicmp( pKey, "SKILL", 5 ))
	{
		m_Skill[ atoi(&pKey[5]) ] = GetRangeVal( pVal );
		return true;
	}

	int i = FindTable( pKey, &(Skills[0].m_key), COUNTOF( Skills ), sizeof( Skills[0] ));
	if ( i >= 0 )
	{
		// Check some skill name.
		int iVal = GetRangeVal( pVal );
		if ( i == SKILL_QTY )
		{
			for ( i=0;i<SKILL_QTY;i++)
			{
				m_Skill[i]=iVal;
			}
		}	
		else
		{
			m_Skill[i]=iVal;
		}
		return true;
	}
	i = FindTable( pKey, Stat_Name, COUNTOF( Stat_Name ));
	if ( i >= 0 )
	{
		m_Stat[i] = GetRangeVal( pVal );
		if ( i <= STAT_DEX && m_Stat[i] <= 0 ) m_Stat[i] = 1;
		return true;
	}

	return( CObjBase :: LoadVal( pKey, pVal ));
}

void CChar :: Load( CScript & s ) // Load a character from script
{
	while ( 1 )
	{
		if ( ! s.ReadParse()) break;
		if ( ! strcmp( s.m_Line, "SERIAL" ))
		{
			SetPrivateUID( ahextoi(s.m_pArg), false );
		}
		else
		{
			LoadVal( s.m_Line, s.m_pArg );
		}
	}
	if ( m_UID == UID_UNUSED )	// Make sure we got a UID
	{
		SetPrivateUID( 0, false );
	}
	if ( m_NPC_Brain == NPCBRAIN_NONE )	// un-logged in player.
	{
		World.m_CharsIdle.InsertAfter( this );
	}
}

void CChar :: Create( const CEvent * pBin )
{
	SetID( ( pBin->Create.m_sex == 0 ) ? CREID_MAN : CREID_WOMAN );
	m_prev_id = (CREID_TYPE) GetID();
	SetName( pBin->Create.m_name );

	m_color = pBin->Create.m_color | 0x8000 ;
	if ( m_color < 0x83EA || m_color > 0x8422 )
	{
		m_color = 0x83EA;
	}
	m_prev_color = m_color;
	m_fonttype = FONT_NORMAL;

	MoveTo( Serv.m_Starts[pBin->Create.m_startloc].m_p );

	// randomize the skills first.
	int i = 0;
	for ( ; i < SKILL_QTY; i++ )
	{
		m_Skill[i] = GetRandVal( 256 );
	}

	m_health = m_Stat[STAT_STR] = pBin->Create.m_str + 1;
	m_stam   = m_Stat[STAT_DEX] = pBin->Create.m_dex + 1;
	m_mana   = m_Stat[STAT_INT] = pBin->Create.m_int + 1;

	m_Skill[pBin->Create.m_skill1] = pBin->Create.m_val1*10;
	m_Skill[pBin->Create.m_skill2] = pBin->Create.m_val2*10;
	m_Skill[pBin->Create.m_skill3] = pBin->Create.m_val3*10;

	// ??? Get special equip for the starting skills.

	m_NPC_Brain = NPCBRAIN_NONE;
	m_NPC_Speech = 0;

	ITEMID_TYPE id = (ITEMID_TYPE)(WORD) pBin->Create.m_hairid;
	if ( FindID( id, Item_Hair, COUNTOF(Item_Hair)) != -1 )
	{
		CItem * pHair = ItemCreateScript( id );
		pHair->m_color = pBin->Create.m_haircolor;
		if ( pHair->m_color<0x044E || pHair->m_color > 0x04AD )
		{
			pHair->m_color = 0x044E;
		}
		pHair->m_Attr |= ATTR_NEWBIE;
		ContentAdd( pHair );	// add content
	}

	id = (ITEMID_TYPE)(WORD) pBin->Create.m_beardid;
	if ( FindID( id, Item_Beards, COUNTOF(Item_Beards)) != -1 && GetID() == CREID_MAN )
	{
		CItem * pBeard = ItemCreateScript( id );
		pBeard->m_color = pBin->Create.m_beardcolor;
		if ( pBeard->m_color < 0x044E || pBeard->m_color > 0x04AD )
		{
			pBeard->m_color = 0x044E;
		}
		pBeard->m_Attr |= ATTR_NEWBIE;
		ContentAdd( pBeard );	// add content
	}

	// create the bank box.
	CContainerItem * pBankBox = dynamic_cast <CContainerItem *>( ItemCreateScript( ITEMID_BANK_BOX ));
	pBankBox->m_Attr |= ATTR_NEWBIE;
	LayerAdd( pBankBox, LAYER_BANKBOX );	

	CContainerItem * pPack = dynamic_cast <CContainerItem *>(ItemCreateScript( ITEMID_BACKPACK ));
	pPack->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pPack );	// add content

	CItem * pItem = ItemCreateScript( ITEMID_GOLD );
	pItem->SetAmount( 100 );
	pItem->m_Attr |= ATTR_NEWBIE;
	pPack->ContentAdd( pItem );	// add content
	
	pItem = ItemCreateScript( ITEMID_SPELLBOOK );
	pItem->m_Attr |= ATTR_NEWBIE;
	pItem->m_type = ITEM_RUNE;	// allow marking of it.
	pItem->m_spells1 = 0xFFFFFFFF; // all spells already in it.
	pItem->m_spells2 = 0xFFFFFFFF; // all spells already in it.
	pPack->ContentAdd( pItem );	// add content

static const WORD pantscolor[] = 
{
	0x0284,
	0x021F,
	0x03C3,
	0x03D9,
	0x02E8,
	0x02D1,
};

	pItem = ItemCreateScript( GetRandVal( 2 ) ? ITEMID_PANTS_FANCY : ITEMID_PANTS1 );
	pItem->m_color = pantscolor[ GetRandVal( COUNTOF(pantscolor)) ];
	pItem->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pItem );	// add content

static const WORD shirtcolor[] =
{
	0x0134,
	0x0028,
	0x0035,
	0x01CA,
	0x021A,
};

	pItem = ItemCreateScript( GetRandVal( 2 ) ? ITEMID_SHIRT1 : ITEMID_SHIRT_FANCY );
	pItem->m_color = shirtcolor[ GetRandVal( COUNTOF(shirtcolor)) ];
	pItem->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pItem );	// add content

	pItem = ItemCreateScript( ITEMID_SHOES );
	pItem->m_color = 0x0287;
	pItem->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pItem );	// add content

	pItem = ItemCreateScript( ITEMID_DAGGER );
	pItem->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pItem );	// add content

static const ITEMID_TYPE masks[] =
{ 
	ITEMID_HELM_BEAR,
	ITEMID_HELM_DEER,
	ITEMID_MASK_TREE,
	ITEMID_MASK_VOODOO,
	ITEMID_HAT_JESTER,
	//ITEMID_HELM_BONE,
	//ITEMID_HELM_ORC,
};

	pItem = ItemCreateScript( masks[ GetRandVal( COUNTOF( masks ))] );
	pItem->m_Attr |= ATTR_NEWBIE;
	ContentAdd( pItem );	// add content
}

void CChar :: CreateScript( int id )
{
	// Create an NPC from script.
	m_NPC_Brain = NPCBRAIN_ANIMAL;	// Must have a default brain.

	CString sSec;
	sSec.Format( "%X", id );
	if ( ! World.m_scrChars.FindSec( sSec ))
		return;

	CItem * pItem = NULL;
	while ( 1 )
	{
		if ( ! World.m_scrChars.ReadParse()) break;
		if ( ! strcmp("ITEM",World.m_scrChars.m_Line))
		{
			// Possible loot item.
			char * pArg = World.m_scrChars.m_pArg;
			int id = ahextoi(pArg);
			int amount = 1;
			if ( Parse( pArg, &pArg ))
			{
				if ( pArg[0] != 'R' )
				{
					amount = GetRangeVal( pArg );
					Parse( pArg, &pArg );
				}
				if ( pArg[0] == 'R' )
				{
					if ( GetRandVal( atoi( pArg+1 ))) 
						continue;
				}
			}
			pItem = ItemCreateScript( (ITEMID_TYPE) id );
			if ( pItem  == NULL ) break;
			pItem->SetAmount( amount );
			ContentAdd( pItem );
		}
		else if ( pItem != NULL )
		{
			pItem->LoadVal( World.m_scrChars.m_Line, World.m_scrChars.m_pArg );
		}
		else
		{
			LoadVal( World.m_scrChars.m_Line, World.m_scrChars.m_pArg );
		}
	}

	m_prev_id = GetID();
	m_prev_color = m_color;
}

bool CChar :: CanSee( const CObjBase * pObj ) const
{
	// Can I see item or char B ?
	// This should check line of sight ?
	// Am I blind ?

	if ( pObj->IsItem())
	{
		CItem * pItem = (CItem*) pObj;
		if ( ! IsPriv( PRIV_GM ))
		{
			if ( pItem->m_Attr & ATTR_INVIS )
				return( false );
		}
		CObjBase * pObjCont = pItem->GetContainer();
		if ( pObjCont != NULL ) 
		{
			return( CanSee( pObjCont ));			
		}
	}
	else
	{
		// Characters can be invisible, but not to GM's (true sight ?)
		if ( ! IsPriv( PRIV_GM ))
		{
			if ( ((CChar*) pObj )->m_StatFlag & ( STATF_Invisible | STATF_Hidden )) 
				return( false );
		}
	}

	return( GetDist( pObj ) <= pObj->GetVisualRange());
}

bool CChar :: CanTouch( const CObjBase * pObj ) const
{
	// Can I reach this from where i am. 
	// swords, spears, arms length = x units.
	// Use or Grab.
	// Check for blocking objects. 
	// It this in a container we can't get to ?

	if ( ! IsPriv( PRIV_GM ))
	{
		if ( m_StatFlag & ( STATF_DEAD | STATF_Sleeping ))
			return( false );	
		if ( GetDist( pObj->GetTopLevelObj() ) > 8 ) 
			return( false );
	}

	return( CanSee( pObj ));
}

bool CChar :: CanMove( const CItem * pItem ) const
{
	if ( IsPriv( PRIV_ALLMOVE | PRIV_DEBUG ))
		return( true );
	if ( pItem->m_Attr & ATTR_MOVE_NEVER ) 
		return( false );
	return( pItem->m_Attr & ATTR_MOVE_ALWAYS );
}

const char * CChar :: GetFameTitle() const
{
	if ( m_Stat[STAT_Fame] <= 9900 ) return( "" );
	if ( IsFemale( GetID())) return( "Lady " );
	return( "Lord " );
}

int CChar :: GetNotoLevel() const
{
	// Paperdoll title for character
	// This is so we can inform user of change in title !

	static const int KarmaLevel[] =
	{ 9900, 5000, 1000, 500, 100, -100, -500, -1000, -5000, -9900 };

	int i=0;
	for ( ; i<COUNTOF( KarmaLevel ) && m_Stat[STAT_Karma] < KarmaLevel[i]; i++ )
		;

	static const WORD FameLevel[] =
	{ 500, 1000, 5000, 9900 };

	int j =0;
	for ( ; j<COUNTOF( FameLevel ) && m_Stat[STAT_Fame] > FameLevel[j]; j++ )
		;

	return( ( i * 5 ) + j );
}

const char * CChar :: GetNotoTitle() const
{
	// Paperdoll title for character
	static const char * NotoTitles[5*11] = 
	{
		// Highest karma
		"Trustworthy",
		"Estimable",
		"Great",
		"Glorious",
		"Glorious",

		"Honest",
		"Commendable",
		"Famed",
		"Illustrious",
		"Illustrious",

		"Good",
		"Honorable",
		"Admirable",
		"Noble",
		"Noble",

		"Kind",
		"Respectable",
		"Proper",
		"Eminent",
		"Eminent",

		"Fair",
		"Upstanding",
		"Reputable",
		"Distinguished",
		"Distinguished",

		"",
		"Notable",
		"Prominent",
		"Renowned",
		"",

		"Rude",
		"Disreputable",
		"Notorious",
		"Imfamous",
		"Dishonored",

		"Unsavory",
		"Dishonorable",
		"Ignoble",
		"Sinister",
		"Sinister",

		"Scoundrel",
		"Malicious",
		"Vile",
		"Villainous",
		"Dark",

		"Despicable",
		"Dastardly",
		"Wicked",
		"Evil",
		"Evil",

		"Outcast",
		"Wretched",
		"Nefarious",
		"Dread",
		"Dread",
	};

	// Paperdoll title for character

	int iLevel = GetNotoLevel();

	const char * pTitle = NotoTitles[ iLevel ];

	char * pTemp = GetTempStr();
	sprintf( pTemp, "%s%s%s%s%s",
		(pTitle[0]) ? "The " : "",
		pTitle, 
		(pTitle[0]) ? " " : "",
		GetFameTitle(),
		GetName());

	return( pTemp );
}

const char * CChar :: GetTradeTitle() const // Paperdoll title for character p (2)
{
	// If NPC Brain type is one we recognize then use it instead.
	if ( m_NPC_Brain || m_NPC_Speech )
	{
		switch ( m_NPC_Brain )
		{
		case NPCBRAIN_HEALER:
			return( "the Healer" );
		case NPCBRAIN_GUARD:
			return( "the Guard" );
		}
	}

	if ( ! m_sTitle.IsEmpty())
		return( m_sTitle );

	char * pTemp = GetTempStr();

	// Incognito ?
	// If polymorphed then use the poly name.
	if (( m_StatFlag & STATF_Incognito ) || 
		( GetID() != CREID_MAN && GetID() != CREID_WOMAN ))
	{
		sprintf( pTemp, "the %s", m_pCre->m_name );
		return( pTemp );
	}
		
	int skill =  GetBestSkill();
	WORD x = m_Skill[skill];

	const char * pLevel;
	if (x>=980) pLevel="Grandmaster";
	else if (x>=900) pLevel="Master";
	else if (x>=800) pLevel="Adept";
	else if (x>=700) pLevel="Expert";
	else if (x>=600) pLevel="Journeyman";
	else if (x>=500) pLevel="Apprentice";
	else if (x>=400) pLevel="Novice";
	else if (x>=300) pLevel="Neophyte";
	else return( "" );

	sprintf( pTemp, "%s %s", pLevel, Skills[skill].m_title );
	return( pTemp );
}

SKILL_TYPE CChar :: GetWeaponSkill() const
{
	CItem * pWeapon = m_weapon.ItemFind();
	if ( pWeapon != NULL ) 
		return( pWeapon->GetWeaponSkill());
	return( SKILL_WRESTLING );
}

void CChar :: SetWeaponSwingTimer( bool fNewWeapon )
{
	// We have just equipped the weapon or gone into War mode.
	// Set the swing timer for the weapon or on us for .

	if ( ! ( m_StatFlag & STATF_War )) return;	// not in war mode anyhow.

	CItem * pWeapon = m_weapon.ItemFind();
	if ( pWeapon == NULL ) 
	{
		// Fists.

	}
	else
	{

	}

	// if ( fNewWeapon ) we can set the timer lower.
}

void CChar :: UpdateDrag( CItem * pItem, CObjBase * pCont, CPoint * pp )
{
	// Show the world that I am picking up or putting down this object.
#ifdef COMMENT
	CCommand cmd;
	cmd.DragAnim.m_Cmd = 0x23;
	cmd.DragAnim.m_id = pItem->GetID();
	cmd.DragAnim.m_unk3 = 0;
	cmd.DragAnim.m_unk5 = 0;
	cmd.DragAnim.m_amount = pItem->m_amount;

	if ( pCont != NULL )
	{
		// I'm putting an object in a cont..
		CObjBase * pObjTop = pCont->GetTopLevelObj();
		if ( pObjTop == this ) return;	// move stuff in my own pack.

		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = m_p.m_x;
		cmd.DragAnim.m_src_y = m_p.m_y;
		cmd.DragAnim.m_src_x = m_p.m_z;
		cmd.DragAnim.m_dstUID = pObjTop->GetUID();
		cmd.DragAnim.m_dst_x = pObjTop->m_p.m_x;
		cmd.DragAnim.m_dst_y = pObjTop->m_p.m_y;
		cmd.DragAnim.m_dst_z = pObjTop->m_p.m_z;
	}
	else if ( pp != NULL )
	{
		// putting on ground.
		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = m_p.m_x;
		cmd.DragAnim.m_src_y = m_p.m_y;
		cmd.DragAnim.m_src_x = m_p.m_z;
		cmd.DragAnim.m_dstUID = 0;
		cmd.DragAnim.m_dst_x = pp->m_x;
		cmd.DragAnim.m_dst_y = pp->m_y;
		cmd.DragAnim.m_dst_z = pp->m_z;
	}
	else
	{
		// I'm getting an object from where ever it is.

		CObjBase * pObjTop = pItem->GetTopLevelObj();
		if ( pObjTop == this ) return;	// move stuff in my own pack.

		cmd.DragAnim.m_srcUID = (pObjTop==pItem) ? 0 : pObjTop->GetUID();
		cmd.DragAnim.m_src_x = pObjTop->m_p.m_x;
		cmd.DragAnim.m_src_y = pObjTop->m_p.m_y;
		cmd.DragAnim.m_src_z = pObjTop->m_p.m_z;
		cmd.DragAnim.m_dstUID = GetUID();
		cmd.DragAnim.m_dst_x = m_p.m_x;
		cmd.DragAnim.m_dst_y = m_p.m_y;
		cmd.DragAnim.m_dst_x = m_p.m_z;
	}

	UpdateCanSee( &cmd, sizeof(cmd.DragAnim), m_pClient );
#endif

}

void CChar :: UpdateStats( STAT_TYPE type )
{
	// ??? Weight ? AC ? Gold ?
	if ( m_pClient != NULL )
	{
		if ( m_pClient->m_fPaused )
		{
			m_pClient->m_UpdateStats |= ( type + 1 );
			return;
		}
		m_pClient->m_UpdateStats = 0;
	}

	WORD iVal;
	WORD iMax = m_Stat[type];
	switch ( type )
	{
	case STAT_STR:	// 0
		iVal=m_health;
		break;
	case STAT_INT:	// 1
		iVal=m_mana;
		break;
	case STAT_DEX:	// 2
		iVal=m_stam;
		break;
	default:
		// I guess it's just some other type so update all my own stats.
		if ( m_pClient != NULL )
		{
			m_pClient->addStatWindow( GetUID());
		}
		return;
	}

	CCommand cmd;
	cmd.StatChng.m_Cmd = 0xA1 + type;
	cmd.StatChng.m_UID = GetUID();
	cmd.StatChng.m_max = iMax;
	cmd.StatChng.m_val = iVal;
	UpdateCanSee( &cmd, sizeof(cmd.StatChng));
}

void CChar :: UpdateAnimate( ANIM_TYPE action, bool fTranslate ) // NPC or character does a certain Animate
{
	// Translate the animation based on creature type.

	if ( fTranslate )
	{
	if ( m_weapon.IsValidUID() && action == ANIM_ATTACK_1H_WIDE )
	{
		// action depends on weapon type (skill) and 2 Hand type.
		CItem * pWeapon = m_weapon.ItemFind();
		ASSERT( pWeapon != NULL );
		switch ( pWeapon->m_type )
		{
		case ITEM_WEAPON_MACE:			// Can be used for smithing ?
		case ITEM_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
			action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
				ANIM_ATTACK_2H_DOWN :
				ANIM_ATTACK_1H_DOWN;
			break;
		case ITEM_WEAPON_SWORD:
			action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
				ANIM_ATTACK_2H_WIDE :
				ANIM_ATTACK_1H_WIDE;
			break;
		case ITEM_WEAPON_FENCE:
			action = ( pWeapon->m_pDef->m_layer == LAYER_HAND2 ) ?
				ANIM_ATTACK_2H_JAB :
				ANIM_ATTACK_1H_JAB;
			break;
		case ITEM_WEAPON_BOW:
			// normal or xbow ?
			action = ( pWeapon->IsSameID( ITEMID_BOW ) || pWeapon->IsSameID( ITEMID_BOW2 )) ?
				ANIM_ATTACK_BOW :
				ANIM_ATTACK_XBOW;
			break;
		}
	}

	if ( m_StatFlag & STATF_OnHorse )	// on horse back.
	{
		// Horse back anims are dif.
		switch ( action )
		{
		case ANIM_WALK_UNARM:
		case ANIM_WALK_ARM:
		case ANIM_WALK_WAR:
			action = ANIM_HORSE_RIDE_SLOW;
			break;
		case ANIM_RUN_UNARM:
		case ANIM_RUN_ARMED:
			action = ANIM_HORSE_RIDE_FAST;
			break;
		case ANIM_STAND:
		case ANIM_STAND_WAR_1H:
		case ANIM_STAND_WAR_2H:
		default:
			action = ANIM_HORSE_STAND; 
			break;
		case ANIM_CAST_DIR:
		case ANIM_ATTACK_1H_WIDE:
		case ANIM_ATTACK_1H_JAB:
		case ANIM_ATTACK_1H_DOWN:
		case ANIM_ATTACK_2H_JAB:
		case ANIM_ATTACK_2H_WIDE:
		case ANIM_ATTACK_UNARM:
			action = ANIM_HORSE_ATTACK; 
			break;
		case ANIM_CAST_AREA:
		case ANIM_ATTACK_BOW:
			action = ANIM_HORSE_ATTACK_BOW; 
			break;
		case ANIM_EAT:
		case ANIM_ATTACK_XBOW:
		case ANIM_SALUTE:
		case ANIM_BOW:
			action = ANIM_HORSE_ATTACK_XBOW; 
			break;
		case ANIM_ATTACK_2H_DOWN:
		case ANIM_TURN:
		case ANIM_FIDGET1:
		case ANIM_FIDGET2:
		case ANIM_GET_HIT:	
			action = ANIM_HORSE_SLAP; 
			break;
		}
	}
	else if ( GetID() < CREID_MAN )
	{
		// Animals have certain anims. Monsters have others.

		if ( GetID() >= CREID_HORSE1 )
		{
			// All animals have all these anims thankfully
			switch ( action )
			{
			case ANIM_WALK_UNARM:
			case ANIM_WALK_ARM:
			case ANIM_WALK_WAR:
				action = ANIM_ANI_WALK;
				break;
			case ANIM_RUN_UNARM:
			case ANIM_RUN_ARMED:
				action = ANIM_ANI_RUN;
				break;
			case ANIM_STAND:
			case ANIM_STAND_WAR_1H:
			case ANIM_STAND_WAR_2H:
			default:
				action = ANIM_ANI_STAND;
				break;

			case ANIM_FIDGET1:
				action = ANIM_ANI_FIDGET1;
				break;
			case ANIM_FIDGET2:
				action = ANIM_ANI_FIDGET2;
				break;
			case ANIM_CAST_DIR:
				action = ANIM_ANI_ATTACK1;
				break;
			case ANIM_CAST_AREA:
				action = ANIM_ANI_ATTACK2;
				break;
			case ANIM_GET_HIT:
				action = ANIM_ANI_ATTACK3;
				break;

			case ANIM_ATTACK_1H_WIDE:
			case ANIM_ATTACK_1H_JAB:
			case ANIM_ATTACK_1H_DOWN:
			case ANIM_ATTACK_2H_DOWN:
			case ANIM_ATTACK_2H_JAB:
			case ANIM_ATTACK_2H_WIDE:
			case ANIM_ATTACK_BOW:
			case ANIM_ATTACK_XBOW:
			case ANIM_ATTACK_UNARM:
				switch ( GetRandVal(3))
				{
				case 0: action = ANIM_ANI_ATTACK1; break;
				case 1: action = ANIM_ANI_ATTACK2; break;
				case 2: action = ANIM_ANI_ATTACK3; break;
				}
				break;

			case ANIM_DIE_BACK:
				action = ANIM_ANI_DIE1;
				break;
			case ANIM_DIE_FORWARD:
				action = ANIM_ANI_DIE2;
				break;
			case ANIM_TURN:
			case ANIM_BOW:
			case ANIM_SALUTE:
				action = ANIM_ANI_SLEEP;
				break;
			case ANIM_EAT:
				action = ANIM_ANI_EAT;
				break;
			}
		}
		else 
		{
			// NOTE: Available actions depend HEAVILY on creature type !
			// ??? Monsters don't have all anims in common !
			// translate these !
			switch ( action )
			{
			case ANIM_CAST_DIR: 
				action = ANIM_MON_MISC_BREATH; 
				break;
			case ANIM_CAST_AREA:
				action = ANIM_MON_MISC_STOMP; 
				break;
			case ANIM_DIE_BACK: 
				action = ANIM_MON_DIE1; 
				break;
			case ANIM_DIE_FORWARD: 
				action = ANIM_MON_DIE2; 
				break;
			case ANIM_GET_HIT:	
				switch ( GetRandVal(4))
				{
				case 0: action = ANIM_MON_GETHIT1; break;
				case 1: action = ANIM_MON_GETHIT2; break;
				case 2: action = ANIM_MON_GETHIT3; break;
				case 3: action = ANIM_MON_STUMBLE; break;
				}
				break;
			case ANIM_ATTACK_1H_WIDE:
			case ANIM_ATTACK_1H_JAB:
			case ANIM_ATTACK_1H_DOWN:
			case ANIM_ATTACK_2H_DOWN:
			case ANIM_ATTACK_2H_JAB:
			case ANIM_ATTACK_2H_WIDE:
			case ANIM_ATTACK_BOW:
			case ANIM_ATTACK_XBOW:
			case ANIM_ATTACK_UNARM:
				switch ( GetRandVal(3))
				{
				case 0: action = ANIM_MON_ATTACK1; break;
				case 1: action = ANIM_MON_ATTACK2; break;
				case 2: action = ANIM_MON_ATTACK3; break;
				}
				break;
			default:			
				action = ANIM_WALK_UNARM; 
				break;
			}
		}
	}
	}

	CCommand cmd;
	cmd.CharAction.m_Cmd = 0x6E;
	cmd.CharAction.m_UID = GetUID();
	cmd.CharAction.m_action = action;
	cmd.CharAction.m_zero7 = 0;
	cmd.CharAction.m_dir = m_dir;
	cmd.CharAction.m_repeat = 1;
	cmd.CharAction.m_zero11 = 0;
	cmd.CharAction.m_repflag = 0;
	cmd.CharAction.m_framedelay = 1;
	UpdateCanSee( &cmd, sizeof(cmd.CharAction));
}

void CChar :: UpdateMode(  CClient * pExcludeClient )
{
	// If character status has been changed
	// (Polymorph, war mode or hide), resend him
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pExcludeClient == pClient ) continue;
		if ( ! pClient->CanSee( this )) continue;
		if ( pClient->IsPriv( PRIV_DEBUG )) continue;
		pClient->addCharMove( this );
	}
}

void CChar :: UpdateMove( CPoint pold, CClient * pExcludeClient )
{
	// Who now sees this char ?
	// Did they just see him move ?
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pExcludeClient ) continue;	// no need to see self move.
		if ( pClient == m_pClient )
		{
			pClient->addPlayerView( pold );
			continue;
		}
		if ( pClient->m_pChar == NULL ) continue;

		bool fCouldSee = ( 
			abs( pold.m_x - pClient->m_pChar->m_p.m_x ) <= UO_MAP_VIEW_SIZE && 
			abs( pold.m_y - pClient->m_pChar->m_p.m_y ) <= UO_MAP_VIEW_SIZE );

		if ( ! pClient->CanSee( this )) 
		{	// can't see me now.
			if ( fCouldSee ) pClient->addObjectRemove( this );
		}
		else if ( fCouldSee )
		{	// They see me move.
			pClient->addCharMove( this );
		}
		else
		{	// first time this client has seen me.
			pClient->addChar( this );
		}
	}
}

void CChar :: UpdateDir( CObjBase * pObj )
{
	pObj = pObj->GetTopLevelObj();
	if ( pObj == this )		// In our own pack.
		return;
	DIR_TYPE dir = GetDir( pObj );
	if ( dir != m_dir )
	{
		m_dir = dir;	// face victim.
		UpdateMove(m_p);
	}
}

void CChar :: Update( CClient * pClientExclude ) // If character status has been changed (Polymorph), resend him
{
	// Or I changed looks.
	// I moved or somebody moved me  ?
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude ) continue;
		if ( pClient == m_pClient )
		{	
			pClient->addReSync();
		}
		else if ( pClient->CanSee( this ))
		{
			pClient->addChar( this );
		}
	}
}

void CChar :: SoundChar( CRESND_TYPE type )
{
	WORD id = 0;

	switch ( type )
	{
	case CRESND_RAND1:	id = m_pCre->m_sound1; break;
	case CRESND_RAND2:	id = m_pCre->m_sound2; break;
	case CRESND_HIT:	id = m_pCre->m_sound_hit; break;
	case CRESND_GETHIT:	id = m_pCre->m_sound_gethit; break;
	case CRESND_DIE:	id = m_pCre->m_sound_die; break;
	}

	if ( GetID() >= CREID_MAN && GetID() != CREID_BLADES ) 
	{

static const WORD Snd_Man_Die[] = { 0x15a, 0x15b, 0x15c, 0x15d };
static const WORD Snd_Man_Omf[] = { 0x154, 0x155, 0x156, 0x157, 0x158, 0x159 };
static const WORD Snd_Wom_Die[] = { 0x150, 0x151, 0x152, 0x153 };
static const WORD Snd_Wom_Omf[] = { 0x14b, 0x14c, 0x14d, 0x14e, 0x14f };

		if ( IsFemale( GetID() ))
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = Snd_Wom_Omf[ GetRandVal( COUNTOF(Snd_Wom_Omf)) ];
				break;
			case CRESND_DIE:
				id = Snd_Wom_Die[ GetRandVal( COUNTOF(Snd_Wom_Die)) ];
				break;
			}
		}
		else
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = Snd_Man_Omf[ GetRandVal( COUNTOF(Snd_Man_Omf)) ];
				break;
			case CRESND_DIE:
				id = Snd_Man_Die[ GetRandVal( COUNTOF(Snd_Man_Die)) ];
				break;
			}
		}
	}

	if ( type == CRESND_HIT && m_weapon.IsValidUID())
	{
		CItem * pWeapon = m_weapon.ItemFind();
		ASSERT( pWeapon != NULL );

		// ??? weapon type strike noise based on type of weapon and how hard hit.
		id = 0x232;	// bolt 0x223 = bolt miss or dart ?
	}

	if ( ! id ) return;
	Sound( id );
}

void CChar :: SysMessage( const char * pMsg ) const
{
	// Push a message back to the client if there is one.

	if ( m_pClient == NULL ) return;
	m_pClient->addSysMessage( pMsg );
}

//----------------------------------------------------------------------
// Skills

SKILL_TYPE CChar :: GetBestSkill() const // Which skill is the highest for character p
{
	SKILL_TYPE best_skill = SKILL_QTY;
	int best_value=-1;
	for ( int i=0;i<SKILL_QTY;i++) 
		if ( m_Skill[i] > best_value )
		{
			best_skill = (SKILL_TYPE) i;
			best_value = m_Skill[i];
		}
	return best_skill;
}

void CChar :: Skill_Experience( SKILL_TYPE skill, int difficulty )
{
	// Give the char credit for using the skill.
	// More credit for the more difficult. or none if too easy
	if ( skill > SKILL_QTY ) 
		return;
	if ( difficulty/2 > m_Skill[skill] )	// way too hard for you
		return;
	m_Skill[skill] ++;
	if ( m_pClient != NULL )
	{
		// Update the skills list
		m_pClient->addSkillWindow( skill );
	}
}

bool CChar :: Skill_CheckSuccess( SKILL_TYPE skill, int difficulty )
{
	// Check a skill for success or fail.
	if ( skill > SKILL_QTY ) 
		return( true );
	if ( m_Skill[ skill ] < GetRandVal( difficulty )) 
		return( false );	// need some skill here.
	return( true );
}

bool CChar :: Skill_UseQuick( SKILL_TYPE skill, int difficulty )
{
	// Use a skill instantly. No wait at all.
	// No interference with other skills.
	if ( ! Skill_CheckSuccess( skill, difficulty ))
	{
		Skill_Experience( skill, difficulty/2 );
		return( false );
	}
	Skill_Experience( skill, difficulty );
	return( true );
}

void CChar :: Skill_Cleanup( void )
{
	// We are done with the skill.
	// We may have succeeded, failed, or cancelled.
	m_Act_Skill = SKILL_NONE;
	m_Act_Spell = SPELL_NONE;
	m_Act_Arg = 0;
	m_Act_Level = 0;
	// m_Act_Targ = 0

	SetTimeout( 1 );		// we should get a brain tick next time.
}

void CChar :: Skill_Fail( void )
{
	// We still get some credit for having tried.

	if ( m_Act_Skill == SKILL_NONE || m_Act_Skill > SKILL_QTY ) 
		return;

	const char * pMsg = Skills[ m_Act_Skill ].m_failmsg;
	switch ( m_Act_Skill )
	{
	case SKILL_ALCHEMY:
		Sound( 0x031 );
		break;
	case SKILL_ANATOMY:
	case SKILL_ANIMALLORE:
	case SKILL_ITEMID:
	case SKILL_ARMSLORE:
	case SKILL_PARRYING:
	case SKILL_BEGGING:
	case SKILL_BLACKSMITHING:
	case SKILL_BOWCRAFT:
	case SKILL_PEACEMAKING:
	case SKILL_CAMPING:
	case SKILL_CARPENTRY:
	case SKILL_CARTOGRAPHY:
	case SKILL_COOKING:
	case SKILL_DETECTINGHIDDEN:
	case SKILL_ENTICEMENT:
	case SKILL_EVALINT:
	case SKILL_HEALING:
	case SKILL_FISHING:
	case SKILL_FORENSICS:
	case SKILL_HERDING:	// 20
	case SKILL_HIDING:
	case SKILL_PROVOCATION:
	case SKILL_INSCRIPTION:
	case SKILL_LOCKPICKING:
		break;
	case SKILL_MAGERY:
		Effect( 3, ITEMID_FX_SPELL_FAIL, this, 0, 30 );
		Sound( 0x05C );
		break;
	case SKILL_MAGICRESISTANCE:
	case SKILL_TACTICS:
		break;
	case SKILL_SNOOPING:
		break;
	case SKILL_MUSICIANSHIP:
		break;
	case SKILL_POISONING:	// 30
		break;
	case SKILL_ARCHERY:
		break;
	case SKILL_SPIRITSPEAK:
		break;
	case SKILL_STEALING:
		break;
	case SKILL_TAILORING:
		break;
	case SKILL_TAMING:
		break;
	case SKILL_TASTEID:
		break;
	case SKILL_TINKERING:
	case SKILL_TRACKING:
	case SKILL_VETERINARY:
	case SKILL_SWORDSMANSHIP:	// 40
	case SKILL_MACEFIGHTING:
	case SKILL_FENCING:
	case SKILL_WRESTLING:
	case SKILL_LUMBERJACKING:
	case SKILL_MINING:
		break;
	}
	if ( pMsg != NULL ) SysMessage( pMsg );

	// Get some experience for failure ?
	Skill_Experience( m_Act_Skill, m_Act_Level/3 );

	Skill_Cleanup();
}

void CChar :: Skill_Done()	
{
	// We just finished using a skill. ASYNC timer expired.
	// m_Act_Skill = the skill.
	// Consume resources that have not already been consumed.
	// Confer the benefits of the skill.
	// calc skill gain based on this.
	// ex. magery skill goes up FAR less if we use a scroll or magic device !
	// Did we succeed or fail ?

	if ( m_Act_Skill == SKILL_NONE || m_Act_Skill >= SKILL_QTY )
		return;

	// Advance the skill 
	Skill_Experience( m_Act_Skill, m_Act_Level );

	// Success for the skill.
	switch ( m_Act_Skill )
	{
	case SKILL_TRACKING:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL ) break;
			DIR_TYPE dir = GetDir( pChar );
			if ( m_pClient != NULL )
			{
				
			}
		}
		return;		// keep the skill active.

	case SKILL_DETECTINGHIDDEN:
		// Look around for who is hiding.
		// Detect them based on skill diff.
		// ??? Hidden objects ?
		{
			CWorldSearch Area( m_p, ( m_Skill[ SKILL_DETECTINGHIDDEN ] / 8 ) + 1 );
			CChar * pCharNext;
			for ( CChar * pChar = Area.GetChar(); pChar != NULL; pChar = pCharNext )
			{
				pCharNext = Area.GetChar();
				if ( pChar == this ) continue;
				if ( ! ( pChar->m_StatFlag & ( STATF_Invisible | STATF_Hidden )))
					continue;
				// Try to detect them.
				if ( pChar->m_StatFlag & STATF_Hidden )
				{
					// If there hiding skill is much better than our detect then stay hidden
				}
				pChar->Reveal(STATF_Invisible|STATF_Hidden);
				CString sMsg;
				sMsg.Format( "You find %s", pChar->GetName());
				SysMessage( sMsg );
			}
		}
		break;

	case SKILL_MAGERY:
		Spell_Done();
		break;
	case SKILL_MACEFIGHTING:
	case SKILL_FENCING:
	case SKILL_WRESTLING:
	case SKILL_ARCHERY:

		if ( m_Act_Targ.IsValidUID() && ( m_StatFlag & STATF_War ))
		{
			if ( Hit( m_Act_Targ.CharFind() ))
				return;

			// Might be dead ? Clear this.
			Attack( NULL );
		}
		break;

	case SKILL_HIDING:
		if ( m_StatFlag & ( STATF_Invisible | STATF_Hidden ))
		{
			m_StatFlag &= ~( STATF_Invisible | STATF_Hidden );
		}
		else
		{
			SysMessage( "You have hidden yourself well" );
			m_StatFlag |= STATF_Hidden;
		}
		UpdateMode();
		break;

	case SKILL_TAMING:
		{
			CChar * pChar = m_Act_Targ.CharFind();
			if ( pChar == NULL ) break;
			pChar->m_owner = GetUID();
			pChar->Skill_Setup( NPCACT_FOLLOW_OWN );
			SysMessage( "It seems to accept you as master" );
		}
		break;

	case SKILL_FISHING:
		{
			static const WORD Fish[] = { ITEMID_FISH_1, ITEMID_FISH_2, ITEMID_FISH_3, ITEMID_FISH_4 };

			CItem * pItem = ItemCreateScript( (ITEMID_TYPE) Fish[ GetRandVal( COUNTOF( Fish )) ] );
			pItem->PutOnGround( m_p );	// put at my feet.

			// Create the little splash effect.
			Sound( 0x027 );
			pItem = ItemCreateBase( ITEMID_FX_SPLASH );
			pItem->SetTimeout( 3 );
			pItem->PutOnGround( m_Act_p );

			SysMessage( "You pull out a nice fish!" );
		}
		break;

	case SKILL_SPIRITSPEAK:
		Sound( 0x24a );
		break;
	}

	Skill_Cleanup();
}

bool CChar :: Skill_Start( int iDifficulty )
{
	// We have all the info we need to do the skill. (targeting, dificulty etc)
	// Set up how long we have to wait before we get the desired results from this skill.
	// Set up any animations/sounds in the mean time.
	// Calc if we will succeed or fail.
	// RETURN: false = failed outright with no wait. "You have not chance of taming this"

	if ( m_Act_Skill == SKILL_NONE ) return( false );

	m_Act_Level = iDifficulty;

	bool fSuccess = Skill_CheckSuccess( m_Act_Skill, iDifficulty );
	if ( ! fSuccess )
	{
		// ??? Fail immediatly. we should pause this as well.
		Skill_Fail();
		return( false );
	}

	WORD wWaitTime = 0;
	switch ( m_Act_Skill )
	{
	case SKILL_MACEFIGHTING:
	case SKILL_FENCING:
	case SKILL_WRESTLING:
	case SKILL_ARCHERY:
		// When do we get our next shot?
		wWaitTime = (100-m_Stat[STAT_DEX]) / 25 + 1;
		break;
	case SKILL_MAGERY:
		// Casting time goes up with difficulty 
		// but down with skill, int and dex
		wWaitTime = (100-iDifficulty) / 25 + 1;
		break;
	case SKILL_TAMING:
		// Check the min required skill for this creature.
		break;
	case SKILL_HIDING:
		// Skill required varies with terrain and situation ?
		break;
	case SKILL_FISHING:
		{
		UpdateAnimate( ANIM_ATTACK_2H_DOWN );
		Sound( 0x027 );
		CItem * pItem = ItemCreateBase( ITEMID_FX_SPLASH );
		pItem->SetTimeout( 3 );
		pItem->PutOnGround( m_Act_p );
		}
		break;
	case SKILL_INSCRIPTION:
		Sound( 0x249 );
		break;
	case SKILL_ALCHEMY:
		Sound( 0x243 );
		break;
	case SKILL_CARPENTRY:
		break;

	// default:
		// return( true );
	}

	// How long before complete skill.
	SetTimeout( wWaitTime );
	return( true );
}

bool CChar :: Skill_Setup( SKILL_TYPE sk )
{
	// We are starting to use a skill.
	// We can do nothing else during this time else the skill is interrupted.
	// Do things like target the item or select from menu after this.
	//
	if ( m_Act_Skill != SKILL_NONE )
	{
		Skill_Fail();	// Fail previous skill unfinished.
	}

	m_Act_Skill = sk;	// Start using a skill.

	// Some skill can start right away. Need no targetting.
	int iDifficulty = 1;
	switch ( sk )
	{
	case SKILL_HIDING:
		// Difficulty depends on terrain.
		iDifficulty = 10;
		break;
	case SKILL_SPIRITSPEAK:
		// difficulty based on spirits near ?
		iDifficulty = 10;
		break;
	case SKILL_PEACEMAKING:
		// Find musical inst first.

		// Basic skill check.
		if ( ! Skill_UseQuick( SKILL_MUSICIANSHIP, 10 ))
		{
			SysMessage( "You play poorly" );
			return( false );
		}

		// Who is fighting around us ? determines difficulty.
		iDifficulty = 10;
		break;
	case SKILL_DETECTINGHIDDEN:
		// Based on who is hiding ?
		iDifficulty = 10;
		break;
	default:
		if ( sk <= SKILL_QTY )	// Need client targetting.
			return( true );
		break;
	}

	// No targetting required do it now.
	return( Skill_Start( iDifficulty ));
}

//----------------------------------------------------------------------
// Spells 

void CChar :: Spell_Dispel()
{
	// remove all the spells
	for ( int i=LAYER_SPELL_STATS; i<= LAYER_SPELL_Polymorph; i++ )
	{
		CItem * pSpell = LayerFind( (LAYER_TYPE) i );
		if ( pSpell == NULL ) continue;
		delete pSpell;
	}
	Update();
}

void CChar :: Spell_Polymorph( CREID_TYPE id )
{
	SetID( id );
	// ??? Sound();
	Update();		// show everyone I am now a new type
}

void CChar :: Spell_Teleport( CPoint p )
{
	// Teleport you to this place.
	if ( ! p.IsValid()) return;
	if ( m_pClient != NULL )
	{
		m_pClient->addPause( true );
	}

#if 1
	CItem * pItem = ItemCreateBase( ITEMID_FX_TELE_VANISH );
	pItem->SetTimeout( 2 );
	pItem->PutOnGround( m_p );
#else
	Effect( 2, ITEMID_FX_TELE_VANISH, this );
#endif

	CPoint pold = m_p;
	MoveTo( p );
	UpdateMove( pold );

	Sound( Spells[ SPELL_Teleport ].m_sound );	// 0x01fe
	Effect( 3, ITEMID_FX_TELE_VANISH, this, 9, 6 );
}

void CChar :: Spell_Summon( int id, CPoint p )
{
	if ( ! id ) return;
	CChar * pChar;
	if ( id < 0 )
	{
		// More specific npc type from script ?
		pChar = CharCreateScript( -id );
		if ( pChar == NULL ) return;
	}
	else
	{
		// These type summons make me it's master.
		pChar = CharCreateScript( (CREID_TYPE) id );
		if ( pChar == NULL ) return;
		pChar->m_owner = GetUID();
		pChar->m_StatFlag |= STATF_Conjured;
	}
		
	pChar->MoveTo( p );
	pChar->Update();
	pChar->UpdateAnimate( ANIM_CAST_DIR );
	pChar->SoundChar( CRESND_GETHIT );
}

void CChar :: Spell_Resurrection()
{
	if ( ! ( m_StatFlag & STATF_DEAD )) return;

	SetID( m_prev_id );
	m_color = m_prev_color;
	m_health = m_Stat[STAT_STR];
	m_StatFlag &= ~STATF_DEAD;

	CItem * pRobe = ContentFind( ITEMID_DEATHSHROUD );
	if ( pRobe != NULL )
	{
		pRobe->SetID( ITEMID_ROBE );
		pRobe->SetName( "Resurrection Robe" );
		pRobe->Update();
	}
	Effect( 3, ITEMID_FX_HEAL_EFFECT, this, 9, 14 );
	Update();

	// ??? If they are standing on there own corpse then res the corpse !
}

void CChar :: Spell_Effect_Remove( CItem * pSpell )
{
	// we are removing the spell effect.
	if ( ! ( pSpell->m_Attr & ATTR_MAGIC )) return;

	if ( pSpell->GetEquipLayer() == LAYER_SPELL_STATS )
	{
		UpdateStats();
	}

	switch ( pSpell->m_item_spell )
	{
	case SPELL_Clumsy:
		m_Stat[ STAT_DEX ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Feeblemind:
		m_Stat[ STAT_INT ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Weaken:
		m_Stat[ STAT_STR ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Agility:
		m_Stat[ STAT_DEX ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Cunning:
		m_Stat[ STAT_INT ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Strength:
		m_Stat[ STAT_STR ] -= pSpell->m_item_magiclevel;
		break;

	case SPELL_Reactive_Armor:
		m_StatFlag &= ~STATF_Reactive;
		break;
	case SPELL_Night_Sight:
		m_StatFlag &= ~STATF_NightSight;
		if ( m_pClient != NULL )
		{
			m_pClient->addLight();
		}
		break;
	case SPELL_Magic_Reflect:
		m_StatFlag &= ~STATF_Reflection;
		break;
	case SPELL_Bless:
		m_Stat[ STAT_DEX ] -= pSpell->m_item_magiclevel;
		m_Stat[ STAT_INT ] -= pSpell->m_item_magiclevel;
		m_Stat[ STAT_STR ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Curse:
	case SPELL_Mass_Curse:
		m_Stat[ STAT_DEX ] += pSpell->m_item_magiclevel;
		m_Stat[ STAT_INT ] += pSpell->m_item_magiclevel;
		m_Stat[ STAT_STR ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Poison:
		m_StatFlag &= ~STATF_Poisoned;
		break;
	case SPELL_Protection:
	case SPELL_Arch_Prot:
		m_defense -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Incognito:
		m_StatFlag &= ~STATF_Incognito;
		break;
	case SPELL_Invis:
		Reveal(STATF_Invisible);
		break;
	case SPELL_Paralyze:
		m_StatFlag &= ~STATF_Freeze;
		break;
	case SPELL_Polymorph:
		if (( m_StatFlag & STATF_Polymorph ) || m_prev_id != GetID())
		{
			// poly back to orig form.
			Spell_Polymorph( m_prev_id );
			m_StatFlag &= ~STATF_Polymorph;
		}
		break;
	}
}

void CChar :: Spell_Effect_Add( SPELL_TYPE spell, LAYER_TYPE layer, int iLevel, int iDuration )
{
	// Attach the spell effect for a duration.

	CItem * pSpell = ItemCreateBase( Spells[ spell ].m_SpellID );
	pSpell->m_type = ITEM_SPELL;
	pSpell->m_Attr |= ATTR_MAGIC | ATTR_NEWBIE;
	pSpell->m_item_spell = spell;
	pSpell->m_item_magiclevel = 10; // iLevel;
	pSpell->m_item_charges = 1;
	pSpell->SetTimeout( iDuration );
	LayerAdd( pSpell, layer );	// Remove any competing effect first.

	switch ( spell )
	{
	case SPELL_Clumsy:
		m_Stat[ STAT_DEX ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Feeblemind:
		m_Stat[ STAT_INT ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Weaken:
		m_Stat[ STAT_STR ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Agility:
		m_Stat[ STAT_DEX ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Cunning:
		m_Stat[ STAT_INT ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Strength:
		m_Stat[ STAT_STR ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Reactive_Armor:
		m_StatFlag |= STATF_Reactive;
		break;
	case SPELL_Night_Sight:
		m_StatFlag |= STATF_NightSight;
		if ( m_pClient != NULL )
		{
			m_pClient->addLight();
		}
		break;
	case SPELL_Magic_Reflect:
		m_StatFlag |= STATF_Reflection;
		break;
	case SPELL_Bless:
		m_Stat[ STAT_DEX ] += pSpell->m_item_magiclevel;
		m_Stat[ STAT_INT ] += pSpell->m_item_magiclevel;
		m_Stat[ STAT_STR ] += pSpell->m_item_magiclevel;
		break;
	case SPELL_Curse:
	case SPELL_Mass_Curse:
		m_Stat[ STAT_DEX ] -= pSpell->m_item_magiclevel;
		m_Stat[ STAT_INT ] -= pSpell->m_item_magiclevel;
		m_Stat[ STAT_STR ] -= pSpell->m_item_magiclevel;
		break;
	case SPELL_Poison:
		m_StatFlag |= STATF_Poisoned;
		break;
	case SPELL_Protection:
	case SPELL_Arch_Prot:
		m_defense += pSpell->m_item_magiclevel;
		break;
	case SPELL_Incognito:
		m_StatFlag |= STATF_Incognito;
		break;
	case SPELL_Invis:
		m_StatFlag |= STATF_Invisible;
		UpdateMove(m_p);
		break;
	case SPELL_Paralyze:
		m_StatFlag |= STATF_Freeze;
		break;
	case SPELL_Polymorph:
		m_StatFlag |= STATF_Polymorph;
		break;
	}
	if ( layer == LAYER_SPELL_STATS )
	{
		UpdateStats();
	}
}

void CChar :: Spell_Effect( SPELL_TYPE spell, CChar * pCharSrc, int iLevel )
{
	// Spell has a direct effect on this char.
	// This should effect noto of source.

	if ( this == NULL )
		return;
	if ( IsItem())
		return;
	if ( iLevel <= 0 )	// spell died (fizzled?).
		return;

	if ( Spells[spell].m_wFlags & SPELLFLAG_HARM )
	{
		// Check resistance to magic ?
		if ( Skill_UseQuick( SKILL_MAGICRESISTANCE, ((spell/8)+1) * 10 ))
		{
			SysMessage( "You feel yourself resisting magic" );
			iLevel /= 2;	// ??? reduce effect of spell.
		}

		// Check magic reflect.
		if ( m_StatFlag & STATF_Reflection )	// reflected.
		{
			m_StatFlag &= ~STATF_Reflection;
			Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
			if ( pCharSrc != NULL )
			{
				pCharSrc->Spell_Effect( spell, NULL, iLevel/2 );
			}
			return;
		}
	}

	switch ( spell )
	{

	case SPELL_Clumsy:
	case SPELL_Feeblemind:
	case SPELL_Weaken:
		Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_STATS, iLevel, 120 );
		break;

	case SPELL_Great_Heal:
	case SPELL_Heal:
		Effect( 3, ITEMID_FX_HEAL_EFFECT, this, 9, 14 );
		m_health = m_Stat[STAT_STR];
		UpdateStats(STAT_STR);
		break;

	case SPELL_Night_Sight:
		Effect( 3, ITEMID_FX_HEAL_EFFECT, this, 9, 6 );
		Spell_Effect_Add( spell, LAYER_SPELL_Night_Sight, iLevel, 120 );
		break;

	case SPELL_Reactive_Armor:
		Effect( 3, ITEMID_FX_HEAL_EFFECT, this, 9, 6 );
		Spell_Effect_Add( spell, LAYER_SPELL_Reactive, iLevel, 10000 );
		break;

	case SPELL_Harm:
		TakeHit( (10 + GetRandVal(5)), pCharSrc );
		Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 9, 6 );
		break;

	case SPELL_Agility:
	case SPELL_Cunning:
	case SPELL_Strength:
		Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_STATS, iLevel, 120 );
		break;

	case SPELL_Magic_Reflect:
		Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_Magic_Reflect, iLevel, 120 );
		break;

	case SPELL_Bless:
		Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_STATS, iLevel, 120 );
		break;

	case SPELL_Curse:
	case SPELL_Mass_Curse:
		Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_STATS, iLevel, 120 );
		break;

	case SPELL_Poison:
		Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_Poison, iLevel, 120 );
		break;

	case SPELL_Cure:	
	case SPELL_Arch_Cure:
		Effect( 3, ITEMID_FX_HEAL_EFFECT, this, 9, 14 );
		m_StatFlag &= ~( STATF_Poisoned );
		Update();
		break;

	case SPELL_Protection:
	case SPELL_Arch_Prot:
		Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_Protection, iLevel, 120 );
		break;

	case SPELL_Dispel:
	case SPELL_Mass_Dispel:
		Spell_Dispel();
		break;

	case SPELL_Reveal:
		if ( ! Reveal(STATF_Invisible|STATF_Hidden)) break;
		Effect( 3, ITEMID_FX_BLESS_EFFECT, this, 0, 15 );
		break;

	case SPELL_Invis:
		Spell_Effect_Add( spell, LAYER_SPELL_Invis, iLevel, 120 );
		break;

	case SPELL_Incognito:
		Spell_Effect_Add( spell, LAYER_SPELL_Incognito, iLevel, 120 );
		break;

	case SPELL_Paralyze:
		Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 0, 15 );
		Spell_Effect_Add( spell, LAYER_SPELL_Paralyze, iLevel, 120 );
		break;

	case SPELL_Explosion:
		Effect( 3, ITEMID_FX_EXPLODE_3, this, 9, 6 );
		TakeHitArmor( (15 + GetRandVal(15)), pCharSrc );
		break;

	case SPELL_Magic_Arrow:
		TakeHitArmor( (10 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Fireball:
		TakeHitArmor( (15 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Energy_Bolt:
		TakeHit( (10 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Meteor_Swarm:
		Effect( 3, ITEMID_FX_EXPLODE_3, this, 9, 6 );
		TakeHitArmor( (20 + GetRandVal(15)), pCharSrc );
		break;
	case SPELL_Earthquake:
		TakeHitArmor( (15 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Lightning:
		Effect( 1, ITEMID_NOTHING, pCharSrc );
		TakeHit( (20 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Chain_Lightning:
		Effect( 1, ITEMID_NOTHING, pCharSrc );
		TakeHit( (15 + GetRandVal(15)), pCharSrc );
		break;
	case SPELL_Flame_Strike:
		// Burn whoever is there.
		TakeHit( (15 + GetRandVal(10)), pCharSrc );
		break;
	case SPELL_Resurrection:
		Spell_Resurrection();
		break;
	}
}

void CChar :: Spell_Bolt( CObjBase * pObj, ITEMID_TYPE idBolt )
{
	// I am casting a bolt spell.
	if ( pObj == NULL ) return;
	pObj->Effect( 0, idBolt, this, 5, 1, true );

	// Take damage !
	if ( pObj->IsItem())
	{
		// ??? Potions should explode when hit (etc..)
	}
	else
	{
		// Take damage.
		(dynamic_cast <CChar*>(pObj))->Spell_Effect( m_Act_Spell, this, m_Act_Level );
	}
}

void CChar :: Spell_Area( CPoint p, int iDist )
{
	// Effects all creatures in the area. (but not us)
	CWorldSearch Area( p, iDist );
	CChar * pCharNext;
	for ( CChar * pChar = Area.GetChar(); pChar != NULL; pChar = pCharNext )
	{
		pCharNext = Area.GetChar();
		if ( pChar == this )
		{
			if ( Spells[m_Act_Spell].m_wFlags & SPELLFLAG_HARM )
				continue;
		}
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
	}
}

void CChar :: Spell_Field( CPoint p, ITEMID_TYPE idEW, ITEMID_TYPE idNS )
{
	// Cast the field spell to here.
	//

	// get the dir of the field.
	int dx = abs( p.m_x - m_p.m_x );
	int dy = abs( p.m_y - m_p.m_y );
	WORD id = ( dx > dy ) ? idNS : idEW;

	for ( int i=-3; i<=3; i++ )
	{
		CItem * pSpell = ItemCreateBase( (ITEMID_TYPE) id );
		pSpell->m_type = ITEM_SPELL;
		pSpell->m_Attr |= ATTR_MAGIC;
		pSpell->m_item_spell = m_Act_Spell;
		pSpell->m_item_magiclevel = 30;
		pSpell->m_item_charges = 1;
		pSpell->SetTimeout( 30 + GetRandVal(60) );

		CPoint pg = p;
		if ( dx > dy ) 
			pg.m_y += i;
		else
			pg.m_x += i;
		pSpell->PutOnGround( pg );

		// ??? Check for direct cast on a creature.
	}
}

static const BYTE Spell_ManaReq[8] = { 4, 6, 9, 11, 14, 20, 40, 50 };

bool CChar :: Spell_CanCast( SPELL_TYPE spell ) const
{
	// Do we have enough mana to start ?
	if ( m_mana < Spell_ManaReq[ (spell-1)/8 ] && ! IsPriv( PRIV_GM ))
	{
		SysMessage( "You lack sufficient mana for this spell" );
		return( false );
	}

	// Check for regs ?
	// for ( int i=0; i<Reagents; i++ )
	//	  if ( ! pPack->ContentConsume( ITEMID_REAGENTTYPE, 1, true ))
	//		return( false );

	return( true );
}

void CChar :: Spell_Done( void )
{
	// Ready for the spell effect.
	// ??? What if spell was magic item or scroll ?
	ASSERT( m_Act_Spell != SPELL_NONE );

	CObjBase * pObj = m_Act_Targ.ObjFind();	// dont always need a target.
	if ( pObj != NULL )
	{
		UpdateDir( pObj );
	}
	else if ( Spells[m_Act_Spell].m_wFlags & SPELLFLAG_TARG_OBJ )
	{
		// Need a target.
		SysMessage( "This spell needs a target object" );
		return;
	}
	CChar * pChar = dynamic_cast <CChar*> ( pObj );
	if ( Spells[m_Act_Spell].m_wFlags & SPELLFLAG_TARG_CHAR )
	{
		// Need a char target.
		if ( pChar == NULL )
		{
			SysMessage( "This spell has no effect on objects" );
			return;
		}
	}

	switch ( m_Act_Spell )
	{
	// 1st
	case SPELL_Create_Food:
		// Create object. Normally food.
		if ( pObj == NULL )
		{
			static WORD Item_Foods[] =	// possible foods.
			{
				ITEMID_FOOD_BACON, 
				ITEMID_FOOD_SAUSAGE,
				ITEMID_FOOD_HAM,
				ITEMID_FOOD_CAKE,
				ITEMID_FOOD_BREAD,
			};

			CItem * pItem = ItemCreateScript( Item_Foods[ GetRandVal( COUNTOF( Item_Foods )) ] );
			pItem->m_type = ITEM_FOOD;	// should already be set .
			pItem->PutOnGround( m_Act_p );
		}
		break;

	case SPELL_Magic_Arrow:
		Spell_Bolt( pObj, ITEMID_FX_MAGIC_ARROW );
		break;

	case SPELL_Clumsy:
	case SPELL_Feeblemind:
	case SPELL_Heal:
	case SPELL_Night_Sight:
	case SPELL_Reactive_Armor:
	case SPELL_Weaken:

	// 2nd
	case SPELL_Agility:
	case SPELL_Cunning:
	case SPELL_Cure:
	case SPELL_Harm:
	case SPELL_Protection:
	case SPELL_Strength:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Magic_Trap:
	case SPELL_Magic_Untrap:
		break;

	// 3rd
	case SPELL_Bless:
	case SPELL_Poison:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;
	case SPELL_Fireball:
		Spell_Bolt( pObj, ITEMID_FX_FIRE_BALL );
		break;
	case SPELL_Magic_Lock:
		{
			if ( pObj == NULL ) return;
			CItem * pItem = dynamic_cast <CItem*> (pObj);
			if ( pItem == NULL || 
				pItem->GetID() == ITEMID_CORPSE || 
				(pItem->m_type != ITEM_CONTAINER && pItem->m_type != ITEM_DOOR) )
			{
				SysMessage( "You cannot lock that!" );
				return;
			}
			switch ( pItem->m_type )
			{
			case ITEM_CONTAINER:
				pItem->m_type=ITEM_CONTAINER_LOCKED;
				SysMessage( "You lock the container.");
				break;
			case ITEM_DOOR:
				pItem->m_type=ITEM_DOOR_LOCKED;
				SysMessage( "You lock the door.");
				break;
			}
		} 
		break;
	case SPELL_Telekin:
		break;
	case SPELL_Teleport:
		Spell_Teleport( m_Act_p );
		break;
	case SPELL_Unlock:
		{
			if ( pObj == NULL ) return;
			CItem * pItem = dynamic_cast <CItem*> (pObj);
			if ( pItem == NULL || pItem->GetID() == ITEMID_CORPSE ||
				(pItem->m_type != ITEM_CONTAINER_LOCKED && pItem->m_type != ITEM_DOOR_LOCKED) )
			{
				SysMessage( "You cannot unlock that!" );
				return;
			}
			switch ( pItem->m_type )
			{
			case ITEM_CONTAINER_LOCKED:
				pItem->m_type=ITEM_CONTAINER;
				SysMessage( "You unlock the container.");
				break;
			case ITEM_DOOR_LOCKED:
				pItem->m_type=ITEM_DOOR;
				SysMessage( "You unlock the door.");
				break;
			}
		} 
		break;
	case SPELL_Wall_of_Stone:
		Spell_Field( m_Act_p,  (ITEMID_TYPE) 0x0080, (ITEMID_TYPE) 0x0080 );
		break;

	// 4th
	case SPELL_Arch_Cure:
		Spell_Area( m_Act_p, 5 );
		break;
	case SPELL_Arch_Prot:
		Spell_Area( m_Act_p, 5 );
		break;
	case SPELL_Curse:
		// Curse a object ?
		if ( pObj->IsItem())
		{
			break;
		}
	case SPELL_Great_Heal:
	case SPELL_Mana_Drain:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;
	case SPELL_Fire_Field:
		Spell_Field( m_Act_p, ITEMID_FX_FIRE_F_EW, ITEMID_FX_FIRE_F_NS );
		break;
	case SPELL_Lightning:
		if ( pObj->IsItem())
		{
			pObj->Effect( 1, ITEMID_NOTHING, this );
		}
		else
		{
			pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		}
		break;

	case SPELL_Recall:
		{
		CItem * pItem = dynamic_cast <CItem*> (pObj);
		if ( pItem == NULL ||
			( pItem->m_type != ITEM_RUNE && pItem->m_type != ITEM_TELEPAD ))
		{	
			SysMessage( "That item is not a recall rune." );
			return;
		}
		if ( ! pItem->m_magic_p.IsValid() )
		{
			SysMessage( "The recall rune is blank." );
			return;
		}
		MoveTo( pItem->m_magic_p );
		Update();
		}
		break;

	// 5th

	case SPELL_Blade_Spirit:
		goto summon;

	case SPELL_Dispel_Field:
	{
		CItem * pItem = dynamic_cast <CItem*> (pObj);
		if ( pItem == NULL || ( pItem->m_type != ITEM_FIRE && pItem->m_type != ITEM_SPELL ))
		{
			SysMessage( "That is not a field!" );
			return;
		}
		delete pItem;
		break;
	}

	case SPELL_Mind_Blast:
	    Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 0, 15);
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Paralyze:
	case SPELL_Magic_Reflect:
	case SPELL_Incognito:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Poison_Field:
		Spell_Field( m_Act_p, ITEMID_FX_POISON_F_EW, ITEMID_FX_POISON_F_NS );
		break;

	case SPELL_Summon:
	summon:
		Spell_Summon( m_Act_Arg, m_Act_p );
		break;

	// 6th

	case SPELL_Dispel:
	case SPELL_Invis:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Energy_Bolt:
		Spell_Bolt( pObj, ITEMID_FX_ENERGY_BOLT );
		break;

	case SPELL_Explosion:
		Spell_Area( m_Act_p, 2 );
		break;

	case SPELL_Mark:
		{
			if ( pObj == NULL ) return;
			CItem * pItem = dynamic_cast <CItem*> (pObj);
			if ( pItem == NULL ||
				( ! IsPriv(PRIV_GM) && pItem->m_type != ITEM_RUNE && pItem->m_type != ITEM_TELEPAD ))
			{
				SysMessage( "That item is not a recall rune." );
				return;
			}
			if ( pItem->m_Attr & ATTR_MAGIC )
			{
				// cannot mark a magical item !!
				SysMessage( "The item rejects the magic charge." );
				return;
			}
			pItem->m_magic_p = m_p;
		} 
		break;

	case SPELL_Mass_Curse:
		Spell_Area( m_Act_p, 5 );
		break;
	case SPELL_Paralyze_Field:
		Spell_Field( m_Act_p, ITEMID_FX_PARA_F_EW, ITEMID_FX_PARA_F_NS );
		break;
	case SPELL_Reveal:
		Spell_Area( m_Act_p, UO_MAP_VIEW_SIZE );
		break;

	// 7th

	case SPELL_Chain_Lightning:
		Spell_Area( m_Act_p, 5 );
		break;
	case SPELL_Energy_Field:
		Spell_Field( m_Act_p, ITEMID_FX_ENERGY_F_EW, ITEMID_FX_ENERGY_F_NS );
		break;

	case SPELL_Flame_Strike:
		// Display spell.
		if ( pObj == NULL )
		{
			CItem * pItem = ItemCreateBase( ITEMID_FX_FLAMESTRIKE );
			pItem->m_type = ITEM_SPELL;
			pItem->m_item_spell = SPELL_Flame_Strike;
			pItem->SetTimeout( 2 );
			pItem->PutOnGround( m_Act_p );
		}
		else
		{
			pObj->Effect( 3, ITEMID_FX_FLAMESTRIKE, pObj, 6, 15 );
			// Burn person at location.
			if ( ! pObj->IsItem())
			{
				pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
			}
		}
		break;

	case SPELL_Gate_Travel:
		{
			CItem * pItem = dynamic_cast <CItem*> (pObj);
			if ( pItem == NULL || 
				( pItem->m_type != ITEM_RUNE && pItem->m_type != ITEM_TELEPAD ))
			{
				SysMessage( "That item is not a recall rune." );
				return;
			}
			if ( ! pItem->m_magic_p.IsValid() )
			{
				SysMessage( "The recall rune is blank." );
				return;
			}

			CItem * pGate = ItemCreateBase( ITEMID_BLUEMOONGATE );
			pGate->m_type = ITEM_TELEPAD;
			pGate->m_magic_p = pItem->m_magic_p;
			pGate->SetTimeout( 60 );
			pGate->PutOnGround( m_p );

			// Effect( 0x1af3 );

			// Far end gate.
			pGate = ItemCreateBase( ITEMID_BLUEMOONGATE );
			pGate->m_type = ITEM_TELEPAD;
			pGate->m_magic_p = m_p;
			pGate->SetTimeout( 60 );
			pGate->PutOnGround( pItem->m_magic_p );
			pGate->Sound( Spells[ SPELL_Gate_Travel ].m_sound );
		}
		break;

	case SPELL_Mana_Vamp:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Mass_Dispel:
		Spell_Area( m_Act_p, 15 );
		break;

	case SPELL_Meteor_Swarm:
		// Multi explosion ??? 0x36b0
		Spell_Area( m_Act_p, 4 );
		break;

	case SPELL_Polymorph:
		// This has a menu select for client. doesn't come here.
		break;

	// 8th

	case SPELL_Earthquake:
		Spell_Area( m_p, UO_MAP_VIEW_SIZE );
		break;

	case SPELL_Vortex:
		m_Act_Arg = CREID_AIR_ELEM;
		goto summon;

	case SPELL_Resurrection:
		pChar->Spell_Effect( m_Act_Spell, this, m_Act_Level );
		break;

	case SPELL_Air_Elem:
		m_Act_Arg = CREID_AIR_ELEM;
		goto summon;
	case SPELL_Daemon:
		m_Act_Arg = ( GetRandVal( 2 )) ? CREID_DAEMON_SWORD : CREID_DAEMON;
		goto summon;
	case SPELL_Earth_Elem:
		m_Act_Arg = CREID_EARTH_ELEM;
		goto summon;
	case SPELL_Fire_Elem:
		m_Act_Arg = CREID_FIRE_ELEM;
		goto summon;
	case SPELL_Water_Elem:
		m_Act_Arg = CREID_WATER_ELEM;
		goto summon;

		// Necro
	case SPELL_Summon_Undead:
		switch (GetRandVal(15))
		{
		case 1:
			m_Act_Arg = CREID_LICH;
			break;
		case 3:
		case 5:
		case 7:
		case 9:
			m_Act_Arg = CREID_SKELETON;
			break;
		default:
			m_Act_Arg = CREID_ZOMBIE;
			break;
		}
		goto summon;

	case SPELL_Animate_Dead:
	{
		CItem * pItem = dynamic_cast <CItem*> (pObj);
		if ( pItem == NULL || pItem->GetID() != ITEMID_CORPSE )
		{
			SysMessage( "That is not a corpse!" );
			return;
		}
		if ( pItem->m_amount < CREID_MAN ) 	// Must be a human corpse ? 
		{
			SysMessage( "The monster corpse stirs for a moment" );
			return;
		}
		// Dump any stuff on corpse. ???
		pItem->GetThisContainer()->ContentsDump( pItem->m_p );
		// pItem->RaiseCorpse( dynamic_cast <CContainerItem *>(pCorpse) );
		delete pItem;
		m_Act_Arg = CREID_ZOMBIE;
		goto summon;
	}

	case SPELL_Bone_Armor:
	{
		CItem * pItem = dynamic_cast <CItem*> (pObj);
		if ( pItem == NULL || pItem->GetID() != ITEMID_CORPSE )
		{
			SysMessage( "That is not a corpse!" );
			return;
		}
		if ( pItem->m_amount != CREID_SKELETON ) 	// Must be a skeleton corpse 
		{
			SysMessage( "The body stirs for a moment" );
			return;
		}
		// Dump any stuff on corpse
		pItem->GetThisContainer()->ContentsDump( pItem->m_p );
		delete pItem;
		switch (GetRandVal(30))
		{
		case 1:
		case 16:
			pItem = ItemCreateScript( ITEMID_BONE_ARMS );
			pItem->PutOnGround( m_Act_p );
			break;
		case 2:
		case 7:
			pItem = ItemCreateScript( ITEMID_BONE_ARMOR );
			pItem->PutOnGround( m_Act_p );
			break;
		case 3:
		case 22:
			pItem = ItemCreateScript( ITEMID_BONE_GLOVES );
			pItem->PutOnGround( m_Act_p );
			break;
		case 4:
		case 9:
			pItem = ItemCreateScript( ITEMID_BONE_HELM );
			pItem->PutOnGround( m_Act_p );
			break;
		case 5:
		case 15:
			pItem = ItemCreateScript( ITEMID_BONE_LEGS );
			pItem->PutOnGround( m_Act_p );
			break;
		case 30:
			pItem = ItemCreateScript( ITEMID_BONE_ARMS );
			pItem->PutOnGround( m_Act_p );
			pItem = ItemCreateScript( ITEMID_BONE_ARMOR );
			pItem->PutOnGround( m_Act_p );
			pItem = ItemCreateScript( ITEMID_BONE_GLOVES );
			pItem->PutOnGround( m_Act_p );
			pItem = ItemCreateScript( ITEMID_BONE_HELM );
			pItem->PutOnGround( m_Act_p );
			pItem = ItemCreateScript( ITEMID_BONE_LEGS );
			pItem->PutOnGround( m_Act_p );
			break;
		default:
			SysMessage( "The bones shatter into dust!" );
			break;
		}
	}
	}

	// Consume the reagents ???
	// for ( int i=0; i<Reagents; i++ )
	//		pPack->ContentConsume( ITEMID_REAGENTTYPE, 1 );

	// Consume mana.
	m_mana -= Spell_ManaReq[ m_Act_Spell/8 ];
	if ( m_mana < 0 ) m_mana = 0;
	UpdateStats(STAT_INT);

	// Make noise.
	Sound( Spells[ m_Act_Spell ].m_sound );
}

bool CChar :: Spell_Cast( CObjUID uid, CPoint p )
{
	// The char (player or npc) is casting a spell.
	// Begin casting the spell
	// m_Act_Spell = the spell we are casting.

	ASSERT( m_Act_Skill == SKILL_MAGERY );

	if ( ! uid.IsValidUID() &&	// need a target ?
		( Spells[m_Act_Spell].m_wFlags & ( SPELLFLAG_TARG_OBJ | SPELLFLAG_TARG_CHAR )))
		return( false );

	m_Act_Targ = uid;	
	m_Act_p = p;		

	// Animate casting.
	UpdateAnimate(( Spells[m_Act_Spell].m_wFlags & SPELLFLAG_DIR_ANIM ) ? ANIM_CAST_DIR : ANIM_CAST_AREA );

	// Say words of power.
	// Some monsters don't speak ?
	int len=0;
	char * pTemp = GetTempStr();
	int i=0;
	for ( ; Spells[m_Act_Spell].m_Runes[i]; i++ )
	{
		len += sprintf( pTemp+len, Runes[ Spells[m_Act_Spell].m_Runes[i]-'A' ] );
		pTemp[len++]=' ';
	}
	pTemp[len] = '\0';
	Speak( pTemp );

	// Calculate the difficulty
	return( Skill_Start( ((( m_Act_Spell-1 ) / 8 ) + 1 ) * 10 ));
}

void CChar :: MakeKill( CChar * pKill )
{
	if ( m_Act_Targ != pKill->GetUID()) return;	// Mistake i guess.

	int iPrvLevel = GetNotoLevel();

	// Attribute a kill to me.
	m_Stat[STAT_Kills]++;

	m_Stat[STAT_Fame] += pKill->m_Stat[STAT_Fame] / 10;
	if ( m_Stat[STAT_Fame] > 10000) m_Stat[STAT_Fame] = 10000;

    m_Stat[STAT_Karma] -= pKill->m_Stat[STAT_Karma] / 10;
	if ( m_Stat[STAT_Karma] > 10000) m_Stat[STAT_Karma] = 10000;
	if ( m_Stat[STAT_Karma] < -10000) m_Stat[STAT_Karma] = -10000;

	if ( iPrvLevel != GetNotoLevel())
	{
		// reached a new title level ?
		CString sMsg;
		sMsg.Format( "You are now %s", GetNotoTitle());
		SysMessage( sMsg );
	}
}

bool CChar :: Reveal( DWORD flags )
{
	m_StatFlag &= ~flags;
	if ( ! ( m_StatFlag & ( STATF_Invisible | STATF_Hidden )))
		return( false );
	m_StatFlag &= ~( STATF_Invisible | STATF_Hidden );
	UpdateMove(m_p);
	SysMessage( "You have been revealed" );
	return( true );
}

void CChar :: Speak( const char * pText, WORD color, TALKMODE_TYPE mode )
{
	// Speak to all clients in the area.
	if ( mode == TALKMODE_YELL && IsPriv(PRIV_BROADCAST|PRIV_GM))
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	World.Speak( this, pText, color, mode, m_fonttype );
}

void CChar :: Hear( const char * pCmd, CChar * pSrc )
{
	// This CChar has heard you say something.
	if ( ! m_NPC_Speech )
		return;

	// too busy to talk ?
	if ( m_Act_Skill == SKILL_BEGGING ) // busy
		return;
	if ( m_StatFlag & STATF_War )
	{
		if ( pSrc->GetUID() == m_Act_Targ )
		{
			Speak( "Be quiet and fight scoundrel" );
			return;
		}
		Speak( "I am too busy fighting to speak with thee" );
		return;
	}

	CString sSec;
	sSec.Format( "%x", m_NPC_Speech );
	if ( ! World.m_scrSpeech.FindSec(sSec))
	{
		return;
	}

	// Was NPC talking to someone else ?
	if (( m_Act_Skill == NPCACT_TALK || m_Act_Skill == NPCACT_TALK_FOLLOW )
		&& m_Act_Targ != pSrc->GetUID() )
	{
		CChar * pCharOld = m_Act_Targ.CharFind();
		if ( pCharOld != NULL && GetDist( pCharOld ) < UO_MAP_VIEW_SIZE )
		{
			CString sMsg;
			sMsg.Format( "Excuse me %s, but %s wants to speak to me",
				pCharOld->GetName(), pSrc->GetName());
			Speak( sMsg );
		}
	}

	// You are the new speaking target.
	m_Act_Targ = pSrc->GetUID();
	Skill_Setup( NPCACT_TALK );
	SetTimeout( 60 );

	bool fMatch=false;
	bool fReacted = false;

	sSec.Empty();	// default text.
	while ( 1 )
	{
		if ( ! World.m_scrSpeech.ReadParse()) break;
		if ( ! strcmp("DEFAULT",World.m_scrSpeech.m_Line))
		{
			sSec = World.m_scrSpeech.m_pArg;
			continue;
		}
		if (!strcmp("ON",World.m_scrSpeech.m_Line))
		{
			if ( fReacted ) return;

			// Look for some key word.
			if ( FindStrWord( pCmd, World.m_scrSpeech.m_pArg ))
				fMatch = true;
			continue;
		}

		if ( ! fMatch ) continue;	// look for the next "ON" section.

		if (!strcmp("SAY",World.m_scrSpeech.m_Line))
		{
			Speak( World.m_scrSpeech.m_pArg );
		}
		else if (!strcmp("BANK",World.m_scrSpeech.m_Line))
		{
			// Open the bank box.
			if ( pSrc->m_pClient == NULL ) break; // NPC ?
			pSrc->m_pClient->addBankOpen( pSrc );
		}
		else if (!strcmp("BUY",World.m_scrSpeech.m_Line))
		{
			// Open up the buy dialog.
			if ( pSrc->m_pClient == NULL ) break;
			if ( ! pSrc->m_pClient->addShopMenuBuy( this ))
			{
				Speak( "Sorry I have no goods to sell" );
			}
		}
		else if (!strcmp("SELL",World.m_scrSpeech.m_Line))
		{
			// Open up the sell dialog.
			if ( pSrc->m_pClient == NULL ) break;
			if ( ! pSrc->m_pClient->addShopMenuSell( this ))
			{
				Speak( "You have nothing I'm interested in" );
			}
		}
		else if (!strcmp("BYE",World.m_scrSpeech.m_Line))
		{
			Skill_Setup( SKILL_NONE );
		}
		if ( pSrc->m_pClient != NULL && 
			( pSrc->IsPriv(PRIV_GM) || pSrc->GetUID() == m_owner ))
		{
			if (!strcmp("STOCK",World.m_scrSpeech.m_Line))
			{
				Speak( "Put restockable items in here." );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_STOCK );
			}
			else if (!strcmp("INVENTORY",World.m_scrSpeech.m_Line))
			{
				Speak( "Put items you want to sell in here." );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_EXTRA );
			}
			else if (!strcmp("WANT",World.m_scrSpeech.m_Line))
			{
				Speak( "Put items like what you want me to purchase in here" );
				pSrc->m_pClient->addBankOpen( this, LAYER_VENDOR_SELL );
			}
		}

		fReacted = true;
	}

	if ( ! fMatch )
	{
		if ( sSec.IsEmpty())
		{
			// Come up with some other default.
static const char * Speak_NoUnderstand[] =
{
	"Huh?",
	"I don't understand thee",
	"I'm sorry but that's not anything I know about",
};
			sSec = Speak_NoUnderstand[ GetRandVal( COUNTOF( Speak_NoUnderstand )) ];
		}
		if ( ++ m_Act_Arg > 4 ) Skill_Setup( SKILL_NONE ); // say good by
		Speak( sSec );
	}
}

void CChar :: MakeCorpse()
{
	// some creatures (Elems) have no corpses.

	CContainerItem * pCorpse = NULL;

	if ( ! ( m_StatFlag & STATF_Conjured ) &&
		m_prev_id != CREID_WATER_ELEM &&
		m_prev_id != CREID_AIR_ELEM &&
		m_prev_id != CREID_FIRE_ELEM && 
		m_prev_id != CREID_BLADES )
	{
		pCorpse = dynamic_cast <CContainerItem *>(ItemCreateScript( ITEMID_CORPSE ));
		pCorpse->m_sName.Format( "Body of %s", GetName() );
		pCorpse->m_color = m_prev_color;
		pCorpse->m_amount = m_prev_id;	// id the corpse type here !
		pCorpse->m_corpse_deathtime = World.m_Clock_Time;	// death time.
		pCorpse->m_corpse_killer_UID = m_Act_Targ;
		pCorpse->m_corpse_dir = m_dir;
		pCorpse->m_Attr |= ATTR_INVIS;	// Don't display til ready.
		pCorpse->SetTimeout( 10*60 );

		// Copy hair and beard to corpse.
		CItem * pItem = LayerFind( LAYER_HAIR ); 
		if ( pItem != NULL )
		{
			pItem->m_Attr |= ATTR_NEWBIE;
			pCorpse->ContentAdd( ItemCreateDupe( pItem ));	// add content
		}
		pItem = LayerFind( LAYER_BEARD ); 
		if ( pItem != NULL )
		{
			pItem->m_Attr |= ATTR_NEWBIE;	// should not come off when corpse is cut up.
			pCorpse->ContentAdd( ItemCreateDupe( pItem ));	// add content
		}
	}

	// Move non-newbie contents of the pack to corpse.
	CContainerItem * pPack = dynamic_cast <CContainerItem *>(LayerFind( LAYER_PACK ));
	if ( pPack != NULL )
	{
		CItem* pItemNext; 
		for ( CItem* pItem=pPack->GetContentHead(); pItem!=NULL; pItem=pItemNext)
		{
			pItemNext = pItem->GetNext();
			if ( pItem->m_Attr & ATTR_NEWBIE )	// keep newbie stuff.
				continue;
			if ( pCorpse != NULL )
				pCorpse->ContentAdd( pItem );	// add content
			else
				pItem->PutOnGround( m_p );
		}
	}

	// transfer equipped items to corpse or your pack (if newbie).
	UnEquipAllItems( pCorpse );

#ifdef COMMENT
	// Death anim.
	// Die in flight ?
	// UpdateAnimate( (GetRandVal(2)) ? ANIM_DIE_BACK : ANIM_DIE_FORWARD, true );

	CCommand cmd;
	cmd.CharDeath.m_Cmd = 0xAF;
	cmd.CharDeath.m_UID = GetUID();	// 1-4
	cmd.CharDeath.m_zero5 = 0;
	cmd.CharDeath.m_UIDCorpse = ( pCorpse == NULL ) ? 0 : pCorpse->GetUID(); // 9-12
	cmd.CharDeath.m_zero13 = 0;
	UpdateCanSee( &cmd, sizeof( cmd.CharDeath ));
#endif

	if ( pCorpse != NULL )
	{
		pCorpse->m_Attr &= ~ATTR_INVIS;	// make visible.
		pCorpse->PutOnGround( m_p );
	}
}

void CChar :: RaiseCorpse( CContainerItem * pCorpse )
{
	// We are creating a char from the current char and the corpse.

	// if ( GetID() != (CREID_TYPE) pCorpse->m_amount ) return;	// incompatible !

	// Move the items from the corpse back onto us.
	// Just toss it all into our pack for now.
	CContainerItem * pPack = dynamic_cast <CContainerItem *>(LayerFind( LAYER_PACK ));
	if ( pPack == NULL ) 
		return;

	CItem* pItemNext; 
	for ( CItem* pItem=pCorpse->GetContentHead(); pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( pItem->GetEquipLayer() )
			ContentAdd( pItem );	// Equip the item.
		else
			pPack->ContentAdd( pItem );	// Toss into pack.
	}

	// Corpse is now gone.
	delete pCorpse;
}

void CChar :: Death()
{
	if ( m_StatFlag & ( STATF_DEAD | STATF_INVUL )) return;

	if ( m_pClient != NULL )	// Prevents crashing ?
	{
		m_pClient->addPause( true );
	}

	Spell_Dispel();		// Get rid of all spell effects.
	Horse_UnMount();
	SoundChar( CRESND_DIE );

	// ??? May not need to say this (menu is ok)
	SysMessage( "You are dead." );

	// Attribute the kill to the attacker.
	CChar * pAttacker = m_Act_Targ.CharFind();
	if ( pAttacker != NULL )
	{
		pAttacker->MakeKill( this );
	}

	// lose some fame for death.
	m_Stat[STAT_Fame] -= m_Stat[STAT_Fame]/10;

	// create the corpse item.
	MakeCorpse();

	if ( m_pClient == NULL )
	{
		if ( m_pHome != NULL )
		{
			// Respawn the NPC later
			World.m_CharsIdle.InsertAfter( this );
		}
		else
		{
			delete this;
		}
		return;
	}

	ContentAdd( ItemCreateScript( ITEMID_DEATHSHROUD ));

	m_health = 0;	// on my way to death.
	m_color = 0x0000;
	m_StatFlag |= STATF_DEAD;
	SetID( ( IsFemale( m_prev_id )) ? CREID_GHOSTWOMAN : CREID_GHOSTMAN );
	Update();		// show everyone I am now a ghost.

	CCommand cmd;
	cmd.DeathMenu.m_Cmd = 0x2c;
	cmd.DeathMenu.m_shift = 0;
	m_pClient->xSendPkt( &cmd, sizeof( cmd.DeathMenu ));
}

void CChar :: Use_Item( CItem * pItem, bool fLink )
{
	// don't check distance here.
	// Could be a switch or something.

	CObjUID uidLink( pItem->m_Door_Link );

	switch ( pItem->m_type )
	{
	case ITEM_SWITCH:
		// Switches can be linked to gates and doors and such
		break;

	case ITEM_GATE:
		// Open a gate.
		break;

	case ITEM_DOOR:
		// Trapped door ?

		// Guarded door ?
		// Linked doors.
		{
		bool fOpen = pItem->Use_Door( fLink );
		if ( fLink || ! fOpen ) return;
		break;
		}
	}

	if ( uidLink.IsValidUID())
	{
		CItem * pLinkItem = uidLink.ItemFind();
		if ( pLinkItem != NULL && pLinkItem != pItem )
		{
			Use_Item( pLinkItem, true );
		}
	}
}

void CChar :: Eat( CItem * pItem )
{
static const WORD EatSounds[] = { 0x03a, 0x03b, 0x03c };

	const char * pMsg;
	switch ( m_food )
	{
    case 0:
		pMsg = "You eat the food, but are still extremely hungry.";
        break;
    case 1:
		pMsg = "After eating the food, you feel much less hungry.";
        break;
    case 2:
    case 3:
		pMsg = "You eat the food, and begin to feel more satiated.";
		break;
    case 4:
		pMsg = "You are nearly stuffed, but manage to eat the food.";
        break;
    case 5:
		pMsg = "You feel quite full after consuming the food.";
        break;
    case 6:
	default:
		pMsg = "You are simply too full to eat any more!";
		break;
	}

	// ??? monsters should be able to eat corpses 

	SysMessage( pMsg );
	if ( m_food >= 6 ) return;
	m_food ++;	// ??? some food should have more value than other !

	Sound( EatSounds[ GetRandVal( COUNTOF(EatSounds)) ] );
	UpdateAnimate( ANIM_EAT );
	delete pItem;
}

static WORD Item_Blood[] =	// ??? not used
{
	0x1cf2, ITEMID_BLOOD1, ITEMID_BLOOD6,
};

void CChar :: CarveCorpse( CContainerItem * pCorpse )
{
	ASSERT( pCorpse->GetID() == ITEMID_CORPSE );

	UpdateAnimate( ANIM_BOW );

	if ( pCorpse->m_amount >= CREID_MAN )
	{
		// dismember the corpse.
		static const WORD Item_BodyParts[] = { 0x1ce1, 0x1cdd, 0x1ce5, 0x1ce2, 0x1cec, 0x1ced, 0x1cf1 };

		CItem * pParts = ItemCreateBase( (ITEMID_TYPE) Item_Blood[ GetRandVal( COUNTOF( Item_Blood )) ] );
		pParts->SetTimeout( 60 );
		pParts->PutOnGround( pCorpse->m_p );

		for ( int i=0; i < COUNTOF( Item_BodyParts ); i++ )
		{
			pParts = ItemCreateScript( (ITEMID_TYPE) Item_BodyParts[i] );
			pParts->SetTimeout( 5*60 );
			pParts->PutOnGround( pCorpse->m_p );
		}

		// Dump any stuff on corpse. 
		pCorpse->ContentsDump( pCorpse->m_p );
		delete pCorpse;
		return;
	}

	// based on corpse type.
	const CCharBase * pCre = FindBase( (CREID_TYPE) pCorpse->m_amount );
	if ( pCre == NULL || 
		! pCorpse->m_corpse_deathtime || 
		pCre->pCorpseResources == NULL ||
		pCre->pCorpseResources[0] == '\0' )
	{
	    SysMessage( "You carve the corpse but find nothing usefull." );
		return;
	}

	pCorpse->m_corpse_deathtime = 0;	// been carved.
	pCorpse->m_corpse_killer_UID = GetUID();	// by you.

	char * ppCmd[16];
	char * pTemp = GetTempStr();
	strcpy( pTemp, pCre->pCorpseResources );
	int iCount = ParseCmds( pTemp, ppCmd, COUNTOF( ppCmd ));

	bool fMsg = false;
	for ( int i=0; i<iCount; i++ )
	{
		const char * pszMsg = NULL;
		int iQty = atoi( ppCmd[i]+1 );
		ITEMID_TYPE id;
		switch ( toupper( ppCmd[i][0] ))
		{
		case 'M':	// meat
			id = ITEMID_RAW_MEAT;
			pszMsg = "You carve away some meat.";
			break;
		case 'F':	// feathers.
			id = ITEMID_FEATHERS3;
			pszMsg = "You pluck the bird and get some feathers.";
			break;
		case 'H':	// hides
			id = (ITEMID_TYPE) 0x1078;
			pszMsg = "You skin the corpse and get the hides.";
			break;
		case 'S':	// fur
			id = (ITEMID_TYPE) 0x11fa;
			pszMsg = "You skin the corpse and get some fur.";
			break;
		case 'W':	// wool
			id = (ITEMID_TYPE) 0x0df8;
			pszMsg = "You skin the corpse and get some unspun wool.";
			break;
		case '?':	// Special item like dragon blood and daemon bone.
			id = (ITEMID_TYPE) ahextoi(ppCmd[++i]);
			break;
		}

		CItem * pPart = ItemCreateScript( id );
		pPart->SetAmount( iQty );
		pPart->SetTimeout( 160 );
		pPart->PutOnGround( pCorpse->m_p );

		if ( ! fMsg && pszMsg != NULL )
		{
			fMsg = true;
			SysMessage( pszMsg );
		}
	}

	CItem * pBlood = ItemCreateBase( ITEMID_BLOOD1 );
	pBlood->SetTimeout( 60 );
	pBlood->PutOnGround( pCorpse->m_p );
}

void CChar :: Drink( CItem * pItem )
{
static const WORD DrinkSounds[] = { 0x030, 0x031 };

#ifdef COMMENT
	// If the item is poison. make sure we know we are poisoning ourself !
	//
	SKILL_POISONING,	// 30

 int s, j;
 
 s=calcSocketFromChar(p);
 
 switch(items[i].morey)
 {
 case 1: // Agility Potion
  break;
 case 2: // Cure Potion
  break;
 case 3: // Explosion Potion
  break;
 case 4: // Heal Potion
  switch(items[i].morez)
  {
  case 1:
   chars[p].hp=min(chars[p].hp+3+(rand()%5)+(items[i].morex/200), chars[p].st);
   break;
  case 2:
   chars[p].hp=min(chars[p].hp+13+(rand()%5)+(items[i].morex/200), chars[p].st);
   break;
  case 3:
   chars[p].hp=min(chars[p].hp+15+(rand()%9)+(items[i].morex/100), chars[p].st);
   break;
  }
  if (s!=-1) updatestats(p, 0);
  staticeffect(p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
  break;
 case 5: // Night Sight Potion
  break;
 case 6: // Poison Potion
  break;
 case 7: // Refresh Potion
  break;
 case 8: // Strength Potion
  break;
 case 9: // Mana Potion
  switch(items[i].morez)
  {
  case 1:
   chars[p].mn=min(chars[p].mn+10+items[i].morex/100, chars[p].in);
   break;
  case 2:
   chars[p].mn=min(chars[p].mn+20+items[i].morex/50, chars[p].in);
   break;
  }
  if (s!=-1) updatestats(p, 1);
  staticeffect(p, 0x37, 0x6A, 0x09, 0x06); // Sparkle effect
  break;
 }
 soundeffect2(p, 0x00, 0x30); 
 if (chars[p].id1>=1 && chars[p].id2>90 && chars[p].onhorse==0) npcaction(p, 0x22);
 if (items[i].amount!=1)
 {
  items[i].amount--;
  for (j=0;j<now;j++) if (perm[j] && inrange2(j, i)) senditem(j, i);
 }
 else
 {
  deleitem(i);
 }
}
	// ?? Drunk or halucinate
#endif

	Sound( DrinkSounds[ GetRandVal( COUNTOF(DrinkSounds)) ] );
	UpdateAnimate( ANIM_EAT );
	delete pItem;
}

bool CChar :: Horse_UnMount() // Get off a horse (Remove horse item and spawn new horse)
{
	if ( ! ( m_StatFlag & STATF_OnHorse )) return( false );

	CItem * pItem = LayerFind( LAYER_HORSE );
	if ( pItem != NULL )
	{
		CREID_TYPE id = CREID_HORSE1;
		switch (pItem->GetID())
		{
		case ITEMID_M_HORSE1: id=CREID_HORSE1; break;
		case ITEMID_M_HORSE2: id=CREID_HORSE2; break;
		case ITEMID_M_HORSE3: id=CREID_HORSE3; break;
		case ITEMID_M_HORSE4: id=CREID_HORSE4; break;
		}

		CChar * pHorse = CharCreate( id );
		pHorse->SetName( pItem->GetName() );
		pHorse->m_color = pHorse->m_prev_color = pItem->m_color;
		pHorse->m_dir = m_dir;
		pHorse->m_owner = GetUID();	// this
		pHorse->m_NPC_Brain = NPCBRAIN_ANIMAL;	// Must have a default brain.
		pHorse->MoveTo( m_p );
		pHorse->Update();
		pHorse->SoundChar( CRESND_RAND1 );	// Horse winny

		// Add back the conjure timer.
		if ( pItem->m_Attr & ATTR_MAGIC )
		{
			pHorse->m_StatFlag |= STATF_Conjured;
			// conjured horses must disapear eventually.
		}

		delete pItem;
	}

	return( true );
}

bool CChar :: Horse_Mount( CChar * pHorse ) // Remove horse char and give player a horse item
{
	if ( m_StatFlag & ( STATF_OnHorse | STATF_DEAD ))	// already on a horse.
		return( false );
	if ( ! ( pHorse->m_owner == GetUID() ))
	{
		SysMessage( "You dont own that horse." );
		return( false );
	}

	ITEMID_TYPE id = ITEMID_M_HORSE1;
	switch ( pHorse->GetID() )
	{
	case CREID_HORSE1: id=ITEMID_M_HORSE1; break;
	case CREID_HORSE2: id=ITEMID_M_HORSE2; break;
	case CREID_HORSE3: id=ITEMID_M_HORSE3; break;
	case CREID_HORSE4: id=ITEMID_M_HORSE4; break;
	}

	CItem * pItem = ItemCreateBase( id );
	pItem->SetName( pHorse->GetName());
	pItem->m_color = pHorse->m_color;

	if ( pHorse->m_StatFlag & STATF_Conjured )
	{
		// ??? conjured horses must disapear eventually.
		pItem->m_Attr |= ATTR_MAGIC;
		pHorse->SetTimeout( 10000 );
	}

	LayerAdd( pItem, LAYER_HORSE );

	pHorse->SoundChar( CRESND_RAND1 );	// Horse winny
	delete pHorse;
	pItem->Update();

	return( true );
}

CRegion * CChar :: GetRegion() const
{
	// What region in the current quadrant am i in ?
	// We only need to update this every 5 or so steps ?
	CQuadrant * pQuad = m_p.GetQuadrant();

	return( NULL );
}

void CChar :: Flip( const char * pCmd )
{
	m_dir = (DIR_TYPE) ( m_dir + 1 );
	if ( m_dir >= DIR_QTY )
		m_dir = DIR_N;
	UpdateMode();
}

bool CChar :: CheckLocation( bool fStanding )
{
	// We are at this location 
	// what will happen ?
	// RETURN: true = we teleported.

	CWorldSearch AreaItems( m_p );
	while ( 1 )
	{
		CItem * pItem = AreaItems.GetItem();
		if ( pItem == NULL ) break;
		
		switch ( pItem->m_type )
		{
		case ITEM_FIRE:
			// ??? fire object hurts us ?
			Spell_Effect( SPELL_Fire_Field, NULL, 50 );
			Sound( 0x15f ); // ??? Fire noise.
			return( false );
		case ITEM_SPELL:
			Spell_Effect( (SPELL_TYPE) pItem->m_item_spell, NULL, 50 );
			Sound( Spells[(SPELL_TYPE) pItem->m_item_spell].m_sound  );
			return( false );
		case ITEM_TRAP:
			TakeHitArmor( pItem->Use_Trap(), NULL );
			return( false );
		case ITEM_TELEPAD:
			if ( fStanding ) continue;
			Spell_Teleport( pItem->m_magic_p );
			return( true );
		}
	}   

	if ( fStanding ) return( false );

	// Did we step into a new region ?
	CRegion * pNewRegion = GetRegion();
	if ( pNewRegion != m_pRegion )
	{
		// Is it guarded ???

		m_pRegion = pNewRegion;
	}
	
	// Bump into other creatures ?
	if ( ! ( m_StatFlag & ( STATF_DEAD | STATF_Sleeping )))
	{
		CWorldSearch AreaChars( m_p );
		while ( 1 )
		{
			CChar * pChar = AreaChars.GetChar();
			if ( pChar == NULL ) break;
			if ( pChar == this ) continue;

			if ( pChar->m_StatFlag & STATF_DEAD )
			{
				SysMessage( "You feel a tingling sensation" );
				continue;
			}
			CString sMsg;
			if ( pChar->m_StatFlag & STATF_Invisible )
			{
				sMsg.Format( "You push past something invisible" );
			}
			else if ( pChar->m_StatFlag & STATF_Hidden )
			{
				// reveal hidden people ?
				sMsg.Format( "You stumble apon %s hidden.", pChar->GetName());
				pChar->Reveal(STATF_Hidden);
			}
			else if ( pChar->m_StatFlag & STATF_Sleeping )
			{
				sMsg.Format( "You step on the body of %s.", pChar->GetName());
				// ??? damage.
			}
			else
			{
				sMsg.Format( "You shove %s out of the way.", pChar->GetName());
			}
			SysMessage( sMsg );
			m_stam = max( m_stam-10, 0 );
			UpdateStats( STAT_DEX );
			break;
		}
	}

	// Hard coded teleporters here ?
	// ??? break down by CQuadrant.

static const struct 
{
	WORD m_src_x;
	WORD m_src_y;
	short m_delta;
	WORD m_dst_x;		// CPoint
	WORD m_dst_y;
	signed char m_dst_z;

} Teleporters[] = 
{
	// Moon gates of course.
	{ 1336, 1997,	0,	1829, 2949, -20 },		// britain_mg
	{ 1828, 2948,	0,	2702, 693,  5   },		// trinsic_mg
	{ 2701, 692,	0,	4468, 1284, 5   },		// vesper_mg
	{ 771,  752,	0,	1500, 3772, 5	},		// yew_mg
	{ 3563, 2139,	0,	644,  2068, 5   },		// magincia_mg
	{ 643,	2067,	0,	1337, 1998, 5	},		// skara_mg
	{ 4467,	1283,	0,	772,  753,  5   },		// moonglow_mg
	{ 1499, 3771,	0,	3564, 2140, 34	},		// jhelom_mg

	// What is HV ?
	{ 1653, 2963,	0,	1676, 2986, 1	},		// Hidden Valley
	{ 1677, 2987,	0,	1675, 2987, 21	},		// hv_b_b

	// Cities.

	{ 4449, 1115,	0,	4671, 1135, 10  },		// moonglow_1
	{ 4443, 1137,	0,	4487, 1475, 4 },		// moonglow_2
	{ 4436, 1107,	0,	4300, 992,	5 },		// moonglow_3
	{ 4449, 1107,	0,	4539, 890,  27	},		// moonglow_4

	{ 4663, 1134,	0,	4442, 1122, 4 },		// moonglow_4
	{ 4300, 1475,	0,	4442, 1122, 4 },		
	{ 4496, 968,	0,	4442, 1122, 4 },		
	{ 4540, 898,	0,	4442, 1122, 4 },

	{ 1409, 3824,	0,	1124, 3623, 6 },	// jhelom_m_n N=North M=Middle S=South Isle
	{ 1419, 3832,	0,	1466, 4015, 6 },	// jhelom_m_s

	{ 1142,	3621, 	0,	1414, 3828, 6 },	// jhelom_ns_m
	{ 1142, 3996, 	0,	1414, 3828, 6 },  
	{ 1406, 3621,  	0,	1414, 3828, 6 }, 
	{ 1406, 3996,  	0,	1414, 3828, 6 }, 

	{ 1361, 883,	0,	5166, 244, 15 },	// wind_out
	{ 5166, 245,	0,	1361, 884, 1 },		// wind_ent_out
	{ 5200, 71,		0,	5211, 22, 15 },		// wind_w_e W=Wind E=Eden
	{ 5217, 18,		0,	5204, 74, 17 },		// wind_e_w

	// Dungeons. (shuffle these ???)

	{ 2498, 916,	3,	5456, 1863, 0 },	// covetous_out
	{ 5455, 1864,   3,	2499,  917, 1 },    // covetous_ent_out

	{ 5392, 1959,	3,	2421, 884, 1 },		// covetous_1_2
	{ 2420, 883,	3,	5393, 1958, 0 },	// covetous_2_1_x
	{ 2384, 836,	-3,	5614, 1997, 0 },	// covetous_2_3
	{ 5615, 1996,   -3,	2385, 837, 1 },		// covetous_3_2
	{ 5388, 2027,	3,	2456, 859, 1 },		// covetous_3_4
	{ 2455, 858,	3,	5389, 2026, 0 },	// covetous_4_3	
	{ 2544, 849,	3,	5579, 1924, 0 },	// covetous_4_5
	{ 5578, 1926,	3,	2545, 851, 1 },		// covetous_5_4
	{ 5556, 1827,	2,	5552, 1807, 0 },	// covetous_5_6a
	{ 5551, 1805,	2,	5557, 1824, 0 },	// covetous_6a_5
	{ 5595, 1840,	-2,	5467, 1805, 5 },	// covetous_5_6b
	{ 5466, 1804,	-2,	5592, 1841, 0 },	// covetous_6b_5

	{ 4721, 3813,   3,	5905, 20, 44 },		// hythloth_out
	{ 5904, 18,		3,	4722, 3815, 1 },	// hythloth_ent_out

	{ 5905, 97,		0,	5976, 169, 1 },		// hythloth_1_2
	{ 5973, 169,	0,	5905, 99, 0 },		// hythloth_2_1
	{ 5921, 168,	-3,	6083, 145, -16 },	// hythloth_2_3
	{ 6082, 144,	-3,	5920, 169, 20 },	// hythloth_3_2
	{ 6040, 192,	-3,	6059, 89, 24 },		// hythloth_3_4
	{ 6058, 88,		-3,	6039, 193, 17 },	// hythloth_4_3

	{ 4110, 430,	3,	5188, 638, 1 },		// deceit_out
	{ 5187, 639,	3,	4111, 431, 6 },		// deceit_ent_out

	{ 5216, 586,	3,	6305, 533, 6 },		// deceit_1_2
	{ 5304, 532,	3,	5217, 585, -5 },	// deceit_2_1
	{ 5304, 650,	3,	5219, 759, -20 },	// deceit_4_3
	{ 5346, 578,	0,	5137, 650, 6 },		// deceit_2_3
	{ 5137, 649,	0,	5346, 579, 6 },		// deceit_3_2
	{ 5218,	761, 	2,	5306, 652, 11 },	// deceit_3_4

	{ 512,	1559,	3,	5395, 126, 0 },		// shame_out
	{ 5394, 127,	3,	513, 1560, 1 },		// shame_ent_out

	{ 5490, 19,		0,	5515, 11, 0	 },		// shame_1_2
	{ 5414, 10,		0,	5491, 19, -30 },	// shame_2_1
	{ 5414, 147,	0,	5604, 101, 1 },		// shame_3_2
	{ 5604, 102,	0,	5514, 148, 21 },	// shame_2_3
	{ 5538, 170,	0,	5516, 176, 1 },		// shame_3_4
	{ 5513, 176,	0,	5539, 170, 1 },		// shame_4_3
	{ 5507, 162,	0,	5875, 20, -5 },		// shame_4_5
	{ 5875, 19,		0,	5507, 161, 1 },		// shame_5_4

	{ 1175, 2635,	3,	5243, 1006, 0 },	// destard_out
	{ 5242, 1007,	3,	1176, 2636, 1 },	// destard_ent_out

	{ 5129,	909,	4,	5143, 801, 10 }, 	// destard_1_2
	{ 5142,	800,	4,	5130, 908, -25 },	// destard_2_1 
	{ 5135, 808,	-4,	5137, 986, 5 },		// destard_2_3
	{ 5136, 984,	-4,	5152, 809, -10 },	// destard_3_2

	{ 1296, 1080,   -3,	5587, 631, 30  },	// despise_out
	{ 5588, 630,	-3,	1297, 1081, 0 },	// despise_ent_out
	{ 5573, 628,	-3,	5501, 570, 57 },	// despise_ent_1
	{ 5573, 632,	-3,	5519, 673, 20 },	// despise_ent_2
	{ 5504, 569,	-3,	5576, 629, 30 },	// despise_1_ent
	{ 5521, 672,	-3,	5575, 633, 27 },	// despise_2_ent
	{ 5386, 755,	-3,	5407, 859, 45 },	// despise_2_3
	{ 5409, 858,	-3,	5389, 756, 3 },		// despise_3_2

	{ 2041, 215,	3,	5825, 630, 0 },		// wrong entrance
	{ 5824,	631,	3,	2042, 216, 15 },	// wrong_ent_out

	{ 5829, 593,	0,	5825, 630, 0 }, // wrong_1_2	 ???
	{ 5690, 569,	0,	5825, 630, 0 }, // wrong_2a_1 ???
	{ 5732, 554,	0,	5825, 630, 0 }, // wrong_2b_1 ???

};

	for ( int i =0; i < COUNTOF( Teleporters ); i++ )
	{
		int delta = Teleporters[i].m_delta;
		if ( ! delta )	// single space.
		{
			if ( m_p.m_x != Teleporters[i].m_src_x ) continue;
			if ( m_p.m_y != Teleporters[i].m_src_y ) continue;
		}
		else if ( delta > 0 )	// x delta
		{
			if ( m_p.m_y != Teleporters[i].m_src_y ) continue;
			if ( m_p.m_x < Teleporters[i].m_src_x ) continue;
			if ( m_p.m_x >= Teleporters[i].m_src_x + delta ) continue;
		}
		else	// y delta
		{
			if ( m_p.m_x != Teleporters[i].m_src_x ) continue;
			if ( m_p.m_y < Teleporters[i].m_src_y ) continue;
			if ( m_p.m_y >= Teleporters[i].m_src_y - delta ) continue;
		}

		CPoint p( Teleporters[i].m_dst_x, Teleporters[i].m_dst_y, Teleporters[i].m_dst_z );
		MoveTo( p );
		Update();
		return( true );
	}

	return( false );
}

bool CChar :: Attack( CChar * pChar )
{
	// RETURN: false = decline the attack.

	if ( pChar == NULL ||
		pChar == this ||
		( pChar->m_StatFlag & STATF_DEAD ) ||
		( m_StatFlag & STATF_DEAD )) 
	{
		// Turn war mode off if this is an NPC.
		m_Act_Targ.ClearUID();
		if ( ! m_NPC_Brain ) return( false );

		m_StatFlag &= ~STATF_War;
		Skill_Setup( SKILL_NONE );
		UpdateMode();
		return( false );
	}

	// I am attacking. (or defending)
	m_StatFlag |= STATF_War;
	Skill_Setup( GetWeaponSkill());
	if ( m_Act_Targ == pChar->GetUID()) return( true );

	m_Act_Targ = pChar->GetUID();
	UpdateDir( pChar );
	SetWeaponSwingTimer( false );	// ready to attack ?

	// Am i the aggressor ?
	if ( pChar->m_Act_Targ != GetUID())
	{
		m_StatFlag |= STATF_Aggressor;
	}
	else
	{
		m_StatFlag &= ~STATF_Aggressor;
	}

	// Am i a criminal ???


	// This may be a useless command.
	CCommand cmd;
	cmd.Fight.m_Cmd = 0x2f;
	cmd.Fight.m_zero1 = 0;
	cmd.Fight.m_AttackerUID = GetUID();
	cmd.Fight.m_AttackedUID = pChar->GetUID();

	CString sMsg;
	sMsg.Format( "* You see %s attacking %s *", GetName(), pChar->GetName() );

	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == m_pClient ) continue;
		if ( ! pClient->CanSee( this )) continue;
		pClient->xSendPkt( &cmd, sizeof(cmd.Fight));
		pClient->addItemMessage( sMsg, this );
	}
	return( true );
}

bool CChar :: ItemGive( CChar * pCharSrc, CItem * pItem )
{
	// Someone (Player) is giving me an item.
	// REUTRN: true = accepted.

	// The NPC might want it ?
	switch ( m_NPC_Brain )
	{
	case NPCBRAIN_NONE:
		break;

	case NPCBRAIN_HUMAN:
		if ( pItem->IsSameID( ITEMID_GOLD ))
		{
			Speak( "Gold is always welcome. thank thee." );
			ContentAdd( pItem );
			pItem->Update();
			return( true );
		}
		Speak( "I don't want this thank you" );
		return( false );

	case NPCBRAIN_BANKER:
		// I shall put this item in your bank account.
		if ( pItem->IsSameID( ITEMID_GOLD ))
		{
			CContainerItem * pBankBox = dynamic_cast <CContainerItem *>(pCharSrc->LayerFind( LAYER_BANKBOX ));
			if ( pBankBox != NULL )
			{
				Speak( "You will have deposited some gold" );
				pBankBox->ContentAdd( pItem );
				return( true );  // true when implimented
			}
		}
		Speak( "I don't want this thank you" );
		return( false );

	case NPCBRAIN_VENDOR:
		Speak( "To trade with me, please say 'vendor sell'" );
		return( false );

	case NPCBRAIN_ANIMAL:
		// Might want food ?
		break;

	case NPCBRAIN_BEGGAR:
		if (( pItem->GetID() == ITEMID_GOLD ) ||
			 ( pItem->m_type == ITEM_FOOD ))
			Speak( "Thank thee! Now I can feed my children!" );
		else
			Speak( "I'll sell this for gold! Thank thee!" );
		ContentAdd( pItem );
		pItem->Update();
		if (m_Act_Targ == pCharSrc->GetUID())
		{
			Speak( "If only others were as kind as you." );
			m_PrvTarg = m_Act_Targ;
			m_Act_Targ.ClearUID();
			Skill_Setup( SKILL_NONE );
		}
		return( true );
	}

	if ( m_pClient == NULL )
	{
		pCharSrc->m_pClient->addItemMessage( "They don't appear to want the item", this );
		return( false );
	}

	// ??? Put up the Secure transfer window for both clients.

#ifdef COMMENT
	CItem * pTrade = ItemCreateBase( ITEMID_Bulletin );
	ContentAdd( pTrade );

 int ps, pi, bps, bpi, s2;
 char msg[90];
 
 bps=packitem(currchar[s]);
 bpi=packitem(i);
 s2=calcSocketFromChar(i);

 inititem(itemcount);
 items[itemcount].ser1=itemcount2/16777216;
 items[itemcount].ser2=itemcount2/65536;
 items[itemcount].ser3=itemcount2/256;
 items[itemcount].ser4=itemcount2;
 items[itemcount].id1=0x1E;	// ITEMID_Bulletin
 items[itemcount].id2=0x5E;
 items[itemcount].x=26;
 items[itemcount].y=0;
 items[itemcount].z=0;
 items[itemcount].cont1=chars[currchar[s]].ser1;
 items[itemcount].cont2=chars[currchar[s]].ser2;
 items[itemcount].cont3=chars[currchar[s]].ser3;
 items[itemcount].cont4=chars[currchar[s]].ser4;
 items[itemcount].layer=0;
 items[itemcount].type=1;
 items[itemcount].dye=0;
 ps=itemcount;
 itemcount++;
 itemcount2++;
 sendbpitem(s, ps); 
 if (s2!=-1) sendbpitem(s2, ps);

 inititem(itemcount);
 items[itemcount].ser1=itemcount2/16777216;
 items[itemcount].ser2=itemcount2/65536;
 items[itemcount].ser3=itemcount2/256;
 items[itemcount].ser4=itemcount2;
 items[itemcount].id1=0x1E;	// ITEMID_Bulletin
 items[itemcount].id2=0x5E;
 items[itemcount].x=26;
 items[itemcount].y=0;
 items[itemcount].z=0;
 items[itemcount].cont1=chars[i].ser1;
 items[itemcount].cont2=chars[i].ser2;
 items[itemcount].cont3=chars[i].ser3;
 items[itemcount].cont4=chars[i].ser4;
 items[itemcount].layer=0;
 items[itemcount].type=1;
 items[itemcount].dye=0;
 pi=itemcount;
 itemcount++;
 itemcount2++;
 sendbpitem(s, pi);
 if (s2!=-1) sendbpitem(s2, pi);

 items[pi].moreb1=items[ps].ser1;
 items[pi].moreb2=items[ps].ser2;
 items[pi].moreb3=items[ps].ser3;
 items[pi].moreb4=items[ps].ser4;
 items[ps].moreb1=items[pi].ser1;
 items[ps].moreb2=items[pi].ser2;
 items[ps].moreb3=items[pi].ser3;
 items[ps].moreb4=items[pi].ser4;
 items[pi].morez=0;
 items[ps].morez=0;

 msg[0]=0x6F; // Header Byte
 msg[1]=0; // Size
 msg[2]=47; // Size
 msg[3]=0; // Initiate
 msg[4]=chars[i].ser1;
 msg[5]=chars[i].ser2;
 msg[6]=chars[i].ser3;
 msg[7]=chars[i].ser4;
 msg[8]=items[ps].ser1;
 msg[9]=items[ps].ser2;
 msg[10]=items[ps].ser3;
 msg[11]=items[ps].ser4;
 msg[12]=items[pi].ser1;
 msg[13]=items[pi].ser2;
 msg[14]=items[pi].ser3;
 msg[15]=items[pi].ser4;
 msg[16]=1;
 sprintf(&(msg[17]), "%s", chars[i].name);
 xsend(s, msg, 47, 0);

 msg[0]=0x6F; // Header Byte
 msg[1]=0; // Size
 msg[2]=47; // Size
 msg[3]=0; // Initiate
 msg[4]=chars[currchar[s]].ser1;
 msg[5]=chars[currchar[s]].ser2;
 msg[6]=chars[currchar[s]].ser3;
 msg[7]=chars[currchar[s]].ser4;
 msg[8]=items[pi].ser1;
 msg[9]=items[pi].ser2;
 msg[10]=items[pi].ser3;
 msg[11]=items[pi].ser4;
 msg[12]=items[ps].ser1;
 msg[13]=items[ps].ser2;
 msg[14]=items[ps].ser3;
 msg[15]=items[ps].ser4;
 msg[16]=1;
 sprintf(&(msg[17]), "%s", chars[currchar[s]].name);
 xsend(s2, msg, 47, 0);

 return ps;
}

     items[i].cont1=items[j].ser1;
     items[i].cont2=items[j].ser2;
     items[i].cont3=items[j].ser3;
     items[i].cont4=items[j].ser4;
     items[i].x=30;
     items[i].y=30;
     items[i].z=9;
     removeitem[1]=items[i].ser1;
     removeitem[2]=items[i].ser2;
     removeitem[3]=items[i].ser3;
     removeitem[4]=items[i].ser4;
     for (k=0;k<now;k++)
     {
      if (perm[k])
      {
       xsend(k, removeitem, 5, 0);
       sendbpitem(k, i);
      }
     }
    }

#endif

	pCharSrc->m_pClient->addItemMessage( "Sorry Trade windows are not done yet", this );
	return( false );
}

bool CChar :: TakeHit( int iDmg, CChar * pSrc )
{
	// Someone hit us.
	// RETURN: false = dead.

	if ( m_StatFlag&STATF_INVUL )	// can't hurt us anyhow.
		return( true );
	if ( m_health <= 0 )	// Already dead.
		return( false );
	if ( ! iDmg ) 
		return( true );

	// defend myself.
	if ( ! m_Act_Targ.IsValidUID() && pSrc != NULL )
	{
		Attack( pSrc );
	}

	// Make blood depending on hit damage.
	// ??? assuming the creature has blood ?
	ITEMID_TYPE id = ITEMID_NOTHING;
	if ( iDmg > 10 )
	{
		id = (ITEMID_TYPE)( ITEMID_BLOOD1 + GetRandVal(ITEMID_BLOOD6-ITEMID_BLOOD1));
	}
	else if ( GetRandVal( iDmg ) > 5 )
	{
		id = ITEMID_BLOOD_SPLAT;
	}
	if ( id )
	{
		CItem * pBlood = ItemCreateBase( id );
		pBlood->SetTimeout( 7 );
		pBlood->PutOnGround( m_p );
	}

	m_health -= iDmg;
	if ( m_health < 0 ) m_health=0;
	UpdateStats( STAT_STR );
	if ( m_health <= 0 )
		return( false );

	SoundChar( CRESND_GETHIT );
	UpdateAnimate( ANIM_GET_HIT );
	return( true );
}

bool CChar :: TakeHitArmor( int iDmg, CChar * pSrc )
{
	// Try to allow the armor or shield to take some damage.

	if ( ! iDmg ) return( true );

	// Can we just dodge some damage ?
	iDmg /=  1 + ( m_Stat[STAT_DEX]/100 );

	// Can we block with shield ?
	if ( Skill_UseQuick( SKILL_PARRYING, 100 ))
	{
		iDmg /= 2;
	}

	if ( m_StatFlag & STATF_Reactive )
	{
		// reflect some damage back.
		if ( pSrc != NULL && GetDist( pSrc ) <= 2 )
		{
			iDmg /= 2;
			pSrc->TakeHit( iDmg, NULL );
			pSrc->Effect( 3, ITEMID_FX_CURSE_EFFECT, this, 9, 6 );
		}
	}

	// absorbed by armor ?
	if ( m_defense ) iDmg /= m_defense;

	return( TakeHit( iDmg, pSrc ));
}

bool CChar :: Hit( CChar * pChar )
{
	// Attempt to hit our target.
	// pChar = the target.

	if ( pChar == NULL ) 
		return( false );
	if ( pChar->m_StatFlag & STATF_DEAD )
		return( false );

	// When do we get our next hit ? (speed)
	Skill_Start( 1 );

	int dist = GetDist( pChar );
	if ( dist > UO_MAP_VIEW_BIG_SIZE ) return( false );
	if ( dist > 1 ) return( true );	// can't hit now.

	if ( ! pChar->m_Act_Targ.IsValidUID())
	{
		// Allow target to Defend self.
		pChar->Attack( this );
	}

	m_StatFlag &= ~( STATF_Invisible | STATF_Hidden );
	UpdateDir( pChar );
	UpdateAnimate( ANIM_ATTACK_1H_WIDE );

	// Calc to hit based on SKILL_TACTICS, DEX and weapon skill used.
	Skill_UseQuick( SKILL_TACTICS, 100 );

	int iSkill = m_Stat[STAT_DEX] - pChar->m_Stat[STAT_DEX];
	if ( GetRandVal(200) >= iSkill + m_Skill[ SKILL_TACTICS ] + m_Skill[ m_Act_Skill ] )
	{
		// We missed.
		SoundChar( GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 );
		return( true );
	}

	// Hit noise.
	SoundChar( CRESND_HIT );

	int iDmg = ( ( m_pCre->m_attack + m_attack ) * 10 * ( 1+( m_Stat[STAT_STR]/100 )));

	// ??? damage the weapon ? m_weapon

	return( pChar->TakeHitArmor( iDmg, this ));
}

void CChar :: NPC_WalkTo()
{
	// Walk towards this point as best we can.
	if ( m_StatFlag & STATF_Freeze ) return;	// can't move.

	CPoint p = m_p;
	DIR_TYPE Dir = p.GetDir( m_Act_p );
	p.Move( Dir );

	// ??? retry dif things if blocked from dest.
	// ok to go here ? 
	if ( ! World.CheckValidMove( p ))
		return;

	CPoint pold = m_p;
	m_dir = Dir;	// Face this direction.
	MoveTo( p );
	UpdateMove( pold );

	// Check for teleporters, traps, etc ?
	if ( CheckLocation())
	{
		// We stepped on teleporter
		return;
	}
}

void CChar :: NPC_Wander()
{
	// Staggering Walk around.
	m_Act_p = m_p;
	m_Act_p.Move( (DIR_TYPE) GetRandVal( DIR_QTY ) );
	NPC_WalkTo();

	// Stop wandering ?
	if ( GetRandVal( m_Stat[ STAT_DEX ] ) < 15 )
		Skill_Setup( SKILL_NONE );
}

bool CChar :: NPC_Follow( CObjUID targ )
{
	// Follow our owner.
	CChar * pChar = targ.CharFind();
	if ( pChar == NULL )
	{
		if ( m_owner.IsValidUID() && ! m_owner.IsItem())
		{
			// follow my owner again.
			Skill_Setup( NPCACT_FOLLOW_OWN );
		}
		else
		{
			// free to do as i wish !
			Skill_Setup( SKILL_NONE );
		}
		return( false );
	}
	// Have to be able to see target to follow.
	if ( CanSee( pChar ))
	{
		m_Act_p = pChar->m_p;
	}
	else
	{
		// Monster may get confused because he can't see you.
		// There is a chance they could forget about you if hidden for a while.
		if ( GetRandVal(75) > m_Stat[STAT_INT] )
			return( false );
	}
	int dist = m_p.GetDist( m_Act_p );
	if ( dist <= 1 ) return( true );
	if ( dist > UO_MAP_VIEW_BIG_SIZE*2 ) return( false );
	NPC_WalkTo();
	return( true );
}

bool CChar :: NPC_Beg( CChar * pChar )
{
	static const char * Beggar[] =
	{
		"Can you spare any gold?",
		"I have children to feed.",
		"I haven't eaten in days.",
		"Please? Just a few coins.",
	};

	if ( pChar == NULL || pChar == this ) 
	{
		m_Act_Targ.ClearUID();
		Skill_Setup( SKILL_NONE );
		return( false );
	}

	if ( pChar->m_NPC_Brain ||
		 ( pChar->GetUID() == m_PrvTarg )) 
		 return( false );	// Don't beg from NPC's or the PC you just begged from

	// Time to beg
	Skill_Setup( SKILL_BEGGING );
	if ( ! GetRandVal(5))
		Speak( Beggar[ GetRandVal( COUNTOF( Beggar )) ] );
	if ( m_Act_Targ == pChar->GetUID())
	{
		if ( ! CanSee( pChar ))
		{
			if ( GetRandVal(75) > m_Stat[STAT_INT] ) // Dumb beggars think I'm gone
			{
				m_Act_Targ.ClearUID();
				Skill_Setup( SKILL_NONE );
			}
		}
		return( true );
	}
	m_dir = GetDir( pChar );	// face PC
	m_Act_Targ = pChar->GetUID();
	return( NPC_Follow( m_Act_Targ ));
}

void CChar :: NPC_Idle()
{
	// Free to do as we please.
	// Idle NPC's should try to take some action.

	// Make some random noise 
	//
	if ( GetRandVal( 10 ) == 0 )
	{
		SoundChar( GetRandVal(2) ? CRESND_RAND1 : CRESND_RAND2 );
	}
	if ( m_StatFlag & STATF_War ) 
	{
		// Continue with my attack.
		Attack( m_Act_Targ.CharFind() );
		return;
	}

	if ( m_owner.IsValidUID())
	{
		// pets should protect there owners unless told otherwise.
		CChar * pChar = m_owner.CharFind();
		if ( pChar != NULL && ( pChar->m_StatFlag & STATF_War ))
		{
			Attack( pChar->m_Act_Targ.CharFind() );
			return;
		}
	}

	static const char * Healer[] =
	{
		"Thou art dead, but 'tis within my power to resurrect thee.  Live!",
     	"Allow me to resurrect thee ghost.  Thy time of true death has not yet come.",
        "Perhaps thou shouldst be more careful.  Here, I shall resurrect thee.",
     	"Live again, ghost!  Thy time in this world is not yet done.",
     	"I shall attempt to resurrect thee.",
	};

	// Take a look around for other people/chars.

	CWorldSearch Area( m_p, UO_MAP_VIEW_SIZE );
	while ( 1 )
	{
		CChar * pChar = Area.GetChar();
		if ( pChar == NULL ) break;
		if ( pChar == this ) continue;
		if ( m_NPC_Brain != NPCBRAIN_HEALER && ( pChar->m_StatFlag & STATF_DEAD )) continue;
		if ( ! CanSee( pChar )) continue;

		// m_NPC_Brain = type of brain creature has.
		switch ( m_NPC_Brain )
		{
		case NPCBRAIN_HEALER:
			// Healers should look around for ghosts.
			if ( ! ( pChar->m_StatFlag & STATF_DEAD )) continue;
			if ( GetDist( pChar ) > 3 ) continue;

			// Attempt to res.
			Speak( Healer[ GetRandVal( COUNTOF( Healer )) ] );
			UpdateAnimate( ANIM_CAST_AREA );
			pChar->Spell_Effect( SPELL_Resurrection, this, 100 );
			return;
	
		case NPCBRAIN_GUARD:
			// Guards should look around for criminals or nasty creatures.

			if (( pChar->m_StatFlag & STATF_Criminal ) ||
				pChar->m_Stat[ STAT_Karma ] <= -9000 )
			{
				// Attack ! ?? Near target.
				Spell_Teleport( pChar->m_p );
				Speak( "Thou shalt regret thine actions, swine!" );
				Attack( pChar );
				return;
			}
			break;

		case NPCBRAIN_BEGGAR:
			if ( NPC_Beg( pChar ))
				return;
			break;

		case NPCBRAIN_BANKER:
		case NPCBRAIN_VENDOR:
		case NPCBRAIN_ANIMAL:
		case NPCBRAIN_HUMAN:
			// ??? Attack others if we are evil. we should pick the closest 
			if ( m_Stat[ STAT_Karma ] <= -9000 )		// I am evil.
			{
				Attack( pChar );
				return;
			}
		}
	}

	// dex determines how jumpy they are.
	// Decide to wander about ? 
	if ( GetRandVal( m_Stat[ STAT_DEX ] ) > 30 )
		Skill_Setup( NPCACT_WANDER );
}

bool CChar :: NPC_Cast()
{
	// cast a spell.

	if ( m_Skill[ SKILL_MAGERY ] < 200 )
		return( false );
	if ( GetRandVal( m_Skill[ SKILL_MAGERY ] ) < 400 )
		return( false );

	// select proper spell.
	// defensive spells ???
	SPELL_TYPE spell = SPELL_Harm;
	int imaxspell = min( ( m_Skill[ SKILL_MAGERY ] / 10 ) * 8, SPELL_BOOK_QTY );
	for ( int i = GetRandVal( imaxspell ); i<imaxspell; i++ )
	{
		if ( ! ( Spells[i].m_wFlags & SPELLFLAG_HARM )) continue;
		spell = (SPELL_TYPE) i;
		break;
	}

	if ( ! Spell_CanCast( spell ))
		return( false );

	Skill_Setup( SKILL_MAGERY );
	m_Act_Spell = spell;
	if ( Spell_Cast( m_Act_Targ, m_Act_Targ.CharFind()->m_p ))
		return( true );
	Skill_Cleanup();
	return( false );
}

void CChar :: NPC_Action()
{
	// default next tick
	SetTimeout( ( m_Stat[STAT_DEX] > 40 ) ? 1 : 2 );

	if ( m_owner.IsValidUID())
	{
		// ??? How happy are we with being a pet ?

	}

	switch ( m_Act_Skill )
	{
	case SKILL_NONE:
		// We should try to do something.
		NPC_Idle();
		return;

	case SKILL_BEGGING:
		NPC_Beg( m_Act_Targ.CharFind() );
		return;

	case SKILL_WRESTLING:
	case SKILL_ARCHERY:
	case SKILL_FENCING:
	case SKILL_MACEFIGHTING:
	case SKILL_SWORDSMANSHIP:
		// Allow magic attacks as well if m_NPC_Brain type is ok.

		if ( ! NPC_Follow( m_Act_Targ ))
			break;

		// Maybe i'll cast a spell if I can
		if ( NPC_Cast())
			return;
		break;
	case NPCACT_FOLLOW_TARG:
		NPC_Follow( m_Act_Targ );
		break;
	case NPCACT_FOLLOW_OWN:
		NPC_Follow( m_owner );
		break;
	case NPCACT_STAY:
		break;
	case NPCACT_GOTO:
		NPC_WalkTo();
		break;
	case NPCACT_WANDER:
		NPC_Wander();
		break;
	case NPCACT_TALK:
	case NPCACT_TALK_FOLLOW:
		if ( m_NPC_Speech )
		{
			static const char * szText[] =
			{
				"Well it was nice speaking to you %s but i must go about my business",
				"Nice speaking to you %s", 
			};
			CChar * pChar = m_Act_Targ.CharFind();
			const char * pName = ( pChar == NULL ) ? "" : pChar->GetName();
			CString sMsg;
			sMsg.Format( szText[ GetRandVal( COUNTOF( szText )) ], pName );
			Speak( sMsg );
		}
		Skill_Setup( NPCACT_WANDER );
		break;
	}
}

void CChar :: OnTick()
{
	// Get a timer tick when our timer expires.
	if ( m_StatFlag & STATF_DEAD ) return;

	if ( m_health <= 0 )	// We can only die on our own tick.
	{
		Death();
		return;
	}

	// decay equipped items (spells)
	CItem* pItemNext; 
	CItem* pItem=GetContentHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( pItem->m_timeout && ( pItem->m_timeout <= World.m_Clock_Time ))
		{
			pItem->OnTick();
		}
	}

#define REGENRATE1	8 // Seconds to heal ONE hp
#define REGENRATE2	3 // Seconds to heal ONE stm
#define REGENRATE3	5 // Seconds to heal ONE mn

	if ( m_regen_health <= World.m_Clock_Time )
	{
		m_regen_health=World.m_Clock_Time + REGENRATE1;
		if ( m_health != m_Stat[STAT_STR] )
		{
			if ( m_health > m_Stat[STAT_STR] ) m_health--; else m_health++;
			UpdateStats( STAT_STR );
		}
	}
	if ( m_regen_mana <= World.m_Clock_Time )
	{
		m_regen_mana = World.m_Clock_Time + REGENRATE3;
		if ( m_mana != m_Stat[STAT_INT] )
		{
			if ( m_mana > m_Stat[STAT_INT] ) m_mana--; else m_mana++;
			UpdateStats( STAT_INT );
		}
	}
	if ( m_regen_stam <= World.m_Clock_Time )
	{
		m_regen_stam = World.m_Clock_Time + REGENRATE2;
		if ( m_stam != m_Stat[STAT_DEX] )
		{
			if ( m_stam > m_Stat[STAT_DEX] ) m_stam--; else m_stam++;
			UpdateStats( STAT_DEX );
		}
	}
	if ( m_regen_food <= World.m_Clock_Time )
	{
static const char * HungerMessage[] =
{
	"You are weak with hunger",
	"You are starving",
	"You feel very hungry",
	"You feel hungry",
	"You feel a might pekish",
};
		m_regen_food = World.m_Clock_Time + 60*5;
		if ( m_food > 0 )
		{
			m_food --;
		}
		if ( m_food < COUNTOF(HungerMessage))
		{
			SysMessage( HungerMessage[m_food] );
		}
	}
	if ( m_timeout <= World.m_Clock_Time )
	{
		// My turn to do some action.
		if ( m_Act_Skill != SKILL_TRACKING )
		{
			Skill_Done();
		}
		if ( m_NPC_Brain )	// What to do next ?
		{
			NPC_Action();
		}
	}

	// Check location periodically for standing in fire fields, traps, etc.
	CheckLocation( true );
}

