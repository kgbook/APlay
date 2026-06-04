include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol_mdns_objects)
set(LOCAL_SRC_FILES
    mdns.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include(${BUILD_OBJECT_LIBRARY})

target_sources(aplay_protocol_mdns_objects
    PRIVATE
        mdns_posix.cpp)

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol_mdns)
set(LOCAL_OBJECT_LIBRARIES
    aplay_protocol_mdns_objects)
include(${BUILD_INTERFACE_LIBRARY})
