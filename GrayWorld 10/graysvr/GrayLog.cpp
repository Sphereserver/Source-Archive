//
// GrayLog.CPP
//
// Login server for grayworld.
// Some have expressed the need to have a spererate login server app 
// to take the load off Main GrayWorld server while people fail/retry logins.
//
// NOTE: This uses the same port as GrayWorld. 
// It MUST BE RUN ON A SEPERATE MACHINE
//
// http://www.ayekan.com/awesomedev
// or see my page: http://www.menasoft.com for more details.

#include "..\graysvr\graycom.h"

#define GRAY_TITLE		"GrayLog"
#define GRAY_VERSION	"0.01" 

class CLogServer : public CSockets
{

};

int main( int argc, char *argv[] )
{
#ifdef _WIN32
	SetConsoleTitle( GRAY_TITLE " V" GRAY_VERSION );
#endif
	printf( GRAY_TITLE " V" GRAY_VERSION 
#ifdef _WIN32
		" for Win32\n"
#else
		" for Linux\n"
#endif
		"Client Version: " GRAY_CLIENT_STR "\n"
		"Compiled at " __DATE__ " (" __TIME__ " " GRAY_TIMEZONE ")\n"
		"Compiled by " GRAY_NAME " <" GRAY_EMAIL ">\n"
		"\n" );

	Log.SocketsInit();

	while ( 1 )
	{
		if ( kbhit())
		{
			// Send the char over to the server.
			char ch = getch();
			if ( ch == 0x1b ) break;	// ESCAPE KEY
		}

		// Look for incoming data.
		if ( ! Log.SocketsReceive())
			break;
	}

	Log.SocketsClose();
	return( 0 );
}

