set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -O2")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIC -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fPIC -O2")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (
  (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) OR
  (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
)
  message(STATUS "[build] detected old compiler, manually adding -lstdc++fs")
  set(autom8_LINK_LIBRARIES ${autom8_LINK_LIBRARIES} stdc++fs)
endif()

if (EXISTS "/etc/arch-release" OR EXISTS "/etc/manjaro-release" OR NO_NCURSESW)
  add_definitions (-DNO_NCURSESW)
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions (-DNO_NCURSESW)
endif()

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
  message(STATUS "[ccache] ccache enabled!")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)