cmake_minimum_required(VERSION 3.14)
project(SDL_minizip VERSION 1.0.0 LANGUAGES C)

# Try to find SDL3 locally, if not, fetch it
find_package(SDL3 QUIET)
if (NOT SDL3_FOUND)
    message(STATUS "SDL3 not found locally, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.0
    )
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
endif()

# Fetch minizip-ng
include(FetchContent)
FetchContent_Declare(
    minizip_ng
    GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
    GIT_TAG        3.0.4
)

set(MZ_COMPAT OFF CACHE BOOL "" FORCE)
set(MZ_ZLIB ON CACHE BOOL "" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
set(MZ_LZMA OFF CACHE BOOL "" FORCE)
set(MZ_PPMD OFF CACHE BOOL "" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)
set(MZ_PKCRYPT OFF CACHE BOOL "" FORCE)
set(MZ_WZAES OFF CACHE BOOL "" FORCE)
set(MZ_OPENSSL OFF CACHE BOOL "" FORCE)
set(MZ_LIBBSD OFF CACHE BOOL "" FORCE)
set(MZ_ICONV OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(MZ_FETCH_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(minizip_ng)

# SDL_minizip
add_library(SDL_minizip INTERFACE)
target_include_directories(SDL_minizip INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SDL_minizip INTERFACE MINIZIP::minizip SDL3::SDL3)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_executable(SDL_minizip_test test/SDL_minizip_test.c)
    target_compile_options(SDL_minizip_test PRIVATE -fsanitize=address -g)
    target_link_options(SDL_minizip_test PRIVATE -fsanitize=address)
    target_link_libraries(SDL_minizip_test PRIVATE MINIZIP::minizip SDL3::SDL3 SDL3::SDL3_test)
    target_include_directories(SDL_minizip_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${minizip_ng_SOURCE_DIR} ${minizip_ng_BINARY_DIR})
    add_test(NAME MinizipIOTest COMMAND SDL_minizip_test)

    add_custom_command(TARGET SDL_minizip_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/test/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
endif()
