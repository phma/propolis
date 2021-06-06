project(propolis)
cmake_minimum_required(VERSION 3.8.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17) # appeared in CMake 3.8
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/propolis)
add_definitions(-DBOOST_ALL_NO_LIB)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
set(Boost_USE_STATIC_LIBS ON)
else ()
set(Boost_USE_STATIC_LIBS OFF)
endif ()

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Threads REQUIRED)

add_executable(propolis propolis.cpp hvec.cpp letters.cpp contour.cpp genetic.cpp
               lagrange.cpp ps.cpp arrange.cpp encoding.cpp raster.cpp hamming.cpp threads.cpp)

target_link_libraries(propolis ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

install(TARGETS propolis DESTINATION bin)
include(TestBigEndian)
include(CheckIncludeFiles)

check_include_files(time.h HAVE_TIME_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(windows.h HAVE_WINDOWS_H)
test_big_endian(BIGENDIAN)

set(PROPOLIS_MAJOR_VERSION 0)
set(PROPOLIS_MINOR_VERSION 2)
set(PROPOLIS_PATCH_VERSION 0)
set(PROPOLIS_VERSION ${PROPOLIS_MAJOR_VERSION}.${PROPOLIS_MINOR_VERSION}.${PROPOLIS_PATCH_VERSION})

include_directories(${PROJECT_BINARY_DIR})
configure_file (config.h.in config.h)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROPOLIS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${PROPOLIS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${PROPOLIS_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)
