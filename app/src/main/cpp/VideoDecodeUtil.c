//
// Created by wjw on 2019-08-29.
//

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <android/log.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <unistd.h>
#include "VideoDecodeUtil.h"
#include "CLogUtil.h"
#include <android/native_window_jni.h>


void videoDecode(const char *input, const char *output) {

    LOGI(TAG, "%s", "开始转码");

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

    prepareReadFrame(AV_PIX_FMT_YUV420P);

    decodeScale(output);

    readFrame();

    releaseResource();
    LOGI(TAG, "%s", "转码结束");
}

void videoPlay(JNIEnv *jniEnv, const char *input, jobject surface) {
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

void playReadFrame() {
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == v_stream_index) {

            LOGI(TAG, "%s", "解码");
            avcodec_send_packet(avCodecContext, avPacket);

            if (code < 0) {
                LOGI(TAG, "%s :: %d", "avcodec_send_packet 失败", code);
            }

            //这个不能改成使用code 判断；
            while (1) {
                code = avcodec_receive_frame(avCodecContext, avFrameYUV);
                if (code < 0) {
                    break;
                }

                //说明有内容
                //绘制之前配置nativewindow
                ANativeWindow_setBuffersGeometry(aNativeWindow, srcWidth, srcHeight,
                                                 WINDOW_FORMAT_RGBA_8888);

                //上锁
                ANativeWindow_lock(aNativeWindow, &aNativeWindow_buffer, NULL);

                //转换为rgb格式
                sws_scale(sws_ctx, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0,
                          avFrame->height, avFrameYUV->data, avFrameYUV->linesize);

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

void getANativeWindow(JNIEnv *jniEnv, jobject surface) {
    aNativeWindow = ANativeWindow_fromSurface(jniEnv, surface);
    if (aNativeWindow == NULL) {
        LOGE(TAG, "%s", "ANativeWindow 为null");
        return;
    }
}

void ffmpegRegister() {
    av_register_all();
}

bool getStreamInfo(const char *input) {

    avFormatContext = avformat_alloc_context();

    //2 打开输入视频文件
    //打开媒体文件,成功返回0；否则为负数；
    code = avformat_open_input(&avFormatContext, input, NULL, NULL);

    if (code < 0) {
        LOGI(TAG, "%s", "无法打开视频文件");
        return false;
    }

    //3 获取视频文件信息
    code = avformat_find_stream_info(avFormatContext, NULL);

    if (code < 0) {
        LOGI(TAG, "%s", "无法获取视频信息");
        return false;
    }

    //添加该方法
    //av_dump_format(avFormatContext, 0, input_, 0);

    return true;
}


bool getVideoIndex() {

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    v_stream_index = -1;
    int i = 0;
    for (; i < avFormatContext->nb_streams; i++) {
        //流的类型  为视频流;//AVMEDIA_TYPE_AUDIO 为音频流；
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            v_stream_index = i;
            break;
        }
    }

    if (v_stream_index == -1) {
        LOGI(TAG, "%s", "找不到视频流")
        return false;
    }

    LOGI(TAG, "找到了视频流::%d", v_stream_index);

    return true;
}


bool getAVCodec() {
    //只有知道视频的编码方式，才能够根据编码方式找到解码器
    avCodecParameters = avFormatContext->streams[v_stream_index]->codecpar;

    //4 根据编解码上下文中的编码ID查找对应的解码
    avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    if (avCodec == NULL) {
        LOGI(TAG, "%s", "找不到解码器");
        return false;
    }

    //获取视频流中的编解码上下文 需要使用avcodec_free_context释放
    avCodecContext = avcodec_alloc_context3(avCodec);

    code = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    if (code < 0) {
        LOGI(TAG, "%s", "avcodec_parameters_to_context Fail")
        return false;
    }
    return true;
}

bool openAvCodec() {
    //5 打开解码器
    code = avcodec_open2(avCodecContext, avCodec, NULL);

    if (code < 0) {
        LOGI(TAG, "%s", "解码器无法打开");
        return false;
    }


    int64_t dura = avFormatContext->duration;

    //输出视频信息
    srcWidth = avCodecParameters->width;
    srcHeight = avCodecParameters->height;
    LOGI(TAG, "视频文件格式：%d,%d", srcWidth, srcHeight);


    LOGI(TAG, "视频文件名：%s", avFormatContext->filename);
    LOGI(TAG, "视频文件格式：%s", avFormatContext->iformat->name);
    LOGI(TAG, "视频时长：%lld", dura / 1000000);
    LOGI(TAG, "解码器名称：%s", avCodec->name);

    return true;
}


void prepareReadFrame(enum AVPixelFormat aVPixelFormat) {
//准备读取
    //缓冲区，开辟空间 AVPacket用来存储一帧一帧的压缩数据（H264）
//    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
//    av_init_packet(avPacket);

    avPacket = av_packet_alloc();
    if (avPacket == NULL) {
        LOGI(TAG, "%s", "avPacket 不能为空")
        return;
    }

    //AVFrame用于存储解码后的像素数据（YUV）
    //内存分配
    avFrame = av_frame_alloc();

    //YUV420
    avFrameYUV = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓存区分配内存
    out_buffer = av_malloc(
            (size_t) av_image_get_buffer_size(aVPixelFormat, srcWidth, srcHeight, 1));


    //初始化缓存区
    av_image_fill_arrays(avFrameYUV->data, avFrameYUV->linesize, out_buffer, aVPixelFormat,
                         srcWidth, srcHeight, 1);

    //用于转码 (缩放) 的参数，转之前的宽高，转之后的宽高，格式等
    sws_ctx = sws_getContext(srcWidth, srcWidth, avCodecContext->pix_fmt,
                             srcWidth, srcWidth, aVPixelFormat, SWS_BICUBIC,
                             NULL, NULL, NULL);
}

void decodeScale(const char *output) {
    fp_yuv = fopen(output, "wb+");
}

void readFrame() {
    int frame_count = 0;
    //6 一帧一帧的读取压缩数据 成功返回0，否则小于0
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        LOGI(TAG, "解码第%d帧", frame_count);

        //只压缩视频流数据（根据流的索引位置判断）
        if (avPacket->stream_index == v_stream_index) {
            //7 解码一帧视频压缩数据，得到视频像素数据
            code = avcodec_send_packet(avCodecContext, avPacket);
            av_packet_unref(avPacket);
            if (code < 0) {
                LOGI(TAG, "%s :: %d", "avcodec_send_packet 失败", code);
            }

            //这个不能改成使用code 判断；
            while (1) {
                code = avcodec_receive_frame(avCodecContext, avFrameYUV);
                if (code < 0) {
                    break;
                }

                //AVFrame转为像素格式YUV420，宽高
                //参数2、6:输入、输出数据
                //参数3、7:输入、输出画面一行的数据大小 AVFrame转换是一行一行转换的
                //参数4:输入数据第一列要转码的位置 从0开始
                //参数5:输入画面的高度
                sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avCodecContext->height,
                          avFrameYUV->data, avFrameYUV->linesize);

                //输出到YUV文件
                //AVFrame像素帧写入文件
                //data解码后的图像像素数据 (音频采样数据)
                //Y 亮度 U 色度 (压缩了) 人对亮度更加敏感
                //U V 个数是Y的1/4
                size_t y_size = (size_t) (srcWidth * srcHeight);
                fwrite(avFrameYUV->data[0], 1, y_size, fp_yuv); //Y
                fwrite(avFrameYUV->data[1], 1, y_size / 4, fp_yuv); //U
                fwrite(avFrameYUV->data[2], 1, y_size / 4, fp_yuv); //V

                frame_count++;
            }
        }
        //释放资源
        av_packet_unref(avPacket);
    }
}

void releaseResource() {
    if(fp_yuv != NULL){
        fclose(fp_yuv);
    }

    av_frame_free(&avFrame);
    av_frame_free(&avFrameYUV);
    avcodec_free_context(&avCodecContext);
    avformat_free_context(avFormatContext);
}












