//
// graybase.h
// Copyright Menace Software (www.menasoft.com).
// common header file.
//

#ifndef _INC_GRAYBASE_H
#define _INC_GRAYBASE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef _DEBUG
#ifdef GRAY_SVR
#ifndef ASSERT
extern void Assert_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define ASSERT(exp)			(void)( (exp) || (Assert_CheckFail(#exp, __FILE__, __LINE__), 0) )
#endif	// ASSERT
#ifndef DEBUG_CHECK
extern void Debug_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define DEBUG_CHECK(exp)	(void)( (exp) || (Debug_CheckFail(#exp, __FILE__, __LINE__), 0) )
#endif	// DEBUG_CHECK
#else	// GRAY_SVR
#ifndef ASSERT
#define ASSERT			assert
#endif
#ifndef DEBUG_CHECK
#define DEBUG_CHECK		assert
#endif
#endif	// ! GRAY_SVR
#ifndef STATIC_CAST
#define STATIC_CAST dynamic_cast
#endif
#else	// _DEBUG
#ifndef ASSERT
#ifdef WIN32
#define ASSERT(exp)
#else
extern void Assert_CheckFail( const char * pExp, const char *pFile, unsigned uLine );
#define ASSERT(exp)			(void)( (exp) || (Assert_CheckFail(#exp, __FILE__, __LINE__), 0) )
#endif  // WIN32
#endif	// ASSERT
#ifndef DEBUG_CHECK
#define DEBUG_CHECK(exp)
#endif
#ifndef STATIC_CAST
#define STATIC_CAST static_cast
#endif
#endif	// ! _DEBUG

#include "common.h"

class CObjBase;
class CChar;
class CItem;

class CGrayError
{
	// we can throw this structure to produce an error.
private:
	const TCHAR * m_pszDescription;
	const LOGL_TYPE m_eSeverity;
	const DWORD m_hError;	// HRESULT S_OK, "winerror.h" code. 0x20000000 = start of custom codes.
public:
	CGrayError( LOGL_TYPE eSev, DWORD hErr, const TCHAR *pszDescription ) :
		m_eSeverity( eSev ),
		m_hError( hErr ),
		m_pszDescription( pszDescription )
	{
		DEBUG_ERR(( "CGrayError Exception sev=%d,code=0%x,'%s'\n", eSev, hErr, pszDescription ));
	}
	virtual ~CGrayError() {}

	DWORD GetErrorCode() const { return m_hError; }
	LOGL_TYPE GetSeverity() const { return m_eSeverity; }
	const TCHAR * GetDescription() const { return m_pszDescription; }
	int GetErrorCodeDescription( TCHAR * pszText, int iTextLen ) const;
};

struct CValStr
{
	// Associate a val with a string.
	// Assume sorted values from min to max.
public:
	const TCHAR * m_pszName;
	int m_iVal;
public:
	void SetValues( int iVal, const TCHAR * pszName )
	{
		m_iVal = iVal;
		m_pszName = pszName;
	}
	const TCHAR * FindName( int iVal ) const;
	int GetValue() const
	{
		return( m_iVal );
	}
	void SetValue( int iVal )
	{
		m_iVal = iVal;
	}
};

struct CObjUIDBase		// A unique system serial id. 4 bytes long
{
	// This is a ref to a game object. It may or may not be valid.
	// The top few bits are just flags.
#define UID_CLEAR		0
#define UID_UNUSED		0xFFFFFFFF	// 0 = not used as well.
#define UID_SPEC		0x80000000	// pileable or special macro flag passed to client.
#define UID_ITEM		0x40000000	// CItem as apposed to CChar based
#define UID_EQUIPPED	0x20000000	// This item is equipped.
#define UID_CONTAINED	0x10000000	// This item is inside another container
#define UID_MASK		0x0FFFFFFF	// lose the upper bits.
#define UID_FREE		0x01000000	// Spellbook needs unused UID's ?
private:
	DWORD m_Val;
public:

	bool IsValidUID( void ) const
	{
		return( ( m_Val & UID_MASK ) && ( m_Val & UID_MASK ) != UID_MASK );
	}
	void ClearUID( void )
	{
		m_Val = UID_UNUSED;
	}

	bool IsItem( void ) const	// Item vs. Char
	{
		return(( m_Val & UID_ITEM ) ? true : false );	// check high byte
	}
	bool IsChar( void ) const	// Item vs. Char
	{
		return( (!( m_Val & UID_ITEM )) ? true : false );	// check high byte
	}
	bool IsEquipped( void ) const
	{
		return(( m_Val & UID_EQUIPPED ) ? true : false );
	}
	bool IsInContainer( void ) const
	{
		return(( m_Val & UID_CONTAINED ) ? true : false );
	}
	bool IsDisconnected( void ) const	// Not in the game world for some reason.
	{
		return(( m_Val & UID_SPEC ) ? true : false );
	}
	bool IsTopLevel( void ) const	// on the ground in the world.
	{
		return(( ! ( m_Val & ( UID_SPEC | UID_CONTAINED | UID_EQUIPPED ))) ? true : false );
	}
	void SetContainerFlags( DWORD dwFlags = 0 )
	{
		ASSERT( ! ( dwFlags & (UID_MASK|UID_ITEM)));
		m_Val = ( m_Val & (UID_MASK|UID_ITEM )) | dwFlags;
	}

	DWORD GetIndex() const
	{
		return( m_Val & (UID_MASK|UID_ITEM) );
	}
	bool operator == ( DWORD index )
	{
		return( GetIndex() == index );
	}
	bool operator != ( DWORD index )
	{
		return( GetIndex() != index );
	}
    operator DWORD () const
    {
		return( GetIndex());
    }
	void SetUID( DWORD dwVal )
	{
		// can be set to -1 by the client.
		m_Val = ( dwVal & (UID_MASK|UID_ITEM)) | ( m_Val &~ (UID_MASK|UID_ITEM));
	}
	CObjBase * ObjFind() const;
	CItem * ItemFind() const; // Does item still exist or has it been deleted
	CChar * CharFind() const; // Does character still exist
};

struct CObjUID : public CObjUIDBase
{
	CObjUID()
	{
		ClearUID();
	}
	CObjUID( DWORD dw )
	{
		SetUID( dw );
	}
};

class CRealTime
{
	// Get time stamp in the real world.
public:
	BYTE m_Year;	// years since 1900
	BYTE m_Month;	// months since January - [0,11]
	BYTE m_Day;		// day of the month - [1,31]
	BYTE m_Hour;	// hours since midnight - [0,23]
	BYTE m_Min;		// minutes after the hour - [0,59]
	BYTE m_Sec;		// seconds after the minute - [0,59]
public:
	CRealTime()
	{
		m_Year = 0;
		m_Month = 0;
		m_Day = 0;
		m_Hour = 0;
		m_Min = 0;
		m_Sec = 0;
	}
	bool Read( TCHAR * pVal );
	const TCHAR * Write( void ) const;
	bool IsValid() const
	{
		return( m_Year ? true : false );
	}
	void FillRealTime();	// fill in with real time
	int GetDaysTotal() const
	{
		// Needs to be more consistant than accurate. just for compares.
		return((m_Year * 366) + (m_Month*31) + m_Day );
	}
};

struct CSocketAddress : public in_addr
{
#define SOCKET_LOCAL_ADDRESS 0x0100007f
private:
	WORD m_port;
public:
	DWORD GetAddr() const
	{
		return( s_addr );
	}
	WORD GetPort() const
	{
		return( m_port );
	}
	void SetAddr( DWORD dwIP )
	{
		s_addr = dwIP;
	}
	void SetPort( WORD wPort )
	{
		m_port = wPort;
	}
	void SetPortStr( const TCHAR * pszPort )
	{
		m_port = atoi( pszPort );
	}
	void SetAddrStr( const TCHAR * pszIP )
	{
		s_addr = inet_addr( pszIP );
	}
	void SetPortExtStr( TCHAR * pszIP )
	{
		TCHAR * pszPort = strchr( pszIP, ',' );
		if ( pszPort )
		{
			SetPortStr( pszPort + 1 );
			*pszPort = '\0';
		}
	}
	void SetAddrPortStr( const TCHAR * pszIP )
	{
		TCHAR szIP[256];
		strncpy( szIP, pszIP, sizeof(szIP));
		SetPortExtStr( szIP );
		SetAddrStr( szIP );
	}
	bool SetHostStr( const TCHAR * pszIP )
	{
		// try to resolve the host name with DNS.
		if ( pszIP[0] == '\0' )
			return( false );
		if ( isdigit( pszIP[0] ))
		{
			SetAddrStr( pszIP ); // 0.1.2.3
			return( true );
		}
		struct hostent * pHost = gethostbyname( pszIP );
		if ( pHost == NULL || pHost->h_addr == NULL )	// can't resolve the address.
		{
			return( false );
		}
		SetAddr( *((DWORD*)( pHost->h_addr ))); // 0.1.2.3
		return true;
	}
	bool SetHostPortStr( const TCHAR * pszIP )
	{
		TCHAR szIP[256];
		strncpy( szIP, pszIP, sizeof(szIP));
		SetPortExtStr( szIP );
		return SetHostStr( szIP );
	}
	const TCHAR * GetAddrStr() const
	{
		return inet_ntoa( *this );
	}
	CSocketAddress()
	{
		s_addr = 0;
		m_port = 0;
	}
	bool operator==(CSocketAddress ip ) const
	{
		return( ip.GetAddr() == GetAddr() && ip.GetPort() == GetPort() );
	}
};

class CCryptBase
{
	// The encrypt/decrypt interface.
private:
	bool m_fInit;
	int   m_iClientVersion;		// GRAY_CLIENT_VER 12537

protected:
	DWORD m_MasterHi;
	DWORD m_MasterLo;

	DWORD m_CryptMaskHi;
	DWORD m_CryptMaskLo;

	DWORD m_seed;	// seed we got from the client.

public:
	CCryptBase()
	{
		m_fInit = false;
		m_iClientVersion = 0;	// GRAY_CLIENT_VER
	}
	bool SetClientVersion( int iVer );
	int GetClientVersion() const
	{
		return( m_iClientVersion );
	}
	bool IsInit() const
	{
		return( m_fInit );
	}
	bool IsValid() const
	{
		return( m_iClientVersion >= 0 );
	}
	void Init( const BYTE * pKey );
	void Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
	void Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
	int WriteClientVersion( TCHAR * pStr );
};

enum SERVER_TYPE { SERVER_Login, SERVER_Game, SERVER_Auto };

class CCrypt : public CCryptBase
{
#define CRYPT_AUTO_VALUE	0x80

#define CRYPT_GAMEKEY_LENGTH	6
#define CRYPT_GAMEKEY_COUNT		25

#define CRYPT_GAMESEED_LENGTH	8
#define CRYPT_GAMESEED_COUNT	25

#define CRYPT_GAMETABLE_START	1
#define CRYPT_GAMETABLE_STEP	3
#define CRYPT_GAMETABLE_MODULO	11
#define CRYPT_GAMETABLE_TRIGGER	21036

protected:
	static bool		m_bTablesReady;
private:
	SERVER_TYPE		m_type;
	unsigned char	m_gameSeed[CRYPT_GAMESEED_LENGTH]; // game server needs this.
	unsigned char	m_gameSeed_outgoing[CRYPT_GAMESEED_LENGTH]; // game uses this.
	int				m_gameTable;
	int				m_gameBlockPos;
	int				m_gameStreamPos;

	static const DWORD sm_p_box[];
	static const BYTE  sm_b_box[];
	static const DWORD sm_s_box[];

	static unsigned int p_table[CRYPT_GAMEKEY_COUNT][18];
	static unsigned int s_table[CRYPT_GAMEKEY_COUNT][1024];
	static unsigned char key_table[CRYPT_GAMEKEY_COUNT][6];
	static unsigned char seed_table[2][CRYPT_GAMESEED_COUNT][2][CRYPT_GAMESEED_LENGTH];

private:
	void RawDecrypt(unsigned int *pValues, int table);
	void InitTables();
public:
	void Init( const BYTE *pvSeed, SERVER_TYPE type = SERVER_Auto );
	void Decrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
	void Encrypt( BYTE * pOutput, const BYTE * pInput, int iLen );
};

class CCompressTree
{
	// For compressing/decompressing stuff from game server to client.
public:
	int m_Value; // -1=tree branch, 0-255=character value, 256=PROXY_BRANCH_QTY-1=end
    CCompressTree *m_pZero;
    CCompressTree *m_pOne;
private:
	static const WORD xCompress_Base[256+1];
private:
	bool AddBranch(int Value, WORD wCode, int iBits );
public:
	CCompressTree()
	{
		m_Value=-1;	// just a pass thru branch til later.
		m_pZero=NULL;
		m_pOne=NULL;
	}
	~CCompressTree()
	{
		if ( m_pOne != NULL )
			delete m_pOne;
		if ( m_pZero != NULL )
			delete m_pZero;
	}
	static int Encode( BYTE * pOutput, const BYTE * pInput, int inplen );
	bool Load();
	int  Decode( BYTE * pOutput, const BYTE * pInput, int inpsize ) const;
	bool IsLoaded() const
	{
		return( m_pZero != NULL );
	}
};

#ifdef WM_USER
#define WM_GRAY_CLIENT_COMMAND	(WM_USER+123)	// command the client to do something.
#endif

enum TARGMODE_TYPE	// addTarget for async target event
{
	// Capture the user input for this mode.
	TARGMODE_NONE = 0,		// No targetting going on. we are just walking around etc.

	// Some sort of gump dialog. These match up with TOME.SCP ?
	// NOTE: put these near the start because GumpText only allows one byte to id our source (parent) gump
	TARGMODE_GUMP_PROP1,
	TARGMODE_GUMP_PROP2,
	TARGMODE_GUMP_PROP3,
	TARGMODE_GUMP_PROP4,
	TARGMODE_GUMP_PROP5,
	TARGMODE_GUMP_HAIR_DYE = 6,
	TARGMODE_GUMP_TOME = 8,

	TARGMODE_GUMP_CHAR_PROP_EXTRA = 10,
	TARGMODE_GUMP_HELP_TEST = 11,
	TARGMODE_GUMP_HELP_CMD = 12,

	TARGMODE_GUMP_ADMIN = 26,
	TARGMODE_GUMP_EMAIL	= 27,	// Force the user to enter their emial address. (if they have not yet done so)
	TARGMODE_GUMP_GUILD = 28,	// reserved.

	TARGMODE_GUMP_VAL	= 531,	// special text input dialog

	// Making a selection from a menu. ----------------------------------------------
	TARGMODE_MENU_GM	= 532,		// [GMMENU x] The GM type Menus.
	TARGMODE_MENU_GM_PAGE = 533,

	// ...
	TARGMODE_MENU_ADMIN = 548,		// [ADMIN x] The Admin menu
	TARGMODE_MENU_GUILDSTONE1 = 556,	// [GUILDMENU x] The guildstone menus
	TARGMODE_MENU_GUILDSTONE2,
	TARGMODE_MENU_GUILDSTONE3,
	TARGMODE_MENU_GUILDSTONE4,
	TARGMODE_MENU_GUILDSTONE5,
	TARGMODE_MENU_TOWNSTONE1,		// [TOWNMENU x] The townstone menus
	TARGMODE_MENU_TOWNSTONE2,
	TARGMODE_MENU_TOWNSTONE3,
	TARGMODE_MENU_TOWNSTONE4,

	TARGMODE_MENU_ITEMS = 1570,		// [ITEMMENU x] differs from GM menus. (polymorph menu etc.)
	// ...

	TARGMODE_MENU_SKILL = 5024,		// result of some skill. tracking, tinkering, blacksmith, etc.
	TARGMODE_MENU_SKILL_TRACK_SETUP,
	TARGMODE_MENU_SKILL_TRACK,
	TARGMODE_MENU_SKILL_INSCRIPTION,
	TARGMODE_MENU_GM_PAGES,
	TARGMODE_MENU_EDIT,

	// setup events ------------------------------------------------
	TARGMODE_SETUP_NOCONNECT = 0x4000,	// There is no connection.
	TARGMODE_SETUP_CONNECT,	// client has just connected. waiting for first message.
	TARGMODE_SETUP_SERVERS,		// client has received the servers list.
	TARGMODE_SETUP_SERV_SELECT,	// client has seleted a server.
	TARGMODE_SETUP_RELAY,		// client has been relayed to the game server. wait for new login.
	TARGMODE_SETUP_GAME_LOGIN,	// client is logging into the game server.
	TARGMODE_SETUP_CHARLIST,	// client has the char list and may (play a char, delete a char, create a new char)
	TARGMODE_SETUP_CHARPLAY,	// client has requested to play a char.

	TARGMODE_CONSOLE_LOGIN,		// I got a space. so we are looking for the password.
	TARGMODE_CONSOLE,			// we are in console mode.
	TARGMODE_PEER_SERVER_REQUEST,	// only peer servers can talk to us like this.

	// asyc events enum here. --------------------------------------------------------
	TARGMODE_DRAG = 0x6000,		// I'm dragging something. (not quite a targeting but similar)
	TARGMODE_DRAG_QTY,
	TARGMODE_DEATH,				// The death menu is up.
	TARGMODE_DYE,				// The dye dialog is up.

	TARGMODE_PROMPT,			// Some sort of text prompt input.
	TARGMODE_NAME_RUNE,
	TARGMODE_NAME_KEY,			// naming a key.
	TARGMODE_NAME_SIGN,			// name a house sign
	TARGMODE_NAME_SHIP,
	TARGMODE_GM_PAGE_TEXT,		// allowed to enter text for page.
	TARGMODE_VENDOR_PRICE,		// What would you like the price to be ?
	TARGMODE_ENTER_TARG_VERB,		// Send a msg to another player.

	// Targetting cursor. -------------------------------------------------------------
	TARGMODE_MOUSE_TYPE,	// Greater than this = mouse type targetting.
	TARGMODE_MOUSE_GROUND,	// ground select.
	TARGMODE_MOUSE_OBJ,		// must select an object.
	TARGMODE_MOUSE_DEAD,

	// GM command stuff.
	TARGMODE_OBJ_SET,		// Set some attribute of the item i will show.
	TARGMODE_OBJ_PROPS,
	TARGMODE_OBJ_FLIP,

	TARGMODE_CHAR_PRIVSET,
	TARGMODE_CHAR_BANK,
	TARGMODE_CHAR_CONTROL,

	TARGMODE_ADDMULTIITEM,		// Break out Multi items
	TARGMODE_ADDITEM,		// items
	TARGMODE_LINK,
	TARGMODE_TILE,

	// Normal user stuff. (mouse targetting)
	TARGMODE_SKILL,				// targeting a skill or spell.
	TARGMODE_SKILL_MAGERY,
	TARGMODE_SKILL_HERD_DEST,
	TARGMODE_SKILL_POISON,
	TARGMODE_SKILL_PROVOKE,

	TARGMODE_USE_ITEM,			// target for using the selected item
	TARGMODE_PET_CMD,			// targetted pet command
	TARGMODE_PET_STABLE,		// Pick a creature to stable.
	TARGMODE_REPAIR,		// attempt to repair an item.

	TARGMODE_STONE_NAME,		// prompt for text.
	TARGMODE_STONE_SET_ABBREV,
	TARGMODE_STONE_SET_TITLE,
	TARGMODE_STONE_GRANT_TITLE,
	TARGMODE_STONE_RECRUIT,		// Recruit members for a stone	(mouse select)
};

#endif	// _INC_GRAYBASE_H
