include(${CLEAR_VARS})
set(LOCAL_MODULE interface)
set(LOCAL_SRC_FILES
    src/network_interface.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_STATIC_LIBRARY})
