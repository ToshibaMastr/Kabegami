# File name: CMakeLists.txt
# Author: ToshibaMastru
# Copyright (c) 2024 ToshibaMastru
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.10)

project(Kabegami VERSION 0.7)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(EXECUTABLE_NAME kabegami)


find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0)
pkg_check_modules(X11 REQUIRED x11 xrandr)


if (UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()


include_directories(include)


file(GLOB SOURCES "src/*.cpp")


add_executable(${EXECUTABLE_NAME} ${SOURCES})


target_include_directories(${EXECUTABLE_NAME} PRIVATE ${GSTREAMER_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})


target_link_libraries(${EXECUTABLE_NAME} ${GSTREAMER_LIBRARIES} ${X11_LIBRARIES})


install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()


set(INSTALL_BINARY_PATH "${CMAKE_INSTALL_PREFIX}/bin")


add_custom_target(uninstall
    COMMAND rm -f "${INSTALL_BINARY_PATH}/${EXECUTABLE_NAME}"
    COMMENT "Uninstalling ${PROJECT_NAME}..."
)


add_compile_definitions(
    PROJECT_NAME="${PROJECT_NAME}"
    EXECUTABLE_NAME="${EXECUTABLE_NAME}"
)
