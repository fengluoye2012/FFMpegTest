//
// Created by wjw on 2019-09-10.
//
//只加载一次
#pragma once

//如果宏没有定义，则返回真；
#ifndef FFMPEGTEST_FFMPEGMUSICPLAYER_H
//定义宏
#define FFMPEGTEST_FFMPEGMUSICPLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "iostream"
#include "vector"

using namespace std;
using std::vector;


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}

//结束语句
#endif //FFMPEGTEST_FFMPEGMUSICPLAYER_H

class FFmpegAudioPlayer {

public:
    FFmpegAudioPlayer();

    ~FFmpegAudioPlayer();

    //入栈
    int put(AVPacket *avPacket);

    //出栈
    int get(AVPacket *avPacket);

    //解码器上下文
    void setAvCodecContext(AVCodecContext *avCodecContext);

    void play();

    void pause();

    void stop();

    //创建opensl es播放器
    int createPlayer();


public:

    int index;//流索引
    int isPlay = -1; //是否正在播放
    int isPause = -1; //是否暂停
    pthread_t playId;//处理线程

    vector<AVPacket *> queue;//队列
    AVCodecContext *codec;//解码器上下文
    SwrContext *swrContext;

    uint8_t *out_buffer;
    int out_channer_nb;

    //同步锁
    pthread_mutex_t mutex;
    //条件变量
    pthread_cond_t cond;

    double clock;//从第一帧开始所需时间

    AVRational time_base;

    SLObjectItf engineObject;
    SLEngineItf engineEnfine;

    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLObjectItf outputMixObject;

    SLObjectItf bqPlayerObject;
    SLEffectSendItf bqPlayerEffectSend;
    SLVolumeItf bqPlayerVolume;

    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    void createFFmpeg(FFmpegAudioPlayer *pPlayer);
};