//
// Created by 风落叶 on 2019-08-18.
//

/**
 * 动态注册Native方法，相比于静态注册，更加灵活，通用；
 */

#include "jni.h"
#include "string"

using namespace std;
using std::string;

//C++ 层代码
jstring native_hello(JNIEnv *env, jobject cls) {
    return env->NewStringUTF("Dynamic Hello World");
}

//动态注册方法，调用Java静态方法；
jstring native_call_static_method(JNIEnv *env, jobject jobj) {

    jclass cls = env->GetObjectClass(jobj);

    jmethodID methodId = env->GetMethodID(cls, "printStatic", "()V");

    env->CallVoidMethod(jobj,methodId);

    return env->NewStringUTF("动态");
}

/**
 * 动态注册，每增加一个native方法，需要在数组中增加一个JNINativeMethod结构体；
 * JNINativeMethod 是结构体
 *
 * const char* name; Java 中的函数名
 * const char* signature; Java函数签名，格式为(输入参数类型)返回值类型
 * void* fnPtr; native函数名
 */
static JNINativeMethod gMethods[] = {
        {"getHelloWorldFormDynamicJNI", "()Ljava/lang/String;", (void *) native_hello},
        {"callJavaStaticMethod",        "()Ljava/lang/String;", (void *) native_call_static_method}
};

//System.loadLibrary过程会自动调用JNI_OnLoad,在此动态注册；
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *resetved) {
    JNIEnv *env = NULL;
    jint result = JNI_FALSE;

    //获取env指针
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }
    if (env == NULL) {
        return result;
    }

    //通过全类名获取jclass对象
    jclass cls = env->FindClass("com/ffmpeg/test/JNIDynamicUtils");
    if (cls == NULL) {
        return result;
    }

    //注册方法
    /**
     * jclass clazz:
     * const JNINativeMethod* methods: JNINativeMethod指针
     * jint nMethods://方法个数
     */
    jint j_register = env->RegisterNatives(cls, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
    if (j_register < 0) {
        return result;
    }

    result = JNI_VERSION_1_6;
    return result;
}

