package com.ffmpeg.test;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.

    private String TAG = MainActivity.class.getSimpleName();
    private TextView textView;
    private TextView convertStr;
    private Activity act;
    private RxPermissions rxPermissions;
    private TextView tvPlayVideo;
    private TextView tvPlayVideoShader;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        act = this;
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        textView = findViewById(R.id.sample_text);
        convertStr = findViewById(R.id.convert_str);
        tvPlayVideo = findViewById(R.id.tv_play_video);
        tvPlayVideo = findViewById(R.id.tv_play_video);
        tvPlayVideoShader = findViewById(R.id.tv_play_video_shader);

        initListener();
        jniFormCPlus();
        jniFormC();
    }

    private void initListener() {


        textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                test();
            }
        });

        convertStr.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goVideoPlayActivity();
            }
        });

        tvPlayVideo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(act, VideoPlayActivity.class));
            }
        });

        tvPlayVideoShader.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent(act, ShaderVideoActivity.class));
            }
        });
    }

    private void goVideoPlayActivity() {
        Intent intent = new Intent(act, VideoActivity.class);
        startActivity(intent);
    }

    private void test() {
        rxPermissions = new RxPermissions(MainActivity.this);
        Disposable disposable = rxPermissions
                .request(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean aBoolean) throws Exception {
                        if (aBoolean != null && aBoolean) {
                            String inPath = "/storage/emulated/0/DCIM/Camera/VID_20190828_174922.mp4";
                            String outPath = "/storage/emulated/0/DCIM/Camera/output_1280x720_yuv420p.yuv";

                            String outFlvPath = "/storage/emulated/0/DCIM/Camera/flv_type.flv";

                            String networkUrl = "http://dev.cdlianmeng.com/llQXenrPbCvvSiwHpr3QZtfWrKQt";

                            FFmpegTest.videoDecode(inPath, outPath);

                            //JNIDynamicUtils.getInstance().videoDecode(inPath, outPath);
                            //videoView.play(networkUrl);

                            //JNIDynamicUtils.getInstance().mp4ToFlv(inPath, outFlvPath);
                            //JNIDynamicUtils.getInstance().singleton();

                            //String inPathAudio = "/storage/emulated/0/netease/cloudmusic/Music/千陵安浅 - 归去来兮（原调版）（Cover：叶炫清）.mp3";
                            //JNIDynamicUtils.getInstance().playAudio(inPathAudio);
                        } else {
                            Toast.makeText(act, "请打开权限", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
    }

    private void jniFormC() {
        textView.setText(FFmpegTest.ffmpegConfig());
    }

    private void jniFormCPlus() {
        textView.setText(JNIDynamicUtils.getHelloWorldFormDynamicJNI());
        convertStr.setText(JNIDynamicUtils.getInstance().convertStringFormJNI("Hello"));
        //Log.e(TAG, "动态：：" + JNIDynamicUtils.getInstance().callJavaStaticMethod());
    }
}
