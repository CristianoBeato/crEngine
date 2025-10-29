##############################
##         renderer         ## 				
## Engine renderer systems  ##
##############################

## Sources
##################################################################

set( RENDERER_MAIN_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/AutoRenderBink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AutoRenderBink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AutoRender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AutoRender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BoundsTrack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BoundsTrack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Cinematic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Cinematic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/Font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Font.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Framebuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLMatrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GLMatrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsAPIWrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Interaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Interaction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderLog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderLog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderSystem_init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderTexture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld
    ${CMAKE_CURRENT_SOURCE_DIR}/ResolutionScale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResolutionScale.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ScreenRect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ScreenRect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simplex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tr_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tr_trace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tr_trisurf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VertexCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/VertexCache.h
    )

set( RENDERER_BACKEND_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/RenderProgs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/RenderProgs_GLSL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/RenderProgs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/tr_backend_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/tr_backend_rendertools.cpp
    )

set( RENDERER_COLOR_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Color/ColorSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Color/ColorSpace.h
    )


set( RENDERER_DXT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTCodec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTCodec_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTDecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTEncoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DXT/DXTEncoder_SSE2.cpp
    )

set( RENDERER_FRONTEND_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_addlights.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_addmodels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_deform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_guisurf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/frontend/tr_frontend_subview.cpp
    )

set( RENDERER_IMAGES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/images/BinaryImage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/BinaryImageData.h
    ${CMAKE_CURRENT_SOURCE_DIR}/images/BinaryImage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image_files.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image_intrinsic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/ImageManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/ImageOpts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image_process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Image_program.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/jpeg_memory_src.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/images/jpeg_memory_src.h
    )

set( RENDERER_JOBS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/ShadowShared.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/ShadowShared.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/dynamicshadowvolume/DynamicShadowVolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/dynamicshadowvolume/DynamicShadowVolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/dynamicshadowvolume/DynamicShadowVolume_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/prelightshadowvolume/PreLightShadowVolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/prelightshadowvolume/PreLightShadowVolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/prelightshadowvolume/PreLightShadowVolume_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/staticshadowvolume/StaticShadowVolume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/staticshadowvolume/StaticShadowVolume.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/staticshadowvolume/StaticShadowVolume_local.h
    )

set( RENDERER_MODELS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/models/GuiModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/GuiModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_ase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_ase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_beam.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelDecal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelDecal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_liquid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_lwo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_lwo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_ma.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_ma.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_md3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_md3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_md5.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelOverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/ModelOverlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_prt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/models/Model_sprite.cpp
    )

set( RENDERER_OPENGL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_Framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_GraphicsAPIWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_Image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/OpenGL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/OpenGL.h
    )

set( RENDERER_RENDERWORLD_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderEntity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld_defs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld_load.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld/RenderWorld_portals.cpp
    )

set( RENDERER_SOURCES
    ${RENDERER_MAIN_SOURCES}
    ${RENDERER_BACKEND_SOURCES}
    ${RENDERER_COLOR_SOURCES}
    ${RENDERER_DXT_SOURCES}
    ${RENDERER_FRONTEND_SOURCES}
    ${RENDERER_IMAGES_SOURCES}
    ${RENDERER_JOBS_SOURCES}
    ${RENDERER_MODELS_SOURCES}
    ${RENDERER_OPENGL_SOURCES}
    ${RENDERER_RENDERWORLD_SOURCES}
    )

## library configurarion 
##################################################################

add_library( renderer STATIC ${RENDERER_SOURCES} )

#set the idLib specific definitions
#target_compile_definitions( renderer PRIVATE __DOOM_DLL__ )

# d3xp need idLib
add_dependencies( renderer idLib )

#set the idLib root dir as include
target_include_directories( renderer 
    PUBLIC ${CMAKE_SOURCE_DIR}/idlib 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/crglLib/lib/include ) 

# set the precompiled header
target_precompile_headers( renderer 
	PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
	PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/tr_local.h
	) 
