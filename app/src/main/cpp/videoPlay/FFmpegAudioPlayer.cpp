//
// Created by wjw on 2019-09-10.
//

#include <android/log.h>
#include "FFmpegAudioPlayer.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

FFmpegAudioPlayer::FFmpegAudioPlayer() {
    this->clock = 0;
    //初始化锁
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

FFmpegAudioPlayer::~FFmpegAudioPlayer() {

    if (out_buffer != NULL) {
        free(out_buffer);
    }

    for (int i = 0; i < queue.size(); i++) {
        AVPacket *avPacket = queue.front();
        queue.erase(queue.begin());
        LOGI_TAG("销毁音频帧%d", queue.size());
        av_free(avPacket);
    }

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

int FFmpegAudioPlayer::put(AVPacket *avPacket) {
    return 0;
}

int FFmpegAudioPlayer::get(AVPacket *avPacket) {
    return 0;
}

void FFmpegAudioPlayer::setAvCodecContext(AVCodecContext *avCodecContext) {
    this->codec = avCodecContext;
    createFFmpeg(this);

}

void FFmpegAudioPlayer::play() {

}

void FFmpegAudioPlayer::pause() {

}

void FFmpegAudioPlayer::stop() {

}

int FFmpegAudioPlayer::createPlayer() {
    return 0;
}

void FFmpegAudioPlayer::createFFmpeg(FFmpegAudioPlayer *pPlayer) {

}
