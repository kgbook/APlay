set(APLAY_UTILS_MODULES
    poll
    socket
    thread)

foreach(APLAY_UTILS_MODULE IN LISTS APLAY_UTILS_MODULES)
    add_subdirectory(${APLAY_UTILS_MODULE})
endforeach()

set(APLAY_OPTIONAL_UTILS_MODULES
    module)

foreach(APLAY_UTILS_MODULE IN LISTS APLAY_OPTIONAL_UTILS_MODULES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${APLAY_UTILS_MODULE}/CMakeLists.txt")
        add_subdirectory(${APLAY_UTILS_MODULE})
    endif()
endforeach()
