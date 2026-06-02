package com.kgbook.aplay;

public final class APlaySdk {
    private APlaySdk() {
    }

    public static String version() {
        return APlayNative.version();
    }

    public static String defaultReceiverName() {
        return APlayNative.defaultReceiverName();
    }
}
