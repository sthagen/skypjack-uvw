#
# uvw
#

cmake_minimum_required(VERSION 3.13)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#
set(UVW_VERSION_MAJOR 3)
set(UVW_VERSION_MINOR 5)
set(UVW_VERSION_PATCH 0)

project(
    uvw
    VERSION ${UVW_VERSION_MAJOR}.${UVW_VERSION_MINOR}.${UVW_VERSION_PATCH}
    DESCRIPTION "Header-only, event based, tiny and easy to use libuv wrapper in modern C++ - now available also as static library!"
    HOMEPAGE_URL "https://github.com/skypjack/uvw"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(UVW_USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if available." ON)
option(UVW_USE_ASAN "Use address sanitizer by adding -fsanitize=address -fno-omit-frame-pointer flags" OFF)
option(UVW_USE_UBSAN "Use address sanitizer by adding -fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer flags" OFF)
option(UVW_USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(UVW_BUILD_LIBS "Prepare targets for static library rather than for a header-only library." OFF)
option(UVW_BUILD_SHARED_LIB "Prepare targets for shared library rather than for a header-only library." OFF)
option(UVW_FIND_LIBUV "Try finding libuv library development files in the system" OFF)

if(UVW_BUILD_SHARED_LIB)
    set(UVW_BUILD_LIBS BOOL:ON)
endif()

#
# Compiler stuff
#

if(NOT WIN32 AND UVW_USE_LIBCPP)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)

    cmake_push_check_state()

    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

    check_cxx_source_compiles("
        #include<type_traits>
        int main() { return std::is_same_v<int, char>; }
    " UVW_HAS_LIBCPP)

    if(NOT UVW_HAS_LIBCPP)
        message(WARNING "The option UVW_USE_LIBCPP is set (by default) but libc++ is not available. The flag will not be added to the target.")
    endif()

    cmake_pop_check_state()
endif()

if(UVW_USE_CLANG_TIDY)
    find_program(UVW_CLANG_TIDY_EXECUTABLE "clang-tidy")

    if(NOT UVW_CLANG_TIDY_EXECUTABLE)
        message(VERBOSE "The option UVW_USE_CLANG_TIDY is set but clang-tidy executable is not available.")
    endif()
endif()

# Required minimal libuv version
set(UVW_LIBUV_VERSION 1.50.0)

function(fetch_libuv)
    if (UVW_FETCH_LIBUV)
        include(FetchContent)

        FetchContent_Declare(
            libuv
            GIT_REPOSITORY https://github.com/libuv/libuv.git
            GIT_TAG "v${UVW_LIBUV_VERSION}"
            GIT_SHALLOW 1
        )

        FetchContent_GetProperties(libuv)

        if(NOT libuv_POPULATED)
            FetchContent_Populate(libuv)
            add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        if(UVW_BUILD_SHARED_LIB)
            add_library(uv::uv-shared ALIAS uv)
            set_target_properties(uv PROPERTIES POSITION_INDEPENDENT_CODE 1)
        else()
            add_library(uv::uv-static ALIAS uv_a)
            set_target_properties(uv_a PROPERTIES POSITION_INDEPENDENT_CODE 1)
        endif()
    endif(UVW_FETCH_LIBUV)
endfunction()

function(use_libuv)
    set(UVW_FETCH_LIBUV_DEFAULT ON)

    if (UVW_FIND_LIBUV)
        find_package(libuv ${LIBUV_VERSION} QUIET)
        if (libuv_FOUND)
            add_library(uv::uv-shared ALIAS uv)
            set(UVW_FETCH_LIBUV_DEFAULT OFF)
            message(STATUS "libuv ${libuv_VERSION} found via cmake")
        else(libuv_FOUND)
            find_package(PkgConfig QUIET)
            if (PkgConfig_FOUND)
                pkg_check_modules(libuv IMPORTED_TARGET libuv>=${LIBUV_VERSION})
                if (libuv_FOUND)
                    add_library(uv::uv-shared ALIAS PkgConfig::libuv)
                    set(UVW_FETCH_LIBUV_DEFAULT OFF)
                    message(STATUS "libuv ${libuv_VERSION} found via pkg-config")
                endif(libuv_FOUND)
            endif(PkgConfig_FOUND)
        endif(libuv_FOUND)
    endif(UVW_FIND_LIBUV)

    option(UVW_FETCH_LIBUV "Fetch the libuv repo using CMake FetchContent facility" ${UVW_FETCH_LIBUV_DEFAULT})

    fetch_libuv()
endfunction()

#
# Add uvw target
#

include(GNUInstallDirs)

if(UVW_BUILD_LIBS)
    use_libuv()

    add_subdirectory(src)
    file(GLOB HEADERS src/uvw/*.h src/uvw/*.hpp)
else()
    add_library(uvw INTERFACE)
    add_library(uvw::uvw ALIAS uvw)

    target_compile_features(uvw INTERFACE cxx_std_17)

    target_include_directories(
        uvw
        INTERFACE
            $<BUILD_INTERFACE:${uvw_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    if(UVW_USE_ASAN)
        target_compile_options(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
        target_link_libraries(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()

    if(UVW_USE_UBSAN)
        target_compile_options(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer>)
        target_link_libraries(uvw INTERFACE $<$<CONFIG:Debug>:-fsanitize=undefined>)
    endif()

    if(UVW_CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY "${UVW_CLANG_TIDY_EXECUTABLE};--config-file=${uvw_SOURCE_DIR}/.clang-tidy;--header-filter=${uvw_SOURCE_DIR}/src/uvw/.*")
    endif()

    if(UVW_HAS_LIBCPP)
        target_compile_options(uvw BEFORE INTERFACE -stdlib=libc++)
    endif()

    file(GLOB HEADERS src/uvw/*.h src/uvw/*.hpp)
endif()

#
# Install targets
#

install(
    FILES ${HEADERS}
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uvw
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

install(
    FILES src/uvw.hpp
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

#
# Install targets
#

if (UVW_BUILD_LIBS)
    set_target_properties(
        uvw PROPERTIES
        VERSION ${UVW_VERSION_MAJOR}.${UVW_VERSION_MINOR}.${UVW_VERSION_PATCH}
        SOVERSION ${UVW_VERSION_MAJOR}
    )
endif()

install(
    EXPORT uvwConfig 
    NAMESPACE uvw:: 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uvw
)

install(
    TARGETS uvw 
    EXPORT uvwConfig
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(UVW_FETCH_LIBUV AND UVW_BUILD_LIBS)
    # libuv is only fetched when both above conditions are true
    install(DIRECTORY ${libuv_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uvw/uv/include)
    if (UVW_BUILD_SHARED_LIB)
        install(TARGETS uv EXPORT uvwConfig LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/uvw)
    else()
        install(TARGETS uv_a EXPORT uvwConfig ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/uvw)
    endif()
endif(UVW_FETCH_LIBUV AND UVW_BUILD_LIBS)

export(EXPORT uvwConfig)

### Testing

option(UVW_BUILD_TESTING "Enable testing with ctest." OFF)

if(UVW_BUILD_TESTING)
    option(UVW_FIND_GTEST_PACKAGE "Enable finding gtest package." OFF)

    if (NOT UVW_BUILD_LIBS)
        use_libuv()
    endif()

    enable_testing()
    add_subdirectory(test)
endif()

#
# Documentation
#

option(UVW_BUILD_DOCS "Enable building with documentation." OFF)

if(UVW_BUILD_DOCS)
    find_package(Doxygen 1.10)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()
