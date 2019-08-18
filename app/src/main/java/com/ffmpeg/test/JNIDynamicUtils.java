package com.ffmpeg.test;

import android.util.Log;

public class JNIDynamicUtils {

    private static String TAG = JNIDynamicUtils.class.getSimpleName();

    static {
        System.loadLibrary("dynamic-native-lib");
    }


    public static native String getHelloWorldFormDynamicJNI();

    public static native String callJavaStaticMethod();

    public void printStatic() {
        Log.e(TAG, "动态注册JNI 调用静态方法:::");
    }
}

