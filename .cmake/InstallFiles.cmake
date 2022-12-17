include(InstallSymlink)

if (APPLE)
  file(GLOB libraries ${autom8_project_SOURCE_DIR}/__output/*.dylib)
else ()
  file(GLOB libraries ${autom8_project_SOURCE_DIR}/__output/*.so)
endif ()

file(GLOB themes ${autom8_project_SOURCE_DIR}/__output/themes/*.json)
file(GLOB locales ${autom8_project_SOURCE_DIR}/__output/locales/*.json)
file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/)

install(FILES ${libraries} DESTINATION share/autom8/)
install(FILES ${themes} DESTINATION share/autom8/themes)
install(FILES ${locales} DESTINATION share/autom8/locales)

install(
  FILES __output/autom8
  DESTINATION share/autom8
  PERMISSIONS
    OWNER_EXECUTE OWNER_READ OWNER_WRITE
    GROUP_EXECUTE GROUP_READ GROUP_WRITE
    WORLD_EXECUTE WORLD_READ)

install(
    FILES __output/autom8d
    DESTINATION share/autom8
    PERMISSIONS
        OWNER_EXECUTE OWNER_READ OWNER_WRITE
        GROUP_EXECUTE GROUP_READ GROUP_WRITE
        WORLD_EXECUTE WORLD_READ)

install_symlink(
    ../share/autom8/autom8
    ${CMAKE_INSTALL_PREFIX}/bin/autom8)

install_symlink(
    ../share/autom8/autom8d
    ${CMAKE_INSTALL_PREFIX}/bin/autom8d)
