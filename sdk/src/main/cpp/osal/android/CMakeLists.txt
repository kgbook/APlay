set(APLAY_ANDROID_OSAL_MODULES
    codec
    render
    jni)

foreach(APLAY_ANDROID_OSAL_MODULE IN LISTS APLAY_ANDROID_OSAL_MODULES)
    add_subdirectory(${APLAY_ANDROID_OSAL_MODULE})
endforeach()
# Concrete codec/render targets will be added as implementation moves beyond
# the phase-0 build boundary.
