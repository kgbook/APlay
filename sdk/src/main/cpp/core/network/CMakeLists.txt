include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE network)
set(LOCAL_INTERFACE_LIBRARIES interface socket)
include(${BUILD_INTERFACE_LIBRARY})