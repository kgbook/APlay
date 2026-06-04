set(APLAY_STREAMING_MODULES
    airplay
    hls
    jitterbuffer
    raop)

foreach(APLAY_STREAMING_MODULE IN LISTS APLAY_STREAMING_MODULES)
    add_subdirectory(${APLAY_STREAMING_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_streaming)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_streaming_airplay
    aplay_streaming_hls
    aplay_streaming_jitterbuffer
    aplay_streaming_raop)
include(${BUILD_INTERFACE_LIBRARY})
