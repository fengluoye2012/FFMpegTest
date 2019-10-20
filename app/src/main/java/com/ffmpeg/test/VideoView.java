package com.ffmpeg.test;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * 或者继承GLSurfaceView、SurfaceView
 */
public class VideoView extends GLSurfaceView implements SurfaceHolder.Callback,Runnable {

    private SurfaceHolder holder;

    public VideoView(Context context) {
        this(context, null);
    }

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        String networkUrl = "http://dev.cdlianmeng.com/llQXenrPbCvvSiwHpr3QZtfWrKQt";
        JNIDynamicUtils.getInstance().playVideo(networkUrl, getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(this).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

}
