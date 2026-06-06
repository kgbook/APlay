include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE core)
set(LOCAL_INTERFACE_LIBRARIES
    pattern
    eventloop
    network
    poll
    runtime
    thread
    module)
include(${BUILD_INTERFACE_LIBRARY})
