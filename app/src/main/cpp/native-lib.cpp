#include <jni.h>
#include <string>
#include "LogUtils.h"

using namespace std;
using std::string;

/**
 * JNIEnv 是指向可用JNI函数表的接口指针;
 * jobject
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_ffmpeg_test_JNITest_stringFromJNI(JNIEnv *env, jobject instance) {

    string str = "Hello World C++";
    return (env)->NewStringUTF(str.c_str());
}


/**
 * java String和C/C++ String 拼接
 *
 * 转成相同类型进行操作；1）都转换为string类型; 2） 都转为char * 类型；通过下面操作对比，优先使用方案1；
 *
 * C++字符串处理总结（char、string）:https://blog.csdn.net/u013834525/article/details/82533935
 */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_ffmpeg_test_JNITest_converStrFormJNI(JNIEnv *env, jobject instance, jstring str) {


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

extern "C"
JNIEXPORT void JNICALL
Java_com_ffmpeg_test_JNITest_setJavaStaticFile(JNIEnv *env, jobject instance) {

    //1) 调用并给Java成员变量赋值；
    jclass cls = env->GetObjectClass(instance);
    /*
     * jclass clazz：jclass 对象
     * const char* name:变量名
     * const char* sig：变量签名
     */
    jfieldID fieldId = env->GetFieldID(cls, "age", "I");
    env->SetIntField(instance, fieldId, 10);

    //2）调用Java static 变量；
    jfieldID fileId_static = env->GetStaticFieldID(cls, "name", "Ljava/lang/String;");

    //创建jstring 对象
    jstring str = env->NewStringUTF("fengluo2012");
    env->SetStaticObjectField(cls, fileId_static, str);
}

/**
 * C++ 调用Java的方法
 * 1）C++ 主动调用Java的方法
 * 2）Java调用C++的方法，然后C++ 在调用Java的方法；
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_ffmpeg_test_JNITest_callJavaMethod(JNIEnv *env, jobject instance) {

    //1） 调用无参无返回值方法；
    //获取jclass对象；
    jclass cls = env->GetObjectClass(instance);
    //通过全类名获取jclass对象；
    //jclass cls = env->FindClass("com/ffmpeg/test/JNITest");

    /**
     * jclass clazz:
     * const char* name:方法名
     * const char* sig：方法签名
     */
    jmethodID methodId = env->GetMethodID(cls, "printLog",
                                          "()V");
    //调用无返回值方法
    env->CallVoidMethod(instance, methodId);

    //2）调用有参方法
    jmethodID methodID = env->GetMethodID(cls, "printLog",
                                          "(Ljava/lang/String;)V");
    string str = string("fengluoye");
    jstring jstr = env->NewStringUTF(str.c_str());
    env->CallVoidMethod(instance, methodID, jstr);

    //3）调用有参有返回值的方法 返回值为int
    jmethodID jmethodID1 = env->GetMethodID(cls, "printLogStr", "(Ljava/lang/String;)I");
    //调用有参有返回值的方法
    jint jint1 = env->CallIntMethod(instance, jmethodID1, jstr);
    LogUtils::logInfo("返回值：：" + jint1);

    //4）调用有参有返回值的方法,返回值是String对象；
    jmethodID jmethodID2 = env->GetMethodID(cls, "printLogS",
                                            "(Ljava/lang/String;)Ljava/lang/String;");
    jobject jobj = env->CallObjectMethod(instance, jmethodID2, jstr);
    const char *return_str = env->GetStringUTFChars(static_cast<jstring>(jobj), 0);
    LogUtils::logInfo(string("返回值：：") + string(return_str));

    //5）调用静态方法
    jmethodID static_method_id = env->GetStaticMethodID(cls, "staticPrint",
                                                        "(Ljava/lang/String;)Ljava/lang/String;");
    //string str_param = string("静态方法参数");
    jstring str_param = env->NewStringUTF("静态方法参数");
    jobject obj_static = env->CallStaticObjectMethod(cls, static_method_id, str_param);
    //类型强转，将jobject类型转换为jstring;
    const char *str_static = env->GetStringUTFChars(static_cast<jstring>(obj_static), 0);
    LogUtils::logInfo(str_static);
}