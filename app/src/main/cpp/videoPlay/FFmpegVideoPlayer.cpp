// Created by wjw on 2019-09-10.
//

#include <android/log.h>
#include "FFmpegVideoPlayer.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}

static void (*video_call)(AVFrame *frame);

void *videoPlay(void *args) {
    FFmpegVideoPlayer *fFmpegVideoPlayer = (FFmpegVideoPlayer *) args;

    //分配一个AvFrame结构体，AVFrame结构体一般用于存储原始数据，指向解码后的原始帧；
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *rgbAvFrame = av_frame_alloc();

    AVPacket *avPacket = av_packet_alloc();

    int width = fFmpegVideoPlayer->codec->width;
    int height = fFmpegVideoPlayer->codec->height;
    LOGE_TAG("视频width::%d，，height::%d", width, height);


    int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, width, height, 1);
    //缓存区
    uint8_t *out_buffer = static_cast<uint8_t *>(av_malloc(static_cast<size_t>(size)));

    //与缓存区相关联，设置rgbAvFrame缓存区
    av_image_fill_arrays(rgbAvFrame->data, rgbAvFrame->linesize, out_buffer, AV_PIX_FMT_RGBA,
                         width, height, 1);

    LOGE_TAG("%s", "转换成rgba格式");
    //flag：SWS_BICUBIC 这个参数左尺寸缩放；
    fFmpegVideoPlayer->swsContext = sws_getContext(width, height, fFmpegVideoPlayer->codec->pix_fmt,
                                                   width, height, AV_PIX_FMT_RGBA, SWS_BICUBIC,
                                                   NULL, NULL, NULL);

    //打印内存地址
    //LOGI_TAG("codec 的内存地址 %f", *fFmpegVideoPlayer->codec);

    double last_play = 0 //上一帧的播放时间
    , play  //当前帧的播放时间
    , last_delay = 0 //上一次播放视频的两帧视频间隔时间
    , delay //两帧视频间隔时间
    , audio_lock //音频轨道 时间播放时间
    , diff //音频帧与视频帧相差时间
    , syn_threshold = 0
    , start_time //从第一帧开始的绝对时间
    , pts
    , actual_delay = 0//真正需要延迟时间
    ;


    //从第一帧开始的绝对时间
    start_time = av_gettime() / 1000000.0;

    LOGE_TAG("视频准备解码::%d", fFmpegVideoPlayer->isPlay);
    while (fFmpegVideoPlayer->isPlay) {
        fFmpegVideoPlayer->get(avPacket);
        LOGI_TAG("视频解码 %d", avPacket->stream_index);

        //avcodec_decode_video2()//已过时，使用avcodec_send_packet()和avcodec_receive_frame()替代；
        if (avcodec_send_packet(fFmpegVideoPlayer->codec, avPacket) < 0) {
            LOGI_TAG("%s", "avcodec_send_packet fail");
        }

        while (true) {
            if (avcodec_receive_frame(fFmpegVideoPlayer->codec, rgbAvFrame) < 0) {
                LOGI_TAG("%s", "avcodec_receive_packet fail");
                break;
            }

            //转换为rgb格式
            sws_scale(fFmpegVideoPlayer->swsContext, (const uint8_t *const *) avFrame->data,
                      avFrame->linesize, 0, avFrame->height, rgbAvFrame->data,
                      rgbAvFrame->linesize);

            LOGI_TAG("avFrame 宽%d,,高%d", avFrame->width, avFrame->height);
            LOGI_TAG("rgbAvFrame 宽%d,,高%d", rgbAvFrame->width, rgbAvFrame->height);

            if ((pts = av_frame_get_best_effort_timestamp(avFrame)) == AV_NOPTS_VALUE) {
                pts = 0;
            }

            play = pts * av_q2d(fFmpegVideoPlayer->time_base);
            //纠正时间
            play = fFmpegVideoPlayer->synchronize(avFrame, play);

            delay = play - last_play;
            if (delay <= 0 || delay > 1) {
                delay = last_delay;
            }
            audio_lock = fFmpegVideoPlayer->fFmpegAudioPlayer->clock;
            last_delay = delay;
            last_play = play;

            //音频与视频的时间差
            diff = fFmpegVideoPlayer->clock - audio_lock;
            //在合理范围外才会延迟、加快
            if (fabs(diff) < 10) {
                if (diff <= -syn_threshold) {
                    delay = 0;
                } else if (diff >= syn_threshold) {
                    delay = 2 * delay;
                }
            }

            start_time += delay;
            actual_delay = start_time - av_gettime() / 1000000.0;
            if (actual_delay < 0.01) {
                actual_delay = 0.01;
            }

            LOGI_TAG("actual_delay::%f", actual_delay);

            //参数微秒
            av_usleep(static_cast<unsigned int>(actual_delay * 1000000.0 + 6000));
            LOGI_TAG("%s", "视频播放");

            video_call(rgbAvFrame);
        }
    }

    LOGI_TAG("%s", "释放资源");
    av_frame_free(&avFrame);
    av_frame_free(&rgbAvFrame);
    av_packet_free(&avPacket);
    sws_freeContext(fFmpegVideoPlayer->swsContext);

    size_t queueSize = fFmpegVideoPlayer->queue.size();
    for (int i = 0; i < queueSize; ++i) {
        AVPacket *packet = fFmpegVideoPlayer->queue.front();
        av_free(packet);
        fFmpegVideoPlayer->queue.erase(fFmpegVideoPlayer->queue.begin());
    }

    LOGI_TAG("%s", "Video Exit");
    //销毁线程
    pthread_exit(0);
}

FFmpegVideoPlayer::FFmpegVideoPlayer() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

FFmpegVideoPlayer::~FFmpegVideoPlayer() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void FFmpegVideoPlayer::setAvCodecContext(AVCodecContext *avCodecContext) {
    this->codec = avCodecContext;
}

int FFmpegVideoPlayer::put(AVPacket *avPacket) {
    LOGI_TAG("%s", "video 插入队列");
    AVPacket *avPacket1 = av_packet_clone(avPacket);

    //push的时候需要锁住，有数据的时候再解锁
    pthread_mutex_lock(&mutex);
    queue.push_back(avPacket1);//将avPacket压入队列
    //压入过后再发出消息并且解锁
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 1;
}

int FFmpegVideoPlayer::get(AVPacket *avPacket) {
    LOGI_TAG("%s", "从队列取出Avpacket");
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty() && isPause) {
            //如果队列中有数据可以拿出来
            if (av_packet_ref(avPacket, queue.front())) {
                break;
            }
            //取成功了，弹出队列，销毁avPacket
            AVPacket *avPacket1 = queue.front();
            queue.erase(queue.begin());
            av_free(avPacket1);
            break;
        } else {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    LOGI_TAG("%s", "解锁");
    pthread_mutex_unlock(&mutex);
    return 0;
}

void FFmpegVideoPlayer::play() {
    LOGI_TAG("%s", "开启视频播放线程");
    LOGI_TAG("ISPAUSE AAAA %d", isPlay);
    if (isPlay == 0) {
        LOGI_TAG("%s", "ssssssssssssssssssss");
    }

    isPlay = 1;
    isPause = 1;
    //开启begin线程
    pthread_create(&playId, NULL, videoPlay, this);
}

void FFmpegVideoPlayer::stop() {
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    //因为可能卡在queue
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(playId, 0);
    LOGE_TAG("%s", "Video join pass");
    if (this->codec) {
        if (avcodec_is_open(this->codec)) {
            avcodec_close(this->codec);
        }
        avcodec_free_context(&this->codec);
        this->codec = 0;
    }
    LOGE_TAG("%s", "Video close");
}

void FFmpegVideoPlayer::pause() {
    if (isPause == 1) {
        isPause = 0;
    } else {
        isPause = 1;
        pthread_cond_signal(&cond);
    }
}

double FFmpegVideoPlayer::synchronize(AVFrame *avFrame, double play) {
    //clock是当前播放的时间位置
    if (play != 0) {
        clock = play;
    } else {
        //pst 为0，则先把pts设置为上一帧时间
        play = clock;
    }
    //可能有pts为0 则主动增加clock
    double repeat_pict = avFrame->repeat_pict;
    //使用AvCodecContext的而不是stream的
    double frame_deley = av_q2d(codec->time_base);
    //如果time_base是1，25  把1s分成25份，则fps为25；
    double fps = 1 / frame_deley;
    //pts 加上延迟 是显示时间
    double extra_delay = repeat_pict / (2 * fps);
    double delay = extra_delay + frame_deley;
    clock += delay;

    return play;
}

void FFmpegVideoPlayer::setFFmpegMusic(FFmpegAudioPlayer *fFmpegAudioPlayer) {
    this->fFmpegAudioPlayer = fFmpegAudioPlayer;
}

void FFmpegVideoPlayer::setPlayCall(void (*call)(AVFrame *)) {
    video_call = call;
}
