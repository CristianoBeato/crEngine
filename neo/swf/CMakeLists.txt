##############################
##          SWF             ## 				
## Flash interface rendering##
##############################

## Sources
##################################################################

set( SWF_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Bitstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Bitstream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Dictionary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Enums.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Names.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ParmList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ParmList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_PlaceObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptVar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ScriptVar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ShapeParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_ShapeParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Shapes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Sounds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_SpriteInstance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_SpriteInstance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Sprites.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Sprites.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_TextInstance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_TextInstance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SWF_Zlib.cpp
    )

## library configurarion 
##################################################################

add_library( swf STATIC ${SWF_SOURCES} )

# d3xp need idLib
add_dependencies( swf idLib )

#set the idLib root dir as include
target_include_directories( swf PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( swf PUBLIC ${CMAKE_SOURCE_DIR}/idlib/precompiled.h ) 
