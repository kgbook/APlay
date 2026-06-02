add_executable(aplay_harness_smoke
    smoke_main.cpp)
target_link_libraries(aplay_harness_smoke PRIVATE aplay_linux_app)

add_executable(aplay_harness_protocol_replay
    protocol_replay.cpp)

add_executable(aplay_harness_rtp_replay
    rtp_replay.cpp)

add_executable(aplay_harness_crypto_vectors
    crypto_vectors.cpp)

add_test(NAME aplay_help COMMAND aplay --help)
add_test(NAME aplay_smoke COMMAND aplay_harness_smoke)
add_test(NAME protocol_fixture
    COMMAND aplay_harness_protocol_replay ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/rtsp_options.txt)
add_test(NAME rtp_fixture
    COMMAND aplay_harness_rtp_replay ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/rtp_audio.csv)
add_test(NAME crypto_vectors COMMAND aplay_harness_crypto_vectors)
