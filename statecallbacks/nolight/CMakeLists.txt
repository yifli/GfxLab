include_directories(${CMAKE_SOURCE_DIR}/src)
set(TARGET_NAME NoLighting)

add_library(${TARGET_NAME} SHARED nolight_statecbs.cpp ../init.cpp statecbs.def)
target_link_libraries(${TARGET_NAME} glew32s opengl32)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_SOURCE_DIR}/bin)