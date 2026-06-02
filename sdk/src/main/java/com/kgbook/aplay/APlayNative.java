package com.kgbook.aplay;

final class APlayNative {
    static {
        System.loadLibrary("APlaySdk");
    }

    private APlayNative() {
    }

    static String version() {
        return nativeVersion();
    }

    static String defaultReceiverName() {
        return nativeDefaultReceiverName();
    }

    private static native String nativeVersion();
    private static native String nativeDefaultReceiverName();
}
