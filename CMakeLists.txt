#cmake -DCMAKE_BUILD_TYPE=Release .
#cmake -DCMAKE_BUILD_TYPE=Debug .

cmake_minimum_required(VERSION 3.1)

project(autom8_project)

set (autom8_project_VERSION_MAJOR 0)
set (autom8_project_VERSION_MINOR 99)
set (autom8_project_VERSION_PATCH 1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/.cmake)

set (EXECUTABLE_OUTPUT_PATH ${autom8_project_SOURCE_DIR}/__output)
set (LIBRARY_OUTPUT_PATH ${autom8_project_SOURCE_DIR}/__output)

include(ConfigureCompilerFlags)
include(ConfigureBsdPaths)
include(InstallFiles)

include_directories (
  "${autom8_project_SOURCE_DIR}/src"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/include"
  "${autom8_project_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/asio/asio/include"
  "${autom8_project_SOURCE_DIR}/src/cursespp/src"
  "${autom8_project_SOURCE_DIR}/src/libautom8/include"
)

link_directories(${autom8_project_SOURCE_DIR}/__output)

find_library(LIBCRYPTO NAMES crypto PATHS ${VENDOR_LINK_DIRECTORIES})
find_library(LIBSSL NAMES ssl PATHS ${VENDOR_LINK_DIRECTORIES})
set(autom8_LINK_LIBRARIES ${autom8_LINK_LIBRARIES} ${LIBCRYPTO} ${LIBSSL} f8n cursespp)

add_subdirectory("${autom8_project_SOURCE_DIR}/src/cursespp/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/libautom8/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/app/")
add_subdirectory("${autom8_project_SOURCE_DIR}/src/daemon/")

set(autom8_EXECUTABLE_DEPENDENCIES f8n cursespp libautom8)
add_dependencies(app ${autom8_EXECUTABLE_DEPENDENCIES})
add_dependencies(daemon ${autom8_EXECUTABLE_DEPENDENCIES})

add_custom_target(postbuild ALL DEPENDS app daemon)

add_custom_command(
  TARGET postbuild
  POST_BUILD
  COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/script/post-build.sh"
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BUILD_TYPE})
