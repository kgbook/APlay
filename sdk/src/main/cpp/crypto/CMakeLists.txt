set(APLAY_CRYPTO_MODULES
    aes
    ed25519
    fairplay)

foreach(APLAY_CRYPTO_MODULE IN LISTS APLAY_CRYPTO_MODULES)
    add_subdirectory(${APLAY_CRYPTO_MODULE})
endforeach()
