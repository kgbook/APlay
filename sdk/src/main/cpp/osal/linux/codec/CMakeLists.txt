set(APLAY_LINUX_CODEC_MODULES
    acodec
    vcodec)

foreach(APLAY_LINUX_CODEC_MODULE IN LISTS APLAY_LINUX_CODEC_MODULES)
    add_subdirectory(${APLAY_LINUX_CODEC_MODULE})
endforeach()
