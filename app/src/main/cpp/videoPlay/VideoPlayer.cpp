
#include <jni.h>
#include <pthread.h>
#include <android/native_window.h>
#include <libavutil/time.h>
#include "VideoPlayer.h"
#include "FFmpegAudioPlayer.h"
#include "FFmpegVideoPlayer.h"

extern "C" {
#include <libavformat/avformat.h>
}

const char *in_put;
int64_t *totalTime;
FFmpegVideoPlayer *fFmpegVideoPlayer;
FFmpegAudioPlayer *fFmpegAudioPlayer;
pthread_t p_tid;
int isPlay;
ANativeWindow *window = NULL;
int64_t duration;
AVFormatContext *avFormatContext;
AVPacket *packet;
int step = 0;
jboolean isSeek = JNI_FALSE;


void call_video_play(AVFrame *frame) {
    if (window == NULL) {
        return;
    }

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0) > -1) {
        return;
    }

    LOGI_TAG("绘制 宽%d,,高%d", frame->width, frame->height);
    LOGI_TAG("绘制宽%d,高%d,行字节%d", window_buffer.width, window_buffer.height, frame->linesize[0]);

    uint8_t *dst = (uint8_t *) (window_buffer.bits);
    int32_t dstStride = window_buffer.stride * 4;

    uint8_t *src = frame->data[0];
    size_t srcStride = (size_t) frame->linesize[0];

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

void *begin(void *args) {

    //找到视频流和音频流
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        AVCodecParameters *avCodecParameters = avFormatContext->streams[i]->codecpar;

        //获取解码器
        AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

        AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
        if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
            LOGI_TAG("%s", "打开失败");
            continue;
        }

        //视频流
        if (avCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            fFmpegVideoPlayer->index = i;
            fFmpegVideoPlayer->setAvCodecContext(avCodecContext);
            fFmpegVideoPlayer->time_base = avFormatContext->streams[i]->time_base;

            if (window != NULL) {
                ANativeWindow_setBuffersGeometry(window, avCodecParameters->width,
                                                 avCodecParameters->height,
                                                 avCodecParameters->format);
            }
        }

        //音频流
        if (avCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            fFmpegAudioPlayer->index = i;
            fFmpegAudioPlayer->setAvCodecContext(avCodecContext);
            fFmpegAudioPlayer->time_base = avFormatContext->streams[i]->time_base;
        }
    }

    //开启播放
    fFmpegVideoPlayer->setFFmpegMusic(fFmpegAudioPlayer);
    fFmpegAudioPlayer->play();
    fFmpegVideoPlayer->play();
    isPlay = 1;

    //解码packet，并压入队列中
    packet = av_packet_alloc();
    int code;
    while (isPlay) {
        code = av_read_frame(avFormatContext, packet);
        if (code == 0) {
            if (fFmpegVideoPlayer != NULL && fFmpegVideoPlayer->isPlay &&
                packet->stream_index == fFmpegVideoPlayer->index) {
                //将视频packet压入队列
                fFmpegVideoPlayer->put(packet);
            } else if (fFmpegAudioPlayer != NULL && fFmpegAudioPlayer->isPlay &&
                       packet->stream_index == fFmpegAudioPlayer->index) {
                fFmpegAudioPlayer->put(packet);
            }
            av_packet_unref(packet);
        } else if (code == AVERROR_EOF) {
            //读取完毕，但不一定是播放完毕
            while (isPlay) {
                if (fFmpegVideoPlayer->queue.empty() && fFmpegAudioPlayer->queue.empty()) {
                    break;
                }
                av_usleep(10000);
            }
        }
    }

    //解码完成后，可能还没有播放完成
    isPlay = 0;
    if (fFmpegAudioPlayer != NULL && fFmpegAudioPlayer->isPlay) {
        fFmpegAudioPlayer->stop();
    }

    if (fFmpegVideoPlayer != NULL && fFmpegVideoPlayer->isPlay) {
        fFmpegVideoPlayer->stop();
    }

    //释放
    av_packet_free(&packet);
    avformat_free_context(avFormatContext);
    //退出线程
    pthread_exit(0);
}

void native_prepare(JNIEnv *env, jobject obj, jstring inputStr) {
    in_put = env->GetStringUTFChars(inputStr, JNI_FALSE);
    init();
    fFmpegVideoPlayer = new FFmpegVideoPlayer();
    fFmpegAudioPlayer = new FFmpegAudioPlayer();

    fFmpegVideoPlayer->setPlayCall(call_video_play);

    //创建线程，开启begin线程
    /*
     * pthread_t *thread：表示创建的线程的id号
     * const pthread_attr_t *attr：表示线程的属性设置
     * void *(*start_routine) (void *)：很明显这是一个函数指针，表示线程创建后回调执行的函数
     * void *arg：这个参数很简单，表示的就是函数指针回调执行函数的参数
     */
    pthread_create(&p_tid, NULL, begin, NULL);

    env->ReleaseStringUTFChars(inputStr, in_put);
}

void init() {
    LOGI_TAG("%s", "开启解码线程");
    //1 注册组件
    av_register_all();
    avformat_network_init();

    //封装格式上下文
    avFormatContext = avformat_alloc_context();

    //2 打开输入视频文件
    if (avformat_open_input(&avFormatContext, in_put, NULL, NULL) < 0) {
        LOGI_TAG("%s", "无法打开视频文件");
        return;
    }

    //3 获取视频信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGI_TAG("%s", "获取视频信息失败");
        return;
    }

    //获取播放总时间
    if (avFormatContext->duration != AV_NOPTS_VALUE) {
        duration = avFormatContext->duration;//毫秒
    }
}

void native_play(JNIEnv *env, jobject obj, jobject surface) {

}

jint native_getTotalTime(JNIEnv *env, jobject obj) {
    return 0;
}

jdouble native_getCurPos(JNIEnv *env, jobject obj) {

    return 0;
}

void native_seekTo(JNIEnv *env, jobject obj, jint seekPos) {

}

void native_stepUp(JNIEnv *env, jobject obj) {

}

void native_stepBack(JNIEnv *env, jobject obj) {

}

void native_stop(JNIEnv *env, jobject obj) {

}

void native_release(JNIEnv *env, jobject obj) {

}





