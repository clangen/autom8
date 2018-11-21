#cmake -DCMAKE_BUILD_TYPE=Release .
#cmake -DCMAKE_BUILD_TYPE=Debug .

cmake_minimum_required(VERSION 3.1)

project(autom8_curses)

set (autom8_curses_VERSION_MAJOR 0)
set (autom8_curses_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

if (${LINK_STATICALLY} MATCHES "true")
  set(Boost_USE_STATIC_LIBS ON)
endif()

set (BOOST_LIBS system filesystem thread regex)
find_package(Boost 1.55.0 REQUIRED ${BOOST_LIBS})
add_definitions (-DHAVE_BOOST -D_FILE_OFFSET_BITS=64 -DSIGSLOT_USE_POSIX_THREADS)
set (BOOST_LINK_LIBS ${Boost_LIBRARIES})

set (EXECUTABLE_OUTPUT_PATH ${autom8_curses_SOURCE_DIR}/__output)

include_directories (
  "${autom8_curses_SOURCE_DIR}/src"
  "${autom8_curses_SOURCE_DIR}/src/cursespp/f8n/src"
  "${autom8_curses_SOURCE_DIR}/src/cursespp/f8n/src/3rdparty/include"
  "${autom8_curses_SOURCE_DIR}/src/cursespp/src"
  "${autom8_curses_SOURCE_DIR}/src/libautom8/include"
)

# "/usr/local" doesn't seem to be included by default on macOS 10.12+
if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  link_directories ("/usr/local/lib")
  link_directories ("/usr/local/opt/openssl/lib")
  include_directories("/usr/local/include")
  include_directories("/usr/local/opt/openssl/include")
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

if (EXISTS "/etc/arch-release" OR EXISTS "/etc/manjaro-release" OR NO_NCURSESW)
  add_definitions (-DNO_NCURSESW)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions (-DNO_NCURSESW)
endif()

set (autom8_curses_SRCS
  ./src/libautom8/net/client.cpp
  ./src/libautom8/net/session.cpp
  ./src/libautom8/net/server.cpp
  ./src/libautom8/util/ssl_certificate.cpp
  ./src/libautom8/util/utility.cpp
  ./src/libautom8/autom8.cpp
  ./src/libautom8/message/request_handler_registrar.cpp
  ./src/libautom8/message/message.cpp
  ./src/libautom8/message/request.cpp
  ./src/libautom8/message/requests/get_device_list.cpp
  ./src/libautom8/message/requests/send_device_command.cpp
  ./src/libautom8/message/requests/get_security_alert_count.cpp
  ./src/libautom8/message/response.cpp
  ./src/libautom8/message/request_handler_factory.cpp
  ./src/libautom8/message/common_messages.cpp
  ./src/libautom8/db/db.cpp
  ./src/libautom8/device/device_base.cpp
  ./src/libautom8/device/null_device_system.cpp
  ./src/libautom8/device/simple_device.cpp
  ./src/libautom8/device/x10/cm15a/cm15a_device_system.cpp
  ./src/libautom8/device/x10/mochad/mochad_device_system.cpp
  ./src/libautom8/device/x10/mochad/mochad_controller.cpp
  ./src/libautom8/device/x10/x10_device.cpp
  ./src/libautom8/device/x10/x10_security_sensor.cpp
  ./src/libautom8/device/x10/x10_lamp.cpp
  ./src/libautom8/device/x10/x10_device_factory.cpp
  ./src/libautom8/device/x10/x10_appliance.cpp
  ./src/libautom8/device/device_system.cpp
  ./src/libautom8/device/device_model.cpp
  ./src/app/main.cpp
  ./src/app/adapter/DeviceListAdapter.cpp
  ./src/app/adapter/DeviceModelAdapter.cpp
  ./src/app/layout/ClientLayout.cpp
  ./src/app/layout/ServerLayout.cpp
  ./src/app/layout/SettingsLayout.cpp
  ./src/app/layout/MainLayout.cpp
  ./src/app/overlay/DeviceEditOverlay.cpp
  ./src/app/util/Device.cpp
  ./src/app/util/Message.cpp
  ./src/app/util/Settings.cpp
)

add_executable(autom8_curses ${autom8_curses_SRCS})

add_subdirectory("${autom8_curses_SOURCE_DIR}/src/cursespp/")

add_dependencies(autom8_curses f8n cursespp)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(autom8_curses ${BOOST_LINK_LIBS} ncursesw panelw f8n cursespp ssl crypto)
else (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(autom8_curses ${BOOST_LINK_LIBS} curses panel f8n cursespp ssl crypto)
endif (CMAKE_SYSTEM_NAME MATCHES "Linux")

file(GLOB themes "src/app/data/themes/*.json")
file(COPY ${themes} DESTINATION __output/themes)

file(GLOB locales "src/app/data/locales/*.json")
file(COPY ${locales} DESTINATION __output/locales)

# install(
#   FILES lib/libautom8_curses.a
#   DESTINATION lib/
# )
