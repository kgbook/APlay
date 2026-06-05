include(${CLEAR_VARS})
set(LOCAL_MODULE poll)
set(LOCAL_SRC_FILES
    src/epoll_poller_impl.cpp
    src/poll_poller_impl.cpp
    src/poller.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
include(${BUILD_STATIC_LIBRARY})
