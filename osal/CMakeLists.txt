if(APLAY_BUILD_LINUX_APP)
    add_subdirectory(linux)
endif()

if(APLAY_BUILD_ANDROID_NATIVE)
    add_subdirectory(android)
endif()
