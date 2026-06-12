include(${CLEAR_VARS})
set(LOCAL_MODULE mdns)
set(LOCAL_SRC_DIRS src)
set(LOCAL_EXPORT_C_INCLUDES include)
set(LOCAL_INTERFACE_LIBRARIES
    binary
    eventloop
    interface
    pattern
    SpdlogHelper
    socket
    thread)
include(${BUILD_OBJECT_LIBRARY})
