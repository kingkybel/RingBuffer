cmake_minimum_required(VERSION 3.26)
project(DkybCMakeCommon
        VERSION 0.1.0
        DESCRIPTION "Shared CMake settings and helpers for dkyb C++ repos"
        HOMEPAGE_URL "https://github.com/kingkybel/cmake-common")

include(cmake/DkybBuildSettings.cmake)

message(STATUS "DkybCMakeCommon loaded. Use include(cmake/DkybBuildSettings.cmake) from consumers.")
