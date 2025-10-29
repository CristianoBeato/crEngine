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
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_main.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_video.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_video.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_paths.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/achievements.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/achievements.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/localuser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/localuser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/savegame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/session_local.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/signin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/signin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/socket_net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/joystick.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/joystick.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/events.cpp
    )

# linux sources 
set( SYSTEM_LINUX_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_local.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_public.h
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/posix_signal.cpp
    )

set( SYSTEM_WIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_glimp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_input.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_nanoafx.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_gl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_shared.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_snd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_syscon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_taskkeyhook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/win32/win_wndproc.cpp
    )

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

##
## GCC require a specific modules linking order 
##
## d3xp ( if monolithic )
## renderer 
## sound 
## swf 
## framework 
## tools 
## idLib 
## contrib
set( MODULES_LIBRARIES 
    framework
    swf
    tools
    sound
    renderer
    d3xp
    idLib
    contrib    
    )

# to build the executable we need to build all modules
if( MONOLITH )
    add_dependencies( crEngine ${MODULES_LIBRARIES} )
    target_link_libraries( crEngine PRIVATE -Wl,--start-group ${MODULES_LIBRARIES} -Wl,--end-group )
else( )
    add_dependencies( crEngine ${MODULES_LIBRARIES} )
    target_link_libraries( crEngine PRIVATE -Wl,--start-group ${MODULES_LIBRARIES} -Wl,--end-group )
endif( )

# link external libraries
target_link_libraries( crEngine PUBLIC ZLIB::ZLIB PNG::PNG JPEG::JPEG OpenGL::OpenGL OpenAL::OpenAL SDL3 )

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
