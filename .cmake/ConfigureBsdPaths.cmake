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
endif()