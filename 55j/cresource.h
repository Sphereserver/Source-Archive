//
// CResource.h
//

#ifndef _INC_CRESOURCE_H
#define _INC_CRESOURCE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../common/cassoc.h"

class CAccount;
class CClient;
class CLogIP;
class CServerDef;

typedef CServerDef * CServerRef; // CThreadLockRef

enum BODYPART_TYPE
{
	ARMOR_HEAD = 0,
	ARMOR_NECK,
	ARMOR_BACK,
	ARMOR_CHEST,	// or thorax
	ARMOR_ARMS,
	ARMOR_HANDS,
	ARMOR_LEGS,
	ARMOR_FEET,
	ARMOR_QTY,		// All the parts that armor will cover.

	BODYPART_LEGS2,	// Alternate set of legs (spider)
	BODYPART_TAIL,	// Dragon, Snake, Alligator, etc. (tail attack?)
	BODYPART_WINGS,	// Dragon, Mongbat, Gargoyle
	BODYPART_CLAWS,	// can't wear any gloves here!
	BODYPART_HOOVES,	// No shoes
	BODYPART_HORNS,	// Bull, Daemon

	BODYPART_STALKS,		// Gazer or Corpser
	BODYPART_BRANCHES,	// Reaper.
	BODYPART_TRUNK,		// Reaper.
	BODYPART_PSEUDOPOD,	// Slime
	BODYPART_ABDOMEN,		// Spider or insect. asusme throax and chest are the same.

	BODYPART_QTY,
};

#define DAMAGE_GOD			0x0001	// Nothing can block this.
#define DAMAGE_HIT_BLUNT	0x0002	// Physical hit of some sort.
#define DAMAGE_MAGIC		0x0004	// Magic blast of some sort. (we can be immune to magic to some extent)
#define DAMAGE_POISON		0x0008	// Or biological of some sort ? (HARM spell)
#define DAMAGE_FIRE			0x0010	// Fire damage of course.  (Some creatures are immune to fire)
#define DAMAGE_ELECTRIC		0x0020	// lightning.
#define DAMAGE_DRAIN		0x0040	// level drain = negative energy.
#define DAMAGE_GENERAL		0x0080	// All over damage. As apposed to hitting just one point.
#define DAMAGE_ACID			0x0100	// Corrosive will destroy armor.
#define DAMAGE_COLD			0x0200	// Cold freezing damage
#define DAMAGE_HIT_SLASH	0x0400	// sword
#define DAMAGE_HIT_PIERCE	0x0800	// spear.

typedef WORD DAMAGE_TYPE;		// describe a type of damage.

///////////////////////////////////////

struct CValueRangeDef
{
	// Simple linearity
public:
	int m_iLo;
	int m_iHi;
public:
	void Init()
	{
		m_iLo = INT_MIN;
		m_iHi = INT_MIN;
	}
	int GetRange() const
	{
		return( m_iHi - m_iLo );
	}
	int GetLinear( int iPercent ) const
	{	
		// ARGS: iPercent = 0-1000
		return( m_iLo + IMULDIV( GetRange(), iPercent, 1000 ));
	}
	int GetRandom() const
	{	
		return( m_iLo + Calc_GetRandVal( GetRange()));
	}
	int GetRandomLinear( int iPercent ) const;
	bool Load( TCHAR * pszDef );
	const TCHAR * Write() const;
	CValueRangeDef()
	{
		Init();
	}
};

struct CValueCurveDef
{
	// Describe an arbitrary curve.
	// for a range from 0.0 to 100.0 (1000)
	// May be a list of probabilties from 0 skill to 100.0% skill.
public:
	CGTypedArray<int,int> m_aiValues;		// 0 to 100.0 skill levels
public:
	void Init()
	{
		m_aiValues.Empty();
	}
	bool Load( TCHAR * pszDef );
	const TCHAR * Write() const;
	int GetLinear( int iSkillPercent ) const;
	int GetChancePercent( int iSkillPercent ) const;
};

class CCharRefArray
{
private:
	// List of Players and NPC's involved in the quest/party/account etc..
	CGTypedArray< CGrayUID, CGrayUID> m_uidCharArray;
public:
	int FindChar( const CChar * pChar ) const;
	bool IsCharIn( const CChar * pChar ) const
	{
		return( FindChar( pChar ) >= 0 );
	}
	int AttachChar( const CChar * pChar );
	void DetachChar( int i );
	int DetachChar( const CChar * pChar );
	void DeleteChars();
	int GetCharCount() const
	{
		return( m_uidCharArray.GetCount());
	}
	CGrayUID GetChar( int i ) const
	{
		return( m_uidCharArray[i] );
	}
	void WritePartyChars( CScript & s );
};

class CQuestDef : public CResourceDef
{
	// RES_QUEST
	// Quests are scripted things that run groups of NPC's and players.
public:
	static LPCTSTR const sm_szLoadKeys[];

	CGString m_sName;	// List is sorted by name of the quest. This is the TAG.NAME on player?
	int m_iState;		// What is the STATE of the quest.
	DWORD m_dwFlags;	// Arbitrary flags for the quest.
	CCharRefArray m_Chars;	// Chars involved in the quest.

	// These generate Titles that may be listed for the players in Profiles.

public:
	LPCTSTR GetName() const
	{
		// ( every CScriptObj must have at least a type name )
		return( m_sName );
	}
	CQuestDef();
	virtual ~CQuestDef()
	{
	}
};

class CRegionResourceDef : public CResourceDef
{
	// RES_REGIONRESOURCE
	// When mining/lumberjacking etc. What can we get?
protected:
	DECLARE_MEM_DYNAMIC;
public:
	static LPCTSTR const sm_szLoadKeys[];

	// What item do we get when we try to mine this.
	ITEMID_TYPE m_ReapItem;	// ITEMID_ORE_1 most likely
	CValueRangeDef m_ReapAmount;	// How much can we reap at one time (based on skill)

	CValueRangeDef m_Amount;		// How is here total
	CValueRangeDef m_Skill;			// Skill levels required to mine this.
	int m_iRegenerateTime;			// TICK_PER_SEC once found how long to regen this type.

public:
	CRegionResourceDef( RESOURCE_ID rid );
	virtual ~CRegionResourceDef();
	bool r_LoadVal( CScript & s );
	void r_DumpLoadKeys( CTextConsole * pSrc );
	bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc = NULL );
};

enum WEBPAGE_TYPE
{
	WEBPAGE_TEMPLATE,
	WEBPAGE_TEXT,
	WEBPAGE_BMP,
	WEBPAGE_GIF,
	WEBPAGE_JPG,
	WEBPAGE_BIN,	// just some other binary format.
	WEBPAGE_QTY,
};

enum WTRIG_TYPE
{
	// XTRIG_UNKNOWN	= some named trigger not on this list.
	WTRIG_Load=1,
	WTRIG_QTY,
};

class CWebPageDef : public CResourceLink
{
	// RES_WEBPAGE

	// This is a single web page we are generating or serving.
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
	static LPCTSTR const sm_szPageType[];
	static LPCTSTR const sm_szPageExt[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	WEBPAGE_TYPE m_type;				// What basic format of file is this ? 0=text
	CGString m_sSrcFilePath;	// source template for the generated web page.
private:
	PLEVEL_TYPE m_privlevel;	// What priv level to see this page ?

	// For files that are being translated and updated.
	CGString m_sDstFilePath;	// where is the page served from ?
	int  m_iUpdatePeriod;	// How often to update the web page. 0 = never.
	int  m_iUpdateLog;		// create a daily log of the page.
	CServTime  m_timeNextUpdate;

	// details for generating a page.
	CGString m_sClientListFormat;	// Old junk. get rid of this.
	CGString m_sServListFormat;		// Old junk. get rid of this.

public:
	static int sm_iListIndex;
	static LPCTSTR const sm_szTrigName[WTRIG_QTY+1];
private:
	int ServPageRequest( CClient * pClient, LPCTSTR pszURLArgs, CGTime * pdateLastMod );
public:
	LPCTSTR GetName() const
	{
		return( m_sSrcFilePath );
	}
	LPCTSTR GetDstName() const
	{
		return( m_sDstFilePath );
	}
	void SetPageType( WEBPAGE_TYPE iType )
	{
		m_type = iType;
	}
	bool IsMatch( LPCTSTR IsMatchPage ) const;

	bool SetSourceFile( LPCTSTR pszName, CClient * pClient );
	bool ServPagePost( CClient * pClient, LPCTSTR pszURLArgs, TCHAR * pPostData, int iContentLength );

	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc = NULL );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );	// some command on this object as a target

	void WebPageLog();
	bool WebPageUpdate( bool fNow, LPCTSTR pszDstName, CTextConsole * pSrc );

	static bool ServPage( CClient * pClient, TCHAR * pszPage, CGTime * pdateLastMod );

	CWebPageDef( RESOURCE_ID id );
	virtual ~CWebPageDef()
	{
	}
};

class CSpellDef : public CResourceDef	// 1 based spells. See SPELL_*
{
	// RES_SPELL
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
#define SPELLFLAG_RESIST	0x0400	// Allowed to resist this.	
#define SPELLFLAG_DISABLED	0x8000
	CGString m_sName;	// spell name

public:
	static LPCTSTR const sm_szLoadKeys[];
	SOUND_TYPE m_sound;			// What noise does it make when done.
	CGString m_sRunes;			// Letter Runes for Words of power.
	CResourceQtyArray m_Reags;	// What reagents does it take ?
	CResourceQtyArray m_SkillReq;	// What skills/unused reagents does it need to cast.
	ITEMID_TYPE m_idSpell;		// The rune graphic for this.
	ITEMID_TYPE m_idScroll;		// The scroll graphic item for this.
	ITEMID_TYPE m_idEffect;	// Animation effect ID
	WORD m_wManaUse;			// How much mana does it need.
	int m_iCastTime;			// In TICK_PER_SEC.
	CValueCurveDef m_Effect;	// Damage or effect level based on skill of caster.100% magery
	CValueCurveDef m_Duration;	// length of effect. in TICK_PER_SEC
public:
	bool IsSpellType( WORD wFlags ) const
	{
		return(( m_wFlags & wFlags ) ? true : false );
	}
	CSpellDef( SPELL_TYPE id, CScript & s );
	virtual ~CSpellDef()
	{
	}
	LPCTSTR GetName() const { return( m_sName ); }
	bool r_LoadVal( CScript & s );
};

class CRandGroupDef	: public CResourceLink // A spawn group.
{
	// RES_SPAWN or RES_REGIONTYPE
protected:
	DECLARE_MEM_DYNAMIC;
private:
	static LPCTSTR const sm_szLoadKeys[];
	int m_iTotalWeight;
	CResourceQtyArray m_Members;
private:
	int CalcTotalWeight();
public:
	CRandGroupDef( RESOURCE_ID rid ) :
		CResourceLink( rid )
	{
		m_iTotalWeight = 0;
	}
	virtual ~CRandGroupDef()
	{
	}
	virtual bool r_LoadVal( CScript & s );
	int GetTotalWeight() const
	{
		return m_iTotalWeight;
	}
	int GetRandMemberIndex() const;
	CResourceQty GetMember( int i ) const
	{
		return( m_Members[i] );
	}
	RESOURCE_ID GetMemberID( int i ) const
	{
		return( m_Members[i].GetResourceID() );
	}
	int GetMemberWeight( int i ) const
	{
		return( m_Members[i].GetResQty() );
	}
};

enum STAT_TYPE	// Character stats
{
	STAT_NONE = -1,
	STAT_STR = 0,
	STAT_INT,
	STAT_DEX,
	STAT_BASE_QTY,
	STAT_FoodRegen = 3,	// just used as a regen rate. (as karma does not decay)

	// Notoriety.
	STAT_Karma = 3,		// -10000 to 10000 - also used as the food consumption main timer.
	STAT_Fame,
	STAT_QTY,
};

class CSkillClassDef : public CResourceLink // For skill def table
{
	// Similar to character class.
	// RES_SKILLCLASS
	static LPCTSTR const sm_szLoadKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CGString m_sName;	// The name of this skill class.

	WORD m_StatSumMax;
	WORD m_SkillSumMax;

	WORD m_StatMax[STAT_BASE_QTY];	// STAT_BASE_QTY
	WORD m_SkillLevelMax[ SKILL_QTY ];

private:
	void Init();
public:
	CSkillClassDef( RESOURCE_ID rid ) :
		CResourceLink( rid )
	{
		// If there was none defined in scripts.
		Init();
	}
	virtual ~CSkillClassDef()
	{
	}

	LPCTSTR GetName() const { return( m_sName ); }

	void r_DumpLoadKeys( CTextConsole * pSrc );
	bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	bool r_LoadVal( CScript & s );
};

enum SKTRIG_TYPE
{
	// All skills may be scripted.
	// XTRIG_UNKNOWN	= some named trigger not on this list.
	SKTRIG_ABORT=1,	// Some odd thing went wrong.
	SKTRIG_FAIL,	// we failed the skill check.
	SKTRIG_SELECT,	// just selecting params for the skill
	SKTRIG_START,	// params for skill are done. (stroke)
	SKTRIG_STROKE,	
	SKTRIG_SUCCESS,
	SKTRIG_QTY,
};

struct CSkillDef : public CResourceLink // For skill def table
{
	// RES_SKILL
	static LPCTSTR const sm_szTrigName[SKTRIG_QTY+1];
	static LPCTSTR const sm_szLoadKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sKey;	// script key word for skill.
public:
	CGString m_sTitle;	// title one gets if it's your specialty.
	CGString m_sTargetPrompt;	// targetting prompt. (if needed)

	CValueCurveDef m_Delay;	//	The base delay for the skill. (tenth of seconds)
	CValueCurveDef m_Effect;	// depends on skill

	// Stat effects.
	// You will tend toward these stat vals if you use this skill a lot.
	BYTE m_Stat[STAT_BASE_QTY];	// STAT_STR, STAT_INT, STAT_DEX
	BYTE m_StatPercent; // BONUS_STATS = % of success depending on stats
	BYTE m_StatBonus[STAT_BASE_QTY]; // % of each stat toward success at skill, total 100

	CValueCurveDef m_AdvRate;		// ADV_RATE defined "skill curve" 0 to 100 skill levels.
	CValueCurveDef m_Values;	// VALUES= influence for items made with 0 to 100 skill levels.

	// Delay before skill complete. modified by skill level of course !
public:
	CSkillDef( SKILL_TYPE iSkill );
	virtual ~CSkillDef()
	{
	}
	LPCTSTR GetKey() const
	{
		return( m_sKey );
	}

	LPCTSTR GetName() const { return( GetKey()); }
	bool r_LoadVal( CScript & s );
	void r_DumpLoadKeys( CTextConsole * pSrc );
	bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
};

class CSkillKeySortArray : public CGObSortArray< CValStr*, LPCTSTR >
{
	int CompareKey( LPCTSTR pszKey, CValStr * pVal ) const
	{
		ASSERT( pszKey );
		ASSERT( pVal->m_pszName );
		return( strcmpi( pszKey, pVal->m_pszName ));
	}
};

struct CMultiDefArray : public CGObSortArray< CGrayMulti*, MULTI_TYPE >
{
	// store the static components of a IT_MULTI
	// Sorted array
	int CompareKey( MULTI_TYPE id, CGrayMulti* pBase ) const
	{
		ASSERT( pBase );
		return( id - pBase->GetMultiID());
	}
};

struct CThreadLockableDefArray : public CObNameSortArray, public CThreadLockableObj
{
	bool DeleteOb( CScriptObj* pObj )
	{
		CThreadLockRef lock( this );
		return CObNameSortArray::DeleteOb( pObj );
	}
	int AddSortKey( CScriptObj* pNew, LPCTSTR key )
	{
		CThreadLockRef lock( this );
		return( CObNameSortArray::AddSortKey( pNew, key ));
	}

	// FindKey, RemoveAt and DeleteAt must use a lock outside as well ! (for index to be meaningful)
};

extern class CResource : public CResourceBase
{
	// Script defined resources (not saved in world file)
	static const CAssocReg sm_szLoadKeys[];

public:
	CServTime m_timePeriodic;	// When to perform the next periodic update

	// Begin INI file options.
	bool m_fUseANSI;				// console can use ANSI output.
	bool m_fUseNTService;
	bool m_fUseIRC;
	int  m_iPollServers;		// background polling of peer servers. (minutes)
	CGString m_sRegisterServer;	// GRAY_MAIN_SERVER
	CGString m_sMainLogServerDir;	// This is the main log server Directory. Will list any server that polls.
	int  m_iMapCacheTime;		// Time in sec to keep unused map data.
	int	 m_iSectorSleepMask;	// The mask for how long sectors will sleep.

	CGString m_sWorldBaseDir;	// "e:\graysvr\worldsave\" = world files go here.
	CGString m_sAcctBaseDir;	// Where do the account files go/come from ?
	CGString m_sSCPInBoxDir;	// Take files from here and replace existing files.

	bool m_fSecure;				// Secure mode. (will trap exceptions)
	int  m_iFreezeRestartTime;	// # seconds before restarting.
#define DEBUGF_NPC_EMOTE		0x0001
#define DEBUGF_ADVANCE_STATS	0x0002
#define DEBUGF_MOTIVATION		0x0004	// display motication level debug messages.
#define DEBUGF_SKILL_TIME		0x0010	// Show the amount of time til skill exec.
#define DEBUGF_WALKCODES		0x0080	// try the new walk code checking stuff.
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

	// Account
	bool m_fRequireEmail;		// Valid Email required to leave GUEST mode.
	int  m_iDeadSocketTime;
	bool m_fArriveDepartMsg;    // General switch to turn on/off arrival/depart messages.
	int  m_iClientsMax;			// Maximum (FD_SETSIZE) open connections to server
	int  m_iGuestsMax;			// Allow guests who have no accounts ?
	int  m_iClientLingerTime;	// How long logged out clients linger in seconds.
	int  m_iMinCharDeleteTime;	// How old must a char be ? (minutes)
	int  m_iMaxCharsPerAccount;	// MAX_CHARS_PER_ACCT
	bool m_fLocalIPAdmin;		// The local ip is the admin ?

	CServerRef m_pServAccountMaster;	// Verify my accounts through here.

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
	int  m_iGameMinuteLength;	// Length of the game world minute in real world (TICK_PER_SEC) seconds.
	bool m_fNoWeather;			// Turn off all weather.
	bool m_fCharTags;			// Put [NPC] tags over chars.
	bool m_fFlipDroppedItems;	// Flip dropped items.
	bool m_fMonsterFight;	// Will creatures fight amoung themselves.
	bool m_fMonsterFear;	// will they run away if hurt ?
	int	 m_iBankIMax;	// Maximum number of items allowed in bank.
	int  m_iBankWMax;	// Maximum weight in WEIGHT_UNITS stones allowed in bank.
	int  m_iVendorMaxSell;		// Max things a vendor will sell in one shot.
	int  m_iMaxCharComplexity;		// How many chars per sector.
	int  m_iMaxItemComplexity;		// How many items per meter.
	bool m_fPlayerGhostSounds;	// Do player ghosts make a ghostly sound?
	bool m_fAutoNewbieKeys;		// Are house and boat keys newbied automatically?
	int  m_iStamRunningPenalty;		// Weight penalty for running (+N% of max carry weight)
	int  m_iStaminaLossAtWeight;	// %Weight at which characters begin to lose stamina
	int  m_iHitpointPercentOnRez;// How many hitpoints do they get when they are rez'd?
	int  m_iMaxBaseSkill;		// Maximum value for base skills at char creation
	int  m_iTrainSkillPercent;	// How much can NPC's train up to ?
	int	 m_iTrainSkillMax;
	int	 m_iMountHeight;		// The height at which a mounted person clears ceilings.
	int  m_iArcheryMinDist;

	// Criminal/Karma
	bool m_fGuardsInstantKill;	// Will guards kill instantly or follow normal combat rules?
	int	 m_iGuardLingerTime;	// How long do guards linger about.
	int  m_iSnoopCriminal;		// 1 in # chance of getting criminalflagged when succesfully snooping.
	int  m_iMurderMinCount;		// amount of murders before we get title.
	int	 m_iMurderDecayTime;	// (minutes) Roll murder counts off this often.
	bool m_fHelpingCriminalsIsACrime;// If I help (rez, heal, etc) a criminal, do I become one too?
	bool m_fLootingIsACrime;	// Looting a blue corpse is bad.
	int  m_iCriminalTimer;		// How many minutes are criminals flagged for?
	int	 m_iPlayerKarmaNeutral;	// How much bad karma makes a player neutral?
	int	 m_iPlayerKarmaEvil;

	// End INI file options.

	CResourceScript m_scpIni;	// Keep this around so we can link to it.
private:
	CResourceScript m_scpTables;

	CGObArray< CLogIP *> m_LogIP;	// Block these IP numbers
	CStringSortArray m_Obscene;	// Bad Names/Words etc.
	CGObArray< TCHAR* > m_NotoTitles;	// Noto titles.
	CGObArray< TCHAR* > m_Runes;	// Words of power. (A-Z)

	CMultiDefArray m_MultiDefs;	// read from the MUL files. Cached here on demand.

	CObNameSortArray m_SkillNameDefs;	// const CSkillDef* Name sorted
	CGPtrTypeArray< CSkillDef* > m_SkillIndexDefs;	// Defined Skills indexed by number
	CGObArray< const CSpellDef* > m_SpellDefs;	// Defined Spells

	CStringSortArray m_PrivCommands[PLEVEL_QTY];	// what command are allowed for a priv level?

public:
	CThreadLockableDefArray m_Servers; // Servers list. we act like the login server with this.
	CObNameSortArray m_Functions;	// subroutines that can be used in scripts.
	CObNameSortArray m_HelpDefs;	// Help on these commands.

	// static definition stuff from *TABLE.SCP mostly.
	CGObArray< const CStartLoc* > m_StartDefs; // Start points list
	CValueCurveDef m_StatAdv[STAT_BASE_QTY]; // "skill curve"
	CGTypedArray<CPointBase,CPointBase&> m_MoonGates;	// The array of moongates.

	CResourceHashArray m_WebPages;	// These can be linked back to the script.

private:
	void LoadChangedFiles();

	RESOURCE_ID ResourceGetNewID( RES_TYPE restype, LPCTSTR pszName, CVarDefNum ** ppVarNum );

public:
	CResource();
	~CResource();

	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	bool r_LoadVal( CScript &s );
	bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	void r_Write( CScript &s );

	bool SaveIni();
	bool LoadIni( bool fTest );
	bool Load( bool fResync );
	void Unload( bool fResync );
	void OnTick( bool fNow );

	bool ResourceTest( LPCTSTR pszFilename );
	bool ResourceTestSort( LPCTSTR pszFilename );
	bool ResourceTestSkills();
	bool ResourceTestItemDupes();
	bool ResourceTestItemMuls();
	bool ResourceTestCharAnims();

	bool LoadResourceSection( CScript * pScript );
	CResourceDef * ResourceGetDef( RESOURCE_ID_BASE rid ) const;

	// Specialized resource accessors.

	bool CanUsePrivVerb( const CScriptObj * pObjTarg, LPCTSTR pszCmd, CTextConsole * pSrc ) const;
	PLEVEL_TYPE GetPrivCommandLevel( LPCTSTR pszCmd ) const;

	CLogIP * FindLogIP( CSocketAddressIP dwIP, bool fCreate );
	bool SetLogIPBlock( LPCTSTR pszIP, bool fBlock );

	static STAT_TYPE FindStatKey( LPCTSTR pszKey );
	static bool IsValidEmailAddressFormat( LPCTSTR pszText );
	bool IsObscene( LPCTSTR pszText ) const;
	CWebPageDef * FindWebPage( LPCTSTR pszPath ) const;

	CServerRef Server_GetDef( int index );

	const CSpellDef* GetSpellDef( SPELL_TYPE index ) const
	{
		if ( ! index || ! m_SpellDefs.IsValidIndex(index))
			return( NULL );
		return( m_SpellDefs[index] );
	}

	LPCTSTR GetSkillKey( SKILL_TYPE index ) const
	{
		return( m_SkillIndexDefs[index]->GetKey());
	}
	const CSkillDef* GetSkillDef( SKILL_TYPE index ) const
	{
		return( m_SkillIndexDefs[index] );
	}
	CSkillDef* GetSkillDef( SKILL_TYPE index )
	{
		return( m_SkillIndexDefs[index] );
	}
	const CSkillDef* FindSkillDef( LPCTSTR pszKey ) const
	{
		// Find the skill name in the alpha sorted list.
		// RETURN: SKILL_NONE = error.
		int i = m_SkillNameDefs.FindKey( pszKey );
		if ( i < 0 )
			return( NULL );
		return( STATIC_CAST <const CSkillDef*>(m_SkillNameDefs[i]));
	}
	SKILL_TYPE FindSkillKey( LPCTSTR pszKey ) const;

	int GetSpellEffect( SPELL_TYPE spell, int iSkillval ) const;

	LPCTSTR GetRune( TCHAR ch ) const
	{
		ch = toupper(ch) - 'A';
		if ( ! m_Runes.IsValidIndex(ch))
			return( _TEXT("?"));
		return( m_Runes[ ch ] );
	}
	LPCTSTR GetNotoTitle( int iLevel ) const
	{
		if ( ! m_NotoTitles.IsValidIndex(iLevel))
		{
			return( _TEXT(""));
		}
		else
		{
			return m_NotoTitles[ iLevel ];
		}
	}

	const CGrayMulti * GetMultiItemDefs( ITEMID_TYPE itemid );

	bool CanRunBackTask() const;
	bool IsConsoleCmd( TCHAR ch ) const;

	CPointMap GetRegionPoint( LPCTSTR pCmd ) const; // Decode a teleport location number into X/Y/Z

	int Calc_MaxCarryWeight( const CChar * pChar ) const;
	int Calc_DropStamWhileMoving( CChar * pChar, int iWeightLoadPercent );
	int Calc_WalkThroughChar( CChar * pCharMove, CChar * pCharObstacle );
	int Calc_CombatAttackSpeed( CChar * pChar, CItem * pWeapon );
	int Calc_CombatChanceToHit( CChar * pChar, SKILL_TYPE skill, CChar * pCharTarg, CItem * pWeapon );
	int Calc_CombatDamage( CChar * pChar, CItem * pWeapon, CChar * pCharTarg );
	int Calc_CharDamageTake( CChar * pChar, CItem * pWeapon, CChar * pCharAttacker, int iDamage, DAMAGE_TYPE DamageType, BODYPART_TYPE LocationHit );
	int Calc_ItemDamageTake( CItem * pItem, CItem * pWeapon, CChar * pCharAttacker, int iDamage, DAMAGE_TYPE DamageType, BODYPART_TYPE LocationHit );
	bool Calc_SkillCheck( int iSkillLevel, int iDifficulty );
	int  Calc_StealingItem( CChar * pCharThief, CItem * pItem, CChar * pCharMark );
	bool Calc_CrimeSeen( CChar * pCharThief, CChar * pCharViewer, SKILL_TYPE SkillToSee, bool fBonus );
	int Calc_FameKill( CChar * pKill );
	int Calc_FameScale( int iFame, int iFameChange );
	int Calc_KarmaKill( CChar * pKill, NOTO_TYPE NotoThem );
	int Calc_KarmaScale( int iKarma, int iKarmaChange );

} g_Cfg;

#endif	// _INC_CRESOURCE_H
