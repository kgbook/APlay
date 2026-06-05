include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_protocol_mdns
    aplay_protocol_rtsp)
include(${BUILD_INTERFACE_LIBRARY})
