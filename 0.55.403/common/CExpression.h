//
// CExpression.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CEXPRSSION_H
#define _INC_CEXPRSSION_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CAtom.h"

#define _ISCSYMF(ch) ( isalpha(ch) || (ch)=='_')	// __iscsym or __iscsymf
#define _ISCSYM(ch) ( isalnum(ch) || (ch)=='_')	// __iscsym or __iscsymf

class CVarDefBase : public CMemDynamic	// A variable from GRAYDEFS.SCP or other.
{
	// Similar to CScriptKey
private:
#define EXPRESSION_MAX_KEY_LEN SCRIPT_MAX_SECTION_LEN
	const CAtomRef m_aKey;	// the key for sorting/ etc.
public:
	LPCTSTR GetKey() const
	{
		return( m_aKey.GetStr() );
	}
	CVarDefBase( LPCTSTR pszKey ) :
		m_aKey( pszKey )
	{
	}
	virtual LPCTSTR GetValStr() const = 0;
	virtual int GetValNum() const = 0;
	virtual CVarDefBase * CopySelf() const = 0;
};

class CVarDefNum : public CVarDefBase
{
	// Simple number equiv.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	int m_iVal;	// the assigned value.
public:
	int GetValNum() const
	{
		return( m_iVal );
	}
	void SetValNum( int iVal )
	{
		m_iVal = iVal;
	}
	LPCTSTR GetValStr() const;
	bool r_LoadVal( CScript & s )
	{
		SetValNum( s.GetArgVal());
		return( true );
	}
	bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc = NULL )
	{
		UNREFERENCED_PARAMETER(pKey);
		UNREFERENCED_PARAMETER(pSrc);
		sVal.FormatVal( GetValNum());
		return( true );
	}
	virtual CVarDefBase * CopySelf() const
	{
		return new CVarDefNum( GetKey(), m_iVal );
	}
	CVarDefNum( LPCTSTR pszKey, int iVal ) :
		CVarDefBase( pszKey ),
		m_iVal( iVal )
	{
	}
	CVarDefNum( LPCTSTR pszKey ) :
		CVarDefBase( pszKey )
	{
	}
};

class CVarDefStr : public CVarDefBase
{
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sVal;	// the assigned value. (What if numeric?)
public:
	LPCTSTR GetValStr() const
	{
		return( m_sVal );
	}
	int GetValNum() const;
	void SetValStr( LPCTSTR pszVal )
	{
		m_sVal.Copy( pszVal );
	}
	bool r_LoadVal( CScript & s )
	{
		SetValStr( s.GetArgStr());
		return( true );
	}
	bool r_WriteVal( LPCTSTR pKey, CGString & sVal, CTextConsole * pSrc = NULL )
	{
		UNREFERENCED_PARAMETER(pKey);
		UNREFERENCED_PARAMETER(pSrc);
		sVal = GetValStr();
		return( true );
	}
	virtual CVarDefBase * CopySelf() const
	{
		return new CVarDefStr( GetKey(), m_sVal );
	}
	CVarDefStr( LPCTSTR pszKey, LPCTSTR pszVal ) :
		CVarDefBase( pszKey ),
		m_sVal( pszVal )
	{
	}
	CVarDefStr( LPCTSTR pszKey ) :
		CVarDefBase( pszKey )
	{
	}
};

struct CVarDefArray : public CGObSortArray< CVarDefBase *, LPCTSTR>
{
	// Sorted array
protected:
	int CompareKey( LPCTSTR pKey, CVarDefBase * pVar, bool fNoSpaces ) const
	{
		ASSERT( pVar );
		return( strcmpi( pKey, pVar->GetKey()));
	}
	int Add( CVarDefBase * pVar )
	{
		int i = AddSortKey( pVar, pVar->GetKey());
		if ( i < 0 )
		{
			// NOTE: pVar has ALREADY BEEN DELETED !!!???
			DEBUG_ERR(( "Duplicate def %s\n", pVar->GetKey()));	// duplicate should not happen.
		}
		return( i );
	}

public:
	void Copy( const CVarDefArray * pArray );

	CVarDefArray & operator = ( const CVarDefArray & array )
	{
		Copy( &array );
		return( *this );
	}

	int FindValNum( int iVal ) const;
	int FindValStr( LPCTSTR pVal ) const;

	// Manipulate the list of Vars
	CVarDefBase * GetKey( LPCTSTR pszKey ) const;
	LPCTSTR GetKeyStr( LPCTSTR pszKey, bool fZero = false ) const;

	CVarDefBase * GetParseKey( LPCTSTR & pArgs ) const;
	bool GetParseVal( LPCTSTR & pArgs, long * plVal ) const;

	int SetNumNew( LPCTSTR pszKey, int iVal );

	int SetNum( LPCTSTR pszKey, int iVal, bool fZero = false );
	int SetStr( LPCTSTR pszKey, bool fQuoted, LPCTSTR pszVal, bool fZero = false );

	bool r_LoadVal( CScript & s )
	{
		bool fQuoted = false;
		return SetStr( s.GetKey(), fQuoted, s.GetArgStr( &fQuoted )) ? true : false;
	}
	void r_WriteKeys( CScript & s )
	{
		// Write with no prefix.
		for ( int i=GetCount(); --i >= 0; )
		{
			const CVarDefBase * pVar = GetAt(i);
			s.WriteKey( pVar->GetKey(), pVar->GetValStr());
		}
	}
	void r_WriteTags( CScript & s );
	void DumpKeys( CTextConsole * pSrc, LPCTSTR pszPrefix = NULL );
	void ClearKeys(LPCTSTR mask = NULL);
};


enum DEFMSG_TYPE
{
	DEFMSG_ALCHEMY_DUNNO,
	DEFMSG_ALCHEMY_LACK,
	DEFMSG_ALCHEMY_NOBOTTLES,
	DEFMSG_ALCHEMY_NOT_REG,
	DEFMSG_ALCHEMY_POUR,
	DEFMSG_ALCHEMY_STAGE_1,
	DEFMSG_ALCHEMY_STAGE_2,
	DEFMSG_ALCHEMY_TOSS,
	DEFMSG_ANATOMY_DEX_1,
	DEFMSG_ANATOMY_DEX_10,
	DEFMSG_ANATOMY_DEX_2,
	DEFMSG_ANATOMY_DEX_3,
	DEFMSG_ANATOMY_DEX_4,
	DEFMSG_ANATOMY_DEX_5,
	DEFMSG_ANATOMY_DEX_6,
	DEFMSG_ANATOMY_DEX_7,
	DEFMSG_ANATOMY_DEX_8,
	DEFMSG_ANATOMY_DEX_9,
	DEFMSG_ANATOMY_MAGIC,
	DEFMSG_ANATOMY_RESULT,
	DEFMSG_ANATOMY_STR_1,
	DEFMSG_ANATOMY_STR_10,
	DEFMSG_ANATOMY_STR_2,
	DEFMSG_ANATOMY_STR_3,
	DEFMSG_ANATOMY_STR_4,
	DEFMSG_ANATOMY_STR_5,
	DEFMSG_ANATOMY_STR_6,
	DEFMSG_ANATOMY_STR_7,
	DEFMSG_ANATOMY_STR_8,
	DEFMSG_ANATOMY_STR_9,
	DEFMSG_ANIMALLORE_CONJURED,
	DEFMSG_ANIMALLORE_FOOD,
	DEFMSG_ANIMALLORE_FREE,
	DEFMSG_ANIMALLORE_MASTER,
	DEFMSG_ANIMALLORE_MASTER_YOU,
	DEFMSG_ANIMALLORE_RESULT,
	DEFMSG_ARMSLORE_DAM,
	DEFMSG_ARMSLORE_DAM_1,
	DEFMSG_ARMSLORE_DAM_10,
	DEFMSG_ARMSLORE_DAM_2,
	DEFMSG_ARMSLORE_DAM_3,
	DEFMSG_ARMSLORE_DAM_4,
	DEFMSG_ARMSLORE_DAM_5,
	DEFMSG_ARMSLORE_DAM_6,
	DEFMSG_ARMSLORE_DAM_7,
	DEFMSG_ARMSLORE_DAM_8,
	DEFMSG_ARMSLORE_DAM_9,
	DEFMSG_ARMSLORE_DEF,
	DEFMSG_ARMSLORE_DEF_1,
	DEFMSG_ARMSLORE_DEF_10,
	DEFMSG_ARMSLORE_DEF_2,
	DEFMSG_ARMSLORE_DEF_3,
	DEFMSG_ARMSLORE_DEF_4,
	DEFMSG_ARMSLORE_DEF_5,
	DEFMSG_ARMSLORE_DEF_6,
	DEFMSG_ARMSLORE_DEF_7,
	DEFMSG_ARMSLORE_DEF_8,
	DEFMSG_ARMSLORE_DEF_9,
	DEFMSG_ARMSLORE_PSN_1,
	DEFMSG_ARMSLORE_PSN_10,
	DEFMSG_ARMSLORE_PSN_2,
	DEFMSG_ARMSLORE_PSN_3,
	DEFMSG_ARMSLORE_PSN_4,
	DEFMSG_ARMSLORE_PSN_5,
	DEFMSG_ARMSLORE_PSN_6,
	DEFMSG_ARMSLORE_PSN_7,
	DEFMSG_ARMSLORE_PSN_8,
	DEFMSG_ARMSLORE_PSN_9,
	DEFMSG_ARMSLORE_REP,
	DEFMSG_ARMSLORE_REP_0,
	DEFMSG_ARMSLORE_UNABLE,
	DEFMSG_BEGGING_START,
	DEFMSG_BVBOX_FULL_ITEMS,
	DEFMSG_BVBOX_FULL_WEIGHT,
	DEFMSG_BVBOX_OPEN_OTHER,
	DEFMSG_BVBOX_OPEN_SELF,
	DEFMSG_CANT_MAKE,
	DEFMSG_CANT_MAKE_RES,
	DEFMSG_CARTOGRAPHY_CANT,
	DEFMSG_CARTOGRAPHY_FAIL,
	DEFMSG_CARTOGRAPHY_NOMAP,
	DEFMSG_CARTOGRAPHY_WMAP,
	DEFMSG_CARVE_CORPSE_1,
	DEFMSG_CARVE_CORPSE_2,
	DEFMSG_CARVE_CORPSE_FEATHERS,
	DEFMSG_CARVE_CORPSE_FUR,
	DEFMSG_CARVE_CORPSE_HIDES,
	DEFMSG_CARVE_CORPSE_MEAT,
	DEFMSG_CARVE_CORPSE_WOOL,
	DEFMSG_CMD_INVALID,
	DEFMSG_CMDAFK_ENTER,
	DEFMSG_CMDAFK_LEAVE,
	DEFMSG_COMBAT_ARCH_NOAMMO,
	DEFMSG_COMBAT_ARCH_TOOCLOSE,
	DEFMSG_COMBAT_ATTACKO,
	DEFMSG_COMBAT_ATTACKS,
	DEFMSG_COMBAT_HIT_ARM1O,
	DEFMSG_COMBAT_HIT_ARM1S,
	DEFMSG_COMBAT_HIT_ARM2O,
	DEFMSG_COMBAT_HIT_ARM2S,
	DEFMSG_COMBAT_HIT_ARM3O,
	DEFMSG_COMBAT_HIT_ARM3S,
	DEFMSG_COMBAT_HIT_BACK1O,
	DEFMSG_COMBAT_HIT_BACK1S,
	DEFMSG_COMBAT_HIT_CHEST1O,
	DEFMSG_COMBAT_HIT_CHEST1S,
	DEFMSG_COMBAT_HIT_CHEST2O,
	DEFMSG_COMBAT_HIT_CHEST2S,
	DEFMSG_COMBAT_HIT_CHEST3O,
	DEFMSG_COMBAT_HIT_CHEST3S,
	DEFMSG_COMBAT_HIT_CHEST4O,
	DEFMSG_COMBAT_HIT_CHEST4S,
	DEFMSG_COMBAT_HIT_CHEST5O,
	DEFMSG_COMBAT_HIT_CHEST5S,
	DEFMSG_COMBAT_HIT_CHEST6O,
	DEFMSG_COMBAT_HIT_CHEST6S,
	DEFMSG_COMBAT_HIT_FEET1O,
	DEFMSG_COMBAT_HIT_FEET1S,
	DEFMSG_COMBAT_HIT_HAND1O,
	DEFMSG_COMBAT_HIT_HAND1S,
	DEFMSG_COMBAT_HIT_HAND2O,
	DEFMSG_COMBAT_HIT_HAND2S,
	DEFMSG_COMBAT_HIT_HAND3O,
	DEFMSG_COMBAT_HIT_HAND3S,
	DEFMSG_COMBAT_HIT_HEAD1O,
	DEFMSG_COMBAT_HIT_HEAD1S,
	DEFMSG_COMBAT_HIT_HEAD2O,
	DEFMSG_COMBAT_HIT_HEAD2S,
	DEFMSG_COMBAT_HIT_HEAD3O,
	DEFMSG_COMBAT_HIT_HEAD3S,
	DEFMSG_COMBAT_HIT_HEAD4O,
	DEFMSG_COMBAT_HIT_HEAD4S,
	DEFMSG_COMBAT_HIT_HEAD5O,
	DEFMSG_COMBAT_HIT_HEAD5S,
	DEFMSG_COMBAT_HIT_HEAD6O,
	DEFMSG_COMBAT_HIT_HEAD6S,
	DEFMSG_COMBAT_HIT_LEGS1O,
	DEFMSG_COMBAT_HIT_LEGS1S,
	DEFMSG_COMBAT_HIT_LEGS2O,
	DEFMSG_COMBAT_HIT_LEGS2S,
	DEFMSG_COMBAT_HIT_LEGS3O,
	DEFMSG_COMBAT_HIT_LEGS3S,
	DEFMSG_COMBAT_HIT_NECK1O,
	DEFMSG_COMBAT_HIT_NECK1S,
	DEFMSG_COMBAT_HIT_NECK2O,
	DEFMSG_COMBAT_HIT_NECK2S,
	DEFMSG_COMBAT_MISSO,
	DEFMSG_COMBAT_MISSS,
	DEFMSG_COMBAT_PARRY,
	DEFMSG_CONT_FULL,
	DEFMSG_CONT_MAGIC,
	DEFMSG_CONT_TOOSMALL,
	DEFMSG_COOKING_SUCCESS,
	DEFMSG_CORPSE_NAME,
	DEFMSG_DETECTHIDDEN_SUCC,
	DEFMSG_DRINK_CANTMOVE,
	DEFMSG_DRINK_POTION_DELAY,
	DEFMSG_EVALINT_INT_1,
	DEFMSG_EVALINT_INT_10,
	DEFMSG_EVALINT_INT_2,
	DEFMSG_EVALINT_INT_3,
	DEFMSG_EVALINT_INT_4,
	DEFMSG_EVALINT_INT_5,
	DEFMSG_EVALINT_INT_6,
	DEFMSG_EVALINT_INT_7,
	DEFMSG_EVALINT_INT_8,
	DEFMSG_EVALINT_INT_9,
	DEFMSG_EVALINT_MAG_1,
	DEFMSG_EVALINT_MAG_2,
	DEFMSG_EVALINT_MAG_3,
	DEFMSG_EVALINT_MAG_4,
	DEFMSG_EVALINT_MAG_5,
	DEFMSG_EVALINT_MAG_6,
	DEFMSG_EVALINT_MAN_1,
	DEFMSG_EVALINT_MAN_2,
	DEFMSG_EVALINT_MAN_3,
	DEFMSG_EVALINT_MAN_4,
	DEFMSG_EVALINT_MAN_5,
	DEFMSG_EVALINT_MAN_6,
	DEFMSG_EVALINT_RESULT,
	DEFMSG_EVALINT_RESULT_2,
	DEFMSG_FISHING_1,
	DEFMSG_FISHING_2,
	DEFMSG_FISHING_3,
	DEFMSG_FISHING_PROMT,
	DEFMSG_FISHING_REACH,
	DEFMSG_FISHING_SUCCESS,
	DEFMSG_FOOD_CANTEAT,
	DEFMSG_FOOD_CANTEATF,
	DEFMSG_FOOD_CANTMOVE,
	DEFMSG_FOOD_FULL_1,
	DEFMSG_FOOD_FULL_2,
	DEFMSG_FOOD_FULL_3,
	DEFMSG_FOOD_FULL_4,
	DEFMSG_FOOD_FULL_5,
	DEFMSG_FOOD_FULL_6,
	DEFMSG_FOOD_RCANTEAT,
	DEFMSG_FORENSICS_ALIVE,
	DEFMSG_FORENSICS_CARVE_1,
	DEFMSG_FORENSICS_CARVE_2,
	DEFMSG_FORENSICS_CORPSE,
	DEFMSG_FORENSICS_FAILNAME,
	DEFMSG_FORENSICS_NAME,
	DEFMSG_FORENSICS_REACH,
	DEFMSG_FORENSICS_TIMER,
	DEFMSG_GMPAGE_PROMPT,
	DEFMSG_GRANDMASTER_MARK,
	DEFMSG_HEALING_AM,
	DEFMSG_HEALING_ATTEMPT,
	DEFMSG_HEALING_ATTEMPTF,
	DEFMSG_HEALING_BEYOND,
	DEFMSG_HEALING_CORPSEG,
	DEFMSG_HEALING_CURE_1,
	DEFMSG_HEALING_CURE_2,
	DEFMSG_HEALING_GHOST,
	DEFMSG_HEALING_HEALTHY,
	DEFMSG_HEALING_INTERRUPT,
	DEFMSG_HEALING_NOAIDS,
	DEFMSG_HEALING_NONCHAR,
	DEFMSG_HEALING_NONEED,
	DEFMSG_HEALING_REACH,
	DEFMSG_HEALING_RES,
	DEFMSG_HEALING_SELF,
	DEFMSG_HEALING_TO,
	DEFMSG_HEALING_TOOFAR,
	DEFMSG_HEALING_WHO,
	DEFMSG_HEALING_WITEM,
	DEFMSG_HERDING_ANNOYED,
	DEFMSG_HERDING_LTARG,
	DEFMSG_HERDING_NOCROOK,
	DEFMSG_HERDING_SUCCESS,
	DEFMSG_HIDING_REVEALED,
	DEFMSG_HIDING_STUMBLE,
	DEFMSG_HIDING_SUCCESS,
	DEFMSG_HIDING_TOOLIT,
	DEFMSG_INSCRIPTION_FAIL,
	DEFMSG_ITEM_MAGIC,
	DEFMSG_ITEM_NEWBIE,
	DEFMSG_ITEM_REPAIR,
	DEFMSG_ITEMID_GOLD,
	DEFMSG_ITEMID_MADEOF,
	DEFMSG_ITEMID_NOVAL,
	DEFMSG_ITEMID_RESULT,
	DEFMSG_ITEMUSE_ADVGATE_FAIL,
	DEFMSG_ITEMUSE_ADVGATE_NO,
	DEFMSG_ITEMUSE_ARCHB_BLOCK,
	DEFMSG_ITEMUSE_ARCHB_DEST,
	DEFMSG_ITEMUSE_ARCHB_EMPTY,
	DEFMSG_ITEMUSE_ARCHB_HIT_1,
	DEFMSG_ITEMUSE_ARCHB_HIT_2,
	DEFMSG_ITEMUSE_ARCHB_HIT_3,
	DEFMSG_ITEMUSE_ARCHB_HIT_4,
	DEFMSG_ITEMUSE_ARCHB_MISS_1,
	DEFMSG_ITEMUSE_ARCHB_MISS_2,
	DEFMSG_ITEMUSE_ARCHB_MISS_3,
	DEFMSG_ITEMUSE_ARCHB_MISS_4,
	DEFMSG_ITEMUSE_ARCHB_NOAMMO,
	DEFMSG_ITEMUSE_ARCHB_P,
	DEFMSG_ITEMUSE_ARCHB_REM,
	DEFMSG_ITEMUSE_ARCHB_REMS,
	DEFMSG_ITEMUSE_ARCHB_SKILL,
	DEFMSG_ITEMUSE_ARCHB_SPLIT,
	DEFMSG_ITEMUSE_ARCHB_WS,
	DEFMSG_ITEMUSE_ARCHB_X,
	DEFMSG_ITEMUSE_BANDAGE_CLEAN,
	DEFMSG_ITEMUSE_BANDAGE_PROMT,
	DEFMSG_ITEMUSE_BANDAGE_REACH,
	DEFMSG_ITEMUSE_BBOARD_COR,
	DEFMSG_ITEMUSE_BBOARD_DEL,
	DEFMSG_ITEMUSE_BBOARD_REACH,
	DEFMSG_ITEMUSE_BEDROLL,
	DEFMSG_ITEMUSE_BEEHIVE,
	DEFMSG_ITEMUSE_BEEHIVE_STING,
	DEFMSG_ITEMUSE_BOLT_1,
	DEFMSG_ITEMUSE_BOLT_2,
	DEFMSG_ITEMUSE_BOLT_3,
	DEFMSG_ITEMUSE_BOLT_4,
	DEFMSG_ITEMUSE_BOLT_5,
	DEFMSG_ITEMUSE_BOOK_FAIL,
	DEFMSG_ITEMUSE_BOW_SHIELD,
	DEFMSG_ITEMUSE_CANNON_EMPTY,
	DEFMSG_ITEMUSE_CANNON_HPOWDER,
	DEFMSG_ITEMUSE_CANNON_HSHOT,
	DEFMSG_ITEMUSE_CANNON_LPOWDER,
	DEFMSG_ITEMUSE_CANNON_LSHOT,
	DEFMSG_ITEMUSE_CANNON_POWDER,
	DEFMSG_ITEMUSE_CANNON_SHOT,
	DEFMSG_ITEMUSE_CANNON_TARG,
	DEFMSG_ITEMUSE_CANTTHINK,
	DEFMSG_ITEMUSE_CBALL_PROMT,
	DEFMSG_ITEMUSE_COTTON_CREATE,
	DEFMSG_ITEMUSE_CROOK_PROMT,
	DEFMSG_ITEMUSE_CROOK_TRY,
	DEFMSG_ITEMUSE_DYE_FAIL,
	DEFMSG_ITEMUSE_DYE_NOHAIR,
	DEFMSG_ITEMUSE_DYE_REACH,
	DEFMSG_ITEMUSE_DYE_TARG,
	DEFMSG_ITEMUSE_DYE_UNABLE,
	DEFMSG_ITEMUSE_DYE_VAT,
	DEFMSG_ITEMUSE_FISH_FAIL,
	DEFMSG_ITEMUSE_FISH_UNABLE,
	DEFMSG_ITEMUSE_FOODRAW_PROMT,
	DEFMSG_ITEMUSE_FOODRAW_TOUCH,
	DEFMSG_ITEMUSE_FOODRAW_USE,
	DEFMSG_ITEMUSE_FORGE,
	DEFMSG_ITEMUSE_GAMEBOARD_FAIL,
	DEFMSG_ITEMUSE_GUILDSTONE_NEW,
	DEFMSG_ITEMUSE_HATCH_FAIL,
	DEFMSG_ITEMUSE_JUNK_REACH,
	DEFMSG_ITEMUSE_KEY_FAIL,
	DEFMSG_ITEMUSE_KEY_NOKEY,
	DEFMSG_ITEMUSE_KEY_NOLOCK,
	DEFMSG_ITEMUSE_KEY_PROMT,
	DEFMSG_ITEMUSE_KINDLING_CONT,
	DEFMSG_ITEMUSE_KINDLING_FAIL,
	DEFMSG_ITEMUSE_LOCKED,
	DEFMSG_ITEMUSE_LOG_UNABLE,
	DEFMSG_ITEMUSE_LOG_USE,
	DEFMSG_ITEMUSE_LOOM,
	DEFMSG_ITEMUSE_LOOM_REMOVE,
	DEFMSG_ITEMUSE_MACEPICK_TARG,
	DEFMSG_ITEMUSE_MAP_FAIL,
	DEFMSG_ITEMUSE_MORTAR_PROMT,
	DEFMSG_ITEMUSE_MULTI_BLOCKED,
	DEFMSG_ITEMUSE_MULTI_BUMP,
	DEFMSG_ITEMUSE_MULTI_COLLAPSE,
	DEFMSG_ITEMUSE_MULTI_FAIL,
	DEFMSG_ITEMUSE_MULTI_INTWAY,
	DEFMSG_ITEMUSE_MULTI_SHIPW,
	DEFMSG_ITEMUSE_NO_MORTAR,
	DEFMSG_ITEMUSE_PDUMMY_OK,
	DEFMSG_ITEMUSE_PDUMMY_P,
	DEFMSG_ITEMUSE_PDUMMY_SKILL,
	DEFMSG_ITEMUSE_PITCHER_FILL,
	DEFMSG_ITEMUSE_PITCHER_REACH,
	DEFMSG_ITEMUSE_PITCHER_TARG,
	DEFMSG_ITEMUSE_PORT_LOCKED,
	DEFMSG_ITEMUSE_POTION_FAIL,
	DEFMSG_ITEMUSE_RUNE_NAME,
	DEFMSG_ITEMUSE_SCISSORS_USE,
	DEFMSG_ITEMUSE_SEWKIT_PROMT,
	DEFMSG_ITEMUSE_SEXTANT,
	DEFMSG_ITEMUSE_SEXTANT_T2A,
	DEFMSG_ITEMUSE_SHIPSIDE,
	DEFMSG_ITEMUSE_SHRINE,
	DEFMSG_ITEMUSE_SKIT_UNABLE,
	DEFMSG_ITEMUSE_SPAWNCHAR_NEG,
	DEFMSG_ITEMUSE_SPAWNCHAR_RSET,
	DEFMSG_ITEMUSE_SPAWNITEM_TRIG,
	DEFMSG_ITEMUSE_SPINWHEEL,
	DEFMSG_ITEMUSE_SPYGLASS_FE,
	DEFMSG_ITEMUSE_SPYGLASS_M1,
	DEFMSG_ITEMUSE_SPYGLASS_M2,
	DEFMSG_ITEMUSE_SPYGLASS_M3,
	DEFMSG_ITEMUSE_SPYGLASS_M4,
	DEFMSG_ITEMUSE_SPYGLASS_M5,
	DEFMSG_ITEMUSE_SPYGLASS_M6,
	DEFMSG_ITEMUSE_SPYGLASS_M7,
	DEFMSG_ITEMUSE_SPYGLASS_M8,
	DEFMSG_ITEMUSE_SPYGLASS_TR,
	DEFMSG_ITEMUSE_STEAL,
	DEFMSG_ITEMUSE_SWEB_STUCK,
	DEFMSG_ITEMUSE_TDUMMY_ARCH,
	DEFMSG_ITEMUSE_TDUMMY_P,
	DEFMSG_ITEMUSE_TDUMMY_SKILL,
	DEFMSG_ITEMUSE_TELESCOPE,
	DEFMSG_ITEMUSE_TILLERMAN,
	DEFMSG_ITEMUSE_TOOFAR,
	DEFMSG_ITEMUSE_TRACKER_ATTUNE,
	DEFMSG_ITEMUSE_TRASHCAN,
	DEFMSG_ITEMUSE_UNABLE,
	DEFMSG_ITEMUSE_WEAPON_IMMUNE,
	DEFMSG_ITEMUSE_WEAPON_PROMT,
	DEFMSG_ITEMUSE_WEAPON_WWAIT,
	DEFMSG_ITEMUSE_WOOL_CREATE,
	DEFMSG_LOCKPICKING_NOPICK,
	DEFMSG_LOCKPICKING_PREACH,
	DEFMSG_LOCKPICKING_REACH,
	DEFMSG_LOCKPICKING_WITEM,
	DEFMSG_LOGIN_PLAYER,
	DEFMSG_LOGIN_PLAYERS,
	DEFMSG_LUMBERJACKING_1,
	DEFMSG_LUMBERJACKING_2,
	DEFMSG_LUMBERJACKING_3,
	DEFMSG_LUMBERJACKING_4,
	DEFMSG_LUMBERJACKING_5,
	DEFMSG_LUMBERJACKING_6,
	DEFMSG_LUMBERJACKING_LOS,
	DEFMSG_LUMBERJACKING_REACH,
	DEFMSG_MAGERY_1,
	DEFMSG_MAGERY_2,
	DEFMSG_MAGERY_3,
	DEFMSG_MAGERY_4,
	DEFMSG_MAKESUCCESS_1,
	DEFMSG_MAKESUCCESS_2,
	DEFMSG_MAKESUCCESS_3,
	DEFMSG_MAKESUCCESS_4,
	DEFMSG_MAKESUCCESS_5,
	DEFMSG_MAKESUCCESS_6,
	DEFMSG_MEDITATION_PEACE_1,
	DEFMSG_MEDITATION_PEACE_2,
	DEFMSG_MEDITATION_TRY,
	DEFMSG_MINING_1,
	DEFMSG_MINING_2,
	DEFMSG_MINING_3,
	DEFMSG_MINING_4,
	DEFMSG_MINING_CANTUSE,
	DEFMSG_MINING_CONSUMED,
	DEFMSG_MINING_FIRE,
	DEFMSG_MINING_FORGE,
	DEFMSG_MINING_INGOTS,
	DEFMSG_MINING_LOS,
	DEFMSG_MINING_NOT_ORE,
	DEFMSG_MINING_NOTHING,
	DEFMSG_MINING_REACH,
	DEFMSG_MINING_SKILL,
	DEFMSG_MINING_TOOL,
	DEFMSG_ACC_ALREADYU,
	DEFMSG_ACC_BADPASS,
	DEFMSG_ACC_BLOCK,
	DEFMSG_ACC_BLOCKED,
	DEFMSG_ACC_CHAR_DEL,
	DEFMSG_ACC_DEL,
	DEFMSG_ACC_DENIED,
	DEFMSG_ACC_EMAIL_FAIL,
	DEFMSG_ACC_EMAIL_SUCCESS,
	DEFMSG_ACC_GUSED,
	DEFMSG_ACC_KICK,
	DEFMSG_ACC_NEEDPASS,
	DEFMSG_ACC_PRIV,
	DEFMSG_ACC_UNK,
	DEFMSG_ACC_WCLI,
	DEFMSG_ALREADYONLINE,
	DEFMSG_ARRDEP_1,
	DEFMSG_ARRDEP_2,
	DEFMSG_ARRDEP_3,
	DEFMSG_BOUNCE_PACK,
	DEFMSG_CANTPUSH,
	DEFMSG_CANTSLEEP,
	DEFMSG_CMD_LACKPRIV,
	DEFMSG_CORPSE_OF,
	DEFMSG_COWARD_1,
	DEFMSG_COWARD_2,
	DEFMSG_CRIMINAL,
	DEFMSG_EATSOME,
	DEFMSG_EMAIL_FAILED,
	DEFMSG_EMAIL_NOTSET_1,
	DEFMSG_EMAIL_NOTSET_2,
	DEFMSG_EMAIL_NOTVERIFIED,
	DEFMSG_EMOTE_1,
	DEFMSG_EMOTE_2,
	DEFMSG_EMOTE_3,
	DEFMSG_EMOTE_4,
	DEFMSG_EMOTE_5,
	DEFMSG_EMOTE_6,
	DEFMSG_EMOTE_7,
	DEFMSG_ERR_INVSET,
	DEFMSG_ERR_NOBLANKRING,
	DEFMSG_ERR_NOT4SALE,
	DEFMSG_ERR_NOTGAMEPIECE,
	DEFMSG_ERR_NOTKEY,
	DEFMSG_FATIGUE,
	DEFMSG_FEET,
	DEFMSG_FIGURINE_NOTYOURS,
	DEFMSG_FOLLOW_ARROW,
	DEFMSG_FOOD_LVL_1,
	DEFMSG_FOOD_LVL_2,
	DEFMSG_FOOD_LVL_3,
	DEFMSG_FOOD_LVL_4,
	DEFMSG_FOOD_LVL_5,
	DEFMSG_FOOD_LVL_6,
	DEFMSG_FOOD_LVL_7,
	DEFMSG_FOOD_LVL_8,
	DEFMSG_FORGIVEN,
	DEFMSG_FROZEN,
	DEFMSG_GMPAGE_CANCELED,
	DEFMSG_GMPAGE_NOTIFIED,
	DEFMSG_GMPAGE_QNUM,
	DEFMSG_GMPAGE_QUED,
	DEFMSG_GMPAGE_REC,
	DEFMSG_GMPAGE_UPDATE,
	DEFMSG_GMPAGES,
	DEFMSG_GUARDS,
	DEFMSG_GUEST,
	DEFMSG_GUILDRESIGN,
	DEFMSG_HEAVY,
	DEFMSG_HUNGER,
	DEFMSG_INVISIBLE,
	DEFMSG_IT_IS_DEAD,
	DEFMSG_ITEMPLACE,
	DEFMSG_JAILED,
	DEFMSG_KEY_BLANKS,
	DEFMSG_KEY_CANTREACH,
	DEFMSG_KEY_DOORLOCKED,
	DEFMSG_KEY_FAILC,
	DEFMSG_KEY_ISBLANK,
	DEFMSG_KEY_NOLOCK,
	DEFMSG_KEY_NOTBLANKS,
	DEFMSG_KEY_SETNAME,
	DEFMSG_KEY_TARG,
	DEFMSG_KEY_TARG_CONT_LOCK,
	DEFMSG_KEY_TARG_CONT_ULOCK,
	DEFMSG_KEY_TARG_DOOR_LOCK,
	DEFMSG_KEY_TARG_DOOR_ULOCK,
	DEFMSG_KEY_TARG_HOLD_LOCK,
	DEFMSG_KEY_TARG_HOLD_ULOCK,
	DEFMSG_KEY_TARG_NOLOCK,
	DEFMSG_KEY_TARG_REACH,
	DEFMSG_KEY_TARG_SHIP_LOCK,
	DEFMSG_KEY_TARG_SHIP_ULOCK ,
	DEFMSG_KEY_TARG_SIGN,
	DEFMSG_KEY_WRONGLOCK,
	DEFMSG_KILLED_BY,
	DEFMSG_MAGIC_BLOCK,
	DEFMSG_MAILBAG_DROP_1,
	DEFMSG_MAILBAG_DROP_2,
	DEFMSG_MAXCHARS,
	DEFMSG_MOUNT_CEILING,
	DEFMSG_MOUNT_DIST,
	DEFMSG_MOUNT_DONTOWN,
	DEFMSG_MOUNT_UNABLE,
	DEFMSG_MURDERER,
	DEFMSG_NOSPELLBOOK,
	DEFMSG_NOTO_CHANGE_0,
	DEFMSG_NOTO_CHANGE_1,
	DEFMSG_NOTO_CHANGE_2,
	DEFMSG_NOTO_CHANGE_3,
	DEFMSG_NOTO_CHANGE_4,
	DEFMSG_NOTO_CHANGE_5,
	DEFMSG_NOTO_CHANGE_6,
	DEFMSG_NOTO_CHANGE_7,
	DEFMSG_NOTO_CHANGE_8,
	DEFMSG_NOTO_CHANGE_GAIN,
	DEFMSG_NOTO_CHANGE_LOST,
	DEFMSG_NOTO_GETTITLE,
	DEFMSG_OVERLOAD,
	DEFMSG_PET_FOOD_1,
	DEFMSG_PET_FOOD_2,
	DEFMSG_PET_FOOD_3,
	DEFMSG_PET_FOOD_4,
	DEFMSG_PET_FOOD_5,
	DEFMSG_PET_FOOD_6,
	DEFMSG_PET_FOOD_7,
	DEFMSG_PET_FOOD_8,
	DEFMSG_PET_HAPPY_1,
	DEFMSG_PET_HAPPY_2,
	DEFMSG_PET_HAPPY_3,
	DEFMSG_PET_HAPPY_4,
	DEFMSG_PET_HAPPY_5,
	DEFMSG_PET_HAPPY_6,
	DEFMSG_PET_HAPPY_7,
	DEFMSG_PET_HAPPY_8,
	DEFMSG_PUSH,
	DEFMSG_REGION_ENTER,
	DEFMSG_REGION_GUARD_ART,
	DEFMSG_REGION_GUARDS_1,
	DEFMSG_REGION_GUARDS_2,
	DEFMSG_REGION_GUARDSP,
	DEFMSG_REGION_GUARDSPT,
	DEFMSG_REGION_PVPNOT,
	DEFMSG_REGION_PVPSAFE,
	DEFMSG_REGION_SAFETYGET,
	DEFMSG_REGION_SAFETYLOSE,
	DEFMSG_REGION_WATER_1,
	DEFMSG_REGION_WATER_2,
	DEFMSG_RENAME_CANCEL,
	DEFMSG_RENAME_SUCCESS,
	DEFMSG_RENAME_WNAME,
	DEFMSG_SEED_ATREE,
	DEFMSG_SEED_NOGOOD,
	DEFMSG_SEED_REACH,
	DEFMSG_SEED_TARGSOIL,
	DEFMSG_SERV_AO,
	DEFMSG_SERV_FULL,
	DEFMSG_SERV_LD,
	DEFMSG_SHIPNAME_PROMT,
	DEFMSG_STEAL,
	DEFMSG_STEPON_BODY,
	DEFMSG_STONEREG_TIME,
	DEFMSG_TARG_LINK,
	DEFMSG_TARG_LINK_SAME,
	DEFMSG_TARG_LINK2,
	DEFMSG_TARG_LINKS,
	DEFMSG_TARG_PC,
	DEFMSG_TINGLING,
	DEFMSG_TOOFAR_VENDOR,
	DEFMSG_TRADE_TOOFAR,
	DEFMSG_TRADE_WAIT,
	DEFMSG_UNCONSCIOUS,
	DEFMSG_WRESTLING_NOGO,
	DEFMSG_YOUNOTICE_1,
	DEFMSG_YOUNOTICE_2,
	DEFMSG_YOUNOTICE_S,
	DEFMSG_YOUNOTICE_YOUR,
	DEFMSG_MULTI_LOCKDOWN,
	DEFMSG_MULTI_LOCKUP,
	DEFMSG_MUSICANSHIP_NOTOOL,
	DEFMSG_MUSICANSHIP_POOR,
	DEFMSG_NCP_VENDOR_GUARDS,
	DEFMSG_NCP_VENDOR_SELL_TY,
	DEFMSG_NON_ALIVE,
	DEFMSG_NPC_BANKER_DEPOSIT,
	DEFMSG_NPC_BEGGAR_BEG_1,
	DEFMSG_NPC_BEGGAR_BEG_2,
	DEFMSG_NPC_BEGGAR_BEG_3,
	DEFMSG_NPC_BEGGAR_BEG_4,
	DEFMSG_NPC_BEGGAR_BEG_5,
	DEFMSG_NPC_BEGGAR_BEG_6,
	DEFMSG_NPC_BEGGAR_FOOD_TAL,
	DEFMSG_NPC_BEGGAR_FOOD_TY,
	DEFMSG_NPC_BEGGAR_IFONLY,
	DEFMSG_NPC_BEGGAR_SELL,
	DEFMSG_NPC_GENERIC_CRIM,
	DEFMSG_NPC_GENERIC_DONTWANT,
	DEFMSG_NPC_GENERIC_GONE_1,
	DEFMSG_NPC_GENERIC_GONE_2,
	DEFMSG_NPC_GENERIC_INTERRUPT,
	DEFMSG_NPC_GENERIC_SEECRIM,
	DEFMSG_NPC_GENERIC_SEEMONS,
	DEFMSG_NPC_GENERIC_SNOOPED_1,
	DEFMSG_NPC_GENERIC_SNOOPED_2,
	DEFMSG_NPC_GENERIC_SNOOPED_3,
	DEFMSG_NPC_GENERIC_SNOOPED_4,
	DEFMSG_NPC_GENERIC_THANKS,
	DEFMSG_NPC_GUARD_STRIKE_1,
	DEFMSG_NPC_GUARD_STRIKE_2,
	DEFMSG_NPC_GUARD_STRIKE_3,
	DEFMSG_NPC_GUARD_STRIKE_4,
	DEFMSG_NPC_GUARD_STRIKE_5,
	DEFMSG_NPC_GUARD_THREAT_1,
	DEFMSG_NPC_GUARD_THREAT_2,
	DEFMSG_NPC_GUARD_THREAT_3,
	DEFMSG_NPC_GUARD_THREAT_4,
	DEFMSG_NPC_GUARD_THREAT_5,
	DEFMSG_NPC_HEALER_FAIL_1,
	DEFMSG_NPC_HEALER_FAIL_2,
	DEFMSG_NPC_HEALER_MANIFEST,
	DEFMSG_NPC_HEALER_RANGE,
	DEFMSG_NPC_HEALER_REF_CRIM_1,
	DEFMSG_NPC_HEALER_REF_CRIM_2,
	DEFMSG_NPC_HEALER_REF_CRIM_3,
	DEFMSG_NPC_HEALER_REF_EVIL_1,
	DEFMSG_NPC_HEALER_REF_EVIL_2,
	DEFMSG_NPC_HEALER_REF_EVIL_3,
	DEFMSG_NPC_HEALER_REF_GOOD_1,
	DEFMSG_NPC_HEALER_REF_GOOD_2,
	DEFMSG_NPC_HEALER_REF_GOOD_3,
	DEFMSG_NPC_HEALER_RES_1,
	DEFMSG_NPC_HEALER_RES_2,
	DEFMSG_NPC_HEALER_RES_3,
	DEFMSG_NPC_HEALER_RES_4,
	DEFMSG_NPC_HEALER_RES_5,
	DEFMSG_NPC_PET_CANTSELL,
	DEFMSG_NPC_PET_CARRYNOTHING,
	DEFMSG_NPC_PET_DAYS_LEFT,
	DEFMSG_NPC_PET_DECIDE_MASTER,
	DEFMSG_NPC_PET_DESERTED,
	DEFMSG_NPC_PET_DROP,
	DEFMSG_NPC_PET_EMPLOYED,
	DEFMSG_NPC_PET_FAILURE,
	DEFMSG_NPC_PET_FOOD_NO,
	DEFMSG_NPC_PET_FOOD_TY,
	DEFMSG_NPC_PET_GETGOLD_1,
	DEFMSG_NPC_PET_GETGOLD_2,
	DEFMSG_NPC_PET_HIRE_AMNT,
	DEFMSG_NPC_PET_HIRE_RATE,
	DEFMSG_NPC_PET_HIRE_TIME,
	DEFMSG_NPC_PET_HIRE_TIMEUP,
	DEFMSG_NPC_PET_INV_ONLY,
	DEFMSG_NPC_PET_ITEMS_BUY,
	DEFMSG_NPC_PET_ITEMS_SAMPLE,
	DEFMSG_NPC_PET_ITEMS_SELL,
	DEFMSG_NPC_PET_MONEY,
	DEFMSG_NPC_PET_NOT_ENOUGH,
	DEFMSG_NPC_PET_NOT_FOR_HIRE,
	DEFMSG_NPC_PET_NOT_WORK,
	DEFMSG_NPC_PET_SELL,
	DEFMSG_NPC_PET_SETPRICE,
	DEFMSG_NPC_PET_SUCCESS,
	DEFMSG_NPC_PET_TARG_ATT,
	DEFMSG_NPC_PET_TARG_FETCH,
	DEFMSG_NPC_PET_TARG_FOLLOW,
	DEFMSG_NPC_PET_TARG_FRIEND,
	DEFMSG_NPC_PET_TARG_GO,
	DEFMSG_NPC_PET_TARG_GUARD,
	DEFMSG_NPC_PET_TARG_TRANSFER,
	DEFMSG_NPC_PET_THANKS,
	DEFMSG_NPC_PET_WAGE_COST,
	DEFMSG_NPC_PET_WEAK,
	DEFMSG_NPC_STABLEMASTER_FAIL,
	DEFMSG_NPC_STABLEMASTER_FULL,
	DEFMSG_NPC_STABLEMASTER_LOS,
	DEFMSG_NPC_STABLEMASTER_NOPETS,
	DEFMSG_NPC_STABLEMASTER_REM,
	DEFMSG_NPC_STABLEMASTER_SELECT,
	DEFMSG_NPC_STABLEMASTER_TARG_STABLE,
	DEFMSG_NPC_STABLEMASTER_TOOMANY ,
	DEFMSG_NPC_STABLEMASTER_TREATWELL,
	DEFMSG_NPC_TEXT_MURD_1,
	DEFMSG_NPC_TEXT_MURD_2,
	DEFMSG_NPC_TEXT_MURD_3,
	DEFMSG_NPC_TEXT_MURD_4,
	DEFMSG_NPC_TRAINER_DUNNO_1,
	DEFMSG_NPC_TRAINER_DUNNO_2,
	DEFMSG_NPC_TRAINER_DUNNO_3,
	DEFMSG_NPC_TRAINER_DUNNO_4,
	DEFMSG_NPC_TRAINER_ENEMY,
	DEFMSG_NPC_TRAINER_FORGOT,
	DEFMSG_NPC_TRAINER_PRICE,
	DEFMSG_NPC_TRAINER_PRICE_1,
	DEFMSG_NPC_TRAINER_PRICE_2,
	DEFMSG_NPC_TRAINER_PRICE_3,
	DEFMSG_NPC_TRAINER_SUCCESS,
	DEFMSG_NPC_TRAINER_THATSALL_1,
	DEFMSG_NPC_TRAINER_THATSALL_2,
	DEFMSG_NPC_TRAINER_THATSALL_3,
	DEFMSG_NPC_TRAINER_THATSALL_4,
	DEFMSG_NPC_VENDOR_B1,
	DEFMSG_NPC_VENDOR_CA,
	DEFMSG_NPC_VENDOR_CANTAFFORD,
	DEFMSG_NPC_VENDOR_HYARE,
	DEFMSG_NPC_VENDOR_NO_GOODS,
	DEFMSG_NPC_VENDOR_NOMONEY,
	DEFMSG_NPC_VENDOR_NOTHING_BUY,
	DEFMSG_NPC_VENDOR_NOTHING_SELL,
	DEFMSG_NPC_VENDOR_OFFDUTY,
	DEFMSG_NPC_VENDOR_S1,
	DEFMSG_NPC_VENDOR_SELL,
	DEFMSG_NPC_VENDOR_SETPRICE_1,
	DEFMSG_NPC_VENDOR_SETPRICE_2,
	DEFMSG_NPC_VENDOR_STAT_GOLD_1,
	DEFMSG_NPC_VENDOR_STAT_GOLD_2,
	DEFMSG_NPC_VENDOR_STAT_GOLD_3,
	DEFMSG_NPC_VENDOR_STAT_GOLD_4,
	DEFMSG_NPC_VENDOR_TY,
	DEFMSG_PARTY_ADDED,
	DEFMSG_PARTY_DISBANDED,
	DEFMSG_PARTY_INVITE,
	DEFMSG_PARTY_INVITE_TARG,
	DEFMSG_PARTY_JOINED,
	DEFMSG_PARTY_LEAVE_1,
	DEFMSG_PARTY_LEAVE_2,
	DEFMSG_PARTY_LOOT_ALLOW,
	DEFMSG_PARTY_LOOT_BLOCK,
	DEFMSG_PARTY_PART_1,
	DEFMSG_PARTY_PART_2,
	DEFMSG_PARTY_SELECT,
	DEFMSG_PARTY_TARG_WHO,
	DEFMSG_POISONING_SELECT_1,
	DEFMSG_POISONING_SUCCESS,
	DEFMSG_POISONING_WITEM,
	DEFMSG_PROVOCATION_EMOTE_1,
	DEFMSG_PROVOCATION_EMOTE_2,
	DEFMSG_PROVOCATION_PLAYER,
	DEFMSG_PROVOCATION_UPSET,
	DEFMSG_PROVOKE_SELECT,
	DEFMSG_PROVOKE_UNABLE,
	DEFMSG_REACH_FAIL,
	DEFMSG_REACH_GHOST,
	DEFMSG_REACH_UNABLE,
	DEFMSG_REMOVETRAPS_REACH,
	DEFMSG_REMOVETRAPS_WITEM,
	DEFMSG_REPAIR_1,
	DEFMSG_REPAIR_2,
	DEFMSG_REPAIR_3,
	DEFMSG_REPAIR_4,
	DEFMSG_REPAIR_5,
	DEFMSG_REPAIR_ANVIL,
	DEFMSG_REPAIR_FULL,
	DEFMSG_REPAIR_LACK_1,
	DEFMSG_REPAIR_LACK_2,
	DEFMSG_REPAIR_MSG,
	DEFMSG_REPAIR_NOT,
	DEFMSG_REPAIR_REACH,
	DEFMSG_REPAIR_UNK,
	DEFMSG_REPAIR_WORN,
	DEFMSG_SERVER_RESYNC_FAILED,
	DEFMSG_SERVER_RESYNC_START,
	DEFMSG_SERVER_RESYNC_SUCCESS,
	DEFMSG_SERVER_WORLDSAVE,
	DEFMSG_SKILL_NOSKILL,
	DEFMSG_SKILLWAIT_1,
	DEFMSG_SKILLWAIT_2,
	DEFMSG_SKILLWAIT_3,
	DEFMSG_SLEEP_AWAKE_1,
	DEFMSG_SLEEP_AWAKE_2,
	DEFMSG_SMITHING_FAIL,
	DEFMSG_SMITHING_FORGE,
	DEFMSG_SMITHING_HAMMER,
	DEFMSG_SMITHING_REACH,
	DEFMSG_SNOOPING_MARK,
	DEFMSG_SNOOPING_REACH,
	DEFMSG_SNOOPING_SOMEONE,
	DEFMSG_SNOOPING_YOUR,
	DEFMSG_SPELL_ALCOHOL_HIC,
	DEFMSG_SPELL_ANIMDEAD_FAIL,
	DEFMSG_SPELL_ANIMDEAD_NC,
	DEFMSG_SPELL_CURSED_MAGIC,
	DEFMSG_SPELL_DISPELLF_WT,
	DEFMSG_SPELL_ENCHANT_ACTIVATE,
	DEFMSG_SPELL_ENCHANT_LACK,
	DEFMSG_SPELL_GATE_AM,
	DEFMSG_SPELL_GEN_FIZZLES,
	DEFMSG_SPELL_LOOKS,
	DEFMSG_SPELL_POISON_1,
	DEFMSG_SPELL_POISON_2,
	DEFMSG_SPELL_POISON_3,
	DEFMSG_SPELL_POISON_4,
	DEFMSG_SPELL_RECALL_BLANK,
	DEFMSG_SPELL_RECALL_FADEC,
	DEFMSG_SPELL_RECALL_FADED,
	DEFMSG_SPELL_RECALL_NOTRUNE,
	DEFMSG_SPELL_RECALL_SFADE,
	DEFMSG_SPELL_RES_AM,
	DEFMSG_SPELL_RES_REJOIN,
	DEFMSG_SPELL_RES_ROBENAME,
	DEFMSG_SPELL_TARG_CONT,
	DEFMSG_SPELL_TARG_FIELDC,
	DEFMSG_SPELL_TARG_LOS,
	DEFMSG_SPELL_TARG_NOEFFECT,
	DEFMSG_SPELL_TARG_OBJ,
	DEFMSG_SPELL_TELE_AM,
	DEFMSG_SPELL_TELE_CANT,
	DEFMSG_SPELL_TELE_JAILED_1,
	DEFMSG_SPELL_TELE_JAILED_2,
	DEFMSG_SPELL_TRY_AM,
	DEFMSG_SPELL_TRY_DEAD,
	DEFMSG_SPELL_TRY_NOBOOK,
	DEFMSG_SPELL_TRY_NOMANA,
	DEFMSG_SPELL_TRY_NOREGS,
	DEFMSG_SPELL_TRY_NOTYOURBOOK,
	DEFMSG_SPELL_TRY_THEREG,
	DEFMSG_SPELL_WAND_NOCHARGE,
	DEFMSG_SPELL_YOUFEEL,
	DEFMSG_SPIRITSPEAK_SUCCESS,
	DEFMSG_STEALING_CORPSE,
	DEFMSG_STEALING_EMPTY,
	DEFMSG_STEALING_GAMEBOARD,
	DEFMSG_STEALING_HEAVY,
	DEFMSG_STEALING_MARK,
	DEFMSG_STEALING_NONEED,
	DEFMSG_STEALING_NOTHING,
	DEFMSG_STEALING_PICKPOCKET,
	DEFMSG_STEALING_REACH,
	DEFMSG_STEALING_SAFE,
	DEFMSG_STEALING_SOMEONE,
	DEFMSG_STEALING_STOP,
	DEFMSG_STEALING_TRADE,
	DEFMSG_STEALING_YOUR,
	DEFMSG_TAMING_1,
	DEFMSG_TAMING_2,
	DEFMSG_TAMING_3,
	DEFMSG_TAMING_4,
	DEFMSG_TAMING_CANT,
	DEFMSG_TAMING_LOS,
	DEFMSG_TAMING_REACH,
	DEFMSG_TAMING_REMEMBER,
	DEFMSG_TAMING_SUCCESS,
	DEFMSG_TAMING_TAME,
	DEFMSG_TAMING_TAMED,
	DEFMSG_TAMING_YMASTER,
	DEFMSG_TARGET_PROMT,
	DEFMSG_TASTEID_CHAR,
	DEFMSG_TASTEID_RESULT,
	DEFMSG_TASTEID_SELF,
	DEFMSG_TASTEID_UNABLE,
	DEFMSG_TRACKING_DIST_0,
	DEFMSG_TRACKING_DIST_1,
	DEFMSG_TRACKING_DIST_2,
	DEFMSG_TRACKING_DIST_3,
	DEFMSG_TRACKING_DIST_4,
	DEFMSG_TRACKING_FAIL_1,
	DEFMSG_TRACKING_FAIL_2,
	DEFMSG_TRACKING_FAIL_3,
	DEFMSG_TRACKING_FAIL_4,
	DEFMSG_TRACKING_FAIL_5,
	DEFMSG_TRACKING_FAIL_6,
	DEFMSG_TRACKING_SPYGLASS,
	DEFMSG_TRACKING_UNABLE,
	// additional messages will be added here in unsorted fashion
	DEFMSG_QTY,
};

extern class CExpression
{
public:
	CVarDefArray	m_VarDefs;		// Defined variables in sorted order.
	CVarDefArray	m_VarGlobals;	// Global variables
	CGString		m_sTmp;

								//	defined default messages
	static TCHAR sm_szMessages[DEFMSG_QTY+1][256];			// like: "You put %s to %s"
	static LPCTSTR const sm_szMsgNames[DEFMSG_QTY+1][32];	// like: "put_it"

	CVarDefArray	m_DefMessages;	//	DEFMESSAGES (old part of them not yet moved to faster - sm_sz*)

public:
	// Strict G++ Prototyping produces an error when not casting char*& to const char*&
	// So this is a rather lazy workaround
	inline int GetSingle( LPSTR &pArgs )
	{
		return GetSingle( (LPCTSTR&)pArgs );
	}

	inline int GetRange( LPSTR &pArgs )
	{
		return GetRange( (LPCTSTR&)pArgs );
	}

	inline int GetRangeVals( LPSTR &pExpr, int * piVals, int iMaxQty )
	{
		return GetRangeVals( (LPCTSTR&)pExpr, piVals, iMaxQty );
	}

	inline int GetVal( LPSTR &pArgs )
	{
		return GetVal( (LPCTSTR&)pArgs );
	}

	// Evaluate using the stuff we know.
	int GetSingle( LPCTSTR & pArgs );
	int GetVal( LPCTSTR & pArgs );
	int GetValMath( int lVal, LPCTSTR & pExpr );
	int GetRangeVals( LPCTSTR & pExpr, int * piVals, int iMaxQty );
	int GetRange( LPCTSTR & pArgs );

	CExpression();
	~CExpression();

} g_Exp;

extern bool IsSimpleNumberString( LPCTSTR pszTest );
extern bool IsStrNumericDec( LPCTSTR pszTest );
extern bool IsStrNumeric( LPCTSTR pszTest );
extern bool IsStrEmpty( LPCTSTR pszTest );

// Numeric formulas
extern int Calc_GetRandVal( int iqty );
extern int Calc_GetLog2( UINT iVal );
extern int Calc_GetSCurve( int iValDiff, int iVariance );
extern int Calc_GetBellCurve( int iValDiff, int iVariance );

extern DWORD ahextoi( LPCTSTR pArgs ); // Convert hex string to integer

#define Exp_GetSingle( pa ) g_Exp.GetSingle( pa )
#define Exp_GetVal( pa )	g_Exp.GetVal( pa )
#define Exp_GetRange( pa )	g_Exp.GetRange( pa )

inline int CVarDefStr::GetValNum() const
{
	LPCTSTR pszStr = m_sVal;
	return( Exp_GetVal(pszStr));
}

inline LPCTSTR CVarDefNum::GetValStr() const
{
	TCHAR * pszTmp = Str_GetTemp();
	sprintf( pszTmp, _TEXT("0%x"), m_iVal );
	return( pszTmp );
}

#endif	// _INC_CEXPRSSION_H