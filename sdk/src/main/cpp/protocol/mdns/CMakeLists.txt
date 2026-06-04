include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol_mdns_objects)
set(LOCAL_SRC_FILES
    src/mdns.cpp
    src/mdns_packet.cpp
    src/mdns_responder.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_core_eventloop
    aplay_core_network_interface
    aplay_core_pattern
    aplay_core_socket
    aplay_core_thread)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_protocol_mdns)
set(LOCAL_OBJECT_LIBRARIES
    aplay_protocol_mdns_objects)
include(${BUILD_INTERFACE_LIBRARY})
