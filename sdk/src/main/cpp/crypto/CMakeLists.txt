include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE crypto)
set(LOCAL_INTERFACE_LIBRARIES
    aes
    ed25519
    fairplay)
include(${BUILD_INTERFACE_LIBRARY})
