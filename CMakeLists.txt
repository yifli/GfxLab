cmake_minimum_required(VERSION 2.8)

project(GfxLab)

include_directories(${CMAKE_SOURCE_DIR}/external/include)
link_directories(${CMAKE_SOURCE_DIR}/external/lib)

add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE SRCS "./src/*.cpp")
file(GLOB_RECURSE HEADERS "./src/*.h")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OpenMeshLib OpenMeshCored)
else()
    set(OpenMeshLib OpenMeshCore)
endif()

set(TARGET_NAME gfxlab)
add_executable(${TARGET_NAME} ${SRCS} ${HEADERS})
target_link_libraries(${TARGET_NAME} glfw3 SOIL ${OpenMeshLib} glew32s)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_SOURCE_DIR}/bin)

if (WIN32)
    target_link_libraries(gfxlab opengl32)
endif()

if (MSVC)
    source_group("Headers" FILES ${HEADERS})
endif()

add_subdirectory(statecallbacks/nolight)
add_subdirectory(statecallbacks/lighting)
add_subdirectory(statecallbacks/postprocessing)