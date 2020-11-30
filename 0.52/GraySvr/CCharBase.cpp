//
// CCharBase.cpp
// Copyright Menace Software (www.menasoft.com).
//
#include "graysvr.h"	// predef header.

#ifdef COMMENT
		// 0x400 = stuff not directly linked to graphic.
		// More sounds. diff colors ?
		{ NPCID_Cougar,		"Cougar",		CREID_Panther,		0x2119,	0x073, NULL, 0, 0, 0xFFFFF, CAN_C_WALK },		// black
		{ NPCID_Lion,		"Lion",			CREID_Panther,		0x2119, 0x1b2, NULL, 0, 0, 0xFFFFF, CAN_C_WALK},		// yellow
		{ NPCID_Bird,		"Bird",			CREID_BIRD,			0x20ee, 0x017, NULL, 0, 0, 0xFFFFF, CAN_C_WALK},
		{ NPCID_JungleBird,	"Jungle bird",	CREID_BIRD,			0x20ee, 0x0ad, NULL, 0, 0, 0xFFFFF, CAN_C_WALK},
		{ NPCID_Parrot,		"Parrot",		CREID_BIRD,			0x20ee, 0x0bf, NULL, 0, 0, 0xFFFFF, CAN_C_WALK},
		{ NPCID_Raven,		"Raven",		CREID_BIRD,			0x20ee, 0x0d1, NULL, 0, 0, 0xFFFFF, CAN_C_WALK},	// black
#endif

/////////////////////////////////////////////////////////////////
// -CCharBaseDisp

// #define B(x) (1<<(x))

	static CCharBaseDisp CharBaseDisp[] =	// ??? Put all this in GRAYCHAR.SCP
	{
		{ CREID_AIR_ELEM,		ITEMID_TRACK_ELEM_AIR,		CAN_C_WALK|CAN_C_FLY },
		{ CREID_EARTH_ELEM,		ITEMID_TRACK_ELEM_EARTH,	CAN_C_WALK|CAN_C_GHOST },
		{ CREID_FIRE_ELEM,		ITEMID_TRACK_ELEM_FIRE,		CAN_C_WALK },
		{ CREID_WATER_ELEM,		ITEMID_TRACK_ELEM_WATER,	CAN_C_WALK|CAN_C_SWIM },

		{ CREID_OGRE,			ITEMID_TRACK_OGRE,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },

		{ CREID_ETTIN,			ITEMID_TRACK_ETTIN,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },
		{ CREID_ETTIN_AXE,		ITEMID_TRACK_ETTIN,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },

		{ CREID_LIZMAN,			(ITEMID_TYPE) 0x20ca,		CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },
		{ CREID_LIZMAN_SPEAR,	(ITEMID_TYPE) 0x20ca,		CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },
		{ CREID_LIZMAN_MACE,	(ITEMID_TYPE) 0x20ca,		CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW },

		{ CREID_DAEMON,			(ITEMID_TYPE) 0x20D3,		CAN_C_WALK|CAN_C_FLY|CAN_C_USEHANDS},
		{ CREID_DAEMON_SWORD,	(ITEMID_TYPE) 0x20D3,		CAN_C_WALK|CAN_C_FLY|CAN_C_USEHANDS},

		{ CREID_ORC,			(ITEMID_TYPE) 0x20E0,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_ORC_CLUB,		(ITEMID_TYPE) 0x20E0,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_ORC_LORD,		(ITEMID_TYPE) 0x20E0,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},

		{ CREID_RATMAN,			(ITEMID_TYPE) 0x20e3,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_RATMAN_CLUB,	(ITEMID_TYPE) 0x20e3,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_RATMAN_SWORD,	(ITEMID_TYPE) 0x20e3,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_Rat,			(ITEMID_TYPE) 0x2123,		CAN_C_WALK|CAN_C_EATRAW },
		{ CREID_GiantRat,		(ITEMID_TYPE) 0x20D0,		CAN_C_WALK|CAN_C_EATRAW},

		{ CREID_TROLL_SWORD,	ITEMID_TRACK_TROLL,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW},
		{ CREID_TROLL,			ITEMID_TRACK_TROLL,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW},
		{ CREID_TROLL_MACE,		ITEMID_TRACK_TROLL,			CAN_C_WALK|CAN_C_USEHANDS|CAN_C_EATRAW},

		{ CREID_LICH,			(ITEMID_TYPE) 0x20f8,		CAN_C_WALK|CAN_C_USEHANDS},
		{ CREID_SPECTRE,		(ITEMID_TYPE) 0x2109,		CAN_C_WALK},
		{ CREID_ZOMBIE,			(ITEMID_TYPE) 0x20ec,		CAN_C_WALK|CAN_C_EATRAW},

		{ CREID_SKELETON,		(ITEMID_TYPE) 0x20e7,		CAN_C_WALK},
		{ CREID_SKEL_AXE,		(ITEMID_TYPE) 0x20e7,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},
		{ CREID_SKEL_SW_SH,		(ITEMID_TYPE) 0x20e7,		CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS},

		{ CREID_DRAGON_GREY,	(ITEMID_TYPE) 0x20d6,		CAN_C_WALK|CAN_C_FLY|CAN_C_EATRAW},
		{ CREID_DRAGON_RED,		(ITEMID_TYPE) 0x20d6,		CAN_C_WALK|CAN_C_FLY|CAN_C_EATRAW},
		{ CREID_DRAKE_GREY,		(ITEMID_TYPE) 0x20d6,		CAN_C_WALK|CAN_C_FLY|CAN_C_EATRAW},
		{ CREID_DRAKE_RED,		(ITEMID_TYPE) 0x20d6,		CAN_C_WALK|CAN_C_FLY|CAN_C_EATRAW},

		{ CREID_GIANT_SNAKE,	(ITEMID_TYPE) 0x20fe,		CAN_C_WALK|CAN_C_SWIM|CAN_C_NONHUMANOID},
		{ CREID_Snake,			(ITEMID_TYPE) 0x20fc,		CAN_C_WALK|CAN_C_SWIM|CAN_C_NONHUMANOID},
		{ CREID_SEA_SERP,		ITEMID_TRACK_SEASERP,		CAN_C_SWIM|CAN_C_NONHUMANOID},

		{ CREID_EAGLE,			(ITEMID_TYPE) 0x211d,		CAN_C_WALK|CAN_C_FLY },
		{ CREID_BIRD,			(ITEMID_TYPE) 0x20ee,		CAN_C_WALK|CAN_C_FLY },
		{ CREID_HARPY,			(ITEMID_TYPE) 0x20dc,		CAN_C_WALK|CAN_C_FLY|CAN_C_EATRAW },
		{ CREID_MONGBAT,		(ITEMID_TYPE) 0x20f9,		CAN_C_WALK|CAN_C_FLY },

		{ CREID_CORPSER,		(ITEMID_TYPE) 0x20d2,		CAN_C_EATRAW|CAN_C_NONHUMANOID },
		{ CREID_REAPER,			(ITEMID_TYPE) 0x20fa,		CAN_C_WALK|CAN_C_NONHUMANOID },

		{ CREID_BrownBear,		(ITEMID_TYPE) 0x20CF,		CAN_C_WALK|CAN_C_EATRAW},
		{ CREID_PolarBear,		(ITEMID_TYPE) 0x20E1,		CAN_C_WALK|CAN_C_EATRAW},
		{ CREID_GrizzlyBear,	(ITEMID_TYPE) 0x211e,		CAN_C_WALK|CAN_C_EATRAW},

		{ CREID_Hart,			(ITEMID_TYPE) 0x20D4,		CAN_C_WALK },
		{ CREID_Deer,			(ITEMID_TYPE) 0x20d4,		CAN_C_WALK },

		{ CREID_Pig,			(ITEMID_TYPE) 0x2101,		CAN_C_WALK },
		{ CREID_Boar,			(ITEMID_TYPE) 0x2101,		CAN_C_WALK },

		{ CREID_HORSE1,			ITEMID_TRACK_HORSE,			CAN_C_WALK },
		{ CREID_HORSE2,			(ITEMID_TYPE) 0x211F,		CAN_C_WALK },
		{ CREID_HORSE3,			(ITEMID_TYPE) 0x2124,		CAN_C_WALK },
		{ CREID_HORSE4,			(ITEMID_TYPE) 0x2121,		CAN_C_WALK },
		{ CREID_HORSE_PACK,		ITEMID_TRACK_PACK_HORSE,	CAN_C_WALK },

		{ CREID_Llama,			(ITEMID_TYPE) 0x20f6,		CAN_C_WALK },
		{ CREID_LLAMA_PACK,		ITEMID_TRACK_PACK_LLAMA,	CAN_C_WALK },

		{ CREID_Sheep,			(ITEMID_TYPE) 0x20EB,		CAN_C_WALK },
		{ CREID_Sheep_Sheered,	(ITEMID_TYPE) 0x20E6,		CAN_C_WALK },
		{ CREID_Goat,			(ITEMID_TYPE) 0x2108,		CAN_C_WALK },
		{ CREID_Chicken,		(ITEMID_TYPE) 0x20D1,		CAN_C_WALK },

		{ CREID_Dog,			(ITEMID_TYPE) 0x211c,		CAN_C_WALK|CAN_C_EATRAW},
		{ CREID_Wolf,			(ITEMID_TYPE) 0x20D5,		CAN_C_WALK|CAN_C_EATRAW},

		{ CREID_Cat,			(ITEMID_TYPE) 0x211b,		CAN_C_WALK},
		{ CREID_Panther,		(ITEMID_TYPE) 0x2119,		CAN_C_WALK|CAN_C_EATRAW},

		{ CREID_Cow_BW,			(ITEMID_TYPE) 0x2103,		CAN_C_WALK},
		{ CREID_Cow2,			(ITEMID_TYPE) 0x2103,		CAN_C_WALK},
		{ CREID_Bull_Brown,		(ITEMID_TYPE) 0x20f0,		CAN_C_WALK},
		{ CREID_Bull2,			(ITEMID_TYPE) 0x20f0,		CAN_C_WALK},

		{ CREID_Alligator,		(ITEMID_TYPE) 0x20DA,		CAN_C_WALK|CAN_C_SWIM|CAN_C_EATRAW},
		{ CREID_Dolphin,		(ITEMID_TYPE) 0x20f1,		CAN_C_SWIM|CAN_C_EATRAW|CAN_C_NONHUMANOID},
		{ CREID_GORILLA,		(ITEMID_TYPE) 0x20c9,		CAN_C_WALK},
		{ CREID_Rabbit,			ITEMID_TRACK_RABBIT,		CAN_C_WALK}, // 20E2, but 2125 is the pet.
		{ CREID_Walrus,			(ITEMID_TYPE) 0x20ff,		CAN_C_WALK|CAN_C_SWIM|CAN_C_EATRAW|CAN_C_NONHUMANOID},

		{ CREID_GIANT_SPIDER,	(ITEMID_TYPE) 0x20fd,		CAN_C_WALK|CAN_C_NONHUMANOID },
		{ CREID_SCORP,			(ITEMID_TYPE) 0x20e4,		CAN_C_WALK|CAN_C_NONHUMANOID },
		{ CREID_SLIME,			(ITEMID_TYPE) 0x20e8,		CAN_C_WALK|CAN_C_SWIM|CAN_C_EATRAW|CAN_C_NONHUMANOID },
		{ CREID_WISP,			ITEMID_TRACK_WISP,			CAN_C_WALK|CAN_C_FLY|CAN_C_NONHUMANOID },
		{ CREID_GARGOYLE,		(ITEMID_TYPE) 0x20d9,		CAN_C_WALK|CAN_C_FLY|CAN_C_USEHANDS},
		{ CREID_GAZER,			(ITEMID_TYPE) 0x20f4,		CAN_C_WALK|CAN_C_FLY|CAN_C_NONHUMANOID},
		{ CREID_HEADLESS,		(ITEMID_TYPE) 0x210a,		CAN_C_WALK|CAN_C_NONHUMANOID },

		// 0x46 = Terathen Warrior
		{ CREID_Tera_Warrior,	ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x47 = Terathen Drone
		{ CREID_Tera_Drone,		ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x48 = Terathen Matriarch
		{ CREID_Tera_Matriarch,	ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },

		// 0x4b = Titan
		{ CREID_Titan,			ITEMID_TRACK_OGRE,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x4c = Cyclops
		{ CREID_Cyclops,		ITEMID_TRACK_OGRE,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x50 = Giant Toad
		{ CREID_Giant_Toad,		ITEMID_TRACK_MAN,			CAN_C_WALK },
		// 0x51 = Bull Frog
		{ CREID_Bull_Frog,		ITEMID_TRACK_MAN,			CAN_C_WALK },

		// 0x55 = Ophidian Mage
		{ CREID_Ophid_Mage,		ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x56 = Ophidian Warrior
		{ CREID_Ophid_Warrior,	ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		// 0x57 = Ophidian Queen
		{ CREID_Ophid_Queen,	ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },

		// 0x5f = (Unknown Sea Creature)
		{ CREID_SEA_Creature,	ITEMID_TRACK_SEASERP,		CAN_C_SWIM|CAN_C_NONHUMANOID },
		// 0xce = Lava Lizard
		{ CREID_LavaLizard,		ITEMID_TRACK_MAN,			CAN_C_WALK },
		// 0xd2 = Desert Ostard (ridable)
		{ CREID_Ostard_Desert,	ITEMID_TRACK_HORSE,			CAN_C_WALK },
		// 0xda = Frenzied Ostard (ridable)
		{ CREID_Ostard_Frenz,	ITEMID_TRACK_HORSE,			CAN_C_WALK },
		// 0xdb = Forest Ostard (ridable)
		{ CREID_Ostard_Forest,	ITEMID_TRACK_HORSE,			CAN_C_WALK },

		// all here are humanish
		{ CREID_VORTEX,			ITEMID_FX_BLADES,			CAN_C_WALK|CAN_C_NONHUMANOID },
		{ CREID_BLADES,			ITEMID_FX_BLADES,			CAN_C_WALK|CAN_C_NONHUMANOID },

		{ CREID_MAN,			ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS },
		{ CREID_WOMAN,			ITEMID_TRACK_WOMAN,			CAN_C_WALK|CAN_C_EQUIP|CAN_C_USEHANDS },

		// not to be picked randomly
		{ CREID_GHOSTMAN,		ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_GHOST },
		{ CREID_GHOSTWOMAN,		ITEMID_TRACK_WOMAN,			CAN_C_WALK|CAN_C_GHOST },

		// This artwork was removed from t2a ???
		{ CREID_CHILD_MB,		ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		{ CREID_CHILD_MD,		ITEMID_TRACK_MAN,			CAN_C_WALK|CAN_C_USEHANDS },
		{ CREID_CHILD_FD,		ITEMID_TRACK_WOMAN,			CAN_C_WALK|CAN_C_USEHANDS },
		{ CREID_CHILD_FB,		ITEMID_TRACK_WOMAN,			CAN_C_WALK|CAN_C_USEHANDS },
	};

CREID_TYPE CCharBaseDisp::GetRandBase() // static
{
	return( CharBaseDisp[ GetRandVal(COUNTOF( CharBaseDisp )) ].m_dispID );
}

CREID_TYPE CCharBaseDisp::FindCharTrack( ITEMID_TYPE trackID )	// static
{
	for ( int j=0; g_Item_Horse_Mounts[j][0]; j++ )
	{
		if ( trackID == g_Item_Horse_Mounts[j][0] )
			return( (CREID_TYPE) g_Item_Horse_Mounts[j][1] );
	}

	// Find the eqiv creature id.
	for ( int i=0; 1; i++ )
	{
		if ( i>=COUNTOF( CharBaseDisp ))
			return( CREID_INVALID );
		if ( CharBaseDisp[i].m_trackID == trackID )
		{
			return( CharBaseDisp[i].m_dispID );
		}
	}
}

/////////////////////////////////////////////////////////////////
// -CCharBase

CCharBase::CCharBase( CREID_TYPE id ) : CBaseBase( id )
{
	m_iHireDayWage = 0;
	m_trackID = ITEMID_TRACK_WISP;
	m_soundbase = 0;
	m_defense = 0;
	m_Anims = 0xFFFFFF;
  	m_MaxFood = 15;			// Default value
	m_Str = 1;
	m_Dex = 1;

	if ( IsValidDispID(id))
	{
		// in display range.
		for ( int i=0; i<COUNTOF( CharBaseDisp ); i++ )
		{
			if ( CharBaseDisp[i].m_dispID == id )
			{
				m_trackID = CharBaseDisp[i].m_trackID;
				m_Can = CharBaseDisp[i].m_Can | CAN_C_INDOORS | CAN_C_MOVE_ITEMS;
				break;
			}
		}
	}
	else
	{
		m_wDispIndex = CREID_INVALID;	// must read from SCP file later
	}
}

const TCHAR * CCharBase::GetTradeName() const
{
	// From "Bill the carpenter" or "#HUMANMALE the Carpenter",
	// Get "Carpenter"
	const TCHAR * pName = CBaseBase::GetTypeName();
	if ( pName[0] != '#' )
		return( pName );

	const TCHAR * pSpace = strchr( pName, ' ' );
	if ( pSpace == NULL )
		return( pName );

	pSpace++;
	if ( ! strnicmp( pSpace, "the ", 4 )) pSpace += 4;
	return( pSpace );
}

bool CCharBase::IsFemaleID( CREID_TYPE id )	// static
{
	// ??? CAN_FEMALE
	// if ( m_Can & CAN_FEMALE ) return( true );

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

void CCharBase::DupeCopy( const CCharBase * pDef )
{
	m_trackID = pDef->m_trackID;
	m_soundbase = pDef->m_soundbase;

	m_sCorpseResources = pDef->m_sCorpseResources;	// RESOURCES=10 MEAT
	m_sFoodType = pDef->m_sFoodType;
	m_MaxFood = pDef->m_MaxFood;

	m_defense = pDef->m_defense;
	m_attackBase = pDef->m_attackBase;
	m_attackRange = pDef->m_attackRange;
	m_Anims = pDef->m_Anims;
	m_Can = pDef->m_Can;
}

bool CCharBase::SetDispID( CREID_TYPE id )
{
	// Setting the "ID" for this.
	if ( id == GetID())
		return true;
	if ( id == GetDispID())
		return true;
	if ( ! IsValidDispID( id ))
	{
		DEBUG_ERR(( "Creating char SetDispID(%d) > %d\n", id, CREID_QTY ));
		return false; // must be in display range.
	}

	// Copy the rest of the stuff from the display base.
	// NOTE: This can mess up g_Serv.m_scpChar offset!!
	CCharBase * pDef = FindCharBase( id );
	if ( pDef == NULL )
	{
		DEBUG_ERR(( "Creating char SetDispID(%d) BAD\n", id ));
		return( false );
	}

	// m_sName = pDef->m_sName;	// Base type name. should set this itself. (don't overwrite it!)
	m_wDispIndex = id;
	DupeCopy( pDef );
	return( true );
}

void CCharBase::SetFoodType( const TCHAR * pszFood )
{
  	m_sFoodType = pszFood;

  	// Try to determine the real value
  	TCHAR szFoodString[256];
  	strcpy( szFoodString, pszFood );

  	TCHAR * pTok = strtok(szFoodString, ",");
  	if ( pTok == NULL)
		return;

	TCHAR szFoodName[256];
  	TCHAR szTmp[256];
  	TCHAR *pTmp = &szTmp[0];

  	Parse(pTok, &pTmp, " ");
  	pTok = pTmp;
  	if (pTok == NULL)
		return;

	strcpy( szFoodName, pTok);
	Parse(pTok, &pTmp, " ");
	if (pTok != NULL)
	{
		if ( strcmpi(szFoodName, "GRASS") == 0)
			m_MaxFood = atoi(pTok) / 10;
		else
			m_MaxFood = atoi(pTok);
	}
}

enum
{
	CT_ANIM,
	CT_ARMOR,
	CT_DEF,		
	CT_DESIRES,
	CT_DEX,
	CT_FOODTYPE,
	CT_HIREDAYWAGE,
	CT_ICON,
	CT_ID,
	CT_MAXFOOD,
	CT_RESOURCES,
	CT_SOUND,
	CT_STR,
	CT_TEVENTS,
	CT_TSPEECH,
};

const TCHAR * CCharBase::sm_KeyTable[] =
{
	"ANIM",
	"ARMOR",
	"DEF",		
	"DESIRES",
	"DEX",
	"FOODTYPE",
	"HIREDAYWAGE",
	"ICON",
	"ID",
	"MAXFOOD",
	"RESOURCES",
	"SOUND",
	"STR",
	"TEVENTS",
	"TSPEECH",
};

bool CCharBase::r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pChar )
{
	if ( ! strcmpi( pKey, "JOB" ))
	{
		sVal = GetTradeName();
		return( true );
	}

	switch ( FindTableSorted( pKey, sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case CT_ANIM:
		sVal.FormatHex( m_Anims );
		break;
	case CT_ARMOR:
	case CT_DEF:
		sVal.FormatVal( m_defense );
		break;
	case CT_DESIRES:
		sVal = m_sDesires;
		break;
	case CT_DEX:
		sVal.FormatVal( m_Dex );
		break;
	case CT_FOODTYPE:
		sVal = m_sFoodType;
		break;
	case CT_HIREDAYWAGE:
		sVal.FormatVal( m_iHireDayWage );
		break;
	case CT_ICON:
		sVal.FormatHex( m_trackID );
		break;
	case CT_MAXFOOD:
		sVal.FormatVal( m_MaxFood );
		break;
	case CT_RESOURCES:
		sVal = m_sCorpseResources;
		break;
	case CT_SOUND:
		sVal.FormatHex( m_soundbase );
		break;
	case CT_STR:
		sVal.FormatVal( m_Str );
		break;
	case CT_TEVENTS:
		m_Events.WriteFragList( sVal );
		break;
	case CT_TSPEECH:
		m_Speech.WriteFragList( sVal );
		break;
	default:
		return( CBaseBase::r_WriteVal( pKey, sVal ));
	}
	return( true );
}

bool CCharBase::r_LoadVal( CScript & s )
{
	if ( ! s.HasArgs()) 
		return( false );
	switch ( FindTableSorted( s.GetKey(), sm_KeyTable, COUNTOF( sm_KeyTable )))
	{
	case CT_ANIM:
		m_Anims = s.GetArgRange();
		break;
	case CT_ARMOR:
	case CT_DEF:
		m_defense = s.GetArgVal();
		break;
	case CT_DESIRES:
		m_sDesires = s.GetArgStr();
		break;
	case CT_DEX:
		m_Dex = s.GetArgVal();
		break;
	case CT_FOODTYPE:
		SetFoodType( s.GetArgStr());
		break;
	case CT_HIREDAYWAGE:
		m_iHireDayWage = s.GetArgVal();
		break;
	case CT_ICON:
		{
			ITEMID_TYPE id = (ITEMID_TYPE) s.GetArgHex();
			if ( id >= ITEMID_MULTI )
			{
				return( false );
			}
			m_trackID = id;
		}
		break;
	case CT_ID:
		return SetDispID( (CREID_TYPE) s.GetArgHex());
	case CT_MAXFOOD:
		m_MaxFood = s.GetArgVal();
		break;
	case CT_RESOURCES:
		m_sCorpseResources = s.GetArgStr();
		break;
	case CT_SOUND:
		m_soundbase = s.GetArgVal();
		break;
	case CT_STR:
		m_Str = s.GetArgVal();
		break;
	case CT_TEVENTS:
		return( m_Events.r_LoadVal( s ));
	case CT_TSPEECH:
		return( m_Speech.r_LoadVal( s ));
	default:
		return( CBaseBase::r_LoadVal( s ));
	}
	return true;
}

CBaseBaseArray * CCharBase::GetBaseArray() const
{
	return( &(g_Serv.m_CharBase));
}

bool CCharBase::Load()
{
	CScriptLock s;
	if ( ! ScriptLockBase(s))
	{
		// don't allow non-script creatures. (even if in general display range)
		if ( GetInstances() == 0 ) // scripts got swaped out ?!
		{
			// Whatever, it's gone, but this is fine.
			bool fCheck = g_Serv.m_CharBase.DeleteOb( this );
			ASSERT( fCheck );
		}
		else
		{
			UnLoad();	// hmm this is a problem !
		}
		return( false );
	}

	// Do a prelim read from the script file.
	while ( s.ReadKeyParse())
	{
		if ( s.IsKey("ITEM"))	// This is past the stuff we care about.
			break;
		r_LoadVal( s );
	}

	if ( ! IsValidDispID( GetDispID()))
	{
		// Bad display id ? could be an advance gate.
		DEBUG_WARN(( "Char Script 0%x has bad dispid 0%x\n", GetID(), GetDispID()));
		m_wDispIndex = CREID_MAN;
	}
	if ( m_sName.IsEmpty())
	{
		DEBUG_ERR(( "Char Script 0%x has no name!\n", GetID()));
		return( false );
	}
	return( true );
}

CCharBase * CCharBase::FindCharBase( CREID_TYPE baseID ) // static
{
	if ( baseID <= 0 || baseID >= NPCID_Qty )
		return( NULL );

	CCharBase * pBase;

	// find it (or near it) if already loaded.
	int index = g_Serv.m_CharBase.FindKey( baseID );
	if ( index >= 0 )	// found it.
	{
		pBase = STATIC_CAST <CCharBase *> ( g_Serv.m_CharBase[index] );
		if ( pBase->IsLoaded())
			return( pBase );
	}
	else
	{
		// create a new base.
		pBase = new CCharBase( baseID );
		g_Serv.m_CharBase.AddSortKey( pBase, baseID );
	}

	if ( ! pBase->Load())
	{
		return( NULL );		
	}

	return( pBase );
}

