# Microsoft Developer Studio Project File - Name="GrayMaker" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GrayMaker - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GrayMaker.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GrayMaker.mak" CFG="GrayMaker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GrayMaker - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GrayMaker - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GrayMaker - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "GRAY_MAKER" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib winmm.lib dsound.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "GrayMaker - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "GRAY_MAKER" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib winmm.lib dsound.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GrayMaker - Win32 Release"
# Name "GrayMaker - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CDataAnim.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataDef.cpp
# End Source File
# Begin Source File

SOURCE=.\cDataFont.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataGump.cpp
# End Source File
# Begin Source File

SOURCE=.\cDataHue.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataItem.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataLight.cpp
# End Source File
# Begin Source File

SOURCE=.\cDataList.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataPage.cpp
# End Source File
# Begin Source File

SOURCE=.\cdataprops.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataSkill.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataSound.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataTerrain.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataTileData.cpp
# End Source File
# Begin Source File

SOURCE=.\CDataVerData.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\GrayClient\cSound.cpp
# End Source File
# Begin Source File

SOURCE=..\GrayClient\CSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\GrayClient\cTileMul.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayMaker.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayMaker.rc
# End Source File
# Begin Source File

SOURCE=.\GrayMakerDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayMakerView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CDataAnim.h
# End Source File
# Begin Source File

SOURCE=.\CDataDef.h
# End Source File
# Begin Source File

SOURCE=.\cDataFont.h
# End Source File
# Begin Source File

SOURCE=.\CDataGump.h
# End Source File
# Begin Source File

SOURCE=.\cDataHue.h
# End Source File
# Begin Source File

SOURCE=.\CDataItem.h
# End Source File
# Begin Source File

SOURCE=.\CDataLight.h
# End Source File
# Begin Source File

SOURCE=.\cDataList.h
# End Source File
# Begin Source File

SOURCE=.\cdatamulti.h
# End Source File
# Begin Source File

SOURCE=.\CDataPage.h
# End Source File
# Begin Source File

SOURCE=.\cdataprops.h
# End Source File
# Begin Source File

SOURCE=.\CDataSkill.h
# End Source File
# Begin Source File

SOURCE=.\CDataSound.h
# End Source File
# Begin Source File

SOURCE=.\CDataTerrain.h
# End Source File
# Begin Source File

SOURCE=.\CDataTexture.h
# End Source File
# Begin Source File

SOURCE=.\CDataTileData.h
# End Source File
# Begin Source File

SOURCE=.\CDataVerData.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=..\GrayClient\csound.h
# End Source File
# Begin Source File

SOURCE=..\GrayClient\cTileMul.h
# End Source File
# Begin Source File

SOURCE=.\GrayMaker.h
# End Source File
# Begin Source File

SOURCE=.\GrayMakerDoc.h
# End Source File
# Begin Source File

SOURCE=.\GrayMakerView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\BtnHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\GrayMaker.ico
# End Source File
# Begin Source File

SOURCE=.\res\GrayMaker.ico
# End Source File
# Begin Source File

SOURCE=.\res\GrayMaker.rc2
# End Source File
# Begin Source File

SOURCE=.\GrayMakerDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\GrayMakerDoc.ico
# End Source File
# Begin Source File

SOURCE=.\iAnim.ico
# End Source File
# Begin Source File

SOURCE=.\res\iAnim.ico
# End Source File
# Begin Source File

SOURCE=.\iFont.ico
# End Source File
# Begin Source File

SOURCE=.\res\ifont.ico
# End Source File
# Begin Source File

SOURCE=.\iGump.ico
# End Source File
# Begin Source File

SOURCE=.\res\iGump.ico
# End Source File
# Begin Source File

SOURCE=.\iHue.ico
# End Source File
# Begin Source File

SOURCE=.\res\iHue.ico
# End Source File
# Begin Source File

SOURCE=.\iItem.ico
# End Source File
# Begin Source File

SOURCE=.\res\iItem.ico
# End Source File
# Begin Source File

SOURCE=.\iLight.ico
# End Source File
# Begin Source File

SOURCE=.\res\iLight.ico
# End Source File
# Begin Source File

SOURCE=.\iMulti.ico
# End Source File
# Begin Source File

SOURCE=.\res\iMulti.ico
# End Source File
# Begin Source File

SOURCE=.\iSkill.ico
# End Source File
# Begin Source File

SOURCE=.\res\iSkill.ico
# End Source File
# Begin Source File

SOURCE=.\iSound.ico
# End Source File
# Begin Source File

SOURCE=.\res\iSound.ico
# End Source File
# Begin Source File

SOURCE=.\iTexture.ico
# End Source File
# Begin Source File

SOURCE=.\res\iTexture.ico
# End Source File
# Begin Source File

SOURCE=.\iVerData.ico
# End Source File
# Begin Source File

SOURCE=.\res\iVerData.ico
# End Source File
# Begin Source File

SOURCE=.\PUSHPIND.bmp
# End Source File
# Begin Source File

SOURCE=.\PUSHPINU.bmp
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\carray.cpp
# End Source File
# Begin Source File

SOURCE=..\common\carray.h
# End Source File
# Begin Source File

SOURCE=..\common\cassoc.h
# End Source File
# Begin Source File

SOURCE=..\common\cexpression.h
# End Source File
# Begin Source File

SOURCE=..\common\cfile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cfile.h
# End Source File
# Begin Source File

SOURCE=..\common\cGrayData.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cgrayinst.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cgrayinst.h
# End Source File
# Begin Source File

SOURCE=..\common\cGrayMap.h
# End Source File
# Begin Source File

SOURCE=..\common\cmemblock.h
# End Source File
# Begin Source File

SOURCE=..\common\common.h
# End Source File
# Begin Source File

SOURCE=..\common\cregion.h
# End Source File
# Begin Source File

SOURCE=..\common\cscript.h
# End Source File
# Begin Source File

SOURCE=..\common\cstring.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cstring.h
# End Source File
# Begin Source File

SOURCE=..\common\graybase.h
# End Source File
# Begin Source File

SOURCE=..\common\graycom.h
# End Source File
# Begin Source File

SOURCE=..\common\graymul.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\GrayMaker.reg
# End Source File
# End Target
# End Project
