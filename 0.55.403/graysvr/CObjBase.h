//
// CObjBase.h
//

#ifndef _INC_COBJBASE_H
#define _INC_COBJBASE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

enum MEMORY_TYPE
{
	// IT_EQ_MEMORY_OBJ
	// Types of memory a CChar has about a game object. (m_wHue)
	MEMORY_NONE			= 0,
	MEMORY_SAWCRIME		= 0x0001,	// I saw them commit a crime or i was attacked criminally. I can call the guards on them. the crime may not have been against me.
	MEMORY_IPET			= 0x0002,	// I am a pet. (this link is my master) (never time out)
	MEMORY_FIGHT		= 0x0004,	// Active fight going on now. may not have done any damage. and they may not know they are fighting me.
	MEMORY_IAGGRESSOR	= 0x0008,	// I was the agressor here. (good or evil)
	MEMORY_HARMEDBY		= 0x0010,	// I was harmed by them. (but they may have been retaliating)
	MEMORY_IRRITATEDBY	= 0x0020,	// I saw them snoop from me or someone.
	MEMORY_SPEAK		= 0x0040,	// We spoke about something at some point. (or was tamed) (NPC_MEM_ACT_TYPE)
	MEMORY_AGGREIVED	= 0x0080,	// I was attacked and was the inocent party here !
	MEMORY_GUARD		= 0x0100,	// Guard this item (never time out)
	MEMORY_ISPAWNED		= 0x0200,	// I am spawned from this item. (never time out)
	MEMORY_GUILD		= 0x0400,	// This is my guild stone. (never time out) only have 1
	MEMORY_TOWN			= 0x0800,	// This is my town stone. (never time out) only have 1
	MEMORY_FOLLOW		= 0x1000,	// I am following this Object (never time out)
	MEMORY_WAR_TARG		= 0x2000,	// This is one of my current war targets.
	MEMORY_FRIEND		= 0x4000,	// They can command me but not release me. (not primary blame)
	MEMORY_GUMPRECORD	= 0x8000,	// Gump record memory (More1 = Context, More2 = Uid)

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
	// All Instances of CItem or CChar have these base attributes.
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];

private:
	CServTime m_timeout;		// when does this rot away ? or other action. 0 = never, else system time
	HUE_TYPE m_wHue;			// Hue or skin color. (CItems must be < 0x4ff or so)
protected:
	CResourceRef m_BaseRef;	// Pointer to the resource that describes this type.
	CVarDefArray m_TagDefs;		// attach extra tags here.

public:
	CResourceRefArray m_OEvents;
	static int sm_iCount;	// how many total objects in the world ?
	CVarDefArray * GetTagDefs()
	{
		return( &m_TagDefs );
	}

	virtual void DeletePrepare();

protected:
	virtual void DupeCopy( const CObjBase * pObj )
	{
		CObjBaseTemplate::DupeCopy( pObj );
		m_wHue = pObj->GetHue();
		// m_timeout = pObj->m_timeout;
		m_TagDefs.Copy( &( pObj->m_TagDefs ) );
	}

public:
	virtual bool OnTick() = 0;
	virtual int FixWeirdness() = 0;
	virtual int GetWeight( void ) const = 0;
	virtual bool IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwArg ) = 0;

	virtual int IsWeird() const;
	void Delete();

	// Accessors

	virtual WORD GetBaseID() const = 0;
	CBaseBaseDef * Base_GetDef() const
	{
		return( STATIC_CAST <CBaseBaseDef *>( m_BaseRef.GetRef() ));
	}

	void SetUID( DWORD dwVal, bool fItem );
	CObjBase* GetNext() const
	{
		return( STATIC_CAST <CObjBase*>( CGObListRec::GetNext()));
	}
	virtual LPCTSTR GetName() const	// resolve ambiguity w/CScriptObj
	{
		return( CObjBaseTemplate::GetName());
	}
	LPCTSTR GetResourceName() const
	{
		return Base_GetDef()->GetResourceName();
	}

public:
	// Hue
	void SetHue( HUE_TYPE wHue )
	{
		m_wHue = wHue;
	}
	HUE_TYPE GetHue() const
	{
		return( m_wHue );
	}

protected:
	WORD GetHueAlt() const
	{
		// DEBUG_CHECK( IsItemEquipped());
		// IT_EQ_MEMORY_OBJ = MEMORY_TYPE mask
		// IT_EQ_NPC_SCRIPT = previous book script time.
		// IT_EQ_VENDOR_BOX = restock time.
		return( m_wHue );
	}
	void SetHueAlt( HUE_TYPE wHue )
	{
		m_wHue = wHue;
	}

public:
	// Timer
	virtual void SetTimeout( int iDelayInTicks );
	bool IsTimerSet() const
	{
		return( m_timeout.IsTimeValid());
	}
	int GetTimerDiff() const;	// return: < 0 = in the past ( m_timeout - CServTime::GetCurrentTime() )
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

	int GetTimerDAdjusted() const
	{
		// RETURN: time in seconds from now.
		if ( ! IsTimerSet())
			return( -1 );
		int iDiffInTicks = GetTimerDiff();
		if ( iDiffInTicks < 0 )
			return( 0 );
		return( iDiffInTicks );
	}

public:
	// Location
	virtual bool MoveTo( CPointMap pt ) = 0;	// Move to a location at top level.

	virtual void MoveNear( CPointMap pt, int iSteps = 0, WORD wCan = CAN_C_WALK );
	virtual bool MoveNearObj( const CObjBaseTemplate * pObj, int iSteps = 0, WORD wCan = CAN_C_WALK );

	bool SetNamePool( LPCTSTR pszName );

	void Sound( SOUND_TYPE id, int iRepeat = 1 ) const; // Play sound effect from this location.
	void Effect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBase * pSource = NULL, BYTE bspeedseconds = 5, BYTE bloop = 1, bool fexplode = false, DWORD color = 0, DWORD render = 0 ) const;

	void r_WriteSafe( CScript & s );

	virtual void r_Serialize( CGFile & a );	// binary
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual void r_Write( CScript & s );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );	// some command on this object as a target
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	void Emote( LPCTSTR pText, CClient * pClientExclude = NULL, bool fPossessive = false );

	virtual void Speak( LPCTSTR pText, HUE_TYPE wHue = HUE_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_NORMAL );
	virtual void SpeakUTF8( LPCTSTR pText, HUE_TYPE wHue= HUE_TEXT_DEF, TALKMODE_TYPE mode= TALKMODE_SAY, FONT_TYPE font= FONT_NORMAL, CLanguageID lang = 0 );
	virtual void SpeakUTF8Ex( const NWORD * pText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang );

	void RemoveFromView( CClient * pClientExclude = NULL );	// remove this item from all clients.
	void UpdateCanSee( const CCommand * pCmd, int iLen, CClient * pClientExclude = NULL ) const;
	void UpdateObjMessage( LPCTSTR pTextThem, LPCTSTR pTextYou, CClient * pClientExclude, HUE_TYPE wHue, TALKMODE_TYPE mode ) const;

	TRIGRET_TYPE OnHearTrigger( CResourceLock & s, LPCTSTR pCmd, CChar * pSrc, TALKMODE_TYPE & mode );

	bool IsContainer( void ) const;

	virtual void Update( const CClient * pClientExclude = NULL )	// send this new item to clients.
		= 0;
	virtual void Flip( LPCTSTR pCmd = NULL )
		= 0;
	virtual int OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType = DAMAGE_HIT_BLUNT )
		= 0;
	virtual bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel, CItem * pSourceItem )
		= 0;

	virtual TRIGRET_TYPE Spell_OnTrigger( SPELL_TYPE spell, SPTRIG_TYPE stage, CChar * pSrc, CScriptTriggerArgs * pArgs );
	CObjBase( bool fItem );
	virtual ~CObjBase();

	//	Some global object variables
	signed int m_ModAr;
};

enum STONEALIGN_TYPE // Types of Guild/Town stones
{
	STONEALIGN_STANDARD = 0,
	STONEALIGN_ORDER,
	STONEALIGN_CHAOS,
};

enum ITRIG_TYPE
{
	// XTRIG_UNKNOWN = some named trigger not on this list.
	ITRIG_Click=1,
	ITRIG_Create,		// Item is being created.
	ITRIG_DAMAGE,		// I have been damaged in some way
	ITRIG_DCLICK,		// I have been dclicked.
	ITRIG_Destroy,		// Item is being destroyed.
	ITRIG_DROPON_CHAR,		// I have been dropped on this char
	ITRIG_DROPON_GROUND,		// I have been dropped on the ground here
	ITRIG_DROPON_ITEM,		// An item has been 
	ITRIG_DROPON_SELF,		// An item has been dropped upon me
	ITRIG_EQUIP,		// I have been equipped.
	ITRIG_EQUIPTEST,
	ITRIG_PICKUP_GROUND,
	ITRIG_PICKUP_PACK,	// picked up from inside some container.
	ITRIG_PICKUP_SELF,	// picked up from this container
	ITRIG_SPELLEFFECT,		// cast some spell on me.
	ITRIG_STACKON,		// something stacked on top of me.
	ITRIG_STEP,			// I have been walked on. (or shoved)
	ITRIG_TARGON_CANCEL,
	ITRIG_TARGON_CHAR,
	ITRIG_TARGON_GROUND,
	ITRIG_TARGON_ITEM,	// I am being combined with an item
	ITRIG_TIMER,		// My timer has expired.
	ITRIG_ToolTip,
	ITRIG_UNEQUIP,
	ITRIG_QTY,
};

enum ITC_TYPE	// Item Template commands
{
	ITC_BUY,
	ITC_CONTAINER,
	ITC_FULLINTERP,
	ITC_ITEM,
	ITC_ITEMNEWBIE,
	ITC_NEWBIESWAP,
	ITC_SELL,
	ITC_QTY,
};

class CItem : public CObjBase
{
	// RES_WORLDITEM
protected:
	DECLARE_MEM_DYNAMIC;

public:
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
	static LPCTSTR const sm_szTrigName[ITRIG_QTY+1];
	static LPCTSTR const sm_szTemplateTable[ITC_QTY+1];

private:
	WORD m_wDispIndex;		// The current display type. ITEMID_TYPE
	WORD m_amount;		// Amount of items in pile. 64K max (or corpse type)
	IT_TYPE m_type;		// What does this item do when dclicked ? defines dynamic_cast type

public:

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
	CGrayUID m_uidLink;		// Linked to this other object in the world. (owned, key, etc)

	// Type specific info. IT_TYPE
	union // 4(more1) + 4(more2) + 5(morep) = 13 bytes
	{
		// IT_NORMAL
		struct	// used only to save and restore all this junk.
		{
			DWORD m_more1;
			DWORD m_more2;
			CPointBase m_morep;
		} m_itNormal;

		// IT_CONTAINER
		// IT_CONTAINER_LOCKED
		// IT_DOOR
		// IT_DOOR_OPEN
		// IT_DOOR_LOCKED
		// IT_SHIP_SIDE
		// IT_SHIP_SIDE_LOCKED
		// IT_SHIP_PLANK
		// IT_SHIP_HOLD
		// IT_SHIP_HOLD_LOCK
		struct	// IsTypeLockable()
		{
			CGrayUIDBase m_lockUID;		// more1=the lock code. normally this is the same as the uid (magic lock=non UID)
			DWORD m_lock_complexity;	// more2=0-1000 = How hard to pick or magic unlock. (conflict with door ?)
			WORD  m_TrapType;			// morex = poison or explosion. (NOT USED YET)
		} m_itContainer;

		// IT_SHIP_TILLER
		// IT_KEY
		// IT_SIGN_GUMP
		struct
		{
			CGrayUIDBase m_lockUID;		// more1 = the lock code. Normally this is the UID, except if uidLink is set.
		} m_itKey;

		// IT_EQ_BANK_BOX
		struct
		{
			DWORD m_Check_Amount;		// more1=Current amount of gold in account..
			DWORD m_Check_Restock;		// more2= amount to restock the bank account to
			CPointBase m_pntOpen;	// morep=point we are standing on when opened bank box.
		} m_itEqBankBox;

		// IT_EQ_VENDOR_BOX
		struct
		{
			DWORD m_junk1;
			DWORD m_junk2;
			CPointBase m_pntOpen;	// morep=point we are standing on when opened vendor box.
		} m_itEqVendorBox;

		// IT_GAME_BOARD
		struct
		{
			int m_GameType;		// more1=0=chess, 1=checkers, 2= no pieces.
		} m_itGameBoard;

		// IT_WAND
		// IT_WEAPON_*
		struct
		{
			WORD m_Hits_Cur;		// more1l=eqiv to quality of the item (armor/weapon).
			WORD m_Hits_Max;		// more1h=can only be repaired up to this level.
			int  m_spellcharges;	// more2=for a wand etc.
			WORD m_spell;			// morex=SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_spelllevel;		// morey=level of the spell. (0-1000)
			BYTE m_poison_skill;	// morez=0-100 = Is the weapon poisoned ?
		} m_itWeapon;

		// IT_ARMOR
		// IT_ARMOR_LEATHER
		// IT_SHIELD:
		// IT_CLOTHING
		// IT_JEWELRY
		struct
		{
			WORD m_Hits_Cur;		// more1l= eqiv to quality of the item (armor/weapon).
			WORD m_Hits_Max;		// more1h= can only be repaired up to this level.
			int  m_spellcharges;	// more2 = ? spell charges ? not sure how used here..
			WORD m_spell;			// morex = SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_spelllevel;		// morey=level of the spell. (0-1000)
		} m_itArmor;

		// IT_SPELL = a magic spell effect. (might be equipped)
		// IT_FIRE
		// IT_SCROLL
		// IT_COMM_CRYSTAL
		// IT_CAMPFIRE
		// IT_LAVA
		struct
		{
			short m_PolyStr;	// more1l=polymorph effect of this. (on strength)
			short m_PolyDex;	// more1h=polymorph effect of this. (on dex)
			int  m_spellcharges; // more2=not sure how used here..
			WORD m_spell;		// morex=SPELL_TYPE = The magic spell cast on this. (daemons breath)(boots of strength) etc
			WORD m_spelllevel;	// morey=0-1000=level of the spell.
			BYTE m_pattern;		// morez = light pattern - CAN_I_LIGHT LIGHT_QTY
		} m_itSpell;

		// IT_SPELLBOOK
		struct	// Spellbook extra spells.
		{
			DWORD m_spells1;	// more1=Mask of avail spells for spell book.
			DWORD m_spells2;	// more2=Mask of avail spells for spell book.
			DWORD m_spells3;	// morex,morey=necro type spells.
			WORD  m_spells4;	// morez
		} m_itSpellbook;

		// IT_POTION
		struct
		{
			SPELL_TYPE m_Type;		// more1 = potion effect type
			DWORD m_skillquality;	// more2 = 0-1000 Strength of the resulting spell.
			WORD m_tick;			// morex = countdown to explode purple.
		} m_itPotion;

		// IT_MAP
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

		// IT_FRUIT
		// IT_FOOD
		// IT_FOOD_RAW
		// IT_MEAT_RAW
		struct
		{
			ITEMID_TYPE m_cook_id;		// more1=Cooks into this. (only if raw)
			CREID_TYPE m_MeatType;		// more2= Meat from what type of creature ?
			WORD m_spell;				// morex=SPELL_TYPE = The magic spell cast on this. ( effect of eating.)
			WORD m_spelllevel;			// morey=level of the spell. (0-1000)
			BYTE m_poison_skill;		// morez=0-100 = Is poisoned ?
		} m_itFood;

		// IT_CORPSE
		struct	// might just be a sleeping person as well
		{
			CServTime	m_timeDeath;	// more1=Death time of corpse object. 0=sleeping or carved
			CGrayUIDBase m_uidKiller;	// more2=Who killed this corpse, carved or looted it last. sleep=self.
			CREID_TYPE	m_BaseID;		// morex,morey=The true type of the creature who's corpse this is.
			BYTE		m_facing_dir;	// morez=corpse dir. 0x80 = on face. DIR_TYPE
			// m_amount = the body type.
			// m_uidLink = the creatures ghost.
		} m_itCorpse;

		// IT_LIGHT_LIT
		// IT_LIGHT_OUT
		// IT_WINDOW
		struct
		{
			// CAN_I_LIGHT may be set for others as well..ie.Moon gate conflict
			DWORD   m_junk1;
			DWORD	m_junk2;
			WORD	m_junk3;
			WORD	m_charges;	// morey = how long will the torch last ?
			BYTE	m_pattern;	// morez = light rotation pattern (LIGHT_PATTERN)
		} m_itLight;

		// IT_EQ_TRADE_WINDOW
		struct
		{
			DWORD	m_junk1;
			DWORD	m_junk2;
			DWORD	m_junk3;
			BYTE	m_fCheck;	// morez=Check box for trade window.
		} m_itEqTradeWindow;

		// IT_SPAWN_ITEM
		struct
		{
			RESOURCE_ID_BASE m_ItemID;	// more1=The ITEMID_* or template for items)
			DWORD	m_pile;			// more2=The max # of items to spawn per interval.  If this is 0, spawn up to the total amount
			WORD	m_TimeLoMin;	// morex=Lo time in minutes.
			WORD	m_TimeHiMin;	// morey=
			BYTE	m_DistMax;		// morez=How far from this will it spawn?
		} m_itSpawnItem;



		// IT_RESEARCH_ITEM
		struct
		{
			RESOURCE_ID_BASE m_ItemID;		// more1	= the item researched
			WORD		m_iCompleted;	// more2l	= research percent
			WORD		m_junk1;
			DWORD		m_junk2;		// morex = can we just step on this to activate ?
		} m_itResearchItem;


		// IT_SPAWN_CHAR
		struct
		{
			RESOURCE_ID_BASE m_CharID;	// more1=CREID_*,  or (SPAWNTYPE_*,
			DWORD	m_current;		// more2=The current spawned from here. m_amount = the max count.
			WORD	m_TimeLoMin;	// morex=Lo time in minutes.
			WORD	m_TimeHiMin;	// morey=
			BYTE	m_DistMax;		// morez=How far from this will they wander ?
		} m_itSpawnChar;

		// IT_EXPLOSION
		struct
		{
			DWORD	m_junk1;
			DWORD	m_junk2;
			WORD	m_iDamage;		// morex = damage of the explosion
			WORD	m_wFlags;		// morey = DAMAGE_TYPE = fire,magic,etc
			BYTE	m_iDist;		// morez = distance range of damage.
		} m_itExplode;	// Make this asyncronous.

		// IT_BOOK
		// IT_MESSAGE
		struct
		{
			RESOURCE_ID_BASE m_ResID;	// more1 = preconfigured book id from RES_BOOK or Time date stamp for the book/message creation. (if |0x80000000)
			CServTime   	 m_Time;	// more2= Time date stamp for the book/message creation.
		} m_itBook;

		// IT_EQ_NPC_SCRIPT
		struct
		{
			RESOURCE_ID_BASE m_ResID;	// more1= preconfigured book id from RES_BOOK or 0 = local
			CServTime   	 m_Time;	// more2= Time date stamp for the book/message creation.
			WORD	m_ExecPage;			// morex= what page of the script are we on ?
			WORD	m_ExecOffset;		// morey= offset on the page. 0 = not running (yet).
			BYTE	m_iPriorityLevel;	// morez = How much % they want to do this.
		} m_itNPCScript;

		// IT_DEED
		struct
		{
			ITEMID_TYPE m_Type;		// more1 = deed for what multi, item or template ?
			DWORD		m_dwKeyCode;	// more2 = previous key code. (dry docked ship)
		} m_itDeed;

		// IT_CROPS
		// IT_FOLIAGE - the leaves of a tree normally.
		struct
		{
			int m_Respawn_Sec;		// more1 = plant respawn time in seconds. (for faster growth plants)
			ITEMID_TYPE m_ReapFruitID;	// more2 = What is the fruit of this plant.
			WORD m_ReapStages;		// morex = how many more stages of this to go til ripe.
		} m_itCrop;

		// IT_TREE
		// ? IT_ROCK
		// ? IT_WATER
		// ? IT_GRASS
		struct	// Natural resources. tend to be statics.
		{
			RESOURCE_ID_BASE m_rid_res;	// more1 = base resource type. RES_REGIONRESOURCE
		} m_itResource;

		// IT_FIGURINE
		// IT_EQ_HORSE
		struct
		{
			CREID_TYPE m_ID;	// more1 = What sort of creature will this turn into.
			CGrayUIDBase m_UID;	// more2 = If stored by the stables. (offline creature)
		} m_itFigurine;

		// IT_RUNE
		struct
		{
			int m_Strength;			// more1 = How many uses til a rune will wear out ?
			DWORD m_junk2;
			CPointBase m_pntMark;	// morep = rune marked to a location or a teleport ?
		} m_itRune;

		// IT_TELEPAD
		// IT_MOONGATE
		struct
		{
			int m_fPlayerOnly;		// more1 = The gate is player only. (no npcs, xcept pets)
			int m_fQuiet;			// more2 = The gate/telepad makes no noise.
			CPointBase m_pntMark;	// morep = marked to a location or a teleport ?
		} m_itTelepad;

		// IT_EQ_MEMORY_OBJ
		struct
		{
			// m_amount = memory type mask.
			WORD m_Action;		// more1l = NPC_MEM_ACT_TYPE What sort of action is this memory about ? (1=training, 2=hire, etc)
			WORD m_Skill;		// more1h = SKILL_TYPE = training a skill ?
			CServTime m_timeStart;	// more2 = When did the fight start or action take place ?
			CPointBase m_pt;		// morep = Location the memory occured.
			// m_uidLink = what is this memory linked to. (must be valid)
		} m_itEqMemory;

		// IT_MULTI
		// IT_SHIP
		struct
		{
			CGrayUIDBase m_UIDCreator;	// more1 = who created this house or ship ?
			BYTE m_fSail;		// more2.b1 = ? speed ?
			BYTE m_fAnchored;
			BYTE m_DirMove;		// DIR_TYPE
			BYTE m_DirFace;
			// uidLink = my IT_SHIP_TILLER or IT_SIGN_GUMP,
		} m_itShip;

		// IT_PORTCULIS
		// IT_PORT_LOCKED
		struct
		{
			int m_z1;			// more1 = The down z height.
			int m_z2;			// more2 = The up z height.
		} m_itPortculis;

		// IT_ADVANCE_GATE
		struct
		{
			CREID_TYPE m_Type;		// more1 = What char template to use.
		} m_itAdvanceGate;

		// IT_BEE_HIVE
		struct
		{
			int m_honeycount;		// more1 = How much honey has accumulated here.
		} m_itBeeHive;

		// IT_LOOM
		struct
		{
			ITEMID_TYPE m_ClothID;	// more1 = the cloth type currenctly loaded here.
			int m_ClothQty;			// more2 = IS the loom loaded with cloth ?
		} m_itLoom;

		// IT_ARCHERY_BUTTE
		struct
		{
			ITEMID_TYPE m_AmmoType;	// more1 = arrow or bolt currently stuck in it.
			int m_AmmoCount;		// more2 = how many arrows or bolts ?
		} m_itArcheryButte;

		// IT_CANNON_MUZZLE
		struct
		{
			DWORD m_junk1;
			DWORD m_Load;			// more2 = Is the cannon loaded ? Mask = 1=powder, 2=shot
		} m_itCannon;

		// IT_EQ_MURDER_COUNT
		struct
		{
			DWORD m_Decay_Balance;	// more1 = For the murder flag, how much time is left ?
		} m_itEqMurderCount;

		// IT_ITEM_STONE
		struct
		{
			ITEMID_TYPE m_ItemID;	// more1= generate this item or template.
			int m_iPrice;			// more2= ??? gold to purchase / sellback. (vending machine)
			WORD m_wRegenTime;		// morex=regen time in seconds. 0 = no regen required.
			WORD m_wAmount;			// morey=Total amount to deliver. 0 = infinite, 0xFFFF=none left
		} m_itItemStone;

		// IT_EQ_STUCK
		struct
		{
			// LINK = what are we stuck to ?
		} m_itEqStuck;

		// IT_WEB
		struct
		{
			DWORD m_Hits_Cur;	// more1 = how much damage the web can take.
		} m_itWeb;

		// IT_DREAM_GATE
		struct
		{
			int m_index;	// more1 = how much damage the web can take.
		} m_itDreamGate;

		// IT_TRAP
		// IT_TRAP_ACTIVE
		// IT_TRAP_INACTIVE
		struct
		{
			ITEMID_TYPE m_AnimID;	// more1 = What does a trap do when triggered. 0=just use the next id.
			int	m_Damage;			// more2 = Base damage for a trap.
			WORD m_wAnimSec;		// morex = How long to animate as a dangerous trap.
			WORD m_wResetSec;		// morey = How long to sit idle til reset.
			BYTE m_fPeriodic;		// morez = Does the trap just cycle from active to inactive ?
		} m_itTrap;

		// IT_ANIM_ACTIVE
		struct
		{
			// NOTE: This is slightly dangerous to use as it will overwrite more1 and more2
			ITEMID_TYPE m_PrevID;	// more1 = What to turn back into after the animation.
			IT_TYPE m_PrevType;	// more2 = Any item that will animate.	??? Get rid of this !!
		} m_itAnim;

		// IT_SWITCH
		struct
		{
			ITEMID_TYPE m_SwitchID;	// more1 = the next state of this switch.
			DWORD		m_junk2;
			WORD		m_fStep;	// morex = can we just step on this to activate ?
			WORD		m_wDelay;	// morey = delay this how long before activation.
			// uidLink = the item to use when this item is thrown or used.
		} m_itSwitch;

		// IT_SOUND
		struct
		{
			DWORD	m_Sound;	// more1 = SOUND_TYPE
			int		m_Repeat;	// more2 =
		} m_itSound;

		// IT_STONE_GUILD
		// IT_STONE_TOWN
		// IT_STONE_ROOM
		struct
		{
			STONEALIGN_TYPE m_iAlign;	// more1=Neutral, chaos, order.
			int m_iAccountGold;			// more2=How much gold has been dropped on me?
			// ATTR_OWNED = auto promote to member.
		} m_itStone;

		// IT_LEATHER
		// IT_FEATHER
		// IT_FUR
		// IT_WOOL
		// IT_BLOOD
		// IT_BONE
	 	struct
		{
			int m_junk1;
			CREID_TYPE m_creid;	// more2=the source creature id. (CREID_TYPE)
		} m_itSkin;

	};	// IT_QTY

protected:
	CItem( ITEMID_TYPE id, CItemBase * pItemDef );	// only created via CreateBase()
	bool SetBase( CItemBase * pItemDef );
	virtual int FixWeirdness();

public:
	virtual ~CItem();
	virtual bool OnTick();
	virtual void OnHear( LPCTSTR pszCmd, CChar * pSrc )
	{
		// This should never be called directly. Normal items cannot hear. IT_SHIP and IT_COMM_CRYSTAL
	}

	CItemBase * Item_GetDef() const
	{
		return( STATIC_CAST <CItemBase*>( Base_GetDef()));
	}

	ITEMID_TYPE GetID() const
	{
		const CItemBase * pItemDef = Item_GetDef();
		ASSERT(pItemDef);
		return( pItemDef->GetID());
	}
	WORD GetBaseID() const
	{
		return( GetID());
	}
	bool SetBaseID( ITEMID_TYPE id );
	bool SetID( ITEMID_TYPE id );
	ITEMID_TYPE GetDispID() const
	{
		// This is what the item looks like.
		// May not be the same as the item that defines it's type.
		// DEBUG_CHECK( CItem::IsValidDispID( m_wDispIndex ));
		return((ITEMID_TYPE) m_wDispIndex );
	}
	bool IsSameDispID( ITEMID_TYPE id ) const	// account for flipped types ?
	{
		const CItemBase * pItemDef = Item_GetDef();
		ASSERT(pItemDef);
		return( pItemDef->IsSameDispID( id ));
	}
	bool SetDispID( ITEMID_TYPE id );
	void SetAnim( ITEMID_TYPE id, int iTime );

	int IsWeird() const;
	void SetDisconnected();

	void SetAttr( WORD wAttr )
	{
		m_Attr |= wAttr;
	}
	void ClrAttr( WORD wAttr )
	{
		m_Attr &= ~wAttr;
	}
	bool IsAttr( WORD wAttr ) const	// ATTR_DECAY
	{
		return(( m_Attr & wAttr ) ? true : false );
	}

	int  GetDecayTime() const;
	void SetDecayTime( int iTime = 0 );
	SOUND_TYPE GetDropSound( const CObjBase * pObjOn ) const;
	bool IsTopLevelMultiLocked() const;
	bool IsMovableType() const;
	bool IsMovable() const;
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
	int ConsumeAmount( int iQty = 1, bool fTest = false );

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

	LPCTSTR GetName() const;	// allowed to be default name.
	LPCTSTR GetNameFull( bool fIdentified ) const;

	virtual bool SetName( LPCTSTR pszName );

	virtual int GetWeight() const
	{
		const CItemBase * pItemDef = Item_GetDef();
		ASSERT(pItemDef);
		int iWeight = pItemDef->GetWeight() * GetAmount();
		DEBUG_CHECK( iWeight >= 0 );
		return( iWeight );
	}

	void SetTimeout( int iDelay );

	virtual void OnMoveFrom()	// Moving from current location.
	{
	}
	virtual void OnMoveTo() // Put item on the ground.
	{
	}
	virtual bool MoveTo( CPointMap pt ); // Put item on the ground here.
	bool MoveToDecay( const CPointMap & pt, int iDecayTime )
	{
		SetDecayTime( iDecayTime );
		return MoveTo( pt );
	}
	bool MoveToCheck( const CPointMap & pt, CChar * pCharMover = NULL );
	virtual bool MoveNearObj( const CObjBaseTemplate * pItem, int iSteps = 0, WORD wCan = CAN_C_WALK );

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
			return( const_cast <CItem*>(this) );
		return( pObj->GetTopLevelObj());
	}

	void  Update( const CClient * pClientExclude = NULL );		// send this new item to everyone.
	void  Flip( LPCTSTR pCmd = NULL );
	bool  LoadSetContainer( CGrayUID uid, LAYER_TYPE layer );

	void WriteUOX( CScript & s, int index );

	void r_WriteMore1( CGString & sVal );
	void r_WriteMore2( CGString & sVal );

	virtual void r_Serialize( CGFile & a );	// binary
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & s, CTextConsole * pSrc );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
	virtual bool  r_LoadVal( CScript & s  );
	virtual bool  r_Load( CScript & s ); // Load an item from script
	virtual bool  r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script

private:
	TRIGRET_TYPE OnTrigger( LPCTSTR pszTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs );
public:
	TRIGRET_TYPE OnTrigger( ITRIG_TYPE trigger, CTextConsole * pSrc, CScriptTriggerArgs * pArgs = NULL )
	{
		ASSERT( trigger < ITRIG_QTY );
		return( OnTrigger( MAKEINTRESOURCE(trigger), pSrc, pArgs ));
	}

	int GetBlessCurseLevel() const;
	bool SetBlessCurse( int iLevel, CChar * pCharSrc );

	// Item type specific stuff.
	bool IsType( IT_TYPE type ) const
	{
		return( m_type == type );
	}
	IT_TYPE GetType() const
	{
		return( m_type );
	}
	CItem * SetType( IT_TYPE type );
	bool IsTypeLit() const
	{
		// has m_pattern arg
		switch(m_type)
		{
		case IT_SPELL: // a magic spell effect. (might be equipped)
		case IT_FIRE:
		case IT_LIGHT_LIT:
		case IT_CAMPFIRE:
		case IT_LAVA:
		case IT_WINDOW:
			return( true );
		}
		return( false );
	}
	bool IsTypeBook() const
	{
		switch( m_type )
		{
		case IT_BOOK:
		case IT_MESSAGE:
		case IT_EQ_NPC_SCRIPT:
			return( true );
		}
		return( false );
	}
	bool IsTypeArmor() const
	{
		return( CItemBase::IsTypeArmor(m_type));
	}
	bool IsTypeWeapon() const
	{
		return( CItemBase::IsTypeWeapon(m_type));
	}
	bool IsTypeArmorWeapon() const
	{
		// Armor or weapon.
		if ( IsTypeArmor())
			return( true );
		return( IsTypeWeapon());
	}
	bool IsTypeLocked() const
	{
		switch ( m_type )
		{
		case IT_SHIP_SIDE_LOCKED:
		case IT_CONTAINER_LOCKED:
		case IT_SHIP_HOLD_LOCK:
		case IT_DOOR_LOCKED:
			return( true );
		}
		return(false);
	}
	bool IsTypeLockable() const
	{
		switch( m_type )
		{
		case IT_CONTAINER:
		case IT_DOOR:
		case IT_DOOR_OPEN:
		case IT_SHIP_SIDE:
		case IT_SHIP_PLANK:
		case IT_SHIP_HOLD:
			return( true );
		}
		return( IsTypeLocked() );
	}
	bool IsTypeSpellable() const
	{
		// m_itSpell
		switch( m_type )
		{
		case IT_SCROLL:
		case IT_SPELL:
		case IT_FIRE:
			return( true );
		}
		return( IsTypeArmorWeapon());
	}

	bool IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwArg );

	bool IsValidLockLink( CItem * pItemLock ) const;
	bool IsValidLockUID() const;
	bool IsKeyLockFit( DWORD dwLockUID ) const
	{
		DEBUG_CHECK( IsType( IT_KEY ));
		return( m_itKey.m_lockUID == dwLockUID );
	}

	void ConvertBolttoCloth();
	SPELL_TYPE GetScrollSpell() const;
	bool IsSpellInBook( SPELL_TYPE spell ) const;
	int  AddSpellbookScroll( CItem * pItem );
	int AddSpellbookSpell( SPELL_TYPE spell, bool fUpdate );

	bool IsDoorOpen() const;
	bool Use_Door( bool fJustOpen );
	bool Use_Portculis();
	SOUND_TYPE Use_Music( bool fWell ) const;

	bool SetMagicLock( CChar * pCharSrc, int iSkillLevel );
	void SetSwitchState();
	void SetTrapState( IT_TYPE state, ITEMID_TYPE id, int iTimeSec );
	int Use_Trap();
	bool Use_Light();
	int Use_LockPick( CChar * pCharSrc, bool fTest, bool fFail );
	LPCTSTR Use_SpyGlass( CChar * pUser ) const;
	LPCTSTR Use_Sextant( CPointMap pntCoords ) const;

	bool IsBookWritable() const
	{
		DEBUG_CHECK( IsTypeBook());
		return( m_itBook.m_ResID.GetPrivateUID() == 0 && m_itBook.m_Time.GetTimeRaw() == 0 );
	}
	bool IsBookSystem() const	// stored in RES_BOOK
	{
		DEBUG_CHECK( IsTypeBook());
		return( m_itBook.m_ResID.GetResType() == RES_BOOK );
	}

	bool OnExplosion();
	bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel, CItem * pSourceItem );
	int OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType = DAMAGE_HIT_BLUNT );

	int Armor_GetRepairPercent() const;
	LPCTSTR Armor_GetRepairDesc() const;
	bool Armor_IsRepairable() const;
	int Armor_GetDefense() const;
	int Weapon_GetAttack() const;
	SKILL_TYPE Weapon_GetSkill() const;

	void Spawn_OnTick( bool fExec );
	void Spawn_KillChildren();
	CCharBase * Spawn_SetTrackID();
	void Spawn_GenerateItem( CResourceDef * pDef );
	void Spawn_GenerateChar( CResourceDef * pDef );
	CResourceDef * Spawn_FixDef();
	int Spawn_GetName( TCHAR * pszOut ) const;

	bool IsMemoryTypes( WORD wType ) const
	{
		// MEMORY_FIGHT
		if ( ! IsType( IT_EQ_MEMORY_OBJ ))
			return( false );
		return(( GetHueAlt() & wType ) ? true : false );
	}

	bool Ship_Plank( bool fOpen );

	void Plant_SetTimer();
	bool Plant_OnTick();
	void Plant_CropReset();
	bool Plant_Use( CChar * pChar );

	virtual void DupeCopy( const CItem * pItem );
	CItem * UnStackSplit( int amount, CChar * pCharSrc = NULL );

	static CItem * CreateBase( ITEMID_TYPE id );
	static CItem * CreateHeader( TCHAR * pArg, CObjBase * pCont = NULL, bool fDupeCheck = false, CChar * pSrc = NULL );
	static CItem * CreateScript( ITEMID_TYPE id, CChar * pSrc = NULL );
	static CItem * CreateDupeItem( const CItem * pItem, CChar * pSrc = NULL, bool fSetNew = false );
	static CItem * CreateTemplate( ITEMID_TYPE id, CObjBase* pCont = NULL, CChar * pSrc = NULL );

	static CItem * ReadTemplate( CResourceLock & s, CObjBase * pCont );
};

class CItemVendable : public CItem
{
	// Any item that can be sold and has value.
protected:
	DECLARE_MEM_DYNAMIC;

private:
	static LPCTSTR const sm_szLoadKeys[];
	WORD m_quality;		// 0-100 quality.
	LONG m_price;		// The price of this item if on a vendor. (allow random (but remembered) pluctuations)

public:
	CItemVendable( ITEMID_TYPE id, CItemBase * pItemDef );
	~CItemVendable();

	WORD	GetQuality() const {return m_quality;}
	void	SetQuality(WORD quality = 0)
	{
		DEBUG_CHECK( quality <= 200 );
		m_quality = quality;
	}

	void SetPlayerVendorPrice( LONG dwVal );
	LONG GetBasePrice() const;
	LONG GetVendorPrice( int iConvertFactor );

	bool  IsValidSaleItem( bool fBuyFromVendor ) const;
	bool  IsValidNPCSaleItem() const;

	virtual void DupeCopy( const CItem * pItem );

	void	Restock( bool fSellToPlayers );
	virtual void r_Serialize( CGFile & a );	// binary
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool  r_LoadVal( CScript & s  );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
};

class CContainer : public CGObList	// This class contains a list of items but may or may not be an item itself.
{
private:
	int	m_totalweight;	// weight of all the items it has. (1/WEIGHT_UNITS pound)
protected:
	static LPCTSTR const sm_szLoadKeys[];
	virtual void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
	void ContentAddPrivate( CItem * pItem );

	void r_WriteContent( CScript & s ) const;
	void r_SerializeContent( CGFile & a ) const;	// binary

	bool r_WriteValContainer( LPCTSTR pszKey, CGString &sVal );
	bool r_GetRefContainer( LPCTSTR & pszKey, CScriptObj * & pRef );

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

	bool ContentFindKeyFor( CItem * pLocked ) const;
	// bool IsItemInside( CItem * pItem ) const;

	CItem * ContentFindRandom( void ) const;
	void ContentsDump( const CPointMap & pt, WORD wAttr = 0 );
	void ContentsTransfer( CItemContainer * pCont, bool fNoNewbie );
	void ContentAttrMod( WORD wAttr, bool fSet );

	// For resource usage and gold.
	CItem * ContentFind( RESOURCE_ID_BASE rid, DWORD dwArg = 0, int iDecendLevels = 255 ) const;
	TRIGRET_TYPE OnContTriggerForLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, CScriptLineContext & StartContext, CScriptLineContext & EndContext, RESOURCE_ID_BASE rid, DWORD dwArg = 0, int iDecendLevels = 255 );
	TRIGRET_TYPE OnGenericContTriggerForLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, CScriptLineContext & StartContext, CScriptLineContext & EndContext, int iDecendLevels = 255 );
	int ContentCount( RESOURCE_ID_BASE rid, DWORD dwArg = 0 );
	int ContentCountAll() const;
	int ContentConsume( RESOURCE_ID_BASE rid, int iQty = 1, bool fTest = false, DWORD dwArg = 0 );

	int ResourceConsume( const CResourceQtyArray * pResources, int iReplicationQty, bool fTest = false, DWORD dwArg = 0 );
	int ResourceConsumePart( const CResourceQtyArray * pResources, int iReplicationQty, int iFailPercent, bool fTest = false );

	virtual void OnWeightChange( int iChange );
	virtual void ContentAdd( CItem * pItem ) = 0;
};

class CItemContainer : public CItemVendable, public CContainer
{
	// This item has other items inside it.
	static LPCTSTR const sm_szVerbKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
public:
	// bool m_fTinkerTrapped;	// magic trap is diff.
	void DeletePrepare()
	{
		if ( IsType( IT_EQ_TRADE_WINDOW ))
		{
			Trade_Delete();
		}
		CItem::DeletePrepare();
	}

protected:
public:
	CItemContainer( ITEMID_TYPE id, CItemBase * pItemDef );
	virtual ~CItemContainer()
	{
		DeleteAll();	// get rid of my contents first to protect against weight calc errors.
		DeletePrepare();
	}
public:
	bool IsWeighed() const
	{
		if ( IsType(IT_EQ_BANK_BOX ))
			return false;
		if ( IsType(IT_EQ_VENDOR_BOX))
			return false;
		if ( IsAttr(ATTR_MAGIC))	// magic containers have no weight.
			return false;
		return( true );
	}
	bool IsSearchable() const
	{
		if ( IsType(IT_EQ_BANK_BOX ))
			return false;
		if ( IsType(IT_EQ_VENDOR_BOX))
			return false;
		if ( IsType(IT_CONTAINER_LOCKED))
			return( false );
		return( true );
	}

	bool IsItemInside(const CItem * pItem) const;
	bool CanContainerHold(const CItem * pItem, const CChar * pCharMsg );

	virtual void r_Serialize( CGFile & a );	// binary
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );

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
		return( GetHueAlt());
	}
	void SetRestockTimeSeconds( int iSeconds )
	{
		SetHueAlt(iSeconds);
	}

	virtual void DupeCopy( const CItem * pItem );

	CPointMap GetRandContainerLoc() const;

	void OnOpenEvent( CChar * pCharOpener, const CObjBaseTemplate * pObjTop );
};

class CItemScript : public CItemVendable	// A message for a bboard or book text.
{
	// IT_SCRIPT, IT_EQ_SCRIPT
protected:
	DECLARE_MEM_DYNAMIC;
public:
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
public:
	CItemScript( ITEMID_TYPE id, CItemBase * pItemDef ) : CItemVendable( id, pItemDef )
	{
	}
	virtual ~CItemScript()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}

	virtual void r_Serialize( CGFile & a );	// binary
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );	// some command on this object as a target
	virtual void r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc = NULL );
	virtual bool r_LoadVal( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
	virtual void DupeCopy( const CItem * pItem );
};

class CItemCorpse : public CItemContainer
{
	// IT_CORPSE
	// A corpse is a special type of item.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CItemCorpse( ITEMID_TYPE id, CItemBase * pItemDef ) :
		CItemContainer( id, pItemDef )
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
	// IT_MULTI IT_SHIP
	// A ship or house etc.
protected:
	DECLARE_MEM_DYNAMIC;

private:
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
	CRegionWorld * m_pRegion;		// we own this region.

private:
	void Multi_Delete();
	void MultiUnRealizeRegion();
	bool MultiRealizeRegion();

	CItem * Multi_FindItemType( IT_TYPE type ) const;
	CItem * Multi_FindItemComponent( int iComp ) const;

	bool Multi_IsPartOf( const CItem * pItem ) const;
	int Multi_IsComponentOf( const CItem * pItem ) const;

	const CItemBaseMulti * Multi_GetDef() const
	{
		return( STATIC_CAST <const CItemBaseMulti *>( Base_GetDef()));
	}
	bool Multi_DeedConvert( CChar * pChar );
	bool Multi_CreateComponent( ITEMID_TYPE id, int dx, int dy, int dz, DWORD dwKeyCode );
	int Multi_GetMaxDist() const;

	CItem * Multi_GetSign();	// or Tiller
	int Ship_GetFaceOffset() const
	{
		return( GetID() & 3 );
	}
	int  Ship_ListObjs( CObjBase ** ppObjList );
	bool Ship_CanMoveTo( const CPointMap & pt ) const;
	bool Ship_SetMoveDir( DIR_TYPE dir );
	bool Ship_MoveDelta( CPointBase pdelta );
	bool Ship_Face( DIR_TYPE dir );
	bool Ship_Move( DIR_TYPE dir );
	bool Ship_OnMoveTick( void );

public:
	CItemMulti( ITEMID_TYPE id, CItemBase * pItemDef );
	virtual ~CItemMulti();

public:
	virtual bool OnTick();
	virtual bool MoveTo( CPointMap pt ); // Put item on the ground here.
	virtual void OnMoveFrom();	// Moving from current location.
	void OnHearRegion( LPCTSTR pszCmd, CChar * pSrc );

	void Multi_Create( CChar * pChar, DWORD dwKeyCode );
	static const CItemBaseMulti * Multi_GetDef( ITEMID_TYPE id );

	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_Serialize( CGFile & a );	// binary
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool  r_LoadVal( CScript & s  );
	virtual void DupeCopy( const CItem * pItem );

	void Ship_Stop( void );
};

class CItemMemory : public CItem
{
	// IT_EQ_MEMORY
	// Allow extra tags for the memory
protected:
	DECLARE_MEM_DYNAMIC;
public:
	CItemMemory( ITEMID_TYPE id, CItemBase * pItemDef ) :
		CItem( ITEMID_MEMORY, pItemDef )
	{
	}
	virtual ~CItemMemory()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}
	WORD SetMemoryTypes( WORD wType )	// For memory type objects.
	{
		DEBUG_CHECK( IsType( IT_EQ_MEMORY_OBJ ));
		SetHueAlt( wType );
		return( wType );
	}
	WORD GetMemoryTypes() const
	{
		DEBUG_CHECK( IsType( IT_EQ_MEMORY_OBJ ));
		return( GetHueAlt());	// MEMORY_FIGHT
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
	// IT_MAP
protected:
	DECLARE_MEM_DYNAMIC;
	static LPCTSTR const sm_szLoadKeys[];
public:
	enum
    {
        MAX_PINS = 128,
    };

	bool m_fPlotMode;	// should really be per-client based but oh well.
	CGTypedArray<CMapPinRec,CMapPinRec&> m_Pins;

public:
	CItemMap( ITEMID_TYPE id, CItemBase * pItemDef ) : CItemVendable( id, pItemDef )
	{
		m_fPlotMode = false;
	}
	virtual ~CItemMap()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}

	virtual void r_Serialize( CGFile & a );	// binary
	virtual void r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc = NULL );
	virtual bool r_LoadVal( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void DupeCopy( const CItem * pItem );
};

class CItemCommCrystal : public CItemVendable
{
	// STATF_COMM_CRYSTAL and IT_COMM_CRYSTAL
	// What speech blocks does it like ?
protected:
	DECLARE_MEM_DYNAMIC;
	static LPCTSTR const sm_szLoadKeys[];
	CResourceRefArray m_Speech;	// Speech fragment list (other stuff we know)
public:
	CItemCommCrystal( ITEMID_TYPE id, CItemBase * pItemDef ) : CItemVendable( id, pItemDef )
	{
	}
	virtual ~CItemCommCrystal()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}

	virtual void OnMoveFrom();
	virtual bool MoveTo( CPointMap pt );

	virtual void OnHear( LPCTSTR pszCmd, CChar * pSrc );
	virtual void r_Serialize( CGFile & a );	// binary
	virtual void  r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc );
	virtual bool  r_LoadVal( CScript & s  );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void DupeCopy( const CItem * pItem );
};

enum STONEPRIV_TYPE // Priv level for this char
{
	STONEPRIV_CANDIDATE = 0,
	STONEPRIV_MEMBER,
	STONEPRIV_MASTER,
	STONEPRIV_BANISHED,	// This char is banished from the town/guild.
	STONEPRIV_ACCEPTED,	// The candidate has been accepted. But they have not dclicked on the stone yet.
	STONEPRIV_RESIGNED,	// We have resigned but not timed out yet.
	STONEPRIV_ENEMY = 100,	// this is an enemy town/guild.
};

class CStoneMember : public CGObListRec	// Members for various stones, and links to stones at war with
{
	// NOTE: Chars are linked to the CItemStone via a memory object.
	friend class CItemStone;
protected:
	DECLARE_MEM_DYNAMIC;
private:
	STONEPRIV_TYPE m_iPriv;	// What is my status level in the guild ?
	CGrayUID m_uidLinkTo;			// My char uid or enemy stone UID

	// Only apply to members.
	CGrayUID m_uidLoyalTo;	// Who am i loyal to? invalid value = myself.
	CGString m_sTitle;		// What is my title in the guild?

	union	// Depends on m_iPriv
	{
		struct	// Unknown type.
		{
			int m_Val1;
			int m_Val2;
			int m_Val3;
		} m_UnDef;

		struct // STONEPRIV_ENEMY
		{
			int m_fTheyDeclared;
			int m_fWeDeclared;
		} m_Enemy;

		struct	// a char member (NOT STONEPRIV_ENEMY)
		{
			int m_fAbbrev;			// Do they have their guild abbrev on or not ?
			int m_iVoteTally;		// Temporary space to calculate votes for me.
			int m_iAccountGold;		// how much i still owe to the guild or have surplus (Normally negative).
		} m_Member;
	};

public:
	CStoneMember( CItemStone * pStone, CGrayUID uid, STONEPRIV_TYPE iType, LPCTSTR pTitle = "", CGrayUID loyaluidLink = 0, bool fArg1 = false, bool fArg2 = false, int nAccountGold = 0);
	virtual ~CStoneMember();
	CStoneMember* GetNext() const
	{
		return( STATIC_CAST <CStoneMember *>( CGObListRec::GetNext()));
	}
	CItemStone * GetParentStone();

	CGrayUID GetLinkUID() const { return m_uidLinkTo; }

	STONEPRIV_TYPE GetPriv() const { return m_iPriv; }
	void SetPriv(STONEPRIV_TYPE iPriv) { m_iPriv = iPriv; }
	bool IsPrivMaster() const { return m_iPriv == STONEPRIV_MASTER; }
	bool IsPrivMember() const { return( m_iPriv == STONEPRIV_MASTER || m_iPriv == STONEPRIV_MEMBER ); }
	LPCTSTR GetPrivName() const;

	// If the member is really a war flag (STONEPRIV_ENEMY)
	void SetWeDeclared(bool f)
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		m_Enemy.m_fWeDeclared = f;
	}
	bool GetWeDeclared() const
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		return ( m_Enemy.m_fWeDeclared ) ? true : false;
	}
	void SetTheyDeclared(bool f)
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		m_Enemy.m_fTheyDeclared = f;
	}
	bool GetTheyDeclared() const
	{
		DEBUG_CHECK( m_iPriv == STONEPRIV_ENEMY );
		return ( m_Enemy.m_fTheyDeclared ) ? true : false;
	}

	// Member
	bool IsAbbrevOn() const { return ( m_Member.m_fAbbrev ) ? true : false; }
	void ToggleAbbrev() { m_Member.m_fAbbrev = !m_Member.m_fAbbrev; }
	void SetAbbrev(bool mode) { m_Member.m_fAbbrev = mode; }

	LPCTSTR GetTitle() const
	{
		return( m_sTitle );
	}
	void SetTitle( LPCTSTR pTitle )
	{
		m_sTitle = pTitle;
	}

	CGrayUID GetLoyalToUID() const
	{
		return( m_uidLoyalTo );
	}
	bool SetLoyalTo( const CChar * pChar);

	int GetAccountGold() const
	{
		return( m_Member.m_iAccountGold );
	}
	void SetAccountGold( int iGold )
	{
		m_Member.m_iAccountGold = iGold;
	}
};

enum STONEDISP_TYPE	// Hard coded Menus
{
	STONEDISP_NONE = 0,
	STONEDISP_ROSTER,
	STONEDISP_CANDIDATES,
	STONEDISP_FEALTY,
	STONEDISP_ACCEPTCANDIDATE,
	STONEDISP_REFUSECANDIDATE,
	STONEDISP_DISMISSMEMBER,
	STONEDISP_VIEWCHARTER,
	STONEDISP_SETCHARTER,
	STONEDISP_VIEWENEMYS,
	STONEDISP_VIEWTHREATS,
	STONEDISP_DECLAREWAR,
	STONEDISP_DECLAREPEACE,
	STONEDISP_GRANTTITLE,
	STONEDISP_VIEWBANISHED,
	STONEDISP_BANISHMEMBER,
};

class CItemStone : public CItem, public CGObList
{
	// IT_STONE_GUILD
	// IT_STONE_TOWN
	// ATTR_OWNED = auto promote to member.

	friend class CStoneMember;
	static LPCTSTR const sm_szVerbKeys[];
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szLoadKeysM[];
	static LPCTSTR const sm_szLoadKeysG[];

protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sCharter[6];
	CGString m_sWebPageURL;
	CGString m_sAbbrev;
	int		 m_iDailyDues;	// Real world dues in gp.
	int      m_iGoldReserve;

private:

	void SetTownName();
	bool SetName( LPCTSTR pszName );
	virtual bool MoveTo( CPointMap pt );

	MEMORY_TYPE GetMemoryType() const
	{
		switch ( GetType() )
		{
		case IT_STONE_TOWN: return( MEMORY_TOWN );
		case IT_STONE_GUILD: return( MEMORY_GUILD );
		}
		// Houses have no memories.
		return( MEMORY_NONE );
	}
	bool IsInMenu( STONEDISP_TYPE iStoneMenu, const CItemStone * pOtherStone ) const;
	bool IsInMenu( STONEDISP_TYPE iStoneMenu, const CStoneMember * pMember ) const;

	LPCTSTR GetCharter(int iLine) const { ASSERT(iLine<COUNTOF(m_sCharter)); return(  m_sCharter[iLine] ); }
	void SetCharter( int iLine, LPCTSTR pCharter ) { ASSERT(iLine<COUNTOF(m_sCharter)); m_sCharter[iLine] = pCharter; }
	LPCTSTR GetWebPageURL() const { return( m_sWebPageURL ); }
	void SetWebPage( LPCTSTR pWebPage ) { m_sWebPageURL = pWebPage; }

	// Client interaction.
	int  addStoneListSetup( STONEDISP_TYPE iStoneMenu, CGString * psText, int iTexts );
	void addStoneList( CClient * pClient, STONEDISP_TYPE iStoneMenu );
	void addStoneSetViewCharter( CClient * pClient, STONEDISP_TYPE iStoneMenu );
	void addStoneDialog( CClient * pClient, STONEDISP_TYPE menuid );

	void SetupMenu( CClient * pClient, bool fMasterFunc=false );

	void ElectMaster();
public:
	CStoneMember * AddRecruit( const CChar * pChar, STONEPRIV_TYPE iPriv );

	// War
private:
	void TheyDeclarePeace( CItemStone* pEnemyStone, bool fForcePeace );
	bool WeDeclareWar(CItemStone * pEnemyStone);
	void WeDeclarePeace(CGrayUID uidEnemy, bool fForcePeace = false);
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
	CItemStone( ITEMID_TYPE id, CItemBase * pItemDef );
	virtual ~CItemStone();

	virtual void r_Serialize( CGFile & a );	// binary
	virtual void r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & s, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	LPCTSTR GetTypeName() const;
	static bool IsUniqueName( LPCTSTR pName );
	CChar * GetMaster() const;
	bool NoMembers() const;
	CStoneMember * GetMasterMember() const;
	CStoneMember * GetMember( const CObjBase * pObj) const;
	bool IsPrivMember( const CChar * pChar ) const;

	// Simple accessors.
	STONEALIGN_TYPE GetAlignType() const { return m_itStone.m_iAlign; }
	void SetAlignType(STONEALIGN_TYPE iAlign) { m_itStone.m_iAlign = iAlign; }
	LPCTSTR GetAlignName() const;
	LPCTSTR GetAbbrev() const { return( m_sAbbrev ); }
	void SetAbbrev( LPCTSTR pAbbrev ) { m_sAbbrev = pAbbrev; }

	bool OnPromptResp( CClient * pClient, CLIMODE_TYPE TargMode, LPCTSTR pszText, CGString & sMsg );
	bool OnDialogButton( CClient * pClient, STONEDISP_TYPE type, CDialogResponseArgs & resp );
	void Use_Item( CClient * pClient );
};

enum CIC_TYPE
{
	CIC_AUTHOR,
	CIC_BODY,
	CIC_PAGES,
	CIC_TITLE,
	CIC_QTY,
};

class CItemMessage : public CItemVendable	// A message for a bboard or book text.
{
	// IT_BOOK, IT_MESSAGE, IT_EQ_NPC_SCRIPT = can be written into.
	// the name is the title for the message. (ITEMID_BBOARD_MSG)
protected:
	DECLARE_MEM_DYNAMIC;
	static LPCTSTR const sm_szVerbKeys[];
private:
	CGObArray<CGString*> m_sBodyLines;	// The main body of the text for bboard message or book.
public:
	CGString m_sAuthor;					// Should just have author name !
	static LPCTSTR const sm_szLoadKeys[CIC_QTY+1];
public:
	CItemMessage( ITEMID_TYPE id, CItemBase * pItemDef ) : CItemVendable( id, pItemDef )
	{
	}
	virtual ~CItemMessage()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
		UnLoadSystemPages();
	}

	WORD GetScriptTimeout() const
	{
		return( GetHueAlt());
	}
	void SetScriptTimeout( WORD wTimeInTenths )
	{
		SetHueAlt( wTimeInTenths );
	}

	virtual void r_Serialize( CGFile & a );	// binary
	virtual void r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute command from script
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	int GetPageCount() const
	{
		return( m_sBodyLines.GetCount());
	}
	LPCTSTR GetPageText( int iPage ) const
	{
		if ( iPage >= GetPageCount())
			return( NULL );
		if ( m_sBodyLines[iPage] == NULL )
			return( NULL );
		return( * m_sBodyLines[iPage] );
	}
	void SetPageText( int iPage, LPCTSTR pszText )
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
	void AddPageText( LPCTSTR pszText )
	{
		m_sBodyLines.Add( new CGString( pszText ));
	}

	virtual void DupeCopy( const CItem * pItem );
	bool LoadSystemPages();
	void UnLoadSystemPages()
	{
		m_sAuthor.Empty();
		m_sBodyLines.RemoveAll();
	}
};

class CItemServerGate : public CItem
{
	// IT_DREAM_GATE
protected:
	DECLARE_MEM_DYNAMIC;
	CGString m_sServerName;	// Link to the other server by name.
public:
	static LPCTSTR const sm_szLoadKeys[];
public:
	CItemServerGate( ITEMID_TYPE id, CItemBase * pItemDef ) :
		CItem( ITEMID_MEMORY, pItemDef )
	{
	}
	virtual ~CItemServerGate()
	{
		DeletePrepare();	// Must remove early because virtuals will fail in child destructor.
	}

	CServerRef GetServerRef() const;
	bool SetServerLink( LPCTSTR pszName );
	LPCTSTR GetServerLink() const
	{
		return( m_sServerName );	// MEMORY_FIGHT
	}

	virtual int FixWeirdness();
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual void r_Serialize( CGFile & a );	// binary
	virtual void r_Write( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
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
	NPCBRAIN_THIEF,		// 9 = should try to steal ?
	NPCBRAIN_MONSTER,	// 10 = not tamable. normally evil.
	NPCBRAIN_BESERK,	// 11 = attack closest (blades, vortex)
	NPCBRAIN_UNDEAD,	// 12 = disapears in the light.
	NPCBRAIN_DRAGON,	// 13 = we can breath fire. may be tamable ? hirable ?
	NPCBRAIN_VENDOR_OFFDUTY,	// 14 = "Sorry i'm not working right now. come back when my shop is open.
	NPCBRAIN_CRIER,		// 15 = will speak periodically.
	NPCBRAIN_CONJURED,	// 16 = elemental or other conjured creature.
	NPCBRAIN_QTY,
};

struct CCharNPC : public CMemDynamic
{
	// This is basically the unique "brains" for any character.
protected:
	DECLARE_MEM_DYNAMIC;
public:
	// Stuff that is specific to an NPC character instance (not an NPC type see CCharBase for that).
	// Any NPC AI stuff will go here.
	static LPCTSTR const sm_szVerbKeys[];

	NPCBRAIN_TYPE m_Brain;	// For NPCs: Number of the assigned basic AI block
	WORD	m_Home_Dist_Wander;	// Distance to allow to "wander".
	BYTE    m_Act_Motivation;		// 0-100 (100=very greatly) how bad do i want to do the current action.
	CPointBase m_Act_p_Prev;	// Location before we started this action (so we can get back if necessary)

	// We respond to what we here with this.
	CResourceRefArray m_Speech;	// Speech fragment list (other stuff we know)
	HUE_TYPE m_SpeechHue;

	CResourceQty m_Need;	// What items might i need/Desire ? (coded as resource scripts) ex "10 gold,20 logs" etc.

	static LPCTSTR const sm_szLoadKeys[];

public:
	void r_SerializeChar( CChar * pChar, CGFile & a );	// binary
	void r_WriteChar( CChar * pChar, CScript & s );
	bool r_WriteVal( CChar * pChar, LPCTSTR pszKey, CGString & s );
	bool r_LoadVal( CChar * pChar, CScript & s );
	void r_DumpLoadKeys( CTextConsole * pSrc );
	void r_DumpVerbKeys( CTextConsole * pSrc );

	bool IsVendor() const
	{
		return( m_Brain == NPCBRAIN_HEALER || m_Brain == NPCBRAIN_STABLE || m_Brain == NPCBRAIN_VENDOR );
	}

	CCharNPC( CChar * pChar, NPCBRAIN_TYPE NPCBrain );
	~CCharNPC();
};

#define IS_SKILL_BASE(sk) ((sk)> SKILL_NONE && (sk)< MAX_SKILL )

struct CCharPlayer : public CMemDynamic
{
	// Stuff that is specific to a player character.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	BYTE m_SkillLock[SKILL_QTY];	// SKILLLOCK_TYPE List of skill lock states for this player character
	CResourceRef m_SkillClass;	// RES_SKILLCLASS CSkillClassDef What skill class group have we selected.
public:
	CAccount * m_pAccount;	// The account index. (for idle players mostly)
	static LPCTSTR const sm_szVerbKeys[];

	CServTime m_timeLastUsed;	// Time the player char was last used.

	CGString m_sProfile;	// limited to SCRIPT_MAX_LINE_LEN-16

	WORD m_wMurders;		// Murder count.
	WORD m_wDeaths;		// How many times have i died ?

	static LPCTSTR const sm_szLoadKeys[];

public:
	SKILL_TYPE Skill_GetLockType( LPCTSTR pszKey ) const;

	SKILLLOCK_TYPE Skill_GetLock( SKILL_TYPE skill ) const
	{
		ASSERT( skill < COUNTOF(m_SkillLock));
		return((SKILLLOCK_TYPE) m_SkillLock[skill] );
	}
	void Skill_SetLock( SKILL_TYPE skill, SKILLLOCK_TYPE state )
	{
		ASSERT( skill < COUNTOF(m_SkillLock));
		m_SkillLock[skill] = state;
	}

	void r_SerializeChar( CChar * pChar, CGFile & a );	// binary
	void r_WriteChar( CChar * pChar, CScript & s );
	bool r_WriteVal( CChar * pChar, LPCTSTR pszKey, CGString & s );
	bool r_LoadVal( CChar * pChar, CScript & s );
	void r_DumpLoadKeys( CTextConsole * pSrc );
	void r_DumpVerbKeys( CTextConsole * pSrc );

	bool SetSkillClass( CChar * pChar, RESOURCE_ID rid );
	CSkillClassDef * GetSkillClass() const;

	CAccountRef GetAccount() const
	{
		ASSERT( m_pAccount );
		return( m_pAccount );
	}

	CCharPlayer( CChar * pChar, CAccount * pAccount );
	~CCharPlayer();
};

enum WAR_SWING_TYPE	// m_Act_War_Swing_State
{
	WAR_SWING_INVALID = -1,
	WAR_SWING_EQUIPPING = 0,	// we are recoiling our weapon.
	WAR_SWING_READY,			// we can swing at any time.
	WAR_SWING_SWINGING,			// we are swinging our weapon.
};

enum CTRIG_TYPE
{
	CTRIG_AAAUNUSED		= 0,
	CTRIG_Attack,			// I am attacking someone (SRC)
	CTRIG_CallGuards,
	CTRIG_Click,			// I got clicked on by someone.
	CTRIG_Create,			// Newly created (not in the world yet)
	CTRIG_DClick,			// Someone has dclicked on me.
	CTRIG_Death,			//+I just got killed.
	CTRIG_DeathCorpse,		// Corpse
	CTRIG_Destroy,			// Permanently gone.
	CTRIG_EnvironChange,	// my environment changed somehow (light,weather,season,region)
	CTRIG_FearOfDeath,		// I'm not healthy.
	CTRIG_FightSwing,		// randomly choose to speak while fighting.
	CTRIG_GetHit,			// I just got hit.
	CTRIG_Hit,				// I just hit someone. (TARG)
	CTRIG_HitMiss,			// I just missed.
	CTRIG_HitTry,			// I am trying to hit someone. starting swing.,
	CTRIG_Hunger,			//+Ready to update the food level

	// ITRIG_QTY
	CTRIG_itemClick,		// I clicked on an item
	CTRIG_itemCreate_UNUSED,	
	CTRIG_itemDAMAGE,		// I have damaged item in some way
	CTRIG_itemDCLICK,		// I have dclicked item
	CTRIG_itemDestroy_UNUSED,
	CTRIG_itemDROPON_CHAR,		// I have been dropped on this char
	CTRIG_itemDROPON_GROUND,	// I dropped an item on the ground
	CTRIG_itemDROPON_ITEM,		// I have been dropped on this item
	CTRIG_itemDROPON_SELF,		// I have been dropped on this item
	CTRIG_itemEQUIP,		// I have equipped an item
	CTRIG_itemEQUIPTEST,
	CTRIG_itemPICKUP_GROUND,
	CTRIG_itemPICKUP_PACK,	// picked up from inside some container.
	CTRIG_itemPICKUP_SELF,	// picked up from this (ACT) container.
	CTRIG_itemSPELL,		// cast some spell on the item.
	CTRIG_itemSTACKON,		// stacked item on another item
	CTRIG_itemSTEP,			// stepped on an item
	CTRIG_itemTARGON_CANCEL,
	CTRIG_itemTARGON_CHAR,
	CTRIG_itemTARGON_GROUND,
	CTRIG_itemTARGON_ITEM,	// I am being combined with an item
	CTRIG_itemTIMER_UNUSED,
	CTRIG_itemToolTip,		// Did tool tips on an item
	CTRIG_itemUNEQUIP,		// i have unequipped (or try to unequip) an item

	CTRIG_LogIn,			// Client logs in
	CTRIG_LogOut,			// Client logs out (21)
	CTRIG_MurderDecay,		// I have decayed one of my kills
	CTRIG_MurderMark,		// I am gonna to be marked as a murder

	CTRIG_NPCAcceptItem,		// (NPC only) i've been given an item i like (according to DESIRES)
	CTRIG_NPCActFight,
	CTRIG_NPCActFollow,		// (NPC only) decided to follow someone
	CTRIG_NPCHearGreeting,		// (NPC only) i have been spoken to for the first time. (no memory of previous hearing)
	CTRIG_NPCHearNeed,			// (NPC only) i heard someone mention something i need. (11)
	CTRIG_NPCHearUnknown,		//+(NPC only) I heard something i don't understand.
	CTRIG_NPCLookAtChar,		//
	CTRIG_NPCLookAtItem,		//
	CTRIG_NPCLostTeleport,		//+(NPC only) ready to teleport back to spawn
	CTRIG_NPCRefuseItem,		// (NPC only) i've been given an item i don't want.
	CTRIG_NPCRestock,			// (NPC only) 
	CTRIG_NPCSeeNewPlayer,		//+(NPC only) i see u for the first time. (in 20 minutes) (check memory time)
	CTRIG_NPCSeeWantItem,		// (NPC only) i see something good.

	// party related triggers, only work for players only (or should only)
	CTRIG_PartyCreate,			//+(PLAYER only) trying to create party (or is it being created right now?)
	CTRIG_PartyInvite,			//+(PLAYER only) i was invited to a party? do i accept or not?
	CTRIG_PartyJoin,			//+(PLAYER only) i decided to join a party
	CTRIG_PartyLeave,			//+(PLAYER only) i am leaving the party (would return 1 work here?)

	CTRIG_PersonalSpace,	//+i just got stepped on.
	CTRIG_Profile,			// someone hit the profile button for me.
	CTRIG_ReceiveItem,		// I was just handed an item (Not yet checked if i want it)

	CTRIG_SeeCrime,			// I am seeing a crime

	// SKTRIG_QTY
	CTRIG_SkillAbort,		// SKTRIG_ABORT
	CTRIG_SkillFail,		// SKTRIG_FAIL
	CTRIG_SkillGain,		// SKTRIG_GAIN
	CTRIG_SkillMakeItem,
	CTRIG_SkillSelect,
	CTRIG_SkillStart,
	CTRIG_SkillStroke,
	CTRIG_SkillSuccess,

	CTRIG_SpellBook,
	CTRIG_SpellCast,		//+Char is casting a spell.
	CTRIG_SpellEffect,		//+A spell just hit me.
	CTRIG_Step,				// I took a step.
	CTRIG_StepStealth,		//+Made a step while being in stealth mode
	CTRIG_ToolTip,		// someone did tool tips on me.

	CTRIG_UserChatButton,
	CTRIG_UserExtCmd,
	CTRIG_UserExWalkLimit,
	CTRIG_UserMailBag,
	CTRIG_UserOverrideRecv,
	CTRIG_UserOverrideSend,
	CTRIG_UserSkills,
	CTRIG_UserStats,
	CTRIG_UserVirtue,
	CTRIG_UserWarmode,

	CTRIG_QTY,				// 55
};

class CPartyDef;

class CChar : public CObjBase, public CContainer, public CTextConsole
{
	// RES_WORLDCHAR
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
#define STATF_NightSight	0x00000100	// All is light to you
#define STATF_Reflection	0x00000200	// Magic reflect on.
#define STATF_Polymorph		0x00000400	// We have polymorphed to another form.
#define STATF_Incognito		0x00000800	// Dont show skill titles
#define STATF_SpiritSpeak	0x00001000	// I can hear ghosts clearly.
#define STATF_Insubstantial	0x00002000	// Ghost has not manifest. or GM hidden
#define STATF_EmoteAction	0x00004000	// The creature will emote its actions to it's owners.
#define STATF_COMM_CRYSTAL	0x00008000	// I have a IT_COMM_CRYSTAL or listening item on me.
#define STATF_HasShield		0x00010000	// Using a shield
#define STATF_Script_Play	0x00020000	// Playing a Script. (book script)
#define STATF_Stone			0x00040000	// turned to stone.
#define STATF_Script_Rec	0x00080000	// Recording a script.
#define STATF_Fly			0x00100000	// Flying or running ? (anim)
#define STATF_RespawnNPC	0x00200000	// This NPC is a plot NPC that must respawn later.
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
	short m_Skill[SKILL_QTY];	// List of skills ( skill * 10 ) (might go temporariliy negative!)

	// This is a character that can either be NPC or PC.
	// Player vs NPC Stuff
	CClient * m_pClient;	// is the char a logged in m_pPlayer ?

public:
	CCharPlayer * m_pPlayer;	// May even be an off-line player !
	CCharNPC * m_pNPC;			// we can be both a player and an NPC if "controlled" ?
	CPartyDef * m_pParty;		// What party am i in ?
	CRegionWorld * m_pArea; // What region are we in now. (for guarded message)

	static LPCTSTR const sm_szRefKeys[];
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
	static LPCTSTR const sm_szTrigName[CTRIG_QTY+1];
	static const LAYER_TYPE sm_VendorLayers[4];

	// Combat stuff. cached data. (not saved)
	CGrayUID m_uidWeapon;		// current Wielded weapon.	(could just get rid of this ?)
	WORD m_defense;			// calculated armor worn (NOT intrinsic armor)

	signed int m_ModMaxWeight;

	//DIR_TYPE m_dirClimb;	// we are standing on a CAN_I_CLIMB or UFLAG2_CLIMBABLE, DIR_QTY = not on climbable
	bool m_fClimbUpdated;	// FixClimbHeight() called?
	signed char m_zClimbHeight;	// The height at the end of the climbable.

	// Saved stuff.
	DIR_TYPE m_dirFace;	// facing this dir.
	CGString m_sTitle;		// Special title such as "the guard" (replaces the normal skill title).
	CPointMap m_ptHome;			// What is our "home" region. (towns and bounding of NPC's)
	FONT_TYPE m_fonttype;	// Speech font to use // can client set this ?

	// In order to revert to original Hue and body.
	CREID_TYPE m_prev_id;		// Backup of body type for ghosts and poly
	HUE_TYPE m_prev_Hue;	// Backup of skin color. in case of polymorph etc.

	// Client's local light
	BYTE m_LocalLight;

	// When events happen to the char. check here for reaction scripts.

	// Skills, Stats and health
	struct
	{
		short	m_base;
		short	m_mod;			// signed for modifier
		short	m_val;			// signed for karma
		short	m_max;			// max
		unsigned short m_regen;	// Tick time since last regen.
	} m_Stat[STAT_QTY];


	CServTime m_timeLastRegen;	// When did i get my last regen tick ?
	CServTime m_timeCreate;		// When was i created ?

	CServTime m_timeLastHitsUpdate;
	bool m_fHitsUpdate;

	// Some character action in progress.
	SKILL_TYPE	m_Act_SkillCurrent;		// Currently using a skill. Could be combat skill.
	CGrayUID		m_Act_Targ;			// Current combat/action target
	CGrayUID		m_Act_TargPrv;		// Targeted bottle for alchemy or previous beg target.
	int			m_Act_Difficulty;	// -1 = fail skill. (0-100) for skill advance calc.
	CPointBase  m_Act_p;			// Moving to this location. or location of forge we are working on.

	union	// arg specific to the action type.(m_Act_SkillCurrent)
	{
		struct
		{
			DWORD m_Arg1;	// "ACTARG1"
			DWORD m_Arg2;	// "ACTARG2"
			DWORD m_Arg3;	// "ACTARG3"
		} m_atUnk;

		// SKILL_MAGERY
		struct
		{
			SPELL_TYPE	m_Spell;		// ACTARG1=Currently casting spell.
			CREID_TYPE	m_SummonID;		// ACTARG2=A sub arg of the skill. (summoned type ?)
			WORD	m_fSummonPet;		// ACTARG3=
		} m_atMagery;

		// SKILL_MUSICIANSHIP
		// SKILL_ENTICEMENT
		// SKILL_PROVOCATION
		// SKILL_PEACEMAKING
		struct
		{
			int m_iMusicDifficulty;		// ACTARG1=Base music diff, (whole thing won't work if this fails)
		} m_atMusician;

		// SKILL_ALCHEMY
		// SKILL_BLACKSMITHING
		// SKILL_BOWCRAFT
		// SKILL_CARPENTRY
		// SKILL_CARTOGRAPHY:
		// SKILL_INSCRIPTION
		// SKILL_TAILORING:
		// SKILL_TINKERING,

		struct	// creation type skill.
		{
			ITEMID_TYPE m_ItemID;		// ACTARG1=Making this item.
			WORD m_Stroke_Count;		// ACTARG2=For smithing, tinkering, etc. all requiring multi strokes.
			WORD m_Amount;				// How many of this item are we making?
		} m_atCreate;

		// SKILL_LUMBERJACKING,
		// SKILL_MINING
		// SKILL_FISHING
		struct
		{
			DWORD		m_junk1;
			DWORD		m_Stroke_Count;		// all requiring multi strokes.
			DWORD		m_ridType;			// type of item we're harvesting
		} m_atResource;

		// SKILL_TAMING
		// SKILL_MEDITATION
		struct
		{
			DWORD m_junk1;
			WORD m_Stroke_Count;		// all requiring multi strokes.
		} m_atTaming;

		// SKILL_SWORDSMANSHIP
		// SKILL_MACEFIGHTING,
		// SKILL_FENCING,
		// SKILL_WRESTLING
		struct
		{
			WAR_SWING_TYPE	m_War_Swing_State;	// We are in the war mode swing.
			// m_Act_Targ = who are we currently attacking?
			WORD			m_fMoved;
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
			CGrayUIDBase m_FigurineUID;	// This creature is being ridden by this object link. IT_FIGURINE IT_EQ_HORSE
		} m_atRidden;

		// NPCACT_TALK
		// NPCACT_TALK_FOLLOW
		struct
		{
			int	 m_HearUnknown;	// Speaking NPC has no idea what u're saying.
			int  m_WaitCount;	// How long have i been waiting (xN sec)
			// m_Act_Targ = who am i talking to ?
		} m_atTalk;

		// NPCACT_FLEE
		// m_Act_Targ = who am i fleeing from ?
		struct
		{
			int	 m_iStepsMax;	// how long should it take to get there.
			int	 m_iStepsCurrent;	// how long has it taken ?
		} m_atFlee;

		// NPCACT_LOOTING
		// m_Act_Targ = what am i looting ? (Corpse)
		// m_Act_TargPrv = targets parent
		// m_Act_p = location of the target.
		struct
		{
			int	 m_iDistEstimate;	// how long should it take to get there.
			int	 m_iDistCurrent;	// how long has it taken ?
		} m_atLooting;

		// NPCACT_TRAINING
		// m_Act_Targ = what am i training on
		// m_Act_TargPrv = weapon
		//
	};

public:
	CChar( CREID_TYPE id );
	virtual ~CChar(); // Delete character

public:
	// Status and attributes ------------------------------------
	int IsWeird() const;
	bool IsStatFlag( DWORD dwStatFlag ) const
	{
		return(( m_StatFlag & dwStatFlag) ? true : false );
	}
	void StatFlag_Set( DWORD dwStatFlag )
	{
		m_StatFlag |= dwStatFlag;
	}
	void StatFlag_Clear( DWORD dwStatFlag )
	{
		m_StatFlag &= ~dwStatFlag;
	}
	void StatFlag_Mod( DWORD dwStatFlag, bool fMod )
	{
		if ( fMod )
			m_StatFlag |= dwStatFlag;
		else
			m_StatFlag &= ~dwStatFlag;
	}
	bool IsPriv( WORD flag ) const
	{	// PRIV_GM flags
		if ( m_pPlayer == NULL )
			return( false );	// NPC's have no privs.
		return( m_pPlayer->GetAccount()->IsPriv( flag ));
	}
	PLEVEL_TYPE GetPrivLevel() const
	{
		// The higher the better. // PLEVEL_Counsel
		if ( ! m_pPlayer )
			return( PLEVEL_Player );
		return( m_pPlayer->GetAccount()->GetPrivLevel());
	}

	CCharBase * Char_GetDef() const
	{
		return( STATIC_CAST <CCharBase*>( Base_GetDef()));
	}
	CRegionWorld * GetRegion() const
	{
		return m_pArea; // What region are we in now. (for guarded message)
	}

	bool IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwArg );
	bool IsResourceMatch( RESOURCE_ID_BASE rid, DWORD dwArg, DWORD dwArgResearch );

	bool IsSpeakAsGhost() const
	{
		return( IsStatFlag(STATF_DEAD) &&
			! IsStatFlag( STATF_SpiritSpeak ) &&
			! IsPriv( PRIV_GM ));
	}
	bool CanUnderstandGhost() const
	{
		// Can i understand player ghost speak ?
		if ( m_pNPC && m_pNPC->m_Brain == NPCBRAIN_HEALER )
			return( true );
		return( IsStatFlag( STATF_SpiritSpeak | STATF_DEAD ) || IsPriv( PRIV_GM|PRIV_HEARALL ));
	}
	bool IsRespawned() const
	{
		// Can be resurrected later.
		return( m_pPlayer ||
			( IsStatFlag( STATF_RespawnNPC ) &&
			m_ptHome.IsValidPoint() && 
			! IsStatFlag( STATF_Spawned|STATF_Conjured )));
	}
	bool IsHuman() const
	{
		return( CCharBase::IsHumanID( GetDispID()));
	}
	int	 GetHealthPercent() const;
	LPCTSTR GetTradeTitle() const; // Paperdoll title for character p (2)

	// Information about us.
	CREID_TYPE GetID() const
	{
		CCharBase * pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		return( pCharDef->GetID());
	}
	WORD GetBaseID() const
	{
		return( GetID());
	}
	CREID_TYPE GetDispID() const
	{
		CCharBase * pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		return( pCharDef->GetDispID());
	}
	void SetID( CREID_TYPE id );

	CItemBase *		GetKeyItemBase( LPCTSTR pszKey );

	LPCTSTR GetKeyStr( LPCTSTR pszKey ) const
	{
		return m_TagDefs.GetKeyStr( pszKey );
	}

	CVarDefBase * GetKey( LPCTSTR pszKey, bool fDef ) const
	{
		CVarDefBase	* pVar	= m_TagDefs.GetKey( pszKey );
		if ( !fDef || pVar )	return pVar;
		CCharBase * pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		return pCharDef->m_TagDefs.GetKey( pszKey );
	}

	CVarDefBase * GetKeyAny( LPCTSTR pszKey ) const
	{

		return m_TagDefs.GetKey( pszKey );
	}

	LPCTSTR GetName() const
	{
		return GetName( true );
	}

	LPCTSTR GetName( bool fAllowAlt ) const
	{
		if ( fAllowAlt )
		{
			LPCTSTR pAltName = GetKeyStr( "NAME.ALT" );
			if ( pAltName && *pAltName )
				return pAltName;
		}
		if ( ! IsIndividualName())			// allow some creatures to go unnamed.
		{
			CCharBase * pCharDef = Char_GetDef();
			ASSERT(pCharDef);
			return( pCharDef->GetTypeName());	// Just use it's type name instead.
		}
		return( CObjBase::GetName());
	}

	bool SetName( LPCTSTR pName );

	bool CanSeeTrue( const CChar * pChar = NULL ) const
	{
		if ( pChar == NULL || pChar == this )
			return( false );
		// if ( pChar->IsStatFlag( STATF_Sleeping )) return( false );
		return( pChar->GetPrivLevel() >= GetPrivLevel());
	}
	bool CanSeeInContainer( const CItemContainer * pContItem ) const;
	bool CanSee( const CObjBaseTemplate * pObj ) const;
	bool CanSeeLOS( const CPointMap & pd, CPointMap * pBlock = NULL, int iMaxDist = UO_MAP_VIEW_SIGHT ) const;
	bool CanSeeLOS( const CObjBaseTemplate * pObj ) const;
	bool CanHear( const CObjBaseTemplate * pSrc, TALKMODE_TYPE mode ) const;
	bool CanSeeItem( const CItem * pItem ) const
	{
		ASSERT(pItem);
		if ( ! IsPriv( PRIV_GM ))
		{
			if ( pItem->IsAttr( ATTR_INVIS ))
				return( false );
		}
		return( true );
	}
	bool CanTouch( const CPointMap & pt ) const;
	bool CanTouch( const CObjBase * pObj ) const;
	IT_TYPE CanTouchStatic( CPointMap & pt, ITEMID_TYPE id, CItem * pItem );
	bool CanMove( CItem * pItem, bool fMsg = true ) const;
	BYTE GetLightLevel() const;
	bool CanUse( CItem * pItem, bool fMoveOrConsume ) const;

	int  Food_CanEat( CObjBase * pObj ) const;
	int  Food_GetLevelPercent() const;
	LPCTSTR Food_GetLevelMessage( bool fPet, bool fHappy ) const;

public:
	short	Stat_GetAdjusted( STAT_TYPE i ) const;

	void	Stat_SetBase( STAT_TYPE i, short iVal );
	short	Stat_GetBase( STAT_TYPE i ) const;
	void	Stat_AddBase( STAT_TYPE i, short iVal );

	void	Stat_AddMod( STAT_TYPE i, short iVal );
	void	Stat_SetMod( STAT_TYPE i, short iVal );
	short	Stat_GetMod( STAT_TYPE i ) const;

	void	Stat_SetVal( STAT_TYPE i, short iVal );
	short	Stat_GetVal( STAT_TYPE i ) const;

	void	Stat_SetMax( STAT_TYPE i, short iVal );
	short	Stat_GetMax( STAT_TYPE i ) const;

	short	Stat_GetLimit( STAT_TYPE i ) const;

	// Location and movement ------------------------------------
private:
	bool TeleportToCli( int iType, int iArgs );
	bool TeleportToObj( int iType, TCHAR * pszArgs );
private:
	CRegionBase * CheckValidMove( CPointBase & pt, WORD * pwBlockFlags, DIR_TYPE dir = DIR_QTY ) const;
	CRegionBase * CheckValidMove_New( CPointBase & ptDest, WORD * pwBlockFlags, DIR_TYPE dir, signed char * ClimbHeight ) const;
	void FixClimbHeight();
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
	bool MoveToChar( CPointMap pt );
	bool MoveTo( CPointMap pt )
	{
		m_fClimbUpdated = false; // update climb height
		return MoveToChar( pt );
	}
	virtual void SetTopZ( signed char z )
	{
		CObjBaseTemplate::SetTopZ( z );
		m_fClimbUpdated = false; // update climb height
		FixClimbHeight();
	}
	bool MoveToValidSpot( DIR_TYPE dir, int iDist, int iDistStart = 1 );
	virtual bool MoveNearObj( const CObjBaseTemplate * pObj, int iSteps = 0, WORD wCan = CAN_C_WALK )
	{
		return CObjBase::MoveNearObj( pObj, iSteps, GetMoveBlockFlags());
	}
	void MoveNear( CPointMap pt, int iSteps = 0, WORD wCan = CAN_C_WALK )
	{
		CObjBase::MoveNear( pt, iSteps, GetMoveBlockFlags());
	}

	CRegionBase * CanMoveWalkTo( CPointBase & pt, bool fCheckChars = true, bool fCheckOnly = false, DIR_TYPE dir = DIR_QTY );
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

	bool SetPrivLevel( CTextConsole * pSrc, LPCTSTR pszFlags );
	bool IsDND() const
	{
		return( GetPrivLevel() >= PLEVEL_Counsel && IsStatFlag( STATF_Insubstantial ));
	}
	bool CanDisturb( CChar * pChar ) const;
	void SetDisconnected();
	bool SetPlayerAccount( CAccount * pAccount );
	bool SetPlayerAccount( LPCTSTR pszAccount );
	bool SetNPCBrain( NPCBRAIN_TYPE NPCBrain );
	NPCBRAIN_TYPE GetNPCBrain( bool fDefault = false ) const; // return 1 for animal, 2 for monster, 3 for NPC humans and PCs
	void ClearNPC();
	void ClearPlayer();

public:
	void ObjMessage( LPCTSTR pMsg, const CObjBase * pSrc ) const
	{
		if ( ! IsClient())
			return;
		GetClient()->addObjMessage( pMsg, pSrc );
	}
	void SysMessage( LPCTSTR pMsg ) const	// Push a message back to the client if there is one.
	{
		if ( ! IsClient())
			return;
		GetClient()->SysMessage( pMsg );
	}

	void UpdateStatsFlag() const;
	void UpdateStatVal( STAT_TYPE x, int iChange = 0, int iLimit = 0 );
	void UpdateHitsFlag();
	void UpdateManaFlag() const;
	void UpdateStamFlag() const;
	void UpdateHitsForOthers() const;
	bool UpdateAnimate( ANIM_TYPE action, bool fTranslate = true, bool fBackward = false, BYTE iFrameDelay = 1 );

	void UpdateMode(  CClient * pExcludeClient = NULL, bool fFull= false );
	void UpdateMove( CPointMap pold, CClient * pClientExclude = NULL, bool fFull = false );
	void UpdateDir( DIR_TYPE dir );
	void UpdateDir( const CPointMap & pt );
	void UpdateDir( const CObjBaseTemplate * pObj );
	void UpdateDrag( CItem * pItem, CObjBase * pCont = NULL, CPointMap * pp = NULL );
	void Update( const CClient * pClientExclude = NULL );

public:
	LPCTSTR GetPronoun() const;	// he
	LPCTSTR GetPossessPronoun() const;	// his
	BYTE GetModeFlag( bool fTrueSight = false ) const;
	BYTE GetDirFlag() const
	{
		BYTE dir = m_dirFace;
		ASSERT( dir<DIR_QTY );
		if ( IsStatFlag( STATF_Fly ))
			dir |= 0x80; // running/flying ?
		return( dir );
	}
	WORD GetMoveBlockFlags() const
	{
		// What things block us ?
		if ( IsPriv(PRIV_GM|PRIV_ALLMOVE))	// nothing blocks us.
			return( 0xFFFF );
		CCharBase * pCharDef = Char_GetDef();
		ASSERT(pCharDef);
		return( pCharDef->m_Can );
	}

	int FixWeirdness();
	void CreateNewCharCheck();

private:
	// Contents/Carry stuff. ---------------------------------
	void ContentAdd( CItem * pItem )
	{
		LayerAdd( pItem, LAYER_QTY );
	}
protected:
	void OnRemoveOb( CGObListRec* pObRec );	// Override this = called when removed from list.
public:
	LAYER_TYPE CanEquipLayer( CItem * pItem, LAYER_TYPE layer, CChar * pCharMsg, bool fTest );
	CItem * LayerFind( LAYER_TYPE layer ) const;
	void LayerAdd( CItem * pItem, LAYER_TYPE layer = LAYER_QTY );
	
	TRIGRET_TYPE OnCharTrigForLayerLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, LAYER_TYPE layer );
	TRIGRET_TYPE OnCharTrigForMemTypeLoop( CScript &s, CTextConsole * pSrc, CScriptTriggerArgs * pArgs, CGString * pResult, WORD wMemType );

	void OnWeightChange( int iChange );
	int GetWeight( void ) const
	{
		return( CContainer::GetTotalWeight());
	}
	int GetWeightLoadPercent( int iWeight ) const;
	bool CanCarry( const CItem * pItem ) const;

	CItem * GetSpellbook( SPELL_TYPE iSpell = SPELL_Clumsy ) const;
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

//private:
	TRIGRET_TYPE OnTrigger( LPCTSTR pTrigName, CTextConsole * pSrc, CScriptTriggerArgs * pArgs );

public:
	// Load/Save----------------------------------

	virtual void r_Serialize( CGFile & a );	// binary
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_Load( CScript & s );  // Load a character from Script
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & s, CTextConsole * pSrc = NULL );
	virtual void r_Write( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	void r_WriteParity( CScript & s );	

	TRIGRET_TYPE OnTrigger( CTRIG_TYPE trigger, CTextConsole * pSrc, CScriptTriggerArgs * pArgs = NULL )
	{
		ASSERT( trigger < CTRIG_QTY );
		return( OnTrigger( MAKEINTRESOURCE(trigger), pSrc, pArgs ));
	}

private:
	// Noto/Karma stuff. --------------------------------
	void Noto_Karma( int iKarma, int iBottom=-10000 );
	void Noto_Fame( int iFameChange );
	void Noto_ChangeNewMsg( int iPrv );
	void Noto_ChangeDeltaMsg( int iDelta, LPCTSTR pszType );

public:
	NOTO_TYPE Noto_GetFlag( const CChar * pChar, bool fIncog = false ) const;
	HUE_TYPE Noto_GetHue( const CChar * pChar, bool fIncog = false ) const;
	bool Noto_IsNeutral() const;
	bool Noto_IsMurderer() const;
	bool Noto_IsEvil() const;
	bool Noto_IsCriminal() const
	{
		// do the guards hate me ?
		if ( IsStatFlag( STATF_Criminal ))
			return( true );
		return Noto_IsEvil();
	}
	int Noto_GetLevel() const;
	LPCTSTR Noto_GetFameTitle() const;
	LPCTSTR Noto_GetTitle() const;

	void Noto_Kill( CChar * pKill, bool fPetKill = false );
	void Noto_Criminal();
	void Noto_Murder();
	void Noto_KarmaChangeMessage( int iKarmaChange, int iLimit );

	bool IsTakeCrime( const CItem * pItem, CChar ** ppCharMark = NULL ) const;

public:
	// skills and actions. -------------------------------------------
	static bool IsSkillBase( SKILL_TYPE skill );
	static bool IsSkillNPC( SKILL_TYPE skill );

	SKILL_TYPE Skill_GetBest( int iRank = 0 ) const; // Which skill is the highest for character p
	SKILL_TYPE Skill_GetActive() const
	{
		return( m_Act_SkillCurrent );
	}
	LPCTSTR Skill_GetName( bool fUse = false ) const;
	short Skill_GetBase( SKILL_TYPE skill ) const
	{
		ASSERT( IsSkillBase(skill));
		return( m_Skill[skill] );
	}
	int Skill_GetMax( SKILL_TYPE skill ) const;
	SKILLLOCK_TYPE Skill_GetLock( SKILL_TYPE skill ) const
	{
		if ( ! m_pPlayer )
			return( SKILLLOCK_UP );
		return( m_pPlayer->Skill_GetLock(skill));
	}
	short Skill_GetAdjusted( SKILL_TYPE skill ) const;
	void Skill_SetBase( SKILL_TYPE skill, short wValue );
	bool Skill_UseQuick( SKILL_TYPE skill, int difficulty );

	bool Skill_CheckSuccess( SKILL_TYPE skill, int difficulty ) const;
	bool Skill_Wait( SKILL_TYPE skilltry );
	bool Skill_Start( SKILL_TYPE skill, int iDifficulty = 0 ); // calc skill progress.
	void Skill_Fail( bool fCancel = false );
	int Skill_Stage( SKTRIG_TYPE stage );
	TRIGRET_TYPE	Skill_OnTrigger( SKILL_TYPE skill, SKTRIG_TYPE  stage, int * argn2 = NULL, int * argn3 = NULL ); // call script triggers

	bool Skill_Mining_Smelt( CItem * pItemOre, CItem * pItemTarg );
	bool Skill_Tracking( CGrayUID uidTarg, DIR_TYPE & dirPrv, int iDistMax = SHRT_MAX );
	bool Skill_MakeItem( ITEMID_TYPE id, CGrayUID uidTarg, SKTRIG_TYPE stage, bool fSkillOnly = false );
	bool Skill_MakeItem_Success();
	bool Skill_Snoop_Check( const CItemContainer * pItem );
	void Skill_Cleanup();	 // may have just cancelled targetting.

	// test for skill towards making an item
	int SkillResourceTest( const CResourceQtyArray * pResources, ITEMID_TYPE id );

	void Spell_Effect_Remove( CItem * pSpell, bool fOldStyle = false );
	void Spell_Effect_Add( CItem * pSpell );

private:
	int Skill_Done();	 // complete skill (async)
	bool Skill_Degrade( SKILL_TYPE skill );
	void Skill_Experience( SKILL_TYPE skill, int difficulty );

	int Skill_NaturalResource_Setup( CItem * pResBit );
	CItem * Skill_NaturalResource_Create( CItem * pResBit, SKILL_TYPE skill );
	void Skill_SetTimeout();

	int	Skill_Scripted( SKTRIG_TYPE stage );

	int Skill_Inscription( SKTRIG_TYPE stage );
	int Skill_MakeItem( SKTRIG_TYPE stage );
	int Skill_Information( SKTRIG_TYPE stage );
	int Skill_Hiding( SKTRIG_TYPE stage );
	int Skill_Enticement( SKTRIG_TYPE stage );
	int Skill_Bowcraft( SKTRIG_TYPE stage );
	int Skill_Snooping( SKTRIG_TYPE stage );
	int Skill_Stealing( SKTRIG_TYPE stage );
	int Skill_Mining( SKTRIG_TYPE stage );
	int Skill_Lumberjack( SKTRIG_TYPE stage );
	int Skill_Taming( SKTRIG_TYPE stage );
	int Skill_Fishing( SKTRIG_TYPE stage );
	int Skill_Cartography(SKTRIG_TYPE stage);
	int Skill_DetectHidden(SKTRIG_TYPE stage);
	int Skill_Herding(SKTRIG_TYPE stage);
	int Skill_Blacksmith(SKTRIG_TYPE stage);
	int Skill_Lockpicking(SKTRIG_TYPE stage);
	int Skill_Peacemaking(SKTRIG_TYPE stage);
	int Skill_Alchemy(SKTRIG_TYPE stage);
	int Skill_Carpentry(SKTRIG_TYPE stage);
	int Skill_Provocation(SKTRIG_TYPE stage);
	int Skill_Poisoning(SKTRIG_TYPE stage);
	int Skill_Cooking(SKTRIG_TYPE stage);
	int Skill_Healing(SKTRIG_TYPE stage);
	int Skill_Meditation(SKTRIG_TYPE stage);
	int Skill_RemoveTrap(SKTRIG_TYPE stage);
	int Skill_Begging( SKTRIG_TYPE stage );
	int Skill_SpiritSpeak( SKTRIG_TYPE stage );
	int Skill_Magery( SKTRIG_TYPE stage );
	int Skill_Tracking( SKTRIG_TYPE stage );
	int Skill_Fighting( SKTRIG_TYPE stage );
	int Skill_Musicianship( SKTRIG_TYPE stage );
	int Skill_Tailoring( SKTRIG_TYPE stage );

	int Skill_Act_Napping(SKTRIG_TYPE stage);
	int Skill_Act_Throwing(SKTRIG_TYPE stage);
	int Skill_Act_Breath(SKTRIG_TYPE stage);
	int Skill_Act_Training( SKTRIG_TYPE stage );
	int Skill_Act_Looting( SKTRIG_TYPE stage );

	void Spell_Dispel( int iskilllevel );
CChar * Spell_Summon( CREID_TYPE id, CPointMap pt, bool fPet );
	bool Spell_Recall( CItem * pRune, bool fGate );
	CItem * Spell_Effect_Create( SPELL_TYPE spell, LAYER_TYPE layer, int iSkillLevel, int iDuration, CObjBase * pSrc = NULL );
	bool Spell_Equip_OnTick( CItem * pItem );

	void Spell_Bolt( CObjBase * pObj, ITEMID_TYPE idBolt, int iSkill );
	void Spell_Field( CPointMap pt, ITEMID_TYPE idEW, ITEMID_TYPE idNS, int iSkill );
	void Spell_Area( CPointMap pt, int iDist, int iSkill );
	bool Spell_TargCheck();
	bool Spell_Unequip( LAYER_TYPE layer );

	int  Spell_CastStart();
	void Spell_CastFail();

public:
	bool Spell_CastDone();
	bool OnSpellEffect( SPELL_TYPE spell, CChar * pCharSrc, int iSkillLevel, CItem * pSourceItem );
	bool Spell_Resurrection( int iSkillLoss, CItemCorpse * pCorpse );
	bool Spell_Teleport( CPointBase pt, bool fTakePets = false, bool fCheckAntiMagic = true, ITEMID_TYPE iEffect = ITEMID_TEMPLATE, SOUND_TYPE iSound = 0x01fe );
	bool Spell_CanCast( SPELL_TYPE spell, bool fTest, CObjBase * pSrc, bool fFailMsg );
	int	GetSpellEffect( SPELL_TYPE spell, int iSkillLevel, int iEffectMult );
	int	GetSpellDuration( SPELL_TYPE spell, int iSkillLevel, int iEffectMult );
	// Memories about objects in the world. -------------------
private:
	bool Memory_OnTick( CItemMemory * pMemory );
	bool Memory_UpdateFlags( CItemMemory * pMemory );
	bool Memory_UpdateClearTypes( CItemMemory * pMemory, WORD MemTypes );
	void Memory_AddTypes( CItemMemory * pMemory, WORD MemTypes );
	bool Memory_ClearTypes( CItemMemory * pMemory, WORD MemTypes );
	CItemMemory * Memory_CreateObj( CGrayUID uid, WORD MemTypes );
	CItemMemory * Memory_CreateObj( const CObjBase * pObj, WORD MemTypes )
	{
		ASSERT(pObj);
		return Memory_CreateObj( pObj->GetUID(), MemTypes );
	}

public:
	void Memory_ClearTypes( WORD MemTypes );
	CItemMemory * Memory_FindObj( CGrayUID uid ) const;
	CItemMemory * Memory_FindObj( const CObjBase * pObj ) const
	{
		if ( pObj == NULL )
			return( NULL );
		return Memory_FindObj( pObj->GetUID());
	}
	CItemMemory * Memory_AddObjTypes( CGrayUID uid, WORD MemTypes );
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
	// -------- Public alias for MemoryCreateObj ------------------
	CItemMemory * Memory_AddObj( CGrayUID uid, WORD MemTypes )
	{
		return Memory_CreateObj( uid, MemTypes );
	}
	CItemMemory * Memory_AddObj( const CObjBase * pObj, WORD MemTypes )
	{
		return Memory_CreateObj( pObj, MemTypes );
	}
	// ------------------------------------------------------------

public:
	SOUND_TYPE SoundChar( CRESND_TYPE type );
	void Action_StartSpecial( CREID_TYPE id );

private:
	void OnNoticeCrime( CChar * pCriminal, const CChar * pCharMark );
public:
	bool CheckCrimeSeen( SKILL_TYPE SkillToSee, CChar * pCharMark, const CObjBase * pItem, LPCTSTR pAction );

private:
	// Armor, weapons and combat ------------------------------------
	int	CalcFightRange( CItem * pWeapon = NULL, CItemBase * pWeaponDef = NULL );

	SKILL_TYPE Fight_GetWeaponSkill() const;
	void Fight_ResetWeaponSwingTimer();
	int Fight_GetWeaponSwingTimer();
	bool Fight_IsActive() const;
public:
	int CalcArmorDefense( void ) const;

	void Memory_Fight_Retreat( CChar * pTarg, CItemMemory * pFight );
	void Memory_Fight_Start( const CChar * pTarg );
	bool Memory_Fight_OnTick( CItemMemory * pMemory );

	bool Fight_Stunned( DIR_TYPE dir );
	bool Fight_Attack( const CChar * pCharTarg );
	bool Fight_Clear( const CChar * pCharTarg );
	void Fight_ClearAll();
	CChar * Fight_FindBestTarget();
	bool Fight_AttackNext();
	void Fight_HitTry( void );
	WAR_SWING_TYPE Fight_Hit( CChar * pCharTarg );
	int  Fight_CalcDamage( CItem * pWeapon, SKILL_TYPE skill ) const;

	bool Player_OnVerb( CScript &s, CTextConsole * pSrc );
	void InitPlayer( const CEvent * pEvent, CClient * pClient );
	bool ReadScriptTrig( CCharBase * pCharDef, CTRIG_TYPE trig );
	bool ReadScript( CResourceLock &s );
	void NPC_LoadScript( bool fRestock );

	// Mounting and figurines
	bool Horse_Mount( CChar * pHorse ); // Remove horse char and give player a horse item
	bool Horse_UnMount(); // Remove horse char and give player a horse item

private:
	CItem * Horse_GetMountItem() const;
	CChar * Horse_GetMountChar() const;
public:
	CChar * Use_Figurine( CItem * pItem, int iPaces = 0 );
	CItem * Make_Figurine( CGrayUID uidOwner, ITEMID_TYPE id = ITEMID_NOTHING );
	CItem * NPC_Shrink();

	int  ItemPickup( CItem * pItem, int amount );
	bool ItemEquip( CItem * pItem, CChar * pCharMsg = NULL );
	bool ItemEquipWeapon( bool fForce );
	bool ItemEquipArmor( bool fForce );
	bool ItemBounce( CItem * pItem );
	bool ItemDrop( CItem * pItem, const CPointMap & pt );

	void Flip( LPCTSTR pCmd = NULL );
	bool SetPoison( int iLevel, int iTicks, CChar * pCharSrc );
	bool SetPoisonCure( int iLevel, bool fExtra );
	bool CheckCorpseCrime( const CItemCorpse *pCorpse, bool fLooting, bool fTest );
	CItemCorpse * FindMyCorpse( int iRadius = 2 ) const;
	CItemCorpse * MakeCorpse( bool fFrontFall );
	bool RaiseCorpse( CItemCorpse * pCorpse );
	bool Death();
	bool Reveal( DWORD dwFlags = ( STATF_Invisible | STATF_Hidden | STATF_Sleeping ));
	void Jail( CTextConsole * pSrc, bool fSet, int iCell );
	void EatAnim( LPCTSTR pszName, int iQty );
	void CallGuards( CChar * pCriminal );

	virtual void Speak( LPCTSTR pText, HUE_TYPE wHue = HUE_TEXT_DEF, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_NORMAL );
	virtual void SpeakUTF8( LPCTSTR pText, HUE_TYPE wHue= HUE_TEXT_DEF, TALKMODE_TYPE mode= TALKMODE_SAY, FONT_TYPE font= FONT_NORMAL, CLanguageID lang = 0 );
	virtual void SpeakUTF8Ex( const NWORD * pText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang );

	bool OnFreezeCheck();
	void DropAll( CItemContainer * pCorpse = NULL, WORD wAttr = 0 );
	void UnEquipAllItems( CItemContainer * pCorpse = NULL );
	void CancelAllTrades();
	void Wake();
	void SleepStart( bool fFrontFall );

	void Guild_Resign( MEMORY_TYPE memtype );
	CItemStone * Guild_Find( MEMORY_TYPE memtype ) const;
	CStoneMember * Guild_FindMember( MEMORY_TYPE memtype ) const;
	LPCTSTR Guild_Abbrev( MEMORY_TYPE memtype ) const;
	LPCTSTR Guild_AbbrevBracket( MEMORY_TYPE memtype ) const;

	void Use_EatQty( CItem * pFood, int iQty = 1 );
	bool Use_Eat( CItem * pItem, int iQty = 1 );
	bool Use_MultiLockDown( CItem * pItemTarg );
	void Use_CarveCorpse( CItemCorpse * pCorpse );
	bool Use_Repair( CItem * pItem );
	int  Use_PlayMusic( CItem * pInstrument, int iDifficultyToPlay );
	bool Use_Drink( CItem * pItem );
	bool Use_Cannon_Feed( CItem * pCannon, CItem * pFeed );
	bool Use_Item_Web( CItem * pItem );
	void Use_AdvanceGate( CItem * pItem );
	void Use_MoonGate( CItem * pItem );
	bool Use_Kindling( CItem * pKindling );
	bool Use_BedRoll( CItem * pItem );
	bool Use_Seed( CItem * pItem, CPointMap * pPoint );
	bool Use_Key( CItem * pKey, CItem * pItemTarg );
	bool Use_KeyChange( CItem * pItemTarg );
	bool Use_Train_PickPocketDip( CItem * pItem, bool fSetup );
	bool Use_Train_Dummy( CItem * pItem, bool fSetup );
	bool Use_Train_ArcheryButte( CItem * pButte, bool fSetup );
	bool Use_Item( CItem * pItem, bool fLink = false );
	bool Use_Obj( CObjBase * pObj, bool fTestTouch, bool fScript = false );

	// NPC AI -----------------------------------------
private:
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
	int NPC_GetWeaponUseScore( CItem * pItem );

	int  NPC_GetHostilityLevelToward( const CChar * pCharTarg ) const;
	int	 NPC_GetAttackContinueMotivation( CChar * pChar, int iMotivation = 0 ) const;
	int  NPC_GetAttackMotivation( CChar * pChar, int iMotivation = 0 ) const;
	bool NPC_CheckHirelingStatus();
	int  NPC_GetTrainMax( const CChar * pStudent, SKILL_TYPE Skill ) const;

	bool NPC_OnVerb( CScript &s, CTextConsole * pSrc = NULL );
	bool NPC_MotivationCheck( int iMotivation );
	void NPC_OnHirePayMore( CItem * pGold, bool fHire = false );
	bool NPC_OnHirePay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold );
	bool NPC_OnHireHear( CChar * pCharSrc );
	bool NPC_Script_Command( LPCTSTR pCmd, bool fSystemCheck );
	void NPC_Script_OnTick( CItemMessage * pScriptItem, bool fForce = false );
	int	 NPC_OnTrainCheck( CChar * pCharSrc, SKILL_TYPE Skill );
	bool NPC_OnTrainPay( CChar * pCharSrc, CItemMemory * pMemory, CItem * pGold );
	bool NPC_OnTrainHear( CChar * pCharSrc, LPCTSTR pCmd );
	bool NPC_CheckWalkHere( const CPointBase & pt, const CRegionBase * pArea, WORD wBlockFlags ) const;
	void NPC_OnNoticeSnoop( CChar * pCharThief, CChar * pCharMark );

	bool NPC_LootContainer( CItemContainer * pItem );
	void NPC_LootMemory( CItem * pItem );
	bool NPC_LookAtCharGuard( CChar * pChar );
	bool NPC_LookAtCharHealer( CChar * pChar );
	bool NPC_LookAtCharHuman( CChar * pChar );
	bool NPC_LookAtCharMonster( CChar * pChar );
	bool NPC_LookAtChar( CChar * pChar, int iDist );
	bool NPC_LookAtItem( CItem * pItem, int iDist );
	bool NPC_LookAround( bool fForceCheckItems = false );
	int  NPC_WalkToPoint( bool fRun = false );
	bool NPC_FightMagery( CChar * pChar );
	bool NPC_FightArchery( CChar * pChar );
	bool NPC_FightMayCast() const;

	bool NPC_Act_Follow( bool fFlee = false, int maxDistance = 1, bool forceDistance = false );
	void NPC_Act_GoHome();
	bool NPC_Act_Talk();
	void NPC_Act_Wander();
	bool NPC_Act_Begging( CChar * pChar );
	void NPC_Act_Fight();
	void NPC_Act_Idle();
	void NPC_Act_Looting();
	void NPC_Act_Eating();
	void NPC_Act_Flee();
	void NPC_Act_Goto();

	void NPC_ActStart_SpeakTo( CChar * pSrc );

	void NPC_OnTickAction();
	bool NPC_OnTickFood( int nHungerPercent );

public:
	void NPC_PetDesert();	
	void NPC_PetClearOwners();
	bool NPC_PetSetOwner( const CChar * pChar );
	CChar * NPC_PetGetOwner() const;
	bool NPC_IsOwnedBy( const CChar * pChar, bool fAllowGM = true ) const;
	bool NPC_IsSpawnedBy( const CItem * pItem ) const;
	bool NPC_CanSpeak() const;

	static CItemVendable * NPC_FindVendableItem( CItemVendable * pVendItem, CItemContainer * pVend1, CItemContainer * pVend2 );

	bool NPC_IsVendor() const
	{
		if ( ! m_pNPC )
			return( false );
		return( m_pNPC->IsVendor());
	}
	bool NPC_Vendor_Restock( int iTimeSec );
	bool NPC_Vendor_Dump( CItemContainer * pBank );
	int NPC_GetVendorMarkup( const CChar * pChar ) const;

	void NPC_OnPetCommand( bool fSuccess, CChar * pMaster );
	bool NPC_OnHearPetCmd( LPCTSTR pszCmd, CChar * pSrc, bool fAllpets );
	bool NPC_OnHearPetCmdTarg( int iCmd, CChar * pSrc, CObjBase * pObj, const CPointMap & pt, LPCTSTR pszArgs );
	int  NPC_OnHearName( LPCTSTR pszText ) const;
	void NPC_OnHear( LPCTSTR pCmd, CChar * pSrc );
	bool NPC_OnItemGive( CChar * pCharSrc, CItem * pItem );
	bool NPC_SetVendorPrice( CItem * pItem, int iPrice );
	bool OnTriggerSpeech( LPCTSTR pszName, LPCTSTR pszText, CChar * pSrc, TALKMODE_TYPE & mode );

	// Outside events that occur to us.
	int  OnTakeDamage( int iDmg, CChar * pSrc, DAMAGE_TYPE uType );
	int  OnTakeDamageHitPoint( int iDmg, CChar * pSrc, DAMAGE_TYPE uType );
	void OnHarmedBy( CChar * pCharSrc, int iHarmQty );
	bool OnAttackedBy( CChar * pCharSrc, int iHarmQty, bool fPetsCommand = false );

	void OnHearEquip( CChar * pChar, TCHAR * szText );
	bool OnTickEquip( CItem * pItem );
	void OnTickFood();
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

#endif
