cmake_minimum_required( VERSION 2.8.11 )
project( CG_Engine )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( OpenGL_GL_PREFERENCE "GLVND" )

find_package( glfw3 3.2 REQUIRED )
find_package( OpenGL REQUIRED )
find_package( assimp REQUIRED )
find_package( glm REQUIRED )

file( GLOB_RECURSE Eng_SOURCES "CG_Engine/*.cpp" )
file( GLOB_RECURSE Eng_HEADERS "CG_Engine/*.h" )
file( GLOB_RECURSE Eng_GLSL "CG_Engine/*.glsl" )
#if ( assimp_FOUND )
#    set( ASSIMP_LIBRARY "assimp" )
#    add_library( ${ASSIMP_LIBRARY} SHARED IMPORTED )
#    set_target_properties( ${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.so" )
#endif( assimp_FOUND )

add_library( CG_Engine STATIC ${Eng_SOURCES} CG_Engine/glad.c ) 
target_link_libraries( CG_Engine glfw assimp dl stdc++fs )
include_directories( "include" )
include_directories( "$(PROJECT_SOURCE_DIR)/CG_Engine" )

install( TARGETS CG_Engine EXPORT CG_Engine DESTINATION lib )
install( FILES ${Eng_HEADERS} ${Eng_GLSL} DESTINATION include/CG_Engine )
install( FILES CG_EngineConfig.cmake DESTINATION lib/cmake/CG_Engine/ )
install( EXPORT CG_Engine DESTINATION lib/cmake/CG_Engine )

