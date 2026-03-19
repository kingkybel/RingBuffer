# Repository:  https://github.com/kingkybel/RingBuffer
# File Name:   test/CMakeLists.txt
# Description: CMake configuration for RingBuffer tests.
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

include(CTest)
include(FetchContent)

# Prefer system-installed GTest but fall back to FetchContent so CI can build.
find_package(GTest QUIET)

if(NOT TARGET GTest::gtest)
    message(STATUS "Fetching GoogleTest for test build")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.14.0
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Create the test executable
add_executable(run_tests
    ring_buffer_tests.cc
    thread_safe_ring_buffer_tests.cc
)

# Link with GoogleTest and RingBuffer
target_link_libraries(run_tests
    GTest::gtest
    GTest::gtest_main
    DebugTrace
    pthread
)

# Add the test
add_test(NAME RingBufferTests COMMAND run_tests)
