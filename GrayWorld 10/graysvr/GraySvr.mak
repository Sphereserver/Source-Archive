# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=GraySvr - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to GraySvr - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "GraySvr - Win32 Release" && "$(CFG)" !=\
 "GraySvr - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "GraySvr.mak" CFG="GraySvr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GraySvr - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "GraySvr - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "GraySvr - Win32 Debug"
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
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\GraySvr.exe"

CLEAN : 
	-@erase "$(INTDIR)\CChar.obj"
	-@erase "$(INTDIR)\CClient.obj"
	-@erase "$(INTDIR)\CItem.obj"
	-@erase "$(INTDIR)\CString.obj"
	-@erase "$(INTDIR)\CWorld.obj"
	-@erase "$(INTDIR)\graysvr.obj"
	-@erase "$(INTDIR)\GraySvr.res"
	-@erase "$(OUTDIR)\GraySvr.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/GraySvr.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/GraySvr.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GraySvr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/GraySvr.pdb" /machine:I386 /out:"$(OUTDIR)/GraySvr.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CChar.obj" \
	"$(INTDIR)\CClient.obj" \
	"$(INTDIR)\CItem.obj" \
	"$(INTDIR)\CString.obj" \
	"$(INTDIR)\CWorld.obj" \
	"$(INTDIR)\graysvr.obj" \
	"$(INTDIR)\GraySvr.res"

"$(OUTDIR)\GraySvr.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

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
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\GraySvr.exe"

CLEAN : 
	-@erase "$(INTDIR)\CChar.obj"
	-@erase "$(INTDIR)\CClient.obj"
	-@erase "$(INTDIR)\CItem.obj"
	-@erase "$(INTDIR)\CString.obj"
	-@erase "$(INTDIR)\CWorld.obj"
	-@erase "$(INTDIR)\graysvr.obj"
	-@erase "$(INTDIR)\GraySvr.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\GraySvr.exe"
	-@erase "$(OUTDIR)\GraySvr.ilk"
	-@erase "$(OUTDIR)\GraySvr.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/GraySvr.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/GraySvr.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GraySvr.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/GraySvr.pdb" /debug /machine:I386 /out:"$(OUTDIR)/GraySvr.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CChar.obj" \
	"$(INTDIR)\CClient.obj" \
	"$(INTDIR)\CItem.obj" \
	"$(INTDIR)\CString.obj" \
	"$(INTDIR)\CWorld.obj" \
	"$(INTDIR)\graysvr.obj" \
	"$(INTDIR)\GraySvr.res"

"$(OUTDIR)\GraySvr.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "GraySvr - Win32 Release"
# Name "GraySvr - Win32 Debug"

!IF  "$(CFG)" == "GraySvr - Win32 Release"

!ELSEIF  "$(CFG)" == "GraySvr - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "GraySvr - Win32 Release"

!ELSEIF  "$(CFG)" == "GraySvr - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CClient.cpp
DEP_CPP_CCLIE=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CCLIE=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\CClient.obj" : $(SOURCE) $(DEP_CPP_CCLIE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\CItem.cpp
DEP_CPP_CITEM=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CITEM=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\CItem.obj" : $(SOURCE) $(DEP_CPP_CITEM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\CString.cpp
DEP_CPP_CSTRI=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CSTRI=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\CString.obj" : $(SOURCE) $(DEP_CPP_CSTRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\graysvr.cpp
DEP_CPP_GRAYS=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_GRAYS=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\graysvr.obj" : $(SOURCE) $(DEP_CPP_GRAYS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\graysvr.h

!IF  "$(CFG)" == "GraySvr - Win32 Release"

!ELSEIF  "$(CFG)" == "GraySvr - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GraySvr.rc

"$(INTDIR)\GraySvr.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\CChar.cpp
DEP_CPP_CCHAR=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CCHAR=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\CChar.obj" : $(SOURCE) $(DEP_CPP_CCHAR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\CWorld.cpp
DEP_CPP_CWORL=\
	".\graysvr.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_CWORL=\
	".\linux\in.h"\
	".\linux\net.h"\
	".\linux\tcp.h"\
	".\netdb.h"\
	".\sys\socket.h"\
	

"$(INTDIR)\CWorld.obj" : $(SOURCE) $(DEP_CPP_CWORL) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
