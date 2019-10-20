// Created by 风落叶 on 2019-08-18.
//

/**
 * 动态注册Native方法，相比于静态注册，更加灵活，通用；
 */
#include "DynamicNative.h"
#include <unistd.h>
#include <iostream>
#include "jni.h"
#include "string"
#include "android/log.h"
#include "thread"
#include "tgmath.h"
#include "play/FFmpegVideoPlay.h"
#include "Singleton/SingletonTest.h"
#include "Singleton/SingletonTest1.h"
#include "CPlusLogUtil.h"
#include "play/OpenSLESPlayMusic.h"
#include "videoPlay/VideoPlayer.h"
#include "play/ShaderVideoPlay.h"

//引用C的头文件
extern "C" {
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/avutil.h"
#include "VideoDecodeUtil.h"
}


using namespace std;
using std::string;

//定义全局共享的虚拟机指针；
JavaVM *javaVM;
jobject obj;
// Android log function wrappers
OpenSLESPlayMusic *openSLESPlayMusic;

//C++ 层代码
jstring native_hello(JNIEnv *env, jobject obj) {
    string str = avcodec_configuration();
    LOGE_TAG(kTAG, "C++  avcodec_configuration::%s", str.c_str());
    LOGI_TAG("%s", str.c_str());
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
        LOGI_TAG("%s：：%d", name.c_str(), i);
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
    char name[] = "线程-2";

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
        LOGI_TAG("%s：：%d", name, i);
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

    string str1 = string("线程-1");
    //3.1 thread 创建线程
    thread thread1(thread_01, str1);
    //thread1.join();  //主线程等待子线程执行完成之后，再执行；
    thread1.detach(); //子线程无需和主线程会合，各自执行；


    string str2 = string("线程-2");
    //3.2 pthread_create创建线程 https://www.jianshu.com/p/986d608a8a35
    //线程id
    pthread_t tid;

    char *cha = const_cast<char *>(str2.c_str());
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

    LOGI_TAG("Android Version - %s", "静态方法被调用");

    threadTest(env, jobj);

    return env->NewStringUTF("动态");
}

void native_videoDecode(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    const char *out_path = env->GetStringUTFChars(outPath, JNI_FALSE);

    videoDecode(in_path, out_path);

    env->ReleaseStringUTFChars(inPath, in_path);
    env->ReleaseStringUTFChars(outPath, out_path);
}

/**
 * 播放视频
 * @param env
 * @param jobj
 */
void native_ffmpeg_play(JNIEnv *env, jobject jobj, jstring inPath, jobject surface) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    //videoPlay(env, in_path, surface);
    FFmpegVIdeoPlay *fFmpegVIdeoPlay = new FFmpegVIdeoPlay();
    fFmpegVIdeoPlay->videoPlay(env, in_path, surface);
    env->ReleaseStringUTFChars(inPath, in_path);
}

/**
 * 音频播放
 * @param env
 * @param jobj
 * @param inPath
 */
void native_ffmpeg_play_audio(JNIEnv *env, jobject jobj, jstring inPath) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    audioPlay(env, jobj, in_path);
    //释放字符
    env->ReleaseStringUTFChars(inPath, in_path);
}

void native_ffmpeg_play_audio_openSL(JNIEnv *env, jobject jobj, jstring inPath) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);

    OpenSLESPlayMusic *openSLESPlayMusic = new OpenSLESPlayMusic();
    openSLESPlayMusic->playMusic(in_path);

    //释放字符
    env->ReleaseStringUTFChars(inPath, in_path);
}

void native_ffmpeg_stop_audio_openSL(JNIEnv *env, jobject jobj) {
    if (openSLESPlayMusic != NULL) {
        openSLESPlayMusic->releaseResource();
    }
}

void native_mp4ToFlv(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath) {
    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    const char *out_path = env->GetStringUTFChars(outPath, JNI_FALSE);

    mp4Toflv(env, in_path, out_path);
}

void native_mp4ToM3U8(JNIEnv *env, jobject jobj, jstring inPath, jstring outPath) {

    const char *in_path = env->GetStringUTFChars(inPath, JNI_FALSE);
    const char *out_path = env->GetStringUTFChars(outPath, JNI_FALSE);

//    decodeToM3U8(in_path, out_path);



}

void native_singleton(JNIEnv *env, jobject jobj) {

    //饿汉式
    if (SingletonTest::getInstance() == SingletonTest::getInstance()) {
        LOGE_TAG("%s", "懒汉式 单利");
    } else {
        LOGE_TAG("%s", "创建了多个实例对象");
    }
    SingletonTest::getInstance()->printStr();


    //SingletonTest1::getInstance()->printStr();
    SingletonTest1::instance->printStr();

    if (SingletonTest1::getInstance() == SingletonTest1::instance) {
        LOGE_TAG("%s", "饿汉式 单利");
    } else {
        LOGE_TAG("%s", "创建了多个实例对象");
    }

}

void native_shader_video_play(JNIEnv *env, jobject jobj, jstring inPath,jobject surface) {
    ShaderVideoPlay *shaderVideoPlay = new ShaderVideoPlay();
    shaderVideoPlay->shader_play_video(env,inPath,surface);
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
        {"getHelloWorldFormDynamicJNI", "()Ljava/lang/String;",                        (void *) native_hello},
        {"convertStringFormJNI",        "(Ljava/lang/String;)Ljava/lang/String;",      (void *) native_convert},
        {"callJavaStaticMethod",        "()Ljava/lang/String;",                        (void *) native_call_static_method},
        {"videoDecode",                 "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) native_videoDecode},
        {"playVideo",                   "(Ljava/lang/String;Landroid/view/Surface;)V", (void *) native_ffmpeg_play},
        {"mp4ToFlv",                    "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) native_mp4ToFlv},
        {"mp4ToM3U8",                   "(Ljava/lang/String;Ljava/lang/String;)V",     (void *) native_mp4ToM3U8},
        {"singleton",                   "()V",                                         (void *) native_singleton},
        {"playAudio",                   "(Ljava/lang/String;)V",                       (void *) native_ffmpeg_play_audio},
        {"playAudioOpenSL",             "(Ljava/lang/String;)V",                       (void *) native_ffmpeg_play_audio_openSL},
        {"stopAuidoOpenSL",             "()V",                                         (void *) native_ffmpeg_stop_audio_openSL}
};

int registerNatives(JNIEnv *env) {

    //通过全类名获取jclass对象
    jclass cls = env->FindClass("com/ffmpeg/test/JNIDynamicUtils");
    if (cls == NULL) {
        return JNI_FALSE;
    }

    //注册方法
    /**
     * jclass clazz:
     * const JNINativeMethod* methods: JNINativeMethod指针
     * jint nMethods://方法个数
     */
    jint j_register = env->RegisterNatives(cls, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));
    if (j_register < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

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

    //多个类的native动态注册
    if (registerNatives(env) < 1) {
        return result;
    }

    if (registerNativeVideoPlayer(env) < 1) {
        return result;
    }

    result = JNI_VERSION_1_6;
    return result;
}

