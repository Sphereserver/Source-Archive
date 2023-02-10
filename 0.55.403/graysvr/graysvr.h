//
// GraySvr.H
// Copyright Menace Software (www.menasoft.com).
// Precompiled header
//

#ifndef _INC_GRAYSVR_H_
#define _INC_GRAYSVR_H_
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef GRAY_SVR
#error GRAY_SVR must be -defined on compiler command line for common code to work!
#endif

// GRAY_VERSION
#define GRAY_TITLE			"Sphere"	// "Sphere"
#define EVO_TITLE			"ProfaniaServer"
#define GRAY_MAIN_SERVER	"list.sphereserver.com"
#define GRAY_GAME_SERVER
#define GRAY_LOG_SERVER
#define GRAY_URL			"www.profania.net"	// default url.

#ifdef _WIN32
// NOTE: If we want a max number of sockets we must compile for it !
#undef FD_SETSIZE		// This shuts off a warning
#define FD_SETSIZE 1024 // for max of n users ! default = 64
#endif

//	Build settings

//	Disable seems not used for anyone IRC client. Takes ~80Kb of extra EXE space
#define BUILD_WITHOUT_IRCSERVER
//	Enable memory profiling info via using overrided new/delete operators
//	Note: they were marked as possible unstable for unix by darkstorm
#ifdef _WIN98
	#define BUILD_WITHOUT_CUSTOMNEW
#endif
#ifndef _WIN32
	#define BUILD_WITHOUT_CUSTOMNEW
#endif
//	Enable advanced exceptions catching. Consumes some more resurces, but is very useful
//	for debug on a running environment. Also it makes sphere more stable since exceptionals
//	are local
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
	#define EXCEPTIONS_DEBUG
	// set this to get full call stack if applicable of the error, one throwing to the previous
	// helps finding out what happened, but makes server less error-less
	//#define EXCEPTIONS_USETHROW
#endif

class CServTime
{
#undef GetCurrentTime
#define TICK_PER_SEC 10
	// A time stamp in the server/game world.
public:
	long m_lPrivateTime;
public:
	long GetTimeRaw() const
	{
		return m_lPrivateTime;
	}
	int GetTimeDiff( const CServTime & time ) const
	{
		return( m_lPrivateTime - time.m_lPrivateTime );
	}
	void Init()
	{
		m_lPrivateTime = 0;
	}
	void InitTime( long lTimeBase )
	{
		m_lPrivateTime = lTimeBase;
	}
	bool IsTimeValid() const
	{
		return( m_lPrivateTime ? true : false );
	}
	CServTime operator+( int iTimeDiff ) const
	{
		CServTime time;
		time.m_lPrivateTime = m_lPrivateTime + iTimeDiff;
		return( time );
	}
	CServTime operator-( int iTimeDiff ) const
	{
		CServTime time;
		time.m_lPrivateTime = m_lPrivateTime - iTimeDiff;
		return( time );
	}
	int operator-( CServTime time ) const
	{
		return(m_lPrivateTime-time.m_lPrivateTime);
	}
	bool operator==(CServTime time) const
	{
		return(m_lPrivateTime==time.m_lPrivateTime);
	}
	bool operator!=(CServTime time) const
	{
		return(m_lPrivateTime!=time.m_lPrivateTime);
	}
	bool operator<(CServTime time) const
	{
		return(m_lPrivateTime<time.m_lPrivateTime);
	}
	bool operator>(CServTime time) const
	{
		return(m_lPrivateTime>time.m_lPrivateTime);
	}
	bool operator<=(CServTime time) const
	{
		return(m_lPrivateTime<=time.m_lPrivateTime);
	}
	bool operator>=(CServTime time) const
	{
		return(m_lPrivateTime>=time.m_lPrivateTime);
	}
	static CServTime GetCurrentTime();
};

// Please note that includes are case sensitive for Linux
#include "../common/graycom.h"
#include "../common/graymul.h"
#include "../common/grayproto.h"
#include "../common/CGrayInst.h"
#include "../common/CResourceBase.h"
#include "../common/CRegion.h"
#include "../common/CGrayMap.h"
#include "../common/CQueue.h"
#include "../common/CSectorTemplate.h"

#include "CResource.h"
#include "CServRef.h"
#include "CAccount.h"

class CClient;
class CAccount;
class CWebPageDef;
class CServerDef;
class CCharBase;
class CItemBase;
class CItemContainer;
class CItemMessage;
class CItemMap;
class CItemServerGate;

///////////////////////////////////////////////

// Text mashers.

extern DIR_TYPE GetDirStr( LPCTSTR pszDir );
extern LPCTSTR GetTimeMinDesc( int dwMinutes );
extern int FindStrWord( LPCTSTR pTextSearch, LPCTSTR pszKeyWord );

enum LOGEVENT_TYPE
{
	LOGEVENT_Startup,
	LOGEVENT_Shutdown,

	LOGEVENT_LoadBegin,
	LOGEVENT_LoadStatus,	// Arg1=int percent
	LOGEVENT_LoadDone,

	LOGEVENT_SaveBegin,
	LOGEVENT_SaveStatus,	// Arg1=int percent
	LOGEVENT_SaveDone,

	LOGEVENT_ServerMsg,		// Arg1=CComBSTR msg(pStr)
	LOGEVENT_GarbageStatus,	// Arg1=int percent

	LOGEVENT_ClientAttach,	// Arg1=int socket
	LOGEVENT_ClientLogin,	// Arg1=int socket
	LOGEVENT_ClientDetach,	// Arg1=int socket

	LOGEVENT_QTY,
};

extern struct CLog : public CFileText, public CEventLog, public CThreadLockableObj
{
	// subject matter. (severity level is first 4 bits, LOGL_EVENT)
#define LOGM_ACCOUNTS		0x00080
//#define LOGM_INIT			0x00100	// start up messages.
#define LOGM_SAVE			0x00200	// world save status.
#define LOGM_CLIENTS_LOG	0x00400	// all clients as they log in and out.
#define LOGM_GM_PAGE		0x00800	// player gm pages.
#define LOGM_PLAYER_SPEAK	0x01000	// All that the players say.
#define LOGM_GM_CMDS		0x02000	// Log all GM commands.
#define LOGM_CHEAT			0x04000	// Probably an exploit !
#define LOGM_KILLS			0x08000	// Log player combat results.
#define LOGM_HTTP			0x10000

private:
	DWORD m_dwMsgMask;			// Level of log detail messages. IsLogMsg()
	CGTime m_dateStamp;			// last real time stamp.
	CGString m_sBaseDir;

	const CScript * m_pScriptContext;	// The current context.
	const CScriptObj * m_pObjectContext;	// The current context.

	static CGTime sm_prevCatchTick;	// don't flood with these.
public:
	bool m_fLockOpen;

public:
	const CScript * SetScriptContext( const CScript * pScriptContext )
	{
		const CScript * pOldScript = m_pScriptContext;
		m_pScriptContext = pScriptContext;
		return( pOldScript );
	}
	const CScriptObj * SetObjectContext( const CScriptObj * pObjectContext )
	{
		const CScriptObj * pOldObject = m_pObjectContext;
		m_pObjectContext = pObjectContext;
		return( pOldObject );
	}
	bool SetFilePath( LPCTSTR pszName )
	{
		ASSERT( ! IsFileOpen());
		return CFileText::SetFilePath( pszName );
	}

	LPCTSTR GetLogDir() const
	{
		return( m_sBaseDir );
	}
	bool OpenLog( LPCTSTR pszName = NULL );	// name set previously.
	DWORD GetLogMask() const
	{
		return( m_dwMsgMask &~ 0x0f ) ;
	}
	void SetLogMask( DWORD dwMask )
	{
		m_dwMsgMask = GetLogLevel() | dwMask;
	}
	bool IsLoggedMask( DWORD dwMask ) const
	{
		return(( GetLogMask() & ( dwMask &~ 0x0f )) ? true : false );
	}
	LOGL_TYPE GetLogLevel() const
	{
		return((LOGL_TYPE)( m_dwMsgMask & 0x0f ));
	}
	void SetLogLevel( LOGL_TYPE level )
	{
		m_dwMsgMask = GetLogMask() | level;
	}
	bool IsLogged( DWORD wMask ) const
	{
		return( IsLoggedMask(wMask) ||
			( GetLogLevel() >= ( wMask & 0x0f )));
	}

	void Dump( const BYTE * pData, int len );

	virtual int EventStr( DWORD wMask, LPCTSTR pszMsg );
	void _cdecl CatchEvent( CGrayError * pErr, LPCTSTR pszCatchContext, ...  );
	void _cdecl FireEvent( LOGEVENT_TYPE type, ... );

	CLog()
	{
		m_fLockOpen = false;
		m_pScriptContext = NULL;
		m_dwMsgMask = LOGL_EVENT |
			LOGM_INIT | LOGM_CLIENTS_LOG | LOGM_GM_PAGE;
		SetFilePath( GRAY_FILE "log.log" );	// default name to go to.
	}

} g_Log;		// Log file

//////////////////

class CGMPage : public CGObListRec, public CScriptObj
{
	// RES_GMPAGE
	// ONly one page allowed per account at a time.
	static LPCTSTR const sm_szLoadKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CGString m_sAccount;	// The account that paged me.
	CClient * m_pGMClient;	// assigned to a GM
	CGString m_sReason;		// Players Description of reason for call.
public:
	// Queue a GM page. (based on account)
	CServTime  m_timePage;	// Time of the last call.
	CPointMap  m_ptOrigin;		// Origin Point of call.
public:
	CGMPage( LPCTSTR pszAccount );
	~CGMPage();
	CAccountRef FindAccount() const;
	LPCTSTR GetAccountStatus() const;
	LPCTSTR GetName() const
	{
		return( m_sAccount );
	}
	LPCTSTR GetReason() const
	{
		return( m_sReason );
	}
	void SetReason( LPCTSTR pszReason )
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
	int GetAge() const;

	bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	void r_Write( CScript & s ) const;
	bool r_LoadVal( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );

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
	friend class CChatChannel;
	friend class CChat;
	// friend CClient;
	bool GetWhoIs() const { return m_fAllowWhoIs; }
	void SetWhoIs(bool fAllowWhoIs) { m_fAllowWhoIs = fAllowWhoIs; }
	bool IsReceivingAllowed() const { return m_fReceiving; }
	LPCTSTR GetChatName() const;

	int FindIgnoringIndex( LPCTSTR pszName) const;

protected:
	// void SetChatName(LPCTSTR pszName) { m_sName = pszName; }
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
	void SendChatMsg( CHATMSG_TYPE iType, LPCTSTR pszName1 = NULL, LPCTSTR pszName2 = NULL, CLanguageID lang = 0 );
	void RenameChannel(LPCTSTR pszName);

	void Ignore(LPCTSTR pszName);
	void DontIgnore(LPCTSTR pszName);
	void ToggleIgnore(LPCTSTR pszName);
	void ClearIgnoreList();
	bool IsIgnoring(LPCTSTR pszName) const
	{
		return( FindIgnoringIndex( pszName ) >= 0 );
	}
};

class CChatChannel : public CGObListRec
{
	// a number of clients can be attached to this chat channel.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	friend class CChatChanMember;
	friend class CChat;
	CGString m_sName;
	CGString m_sPassword;
	bool m_fVoiceDefault;	// give others voice by default.
public:
	CGObArray< CGString * > m_NoVoices;// Current list of channel members with no voice
	CGObArray< CGString * > m_Moderators;// Current list of channel's moderators (may or may not be currently in the channel)
	CGPtrTypeArray< CChatChanMember* > m_Members;	// Current list of members in this channel
	//CGPtrTypeArray< CAccount* > m_BannedAccounts; // Current list of banned accounts in this channel
private:
	void SetModerator(LPCTSTR pszName, bool fFlag = true);
	void SetVoice(LPCTSTR pszName, bool fFlag = true);
	void RenameChannel(CChatChanMember * pBy, LPCTSTR pszName);
	int FindMemberIndex( LPCTSTR pszName ) const;

public:
	CChatChannel(LPCTSTR pszName, LPCTSTR pszPassword = NULL)
	{
		m_sName = pszName;
		m_sPassword = pszPassword;
		m_fVoiceDefault = true;
	};
	CChatChannel* GetNext() const
	{
		return( static_cast <CChatChannel *>( CGObListRec::GetNext()));
	}
	LPCTSTR GetName() const
	{
		return( m_sName );
	}
	LPCTSTR GetModeString() const
	{
		// (client needs this) "0" = not passworded, "1" = passworded
		return(( IsPassworded()) ? "1" : "0" );
	}

	LPCTSTR GetPassword() const
	{
		return( m_sPassword );
	}
	void SetPassword( LPCTSTR pszPassword)
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
	void ToggleVoiceDefault(LPCTSTR  pszBy);
	void DisableVoiceDefault(LPCTSTR  pszBy);
	void EnableVoiceDefault(LPCTSTR  pszBy);
	void Emote(LPCTSTR pszBy, LPCTSTR pszMsg, CLanguageID lang = 0 );
	void WhoIs(LPCTSTR pszBy, LPCTSTR pszMember);
	bool AddMember(CChatChanMember * pMember);
	void KickMember( CChatChanMember *pByMember, CChatChanMember * pMember );
	void Broadcast(CHATMSG_TYPE iType, LPCTSTR pszName, LPCTSTR pszText, CLanguageID lang = 0, bool fOverride = false);
	void SendThisMember(CChatChanMember * pMember, CChatChanMember * pToMember = NULL);
	void SendMembers(CChatChanMember * pMember);
	void RemoveMember(CChatChanMember * pMember);
	CChatChanMember * FindMember(LPCTSTR pszName) const
	{
		int i = FindMemberIndex( pszName );
		return(( i >= 0 ) ? m_Members[i] : NULL );
	}
	bool RemoveMember(LPCTSTR pszName)
	{
		int i = FindMemberIndex( pszName );
		if ( i >= 0 )
		{
			RemoveMember(m_Members[i]);
			return true;
		}
		return false;
	}
	void SetName(LPCTSTR pszName)
	{
		m_sName = pszName;
	}
	bool IsModerator(LPCTSTR pszName) const;
	bool HasVoice(LPCTSTR pszName) const;

	void MemberTalk(CChatChanMember * pByMember, LPCTSTR pszText, CLanguageID lang );
	void ChangePassword(CChatChanMember * pByMember, LPCTSTR pszPassword);
	void GrantVoice(CChatChanMember * pByMember, LPCTSTR pszName);
	void RevokeVoice(CChatChanMember * pByMember, LPCTSTR pszName);
	void ToggleVoice(CChatChanMember * pByMember, LPCTSTR pszName);
	void GrantModerator(CChatChanMember * pByMember, LPCTSTR pszName);
	void RevokeModerator(CChatChanMember * pByMember, LPCTSTR pszName);
	void ToggleModerator(CChatChanMember * pByMember, LPCTSTR pszName);
	void SendPrivateMessage(CChatChanMember * pFrom, LPCTSTR pszTo, LPCTSTR  pszMsg);
	void KickAll(CChatChanMember * pMember = NULL);
};

class CChat
{
	// All the chat channels.
private:
	bool m_fChatsOK;	// allowed to create new chats ?
	CGObList m_Channels;		// CChatChannel // List of chat channels.
private:
	void DoCommand(CChatChanMember * pBy, LPCTSTR szMsg);
	void DeleteChannel(CChatChannel * pChannel);
	void WhereIs(CChatChanMember * pBy, LPCTSTR pszName) const;
	void KillChannels();
	bool JoinChannel(CChatChanMember * pMember, LPCTSTR pszChannel, LPCTSTR pszPassword);
	bool CreateChannel(LPCTSTR pszName, LPCTSTR pszPassword, CChatChanMember * pMember);
	void CreateJoinChannel(CChatChanMember * pByMember, LPCTSTR pszName, LPCTSTR pszPassword);
	CChatChannel * FindChannel(LPCTSTR pszChannel) const
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

	void EventMsg( CClient * pClient, const NCHAR * pszText, int len, CLanguageID lang ); // Text from a client

	static bool IsValidName(LPCTSTR pszName, bool fPlayer);

	void SendDeleteChannel(CChatChannel * pChannel);
	void SendNewChannel(CChatChannel * pNewChannel);
	bool IsDuplicateChannelName(const char * pszName) const
	{
		return FindChannel(pszName) != NULL;
	}

	void Broadcast(CChatChanMember * pFrom, LPCTSTR pszText, CLanguageID lang = 0, bool fOverride = false);
	void QuitChat(CChatChanMember * pClient);
	static void DecorateName(CGString & sName, const CChatChanMember * pMember = NULL, bool fSystem = false);
};

struct CDialogResponseString
{
public:
	const WORD m_ID;
	CGString const m_sText;
public:
	CDialogResponseString( WORD id, LPCTSTR pszText ) :
		m_ID(id),
		m_sText(pszText)
	{
	}
};

class CDialogResponseArgs : public CScriptTriggerArgs
{
	// The scriptable return from a gump dialog.
	// "ARG" = dialog args script block. ex. ARGTXT(id), ARGCHK(i)
public:
	static LPCTSTR const sm_szLoadKeys[];
	CGTypedArray <DWORD,DWORD> m_CheckArray;
	CGObArray< CDialogResponseString *> m_TextArray;
public:
	void AddText( WORD id, LPCTSTR pszText )
	{
		m_TextArray.Add( new CDialogResponseString( id, pszText ));
	}
	LPCTSTR GetName() const
	{
		return( "ARGD" );
	}
	bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
};

struct CMenuItem 	// describe a menu item.
{
public:
	WORD m_id;			// ITEMID_TYPE in base set.
	CGString m_sText;
public:
	bool ParseLine( TCHAR * pszArgs, CScriptObj * pObjBase, CTextConsole * pSrc );
};

struct CSectorEnviron	// When these change it is an CTRIG_EnvironChange,
{
#define LIGHT_OVERRIDE 0x80
public:
	BYTE m_Light;		// the calculated light level in this area. |0x80 = override.
	SEASON_TYPE m_Season;		// What is the season for this sector.
	WEATHER_TYPE m_Weather;		// the weather in this area now.
public:
	CSectorEnviron()
	{
		m_Light = LIGHT_BRIGHT;	// set based on time later.
		m_Season = SEASON_Summer;
		m_Weather = WEATHER_DRY;
	}
	void SetInvalid()
	{
		// Force a resync of all this. we changed location by teleport etc.
		m_Light = -1;	// set based on time later.
		m_Season = SEASON_QTY;
		m_Weather = WEATHER_DEFAULT;
	}
};

enum REGRES_TYPE	// Registration server return values.
{
	REGRES_NOT_ATTEMPTED = 0,			// Have not set this to anything yet.
	REGRES_NOT_REQUIRED,	// This is the list server.
	REGRES_LIST_SERVER_NONE,
	REGRES_LIST_SERVER_UNRESOLVABLE,

	REGRES_FAIL_CREATE_SOCKET,
	REGRES_FAIL_CONNECT,
	REGRES_FAIL_SEND,

	// Code returned from list server.

	REGRES_RET_INVALID,				// registration is bad format.
	REGRES_RET_FAILURE,			// Some other failure decoding the msg.
	REGRES_RET_OK,				// Listed OK
	REGRES_RET_BAD_PASS,		// Password does not match for server.
	REGRES_RET_NAME_UNK,		// Not on the approved name list.
	REGRES_RET_DEBUG_MODE,	// List server is in debug mode. (no listing available)
	REGRES_RET_IP_UNK,

	// Not used yet codes.
	REGRES_RET_IP_INVALID,
	REGRES_RET_IP_NOT_RESPONDING,
	REGRES_RET_IP_BLOCKED,
	REGRES_RET_WEB_INVALID,
	REGRES_RET_WEB_NOT_RESPONDING,
	REGRES_RET_WEB_BLOCKED,
	REGRES_RET_EMAIL_INVALID,
	REGRES_RET_EMAIL_NOT_RESPONDING,
	REGRES_RET_EMAIL_BLOCKED,
	REGRES_RET_STATS_INVALID,	//  Statistics look invalid.

	REGRES_QTY,
};

enum CLIMODE_TYPE	// What mode is the client to server connection in ? (waiting for input ?)
{
	// setup events ------------------------------------------------

	CLIMODE_SETUP_CONNECTING,
	CLIMODE_SETUP_SERVERS,		// client has received the servers list.
	CLIMODE_SETUP_RELAY,		// client has been relayed to the game server. wait for new login.
	CLIMODE_SETUP_CHARLIST,	// client has the char list and may (play a char, delete a char, create a new char)

	// Capture the user input for this mode. ------------------------------------------
	CLIMODE_NORMAL,		// No targetting going on. we are just walking around etc.

	// asyc events enum here. --------------------------------------------------------
	CLIMODE_DRAG,			// I'm dragging something. (not quite a targeting but similar)
	CLIMODE_DEATH,			// The death menu is up.
	CLIMODE_DYE,			// The dye dialog is up.
	CLIMODE_INPVAL,		// special text input dialog (for setting item attrib)

	// Some sort of general gump dialog ----------------------------------------------
	CLIMODE_DIALOG,		// from RES_DIALOG

	// Hard-coded (internal) gumps.
	CLIMODE_DIALOG_ADMIN,
	CLIMODE_DIALOG_EMAIL,	// Force the user to enter their emial address. (if they have not yet done so)
	CLIMODE_DIALOG_GUILD,	// reserved.
	CLIMODE_DIALOG_HAIR_DYE, // Using hair dye
	CLIMODE_DIALOG_TOME,
	CLIMODE_DIALOG_VIRTUE = 0x1CD,

	// Making a selection from a menu. ----------------------------------------------
	CLIMODE_MENU,		// RES_MENU

	// Hard-coded (internal) menus.
	CLIMODE_MENU_SKILL,		// result of some skill. tracking, tinkering, blacksmith, etc.
	CLIMODE_MENU_SKILL_TRACK_SETUP,
	CLIMODE_MENU_SKILL_TRACK,
	CLIMODE_MENU_GM_PAGES,		// show me the gm pages .
	CLIMODE_MENU_EDIT,		// edit the contents of a container.

	// promting for text input.------------------------------------------------------
	//CLIMODE_PROMPT,					// Some sort of text prompt input.
	CLIMODE_PROMPT_NAME_RUNE,
	CLIMODE_PROMPT_NAME_KEY,		// naming a key.
	CLIMODE_PROMPT_NAME_SIGN,		// name a house sign
	CLIMODE_PROMPT_NAME_SHIP,
	CLIMODE_PROMPT_GM_PAGE_TEXT,	// allowed to enter text for page.
	CLIMODE_PROMPT_VENDOR_PRICE,	// What would you like the price to be ?
	CLIMODE_PROMPT_TARG_VERB,		// Send a msg to another player.
	CLIMODE_PROMPT_STONE_NAME,		// prompt for text.
	CLIMODE_PROMPT_STONE_SET_ABBREV,
	CLIMODE_PROMPT_STONE_SET_TITLE,
	CLIMODE_PROMPT_STONE_GRANT_TITLE,

	// Targetting mouse cursor. -------------------------------------------------------------
	CLIMODE_MOUSE_TYPE,	// Greater than this = mouse type targetting.

	// GM targetting command stuff.
	CLIMODE_TARG_OBJ_SET,		// Set some attribute of the item i will show.
	CLIMODE_TARG_OBJ_INFO,		// what item do i want props for ?
	CLIMODE_TARG_OBJ_FUNC,

	CLIMODE_TARG_UNEXTRACT,		// Break out Multi items
	CLIMODE_TARG_ADDITEM,		// "ADDITEM" command.
	CLIMODE_TARG_LINK,			// "LINK" command
	CLIMODE_TARG_TILE,			// "TILE" command.

	// Normal user stuff. (mouse targetting)
	CLIMODE_TARG_SKILL,				// targeting a skill or spell.
	CLIMODE_TARG_SKILL_MAGERY,
	CLIMODE_TARG_SKILL_HERD_DEST,
	CLIMODE_TARG_SKILL_POISON,
	CLIMODE_TARG_SKILL_PROVOKE,

	CLIMODE_TARG_USE_ITEM,			// target for using the selected item
	CLIMODE_TARG_PET_CMD,			// targetted pet command
	CLIMODE_TARG_PET_STABLE,		// Pick a creature to stable.
	CLIMODE_TARG_REPAIR,		// attempt to repair an item.
	CLIMODE_TARG_STONE_RECRUIT,		// Recruit members for a stone	(mouse select)
	CLIMODE_TARG_PARTY_ADD,

	CLIMODE_TARG_QTY,
};

class CClient : public CGObListRec, public CScriptObj, public CChatChanMember, public CTextConsole
{
	// TCP/IP connection to the player or console.
public:
	static LPCTSTR const sm_szRefKeys[];
	static LPCTSTR const sm_szLoadKeys[];
	static LPCTSTR const sm_szVerbKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	CChar * m_pChar;			// What char are we playing ?

	// Client last know state stuff.
	CPointMap m_pntSoundRecur;	// Get rid of this in the future !!???
	CSectorEnviron m_Env;		// Last Environment Info Sent. so i don't have to keep resending if it's the same.

	// Sync up with movement.
	bool m_fPaused;			// We paused the client for big download. (remember to unpause it)

	char m_fUpdateStats;	// update our own status (weight change) when done with the cycle.

	// Walk limiting code
	WORD m_wWalkCount;		// Make sure we are synced up with client walk count. may be be off by 4
	int	m_iWalkTimeAvg;
	int m_iWalkStepCount;		// Count the actual steps . Turning does not count.
	LONGLONG m_timeWalkStep;	// the last %8 walk step time.
	
	// Stupid Walk limiting code. (Not working really)
	DWORD	m_Walk_LIFO[16];	// Client > 1.26 must match these .
	int		m_Walk_InvalidEchos;
	int		m_Walk_CodeQty;

public:
	CONNECT_TYPE	m_iConnectType;	// what sort of a connection is this ?
	CGSocket		m_Socket;	
	bool			m_fClosed;	// flag it to get the socket closed
	CSocketAddress	m_PeerName;
	CAccount * m_pAccount;		// The account name. we logged in on

	static BYTE sm_xCompress_Buffer[MAX_BUFFER];

	CServTime m_timeLogin;		// World clock of login time. "LASTCONNECTTIME"
	CServTime m_timeLastEvent;	// Last time we got event from client.
	CServTime m_timeLastSend;		// Last time i tried to send to the client

	// GM only stuff.
	CGMPage * m_pGMPage;		// Current GM page we are connected to.
	CGrayUID m_Prop_UID;		// The object of /props (used for skills list as well!)

	// Current operation context args for modal async operations..
private:
	CLIMODE_TYPE m_Targ_Mode;	// Type of async operation under way.
public:
	CGrayUID m_Targ_UID;			// The object of interest to apply to the target.
	CGrayUID m_Targ_PrvUID;		// The object of interest before this.
	CGString m_Targ_Text;		// Text transfered up from client.
	CPointMap  m_Targ_p;			// For script targeting,

	// Context of the targetting setup. depends on CLIMODE_TYPE m_Targ_Mode
	union
	{
		// CLIMODE_SETUP_CONNECTING
		struct
		{
			DWORD m_dwIP;
			int m_iConnect;	// used for debug only.
		} m_tmSetup;

		// CLIMODE_SETUP_CHARLIST
		CGrayUIDBase m_tmSetupCharList[MAX_CHARS_PER_ACCT];

/*
		// CLIMODE_DIALOG_*
		struct
		{
			CGrayUIDBase m_UID;
			RESOURCE_ID_BASE m_ResourceID;	// the gump that is up.
		} m_tmGumpDialog;
*/
		// CLIMODE_DIALOG_ADMIN
		struct
		{
			CGrayUIDBase m_UID;
			BYTE m_Page;		// For long list dialogs. (what page are we on)
#define ADMIN_CLIENTS_PER_PAGE 8
			DWORD m_Item[MAX_MENU_ITEMS];	// This saves the inrange tracking targets or other context
		} m_tmGumpAdmin;

		// CLIMODE_INPVAL
		struct
		{
			CGrayUIDBase m_UID;
			RESOURCE_ID_BASE m_PrvGumpID;	// the gump that was up before this
		} m_tmInpVal;

		// CLIMODE_MENU_*
		// CLIMODE_MENU_SKILL
		// CLIMODE_MENU_GM_PAGES
		struct
		{
			CGrayUIDBase m_UID;
			RESOURCE_ID_BASE m_ResourceID;		// What menu is this ?
			DWORD m_Item[MAX_MENU_ITEMS];	// This saves the inrange tracking targets or other context
		} m_tmMenu;	// the menu that is up.

		// CLIMODE_TARG_PET_CMD
		struct
		{
			int	m_iCmd;
			bool m_fAllPets;
		} m_tmPetCmd;	// which pet command am i targetting ?

		// CLIMODE_TARG_CHAR_BANK
		struct
		{
			LAYER_TYPE m_Layer;	// gm command targetting what layer ?
		} m_tmCharBank;

		// CLIMODE_TARG_TILE
		// CLIMODE_TARG_UNEXTRACT
		struct
		{
			CPointBase m_ptFirst; // Multi stage targetting.
			int m_Code;
			int m_id;
		} m_tmTile;

		// CLIMODE_TARG_ADDITEM
		struct
		{
			DWORD m_junk0;
			ITEMID_TYPE m_id;
			int		m_fStatic;
		} m_tmAdd;

		// CLIMODE_TARG_SKILL
		struct
		{
			SKILL_TYPE m_Skill;			// targetting what spell ?
		} m_tmSkillTarg;

		// CLIMODE_TARG_SKILL_MAGERY
		struct
		{
			SPELL_TYPE m_Spell;			// targetting what spell ?
			CREID_TYPE m_SummonID;
			bool m_fSummonPet;
		} m_tmSkillMagery;

		// CLIMODE_TARG_USE_ITEM
		struct
		{
			CGObList * m_pParent;	// the parent of the item being targetted .
		} m_tmUseItem;
	};

private:
	// Low level data transfer to client.
	XCMD_TYPE m_bin_PrvMsg;
	XCMD_TYPE m_bin_ErrMsg;
	int m_bin_msg_len;		// the current message packet to decode. (estimated length)

	// ??? Since we really only deal with one input at a time we can make this static ?
	CQueueBytes m_bin;		// CEvent in buffer. (from client)
	CQueueBytes m_bout;		// CCommand out buffer. (to client) (we can build output to multiple clients at the same time)

	// encrypt/decrypt stuff.
	CCrypt m_Crypt;			// Client source communications are always encrypted.
	DWORD  m_dwCompressXORIndex;	// used only in Client 2.0.4 and above.
	static CCompressTree sm_xComp;

private:
	bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );

	bool OnRxConsoleLoginComplete();
	bool OnRxConsole( const BYTE * pData, int len );
	REGRES_TYPE OnRxAutoServerRegister( const BYTE * pData, int len );
	bool OnRxPeerServer( const BYTE * pData, int len );
	bool OnRxPing( const BYTE * pData, int len );
	bool OnRxWebPageRequest( CWebPageDef * pPage, LPCTSTR pszMatch );
	bool OnRxWebPageRequest( BYTE * pRequest, int len );

	LOGIN_ERR_TYPE LogIn( CAccountRef pAccount, CGString & sMsg );
	LOGIN_ERR_TYPE LogIn( LPCTSTR pszName, LPCTSTR pPassword, CGString & sMsg );

	bool IsBlockedIP() const;
	CLogIP * GetLogIP() const;
	void	UpdateLogIPConnecting( bool fIncrease );
	void	UpdateLogIPConnected( bool fIncrease );
	int	GetLogIPConnecting() const;
	int	GetLogIPConnected() const;

	// Low level message traffic.
	bool xCheckMsgSize0( int len );	// check packet only for sizeof
	bool xCheckMsgSize( int len );	// check packet.

#ifdef _DEBUG
	void xInit_DeCrypt_FindKey( const BYTE * pCryptData, int len );
#endif

	int  Setup_FillCharList( CEventCharDef * pCharList, const CChar * pCharFirst );

	bool CanInstantLogOut() const;
	void Cmd_GM_PageClear();
	void GetAdjustedCharID( const CChar * pChar, CREID_TYPE & id, HUE_TYPE &wHue );
	void GetAdjustedItemID( const CChar * pChar, const CItem * pItem, HUE_TYPE &wHue );
	void CharDisconnect();

	bool CmdTryVerb( CObjBase * pObj, LPCTSTR pszVerb );

	TRIGRET_TYPE Menu_OnSelect( RESOURCE_ID_BASE rid, int iSelect, CObjBase * pObj );
	TRIGRET_TYPE Dialog_OnButton( RESOURCE_ID_BASE rid, DWORD dwButtonID, CObjBase * pObj, CDialogResponseArgs * pArgs );

	LOGIN_ERR_TYPE Login_ServerList( const char * pszAccount, const char * pszPassword );		// Initial login (Login on "loginserver", new format)
	bool Login_Relay( int iServer );			// Relay player to a certain IP
	void Announce( bool fArrive ) const;

	LOGIN_ERR_TYPE Setup_ListReq( const char * pszAccount, const char * pszPassword, bool fTest );	// Gameserver login and character listing
	bool Setup_Start( CChar * pChar );	// Send character startup stuff to player
	void Setup_CreateDialog( const CEvent * pEvent );	// All the character creation stuff
	DELETE_ERR_TYPE Setup_Delete( int iSlot );			// Deletion of character
	bool Setup_Play( int iSlot );		// After hitting "Play Character" button

	// GM stuff.
	bool OnTarg_Obj_Set( CObjBase * pObj );
	bool OnTarg_Obj_Info( CObjBase * pObj, const CPointMap & pt, ITEMID_TYPE id );
	bool OnTarg_Obj_Function( CObjBase * pObj, const CPointMap & pt, ITEMID_TYPE id );

	bool OnTarg_UnExtract( CObjBase * pObj, const CPointMap & pt ) ;
	bool OnTarg_Stone_Recruit( CChar* pChar );
	bool OnTarg_Item_Add( CObjBase * pObj, const CPointMap & pt ) ;
	bool OnTarg_Item_Link( CObjBase * pObj );
	bool OnTarg_Tile( CObjBase * pObj, const CPointMap & pt );

	// Normal user stuff.
	bool OnTarg_Use_Deed( CItem * pDeed, const CPointMap &pt );
	bool OnTarg_Use_Item( CObjBase * pObj, CPointMap & pt, ITEMID_TYPE id );
	bool OnTarg_Party_Add( CChar * pChar );
	CItem* OnTarg_Use_Multi( const CItemBase * pItemDef, const CPointMap & pt, WORD wAttr, HUE_TYPE wHue );

	int OnSkill_AnimalLore( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_Anatomy( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_Forensics( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_EvalInt( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_ArmsLore( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_ItemID( CGrayUID uid, int iTestLevel, bool fTest );
	int OnSkill_TasteID( CGrayUID uid, int iTestLevel, bool fTest );

	bool OnTarg_Skill_Magery( CObjBase * pObj, const CPointMap & pt );
	bool OnTarg_Skill_Herd_Dest( CObjBase * pObj, const CPointMap & pt );
	bool OnTarg_Skill_Poison( CObjBase * pObj );
	bool OnTarg_Skill_Provoke( CObjBase * pObj );
	bool OnTarg_Skill( CObjBase * pObj );

	bool OnTarg_Pet_Command( CObjBase * pObj, const CPointMap & pt );
	bool OnTarg_Pet_Stable( CChar * pCharPet );

	// Commands from client
	void Event_ClientVersion( const char * pData, int Len );
	void Event_Spy( const CEvent * pEvent );
	void Event_Target( const CEvent * pEvent );
	void Event_Attack( CGrayUID uid );
	void Event_Skill_Locks( const CEvent * pEvent );
	void Event_Skill_Use( SKILL_TYPE x ); // Skill is clicked on the skill list
	void Event_Tips( WORD i); // Tip of the day window
	void Event_Book_Title( CGrayUID uid, LPCTSTR pszTitle, LPCTSTR pszAuthor );
	void Event_Book_Page( CGrayUID uid, const CEvent * pEvent ); // Book window
	void Event_Item_Dye( CGrayUID uid, HUE_TYPE wHue );	// Rehue an item
	void Event_Item_Pickup( CGrayUID uid, int amount ); // Client grabs an item
	void Event_Item_Equip( const CEvent * pEvent ); // Item is dropped on paperdoll
	void Event_Item_Drop( const CEvent * pEvent ); // Item is dropped on ground
	void Event_VendorBuy( CGrayUID uidVendor, const CEvent * pEvent );
	void Event_VendorSell( CGrayUID uidVendor, const CEvent * pEvent );
	void Event_SecureTrade( CGrayUID uid, const CEvent * pEvent );
	bool Event_DeathOption( DEATH_MODE_TYPE mode, const CEvent * pEvent );
	void Event_Walking( BYTE rawdir, BYTE count, DWORD dwCryptCode = 0 ); // Player moves
	void Event_CombatMode( bool fWar ); // Only for switching to combat mode
	void Event_MenuChoice( const CEvent * pEvent ); // Choice from GMMenu or Itemmenu received
	void Event_PromptResp( LPCTSTR pszText, int len );
	void Event_Talk_Common( TCHAR * szText ); // PC speech
	void Event_Talk( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode ); // PC speech
	void Event_TalkUNICODE( const CEvent * pEvent );
	void Event_SingleClick( CGrayUID uid );
	void Event_SetName( CGrayUID uid, const char * pszCharName );
	void Event_ExtCmd( EXTCMD_TYPE type, const char * pszName );
	bool Event_Command( LPCTSTR pszCommand ); // Client entered a '/' command like /ADD
	void Event_GumpInpValRet( const CEvent * pEvent );
	void Event_GumpDialogRet( const CEvent * pEvent );
	void Event_ToolTip( CGrayUID uid );
	void Event_ExtData( EXTDATA_TYPE type, const CExtData * pData, int len );
	void Event_MailMsg( CGrayUID uid1, CGrayUID uid2 );
	void Event_Profile( BYTE fWriteMode, CGrayUID uid, const CEvent * pEvent );
	void Event_MapEdit( CGrayUID uid, const CEvent * pEvent );
	void Event_BBoardRequest( CGrayUID uid, const CEvent * pEvent );
	void Event_ChatButton(const NCHAR * pszName); // Client's chat button was pressed
	void Event_ChatText( const NCHAR * pszText, int len, CLanguageID lang = 0 ); // Text from a client
	bool Event_WalkingCheck(DWORD dwEcho);
	void Event_ScrollClose( DWORD dwContext );

public:
	bool Event_DoubleClick( CGrayUID uid, bool fMacro, bool fTestTouch, bool fScript = false );

	// translated commands.
private:
	void Cmd_GM_PageInfo();
	int Cmd_Extract( CScript * pScript, CRectMap &rect, int & zlowest );
public:
	bool Cmd_CreateItem( ITEMID_TYPE id, bool fStatic = false );
	bool Cmd_CreateChar( CREID_TYPE id, SPELL_TYPE iSpell = SPELL_Summon, bool fPet = true );

	void Cmd_GM_PageMenu( int iEntryStart = 0 );
	void Cmd_GM_PageCmd( LPCTSTR pCmd );
	void Cmd_GM_PageSelect( int iSelect );
	void Cmd_GM_Page( LPCTSTR pszreason); // Help button (Calls GM Call Menus up)

	bool Cmd_Skill_Menu( RESOURCE_ID_BASE rid, int iSelect = -1 );
	bool Cmd_Skill_Smith( CItem * pIngots );
	bool Cmd_Skill_Magery( SPELL_TYPE iSpell, CObjBase * pSrc );
	bool Cmd_Skill_Tracking( int track_type = -1, bool fExec = false ); // Fill menu with specified creature types
	bool Cmd_Skill_Inscription();
	bool Cmd_Skill_Alchemy( CItem * pItem );
	bool Cmd_Skill_Cartography( int iLevel );
	void Cmd_Skill_Heal( CItem * pBandages, CChar * pTarg );
	bool Cmd_SecureTrade( CChar * pChar, CItem * pItem );
	bool Cmd_Control( CChar * pChar );

public:

	CClient( SOCKET client );
	~CClient();

	CClient* GetNext() const
	{
		return( STATIC_CAST <CClient*>( CGObListRec::GetNext()));
	}

	static int r_GetVerbIndex( LPCTSTR pszKey );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc ); // Execute script type command on me
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & s, CTextConsole * pSrc );
	virtual bool r_LoadVal( CScript & s );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );

	// Low level message traffic.
	static int xCompress( BYTE * pOutput, const BYTE * pInput, int inplen );
	static int xDeCompress( BYTE * pOutput, const BYTE * pInput, int inplen );

	void xSendReady( const void *pData, int length ); // We could send the packet now if we wanted to but wait til we have more.
	void xSend( const void *pData, int length ); // Buffering send function
	void xFlush();				// Sends buffered data at once
	void xSendPkt( const CCommand * pCmd, int length )
	{
		xSendReady((const void *)( pCmd->m_Raw ), length );
	}
	bool xHasData() const
	{
		return(( m_bin.GetDataQty()) ? true : false );
	}
	bool xProcessClientSetup( CEvent * pEvent, int iLen );
	void xProcessMsg( bool );	// Process a packet
	bool xDispatchMsg();
	bool xRecvData();			// High Level Receive message from client
	bool xCanEncLogin();	// Login crypt check
	// Low level push world data to the client.

	bool addRelay( const CServerDef * pServ );
	bool addLoginErr( LOGIN_ERR_TYPE code );
	void addPause( bool fPause = true );
#define SF_UPDATE_HITS		0x01
#define SF_UPDATE_MANA		0x02
#define SF_UPDATE_STAM		0x04
#define SF_UPDATE_STATUS	0x08

	void addUpdateStatsFlag()
	{
		m_fUpdateStats |= SF_UPDATE_STATUS;
		addPause();
	}
	void addUpdateHitsFlag()
	{
		m_fUpdateStats |= SF_UPDATE_HITS;
	}
	void addUpdateManaFlag()
	{
		m_fUpdateStats |= SF_UPDATE_MANA;
	}
	void addUpdateStamFlag()
	{
		m_fUpdateStats |= SF_UPDATE_STATUS;
	}
	void UpdateStats();
	bool addDeleteErr( DELETE_ERR_TYPE code );
	void addExtData( EXTDATA_TYPE type, const CExtData * pData, int iSize );
	void addSeason( SEASON_TYPE season, bool fNormalCursor = true );
	void addOptions();
	void addObjectRemoveCantSee( CGrayUID uid, LPCTSTR pszName = NULL );
	void addObjectRemove( CGrayUID uid );
	void addObjectRemove( const CObjBase * pObj );
	void addRemoveAll( bool fItems, bool fChars );
	void addObjectLight( const CObjBase * pObj, LIGHT_PATTERN iLightType = LIGHT_LARGE );		// Object light level.

	void addItem_OnGround( CItem * pItem ); // Send items (on ground)
	void addItem_Equipped( const CItem * pItem );
	void addItem_InContainer( const CItem * pItem );
	void addItem( CItem * pItem );

	void addOpenGump( const CObjBase * pCont, GUMP_TYPE gump );
	int  addContents( const CItemContainer * pCont, bool fCorpseEquip = false, bool fCorpseFilter = false, bool fShop = false, bool fHasSkill = false ); // Send items
	bool addContainerSetup( const CItemContainer * pCont ); // Send Backpack (with items)

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
	void addEffect( EFFECT_TYPE motion, ITEMID_TYPE id, const CObjBaseTemplate * pDst, const CObjBaseTemplate * pSrc, BYTE speed = 5, BYTE loop = 1, bool explode = false, DWORD color = 0, DWORD render = 0 );
	void addSound( SOUND_TYPE id, const CObjBaseTemplate * pBase = NULL, int iRepeat = 1 );
	void addReSync();
	void addMapplane( const CPointMap * pOldP );
	void addPing( BYTE bCode = 0 );

	void addBark( LPCTSTR pText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue = HUE_DEFAULT, TALKMODE_TYPE mode = TALKMODE_SAY, FONT_TYPE font = FONT_BOLD );
	void addBarkUNICODE( const NCHAR * pText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang = 0 );
	void addBarkSpeakTable( SPKTAB_TYPE index, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font );
	void addBarkParse( LPCTSTR pszText, const CObjBaseTemplate * pSrc, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font );
	void addSysMessage( LPCTSTR pMsg ); // System message (In lower left corner)
	void addObjMessage( LPCTSTR pMsg, const CObjBaseTemplate * pSrc, HUE_TYPE wHue = HUE_TEXT_DEF ); // The message when an item is clicked

	void addDyeOption( const CObjBase * pBase );
	void addItemDragCancel( BYTE type );
	void addWebLaunch( LPCTSTR pMsg ); // Direct client to a web page

	void addPromptConsole( CLIMODE_TYPE mode, LPCTSTR pMsg );
	void addTarget( CLIMODE_TYPE targmode, LPCTSTR pMsg, bool fAllowGround = false, bool fCheckCrime=false ); // Send targetting cursor to client
	void addTargetDeed( const CItem * pDeed );
	bool addTargetItems( CLIMODE_TYPE targmode, ITEMID_TYPE id );
	bool addTargetChars( CLIMODE_TYPE mode, CREID_TYPE id, bool fNoto );
	void addTargetVerb( LPCTSTR pCmd, LPCTSTR pArg );
	void addTargetFunction( LPCTSTR pszFunction, bool fAllowGround, bool fCheckCrime );

	void addScrollText( LPCTSTR pszText );
	void addScrollScript( CResourceLock &s, SCROLL_TYPE type, DWORD dwcontext = 0, LPCTSTR pszHeader = NULL );
	void addScrollResource( LPCTSTR szResourceName, SCROLL_TYPE type, DWORD dwcontext = 0 );

	void addVendorClose( const CChar * pVendor );
	int  addShopItems( CChar * pVendor, LAYER_TYPE layer );
	bool addShopMenuBuy( CChar * pVendor );
	int  addShopMenuSellFind( CItemContainer * pSearch, CItemContainer * pFrom1, CItemContainer * pFrom2, int iConvertFactor, CCommand * & pCur );
	bool addShopMenuSell( CChar * pVendor );
	void addBankOpen( CChar * pChar, LAYER_TYPE layer = LAYER_BANKBOX );

	void addSpellbookOpen( CItem * pBook, BYTE offset = 1 );
	void addCustomSpellbookOpen( CItem * pBook, DWORD gumpID );
	bool addBookOpen( CItem * pBook );
	void addBookPage( const CItem * pBook, int iPage );
	void addCharStatWindow( CGrayUID uid, bool fRequested = false ); // Opens the status window
	void addHitsUpdate( CGrayUID uid );
	void addManaUpdate( CGrayUID uid );
	void addStamUpdate( CGrayUID uid );
	void addSkillWindow( SKILL_TYPE skill ); // Opens the skills list
	void addBulletinBoard( const CItemContainer * pBoard );
	bool addBBoardMessage( const CItemContainer * pBoard, BBOARDF_TYPE flag, CGrayUID uidMsg );
	void addCharList3();

	void addToolTip( const CObjBase * pObj, LPCTSTR psztext );
	void addMap( CItemMap * pItem );
	void addMapMode( CItemMap * pItem, MAPCMD_TYPE iType, bool fEdit = false );

	void addGumpTextDisp( const CObjBase * pObj, GUMP_TYPE gump, LPCTSTR pszName, LPCTSTR pszText );
	void addGumpInpVal( bool fcancel, INPVAL_STYLE style,
		DWORD dwmask, LPCTSTR ptext1, LPCTSTR ptext2, CObjBase * pObj );

	void addItemMenu( CLIMODE_TYPE mode, const CMenuItem * item, int count, CObjBase * pObj = NULL );
	void addGumpDialog( CLIMODE_TYPE mode, const CGString * sControls, int iControls, const CGString * psText, int iTexts, int x, int y, CObjBase * pObj = NULL, DWORD rid = 0 );

	bool addGumpDialogProps( CGrayUID uid );
	void addGumpDialogAdmin( int iPage );

	void addEnableFeatures( int features );
	void addChatSystemMessage(CHATMSG_TYPE iType, LPCTSTR pszName1 = NULL, LPCTSTR pszName2 = NULL, CLanguageID lang = 0 );

	bool addWalkCode( EXTDATA_TYPE iType, int iQty );

	void addAOSTooltip( const CObjBase * pObj );
	void Event_AOSPopupMenu( DWORD uid, WORD EntryTag = 0 );

	void SendPacket( TCHAR * pszPacket );

	// Test what I can do
	CAccountRef GetAccount() const
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
	LPCTSTR GetName() const
	{
		if ( GetAccount() == NULL )
			return( "NA" );
		return( GetAccount()->GetName());
	}
	CChar * GetChar() const
	{
		return( m_pChar );
	}

	void SysMessage( LPCTSTR pMsg ) const; // System message (In lower left corner)
	bool CanSee( const CObjBaseTemplate * pObj ) const;
	bool CanHear( const CObjBaseTemplate * pSrc, TALKMODE_TYPE mode ) const;

	bool Dialog_Setup( CLIMODE_TYPE mode, RESOURCE_ID_BASE rid, int iPage, CObjBase * pObj, LPCTSTR Arguments = "" );
	bool Dialog_Close( CObjBase * pObj, RESOURCE_ID_BASE rid, int buttonID );
	void Menu_Setup( RESOURCE_ID_BASE rid, CObjBase * pObj = NULL );

	int OnSkill_Info( SKILL_TYPE skill, CGrayUID uid, int iTestLevel, bool fTest );

	bool Cmd_Use_Item( CItem * pItem, bool fTestTouch, bool fScript = false );
	void Cmd_EditItem( CObjBase * pObj, int iSelect );

	virtual bool IsAnsi() const
	{
		// can we use ANSI escape codes here ? for colors and sounds ?
		return( m_iConnectType == CONNECT_TELNET );
	}
	bool IsConnectTypePacket() const
	{
		// This is a game or login server.
		// m_Crypt.IsInit()
		return( m_iConnectType == CONNECT_CRYPT || m_iConnectType == CONNECT_LOGIN || m_iConnectType == CONNECT_GAME );
	}
	
	CONNECT_TYPE	GetConnectType() const
	{
		return m_iConnectType;
	}

	void SetConnectType( CONNECT_TYPE iType );
	
	// Stuff I am doing. Started by a command
	CLIMODE_TYPE GetTargMode() const
	{
		return( m_Targ_Mode );
	}
	void SetTargMode( CLIMODE_TYPE targmode = CLIMODE_NORMAL, LPCTSTR pszPrompt = NULL );
	void ClearTargMode()
	{
		// done with the last mode.
		m_Targ_Mode = CLIMODE_NORMAL;
	}

	bool IsConnecting();

	char	m_zLogin[64];
};

////////////////////////////////////////////////////////////////////////////////////

#include "CBase.h"
#include "CObjBase.h"
#include "CWorld.h"

////////////////////////////////////////////////////////////////////////////////////

class CMailSMTP 
{
	// Mail sending class.
protected:
    CGString m_sResponse;
#define PORT_SMTP	25
	CGSocket m_Socket;
private:
    bool ReceiveResponse();

protected:
    bool OpenConnection(LPCTSTR pszMailServerIPAddress );
    bool SendLine(LPCTSTR pszMessage, int iResponseExpect = 1 );
public:
	bool SendMail( LPCTSTR pszDstMail, LPCTSTR pszSrcMail, CGObArray< CGString * > * pStrings );
	LPCTSTR GetLastResponse() const
	{
		return m_sResponse;
	}
};

enum PROFILE_TYPE
{
	PROFILE_IDLE,		// Wait for stuff.
	PROFILE_OVERHEAD,	// In between stuff.
#ifndef BUILD_WITHOUT_IRCSERVER
	PROFILE_IRC,
#endif
	PROFILE_NETWORK_RX,	// Just get client info and monitor new client requests. No processing.
	PROFILE_CLIENTS,	// Client processing.
	PROFILE_NETWORK_TX,
	PROFILE_CHARS,
	PROFILE_ITEMS,
	PROFILE_NPC_AI,
	PROFILE_SCRIPTS,
	PROFILE_TIME_QTY,

	// Qty of bytes. Not Time.
	PROFILE_DATA_TX = PROFILE_TIME_QTY,
	PROFILE_DATA_RX,

	PROFILE_QTY,
};

struct CProfileDataRec
{
	LONGLONG m_Time;	// accumulated time in msec.
	int m_iCount;		// how many passes made into this.
};

class CProfileData
{
protected:
	CProfileDataRec	m_AvgTimes[ PROFILE_QTY ];
	CProfileDataRec m_PrvTimes[ PROFILE_QTY ];
	CProfileDataRec m_CurTimes[ PROFILE_QTY ];

	int m_iActiveWindowSec;		// The sample window size in seconds. 0=off
	int	m_iAvgCount;

	LONGLONG m_Frequency;	// QueryPerformanceFrequency()
	LONGLONG m_TimeTotal;	// Average this over a total time period.

	// Store the last time start time.
	PROFILE_TYPE  m_CurTask;	// What task are we currently processing ?
	LONGLONG m_CurTime;		// QueryPerformanceCount()

public:
	bool IsActive() const { return( m_iActiveWindowSec ? true : false ); }
	void SetActive( int iSampleSec );
	int GetActiveWindow() const { return m_iActiveWindowSec; }

	PROFILE_TYPE	GetCurrentTask();
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

	LPCTSTR GetName( PROFILE_TYPE id ) const;
	LPCTSTR GetDesc( PROFILE_TYPE id ) const;
};

enum SERVMODE_TYPE
{
	SERVMODE_ScriptBook,	// Run at Lower Priv Level
	SERVMODE_RestockAll,	// Major event.
	SERVMODE_Saving,		// Forced save freezes the system.
	SERVMODE_Run,			// Game is up and running
	SERVMODE_ResyncPause,	// paused during resync

	SERVMODE_Loading,		// Initial load.
	SERVMODE_ResyncLoad,	// Loading after resync
	SERVMODE_Exiting,		// Closing down
	SERVMODE_Test5,			// Sometest modes.
	SERVMODE_Test8,			// Sometest modes.
};

extern class CServer : public CServerDef, public CTextConsole
{
	static LPCTSTR const sm_szVerbKeys[];

public:
	SERVMODE_TYPE m_iModeCode;  // Just some error code to return to system.
	int  m_iExitFlag;	// identifies who caused the exit. <0 = error
	bool m_fResyncPause;		// Server is temporarily halted so files can be updated.
	DWORD m_dwParentThread;	// The thread we got Init in.

	CGSocket m_SocketMain;	// This is the incoming monitor socket.(might be multiple ports?)
	CGSocket m_SocketGod;	// This is for god clients.
	unsigned int m_uSizeMsgMax;	// get this from the TCP/IP stack params

	// admin console.
	int m_iAdminClients;		// how many of my clients are admin consoles ?
	CGString m_sConsoleText;
	bool m_fConsoleTextReadyFlag;	// interlocking flag for moving between tasks.

	CServTime m_timeShutdown;	// When to perform the shutdowm (g_World.clock)

	int m_nGuestsCur;		// How many of the current clients are "guests". Not accurate !
	CGObList m_Clients;		// Current list of clients (CClient)

	CProfileData m_Profile;	// the current active statistical profile.
	CChat m_Chats;	// keep all the active chats

public:
	CServer();
	~CServer();

	virtual bool IsAnsi() const;

	bool IsValidBusy() const;
	bool OnTick_Busy() const;
	void SetServerMode( SERVMODE_TYPE mode );
	bool TestServerIntegrity();

	void SetExitFlag( int iFlag );
	void Shutdown( int iMinutes );
	bool IsLoading() const
	{
		return( m_iModeCode > SERVMODE_Run || m_fResyncPause );
	}
	void SetSignals( bool fMsg = true );

	bool SocketsInit(); // Initialize sockets
	bool SocketsInit( CGSocket & socket, bool fGod );
	void SocketsReceive();
	CClient * SocketsReceive( CGSocket & socket, bool fGod );
	void SocketsFlush();
	void SocketsClose();

	bool Load();

	void SysMessage( LPCTSTR pMsg ) const;
	void PrintTelnet( LPCTSTR pszMsg ) const;
	void PrintStr( LPCTSTR pMsg ) const;
	int  PrintPercent( long iCount, long iTotal );

	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc = NULL );
	virtual bool r_LoadVal( CScript & s );
	virtual bool r_Verb( CScript & s, CTextConsole * pSrc );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
	void r_Write( CScript &s );

	LPCTSTR GetStatusString( BYTE iIndex = 0 ) const;
	LPCTSTR GetStatusStringRegister( bool fFirst ) const;
	int GetAgeHours() const;

	bool OnConsoleCmd( CGString & sText, CTextConsole * pSrc );

	void OnTick();

private:
	void ListServers( CTextConsole * pConsole ) const;

public:
	void ListClients( CTextConsole * pClient ) const;
	void ListGMPages( CTextConsole * pConsole ) const;

	CClient * GetClientHead() const
	{
		return( STATIC_CAST <CClient*>( m_Clients.GetHead()));
	}

	void SetResyncPause( bool fPause, CTextConsole * pSrc );
	bool CommandLine( int argc, TCHAR * argv[] );

	LPCTSTR GetName() const { return( CServerDef::GetName()); }
	PLEVEL_TYPE GetPrivLevel() const
	{
		if ( m_iModeCode == SERVMODE_ScriptBook )
			return( PLEVEL_GM );
		return( PLEVEL_Admin );
	}

} g_Serv;	// current state stuff not saved.

extern class CBackTask : public CThread
{
	// Do polling ad other stuff in a background thread.
private:
	CServTime m_timeNextRegister;	// only register once every 2 hours or so.
	CServTime m_timeNextPoll;		// Only poll listed CServerDef once every 10 minutes or so.
	CServTime m_timeNextMail;		// Time to check for outgoing mail ?
public:
	CSocketAddress m_RegisterIP;	// look up this ip. (looked up host name)
	CGString m_sRegisterResult;			// result of last register attempt.
	int		m_iTotalPolledClients;
	int		m_iTotalPolledAccounts;
private:
	static THREAD_ENTRY_RET _cdecl EntryProc( void * lpThreadParameter );
	void EntryTask();
	bool RegisterServer();
	void PollServers();
	void SendOutgoingMail();

public:
	CBackTask()
	{
		m_timeNextRegister.Init();
		m_timeNextPoll.Init();
		m_timeNextMail.Init();
		m_RegisterIP.SetPort( GRAY_DEF_PORT );
		m_iTotalPolledClients = 0;
	}

	// Check the creation of the back task.
	void CreateThread();
	void CheckStuckThread();

} g_BackTask;

extern class CMainTask : public CThread
{
	// The main world thread. (handles all client connections)
private:
	CServTime m_timeRestart;
	CServTime m_timeWarn;
	CServTime m_timePrev;
public:
	static THREAD_ENTRY_RET _cdecl EntryProc( void * lpThreadParameter );
public:
	void CreateThread();
	void CheckStuckThread();
	CMainTask()
	{
		m_timeRestart.Init();
		m_timeWarn.Init();
		m_timePrev.Init();
	}

} g_MainTask;

//////////////////////////////////////////////////////////////

extern LPCTSTR g_szServerDescription;
extern LPCTSTR const g_Stat_Name[STAT_QTY];
extern const CPointMap g_pntLBThrone; // This is origin for degree, sextant minute coordinates
extern const WORD g_Item_Horse_Mounts[][2];

#ifdef _WIN32
extern bool NTWindow_Init( HINSTANCE hInstance, LPSTR lpCmdLinel, int nCmdShow );
extern void NTWindow_Exit();
extern bool NTWindow_OnTick( int iWaitmSec );
extern bool NTWindow_PostMsg( LPCTSTR pszMsg );
extern bool NTWindow_PostMsgColor( COLORREF color );
extern void NTWindow_SetWindowTitle( LPCTSTR pText = NULL );
#endif

extern int Sphere_InitServer( int argc, char *argv[] );
extern int Sphere_OnTick();
extern void Sphere_ExitServer();
extern int Sphere_MainEntryPoint( int argc, char *argv[] );

extern "C"
{
extern void globalstartsymbol();
extern void globalendsymbol();
extern const int globalstartdata;
extern const int globalenddata;
}

///////////////////////////////////////////////////////////////
// -CGrayUID

inline int CObjBase::GetTimerDiff() const
{
	// How long till this will expire ?
	return( g_World.GetTimeDiff( m_timeout ));
}
inline CObjBase * CGrayUIDBase::ObjFind() const
{
	if ( IsResource())
		return( NULL );
	return( g_World.FindUID( m_dwInternalVal & UID_O_INDEX_MASK ));
}
inline CItem * CGrayUIDBase::ItemFind() const
{
	// IsItem() may be faster ?
	return( dynamic_cast <CItem *>( ObjFind()));
}
inline CChar * CGrayUIDBase::CharFind() const
{
	return( dynamic_cast <CChar *>( ObjFind()));
}

//	Exceptions debugging routine.
#ifdef EXCEPTIONS_DEBUG
	enum EXC_TYPE
	{
		EXC_NONE,
		EXC_CALL,
		EXC_DORAND,
		EXC_FOR,
		EXC_FORCHAR,
		EXC_FORCHARLAYER,
		EXC_FORCLIENTS,
		EXC_FORCONT,
		EXC_FORCONTID,
		EXC_FORITEM,
		EXC_FOROBJ,
		EXC_FORPLAYERS,
		EXC_KEY_INIT,
		EXC_KEY_READ,
		EXC_MEMCPY,
		EXC_PARSING,
		EXC_PARSING_BEGIN,
		EXC_PARSING_IF,
		EXC_REENTRANT,
		EXC_RETURN,
		EXC_VERB,
		EXC_WHILE,
		EXC_WRITEVAL,
		EXC_QTY,
	};

	#define EXC_TRY(a) \
						const char *inLocalBlock; \
						TCHAR	*inLocalArgs; \
						int inLocalBlockCnt(0); \
						try \
						{ \
							g_ExcArguments = inLocalArgs = Exc_GetStack(); \
							*inLocalArgs = 0; \
							exceptions_debug_init a; \
							inLocalBlock = m_ExcKeys[EXC_NONE]
	#define EXC_SET(a)	inLocalBlock = m_ExcKeys[a]; \
						inLocalBlockCnt++
	#define EXC_CATCH_NOTHROW(a)	}	\
							catch ( CGrayError &e )	\
							{ \
								g_Log.CatchEvent(&e, "%s (%i,%s) - %s", inLocalArgs, inLocalBlockCnt, inLocalBlock, a); \
							} \
							catch (...) \
							{ \
								g_Log.CatchEvent(NULL, "%s (%i,%s) - %s", inLocalArgs, inLocalBlockCnt, inLocalBlock, a); \
							}
	#define EXC_CATCH_THROW(a)	}	\
							catch ( CGrayError &e )	\
							{ \
								g_Log.CatchEvent(&e, "%s (%i,%s) - %s", inLocalArgs, inLocalBlockCnt, inLocalBlock, a); \
								throw(e); \
							} \
							catch (...) \
							{ \
								g_Log.CatchEvent(NULL, "%s (%i,%s) - %s", inLocalArgs, inLocalBlockCnt, inLocalBlock, a); \
								throw; \
							}

	extern LPCTSTR const m_ExcKeys[];
	extern char *g_ExcArguments;
	extern TCHAR g_ExcStack[];
	extern TCHAR *Exc_GetStack();
	extern int g_ExcCurrent;
	extern void _cdecl exceptions_debug_init(LPCTSTR pszFormat, ...);
#else
	#define EXC_TRY(a)
	#define EXC_SET(a)
	#define EXC_CATCH_NOTHROW(a)
	#define EXC_CATCH_THROW(a)
#endif
#ifdef EXCEPTIONS_USETHROW
	#define EXC_CATCH	EXC_CATCH_THROW
#else
	#define EXC_CATCH	EXC_CATCH_NOTHROW
#endif

#endif	// _INC_GRAYSVR_H_
