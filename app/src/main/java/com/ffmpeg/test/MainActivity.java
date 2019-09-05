package com.ffmpeg.test;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.

    private String TAG = MainActivity.class.getSimpleName();
    private TextView textView;
    private TextView convertStr;
    private VideoView videoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        textView = findViewById(R.id.sample_text);
        convertStr = findViewById(R.id.convert_str);
        videoView = findViewById(R.id.videoView);

        textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String inPath = "/storage/emulated/0/DCIM/Camera/VID_20190828_174922.mp4";
                String outPath = "/storage/emulated/0/DCIM/Camera/output_1280x720_yuv420p.yuv";

                String outFlvPath = "/storage/emulated/0/DCIM/Camera/flv_type.flv";

                //FFmpegTest.videoDecode(inPath, outPath);
                //JNIDynamicUtils.getInstance().videoDecode(inPath, outPath);
                //videoView.play(outFlvPath);

                //JNIDynamicUtils.getInstance().mp4ToFlv(inPath, outFlvPath);
                JNIDynamicUtils.getInstance().singleton();
            }
        });

        jniFormCPlus();

        jniFormC();
    }

    private void jniFormC() {
        textView.setText(FFmpegTest.ffmpegConfig());
    }

    private void jniFormCPlus() {
        textView.setText(JNIDynamicUtils.getHelloWorldFormDynamicJNI());
        convertStr.setText(JNIDynamicUtils.getInstance().convertStringFormJNI("Hello"));
        Log.e(TAG, "动态：：" + JNIDynamicUtils.getInstance().callJavaStaticMethod());
    }


}
