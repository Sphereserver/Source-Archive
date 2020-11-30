//
// GRAYSRV.CPP.
// Copyright Menace Software (www.menasoft.com).
//
// game/login server for uo client
// http://www.menasoft.com for more details.
// I'm liking http://www.cs.umd.edu/users/cml/cstyle/CppCodingStandard.html as a coding standard.
//
// [menace@unforgettable.com] Dennis Robinson, http://www.menasoft.com
// [nixon@mediaone.net] Nix, Scripts
// [petewest@mindspring.com] Westy, /INFO, Maps
// [johnr@cyberstreet.com] John Garion (The Answer Guy) docs.web pages.
// [bytor@mindspring.com] Philip Esterle (PAE)(Altiara Dragon) tailoring,weather,moons,
// [will@w_tigre.cuug.ab.ca] Will Merkins  (Smeg) LINUX
//
// UNKNOWN STATUS:
// [allmight@rospiggen.com] Mans Sjoberg (Allmight)
// [gander@jester.vte.com] Gerald D. Anderson (ROK) mailing list
// [kylej@blueworld.com] Kyle Jessup (KRJ) Line of sight.
// [udc@home.com] ZARN Brad Patten,
// [kmayers@gci.net] Keif Mayers (Avernus)
// [damiant@seanet.com] Damian Tedrow , http://www.ayekan.com/awesomedev
// [alluvian@onramp.net] Alluvian, Charles Manick Livermore
//
// Current:
//  Stam penalties for Weight carried. (Platemail penalties)
//	Unconscious state.
//  dispel blades.
//  vendor economy.
//  NPC AI
//  Interserver registration.
//
// Top Issues:
// duplicate accounts.
//  new mining/smelting system. more than 1 ingot for smelting 1 ore.
//  scripting files . char2.scp, binary or not for char.scp
//  weapon table for speed and damage. DEX for speed.
//
// Wish list:
//  chop sound.
//  inscribe magic weapons, wands from menu
//  Magery skill degradation for plate and weapons in hand.
//  Skill sticking. set a skill so it will stick at a value and not degrade.
//  NPC smith repair.
//  GrandMaster Created items are better and labled with name of the creator.
//  Portrait of PC. or profile support
//  Dex as part of AR ?
//  Stable limits are per player/account based.
//
// Protocol Issues:
// 1. Looking through all the gump menus for the house.
// 2. placing an item like a forge or any other deeded item. (xcept houses and ships, i know how to do those.)
// 3. casting a gate spell. ok to watch someone else do it.
// 4. Watching someone else drag an item from a container on the ground to their pack. and visa versa.
// 5. Watching someone else drag an item from a pack to the ground. and visa versa.
// 6. Adding a person to the party system.
// 7. Chop a tree. (i want to get the proper sound)
// 8. Bring up a player profile
//
// BUGS:
//   npc sheild item. block npc's from walking here .
//   bless > 100 will lose stats
//   account update before savbe ?
//
// 2. Hiding does not work right.
//	You are still attacked by monsters when hidden.
// 3. Fletching does not work right.
//	If you have 100 shafts and 100 feathers and you fail to make arrow/bolts you lose everything.
// 4. Combat is EXTREMELY SLOW.
// 5. There are some truly Physotic Monsters out there.
//	For example: There are these archers that look like Orcish Lords that shot 4 - 5 arrows at a time.
//	So by the time you swing at them once you have 15 - 20 arrows in ya and your TOAST.
// 6. Targeting after casting.
//	If you are attacking a monster with a sword and cast a spell you lose your target. You just stand
//	there while the monster rips you a new asshole until you 2x click and attack him again.
//
// ADDNPC from scripts, "CONT" keyword

#include "graysvr.h"	// predef header.

const TCHAR * g_Stat_Name[STAT_QTY] =
{
	"STR",
	"INT",
	"DEX",
	"KARMA",
	"FAME",
};

const CPointMap g_pntLBThrone(1323,1624,0); // This is OSI's origin for degree, sextant minute coordinates

const TCHAR * g_szServerDescription =
#ifdef _WIN32
GRAY_TITLE " Version " GRAY_VERSION " [WIN32] by " GRAY_URL;
#else
GRAY_TITLE " Version " GRAY_VERSION " [LINUX] by " GRAY_URL;
#endif

int CObjBase::sm_iCount = 0;	// UID table.

// game servers stuff.
CWorld		g_World;	// the world. (we save this stuff)
CServer		g_Serv;	// current state stuff not saved.
CMain		g_Main;
CGrayInstall g_Install;
CVerDataMul	g_VerData;
CExpression g_Exp;	// Global script variables.
CLog		g_Log;
CEventLog * g_pLog = &g_Log;

//////////////////////////////////////////////////////////////////
// util type stuff.

extern void globalstartsymbol();

//
// Provide these functions globally.
//

#define MEMORY_ALLOC_SIGNATURE	0xBADF00D
#ifdef _DEBUG
size_t DEBUG_ValidateAlloc( const void * pThis )
{
	// make sure the dynamic allocation below is valid.
	pThis = ( const BYTE*) pThis - sizeof(size_t);
	size_t stAllocateBlock = *((size_t*)pThis);
	ASSERT( stAllocateBlock != -1 );
	ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );
	return( stAllocateBlock );
}
#endif

void * _cdecl operator new( size_t stAllocateBlock )
{
	stAllocateBlock += sizeof(size_t) + sizeof(DWORD);

	void * pThis = malloc( stAllocateBlock );
	ASSERT(pThis);
	*((size_t*)pThis) = stAllocateBlock;
	*((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = MEMORY_ALLOC_SIGNATURE;

	g_Serv.StatChange(SERV_STAT_MEM, stAllocateBlock );
	g_Serv.StatInc(SERV_STAT_ALLOCS);
	return((BYTE*) pThis + sizeof(size_t));
}

void * _cdecl operator new[]( size_t stAllocateBlock )
{
	stAllocateBlock += sizeof(size_t) + sizeof(DWORD);

	void * pThis = malloc( stAllocateBlock );
	ASSERT(pThis);
	*((size_t*)pThis) = stAllocateBlock;
	*((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = MEMORY_ALLOC_SIGNATURE;

	g_Serv.StatChange(SERV_STAT_MEM, stAllocateBlock );
	g_Serv.StatInc(SERV_STAT_ALLOCS);
	return((BYTE*) pThis + sizeof(size_t));
}

void _cdecl operator delete( void * pThis )
{
	if ( pThis == NULL )
	{
		DEBUG_ERR(("delete:NULL\n" ));
		return;
	}

	pThis = (BYTE*) pThis - sizeof(size_t);
	size_t stAllocateBlock = *((size_t*)pThis);
	ASSERT( stAllocateBlock != -1 );
	ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );

	g_Serv.StatChange(SERV_STAT_MEM, - (long) stAllocateBlock );
	g_Serv.StatDec(SERV_STAT_ALLOCS);

	// invalidate the block.
	*((size_t*)pThis) = -1;
	*((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = 0;

	free( pThis );
}

void _cdecl operator delete[]( void * pThis )
{
	if ( pThis == NULL )
	{
		DEBUG_ERR(("delete:NULL\n" ));
		return;
	}

	pThis = (BYTE*) pThis - sizeof(size_t);
	size_t stAllocateBlock = *((size_t*)pThis);
	ASSERT( stAllocateBlock != -1 );
	ASSERT( *((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) == MEMORY_ALLOC_SIGNATURE );

	g_Serv.StatChange(SERV_STAT_MEM, - (long) stAllocateBlock );
	g_Serv.StatDec(SERV_STAT_ALLOCS);

	// invalidate the block.
	*((size_t*)pThis) = -1;
	*((DWORD*)(((BYTE*)pThis)+stAllocateBlock-sizeof(DWORD))) = 0;

	free( pThis );
}

#ifdef _DEBUG
void Debug_CheckFail( const TCHAR * pExp, const TCHAR *pFile, unsigned uLine )
{
	g_Log.Event( LOGL_ERROR, "Check Fail:'%s' file '%s', line %d\n", pExp, pFile, uLine );
}

void Debug_Assert_CheckFail( const TCHAR * pExp, const TCHAR *pFile, unsigned uLine )
{
	g_Log.Event( LOGL_CRIT, "Debug Assert:'%s' file '%s', line %d\n", pExp, pFile, uLine );
	// if ( g_Serv.m_fSecure )	{}
	throw( CGrayError( LOGL_CRIT, 0, pExp ));
}

#endif

#ifndef VISUAL_SPHERE
void Assert_CheckFail( const TCHAR * pExp, const TCHAR *pFile, unsigned uLine )
{
	// These get left in in relesae code .
	g_Log.Event( LOGL_CRIT, "Assert:'%s' file '%s', line %d\n", pExp, pFile, uLine );
	// if ( g_Serv.m_fSecure )	{}
	throw( CGrayError( LOGL_CRIT, 0, pExp ));
}
#endif // VISUAL_SPHERE

#ifdef _WIN32

extern "C" {

#ifndef VISUAL_SPHERE
_CRTIMP void _cdecl _assert( void *pExp, void *pFile, unsigned uLine )
{
	// trap the system version of this just in case.
	Assert_CheckFail((const char*) pExp, (const char*) pFile, uLine );
}
#endif // VISUAL_SPHERE

class CGrayException : public CGrayError
{
	// Catch and get details on the system exceptions.
	// NULL pointer access etc.
public:
	unsigned int m_uCode;
	DWORD m_dwAddress;
public:
	CGrayException( unsigned int uCode, DWORD dwAddress )
		: CGrayError( LOGL_CRIT, 0, _TEXT("fault"))
	{
		m_uCode = uCode;
		m_dwAddress = dwAddress;
	}
};

#ifndef VISUAL_SPHERE
void _cdecl Exception_Win32( unsigned int id, struct _EXCEPTION_POINTERS* pData )
{
	// id = 0xc0000094 for divide by zero.
	// STATUS_ACCESS_VIOLATION is 0xC0000005.

	DWORD dwCodeStart = (DWORD)(PVOID) globalstartsymbol;
	//	_asm mov dwCodeStart, CODE

	throw( CGrayException( id, (DWORD)( pData->ExceptionRecord->ExceptionAddress ) - dwCodeStart ));
}
#endif // VISUAL_SPHERE

int _cdecl _purecall()
{
	// catch this type of exception as well.
	Assert_CheckFail( "purecall", "unknown", 1 );
	return 0;
}

#if 0
// try to trap some of the other strange exit conditions !!!
void _cdecl _amsg_exit( int iArg )
{
	Assert_CheckFail( "_amsg_exit", "unknown", 1 );
}
#endif

}	// extern "C"

#ifndef VISUAL_SPHERE
BOOL WINAPI ConsoleHandlerRoutine( DWORD dwCtrlType )
{
	//  control signal type. CTRL_C_EVENT
	// SetConsoleCtrlHandler
	switch ( dwCtrlType )
	{
	case CTRL_C_EVENT: // A CTRL+c signal was received, either from keyboard input or from a signal generated by the GenerateConsoleCtrlEvent function.
	case CTRL_BREAK_EVENT: //  A CTRL+BREAK signal was received, either from keyboard input or from a signal generated by GenerateConsoleCtrlEvent.
	case CTRL_CLOSE_EVENT: // A signal that the system sends to all processes attached to a console when the user closes the console (either by choosing the Close command from the console window's System menu, or by choosing the End Task command from the Task List).
		if ( g_Serv.m_fSecure && ! g_Serv.IsLoading())	// enable the try code.
		{
			g_Serv.SysMessage( "Press 'y' to exit safely, 'x' to exit immediatly." );
			while (true)
			{
				int ch = _getch();
				if ( ch == 0 )
					continue;
				ch = toupper( ch );
				if ( ch == 'X' )
					return( FALSE );
				if ( ch != 'Y' )
					return( TRUE );
				break;
			}
			g_Serv.m_wExitFlag = 0x11;
			return( TRUE );
		}
		break;
	case CTRL_LOGOFF_EVENT: // A signal that the system sends to all console processes when a user is logging off. This signal does not indicate which user is logging off, so no assumptions can be made.
	case CTRL_SHUTDOWN_EVENT: // A signal that the system sends to all console processes when the system is shutting down
		break;
	}
	return FALSE;	// process normally.
}
#endif // VISUAL_SPHERE

#endif	// _WIN32

DIR_TYPE GetDirStr( const TCHAR * pszDir )
{
	int iDir = toupper( pszDir[0] );
	int iDir2 = 0;
	if ( iDir ) iDir2 = toupper( pszDir[1] );
	switch ( iDir )
	{
	case 'E': return( DIR_E );
	case 'W': return( DIR_W );
	case 'N':
		if ( iDir2 == 'E' ) return( DIR_NE );
		if ( iDir2 == 'W' ) return( DIR_NW );
		return( DIR_N );
	case 'S':
		if ( iDir2 == 'E' ) return( DIR_SE );
		if ( iDir2 == 'W' ) return( DIR_SW );
		return( DIR_S );
	}
	return( DIR_QTY );
}

ITEMID_TYPE GetResourceItem( const TCHAR * & pRes )
{
	static const WORD Resource[] = // preserve the old defs.
	{
		'BW',ITEMID_REAG_1, 	// ITEMID_REAG_1,	//'Batwing'
		'BL',ITEMID_REAG_1+1, 	//'Blackmoor'
		'BP',ITEMID_REAG_BP, 	//'Black Pearl'
		'BM',ITEMID_REAG_BM, 	//'Blood Moss'
		'BS',ITEMID_REAG_1+4, 	//'Bloodspawn'
		'VB',ITEMID_REAG_1+5, 	//'Vial of Blood'
		'BO',ITEMID_REAG_1+6, 	//'Bone'
		'BR',ITEMID_REAG_1+7, 	//'Brimstone'
		'DA',ITEMID_REAG_1+8, 	//'Daemon Bone'
		'FD',ITEMID_REAG_1+9, 	//'Fertile Dirt'
		'DB',ITEMID_REAG_1+10, 	//'Dragon's Blood'
		'EC',ITEMID_REAG_1+11, 	//'Executioner's Cap'
		'GA',ITEMID_REAG_GA, 	//'Garlic'
		'GI',ITEMID_REAG_GI, 	//'Ginseng'
		'MR',ITEMID_REAG_MR, 	//'Mandrake Root'
		'EN',ITEMID_REAG_1+15, 	//'Eye of Newt'
		'NS',ITEMID_REAG_NS, 	//'Nightshade'
		'OB',ITEMID_REAG_1+17, 	//'Obsidian'
		'PI',ITEMID_REAG_1+18, 	//'Pig Iron'
		'PU',ITEMID_REAG_1+19, 	//'Pumice'
		'SA',ITEMID_REAG_SA,	//'Sulfurous Ash'
		'SS',ITEMID_REAG_SS,	//'Spider's Silk'
		'SE',ITEMID_REAG_1+22,	//'Serpent's Scales'
		'VA',ITEMID_REAG_1+23,	//'Volcanic Ash'
		'DW',ITEMID_REAG_1+24,	//'Dead Wood '
		'WH',ITEMID_REAG_26,	//'Worm's Heart'

		'ME', ITEMID_FOOD_MEAT_RAW,
		'FE', ITEMID_FEATHERS3,
		'HI', ITEMID_HIDES,
		'LE', ITEMID_HIDES,
		'FU', ITEMID_FURS,
		'WO', ITEMID_WOOL,

		'IN', ITEMID_INGOT_IRON,
		'IG', ITEMID_INGOT_GOLD,
		'IS', ITEMID_INGOT_SILVER,
		'IC', ITEMID_INGOT_COPPER,

		'LO', ITEMID_LOG_1,
		'SH', ITEMID_SHAFTS3,
		'BT', ITEMID_EMPTY_BOTTLE,	// For alchemy.
		'SC', ITEMID_SCROLL2_BLANK,

		'CL', ITEMID_CLOTH1,    // For tailoring
		'HA', ITEMID_HAY,		// For straw hats
		'FL', ITEMID_FLOUR,		// baking bread etc.
	};

	GETNONWHITESPACE( pRes );	// Skip leading spaces.

	if ( pRes == NULL )
		return( ITEMID_NOTHING );

	if ( isalpha( pRes[0] ) && isalpha( pRes[1] ) && ! isalpha( pRes[2] ))
	{
		WORD wResource = (((WORD) toupper(pRes[0])) << 8 ) | toupper(pRes[1]);
		for ( int i=0; i<COUNTOF(Resource); i+=2 )
		{
			if ( wResource == Resource[i] )
			{
				pRes += 2;
				return((ITEMID_TYPE) ( Resource[i+1] ));
			}
		}
	}

	return((ITEMID_TYPE) Exp_GetSingle( pRes ));
}

ITEMID_TYPE GetResourceEntry( const TCHAR * & pRes, int & iQty )
{
	if ( pRes == NULL )
		return( ITEMID_NOTHING );

	GETNONWHITESPACE( pRes );	// Skip leading spaces.
	if ( pRes[0] == '\0' )
		return( ITEMID_NOTHING );

	if ( isdigit( pRes[0] ))
	{
		iQty = Exp_GetSingle( pRes );
	}
	else
	{
		iQty = 1;
	}
	if ( *pRes == ',' )
	{
		++pRes;
	}
	ITEMID_TYPE id = GetResourceItem( pRes );
	if ( *pRes == ',' )
	{
		++pRes;
	}
	return( id );
}

const TCHAR * GetTimeMinDesc( int minutes )
{
	if ( minutes < 0 )
	{
		TCHAR * pTime = GetTempStr();
		strcpy( pTime, "Hmmm...I can't tell what time it is!");
		return( pTime );
	}
	int minute = minutes % 60;
	int hour = ( minutes / 60 ) % 24;

	const TCHAR * pMinDif;
	if (minute<=14) pMinDif = "";
	else if ((minute>=15)&&(minute<=30)) pMinDif = " a quarter past";
	else if ((minute>=30)&&(minute<=45)) pMinDif = " half past";
	else
	{
		pMinDif = " a quarter till";
		hour = ( hour + 1 ) % 24;
	}

	static const TCHAR * ClockHour[] =
	{
		"midnight",
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine",
		"ten",
		"eleven",
		"noon",
	};

	const TCHAR * pTail;
	if ( hour == 0 || hour==12 )
		pTail = "";
	else if ( hour > 12 )
	{
		hour -= 12;
		if ((hour>=1)&&(hour<6))
			pTail = " o'clock in the afternoon";
		else if ((hour>=6)&&(hour<9))
			pTail = " o'clock in the evening.";
		else
			pTail = " o'clock at night";
	}
	else
	{
		pTail = " o'clock in the morning";
	}

	TCHAR * pTime = GetTempStr();
	sprintf( pTime, "It is%s %s%s.", pMinDif, ClockHour[hour], pTail );
	return( pTime );
}

int FindID( WORD id, const WORD * pID, int iCount )
{
	for ( int i=0; i < iCount; i++ )
	{
		if ( pID[i] == id )
			return( i );
	}
	return( -1 );
}

int FindStrWord( const TCHAR * pTextSearch, const TCHAR * pszKeyWord )
{
	// Find the pszKeyWord in the pTextSearch string.
	// Make sure we look for starts of words.

	int j=0;
	for ( int i=0; 1; i++ )
	{
		if ( pszKeyWord[j] == '\0' )
		{
			if ( pTextSearch[i]=='\0' || ISWHITESPACE(pTextSearch[i]))
				return( i );
			return( 0 );
		}
		if ( pTextSearch[i] == '\0' )
			return( 0 );
		if ( !j && i )
		{
			TCHAR ch = toupper( pTextSearch[i-1] );
			if ( ch >= 'A' && ch <= 'Z' )	// not start of word ?
				continue;
		}
		if ( toupper( pTextSearch[i] ) == toupper( pszKeyWord[j] ))
			j++;
		else
			j=0;
	}
}

///////////////////////////////////////////////////////////////
// -CLog

bool CLog::Open( TCHAR * pszBaseDirName )	// name set previously.
{
	if ( pszBaseDirName != NULL )
	{
		if ( pszBaseDirName[0] && pszBaseDirName[1] == '\0' )
		{
			if ( *pszBaseDirName == '0' )
			{
				Close();
				return false;
			}
		}
		else
		{
			m_sBaseDir = pszBaseDirName;
		}
	}

	// Get the new name based on date.
	m_Stamp.FillRealTime();
	CGString sName;
	sName.Format( GRAY_FILE "%d%02d%02d.log", m_Stamp.m_Year, m_Stamp.m_Month+1, m_Stamp.m_Day );

	return( CFileText::Open( GetMergedFileName( m_sBaseDir, sName ), OF_READWRITE|OF_WRITE|OF_TEXT ));
}

int CLog::EventStr( WORD wMask, const TCHAR * pszMsg )
{
	if ( ! IsLogged( wMask ))	// I don't care about these.
		return( 0 );

	static bool fSpinLock = false;	// make sure we don't go reentrant here.
	if ( fSpinLock )
		return( 0 );
	fSpinLock = true;

	try
	{

	ASSERT( pszMsg && * pszMsg );

	// Put up the date/time.
	CRealTime NewStamp;			// last real time stamp.
	NewStamp.FillRealTime();

	if ( NewStamp.m_Day != m_Stamp.m_Day ||
		NewStamp.m_Month != m_Stamp.m_Month ||
		NewStamp.m_Year != m_Stamp.m_Year )
	{
		// it's a new day, open with new day name.
		Close();	// LINUX should alrady be closed.
		Open();
		Printf( NewStamp.Write());
		Printf( "\n" );
	}
	else
	{
#ifndef _WIN32
		Open();	// LINUX needs to close and re-open for each log line !
#endif
	}

	CGString sTime;
	sTime.Format( "%02d:%02d:", NewStamp.m_Hour, NewStamp.m_Min );
	m_Stamp = NewStamp;

	const TCHAR * pszLabel = NULL;

	switch ( wMask & 0x07 )
	{
	case LOGL_FATAL:	// fatal error !
		pszLabel = "FATAL:";
		break;
	case LOGL_CRIT:	// critical.
		pszLabel = "CRITICAL:";
		break;
	case LOGL_ERROR:	// non-fatal errors.
		pszLabel = "ERROR:";
		break;
	case LOGL_WARN:
		pszLabel = "WARNING:";
		break;
	}

	// Get the script context. (if there is one)
	CGString sScriptContext;
	if ( m_pScriptContext )
	{
		sScriptContext.Format( "(%s,%d)", m_pScriptContext->GetFileTitle(), m_pScriptContext->GetLineNumber());
	}

	// Print to screen.
	if ( ! ( wMask & LOGM_INIT ) && ! g_Serv.IsLoading())
	{
		g_Serv.PrintStr( sTime );
	}
	if ( pszLabel )	g_Serv.PrintStr( pszLabel );
	if ( ! sScriptContext.IsEmpty())
	{
		g_Serv.PrintStr( sScriptContext );
	}
	g_Serv.PrintStr( pszMsg );

	// Print to log file.
	WriteStr( sTime );
	if ( pszLabel )	WriteStr( pszLabel );
	if ( ! sScriptContext.IsEmpty())
	{
		WriteStr( sScriptContext );
	}
	WriteStr( pszMsg );

	}
	catch (...)
	{
		fSpinLock = false;
#ifndef _WIN32
		Close();
#endif
		return( 0 );
	}

	fSpinLock = false;
#ifndef _WIN32
	Close();
#endif
	return( 1 );
}

void CLog::Dump( const BYTE * pData, int len )
{
	// Just dump a bunch of bytes. 16 bytes per line.
	while ( len )
	{
		TCHAR szTmp[16*3+10];
		int j=0;
		for ( ; j < 16; j++ )
		{
			if ( ! len ) break;
			sprintf( szTmp+(j*3), "%02x ", * pData );
			len --;
			pData ++;
		}
		strcpy( szTmp+(j*3), "\n" );
		g_Serv.PrintStr( szTmp );
		WriteStr( szTmp );	// Print to log file.
	}
}

////////////////////////////////////////////////////////////////////////
// -CMain

#ifdef _WIN32
void _cdecl CMain::EntryProc( void * lpThreadParameter ) // static
#else
void * _cdecl CMain::EntryProc( void * lpThreadParameter ) // static
#endif
{
	g_Main.OnCreate();
	while ( ! g_Serv.m_wExitFlag )
	{
#ifdef _WIN32
		// not used in Linux
		if ( g_Service.IsServiceStopPending())
		{
			// Let the SCM know we aren't ignoring it
			g_Service.ReportStatusToSCMgr( SERVICE_STOP_PENDING, NO_ERROR, 1000 );
			if (!g_World.IsSaving())
				g_Serv.m_wExitFlag = 0xBABE;
		}
#endif

		if ( g_Serv.m_fSecure )	// enable the try code.
		{
			static time_t preverrortick = 0;
			try
			{
				g_World.OnTick();
				g_Serv.OnTick();
			}

#ifdef _WIN32
			catch ( CGrayException e )
			{
				if ( preverrortick != g_World.GetTime())
				{
					g_Log.Event( LOGL_CRIT, "Unhandled Exception time=%d, code=0%0x, addr=0%0x!!\n", g_World.GetTime(), e.m_uCode, e.m_dwAddress );
					preverrortick = g_World.GetTime();
				}
			}
#endif
			catch (...)	// catch all
			{
				if ( preverrortick != g_World.GetTime())
				{
					g_Log.Event( LOGL_CRIT, "Unhandled Exception time=%d!!\n", g_World.GetTime());
					preverrortick = g_World.GetTime();
				}
				// Try garbage collection here ! Try to recover ?
			}
		}
		else
		{
			g_World.OnTick();
			g_Serv.OnTick();
		}
	}

	g_Main.OnClose();
}

/////////////////////////////////////////////////////////////////

#ifdef _WIN32
void MainMonitorLoop()
{
	// just make sure the main loop is alive every so often.
	// try to restart it if it is not.

	time_t restarttime = 0;
	time_t warntime = 0;
	time_t prevtime = 0;

	while ( ! g_Serv.m_wExitFlag )
	{
		if ( g_Serv.m_iFreezeRestartTime <= 0 )
		{
			DEBUG_ERR(( "Freeze Restart Time cannot be cleared at run time\n" ));
			g_Serv.m_iFreezeRestartTime = 10*TICK_PER_SEC;
		}
		Sleep( g_Serv.m_iFreezeRestartTime * 1000 / TICK_PER_SEC );
		time_t curtime = g_World.GetTime();
		if ( g_Serv.m_fResyncPause )
			continue;
		if ( ! g_Serv.m_fSecure )
			continue;
		if ( g_World.IsSaving() && g_World.m_fSaveForce )
			continue;
		if ( curtime == prevtime )
		{
			if ( restarttime == curtime )
			{
				g_Log.Event( LOGL_FATAL, "Main loop freeze RESTART FAILED!\n" );
				//g_Serv.m_wExitFlag = 0xDEAD;
				//g_Serv.m_iExitCode = -1234;
				//return;
				//_asm int 3;
			}

			if ( warntime == curtime )
			{
				// Kill and revive the main process
				g_Log.Event( LOGL_CRIT, "Main loop freeze RESTART!\n" );

#ifndef _DEBUG
				g_Main.TerminateThread( 0xDEAD );

				// try to restart it.
				g_Main.CreateThread( CMain::EntryProc );
#endif
				restarttime = curtime;
			}
			else
			{
				g_Log.Event( LOGL_WARN, "Main loop frozen ?\n" );
				warntime = curtime;
			}
		}

		prevtime = curtime;
	}

	g_Main.WaitForClose( 10 );
}
#endif

/////////////////////////////////////////////////////////////////
// extern void NTWindowFind();

#ifdef _WIN32
int _cdecl SphereMainEntryPoint( int argc, char *argv[] )
#else
int _cdecl main(int argc, char * argv[])
#endif
{
	ASSERT( MAX_BUFFER >= sizeof( CCommand ));
	ASSERT( MAX_BUFFER >= sizeof( CEvent ));
	ASSERT( sizeof( int ) == sizeof( DWORD ));	// make this assumption often.
	ASSERT( sizeof( ITEMID_TYPE ) == sizeof( DWORD ));
	ASSERT( sizeof( WORD ) == 2 );
	ASSERT( sizeof( DWORD ) == 4 );
	ASSERT( sizeof( NWORD ) == 2 );
	ASSERT( sizeof( NDWORD ) == 4 );
	ASSERT(( UO_BLOCKS_X % SECTOR_SIZE_X ) == 0 );
	ASSERT(( UO_BLOCKS_Y % SECTOR_SIZE_Y ) == 0 );
	ASSERT( sizeof(CUOItemTypeRec) == 37 );	// byte pack working ?

#ifdef _WIN32
	SetConsoleTitle( GRAY_TITLE " V" GRAY_VERSION );
#ifndef VISUAL_SPHERE
	_set_se_translator( Exception_Win32 );
	SetConsoleCtrlHandler( ConsoleHandlerRoutine, TRUE );
#endif // VISUAL_SPHERE
 	// NTWindowFind();
#endif

	g_Log.Event( LOGM_INIT, "\n%s\n"
		"Compiled at " __DATE__ " (" __TIME__ ")\n"
		"\n",
		g_szServerDescription );

	if ( ! g_Serv.Load())
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "The " GRAY_FILE ".INI file is corrupt or missing\n" );
		g_Serv.m_iExitCode = -1;
		goto world_bail;
	}

	if ( ! g_World.Load())
	{
		g_Serv.m_iExitCode = -8;
		goto world_bail;
	}

	if ( argc > 1 )
	{
		// Do special debug type stuff.
		g_Serv.m_iExitCode = 2;
		if ( ! g_Serv.CommandLine( argc, argv ))
		{
			goto world_bail;
		}
		g_Serv.m_iExitCode = 1;
	}

	if ( ! g_Serv.SocketsInit())
	{
		g_Serv.m_iExitCode = -9;
		goto socket_bail;
	}

	g_Log.Event( LOGM_INIT, g_Serv.GetStatusString( 0x24 ));
	g_Log.Event( LOGM_INIT, _TEXT("Startup complete. items=%d, chars=%d\n"), g_Serv.StatGet(SERV_STAT_ITEMS), g_Serv.StatGet(SERV_STAT_CHARS));

#ifdef _WIN32
	g_Log.Event( LOGM_INIT, "Press '?' for console commands\n" );

	if ( g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		g_Serv.m_iFreezeRestartTime )
	{
		g_Main.CreateThread( CMain::EntryProc );
		MainMonitorLoop();
	}
	else
	{
		g_Main.EntryProc( 0 );
	}
#else
	g_Main.EntryProc( 0 );
#endif

socket_bail:
	g_Serv.SocketsClose();
world_bail:
	g_World.Close();

#ifdef _WIN32
#ifndef VISUAL_SPHERE
	SetConsoleCtrlHandler( ConsoleHandlerRoutine, FALSE );
#endif // VISUAL_SPHERE
#endif

	if ( g_Serv.m_iExitCode )
	{
		g_Log.Event( LOGL_FATAL, "Server terminated by error %d!\n", g_Serv.m_iExitCode );
#ifdef _WIN32
		g_Serv.SysMessage( "Press any key to exit" );
		while ( _getch() == 0 ) ;
#endif
	}
	else
	{
		g_Log.Event( LOGL_EVENT, "Server shutdown complete!\n");
	}
	g_Log.Close();

	return( g_Serv.m_iExitCode );
}
