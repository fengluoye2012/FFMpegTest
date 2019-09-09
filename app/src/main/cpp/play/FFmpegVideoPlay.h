//
// Created by wjw on 2019-09-09.
//

extern "C" {
#include <android/native_window.h>
#include <libavutil/pixfmt.h>
#include <libavformat/avformat.h>
}


#ifndef FFMPEGTEST_FFMPEGVIDEOPLAY_H
#define FFMPEGTEST_FFMPEGVIDEOPLAY_H

#endif //FFMPEGTEST_FFMPEGVIDEOPLAY_H

class FFmpegVIdeoPlay {

public:

    AVFormatContext *avFormatContext;
    int code;
    int target_stream_index;
    AVCodecParameters *avCodecParameters;
    AVCodecContext *avCodecContext;
    AVCodec *avCodec;

    int srcWidth, srcHeight;
    AVPacket *avPacket;
    AVFrame *avFrame;
    AVFrame *avFrameYUV;
    uint8_t *out_buffer;
    struct SwsContext *sws_ctx;
    ANativeWindow *aNativeWindow;
    //视频缓冲区
    ANativeWindow_Buffer aNativeWindow_buffer;

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

/*
 * 准备对去帧数据
 * @param output
 */
    void prepareReadFrame(enum AVPixelFormat aVPixelFormat);


/**
 * 释放资源
 */
    void releaseResource();


    void getANativeWindow(JNIEnv *jniEnv, jobject surface);

    void playReadFrame();
};