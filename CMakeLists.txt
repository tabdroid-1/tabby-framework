cmake_minimum_required(VERSION 3.22)
project(tabby
	VERSION 0.0.1
    DESCRIPTION "Framework"
	HOMEPAGE_URL "https://github.com/tabdroid-1/tabby-framework"
    LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    message(STATUS "Including Objective-C++ for APPLE")
    list(APPEND LANGUAGES OBJC)
    list(APPEND LANGUAGES OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 14)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()


option(TB_DIST "Enable distrubition build" OFF)
option(TB_PROFILE "Enable profiling" OFF)
option(TB_HEADLESS "Enable headless mode" OFF)

add_library(tabby_flags INTERFACE)

if(TB_DIST)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build utility programs" FORCE)
    set(TB_PROFILE OFF CACHE BOOL "" FORCE)
endif(TB_DIST)

if(TB_PROFILE)
    target_compile_definitions(tabby_flags INTERFACE 
        TB_PROFILE=1 
        TRACY_ENABLE=1
        )
    add_compile_definitions(
        TB_PROFILE=1 
        TRACY_ENABLE=1
        )
    message(WARNING "Tabby: Extremely high memory usage when profiler is enabled")
endif(TB_PROFILE)

if(TB_HEADLESS)
    target_compile_definitions(tabby_flags INTERFACE TB_HEADLESS=1)
    add_compile_definitions(TB_HEADLESS=1)
else()
    target_compile_definitions(tabby_flags INTERFACE TB_HEADLESS=0)
    add_compile_definitions(TB_HEADLESS=0)
endif(TB_HEADLESS)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build a static version of the library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build the SDL3_test library" FORCE)
set(ENKITS_BUILD_C_INTERFACE OFF CACHE BOOL "Build C interface" FORCE)
set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build example applications" FORCE)
set(SPDLOG_HEADER_ONLY OFF CACHE BOOL "" FORCE)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(BGFX_CONFIG_MULTITHREADED OFF CACHE BOOL "" FORCE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BX_CONFIG_DEBUG ON CACHE BOOL "" FORCE)
else ()
    set(BX_CONFIG_DEBUG OFF CACHE BOOL "" FORCE)
endif()

set(PROJECT_SOURCE_DIR "src")
set(PROJECT_INCLUDE_DIR "include")
set(PROJECT_VENDOR_DIR "vendor")

# ##############################################################################
# Add dependencies 
# ##############################################################################

set(SDL ${PROJECT_VENDOR_DIR}/SDL)
set(BGFX ${PROJECT_VENDOR_DIR}/bgfx)
set(TRACY ${PROJECT_VENDOR_DIR}/tracy)
set(ENKITS ${PROJECT_VENDOR_DIR}/enkiTS)
set(SPDLOG ${PROJECT_VENDOR_DIR}/spdlog)

# ##############################################################################
# Project executable setup
# ##############################################################################

file(GLOB_RECURSE PROJECT_SOURCES 
        ${PROJECT_INCLUDE_DIR}/*.h
        ${PROJECT_SOURCE_DIR}/*.h
        ${PROJECT_SOURCE_DIR}/*.cpp
        )

# # Additional project sources (vendors, etc.)
list(APPEND PROJECT_SOURCES
        )

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

get_filename_component(SDL_INCLUDE_ABS "${SDL}/include" ABSOLUTE)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}) 
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIR}) 
target_include_directories(${PROJECT_NAME} PUBLIC ${SPDLOG}/include) 
target_include_directories(${PROJECT_NAME} PUBLIC ${BGFX}/bgfx/3rdparty) 
target_include_directories(${PROJECT_NAME} PUBLIC ${BGFX}/bgfx/include) 
target_include_directories(${PROJECT_NAME} PUBLIC ${BGFX}/bimg/include) 
target_include_directories(${PROJECT_NAME} PUBLIC ${BGFX}/bx/include) 
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL}/include) 
target_include_directories(${PROJECT_NAME} PUBLIC ${ENKITS}/src) 
target_include_directories(${PROJECT_NAME} PUBLIC ${TRACY}) 

# ####################
# Add Subdirectories
# ####################



if (NOT TARGET bgfx)
    add_subdirectory(${BGFX})
endif()
if (NOT TARGET SDL3-static)
    add_subdirectory(${SDL})
endif()
if (NOT TARGET tracy)
    add_subdirectory(${TRACY})
endif()
if (NOT TARGET enkiTS)
    add_subdirectory(${ENKITS})
endif()
if (NOT TARGET spdlog)
    add_subdirectory(${SPDLOG})
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Building for Windows")

    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /experimental:c11atomics console)

        if(CMAKE_BUILD_TYPE MATCHES "Debug")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
        endif()
    endif()

    if (MINGW)
        set(CMAKE_SYSTEM_NAME Windows)
        set(ENABLE_STATIC_RUNTIME ON CACHE BOOL "Enable static runtime" FORCE)
        target_link_options(${PROJECT_NAME} PUBLIC -static)

        # does this even work?
        if(TB_DIST)
            target_link_options(${PROJECT_NAME} PUBLIC -mwindows)
        else()
            target_link_options(${PROJECT_NAME} PUBLIC -mconsole)
        endif()
    endif()

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Building for Linux")

    target_compile_definitions(bgfx PRIVATE "WL_EGL_PLATFORM=1")
    target_link_libraries(bgfx PRIVATE wayland-egl) 
    target_link_libraries(${PROJECT_NAME} PUBLIC X11)

    target_link_options(${PROJECT_NAME} PUBLIC -ludev)

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if(CMAKE_OSX_SYSROOT STREQUAL "iphoneos" OR CMAKE_OSX_SYSROOT STREQUAL "iphonesimulator")
        message(STATUS "Building for iOS")
    else()
        message(STATUS "Building for macOS")

        target_link_libraries(${PROJECT_NAME} PRIVATE 
            "-framework QuartzCore" 
            "-framework Metal" 
            "-framework Foundation" 
            "-framework IOSurface" 
            "-framework CoreGraphics" 
            "-framework IOKit" 
            "-framework AppKit" 
            )
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    message(STATUS "Building for Android")

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        log
        EGL
        GLESv2
        GLESv3
        android
        )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    message(STATUS "Building for Emscripten (WebAssembly)")

    target_link_options(${PROJECT_NAME} PUBLIC "-sFORCE_FILESYSTEM=1")
    target_link_options(${PROJECT_NAME} PUBLIC "-sALLOW_MEMORY_GROWTH")
    target_link_options(${PROJECT_NAME} PUBLIC "-sOFFSCREEN_FRAMEBUFFER=1")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${PROJECT_NAME} PUBLIC "-sEXIT_RUNTIME=1")
        target_link_options(${PROJECT_NAME} PUBLIC "-sASSERTIONS=1")
    endif()
else()
    message(FATAL_ERROR "Unknown platform: ${CMAKE_SYSTEM_NAME}")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
    bx
    bgfx
    bimg
    tracy 
    enkiTS
    spdlog
    SDL3::SDL3-static
    )



if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")
    target_compile_definitions(tabby PUBLIC 
        BX_CONFIG_DEBUG
        TB_DEBUG 
        TRACY_ENABLE
    )

    add_compile_options(
      "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
    )
else (CMAKE_BUILD_TYPE STREQUAL "Release" AND ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    target_compile_definitions(tabby PUBLIC 
        BX_CONFIG_DEBUG
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Ensure that hot-reload is enabled for VS
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/tbpch.h")
