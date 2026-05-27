#include <jni.h>

extern "C" jint JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}
