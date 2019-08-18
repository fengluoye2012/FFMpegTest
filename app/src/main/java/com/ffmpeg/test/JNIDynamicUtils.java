package com.ffmpeg.test;

public class JNIDynamicUtils {
    public static native String getHelloWorldFormDynamicJNI();

    static {
        System.loadLibrary("dynamic-native-lib");
    }

}
