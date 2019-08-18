package com.ffmpeg.test;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.

    private String TAG = MainActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        TextView convertStr = findViewById(R.id.convert_str);
        TextView textView = findViewById(R.id.textView);
        tv.setText(JNITest.getInstance().stringFromJNI());

        JNITest.getInstance().setJavaStaticFile();
        Log.e(TAG, "file int ::" + JNITest.getInstance().getAge());
        Log.e(TAG, "file static  name ::" + JNITest.name);
        JNITest.getInstance().callJavaMethod();

        convertStr.setText(JNITest.getInstance().converStrFormJNI("fengluoye"));
        textView.setText(JNIDynamicUtils.getHelloWorldFormDynamicJNI());

        Log.e(TAG, "动态：：" + JNIDynamicUtils.callJavaStaticMethod());
    }


}
