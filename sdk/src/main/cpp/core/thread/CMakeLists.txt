find_package(Threads REQUIRED)

include(${CLEAR_VARS})
set(LOCAL_MODULE thread)
set(LOCAL_SRC_FILES
    src/thread.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    Threads::Threads)
include(${BUILD_STATIC_LIBRARY})
