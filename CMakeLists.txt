cmake_minimum_required(VERSION 3.20)
project(firmir-paint CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Firmir dependency: prefer the installed libraries via pkg-config
# (firmir-gui.pc); fall back to a sibling ../firmir checkout otherwise.
# FIRMIR_ROOT points at a firmir source checkout: builds against it
# directly (no installation needed). Empty -> try the installed firmir.
set(FIRMIR_ROOT "" CACHE PATH "firmir source tree to build against")
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND AND NOT FIRMIR_ROOT)
    pkg_check_modules(FIRMIR_PC QUIET IMPORTED_TARGET firmir-gui)
endif()
if(FIRMIR_ROOT)
    # Explicit source-tree build: drop any stale pkg-config results
    # cached by an earlier configuration of this build directory.
    set(FIRMIR_PC_FOUND FALSE)
endif()
if(FIRMIR_PC_FOUND)
    message(STATUS "firmir-paint: using firmir via pkg-config "
                   "(${FIRMIR_PC_VERSION})")
    # The UI is written in Firml: compile it with the installed firmlc.
    pkg_get_variable(FIRMIR_PREFIX firmir-gui prefix)
    include(${FIRMIR_PREFIX}/share/firmir/cmake/FirmirFirml.cmake)
    firmir_add_fml(paint_codegen SOURCES paint.fml
        INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
else()
    if(NOT FIRMIR_ROOT)
        get_filename_component(FIRMIR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../firmir"
                               ABSOLUTE)
    endif()
    if(NOT EXISTS "${FIRMIR_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR
            "firmir not found via pkg-config and no sibling checkout at "
            "${FIRMIR_ROOT} — either 'sudo make install' the firmir project "
            "or clone it next to this directory.")
    endif()
    message(STATUS "firmir-paint: using firmir from ${FIRMIR_ROOT}")
    set(FIRMIR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(FIRMIR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FIRMIR_BUILD_DOC_TOOL OFF CACHE BOOL "" FORCE)
    set(FIRMIR_BUILD_TOOL_FIRML ON CACHE BOOL "" FORCE)
    add_subdirectory(${FIRMIR_ROOT} ${CMAKE_BINARY_DIR}/firmir)
    include(${FIRMIR_ROOT}/tools/firml/cmake/FirmlCodegen.cmake)
    firml_add_target(paint_codegen SOURCES paint.fml
        INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                     ${FIRMIR_ROOT}/libs/gui/include)
    target_include_directories(paint_codegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Optional PNG export (Print / Ctrl+S). Without libpng the canvas still works
# and "Save" reports the missing support in the status bar.
find_package(PkgConfig QUIET)
set(FIRMIR_PAINT_HAVE_PNG OFF)
if(PkgConfig_FOUND)
    pkg_check_modules(PNG IMPORTED_TARGET libpng)
    if(PNG_FOUND)
        set(FIRMIR_PAINT_HAVE_PNG ON)
    endif()
endif()

add_executable(firmir-paint main.cpp PaintApp.cpp)
target_include_directories(firmir-paint PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(FIRMIR_PC_FOUND)
    target_link_libraries(firmir-paint PRIVATE paint_codegen PkgConfig::FIRMIR_PC)
else()
    target_link_libraries(firmir-paint PRIVATE paint_codegen firmir-gui)
endif()
if(FIRMIR_PAINT_HAVE_PNG)
    target_compile_definitions(firmir-paint PRIVATE FIRMIR_PAINT_HAVE_PNG=1)
    target_link_libraries(firmir-paint PRIVATE PkgConfig::PNG)
else()
    message(STATUS "firmir-paint: libpng not found — PNG saving disabled")
endif()
