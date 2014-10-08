# Microsoft Developer Studio Project File - Name="Epiar" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Epiar - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Epiar.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Epiar.mak" CFG="Epiar - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Epiar - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Epiar - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Epiar - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "SDL-win32\include" /I "SDL_image-win32\include" /I "src\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "NAUDIO" /D "SYS_IS_BRAINDEAD" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Epiar - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "SDL-win32\include" /I "SDL_image-win32\include" /I "src\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "NAUDIO" /D "NDEBUG" /D "SYS_IS_BRAINDEAD" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Epiar - Win32 Release"
# Name "Epiar - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\system\afont_base.c
# End Source File
# Begin Source File

SOURCE=.\src\system\afont_sdl.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai.c
# End Source File
# Begin Source File

SOURCE=.\src\alliances\alliances.c
# End Source File
# Begin Source File

SOURCE=.\src\asteroid\asteroid.c
# End Source File
# Begin Source File

SOURCE=.\src\audio\audio.c
# End Source File
# Begin Source File

SOURCE=.\src\system\video\backbuffer.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\chunk.c
# End Source File
# Begin Source File

SOURCE=.\src\comm\comm.c
# End Source File
# Begin Source File

SOURCE=.\src\system\debug.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\defender.c
# End Source File
# Begin Source File

SOURCE=.\src\system\eaf.c
# End Source File
# Begin Source File

SOURCE=.\src\system\esf.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\explorer.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\fire.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\flare.c
# End Source File
# Begin Source File

SOURCE=.\src\system\font.c
# End Source File
# Begin Source File

SOURCE=.\src\force\force.c
# End Source File
# Begin Source File

SOURCE=.\src\game\game.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\gate.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\gate_defender.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_btab.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_button.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_checkbox.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_commondlg.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_frame.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_image.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_keybox.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_label.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_listbox.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_scrollbar.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_session.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_tab.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_text_entry.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_textbox.c
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_window.c
# End Source File
# Begin Source File

SOURCE=.\src\hud\hud.c
# End Source File
# Begin Source File

SOURCE=.\src\system\init.c
# End Source File
# Begin Source File

SOURCE=.\src\input\input.c
# End Source File
# Begin Source File

SOURCE=.\src\land\land.c
# End Source File
# Begin Source File

SOURCE=.\src\land\land_dlg.c
# End Source File
# Begin Source File

SOURCE=.\src\main.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\maneuvers.c
# End Source File
# Begin Source File

SOURCE=.\src\system\math.c
# End Source File
# Begin Source File

SOURCE=.\src\menu\menu.c
# End Source File
# Begin Source File

SOURCE=.\src\missions\missions.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\model.c
# End Source File
# Begin Source File

SOURCE=.\src\audio\music.c
# End Source File
# Begin Source File

SOURCE=.\src\navigation\navigation.c
# End Source File
# Begin Source File

SOURCE=.\src\network\net_fire.c
# End Source File
# Begin Source File

SOURCE=.\src\network\net_sprite.c
# End Source File
# Begin Source File

SOURCE=.\src\network\network.c
# End Source File
# Begin Source File

SOURCE=.\src\menu\options.c
# End Source File
# Begin Source File

SOURCE=.\src\outfit\outfit.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\particle.c
# End Source File
# Begin Source File

SOURCE=.\src\system\path.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\pirate.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\planet.c
# End Source File
# Begin Source File

SOURCE=.\src\audio\playlist.c
# End Source File
# Begin Source File

SOURCE=.\src\system\plugin.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\r_ships.c
# End Source File
# Begin Source File

SOURCE=.\src\system\rander.c
# End Source File
# Begin Source File

SOURCE=.\src\system\save.c
# End Source File
# Begin Source File

SOURCE=.\src\game\scenario.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\sprite.c
# End Source File
# Begin Source File

SOURCE=.\src\menu\status.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\target.c
# End Source File
# Begin Source File

SOURCE=.\src\system\timer.c
# End Source File
# Begin Source File

SOURCE=.\src\racing\track.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\trader.c
# End Source File
# Begin Source File

SOURCE=.\src\system\trig.c
# End Source File
# Begin Source File

SOURCE=.\src\tutorial\tutorial.c
# End Source File
# Begin Source File

SOURCE=.\src\game\update.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\upgrade.c
# End Source File
# Begin Source File

SOURCE=.\src\system\video\video.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\warship.c
# End Source File
# Begin Source File

SOURCE=.\src\sprite\weapon.c
# End Source File
# Begin Source File

SOURCE=.\src\osdep\win32\win32_misc.c
# End Source File
# Begin Source File

SOURCE=.\src\osdep\win32\win32_video.c
# End Source File
# Begin Source File

SOURCE=.\src\system\video\zoom.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\system\afont.h
# End Source File
# Begin Source File

SOURCE=.\src\system\afont_sdl.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai.h
# End Source File
# Begin Source File

SOURCE=.\src\alliances\alliances.h
# End Source File
# Begin Source File

SOURCE=.\src\audio\audio.h
# End Source File
# Begin Source File

SOURCE=.\src\system\video\backbuffer.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\chunk.h
# End Source File
# Begin Source File

SOURCE=.\src\com_defs.h
# End Source File
# Begin Source File

SOURCE=.\src\comm\comm.h
# End Source File
# Begin Source File

SOURCE=.\src\system\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\defender.h
# End Source File
# Begin Source File

SOURCE=.\src\system\eaf.h
# End Source File
# Begin Source File

SOURCE=.\src\system\esf.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\explorer.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\fire.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\flare.h
# End Source File
# Begin Source File

SOURCE=.\src\system\font.h
# End Source File
# Begin Source File

SOURCE=.\src\force\force.h
# End Source File
# Begin Source File

SOURCE=.\src\game\game.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\gate.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\gate_defender.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_btab.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_button.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_checkbox.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_commondlg.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_frame.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_image.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_keybox.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_label.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_listbox.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_scrollbar.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_session.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_tab.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_text_entry.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_textbox.h
# End Source File
# Begin Source File

SOURCE=.\src\gui\gui_window.h
# End Source File
# Begin Source File

SOURCE=.\src\hud\hud.h
# End Source File
# Begin Source File

SOURCE=.\src\includes.h
# End Source File
# Begin Source File

SOURCE=.\src\system\init.h
# End Source File
# Begin Source File

SOURCE=.\src\input\input.h
# End Source File
# Begin Source File

SOURCE=.\src\land\land.h
# End Source File
# Begin Source File

SOURCE=.\src\land\land_dlg.h
# End Source File
# Begin Source File

SOURCE=.\src\main.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\maneuvers.h
# End Source File
# Begin Source File

SOURCE=.\src\system\math.h
# End Source File
# Begin Source File

SOURCE=.\src\menu\menu.h
# End Source File
# Begin Source File

SOURCE=.\src\missions\missions.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\model.h
# End Source File
# Begin Source File

SOURCE=.\src\audio\music.h
# End Source File
# Begin Source File

SOURCE=.\src\navigation\navigation.h
# End Source File
# Begin Source File

SOURCE=.\src\network\net_fire.h
# End Source File
# Begin Source File

SOURCE=.\src\network\net_sprite.h
# End Source File
# Begin Source File

SOURCE=.\src\network\network.h
# End Source File
# Begin Source File

SOURCE=.\src\menu\options.h
# End Source File
# Begin Source File

SOURCE=.\src\osdep\osdep.h
# End Source File
# Begin Source File

SOURCE=.\src\outfit\outfit.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\particle.h
# End Source File
# Begin Source File

SOURCE=.\src\system\path.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\pirate.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\planet.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\player.h
# End Source File
# Begin Source File

SOURCE=.\src\audio\playlist.h
# End Source File
# Begin Source File

SOURCE=.\src\system\plugin.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\r_ships.h
# End Source File
# Begin Source File

SOURCE=.\src\system\rander.h
# End Source File
# Begin Source File

SOURCE=.\src\system\save.h
# End Source File
# Begin Source File

SOURCE=.\src\game\scenario.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\ship.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\sprite.h
# End Source File
# Begin Source File

SOURCE=.\src\menu\status.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\target.h
# End Source File
# Begin Source File

SOURCE=.\src\system\timer.h
# End Source File
# Begin Source File

SOURCE=.\src\racing\track.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\trader.h
# End Source File
# Begin Source File

SOURCE=.\src\system\trig.h
# End Source File
# Begin Source File

SOURCE=.\src\tutorial\tutorial.h
# End Source File
# Begin Source File

SOURCE=.\src\game\update.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\upgrade.h
# End Source File
# Begin Source File

SOURCE=.\src\system\video\video.h
# End Source File
# Begin Source File

SOURCE=.\src\ai\warship.h
# End Source File
# Begin Source File

SOURCE=.\src\sprite\weapon.h
# End Source File
# Begin Source File

SOURCE=.\src\osdep\win32\win32_misc.h
# End Source File
# Begin Source File

SOURCE=.\src\osdep\win32\win32_video.h
# End Source File
# Begin Source File

SOURCE=.\src\system\video\zoom.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=".\SDL-win32\lib\SDL.lib"
# End Source File
# Begin Source File

SOURCE=".\SDL-win32\lib\SDLmain.lib"
# End Source File
# Begin Source File

SOURCE=".\SDL_image-win32\lib\SDL_image.lib"
# End Source File
# Begin Source File

SOURCE="..\..\..\Program Files\Microsoft Visual Studio\Vc98\Lib\Wsock32.lib"
# End Source File
# End Target
# End Project
