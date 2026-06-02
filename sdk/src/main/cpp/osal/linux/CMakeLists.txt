set(APLAY_LINUX_OSAL_MODULES
    codec
    render)

foreach(APLAY_LINUX_OSAL_MODULE IN LISTS APLAY_LINUX_OSAL_MODULES)
    add_subdirectory(${APLAY_LINUX_OSAL_MODULE})
endforeach()
