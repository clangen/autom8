set (app_SRCS
  ./main.cpp
  ./adapter/DeviceListAdapter.cpp
  ./adapter/DeviceModelAdapter.cpp
  ./layout/ClientLayout.cpp
  ./layout/ConsoleLayout.cpp
  ./layout/SettingsLayout.cpp
  ./layout/MainLayout.cpp
  ./overlay/DeviceEditOverlay.cpp
  ./util/ConsoleLogger.cpp
  ./util/Device.cpp
  ./util/Message.cpp
  ./util/Settings.cpp
)

add_executable(app ${app_SRCS})
set_target_properties(app PROPERTIES OUTPUT_NAME "autom8")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(app ${autom8_LINK_LIBRARIES} ncursesw panelw autom8)
else ()
  target_link_libraries(app ${autom8_LINK_LIBRARIES} curses panel autom8)
endif ()