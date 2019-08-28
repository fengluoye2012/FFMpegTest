package com.ffmpeg.test;

public class FFmpegTest {

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("dynamic-native-lib");
    }

    public static native String ffmpegConfig();

    public static native void videoDecode(String inStr,String outStr);
}
