set(APLAY_PROTOCOL_MODULES
    mdns
    rtsp)

foreach(APLAY_PROTOCOL_MODULE IN LISTS APLAY_PROTOCOL_MODULES)
    add_subdirectory(${APLAY_PROTOCOL_MODULE})
endforeach()
