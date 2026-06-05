include(${CLEAR_VARS})
set(LOCAL_MODULE runtime)
set(LOCAL_SRC_FILES
    src/sdk.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_STATIC_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE sdk_static)
set(LOCAL_SRC_FILES
    src/sdk.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LOCAL_INTERFACE_LIBRARIES
    cpp_sdk
    osal)
set(LOCAL_MODULE_FILENAME
    APlaySdk)
set(LOCAL_ARCHIVE_OUTPUT_DIRECTORY
    ${APLAY_SDK_CPP_BINARY_DIR})
include(${BUILD_STATIC_LIBRARY})

if(APLAY_BUILD_LINUX)
    include(${CLEAR_VARS})
    set(LOCAL_MODULE sdk)
    set(LOCAL_SRC_FILES
        src/sdk.cpp)
    set(LOCAL_EXPORT_C_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(LOCAL_INTERFACE_LIBRARIES
        cpp_sdk
        osal)
    set(LOCAL_MODULE_FILENAME
        APlaySdk)
    set(LOCAL_LIBRARY_OUTPUT_DIRECTORY
        ${APLAY_SDK_CPP_BINARY_DIR})
    include(${BUILD_SHARED_LIBRARY})
endif()
