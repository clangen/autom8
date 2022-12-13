set (DAEMON_SRCS
  ./main.cpp
)

add_executable(daemon ${DAEMON_SRCS})

if (${BUILD_STANDALONE} MATCHES "true")
    find_library(EVLIB NAMES libev.a ev)
else()
  find_library(EVLIB NAMES ev)
endif()

target_link_libraries(daemon ${EVLIB} autom8)
set_target_properties(daemon PROPERTIES OUTPUT_NAME "autom8d")