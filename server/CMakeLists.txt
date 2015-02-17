cmake_minimum_required(VERSION 2.8.0)
project(Urobot)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    )

set (VERSION "1.1.0")
set (PACKAGE "urobot")
set (PACKAGE_NAME "@PACKAGE@")
set (SYSCONFDIR "/usr/local/etc")
set (LOCALEDIR "/usr/local/share/locale")
set (CONFIG_NLS 1)

include (FindPkgConfig)

### ---[ Find universal dependencies

find_package(PkgConfig)

# pthread
find_package(Threads REQUIRED)

# libupnp
find_package(LibUpnp REQUIRED)
include_directories(SYSTEM ${LIBUPNP_INCLUDE_DIR})

# libjpeg
find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

# libctemplate
find_package(Ctemplate REQUIRED)
include_directories(${CTEMPLATE_INCLUDE_DIR})

# OpenNI
find_package(OpenNI)
if (OPENNI_FOUND)
  set(HAVE_OPENNI ON)
  include_directories(SYSTEM ${OPENNI_INCLUDE_DIRS})
endif(OPENNI_FOUND)

# Kobuki Driver
find_package(KobukiDriver)
if (KOBUKI_DRIVER_FOUND)
  set(HAVE_KOBUKI_DRIVER ON)
  include_directories(SYSTEM ${KOBUKI_DRIVER_INCLUDE_DIRS})
endif(KOBUKI_DRIVER_FOUND)

# RaspberryPi
find_package(Rpi)
if (RPI_FOUND)
  set(HAVE_RPI ON)
  include_directories(SYSTEM ${RPI_INCLUDE_DIRS})
endif(RPI_FOUND)

include(CheckCCompilerFlag)
check_c_compiler_flag(-D_FILE_OFFSET_BITS=64 HAS_FILE_OFFSET_BITS64)
if (HAS_FILE_OFFSET_BITS64)
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64)
endif (HAS_FILE_OFFSET_BITS64)

add_subdirectory(src)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
