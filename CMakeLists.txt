cmake_minimum_required(VERSION 3.10)
project(
    luaint
    VERSION 0.1 
    LANGUAGES CXX)

include(FindLua)
include(GNUInstallDirs) # for install
  
find_package(Flatland 0.1 REQUIRED)
find_package(sol2 3.0 REQUIRED)
find_package(Lua REQUIRED)

add_library(luaint SHARED
     ${CMAKE_CURRENT_SOURCE_DIR}/lua_state.cpp
)
 
target_compile_options(luaint
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
target_compile_features(demo
    PRIVATE
        cxx_std_17
)

target_link_libraries(luaint
    PRIVATE
        Flatland::Flatland
        sol2::sol2
        ${LUA_LIBRARIES}
)

target_include_directories(luaint
    PUBLIC
        $<INSTALL_INTERFACE:include/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_compile_definitions(luaint
    PRIVATE
        ASSETS_INSTALL_PREFIX="${CMAKE_INSTALL_DATADIR}/lauint/"
)

# Install section
# public headers
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/luaint/lua_scene.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/luaint/lua_sprite.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/luaint/lua_state.hpp

    DESTINATION 
        ${CMAKE_INSTALL_INCLUDEDIR}/luaint
)

# executable
install(
    TARGETS luaint
    EXPORT luaint-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# assets
install(
    DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/luaint
)
