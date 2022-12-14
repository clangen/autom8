set (DAEMON_SRCS
  ./main.cpp
)

add_executable(daemon ${DAEMON_SRCS})

find_library(EVLIB NAMES ev)
target_link_libraries(daemon ${EVLIB} ${autom8_LINK_LIBRARIES} autom8)
set_target_properties(daemon PROPERTIES OUTPUT_NAME "autom8d")
target_include_directories(daemon BEFORE PRIVATE ${VENDOR_INCLUDE_DIRECTORIES})
