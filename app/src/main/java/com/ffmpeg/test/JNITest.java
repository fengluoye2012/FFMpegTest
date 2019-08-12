package com.ffmpeg.test;

import android.util.Log;

public class JNITest {

    private String TAG = JNITest.class.getSimpleName();

    static {
        System.loadLibrary("native-lib");
    }

    private static JNITest instance = new JNITest();

    public static JNITest getInstance() {
        return instance;
    }

    //Java 调用C++
    public native String stringFromJNI();

    //实现Java和C/C++ 字符串拼接
    public native String converStrFormJNI(String string);

    //C、C++ 调用Java方法；
    public native void callJavaMethod(String[] strings);

    public String printLog(String str) {
        Log.e(TAG, str);
        return "测试";
    }
}
