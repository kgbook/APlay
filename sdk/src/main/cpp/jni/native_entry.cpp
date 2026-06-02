#include "aplay/sdk.h"

#include <jni.h>

extern "C" jint JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_kgbook_aplay_APlayNative_nativeVersion(JNIEnv* env, jclass) {
    return env->NewStringUTF(aplay_sdk_version());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_kgbook_aplay_APlayNative_nativeDefaultReceiverName(JNIEnv* env, jclass) {
    return env->NewStringUTF(aplay_sdk_default_receiver_name());
}
