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
#include <libavutil/timestamp.h>


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

/**
 * mp4转成flv格式文件步骤如下:
 *
 * 1.打开输入文件，创建输入文件和输出文件的上下文环境
 * 2.遍历输入文件的每一路流，每个输入流对应创建一个输出流，将输入流中的编解码参数直接拷贝到输出流中。
 * 3.文件的写入。
 * 先写入新的多媒体文件的头。
 * 然后遍历输入文件的每一帧，对每一帧进行时间基的转换，转换好后写入新的多媒体文件。
 * 最后再多媒体文件中写入文件尾。
 *
 * @param jniEnv
 * @param input
 * @param output
 */
void mp4Toflv(JNIEnv *jniEnv, const char *input, const char *output) {

    //1.1 注册，并且获取StreamInfo；
    ffmpegRegister();

    if (!getStreamInfo(input)) {
        return;
    }

    //1.2 通过avformat_alloc_output_context2() 获取 输出文件的 AVFormatContext；
    code = avformat_alloc_output_context2(&outAvFormatContext, NULL, NULL, output);

    if (code) {
        LOGI(TAG, "%s", "avformat_alloc_output_context2 Fail");
        return;
    }

    //2 遍历输入文件的每一路流，每个输入流对应创建一个输出流，将输入流中的编解码参数直接拷贝到输出流中。
    int stream_mapping_size = avFormatContext->nb_streams;
    //定义一个数组，数组申请动态分配内存；
    int *stream_mapping = NULL;

    //数组申请动态分配内存
    stream_mapping = av_mallocz_array((size_t) stream_mapping_size, sizeof(stream_mapping));

    if (!stream_mapping) {
        LOGI(TAG, "%s", "av_mallocz_array fail");
        return;
    }

    AVOutputFormat *outAvOutputFormat = outAvFormatContext->oformat;
    int stream_index = 0;

    // 遍历avformat_new_stream()复制流，avcodec_parameters_copy() 复制参数；
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVStream *out_stream;
        AVStream *in_stream = avFormatContext->streams[i];

        AVCodecParameters *in_codecpar = in_stream->codecpar;
        enum AVMediaType in_type = in_codecpar->codec_type;
        if (in_type != AVMEDIA_TYPE_AUDIO && in_type != AVMEDIA_TYPE_VIDEO &&
            in_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;
        out_stream = avformat_new_stream(outAvFormatContext, NULL);
        if (!out_stream) {
            return;
        }

        code = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (code < 0) {
            LOGI(TAG, "%s", "avcodec_parameters_copy Fail");
            return;
        }

        out_stream->codecpar->codec_tag = 0;
    }


    av_dump_format(outAvFormatContext, 0, output, 1);
    if (!(outAvFormatContext->flags & AVFMT_NOFILE)) {
        code = avio_open(&outAvFormatContext->pb, output, AVIO_FLAG_WRITE);
        if (code < 0) {
            LOGI(TAG, "%s", "avio_open Fail");
            return;
        }
    }

    //写入头文件
    code = avformat_write_header(outAvFormatContext, NULL);

    if (code < 0) {
        LOGI(TAG, "%s", "avformat_write_header Fail");
        return;
    }

    AVPacket *pkt = av_packet_alloc();
    AVStream *in_stream, *out_stream;
    LOGE(TAG, "%s", "开始读取Frame");

    //读取每一帧
    while (av_read_frame(avFormatContext, pkt) >= 0) {
        in_stream = avFormatContext->streams[pkt->stream_index];
        if (pkt->stream_index >= stream_mapping_size || stream_mapping[pkt->stream_index] < 0) {
            continue;
        }

        pkt->stream_index = stream_mapping[pkt->stream_index];
        out_stream = outAvFormatContext->streams[pkt->stream_index];

        log_packet(avFormatContext, pkt, "in");

        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base,
                                    AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base,
                                    AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

        pkt->pos = -1;

        log_packet(outAvFormatContext, pkt, "out");

        code = av_interleaved_write_frame(outAvFormatContext, pkt);

        if (code < 0) {
            LOGI(TAG, "%s", "av_interleaved_write_frame Fail");
            break;
        }

        av_packet_unref(pkt);
    }
    LOGE(TAG, "%s", "读取完成");

    av_write_trailer(outAvFormatContext);
    //释放资源
    avformat_close_input(&avFormatContext);
    if (outAvFormatContext && !(outAvOutputFormat->flags & AVFMT_NOFILE)) {
        avio_closep(&outAvFormatContext->pb);
    }

    avformat_free_context(outAvFormatContext);
    av_free(stream_mapping);
}


void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {

    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s, dts:%s, dts_time:%s duration:%s duration_time:%s, stream_index:%d\n",
           tag, av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base), av_ts2str(pkt->dts),
           av_ts2timestr(pkt->dts, time_base), av_ts2str(pkt->duration),
           av_ts2timestr(pkt->duration, time_base), pkt->stream_index);

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
    if (fp_yuv != NULL) {
        fclose(fp_yuv);
    }

    av_frame_free(&avFrame);
    av_frame_free(&avFrameYUV);
    avcodec_free_context(&avCodecContext);
    avformat_free_context(avFormatContext);
}












