//
// Created by 风落叶 on 2019-08-18.
//

/**
 * 动态注册Native方法，相比于静态注册，更加灵活，通用；
 */

#include <unistd.h>
#include <iostream>
#include "jni.h"
#include "string"
#include "android/log.h"
#include "thread"
#include "pthread.h"

using namespace std;
using std::string;

//定义全局共享的虚拟机指针；
JavaVM *javaVM;
jobject obj;
// Android log function wrappers
static const char *kTAG = "DynamicNative";

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))


//C++ 层代码
jstring native_hello(JNIEnv *env, jobject obj) {
    return env->NewStringUTF("Dynamic Hello World");
}

void callJavaMethod(JNIEnv *env, jobject obj, jmethodID methodId) {
    env->CallVoidMethod(obj, methodId);
}

void *thread_01(const string& name) {

    JNIEnv *jniEnv = NULL;
    if (javaVM->AttachCurrentThread(&jniEnv, NULL) != JNI_OK) {
        return NULL;
    }

    if (obj == NULL) {
        return NULL;
    }

    jclass cls = jniEnv->GetObjectClass(obj);
    if (cls == NULL) {
        return NULL;
    }

    //1)动态注册调用Java方法
    jmethodID methodId = jniEnv->GetMethodID(cls, "printStatic", "()V");

    if (methodId == NULL) {
        return NULL;
    }

    const int count = 10;
    for (int i = 0; i < count; i++) {
        LOGI("%s：：%d", name.c_str(), i);
        callJavaMethod(jniEnv, obj, methodId);
        sleep(1);//秒数
        //usleep(1000);//毫秒数
    }

    //删除全局引用；
    //jniEnv->DeleteGlobalRef(obj);
}

//动态注册方法，调用Java静态方法；
jstring native_call_static_method(JNIEnv *env, jobject jobj) {

    //创建全局变量
    obj = env->NewGlobalRef(jobj);

    jclass cls = env->GetObjectClass(jobj);

    //1)动态注册调用Java方法
    jmethodID methodId = env->GetMethodID(cls, "printStatic", "()V");
    env->CallVoidMethod(jobj, methodId);

    //2）动态注册调用Java静态方法
    jmethodID method_id_static = env->GetStaticMethodID(cls, "print_static",
                                                        "(Ljava/lang/String;)V");

    string str = string("静态方法调用");
    //创建字符串
    jstring jstr = env->NewStringUTF(str.c_str());
    env->CallStaticVoidMethod(cls, method_id_static, jstr);

    LOGI("Android Version - %s", "静态方法被调用");

    //3)使用线程 并且给线程传参,通过pthread_create 启动的线程可以附加JNI AttachCurrentThread 或 AttachCurrentThreadAsDaemon 函数。
    // 在附加之前，线程不包含任何 JNIEnv，也无法调用 JNI。
    //JNIEnv 不能跨线程传递

    thread thread1(thread_01, string("线程-2"));
    //thread1.join();  //主线程等待子线程执行完成之后，再执行；
    thread1.detach(); //子线程无需和主线程会合，各自执行；

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

    //赋值
    javaVM = jvm;

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

