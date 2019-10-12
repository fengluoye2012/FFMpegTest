// Created by wjw on 2019-09-10.
//

#include <android/log.h>
#include "FFmpegAudioPlayer.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
}


void *musicPlay(void *args) {
    FFmpegAudioPlayer *fFmpegAudioPlayer = (FFmpegAudioPlayer *) args;
    //创建音频播放器
    fFmpegAudioPlayer->createPlayer();
    pthread_exit(0);//退出线程
}

//获取Pcm数据
int getPcm(FFmpegAudioPlayer *pPlayer) {
    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();

    int size = 0;
    LOGI_TAG("%s", "准备解码");
    while (pPlayer->isPlay) {
        size = 0;
        pPlayer->get(avPacket);
        //时间矫正
        if (avPacket->pts != AV_NOPTS_VALUE) {
            pPlayer->clock = av_q2d(pPlayer->time_base) * avPacket->pts;
        }

        LOGI_TAG("%s", "音频解码");
        if (avcodec_send_packet(pPlayer->codec, avPacket) < 0) {
            LOGI_TAG("%s", "avcodec_send_packet fail");
        }

        while (1) {
            if (avcodec_receive_frame(pPlayer->codec, avFrame)) {
                break;
            }

            swr_convert(pPlayer->swrContext, &pPlayer->out_buffer, 44100 * 2,
                        (const uint8_t **) avFrame->data, avFrame->nb_samples);
            //缓冲区的大小
            size = av_samples_get_buffer_size(NULL, pPlayer->out_channer_nb, avFrame->nb_samples,
                                              AV_SAMPLE_FMT_S16, 1);
            goto end;
        }
    }

    end:
    av_packet_free(&avPacket);
    av_frame_free(&avFrame);

    return size;
}

//回调函数
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    //得到pcm数据
    LOGI_TAG("%s", "回调pcm数据");
    FFmpegAudioPlayer *fFmpegAudioPlayer = (FFmpegAudioPlayer *) context;
    int dataSize = getPcm(fFmpegAudioPlayer);
    if (dataSize > 0) {
        //第一帧所需要时间 采样字节/采样率
        double time = dataSize / (44100 * 2 * 2);

        fFmpegAudioPlayer->clock = time + fFmpegAudioPlayer->clock;
        LOGI_TAG("当前一帧声音时间%f,  播放时间%f", time, fFmpegAudioPlayer->clock);

        (*bq)->Enqueue(bq, fFmpegAudioPlayer->out_buffer, static_cast<SLuint32>(dataSize));
        LOGI_TAG("播放 %d", fFmpegAudioPlayer->queue.size());
    }
}


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

//将AVPacket压入队列
int FFmpegAudioPlayer::put(AVPacket *avPacket) {
    LOGI_TAG("%s", "音频 AVPacket插入队列");
    AVPacket *avPacket1 = av_packet_clone(avPacket);

    //push的时候需要锁住，有数据的时候再解锁
    pthread_mutex_lock(&mutex);
    //将avPacket亚入队列
    queue.push_back(avPacket1);
    //压入过后发出消息并且解锁
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 1;
}

//将AvPacket 弹出队列
int FFmpegAudioPlayer::get(AVPacket *avPacket) {
    LOGI_TAG("%s", "取出队列");
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty() && isPause) {
            LOGI_TAG("isPause %d", isPause);
            //如果队列中有数据可以拿出来
            if (av_packet_ref(avPacket, queue.front())) {
                break;
            }

            //取成功了，弹出队列，销毁avPacket
            AVPacket *avPacket1 = queue.front();
            queue.erase(queue.begin());
            av_packet_free(&avPacket1);
            break;
        } else {
            LOGI_TAG("%s", "音频执行wait");
            LOGI_TAG("isPause %d", isPause);
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

    return 0;
}

void FFmpegAudioPlayer::setAvCodecContext(AVCodecContext *avCodecContext) {
    this->codec = avCodecContext;
    createFFmpeg(this);

}

void FFmpegAudioPlayer::play() {
    isPause = 1;
    isPlay = 1;
    LOGE_TAG("%s", "开启音频播放线程");
    //开启begin线程
    pthread_create(&playId, NULL, musicPlay, this);
}

void FFmpegAudioPlayer::pause() {
    if (isPause == 1) {
        isPause = 0;
    } else {
        isPause = 1;
        pthread_cond_signal(&cond);
    }
}

void FFmpegAudioPlayer::stop() {
    LOGI_TAG("%s", "声音暂停");
    //因为可能卡在queue
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(playId, 0);

    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = 0;
    }

    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bqPlayerBufferQueue = 0;
        bqPlayerVolume = 0;
    }

    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineEnfine = 0;
    }

    if (swrContext) {
        swr_free(&swrContext);
    }

    if (this->codec) {
        if (avcodec_is_open(this->codec)) {
            avcodec_close(this->codec);
        }
        avcodec_free_context(&this->codec);
        this->codec = 0;
    }
    LOGE_TAG("%s", "Audio clear");
}

//使用OpenSL ES创建音频播放器
int FFmpegAudioPlayer::createPlayer() {
    LOGI_TAG("%s", "创建OpenSL ES 播放器");

    //创建播放器
    SLresult result;
    //创建引擎engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }

    //实现引擎engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }

    //获取引擎接口engineEnfine
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEnfine);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }

    //创建混音器outputMixObject
    result = (*engineEnfine)->CreateOutputMix(engineEnfine, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }

    //实现混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }

    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);

    const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }

    //===============================================
    SLDataLocator_AndroidBufferQueue androidBufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                           2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //新建一个数据源，将上述配置信息方法这个数据源中
    SLDataSource slDataSource = {&androidBufferQueue, &pcm};
    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink slDataSink = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};

    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEnfine)->CreateAudioPlayer(engineEnfine, &bqPlayerObject, &slDataSource, &slDataSink, 2,
                                       ids, req);

    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用，获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);

    //注册回调缓冲区 获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    //获取音量接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);

    //获取播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);

    return 1;
}

//初始化FFmpeg
void FFmpegAudioPlayer::createFFmpeg(FFmpegAudioPlayer *pPlayer) {
    LOGI_TAG("%s", "初始化ffmpeg");
    pPlayer->swrContext = swr_alloc();

    pPlayer->out_buffer = (uint8_t *) av_malloc(44100 * 2);

    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //输出采样率 16位
    enum AVSampleFormat out_formart = AV_SAMPLE_FMT_S16;
    //输出采样率必须和输入相同
    int out_sample_rate = pPlayer->codec->sample_rate;

    swr_alloc_set_opts(pPlayer->swrContext, out_ch_layout, out_formart, out_sample_rate,
                       pPlayer->codec->channel_layout, pPlayer->codec->sample_fmt,
                       pPlayer->codec->sample_rate, 0, NULL);

    swr_init(pPlayer->swrContext);
    //获取通道数 2
    pPlayer->out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    LOGI_TAG("通道数%d", pPlayer->out_channer_nb);

}
