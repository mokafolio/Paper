cmake_minimum_required (VERSION 2.8.11)
project (Paper)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions -w")

option(BuildSubmodules "BuildSubmodules" OFF)
option(AddTests "AddTests" ON)

find_package(OpenGL REQUIRED)

if(BuildSubmodules)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Libs ${CMAKE_CURRENT_SOURCE_DIR}/Submodules ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Stick ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Crunch ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Brick ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Scrub)
else()
    include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Tarp ${CMAKE_CURRENT_SOURCE_DIR}/Paper/Libs)
endif()

link_directories(/usr/local/lib ${CMAKE_INSTALL_PREFIX}/lib)

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
Paper/Paint.hpp
Paper/Path.hpp
Paper/PlacedSymbol.hpp
Paper/RenderInterface.hpp
Paper/Segment.hpp
Paper/Symbol.hpp
Paper/Libs/GL/gl3w.h
#Paper/OpenGL/GLRenderer.hpp
Paper/Private/Allocator.hpp
Paper/Private/BooleanOperations.hpp
Paper/Private/ContainerView.hpp
Paper/Private/JoinAndCap.hpp
Paper/Private/PathFitter.hpp
Paper/Private/PathFlattener.hpp
Paper/Private/Shape.hpp
Paper/Private/StrokeTriangulator.hpp
Paper/SVG/SVGExport.hpp
Paper/SVG/SVGImport.hpp
Paper/SVG/SVGImportResult.hpp
Paper/Tarp/TarpRenderer.hpp
)

set (PAPERSRC 
Paper/Curve.cpp
Paper/CurveLocation.cpp
Paper/Document.cpp
Paper/Group.cpp
Paper/Item.cpp
Paper/Paint.cpp
Paper/Path.cpp
Paper/PlacedSymbol.cpp
Paper/RenderInterface.cpp
Paper/Segment.cpp
Paper/Symbol.cpp
Paper/Libs/GL/gl3w.c
#Paper/OpenGL/GLRenderer.cpp
Paper/Private/BooleanOperations.cpp
Paper/Private/JoinAndCap.cpp
Paper/Private/PathFitter.cpp
Paper/Private/PathFlattener.cpp
Paper/Private/Shape.cpp
Paper/Private/StrokeTriangulator.cpp
Paper/SVG/SVGExport.cpp
Paper/SVG/SVGImport.cpp
Paper/SVG/SVGImportResult.cpp
Paper/Tarp/TarpRenderer.cpp
)

if(BuildSubmodules)
    set(PrevAddTests ${AddTests})
    set(AddTests OFF)
    set(BuildSubmodules OFF) #we don't want to build submodules of submodules
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Stick)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Crunch)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Brick)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Scrub)
    set(AddTests ${PrevAddTests})
endif()

add_library (Paper SHARED ${PAPERSRC})
add_library (PaperStatic STATIC ${PAPERSRC})
target_link_libraries(Paper ${PAPERDEPS})
target_link_libraries(PaperStatic ${PAPERDEPS})
install (TARGETS Paper PaperStatic DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
foreach ( file ${PAPERINC} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${dir} )
endforeach()
if(AddTests)
    add_subdirectory (Tests)
endif()
add_subdirectory (Playground)
