cmake_minimum_required(VERSION 3.16)
project(PaperNX VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

add_library(papernx_core STATIC
    src/core.cpp
    src/gestures.cpp
)
target_include_directories(papernx_core PUBLIC include)

include(CTest)
if(BUILD_TESTING)
    add_executable(papernx_core_tests tests/core_tests.cpp)
    target_link_libraries(papernx_core_tests PRIVATE papernx_core)
    add_test(NAME papernx_core_tests COMMAND papernx_core_tests)
endif()

option(PAPERNX_NATIVE "Build SDL2/MuPDF native reader (dependencies required)" OFF)
if(PAPERNX_NATIVE)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
        pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
    else()
        find_package(SDL2 REQUIRED)
        find_package(SDL2_ttf REQUIRED)
    endif()

    if(MUPDF_PREFIX)
        set(MUPDF_INCLUDE_HINTS "${MUPDF_PREFIX}/include")
        set(MUPDF_LIB_HINTS "${MUPDF_PREFIX}/lib")
    endif()

    find_path(MUPDF_INCLUDE_DIR NAMES mupdf/fitz.h HINTS ${MUPDF_INCLUDE_HINTS} REQUIRED)
    find_library(MUPDF_LIBRARY NAMES mupdf HINTS ${MUPDF_LIB_HINTS} REQUIRED)
    find_library(MUPDF_THIRD_LIBRARY NAMES mupdf-third HINTS ${MUPDF_LIB_HINTS})

    add_executable(PaperNX
        src/main.cpp
        src/mupdf_bridge.c
        src/typography.cpp
        src/page_cache.cpp
    )

    target_include_directories(PaperNX PRIVATE ${MUPDF_INCLUDE_DIR} include)

    if(TARGET PkgConfig::SDL2 AND TARGET PkgConfig::SDL2_TTF)
        target_link_libraries(PaperNX PRIVATE papernx_core PkgConfig::SDL2 PkgConfig::SDL2_TTF ${MUPDF_LIBRARY})
    else()
        target_link_libraries(PaperNX PRIVATE papernx_core ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${MUPDF_LIBRARY})
    endif()

    if(MUPDF_THIRD_LIBRARY)
        target_link_libraries(PaperNX PRIVATE ${MUPDF_THIRD_LIBRARY})
    endif()

    if(UNIX AND NOT APPLE)
        target_link_libraries(PaperNX PRIVATE m pthread dl)
    endif()

    # Native MuPDF rendering integration tests
    add_executable(mupdf_render_tests
        tests/mupdf_render_tests.c
        src/mupdf_bridge.c
    )
    target_include_directories(mupdf_render_tests PRIVATE ${MUPDF_INCLUDE_DIR} include)
    target_link_libraries(mupdf_render_tests PRIVATE ${MUPDF_LIBRARY})
    if(MUPDF_THIRD_LIBRARY)
        target_link_libraries(mupdf_render_tests PRIVATE ${MUPDF_THIRD_LIBRARY})
    endif()
    if(UNIX AND NOT APPLE)
        target_link_libraries(mupdf_render_tests PRIVATE m pthread dl)
    endif()
    add_test(NAME mupdf_render_tests COMMAND mupdf_render_tests)
endif()

