#
# MegaMol™ Core
# Copyright 2017, by MegaMol Team
# Alle Rechte vorbehalten. All rights reserved.
#
option(BUILD_CORE "Build core" ON)

if(BUILD_CORE)
  project(core)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

  # Check option dependencies: First list item is always the current build option,
  # afterwards list the dependencies.
  set(DEP_LIST "${DEP_LIST};BUILD_CORE BUILD_VISLIB" CACHE INTERNAL "")

  # Add expat.
  if(WIN32)
    option(USE_INTERNAL_EXPAT "Compiles the expat provided by MegaMol" ON)
    if(USE_INTERNAL_EXPAT)
      add_subdirectory(expat)
      set(EXPAT_LIBRARIES expat)
      set(EXPAT_INCLUDE_DIRS "")
    else()
      find_package(EXPAT REQUIRED)
    endif()
  else()
    find_package(EXPAT REQUIRED)
  endif()

  # Add Lua.
  add_subdirectory(lua)

  # Add externals.
  require_external(libzmq)
  require_external(libcppzmq)
  require_external(zlib)
  require_external(libpng)
  require_external(glm)
  if(USE_OPENGL)
    require_external(glowl)
  endif()
  require_external(json)

  if(USE_OPENGL)
    file(GLOB_RECURSE gl_public_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/include/*.h" "${PROJECT_NAME}_gl/include/*.inl")
    file(GLOB_RECURSE gl_source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/src/*.cpp")
    file(GLOB_RECURSE gl_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/src/*.h")
    file(GLOB_RECURSE shader_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/Shaders/*")
    set(build_interface_gl_dirs "${PROJECT_NAME}_gl/include" "${PROJECT_NAME}_gl/src")
  endif()
  file(GLOB_RECURSE resource_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/*")
  file(GLOB_RECURSE public_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h" "include/*.inl")
  file(GLOB_RECURSE source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
  file(GLOB_RECURSE header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h")
  set(version_files ${CMAKE_BINARY_DIR}/version/MMCoreVersionInfo.cpp)

  # Target definition
  add_library(${PROJECT_NAME} SHARED ${public_header_files} ${header_files} ${shader_files} ${source_files}
                                     ${version_files} ${gl_header_files} ${gl_public_header_files} ${gl_source_files})
  target_compile_definitions(${PROJECT_NAME} PRIVATE MEGAMOLCORE_EXPORTS)
  target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> "src" "include" ${build_interface_gl_dirs}
    PUBLIC ${EXPAT_INCLUDE_DIRS})
  if(USE_OPENGL)
    target_include_directories(${PROJECT_NAME}
      PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> "${PROJECT_NAME}_gl/src" "${PROJECT_NAME}_gl/include")
  endif()

  target_link_libraries(${PROJECT_NAME} 
    PUBLIC vislib glm zlib libpng json lua ${EXPAT_LIBRARIES}
    PRIVATE libzmq libcppzmq ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} abstract_render_api input_events)
  if(USE_OPENGL)
    target_link_libraries(${PROJECT_NAME} PUBLIC glowl glad)
  endif()
                                   
  # Add libunwind.
  if(UNIX)
    option(USE_LIBUNWIND "use libunwind" OFF)
    if(USE_LIBUNWIND)
      find_package(LIBUNWIND REQUIRED)
      message(STATUS ${LIBUNWIND_LIBRARIES})
      message(STATUS ${LIBUNWIND_INCLUDE_DIR})
      if(LIBUNWIND_FOUND)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "HAS_LIBUNWIND")
        target_include_directories(${PROJECT_NAME} PUBLIC ${LIBUNWIND_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBUNWIND_LIBRARIES} )
      else()
        message(FATAL_ERROR "Can not find libunwind.")
      endif()
    endif()
  endif()

  if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DYNAMICBASE:NO")
  endif()

  if(MPI_C_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_C)
  endif()

  # Source groups in Visual Studio
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER base)
  foreach(FILE_NAME ${source_files} ${gl_source_files})
    get_filename_component(FILE_PATH ${FILE_NAME} PATH)
    string(REPLACE "/" "\\" GROUP_NAME ${FILE_PATH})
    string(REGEX REPLACE "^src" "Source Files" GROUP_NAME ${GROUP_NAME})
    source_group(${GROUP_NAME} FILES ${FILE_NAME})
  endforeach()
  foreach(FILE_NAME ${header_files} ${gl_header_files})
    get_filename_component(FILE_PATH ${FILE_NAME} PATH)
    string(REPLACE "/" "\\" GROUP_NAME ${FILE_PATH})
    string(REGEX REPLACE "^src" "Header Files" GROUP_NAME ${GROUP_NAME})
    source_group(${GROUP_NAME} FILES ${FILE_NAME})
  endforeach()
  foreach(FILE_NAME ${public_header_files} ${gl_public_header_files})
    get_filename_component(FILE_PATH ${FILE_NAME} PATH)
    string(REPLACE "/" "\\" GROUP_NAME ${FILE_PATH})
    string(REGEX REPLACE "^include\\\\mmcore" "Public Header Files" GROUP_NAME ${GROUP_NAME})
    source_group(${GROUP_NAME} FILES ${FILE_NAME})
  endforeach()
  source_group("Shaders" FILES ${shader_files})
  source_group("Resources" FILES ${resource_files})
  source_group("Version" FILES ${version_files})

  # Installation rules for generated files
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "include")
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION "share/resources")
  if(USE_OPENGL)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}_gl/Shaders/ DESTINATION "share/shaders")
  endif()

  get_target_property(zmqcpp_include libcppzmq INTERFACE_INCLUDE_DIRECTORIES)
  install(DIRECTORY ${zmqcpp_include} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libzmq FILES_MATCHING PATTERN "*.hpp")
  if(WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION "lib")
    if (MSVC)
      install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
    endif()
  else()
    install(TARGETS ${PROJECT_NAME} DESTINATION "lib" EXPORT ${PROJECT_NAME}-target)
  endif()

  add_subdirectory(remoteconsole)
endif(BUILD_CORE)
