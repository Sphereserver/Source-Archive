//
// GRAYSRV.CPP.
// game/login server for uo client 
// http://www.ayekan.com/awesomedev
// or see my page: http://www.menasoft.com for more details.
//
// Dennis Robinson [menace@unforgettable.com], http://www.menasoft.com
// Damian Tedrow [damiant@seanet.com], http://www.ayekan.com/awesomedev
// Nix [nixon@mediaone.net]
// Westy [petewest@mindspring.com]
// Terry [terryde@dallas.net]
// Alluvian [alluvian@onramp.net]
// Keif Mayers [kmayers@gci.net]
//
// In works:
// game/chess board.
// Weapons damage. weapon animation. skill check and wrestling = no damage ?
// Gray Agent Program to swap the login.cfg on client machine. (Terry)
// New spells, Spells damage, and effects. (Alluvian)
// Spawns and respawns of items destroyed/killed or taken. (me)
// Regions - music and named regions. Adapt UOMAPX to create these.
//
// BUGS:
// Fall thru floor of houses.
// Strange accumulated lag ?
// dclick sometimes not work at all ?
// Skills used on corpse can't target.
// Skills cause the "Criminal" dialog to appear on targetting ?
//   Missed clicks on skills and spells. Target object required flag ?
// Scrolls to spell books.
// Extra spells in spell book ? Necromancy can crash client
// 
// Needs:
// Increase the number of regions. Timer Items List ?
// Tune Skill Checking and skill delays along with spell delays
// Save world file. (binary) 
// Free guest log ins.
// Enum creatures and there avail anims (blink out when bad anim occurs)
// Enum flipped objects, dif num objects, anim objects.
// Tweak menu.
// Secure trade dialog.
// Skills - Raw material skills - Lumber jacking, Mining, Fishing 
// Skills - Production skills - Smithy, Bowery, Carpentry, Tinkering, Tailoring, Inscrip, Alchemy 
// Skills - Transform skills - Cooking, Cartography,  
// Skills - Information skills - Forensics, Taste ID 
//  Mining equipment and smelting 
//  Sewing and scissors 
//  Bandages and healing 
// Boats
// Shrines for Resurrection 
// Move off line chars out of world file on world save ?
// Single OnTick() for chars and items ?
//
// FIXED BUGS:
// Vendors - Buy/Sell
// Weight display ?
// Skills not working- herding, hiding, etc.
// anim human death = crash, equip items on corpse.
// move after res = crash. unless full update.
// +set color for chars. need to remove char
// +res. gown is still ghost gown.
// +gochar command
// +quake = crash
// +drop on top = ground.
// +set color w/ no args.
// +cutting piles.
// +set karma -10000
// +filter unprintables | or ~
//
// done:
// opt weight changes and stat update s
// dupe contents and creatures.
// Houses and tents. Deeds
// Location/Quandrant structures. -> Find by location faster. sparse array (me)
// Default Creature templates. GrayChar.scp (Nix)
// Enum of sounds. Lots of sounds we are not using for effects. (Alluvian)
// Monsters casting spells.
// Better bounding and mapping of the world. (NPC's walk through walls now)
// Import WSC world files.
// Healers
// Food and hunger. Eating and drinking.
// Win32 try and catch. exception handling.
// /MUSIC command
// /GOCLI command
// Name rune mode. Cancel naming ?
// Login with ip as encryption code fails ? (compiler related?)
// Fix the stupid crashes. (location related crashes) = bad colors displayed !
// Store inventory by container.
// Translate UID to pointers in game ? very quick lookup
// spell sounds / creature sounds
// Get dungeon and world teleporters.
// follow/attack by npcs and other pet commands
// Dynamic allocation of stuff.


#include "graysvr.h"	// predef header.

const char * Dirs[DIR_QTY] =
{
	"N",
	"NE",
	"E",
	"SE",
	"S",
	"SW",
	"W",
	"NW",
};

const char * Runes[] = /// ??? fill this in.
{
	"An",
	"Bet",
	"Corp",
	"Des",
	"Ex",
	"Flam",
	"Grav",
	"Hur",
	"In",
	"Jux",
	"Kal",
	"Lor",
	"Mani",
	"Nox",
	"Ort",
	"Por",
	"Quas",
	"Rel",
	"Sanct",
	"Tym",	// never used. (Time?)
	"Uus",
	"Vas",
	"Wis",
	"Xen",
	"Ylem",
	"Zan",	// never used.
};

const char * Stat_Name[STAT_QTY] = 
{
	"STR", 
	"INT",
	"DEX", 
	"KARMA",
	"FAME",
	"KILLS",
};

// Usefull info about skills.

const CSkillDef Skills[SKILL_QTY+1] =
{
	{ "ALCHEMY",		"Alchemist",
		NULL,
		"You pour the failed mixture from the mortar.",
	},	
	{ "ANATOMY",		"Scholar",	
		"Select char to inspect.",
		"You can't think of anything about this creature",
	},
	{ "ANIMALLORE",		"Scholar",
		"What animal do you want information on?",
		"You can't think of anything off hand",
	},
	{ "ITEMID",			"Merchant",	
		"Select item to inspect.",
		"You can't think of anything off hand"
	},
	{ "ARMSLORE",		"Armsman",	
		"Choose item to evaluate.",
		"You are uncertain about this item"
	},
	{ "PARRYING",		"Shieldfighter",	},
	{ "BEGGING",		"Beggar",
		"Who do you want to grovel to?",
	},
	{ "BLACKSMITHING",	"Blacksmith",	
		NULL,
		"You have failed to make anything",
	},	
	{ "BOWCRAFT",		"Bowyer",		
		NULL,
		"You fail to create the item",
	},
	{ "PEACEMAKING",	"Bard",			
		NULL,
		"You attempt at peacemaking has failed",
	},
	{ "CAMPING",		"Camper",			
		NULL,
		"You fail to light the camp fire",
	},
	{ "CARPENTRY",		"Carpenter",
		NULL,
		"You fail to create the item",
	},
	{ "CARTOGRAPHY",	"Cartographer",
		NULL,
		"Your hand is unsteady and you map is a failure",	
	},
	{ "COOKING",		"Cook",
		"What would you like to cook?",
		"Your cooking fails",
	},
	{ "DETECTINGHIDDEN","Detective",	
		NULL,
		"You can't seem to find anything unussual",
	},
	{ "ENTICEMENT",		"Bard",
		"Who would you like to entice?",
		"You music fails to entice them",
	},
	{ "EVALUATINGINTEL","Scholar",
		"Select char to inspect.",
		"You cannot seem to judge the creature correctly",
	},
	{ "HEALING",		"Healer",
		"Who would you like to use the bandage on?",
		"You fail to apply the bandages correctly",
	},
	{ "FISHING",		"Fisherman",
		"Where do you want to fish?",
		"You have no luck fishing",
	},	
	{ "FORENSICS", 		"Scholar",	
		"What corpse would you like to examine?",
		"You can tell nothing about the corpse.",
	},
	{ "HERDING", 		"Ranger",		
		NULL,
		"You cannot seem to persuade the creature to move",
	},
	{ "HIDING", 		"Rogue",			
		NULL,
		"You cannot seem to hide here",
	},
	{ "PROVOCATION", 	"Bard",	
		"Who would you like to provoke?",
		"You fail to provoke enough anger to cause a fight",
	},
	{ "INSCRIPTION", 	"Scribe",
		NULL,
		"Your inscription fails. The scroll is ruined",
	},
	{ "LOCKPICKING", 	"Locksmith",
		"What would you like to use the pick on?",
	},
	{ "MAGERY",			"Mage",		
		NULL,
		"The spell fizzles",
	},
	{ "MAGICRESISTANCE","Resistor",			},
	{ "TACTICS", 		"Warrior",			},
	{ "SNOOPING", 		"Rogue",			
		NULL,
		"You fail to peek into the container",
	},
	{ "MUSICIANSHIP", 	"Bard",			
		NULL,
		"You play poorly",
	},
	{ "POISONING", 		"Assassin",
		NULL,
		"You fail to apply a does of poison",
	},
	{ "ARCHERY", 		"Archer",			},
	{ "SPIRITSPEAK",	"Medium",
		NULL,
		"You fail your attempt to contact the neither world",
	},
	{ "STEALING",		"Thief",	
		"What do you want to steal?",
		"You fail to steal anything",
	},
	{ "TAILORING", 		"Tailor",	
		NULL,
		"You lose some cloth",
	},
	{ "TAMING", 		"Animal Tamer",	
		"Choose the creature to tame.",
		"You fail to tame the creature",
	},
	{ "TASTEID", 		"Food Taster",
		"What would you like to taste?",
		"You cannot descern anything about this item",
	},
	{ "TINKERING", 		"Tinker",			
		NULL,
		"Your tinkering attempt fails",
	},
	{ "TRACKING", 		"Ranger",
		NULL,
		"You fail to see any tracks",
	},
	{ "VETERINARY", 	"Veterinarian",
		"What animal would you like to heal?",
		"You fail to apply the bandages correctly",
	},
	{ "SWORDSMANSHIP", 	"Swordsman",		},
	{ "MACEFIGHTING", 	"Macefighter",		},
	{ "FENCING",		"Fencer",			},
	{ "WRESTLING", 		"Wrestler",			},
	{ "LUMBERJACKING", 	"Lumberjack",
		"What tree do you want to chop?",
		"You hack at the tree for a while but fail to get usable wood",
	},
	{ "MINING",			"Miner",	
		"Where would you like to mine?",
		"You break up the rocks a bit but fail to find usable ore",
	},
	{ "ALLSKILLS",		NULL,				},
};

// Usefull info about spells.

const CSpellDef Spells[] =	// describe all my spells.
{
	{ 0 },

	// all spells make a noise either from you or at target.
	// sound, runes,regs,scroll, image,  directed, name
	// 1st
	{ "Clumsy",			0x1df, "UJ",  "BmNs",	ITEMID_SPELL_1,		  ITEMID_SCROLL_2,		SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Clumsy,
	{ "Create Food",	0x1e2, "IMY", "GaGiMr",	(ITEMID_TYPE) 0x2081, (ITEMID_TYPE) 0x1F2F, SPELLFLAG_TARG_XYZ, },					// SPELL_Create_Food,
	{ "Feeblemind",		0x1e4, "RW",  "GiNs",	(ITEMID_TYPE) 0x2082, (ITEMID_TYPE) 0x1F30, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Feeblemind,
	{ "Heal",			0x1f2, "IM",  "GaGsSs",	(ITEMID_TYPE) 0x2083, (ITEMID_TYPE) 0x1f31, SPELLFLAG_TARG_CHAR }, 					// SPELL_Heal,
	{ "Magic Arrow",	0x1e5, "IPY", "BpNs",	(ITEMID_TYPE) 0x2084, (ITEMID_TYPE) 0x1f32, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Magic_Arrow,
	{ "Night Sight",	0x1e3, "IL",  "SsSa",	(ITEMID_TYPE) 0x2085, (ITEMID_TYPE) 0x1f33, SPELLFLAG_TARG_CHAR },					// SPELL_Night_Sight,
	{ "Reactive Armor",	0x1f2, "FS",  "GaSsSa",	(ITEMID_TYPE) 0x2086, ITEMID_SCROLL_1,		SPELLFLAG_TARG_CHAR },					// SPELL_Reactive_Armor,
	{ "Weaken",			0x1e6, "DM",  "GaNs",	(ITEMID_TYPE) 0x2087, (ITEMID_TYPE) 0x1f34, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Weaken,

	// 2nd
	{ "Agility",		0x1e7, "EU",  "BmMr",	(ITEMID_TYPE) 0x2088, (ITEMID_TYPE) 0x1f35, SPELLFLAG_TARG_CHAR, },					// SPELL_Agility,
	{ "Cunning",		0x1eb, "UW",  "MrNs",	(ITEMID_TYPE) 0x2089, (ITEMID_TYPE) 0x1f36, SPELLFLAG_TARG_CHAR, },					// SPELL_Cunning,
	{ "Cure",			0x1e0, "AN",  "GaGi",	(ITEMID_TYPE) 0x208a, (ITEMID_TYPE) 0x1f37, SPELLFLAG_TARG_CHAR, },					// SPELL_Cure,
	{ "Harm",			0x1f1, "AM",  "NsSs",	(ITEMID_TYPE) 0x208b, (ITEMID_TYPE) 0x1f3b, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Harm,
	{ "Magic Trap",		0x1ef, "IJ",  "GaSsSa",	(ITEMID_TYPE) 0x208c, (ITEMID_TYPE) 0x1f39, SPELLFLAG_TARG_OBJ, },					// SPELL_Magic_Trap,
	{ "Magic Untrap",	0x1f0, "AJ",  "BmSa",	(ITEMID_TYPE) 0x208d, (ITEMID_TYPE) 0x1f3a, SPELLFLAG_TARG_OBJ, },					// SPELL_Magic_Untrap,
	{ "Protection",		0x1ed, "US",  "GaGiSa",	(ITEMID_TYPE) 0x208e, (ITEMID_TYPE) 0x1f3b, SPELLFLAG_TARG_CHAR },					// SPELL_Protection,
	{ "Strength",		0x1ee, "UM",  "MrNs",	(ITEMID_TYPE) 0x208f, (ITEMID_TYPE) 0x1f3c, SPELLFLAG_TARG_CHAR },					// SPELL_Strength,

	// 3rd
	{ "Bless",			0x1ea, "RS",  "0", (ITEMID_TYPE) 0x2090, (ITEMID_TYPE) 0x1f3d, SPELLFLAG_TARG_OBJ },		// SPELL_Bless,
	{ "Fireball",		0x15f, "VF",  "0", (ITEMID_TYPE) 0x2091, (ITEMID_TYPE) 0x1f3e, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Fireball,
	{ "Magic Lock",		0x1f4, "AP",  "0", (ITEMID_TYPE) 0x2092, (ITEMID_TYPE) 0x1f3f, SPELLFLAG_TARG_OBJ 	},		// SPELL_Magic_Lock,
	{ "Poison",			0x205, "IN",  "0", (ITEMID_TYPE) 0x2093, (ITEMID_TYPE) 0x1f40, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Poison,
	{ "Telekin",		0x1f5, "OPY", "0", (ITEMID_TYPE) 0x2094, (ITEMID_TYPE) 0x1f41, SPELLFLAG_TARG_OBJ },		// SPELL_Telekin,
	{ "Teleport",		0x1fe, "RP",  "0", (ITEMID_TYPE) 0x2095, (ITEMID_TYPE) 0x1f42, SPELLFLAG_TARG_XYZ },		// SPELL_Teleport,
	{ "Unlock",			0x1ff, "EP",  "0", (ITEMID_TYPE) 0x2096, (ITEMID_TYPE) 0x1f43, SPELLFLAG_TARG_OBJ },		// SPELL_Unlock,
	{ "Wall of Stone",	0x1f6, "ISY", "0", (ITEMID_TYPE) 0x2097, (ITEMID_TYPE) 0x1f44, SPELLFLAG_TARG_XYZ },		// SPELL_Wall_of_Stone,

	// 4th
	{ "Arch Cure",		0x1e8, "VAN", "0", (ITEMID_TYPE) 0x2098, (ITEMID_TYPE) 0x1f45, 0 },		// SPELL_Arch_Cure,
	{ "Arch Protection",0x1f7, "VUS", "0", (ITEMID_TYPE) 0x2099, (ITEMID_TYPE) 0x1f46, 0 },		// SPELL_Arch_Prot,
	{ "Curse",			0x1e1, "DS",  "0", (ITEMID_TYPE) 0x209a, (ITEMID_TYPE) 0x1f47, SPELLFLAG_TARG_OBJ | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Curse,
	{ "Fire Field",		0x225, "IFG", "0", (ITEMID_TYPE) 0x209b, (ITEMID_TYPE) 0x1f48, SPELLFLAG_TARG_XYZ | SPELLFLAG_HARM },		// SPELL_Fire_Field,
	{ "Greater Heal",	0x202, "IVM", "0", (ITEMID_TYPE) 0x209c, (ITEMID_TYPE) 0x1f49, SPELLFLAG_TARG_CHAR	},		// SPELL_Great_Heal,
	{ "Lightning",		0x029, "POG", "0", (ITEMID_TYPE) 0x209d, (ITEMID_TYPE) 0x1f4a, SPELLFLAG_TARG_CHAR | SPELLFLAG_HARM 	},		// SPELL_Lightning,
	{ "Mana Drain",		0x1f8, "OR",  "0", (ITEMID_TYPE) 0x209e, (ITEMID_TYPE) 0x1f4b, SPELLFLAG_TARG_CHAR | SPELLFLAG_DIR_ANIM | SPELLFLAG_HARM },		// SPELL_Mana_Drain,
	{ "Recall",			0x1fc, "KOP", "0", (ITEMID_TYPE) 0x209f, (ITEMID_TYPE) 0x1f4c, SPELLFLAG_TARG_OBJ },		// SPELL_Recall,

	// 5th	
	{ "Blade Spirit",	0x212, "IHJY","0", (ITEMID_TYPE) 0x20a0, (ITEMID_TYPE) 0x1f4d, SPELLFLAG_TARG_XYZ | SPELLFLAG_HARM	},		// SPELL_Blade_Spirit,
	{ "Dispel Field",	0x210, "AG",  "0", (ITEMID_TYPE) 0x20a1, (ITEMID_TYPE) 0x1f4e, SPELLFLAG_TARG_OBJ },		// SPELL_Dispel_Field,
	{ "Incognito",		0x1ec, "KIE", "0", (ITEMID_TYPE) 0x20a2, (ITEMID_TYPE) 0x1f4f, 0 	},		// SPELL_Incognito,
	{ "Magic Reflect",	0x1e8, "IJS", "0", (ITEMID_TYPE) 0x20a3, (ITEMID_TYPE) 0x1f50, SPELLFLAG_TARG_CHAR },		// SPELL_Magic_Reflect,
	{ "Mind_Blast",		0x213, "PCW", "0", (ITEMID_TYPE) 0x20a4, (ITEMID_TYPE) 0x1f51, SPELLFLAG_TARG_CHAR|SPELLFLAG_DIR_ANIM| SPELLFLAG_HARM },		// SPELL_Mind_Blast,
	{ "Paralyze",		0x204, "AEP", "0", (ITEMID_TYPE) 0x20a5, (ITEMID_TYPE) 0x1f52, SPELLFLAG_TARG_CHAR|SPELLFLAG_DIR_ANIM| SPELLFLAG_HARM },		// SPELL_Paralyze,
	{ "Poison Field",	0x226, "ING", "0", (ITEMID_TYPE) 0x20a6, (ITEMID_TYPE) 0x1f53, SPELLFLAG_TARG_XYZ | SPELLFLAG_HARM	},		// SPELL_Poison_Field,
	{ "Summon",			0x215, "KX",  "0", (ITEMID_TYPE) 0x20a7, (ITEMID_TYPE) 0x1f54, SPELLFLAG_TARG_XYZ },		// SPELL_Summon,

	// 6th	
	{ "Dispel",			0x201, "AO",  "0", (ITEMID_TYPE) 0x20a8, (ITEMID_TYPE) 0x1f55, SPELLFLAG_TARG_CHAR |SPELLFLAG_DIR_ANIM,  	},		// SPELL_Dispel,
	{ "Energy Bolt",	0x20a, "CP",  "0", (ITEMID_TYPE) 0x20a9, (ITEMID_TYPE) 0x1f56, SPELLFLAG_TARG_CHAR |SPELLFLAG_DIR_ANIM| SPELLFLAG_HARM },		// SPELL_Energy_Bolt,
	{ "Explosion",		0x207, "VOF", "0", (ITEMID_TYPE) 0x20aa, (ITEMID_TYPE) 0x1f57, SPELLFLAG_TARG_CHAR |SPELLFLAG_DIR_ANIM| SPELLFLAG_HARM },		// SPELL_Explosion,
	{ "Invisibility",	0x203, "ALX", "0", (ITEMID_TYPE) 0x20ab, (ITEMID_TYPE) 0x1f58, SPELLFLAG_TARG_CHAR 	},		// SPELL_Invis,
	{ "Mark",			0x1fa, "KPY", "0", (ITEMID_TYPE) 0x20ac, (ITEMID_TYPE) 0x1f59, SPELLFLAG_TARG_OBJ	},		// SPELL_Mark,
	{ "Mass_Curse",		0x1fb, "VDS", "0", (ITEMID_TYPE) 0x20ad, (ITEMID_TYPE) 0x1f5a, SPELLFLAG_HARM	},		// SPELL_Mass_Curse,
	{ "Paralyze Field",	0x211, "IEG", "0", (ITEMID_TYPE) 0x20ae, (ITEMID_TYPE) 0x1f5b, SPELLFLAG_HARM },		// SPELL_Paralyze_Field,
	{ "Reveal",			0x1fd, "WQ",  "0", (ITEMID_TYPE) 0x20af, (ITEMID_TYPE) 0x1f5c, 0, 	},		// SPELL_Reveal,

	// 7th	
	{ "Chain Lightning",0x029, "VOG", "0", (ITEMID_TYPE) 0x20b0, (ITEMID_TYPE) 0x1f5d, SPELLFLAG_TARG_XYZ|SPELLFLAG_HARM },		// SPELL_Chain_Lightning,
	{ "Energy Field",	0x20b, "ISG", "0", (ITEMID_TYPE) 0x20b1, (ITEMID_TYPE) 0x1f5e, SPELLFLAG_TARG_XYZ|SPELLFLAG_HARM	},		// SPELL_Energy_Field,
	{ "Flame Strike",	0x208, "KVF", "0", (ITEMID_TYPE) 0x20b2, (ITEMID_TYPE) 0x1f5f, SPELLFLAG_TARG_OBJ|SPELLFLAG_HARM },		// SPELL_Flame_Strike,
	{ "Gate Travel",	0x20e, "VRP", "0", (ITEMID_TYPE) 0x20b3, (ITEMID_TYPE) 0x1f60, SPELLFLAG_TARG_OBJ 	},		// SPELL_Gate_Travel,
	{ "Mana Vampire",	0x1f9, "OS",  "0", (ITEMID_TYPE) 0x20b4, (ITEMID_TYPE) 0x1f61, SPELLFLAG_TARG_CHAR|SPELLFLAG_DIR_ANIM| SPELLFLAG_HARM },		// SPELL_Mana_Vamp,
	{ "Mass Dispel",	0x209, "VAO", "0", (ITEMID_TYPE) 0x20b5, (ITEMID_TYPE) 0x1f62, 0  	},		// SPELL_Mass_Dispel,
	{ "Meteor Swarm",	0x160, "FKDY","0", (ITEMID_TYPE) 0x20b6, (ITEMID_TYPE) 0x1f63, SPELLFLAG_TARG_XYZ|SPELLFLAG_HARM	},		// SPELL_Meteor_Swarm,
	{ "Polymorph",		0x20f, "VYR", "0", (ITEMID_TYPE) 0x20b7, (ITEMID_TYPE) 0x1f64, 0 	},		// SPELL_Polymorph,

	// 8th	
	{ "Earthquake",		0x20d, "IVP", "0", (ITEMID_TYPE) 0x20b8, (ITEMID_TYPE) 0x1f65,	SPELLFLAG_HARM },		// SPELL_Earthquake,
	{ "Energy Vortex",	0x212, "VCP", "0", (ITEMID_TYPE) 0x20b9, (ITEMID_TYPE) 0x1f66,	SPELLFLAG_TARG_XYZ | SPELLFLAG_HARM },		// SPELL_Vortex,
	{ "Resurrection",	0x214, "AC",  "0", (ITEMID_TYPE) 0x20ba, (ITEMID_TYPE) 0x1f67,	SPELLFLAG_TARG_CHAR, 	},		// SPELL_Resurrection,
	{ "Air Elemental",	0x217, "KVXH","0", (ITEMID_TYPE) 0x20bb, (ITEMID_TYPE) 0x1f68,	SPELLFLAG_TARG_XYZ 	},		// SPELL_Air_Elem,
	{ "Daemon",			0x216, "KVXC","0", (ITEMID_TYPE) 0x20bc, (ITEMID_TYPE) 0x1f69,	SPELLFLAG_TARG_XYZ 	},		// SPELL_Daemon,
	{ "Earth Elemental",0x217, "KVXY","0", (ITEMID_TYPE) 0x20bd, (ITEMID_TYPE) 0x1f6a,	SPELLFLAG_TARG_XYZ 	},		// SPELL_Earth_Elem,
	{ "Fire Elemental",	0x217, "KVXF","0", (ITEMID_TYPE) 0x20be, (ITEMID_TYPE) 0x1f6b,	SPELLFLAG_TARG_XYZ 	},		// SPELL_Fire_Elem,
	{ "Water Elemental",0x217, "KVXAF","0",ITEMID_SPELL_64,		 ITEMID_SCROLL_64,		SPELLFLAG_TARG_XYZ	},		// SPELL_Water_Elem,

	// Necro
	{ "Summon Undead",	0x24a, "KNM",	"0", ITEMID_ALCH_SYM_1, ITEMID_SCROLL_A, SPELLFLAG_TARG_XYZ },
	{ "Animate Dead",   0x1e8, "IAMG",	"0", ITEMID_ALCH_SYM_2, ITEMID_SCROLL_B, SPELLFLAG_TARG_OBJ },
	{ "Bone Armor",     0x241, "ICSY",	"0", ITEMID_ALCH_SYM_3, ITEMID_SCROLL_C, SPELLFLAG_TARG_OBJ },

};

const char * Gray_szDesc = 
#ifdef _WIN32
	GRAY_TITLE " Version " GRAY_VERSION " Alpha [WIN32] by " GRAY_NAME " <" GRAY_EMAIL ">";
#else
	GRAY_TITLE " Version " GRAY_VERSION " Alpha [LINUX] by " GRAY_NAME " <" GRAY_EMAIL ">";
#endif

// chars
int CChar::sm_iCount = 0;

// UID table.
int CObjBase::sm_iCount = 0;
CObjBase * UIDs[ MAX_OBJECTS ] = { NULL };	// UID Translation table.

// game servers stuff.
CWorld  World;	// the world. (we save this stuff)
CServer Serv;	// current state stuff not saved.
CLog	Log;
CObList	GMPage;	// List of GM pages.

// items.
CObList ItemBase;	// CItemBase

int CItem::sm_iCount = 0;

//////////////////////////////////////////////////////////////////
// util type stuff.

#ifdef COMMENT
int GetRandVal( int iqty )
{
	return( rand() % iqty );
	// return( ( rand() * (DWORD) iqty ) / ( RAND_MAX + 1 )) ;
}
#endif

DWORD ahextoi( const char * pszStr ) // Convert hex string to integer
{
	// Unfortunatly the library func cant handle the number FFFFFFFF
	// char * sstop; return( strtol( s, &sstop, 16 ));

	DWORD val = 0;
	while ( 1 )
	{
		char ch = *pszStr;
		if ( ch >= '0' && ch <= '9' )
			ch -= '0';
		else
		{
			ch |= 0x20;	// toupper()
			if ( ch > 'f' || ch <'a' ) break;
			ch -= 'a' - 10;
		}
		val *= 0x10;
		val += ch;
		pszStr ++;
	}
	return( val );
}

int GetDecimalVal( const char * & pArgs )
{
	int iVal = 0;
	bool fNeg = ( *pArgs == '-' );
	if ( fNeg ) pArgs ++;
	while ( *pArgs >= '0' && *pArgs <= '9' )
	{
		iVal *= 10;
		iVal += *pArgs - '0';
		pArgs++;
	}
	if ( *pArgs == '.' )
	{
		pArgs++;
		while ( *pArgs >= '0' && *pArgs <= '9' )
		{
			iVal *= 10;
			iVal += *pArgs - '0';
			pArgs++;
		}
	}
	if ( fNeg ) return( -iVal );
	return( iVal );
}
// FIXIT!
int GetRangeVal( const char *  pArgs )
{
	int iVal = GetDecimalVal( pArgs );
	while ( *pArgs == ' ' ) pArgs ++;
	if ( *pArgs != '-' )
		return( iVal );
	pArgs++;
	while ( *pArgs == ' ' ) pArgs ++;
	int iRange = GetDecimalVal( pArgs ) - iVal;
	if ( iRange <= 0 ) return( iVal );
	return( iVal + GetRandVal( iRange ));
}

char * GetTempStr( void )
{
	// Some scratch string space, random uses
	static int i=0;
	static char szTempStr[8][256];
	if ( ++i >= 8 ) i = 0;
	return( szTempStr[i] );
}

int FindID( WORD id, const WORD * pID, int iCount ) 
{
	for ( int i=0; i < iCount; i++ )
	{
		if ( pID[i] == id )
			return( i );
	}
	return( -1 );
}

int FindTable( const char * pFind, const char * const * ppTable, int iCount, int iElemSize )
{
	for ( int i=0; i<iCount; i++ )
	{
		if ( ! strcmpi( *ppTable, pFind ))
			return( i );
		ppTable = (const char * const *)((( const BYTE*) ppTable ) + iElemSize );
	}
	return( -1 );
}

bool FindStrWord( const char * pTextSearch, const char * pszKeyWord )
{
	// Find the pszKeyWord in the pTextSearch string.
	// Make sure we look for starts of words.

	int j=0;
	for ( int i=0; 1; i++ )
	{
		if ( pszKeyWord[j] == '\0' )
			return( true );
		if ( pTextSearch[i] == '\0' )
			return( false );
		if ( !j && i )
		{
			char ch = toupper( pTextSearch[i-1] );
			if ( ch >= 'A' && ch <= 'Z' )	// not start of word ?
				continue;
		}
		if ( pTextSearch[i] == toupper( pszKeyWord[j] ))
			j++;
		else
			j=0;
	}
}

bool Parse( char * pLine, char ** pLine2 )
{
	// similar to strtok()
	int i=0;
	for ( ; pLine[i] != '\0'; i++ )
	{
		if ( pLine[i] == '=' ||
			pLine[i] == ',' ||
			pLine[i] == ' ' )
			break;
	}
	if ( pLine[i] == '\0' ) 
	{
		*pLine2 = "";
		return false;
	}
	pLine[i] = '\0';
	*pLine2 = &pLine[i+1];
	return true;
}

int ParseCmds( char * pCmdLine, char ** ppCmd, int iMax )
{
	int iQty = 1;
	ppCmd[0] = pCmdLine;
	while ( Parse( ppCmd[iQty-1], &(ppCmd[iQty])))
	{
		if ( ++iQty >= iMax ) break;
	}
	return( iQty );
}

///////////////////////////////////////////////////////////////
// -CObjUID

CObjBase * CObjUID :: ObjFind() const 
{
	DWORD dwVal = ( m_Val & UID_MASK );
	if ( dwVal > COUNTOF( UIDs )) return( NULL );
	return( UIDs[ dwVal ] );
}

CItem * CObjUID :: ItemFind() const // Does item still exist or has it been deleted
{
	return( dynamic_cast <CItem *>( ObjFind()) );
}

CChar * CObjUID :: CharFind() const // Does character still exist
{
	return( dynamic_cast <CChar *>( ObjFind()) );
}

///////////////////////////////////////////////////////////////
// -CFile

void CFile :: Close()
{
	if ( m_pFile == NULL ) return;
	fclose( m_pFile );
	m_pFile = NULL;
}

bool CFile :: Open( const char *szFilename, bool fWrite, bool fBinary )
{
	// Open a file.
	Close();

	const char * pMode;
	if ( fBinary )
		pMode = ( fWrite ) ? "wb" : "rb";
	else
		pMode = ( fWrite ) ? "w" : "r";

	m_pFile = fopen( szFilename, pMode );
	if ( m_pFile == NULL )
	{
		DEBUG_ERR(( "ERROR: %s not found...\n", szFilename ));
		return( false );
	}

	return( true );
}

size_t CFile :: VPrintf( const char * pFormat, va_list args )
{
	if ( ! IsOpen()) return( 0 );
	char * pTemp = GetTempStr();
    size_t len = vsprintf( pTemp, pFormat, args );
	return( fwrite( pTemp, len, 1, m_pFile ));
}

///////////////////////////////////////////////////////////////
// -CScript 

int CScript :: ReadLine() // Read a line from the opened script file
{
	if ( m_fUnRead )
	{
		m_fUnRead = false;
		return( 1 );
	}
	while ( 1 )
	{
		if ( CFile::ReadLine( m_Line, sizeof( m_Line )) == NULL ) 
		{
			m_Line[0] = '\0';
			return( EOF );
		}

		// Remove CR and LF from the end of the line.
		int len = strlen( m_Line );
		while ( len )
		{
			len --;
			if ( m_Line[len] != '\n' && m_Line[len] != '\r' )
			{
				len ++;
				break;
			}
		}
		if ( ! len ) continue;
		m_Line[len] = '\0';
		if ( m_Line[0] == '/' && m_Line[1] == '/' ) continue;
		return( 1 );
	}
}

bool CScript :: FindSec( const char *pName ) // Find a section in the current script
{
	if ( strlen( pName ) > MAX_NAME_SIZE )
	{
		DEBUG_ERR(( "ERROR: Bad script section name\n" ));
		return( false );
	}

	Seek();

	CString sSec;
	sSec.Format( "[%s]", pName );
	do
	{
		if ( ReadLine() == EOF )
		{
			DEBUG_ERR(( "WARNING: Did not find script section '%s'\n", pName ));
			return( false );
		}
	}
	while ( strnicmp( sSec, m_Line, sSec.GetLength() ));
	return( true );
}

bool CScript :: ReadParse() // Read line from script
{
	if ( ! Read1()) 
	{
		m_pArg = "";
		return( false );	// end of section.
	}
	Parse( m_Line, &m_pArg );
	return( true );
}

///////////////////////////////////////////////////////////////
// -CLog

void CLog :: Event( const char * pFormat, ... )
{
	va_list args;
	va_start( args, pFormat );

	// Print to screen.
	Serv.VPrintf( pFormat, args );

	// Print to log file.
	VPrintf( pFormat, args );
}

void CLog :: Error( const char * pFormat, ... )
{
	va_list args;
	va_start( args, pFormat );

	// Print to screen.
	Serv.VPrintf( pFormat, args );

	// Print to file.
	VPrintf( pFormat, args );
}

void CLog :: Dump( const BYTE * pData, int len )
{
	// Just dump a bunch of bytes. 16 bytes per line.
	while ( len )
	{
		char szTmp[16*3+10];
		int j=0;
		for ( ; j < 16; j++ )
		{
			if ( ! len ) break;
			sprintf( szTmp+(j*3), "%02x ", * pData );
			len --;
			pData ++;
		}
		strcpy( szTmp+(j*3), "\n" );
		Serv.Printf( szTmp );
		Printf( szTmp );	// Print to log file.
	}
}

///////////////////////////////////////////////////////////////
// -CPoint

int CPoint :: GetDist( CPoint p ) const // Distance between points
{
	int dx = abs(m_x-p.m_x);
	int dy = abs(m_y-p.m_y);
	return( max( dx, dy ));
}

DIR_TYPE CPoint :: GetDir( CPoint p ) const // Direction to point p
{
	int dx = (m_x-p.m_x);
	int dy = (m_y-p.m_y);

	if ( dx > 0 )	// westish
	{
		if ( dy > 0 ) return( DIR_NW );
		if ( dy < 0 ) return( DIR_SW );
		return( DIR_W );
	}
	else if ( dx < 0 ) // eastish
	{
		if ( dy > 0 ) return( DIR_NE );
		if ( dy < 0 ) return( DIR_SE );
		return( DIR_E );
	}
	else
	{
		if ( dy < 0 ) return( DIR_S );
		if ( dy > 0 ) return( DIR_N );
		return( DIR_QTY );	// here ?
	}
}

void CPoint :: Move( DIR_TYPE dir, int amount )
{
	// Move a point in a direction.
	switch ( dir )
	{
	case DIR_N:	 m_y -= amount; break;
	case DIR_NE: m_x += amount; m_y -= amount; break;
	case DIR_E:	 m_x += amount; break;
	case DIR_SE: m_x += amount; m_y += amount; break;
	case DIR_S:	 m_y += amount; break;
	case DIR_SW: m_x -= amount; m_y += amount; break;
	case DIR_W:	 m_x -= amount; break;
	case DIR_NW: m_x -= amount; m_y -= amount; break;
	}
}

void CPoint :: Read( char * pStr1 )
{
	char * pStr2;
	if ( ! Parse( pStr1, &pStr2 )) return;
	m_x = atoi( pStr1 );
	if ( ! Parse( pStr2, &pStr1 )) return;
	m_y = atoi( pStr2 );
	m_z = atoi( pStr1 );
}

const char * CPoint :: Write( void ) const
{
	char * pTemp = GetTempStr();
	sprintf( pTemp, "%d,%d,%d", m_x, m_y, m_z );
	return( pTemp );
}

CQuadrant * CPoint :: GetQuadrant() const
{
	// Get the world quadrant we are in.
	return( & ( World.m_Quads[ (( m_y / QUAD_SIZE_Y ) * QUAD_COLS ) + ( m_x / QUAD_SIZE_X )	] ));
}

/////////////////////////////////////////////////////////////////
// -CObjBase stuff
// Either a player, npc or item.

CObjBase :: CObjBase( WORD id, bool fItem )
{
	sm_iCount ++;
#ifdef _DEBUG
	m_dwSignature = COBJBASE_SIGNATURE;
#endif
	SetPrivateID( m_id );
	m_color=0;
	m_timeout=0;

	// Find a free UID slot for this.
	m_UID = UID_UNUSED;
	if ( ! World.IsLoading())
	{
		// Don't do this yet if we are loading.
		SetPrivateUID( 0, fItem );
	}
}

CObjBase :: ~CObjBase()
{
	sm_iCount --;
	Remove();

	// free up the UID slot.
	SetPrivateUID( UID_UNUSED, false );
#ifdef _DEBUG
	ASSERT( m_dwSignature == COBJBASE_SIGNATURE );
	m_dwSignature = 0;
#endif
}

void CObjBase :: SetPrivateUID( DWORD dwVal, bool fItem )
{
	// Move the serial number,
	// This is possibly dangerous if conflict arrises.

	if ( m_UID != UID_UNUSED )
	{
		// remove the old UID.
		UIDs[ ((DWORD)m_UID) & UID_MASK ] = NULL;
		m_UID.ClearUID();	// Invalid this just in case.
	}

	if ( dwVal != UID_UNUSED )
	{
		dwVal &= UID_MASK;
		if ( dwVal == 0 || dwVal >= MAX_OBJECTS )	// Just find the first free one.
		{
			for ( dwVal=1; dwVal<MAX_OBJECTS; dwVal++ )
			{
				if ( UIDs[dwVal] == NULL ) break;
			}
			if ( dwVal >= MAX_OBJECTS )
			{
				// We have run out of free UID's !!!
				// Replace a random item. (NEVER replace a char)
				dwVal = GetRandVal( MAX_OBJECTS-1 ) + 1;
				for ( int iCount = MAX_OBJECTS; iCount--; dwVal ++ )
				{
					if ( ! dwVal ) continue;	// don't take this one.
					if ( UIDs[ dwVal ]->IsItem()) break;
				}
			}
		}
		if ( UIDs[ dwVal ] != this )
		{
			if ( UIDs[ dwVal ] != NULL )
			{
				DEBUG_ERR(( "UID conflict delete %ld, '%s'\n", dwVal, UIDs[ dwVal ]->GetName() ));
				delete UIDs[ dwVal ];
				// ASSERT( 0 );
			}
			UIDs[ dwVal ] = this;
		}
		if ( fItem ) dwVal |= UID_ITEM;
		m_UID.SetUID( dwVal );
	}
}

void CObjBase :: SetTimeout( int iDelay )
{
	if ( iDelay < 0 )
	{
		m_timeout = 0;
	}
	else 
	{
		m_timeout = World.m_Clock_Time + iDelay;
	}
}

void CObjBase :: Sound( WORD id ) const // Play sound effect for player
{
	// play for everyone near by.

	if ( id >= 0x300 ) return;	// Max sound id ???

	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->CanSee( this )) continue;
		pClient->addSound( id, this );
	}
}

void CObjBase :: Effect( BYTE motion, ITEMID_TYPE id, const CObjBase * pSource, BYTE speed, BYTE loop, BYTE explode ) const
{
	// show for everyone near by.

	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->CanSee( this )) continue;
		pClient->addEffect( motion, id, this, pSource, speed, loop, explode );
	}
}

void CObjBase :: MoveTo( CPoint p )
{
	// Move this item to it's point in the world. (ground or top level)
	// Low level. DOES NOT UPDATE DISPLAYS
	CQuadrant * pQuad = p.GetQuadrant();
	if ( IsItem())
		pQuad->m_Items.InsertAfter( this );
	else
		pQuad->m_Chars.InsertAfter( this );
	m_p = p;
}

void CObjBase :: UpdateCanSee( CCommand * pCmd, int iLen, CClient * pClientExclude ) const
{
	// Send this update message to everyone who can see this.
	// NOTE: Need not be a top level object. CanSee() will calc that.
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude ) continue;
		if ( ! pClient->CanSee( this )) continue;
		pClient->xSendPkt( pCmd, iLen );
	}
}

void CObjBase :: Write( CScript & s ) const
{
	s.Printf( "SERIAL=%x\n", GetUID() );
	if ( ! m_sName.IsEmpty())
		s.Printf( "NAME=%s\n", (const char*) m_sName );
	if ( m_color )
		s.Printf( "COLOR=%x\n", m_color );
	if ( m_timeout )
		s.Printf( "TIMER=%lx\n", m_timeout );
}

bool CObjBase :: LoadVal( const char * pKey, char * pVal )
{   
	static const char * table[] =
	{
		"NAME",
		"COLOR", 
		"TIMER", 
	};
	
	// load the basic stuff.
	switch ( FindTable( pKey, table, COUNTOF( table )))
	{
	case 0:	SetName( pVal ); return true;
	case 1: m_color = ahextoi(pVal); return true;
	case 2: m_timeout = atoi(pVal); return true;
	}
	return false;
}

void CObjBase :: Remove( CClient * pClientExclude )
{
	// Remove this item from all clients.

	//CObjBase * pObj = GetTopLevelObj(); 
	for ( CClient * pClient = Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClientExclude == pClient ) continue;
		//if ( pClient->m_pChar->GetDist( pObj ) > UO_MAP_VIEW_BIG_SIZE ) continue;
		pClient->addObjectRemove( this );
	}
}

/////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
{
	assert( MAX_BUFFER >= sizeof( CCommand ));
	assert( MAX_BUFFER >= sizeof( CEvent ));
	assert( sizeof( int ) == sizeof( DWORD ));	// make this assumption often.
	assert( ( UO_BLOCKS_X % QUAD_SIZE_X ) == 0 );
	assert( ( UO_BLOCKS_Y % QUAD_SIZE_Y ) == 0 );

#ifdef _WIN32
	SetConsoleTitle( GRAY_TITLE " V" GRAY_VERSION );
#endif
	Log.Event( GRAY_TITLE " V" GRAY_VERSION 
#ifdef _WIN32
		" for Win32\n"
#else
		" for Linux\n"
#endif
		"Client Version: " GRAY_CLIENT_STR "\n"
		"Compiled at " __DATE__ " (" __TIME__ " " GRAY_TIMEZONE ")\n"
		"by " GRAY_NAME " <" GRAY_EMAIL ">\n"
		"\n" );

	if ( ! Serv.IniRead())
	{
		Log.Error( "ERROR: The .INI file is corrupt or missing\n" );
		return( 1 );
	}
	if ( ! World.Load())
	{
		return( 1 );
	}

	if ( argc > 1 )
	{
		// Do special debug type stuff.
		if ( ! World.CommandLine( argc, argv ))
			return( 1 );
	}


	if ( ! Serv.SocketsInit())
	{
		return( 1 );
	}

	Log.Event( "Startup complete. %d items %d chars\n", CItem::sm_iCount, CChar::sm_iCount );
	while ( Serv.m_fKeepRun )
	{
#ifdef _CPPUNWIND
		if ( Serv.m_fSecure )	// enable the try code.
		{
			try
			{
				Serv.OnTick();
			}
			catch (...)	// catch all
			{
				static time_t preverrortick = 0;
				if ( preverrortick != World.m_Clock_Time )
				{
					DEBUG_ERR(( "ERROR: Unhandled Exception time=%d!!\n", World.m_Clock_Time ));
					preverrortick = World.m_Clock_Time;
				}
				// Try garbage collection here ! Try to recover ?
			}
		}
		else
#endif
		Serv.OnTick();
	}

	Serv.SocketsClose();
	World.Close();

	if ( Serv.m_error )
		Log.Error( "ERROR: Server terminated by error!\n" );
	else 
		Log.Event( "Server shutdown complete!\n");

	return( Serv.m_error );
}

