package com.ffmpeg.test;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;
import android.view.Surface;

public class JNIDynamicUtils {

    private static String TAG = JNIDynamicUtils.class.getSimpleName();

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("dynamic-native-lib");
    }

    private static JNIDynamicUtils instance;
    private AudioTrack audioTrack;

    public static JNIDynamicUtils getInstance() {
        if (instance == null) {
            synchronized (JNIDynamicUtils.class) {
                if (instance == null) {
                    instance = new JNIDynamicUtils();
                }
            }
        }
        return instance;
    }

    public static native String getHelloWorldFormDynamicJNI();

    public native String convertStringFormJNI(String str);

    public native String callJavaStaticMethod();


    public void printStatic() {
        Log.e(TAG, "动态注册JNI 调用静态方法:::");
    }

    public static void print_static(String str) {
        Log.e(TAG, "静态注册：：" + str);
    }

    public native void videoDecode(String inPath, String outPath);


    public native void playVideo(String inPath, Surface surface);

    public native void mp4ToFlv(String inPath, String outPath);

    public native void mp4ToM3U8(String inPath, String outPath);

    public native void singleton();

    public native void playAudio(String inPath);

    public native void playAudioOpenSL(String inPath);

    public native void stopAuidoOpenSL();

    //这个方法  是C进行调用
    public void createTrack(int sampleRateInHz, int nb_channals) {
        Log.e(TAG, "createTrack:sampleRateInHz:" + sampleRateInHz + ",,nb_channals：：" + nb_channals);
        int channelConfig;//通道数
        if (nb_channals == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (nb_channals == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        }

        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, AudioFormat.ENCODING_PCM_16BIT);
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz, channelConfig,
                AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
        audioTrack.play();
    }

    //C传入音频数据
    public void playTrack(byte[] buffer, int length) {
        Log.e(TAG, "传入音频数据");
        if (audioTrack != null && audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
            audioTrack.write(buffer, 0, length);
        }
    }

    public native void open_shader(String  urlPath,Surface surface);
}

