include(${ALL_CMAKELISTS_UNDER})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_core_pattern
    aplay_core_eventloop
    aplay_core_network_interface
    aplay_core_poll
    aplay_core_runtime
    aplay_core_socket
    aplay_core_thread
    aplay_core_module)
include(${BUILD_INTERFACE_LIBRARY})
