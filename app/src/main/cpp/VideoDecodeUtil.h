//
// Created by wjw on 2019-08-29.
//


// C 语言的头文件定义  头文件是扩展名为 .h 的文件，包含了 C 函数声明和宏定义，被多个源文件中引用共享。
// 头文件的声明 https://blog.csdn.net/qq_41912125/article/details/82658046

#include <stdbool.h>
#include <jni.h>
#include <android/native_window.h>
#include <libswresample/swresample.h>

#ifndef FFMPEGTEST_VIDEODECODEUTIL_H
#define FFMPEGTEST_VIDEODECODEUTIL_H

#endif //FFMPEGTEST_VIDEODECODEUTIL_H

static const char *TAG = "VideoDecodeUtil";

AVFormatContext *avFormatContext;
int code;

//对应视频流、音频流的下标；
int target_stream_index;

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

AVFormatContext *outAvFormatContext;

struct SwrContext *swrContext;

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
 * 使用FFmpeg 配合 AudioTrack 播放音频
 * @param jniEnv
 * @param input
 */
void audioPlay(JNIEnv *jniEnv, jobject jobj, const char *input);

bool getAudioIndex();

void audioPrepareReadFrame();

void readAudioFrame(JNIEnv *jniEnv, jobject jobj);

/**
 * FFmpeg 配合 OpenGL ES 播放音频
 * @param jniEnv
 * @param jobj
 * @param input
 */
void audioPlayOpenSL(JNIEnv *jniEnv, jobject jobj, const char *input);

void createEngine();



/**
 * 将mp4 格式转换为flv  https://www.jianshu.com/p/40e55897e9a7
 * @param jniEnv
 * @param input
 * @param output
 */
void mp4Toflv(JNIEnv *jniEnv, const char *input, const char *output);

void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag);

void mp4ToM3U8(JNIEnv *jniEnv, const char *input, const char *output);

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
