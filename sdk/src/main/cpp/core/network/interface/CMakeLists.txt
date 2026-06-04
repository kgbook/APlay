include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_network_interface_objects)
set(LOCAL_SRC_FILES
    src/network_interface.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_network_interface)
set(LOCAL_OBJECT_LIBRARIES
    aplay_core_network_interface_objects)
include(${BUILD_INTERFACE_LIBRARY})
