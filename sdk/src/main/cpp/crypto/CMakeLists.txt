set(APLAY_CRYPTO_MODULES
    aes
    ed25519
    fairplay)

foreach(APLAY_CRYPTO_MODULE IN LISTS APLAY_CRYPTO_MODULES)
    add_subdirectory(${APLAY_CRYPTO_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_crypto)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_crypto_aes
    aplay_crypto_ed25519
    aplay_crypto_fairplay)
include(${BUILD_INTERFACE_LIBRARY})
