//
// Created by wjw on 2019-08-27.
//

#include <libavformat/avformat.h>
#include <android/log.h>
#include "VideoDecodeUtil.h"

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"heiko",FORMAT,__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"heiko",FORMAT,__VA_ARGS__);

void VideoDecodeUtil::videoDecode(const char *inPath, const char *outPath) {

    av_register_all();

    AVFormatContext *avFormatContext = avformat_alloc_context();
    //打开媒体文件,成功返回0；否则为负数；
    int code = avformat_open_input(&avFormatContext, inPath, NULL, NULL);

    if (code < 0) {
        LOGI("%s", "无法打开视频文件");
        return;
    }

    //获取文件信息
    code = avformat_find_stream_info(avFormatContext, NULL);

    if (code < 0) {
        LOGI("%s", "无法获取视频信息");
        return;
    }


    //获取视频流的索引位置
    //遍历所有类型的流（音频流、视频流、字幕流），找到视频流
    int v_stream_index = -1;
    int i = 0;
    for (; i < avFormatContext->nb_streams; i++) {
        //流的类型  为视频流
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            v_stream_index = i;
            break;
        }
    }

    if(v_stream_index == -1){

        return;
    }
}
