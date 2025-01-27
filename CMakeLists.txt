# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.14)

# ---------------------------------------------------------------------------------------
# Start spdlog project
# ---------------------------------------------------------------------------------------
include(cmake/utils.cmake)
include(cmake/ide.cmake)

spdlog_extract_version()

project(spdlog VERSION ${SPDLOG_VERSION} LANGUAGES CXX)

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------------------------
# Set default build to release
# ---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

# ---------------------------------------------------------------------------------------
# Compiler config
# ---------------------------------------------------------------------------------------
# c++ standard >=17 is required
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
elseif(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "Minimum supported CMAKE_CXX_STANDARD is 17, but it is set to ${CMAKE_CXX_STANDARD}")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME MATCHES "CYGWIN" OR CMAKE_SYSTEM_NAME MATCHES "MSYS" OR CMAKE_SYSTEM_NAME MATCHES "MINGW")
    set(CMAKE_CXX_EXTENSIONS ON)
endif()

# ---------------------------------------------------------------------------------------
# Set SPDLOG_MASTER_PROJECT to ON if we are building spdlog
# ---------------------------------------------------------------------------------------
# Check if spdlog is being used directly or via add_subdirectory, but allow overriding
if(NOT DEFINED SPDLOG_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(SPDLOG_MASTER_PROJECT ON)
    else()
        set(SPDLOG_MASTER_PROJECT OFF)
    endif()
endif()

option(SPDLOG_BUILD_ALL "Build all artifacts" OFF)

# build shared option
option(SPDLOG_BUILD_SHARED "Build shared library" OFF)

# example options
option(SPDLOG_BUILD_EXAMPLE "Build example" ${SPDLOG_MASTER_PROJECT})

# testing options
option(SPDLOG_BUILD_TESTS "Build tests" OFF)

# bench options
option(SPDLOG_BUILD_BENCH "Build benchmarks (Requires https://github.com/google/benchmark.git to be installed)" OFF)

# sanitizer options
option(SPDLOG_SANITIZE_ADDRESS "Enable address sanitizer in tests" OFF)

# warning options
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" OFF)

# install options
option(SPDLOG_SYSTEM_INCLUDES "Include as system headers (skip for clang-tidy)." OFF)
option(SPDLOG_INSTALL "Generate the install target" ${SPDLOG_MASTER_PROJECT})
option(SPDLOG_FMT_EXTERNAL "Use external fmt library instead of of fetching from gitub." OFF)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    option(SPDLOG_CLOCK_COARSE "Use CLOCK_REALTIME_COARSE instead of the regular clock," OFF)
else()
    set(SPDLOG_CLOCK_COARSE OFF CACHE BOOL "non supported option" FORCE)
endif()

option(SPDLOG_PREVENT_CHILD_FD "Prevent from child processes to inherit log file descriptors" OFF)
option(SPDLOG_NO_THREAD_ID "prevent spdlog from querying the thread id on each log call if thread id is not needed" OFF)
option(SPDLOG_DISABLE_GLOBAL_LOGGER "Disable global logger creation" OFF)
option(SPDLOG_NO_TLS "Disable thread local storage" OFF)

# clang-tidy
option(SPDLOG_TIDY "run clang-tidy" OFF)

if(SPDLOG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Enabled clang-tidy")
endif()

if(SPDLOG_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # place dlls and libs and executables in the same directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)

    # make sure __cplusplus is defined
    add_compile_options(/Zc:__cplusplus)
    # enable parallel build for the solution
    add_compile_options(/MP)
endif()

message(STATUS "spdlog version: ${SPDLOG_VERSION}")
message(STATUS "spdlog build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "spdlog build shared: " ${BUILD_SHARED_LIBS})
message(STATUS "spdlog fmt external: " ${SPDLOG_FMT_EXTERNAL})

# ---------------------------------------------------------------------------------------
# Find {fmt} library
# ---------------------------------------------------------------------------------------
if(SPDLOG_FMT_EXTERNAL)
    find_package(fmt REQUIRED)
    message(STATUS "Using external fmt lib version: ${fmt_VERSION}")
else()
    include(cmake/fmtlib.cmake)
endif()

# ---------------------------------------------------------------------------------------
# Threads library is required
# ---------------------------------------------------------------------------------------
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------------------
# Library sources
# ---------------------------------------------------------------------------------------
set(SPDLOG_HEADERS
    "include/spdlog/async.h"
    "include/spdlog/async_logger.h"
    "include/spdlog/common.h"
    "include/spdlog/formatter.h"
    "include/spdlog/fwd.h"
    "include/spdlog/logger.h"
    "include/spdlog/pattern_formatter.h"
    "include/spdlog/source_loc.h"
    "include/spdlog/spdlog.h"
    "include/spdlog/stopwatch.h"
    "include/spdlog/version.h"
    "include/spdlog/details/circular_q.h"
    "include/spdlog/details/file_helper.h"
    "include/spdlog/details/fmt_helper.h"
    "include/spdlog/details/log_msg.h"
    "include/spdlog/details/log_msg_buffer.h"
    "include/spdlog/details/mpmc_blocking_q.h"
    "include/spdlog/details/null_mutex.h"
    "include/spdlog/details/os.h"
    "include/spdlog/details/periodic_worker.h"
    "include/spdlog/details/context.h"
    "include/spdlog/details/synchronous_factory.h"
    "include/spdlog/details/thread_pool.h"
    "include/spdlog/fmt/bin_to_hex.h"
    "include/spdlog/fmt/fmt.h"
    "include/spdlog/sinks/android_sink.h"
    "include/spdlog/sinks/base_sink.h"
    "include/spdlog/sinks/basic_file_sink.h"
    "include/spdlog/sinks/callback_sink.h"
    "include/spdlog/sinks/daily_file_sink.h"
    "include/spdlog/sinks/dist_sink.h"
    "include/spdlog/sinks/dup_filter_sink.h"
    "include/spdlog/sinks/hourly_file_sink.h"
    "include/spdlog/sinks/kafka_sink.h"
    "include/spdlog/sinks/mongo_sink.h"
    "include/spdlog/sinks/msvc_sink.h"
    "include/spdlog/sinks/null_sink.h"
    "include/spdlog/sinks/ostream_sink.h"
    "include/spdlog/sinks/qt_sinks.h"
    "include/spdlog/sinks/ringbuffer_sink.h"
    "include/spdlog/sinks/rotating_file_sink.h"
    "include/spdlog/sinks/sink.h"
    "include/spdlog/sinks/stdout_color_sinks.h"
    "include/spdlog/sinks/stdout_sinks.h"
    "include/spdlog/sinks/syslog_sink.h"
    "include/spdlog/sinks/systemd_sink.h"
    "include/spdlog/sinks/tcp_sink.h"
    "include/spdlog/sinks/udp_sink.h")

set(SPDLOG_SRCS
    "src/async_logger.cpp"
    "src/common.cpp"
    "src/logger.cpp"
    "src/pattern_formatter.cpp"
    "src/spdlog.cpp"
    "src/details/file_helper.cpp"
	"src/details/os_filesystem.cpp"
    "src/details/log_msg.cpp"
    "src/details/log_msg_buffer.cpp"
        "src/details/context.cpp"
    "src/details/thread_pool.cpp"
    "src/sinks/base_sink.cpp"
    "src/sinks/basic_file_sink.cpp"
    "src/sinks/rotating_file_sink.cpp"
    "src/sinks/sink.cpp"
    "src/sinks/stdout_color_sinks.cpp"
    "src/sinks/stdout_sinks.cpp")

if(WIN32)
    list(APPEND SPDLOG_SRCS
            "src/details/os_windows.cpp"
            "src/sinks/wincolor_sink.cpp")
    list(
        APPEND SPDLOG_HEADERS
            "include/spdlog/sinks/wincolor_sink.h"
            "include/spdlog/details/tcp_client_windows.h"
            "include/spdlog/details/udp_client_windows.h"
            "include/spdlog/details/windows_include.h"
            "include/spdlog/sinks/win_eventlog_sink.h")
else()
    list(APPEND SPDLOG_SRCS
            "src/details/os_unix.cpp"
            "src/sinks/ansicolor_sink.cpp")
    list(APPEND SPDLOG_HEADERS
        "include/spdlog/details/tcp_client_unix.h"
        "include/spdlog/details/udp_client_unix.h"
        "include/spdlog/sinks/ansicolor_sink.h")
endif()

# ---------------------------------------------------------------------------------------
# Check if fwrite_unlocked/_fwrite_nolock is available
# ---------------------------------------------------------------------------------------
include(CheckSymbolExists)
if(WIN32)
    check_symbol_exists(_fwrite_nolock "stdio.h" HAVE_FWRITE_UNLOCKED)
else()
    check_symbol_exists(fwrite_unlocked "stdio.h" HAVE_FWRITE_UNLOCKED)
endif()
if(HAVE_FWRITE_UNLOCKED)
    set(SPDLOG_FWRITE_UNLOCKED 1)
endif()

# ---------------------------------------------------------------------------------------
# spdlog library
# ---------------------------------------------------------------------------------------
if(BUILD_SHARED_LIBS)
    if(WIN32)
        set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
    endif()
    add_library(spdlog SHARED ${VERSION_RC})
    target_compile_definitions(spdlog PUBLIC SPDLOG_SHARED_LIB)
    if(MSVC)
        # disable dlls related warnings on msvc
        target_compile_options(spdlog PUBLIC $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<COMPILE_LANGUAGE:CUDA>>>:/wd4251
                                             /wd4275>)
    endif()
else()
    add_library(spdlog STATIC)
endif()
add_library(spdlog::spdlog ALIAS spdlog)
target_sources(spdlog PRIVATE ${SPDLOG_SRCS})
target_sources(
    spdlog
    PUBLIC FILE_SET pub_headers
           TYPE HEADERS
           BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
           FILES ${SPDLOG_HEADERS})

set(SPDLOG_INCLUDES_LEVEL "")
if(SPDLOG_SYSTEM_INCLUDES)
    set(SPDLOG_INCLUDES_LEVEL "SYSTEM")
endif()

target_include_directories(spdlog ${SPDLOG_INCLUDES_LEVEL} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
                                                                  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(spdlog PUBLIC Threads::Threads)
target_link_libraries(spdlog PUBLIC fmt::fmt)
spdlog_enable_warnings(spdlog)
set_target_properties(spdlog PROPERTIES VERSION ${SPDLOG_VERSION} SOVERSION
                                                                  ${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR})

set(SPDLOG_NAME spdlog-${SPDLOG_VERSION_MAJOR})
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX  "-${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR}d")

# ---------------------------------------------------------------------------------------
# set source groups for visual studio
# ---------------------------------------------------------------------------------------
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX include FILES ${SPDLOG_HEADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX sources FILES ${SPDLOG_SRCS})
    source_group(sources FILES ${VERSION_RC})
endif()

# ---------------------------------------------------------------------------------------
# Add required libraries for Android CMake build
# ---------------------------------------------------------------------------------------
if(ANDROID)
    target_link_libraries(spdlog PUBLIC log)
endif()

# ---------------------------------------------------------------------------------------
# spdlog private defines according to the options
# ---------------------------------------------------------------------------------------
foreach(SPDLOG_OPTION
    SPDLOG_CLOCK_COARSE
    SPDLOG_PREVENT_CHILD_FD
    SPDLOG_NO_THREAD_ID
    SPDLOG_DISABLE_GLOBAL_LOGGER
    SPDLOG_NO_TLS
    SPDLOG_FWRITE_UNLOCKED)
    if(${SPDLOG_OPTION})
        target_compile_definitions(spdlog PRIVATE ${SPDLOG_OPTION})
    endif()
endforeach()

# ---------------------------------------------------------------------------------------
# Build binaries
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating example(s)")
    add_subdirectory(example)
    spdlog_enable_warnings(example)
endif()

if(SPDLOG_BUILD_TESTS OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if(SPDLOG_BUILD_BENCH OR SPDLOG_BUILD_ALL)
    message(STATUS "Generating benchmarks")
    add_subdirectory(bench)
endif()

# ---------------------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------------------
if(SPDLOG_INSTALL)
    message(STATUS "Generating install")
    set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/spdlogConfig.cmake.in")
    set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfig.cmake")
    set(config_targets_file "spdlogConfigTargets.cmake")
    set(version_config_file "${CMAKE_CURRENT_BINARY_DIR}/spdlogConfigVersion.cmake")
    set(export_dest_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${SPDLOG_NAME}")

    # ---------------------------------------------------------------------------------------
    # Include files
    # ---------------------------------------------------------------------------------------
    set(installed_include_dir "${CMAKE_INSTALL_INCLUDEDIR}/${SPDLOG_NAME}")
    install(
        TARGETS spdlog
        EXPORT spdlogTargets
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${SPDLOG_NAME}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${SPDLOG_NAME}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/${SPDLOG_NAME}"
        FILE_SET pub_headers
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${SPDLOG_NAME}")
    message(STATUS "Installing spdlog in ${CMAKE_INSTALL_LIBDIR}/${SPDLOG_NAME}")

    # ---------------------------------------------------------------------------------------
    # Install CMake config files
    # ---------------------------------------------------------------------------------------
    install(EXPORT spdlogTargets DESTINATION ${export_dest_dir} NAMESPACE spdlog:: FILE ${config_targets_file})

    include(CMakePackageConfigHelpers)
    configure_package_config_file("${project_config_in}" "${project_config_out}" INSTALL_DESTINATION ${export_dest_dir})

    write_basic_package_version_file("${version_config_file}" COMPATIBILITY SameMajorVersion)
    install(FILES "${project_config_out}" "${version_config_file}" DESTINATION "${export_dest_dir}")

    # ---------------------------------------------------------------------------------------
    # Support creation of installable packages
    # ---------------------------------------------------------------------------------------
    include(cmake/spdlogCPack.cmake)
endif()
