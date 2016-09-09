# This file is part of the Bus Pirate project
# (http://code.google.com/p/the-bus-pirate/).
#
# Written and maintained by the Bus Pirate project.
#
# To the extent possible under law, the project has
# waived all copyright and related or neighboring rights to Bus Pirate. This
# work is published from United States.
#
# For details see: http://creativecommons.org/publicdomain/zero/1.0/.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project("ft232testapp0")
set (SOURCE_FILES "ft232testapp0.cpp" "stdafx.cpp" "ftd2xx.h" "stdafx.h"
  "targetver.h")
add_executable("ft232testapp0" ${SOURCE_FILES})
target_link_libraries("ft232testapp0" "${CMAKE_SOURCE_DIR}\\ftd2xx.lib")
add_custom_target(copy_dll COMMAND ${CMAKE_COMMAND}
  -E copy "${CMAKE_SOURCE_DIR}/ftd2xx.dll" ${CMAKE_BINARY_DIR}/Debug)
add_dependencies("ft232testapp0" copy_dll)

# vim:sts=2:sw=2:ts=2:et:syn=cmake:fdm=marker:ff=unix:fenc=utf8:number:cc=80
