//
// Created by wjw on 2019-09-09.
//

#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <regex>
#include "FFmpegVideoPlay.h"
#include "../CPlusLogUtil.h"
#include "../Singleton/SingletonTest.h"

extern "C" {
#include <android/native_window_jni.h>
#include "android/native_window.h"
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


static double r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0 : (double) r.num / (double) r.den;
}

/**
 * 视频播放 的方式 1、使用Surface的双缓存机制 2、使用shader
 * @param jniEnv
 * @param input
 * @param surface
 */
void FFmpegVIdeoPlay::videoPlay(JNIEnv *jniEnv, const char *input, jobject surface) {
    //1 注册所有组件
    ffmpegRegister();

    if (!getStreamInfo(input)) {
        return;
    }

    if (!getVideoIndex()) {
        return;
    }

    if (!getAVCodec()) {
        return;
    }


    if (!openAvCodec()) {
        return;
    }
    prepareReadFrame(AV_PIX_FMT_ABGR);
    getANativeWindow(jniEnv, surface);
    playReadFrame();
    releaseResource();
}


void FFmpegVIdeoPlay::ffmpegRegister() {
    av_register_all();
    //播放网络视频
    avformat_network_init();

    avcodec_register_all();
}

bool FFmpegVIdeoPlay::getStreamInfo(const char *input) {

    avFormatContext = avformat_alloc_context();

    //2 打开输入视频文件
    //打开媒体文件,成功返回0；否则为负数；
    code = avformat_open_input(&avFormatContext, input, NULL, NULL);

    if (code < 0) {
        LOGI_TAG("%s", "无法打开视频文件");
        return false;
    }

    //3 获取视频文件信息
    code = avformat_find_stream_info(avFormatContext, NULL);

    if (code < 0) {
        LOGI_TAG("%s", "无法获取视频信息");
        return false;
    }

    return true;
}


bool FFmpegVIdeoPlay::getVideoIndex() {

    //使用av_find_best_stream()找到对应的流Index,替换遍历的方法
    target_stream_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
//    for (int i = 0; i < avFormatContext->nb_streams; i++) {
//        //流的类型  为视频流;//AVMEDIA_TYPE_AUDIO 为音频流；
//        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            target_stream_index = i;
//            break;
//        }
//    }

    if (target_stream_index == -1) {
        LOGI_TAG("%s", "找不到视频流");
        return false;
    }

    LOGI_TAG("找到了视频流::%d", target_stream_index);

    return true;
}


bool FFmpegVIdeoPlay::getAVCodec() {
    //只有知道视频的编码方式，才能够根据编码方式找到解码器
    avCodecParameters = avFormatContext->streams[target_stream_index]->codecpar;

    //软解码
    //4 根据编解码上下文中的编码ID查找对应的解码
    avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    //硬解码
    //avcodec_find_decoder_by_name("h264_mediacodec");

    if (avCodec == NULL) {
        LOGI_TAG("%s", "找不到解码器");
        return false;
    }

    //初始化解码器
    //获取视频流中的编解码上下文 需要使用avcodec_free_context释放
    avCodecContext = avcodec_alloc_context3(avCodec);

    code = avcodec_parameters_to_context(avCodecContext, avCodecParameters);

    //指定多线程解码，提高解码速度
    avCodecContext->thread_count = 1;
    if (code < 0) {
        LOGI_TAG("%s", "avcodec_parameters_to_context Fail");
        return false;
    }
    return true;
}

bool FFmpegVIdeoPlay::openAvCodec() {
    //5 打开解码器
    code = avcodec_open2(avCodecContext, avCodec, NULL);

    if (code < 0) {
        LOGI_TAG("%s", "解码器无法打开");
        return false;
    }

    int64_t dura = avFormatContext->duration;

    //输出视频信息,宽高不一定有
    srcWidth = avCodecParameters->width;
    srcHeight = avCodecParameters->height;
    LOGI_TAG("视频宽高：%d,%d", srcWidth, srcHeight);

    outWidth = 1280;
    outHeight = 720;


    LOGI_TAG("视频文件名：%s", avFormatContext->filename);
    LOGI_TAG("视频文件格式：%s", avFormatContext->iformat->name);
    LOGI_TAG("视频时长：%lld", dura / 1000000);
    LOGI_TAG("解码器名称：%s", avCodec->name);

    return true;
}


void FFmpegVIdeoPlay::prepareReadFrame(enum AVPixelFormat aVPixelFormat) {
    //准备读取
    avPacket = av_packet_alloc();
    if (avPacket == NULL) {
        LOGI_TAG("%s", "avPacket 不能为空");
        return;
    }

    //AVFrame用于存储解码后的像素数据（YUV）
    //内存分配
    avFrame = av_frame_alloc();

    //用于转码 (缩放) 的参数，转之前的宽高，转之后的宽高，格式等
    sws_ctx = sws_getContext(srcWidth, srcHeight, avCodecContext->pix_fmt,
                             outWidth, outHeight, AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR,
                             NULL, NULL, NULL);
}

void FFmpegVIdeoPlay::getANativeWindow(JNIEnv *jniEnv, jobject surface) {
    //显示窗口初始化
    aNativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    if (aNativeWindow == NULL) {
        LOGE_TAG("%s", "ANativeWindow 为null");
        return;
    }

    //为什么宽为580，高为360，而视频展示宽度比较小呢，为什么这个地方狂傲按实际的设置，则显示越小呢？？？
    //绘制之前配置nativewindow，
    ANativeWindow_setBuffersGeometry(aNativeWindow, outWidth, outHeight, WINDOW_FORMAT_RGBA_8888);
}

void FFmpegVIdeoPlay::playReadFrame() {
    char *rgb = new char[1920 * 1080 * 4];
    //视频缓冲区
    ANativeWindow_Buffer aNativeWindow_buffer;

    while (1) {
        code = av_read_frame(avFormatContext, avPacket);
        if (code != 0) {

            //播放出错
            if (code == AVERROR_EOF) {
                LOGI_TAG("%s", "AVERROR_EOF");
                break;
            }

            LOGI_TAG("%s", "读到结尾处");
            //seek到20s处
            int pos = static_cast<int>(20 *
                                       r2d(avFormatContext->streams[target_stream_index]->time_base));
            av_seek_frame(avFormatContext, target_stream_index, pos,
                          AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }

        //avcodec_send_packet 将avPacket不断的发送到缓存空间；
        //avcodec_receive_frame 不断的从缓存空间取出AVFrame
        //但是每次发送一个avPacket，但是取出来的可能是多个，所以要添加循环；
        //发送到线程中解码
        code = avcodec_send_packet(avCodecContext, avPacket);

        //清理
        av_packet_unref(avPacket);

        if (code != 0) {
            //LOGI_TAG("%s :: %d", "avcodec_send_packet 失败", code);
            continue;
        }
        LOGI_TAG("%s :: %d", "avcodec_send_packet 成功", code);

        //这个不能改成使用code 判断；
        while (1) {
            code = avcodec_receive_frame(avCodecContext, avFrame);
            if (code != 0) {
                break;
            }

            //视频帧
            if (avPacket->stream_index == target_stream_index) {

                uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                data[0] = (uint8_t *) rgb;
                int lines[AV_NUM_DATA_POINTERS] = {0};
                lines[0] = outWidth * 4;

                //转换为rgb格式
                int h = sws_scale(sws_ctx, (const uint8_t *const *) avFrame->data,
                                  avFrame->linesize, 0,
                                  avFrame->height, data, lines);

                LOGI_TAG("sws_scale == %d", h);
                if (h > 0) {
                    //上锁
                    ANativeWindow_lock(aNativeWindow, &aNativeWindow_buffer, NULL);
                    uint8_t *dst = (uint8_t *) aNativeWindow_buffer.bits;
                    memcpy(dst, rgb, static_cast<size_t>(outWidth * outHeight * 4));
                    //解锁
                    ANativeWindow_unlockAndPost(aNativeWindow);
                }
            }
        }

        ANativeWindow_release(aNativeWindow);
    }
}


void FFmpegVIdeoPlay::releaseResource() {
//    if (avFormatContext != NULL) {
//        avformat_free_context(avFormatContext);
//    }
//
//    if (avCodecParameters != NULL) {
//        avcodec_parameters_free(&avCodecParameters);
//    }
//
//    if (avCodecContext != NULL) {
//        avcodec_free_context(&avCodecContext);
//    }
//
//    if (avPacket != NULL) {
//        av_packet_unref(avPacket);
//    }
//
//    if (avFrame != NULL) {
//        av_frame_free(&avFrame);
//    }
}