include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_socket_objects)
set(LOCAL_SRC_FILES
    src/ipv4_endpoint.cpp
    src/socket_internal.cpp
    src/tcp_socket.cpp
    src/udp_socket.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_socket)
set(LOCAL_OBJECT_LIBRARIES
    aplay_core_socket_objects)
include(${BUILD_INTERFACE_LIBRARY})
