# Microsoft Developer Studio Project File - Name="STUNS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=STUNS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "STUNS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "STUNS.mak" CFG="STUNS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "STUNS - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "STUNS - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "STUNS - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "ucl" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"STUNS.exe"

!ELSEIF  "$(CFG)" == "STUNS - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "UCL" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"STUNS.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "STUNS - Win32 Release"
# Name "STUNS - Win32 Debug"
# Begin Group "Main"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\FBaseDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\FBaseDecoder.h
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# End Group
# Begin Group "Deflate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Deflate\FDeflateDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Deflate\FDeflateDecoder.h
# End Source File
# Begin Source File

SOURCE=.\Deflate\puff.c
# End Source File
# Begin Source File

SOURCE=.\Deflate\puff.h
# End Source File
# End Group
# Begin Group "PKWare"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PKWare\blast.c
# End Source File
# Begin Source File

SOURCE=.\PKWare\blast.h
# End Source File
# Begin Source File

SOURCE=.\PKWare\FPKWareDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\PKWare\FPKWareDecoder.h
# End Source File
# End Group
# Begin Group "LZO"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LZO\FLZODecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\LZO\FLZODecoder.h
# End Source File
# Begin Source File

SOURCE=.\LZO\lzoconf.h
# End Source File
# Begin Source File

SOURCE=.\LZO\minilzo.c
# End Source File
# Begin Source File

SOURCE=.\LZO\minilzo.h
# End Source File
# End Group
# Begin Group "UCL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UCL\alloc.c
# End Source File
# Begin Source File

SOURCE=.\UCL\FUCL2BDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\UCL\FUCL2BDecoder.h
# End Source File
# Begin Source File

SOURCE=.\UCL\FUCL2DDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\UCL\FUCL2DDecoder.h
# End Source File
# Begin Source File

SOURCE=.\UCL\getbit.h
# End Source File
# Begin Source File

SOURCE=.\UCL\internal.h
# End Source File
# Begin Source File

SOURCE=.\UCL\io.c
# End Source File
# Begin Source File

SOURCE=.\UCL\n2b_d.c
# End Source File
# Begin Source File

SOURCE=.\UCL\n2b_ds.c
# End Source File
# Begin Source File

SOURCE=.\UCL\n2d_d.c
# End Source File
# Begin Source File

SOURCE=.\UCL\n2d_ds.c
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl.h
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_conf.h
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_crc.c
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_ptr.c
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_ptr.h
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_str.c
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_util.c
# End Source File
# Begin Source File

SOURCE=.\UCL\ucl_util.h
# End Source File
# Begin Source File

SOURCE=.\UCL\uclconf.h
# End Source File
# Begin Source File

SOURCE=.\UCL\uclutil.h
# End Source File
# End Group
# End Target
# End Project
