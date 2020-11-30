//
// NTWindow.CPP
// Copyright Menace Software (www.menasoft.com).
//
// Put up a window for data (other than the console)
// This whole idea just does NOT WORK ! NT blocks this.
//

#ifdef _WIN32

#include "graysvr.h"	// predef header.
#include "../common/cwindow.h"

class CNTWindow : public CWindow
{
	// Create a dialog to do stuff.
private:
	void OnCreate( LPCREATESTRUCT lParam );

};

#endif
