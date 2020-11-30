# Microsoft Developer Studio Project File - Name="GraySvr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=GraySvr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GraySvr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GraySvr.mak" CFG="GraySvr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GraySvr - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "GraySvr - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GraySvr", FCAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GraySvr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W3 /GR /GX /Ot /Oi /Ob2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "GRAY_SVR" /FR /YX /FD /c
# SUBTRACT CPP /Og
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib libcmt.lib kernel32.lib advapi32.lib user32.lib /nologo /version:0.10 /subsystem:console /debug /machine:I386 /nodefaultlib

!ELSEIF  "$(CFG)" == "GraySvr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /MTd /W2 /GR /GX /ZI /Ot /Oi /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "GRAY_SVR" /FR /YX /J /FD /c
# SUBTRACT CPP /Og
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib libcmt.lib kernel32.lib advapi32.lib user32.lib /nologo /version:0.12 /subsystem:console /map /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ENDIF 

# Begin Target

# Name "GraySvr - Win32 Release"
# Name "GraySvr - Win32 Debug"
# Begin Group "Scripts"

# PROP Default_Filter "scp,ini"
# Begin Source File

SOURCE=.\sphere.ini
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereBook.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spherechar.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spherechar2.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheredefs.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereGump.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spherehelp.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereitem.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereitem2.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheremap.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheremap2.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheremenu.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereNAME.SCP
# End Source File
# Begin Source File

SOURCE=..\scripts\spherenewb.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\sphereskill.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spherespee.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheretables.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheretemplate.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheretrig.scp
# End Source File
# Begin Source File

SOURCE=..\scripts\spheretrig2.scp
# End Source File
# End Group
# Begin Group "Web Pages"

# PROP Default_Filter "htm"
# Begin Source File

SOURCE=..\public_html\gray\dev.htm
# End Source File
# Begin Source File

SOURCE=..\public_html\gray\ideas.htm
# End Source File
# Begin Source File

SOURCE=..\public_html\gray\index.html
# End Source File
# Begin Source File

SOURCE=..\public_html\gray\readme.htm
# End Source File
# End Group
# Begin Group "Sources"

# PROP Default_Filter "cpp,c,h,rc"
# Begin Source File

SOURCE=.\CAccount.cpp
# End Source File
# Begin Source File

SOURCE=.\CBackTask.cpp
# End Source File
# Begin Source File

SOURCE=.\cbase.cpp
# End Source File
# Begin Source File

SOURCE=.\CChar.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharBase.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharFight.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharNPC.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharNPCAct.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharNPCFood.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharNPCPet.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharNPCStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\ccharskill.cpp
# End Source File
# Begin Source File

SOURCE=.\CCharStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\ccharuse.cpp
# End Source File
# Begin Source File

SOURCE=.\CChat.cpp
# End Source File
# Begin Source File

SOURCE=.\CClient.cpp
# End Source File
# Begin Source File

SOURCE=.\cclientevent.cpp
# End Source File
# Begin Source File

SOURCE=.\CClientLog.cpp
# End Source File
# Begin Source File

SOURCE=.\CClientMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\cclienttarg.cpp
# End Source File
# Begin Source File

SOURCE=.\CContain.cpp
# End Source File
# Begin Source File

SOURCE=.\cfragment.cpp
# End Source File
# Begin Source File

SOURCE=.\CItem.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemBase.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemSp.cpp
# End Source File
# Begin Source File

SOURCE=.\CItemStone.cpp
# End Source File
# Begin Source File

SOURCE=.\CMail.cpp
# End Source File
# Begin Source File

SOURCE=.\csector.cpp
# End Source File
# Begin Source File

SOURCE=.\CServer.cpp
# End Source File
# Begin Source File

SOURCE=.\cvendoritem.cpp
# End Source File
# Begin Source File

SOURCE=.\CWorld.cpp
# End Source File
# Begin Source File

SOURCE=.\cworldimport.cpp
# End Source File
# Begin Source File

SOURCE=.\cworldmap.cpp
# End Source File
# Begin Source File

SOURCE=.\graysvr.cpp
# End Source File
# Begin Source File

SOURCE=.\ntservice.cpp
# End Source File
# Begin Source File

SOURCE=.\ntwindow.cpp
# End Source File
# End Group
# Begin Group "Data Files"

# PROP Default_Filter "scp"
# Begin Source File

SOURCE=.\docs\items.txt
# End Source File
# Begin Source File

SOURCE=.\docs\npcs.txt
# End Source File
# Begin Source File

SOURCE=.\docs\sounds.txt
# End Source File
# Begin Source File

SOURCE=.\docs\terrain.txt
# End Source File
# Begin Source File

SOURCE=.\docs\UoLog.txt
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\graysvr.h
# End Source File
# Begin Source File

SOURCE=.\graysvr.rc
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

SOURCE=..\common\ccrypt.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cexpression.cpp
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

SOURCE=..\common\cfilelist.cpp
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

SOURCE=..\common\cGrayMap.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cGrayMap.h
# End Source File
# Begin Source File

SOURCE=..\common\common.h
# End Source File
# Begin Source File

SOURCE=..\common\cregion.cpp
# End Source File
# Begin Source File

SOURCE=..\common\cregion.h
# End Source File
# Begin Source File

SOURCE=..\common\cscript.cpp
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

SOURCE=..\common\graycom.cpp
# End Source File
# Begin Source File

SOURCE=..\common\graycom.h
# End Source File
# Begin Source File

SOURCE=..\common\graymul.h
# End Source File
# Begin Source File

SOURCE=..\common\grayproto.h
# End Source File
# End Group
# Begin Group "Speech"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\speech\convbye.scp
# End Source File
# Begin Source File

SOURCE=.\speech\horse.scp
# End Source File
# Begin Source File

SOURCE=.\speech\humangeneral.scp
# End Source File
# Begin Source File

SOURCE=.\speech\humanprime.scp
# End Source File
# Begin Source File

SOURCE=.\speech\humanunknown.scp
# End Source File
# Begin Source File

SOURCE=.\speech\humanworld.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobactor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobalchemist.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobanimal.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobarchitect.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobarmourer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobartist.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobbaker.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobbanker.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobbard.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobbeekeeper.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobBrigand.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobbutcher.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobcarpenter.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobcobbler.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobfarmer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobfurtrader.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobgambler.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobglassblower.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobGuard.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobherbalist.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobjailor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobjudge.scp
# End Source File
# Begin Source File

SOURCE=.\speech\joblaborer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobMage.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobMageEvil.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobMageShop.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobmayor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobmiller.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobminer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobminter.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobmonk.scp
# End Source File
# Begin Source File

SOURCE=.\speech\joboverseer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobparliament.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobpirate.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobpriest.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobprisoner.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobrancher.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobrealtor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobrunner.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobsailor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobscholar.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobscribe.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobsculptor.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobservant.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobshepherd.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobshipwright.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobtanner.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobtinker.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobveggie.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobwaiter.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobweaponstrainer.scp
# End Source File
# Begin Source File

SOURCE=.\speech\jobweaver.scp
# End Source File
# Begin Source File

SOURCE=.\speech\master.scp
# End Source File
# Begin Source File

SOURCE=.\speech\needs.scp
# End Source File
# Begin Source File

SOURCE=.\speech\orc.scp
# End Source File
# Begin Source File

SOURCE=.\speech\rehello.scp
# End Source File
# Begin Source File

SOURCE=.\speech\shopkeep.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townbritain.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townjhelom.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townmagincia.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townmoonglow.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townnujelm.scp
# End Source File
# Begin Source File

SOURCE=.\speech\townvesper.scp
# End Source File
# Begin Source File

SOURCE=.\speech\traveler.scp
# End Source File
# End Group
# Begin Group "events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\speech\eacceptitem.scp
# End Source File
# Begin Source File

SOURCE=.\speech\econvinit.scp
# End Source File
# Begin Source File

SOURCE=.\speech\egreetings.scp
# End Source File
# Begin Source File

SOURCE=.\speech\ehearunk.scp
# End Source File
# Begin Source File

SOURCE=.\speech\einternalspace.scp
# End Source File
# Begin Source File

SOURCE=.\speech\eneedresponse.scp
# End Source File
# Begin Source File

SOURCE=.\speech\erefuseitem.scp
# End Source File
# Begin Source File

SOURCE=.\speech\escavenger.scp
# End Source File
# End Group
# Begin Source File

SOURCE=.\GraySvr.ico
# End Source File
# Begin Source File

SOURCE=.\REVISIONS.txt
# End Source File
# Begin Source File

SOURCE=.\spheresvr.ico
# End Source File
# Begin Source File

SOURCE=.\TUSSvr.ico
# End Source File
# Begin Source File

SOURCE=.\TusSvr2.ico
# End Source File
# End Target
# End Project
