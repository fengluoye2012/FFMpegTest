package com.ffmpeg.test;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoView extends SurfaceView {

    private SurfaceHolder holder;

    public VideoView(Context context) {
        this(context, null);
    }

    public VideoView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);

        holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
    }


    public void play(final String input) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                JNIDynamicUtils.getInstance().playVideo(input, holder.getSurface());
            }
        }).start();
    }
}
