# Repository:  https://github.com/kingkybel/RingBuffer
# File Name:   examples/CMakeLists.txt
# Description: CMake configuration for RingBuffer examples.
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

# Create the executables
add_executable(audio_buffer_example audio_buffer_example.cc)
add_executable(logging_example logging_example.cc)
add_executable(producer_consumer_example producer_consumer_example.cc)
