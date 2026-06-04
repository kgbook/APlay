set(APLAY_PROTOCOL_MODULES
    mdns
    rtsp)

foreach(APLAY_PROTOCOL_MODULE IN LISTS APLAY_PROTOCOL_MODULES)
    add_subdirectory(${APLAY_PROTOCOL_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_protocol_mdns
    aplay_protocol_rtsp)
include(${BUILD_INTERFACE_LIBRARY})
