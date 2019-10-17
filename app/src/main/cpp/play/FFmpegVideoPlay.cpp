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
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


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

    //添加该方法
    //av_dump_format(avFormatContext, 0, input_, 0);

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

    //4 根据编解码上下文中的编码ID查找对应的解码
    avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    if (avCodec == NULL) {
        LOGI_TAG("%s", "找不到解码器");
        return false;
    }

    //获取视频流中的编解码上下文 需要使用avcodec_free_context释放
    avCodecContext = avcodec_alloc_context3(avCodec);

    code = avcodec_parameters_to_context(avCodecContext, avCodecParameters);

    //指定多线程解码，提高解码速度
    avCodecContext->thread_count = 4;
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

    //输出视频信息
    srcWidth = avCodecParameters->width;
    srcHeight = avCodecParameters->height;
    LOGI_TAG("视频宽高：%d,%d", srcWidth, srcHeight);


    LOGI_TAG("视频文件名：%s", avFormatContext->filename);
    LOGI_TAG("视频文件格式：%s", avFormatContext->iformat->name);
    LOGI_TAG("视频时长：%lld", dura / 1000000);
    LOGI_TAG("解码器名称：%s", avCodec->name);

    return true;
}


void FFmpegVIdeoPlay::prepareReadFrame(enum AVPixelFormat aVPixelFormat) {
//准备读取
    //缓冲区，开辟空间 AVPacket用来存储一帧一帧的压缩数据（H264）
//    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
//    av_init_packet(avPacket);

    avPacket = av_packet_alloc();
    if (avPacket == NULL) {
        LOGI_TAG("%s", "avPacket 不能为空");
        return;
    }

    //AVFrame用于存储解码后的像素数据（YUV）
    //内存分配
    avFrame = av_frame_alloc();

    //YUV420
    avFrameYUV = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓存区分配内存
    out_buffer = static_cast<uint8_t *>(av_malloc(
            (size_t) av_image_get_buffer_size(aVPixelFormat, srcWidth, srcHeight, 1)));

    //初始化缓存区
    av_image_fill_arrays(avFrameYUV->data, avFrameYUV->linesize, out_buffer, aVPixelFormat,
                         srcWidth, srcHeight, 1);

    //用于转码 (缩放) 的参数，转之前的宽高，转之后的宽高，格式等
    sws_ctx = sws_getContext(srcWidth, srcHeight, avCodecContext->pix_fmt,
                             srcWidth, srcHeight, aVPixelFormat, SWS_BICUBIC,
                             NULL, NULL, NULL);

    //也可以考虑使用 sws_getCachedContext()
}

void FFmpegVIdeoPlay::getANativeWindow(JNIEnv *jniEnv, jobject surface) {
    aNativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    if (aNativeWindow == NULL) {
        LOGE_TAG("%s", "ANativeWindow 为null");
        return;
    }
}

void FFmpegVIdeoPlay::playReadFrame() {
    LOGE_TAG("srcWidth == %d，，srcHeight == %d", srcWidth, srcHeight);
    //为什么宽为580，高为360，而视频展示宽度比较小呢，为什么这个地方狂傲按实际的设置，则显示越小呢？？？
    //绘制之前配置nativewindow
    ANativeWindow_setBuffersGeometry(aNativeWindow, srcWidth, srcHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == target_stream_index) {

            //avcodec_send_packet 将avPacket不断的发送到缓存空间；
            //avcodec_receive_frame 不断的从缓存空间取出AVFrame
            //但是每次发送一个avPacket，但是取出来的可能是多个，所以要添加循环；
            // LOGI_TAG("%s", "解码");
            avcodec_send_packet(avCodecContext, avPacket);

            if (code < 0) {
                //LOGI_TAG("%s :: %d", "avcodec_send_packet 失败", code);
            }

            //这个不能改成使用code 判断；
            while (1) {
                code = avcodec_receive_frame(avCodecContext, avFrameYUV);
                if (code < 0) {
                    break;
                }

                //说明有内容
                //上锁
                ANativeWindow_lock(aNativeWindow, &aNativeWindow_buffer, NULL);

                //转换为rgb格式
                int h = sws_scale(sws_ctx, (const uint8_t *const *) avFrame->data,
                                  avFrame->linesize, 0,
                                  avFrame->height, avFrameYUV->data, avFrameYUV->linesize);

                LOGI_TAG("h == %d",h);

                // rgb_frame是有画面数据
                uint8_t *dst = (uint8_t *) aNativeWindow_buffer.bits;
                //拿到一行有多少个字节 RGBA
                int destStride = aNativeWindow_buffer.stride * 4;
                //像素数据的首地址
                uint8_t *src = avFrameYUV->data[0];
                //实际内存一行数量
                size_t srcStride = (size_t) avFrameYUV->linesize[0];
                //int i=0;
                for (int i = 0; i < avCodecContext->height; ++i) {
                    //memcpy(void *dest, const void *src, size_t n)
                    // 将rgb_frame中每一行的数据复制给nativewindow
                    memcpy(dst + i * destStride, src + i * srcStride, srcStride);
                }

                //解锁
                ANativeWindow_unlockAndPost(aNativeWindow);
                usleep(1000 * 16);
            }
        }
        av_packet_unref(avPacket);
    }

    ANativeWindow_release(aNativeWindow);
}

void FFmpegVIdeoPlay::releaseResource() {
    if (avFormatContext != NULL) {
        avformat_free_context(avFormatContext);
    }

    if (avCodecParameters != NULL) {
        avcodec_parameters_free(&avCodecParameters);
    }

    if (avCodecContext != NULL) {
        avcodec_free_context(&avCodecContext);
    }

    if (avPacket != NULL) {
        av_packet_unref(avPacket);
    }

    if (avFrame != NULL) {
        av_frame_free(&avFrame);
    }

    if (avFrameYUV != NULL) {
        av_frame_free(&avFrameYUV);
    }

    if (out_buffer != NULL) {
        av_free(out_buffer);
    }


}