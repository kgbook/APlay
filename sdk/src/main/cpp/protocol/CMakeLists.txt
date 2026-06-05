include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE protocol)
set(LOCAL_INTERFACE_LIBRARIES
    mdns
    rtsp)
include(${BUILD_INTERFACE_LIBRARY})
