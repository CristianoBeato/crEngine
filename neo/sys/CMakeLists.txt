##############################
##          crEngine        ## 				
## Platform specifis engine ##
##          systems         ##
##############################

## Sources
##################################################################

set( SYSTEM_MAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/LightweightCompression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LightweightCompression.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PacketProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PacketProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Snapshot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Snapshot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Snapshot_Jobs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Snapshot_Jobs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SnapshotProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SnapshotProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_achievements.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_achievements.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_dedicated_server_search.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_dedicated_server_search.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_leaderboards.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_backend_direct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_backend_direct.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_migrate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_snapshot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_lobby_users.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_local.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_localuser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_localuser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_profile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_profile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_public.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_savegame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_savegame.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session_callbacks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session_local.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session_savegames.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_session_savegames.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_signin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_signin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_stats.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_stats_misc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_voicechat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_voicechat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/achievements.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/achievements.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/localuser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/localuser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/savegame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/session_local.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/signin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/signin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/socket_net.cpp
    )

# linux sources 
set( SYSTEM_LINUX_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/linux/linux_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_public.h
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_signal.cpp
    )

#
set( SYSTEM_SDL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl/sdl_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl/sdl_events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl/sdl_glimp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl/sdl_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl/sdl_qgl.cpp
    )

set( SYSTEM_WIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_glimp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_input.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_nanoafx.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_qgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_shared.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_snd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_syscon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_taskkeyhook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_wndproc.cpp
    )

#win32/rc/AFEditor.rc
#win32/rc/AFEditor_resource.h
#win32/rc/Common.rc
#win32/rc/Common_resource.h
#win32/rc/CreateResourceIDs.cpp
#win32/rc/CreateResourceIDs.h
#win32/rc/Debugger.rc
#win32/rc/Debugger_resource.h
#win32/rc/DeclEditor.rc
#win32/rc/DeclEditor_resource.h
#win32/rc/doom.aps
#win32/rc/doom.rc
#win32/rc/doom_resource.h
#win32/rc/GuiEd.rc
#win32/rc/GuiEd_resource.h
#win32/rc/MaterialEditor.rc
#win32/rc/MaterialEditor_Resource.h
#win32/rc/ParticleEditor.rc
#win32/rc/ParticleEditor_resource.h
#win32/rc/PDAEditor.rc
#win32/rc/PDAEditor_resource.h
#win32/rc/PropTree.rc
#win32/rc/PropTree_resource.h
#win32/rc/Radiant.rc
#win32/rc/Radiant_resource.h
#win32/rc/ScriptEditor.rc
#win32/rc/ScriptEditor_resource.h
#win32/rc/SoundEditor.rc
#win32/rc/SoundEditor_resource.h
#win32/rc/res/BEVEL.BMP
#win32/rc/res/BITMAP2.BMP
#win32/rc/res/BMP00001.BMP
#win32/rc/res/bmp00002.bmp
#win32/rc/res/bmp00003.bmp
#win32/rc/res/bmp00004.bmp
#win32/rc/res/bmp00005.bmp
#win32/rc/res/BMP0002.BMP
#win32/rc/res/cchsb.bmp
#win32/rc/res/ccrgb.bmp
#win32/rc/res/dbg_back.bmp
#win32/rc/res/dbg_breakpoint.ico
#win32/rc/res/dbg_current.ico
#win32/rc/res/dbg_currentline.ico
#win32/rc/res/dbg_empty.ico
#win32/rc/res/dbg_open.bmp
#win32/rc/res/dbg_toolbar.bmp
#win32/rc/res/DEFTEX.WAL
#win32/rc/res/doom.ico
#win32/rc/res/ENDCAP.BMP
#win32/rc/res/fpoint.cur
#win32/rc/res/fxeditor.ico
#win32/rc/res/fxed_link.ico
#win32/rc/res/fxed_toolbar.bmp
#win32/rc/res/GetString.htm
#win32/rc/res/guied_collapse.ico
#win32/rc/res/guied_expand.ico
#win32/rc/res/guied_hand.cur
#win32/rc/res/guied.ico
#win32/rc/res/guied_nav_visibledisabled.ico
#win32/rc/res/guied_nav_visible.ico
#win32/rc/res/guied_scripts.ico
#win32/rc/res/guied_scripts_white.ico
#win32/rc/res/guied_viewer_toolbar.bmp
#win32/rc/res/IBEVEL.BMP
#win32/rc/res/icon2.ico
#win32/rc/res/IENDCAP.BMP
#win32/rc/res/logo_sm3dfx.bmp
#win32/rc/res/matedtree.bmp
#win32/rc/res/MaterialEditor.ico
#win32/rc/res/me_disabled_icon.ico
#win32/rc/res/me_enabled.ico
#win32/rc/res/MEFileToolbar.bmp
#win32/rc/res/me_off_icon.ico
#win32/rc/res/me_on_icon.ico
#win32/rc/res/MEtoolbar.bmp
#win32/rc/res/PropTree.rc2
#win32/rc/res/Q.BMP
#win32/rc/res/qe3.ico
#win32/rc/res/RADIANT3.GIF
#win32/rc/res/RadiantDoc.ico
#win32/rc/res/Radiant.ico
#win32/rc/res/shaderbar.bmp
#win32/rc/res/shaderdoc.ico
#win32/rc/res/shaderframe.ico
#win32/rc/res/spliter.cur
#win32/rc/res/stormengine2.ico
#win32/rc/res/TOOLBAR1.BMP
#win32/rc/res/TOOLBAR2.BMP
#win32/rc/res/Toolbar.bmp
#win32/rc/res/VIEWDEFA.BMP
#win32/rc/res/VIEWOPPO.BMP
if( WIN32 )
    set ( SYSTEM_PLATFORM_SOURCES ${SYSTEM_WIN_SOURCES} )
elseif( "${CMAKE_SYSTEM}" MATCHES "Linux")
    set ( SYSTEM_PLATFORM_SOURCES ${SYSTEM_LINUX_SOURCES} ${SYSTEM_SDL_SOURCES} )
endif( )

set( SYSTEM_SOURCES  
    ${SYSTEM_MAIN_SOURCES}
    ${SYSTEM_PLATFORM_SOURCES}    
    )

## Link Libraries
##################################################################


if( WIN332 )
    set( SYSTEM_LIBRARIES  )
else( )

endif()

## Executable 
##################################################################

add_executable( crEngine WIN32 ${SYSTEM_SOURCES} )

# to build the executable we need to build all modules
if( MONOLITH )
    add_dependencies( crEngine idLib d3xp tools swf framework renderer sound   )
    target_link_libraries( crEngine PRIVATE d3xp tools renderer sound framework swf idLib  )
else( )
    add_dependencies( crEngine idLib swf framework renderer sound  )
    target_link_libraries( crEngine PRIVATE tools renderer sound framework swf d3xp idLib )
endif( )

# link external libraries
target_link_libraries( crEngine PUBLIC ZLIB::ZLIB SDL2::SDL2 PNG::PNG JPEG::JPEG OpenGL::OpenGL OpenAL::OpenAL )

#set the idLib root dir as include
target_include_directories( crEngine PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( crEngine 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
    PRIVATE
        sys_public.h
        sys_local.h
        ) 
