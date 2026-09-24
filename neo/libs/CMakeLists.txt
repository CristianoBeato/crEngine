## External Libraries 

#these are contrib software
set( MINIZIP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/zip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/zip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/unzip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/unzip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/mztools.h
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/mztools.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/minizip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/minizip.1
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/miniunzip.1
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/miniunz.c
    #${CMAKE_CURRENT_SOURCE_DIR}/minizip/iowin32.h
    #${CMAKE_CURRENT_SOURCE_DIR}/minizip/iowin32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/ioapi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/ioapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/minizip/crypt.h
    )

#GWen
set( GWEN_SOURCES 

    ## Gwen Base
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Align.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Anim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Anim.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/BaseRender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/BaseRender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/ControlList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/ControlList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/DragAndDrop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/DragAndDrop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/events.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Events.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Exports.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/FrameBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Gwen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Gwen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Hook.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Hook.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/inputhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/InputHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Precompiled.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Skin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Skin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Structures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Structures.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/TextObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/ToolTip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/ToolTip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/UserData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Utility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Utility.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/WindowProvider.h

    ## Gwen Controls
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Button.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Canvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CheckBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CheckBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CollapsibleCategory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CollapsibleCategory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CollapsibleList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ColorControls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ColorControls.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ColorPicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ColorPicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ComboBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ComboBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CrossSplitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/CrossSplitter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DockBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DockBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DockedTabControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DockedTabControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dragger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dragger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DrawPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/DrawPanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/FieldLabel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/FilePicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/FolderPicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/GroupBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/GroupBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Highlight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HorizontalScrollBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HorizontalScrollBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HorizontalSlider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HorizontalSlider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HSVColorPicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/HSVColorPicker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ImagePanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ImagePanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Label.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Label.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/LabelClickable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/LabelClickable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ListBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ListBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Menu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Menu.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/MenuItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/MenuItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/MenuStrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/MenuStrip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Modal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/NumericUpDown.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/NumericUpDown.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/PageControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/PageControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ProgressBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ProgressBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Properties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/PropertyTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/PropertyTree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RadioButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RadioButton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RadioButtonController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RadioButtonController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Rectangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Rectangle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ResizableControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ResizableControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Resizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Resizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RichLabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/RichLabel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBarBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBarBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBarButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollBarButton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ScrollControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Slider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/SplitterBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/SplitterBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Splitters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/StatusBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabButton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabButton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabStrip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabStrip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TabTitleBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TextBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TextBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TextBoxNumeric.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/ToolBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TreeControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TreeControl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TreeNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/TreeNode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/VerticalScrollBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/VerticalScrollBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/VerticalSlider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/VerticalSlider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/WindowButtons.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/WindowCanvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/WindowCanvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/WindowControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/WindowControl.h
    
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FileOpen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FileOpen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FileSave.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FileSave.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FolderOpen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/FolderOpen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/Query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Dialogs/Query.h

    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Layout/Position.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Layout/Table.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Layout/Tile.h
    
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/BaseProperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/Checkbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/ColorSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/ComboBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/File.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/Folder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Controls/Property/Text.h

    ## Use the engine hook
    ## ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Input/SDL3.h
    ## ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Platforms/SDL3.cpp

    ##${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Renderers/FontData.h
    ##${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Renderers/OpenGL.cpp
    ##${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Renderers/OpenGL.h
    
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Skins/Simple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Skins/TexturedBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gwen/Gwen/Skins/Texturing.h
    )

## JPeg6 lib 
## In future i want to remove the dependencies
## instead of roc video use the open format Ogg Theora 

#
set( CONTRIB_SOURCES 
    ${MINIZIP_SOURCES}
    ${GWEN_SOURCES}
    )

## library
##################################################################
add_library( contrib STATIC ${CONTRIB_SOURCES} )