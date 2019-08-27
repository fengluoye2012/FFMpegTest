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
#include "tgmath.h"

extern "C"
{
#include "include/libavcodec/avcodec.h"
}


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
    string str = avcodec_configuration();
    return env->NewStringUTF(str.c_str());
//    return env->NewStringUTF("Dynamic Hello World");
}

jstring native_convert(JNIEnv *env, jobject instance, jstring str) {
    //将jstring 类型转换为 char* 类型  0 代表false,JNI_FALSE;s
    const char *string_char = env->GetStringUTFChars(str, JNI_FALSE);

    //1)实现字符串拼接
    //将char* 转成字符串；
    //string str_name = string(string_char);
    string str_wel = string("欢迎回家：：");
    //str_wel += string_char;
    str_wel.append(string_char);

    //2)实现字符串拼接
    //字符串申明；
    char name[] = "欢迎回家呼呼：：";
    //在使用strcat拼接字符串时，将两个字符串拼接起来了，但是这导致内存操作错误，因为strcat只是将src加到了dest的后面，但是dest没有多余的容量来容纳这些数据；
    //char *after = strcat(name, welcome);

    //先申请足够容量的空间；
    char *after = static_cast<char *>(malloc(strlen(name) + strlen(string_char)));
    //将name内容拷贝到after;
    strcpy(after, name);
    //拼接字符串；
    strcat(after, string_char);

    env->ReleaseStringUTFChars(str, string_char);

//    return env->NewStringUTF(after);

    //将string 和 char* 相互转换;
    const char *after_name = str_wel.c_str();
    return env->NewStringUTF(after_name);

}

void callJavaMethod(JNIEnv *env, jobject obj, jmethodID methodId) {
    env->CallVoidMethod(obj, methodId);
}

//目前子线程报 Fatal signal 5 (SIGTRAP), code 1 (TRAP_BRKPT)
void *thread_01(const string name) {

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


//    javaVM->DetachCurrentThread();
    //删除全局引用；
    //jniEnv->DeleteGlobalRef(obj);

    return nullptr;
}

void *thread_callback(void *cha) {
    char *name = (char *) cha;

    //记录从jvm中申请JNIEnv*的状态
    int status;
    //用于标记线程的状态，用于开启，释放
    bool isAttached = false;
    JNIEnv *jniEnv = NULL;

    //获取当前状态，查看是否已经拥有过JNIEnv指针
    status = javaVM->GetEnv((void **) &jniEnv, JNI_VERSION_1_6);

    if (status < 0) {
        if (javaVM->AttachCurrentThread(&jniEnv, NULL) != JNI_OK) {
            return NULL;
        }
        isAttached = true;
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
        LOGI("%s：：%d", name, i);
        callJavaMethod(jniEnv, obj, methodId);
        sleep(1);//秒数
        //usleep(1000);//毫秒数
    }


    //删除全局引用；
    //jniEnv->DeleteGlobalRef(obj);
    if (isAttached) {
        javaVM->DetachCurrentThread();
    }

    return nullptr;
}

void threadTest(JNIEnv *env, jobject jobj) {
    //3)使用线程 并且给线程传参,通过pthread_create 启动的线程可以附加JNI AttachCurrentThread 或 AttachCurrentThreadAsDaemon 函数。
    // 在附加之前，线程不包含任何 JNIEnv，也无法调用 JNI。
    //JNIEnv 不能跨线程传递

    string str = string("线程-2");
    //3.1 thread 创建线程
    thread thread1(thread_01, str);
    //thread1.join();  //主线程等待子线程执行完成之后，再执行；
    thread1.detach(); //子线程无需和主线程会合，各自执行；


    //3.2 pthread_create创建线程 https://www.jianshu.com/p/986d608a8a35
    //线程id
    pthread_t tid;

    char *cha = const_cast<char *>(str.c_str());
    /*
     * pthread_t *thread：表示创建的线程的id号
     * const pthread_attr_t *attr：表示线程的属性设置
     * void *(*start_routine) (void *)：很明显这是一个函数指针，表示线程创建后回调执行的函数
     * void *arg：这个参数很简单，表示的就是函数指针回调执行函数的参数
     */
    pthread_create(&tid, NULL, thread_callback, cha);


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

    //threadTest(env, jobj);

//    GetTicks();


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
        {"convertStringFormJNI", "(Ljava/lang/String;)Ljava/lang/String;", (void *) native_convert},
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

