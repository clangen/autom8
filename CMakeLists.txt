#cmake -DCMAKE_BUILD_TYPE=Release .
#cmake -DCMAKE_BUILD_TYPE=Debug .

cmake_minimum_required(VERSION 3.1)

project(cursespp_sample)

set (cursespp_sample_VERSION_MAJOR 0)
set (cursespp_sample_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

if (${LINK_STATICALLY} MATCHES "true")
  set(Boost_USE_STATIC_LIBS ON)
endif()

set (BOOST_LIBS system filesystem)
find_package(Boost 1.55.0 REQUIRED ${BOOST_LIBS})
add_definitions (-DHAVE_BOOST -D_FILE_OFFSET_BITS=64 -DSIGSLOT_USE_POSIX_THREADS)
set (BOOST_LINK_LIBS ${Boost_LIBRARIES})

set (EXECUTABLE_OUTPUT_PATH ${cursespp_sample_SOURCE_DIR}/__output)

include_directories (
  "${cursespp_sample_SOURCE_DIR}/src"
  "${cursespp_sample_SOURCE_DIR}/src/cursespp/f8n/src"
  "${cursespp_sample_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/include"
  "${cursespp_sample_SOURCE_DIR}/src/cursespp/src"
)

# "/usr/local" doesn't seem to be included by default on macOS 10.12+
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  link_directories ("/usr/local/lib")
  include_directories("/usr/local/include")
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

set (cursespp_sample_SRCS
  ./src/app/main.cpp
)

add_executable(cursespp_sample ${cursespp_sample_SRCS})

add_subdirectory("${cursespp_sample_SOURCE_DIR}/src/cursespp/")

add_dependencies(cursespp_sample f8n cursespp)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(cursespp_sample ${BOOST_LINK_LIBS} ncursesw panelw f8n cursespp)
else (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(cursespp_sample ${BOOST_LINK_LIBS} curses panel)
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

# install(
#   FILES lib/libcursespp_sample.a
#   DESTINATION lib/
# )
