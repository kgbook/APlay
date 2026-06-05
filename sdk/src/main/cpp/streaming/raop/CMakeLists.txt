include(${CLEAR_VARS})
set(LOCAL_MODULE raop)
set(LOCAL_SRC_FILES
    src/raop.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/SpdlogHelper/helper)
set(LOCAL_INTERFACE_LIBRARIES
    mdns)
include(${BUILD_STATIC_LIBRARY})
