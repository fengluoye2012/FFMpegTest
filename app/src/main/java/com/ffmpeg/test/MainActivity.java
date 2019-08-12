package com.ffmpeg.test;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        TextView convertStr = findViewById(R.id.convert_str);
        tv.setText(JNITest.getInstance().stringFromJNI());

        convertStr.setText(JNITest.getInstance().converStrFormJNI("fengluoye"));
    }


}
