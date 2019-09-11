package com.ffmpeg.test;


import android.view.Surface;

public class JNIVideoPlayer {

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("dynamic-native-lib");
    }

    private static JNIVideoPlayer instance;

    public static JNIVideoPlayer getInstance() {
        if (instance == null) {
            synchronized (JNIVideoPlayer.class) {
                if (instance == null) {
                    instance = new JNIVideoPlayer();
                }
            }
        }
        return instance;
    }

    public native void prepare(String inputStr);

    public native void play(Surface surface);

    public native int getTotalTime();

    public native double getCurPos();

    public native void seekTo(int seekPos);

    public native void stepUp();

    public native void stepBack();

    public native void stop();

    public native void release();
}
