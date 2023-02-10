//
// CResourceDef.h
// Copyright Menace Software (www.menasoft.com).
//

#ifdef GRAY_SVR
#include "../graysvr/graysvr.h"
#elif defined(GRAY_MAP)
#include "../graymap/stdafx.h"
#include "../graymap/graymap.h"
#elif defined(GRAY_AGENT)
#include "../grayagent/stdafx.h"
#include "../grayagent/grayagent.h"
#elif defined(GRAY_MAKER)
#include "../graymaker/stdafx.h"
#include "../graymaker/graymaker.h"
#else
#include "graycom.h"
#endif
#include "CFileList.h"
#include "CResourceBase.h"

int FindResourceID( int id, const int * pID, int iCount )
{
	for ( int i=0; i < iCount; i++ )
	{
		if ( pID[i] == id )
			return( i );
	}
	return( -1 );
}

//***************************************************
// CResourceBase

LPCTSTR const CResourceBase::sm_szResourceBlocks[RES_QTY] =	// static
{
	"AAAUNUSED",	// unused / unknown.
    "ACCOUNT",		// Define an account instance.
	"ADVANCE",		// Define the advance rates for stats.
	"AREA",			// Complex region. (w/ extra tags)
	"BLOCKEMAIL",		// (SL) A list of email addresses we will not accept.
	"BLOCKIP",		// (SL) A list of IP's to block.
	"BOOK",			// A book or a page from a book.
	"CHARDEF",		// Define a char type.
	"COMMENT",		// A commented out block type.
	"CRYSTALBALL",	// (SL) Random (short) tip type messages.
	"DEFNAME",		// (SL) Just add a bunch of new defs and equivs str/values.
	"DIALOG",			// A scriptable gump dialog", text or handler block.
	"ECONOMY",		// define the prices of a bunch of stuff.
	"EMAILMSG",		// define an email msg that could be sent to an account.
	"EVENTS",			// (SL) Preload these Event files.
	"FUNCTION",		// Define a new command verb script that applies to a char.
	"GMPAGE",			// A GM page. (SAVED in World)
	"HELP",			// Command help blocks. (read in as needed)
	"ITEMDEF",		// Define an item type
	"LOCATION",		// Ignore this AXIS data.
	"MAP",			// Define info about mapx.mul files.
	"MENU",			// General scriptable menus.
	"MOONGATES",		// (SL) Define where the moongates are.
	"NAMES",		// A block of possible names for a NPC type. (read as needed)
	"NEWBIE",			// Triggers to execute on Player creation (based on skills selected)
	"NOTOTITLES",		// (SI) Define the noto titles used.
	"OBSCENE",		// (SL) A list of obscene words.
	"PLEVEL",			// Define the list of commands that a PLEVEL can access. (or not access)
	"QUEST",			// ability to get accomplishment titles.
	"RACECLASS",		// General race params about creature types. regen rates, etc
	"REGIONRESOURCE",	// Define Ore types.
	"REGIONTYPE",			// Triggers etc. that can be assinged to a "AREA
	"RESOURCES",		// (SL) list of all the resource files we should index !
	"ROOM",			// Non-complex region. (no extra tags)
	"RUNES",			// (SI) Define list of the magic runes.
	"SCHEDULE",			// A list of times things can happen.
	"SCROLL",			// SCROLL_GUEST=message scroll sent to player at guest login. SCROLL_MOTD", SCROLL_NEWBIE
	"SECTOR",			// Make changes to a sector. (SAVED in World)
	"SERVER",			// Define a peer sphere server we should link to. (SAVED in World)
	"SERVERS",		// List a number of servers in 3 line format.
	"SKILL",			// Define attributes for a skill (how fast it raises etc)
	"SKILLCLASS",		// Define class specifics for a char with this skill class.
	"SKILLMENU",		// A menu that is attached to a skill. special arguments over other menus.
	"SPAWN",			// Define a list of NPC's and how often they may spawn.
	"SPEECH",			// (SL) Preload these speech files.
	"SPELL",			// Define a magic spell. (0-64 are reserved)
	"SPHERE",			// Main Server INI block
	"STARTS",			// (SI) List of starting locations for newbies.
	"STAT",			// Stats elements like KARMA,STR,DEX,FOOD,FAME,CRIMINAL etc. Used for resource and desire scripts.
	"TELEPORTERS",	// (SL) Where are the teleporteres in the world ?
	"TEMPLATE",		// Define a list of items. (for filling loot etc)
	"TIP",			// Tips that can come up at startup.
	"TYPEDEF",			// Define a trigger block for a "WORLDITEM m_type.
	"TYPEDEFS",
	"WEBPAGE",		// Define a web page template.
	"WORLDCHAR",		// Define instance of char in the world. (SAVED in World)
	"WORLDITEM",		// Define instance of item in the world. (SAVED in World)
	"WORLDSCRIPT",		// Define instance of item in the world. (SAVED in World)
	"WORLDVARS",		// block of global variables
};


//*********************************************************
// Resource Files 

CResourceScript * CResourceBase::FindResourceFile( LPCTSTR pszPath )
{
	// Just match the titles ( not the whole path)

	LPCTSTR pszTitle = CScript::GetFilesTitle( pszPath );

	for ( int i=0; true; i++ )
	{
		CResourceScript * pResFile = GetResourceFile(i);
		if ( pResFile == NULL )
			break;
		LPCTSTR pszTitle2 = pResFile->GetFileTitle();
		if ( ! strcmpi( pszTitle2, pszTitle ))
			return( pResFile );
	}
	return( NULL );
}

CResourceScript * CResourceBase::AddResourceFile( LPCTSTR pszName )
{
	// Is this really just a dir name ?

	LPCTSTR pszTitle = CScript::GetFilesTitle( pszName );
	if ( pszTitle[0] == '\0' )
	{
		AddResourceDir( pszName );
		return NULL;
	}

	if ( ! strnicmp( pszTitle, GRAY_FILE "tables", strlen(GRAY_FILE "tables")))
	{
		// Don't dupe this.
		return NULL;
	}

	// Try to prevent dupes
	CResourceScript * pNewRes = FindResourceFile(pszTitle);
	if ( pNewRes )
		return( pNewRes );

	// Find correct path
	CScript s;
	if ( ! OpenResourceFind( s, pszName ))
	{
		return( NULL );
	}

	pNewRes = new CResourceScript( s.GetFilePath() );
	m_ResourceFiles.Add(pNewRes);
	return( pNewRes );
}

void CResourceBase::AddResourceDir( LPCTSTR pszDirName )
{
	if ( pszDirName[0] == '\0' )
		return;

	CGString sFilePath = CGFile::GetMergedFileName( pszDirName, "*" GRAY_SCRIPT );

	CFileList filelist;
	int iRet = filelist.ReadDir( sFilePath );
	if ( iRet < 0 )
	{
		DEBUG_ERR(( "DirList=%d for '%s'\n", iRet, (LPCTSTR) pszDirName ));
		return;
	}
	if ( iRet <= 0 )	// no files here.
	{
		return;
	}

	CGStringListRec * psFile = filelist.GetHead();
	for ( ; psFile; psFile = psFile->GetNext())
	{
		sFilePath = CGFile::GetMergedFileName( pszDirName, *psFile );
		AddResourceFile( sFilePath );
	}
}

void CResourceBase::LoadResourcesOpen( CScript * pScript )
{
	// Load an already open resource file.

	ASSERT(pScript);
	ASSERT( pScript->IsFileOpen());

	// 	pScript->ClearLinkRefCount();

	int iSections = 0;
	while ( pScript->FindNextSection())
	{
		LoadResourceSection( pScript );
		iSections ++;
	}

	if ( ! iSections )
	{
		DEBUG_WARN(( "No resource sections in '%s'\n", pScript->GetFilePath()));
	}
}

bool CResourceBase::LoadResources( CResourceScript * pScript )
{
	// Open the file then load it.
	if ( pScript == NULL )
		return( false );

	if ( ! pScript->Open())
	{
		g_pLog->Event( LOGL_CRIT|LOGM_INIT, "[RESOURCES] '%s' not found...\n", (LPCTSTR) pScript->GetFilePath());
		return( false );
	}

	g_pLog->Event(LOGM_INIT, "Loading file %s\n", (LPCTSTR) pScript->GetFilePath());

	LoadResourcesOpen( pScript );
	pScript->Close();
	pScript->CloseForce();
	return( true );
}

CResourceScript * CResourceBase::LoadResourcesAdd( LPCTSTR pszNewFileName )
{
	// Make sure this is added to my list of resource files
	// And load it now.

	CResourceScript * pScript = AddResourceFile( pszNewFileName );
	if ( ! LoadResources(pScript))
		return( NULL );
	return( pScript );
}

bool CResourceBase::OpenResourceFind( CScript &s, LPCTSTR pszFilename )
{
	// Open a single resource script file.
	// Look in the specified path.

	if ( pszFilename == NULL )
	{
		pszFilename = s.GetFilePath();
	}

	// search the local dir or full path first.
	CGString sStartName = pszFilename;
	if ( s.Open( sStartName, OF_READ | OF_NONCRIT ))
		return( true );

	// search the script file path.
	LPCTSTR pszTitle = CGFile::GetFilesTitle( sStartName );
	CGString sPathName = CGFile::GetMergedFileName( m_sSCPBaseDir, pszTitle );
	return( s.Open( sPathName, OF_READ ));
}

bool CResourceBase::LoadResourceSection( CScript * pScript )
{
	// Just stub this out for others for now.
	ASSERT(pScript);
	return( false );
}

//*********************************************************
// Resource Block Definitions

LPCTSTR CResourceBase::ResourceGetName( RESOURCE_ID_BASE rid ) const
{
	// Get a portable name for the resource id type.

	if ( ! m_fWriteNumIDs )
	{
		CResourceDef * pResourceDef = dynamic_cast <CResourceDef *>( ResourceGetDef( rid ));
		if ( pResourceDef )
		{
			return( pResourceDef->GetResourceName());
		}
	}

	TCHAR * pszTmp = Str_GetTemp();
	ASSERT(pszTmp);
	if ( ! rid.IsValidUID())
	{
		sprintf( pszTmp, _TEXT("%d"), rid.GetPrivateUID() );
	}
	else
	{
		sprintf( pszTmp, _TEXT("0%x"), rid.GetResIndex() );
	}
	return( pszTmp );
}

RESOURCE_ID CResourceBase::ResourceGetID( RES_TYPE restype, LPCTSTR & pszName )
{
	// Find the Resource ID given this name.
	// We are NOT creating a new resource. just looking up an existing one
	// NOTE: Do not enforce the restype.
	//		Just fill it in if we are not sure what the type is.
	// NOTE: 
	//  Some restype's have private name spaces. (ie. RES_AREA and RES_SERVER)
	// RETURN:
	//  pszName is now set to be after the expression.

	// We are NOT creating.
	RESOURCE_ID rid;

	// Try to handle private name spaces.
	switch ( restype )
	{
	case RES_ACCOUNT:
	case RES_AREA:
	case RES_GMPAGE:
	case RES_ROOM:
	case RES_SERVER:
	case RES_SECTOR:
		break;
	}

	rid.SetPrivateUID( Exp_GetVal(pszName));	// May be some complex expression {}

	if ( restype != RES_UNKNOWN && rid.GetResType() == RES_UNKNOWN )
	{
		// Label it with the type we want.
		return RESOURCE_ID( restype, rid.GetResIndex());
	}

	return( rid );
}

int CResourceBase::ResourceGetIndex( RES_TYPE restype, LPCTSTR pszName )
{
	// Get a resource but do not enforce the type.
	RESOURCE_ID rid = ResourceGetID( restype, pszName );
	if ( ! rid.IsValidUID())
		return( -1 );
	return( rid.GetResIndex());
}

int CResourceBase::ResourceGetIndexType( RES_TYPE restype, LPCTSTR pszName )
{
	// Get a resource of just this index type.
	RESOURCE_ID rid = ResourceGetID( restype, pszName );
	if ( rid.GetResType() != restype )
		return( -1 );
	return( rid.GetResIndex());
}

CResourceDef * CResourceBase::ResourceGetDef( RESOURCE_ID_BASE rid ) const
{
	if ( ! rid.IsValidUID())
		return( NULL );
	int	index = m_ResHash.FindKey( rid );
	if ( index < 0 )
		return( NULL );
	return( m_ResHash.GetAt( rid, index ));
}

//*******************************************************
// Open resource blocks.

bool CResourceBase::ResourceLock( CResourceLock & s, RESOURCE_ID_BASE rid )
{
	// Lock a referenced resource object.
	if ( ! rid.IsValidUID())
		return( false );
	CResourceLink * pResourceLink = dynamic_cast <CResourceLink *>( ResourceGetDef( rid ));
	if ( pResourceLink )
	{
		return( pResourceLink->ResourceLock(s));
	}
	return( false );
}

/////////////////////////////////////////////////
// -CResourceDef

bool CResourceDef::SetResourceName( LPCTSTR pszName )
{
	ASSERT(pszName);

	// This is the global def for this item.
	for ( int i=0; pszName[i]; i++ )
	{
		if ( i>=EXPRESSION_MAX_KEY_LEN )
		{
			DEBUG_ERR(( "Too long DEFNAME=%s\n", pszName ));
			return( false );
		}
		if ( ! _ISCSYM(pszName[i]))
		{
			DEBUG_ERR(( "Bad chars in DEFNAME=%s\n", pszName ));
			return( false );
		}
	}

	int iVarNum;

	CVarDefBase * pVarKey = g_Exp.m_VarDefs.GetKey( pszName );
	if ( pVarKey )
	{
		if ( pVarKey->GetValNum() == GetResourceID().GetPrivateUID() )
		{
			return( true );
		}

		if ( RES_GET_INDEX(pVarKey->GetValNum()) == GetResourceID().GetResIndex())
		{
			DEBUG_WARN(( "The DEFNAME=%s has a strange type mismatch? 0%x!=0%x\n", pszName, pVarKey->GetValNum(), GetResourceID().GetPrivateUID() ));
		}
		else
		{
			DEBUG_WARN(( "The DEFNAME=%s already exists! 0%x!=0%x\n", pszName, RES_GET_INDEX(pVarKey->GetValNum()), GetResourceID().GetResIndex() ));
		}

		iVarNum = g_Exp.m_VarDefs.SetNum( pszName, GetResourceID().GetPrivateUID() );
	}
	else
	{
		iVarNum = g_Exp.m_VarDefs.SetNumNew( pszName, GetResourceID().GetPrivateUID() );
	}

	if ( iVarNum < 0 )
		return( false );

	SetResourceVar( dynamic_cast <const CVarDefNum*>( g_Exp.m_VarDefs.GetAt( iVarNum )));
	return( true );
}

LPCTSTR CResourceDef::GetResourceName() const
{
	if ( m_pDefName 
#ifdef GRAY_SVR
		&& ! g_Cfg.m_fWriteNumIDs 
#endif
		)
	{
		return( m_pDefName->GetKey());
	}
	TCHAR * pszTmp = Str_GetTemp();
	sprintf( pszTmp, _TEXT("0%x"), GetResourceID().GetResIndex());
	return( pszTmp );
}


bool	CResourceDef::HasResourceName()
{
	if ( m_pDefName )
		return true;
	return false;
}


bool	CResourceDef::MakeResourceName()
{
	if ( m_pDefName )
		return true;
	LPCTSTR		pszName		= GetName();

	GETNONWHITESPACE( pszName );
	TCHAR	*pbuf = Str_GetTemp();
	TCHAR	ch;
	TCHAR *	pszDef;

	strcpy(pbuf, "a_");

	LPCTSTR		pszKey		= NULL;	// auxiliary, the key of a similar CVarDef, if any found
	pszDef	= pbuf + 2;

	for ( ; *pszName; pszName++ )
	{
		ch	= *pszName;
		if ( ch == ' ' || ch == '\t' || ch == '-' )
			ch	= '_';
		else if ( !isalnum( ch ) )
			continue;
		// collapse multiple spaces together
		if ( ch == '_' && *(pszDef-1) == '_' )
			continue;
		*pszDef	= ch;
		pszDef++;
	}
	*pszDef	= '_';
	*(++pszDef)	= '\0';

	
	int			iMax	= g_Exp.m_VarDefs.GetCount();
	int			iVar	= 1;
	int			iLen	= strlen( pbuf );

	for ( int i = 0; i < iMax; i++ )
	{
		pszKey	= g_Exp.m_VarDefs.GetAt(i)->GetKey();
		int			iLenKey	= strlen( pszKey );

		// Is this a similar key?
		if ( strnicmp( pbuf, pszKey, iLen ) )
			continue;

		// skip underscores
		pszKey	= pszKey+iLen;
		while ( *pszKey	== '_' )
			pszKey++;

		// Is this is subsequent key with a number? Get the highest (plus one)
		if ( IsStrNumericDec( pszKey ) )
		{
			int iVarThis	= atoi( pszKey );
			if ( iVarThis >= iVar )
				iVar	= iVarThis+1;
		}
		else
			iVar++;
	}

	// add an extra _, hopefully won't conflict with named areas
	sprintf( pszDef, "_%i", iVar );
	SetResourceName( pbuf );
	// Assign name
	return true;
}



bool	CRegionBase::MakeRegionName()
{
	if ( m_pDefName )
		return true;

	TCHAR		ch;
	LPCTSTR		pszKey		= NULL;	// auxiliary, the key of a similar CVarDef, if any found
	TCHAR		*pbuf = Str_GetTemp();
	TCHAR *		pszDef				= pbuf + 2;
	strcpy(pbuf, "a_");

	LPCTSTR		pszName		= GetName();
	GETNONWHITESPACE( pszName );

	if ( !strnicmp( "the ", pszName, 4 ) )
		pszName	+= 4;
	else if ( !strnicmp( "a ", pszName, 2 ) )
		pszName	+= 2;
	else if ( !strnicmp( "an ", pszName, 3 ) )
		pszName	+= 3;
	else if ( !strnicmp( "ye ", pszName, 3 ) )
		pszName	+= 3;

	for ( ; *pszName; pszName++ )
	{
		if ( !strnicmp( " of ", pszName, 4 ) || !strnicmp( " in ", pszName, 4 ) )
		{	pszName	+= 4;	continue;	}
		if ( !strnicmp( " the ", pszName, 5 )  )
		{	pszName	+= 5;	continue;	}

		ch	= *pszName;
		if ( ch == ' ' || ch == '\t' || ch == '-' )
			ch	= '_';
		else if ( !isalnum( ch ) )
			continue;
		// collapse multiple spaces together
		if ( ch == '_' && *(pszDef-1) == '_' )
			continue;
		*pszDef	= tolower(ch);
		pszDef++;
	}
	*pszDef	= '_';
	*(++pszDef)	= '\0';

	
	int			iMax	= g_Cfg.m_RegionDefs.GetCount();
	int			iVar	= 1;
	int			iLen	= strlen( pbuf );

	for ( int i = 0; i < iMax; i++ )
	{
		CRegionBase * pRegion = dynamic_cast <CRegionBase*> (g_Cfg.m_RegionDefs.GetAt(i));
		if ( !pRegion )
			continue;
		pszKey		= pRegion->GetResourceName();
		if ( !pszKey )
			continue;

		int	iLenKey	= strlen( pszKey );

		// Is this a similar key?
		if ( strnicmp( pbuf, pszKey, iLen ) )
			continue;

		// skip underscores
		pszKey	= pszKey+iLen;
		while ( *pszKey	== '_' )
			pszKey++;

		// Is this is subsequent key with a number? Get the highest (plus one)
		if ( IsStrNumericDec( pszKey ) )
		{
			int iVarThis	= atoi( pszKey );
			if ( iVarThis >= iVar )
				iVar	= iVarThis+1;
		}
		else
			iVar++;
	}

	// Only one, no need for the extra "_"
	sprintf( pszDef, "%i", iVar );
	SetResourceName( pbuf );
	// Assign name
	return true;
}

//***************************************************************************
// -CResourceScript

bool CResourceScript::CheckForChange()
{
	// Get Size/Date info on the file to see if it has changed.
	time_t dateChange;
	DWORD dwSize;

	if ( ! CFileList::ReadFileInfo( GetFilePath(), dateChange, dwSize ))
	{
		DEBUG_ERR(( "Can't get stats info for file '%s'\n", (LPCTSTR) GetFilePath() ));
		return false;
	}

	bool fChange = false;

	// See If the script has changed
	if ( ! IsFirstCheck())
	{
		if ( m_dwSize != dwSize || m_dateChange != dateChange )
		{
			g_pLog->Event( LOGL_WARN, "Resource '%s' changed, resync.\n", (LPCTSTR) GetFilePath());
			fChange = true;
		}
	}

	m_dwSize = dwSize;			// Compare to see if this has changed.

#ifdef GRAY_SVR
	m_dateChange = dateChange;	// real world time/date of last change.
#else
	(CTime)m_dateChange = dateChange;	// real world time/date of last change.
#endif

	return( fChange );
}

#ifdef GRAY_SVR

void CResourceScript::CheckCloseUnused( bool fForceNow )
{
	if ( ! IsFileOpen())
	{
		DEBUG_CHECK( ! m_iOpenCount );
		return;
	}
	if ( m_iOpenCount ) // someone is using it now!
	{
		if ( ! fForceNow )
			return;
	}
	// Has not been used for a minute or so.
	if ( fForceNow || (( CServTime::GetCurrentTime() - m_timeLastAccess ) >= 60*TICK_PER_SEC ))
	{
		CloseForce();
	}
}

void CResourceScript::ReSync()
{
	if ( ! IsFirstCheck())
	{
		if ( ! CheckForChange())
			return;
	}
	if ( ! Open())
		return;
	g_Cfg.LoadResourcesOpen( this );
	Close();
}

bool CResourceScript::Open( LPCTSTR pszFilename, UINT wFlags )
{
	// Open the file if it is not already open for use.

	if ( ! IsFileOpen())
	{
		if ( ! CScript::Open( pszFilename, wFlags|OF_SHARE_DENY_WRITE ))	// OF_READ
			return( false );
		if ( ! ( wFlags & OF_READWRITE ) && CheckForChange())
		{
			//  what should we do about it ? reload it of course !
#ifdef GRAY_SVR
			g_Cfg.LoadResourcesOpen( this );
#endif
		}
	}

	m_iOpenCount++;
	DEBUG_CHECK(m_iOpenCount>0);
	ASSERT( IsFileOpen());
	return( true );
}

void CResourceScript::CloseForce()
{
	DEBUG_CHECK( ! m_iOpenCount );
	m_iOpenCount = 0;
	CScript::CloseForce();
}

void CResourceScript::Close()
{
	// Don't close the file yet.
	// Close it later when we know it has not been used for a bit.
	if ( ! IsFileOpen())
		return;
	m_iOpenCount--;
	DEBUG_CHECK(m_iOpenCount>=0);

	if ( ! m_iOpenCount )
	{
		m_timeLastAccess = CServTime::GetCurrentTime();
		// Just leave it open for caching purposes
#if defined(_WIN32) && defined(GRAY_SVR)
		if ( g_Serv.m_iModeCode != SERVMODE_Test8 )
#endif
		{
			CloseForce();
		}
	}
}

//***************************************************************************
//	CResourceLock
//

bool CResourceLock::OpenBase( void * pExtra )
{
	ASSERT(m_pLock);

	if ( m_pLock->IsFileOpen())
	{
		m_PrvLockContext = m_pLock->GetContext();
		DEBUG_CHECK( m_PrvLockContext.IsValid());
	}
	else
	{
		DEBUG_CHECK( ! m_PrvLockContext.IsValid());
	}

	if ( ! m_pLock->Open())	// make sure the original is open.
		return( false );

	DEBUG_CHECK( m_pLock->IsFileOpen());

	// Open a seperate copy of an already opend file.

#if 0
	// Get my own _dup(ed) low level handle first.
	if ( ! CGFile::OpenBaseCopy( file ))
		return( false );
	// Now get my stream handle.
	m_pStream = _fdopen( m_hFile, GetModeStr());
	if ( m_pStream == NULL )
	{
		return( false );
	}
#else
	m_pStream = m_pLock->m_pStream;
	m_hFile = m_pLock->m_hFile;
#endif

	// Assume this is the new error context !
	m_PrvScriptContext.OpenScript( this );
	return( true );
}

void CResourceLock::CloseBase()
{
	ASSERT(m_pLock);
	m_pStream = NULL;

	// Assume this is not the context anymore.
	m_PrvScriptContext.Close();

	if ( m_PrvLockContext.IsValid())
	{
		m_pLock->SeekContext(m_PrvLockContext);	// no need to set the line number as it should not have changed.
	}

	// Restore old position in the file (if there was one)
	m_pLock->Close();	// decrement open count on the orig.

	Init();
}

bool CResourceLock::ReadTextLine( bool fRemoveBlanks ) // Read a line from the opened script file
{
	// ARGS:
	// fRemoveBlanks = Don't report any blank lines, (just keep reading)
	//

	ASSERT(m_pLock);
	ASSERT( ! IsBinaryMode());

	while ( CFileText::ReadString( GetKeyBufferRaw(SCRIPT_MAX_LINE_LEN), SCRIPT_MAX_LINE_LEN ))
	{
		m_pLock->m_iLineNum = ++m_iLineNum;	// share this with original open.
		if ( fRemoveBlanks )
		{
			if ( ParseKeyEnd() <= 0 )
				continue;
		}
		return( true );
	}

	m_pszKey[0] = '\0';
	return( false );
}

int CResourceLock::OpenLock( CResourceScript * pLock, CScriptLineContext context )
{
	// ONLY called from CResourceLink
	ASSERT(pLock);

	Close();
	m_pLock = pLock;

	if ( ! Open( m_pLock->GetFilePath(), m_pLock->GetMode() ))	// open my copy.
		return( -2 );

	if ( ! SeekContext( context ))
	{
		return( -3 );
	}

	return( 0 );
}

/////////////////////////////////////////////////
//	-CResourceLink

CResourceLink::CResourceLink( RESOURCE_ID rid, const CVarDefNum * pDef ) :
	CResourceDef( rid, pDef )
{
	m_pScript = NULL;
	m_Context.Init(); // not yet tested.
	m_lRefInstances = 0;
	ClearTriggers();
}

CResourceLink::~CResourceLink()
{
	DEBUG_CHECK( ! m_lRefInstances );
}

void CResourceLink::ScanSection( RES_TYPE restype )
{
	// Scan the section we are linking to for useful stuff.

	ASSERT(m_pScript);
#if 0 // def _DEBUG
	if ( strstr( m_pScript->GetFilePath(), "spheretest.scp" ))
	{
		DEBUG_MSG(("trip\n"));
	}
#endif

	LPCTSTR const * ppTable = NULL;
	int iQty = 0;

#ifdef GRAY_SVR
	switch (restype)
	{
	case RES_TYPEDEF:
	case RES_ITEMDEF:
		ppTable = CItem::sm_szTrigName;
		iQty = ITRIG_QTY;
		break;
	case RES_CHARDEF:
	case RES_EVENTS:
	case RES_SKILLCLASS:
		ppTable = CChar::sm_szTrigName;
		iQty = CTRIG_QTY;
		break;
	case RES_SKILL:
		ppTable = CSkillDef::sm_szTrigName;
		iQty = SKTRIG_QTY;
		break;
	case RES_SPELL:
		ppTable = CSpellDef::sm_szTrigName;
		iQty = SPTRIG_QTY;
		break;
	case RES_AREA:
	case RES_REGIONTYPE:
		ppTable = CRegionWorld::sm_szTrigName;
		iQty = RTRIG_QTY;
		break;
	case RES_WEBPAGE:
		ppTable = CWebPageDef::sm_szTrigName;
		iQty = WTRIG_QTY;
		break;
	// case RES_DIALOG:	// numeric labels.
	// case RES_MENU:	// these are the text labels.
	// case RES_SPEECH:	// these cannot be predicted.
	}
#endif
	ClearTriggers();

	while ( m_pScript->ReadKey(false))
	{
		if ( m_pScript->IsKeyHead( "DEFNAME", 7 ))
		{
			m_pScript->ParseKeyLate();
			SetResourceName( m_pScript->GetArgRaw());
		}
		if ( m_pScript->IsKeyHead( "ON", 2 ))
		{
			int iTrigger;
			if ( iQty )
			{
				m_pScript->ParseKeyLate();
				iTrigger = FindTableSorted( m_pScript->GetArgRaw(), ppTable, iQty );

				if ( iTrigger < 0 )	// unknown triggers ?
				{
					if ( restype != RES_WEBPAGE )
					{
						// KELL
						// DEBUG_WARN(( "Unknown trigger '%s' in '%s'\n", m_pScript->GetArgRaw(), GetResourceName()));
					}
					iTrigger = XTRIG_UNKNOWN;
				}
				else if ( HasTrigger(iTrigger))
				{
					DEBUG_ERR(( "Duplicate trigger '%s' in '%s'\n", ppTable[iTrigger], GetResourceName()));
					continue;
				}
			}
			else
			{
				// suffice to know that it has triggers of some sort.
				iTrigger = XTRIG_UNKNOWN;
			}
			SetTrigger(iTrigger); 
		}
	}
}

bool CResourceLink::ResourceLock( CResourceLock &s )
{
	// Find the definition of this item in the scripts.
	// Open a locked copy of this script
	// NOTE: How can we tell the file has changed since last open ?
	// RETURN: true = found it.

	if ( ! IsLinked())	// has already failed previously.
	{
		return( false );
	}

	ASSERT(m_pScript);

	int iRet = s.OpenLock( m_pScript, m_Context );
	if ( ! iRet )
	{
		return( true );
	}

	s.AttachObj( this );

	// ret = -2 or -3

	LPCTSTR pszName = GetResourceName();
	DEBUG_ERR(( "ResourceLock '%s':%d id=%s FAILED\n",		// type=%s,
		(LPCTSTR) s.GetFilePath(),
		m_Context.m_lOffset,
		// CResourceBase::GetResourceBlockName( GetResourceType()),
		pszName ));

	//SetBadLinkOffset( iRet ); // tried this and failed so don't bother trying again.

	// didn't find it.
	return( false );
}

//***************************************************************************
//	CScriptFileContext

void CScriptFileContext::OpenScript( const CScript * pScriptContext )
{
	Close();
	m_fOpenScript = true;
	m_pPrvScriptContext = g_Log.SetScriptContext( pScriptContext );
}

void CScriptFileContext::Close()
{
	if ( m_fOpenScript )
	{
		m_fOpenScript = false;
		g_Log.SetScriptContext( m_pPrvScriptContext );
	}
}

//***************************************************************************
//	CScriptObjectContext

void CScriptObjectContext::OpenObject( const CScriptObj * pObjectContext )
{
	Close();
	m_fOpenObject = true;
	m_pPrvObjectContext = g_Log.SetObjectContext( pObjectContext );
}

void CScriptObjectContext::Close()
{
	if ( m_fOpenObject )
	{
		m_fOpenObject = false;
		g_Log.SetObjectContext( m_pPrvObjectContext );
	}
}

/////////////////////////////////////////////////
// -CResourceRefArray

bool CResourceRefArray::r_LoadVal( CScript & s, RES_TYPE restype )
{
	bool fRet = false;
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	// A bunch of CResourceLink (CResourceDef) pointers.
	// Add or remove from the list.
	// RETURN: false = it failed.

	// ? "TOWN" and "REGION" are special !

	TCHAR * pszCmd = s.GetArgStr();

	TCHAR * ppBlocks[128];	// max is arbitrary
	int iArgCount = Str_ParseCmds( pszCmd, ppBlocks, COUNTOF(ppBlocks));

	for ( int i=0; i<iArgCount; i++ )
	{
		pszCmd = ppBlocks[i];

		if ( pszCmd[0] == '-' )
		{
			// remove a frag or all frags.
			pszCmd ++;
			if ( pszCmd[0] == '0' || pszCmd[0] == '*' )
			{
				RemoveAll();
				fRet = true;
				continue;
			}

			CResourceLink * pResourceLink = dynamic_cast<CResourceLink *>( g_Cfg.ResourceGetDefByName( restype, pszCmd ));
			if ( pResourceLink == NULL )
			{
				fRet = false;
				continue;
			}

			int iIndex = RemovePtr(pResourceLink);
			fRet = ( iIndex >= 0 );
		}
		else
		{
			// Add a single knowledge fragment or appropriate group item.

			if ( pszCmd[0] == '+' ) pszCmd ++;

			CResourceLink * pResourceLink = dynamic_cast<CResourceLink *>( g_Cfg.ResourceGetDefByName( restype, pszCmd ));
			if ( pResourceLink == NULL )
			{
				fRet = false;
				DEBUG_ERR(( "Unknown '%s' Resource '%s'\n", CResourceBase::GetResourceBlockName(restype), pszCmd ));
				continue;
			}

			// Is it already in the list ?
			fRet = true;
			if ( FindPtr(pResourceLink) >= 0 )
			{
				continue;
			}

			Add( pResourceLink );
		}
	}
	EXC_CATCH("CResourceRefArray");
	return( fRet );
}

void CResourceRefArray::WriteResourceRefList( CGString & sVal ) const
{
	TCHAR *pszVal = Str_GetTemp();
	int len = 0;
	for ( int j=0;j<GetCount(); j++ )
	{
		if ( j )
		{
			pszVal[ len ++ ] = ',';
		}
		len += strcpylen( pszVal+len, GetResourceName( j ));
		if ( len >= SCRIPT_MAX_LINE_LEN-1 )
			break;
	}
	pszVal[ len ] = '\0';
	sVal = pszVal;
}

int CResourceRefArray::FindResourceType( RES_TYPE restype ) const
{
	// Is this resource already in the list ?
	int iQty = GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetRef()->GetResourceID();
		if ( ridtest.GetResType() == restype )
			return( i );
	}
	return( -1 );
}

int CResourceRefArray::FindResourceID( RESOURCE_ID_BASE rid ) const
{
	// Is this resource already in the list ?
	int iQty = GetCount();
	for ( int i=0; i<iQty; i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetRef()->GetResourceID();
		if ( ridtest == rid )
			return( i );
	}
	return( -1 );
}

int CResourceRefArray::FindResourceName( RES_TYPE restype, LPCTSTR pszKey ) const
{
	// Is this resource already in the list ?
	CResourceLink * pResourceLink = dynamic_cast <CResourceLink *>( g_Cfg.ResourceGetDefByName( restype, pszKey ));
	if ( pResourceLink == NULL )
		return( false );

	return( FindPtr(pResourceLink));
}

void CResourceRefArray::r_Write( CScript & s, LPCTSTR pszKey ) const
{
	for ( int j=0;j<GetCount(); j++ )
	{
		s.WriteKey( pszKey, GetResourceName( j ));
	}
}

//**********************************************
// -CResourceQty

int CResourceQty::WriteKey( TCHAR * pszArgs, bool fQtyOnly, bool fKeyOnly ) const
{
	int i=0;
	if ( (GetResQty() || fQtyOnly) && !fKeyOnly )
	{
		i = sprintf( pszArgs, "%d ", GetResQty());
	}
	if ( !fQtyOnly )
		i += strcpylen( pszArgs+i, g_Cfg.ResourceGetName( m_rid ));
	return( i );
}

int CResourceQty::WriteNameSingle( TCHAR * pszArgs ) const
{
	int i;
	CScriptObj * pResourceDef = g_Cfg.ResourceGetDef( m_rid );
	if ( pResourceDef )
	{
		i = strcpylen( pszArgs, pResourceDef->GetName());
	}
	else
	{
		i = strcpylen( pszArgs, g_Cfg.ResourceGetName( m_rid ));
	}
	return( i );
}

bool CResourceQty::Load( LPCTSTR & pszCmds )
{
	// Can be either order.:
	// "Name Qty" or "Qty Name"

	GETNONWHITESPACE( pszCmds );	// Skip leading spaces.

	m_iQty = 0;
	if ( ! isalpha( *pszCmds )) // might be { or .
	{
		m_iQty = Exp_GetVal(pszCmds);
		GETNONWHITESPACE( pszCmds );	// Skip leading spaces.
	}

	if ( *pszCmds == '\0' )
		return false;

	LPCTSTR pszPrv = pszCmds;
	m_rid = g_Cfg.ResourceGetID( RES_UNKNOWN, pszCmds );

	if ( m_rid.GetResType() == RES_UNKNOWN )
	{
		// This means nothing to me!
		DEBUG_ERR(( "Bad resource list id '%s'\n", pszPrv ));
		return( false );
	}

	GETNONWHITESPACE( pszCmds );	// Skip leading spaces.

	if ( ! m_iQty )	// trailing qty?
	{
		if ( *pszCmds == '\0' || *pszCmds == ',' )
		{
			m_iQty = 1;
		}
		else
		{
			m_iQty = Exp_GetVal(pszCmds);
			GETNONWHITESPACE( pszCmds );	// Skip leading spaces.
		}
	}

	return( true );
}

//**********************************************
// -CResourceQtyArray

int CResourceQtyArray::FindResourceType( RES_TYPE type ) const
{
	// is this RES_TYPE in the array ?
	// -1 = fail
	for ( int i=0; i<GetCount(); i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetResourceID();
		if ( type == ridtest.GetResType() )
			return( i );
	}
	return( -1 );
}

int CResourceQtyArray::FindResourceID( RESOURCE_ID_BASE rid ) const
{
	// is this RESOURCE_ID in the array ?
	// -1 = fail
	for ( int i=0; i<GetCount(); i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetResourceID();
		if ( rid == ridtest )
			return( i );
	}
	return( -1 );
}

int CResourceQtyArray::FindResourceMatch( CObjBase * pObj ) const
{
	// Is there a more vague match in the array ?
	// Use to find intersection with this pOBj raw material and BaseResource creation elements.
	for ( int i=0; i<GetCount(); i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetResourceID();
		if ( pObj->IsResourceMatch( ridtest, 0 ))
			return( i );
	}
	return( -1 );
}

bool CResourceQtyArray::IsResourceMatchAll( CChar * pChar, DWORD dwArg ) const
{
	// Check all required skills and non-consumable items.
	// RETURN:
	//  false = failed.

	for ( int i=0; i<GetCount(); i++ )
	{
		RESOURCE_ID ridtest = GetAt(i).GetResourceID();

		if (	ridtest.GetResType() == RES_TYPEDEF
			&&	ridtest.GetResIndex() == IT_RESEARCH_ITEM
			&&	dwArg == 0 )
			continue;
		if ( ! pChar->IsResourceMatch( ridtest, GetAt(i).GetResQty(), dwArg ))
			return( false );
	}

	return( true );
}

int CResourceQtyArray::Load( LPCTSTR pszCmds )
{
	// 0 = clear the list.

	int iValid = 0;
	ASSERT(pszCmds);
	while ( *pszCmds )
	{
		if ( *pszCmds == '0' && 
			( pszCmds[1] == '\0' || pszCmds[1] == ',' ))
		{
			RemoveAll();	// clear any previous stuff.
			pszCmds ++;
		}
		else
		{
			CResourceQty res;
			if ( ! res.Load( pszCmds ))
				break;

			if ( res.GetResourceID().IsValidUID())
			{
				// Replace any previous refs to this same entry ?
				int i = FindResourceID( res.GetResourceID() );
				if ( i >= 0 )
				{
					SetAt(i,res); 
				}
				else
				{
					Add(res);
				}
				iValid++;
			}
		}

		if ( *pszCmds != ',' )
		{
			break;
		}

		pszCmds++;
	}

	return( iValid );
}

void CResourceQtyArray::WriteKeys( TCHAR * pszArgs, int index, bool fQtyOnly, bool fKeyOnly ) const
{
	int		max	= GetCount();
	if ( index > 0 && index < max )
		max		= index;

	for ( int i = (index > 0 ? index-1 : 0); i < max; i++ )
	{
		if ( i && !index )
		{
			pszArgs += sprintf( pszArgs, "," );
		}
		pszArgs += GetAt(i).WriteKey( pszArgs, fQtyOnly, fKeyOnly );
	}
	*pszArgs = '\0';
}


void CResourceQtyArray::WriteNames( TCHAR * pszArgs, int index ) const
{
	int		max	= GetCount();
	if ( index > 0 && index < max )
		max		= index;

	for ( int i = (index > 0 ? index-1 : 0); i < max; i++ )
	{
		if ( i && !index )
		{
			pszArgs += sprintf( pszArgs, ", " );
		}
		if ( GetAt(i).GetResQty())
		{
			if ( GetAt(i).GetResType() == RES_SKILL )
			{
				pszArgs += sprintf( pszArgs, "%d.%d ",
						GetAt(i).GetResQty() / 10, GetAt(i).GetResQty() % 10 );
			}
			else
				pszArgs += sprintf( pszArgs, "%d ", GetAt(i).GetResQty());
		}
		pszArgs += GetAt(i).WriteNameSingle( pszArgs );
	}
	*pszArgs = '\0';
}

bool CResourceQtyArray::operator == ( const CResourceQtyArray & array ) const
{
	if ( GetCount() != array.GetCount())
		return( false );

	for ( int i=0; i<GetCount(); i++ )
	{
		for ( int j=0; true; j++ )
		{
			if ( j>=array.GetCount())
				return( false );
			if ( ! ( GetAt(i).GetResourceID() == array[j].GetResourceID() ))
				continue;
			if ( GetAt(i).GetResQty() != array[j].GetResQty() )
				continue;
			break;
		}
	}
	return( true );
}

#endif // GRAY_SVR
