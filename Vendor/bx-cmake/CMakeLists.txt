cmake_minimum_required(VERSION 3.21)
project(bx)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

option(BX_AMALGAMATED  "Amalgamated bx build for faster compilation"    OFF)
option(BX_CONFIG_DEBUG "Enables debug configuration"                    OFF)

# Get current compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(IS_CLANG true)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(IS_GCC true)
endif()

if (UNIX AND NOT APPLE)
    set (IS_LINUX true)
endif()

if(NOT BX_DIR)
	set(BX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bx" CACHE STRING "Location of bx.")
elseif(NOT IS_ABSOLUTE "${BX_DIR}")
	get_filename_component(BX_DIR "${BX_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if(NOT IS_DIRECTORY ${BX_DIR})
	message(SEND_ERROR "Could not load bx, directory does not exist. ${BX_DIR}")
	return()
endif()

# Get bx source
file(GLOB BX_SOURCES ${BX_DIR}/src/*.cpp)

if(BX_AMALGAMATED)
	set(BX_NOBUILD ${BX_SOURCES})
	list(REMOVE_ITEM BX_NOBUILD ${BX_DIR}/src/amalgamated.cpp)
	foreach(BX_SRC ${BX_NOBUILD})
		set_source_files_properties(${BX_SRC} PROPERTIES HEADER_FILE_ONLY ON)
	endforeach()
else()
	set_source_files_properties( ${BX_DIR}/src/amalgamated.cpp PROPERTIES HEADER_FILE_ONLY ON )
endif()

add_library(bx STATIC ${BX_SOURCES})
    target_include_directories(bx PUBLIC ${BX_DIR}/include ${BX_DIR}/3rdparty)

    if(APPLE)
        target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/osx)
    endif()

    target_compile_definitions(bx PRIVATE "$<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>")
    if(BX_CONFIG_DEBUG)
        target_compile_definitions(bx PRIVATE BX_CONFIG_DEBUG=1)
    endif()

    target_compile_definitions(bx PUBLIC "__STDC_LIMIT_MACROS")
    target_compile_definitions(bx PUBLIC "__STDC_FORMAT_MACROS")
    target_compile_definitions(bx PUBLIC "__STDC_CONSTANT_MACROS")

    # OS specific settings
    if (MSVC)
        target_compile_definitions(bx PUBLIC "_HAS_EXCEPTIONS=0")
        target_compile_definitions(bx PUBLIC "_SCL_SECURE=0")
        target_compile_definitions(bx PUBLIC "_SECURE_SCL=0")
        target_compile_definitions(bx PUBLIC "_SCL_SECURE_NO_WARNINGS")
        target_compile_definitions(bx PUBLIC "_CRT_SECURE_NO_WARNINGS")
        target_compile_definitions(bx PUBLIC "_CRT_SECURE_NO_DEPRECATE")

        target_compile_options(bx PUBLIC "/wd4201")     # warning C4201: nonstandard extension used: nameless struct/union
        target_compile_options(bx PUBLIC "/wd4324")     # warning C4324: '': structure was padded due to alignment specifier
        target_compile_options(bx PUBLIC "/Ob2")        # The Inline Function Expansion

        target_link_options(bx PUBLIC "/ignore:4221")       # LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library

        if (MSVC_VERSION EQUAL 1600) # vs2008
            target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/msvc/pre1600)
        endif()

        if(WINDOWS_STORE)
            target_compile_options(bx PUBLIC "/wd4530")     # vccorlib.h(1345): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc

            target_link_options(bx PUBLIC "/ignore:4264")       # LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
        endif()

        target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/msvc)

        target_link_libraries(bx psapi)
    endif()
    if (WIN32 AND MINGW)
        target_compile_definitions(bx PUBLIC "WIN32")
        target_compile_definitions(bx PUBLIC "MINGW_HAS_SECURE_API=1")

        target_compile_options(bx PUBLIC "-Wunused-value")
        target_compile_options(bx PUBLIC "-fdata-sections")
        target_compile_options(bx PUBLIC "-ffunction-sections")
        target_compile_options(bx PUBLIC "-msse2")
        target_compile_options(bx PUBLIC "-Wunused-value")
        target_compile_options(bx PUBLIC "-Wundef")

        target_link_options(bx PUBLIC "-Wl,--gc-sections")
        target_link_options(bx PUBLIC "-static")
        target_link_options(bx PUBLIC "-static-libgcc")
        target_link_options(bx PUBLIC "-static-libstdc++")
        
        target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/mingw)

        target_link_libraries(bx psapi)
    endif()
    if (WIN32 AND IS_CLANG)
        target_compile_definitions(bx PUBLIC "_HAS_EXCEPTIONS=0")
        target_compile_definitions(bx PUBLIC "_SCL_SECURE=0")
        target_compile_definitions(bx PUBLIC "_SECURE_SCL=0")
        target_compile_definitions(bx PUBLIC "_SCL_SECURE_NO_WARNINGS")
        target_compile_definitions(bx PUBLIC "_CRT_SECURE_NO_WARNINGS")
        target_compile_definitions(bx PUBLIC "_CRT_SECURE_NO_DEPRECATE")

        target_compile_options(bx PUBLIC "-Wshadow")
        target_compile_options(bx PUBLIC "-Wunused-value")
        
        target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/msvc)

        target_link_libraries(bx psapi)
    endif()
    if(NOT ANDROID AND IS_LINUX AND (IS_GCC OR IS_CLANG))
        if (IS_GCC)
            target_compile_options(bx PUBLIC "-mfpmath=sse")
        endif()
        target_compile_options(bx PUBLIC "-msse2")
        target_compile_options(bx PUBLIC "-Wshadow")
        target_compile_options(bx PUBLIC "-Wunused-value")
        target_compile_options(bx PUBLIC "-Wundef")

        target_link_options(bx PUBLIC "-Wl,--gc-sections")
        target_link_options(bx PUBLIC "-Wl,--as-needed")

        find_package(Threads)
        target_link_libraries(bx ${CMAKE_THREAD_LIBS_INIT} rt dl)
    endif()
    if(ANDROID)
        message("Currently building for Android.")
        include(cmake/android.cmake)

        # https://android.googlesource.com/platform/tools/cmake-utils/+/refs/heads/cmake-master-dev/android.toolchain.cmake#332
        if(NOT IS_DIRECTORY "${ANDROID_NDK}"
        OR NOT IS_DIRECTORY "${ANDROID_NDK}/platforms"
        OR NOT IS_DIRECTORY "${ANDROID_NDK}/prebuilt"
        OR NOT IS_DIRECTORY "${ANDROID_NDK}/sources"
        OR NOT IS_DIRECTORY "${ANDROID_NDK}/toolchains")
            message(FATAL_ERROR "Invalid Android NDK: ${ANDROID_NDK}.")
        elseif(NOT IS_DIRECTORY "${ANDROID_NDK}/platforms/${ANDROID_PLATFORM}")
            message(FATAL_ERROR "Invalid Android platform: ${ANDROID_PLATFORM}.")
        elseif(NOT IS_DIRECTORY "${CMAKE_SYSROOT}")
            message(FATAL_ERROR "Invalid Android sysroot: ${CMAKE_SYSROOT}.")
        endif()

        target_compile_options(bx PUBLIC "-fPIC")
        target_compile_options(bx PUBLIC "-no-canonical-prefixes")
        target_compile_options(bx PUBLIC "-Wa,--noexecstack")
        target_compile_options(bx PUBLIC "-fstack-protector-strong")
        target_compile_options(bx PUBLIC "-ffunction-sections")
        target_compile_options(bx PUBLIC "-Wunused-value")
        target_compile_options(bx PUBLIC "-Wundef")

        target_link_options(bx PUBLIC "-nostdlib")
        target_link_options(bx PUBLIC "-no-canonical-prefixes")
        target_link_options(bx PUBLIC "-Wl,--no-undefined")
        target_link_options(bx PUBLIC "-Wl,-z,noexecstack")
        target_link_options(bx PUBLIC "-Wl,-z,relro")
        target_link_options(bx PUBLIC "-Wl,-z,now")

        target_include_directories(bx PUBLIC "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/include")
        target_include_directories(bx PUBLIC "${ANDROID_NDK}/sysroot/usr/include")
        target_include_directories(bx PUBLIC "${ANDROID_NDK}/sources/android/native_app_glue")

        target_link_libraries(bx c dl m android log c++_shared gcc)
        
        target_compile_options(bx PUBLIC "--sysroot=${ANDROID_SYSROOT}")
        target_link_options(bx PUBLIC "--sysroot=${ANDROID_SYSROOT}")
        if (ANDROID_ABI EQUAL armeabi-v7a) # ARM
            target_compile_options(bx PUBLIC "-target armv7-none-linux-androideabi")
            target_compile_options(bx PUBLIC "-march=armv7-a")
            target_compile_options(bx PUBLIC "-mthumb")
            target_compile_options(bx PUBLIC "-mfloat-abi=softfp")
            target_compile_options(bx PUBLIC "-mfpu=neon")
            
            target_link_options(bx PUBLIC "-target armv7-none-linux-androideabi")
            target_link_options(bx PUBLIC "-march=armv7-a")

            target_include_directories(bx PUBLIC "${ANDROID_NDK}/sysroot/usr/include/arm-linux-androideabi")

            target_link_directories(bx PUBLIC "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a")
        elseif(ANDROID_ABI EQUAL arm64-v8a) # ARM64
            target_compile_options(bx PUBLIC "-target aarch64-none-linux-androideabi")
            target_compile_options(bx PUBLIC "-march=armv8-a")
            
            target_link_options(bx PUBLIC "-target aarch64-none-linux-androideabi")
            target_link_options(bx PUBLIC "-march=armv8-a")

            target_include_directories(bx PUBLIC "${ANDROID_NDK}/sysroot/usr/include/aarch64-linux-android")

            target_link_directories(bx PUBLIC "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/arm64-v8a")
        elseif(ANDROID_ABI EQUAL x86 OR ANDROID_ABI EQUAL x86_64) # x86_*
            target_compile_options(bx PUBLIC "-target i686-none-linux-android")
            target_compile_options(bx PUBLIC "-march=i686")
            target_compile_options(bx PUBLIC "-mtune=atom")
            target_compile_options(bx PUBLIC "-mstackrealign")
            target_compile_options(bx PUBLIC "-msse3")
            target_compile_options(bx PUBLIC "-mfpmath=sse")
            
            target_link_options(bx PUBLIC "-target i686-none-linux-android")

            target_include_directories(bx PUBLIC "${ANDROID_NDK}/sysroot/usr/include/x86_64-linux-android")

            target_link_directories(bx PUBLIC "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/x86")
        endif()
    endif()
    if (APPLE AND NOT IS_LINUX)
        if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            target_compile_options(bx PUBLIC "-Wno-error=unused-command-line-argument")
            target_compile_options(bx PUBLIC "-Wno-unused-command-line-argument")
            target_compile_options(bx PUBLIC "-Wfatal-errors")
            target_compile_options(bx PUBLIC "-Wunused-value")
            target_compile_options(bx PUBLIC "-Wundef")
            target_compile_options(bx PUBLIC "-Wshadow")

            target_include_directories(bx PUBLIC ${BX_DIR}/include/compat/osx)
        endif()
    endif()

