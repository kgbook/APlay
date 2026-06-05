include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE harmony)
set(LOCAL_INTERFACE_LIBRARIES
    codec
    render)
include(${BUILD_INTERFACE_LIBRARY})
