set(APLAY_HARMONY_OSAL_MODULES
    codec
    render
    napi)

foreach(APLAY_HARMONY_OSAL_MODULE IN LISTS APLAY_HARMONY_OSAL_MODULES)
    add_subdirectory(${APLAY_HARMONY_OSAL_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE harmony)
set(LOCAL_INTERFACE_LIBRARIES
    codec
    render)
include(${BUILD_INTERFACE_LIBRARY})
