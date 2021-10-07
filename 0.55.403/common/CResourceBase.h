//
// CResourceBase.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CResourceBase_H
#define _INC_CResourceBase_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CTime.h"

enum RES_TYPE	// all the script resource blocks we know how to deal with !
{
	// NOTE: SPHERE.INI, SPHERETABLE.SCP are read at start.
	// All other files are indexed from the SPEECHFILES and SCPFILES directories.
	// (SI) = Single instance types.
	// (SL) = single line multiple definitions.
	// Alphabetical order.
	RES_UNKNOWN = 0,		// Not to be used.
	RES_ACCOUNT,		// Define an account instance.
	RES_ADVANCE,		// Define the advance rates for stats.
	RES_AREA,			// Complex region. (w/extra tags)
	RES_BLOCKEMAIL,		// (SL) A list of email addresses we will not accept.
	RES_BLOCKIP,		// (SL) A list of IP's to block.
	RES_BOOK,			// A book or a page from a book.
	RES_CHARDEF,		// Define a char type. (overlap with RES_SPAWN)
	RES_COMMENT,		// A commented out block type.
	RES_CRYSTALBALL,	// (SL) Random (short) tip type messages.
	RES_DEFNAME,		// (SL) Just add a bunch of new defs and equivs str/values.
	RES_DIALOG,			// A scriptable gump dialog, text or handler block.
	RES_ECONOMY,		// Define the prices of a bunch of stuff.
	RES_EMAILMSG,		// define an email msg that could be sent to an account.
	RES_EVENTS,			// An Event handler block with the trigger type in it. ON=@Death etc.
	RES_FUNCTION,		// Define a new command verb script that applies to a char.
	RES_GMPAGE,			// A GM page. (SAVED in World)
	RES_HELP,			// Command help blocks. (read in as needed)
	RES_ITEMDEF,		// Define an item type. (overlap with RES_TEMPLATE)
	RES_LOCATION,		// Ignore this AXIS data.
	RES_MAP,			// Define info about the mapx.mul files
	RES_MENU,			// General scriptable menus.
	RES_MOONGATES,		// (SL) Define where the moongates are.
	RES_NAMES,			// A block of possible names for a NPC type. (read as needed)
	RES_NEWBIE,			// Triggers to execute on Player creation (based on skills selected)
	RES_NOTOTITLES,		// (SI) Define the noto titles used.
	RES_OBSCENE,		// (SL) A list of obscene words.
	RES_PLEVEL,			// Define the list of commands that a PLEVEL can access. (or not access)
	RES_QUEST,
	RES_RACECLASS,		// General race params about creature types. regen rates, etc
	RES_REGIONRESOURCE,	// Define an Ore type.
	RES_REGIONTYPE,		// Triggers etc. that can be assinged to a RES_AREA
	RES_RESOURCES,		// (SL) list of all the resource files we should index !
	RES_ROOM,			// Non-complex region. (no extra tags)
	RES_RUNES,			// (SI) Define list of the magic runes.
	RES_SCHEDULE,		// A list of times things can happen.
	RES_SCROLL,			// SCROLL_GUEST=message scroll sent to player at guest login. SCROLL_MOTD, SCROLL_NEWBIE
	RES_SECTOR,			// Make changes to a sector. (SAVED in World)
	RES_SERVER,			// Define a peer sphere server we should link to. (SAVED in World)
	RES_SERVERS,		// List a number of servers in 3 line format. (Phase this out)
	RES_SKILL,			// Define attributes for a skill (how fast it raises etc)
	RES_SKILLCLASS,		// Define specifics for a char with this skill class. (ex. skill caps)
	RES_SKILLMENU,		// A menu that is attached to a skill. special arguments over other menus.
	RES_SPAWN,			// Define a list of NPC's and how often they may spawn.
	RES_SPEECH,			// A speech block with ON=*blah* in it.
	RES_SPELL,			// Define a magic spell. (0-64 are reserved)
	RES_SPHERE,			// Main Server INI block
	RES_STARTS,			// (SI) List of starting locations for newbies.
	RES_STAT,			// Stats elements like KARMA,STR,DEX,FOOD,FAME,CRIMINAL etc. Used for resource and desire scripts.
	RES_TELEPORTERS,	// (SL) Where are the teleporters in the world ? dungeon transports etc.
	RES_TEMPLATE,		// Define lists of items. (for filling loot etc)
	RES_TIP,			// Tips (similar to RES_SCROLL) that can come up at startup.
	RES_TYPEDEF,			// Define a trigger block for a RES_WORLDITEM m_type.
	RES_TYPEDEFS,
	RES_WEBPAGE,		// Define a web page template.
	RES_WORLDCHAR,		// Define instance of char in the world. (SAVED in World)
	RES_WORLDITEM,		// Define instance of item in the world. (SAVED in World)
	RES_WORLDSCRIPT,	// Something to load into a script.
	RES_WORLDVARS,
	RES_QTY,			// Don't care
};

#define RES_DIALOG_TEXT				1	// sub page for the section.
#define RES_DIALOG_BUTTON			2
#define RES_NEWBIE_MALE_DEFAULT		(10000+1)	// just an unused number for the range.
#define RES_NEWBIE_FEMALE_DEFAULT	(10000+2)

struct RESOURCE_ID_BASE : public CGrayUIDBase
{
#define RES_TYPE_SHIFT	25	// leave 6 bits = 64 for RES_TYPE
#define RES_TYPE_MASK	63
#define RES_PAGE_SHIFT	18	// leave 7 bits = 128 pages of space.
#define RES_PAGE_MASK	127
#define RES_INDEX_SHIFT	0	// leave 18 bits = 262144 entries.
#define RES_INDEX_MASK	0x3FFFF

public:
#define RES_GET_TYPE(dw)	( ( dw >> RES_TYPE_SHIFT) & RES_TYPE_MASK )
	RES_TYPE GetResType() const
	{
		DWORD dwVal = RES_GET_TYPE(m_dwInternalVal);
		return( (RES_TYPE) dwVal );
	}
#define RES_GET_INDEX(dw)	((dw)&RES_INDEX_MASK)
	int GetResIndex() const
	{
		return ( RES_GET_INDEX(m_dwInternalVal) );
	}
	int GetResPage() const
	{
		DWORD dwVal = m_dwInternalVal >> RES_PAGE_SHIFT;
		dwVal &= RES_PAGE_MASK;
		return(dwVal);
	}
	bool operator == ( const RESOURCE_ID_BASE & rid ) const
	{
		return( rid.m_dwInternalVal == m_dwInternalVal );
	}
};

struct RESOURCE_ID : public RESOURCE_ID_BASE
{
	RESOURCE_ID()
	{
		InitUID();
	}
	RESOURCE_ID( RES_TYPE restype )
	{
		// single instance type.
		m_dwInternalVal = UID_F_RESOURCE|((restype)<<RES_TYPE_SHIFT);
	}
	RESOURCE_ID( RES_TYPE restype, int index )
	{
		ASSERT( index < RES_INDEX_MASK );
		m_dwInternalVal = UID_F_RESOURCE|((restype)<<RES_TYPE_SHIFT)|(index);
	}
	RESOURCE_ID( RES_TYPE restype, int index, int iPage )
	{
		ASSERT( index < RES_INDEX_MASK );
		ASSERT( iPage < RES_PAGE_MASK );
		m_dwInternalVal = UID_F_RESOURCE|((restype)<<RES_TYPE_SHIFT)|((iPage)<<RES_PAGE_SHIFT)|(index);
	}
	RESOURCE_ID_BASE & operator = ( const RESOURCE_ID_BASE & rid )
	{
		ASSERT( rid.IsValidUID());
		ASSERT( rid.IsResource());
		m_dwInternalVal = rid.GetPrivateUID();
		return( *this );
	}
};

// Desguise an id as a pointer.
#ifndef MAKEINTRESOURCE
#define MAKEINTRESOURCE(id) ((LPCTSTR)((DWORD)((WORD)(id))))
#endif
#define ISINTRESOURCE(p)	(!(((DWORD)p)&0xFFFFF000))
#define GETINTRESOURCE(p)	(((DWORD)p)&0x0FFF)

//*********************************************************

struct CResourceQty
{
private:
	RESOURCE_ID m_rid;	// A RES_SKILL, RES_ITEMDEF, or RES_TYPEDEF
	WORD m_iQty;			// How much of this ?	amount is a WORD
public:
	RESOURCE_ID GetResourceID() const
	{
		return( m_rid );
	}
	void SetResourceID( RESOURCE_ID rid, WORD iQty )
	{
		m_rid = rid;
		m_iQty = iQty;
	}
	RES_TYPE GetResType() const
	{
		return( m_rid.GetResType());
	}
	int GetResIndex() const
	{
		return( m_rid.GetResIndex());
	}
	WORD GetResQty() const
	{
		return( m_iQty );
	}
	void SetResQty( WORD wQty )
	{
		m_iQty = wQty;
	}

	inline bool Load( LPSTR & arg )
	{
		return Load( (LPCTSTR&)arg );
	}

	bool Load( LPCTSTR & pszCmds );
	int WriteKey( TCHAR * pszArgs, bool fQtyOnly = false, bool fKeyOnly = false ) const;
	int WriteNameSingle( TCHAR * pszArgs ) const;
};

class CResourceQtyArray : public CGTypedArray<CResourceQty, CResourceQty&>
{
	// Define a list of index id's (not references) to resource objects. (Not owned by the list)
public:
	CResourceQtyArray()
	{
	}
	CResourceQtyArray( LPCTSTR pszCmds )
	{
		Load( pszCmds );
	}
	bool operator == ( const CResourceQtyArray & array ) const;
	int Load( LPCTSTR pszCmds );
	void WriteKeys( TCHAR * pszArgs, int index = 0, bool fQtyOnly = false, bool fKeyOnly = false ) const;
	void WriteNames( TCHAR * pszArgs, int index = 0 ) const;

	int FindResourceID( RESOURCE_ID_BASE rid ) const;
	int FindResourceType( RES_TYPE type ) const;
	int FindResourceMatch( CObjBase * pObj ) const;
	bool IsResourceMatchAll( CChar * pChar, DWORD dwArg = 0 ) const;
};

//*********************************************************

class CScriptFileContext
{
	// Track a temporary context into a script.
	// NOTE: This should ONLY be stack based !
private:
	bool m_fOpenScript;	// NULL context may be legit.
	const CScript * m_pPrvScriptContext;	// previous general context before this was opened.
private:
	void Init()
	{
		m_fOpenScript = false;
	}
public:
	void OpenScript( const CScript * pScriptContext );
	void Close();
	CScriptFileContext()
	{
		Init();
	}
	CScriptFileContext( const CScript * pScriptContext )
	{
		Init();
		OpenScript( pScriptContext );
	}
	~CScriptFileContext()
	{
		Close();
	}
};

class CScriptObjectContext
{
	// Track a temporary context of an object.
	// NOTE: This should ONLY be stack based !
private:
	bool m_fOpenObject;	// NULL context may be legit.
	const CScriptObj * m_pPrvObjectContext;	// previous general context before this was opened.
private:
	void Init()
	{
		m_fOpenObject = false;
	}
public:
	void OpenObject( const CScriptObj * pObjectContext );
	void Close();
	CScriptObjectContext()
	{
		Init();
	}
	CScriptObjectContext( const CScriptObj * pObjectContext )
	{
		Init();
		OpenObject( pObjectContext );
	}
	~CScriptObjectContext()
	{
		Close();
	}
};

//*********************************************************

class CResourceScript : public CScript, public CMemDynamic
{
	// A script file containing resource, speech, motives or events handlers.
	// NOTE: we should check periodically if this file has been altered externally ?

protected:
	DECLARE_MEM_DYNAMIC;
private:
	int		m_iLinkCount;		// How many CResourceLink(s) use this ?
	int		m_iOpenCount;		// How many CResourceLock(s) have this open ?
	CServTime m_timeLastAccess;	// CWorld time of last access/Open.

	// Last time it was closed. What did the file params look like ?
	DWORD m_dwSize;			// Compare to see if this has changed.
	CGTime m_dateChange;	// real world time/date of last change.

private:
	void Init()
	{
		m_iLinkCount = 0;
		m_iOpenCount = 0;
		m_timeLastAccess.Init();
		m_dwSize = (DWORD) -1;			// Compare to see if this has changed.
	}
	bool CheckForChange();

public:
	void IncLinkRef()
	{
		m_iLinkCount++;
	}
	void DecLinkRef()
	{
		m_iLinkCount--;
	}
	void ClearLinkRefCount()
	{
		// If we are resyncing this file.
		m_iLinkCount = 0;
	}
	int GetLinkRefCount() const
	{
		return m_iLinkCount;
	}

	CResourceScript( LPCTSTR pszFileName )
	{
		Init();
		SetFilePath( pszFileName );
	}
	CResourceScript()
	{
		Init();
	}

	bool IsFirstCheck() const
	{
		return( m_dwSize == -1 && ! m_dateChange.IsTimeValid());
	}

	void ReSync();
	void CheckCloseUnused( bool fForceNow );
	bool Open( LPCTSTR pszFilename = NULL, UINT wFlags = OF_READ );
	virtual void Close();
	virtual void CloseForce();
};

class CResourceLock : public CScript
{
	// Open a copy of a scipt that is already open
	// NOTE: This should ONLY be stack based !
	// preserve the previous openers offset in the script.
private:
	CResourceScript * m_pLock;
	CScriptLineContext m_PrvLockContext;	// i must return the locked file back here.	

	CScriptFileContext m_PrvScriptContext;	// where was i before (context wise) opening this. (for error tracking)
	CScriptObjectContext m_PrvObjectContext; // object context (for error tracking)
private:
	void Init()
	{
		m_pLock = NULL;
		m_PrvLockContext.Init();	// means the script was NOT open when we started.
	}

protected:
	virtual bool OpenBase( void * pExtra );
	virtual void CloseBase();
	virtual bool ReadTextLine( bool fRemoveBlanks );

public:
	CResourceLock()
	{
		Init();
	}
	~CResourceLock()
	{
		Close();
	}
	int OpenLock( CResourceScript * pLock, CScriptLineContext context );
	void AttachObj( const CScriptObj * pObj )
	{
		m_PrvObjectContext.OpenObject(pObj);
	}
};

//****************************************************
#ifdef GRAY_MAP
class CResourceDef : public CScriptObj
{
#define RESOURCE_ID DWORD
	// dummy this out.
public:
	DWORD m_RegionType;	// REGION_TYPE_AREA
	CGString m_sDefName;
public:
	CResourceDef( DWORD rid )
	{
		m_RegionType = rid;
	}
	DWORD GetResourceID() const
	{
		// REGION_TYPE_AREA
		return( m_RegionType );
	}
	bool IsMatchType( DWORD dwID ) const
	{
		return(( m_RegionType & dwID ) ? true : false );
	}

	LPCTSTR GetResourceName() const
	{
		return( m_sDefName );
	}
	bool SetResourceName( LPCTSTR psDefName )
	{
		m_sDefName = psDefName;
		return( true );
	}

	bool	HasResourceName();
	bool	MakeResourceName();
};
#endif



class CResourceDef : public CScriptObj, public CMemDynamic
{
	// Define a generic  resource block in the scripts.
	// Now the scripts can be modular. resources can be defined any place.
	// NOTE: This may be loaded fully into memory or just an Index to a file.
protected:
	DECLARE_MEM_DYNAMIC;
private:
	RESOURCE_ID m_rid;		// the true resource id. (must be unique for the RES_TYPE)
protected:
	const CVarDefNum * m_pDefName;	// The name of the resource. (optional)
public:
	// virtual RES_TYPE GetResourceType() const = 0;
public:
	CResourceDef( RESOURCE_ID rid, LPCTSTR pszDefName ) :
		m_rid( rid ),
		m_pDefName( NULL )
	{
		DEBUG_CHECK( rid.IsValidUID());
		SetResourceName( pszDefName );
	}
	CResourceDef( RESOURCE_ID rid, const CVarDefNum * pDefName = NULL ) :
		m_rid( rid ),
		m_pDefName( pDefName )
	{
		DEBUG_CHECK( rid.IsValidUID());
	}
	virtual ~CResourceDef()	// need a virtual for the dynamic_cast to work.
	{
		// ?? Attempt to remove m_pDefName ?
	}
	void SetResourceID( RESOURCE_ID_BASE rid )
	{
		// This is not a normal thing to do.
		ASSERT( rid.IsValidUID());
		m_rid = rid;
	}
	RESOURCE_ID GetResourceID() const
	{
		return( m_rid );
	}
	RES_TYPE GetResType() const
	{
		DEBUG_CHECK( m_rid.IsResource());
		return( m_rid.GetResType() );
	}
	int GetResPage() const
	{
		DEBUG_CHECK( m_rid.IsResource());
		return( m_rid.GetResPage());
	}

	void CopyDef( const CResourceDef * pLink )
	{
		DEBUG_CHECK( m_rid == pLink->m_rid );
		m_pDefName = pLink->m_pDefName;
	}

	// Get the name of the resource item. (Used for saving) may be number or name
	LPCTSTR GetResourceName() const;
	virtual LPCTSTR GetName() const	// default to same as the DEFNAME name.
	{
		return( GetResourceName());
	}

	// Give it another DEFNAME= even if it already has one. it's ok to have multiple names.
	bool SetResourceName( LPCTSTR pszName );
	void SetResourceVar( const CVarDefNum* pVarNum )
	{
		if ( pVarNum != NULL && m_pDefName == NULL )
		{
			m_pDefName = pVarNum;
		}
	}

	// unlink all this data. (tho don't delete the def as the pointer might still be used !)
	virtual void UnLink()
	{
		// This does nothing in the CResourceDef case, Only in the CResourceLink case.
	}

	bool	HasResourceName();
	bool	MakeResourceName();
};

class CResourceLink : public CResourceDef
{
	// A single resource object that also has part of itself remain in resource file.
	// A pre-indexed link into a script file.
	// This is a CResourceDef not fully read into memory at index time.
	// We are able to lock it and read it as needed

protected:
	DECLARE_MEM_DYNAMIC;
private:
	CResourceScript * m_pScript;	// we already found the script.
	CScriptLineContext m_Context;

	DWORD m_lRefInstances;	// How many CResourceRef objects refer to this ?

	DWORD m_dwOnTriggers1;	// a mask of known ON=triggers supported here. (diff for each RES_TYPE)
	DWORD m_dwOnTriggers2;	// a mask of known ON=triggers supported here. (diff for each RES_TYPE)
	DWORD m_dwOnTriggers3;

#define XTRIG_UNKNOWN 0	// bit 0 is reserved to say there are triggers here that do not conform.

private:
	void SetBadLinkOffset( int iCode )
	{
		// just set the offset to an error code.
		ASSERT( iCode < -1 );
		m_Context.m_lOffset = iCode;
	}

public:

	void AddRefInstance() { m_lRefInstances ++; }
	void DelRefInstance() { m_lRefInstances --; }
	int GetRefInstances() const { return( m_lRefInstances ); }

	bool IsLinked() const	// been loaded from the scripts ?
	{
		if ( m_pScript == NULL )
			return( false );
		return( m_Context.IsValid());
	}

	CResourceScript * GetLinkFile() const
	{
		return( m_pScript );
	}
	long GetLinkOffset() const
	{
		return( m_Context.m_lOffset );
	}

	void SetLink( CResourceScript * pScript )
	{
		DEBUG_CHECK(pScript);
		m_pScript = pScript;
		m_pScript->IncLinkRef();
		m_Context = pScript->GetContext();
	}
    void CopyTransfer( CResourceLink * pLink )
    {
		ASSERT(pLink);
		CResourceDef::CopyDef( pLink );
		m_pScript = pLink->m_pScript;
		m_pScript->IncLinkRef();
		DEBUG_CHECK(m_pScript);
		m_Context = pLink->m_Context;
		m_dwOnTriggers1 = pLink->m_dwOnTriggers1;
		m_dwOnTriggers2 = pLink->m_dwOnTriggers2;
		m_dwOnTriggers3 = pLink->m_dwOnTriggers3;
		m_lRefInstances = pLink->m_lRefInstances;
		pLink->m_lRefInstances = 0;	// instance has been transfered.
	}

	void ScanSection( RES_TYPE restype );

	void ClearTriggers()
	{
		m_dwOnTriggers1 = 0;
		m_dwOnTriggers2 = 0;
		m_dwOnTriggers3 = 0;
	}
	void SetTrigger( int i )
	{
		ASSERT(i>=0);
		if ( i < 32 )
			m_dwOnTriggers1 |= ( 1 << i );
		else
		{
			i -= 32;
			if ( i < 32 )
				m_dwOnTriggers2 |= ( 1 << i );
			else
			{
				i -= 32;
				m_dwOnTriggers3 |= ( 1 << i );
			}
		}
	}
	bool HasTrigger( int i ) const
	{
		// Specific to the RES_TYPE;
		// CTRIG_QTY, ITRIG_QTY or RTRIG_QTY
		if (i<=XTRIG_UNKNOWN)	// i can't record these so just assume we have to go looking.
		{
			i = XTRIG_UNKNOWN;
		}
		if ( i < 32 )
			return(( m_dwOnTriggers1 & ( 1 << i )) ? true : false );
		i -= 32;
		if ( i < 32 )
			return(( m_dwOnTriggers2 & ( 1 << i )) ? true : false );
		i -= 32;
		return(( m_dwOnTriggers3 & ( 1 << i )) ? true : false );		
	}

	bool ResourceLock( CResourceLock & s );
	virtual ~CResourceLink();
	CResourceLink( RESOURCE_ID rid, const CVarDefNum * pDef = NULL );
};

class CResourceNamed : public CResourceLink
{
	// Private name pool. (does not use DEFNAME)
	// RES_FUNCTION or RES_HELP
protected:
	DECLARE_MEM_DYNAMIC;
public:
	const CGString m_sName;
public:
	CResourceNamed( RESOURCE_ID rid, LPCTSTR pszName ) :
		CResourceLink( rid ),
		m_sName( pszName )
	{
	}
	virtual ~CResourceNamed()
	{
	}
	LPCTSTR GetName() const
	{
		return( m_sName );
	}
};

//***********************************************************

class CResourceRef
{
private:
	CResourceLink* m_pLink;
public:
	CResourceRef( CResourceLink* pLink ) :
		m_pLink(pLink)
	{
		ASSERT(pLink);
		pLink->AddRefInstance();
	}
	CResourceRef()
	{
		m_pLink = NULL;
	}
	~CResourceRef()
	{
		if ( m_pLink )
		{
			m_pLink->DelRefInstance();
		}
	}
	CResourceLink* GetRef() const
	{
		return(m_pLink);
	}
	void SetRef( CResourceLink* pLink )
	{
		if ( m_pLink != NULL )
		{
			m_pLink->DelRefInstance();
		}
		m_pLink = pLink;
		if (pLink)
		{
			pLink->AddRefInstance();
		}
	}
	operator CResourceLink*() const
    {
		return( GetRef());
    }
};

class CResourceRefArray : public CGPtrTypeArray<CResourceRef>
{
	// Define a list of pointer references to resource. (Not owned by the list)
	// An indexed list of CResourceLink s.
private:
	LPCTSTR GetResourceName( int iIndex ) const
	{
		// look up the name of the fragment given it's index.
		CResourceLink * pResourceLink = GetAt( iIndex );
		ASSERT(pResourceLink);
		return( pResourceLink->GetResourceName());
	}
public:
	int FindResourceType( RES_TYPE type ) const;
	int FindResourceID( RESOURCE_ID_BASE rid ) const;
	int FindResourceName( RES_TYPE restype, LPCTSTR pszKey ) const;

	void WriteResourceRefList( CGString & sVal ) const;
	bool r_LoadVal( CScript & s, RES_TYPE restype );
	void r_Write( CScript & s, LPCTSTR pszKey ) const;
};

//*********************************************************

class CResourceHashArray : public CGObSortArray< CResourceDef*, RESOURCE_ID_BASE >
{
	// This list OWNS the CResourceDef and CResourceLink objects.
	// Sorted array of RESOURCE_ID
public:
	int CompareKey( RESOURCE_ID_BASE rid, CResourceDef * pBase, bool fNoSpaces ) const
	{
		DWORD dwID1 = rid.GetPrivateUID();
		ASSERT( pBase );
		DWORD dwID2 = pBase->GetResourceID().GetPrivateUID();
		if (dwID1 > dwID2 )
			return(1);
		if (dwID1 == dwID2 )
			return(0);
		return(-1);
	}
};

class CResourceHash
{
public:
	CResourceHashArray m_Array[16];
private:
	int GetHashArray( RESOURCE_ID_BASE rid ) const
	{
		return( rid.GetResIndex() & 0x0F );
	}
public:
	int FindKey( RESOURCE_ID_BASE rid ) const
	{
		return( m_Array[ GetHashArray( rid ) ].FindKey(rid));
	}
	CResourceDef* GetAt( RESOURCE_ID_BASE rid, int index ) const
	{
		return( m_Array[ GetHashArray( rid ) ].GetAt(index));
	}
	int AddSortKey( RESOURCE_ID_BASE rid, CResourceDef* pNew )
	{
		return( m_Array[ GetHashArray( rid ) ].AddSortKey( pNew, rid ));
	}
	void SetAt( RESOURCE_ID_BASE rid, int index, CResourceDef* pNew )
	{
		m_Array[ GetHashArray( rid ) ].SetAt( index, pNew );
	}
};

//*************************************************

struct CStringSortArray : public CGObSortArray< TCHAR*, TCHAR* >
{
	// Sorted array of strings
	int CompareKey( TCHAR* pszID1, TCHAR* pszID2, bool fNoSpaces ) const
	{
		ASSERT( pszID2 );
		return( strcmpi( pszID1, pszID2));
	}

	void AddSortString( LPCTSTR pszText )
	{
		ASSERT(pszText);
		int len = strlen( pszText );
		TCHAR * pNew = new TCHAR [ len + 1 ];
		strcpy( pNew, pszText );
		AddSortKey( pNew, pNew );
	}
};

class CObNameSortArray : public CGObSortArray< CScriptObj*, LPCTSTR >
{
	// Array of CScriptObj. name sorted.
	int CompareKey( LPCTSTR pszID, CScriptObj* pObj, bool fNoSpaces ) const
	{
		ASSERT( pszID );
		ASSERT( pObj );
		if ( fNoSpaces )
		{
			char *	p	= strchr( pszID, ' ' );
			if ( p )
			{
				int		iLen	= p - pszID;
				return( strnicmp( pszID, pObj->GetName(), iLen ) );
			}
		}
		return( strcmpi( pszID, pObj->GetName()));
	}
};

//***************************************************************8

class CResourceBase : public CScriptObj
{
protected:
	static LPCTSTR const sm_szResourceBlocks[RES_QTY];

	CGObArray< CResourceScript* > m_ResourceFiles;	// All resource files we need to get blocks from later.

public:
	CResourceHash m_ResHash;		// All script linked resources RES_QTY

	// INI File options.
	CGString m_sSCPBaseDir;		// if we want to get *.SCP files from elsewhere.
	bool m_fWriteNumIDs;		// don't write resource names to the save. Just use id numbers.

protected:
	CResourceScript * AddResourceFile( LPCTSTR pszName );
	void AddResourceDir( LPCTSTR pszDirName );

public:
	void LoadResourcesOpen( CScript * pScript );
	bool LoadResources( CResourceScript * pScript );

	static LPCTSTR GetResourceBlockName( RES_TYPE restype );

	LPCTSTR GetName() const
	{
		return( _TEXT("CFG"));
	}

	CResourceScript* GetResourceFile( int i )
	{
		if ( i>=m_ResourceFiles.GetCount())
		{
			return( NULL );	// All resource files we need to get blocks from later.
		}
		return( m_ResourceFiles[i] );
	}

	RESOURCE_ID ResourceGetID( RES_TYPE restype, LPCTSTR & pszName );
	RESOURCE_ID ResourceGetIDType( RES_TYPE restype, LPCTSTR pszName )
	{
		// Get a resource of just this index type.
		RESOURCE_ID rid = ResourceGetID( restype, pszName );
		if ( rid.GetResType() != restype )
		{
			rid.InitUID();
			return( rid );
		}
		return( rid );
	}
	int ResourceGetIndex( RES_TYPE restype, LPCTSTR pszName );
	int ResourceGetIndexType( RES_TYPE restype, LPCTSTR pszName );

	LPCTSTR ResourceGetName( RESOURCE_ID_BASE rid ) const;

	CScriptObj * ResourceGetDefByName( RES_TYPE restype, LPCTSTR pszName )
	{
		// resolve a name to the actual resource def.
		return( ResourceGetDef( ResourceGetID( restype, pszName )));
	}

	bool ResourceLock( CResourceLock & s, RESOURCE_ID_BASE rid );
	bool ResourceLock( CResourceLock & s, RES_TYPE restype, LPCTSTR pszName )
	{
		return ResourceLock( s, ResourceGetIDType( restype, pszName ));
	}

	CResourceScript * FindResourceFile( LPCTSTR pszTitle );
	CResourceScript * LoadResourcesAdd( LPCTSTR pszNewName );
	
	virtual CResourceDef * ResourceGetDef( RESOURCE_ID_BASE rid ) const;
	virtual bool OpenResourceFind( CScript &s, LPCTSTR pszFilename );
	virtual bool LoadResourceSection( CScript * pScript ) = 0;
};

inline LPCTSTR CResourceBase::GetResourceBlockName( RES_TYPE restype )	// static
{
	if ( restype < 0 || restype >= RES_QTY )
		restype = RES_UNKNOWN;
	return( sm_szResourceBlocks[restype] );
}

extern int FindResourceID( int id, const int * pID, int iCount );

#endif // _INC_CResourceBase_H
