cmake_minimum_required(VERSION 3.16)
project(PaperNX VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options($<$<C_COMPILER_ID:GNU,Clang>:-Wall> $<$<C_COMPILER_ID:GNU,Clang>:-Wextra>)

add_library(papernx_core STATIC
    src/core.cpp
    src/gestures.cpp
)
target_include_directories(papernx_core PUBLIC include)

include(CTest)
if(BUILD_TESTING)
    add_executable(papernx_core_tests tests/core_tests.cpp)
    if(MSVC)
        target_compile_options(papernx_core_tests PRIVATE /UNDEBUG)
    else()
        target_compile_options(papernx_core_tests PRIVATE -UNDEBUG)
    endif()
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
    find_package(Threads REQUIRED)
    if(PKG_CONFIG_FOUND AND NOT MUPDF_PREFIX)
        pkg_check_modules(MUPDF_DEPS REQUIRED mupdf)
        set(MUPDF_TRANSITIVE ${MUPDF_DEPS_STATIC_LIBRARIES})
        pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
        list(APPEND MUPDF_TRANSITIVE PkgConfig::HARFBUZZ)
    endif()

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
    target_link_libraries(PaperNX PRIVATE ${MUPDF_TRANSITIVE} Threads::Threads)

    if(UNIX AND NOT APPLE)
        target_link_libraries(PaperNX PRIVATE m pthread dl)
    endif()

    # Native MuPDF rendering integration tests
    add_executable(mupdf_render_tests
        tests/mupdf_render_tests.c
        src/mupdf_bridge.c
    )
    set_target_properties(mupdf_render_tests PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_options(mupdf_render_tests PRIVATE -UNDEBUG)
    target_include_directories(mupdf_render_tests PRIVATE ${MUPDF_INCLUDE_DIR} include)
    target_link_libraries(mupdf_render_tests PRIVATE ${MUPDF_LIBRARY})
    if(MUPDF_THIRD_LIBRARY)
        target_link_libraries(mupdf_render_tests PRIVATE ${MUPDF_THIRD_LIBRARY})
    endif()
    if(UNIX AND NOT APPLE)
        target_link_libraries(mupdf_render_tests PRIVATE m pthread dl)
    endif()
    target_link_libraries(mupdf_render_tests PRIVATE ${MUPDF_TRANSITIVE} Threads::Threads)
    add_test(NAME mupdf_render_tests COMMAND mupdf_render_tests)
    set_tests_properties(mupdf_render_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

    if(BUILD_TESTING)
        add_executable(papernx_app_tests tests/app_tests.cpp src/mupdf_bridge.c src/page_cache.cpp src/typography.cpp)
        target_include_directories(papernx_app_tests PRIVATE include ${MUPDF_INCLUDE_DIR})
        target_compile_options(papernx_app_tests PRIVATE -UNDEBUG)
        target_link_libraries(papernx_app_tests PRIVATE papernx_core PkgConfig::SDL2 PkgConfig::SDL2_TTF ${MUPDF_LIBRARY} ${MUPDF_THIRD_LIBRARY} ${MUPDF_TRANSITIVE} Threads::Threads m)
        add_test(NAME papernx_app_tests COMMAND papernx_app_tests)
        set_tests_properties(papernx_app_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" ENVIRONMENT "SDL_VIDEODRIVER=dummy")
    endif()
endif()

