set(APLAY_LOG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdk/src/main/cpp/third-party/SpdlogHelper/helper)

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_example_mdns_replay)
set(LOCAL_SRC_FILES
    mdns_replay.cpp)
set(LOCAL_C_INCLUDES
    ${APLAY_LOG_INCLUDE_DIR})
set(LOCAL_INTERFACE_LIBRARIES
    aplay_protocol
    aplay_streaming_airplay
    aplay_streaming_raop)
include(${BUILD_EXECUTABLE})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_example_mdns_announce)
set(LOCAL_SRC_FILES
    mdns_announce.cpp)
set(LOCAL_C_INCLUDES
    ${APLAY_LOG_INCLUDE_DIR})
set(LOCAL_INTERFACE_LIBRARIES
    aplay_protocol
    aplay_streaming_airplay
    aplay_streaming_raop)
include(${BUILD_EXECUTABLE})
