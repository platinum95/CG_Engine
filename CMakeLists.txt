cmake_minimum_required( VERSION 2.8.11 )
project( CG_Engine )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O3")

find_package( glfw3 3.2 REQUIRED )
find_package( OpenGL REQUIRED )
find_package( assimp REQUIRED )
find_package( glm REQUIRED )

file(GLOB_RECURSE Eng_SOURCES "CG_Engine/*.cpp")

if (assimp_FOUND)
    set(ASSIMP_LIBRARY "assimp")
    add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
    set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.so")
endif(assimp_FOUND)

add_library( CG_Engine STATIC ${Eng_SOURCES} ) 
target_link_libraries( CG_Engine glfw assimp glm dl )
include_directories( "include" )
include_directories( "$(PROJECT_SOURCE_DIR)/CG_Engine" )
