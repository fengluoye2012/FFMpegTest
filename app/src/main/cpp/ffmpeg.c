//
// Created by wjw on 2019-08-28.
//

# include <stdio.h>
#include <jni.h>
#include <libavformat/avformat.h>

#include "VideoDecodeUtil.h"

JNIEXPORT jstring JNICALL
Java_com_ffmpeg_test_FFmpegTest_ffmpegConfig(JNIEnv *env, jclass type) {
    const char *str = avcodec_configuration();
    return (*env)->NewStringUTF(env, str);
}


JNIEXPORT void Java_com_ffmpeg_test_FFmpegTest_videoDecode(JNIEnv *env, jclass cls, jstring input_,
                                                           jstring output_) {

    const char *input = (*env)->GetStringUTFChars(env, input_, 0);
    const char *output = (*env)->GetStringUTFChars(env, output_, 0);

    videoDecode(input, output);

    (*env)->ReleaseStringUTFChars(env, input_, input);
    (*env)->ReleaseStringUTFChars(env, output_, output);
}