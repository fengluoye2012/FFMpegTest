////
//// Created by wjw on 2019-09-02.
////
//
//#include <android/log.h>
//#include <libavutil/opt.h>
//#include "Mp4ToM3U8.h"
//#include "CLogUtil.h"
//
//
//void decodeToM3U8(const char *input, const char *output) {
//    av_register_all();
//
//    if (!open_input_file(input)) {
//        return;
//    }
//
//    if (!open_output_file(output)) {
//        return;
//    }
//
//    aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
//
//    pkt = av_packet_alloc();
//    int flag = 1;
//
//    while (av_read_frame(avFormatContext, pkt) >= 0) {
//        stream_index = pkt->stream_index;
//        type = avFormatContext->streams[stream_index]->codec->codec_type;
//
//        avFrame = av_frame_alloc();
//
//        if (avFrame == NULL) {
//            break;
//        }
//
//        av_packet_rescale_ts(pkt, avFormatContext->streams[stream_index]->time_base,
//                             avFormatContext->streams[stream_index]->codec->time_base);
//
//        dec_func = (type == AVMEDIA_TYPE_VIDEO) ? avcodec_decode_video2 : avcodec_decode_audio4;
//
//        code = dec_func(avFormatContext->streams[stream_index]->codec, avFrame, &goFrame, pkt);
//        if (code < 0) {
//            av_frame_free(&avFrame);
//            break;
//        }
//
//        if (goFrame) {
//            avFrame->pts = avFrame->pkt_pts;
//
//            if (type == AVMEDIA_TYPE_VIDEO) {
//                code = encode_write_frame(avFrame, stream_index, NULL);
//            } else {
//                if (flag) {
//                    initSwr(stream_index);
//                    flag = 0;
//                }
//                AVFrame *outAVFrame = av_frame_alloc();
//                if (0 != transSample(avFrame, outAVFrame, stream_index)) {
//                    LOGI_TAG(TAG, "%s", "转换音频失败");
//                    code = -1;
//                }
//                code = encode_write_frame(outAVFrame, stream_index, NULL);
//                av_frame_free(&outAVFrame);
//            }
//            av_frame_free(&avFrame);
//            if (code < 0) {
//                return;
//            }
//        } else {
//            av_frame_free(&avFrame);
//        }
//        av_free_packet(pkt);
//    }
//    av_write_trailer(outAVFormatContext);
//
//    av_bitstream_filter_close(aacbsfc);
//    for (int i = 0; i < avFormatContext->nb_streams; i++) {
//        avcodec_close(avFormatContext->streams[i]->codec);
//        if (outAVFormatContext && outAVFormatContext->nb_streams > i &&
//            outAVFormatContext->streams[i] && outAVFormatContext->streams[i]->codec) {
//            avcodec_close(outAVFormatContext->streams[i]->codec);
//        }
//    }
//
//    avformat_close_input(&avFormatContext);
//    if (outAVFormatContext && !(outAVFormatContext->oformat->flags & AVFMT_NOFILE)) {
//        avio_closep(&outAVFormatContext->pb);
//    }
//    avformat_free_context(outAVFormatContext);
//
//}
//
///**
// * 根据输入路径打开输入流 写入头文件
// * @param input
// * @return
// */
//bool open_input_file(const char *input) {
//    avFormatContext = avformat_alloc_context();
//
//    //2 打开输入视频文件
//    //打开媒体文件,成功返回0；否则为负数；
//    code = avformat_open_input(&avFormatContext, input, NULL, NULL);
//
//    if (code < 0) {
//        LOGI_TAG(TAG, "%s", "无法打开视频文件");
//        return false;
//    }
//
//    //3 获取视频文件信息
//    code = avformat_find_stream_info(avFormatContext, NULL);
//
//    if (code < 0) {
//        LOGI_TAG(TAG, "%s", "无法获取视频信息");
//        return false;
//    }
//
//    //打开输入流
//    for (int i = 0; i < avFormatContext->nb_streams; i++) {
//        AVCodecParameters *avCodecParameters = avFormatContext->streams[i]->codecpar;
//        enum AVMediaType avMediaType = avCodecParameters->codec_type;
//
//        AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
//        AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
//
//        //流的类型  为视频流;//AVMEDIA_TYPE_AUDIO 为音频流；
//        if (avMediaType == AVMEDIA_TYPE_VIDEO || avMediaType == AVMEDIA_TYPE_AUDIO) {
//            code = avcodec_open2(avCodecContext, avCodec, NULL);
//            if (code < 0) {
//                LOGI_TAG(TAG, "%s", "无法打开输入流");
//                return false;
//            }
//        }
//    }
//
//    return true;
//}
//
///**
// * 根据输入路径
// * @param output
// * @return
// */
//bool open_output_file(const char *output) {
//    avformat_alloc_output_context2(&outAVFormatContext, NULL, "hls", output);
//
//    if (outAVFormatContext == NULL) {
//        LOGI_TAG(TAG, "%s", "无法创建 outAVFormatContext ")
//        return false;
//    }
//
//
//    for (int i = 0; i < avFormatContext->nb_streams; i++) {
//        outAvStream = avformat_new_stream(outAVFormatContext, NULL);
//        if (outAvStream == NULL) {
//            return false;
//        }
//
//        inAvStream = avFormatContext->streams[i];
//
//        inAvCodecContext = avcodec_alloc_context3(
//                avcodec_find_decoder(inAvStream->codecpar->codec_id));
//
//        outAvCodecContext = avcodec_alloc_context3(
//                avcodec_find_decoder(outAvStream->codecpar->codec_id));
//
//
//        if (inAvCodecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
//            encoder = avcodec_find_decoder(AV_CODEC_ID_H264);
//            if (encoder == NULL) {
//                LOGI_TAG(TAG, "%s", "encoder 无法获取")
//                return false;
//            }
//
//            outAvCodecContext->height = inAvCodecContext->height;
//            outAvCodecContext->height = inAvCodecContext->width;
//            outAvCodecContext->sample_aspect_ratio = inAvCodecContext->sample_aspect_ratio;
//            outAvCodecContext->pix_fmt = encoder->pix_fmts[0];
//            outAvCodecContext->time_base = inAvCodecContext->time_base;
//            outAvCodecContext->qmin = 10;
//            outAvCodecContext->qmax = 51;
//            outAvCodecContext->max_b_frames = 3;
//            outAvCodecContext->gop_size = 250;
//            outAvCodecContext->bit_rate = 50000;
//            outAvCodecContext->time_base.num = inAvCodecContext->time_base.num;
//            outAvCodecContext->time_base.den = inAvCodecContext->time_base.den;
//
//            code = avcodec_open2(outAvCodecContext, encoder, NULL);
//            if (code < 0) {
//                LOGI_TAG(TAG, "%s", "无法打开 encoder 流");
//                return false;
//            }
//            av_opt_set(outAVFormatContext->priv_data, "preset", "superfast", 0);
//            av_opt_set(outAVFormatContext->priv_data, "true", "zerolatency", 0);
//
//        } else if (inAvCodecContext->codec_type == AVMEDIA_TYPE_UNKNOWN) {
//            LOGI_TAG(TAG, "%s", "AVMEDIA_TYPE_UNKNOWN");
//            return false;
//        } else if (inAvCodecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
//            encoder = avcodec_find_decoder(AV_CODEC_ID_AAC);
//            outAvCodecContext->sample_rate = inAvCodecContext->sample_rate;
//            outAvCodecContext->channel_layout = inAvCodecContext->channel_layout;
//            outAvCodecContext->sample_fmt = encoder->sample_fmts[0];
//
//            AVRational avRational = {1, outAvCodecContext->sample_rate};
//            outAvCodecContext->time_base = avRational;
//
//            code = avcodec_open2(outAvCodecContext, encoder, NULL);
//            if (code < 0) {
//                LOGI_TAG(TAG, "%s", "无法打开音频 encode")
//                return false;
//            }
//        } else {
//            code = avcodec_copy_context(outAvCodecContext, inAvCodecContext);
//            if (code < 0) {
//
//                return false;
//            }
//        }
//
//        if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
//            outAvCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
//        }
//    }
//
//    av_dump_format(outAVFormatContext, 0, output, 1);
//    code = avio_open(&outAVFormatContext->pb, output, AVIO_FLAG_WRITE);
//
//    if (code < 0) {
//        LOGI_TAG(TAG, "%s", "无法打开输入文件")
//        return false;
//    }
//
//    code = avformat_write_header(outAVFormatContext, NULL);
//    if (code < 0) {
//        LOGI_TAG(TAG, "%s", "写入头文件失败")
//        return false;
//    }
//
//    return true;
//}
//
//bool encode_write_frame(AVFrame *avFrame, int stream_index, int *got_frame) {
//    static a_total_duration = 0;
//    static v_tatal_duration = 0;
//
//
//    return 0;
//}
//
//
//int transSample(AVFrame *avFrame, AVFrame *outAVFrame, int stream_index) {
//    return 0;
//}
//
//void initSwr(int index) {
//
//}
//
