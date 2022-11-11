# CG_Engine-config.cmake - package configuration file

get_filename_component( SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH )
include( ${SELF_DIR}/${CMAKE_BUILD_TYPE}/CG_Engine.cmake )
