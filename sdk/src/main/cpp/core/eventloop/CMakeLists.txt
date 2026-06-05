include(${CLEAR_VARS})
set(LOCAL_MODULE eventloop)
set(LOCAL_SRC_FILES
    src/event_loop.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    poll)
include(${BUILD_STATIC_LIBRARY})
