set(APLAY_CORE_MODULES
    eventloop
    network/interface
    pattern
    poll
    socket
    thread)

foreach(APLAY_CORE_MODULE IN LISTS APLAY_CORE_MODULES)
    add_subdirectory(${APLAY_CORE_MODULE})
endforeach()

set(APLAY_OPTIONAL_CORE_MODULES
    module)

foreach(APLAY_CORE_MODULE IN LISTS APLAY_OPTIONAL_CORE_MODULES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${APLAY_CORE_MODULE}/CMakeLists.txt")
        add_subdirectory(${APLAY_CORE_MODULE})
    endif()
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_core_pattern
    aplay_core_eventloop
    aplay_core_network_interface
    aplay_core_poll
    aplay_core_socket
    aplay_core_thread
    aplay_core_module)
include(${BUILD_INTERFACE_LIBRARY})
