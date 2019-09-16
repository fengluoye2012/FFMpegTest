////
//// Created by wjw on 2019-09-09.
////
//
//
#ifndef FFMPEGTEST_VIDEOPLAYER_H
#define FFMPEGTEST_VIDEOPLAYER_H

#include <jni.h>
#include <android/log.h>
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

#endif //FFMPEGTEST_VIDEOPLAYER_H

void native_prepare(JNIEnv *env, jobject obj, jstring inputStr);

void native_play(JNIEnv *env, jobject obj, jobject surface);

jint native_getTotalTime(JNIEnv *env, jobject obj);

jdouble native_getCurPos(JNIEnv *env, jobject obj);

void native_seekTo(JNIEnv *env, jobject obj, jint seekPos);

void native_stepUp(JNIEnv *env, jobject obj);

void native_stepBack(JNIEnv *env, jobject obj);

void native_stop(JNIEnv *env, jobject obj);

void native_release(JNIEnv *env, jobject obj);


bool init();

void seekTo(jint i);


/**
* 动态注册，每增加一个native方法，需要在数组中增加一个JNINativeMethod结构体；
* JNINativeMethod 是结构体
*
* const char* name; Java 中的函数名
* const char* signature; Java函数签名，格式为(输入参数类型)返回值类型
* void* fnPtr; native函数名
*/
static JNINativeMethod gMethods_player[] = {
        {"prepare",      "(Ljava/lang/String;)V",     (void *) native_prepare},
        {"play",         "(Landroid/view/Surface;)V", (void *) native_play},
        {"getTotalTime", "()I",                       (void *) native_getTotalTime},
        {"getCurPos",    "()D",                       (void *) native_getCurPos},
        {"seekTo",       "(I)V",                      (void *) native_seekTo},
        {"stepUp",       "()V",                       (void *) native_stepUp},
        {"stepBack",     "()V",                       (void *) native_stepBack},
        {"stop",         "()V",                       (void *) native_stop},
        {"release",      "()V",                       (void *) native_release}
};

static int registerNativeVideoPlayer(JNIEnv *env) {
    jclass cls;
    //通过全类名获取jclass对象
    cls = env->FindClass("com/ffmpeg/test/JNIVideoPlayer");
    if (cls == NULL) {
        return JNI_FALSE;
    }

    LOGI_TAG("%s", "registerNativeVideoPlayer");

    /*
     * 注册方法
     * jclass clazz:
     * const JNINativeMethod* methods: JNINativeMethod指针
     * jint nMethods://方法个数
     */
    jint j_register = env->RegisterNatives(cls, gMethods_player, sizeof(gMethods_player) /
                                                                 sizeof(gMethods_player[0]));
    if (j_register < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}