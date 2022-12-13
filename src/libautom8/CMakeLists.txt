set(LIBAUTOM8_SOURCES
  ./autom8.cpp
  ./db/db.cpp
  ./device/device_base.cpp
  ./device/device_model.cpp
  ./device/device_system.cpp
  ./device/null_device_system.cpp
  ./device/simple_device.cpp
  ./device/x10/cm15a/cm15a_device_system.cpp
  ./device/x10/mochad/mochad_controller.cpp
  ./device/x10/mochad/mochad_device_system.cpp
  ./device/x10/x10_appliance.cpp
  ./device/x10/x10_device_factory.cpp
  ./device/x10/x10_device.cpp
  ./device/x10/x10_lamp.cpp
  ./device/x10/x10_security_sensor.cpp
  ./message/common_messages.cpp
  ./message/message.cpp
  ./message/request_handler_factory.cpp
  ./message/request_handler_registrar.cpp
  ./message/request.cpp
  ./message/requests/get_device_list.cpp
  ./message/requests/get_security_alert_count.cpp
  ./message/requests/send_device_command.cpp
  ./message/response.cpp
  ./net/client.cpp
  ./net/server.cpp
  ./net/session.cpp
  ./util/ssl_certificate.cpp
  ./util/utility.cpp
)

add_library(libautom8 SHARED ${LIBAUTOM8_SOURCES})
set_target_properties(libautom8 PROPERTIES OUTPUT_NAME "autom8")
target_link_libraries(libautom8 ${autom8_LINK_LIBRARIES})
set_target_properties(libautom8 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${autom8_project_SOURCE_DIR}/__output)
