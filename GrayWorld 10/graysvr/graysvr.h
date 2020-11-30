//
// GraySvr.H
// Precompiled header
// server for uo

#include "graycom.h"

#define GRAY_TITLE		"GrayWorld Server"
#define GRAY_VERSION	"0.10" 

#define MAX_BUFFER		2560	// Buffer Size (For socket operations)
#define MAX_OBJECTS		(16*1024)	// kind of arbitrary

#if defined( _CPPUNWIND ) && defined( _DEBUG )
#define ASSERT(exp) (void)( (exp) || (Serv.Assert(#exp, __FILE__, __LINE__), 0) )
#else
#define ASSERT assert
#endif

class CObList;
class CObjBase;
class CChar;
class CItem;
class CClient;
class CContainer;
class CContainerItem;
class CQuadrant;
class CRegion;

class CString
{
private:
    char * m_pchData;	// This cannot be just zeroed !
	static const char sm_Nil;		// Use this instead of null.
    int m_iLength;

private:
    int SetLength( int iLen );
	const char * GetPtr( void ) const
	{
		return( m_pchData );
	}
    void Init( void )
    {
    	m_pchData = (char*)&sm_Nil;
    	m_iLength = 0;
	}    
public:
    const CString& Copy( const char * pStr );
   
	CString()
	{
		Init();
	}
    CString( const char * pStr )
    {
	    Init();
    	Copy( pStr );
    }
    operator const char*() const       // as a C string
    {
		return( GetPtr());
    }
	const CString operator+=(const char* psz);	// like strcat
    
    int GetLength() const
    {
        return( m_iLength );
    }
    bool IsEmpty() const
    {
    	return( ! m_iLength );
   	}
    void Empty();
    ~CString( void )
    {
        Empty();
    }
    int CopyTo( char * pStr ) const
    {
    	strcpy( pStr, GetPtr() );
    	return( GetLength());
	}
    const CString& operator=( const CString &s )
    {
		return Copy( s.GetPtr() );
    }
    const CString& operator=( const char * pStr )
    {
        return Copy( pStr );
    }
	int Format( const char * pStr, ... );
	int Compare( const char * pStr ) const
	{
		return( strcmp( GetPtr(), pStr ));
	}
	int CompareNoCase( const char * pStr ) const
	{
		return( strcmpi( GetPtr(), pStr ));
	}
};

class CObListRec	// generic list record.
{
	friend class CObList ;
protected:
	CObList  * 	m_pParent;		// link me back to my parent object.
	CObListRec * 	m_pNext;
	CObListRec * 	m_pPrev;
public:
	CObList  * 	GetParent() const { return( m_pParent ); }
	CObListRec * 	GetNext() const { return( m_pNext ); }
	CObListRec * 	GetPrev() const { return( m_pPrev ); }
public:
	CObListRec()
	{
		m_pParent = NULL;	// not linked yet.
		m_pNext = NULL;
		m_pPrev = NULL;
	}
	void RemoveSelf();	// remove myself from my parent list.
	virtual ~CObListRec()
	{
		RemoveSelf();
	}
};

class CObList	// MFC like generic list.
{
private:
	CObListRec * m_pHead;
	CObListRec * m_pTail;	// Do we really care about tail ? (as it applies to lists anyhow)
	int m_iCount;
public:
	virtual CContainer * GetThisContainer( void )
	{
		return( NULL );
	}
	virtual void RemoveAt( CObListRec * pRec );
	virtual void InsertAfter( CObListRec * pNewRec, CObListRec * pPrev = NULL );
	void DeleteAt( CObListRec * pRec );
	void DeleteAll();	// Empty()
	CObListRec * GetHead( void ) const { return( m_pHead ); }
	CObListRec * GetTail( void ) const { return( m_pTail ); }
	int GetCount() const { return( 	m_iCount ); }
	CObList()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_iCount = 0;
	}
	virtual ~CObList()
	{
		DeleteAll();
	}
};

// Script reading stuff.

class CFile
{
private:
	FILE * m_pFile;		// the current open script type file.
public:
	CFile()
	{
		m_pFile = NULL;
	}
	bool IsOpen() const
	{
		return( m_pFile != NULL );
	}
	bool Open( const char * pFileName, bool fWrite = false, bool fBinary = false );
	void Close();
	~CFile()
	{
		Close();
	}
	int Seek( long offset = 0, int origin = SEEK_SET ) const
	{
		return( fseek( m_pFile, offset, origin ));
	}
	void Flush() const
	{
		fflush(m_pFile);
	}
	long GetPos() const
	{
		return( ftell(m_pFile));
	}
	size_t Read( void * pBuffer, size_t size ) const
	{
		return( fread( pBuffer, size, 1, m_pFile ));
	}
	char * ReadLine( char * pBuffer, size_t size ) const
	{
		// Read a line of text. NULL = EOF
		return( fgets( pBuffer, size, m_pFile ));
	}
	size_t VPrintf( const char * pFormat, va_list pArgs );
	size_t Printf( const char * pFormat, ... )
	{
		va_list args;
		va_start( args, pFormat );
		return( VPrintf( pFormat, args ));
	}
};

class CScript : public CFile
{
	// ??? add some caching to this for [sections]. (Erwin)
private:
	bool m_fUnRead;
public:
	char m_Line[256];		// current line read
	char * m_pArg;			// for parsing
public:
	CScript()
	{
		m_pArg = NULL;
		m_fUnRead = false;
	}
	int ReadLine();
	void UnReadLine()
	{
		m_fUnRead = true;
	}
	bool FindSec( const char * pName ); // Find a section in the current script
	bool Read1()
	{
		return( ReadLine() > 0 && m_Line[0] != '[' && m_Line[0] != '}' );
	}
	bool ReadParse();
};

struct CLog : public CFile
{
	bool Open( void )
	{
		return( CFile::Open( GRAY_FILE "Log.LOG", true ));
	}
	void Event( const char * pFormat, ... );
	void Error( const char * pFormat, ... );
	void Dump( const BYTE * pData, int len );

#ifdef _DEBUG
#define DEBUG_MSG(_x_)	Log.Event _x_	
#define DEBUG_ERR(_x_)	Log.Error _x_
#else
#define DEBUG_MSG(_x_)				// this allows all the variable args to be removed
#define DEBUG_ERR(_x_)	Log.Error _x_
#endif
};

//////////////////

typedef WORD COLOR_TYPE;	// Human color is | 0x8000, 0x7cf8 = clear
typedef WORD SOUND_TYPE;	// Sound ID

enum ITEMID_TYPE	// InsideUO is great for this stuff.
{
	ITEMID_NOTHING		= 0x0000,	// Used for lightning.

	ITEMID_DOOR_SECRET_1			= 0x00E8,
	ITEMID_DOOR_SECRET_2			= 0x0314,
	ITEMID_DOOR_SECRET_3			= 0x0324,
	ITEMID_DOOR_SECRET_4			= 0x0334,
	ITEMID_DOOR_SECRET_5			= 0x0344,
	ITEMID_DOOR_SECRET_6			= 0x0354,

	ITEMID_DOOR_METAL_S			= 0x0675,	// 1
	ITEMID_DOOR_METAL_S_2		= 0x0677,
	ITEMID_DOOR_METAL_S_3		= 0x067D,
	ITEMID_DOOR_BARRED			= 0x0685,	// 2
	ITEMID_DOOR_RATTAN			= 0x0695,	// 3
	ITEMID_DOOR_WOODEN_1		= 0x06A5,	// 4
	ITEMID_DOOR_WOODEN_1_2		= 0x06A7, 
	ITEMID_DOOR_WOODEN_2		= 0x06B5,	// 5
	ITEMID_DOOR_METAL_L			= 0x06C5,	// 6
	ITEMID_DOOR_WOODEN_3		= 0x06D5,	// 7
	ITEMID_DOOR_WOODEN_4		= 0x06E5,	// 8
	ITEMID_DOOR_HI				= 0x06f4,

	ITEMID_DOOR_WOODENGATE_1	= 0x0839,
	ITEMID_DOOR_IRONGATE		= 0x084C,
	ITEMID_DOOR_WOODENGATE_2	= 0x0866,

	ITEMID_FOOD_BACON   = 0x0976,
	ITEMID_RBASKET		= 0x0990,	//0x0E78,
	ITEMID_CHEST_METAL	= 0x09ab,
	ITEMID_BACKPACK2	= 0x09b2, // another pack.
	ITEMID_FOOD_SAUSAGE = 0x09c0,
	ITEMID_FOOD_HAM		= 0x09C9,

	ITEMID_FISH_1		= 0x09CC,
	ITEMID_FISH_2		= 0x09CD,
	ITEMID_FISH_3		= 0x09CE,
	ITEMID_FISH_4		= 0x09CF,

	ITEMID_FOOD_CAKE    = 0x09e9,
	ITEMID_RAW_MEAT		= 0x09f1,

	ITEMID_SIGN_BRASS   = 0x0bd2,
	
	ITEMID_FISH_POLE	= 0x0dbf,
	ITEMID_SCISSORS1	= 0x0dfc,
	ITEMID_SCISSORS2	= 0x0dfd,

	ITEMID_KINDLING		= 0x0de1,
	ITEMID_KINDLING2	= 0x0de2,
	ITEMID_CAMPFIRE		= 0x0de3,
	ITEMID_EMBERS		= 0x0de9,

	ITEMID_WAND			= 0x0df2,
	ITEMID_WOOL			= 0x0df8,
	ITEMID_FEATHERS		= 0x0dfa,
	ITEMID_FEATHERS2	= 0x0dfb,

	ITEMID_GAME_BACKGAM = 0x0e1c,

	ITEMID_BANDAGES		= 0x0e21,	// clean
	ITEMID_BOTTLE_DYE	= 0x0e25,
	ITEMID_SCROLL_BLANK	= 0x0e34,
	ITEMID_SCROLL_B1	= 0x0e35,
	ITEMID_SCROLL_B6	= 0x0e3a,

	ITEMID_SPELLBOOK2	= 0x0E3b,
	ITEMID_CRATE1		= 0x0e3c,	// n/s	
	ITEMID_CRATE2		= 0x0e3d,	// e/w
	ITEMID_CRATE3		= 0x0e3e,
	ITEMID_CRATE4		= 0x0e3f,

	ITEMID_CHEST1		= 0x0e40,
	ITEMID_CHEST2		= 0x0e41,
	ITEMID_CHEST3		= 0x0e42,
	ITEMID_CHEST4		= 0x0e43,

	ITEMID_BACKPACK		= 0x0E75,	// containers.
	ITEMID_BAG			= 0x0E76,
	ITEMID_BARREL		= 0x0E77,
	ITEMID_BASIN		= 0x0e78,
	ITEMID_POUCH		= 0x0E79,
	ITEMID_SBASKET		= 0x0E7A,	// picknick basket
	ITEMID_CHEST_METAL2	= 0x0E7C,
	ITEMID_WOODBOX		= 0x0E7D,
	ITEMID_CRATE		= 0x0E7E,
	ITEMID_KEG			= 0x0E7F,
	ITEMID_BRASSBOX		= 0x0E80,

	ITEMID_HERD_CROOK1	= 0x0E81,	// Shepherds Crook
	ITEMID_HERD_CROOK2	= 0x0e82,
	ITEMID_Pickaxe1		= 0x0e85,
	ITEMID_Pickaxe2		= 0x0e86,
	ITEMID_Pitchfork	= 0x0e87,
	ITEMID_MORTAR		= 0x0e9b,

	ITEMID_GOLD			= 0x0EED,	// big pile
	ITEMID_SPELLBOOK	= 0x0EFA,

	ITEMID_POTION_BLACK	= 0x0f06,
	ITEMID_POTION_ORANGE,
	ITEMID_POTION_BLUE,
	ITEMID_POTION_WHITE,
	ITEMID_POTION_GREEN,
	ITEMID_POTION_RED,
	ITEMID_POTION_YELLOW,
	ITEMID_POTION_PURPLE= 0x0f0d,
	ITEMID_EMPTY_BOTTLE = 0x0f0e,

	ITEMID_Shovel		= 0x0f39,
	ITEMID_Arrow		= 0x0f3f,	// Need these to use a bow.
	ITEMID_DAGGER		= 0x0F51,

	ITEMID_BLUEMOONGATE	= 0x0f6c,
	ITEMID_REAG_1		= 0x0f78,	// batwing
	ITEMID_REAG_26		= 0x0f91,	// Worms heart

	ITEMID_CLOTH_BOLT	= 0x0f95,
	ITEMID_SEWINGKIT	= 0x0f9d,

	ITEMID_GAME_BOARD	= 0x0fa6,

	ITEMID_DYE			= 0x0FA9,
	ITEMID_DYEVAT		= 0x0FAB,
	ITEMID_ANVIL1		= 0x0FAF,
	ITEMID_FORGE		= 0x0FB1,
	ITEMID_ANVIL2		= 0x0FB0,

	ITEMID_KEY_COPPER	= 0x100e,
	ITEMID_SHAFTS		= 0x1024,
	ITEMID_SHAFTS2		= 0x1025,

	ITEMID_FOOD_BREAD   = 0x103b,
	ITEMID_DUMMY		= 0x1070,
	ITEMID_FX_DUMMY		= 0x1071,	// just an effect ???

	ITEMID_FX_SPARKLES	= 0x1153,	// magic sparkles.

	ITEMID_BLOOD1		= 0x122a,
	ITEMID_BLOOD6		= 0x122f,

	ITEMID_BOW			= 0x13b1,
	ITEMID_BOW2,

	ITEMID_SMITH_HAMMER = 0x13E4,

	ITEMID_BONE_ARMS    = 0x144e,
	ITEMID_BONE_ARMOR   = 0x144f,
	ITEMID_BONE_GLOVES  = 0x1450,
	ITEMID_BONE_HELM	= 0x1451,
	ITEMID_BONE_LEGS    = 0x1452,

	ITEMID_MAP			= 0x14EB,
	ITEMID_MAP_BLANK	= 0x14ec,

	ITEMID_DEED1		= 0x14ef,
	ITEMID_DEED2		= 0x14f0,
	ITEMID_SHIP_PLANS1	= 0x14f1,
	ITEMID_SHIP_PLANS2	= 0x14f2,

	ITEMID_LOCKPICK		= 0x14fb,
	ITEMID_SHIRT1		= 0x1517,
	ITEMID_PANTS1		= 0x152E,
	ITEMID_PANTS_FANCY	= 0x1539,

	ITEMID_HELM_BEAR	= 0x1545,
	ITEMID_HELM_DEER	= 0x1547,
	ITEMID_MASK_TREE	= 0x1549,
	ITEMID_MASK_VOODOO	= 0x154b,
	ITEMID_BLOOD_SPLAT	= 0x1645,

	ITEMID_SHOES		= 0x170F,

	ITEMID_HAT_WIZ		= 0x1718,
	ITEMID_HAT_JESTER	= 0x171c,
	ITEMID_CLOTH		= 0x175d,
	ITEMID_KEYRING		= 0x1769,

	ITEMID_ALCH_SYM_1	= 0x181d,
	ITEMID_ALCH_SYM_2	= 0x181e,
	ITEMID_ALCH_SYM_3	= 0x181f,
	ITEMID_ALCH_SYM_4	= 0x1820,
	ITEMID_ALCH_SYM_5	= 0x1821,
	ITEMID_ALCH_SYM_6	= 0x1822,
	ITEMID_ALCH_SYM_7	= 0x1823,
	ITEMID_ALCH_SYM_8	= 0x1824,
	ITEMID_ALCH_SYM_9	= 0x1825,
	ITEMID_ALCH_SYM_10	= 0x1826,
	ITEMID_ALCH_SYM_11	= 0x1827,
	ITEMID_ALCH_SYM_12	= 0x1828,

	ITEMID_ORE			= 0x19b9,

	ITEMID_FEATHERS3	= 0x1bd1,
	ITEMID_FEATHERS4	= 0x1bd2,
	ITEMID_FEATHERS5	= 0x1bd3,
	ITEMID_SHAFTS3		= 0x1bd4,
	ITEMID_SHAFTS4		= 0x1bd5,
	ITEMID_SHAFTS5		= 0x1bd6,

	ITEMID_LOGS			= 0x1bde,
	ITEMID_INGOT		= 0x1bf8,
	ITEMID_XBolt		= 0x1bfb,

	ITEMID_Bulletin		= 0x1e5e,
	ITEMID_WorldGem		= 0x1ea7,	// Typically a spawn item
	ITEMID_TINKER		= 0x1ebc,
	ITEMID_DOOR_MAGIC	= 0x1EED,
	ITEMID_SHIRT_FANCY	= 0x1EFD,

	ITEMID_ROBE			= 0x1F03,
	ITEMID_HELM_ORC		= 0x1f0b,
	ITEMID_RECALLRUNE	= 0x1f14,

	ITEMID_SCROLL_1		= 0x1f2d,	// Reactive armor.
	ITEMID_SCROLL_2		= 0x1f2e,
	ITEMID_SCROLL_64	= 0x1f6c,	// summon water

	ITEMID_SCROLL_A		= 0x1f6d,
	ITEMID_SCROLL_B		= 0x1f6e,
	ITEMID_SCROLL_C		= 0x1f6f,
	ITEMID_SCROLL_D		= 0x1f70,
	ITEMID_SCROLL_E		= 0x1f71,
	ITEMID_SCROLL_F		= 0x1f72,

	ITEMID_CORPSE		= 0x2006,	// This is all corpses.

	ITEMID_NPC_1		= 0x2008,	// NPC peasant object ?
	ITEMID_NPC_X		= 0x2036,	// NPC 

	ITEMID_HAIR_SHORT		= 0x203B,
	ITEMID_HAIR_LONG		= 0x203C,
	ITEMID_HAIR_PONY		= 0x203D,
	ITEMID_HAIR_MOHAWK		= 0x2044,
	ITEMID_HAIR_PAGE		= 0x2045,
	ITEMID_HAIR_CURL		= 0x2046,
	ITEMID_HAIR_7			= 0x2047,
	ITEMID_HAIR_RECEDE		= 0x2048,
	ITEMID_HAIR_2TAILS		= 0x2049,
	ITEMID_HAIR_TOPKNOT		= 0x204A,

	ITEMID_BEARD_LONG		= 0x203E,
	ITEMID_BEARD_SHORT		= 0x203F,
	ITEMID_BEARD_GOATEE		= 0x2040,
	ITEMID_BEARD_MOUSTACHE	= 0x2041,
	ITEMID_BEARD_SH_M		= 0x204B,
	ITEMID_BEARD_LG_M		= 0x204C,
	ITEMID_BEARD_GO_M		= 0x204D,

	ITEMID_DEATHSHROUD		= 0x204E,
	ITEMID_GM_ROBE			= 0x204f,

	ITEMID_SPELL_1			= 0x2080,
	ITEMID_SPELL_64			= 0x20bf,

	ITEMID_SPELL_CIRCLE1	= 0x20c0,
	ITEMID_SPELL_CIRCLE8	= 0x20c7,

	// Item equiv of creatures.
	ITEMID_TRACK_BEGIN	= 0x20DB,
	ITEMID_TRACK_ELEM_AIR = 0x20ed,
	ITEMID_TRACK_ELEM_EARTH = 0x20d7,
	ITEMID_TRACK_ELEM_FIRE  = 0x20f3,
	ITEMID_TRACK_ELEM_WATER = 0x210b,	
	ITEMID_TRACK_MAN	= 0x2106,
	ITEMID_TRACK_WOMAN	= 0x2107,
	ITEMID_TRACK_END	= 0x2121,

	ITEMID_BANK_BOX		= 0x2af8, // Vendor container or bank box

	// effects.
	ITEMID_FX_SPLASH		= 0x352d,

	ITEMID_FX_EXPLODE_3	= 0x36b0,
	ITEMID_FX_EXPLODE_2	= 0x36bd,
	ITEMID_FX_EXPLODE_1	= 0x36ca,
	ITEMID_FX_FIRE_BALL	= 0x36d4,
	ITEMID_FX_MAGIC_ARROW	= 0x36e4,

	ITEMID_FX_FLAMESTRIKE	= 0x3709,
	ITEMID_FX_TELE_VANISH	= 0x372A,	// teleport vanish

	ITEMID_FX_SPELL_FAIL	= 0x3735,
	ITEMID_FX_BLESS_EFFECT	= 0x373A,
	ITEMID_FX_CURSE_EFFECT	= 0x374A,

	ITEMID_FX_HEAL_EFFECT	= 0x376A,

	ITEMID_FX_BLUEMOONSTART= 0x3789,	// swirl
	ITEMID_FX_ENERGY_BOLT	= 0x379f,
	ITEMID_FX_BLADES		= 0x37eb,
	ITEMID_FX_POISON_F_EW	= 0x3914,
	ITEMID_FX_POISON_F_NS	= 0x3920,
	ITEMID_FX_ENERGY_F_EW	= 0x3947,
	ITEMID_FX_ENERGY_F_NS	= 0x3956,
	ITEMID_FX_PARA_F_EW	= 0x3967,
	ITEMID_FX_PARA_F_NS	= 0x3979,
	ITEMID_FX_FIRE_F_EW	= 0x398c,	// E/W
	ITEMID_FX_FIRE_F_NS	= 0x3996,	// N/S

	ITEMID_M_HORSE1		= 0x3E9F,	// horse item when ridden	
	ITEMID_M_HORSE2		= 0x3EA0,
	ITEMID_M_HORSE3		= 0x3EA1,
	ITEMID_M_HORSE4		= 0x3EA2,

	// Large composite objects here.
	ITEMID_BOAT_N		= 0x4000,
	ITEMID_BOAT_E,
	ITEMID_BOAT_S,
	ITEMID_BOAT_W,
	ITEMID_HOUSE		= 0x4064,
	ITEMID_STONEHOUSE	= 0x4066,
	ITEMID_TENT_BLUE	= 0x4070,
	ITEMID_TENT_GREEN	= 0x4072,
	ITEMID_3ROOM		= 0x4074,	// 3 room house
	ITEMID_2STORY_STUKO = 0x4076,
	ITEMID_2STORY_SAND	= 0x4078,
	ITEMID_TOWER		= 0x407a,
	ITEMID_KEEP		 	= 0x407C,	// keep
	ITEMID_CASTLE		= 0x407E,	// castle 7f also.

	ITEMID_LARGESHOP	= 0x408c,


	// GrayWorld special objects defined after this.
	ITEMID_SCRIPT		= 0x4100,	// script objects beyond here.

	ITEMID_QTY			= 0x4800,
};

enum CREID_TYPE		// enum the creature art work. (dont allow any others !)
{    
	CREID_OGRE			= 0x0001,
	CREID_ETTIN			= 0x0002,		
	CREID_ZOMBIE		= 0x0003,
	CREID_GARG			= 0x0004,
	CREID_EAGLE			= 0x0005,
	CREID_BIRD			= 0x0006,	
	CREID_ORC_LORD		= 0x0007,
	CREID_CORPSER		= 0x0008,
	CREID_DAEMON		= 0x0009,
	CREID_DAEMON_SWORD	= 0x000A,

	CREID_DRAGON_GREY	= 0x000c,
	CREID_AIR_ELEM		= 0x000d,
	CREID_EARTH_ELEM	= 0x000e,
	CREID_FIRE_ELEM		= 0x000f,
	CREID_WATER_ELEM	= 0x0010,
	CREID_ORC			= 0x0011,
	CREID_ETTIN_AXE		= 0x0012,

	CREID_GIANT_SNAKE	= 0x0015,
	CREID_GAZER			= 0x0016,

	CREID_LICH			= 0x0018,

	CREID_SPECTRE		= 0x001a,

	CREID_GIANT_SPIDER	= 0x001c,
	CREID_GORILLA		= 0x001d,
	CREID_HARPY			= 0x001e,
	CREID_HEADLESS		= 0x001f,

	CREID_LIZMAN		= 0x0021,
	CREID_LIZMAN_SPEAR	= 0x0023,
	CREID_LIZMAN_MACE	= 0x0024,

	CREID_MONGBAT		= 0x0027,

	CREID_ORC_CLUB		= 0x0029,
	CREID_RATMAN		= 0x002a,

	CREID_RATMAN_CLUB	= 0x002c,
	CREID_RATMAN_SWORD	= 0x002d,

	CREID_REAPER		= 0x002f,	// tree
	CREID_SCORP			= 0x0030,	// giant scorp.

	CREID_SKELETON      = 0x0032,
	CREID_SLIME			= 0x0033,
	CREID_Snake			= 0x0034,
	CREID_TROLL_SWORD	= 0x0035,
	CREID_TROLL			= 0x0036,
	CREID_TROLL_MACE	= 0x0037,
	CREID_SKEL_AXE		= 0x0038,
	CREID_SKEL_SW_SH	= 0x0039,	// sword and sheild
	CREID_WISP			= 0x003a,
	CREID_DRAGON_RED,
	CREID_DRAKE_GREY	= 0x003c,
	CREID_DRAKE_RED		= 0x003d,

	CREID_SEA_SERP		= 0x0096,
	CREID_Dolphin		= 0x0097,

	// Animals (Low detail critters)

	CREID_HORSE1		= 0x00C8,	// white
	CREID_Cat			= 0x00c9,
	CREID_Alligator		= 0x00CA,
	CREID_Pig			= 0x00CB,
	CREID_HORSE4		= 0x00CC,	// brown
	CREID_Rabbit		= 0x00CD,
	CREID_Sheep			= 0x00CF,

	CREID_Chicken		= 0x00D0,
	CREID_Goat			= 0x00d1,
	CREID_BrownBear		= 0x00D3,
	CREID_GrizzlyBear	= 0x00D4,
	CREID_PolarBear		= 0x00D5,
	CREID_Panther		= 0x00d6,
	CREID_GiantRat		= 0x00D7,
	CREID_Cow_BW		= 0x00d8,
	CREID_Dog			= 0x00D9,
	CREID_Llama			= 0x00dc,
	CREID_Walrus		= 0x00dd,
	CREID_Lamb			= 0x00df,
	CREID_Wolf			= 0x00e1,
	CREID_HORSE2		= 0x00E2,
	CREID_HORSE3		= 0x00E4,
	CREID_Cow2			= 0x00e7,
	CREID_Bull_Brown	= 0x00e8,	// light brown
	CREID_Bull2			= 0x00e9,	// dark brown
	CREID_Hart			= 0x00EA,	// Male deer.
	CREID_Deer			= 0x00ED,
	CREID_Rat			= 0x00ee,
	
	CREID_Boar			= 0x0122,	// large pig
	CREID_HORSE_PACK	= 0x0123,	// Pack horse with saddle bags
	CREID_LLAMA_PACK	= 0x0124,	// Pack llama with saddle bags

	// all below here are humanish or clothing.
	CREID_MAN			= 0x0190,	
	CREID_WOMAN			= 0x0191,
	CREID_GHOSTMAN		= 0x0192,	// Ghost robe is not automatic !
	CREID_GHOSTWOMAN	= 0x0193,

	CREID_CHILD_MB		= 0x01a4, // Male Kid (Blond Hair)
	CREID_CHILD_MD		= 0x01a5, // Male Kid (Dark Hair)
	CREID_CHILD_FB		= 0x01a6, // Female Kid (Blond Hair) (toddler)
	CREID_CHILD_FD		= 0x01a7, // Female Kid (Dark Hair)

	CREID_INVISIBLE		= 0x01a8,	// Use this as our invis man id.

	CREID_BLADES		= 0x023e,	// blade spirits (in human range? not sure why)

	CREID_QTY			= 0x0400,	// Max number.
};

enum ANIM_TYPE	// not all creatures animate the same for some reason.
{
	ANIM_WALK_UNARM		= 0x00,	// unarmed

	// human anim. (supported by all humans)
	ANIM_WALK_ARM		= 0x01,	// armed

	ANIM_RUN_UNARM		= 0x02,	
	ANIM_RUN_ARMED		= 0x03,	

	ANIM_STAND			= 0x04,

	ANIM_FIDGET1		= 0x05,		// twist
	ANIM_FIDGET2		= 0x06,		

	ANIM_STAND_WAR_1H	= 0x07,	// Stand for 1 hand attack.
	ANIM_STAND_WAR_2H	= 0x08,	// Stand for 2 hand attack.

	ANIM_ATTACK_1H_WIDE	= 0x09,	// sword type
	ANIM_ATTACK_1H_JAB	= 0x0a,	// fencing type.
	ANIM_ATTACK_1H_DOWN	= 0x0b,	// mace type

	ANIM_ATTACK_2H_DOWN = 0x0c,	// mace type
	ANIM_ATTACK_2H_JAB	= 0x0e,	// spear type weapon.
	ANIM_ATTACK_2H_WIDE = 0x0d,

	ANIM_WALK_WAR		= 0x0f,	// walk in attack position.

	ANIM_CAST_DIR		= 0x10,	
	ANIM_CAST_AREA		= 0x11,	

	ANIM_ATTACK_BOW		= 0x12,
	ANIM_ATTACK_XBOW	= 0x13,
	ANIM_GET_HIT		= 0x14,

	ANIM_DIE_BACK		= 0x15,
	ANIM_DIE_FORWARD	= 0x16,

	ANIM_TURN			= 0x1e,
	ANIM_ATTACK_UNARM 	= 0x1f,	// attack while walking ?

	ANIM_BOW			= 0x20,
	ANIM_SALUTE			= 0x21,
	ANIM_EAT			= 0x22,

	// don't use these directly these are just for translation.
	// Human on horseback
	ANIM_HORSE_RIDE_SLOW	= 0x17,
	ANIM_HORSE_RIDE_FAST	= 0x18,
	ANIM_HORSE_STAND		= 0x19,
	ANIM_HORSE_ATTACK		= 0x1a,
	ANIM_HORSE_ATTACK_BOW	= 0x1b,
	ANIM_HORSE_ATTACK_XBOW	= 0x1c,
	ANIM_HORSE_SLAP			= 0x1d,

	// monster anim	- (not all anims are supported for each creature)
	ANIM_MON_WALK 		= 0x00,
	ANIM_MON_STAND		= 0x01,
	ANIM_MON_DIE1		= 0x02,	// back
	ANIM_MON_DIE2		= 0x03, // fore or side.
	ANIM_MON_ATTACK1	= 0x04,
	ANIM_MON_ATTACK2	= 0x05,
	ANIM_MON_ATTACK3	= 0x06,
	ANIM_MON_STUMBLE 	= 0x0a,
	ANIM_MON_MISC_STOMP	= 0x0b,	// Misc, Stomp, slap ground, lich conjure.
	ANIM_MON_MISC_BREATH= 0x0c,	// Misc Cast, breath fire, elem creation.
	ANIM_MON_GETHIT1	= 0x0d,
	ANIM_MON_GETHIT2	= 0x0f,
	ANIM_MON_GETHIT3	= 0x10,

	ANIM_MON_FIDGET1	= 0x11,
	ANIM_MON_FIDGET2	= 0x12,
	ANIM_MON_MISC_ROLL 	= 0x00,	// Misc Roll over, 
	ANIM_MON_MISC1		= 0x00, // air/fire elem = flail arms.

	ANIM_MON_FLY		= 0x13,
	ANIM_MON_LAND		= 0x14,
	ANIM_MON_DIE_FLIGHT	= 0x15,

	// animals. (All animals have all anims)
	ANIM_ANI_WALK		= 0x00,
	ANIM_ANI_RUN		= 0x01,
	ANIM_ANI_STAND		= 0x02,
	ANIM_ANI_EAT		= 0x03,

	ANIM_ANI_ATTACK1	= 0x05,
	ANIM_ANI_ATTACK2	= 0x06,
	ANIM_ANI_ATTACK3 	= 0x07,
	ANIM_ANI_DIE1 		= 0x08,
	ANIM_ANI_FIDGET1	= 0x09,
	ANIM_ANI_FIDGET2	= 0x0a,
	ANIM_ANI_SLEEP		= 0x0b,	// lie down
	ANIM_ANI_DIE2		= 0x0c,
};

enum CRESND_TYPE	// Creature sound types.
{
	CRESND_RAND1,
	CRESND_RAND2,
	CRESND_HIT,
	CRESND_GETHIT,
	CRESND_DIE,
};

enum TALKMODE_TYPE	// Modes we can talk/bark in.
{
	TALKMODE_SYSTEM = 0,	// just normal talk mode.
	TALKMODE_EMOTE = 2,		// :	*smiles*
	TALKMODE_SAY = 3,		// ???
	TALKMODE_ITEM = 6,		// ???
	TALKMODE_WHISPER = 8,	// ;
	TALKMODE_YELL = 9,		// !
	TALKMODE_BROADCAST = 0xFF,
};

enum FONT_TYPE
{
	FONT_BOLD,		// 0 - Bold Text
	FONT_SHAD,		// 1 - Text with shadow
	FONT_BOLD_SHAD,	// 2 - Bold+Shadow
	FONT_NORMAL,	// 3 - Normal (default)
	FONT_GOTH,		// 4 - Gothic
	FONT_ITAL,		// 5 - Italic Script
	FONT_SM_DARK,	// 6 - Small Dark Letters
	FONT_COLOR,		// 7 - Colorful Font (Buggy?)
	FONT_RUNE,		// 8 - Rune font (Only use capital letters with this!)
	FONT_SM_LITE,	// 9 - Small Light Letters
};

enum STAT_TYPE	// Character stats
{
	STAT_STR = 0,
	STAT_INT,
	STAT_DEX,

	// ??? Weight,AC,Gold.
	// Notoriety.
	STAT_Karma,		// -10000 to 10000
	STAT_Fame,
	STAT_Kills,

	STAT_QTY,
};

enum SKILL_TYPE	// List of skill numbers (things that can be done at a given time)
{
	SKILL_NONE = -1,

	SKILL_ALCHEMY = 0,
	SKILL_ANATOMY,
	SKILL_ANIMALLORE,
	SKILL_ITEMID,
	SKILL_ARMSLORE,
	SKILL_PARRYING,
	SKILL_BEGGING,
	SKILL_BLACKSMITHING,
	SKILL_BOWCRAFT,
	SKILL_PEACEMAKING,
	SKILL_CAMPING,	// 10
	SKILL_CARPENTRY,
	SKILL_CARTOGRAPHY,
	SKILL_COOKING,
	SKILL_DETECTINGHIDDEN,
	SKILL_ENTICEMENT,
	SKILL_EVALINT,
	SKILL_HEALING,
	SKILL_FISHING,
	SKILL_FORENSICS,
	SKILL_HERDING,	// 20
	SKILL_HIDING,
	SKILL_PROVOCATION,
	SKILL_INSCRIPTION,
	SKILL_LOCKPICKING,
	SKILL_MAGERY,		// 25
	SKILL_MAGICRESISTANCE,
	SKILL_TACTICS,
	SKILL_SNOOPING,
	SKILL_MUSICIANSHIP,
	SKILL_POISONING,	// 30
	SKILL_ARCHERY,
	SKILL_SPIRITSPEAK,
	SKILL_STEALING,
	SKILL_TAILORING,
	SKILL_TAMING,
	SKILL_TASTEID,
	SKILL_TINKERING,
	SKILL_TRACKING,
	SKILL_VETERINARY,
	SKILL_SWORDSMANSHIP,	// 40
	SKILL_MACEFIGHTING,
	SKILL_FENCING,
	SKILL_WRESTLING,
	SKILL_LUMBERJACKING,
	SKILL_MINING,

	SKILL_QTY,	// 50

	// Actions a npc will perform. (no need to track skill level for these)
	NPCACT_FOLLOW_OWN,
	NPCACT_FOLLOW_TARG,
	NPCACT_STAY,
	NPCACT_GOTO,			// Go to a location x,y
	NPCACT_WANDER,			// Wander aimlessly.
	NPCACT_WANDERPOINT,		// Just wander around a central point.
	NPCACT_FLEE,			// Run away from target.
	NPCACT_TALK,			// Talking to my target.
	NPCACT_TALK_FOLLOW,
	NPCACT_GUARD_TARG,		// Guard a targetted object.
	// NPCACT_PICKUP,		// Pick up a thing.
};

enum LOGIN_ERR_TYPE	// error codes sent to client.
{
	LOGIN_ERR_NONE = 0,		// no account
	LOGIN_ERR_USED = 1,		// already in use.
	LOGIN_ERR_BLOCKED = 2,	// we don't like you
	LOGIN_ERR_BADPASS = 3,
	LOGIN_ERR_OTHER,		// like timeout.
};

enum LAYER_TYPE		// defined by UO. Only one item can be in a slot.
{
	LAYER_NONE = 0,	// spells that are layed on the CChar ?
	LAYER_HAND1,	// spellbook or weapon.
	LAYER_HAND2,	// other hand or 2 handed thing. = shield
	LAYER_SHOES,
	LAYER_PANTS,
	LAYER_SHIRT,
	LAYER_HELM,		// 6
	LAYER_GLOVES,	// 7
	LAYER_RING,		
	LAYER_UNUSED9,	// 9 = not used
	LAYER_COLLAR,	// 10 = gorget or necklace.
	LAYER_HAIR,		// 11 = 0x0b =
	LAYER_HALF_APRON,// 12 = 0x0c =
    LAYER_CHEST,	// 13 = 0x0d = armor chest
	LAYER_WRIST,	// 14 = 0x0e = watch
	LAYER_PACK2,	// ??? 15 = 0x0f = alternate pack ? LAYER_BACKPACK2 
	LAYER_BEARD,	// 16 = try to have only men have this.
	LAYER_TUNIC,	// 17 = jester suit or full apron.
	LAYER_EARS,
    LAYER_ARMS,		// 19 = armor
	LAYER_CAPE,		// 20 = cape
	LAYER_PACK,		// 21 = 0x15 = only used by ITEMID_BACKPACK
	LAYER_ROBE,		// 22 = robe over all.
    LAYER_SKIRT,
    LAYER_LEGS,		// 24= 0x18 = plate legs.

	// These are not part of the paper doll
	LAYER_HORSE,		// 25 = 0x19 = ride this object. (horse objects are strange?)
	LAYER_VENDOR_STOCK,	// 26 = 0x1a
	LAYER_VENDOR_EXTRA,
	LAYER_VENDOR_SELL,	// 27 = 0x1c = the stuff the vendor can buy from player.
	LAYER_BANKBOX,		// 28 = 0x1d

	// Don't bother sending these to client.
	LAYER_DRAGGING,

	// Spells that are effecting us go here.
	LAYER_SPELL_STATS,		// 29 = Stats effecting spell. These cancel each other out.
	LAYER_SPELL_Reactive,	// 30 = 
	LAYER_SPELL_Night_Sight,
	LAYER_SPELL_Poison,
	LAYER_SPELL_Protection,
	LAYER_SPELL_Incognito,
	LAYER_SPELL_Magic_Reflect,
	LAYER_SPELL_Paralyze,
	LAYER_SPELL_Invis,
	LAYER_SPELL_Polymorph,

};

enum DIR_TYPE	// Walking directions. m_dir
{
	DIR_N = 0,
	DIR_NE,
	DIR_E,
	DIR_SE,
	DIR_S,
	DIR_SW,
	DIR_W,
	DIR_NW,
	DIR_QTY,
};

//////////////////

#if defined _WIN32 && (!__MINGW32__)
// Microsoft dependant pragma
#pragma pack(1)	
#define PACK_NEEDED
#else
// GCC based compiler you can add:
#define PACK_NEEDED __attribute__ ((packed))
#endif

struct NWORD
{
	// Deal with the stupid Little Endian / Big Endian crap just once.
	// On little endian machines this code would do nothing.
	WORD m_val;
	operator WORD () const
	{
		return( ntohs( m_val ));
	}
	NWORD & operator = ( WORD val )
	{
		m_val = htons( val );
		return( * this );
	}
} PACK_NEEDED;
struct NDWORD
{
	DWORD m_val;
	operator DWORD () const
	{
		return( ntohl( m_val ));
	}
	NDWORD & operator = ( DWORD val )
	{
		m_val = htonl( val );
		return( * this );
	}
} PACK_NEEDED;

struct CEvent	// event buffer from client.
{
#define MAX_SERVERS		32		// Maximum servers in login listing
#define MAX_STARTS		32		// Maximum starting locations
#define MAX_ITEMS_CONT  128		// max items in a container.
#define MAX_NAME_SIZE	30		// imposed by client for protocol

	// Some messages are bydirectional.

	union
	{
		BYTE m_Raw[ MAX_BUFFER ];

		struct 
		{
			BYTE m_Cmd;		// 0 = ?
			BYTE m_Arg[1];	// unknown size.
		} Default;	// default unknown type.

		struct	// size = 100	// create a new char
		{
			BYTE m_Cmd;		// 0=0
			BYTE m_unk1[9];
			char m_name[MAX_NAME_SIZE];		// 10
			char m_pass[MAX_NAME_SIZE];		// 40

			BYTE m_sex;		// 70, 0 = male
			BYTE m_str;		// 71
			BYTE m_dex;		// 72
			BYTE m_int;		// 73

			BYTE m_skill1;
			BYTE m_val1;
			BYTE m_skill2;
			BYTE m_val2;
			BYTE m_skill3;
			BYTE m_val3;

			NWORD m_color;	// 0x50 // COLOR_TYPE
			NWORD m_hairid;
			NWORD m_haircolor;
			NWORD m_beardid;
			NWORD m_beardcolor;	// 0x58
			NWORD m_startloc;	// 0x5B
			NWORD m_unk5c;
			NWORD m_slot;
			BYTE m_clientip[4];
		} Create;

		struct	// size = 3
		{
			BYTE m_Cmd;		// 0 = 0x02
			BYTE m_dir;		// 1 = DIR_TYPE (| 0x80 = running)
			BYTE m_count; 	// 2 = just a count that goes up as we walk. (handshake)
		} Walk;

		struct	// size = >3	// user typed in text.
		{
			BYTE m_Cmd;		// 0 = 0x03
			NWORD m_len;	// 1,2=length of packet
			BYTE m_mode;	// 3=mode(9=yell)
			NWORD m_color;
			BYTE m_unk6[2];	// 6,7 = font ?
			char m_text[1];	// 8=var size
		} Talk;

		struct	// size = 5
		{
			BYTE m_Cmd;	// 0 = 0x05, 0x06 or 0x09
			NDWORD m_UID;
		} Click;	// Attack, Click or DClick

		struct // size = 7 = pick up an item
		{
			BYTE m_Cmd;			// 0 = 0x07
			NDWORD m_UID;		// 1-4
			NWORD m_amount;
		} ItemGet;

		struct	// size = 14 = drop item on ground or in container.
		{
			BYTE m_Cmd;			// 0 = 0x08
			NDWORD m_UID;		// 1-4 = object being dropped.
			NWORD m_x;		// 5,6 = 255 = invalid
			NWORD m_y;		// 7,8
			BYTE m_z;			// 9
			NDWORD m_UIDCont;	// 10 = dropped on this object. 0xFFFFFFFF = no object
		} ItemDrop;

		struct	// size > 3
		{
			BYTE m_Cmd;		// 0 = 0x12
			NWORD m_len;	// 1-2 = len
			BYTE m_type;	// 3 = 0xc7=anim, 0x24=skill...
			char m_name[1];	// 4=var size string
		} ExtCmd;

		struct	// size = 10 = item dropped on paper doll.
		{
			BYTE m_Cmd;		// 0 = 0x13
			NDWORD m_UID;	// 1-4	item UID.
			BYTE m_layer;	// LAYER_TYPE
			NDWORD m_UIDChar;
		} ItemEquip;

		struct // size = 3	// WalkAck gone bad
		{
			BYTE m_Cmd;		// 0 = 0x22
			BYTE m_unk1[2];
		} ReSyncReq;

		struct // size = 7(m_mode==0) or 2(m_mode!=0)  // Manifest ghost (War Mode) or auto res ?
		{
			BYTE m_Cmd;		// 0 = 0x2c
			BYTE m_mode;	// 1 = 0=manifest or unmanifest, 1=res w/penalties, 2=play as a ghost
			BYTE m_unk2;	// 2 = 72 or 73
			BYTE m_manifest;// 3 = manifest or not. = war mode.
			BYTE m_unk4[3]; // 4 = unk = 00 32 00
		} DeathOpt;

		struct	// size = 10	// Client requests stats.
		{
			BYTE m_Cmd;		// 0 =0x34
			NDWORD m_edededed;	// 1-4 = 0xedededed
			BYTE m_type;	// 5 = 4 = Basic Stats (Packet 0x11 Response)  5 = Request Skills (Packet 0x3A Response) 
			NDWORD m_UID;	// 6 = character UID for status
		} CharStatReq;

		struct // size = variable // Buy item from vendor.
		{
			BYTE m_Cmd;			// 0 =0x3b
			NWORD m_len;		// 1-2=
			NDWORD m_UIDVendor;	// 3-6=
			BYTE m_flag;	// 0x00 = no items following, 0x02 - items following 
			struct 
			{
				BYTE m_layer; // (0x1A) 
				NDWORD m_UID; // itemID (from 3C packet) 
				NWORD m_amount; //  # bought 
			} items[1];
		} VendorBuy;

		struct // size = 7	// ??? No idea
		{
			BYTE m_Cmd;		// 0 =0x3d
			BYTE m_unk1[6];
		} Unk3d;

		struct	// size = 11	// plot course on map.
		{
			BYTE m_Cmd;		// 0 = 0x56
			BYTE m_unk1[10];
		} MapPlot;

		struct	// size = 0x49 = 73	// play this slot char
		{
			BYTE m_Cmd;			// 0 = 0x5D
			NDWORD m_edededed;	// 1-4 = ed ed ed ed
			char m_name[MAX_NAME_SIZE];
			char m_pass[MAX_NAME_SIZE];
			NDWORD m_slot;		// 0x44 = slot
			BYTE m_clientip[4];	// = 18 80 ea 15
		} Play;

		struct	// size > 3
		{
			BYTE m_Cmd;		// 0 = 0x66
			NWORD m_len;	// 1-2
			NDWORD m_UID;	// 3-6 = the book
			NWORD m_pages;	// 7-8 = 0x0001 = # of pages
			NWORD m_page;	// 9-10=page number 
			NWORD m_lines;	// 11
			char m_text[1];
		} BookPage;

		struct	// size = var // Client text color changed but it does not tell us to what !!
		{
			BYTE m_Cmd;		// 0=0x69
			NWORD m_len;
			BYTE m_data[1];
		} Options;

		struct	// size = 19
		{
			BYTE m_Cmd;		// 0 = 0x6C
			BYTE m_Req;		// 1 = 1=send, 0=response ? NOT REALLY CORRECT
			NWORD m_One;	// 2-3 = 1 ?
			NWORD m_code;	// 2-5 = we sent this at target setup.
			BYTE m_type;	// 6= 0=select object, 1=x,y,z
			
			NDWORD m_UID;	// 7-10	= serial number of the target.
			NWORD m_x;		// 11,12
			NWORD m_y;		// 13,14
			BYTE m_unk2;	// 15 = fd ?
			BYTE m_z;		// 16
			NWORD m_id;		// 17-18 = static id of tile
		} Target;

		struct // size = var // Secure trading
		{
			BYTE m_Cmd;		// 0=0x6F
			NWORD m_len;	// 1-2 = len
			BYTE m_action;	// 3 = trade action
			NDWORD m_UID;	// 4-7 = uid 

		} SecureTrade;

		struct // size = 5	// Client wants to go into war mode.
		{
			BYTE m_Cmd;		// 0 = 0x72
			BYTE m_warmode;	// 1 = 0 or 1 
			BYTE m_unk2[3];	// 2 = 00 32 00
		} War;

		struct // size = 2	// ping goes both ways.
		{
			BYTE m_Cmd;		// 0 = 0x73
			BYTE m_unk1;	// 1 = ?
		} Ping;

		struct // size = 35	// set the name for this char
		{
			BYTE m_Cmd;		// 0=0x75
			NDWORD m_UID;
			char m_name[ MAX_NAME_SIZE ];
		} CharName;

		struct	// size = 13	// choice off a menu
		{
			BYTE m_Cmd;		// 0 = 0x7d
			NDWORD m_UID;	// 1 = dialog id from 0x7c message
			NWORD m_menuid;	// 5,6
			NWORD m_select;	// 7,8
			BYTE m_unk9[4];
		} Choice;

		struct // size = 62		// first login to req listing the servers.
		{
			BYTE m_Cmd;	// 0 = 0x80
			char m_name[ MAX_NAME_SIZE ];
			char m_password[ MAX_NAME_SIZE ];
			BYTE m_unk;	// 61 = ff
		} Servers;

		struct	// size = 39  // delete the char in this slot. 
		{
			BYTE m_Cmd;		// 0 = 0x83
			BYTE m_password[MAX_NAME_SIZE];
			NDWORD m_slot;	// 0x22
			BYTE m_clientip[4];
		} Delete;

		struct	// size = 65	// request to list the chars I can play.
		{
			BYTE m_Cmd;		  // 0 = 0x91
			NDWORD m_Account; // 1-4 = account id from Relay message to log server.
			char m_account[MAX_NAME_SIZE];	// This is corrupted or encrypted seperatly ?
			char m_password[MAX_NAME_SIZE];
		} List;

		struct	// size = 9		// select color from dye chart.
		{
			BYTE m_Cmd;		// 0x95
			NDWORD m_UID;	// 1-4
			NWORD m_unk5;
			NWORD m_color;	// 7,8
		} DyeVat;

		struct // size = 0x102	// GM page = request for help.
		{
			BYTE m_Cmd;		// 0 = 0x9b
			BYTE m_unk1[0x101];
		} HelpPage;

		struct // size = var // Sell stuff to vendor
		{
			BYTE m_Cmd;		// 0=0x9F
			NWORD m_len;	// 1-2
			NDWORD m_UIDVendor;	// 3-6
			NWORD m_count;		// 7-8
			struct
			{
				NDWORD m_UID;
				NWORD m_amount;
			} items[1];
		} VendorSell;

		struct	// size = 3;	// relay me to the server i choose.
		{
			BYTE m_Cmd;		// 0 = 0xa0
			NWORD m_select;	// 2 = selection this server number
		} Relay;

		struct	// sizeof = 149 // not sure what this is for.
		{
			BYTE m_Cmd;		// 0=0xA4
			BYTE m_unk1[ 148 ];	// Just junk about your computers equip.
		} Spy;

		struct // size = 5	// scroll close ??? not sure
		{
			BYTE m_Cmd;			// 0=0xA6
			BYTE m_unk1[4];
		} ScrollClose;

		struct	// size = 4 // request for tip
		{
			BYTE m_Cmd;		// 0=0xA7
			NWORD m_index;
			BYTE m_type;	// 0=tips, 1=notice
		} Tip;

		struct // size = var // Get text from gump input
		{
			BYTE m_Cmd;		// 0=0xAC
			NWORD m_len;
		} GumpText;

		struct // size = var // Get gump button change
		{
			BYTE m_Cmd;		// 0=0xB1
			NWORD m_len;
		} GumpButton;

	};
} PACK_NEEDED;

struct CCommand	// command buffer to client.
{
	union
	{
		BYTE m_Raw[ MAX_BUFFER ];

		struct // size = 36 + 30 = 66	// Get full status.
		{
			BYTE m_Cmd;			// 0 = 0x11
			NWORD  m_len;		// 1-2 = size of packet (2 bytes)
            NDWORD m_UID;		// 3-6 = (first byte is suspected to be an id byte - 0x00 = player)
            char m_name[ MAX_NAME_SIZE ];	// 7-36= character name (30 bytes) ( 00 padded at end)
			NWORD m_health;		// 37-38 = current health 
            NWORD m_maxhealth;	// 39-40 = max health 
			BYTE m_perm;		// 41 = permission to change name ? 0=stats invalid, 1=stats valid., 0xff = i can change the name
			BYTE m_ValidStats;	// 42 = 1 = valid stats ? not sure.
            BYTE m_sex;			// 43 = sex (0 = male)
            NWORD m_str;		// 44-45 = strength 
            NWORD m_dex;		// 46-47 = dexterity
            NWORD m_int;		// 48-49 = intelligence 
            NWORD m_stam;		// 50-51 = current stamina 
            NWORD m_maxstam;	// 52-53 = max stamina 
            NWORD m_mana;		// 54-55 = current mana 
            NWORD m_maxmana;	// max mana (2 bytes)
            NDWORD m_gold;		// 58-61 = gold 
            NWORD m_armor;		// 62-63 = armor class
            NWORD m_weight;		// 64-65 = weight in stones
		} Status;

		struct // size = 14 + 30 + var
		{
			BYTE m_Cmd;			// 0 = 0x1C
			NWORD m_len;		// 1-2 = var len size.
			NDWORD m_UID;		// 3-6 = UID num of speaker.
			NWORD m_id;			// 7-8 = CREID_TYPE of speaker.
			BYTE m_mode;		// 9 = TALKMODE_TYPE
			NWORD m_color;		// 10-11 = color code.
			NWORD m_font;		// 12-13 = FONT_TYPE	
			char m_name[MAX_NAME_SIZE];	// 14
			char m_text[1];		// var size.
		} Speak;

		struct // sizeo = 5	// remove an object (item or char)
		{
			BYTE m_Cmd;			// 0 = 0x1D
			NDWORD m_UID;		// 1-4 = object UID.
		} Remove;

		struct // size = 19	or var len // draw the item at a location
		{
			BYTE m_Cmd;		// 0 = 0x1a
			NWORD m_len;	// 1-2 = var len = 0x0013 or 0x000e or 0x000f
			NDWORD m_UID;	// 3-6 = UID | UID_SPEC
			NWORD m_id;		// 7-8
			NWORD m_amount;	// 9-10 - only present if m_UID | UID_SPEC = pile
			NWORD m_x;		// 11-12 - | 0x8000 = direction arg.
			NWORD m_y;		// 13-14 = y | 0xC000 = color and move fields.	
			BYTE m_z;		// 15
			NWORD m_color;	// 16-17
			BYTE m_movable;	// 18 = 0x20 = is it movable ? (direction?)
		} Put;

		struct // size = 37	// start up
		{
			BYTE m_Cmd;		// 0 = 0x1B
			NDWORD m_UID;	// 1-4
			NDWORD m_zero5;	// 5-8 = 0
			NWORD m_id;		// 9-10
			NWORD m_x;
			NWORD m_y;
			NWORD m_z;		// 16
			BYTE m_dir;		// 17
			BYTE m_unk18[10];	// 18= (19)
			BYTE m_mode;	// 28=0=normal, 0x40=attack, 0x80=hidden	
			BYTE m_unk29[8];
		} Start;

		struct // size = 19 // set client view x,y,z.
		{
			BYTE m_Cmd;			// 0 = 0x20
			NDWORD m_UID;		// 1-4 = my UID.
			NWORD m_id;			// 5-6
			BYTE m_zero7;		// 7 = 0
			NWORD m_color;		// 8-9
			BYTE m_mode;		// 10 = 0x40 = attack mode., 0x80 = hidden
			NWORD m_x;			// 11-12
			NWORD m_y;			// 13-14
			NWORD m_zero15;		// 15-16 = noto ?
			BYTE m_dir;			// 17
			BYTE m_z;			// 18
		} View;	

		struct // size = 8
		{
			BYTE m_Cmd;			// 0 = 0x21
			BYTE m_count;		// sequence # 
			NWORD m_x;
			NWORD m_y;
			BYTE m_dir;
			BYTE m_z;
		} MoveCancel;

		struct	// size=3
		{
			BYTE m_Cmd;		// 0 = 0x22
			BYTE m_count; 	// 1 = goes up as we walk. (handshake)
			BYTE m_unused;	// 2 = 0 or 0x41 (I can't make this do anything???)
		} WalkAck;

		struct // size = 26 
		{
			BYTE m_Cmd;		// 0=0x23
			NWORD m_id;
			NWORD m_unk3;	// 3-4
			BYTE  m_unk5;	// 5
			NWORD m_amount;	// 6-7
			NDWORD m_srcUID;
			NWORD m_src_x;
			NWORD m_src_y;
			BYTE m_src_z;
			NDWORD m_dstUID;
			NWORD m_dst_x;
			NWORD m_dst_y;
			BYTE m_dst_z;
		} DragAnim;

		struct // size = 7	// Open a container gump
		{
			BYTE m_Cmd;		// 0 = 0x24
			NDWORD m_UID;	// 1-4
			NWORD m_gump;	// 5 = gump id.
		} Open;

		struct // size = 20	// Add Single Item To Container.
		{
			BYTE m_Cmd;		// 0 = 0x25
			NDWORD m_UID;	// 1-4
			NWORD m_id;
			BYTE m_zero7;
			NWORD m_amount;
			NWORD m_x;
			NWORD m_y;
			NDWORD m_contUID;
			NWORD m_color;
		} ItemAddCont;

		struct // size = 5	// Kick the player off.
		{
			BYTE m_Cmd;		// 0 = 0x26
			NDWORD m_unk1;	// 1-4 = 0 = GM who issued kick ?
		} Kick;

		struct // size = 2	// kill a drag
		{
			BYTE m_Cmd;		// 0 = 0x27
			BYTE m_type;	// 1 = bounce type ? = 5 = drag
		} DragCancel;

		struct // size = 5	// clear square. Never used this.
		{
			BYTE m_Cmd;		// 0 = 0x28
			NWORD m_x;
			NWORD m_y;
		} ClearSquare;

		struct // size = 2 Death Menu
		{
			BYTE m_Cmd;		// 0 = 0x2c
			BYTE m_shift;	// 1 = 0
		} DeathMenu;
		
		struct // size = 15 = equip this item
		{
			BYTE m_Cmd;			// 0 = 0x2e
			NDWORD m_UID;		// 1-4 = object UID.
			NWORD m_id;			// 5-6
			BYTE m_zero7;
			BYTE m_layer;		// 8=layer
			NDWORD m_UIDchar;	// 9-12=UID num of owner.
			NWORD m_color;
		} Equip;

		struct // size = 10	// There is a fight some place.
		{
			BYTE m_Cmd;		// 0 = 0x2f
			BYTE m_zero1;	// 1 = 0
			NDWORD m_AttackerUID;
			NDWORD m_AttackedUID;
		} Fight;

		struct // size = 2
		{
			BYTE m_Cmd;		// 0 = 0x33
			BYTE m_Arg;		// 1 = (1=pause,0=restart)
		} Pause;

		struct // size = 0xbe = Fill in the skills list.
		{
			BYTE m_Cmd;		// 0= 0x3A
			NWORD m_size;	// 1= 0x00be
			BYTE m_zero3;	// 3=0
			struct
			{
				NWORD m_index;	// 1 based, 0 = terminator. (no val)
				NWORD m_val;	// Skill * 10 	
			} skills[1];
		} Skill;

		struct // size = variable	// close the vendor window.
		{
			BYTE m_Cmd;		// 0 =0x3b
			NWORD m_len;
			NDWORD m_UIDVendor;
			BYTE m_flag;	// 0x00 = no items following, 0x02 - items following 
		} VendorClose;

		struct // size = 5 + ( x * 19 ) // set up the content of some container.
		{
			BYTE m_Cmd;		// 0 = 0x3C
			NWORD m_len;
			NWORD m_count;

			struct // size = 19
			{
				NDWORD m_UID;	// 0-3
				NWORD m_id;	// 4-5
				BYTE m_zero6;
				NWORD m_amount;	// 7-8
				NWORD m_x;	// 9-10	
				NWORD m_y;	// 11-12
				NDWORD m_UIDCont;	// 13-16 = What container is it in.
				NWORD m_color;	// 17-18
			} items[ MAX_ITEMS_CONT ];
		} Content;

		struct // size = 6	// personal light level.
		{
			BYTE m_Cmd;		// 0 = 0x4e
			NDWORD m_UID;	// 1 = creature uid
			BYTE m_level;	// 5 = 0
		} LightPoint;

		struct // size = 2
		{
			BYTE m_Cmd;			// 0 = 0x4f
			BYTE m_level;		// 1=0-19, 19=dark
		} Light;

		struct // size = 12
		{
			BYTE m_Cmd;		// 0 = 0x54
			BYTE m_unk1;	// 1 = 1 (i have seen 0)
			NWORD m_id;		// 2-3=sound id (SOUND_TYPE)
			NWORD m_zero4;	// 4-5=0 = (speed/volume modifier?)
			NWORD m_x;		// 6-7
			NWORD m_y;		// 8-9	
			NWORD m_z;		// 10-11
		} Sound;

		struct	// size = 1	// No idea. Sent at start up.
		{
			BYTE m_Cmd;		// 0 =0x55
		} Unk55;

		struct	// size = 11
		{ 
			BYTE m_Cmd; // 0 = 0x56
			NDWORD m_UID;
			BYTE m_unk[6];
		} Map2;

		struct	// size = 4	// Set Game Time
		{
			BYTE m_Cmd;		// 0 =0x5b
			BYTE m_hours;
			BYTE m_min;
			BYTE m_sec;
		} Time;

		struct // size = 4
		{
			BYTE m_Cmd;			// 0 = 0x65
			BYTE m_type;		// 1 = type. 0=dry, 1=rain, 2=snow
			NWORD m_ext;		// 2 = other weather info (severity?)
		} Weather;

		struct	// size = 13 + var // send a book page.
		{
			BYTE m_Cmd;		// 0 = 0x66
			NWORD m_len;	// 1-2
			NDWORD m_UID;	// 3-6 = the book
			NWORD m_pages;	// 7-8 = 0x0001 = # of pages
			NWORD m_page;	// 9-10=page number 
			NWORD m_lines;	// 11
			char m_text[1];
		} BookPage;

		struct	// size = var
		{
			BYTE m_Cmd;		// 0=0x69
			NWORD m_len;
			BYTE m_data[1];
		} Options;

		struct	// size = 19
		{
			BYTE m_Cmd;		// 0 = 0x6C
			BYTE m_Req;		// 1 = 1=send, 0=response ? NOT REALLY CORRECT
			NWORD m_One;	// 2-3 = 1
			NWORD m_code;	// 4-5 = we sent this at target setup.
			BYTE m_type;	// 6= 0=select object, 1=x,y,z
			
			NDWORD m_UID;	// 7-10	= serial number of the target.
			NWORD m_x;		// 11,12
			NWORD m_y;		// 13,14
			BYTE m_unk2;	// 15 = fd ?
			BYTE m_z;		// 16
			NWORD m_id;		// 17-18 = static id of tile
		} Target;

		struct // size = 3 // play music
		{
			BYTE m_Cmd;		// 0 = 0x6d
			NWORD m_musicid;// 1= music id number	
		} PlayMusic;

		struct // size = 14 // Combat type animation.
		{
			BYTE m_Cmd;		// 0 = 0x6e
			NDWORD m_UID;	// 1-4=uid
			NWORD m_action;	// 5-6 = ANIM_TYPE
			BYTE m_zero7;
			BYTE m_dir;
			NWORD m_repeat;	// 9-10 = repeat count. 0=forever.
			BYTE m_zero11;
			BYTE m_repflag;	// 0 = dont repeat. 1=repeat
			BYTE m_framedelay;	// 0=fastest.
		} CharAction;
		
		struct // size = 28 // Graphical effect.
		{
			BYTE m_Cmd;			// 0 = 0x70
			BYTE m_motion;		// 1= the motion type. (0=point to point,3=static)
			NDWORD m_UID;		// 2-5 = The target item.
			NDWORD m_targUID;	// 6-9 = 0
			NWORD m_id;			// 10-11 = base display item 0x36d4 = fireball
			NWORD m_srcx;		// 12
			NWORD m_srcy;		// 14	
			BYTE  m_srcz;		// 16
			NWORD m_dstx;		// 17
			NWORD m_dsty;		// 19
			BYTE  m_dstz;		// 21
			BYTE  m_speed;		// 22= 0=very fast, 7=slow.
			BYTE  m_loop;		// 23= 0 is really long.  1 is the shortest.
			WORD  m_zero24;		// 24=0 unknown ? color ?
			BYTE  m_OneDir;		// 26=1=point in single dir else turn.
			BYTE  m_explode;	// 27=effects that explode on impact.
 
		} Effect;

		struct // size = 5	// put client to war mode.
		{
			BYTE m_Cmd;		// 0 = 0x72
			BYTE m_warmode;	// 1 = 0 or 1 
			BYTE m_unk2[3];	// 2 = 00 32 00
		} War;

		struct // size = var // Open buy window
		{
			BYTE m_Cmd;		// 0 = 0x74
			NWORD m_len;
			NDWORD m_VendorUID;
			BYTE m_count;
			struct
			{
				NDWORD m_price;
				BYTE m_len;
				char m_text[1];
			} items[1];
		} OpenBuy;

		struct // size = 17	// simple move of a char already on screen.
		{
			BYTE m_Cmd;		// 0 = 0x77
			NDWORD m_UID;	// 1-4
			NWORD m_id;		// 5-6 = id
			NWORD m_x;		// 7-8 = x
			NWORD m_y;		// 9-10
			BYTE m_z;		// 11
			BYTE m_dir;		// 12 = dir (| 0x80 = running ?)
			NWORD m_color;	// 13-14 = color
			BYTE m_mode;	// 15 = war mode = 0x40 else 0, 0x80 = hidden
			BYTE m_noto;	// 16 = 0, 1, 2 or 3 = notoriety ?
		} CharMove;

		struct // size = 23 or var len // draw char
		{
			BYTE m_Cmd;		// 0 = 0x78
			NWORD m_len;	// 1-2 = 0x0017 or var len?
			NDWORD m_UID;	// 3-6=
			NWORD m_id;	// 7-8
			NWORD m_x;	// 9-10
			NWORD m_y;	// 11-12
			BYTE m_z;		// 13
			BYTE m_dir;		// 14
			NWORD m_color;	// 15-16
			BYTE m_mode;	// 17 = war mode = 0x40 else 0, 0x80 = hidden
			BYTE m_noto;	// 18 = 0, 1, 2 or 3 = notoriety ?

			struct	// This packet is extendable to show equip.
			{
				NDWORD m_UID;	// 0 = 0 = end of the list.
				NWORD m_id;		// 
				BYTE m_layer;	// LAYER_TYPE
				NWORD m_color;	// only if m_id | 0x8000
			} equip[1];

		} Char;

		struct // size = var // put up a menu of items.
		{
			BYTE m_Cmd;			// 0=0x7C
			NWORD m_len;
			NDWORD m_UID;		// if player then gray menu choice bar.
			NWORD m_id;			// The menu (gump?) id
			BYTE m_lenname;
			char m_name[1];		// var len
			struct		// size = var
			{
				NWORD m_id;		// image next to menu.
				NWORD m_zero2;	// check or not ?
				BYTE m_lentext;
				char m_name[1];	// var len
			} items[1];
		} ItemMenu;

		struct // size = 2
		{
			BYTE m_Cmd;		// 0 = 0x82
			BYTE m_code;	// 1 = LOGIN_ERR_*
		} LogBad;

		struct // size = var	// refill char list after delete.
		{
			BYTE m_Cmd;		// 0 = 0x86
			NWORD m_len;
			BYTE m_count;
			struct
			{
				char m_name[MAX_NAME_SIZE];
				char m_pass[MAX_NAME_SIZE];
			} m_char[5];
		} CharList2;

		struct // size = 66
		{
			BYTE m_Cmd;			// 0 = 0x88
			NDWORD m_UID;		// 1-4 = 
			char m_text[60];	// 5-	
			BYTE m_mode;		// 0=normal, 0x40 = attack
		} PaperDoll;

		struct // size = 7 + count * 5 // Equip stuff on corpse.
		{
			BYTE m_Cmd;		// 0 = 0x89
			NWORD m_len;
			NDWORD m_UID;

			struct // size = 5
			{
				BYTE m_layer;	// 0 = LAYER_TYPE
				NDWORD m_UID;	// 1-4
			} items[ MAX_ITEMS_CONT ];
		} CorpEquip;	

		struct	// size = 11
		{
			BYTE m_Cmd;			// 0 = 0x8C
			BYTE m_ip[4];		// 1 = struct in_addr 
			NWORD m_port;		// 5 = Port server is on
			NDWORD m_Account;	// 7-10 = customer id (sent to game server)
		} Relay;

		struct	// size = 19
		{ 
			BYTE m_Cmd; // 0 = 0x90
			NDWORD m_UID;
			BYTE m_unk[14];
		} Map1;

		struct // size = 8 + 3 * MAX_NAME_SIZE
		{
			BYTE m_Cmd;			// 0 = 0x93
			NDWORD m_UID;		// 1-4 = book
			BYTE m_writable;	// 5 = 0 = non writable, 1 = writable.
			NWORD m_pages;		// 6-7 = number of pages.
			char m_title[ 2 * MAX_NAME_SIZE ];
			char m_author[ MAX_NAME_SIZE ];
		} BookOpen;

		struct // size = 9
		{
			BYTE m_Cmd;			// 0 = 0x95
			NDWORD m_UID;		// 1-4
			NWORD m_zero5;		// 5-6
			NWORD m_id;		// 7-8
		} DyeVat;

		struct // size = 26	// preview a house/multi
		{
			BYTE m_Cmd;		// 0 = 0x99
			BYTE m_Req;		// 1 = 1=send, 0=response ? NOT REALLY CORRECT
			NWORD m_One;	// 2-3 = 1
			NWORD m_code;	// 4-5 = we sent this at target setup.
			BYTE m_zero6[12];	// 6-17
			NWORD m_id;		// 18-19 = The muitl id to preview. (id-0x4000)
			BYTE m_zero20[6];
		} TargetMulti;

		struct // size = var	// vendor sell dialog
		{
			BYTE m_Cmd;		// 0 = 0x9e
			NWORD m_len;	// 1-2
			NDWORD m_UIDVendor;	// 3-6
			NWORD m_count;	// 7-8
			struct
			{
				NDWORD m_UID;
				NWORD m_id;
				NWORD m_color;
				NWORD m_amount;
				NWORD m_price;
				NWORD m_len;
				char m_text[1];
			} items[1];
		} OpenSell;

		struct	// size = 9	// update some change in stats.
		{
			BYTE m_Cmd;	// 0=0xa1, 0xa2, or 0xa3
			NDWORD m_UID;	// 1-4
			NWORD m_max;	// 5-6
			NWORD m_val;	// 7-8
		} StatChng;

		struct // size = 3+var
		{
			BYTE m_Cmd;		// 0 = 0xA5
			NWORD m_len;
			char m_page[ 1 ];	// var size
		} Web;

		struct // size = 10 + var	// Read scroll
		{
			BYTE m_Cmd;			// 0=0xA6
			NWORD m_len;		// 1-2
			BYTE  m_type;		// 3 = form or gump = 0=tips,1=notice or 2
			NWORD m_zero4;
			NWORD m_tip;		// 6-7 = response type ? = 31 1c
			NWORD m_lentext;	// 8
			char m_text[1];		// 10
		} Scroll;

		struct // size = 6+servers*40
		{
			BYTE m_Cmd;			// 0 = 0xA8
			NWORD m_len;		// 1-2
			BYTE m_unk3;		// 3=0x14 ?
			NWORD m_count;	// 4-5=num servers.

			struct	// size=40
			{
				NWORD m_count;	// 0=0 based enum
				char m_name[MAX_NAME_SIZE];	// 2
				NWORD m_zero32;	// 32 = 0
				BYTE m_percentfull;	// 34 = 25 or 2e
				BYTE m_timezone;	// 35 = 0x05 or 0x08
				BYTE m_ip[4];		// 36-39 = struct in_addr  
			} m_serv[ MAX_SERVERS ];
		} Servers;

		struct // size = 4+(5*2*MAX_NAME_SIZE)+1+(starts*63) // list available chars for your account.
		{
			BYTE m_Cmd;		// 0 = 0xa9
			NWORD m_len;	// 1-2 = var len

			BYTE m_count;	// 3=5 needs to be 5 for some reason.
			struct // size = 5*60
			{
				char m_name[MAX_NAME_SIZE];
				char m_pass[MAX_NAME_SIZE];
			} m_char[5];

			BYTE m_startcount;
			struct	// size = 63
			{
				BYTE m_id;
				char m_area[MAX_NAME_SIZE+1];
				char m_name[MAX_NAME_SIZE+1];
			} m_start[MAX_STARTS];
		} CharList;

		struct // size = 5
		{
			BYTE m_Cmd;		// 0= 0xaa
			NDWORD m_UID;	// 1= char attacked. 0=attack refused.
		} AttackOK;

		struct // size = 15	// Death of a creature
		{
			BYTE m_Cmd;		// 0 = 0xaf
			NDWORD m_UID;	// 1-4
			NDWORD m_zero5;
			NDWORD m_UIDCorpse; // 9-12
			NWORD m_zero13;
		} CharDeath;

		struct // size = var
		{
			BYTE m_Cmd;		// 0 = 0xb0
			NWORD m_len;	// 1	
			NDWORD m_UID;	// 3
			NWORD m_zero7;	// 7-8
			NWORD m_gump;	// 9-10 = the gump base.
		} GumpMenu;
	};
} PACK_NEEDED;

/////////////////////////////////////////////////////////////////
// File blocks

#define UO_BLOCK_SIZE		8       // Base size of a block.
#define UO_BLOCK_ALIGN(i) 	((i) &~ 7 )
#define UO_BLOCK_ROUND(i)	UO_BLOCK_ALIGN((i)+7)
#define UO_BLOCKS_X			768		// blocks
#define UO_BLOCKS_Y			512
#define UO_SIZE_X			(UO_BLOCKS_X*UO_BLOCK_SIZE)	// Total Size In meters
#define UO_SIZE_Y			(UO_BLOCKS_Y*UO_BLOCK_SIZE)
#define UO_SIZE_X_REAL		(640*UO_BLOCK_SIZE)		// The actual world is only this big

struct CUOMapMeter	// 3 bytes
{
	WORD m_wIndex;	// index to Radarcol and CUOTileTerrainBlock
	signed char m_z;
} PACK_NEEDED;

struct CUOMapBlock	// 196 byte block = 8x8 meters, (map0.mul)
{
	WORD m_wID1;	// ?
	WORD m_wID2;
	CUOMapMeter m_Meter[ UO_BLOCK_SIZE * UO_BLOCK_SIZE ];
} PACK_NEEDED;

struct CUOIndexBlock	// 12 byte block = used for CUOStaticsBlock table dscription. 
{
	DWORD	m_dwOffset;	// 0xFFFFFFFF = nothing here ! else pointer to something (CUOStaticsBlock possibly)
	DWORD 	m_dwLength; // Length of the object in question.
	WORD 	m_wVal3;	// Varied uses
	WORD 	m_wVal4;	// ?
} PACK_NEEDED;

struct CUOStaticsBlock	// 7 byte block = static items
{
	WORD	m_wID;		// ITEMID_TYPE = Index to tile CUOTileItemBlock
	BYTE	m_x;		// x <= 7 = offset from block.
	BYTE 	m_y;		// y <= 7
	signed char m_z;	//
	WORD 	m_wZero;	// 2[0]
} PACK_NEEDED;

struct CUOTileTerrainBlock	// size = 0x1a = 26
{	// First half of file is for terrain tiles.
	BYTE m_type;	// 0xc0=water, 0x40=dirt or rock, 0x60=lava, 0x50=cave, 0=floor
	BYTE m_unk1;
	WORD m_unk2;
	WORD m_index;	// just counts up.  0 = unused.
	char m_name[20];
} PACK_NEEDED;

struct CUOTileItemBlock	// size = 37
{	// Second half of file is for item tiles (ITEMID_TYPE).
	// if all entries are 0 then this is unused and undisplayable.

	// 0x02 = equipable. m_layer is LAYER_TYPE
	// 0x40 = blocked for normal human.
	// 0x80 = water
	BYTE m_type;
	// 0x02 = platform/flat (can stand on)
	// 0x04 = climbable (stairs). m_height /= 2(For Stairs+Ladders)
	// 0x08 = pileable/stackable
	// 0x40 = a
	// 0x80 = an
	BYTE m_flag2;
	// 0x40 = equipable (except for spellbook?)
	BYTE m_unk3;
	// 0x01 = animation with next several object frames.
	BYTE m_unk4;		
	BYTE m_weight;		// 255 = unmovable.
	BYTE m_layer;		// LAYER_TYPE for m_type = 0 or 2
	DWORD m_dwUnk7;		// ??? qty in the case of stackables ?
	DWORD m_dwAnim;		// equipable items animation index.
	WORD m_wUnk15;
	BYTE m_height;		// z height but may not be blocking.
	char m_name[20];	// sometimes legit not to have a name
} PACK_NEEDED;

enum
{
	VERFILE_MAP			= 0x00,
	VERFILE_STAIDX		= 0x01,
	VERFILE_STATICS		= 0x02,
	VERFILE_ARTIDX		= 0x03,
	VERFILE_ART			= 0x04,
	VERFILE_ANIMIDX		= 0x05,
	VERFILE_ANIM		= 0x06,
	VERFILE_SOUNDIDX	= 0x07,
	VERFILE_SOUND		= 0x08,
	VERFILE_TEXIDX		= 0x09,
	VERFILE_TEXMAPS		= 0x0A,
	VERFILE_GUMPIDX		= 0x0B,
	VERFILE_GUMPART		= 0x0C,
	VERFILE_MULTIIDX	= 0x0D,
	VERFILE_MULTI		= 0x0E,
	VERFILE_SKILLSIDX	= 0x0F,
	VERFILE_SKILLS		= 0x10,
	VERFILE_TILEDATA	= 0x1E,
	VERFILE_ANIMDATA	= 0x1F,
};

struct CUOVersionBlock	// ??? fill this in. (verinfo.mul)
{
	DWORD m_file;		// file type id. tiles = 0x1E
	DWORD m_block;		// tile number + 0x200
	DWORD m_filepos;
	DWORD m_length;
	DWORD m_unknown;
} PACK_NEEDED;

// Turn off structure packing.
#if defined _WIN32 && (!__MINGW32__)
#pragma pack()
#endif

struct CPoint
{
	// A point in the world (or in a container)
	WORD m_x;	// equipped items dont need x,y
	WORD m_y;
	signed char m_z; // This might be layer if equipped ? or eqipped on corpse. Not used if in other container.

public:
	void Init()
	{
		m_x = 0;	// invalid location.
		m_y = 0;
		m_z = 0;
	}
	void Move( DIR_TYPE dir, int speed = 1 );
	int GetDist( CPoint p ) const; // Distance between points
	DIR_TYPE GetDir( CPoint p ) const; // Direction to point p
	bool IsValid() const
	{
		if ( m_z <= -127 || m_z >= 127 ) return( false );
		if ( m_x <= 0 || m_x >= UO_SIZE_X ) return( false );
		if ( m_y >= UO_SIZE_Y ) return( false );
		return( true );
	}
	CPoint() { Init(); }
	CPoint( WORD x, WORD y, signed char z = 0 ) 
	{
		m_x = x; m_y = y; m_z = z;
	}
	bool operator == ( CPoint & p )
	{
		return( m_x == p.m_x && m_y == p.m_y && m_z == p.m_z );
	}
	void Read( char * pVal );
	const char * Write( void ) const;
	CQuadrant * GetQuadrant() const;
};

struct CObjUID		// A unique system serial id. 4 bytes long
{
	// This is a ref to a game object. It may or may not be valid.
	// The top few bits are just flags.
#define UID_UNUSED		0xFFFFFFFF	// 0 = not used as well.
#define UID_SPEC		0x80000000	// pileable or special macro flag passed to client.
#define UID_ITEM		0x40000000	// CItem as apposed to CChar based
#define UID_EQUIPPED	0x20000000	// This item is equipped.
#define UID_CONTAINED	0x10000000	// This item is inside another container
#define UID_MASK		0x0FFFFFFF	// lose the upper bits.
#define UID_FREE		0x08000000	// Spellbook needs unused UID's ?
private:
	DWORD m_Val;
public:

	bool IsValidUID( void ) const
	{
		return( m_Val && m_Val != UID_UNUSED );
	}
	void ClearUID( void )
	{
		m_Val = UID_UNUSED;
	}
	bool IsItem( void ) const	// Item vs. Char 
	{
		return( m_Val & UID_ITEM );	// check high byte
	}
	bool IsEquipped( void ) const
	{
		return( m_Val & UID_EQUIPPED );	
	}
	bool IsInContainer( void ) const
	{
		return( m_Val & UID_CONTAINED );
	}
	bool IsTopLevel( void ) const
	{
		return( ! ( m_Val & ( UID_CONTAINED | UID_EQUIPPED )));
	}
	bool operator == ( CObjUID & uid )
	{
		return( m_Val == uid.m_Val );
	}
	bool operator != ( CObjUID & uid )
	{
		return( m_Val != uid.m_Val );
	}
    operator DWORD () const       // as a C string
    {
		return( m_Val );
    }
	void SetContainerFlags( DWORD dwFlags = 0 )
	{
		m_Val &= ( UID_MASK | UID_ITEM );
		m_Val |= dwFlags;
	}
	void SetUID( DWORD dwVal )
	{
		m_Val = dwVal;
	}
	CObjUID()
	{
		ClearUID();
	}
	CObjUID( DWORD dw  )
	{
		m_Val = dw;
	}
	CObjUID( NDWORD dw  )
	{
		m_Val = dw;
	}

	CObjBase * ObjFind() const;
	CItem * ItemFind() const;	// Does item still exist or has it been deleted
	CChar * CharFind() const;	// Does character still exist ?
};

class CObjBase : public CObListRec
{
	// All items or chars have these base attributes.
private:
#ifdef _DEBUG
#define COBJBASE_SIGNATURE	0xDEADBEEF	// used just to make sure this is valid.
	DWORD m_dwSignature;
#endif

	WORD  m_id;		// display type. ITEMID_TYPE or CREID_TYPE
protected:
	CObjUID m_UID;	// Unique id number.

public:
	static int sm_iCount;
	CString m_sName;		// any item/char can have a unique name. if empty then use type name !
	CPoint m_p;				// Map location or (x,y) in container.
	COLOR_TYPE m_color;		// Hue or skin color. (CItems must be < 0x4ff or so)
	time_t	m_timeout;		// when does this rot away ? or other action.

protected:
	WORD GetPrivateID() const
	{
		return( m_id );
	}
	void SetPrivateID( WORD id )
	{
		// Assume the id is basically valid.
		m_id = id;
	}
	void SetPrivateUID( DWORD dwVal, bool fItem );

public:
	CObjBase( WORD id, bool fItem );
	virtual ~CObjBase();
	void SetTimeout( int iDelay );
	virtual void OnTick()
	{
		// What happens when it times out ?
	}

	// UID stuff.
	CObjUID GetUID() const		{	return( m_UID ); }
	bool IsItem( void ) const	{	return( m_UID.IsItem()); }
	bool IsInContainer() const	{	return( m_UID.IsInContainer() ); }
	bool IsEquipped() const		{	return( m_UID.IsEquipped() ); }
	bool IsTopLevel( void ) const {	return( m_UID.IsTopLevel() ); }

	virtual bool IsValid() const
	{
		if ( IsBadReadPtr( this, sizeof(*this))) return( false );
#ifdef _DEBUG
		if ( m_dwSignature != COBJBASE_SIGNATURE ) return( false );
#endif
		return( m_UID.IsValidUID());
	}

	virtual const char * GetName() const
	{	// allow some creatures to go unnamed.
		return( m_sName );
	}
	virtual void SetName( const char * pName )
	{
		m_sName = pName;
	}

	int GetDist( const CObjBase * pObj ) const
	{
		return( m_p.GetDist( pObj->m_p ));
	}
	DIR_TYPE GetDir( const CObjBase * pObj ) const
	{
		return( m_p.GetDir( pObj->m_p ));
	}

	int GetVisualRange() const
	{
#define UO_MAP_VIEW_SIZE		18 // Visibility for normal items
#define UO_MAP_VIEW_BIG_SIZE	31 // Visibility for castles, keeps and boats
		if ( IsItem())
		{
			if ( m_id >= ITEMID_BOAT_N ) return( UO_MAP_VIEW_BIG_SIZE );
		}
		return( UO_MAP_VIEW_SIZE );
	}

	void MoveTo( CPoint p );
	void Sound( SOUND_TYPE id ) const; // Play sound effect from this location.
	void Effect( BYTE motion, ITEMID_TYPE id, const CObjBase * pSource = NULL, BYTE speed = 5, BYTE loop = 1, BYTE explode = false ) const;

	virtual void Write( CScript & s ) const;
	virtual bool LoadVal( const char * pKey, char * pVal );
	void Remove( CClient * pClientExclude = NULL );	// remove this item from all clients.

	virtual void Update( CClient * pClientExclude = NULL )	// send this new item to clients.
	{
		assert(0);
	}
	void UpdateCanSee( CCommand * pCmd, int iLen, CClient * pClientExclude = NULL ) const;
	virtual CContainer * GetThisContainer( void )
	{
		return( NULL );
	}
	bool IsContainer( void )
	{
		return( GetThisContainer() != NULL );
	}
	virtual CObjBase * GetTopLevelObj( void ) const
	{
		// Get the object that has a location in the world. (Ground level)
		return( (CObjBase *) this );
	}
	virtual void Flip( const char * pCmd )
	{
		assert(0);
	}
};

enum TARGMODE_TYPE	// addTarget for async target event
{
	// Capture the user input for this mode.
	TARGMODE_NONE = 0,		// No targetting going on.

	// Making a selection from a menu.
	TARGMODE_MENU_GM,				// The GM type Menu.
	TARGMODE_MENU_ITEMS = 128,	// differs from GM menu.
	TARGMODE_MENU_GUMP  = 256,
	TARGMODE_MENU_SKILL = 512,		// result of some skill. tracking, tinkering, blacksmith, etc.

	// asyc events enum here.
	TARGMODE_DRAG = 1024,		// I'm dragging something. (not quite a targeting but similar)
	TARGMODE_NAME_RUNE,
	TARGMODE_NAME_KEY,			// naming a key.
	TARGMODE_NAME_SIGN,			// name a house sign

	TARGMODE_TEST_BOLT,

	// Targetting cursor.
	// GM command stuff.
	TARGMODE_REMOVE,	// 
	TARGMODE_SET,		// Set some attribute of the item i will show.
	TARGMODE_FLIP,
	TARGMODE_DUPE,
	TARGMODE_SET_Z,
	TARGMODE_ADDITEM,		// items
	TARGMODE_ADDCHAR,		// chars
	TARGMODE_KILL,		
	TARGMODE_KICK,
	TARGMODE_SET_PRIV,
	TARGMODE_BANK,
	TARGMODE_CONTROL,
	TARGMODE_DOOR_LINK,
	TARGMODE_SHRINK,

	// Normal user stuff.
	TARGMODE_SKILL,				// targeting a skill or spell.
	TARGMODE_SKILL_HERD_PICK,
	TARGMODE_USE_ITEM,			// using the selected item
	TARGMODE_PET_CMD,
};

enum SPELL_TYPE	// List of spell numbers
{
	SPELL_NONE = 0,

	SPELL_Clumsy = 1,
	SPELL_Create_Food,
	SPELL_Feeblemind,
	SPELL_Heal,
	SPELL_Magic_Arrow,
	SPELL_Night_Sight,
	SPELL_Reactive_Armor,
	SPELL_Weaken,

	// 2nd
	SPELL_Agility,
	SPELL_Cunning,
	SPELL_Cure,
	SPELL_Harm,
	SPELL_Magic_Trap,
	SPELL_Magic_Untrap,
	SPELL_Protection,
	SPELL_Strength,

	// 3rd
	SPELL_Bless,
	SPELL_Fireball,
	SPELL_Magic_Lock,
	SPELL_Poison,
	SPELL_Telekin,
	SPELL_Teleport,
	SPELL_Unlock,
	SPELL_Wall_of_Stone,

	// 4th
	SPELL_Arch_Cure,
	SPELL_Arch_Prot,
	SPELL_Curse,
	SPELL_Fire_Field,
	SPELL_Great_Heal,
	SPELL_Lightning,
	SPELL_Mana_Drain,
	SPELL_Recall,

		// 5th
	SPELL_Blade_Spirit,
	SPELL_Dispel_Field,
	SPELL_Incognito,
	SPELL_Magic_Reflect,
	SPELL_Mind_Blast,
	SPELL_Paralyze,
	SPELL_Poison_Field,
	SPELL_Summon,

		// 6th
	SPELL_Dispel,
	SPELL_Energy_Bolt,
	SPELL_Explosion,
	SPELL_Invis,
	SPELL_Mark,
	SPELL_Mass_Curse,
	SPELL_Paralyze_Field,
	SPELL_Reveal,

		// 7th
	SPELL_Chain_Lightning,
	SPELL_Energy_Field,
	SPELL_Flame_Strike,
	SPELL_Gate_Travel,
	SPELL_Mana_Vamp,
	SPELL_Mass_Dispel,
	SPELL_Meteor_Swarm,
	SPELL_Polymorph,

		// 8th
	SPELL_Earthquake,
	SPELL_Vortex,
	SPELL_Resurrection,
	SPELL_Air_Elem,
	SPELL_Daemon,
	SPELL_Earth_Elem,
	SPELL_Fire_Elem,
	SPELL_Water_Elem,

	// Necro
	SPELL_Summon_Undead,
	SPELL_Animate_Dead,
	SPELL_Bone_Armor,

	SPELL_BOOK_QTY, 

	SPELL_QTY,
};

enum POTION_TYPE
{
	POTION_WATER,	// No effect.

	POTION_AGILITY,
	POTION_AGILITY_GREAT,
	POTION_CURE_LESS,
	POTION_CURE,
	POTION_CURE_GREAT,
	POTION_EXPLODE_LESS,
	POTION_EXPLODE,
	POTION_EXPLODE_GREAT,
	POTION_HEAL_LESS,
	POTION_HEAL,
	POTION_HEAL_GREAT,
	POTION_NIGHTSIGHT,
	POTION_POISON_LESS,
	POTION_POISON,
	POTION_POISON_GREAT,
	POTION_POISON_DEADLY,
	POTION_REFRESH,
	POTION_REFRESH_TOTAL,
	POTION_STR,
	POTION_STR_GREAT,

	POTION_BEER,	// Beer or ale
	POTION_WINE,	// Wine.

	POTION_QTY,

	// Invis ?
};

struct CMenuItem : public CObListRec	// describe a menu item.
{
	WORD m_id;			// ITEMID_TYPE normally
	CString m_text;
	DWORD m_context;	// more info about what this is.
};

class CClient : public CObListRec, public CSocket
{
	// TCP/IP connection to the player.
public:
	CChar * m_pChar;			// What char are we playing ?
	CString m_sAccount;			// The account name.

#define PRIV_Administrator	0x01 // Can switch in and out of gm mode. assign GM's
#define PRIV_GM			0x02 // GM command clearance
#define PRIV_COUNSEL	0x04 // Counselor command clearance
#define PRIV_GM_PAGE	0x08 // Can listen to GM pages
#define PRIV_BROADCAST  0x10 // Broadcast. Use Yelling on client (! Text) to broadcast.
#define PRIV_ALLMOVE	0x20 // I can move all things. (GM only)
#define PRIV_SHOWUID	0x40 // Get UID numbers on singleclick (Only for debugging)
#define PRIV_DEBUG		0x80 // Show all objects as boxes and chars as humans.
	BYTE m_Priv;				// GM/Counselor privileges for char (bit-mapped)

	// Current operation args for modal async operations..
	TARGMODE_TYPE m_Targ_Mode;	// Type of async operation under way.
	CObjUID m_Targ_UID;			// The object of interest to apply to the target.
	CString m_Targ_Text;		// Text transfered up from client.
	int m_Targ_Index;			// The item or char index, or index of script menu 
	int m_Targ_Val;				// Some value that will apply to the target.
	CObList m_Targ_Menu;		// The current CMenuItem list.

public:
	// Low level data transfer to client.
	int m_bin_pkt;		// the current packet to decode. (estimated length)	
	int m_bin_len;
	CEvent m_bin;		// in buffer. (from client)
	int m_bout_len;
	CCommand m_bout;	// out buffer. (to client)

	// encrypt/decrypt stuff.
	bool m_DeCryptInit;	// Is this the first message from the interface ?
	DWORD m_DeCryptMaskHi;
	DWORD m_DeCryptMaskLo;
	bool m_EnCrypt;			// Encrypt the output.

	// Sync up with movement.
	BYTE m_WalkCount;		// Make sure we are synced up.
	bool m_fPaused;			// We paused the client for big download. (remember to unpause it)
	BYTE m_UpdateStats;	// update our own status (weight change)

public:
	// Low level message traffic.
	void xFlush();				// Sends buffered data at once
	void xSend( const void *pData, int length ); // Buffering send function
	void xSendPkt( CCommand * pCmd, int length )
	{
		xSend( pCmd->m_Raw, length );
		xFlush();
	}

	void xInit_DeCrypt_FindKey( const BYTE * pCryptData, int len );
	void xInit_DeCrypt( const BYTE * pDeCryptID ); // Initialize decryption table
	void xProcess( bool fGood );	// Process a packet
	bool xCheckSize( int len );	// check packet.
	bool xRecvData();			// High Level Receive message from client

public:

	CClient( SOCKET client );
	~CClient();	

	CClient* GetNext() const
	{
		return( static_cast <CClient*>( CObListRec::GetNext()));
	}

	bool LogIn( const char * pName, const char * pPassword );
	void Login_ServerList();		// Initial login (Login on "loginserver", new format)
	void Login_Relay();			// Relay player to a certain IP

	void Setup_List();		// Gameserver login and character listing
	void Setup_Delete();			// Deletion of character
	void Setup_ListFill();
	void Setup_Start();		// Send character startup stuff to player
	void Setup_CreateDialog();	// All the character creation stuff
	void Setup_Play();		// After hitting "Play Character" button

	bool DispatchMsg();

	// Low level push world data to the client.

	void addLoginErr( LOGIN_ERR_TYPE code );
	void addPause( bool fPause = true );
	void addOptions();
	void addObjectRemove( CObjBase * pObj );
	void addObjectLight( CObjBase * pObj );		// Object light level.

	void addItemPut( CItem * pItem ); // Send items (on ground)
	void addItemEquip( CItem * pItem );
	void addItemCont( CItem * pItem );

	void addOpenGump( CObjBase * pCont, WORD gump );
	int  addContents( CContainerItem * pCont, bool fEquip = false, bool fShop = false ); // Send items
	void addContainerSetup( CContainerItem * pCont ); // Send Backpack (with items)

	void addPlayerStart();
	void addPlayerSee( CPoint & p ); // Send objects the player can now see 
	void addPlayerView( CPoint & p );
	void addMoveCancel();
	void addPlayerWarMode();

	void addWeather(); // Send new weather to player
	void addEffect( BYTE motion, ITEMID_TYPE id, const CObjBase * pDst, const CObjBase * pSrc, BYTE speed = 5, BYTE loop = 1, BYTE explode = false );
	void addSound( SOUND_TYPE id, const CObjBase * pBase );
	void addMusic( WORD id );

	void addBark( const char * pText, CObjBase * pSrc, WORD color = 0, TALKMODE_TYPE type = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD );
	void addLight();
	void addSysMessage( const char * pMsg ); // System message (In lower left corner)
	void addItemMessage( const char * pMsg, CObjBase * pSrc, WORD color = 0x03B2 ); // The message when an item is clicked

	void addCharMove( CChar * pChar );
	void addChar( CChar * pChar );
	void addCharName( CChar * pChar ); // Singleclick text for a character
	void addDyeOption( CObjBase * pBase );
	void addItemDragCancel( BYTE type );
	void addWebLaunch( const char *pMsg ); // Direct client to a web page
	void addGumpMenu( int menuid );
	void addItemMenu( CMenuItem * item, int count );
	void addScriptMenu( int menuid, TARGMODE_TYPE offset ); // Menus for item creation
	void addTarget( TARGMODE_TYPE d, const char *pMsg, bool fObject = false ); // Send targetting cursor to client
	void addTargetMulti( CItem * pDeed );
	void addScrollRead( const char * szSec, BYTE, WORD );
	void addMap( CObjUID uid );

	void addVendorClose( CChar * pVendor );
	int  addShopItems( CChar * pVendor, LAYER_TYPE layer );
	bool addShopMenuBuy( CChar * pVendor );
	int  addShopItemsSell( CCommand * pBase, CChar * pVendor, LAYER_TYPE layer );
	bool addShopMenuSell( CChar * pVendor );
	void addBankOpen( CChar * pChar, LAYER_TYPE layer = LAYER_BANKBOX );

	void addReSync();
	void addSpellbookOpen( CItem * pBook );
	void addBookOpen( CItem * pItem );
	void addStatWindow( CObjUID uid ); // Opens the status window
	void addSkillWindow( SKILL_TYPE skill ); // Opens the skills list

	// Test what I can do
	bool IsPriv( BYTE flag ) const
	{
		return( m_Priv & flag );
	}
	bool Can_Snoop_Container( CContainerItem * pItem );
	bool CanSee( const CObjBase * pObj ) const;

	// Handle async events started by a previous command.

	// GM stuff.
	bool OnTarg_Test_Bolt();

	bool OnTarg_Obj_Set( CObjUID uid );
	bool OnTarg_Obj_Flip( CObjUID uid );
	bool OnTarg_Obj_Dupe( CObjUID uid );
	bool OnTarg_Obj_Remove( CObjUID uid );
	bool OnTarg_Obj_Set_Z( CObjUID uid ) ;

	bool OnTarg_Char_Kill( CObjUID uid );
	bool OnTarg_Char_Kick( CObjUID uid );
	bool OnTarg_Char_Set_Priv( CObjUID uid, const char * pszFlags );
	bool OnTarg_Char_Bank( CObjUID uid );
	bool OnTarg_Char_Control( CObjUID uid );
	bool OnTarg_Char_Shrink( CObjUID uid );

	bool OnTarg_Item_Add() ;
	bool OnTarg_Item_Door_Link( CObjUID uid );

	// Normal user stuff.
	bool OnTarg_Use_Key( CItem * pItem, CItem * pKey );
	bool OnTarg_Use_Item();
	
	bool OnTarg_Skill_AnimalLore( CObjUID uid ); 
	bool OnTarg_Skill_Anatomy( CObjUID uid );
	bool OnTarg_Skill_Forensics( CObjUID uid );
	bool OnTarg_Skill_EvalInt( CObjUID uid );
	bool OnTarg_Skill_ArmsLore( CObjUID uid ); 
	bool OnTarg_Skill_ItemID( CObjUID uid );
	bool OnTarg_Skill_Tasting( CObjUID uid );
	bool OnTarg_Skill_Begging( CObjUID uid );

	bool OnTarg_Skill_Tame( CObjUID uid );
	bool OnTarg_Skill_Steal( CObjUID uid );
	bool OnTarg_Skill_Fishing();
	bool OnTarg_Skill_Herd_Pick( CObjUID uid );
	bool OnTarg_Skill_Herd_Move();
	bool OnTarg_Skill_Magery();
	bool OnTarg_Skill();
	bool OnTarg_Pet_Command();

	// Commands from client
	void Event_Target();
	void Event_Attack();
	void Event_Skill_Use( SKILL_TYPE x ); // Skill is clicked on the skill list
	void Event_Tips( int i); // Tip of the day window
	void Event_Book_Read(); // Book window
	void Event_Item_Dye();	// Rehue an item
	void Event_Item_Get(); // Client grabs an item
	void Event_Item_Equip(); // Item is dropped on paperdoll
	void Event_Item_Drop(); // Item is dropped on ground
	void Event_VendorBuy();
	void Event_VendorSell();
	void Event_SecureTrade();
	bool Event_DeathOption();
	void Event_Walking(); // Player moves
	void Event_CombatMode( bool fWar ); // Only for switching to combat mode
	void Event_Choice(); // Choice from GMMenu or Itemmenu received
	void Event_Talk(); // PC speech
	void Event_SingleClick();
	void Event_DoubleClick();
	void Event_SetName();
	void Event_ExtCmd();
	void Event_Command(); // Client entered a '/' command like /ADD

	// translated commands.
	
	void Cmd_CreateItem( int id );
	void Cmd_CreateChar( int id );
	
	void Cmd_GM_Page( const char *reason); // Help button (Calls GM Call Menus up)
	void Cmd_Skill_Magery( SPELL_TYPE iSpell, CObjBase * pSrc );
	void Cmd_Skill_Camping( CItem * pKindling );
	bool Cmd_Skill_Tracking();
	bool Cmd_Skill_Inscription( int ilevel );
	bool Cmd_Skill_Tinker( int ilevel );
	bool Cmd_Skill_Alchemy();
	bool Cmd_Skill_Blacksmith( int iLevel );
	bool Cmd_Skill_Carpentry( int iLevel );
	bool Cmd_Skill_Bowcraft( int iLevel );
	bool Cmd_Skill_Cartography( int iLevel );
	void Cmd_Skill_Heal( CItem * pBandages, CChar * pTarg );
	bool Cmd_Pet( const char * pCmd, CChar * pChar );
	void Cmd_SecureTrading( CChar * pChar, CItem * pItem );

	// Stuff I am doing. Started by a command

	void SetTargMode( TARGMODE_TYPE targmode = TARGMODE_NONE, const char *pPrompt = NULL );

	void Script_MenuCmd( const char * pCmd, const char *pArg ); // Execute script type command
	CPoint Script_GetPlace( int i ); // Decode a teleport location number into X/Y/Z
};

enum ITEM_TYPE		// double click type action.
{
	// NOTE: Never changethis list as it will mess up the GRAYITEMS.SCP file.
	// Just add stuff to end.
	ITEM_NORMAL = 0,
	ITEM_CONTAINER = 1,		// any unlocked container or corpse. CContainer based
	ITEM_CONTAINER_LOCKED,	// 2 = 
	ITEM_DOOR,				// 3 = door can be opened
	ITEM_DOOR_LOCKED,		// 4 = a locked door.
	ITEM_KEY,				// 5 = 
	ITEM_LIGHT_LIT,			// 6 = Local Light giving object
	ITEM_LIGHT_OUT,			// 7 = Can be lit.
	ITEM_FOOD,				// 8 = edible food. (poisoned food ?)
	ITEM_FOOD_RAW,			// 9 = Must cook raw meat unless your an animal.
	ITEM_ARMOR,				// 10 = some type of armor. (no real action)
	ITEM_WEAPON_MACE,		// 11 = Can be used for smithing ?
	ITEM_WEAPON_MACE_SHARP,	// 12 = war axe can be used to cut/chop trees.
	ITEM_WEAPON_SWORD,		// 13 = 
	ITEM_WEAPON_FENCE,		// 14 = can't be used to chop trees. (make kindling)
	ITEM_WEAPON_BOW,		// 15 = bow or xbow
	ITEM_TELEPORT,			// 16 = A ring of teleport type object.
	ITEM_TELEPAD,			// 17 = walk on teleport
	ITEM_SWITCH,			// 18 = this is a switch which effects some other object in the world.
	ITEM_BOOK,				// 19 = read this book.
	ITEM_RUNE,				// 20 = can be marked and renamed as a recall rune.
	ITEM_DRINK,				// 21 = some sort of drink (booze ?)
	ITEM_POTION,			// 22 = Some magic effect.	
	ITEM_FIRE,				// 23 = It will burn you.
	ITEM_CLOCK,				// 24 = or a wristwatch
	ITEM_TRAP,				// 25 = walk on trap.
	ITEM_TRAP_ACTIVE,		// 26 = some animation 
	ITEM_MUSICAL,			// 27 = a musical instrument.
	ITEM_SPELL,				// 28 = magic spell effect.
	ITEM_GEM,
	ITEM_WATER,				// 30 = This is water (fishable)
	ITEM_CLOTHING,			// 31 = All cloth based wearable stuff,
	ITEM_SCROLL,			// 32 = magic scroll
	ITEM_CARPENTRY,			// 33 = tool of some sort.
	ITEM_SPAWN,				// 34 = spawn object. should be invis also.
	ITEM_GAME_PIECE,		// 35 = can't be removed from game.
	ITEM_GATE,				// 36 = Z delta moving gate.
	ITEM_FIGURINE,			// 37 = magic figure that turns into a creature when activated.
	ITEM_QTY,
};

struct CItemBase : public CObListRec
{
	// This is to be used in hand with grayitem.scp file.
	// Describe basic stuff about all items.
	int			m_id;		// The unique id. same as m_disp_id most of the time.
	// ITEMID_TYPE	m_disp_id;	// Base display id	
	ITEM_TYPE	m_type;		// default double click action type.
	WORD		m_buyvalue;		// Base value before magic and extras.
	WORD		m_sellvalue;
	ITEMID_TYPE * m_flip_id; // can be flipped to make this.

	BYTE		m_Can;		// Base attributes.
#define CAN_PILE		0x01	// Can item be piled (*.mul)
#define CAN_DYE			0x02	// Can item be dyed
#define CAN_BLOCK		0x04	// Can item be climbed on ???
#define CAN_FLIP		0x08	// will flip by default.

	// stuff read from .mul file.
	CString m_sName;	// default type name.
	BYTE	m_weight;	// weight (is it movable?) defaults from the .MUL file.
	LAYER_TYPE m_layer;	// LAYER_TYPE if it can be equipped on paperdoll, LAYER_TYPE . defaults from the .MUL file.
	// BYTE	m_height;	// height (can I walk on it ?) (.MUL)

	BYTE   m_attack;		// base attack for weapons or AC for armor. not magic plus

public:
	bool IsSameID( int id ) const;
	void LoadSameID( int id );
	CItemBase( int id, CUOTileItemBlock &item );
};

class CWeaponBase : public CItemBase	// NOT USED YET. or armor
{
	BYTE m_attack;	// base attack val.
	BYTE m_ReqStr;	// Required to equiv.
	BYTE m_MaxQuality;	// Normal max quality
	BYTE m_speed;	// Speed modifier. related to weight ?
	// sound type of weapon. big sword, small sword sounds.
};

class CItem : public CObjBase
{
public:
	static int sm_iCount;

	ITEM_TYPE m_type;		// For things that do special things on doubleclicking 1,8 = container

	// static type attributes.
	CItemBase * m_pDef;
	// LAYER_TYPE m_layer;		// ??? Layer if equipped on paperdoll, LAYER_TYPE . use m_z instead ?
#define m_equip_layer m_p.m_z	// layer equipped odd.

	// Attribute flags. 
#define ATTR_NEWBIE			0x04	// Not lost on death or sellable ?
#define ATTR_MOVE_ALWAYS	0x08	// Always movable (else Default as stored in client) 
#define ATTR_MOVE_NEVER		0x10	// Never movable (else Default as stored in client)
#define ATTR_MAGIC			0x20	// DON'T SET THIS WHILE WORN! This item is magic as apposed to marked or markable.	
#define ATTR_OWNED			0x40	// This is owned by the town. You need to steal it.
#define ATTR_INVIS			0x80	// Gray hidden item (to GM's or owners?)
	BYTE	m_Attr;
	WORD	m_amount;		// Amount of items in pile. 64K max (or corpse type)

	// Type specific info.
	// Magical point info.
	CPoint m_magic_p;	// rune marked to a location or a teleport ?
	// If ATTR_MAGIC ex: "indestructable supremely accurate viking sword of smiting and mages bane with 1 charge named MourneBlade"
#define m_item_spell		m_magic_p.m_x	// The magic spell cast on this. (daemons breath)
#define m_item_magiclevel	m_magic_p.m_y	// ex: indestructable supremely accurate sword of smiting or plate of invulnerability
#define m_item_charges		m_magic_p.m_z	// a wand, torch. candle or oil lamp ?	
#define m_corpse_dir		m_magic_p.m_z	

	union
	{
		DWORD m_more1;		// For various stuff 
		DWORD m_spells1;	// Mask of avail spells for spell book.
		DWORD m_lockID;		// For key, door or contianer.
		DWORD m_bookID;		
		DWORD m_corpse_deathtime;	// For corpse object.
		struct // for weapons and armor.
		{
			WORD m_hitpoints;	// eqiv to quality of the item (armor/weapon).
			WORD m_hitpoints_max;
		};
		WORD m_trap_resetID;	// reset the trap to this.
		WORD m_deed_type;		// deed for what multi ?
		POTION_TYPE m_potion;
		WORD m_spawn_type;		// The class of spawn.
		WORD m_map_type;
		int  m_Gate_DeltaZ;		// The Z delta to change for a gate.
	};
	union
	{
		DWORD m_more2;		// more stuff
		DWORD m_spells2;	// More spells in spell book.
		int m_poisoned;		// Is the food or weapon poisoned ?
		DWORD m_corpse_killer_UID;	// Who killed this corpse.
		WORD m_spawn_max;		// The max qty spawned from here. m_amount = the current count.
		DWORD m_Door_Link;	// UID of linked door or switch.
	};

public:
	CItem( ITEMID_TYPE id );
	virtual ~CItem();
	void OnTick();

	ITEMID_TYPE GetID() const	// maybe get the base id ? as apposed to the flipped id ?
	{
		return( (ITEMID_TYPE) GetPrivateID() );
	}
	bool IsSameID( ITEMID_TYPE id ) const	// account for flipped types ?
	{
		return( m_pDef->IsSameID( id ));
	}
	void SetID( ITEMID_TYPE id );

	bool  IsValid() const;
	void SetContainerFlags( DWORD dwFlags = 0 )
	{
		m_UID.SetContainerFlags( dwFlags );
	}

	static int	IsDoorID( ITEMID_TYPE id );
	static int	IsMusicalID( ITEMID_TYPE id );
	static WORD IsContainerID( ITEMID_TYPE id );
	static bool IsTrapID( ITEMID_TYPE id );
	static bool IsWaterID( ITEMID_TYPE id );

	bool  IsStackable( CItem * pItem ) const;
	bool  IsSameType( CObjBase * pObj ) const;

	virtual int GetWeight() const
	{
		return( m_pDef->m_weight * m_amount );
	}
	int   GetPrice() const;
	void  SetAmount( int amount );
	const char * GetName() const;	// allowed to be default name.
	LAYER_TYPE GetEquipLayer() const
	{
		// Currently equipped in this layer.
		//assert( IsEquipped());
		return( (LAYER_TYPE) m_equip_layer );
	}

	void PutOnGround( CPoint p ) // Put item on the ground here.
	{
		SetContainerFlags(0);
		MoveTo( p );
		Update();
	}

	CItem* GetNext() const
	{
		return( static_cast <CItem*>( CObjBase::GetNext()));
	}
	CObjBase * GetContainer() const;
	bool LoadSetContainer( CObjUID uid, LAYER_TYPE layer );
	CObjUID GetContainerUID() const
	{
		return( GetContainer()->GetUID() );
	}
	CObjBase * GetTopLevelObj( void ) const;

	void  Dupe( CItem * pItem );
	void  Update( CClient * pClientExclude = NULL );		// send this new item to everyone.
	void  Flip( const char * pCmd );

	virtual void  Write( CScript & s ) const;
	bool  LoadVal( const char * pKey, char * pVal );
	void  Load( CScript & s ); // Load an item from script

	void CreateMulti( CChar * pChar );

	// Item type specific stuff.
	bool IsSpellInBook( SPELL_TYPE spell ) const;
	int  AddSpellbookScroll( CItem * pItem );
	bool Use_Door( bool fJustOpen );
	WORD Use_Music( bool fWell );
	const char * Use_Clock();
	SKILL_TYPE GetWeaponSkill() const;
	int Use_Trap();
	void Kill_Spawn_Children();
	bool IsDoorClosed() const
	{
		int itype = IsDoorID( GetID());
		if ( ! itype ) return( true );
		return( ! ((itype-1) & 1 ));
	}
};

class CContainer : public CObList	// This class contains a list of items but may or may not be an item itself.
{
protected:
	int	m_totalweight;	// weight of all the items it has. (1/10 pound)
private:
	void InsertAfter( CObListRec * pNewRec, CObListRec * pPrev = NULL )	// override this virtual function.
	{
		ContentAdd( dynamic_cast <CItem*>( pNewRec ));
	}
	void RemoveAt( CObListRec* pItem )	// override virtual
	{
		ContentRemove( dynamic_cast <CItem*>( pItem ));	// called during destructor ???
	}
public:
	virtual CContainer * GetThisContainer( void )
	{
		return( this );
	}
	CContainer( void )
	{
		m_totalweight = 0;
	}
	int	GetTotalWeight( void ) const 
	{
		return( m_totalweight );
	}
	CItem* GetContentHead() const
	{
		return( static_cast <CItem*>( GetHead()));
	}
	virtual void ContentAdd( CItem * pItem );
	virtual void ContentRemove( CItem * pItem );

	CItem * ContentFind( ITEMID_TYPE id );
	// CItem * ContentFindRandom( void );
	void ContentsDump( CPoint p );

	// For resource usage and gold.
	int ContentCount( ITEMID_TYPE id ) const;
	int ContentConsume( ITEMID_TYPE id, int amount, bool fTest = false );

	void WriteContent( CScript & s ) const;
	virtual void WeightChange( int iChange );

	virtual CObjBase * GetObject()
	{
		return( NULL );	// not based on any object.
	}
};

class CContainerItem : public CItem, public CContainer
{	// 
	// This item has other items inside it. 
	// Multiple inheritance is DANGEROUS !!! cast from CContainer to CContainerItem will fail !

public:
	bool m_fTinkerTrapped;	// magic trap is diff.

public:
	CContainerItem( ITEMID_TYPE id );
	~CContainerItem()
	{
		RemoveSelf();	// Must remove early or else virtuals will fail.
	}
	void  Write( CScript & s ) const
	{
		CItem::Write(s);
		WriteContent(s);
	}
	int GetWeight( void ) const
	{	// true weight == container item + contents.
		return( CItem::GetWeight() + CContainer::GetTotalWeight());
	}
	void WeightChange( int iChange );
	CContainer * GetThisContainer( void )
	{
		return( this );
	}
	void ContentAdd( CItem * pItem );
	void ContentAdd( CItem * pItem, CPoint p );

	void CreateGamePieces();
	void Restock();
	void OnTick();

	// void ContentRemove( CItem * pItem );
	CObjBase * GetObject()
	{
		return( this );
	}
};

enum NPCBRAIN_TYPE	// General AI type.
{
	NPCBRAIN_NONE = 0,	// 0 = Player
	NPCBRAIN_ANIMAL,	// 1
	NPCBRAIN_HUMAN,		// 2 generic good/neutral human.
	NPCBRAIN_HEALER,	// 3
	NPCBRAIN_GUARD,		// 4 inside cities 
	NPCBRAIN_BANKER,	// 5
	NPCBRAIN_VENDOR,	// 6
	NPCBRAIN_BEGGAR,	// 7
	NPCBRAIN_ZOMBIE,	
};

struct CCharBase // define basic info about each monster type.	
{
	// This is to be used in hand with graychar.scp file.
	WORD m_wUsed;			// Can this be removed.
	const char * m_name;	// Base type name 
	CREID_TYPE	m_id;		// display body type.
	COLOR_TYPE  m_color;	// color offset for type.
	ITEMID_TYPE m_trackID;	// ITEMID_TYPE what look like on tracking.

	// sounds ( typically 5 sounds per creature, humans and birds have more.)
	SOUND_TYPE m_sound1;		// just random noise. "yes"
	SOUND_TYPE m_sound2;		// "no" command response 
	SOUND_TYPE m_sound_hit;
	SOUND_TYPE m_sound_gethit;
	SOUND_TYPE m_sound_die;

	//RESOURCES=10 MEAT
	const char * pCorpseResources;

	int m_defense;	// base defense. can be modified by armor.
	int m_attack;	// base attack. can be modified by weapons.
	DWORD m_Anims;	// Bitmask of animations available for monsters. ANIM_TYPE

	//FOOD=MEAT 15 (3)
	//SHELTER=FORESTS (P), MOUNTAINS (P)
	//DESIRES=GOLD (C), OGRES (C)
	//AVERSIONS=TRAPS, CIVILIZATION

	// Carnivourous ?
	// loot ? (should be a script thing)
	// Fly, swim or walk ?
};

class CChar : public CObjBase, public CContainer
{
	// Multiple inheritance is DANGEROUS !!! cast from CContainer to CChar will fail !
public:
	static int sm_iCount;
	// This is a character that can either be NPC or PC.
	const CCharBase * m_pCre;

	// Spell type effects.
#define STATF_INVUL			0x00000001	// Invulnerability 
#define STATF_DEAD			0x00000002
#define STATF_Freeze		0x00000004	// Paralyzed.
#define STATF_Invisible		0x00000008	// Invisible or hidden.
#define STATF_Sleeping		0x00000010	// You look like a corpse ?
#define STATF_War			0x00000020	// War mode on ?

#define STATF_Reactive		0x00000040	// have reactive armor on.
#define STATF_Poisoned		0x00000080	// Poison level ?
#define STATF_NightSight	0x00000100
#define STATF_Reflection	0x00000200	// Magic reflect on.
#define STATF_Polymorph		0x00000400	// We have polymorphed to another form.
#define STATF_Incognito		0x00000800	// Dont show skill titles 
#define STATF_SpiritSpeak	0x00001000

#define STATF_Stone			0x00040000	// turned to stone.
#define STATF_Swim			0x00080000
#define STATF_Fly			0x00100000
#define STATF_Drunk			0x00200000	// Drinking booze.
#define STATF_Halucinating	0x00400000	// eat 'shrooms or bad food.
#define STATF_Hidden		0x00800000	// Hidden (non-magical)
#define STATF_Aggressor		0x01000000	// Was I the aggrresor in the latest conflict ? used for karma tracking 
#define STATF_Criminal		0x02000000	// The guards will attack me.
#define STATF_Conjured		0x04000000	// This creature is conjured and will expire.
#define STATF_OnHorse		0x80000000	// Mounted on horseback.

	// Plus_Luck	// bless luck or curse luck ?
	
	DWORD m_StatFlag;		// Flags above
	CRegion * m_pRegion;	// What region are we in now. (for guarded message)

	// Combat stuff.
	CObjUID m_weapon;		// current Weilded weapon.
	BYTE m_attack;			// weapon (NOT intrinsic attack)
	BYTE m_defense;			// armor worn (NOT intrinsic armor)

	// Skills, Stats and health
	int m_health;	// Hitpoints
	time_t m_regen_health;
	int m_stam;		// Stamina
	time_t m_regen_stam;
	int m_mana;		// Mana
	time_t m_regen_mana;
	int m_food;		// Food level
	time_t m_regen_food;	// rate at which it goes down

	WORD m_Skill[SKILL_QTY];	// List of skills ( skill * 10 )
	short m_Stat[STAT_QTY];		// karma is signed.

	// Some character action in progress.
	SKILL_TYPE m_Act_Skill;		// Currently using a skill. Could be combat skill.
	SPELL_TYPE m_Act_Spell;		// Currently casting spell.
	CObjUID m_Act_Targ;			// Current combat/action target
	CPoint m_Act_p;				// Moving to this location.
	int m_Act_Arg;			// A sub arg of the skill. (summoned type ?)
	int m_Act_Level;			// -1 = fail skill. for skill advance calc

	// other
	DIR_TYPE m_dir;			// facing this dir.
	CREID_TYPE m_prev_id;	// Backup of body type for ghosts and poly
	WORD m_prev_color;		// Backup of skin color
	FONT_TYPE m_fonttype;	// Speech font to use // can client set this ?
	CString m_sTitle;		// Special title such as guard or mayor.
	CRegion * m_pHome;		// What is our "home" region. (towns and bounding of NPC's)

	// NPC stuff.
	NPCBRAIN_TYPE m_NPC_Brain;	// For NPCs: Number of the assigned AI block
	WORD m_NPC_Speech;	// speech script
	CObjUID m_owner;	// If Char is an NPC, this sets its owner. Spawn object ?
	CObjUID m_PrvTarg;		// Previous beg target.

	// Player Stuff
	CString m_sAccount;		// The account index. (for idle players)
	CClient * m_pClient;	// is the char a logged in player ?

private:
	
public:
	CChar( CREID_TYPE id );
	~CChar(); // Delete character

	CChar* GetNext() const
	{
		return( static_cast <CChar*>( CObjBase::GetNext()));
	}
	bool IsValid() const;
	static bool IsFemale( CREID_TYPE id );
	const char * GetPronoun() const;	// he
	const char * GetPossessPronoun() const;	// his
	BYTE GetModeFlag() const
	{
		BYTE mode = 0;
		if ( m_StatFlag & STATF_War )
			mode |= 0x40;
		if ( m_StatFlag & ( STATF_Invisible | STATF_Hidden | STATF_Sleeping ))	// if not me, this will not show up !
			mode |= 0x80;
		return( mode );
	}
	void WeightChange( int iChange );
	BYTE GetNotoFlag() const
	{
		// This allows the noto attack check in the client.
		// 6 = evil(red), 1=good(blue)
		return( ( m_Stat[ STAT_Karma ] < -9000 ) ? 6 : 1 );
	}
	CContainer * GetThisContainer( void )
	{
		return( this );
	}
	CObjBase * GetObject()
	{
		return( this );
	}
	CItem * LayerFind( LAYER_TYPE layer );
	void LayerAdd( CItem * pItem, LAYER_TYPE layer );
	void ContentAdd( CItem * pItem )
	{
		LayerAdd( pItem, pItem->m_pDef->m_layer );
	}
	void ContentRemove( CItem * pItem );

	bool LoadVal( const char * pKey, char * pVal );
	void Load( CScript & s );  // Load a character from Script
	void Write( CScript & s ) const;

	CREID_TYPE GetID() const
	{
		return( m_pCre->m_id );
	}
	void SetID( CREID_TYPE id );

	// Information about us.
	const char * GetName() const;
	void SetName( const char * pName );

	CRegion * GetRegion() const;
	CContainerItem * GetPack( void );

	bool IsPriv( BYTE flag ) const
	{
		if ( m_pClient == NULL ) return( false );
		return( m_pClient->IsPriv( flag ));
	}
	bool CanSee( const CObjBase * pObj ) const;
	bool CanTouch( const CObjBase * pObj ) const;
	bool CanMove( const CItem * pItem ) const;
	bool IsInDungeon() const;
	int  GetLightLevel() const;

	SKILL_TYPE GetBestSkill() const; // Which skill is the highest for character p
	int GetNotoLevel() const;
	const char * GetFameTitle() const;
	const char * GetNotoTitle() const;
	const char * GetTradeTitle() const; // Paperdoll title for character p (2)
	bool Skill_Check( SKILL_TYPE sk, int iLevelReq ) const; 
	SKILL_TYPE GetWeaponSkill() const;
	void SetWeaponSwingTimer( bool fNewWeapon );

	// Push status change to all who can see us.
	void UpdateStats( STAT_TYPE x = STAT_QTY );
	void UpdateAnimate( ANIM_TYPE x, bool fTranslate = true ); // NPC character does a certain Animate
	void UpdateMode(  CClient * pExcludeClient = NULL );
	void UpdateMove( CPoint pold, CClient * pClientExclude = NULL );
	void UpdateDir( CObjBase * pObj );
	void UpdateDrag( CItem * pItem, CObjBase * pCont = NULL, CPoint * pp = NULL );
	void Update( CClient * pClientExclude = NULL );

	// actions.
	void SoundChar( CRESND_TYPE type );
	void SysMessage( const char * pMsg ) const;	// Push system message back to client.

	void Skill_Experience( SKILL_TYPE skill, int difficulty );
	bool Skill_CheckSuccess( SKILL_TYPE skill, int difficulty );
	bool Skill_UseQuick( SKILL_TYPE skill, int difficulty );
	void Skill_Cleanup( void );	 // may have just cancelled targetting.
	void Skill_Fail( void );
	void Skill_Done( void );	 // complete skill (async)
	bool Skill_Start( int iDifficulty ); // calc skill progress.
	bool Skill_Setup( SKILL_TYPE skill );

	void Spell_Dispel();
	void Spell_Summon( int id, CPoint p );
	void Spell_Resurrection();
	void Spell_Polymorph( CREID_TYPE id );
	void Spell_Teleport( CPoint p );
	void Spell_Effect_Remove( CItem * pItem );
	void Spell_Effect_Add( SPELL_TYPE spell, LAYER_TYPE layer, int iLevel, int iDuration );
	void Spell_Effect( SPELL_TYPE spell, CChar * pCharSrc, int iLevel );

	void Spell_Bolt( CObjBase * pObj, ITEMID_TYPE idBolt );
	void Spell_Field( CPoint p, ITEMID_TYPE idEW, ITEMID_TYPE idNS );
	void Spell_Area( CPoint p, int iDist );
	void Spell_Done( void );
	bool Spell_CanCast( SPELL_TYPE spell ) const;
	bool Spell_Cast( CObjUID uid, CPoint p );

	void Create( const CEvent * bin );
	void Flip( const char * pCmd );
	void CreateScript( int id );
	void MakeCorpse();
	void RaiseCorpse( CContainerItem * pCorpse );
	void Death();
	void MakeKill( CChar * pKill );
	bool CheckLocation( bool fStanding = false );
	bool ItemGive( CChar * pCharSrc, CItem * pItem );
	void Hear( const char * pCmd, CChar * pSrc );
	bool Reveal( DWORD dwFlags );
	void CarveCorpse( CContainerItem * pCorpse );
	void Eat( CItem * pItem );
	void Drink( CItem * pItem );
	void Speak( const char * pText, WORD color = 0, TALKMODE_TYPE mode = TALKMODE_SAY );
	bool Attack( CChar * pCharTarg );
	bool Hit( CChar * pCharTarg );
	bool TakeHit( int iDmg, CChar * pSrc );
	bool TakeHitArmor( int iDmg, CChar * pSrc );
	void Use_Item( CItem * pItem, bool fLink = false );
	bool Horse_Mount( CChar * pHorse ); // Remove horse char and give player a horse item
	bool Horse_UnMount(); // Remove horse char and give player a horse item
	void UnEquipAllItems( CContainerItem * pDest = NULL );

	bool NPC_Beg( CChar * pChar );
	void NPC_WalkTo();
	void NPC_Wander();
	bool NPC_Follow( CObjUID targ );
	bool NPC_Cast();
	void NPC_Idle();
	void NPC_Action();

	void OnTick();
};

struct CStartLoc	// The start locations for creating a new char.
{
	CString m_sArea;	// Area/City Name = Britain or Occlo
	CString m_sName;	// Place name = Castle Britannia or Docks
	CPoint m_p;
};

class CRegion : public CObListRec 
{
	// square region of the world.
	// Of arbitrary size and location.
public:
	WORD m_left;
	WORD m_right;
	WORD m_top;
	WORD m_bottom;

	CPoint m_p;			// safe point in the region. (for teleporting to)

	// WORD m_wMusic;	// regional music

	bool m_fGuarded;	// Guarded region or not ?
	int m_iRegionType;	// 0=Area/Continent, 1=City, 2=Building/House, 3=spot or teleport.
	// ??? shelter from weather
	// Teleport in ?
	// anti recall in/out ?

	CString m_sName;	// Name of the region.
};

class CInteractionLog	// Some interaction between characters
{
	// Someone has attacked someone else.
	// or is transacting.
	CChar * m_pChar;	// The char that started this transaction.
	CObjUID m_DstChar;


	// ? damage done to each. for calculating experience from fights.
};

class CQuadrant	// square region of the world.
{
#define QUAD_SIZE_X	256	// ??? 64 ?
#define QUAD_SIZE_Y	256
#define QUAD_COLS	( UO_SIZE_X / QUAD_SIZE_X )
#define QUAD_ROWS	( UO_SIZE_Y / QUAD_SIZE_Y )
#define QUAD_QTY	( QUAD_COLS * QUAD_ROWS )

public:
	// A square region of the world. Dungeon Quadrants are 256 by 256 meters
	BYTE m_weather;				// the weather in this area. 
	// ??? anti magic quadrant = jail ?

public:
	// Search for items and chars in a region must check 4 quandrants around location.
	CObList m_Chars;	// CChar(s) in this CQuadrant.
	CObList m_Items;	// CItem(s) in this CQuadrant.
	CObList m_Regions;	// CRegion(s) in this CQuadrant.

public:
	CQuadrant();
	// bool IsDungeon();
	void OnTick();
	void SetWeather( int w );
	void Close();
};

struct CWorld	// the world.
{
	BYTE m_globallight;			// This is time dependant. (0-19)

	CString m_sBase;		// e:\uo\      
	CFile m_MapFile;		// map0.mul
	CFile m_SIdxFile;		// staidx0.mul
	CFile m_StatFile;		// statics0.mul
	CFile m_TileFile;		// *.mul

	CScript m_scrChars;
	CScript m_scrItems;
	CScript m_scrSpeech;

	// Clock stuff. how long have we been running ? all i care about.
	time_t m_Clock_Start;		// world time of the last restored save. (in seconds)
	time_t m_Clock_Time;		// the current relative tick time  (in seconds)
	time_t m_Clock_PrevTime;	// The previous tick time.

	time_t m_Clock_StartSys;	// System time of the last restored save. (CLOCKS_PER_SEC)
	time_t m_Clock_Save;		// when to auto save ?

	CQuadrant m_Quads[ QUAD_QTY ];

	CObList m_CharsIdle;	// Chars out of play for the moment (dead npcs or logged out players).

private:
	signed char GetMapHeight( CPoint & p, bool & fBlock ) const; // Height of MAP0.MUL at given coordinates

public:
	CWorld();

	void GetMapMeter( CPoint & p, CUOMapMeter & meter ) const; // Height of MAP0.MUL at given coordinates
	bool IsLoading() const
	{
		return( m_Clock_Start == m_Clock_Time );
	}

	bool GetItemData( ITEMID_TYPE id, CUOTileItemBlock * pTile ) const;
	bool GetTerrainData( int id, CUOTileTerrainBlock * pTile ) const;

#define PLAYER_HEIGHT 8		// We need x units of room to walk under something.

	signed char GetItemHeight( ITEMID_TYPE id, bool & fBlock ) const;

	signed char GetHeight( CPoint & p, bool & fBlock ) const; // Height of player who walked to X/Y/OLDZ
	bool CheckValidMove( CPoint & p ) const;

	WORD GetDayTimeMinutes() const;
	void SetTime();
	void SetLight( int light );
	void OnTick();
	void GarbageCollection();

	void Speak( CObjBase * pSrc, const char * pText, WORD color = 0, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD );
	void Broadcast( const char * pMsg );

	const char * GetInformation() const;
	bool CommandLine( int argc, char * argv[] );

	bool Import( const char * pszFilename );
	void Save(); // Save world state
	bool Load();
	void Close();
};

class CWorldSearch	// define a search of the world.
{
private:
	CPoint m_p;			// Base point of our search.
	int m_iDist;		// How far from the point are we interested in
	CObListRec * m_pObj;	// The current object of interest.

	int m_qx;			// current quadrant x
	int m_qy;			// current quadrant y
	CQuadrant * m_pQuad;
private:
	bool GetNextQuadrant();
public:
	CWorldSearch( CPoint p, int iDist = 0 );
	CChar * GetChar();
	CItem * GetItem();
};

struct CServRef		// Array of servers we point to.
{	// NOTE: Server 0 = us !
	CString m_sName;
	struct in_addr m_ip;
	WORD m_port;
	// ??? I should try to poll these if they are up ?
};

class CServer : public CSocket
{
public:

	int  m_error;		// Just some error code to return to system.
	bool m_fKeepRun;
	bool m_fVerbose;	// Level of debug detail.
	bool m_fSecure;     // Secure mode. (will trap exceptions)

	bool m_fBroadcast;  // Broadcast what u type.
	char m_szSpeakText[80];
	int  m_iSpeakIndex;

	time_t m_Clock_Shutdown;	// When to perform the shutdowm (World.clock)

	int m_nClientsMax;		// Maximum (FD_SETSIZE) open connections to server
	int m_nGuestsMax;		// Allow guests who have no accounts ?

	CObList m_Clients;		// Current list of clients
	CClient * m_pAdminClient;	// Remote admin socket from list.

	// login server stuff. 0 = us.
	int m_ServerCount;
	CServRef m_Servers[MAX_SERVERS]; // Servers list. we act like the login server with this.

	// starting points.
	int m_StartCount;
	CStartLoc m_Starts[MAX_STARTS]; // Startpoints list

public:
	CServer();

	void Shutdown( int iSecs );
	bool IniRead();

	bool SocketsInit(); // Initialize sockets
	void SocketsReceive();
	void SocketsFlush();
	void SocketsClose();

#if defined( _CPPUNWIND ) && defined( _DEBUG )
	void Assert( void * pExp, void *pFile, unsigned uLine );
#endif

	void VPrintf( const char * pFormat, va_list pArgs ) const;
	void Printf( const char * pStr ) const;
	void OnConsoleCmd( char key );
	void OnTick();
	void ListClients( CClient * pClient = NULL ) const;

	CClient * GetClientHead() const 
	{
		return( static_cast <CClient*>( m_Clients.GetHead()));
	}
};

// Internal defs.

struct CGMPage : public CObListRec  // ??? TBD	
{
	// Queue a GM page. (based on account)
	CObjUID	m_uidGM;	// assigned to a GM
	CString m_sAccount;	// The account that paged me.
	CString m_sReason;	// Reason for call.
	LONG	m_lTime;	// Time of the last call.
};

struct CPotionDef
{
	const char * m_name;	// Potion real name.
	ITEMID_TYPE m_potion;	// resulting bottle type.
	const char * m_pRegs;	// Regs string similar to spells
	int m_SkillReq;			// Required skill level.
};

struct CSpellDef	// 1 based spells. See SPELL_*
{
	const char * m_name;	// spell name of course
	WORD m_sound;			// What noise does it make when done.
	const char * m_Runes;	// Letter Runes for Words of power.
	const char * m_Reags;	// What reagents does it take ?
	ITEMID_TYPE m_SpellID;		// The rune graphic for this.
	ITEMID_TYPE m_ScrollID;		// The scroll graphic item for this.
#define SPELLFLAG_DIR_ANIM  0x01	// Evoke type cast or directed. (animation)
#define SPELLFLAG_TARG_OBJ  0x02	// Need to target an object or char ?
#define SPELLFLAG_TARG_CHAR 0x04	// Needs to target a living thing
#define SPELLFLAG_TARG_XYZ  0x08	// Can just target a location.
#define SPELLFLAG_HARM		0x10	// The spell is in some way harmfull.
	WORD m_wFlags;
};

struct CSkillDef  // define the skills.	SKILL_*
{
	const char * m_key;		// script key word for skill.	
	const char * m_title;	// title one gets.

	const char * m_targetprompt;	// targetting prompt.
	const char * m_failmsg;

	// Stat dependancies. STAT_STR
	WORD m_wStatStr;
	WORD m_wStatInt;
	WORD m_wStatDex;

	// Advancement rates.
	WORD m_wAdvanceFail;
	WORD m_wAdvanceSuccess;	// Basic key for advancing on success.

	// Delay before skill complete. modified by skill level of course !
	// Player Start items ?
};

//////////////////////////////////////////////////////////////

extern const CSpellDef Spells[SPELL_QTY];	// describe all my spells.
extern const CSkillDef Skills[SKILL_QTY+1];
extern const CPotionDef Potions[];

extern const CWeaponBase Item_Weapons[]; // ???

extern const char * Dirs[DIR_QTY];
extern const char * Stat_Name[STAT_QTY];
extern const char * Runes[];
extern const char * Gray_szDesc;

// items
extern CObList ItemBase;	// CItemBase

// all the UID's in the World. ??? Split this for chars and items ?
extern CObjBase * UIDs[ MAX_OBJECTS ];	// UID Translation table.

// game servers stuff.
extern CWorld   World;	// the world. (we save this stuff)
extern CServer  Serv;	// current state stuff not saved.

extern CLog		Log;	// Log file
extern CObList	GMPage;	// List of GM pages.

///////////////////////////////////////////////

#if 1
#define GetRandVal(x) 	( rand() % (x) )
#else
extern int GetRandVal( int iqty );
#endif
// FIXIT!
extern int GetRangeVal( const char * pArgs );
extern DWORD ahextoi( const char * pArgs ); // Convert hex string to integer

extern char * GetTempStr();
extern int FindTable( const char * pFind, const char * const * ppTable, int iCount, int iElemSize = sizeof(const char *) );
extern int FindID( WORD id, const WORD * pID, int iCount );
extern bool Parse( char * pLine, char ** pLine2 );
extern int ParseCmds( char * pCmdLine, char ** ppCmd, int iMax );
extern bool FindStrWord( const char * pTextSearch, const char * pszKeyWord );

extern CItem * ItemCreateBase( ITEMID_TYPE id );
extern CItem * ItemCreateScript( int id );
extern CItem * ItemCreateDupe( CItem * pItem );
extern CChar * CharCreate( CREID_TYPE id );
extern CChar * CharCreateScript( int id );
