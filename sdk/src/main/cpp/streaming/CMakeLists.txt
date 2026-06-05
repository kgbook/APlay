include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_streaming)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_streaming_airplay
    aplay_streaming_hls
    aplay_streaming_jitterbuffer
    aplay_streaming_raop)
include(${BUILD_INTERFACE_LIBRARY})
