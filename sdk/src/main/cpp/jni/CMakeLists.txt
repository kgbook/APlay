add_library(aplay_jni SHARED
    native_entry.cpp)

if(NOT ANDROID)
    find_package(JNI REQUIRED)
    target_include_directories(aplay_jni
        PRIVATE
            ${JNI_INCLUDE_DIRS})
endif()

target_compile_features(aplay_jni PRIVATE cxx_std_17)

target_link_libraries(aplay_jni
    PRIVATE
        aplay_cpp_sdk)

set_target_properties(aplay_jni PROPERTIES
    OUTPUT_NAME aplay_jni)
