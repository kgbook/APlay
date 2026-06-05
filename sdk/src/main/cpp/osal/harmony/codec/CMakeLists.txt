set(APLAY_HARMONY_CODEC_MODULES
    acodec
    vcodec)

foreach(APLAY_HARMONY_CODEC_MODULE IN LISTS APLAY_HARMONY_CODEC_MODULES)
    add_subdirectory(${APLAY_HARMONY_CODEC_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE codec)
set(LOCAL_INTERFACE_LIBRARIES
    acodec
    vcodec)
include(${BUILD_INTERFACE_LIBRARY})
