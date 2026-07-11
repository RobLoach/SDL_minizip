cmake_minimum_required(VERSION 3.14)
project(SDL_minizip VERSION 1.0.0 LANGUAGES C)

# Add the cmake package list.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.cmake")

# SDL3
find_package(SDL3 REQUIRED)

# minizip-ng
find_package(minizipng REQUIRED)

# SDL_minizip
include(GNUInstallDirs)
add_library(SDL_minizip INTERFACE)
add_library(SDL_minizip::SDL_minizip ALIAS SDL_minizip)
target_include_directories(SDL_minizip INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(SDL_minizip INTERFACE MINIZIP::minizip SDL3::SDL3)

# Install
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CMakePackageConfigHelpers)

    install(FILES SDL_minizip.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/.cmake/SDL_minizipConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/SDL_minizipConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SDL_minizip
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/SDL_minizipConfigVersion.cmake
        COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/SDL_minizipConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/SDL_minizipConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SDL_minizip)
endif()

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_executable(SDL_minizip_test test/SDL_minizip_test.c)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(SDL_minizip_test PRIVATE -fsanitize=address -g)
        target_link_options(SDL_minizip_test PRIVATE -fsanitize=address)
    endif()
    target_link_libraries(SDL_minizip_test PRIVATE SDL_minizip SDL3::SDL3_test)
    add_test(NAME MinizipIOTest COMMAND SDL_minizip_test)

    add_custom_command(TARGET SDL_minizip_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/test/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
endif()
