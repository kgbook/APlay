include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE codec)
set(LOCAL_INTERFACE_LIBRARIES
    acodec
    vcodec)
include(${BUILD_INTERFACE_LIBRARY})
