package com.ffmpeg.test;


import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class JNIVideoPlayer implements SurfaceHolder.Callback {

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
    private SurfaceView surfaceView;

    private JNIVideoPlayer() {
    }

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

    public void setSurfaceView(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
        play(surfaceView.getHolder().getSurface());
        surfaceView.getHolder().addCallback(this);

    }

    public  void prepareJava(String path) {
        if (surfaceView == null) {
            return;
        }
        prepare(path);
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

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        play(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
