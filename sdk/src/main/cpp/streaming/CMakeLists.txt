set(APLAY_STREAMING_MODULES
    airplay
    hls
    jitterbuffer
    raop)

foreach(APLAY_STREAMING_MODULE IN LISTS APLAY_STREAMING_MODULES)
    add_subdirectory(${APLAY_STREAMING_MODULE})
endforeach()
