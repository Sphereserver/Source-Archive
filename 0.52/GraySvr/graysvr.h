//
// GraySvr.H
// Copyright Menace Software (www.menasoft.com).
// Precompiled header
//

#ifndef _GRAYSVR_H_
#define _GRAYSVR_H_

#ifndef GRAY_SVR
#error GRAY_SVR must be -defined on compiler command line for common code to work!
#endif

#ifdef _DEBUG
#define DEBUG_VALIDATE_ALLOC
extern size_t DEBUG_ValidateAlloc( const void * pThis );
#endif

#ifdef _WIN32
// NOTE: If we want a max number of sockets we must compile for it !
#undef FD_SETSIZE		// This shuts off a warning
#define FD_SETSIZE 1024 // for max of n users ! default = 64
#endif

#include "../common/graycom.h"	// put slashes this way for LINUX, WIN32 does not care.
#include "../common/graymul.h"
#include "../common/grayproto.h"
#include "../common/cgrayinst.h"
#include "../common/cregion.h"
#include "../common/cgraymap.h"

#define GRAY_TITLE			"Sphere"	// "Sphere"
#define GRAY_VERSION		"0.52"
#define GRAY_MAIN_SERVER	"menace.ne.mediaone.net"
#define GRAY_GAME_SERVER
#define GRAY_LOG_SERVER
#define GRAY_URL			"www.sphereserver.com"	// default url.

class CCharBase;
class CClient;
class CAccount;
class CItemContainer;
class CItemMessage;
class CItemMap;

///////////////////////////////////////////////

// Stuff for the NT Service
#ifdef _WIN32
#include <eh.h> //	exception handling info.

class CNTService
{
private:
	bool m_fIsNTService;
	bool m_fDebugService;

	HANDLE m_hServerStopEvent;
	SERVICE_STATUS_HANDLE m_hStatusHandle;
	SERVICE_STATUS m_sStatus;

private:
	int  ServiceStart(DWORD, LPTSTR *);
	void ServiceStop();
	void ServiceStartMain(DWORD dwArgc, LPTSTR *lpszArgv);

	void AddToMessageLog(LPTSTR, DWORD, WORD);

	// Our exported API.
	static BOOL WINAPI ControlHandler(DWORD dwCtrlType);
	static void WINAPI service_ctrl(DWORD dwCtrlCode);
	static VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);

public:
	CNTService()
	{
		m_fDebugService = false;
		m_fIsNTService = false;
	}

	// command line entries.
	void CmdInstallService();
	void CmdRemoveService();
	int  CmdDebugService(int, TCHAR **);
	void CmdMainStart();

	bool IsServiceStopPending() const
	{
		return( m_fIsNTService && m_sStatus.dwCurrentState == SERVICE_STOP_PENDING );
	}

	BOOL ReportStatusToSCMgr(DWORD, DWORD, DWORD);
	// int InstallService(int argc, TCHAR * argv[]);
};

extern CNTService g_Service;
extern int _cdecl SphereMainEntryPoint( int argc, TCHAR *argv[] );

extern OSVERSIONINFO g_osInfo;
extern const OSVERSIONINFO * GRAY_GetOSInfo();

#endif

// Text mashers.

extern ITEMID_TYPE GetResourceItem( TCHAR * & pRes );
extern ITEMID_TYPE GetResourceEntry( TCHAR * & pRes, int & iQty );
extern int FindID( WORD id, const WORD * pID, int iCount );

extern DIR_TYPE GetDirStr( const TCHAR * pszDir );
extern const TCHAR * GetTimeMinDesc( int dwMinutes );

extern int FindStrWord( const TCHAR * pTextSearch, const TCHAR * pszKeyWord );

extern struct CLog : public CFileText, public CEventLog
{
	// subject matter. (severity level is first 4 bits, LOGL_EVENT)
#define LOGM_ACCOUNTS		0x0080
#define LOGM_INIT			0x0100	// start up messages.
#define LOGM_SAVE			0x0200	// world save status.
#define LOGM_CLIENTS_LOG	0x0400	// all clients as they log in and out.
#define LOGM_GM_PAGE		0x0800	// player gm pages.
#define LOGM_PLAYER_SPEAK	0x1000	// All that the players say.
#define LOGM_GM_CMDS		0x2000	// Log all GM commands.
#define LOGM_CHEAT			0x4000	// Probably an exploit !
#define LOGM_KILLS			0x8000	// Log player combat results.

private:
	WORD m_wMsgMask;			// Level of log detail messages. IsLogMsg()
	CRealTime m_Stamp;			// last real time stamp.
	CGString m_sBaseDir;
	const CScript * m_pScriptContext;	// The current context.

public:
	const CScript * SetScriptContext( const CScript * pScriptContext )
	{
		const CScript * pOldScript = m_pScriptContext;
		m_pScriptContext = pScriptContext;
		return( pOldScript );
	}
	bool SetFilePath( const TCHAR * pszName )
	{
		ASSERT( ! IsFileOpen());
		return CFileText::SetFilePath( pszName );
	}
	CLog()
	{
		m_pScriptContext = NULL;
		m_wMsgMask = LOGL_EVENT |
			LOGM_INIT | LOGM_CLIENTS_LOG | LOGM_GM_PAGE;
		SetFilePath( GRAY_FILE "log.log" );	// default name to go to.
	}

	bool Open( TCHAR * pszName = NULL );	// name set previously.
	WORD GetLogMask() const
	{
		return( m_wMsgMask &~ 0x0f ) ;
	}
	LOGL_TYPE GetLogLevel() const
	{
		return((LOGL_TYPE)( m_wMsgMask & 0x0f ));
	}
	void SetLogMask( WORD wMask )
	{
		m_wMsgMask = GetLogLevel() | wMask;
	}
	void SetLogLevel( LOGL_TYPE level )
	{
		m_wMsgMask = GetLogMask() | level;
	}
	bool IsLoggedMask( WORD wMask ) const
	{
		return(( GetLogMask() & ( wMask &~ 0x0f )) ? true : false );
	}
	bool IsLogged( WORD wMask ) const
	{
		return( IsLoggedMask(wMask) ||
			( GetLogLevel() >= ( wMask & 0x0f )));
	}

	void Dump( const BYTE * pData, int len );

	int EventStr( WORD wMask, const TCHAR * pszMsg );

} g_Log;		// Log file

//////////////////

enum SCPFILE_TYPE	// my required data files.
{
	SCPFILE_INVALID = -1,
	SCPFILE_CHAR = 0,
	SCPFILE_CHAR_2,
	SCPFILE_ITEM,
	SCPFILE_ITEM_2,
	SCPFILE_TRIG,
	SCPFILE_TRIG_2,
	SCPFILE_BOOK,
	SCPFILE_BOOK_2,
	SCPFILE_MENU,
	SCPFILE_MENU_2,
	SCPFILE_GUMP,
	SCPFILE_GUMP_2,
	SCPFILE_SKILL,
	SCPFILE_SKILL_2,
	SCPFILE_TEMPLATE,
	SCPFILE_TEMPLATE_2,
	SCPFILE_NAME,
	SCPFILE_NAME_2,
	SCPFILE_SPEE,
	SCPFILE_SPEE_2,
	SCPFILE_QTY,
};

class CBaseStub : public CMemDynamic
{
private:
	const WORD m_wBaseIndex;	// The unique index id.	(WILL not be the same as artwork if outside artwork range)
public:
	CBaseStub( WORD wIndex ) :
		m_wBaseIndex( wIndex )
	{
	}
	WORD GetBaseID() const { return( m_wBaseIndex ); }
	virtual void UnLoad() = 0;	// must have at least 1 virtual for dynamic_cast to work
};

struct CBaseBaseArray : public CGObSortArray< CBaseStub*, WORD >
{
	// Sorted array
	int CompareKey( WORD id, CBaseStub * pBase ) const
	{
		ASSERT( pBase );
		return( id - pBase->GetBaseID());
	}
};

struct CBaseBase : public CScriptObj, public CBaseStub
{
	static const TCHAR * sm_KeyTable[];
	// Base type of both CItemBase and CCharBase
protected:
	WORD	m_wDispIndex;	// The artwork id. (may be the same as m_wBaseIndex)
	DWORD	m_lInstances;	// How many instances of this type exist ?
	CScriptLink	m_ScriptLink;	// Offset into GRAYITEM or GRAYCHAR.SCP file.
	CGString m_sName;		// default type name.
private:
	BYTE	m_Height;
public:
	WORD	m_Can;			// Base attribute flags.
	BYTE    m_attackBase;	// base attack for weapons or AC for armor. not magic plus
	BYTE	m_attackRange;	// variable range of attack damage.

// Map Movement flags.
#define CAN_C_GHOST			0x0001	// Moves thru doors etc.
#define CAN_C_SWIM			0x0002	// dolphin, elemental or is water
#define CAN_C_WALK			0x0004	// Can walk on land, climbed on walked over else Frozen by nature(Corpser) or can just swim
#define CAN_C_PASSWALLS		0x0008	// Walk thru walls.
#define CAN_C_FLY			0x0010	// Mongbat etc.
#define CAN_C_FIRE_IMMUNE	0x0020	// Has some immunity to fire ? (will walk into it (lava))
#define CAN_C_INDOORS		0x0040	// Can go under roof. Not really used except to mask.
#define CAN_C_MOVE_ITEMS	0x0080	// We can move items.

#define CAN_I_DOOR			0x0001	// Is a door
#define CAN_I_WATER			0x0002	// Need to swim in it.
#define CAN_I_CLIMB			0x0004	// step up on it,
#define CAN_I_BLOCK			0x0008	// need to walk thru walls or fly over.
#define CAN_I_PLATFORM		0x0010	// we can walk on top of it.
#define CAN_I_FIRE			0x0020	// Is a fire. Ussually blocks as well.
#define CAN_I_ROOF			0x0040	// We are under a roof. can't rain on us.
#define CAN_I_MOVABLE		0x0080	// We are being blocked be a movable item

	// CItemBase specific defs.
#define CAN_I_PILE			0x0100	// Can item be piled UFLAG2_STACKABLE (*.mul)
#define CAN_I_DYE			0x0200	// Can item be dyed UFLAG3_CLOTH
#define CAN_I_FLIP			0x0400	// will flip by default.
#define CAN_I_LIGHT			0x0800	// UFLAG3_LIGHT
#define CAN_I_REPAIR		0x1000	// Is it repairable (difficulty based on value)

	// CCharBase specific defs.
#define CAN_C_EQUIP			0x0100	// Can equip stuff. (humanoid)
#define CAN_C_USEHANDS		0x0200	// Can wield weapons (INT dependant), open doors ?, pick up/manipulate things
#define CAN_C_EATRAW		0x0400	// Carnivourous ?
#define CAN_C_FEMALE		0x0800	// It is female by nature.
#define CAN_C_NONHUMANOID	0x1000	// Body type for combat messages.
#define CAN_C_RUN			0x2000	// Can run (not needed if they can fly)
//#define CAN_C_NOCORPSE		0x2000	// Will not leave a corpse.

#define CAN_TRIGGER			0x8000	// This item has OnTrigger stuff in the GRAYITEM.SCP or GRAYCHAR.SCP

private:
	CScriptLink * FindScpPrevLoad( CScript * pScript ) const;
protected:
	virtual SCPFILE_TYPE GetScpFileIndex( bool fSecondary ) const = 0;
	virtual CBaseBaseArray * GetBaseArray() const = 0;
public:
	CBaseBase( WORD wIndex ) :
		CBaseStub( wIndex )
	{
		m_wDispIndex = wIndex;	// Assume the same til told differently.
		m_lInstances = 0;
		m_attackBase = 0;
		m_attackRange = 0;
		m_Can = CAN_C_INDOORS;	// most things can cover us from the weather.
	}
	~CBaseBase()
	{
		DEBUG_CHECK( GetInstances() == 0 );
	}
	const TCHAR * GetTypeName() const
	{
		return( m_sName );
	}
	const TCHAR * GetName() const
	{
		return( GetTypeName());
	}
	bool HasTypeName() const	// some CItem may not.
	{
		return( ! m_sName.IsEmpty());	// default type name.
	}
	void SetTypeName( const TCHAR * pszName )
	{
		GETNONWHITESPACE( pszName );
		m_sName = pszName;
	}
	bool Can( WORD wCan ) const
	{
		return(( m_Can & wCan ) ? true : false );
	}
	void AddInstance() { m_lInstances ++; }
	void DelInstance() { m_lInstances --; }
	int GetInstances() const { return( m_lInstances ); }

	virtual void UnLoad() 
	{ 
		m_ScriptLink.InitLink(); 
	}
	bool IsLoaded() const	// been loaded from the scripts2 ?
	{
		return( m_ScriptLink.IsLinked());
	}

	virtual bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc = NULL );
	virtual bool r_LoadVal( CScript & s );

	bool IsValid() const
	{
#ifdef _DEBUG
		IsValidDynamic();
#endif
		return( m_sName.IsValid());
	}

	BYTE GetHeight() const
	{
		return( m_Height );
	}
	void SetHeight( BYTE Height )
	{
		m_Height = Height;
	}

	void DupeCopy( const CBaseBase * pSrc );

	bool ScriptLockBase( CScriptLock & s );
};

#define DAMAGE_GOD		0x001	// Nothing can block this.
#define DAMAGE_HIT		0x002	// Physical hit of some sort.
#define DAMAGE_MAGIC	0x004	// Magic blast of some sort. (we can be immune to magic to some extent)
#define DAMAGE_POISON	0x008	// Or biological of some sort ? (HARM spell)
#define DAMAGE_FIRE		0x010	// Fire damage of course.  (Some creatures are immune to fire)
#define DAMAGE_ELECTRIC 0x020	// lightning.
#define DAMAGE_DRAIN	0x040	// level drain = negative energy.
#define DAMAGE_GENERAL	0x080	// All over damage. As apposed to hitting just one point.
#define DAMAGE_ACID		0x100	// Corrosive will destroy armor.
typedef WORD DAMAGE_TYPE;		// describe a type of damage.

enum MEMORY_TYPE
{
	// ITEM_EQ_MEMORY_OBJ
	// Types of memory a CChar has about a game object. (m_color)
	MEMORY_NONE			= 0,
	MEMORY_SAWCRIME		= 0x0001,	// I saw them commit a crime or i was attacked criminally. I can call the guards on them.
	MEMORY_IPET			= 0x0002,	// I am a pet. (this link is my master) (never time out)
	MEMORY_FIGHT		= 0x0004,	// Active fight going on now. may not have done any damage.
	MEMORY_IAGGRESSOR	= 0x0008,	// I was the agressor here. (good or evil)
	MEMORY_HARMEDBY		= 0x0010,	// I was attacked by them. (but they may have been retaliating)
	MEMORY_IRRITATEDBY	= 0x0020,	// I saw them snoop from me or someone.
	MEMORY_SPEAK		= 0x0040,	// We spoke about something at some point. (or was tamed) (NPC_MEM_ACT_TYPE)
	MEMORY_AGGREIVED	= 0x0080,	// ???

	MEMORY_GUARD		= 0x0100,	// Guard this item (never time out)
	MEMORY_ISPAWNED		= 0x0200,	// I am spawned from this item. (never time out)
	MEMORY_GUILD		= 0x0400,	// This is my guild stone. (never time out)
	MEMORY_TOWN			= 0x0800,	// This is my town stone. (never time out)
	MEMORY_FOLLOW		= 0x1000,	// I am following this Object (never time out)

	OLDMEMORY_LOOTITEM		= 0x2000,	// I've looted this corpse or just looked at this item in general.
	OLDMEMORY_MURDERDECAY	= 0x4000,	// Next time a murder count gets dropped
};

enum NPC_MEM_ACT_TYPE	// A simgle primary memory about the object.
{
	// related to MEMORY_SPEAK
	NPC_MEM_ACT_NONE = 0,		// we spoke about something non-specific,
	NPC_MEM_ACT_SPEAK_TRAIN,	// I am speaking about training. Waiting for money
	NPC_MEM_ACT_SPEAK_HIRE,		// I am speaking about being hired. Waiting for money
	NPC_MEM_ACT_FIRSTSPEAK,		// I attempted (or could have) to speak to player. but have had no response.
	NPC_MEM_ACT_TAMED,			// I was tamed by this person previously.
	NPC_MEM_ACT_IGNORE,			// I looted or looked at and discarded this item (ignore it)
};

class CObjBase : public CObjBaseTemplate, public CScriptObj
{
	// All items or chars have these base attributes.
private:
	time_t	m_timeout;		// when does this rot away ? or other action. 0 = never, else system time
	COLOR_TYPE m_color;		// Hue or skin color. (CItems must be < 0x4ff or so)

public:
	static int sm_iCount;
	static const TCHAR * sm_KeyTable[];

protected:
	virtual void DeletePrepare();

	void DupeCopy( const CObjBase * pObj )
	{
		CObjBaseTemplate::DupeCopy( pObj );
		m_color  = pObj->GetColor();
		// m_timeout = pObj->m_timeout;
	}

public:
	CObjBase( bool fItem );
	virtual ~CObjBase();
	virtual int IsWeird() const;
	void Delete();
	virtual bool OnTick() = 0;
	virtual int FixWeirdness() = 0;

	// Accessors

	virtual WORD GetBaseID() const = 0;
	virtual CBaseBase * GetBase() const	= 0;

	void SetPrivateUID( DWORD dwVal, bool fItem );
	CObjBase* GetNext() const
	{
		return( STATIC_CAST <CObjBase*>( CGObListRec::GetNext()));
	}
	virtual const TCHAR * GetName() const	// resolve ambiguity w/CScriptObj
	{
		return( CObjBaseTemplate::GetName());
	}

public:
	// Color
	void SetColor( COLOR_TYPE color )
	{
		m_color = color;
	}
	COLOR_TYPE GetColor() const
	{
		return( m_color );
	}

protected:
	WORD GetColorAlt() const
	{
		// DEBUG_CHECK( IsEquipped());
		// ITEM_EQ_MEMORY_OBJ = MEMORY_TYPE mask
		// ITEM_EQ_NPC_SCRIPT = previous book script time.
		// ITEM_EQ_VENDOR_BOX = restock time.
		return( m_color );
	}
	void SetColorAlt( COLOR_TYPE color )
	{
		m_color = color;
	}

public:
	// Timer
#define TICK_PER_SEC 10
	virtual void SetTimeout( int iDelayInTicks );
	bool IsTimerSet() const
	{
		return( m_timeout != 0 );
	}
	int GetTimerDiff() const;	// return: < 0 = in the past ( m_timeout - g_World.GetTime() )
	bool IsTimerExpired() const
	{
		return( GetTimerDiff() <= 0 );
	}
	int GetTimerAdjusted() const
	{
		// RETURN: time in seconds from now.
		if ( ! IsTimerSet()) 
			return( -1 );
		int iDiffInTicks = GetTimerDiff();
		if ( iDiffInTicks < 0 ) 
			return( 0 );
		return( iDiffInTicks / TICK_PER_SEC );
	}

	DWORD GetPlayerVendorPrice() const
	{
		// The timer is not really a timer.
		// override the vendor price of an item (in container).
		// DEBUG_CHECK( g_World.IsLoading() || IsInContainer() );
		return( m_timeout );
	}
	void SetPlayerVendorPrice( DWORD dwVal )
	{
		// This can only be inside a vendor container.
		// DEBUG_CHECK( g_World.IsLoading() || IsInContainer() );
		m_timeout= dwVal;
	}

public:
	// Location
	virtual bool MoveTo( CPointMap pt ) = 0;	// Move to a location at top level.

	virtual void MoveNear( CPointMap pt, int iSteps = 0, WORD wCan = CAN_C_WALK );
	virtual void MoveNearObj( const CObjBaseTemplate * pObj, int iSteps = 0, WORD wCan = CAN_C_WALK )
	{
		ASSERT( pObj );
		pObj = pObj->GetTopLevelObj();
		MoveNear( pObj->GetTopPoint(), iSteps, wCan );
	}

	bool SetNamePool( const TCHAR * pszName );

	void Sound( SOUND_TYPE id, int iRepeat = 1 ) const; // Play sound effect from this location.
	void Effect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBase * pSource = NULL, BYTE speed = 5, BYTE loop = 1, bool explode = false ) const;

	void WriteTry( CScript & s );

	virtual bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc = NULL );
	virtual void r_Write( CScript & s );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );	// some command on this object as a target

	void Speak( const TCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_NORMAL );
	void SpeakUTF8( const TCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_NORMAL, const TCHAR * pszLanguage = NULL );
	void SpeakUNICODE( const NCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_NORMAL, const TCHAR * pszLanguage = NULL );

	void RemoveFromView( CClient * pClientExclude = NULL );	// remove this item from all clients.
	void UpdateCanSee( const CCommand * pCmd, int iLen, CClient * pClientExclude = NULL ) const;
	bool IsContainer( void ) const;

	virtual void Update( const CClient * pClientExclude = NULL )	// send this new item to clients.
		= 0;
	virtual void Flip( const TCHAR * pCmd = NULL )
		= 0;
	virtual int OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType = DAMAGE_HIT )
		= 0;
	virtual bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel )
		= 0;
};

enum POTION_TYPE
{
	POTION_WATER = 0,	// No effect.

	POTION_AGILITY,
	POTION_AGILITY_GREAT,
	POTION_CURE_LESS,
	POTION_CURE,
	POTION_CURE_GREAT,
	POTION_EXPLODE_LESS,	// 6 = purple
	POTION_EXPLODE,
	POTION_EXPLODE_GREAT,
	POTION_HEAL_LESS,		// 9 = yellow
	POTION_HEAL,			// 10
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

	// Add non standard to the end.
	POTION_FLOAT_LESS,	// 21
	POTION_FLOAT,
	POTION_FLOAT_GREAT,
	POTION_SHRINK,		// 0x18 = 24
	POTION_INVISIBLE,
	POTION_MANA,
	POTION_MANA_TOTAL,
	POTION_LAVA,

	POTION_QTY,		// 29

	// Invis ?
};

class CAccount : public CMemDynamic, public CScriptObj
{
	static const TCHAR * sm_KeyTable[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	PLEVEL_TYPE m_PrivLevel;
	CGString m_sName;			// Name = no spaces. case independant.
	CGString m_sPassword;

#define PRIVOLD_ADMIN		0x0001
#define PRIVOLD_COUNS		0x0004
#define PRIVOLD_GM_TOGGLE	0x0400	// I can turn GM on or off.

#define PRIV_GM				0x0002	// Acts as a GM (dif from having GM level)
#define PRIV_GM_PAGE		0x0008	// Listen to GM pages or not.
#define PRIV_HEARALL		0x0010	// I can hear everything said.
#define PRIV_ALLMOVE		0x0020	// I can move all things. (GM only)
#define PRIV_DETAIL			0x0040	// Show combat detail messages
#define PRIV_DEBUG			0x0080	// Show all objects as boxes and chars as humans.
#define PRIV_EMAIL_VALID	0x0100	// The email address has been validated.
#define PRIV_PRIV_NOSHOW	0x0200	// Show the GM title and Invul flags.
#define PRIV_JAILED			0x0800	// Must be /PARDONed from jail.
#define PRIV_T2A			0x1000	// They have a t2a client.exe (30=dark,new creatures,etc)
#define PRIV_BLOCKED		0x2000	// The account is blocked.
#define PRIV_ALLSHOW		0x4000	// Show even the offline chars.

public:
	WORD m_PrivFlags;			// optional privileges for char (bit-mapped)

	char m_lang[4];				// UNICODE language pref. (ENU=english)
	CGString m_sChatName;		// Chat System Name

	time_t m_Total_Connect_Time;		// Previous total amount of time in game. (minutes) "TOTALCONNECTTIME"

	struct in_addr m_Last_IP;	// last ip i logged in from.
	CRealTime m_Last_Connect_Date;			// The last date i logged in. (use localtime())
	time_t  m_Last_Connect_Time;			// Amount of time spent online last time.

	struct in_addr m_First_IP;		// first ip i logged in from.
	CRealTime m_First_Connect_Date;			// The first date i logged in. (use localtime())

	CObjUID m_uidLastChar;		// Last char i logged in with.
	// CObjUID m_uidChar[ MAX_CHARS_PER_ACCT ];	// all the chars belonging to this account.

	CGTypedArray<WORD,WORD> m_EMailSchedule;
	BYTE m_iEmailFailures;
	CGString m_sEMail;			// My auto event notifier.
	CGString m_sComment;			// Some GM comments about me.

private:
	bool SetEmailAddress( const TCHAR * pszEmail, bool fBlockHost );
	bool ScheduleEmailMessage( WORD iEmailMessage );
	static bool CheckBlockedEmail( const TCHAR * pszEmail);
	bool SendOutgoingMail( int iMsg );

public:
	CAccount( const TCHAR * pszName, bool fGuest = false );
	~CAccount();

	static int NameStrip( TCHAR * pszNameOut, const TCHAR * pszNameInp );
	static PLEVEL_TYPE GetPrivLevelText( const TCHAR * pszFlags );

	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc );
	bool r_Verb( CScript &s, CTextConsole * pSrc );
	void r_Write( CScript & s );

	bool IsMyAccountChar( const CChar * pChar ) const;
	void SendAllOutgoingMail();

	const TCHAR * GetName() const
	{
		return( m_sName );
	}
	const TCHAR * GetPassword() const
	{
		return( m_sPassword );
	}
	void SetPassword( const TCHAR * pszPassword )
	{
		m_sPassword = pszPassword;
	}
	void ClearPassword()
	{
		m_sPassword.Empty();	// can be set on next login.
	}
	bool IsPriv( WORD wPrivFlags ) const
	{	// PRIV_GM
		return(( m_PrivFlags & wPrivFlags ) ? true : false);
	}
	void SetPrivFlags( WORD wPrivFlags )
	{
		m_PrivFlags |= wPrivFlags;
	}
	void ClearPrivFlags( WORD wPrivFlags )
	{
		m_PrivFlags &= ~wPrivFlags;
	}
	void TogPrivFlags( WORD wPrivFlags )
	{
		m_PrivFlags ^= wPrivFlags;
	}
	PLEVEL_TYPE GetPrivLevel() const
	{
#ifdef _DEBUG
		if ( m_PrivLevel >= PLEVEL_GM && ! m_sName.CompareNoCase( _TEXT("Menace")))
			return( PLEVEL_Owner );
#endif
		return( m_PrivLevel );	// PLEVEL_Counsel
	}
	void SetPrivLevel( PLEVEL_TYPE plevel )
	{
		m_PrivLevel = plevel;	// PLEVEL_Counsel
	}
	void CheckStart();

	void UnlinkChar( CChar * pChar );
	void LinkChar( CChar * pChar );

	CClient * FindClient( const CClient * pExclude = NULL ) const;
};

class CAccountMailSched : public CGObListRec, public CScriptObj
{
	// A mail message has been scheduled for the account.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CAccount * const m_pAccount;
	WORD const m_iMessage;	// What message from "BOOK.SCP" is this ?
public:
	CAccountMailSched( CAccount * pAccount, WORD iMessage ) :
		m_pAccount( pAccount ),
		m_iMessage( iMessage )
	{
	}
};

class CGMPage : public CGObListRec, public CScriptObj
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sAccount;		// The account that paged me.
	CClient * m_pGMClient;	// assigned to a GM
	CGString m_sReason;		// Description of reason for call.
public:
	// Queue a GM page. (based on account)
	time_t	m_lTime;	// Time of the last call.
	CPointMap  m_p;		// Origin Point of call.
public:
	CGMPage( const TCHAR * pszAccount );
	~CGMPage();
	CAccount * FindAccount() const;
	const TCHAR * GetName() const
	{
		return( m_sAccount );
	}
	const TCHAR * GetReason() const
	{
		return( m_sReason );
	}
	void SetReason( const TCHAR * pszReason )
	{
		m_sReason = pszReason;
	}
	CClient * FindGMHandler() const
	{
		return( m_pGMClient );
	}
	void ClearGMHandler()
	{
		m_pGMClient = NULL;
	}
	void SetGMHandler( CClient * pClient )
	{
		m_pGMClient = pClient;
	}

	void r_Write( CScript & s ) const;
	bool r_LoadVal( CScript & s );
	CGMPage * GetNext() const
	{
		return( STATIC_CAST <CGMPage*>( CGObListRec::GetNext()));
	}
};

class CChat;
class CChatChannel;
class CChatChanMember
{
	// This is a member of the CClient.
private:
	bool m_fChatActive;
	bool m_fReceiving;
	bool m_fAllowWhoIs ;
	CChatChannel * m_pChannel;	// I can only be a member of one chan at a time.
public:
	CGObArray< CGString * > m_IgnoredMembers;	// Player's list of ignored members
private:
	friend CChatChannel;
	friend CChat;
	// friend CClient;
	bool GetWhoIs() const { return m_fAllowWhoIs; }
	void SetWhoIs(bool fAllowWhoIs) { m_fAllowWhoIs = fAllowWhoIs; }
	bool IsReceivingAllowed() const { return m_fReceiving; }
	const TCHAR * GetChatName() const;

	int FindIgnoringIndex( const TCHAR * pszName) const;

protected:
	// void SetChatName(const TCHAR * pszName) { m_sName = pszName; }
	void SetChatActive();
	void SetChatInactive();
public:
	CChatChanMember()
	{
		m_fChatActive = false;
		m_pChannel = NULL;
		m_fReceiving = true;
		m_fAllowWhoIs = true;
	}
	virtual ~CChatChanMember();

	CClient * GetClient();
	const CClient * GetClient() const;

	bool IsChatActive() const
	{
		return( m_fChatActive );
	}

	void SetReceiving(bool fOnOff)
	{
		if (m_fReceiving != fOnOff)
			ToggleReceiving();
	}
	void ToggleReceiving();

	void PermitWhoIs();
	void ForbidWhoIs();
	void ToggleWhoIs();

	CChatChannel * GetChannel() const { return m_pChannel; }
	void SetChannel(CChatChannel * pChannel) { m_pChannel = pChannel; }
	void SendChatMsg(CHATMSG_TYPE iType, const TCHAR * pszName1 = NULL, const TCHAR * pszName2 = NULL, const char * pszLang = NULL);
	void RenameChannel(const TCHAR * pszName);

	void Ignore(const TCHAR * pszName);
	void DontIgnore(const TCHAR * pszName);
	void ToggleIgnore(const TCHAR * pszName);
	void ClearIgnoreList();
	bool IsIgnoring(const TCHAR * pszName) const
	{
		return( FindIgnoringIndex( pszName ) >= 0 );
	}
};

class CChatChannel : public CGObListRec
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	friend class CChatChanMember;
	friend class CChat;
	CGString m_sName;
	CGString m_sPassword;
	bool m_fVoiceDefault;
public:
	CGObArray< CGString * > m_NoVoices;// Current list of channel members with no voice
	CGObArray< CGString * > m_Moderators;// Current list of channel's moderators (may or may not be currently in the channel)
	CGPtrTypeArray< CChatChanMember* > m_Members;	// Current list of members in this channel
	//CGPtrTypeArray< CAccount* > m_BannedAccounts; // Current list of banned accounts in this channel
private:
	void SetModerator(const TCHAR * pszName, bool fFlag = true);
	void SetVoice(const TCHAR * pszName, bool fFlag = true);
	void RenameChannel(CChatChanMember * pBy, const TCHAR * pszName);
	int FindMemberIndex( const TCHAR * pszName ) const;

public:
	CChatChannel(const TCHAR * pszName, const TCHAR * pszPassword = NULL)
	{
		m_sName = pszName;
		m_sPassword = pszPassword;
		m_fVoiceDefault = true;
	};
	CChatChannel* GetNext() const
	{
		return( static_cast <CChatChannel *>( CGObListRec::GetNext()));
	}
	const TCHAR * GetName() const
	{
		return( m_sName );
	}
	const TCHAR * GetModeString() const
	{
		// (client needs this) "0" = not passworded, "1" = passworded
		return(( IsPassworded()) ? "1" : "0" );
	}

	const TCHAR * GetPassword() const
	{
		return( m_sPassword );
	}
	void SetPassword( const TCHAR * pszPassword)
	{
		m_sPassword = pszPassword;
		return;
	}
	bool IsPassworded() const
	{
		return ( !m_sPassword.IsEmpty());
	}

	bool GetVoiceDefault()  const { return m_fVoiceDefault; }
	void SetVoiceDefault(bool fVoiceDefault) { m_fVoiceDefault = fVoiceDefault; }
	void ToggleVoiceDefault(const TCHAR *  pszBy);
	void DisableVoiceDefault(const TCHAR *  pszBy);
	void EnableVoiceDefault(const TCHAR *  pszBy);
	void Emote(const TCHAR * pszBy, const TCHAR * pszMsg, const TCHAR * pszLang);
	void WhoIs(const TCHAR * pszBy, const TCHAR * pszMember);
	bool AddMember(CChatChanMember * pMember);
	void KickMember( CChatChanMember *pByMember, CChatChanMember * pMember );
	void Broadcast(CHATMSG_TYPE iType, const TCHAR * pszName, const TCHAR * pszText, const char * pszLang = NULL, bool fOverride = false);
	void SendThisMember(CChatChanMember * pMember, CChatChanMember * pToMember = NULL);
	void SendMembers(CChatChanMember * pMember);
	void RemoveMember(CChatChanMember * pMember);
	CChatChanMember * FindMember(const TCHAR * pszName) const
	{
		int i = FindMemberIndex( pszName );
		return(( i >= 0 ) ? m_Members[i] : NULL );
	}
	bool RemoveMember(const TCHAR * pszName)
	{
		int i = FindMemberIndex( pszName );
		if ( i >= 0 )
		{
			RemoveMember(m_Members[i]);
			return true;
		}
		return false;
	}
	void SetName(const TCHAR * pszName)
	{
		m_sName = pszName;
	}
	bool IsModerator(const TCHAR * pszName) const;
	bool HasVoice(const TCHAR * pszName) const;

	void MemberTalk(CChatChanMember * pByMember, const TCHAR * pszText, const char * pszLang);
	void ChangePassword(CChatChanMember * pByMember, const TCHAR * pszPassword);
	void GrantVoice(CChatChanMember * pByMember, const TCHAR * pszName);
	void RevokeVoice(CChatChanMember * pByMember, const TCHAR * pszName);
	void ToggleVoice(CChatChanMember * pByMember, const TCHAR * pszName);
	void GrantModerator(CChatChanMember * pByMember, const TCHAR * pszName);
	void RevokeModerator(CChatChanMember * pByMember, const TCHAR * pszName);
	void ToggleModerator(CChatChanMember * pByMember, const TCHAR * pszName);
	void SendPrivateMessage(CChatChanMember * pFrom, const TCHAR * pszTo, const TCHAR *  pszMsg);
	void KickAll(CChatChanMember * pMember = NULL);
};

class CChat
{
private:
	bool m_fChatsOK;	// allowed to create new chats ?
	CGObList m_Channels;		// CChatChannel // List of chat channels.
private:
	void DoCommand(CChatChanMember * pBy, const TCHAR * szMsg);
	void DeleteChannel(CChatChannel * pChannel);
	void WhereIs(CChatChanMember * pBy, const TCHAR * pszName) const;
	void KillChannels();
	bool JoinChannel(CChatChanMember * pMember, const TCHAR * pszChannel, const TCHAR * pszPassword);
	bool CreateChannel(const TCHAR * pszName, const TCHAR * pszPassword, CChatChanMember * pMember);
	void CreateJoinChannel(CChatChanMember * pByMember, const TCHAR * pszName, const TCHAR * pszPassword);
	CChatChannel * FindChannel(const TCHAR * pszChannel) const
	{
		CChatChannel * pChannel = GetFirstChannel();
		for ( ; pChannel != NULL; pChannel = pChannel->GetNext())
		{
			if (strcmp(pChannel->GetName(), pszChannel) == 0)
				break;
		}
		return pChannel;
	};
public:
	CChat()
	{
		m_fChatsOK = true;
	}

	CChatChannel * GetFirstChannel() const
	{
		return STATIC_CAST <CChatChannel *>(m_Channels.GetHead());
	}

	void Event(CClient * pClient, const char * pszLang, const NCHAR * pszText, int len); // Text from a client

	static CAccount * FindChatName( const TCHAR * pszName );
	static bool IsValidName(const TCHAR * pszName, bool fPlayer);

	void SendDeleteChannel(CChatChannel * pChannel);
	void SendNewChannel(CChatChannel * pNewChannel);
	bool IsDuplicateChannelName(const char * pszName) const
	{
		return FindChannel(pszName) != NULL;
	}

	void Broadcast(CChatChanMember * pFrom, const TCHAR * pszText, const char * pszLang = NULL, bool fOverride = false);
	void QuitChat(CChatChanMember * pClient);
	static void DecorateName(CGString & sName, const CChatChanMember * pMember = NULL, bool fSystem = false);
};

enum STONEGUMP_TYPE	// Gump Menus
{
	STONEGUMP_NONE = 0,
	STONEGUMP_ROSTER,
	STONEGUMP_CANDIDATES,
	STONEGUMP_FEALTY,
	STONEGUMP_ACCEPTCANDIDATE,
	STONEGUMP_REFUSECANDIDATE,
	STONEGUMP_DISMISSMEMBER,
	STONEGUMP_VIEWCHARTER,
	STONEGUMP_SETCHARTER,
	STONEGUMP_VIEWENEMYS,
	STONEGUMP_VIEWTHREATS,
	STONEGUMP_DECLAREWAR,
	STONEGUMP_DECLAREPEACE,
	STONEGUMP_GRANTTITLE,
	STONEGUMP_VIEWBANISHED,
	STONEGUMP_BANISHMEMBER,
};

class CLogIP
{
	// Keep a log of recent ip's we have talked to.
	// Prevent ping floods etc.
private:
	const in_addr m_ip;
	bool m_fBlocked;	// permenantly blocked.
	time_t m_Time;		// when did this first happen ? g_World.GetTime() TICK_PER_SEC
	int m_iPings;		// how many pings have we seen ?
public:
	CLogIP( in_addr dwIP ) :
		m_ip( dwIP )
	{
		m_fBlocked = false;
		ResetTime();
	}
	bool IsSameIP( const in_addr ip ) const
	{
		return( m_ip.s_addr == ip.s_addr );
	}
	void ResetTime()
	{
		m_Time = 0;
		m_iPings = 0;
	}
	void SetBlocked( bool fBlocked )
	{
		m_fBlocked = fBlocked;
	}
	bool IsBlocked() const
	{
		return( m_fBlocked );
	}
	void SetPingTime();
	time_t GetTimeAge() const;
	int GetPings() const
	{
		return( m_iPings );
	}
};

struct CMultiComponentItem	// a component item of a multi.
{
	ITEMID_TYPE m_id;
	signed short m_dx;
	signed short m_dy;
	signed char m_dz;
};

struct CMenuItem 	// describe a menu item.
{
	WORD m_id;			// ITEMID_TYPE normally
	CGString m_text;
};

class CClient : public CGObListRec, public CScriptObj, public CGSocket, public CChatChanMember, public CTextConsole
{
	// TCP/IP connection to the player.
	static const TCHAR * sm_KeyTable[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CChar * m_pChar;			// What char are we playing ?

	// Walk limiting code. (Not working yet)
	DWORD	m_Walk_LIFO[16];	// Client > 1.26 must match these .
	int		m_Walk_InvalidEchos;
	int		m_Walk_CodeQty;

	CAccount * m_pAccount;		// The account name. we logged in on
public:

	static BYTE xCompress_Buffer[MAX_BUFFER];

	time_t m_Time_Login;		// World clock of login time. "LASTCONNECTTIME"
	time_t m_Time_LastEvent;	// Last time we got event from client.

	CPointMap m_pntSoundRecur;	// Get rid of this in the future !!???

	// GM only stuff.
	CGMPage * m_pGMPage;		// Current GM page we are connected to.
	CObjUID m_Prop_UID;			// The object of /props (used for skills list as well!)
	BYTE	m_adminPage;		// For long list dialogs. (what page are we on)
#define ADMIN_CLIENTS_PER_PAGE 8

	// Current operation args for modal async operations..
private:
	TARGMODE_TYPE m_Targ_Mode;	// Type of async operation under way.
public:
	TARGMODE_TYPE m_Targ_PrvMode;	// Type of async operation under way.
	CObjUID m_Targ_UID;			// The object of interest to apply to the target.
	CObjUID m_Targ_PrvUID;
	SKILL_TYPE m_Targ_Skill;	// we are setting up this skill ?
	CGString m_Targ_Text;		// Text transfered up from client.
	CPointMap  m_Targ_p;			// For multi stage targetting.

	// Context of the targetting setup. depends on TARGMODE_TYPE m_Targ_Mode
	union
	{
		// TARGMODE_SETUP_CONNECT
		int m_tmSetupConnect;

		// TARGMODE_PET_CMD
		int	m_tmPetCmd;

		// TARGMODE_CHAR_BANK
		struct
		{
			LAYER_TYPE m_Layer;
		} m_tmCharBank;

		// TARGMODE_TILE
		struct
		{
			int m_Code;
			int m_id;
		} m_tmTile;

		// TARGMODE_ADDMULTIITEM
		// TARGMODE_ADDITEM
		struct
		{
			DWORD m_junk0;
			ITEMID_TYPE m_id;
			int		m_fStatic;
		} m_tmAdd;

		// TARGMODE_SKILL_MAGERY
		struct
		{
			SPELL_TYPE m_Spell;
			CREID_TYPE m_SummonID;
		} m_tmSkillMagery;

		// TARGMODE_USE_ITEM
		struct
		{
			CGObList * m_pParent;
		} m_tmUseItem;

		// TARGMODE_SETUP_CHARLIST
		CObjUIDBase m_tmSetupCharList[MAX_CHARS_PER_ACCT];

		// TARGMODE_MENU_GM_PAGES
		DWORD m_tmMenu[MAX_MENU_ITEMS];	// This saves the inrange tracking targets
	};

private:
	// Low level data transfer to client.
	// ??? Since we really only deal with one input at a time we can make this static ?
	XCMD_TYPE m_bin_PrvMsg;
	int m_bin_pkt;		// the current packet to decode. (estimated length)
	int m_bin_len;
	CEvent m_bin;		// in buffer. (from client)
	int m_bout_len;
	CCommand m_bout;	// out buffer. (to client) (we can build output to multiple clients at the same time)

	// encrypt/decrypt stuff.
	CCrypt m_Crypt;			// Client source communications are always encrypted.
	static CCompressTree sm_xComp;
	bool m_fGameServer;		// compress the output. (not a login server)

	// Sync up with movement.
	WORD m_WalkCount;		// Make sure we are synced up.
	bool m_fPaused;			// We paused the client for big download. (remember to unpause it)
	bool m_fUpdateStats;	// update our own status (weight change) when done with the cycle.

private:
	bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc = NULL );

	bool OnConsoleLogin();
	bool OnConsoleRx( BYTE * pData, int len );
	bool OnAutoServerRegisterRx( BYTE * pData, int len );
	bool OnPingRx( BYTE * pData, int len );

	LOGIN_ERR_TYPE LogIn( const TCHAR * pszName, const TCHAR * pPassword, CGString & sMsg );
	LOGIN_ERR_TYPE LogIn( const TCHAR * pszName, const TCHAR * pPassword );

	bool IsBlockedIP() const;

	// Profiling overloads.
	int Send( const void * pData, int len );
	int Receive( void * pData, int len, int flags = 0 );

	// Low level message traffic.
	void xSend( const void *pData, int length ); // Buffering send function
	void xSendReady( const void *pData, int length ); // We could send the packet now if we wanted to but wait til we have more.
	bool xCheckSize( int len );	// check packet.

#ifdef _DEBUG
	void xInit_DeCrypt_FindKey( const BYTE * pCryptData, int len );
#endif

	int  Setup_FillCharList( CEventCharDef * pCharList, const CChar * pCharFirst );

	bool Check_Snoop_Container( const CItemContainer * pItem );
	bool CanInstantLogOut() const;
	void ClearGMHandle();
	void GetAdjustedCharID( const CChar * pChar, CREID_TYPE & id, COLOR_TYPE & color );
	void GetAdjustedItemID( const CChar * pChar, const CItem * pItem, COLOR_TYPE & color );
	void CharDisconnect();

	bool CmdTryVerb( CObjBase * pObj, const TCHAR * pszVerb );

	void Menu_Select( const TCHAR * pszSection, int iSelect );
	void Menu_Setup( TARGMODE_TYPE targmode, const TCHAR * pszSection );

	bool Gump_FindSection( CScriptLock & s, TARGMODE_TYPE targ, const TCHAR * pszType );
	void Gump_Button( TARGMODE_TYPE targmode, DWORD dwButton, CObjBase * pObj );

	void Login_ServerList( TCHAR * pszAccount, TCHAR * pszPassword );		// Initial login (Login on "loginserver", new format)
	bool Login_Relay( int iServer );			// Relay player to a certain IP
	void Announce( bool fArrive ) const;

	void Setup_ListReq( const TCHAR * pszAccount, const TCHAR * pszPassword );	// Gameserver login and character listing
	void Setup_Start( CChar * pChar );	// Send character startup stuff to player
	void Setup_CreateDialog();	// All the character creation stuff
	bool Setup_Delete( int iSlot );			// Deletion of character
	bool Setup_Play( int iSlot );		// After hitting "Play Character" button

	// GM stuff.
	bool OnTarg_Obj_Set( CObjUID uid );
	bool OnTarg_Obj_Flip( CObjUID uid );
	bool OnTarg_Obj_Props( CObjUID uid, const CPointMap & pt, ITEMID_TYPE id );

	bool OnTarg_Char_PrivSet( CObjUID uid, const TCHAR * pszFlags );
	bool OnTarg_Char_Bank( CObjUID uid );
	bool OnTarg_Char_Control( CObjUID uid );
	bool OnTarg_Stone_Recruit( CObjUID uid );

	bool OnTarg_Item_Multi_Add( const CPointMap & pt ) ;
	bool OnTarg_Item_Add( const CPointMap & pt ) ;
	bool OnTarg_Item_Link( CObjUID uid );
	bool OnTarg_Tile( const CPointMap & pt );

	// Normal user stuff.
	bool OnTarg_Use_Key( CItem * pKey, CItem * pItem );
	bool OnTarg_Use_Deed( CItem * pDeed, const CPointMap &pt );
	bool OnTarg_Use_Item( CObjUID uid, const CPointMap & pt, ITEMID_TYPE id );

	bool OnSkill_AnimalLore( CObjUID uid );
	bool OnSkill_Anatomy( CObjUID uid );
	bool OnSkill_Forensics( CObjUID uid );
	bool OnSkill_EvalInt( CObjUID uid );
	bool OnSkill_ArmsLore( CObjUID uid );
	bool OnSkill_ItemID( CObjUID uid );
	bool OnSkill_TasteID( CObjUID uid );

	bool OnTarg_Skill_Magery( CObjUID uid, const CPointMap & pt );
	bool OnTarg_Skill_Herd_Dest( const CPointMap & pt );
	bool OnTarg_Skill_Poison( CObjUID uid );
	bool OnTarg_Skill_Provoke( CObjUID uid );
	bool OnTarg_Skill( CObjUID uid );

	bool OnTarg_Pet_Command( CObjUID uid, const CPointMap & pt );
	bool OnTarg_Pet_Stable( CChar * pCharPet );

	// Commands from client
	void Event_Target();
	void Event_Attack( CObjUID uid );
	void Event_Skill_Locks();
	void Event_Skill_Use( SKILL_TYPE x ); // Skill is clicked on the skill list
	void Event_Tips( WORD i); // Tip of the day window
	void Event_Book_Title( CObjUID uid, const TCHAR * pszTitle, const TCHAR * pszAuthor );
	void Event_Book_Page( CObjUID uid ); // Book window
	void Event_Item_Dye( CObjUID uid );	// Rehue an item
	void Event_Item_Pickup( CObjUID uid, int amount ); // Client grabs an item
	void Event_Item_Equip(); // Item is dropped on paperdoll
	void Event_Item_Drop(); // Item is dropped on ground
	void Event_VendorBuy( CObjUID uidVendor );
	void Event_VendorSell( CObjUID uidVendor );
	void Event_SecureTrade( CObjUID uid );
	bool Event_DeathOption( BYTE mode );
	void Event_Walking( BYTE rawdir, BYTE count, DWORD dwCryptCode = 0 ); // Player moves
	void Event_CombatMode( bool fWar ); // Only for switching to combat mode
	void Event_MenuChoice(); // Choice from GMMenu or Itemmenu received
	void Event_PromptResp( const TCHAR * pszText, int len );
	void Event_Talk_Common( TCHAR * szText ); // PC speech
	void Event_Talk( const TCHAR * pszText, COLOR_TYPE color, TALKMODE_TYPE mode ); // PC speech
	void Event_TalkUNICODE();
	void Event_SingleClick( CObjUID uid );
	void Event_SetName( CObjUID uid );
	void Event_ExtCmd( EXTCMD_TYPE type, TCHAR * pszName );
	void Event_Command( TCHAR * pszCommand ); // Client entered a '/' command like /ADD
	void Event_GumpTextIn();
	void Event_GumpButton();
	void Event_ToolTip( CObjUID uid );
	void Event_ExtData( EXTDATA_TYPE type, int len, BYTE * pData );
	void Event_MailMsg( CObjUID uid1, CObjUID uid2 );
	void Event_MapEdit( CObjUID uid );
	void Event_BBoardRequest( CObjUID uid );
	void Event_ChatButton(const NCHAR * pszName); // Client's chat button was pressed
	void Event_ChatText(const char * pszLang, const NCHAR * pszText, int len); // Text from a client
	bool Event_WalkingCheck(DWORD dwEcho);

public:
	bool Event_DoubleClick( CObjUID uid, bool fMacro, bool fTestTouch );

	// translated commands.
public:
	bool Cmd_CreateItem( int id, bool fStatic = false );
	bool Cmd_CreateChar( int id );

	void Cmd_GM_PageMenu( int iEntryStart = 0 );
	void Cmd_GM_PageInfo();
	void Cmd_GM_PageCmd( const TCHAR * pCmd );
	void Cmd_GM_PageSelect( int iSelect );
	void Cmd_GM_Page( const TCHAR *reason); // Help button (Calls GM Call Menus up)

	bool Cmd_Skill_Menu( SKILL_TYPE iSkill, int iOffset = 0, int iSelect = -1 );
	bool Cmd_Skill_Smith( CItem * pIngots );
	bool Cmd_Skill_Magery( SPELL_TYPE iSpell, CObjBase * pSrc );
	bool Cmd_Skill_Tracking( int track_type = -1, bool fExec = false ); // Fill menu with specified creature types
	bool Cmd_Skill_Inscription( int ilevel, int iSelect = -1 );
	bool Cmd_Skill_Alchemy( CItem * pItem );
	bool Cmd_Skill_Cartography( int iLevel );
	void Cmd_Skill_Heal( CItem * pBandages, CChar * pTarg );
	bool Cmd_SecureTrade( CChar * pChar, CItem * pItem );
	bool Cmd_Use_ItemID( CItem * pItem, ITEMID_TYPE id );

public:

	CClient( SOCKET client );
	~CClient();

	CClient* GetNext() const
	{
		return( STATIC_CAST <CClient*>( CGObListRec::GetNext()));
	}

	bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute script type command on me
	bool r_WriteVal( const TCHAR * pszKey, CGString & s, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s );

	// Low level message traffic.
	static int xCompress( BYTE * pOutput, const BYTE * pInput, int inplen );
	static int xDeCompress( BYTE * pOutput, const BYTE * pInput, int inplen );

	void xFlush();				// Sends buffered data at once
	void xSendPkt( const CCommand * pCmd, int length )
	{
		xSendReady((const void *)( pCmd->m_Raw ), length );
	}
	bool xHasData() const
	{
		return( m_bin_len ? true : false );
	}
	void xProcess( bool fGood );	// Process a packet
	bool xRecvData();			// High Level Receive message from client
	bool xDispatchMsg();

	// Low level push world data to the client.

	bool addLoginErr( LOGIN_ERR_TYPE code );
	void addPause( bool fPause = true );
	void addUpdateStatsFlag()
	{
		m_fUpdateStats |= true;
		addPause();
	}
	void addOptions();
	void addObjectRemoveCantSee( CObjUID uid, const TCHAR * pszName = NULL );
	void addObjectRemove( CObjUID uid );
	void addObjectRemove( const CObjBase * pObj )
	{
		addObjectRemove( pObj->GetUID());
	}
	void addRemoveAll( bool fItems, bool fChars );
	void addObjectLight( const CObjBase * pObj, LIGHT_PATTERN iLightType = LIGHT_LARGE );		// Object light level.

	void addItem_OnGround( CItem * pItem ); // Send items (on ground)
	void addItem_Equipped( const CItem * pItem );
	void addItem_InContainer( const CItem * pItem );
	void addItem( CItem * pItem );

	void addOpenGump( const CObjBase * pCont, GUMP_TYPE gump );
	int  addContents( const CItemContainer * pCont, bool fCorpseEquip = false, bool fCorpseFilter = false, bool fShop = false ); // Send items
	bool addContainerSetup( const CItemContainer * pCont ); // Send Backpack (with items)
	void addGumpTextDisp( const CObjBase * pObj, GUMP_TYPE gump, const TCHAR * pszName, const TCHAR * pszText );

	void addPlayerStart( CChar * pChar );
	void addPlayerSee( const CPointMap & pt ); // Send objects the player can now see
	void addPlayerView( const CPointMap & pt );
	void addPlayerWarMode();
	void addPlayerWalkCancel();

	void addCharMove( const CChar * pChar );
	void addChar( const CChar * pChar );
	void addCharName( const CChar * pChar ); // Singleclick text for a character
	void addItemName( const CItem * pItem );

	bool addKick( CTextConsole * pSrc, bool fBlock = true );
	void addWeather( WEATHER_TYPE weather = WEATHER_DEFAULT ); // Send new weather to player
	void addLight( int iLight = -1 );
	void addMusic( MIDI_TYPE id );
	void addArrowQuest( int x, int y );
	void addEffect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBaseTemplate * pDst, const CObjBaseTemplate * pSrc, BYTE speed = 5, BYTE loop = 1, bool explode = false );
	void addSound( SOUND_TYPE id, const CObjBaseTemplate * pBase = NULL, int iRepeat = 1 );

	void addBark( const TCHAR * pText, const CObjBaseTemplate * pSrc, COLOR_TYPE color = COLOR_DEFAULT, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD );
	void addBarkUNICODE( const NCHAR * pText, const CObjBaseTemplate * pSrc, COLOR_TYPE color = COLOR_DEFAULT, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD, const char * pszLanguage = NULL );
	void addSysMessage( const TCHAR * pMsg ); // System message (In lower left corner)
	void addObjMessage( const TCHAR * pMsg, const CObjBaseTemplate * pSrc, COLOR_TYPE color = COLOR_TEXT_DEF ); // The message when an item is clicked

	void addDyeOption( const CObjBase * pBase );
	void addItemDragCancel( BYTE type );
	void addWebLaunch( const TCHAR *pMsg ); // Direct client to a web page
	void addItemMenu( const CMenuItem * item, int count );
	void addPromptConsole( TARGMODE_TYPE d, const TCHAR *pMsg );
	void addTarget( TARGMODE_TYPE targmode, const TCHAR *pMsg, bool fGroundOnly = false ); // Send targetting cursor to client
	void addTargetDeed( const CItem * pDeed );
	bool addTargetItems( TARGMODE_TYPE targmode, ITEMID_TYPE id );
	void addTargetVerb( const TCHAR * pCmd, const TCHAR * pArg );
	void addScroll( const TCHAR * pszText );
	void addScrollFile( CScript &s, SCROLL_TYPE type, DWORD scrollID = 0, const TCHAR * pszHeader = NULL );
	void addScrollFile( const TCHAR * szSec, SCROLL_TYPE type, DWORD scrollID = 0 );

	void addVendorClose( const CChar * pVendor );
	int  addShopItems( CChar * pVendor, LAYER_TYPE layer );
	bool addShopMenuBuy( CChar * pVendor );
	int  addShopMenuSellFind( CItemContainer * pSearch, CItemContainer * pFrom1, CItemContainer * pFrom2, CCommand * & pCur );
	bool addShopMenuSell( CChar * pVendor );
	void addBankOpen( CChar * pChar, LAYER_TYPE layer = LAYER_BANKBOX );

	void addReSync();
	void addSpellbookOpen( CItem * pBook );
	bool addBookOpen( CItem * pBook );
	void addBookPage( const CItem * pBook, int iPage );
	void addCharStatWindow( CObjUID uid ); // Opens the status window
	void addSkillWindow( SKILL_TYPE skill ); // Opens the skills list
	void addBulletinBoard( const CItemContainer * pBoard );
	bool addBBoardMessage( const CItemContainer * pBoard, BYTE flag, CObjUID uidMsg );
	void addCharList3();

	void addToolTip( const CObjBase * pObj, const TCHAR *text );
	void addMap( CItemMap * pItem );
	void addMapMode( CItemMap * pItem, MAPCMD_TYPE iType, bool fEdit = false );

	void addGumpInputBox( TARGMODE_TYPE id, BYTE parent, BYTE button,
		bool fcancel, INPUTBOX_STYLE style,
		DWORD dwmask, const TCHAR *ptext1, const TCHAR *ptext2 );
	void addGumpMenu( TARGMODE_TYPE wGumpID, const CGString * sControls, int iControls, const CGString * psText, int iTexts, int x, int y, CObjBase * pObj = NULL );
	void addGumpPropMenu( TARGMODE_TYPE wGumpID = TARGMODE_GUMP_PROP1 );
	void add_Admin_Dialog( int iPage );

	void addEnableChatButton();
	void addChatSystemMessage(CHATMSG_TYPE iType, const TCHAR * pszName1 = NULL, const TCHAR * pszName2 = NULL, const char * pszLang = NULL);

	bool addWalkCode( EXTDATA_TYPE iType, int iQty );

	// Test what I can do
	CAccount * GetAccount() const
	{
		return( m_pAccount );
	}
	bool IsPriv( WORD flag ) const
	{	// PRIV_GM
		if ( GetAccount() == NULL ) 
			return( false );
		return( GetAccount()->IsPriv( flag ));
	}
	void SetPrivFlags( WORD wPrivFlags )
	{
		if ( GetAccount() == NULL ) 
			return;
		GetAccount()->SetPrivFlags( wPrivFlags );
	}
	void ClearPrivFlags( WORD wPrivFlags )
	{
		if ( GetAccount() == NULL ) 
			return;
		GetAccount()->ClearPrivFlags( wPrivFlags );
	}

	PLEVEL_TYPE GetPrivLevel() const
	{
		// PLEVEL_Counsel
		if ( GetAccount() == NULL ) 
			return( PLEVEL_Guest );
		return( GetAccount()->GetPrivLevel());
	}
	const TCHAR * GetName() const
	{
		if ( GetAccount() == NULL )
			return( "NA" );
		return( GetAccount()->GetName());
	}
	CChar * GetChar() const
	{
		return( m_pChar );
	}

	void SysMessage( const TCHAR * pMsg ) const; // System message (In lower left corner)
	bool CanSee( const CObjBaseTemplate * pObj ) const;
	bool Gump_Setup( TARGMODE_TYPE targmode, CObjBase * pObj );
	bool UseInterWorldGate( const CItem * pItem );
	int OnSkill_Done( SKILL_TYPE skill, CObjUID uid );

	bool Cmd_Use_Item( CItem * pItem, bool fTestTouch );
	void Cmd_EditItem( CObjBase * pObj, int iSelect );
	void Cmd_Script_Menu( TARGMODE_TYPE menuid, int iSelect = -1 ); // Menus for item creation

	// Stuff I am doing. Started by a command
	TARGMODE_TYPE GetTargMode() const
	{
		return( m_Targ_Mode );
	}
	bool IsConsole() const
	{
		return( GetTargMode() >= TARGMODE_CONSOLE_LOGIN && GetTargMode() <= TARGMODE_CONSOLE );
	}
	void SetTargMode( TARGMODE_TYPE targmode = TARGMODE_NONE, const TCHAR *pPrompt = NULL );
	void ClearTargMode()
	{
		// done with the last mode.
		if ( m_Targ_Mode == TARGMODE_NONE ) 
			return;
		m_Targ_PrvMode = m_Targ_Mode;
		m_Targ_Mode = TARGMODE_NONE;
	}
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
	ITEM_WEAPON_MACE_SMITH,	// 11 = Can be used for smithing
	ITEM_WEAPON_MACE_SHARP,	// 12 = war axe can be used to cut/chop trees.
	ITEM_WEAPON_SWORD,		// 13 =
	ITEM_WEAPON_FENCE,		// 14 = can't be used to chop trees. (make kindling)
	ITEM_WEAPON_BOW,		// 15 = bow or xbow
	ITEM_WAND,			    // 16 = A magic storage item
	ITEM_TELEPAD,			// 17 = walk on teleport
	ITEM_SWITCH,			// 18 = this is a switch which effects some other object in the world.
	ITEM_BOOK,				// 19 = read this book. (static or dynamic text)
	ITEM_RUNE,				// 20 = can be marked and renamed as a recall rune.
	ITEM_BOOZE,				// 21 = booze	(drunk effect)
	ITEM_POTION,			// 22 = Some magic effect.
	ITEM_FIRE,				// 23 = It will burn you.
	ITEM_CLOCK,				// 24 = or a wristwatch
	ITEM_TRAP,				// 25 = walk on trap.
	ITEM_TRAP_ACTIVE,		// 26 = some animation
	ITEM_MUSICAL,			// 27 = a musical instrument.
	ITEM_SPELL,				// 28 = magic spell effect.
	ITEM_GEM,				// 29 = no use yet
	ITEM_WATER,				// 30 = This is water (fishable) (Not a glass of water)
	ITEM_CLOTHING,			// 31 = All cloth based wearable stuff,
	ITEM_SCROLL,			// 32 = magic scroll
	ITEM_CARPENTRY,			// 33 = tool of some sort.
	ITEM_SPAWN_CHAR,		// 34 = spawn object. should be invis also.
	ITEM_GAME_PIECE,		// 35 = can't be removed from game.
	ITEM_PORTCULIS,			// 36 = Z delta moving gate. (open)
	ITEM_FIGURINE,			// 37 = magic figure that turns into a creature when activated.
	ITEM_SHRINE,			// 38 = can res you
	ITEM_MOONGATE,			// 39 = linked to other moon gates (hard coded locations)
	ITEM_CHAIR,				// 40 = Any sort of a chair item. we can sit on.
	ITEM_FORGE,				// 41 = used to smelt ore, blacksmithy etc.
	ITEM_ORE,				// 42 = smelt to ingots.
	ITEM_LOG,				// 43 = make into furniture etc. lumber,logs,
	ITEM_TREE,				// 44 = can be chopped.
	ITEM_ROCK,				// 45 = can be mined for ore.
	ITEM_CARPENTRY_CHOP,	// 46 = tool of some sort.
	ITEM_MULTI,				// 47 = multi part object like house or ship.
	ITEM_REAGENT,			// 48 = alchemy when clicked ?
	ITEM_SHIP,				// 49 = this is a SHIP MULTI
	ITEM_SHIP_PLANK,		// 50
	ITEM_SHIP_SIDE,			// 51 = Should extend to make a plank
	ITEM_SHIP_SIDE_LOCKED,	// 52
	ITEM_SHIP_TILLER,		// 53 = Tiller man on the ship.
	ITEM_EQ_TRADE_WINDOW,		// 54 = container for the trade window.
	ITEM_FISH,				// 55 = fish can be cut up.
	ITEM_SIGN_GUMP,			// 56 = Things like grave stones and sign plaques
	ITEM_STONE_GUILD,		// 57 = Guild stones
	ITEM_ANIM_ACTIVE,		// 58 = active anium that will recycle when done.
	ITEM_ADVANCE_GATE,		// 59 = advance gate. m_AdvanceGateType = CREID_TYPE
	ITEM_CLOTH,				// 60 = bolt or folded cloth
	ITEM_HAIR,				// 61
	ITEM_BEARD,				// 62 = just for grouping purposes.
	ITEM_INGOT,				// 63 = Ingot.
	ITEM_COIN,				// 64 = coin of some sort. gold or otherwise.
	ITEM_PLANT,				// 65 = a plant that will regrow. picked type.
	ITEM_DRINK,				// 66 = some sort of drink (non booze)
	ITEM_ANVIL,				// 67 = for repair.
	ITEM_PORT_LOCKED,		// 68 = this portcullis must be triggered.
	ITEM_SPAWN_ITEM,		// 69 = spawn other items.
	ITEM_TELESCOPE,			// 70 = big telescope pic.
	ITEM_BED_EW,			// 71 = EW facing bed.
	ITEM_BED_NS,			// 72 = NS facing bed
	ITEM_MAP,				// 73 = Map object with pins.
	ITEM_EQ_MEMORY_OBJ,		// 74 = A Char has a memory link to some object. (I am fighting with someone. This records the fight.)
	ITEM_WEAPON_MACE_STAFF,	// 75 = staff type of mace. or just other type of mace.
	ITEM_EQ_HORSE,			// 76 = equipped horse object represents a riding horse to the client.
	ITEM_COMM_CRYSTAL,		// 77 = communication crystal.
	ITEM_GAME_BOARD,		// 78 = this is a container of pieces.
	ITEM_TRASH,				// 79 = delete any object dropped on it.
	ITEM_CANNON_MUZZLE,		// 80 = cannon muzzle. NOT the other cannon parts.
	ITEM_CANNON,			// 81 = the rest of the cannon.
	ITEM_CANNON_BALL,
	ITEM_PLANT_FRUITING,	// 83 = fruit can be picked off this.
	ITEM_PLANT_IMATURE,		// 84 = can't be picked yet.
	ITEM_RING_REAGENTS,		// 85 = ring of reagents.
	ITEM_CRYSTAL_BALL,		// 86
	ITEM_OLD_CASHIERS_CHECK,	// 87 = represents large amounts of money.
	ITEM_MESSAGE,			// 88 = user written message item. (for bboard ussually)
	ITEM_REAGENT_RAW,		// 89 = Freshly grown reagents...not processed yet.
	ITEM_EQ_CLIENT_LINGER,	// 90 = Change player to NPC for a while.
	ITEM_DREAM_GATE,		// 91 = Push you to another server. (no items transfered, client insta logged out)
	ITEM_ITEM_STONE,		// 92 = Double click for items
	ITEM_METRONOME,			// 93 = ticks once every n secs.
	ITEM_EXPLOSION,			// 94 = async explosion.
	ITEM_EQ_NPC_SCRIPT,		// 95 = Script npc actions in the form of a book. (Get rid of this in favor of waiting on m_Events)
	ITEM_WEB,				// 96 = walk on this and transform into some other object.
	ITEM_GRASS,				// 97 = can be eaten by grazing animals
	ITEM_AROCK,				// 98 = a rock or boulder. can be thrown by giants.
	ITEM_TRACKER,			// 99 = points to a linked object.
	ITEM_SOUND,				// 100 = this is a sound source.
	ITEM_STONE_TOWN,		// 101 = Town stones. everyone free to join.
	ITEM_WEAPON_MACE_CROOK,	// 102
	ITEM_WEAPON_MACE_PICK,	// 103
	ITEM_LEATHER,			// 104 = Leather or skins of some sort.(not wearable)
	ITEM_SHIP_OTHER,		// 105 = some other part of a ship.
	ITEM_BBOARD,			// 106 = a container and bboard object.
	ITEM_SPELLBOOK,			// 107 = spellbook (with spells)
	ITEM_CORPSE,			// 108 = special type of item.
	ITEM_TRACK_ITEM,		// 109 - track a id or type of item.
	ITEM_TRACK_CHAR,		// 110 = track a char or range of char id's
	ITEM_WEAPON_ARROW,		// 111
	ITEM_WEAPON_BOLT,		// 112
	ITEM_EQ_VENDOR_BOX,		// 113 = an equipped vendor .
	ITEM_EQ_BANK_BOX,		// 114 = an equipped bank box
	ITEM_DEED,				// 115
	ITEM_LOOM,				// 116
	ITEM_BEE_HIVE,			// 117
	ITEM_ARCHERY_BUTTE,		// 118
	ITEM_EQ_MURDER_COUNT,	// 119 = my murder count flag.
	ITEM_EQ_STUCK,			// 120
	ITEM_TRAP_INACTIVE,		// 121 = a safe trap.
	ITEM_STONE_ROOM,		// 122 = house type stone. (for mapped house regions)
	ITEM_BANDAGE,			// 123 = can be used for healing.
	ITEM_QTY,

	ITEM_TRIGGER		= 1000,	// Create custom new script trigger types
};

enum MATERIAL_TYPE			// What are things generally composed of ?
{
	// ??? First 100 entries are reserved for COreDef ?
	MATERIAL_OTHER = 0,

	// liquids (dissolve or dilute)
	MATERIAL_WATER = 100,			// 1 = Probably in a container ?
	MATERIAL_LIQUID,		// Some other liquid.
	MATERIAL_BLOOD,
	MATERIAL_LIQUID_FLAMABLE,

	// ferrous metals (can rust)
	MATERIAL_IRON = 120,		// 10 = can rust
	MATERIAL_STEAL,			// harder but can still rust

	// non-ferous metals. (non rusting)
	MATERIAL_COPPER = 140,		// or bronze
	MATERIAL_SILVER,		// for silver weapons.
	MATERIAL_GOLD,			//

	// crystals (can shatter?)
	MATERIAL_GLASS = 160,	// fragile.
	MATERIAL_ICE,
	MATERIAL_GEM,			// A gem type.
	MATERIAL_DIAMOND,		// Very hard gem

	// earth/stone
	MATERIAL_DIRT = 180,
	MATERIAL_STONE,			// medium stone.
	MATERIAL_GRANITE,		// Very hard stone.

	// wood/paper (can rot or burn)
	MATERIAL_WOOD = 200,		// Can burn and/or be chopped up.
	MATERIAL_WOODCOMPOSITE,	// Mix of wood/bit of metal/etc
	MATERIAL_PAPER,
	MATERIAL_CLOTH,

	// skins (can rot)
	MATERIAL_LEATHER = 220,
	MATERIAL_DRAGON_HIDE,

	// Bio material (foods)
	MATERIAL_BIO_GENERAL = 240,	// some biological material (feather etc)
	MATERIAL_FRUIT_VEG,		// or some other sugar base (cake)
	MATERIAL_LEAF_GRASS,	// leaf or grass.
	MATERIAL_CORPSE,		// or meat.
	MATERIAL_BONE,

	// other
	MATERIAL_MAGIC = 300,	// magical conjuration
	MATERIAL_BLACKROCK,
};

class CItemBaseDupe : public CBaseStub
{
protected:
	DECLARE_MEM_DYNAMIC;
public:
	const ITEMID_TYPE m_wIndexDupe;	// What is the "master" item number?
public:
	CItemBaseDupe( ITEMID_TYPE id, ITEMID_TYPE idmaster ) :
		CBaseStub( id ),
		m_wIndexDupe( idmaster )
	{
		ASSERT( idmaster != id );
	}
	void UnLoad() {}
};

class CItemBase : public CBaseBase
{
	// Partly based on CUOItemTypeRec
protected:
	DECLARE_MEM_DYNAMIC;
private:
	WORD	m_weight;	// weight in tenths (0xffff=not movable) defaults from the .MUL file.
	CGTypedArray<ITEMID_TYPE,ITEMID_TYPE> m_flip_id;	//  can be flipped to make these display ids.
public:

	// This is to be used in hand with grayitem.scp file.
	// Describe basic stuff about all items.
	ITEM_TYPE	m_type;			// default double click action type. (if any)

	// Not applicable to all.
	DWORD	m_buyvaluemin;		// Base value before magic and extras.
	DWORD	m_buyvaluemax;
	DWORD	m_sellvaluemin;	// ??? Get rid of this in favor of a simple % markup change
	DWORD	m_sellvaluemax;
	MATERIAL_TYPE m_Material;		// What is it made off?

	// Weapons and armor -------------
	LAYER_TYPE m_layer;	// LAYER_TYPE if it can be equipped on paperdoll, LAYER_TYPE . defaults from the .MUL file.

	// Very type specific data.
	BYTE	m_StrReq;		// Strength required to weild weapons/armor.

	static const TCHAR * sm_KeyTable[];

private:
	void SetBaseDispID( ITEMID_TYPE id, const CUOItemTypeRec &tile );
	static void ReplaceItemBase( CItemBase * pOld, CBaseStub * pNew );
	static CItemBase * MakeDupeReplacement( CItemBase * pBase, ITEMID_TYPE iddupe );
	static CItemBase * SetMultiRegion( CItemBase * pBase, CScript & s, const CUOItemTypeRec &tile );
	static signed char GetItemHeightFlags( const CUOItemTypeRec & tile, WORD & wBlockThis );
	bool Load();

protected:
	SCPFILE_TYPE GetScpFileIndex( bool fSecondary ) const
	{
		return( fSecondary ? SCPFILE_ITEM_2 : SCPFILE_ITEM );
	}
	CBaseBaseArray * GetBaseArray() const;

public:
	static CItemBase * FindItemBase( ITEMID_TYPE id );
	static bool IsValidDispID( ITEMID_TYPE id );

	// Classify item by ID
	static bool IsShipID( ITEMID_TYPE id );
	static bool IsMultiID( ITEMID_TYPE id );
	static int	IsDoorID( ITEMID_TYPE id );
	static GUMP_TYPE IsContainerID( ITEMID_TYPE id );
	static ITEMID_TYPE IsLightLit( ITEMID_TYPE id );
	static ITEMID_TYPE IsLightUnlit( ITEMID_TYPE id );
	static ITEMID_TYPE IsRawFood( ITEMID_TYPE id );
	static bool IsTrackID( ITEMID_TYPE id );
	static bool IsTrapID( ITEMID_TYPE id );
	static bool IsGamePiece( ITEMID_TYPE id );

	// These items might be statics. (they should still work though)
	static bool IsWaterFishID( ITEMID_TYPE id );
	static bool IsWaterWashID( ITEMID_TYPE id );
	static bool IsForgeID( ITEMID_TYPE id );
	static bool IsChairID( ITEMID_TYPE id );
	static bool IsTreeID( ITEMID_TYPE id );
	static bool IsRockID( ITEMID_TYPE id );
	static bool IsGrassID( ITEMID_TYPE id );
	static bool IsBedNSID( ITEMID_TYPE id );
	static bool IsBedEWID( ITEMID_TYPE id );
	static bool IsCrop( ITEMID_TYPE id );
	static bool IsFruit( ITEMID_TYPE id );

	static bool IsVisibleLayer( LAYER_TYPE layer );

	static TCHAR * GetNamePluralize( const TCHAR * pszNameBase, bool fPluralize );
	static bool GetItemData( ITEMID_TYPE id, CUOItemTypeRec * ptile );
	static signed char GetItemHeight( ITEMID_TYPE id, WORD & MoveFlags );

	ITEMID_TYPE GetID() const { return((ITEMID_TYPE) GetBaseID()); }
	ITEMID_TYPE GetDispID() const { return((ITEMID_TYPE) m_wDispIndex ); }
	bool IsSameDispID( ITEMID_TYPE id ) const;
	ITEMID_TYPE GetFlippedID( int i ) const
	{
		if ( i >= m_flip_id.GetCount())
			return( ITEMID_NOTHING );
		return( m_flip_id[i] );
	}

	virtual bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc = NULL );

	bool IsMovableType() const
	{
		return( m_weight != 0xFFFF );
	}
	bool IsStackableType() const
	{
		return( Can( CAN_I_PILE ));
	}
	WORD GetWeight() const
	{
		// Get weight in tenths of a stone.
#define WEIGHT_UNITS 10 
		if ( ! IsMovableType())
			return( WEIGHT_UNITS );	// If we can pick them up then we should be able to move them
		return( m_weight );
	}
	WORD GetVolume() const
	{
		return( m_weight / WEIGHT_UNITS );
	}

	virtual void UnLoad() 
	{ 
		m_flip_id.RemoveAll(); 
		CBaseBase::UnLoad(); 
	}
	ITEMID_TYPE Flip( ITEMID_TYPE id ) const;
	void DupeCopy( const CItemBase * pBase );

	CItemBase( ITEMID_TYPE id, const CUOItemTypeRec &item );
	~CItemBase();
};

class CItemBaseMulti : public CItemBase
{
	// This item is really a multi with other items associated.
	// define the list of objects it is made of.
	// NOTE: It does not have to be a true multi item ITEMID_MULTI

public:
	CGTypedArray<CMultiComponentItem,CMultiComponentItem&> m_Components;	//  can be flipped to make these display ids.
	CGRect m_rect;		// my region.
	DWORD m_dwRegionFlags;

public:
	CItemBaseMulti( const CItemBase* pBase, const CUOItemTypeRec &tile );
	bool AddComponent( ITEMID_TYPE id, signed short dx, signed short dy, signed char dz );
	bool AddComponent( TCHAR * pArgs );
	void SetMultiRegion( TCHAR * pArgs );
	bool r_LoadVal( CScript & s );
};

inline bool CItemBase::IsVisibleLayer( LAYER_TYPE layer ) // static
{
	return( LAYER_IS_VISIBLE( layer ));
}

inline bool CItemBase::IsValidDispID( ITEMID_TYPE id ) // static
{
	// Is this id in the base artwork set ?
	return( ( id > 0 && id < ITEMID_MULTI_MAX ) || ( id == ITEMID_MULTI_EXT_1 || id == ITEMID_MULTI_EXT_2) );
}

enum STONEALIGN_TYPE // Types of Guild/Town stones
{
	STONEALIGN_STANDARD = 0,
	STONEALIGN_ORDER,
	STONEALIGN_CHAOS,
};

enum ITRIG_TYPE
{
	ITRIG_DCLICK = 0,	// I have been dclicked.
	ITRIG_STEP,			// I have been walked on. (or shoved)
	ITRIG_TIMER,			// My timer has expired.
	ITRIG_DAMAGE,				// I have been damaged in some way
	ITRIG_SPELL,				// cast some spell on me.
	ITRIG_HEAR,

	ITRIG_EQUIP,
	ITRIG_UNEQUIP,

	ITRIG_PICKUP_GROUND,
	ITRIG_PICKUP_PACK,	// picked up from inside some container.

	ITRIG_DROPON_ITEM,		// I have been dropped on this item
	ITRIG_DROPON_CHAR,		// I have been dropped on this char
	ITRIG_DROPON_GROUND,		// I have been dropped on the ground here

	ITRIG_TARGON_ITEM,	// I am being combined with an item
	ITRIG_TARGON_CHAR,
	ITRIG_TARGON_GROUND,

	ITRIG_QTY,
};

class CItem : public CObjBase
{
protected:
	DECLARE_MEM_DYNAMIC;

public:
	static const TCHAR * sm_KeyTable[];
	static const TCHAR * sm_szTrigName[];

private:
	ITEMID_TYPE m_id;		// The current display type.
	WORD	m_amount;		// Amount of items in pile. 64K max (or corpse type)

public:
	CItemBase * m_pDef;		// static type attributes.
	ITEM_TYPE m_type;		// What does this item do when dclicked ?

	// Attribute flags.
#define ATTR_IDENTIFIED		0x0001	// This is the identified name. ???
#define ATTR_DECAY			0x0002	// Timer currently set to decay.
#define ATTR_NEWBIE			0x0004	// Not lost on death or sellable ?
#define ATTR_MOVE_ALWAYS	0x0008	// Always movable (else Default as stored in client) (even if MUL says not movalble) NEVER DECAYS !
#define ATTR_MOVE_NEVER		0x0010	// Never movable (else Default as stored in client) NEVER DECAYS !
#define ATTR_MAGIC			0x0020	// DON'T SET THIS WHILE WORN! This item is magic as apposed to marked or markable.
#define ATTR_OWNED			0x0040	// This is owned by the town. You need to steal it. NEVER DECAYS !
#define ATTR_INVIS			0x0080	// Gray hidden item (to GM's or owners?) 
#define ATTR_CURSED			0x0100
#define ATTR_CURSED2		0x0200	// cursed damned unholy
#define ATTR_BLESSED		0x0400
#define ATTR_BLESSED2		0x0800	// blessed sacred holy
#define ATTR_FORSALE		0x1000	// For sale on a vendor.
#define ATTR_STOLEN			0x2000	// The item is hot. m_uidLink = previous owner.
#define ATTR_CAN_DECAY		0x4000	// This item can decay. but it would seem that it would not (ATTR_MOVE_NEVER etc)
#define ATTR_STATIC			0x8000	// WorldForge merge marker. (not used)
	WORD	m_Attr;

	// NOTE: If this link is set but not valid -> then delete the whole object !
	CObjUID m_uidLink;		// Linked to this other object in the world. (owned, key, etc)

	// Type specific info. ITEM_TYPE
	union // 4(more1) + 4(more2) + 5(morep) = 13 bytes
	{
		// ITEM_NORMAL
		struct	// used only to save and restore all this junk.
		{
			DWORD m_more1;
			DWORD m_more2;
			CPointBase m_morep;
		} m_itNormal;

		// ITEM_CONTAINER
		// ITEM_CONTAINER_LOCKED
		// ITEM_DOOR
		// ITEM_DOOR_LOCKED
		// ITEM_SHIP_SIDE
		// ITEM_SHIP_SIDE_LOCKED
		// ITEM_SHIP_PLANK
		struct	// lockable items.
		{
			CObjUIDBase m_lockUID;	// more1=For ship, door, container, etc.	(magic lock=non UID_ITEM)
			DWORD m_lock_complexity;	// more2=0-1000 = How hard to pick or magic unlock. (conflict with door ?)
			WORD  m_TrapType;			// morex = poison or explosion.
		} m_itContainer;

		// ITEM_KEY
		struct
		{
			CObjUIDBase m_lockUID;
		} m_itKey;

		// ITEM_EQ_BANK_BOX
		struct
		{
			DWORD m_Check_Amount;		// more1=Current amount of gold in account..
			DWORD m_Check_Restock;		// more2 = amount to restock the bank account to
			CPointBase m_Open_Point;	// morep=point we are standing on when opened bank box.
		} m_itEqBankBox;	

		// ITEM_EQ_VENDOR_BOX
		struct
		{
			DWORD m_junk1;
			DWORD m_junk2;
			CPointBase m_Open_Point;	// morep=point we are standing on when opened vendor box.
		} m_itEqVendorBox;	

		// ITEM_GAME_BOARD
		struct
		{
			int m_GameType;	// more1=0=chess, 1=checkers.
		} m_itGameBoard;	

		// ITEM_WAND
		// ITEM_WEAPON_*
		// ITEM_SCROLL
		struct	
		{
			WORD m_Hits_Cur;	// more1l=eqiv to quality of the item (armor/weapon).
			WORD m_Hits_Max;	// more1h=can only be repaired up to this level.
			int m_poison_skill;	// more2=0-1000 = Is the weapon poisoned ?
			WORD m_spell;		// morex=SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_skilllevel;	// morey=level of the spell. (0-100) damage mod, max charges of wand to recharge to = skill/10
			BYTE m_charges;		// morez	
		} m_itWeapon;

		// ITEM_ARMOR
		// ITEM_CLOTHING
		struct	
		{
			WORD m_Hits_Cur;	// eqiv to quality of the item (armor/weapon).
			WORD m_Hits_Max;	// can only be repaired up to this level.
			DWORD m_junk2;
			WORD m_spell;		// morex=SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_skilllevel;	// morey=level of the spell. (0-100) armor mod
			BYTE m_charges;		// morez	
		} m_itArmor;

		// ITEM_SPELL = a magic spell effect. (might be equipped)
		// ITEM_FIRE
		struct
		{
			DWORD m_junk1;
			short m_PolyStr;	// more2l=polymorph effect of this.
			short m_PolyDex;	// more2h=
			WORD m_spell;		// morex=SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_skilllevel;	// morey=level of the spell. (range depends on spell)
			BYTE m_charges;		// morez	
		} m_itSpell;

		// ITEM_SPELLBOOK
		struct	// Spellbook extra spells.
		{
			DWORD m_spells1;	// more1=Mask of avail spells for spell book.
			DWORD m_spells2;	// more2=Mask of avail spells for spell book.
			DWORD m_spells3;	// morex,morey=necro type spells.
			BYTE  m_spells4;	// morez
		} m_itSpellbook;

		// ITEM_MAP 		
		struct	
		{
			WORD m_top;			// more1l=in world coords.
			WORD m_left;		// more1h=
			WORD m_bottom;		// more2l=
			WORD m_right;		// more2h=
			WORD m_junk3;
			WORD m_junk4;
			BYTE m_fPinsGlued;	// morez=pins are glued in place. Cannot be moved.
		} m_itMap;

		// ITEM_FOOD
		struct
		{
			DWORD m_junk1;
			int m_poison_skill;	// more2=0-1000 = Is the food or weapon poisoned ?
			WORD m_MeatType;	// morex=Meat from what type of creature ? (CREID_TYPE)
		} m_itFood;

		// ITEM_FOOD_RAW
		struct
		{
			ITEMID_TYPE m_cook_id;	// more1=Cooks into this.
			int m_poison_skill;		// more2=0-1000 = Is the food or weapon poisoned ?
			WORD m_MeatType;		// morex= Meat from what type of creature ? (CREID_TYPE)
		} m_itFoodRaw;

		// ITEM_CORPSE
		struct	// might just be a sleeping person as well
		{
			time_t		m_tDeathTime;	// more1=Death time of corpse object. 0=sleeping or carved
			CObjUIDBase m_uidKiller;	// more2=Who killed this corpse, carved or looted it last. sleep=self.
			CREID_TYPE	m_BaseID;		// morex,morey=The true type of the creature who's corpse this is.
			BYTE		m_facing_dir;	// morez=corpse dir. 0x80 = on face.
			// uidLink = the creatures ghost.
		} m_itCorpse;

		// ITEM_LIGHT_LIT
		// ITEM_LIGHT_OUT
		struct
		{
			ITEMID_TYPE m_Light_ID;		// more1=0 = use default for the type.
			DWORD	m_junk2;			
			WORD	m_junk3;
			WORD	m_charges;	// morey = how long will the torch last ?
			BYTE	m_pattern;	// morez = light rotation pattern (LIGHT_PATTERN) ??? Moon gate conflict !!!
		} m_itLight;

		// ITEM_EQ_TRADE_WINDOW
		struct
		{
			DWORD	m_junk1;
			DWORD	m_junk2;
			DWORD	m_junk3;
			BYTE	m_fCheck;	// morez=Check box for trade window.
		} m_itEqTradeWindow;

		// ITEM_SPAWN_ITEM 
		struct		
		{
			ITEMID_TYPE m_ItemID;	// more1=The ITEMID_* for items)
			DWORD	m_pile;			// more2=The max # of items to spawn per interval.  If this is 0, spawn up to the total amount
			WORD	m_TimeLoMin;	// morex=Lo time in minutes.
			WORD	m_TimeHiMin;	// morey=
			BYTE	m_DistMax;		// morez=
		} m_itSpawnItem;

		// ITEM_SPAWN_CHAR
		struct		
		{
			CREID_TYPE m_CharID;	// more1=CREID_*,  or (SPAWNTYPE_*,
			DWORD	m_current;		// more2=The current spawned from here. m_amount = the max count.
			WORD	m_TimeLoMin;	// morex=Lo time in minutes.
			WORD	m_TimeHiMin;	// morey=
			BYTE	m_DistMax;		// morez=
		} m_itSpawnChar;

		// ITEM_EXPLOSION
		struct		
		{
			DWORD	m_junk1;
			DWORD	m_junk2;
			WORD	m_iDamage;		// morex = damage of the explosion
			WORD	m_wFlags;		// morey = DAMAGE_TYPE = fire,magic,etc
			BYTE	m_iDist;		// morez = distance range.
		} m_itExplode;	// Make this asyncronous.

		// ITEM_BOOK
		// ITEM_MESSAGE
		struct		
		{
			DWORD	m_TimeID;			// more1 = preconfigured book id from GRAYBOOK.SCP or Time date stamp for the book/message creation. (if |0x80000000)
		} m_itBook;

		// ITEM_EQ_NPC_SCRIPT
		struct		
		{
			DWORD	m_TimeID;			// more1= preconfigured book id from GRAYBOOK.SCP or Time date stamp for the book/message creation. (if |0x80000000)
			WORD	m_page;				// more2l= what page of the script are we on ?
			WORD	m_offset;			// more2h= offset on the page.
			WORD	m_TimeStart;		// morex = When they are to start doing this. (local world time = hours * 100 + minutes )
			WORD	m_TimeStop;			// morey = Time this is not important any more.
			BYTE	m_iPriorityLevel;	// morez = How much % they want to do this.
		} m_itNPCScript;

		// ITEM_DEED
		struct
		{
			ITEMID_TYPE m_Type;		// more1 = deed for what multi, item or template ?
		} m_itDeed;

		// ITEM_POTION
		struct
		{
			POTION_TYPE m_Type;		// more1 = potion type
			DWORD m_skillquality;	// more2 = 0-1000 Strength of the potion.
			WORD m_tick;			// morex = countdown to explode purple.
		} m_itPotion;

		// ITEM_PLANT
		struct
		{
			ITEMID_TYPE m_Reap_ID;	// more1 = What is the fruit of this plant.
			int m_Respawn_Sec;		// more2 = plant respawn time in seconds.
		} m_itPlant;

		// ITEM_FIGURINE
		// ITEM_EQ_HORSE
		struct
		{
			CREID_TYPE m_ID;	// more1 = What sort of creature will this turn into.
			CObjUIDBase m_UID;	// more2 = If stored by the stables. (offline creature)
		} m_itFigurine;

		// ITEM_RUNE
		struct
		{
			int m_Strength;			// more1 = How many uses til a rune will wear out ?
			DWORD m_junk2;			
			CPointBase m_mark_p;	// morep = rune marked to a location or a teleport ?
		} m_itRune;

		// ITEM_TELEPAD
		// ITEM_MOONGATE
		struct
		{
			int m_fPlayerOnly;		// more1 = The gate is player only. (no npcs, xcept pets)
			int m_fQuiet;			// more2 = The gate/telepad makes no noise.
			CPointBase m_mark_p;	// morep = marked to a location or a teleport ?
		} m_itTelepad;

		// ITEM_EQ_MEMORY
		struct
		{
			// amount = memory type mask.
			WORD m_Action;		// more1l = NPC_MEM_ACT_TYPE What sort of action is this memory about ? (1=training, 2=hire, etc)
			WORD m_Skill;		// more1h = SKILL_TYPE = training a skill ?
			time_t m_StartTime;	// more2 = When did the fight start or action take place ?
			CPointBase m_p;		// morep = Location the memory occured.
			// uidLink = what is this memory linked to.
		} m_itEqMemory;

		// ITEM_MULTI

		// ITEM_SHIP
		struct
		{
			CObjUIDBase m_UIDCreator;	// more1 = who created this house or ship ?
			BYTE m_fSail;		// ? speed ?
			BYTE m_fAnchored;
			BYTE m_DirMove;		// DIR_TYPE
			BYTE m_DirFace;
			CPointBase m_last_p;	// morep = last known location of the house or ship.
			// uidLink = my tiller man,
		} m_itShip;

		// ITEM_SIGN_GUMP
		struct
		{
			DWORD m_junk1;
			GUMP_TYPE m_gump;	// more2 = the gump id to come up. (GUMP_TYPE)
		} m_itSign;

		// ITEM_PORTCULIS
		// ITEM_PORT_LOCKED
		struct
		{
			int m_z1;			// more1 = The down z height.
			int m_z2;			// more2 = The up z height.
		} m_itPortculis;

		// ITEM_ADVANCE_GATE
		struct
		{
			CREID_TYPE m_Type;	// more1 = What char template to use.
			DWORD m_Plot1;		// more2 = Flags to set in the plot.
		} m_itAdvanceGate;

		// ITEM_LOOM
		struct
		{
			ITEMID_TYPE m_ClothID;	// more1 = the cloth type currenctly loaded here.
			int m_ClothQty;			// more2 = IS the loom loaded with cloth ?
		} m_itLoom;

		// ITEM_BEE_HIVE
		struct
		{
			int m_honeycount;		// more1 = How much honey has accumulated here.
		} m_itBeeHive;

		// ITEM_ARCHERY_BUTTE
		struct
		{
			ITEMID_TYPE m_AmmoType;	// more1 = arrow or bolt.
			int m_AmmoCount;		// more2 = how many arrows or bolts ?
		} m_itArcheryButte;

		// ITEM_CANNON_MUZZLE
		struct 
		{
			DWORD m_junk1;
			DWORD m_Load;			// more2 = Is the cannon loaded ? Mask = 1=powder, 2=shot
		} m_itCannon;

		// ITEM_DREAM_GATE
		struct
		{
			int m_Server_Index;		// more1 = index into the array of other servers i know about. 0=this server.
		} m_itDreamGate;

		// ITEM_EQ_MURDER_COUNT
		struct 
		{
			DWORD m_Decay_Balance;	// more1 = For the murder flag, how much time is left ?
		} m_itEqMurderCount;
		
		// ITEM_ITEM_STONE
		struct 
		{
			ITEMID_TYPE m_ItemID;	// more1 = generate this item or template.
			DWORD m_Plot1;			// more2 = Flags to set in the plot.
		} m_itItemStone;

		// ITEM_EQ_STUCK
		struct 
		{
			DWORD m_junk1;
			DWORD m_junk2;
			CPointBase m_p;	// morep = point we are standing on when stuck.
		} m_itEqStuck;

		// ITEM_WEB
		struct 
		{
			DWORD m_Hits_Cur;	// more1 = how much damage the web can take.
		} m_itWeb;

		// ITEM_TRAP
		// ITEM_TRAP_ACTIVE
		// ITEM_TRAP_INACTIVE
		struct
		{
			ITEMID_TYPE m_AnimID;	// more1 = What does a trap do when triggered. 0=just use the next id.
			int	m_Damage;			// more2 = Base damage for a trap.
			WORD m_wAnimSec;		// morex = How long to animate as a dangerous trap.
			WORD m_wResetSec;		// morey = How long to sit idle til reset.
			BYTE m_fPeriodic;		// morez = Does the trap just cycle from active to inactive ?
		} m_itTrap;

		// ITEM_ANIM_ACTIVE
		struct
		{
			// NOTE: This is slightly dangerous to use as it will overwrite more1 and more2
			ITEMID_TYPE m_PrevID;	// more1 = What to turn back into after the animation.
			ITEM_TYPE m_PrevType;	// more2 = Any item that will animate.	??? Get rid of this !!
		} m_itAnim;

		// ITEM_SWITCH
		struct 
		{
			ITEMID_TYPE m_SwitchID;	// more1 = the next state of this switch.
			DWORD		m_junk2;
			WORD		m_fStep;	// morex = can we just step on this to activate ?
			// uidLink = the item to use when this item is thrown or used.
		} m_itSwitch;

		// ITEM_SOUND
		struct
		{
			DWORD	m_Sound;	// more1 = SOUND_TYPE
			int		m_Repeat;	// more2 = 
		} m_itSound;

		// ITEM_STONE_GUILD
		// ITEM_STONE_TOWN
		// ITEM_STONE_ROOM
		struct
		{
			STONEALIGN_TYPE m_iAlign;	// Neutral, chaos, order.
			// ATTR_OWNED = auto promote to member.
		} m_itStone;

	};	// ITEM_QTY


protected:
	CItem( ITEMID_TYPE id, CItemBase * pDef );	// only created via CreateBase()
	bool SetBase( CItemBase * pDef );
	virtual int FixWeirdness();

public:
	virtual ~CItem();
	virtual bool OnTick();

	ITEMID_TYPE GetID() const
	{
		return( m_pDef->GetID());
	}
	WORD GetBaseID() const
	{
		return( GetID());
	}
	bool SetBaseID( ITEMID_TYPE id );
	ITEMID_TYPE GetDispID() const
	{
		return( m_id );
	}
	bool IsSameDispID( ITEMID_TYPE id ) const	// account for flipped types ?
	{
		return( m_pDef->IsSameDispID( id ));
	}
	bool SetDispID( ITEMID_TYPE id );
	void SetAnim( ITEMID_TYPE id, int iTime );

	CBaseBase * GetBase() const
	{
		return( m_pDef );
	}
	int IsWeird() const;
	void SetDisconnected();

	bool IsAttr( WORD wAttr ) const	// ATTR_DECAY
	{
		return(( m_Attr & wAttr ) ? true : false );
	}

	int  GetDecayTime() const;
	void SetDecayTime( int iTime = 0 );
	SOUND_TYPE GetDropSound() const;
	bool IsMovableType() const
	{
		if ( IsAttr( ATTR_MOVE_NEVER ))
			return( false );
		if ( IsAttr( ATTR_MOVE_ALWAYS ))
			return( true );
		if ( ! m_pDef->IsMovableType() ||
			m_pDef->m_type == ITEM_WATER ||
			m_pDef->Can( CAN_I_WATER ))
			return( false );
		return( true );
	}
	bool IsMovable() const
	{
		if ( ! IsTopLevel())
			return( true );
		return( IsMovableType());
	}
	int GetVisualRange() const	// virtual
	{
		if ( GetDispID() >= ITEMID_MULTI )
			return( UO_MAP_VIEW_RADAR );
		return( UO_MAP_VIEW_SIZE );
	}

	bool  IsStackableException() const;
	bool  IsStackable( const CItem * pItem ) const;
	bool  IsSameType( const CObjBase * pObj ) const;
	bool  Stack( CItem * pItem );

	CREID_TYPE GetCorpseType() const
	{
		return( (CREID_TYPE) GetAmount());	// What does the corpse look like ?
	}
	void  SetCorpseType( CREID_TYPE id )
	{
		DEBUG_CHECK( GetDispID() == ITEMID_CORPSE );
		m_amount = id;	// m_corpse_DispID
	}
	void  SetAmount( int amount );
	void  SetAmountUpdate( int amount );
	int	  GetAmount() const { return( m_amount ); }

	const TCHAR * GetName() const;	// allowed to be default name.
	const TCHAR * GetArticleAndSpace() const;
	TCHAR * GetNameFull( bool fIdentified ) const;

	virtual bool SetName( const TCHAR * pszName );

	virtual int GetWeight() const
	{
		int iWeight = m_pDef->GetWeight() * GetAmount();
		DEBUG_CHECK( iWeight >= 0 );
		return( iWeight );
	}
	int GetBasePrice( bool fBuyFromVendor ) const;
	int GetVendorPrice( bool fBuyFromVendor ) const;

	bool  IsValidSaleItem( bool fBuyFromVendor ) const;
	virtual bool  IsValidNPCSaleItem() const;

	void SetTimeout( int iDelay );
	virtual bool MoveTo( CPointMap pt ); // Put item on the ground here.

	void MoveToDecay( CPointMap pt );
	bool MoveToCheck( CPointMap pt );
	void MoveNearObj( const CObjBaseTemplate * pItem, int iSteps = 0, WORD wCan = CAN_C_WALK );

	CItem* GetNext() const
	{
		return( STATIC_CAST <CItem*>( CObjBase::GetNext()));
	}
	CObjBase * GetContainer() const
	{
		// What is this CItem contained in ?
		// Container should be a CChar or CItemContainer
		return( dynamic_cast <CObjBase*> (GetParent()));
	}
	CObjBaseTemplate * GetTopLevelObj( void ) const
	{
		// recursively get the item that is at "top" level.
		CObjBase * pObj = GetContainer();
		if ( pObj == NULL ) 
			return( (CItem*) this ); // cast away const
		return( pObj->GetTopLevelObj());
	}

	void  Update( const CClient * pClientExclude = NULL );		// send this new item to everyone.
	void  Flip( const TCHAR * pCmd = NULL );
	bool  LoadSetContainer( CObjUID uid, LAYER_TYPE layer );

	void WriteUOX( CScript & s, int index );

	virtual bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc );
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( const TCHAR * pKey, CGString & s, CTextConsole * pSrc );
	bool  r_LoadVal( CScript & s  );
	bool  r_Load( CScript & s ); // Load an item from script
	bool  r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script

private:
	bool OnTrigger( const TCHAR * pTrigName, CTextConsole * pSrc, int iArg = 0 );
public:
	bool OnTrigger( ITRIG_TYPE trigger, CTextConsole * pSrc, int iArg = 0 )
	{
		ASSERT( trigger < ITRIG_QTY );
		return( OnTrigger( sm_szTrigName[trigger], pSrc, iArg ));
	}
	void Emote( const TCHAR * pText, CChar * pCharSrc = NULL );

	void SetBlessCurse( int iLevel, CChar * pCharSrc );

	// Item type specific stuff.
	bool IsType( ITEM_TYPE type ) const
	{
		return( m_type == type );
	}
	bool IsValidLockUID() const;
	bool IsKeyLockFit( DWORD dwLockUID ) const
	{
		DEBUG_CHECK( m_type == ITEM_KEY );
		if (( m_itKey.m_lockUID & UID_MASK ) == UID_MASK )	// master key.
			return( true );
		return( m_itKey.m_lockUID == dwLockUID );
	}

	void ConvertBolttoCloth();
	SPELL_TYPE GetScrollSpell() const;
	bool IsSpellInBook( SPELL_TYPE spell ) const;
	int  AddSpellbookScroll( CItem * pItem );

	bool IsDoorOpen() const;
	bool Use_Door( bool fJustOpen );
	bool Use_Portculis();
	SOUND_TYPE Use_Music( bool fWell ) const;

	void SetSwitchState();
	void SetTrapState( ITEM_TYPE state, ITEMID_TYPE id, int iTimeSec );
	int Use_Trap();
	bool Use_Light();
	ITEMID_TYPE Use_Cook() const
	{
		DEBUG_CHECK( m_type == ITEM_FOOD_RAW );
		if ( m_itFoodRaw.m_cook_id )
			return( m_itFoodRaw.m_cook_id );
		return CItemBase::IsRawFood( GetDispID());
	}
	bool Use_LockPick( CChar * pCharSrc );
	const TCHAR * Use_SpyGlass( CChar * pUser ) const;
	const TCHAR * Use_Sextant( CPointMap pntCoords ) const;

	bool IsBookWritable() const
	{
		DEBUG_CHECK( m_type == ITEM_BOOK || m_type == ITEM_MESSAGE || m_type == ITEM_EQ_NPC_SCRIPT );
		return( ! m_itBook.m_TimeID );
	}
	bool IsBookSystem() const	// stored in BOOK.SCP
	{
		DEBUG_CHECK( m_type == ITEM_BOOK || m_type == ITEM_MESSAGE || m_type == ITEM_EQ_NPC_SCRIPT );
		return( m_itBook.m_TimeID && ! ( m_itBook.m_TimeID & 0x80000000 ));
	}

	bool OnExplosion();
	bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel );
	int OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType = DAMAGE_HIT );

	bool IsArmor() const
	{
		return( m_type == ITEM_CLOTHING || m_type == ITEM_ARMOR );
	}
	bool IsWeapon() const
	{
		// NOTE: a wand can be a weapon.
		switch( m_type )
		{
		case ITEM_WEAPON_MACE_STAFF:
		case ITEM_WEAPON_MACE_CROOK:
		case ITEM_WEAPON_MACE_PICK:
			return( true );	// sort of armor.
		}
		return( m_type > ITEM_ARMOR && m_type <= ITEM_WAND );
	}
	bool IsArmorWeapon() const
	{
		// Armor or weapon.
		switch( m_type )
		{
		case ITEM_CLOTHING:
		case ITEM_WEAPON_MACE_STAFF:
		case ITEM_WEAPON_MACE_CROOK:
		case ITEM_WEAPON_MACE_PICK:
			return( true );	// sort of armor.
		}
		return( m_type >= ITEM_ARMOR && m_type <= ITEM_WAND );
	}
	int Armor_GetRepairPercent() const;
	const TCHAR * Armor_GetRepairDesc() const;
	bool Armor_IsXBow() const
	{
		// else its a bow and arrow.
		return( GetDispID() != ITEMID_BOW1 && GetDispID() != ITEMID_BOW2 );
	}
	bool Armor_IsRepairable() const;
	int Armor_GetDefense() const;
	int Weapon_GetAttack() const;
	SKILL_TYPE Weapon_GetSkill() const;

	void Spawn_OnTick( bool fExec );
	void Spawn_KillChildren();
	CCharBase * Spawn_SetTrackID();
	int Spawn_GetName( TCHAR * pszOut ) const;

	bool IsMemoryTypes( WORD wType ) const
	{
		// MEMORY_FIGHT
		if ( m_type != ITEM_EQ_MEMORY_OBJ )
			return( false );
		return(( GetColorAlt() & wType ) ? true : false );
	}

	bool Ship_Plank( bool fOpen );

	bool Plant_OnTick();
	void Plant_CropReset();

	virtual void DupeCopy( const CItem * pItem );
	CItem * CreateDupeSplit( int amount );

	static CItem * CreateBase( ITEMID_TYPE id );
	static CItem * CreateScript( ITEMID_TYPE id );
	static CItem * CreateDupeItem( const CItem * pItem );
	static const TCHAR * sm_TemplateTable[];
	static CItem * CreateTemplate( ITEMID_TYPE id, CObjBase* pCont = NULL );
	static CItem * CreateHeader( ITEMID_TYPE id, TCHAR * pArg, CObjBase * pCont = NULL );
};

class CItemVendable : public CItem
{
	// Any item that can be sold and has value.
protected:
	DECLARE_MEM_DYNAMIC;

private:
	WORD	m_quality;		// 0-100 quality.

	bool	m_bBuyFixed;
	bool	m_bSellFixed;
	DWORD	m_buyprice;
	DWORD	m_sellprice;

public:
	CItemVendable( ITEMID_TYPE id, CItemBase * pDef );
	~CItemVendable();

	void	SetBuyPrice(DWORD dwPrice = 0);
	DWORD	GetBuyPrice() const;
	void	SetSellPrice(DWORD dwPrice = 0);
	DWORD	GetSellPrice() const;

	WORD	GetQuality() const {return m_quality;}
	void	SetQuality(WORD quality = 0)
	{
		DEBUG_CHECK( quality <= 200 );
		m_quality = quality;
	}

	virtual void DupeCopy( const CItem * pItem );

	void	Restock();
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool  IsValidNPCSaleItem() const;
	bool  r_LoadVal( CScript & s  );
	static const TCHAR * sm_KeyTable[];
};

class CContainer : public CGObList	// This class contains a list of items but may or may not be an item itself.
{
private:
	int	m_totalweight;	// weight of all the items it has. (1/WEIGHT_UNITS pound)
private:
	void InsertAfter( CGObListRec * pNewRec, CGObListRec * pPrev = NULL )	// override this virtual function.
	{
		ASSERT(0);//ContentAdd( dynamic_cast <CItem*>( pNewRec ));
	}
protected:
	virtual void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
	void ContentAddPrivate( CItem * pItem );
public:
	CContainer( void )
	{
		m_totalweight = 0;
	}
	~CContainer()
	{
		DeleteAll(); // call this early so the virtuals will work.
	}

	CItem * GetAt( int index ) const
	{
		return( dynamic_cast <CItem*>( CGObList::GetAt( index )));
	}
	int	GetTotalWeight( void ) const
	{
		DEBUG_CHECK( m_totalweight >= 0 );
		return( m_totalweight );
	}
	int FixWeight();
	CItem* GetContentHead() const
	{
		return( STATIC_CAST <CItem*>( GetHead()));
	}

	CItem * ContentFind( ITEMID_TYPE id, int iDecendLevels = 255 ) const;
	CItem * ContentFindType( ITEM_TYPE iType, DWORD dwArg = 0, int iDecendLevels = 255 ) const;
	bool ContentFindKeyFor( CItem * pLocked ) const;
	// bool IsItemInContainer( CItem * pItem ) const;

	CItem * ContentFindRandom( void ) const;
	void ContentsDump( const CPointMap & pt );
	void ContentsTransfer( CItemContainer * pCont, bool fNoNewbie );

	// For resource usage and gold.
	int ContentCount( ITEMID_TYPE id );
	int ContentCountAll() const;
	int ContentConsume( ITEMID_TYPE id, int amount = 1, bool fTest = false, COLOR_TYPE color = COLOR_DEFAULT );
	int ResourceConsume( TCHAR * pResource, int amount = 1, bool fTest = false, COLOR_TYPE color = COLOR_DEFAULT );

	void WriteContent( CScript & s ) const;
	virtual void OnWeightChange( int iChange );
	virtual void ContentAdd( CItem * pItem ) = 0;
};

class CItemContainer : public CItemVendable, public CContainer
{
	// This item has other items inside it.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	// bool m_fTinkerTrapped;	// magic trap is diff.
protected:
	void DeletePrepare()
	{
		if ( m_type == ITEM_EQ_TRADE_WINDOW )
		{
			Trade_Delete();
		}
		CItem::DeletePrepare();
	}
public:
	CItemContainer( ITEMID_TYPE id, CItemBase * pDef );
	virtual ~CItemContainer()
	{
		DeleteAll();	// get rid of my contents first to protect against weight calc errors.
		DeletePrepare();
	}
public:
	bool IsWeighed() const
	{
		if ( GetID() == ITEMID_BANK_BOX ) 
			return false;
		if ( GetID() == ITEMID_VENDOR_BOX )
			return false;
		if ( IsAttr(ATTR_MAGIC))
			return false;
		return( true );
	}
	bool IsSearchable() const
	{
		if ( GetID() == ITEMID_BANK_BOX ) 
			return( false );
		if ( GetID() == ITEMID_VENDOR_BOX ) 
			return( false );
		if ( m_type == ITEM_CONTAINER_LOCKED )
			return( false );
		return( true );
	}

	bool IsItemInContainer(const CItem * pItem) const;
	bool CanContainerHold(const CItem * pItem, const CChar * pCharMsg ) const;

	bool r_Verb( CScript & s, CTextConsole * pSrc );
	void  r_Write( CScript & s )
	{
		CItem::r_Write(s);
		WriteContent(s);
	}
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc );

	virtual int GetWeight( void ) const
	{	// true weight == container item + contents.
		return( CItem::GetWeight() + CContainer::GetTotalWeight());
	}
	void OnWeightChange( int iChange );

	void ContentAdd( CItem * pItem );
	void ContentAdd( CItem * pItem, CPointMap pt );
protected:
	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
public:
	void Trade_Status( bool fCheck );
	void Trade_Delete();

	void MakeKey();
	void SetKeyRing();
	void Game_Create();
	void Restock();
	bool OnTick();

	int GetRestockTimeSeconds() const
	{
		// Vendor or Bank boxes.
		return( GetColorAlt());
	}
	void SetRestockTimeSeconds( int iSeconds )
	{
		SetColorAlt(iSeconds);
	}

	void DupeCopy( const CItem * pItem );

	CPointMap GetRandContainerLoc() const;
};

class CItemCorpse : public CItemContainer
{
	// A corpse is a special type of item.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CItemCorpse( ITEMID_TYPE id, CItemBase * pDef ) :
		CItemContainer( id, pDef )
	{
	}
	virtual ~CItemCorpse()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}
public:
	CChar * IsCorpseSleeping() const;

	int GetWeight( void ) const
	{	
		// GetAmount is messed up.
		// true weight == container item + contents.
		return( 1 + CContainer::GetTotalWeight());
	}
};

class CItemMulti : public CItem
{
	// A ship or house etc.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	void Multi_Delete();
	void MultiRemoveRegion();
	bool MultiAddRegion();
	CItem * Multi_FindItem( ITEM_TYPE type );
	bool Multi_IsPartOf( const CItem * pItem ) const;
	const CItemBaseMulti * Multi_GetDef() const
	{
		return( dynamic_cast <const CItemBaseMulti *> (m_pDef));
	}
	bool Multi_CreateComponent( ITEMID_TYPE id, int dx, int dy, int dz );

	CItem * Ship_GetTiller();
	int Ship_GetFaceOffset() const
	{
		return( GetID() & 3 );
	}
	int  Ship_ListObjs( CObjBase ** ppObjList, const CRegionWorld * pRegion  );
	bool Ship_CanMoveTo( const CPointMap & pt ) const;
	bool Ship_SetMoveDir( DIR_TYPE dir );
	bool Ship_MoveDelta( CPointBase pdelta, const CRegionWorld * pRegion );
	void Ship_Face( DIR_TYPE dir, CRegionWorld * pArea );
	bool Ship_OnMoveTick( void );

public:
	CItemMulti( ITEMID_TYPE id, CItemBase * pDef ) :	// CItemBaseMulti
		CItem( id, pDef )
	{
	}
	virtual ~CItemMulti();

public:
	bool MoveTo( CPointMap pt ); // Put item on the ground here.
	bool OnTick();

	void Multi_Create( CChar * pChar );
	static const CItemBaseMulti * Multi_GetDef( ITEMID_TYPE id );

	void Ship_Stop( void );
	bool Ship_Command( const TCHAR * pszCmd, CChar * pChar );
};

class CItemMemory : public CItem
{
	// Allow extra tags for the memory
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CItemMemory( ITEMID_TYPE id, CItemBase * pDef ) :
		CItem( ITEMID_MEMORY, pDef )
	{
	}
	virtual ~CItemMemory()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}
	WORD SetMemoryTypes( WORD wType )	// For memory type objects.
	{
		DEBUG_CHECK( m_type == ITEM_EQ_MEMORY_OBJ );
		SetColorAlt( wType );
		return( wType );
	}
	WORD GetMemoryTypes() const
	{
		DEBUG_CHECK( m_type == ITEM_EQ_MEMORY_OBJ );
		return( GetColorAlt());	// MEMORY_FIGHT
	}
	virtual int FixWeirdness();
};

struct CMapPinRec // Pin on a map
{
	short m_x;
	short m_y;
public:
	CMapPinRec() {}
	CMapPinRec( short x, short y ) { m_x = x; m_y = y; }
};

class CItemMap : public CItemVendable
{
protected:
	DECLARE_MEM_DYNAMIC;
public:
	enum
    {
        MAX_PINS = 128,
    };

	bool m_fPlotMode;	// should really be per-client based but oh well.
	CGTypedArray<CMapPinRec,CMapPinRec&> m_Pins;

public:
	CItemMap( ITEMID_TYPE id, CItemBase * pDef ) : CItemVendable( id, pDef )
	{
	}
	virtual ~CItemMap()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}
public:
	void r_Write( CScript & s );
	bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc = NULL );
	bool r_LoadVal( CScript & s );
	void DupeCopy( const CItem * pItem );
};

enum STONEPRIV_TYPE // Priv level for this char
{
	STONEPRIV_CANDIDATE = 0,
	STONEPRIV_MEMBER,
	STONEPRIV_MASTER,
	STONEPRIV_BANISHED,	// This char is banished from the town/guild.
	STONEPRIV_ENEMY = 100,	// this is an enemy town/guild.
};

class CStoneMember : public CGObListRec	// Members for various stones, and links to stones at war with
{
	// NOTE: Chars are linked to the CItemStone via a memory object.
	friend class CItemStone;
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CObjUID m_LinkUID;			// My char uid or enemy stone UID

	STONEPRIV_TYPE m_iPriv;	// What is my status level in the guild ?
	CObjUID m_LoyalToUID;	// Who am i loyal to? invalid value = myself.
	CGString m_sTitle;		// What is my title in the guild?

	union	// Depends on m_iPriv
	{
		struct	// Unknown type.
		{
			int m_Val1;
			int m_Val2;
		} m_UnDef;

		struct // STONEPRIV_ENEMY
		{
			int m_fTheyDeclared;	
			int m_fWeDeclared;	
		} m_Enemy;

		struct	// a char member (NOT STONEPRIV_ENEMY)
		{
			int m_fAbbrev;			
			int m_iVoteTally;		// Temporary space to calculate votes.
		} m_Member;
	};

public:
	CStoneMember( CItemStone * pStone, CObjUID uid, STONEPRIV_TYPE iType, const TCHAR * pTitle = "", CObjUID loyaluidLink = 0, bool fArg1 = false, bool fArg2 = false );
	virtual ~CStoneMember();
	CStoneMember* GetNext() const
	{
		return( STATIC_CAST <CStoneMember *>( CGObListRec::GetNext()));
	}

	CObjUID GetLinkUID() const { return m_LinkUID; }

	STONEPRIV_TYPE GetPriv() const { return m_iPriv; }
	void SetPriv(STONEPRIV_TYPE iPriv) { m_iPriv = iPriv; }
	bool IsMaster() const { return m_iPriv == STONEPRIV_MASTER; }

	// If the member is really a war flag (STONEPRIV_ENEMY)
	void SetWeDeclared(bool f)
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		m_Enemy.m_fWeDeclared = f;
	}
	bool GetWeDeclared() const 
	{ 
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		return m_Enemy.m_fWeDeclared; 
	}
	void SetTheyDeclared(bool f)
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		m_Enemy.m_fTheyDeclared = f;
	}
	bool GetTheyDeclared() const 
	{ 
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		return m_Enemy.m_fTheyDeclared; 
	}

	// Member
	bool IsAbbrevOn() const { return m_Member.m_fAbbrev; }
	void ToggleAbbrev() { m_Member.m_fAbbrev = !m_Member.m_fAbbrev; }

	const TCHAR * GetTitle() const
	{
		return( m_sTitle );
	}
	void SetTitle( const TCHAR * pTitle )
	{
		m_sTitle = pTitle;
	}

	CObjUID GetLoyalTo() const
	{
		return( m_LoyalToUID );
	}
	bool SetLoyalTo( const CChar * pChar);
};

class CItemStone : public CItem, public CGObList
{
	// ITEM_STONE_GUILD
	// ITEM_STONE_TOWN
	// ATTR_OWNED = auto promote to member.

	friend class CStoneMember;
	static const TCHAR * sm_KeyTable[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sCharter[6];
	CGString m_sWebPageURL;
	CGString m_sAbbrev;

private:

	const TCHAR * GetTypeName() const;
	void SetTownName();
	bool SetName( const TCHAR * pszName );
	bool MoveTo( CPointMap pt );

	MEMORY_TYPE GetMemoryType() const
	{
		switch ( m_type )
		{
		case ITEM_STONE_TOWN: return( MEMORY_TOWN );
		case ITEM_STONE_GUILD: return( MEMORY_GUILD );
		}
		// Houses have no memories.
		return( MEMORY_NONE );
	}
	bool IsStoneMenuMember( STONEGUMP_TYPE iStoneMenu, const CItemStone * pOtherStone ) const;
	bool IsStoneMenuMember( STONEGUMP_TYPE iStoneMenu, const CStoneMember * pMember ) const;

	const TCHAR * GetCharter(int iLine) const { ASSERT(iLine<COUNTOF(m_sCharter)); return(  m_sCharter[iLine] ); }
	void SetCharter( int iLine, const TCHAR * pCharter ) { ASSERT(iLine<COUNTOF(m_sCharter)); m_sCharter[iLine] = pCharter; }
	const TCHAR * GetWebPageURL() const { return( m_sWebPageURL ); }
	void SetWebPage( const TCHAR * pWebPage ) { m_sWebPageURL = pWebPage; }

	// Client interaction.
	int  addStoneGumpListSetup( STONEGUMP_TYPE iStoneMenu, CGString * psText, int iTexts );
	void addStoneGumpList( CClient * pClient, STONEGUMP_TYPE iStoneMenu );
	void addStoneSetViewCharter( CClient * pClient, STONEGUMP_TYPE iStoneMenu );
	void addStoneGump( CClient * pClient, STONEGUMP_TYPE menuid );

	void SetupEntryGump( CClient * pClient );
	void SetupMasterGump( CClient * pClient );

	void ElectMaster();
	CStoneMember * PromoteRecruit( const CChar * pChar );
public:
	CStoneMember * AddRecruit(const CChar * pChar);

	// War
private:
	void TheyDeclarePeace( CItemStone* pEnemyStone, bool fForcePeace );
	bool WeDeclareWar(CItemStone * pEnemyStone);
	void WeDeclarePeace(CObjUID uidEnemy, bool fForcePeace = false);
	void AnnounceWar( const CItemStone * pEnemyStone, bool fWeDeclare, bool fWar );
public:
	bool IsAtWarWith( const CItemStone * pStone ) const;
	bool IsSameAlignType( const CItemStone * pStone) const
	{
		if ( pStone == NULL ) 
			return( false );
		return( GetAlignType() != STONEALIGN_STANDARD &&
			GetAlignType() == pStone->GetAlignType());
	}

	bool CheckValidMember(CStoneMember * pMember);
	int FixWeirdness();

public:
	CItemStone( ITEMID_TYPE id, CItemBase * pDef );
	virtual ~CItemStone();

	void r_Write( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & s, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s );
	bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script

	static bool IsUniqueName( const TCHAR * pName );
	CChar * GetMaster() const;
	bool NoMembers() const;
	CStoneMember * GetMasterMember() const;
	CStoneMember * GetMember( const CObjBase * pObj) const;

	// Simple accessors.
	STONEALIGN_TYPE GetAlignType() const { return m_itStone.m_iAlign; }
	void SetAlignType(STONEALIGN_TYPE iAlign) { m_itStone.m_iAlign = iAlign; }
	const TCHAR * GetAlignName() const;
	const TCHAR * GetAbbrev() const { return( m_sAbbrev ); }
	void SetAbbrev( const TCHAR * pAbbrev ) { m_sAbbrev = pAbbrev; }

	bool OnPromptResp( CClient * pClient, TARGMODE_TYPE TargMode, const TCHAR * pszText, CGString & sMsg );
	bool OnGumpButton( CClient * pClient, STONEGUMP_TYPE type, DWORD * pCheckID, int iCheckQty, CGString * psText, WORD * piTextID, int iTextQty );
	void Use_Item( CClient * pClient );
};

class CItemMessage : public CItemVendable	// A message for a bboard or book text.
{
	// ITEM_BOOK, ITEM_MESSAGE, ITEM_EQ_NPC_SCRIPT = can be written into.
	// the name is the title for the message. (ITEMID_BBOARD_MSG)
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGObArray<CGString*> m_sBodyLines;	// The main body of the text for bboard message or book.
public:
	CGString m_sAuthor;					// Should just have author name !
public:
	CItemMessage( ITEMID_TYPE id, CItemBase * pDef ) : CItemVendable( id, pDef )
	{
	}
	virtual ~CItemMessage()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
		UnLoadSystemPages();
	}

	WORD GetScriptTimeout() const
	{
		return( GetColorAlt());
	}
	void SetScriptTimeout( WORD wTimeInTenths )
	{
		SetColorAlt( wTimeInTenths );
	}

	void r_Write( CScript & s );
	bool r_LoadVal( CScript & s );
	bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script

	int GetPageCount() const
	{
		return( m_sBodyLines.GetCount());
	}
	const TCHAR * GetPageText( int iPage ) const
	{
		if ( iPage >= GetPageCount())
			return( NULL );
		if ( m_sBodyLines[iPage] == NULL )
			return( NULL );
		return( * m_sBodyLines[iPage] );
	}
	void SetPageText( int iPage, const TCHAR * pszText )
	{
		if ( iPage < m_sBodyLines.GetCount() && m_sBodyLines[ iPage ] )
		{
			m_sBodyLines.RemoveAt( iPage );
		}
		if ( pszText == NULL )
		{
			return;
		}
		m_sBodyLines.SetAtGrow( iPage, new CGString( pszText ));
	}
	void AddPageText( const TCHAR * pszText )
	{
		m_sBodyLines.Add( new CGString( pszText ));
	}

	void DupeCopy( const CItem * pItem );
	bool LoadSystemPages();
	void UnLoadSystemPages()
	{
		m_sAuthor.Empty();
		m_sBodyLines.RemoveAll();
	}
};

class CFragmentFile : public CScript, public CMemDynamic	
{
	// A script file with speech, motives or events handlers.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	int m_iRefs;	// How many CFragmentDef(s) use this ?
public:
	void IncFragRef()
	{
		m_iRefs++;
	}
	void DecFragRef()
	{
		m_iRefs--;
	}
	int GetFragRefs() const
	{
		return m_iRefs;
	}
	CFragmentFile( const TCHAR * pszName )
	{
		m_iRefs = 0;
		SetFilePath( pszName );
	}
};

class CFragmentDef : public CMemDynamic	// Hold convo fragment info
{
	// Hold a fragment of "knowledge" for an NPC.
	// A single event, motive or speech handler block.
protected:
	DECLARE_MEM_DYNAMIC;

private:
	CGString m_sName;	// friendly name. or Alias. default is "file:index"
	CScriptLink m_ScriptLink;	// pre-indexed link into the script file.
	WORD m_wIndex;	// This is really an index into *SPEE.SCP

private:
	CFragmentDef( const TCHAR * pName, const TCHAR * pExtra );

public:
	bool IsDefFile() const
	{
		return( m_wIndex != 0 );
	}
	WORD GetDefIndex() const
	{
		return( m_wIndex );
	}
	void ClearFragLink()
	{
		m_ScriptLink.ClearLinkOffset();
	}
	static CFragmentDef * FindFragName( const TCHAR * pszName, bool fAdd );

	const TCHAR * GetFragName() const;
	bool OpenFrag( CScriptLock & s );
};

class CFragmentArray : public CGPtrTypeArray<CFragmentDef*>
{
	// What script sections does this CChar want to use ?
	// An indexed list of CFragmentDef s.
private:
	const TCHAR * GetFragName( int iIndex ) const
	{
		// look up the name of the fragment given it's index.
		const CFragmentDef * pFragDef = GetAt( iIndex );
		ASSERT(pFragDef);
		return( pFragDef->GetFragName());
	}
public:
	bool IsFragIn( const TCHAR * pszKey ) const;
	void WriteFragList( CGString & sVal ) const;
	bool r_LoadVal( CScript & s );
	void r_Write( CScript & s, const TCHAR * pszKey ) const;
};

enum NPCBRAIN_TYPE	// General AI type.
{
	NPCBRAIN_NONE = 0,	// 0 = This should never really happen.
	NPCBRAIN_ANIMAL,	// 1 = can be tamed.
	NPCBRAIN_HUMAN,		// 2 = generic human.
	NPCBRAIN_HEALER,	// 3 = can res.
	NPCBRAIN_GUARD,		// 4 = inside cities
	NPCBRAIN_BANKER,	// 5 = can open your bank box for you
	NPCBRAIN_VENDOR,	// 6 = will sell from vendor boxes.
	NPCBRAIN_BEGGAR,	// 7 = begs.
	NPCBRAIN_STABLE,	// 8 = will store your animals for you.
	NPCBRAIN_THEIF,		// 9 = should try to steal ?
	NPCBRAIN_MONSTER,	// 10 = not tamable. normally evil.
	NPCBRAIN_BESERK,	// 11 = attack closest (blades, vortex)
	NPCBRAIN_UNDEAD,	// 12 = disapears in the light.
	NPCBRAIN_DRAGON,	// 13 = we can breath fire. may be tamable ? hirable ?
	NPCBRAIN_VENDOR_OFFDUTY,	// 14 = "Sorry i'm not working right now. come back when my shop is open.
	NPCBRAIN_CRIER,		// 15 = will speak periodically.
	NPCBRAIN_CONJURED,	// 16 = elemental or other conjured creature.
	NPCBRAIN_QTY,
};

struct CCharBaseDisp
{
	CREID_TYPE m_dispID;
	ITEMID_TYPE m_trackID;	// For looking up spawn items.
	WORD  m_Can;	// ??? Move this to GRAYCHAR.SCP
public:
	static CREID_TYPE GetRandBase();
	static CREID_TYPE FindCharTrack( ITEMID_TYPE trackID );
};

class CCharBase : public CBaseBase // define basic info about each "TYPE" of monster/creature.
{
protected:
	DECLARE_MEM_DYNAMIC;
	// This is to be used in hand with graychar.scp file.
private:
	int m_iHireDayWage;

public:
	ITEMID_TYPE m_trackID;	// ITEMID_TYPE what look like on tracking.
	SOUND_TYPE m_soundbase;	// sounds ( typically 5 sounds per creature, humans and birds have more.)

	CGString m_sCorpseResources;	//RESOURCES=10 MEAT
	CGString m_sFoodType; // FOODTYPE=MEAT 15 (3)

	//DESIRES=GOLD (C), OGRES (C)
	CGString m_sDesires;	// Desires that are typical for the char class. see also m_sNeed

	BYTE  m_defense;	// base defense. can be modified by armor.
	DWORD m_Anims;	// Bitmask of animations available for monsters. ANIM_TYPE

	short m_MaxFood;	// Derived from foodtype...this is the max amount of food we can eat.
	short m_Str;	// Base Str for type. (in case of polymorph)
	short m_Dex;

	//SHELTER=FORESTS (P), MOUNTAINS (P)
	//AVERSIONS=TRAPS, CIVILIZATION

	// If this is an NPC.
	// We respond to what we here with this.
	CFragmentArray m_Speech;	// Speech fragment list (other stuff we know)

	// When events happen to the char. check here for reaction.
	CFragmentArray m_Events;	// Action or motivation type indexes.

	static const TCHAR * sm_KeyTable[];

private:
	void SetFoodType( const TCHAR * pszFood );
	void DupeCopy( const CCharBase * pDef );
	bool Load();

protected:
	SCPFILE_TYPE GetScpFileIndex( bool fSecondary ) const
	{
		return( fSecondary ? SCPFILE_CHAR_2 : SCPFILE_CHAR );
	}
	CBaseBaseArray * GetBaseArray() const;

	virtual void UnLoad() 
	{ 
		m_Speech.RemoveAll();
		m_Events.RemoveAll();
		CBaseBase::UnLoad(); 
	}

public:

	CCharBase( CREID_TYPE id );
	~CCharBase() {}

	CREID_TYPE GetID() const { return((CREID_TYPE) GetBaseID()); }
	CREID_TYPE GetDispID() const { return((CREID_TYPE) m_wDispIndex ); }
	bool SetDispID( CREID_TYPE id );

	int GetHireDayWage() const { return( m_iHireDayWage ); }

	static CCharBase * FindCharBase( CREID_TYPE id );
	static bool IsFemaleID( CREID_TYPE id );
	static bool IsValidDispID( CREID_TYPE id );
	static bool IsHuman( CREID_TYPE id );

	const TCHAR * GetTradeName() const;

	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc = NULL );
};

inline bool CCharBase::IsValidDispID( CREID_TYPE id ) //  static
{
	return( id > 0 && id < CREID_QTY );
}

inline bool CCharBase::IsHuman( CREID_TYPE id ) // static
{
	return( id == CREID_MAN || id == CREID_WOMAN || id == CREID_EQUIP_GM_ROBE );
}

enum STAT_TYPE	// Character stats
{
	STAT_NONE = -1,
	STAT_STR = 0,
	STAT_INT,
	STAT_DEX,
	STAT_BASE_QTY,

	// Notoriety.
	STAT_Karma = 3,		// -10000 to 10000 - also used as the food consumption main timer.
	STAT_Fame,
	STAT_QTY,
};

struct CCharNPC : public CMemDynamic
{
	// This is basically the unique "brains" for any character.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	// Stuff that is specific to an NPC character instance (not an NPC type see CCharBase for that).
	// Any NPC AI stuff will go here.

	NPCBRAIN_TYPE m_Brain;	// For NPCs: Number of the assigned basic AI block
	WORD	m_Home_Dist_Wander;	// Distance to allow to "wander".
	BYTE    m_Act_Motivation;		// 0-100 (100=very greatly) how bad do i want to do the current action.
	CPointBase m_Act_p_Prev;	// Location before we started this action (so we can get back if necessary)

	// We respond to what we here with this.
	CFragmentArray m_Speech;	// Speech fragment list (other stuff we know)

	CGString m_sNeed;	// What items might i need/Desire ? (coded as resource scripts) ex "10 gold,20 logs" etc.

	static const TCHAR * sm_KeyTable[];

public:
	CCharNPC( NPCBRAIN_TYPE NPCBrain );

	bool r_WriteVal( const TCHAR * pKey, CGString & s );
	void r_Write( CScript & s ) const;
	bool r_LoadVal( CScript & s );

	const TCHAR * GetNeed() const
	{
		return( m_sNeed );
	}
	bool IsVendor() const
	{
		return( m_Brain == NPCBRAIN_HEALER || m_Brain == NPCBRAIN_STABLE || m_Brain == NPCBRAIN_VENDOR );
	}
};

enum PATRON_TYPE	// TBD ???
{
	PATRON_None		= 0,
	PATRON_Hecate,			// Low Chaos
	PATRON_Set,				// Famous Chaos
	PATRON_Druid,			// Low Neutral
	PATRON_Destiny,			// Famous Neutral
	PATRON_Law,				// ?Beginner Law
	PATRON_Athena,			// Famous Law
};

#define IS_SKILL_BASE(sk) ((sk)> SKILL_NONE && (sk)< SKILL_QTY )

struct CCharPlayer : public CMemDynamic
{
	// Stuff that is specific to a player character.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CAccount * const m_pAccount;	// The account index. (for idle players mostly)
	BYTE m_SkillLock[SKILL_QTY];	// SKILLLOCK_TYPE List of skill lock states for this player character
public:

	// General plot type stuff. ??? Get rid of this in favor of m_Events
	DWORD m_Plot1;	// script triggers the get item interaction events .
	DWORD m_Plot2;	// non-triggers.

	WORD m_Murders;		// Murder count.
	BYTE m_SkillClass;	// What skill class group have we selected.
	// PATRON_TYPE m_Patron;	// Religion Plot stuff.

	static const TCHAR * sm_KeyTable[];

public:
	CCharPlayer( CAccount * pAccount );

	SKILL_TYPE Skill_GetLockType( const TCHAR * pszKey ) const;

	SKILLLOCK_TYPE Skill_GetLock( SKILL_TYPE skill ) const
	{
		ASSERT( IS_SKILL_BASE(skill));
		return((SKILLLOCK_TYPE) m_SkillLock[skill] );
	}
	void Skill_SetLock( SKILL_TYPE skill, SKILLLOCK_TYPE state )
	{
		ASSERT( IS_SKILL_BASE(skill));
		m_SkillLock[skill] = state;
	}

	bool r_WriteVal( const TCHAR * pKey, CGString & s );
	void r_Write( CScript & s ) const;
	bool r_LoadVal( CScript & s );

	CAccount * GetAccount() const
	{
		ASSERT( m_pAccount );
		return( m_pAccount );
	}
};

enum WAR_SWING_TYPE	// m_Act_War_Swing_State
{
	WAR_SWING_EQUIPPING = 0,	// we are recoiling our weapon.
	WAR_SWING_READY,			// we can swing at any time.
	WAR_SWING_SWINGING,			// we are swinging our weapon.
};

enum CTRIG_TYPE
{
	CTRIG_HearGreeting,		// (NPC only) i have been spoken to for the first time. (no memory of previous hearing)
	CTRIG_HearUnknown,		//+(NPC only) I heard something i don't understand.
	CTRIG_SpellCast,		//+Char is casting a spell.
	CTRIG_SpellEffect,		//+A spell just hit me.
	CTRIG_SeeWantItem,		// (NPC only) i see something good.
	CTRIG_PersonalSpace,	//+i just got stepped on.
	CTRIG_ReceiveItem,		//+(NPC only) I was just handed an item (Not yet checked if i want it)
	CTRIG_AcceptItem,		// (NPC only) i've been given an item i like (according to DESIRES)
	CTRIG_RefuseItem,		// (NPC only) i've been given an item i don't want.
	CTRIG_SeeNewPlayer,		//+(NPC only) i see u for the first time. (in 20 minutes) (check memory time)
	CTRIG_FightSwing,		// randomly choose to speak while fighting.
	CTRIG_FearOfDeath,		// I'm not healthy.
	CTRIG_GetHit,			// I just got hit.
	CTRIG_Hit,				// I just hit someone. (TARG)
	CTRIG_Death,			//+I just got killed.

	CTRIG_QTY,
};

class CChar : public CObjBase, public CContainer, public CTextConsole
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	// Spell type effects.
#define STATF_INVUL			0x00000001	// Invulnerability
#define STATF_DEAD			0x00000002
#define STATF_Freeze		0x00000004	// Paralyzed. (spell)
#define STATF_Invisible		0x00000008	// Invisible (spell).
#define STATF_Sleeping		0x00000010	// You look like a corpse ?
#define STATF_War			0x00000020	// War mode on ?
#define STATF_Reactive		0x00000040	// have reactive armor on.
#define STATF_Poisoned		0x00000080	// Poison level is in the poison object
#define STATF_NightSight	0x00000100	// All a light to you
#define STATF_Reflection	0x00000200	// Magic reflect on.
#define STATF_Polymorph		0x00000400	// We have polymorphed to another form.
#define STATF_Incognito		0x00000800	// Dont show skill titles
#define STATF_SpiritSpeak	0x00001000	// I can hear ghosts clearly.
#define STATF_Insubstantial	0x00002000	// Ghost has not manifest. or GM hidden
#define STATF_EmoteAction	0x00004000	// The creature will emote its actions to it's owners.
#define STATF_HasShield		0x00010000	// Using a shield
#define STATF_Script_Play	0x00020000	// Playing a Script. (book script)
#define STATF_Stone			0x00040000	// turned to stone.
#define STATF_Script_Rec	0x00080000	// Recording a script.
#define STATF_Fly			0x00100000	// Flying or running ? (anim)
#define STATF_Drunk			0x00200000	// Drinking booze.
#define STATF_Hallucinating	0x00400000	// eat 'shrooms or bad food.
#define STATF_Hidden		0x00800000	// Hidden (non-magical)
#define STATF_InDoors		0x01000000	// we are covered from the rain.
#define STATF_Criminal		0x02000000	// The guards will attack me. (someone has called guards)
#define STATF_Conjured		0x04000000	// This creature is conjured and will expire. (leave no corpse or loot)
#define STATF_Pet			0x08000000	// I am a pet/hirling. check for my owner memory.
#define STATF_Spawned		0x10000000	// I am spawned by a spawn item.
#define STATF_SaveParity	0x20000000	// Has this char been saved or not ?
#define STATF_Ridden		0x40000000	// This is the horse. (don't display me) I am being ridden
#define STATF_OnHorse		0x80000000	// Mounted on horseback.

	// Plus_Luck	// bless luck or curse luck ?

	DWORD m_StatFlag;		// Flags above

#define SKILL_VARIANCE 100		// Difficulty modifier for determining success. 10.0 %
	WORD m_Skill[SKILL_QTY];	// List of skills ( skill * 10 )
	short m_Stat[STAT_QTY];		// karma is signed.

public:
	static const TCHAR * sm_KeyTable[];
	static const TCHAR * sm_szTrigName[];
	static int sm_Stat_Val_Regen[4];
	static const LAYER_TYPE sm_VendorLayers[4];

	// This is a character that can either be NPC or PC.
	CCharBase * m_pDef;		// define the type of the creature i am.
	CRegionWorld * m_pArea; // What region are we in now. (for guarded message)

	// Player vs NPC Stuff
	CClient * m_pClient;	// is the char a logged in player ?
	CCharPlayer * m_pPlayer;	// May even be an off-line player !
	CCharNPC * m_pNPC;			// we can be both a player and an NPC is "controlled" ?

	// Combat stuff. cached data. (not saved)
	CObjUID m_weapon;		// current Wielded weapon.	(could just get rid of this ?)
	BYTE m_defense;			// calculated armor worn (NOT intrinsic armor)

	// Saved stuff.
	DIR_TYPE m_face_dir;	// facing this dir.
	CGString m_sTitle;		// Special title such as "the guard" (replaces the normal skill title).
	CPointMap  m_Home;			// What is our "home" region. (towns and bounding of NPC's)
	FONT_TYPE m_fonttype;	// Speech font to use // can client set this ?

	CREID_TYPE m_prev_id;		// Backup of body type for ghosts and poly
	COLOR_TYPE m_prev_color;	// Backup of skin color. in case of polymorph etc.

	// When events happen to the char. check here for reaction.
	CFragmentArray m_Events;

	// Skills, Stats and health
#define m_StatHealth	m_StatVal[ STAT_STR ].m_val
#define m_StatMana		m_StatVal[ STAT_INT ].m_val
#define m_StatStam		m_StatVal[ STAT_DEX ].m_val
#define m_food			m_StatVal[ 3 ].m_val	// Food level (.5 days food level at normal burn)

	struct
	{
		short m_val;			// signed for karma
		unsigned short m_regen;	// Tick time since last regen.
	} m_StatVal[4];

	time_t m_time_last_regen;	// When did i get my last regen tick ?
	time_t m_time_create;		// When was i created ?

	// Some character action in progress.
	SKILL_TYPE	m_Act_SkillCurrent;		// Currently using a skill. Could be combat skill.
	CObjUID		m_Act_Targ;			// Current combat/action target
	CObjUID		m_Act_TargPrv;		// Targeted bottle for alchemy or previous beg target.
	int			m_Act_Difficulty;	// -1 = fail skill. (0-100) for skill advance calc.
	CPointBase  m_Act_p;			// Moving to this location. or location of forge we are working on.

	union	// arg specific to the action type.(m_Act_SkillCurrent)
	{
		struct
		{
			DWORD		m_Arg1;	// "ACTARG1"
			DWORD		m_Arg2;	// "ACTARG2"
			DWORD		m_Arg3;	// "ACTARG3"
		} m_atUnk;

		// SKILL_MAGERY
		// SKILL_INSCRIPTION
		struct
		{
			SPELL_TYPE	m_Spell;		// Currently casting spell.
			CREID_TYPE	m_SummonID;		// A sub arg of the skill. (summoned type ?)
		} m_atMagery;

		// SKILL_ALCHEMY
		struct
		{
			POTION_TYPE m_Potion;	// Potion being attempted.
			WORD m_Stroke_Count;
		} m_atAlchemy;

		// SKILL_BLACKSMITHING
		// SKILL_BOWCRAFT
		// SKILL_CARPENTRY
		// SKILL_LUMBERJACKING,
		// SKILL_MINING
		// SKILL_TINKERING,
		struct	// creation type skill.
		{
			ITEMID_TYPE m_ItemID;		// Making this item.
			WORD m_Stroke_Count;		// For mining, smithing, tinkering, lumberjacking, etc. all requiring multi strokes.
			WORD m_Amount;				// How many of this item are we making?
			COLOR_TYPE	m_Color;		// for creating an item. smithing color.
		} m_atCreate;

		// SKILL_TAMING
		// SKILL_MEDITATION
		struct
		{
			DWORD m_junk1;
			WORD m_Stroke_Count;		// For mining, smithing, tinkering, lumberjacking, etc. all requiring multi strokes.
		} m_atTaming;

		// SKILL_SWORDSMANSHIP
		// SKILL_MACEFIGHTING,
		// SKILL_FENCING,
		// SKILL_WRESTLING
		struct
		{
			WAR_SWING_TYPE m_War_Swing_State;	// We are in the war mode swing.
			// m_Act_Targ = who are we currently attacking?
		} m_atFight;

		// SKILL_TRACKING
		struct
		{
			DIR_TYPE	m_PrvDir; // Previous direction of tracking target, used for when to notify player
		} m_atTracking;

		// SKILL_CARTOGRAPHY
		struct
		{
			int	m_Dist;
		} m_atCartography;

		// NPCACT_FOLLOW_TARG
		struct
		{
			int	m_DistMin;		// Try to force this distance.
			int	m_DistMax;		// Try to force this distance.
			// m_Act_Targ = what am i folloiwng ? 
		} m_atFollowTarg;

		// NPCACT_RIDDEN
		struct	
		{
			CObjUIDBase m_FigurineUID;	// This creature is being ridden by this object link. ITEM_FIGURINE ITEM_EQ_HORSE
		} m_atRidden;

		// NPCACT_TALK
		// NPCACT_TALK_FOLLOW
		struct	
		{
			int	 m_HearUnknown;	// Speaking NPC has no idea what u're saying.
		} m_atTalk;

		// NPCACT_LOOTING
		// m_Act_Targ = what am i looting ? (Corpse)
	};

public:
	CChar( CREID_TYPE id );
	virtual ~CChar(); // Delete character

public:
	// Status and attributes ------------------------------------
	int IsWeird() const;
	bool IsStat( DWORD dwStatFlag ) const
	{
		return(( m_StatFlag & dwStatFlag) ? true : false );
	}
	void SetStat( DWORD dwStatFlag )
	{
		m_StatFlag |= dwStatFlag;
	}
	void ClearStat( DWORD dwStatFlag )
	{
		m_StatFlag &= ~dwStatFlag;
	}
	void ModStat( DWORD dwStatFlag, bool fMod )
	{
		if ( fMod )
			m_StatFlag |= dwStatFlag;
		else
			m_StatFlag &= ~dwStatFlag;
	}
	bool IsPriv( WORD flag ) const
	{	// PRIV_GM flags
		if ( ! IsClient()) 
			return( false );	// NPC's have no privs.
		return( m_pClient->IsPriv( flag ));
	}
	PLEVEL_TYPE GetPrivLevel() const
	{
		// The higher the better. // PLEVEL_Counsel
		if ( ! m_pPlayer )
			return( PLEVEL_Player );
		return( m_pPlayer->GetAccount()->GetPrivLevel());
	}
	CBaseBase * GetBase() const
	{
		return( m_pDef );
	}

	bool IsSpeakAsGhost() const
	{
		return( IsStat(STATF_DEAD) &&
			! IsStat( STATF_SpiritSpeak ) &&
			GetPrivLevel() < PLEVEL_Counsel );
	}
	bool CanUnderstandGhost() const
	{
		// Can i understand player ghost speak ?
		if ( m_pNPC && m_pNPC->m_Brain == NPCBRAIN_HEALER ) 
			return( true );
		return( IsStat( STATF_SpiritSpeak | STATF_DEAD ) || GetPrivLevel() >= PLEVEL_Counsel );
	}
	bool IsRespawned() const
	{
		// Can be resurrected later.
		return( m_pPlayer ||
			( m_Home.IsValid() && ! IsStat( STATF_Spawned|STATF_Conjured|STATF_Pet )));
	}
	bool IsHuman() const
	{
		return( CCharBase::IsHuman( GetDispID()));
	}
	int	 GetHealthPercent() const;
	int  GetFoodLevelPercent() const;
	const TCHAR * GetTradeTitle() const; // Paperdoll title for character p (2)

	// Information about us.
	CREID_TYPE GetID() const
	{
		ASSERT(m_pDef);
		return( m_pDef->GetID());
	}
	WORD GetBaseID() const
	{
		return( GetID());
	}
	CREID_TYPE GetDispID() const
	{
		return( m_pDef->GetDispID());
	}
	void SetID( CREID_TYPE id );

	const TCHAR * GetName() const
	{
		if ( ! IsIndividualName())			// allow some creatures to go unnamed.
			return( m_pDef->GetTypeName());	// Just use it's type name instead.
		return( CObjBase::GetName());
	}

	bool SetName( const TCHAR * pName );

	bool CanSeeTrue( const CChar * pChar = NULL ) const
	{
		if ( pChar == NULL || pChar == this ) 
			return( false );
		// if ( pChar->IsStat( STATF_Sleeping )) return( false );
		return( pChar->GetPrivLevel() >= GetPrivLevel());
	}
	bool CanSee( const CObjBaseTemplate * pObj ) const;
	bool CanSeeLOS( const CPointMap & pd, CPointMap * pBlock = NULL, int iMaxDist = UO_MAP_VIEW_SIGHT ) const;
	bool CanSeeLOS( const CObjBaseTemplate * pObj ) const;
	bool CanTouch( const CPointMap & pt ) const;
	bool CanTouch( const CObjBase * pObj ) const;
	bool CanMove( CItem * pItem, bool fMsg = true ) const;
	BYTE GetLightLevel() const;
	NPCBRAIN_TYPE GetCreatureType() const; // return 1 for animal, 2 for monster, 3 for NPC humans and PCs
	bool CanUse( CItem * pItem ) const;
	int  CanEat( const CItem * pItem ) const;
	int  CanEat( const CChar * pChar ) const;

	const TCHAR * GetFoodLevelMessage( bool fPet, bool fHappy ) const;

public:
	void Stat_Set( STAT_TYPE i, short iVal );
	short Stat_Get( STAT_TYPE i ) const
	{
		ASSERT(((WORD)i)<STAT_QTY );
		return m_Stat[i];
	}

	// Location and movement ------------------------------------
private:
	bool TeleportToCli( int iType, int iArgs );
	bool TeleportToObj( int iType, TCHAR * pszArgs );
private:
	CRegionBase * CheckValidMove( CPointBase & pt, WORD * pwBlockFlags = NULL ) const;
	bool MoveToRegion( CRegionWorld * pNewArea, bool fAllowReject );

public:
	CChar* GetNext() const
	{
		return( STATIC_CAST <CChar*>( CObjBase::GetNext()));
	}
	CObjBaseTemplate * GetTopLevelObj( void ) const
	{
		// Get the object that has a location in the world. (Ground level)
		return( (CChar*) this ); // cast away const
	}

	bool IsSwimming() const;

	bool MoveToRegionReTest( DWORD dwType )
	{
		return( MoveToRegion( dynamic_cast <CRegionWorld *>( GetTopPoint().GetRegion( dwType )), false ));
	}
	bool MoveTo( CPointMap pt );
	bool MoveToValidSpot( DIR_TYPE dir, int iDist, int iDistStart = 1 );
	void MoveNearObj( const CObjBaseTemplate * pObj, int iSteps = 0, WORD wCan = CAN_C_WALK )
	{
		CObjBase::MoveNearObj( pObj, iSteps, GetMoveBlockFlags());
	}
	void MoveNear( CPointMap pt, int iSteps = 0, WORD wCan = CAN_C_WALK )
	{
		CObjBase::MoveNear( pt, iSteps, GetMoveBlockFlags());
	}

	CRegionBase * CanMoveWalkTo( CPointBase & pt, bool fCheckChars = true );
	void CheckRevealOnMove();
	bool CheckLocation( bool fStanding = false );

public:
	// Client Player specific stuff. -------------------------
	void ClientAttach( CClient * pClient );
	void ClientDetach();
	bool IsClient() const { return( m_pClient != NULL ); }
	CClient * GetClient() const
	{
		return( m_pClient );
	}

	bool IsDND() const
	{
		return( GetPrivLevel() >= PLEVEL_Counsel && IsStat( STATF_Insubstantial ));
	}
	bool CanDisturb( CChar * pChar ) const;
	void SetDisconnected();
	bool SetPlayerAccount( CAccount * pAccount );
	bool SetPlayerAccount( const TCHAR * pszAccount );
	void SetNPCBrain( NPCBRAIN_TYPE NPCBrain );

public:
	void ObjMessage( const TCHAR * pMsg, const CObjBase * pSrc ) const
	{
		if ( ! IsClient())
			return;
		GetClient()->addObjMessage( pMsg, pSrc );
	}
	void SysMessage( const TCHAR * pMsg ) const	// Push a message back to the client if there is one.
	{
		if ( ! IsClient())
			return;
		GetClient()->SysMessage( pMsg );
	}

	void UpdateStatsFlag() const
	{
		// Push status change to all who can see us.
		// For Weight, AC, Gold must update all
		// Just flag the stats to be updated later if possible.
		if ( ! IsClient())
			return;
		GetClient()->addUpdateStatsFlag();
	}
	void UpdateStats( STAT_TYPE x, int iChange = 0, int iLimit = 0 );
	void UpdateAnimate( ANIM_TYPE action, bool fTranslate = true, bool fBackward = false, BYTE iFrameDelay = 1 );

	void UpdateMode(  CClient * pExcludeClient = NULL, bool fFull= false );
	void UpdateMove( CPointMap pold, CClient * pClientExclude = NULL );
	void UpdateDir( DIR_TYPE dir );
	void UpdateDir( const CPointMap & pt );
	void UpdateDir( const CObjBaseTemplate * pObj );
	void UpdateDrag( CItem * pItem, CObjBase * pCont = NULL, CPointMap * pp = NULL );
	void Update( const CClient * pClientExclude = NULL );

public:
	const TCHAR * GetPronoun() const;	// he
	const TCHAR * GetPossessPronoun() const;	// his
	BYTE GetModeFlag( bool fTrueSight = false ) const
	{
		BYTE mode = 0;
		if ( IsStat( STATF_Poisoned ))
			mode |= CHARMODE_POISON;
		if ( IsStat( STATF_War ))
			mode |= CHARMODE_WAR;
		if ( ! fTrueSight && IsStat( STATF_Insubstantial | STATF_Invisible | STATF_Hidden | STATF_Sleeping ))	// if not me, this will not show up !
			mode |= CHARMODE_INVIS;
		return( mode );
	}
	BYTE GetDirFlag() const
	{
		BYTE dir = m_face_dir;
		ASSERT( dir<DIR_QTY );
		if ( IsStat( STATF_Fly ))
			dir |= 0x80; // running/flying ?
		return( dir );
	}
	WORD GetMoveBlockFlags() const
	{
		// What things block us ?
		if ( IsPriv(PRIV_GM|PRIV_ALLMOVE))	// nothing blocks us.
			return( 0xFFFF );
		return( m_pDef->m_Can );
	}

	int FixWeirdness();

private:
	// Contents/Carry stuff. ---------------------------------
	void ContentAdd( CItem * pItem )
	{
		LayerAdd( pItem, LAYER_QTY );
	}
protected:
	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
public:
	void OnWeightChange( int iChange );
	LAYER_TYPE CanEquipLayer( CItem * pItem, LAYER_TYPE layer = LAYER_QTY );
	CItem * LayerFind( LAYER_TYPE layer ) const;
	void LayerAdd( CItem * pItem, LAYER_TYPE layer = LAYER_QTY );
	int GetMaxWeightCarry() const
	{
		// weight is in tenths of a stone.
		return( Stat_Get(STAT_STR) * 4 * WEIGHT_UNITS + ( 30 * WEIGHT_UNITS ));
	}
	bool CanCarry( const CItem * pItem ) const
	{
		if ( IsPriv(PRIV_GM))
			return( true );

		// We are already carrying it. (sort of)
		const CChar * pCharParent = static_cast <const CChar*> (pItem->GetParent());
		if ( this == pCharParent )
		{
			if ( pItem->GetEquipLayer() == LAYER_DRAGGING )
				return( GetTotalWeight() <= GetMaxWeightCarry());
		}
		return(( GetTotalWeight() + pItem->GetWeight()) <= GetMaxWeightCarry());
	}
	int GetWeightLoadPercent( int iWeight ) const
	{
		// Get a percent of load.
		if ( IsPriv(PRIV_GM)) 
			return( 1 );
		return( IMULDIV( iWeight, 100, GetMaxWeightCarry()));
	}
	CItem * GetSpellbook() const;
	CItemContainer * GetPack( void ) const
	{
		return( dynamic_cast <CItemContainer *>(LayerFind( LAYER_PACK )));
	}
	CItemContainer * GetBank( LAYER_TYPE layer = LAYER_BANKBOX );
	CItemContainer * GetPackSafe( void )
	{
		return( GetBank(LAYER_PACK));
	}
	void AddGoldToPack( int iAmount, CItemContainer * pPack=NULL );

public:
	// Load/Save----------------------------------

	bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc );
	bool r_Verb( CScript & s, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s );
	bool r_Load( CScript & s );  // Load a character from Script
	bool r_WriteVal( const TCHAR * pKey, CGString & s, CTextConsole * pSrc = NULL );
	void r_Write( CScript & s );

private:
	bool OnTrigger( const TCHAR * pTrigName, CTextConsole * pSrc, int iArg = 0 );
	bool OnTrigger( CTRIG_TYPE trigger, CTextConsole * pSrc, int iArg = 0 )
	{
		ASSERT( trigger < CTRIG_QTY );
		return( OnTrigger( sm_szTrigName[trigger], pSrc, iArg ));
	}

private:
	// Noto/Karma stuff. --------------------------------
	void Noto_Karma( int iKarma, int iBottom=-10000 );
	void Noto_Fame( int iFameChange );
	void Noto_ChangeNewMsg( int iPrv );
	void Noto_ChangeDeltaMsg( int iDelta, const TCHAR * pszType );

public:
	NOTO_TYPE GetNotoFlag( const CChar * pChar, bool fIncog = false ) const;
	bool IsNeutral() const;
	bool IsMurderer() const;
	bool IsEvil() const;
	bool IsCriminal() const
	{
		// do the guards hate me ?
		if ( IsStat( STATF_Criminal )) 
			return( true );
		return IsEvil();
	}
	int Noto_GetLevel() const;
	const TCHAR * GetFameTitle() const;
	const TCHAR * Noto_GetTitle() const;

	void Noto_Kill( CChar * pKill, bool fGoodKill, bool fPetKill = false );
	void Noto_Criminal();
	void Noto_Murder();
	void Noto_KarmaChangeMessage( int iKarmaChange, int iLimit );

	bool IsTakeCrime( const CItem * pItem, CChar ** ppCharMark = NULL ) const;

public:
	// skills and actions. -------------------------------------------
	SKILL_TYPE Skill_GetBest() const; // Which skill is the highest for character p
	SKILL_TYPE GetActiveSkill() const
	{
		return( m_Act_SkillCurrent );
	}
	static bool IsSkillBase( SKILL_TYPE skill );
	static bool IsSkillNPC( SKILL_TYPE skill );
	const TCHAR * Skill_GetName( bool fUse = false ) const;
	short Skill_GetBase( SKILL_TYPE skill ) const
	{
		ASSERT( IsSkillBase(skill));
		return( m_Skill[skill] );
	}
	SKILLLOCK_TYPE Skill_GetLock( SKILL_TYPE skill ) const
	{
		if ( ! m_pPlayer ) 
			return( SKILLLOCK_UP );
		return( m_pPlayer->Skill_GetLock(skill));
	}
	short Skill_GetAdjusted( SKILL_TYPE skill ) const;
	void Skill_SetBase( SKILL_TYPE skill, short wValue );
	bool Skill_UseQuick( SKILL_TYPE skill, int difficulty );
	int  Skill_TestScript( TCHAR * pszCmd );
	bool Skill_CheckSuccess( SKILL_TYPE skill, int difficulty );
	bool Skill_Wait();
	bool Skill_Start( SKILL_TYPE skill, int iDifficulty = -1 ); // calc skill progress.
	void Skill_Fail( bool fCancel = false );
	bool Skill_MiningSmelt( CItem * pItemUse, CItem * pItemTarg );
	bool Skill_Tracking( CObjUID uidTarg, DIR_TYPE & dirPrv, int iDistMax = 0xFFFF );

private:
	void Skill_Cleanup();	 // may have just cancelled targetting.
	bool Skill_Done();	 // complete skill (async)
	int Skill_Linearity( SKILL_TYPE skill, int iLow, int iHigh ) const;
	void Skill_Experience( SKILL_TYPE skill, int difficulty );
	int  Skill_Snoop( bool fTest, bool fFail );
	int  Skill_Steal( bool fTest, bool fFail );
	bool Skill_Mining();
	void Skill_SetTimeout();

private:
	int  Spell_Linearity( SPELL_TYPE spell, int iskillval ) const;
	void Spell_Dispel( int iskilllevel );
	CChar * Spell_Summon( CREID_TYPE id, CPointMap pt );
	bool Spell_Recall( CItem * pRune, bool fGate );
	void Spell_Effect_Remove( CItem * pSpell );
	void Spell_Effect_Add( CItem * pSpell );
	CItem * Spell_Effect_Create( SPELL_TYPE spell, LAYER_TYPE layer, int iLevel, int iDuration, CObjBase * pSrc = NULL );

	void Spell_Bolt( CObjBase * pObj, ITEMID_TYPE idBolt, int iSkill );
	void Spell_Field( CPointMap pt, ITEMID_TYPE idEW, ITEMID_TYPE idNS, int iSkill );
	void Spell_Area( CPointMap pt, int iDist, int iSkill );
	bool Spell_Done( void );
	bool Spell_TargCheck();
	int  Spell_StartCast();

public:
	bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel );
	static int Spell_GetBaseDifficulty( SPELL_TYPE spell );
	bool Spell_Resurrection( int iSkillLoss );
	bool Spell_Teleport( CPointBase pt, bool fTakePets = false, bool fCheckAntiMagic = true, ITEMID_TYPE iEffect = ITEMID_FX_TELE_VANISH, SOUND_TYPE iSound = 0x01fe );
	bool Spell_Cast( SPELL_TYPE spell, CObjUID uid, CPointMap pt );
	bool Spell_CanCast( SPELL_TYPE spell, bool fTest, CObjBase * pSrc, bool fFailMsg );

	// Memories about objects in the world. -------------------
private:
	bool Memory_OnTick( CItemMemory * pMemory );
	bool Memory_UpdateFlags( CItemMemory * pMemory );
	bool Memory_UpdateClearTypes( CItemMemory * pMemory, WORD MemTypes );
	void Memory_AddTypes( CItemMemory * pMemory, WORD MemTypes );
	bool Memory_ClearTypes( CItemMemory * pMemory, WORD MemTypes );
	CItemMemory * Memory_CreateObj( CObjUID uid, WORD MemTypes );
	CItemMemory * Memory_CreateObj( const CObjBase * pObj, WORD MemTypes )
	{
		ASSERT(pObj);
		return Memory_CreateObj( pObj->GetUID(), MemTypes );
	}

public:
	void Memory_ClearTypes( WORD MemTypes );
	CItemMemory * Memory_FindObj( CObjUID uid ) const;
	CItemMemory * Memory_FindObj( const CObjBase * pObj ) const
	{
		if ( pObj == NULL )
			return( NULL );
		return Memory_FindObj( pObj->GetUID());
	}
	CItemMemory * Memory_AddObjTypes( CObjUID uid, WORD MemTypes );
	CItemMemory * Memory_AddObjTypes( const CObjBase * pObj, WORD MemTypes )
	{
		ASSERT(pObj);
		return Memory_AddObjTypes( pObj->GetUID(), MemTypes );
	}
	CItemMemory * Memory_FindTypes( WORD MemTypes ) const;
	CItemMemory * Memory_FindObjTypes( const CObjBase * pObj, WORD MemTypes ) const
	{
		CItemMemory * pMemory = Memory_FindObj(pObj);
		if ( pMemory == NULL ) 
			return( NULL );
		if ( ! pMemory->IsMemoryTypes( MemTypes ))
			return( NULL );
		return( pMemory );
	}

public:
	SOUND_TYPE SoundChar( CRESND_TYPE type );
	void Action_StartSpecial( CREID_TYPE id );

public:
	void OnNoticeCrime( CChar * pCriminal, const CChar * pCharMark );
	bool CheckCrimeSeen( SKILL_TYPE SkillToSee, const CChar * pCharMark, const CObjBase * pItem, const TCHAR * pAction );

private:
	// Armor, weapons and combat ------------------------------------
	int CalcArmorDefense( void );
	void CreateNewCharCheck();
	SKILL_TYPE GetWeaponSkill() const;
	int GetTargetHitDifficulty( SKILL_TYPE skill ) const;
	int SetWeaponSwingTimer();

public:
	void Fight_Retreat( CChar * pTarg, CItemMemory * pFight );
	void Fight_Start( const CChar * pTarg );
	bool Fight_OnTick( CItemMemory * pMemory );

	bool Player_OnVerb( CScript &s, CTextConsole * pSrc );
	void InitPlayer( CEvent * bin, CClient * pClient );
	bool ReadScript( CScript &s, bool fRestock = false, bool fNewbie = false );
	void NPC_LoadScript( bool fRestock );

	// Mounting and figurines
	bool Horse_Mount( CChar * pHorse ); // Remove horse char and give player a horse item
	bool Horse_UnMount(); // Remove horse char and give player a horse item

private:
	CItem * Horse_GetMountItem() const;
	CChar * Horse_GetMountChar() const;
public:
	CChar * Use_Figurine( CItem * pItem, int iPaces = 0 );
	CItem * Make_Figurine( CObjUID uidOwner, ITEMID_TYPE id = ITEMID_NOTHING );
	CItem * NPC_Shrink();

	int  ItemPickup( CItem * pItem, int amount );
	bool ItemEquip( CItem * pItem );
	bool ItemBounce( CItem * pItem );
	bool ItemDrop( CItem * pItem );

	void Flip( const TCHAR * pCmd = NULL );
	void SetPoison( int iLevel, CChar * pCharSrc );
	void SetPoisonCure( int iLevel, bool fExtra );
	CItemCorpse * FindMyCorpse() const;
	bool MakeCorpse( bool fFrontFall );
	bool RaiseCorpse( CItemCorpse * pCorpse );
	bool Death();
	bool Reveal( DWORD dwFlags = ( STATF_Invisible | STATF_Hidden | STATF_Sleeping ));
	void Jail( CTextConsole * pSrc, bool fSet );
	void EatAnim( const TCHAR * pszName, int iQty );
	void CallGuards();
	void Emote( const TCHAR * pText, CClient * pClientExclude = NULL, bool fPossessive = false );
	void Speak( const TCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY );
	void SpeakUTF8( const TCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, const TCHAR * pszLanguage = NULL );
	void SpeakUNICODE( const NCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, const TCHAR * pszLanguage = NULL );
	bool Attack( const CChar * pCharTarg );
	bool HitTry( void );
	int  Hit( CChar * pCharTarg );
	bool OnFreezeCheck();
	void DropAll( CItemContainer * pCorpse = NULL );
	void UnEquipAllItems( CItemContainer * pCorpse = NULL );
	void CancelAllTrades();
	void Wake();
	void SleepStart( bool fFrontFall );

	void Guild_Resign( MEMORY_TYPE memtype );
	CItemStone * Guild_Find( MEMORY_TYPE memtype ) const;
	CStoneMember * Guild_FindMember( MEMORY_TYPE memtype ) const;
	const TCHAR * Guild_Abbrev( MEMORY_TYPE memtype ) const;
	const TCHAR * Guild_AbbrevBracket( MEMORY_TYPE memtype ) const;

	void Use_EatQty( CItem * pFood, int iQty = 1 );
	bool Use_Eat( CItem * pItem, int iQty = 1 );
	int  Use_Consume( CItem * pItem, int iQty = 1 );
	bool Use_CarveCorpseTest( CChar *pCorpse, const CItemCorpse *pItem, bool fLooting = true );
	void Use_CarveCorpse( CItemCorpse * pCorpse );
	bool Use_Repair( CItem * pItem );
	int  Use_PlayMusic( CItem * pInstrument, int iDifficultyToPlay );
	bool Use_Drink( CItem * pItem );
	bool Use_Cannon_Feed( CItem * pCannon, CItem * pFeed );
	bool Use_Item_Web( CItem * pItem );
	void Use_AdvanceGate( CItem * pItem );
	void Use_MoonGate( CItem * pItem );
	bool Use_Campfire( CItem * pKindling );
private:
	bool Use_Plant( CItem * pItem );
	bool Use_PickPocketDip( CItem * pItem );
	bool Use_TrainingDummy( CItem * pItem );
	bool Use_ArcheryButte( CItem * pButte );
public:
	bool Use_Item( CItem * pItem, bool fLink = false );
	bool Use_Obj( CObjBase * pObj, bool fTestTouch );

	// NPC AI -----------------------------------------
private:
	bool NPC_OnHearTrigger( CScriptLock & s, const TCHAR * pCmd, CChar * pSrc = NULL );
	static CREID_TYPE NPC_GetAllyGroupType(CREID_TYPE idTest);

	void NPC_Food_Search();
	bool NPC_Food_MeatCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_FruitCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_CropCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_GrainCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_FishCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_GrassCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_GarbageCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_LeatherCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_HayCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_EdibleCheck(int iAmount, int iBite, int iSearchDist);
	bool NPC_Food_VendorCheck(int iAmount, int iBite);
	bool NPC_Food_VendorFind(int iSearchDist);
	CItem * NPC_Food_Trade( bool fTest );

	bool NPC_StablePetRetrieve( CChar * pCharPlayer );
	bool NPC_StablePetSelect( CChar * pCharPlayer );

	int NPC_WantThisItem( CItem * pItem ) const;
	int NPC_WantToUseThisItem( const CItem * pItem ) const;
	
	int  NPC_GetHostilityLevelToward( CChar * pCharTarg ) const;
	int	 NPC_GetAttackContinueMotivation( CChar * pChar, int iMotivation = 0 ) const;
	int  NPC_GetAttackMotivation( CChar * pChar, int iMotivation = 0 ) const;
	bool NPC_CheckHirelingStatus();
	int  NPC_GetTrainMax( SKILL_TYPE Skill ) const;

	bool NPC_OnVerb( CScript &s, CTextConsole * pSrc = NULL );
	bool NPC_MotivationCheck( int iMotivation );
	void NPC_OnHirePayMore( CItem * pGold, bool fHire = false );
	bool NPC_OnHirePay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold );
	bool NPC_OnHireHear( CChar * pCharSrc );
	bool NPC_Script_Command( const TCHAR * pCmd, bool fSystemCheck );
	void NPC_Script_OnTick( CItemMessage * pScriptItem, bool fForce = false );
	int	 NPC_OnTrainCheck( CChar * pCharSrc, SKILL_TYPE Skill );
	bool NPC_OnTrainPay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold );
	bool NPC_OnTrainHear( CChar * pCharSrc, const TCHAR * pCmd );
	bool NPC_CheckWalkHere( const CPointBase & pt, const CRegionBase * pArea ) const;

	bool NPC_LootContainer( CItemContainer * pItem );
	void NPC_LootMemory( CItem * pItem );
	bool NPC_LookAtCharGuard( CChar * pChar );
	bool NPC_LookAtCharHealer( CChar * pChar );
	bool NPC_LookAtCharHuman( CChar * pChar );
	bool NPC_LookAtCharMonster( CChar * pChar );
	bool NPC_LookAtChar( CChar * pChar, int iDist );
	bool NPC_LookAtItem( CItem * pItem, int iDist );
	bool NPC_LookAround();
	int  NPC_WalkToPoint( bool fRun = false );
	bool NPC_FightCast( CChar * pChar );
	bool NPC_FightMayCast() const;

	bool NPC_Act_Follow( bool fFlee = false, int maxDistance = 1, bool forceDistance = false );
	void NPC_Act_GoHome();
	void NPC_Act_Wander();
	bool NPC_Act_Begging( CChar * pChar );
	void NPC_Act_Fight();
	void NPC_Act_Idle();
	void NPC_Act_Looting();
	void NPC_Act_Eating();

	void NPC_ActStart_SpeakTo( CChar * pSrc );

	void NPC_OnTickAction();
	bool NPC_OnFoodTick( int nHungerPercent );

public:
	void NPC_ClearOwners();
	bool NPC_SetOwner( const CChar * pChar );
	CChar * NPC_GetOwner() const;
	bool NPC_IsOwnedBy( const CChar * pChar, bool fAllowGM = true ) const;
	bool NPC_IsSpawnedBy( const CItem * pItem ) const;
	bool NPC_CanSpeak() const;

	bool NPC_Vendor_Restock( int iTimeSec );
	bool NPC_Vendor_Dump( CItemContainer * pBank );

	void NPC_OnPetCommand( bool fSuccess, CChar * pMaster );
	bool NPC_OnHearPetCmd( const TCHAR * pszCmd, CChar * pSrc );
	bool NPC_OnHearPetCmdTarg( int iCmd, CChar * pSrc, CObjUID uid, const CPointMap & pt, const TCHAR * pszArgs );
	int  NPC_OnHearName( const TCHAR * pszText ) const;
	void NPC_OnHear( const TCHAR * pCmd, CChar * pSrc );
	bool NPC_OnItemGive( CChar * pCharSrc, CItem * pItem );
	bool NPC_SetVendorPrice( CItem * pItem, int iPrice );

	// Outside events that occur to us.
	int  OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType );
	int  OnTakeDamageHitPoint( int iDmg, CChar * pSrc, DAMAGE_TYPE uType );
	void OnOffendedBy( CChar * pCharSrc );
	bool OnAttackedBy( CChar * pCharSrc, bool fCommandPet );
//	bool OnAttackedBy( CChar * pCharSrc );

	bool OnEquipTick( CItem * pItem );
	void OnFoodTick();
	bool OnTick();

	static CChar * CreateBasic( CREID_TYPE baseID );
	static CChar * CreateNPC( CREID_TYPE id );
};

inline bool CChar::IsSkillBase( SKILL_TYPE skill ) // static
{
	// Is this in the base set of skills.
	return( IS_SKILL_BASE(skill));
}
inline bool CChar::IsSkillNPC( SKILL_TYPE skill )  // static
{
	// Is this in the NPC set of skills.
	return( skill >= NPCACT_FOLLOW_TARG && skill < NPCACT_QTY );
}

////////////////////////////////////////////////////////////////////////////////////

#if 0

class CQuest : public CGObListRec : public CScriptObj // NOT USED YET
{
	// Quests are scripted things that run groups of NPC's and players.
	CGString m_sName;	// List is sorted by name of the quest.
	int m_iState;	// what is the state of the quest.
	DWORD m_dwFlags;	// Arbitrary flags for the quest.
	// List of Players and NPC's involved in the quest. (and what there status is)

public:

	const TCHAR * GetName() const 
	{
		// ( every object must have at least a type name )
		return( m_sName );
	}

};

#endif

class CItemsDisconnectList : public CGObList
{
};

class CCharsDisconnectList : public CGObList
{
};

class CCharsActiveList : public CGObList
{
private:
	int	   m_iClients;			// How many clients in this sector now?
public:
	time_t m_LastClientTime;	// age the sector based on last client here.
private:
	void InsertAfter( CGObListRec * pNewRec, CGObListRec * pPrev = NULL )
	{
		ASSERT(0);
	}
protected:
	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
public:
	int HasClients() const { return( m_iClients ); }
	void ClientAttach()
	{
		m_iClients++;
	}
	void ClientDetach()
	{
		DEBUG_CHECK(m_iClients>0);
		m_iClients--;
	}
	void AddToSector( CChar * pChar )
	{
		ASSERT( pChar );
		// ASSERT( pChar->m_p.IsValid());
		if ( pChar->IsClient())
		{
			ClientAttach();
		}
		CGObList::InsertAfter(pChar);
	}
	CCharsActiveList()
	{
		m_LastClientTime = 0;
		m_iClients = 0;
	}
};

class CItemsList : public CGObList
{
	// Top level list of items.
private:
	void InsertAfter( CGObListRec * pNewRec, CGObListRec * pPrev = NULL )
	{
		ASSERT( 0 );	// do not use this.
	}
protected:
	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
public:
	void AddToSector( CItem * pItem )
	{
		// Add to top level.
		// Assume MoveTo() is being called as well
		ASSERT( pItem );
		CGObList::InsertAfter( pItem );
	}
};

class CSector : public CScriptObj	// square region of the world.
{
	// A square region of the world. ex: MAP0.MUL Dungeon Sectors are 256 by 256 meters
#define SECTOR_TICK_PERIOD (TICK_PER_SEC/4) // length of a pulse.

private:
	static const TCHAR * sm_KeyTable[];

	bool   m_fSaveParity;		// has the sector been saved relative to the char entering it ?
	WEATHER_TYPE m_weather;		// the weather in this area now.

#define LIGHT_OVERRIDE 0x80
	BYTE m_locallight;		// the calculated light level in this area. |0x80 = override.
	BYTE m_RainChance;	// 0 to 100%
	BYTE m_ColdChance;		// Will be snow if rain chance success.

	CGObArray<CGrayMapBlock*> m_CacheMap;	// Cache Map Stuff
	CGObArray<CTeleport*> m_Teleports;	// CTeleport

public:
	// Search for items and chars in a region must check 4 quandrants around location.
	CCharsActiveList m_Chars;		// CChar(s) in this CSector.
	CCharsDisconnectList m_Chars_Disconnect;	// Idle player characters. Dead NPC's and ridden horses.
	CItemsList m_Items_Timer;	// CItem(s) in this CSector that need timers.
	CItemsList m_Items_Inert;	// CItem(s) in this CSector. (no timer required)
	CGObList m_Regions;		// CRegionBase(s) in this CSector.

private:
	int GetIndex() const;
	WEATHER_TYPE GetWeatherCalc() const;
	BYTE GetLightCalc( bool fQuickSet ) const;
	void SetLightNow( bool fFlash = false );
	bool IsMoonVisible( int iPhase, int iLocalTime ) const;
	void SetDefaultWeatherChance();

public:
	CSector();
	~CSector()
	{
		ASSERT( ! HasClients());
	}
	void OnTick( int iPulse );

	// Location map units.
	CPointMap GetBase() const
	{
		// What is the coord base of this sector. upper left point.
		int index = GetIndex();
		ASSERT( index >= 0 && index < SECTOR_QTY );
		CPointMap pt(( index % SECTOR_COLS ) * SECTOR_SIZE_X, ( index / SECTOR_COLS ) * SECTOR_SIZE_Y );
		return( pt );
	}
	CPointMap GetMid() const
	{
		CPointMap pt = GetBase();
		pt.m_x += SECTOR_SIZE_X/2;	// East
		pt.m_y += SECTOR_SIZE_Y/2;	// South
		return( pt );
	}
	CRectMap GetRect() const
	{
		CPointMap pt = GetBase();
		CRectMap rect;
		rect.m_left = pt.m_x;
		rect.m_top = pt.m_y;
		rect.m_right = pt.m_x + SECTOR_SIZE_X;	// East
		rect.m_bottom = pt.m_y + SECTOR_SIZE_Y;	// South
		return( rect );
	}

	// Time
	int GetLocalTime() const;
	const TCHAR * GetGameTime() const;

	// Weather
	WEATHER_TYPE GetWeather() const	// current weather.
	{
		return m_weather;
	}
	bool IsRainOverriden() const
	{
		return(( m_RainChance & LIGHT_OVERRIDE ) ? true : false );
	}
	BYTE GetRainChance() const
	{
		return( m_RainChance &~ LIGHT_OVERRIDE );
	}
	bool IsColdOverriden() const
	{
		return(( m_ColdChance & LIGHT_OVERRIDE ) ? true : false );
	}
	BYTE GetColdChance() const
	{
		return( m_ColdChance &~ LIGHT_OVERRIDE );
	}
	void SetWeather( WEATHER_TYPE w );
	void SetWeatherChance( bool fRain, int iChance );

	// Light
	bool IsLightOverriden() const
	{
		return(( m_locallight & LIGHT_OVERRIDE ) ? true : false );
	}
	BYTE GetLight() const
	{
		return( m_locallight &~ LIGHT_OVERRIDE );
	}
	void LightFlash()
	{
		SetLightNow( true );
	}
	void SetLight( int light );

	// Chars in the sector.
	bool IsCharActiveIn( const CChar * pChar ) //const
	{
		// assume the char is active (not disconnected)
		return( pChar->GetParent() == &m_Chars );
	}
	bool IsCharDisconnectedIn( const CChar * pChar ) //const
	{
		// assume the char is active (not disconnected)
		return( pChar->GetParent() == &m_Chars_Disconnect );
	}
	int GetComplexity() const
	{
		return( m_Chars.GetCount());
	}
	int HasClients() const
	{
		return( m_Chars.HasClients());
	}
	time_t GetLastClientTime() const
	{
		return( m_Chars.m_LastClientTime );
	}
	bool IsSectorSleeping() const;
	void SetSectorWakeStatus();	// Ships may enter a sector before it's riders !
	void ClientAttach( CChar * pChar )
	{
		if ( ! IsCharActiveIn( pChar )) return;
		m_Chars.ClientAttach();
	}
	void ClientDetach( CChar * pChar )
	{
		if ( ! IsCharActiveIn( pChar )) return;
		m_Chars.ClientDetach();
	}
	void MoveToSector( CChar * pChar );

	// General.
	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc );
	void r_Write( CScript & s );
	bool r_Verb( CScript & s, CTextConsole * pSrc );

	void AddTeleport( CTeleport * pTel )
	{
		m_Teleports.Add( pTel );
	}
	const CTeleport * GetTeleport( const CPointMap & pt ) const;
	void UnLoadRegions();

	const CGrayMapBlock * GetMapBlock( const CPointMap & pt );
	void Restock( long iTime = 0 );
	void RespawnDeadNPCs();
	CItem * CheckNaturalResource( const CPointMap & pt, ITEM_TYPE Type, bool fTest = true );

	void Close();
	const TCHAR * GetName() const { return( "Sector" ); }
};

struct CAccountArray : public CGObSortArray< CAccount*, const TCHAR *>
{
	// Sorted array
	int CompareKey( const TCHAR * pName, CAccount * pAccount ) const
	{
		ASSERT( pAccount );
		return( strcmpi( pName, pAccount->GetName()));
	}
};

struct CMultiDefArray : public CGObSortArray< CGrayMulti*, MULTI_TYPE >
{
	// Sorted array
	int CompareKey( MULTI_TYPE id, CGrayMulti* pBase ) const
	{
		ASSERT( pBase );
		return( id - pBase->GetMultiID());
	}
};

extern class CWorld : public CScriptObj	// the world. Stuff saved in *World.SCP file.
{
	static const TCHAR * sm_Table[];
private:
	// Clock stuff. how long have we been running ? all i care about.
	time_t m_Clock_Time;		// the current relative tick time  (in seconds)
	DWORD  m_Clock_PrevSys;		// System time of the last OnTick(). (CLOCKS_PER_SEC)

	// Special purpose timers.
	time_t	m_Clock_Sector;		// next time to do sector stuff.
	time_t	m_Clock_Save;		// when to auto save ?
	time_t	m_Clock_Respawn;	// when to res dead NPC's ?
	int		m_Sector_Pulse;		// Slow some stuff down that doesn't need constant processing.

	int		m_iSaveCount;	// Current archival backup status.
	int		m_iSaveStage;	// Current stage of the background save.

	CGObArray<CObjBase*> m_UIDs;	// all the UID's in the World. CChar and CItem.

public:
	CScript m_File;				// Save or Load file.
	bool	m_fSaveParity;		// has the sector been saved relative to the char entering it ?
	bool	m_fSaveForce;
	int		m_iSaveVersion;		// Previous save version. (only used during load of course)
	long	m_lLoadSize;		// Current size of the loading scripts.
	time_t  m_Clock_Startup;		// When did the system restore save ?

	// World data.
	CSector m_Sectors[ SECTOR_QTY ];
	CItemsDisconnectList m_ItemsNew;	// Item created but not yet placed in the world.
	CCharsDisconnectList m_CharsNew;	// Chars created but not yet placed.
	CGObList m_ObjDelete;		// Objects to be deleted.

	CGObList m_Regions;		// CRegionWorlds(s)
	CGObList m_GMPages;		// Current outstanding GM pages. (CGMPage)
	CGTypedArray<CPointBase,CPointBase&> m_MoonGates;	// The array of moongates.
	CGPtrTypeArray<CItemStone*> m_Stones;	// links to leige stones.
	CAccountArray m_Accounts;	// All the player accounts. name sorted
	CMultiDefArray m_MultiDefs;

	static const TCHAR * sm_KeyTable[];

private:
	bool LoadSection();
	bool LoadRegions( CScript & s );

	void SaveTry(bool fForceImmediate); // Save world state
	void GarbageCollection_New();
	void GarbageCollection_GMPages();
	int FixObjTry( CObjBase * pObj, int iUID = 0 );
	bool SaveStage();
	void GetBackupName( CGString & sArchive, TCHAR chType ) const;
	void SaveForce(); // Save world state

public:
	CWorld();
	~CWorld()
	{
		Close();
	}
	bool IsSaving() const
	{
		return( m_File.IsFileOpen() && m_File.IsWriteMode());
	}

	// Time

	time_t GetTime() const
	{
		return m_Clock_Time;  // Time in TICK_PER_SEC
	}
#define TRAMMEL_SYNODIC_PERIOD 105 // in minutes
#define FELUCCA_SYNODIC_PERIOD 840 // in minutes
#define TRAMMEL_FULL_BRIGHTNESS 2 // light units LIGHT_BRIGHT
#define FELUCCA_FULL_BRIGHTNESS 6 // light units LIGHT_BRIGHT
	int GetMoonPhase( bool bMoonIndex = false ) const;
	DWORD GetNextNewMoon (bool bMoonIndex) const;

	DWORD GetGameWorldTime( DWORD basetime ) const;
	DWORD GetGameWorldTime() const	// return game world minutes
	{
		return( GetGameWorldTime( GetTime()));
	}

	// UID Managenent

	DWORD GetUIDCount() const
	{
		return( m_UIDs.GetCount());
	}
#define UID_PLACE_HOLDER ((CObjBase*)0xFFFFFFFF)
	CObjBase * FindUID( DWORD dwIndex ) const
	{
		if ( ! dwIndex || dwIndex >= GetUIDCount())
			return( NULL );
		if ( m_UIDs[ dwIndex ] == UID_PLACE_HOLDER )	// unusable for now.
			return( NULL );
		return( m_UIDs[ dwIndex ] );
	}
	void FreeUID( DWORD dwIndex )
	{
		// Can't free up the UID til after the save !
		m_UIDs[dwIndex] = ( IsSaving()) ? UID_PLACE_HOLDER : NULL;
	}
	DWORD AllocUID( DWORD dwIndex, CObjBase * pObj );

	// World Map stuff.

	signed char GetHeight( const CPointBase & pt, WORD & wBlockFlags, const CRegionBase * pRegion = NULL ); // Height of player who walked to X/Y/OLDZ
	const CGrayMapBlock * GetMapBlock( const CPointMap & pt )
	{
		return( pt.GetSector()->GetMapBlock(pt));
	}
	const CUOMapMeter * GetMapMeter( const CPointMap & pt ) const // Height of MAP0.MUL at given coordinates
	{
		const CGrayMapBlock * pMapBlock = pt.GetSector()->GetMapBlock(pt);
		ASSERT(pMapBlock);
		return( pMapBlock->GetTerrain( UO_BLOCK_OFFSET(pt.m_x), UO_BLOCK_OFFSET(pt.m_y)));
	}
	const CGrayMulti * GetMultiItemDefs( ITEMID_TYPE itemid );

	CPointMap FindItemTypeNearby( const CPointMap & pt, ITEM_TYPE iType, int iDistance = 0 );
	bool IsItemTypeNear( const CPointMap & pt, ITEM_TYPE iType, int iDistance = 0 )
	{
		CPointMap ptn = FindItemTypeNearby( pt, iType, iDistance );
		return( ptn.IsValid());
	}
	CItem * CheckNaturalResource( const CPointMap & pt, ITEM_TYPE Type, bool fTest = true )
	{
		// RETURN: Quantity they wanted. 0 = none here.
		if ( ! pt.IsValid())
			return( NULL );
		return( pt.GetSector()->CheckNaturalResource( pt, Type, fTest ));
	}

	// Regions
	bool LoadRegions();
	static void UnLoadRegions( CGObList & List );
	void UnLoadRegions();
	void ReLoadBases();

	// Accounts.
	bool SaveAccounts();
	CAccount * AccountFind( const TCHAR * pszName, bool fCreate = false, bool fGuest = false );
	bool LoadAccounts( bool fChanges = true, bool fClearChanges = false );
	bool OnAccountCmd( TCHAR * pszArgs, CTextConsole * pSrc );

	bool r_Verb( CScript & s, CTextConsole * pSrc );
	bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s ) ;

	void OnTick();

	void GarbageCollection();
	int  FixObj( CObjBase * pObj, int iUID = 0 );
	void Restock();
	void RespawnDeadNPCs();

	void Speak( const CObjBaseTemplate * pSrc, const TCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD );
	void SpeakUNICODE( const CObjBaseTemplate * pSrc, const NCHAR * pText, COLOR_TYPE color = COLOR_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD, const char * pszLanguage = NULL );
	void Broadcast( const TCHAR * pMsg );
	CItem * Explode( CChar * pSrc, CPointMap pt, int iDist, int iDamage, WORD wFlags = DAMAGE_GENERAL | DAMAGE_HIT );

	CPointMap GetRegionPoint( const TCHAR * pCmd ) const; // Decode a teleport location number into X/Y/Z
	const TCHAR * GetGameTime() const;

	bool Export( const TCHAR * pszFilename, const CChar* pSrc, WORD iModeFlags = 1, int iDist = 0xFFFF, int dx = 0, int dy = 0 );
	bool Import( const TCHAR * pszFilename, const CChar* pSrc, WORD iModeFlags = 1, int iDist = 0xFFFF, TCHAR *pszAgs1 = NULL, TCHAR *pszAgs2 = NULL );
	void Save( bool fForceImmediate ); // Save world state
	bool Load();
	void Close();

	const TCHAR * GetName() const { return( "World" ); }

} g_World;

inline int CObjBase::GetTimerDiff() const
{
	// How long till this will expire ?
	return( m_timeout - g_World.GetTime());
}
inline int CSector::GetIndex() const
{
	int i = this - g_World.m_Sectors;
	ASSERT( i < SECTOR_QTY );
	return( i );
}

class CWorldSearch	// define a search of the world.
{
private:
	const CPointMap m_p;		// Base point of our search.
	const int m_iDist;			// How far from the point are we interested in
	bool m_fInertCharUse;

	CObjBase * m_pObj;	// The current object of interest.
	CObjBase * m_pObjNext;	// In case the object get deleted.
	bool m_fInertToggle;

	CSector * m_pSectorBase;	// Don't search the center sector 2 times.
	CSector * m_pSector;	// current Sector
	CPointMap m_pntSector;		// What is the current point for the current sector.
	CRectMap m_rectSector;		// A rectangle containing our sectors we can search.
private:
	bool GetNextSector();
public:
	CWorldSearch( const CPointMap pt, int iDist = 0 );
	void SetInertView( bool fView ) { m_fInertCharUse = fView; }
	void SetRegion( const CRegionBase * pRegion );
	void SetFilter( const TCHAR * pszFilter );
	CChar * GetChar();
	CItem * GetItem();
};

// Internal defs.

struct CPotionDef : public CMemDynamic, public CScriptObj
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sName;		// Potion real name.
public:
	static const TCHAR * sm_PotionTable[];
	ITEMID_TYPE m_potionID;// resulting bottle type.
	ITEMID_TYPE m_RegID;		// Reagent code.
	int m_RegReq;			// Amount of reagent required.
	int m_SkillReq;		// Minuimum required skill level for this potion.
	COLOR_TYPE m_color;	// For nonstandard potions (COLOR_DEFAULT for normal ones)
public:
	CPotionDef( CScript & s )
	{
		r_Load( s );
	}
	const TCHAR * GetName() const { return( m_sName ); }
	bool r_LoadVal( CScript & s );
};

class COreDef : public CMemDynamic, public CScriptObj
{
	// NOTE: Ore defs reserve the low range of MATERIAL_TYPE
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sName;
public:
	static const TCHAR * sm_OresTable[];
	COLOR_TYPE m_wColor;	// what color is the ore. (ores and metals are unique defined by color)
	int m_iMinSkill;		// Mining/Smelting skill needed to find it.
	int m_iVeinChance;		// rareity in the random world. (may be higher locally)
	int m_iMinAmount;		// How much to get of it.
	int m_iMaxAmount;
	ITEMID_TYPE m_wIngotItem;	// The ingot item ??? This should just be a colored / named item ?

	// Physical properties of the ore ?
	// By trigger or by param ?
public:
	const TCHAR * GetName() const { return( m_sName ); }
	COreDef( CScript & s);
};

class CSpellDef : public CMemDynamic, public CScriptObj	// 1 based spells. See SPELL_*
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	WORD m_wFlags;
#define SPELLFLAG_DIR_ANIM  0x0001	// Evoke type cast or directed. (animation)
#define SPELLFLAG_TARG_OBJ  0x0002	// Need to target an object or char ?
#define SPELLFLAG_TARG_CHAR 0x0004	// Needs to target a living thing
#define SPELLFLAG_TARG_XYZ  0x0008	// Can just target a location.
#define SPELLFLAG_HARM		0x0010	// The spell is in some way harmfull.
#define SPELLFLAG_FX_BOLT	0x0020	// Effect is a bolt to the target.
#define SPELLFLAG_FX_TARG	0x0040	// Effect is at the target.
#define SPELLFLAG_FIELD		0x0080	// create a field of stuff. (fire,poison,wall)
#define SPELLFLAG_SUMMON	0x0100	// summon a creature.
#define SPELLFLAG_GOOD		0x0200	// The spell is a good spell. u intend to help to receiver.
#define SPELLFLAG_DISABLED	0x8000
	CGString m_sName;	// spell name of course

public:
	static const TCHAR * sm_SpellsTable[];
	SOUND_TYPE m_sound;			// What noise does it make when done.
	CGString m_sRunes;	// Letter Runes for Words of power.
	CGString m_sReags;	// What reagents does it take ?
	ITEMID_TYPE m_SpellID;		// The rune graphic for this.
	ITEMID_TYPE m_ScrollID;		// The scroll graphic item for this.
	ITEMID_TYPE m_wEffectID;	// Animation effect ID
	WORD m_wEffectLo;	// Damage or time or effect based on skill of caster.
	WORD m_wEffectHi;	// Damage or time of effect based on 100% magery
	WORD m_wCastTime;	// In tenths.
	WORD m_wDurationTimeLo;
	WORD m_wDurationTimeHi;
public:
	bool IsSpellType( WORD wFlags ) const
	{
		return(( m_wFlags & wFlags ) ? true : false );
	}
	CSpellDef( CScript & s )
	{
		m_wFlags = SPELLFLAG_DISABLED;
		r_Load( s );
	}
	const TCHAR * GetName() const { return( m_sName ); }
	bool r_LoadVal( CScript & s );
};

struct CRandGroupRec
{
	DWORD m_dwVal;
	int m_iWeight;	// relative random weight for this possible value.
};

class CRandGroupDef	: public CMemDynamic, public CScriptObj // A spawn group.
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	int m_iTotalWeight;
	static const TCHAR * sm_Table[];
	CGTypedArray<CRandGroupRec, CRandGroupRec&> m_Members;
	CGString m_sName;	// Name the group.
public:
	CRandGroupDef()
	{
		m_iTotalWeight = 0;
	}
	CRandGroupDef( CScript & s )
	{
		m_iTotalWeight = 0;
		r_Load( s );
	}
	const TCHAR * GetName() const { return( m_sName ); }
	bool r_LoadVal( CScript & s );
	int GetRandMemberIndex() const
	{
		int iCount = m_Members.GetCount();
		if ( ! iCount ) 
			return( -1 );
		int iWeight = GetRandVal( m_iTotalWeight ) + 1;
		int i=0;
		for ( ; iWeight > 0 && i<iCount; i++ )
		{
			iWeight -= m_Members[i].m_iWeight;
		}
		return( i - 1);
	}
	DWORD GetMemberVal( int i ) const
	{
		return( m_Members[i].m_dwVal );
	}
};

struct CAdvanceDef
{
	int m_Rate[3];
public:
	void Load( TCHAR * pszDef );
	int GetChance( int iSkillAdj ) const;
};

struct CSkillClassDef : public CMemDynamic, public CScriptObj // For skill def table
{
	// Similar to character class.
	static const TCHAR * sm_Table[];
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CGString m_sName;	// The name of this skill class.
	WORD m_StatMax[STAT_BASE_QTY];	// STAT_BASE_QTY
	WORD m_SkillLevelMax[ SKILL_QTY ];

	// Triggers that come with the skillclass 
	DWORD m_dwPlotFlags;
	CFragmentArray m_Events;	// Action or motivation type indexes.

private:
	void Init()
	{
		m_dwPlotFlags = 0;
		for ( int i=0; i<COUNTOF(m_SkillLevelMax); i++ )
		{
			m_SkillLevelMax[i] = 1000;
		}
		for ( int i=0; i<COUNTOF(m_StatMax); i++ )
		{
			m_StatMax[i] = 100;
		}
	}
public:
	CSkillClassDef( CScript & s )
	{
		Init();
		r_Load( s );
	}
	CSkillClassDef()
	{
		Init();
	}
	const TCHAR * GetName() const { return( m_sName ); }
	bool r_LoadVal( CScript & s );
};

struct CSkillDef : public CMemDynamic, public CScriptObj // For skill def table
{
	static const TCHAR * sm_SkillsTable[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sKey;	// script key word for skill.
public:
	CGString m_sTitle;	// title one gets if it's your specialty.

	CGString m_sTargetPrompt;	// targetting prompt.
	CGString m_sFailMsg;			// Message on skill fail

	WORD m_wDelay;	//	The base delay for the skill. (tenth of seconds)

	// Stat effects.
	// You will tend toward these stat vals if you use this skill a lot.
	BYTE m_Stat[3];	// STAT_STR, STAT_INT, STAT_DEX
	BYTE m_StatBonus[3]; // % of each stat toward success at skill
	BYTE m_SkillStat; // % of skill toward success at skill (heh funny sounding eh?)
	CAdvanceDef m_Adv; // OSI defined "skill curve"

	// Delay before skill complete. modified by skill level of course !
public:
	CSkillDef( SKILL_TYPE i, CScript & s );
	const TCHAR * GetName() const { return( GetKey()); }
	const TCHAR * GetKey() const
	{
		return( m_sKey );
	}
	bool r_LoadVal( CScript & s );
};

enum SERV_STAT_TYPE
{
	SERV_STAT_CLIENTS,	// How many clients does it say it has ? (use as % full)
	SERV_STAT_CHARS,
	SERV_STAT_ITEMS,
	SERV_STAT_MEM,
	SERV_STAT_ALLOCS,	// How many calls to new()
	SERV_STAT_QTY,
};

enum ACCAPP_TYPE	// types of new account applications.
{
	ACCAPP_Closed,		// Closed. Not accepting more.
	ACCAPP_EmailApp,	// Must send email to apply.
	ACCAPP_Free,		// Anyone can just log in and create a full account.
	ACCAPP_GuestAuto,	// You get to be a guest and are automatically sent email with u're new password.
	ACCAPP_GuestTrial,	// You get to be a guest til u're accepted for full by an Admin.
	ACCAPP_Other,		// specified but other ?
	ACCAPP_Unspecified,	// Not specified.
	ACCAPP_WebApp,		// Must fill in a web form and wait for email response
	ACCAPP_WebAuto,		// Must fill in a web form and automatically have access
	ACCAPP_QTY,
};

struct CServRef	: public CScriptObj	// Array of servers we point to.
{	// NOTE: g_Serv = this server.
	static const TCHAR * sm_KeyTable[];

private:
	CGString m_sName;	// What the name should be. Fill in from ping.

	time_t  m_LastPollTime;		// Last poll time in World.GetTime() (if polling)
	time_t  m_LastValidTime;	// Last valid poll time in g_World.GetTime()
	CRealTime m_LastValidDate;

	time_t  m_CreateTime;	// When added to the list ? 0 = at start up.

	// Status read from returned string.
	CGString m_sStatus;	// last returned status string.

	// statistics
	DWORD m_dwStat[ SERV_STAT_QTY ];

protected:
	int m_iClientsAvg;	// peak per day of clients.

public:
	CSocketAddress m_ip;	// socket and port.

	// Breakdown the string. or filled in locally.
	signed char m_TimeZone;	// Hours from GMT. +5=EST
	CGString m_sEMail;		// Admin email address.
	CGString m_sURL;			// URL for the server.
	CGString m_sVersion;	// Version string this server is using.
	CGString m_sLang;
	CGString m_sNotes;		// Notes to be listed for the server.
	ACCAPP_TYPE m_eAccApp;	// types of new account applications.
	CGString m_sRegisterPassword;	// The password  the server uses to id itself to the registrar.

public:
	CServRef( const TCHAR * pszName, DWORD dwIP );

	const TCHAR * GetStatus() const
	{
		return(m_sStatus);
	}

	DWORD StatGet( SERV_STAT_TYPE i ) const
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		return( m_dwStat[i] );
	}
	void StatInc( SERV_STAT_TYPE i )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i]++;
	}
	void StatDec( SERV_STAT_TYPE i )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		DEBUG_CHECK( m_dwStat[i] );
		m_dwStat[i]--;
	}
	void StatChange( SERV_STAT_TYPE i, int iChange )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i] += iChange;
	}
	void SetStat( SERV_STAT_TYPE i, DWORD dwVal )
	{
		ASSERT( i>=0 && i<SERV_STAT_QTY );
		m_dwStat[i] = dwVal;
	}

	const TCHAR * GetName() const { return( m_sName ); }
	void SetName( const TCHAR * pszName );

	bool IsValidStatus() const;
	bool ParseStatus( const TCHAR * pszStatus, bool fStore );
	bool PollStatus();	// Do this on a seperate thread.

	virtual int GetAgeHours() const;

	bool IsSame( const CServRef * pServNew ) const
	{
		if ( m_sRegisterPassword.IsEmpty())
			return( true );
		return( ! strcmpi( m_sRegisterPassword, pServNew->m_sRegisterPassword ));
	}

	int GetTimeSinceLastValid() const;

	void WriteCreated( CScript & s );
	void ClientsInc()
	{
		StatInc( SERV_STAT_CLIENTS );
		if ( StatGet(SERV_STAT_CLIENTS) > m_iClientsAvg )
		{
			m_iClientsAvg = StatGet(SERV_STAT_CLIENTS);
		}
	}
	int GetClientsAvg() const
	{
		if ( m_iClientsAvg )
			return( m_iClientsAvg );
		return( StatGet(SERV_STAT_CLIENTS));
	}
	void ClientsResetAvg();

	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR *pKey, CGString &sVal, CTextConsole * pSrc = NULL );

	bool IsConnected() const
	{
		return( m_LastValidTime && ( m_LastPollTime <= m_LastValidTime ));
	}
	void addToServersList( CCommand & cmd, int iThis, int jArray ) const;
};

class CThread	// basic multi tasking functionality.
{
protected:
	DWORD m_hThread;
#ifdef _WIN32
	DWORD m_ThreadID;
#endif
protected:
	void OnClose();
public:
	CThread()
	{
		m_hThread = 0;
	}
	~CThread()
	{
		TerminateThread( -1 );
	}
	void OnCreate()
	{
#ifdef _WIN32
		m_ThreadID = GetCurrentThreadId();
#endif
	}
	bool IsActive() const
	{
		return( m_hThread ? true : false );
	}
	bool TerminateThread( DWORD dwExitCode );
#ifdef _WIN32
	void CreateThread( void ( _cdecl * pEntryProc )(void *));
#else
	void CreateThread( void * ( _cdecl * pEntryProc )(void *));
#endif
	void WaitForClose( int iSec );
};

class CBackTask : public CThread
{
	// Do polling ad other stuff in a background thread.
private:
	DWORD m_dwNextRegisterTime;	// only register once every 2 hours or so.
	DWORD m_dwNextPollTime;		// Only poll servers once every 10 minutes or so.
	DWORD m_dwNextMailTime;
public:
	CSocketAddress m_RegisterIP;	// look up this ip.
	int		m_iTotalPolledClients;
private:
#ifdef _WIN32
	static void _cdecl EntryProc( void * lpThreadParameter );
#else
	static void * _cdecl EntryProc( void * lpThreadParameter );
#endif
	bool RegisterServer();
	void PollServers();
	void SendOutgoingMail();

public:
	CBackTask()
	{
		m_dwNextRegisterTime = 0;
		m_dwNextPollTime = 0;
		m_dwNextMailTime = 0;
		m_RegisterIP.SetPort( GRAY_DEF_PORT );
		m_iTotalPolledClients = 0;
	}

	// Check the creation of the back task.
	void CreateThread();
};

class CMailSMTP : protected CGSocket
{
	// Mail sending class.
protected:
    CGString m_sResponse;
#define PORT_SMTP	25

private:
    bool ReceiveResponse();

protected:
    bool OpenConnection(const TCHAR *szMailServerIPAddress);
    bool SendLine(const TCHAR *szMessage, int iResponseExpect = 1 );
public:
	bool SendMail( const TCHAR * pszDstMail, const TCHAR * pszSrcMail, CGString * ppszBody, int iBodyLines );
	const TCHAR * GetLastResponse() const
	{
		return m_sResponse;
	}
};

enum PROFILE_TYPE
{
	PROFILE_OVERHEAD,	// In between stuff.
	PROFILE_IDLE,		// Wait for stuff.
	PROFILE_NETWORK_RX,	// Just get client info and monitor new client requests. No processing.
	PROFILE_CLIENTS,	// Client processing.
	PROFILE_NETWORK_TX,
	PROFILE_CHARS,
	PROFILE_ITEMS,
	PROFILE_NPC_AI,

	PROFILE_TIME_QTY,

	// Qty of bytes. Not Time.
	PROFILE_DATA_TX = PROFILE_TIME_QTY,
	PROFILE_DATA_RX,

	PROFILE_QTY,
};

class CProfileData
{
protected:
	struct
	{
		LONGLONG m_Time;	// accumulated time in msec.
		int m_iCount;		// how many passes made into this.
	} m_PrvTimes[ PROFILE_QTY ];
	struct
	{
		LONGLONG m_Time;	// accumulated time in msec.
		int m_iCount;		// how many passes made into this.
	} m_CurTimes[ PROFILE_QTY ];

	int m_iActiveWindowSec;		// The sample window size in seconds. 0=off
	LONGLONG m_Frequency;	// QueryPerformanceFrequency()
	LONGLONG m_TimeTotal;	// Average this over a total time period.

	// Store the last time start time.
	PROFILE_TYPE  m_CurTask;	// What task are we currently processing ?
	LONGLONG m_CurTime;		// QueryPerformanceCount()

public:
	bool IsActive() const { return( m_iActiveWindowSec ? true : false ); }
	void SetActive( int iSampleSec );
	int GetActiveWindow() const { return m_iActiveWindowSec; }

	void Count( PROFILE_TYPE id, DWORD dwVal )
	{
		ASSERT( id >= PROFILE_TIME_QTY && id < PROFILE_QTY );
		m_CurTimes[id].m_Time += dwVal;
		m_CurTimes[id].m_iCount ++;
	}
	CProfileData()
	{
		SetActive(10);	// default to 10 sec window.
	}

	void Start( PROFILE_TYPE id );

	const TCHAR * GetName( PROFILE_TYPE id ) const;
	const TCHAR * GetDesc( PROFILE_TYPE id ) const;
};

struct CScriptIndexArray : public CGObSortArray< CScriptIndexLink *, DWORD >
{
	// Sorted array of links to scripts.
	int CompareKey( DWORD id, CScriptIndexLink * pBase ) const
	{
		ASSERT( pBase );
		return( id - pBase->GetIndexID());
	}
};

struct CStringSortArray : public CGObSortArray< TCHAR*, TCHAR* >
{
	// Sorted array of strings
	int CompareKey( TCHAR* pszID1, TCHAR* pszID2 ) const
	{
		ASSERT( pszID2 );
		return( strcmpi( pszID1, pszID2));
	}

	void AddSortString( const TCHAR * pszText )
	{
		ASSERT(pszText);
		int len = strlen( pszText );
		TCHAR * pNew = new TCHAR [ len + 1 ];
		strcpy( pNew, pszText );
		AddSortKey( pNew, pNew );
	}
};

class CServerArray : public CGObSortArray< CServRef*, const TCHAR* >
{
	int CompareKey( const TCHAR* pszID, CServRef* pServ ) const
	{
		ASSERT( pszID );
		ASSERT( pServ );
		return( strcmpi( pszID, pServ->GetName()));
	}
};

class CWebPageDef : public CScriptObj
{
	// This is a single web page we are generating.
	static const TCHAR * sm_KeyTable[];
private:
	int  m_iUpdatePeriod;	// How often to update the web page. (secs)
	time_t  m_NextUpdateTime;
	CGString m_sSrcFilePath;
	CGString m_sDstFilePath;
	CGString m_sClientListFormat;
	CGString m_sServListFormat;
public:
	static int sm_iServListIndex;
private:
	void OutputServList( CFileText & FileOut, CServRef ** ppArray, int iSize );
public:
	const TCHAR * GetName() const
	{
		return( m_sSrcFilePath );
	}
	CWebPageDef( int id );
	bool r_LoadVal( CScript & s );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc = NULL );
	void WebPageUpdate( bool fNow );
};

extern class CServer : public CGSocket, public CServRef, public CTextConsole	// Stuff NOT saved in the world scp file.
{
	static const TCHAR * sm_KeyStatsTable[];
	static const TCHAR * sm_KeyTable[];

private:
	// My base script files that are used frequently.
	CScript m_Scripts[ SCPFILE_QTY ];

public:
	int  m_iExitCode;  // Just some error code to return to system.
	WORD m_wExitFlag;	// identifies who caused the exit.

	// Begin INI file options.
	bool m_fUseNTService;
	int  m_iPollServers;		// background polling of peer servers. (minutes)
	CGString m_sRegisterServer;	// GRAY_MAIN_SERVER
	bool m_fMainLogServer;		// This is the main log server. Will list any server that polls.
	int  m_iMapCacheTime;		// Time in sec to keep unused map data.
	int	 m_iSectorSleepMask;	// The mask for how long sectors will sleep.

	CGString m_sWorldBaseDir;	// "e:\graysvr\worldsave\"
	CGString m_sAcctBaseDir;		// Where do the account files go/come from ?
	CGString m_sSCPBaseDir;		// if we want to get *.SCP files from elsewhere.
	CGString m_sSpeechBaseDir;	// Where do the speech script files go ?
	CGString m_sChangedBaseDir;	// Take files from here and replace existing files.

	bool m_fSecure;     // Secure mode. (will trap exceptions)
	int  m_iFreezeRestartTime;	// # seconds before restarting.
#define DEBUGF_NPC_EMOTE		0x01
#define DEBUGF_ADVANCE_STATS	0x02
#define DEBUGF_MOTIVATION		0x04	// display motication level debug messages.
#define DEBUGF_WALKCODES		0x80	// try the new walk code checking stuff.
	WORD m_wDebugFlags;			// DEBUG In game effects to turn on and off.

	// Decay
	int  m_iDecay_Item;			// Base decay time in minutes.
	int  m_iDecay_CorpsePlayer;	// Time for a playercorpse to decay (mins).
	int  m_iDecay_CorpseNPC;	// Time for a NPC corpse to decay.

	// Save
	int  m_iSavePeriod;			// Minutes between saves.
	int  m_iSaveBackupLevels;	// How many backup levels.
	int  m_iSaveBackgroundTime;	// Speed of the background save in minutes.
	bool m_fSaveGarbageCollect;	// Always force a full garbage collection.
	bool m_fSaveInBackground;	// Do background save stuff.
	bool m_fSaveBinary;			// ??? Not ready yet.

	// Account
	bool m_fRequireEmail;		// Valid Email required to leave GUEST mode.
	int  m_iDeadSocketTimeMin;
	bool m_fArriveDepartMsg;    // General switch to turn on/off arrival/depart messages.
	int  m_nClientsMax;			// Maximum (FD_SETSIZE) open connections to server
	int  m_nGuestsMax;			// Allow guests who have no accounts ?
	int  m_iClientLingerTime;	// How long logged out clients linger in seconds.
	int  m_iMinCharDeleteTime;	// How old must a char be ? (minutes)
	CCryptBase m_ClientVersion;		// GRAY_CLIENT_VER 12537
	int  m_iMaxCharsPerAccount;	// MAX_CHARS_PER_ACCT

	// Magic
	bool m_fReagentsRequired;
	bool m_fWordsOfPowerPlayer; // Words of Power for players
	bool m_fWordsOfPowerStaff;	// Words of Power for staff
	bool m_fEquippedCast;		// Allow casting while equipped.
	bool m_fReagentLossFail;	// ??? Lose reags when failed.
	int m_iMagicUnlockDoor;  // 1 in N chance of magic unlock working on doors -- 0 means never
	//ITEMID_TYPE m_iSpell_Teleport_Effect_Player;
	//SOUND_TYPE m_iSpell_Teleport_Sound_Player;
	ITEMID_TYPE m_iSpell_Teleport_Effect_Staff;
	SOUND_TYPE m_iSpell_Teleport_Sound_Staff;

	// In Game Effects
	int	 m_iLightDungeon;
	int  m_iLightDay;		// Outdoor light level.
	int  m_iLightNight;		// Outdoor light level.
	bool m_fMonsterFight;	// Will creatures fight amoung themselves.
	bool m_fMonsterFear;	// will they run away if hurt ?
	int	 m_iBankIMax;	// Maximum number of items allowed in bank.
	int  m_iBankWMax;	// Maximum weight in WEIGHT_UNITS stones allowed in bank.
	bool m_fGuardsInstantKill;	// Will guards kill instantly or follow normal combat rules?
	int  m_iSnoopCriminal;		// 1 in # chance of getting criminalflagged when succesfully snooping.
	int  m_iTrainSkillPercent;	// How much can NPC's train up to ?
	int	 m_iTrainSkillMax;
	bool m_fCharTags;			// Put [NPC] tags over chars.
	int  m_iVendorMaxSell;		// Max things a vendor will sell in one shot.
	int  m_iGameMinuteLength;	// Length of the game world minute in real world (TICK_PER_SEC) seconds.
	bool m_fFlipDroppedItems;	// Flip dropped items.
	bool m_fNoWeather;			// Turn off all weather.
	int  m_iMaxSumOfStats;		// Stat Cap
	int  m_iAvgSumOfStats;		// Stat Avg. decay can set in here.
	int  m_iMurderMinCount;		// amount of murders before we get title.
	int	 m_iMurderDecayTime;	// (minutes) Roll murder counts off this often.
	int  m_iMaxComplexity;		// How many chars per area.
	int	 m_iPlayerKarmaNeutral;	// How much bad karma makes a player neutral?
	int	 m_iPlayerKarmaEvil;
	int	 m_iGuardLingerTime;	// How long do guards linger about.
	int  m_iCriminalTimer;		// How many minutes are criminals flagged for?
	int  m_iHitpointPercentOnRez;// How many hitpoints do they get when they are rez'd?
	bool m_fLootingIsACrime;	// Looting a blue corpse is bad.
	bool m_fHelpingCriminalsIsACrime;// If I help (rez, heal, etc) a criminal, do I become one too?
	bool m_fPlayerGhostSounds;	// Do player ghosts make a ghostly sound?
	bool m_fAutoNewbieKeys;		// Are house and boat keys newbied automatically?
	int  m_iMaxBaseSkill;		// Maximum value for base skills at char creation
	int  m_iStamRunningPenalty;		// Weight penalty for running (+N% of max carry weight)
	int  m_iStaminaLossAtWeight;	// %Weight at which characters begin to lose stamina

private:
	// Web status pages.
	CGObArray <CWebPageDef*> m_WebPages;

	// End INI file options.

public:

	// admin console.
	int m_iAdminClients;
	CGString m_sConsoleText;

	time_t m_Clock_Shutdown;	// When to perform the shutdowm (g_World.clock)
	time_t m_Clock_Periodic;	// When to perform the next periodic update

	bool m_fResyncPause;		// Server is temporarily halted so files can be updated.

	int m_nGuestsCur;		// How many of the current clients are "guests". Not accurate !
	CGObList m_Clients;		// Current list of clients (CClient)

	// login server stuff. 0 = us.
	CBackTask  m_BackTask;
	CServerArray m_Servers; // Servers list. we act like the login server with this.
private:
	CGObArray< CLogIP *> m_LogIP;	// Block these IP numbers
public:
	// static definition stuff from GRAYTABLE.SCP mostly.
	CGObArray< const CStartLoc* > m_StartDefs; // Start points list
	CGObArray< const CPotionDef* > m_PotionDefs;	 // Defined potions
	CGObArray< const CSpellDef* > m_SpellDefs;	// Defined Spells
	CGTypedArray<CValStr,CValStr&> m_SkillKeySort;
	CGObArray< const CSkillDef* > m_SkillDefs;	// Defined Skills

	CGObArray< const CSkillClassDef* > m_SkillClassDefs;	// Defined Skill classes
	CAdvanceDef m_StatAdv[STAT_BASE_QTY]; // OSI defined "skill curve"

	CGObArray< const CRandGroupDef* > m_SpawnGroupDefs;
	CGObArray< const COreDef* > m_OreDefs;		// Defined ores/materials.

	CGObArray< CFragmentFile* > m_FragFiles;	// Speech/Motive/Event files list
	CGObArray< CFragmentDef* > m_FragDefs;		// Speech/Motive/Event fragment list

	CScriptIndexArray m_PlotScpLinks;
	CScriptIndexArray m_TrigScpLinks;

	CGObArray< TCHAR* > m_Runes;	// Words of power. (A-Z)
	CGObArray< TCHAR* > m_NotoTitles;	// Noto titles.
	CStringSortArray m_Obscene;	// Bad Names.
	CStringSortArray m_PrivCommands[PLEVEL_QTY];	// Noto titles.

	// Type definition information.
	CBaseBaseArray m_ItemBase;		// GRAYITEM.SCP	CItemBase and CItemBaseDupe
	CBaseBaseArray m_CharBase;		// GRAYCHAR.SCP

	CProfileData m_Profile;	// the current active statistical profile.
	CChat m_Chats;

#ifdef _DEBUG
	CPotionDef* m_ppPotionDefs[ POTION_QTY ];	 // Defined potions
	CSpellDef* m_ppSpellDefs[ SPELL_QTY ];	// Defined Spells
	CSkillDef* m_ppSkillDefs[ SKILL_QTY ];	// Defined Skills
#endif

private:
	bool LoadDefs();
	bool LoadTables();
	bool LoadScripts();

	void SetSignals();
	void LoadChangedFiles();
	void UpdatePeriodic( bool fNow );

	bool CmdScriptCheck( const TCHAR *pszName );
public:
	CServer();
	~CServer();

	void Shutdown( int iMinutes );
	bool LoadIni();
	bool CanRunBackTask() const;
	bool IsLoading() const
	{
		return( m_iExitCode > 0 || m_wExitFlag );
	}

	// Some data can be shared amoung multiple tasks.
	// Protection against multiple access.
	void EnterCriticalSection();
	void LeaveCriticalSection();

	// Accounts
private:
	CLogIP * FindLogIP( in_addr dwIP, bool fCreate );

public:
	bool CheckLogIPBlocked( in_addr IP, bool fPreAccept );
	bool SetLogIPBlock( const TCHAR * pszIP, bool fBlock );

	bool SocketsInit(); // Initialize sockets
	void SocketsReceive();
	void SocketsFlush();
	void SocketsClose();

	SKILL_TYPE FindSkillKey( const TCHAR * pKey ) const;
	static STAT_TYPE FindStatKey( const TCHAR * pKey );
	bool Load();
	void Unload();

	void SysMessage( const TCHAR * pMsg ) const;
	void PrintStr( const TCHAR * pStr ) const;
	int  PrintPercent( long iCount, long iTotal );

	bool r_GetRef( const TCHAR * & pszKey, CScriptObj * & pRef, CTextConsole * pSrc );
	bool r_WriteVal( const TCHAR * pKey, CGString & sVal, CTextConsole * pSrc = NULL );
	bool r_LoadVal( CScript & s );
	bool r_Verb( CScript & s, CTextConsole * pSrc );

	const TCHAR * GetStatusString( BYTE iIndex = 0 ) const;
	const TCHAR * GetStatusStringRegister( bool fFirst ) const;
	int GetAgeHours() const;

	bool IsConsoleCmd( TCHAR ch ) const
	{
		return( ch == '/' || ch == '.' );
	}

	bool OnConsoleCmd( CGString & sText, CTextConsole * pSrc );
	bool OnServCmd( TCHAR * pszCmd, TCHAR * pszArgs, CTextConsole * pSrc );

	void OnTick();

private:
	void ListServers( CTextConsole * pConsole ) const;
	void SetResyncPause( bool fPause, CTextConsole * pSrc );

public:
	void ListClients( CTextConsole * pClient ) const;
	CScript * GetScpFile( SCPFILE_TYPE i )
	{
		ASSERT( i < SCPFILE_QTY );
		return( &m_Scripts[i] );
	}
	bool SetScpFile( SCPFILE_TYPE i, const TCHAR * pszName );

	CScript * ScriptLock( CScriptLock & s, SCPFILE_TYPE i, const TCHAR * pszSection = NULL, WORD wModeFlags = 0 );
	bool ScriptLockPlot( CScriptLock & s, int index );
	bool ScriptLockTrig( CScriptLock & s, ITEM_TYPE index );

	const COreDef * GetOreEntry( int iMiningSkill ) const;
	const COreDef * GetOreColor( COLOR_TYPE color ) const;

	CClient * GetClientHead() const
	{
		return( STATIC_CAST <CClient*>( m_Clients.GetHead()));
	}

	bool CommandLine( int argc, TCHAR * argv[] );

	static bool IsValidEmailAddressFormat( const TCHAR * pszText );
	bool IsObscene( const TCHAR * pszText ) const;

	const TCHAR * GetName() const { return( CServRef::GetName()); }
	PLEVEL_TYPE GetPrivLevel() const { return( PLEVEL_Admin ); }

} g_Serv;	// current state stuff not saved.

extern class CMain : public CThread
{
public:
#ifdef _WIN32
	static void _cdecl EntryProc( void * lpThreadParameter );
#else
	static void * _cdecl EntryProc( void * lpThreadParameter );
#endif
} g_Main;

//////////////////////////////////////////////////////////////

// extern const CWeaponBase Item_Weapons[]; // ???

extern const TCHAR * g_szServerDescription;
extern const TCHAR * g_Stat_Name[STAT_QTY];
extern const CPointMap g_pntLBThrone; // This is OSI's origin for degree, sextant minute coordinates
extern const WORD g_Item_Horse_Mounts[][2];

///////////////////////////////////////////////////////////////
// -CObjUID

inline CObjBase * CObjUIDBase::ObjFind() const
{
	return( g_World.FindUID( m_Val & UID_MASK ));
}
inline CItem * CObjUIDBase::ItemFind() const
{
	// IsItem() may be faster ?
	return( dynamic_cast <CItem *>( ObjFind()));
}
inline CChar * CObjUIDBase::CharFind() const
{
	return( dynamic_cast <CChar *>( ObjFind()));
}

#endif	// _GRAYSVR_H_
