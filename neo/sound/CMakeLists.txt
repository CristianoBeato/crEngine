##############################
##        Sound             ## 				
##  Engine sound systems    ##
##############################

## Sources
##################################################################

set( SOUND_MAIN_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/snd_emitter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snd_local.h
    ${CMAKE_CURRENT_SOURCE_DIR}/snd_shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snd_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/snd_world.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SoundVoice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoundVoice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WaveFile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WaveFile.h
    )

if( OPENAL ) ## OpenAL Windows/Unix Sound Backend
    set( SOUND_BACKEND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundHardware.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundHardware.h
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundSample.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundSample.h
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundVoice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/AL_SoundVoice.h
        )

elseif( XAUDIO ) ## Xaudio 2 Windows Sound Backend 

    set( SOUND_BACKEND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundHardware.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundHardware.h
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundSample.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundSample.h
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundVoice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/XAudio2/XA2_SoundVoice.h
        )
else( ) ## USE SDL Backend 
    set( SOUND_BACKEND_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundHardware.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundHardware.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundSample.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundSample.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundVoice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL/SDLSoundVoice.h
        )
endif()

set( SOUND_SOURCES
    ${SOUND_MAIN_SOURCES}
    ${SOUND_BACKEND_SOURCES}
    )

## library configurarion 
##################################################################

add_library( sound STATIC ${SOUND_SOURCES} )

#set the sound specific definitions
if( OPENAL )
    target_compile_definitions( sound PRIVATE USE_OPENAL )  
elseif( XAUDIO )
    target_compile_definitions( sound PRIVATE USE_XAUDIO )  
else()
    target_compile_definitions( sound PRIVATE USE_SDL3 )  
endif()

# d3xp need idLib
add_dependencies( sound idLib )

#set the idLib root dir as include
target_include_directories( sound PUBLIC ${CMAKE_SOURCE_DIR}/idlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ) 

# set the precompiled header
target_precompile_headers( sound 
	PUBLIC 
        ${CMAKE_SOURCE_DIR}/idlib/precompiled.h 
	PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/snd_local.h
        ${CMAKE_CURRENT_SOURCE_DIR}/sound.h
	) 
