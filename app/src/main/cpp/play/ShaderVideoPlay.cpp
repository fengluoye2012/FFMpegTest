//
// Created by wjw on 2019-10-20.
//

#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "ShaderVideoPlay.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"
#include "EGL/egl.h"

void ShaderVideoPlay::shader_play_video(JNIEnv *env, jstring inPath, jobject surface) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    LOGI_TAG("inPath：：%s", in_path);

    //1 获取原始窗口
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);

    //EGL
    //1、dispaly
    eglGetDisplay(EGL);
    //2、surface

    //3、context


}
