if(APLAY_BUILD_HARNESS)
    add_compile_definitions(APLAY_MDNS_HARNESS=1)
    set(APLAY_MDNS_HARNESS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/scripts/harness/mdns)
endif()

include(${CLEAR_VARS})
set(LOCAL_MODULE mdns)
set(LOCAL_SRC_DIRS src)
set(LOCAL_EXPORT_C_INCLUDES include)
set(LOCAL_INTERFACE_LIBRARIES
    binary
    eventloop
    interface
    pattern
    SpdlogHelper
    socket
    thread)
include(${BUILD_OBJECT_LIBRARY})

if(APLAY_BUILD_HARNESS)
    include(${CLEAR_VARS})
    set(LOCAL_MODULE aplay_harness_mdns_replay)
    set(LOCAL_SRC_FILES harness/mdns_harness_replay_main.cpp)
    set(LOCAL_OBJECT_LIBRARIES mdns)
    set(LOCAL_C_INCLUDES src)
    set(LOCAL_RUNTIME_OUTPUT_DIRECTORY ${APLAY_MDNS_HARNESS_OUTPUT_DIR})
    include(${BUILD_EXECUTABLE})

    include(${CLEAR_VARS})
    set(LOCAL_MODULE aplay_harness_mdns_announce)
    set(LOCAL_SRC_FILES harness/mdns_harness_announce_main.cpp)
    set(LOCAL_OBJECT_LIBRARIES mdns)
    set(LOCAL_C_INCLUDES src)
    set(LOCAL_RUNTIME_OUTPUT_DIRECTORY ${APLAY_MDNS_HARNESS_OUTPUT_DIR})
    include(${BUILD_EXECUTABLE})
endif()
