add_subdirectory(poll)
add_subdirectory(socket)
add_subdirectory(thread)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module/CMakeLists.txt)
    add_subdirectory(module)
endif()
