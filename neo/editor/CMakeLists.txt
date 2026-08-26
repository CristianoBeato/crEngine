##############################
##          crEngine        ## 				
##  Egine editor tools      ##
##          systems         ##
##############################


## Sources
##################################################################

set( EDITOR_MAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editor.hpp
    )

set( EDITOR_SOURCES 
    ${EDITOR_MAIN_SOURCES}
    )

##
##################################################################

set( MODULES_LIBRARIES 
    framework
    swf
    tools
    sound
    renderer
#    d3xp
    idLib
    contrib    
    )

set( THIRD_PARTY_LIBRARIES
    ZLIB::ZLIB 
    SDL3::SDL3
    PNG::PNG 
    jpeg #JPEG::JPEG 
#    ogg
#    vorbis
#    vorbisfile
    )

##
##################################################################

add_executable( crEditor WIN32 ${EDITOR_SOURCES} )
add_dependencies( crEditor ${MODULES_LIBRARIES} )
target_link_libraries( crEditor 
        PRIVATE 
            -Wl,--start-group ${MODULES_LIBRARIES}  -Wl,--end-group 
        PUBLIC
            ${THIRD_PARTY_LIBRARIES}
        )
