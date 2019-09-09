//
// Created by wjw on 2019-09-08.
//
extern "C" {
#include <libavformat/avformat.h>
}

#ifndef FFMPEGTEST_FFMPEGMUSIC_H
#define FFMPEGTEST_FFMPEGMUSIC_H

//声明全局变量
#pragma once   //防止重复加载

#endif //FFMPEGTEST_FFMPEGMUSIC_H



class FFmpegMusic {

public:
    AVFormatContext *avFormatContext;
    int code;
    int audio_stream_index;
    AVCodec *avCodec;
    AVCodecContext *avCodecContext;
    AVPacket *avPacket;
    AVFrame *avFrame;
    struct SwrContext *swrContext;
    uint8_t *out_buffer;
    int out_channel_nb;

    void createFFmpeg(const char *input, int *rate, int *channel);

    int getPcm(void **pcm, size_t *pcm_size);

    void releaseFFmpeg();
};