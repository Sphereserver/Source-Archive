//
// Caccount.h
//

#ifndef _INC_CACCOUNT_H
#define _INC_CACCOUNT_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CClient;

class CAccount : public CMemDynamic, public CScriptObj, public CThreadLockableObj
{
	// RES_ACCOUNT
	static LPCTSTR const sm_szVerbKeys[];
	static LPCTSTR const sm_szLoadKeys[];
protected:
	DECLARE_MEM_DYNAMIC;
private:
	PLEVEL_TYPE m_PrivLevel;
	CGString m_sName;			// Name = no spaces. case independant.
	CGString m_sCurPassword;	// Accounts auto-generated but never used should not last long !
	CGString m_sNewPassword;	// The new password will be transfered when they use it.

#define PRIV_SERVER			0x0001	// This is a server account. CServerDef - Same name
#define PRIV_GM				0x0002	// Acts as a GM (dif from having GM level)

#define PRIV_GM_PAGE		0x0008	// Listen to GM pages or not.
#define PRIV_HEARALL		0x0010	// I can hear everything said by people of lower plevel
#define PRIV_ALLMOVE		0x0020	// I can move all things. (GM only)
#define PRIV_DETAIL			0x0040	// Show combat detail messages
#define PRIV_DEBUG			0x0080	// Show all objects as boxes and chars as humans.
#define PRIV_EMAIL_VALID	0x0100	// The email address has been validated.
#define PRIV_PRIV_NOSHOW	0x0200	// Show the GM title and Invul flags.

#define PRIV_JAILED			0x0800	// Must be /PARDONed from jail.
#define PRIV_T2A			0x1000	// They have a t2a client.exe (30=dark,new creatures,etc)
#define PRIV_BLOCKED		0x2000	// The account is blocked.
#define PRIV_ALLSHOW		0x4000	// Show even the offline chars.
#define PRIV_TEMPORARY		0x8000	// Delete this account when done.

	WORD m_PrivFlags;			// optional privileges for char (bit-mapped)
public:

	CLanguageID m_lang;			// UNICODE language pref. (ENU=english)
	CGString m_sChatName;		// Chat System Name

	int m_Total_Connect_Time;	// Previous total amount of time in game. (minutes) "TOTALCONNECTTIME"

	CSocketAddressIP m_Last_IP;	// last ip i logged in from.
	CGTime m_dateLastConnect;	// The last date i logged in. (use localtime())
	int  m_Last_Connect_Time;	// Amount of time spent online last time. (in minutes)

	CSocketAddressIP m_First_IP;	// first ip i logged in from.
	CGTime m_dateFirstConnect;	// The first date i logged in. (use localtime())

	CGrayUID m_uidLastChar;		// Last char i logged in with.
	CCharRefArray m_Chars;		// list of chars attached to this account.
	CVarDefArray m_TagDefs;		// attach extra tags here. GM comments etc.

	CGString m_sEMail;			// My auto event notifier.
	BYTE m_iEmailFailures;		// How many times has this email address failed ?
	CGTypedArray<RESOURCE_ID,RESOURCE_ID> m_EMailSchedule;	// RES_EMAILMSG id's of msgs scheduled.

private:
	bool SetEmailAddress( LPCTSTR pszEmail, bool fBlockHost );
	bool ScheduleEmailMessage( RESOURCE_ID iEmailMessage );
	static bool CheckBlockedEmail( LPCTSTR pszEmail);
	bool SendOutgoingMail( RESOURCE_ID iMsg );

public:
	CAccount( LPCTSTR pszName, bool fGuest = false );
	virtual ~CAccount();

	static int NameStrip( TCHAR * pszNameOut, LPCTSTR pszNameInp );
	static PLEVEL_TYPE GetPrivLevelText( LPCTSTR pszFlags );

	virtual bool r_LoadVal( CScript & s );
	virtual bool r_WriteVal( LPCTSTR pszKey, CGString &sVal, CTextConsole * pSrc );
	virtual bool r_Verb( CScript &s, CTextConsole * pSrc );
	virtual bool r_GetRef( LPCTSTR & pszKey, CScriptObj * & pRef );
	virtual void r_DumpLoadKeys( CTextConsole * pSrc );
	virtual void r_DumpVerbKeys( CTextConsole * pSrc );
	void r_Write( CScript & s, bool fTitle );

	bool IsMyAccountChar( const CChar * pChar ) const;
	void SendAllOutgoingMail();
	bool Kick( CTextConsole * pSrc, bool fBlock );

	LPCTSTR GetName() const
	{
		return( m_sName );
	}
	LPCTSTR GetPassword() const
	{
		return( m_sCurPassword );
	}
	void SetPassword( LPCTSTR pszPassword )
	{
		// limit to 16 chars.
		m_sCurPassword = pszPassword;
		if ( m_sCurPassword.GetLength() > MAX_ACCOUNT_PASSWORD_ENTER )
		{
			m_sCurPassword.SetLength( MAX_ACCOUNT_PASSWORD_ENTER );
		}
	}
	void ClearPassword()
	{
		m_sCurPassword.Empty();	// can be set on next login.
	}
	bool CheckPassword( LPCTSTR pszPassword );

	LPCTSTR GetNewPassword() const
	{
		return( m_sNewPassword );
	}
	void SetNewPassword( LPCTSTR pszPassword );

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
	void TogPrivFlags( WORD wPrivFlags, LPCTSTR pszArgs );
	WORD GetPrivFlags() const
	{
		return m_PrivFlags;
	}
	PLEVEL_TYPE GetPrivLevel() const
	{
		return( m_PrivLevel );	// PLEVEL_Counsel
	}
	void SetPrivLevel( PLEVEL_TYPE plevel );
	void OnLogin( CClient * pClient );
	void OnLogout( CClient * pClient );

	int DetachChar( CChar * pChar );
	int AttachChar( CChar * pChar );

	CClient * FindClient( const CClient * pExclude = NULL ) const;
};

typedef CAccount * CAccountRef;		// CThreadLockRef

class CLogIP
{
	// Keep a log of ALL recent ip's we have talked to.
	// Prevent ping floods etc.
private:
	const CSocketAddressIP m_ip;
	bool m_fBlocked;	// block further input from this ip til decay time.
	int m_iPings;		// how many pings have we seen ?
	CServTime m_timeDecay;		// Time this record should be gone.
	CServTime m_timeFirst;		// when did this first happen ? CServTime::GetCurrentTime() TICK_PER_SEC
	CServTime m_timeLast;		// CServTime::GetCurrentTime() TICK_PER_SEC
	CAccount * m_pAccount;	// associate this with an account.
public:
	CLogIP( const CSocketAddressIP dwIP );
	bool IsSameIP( const CSocketAddressIP ip ) const
	{
		// Allow for pattern matching in the future ?
		return( m_ip == ip );
	}
	int GetPings() const
	{
		return( m_iPings );
	}
	bool IsBlocked() const
	{
		return( m_fBlocked );
	}
	void InitTimes();
	void SetBlocked( bool fBlocked, int iTimeDecay );
	bool CheckPingBlock( bool fPreAccept );
	bool IsTimeDecay() const;

	void SetAccount( CAccount * pAccount )
	{
		m_pAccount = pAccount;
	}
	CAccountRef GetAccount() const
	{
		return m_pAccount;	// associate this with an account.
	}
};

extern class CAccounts
{
	// The full accounts database.
	// Stuff saved in *ACCT.SCP file.
private:
	static LPCTSTR const sm_szVerbKeys[];
	CThreadLockableDefArray m_Accounts;
private:

	bool Cmd_AddNew( CTextConsole * pSrc, bool fMail, LPCTSTR pszName, LPCTSTR pszArg );
	bool Cmd_ListUnused( CTextConsole * pSrc, LPCTSTR pszDays, LPCTSTR pszVerb, LPCTSTR pszArgs );

public:
	// Accounts.
	bool Account_SaveAll();
	bool Account_Load( LPCTSTR pszNameRaw, CScript & s, bool fChanges );
	bool Account_LoadAll( bool fChanges = true, bool fClearChanges = false );
	bool Account_OnCmd( TCHAR * pszArgs, CTextConsole * pSrc );
	int Account_GetCount() const
	{
		return( m_Accounts.GetCount());
	}
	CAccountRef Account_Get( int index );
	CAccountRef Account_Find( LPCTSTR pszName );
	CAccountRef Account_FindCreate( LPCTSTR pszName, bool fCreate = false );
	CAccountRef Account_FindChat( LPCTSTR pszName );
	void Account_Delete( CAccount * pAccount );
	void Account_Add( CAccount * pAccount );

} g_Accounts;	// All the player accounts. name sorted CAccount

#endif

