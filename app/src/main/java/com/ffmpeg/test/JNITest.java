package com.ffmpeg.test;

import android.util.Log;

public class JNITest {
    private int age;
    public static String name;

    private static String TAG = JNITest.class.getSimpleName();

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

    //修改Java静态变量的值
    public native void setJavaStaticFile();

    //C、C++ 调用Java方法；
    public native void callJavaMethod();

    public void printLog() {
        Log.e(TAG, "输出。。。。");
    }

    public void printLog(String str) {
        Log.e(TAG, str);
    }

    public int printLogStr(String str) {
        Log.e(TAG, "输出");
        return 0;
    }

    public String printLogS(String str) {
        Log.e(TAG, str);
        return "ceshi";
    }

    public int getAge() {
        return age;
    }

    public static String staticPrint(String str) {
        Log.e(TAG, "静态方法被调用：：" + str);
        return "静态方法被调用";
    }
}
