package com.ffmpeg.test;

import android.util.Log;
import android.view.Surface;

public class JNIDynamicUtils {

    private static String TAG = JNIDynamicUtils.class.getSimpleName();

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
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

    public native String convertStringFormJNI(String str);

    public native String callJavaStaticMethod();


    public void printStatic() {
        Log.e(TAG, "动态注册JNI 调用静态方法:::");
    }

    public static void print_static(String str) {
        Log.e(TAG, "静态注册：：" + str);
    }

    public native void videoDecode(String inPath, String outPath);


    public native void playVideo(String inPath, Surface surface);
}

