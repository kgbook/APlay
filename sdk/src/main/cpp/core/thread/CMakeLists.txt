find_package(Threads REQUIRED)

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_thread_objects)
set(LOCAL_SRC_FILES
    src/thread.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    Threads::Threads)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_thread)
set(LOCAL_OBJECT_LIBRARIES
    aplay_core_thread_objects)
include(${BUILD_INTERFACE_LIBRARY})
