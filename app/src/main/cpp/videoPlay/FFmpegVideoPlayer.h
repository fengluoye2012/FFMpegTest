//
// Created by wjw on 2019-09-10.
//


#ifndef FFMPEGTEST_FFMPEGVIDEOPLAYER_H
#define FFMPEGTEST_FFMPEGVIDEOPLAYER_H

#include <vector>
#include "FFmpegAudioPlayer.h"
#include "iostream"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

using namespace std;
using std::vector;

#endif //FFMPEGTEST_FFMPEGVIDEOPLAYER_H

class FFmpegVideoPlayer {
public:

    FFmpegVideoPlayer();

    ~FFmpegVideoPlayer();

    void setAvCodecContext(AVCodecContext *avCodecContext);

    //入栈
    int put(AVPacket *avPacket);

    //出栈
    int get(AVPacket *avPacket);

    //播放
    void play();

    void stop();

    void pause();

    double synchronize(AVFrame *avFrame, double play);

    void setFFmpegMusic(FFmpegAudioPlayer *fFmpegAudioPlayer);

    void setPlayCall(void (*call)(AVFrame *avFrame));


public:

    int index;//流索引

    int isPlay = -1;//是否正在播放
    int isPause = -1;//是否暂停
    pthread_t playId; // 处理线程
    vector<AVPacket *> queue;//队列
    AVCodecContext *codec; //解码器上下文
    SwsContext *swsContext;
    pthread_mutex_t mutex; //同步锁
    pthread_cond_t cond; //变量条件
    FFmpegAudioPlayer *fFmpegAudioPlayer;
    AVRational time_base;
    double clock;

};