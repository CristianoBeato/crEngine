##############################
##        framework         ## 				
##  Engine common systems   ##
##############################

## Sources
##################################################################

set( FRAMEWORK_MAIN_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/BuildVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CmdSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CmdSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_demos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_dialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common_frame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_localize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common_printf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Compressor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Compressor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Console.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ConsoleHistory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConsoleHistory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CVarSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CVarSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugGraph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclAF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclAF.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclEntityDef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclEntityDef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclFX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclFX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclParticle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclParticle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclPDA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclPDA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclSkin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclSkin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DeclTable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DemoChecksum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DemoFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DemoFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EditField.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EditField.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EventLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EventLoop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.h
    ${CMAKE_CURRENT_SOURCE_DIR}/File_Manifest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File_Manifest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/File_Resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File_Resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/File_SaveGame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File_SaveGame.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/KeyInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KeyInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Licensee.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerProfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlayerProfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TokenParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TokenParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Unzip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UsercmdGen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UsercmdGen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Zip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Zip.h
    )

set( FRAMEWORK_COLISION_MODEL_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_contacts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_contents.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_files.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_rotate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_trace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cm/CollisionModel_translate.cpp
    )

set( FRAMEWORK_UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/BindWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/BindWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ChoiceWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ChoiceWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/DeviceContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/DeviceContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/EditWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/EditWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/FieldWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/FieldWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameBearShootWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameBearShootWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameBustOutWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameBustOutWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameSSDWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GameSSDWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GuiScript.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/GuiScript.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ListGUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ListGUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ListGUILocal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ListWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ListWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/MarkerWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/MarkerWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Rectangle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/RegExp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/RegExp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/RegExp_old.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/RenderWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/RenderWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/SimpleWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/SimpleWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/SliderWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/SliderWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/UserInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/UserInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/UserInterfaceLocal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Winvar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Winvar.h
    )

set( FRAMEWORK_SOURCE 
    ${FRAMEWORK_MAIN_SOURCES}
    ${FRAMEWORK_COLISION_MODEL_SOURCES}
    ${FRAMEWORK_UI_SOURCES}  
    )

## library configurarion 
##################################################################

add_library( framework STATIC ${FRAMEWORK_SOURCE} )

#set the idLib specific definitions
#target_compile_definitions( framework PRIVATE __DOOM_DLL__ ) TODO: fix on non monolith build 

# d3xp need idLib
add_dependencies( framework idLib sound renderer )

#set the idLib root dir as include
target_include_directories( framework PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( framework 
	PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
	PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/Common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Common_local.h
	) 
