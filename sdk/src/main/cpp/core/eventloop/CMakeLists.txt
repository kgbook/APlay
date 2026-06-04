include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_eventloop_objects)
set(LOCAL_SRC_FILES
    src/event_loop.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_core_poll)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_eventloop)
set(LOCAL_OBJECT_LIBRARIES
    aplay_core_eventloop_objects)
include(${BUILD_INTERFACE_LIBRARY})
