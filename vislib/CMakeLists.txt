option(BUILD_VISLIB "Build vislib" ON)

if(BUILD_VISLIB)
  project(vislib)

  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

  if(USE_OPENGL)
    add_subdirectory(glad)
    set(glad_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
  endif()

  # Build vislib
  file(GLOB_RECURSE public_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "include/*.h")
  file(GLOB_RECURSE header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.h")
  file(GLOB_RECURSE source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
  if(USE_OPENGL)
    file(GLOB_RECURSE gl_public_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/include/*.h")
    file(GLOB_RECURSE gl_header_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/src/*.h")
    file(GLOB_RECURSE gl_source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_NAME}_gl/src/*.cpp")
  endif()
  set(version_files ${CMAKE_BINARY_DIR}/version/VISlibVersionInfo.cpp)


  # Target definition
  add_library(vislib STATIC ${public_header_files} ${header_files} ${source_files} ${version_files}
                            ${gl_public_header_files} ${gl_header_files} ${gl_source_files})
  target_link_libraries(vislib PRIVATE ${CMAKE_THREAD_LIBS_INIT})
  target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                    "include" "src")
    
  if(USE_OPENGL)
    target_include_directories(${PROJECT_NAME} PUBLIC ${glad_include_dir})
    target_compile_definitions(vislib PUBLIC HAS_OPENGL)
    target_link_libraries(vislib PRIVATE glad ${OPENGL_LIBRARIES})
    if(UNIX)
      set(CURSES_NEED_NCURSES True)
      find_package(Curses REQUIRED)
      find_package(UUID REQUIRED)
      find_package(X11 REQUIRED)
      target_link_libraries(vislib PRIVATE ${X11_LIBRARIES} ${UUID_LIBRARIES} ${CURSES_LIBRARIES})
    endif()
  endif()

  # Grouping in Visual Studio
  set_target_properties(vislib PROPERTIES FOLDER base)
  source_group("Public Header Files" FILES ${public_header_files} ${gl_public_header_files})
  source_group("Header Files" FILES ${header_files} ${gl_header_files})
  source_group("Source Files" FILES ${source_files} ${gl_source_files})
  source_group("Version" FILES ${version_files})

  # Installation rules for generated files
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION "include")
  install(TARGETS vislib DESTINATION "lib" EXPORT vislib-target)
endif()
