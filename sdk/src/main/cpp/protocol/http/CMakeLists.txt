include(${CLEAR_VARS})
set(LOCAL_MODULE http)
set(LOCAL_SRC_DIRS src)
set(LOCAL_EXPORT_C_INCLUDES include)
set(LOCAL_INTERFACE_LIBRARIES
    eventloop
    fairplay
    socket
    SpdlogHelper
    thread)
include(${BUILD_OBJECT_LIBRARY})
