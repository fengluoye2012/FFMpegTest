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

    FILE *fp = fopen(in_path, "rb");

    if (!fp) {
        LOGI_TAG("open file %s failed", %in_path);
        return;
    }

    //1 获取原始窗口
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);

    //////////////////////////
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

    if (EGL_TRUE != eglMakeCurrent(display, eglSurface, eglSurface, eglContext)) {
        return;
    }

    LOGI_TAG("%s", "EGL Success");

    //顶点和片元shader初始化
    //顶点shader初始化
    GLuint vsh = InitShader(vertexShader, GL_VERTEX_SHADER);
    //片元yuv420 shader初始化
    GLuint fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);

    ////////////////////

    //创建渲染程序
    GLuint program = glCreateProgram();
    if (program == 0) {
        return;
    }

    //渲染程序中加入着色器
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);

    //链接程序
    glLinkProgram(program);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        return;
    }

    glUseProgram(program);

    LOGI_TAG("glLinkProgram success");
    //////////////////////////////////

    //加入三维顶点数据 两个三角形组成正方形
    static float vers[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
    };

    GLuint apos = (GLuint) glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);

    //传递顶点
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    //加入材质坐标数据
    static float txts[] = {
            1.0f, 0.0f, //右下
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0, 1.0
    };

    GLuint atex = (GLuint) glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex, 2, GL_FLOAT, GL_FALSE, 8, txts);

    int width = 424;
    int height = 240;

    //材质纹理初始化
    //设置纹理层
    glUniform1i(glGetUniformLocation(program, "yTexture"), 0);//对于纹理第一层
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);//对于纹理第一层
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);//对于纹理第一层

    //创建opengl 纹理
    GLuint texts[3] = {0};
    //创建三个纹理
    glGenTextures(3, texts);

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0, //细节基本 0 默认
                 GL_LUMINANCE, //gpu内部格式 亮度，灰度图
                 width, height,//拉升到全凭
                 0, //边框
                 GL_LUMINANCE, //数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[1]);

    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //设置纹理的格式和大小
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    //设置纹理属性
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,           //细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width / 2, height / 2, //拉升到全屏
                 0,             //边框
                 GL_LUMINANCE,//数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    ////////////////////////////////////////
    ////纹理的修改和显示
    unsigned char *buf[3] = {0};
    buf[0] = new unsigned char[width * height];
    buf[1] = new unsigned char[width * height / 4];
    buf[2] = new unsigned char[width * height / 4];

    size_t size = static_cast<size_t>(width * height);
    for (int i = 0; i < 10000; i++) {
        if (feof(fp) == 0) {
            fread(buf[0], 1, size, fp);
            fread(buf[1], 1, size / 4, fp);
            fread(buf[2], 1, size / 4, fp);
        }
    }

    //激活第一层纹理，绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[0]);

    //激活第二层纹理，绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texts[1]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                    buf[1]);


    //激活第三层纹理，绑定到创建的opengl纹理
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, texts[2]);
    //替换纹理内容
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                    buf[2]);

    //三维绘制
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //窗口显示
    eglSwapBuffers(display, eglSurface);


    //释放字符串
    env->ReleaseStringUTFChars(inPath, in_path);
}

GLuint ShaderVideoPlay::InitShader(const char *code, GLint type) {

    //创建shader
    GLuint sh = glCreateShader(static_cast<GLenum>(type));
    if (sh == 0) {
        LOGI_TAG("glCreateShader %d failed", type);
        return 0;
    }

    //加载shader
    glShaderSource(sh,
                   1,//shader 数量
                   &code,//shader 代码
                   0);//代码长度

    //编译shader
    glCompileShader(sh);

    //获取编译情况
    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);

    if (status == 0) {
        LOGI_TAG("glCompileShader failed");
        return 0;
    }
    LOGI_TAG("glCompileShader success");
    return sh;
}
