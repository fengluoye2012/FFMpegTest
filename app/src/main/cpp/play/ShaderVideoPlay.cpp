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
    //1、EGLDisplay 创建并且初始化
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGI_TAG("%s", "eglGetDisplay Fail");
        return;
    }
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        return;
    }

    //2、surface
    //2.1 surface窗口配置
    EGLConfig config;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &config, 1, &configNum)) {
        return;
    }

    //创建surface
    EGLSurface eglSurface = eglCreateWindowSurface(display, config, window, 0);
    if (eglSurface == EGL_NO_SURFACE) {
        return;
    }
    //3、context创建 关联上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };
    EGLContext eglContext = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);

    if (EGL_NO_CONTEXT == eglContext) {
        return;
    }

    eglMakeCurrent(display, eglSurface, eglSurface, eglContext);

    LOGI_TAG("%s", "EGL Success");
    
    //释放字符串
    env->ReleaseStringUTFChars(inPath, in_path);
}
