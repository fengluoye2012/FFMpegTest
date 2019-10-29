package com.ffmpeg.test;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

public class ShaderVideo extends GLSurfaceView implements Runnable, SurfaceHolder.Callback {
    public ShaderVideo(Context context) {
        super(context);
    }

    public ShaderVideo(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void run() {
        String path = "/sdcard/test.yuv";
        JNIDynamicUtils.getInstance().open_shader(path, getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(this).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }
}
