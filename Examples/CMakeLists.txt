find_package(PkgConfig REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

add_executable (PaperExample PaperExample.cpp)
target_link_libraries(PaperExample ${GLFW_STATIC_LIBRARIES} ${PAPERDEPS} Paper)
