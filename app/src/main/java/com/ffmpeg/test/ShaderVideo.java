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
        JNIDynamicUtils.getInstance().open_shader("", getHolder().getSurface());
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        super.surfaceCreated(holder);
        new Thread(this).start();
    }

}
