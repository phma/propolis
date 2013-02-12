project(propolis)
cmake_minimum_required(VERSION 2.8)

add_executable(propolis propolis.cpp hvec.cpp letters.cpp galois.cpp rs.cpp contour.cpp ps.cpp arrange.cpp encoding.cpp)

install(TARGETS propolis DESTINATION bin)
include(TestBigEndian)
include(CheckIncludeFiles)

check_include_files(time.h HAVE_TIME_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(windows.h HAVE_WINDOWS_H)
test_big_endian(BIGENDIAN)

set(PROPOLIS_MAJOR_VERSION 0)
set(PROPOLIS_MINOR_VERSION 1)
set(PROPOLIS_PATCH_VERSION 0)
set(PROPOLIS_VERSION ${PROPOLIS_MAJOR_VERSION}.${PROPOLIS_MINOR_VERSION}.${PROPOLIS_PATCH_VERSION})

include_directories(${PROJECT_BINARY_DIR})
configure_file (config.h.in config.h)

set(CPACK_PACKAGE_VERSION_MAJOR ${PROPOLIS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${PROPOLIS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${PROPOLIS_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)
