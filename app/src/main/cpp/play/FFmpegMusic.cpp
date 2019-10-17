// Created by wjw on 2019-09-08.
//

#include <android/log.h>
#include "FFmpegMusic.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}


void FFmpegMusic::createFFmpeg(const char *input, int *rate, int *channel) {
    av_register_all();
    avFormatContext = avformat_alloc_context();

    code = avformat_open_input(&avFormatContext, input, NULL, NULL);

    if (code < 0) {
        LOGI_TAG("%s", "无法打开音频");
        return;
    }

    code = avformat_find_stream_info(avFormatContext, NULL);
    if (code < 0) {
        LOGI_TAG("%s", "无法打开视频");
        return;
    }

    //使用av_find_best_stream()找到对应的流Index,替换遍历的方法
    audio_stream_index  =  av_find_best_stream(avFormatContext,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);

//    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
//        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
//            audio_stream_index = i;
//            break;
//        }
//    }

    if (audio_stream_index < 0) {
        LOGI_TAG("%s", "无法获取音频流下标");
        return;
    }


    avCodec = avcodec_find_decoder(
            avFormatContext->streams[audio_stream_index]->codecpar->codec_id);

    if (avCodec == NULL) {
        LOGI_TAG("%s", "无法获视频编码");
        return;
    }

    avCodecContext = avcodec_alloc_context3(avCodec);
    if (avCodecContext == NULL) {
        LOGI_TAG("%s", "无法视频编码器上下文");
        return;
    }

    code = avcodec_open2(avCodecContext, avCodec, NULL);
    if (code < 0) {
        LOGI_TAG("%s", "");
        return;
    }

    avPacket = av_packet_alloc();
    avFrame = av_frame_alloc();

    swrContext = swr_alloc();

    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_format = AV_SAMPLE_FMT_S16;
    int out_sample_rate = avCodecContext->sample_rate;

    swr_alloc_set_opts(swrContext, out_ch_layout, out_format, out_sample_rate,
                       avCodecContext->channel_layout, avCodecContext->sample_fmt,
                       avCodecContext->sample_rate, 0, NULL);

    swr_init(swrContext);

    out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    *rate = avCodecContext->sample_rate;
    *channel = avCodecContext->channels;


}


int FFmpegMusic::getPcm(void **pcm, size_t *pcm_size) {
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == audio_stream_index) {
            code = avcodec_send_packet(avCodecContext, avPacket);
            if (code < 0) {
                LOGI_TAG("%s", "avcodec_send_packet Fail");
            }

            while (1) {
                code = avcodec_receive_packet(avCodecContext, avPacket);
                if (code < 0) {
                    LOGI_TAG("%s", "avcodec_receive_packet Fail");
                    break;
                }
                //音频重采样
                swr_convert(swrContext, &out_buffer, 44100 * 2,
                            (const uint8_t **) avFrame->data, avFrame->nb_samples);

                //缓冲区大小
                int size = av_samples_get_buffer_size(NULL, out_channel_nb, avFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);

                *pcm = out_buffer;
                *pcm_size = static_cast<size_t>(size);
                break;
            }

        }

    }

    return 0;
}

void FFmpegMusic::releaseFFmpeg() {
    avformat_close_input(&avFormatContext);
    avcodec_free_context(&avCodecContext);
    // av_free_packet(avPacket);
    av_frame_free(&avFrame);
    swr_free(&swrContext);
    av_free(out_buffer);
}



