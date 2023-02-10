// CIRCServer.cpp
//

#include "graysvr.h"	// predef header.
#include "CIRCServer.h"

#ifndef _WIN32
#include <sys/timeb.h>

#ifndef _BSD
#define _ftime ftime
#define _timeb timeb
#else
#define _ftime time
#define _timeb time_t
#endif

// #include <errno.h>
// #define _cdecl
#endif

#ifndef BUILD_WITHOUT_IRCSERVER

CIRCLocalServer g_IRCLocalServer;

/*
 * CIRCTools is just a bunch of useful IRC functions which all other IRC classes
 * need access to.
 *
 */

LPCTSTR const CIRCTools::sm_CmdTbl[] =
{
	"PRIVMSG",	/* PRIV */
	"WHO",		/* WHO	-> WHOC */
	"WHOIS",	/* WHOI */
	"WHOWAS",	/* WHOW */
	"USER",		/* USER */
	"NICK",		/* NICK */
	"SERVER",	/* SERV */
	"LIST",		/* LIST */
	"TOPIC",	/* TOPI */
	"INVITE",	/* INVI */
	"VERSION",	/* VERS */
	"QUIT",		/* QUIT */
	"SQUIT",	/* SQUI */
	"KILL",		/* KILL */
	"INFO",		/* INFO */
	"LINKS",	/* LINK */
	"STATS",	/* STAT */
	"HELP",		/* HELP */
	"ERROR",	/* ERRO */
	"AWAY",		/* AWAY */
	"CONNECT",	/* CONN */
	"UPING",	/* UPIN */
	"MAP",		/* MAP	*/
	"PING",		/* PING */
	"PONG",		/* PONG */
	"OPER",		/* OPER */
	"PASS",		/* PASS */
	"WALLOPS",	/* WALL */
	"TIME",		/* TIME */
	"SETTIME",	/* SETT */
	"RPING",	/* RPIN */
	"RPONG",	/* RPON */
	"NAMES",	/* NAME */
	"ADMIN",	/* ADMI */
	"TRACE",	/* TRAC */
	"NOTICE",	/* NOTI */
	"WALLCHOPS",/* WC */
	"CPRIVMSG",	/* CPRI */
	"CNOTICE",	/* CNOT */
	"JOIN",		/* JOIN */
	"PART",		/* PART */
	"LUSERS",	/* LUSE */
	"MOTD",		/* MOTD */
	"MODE",		/* MODE */
	"KICK",		/* KICK */
	"USERHOST",	/* USER -> USRH */
	"USERIP",	/* USER -> USIP */
	"ISON",		/* ISON */
	"SQUERY",	/* SQUE */
	"SERVLIST",	/* SERV -> SLIS */
	"SERVSET",	/* SERV -> SSET */
	"REHASH",	/* REHA */
	"RESTART",	/* REST */
	"CLOSE",	/* CLOS */
	"DIE",		/* DIE	*/
	"HASH",		/* HASH */
	"DNS",		/* DNS	-> DNSS */
	"SILENCE",	/* SILE */
	"GLINE",	/* GLIN */
	"BURST",	/* BURS */
	"CREATE",	/* CREA */
	"END_OF_BURST",	/* END_ */
	"EOB_ACK",	/* EOB_ */
};

const char CIRCTools::sm_atoi_tab[4000] =
{
	'0','0','0',0, '0','0','1',0, '0','0','2',0, '0','0','3',0, '0','0','4',0,
	'0','0','5',0, '0','0','6',0, '0','0','7',0, '0','0','8',0, '0','0','9',0,
	'0','1','0',0, '0','1','1',0, '0','1','2',0, '0','1','3',0, '0','1','4',0,
	'0','1','5',0, '0','1','6',0, '0','1','7',0, '0','1','8',0, '0','1','9',0,
	'0','2','0',0, '0','2','1',0, '0','2','2',0, '0','2','3',0, '0','2','4',0,
	'0','2','5',0, '0','2','6',0, '0','2','7',0, '0','2','8',0, '0','2','9',0,
	'0','3','0',0, '0','3','1',0, '0','3','2',0, '0','3','3',0, '0','3','4',0,
	'0','3','5',0, '0','3','6',0, '0','3','7',0, '0','3','8',0, '0','3','9',0,
	'0','4','0',0, '0','4','1',0, '0','4','2',0, '0','4','3',0, '0','4','4',0,
	'0','4','5',0, '0','4','6',0, '0','4','7',0, '0','4','8',0, '0','4','9',0,
	'0','5','0',0, '0','5','1',0, '0','5','2',0, '0','5','3',0, '0','5','4',0,
	'0','5','5',0, '0','5','6',0, '0','5','7',0, '0','5','8',0, '0','5','9',0,
	'0','6','0',0, '0','6','1',0, '0','6','2',0, '0','6','3',0, '0','6','4',0,
	'0','6','5',0, '0','6','6',0, '0','6','7',0, '0','6','8',0, '0','6','9',0,
	'0','7','0',0, '0','7','1',0, '0','7','2',0, '0','7','3',0, '0','7','4',0,
	'0','7','5',0, '0','7','6',0, '0','7','7',0, '0','7','8',0, '0','7','9',0,
	'0','8','0',0, '0','8','1',0, '0','8','2',0, '0','8','3',0, '0','8','4',0,
	'0','8','5',0, '0','8','6',0, '0','8','7',0, '0','8','8',0, '0','8','9',0,
	'0','9','0',0, '0','9','1',0, '0','9','2',0, '0','9','3',0, '0','9','4',0,
	'0','9','5',0, '0','9','6',0, '0','9','7',0, '0','9','8',0, '0','9','9',0,
	'1','0','0',0, '1','0','1',0, '1','0','2',0, '1','0','3',0, '1','0','4',0,
	'1','0','5',0, '1','0','6',0, '1','0','7',0, '1','0','8',0, '1','0','9',0,
	'1','1','0',0, '1','1','1',0, '1','1','2',0, '1','1','3',0, '1','1','4',0,
	'1','1','5',0, '1','1','6',0, '1','1','7',0, '1','1','8',0, '1','1','9',0,
	'1','2','0',0, '1','2','1',0, '1','2','2',0, '1','2','3',0, '1','2','4',0,
	'1','2','5',0, '1','2','6',0, '1','2','7',0, '1','2','8',0, '1','2','9',0,
	'1','3','0',0, '1','3','1',0, '1','3','2',0, '1','3','3',0, '1','3','4',0,
	'1','3','5',0, '1','3','6',0, '1','3','7',0, '1','3','8',0, '1','3','9',0,
	'1','4','0',0, '1','4','1',0, '1','4','2',0, '1','4','3',0, '1','4','4',0,
	'1','4','5',0, '1','4','6',0, '1','4','7',0, '1','4','8',0, '1','4','9',0,
	'1','5','0',0, '1','5','1',0, '1','5','2',0, '1','5','3',0, '1','5','4',0,
	'1','5','5',0, '1','5','6',0, '1','5','7',0, '1','5','8',0, '1','5','9',0,
	'1','6','0',0, '1','6','1',0, '1','6','2',0, '1','6','3',0, '1','6','4',0,
	'1','6','5',0, '1','6','6',0, '1','6','7',0, '1','6','8',0, '1','6','9',0,
	'1','7','0',0, '1','7','1',0, '1','7','2',0, '1','7','3',0, '1','7','4',0,
	'1','7','5',0, '1','7','6',0, '1','7','7',0, '1','7','8',0, '1','7','9',0,
	'1','8','0',0, '1','8','1',0, '1','8','2',0, '1','8','3',0, '1','8','4',0,
	'1','8','5',0, '1','8','6',0, '1','8','7',0, '1','8','8',0, '1','8','9',0,
	'1','9','0',0, '1','9','1',0, '1','9','2',0, '1','9','3',0, '1','9','4',0,
	'1','9','5',0, '1','9','6',0, '1','9','7',0, '1','9','8',0, '1','9','9',0,
	'2','0','0',0, '2','0','1',0, '2','0','2',0, '2','0','3',0, '2','0','4',0,
	'2','0','5',0, '2','0','6',0, '2','0','7',0, '2','0','8',0, '2','0','9',0,
	'2','1','0',0, '2','1','1',0, '2','1','2',0, '2','1','3',0, '2','1','4',0,
	'2','1','5',0, '2','1','6',0, '2','1','7',0, '2','1','8',0, '2','1','9',0,
	'2','2','0',0, '2','2','1',0, '2','2','2',0, '2','2','3',0, '2','2','4',0,
	'2','2','5',0, '2','2','6',0, '2','2','7',0, '2','2','8',0, '2','2','9',0,
	'2','3','0',0, '2','3','1',0, '2','3','2',0, '2','3','3',0, '2','3','4',0,
	'2','3','5',0, '2','3','6',0, '2','3','7',0, '2','3','8',0, '2','3','9',0,
	'2','4','0',0, '2','4','1',0, '2','4','2',0, '2','4','3',0, '2','4','4',0,
	'2','4','5',0, '2','4','6',0, '2','4','7',0, '2','4','8',0, '2','4','9',0,
	'2','5','0',0, '2','5','1',0, '2','5','2',0, '2','5','3',0, '2','5','4',0,
	'2','5','5',0, '2','5','6',0, '2','5','7',0, '2','5','8',0, '2','5','9',0,
	'2','6','0',0, '2','6','1',0, '2','6','2',0, '2','6','3',0, '2','6','4',0,
	'2','6','5',0, '2','6','6',0, '2','6','7',0, '2','6','8',0, '2','6','9',0,
	'2','7','0',0, '2','7','1',0, '2','7','2',0, '2','7','3',0, '2','7','4',0,
	'2','7','5',0, '2','7','6',0, '2','7','7',0, '2','7','8',0, '2','7','9',0,
	'2','8','0',0, '2','8','1',0, '2','8','2',0, '2','8','3',0, '2','8','4',0,
	'2','8','5',0, '2','8','6',0, '2','8','7',0, '2','8','8',0, '2','8','9',0,
	'2','9','0',0, '2','9','1',0, '2','9','2',0, '2','9','3',0, '2','9','4',0,
	'2','9','5',0, '2','9','6',0, '2','9','7',0, '2','9','8',0, '2','9','9',0,
	'3','0','0',0, '3','0','1',0, '3','0','2',0, '3','0','3',0, '3','0','4',0,
	'3','0','5',0, '3','0','6',0, '3','0','7',0, '3','0','8',0, '3','0','9',0,
	'3','1','0',0, '3','1','1',0, '3','1','2',0, '3','1','3',0, '3','1','4',0,
	'3','1','5',0, '3','1','6',0, '3','1','7',0, '3','1','8',0, '3','1','9',0,
	'3','2','0',0, '3','2','1',0, '3','2','2',0, '3','2','3',0, '3','2','4',0,
	'3','2','5',0, '3','2','6',0, '3','2','7',0, '3','2','8',0, '3','2','9',0,
	'3','3','0',0, '3','3','1',0, '3','3','2',0, '3','3','3',0, '3','3','4',0,
	'3','3','5',0, '3','3','6',0, '3','3','7',0, '3','3','8',0, '3','3','9',0,
	'3','4','0',0, '3','4','1',0, '3','4','2',0, '3','4','3',0, '3','4','4',0,
	'3','4','5',0, '3','4','6',0, '3','4','7',0, '3','4','8',0, '3','4','9',0,
	'3','5','0',0, '3','5','1',0, '3','5','2',0, '3','5','3',0, '3','5','4',0,
	'3','5','5',0, '3','5','6',0, '3','5','7',0, '3','5','8',0, '3','5','9',0,
	'3','6','0',0, '3','6','1',0, '3','6','2',0, '3','6','3',0, '3','6','4',0,
	'3','6','5',0, '3','6','6',0, '3','6','7',0, '3','6','8',0, '3','6','9',0,
	'3','7','0',0, '3','7','1',0, '3','7','2',0, '3','7','3',0, '3','7','4',0,
	'3','7','5',0, '3','7','6',0, '3','7','7',0, '3','7','8',0, '3','7','9',0,
	'3','8','0',0, '3','8','1',0, '3','8','2',0, '3','8','3',0, '3','8','4',0,
	'3','8','5',0, '3','8','6',0, '3','8','7',0, '3','8','8',0, '3','8','9',0,
	'3','9','0',0, '3','9','1',0, '3','9','2',0, '3','9','3',0, '3','9','4',0,
	'3','9','5',0, '3','9','6',0, '3','9','7',0, '3','9','8',0, '3','9','9',0,
	'4','0','0',0, '4','0','1',0, '4','0','2',0, '4','0','3',0, '4','0','4',0,
	'4','0','5',0, '4','0','6',0, '4','0','7',0, '4','0','8',0, '4','0','9',0,
	'4','1','0',0, '4','1','1',0, '4','1','2',0, '4','1','3',0, '4','1','4',0,
	'4','1','5',0, '4','1','6',0, '4','1','7',0, '4','1','8',0, '4','1','9',0,
	'4','2','0',0, '4','2','1',0, '4','2','2',0, '4','2','3',0, '4','2','4',0,
	'4','2','5',0, '4','2','6',0, '4','2','7',0, '4','2','8',0, '4','2','9',0,
	'4','3','0',0, '4','3','1',0, '4','3','2',0, '4','3','3',0, '4','3','4',0,
	'4','3','5',0, '4','3','6',0, '4','3','7',0, '4','3','8',0, '4','3','9',0,
	'4','4','0',0, '4','4','1',0, '4','4','2',0, '4','4','3',0, '4','4','4',0,
	'4','4','5',0, '4','4','6',0, '4','4','7',0, '4','4','8',0, '4','4','9',0,
	'4','5','0',0, '4','5','1',0, '4','5','2',0, '4','5','3',0, '4','5','4',0,
	'4','5','5',0, '4','5','6',0, '4','5','7',0, '4','5','8',0, '4','5','9',0,
	'4','6','0',0, '4','6','1',0, '4','6','2',0, '4','6','3',0, '4','6','4',0,
	'4','6','5',0, '4','6','6',0, '4','6','7',0, '4','6','8',0, '4','6','9',0,
	'4','7','0',0, '4','7','1',0, '4','7','2',0, '4','7','3',0, '4','7','4',0,
	'4','7','5',0, '4','7','6',0, '4','7','7',0, '4','7','8',0, '4','7','9',0,
	'4','8','0',0, '4','8','1',0, '4','8','2',0, '4','8','3',0, '4','8','4',0,
	'4','8','5',0, '4','8','6',0, '4','8','7',0, '4','8','8',0, '4','8','9',0,
	'4','9','0',0, '4','9','1',0, '4','9','2',0, '4','9','3',0, '4','9','4',0,
	'4','9','5',0, '4','9','6',0, '4','9','7',0, '4','9','8',0, '4','9','9',0,
	'5','0','0',0, '5','0','1',0, '5','0','2',0, '5','0','3',0, '5','0','4',0,
	'5','0','5',0, '5','0','6',0, '5','0','7',0, '5','0','8',0, '5','0','9',0,
	'5','1','0',0, '5','1','1',0, '5','1','2',0, '5','1','3',0, '5','1','4',0,
	'5','1','5',0, '5','1','6',0, '5','1','7',0, '5','1','8',0, '5','1','9',0,
	'5','2','0',0, '5','2','1',0, '5','2','2',0, '5','2','3',0, '5','2','4',0,
	'5','2','5',0, '5','2','6',0, '5','2','7',0, '5','2','8',0, '5','2','9',0,
	'5','3','0',0, '5','3','1',0, '5','3','2',0, '5','3','3',0, '5','3','4',0,
	'5','3','5',0, '5','3','6',0, '5','3','7',0, '5','3','8',0, '5','3','9',0,
	'5','4','0',0, '5','4','1',0, '5','4','2',0, '5','4','3',0, '5','4','4',0,
	'5','4','5',0, '5','4','6',0, '5','4','7',0, '5','4','8',0, '5','4','9',0,
	'5','5','0',0, '5','5','1',0, '5','5','2',0, '5','5','3',0, '5','5','4',0,
	'5','5','5',0, '5','5','6',0, '5','5','7',0, '5','5','8',0, '5','5','9',0,
	'5','6','0',0, '5','6','1',0, '5','6','2',0, '5','6','3',0, '5','6','4',0,
	'5','6','5',0, '5','6','6',0, '5','6','7',0, '5','6','8',0, '5','6','9',0,
	'5','7','0',0, '5','7','1',0, '5','7','2',0, '5','7','3',0, '5','7','4',0,
	'5','7','5',0, '5','7','6',0, '5','7','7',0, '5','7','8',0, '5','7','9',0,
	'5','8','0',0, '5','8','1',0, '5','8','2',0, '5','8','3',0, '5','8','4',0,
	'5','8','5',0, '5','8','6',0, '5','8','7',0, '5','8','8',0, '5','8','9',0,
	'5','9','0',0, '5','9','1',0, '5','9','2',0, '5','9','3',0, '5','9','4',0,
	'5','9','5',0, '5','9','6',0, '5','9','7',0, '5','9','8',0, '5','9','9',0,
	'6','0','0',0, '6','0','1',0, '6','0','2',0, '6','0','3',0, '6','0','4',0,
	'6','0','5',0, '6','0','6',0, '6','0','7',0, '6','0','8',0, '6','0','9',0,
	'6','1','0',0, '6','1','1',0, '6','1','2',0, '6','1','3',0, '6','1','4',0,
	'6','1','5',0, '6','1','6',0, '6','1','7',0, '6','1','8',0, '6','1','9',0,
	'6','2','0',0, '6','2','1',0, '6','2','2',0, '6','2','3',0, '6','2','4',0,
	'6','2','5',0, '6','2','6',0, '6','2','7',0, '6','2','8',0, '6','2','9',0,
	'6','3','0',0, '6','3','1',0, '6','3','2',0, '6','3','3',0, '6','3','4',0,
	'6','3','5',0, '6','3','6',0, '6','3','7',0, '6','3','8',0, '6','3','9',0,
	'6','4','0',0, '6','4','1',0, '6','4','2',0, '6','4','3',0, '6','4','4',0,
	'6','4','5',0, '6','4','6',0, '6','4','7',0, '6','4','8',0, '6','4','9',0,
	'6','5','0',0, '6','5','1',0, '6','5','2',0, '6','5','3',0, '6','5','4',0,
	'6','5','5',0, '6','5','6',0, '6','5','7',0, '6','5','8',0, '6','5','9',0,
	'6','6','0',0, '6','6','1',0, '6','6','2',0, '6','6','3',0, '6','6','4',0,
	'6','6','5',0, '6','6','6',0, '6','6','7',0, '6','6','8',0, '6','6','9',0,
	'6','7','0',0, '6','7','1',0, '6','7','2',0, '6','7','3',0, '6','7','4',0,
	'6','7','5',0, '6','7','6',0, '6','7','7',0, '6','7','8',0, '6','7','9',0,
	'6','8','0',0, '6','8','1',0, '6','8','2',0, '6','8','3',0, '6','8','4',0,
	'6','8','5',0, '6','8','6',0, '6','8','7',0, '6','8','8',0, '6','8','9',0,
	'6','9','0',0, '6','9','1',0, '6','9','2',0, '6','9','3',0, '6','9','4',0,
	'6','9','5',0, '6','9','6',0, '6','9','7',0, '6','9','8',0, '6','9','9',0,
	'7','0','0',0, '7','0','1',0, '7','0','2',0, '7','0','3',0, '7','0','4',0,
	'7','0','5',0, '7','0','6',0, '7','0','7',0, '7','0','8',0, '7','0','9',0,
	'7','1','0',0, '7','1','1',0, '7','1','2',0, '7','1','3',0, '7','1','4',0,
	'7','1','5',0, '7','1','6',0, '7','1','7',0, '7','1','8',0, '7','1','9',0,
	'7','2','0',0, '7','2','1',0, '7','2','2',0, '7','2','3',0, '7','2','4',0,
	'7','2','5',0, '7','2','6',0, '7','2','7',0, '7','2','8',0, '7','2','9',0,
	'7','3','0',0, '7','3','1',0, '7','3','2',0, '7','3','3',0, '7','3','4',0,
	'7','3','5',0, '7','3','6',0, '7','3','7',0, '7','3','8',0, '7','3','9',0,
	'7','4','0',0, '7','4','1',0, '7','4','2',0, '7','4','3',0, '7','4','4',0,
	'7','4','5',0, '7','4','6',0, '7','4','7',0, '7','4','8',0, '7','4','9',0,
	'7','5','0',0, '7','5','1',0, '7','5','2',0, '7','5','3',0, '7','5','4',0,
	'7','5','5',0, '7','5','6',0, '7','5','7',0, '7','5','8',0, '7','5','9',0,
	'7','6','0',0, '7','6','1',0, '7','6','2',0, '7','6','3',0, '7','6','4',0,
	'7','6','5',0, '7','6','6',0, '7','6','7',0, '7','6','8',0, '7','6','9',0,
	'7','7','0',0, '7','7','1',0, '7','7','2',0, '7','7','3',0, '7','7','4',0,
	'7','7','5',0, '7','7','6',0, '7','7','7',0, '7','7','8',0, '7','7','9',0,
	'7','8','0',0, '7','8','1',0, '7','8','2',0, '7','8','3',0, '7','8','4',0,
	'7','8','5',0, '7','8','6',0, '7','8','7',0, '7','8','8',0, '7','8','9',0,
	'7','9','0',0, '7','9','1',0, '7','9','2',0, '7','9','3',0, '7','9','4',0,
	'7','9','5',0, '7','9','6',0, '7','9','7',0, '7','9','8',0, '7','9','9',0,
	'8','0','0',0, '8','0','1',0, '8','0','2',0, '8','0','3',0, '8','0','4',0,
	'8','0','5',0, '8','0','6',0, '8','0','7',0, '8','0','8',0, '8','0','9',0,
	'8','1','0',0, '8','1','1',0, '8','1','2',0, '8','1','3',0, '8','1','4',0,
	'8','1','5',0, '8','1','6',0, '8','1','7',0, '8','1','8',0, '8','1','9',0,
	'8','2','0',0, '8','2','1',0, '8','2','2',0, '8','2','3',0, '8','2','4',0,
	'8','2','5',0, '8','2','6',0, '8','2','7',0, '8','2','8',0, '8','2','9',0,
	'8','3','0',0, '8','3','1',0, '8','3','2',0, '8','3','3',0, '8','3','4',0,
	'8','3','5',0, '8','3','6',0, '8','3','7',0, '8','3','8',0, '8','3','9',0,
	'8','4','0',0, '8','4','1',0, '8','4','2',0, '8','4','3',0, '8','4','4',0,
	'8','4','5',0, '8','4','6',0, '8','4','7',0, '8','4','8',0, '8','4','9',0,
	'8','5','0',0, '8','5','1',0, '8','5','2',0, '8','5','3',0, '8','5','4',0,
	'8','5','5',0, '8','5','6',0, '8','5','7',0, '8','5','8',0, '8','5','9',0,
	'8','6','0',0, '8','6','1',0, '8','6','2',0, '8','6','3',0, '8','6','4',0,
	'8','6','5',0, '8','6','6',0, '8','6','7',0, '8','6','8',0, '8','6','9',0,
	'8','7','0',0, '8','7','1',0, '8','7','2',0, '8','7','3',0, '8','7','4',0,
	'8','7','5',0, '8','7','6',0, '8','7','7',0, '8','7','8',0, '8','7','9',0,
	'8','8','0',0, '8','8','1',0, '8','8','2',0, '8','8','3',0, '8','8','4',0,
	'8','8','5',0, '8','8','6',0, '8','8','7',0, '8','8','8',0, '8','8','9',0,
	'8','9','0',0, '8','9','1',0, '8','9','2',0, '8','9','3',0, '8','9','4',0,
	'8','9','5',0, '8','9','6',0, '8','9','7',0, '8','9','8',0, '8','9','9',0,
	'9','0','0',0, '9','0','1',0, '9','0','2',0, '9','0','3',0, '9','0','4',0,
	'9','0','5',0, '9','0','6',0, '9','0','7',0, '9','0','8',0, '9','0','9',0,
	'9','1','0',0, '9','1','1',0, '9','1','2',0, '9','1','3',0, '9','1','4',0,
	'9','1','5',0, '9','1','6',0, '9','1','7',0, '9','1','8',0, '9','1','9',0,
	'9','2','0',0, '9','2','1',0, '9','2','2',0, '9','2','3',0, '9','2','4',0,
	'9','2','5',0, '9','2','6',0, '9','2','7',0, '9','2','8',0, '9','2','9',0,
	'9','3','0',0, '9','3','1',0, '9','3','2',0, '9','3','3',0, '9','3','4',0,
	'9','3','5',0, '9','3','6',0, '9','3','7',0, '9','3','8',0, '9','3','9',0,
	'9','4','0',0, '9','4','1',0, '9','4','2',0, '9','4','3',0, '9','4','4',0,
	'9','4','5',0, '9','4','6',0, '9','4','7',0, '9','4','8',0, '9','4','9',0,
	'9','5','0',0, '9','5','1',0, '9','5','2',0, '9','5','3',0, '9','5','4',0,
	'9','5','5',0, '9','5','6',0, '9','5','7',0, '9','5','8',0, '9','5','9',0,
	'9','6','0',0, '9','6','1',0, '9','6','2',0, '9','6','3',0, '9','6','4',0,
	'9','6','5',0, '9','6','6',0, '9','6','7',0, '9','6','8',0, '9','6','9',0,
	'9','7','0',0, '9','7','1',0, '9','7','2',0, '9','7','3',0, '9','7','4',0,
	'9','7','5',0, '9','7','6',0, '9','7','7',0, '9','7','8',0, '9','7','9',0,
	'9','8','0',0, '9','8','1',0, '9','8','2',0, '9','8','3',0, '9','8','4',0,
	'9','8','5',0, '9','8','6',0, '9','8','7',0, '9','8','8',0, '9','8','9',0,
	'9','9','0',0, '9','9','1',0, '9','9','2',0, '9','9','3',0, '9','9','4',0,
	'9','9','5',0, '9','9','6',0, '9','9','7',0, '9','9','8',0, '9','9','9',0 };

char CIRCTools::sm_scratch_buffer[32];

DWORD CIRCTools::MakeSMask( DWORD dwOldMask, char* szArg, DWORD dwType )
{
	// If it begins with a +, count this as an additive mask instead of just
	// a replacement.  If what == MODE_DEL, "+" has no special effect.

	DWORD dwSMaskType;
	DWORD dwNewMask;

	if ( *szArg == '+' )
	{
		szArg++;
		if ( dwType == MODE_ADD)
			dwSMaskType = SMODE_ADD;
		else
			dwSMaskType = SMODE_DEL;
	}
	else if ( *szArg == '-' )
	{
		szArg++;
		if ( dwType == MODE_ADD)
			dwSMaskType = SMODE_DEL;
		else
			dwSMaskType = SMODE_ADD;
	}
	else
		dwSMaskType = ( dwType == MODE_ADD ) ? SMODE_SET : SMODE_DEL;

	dwNewMask = (DWORD)atoi( szArg );
	if ( dwSMaskType == SMODE_DEL )
		dwNewMask = dwOldMask & ~dwNewMask;
	else if ( dwSMaskType == SMODE_ADD)
		dwNewMask |= dwOldMask;
	return dwNewMask;
}

int CIRCTools::IsSMask( char* szWord )
{
	// Check to see if this resembles an SMask.  It is if 1) there is
	// at least one digit and 2) The first digit occurs before the first
	// alphabetic character.
	if ( szWord )
		for ( ; *szWord; szWord++ )
			if ( isdigit( *szWord ) )
				return 1;
			else if ( isalpha( *szWord ) )
				return 0;
	return 0;
}

char* CIRCTools::TheTime(time_t value)
{
  static char buf[28];
  Reg1 char *p;

  strcpy(buf, ctime(&value));
  if ((p = strchr(buf, '\n')) != NULL)
	*p = '\0';

  return buf;
}

// Convert2y[] converts a numeric to the corresponding character.
// The following characters are currently known to be forbidden:
//
// '\0' : Because we use '\0' as end of line.
//
// ' '	: Because parse_*() uses this as parameter seperator.
// ':'	: Because parse_server() uses this to detect if a prefix is a
//	  numeric or a name.
// '+'	: Because m_nick() uses this to determine if parv[6] is a
//	  umode or not.
// '&', '#', '+', '$', '@' and '%' :
//	  Because m_message() matches these characters to detect special cases.

const char CIRCTools::sm_cConvert2y[NUMNICKBASE] =
{
	'A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X',
	'Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3',
	'4','5','6','7','8','9','[',']'
};

const BYTE CIRCTools::sm_ucConvert2n[NUMNICKMAXCHAR + 1] =
{
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	52, 53, 54, 55, 56, 57, 58, 59,
	60, 61,  0,  0,  0,  0,  0,  0,
	 0,  0,  1,  2,  3,  4,  5,  6,
	 7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22,
	23, 24, 25, 62,  0, 63,  0,  0,
	 0, 26, 27, 28,	29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51
};

const char* CIRCTools::IntToBase64(unsigned int i)
{
	static char sm_base64buf[7];
	sm_base64buf[0] = sm_cConvert2y[(i >> 30) & 0x3f];
	sm_base64buf[1] = sm_cConvert2y[(i >> 24) & 0x3f];
	sm_base64buf[2] = sm_cConvert2y[(i >> 18) & 0x3f];
	sm_base64buf[3] = sm_cConvert2y[(i >> 12) & 0x3f];
	sm_base64buf[4] = sm_cConvert2y[(i >> 6) & 0x3f];
	sm_base64buf[5] = sm_cConvert2y[i & 0x3f];
	// base64buf[6] = 0; (static is initialized 0)
	return sm_base64buf;
}

unsigned int CIRCTools::Base64ToInt(const char *str)
{
	register unsigned int i;
	i =  sm_ucConvert2n[(unsigned char)str[5]];
	i += sm_ucConvert2n[(unsigned char)str[4]] << 6;
	i += sm_ucConvert2n[(unsigned char)str[3]] << 12;
	i += sm_ucConvert2n[(unsigned char)str[2]] << 18;
	i += sm_ucConvert2n[(unsigned char)str[1]] << 24;
	i += sm_ucConvert2n[(unsigned char)str[0]] << 30;
	return i;
}

int CIRCTools::match(const char *mask, const char *name)
{
#if 0
	return( Str_Match( mask, name ) == MATCH_VALID );
#else
	// Compare if a given string (name) matches the given
	// mask (which can contain wild cards: '*' - match any
	// number of chars, '?' - match any single character.
	//
	// return  0, if match
	//		 1, if no match
	//

	const char *m = mask;
	const char *n = name;
	const char *ma = mask;
	const char *na = name;
	int wild = 0;
	int q = 0;

	while (1)
	{
		if (*m == '*')
		{
			while (*m == '*')
			m++;
			wild = 1;
			ma = m;
			na = n;
		}

		if (!*m)
		{
			if (!*n)
			return 0;
			for (m--; (m > mask) && (*m == '?'); m--)
				;
			if ((*m == '*') && (m > mask) && (m[-1] != '\\'))
				return 0;
			if (!wild)
				return 1;
			m = ma;
			n = ++ na;
		}
		else if (!*n)
		{
			while (*m == '*')
				m++;
			return (*m != 0);
		}
		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
		{
			m++;
			q = 1;
		}
		else
			q = 0;

		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q))
		{
			if (!wild)
				return 1;
			m = ma;
			n = ++na;
		}
		else
		{
			if (*m)
				m++;
			if (*n)
				n++;
		}
	}
#endif

}

#pragma warning( disable : 4146 )
char * CIRCTools::vsprintf_irc(register char *str, register const char *format, register va_list vl)
{
#if 1
	return str + vsprintf(str, format, vl);
#else

	// Much faster than (v)sprintf and will fall back to that if this can't deal with something
	register char c;

	while ((c = *format++))
	{
		if (c == '%')
		{
			c = *format++; /* May never be '\0' ! */
			if (c == 'c')
			{
				*str++ = (char)va_arg(vl, int);
				continue;
			}
			if (c == 's')
			{
				register const char *p1 = va_arg(vl, const char *);
					if ((*str = *p1))
				while ((*++str = *++p1));
					continue;
				}
			if (c == 'l' && *format == 'u')	/* Prints time_t value in interval
	 					   [ 100000000 , 4294967295 ]
				   Actually prints like "%09lu" */
			{
				register unsigned long v1, v2;
				register const char *ap;
				++format;
				v1 = va_arg(vl, unsigned long);
					if (v1 > 999999999L)
					{
					v2 = v1 / 1000000000;
					v1 -= v2 * 1000000000;
					/* let the compiler do the promotion, then */
					/* promptly undo it. (CAST) */
					*str++ = (char)(v2 + '0');
				}
				v2 = v1 / 1000000;
				v1 -= v2 * 1000000;
				ap = atoi_tab + (v2 << 2);
				*str++ = *ap++;
				*str++ = *ap++;
				*str++ = *ap;
				v2 = v1 / 1000;
				v1 -= v2 * 1000;
				ap = atoi_tab + (v2 << 2);
				*str++ = *ap++;
				*str++ = *ap++;
				*str++ = *ap;
				ap = atoi_tab + (v1 << 2);
				*str++ = *ap++;
				*str++ = *ap++;
				*str++ = *ap;
				continue;
			}
			if (c == 'd')
			{
				register unsigned int v1, v2;
				register const char *ap;
				register char *s = &scratch_buffer[sizeof(scratch_buffer) - 2];
				v1 = va_arg(vl, int);
				if ((int)v1 <= 0)
				{
					if (v1 == 0)
					{
						*str++ = '0';
						continue;
					}
					*str++ = '-';
					v1 = -v1;
				}
				do
				{
					v2 = v1 / 1000;
					ap = atoi_tab + 2 + ((v1 - 1000 * v2) << 2);
					*s-- = *ap--;
					*s-- = *ap--;
					*s-- = *ap;
				} while ((v1 = v2) > 0);
				while ('0' == *++s);
				*str = *s;
				while ((*++str = *++s));
				continue;
			}
			if (c == 'u')
			{
				register unsigned int v1, v2;
				register const char *ap;
				register char *s = &scratch_buffer[sizeof(scratch_buffer) - 2];
				v1 = va_arg(vl, unsigned int);
				if (v1 == 0)
				{
					*str++ = '0';
					continue;
				}
				do
				{
					v2 = v1 / 1000;
					ap = atoi_tab + 2 + ((v1 - 1000 * v2) << 2);
					*s-- = *ap--;
					*s-- = *ap--;
					*s-- = *ap;
				} while ((v1 = v2) > 0);
				while ('0' == *++s);
				*str = *s;
				while ((*++str = *++s));
				continue;
			}
			if (c != '%')
			{
				format -= 2;
				str += vsprintf(str, format, vl);
				break;
			}
		}
		*str++ = c;
	}
	*str = 0;
	return str;
#endif

}
#pragma warning( default : 4146 )

char * _cdecl CIRCTools::sprintf_irc(register char *str, const char *format, ...)
{
	va_list vargs;
	va_start(vargs, format);
	char * pszret = vsprintf_irc(str, format, vargs);
	va_end(vargs);
	return pszret;
}

char * CIRCTools::inetntoa(CSocketAddressIP in)
{
	// inet_ntoa ??? why not use it ?
	static char buf[32];
	Reg1 BYTE *s = (u_char *)&in.s_addr;
	Reg2 BYTE a, b, c, d;

	a = *s++;
	b = *s++;
	c = *s++;
	d = *s++;
	sprintf_irc(buf, "%u.%u.%u.%u", a, b, c, d);

	return buf;
}

CIRCNumeric CIRCTools::sm_NumericErrors[] =
{
	{ ERR_NOSUCHNICK, "%s :No such nick" },											// 401
	{ ERR_NOSUCHSERVER, "%s :No such server" },
	{ ERR_NOSUCHCHANNEL, "%s :No such channel" },
	{ ERR_CANNOTSENDTOCHAN, "%s :Cannot send to channel" },
	{ ERR_TOOMANYCHANNELS, "%s :You have joined too many channels" },
	{ ERR_WASNOSUCHNICK, "%s :There was no such nickname" },
	{ ERR_TOOMANYTARGETS, "%s :Duplicate recipients. No message delivered" },
	{ 0, (char *)NULL },
	{ ERR_NOORIGIN, ":No origin specified" },
	{ 0, (char *)NULL },
	{ ERR_NORECIPIENT, ":No recipient given (%s)" },								// 411
	{ ERR_NOTEXTTOSEND, ":No text to send" },
	{ ERR_NOTOPLEVEL, "%s :No toplevel domain specified" },
	{ ERR_WILDTOPLEVEL, "%s :Wildcard in toplevel Domain" },
	{ 0, (char *)NULL },
	{ ERR_QUERYTOOLONG, "%s :Too many lines in the output, restrict your query" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_UNKNOWNCOMMAND, "%s :Unknown command" },									// 421
	{ ERR_NOMOTD, ":MOTD File is missing" },
	{ ERR_NOADMININFO, "%s :No administrative info available" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NONICKNAMEGIVEN, ":No nickname given" },									// 431
	{ ERR_ERRONEUSNICKNAME, "%s :Erroneus Nickname" },
	{ ERR_NICKNAMEINUSE, "%s :Nickname is already in use." },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NICKCOLLISION, "%s :Nickname collision KILL" },
	{ ERR_BANNICKCHANGE, "%s :Cannot change nickname while banned on channel" },
	{ ERR_NICKTOOFAST, "%s :Nick change too fast. Please wait %d seconds." },
	{ ERR_TARGETTOOFAST, "%s :Target change too fast. Please wait %d seconds." },
	{ 0, (char *)NULL },
	{ ERR_USERNOTINCHANNEL, "%s %s :They aren't on that channel" },					// 441
	{ ERR_NOTONCHANNEL, "%s :You're not on that channel" },
	{ ERR_USERONCHANNEL, "%s %s :is already on channel" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NOTREGISTERED, ":You have not registered" },								// 451
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NEEDMOREPARAMS, "%s :Not enough parameters" },							// 461
	{ ERR_ALREADYREGISTRED, ":You may not reregister" },
	{ ERR_NOPERMFORHOST, ":Your host isn't among the privileged" },
	{ ERR_PASSWDMISMATCH, ":Password Incorrect" },
	{ ERR_YOUREBANNEDCREEP, ":You are banned from this server" },
	{ ERR_YOUWILLBEBANNED, (char *)NULL },
	{ ERR_KEYSET, "%s :Channel key already set" },
	{ ERR_INVALIDUSERNAME, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_CHANNELISFULL, "%s :Cannot join channel (+l)" },							// 471
	{ ERR_UNKNOWNMODE, "%c :is unknown mode char to me" },
	{ ERR_INVITEONLYCHAN, "%s :Cannot join channel (+i)" },
	{ ERR_BANNEDFROMCHAN, "%s :Cannot join channel (+b)" },
	{ ERR_BADCHANNELKEY, "%s :Cannot join channel (+k)" },
	{ ERR_BADCHANMASK, "%s :Bad Channel Mask" },
	{ ERR_MODELESS, "%s :Channel doesn't support modes" },
	{ ERR_BANLISTFULL, "%s %s :Channel ban/ignore list is full" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NOPRIVILEGES, ":Permission Denied- You're not an IRC operator" },			// 481
	{ ERR_CHANOPRIVSNEEDED, "%s :You're not a channel operator" },
	{ ERR_CANTKILLSERVER, ":You cant kill a server!" },
	{ ERR_ISCHANSERVICE, "%s %s :Cannot kill, kick or deop channel service" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_NOOPERHOST, ":No O-lines for your host" },								// 491
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_UMODEUNKNOWNFLAG, ":Unknown MODE flag" },									// 501
	{ ERR_USERSDONTMATCH, ":Cant change mode for other users" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ ERR_SILELISTFULL, "%s :Your silence list is full" },							// 511
	{ ERR_NOSUCHGLINE, "%s@%s :No such gline" },
	{ ERR_BADPING, (char *)NULL}
};

CIRCNumeric CIRCTools::sm_LocalReplies[] =
{
	{ 0, (char *)NULL },															// 000
	{ RPL_WELCOME, ":Welcome to Sphere's Internet Relay Network %s" },
	{ RPL_YOURHOST, ":Your host is %s, running version %s" },
	{ RPL_CREATED, ":This server was created %s" },
	{ RPL_MYINFO, "%s %s" },
	{ RPL_MAP, ":%s%s" },
	{ RPL_MAPMORE, ":%s%s --> *more*" },
	{ RPL_MAPEND, ":End of /MAP" },
	{ RPL_SNOMASK, "%d :: Server notice mask (%#x)" },
	{ RPL_STATMEMTOT, "%u %u :Bytes Blocks" },
	{ RPL_STATMEM, "%u %u %s" },													// 010
	{ 0, (char *)NULL }
};

CIRCNumeric CIRCTools::sm_NumericReplies[] =
{
	{ RPL_NONE, (char *)NULL },														// 300
	{ RPL_AWAY, "%s :%s" },
	{ RPL_USERHOST, ":" },
	{ RPL_ISON, ":" },
	{ RPL_TEXT, (char *)NULL },
	{ RPL_UNAWAY, ":You are no longer marked as being away" },
	{ RPL_NOWAWAY, ":You have been marked as being away" },
	{ RPL_USERIP, ":" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 310
	{ RPL_WHOISUSER, "%s %s %s * :%s" },
	{ RPL_WHOISSERVER, "%s %s :%s" },
	{ RPL_WHOISOPERATOR, "%s :is an IRC Operator" },
	{ RPL_WHOWASUSER, "%s %s %s * :%s" },
	{ RPL_ENDOFWHO, "%s :End of /WHO list." },
	{ 0, (char *)NULL },
	{ RPL_WHOISIDLE, "%s %ld %ld :seconds idle, signon time" },
	{ RPL_ENDOFWHOIS, "%s :End of /WHOIS list." },
	{ RPL_WHOISCHANNELS, "%s :%s" },
	{ 0, (char *)NULL },															// 320
	{ RPL_LISTSTART, "Channel :Users  Name" },
	{ RPL_LIST, "%s %d :%s" },
	{ RPL_LISTEND, ":End of /LIST" },
	{ RPL_CHANNELMODEIS, "%s %s %s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ RPL_CREATIONTIME, "%s %lu" },
	{ 0, (char *)NULL },															// 330
	{ RPL_NOTOPIC, "%s :No topic is set." },
	{ RPL_TOPIC, "%s :%s" },
	{ RPL_TOPICWHOTIME, "%s %s %lu" },
	{ RPL_LISTUSAGE, ":%s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 340
	{ RPL_INVITING, "%s %s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 350
	{ RPL_VERSION, "%s.%s %s :%s" },
	{ RPL_WHOREPLY, "%s" },
	{ RPL_NAMREPLY, "%s" },
	{ RPL_WHOSPCRPL, "%s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 360
	{ RPL_KILLDONE, (char *)NULL },
	{ RPL_CLOSING, "%s :Closed. Status = %d" },
	{ RPL_CLOSEEND, "%d: Connections Closed" },
	{ RPL_LINKS, "%s %s :%d P%u %s" },
	{ RPL_ENDOFLINKS, "%s :End of /LINKS list." },
	{ RPL_ENDOFNAMES, "%s :End of /NAMES list." },
	{ RPL_BANLIST, "%s %s %s %lu" },
	{ RPL_ENDOFBANLIST, "%s :End of Channel Ban List" },
	{ RPL_ENDOFWHOWAS, "%s :End of WHOWAS" },
	{ 0, (char *)NULL },															// 370
	{ RPL_INFO, ":%s" },
	{ RPL_MOTD, ":- %s" },
	{ RPL_INFOSTART, ":Server INFO" },
	{ RPL_ENDOFINFO, ":End of /INFO list." },
	{ RPL_MOTDSTART, ":- %s Message of the Day - " },
	{ RPL_ENDOFMOTD, ":End of /MOTD command." },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 380
	{ RPL_YOUREOPER, ":You are now an IRC Operator" },
	{ RPL_REHASHING, "%s :Rehashing" },
	{ 0, (char *)NULL },
	{ RPL_MYPORTIS, "%d :Port to local server is\r\n" },
	{ RPL_NOTOPERANYMORE, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 390
	{ RPL_TIME, "%s %lu %ld :%s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ RPL_TRACELINK, "Link %s%s %s %s" },											// 200
	{ RPL_TRACECONNECTING, "Try. %d %s" },
	{ RPL_TRACEHANDSHAKE, "H.S. %d %s" },
	{ RPL_TRACEUNKNOWN, "???? %d %s" },
	{ RPL_TRACEOPERATOR, "Oper %d %s %ld" },
	{ RPL_TRACEUSER, "User %d %s %ld" },
	{ RPL_TRACESERVER, "Serv %d %dS %dC %s %s!%s@%s %ld %ld" },
	{ 0, (char *)NULL },
	{ RPL_TRACENEWTYPE, "<newtype> 0 %s" },
	{ RPL_TRACECLASS, "Class %d %d" },
	{ 0, (char *)NULL },															// 210
	{ RPL_STATSLINKINFO, (char *)NULL },
	{ RPL_STATSCOMMANDS, "%s %u %u" },
	{ RPL_STATSCLINE, "%c %s * %s %d %d" },
	{ RPL_STATSNLINE, "%c %s * %s %d %d" },
	{ RPL_STATSILINE, "%c %s * %s %d %d" },
	{ RPL_STATSKLINE, "%c %s %s %s %d %d" },
	{ RPL_STATSPLINE, "%c %d %d %#x" },
	{ RPL_STATSYLINE, "%c %d %d %d %d %ld" },
	{ RPL_ENDOFSTATS, "%c :End of /STATS report" },
	{ 0, (char *)NULL },															// 220
	{ RPL_UMODEIS, "%s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 230
	{ RPL_SERVICEINFO, (char *)NULL },
	{ RPL_ENDOFSERVICES, (char *)NULL },
	{ RPL_SERVICE, (char *)NULL },
	{ RPL_SERVLIST, (char *)NULL },
	{ RPL_SERVLISTEND, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 240
	{ RPL_STATSLLINE, "%c %s * %s %d %d" },
	{ RPL_STATSUPTIME, ":Server Up %d days, %d:%02d:%02d" },
	{ RPL_STATSOLINE, "%c %s * %s %d %d" },
	{ RPL_STATSHLINE, "%c %s * %s %d %d" },
	{ 0, (char *)NULL },
	{ RPL_STATSTLINE, "%c %s %s" },
	{ RPL_STATSGLINE, "%c %s@%s %lu :%s" },
	{ RPL_STATSULINE, "%c %s * %s %d %d" },
	{ 0, (char *)NULL },
	{ RPL_STATSCONN, ":Highest connection count: %d (%d clients)" },				// 250
	{ RPL_LUSERCLIENT, ":There are %d users and %d invisible on %d servers" },
	{ RPL_LUSEROP, "%d :operator(s) online" },
	{ RPL_LUSERUNKNOWN, "%d :unknown connection(s)" },
	{ RPL_LUSERCHANNELS, "%d :channels formed" },
	{ RPL_LUSERME, ":I have %d clients and %d servers" },
	{ RPL_ADMINME, ":Administrative info about %s" },
	{ RPL_ADMINLOC1, ":%s" },
	{ RPL_ADMINLOC2, ":%s" },
	{ RPL_ADMINEMAIL, ":%s" },
	{ 0, (char *)NULL },															// 260
	{ RPL_TRACELOG, "File %s %d" },
	{ RPL_TRACEPING, "Ping %s %s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },															// 270
	{ RPL_SILELIST, "%s %s" },
	{ RPL_ENDOFSILELIST, ":End of Silence List" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ RPL_STATSDLINE, "%c %s %s" },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ 0, (char *)NULL },
	{ RPL_GLIST, "%s@%s %lu %s%s" },												// 280
	{ RPL_ENDOFGLIST, ":End of G-line List"}
};

char CIRCTools::sm_szNumericBuffer[512];

char* CIRCTools::ErrorString( int iNumeric )
{
  Reg1 CIRCNumeric *pNumeric;
  Reg2 int iNum = iNumeric;

  iNum -= sm_NumericErrors[0].m_iNumericValue;
  pNumeric = &sm_NumericErrors[iNum];
  PrepareBuffer( sm_szNumericBuffer, pNumeric->m_iNumericValue, pNumeric->m_szNumericPattern );

  return sm_szNumericBuffer;
}

char* CIRCTools::ReplyString( int iNumeric )
{
  Reg1 CIRCNumeric *pNumeric;
  Reg2 int iNum = iNumeric;

  if ( iNum > ( sizeof( sm_LocalReplies ) / sizeof( CIRCNumeric ) - 2) )
	iNum -= ( iNum > 300) ? 300 : 100;

  if ( iNumeric > 99 )
	pNumeric = &sm_NumericReplies[iNum];
  else
	pNumeric = &sm_LocalReplies[iNum];

  PrepareBuffer( sm_szNumericBuffer, pNumeric->m_iNumericValue, pNumeric->m_szNumericPattern );

  return sm_szNumericBuffer;
}

char * CIRCTools::check_string(char *s)
{
	//
	// Fixes a string so that the first white space found becomes an end of
	// string marker (`\-`).  returns the 'fixed' string or "*" if the string
	// was NULL length or a NULL pointer.
	//
	static char star[2] = "*";
	char *str = s;

	if (BadPtr(s))
		return star;

	for (; *s; s++)
		if (isspace(*s))
		{
			*s = '\0';
			break;
		}

	return (BadPtr(str)) ? star : str;
}

unsigned long CIRCTools::ircrandom(void)
{
#if 0
	return( Calc_GetRandVal( 0xFFFFFFFF ));	// RAND_MAX
#else
	//
	// MD5 transform algorithm, taken from code written by Colin Plumb,
	// and put into the public domain
	//
	// Kev: Taken from Ted T'so's /dev/random random.c code and modified to
	// be slightly simpler.	 That code is released under a BSD-style copyright
	// OR under the terms of the GNU Public License, which should be included
	// at the top of this source file.
	//
	// record: Cleaned up to work with ircd.  RANDOM_TOKEN is defined in
	// setup.h by the make script; if people start to "guess" your cookies,
	// consider recompiling your server with a different random token.
	//
	//
	// The core of the MD5 algorithm, this alters an existing MD5 hash to
	// reflect the addition of 16 longwords of new data.  MD5Update blocks
	// the data and converts bytes into longwords for this routine.
	//
	// original comment left in; this used to be called MD5Transform and took
	// two arguments; I've internalized those arguments, creating the character
	// array "sm_localkey," which should contain 8 bytes of data.  The function also
	// originally returned nothing; now it returns an unsigned long that is the
	// random number.  It appears to be reallyrandom, so... -Kev
	//
	// I don't really know what this does.	I tried to figure it out and got
	// a headache.	If you know what's good for you, you'll leave this stuff
	// for the smart people and do something else.		-record
	//

#define RANDOM_SEED "12345678"

	static const char sm_localkey[] = RANDOM_SEED;

	unsigned char in[16];
	struct timeval tv;

	gettimeofday(&tv, NULL);

	memcpy((void *)in, (void *)sm_localkey, 8);
	memcpy((void *)(in + 8), (void *)&tv.tv_sec, 4);
	memcpy((void *)(in + 12), (void *)&tv.tv_usec, 4);

	DWORD a = 0x67452301;
	DWORD b = 0xefcdab89;
	DWORD c = 0x98badcfe;
	DWORD d = 0x10325476;

/* The four core functions - F1 is optimized somewhat */

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

	MD5STEP(F1, a, b, c, d, (long)in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, (long)in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, (long)in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, (long)in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, (long)in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, (long)in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, (long)in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, (long)in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, (long)in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, (long)in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, (long)in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, (long)in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, (long)in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, (long)in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, (long)in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, (long)in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, (long)in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, (long)in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, (long)in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, (long)in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, (long)in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, (long)in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, (long)in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, (long)in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, (long)in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, (long)in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, (long)in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, (long)in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, (long)in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, (long)in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, (long)in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, (long)in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, (long)in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, (long)in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, (long)in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, (long)in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, (long)in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, (long)in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, (long)in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, (long)in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, (long)in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, (long)in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, (long)in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, (long)in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, (long)in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, (long)in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, (long)in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, (long)in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, (long)in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, (long)in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, (long)in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, (long)in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, (long)in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, (long)in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, (long)in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, (long)in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, (long)in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, (long)in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, (long)in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, (long)in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, (long)in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, (long)in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, (long)in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, (long)in[9] + 0xeb86d391, 21);

	//
	// We have 4 unsigned longs generated by the above sequence; this scrambles
	// them together so that if there is any pattern, it will be obscured.
	//
	return (a ^ b ^ c ^ d);
#endif
}

char* CIRCTools::strtoken(char **save, char *str, char *fs)
{
	// Walk through a string of tokens, using a set of separators.
	char *pos = *save;		/* keep last position across calls */
	Reg1 char *tmp;

	if (str)
		pos = str;			/* new string scan */

	while (pos && *pos && strchr(fs, *pos) != NULL)
		pos++;			/* skip leading separators */

	if (!pos || !*pos)
		return (pos = *save = NULL);	/* string contains only sep's */

	tmp = pos;			/* now, keep position of the token */

	while (*pos && strchr(fs, *pos) == NULL)
		pos++;			/* skip content of the token */

	if (*pos)
		*pos++ = '\0';		/* remove first sep after the token */
	else
		pos = NULL;			/* end of string */

	*save = pos;
	return (tmp);
}
#ifndef _BSD
int CIRCTools::gettimeofday(struct timeval* tv, void* v)
{
	// This is copied from Bleeps brain (without permission), and vica versa ;)
	struct _timeb tb;
	_ftime(&tb);
	ASSERT(0 != tv);
	tv->tv_sec = tb.time;
	tv->tv_usec = tb.millitm * 1000;
	return 0;
}
#endif

char * CIRCTools::collapse(char *pattern)
{
	//
	// collapse a pattern string into minimal components.
	// This particular version is "in place", so that it changes the pattern
	// which is to be reduced to a "minimal" size.
	//
	Reg1 char *s = pattern, *s1, *t;

	if (BadPtr(pattern))
		return pattern;
	//
	// Collapse all \** into \*, \*[?]+\** into \*[?]+
	//
	for (; *s; s++)
		if (*s == '\\')
			if (!*(s + 1))
				break;
			else
				s++;
		else if (*s == '*')
		{
			if (*(t = s1 = s + 1) == '*')
				while (*t == '*')
					t++;
			else if (*t == '?')
				for (t++, s1++; *t == '*' || *t == '?'; t++)
					if (*t == '?')
						*s1++ = *t;
			while ((*s1++ = *t++));
		}
	return pattern;
}

/*
 * CIRCMachine
 *
 */

CIRCMachine::CIRCMachine()
{
	memset( m_szPassword, 0, PASSWDLEN );
	memset( m_szNick, 0, NICKLEN );
	memset( m_szUser, 0, USERLEN );
	memset( m_szHost, 0, HOSTLEN );
	memset( m_szServerName, 0, HOSTLEN );
	memset( m_szRealName, 0, REALLEN );

	m_dwFlags = 0x00000000;
	m_fAuthenticated = false;
}

CIRCMachine::~CIRCMachine()
{

}

char* CIRCMachine::PrettyMask( char *szMask )
{
  Reg1 char *szC;
  Reg2 char *szUser;
  Reg3 char *szHost;

  if ( ( szUser = strchr( ( szC = szMask ), '!' ) ) )
    *szUser++ = '\0';
  if ( ( szHost = strrchr( szUser ? szUser : szC, '@')))
  {
    *szHost++ = '\0';
    if ( !szUser )
      return GetNickUserIP( NULL, szC, szHost );
  }
  else if (!szUser && strchr( szC, '.' ) )
    return GetNickUserHost( NULL, NULL, szC );
  return GetNickUserHost( szC, szUser, szHost );
}

char* CIRCMachine::GetNickUserIP( char* szTempNick, char* szTempUser,char* szTempIP )
{
	//
	// Create a string of form "foo!bar@123.456.789.123"
	//
	static char ipbuf[NICKLEN + USERLEN + 16 + 3];
	register char *s = ipbuf;
	register size_t len;
	char szNick[NICKLEN+1];
	char szUser[USERLEN+1];

	strcpy( szNick, CIRCTools::check_string( szTempNick ) );
	strcpy( szUser, CIRCTools::check_string( szTempUser ) );

	len = strlen( szNick );
	if (len > NICKLEN)
		len = NICKLEN;
	strncpy( ipbuf, szNick, len );
	s += len;
	*s++ = '!';

	strcpy( szUser, CIRCTools::check_string( szTempUser ) );
	len = strlen( szUser );
	if (len > USERLEN)
		len = USERLEN;
	strncpy(s, szUser, len);
	s += len;
	*s++ = '@';
	strcpy( s, szTempIP );
	return (ipbuf);
}

char* CIRCMachine::GetNickUserIP( char* szTempNick )
{
	static char ipbuf[NICKLEN + USERLEN + 16 + 3];
	char szNick[NICKLEN+1];

	if ( !szTempNick )
		strcpy( szNick, CIRCTools::check_string( m_szNick ) );
	else
		strcpy( szNick, CIRCTools::check_string( szTempNick ) );

	return GetNickUserIP( szNick, m_szUser, CIRCTools::inetntoa( m_iaClientIP ) );
}

char* CIRCMachine::GetNickUserHost( char* szTempNick, char* szTempUser,char* szTempHost )
{
	//
	// Create a string of form "foo!bar@fubar" given foo, bar and fubar
	// as the parameters.  If NULL, they become "*".
	//
	static char namebuf[NICKLEN + USERLEN + HOSTLEN + 3];
	register char *s = namebuf;
	register size_t len;
	char szNick[NICKLEN+1];
	char szUser[USERLEN+1];
	char szHost[HOSTLEN+1];

	strcpy( szNick, CIRCTools::check_string( szTempNick ) );
	len = strlen( szNick );
	if (len > NICKLEN)
	len = NICKLEN;
	strncpy( namebuf, szNick, len );
	s += len;
	*s++ = '!';

	strcpy( szUser, CIRCTools::check_string( szTempUser ) );
	len = strlen( szUser );
	if (len > USERLEN)
		len = USERLEN;
	strncpy( s, szUser, len );
	s += len;
	*s++ = '@';

	strcpy( szHost, CIRCTools::check_string( szTempHost ) );
	len = strlen( szHost );
	if (len > HOSTLEN)
		len = HOSTLEN;
	strncpy(s, szHost, len);
	s += len;
	*s = '\0';
	return (namebuf);
}

char* CIRCMachine::GetNickUserHost()
{
	return GetNickUserHost( m_szNick, m_szUser, m_szHost);
}

void CIRCMachine::Authenticate(bool fAuthenticate)
{
	m_fAuthenticated = fAuthenticate;
}

/*
 * CIRCLRClient
 *
 */

CIRCLRClient::CIRCLRClient()
{
	m_iTargets = 0;
	m_iHops = 0;
	m_pServer = NULL;
}

CIRCLRClient::~CIRCLRClient()
{
}

void CIRCLRClient::SetFlag( DWORD dwMode )
{
	if ( dwMode & FLAGS_INVISIBLE )
	{
		if ( !IsFlag( FLAGS_INVISIBLE ) )
		{
			// I'm becoming invisible
			m_dwFlags |= FLAGS_INVISIBLE;
			// TODO: Tell whoever should know that I did this (opers, channel members, myself, etc)
			return;
		}
	}
}

void CIRCLRClient::ClearFlag( DWORD dwMode )
{
	if ( ( dwMode & FLAGS_INVISIBLE ) )
	{
		if ( IsFlag( FLAGS_INVISIBLE ) )
		{
			// I'm becoming visible
			m_dwFlags &= ~FLAGS_INVISIBLE;
			// TODO: Tell whoever should know that I did this
			// (opers, channel members, myself, remote servers, etc)
			return;
		}
	}
}

/*
 * CIRCServer
 *
 */

CIRCServer::CIRCServer()
{
	m_iClients = 0;
	m_iInvisibleClients = 0;
	m_iHops = 0;
	m_tOnLineSince = 0;
	m_tLinkedAt = 0;
	m_szProtocol[0] = 0;
	m_szDescription[0] = 0;
	m_fNeedsSync = true;
}

CIRCServer::~CIRCServer()
{
}

void CIRCServer::SetFlag( DWORD wMode )
{
	return;
}

void CIRCServer::ClearFlag( DWORD wMode )
{
	return;
}

/*
 * CIRCNumNicks
 *
 */

void CIRCNumNicks::AssignNumNick( CIRCNick* pNick, char* szNumNick )
{
	ASSERT( strlen( szNumNick ) == 3 );
	int iSlot = GetSlot( szNumNick );
	ASSERT( iSlot < MAX_MAXCLIENTS );
	m_NumNicks[iSlot].m_pNick = pNick;
	m_NumNicks[iSlot].m_iSlot = iSlot;
}

/*
 * CIRCMyNumNicks
 *
 */

CIRCNumNick* CIRCMyNumNicks::GetNextSlot()
{
	int iSaved = m_iNextSlot;
	while ( m_iNextSlot < MAX_MAXCLIENTS - 1 )	// I guess the last slot is the server itself and it can't be given out?
	{
		CIRCNumNick* pNextNumNick = &m_NumNicks[m_iNextSlot];
		m_iNextSlot++;
		if ( !pNextNumNick->m_pNick )
			// This is free
			return pNextNumNick;
	}
	// No more free ones, see if we can reuse an old previously used but now free one
	for( int i = 0; i < iSaved; i++ )
	{
		CIRCNumNick* pNextNumNick = &m_NumNicks[i];
		if ( !pNextNumNick->m_pNick )
		{
			// This is free
			m_iNextSlot = i + 1;
			return pNextNumNick;
		}
	}
	return NULL;
}

/*
 * CIRCLocalServer
 *
 */

CIRCLocalServer::CIRCLocalServer()
{
	time( &m_tNicksTimer );
	time( &m_tOnlineSince );
	strcpy( m_szDeamonName, "SphereIRC" );
	strcpy( m_szProtocol, "J10" );
	sprintf( m_szVersion, "%s Version 1.0 alpha", m_szDeamonName );
	strcpy( m_szCompile, "1" );
	strcpy( m_szCopyright, "Copyright 2000 Menasoft" );
	m_TSoffset = 0;
	m_wPort = 6667;
}

void CIRCLocalServer::CleanChannelName(char *cn)
{
	//
	// Remove bells and commas from channel name
	//

	for (; *cn; cn++)
	{
		if (*cn == '\007' || *cn == ' ' || *cn == ',')
		{
			*cn = '\0';
			return;
		}
	}
}

int CIRCLocalServer::AnalyzeNick( char *szNick )
{
	//
	// 'AnalyzeNick' ensures that the given parameter (nick) is really a proper
	// string for a nickname (note, the 'nick' may be modified in the process...)
	//
	// RETURNS the length of the final NICKNAME (0, if nickname is illegal)
	//
	// Nickname characters are in range 'A'..'}', '_', '-', '0'..'9'
	// anything outside the above set will terminate nickname.
	// In addition, the first character cannot be '-' or a Digit.
	//

	Reg1 char *ch;

	if ( *szNick == '-' || isdigit( *szNick ) )	/* first character in [0..9-] */
		return 0;

	for ( ch = szNick; *ch && ( ch - szNick ) < NICKLEN; ch++ )
	{
		if ( !isvalid( *ch ) || isspace( *ch ) )
			break;
	}
	*ch = '\0';

	return ( ch - szNick );
}

void CIRCLocalServer::RemoveMachine( CIRCMachine *pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);
	for(int i = 0; i < m_LClients.GetCount(); i++)
	{
		if ( m_LClients[i] == pLClient )
			m_LClients.RemoveAt(i);
	}
	return;
}

CIRCLRClient* CIRCLocalServer::HuntClientByIP( CSocketAddressIP iaLClientIP )
{
	// Check all my lists of attached clients
	for( int i = 0; i < m_LClients.GetCount(); i++ )
	{
		if ( iaLClientIP.s_addr == m_LClients[i]->m_iaClientIP.s_addr )
			return m_LClients[i];
	}
	return NULL;
}

CIRCLRClient* CIRCLocalServer::ChaseClientByNick( char* szNick )
{
	// We're killing, kicking or setting a mode on someone here, and
	// we want the right person (if they did something bad, and then changed
	// their nick to avoid us, check the history

	CIRCLRClient* pLClient = NULL;

	// First see if they're using the same nick
	pLClient = HuntClientByNick( szNick );
	if ( pLClient )
		return pLClient;

	// Ok, they changed their nick, check the history to see if we can find them
	CIRCNick* pNick = FindNick( szNick );
	if ( pNick )
	{
		// Return a pointer to a client that had this nick not more then
		// KILLCHASETIMELIMIT seconds ago, if still on line. Otherwise return NULL
		time_t tNow;
		time( &tNow );
		time_t tTimeLimit = tNow - KILLCHASETIMELIMIT;
		CIRCNickTrack* pNickTrack = pNick->FindHistory( tTimeLimit );
		if ( pNickTrack )
		{
			// Ok, who is this guy now?
			pLClient = HuntClientByIP( pNickTrack->m_iaClientIP );
		}
	}
	return pLClient;
}

CIRCLRClient* CIRCLocalServer::HuntClientByNick( char* szNick )
{
	// Check all my lists of attached clients
	int i;
	for( i = 0; i < m_LClients.GetCount(); i++ )
	{
		CIRCLRClient* pLClient = m_LClients[i];
		if ( !strcmp(pLClient->m_szNick, szNick ) )
			return pLClient;
	}
	for( i = 0; i < m_RClients.GetCount(); i++ )
	{
		CIRCLRClient* pRClient = m_RClients[i];
		if ( !strcmp( pRClient->m_szNick, szNick ) )
			return pRClient;
	}
	return NULL;
}

CIRCChannel* CIRCLocalServer::HuntChannel( char* szChName, char* szKey, CIRCMachine *pMachine )
{
	for(int i = 0; i < m_Channels.GetCount(); i++)
	{
		if ( !strcmp(m_Channels[i]->m_szChName, szChName ) )
			return m_Channels[i];
	}
	return CreateNewChannel( pMachine, szChName, szKey );
}

void CIRCLocalServer::Send_RPL_LIST( CIRCMachine *pMachine, int iMin, int iMax, char* szChName )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	if ( szChName == NULL )
	{
		// Send all qualifying channels out
		for(int i = 0; i < m_Channels.GetCount(); i++)
		{
			CIRCChannel* pChannel = m_Channels[i];
			if ( pChannel->CanList() )
			{
				char szPublicName[CHANNELLEN];
				pChannel->PublicName( szPublicName );

				SendToOne( pMachine, IRCReplyString( RPL_LIST ),
					m_szHost, pMachine->m_szNick, szPublicName,
					pChannel->m_ChanMembers.GetCount(), pChannel->m_szTopic );
			}
		}
	}
	else
	{
		// Just send this one if possible
		CIRCChannel* pChannel = HuntChannel( szChName, NULL, NULL );
		if ( pChannel )
		{
			if ( pChannel->CanList() )
			{
				char szPublicName[CHANNELLEN];
				pChannel->PublicName( szPublicName );

				SendToOne( pMachine, IRCReplyString( RPL_LIST ),
					m_szHost, pMachine->m_szNick, szPublicName,
					pChannel->m_ChanMembers.GetCount(), pChannel->m_szTopic);
			}
		}
	}
}

void CIRCLocalServer::Send_RPL_LISTEND( CIRCMachine *pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	SendToOne( pMachine, IRCReplyString( RPL_LISTEND ), m_szHost, pMachine->m_szNick);
}
void CIRCLocalServer::Send_VERSION( CIRCMachine *pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	TCHAR * tVersionOut = Str_GetTemp();
	sprintf(tVersionOut, g_szServerDescription, (LPCTSTR)g_Cfg.m_sDisguiseName.GetPtr(), (LPCTSTR)g_Cfg.m_sDisguiseVersion.GetPtr());
	
	SendToOne( pMachine, "%s :Game Server: %s\r\n", CIRCTools::sm_CmdTbl[IRC_INFO], (LPCTSTR)tVersionOut );
	SendToOne( pMachine, "%s :%s\r\n", CIRCTools::sm_CmdTbl[IRC_INFO], m_szVersion );
}

void CIRCLocalServer::AddServer( CIRCServer *pServer )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pServer );
	if ( pUplink )
	{
		m_UplinkServers .Add ( pUplink );
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pServer );
	if ( pDownlink )
	{
		m_DownlinkServers .Add ( pDownlink );
		return;
	}

}

void CIRCLocalServer::Send_RPL_TOPIC( CIRCMachine *pMachine, CIRCChannel* pChannel )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	// Send out the appropriate channel topic message
	if ( strlen( pChannel->m_szTopic ) > 0 )
	{
		SendToOne( pLClient, IRCReplyString( RPL_TOPIC ),
			GetHost(), pLClient->m_szNick, pChannel->m_szChName, pChannel->m_szTopic );
		SendToOne( pLClient, IRCReplyString( RPL_TOPICWHOTIME ),
			GetHost(), pLClient->m_szNick, pChannel->m_szChName, pChannel->m_szTopicNick, pChannel->m_tTopicTime );
	}
	else
	{
		SendToOne( pLClient, IRCReplyString( RPL_NOTOPIC ),
			GetHost(), pLClient->m_szNick, pChannel->m_szChName );
	}
}

bool CIRCLocalServer::IsMyClient( CIRCMachine* pMachine )
{
	// Check all the lists of clients connected to us
	for( int i = 0; i < m_LClients.GetCount(); i++ )
	{
		if ( pMachine == m_LClients[i] )
			return true;
	}
	return false;
}

void CIRCLocalServer::JoinChannel( char* szChName, char* szKey, char* szNick, CIRCServer* pServer )
{
	time_t tNow;
	time( &tNow );

	time_t tTSTime = tNow + m_TSoffset;

	// Do we tell other servers about this?
	bool fSendCreate = true;
	bool fMakeChOp = true;
	// Let's try to join a channel
	CleanChannelName( szChName );

	// Did this come from a local client or a server?
	CIRCNick* pNick = FindNick( szNick );
	CIRCLRClient* pLClient = pNick->m_pLRClient;

	// Channel names beginning with '&' are local to this server only
	if ( szChName[0] != '@' && !IsMyClient( pLClient ) )
		return;

	// Joining channel 0 means leave all channels
	if ( szChName[0] == '0' && szChName[0] == '\0' )
	{
		// Part all channels
		for( int iChannel = 0; iChannel < pLClient->m_Channels.GetCount(); iChannel++ )
			PartChannel( pLClient, m_Channels[iChannel]->m_szChName, "Left all channels" );
		return;
	}

	if ( !IsChannelName( szChName ) )
	{
		if ( IsMyClient( pLClient ) )
		{
			SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ),
				GetHost(), pLClient->m_szNick, szChName );
			return;
		}
	}
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( IsMyClient( pLClient ) )
	{
		//
		// Local client is first to enter previously nonexistant
		// channel so make them (rightfully) the Channel Operator.
		// This looks kind of ugly because we try to avoid calling the strlen()
		//

		// I know this next section of code looks weird, but it usually avoids
		// calling strlen (slooooow)
		if ( pChannel )
		{
			fMakeChOp = false;
			fSendCreate = false;
		}
		else if ( strlen( szChName ) > CHANNELLEN)
		{
			*(szChName + CHANNELLEN) = '\0';
			if ( pChannel )
			{
				fMakeChOp = false;
				fSendCreate = false;
			}
			else
			{
				fMakeChOp = IsModelessChannel( szChName) ? false : true;
				fSendCreate = true;
			}
		}
		else
		{
			fMakeChOp = IsModelessChannel( szChName ) ? false : true;
			fSendCreate = true;
		}
		if ( pLClient->m_Channels.GetCount() >= MAXCHANNELSPERUSER )
		{
			SendToOne( pLClient, IRCErrorString( ERR_TOOMANYCHANNELS ),
				GetHost(), pLClient->m_szNick, szChName );
			return;
		}
	}
	pChannel = HuntChannel( szChName, szKey, pServer );
	if ( !pChannel )
		pChannel = HuntChannel( szChName, szKey, pLClient );
	if ( pChannel )
	{
		if ( !pChannel->m_tCreationTime )	// Did a remote JOIN create this channel?
			pChannel->m_tCreationTime = MAGIC_REMOTE_JOIN_TS;
		// See if they can join
		if ( IsMyClient( pLClient ) )
		{
			// Is the client flooding the local server?
			int iDelay = OverTargetLimit( pLClient );
			if ( iDelay > 0 )
			{
				SendToOne( pLClient, IRCErrorString( ERR_TARGETTOOFAST ),
					GetHost(), GetName(), szChName, iDelay );
				return;
			}
		}

		if ( !pChannel->CanJoin( pLClient, szKey ) )
			return;

		// Send user join to the local clients (if any)
		SendToChannel( pChannel, pLClient, NULL, ":%s JOIN :%s\r\n",
			pLClient->GetNickUserIP(), szChName );

		// Put them in the members list
		CIRCChannelMember* pChanMem = new CIRCChannelMember();
		pChanMem->m_pLRClient = pLClient;
		pChannel->m_ChanMembers.Add( pChanMem );
		// Add this channel to their list
		pLClient->m_Channels.Add( pChannel );
		// They going to be an op here?
		if ( fMakeChOp )
			pChanMem->m_dwMode |= ONCHMODE_CHANOP;
		// Join the channel
		pChannel->ClientJoin( pLClient );

		if ( !IsLocalChannel( szChName ) )
		{
			// If it's local, no need to do this
			if ( fSendCreate )
			{
				// CREATE
				SendToServers( NULL, "%s CREATE %s %lu\r\n",
					m_szNick, szChName, tTSTime );
			}
			else
			{
				// JOIN
				if ( strlen( szKey ) > 0 )
				{
					SendToServers( NULL, ":%s JOIN %s %s\r\n",
						pLClient->m_szNick, szChName, szKey );
				}
				else
				{
					SendToServers( NULL, ":%s JOIN %s\r\n",
						m_szNick, szChName );
				}
			}
		}
		// Check if it needs a time stamp
		if ( pChannel && pChannel->m_Mode.m_dwMode & CHMODE_SENDTS )
		{
			// send a TS?
			SendToServers( NULL, ":%s MODE %s + %lu\r\n",
				m_szNick, szChName, pChannel->m_tCreationTime );
			// reset flag
			pChannel->m_Mode.m_dwMode &= ~CHMODE_SENDTS;
		}
	}
	return;
}

void CIRCLocalServer::Do_CREATE( CIRCServer* pServer, char* szNick, char* szChName, time_t tChanTS )
{
	// Either we're sending it to other servers, or we received it from another server
	time_t tNow;
	time( &tNow );

	time_t tTSTime = tNow + m_TSoffset;

	int iAcceptChOp = 0;	// Accept channel op by default

	CIRCLRClient* pLClient;
	CIRCNick* pNick = FindNick( szNick );
	if ( pNick )
		pLClient = pNick->m_pLRClient;

	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( pChannel )
	{
		if ( tTSTime - tChanTS > TS_LAG_TIME)
		{
			// A bounce would not be accepted anyway - if we get here something
			// is wrong with the TS clock syncing (or we have more then
			// TS_LAG_TIME lag, or an admin is hacking
			iAcceptChOp = 2;
			// This causes a HACK notice on all upstream servers from the sender onward
			SendToOne( pServer, ":%s MODE %s -o %s 0\r\n", GetHost(), szChName, pServer->m_szNick );
			// This causes a WALLOPS on all downstream servers and a notice to our
			// own opers
			Send_HackNotice( pServer, iAcceptChOp, 2);
		}
		else if ( pChannel->m_tCreationTime && tChanTS > pChannel->m_tCreationTime &&
				pChannel->m_tCreationTime != MAGIC_REMOTE_JOIN_TS)
		{
			// We (try) to bounce the mode, because the CREATE is used on an older
			// channel, probably a net.ride
			iAcceptChOp = 1;
			// Send a deop upstream:
			SendToOne( pServer, ":%s MODE %s -o %s %lu\r\n",
				GetHost(), szChName, pServer->m_szNick, pChannel->m_tCreationTime );
		}
	}
	else
		// Channel doesn't exist: create it
		pChannel = HuntChannel( szChName, "", pServer );

	// Put them in the members list
	CIRCChannelMember* pChanMem = new CIRCChannelMember();
	pChanMem->m_pLRClient = pLClient;
	pChannel->m_ChanMembers.Add( pChanMem );

	// Add this channel to their list
	pLClient->m_Channels.Add( pChannel );

	// Add and mark ops
	if ( iAcceptChOp && !IsModelessChannel( szChName ) )
	{
		// Join the channel
		if ( iAcceptChOp )
			pChanMem->m_dwMode |= ONCHMODE_CHANOP;
	}

	// Send user join to the local clients (if any)
	SendToChannel( pChannel, pLClient, NULL, ":%s JOIN :%s\r\n",
		pLClient->GetNickUserIP(), szChName );

	if ( !iAcceptChOp )
	{
		// :Nix|Guy JOIN #MyChannel 964925561
		// handle badop: convert CREATE into JOIN
		SendToServers( pServer, ":%s JOIN %s %lu\r\n",
			pLClient->m_szNick, szChName, pChannel->m_tCreationTime );
	}
	else
	{
		// Send the op to all the servers:
		// (if any; extremely unlikely, but it CAN happen)
		// :irc2.sphereserver.com MODE #Channel +o Nix|Guy
		if ( !IsModelessChannel( szChName ) )
		{
			SendToServers( pServer, ":%s MODE %s +o %s\r\n",
				pServer->m_szHost, szChName, pLClient->GetNickUserIP() );
		}

		// Set/correct TS and add the channel to the
		// buffer for accepted channels:
		pChannel->m_tCreationTime = tChanTS;
	}

	if ( pChannel )	// Any channel accepted with ops?
	{
		// CAA CREATE #Channel3 964925152
		SendToServers( pServer, "%s CREATE %s %lu\r\n",
			pServer->m_szNick, szChName, pChannel->m_tCreationTime );
	}

	return;
}

void CIRCLocalServer::Send_HackNotice( CIRCServer* pServer, int iBadOp, int iType )
{
	// Send this to all servers except the pServer and all local opers
}

CIRCChannel* CIRCLocalServer::CreateNewChannel( CIRCMachine* pMachine, char* szChName, char* szKey )
{
	// Assume it doesn't exist
	if ( !pMachine )
		return NULL;
	// Create it
	CIRCChannel* pChannel = new CIRCChannel();
	// Name it
	strcpy( pChannel->m_szChName, szChName );
	// Give it key
	strcpy( pChannel->m_Mode.m_szKey, szKey );
	// Does pMachine "own" this? give it ops?
//	pChannel->m_pOwner = pMachine;
	// If it's a remotely created channel (from a server), clear it's creation time
	CIRCServer* pServer = dynamic_cast <CIRCServer*>( pMachine );
	if ( pServer )
		pChannel->m_tCreationTime = 0;
	// Add to the list of channels
	m_Channels.Add( pChannel );
	return pChannel;
}

int CIRCLocalServer::OverTargetLimit( CIRCLRClient* pLClient )
{
	// Return some delay based on how many targets they have
	// TODO: Actually make this work somehow
	if ( pLClient && pLClient->m_iTargets >= MAXTARGETS )
	{
		return pLClient->m_iTargets * 10;
	}
	return 0;
}

void CIRCLocalServer::DestroyChannel( CIRCChannel* pChannel )
{
	for(int i = 0; i < m_Channels.GetCount(); i++)
	{
		if ( pChannel == m_Channels[i] )
		{
			m_Channels.RemoveAt(i);
			// TODO: Tell the other servers this channel is gone
		}
	}
}

CIRCNick* CIRCLocalServer::FindNick( char* szNick )
{
	for(int i = 0; i < m_Nicks.GetCount(); i++)
	{
		if ( !strcmp( szNick, m_Nicks[i]->m_szNick ) )
			return m_Nicks[i];
	}
	return NULL;
}

void CIRCLocalServer::PartChannel(CIRCLRClient* pLClient, char* szChName, char* szReason)
{
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( pChannel )
	{
		pChannel->ClientPart( pLClient );
		if ( pChannel->m_ChanMembers.GetCount() == 0 )
			DestroyChannel( pChannel );
		else
		{
			// If there's a reason, then the client quit
			// Tell the other clients this one left
			char buffer[NICKLEN + USERLEN + 16 + 3];
			strcpy(buffer, pLClient->GetNickUserIP() );
			if ( strlen( szReason ) > 0 )
				SendToChannel( pChannel, NULL, NULL, ":%s QUIT :%s\r\n", buffer, szReason );
			else
				SendToChannel( pChannel, NULL, NULL, ":%s PART :%s\r\n", buffer, pChannel->m_szChName );
			return;
		}
	}
	else
	{
		SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ), GetHost(), pLClient->m_szNick, szChName );
	}
	return;
}

void CIRCLocalServer::NickAudit()
{
	time_t tNow;
	time( &tNow );

	if ( m_tNicksTimer + NICKAUDITTIMER < tNow )
	{
		// Time to audit (audit backwards through time)
		for( int i = m_Nicks.GetCount()-1; i >= 0; i-- )
		{
			if ( m_Nicks[i]->CanDelete() )
			{
				m_Nicks[i]->ClearHistory();
				m_Nicks.RemoveAt( i );
			}
		}
		// Reset the timer
		time( &m_tNicksTimer );
	}
}

void CIRCLocalServer::KickFromChannel( CIRCLRClient* pLClient, char* szChName, char* szNick, char* szComment )
{
	// Parameter check
	if ( strlen( szChName ) == 0 || strlen( szNick ) == 0 )
	{
		SendToOne( pLClient, IRCErrorString( ERR_NEEDMOREPARAMS ), GetHost() );
		return;
	}
	// Is the victim real? (chase for them!)
	CIRCLRClient* pVictimClient = ChaseClientByNick( szNick );
	if ( !pVictimClient )
	{
		SendToOne( pLClient, IRCErrorString( ERR_NOSUCHNICK ), GetHost(), szNick );
		return;
	}

	// Does this channel exist?
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );

	if ( !pChannel )
	{
		SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ), GetHost(), pLClient->m_szNick, szChName );
		return;
	}
	// Ok, we have the channel, pointers to both local clients,
	// and the comment, pass this kick command to the channel to process
	pChannel->ClientKickingClient( pLClient, pVictimClient, szComment );
	return;
}

CIRCMachine* CIRCLocalServer::FindMachine( CClient* pClient )
{
	for( int i = 0; i < m_Machines.GetCount(); i++ )
	{
		if ( pClient == m_Machines[i]->GetClient() )
			return m_Machines[i];
	}
	return NULL;
}

CIRCLRClient* CIRCLocalServer::FindLClient( CClient* pClient )
{
	for( int i = 0; i < m_LClients.GetCount(); i++ )
	{
		if ( pClient == m_LClients[i]->GetClient() )
			return m_LClients[i];
	}
	return NULL;
}

void CIRCLocalServer::Send_NICK( CIRCMachine *pMachine, char* szNick )
{
	// Only authenticated machines can get here

	// Server's will send this one when one of their local client's changes their nick
	// Handle external nick collisions in here
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	// Can they have this nick?
	CIRCNick* pNick = FindNick( szNick );

	if ( pNick )
	{
		if ( pNick->IsOwned() )
		{
			// We already have this nick listed, is it us?
			if ( strcmp(pLClient->m_szNick, szNick ) )
			{
				// This is a local collision, tell them either
				// they can't have this one or to use a backup nick
				if ( pLClient->IsAuthenticated() )
				{
					SendToOne( pLClient, IRCErrorString( ERR_NICKNAMEINUSE ),
						GetHost(), pLClient->m_szNick, szNick );
					return;
				}
				else
				{
					// This condition happens during login and so this shouldn't happen here
					return;
				}
			}
			else
			{
				// It's our's already, eat it (that's what undernet does)
				return;
			}
			// Fall through
		}
	}

	// Save their nick depending
	char szOldNick[NICKLEN];

	// They're changing their existing nick
	strcpy( szOldNick, pLClient->m_szNick );
	// Update the old nick's history
	CIRCNick* pOldNick = FindNick( szOldNick );

	if ( pOldNick )
		pOldNick->ClearOwner();

	// Give them the new nick
	strcpy( pLClient->m_szNick, szNick );

	// Either create a new nick, or update the existing one
	if ( pNick )
	{
		// This nick is already a listed nick
		// See if they are in the history, if they are, re-use it
		// If not, make a new record
		pNick->NewOwner( pLClient );
	}
	else
	{
		// Create a whole new one
		CIRCNick *pNewNick = new CIRCNick();
		m_Nicks.Add( pNewNick );
		strcpy(pNewNick->m_szNick, szNick);
		pNewNick->NewOwner( pLClient );
	}

	char buffer[NICKLEN + USERLEN + 16 + 3];
	strcpy(buffer,
		pLClient->GetNickUserIP( szOldNick ) );
	// If they are changing their nick, as opposed to being a new client, do this
	// example: :Westy!noneya@10.1.1.1 NICK :WestyAgai

	// First send it back to them
	SendToOne( pMachine, ":%s NICK :%s\r\n", buffer, szNick);

	// They might be on some local channels
	for(int i = 0; i < pLClient->m_Channels.GetCount(); i++)
		( pLClient->m_Channels[i], m_szSendBuffer, pLClient );

	// TODO: Tell the other servers about this

}

void CIRCLocalServer::Send_LUSERS(CIRCMachine *pMachine)
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	SendToOne( pMachine, IRCReplyString( RPL_LUSERCLIENT ),
		m_szHost, pLClient->m_szUser, GetTotalClients(), GetTotalInvisibleClients(),
		m_DownlinkServers.GetCount() + m_UplinkServers.GetCount() + 1);

	int iUsers = m_LClients.GetCount();
	SendToOne( pMachine, IRCReplyString( RPL_LUSERME ),
		m_szHost, pLClient->m_szUser, iUsers,
		m_DownlinkServers.GetCount() + m_UplinkServers.GetCount() );

	SendToOne( pMachine, ":%s NOTICE %s :Highest connection count: 1 (1 clients)\r\n",
		m_szHost, pLClient->m_szUser);
	return;
}

void CIRCLocalServer::Send_MOTD(CIRCMachine *pMachine)
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	SendToOne( pMachine, IRCReplyString( RPL_MOTDSTART ), m_szHost, pMachine->m_szUser, "SphereServer");

	SendToOne( pMachine, IRCReplyString( RPL_MOTD ), m_szHost, pMachine->m_szUser, "Message of the Day");

	SendToOne( pMachine, IRCReplyString( RPL_MOTD ), m_szHost, pMachine->m_szUser, "17/7/2000 19:22");

	SendToOne( pMachine, IRCReplyString( RPL_MOTD ), m_szHost, pMachine->m_szUser, "This is an example MOTD (i hope)");

	SendToOne( pMachine, IRCReplyString( RPL_MOTD ), m_szHost, pMachine->m_szUser, "");

	SendToOne( pMachine, IRCReplyString( RPL_ENDOFMOTD ), m_szHost, pMachine->m_szUser);

	return;
}

void CIRCLocalServer::Send_PING( CIRCMachine *pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	pLClient->m_dwCookie = (CIRCTools::ircrandom() & 0x7fffffff);
	SendToOne( pMachine, "%s :%li\r\n", CIRCTools::sm_CmdTbl[IRC_PING], (DWORD)(pLClient->m_dwCookie));
}

void CIRCLocalServer::Send_RPL_BANLIST( CIRCMachine *pMachine, char* szChName )
{
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( pChannel )
	{
		for( int i = 0; i < pChannel->m_Banned.GetCount(); i++)
		{
			CIRCMask* pMask = pChannel->m_Banned[i];
			SendToOne( pMachine, IRCReplyString( RPL_BANLIST ),
				m_szHost, pMachine->m_szNick, pChannel->m_szChName,
				pMask->m_szNick, pMask->m_szMask, pMask->m_tTime );
		}
	}
}

void CIRCLocalServer::Send_RPL_ENDOFBANLIST( CIRCMachine *pMachine, char* szChName )
{
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( pChannel )
	{
		SendToOne( pMachine, IRCReplyString( RPL_ENDOFBANLIST ),
			m_szHost, pMachine->m_szNick, pChannel->m_szChName );
	}
}

void CIRCLocalServer::Send_RPL_LISTSTART( CIRCMachine *pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	SendToOne( pMachine, IRCReplyString( RPL_LISTSTART ),
		m_szHost, pMachine->m_szNick);
}

bool CIRCLocalServer::SocketsInit() // Initialize sockets
{
	if ( ! m_Socket.Create())
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Unable to create socket\n");
		return( false );
	}

	linger lval;
	lval.l_onoff = 0;
	lval.l_linger = 10;
	int iRet = m_Socket.SetSockOpt( SO_LINGER, (const char*) &lval, sizeof(lval));
	if ( iRet )
	{
		DEBUG_ERR(( "setsockopt linger FAIL\n" ));
	}
#ifdef _WIN32
	DWORD lVal = 1;	// 0 =  block
	iRet = m_Socket.IOCtlSocket( FIONBIO, &lVal );
	DEBUG_CHECK( iRet==0 );
#endif

	//BOOL fon=1;
	//iRet = SetSockOpt( SO_REUSEADDR, &fon, sizeof(fon));

	int bcode = m_Socket.Bind( m_wPort );
	if (bcode<0)
	{
		// Probably already a server running.
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Unable to bind listen socket port %d - Error code: %i\n", m_wPort, bcode );
		return( false );
	}

	// Max number we can deal with. compile time thing.
	if ( g_Cfg.m_iClientsMax > FD_SETSIZE-1 )
		g_Cfg.m_iClientsMax = FD_SETSIZE-1;

	m_Socket.Listen();

	// What are we listing our port as to the world.
	// Tell the admin what we know.

	TCHAR szName[ _MAX_PATH ];
	struct hostent * pHost = NULL;
	iRet = gethostname( szName, sizeof( szName ));
	if ( iRet )
	{
		strcpy( szName, g_Serv.m_ip.GetAddrStr());
	}
	else
	{
		pHost = gethostbyname( szName );
		if ( pHost != NULL &&
			pHost->h_addr != NULL &&
			pHost->h_name &&
			pHost->h_name[0] )
		{
			strcpy( szName, pHost->h_name );
		}
	}

	strcpy(m_szHost, szName );
	g_Log.Event( LOGM_INIT, "IRC Server started on '%s' port %d.\n", szName, m_wPort );

	if ( ! iRet )
	{
		if ( pHost == NULL || pHost->h_addr == NULL )	// can't resolve the address.
		{
			g_Log.Event( LOGL_CRIT|LOGM_INIT, "gethostbyname does not resolve the address.\n" );
		}
		else
		{
			for ( int j=0; pHost->h_aliases[j]; j++ )
			{
				g_Log.Event( LOGM_INIT, "Alias '%s'.\n", (LPCTSTR) pHost->h_aliases[j] );
			}
			// h_addrtype == 2
			// h_length = 4
			for ( int i=0; pHost->h_addr_list[i] != NULL; i++ )
			{
				CSocketAddressIP ip;
				ip.SetAddrIP( *((DWORD*)( pHost->h_addr_list[i] ))); // 0.1.2.3
				g_Log.Event( LOGM_INIT, "Monitoring IP '%s'.\n", (LPCTSTR) ip.GetAddrStr() );
			}
		}
	}

	return( true );
}

bool CIRCLocalServer::UplinksInit()
{
	// NOTE: This is a synchronous function that could take a while.

	for( int iCNLine = 0; iCNLine < m_CNLines.GetCount(); iCNLine++ )
	{
		// Do we try to connect to it? or does it connect to us?
		if ( !m_CNLines[iCNLine]->m_bAutoConnect )
			continue;

		CIRCCNLine* pCNLine = m_CNLines[iCNLine];
		// Make a new uplink server for this
		CIRCUplinkServer* pUplink = new CIRCUplinkServer;
		// Copy all the info over to it
		strcpy( pUplink->m_szPassword, pCNLine->m_szPassword );
		strcpy( pUplink->m_szServerName, pCNLine->m_szServerName );
		strcpy( pUplink->m_szHost, pCNLine->m_szHost );
		pUplink->m_iPort = pCNLine->m_wPort;
		pUplink->m_iaClientIP = pCNLine->m_iaClientIP;

		// look up this ip.
		CSocketAddress RegisterIP( pUplink->m_iaClientIP, pUplink->m_iPort );

		if ( ! pUplink->m_Socket.Create())
		{
			m_BadUplinkServers.Add( pUplink );
			continue;
		}

		if ( pUplink->m_Socket.Connect( RegisterIP ) )
		{
			m_BadUplinkServers.Add( pUplink );
			DEBUG_ERR(( "%d, can't connect to IRC uplink '%s'\n", CGSocket::GetLastError(), pUplink->m_szServerName ) );
			continue;
		}

		// ? need to wait a certain amount of time for the connection to setle out here.

		// got a connection. Send reg command.
		time_t tNow;
		time( &tNow );
		char szTempBuffer[BUFSIZE];
		int iHopCount = 1;
		memset( szTempBuffer, 0, BUFSIZE );	// special registration header.
		sprintf( szTempBuffer, "PASS :%s\r\nSERVER %s %i %lu %lu %s %s :%s\r\n",
			pUplink->m_szPassword, m_szName, iHopCount,
			m_tOnlineSince, tNow, m_szProtocol, m_szNick, m_szDescription );
		int iLen = strcpylen( &m_szSendBuffer[0], szTempBuffer, sizeof( szTempBuffer ) );
		int iLenRet = pUplink->m_Socket.Send( m_szSendBuffer, iLen );
		if ( iLenRet != iLen )
		{
			// WSAENOTSOCK
			m_BadUplinkServers.Add( pUplink );

			DEBUG_ERR(( "%d, can't send to IRC uplink '%s'\n", pUplink->m_Socket.GetLastError(), pUplink->m_szServerName ) );
			continue;
		}
		g_Log.Event( LOGM_INIT, "IRC uplink authenticating to a server named %s.\n", pUplink->m_szServerName );
		m_UplinkServers.Add( pUplink );

	}
	return( true );
}

int CIRCLocalServer::GetClientPing( CIRCMachine* pMachine )
{
	return PINGFREQUENCY;
}

void CIRCLocalServer::CheckPings()
{
	time_t tNow;
	time( &tNow );
	int iPing;

	for( int i = 0; i < m_LClients.GetCount(); i++ )
	{
		CIRCLRClient* pLClient = m_LClients[i];
		if ( !pLClient->IsAuthenticated() )
			continue;
		iPing = GetClientPing( pLClient );
		if ( pLClient->m_dwFlags & FLAGS_PINGSENT )
		{
			// This one might be dead
			if ( tNow - pLClient->m_tLastTime > iPing )
			{
				// Set it to be killed here
				pLClient->m_dwFlags |= FLAGS_DEADSOCKET;
			}
		}
		else
		{
			// If we havent PINGed the connection and we havent
			// heard from it in a while, PING it to make sure
			// it is still alive.
			if ( tNow - pLClient->m_tLastTime > iPing )
			{
				pLClient->m_dwFlags |= FLAGS_PINGSENT;
				pLClient->m_tLastTime = tNow;
				SendToOne( pLClient, "PING :%s\r\n", GetHost() );
			}
		}
	}
}

void CIRCLocalServer::ExitDeadSockets()
{
	for( int i = m_LClients.GetCount() - 1; i >=0; i-- )
	{
		CIRCLRClient* pLClient = m_LClients[i];
		if ( IsDead( pLClient ) )
		{
			ExitClient( pLClient, "Client timed out waiting for ping response." );
			continue;
		}
	}
	return;
}

void CIRCLocalServer::ExitClient( CIRCLRClient* pLClient, char* szReason )
{
	// Save this so we can tell the game server to let them go
	CClient* pGameClient = pLClient->GetClient();
	// First clean them out of our channels and whatnot
	ClientQuit( pLClient, szReason );
	// CClient's destructor will call back to CIRCLocalServer if
	// the targmode is set for an IRC client, i guess we should change it
	pGameClient->SetConnectType( CONNECT_NONE );
	delete pLClient;
}

void CIRCLocalServer::OnTick()
{
	g_Serv.m_Profile.Start( PROFILE_IRC );

	NickAudit();
	PollServers();
	ExitDeadSockets();
	CheckPings();

	g_Serv.m_Profile.Start( PROFILE_OVERHEAD );

	// TODO: Somehow background connect to uplinks which failed on start up
}

void CIRCLocalServer::PollServers() // Check for messages from the uplink servers
{
	// What sockets do I want to look at ?
	fd_set readfds;
	FD_ZERO( &readfds );

	int nfds = m_Socket.GetSocket();
	FD_SET( nfds, &readfds );

	int i;
	for( i = 0; i < m_UplinkServers.GetCount(); i++ )
	{
		SOCKET hSocket = m_UplinkServers[i]->m_Socket.GetSocket();
		FD_SET( hSocket,&readfds );
		if ( hSocket > nfds )
			nfds = hSocket;
	}

	// we task sleep in here. NOTE: this is where we give time back to the OS.
	g_Serv.m_Profile.Start( PROFILE_IDLE );

	timeval Timeout;	// time to wait for data.
	Timeout.tv_sec=0;
	Timeout.tv_usec=100;	// micro seconds = 1/1000000
	int ret = select( nfds+1, &readfds, NULL, NULL, &Timeout );
	g_Serv.m_Profile.Start( PROFILE_IRC );

	if ( ret <= 0 )
		return;

	for( i = m_UplinkServers.GetCount() - 1; i >= 0; i-- )
	{
		CIRCUplinkServer* pUplink = m_UplinkServers[i];
		if ( FD_ISSET( pUplink->m_Socket.GetSocket(), &readfds ))
		{
			pUplink->m_timeLastEvent = CServTime::GetCurrentTime();	// We will always get pinged every couple minutes or so
			if ( !xRecvData( pUplink ))
			{
				m_UplinkServers.RemoveAt( i );
				delete pUplink;
				continue;
			}
		}
		else
		{
			if ( g_Cfg.m_iDeadSocketTime &&
				-g_World.GetTimeDiff( pUplink->m_timeLastEvent ) > g_Cfg.m_iDeadSocketTime )
			{
				// We have not talked in several minutes.
				DEBUG_ERR( ( "%x:Dead IRC Uplink Socket Timeout\n", pUplink->m_Socket.GetSocket() ) );
				m_UplinkServers.RemoveAt( i );
				delete pUplink;
				continue;
			}
		}
	}

	// Any new connections ? what if there are several ???
	if ( FD_ISSET( m_Socket.GetSocket(), &readfds) )
	{
		// This shouldn't happen in here right?
	}

}

bool CIRCLocalServer::xRecvData( CIRCUplinkServer *pUplink ) // Receive message from client
{
	// High level Rx from uplink.
	// RETURN: false = dump the client.

	int iPrev = m_bin_len;
	int count = pUplink->m_Socket.Receive( &( m_Raw[iPrev] ), BUFSIZE - iPrev );
	if ( count <= 0 )	// I should always get data here.
	{
		return( false ); // this means that the client is gone.
	}
	if ( !OnUplinkRequest( pUplink, &m_Raw[0], count ) )
	{
		g_Log.Event( LOGM_INIT, "Socket to IRC server %s has been closed.\n", pUplink->m_szServerName );
		m_bin_len = 0;	// eat the buffer.
		return false;
	}

	m_bin_len = 0;	// eat the buffer.

	return( true );
}

#pragma warning( disable : 4244 4800 )
bool CIRCLocalServer::ConfInit()
{
	CScript s;
	if ( ! g_Cfg.OpenResourceFind( s, GRAY_FILE "IRC" ))
	{
		g_Log.Event( LOGL_FATAL|LOGM_INIT, "Can't open irc def file '%s'\n", (LPCTSTR) s.GetFilePath() );
		return false;
	}

	static LPCTSTR const sm_ConfigSectionTable[] =
	{
		"IRC_LOCALSERVER",
		"IRC_CNLINE",
		"IRC_MOTD",
		"IRC_BAN",
		"IRC_KLINE",
		"IRC_GLINE",
	};
	static LPCTSTR const sm_LocalServerTable[] =
	{
		"DESCRIPTION",
		"PORT",
		"NICK",
		"ACCOUNT_SECURITY",
		"ADMINLINE",
		"ADMIN_USER",
		"ADMIN_IP_OR_HOST_MASK",
		"ADMIN_PASSWORD",
		"ADMIN_HOST_MASK",
	};

	while (s.FindNextSection())
	{
		int iSection = FindTable( s.GetKey(), sm_ConfigSectionTable, COUNTOF( sm_ConfigSectionTable ) );

		switch ( iSection )
		{
			case 0:		// LOCALSERVER
			{
				strcpy( m_szName, s.GetArgStr() );

				while ( s.ReadKeyParse())
				{
					int iCase = FindTable( s.GetKey(), sm_LocalServerTable, COUNTOF( sm_LocalServerTable ) );
					switch ( iCase )
					{
						case 0:		// DESCRIPTION
							strcpy( m_szDescription, s.GetArgStr() );
							break;
						case 1:		// PORT
							m_wPort = s.GetArgVal();
							break;
						case 2:		// NICK
							strcpy( m_szNick, s.GetArgStr() );
							break;
						case 3:		// ACCOUNT_SECURITY
							m_fAccountSecurity = s.GetArgVal();
							break;
						case 4:		// ADMINLINE
						{
							CIRCAdminLine * pAdminLine = new CIRCAdminLine( s.GetArgStr() );
							m_AdminLines.Add( pAdminLine );
							break;
						}
						case 5:		// ADMIN_USER
							strcpy( m_AdminInfo.m_szUser, s.GetArgStr() );
							break;
						case 6:		// ADMIN_IP_OR_HOST_MASK
							strcpy( m_AdminInfo.m_szIPOrHostMask, s.GetArgStr() );
							break;
						case 7:		// ADMIN_PASSWORD
							strcpy( m_AdminInfo.m_szPassword, s.GetArgStr() );
							break;
						case 8:		// ADMIN_HOST_MASK
							strcpy( m_AdminInfo.m_szHostMask, s.GetArgStr() );
							break;
					}
				}
				break;
			}
			case 1:		// CNLINE
			{
				static LPCTSTR const sm_CNLinesTable[] =
				{
					"IP",
					"HOST_OR_IP",
					"PASSWORD",
					"PORT",
					"HOST_MASK",
					"AUTOCONECT",
				};

				CIRCCNLine* pCNLine = new CIRCCNLine();
				m_CNLines.Add( pCNLine );

				strcpy( pCNLine->m_szServerName, s.GetArgStr() );

				while ( s.ReadKeyParse())
				{
					int iCase = FindTable( s.GetKey(), sm_CNLinesTable, COUNTOF( sm_CNLinesTable ) );
					switch ( iCase )
					{
						case 0:		// IP
						{
							// inet_addr ?
							TCHAR szIP[32];
							strcpy( szIP, s.GetArgStr() );
							TCHAR * ppQuads[4];
							int iQty = Str_ParseCmds( szIP, ppQuads, COUNTOF(ppQuads), "." );
#ifdef _WIN32
							pCNLine->m_iaClientIP.S_un.S_un_b.s_b1 = atoi( ppQuads[0] );
							pCNLine->m_iaClientIP.S_un.S_un_b.s_b2 = atoi( ppQuads[1] );
							pCNLine->m_iaClientIP.S_un.S_un_b.s_b3 = atoi( ppQuads[2] );
							pCNLine->m_iaClientIP.S_un.S_un_b.s_b4 = atoi( ppQuads[3] );
#else
							pCNLine->m_iaClientIP.s_addr =
								atoi( ppQuads[0] ) |
								( atoi( ppQuads[1] ) << 8 ) |
								( atoi( ppQuads[2] ) << 16 ) |
								( atoi( ppQuads[3] ) << 24 );
#endif

							break;
						}
						case 1:		// HOST_OR_IP
							strcpy( pCNLine->m_szHost, s.GetArgStr() );
							break;
						case 2:		// PASSWORD
							strcpy( pCNLine->m_szPassword, s.GetArgStr() );
							break;
						case 3:		// PORT
							pCNLine->m_wPort = s.GetArgVal();
							break;
						case 4:		// HOST_MASK
							pCNLine->m_iMaskHost = s.GetArgVal();
							break;
						case 5:		// AUTOCONECT
							pCNLine->m_bAutoConnect = s.GetArgVal();
							break;
					}
				}
				break;
			}
			case 2:		// MOTD
			{
				static LPCTSTR const sm_MOTDTable[] =
				{
					"PATH",
				};

				CIRCMOTD* pMOTD = new CIRCMOTD();
				m_MOTDs.Add( pMOTD );

				strcpy( pMOTD->m_szHostMask, s.GetArgStr() );

				while ( s.ReadKeyParse())
				{
					int iCase = FindTable( s.GetKey(), sm_MOTDTable, COUNTOF( sm_MOTDTable ) );
					switch ( iCase )
					{
						case 0:		// PATH
							strcpy( pMOTD->m_szPath, s.GetArgStr() );
							break;
					}
				}
				break;
			}
			case 3:		// BAN
			case 4:		// KLINE
			case 5:		// GLINE
			{
				static LPCTSTR const sm_BKGTable[] =
				{
					"MASK",
					"REASON_PATH",
					"DURATION",
				};

				CIRCMask* pMask = new CIRCMask();

				switch ( iSection )
				{
					case 0:
						m_Bans.Add( pMask );
					case 1:
						m_KLines.Add( pMask );
					case 2:
						m_GLines.Add( pMask );
				}

				strcpy( pMask->m_szNick, s.GetArgStr() );

				while ( s.ReadKeyParse())
				{
					int iCase = FindTable( s.GetKey(), sm_BKGTable, COUNTOF( sm_BKGTable ) );
					switch ( iCase )
					{
						case 0:		// MASK
							strcpy( pMask->m_szMask, s.GetArgStr() );
							break;
						case 1:		// REASON_PATH
							strcpy( pMask->m_szPath, s.GetArgStr() );
							break;
						case 2:		// DURATION
							pMask->m_tTime = s.GetArgVal();
							break;
					}
				}
				break;
			}
		}
	}

	return true;
}
#pragma warning( default : 4244 4800 )

bool CIRCLocalServer::Init()
{
	if ( ! ConfInit() )
		return false;
	if ( ! SocketsInit() )
	{
		g_Log.Event( LOGM_INIT, "Error in initializing IRC Server sockets.\n" );
		return false;
	}
	if ( ! UplinksInit() )
	{
		// Go open up links to all the IRC servers
		// we're supposed to connect to
		g_Log.Event( LOGM_INIT, "One or more IRC uplinks were not opened.\n" );
	}
	return true;
}

CIRCServer* CIRCLocalServer::FindServerHost( char* szServerHost )
{
	int i;
	for( i = 0; i < m_DownlinkServers.GetCount(); i++ )
	{
		if ( !strcmp( m_DownlinkServers[i]->m_szHost, szServerHost ) )
			return m_DownlinkServers[i];
	}
	for( i = 0; i < m_UplinkServers.GetCount(); i++ )
	{
		if ( !strcmp( m_UplinkServers[i]->m_szHost, szServerHost ) )
			return m_UplinkServers[i];
	}
	return NULL;
}

CIRCServerNick* CIRCLocalServer::FindServerNick( char* szServerNick )
{
	for( int i = 0; i < m_ServerNicks.GetCount(); i++ )
	{
		// Undernet servers are identified by the first character only
		// ( it gives a maximum of 64 servers per network
		if ( strlen( m_ServerNicks[i]->m_szNick ) == 3 )
		{
			// The first character of the nick should match up to the first character
			// of a nick in our list of servers...if it does, then that's where this
			// message originally came from
			// I guess the other 2 are an index into that server's list of nicks
			if ( m_ServerNicks[i]->m_szNick[0] == szServerNick[0] )
			{
				// This is probably an undernet server or one emulating one
				return m_ServerNicks[i];
			}
		}
		else
		{
			// A server with a long nick?
			if ( !strcmp( m_ServerNicks[i]->m_szNick, szServerNick ) )
				return m_ServerNicks[i];
		}
	}
	return NULL;
}

bool CIRCLocalServer::Do_NICK_New( CIRCServer* pServer, CIRCCmd* pCmd )
{
	CIRCLRClient* pRClient = NULL;

	// Create a new remote client
	pRClient = new CIRCLRClient();
	// These have a hop count and a server instead of a CClient
	pRClient->m_iHops = atoi( pCmd->m_szArgs[3] );
	// This is the supposed time they've been on line since
	pRClient->m_tLastNick = atoi( pCmd->m_szArgs[4] );
	// This is the user name they chose for themself
	strcpy( pRClient->m_szUser, pCmd->m_szArgs[5] );
	// Grab their or the originator server's IP address
	strcpy( pRClient->m_szHost, pCmd->m_szArgs[6] );

	char szNick[NICKLEN];
	CIRCServerNick* pServerNick;
	if ( pCmd->m_iQtyArgs == 9 )
	{
		// There's no MODE param
		// This is the IP address which the server resolved at the client's login
		// in BASE64 ascii representation
		pRClient->m_iaClientIP.s_addr = CIRCTools::Base64ToInt( pCmd->m_szArgs[7] );
		// This is the actual server they are logged into with their client i guess
		pServerNick = FindServerNick( pCmd->m_szArgs[8] );
	}
	else
	{
		// Here's the mode they are currently in
		// TODO: Make a nice MODE command parser (pCmd->m_szArgs[7])

		// This is the IP address which the server resolved at the client's login
		// in BASE64 ascii representation
		pRClient->m_iaClientIP.s_addr = CIRCTools::Base64ToInt( pCmd->m_szArgs[8] );
		pServerNick = FindServerNick( pCmd->m_szArgs[9] );
	}


	if ( pServerNick )
	{
		CIRCServer* pLoggedInServer = pServerNick->m_pServer;
		if ( pLoggedInServer )
		{
			pRClient->m_pServer = pLoggedInServer;
		}
		else
		{
			// TODO: Send an error message
			delete pRClient;
			return true;
		}
	}
	else
	{
		// TODO: Send an error message
		delete pRClient;
		return true;
	}
	// And their "real name"
	strcpy( pRClient->m_szRealName, pCmd->m_szTrailing );

	// Get the nick param from the correct place
	strcpy( szNick, pCmd->m_szArgs[2] );

	// Is it a valid looking nick? If it's too long, just fix it for them
	// although RFC 1459 states that we *should* reject it, but why?
	int iLen = AnalyzeNick( szNick );

	if ( iLen == 0 )
	{
		SendToOne( pServer, IRCErrorString( ERR_ERRONEUSNICKNAME ), GetHost(), szNick );
		return true;
	}

	// Check to see if this nick is already taken
	// First check for an existing nick
	CIRCNick *pNick = FindNick( szNick );
	if ( pNick )
	{
		// Now check to see if it's currently owned
		if ( pNick->IsOwned() )
		{
			// It is, send out nick in use message
			// example: :irc.sphereserver.com 433 * Revenge :Nickname is already in use.
			SendToOne( pServer, IRCErrorString( ERR_NICKNAMEINUSE ),
				GetHost(), szNick );
			return true;
		}
	}
	else
	{
		// Now create the nick tracking stuff
		pNick = new CIRCNick();
		m_Nicks.Add( pNick );
		strcpy( pNick->m_szNick, szNick );
	}
	// Save their nick and update the nick tracking record
	strcpy( pRClient->m_szNick, szNick );
	pNick->NewOwner( pRClient );

	// Assign the numnick on this server
	if ( pCmd->m_iQtyArgs == 9 )
		pServer->AssignNumNick( pNick, pCmd->m_szArgs[8] );
	else
		pServer->AssignNumNick( pNick, pCmd->m_szArgs[9] );

	// Add to the list
	m_RClients.Add( pRClient );
	return true;
}

bool CIRCLocalServer::Do_NICK_Change( CIRCServer* pServer, CIRCCmd* pCmd )
{
	// The sender of this message and the originator may be different
	// servers, the sender could be relaying this from else where

	// Get the originator of the message
	char szNumNick[3];
	strcpy( szNumNick, pCmd->m_szArgs[0] );
	CIRCServerNick* pServerNick = FindServerNick( szNumNick );
	if ( pServerNick )
	{
		// 4 args
		// CAA NICK Nick1b 965280656
		CIRCServer* pLoggedInServer = pServerNick->m_pServer;
		char szNick[NICKLEN];
		strcpy( szNick, pCmd->m_szArgs[2] );
		// Get the CIRCNumNick* to the client who is changing their nick
		CIRCNumNick* pNumNick = pLoggedInServer->GetNumNick( pCmd->m_szArgs[0] );
		CIRCLRClient* pRClient;
		if ( pNumNick->m_pNick )
		{
			pRClient = pNumNick->m_pNick->m_pLRClient;
		}
		else
		{
			// They don't have a slot

			// If the client ( [1] & [2] ) corresponding to the
			// prefix ( [0] ) is not found. We must ignore it,
			// it is simply a lagged message travelling
			// upstream before a SQUIT that removed the client

			// There turns out to be other reasons that
			// a prefix is unknown, needing an upstream
			// KILL. Also, next to a SQUIT we better
			// allow a KILL to pass too.
			//
			// ok, i guess this means we send a KILL back upstream
			// cuz this is a NICK message, but for SQUIT, DIE, KILL and KLINE
			// we should let it go??
			SendToOne( pServer, "%c KILL %s :%s (Unknown numeric nick)",
				 m_szNick[0], szNumNick[0], m_szName );
			return true;
		}
		// We're supposed to trust the remote server I guess
		// Check to see if we can re-use a history for this nick
		// First check for an existing nick
		CIRCNick *pNick = FindNick( szNick );
		if ( pNick )
		{
			// Now check to see if it's currently owned
			if ( pNick->IsOwned() )
			{
				// This shouldn't happen, is this a nick colision?
				// It is, send out nick in use message for now back to the sending server
				// example: :irc.sphereserver.com 433 * Revenge :Nickname is already in use.

				// TODO: Send a message to our local server ops telling them about this
				// sendto_ops("Nick change collision from %s to %s (%s %lu <- %s %lu)",
				// sptr->name, acptr->name, acptr->from->name,
				// acptr->lastnick, get_client_name(cptr, FALSE), lastnick);

				// Now remove (kill) the nick on our side if it is the youngest.
				// If no timestamp was received, we ignore the incoming nick
				// (and expect a KILL for our legit nick soon ):
				// If the timestamps are equal we kill both nicks.

				// sendto_highprot_butone(cptr, 10, /* Kill old from outgoing servers */
				// "%c KILL %c%c%c :%s (%s <- %s (Nick collision))",
				// NumServ(&me), NumNick(sptr), me.name, acptr->from->name,
				// get_client_name(cptr, FALSE));
				// if (MyConnect(sptr) && IsServer(cptr) && Protocol(cptr) > 9)
				// sendto_one(cptr, "%c KILL %c%c%c :%s (Ghost2)",
				// NumServ(&me), NumNick(sptr), me.name);
				return true;
			}
		}
		else
		{
			// No past history, create the new nick tracking stuff
			pNick = new CIRCNick();
			m_Nicks.Add( pNick );
			strcpy( pNick->m_szNick, szNick );
		}
		// Save their nick and update the nick tracking record
		pNick->NewOwner( pRClient );
		// New time stamp
		pRClient->m_tLastNick = atoi( pCmd->m_szArgs[3] );
		// Save their nick and update the nick tracking record
		strcpy( pRClient->m_szNick, szNick );
		pLoggedInServer->AssignNumNick( pNick, pCmd->m_szArgs[0] );
	}
	else
	{
		// This is an unknown server numnick to us
		// TODO: Send out some error back to the sending server
		return true;
	}
	return true;
}

bool CIRCLocalServer::Do_NICK_Server( CIRCServer* pServer, CIRCCmd* pCmd )
{
	// RFC 1459, 4.1.2 "Nick Message"
	// COMMAND: NICK <nickname> [ <hopcount> ]
	//
	// Both remote servers and local clients will send this to us
	// Remote servers will send it to introduce new users (so they'll
	// never get to this point in this part of the log on sequence)
	//
	// Servers will send us this (with a hopcount param)
	//	when they introduce a (possibly) new nick to the network
	// replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTERED,
	//			ERR_NICKCOLLISION
	//
	// C NICK Nix|Guy 1 964923610 ~noneya 10.1.1.10 +i AKAQEK CAA :Jim Smith

	// The server is introducing a nick to us, this is no local client here
	// We're supposed to trust other servers, but there are a few things we can
	// object to

	// Make sure the server is authenticated
	if ( !pServer->IsAuthenticated() )
	{
		// Send out an error
		return true;
	}

	// Is this a nick change? or a totally new client?
	if ( pCmd->m_iQtyArgs < 9 )
	{
		// This looks like a simple nick change
		return Do_NICK_Change( pServer, pCmd );
	}

	return Do_NICK_New( pServer, pCmd );
}

bool CIRCLocalServer::OnUplinkRequest( CIRCUplinkServer* pUplink, BYTE * pRequest, int len )
{
	// All IRC uplink connections which we make return requests here
	CIRCCmds IRCCmds;
	CIRCCmds IRCCmdsTrail;		// We might have to parse the trail
	CIRCServer* pOriginator;	// Originator of the message (might just be a relay)

	bool fMabeyRelay = false;
	if ( ParseRequest( pRequest, len, &IRCCmds ) )
	{
		for( int i = 0; i < IRCCmds.m_iQtyCmds; i++ )
		{
			CIRCCmd* pCmd = &IRCCmds.m_Cmd[i];
			IRC_TYPE wType = (IRC_TYPE)FindTable( pCmd->m_szArgs[0], CIRCTools::sm_CmdTbl, COUNTOF(CIRCTools::sm_CmdTbl));
			if ( wType == IRC_NONE )
			{
				if ( pCmd->m_iQtyArgs == 0 )
				{
					// Message is in the trailing. I guess we have to parse that too
					if ( ParseRequest( (unsigned char *)&pCmd->m_szTrailing[0], strlen( pCmd->m_szTrailing ), &IRCCmdsTrail ) )
					{
						// I hope there's only one command coming in like this
						pCmd = &IRCCmdsTrail.m_Cmd[0];
						wType = (IRC_TYPE)FindTable( pCmd->m_szArgs[1], CIRCTools::sm_CmdTbl, COUNTOF(CIRCTools::sm_CmdTbl));
						fMabeyRelay = true;
					}
				}
				else
				{
					// This is probably a server ID (nick really),
					// the real command is probably the 2nd argument
					CIRCServerNick* pServerNick = FindServerNick( pCmd->m_szArgs[0] );
					if ( pServerNick )
					{
						pOriginator = pServerNick->m_pServer;
					}
					if ( pOriginator )
					{
						wType = (IRC_TYPE)FindTable( pCmd->m_szArgs[1], CIRCTools::sm_CmdTbl, COUNTOF(CIRCTools::sm_CmdTbl));
						fMabeyRelay = true;
					}
					else
					{
						// What's this?
						break; // I guess
					}
				}
			}
			switch ( wType )
			{
				case IRC_PRIVATE:
				{
					// :Westy PRIVMSG #Channel2 :this is a message
					char szFrom[NICKLEN];
					char szTo[CHANNELLEN];
					char szMessage[BUFSIZE];

					strcpy( szFrom, pCmd->m_szArgs[0] );
					strcpy( szTo, pCmd->m_szArgs[2] );
					strcpy( szMessage, pCmd->m_szTrailing );

					CIRCLRClient* pRClientFrom = HuntClientByNick( szFrom );

					// What is this message going to?
					// Check to see if these are channels, or host masks
					if ( IsChannelName( szTo ) )
					{
						// Treat them all as channels for now I guess
						CIRCChannel* pChannel = HuntChannel( szTo, "", NULL );
						if ( pChannel )
						{
							// Channel Broadcast
							// example to a channel: :Westy2!noneya@10.1.1.1 PRIVMSG #Channel :hello?
							SendToChannel( pChannel, pRClientFrom, pUplink, ":%s PRIVMSG %s :%s\r\n",
								pRClientFrom->GetNickUserIP(), szTo, pCmd->m_szTrailing );
						}
						else
						{
							// Send an error back to the server, and don't relay this
							continue;
						}
					}
					else
					{
						// Must be a user
						CIRCLRClient* pLClientTo = HuntClientByNick( szTo );
						if ( pLClientTo && IsMyClient( pLClientTo ) )
						{
							SendToOne( pLClientTo, ":%s PRIVMSG %s :%s\r\n",
								pRClientFrom->GetNickUserIP(), pLClientTo->m_szNick, pCmd->m_szTrailing );
						}
					}
					// TODO: Relay this to the other servers
					break;
				}
				case IRC_PASS:
				{
					// Servers send something like this:
					// PASS :passwd
					// SERVER <hostname> 1 <time stamp> <time stamp> <protocol> <numeric nick> :<server info>
					// SERVER irc.sphereserver.com 1 964345215 964345215 J10 B]] :[10.1.1.1] SphereServer, Boston, MA, USA

					// Servers *can* send this to us
					// but it's not required unless we want some security
					// If we get it, then they're probably trying to log in

					// If passwords get sent, they always are always sent first

					// See if this matches what we have for them
					if ( !strcmp( pUplink->m_szPassword, pCmd->m_szTrailing ) )
					{
						// It does, get some info on them
						// Grab their IP address
						CSocketAddress PeerName = pUplink->m_Socket.GetPeerName();
						pUplink->m_iaClientIP = PeerName;
						break;
					}
					// Send an error message back informing them of this
					break;
				} // case IRC_PASS:
				case IRC_SERVER:
				{
					// Only remote servers will send this to us
					// Remote servers will send it to log into the local server
					// and become part of the network of IRC servers
					// If we get this message in here, it's a server logging in
					// or another machine which is trying to trick us
					// SERVER irc2.sphereserver.com 1 964923381 964939435 J10 C]] :[10.1.1.10] SphereServer, Boston, MA, USA

					// Make sure everything it sends to us is valid
					if ( strcmp( pUplink->m_szServerName, pCmd->m_szArgs[1] ) )
					{
						// Send out an error message
						break;
					}
					pUplink->m_iHops = atoi( pCmd->m_szArgs[2] );
					pUplink->m_tOnLineSince = atoi( pCmd->m_szArgs[3] );
					pUplink->m_tLinkedAt = atoi( pCmd->m_szArgs[4] );
					strcpy( pUplink->m_szProtocol, pCmd->m_szArgs[5] );
					strcpy( pUplink->m_szDescription, pCmd->m_szTrailing );
					// This "nick" is really a network id, the first character
					// is the actual id...weird
					strcpy( pUplink->m_szNick, pCmd->m_szArgs[6] );
					pUplink->Authenticate( true );
					// Add this guy's nick to the list of server nicks for quick lookup
					// during relays
					CIRCServerNick* pServerNick = new CIRCServerNick();
					strcpy( pServerNick->m_szNick, pUplink->m_szNick );
					pServerNick->m_pServer = pUplink;
					m_ServerNicks.Add( pServerNick );
					g_Log.Event( LOGM_INIT, "IRC server %s has been authenticated, net burst in progress.\n", (LPCTSTR) pCmd->m_szArgs[1] );
					break;
				} // case IRC_SERVER:
				case IRC_NICK:
				{
					if ( !Do_NICK_Server( pUplink, pCmd ) )
						return false;
					break;
				} // case IRC_NICK:
				case IRC_USER:
				{
					// RFC 1459, 4.1.3 "User Message"
					// COMMAND: USER <username> <hostname> <servername> :<realname>

					// Evidently server's don't use this, they use the NICK
					// command to introduce clients
					break;
				} // case IRC_USER:
				case IRC_PONG:
				{
					// RFC 1459, 4.6.3 "Pong Message"
					// COMMAND: PONG <daemon> [<daemon>]
					// This says nothing about trailing, but the trailing returns the
					// random number sent out with the PING message, so I guess we use it

					// I guess both remote servers and local clients will send this to us
					// Remote servers will send it to in response to a PING sent to it from us
					// Local clients will send it to in response to a PING sent to it from us
					// If we get this message in here, it's either a local client or a server

					DWORD dwCookie = atol( pCmd->m_szTrailing );

					return false;
				} // case IRC_PONG:
				case IRC_PING:
				{
					// :irc1.sphereserver.com PING :irc1.sphereserver.com
					// :irc3.sphereserver.com PONG irc3.sphereserver.com :irc1.sphereserver.com
					// Send back a PONG with out host name back to the originator

					// Is is for us?
					SendToOne( pUplink, ":%s PONG %s :%s\r\n",
						pCmd->m_szArgs[0], pCmd->m_szTrailing, m_szName );
					g_Log.Event( LOGM_INIT, "IRC server %s has pinged us.\n", pCmd->m_szArgs[0] );
					break;
				} // case IRC_PONG:
				case IRC_QUIT:
				{
					// Servers send this when a client quits

					break;
				}
				case IRC_SQUIT:
				{
					// RFC 1459, 4.1.7 "Server quit Message"
					// COMMAND: SQUIT <server> <comment>
					// Replies: ERR_NOPRIVILEGES, ERR_NOSUCHSERVER

					// Trailing is the comment

					// Try to let them gracefully exit the system
					ServerQuit( pUplink, IRCCmds.m_Cmd[i].m_szTrailing );
					return false;
				}
				case IRC_BURST:
				{
					// I guess these are all channels that exist on the network
					// C BURST #MyChannel 964925561 CAA
					// C BURST #Channel3 964925152 CAA:o
					// C BURST #Channel 964923721 CAA:o
					break;
				}
				case IRC_END_OF_BURST:
				{
					// C END_OF_BURST

					// B END_OF_BURST
					// B EOB_ACK
					Do_EOB_ACK( pUplink );
					break;
				}
			}
		}
	}
	pRequest[len] = '\0';
	return true;
}

void CIRCLocalServer::Do_EOB_ACK( CIRCServer* pServer )
{
	if ( pServer->m_fNeedsSync )
	{
		// Send out all our clients and channels here
		// C NICK Revenge 1 965202535 ~imya 10.1.1.10 +ow AKAQEK CAA :Jim Smith
		int i;
		for( i = 0; i < m_LClients.GetCount(); i++ )
		{
			char szBase64IP[6];
			CIRCLRClient* pLC = m_LClients[i];
			char szNumNick[4];
			int iNumNick = MakeNumNick( pLC->m_pNumNick, szNumNick );
			strcpy( szBase64IP, CIRCTools::IntToBase64( pLC->m_iaClientIP.s_addr ) );
			SendToOne( pServer, "%c NICK %s %d %lu %s %s %s %s %s :%s\r\n",
				m_szNick[0], pLC->m_szNick, pLC->m_iHops + 1,
				pLC->m_tLastNick, pLC->m_szUser, pLC->m_szHost,
				/*pLC->m_szMode*/ "", szBase64IP, szNumNick, pLC->m_szRealName );
		}
		for( i = 0; i < m_RClients.GetCount(); i++ )
		{
			CIRCLRClient* pRC = m_RClients[i];
			if ( pRC->m_pServer == pServer )
				continue;

			char szBase64IP[6];
			char szNumNick[4];

			int iNumNick = MakeNumNick( pRC->m_pNumNick, szNumNick );
			strcpy( szBase64IP, CIRCTools::IntToBase64( pRC->m_iaClientIP.s_addr ) );
			SendToOne( pServer, "%c NICK %s %d %lu %s %s %s %s %s :%s\r\n",
				m_szNick[0], pRC->m_szNick, pRC->m_iHops + 1,
				pRC->m_tLastNick , pRC->m_szUser, pRC->m_szHost,
				/*pRC->m_szMode*/ "", szBase64IP, szNumNick, pRC->m_szRealName );
		}
		// Send out all channels registered with us and the members in them
		// Can we split this us somehow if it's going to overflow the buffer?
		// Just do ChOps for now (eventually we gotta send all UMODES too (+v, etc)
		char szSendBuf[2048];
		int iSendBufLen = 0;
		for( i = 0; i < m_Channels.GetCount(); i++ )
		{
			CIRCChannel* pChannel = m_Channels[i];
			if ( !IsLocalChannel( pChannel->m_szChName ) )
			{
				CIRCTools::sprintf_irc( szSendBuf, "%c BURST %s %lu",
					m_szNick[0], pChannel->m_szChName, pChannel->m_tCreationTime );

				char szNumNick[4];
				CIRCLRClient* pLRClient = NULL;
				CIRCChannelMember* pChanMember = NULL;
				// Build lists for each mode
				char szOperators[2048];
				szOperators[0] = 0;
				int iOperators = 0;
				int j;
				for( j = 0; j < pChannel->m_ChanMembers.GetCount(); j++ )
				{
					pChanMember = pChannel->m_ChanMembers[j];
					if ( pChanMember->m_dwMode & ONCHMODE_CHANOP )
					{
						iOperators++;
						MakeNumNick( pChanMember->m_pLRClient->m_pNumNick, szNumNick );
						strcat( szOperators, szNumNick );
						strcat( szOperators, "," );
					}
				}
				if ( iOperators > 0 )
					szOperators[strlen( szOperators ) - 1] = 0;

				char szHasVoice[2048];
				szHasVoice[0] = 0;
				int iHasVoice = 0;
				for( j = 0; j < pChannel->m_ChanMembers.GetCount(); j++ )
				{
					pChanMember = pChannel->m_ChanMembers[j];
					if ( pChanMember->m_dwMode & ONCHMODE_VOICE )
					{
						iHasVoice++;
						MakeNumNick( pChanMember->m_pLRClient->m_pNumNick, szNumNick );
						strcat( szHasVoice, szNumNick );
						strcat( szHasVoice, "," );
					}
				}
				if ( iHasVoice > 0 )
					szHasVoice[strlen( szHasVoice ) - 1] = 0;

				char szMembers[2048];
				szMembers[0] = 0;
				int iMembers = 0;
				for( j = 0; j < pChannel->m_ChanMembers.GetCount(); j++ )
				{
					// Filter out m_Operators and m_HasVoice from this list
					if ( !( pChannel->m_ChanMembers[j]->m_dwMode & ONCHMODE_CHANOP ) &&
						 !( pChannel->m_ChanMembers[j]->m_dwMode & ONCHMODE_VOICE ) )
					{
						iMembers++;
						pLRClient = pChannel->m_ChanMembers[j]->m_pLRClient;
						MakeNumNick( pLRClient->m_pNumNick, szNumNick );
						strcat( szMembers, szNumNick );
						strcat( szMembers, "," );
					}
				}
				iMembers = iMembers - iOperators - iHasVoice;
				if ( iMembers < 0 )
					iMembers = 0;
				if ( iMembers > 0 )
					szMembers[strlen( szMembers ) - 1] = 0;

				iSendBufLen = strlen( szSendBuf );
				// Put the whole thing together
				// TODO: Add not voiced people to this stuff
				if ( IsModelessChannel( pChannel->m_szChName ) )
				{
					// These are easy, everyones voiced and not an op
					// C BURST +Modeless1 965355932 CAB,CAA
					SendToOne( pServer, "%s %s\r\n",
						szSendBuf, szMembers );
				}
				else
				{
					if ( iHasVoice > 0 )
					{
						// These guys go first
						if ( iOperators > 0 )
						{
							// C BURST #Channel2 965355926 CAC,CAD:o,CAB,CAA
							SendToOne( pServer, "%s %s:0,%s\r\n",
								szSendBuf, szHasVoice, szOperators );
						}
						else
						{
							// C BURST +Modeless1 965355932 CAB,CAA
							SendToOne( pServer, "%s %s\r\n",
								szSendBuf, szHasVoice );
						}
					}
					else
					{
						// C BURST #Channel1 965355922 CAB,CAA:o

						// Everyone's a ChOp on this channel
							SendToOne( pServer, "%s %s:o\r\n",
								szSendBuf, szOperators );
					}
				}
			}
		}

		SendToOne( pServer, "%c END_OF_BURST\r\n", m_szNick[0] );

		g_Log.Event( LOGM_INIT, "Net burst with IRC server %s has been completed.\n", pServer->m_szServerName );

		pServer->m_fNeedsSync = false;

	}

	SendToOne( pServer, "%c EOB_ACK\r\n", m_szNick[0] );

}

CIRCDownlinkServer* CIRCLocalServer::FindDownlinkServer( CClient* pClient )
{
	for( int i = 0; i < m_DownlinkServers.GetCount(); i++ )
	{
		if ( pClient == m_DownlinkServers[i]->GetClient() )
			return m_DownlinkServers[i];
	}
	return NULL;
}

int CIRCLocalServer::GetTotalClients()
{
	int iSum = 0;
	int i;
	for( i = 0; i < m_UplinkServers.GetCount(); i++ )
		iSum += m_UplinkServers[i]->m_iClients;
	for( i = 0; i < m_DownlinkServers.GetCount(); i++ )
		iSum += m_DownlinkServers[i]->m_iClients;
	iSum += m_LClients.GetCount();
	return iSum;
}

int CIRCLocalServer::CountInvisibleClients()
{
	int iTotal = 0;
	for( int i = 0; i < m_LClients.GetCount(); i++ )
	{
		if ( m_LClients[i]->IsFlag( FLAGS_INVISIBLE ) )
			iTotal++;
	}
	return iTotal;
}

int CIRCLocalServer::GetTotalInvisibleClients()
{
	int iSum = 0;
	int i;
	for( i = 0; i < m_UplinkServers.GetCount(); i++ )
		iSum += m_UplinkServers[i]->m_iInvisibleClients;
	for( i = 0; i < m_DownlinkServers.GetCount(); i++ )
		iSum += m_DownlinkServers[i]->m_iInvisibleClients;
	iSum += CountInvisibleClients();
	return iSum;
}

bool CIRCLocalServer::OnRequest( CClient* pClient, BYTE * pRequest, int len )
{
	// All IRC local clients and remote servers which connect to us
	// come through here

	// Do we already know who this is?

	// Check for remote server
	CIRCDownlinkServer* pDownlink = FindDownlinkServer( pClient );
	if ( pDownlink )
	{
		// It's a remote IRC server that we already know about
		return OnDownlinkServerRequest( pClient, pRequest, len );
	}
	// Is it an unauthenticated local client?
	CIRCLRClient* pLClient = FindLClient( pClient );
	if ( pLClient )
	{
		if ( pLClient->IsAuthenticated() )
			// It's an attached local client that we already know
			return OnLClientRequest( pLClient, pRequest, len );
	}
	// Must be a machine in the process of logging in to us
	return OnMachineRequest( pClient, pRequest, len );
}

bool CIRCLocalServer::OnMachineRequest( CClient* pClient, BYTE * pRequest, int len )
{
	// Only unidentified machines come in here
	// What is trying to connect to us?
	// With these messages, we can figure out what's going on
	//		IRC_USER, IRC_NICK, IRC_SERVER, IRC_PONG
	CIRCCmds IRCCmds;
	if ( ParseRequest( pRequest, len, &IRCCmds ) )
	{
		for( int i = 0; i < IRCCmds.m_iQtyCmds; i++ )
		{
			CIRCCmd* pCmd = &IRCCmds.m_Cmd[i];
			IRC_TYPE wType = (IRC_TYPE)FindTable( pCmd->m_szArgs[0], CIRCTools::sm_CmdTbl, COUNTOF(CIRCTools::sm_CmdTbl));
			switch ( wType )
			{
				// Servers send something like this:
				// PASS :passwd
				// SERVER <hostname> 1 <time stamp> <time stamp> <protocol> <numeric nick> :<server info>
				// SERVER irc.sphereserver.com 1 964345215 964345215 J10 B]] :[10.1.1.1] SphereServer, Boston, MA, USA
				case IRC_PASS:
				{
					// Both remote servers and local clients CAN send this to us
					// but it's not required unless we want some security
					// If we get it, then they're probably trying to log in

					// Passwords are always sent first, by both remote servers
					// and locally connected clients

					// See if they already sent a password before
					CIRCMachine* pMachine = FindMachine( pClient );

					if ( !pMachine )
					{
						// Add this client to our list of m_Machines

						// Create a new machine
						pMachine = new CIRCMachine();
						// Set the password for later use
						strcpy( pMachine->m_szPassword, IRCCmds.m_Cmd[i].m_szArgs[2] );
						// Set their client
						pMachine->SetClient( pClient );
						// Grab their IP address
						// CSocketAddress PeerName = pClient->m_Socket.GetPeerName();
						pMachine->m_iaClientIP = pClient->m_PeerName;
						// Add to the m_Machines list
						m_Machines.Add( pMachine );
					}
					else
					{
						// RFC 1459 says they can send as many passwords as they want,
						// but only the last one counts, so over write their old password
						// with this one
						strcpy( pMachine->m_szPassword, IRCCmds.m_Cmd[i].m_szArgs[2] );
					}
					break;
				} // case IRC_PASS:
				case IRC_NICK:
				{
					// Both remote servers and local clients will send this to us
					// Remote servers will send it to introduce new users (so they'll
					// never get to this point in this part of the log on sequence)
					// Local clients send it to log on to the local server
					// If we get this message in here, it's a local client logging in

					// RFC 1459, 4.1.2 "Nick Message"
					// COMMAND: NICK <nickname> [ <hopcount> ]
					// If hopcount is supplied by a client, it must be ignored
					//
					// Servers will send us this (with a hopcount param)
					//	when they introduce a (possibly) new nick to the network
					// replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTERED,
					//			ERR_NICKCOLLISION
					//

					// Someone is sending us a nick, see if this guy sent a password yet

					// NOTE: Nick parameter varies depending on if it was rejected
					//		 initially or not,	rejected = nick is in trailing,
					//							not rejected = nick is in regular parameter list

					CIRCLRClient* pLClient = NULL;
					char szNick[NICKLEN];

					CIRCMachine* pMachine = FindMachine( pClient );
					if ( !pMachine )
					{
						// No password sent, just make a local client for them here

						// Add this client to our list of m_Machines

						// Create a new local client
						pLClient = new CIRCLRClient();
						// Set their client
						pLClient->SetClient( pClient );
						// Grab their IP address
						pLClient->m_iaClientIP = pClient->m_PeerName;
						// Add to the m_Machines list
						m_Machines.Add( pLClient );
						// Get the nick param from the correct place
						strcpy( szNick, IRCCmds.m_Cmd[i].m_szArgs[1] );
					}
					else
					{
						// If we're in here, they are either new, or we rejected their first
						// choice of a nick

						pLClient = dynamic_cast <CIRCLRClient*>( pMachine );

						if ( !pLClient )
						{
							// This is a new connection, and it
							// needs promoting to a local client
							pLClient = new CIRCLRClient();
							// Copy all their stuff over
							pLClient->SetClient( pMachine->GetClient() );
							pLClient->m_iaClientIP = pMachine->m_iaClientIP;
							// Remove the old machine from the m_Machines list
							for( int iIndex = 0; iIndex < m_Machines.GetCount(); iIndex++ )
							{
								if ( m_Machines[iIndex] == pMachine )
								{
									m_Machines.RemoveAt( iIndex );
									break;
								}
							}
							// and add the new local client
							m_Machines.Add( pLClient );
							// Get the nick param from the correct place
							strcpy( szNick, IRCCmds.m_Cmd[i].m_szArgs[1] );
						}
						else
						{
							// We rejected their first nick, and the client
							// sends the replacement in a different place
							strcpy( szNick, IRCCmds.m_Cmd[i].m_szTrailing );
						}
					}
					// Now copy or update their nick from the one they just sent

					// Is it a valid looking nick? If it's too long, just fix it for them
					// although RFC 1459 states that we *should* reject it, but why?
					int iLen = AnalyzeNick( szNick );

					if ( iLen == 0 )
					{
						// Send the ERR_ERRONEUSNICKNAME message
						SendToOne( pLClient, IRCErrorString( ERR_ERRONEUSNICKNAME ),
							GetHost(), szNick );
						break;
					}

					// Check to see if this nick is already taken
					// First check for an existing nick
					CIRCNick *pNick = FindNick( szNick );
					if ( pNick )
					{
						// Now check to see if it's currently owned
						if ( pNick->IsOwned() )
						{
							// It is, send out nick in use message
							// example: :irc.sphereserver.com 433 * Revenge :Nickname is already in use.
							SendToOne( pLClient, IRCErrorString( ERR_NICKNAMEINUSE ),
								GetHost(), szNick );
							break;
						}
					}
					else
					{
						// Now create the nick tracking stuff
						pNick = new CIRCNick();
						m_Nicks.Add( pNick );
						strcpy( pNick->m_szNick, szNick );
					}
					// Save their nick and update the nick tracking record
					strcpy( pLClient->m_szNick, szNick );
					pNick->NewOwner( pLClient );

					// Now we can move them from the unknown m_Machines list
					for( int iIndex = 0; iIndex < m_Machines.GetCount(); iIndex++ )
					{
						if ( m_Machines[iIndex] == pLClient )
						{
							m_Machines.RemoveAt( iIndex );
							break;
						}
					}
					// ... to the m_LClients list
					m_LClients.Add( pLClient );
					time_t tStart;
					time( &tStart );

					pLClient->m_tLastNick = tStart;
					// And finally give them a numnick
					CIRCNumNick* pNumNick = GetNextSlot();
					if ( pNumNick )
					{
						pNumNick->m_pNick = pNick;
						pLClient->m_pNumNick = pNumNick;
					}
					else
					{
						// This is bad
					}
					break;
				} // case IRC_NICK:
				case IRC_USER:
				{
					// Both remote servers and local clients will send this to us
					// Remote servers will send it to introduce new users (so they'll
					// never get to this point in this part of the log on sequence)
					// Local clients send it to log on to the local server
					// If we get this message in here, it's a local client logging in

					// RFC 1459, 4.1.3 "User Message"
					// COMMAND: USER <username> <hostname> <servername> :<realname>

					// NOTE: This information can be easily faked

					CIRCLRClient *pLClient = FindLClient( pClient );
					if ( !pLClient )
					{
						// If they're not in with the rest of the local clients,
						// then we probably rejected the nick, so they're still in
						// with the unknowns, but we can still procede with
						// authenticating them if we can cast them to a CIRCLRClient
						CIRCMachine *pMachine = FindMachine( pClient );
						pLClient = dynamic_cast <CIRCLRClient*>(pMachine);
					}

					if ( pLClient )
					{
						// Username
						strcpy( pLClient->m_szUser, IRCCmds.m_Cmd[i].m_szArgs[1] );

						// Hostname
						// mIRC puts quotes around some of this stuff
						char szHost[HOSTLEN];
						strcpy( szHost, IRCCmds.m_Cmd[i].m_szArgs[2] );
						szHost[strlen( szHost )-1] = 0;
						strcpy( pLClient->m_szHostGiven, szHost + 1 );

						// Make a sync call for their real host name
						// Any way to make this async?
						pLClient->m_iaClientIP.GetHostStr( pLClient->m_szHost, sizeof(pLClient->m_szHost) );

						// Server name (what's this for?)
						// I think it's the server's ip or name or something
						// (is this real?)
						// mIRC puts quotes around some of this stuff
						char szServerName[HOSTLEN];
						strcpy( szServerName, IRCCmds.m_Cmd[i].m_szArgs[3] );
						szServerName[strlen( szServerName )-1] = 0;
						strcpy( pLClient->m_szServerName, szServerName + 1 );

						// Realname
						strcpy( pLClient->m_szRealName, IRCCmds.m_Cmd[i].m_szTrailing );
						// Now send the cookie ping (this authenticates who they are sorta)
						Send_PING( pLClient );
					}
					else
					{
						// This shouldn't happen
						pRequest[len] = '\0';
						return false;
					}
					break;
				} // case IRC_USER:
				case IRC_SERVER:
				{
					// Only remote servers will send this to us
					// Remote servers will send it to log into the local server
					// and become part of the network of IRC servers
					// If we get this message in here, it's a server logging in
					// or another machine which is trying to trick us
//					SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
//						CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType]);

					break;
				} // case IRC_SERVER:
				case IRC_PONG:
				{
					// RFC 1459, 4.6.3 "Pong Message"
					// COMMAND: PONG <daemon> [<daemon>]
					// This says nothing about trailing, but the trailing returns the
					//	random number sent out with the PING message, so I guess we use it

					// I guess both remote servers and local clients will send this to us
					// Remote servers will send it to in response to a PING sent to it from us
					// Local clients will send it to in response to a PING sent to it from us
					// If we get this message in here, it's either a local client or a server

					DWORD dwCookie = atol( IRCCmds.m_Cmd[i].m_szTrailing );

					// What are they?
					CIRCLRClient* pLClient = FindLClient( pClient );

					if ( pLClient )
					{
						if ( pLClient->m_dwCookie != dwCookie )
						{
							// They trying to guess our cookies?
							if ( !pLClient->IsAuthenticated() ) // Kill them here
								return false;
							// Why did we get this message again?
							break;
						}
						// Now we can finally authenticate them
						if ( !pLClient->IsAuthenticated() )
						{
							time_t tNow;
							time( &tNow );
							pLClient->m_tLastTime = tNow;
							pLClient->Authenticate( true );
							NewConnection( pLClient );
							Send_LUSERS( pLClient );
							Send_MOTD( pLClient );
						}
						break;
					}
					CIRCDownlinkServer *pDownlink = FindDownlinkServer( pClient );
					if ( pDownlink )
					{
						// now what?
						pDownlink->Authenticate( true );
						AddServer( pDownlink );
						break;
					}
					// Don't know what this is...kill them I guess
					return false;
				} // case IRC_PONG:
				case IRC_QUIT:
				{
					// We'll get here if the connection was dropped on purpose
					// during log on
					// If it's a client, trailing is the reason for leaving

					// See if we can figure out what they are first
					CIRCMachine *pMachine = FindMachine( pClient );
					// Can we cast it up?
					CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>(pMachine);
					if ( pMachine && !pLClient )
					{
						// It's an unknown machine, and they can just go without much cleanup
						RemoveMachine( pMachine );
						delete pMachine;
						return false;
					}
					pLClient = FindLClient( pClient );
					if ( pLClient )
					{
						// Try to let them gracefully exit the system
						ClientQuit( pLClient, IRCCmds.m_Cmd[i].m_szTrailing );
						break;
					}
					CIRCDownlinkServer *pDownlink = FindDownlinkServer( pClient );
					if ( pDownlink )
					{
						// If this is a server, they're not supposed to send this,
						// so I guess we kill them
						return false;
					}
					// Don't see how they can get here so kill them if they do
					return false;
				}
				case IRC_SQUIT:
				{
					// RFC 1459, 4.1.7 "Server quit Message"
					// COMMAND: SQUIT <server> <comment>
					// Replies: ERR_NOPRIVILEGES, ERR_NOSUCHSERVER

					// Trailing is the comment

					// Try to let them gracefully exit the system
					CIRCDownlinkServer *pDownlink = FindDownlinkServer( pClient );
					if ( pDownlink )
					{
						ServerQuit( pDownlink, IRCCmds.m_Cmd[i].m_szTrailing );
						break;
					}
					// Shouldn't happen, could be a client pretending to be
					// a server, guess we gotta kill them
					return false;
				}
			}
		}
	}
	pRequest[len] = '\0';
	return true;
}

void CIRCLocalServer::ServerQuit( CIRCServer* pServer, char* szReason )
{
	g_Log.Event( LOGM_INIT, "Link with %s has been canceled (%s).\n", pServer->m_szServerName, szReason );
	// TODO: Part all of the clients behind this server, and
	// relay this message to all the other servers as well as
	// IRC Opers local to this server
	return;
}

void CIRCLocalServer::ClientQuit( CIRCLRClient* pLClient, char* szReason )
{
	// PART them from all their channels first
	for(int i = 0; i < pLClient->m_Channels.GetCount(); i++ )
		PartChannel( pLClient, pLClient->m_Channels[i]->m_szChName, szReason );
	// Last but not least, update the nick history
	CIRCNick * pNick = FindNick( pLClient->m_szNick );
	pNick->ClearOwner();
	// Now they can go
	RemoveMachine( pLClient );
}

void CIRCLocalServer::ClientQuit( CClient* pClient )
{
	// The game server will call this if a client drops without warning

	// This one left unexpectedly

	// Try to figure out what kind of client this was
	CIRCMachine *pMachine = FindMachine( pClient );
	// Can we cast it up?
	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);
	if ( pMachine && !pLClient )
	{
		// It's an unknown machine, and they can just go without much cleanup
		for( int i = 0; i < m_Machines.GetCount(); i++ )
		{
			if ( pMachine == m_Machines[i] )
			{
				m_Machines.RemoveAt( i );
				break;
			}
		}
		delete pMachine;
		return;
	}
	pLClient = FindLClient( pClient );
	if ( pLClient )
	{
		// Try to let them gracefully exit the system
		ClientQuit( pLClient, "lost connection" );
		return;
	}
	CIRCDownlinkServer *pDownlink = FindDownlinkServer( pClient );
	if ( pDownlink )
	{
		// Try to let them gracefully exit the system
		ServerQuit( pDownlink, "Bad Link" );
		return;
	}
}

bool CIRCLocalServer::OnDownlinkServerRequest( CClient* pClient, BYTE * pRequest, int len )
{
	// Only authenticated downlink servers can come in here
	pRequest[len] = '\0';
	return true;
}

void CIRCLocalServer::Send_INFO( CIRCMachine* pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>( pMachine );

	if ( IsOper( pLClient ) )
	{

		SendToOne( pLClient, IRCReplyString( RPL_INFO ),
			m_szHost, pLClient->m_szNick, m_szDeamonName );
		SendToOne( pLClient, IRCReplyString( RPL_INFO ),
			m_szHost, pLClient->m_szNick, m_szCopyright );
		SendToOne( pLClient, IRCReplyString( RPL_INFO ),
			m_szHost, pLClient->m_szNick, "-" );
	}
	else
	{
		SendToOne( pLClient, IRCReplyString( RPL_INFO ),
			m_szHost, pLClient->m_szNick, m_szDeamonName );
	}

	SendToOne( pLClient, ":%s %d %s :Birth Date: %s, compile #%s\r\n",
		m_szHost, RPL_INFO, pLClient->m_szNick, __DATE__ " at " __TIME__ " EST", m_szCompile );

	char szTZ[1];
	szTZ[0] = ( ( g_Serv.m_TimeZone < 0 ) ? '+' : '-' );
	SendToOne( pLClient, ":%s %d %s :On-line since %s %s%i GMT\r\n",
		m_szHost, RPL_INFO, pLClient->m_szNick,
		CIRCTools::TheTime( m_tOnlineSince ), szTZ, g_Serv.m_TimeZone );

	SendToOne( pLClient, IRCReplyString( RPL_ENDOFINFO ), m_szHost, pLClient->m_szNick );

	return;
}

void CIRCLocalServer::Send_TIME( CIRCMachine* pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>( pMachine );

	time_t tNow;
	time( &tNow );

	SendToOne( pLClient, IRCReplyString( RPL_TIME ),
		m_szHost, pLClient->m_szNick, m_szHost, tNow, 0,
		CIRCTools::TheTime( m_tOnlineSince ), ((g_Serv.m_TimeZone < 0) ? "+" : "-"),g_Serv.m_TimeZone );

	return;
}

void CIRCLocalServer::Send_ADMIN( CIRCMachine* pMachine )
{
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>( pMachine );

	SendToOne( pLClient, IRCReplyString( RPL_ADMINME ),
		m_szHost, pLClient->m_szNick, m_szName );

	SendToOne( pLClient, IRCReplyString( RPL_ADMINLOC1 ),
		m_szHost, pLClient->m_szNick, m_AdminLines[0]->GetLine() );

	SendToOne( pLClient, IRCReplyString( RPL_ADMINLOC2 ),
		m_szHost, pLClient->m_szNick, m_AdminLines[1]->GetLine() );

	SendToOne( pLClient, IRCReplyString( RPL_ADMINEMAIL ),
		m_szHost, pLClient->m_szNick, m_AdminLines[2]->GetLine() );

	return;
}

bool CIRCLocalServer::ParseRequest( BYTE * pRequest, int len, CIRCCmds* pCmds )
{
	// Given the buffer, return it all parsed and ready to go

	char irc_buffer[BUFSIZE];
	memset(irc_buffer, 0, BUFSIZE);
	strncpy(irc_buffer, (char *)pRequest, len);

	// First break up multiple messages (if any) into separate messages
	TCHAR * ppLines[16] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	int iQtyLines = Str_ParseCmds( (TCHAR*) irc_buffer, ppLines, COUNTOF(ppLines), "\n" );
	if ( strlen( ppLines[iQtyLines-1] ) == 0 )
		iQtyLines--;
	if ( iQtyLines < 1 )
	{
		pRequest[len] = '\0';
		return true;
	}

	char irc_messages[16][BUFSIZE];
	int i;
	for(i = 0; i < iQtyLines; i++)
		strcpy(irc_messages[i], ppLines[i]);
	// irc_messages[0] = first message, irc_messages[1] = second, etc etc
	char irc_MainPart[BUFSIZE];
	memset(irc_buffer, 0, BUFSIZE);
	pCmds->m_iQtyCmds = iQtyLines;
	for(i = 0; i < iQtyLines; i++)
	{
		// First separate off the trailing parameter (there's only one, and it's optional)
		TCHAR * ppMsgParts[2] = { NULL, NULL };
		int iQtyMsgParts = Str_ParseCmds( (TCHAR*) irc_messages[i], ppMsgParts, COUNTOF(ppMsgParts), ":" );
		if ( iQtyMsgParts < 1 )
		{
			// Can this happen?
			pRequest[len] = '\0';
			return true;
		}
		// ppMsgParts[0] = main part of the message (space is the delimiter)
		// ppMsgParts[1] = trailing part of the message (may contain spaces)
		strcpy(irc_MainPart, ppMsgParts[0]);
		strcpy(pCmds->m_Cmd[i].m_szTrailing, ( ppMsgParts[1] == NULL ) ? "" : ppMsgParts[1]);

		// Now separate the main part into it's arguments (space is delimiter)
		// (there can be up to 16 of these)
		TCHAR * ppArgs[16];
		int iQtyArgs = Str_ParseCmds( (TCHAR*) irc_MainPart, ppArgs, COUNTOF(ppArgs), " " );
		pCmds->m_Cmd[i].m_iQtyArgs = iQtyArgs;
		int j;
		for(j = 0; j < iQtyArgs; j++)
		{
			strcpy( pCmds->m_Cmd[i].m_szArgs[j], ppArgs[j] );
		}
		for( int j2 = j; j2 < 16; j2++ )
		{
			pCmds->m_Cmd[i].m_szArgs[j][0] = 0;
		}
	}
	pRequest[len] = '\0';
	return true;
}

void CIRCLocalServer::BuildWhoSymbol( SWhoList* pWhoList, CIRCChannel* pChannel )
{
	BYTE bWhoSymPos = 0;
	static const char sm_szWhoSymbols[WHO_OPER_WALLOPS+1] =
	{ 'H', 'G', '*', '@', '+', '!', 'd', 'i', 'w' };

	CIRCLRClient* pMember = pWhoList->m_pLRClient;
	CIRCChannelMember* pOnChan = NULL;
	if ( pChannel )
		pOnChan = pChannel->GetChannelMember( pMember );

	if ( strlen( pMember->m_szAway ) > 0 )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_AWAY];
	else
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_NOT_AWAY];
	if ( IsOper( pMember ) )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_OPER];
	if ( pChannel && pChannel->IsChOp( pMember ) )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_ONCHANNEL_OP];
	if ( pChannel && pChannel->HasVoice( pMember ) )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_ONCHANNEL_VOICE];
	if ( pOnChan && pOnChan->m_dwMode & ONCHMODE_ZOMBIE )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_ONCHANNEL_ZOMBIE];
	if ( pMember->IsFlag( FLAGS_DEAF ) )
		pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_DEAF];
	if ( IsOper( pMember ) )
	{
		if ( pMember->IsFlag( FLAGS_INVISIBLE ) )
			pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_OPER_INVIS];
		if ( pMember->IsFlag( FLAGS_WALLOP ) )
			pWhoList->m_szWhoSymbols[bWhoSymPos++] = sm_szWhoSymbols[WHO_OPER_WALLOPS];
	}
	return;
}

bool CIRCLocalServer::InSameChannel( CIRCLRClient* pLClient1, CIRCLRClient* pLClient2 )
{
	for( int iChannel1 = 0; iChannel1 < pLClient1->m_Channels.GetCount(); iChannel1++ )
	{
		CIRCChannel* pChannel1 = pLClient1->m_Channels[iChannel1];
		for( int iChannel2 = 0; iChannel2 < pLClient2->m_Channels.GetCount(); iChannel2++ )
		{
			CIRCChannel* pChannel2 = pLClient2->m_Channels[iChannel2];
			if ( !strcmp( pChannel1->m_szChName, pChannel2->m_szChName ) )
			{
				return true;
			}
		}
	}
	return false;
}

void CIRCLocalServer::Do_WHO( CIRCMachine* pMachine, char* szQuery, char* szOperParam )
{

	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	bool fOnlyOpers = false;

	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>( pMachine );

	// We'll look for everyone in these lists:
	// m_Channels, m_LClients, m_LInvisibleClients and m_Opers

	// Let's build a list as we go of users who match what we're looking for and that we
	// can "see" and put their symbols here too
	CGPtrTypeArray< SWhoList* > List;

	// NOTES: szQuery is matched against users' host, server, real name and nick if
	// the channel cannot be found.

	// NOTEWELL: If szQuery is empty or consists of "0" or any wildcard which ends up
	// matching all (*!*@*.*, etc) then WHO is supposed to list all users who are not
	// invisible and who DON'T have a common channel (weird huh?)
	char szTempQuery[BUFSIZE];
	bool fNotMyChannel = false;
	if ( szQuery && ( ( szQuery[0] == '\0' ) ||
		(szQuery[1] == '\0' && ( szQuery[0] == '0' ) ) ) )
	{
		strcpy( szTempQuery, "*" );
		fNotMyChannel = true;
	}
	else
		strcpy( szTempQuery, CIRCTools::collapse( szQuery ) );
	int i;

	// Is it a channel they want?
	CIRCChannel* pChannel = HuntChannel( szQuery, "", NULL );
	if ( pChannel )
	{
		// If they are on this channel, they can see everyone, if not,
		// only list the visible ones

		int iMembers = 0;
		for( int iMember = 0; iMember < pChannel->m_ChanMembers.GetCount(); iMember++ )
		{
			CIRCChannelMember* pChanMember = pChannel->m_ChanMembers[iMember];
			CIRCLRClient* pMember = pChanMember->m_pLRClient;

			if ( pChanMember->m_dwMode & ONCHMODE_CHANOP ||
				!pMember->IsFlag( FLAGS_INVISIBLE ) )
			{
				SWhoList* pWhoList = new SWhoList( pMember );
				BuildWhoSymbol( pWhoList, pChannel );
				List.Add( pWhoList );
			}
		}
		goto do_who_list;
	}

	// Do they only want IRC Operators listed?
	// If "o" is specified, match masks against m_Opers only
	if ( strlen( szOperParam ) > 0 )
	{
		if ( szOperParam[0] == 'o' || szOperParam[0] == 'O' )
			fOnlyOpers = true;
	}

	// The query must match either the user, username, host, server's name or info.
	// If all of the fields fail the match, then they don't get listed

	// Looking for ops?
	if ( fOnlyOpers )
	{
		for( int iOper = 0; iOper < m_Opers.GetCount(); iOper++ )
		{
			CIRCLRClient* pOper = m_Opers[iOper];
			// If the oper is invisible they can only be seen by other opers
			if ( !IsOper( pLClient ) && pOper->IsFlag( FLAGS_INVISIBLE ) )
				continue;
			// Does it match the mask?
			if ( ( CIRCTools::match( szTempQuery, pOper->m_szNick ) ) &&
				 ( CIRCTools::match( szTempQuery, pOper->m_szUser ) ) &&
				 ( CIRCTools::match( szTempQuery, pOper->m_szHost ) ) &&
				 ( CIRCTools::match( szTempQuery, pOper->m_szServerName ) ) &&
				 ( CIRCTools::match( szTempQuery, pOper->m_szRealName ) ) )
				 continue;

			SWhoList* pWhoList = new SWhoList( pOper );
			BuildWhoSymbol( pWhoList, pChannel );
			List.Add( pWhoList );
		}
		goto do_who_list;
	}

	// They want anything visible that matches the mask
	for( i = 0; i < m_LClients.GetCount(); i++ )
	{
		CIRCLRClient* pCurrentClient = m_LClients[i];
		// Check visibility
		// Only server opers, same channel clients, and self can see these people
		if ( pCurrentClient->IsFlag( FLAGS_INVISIBLE ) )
			if ( !IsOper( pLClient ) )
				if ( pLClient != pCurrentClient )
					if ( !InSameChannel( pLClient, pCurrentClient ) )
						continue;
		// Filter out same channel people if that's what they want
		if ( fNotMyChannel && InSameChannel( pLClient, pCurrentClient ) )
			continue;
		// Does it match the mask?
		if ( ( CIRCTools::match( szTempQuery, pCurrentClient->m_szNick ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szUser ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szHost ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szServerName ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szRealName ) ) )
			 continue;
		// Add this one to the list
		SWhoList* pWhoList = new SWhoList( pCurrentClient );
		BuildWhoSymbol( pWhoList, pChannel );
		List.Add( pWhoList );
	}

	// Check for remote clients

	// They want anything visible that matches the mask
	for( i = 0; i < m_RClients.GetCount(); i++ )
	{
		CIRCLRClient* pCurrentClient = m_RClients[i];
		// Check visibility
		// Only server opers, same channel clients, and self can see these people
		if ( pCurrentClient->IsFlag( FLAGS_INVISIBLE ) &&
				(   !IsOper( pLClient ) && !InSameChannel( pLClient, pCurrentClient ) &&
					pLClient != pCurrentClient ) )
			continue;
		if ( fNotMyChannel && InSameChannel( pLClient, pCurrentClient ) )
			continue;
		// Does it match the mask?
		if ( ( CIRCTools::match( szTempQuery, pCurrentClient->m_szNick ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szUser ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szHost ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szServerName ) ) &&
			 ( CIRCTools::match( szTempQuery, pCurrentClient->m_szRealName ) ) )
			 continue;
		// Add this one to the list
		SWhoList* pWhoList = new SWhoList( pCurrentClient );
		BuildWhoSymbol( pWhoList, pChannel );
		List.Add( pWhoList );
	}

do_who_list:
	char szHost[16];
	for( i = 0; i < List.GetCount(); i++ )
	{
		int iHops = 0;
		char szServerName[HOSTLEN];
		CIRCLRClient* pLRClient = List[i]->m_pLRClient;
		if ( pLRClient->m_pServer )
		{
			iHops += pLRClient->m_iHops;
			strcpy( szServerName, pLRClient->m_pServer->m_szServerName );
		}
		else
		{
			strcpy( szServerName, m_szName );
		}
		char szWhoLine[BUFSIZE];
		strcpy(szHost, CIRCTools::inetntoa(List[i]->m_pLRClient->m_iaClientIP));
		sprintf( szWhoLine, "* %s %s %s %s %s :%d %s",
			pLRClient->m_szUser, szHost,
			szServerName,
			pLRClient->m_szNick,
			List[i]->m_szWhoSymbols, iHops, pLRClient->m_szRealName );
		SendToOne( pLClient, IRCReplyString( RPL_WHOREPLY ),
			m_szHost, pLClient->m_szNick, szWhoLine );
	}
	SendToOne( pLClient, IRCReplyString( RPL_ENDOFWHO ),
		m_szHost, pLClient->m_szNick, szQuery, szOperParam );
	return;
}

int CIRCLocalServer::Do_WALLCHOPS( CIRCMachine* pMachine, CIRCCmd* pCmd )
{
	SendToOne( pMachine, "%s :Sorry the WALLCHOPS command isn't working yet.\r\n",
		CIRCTools::sm_CmdTbl[IRC_INFO] );
	return 0;
}

int CIRCLocalServer::Do_Message( CIRCMachine* pMachine, CIRCCmd* pCmd, bool fNotice )
{
	SendToOne( pMachine, "%s :Sorry Messaging isn't working yet.\r\n",
		CIRCTools::sm_CmdTbl[IRC_INFO] );
	return 0;
}

int CIRCLocalServer::Do_NOTICE( CIRCMachine* pMachine, CIRCCmd* pCmd )
{
	char szChName[CHANNELLEN];

	strcpy( szChName, pCmd->m_szArgs[1]+1 );

	if ( IsMyClient( pMachine ) && strlen( pCmd->m_szArgs[1] ) > 0 &&
		pCmd->m_szArgs[1][0] == '@' && IsChannelName( szChName ) )
	{
		strcpy( pCmd->m_szArgs[1], szChName ); // Get rid of '@'
		return Do_WALLCHOPS( pMachine, pCmd );
	}
	return Do_Message( pMachine, pCmd, 1 );
}

void CIRCLocalServer::Do_USERHOST( CIRCMachine* pMachine, CIRCCmd* pCmd )
{
	Reg1 char *s;
	Reg2 int i, j = 5;
	char *p = NULL, *sbuf;

	CIRCLRClient* pLRClient;

	if ( pCmd->m_iQtyArgs < 2 )
	{
		SendToOne( pMachine, IRCErrorString( ERR_NEEDMOREPARAMS ),
			m_szHost, pCmd->m_szArgs[0], "USERHOST");
		return;
	}

	sbuf = CIRCTools::sprintf_irc( m_szSendBuffer, IRCReplyString( RPL_USERHOST ),
		m_szHost, pCmd->m_szArgs[1] );
	sbuf = &m_szSendBuffer[0] + strlen( m_szSendBuffer ) - 2;
	for (i = j, s = CIRCTools::strtoken(&p, pCmd->m_szArgs[1], " ");
			i && s; s = CIRCTools::strtoken(&p, (char *)NULL, " "), i--)
	{
		if ( ( pLRClient = HuntClientByNick( s ) ) )
		{
			if (i < j)
				*sbuf++ = ' ';
			CIRCTools::sprintf_irc(sbuf, "%s%s=%c%s@%s\r\n",
				pLRClient->m_szNick, IsAnOper( pLRClient ) ? "*" : "",
				( strlen( pLRClient->m_szAway ) > 0 ) ?  '-' : '+',
//				pLRClient->m_szUser, CIRCTools::inetntoa( pLRClient->m_iaClientIP ) );
				pLRClient->m_szUser, pLRClient->m_szHost );
		}
		else
		{
			if (i < j)
				SendBufferToOne( pMachine );
			SendToOne( pMachine, IRCErrorString( ERR_NOSUCHNICK ),
				m_szHost, pCmd->m_szArgs[0], s);
			sbuf = CIRCTools::sprintf_irc( m_szSendBuffer, IRCReplyString( RPL_USERHOST ),
				m_szHost, pCmd->m_szArgs[0] );
			j = i - 1;
		}
		if (j)
			SendBufferToOne( pMachine );
	}
	return;
}

void CIRCLocalServer::Do_OPER( CIRCLRClient *pLClient, CIRCCmd* pCmd )
{
	char szUser[HOSTLEN];
	char szPassword[PASSWDLEN];

	strcpy( szUser, pCmd->m_szArgs[1] );
	strcpy( szPassword, pCmd->m_szArgs[2] );

	if ( !strcmp( m_AdminInfo.m_szUser, szUser ) )
	{
		if ( !strcmp( m_AdminInfo.m_szPassword, szPassword ) )
		{
			// Give ops
			m_Opers.Add( pLClient );
			pLClient->m_dwFlags |= FLAGS_OPER;
			SendToOne( pLClient, IRCReplyString( RPL_YOUREOPER ),
				GetHost(), pLClient->m_szNick );
			return;
		}
	}
    SendToOne( pLClient, IRCErrorString( ERR_NEEDMOREPARAMS ),
		GetHost(), pLClient->m_szNick, "OPER");
	return;
}

bool CIRCLocalServer::OnLClientRequest( CIRCLRClient *pLClient, BYTE * pRequest, int len )
{
	CIRCCmds IRCCmds;
	if ( ParseRequest( pRequest, len, &IRCCmds ) )

	for( int iCommand = 0; iCommand < IRCCmds.m_iQtyCmds; iCommand++ )
	{
		CIRCCmd* pCmd = &IRCCmds.m_Cmd[iCommand];
		IRC_TYPE wType = (IRC_TYPE)FindTable( pCmd->m_szArgs[0], CIRCTools::sm_CmdTbl, COUNTOF(CIRCTools::sm_CmdTbl));
		switch ( wType )
		{
			case IRC_PRIVATE:
			{
				// RFC 1459, 4.4.1 "Privmsg Message"
				// COMMAND: PRIVMSG <receiver>{,<receiver>} <text to be sent>
				// Replies: ERR_NORECIPIENT, ERR_NOTEXTTOSEND,
				//			ERR_CANNOTSENDTOCHAN, ERR_NOTOPLEVEL,
				//			ERR_WILDTOPLEVEL, ERR_TOOMANYTARGETS,
				//			ERR_NOSUCHNICK, RPL_AWAY
				for(int i = 1; i < pCmd->m_iQtyArgs; i++)
				{
					char szRecipient[CHANNELLEN];
					strcpy( szRecipient, pCmd->m_szArgs[i] );
					if ( szRecipient[0] == '$' )
					{
						// This is a server mask, eat it for now
						continue;
					}
					// Check to see if these are channels, or host masks
					if ( IsChannelName( szRecipient ) )
					{
						// Treat them all as channels for now
						CIRCChannel* pChannel = HuntChannel( szRecipient, "", NULL );
						if ( pChannel )
						{
							// Channel Broadcast
							// example to a channel: :Westy2!noneya@10.1.1.1 PRIVMSG #Channel :hello?
							SendToChannel( pChannel, pLClient, NULL, ":%s PRIVMSG %s :%s\r\n",
								pLClient->GetNickUserIP(), szRecipient, pCmd->m_szTrailing );
							continue;
						}
						else
						{
							SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ),
								GetHost(), pLClient->m_szNick, szRecipient );
							continue;
						}
					}
					// Must be a user
					CIRCLRClient* pLClientTo = HuntClientByNick( szRecipient );
					if ( pLClientTo )
					{
						SendToOne( pLClientTo, ":%s PRIVMSG %s :%s\r\n",
							pLClient->GetNickUserIP(), pLClientTo->m_szNick, pCmd->m_szTrailing );
						continue;
					}
					else
					{
						SendToOne( pLClient, IRCErrorString( ERR_NOSUCHNICK ),
							GetHost(), szRecipient );
						continue;
					}
				}
				break;
			}
			case IRC_WHO:
			{
				Do_WHO( pLClient, pCmd->m_szArgs[1], pCmd->m_szArgs[2] );
				break;
			}
			case IRC_WHOIS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_WHOWAS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_USER:
			{
				// Just eat it
				break;
			}
			case IRC_NICK:
			{
				// RFC 1459, 4.1.2 "Nick Message"
				// COMMAND: NICK <nickname> [ <hopcount> ]
				// If hopcount is supplied by a client, it must be ignored
				//
				// Servers will send us this (with a hopcount param)
				//	when they introduce a (possibly) new nick to the network
				// replies: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTERED,
				//			ERR_NICKCOLLISION
				//

				// Is this a new client?
				// Notice the position (or lack of) the the colon in the examples below
				char szNick[NICKLEN];
				// example: NICK :WestyAgain
				// They're changing their NICK
				strcpy( szNick, pCmd->m_szTrailing );
				// Is this a valid looking nick?
				int iLen = AnalyzeNick( szNick );
				// If iLen == 0, then it's not valid, so reject it
				if ( iLen == 0 )
				{
					// It is, send out the errorneuos nick message
					// example: :irc.sphereserver.com 433 * Revenge :Nickname is already in use.
					SendToOne( pLClient, IRCErrorString( ERR_ERRONEUSNICKNAME ),
						GetHost(), pCmd->m_szArgs[1] );
					break;
				}
				Send_NICK( pLClient, szNick );
				break;
			}
			case IRC_SERVER:
			{
				// Just eat it
				break;
			}
			case IRC_LIST:
			{
				// RFC 1459, 4.2.6 "List Message"
				// COMMAND: LIST [<channel>{,<channel>} [<server>]]
				// REPLIES:	ERR_NOSUCKSERVER. RPL_LISTSTART, RPL_LIST
				//			RPL_LISTEND

				Send_RPL_LISTSTART( pLClient );

				if ( pCmd->m_iQtyArgs > 1 ) // Parse out the channel names
				{
					for( int i = 1; i < pCmd->m_iQtyArgs; i++ )
						Send_RPL_LIST( pLClient, 0, 10000, pCmd->m_szArgs[i] );
				}
				else
					Send_RPL_LIST( pLClient, 0, 10000, NULL);

				Send_RPL_LISTEND( pLClient );
				break;
			}
			case IRC_TOPIC:
			{
				// RFC 1459, 4.2.4 "Topic Message"
				// COMMAND: TOPIC <channel> [<topic>]
				// REPLIES:	ERR_NEEDMOREPARAMS, ERR_NOTONCHANNEL,
				//			RPL_NOTOPIC, RPL_TOPIC, ERR_CHANOPRIVSNEEDED
				// example from client: TOPIC #Channel :this is my topic

				// Are they setting it or do they want it?
				char szTopic[TOPICLEN];
				strcpy( szTopic, pCmd->m_szTrailing );
				CIRCChannel* pChannel = HuntChannel( pCmd->m_szArgs[1], "", NULL );
				if ( pChannel )
				{
					if ( strlen( pCmd->m_szTrailing ) > 0 )
						// Set the topic
						pChannel->SetTopic( pLClient, szTopic );
					else
						// Give just them the topic
						Send_RPL_TOPIC( pLClient, pChannel );
				}
				else
				{
					SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ),
						GetHost(), pLClient->m_szNick, pCmd->m_szArgs[1] );
				}
				break;
			}
			case IRC_INVITE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_VERSION:
			{
				Send_VERSION( pLClient );
				break;
			}
			case IRC_QUIT:
			{
				// Trailing is the clients reason for leaving
				ClientQuit( pLClient, pCmd->m_szTrailing);
				return false;
			}
			case IRC_SQUIT:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_KILL:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_INFO:
			{
				Send_INFO( pLClient );
				break;
			}
			case IRC_LINKS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_STATS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_HELP:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_ERROR:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_AWAY:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_CONNECT:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_UPING:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_MAP:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_PING:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_PONG:
			{
				time_t tNow;
				time( &tNow );
				pLClient->m_dwFlags &= ~FLAGS_PINGSENT;
				pLClient->m_tLastTime = tNow;
				break;
			}
			case IRC_OPER:
			{
				Do_OPER( pLClient, pCmd );
				break;
			}
			case IRC_PASS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_WALLOPS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_TIME:
			{
				Send_TIME( pLClient );
				break;
			}
			case IRC_SETTIME:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_RPING:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_RPONG:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_NAMES:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_ADMIN:
			{
				Send_ADMIN( pLClient );
				break;
			}
			case IRC_TRACE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_NOTICE:
			{
				Do_NOTICE( pLClient, pCmd );
				break;
			}
			case IRC_WALLCHOPS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_CPRIVMSG:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_CNOTICE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_JOIN:
			{
				// RFC 1459, 4.2.1 "Join Message"
				// COMMAND: JOIN <channel>{,<channel>} [<key>{,<key>}]
				// REPLIES: ERR_NEEDMOREPARAMS, ERR_BANNEDFROMCHAN, ERR_INVITEONLYCHAN
				//			ERR_BADCHANNELKEY, ERR_CHANNELISFULL, ERR_BADCHANMASK,
				//			ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS, RPL_TOPIC

				// We might have channel keys here, so find out how many channel
				//	names are given to us here
				// Channel names beginging with '#' are system wide MODE channels
				// Channel names beginging with '+' are system wide modeless channels
				// Channel names beginging with '&' are non system wide mode channels
				char szChName[CHANNELLEN];
				int iChannels = 0;
				int i;
				for(i = 1; i < pCmd->m_iQtyArgs; i++)
				{
					strcpy( szChName, pCmd->m_szArgs[i] );
					if ( IsChannelName( szChName ) )
						iChannels++;
				}
				int iKeys = pCmd->m_iQtyArgs - iChannels - 1;
				// Now we can match up any keys with their respective channels
				for(i = 0; i < iChannels; i++)
				{
					char szKey[KEYLEN];
					strcpy( szChName, pCmd->m_szArgs[i+1] );
					if ( i < iKeys )
						strcpy( szKey, pCmd->m_szArgs[i+iChannels+1] );
					else
						szKey[0] = 0;

					JoinChannel( szChName, szKey, pLClient->m_szNick, NULL );
				}
				break;
			}
			case IRC_PART:
			{
				// RFC 1459, 4.2.2 "Part Message"
				// COMMAND: PART <channel>{,<channel>}
				// REPLIES:	ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL

				char szChName[CHANNELLEN];
				if ( pCmd->m_iQtyArgs == 1 )
				{
					SendToOne( pLClient, IRCErrorString( ERR_NEEDMOREPARAMS ),
						GetHost(), pCmd->m_szArgs[0] );
					break;
				}
				for(int i = 1; i < pCmd->m_iQtyArgs; i++)
				{
					strcpy( szChName, pCmd->m_szArgs[i] );
					PartChannel( pLClient, szChName );
				}
				break;
			}
			case IRC_LUSERS:
			{
				Send_LUSERS( pLClient );
				break;
			}
			case IRC_MOTD:
			{
				Send_MOTD( pLClient );
				break;
			}
			case IRC_MODE:
			{
				// RFC 1459, 4.2.3 "Mode Message"
				// COMMAND: MODE <channel>{[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
				// REPLIES:	ERR_NEEDMOREPARAMS, RPL_CHANNELMODEIS,
				//			ERR_CHANOPRIVNEEDED, ERR_NOSUCHNICK,
				//			ERR_NOTONCHANNEL, ERR_KEYSET,
				//			RPL_BANLIST, RPL_NOSUCHCHANNEL,
				//
				//			ERR_USERSDONTMATCH, RPL_UMODEIS
				//			ERR_UMODEUNKNOWNFLAG

				// If a channel is specified, it's a CMODE
				// else it's a UMODE
				if ( IsChannelName( pCmd->m_szArgs[1] ) )
					Do_CMODE( pCmd, pLClient );
				else
					Do_UMODE( pCmd, pLClient );
				break;
			}
			case IRC_KICK:
			{
				// RFC 1459, 4.2.8 "Kick Message"
				// COMMAND: KICK <channel> <user> [<comment>]
				// REPLIES:	ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL,
				//			ERR_BADCHANMASK,	ERR_CHANOPRIVSNEEDED,
				//			ERR_NOTONCHANNEL

				// ppargs[1] = channel kicked from
				// ppargs[2] = who's being kicked
				// trailing  = reason
				KickFromChannel( pLClient, pCmd->m_szArgs[1], pCmd->m_szArgs[2], pCmd->m_szTrailing );
				break;
			}
			case IRC_USERHOST:
			{
				Do_USERHOST( pLClient, pCmd );
				break;
			}
			case IRC_USERIP:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_ISON:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_SQUERY:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_SERVLIST:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_SERVSET:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_REHASH:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_RESTART:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_CLOSE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_DIE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_HASH:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_DNS:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_SILENCE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_GLINE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_BURST:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_CREATE:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_END_OF_BURST:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			case IRC_END_OF_BURST_ACK:
			{
				SendToOne( pLClient, "%s :Sorry the %s command isn't working yet.\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO], CIRCTools::sm_CmdTbl[wType] );
				break;
			}
			default:
			{
				SendToOne( pLClient, "%s :What's that mean?\r\n",
					CIRCTools::sm_CmdTbl[IRC_INFO] );
				break;

			}
		}
	}
	pRequest[len] = '\0';
	return true;
}

void CIRCChannel::GetModes( CIRCMachine* pMachine, char* szModeBuf, char* szParaBuf )
{
	CIRCServer* pServer = dynamic_cast <CIRCServer*>( pMachine );
	CIRCLRClient* pLRClient = dynamic_cast <CIRCLRClient*>( pMachine );

	*szModeBuf++ = '+';
	if ( m_Mode.m_dwMode & CHMODE_SECRET )
		*szModeBuf++ = 's';
	if ( m_Mode.m_dwMode & CHMODE_PRIVATE )
		*szModeBuf++ = 'p';
	if ( m_Mode.m_dwMode & CHMODE_MODERATED )
		*szModeBuf++ = 'm';
	if ( m_Mode.m_dwMode & CHMODE_TOPICLIMIT )
		*szModeBuf++ = 't';
	if ( m_Mode.m_dwMode & CHMODE_INVITEONLY )
		*szModeBuf++ = 'i';
	if ( m_Mode.m_dwMode & CHMODE_NOPRIVMSGS )
		*szModeBuf++ = 'n';
	if ( m_Mode.m_iLimit > 0 )
	{
		*szModeBuf++ = 'l';
		CIRCTools::sprintf_irc( szParaBuf, "%d", m_Mode.m_iLimit );
	}
	if ( strlen( m_Mode.m_szKey ) > 0 && m_Mode.m_dwMode & CHMODE_KEY )
	{
		*szModeBuf++ = 'k';
		if ( IsChOp( pLRClient ) || pServer )
		{
			if ( m_Mode.m_iLimit )
				strcat( szParaBuf, " ");
			strcat( szParaBuf, m_Mode.m_szKey );
		}
	}
	*szModeBuf = '\0';
	return;
}

void CIRCLocalServer::Do_CMODE( CIRCCmd* pCmd, CIRCLRClient* pLClient )
{
	char szChName[CHANNELLEN+1];

	strcpy( szChName, pCmd->m_szArgs[1] );
	CIRCChannel* pChannel = HuntChannel( szChName, "", NULL );
	if ( !pChannel )
	{
		SendToOne( pLClient, IRCErrorString( ERR_NOSUCHCHANNEL ),
			GetHost(), pLClient->m_szNick, szChName );
		return;
	}
	// Handle a simple mode request here
	if ( pCmd->m_iQtyArgs < 3)
	{
		char szModeBuf[MODEBUFLEN];
		char szParaBuf[MODEBUFLEN];
		szModeBuf[1] = '\0';
		szParaBuf[1] = '\0';
		pChannel->GetModes( pLClient, szModeBuf, szParaBuf );
		// :ntserver.electrotechdesign.com 324 Sam #Channel +l 65535
		// :irc1.sphereserver.com 324 Sam #Channel +
		SendToOne( pLClient, IRCReplyString( RPL_CHANNELMODEIS ),
			m_szHost, pLClient->m_szNick, pChannel->m_szChName, szModeBuf, szParaBuf );
		// :irc1.sphereserver.com 329 Sam #Channel 965710649
		SendToOne( pLClient, IRCReplyString( RPL_CREATIONTIME ),
			m_szHost, pLClient->m_szNick, pChannel->m_szChName, pChannel->m_tCreationTime );
		return;
	}
	// Let the channel do it's own mode stuff
	int fBadOp = 0;
	char szModeBuf[MODEBUFLEN];
	char szParaBuf[MODEBUFLEN];
	int iRet = pChannel->Do_MODE( pCmd, pLClient, szModeBuf, szParaBuf, &fBadOp );
	if ( strlen( szModeBuf ) > 0 )
	{
		// TODO: Send out the new mode stuff to the other server first

		szModeBuf[0] = 0;
		szParaBuf[0] = 0;
		pChannel->GetModes( pLClient, szModeBuf, szParaBuf );	// We can corrupt them now that they're sent out
		SendToOne( pLClient, IRCReplyString( RPL_CHANNELMODEIS ),
			m_szHost, pLClient->m_szNick, pChannel->m_szChName, szModeBuf, szParaBuf );
	}

	return;
}

void _cdecl CIRCLocalServer::SendToOperators( CIRCMachine *pSkip, char *pattern, ... )
{
	va_list vl;
	va_start(vl, pattern);
	va_end(vl);
}

/*
 *  get_client_name
 *	 Return the name of the client for various tracking and
 *	 admin purposes. The main purpose of this function is to
 *	 return the "socket host" name of the client, if that
 *    differs from the advertised name (other than case).
 *    But, this can be used to any client structure.
 *
 *    Returns:
 *	"name[user@ip#.port]" if 'showip' is true;
 *	"name[sockethost]", if name and sockhost are different and
 *	showip is false; else
 *	"name".
 *
 *  NOTE 1:
 *    Watch out the allocation of "nbuf", if either sptr->name
 *    or sptr->sockhost gets changed into pointers instead of
 *    directly allocated within the structure...
 *
 *  NOTE 2:
 *    Function return either a pointer to the structure (sptr) or
 *    to internal buffer (nbuf). *NEVER* use the returned pointer
 *    to modify what it points!!!
 */
char* CIRCMachine::GetClientName( bool fShowIP )
{
	static char szNBuf[HOSTLEN * 2 + USERLEN + 5];

	if ( !g_IRCLocalServer.IsMyClient( this ) )
	{
		if ( fShowIP )
			CIRCTools::sprintf_irc( szNBuf, "%s[%s@%s]",
				m_szNick, ( !( m_dwFlags & FLAGS_GOTID ) ) ? "" :
				m_szUser, CIRCTools::inetntoa( m_iaClientIP ) );
		else
		{
			if ( strcmp( m_szNick, m_szHost ) )
				CIRCTools::sprintf_irc( szNBuf, "%s[%s]",
				m_szNick, m_szHost );
			else
				CIRCTools::sprintf_irc( szNBuf, "%s", m_szNick );
		}
		return szNBuf;
	}
	CIRCTools::sprintf_irc( szNBuf, "%s", m_szNick );
	return szNBuf;
}

DWORD CIRCLocalServer::m_dwUserModes[] =
{
	FLAGS_OPER,			'o',
	FLAGS_LOCOP,		'O',
	FLAGS_INVISIBLE,	'i',
	FLAGS_WALLOP,		'w',
	FLAGS_SERVNOTICE,	's',
	FLAGS_DEAF,			'd',
	FLAGS_CHSERV,		'k',
	0,					0
};

void CIRCLocalServer::Do_UMODE( CIRCCmd* pCmd, CIRCMachine* pMachine )
{
	CIRCLRClient* pClients[2];
	for( int xyz = 0; xyz < m_LClients.GetCount(); xyz++ )
	{
		pClients[xyz] = m_LClients[xyz];
	}
	if ( pCmd->m_iQtyArgs < 2 )
	{
		SendToOne( pMachine, IRCErrorString( ERR_NEEDMOREPARAMS ),
			GetHost(), pMachine->m_szNick, "MODE" );
		return;
	}

	CIRCLRClient* pTarget = HuntClientByNick( pCmd->m_szArgs[1] );
	if ( !pTarget )
	{
		if ( !IsMyClient( pMachine ) )
			SendToOne( pMachine, IRCErrorString( ERR_NEEDMOREPARAMS ),
				GetHost(), pMachine->m_szNick, pCmd->m_szArgs[1] );
		return;
	}

	CIRCServer* pServer = dynamic_cast <CIRCServer*>( pMachine );

	if ( pServer || ( pTarget != pMachine ) )
	{
		if ( pServer )
			SendToOperators( NULL, ":%s WALLOPS :MODE for User %s From %s!%s",
				GetHost(), pMachine->m_szNick,
				pTarget->GetClientName( FALSE ), pTarget->m_szNick );
		else
			SendToOne( pMachine, IRCErrorString( ERR_USERSDONTMATCH ),
				GetHost(), pMachine->m_szNick );
		return;
	}

	Reg2 DWORD *pi;
	Reg3 char *szMode;
	char szBuf[BUFSIZE];
	char szBuf2[BUFSIZE];

	DWORD dwFlag;

	if ( pCmd->m_iQtyArgs < 3)
	{
		szMode = szBuf;
		*szMode++ = '+';
		for ( pi = m_dwUserModes; ( dwFlag = *pi ) && ( szMode - szBuf < BUFSIZE - 4); pi += 2)
			if ( pTarget->m_dwFlags & dwFlag )
				*szMode++ = (char)( *( pi + 1) );
		*szMode = '\0';
		SendToOne( pMachine, IRCReplyString( RPL_UMODEIS ),
			GetHost(), pMachine->m_szNick, szBuf );
		if ( ( pMachine->m_dwFlags & FLAGS_SERVNOTICE ) && IsMyClient( pMachine ) &&
				pMachine->m_dwSMask != ( IsOper( pMachine ) ? SMODE_OPERDEFAULT : SMODE_DEFAULT ) )
			SendToOne( pMachine, IRCReplyString( RPL_SNOMASK ),
				GetHost(), pMachine->m_szNick, pMachine->m_dwSMask, pMachine->m_dwSMask );
		return;
	}

	DWORD dwTmpMask = 0;
	// find flags already set for user
	DWORD dwSetFlags = 0;
	int iIndex = 0;
	szMode = (char*)(m_dwUserModes + iIndex + 1);
	while ( *szMode != 0 )
	{
		dwFlag = m_dwUserModes[iIndex];
		if ( pMachine->m_dwFlags & dwFlag)
			dwSetFlags |= dwFlag;
		iIndex += 2;
		szMode = (char*)(m_dwUserModes + iIndex + 1);
	}

	if ( IsMyClient( pMachine ) )
		dwTmpMask = pMachine->m_dwSMask;

	Reg3 char *p;
	DWORD dwType = 0;
	bool fSMaskGiven = false;
	// parse mode change string(s)
	strcpy( szBuf2, &pCmd->m_szArgs[2][0] );
	for ( p = szBuf2; *p; p++)		// p is changed in loop too
	{
		for ( szMode = p; *szMode; szMode++)
		{
			switch ( *szMode )
			{
				case '+':
					dwType = MODE_ADD;
					break;
				case '-':
					dwType = MODE_DEL;
					break;
				case 's':
					if ( *( p + 1 ) && CIRCTools::IsSMask( p + 1 ) )
					{
						fSMaskGiven = true;
						dwTmpMask = CIRCTools::MakeSMask( dwTmpMask, ++p, dwType );
						dwTmpMask &= ( IsOper( pMachine ) ? SMODE_ALL : SMODE_USER);
					}
					else
						dwTmpMask = ( dwType == MODE_ADD ) ?
							(IsOper( pMachine ) ? SMODE_OPERDEFAULT : SMODE_DEFAULT) : 0;
					if ( dwTmpMask )
						pMachine->m_dwFlags |= FLAGS_SERVNOTICE;
					else
						pMachine->m_dwFlags &= ~FLAGS_SERVNOTICE;
					break;
				// We may not get these, but they shouldnt be in default:
				case ' ':
				case '\n':
				case '\r':
				case '\t':
					break;
				default:
					int iIndex = 0;
					Reg3 char *szMode2;
					szMode2 = (char*)(m_dwUserModes + iIndex + 1);
					dwFlag = m_dwUserModes[iIndex];
					while ( *szMode2 != 0 )
					{
						if ( *szMode == *szMode2 )
						{
							if ( dwType == MODE_ADD )
							{
								pMachine->m_dwFlags = ( pMachine->m_dwFlags | dwFlag );
							}
							else if ( ( dwFlag & ( FLAGS_OPER | FLAGS_LOCOP ) ) )
							{
								pMachine->m_dwFlags &= ~( FLAGS_OPER | FLAGS_LOCOP );
								if ( IsMyClient( pMachine ) )
									dwTmpMask = pMachine->m_dwSMask & ~SMODE_OPER;
							}
							// allow either -o or -O to reset all operator status's...
							else
							{
								pMachine->m_dwFlags &= ~dwFlag;
							}
							break;
						}
						iIndex += 2;
						dwFlag = m_dwUserModes[iIndex];
						szMode2 = (char*)(m_dwUserModes + iIndex + 1);
					}
					if ( dwFlag == 0 && IsMyClient( pMachine ) )
						SendToOne( pMachine, IRCErrorString( ERR_UMODEUNKNOWNFLAG ),
							GetHost(), pTarget->m_szNick );
					break;
			}
		}
	}

	// Stop users making themselves operators too easily:
	if ( !( dwSetFlags & FLAGS_OPER) && IsOper( pMachine ) && !pServer )
		ClearOper( pMachine );
	if ( !( dwSetFlags & FLAGS_LOCOP) && IsLocOp( pMachine ) && !pServer )
		pMachine->m_dwFlags &= ~FLAGS_LOCOP;
	// new umode; servers can set it, local users cannot;
	// prevents users from /kick'ing or /mode -o'ing
	if ( !( dwSetFlags & FLAGS_CHSERV ) && !pServer )
		pMachine->m_dwFlags &= ~FLAGS_CHSERV;

	// Compare new flags with old flags and send string which
	// will cause servers to update correctly.
	Send_UMODE_Out( pMachine, dwSetFlags);

	if ( IsMyClient( pMachine ) )
	{
		if ( dwTmpMask != pMachine->m_dwSMask )
			SetSMask( pMachine, dwTmpMask, SMODE_SET);
		if ( pMachine->m_dwSMask && fSMaskGiven )
			SendToOne( pMachine, IRCReplyString( RPL_SNOMASK ),
				GetHost(), pMachine->m_szNick,
				pMachine->m_dwSMask, pMachine->m_dwSMask );
	}

	return;
}

// Send the MODE string for user (user) to connection cptr
void CIRCLocalServer::Send_UMODE( CIRCMachine* pTo, CIRCMachine* pFrom, DWORD dwOld, DWORD dwSendMask )
{
	Reg1 DWORD *s, dwFlag;
	Reg2 char *m;
	int dwType = MODE_NULL;

	// Build a string in umode_buf to represent the change in the user's
	// mode between the new (sptr->flag) and 'old'.

	m = m_szSendUModeBuffer;
	*m = '\0';
	for ( s = m_dwUserModes; ( dwFlag = *s ); s += 2 )
	{
		if ( IsMyClient( pFrom ) && !( dwFlag & dwSendMask) )
			continue;
		if (( dwFlag & dwOld) && !( pTo && pTo->m_dwFlags & dwFlag ) )
		{
			if ( dwType == MODE_DEL)
				*m++ = *(s + 1);
			else
			{
				dwType = MODE_DEL;
				*m++ = '-';
				*m++ = *(s + 1);
			}
		}
		else if ( !( dwFlag & dwOld ) && ( pFrom->m_dwFlags & dwFlag))
		{
			if ( dwType == MODE_ADD )
				*m++ = *(s + 1);
			else
			{
				dwType = MODE_ADD;
				*m++ = '+';
				*m++ = *(s + 1);
			}
		}
	}
	*m = '\0';
	CIRCLRClient* pLRClient = dynamic_cast <CIRCLRClient*>( pFrom );
	if ( strlen(m_szSendUModeBuffer) > 0 && pTo )
		SendToOne( pTo, ":%s MODE %s :%s\r\n",
			pFrom->m_szNick, pFrom->m_szNick, m_szSendUModeBuffer );
	return;
}

void CIRCLocalServer::Send_UMODE_Out( CIRCMachine* pMachine, DWORD dwSetFlags)
{

	Send_UMODE(NULL, pMachine, dwSetFlags, SEND_UMODES);

	if ( strlen( m_szSendUModeBuffer ) > 0 )
	{
		int i;
		for( i = 0; i < m_UplinkServers.GetCount(); i++ )
		{
			SendToOne( m_UplinkServers[i], ":%s MODE %s :%s\r\n",
				m_UplinkServers[i]->m_szNick, m_UplinkServers[i]->m_szServerName,
				m_szSendUModeBuffer );
		}
		for( i = 0; i < m_DownlinkServers.GetCount(); i++ )
		{
/*
			SendToOne( m_DownlinkServers[i], ":%s MODE %s :%s\r\n",
				m_DownlinkServers[i]->m_szNick, m_DownlinkServers[i]->m_szServerName,
				m_DownlinkServers );
*/
		}
	}

	if ( pMachine && IsMyClient( pMachine ) )
		Send_UMODE( pMachine, pMachine, dwSetFlags, ALL_UMODES);

	return;
}

void CIRCLocalServer::SetSMask( CIRCMachine* pMachine, DWORD dwNewMask, DWORD dwType )
{
	DWORD dwOldMask, dwDiffMask;

	dwOldMask = pMachine->m_dwSMask;

	if ( dwType == SMODE_ADD)
		dwNewMask |= dwOldMask;
	else if ( dwType == SMODE_DEL )
		dwNewMask = dwOldMask & ~dwNewMask;
	else if ( dwType != SMODE_SET)	// absolute set, no math needed
    SendToOperators( NULL, "setsnomask called with %d ?!", dwType );

	dwNewMask &= ( IsAnOper( pMachine ) ? SMODE_ALL : SMODE_USER);
	dwDiffMask = dwOldMask ^ dwNewMask;

	pMachine->m_dwSMask = dwNewMask;
}

void _cdecl CIRCLocalServer::SendToOne( CIRCMachine *pMachine, char* szPattern, ... )
{
	va_list vlList;
	va_start( vlList, szPattern );
	CIRCTools::vsprintf_irc( m_szSendBuffer, szPattern, vlList );
	SendBufferToOne( pMachine );
	va_end( vlList );
}

void CIRCLocalServer::SendBufferToOne( CIRCMachine *pMachine )
{
	// TODO: Put this stuff on a stack or something so we can balance, burst and throttle all
	// our clients and servers

	// Is this an uplink?
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		int iLenRet = pUplink->m_Socket.Send( m_szSendBuffer, strlen( m_szSendBuffer ) );
		return;
	}

	pMachine->GetClient()->m_Socket.Send( m_szSendBuffer, strlen( m_szSendBuffer ) );
}

void _cdecl CIRCLocalServer::SendToChannel( CIRCChannel *pChannel, CIRCLRClient* pFromClient, CIRCServer* pServer, char* szPattern, ... )
{
	va_list vlList;
	va_start( vlList, szPattern );
	CIRCTools::vsprintf_irc( m_szSendBuffer, szPattern, vlList );

	for( int i = 0; i < pChannel->m_ChanMembers.GetCount(); i++)
	{
		CIRCChannelMember* pChanMember = pChannel->m_ChanMembers[i];
		CIRCLRClient* pLClient = pChanMember->m_pLRClient;
		if ( IsMyClient( pLClient ) && pLClient != pFromClient /*&& pLClient->GetServer() != pServer*/ )
			SendBufferToOne( pLClient );
	}

	va_end(vlList);
	return;
}

void _cdecl CIRCLocalServer::SendToServers( CIRCServer* pServer, char* szPattern, ... )
{
	va_list vlList;
	va_start( vlList, szPattern );
	CIRCTools::vsprintf_irc( m_szSendBuffer, szPattern, vlList );

	// Send this to all uplinks and downlinks i guess
	int i;
	for( i = 0; i < m_UplinkServers.GetCount(); i++ )
	{
		CIRCUplinkServer* pUplink = m_UplinkServers[i];
		if ( pUplink->IsAuthenticated() && pUplink != pServer )
			SendBufferToOne( m_UplinkServers[i] );
	}
	for( i = 0; i < m_DownlinkServers.GetCount(); i++ )
	{
		CIRCDownlinkServer* pDownlink = m_DownlinkServers[i];
		if ( pDownlink->IsAuthenticated() && pDownlink != pServer )
			SendBufferToOne( m_DownlinkServers[i] );
	}
	va_end( vlList );
}

void CIRCLocalServer::NewConnection( CIRCMachine *pMachine )
{
	// New server connections need to be brought in sync with the network
	CIRCUplinkServer* pUplink = dynamic_cast <CIRCUplinkServer*>( pMachine );
	if ( pUplink )
	{
		return;
	}

	CIRCDownlinkServer* pDownlink = dynamic_cast <CIRCDownlinkServer*>( pMachine );
	if ( pDownlink )
	{
		return;
	}

	// Clients connections get sent some info about us
	// Let's send the first connection messages
	// (who we are, what we are, etc., etc.)
	CIRCLRClient *pLClient = dynamic_cast <CIRCLRClient*>(pMachine);

	SendToOne( pMachine, IRCReplyString( RPL_WELCOME ),
		m_szHost, pLClient->m_szNick, pLClient->m_szNick );

	SendToOne( pMachine, IRCReplyString( RPL_YOURHOST ),
		m_szHost, pLClient->m_szNick, m_szHost, m_szName, m_szVersion);

	SendToOne( pMachine, IRCReplyString( RPL_CREATED ),
		m_szHost, pLClient->m_szNick, __DATE__ " at " __TIME__ " EST" );

	SendToOne( pMachine, IRCReplyString( RPL_MYINFO ),
		m_szHost, pLClient->m_szNick, m_szHost, m_szVersion);

	return;
};

/*
 * CIRCNick
 *
 */

CIRCNick::CIRCNick()
{
	return;
}

CIRCNick::~CIRCNick()
{
	return;
}

CIRCNickTrack* CIRCNick::FindHistory( time_t t_TimeLimit )
{
	for( int i = m_History.GetCount(); i >= 0 ; i--)
	{
		if ( m_History[i]->m_tEnd > t_TimeLimit )
			return m_History[i];
	}
	return NULL;
}

void CIRCNick::NewOwner( CIRCLRClient *pLRClient )
{
	// Look for a prior record first
	CIRCNickTrack * pNickTrack = NULL;
	for(int i = 0; i < m_History.GetCount(); i++)
	{
		if ( m_History[i]->m_iaClientIP.s_addr == pLRClient->m_iaClientIP.s_addr )
		{
			pNickTrack = m_History[i];
			// Remove it out of the history
			m_History.RemoveAt( i );
			break;
		}
	}

	// Make this client the current owner
	m_pLRClient = pLRClient;

	// If we can't re-use an old one, then make a new one
	if ( !pNickTrack )
	{
		pNickTrack = new CIRCNickTrack();
		// Fill out the fields fields
		strcpy( pNickTrack->m_szUser, pLRClient->m_szUser );
		strcpy( pNickTrack->m_szHost, pLRClient->m_szHost );
		strcpy( pNickTrack->m_szRealName, pLRClient->m_szRealName );
		pNickTrack->m_iaClientIP = pLRClient->m_iaClientIP;
	}

	// Fill out or update these
	time( &pNickTrack->m_tStart );
	pNickTrack->m_tEnd = -1;

	// Add or re-add them to the history
	m_History.Add( pNickTrack );

	// if we somehow hit NICKNAMEHISTORYLENGTH, then
	// get rid of the oldest
	while ( m_History.GetCount() > NICKNAMEHISTORYLENGTH )
	{
		m_History.RemoveAt( 0 );
	}
}

void CIRCNick::ClearHistory()
{
	m_History.RemoveAll();
	return;
}

void CIRCNick::ClearOwner()
{
	// Current owner left, got killed, kicked, changed mode, etc etc,
	if ( m_History.GetCount() > 0 )
	{
		CIRCNickTrack* pTrack = m_History[m_History.GetCount()-1];
		if ( pTrack )
			time( &pTrack->m_tEnd );
	}
	m_pLRClient = NULL;
}

bool CIRCNick::IsOwned()
{
	// Is someone using this nick right now?
	return m_pLRClient != NULL;
}

bool CIRCNick::CanDelete()
{
	// Check the history to see if we can be deleted
	time_t tNow;
	time( &tNow );
	// If there's an owner, then don't get rid of this
	if ( m_pLRClient )
		return false;
	// If all of the limits are expired then it's ok to get rid of this
	for(int i= 0; i < m_History.GetCount(); i++ )
	{
		if ( m_History[i]->m_tEnd + KILLCHASETIMELIMIT < tNow )
			return false;
	}
	return true;
}

/*
 * CIRCChannel
 *
 */

CIRCChannel::CIRCChannel()
{
	m_Mode.m_dwMode = CHMODE_SENDTS;
	m_Mode.m_iLimit = -1;
	m_Mode.m_szKey[0] = 0;
	time( &m_tCreationTime );
	m_szTopic[0] = 0;
	m_szTopicNick[0] = 0;
	m_szChName[0] = 0;
}

int CIRCChannel::CountOperators()
{
	int iSum = 0;
	for( int i = 0; i < m_ChanMembers.GetCount(); i++ )
	{
		if ( m_ChanMembers[i]->m_dwMode & ONCHMODE_CHANOP )
			iSum++;
	}
	return iSum;
}

CIRCChannelMember* CIRCChannel::GetChannelMember( CIRCLRClient* pLRClient )
{
	for( int i = 0; i < m_ChanMembers.GetCount(); i++ )
	{
		if ( pLRClient == m_ChanMembers[i]->m_pLRClient )
			return m_ChanMembers[i];
	}
	return NULL;
}

DWORD CIRCChannel::GetOnChannelMode( CIRCLRClient* pLRClient )
{
	CIRCChannelMember* pChanMem = GetChannelMember( pLRClient );
	return ( pChanMem ? pChanMem->m_dwMode : 0 );
}

bool CIRCChannel::Do_MODE( CIRCCmd* pCmd, CIRCMachine* pMachine,
						  char* szModeBuf, char* szParamBuf, int *fBadOp )
{
	// Mode is accepted when pMachine is a channel operator
	// but also when the mode is received from a server.
	// At this point, let any MEMBER pass, so they are allowed
	// to see the bans.
	szModeBuf[0] = 0;
	szParamBuf[0] = 0;

	CIRCServer* pServer = dynamic_cast <CIRCServer*>( pMachine );
	CIRCLRClient* pLClient = dynamic_cast <CIRCLRClient*>( pMachine );
	if ( !pServer && !IsMember( pLClient ) )	// Only servers and channel members can be in here
		return 0;

	CGPtrTypeArray< CIRCModeChange* > ModeChanges;	// All the mode changes taking place

	time_t tNow;
	time( &tNow );

	if ( pCmd->m_iQtyArgs < 3 )
		return false;

	int iArg = 2;	// Index to the currently parsed parameter
	int iArgNext = 2;	// Index to the next parsed parameter (not set to the next one yet)
	int iArgs = pCmd->m_iQtyArgs - 2;	// Count of args minus the command and the channel name args
	char *pCur = &pCmd->m_szArgs[iArg][0];	// points to the currently parsed paramter
	char *pCurNext = &pCmd->m_szArgs[iArgNext][0];	// Points to other paramters which are paired with the currently parsed paramter

	// Save the original state of the channel's mode
	CIRCMode mOldMode;
	mOldMode.m_dwMode = m_Mode.m_dwMode;
	mOldMode.m_iLimit = m_Mode.m_iLimit;
	strcpy( mOldMode.m_szKey, m_Mode.m_szKey );

	bool fKeyChanged = false;
	bool fInvitedSet = false;
	bool fBanListSent = false;
	bool fLimitSet = false;
	int iLimitUsers = 0;
	bool fSecretSet = false;
	bool fPrivateSet = false;
	bool fModeratedSet = false;
	bool fNoPrivMsgsSet = false;
	bool fTopicLimitSet = false;

	// The "on channel" mode of the person doing the mode command
	DWORD dwOnChMode = GetOnChannelMode( pLClient );

	// These get resolved to nicks passed in the parameters
	CIRCLRClient* pWho = NULL;		// Could be anyone
	CIRCLRClient* pMember = NULL;	// Only if they are a channel member

	DWORD dwType = MODE_ADD;
	DWORD dwType2 = MODE_NULL;

	int iModeChangeCount = 0;		// Can't go over MAXMODEPARAMS (RFC 1459)
	while ( pCur && *pCur )
	{
		switch ( *pCur )
		{
			case '+':
			{
				dwType = MODE_ADD;
				break;
			}
			case '-':
			{
				dwType = MODE_DEL;
				break;
			}
			case 'o':	// Op/DeOp
			case 'v':	// Voice/Devoice
			{
				pCurNext = &pCmd->m_szArgs[++iArgNext][0];
				if ( BadPtr( pCurNext ) )
					iArgNext--;
				pCurNext = CIRCTools::check_string( pCurNext );
				if (	 g_IRCLocalServer.IsMyClient( pLClient ) &&
						 iModeChangeCount >= MAXMODEPARAMS )
					break;

				// Check for nickname changes and try to follow these
				// to make sure the right client is affected by the
				// mode change.
				// Even if we find a nick if we chase it, there
				// is still a reason to ignore in a special case.
				// We need to ignore the mode when:
				// - It is part of a net.burst (from a server and
				//   a MODE_ADD).
				// - The found nick is not on the right side off
				//   the net.junction.
				// This makes it so that when someone tries to
				// ride a net.break and does so with the nick of
				// someone on the otherside, he is nick collided
				// (killed) but his +o does NOT op the other person.

				if ( g_IRCLocalServer.IsMyClient( pLClient ) )
				{
					if ( !( pWho = g_IRCLocalServer.ChaseClientByNick( pCurNext ) ) )
						break;
				}
				else
				{
					CIRCNumNick* pNumNick = g_IRCLocalServer.GetNumNick( pCurNext );
					CIRCNick* pNick;
					if ( pNumNick )
						pNick = pNumNick->m_pNick;
					if ( pNick && !( pWho = pNick->m_pLRClient ) )
						break;
				}
				if ( dwType == MODE_ADD && pServer && pServer != pWho->m_pServer )
					break;
				if ( IsMember( pWho ) )
					pMember = pWho;
				if ( !pMember )
				{
					g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_USERNOTINCHANNEL ),
						g_IRCLocalServer.GetHost(), pLClient->m_szNick, pWho->m_szNick, m_szChName );
					break;
				}
				// if the user is +k, prevent a deop from local user
				if ( dwType == MODE_DEL && IsChanService( pWho ) &&
					g_IRCLocalServer.IsMyClient( pLClient ) && *pCur == 'o' )
				{
					g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_ISCHANSERVICE),
						g_IRCLocalServer.GetHost(), pLClient->m_szNick, pCurNext, m_szChName );
					break;
				}
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					pModeChange->m_Object.m_pMachine = pWho;
					iModeChangeCount++;
					if ( pServer && (!( pServer->m_dwFlags & FLAGS_TS8 ) ||
							( ( *pCur == 'o' ) && !( GetOnChannelMode( pMember ) &
							( ONCHMODE_SERVOPOK | ONCHMODE_CHANOP ) ) ) ) )
						*fBadOp = ( ( GetOnChannelMode( pMember ) & ONCHMODE_DEOPPED ) &&
							( *pCur == 'o' ) ) ? 2 : 3;
					pModeChange->m_dwFlags = ( *pCur == 'o' ) ? ONCHMODE_CHANOP : ONCHMODE_VOICE;
					pModeChange->m_dwFlags |= MODE_ADD;
					ModeChanges.Add( pModeChange );
				}
				else if ( dwType == MODE_DEL )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = (*pCur == 'o') ? ONCHMODE_CHANOP : ONCHMODE_VOICE;
					pModeChange->m_dwFlags |= MODE_DEL;
					ModeChanges.Add( pModeChange );
				}
				break;
			}
			case 'k':
			{
				pCurNext = &pCmd->m_szArgs[++iArgNext][0];
				if ( BadPtr( pCurNext ) )
					iArgNext--;
				// check now so we eat the parameter if present
				if ( fKeyChanged )
					break;
				pCurNext = CIRCTools::check_string( pCurNext );
				if ( !*pCurNext )		// nothing left in key
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) &&
						iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD)
				{
					if ( m_Mode.m_dwMode & CHMODE_KEY && !pServer )
						g_IRCLocalServer.SendToOne( pMachine, IRCErrorString( ERR_KEYSET ),
							g_IRCLocalServer.GetHost(), pLClient->m_szNick, m_szChName );
					else if ( !( m_Mode.m_dwMode & CHMODE_KEY ) || pServer )
					{
						CIRCModeChange* pModeChange = new CIRCModeChange();
						iModeChangeCount++;
						strcpy( pModeChange->m_szString, pCurNext );
						if ( strlen( pModeChange->m_szString ) > (size_t) KEYLEN )
							pModeChange->m_szString[KEYLEN] = '\0';
						pModeChange->m_dwFlags = CHMODE_KEY | MODE_ADD;
						fKeyChanged = true;
						ModeChanges.Add( pModeChange );
					}
				}
				else if ( dwType == MODE_DEL)
				{
					if ( strcmp( m_Mode.m_szKey, pCurNext ) == 0 || pServer )
					{
						CIRCModeChange* pModeChange = new CIRCModeChange();
						iModeChangeCount++;
						strcpy( pModeChange->m_szString, pCurNext );
						pModeChange->m_dwFlags = CHMODE_KEY | MODE_DEL;
						fKeyChanged = true;
						ModeChanges.Add( pModeChange );
					}
				}
				break;
			}
			case 'b':
			{
				if ( iArgs <= 4 )
				{
					if ( fBanListSent )		// Only send it once
						break;
					g_IRCLocalServer.Send_RPL_BANLIST( pLClient, pCmd->m_szArgs[1]);
					g_IRCLocalServer.Send_RPL_ENDOFBANLIST( pLClient, pCmd->m_szArgs[1] );
					fBanListSent = true;
					break;
				}
				pCurNext = &pCmd->m_szArgs[++iArgNext][0];
				if ( BadPtr( pCurNext ) )
				{
					iArgNext--;
					break;
				}
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD)
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_BAN;
					strcpy( pModeChange->m_szString, pCurNext );
					ModeChanges.Add( pModeChange );
				}
				else if ( dwType == MODE_DEL )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_BAN;
					strcpy( pModeChange->m_szString, pCurNext );
					ModeChanges.Add( pModeChange );
				}
				break;
			}
			case 'l':
			{
				 // limit 'l' to only *1* change per mode command but
				 // eat up others.
				if ( fLimitSet )
				{
					if ( dwType == MODE_ADD && --iArg > 0)
						pCurNext = &pCmd->m_szArgs[++iArgNext][0];
					if ( BadPtr( pCurNext ) )
						iArgNext--;
					break;
				}
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_DEL)
				{
					iLimitUsers = -1;
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_LIMIT;
					fLimitSet = true;
					ModeChanges.Add( pModeChange );
					break;
				}
				pCurNext = &pCmd->m_szArgs[++iArgNext][0];
				if ( BadPtr( pCurNext ) )
				{
					iArgNext--;
					g_IRCLocalServer.SendToOne( pMachine, IRCErrorString( ERR_NEEDMOREPARAMS ),
						g_IRCLocalServer.GetHost(), pMachine->m_szNick, "MODE +l" );
					break;
				}
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( !( iLimitUsers = atoi( pCurNext ) ) )
					continue;
				CIRCModeChange* pModeChange = new CIRCModeChange();
				iModeChangeCount++;
				pModeChange->m_dwFlags = MODE_ADD | CHMODE_LIMIT;
				fLimitSet = true;
				ModeChanges.Add( pModeChange );
				break;
			}
			case 's':
				if ( fSecretSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_SECRET;
					fSecretSet = true;
					ModeChanges.Add( pModeChange );
				}
				else
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_SECRET;
					fSecretSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			case 'p':
				if ( fPrivateSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_PRIVATE;
					fPrivateSet = true;
					ModeChanges.Add( pModeChange );
				}
				else
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_PRIVATE;
					fPrivateSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			case 'm':
				if ( fModeratedSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_MODERATED;
					fModeratedSet = true;
					ModeChanges.Add( pModeChange );
				}
				else
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_MODERATED;
					fModeratedSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			case 'n':
				if ( fNoPrivMsgsSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_NOPRIVMSGS;
					fNoPrivMsgsSet = true;
					ModeChanges.Add( pModeChange );
				}
				else
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_NOPRIVMSGS;
					fNoPrivMsgsSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			case 't':
				if ( fTopicLimitSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_TOPICLIMIT;
					fTopicLimitSet = true;
					ModeChanges.Add( pModeChange );
				}
				else
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_TOPICLIMIT;
					fTopicLimitSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			case 'i':
				if ( fInvitedSet )
					break;
				if ( g_IRCLocalServer.IsMyClient( pLClient ) && iModeChangeCount >= MAXMODEPARAMS )
					break;
				if ( dwType == MODE_ADD)
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_ADD | CHMODE_INVITEONLY;
					fInvitedSet = true;
					ModeChanges.Add( pModeChange );
				}
				else if ( dwType == MODE_DEL )
				{
					CIRCModeChange* pModeChange = new CIRCModeChange();
					iModeChangeCount++;
					pModeChange->m_dwFlags = MODE_DEL | CHMODE_INVITEONLY;
					fInvitedSet = true;
					ModeChanges.Add( pModeChange );
				}
				break;
			default:
			{
				if ( pServer )
					g_IRCLocalServer.SendToOne( pServer, IRCErrorString( ERR_UNKNOWNMODE ),
						g_IRCLocalServer.GetHost(), pServer->m_szServerName, *pCur );
			}
			break;
		}
		pCur++;
	}

	// Now we can reject non channel operators
	if ( !pServer && ( !dwOnChMode || !( dwOnChMode & ONCHMODE_CHANOP ) ) )
	{
		return false;
	}

	// Now actually change the channel's mode and make a nice valid mode buffer
	// to send to the other servers
	dwType = MODE_NULL;
	if ( iModeChangeCount )
	{
		for ( int i = 0; i < ModeChanges.GetCount(); i++ )
		{
			CIRCModeChange* pModeChange = ModeChanges[i];
			// make sure we have correct mode change sign
			if ( pModeChange->m_dwFlags & MODE_ADD )
			{
				if ( dwType != MODE_ADD )
				{
					*szModeBuf++ = '+';
					dwType = MODE_ADD;
				}
			}
			else
			{
				if ( dwType != MODE_DEL )
				{
					*szModeBuf++ = '-';
					dwType = MODE_DEL;
				}
			}
			DWORD dwCase = pModeChange->m_dwFlags & ~( MODE_DEL | MODE_ADD );
			switch ( dwCase )
			{
				case ONCHMODE_CHANOP:
					*szModeBuf++ = 'o';
					break;
				case ONCHMODE_VOICE:
					*szModeBuf++ = 'v';
					break;
				case CHMODE_BAN:
					// nick = nick!*@*
					// nick!user = nick!user@*
					// user@host = *!user@host
					// host.name = *!*@host.name
//					szModeParam = CIRCMachine::PrettyMask( pModeChange->m_Value.m_szString );
					break;
				case CHMODE_KEY:
					*szModeBuf++ = 'k';
					if ( pModeChange->m_dwFlags & MODE_ADD )
					{
						char szTemp[KEYLEN+1];
						sprintf( szTemp, "%s ", pModeChange->m_szString );
						strcat( szParamBuf, szTemp );
						strcpy( m_Mode.m_szKey, pModeChange->m_szString );
						m_Mode.m_dwMode |= CHMODE_KEY;
					}
					else
					{
						char szTemp[KEYLEN+1];
						sprintf( szTemp, "%s ", pModeChange->m_szString );
						strcat( szParamBuf, szTemp );
						strcpy( m_Mode.m_szKey, pModeChange->m_szString );
						m_Mode.m_dwMode &= ~CHMODE_KEY;
					}
					break;
				case CHMODE_LIMIT:
					*szModeBuf++ = 'l';
					char szTemp[16];
					sprintf( szTemp, "%i ", iLimitUsers );
					strcat( szParamBuf, szTemp );
					if ( iLimitUsers == -1 )
					{
						m_Mode.m_dwMode &= ~CHMODE_LIMIT;
						m_Mode.m_iLimit = -1;
					}
					else
					{
						m_Mode.m_dwMode |= CHMODE_LIMIT;
						m_Mode.m_iLimit = iLimitUsers;
					}
					break;
				case CHMODE_SECRET:
					*szModeBuf++ = 's';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_SECRET;
					else
						m_Mode.m_dwMode &= ~CHMODE_SECRET;
					break;
				case CHMODE_PRIVATE:
					*szModeBuf++ = 'p';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_PRIVATE;
					else
						m_Mode.m_dwMode &= ~CHMODE_PRIVATE;
					break;
				case CHMODE_MODERATED:
					*szModeBuf++ = 'm';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_MODERATED;
					else
						m_Mode.m_dwMode &= ~CHMODE_MODERATED;
					break;
				case CHMODE_NOPRIVMSGS:
					*szModeBuf++ = 'n';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_NOPRIVMSGS;
					else
						m_Mode.m_dwMode &= ~CHMODE_NOPRIVMSGS;
					break;
				case CHMODE_TOPICLIMIT:
					*szModeBuf++ = 't';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_TOPICLIMIT;
					else
						m_Mode.m_dwMode &= ~CHMODE_TOPICLIMIT;
					break;
				case CHMODE_INVITEONLY:
					*szModeBuf++ = 'i';
					if ( pModeChange->m_dwFlags & MODE_ADD )
						m_Mode.m_dwMode |= CHMODE_INVITEONLY;
					else
						m_Mode.m_dwMode &= ~CHMODE_INVITEONLY;
					break;
			}
		}
		*szModeBuf++ = 0;
		szParamBuf[strlen( szParamBuf ) - 1] = 0;
	}
	return true;
}

bool CIRCChannel::CanList()
{
	// Secret channels are not listed at all
	if ( m_Mode.m_dwMode & CHMODE_SECRET )
		return false;
	return true;
}

void CIRCChannel::PublicName(char * szPubName)
{
	// Private channels are listed as "Prv"
	if ( m_Mode.m_dwMode & CHMODE_PRIVATE )
	{
		strcpy(szPubName, " Prv");
		szPubName[0] = m_szChName[0];
	}
	else
		strcpy(szPubName, m_szChName);
}

bool CIRCChannel::MatchMask( CIRCMask* pMask, CIRCLRClient* pLRClient )
{
	// TODO: Check to see if anything matches
	return true;
}

bool CIRCChannel::CanJoin( CIRCLRClient* pLClient, char* szKey )
{
	char szPublicName[CHANNELLEN];
	PublicName( szPublicName );
	// Check the key first
	if ( strcmp( m_Mode.m_szKey, szKey ) )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_BADCHANNELKEY ),
			g_IRCLocalServer.GetHost(), pLClient->m_szNick, szPublicName);
		return false;
	}
	// Is it at or over the limit of members?
	if ( m_Mode.m_iLimit > 0 && m_ChanMembers.GetCount() >= m_Mode.m_iLimit )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_CHANNELISFULL ),
			g_IRCLocalServer.GetHost(), pLClient->m_szNick, szPublicName);
		// Can moderators can always join???
		return false;
	}
	// Is it invite only?
	bool fInvited = true;
	if ( m_Mode.m_dwMode & CHMODE_INVITEONLY )
	{
		fInvited = false;
		for(int i = 0; i < m_Invited.GetCount(); i++)
			// Does this mask match us up to us?
			if ( !MatchMask( m_Invited[i], pLClient ) )
				fInvited = true;
	}
	if ( !fInvited )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_INVITEONLYCHAN ),
			g_IRCLocalServer.GetHost(), pLClient->m_szNick, szPublicName);
		return false;
	}
	// Check this channel's ban list
	for(int i = 0; i < m_Banned.GetCount(); i++)
	{
		// Does this mask match us up to us?
		if ( !MatchMask( m_Invited[i], pLClient ) )
		{
			g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_BANNEDFROMCHAN ),
				g_IRCLocalServer.GetHost(), pLClient->m_szNick, szPublicName);
			return false;
		}
	}
	return true;
}

void CIRCChannel::ClientJoin( CIRCLRClient* pLClient )
{
	// Send out the command to join a channel
	// example: :Westy!noneya@10.1.1.1 JOIN :#MyChannel..

	g_IRCLocalServer.SendToOne( pLClient, ":%s JOIN :%s\r\n",
		pLClient->GetNickUserIP(), m_szChName );

	// Now send out all the ops of this channel
	//exmple: :irc.sphereserver.com 353 Westy = #MyChannel :@Westy
	int i;
	for( i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( m_ChanMembers[i]->m_dwMode & ONCHMODE_CHANOP )
		{
			g_IRCLocalServer.SendToOne(pLClient, ":%s %d %s = %s :@%s\r\n",
				g_IRCLocalServer.GetHost(), RPL_NAMREPLY,
				m_ChanMembers[i]->m_pLRClient->m_szNick,
				m_szChName, m_ChanMembers[i]->m_pLRClient->m_szNick );
		}
	}
	// Now send out all the voiced members of this channel
	for( i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( m_ChanMembers[i]->m_dwMode & ONCHMODE_VOICE )
		{
			g_IRCLocalServer.SendToOne(pLClient, ":%s %d %s = %s :+%s\r\n",
				g_IRCLocalServer.GetHost(), RPL_NAMREPLY,
				m_ChanMembers[i]->m_pLRClient->m_szNick,
				m_szChName, m_ChanMembers[i]->m_pLRClient->m_szNick );
		}
	}
	// And finally send all the other members of this channel
	for( i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( ! ( m_ChanMembers[i]->m_dwMode & ONCHMODE_CHANOP ) &&
			 ! ( m_ChanMembers[i]->m_dwMode & ONCHMODE_VOICE  ) )
		{
			g_IRCLocalServer.SendToOne(pLClient, ":%s %d %s = %s :%s\r\n",
				g_IRCLocalServer.GetHost(), RPL_NAMREPLY,
				m_ChanMembers[i]->m_pLRClient->m_szNick,
				m_szChName, m_ChanMembers[i]->m_pLRClient->m_szNick );
		}
	}
	// Send the end of names command
	//example: :irc.sphereserver.com 366 Westy #MyChannel :End of /NAMES list.
	g_IRCLocalServer.SendToOne(pLClient, IRCReplyString( RPL_ENDOFNAMES ),
		g_IRCLocalServer.GetHost(), pLClient->m_szNick, m_szChName);

	// Send the topic (if any)
	g_IRCLocalServer.Send_RPL_TOPIC( pLClient, this );

	return;
}

bool CIRCChannel::IsMember( CIRCLRClient* pLClient )
{
	for( int i = 0; i < m_ChanMembers.GetCount(); i++ )
	{
		if ( pLClient == m_ChanMembers[i]->m_pLRClient )
			return true;
	}
	return false;
}

bool CIRCChannel::IsChOp( CIRCLRClient* pLClient )
{
	for(int i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( pLClient == m_ChanMembers[i]->m_pLRClient &&
			( m_ChanMembers[i]->m_dwMode & ONCHMODE_CHANOP ) )
			return true;
	}
	return false;
}

bool CIRCChannel::IsChDeOp( CIRCLRClient* pLClient )
{
	for(int i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( pLClient == m_ChanMembers[i]->m_pLRClient &&
			( m_ChanMembers[i]->m_dwMode & ONCHMODE_DEOPPED ) )
			return true;
	}
	return false;
}

bool CIRCChannel::HasVoice( CIRCLRClient* pLClient )
{
	for(int i = 0; i < m_ChanMembers.GetCount(); i++)
	{
		if ( pLClient == m_ChanMembers[i]->m_pLRClient &&
			( m_ChanMembers[i]->m_dwMode & ONCHMODE_VOICE ) )
			return true;
	}
	return false;
}

void CIRCChannel::SetTopic( CIRCLRClient* pLClient, char* szTopic )
{
	// Is this guy one of my members?
	if ( IsMember( pLClient ) )
	{
		// Check privs on the channel first
		if ( m_Mode.m_dwMode & CHMODE_TOPICLIMIT )
		{
			// Only moderators can change the topic
			if ( !IsChOp( pLClient ) )
			{
				g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_CHANOPRIVSNEEDED ),
					g_IRCLocalServer.GetHost(), pLClient->m_szNick, m_szChName );
				return;
			}
		}
		// Fall through
	}
	else
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_NOTONCHANNEL ),
			g_IRCLocalServer.GetHost(), m_szChName );
		return;
	}

	// Set the topic
	strcpy( m_szTopic, szTopic );
	// Set the topic who/time stuff (undernet extensions)
	strcpy( m_szTopicNick, pLClient->m_szNick );
	time( & m_tTopicTime );
	// Now tell everyone on the channel about it
	char buffer[NICKLEN + USERLEN + 16 + 3];
	strcpy(buffer, pLClient->GetNickUserIP() );

	// example: :Westy2!noneya@10.1.1.1 TOPIC #Channel :This is the topic
	// Do we tell the other servers about this?
	g_IRCLocalServer.SendToChannel( this, NULL, NULL, ":%s TOPIC %s :%s\r\n",
		buffer, m_szChName, m_szTopic );
	return;
}

void CIRCChannel::ClientKickingClient( CIRCLRClient* pLClient, CIRCLRClient* pVictimClient, char* szComment )
{
	// Is the victim on this channel?
	if ( !IsMember( pVictimClient ) )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_NOTONCHANNEL ),
			g_IRCLocalServer.GetHost(), pVictimClient->m_szNick );
		return;
	}

	// Does the kicker has permission to kick (mode +o)
	if ( !IsChOp( pLClient ) )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_CHANOPRIVSNEEDED ),
			g_IRCLocalServer.GetHost(), pLClient->m_szNick );

		return;
	}
	// Now we can kick the victim off this channel

	// Send it to the whole channel
	if ( strlen( szComment ) > 0 )
	{
		g_IRCLocalServer.SendToChannel( this, NULL, NULL, ":%s KICK %s %s :%s says: %s\r\n",
			pLClient->GetNickUserIP(), m_szChName, pVictimClient->m_szNick,
			pLClient->m_szNick, szComment );
	}
	else
	{
		g_IRCLocalServer.SendToChannel( this, NULL, NULL, ":%s KICK %s %s :\r\n",
			pLClient->GetNickUserIP(), m_szChName, pVictimClient->m_szNick );
	}

	// Now take them out of whatever list they were in
	RemoveMember( pVictimClient );

	return;
}

void CIRCChannel::RemoveMember( CIRCLRClient* pLClient )
{
	for( int i = 0; i < m_ChanMembers.GetCount(); i++)
		if ( pLClient == m_ChanMembers[i]->m_pLRClient )
			m_ChanMembers.RemoveAt(i);
	return;
}

void CIRCChannel::ClientPart( CIRCLRClient* pLClient )
{

	if ( !IsMember( pLClient ) )
	{
		g_IRCLocalServer.SendToOne( pLClient, IRCErrorString( ERR_NOTONCHANNEL ),
			g_IRCLocalServer.GetHost(), m_szChName );
		return;
	}
	// Send out the command to the parting client to actually
	// part the channel (the command doesn't have to be issued from the channel window
	// in the client program)

	char buffer[NICKLEN + USERLEN + 16 + 3];
	strcpy(buffer, pLClient->GetNickUserIP() );

	g_IRCLocalServer.SendToOne( pLClient, ":%s PART :%s\r\n",
		buffer, m_szChName);
	RemoveMember( pLClient );
}

#endif