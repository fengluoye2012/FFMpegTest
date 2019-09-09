//
// Created by wjw on 2019-09-09.
//

#ifndef FFMPEGTEST_DYNAMICNATIVE_H
#define FFMPEGTEST_DYNAMICNATIVE_H

#endif //FFMPEGTEST_DYNAMICNATIVE_H

#include "jni.h"

jstring native_hello(JNIEnv *env, jobject obj);

jstring native_convert(JNIEnv *env, jobject instance, jstring str);

void callJavaMethod(JNIEnv *env, jobject obj, jmethodID methodId);

void threadTest(JNIEnv *env, jobject jobj);

jstring native_call_static_method(JNIEnv *env, jobject jobj);

void native_videoDecode(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath);


void native_ffmpeg_play(JNIEnv *env, jobject jobj, jstring inPath, jobject surface);

void native_ffmpeg_play_audio(JNIEnv *env, jobject jobj, jstring inPath);

void native_ffmpeg_play_audio_openSL(JNIEnv *env, jobject jobj, jstring inPath);

void native_ffmpeg_stop_audio_openSL(JNIEnv *env, jobject jobj);


void native_mp4ToFlv(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath);

void native_mp4ToM3U8(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath);

void native_singleton(JNIEnv *env, jobject jobj);

