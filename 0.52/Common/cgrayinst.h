//
// Cgrayinst.h
// Copyright Menace Software (www.menasoft.com).
//

#ifndef _INC_CGRAYINST_H
#define _INC_CGRAYINST_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../common/graymul.h"
#include "../common/cfile.h"
#include "../common/carray.h"

#if 0
#define m_fMap0			m_File[VERFILE_MAP]
#define m_fStaIDX0		m_File[VERFILE_STAIDX]
#define m_fStatics0		m_File[VERFILE_STATICS]
#define m_fArt			m_File[VERFILE_ART]
#define m_fArtIDX		m_File[VERFILE_ARTIDX]
#define m_fTileData		m_File[VERFILE_TILEDATA]
#define m_fFont			m_File[VERFILE_FONTS]
#define m_fTexIdx		m_File[VERFILE_TEXIDX]
#define m_fTexMaps		m_File[VERFILE_TEXMAPS]
#define m_fGumpIdx		m_File[VERFILE_GUMPIDX]
#define m_fGumpArt		m_File[VERFILE_GUMPART]
#define m_fAnimIdx		m_File[VERFILE_ANIMIDX]
#define m_fAnim			m_File[VERFILE_ANIM]
#define m_fSoundIdx		m_File[VERFILE_SOUNDIDX]
#define m_fSound		m_File[VERFILE_SOUND]
#define m_fLightIdx		m_File[VERFILE_LIGHTIDX]
#define m_fLight		m_File[VERFILE_LIGHT]
#define m_fSkillsIdx	m_File[VERFILE_SKILLSIDX]
#define m_fSkills		m_File[VERFILE_SKILLS]
#define m_fMultiIdx		m_File[VERFILE_MULTIIDX]
#define m_fMulti		m_File[VERFILE_MULTI]
#define m_fRadarCol		m_File[VERFILE_RADARCOL]
#define m_fHues			m_File[VERFILE_HUES]
#define m_fVerData		m_File[VERFILE_VERDATA]
#endif

////////////////////////////////////////////////////////

extern struct CGrayInstall
{
	// "Software\\Origin Worlds Online\\Ultima Online\\1.0"
	// bool m_fFullInstall;	// Are all files avail ?
private:
	CGString m_sPreferPath;	// Prefer path in which to choose the files. (look here FIRST)
	CGString m_sExePath;		// Files that are installed. "ExePath"
	CGString m_sCDPath;		// For files that may still be on the CD. "InstCDPath"
public:
	CFileBin m_File[ VERFILE_QTY ];	// Get our list of files we need to access.

public:
	CGString GetFullExePath( const TCHAR * pszName = NULL ) const
	{
		return( GetMergedFileName( m_sExePath, pszName ));
	}
	CGString GetFullExePath( VERFILE_TYPE i ) const
	{
		return( GetFullExePath( GetBaseFileName( i )));
	}
	CGString GetFullCDPath( const TCHAR * pszName = NULL ) const
	{
		return( GetMergedFileName( m_sCDPath, pszName ));
	}
	CGString GetFullCDPath( VERFILE_TYPE i ) const
	{
		return( GetFullCDPath( GetBaseFileName( i )));
	}

public:
	bool FindInstall();
	VERFILE_TYPE OpenFiles( DWORD dwMask );
	bool OpenFile( CFileBin & file, const TCHAR * pszName, WORD wFlags );
	bool OpenFile( VERFILE_TYPE i );
	void CloseFiles();

	static const TCHAR * GetBaseFileName( VERFILE_TYPE i );
	CFileBin * GetMulFile( VERFILE_TYPE i )
	{
		ASSERT( i<VERFILE_QTY );
		return( &(m_File[i]));	
	}

	// set the attributes of a specific file.
	bool SetMulFile( VERFILE_TYPE i, const TCHAR * pszName );

	void SetPreferPath( const TCHAR * pszName )
	{
		m_sPreferPath = pszName;
	}
	CGString GetPreferPath( const TCHAR * pszName = NULL ) const
	{
		return( GetMergedFileName( m_sPreferPath.IsEmpty() ? m_sExePath : m_sPreferPath, pszName ));
	}
	CGString GetPreferPath( VERFILE_TYPE i ) const
	{
		return( GetPreferPath( GetBaseFileName( i )));
	}

	bool ReadMulIndex( VERFILE_TYPE fileindex, VERFILE_TYPE filedata, DWORD id, CUOIndexRec & Index );
	bool ReadMulData( VERFILE_TYPE filedata, const CUOIndexRec & Index, void * pData );

} g_Install;

///////////////////////////////////////////////////////////////////////////////

extern class CVerDataMul
{
	// Find verison diffs to the files listed.
	// Assume this is a sorted array of some sort.
public:
	CGTypedArray < CUOVersionBlock, CUOVersionBlock& > m_Data;
private:
	//static int _cdecl Compare( const void * p1, const void * p2 );
public:
	DWORD GetCount() const
	{
		return( m_Data.GetCount());
	}
	const CUOVersionBlock * GetEntry( DWORD i ) const
	{
		return( &m_Data.ElementAt(i));
	}
	void Unload()
	{
		m_Data.Empty();
	}
	~CVerDataMul()
	{
		Unload();
	}
	void Load();
	bool FindVerDataBlock( VERFILE_TYPE type, DWORD id, CUOIndexRec & Index ) const;

} g_VerData;

#endif	// _INC_CGRAYINST_H