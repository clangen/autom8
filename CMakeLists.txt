#cmake -DCMAKE_BUILD_TYPE=Release .
#cmake -DCMAKE_BUILD_TYPE=Debug .

cmake_minimum_required(VERSION 3.1)

project(autom8_project)

set (autom8_project_VERSION_MAJOR 0)
set (autom8_project_VERSION_MINOR 99)
set (autom8_project_VERSION_PATCH 1)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
  message(STATUS "[ccache] ccache enabled!")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIC -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fPIC -O2")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set (EXECUTABLE_OUTPUT_PATH ${autom8_project_SOURCE_DIR}/__output)
set (LIBRARY_OUTPUT_PATH ${autom8_project_SOURCE_DIR}/__output)

include_directories (
  "${autom8_project_SOURCE_DIR}/src"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/include"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/asio/asio/include"
  "${autom8_project_SOURCE_DIR}/src/cursespp/src"
  "${autom8_project_SOURCE_DIR}/src/libautom8/include"
)

link_directories ("/usr/local/lib")
include_directories("/usr/local/include")

link_directories(${autom8_project_SOURCE_DIR}/__output)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  execute_process(
    COMMAND brew config
    COMMAND grep -i HOMEBREW_PREFIX
    COMMAND awk "{print $2}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE BSD_PATH_PREFIX)
  list(
    APPEND
    VENDOR_LINK_DIRECTORIES
    "${BSD_PATH_PREFIX}/opt/libev/lib"
    "${BSD_PATH_PREFIX}/opt/openssl/lib"
    "${BSD_PATH_PREFIX}/opt/icu4c/lib"
    "${BSD_PATH_PREFIX}/opt/ncurses/lib")
  list(
    APPEND
    VENDOR_INCLUDE_DIRECTORIES
    "${BSD_PATH_PREFIX}/opt/libev/include"
    "${BSD_PATH_PREFIX}/opt/openssl/include"
    "${BSD_PATH_PREFIX}/opt/icu4c/include"
    "${BSD_PATH_PREFIX}/opt/ncurses/include")
endif ()

if (EXISTS "/etc/arch-release" OR EXISTS "/etc/manjaro-release" OR NO_NCURSESW)
  add_definitions (-DNO_NCURSESW)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions (-DNO_NCURSESW)
endif()

find_library(LIBCRYPTO NAMES crypto PATHS ${VENDOR_LINK_DIRECTORIES})
find_library(LIBSSL NAMES ssl PATHS ${VENDOR_LINK_DIRECTORIES})

set(autom8_LINK_LIBRARIES ${LIBCRYPTO} ${LIBSSL} f8n cursespp)
if (
  (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
)
  message(STATUS "[build] detected old compiler, manually adding -lstdc++fs")
  set(autom8_LINK_LIBRARIES ${autom8_LINK_LIBRARIES} stdc++fs)
endif()

add_subdirectory("${autom8_project_SOURCE_DIR}/src/cursespp/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/libautom8/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/app/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/daemon/")

add_dependencies(app f8n cursespp libautom8)
add_dependencies(daemon f8n cursespp libautom8)

add_custom_target(postbuild ALL DEPENDS app daemon)

add_custom_command(
  TARGET postbuild
  POST_BUILD
  COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/script/post-build.sh"
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BUILD_TYPE})
