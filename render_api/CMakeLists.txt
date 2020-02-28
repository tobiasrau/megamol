#
# MegaMol™ Render API modules
# Copyright 2017, by MegaMol TEAM
# Alle Rechte vorbehalten. All rights reserved.
#
option(BUILD_RAPI "build render APIs" ON)


project(render_api)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h" "include/*.hpp" "src/*.h"  "src/*.hpp")
file(GLOB_RECURSE source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
if(USE_OPENGL)
set(build_interface_gl_dirs "${PROJECT_NAME}_gl/include" "${PROJECT_NAME}_gl/src")
  file(GLOB_RECURSE gl_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/include/*.h" "${PROJECT_NAME}_gl/include/*.hpp" "${PROJECT_NAME}_gl/src/*.h"  "${PROJECT_NAME}_gl/src/*.hpp")
  file(GLOB_RECURSE gl_source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/src/*.cpp")
endif()

# Add target
add_library(${PROJECT_NAME} STATIC ${header_files} ${source_files} ${gl_header_files} ${gl_source_files})
set_target_properties(${PROJECT_NAME}
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  OUTPUT_NAME ${PROJECT_NAME})
  
set(INCLUDE_DIRS ${build_interface_gl_dirs} "include")
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})

#target_link_libraries(${PROJECT_NAME} PRIVATE core)

add_library(abstract_render_api INTERFACE)
target_include_directories(abstract_render_api INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:include/>
)

if(WIN32)
  set(VERSION_LIBRARY "Version.lib")
endif()

# GLFW
if(USE_GLFW)
  require_external(glfw3)
  target_link_libraries(${PROJECT_NAME} PRIVATE glfw3)
endif()

if(USE_OPENGL)
  add_subdirectory(glad)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${VERSION_LIBRARY} ${CMAKE_DL_LIBS} glad_)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${VERSION_LIBRARY} ${CMAKE_DL_LIBS} vislib)
# Grouping in Visual Studio
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER base)
source_group("Header Files" FILES ${header_files} ${gl_header_files})
source_group("Source Files" FILES ${source_files} ${gl_source_files})

#  # install executable
#  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")
#  if(USE_GLFW)
#    install_external(TARGETS glfw3)
#  endif()
#  if (WIN32)
#    if(MSVC)
#      install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
#    endif()
#  endif ()


#  # Configuration of the megamol.cfg megamol.sh remoteconsole.sh files
#  set(cfg_LIB_PATH "${CMAKE_INSTALL_PREFIX}/lib")

#  set(cfg_MEGAMOLCON "${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")

  if(UNIX)
    include(GNUInstallDirs)
    set(cfg_EXTERNAL_LIB_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    set(MEGAMOL_SHELL_START_SCRIPT "megamol.sh")
    configure_file(extra/megamol.sh.in ${CMAKE_BINARY_DIR}/${MEGAMOL_SHELL_START_SCRIPT} @ONLY)
    install(PROGRAMS ${CMAKE_BINARY_DIR}/${MEGAMOL_SHELL_START_SCRIPT} DESTINATION "bin")
  endif()

  set(MMPLG_DIR "lib")
  if(WIN32)
    set(MMPLG_DIR "bin")
  endif()

#  configure_file(extra/megamolconfig.lua.in ${CMAKE_BINARY_DIR}/megamolconfig.lua)
#  configure_file(extra/install_megamolconfig_lua.cmake.in ${CMAKE_BINARY_DIR}/install_megamolconfig_lua.cmake @ONLY)

#  install(SCRIPT ${CMAKE_BINARY_DIR}/install_megamolconfig_lua.cmake)

