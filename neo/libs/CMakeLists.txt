#these are contrib software
set( CONTRIB_SOURCES
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

## library
##################################################################
add_library( contrib STATIC ${CONTRIB_SOURCES} )