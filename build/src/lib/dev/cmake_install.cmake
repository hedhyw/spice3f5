# Install script for directory: /home/hedhyw/Development/Github/spice3f5/src/lib/dev

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/disto/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/asrc/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/bjt/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/bsim1/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/bsim2/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/cap/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/cccs/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/ccvs/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/csw/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/dio/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/ind/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/isrc/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/jfet/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/ltra/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/mes/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/mos1/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/mos2/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/mos3/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/mos6/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/res/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/sw/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/tra/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/urc/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/vccs/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/vcvs/cmake_install.cmake")
  include("/home/hedhyw/Development/Github/spice3f5/build/src/lib/dev/vsrc/cmake_install.cmake")

endif()

