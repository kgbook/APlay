include(${ALL_CMAKELISTS_UNDER})

include(${CLEAR_VARS})
set(LOCAL_MODULE core)
set(LOCAL_INTERFACE_LIBRARIES
    pattern
    eventloop
    interface
    poll
    runtime
    socket
    thread
    module)
include(${BUILD_INTERFACE_LIBRARY})
