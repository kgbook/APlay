include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_crypto)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_crypto_aes
    aplay_crypto_ed25519
    aplay_crypto_fairplay)
include(${BUILD_INTERFACE_LIBRARY})
