cmake_minimum_required (VERSION 2.8.11)
project (Paper)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions")

find_package(OpenGL REQUIRED)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include)
link_directories(/usr/local/lib)

set (PAPERDEPS Stick Brick Scrub ${OPENGL_LIBRARIES})

set (PAPERINC 
Paper/BasicTypes.hpp
Paper/Components.hpp
Paper/Constants.hpp
Paper/Curve.hpp
Paper/CurveLocation.hpp
Paper/Document.hpp
Paper/Group.hpp
Paper/Item.hpp
Paper/Path.hpp
Paper/RenderInterface.hpp
Paper/Segment.hpp
Paper/OpenGL/GLRenderer.hpp
Paper/SVG/SVGExport.hpp
)

set (PAPERSRC 
Paper/Curve.cpp
Paper/CurveLocation.cpp
Paper/Document.cpp
Paper/Group.cpp
Paper/Item.cpp
Paper/Path.cpp
Paper/RenderInterface.cpp
Paper/Segment.cpp
Paper/OpenGL/GLRenderer.cpp
Paper/Private/BooleanOperations.cpp
Paper/Private/JoinAndCap.cpp
Paper/Private/PathFlattener.cpp
Paper/Private/Shape.cpp
Paper/Private/StrokeTriangulator.cpp
Paper/SVG/SVGExport.cpp
)

add_library (Paper SHARED ${PAPERSRC})
add_library (PaperStatic STATIC ${PAPERSRC})
target_link_libraries(Paper ${PAPERDEPS})
target_link_libraries(PaperStatic ${PAPERDEPS})
install (TARGETS Paper PaperStatic DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
foreach ( file ${PAPERINC} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${dir} )
endforeach()
add_subdirectory (Tests)
add_subdirectory (Examples)
