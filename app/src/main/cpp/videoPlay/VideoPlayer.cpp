#include <jni.h>
#include <pthread.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "VideoPlayer.h"
#include "FFmpegAudioPlayer.h"
#include "FFmpegVideoPlayer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

const char *in_put;
FFmpegVideoPlayer *fFmpegVideoPlayer;
FFmpegAudioPlayer *fFmpegAudioPlayer;
pthread_t p_tid;
int isPlay;
ANativeWindow *window = NULL;
int64_t duration;
AVFormatContext *formatContext;
AVPacket *packet;

/*
 *  视频播放器播放一个互联网上的视频文件，需要经过以下几个步骤：解协议，解封装，解码视音频，视音频同步。
 *  如果播放本地文件则不需要解协议，为以下几个步骤：解封装，解码视音频，视音频同步。
 */

void call_video_play(AVFrame *frame) {
    if (!window) {
        return;
    }

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }

    LOGI_TAG("绘制 宽%d,,高%d", frame->width, frame->height);
    LOGI_TAG("绘制宽%d,高%d,行字节%d", window_buffer.width, window_buffer.height, frame->linesize[0]);

    //rgb_frame是有画面数据
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int32_t dstStride = window_buffer.stride * 4;

    uint8_t *src = frame->data[0];
    size_t srcStride = (size_t) frame->linesize[0];

    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

//开始准备播放
void *begin(void *args) {

    //可以使用 av_find_best_stream()的方式替换下列方式，找到对应的音频流、视频流索引；
    int videoStream = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int audioStream = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    //打开视频解码器
    AVCodecParameters *avCodecParameters = formatContext->streams[videoStream]->codecpar;

    //打开音频解码器


    //找到视频流和音频流，并且打开音频、视频解码器
    for (int i = 0; i < formatContext->nb_streams; ++i) {

        AVCodecParameters *avCodecParameters = formatContext->streams[i]->codecpar;

        //获取解码器
        AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
        //avcodec_find_decoder_by_name() //也可以通过该方法获取解码器；

        AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
        //关联参数
        avcodec_parameters_to_context(avCodecContext, avCodecParameters);
        //设置线程数
        avCodecContext->thread_count = 1;

        //打开解码器
        if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
            LOGI_TAG("%s", "打开失败");
            continue;
        }

        LOGE_TAG("width::%d，height::%d", avCodecContext->width, avCodecContext->height);
        //sample_rate 采样率，音频独有
        LOGE_TAG("采样率：sample_rate::%d，声道数：height::%d", avCodecContext->sample_rate,
                 avCodecContext->channels);

        LOGE_TAG("format：：%d,WINDOW_FORMAT_RGBA_8888 ==%d", avCodecParameters->format,
                 WINDOW_FORMAT_RGBA_8888);

        //视频流
        if (avCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            fFmpegVideoPlayer->index = i;
            fFmpegVideoPlayer->setAvCodecContext(avCodecContext);
            fFmpegVideoPlayer->time_base = formatContext->streams[i]->time_base;

            if (window) {
                ANativeWindow_setBuffersGeometry(window, avCodecParameters->width,
                                                 avCodecParameters->height,
                                                 WINDOW_FORMAT_RGBA_8888);
            }
            //音频流
        } else if (avCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            fFmpegAudioPlayer->index = i;
            fFmpegAudioPlayer->setAvCodecContext(avCodecContext);
            fFmpegAudioPlayer->time_base = formatContext->streams[i]->time_base;
        }
    }

    //开启播放
    fFmpegVideoPlayer->setFFmpegMusic(fFmpegAudioPlayer);
    fFmpegAudioPlayer->play();
    fFmpegVideoPlayer->play();
    isPlay = 1;

    //解码packet，并压入队列中
    //创建并且初始化；AVPacket结构体存储压缩数据；
    packet = av_packet_alloc();
    //跳转到某一特定的帧上面播放
    int code;
    while (isPlay) {
        //无法判断是到文件结尾还是都是错误，因为到文件结尾也是返回0；
        //对于视频，数据包只包含一帧。对于音频，如果每个帧有一个已知的固定大小(例如PCM或ADPCM数据)，则它包含整数帧。如果音频帧有一个可变的大小(如MPEG音频)，那么它包含一个帧。
        code = av_read_frame(formatContext, packet);
        if (code == 0) {
            if (fFmpegVideoPlayer != NULL && fFmpegVideoPlayer->isPlay &&
                packet->stream_index == fFmpegVideoPlayer->index) {
                //将视频packet压入队列
                fFmpegVideoPlayer->put(packet);
            } else if (fFmpegAudioPlayer != NULL && fFmpegAudioPlayer->isPlay &&
                       packet->stream_index == fFmpegAudioPlayer->index) {
                fFmpegAudioPlayer->put(packet);
            }

            //packet使用完之后，释放掉；
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
    avformat_free_context(formatContext);
    //退出线程
    pthread_exit(0);
}

void native_prepare(JNIEnv *env, jobject obj, jstring inputStr) {

    in_put = env->GetStringUTFChars(inputStr, JNI_FALSE);
    if (!init()) {
        return;
    }
    fFmpegVideoPlayer = new FFmpegVideoPlayer();
    fFmpegAudioPlayer = new FFmpegAudioPlayer();

    //函数指针作为参数
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

bool init() {
    LOGI_TAG("%s", "开启解码线程");
    //1 注册组件
    av_register_all();
    //网络注册
    avformat_network_init();

    //封装格式上下文，分配一个AVFormatContext，使用avformat_free_context()释放context,同时释放其内部分配的所有内容；
    formatContext = avformat_alloc_context();

    if (formatContext == NULL) {
        LOGI_TAG("%s", "formatContext为NULL");
        return false;
    }

    LOGI_TAG("输入视频路径::%s", in_put);
    //2 打开输入视频文件，可以将AVFormatContext传入NULL指针，其内部会分配一个AVFormatContext，并为其赋值；如果想要使用自定义IO，可以预申请；
    if (avformat_open_input(&formatContext, in_put, NULL, NULL) < 0) {
        LOGI_TAG("%s", "无法打开视频文件");
        return false;
    }

    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {

    }

    //3 获取视频信息，该方法不能保证可以打开所有编解码器；
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGI_TAG("%s", "获取视频信息失败");
        return false;
    }

    //获取播放总时间
    if (formatContext->duration != AV_NOPTS_VALUE) {
        duration = formatContext->duration;//毫秒
    }
    return true;
}

void native_play(JNIEnv *env, jobject obj, jobject surface) {
    //得到界面
    if (window != NULL) {
        ANativeWindow_release(window);
        window = NULL;
    }

    window = ANativeWindow_fromSurface(env, surface);
    if (fFmpegVideoPlayer != NULL && fFmpegVideoPlayer->codec != NULL) {
        ANativeWindow_setBuffersGeometry(window, fFmpegVideoPlayer->codec->width,
                                         fFmpegVideoPlayer->codec->height, WINDOW_FORMAT_RGBA_8888);
    }
}

//获取视频总时间
jint native_getTotalTime(JNIEnv *env, jobject obj) {
    return static_cast<jint>(duration);
}

//视频的播放时间向音频的播放时间对齐；
jdouble native_getCurPos(JNIEnv *env, jobject obj) {
    return fFmpegAudioPlayer->clock;
}

void native_seekTo(JNIEnv *env, jobject obj, jint seekPos) {
    seekTo(seekPos);
}

void seekTo(int mesc) {
    if (mesc <= 0) {
        mesc = 0;
    }
    //清空vector
    fFmpegAudioPlayer->queue.clear();
    fFmpegAudioPlayer->queue.clear();

    //av_seek_frame()的问题：1）选择视频还是音频来选择seek()，选择视频帧来移动；
    // 2）时间点附近关键帧往前，还是往后找（前后以当前时间点的前后）；一半是往前找，找关键帧 AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME

    LOGI_TAG("mesc == %d,,av_q2d == %d", fFmpegVideoPlayer->time_base.num,
             fFmpegVideoPlayer->time_base.den);

    LOGI_TAG("mesc == %d,,av_q2d == %f,,,timestamp == %f", mesc,
             av_q2d(fFmpegVideoPlayer->time_base), mesc / av_q2d(fFmpegVideoPlayer->time_base));
    //跳到某个关键帧，在时间戳中找关键帧；
    av_seek_frame(formatContext, fFmpegVideoPlayer->index,
                  (int64_t) (mesc / av_q2d(fFmpegVideoPlayer->time_base)),
                  AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

//    av_seek_frame(formatContext, fFmpegAudioPlayer->index,
//                  (int64_t) (mesc / av_q2d(fFmpegAudioPlayer->time_base)), AVSEEK_FLAG_BACKWARD);
}

void native_stepUp(JNIEnv *env, jobject obj) {
    //点击快进按钮
    seekTo(5);
}

void native_stepBack(JNIEnv *env, jobject obj) {

}

//暂停
void native_stop(JNIEnv *env, jobject obj) {
    fFmpegAudioPlayer->pause();
    fFmpegVideoPlayer->pause();
}

//释放资源
void native_release(JNIEnv *env, jobject obj) {
    if (isPlay) {
        isPlay = 0;
        pthread_join(p_tid, 0);
    }

    if (fFmpegVideoPlayer != NULL) {
        if (fFmpegVideoPlayer->isPlay) {
            fFmpegVideoPlayer->stop();
        }
        delete (fFmpegVideoPlayer);
        fFmpegVideoPlayer = NULL;
    }

    if (fFmpegAudioPlayer != NULL) {
        if (fFmpegAudioPlayer->isPlay) {
            fFmpegAudioPlayer->stop();
        }
        delete (fFmpegAudioPlayer);
        fFmpegAudioPlayer = NULL;
    }

}





