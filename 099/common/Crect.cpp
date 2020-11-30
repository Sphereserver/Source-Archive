    // Crect.cpp
    // Copyright Menace Software (www.menasoft.com).
    // Common for client and server.
    //
    
    #ifdef GRAY_SVR
    #include "../graysvr/graysvr.h"
    #elif defined(GRAY_MAP)
    #include "../graymap/stdafx.h"
    #include "../graymap/graymap.h"
    #else
    #include "graycom.h"
    #include "graymul.h"
    #include "crect.h"
    #endif
    
    //*************************************************************************
    // -CPointBase
    
    LPCTSTR const CPointBase::sm_szDirs =
    {
    	_TEXT("North"),
    	_TEXT("North East"),
    	_TEXT("East"),
    	_TEXT("South East"),
    	_TEXT("South"),
    	_TEXT("South West"),
    	_TEXT("West"),
    	_TEXT("North West"),
    	_TEXT("Here"),
    };
    
    const int CPointBase::sm_Moves =
    {
    	 0, -1, // DIR_N
    	 1, -1, // DIR_NE
    	 1,  0, // DIR_E
    	 1,  1, // DIR_SE
    	 0,  1, // DIR_S
    	-1,  1, // DIR_SW
    	-1,  0, // DIR_W
    	-1, -1, // DIR_NW
     	 0,	 0,	// DIR_QTY = here.
    };
    
    LPCTSTR const CPointBase::sm_szLoadKeys =
    {
    	"MAPPLANE",
    	"X",
    	"Y",
    	"Z",
    	NULL,
    };
    
    bool CPointBase::r_WriteVal( LPCTSTR pszKey, CGString & sVal ) const
    {
    	int iVal;
    	switch ( FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 ))
    	{
    	case 0: iVal = m_mapplane; break;
    	case 1: iVal = m_x; break;
    	case 2: iVal = m_y; break;
    	case 3: iVal = m_z; break;
    	default:
    		return( false );
    	}
    	sVal.FormatVal(iVal);
    	return( true );
    }
    
    bool CPointBase::r_LoadVal( LPCTSTR pszKey, LPCTSTR pszArgs )
    {
    	int index = FindTableSorted( pszKey, sm_szLoadKeys, COUNTOF(sm_szLoadKeys)-1 );
    	if ( index <= 0 )
    	{
    		return( false );
    	}
    	int iVal = Exp_GetVal(pszArgs);
    	switch (index)
    	{
    	case 0: m_mapplane = iVal; break;
    	case 1: m_x = iVal; break;
    	case 2: m_y = iVal; break;
    	case 3: m_z = iVal; break;
    	}
    	return( true );
    }
    
    int CPointBase::GetDist( const CPointBase & pt ) const // Distance between points
    {
    	// Get the basic 2d distance.
    	if ( ! IsSameMapPlane( pt.m_mapplane ))	// as far apart as possible
    		return( SHRT_MAX );
    	return( GetDistBase( pt ));
    }
    
    int CPointBase::GetDist3D( const CPointBase & pt ) const // Distance between points
    {
    	// OK, 1 unit of Z is not the same (real life) distance as 1
    	// unit of X (or Y)
    	int dist = GetDist(pt);
    
    	// Get the deltas and correct the Z for height first
    	int dz = GetDistZAdj(pt); // Take player height into consideration
    	return( max( dist, dz ));
    }
    
    DIR_TYPE CPointBase::GetDir( const CPointBase & pt, DIR_TYPE DirDefault ) const // Direction to point pt
    {
    	// Get the 2D direction between points.
    	int dx = (m_x-pt.m_x);
    	int dy = (m_y-pt.m_y);
    
    	int ax = abs(dx);
    	int ay = abs(dy);
    
    	if ( ay > ax )
    	{
    		if ( ! ax )
    		{
    			return(( dy > 0 ) ? DIR_N : DIR_S );
    		}
    		int slope = ay / ax;
    		if ( slope > 2 )
    			return(( dy > 0 ) ? DIR_N : DIR_S );
    		if ( dx > 0 )	// westish
    		{
    			return(( dy > 0 ) ? DIR_NW : DIR_SW );
    		}
    		return(( dy > 0 ) ? DIR_NE : DIR_SE );
    	}
    	else
    	{
    		if ( ! ay )
    		{
    			if ( ! dx )
    				return( DirDefault );	// here ?
    			return(( dx > 0 ) ? DIR_W : DIR_E );
    		}
    		int slope = ax / ay;
    		if ( slope > 2 )
    			return(( dx > 0 ) ? DIR_W : DIR_E );
    		if ( dy > 0 )
    		{
    			return(( dx > 0 ) ? DIR_NW : DIR_NE );
    		}
    		return(( dx > 0 ) ? DIR_SW : DIR_SE );
    	}
    }
    
    int CPointBase::StepLinePath( const CPointBase & ptSrc, int iSteps )
    {
    	// Take x steps toward this point.
    	int dx = m_x - ptSrc.m_x;
    	int dy = m_y - ptSrc.m_y;
    	int iDist2D = GetDist( ptSrc );
    	if ( ! iDist2D )
    		return 0;
    
    	m_x = ptSrc.m_x + IMULDIV( iSteps, dx, iDist2D );
    	m_y = ptSrc.m_y + IMULDIV( iSteps, dy, iDist2D );
    	return( iDist2D );
    }
    
    LPCTSTR CPointBase::WriteUsed( void ) const
    {
    	return( WriteUsed( Str_GetTemp()));
    }
    
    int CPointBase ::Read( TCHAR * pszVal )
    {
    	// parse reading the point
    	// NOTE: do not use = as a seperator here !
    	m_z = 0;
    	m_mapplane = 0;
    	TCHAR * ppVal;
    	int iArgs = Str_ParseCmds( pszVal, ppVal, COUNTOF( ppVal ), " ,\t" );
    	switch ( iArgs )
    	{
    	default:
    	case 4:	// m_mapplane
    		if ( isdigit(ppVal))
    		{
    			m_mapplane = atoi(ppVal);
    		}
    	case 3: // m_z
    		if ( isdigit(ppVal) || ppVal == '-' )
    		{
    			m_z = atoi(ppVal);
    		}
    	case 2:
    		m_y = atoi(ppVal);
    	case 1:
    		m_x = atoi(ppVal);
    	case 0:
    		break;
    	}
    	return( iArgs );
    }
    
    void CPointBase::MoveWrap( DIR_TYPE dir )
    {
    	// Check for world wrap.
    	// CPointBase ptOld = *this;
    	Move(dir);
    	if (m_y < 0)
    		m_y += UO_SIZE_Y;
    	if (m_x < 0)
    		m_x += UO_SIZE_X_REAL;
    	if (m_y >= UO_SIZE_Y)
    		m_y -= UO_SIZE_Y;
    	if (m_x >= UO_SIZE_X)
    		m_x -= UO_SIZE_X;
    	if (m_x == UO_SIZE_X_REAL)
    		m_x = 0;
    }
    
    #if defined(GRAY_SVR) || defined(GRAY_MAP)	// server only.
    
    CSector * CPointBase::GetSector() const
    {
    	// Get the world Sector we are in.
    	ASSERT(	m_x >= 0 && m_x < UO_SIZE_X );
    	ASSERT(	m_y >= 0 && m_y < UO_SIZE_Y );
    	return( g_World.GetSector( (( m_y / SECTOR_SIZE_Y ) * SECTOR_COLS ) + ( m_x / SECTOR_SIZE_X ) ));
    }
    
    CRegionBase * CPointBase::GetRegion( DWORD dwType ) const
    {
    	// What region in the current CSector am i in ?
    	// We only need to update this every 8 or so steps ?
    	// REGION_TYPE_AREA
    
    	if ( ! IsValidPoint())
    	{
    		return NULL;
    	}
    
    	CSector * pSector = GetSector();
    	ASSERT(pSector);
    	return( pSector->GetRegion( *this, dwType ));
    }
    
    //*************************************************************************
    // -CGRect
    
    #if 0
    int CGRect::Read( TCHAR * pszBuffer )
    {
    	LPCTSTR pSeps = _TEXT(" ,\t\n");
    	TCHAR *pToken = strtok( pszBuffer, pSeps );
    	for ( int i=0; i<4 && pToken != NULL; i++ )
    	{
    		int iVal = atoi( pToken );
    		switch ( i )
    		{
    		case 0: m_left = iVal; break;
    		case 1: m_top = iVal; break;
    		case 2: m_right = iVal; break;
    		case 3: m_bottom = iVal; break;
    		}
    		pToken = strtok( NULL, pSeps );
    	}
    	NormalizeRect();
    	return( i );
    }
    #endif
    
    int CGRect::Read( LPCTSTR pszVal )
    {
    	// parse reading the rectangle
    	TCHAR szTemp;
    	strcpy( szTemp, pszVal );
    	TCHAR * ppVal;
    	int i = Str_ParseCmds( szTemp, ppVal, COUNTOF( ppVal ), _TEXT(" ,\t"));
    	switch (i)
    	{
    	case 4: m_bottom = atoi(ppVal);
    	case 3: m_right = atoi(ppVal);
    	case 2: m_top =	atoi(ppVal);
    	case 1: m_left = atoi(ppVal);
    	}
    	NormalizeRect();
    	return( i );
    }
    
    LPCTSTR CGRect::Write( void ) const
    {
    	return( Write( Str_GetTemp()));
    }
    
    CPointBase CGRect::GetRectCorner( DIR_TYPE dir ) const
    {
    	// Get the point if a directional corner of the CRectMap.
    	CPointBase pt;
    	pt.m_z = 0;	// NOTE: remember this is a nonsense value.
    	pt.m_mapplane = 0;
    	switch ( dir )
    	{
    	case DIR_N:
    		pt.m_x = ( m_left + m_right ) / 2;
    		pt.m_y = m_top;
    		break;
    	case DIR_NE:
    		pt.m_x = m_right;
    		pt.m_y = m_top;
    		break;
    	case DIR_E:
    		pt.m_x = m_right;
    		pt.m_y = ( m_top + m_bottom ) / 2;
    		break;
    	case DIR_SE:
    		pt.m_x = m_right;
    		pt.m_y = m_bottom;
    		break;
    	case DIR_S:
    		pt.m_x = ( m_left + m_right ) / 2;
    		pt.m_y = m_bottom;
    		break;
    	case DIR_SW:
    		pt.m_x = m_left;
    		pt.m_y = m_bottom;
    		break;
    	case DIR_W:
    		pt.m_x = m_left;
    		pt.m_y = ( m_top + m_bottom ) / 2;
    		break;
    	case DIR_NW:
    		pt.m_x = m_left;
    		pt.m_y = m_top;
    		break;
    	case DIR_QTY:
    		pt = GetCenter();
    		break;
    	}
    	return( pt );
    }
    
    CSector * CGRect::GetSector( int i ) const	// ge all the sectors that make up this rect.
    {
    	// get all the CSector(s) that overlap this rect.
    	// RETURN: NULL = no more
    
    	// Align new rect.
    	CRectMap rect;
    	rect.m_left = m_left &~ (SECTOR_SIZE_X-1);
    	rect.m_right = ( m_right | (SECTOR_SIZE_X-1)) + 1;
    	rect.m_top = m_top &~ (SECTOR_SIZE_Y-1);
    	rect.m_bottom = ( m_bottom | (SECTOR_SIZE_Y-1)) + 1;
    	rect.NormalizeRectMax();
    
    	int width = (rect.GetWidth()) / SECTOR_SIZE_X;
    	ASSERT(width<=SECTOR_COLS);
    	int height = (rect.GetHeight()) / SECTOR_SIZE_Y;
    	ASSERT(height<=SECTOR_ROWS);
    
    	int iBase = (( rect.m_top / SECTOR_SIZE_Y ) * SECTOR_COLS ) + ( rect.m_left / SECTOR_SIZE_X );
    
    	if ( i >= ( height * width ))
    	{
    		if ( ! i )
    		{
    			return( g_World.GetSector(iBase) );
    		}
    		return( NULL );
    	}
    
    	int indexoffset = (( i / width ) * SECTOR_COLS ) + ( i % width );
    
    	return( g_World.GetSector(iBase+indexoffset) );
    }
    
    //*************************************************************************
    // -CRectMap
    
    void CRectMap::NormalizeRect()
    {
    	CGRect::NormalizeRect();
    	NormalizeRectMax();
    }
    
    void CRectMap::NormalizeRectMax()
    {
    	CGRect::NormalizeRectMax( UO_SIZE_X, UO_SIZE_Y );
    }
    
    #endif