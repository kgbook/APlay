include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_streaming_airplay_objects)
set(LOCAL_SRC_FILES
    src/airplay.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/SpdlogHelper/helper)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_protocol_mdns)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_streaming_airplay)
set(LOCAL_OBJECT_LIBRARIES
    aplay_streaming_airplay_objects)
include(${BUILD_INTERFACE_LIBRARY})
