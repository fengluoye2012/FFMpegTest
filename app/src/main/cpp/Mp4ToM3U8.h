//
// Created by wjw on 2019-09-02.
//

#include <libavformat/avformat.h>
#include <stdbool.h>

#ifndef FFMPEGTEST_MP4TOM3U8_H
#define FFMPEGTEST_MP4TOM3U8_H

#endif //FFMPEGTEST_MP4TOM3U8_H

char TAG[] = "Mp4ToM3U8";
//输入文件 上下文
AVFormatContext *avFormatContext;
int code;

//输出文件  上下文
AVFormatContext *outAVFormatContext;

AVStream *inAvStream;
AVStream *outAvStream;

AVCodec *encoder;

AVCodecContext *inAvCodecContext;
AVCodecContext *outAvCodecContext;

AVBitStreamFilterContext *aacbsfc;
AVPacket *pkt;
int stream_index;
AVFrame *avFrame;
enum AVMediaType type;

//方法指针
int (*dec_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);

int goFrame;

/**
 * 将视频转码为m3u8
 * @param input
 * @param output
 */
void decodeToM3U8(const char *input, const char *output);

/**
 * 根据输入文件路径  打开输入文件
 * @param input
 * @return
 */
bool open_input_file(const char *input);

/**
 * 根据输出文件路径  打开输入文件
 * @param input
 * @return
 */
bool open_output_file(const char *output);

bool encode_write_frame(AVFrame *avFrame, int stream_index, int *got_frame);