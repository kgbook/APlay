add_library(aplay_jni SHARED
    native_entry.cpp)

target_compile_features(aplay_jni PRIVATE cxx_std_17)
target_link_libraries(aplay_jni PRIVATE aplay_cpp_sdk)
set_target_properties(aplay_jni PROPERTIES
    OUTPUT_NAME aplay_jni)
