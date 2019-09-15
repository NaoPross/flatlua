cmake_minimum_required(VERSION 3.10)
project(
    flatlua
    VERSION 0.1 
    LANGUAGES CXX)

include(FindLua)
include(GNUInstallDirs) # for install
  
find_package(Flatland 0.1 REQUIRED)
find_package(sol2 3.0 REQUIRED)
find_package(Lua REQUIRED)

add_library(flatlua #SHARED
     ${CMAKE_CURRENT_SOURCE_DIR}/lua_state.cpp
)
 
target_compile_options(flatlua
    PRIVATE 
        $<$<CXX_COMPILER_ID:GNU>:
            -fdiagnostics-color
            -pedantic -Wall -Wextra -Wcast-qual -Wcast-align
            -Wpointer-arith -Winit-self -Wshadow -Wswitch-enum 
            -Wredundant-decls -Wfloat-equal -Wundef -Wvla 
            -Wconversion -Wstrict-aliasing
        >
        
        $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

# setting up c++ standard 17
target_compile_features(flatlua
    PRIVATE
        cxx_std_17
)

target_link_libraries(flatlua
    PRIVATE
        Flatland::Flatland
        sol2::sol2
        ${LUA_LIBRARIES}
)

target_include_directories(flatlua
    PUBLIC
        $<INSTALL_INTERFACE:include/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_compile_definitions(flatlua
    PRIVATE
        ASSETS_INSTALL_PREFIX="${CMAKE_INSTALL_DATADIR}/flatlua/"
)

# Install section

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Flatlua)

install(TARGETS flatlua
    EXPORT flatlua-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set_target_properties(flatlua PROPERTIES EXPORT_NAME Flatlua)

# public headers
install(
    FILES
        #${CMAKE_CURRENT_SOURCE_DIR}/include/flatlua/lua_scene.hpp
        #${CMAKE_CURRENT_SOURCE_DIR}/include/flatlua/lua_sprite.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/flatlua/lua_state.hpp

    DESTINATION 
        ${CMAKE_INSTALL_INCLUDEDIR}/flatlua
)

install(
    DIRECTORY 
        ${CMAKE_CURRENT_SOURCE_DIR}/include/flatlua/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

# export targets
install(EXPORT flatlua-targets
    FILE
        FlatluaTargets.cmake
    NAMESPACE
        Flatlua::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# create ConfigVersion.cmake
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FlatluaConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/FlatluaConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FlatluaConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# install config, configversion
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FlatluaConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FlatluaConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

############################
# export from the build tree
export(EXPORT flatlua-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/FlatluaTargets.cmake
    NAMESPACE Flatlua::
)

export(PACKAGE Flatlua)
