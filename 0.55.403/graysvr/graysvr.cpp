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
// [johnr@cyberstreet.com] John Davey (Garion)(The Answer Guy) docs.web pages.
// [bytor@mindspring.com] Philip Esterle (PAE)(Altiara Dragon) tailoring,weather,moons,LINUX
//
// UNKNOWN STATUS:
// [will@w_tigre.cuug.ab.ca] Will Merkins  (Smeg) LINUX
// [allmight@rospiggen.com] Mans Sjoberg (Allmight)
// [gander@jester.vte.com] Gerald D. Anderson (ROK) mailing list
// [kylej@blueworld.com] Kyle Jessup (KRJ) Line of sight.
// [udc@home.com] ZARN Brad Patten,
// [kmayers@gci.net] Keif Mayers (Avernus)
// [damiant@seanet.com] Damian Tedrow , http://www.ayekan.com/awesomedev
// [alluvian@onramp.net] Alluvian, Charles Manick Livermore
//
// Current:
//  Casting penalties for plate
//	Unconscious state.
//  vendor economy.
//  NPC AI
//  Interserver registration.
//
// Top Issues:
//  new mining/smelting system. more than 1 ingot for smelting 1 ore.
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
// 2. placing an item like a forge or any other deeded item. (xcept houses and ships, i know how to do those.)
// 9. grave stone and sign gumps ?
// 4. Watching someone else drag an item from a container on the ground to their pack. and visa versa.
// 5. Watching someone else drag an item from a pack to the ground. and visa versa.
// 6. Adding a person to the party system.
// x. Looking through all the gump menus for the house.
// x. casting a gate spell. ok to watch someone else do it.
// x. Bring up a player profile
// x. Chop a tree. (i want to get the proper sound)
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

#ifdef VISUAL_SPHERE
	#include "..\VisualSphere\stdafx.h"
	#include "..\VisualSphere\VisualSphere.h"
	#include "..\VisualSphere\ServerObject.h"
#else
	#include "graysvr.h"	// predef header.
#include "../common/grayver.h"	// sphere version
#endif

#ifndef BUILD_WITHOUT_IRCSERVER
	#include "CIRCServer.h"
#endif
#ifdef _WIN32
#include "ntservice.h"	// g_Service
#else
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>



int	ATOI( const char * str )
{
	int	res;
	sscanf( str, "%d", &res );
	return res;
}

char * LTOA(long value, char *string, int radix)
{
	switch (radix) {
		case 10:
		default:
			sprintf(string, "%ld", value);
			break;
		case 16:
			sprintf(string, "%lx", value);
			break;
	}
	
	return string;
}

#endif

LPCTSTR const g_Stat_Name[STAT_QTY] =	// not sorted obviously.
{
	"STR",
	"INT",
	"DEX",
	"FOOD",
	"KARMA",
	"FAME",

	// "OSTR",
	// "OINT",
	// "ODEX",
	// "MAXHITS",
	// "MAXMANA",
	// "MAXSTAM",
};

const CPointMap g_pntLBThrone(1323,1624,0); // This is origin for degree, sextant minute coordinates

LPCTSTR g_szServerDescription =
	"%s Version %s "
#ifdef _WIN32
	"[WIN32]"
#else
#ifdef _BSD
	"[FreeBSD]"
#else
	"[Linux]"
#endif
#endif
#ifdef _DEBUG
	"[DEBUG]"
#endif
	" by " GRAY_URL;

int CObjBase::sm_iCount = 0;	// UID table.

// game servers stuff.
CWorld		g_World;	// the world. (we save this stuff)
CServer		g_Serv;	// current state stuff not saved.
CResource	g_Cfg;
CMainTask	g_MainTask;
CBackTask	g_BackTask;
CGrayInstall g_Install;
CVerDataMul	g_VerData;
CExpression g_Exp;	// Global script variables.
CLog		g_Log;
CEventLog * g_pLog = &g_Log;
CAccounts	g_Accounts;	// All the player accounts. name sorted CAccount

//////////////////////////////////////////////////////////////////
// util type stuff.

//
// Provide these functions globally.
//

class CGrayAssert : public CGrayError
{
protected:
	LPCTSTR const m_pExp;
	LPCTSTR const m_pFile;
	const unsigned m_uLine;
public:
	CGrayAssert( LOGL_TYPE eSeverity, LPCTSTR pExp, 
		LPCTSTR pFile, unsigned uLine ) :
		CGrayError( eSeverity, 0, _TEXT("Assert")),
		m_pExp(pExp), m_pFile(pFile), m_uLine(uLine)
	{
	}
	virtual ~CGrayAssert()
	{
	}
	virtual BOOL GetErrorMessage( LPSTR lpszError, UINT nMaxError,	UINT * pnHelpContext )
	{
		sprintf( lpszError, "Assert pri=%d:'%s' file '%s', line %d", m_eSeverity, m_pExp, m_pFile, m_uLine );
		return( true );
	}
};



void Debug_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
{
	g_Log.Event( LOGL_ERROR, "Check Fail:'%s' file '%s', line %d" DEBUG_CR, pExp, pFile, uLine );
}

#ifndef NOTASSERTDEBUG

void Debug_Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
{
	throw CGrayAssert( LOGL_CRIT, pExp, pFile, uLine );
}

void Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
{
	// These get left in release code .
	throw CGrayAssert( LOGL_CRIT, pExp, pFile, uLine );
}

#else

void Debug_Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
{
	g_Log.Event( LOGL_ERROR, "Check Fail:'%s' file '%s', line %d" DEBUG_CR, pExp, pFile, uLine );
}

void Assert_CheckFail( LPCTSTR pExp, LPCTSTR pFile, unsigned uLine )
{
	g_Log.Event( LOGL_ERROR, "Check Fail:'%s' file '%s', line %d" DEBUG_CR, pExp, pFile, uLine );
}

#endif

#if defined(_WIN32) && defined(_CONSOLE)

BOOL WINAPI ConsoleHandlerRoutine( DWORD dwCtrlType )
{
	//  control signal type. CTRL_C_EVENT
	// SetConsoleCtrlHandler
	switch ( dwCtrlType )
	{
	case CTRL_C_EVENT: // A CTRL+c signal was received, either from keyboard input or from a signal generated by the GenerateConsoleCtrlEvent function.
	case CTRL_BREAK_EVENT: //  A CTRL+BREAK signal was received, either from keyboard input or from a signal generated by GenerateConsoleCtrlEvent.
	case CTRL_CLOSE_EVENT: // A signal that the system sends to all processes attached to a console when the user closes the console (either by choosing the Close command from the console window's System menu, or by choosing the End Task command from the Task List).
		if ( g_Cfg.m_fSecure && ! g_Serv.IsLoading())	// enable the try code.
		{
#if 0
			// this never worked.
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
#endif
			g_Serv.SetExitFlag( 3 );
			return( TRUE );
		}
		break;
	case CTRL_LOGOFF_EVENT: // A signal that the system sends to all console processes when a user is logging off. This signal does not indicate which user is logging off, so no assumptions can be made.
	case CTRL_SHUTDOWN_EVENT: // A signal that the system sends to all console processes when the system is shutting down
		break;
	}
	return FALSE;	// process normally.
}

#endif	// _CONSOLE

#ifdef _WIN32

class CGrayException : public CGrayError
{
	// Catch and get details on the system exceptions.
	// NULL pointer access etc.
public:
	const DWORD m_dwAddress;
public:
	CGrayException( unsigned int uCode, DWORD dwAddress ) :
		m_dwAddress( dwAddress ),
		CGrayError( LOGL_CRIT, uCode, _TEXT("Exception"))
	{
	}
	virtual ~CGrayException()
	{
	}
	virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError,	UINT * pnHelpContext )
	{
		sprintf( lpszError, "Exception code=0%0x, addr=0%0x", 
			m_hError, m_dwAddress );
		return( true );
	}
};

// Don't do this in Debug Builds
// Exceptions are caught by our IDE's
#if !defined( _DEBUG )

extern "C" {

void _cdecl Sphere_Exception_Win32( unsigned int id, struct _EXCEPTION_POINTERS* pData )
{
	// WIN32 gets an exception.
/*
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)    
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)    
#define STATUS_USER_APC                  ((DWORD   )0x000000C0L)    
#define STATUS_TIMEOUT                   ((DWORD   )0x00000102L)    
#define STATUS_PENDING                   ((DWORD   )0x00000103L)    
#define STATUS_SEGMENT_NOTIFICATION      ((DWORD   )0x40000005L)    
#define STATUS_GUARD_PAGE_VIOLATION      ((DWORD   )0x80000001L)    
#define STATUS_DATATYPE_MISALIGNMENT     ((DWORD   )0x80000002L)    
#define STATUS_BREAKPOINT                ((DWORD   )0x80000003L)    
#define STATUS_SINGLE_STEP               ((DWORD   )0x80000004L)    
#define STATUS_ACCESS_VIOLATION          ((DWORD   )0xC0000005L)    
#define STATUS_IN_PAGE_ERROR             ((DWORD   )0xC0000006L)    
#define STATUS_INVALID_HANDLE            ((DWORD   )0xC0000008L)    
#define STATUS_NO_MEMORY                 ((DWORD   )0xC0000017L)    
#define STATUS_ILLEGAL_INSTRUCTION       ((DWORD   )0xC000001DL)    
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)    
#define STATUS_INVALID_DISPOSITION       ((DWORD   )0xC0000026L)    
#define STATUS_ARRAY_BOUNDS_EXCEEDED     ((DWORD   )0xC000008CL)    
#define STATUS_FLOAT_DENORMAL_OPERAND    ((DWORD   )0xC000008DL)    
#define STATUS_FLOAT_DIVIDE_BY_ZERO      ((DWORD   )0xC000008EL)    
#define STATUS_FLOAT_INEXACT_RESULT      ((DWORD   )0xC000008FL)    
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)    
#define STATUS_FLOAT_OVERFLOW            ((DWORD   )0xC0000091L)    
#define STATUS_FLOAT_STACK_CHECK         ((DWORD   )0xC0000092L)    
#define STATUS_FLOAT_UNDERFLOW           ((DWORD   )0xC0000093L)    
#define STATUS_INTEGER_DIVIDE_BY_ZERO    ((DWORD   )0xC0000094L)    
#define STATUS_INTEGER_OVERFLOW          ((DWORD   )0xC0000095L)    
#define STATUS_PRIVILEGED_INSTRUCTION    ((DWORD   )0xC0000096L)    
#define STATUS_STACK_OVERFLOW            ((DWORD   )0xC00000FDL)    
#define STATUS_CONTROL_C_EXIT            ((DWORD   )0xC000013AL)    
#define STATUS_FLOAT_MULTIPLE_FAULTS     ((DWORD   )0xC00002B4L)    
#define STATUS_FLOAT_MULTIPLE_TRAPS      ((DWORD   )0xC00002B5L)    
#define STATUS_ILLEGAL_VLM_REFERENCE     ((DWORD   )0xC00002C0L)     
*/

	DWORD dwCodeStart = (DWORD)(BYTE *) &globalstartsymbol;	// sync up to my MAP file.
	//	_asm mov dwCodeStart, CODE

	DWORD dwAddr = (DWORD)( pData->ExceptionRecord->ExceptionAddress );
	dwAddr -= dwCodeStart;

	throw( CGrayException( id, dwAddr ));
}

int _cdecl _purecall()
{
	// catch this special type of C++ exception as well.
	Assert_CheckFail( "purecall", "unknown", 1 );
	return 0;
}
}	// extern "C"

#endif // _DEBUG

#endif	// _WIN32

#ifndef BUILD_WITHOUT_CUSTOMNEW
void* _cdecl operator new( size_t stAllocateBlock )
{
	try
	{
		// We allocate an additional 4 bytes for storing the size of the memory block
		// This is a very very bad habit, but we use it for profiling our memory usage
		stAllocateBlock += sizeof( size_t );

		// Allocate our Block
		void *ptr = malloc( stAllocateBlock );
		ASSERT( ptr );

		// Store the size of our block in the first DWORD
		*( (size_t*)ptr ) = stAllocateBlock;

		// Change the statics for our server
		g_Serv.StatChange( SERV_STAT_MEM, stAllocateBlock );
		g_Serv.StatInc( SERV_STAT_ALLOCS );

		// Skip the additional sizeof( size_t ) we used for creating our 
		// statistics when returning the memory block.
		return (BYTE*)ptr + sizeof( size_t );
	}
	catch (CGrayError e)
	{
		g_Log.CatchEvent(&e, "Allocation of %l+%l bytes (no memory left?)", stAllocateBlock, sizeof(size_t));
		throw e;
	}
	catch (...)
	{
		g_Log.CatchEvent(NULL, "Allocation of %l+%l bytes (no memory left?)", stAllocateBlock, sizeof(size_t));
		throw;
	}
	return NULL;
}

void* _cdecl operator new[]( size_t stAllocateBlock )
{
	try
	{
		// We allocate an additional 4 bytes for storing the size of the memory block
		// This is a very very bad habit, but we use it for profiling our memory usage
		stAllocateBlock += sizeof( size_t );

		// Allocate our Block
		void *ptr = malloc( stAllocateBlock );
		ASSERT( ptr );

		// Store the size of our block in the first DWORD
		*( (size_t*)ptr ) = stAllocateBlock;

		// Change the statics for our server
		g_Serv.StatChange( SERV_STAT_MEM, stAllocateBlock );
		g_Serv.StatInc( SERV_STAT_ALLOCS );

		// Skip the additional sizeof( size_t ) we used for creating our 
		// statistics when returning the memory block.
		return (BYTE*)ptr + sizeof( size_t );
	}
	catch (CGrayError e)
	{
		g_Log.CatchEvent(&e, "Allocation of %l+%l bytes (no memory left?)", stAllocateBlock, sizeof(size_t));
		throw e;
	}
	catch (...)
	{
		g_Log.CatchEvent(NULL, "Allocation of %l+%l bytes (no memory left?)", stAllocateBlock, sizeof(size_t));
		throw;
	}
	return NULL;
}

void _cdecl operator delete( void *ptr )
{
	size_t stAllocBlock(0);
	try
	{
		// delete has to be null pointer safe
		if( ptr )
		{
			// We assume that the block we are deallocating has been allocated
			// with our overrideen new operator. That allows us to to track our memory usage.
			ptr = (BYTE*)ptr - sizeof( size_t );
			size_t stAllocateBlock = *(size_t*)ptr;
			stAllocBlock = stAllocateBlock;

			g_Serv.StatChange( SERV_STAT_MEM, -(long)stAllocateBlock );
			g_Serv.StatDec( SERV_STAT_ALLOCS );
	
			free( ptr );
		}
	}
	catch (CGrayError e)
	{
		g_Log.CatchEvent(&e, "Freeing block of %l bytes", stAllocBlock);
		throw e;
	}
	catch (...)
	{
		g_Log.CatchEvent(NULL, "Freeing block of %l bytes", stAllocBlock);
		throw;
	}
}

void _cdecl operator delete[]( void *ptr )
{
	size_t stAllocBlock(0);
	try
	{
		// delete has to be null pointer safe
		if( ptr )
		{
			// We assume that the block we are deallocating has been allocated
			// with our overrideen new operator. That allows us to to track our memory usage.
			ptr = (BYTE*)ptr - sizeof( size_t );
			size_t stAllocateBlock = *(size_t*)ptr;
			stAllocBlock = stAllocateBlock;

			g_Serv.StatChange( SERV_STAT_MEM, -(long)stAllocateBlock );
			g_Serv.StatDec( SERV_STAT_ALLOCS );
	
			free( ptr );
		}
	}
	catch (CGrayError e)
	{
		g_Log.CatchEvent(&e, "Freeing block of %l bytes", stAllocBlock);
		throw e;
	}
	catch (...)
	{
		g_Log.CatchEvent(NULL, "Freeing block of %l bytes", stAllocBlock);
		throw;
	}
}
#endif

DIR_TYPE GetDirStr( LPCTSTR pszDir )
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

LPCTSTR GetTimeMinDesc( int minutes )
{
	if ( minutes < 0 )
	{
		TCHAR * pTime = Str_GetTemp();
		strcpy( pTime, "Hmmm...I can't tell what time it is!");
		return( pTime );
	}
	int minute = minutes % 60;
	int hour = ( minutes / 60 ) % 24;

	LPCTSTR pMinDif;
	if (minute<=14) pMinDif = "";
	else if ((minute>=15)&&(minute<=30)) 
		pMinDif = " a quarter past";
	else if ((minute>=30)&&(minute<=45)) 
		pMinDif = " half past";
	else
	{
		pMinDif = " a quarter till";
		hour = ( hour + 1 ) % 24;
	}

	static LPCTSTR const sm_ClockHour[] =
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

	LPCTSTR pTail;
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

	TCHAR * pTime = Str_GetTemp();
	sprintf( pTime, "%s %s%s.", pMinDif, sm_ClockHour[hour], pTail );
	return( pTime );
}

int FindStrWord( LPCTSTR pTextSearch, LPCTSTR pszKeyWord )
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
			if ( isalpha( pTextSearch[i-1] ))	// not start of word ?
				continue;
		}
		if ( toupper( pTextSearch[i] ) == toupper( pszKeyWord[j] ))
			j++;
		else
			j=0;
	}
}

//*******************************************************************
// -CMainTask

THREAD_ENTRY_RET _cdecl CMainTask::EntryProc( void * lpThreadParameter ) // static
{
	// The main message loop.
	g_MainTask.OnCreate();
#if defined(_WIN32) && !defined(_DEBUG)
	_set_se_translator( Sphere_Exception_Win32 );	// must be called for each thread.
#endif
	while ( ! Sphere_OnTick() )
	{
	}
	g_MainTask.OnClose();
}

void CMainTask::CreateThread()
{
	// AttachInputThread to us if needed ?
	CThread::CreateThread( EntryProc );	
}

void CMainTask::CheckStuckThread()
{
	// Periodically called to check if the tread is stuck.

	static CGTime sm_timeRealPrev = 0;

	// Has real time changed ?
	CGTime timeRealCur = CGTime::GetCurrentTime();
	int iTimeDiff = timeRealCur.GetTime() - sm_timeRealPrev.GetTime();
	iTimeDiff = abs( iTimeDiff );
	
	if ( iTimeDiff < g_Cfg.m_iFreezeRestartTime )
		return;
	sm_timeRealPrev = timeRealCur;

	// Has server time changed ?
	CServTime timeCur = CServTime::GetCurrentTime();
	if ( timeCur != m_timePrev )	// Seems ok.
	{
		m_timePrev = timeCur;
		return;
	}

	if ( g_Serv.IsValidBusy())	// Server is just busy.
		return;

	if ( m_timeRestart == timeCur )
	{
		g_Log.Event( LOGL_FATAL, "Main loop freeze RESTART FAILED!" DEBUG_CR );
		//g_Serv.SetExitFlag( -1 );
		//return;
		//_asm int 3;
	}

	if ( m_timeWarn == timeCur )
	{
		// Kill and revive the main process
		g_Log.Event( LOGL_CRIT, "Main loop freeze RESTART!" DEBUG_CR );

#ifndef _DEBUG
		TerminateThread( 0xDEAD );

				// try to restart it.
		g_Log.Event( LOGL_EVENT, "Trying to restart the main loop thread" DEBUG_CR );
		CreateThread();
#endif
		m_timeRestart = timeCur;
	}
	else
	{
		g_Log.Event( LOGL_WARN, "Main loop frozen ?" DEBUG_CR );
		m_timeWarn = timeCur;
	}
}

//*******************************************************************

int Sphere_InitServer( int argc, char *argv[] )
{
	ASSERT( MAX_BUFFER >= sizeof( CCommand ));
	ASSERT( MAX_BUFFER >= sizeof( CEvent ));
	ASSERT( sizeof( int ) == sizeof( DWORD ));	// make this assumption often.
	ASSERT( sizeof( ITEMID_TYPE ) == sizeof( DWORD ));
	ASSERT( sizeof( WORD ) == 2 );
	ASSERT( sizeof( DWORD ) == 4 );
	ASSERT( sizeof( NWORD ) == 2 );
	ASSERT( sizeof( NDWORD ) == 4 );
	ASSERT(( UO_SIZE_X % SECTOR_SIZE_X ) == 0 );
	ASSERT(( UO_SIZE_Y % SECTOR_SIZE_Y ) == 0 );
	ASSERT( sizeof(CUOItemTypeRec) == 37 );	// byte pack working ?

#ifdef _WIN32
#if defined(_CONSOLE)
	SetConsoleTitle( EVO_TITLE " V" GRAY_VERSION );
	SetConsoleCtrlHandler( ConsoleHandlerRoutine, true );
#endif

#if !defined(_DEBUG)
	_set_se_translator( Sphere_Exception_Win32 );
#endif

#endif

	if ( ! g_Serv.Load())
	{
		return( -3 );
	}

	if ( argc > 1 )
	{
		// Do special debug type stuff.
		if ( ! g_Serv.CommandLine( argc, argv ))
		{
			return( -1 );
		}
	}

	if ( ! g_Serv.SocketsInit() ) return (-9);
	if ( ! g_World.LoadAll() ) return (-8);
#ifndef BUILD_WITHOUT_IRCSERVER
	if ( g_Cfg.m_fUseIRC ) g_IRCLocalServer.Init();
#endif

	g_Serv.SetServerMode( SERVMODE_Run );	// ready to go. ! IsLoading()

	if ( g_Cfg.CanRunBackTask() ) g_BackTask.CreateThread();
	else g_Log.Event( LOGM_INIT, "Background task not required." DEBUG_CR );

	g_Log.Event( LOGM_INIT, g_Serv.GetStatusString( 0x24 ));
	g_Log.Event( LOGM_INIT, _TEXT("Startup complete. items=%d, chars=%d" DEBUG_CR), g_Serv.StatGet(SERV_STAT_ITEMS), g_Serv.StatGet(SERV_STAT_CHARS));

	//	trigger server start
	g_Serv.r_Call("f_onserver_start", NULL, NULL);

#ifdef _WIN32
	g_Log.Event( LOGM_INIT, "Press '?' for console commands" DEBUG_CR );
#endif
	g_Log.FireEvent( LOGEVENT_Startup );

	return( 0 );
}

void Sphere_ExitServer()
{
	ASSERT(g_Serv.m_iExitFlag);
	g_Serv.SetServerMode( SERVMODE_Exiting );

	g_BackTask.WaitForClose( 15 );
	g_MainTask.WaitForClose( 15 );

	g_Serv.SocketsClose();
	g_World.Close();

#if defined(_WIN32) && defined(_CONSOLE)
	SetConsoleCtrlHandler( ConsoleHandlerRoutine, false );
#endif

	if ( g_Serv.m_iExitFlag < 0 )
	{
		g_Log.Event( LOGM_INIT|LOGL_FATAL, "Server terminated by error %d!" DEBUG_CR, g_Serv.m_iExitFlag );
#if defined(_WIN32) && defined(_CONSOLE)
		g_Serv.SysMessage( "Press any key to exit" );
		while ( _getch() == 0 ) ;
#endif
	}
	else
	{
		g_Log.Event( LOGM_INIT|LOGL_EVENT, "Server shutdown (code %d) complete!" DEBUG_CR, g_Serv.m_iExitFlag );
	}

	g_Log.FireEvent( LOGEVENT_Shutdown );
	g_Log.Close();
}

int Sphere_OnTick()
{
	// Give the world (CMainTask) a single tick.
	// RETURN: 0 = everything is fine.
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
	try
	{
#endif
#ifdef _DEBUG
		DEBUG_CHECK( g_Log.SetScriptContext(NULL) == NULL );
		DEBUG_CHECK( g_Log.SetObjectContext(NULL) == NULL );
#endif
#ifdef _WIN32
		g_Serv.OnTick_Busy();	// not used in Linux
#endif
		g_World.OnTick();
		g_Serv.OnTick();
#ifndef BUILD_WITHOUT_IRCSERVER
		if ( g_Cfg.m_fUseIRC )
		{
			g_IRCLocalServer.OnTick();
		}
#endif
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
	}
	catch ( CGrayError &e )
	{
		g_Log.CatchEvent( &e, "Main Loop" );
	}
	catch (...)	// catch all
	{
		g_Log.CatchEvent( NULL, "Main Loop" );
	}
#endif
	return( g_Serv.m_iExitFlag );
}

//*****************************************************

static void Sphere_MainMonitorLoop()
{
// We don't check for stuck threads in Debug mode
// Whenever we reach a breakpoint, this would trigger.


	// Just make sure the main loop is alive every so often.
	// This should be the parent thread.
	// try to restart it if it is not.

	ASSERT( CThread::GetCurrentThreadId() == g_Serv.m_dwParentThread );
	while ( ! g_Serv.m_iExitFlag )
	{
		char	inLocalBlock[64];
		inLocalBlock[0] = 0;
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
		try
		{
#endif
			if ( g_Cfg.m_iFreezeRestartTime <= 0 )
			{
				DEBUG_ERR(( "Freeze Restart Time cannot be cleared at run time" DEBUG_CR ));
				g_Cfg.m_iFreezeRestartTime = 10;
			}

#ifdef _WIN32
			strcpy(inLocalBlock, "NTWindow OnTick");
			NTWindow_OnTick( g_Cfg.m_iFreezeRestartTime * 1000 );
#else
			Sleep( g_Cfg.m_iFreezeRestartTime * 1000 );
#endif

			// Don't look for freezing when doing certain things.
			if ( g_Serv.IsLoading() || ! g_Cfg.m_fSecure )
				continue;

#if !defined( _DEBUG )
			strcpy(inLocalBlock, "MainLoop Check");
			g_MainTask.CheckStuckThread();
			strcpy(inLocalBlock, "BackgroundLoop Check");
			g_BackTask.CheckStuckThread();
#endif
#if !defined( _DEBUG ) && !defined( NO_INTERNAL_EXCEPTIONS )
		}
		catch ( CGrayError &e )	// catch all
		{
			g_Log.CatchEvent(&e, "Anti-freeze check (%s)", inLocalBlock);
		}
		catch (...)	// catch all
		{
			g_Log.CatchEvent(NULL, "Anti-freeze check (%s)", inLocalBlock);
		}
#endif
	}

}

//******************************************************

#if !defined( _WIN32 )

// This is used to restore the original flags on exit
void resetNonBlockingIo()
{
        termios term_caps;

        if( tcgetattr( STDIN_FILENO, &term_caps ) < 0 )
                return;

        term_caps.c_lflag |= ICANON;

        if( tcsetattr( STDIN_FILENO, TCSANOW, &term_caps ) < 0 )
                return;
}

void setNonBlockingIo()
{
        termios term_caps;

        if( tcgetattr( STDIN_FILENO, &term_caps ) < 0 )
	{
		printf( "ERROR: Could not get the termcap settings for this terminal.\n" );
                return;
	}

        term_caps.c_lflag &= ~ICANON;

        if( tcsetattr( STDIN_FILENO, TCSANOW, &term_caps ) < 0 )
	{
		printf( "ERROR: Could not set the termcap settings for this terminal.\n" );
                return;
	}

        setbuf( stdin, NULL );

        atexit( resetNonBlockingIo );
}
 
#endif

void dword_q_sort(DWORD numbers[], DWORD left, DWORD right)
{
	DWORD	pivot, l_hold, r_hold;

	l_hold = left;
	r_hold = right;
	pivot = numbers[left];
	while (left < right)
	{
		while ((numbers[right] >= pivot) && (left < right)) right--;
		if (left != right)
		{
			numbers[left] = numbers[right];
			left++;
		}
		while ((numbers[left] <= pivot) && (left < right)) left++;
		if (left != right)
		{
			numbers[right] = numbers[left];
			right--;
		}
	}
	numbers[left] = pivot;
	pivot = left;
	left = l_hold;
	right = r_hold;
	if (left < pivot) dword_q_sort(numbers, left, pivot-1);
	if (right > pivot) dword_q_sort(numbers, pivot+1, right);
}

void defragSphere(char *path)
{
	CFileText	inf;
	CGFile		ouf;
	char	z[256], z1[256], buf[1024];
	int		i;
	DWORD	uid(0);
	char	*p, *p1;
	DWORD	dBytesRead;
	DWORD	dTotalMb;
	DWORD	mb10(10*1024*1024);
	DWORD	mb5(5*1024*1024);
	bool	bSpecial;
	DWORD	dTotalUIDs;

	char	c,c1,c2;
	DWORD	d;

	//	NOTE: Sure I could use CVarDefArray, but it is extremely slow with memory allocation, takes hours
	//		to read and save the data. Moreover, it takes less memory in this case and does less convertations.
#define	MAX_UID	5000000L	// limit to 5mln of objects, takes 5mln*4 = 20mb
	DWORD	*uids;

	g_Log.Event(LOGM_INIT,	"Defragmentation (UID alteration) of " EVO_TITLE " saves." DEBUG_CR
		"Use it on your risk and if you know what you are doing since it can possibly harm your server." DEBUG_CR
		"The process can take up to several hours depending on the CPU you have." DEBUG_CR
		"After finished, you will have your '" GRAY_FILE "*.scp' files converted and saved as '" GRAY_FILE "*.scp.new'." DEBUG_CR);

	uids = (DWORD*)calloc(MAX_UID, sizeof(DWORD));
	for ( i = 0; i < 3; i++ )
	{
		strcpy(z, path);
		if ( i == 0 ) strcat(z, GRAY_FILE "statics.scp");
		else if ( i == 1 ) strcat(z, GRAY_FILE "world.scp");
		else strcat(z, GRAY_FILE "chars.scp");

		g_Log.Event(LOGM_INIT, "Reading current UIDs: %s" DEBUG_CR, z);
		if ( !inf.Open(z, OF_READ|OF_TEXT) )
		{
			g_Log.Event(LOGM_INIT, "Cannot open file for reading. Skipped!" DEBUG_CR);
			continue;
		}
		dBytesRead = dTotalMb = 0;
		while ( !feof(inf.m_pStream) )
		{
			fgets(buf, sizeof(buf), inf.m_pStream);
			dBytesRead += strlen(buf);
			if ( dBytesRead > mb10 )
			{
				dBytesRead -= mb10;
				dTotalMb += 10;
				g_Log.Event(LOGM_INIT, "Total read %u Mb" DEBUG_CR, dTotalMb);
			}
			if (( buf[0] == 'S' ) && ( strstr(buf, "SERIAL=") == buf ))
			{
				p = buf;
				p += 7;
				p1 = p;
				while ( *p1 && ( *p1 != '\r' ) && ( *p1 != '\n' )) p1++;
				*p1 = 0;

				//	prepare new uid
				*(p-1) = '0';
				*p = 'x';
				p--;
				uids[uid++] = strtoul(p, &p1, 16);
			}
		}
		inf.Close();
	}
	dTotalUIDs = uid;
	g_Log.Event(LOGM_INIT, "Totally having %u unique objects (UIDs), latest: 0%x" DEBUG_CR, uid, uids[uid-1]);

	g_Log.Event(LOGM_INIT, "Quick-Sorting the UIDs array..." DEBUG_CR);
	dword_q_sort(uids, 0, dTotalUIDs-1);

	for ( i = 0; i < 5; i++ )
	{
		strcpy(z, path);
		if ( !i ) strcat(z, GRAY_FILE "accu.scp");
		else if ( i == 1 ) strcat(z, GRAY_FILE "chars.scp");
		else if ( i == 2 ) strcat(z, GRAY_FILE "data.scp");
		else if ( i == 3 ) strcat(z, GRAY_FILE "world.scp");
		else if ( i == 4 ) strcat(z, GRAY_FILE "statics.scp");
		g_Log.Event(LOGM_INIT, "Updating UID-s in %s to %s.new" DEBUG_CR, z, z);
		if ( !inf.Open(z) )
		{
			g_Log.Event(LOGM_INIT, "Cannot open file for reading. Skipped!" DEBUG_CR);
			continue;
		}
		strcat(z, ".new");
		if ( !ouf.Open(z, OF_WRITE|OF_CREATE) )
		{
			g_Log.Event(LOGM_INIT, "Cannot open file for writing. Skipped!" DEBUG_CR);
			continue;
		}
		dBytesRead = dTotalMb = 0;
		while ( inf.ReadString(buf, sizeof(buf)) )
		{
			uid = strlen(buf);
			buf[uid] = buf[uid+1] = buf[uid+2] = 0;	// just to be sure to be in line always
							// NOTE: it is much faster than to use memcpy to clear before reading
			bSpecial = false;
			dBytesRead += uid;
			if ( dBytesRead > mb5 )
			{
				dBytesRead -= mb5;
				dTotalMb += 5;
				g_Log.Event(LOGM_INIT, "Total processed %u Mb" DEBUG_CR, dTotalMb);
			}
			p = buf;
			if ( 0 ) ;
			//	Note 28-Jun-2004
			//	mounts seems having ACTARG1 > 0x30000000. The actual UID is ACTARG1-0x30000000. The
			//	new also should be new+0x30000000. need investigation if this can help making mounts
			//	not to disappear after the defrag
			else if (( buf[0] == 'A' ) && ( strstr(buf, "ACTARG1=0") == buf ))		// ACTARG1=
				p += 8;
			else if (( buf[0] == 'C' ) && ( strstr(buf, "CONT=0") == buf ))			// CONT=
				p += 5;
			else if (( buf[0] == 'C' ) && ( strstr(buf, "CHARUID=0") == buf ))		// CHARUID=
				p += 8;
			else if (( buf[0] == 'L' ) && ( strstr(buf, "LASTCHARUID=0") == buf ))	// LASTCHARUID=
				p += 12;
			else if (( buf[0] == 'L' ) && ( strstr(buf, "LINK=0") == buf ))			// LINK=
				p += 5;
			else if (( buf[0] == 'M' ) && ( strstr(buf, "MEMBER=0") == buf ))		// MEMBER=
			{
				p += 7;
				bSpecial = true;
			}
			else if (( buf[0] == 'M' ) && ( strstr(buf, "MORE1=0") == buf ))		// MORE1=
				p += 6;
			else if (( buf[0] == 'M' ) && ( strstr(buf, "MORE2=0") == buf ))		// MORE2=
				p += 6;
			else if (( buf[0] == 'S' ) && ( strstr(buf, "SERIAL=0") == buf ))		// SERIAL=
				p += 7;
			else if ((( buf[0] == 'T' ) && ( strstr(buf, "TAG.") == buf )) ||		// TAG.=
					 (( buf[0] == 'R' ) && ( strstr(buf, "REGION.TAG") == buf )))
			{
				while ( *p && ( *p != '=' )) p++;
				p++;
			}
			else if (( i == 2 ) && strchr(buf, '='))	// spheredata.scp - plain VARs
			{
				while ( *p && ( *p != '=' )) p++;
				p++;
			}
			else p = NULL;

			//	UIDs are always hex, so prefixed by 0
			if ( p && ( *p != '0' )) p = NULL;

			//	here we got potentialy UID-contained variable
			//	check if it really is only UID-like var containing
			if ( p )
			{
				p1 = p;
				while ( *p1 &&
					((( *p1 >= '0' ) && ( *p1 <= '9' )) ||
					 (( *p1 >= 'a' ) && ( *p1 <= 'f' )))) p1++;
				if ( !bSpecial )
				{
					if ( *p1 && ( *p1 != '\r' ) && ( *p1 != '\n' )) // some more text in line
						p = NULL;
				}
			}

			//	here we definitely know that this is very uid-like
			if ( p )
			{
				c = *p1;

				*p1 = 0;
				//	here in p we have the current value of the line.
				//	check if it is a valid UID

				//	prepare converting 0.. to 0x..
				c1 = *(p-1);
				c2 = *p;
				*(p-1) = '0';
				*p = 'x';
				p--;
				uid = strtoul(p, &p1, 16);
				p++;
				*(p-1) = c1;
				*p = c2;
				//	Note 28-Jun-2004
				//	The search algourytm is very simple and fast. But maybe integrate some other, at least /2 algorythm
				//	since has amount/2 tryes at worst chance to get the item and never scans the whole array
				//	It should improve speed since defragmenting 150Mb saves takes ~2:30 on 2.0Mhz CPU
				{
					DWORD	dStep = dTotalUIDs/2;
					d = dStep;
					while ( true )
					{
						dStep /= 2;

						if ( uids[d] == uid )
						{
							uid = d | (uids[d]&0xF0000000);	// do not forget attach item and special flags like 04..
							break;
						}
						else if ( uids[d] < uid ) d += dStep;
						else d -= dStep;

						if ( dStep == 1 )
						{
							uid = 0xFFFFFFFFL;
							break; // did not find the UID
						}
					}
				}

				//	Search for this uid in the table
/*				for ( d = 0; d < dTotalUIDs; d++ )
				{
					if ( !uids[d] )	// end of array
					{
						uid = 0xFFFFFFFFL;
						break;
					}
					else if ( uids[d] == uid )
					{
						uid = d | (uids[d]&0xF0000000);	// do not forget attach item and special flags like 04..
						break;
					}
				}*/

				//	replace UID by the new one since it has been found
				*p1 = c;
				if ( uid != 0xFFFFFFFFL )
				{
					*p = 0;
					strcpy(z, p1);
					sprintf(z1, "0%x", uid);
					strcat(buf, z1);
					strcat(buf, z);
				}
			}
			//	output the resulting line
			ouf.Write(buf, strlen(buf));
		}
		inf.Close();
		ouf.Close();
	}
	free(uids);
	g_Log.Event(LOGM_INIT,	"Defragmentation complete." DEBUG_CR);
}

//	Exceptions debugging routine.
#ifdef EXCEPTIONS_DEBUG
LPCTSTR const m_ExcKeys[EXC_QTY+1] =
{
	"",
	"CALLing subfunction",
	"dorand/doswitch",
	"for"
	"forchar",
	"forcharlayer/memorytype",
	"forclients",
	"forcont",
	"forcontid/type",
	"foritem",
	"forobj",
	"forplayers",
	"key initializing",
	"key reading",
	"memcpy/memmove",
	"parsing",
	"parsing begin/loop cycle",
	"parsing IF",
	"too many entrances",
	"return",
	"verbing value",
	"while",
	"writing value",
	NULL,
};
char *g_ExcArguments;
void _cdecl exceptions_debug_init(LPCTSTR pszFormat, ...)
{
	va_list vargs;
	va_start( vargs, pszFormat );
	sprintf(g_ExcArguments, pszFormat, vargs);
	va_end( vargs );
}
static TCHAR	g_ExcStack[64*512];
static int		g_ExcCurrent = 0;

TCHAR *Exc_GetStack()
{
	if ( ++g_ExcCurrent >= 64 ) g_ExcCurrent = 0;
	return (&g_ExcStack[g_ExcCurrent*512]);
}
#endif

#ifdef _WIN32
int Sphere_MainEntryPoint( int argc, char *argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
	// Initialize nonblocking IO 
	// and disable readline on linux
	#if !defined( _WIN32 )
	setNonBlockingIo();
	#endif

	g_Serv.m_iExitFlag = Sphere_InitServer( argc, argv );

	if ( ! g_Serv.m_iExitFlag )
	{
		if (
#ifdef _WIN32
			GRAY_GetOSInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT &&
#endif
			g_Cfg.m_iFreezeRestartTime )
		{
			g_MainTask.CreateThread();
			Sphere_MainMonitorLoop();
		}
		else
		{
			g_MainTask.EntryProc( 0 );
		}
	}

	Sphere_ExitServer();
	return( g_Serv.m_iExitFlag );
}
