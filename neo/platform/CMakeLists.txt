##############################
##      platform            ## 				
##  platform specific stuff   ##
##############################

## Sources
##################################################################

if (WIN32)
    set( SYSTEM_PLATFORM_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_CPUInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_CPUInfo.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Network.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Path.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Platform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_Platform.hpp 
        )
elseif (LINUX)
    set( SYSTEM_PLATFORM_SOURCE 
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Console.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_CPUInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_CPUInfo.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Network.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_Path.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_platform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_platform.hpp
        )
endif ()

set( PLATFORM_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Events.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Input.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Network.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Paths.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Paths.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Video.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Video.hpp
    )

## library configurarion 
##################################################################
add_library( platform STATIC ${PLATFORM_SOURCE} ${SYSTEM_PLATFORM_SOURCE} )

#set the idLib root dir as include
target_include_directories( platform PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( platform
	PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
	PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform.hpp
	) 
