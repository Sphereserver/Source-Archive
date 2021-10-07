//
// CWebPage.cpp
// Copyright Menace Software (www.menasoft.com).
//

#include "graysvr.h"	// predef header.
#include "../common/grayver.h"	// sphere version
#include "../common/CFileList.h"

enum WV_TYPE
{
	WV_CLIENTLIST,
	WV_GMPAGELIST,
	WV_GUILDLIST,
	WV_SERVLIST,
	WV_SERVLIST1,
	WV_SERVLIST2,
	WV_SERVLIST3,
	WV_SERVLIST4,
	WV_SERVLIST5,
	WV_SERVLIST6,
	WV_WEBPAGE,
	WV_QTY,
};

LPCTSTR const CWebPageDef::sm_szVerbKeys[WV_QTY+1] =
{
	"CLIENTLIST",	// make a table of all the clients.
	"GMPAGELIST",	// make a table of the gm pages.
	"GUILDLIST",	// make a table of the guilds.
	"SERVLIST",		// make a name sorted list of the servers.
	"SERVLIST1",	// make a name sorted list of the servers.
	"SERVLIST2",
	"SERVLIST3",
	"SERVLIST4",
	"SERVLIST5",
	"SERVLIST6",
	"WEBPAGE",		// feed a web page to the source caller
	NULL,
};

//////////////////////////////////////////////////////////
// -CServersSortArray

class CServersSortArray : public CGPtrTypeArray<CServerRef>
{
	WV_TYPE m_iType;	// ? SC_TYPE ?
private:
	int QCompare( int i, CServerDef * pServ );
	void QSort( int left, int right );

public:
	bool SortByType( WV_TYPE iType );
};

int CServersSortArray::QCompare( int left, CServerDef * pServRef )
{
	// 1 = left > right
	ASSERT(pServRef);
	switch ( m_iType )
	{
	case WV_SERVLIST1:	// Name
		{
			// special sort based on alnum. ignore non alnum chars
			LPCTSTR pszName1 = pServRef->GetName();
			LPCTSTR pszName2 = GetAt(left)->GetName();
			// return( strcmpi( pszName1, pszName2 ));

			int i=0;
			while ( true )
			{
				TCHAR ch1;
				while(true)
				{
					ch1 = toupper( *pszName1++ );
					if ( ! ch1 )
						break;
					if ( isalpha(ch1))
					{
						if ( ch1 == toupper( *pszName1 ))
							continue;
						break;
					}
				}

				TCHAR ch2;
				while(true)
				{
					ch2 = toupper( *pszName2++ );
					if ( ! ch2 )
						break;
					if ( isalpha(ch2))
					{
						if ( ch2 == toupper( *pszName2 ))
							continue;
						break;
					}
				}

				if ( ch1 == ch2 )
				{
					if ( ch1 == 0 )
						return( 0 );
					i++;
					continue;
				}

				if ( ch1 == 0 && i == 0 )
					return( -1 );
				if ( ch2 == 0 && i == 0 )
					return( 1 );

				return( (int) ch2 - (int) ch1 );
			}
		}

	case WV_SERVLIST2:	// ClientsAvg
		return( pServRef->GetClientsAvg() - GetAt(left)->GetClientsAvg());

	case WV_SERVLIST3:	// Age
		return( pServRef->GetAgeHours() - GetAt(left)->GetAgeHours());

	case WV_SERVLIST4:	// Lang
		return( strcmpi( pServRef->m_sLang, GetAt(left)->m_sLang ));

	case WV_SERVLIST5:	// Ver
		return( strcmpi( pServRef->GetServerVersion(), GetAt(left)->GetServerVersion() ));

	case WV_SERVLIST6: // TZ
		return( pServRef->m_TimeZone - GetAt(left)->m_TimeZone );

	}
	return( 0 );
}

void CServersSortArray::QSort( int left, int right )
{
	static int sm_iReentrant=0;
	ASSERT( left < right );
	int j = left;
	int i = right;

	CServerDef * pServRef = GetAt( (left+right) / 2 );

	do
	{
		while ( QCompare( j, pServRef ) < 0 )
			j++;
		while ( QCompare( i, pServRef ) > 0 )
			i--;

		if ( i >= j )
		{
			if ( i != j )
			{
				CServerDef * pServTmp = GetAt(i);
				SetAt( i, GetAt(j));
				SetAt( j, pServTmp );
			}
			i--;
			j++;
		}

	} while (j <= i);

	sm_iReentrant++;
	if (left < i)  QSort(left,i);
	if (j < right) QSort(j,right);
	sm_iReentrant--;
}

bool CServersSortArray::SortByType( WV_TYPE iType )
{
	// 1 = name
	// 2 = ClientsAvg
	// 3 = Age
	// 4 = Lang

	if ( iType == WV_SERVLIST )		// just leave it the same.
		return false;

	int iSize;
	{
		CThreadLockRef lock( &(g_Cfg.m_Servers));
		iSize = g_Cfg.m_Servers.GetCount();
		SetCount( iSize );
		memcpy( GetBasePtr(), g_Cfg.m_Servers.GetBasePtr(), sizeof(CServerRef) * iSize );
	}

	if ( ! g_Cfg.m_sMainLogServerDir.IsEmpty())
	{
		// List myself first.
		InsertAt( 0, &g_Serv );
		iSize++;
	}

	if ( iSize <= 1 )
		return true;

	m_iType = iType;

	// Now quicksort the list to the needed format.
	QSort( 0, iSize-1 );
	return true;
}

//********************************************************
// -CFileConsole

class CFileConsole : public CTextConsole
{
public:
	CFileText m_FileOut;
public:
	virtual PLEVEL_TYPE GetPrivLevel() const
	{
		return PLEVEL_Admin;
	}
	virtual LPCTSTR GetName() const
	{
		return "WebFile";
	}
	virtual void SysMessage( LPCTSTR pszMessage ) const
	{
		if ( pszMessage == NULL || ISINTRESOURCE(pszMessage))
			return;
		(const_cast <CFileConsole*>(this))->m_FileOut.WriteString(pszMessage);
	}
};

//********************************************************
// -CWebPageDef

int CWebPageDef::sm_iListIndex;

CWebPageDef::CWebPageDef( RESOURCE_ID rid ) :
	CResourceLink( rid )
{
	// Web page m_sWebPageFilePath
	m_type = WEBPAGE_TEMPLATE;
	m_privlevel=PLEVEL_Guest;

	m_timeNextUpdate.Init();
	m_iUpdatePeriod = 2*60*TICK_PER_SEC;

	// default source name
	if ( rid.GetResIndex())
	{
		m_sSrcFilePath.Format( GRAY_FILE "statusbase%d.htm", rid.GetResIndex());
	}
	else
	{
		m_sSrcFilePath = GRAY_FILE "statusbase.htm";
	}
}

enum WC_TYPE
{
	WC_PLEVEL,
	WC_WEBCLIENTLISTFORM,	// m_sWebClientListFormat
	WC_WEBPAGEDST,
	WC_WEBPAGEFILE,
	WC_WEBPAGELOG,
	WC_WEBPAGESRC,
	WC_WEBPAGEUPDATE,
	WC_WEBSERVERLISTFORM,	// m_sWebServListFormat
	WC_QTY,
};

LPCTSTR const CWebPageDef::sm_szLoadKeys[WC_QTY+1] =
{
	"PLEVEL",				// What priv level does one need to be to view this page.
	"WEBCLIENTLISTFORM",	// Old method of formatting. m_sWebClientListFormat
	"WEBPAGEDST",			// For periodic generated pages.
	"WEBPAGEFILE",			// save as WEBPAGEDST
	"WEBPAGELOG",			// daily log a copy of this page.
	"WEBPAGESRC",			// the source name of the web page.
	"WEBPAGEUPDATE",		// how often to generate a page ? (seconds)
	"WEBSERVERLISTFORM",	// m_sWebServListFormat
	NULL,
};

bool CWebPageDef::r_WriteVal( LPCTSTR pszKey, CGString & sVal, CTextConsole * pSrc )
{
	EXC_TRY(("r_WriteVal('%s',,%x)", pszKey, pSrc));
	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case WC_PLEVEL:
		sVal.FormatVal( m_privlevel );
		break;
	case WC_WEBCLIENTLISTFORM:
		sVal = m_sClientListFormat;
		break;
	case WC_WEBPAGEDST:
	case WC_WEBPAGEFILE:
		sVal = m_sDstFilePath;
		break;
	case WC_WEBPAGELOG:
		sVal.FormatVal( m_iUpdateLog );
		break;
	case WC_WEBPAGESRC:
		sVal = m_sSrcFilePath;
		break;
	case WC_WEBPAGEUPDATE:	// (seconds)
		sVal.FormatVal( m_iUpdatePeriod / TICK_PER_SEC );
		break;
	case WC_WEBSERVERLISTFORM:
		sVal = m_sServListFormat;
		break;
	default:
		return( g_Serv.r_WriteVal( pszKey, sVal, pSrc ));
	}
	EXC_CATCH("CWebPageDef");
	return( true );
}

bool CWebPageDef::r_LoadVal( CScript & s ) // Load an item Script
{
	EXC_TRY(("r_LoadVal('%s %s')", s.GetKey(), s.GetArgStr()));
	switch ( FindTableSorted( s.GetKey(), sm_szLoadKeys, COUNTOF( sm_szLoadKeys )-1 ))
	{
	case WC_PLEVEL:
		m_privlevel = (PLEVEL_TYPE) s.GetArgVal();
		break;
	case WC_WEBCLIENTLISTFORM:
		m_sClientListFormat = Str_MakeFiltered( s.GetArgStr());
		break;
	case WC_WEBPAGEDST:
	case WC_WEBPAGEFILE:
		m_sDstFilePath = s.GetArgStr();
		break;
	case WC_WEBPAGELOG:
		m_iUpdateLog = s.GetArgVal();
		break;
	case WC_WEBPAGESRC:
		return SetSourceFile( s.GetArgStr(), NULL );
	case WC_WEBPAGEUPDATE:	// (seconds)
		m_iUpdatePeriod = s.GetArgVal()*TICK_PER_SEC;
		if ( m_iUpdatePeriod && m_type == WEBPAGE_TEXT )
		{
			m_type = WEBPAGE_TEMPLATE;
		}
		break;
	case WC_WEBSERVERLISTFORM:
		m_sServListFormat = Str_MakeFiltered( s.GetArgStr());
		break;
	default:
		return( CScriptObj::r_LoadVal( s ));
	}
	EXC_CATCH("CWebPageDef");
	return( true );
}

bool CWebPageDef::r_Verb( CScript & s, CTextConsole * pSrc )	// some command on this object as a target
{
	EXC_TRY(("r_Verb('%s %s',%x)", s.GetKey(), s.GetArgStr(), pSrc));
	ASSERT(pSrc);
	sm_iListIndex = 0;
	TCHAR *pszTmp2 = Str_GetTemp();

	WV_TYPE iHeadKey = (WV_TYPE) FindTableSorted( s.GetKey(), sm_szVerbKeys, COUNTOF(sm_szVerbKeys)-1 );
	switch ( iHeadKey )
	{
	case WV_WEBPAGE:
		{
		// serv a web page to the pSrc
			CClient * pClient = dynamic_cast <CClient *>(pSrc);
			if ( pClient == NULL )
				return( false );
			return ServPage( pClient, s.GetArgStr(), NULL );
		}
		return( true );

	case WV_CLIENTLIST:
		{
			for ( CClient * pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
			{
				CChar * pChar = pClient->GetChar();
				if ( pChar == NULL )
					continue;
				if ( pChar->IsDND())
					continue;
				DEBUG_CHECK( ! pChar->IsWeird());

				sm_iListIndex++;

				LPCTSTR pszArgs = s.GetArgStr();
				if ( pszArgs[0] == '\0' )
					pszArgs = m_sClientListFormat;
				if ( pszArgs[0] == '\0' )
					pszArgs = _TEXT("<tr><td>%NAME%</td><td>%REGION.NAME%</td></tr>\n");
				strcpy( pszTmp2, pszArgs );
				pChar->ParseText( Str_MakeFiltered( pszTmp2 ), &g_Serv, 1 );
				pSrc->SysMessage( pszTmp2 );
			}
		}
		break;

	case WV_SERVLIST1:
	case WV_SERVLIST2:
	case WV_SERVLIST3:
	case WV_SERVLIST4:
	case WV_SERVLIST5:
	case WV_SERVLIST6:
	case WV_SERVLIST:
		{
			// A list sorted in some special way
			CServersSortArray Array;
			if ( ! Array.SortByType( iHeadKey ))
			{
				// pszFormat = NULL;
			}

			for ( int i=0; i<Array.GetCount(); i++ )
			{
				CServerDef * pServ = Array[i];
				if ( pServ == NULL )
					continue;

				// NOTE: servers list onces per hour. if they have not registered in 8 hours then they are not valid.

				if ( pServ->GetTimeSinceLastValid() > ( TICK_PER_SEC * 60 * 60 * 8 ))
					continue;

				sm_iListIndex ++;

				LPCTSTR pszArgs = s.GetArgStr();
				if ( pszArgs[0] == '\0' )
					pszArgs = m_sServListFormat;
				if ( pszArgs[0] == '\0' )
					pszArgs = _TEXT("<tr><td>%SERVNAME%</td><td>%STATUS%</td></tr>\n");

				strcpy( pszTmp2, pszArgs );
				pServ->ParseText( Str_MakeFiltered( pszTmp2 ), &g_Serv, 1 );
				pSrc->SysMessage( pszTmp2 );
			}
		}
		break;

	case WV_GUILDLIST:
		{
			if ( ! s.HasArgs())
				return( false );

			for ( int i=0; i<g_World.m_Stones.GetCount(); i++ )
			{
				CItemStone * pStone = g_World.m_Stones[i];
				if ( pStone == NULL )
					continue;
				DEBUG_CHECK( ! pStone->IsWeird());

				sm_iListIndex++;

				strcpy( pszTmp2, s.GetArgStr() );
				pStone->ParseText( Str_MakeFiltered( pszTmp2 ), &g_Serv, 1 );
				pSrc->SysMessage( pszTmp2 );
			}
		}
		break;

	case WV_GMPAGELIST:
		{
			if ( ! s.HasArgs())
				return( false );
			CGMPage * pPage = STATIC_CAST <CGMPage*>( g_World.m_GMPages.GetHead());
			for ( ; pPage!=NULL; pPage = pPage->GetNext())
			{
				sm_iListIndex++;
				strcpy( pszTmp2, s.GetArgStr() );
				pPage->ParseText( Str_MakeFiltered( pszTmp2 ), &g_Serv, 1 );
				pSrc->SysMessage( pszTmp2 );
			}
		}
		break;

	default:
		return( CResourceLink::r_Verb(s,pSrc));
	}
	EXC_CATCH("CWebPageDef");
	return( true );
}

bool CWebPageDef::WebPageUpdate( bool fNow, LPCTSTR pszDstName, CTextConsole * pSrc )
{
	// Generate the status web pages.
	// Read in the Base status page "*STATUSBASE.HTM"
	// Filter the XML type tags.
	// Output the status page "*.HTM"
	// Server name
	// Server email
	// Number of clients, items, NPC's

	if ( ! fNow )
	{
		if ( m_iUpdatePeriod <= 0 )
			return false;
		if ( CServTime::GetCurrentTime() < m_timeNextUpdate )
			return true;	// should stilll be valid
	}

	ASSERT(pSrc);
	m_timeNextUpdate = CServTime::GetCurrentTime() + m_iUpdatePeriod;
	if ( pszDstName == NULL )
	{
		pszDstName = m_sDstFilePath;
	}

	if ( m_type != WEBPAGE_TEMPLATE ||
		*pszDstName == '\0' ||
		m_sSrcFilePath.IsEmpty())
		return false;

	// g_Log.Event( LOGM_HTTP|LOGL_TRACE, "HTTP Page Update '%s'" DEBUG_CR, (LPCTSTR) m_sSrcFilePath );

	CScript FileRead;
	if ( ! FileRead.Open( m_sSrcFilePath ))
	{
		return false;
	}

	CScriptFileContext context( &FileRead );	// set this as the context.

	CFileConsole FileOut;
	if ( ! FileOut.m_FileOut.Open( pszDstName, OF_WRITE|OF_TEXT ))
	{
		DEBUG_ERR(( "Can't open web page output '%s'" DEBUG_CR, (LPCTSTR) pszDstName ));
		return false;
	}

	bool fScriptMode = false;

	while ( FileRead.ReadTextLine( false ))
	{
		TCHAR *pszTmp = Str_GetTemp();;
		strcpy( pszTmp, FileRead.GetKey());

		TCHAR * pszHead = strstr( pszTmp, "<script language=\"Sphere\">" );
		if ( pszHead != NULL )
		{
			// Deal with the stuff preceding the scripts.
			*pszHead = '\0';
			pszHead += 26;
			ParseText( pszTmp, pSrc, 1 );
			FileOut.SysMessage( pszTmp );
			fScriptMode = true;
		}
		else
		{
			pszHead = pszTmp;
		}

		// Look for the end of </script>
		if ( fScriptMode )
		{
			GETNONWHITESPACE(pszHead);
			TCHAR * pszFormat = pszHead;

			pszHead = strstr( pszFormat, "</script>" );
			if ( pszHead != NULL )
			{
				*pszHead = '\0';
				pszHead += 9;
				fScriptMode = false;
			}

			if ( pszFormat[0] != '\0' )
			{
				// Allow if/then logic ??? OnTriggerRun( CScript &s, TRIGRUN_SINGLE_EXEC, &FileOut )
				CScript script( pszFormat );
				if ( ! r_Verb( script, &FileOut ))
				{
					DEBUG_ERR(( "Web page source format error '%s'" DEBUG_CR, (LPCTSTR) pszTmp ));
					continue;
				}
			}

			if ( fScriptMode )
				continue;
		}

		// Look for stuff we can displace here. %STUFF%
		ParseText( pszHead, pSrc, 1 );
		FileOut.SysMessage( pszHead );
	}

	return( true );
}

void CWebPageDef::WebPageLog()
{
	if ( ! m_iUpdateLog || ! m_iUpdatePeriod )
		return;
	if ( m_type != WEBPAGE_TEMPLATE )
		return;

	CFileText FileRead;
	if ( ! FileRead.Open( m_sDstFilePath, OF_READ|OF_TEXT ))
		return;

	LPCTSTR pszExt = FileRead.GetFileExt();

	TCHAR szName[ _MAX_PATH ];
	strcpy( szName, m_sDstFilePath );
	szName[ m_sDstFilePath.GetLength() - strlen(pszExt) ] = '\0';

	CGTime datetime = CGTime::GetCurrentTime();

	CGString sName;
	sName.Format( "%s%d%02d%02d%s", (LPCTSTR) szName,
		datetime.GetYear()%100,
		datetime.GetMonth(),
		datetime.GetDay(), (LPCTSTR) pszExt );

	CFileText FileTest;
	if ( FileTest.Open( sName, OF_READ|OF_TEXT ))
	{
		// Already here.
		return;
	}

	// Copy it.
	WebPageUpdate( true, sName, &g_Serv );
}

LPCTSTR const CWebPageDef::sm_szPageExt[] =
{
	".bmp",
	".gif",
	".htm",
	".html",
	".jpeg",
	".jpg",
	".js",
	".txt",
};
static WEBPAGE_TYPE const sm_szPageExtType[] =
{
	WEBPAGE_BMP,
	WEBPAGE_GIF,
	WEBPAGE_TEMPLATE,
	WEBPAGE_TEXT,
	WEBPAGE_JPG,
	WEBPAGE_JPG,
	WEBPAGE_TEXT,
	WEBPAGE_TEXT,
};

bool CWebPageDef::SetSourceFile( LPCTSTR pszName, CClient * pClient )
{
	// attempt to set this to a source file.
	// test if it exists.
	int iLen = strlen( pszName );
	if ( iLen <= 3 )
		return( false );

	bool fMasterList = true;
	if ( pClient )
	{
		if (   !g_BackTask.m_RegisterIP.IsValidAddr()
			|| !pClient->m_PeerName.IsSameIP( g_BackTask.m_RegisterIP ) )
		{
			fMasterList = false;
		}
	}

	LPCTSTR pszExt = CGFile::GetFilesExt( pszName );
	if ( pszExt == NULL || pszExt[0] == '\0' )
		return( false );

	int iType = FindTableSorted( pszExt, sm_szPageExt, COUNTOF( sm_szPageExt ));
	if ( iType < 0 )
	{
		if ( ! fMasterList )
			return( false );
	}
	else
	{
		m_type = sm_szPageExtType[iType];
	}

	if ( pClient == NULL )
	{
		// this is being set via the Script files.
		// make sure the file is valid
		CScript FileRead;
		if ( ! g_Cfg.OpenResourceFind( FileRead, pszName ))
		{
			DEBUG_ERR(( "Can't open web page input '%s'" DEBUG_CR, (LPCTSTR) m_sSrcFilePath ));
			return( false );
		}
		m_sSrcFilePath = FileRead.GetFilePath();
	}
	else
	{
		if ( *pszName == '/' )
			pszName ++;
		if ( strstr( pszName, ".." ))	// this sort of access is not allowed.
			return( false );
		if ( strstr( pszName, "\\\\" ))	// this sort of access is not allowed.
			return( false );
		if ( strstr( pszName, "//" ))	// this sort of access is not allowed.
			return( false );
		m_sSrcFilePath = CGFile::GetMergedFileName( g_Cfg.m_sSCPBaseDir, pszName );
	}

	return( true );
}

bool CWebPageDef::IsMatch( LPCTSTR pszMatch ) const
{
	if ( pszMatch == NULL )	// match all.
		return( true );

	LPCTSTR pszDstName = GetDstName();
	LPCTSTR pszTry;

	if ( pszDstName[0] )
	{
		// Page is generated periodically. match the output name
		pszTry = CScript::GetFilesTitle( pszDstName );
	}
	else
	{
		// match the source name
		pszTry = CScript::GetFilesTitle( GetName());
	}

	return( ! strcmpi( pszTry, pszMatch ));
}

LPCTSTR const CWebPageDef::sm_szPageType[WEBPAGE_QTY+1] =
{
	"text/html",		// WEBPAGE_TEMPLATE
	"text/html",		// WEBPAGE_TEXT
	"image/x-xbitmap",	// WEBPAGE_BMP,
	"image/gif",		// WEBPAGE_GIF,
	"image/jpeg",		// WEBPAGE_JPG,
	"bin",				// WEBPAGE_BIN,
	NULL,				// WEBPAGE_QTY
};

LPCTSTR const CWebPageDef::sm_szTrigName[WTRIG_QTY+1] =	// static
{
	_TEXT("@AAAUNUSED"),
	"@Load",
	NULL,
};

// HTTP error codes:
//
// 200 OK
// 201 Created
// 202 Accepted
// 203 Non-Authorative Information
// 204 No Content
// 205 Reset Content
// 206 Partial Content
//
// 300 Multiple Choices
// 301 Moved Permanently
// 302 Moved Temporarily
// 303 See Other
// 304 Not Modified
// 305 Use Proxy
//
// 400 Bad Request
// 401 Authorization Required
// 402 Payment Required
// 403 Forbidden
// 404 Not Found
// 405 Method Not Allowed
// 406 Not Acceptable (encoding)
// 407 Proxy Authentication Required
// 408 Request Timed Out
// 409 Conflicting Request
// 410 Gone
// 411 Content Length Required
// 412 Precondition Failed
// 413 Request Entity Too Long
// 414 Request URI Too Long
// 415 Unsupported Media Type
//
// 500 Internal Server Error
// 501 Not Implemented
// 502 Bad Gateway
// 503 Service Unavailable
// 504 Gateway Timeout
// 505 HTTP Version Not Supported
//

int CWebPageDef::ServPageRequest( CClient * pClient, LPCTSTR pszURLArgs, CGTime * pdateIfModifiedSince )
{
	// Got a web page request from the client.
	// ARGS:
	//  pszURLArgs = args on the URL line ex. http://www.hostname.com/dir?args
	// RETURN:
	//  HTTP error code = 0=200 page was served.

	ASSERT(pClient);

	if ( HasTrigger(WTRIG_Load))
	{
		CResourceLock s;
		if ( ResourceLock(s))
		{
			TRIGRET_TYPE iRet = CScriptObj::OnTriggerScript( s, sm_szTrigName[WTRIG_Load], pClient, NULL );
			if ( iRet == TRIGRET_RET_TRUE )
			{
				return( 0 );	// Block further action.
			}
		}
	}

	if ( m_privlevel )
	{
		if ( pClient->GetAccount() == NULL )
			return( 401 );	// Authorization required
		if ( pClient->GetPrivLevel() < m_privlevel )
			return( 403 );	// Forbidden
	}

	CGTime datetime = CGTime::GetCurrentTime();

	LPCTSTR pszName;
	bool fGenerate = false;

	if ( m_type == WEBPAGE_TEMPLATE ) // my version of cgi
	{
		pszName = GetDstName();
		if ( pszName[0] == '\0' )
		{
			pszName = _TEXT( "temppage.htm" );
			fGenerate = true;
		}
		else
		{
			fGenerate = ! m_iUpdatePeriod;
		}

		// The page must be generated on demand.
		if ( ! WebPageUpdate( fGenerate, pszName, pClient ))
			return 500;
	}
	else
	{
		pszName = GetName();
	}

	// Get proper Last-Modified: time.
	time_t dateChange;
	DWORD dwSize;
	if ( ! CFileList::ReadFileInfo( pszName, dateChange, dwSize ))
	{
		return 500;
	}

	CGString sDate = datetime.FormatGmt(NULL);	// current date.

	if ( ! fGenerate &&
		! pdateIfModifiedSince &&
		pdateIfModifiedSince->IsTimeValid() &&
		dateChange <= pdateIfModifiedSince->GetTime() )
	{
		// return 304 (not modified)  if the "If-Modified-Since" is not true.
		// DEBUG_MSG(( "Web page '%s' not modified since '%s'" DEBUG_CR, (LPCTSTR) GetName(), ((CGTime)dateChange).FormatGmt(NULL) ));
		// "\r\n" = 0d0a

		CGString sMsgHead;
		sMsgHead.Format(
			"HTTP/1.1 304 Not Modified\r\n"
			"Date: %s\r\n"
			"Server: %s V %s\r\n"
			// "ETag: \"%s\"\r\n"	// No idea. "094e54b9f48bf1:1f83"
			"Content-Length: 0\r\n"
			// "Connection: close\r\n"
			"\r\n",
			(LPCTSTR) sDate,
			(LPCTSTR) g_Cfg.m_sDisguiseVersion.GetPtr(),
			(LPCTSTR)g_Cfg.m_sDisguiseName.GetPtr(),
			(LPCTSTR) GetName());

		pClient->xSendReady( (LPCTSTR) sMsgHead, sMsgHead.GetLength());
		return(0);
	}

	// Now serve up the page.
	CGFile FileRead;
	if ( ! FileRead.Open( pszName, OF_READ|OF_BINARY ))
		return 500;

	// Send the header first.
	TCHAR szTmp[8*1024];
	int iLen = sprintf( szTmp,
		"HTTP/1.1 200 OK\r\n"	// 100 Continue
		"Date: %s\r\n"
		"Server: %s V %s\r\n"	// Microsoft-IIS/4.0
		"Accept-Ranges: bytes\r\n"
		"Content-Type: %s\r\n",
		(LPCTSTR) sDate,
		(LPCTSTR)g_Cfg.m_sDisguiseName.GetPtr(),
		(LPCTSTR) g_Cfg.m_sDisguiseVersion.GetPtr(),
		(LPCTSTR) sm_szPageType[ m_type ]	// type of the file. image/gif, image/x-xbitmap, image/jpeg
		);

	if ( m_type == WEBPAGE_TEMPLATE )
	{
		// WEBPAGE_TEMPLATE pages will always expire.
		// CGTime dateexpire = datetime.GetTime() + 3;
		iLen += sprintf( szTmp+iLen, "Expires: 0\r\n" ); // , dateexpire.FormatGmt(NULL) );
	}
	else
	{
		iLen += sprintf( szTmp+iLen, "Last-Modified: %s\r\n", 
			(LPCTSTR) CGTime(dateChange).FormatGmt(NULL));
	}

	/*
	iLen += sprintf( szTmp+iLen,
		"ETag: \"%s\"\r\n",	// No idea. "094e54b9f48bf1:1f83"
		(LPCTSTR) GetName()
		);
	*/

	iLen += sprintf( szTmp+iLen,
		"Content-Length: %d\r\n"
		"Connection: close\r\n"
		"\r\n",
		dwSize
		);

	pClient->xSendReady( szTmp, iLen );

	while (true)
	{
		iLen = FileRead.Read( szTmp, sizeof( szTmp ));
		if ( iLen <= 0 )
			break;
		pClient->xSendReady( szTmp, iLen );
		dwSize -= iLen;
		if ( iLen < sizeof( szTmp ))
		{
			// memset( szTmp+iLen, 0, sizeof(szTmp)-iLen );
			break;
		}
	}

	DEBUG_CHECK( dwSize == 0 );
	return(0);
}

static int GetHexDigit( TCHAR ch )
{
	if ( isdigit( ch ))
		return( ch - '0' );

	ch = toupper(ch);
	if ( ch > 'F' || ch <'A' )
		return( -1 );

	return( ch - ( 'A' - 10 ));
}

static int HtmlDeCode( TCHAR * pszDst, LPCTSTR pszSrc )
{
	int i=0;
	while ( true )
	{
		TCHAR ch = *pszSrc++;
		if ( ch == '+' )
		{
			ch = ' ';
		}
		else if ( ch == '%' )
		{
			ch = *pszSrc++;
			int iVal = GetHexDigit(ch);
			if ( ch )
			{
				ch = *pszSrc++;
				if ( ch )
				{
					ch = iVal*0x10 + GetHexDigit(ch);
					if ( ch == 0xa0 )
						ch = 0;
				}
			}
		}
		*pszDst++ = ch;
		if ( ch == 0 )
			break;
		i++;
	}
	return( i );
}

bool CWebPageDef::ServPagePost( CClient * pClient, LPCTSTR pszURLArgs, TCHAR * pContentData, int iContentLength )
{
	// RETURN: true = this was the page of interest.

	ASSERT(pClient);

	if ( pContentData == NULL || iContentLength <= 0 )
		return( false );
	if ( ! HasTrigger(XTRIG_UNKNOWN))	// this form has no triggers.
		return( false );

	// Parse the data.
	pContentData[iContentLength] = '\0';
	TCHAR * ppArgs[64];
	int iArgs = Str_ParseCmds( pContentData, ppArgs, COUNTOF(ppArgs), "&" );
	if ( iArgs <= 0 )
		return( false );

	// T or TXT or TEXT = the text fields.
	// B or BTN or BUTTON = the buttons
	// C or CHK or CHECK = the check boxes

	CDialogResponseArgs resp;
	DWORD dwButtonID = -1;
	for ( int i=0; i<iArgs; i++ )
	{
		TCHAR * pszNum = ppArgs[i];
		while ( isalpha(*pszNum))
			pszNum++;
		int iNum = atoi(pszNum);
		while ( *pszNum )
		{
			if ( *pszNum == '=' )
			{
				pszNum++;
				break;
			}
			pszNum++;
		}
		switch ( toupper(ppArgs[i][0]))
		{
		case 'B':
			dwButtonID = iNum;
			break;
		case 'C':
			if ( iNum == 0 )
				continue;
			if ( atoi( pszNum ))
			{
				resp.m_CheckArray.Add( iNum );
			}
			break;
		case 'T':
			if ( iNum )
			{
				TCHAR *pszData = Str_GetTemp();
				HtmlDeCode( pszData, pszNum );
				resp.AddText( iNum, pszData );
			}
			break;
		}
	}

	// Use the data in RES_WEBPAGE block.
	CResourceLock s;
	if ( ! ResourceLock(s))
		return( false );

	// Find the correct entry point.
	while ( s.ReadKeyParse())
	{
		if ( ! s.IsKeyHead( "ON", 2 ))
			continue;
		if ( s.GetArgVal() != dwButtonID )
			continue;
		TRIGRET_TYPE iRet = OnTriggerRun( s, TRIGRUN_SECTION_TRUE, pClient, &resp );
		return( true );
	}

	// Put up some sort of failure page ?

	return( false );
}

bool CWebPageDef::ServPage( CClient * pClient, TCHAR * pszPage, CGTime * pdateIfModifiedSince )	// static
{
	// make sure this is a valid format for the request.

	TCHAR szPageName[ _MAX_PATH ];
	int lenPageName = Str_GetBare( szPageName, pszPage, sizeof(szPageName), "!\"#$%&()*,:;<=>?[]^{|}-+'`" );

	int iError = 404;
	CWebPageDef * pWebPage = g_Cfg.FindWebPage(szPageName);
	if ( pWebPage )
	{
		iError = pWebPage->ServPageRequest( pClient, szPageName, pdateIfModifiedSince );
		if ( ! iError )
			return true;
	}

	// Is it a file in the Script directory ?
	if ( iError == 404 )
	{
		const RESOURCE_ID ridjunk( RES_UNKNOWN, 1 );
		CWebPageDef tmppage( ridjunk );
		if ( tmppage.SetSourceFile( szPageName, pClient ))
		{
			if ( ! tmppage.ServPageRequest( pClient, szPageName, pdateIfModifiedSince ))
				return true;
		}
	}

	// Can't find it !?
	// just take the default page. or have a custom 404 page ?

	pClient->m_Targ_Text = pszPage;

	CGString sErrorPage;
	sErrorPage.Format( GRAY_FILE "%d.htm", iError );

	pWebPage = g_Cfg.FindWebPage( sErrorPage );
	if ( pWebPage )
	{
		if ( ! pWebPage->ServPageRequest( pClient, pszPage, NULL ))
			return true;
	}

	// Hmm we should do something !!!?
	// Try to give a reasonable default error msg.

	LPCTSTR pszErrText;
	switch (iError)
	{
	case 401: pszErrText = "Authorization Required"; break;
	case 403: pszErrText = "Forbidden"; break;
	case 404: pszErrText = "Object Not Found"; break;
	case 500: pszErrText = "Internal Server Error"; break;
	default: pszErrText = "Unknown Error"; break;
	}

	CGTime datetime = CGTime::GetCurrentTime();
	CGString sDate = datetime.FormatGmt(NULL);
	CGString sMsgHead;
	CGString sText;

		sText.Format(
			"<html><head><title>Error %d</title>\r\n"
			"<meta name=\"robots\" content=\"noindex\">\r\n"
			"<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\"></head>\r\n"
			"<body>\r\n"
			"<h2>HTTP Error %d</h2>\r\n"
			"<p><strong>%d %s</strong></p>\r\n"
			"<p>The %s server cannot deliver the file or script you asked for.</p>\r\n"
			"<p>Please contact the server's administrator if this problem persists.</p>\r\n"
			"</body></html>\r\n",
			iError,
			iError,
			iError,
			(LPCTSTR) pszErrText,
			(LPCTSTR)g_Cfg.m_sDisguiseName.GetPtr());

		sMsgHead.Format(
			"HTTP/1.1 %d %s\r\n"
			"Date: %s\r\n"
			"Server: %s V %s\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: %d\r\n"
			"Connection: close\r\n"
			"\r\n%s",
			iError, (LPCTSTR) pszErrText,
			(LPCTSTR) sDate,
			(LPCTSTR)g_Cfg.m_sDisguiseName.GetPtr(),
			(LPCTSTR) g_Cfg.m_sDisguiseVersion.GetPtr(),
			sText.GetLength(),
			(LPCTSTR) sText );

	pClient->xSendReady( (LPCTSTR) sMsgHead, sMsgHead.GetLength() );
	return( false );
}

