##############################
## 			idLib			## 				
## Common engine libraries  ##
##############################

## Sources
##################################################################3

set( IDLIB_MAIN_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/Base64.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Base64.h
	${CMAKE_CURRENT_SOURCE_DIR}/BitMsg.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BitMsg.h
	${CMAKE_CURRENT_SOURCE_DIR}/Callback.h
	${CMAKE_CURRENT_SOURCE_DIR}/CmdArgs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CmdArgs.h
	${CMAKE_CURRENT_SOURCE_DIR}/CommandLink.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/containers
	${CMAKE_CURRENT_SOURCE_DIR}/DataQueue.h
	${CMAKE_CURRENT_SOURCE_DIR}/Dict.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Dict.h
	${CMAKE_CURRENT_SOURCE_DIR}/geometry
	${CMAKE_CURRENT_SOURCE_DIR}/Heap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Heap.h
	${CMAKE_CURRENT_SOURCE_DIR}/LangDict.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LangDict.h
	${CMAKE_CURRENT_SOURCE_DIR}/Lexer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lexer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Lib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Lib.h
	${CMAKE_CURRENT_SOURCE_DIR}/MapFile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MapFile.h
	${CMAKE_CURRENT_SOURCE_DIR}/ParallelJobList.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ParallelJobList.h
	${CMAKE_CURRENT_SOURCE_DIR}/ParallelJobList_JobHeaders.h
	${CMAKE_CURRENT_SOURCE_DIR}/Parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Parser.h
	${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h
	${CMAKE_CURRENT_SOURCE_DIR}/RectAllocator.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.h
	${CMAKE_CURRENT_SOURCE_DIR}/Str.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Str.h
	${CMAKE_CURRENT_SOURCE_DIR}/StrStatic.h
	${CMAKE_CURRENT_SOURCE_DIR}/Swap.h
	${CMAKE_CURRENT_SOURCE_DIR}/Thread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Thread.h
	${CMAKE_CURRENT_SOURCE_DIR}/Timer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
	${CMAKE_CURRENT_SOURCE_DIR}/Token.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Token.h
	)

./bv:
Bounds.cpp
Bounds.h
Box.cpp
Box.h
Frustum.cpp
Frustum.h
Sphere.cpp
Sphere.h

./containers:
Array.h
BinSearch.h
BTree.h
HashIndex.cpp
HashIndex.h
HashTable.h
Hierarchy.h
LinkList.h
List.h
PlaneSet.h
Queue.h
Sort.h
Stack.h
StaticList.h
StrList.h
StrPool.h
VectorSet.h

./geometry:
DrawVert.cpp
DrawVert.h
DrawVert_intrinsics.h
JointTransform.cpp
JointTransform.h
RenderMatrix.cpp
RenderMatrix.h
Surface.cpp
Surface.h
Surface_Patch.cpp
Surface_Patch.h
Surface_Polytope.cpp
Surface_Polytope.h
Surface_SweptSpline.cpp
Surface_SweptSpline.h
TraceModel.cpp
TraceModel.h
Winding2D.cpp
Winding2D.h
Winding.cpp
Winding.h

./hashing:
CRC32.cpp
CRC32.h
MD4.cpp
MD4.h
MD5.cpp
MD5.h

./math:
Angles.cpp
Angles.h
Complex.cpp
Complex.h
Curve.h
Extrapolate.h
Interpolate.h
Lcp.cpp
Lcp.h
Math.cpp
Math.h
Matrix.cpp
Matrix.h
MatX.cpp
MatX.h
Ode.cpp
Ode.h
Plane.cpp
Plane.h
Pluecker.cpp
Pluecker.h
Polynomial.cpp
Polynomial.h
Quat.cpp
Quat.h
Random.h
Rotation.cpp
Rotation.h
Simd.cpp
Simd_Generic.cpp
Simd_Generic.h
Simd.h
Simd_SSE.cpp
Simd_SSE.h
Vector.cpp
Vector.h
VectorI.h
VecX.cpp
VecX.h

./sys:
posix
sys_alloc_tags.h
sys_assert.cpp
sys_assert.h
sys_builddefines.h
sys_defines.h
sys_filesystem.h
sys_includes.h
sys_intrinsics.h
sys_threading.h
sys_types.h
win32

./sys/posix:
posix_thread.cpp

./sys/win32:
win_thread.cpp


##################################################################3



file(GLOB_RECURSE ID_INCLUDES *.h)
file(GLOB_RECURSE ID_SOURCES *.cpp)

if(MSVC)
	list(REMOVE_ITEM ID_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/posix/posix_thread.cpp)	
else()
	list(REMOVE_ITEM ID_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sys/win32/win_thread.cpp)
endif()

#if(STANDALONE)
#	add_definitions(-DSTANDALONE)
#endif()

set(ID_PRECOMPILED_SOURCES ${ID_SOURCES})
list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/geometry/RenderMatrix.cpp)
list(REMOVE_ITEM ID_PRECOMPILED_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/SoftwareCache.cpp)

add_definitions(-D__IDLIB__ -D__DOOM_DLL__)

if(MSVC)
	
	#set_target_properties(idlib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
	
	foreach( src_file ${ID_PRECOMPILED_SOURCES} )
		#message(STATUS "/Yuprecompiled.h for ${src_file}")
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yuprecompiled.h"
            )
    endforeach()
	
	set_source_files_properties(precompiled.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycprecompiled.h"
        )
        
    add_library(idlib ${ID_SOURCES} ${ID_INCLUDES})
else()
	foreach( src_file ${ID_PRECOMPILED_SOURCES} )
		#message(STATUS "-include precompiled.h for ${src_file}")
		set_source_files_properties(
			${src_file}
			PROPERTIES
			COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h"
			)
	endforeach()
	
	include_directories(.)
	
	# precompiled magic for GCC/clang, adapted from https://gist.github.com/573926
	# Updated to support Cmake 3.3 or newer.
	STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
	SET(_compiler_FLAGS ${${_flags_var_name}})
	
	if(NOT CMAKE_VERSION VERSION_LESS "3.3")
		# This section is for Cmake 3.3 or newer.
		get_property(_directory_flags DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS " -I${item}")
		ENDFOREACH(item)
		get_property(_directory_flags DIRECTORY PROPERTY COMPILE_DEFINITIONS)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS " -D${item}")
		ENDFOREACH(item)
	else()
		GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
		FOREACH(item ${_directory_flags})
			LIST(APPEND _compiler_FLAGS " -I${item}")
		ENDFOREACH(item)
		GET_DIRECTORY_PROPERTY(_directory_flags DEFINITIONS)
		LIST(APPEND _compiler_FLAGS ${_directory_flags})
	endif()
	
	SEPARATE_ARGUMENTS(_compiler_FLAGS)
	
	add_custom_target(precomp_header_idlib ALL
	                  COMMAND ${CMAKE_CXX_COMPILER} ${_compiler_FLAGS} -x c++-header precompiled.h -o precompiled.h.gch
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	                  COMMENT "Creating idlib/precompiled.h.gch for idlib"
	                  )
	
	add_library(idlib ${ID_SOURCES} ${ID_INCLUDES})
	add_dependencies(idlib precomp_header_idlib)
	
endif()
	

# if(MSVC)
	# # set_source_files_properties(precompiled.cpp
        # # PROPERTIES
        # # COMPILE_FLAGS "/Ycprecompiled.h"
        # # )
		
	# #set_target_properties(idlib PROPERTIES COMPILE_FLAGS "/Yuprecompiled.h")
# endif()
