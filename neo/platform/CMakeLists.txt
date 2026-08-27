##############################
##      platform            ## 				
##  platform specific stuff   ##
##############################

## Sources
##################################################################

if (WIN32)
    set( SYSTEM_PLATFORM_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/Windows_platform.cpp 
        )
elseif (UNIX)
    set( SYSTEM_PLATFORM_SOURCE 
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/Linux_platform.cpp
        )
endif ()

set( PLATFORM_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform.hpp
    )

## library configurarion 
##################################################################
add_library( platform STATIC ${PLATFORM_SOURCE} )

#set the idLib root dir as include
target_include_directories( platform PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( platform
	PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
	PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/Platform.hpp
	) 
