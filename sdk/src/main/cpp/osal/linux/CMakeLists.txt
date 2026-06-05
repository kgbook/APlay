set(APLAY_LINUX_OSAL_MODULES
    codec
    render)

foreach(APLAY_LINUX_OSAL_MODULE IN LISTS APLAY_LINUX_OSAL_MODULES)
    add_subdirectory(${APLAY_LINUX_OSAL_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE linux)
set(LOCAL_INTERFACE_LIBRARIES
    codec
    render)
include(${BUILD_INTERFACE_LIBRARY})
