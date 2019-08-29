//
// Created by wjw on 2019-08-28.
//

# include <stdio.h>
# include <stdlib.h>
#include <jni.h>
#include <libavformat/avformat.h>
#include <android/log.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"heiko",FORMAT,__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"heiko",FORMAT,__VA_ARGS__);


JNIEXPORT jstring JNICALL
Java_com_ffmpeg_test_FFmpegTest_ffmpegConfig(JNIEnv *env, jclass type) {
    const char *str = avcodec_configuration();
    return (*env)->NewStringUTF(env, str);
}


JNIEXPORT void Java_com_ffmpeg_test_FFmpegTest_videoDecode(JNIEnv *env, jclass cls, jstring input_,
                                                           jstring output_) {

    const char *input = (*env)->GetStringUTFChars(env, input_, 0);
    const char *output = (*env)->GetStringUTFChars(env, output_, 0);

    LOGI("%s", "开始转码");

    //1 注册所有组件
    av_register_all();

    AVFormatContext *avFormatContext = avformat_alloc_context();

    //2 打开输入视频文件
    //打开媒体文件,成功返回0；否则为负数；
    int code = avformat_open_input(&avFormatContext, input, NULL, NULL);

    if (code < 0) {
        LOGI("%s", "无法打开视频文件");
        return;
    }

    //3 获取视频文件信息
    code = avformat_find_stream_info(avFormatContext, NULL);

    //添加该方法
    //av_dump_format(avFormatContext, 0, input_, 0);

    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_index = -1;
    int i = 0;
    for (; i < avFormatContext->nb_streams; i++) {
        //流的类型  为视频流;//AVMEDIA_TYPE_AUDIO 为音频流；
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            v_stream_index = i;
            break;
        }
    }

    if (code < 0) {
        LOGI("%s", "无法获取视频信息");
        return;
    }

    if (v_stream_index == -1) {
        LOGI("%s", "找不到视频流")
        return;
    }

    LOGI("找到了视频流::%d", v_stream_index);

    //只有知道视频的编码方式，才能够根据编码方式找到解码器
    AVCodecParameters *avCodecParameters = avFormatContext->streams[v_stream_index]->codecpar;

    //4 根据编解码上下文中的编码ID查找对应的解码
    AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    if (avCodec == NULL) {
        LOGI("%s", "找不到解码器");
        return;
    }

    //获取视频流中的编解码上下文 需要使用avcodec_free_context释放
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);

    code = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
    if (code < 0) {
        LOGI("%s", "avcodec_parameters_to_context Fail")
        return;
    }

    //5 打开解码器
    code = avcodec_open2(avCodecContext, avCodec, NULL);

    if (code < 0) {
        LOGI("%s", "解码器无法打开");
        return;
    }


    int64_t dura = avFormatContext->duration;

    //输出视频信息
    int srcWidth = avCodecParameters->width;
    int srcHeight = avCodecParameters->height;
    LOGI("视频文件格式：%d,%d", srcWidth, srcHeight);


    LOGI("视频文件名：%s", avFormatContext->filename);
    LOGI("视频文件格式：%s", avFormatContext->iformat->name);
    LOGI("视频时长：%lld", dura / 1000000);
    LOGI("解码器名称：%s", avCodec->name);

    //准备读取
    //缓冲区，开辟空间 AVPacket用来存储一帧一帧的压缩数据（H264）
//    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
//    av_init_packet(avPacket);

    AVPacket *avPacket = av_packet_alloc();
    if (avPacket == NULL) {
        LOGI("%s", "avPacket 不能为空")
        return;
    }

    //AVFrame用于存储解码后的像素数据（YUV）
    //内存分配
    AVFrame *avFrame = av_frame_alloc();

    //YUV420
    AVFrame *avFrameYUV = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓存区分配内存
    uint8_t *out_buffer = av_malloc(
            (size_t) av_image_get_buffer_size(AV_PIX_FMT_YUV420P, srcWidth, srcHeight, 1));


    //初始化缓存区
    av_image_fill_arrays(avFrameYUV->data, avFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P,
                         srcWidth, srcHeight, 1);


    //用于转码 (缩放) 的参数，转之前的宽高，转之后的宽高，格式等
    struct SwsContext *sws_ctx = sws_getContext(srcWidth, srcWidth, avCodecContext->pix_fmt,
                                                srcWidth, srcWidth, AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                                NULL, NULL, NULL);


    FILE *fp_yuv = fopen(output, "wb+");

    int frame_count = 0;

    //6 一帧一帧的读取压缩数据 成功返回0，否则小于0
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        LOGI("解码第%d帧", frame_count);

        //只压缩视频流数据（根据流的索引位置判断）
        if (avPacket->stream_index == v_stream_index) {
            //7 解码一帧视频压缩数据，得到视频像素数据
            code = avcodec_send_packet(avCodecContext, avPacket);
            av_packet_unref(avPacket);
            if (code < 0) {
                LOGI("%s :: %d", "avcodec_send_packet 失败", code);
            }

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

    fclose(fp_yuv);
    (*env)->ReleaseStringUTFChars(env, input_, input);
    (*env)->ReleaseStringUTFChars(env, output_, output);

    av_frame_free(&avFrame);
    avcodec_free_context(&avCodecContext);
    avformat_free_context(avFormatContext);

    LOGI("%s", "转码结束");
}