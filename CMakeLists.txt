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

if ( BUNDLED_DEPS )
	target_include_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/External/include" )
	list( APPEND CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/libs" )
	target_link_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/External/libs" )
	find_library( glfw glfw.lib REQUIRED )
else()
	find_package( glfw3 3.2 REQUIRED )
	find_package( OpenGL REQUIRED )
	find_package( assimp REQUIRED )
	find_package( glm REQUIRED )
endif()

target_include_directories( CG_Engine PRIVATE "include" )
target_include_directories( CG_Engine PRIVATE "src" )
target_include_directories( CG_Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_link_libraries( CG_Engine PUBLIC glfw assimp zlibstaticd )

