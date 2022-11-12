cmake_minimum_required( VERSION 3.13 )
project( CG_Engine VERSION 0.0.1 )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( OpenGL_GL_PREFERENCE "GLVND" )

OPTION( BUNDLED_DEPS "Link against bundled libraries." OFF )

file( GLOB_RECURSE Eng_SOURCES "src/*.cpp" )
file( GLOB_RECURSE Eng_HEADERS "src/*.h" )
file( GLOB_RECURSE Eng_GLSL "src/*.glsl" )

list( APPEND Eng_SOURCES "src/glad.c" )

add_library( CG_Engine STATIC )

target_sources( CG_Engine
	PRIVATE
	${Eng_SOURCES}
)

if ( UNIX )
	set( ADDITIONAL_LIBS dl )
	set( PLATFORM_DIR "nix" )
elseif ( WIN32 )
	set( ADDITIONAL_LIBS zlibstaticd )
	set( PLATFORM_DIR "win32" )
endif()

if ( CMAKE_BUILD_TYPE EQUAL "DEBUG" )
	set( LIB_BUILD_DIR "debug" )
else()
	set( LIB_BUILD_DIR "release" )
endif()

function( WIN32_FIND_LIBRARY win32_find_library_var_libname )
	if ( CMAKE_BUILD_TYPE MATCHES "Debug|DEBUG" )
		set( win32_find_library_var_libfilename "${win32_find_library_var_libname}d.lib" )
	else()
		set( win32_find_library_var_libfilename "${win32_find_library_var_libfilename}.lib" )
	endif()

	find_library( "${win32_find_library_var_libname}" "${win32_find_library_var_libfilename}" REQUIRED )
endfunction()

if ( BUNDLED_DEPS )
	target_include_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/External/include" )
	list( APPEND CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/libs" )
	target_link_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/External/libs" )

	WIN32_FIND_LIBRARY( glfw )
	WIN32_FIND_LIBRARY( assimp )

else()
	find_package( glfw3 3.2 REQUIRED )
	find_package( OpenGL REQUIRED )
	find_package( assimp REQUIRED )
	find_package( glm REQUIRED )
	add_compile_definitions( _ITERATOR_DEBUG_LEVEL=0 )

	set ( glfw glfw )
	set ( assimp assimp )
endif()

target_include_directories( CG_Engine PRIVATE "include" )
target_include_directories( CG_Engine PRIVATE "src" )
target_include_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_link_libraries( CG_Engine PUBLIC ${glfw} ${assimp} ${ADDITIONAL_LIBS} )
