include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_runtime_objects)
set(LOCAL_SRC_FILES
    runtime.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_core_runtime)
set(LOCAL_OBJECT_LIBRARIES
    aplay_core_runtime_objects)
include(${BUILD_INTERFACE_LIBRARY})
