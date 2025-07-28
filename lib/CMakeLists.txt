# lib

include(GNUInstallDirs)

####################
set(HEADERS
    pgm/args.hpp
    pgm/args.ipp
)

####################
set(name ${PROJECT_NAME})

add_library(args INTERFACE ${HEADERS})
target_include_directories(args INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(pgm::args ALIAS args)

####################
install(TARGETS args EXPORT ${name}Targets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pgm)

install(EXPORT ${name}Targets
    FILE ${name}Targets.cmake
    NAMESPACE pgm::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${name}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${name}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${name}ConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${name}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${name}
)
