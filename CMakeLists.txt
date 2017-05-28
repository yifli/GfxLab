cmake_minimum_required(VERSION 2.8)

project(GfxLab)

include_directories(${CMAKE_SOURCE_DIR}/external/include)
link_directories(${CMAKE_SOURCE_DIR}/external/lib)

add_definitions(-D_USE_MATH_DEFINES)

file(GLOB_RECURSE SRCS "./src/*.cpp")
file(GLOB_RECURSE HEADERS "./src/*.h")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(OpenMeshLib OpenMeshCored)
else()
	set(OpenMeshLib OpenMeshCore)
endif()

add_executable(gfxlab ${SRCS} ${HEADERS})
target_link_libraries(gfxlab glfw3 SOIL ${OpenMeshLib} glew32s)

if (WIN32)
	target_link_libraries(gfxlab opengl32)
endif()

if (MSVC)
	source_group("Headers" FILES ${HEADERS})
endif()

add_subdirectory(statecbs)