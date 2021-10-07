//
// CFileList.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CFILELIST_H
#define _INC_CFILELIST_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CAssoc.h"
#include <time.h>

class CFileList : public CGStringList
{
public:
	static bool ReadFileInfo( LPCTSTR pszFilePath, time_t & dwDateChange, DWORD & dwSize );
	int ReadDir( LPCTSTR pszFilePath );
};

#endif	// _INC_CFILELIST_H
