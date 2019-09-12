package com.ffmpeg.test;

import android.Manifest;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.File;
import java.text.SimpleDateFormat;

import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;

public class VideoActivity extends AppCompatActivity {

    JNIVideoPlayer davidPlayer;
    SurfaceView surfaceView;
    TextView mTextView, mTextCurTime;
    SeekBar mSeekBar;
    boolean isSetProgress = false;
    private static final int HIDE_CONTROL_LAYOUT = -1;
    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == HIDE_CONTROL_LAYOUT) {
                refreshControl();
            } else {
                //  mTextCurTime.setText(formatTime(msg.what));
                mSeekBar.setProgress(msg.what);
            }
            // mSeekBar.setProgress(msg.what);
        }
    };
    private RxPermissions rxPermissions;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
        surfaceView = findViewById(R.id.surface);
        davidPlayer = JNIVideoPlayer.getInstance();
        davidPlayer.setSurfaceView(surfaceView);
        mTextView = findViewById(R.id.textview);
        mSeekBar = findViewById(R.id.seekBar);
        mTextCurTime = findViewById(R.id.tvcur);
        init();
    }

    private void init() {
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                //进度改变
                mTextCurTime.setText(formatTime(seekBar.getProgress()));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                //开始拖动
                mTextCurTime.setText(formatTime(seekBar.getProgress()));
                isSetProgress = true;
                refreshControl();
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //停止拖动
                isSetProgress = false;
                davidPlayer.seekTo(seekBar.getProgress());
                refreshControl();
            }
        });
    }

    public void player(View view) {
        if (rxPermissions == null) {
            rxPermissions = new RxPermissions(VideoActivity.this);
        }
        Disposable disposable = rxPermissions
                .request(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean aBoolean) throws Exception {
                        if (aBoolean != null && aBoolean) {
                            playerVideo();
                        }
                    }
                });
    }

    private void playerVideo() {
         /* File file = new File(Environment.getExternalStorageDirectory(), "input.mp4");
                            davidPlayer.prepareJava(file.getAbsolutePath());
                            "http://9890.vod.myqcloud.com/9890_4e292f9a3dd011e6b4078980237cc3d3.f20.mp4"
                            String url = "http://dev.cdlianmeng.com/llQXenrPbCvvSiwHpr3QZtfWrKQt";*/

        String url = "/storage/emulated/0/DCIM/Camera/VID_20190828_174922.mp4";
        davidPlayer.prepareJava(url);

        // mTextView.setText(davidPlayer.getTotalTime()+"");
        if (davidPlayer.getTotalTime() != 0) {
            mTextView.setText(formatTime(davidPlayer.getTotalTime() / 1000));
            mSeekBar.setMax(davidPlayer.getTotalTime() / 1000);
            updateSeekBar();
        }
    }

    public void stop(View view) {
        davidPlayer.release();
        // Toast.makeText(MainActivity.this,davidPlayer.getTotalTime()+"",Toast.LENGTH_SHORT).show();
        //
        //  mTextView.setText(formatTime(davidPlayer.getTotalTime()/1000));
    }

    public void pause(View view) {
        davidPlayer.stop();
    }


    public void stepback(View view) {
        //快退
        davidPlayer.stepBack();
    }

    public void stepup(View view) {
        //快进
        davidPlayer.stepUp();
    }


    private String formatTime(long time) {
        SimpleDateFormat format = new SimpleDateFormat("mm:ss");
        return format.format(time);
    }

    //更新进度
    public void updateSeekBar() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    try {
                        Message message = new Message();
                        message.what = (int) davidPlayer.getCurPos() * 1000;
                        handler.sendMessage(message);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }

    private void refreshControl() {
        if (isSetProgress) {
            isSetProgress = false;
        } else {
            isSetProgress = true;
            handler.removeMessages(HIDE_CONTROL_LAYOUT);
            handler.sendEmptyMessageDelayed(HIDE_CONTROL_LAYOUT, 5000);
        }
    }
}