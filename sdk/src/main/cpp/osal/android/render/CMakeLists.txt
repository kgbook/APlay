include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE render)
set(LOCAL_INTERFACE_LIBRARIES
    arender
    vrender)
include(${BUILD_INTERFACE_LIBRARY})
