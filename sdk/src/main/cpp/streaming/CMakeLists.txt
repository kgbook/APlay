include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE streaming)
set(LOCAL_INTERFACE_LIBRARIES
    airplay
    connection
    hls
    jitterbuffer
    raop)
include(${BUILD_INTERFACE_LIBRARY})
