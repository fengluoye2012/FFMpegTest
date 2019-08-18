package com.ffmpeg.test;

import android.util.Log;

public class JNIDynamicUtils {

    private static String TAG = JNIDynamicUtils.class.getSimpleName();

    static {
        System.loadLibrary("dynamic-native-lib");
    }

    private static JNIDynamicUtils instance;

    public static JNIDynamicUtils getInstance() {
        if (instance == null) {
            synchronized (JNIDynamicUtils.class) {
                if (instance == null) {
                    instance = new JNIDynamicUtils();
                }
            }
        }
        return instance;
    }

    public static native String getHelloWorldFormDynamicJNI();

    public native String callJavaStaticMethod();

    public void printStatic() {
        Log.e(TAG, "动态注册JNI 调用静态方法:::");
    }
}

