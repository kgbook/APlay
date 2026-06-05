set(APLAY_LINUX_RENDER_MODULES
    arender
    vrender)

foreach(APLAY_LINUX_RENDER_MODULE IN LISTS APLAY_LINUX_RENDER_MODULES)
    add_subdirectory(${APLAY_LINUX_RENDER_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE render)
set(LOCAL_INTERFACE_LIBRARIES
    arender
    vrender)
include(${BUILD_INTERFACE_LIBRARY})
