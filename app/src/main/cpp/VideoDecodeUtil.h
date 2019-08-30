//
// Created by wjw on 2019-08-29.
//


// C 语言的头文件定义  头文件是扩展名为 .h 的文件，包含了 C 函数声明和宏定义，被多个源文件中引用共享。
// 头文件的声明 https://blog.csdn.net/qq_41912125/article/details/82658046

#include <stdbool.h>
#include <jni.h>
#include <android/native_window.h>

#ifndef FFMPEGTEST_VIDEODECODEUTIL_H
#define FFMPEGTEST_VIDEODECODEUTIL_H

#endif //FFMPEGTEST_VIDEODECODEUTIL_H

static const char *TAG = "VideoDecodeUtil";

AVFormatContext *avFormatContext;
int code;

int v_stream_index;

AVCodec *avCodec;

AVCodecContext *avCodecContext;

AVCodecParameters *avCodecParameters;

int srcWidth;

int srcHeight;

AVPacket *avPacket;

AVFrame *avFrame;

AVFrame *avFrameYUV;

uint8_t *out_buffer;

struct SwsContext *sws_ctx;

FILE *fp_yuv;


ANativeWindow *aNativeWindow;
//视频缓冲区
ANativeWindow_Buffer aNativeWindow_buffer;

/**
 * 将视频转码为YUV420
 * @param input
 * @param output
 */
void videoDecode(const char *input, const char *output);

/**
 * 使用FFmpeg 播放视频
 */
void videoPlay(JNIEnv *jniEnv, const char *input, jobject surface);

/**
 * 初始化
 */
void ffmpegRegister();

/**
 * 获取对应视频的信息
 * @param input
 * @return
 */
bool getStreamInfo(const char *input);

/**
 * 获取视频对应index
 * @return
 */
bool getVideoIndex();

/**
 * 获取解码器
 * @return
 */
bool getAVCodec();

/**
 * 打开解码器
 * @return
 */
bool openAvCodec();

/**
 * 准备对去帧数据
 * @param output
 */
void prepareReadFrame(enum AVPixelFormat aVPixelFormat);


void decodeScale(const char *output);

/**
 * 循环读取帧数据
 */
void readFrame();

/**
 * 释放资源
 */
void releaseResource();


void getANativeWindow(JNIEnv *jniEnv, jobject surface);

void playReadFrame();
