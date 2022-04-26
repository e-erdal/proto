cmake_minimum_required(VERSION 3.21)
project(bimg)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

if(NOT BIMG_DIR)
	set(BIMG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bimg" CACHE STRING "Location of bimg.")
elseif(NOT IS_ABSOLUTE "${BIMG_DIR}")
	get_filename_component(BIMG_DIR "${BIMG_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if (NOT BX_DIR)
    message(FATAL_ERROR "bimg-cmake requires bx-cmake.")
endif()

file(GLOB BIMG_SOURCES ${BIMG_DIR}/src/*.cpp)
file(GLOB BIMG_ASTC_SOURCES ${BIMG_DIR}/3rdparty/astc/*.cpp)
file(GLOB BIMG_ASTCC_SOURCES ${BIMG_DIR}/3rdparty/astc-codec/src/decoder/*.cc)
file(GLOB BIMG_MINIZ_SOURCES ${BIMG_DIR}/3rdparty/tinyexr/deps/miniz/*.c)
file(GLOB BIMG_ETC1_SOURCES ${BIMG_DIR}/3rdparty/etc1/*.cpp)
file(GLOB BIMG_ETC2_SOURCES ${BIMG_DIR}/3rdparty/etc2/*.cpp)
file(GLOB BIMG_NVTT_SOURCES ${BIMG_DIR}/3rdparty/nvtt/*.cpp)
file(GLOB BIMG_PVRTC_SOURCES ${BIMG_DIR}/3rdparty/pvrtc/*.cpp)
file(GLOB BIMG_IQA_SOURCES ${BIMG_DIR}/3rdparty/iqa/source/*.cpp)
file(GLOB BIMG_LIBSQUISH_SOURCES ${BIMG_DIR}/3rdparty/libsquish/*.cpp)

add_library(bimg STATIC 
        ${BIMG_SOURCES} 
        ${BIMG_MINIZ_SOURCES} 
        ${BIMG_ASTC_SOURCES}
        ${BIMG_ASTCC_SOURCES}
        ${BIMG_ETC1_SOURCES}
        ${BIMG_ETC2_SOURCES}
        ${BIMG_NVTT_SOURCES}
        ${BIMG_PVRTC_SOURCES}
        ${BIMG_IQA_SOURCES}
        ${BIMG_LIBSQUISH_SOURCES}
    )

    target_include_directories(bimg PUBLIC
        ${BIMG_DIR}/include 
        ${BIMG_DIR}/3rdparty
        ${BIMG_DIR}/3rdparty/nvtt 
        ${BIMG_DIR}/3rdparty/iqa/include 
        ${BIMG_DIR}/3rdparty/astc-codec 
        ${BIMG_DIR}/3rdparty/astc-codec/include
        ${BIMG_DIR}/3rdparty/libsquish
        ${BIMG_DIR}/3rdparty/tinyexr/deps/miniz
    )

    target_link_libraries(bimg PUBLIC bx)

    if (UNIX AND NOT APPLE)
        target_compile_options(bimg PUBLIC "-fPIC")
    endif()