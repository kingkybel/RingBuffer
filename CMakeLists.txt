# Repository:  https://github.com/kingkybel/RingBuffer
# File Name:   CMakeLists.txt
# Description: Main CMake configuration for RingBuffer library.
#
# Copyright (C) 2026 Dieter J Kybelksties
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# @date: 2026-03-13
# @author: Dieter J Kybelksties


cmake_minimum_required(VERSION 3.26)
project(RingBuffer
        VERSION 1.0.0
        DESCRIPTION "A header only for conversion of standard containers"
        HOMEPAGE_URL "https://github.com/kingkybel/RingBuffer")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CTest)
enable_testing()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-common/cmake/DkybBuildSettings.cmake)
dkyb_apply_common_settings(
        EXTRA_CXX_FLAGS "-DBOOST_ALLOW_DEPRECATED_HEADERS" "-DBOOST_BIND_GLOBAL_PLACEHOLDERS"
)

execute_process(
        COMMAND bash -c "printf \"%s\" $(cat /etc/os-release | grep -E \"^ID=[^\\n]*|^VERSION_ID=[^\\n]*\" | cut -f2 -d= | tr -d '\"')"
        OUTPUT_VARIABLE LINUX_VERSION
)
message(STATUS "Linux version is '${LINUX_VERSION}'")

set(CMAKE_FIND_LIBRARY_PREFIXES lib)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(
        ${Boost_INCLUDE_DIRS}
        BEFORE SYSTEM ${CMAKE_SOURCE_DIR}/include
        system
)
link_directories(
        /usr/local/lib
        ${Boost_LIBRARY_DIR_RELEASE}
        system
)

add_subdirectory(test)
add_subdirectory(examples)
add_subdirectory(benchmarks)

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=* -p ${CMAKE_SOURCE_DIR}/build")

file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

# Install the header files directly under 'include/dkyb'
install(FILES ${HEADER_FILES}
        DESTINATION include/dkyb)
