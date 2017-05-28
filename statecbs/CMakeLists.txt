include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(StateCbs SHARED statecbs.cpp statecbs.def)
target_link_libraries(StateCbs glew32s opengl32)