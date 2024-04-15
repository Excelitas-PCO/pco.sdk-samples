cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(samples-pco-sdk
    VERSION 1.0.0
    DESCRIPTION "Simple examples showing the use of pco.sdk"
    LANGUAGES CXX)

set(CMAKE_BUILD_RPATH "\$ORIGIN;\$ORIGIN/lib")
set(CMAKE_INSTALL_RPATH "\$ORIGIN;\$ORIGIN/../lib")

if(UNIX)
    add_compile_definitions(PCO_LINUX)
endif(UNIX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# pco dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/externals/pco)
set(PCO_FOLDER "${CMAKE_SOURCE_DIR}/externals/pco")

if(WIN32)
    add_compile_definitions(_FILEFUNCTION_)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/win/console)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/win/sc2_demo)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/win/test_cvdlg)
else()
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/lnx/simple_open)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/lnx/read_from_camera_camram)
    add_subdirectory(${CMAKE_SOURCE_DIR}/src/lnx/grab_while_camera_running)
endif()
