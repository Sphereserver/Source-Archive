/************************************************************************
 *   IRC - Internet Relay Chat
 *
 *	Error message defines
 *
 */

/*
 * Reserve numerics 000-099 for server-client connections where the client
 * is local to the server. If any server is passed a numeric in this range
 * from another server then it is remapped to 100-199. -avalon
 */
// Most of these are the same from network to network (001 to 004)
//	(005 to 014 are used differently from network to network)
#define RPL_WELCOME			001
#define RPL_YOURHOST		002
#define RPL_CREATED			003
#define RPL_MYINFO			004
#define RPL_MAP				005	/* Undernet extension */
#define RPL_MAPMORE			006	/* Undernet extension */
#define RPL_MAPEND			007	/* Undernet extension */
#define RPL_SNOMASK			  8	/* Undernet extension */
#define RPL_STATMEMTOT		  9	/* Undernet extension */
#define RPL_STATMEM			010	/* Undernet extension */

#define	RPL_YOURCOOKIE		014

/*
 * Errors are in the range from 400-599 currently and are grouped by what
 * commands they come from.
 */
#define ERR_NOSUCHNICK			401
#define ERR_NOSUCHSERVER		402
#define ERR_NOSUCHCHANNEL		403
#define ERR_CANNOTSENDTOCHAN	404
#define ERR_TOOMANYCHANNELS		405
#define ERR_WASNOSUCHNICK		406
#define ERR_TOOMANYTARGETS		407
#define ERR_NOSUCHSERVICE		408
#define ERR_NOORIGIN			409

#define ERR_NORECIPIENT			411
#define ERR_NOTEXTTOSEND		412
#define ERR_NOTOPLEVEL			413
#define ERR_WILDTOPLEVEL		414
#define ERR_QUERYTOOLONG		416		// Undernet extension

#define ERR_UNKNOWNCOMMAND		421
#define ERR_NOMOTD				422
#define ERR_NOADMININFO			423

#define ERR_NONICKNAMEGIVEN		431
#define ERR_ERRONEUSNICKNAME	432
#define ERR_NICKNAMEINUSE		433
#define ERR_SERVICENAMEINUSE	434
#define ERR_SERVICECONFUSED		435
#define ERR_NICKCOLLISION		436
#define ERR_BANNICKCHANGE		437		// Undernet extension
#define ERR_NICKTOOFAST			438		// Undernet extension
#define ERR_TARGETTOOFAST		439		// Undernet extension

#define ERR_USERNOTINCHANNEL	441
#define ERR_NOTONCHANNEL		442
#define	ERR_USERONCHANNEL		443

#define ERR_NOTREGISTERED		451

#define ERR_NEEDMOREPARAMS		461
#define ERR_ALREADYREGISTRED	462
#define ERR_NOPERMFORHOST		463
#define ERR_PASSWDMISMATCH		464
#define ERR_YOUREBANNEDCREEP	465
#define ERR_YOUWILLBEBANNED		466
#define ERR_KEYSET				467		// Undernet extension
#define ERR_INVALIDUSERNAME		468		// Undernet extension */

#define ERR_CHANNELISFULL		471
#define ERR_UNKNOWNMODE			472
#define ERR_INVITEONLYCHAN		473
#define ERR_BANNEDFROMCHAN		474
#define ERR_BADCHANNELKEY		475
#define ERR_BADCHANMASK			476		// Undernet extension
#define ERR_MODELESS			477		// Extension to RFC1459
#define ERR_BANLISTFULL			478		// Undernet extension

#define ERR_NOPRIVILEGES		481
#define ERR_CHANOPRIVSNEEDED	482
#define ERR_CANTKILLSERVER		483
#define ERR_ISCHANSERVICE		484		// Undernet extension

#define ERR_NOOPERHOST			491
#define ERR_NOSERVICEHOST		492

#define ERR_UMODEUNKNOWNFLAG	501
#define ERR_USERSDONTMATCH		502

#define ERR_SILELISTFULL		511		// Undernet extension
#define ERR_NOSUCHGLINE			512		// Undernet extension
#define ERR_BADPING				513		// Undernet extension

/*
 * Numberic replies from server commands.
 * These are currently in the range 200-399.
 */
#define RPL_NONE				300
#define RPL_AWAY				301
#define RPL_USERHOST			302
#define RPL_ISON				303
#define RPL_TEXT				304
#define RPL_UNAWAY				305
#define RPL_NOWAWAY				306
#define RPL_USERIP				307		// Undernet extension

#define RPL_WHOISUSER			311
#define RPL_WHOISSERVER			312
#define RPL_WHOISOPERATOR		313
#define RPL_WHOWASUSER			314
#define RPL_ENDOFWHO			315
#define RPL_WHOISCHANOP			316		// redundant and not needed but reserved
#define RPL_WHOISIDLE			317

#define RPL_ENDOFWHOIS			318
#define RPL_WHOISCHANNELS		319

#define RPL_LISTSTART			321
#define RPL_LIST				322
#define RPL_LISTEND				323
#define RPL_CHANNELMODEIS		324
#define RPL_CREATIONTIME		329

#define RPL_NOTOPIC				331
#define RPL_TOPIC				332
#define RPL_TOPICWHOTIME		333		// Undernet extension
#define RPL_LISTUSAGE			334		// Undernet extension

#define RPL_INVITING			341

#define RPL_VERSION				351
#define RPL_WHOREPLY			352
#define RPL_NAMREPLY			353
#define RPL_WHOSPCRPL			354		// Undernet extension, See also RPL_ENDOFWHO

#define RPL_KILLDONE			361
#define RPL_CLOSING				362
#define RPL_CLOSEEND			363
#define RPL_LINKS				364
#define RPL_ENDOFLINKS			365
#define RPL_ENDOFNAMES			366
#define RPL_BANLIST				367
#define RPL_ENDOFBANLIST		368
#define RPL_ENDOFWHOWAS			369

#define RPL_INFO				371
#define RPL_MOTD				372
#define RPL_INFOSTART			373
#define RPL_ENDOFINFO			374
#define RPL_MOTDSTART			375
#define RPL_ENDOFMOTD			376

#define RPL_YOUREOPER			381
#define RPL_REHASHING			382
#define RPL_YOURESERVICE		383
#define RPL_MYPORTIS			384
#define RPL_NOTOPERANYMORE		385

#define RPL_TIME				391

#define RPL_TRACELINK			200
#define RPL_TRACECONNECTING		201
#define RPL_TRACEHANDSHAKE		202
#define RPL_TRACEUNKNOWN		203
#define RPL_TRACEOPERATOR		204
#define RPL_TRACEUSER			205
#define RPL_TRACESERVER			206
#define RPL_TRACESERVICE		207
#define RPL_TRACENEWTYPE		208
#define RPL_TRACECLASS			209

#define RPL_STATSLINKINFO		211
#define RPL_STATSCOMMANDS		212
#define RPL_STATSCLINE			213
#define RPL_STATSNLINE			214
#define RPL_STATSILINE			215
#define RPL_STATSKLINE			216
#define RPL_STATSPLINE			217		// Undernet extenstion
#define RPL_STATSYLINE			218
#define RPL_ENDOFSTATS			219

#define RPL_UMODEIS				221

#define RPL_SERVICEINFO			231
#define RPL_ENDOFSERVICES		232
#define	RPL_SERVICE				233
#define RPL_SERVLIST			234
#define RPL_SERVLISTEND			235

#define	RPL_STATSLLINE			241

#define RPL_STATSUPTIME			242
#define RPL_STATSOLINE			243
#define RPL_STATSHLINE			244
//		RPL_STATSSLINE			245		// Reserved
#define RPL_STATSTLINE			246		// Undernet extension
#define RPL_STATSGLINE			247		// Undernet extension
#define RPL_STATSULINE			248		// Undernet extension
#define RPL_STATSDEBUG			249		// Extension to RFC1459
#define RPL_STATSCONN			250		// Undernet extension

#define RPL_LUSERCLIENT			251
#define RPL_LUSEROP				252
#define RPL_LUSERUNKNOWN		253
#define RPL_LUSERCHANNELS		254
#define RPL_LUSERME				255
#define RPL_ADMINME				256
#define RPL_ADMINLOC1			257
#define RPL_ADMINLOC2			258
#define RPL_ADMINEMAIL			259

#define RPL_TRACELOG			261
#define RPL_TRACEPING			262		// Extension to RFC1459

#define RPL_SILELIST			271		// Undernet extension
#define RPL_ENDOFSILELIST		272		// Undernet extension

//		RPL_STATSDELTA			274		// IRCnet extension
#define RPL_STATSDLINE			275		// Undernet extension

#define RPL_GLIST				280		// Undernet extension
#define RPL_ENDOFGLIST			281		// Undernet extension

// IRC defines, macros, etc., (these are all specified in RFC 1459)
#define NICKLEN		9
#define USERLEN		10
#define HOSTLEN		63
#define REALLEN		50
#define PASSWDLEN	20
#define BUFSIZE		512	/* WARNING: *DONT* CHANGE THIS!!!! */
#define MAXTARGETS	20
#define MAXCHANNELSPERUSER 10

#define BANREASONLEN 128
/*=============================================================================
 * enum all "server to server", "client to server", and "server to client" message types
 */

enum IRC_TYPE
{
	IRC_NONE		= -1,
	IRC_PRIVATE,
	IRC_WHO,
	IRC_WHOIS,
	IRC_WHOWAS,
	IRC_USER,
	IRC_NICK,
	IRC_SERVER,
	IRC_LIST,
	IRC_TOPIC,
	IRC_INVITE,
	IRC_VERSION,
	IRC_QUIT,
	IRC_SQUIT,
	IRC_KILL,
	IRC_INFO,
	IRC_LINKS,
	IRC_STATS,
	IRC_HELP,
	IRC_ERROR,
	IRC_AWAY,
	IRC_CONNECT,
	IRC_UPING,
	IRC_MAP,
	IRC_PING,
	IRC_PONG,
	IRC_OPER,
	IRC_PASS,
	IRC_WALLOPS,
	IRC_TIME,
	IRC_SETTIME,
	IRC_RPING,
	IRC_RPONG,
	IRC_NAMES,
	IRC_ADMIN,
	IRC_TRACE,
	IRC_NOTICE,
	IRC_WALLCHOPS,
	IRC_CPRIVMSG,
	IRC_CNOTICE,
	IRC_JOIN,
	IRC_PART,
	IRC_LUSERS,
	IRC_MOTD,
	IRC_MODE,
	IRC_KICK,
	IRC_USERHOST,
	IRC_USERIP,
	IRC_ISON,
	IRC_SQUERY,
	IRC_SERVLIST,
	IRC_SERVSET,
	IRC_REHASH,
	IRC_RESTART,
	IRC_CLOSE,
	IRC_DIE,
	IRC_HASH,
	IRC_DNS,
	IRC_SILENCE,
	IRC_GLINE,
	IRC_BURST,
	IRC_CREATE,
	IRC_END_OF_BURST,
	IRC_END_OF_BURST_ACK,
	IRC_QTY,
};

// Used for local client, in the MODE parsers, and for synchronizing net joins
// (eventually)
#define FLAGS_PINGSENT		0x00000001		// Unreplied ping sent
#define FLAGS_DEADSOCKET	0x00000002		// Local socket is dead--Exiting soon
#define FLAGS_KILLED		0x00000004		// Prevents "QUIT" from being sent for this
#define FLAGS_OPER			0x00000008		// Operator
#define FLAGS_LOCOP			0x00000010		// Local operator -- SRB
#define FLAGS_INVISIBLE		0x00000020		// makes user invisible
#define FLAGS_WALLOP		0x00000040		// send wallops to them
#define FLAGS_SERVNOTICE	0x00000080		// server notices such as kill
#define FLAGS_BLOCKED		0x00000100		// socket is in a blocked condition
#define FLAGS_UNIX			0x00000200		// socket is in the unix domain, not inet
#define FLAGS_CLOSING		0x00000400		// set when closing to suppress errors
#define FLAGS_LISTEN		0x00000800		// used to mark clients which we listen() on
#define FLAGS_CHKACCESS		0x00001000		// ok to check clients access if set
#define FLAGS_DOINGDNS		0x00002000		// client is waiting for a DNS response
#define FLAGS_AUTH			0x00004000		// client is waiting on rfc931 response
#define FLAGS_WRAUTH		0x00008000		// set if we havent writen to ident server
#define FLAGS_LOCAL			0x00010000		// set for local clients
#define FLAGS_GOTID			0x00020000		// successful ident lookup achieved
#define FLAGS_DOID			0x00040000		// I-lines say must use ident return
#define FLAGS_NONL			0x00080000		// No \n in buffer
#define FLAGS_TS8			0x00100000		// Why do you want to know?
#define FLAGS_PING			0x00200000		// Socket is waiting for udp ping response
#define FLAGS_ASKEDPING		0x00400000		// Client asked for udp ping
#define FLAGS_MAP			0x00800000		// Show server on the map
#define FLAGS_JUNCTION		0x01000000		// Junction causing the net.burst
#define FLAGS_DEAF			0x02000000		// Makes user deaf
#define FLAGS_CHSERV		0x04000000		// Disallow KICK or MODE -o on the user
											// don't display channels in /whois
#define FLAGS_BURST			0x08000000		// Server is receiving a net.burst
#define FLAGS_BURST_ACK		0x10000000		// Server is waiting for eob ack

// Used in the MODE parsers
#define MODE_NULL					0x00000000
#define MODE_DEL					0x20000000
#define MODE_ADD					0x40000000

// On channel MODE defines
#define ONCHMODE_CHANOP				0x00000001	// Channel operator
#define ONCHMODE_VOICE				0x00000002	// the power to speak
#define ONCHMODE_DEOPPED			0x00000004	// Is de-opped by a server
#define ONCHMODE_SERVOPOK			0x00000008	// Server op allowed
#define ONCHMODE_ZOMBIE				0x00000010	// Kicked from channel
#define ONCHMODE_BAN				0x00000020	// ban channel flag
#define ONCHMODE_BAN_IPMASK			0x00000040	// ban mask is an IP-number mask
#define ONCHMODE_BAN_OVERLAPPED		0x00000080	// ban overlapped, need bounce
#define ONCHMODE_OVERLAP			(CHFL_CHANOP|CHFL_VOICE)
#define ONCHMODE_BURST_JOINED		0x00000100	// Just joined by net.junction
#define ONCHMODE_BURST_BAN			0x00000200	// Ban part of last BURST
#define ONCHMODE_BURST_BAN_WIPEOUT	0x00000400	// Ban will be wiped at end of BURST
#define ONCHMODE_BANVALID			0x00000800	// CHFL_BANNED bit is valid
#define ONCHMODE_BANNED				0x00001000	// Channel member is banned
#define ONCHMODE_SILENCE_IPMASK		0x00002000	// silence mask is an IP-number mask

// Channel MODE defines
#define CHMODE_PRIVATE		0x00004000
#define CHMODE_SECRET		0x00008000
#define CHMODE_MODERATED	0x00010000
#define CHMODE_TOPICLIMIT	0x00020000
#define CHMODE_INVITEONLY	0x00040000
#define CHMODE_NOPRIVMSGS	0x00080000
#define CHMODE_KEY			0x00100000
#define CHMODE_BAN			0x00200000
#define CHMODE_LIMIT		0x00400000
#define CHMODE_SENDTS		0x00800000	// TS was 0 during a local user /join; send temporary TS
#define CHMODE_LISTED		0x01000000

#define CHMODE_WPARAS	(ONCHMODE_CHANOP|ONCHMODE_VOICE|ONCHMODE_BAN|CHMODE_KEY|CHMODE_LIMIT)

#define SEND_UMODES \
    (FLAGS_INVISIBLE|FLAGS_OPER|FLAGS_WALLOP|FLAGS_DEAF|FLAGS_CHSERV)
#define ALL_UMODES (SEND_UMODES|FLAGS_SERVNOTICE|FLAGS_LOCOP)
#define FLAGS_ID (FLAGS_DOID|FLAGS_GOTID)

/* server notice stuff */

#define SMODE_ADD			0x00000001
#define SMODE_DEL			0x00000002
#define SMODE_SET			0x00000003
		// DON'T CHANGE THESE VALUES !
		// THE CLIENTS DEPEND ON IT  !
#define SMODE_OLDSNO		0x00000001		// unsorted old messages
#define SMODE_SERVKILL		0x00000002		// server kills (nick collisions)
#define SMODE_OPERKILL		0x00000004		// oper kills
#define SMODE_HACK2			0x00000008		// desyncs
#define SMODE_HACK3			0x00000010		// temporary desyncs
#define SMODE_UNAUTH		0x00000020		// unauthorized connections
#define SMODE_TCPCOMMON		0x00000040		// common TCP or socket errors
#define SMODE_TOOMANY		0x00000080		// too many connections
#define SMODE_HACK4			0x00000100		// Uworld actions on channels
#define SMODE_GLINE			0x00000200		// glines
#define SMODE_NETWORK		0x00000400		// net join/break, etc
#define SMODE_IPMISMATCH	0x00000800		// IP mismatches
#define SMODE_THROTTLE		0x00001000		// host throttle add/remove notices
#define SMODE_OLDREALOP		0x00002000		// old oper-only messages
#define SMODE_CONNEXIT		0x00004000		// client connect/exit (ugh)

#define SMODE_ALL			0x7fffffff		// Don't make it larger then significant,
										// that looks nicer
#define SMODE_USER			(SMODE_ALL & ~SMODE_OPER)
#define SMODE_DEFAULT		(SMODE_NETWORK|SMODE_OPERKILL|SMODE_GLINE)
#define SMODE_OPERDEFAULT	(SMODE_DEFAULT|SMODE_HACK2|SMODE_HACK4|SMODE_THROTTLE|SMODE_OLDSNO)
#define SMODE_OPER			(SMODE_CONNEXIT|SMODE_OLDREALOP)
#define SMODE_NOISY			(SMODE_SERVKILL|SMODE_UNAUTH)

/*=============================================================================
 * General defines
 */

#define Reg1 register
#define Reg2 register
#define Reg3 register
#define Reg4 register
#define Reg5 register
#define Reg6 register
#define Reg7 register
#define Reg8 register
#define Reg9 register
#define Reg10 register

#define BadPtr(x) (!(x) || (*(x) == '\0'))

#define isvalid(c) (((c) >= 'A' && (c) <= '~') || isdigit(c) || (c) == '-')

#define IRCReplyString	CIRCTools::ReplyString
#define IRCErrorString	CIRCTools::ErrorString

/*=============================================================================
 * CIRCChannel MODE command defines
 */

#define PINGFREQUENCY (120)
#define CONNECTTIMEOUT (90)
#define MAXMODEPARAMS	6
#define MODEBUFLEN		200

#define KEYLEN			23
#define TOPICLEN		160
#define CHANNELLEN		200
#define MAXBANS			30
#define MAXBANLENGTH	1024
#define MAXARGS			16

// Define all the standard WHO listing symbols
#define	WHO_NOT_AWAY			0
#define	WHO_AWAY				1
#define	WHO_OPER				2
#define	WHO_ONCHANNEL_OP		3
#define	WHO_ONCHANNEL_VOICE		4
#define	WHO_ONCHANNEL_ZOMBIE	5
#define	WHO_DEAF				6
#define	WHO_OPER_INVIS			7
#define	WHO_OPER_WALLOPS		8

#define NICKNAMEHISTORYLENGTH (800) // There are what UnderNet uses!
#define KILLCHASETIMELIMIT (30)
// I put this in
#define NICKAUDITTIMER (60*10)

// Define for see what kind of channel it is
#define IsLocalChannel(name)	(*(name) == '&')
#define IsModelessChannel(name)	(*(name) == '+')
#define IsChannelName(name)	(*(name) == '#' || \
				IsModelessChannel(name) || IsLocalChannel(name))
//
// Maximum acceptable lag time in seconds: A channel younger than
// this is not protected against hacking admins.
// Mainly here to check if the TS clocks really sync (otherwise this
// will start causing HACK notices.
// This value must be the same on all servers.
//
#define TS_LAG_TIME ((time_t)1200)
//
// A Magic TS that is used for channels that are created by JOIN,
// a channel with this TS accepts all TS without complaining that
// it might receive later via MODE or CREATE.
//
#define MAGIC_REMOTE_JOIN_TS 1270080000

/* These must be the same on ALL servers ! Do not change ! */

#define NUMNICKLOG 6
#define NUMNICKMAXCHAR 'z'	/* See convert2n[] */
#define NUMNICKBASE 64		/* (2 << NUMNICKLOG) */
#define NUMNICKMASK 63		/* (NUMNICKBASE-1) */
#define MAX_MAXCLIENTS 4096	/* (NUMNICKBASE * NUMNICKBASE) */

#define NUMNICKLEN 3		/* strlen("YXX") */

#define IsOper(x)				((x)->m_dwFlags & FLAGS_OPER)
#define IsLocOp(x)				((x)->m_dwFlags & FLAGS_LOCOP)
#define IsInvisible(x)			((x)->m_dwFlags & FLAGS_INVISIBLE)
#define IsDeaf(x)				((x)->m_dwFlags & FLAGS_DEAF)
#define IsChanService(x)		((x)->m_dwFlags & FLAGS_CHSERV)
#define IsAnOper(x)				((x)->m_dwFlags & (FLAGS_OPER|FLAGS_LOCOP))
#define SendWallops(x)			((x)->m_dwFlags & FLAGS_WALLOP)
#define SendServNotice(x)		((x)->m_dwFlags & FLAGS_SERVNOTICE)
#define IsUnixSocket(x)			((x)->m_dwFlags & FLAGS_UNIX)
#define IsListening(x)			((x)->m_dwFlags & FLAGS_LISTEN)
#define DoAccess(x)				((x)->m_dwFlags & FLAGS_CHKACCESS)
#define IsLocal(x)				((x)->m_dwFlags & FLAGS_LOCAL)
#define IsDead(x)				((x)->m_dwFlags & FLAGS_DEADSOCKET)
#define IsJunction(x)			((x)->m_dwFlags & FLAGS_JUNCTION)
#define IsBurst(x)				((x)->m_dwFlags & FLAGS_BURST)
#define IsBurstAck(x)			((x)->m_dwFlags & FLAGS_BURST_ACK)
#define IsBurstOrBurstAck(x)	((x)->m_dwFlags & (FLAGS_BURST|FLAGS_BURST_ACK))

#define SetOper(x)				((x)->m_dwFlags |= FLAGS_OPER)
#define SetLocOp(x)				((x)->m_dwFlags |= FLAGS_LOCOP)
#define SetInvisible(x)			((x)->m_dwFlags |= FLAGS_INVISIBLE)
#define SetWallops(x)			((x)->m_dwFlags |= FLAGS_WALLOP)
#define SetUnixSock(x)			((x)->m_dwFlags |= FLAGS_UNIX)
#define SetDNS(x)				((x)->m_dwFlags |= FLAGS_DOINGDNS)
#define DoingDNS(x)				((x)->m_dwFlags & FLAGS_DOINGDNS)
#define SetAccess(x)			((x)->m_dwFlags |= FLAGS_CHKACCESS)
#define DoingAuth(x)			((x)->m_dwFlags & FLAGS_AUTH)
#define NoNewLine(x)			((x)->m_dwFlags & FLAGS_NONL)
#define DoPing(x)				((x)->m_dwFlags & FLAGS_PING)
#define SetAskedPing(x)			((x)->m_dwFlags |= FLAGS_ASKEDPING)
#define AskedPing(x)			((x)->m_dwFlags & FLAGS_ASKEDPING)
#define SetJunction(x)			((x)->m_dwFlags |= FLAGS_JUNCTION)
#define SetBurst(x)				((x)->m_dwFlags |= FLAGS_BURST)
#define SetBurstAck(x)			((x)->m_dwFlags |= FLAGS_BURST_ACK)

#define ClearOper(x)			((x)->m_dwFlags &= ~FLAGS_OPER)
#define ClearInvisible(x)		((x)->m_dwFlags &= ~FLAGS_INVISIBLE)
#define ClearWallops(x)			((x)->m_dwFlags &= ~FLAGS_WALLOP)
#define ClearDNS(x)				((x)->m_dwFlags &= ~FLAGS_DOINGDNS)
#define ClearAuth(x)			((x)->m_dwFlags &= ~FLAGS_AUTH)
#define ClearAccess(x)			((x)->m_dwFlags &= ~FLAGS_CHKACCESS)
#define ClearPing(x)			((x)->m_dwFlags &= ~FLAGS_PING)
#define ClearAskedPing(x)		((x)->m_dwFlags &= ~FLAGS_ASKEDPING)
#define ClearBurst(x)			((x)->m_dwFlags &= ~FLAGS_BURST)
#define ClearBurstAck(x)		((x)->m_dwFlags &= ~FLAGS_BURST_ACK)

// All our forward references

/*
 *	Classes for servers, clients, etc
 */
// Useful collection of IRC related functions
class CIRCTools;
// This is us
class CIRCLocalServer;		// public CGSocket
// Base class for anything we attach to or attaches to us
class CIRCMachine;
// an IRC client either locally or remotely attached to us
class CIRCLRClient;			// : public CIRCMachine
// an IRC server either we attached to or it attached to us
class CIRCServer;			// : public CIRCMachine
// They attached to us
class CIRCDownlinkServer;	// : public CIRCServer
// We attached to them
class CIRCUplinkServer;		// : public CIRCServer, public CGSocket
// Table which matches up all the server nicks to their server

/*
 *	Classes for channels
 */
// This is a channel
class CIRCChannel;

/*
 *	Classes for nicks and supporting
 */
// This is a currently active nick (it's being used, or it has a history being tracked)
class CIRCNick;
class CIRCNickTrack;

// This is a numnick
class CIRCNumNick;
// Used to keep track of all connected servers nick data
class CIRCNumNicks;

/*
 *	Classes for nicks and channel support
 */
// Holds modes for channels and nicks
struct CIRCMode;
// containes various mopde change
struct CIRCModeChange;

/*
 *	Classes for server configuration and administration
 */
// Describes bans, klines and/or glines
class CIRCMask;
// Information the admin would like all the clients to know about
class CIRCAdminLine;
// Centralizes local server specific configuration information
class CIRCAdminInfo;
// All the MOTD the admin supports
class CIRCMOTD;
// All the "connection/allow connections" permitted on this server
class CIRCCNLine;

/*
 *	Classes for server/client I/O
 */
// A single line command received from a server or client cracked and ready to use
class CIRCCmd;
// List of all the cracked commands (CIRCCmd) received from a client or server
class CIRCCmds;
// Used for temporarily making a list for output to a client request
struct SWhoList;

// Used to wrap all the standard error messages up
typedef struct SIRCNumeric
{
  int m_iNumericValue;
  char* m_szNumericPattern;
} CIRCNumeric;


/*
 *	CIRCTools
 *
 *	Just a collection of useful stuff for various IRC related things
 *
*/

#define PrepareBuffer( buffer, num, tail )			\
{													\
  register char *s = buffer + 4;					\
  register const char *ap = sm_atoi_tab + (num << 2);	\
  													\
  strcpy(buffer, ":%s 000 %s ");					\
  *s++ = *ap++;										\
  *s++ = *ap++;										\
  *s = *ap;											\
  strcpy(s + 5, tail);								\
  strcpy( s + strlen( s ), "\r\n" );				\
}

class CIRCTools
{
	static char sm_scratch_buffer[32];
	static char sm_szNumericBuffer[512];
	static CIRCNumeric sm_NumericErrors[];
	static CIRCNumeric sm_LocalReplies[];
	static CIRCNumeric sm_NumericReplies[];
public:
	// To convert an int to a string
	static const char sm_cConvert2y[NUMNICKBASE];
	// To convert a string to an int
	static const BYTE sm_ucConvert2n[NUMNICKMAXCHAR + 1];
	static LPCTSTR const sm_CmdTbl[];
	static const char sm_atoi_tab[4000];
public:
	static char* TheTime(time_t value);
#ifndef _BSD
	static int gettimeofday(struct timeval* tv, void* v);
#endif
	static unsigned long ircrandom(void);
	static char *check_string(char *s);
	static unsigned int Base64ToInt(const char *str);
	static const char* IntToBase64(unsigned int i);
	static char *inetntoa(CSocketAddressIP in);
	static char * _cdecl sprintf_irc(register char *str, const char *format, ...);
	static char *vsprintf_irc(register char *str, register const char *format, register va_list vl);
	static int match(const char *mask, const char *name);
	static char * collapse(char *pattern);
	static char* ErrorString( int iNumeric );
	static char* ReplyString( int iNumeric );
	static int IsSMask(char *word);
	static DWORD MakeSMask( DWORD dwOldMask, char* szArg, DWORD dwType );
	static char *strtoken(char **save, char *str, char *fs);
};

struct CIRCModeChange
{
	union
	{
		CIRCMachine* m_pMachine;
		CIRCChannel* m_pChannel;
	} m_Object;
	struct
    {
      char* m_szBanStr;
      char* m_szWho;
      time_t m_tWhen;
    } m_Ban;
	char m_szString[HOSTLEN];
	DWORD m_dwFlags;
};

struct CIRCMode
{
public:
	DWORD m_dwMode;
	int m_iLimit;
	char m_szKey[KEYLEN + 1];
};

class CIRCMachine	// Could be a remote server or a client
{
private:
	bool m_fAuthenticated;
	CClient* m_pClient;	// Link back to my CClient (uplinks don't have one)
	int m_iAuthFD;		// fd for rfc931 authentication (IDENTD)
protected:
public:
//	CIRCMode m_Mode;
	char m_szPassword[PASSWDLEN+1];
	char m_szNick[NICKLEN+1];
	char m_szUser[USERLEN+1];
	char m_szHostGiven[HOSTLEN+1];
	char m_szHost[HOSTLEN+1];
	char m_szServerName[HOSTLEN+1];
	char m_szRealName[REALLEN+1];
	char m_szAway[TOPICLEN+1];
	CSocketAddressIP m_iaClientIP;
	DWORD m_dwCookie;
	time_t m_tLastNick;
	DWORD m_dwFlags;
	DWORD m_dwSMask;	// Mask for server messages
	time_t m_tLastTime;		// For pinging
private:
public:
	CIRCMachine();
	~CIRCMachine();
	bool IsAuthenticated() { return m_fAuthenticated; }
	void Authenticate(bool fAuthenticate);
	bool IsFlag( DWORD dwMode )
	{
		return(( m_dwFlags & dwMode) ? true : false );
	}
	virtual void SetFlag( DWORD dwMode ) { return; }
	virtual void ClearFlag( DWORD dwMode )  { return; }
	void SetClient(CClient * pClient)
	{
		m_pClient = pClient;
	}
	CClient * GetClient()
	{
		return m_pClient;
	}
	static char* PrettyMask( char *szMask );

	char* GetNickUserIP( char* szTempNick = NULL );
	static char* GetNickUserIP( char* szTempNick, char* szTempUser,char* szTempIP );

	char* GetNickUserHost();
	static char* GetNickUserHost( char* szTempNick, char* szTempUser,char* szTempHost );
	char *GetClientName( bool fShowIP );
};

class CIRCLRClient	: public CIRCMachine
{
private:
public:
	int m_iTargets;			// used to control flooding
	int m_iHops;			// 0 if local
	CIRCNumNick* m_pNumNick;	// These are unique
	CIRCServer* m_pServer;	// Only if they're remote
public:
	CGPtrTypeArray< CIRCChannel* > m_Channels;	// Channels I'm currently a member of
	CIRCLRClient();
	~CIRCLRClient();
	void SetFlag( DWORD dwMode );
	void ClearFlag( DWORD dwMode );
};

class CIRCServerNick
{
public:
	char m_szNick[NICKLEN];
	CIRCServer* m_pServer;
};

class CIRCNumNick
{
public:
	CIRCNick* m_pNick;				// Points to the current string nick
	int m_iSlot;					// It's position in the array
};

class CIRCNumNicks
{
public:
	// I noticed that once a numnick is given out, it's never reassigned unless the
	// server exhausts all the slots, then it starts re-using old unused ones
	CIRCNumNick m_NumNicks[MAX_MAXCLIENTS];	// Anyway to make this dynamic?
public:
	void AssignNumNick( CIRCNick* pNick, char* szNumNick );
	CIRCNumNick* GetNumNick( char* szNumNick )
	{
		assert( strlen( szNumNick ) == 3 );
		int iSlot = GetSlot( szNumNick );
		assert( iSlot < MAX_MAXCLIENTS );
		return &m_NumNicks[iSlot];
	}
	void FreeNumNick( char* szNumNick )
	{
		AssignNumNick( NULL, szNumNick );
	}
	int GetSlot( char* szNumNick )
	{
		// [0] = server ID, [1] & [2] = index into [0]'s array of nicks
		unsigned char ucNN1 = (unsigned char)szNumNick[1];
		unsigned char ucNN2 = (unsigned char)szNumNick[2];
		int i1 = CIRCTools::sm_ucConvert2n[ucNN1];
		int i2 = CIRCTools::sm_ucConvert2n[ucNN2];
		int iSlot = ( ( i1 << NUMNICKLOG ) + i2 );
		return iSlot;
	}
};

class CIRCMyNumNicks	: public CIRCNumNicks
{
	int m_iNextSlot;
public:
	CIRCMyNumNicks()
	{
		m_iNextSlot = 0;
		for( int i = 0; i < MAX_MAXCLIENTS; i++ )
			m_NumNicks[i].m_iSlot = i;
	}
	CIRCNumNick* GetNextSlot();
};

// These either connect to us, or we connect to them
class CIRCServer	: public CIRCMachine, public CIRCNumNicks
{
private:
public:
	int m_iClients;
	int m_iInvisibleClients;
	bool m_fNeedsSync;
	int m_iHops;
	time_t m_tOnLineSince;
	time_t m_tLinkedAt;
	char m_szProtocol[4];
	char m_szDescription[64];
public:
	CIRCServer();
	~CIRCServer();
	void SetFlag( DWORD wMode );
	void ClearFlag( DWORD wMode );
};

class CIRCDownlinkServer	: public CIRCServer	// These connect to us
{
private:
};

class CIRCUplinkServer		: public CIRCServer	// We connect to them
{
public:
public:
	CGSocket m_Socket;
	WORD m_iPort;
	CServTime m_timeLastEvent;	// Last time we got a message
};

// a Banned only mean they can't log into your server (or join your channel),
// a K-Line means they are banned from interacting with your users
// (the local server will relay their messages to other servers though)
class CIRCMask
{
private:
public:
	DWORD m_dwMode;	// Is it being added or deleted?
	char m_szNick[NICKLEN+1];
	char m_szMask[HOSTLEN+1];
	char m_szPath[_MAX_PATH+1];
	time_t m_tTime;
};

// struct for who's going to be listed, and the symbols they get
struct SWhoList
{
public:
	CIRCLRClient* m_pLRClient;
	char m_szWhoSymbols[WHO_OPER_WALLOPS+1];
public:
	SWhoList( CIRCLRClient* pLRClient )
	{
		m_pLRClient = pLRClient;
		strcpy(m_szWhoSymbols, "        " );
	}
};

class CIRCChannelMember
{
public:
	CIRCLRClient* m_pLRClient;
	DWORD m_dwMode;
};

class CIRCChannel
{
public:
	CIRCMode m_Mode;
	time_t m_tCreationTime;
	char m_szTopic[TOPICLEN + 1];
	char m_szTopicNick[NICKLEN + 1];
	time_t m_tTopicTime;
	CGPtrTypeArray< CIRCChannelMember* > m_ChanMembers;	// All the members and their on channel mode

	CGPtrTypeArray< CIRCMask* > m_Invited;			// List of masks which are invited
	CGPtrTypeArray< CIRCMask* > m_Banned;			// List of masks which are banned
	char m_szChName[CHANNELLEN];
public:
	CIRCChannel();
	bool IsMode( DWORD dwMode )
	{
		return(( m_Mode.m_dwMode & dwMode) ? true : false );
	}
	void SetMode( DWORD dwMode ) { return; }
	void ClearMode( DWORD dwMode )  { return; }

	bool CanList();
	void PublicName(char * szPubName);
	bool CanJoin( CIRCLRClient* pLRClient, char* szKey );
	void ClientJoin( CIRCLRClient* pLRClient );
	void ClientPart( CIRCLRClient* pLRClient );
	void SetTopic( CIRCLRClient* pLRClient, char* szTopic );
	bool IsMember( CIRCLRClient* pLRClient );
	bool IsChOp( CIRCLRClient* pLRClient );
	bool IsChDeOp( CIRCLRClient* pLRClient );
	bool HasVoice( CIRCLRClient* pLRClient );
	void ClientKickingClient( CIRCLRClient* pLRClient, CIRCLRClient* pVictimClient, char* szComment );
	void RemoveMember( CIRCLRClient* pLRClient );
	bool MatchMask( CIRCMask* pMask, CIRCLRClient* pLRClient );
	bool Do_MODE( CIRCCmd* pCmd, CIRCMachine* pMachine, char* szModeBuf, char* szParamBuf, int *fBadOp );
	// This is going to be for bots
	int CountOperators();
	CIRCChannelMember* GetChannelMember( CIRCLRClient* pLRClient );
	DWORD GetOnChannelMode( CIRCLRClient* pLRClient );
	void GetModes( CIRCMachine* pMachine, char* szModeBuf, char* szParaBuf );
};

class CIRCNickTrack
{
public:
	char m_szUser[USERLEN];		// The USER they used
	char m_szHost[HOSTLEN];		// The hostname they had
	char m_szRealName[REALLEN];	// Their "real name"
	CSocketAddressIP m_iaClientIP;		// Their IP they had
	time_t m_tStart;			// When they started using this nick
	time_t m_tEnd;				// When they stopped using this nick
};

class CIRCNick
{
private:
	CGPtrTypeArray< CIRCNickTrack* > m_History;
public:
	// The KILL, MODE and KICK commands need to have nick history tracking
	// No other commands are to have nick changes checked for (RFC 1459, 8.9 para 2)
	char m_szNick[NICKLEN];
	CIRCLRClient* m_pLRClient;	// This is the current logged in owner (either local or remote)
public:
	CIRCNick();
	~CIRCNick();
	bool CanDelete();
	void NewOwner( CIRCLRClient *pLRClient );
	void ClearOwner();
	bool IsOwned();
	void ClearHistory();
	CIRCNickTrack* FindHistory( time_t t_TimeLimit );
};

class CIRCCmd
{
private:
public:
	int m_iQtyArgs;
	char m_szArgs[MAXARGS][BUFSIZE];
	char m_szTrailing[BUFSIZE];
public:
	CIRCCmd()
	{
		m_iQtyArgs = 0;
	}
};

class CIRCCmds
{
private:
public:
	int m_iQtyCmds;
	CIRCCmd m_Cmd[MAXARGS];
public:
	CIRCCmds()
	{
		m_iQtyCmds = 0;
	}
};

class CIRCAdminLine
{
	char m_szLine[80];
public:
	CIRCAdminLine( char* szLine )
	{
		strcpy( m_szLine, szLine );
	}
	const char* GetLine()
	{
		return m_szLine;
	}
};

class CIRCAdminInfo
{
public:
	char m_szIPOrHostMask[HOSTLEN];
	char m_szHostMask[HOSTLEN];
	char m_szUser[HOSTLEN];
	char m_szPassword[PASSWDLEN];
};

class CIRCCNLine
{
public:
	char m_szServerName[HOSTLEN+1];
	char m_szHost[HOSTLEN+1];
	char m_szPassword[PASSWDLEN+1];
	WORD m_wPort;
	int m_iMaskHost;
	bool m_bAutoConnect;
	CSocketAddressIP m_iaClientIP;
};

class CIRCMOTD
{
public:
	char m_szHostMask[HOSTLEN+1];	// Host mask to send this MOTD to
	char m_szPath[_MAX_PATH];		// Full path name to this MOTD
};

class CIRCLocalServer	: public CIRCMyNumNicks
{
private:
	friend class CIRCMachine;				// Makes life easier this way

	CGSocket m_Socket;

	// Stuff that's not configurable
	char m_szVersion[BUFSIZE];
	char m_szDeamonName[BUFSIZE];
	char m_szCompile[16];
	char m_szCopyright[BUFSIZE];
	char m_szProtocol[4];

	// Stuff to customize the users installation
	char m_szHost[HOSTLEN];
	char m_szName[HOSTLEN];
	char m_szDescription[64];
	WORD m_wPort;
	bool m_fAccountSecurity;
	CGPtrTypeArray< CIRCAdminLine* > m_AdminLines;	// List of /ADMIN lines
	CIRCAdminInfo m_AdminInfo;
	time_t m_tNicksTimer;	// Use this to clean up m_Nicks periodically, mabey move this to the nick history itself?
	time_t m_tOnlineSince;	// Time this server booted up
	int m_bin_len;
	BYTE m_Raw[BUFSIZE];
	// Offset to timestamps (for HACK detection mostly)
	time_t m_TSoffset;
public:
	static DWORD m_dwUserModes[16];
	char m_szSendUModeBuffer[64];
	char m_szSendBuffer[2048];
	char m_szNick[1];			// I guess these are more like an ID, only one char

	// All my list of stuff

	// These are where they actually are store

	// List of unknown machines (could be a server or a client attempting to connect)
	CGPtrTypeArray< CIRCMachine* > m_Machines;
	// List of authenticated local humans
	CGPtrTypeArray< CIRCLRClient* > m_LClients;
	// List of authenticated remote whatevers
	CGPtrTypeArray< CIRCLRClient* > m_RClients;
	// List of servers which we have connected to
	CGPtrTypeArray< CIRCUplinkServer* > m_UplinkServers;
	// List of servers which have connected to us
	CGPtrTypeArray< CIRCDownlinkServer* > m_DownlinkServers;
	// X-ref servers and their nicks
	CGPtrTypeArray< CIRCServerNick* > m_ServerNicks;


	// These are just lists
	// List of IRC Operators
	CGPtrTypeArray< CIRCLRClient* > m_Opers;

	// List of connection/authorize lines from the script file
	CGPtrTypeArray< CIRCCNLine* > m_CNLines;

	// List of servers which we're trying to connect to
	CGPtrTypeArray< CIRCUplinkServer* > m_BadUplinkServers;

	// All the channels that this server has local clients on
	CGPtrTypeArray< CIRCChannel* > m_Channels;

	// All the local nick names that are either being used, or still being tracked
	CGPtrTypeArray< CIRCNick* > m_Nicks;

	// List of system and host mask MOTDs
	// (host names can have a special MOTD for themselves by request)
	CGPtrTypeArray< CIRCMOTD* > m_MOTDs;

	// List of people banned from logging in locally or joining a channel globally
	CGPtrTypeArray< CIRCMask* > m_Bans;
	// List of people banned from interacting with your users
	CGPtrTypeArray< CIRCMask* > m_KLines;
	// List of people banned from logging into any server on the network
	CGPtrTypeArray< CIRCMask* > m_GLines;
public:
	CIRCLocalServer();

	// A few accessors
	char* GetHost() { return m_szHost; }
	char* GetName() { return m_szName; }
	char* GetVersion() { return m_szVersion; }

	// Startup and configuration
	bool Init();
	bool ConfInit();
	bool UplinksInit();
	bool SocketsInit(); // Initialize IRC sockets

	// Periodically does stuff in here
	void OnTick();
	// See if any nick names can be deleted and clean up some history
	void NickAudit();
	// Check all the machines which connected to me to make sure they are still alive
	void CheckPings();
	// Drop clients we haven't heard from in a while
	void ExitDeadSockets();
	// Gets freq depending on type of client (irc client or server)
	int GetClientPing( CIRCMachine* pMachine );

	// Check for messages from the uplink servers we connected to
	void PollServers();
	// All uplink I/O comes here ready to use
	bool xRecvData( CIRCUplinkServer* pUplink ); // Receive message from uplink remote server

	// This cracks all our messages for us
	bool ParseRequest( BYTE * pRequest, int len, CIRCCmds* pCmds );
	// All I/O from the game server comes here ready to use
	bool OnRequest( CClient* pClient, BYTE * pRequest, int len );
	// Local machines logging in get processed in here
	bool OnMachineRequest( CClient* pClient, BYTE * pRequest, int len );
	// Local clients get processed here
	bool OnLClientRequest( CIRCLRClient *pLClient, BYTE * pRequest, int len );
	// Downlinks (servers that connected to us) are processed here
	bool OnDownlinkServerRequest( CClient* pClient, BYTE * pRequest, int len );
	// Uplinks (servers we connected to) are processed here
	bool OnUplinkRequest( CIRCUplinkServer* pUplink, BYTE * pRequest, int len );

	// Does this channel really exist? If not and client is valid, then make it
	CIRCChannel* HuntChannel( char* szChName, char* szKey, CIRCMachine *pMachine );

	// Find a server by it's numnick
	CIRCServerNick* FindServerNick( char* szServerNick );
	// Find a server by it's host name
	CIRCServer* FindServerHost( char* szServerHost );
	// Is someone using this or are we tracking it?
	CIRCNick* FindNick( char* szNick );
	// Is soneone using this nick right now?
	CIRCLRClient* HuntClientByNick( char* szNick );
	// Find out you either owns this nick or used to own it
	CIRCLRClient* ChaseClientByNick( char* szNick );
	// Find out who this ip belongs to
	CIRCLRClient* HuntClientByIP( CSocketAddressIP iaLRClientIP );

	// Use this to send a command to one client or server
	void _cdecl SendToOne( CIRCMachine *pMachine, char* szPattern, ... );
	// Helper for SendToOne
	void SendBufferToOne( CIRCMachine *pMachine );
	// Use this to send a command to all servers in our lists, except pServer
	void _cdecl SendToServers( CIRCServer* pServer, char* szPattern, ... );
	// Send this to all the clients on this channel, except the pException
	void _cdecl SendToChannel( CIRCChannel *pChannel, CIRCLRClient* pException, CIRCServer* pServer, char *pattern, ... );
	// Send out to all locally connected IRCCops
	void _cdecl SendToOperators( CIRCMachine *pSkip, char *pattern, ... );

	// Removes illegal characters from channel names
	void CleanChannelName(char *cn);
	// CREATE/JOIN the channel
	void JoinChannel( char* szChName, char* szKey, char* szNick, CIRCServer* pServer );
	// This client is leaving the channel
	void PartChannel(CIRCLRClient* pLRClient, char* szChName, char* szReason = NULL);
	// Now one is left, so delete this channel
	void DestroyChannel(CIRCChannel* pChannel);
	// Make a new channel instance
	CIRCChannel* CreateNewChannel( CIRCMachine* pMachine, char* szChName, char* szKey );
	// Kick this person out of the channel
	void KickFromChannel( CIRCLRClient* pLRClient, char* szChName, char* szNick, char* szComment );
	// Are these 2 in the same channel?
	bool InSameChannel( CIRCLRClient* pLRClient1, CIRCLRClient* pLRClient2 );

	// We use this to drop a client manually
	void ExitClient( CIRCLRClient* pLClient, char* szReason );
	// The game server will use this for clients that are dead
	void ClientQuit( CClient* pClient );
	// This one left properly
	void ClientQuit( CIRCLRClient* pLRClient, char* szReason );
	// This server left properly
	void ServerQuit( CIRCServer* pServer, char* szReason );

	// Machine add/remove functions
	void AddServer( CIRCServer *pServer );
	void RemoveMachine(CIRCMachine *pMachine);
	void NewConnection(CIRCMachine *pMachine);

	// Machine searching functions
	CIRCMachine* FindMachine( CClient* pClient );
	CIRCLRClient* FindLClient( CClient* pClient );
	CIRCDownlinkServer* FindDownlinkServer( CClient* pClient );
	bool IsMyClient( CIRCMachine* pMachine );
	int CountInvisibleClients();

	// Handle incoming messages

	// We only send this ourself or get this from other servers ( CREATE command)
	void Do_CREATE( CIRCServer* pServer, char* szNick, char* szChName, time_t tChanTS );
	// All nick messages from servers come here
	bool Do_NICK_Server( CIRCServer* pServer, CIRCCmd* pCmd );
	// A remote client is being introduced to us
	bool Do_NICK_New( CIRCServer* pServer, CIRCCmd* pCmd );
	// Send this remote server all our nicks, channels, and the nicks and channels behind us too
	void Do_EOB_ACK( CIRCServer* pServer );
	// An existing remote client is changing their nick
	bool Do_NICK_Change( CIRCServer* pServer, CIRCCmd* pCmd );
	// WHO command
	void Do_WHO( CIRCMachine* pMachine, char* szQuery, char* szOperParam );
	void BuildWhoSymbol( SWhoList* pWhoList, CIRCChannel* pChannel );
	// Handles basic CMODE (channel mode) commands from clients
	void Do_CMODE( CIRCCmd* pCmd, CIRCLRClient* pLRClient );
	// Handles basic UMODE (user mode) commands from clients
	void Do_UMODE( CIRCCmd* pCmd, CIRCMachine* pMachine );
	void SetSMask( CIRCMachine* pMachine, DWORD dwNewMask, DWORD dwType );
	void Do_USERHOST( CIRCMachine* pMachine, CIRCCmd* pCmd );
	int Do_WALLCHOPS( CIRCMachine* pMachine, CIRCCmd* pCmd );
	int Do_Message( CIRCMachine* pMachine, CIRCCmd* pCmd, bool fNotice );
	int Do_NOTICE( CIRCMachine* pMachine, CIRCCmd* pCmd );
	void Do_OPER( CIRCLRClient *pLClient, CIRCCmd* pCmd );

	// Send out going messages
	void Send_TIME(CIRCMachine *pMachine );
	void Send_LUSERS(CIRCMachine *pMachine );
	void Send_MOTD(CIRCMachine *pMachine );
	void Send_PING(CIRCMachine *pMachine );
	void Send_VERSION( CIRCMachine *pMachine );
	void Send_ADMIN( CIRCMachine* pMachine );
	void Send_INFO( CIRCMachine* pMachine );
	void Send_RPL_LISTSTART(CIRCMachine *pMachine);
	void Send_RPL_LIST(CIRCMachine *pMachine, int iMin, int iMax, char *pszName);
	void Send_RPL_LISTEND(CIRCMachine *pMachine);
	void Send_RPL_BANLIST( CIRCMachine *pMachine, char* pszChName );
	void Send_RPL_ENDOFBANLIST(CIRCMachine *pMachine, char* pszChName );
	void Send_RPL_TOPIC( CIRCMachine *pMachine, CIRCChannel* pChannel );
	void Send_NICK( CIRCMachine *pMachine, char* pszNick );
	void Send_HackNotice( CIRCServer* pServer, int iBadOp, int iType );
	void Send_UMODE_Out( CIRCMachine* pMachine, DWORD dwSetFlags);
	void Send_UMODE( CIRCMachine* pTo, CIRCMachine* pFrom, DWORD dwOld, DWORD dwSendMask);

	int GetTotalClients();
	int GetTotalInvisibleClients();

	// This checks the validity of the nick (invalid characters)
	static int AnalyzeNick( char *szNick );

	int OverTargetLimit( CIRCLRClient* pLRClient );

	int MakeNumNick( CIRCNumNick* pNumNick, char* szNumNick )
	{
		// Return a numnick in string form to send out to other servers
		// [0] = my ID on the network, [1]  & [2] = the client's slot in the numnick array
		assert( pNumNick );
		int iSlot = pNumNick->m_iSlot;
		char i0;
		CIRCNick* pNick = pNumNick->m_pNick;
		CIRCLRClient* pLRClient;
		if ( pNick )
			pLRClient = pNick->m_pLRClient;
		if ( IsMyClient ( pLRClient ) )
		{
			i0 = m_szNick[0];
		}
		else
		{
			if ( pLRClient->m_pServer )
			{
				i0 = pLRClient->m_pServer->m_szNick[0];
			}
		}
		char i1 = iSlot >> NUMNICKLOG;
		char i2 = iSlot - ( i1 << NUMNICKLOG );
		sprintf( szNumNick, "%c%c%c", i0, CIRCTools::sm_cConvert2y[i1], CIRCTools::sm_cConvert2y[i2] );
		return iSlot;
	}
};

extern CIRCLocalServer g_IRCLocalServer;

