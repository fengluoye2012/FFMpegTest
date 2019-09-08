//
// Created by wjw on 2019-09-06.
//

#include <android/log.h>
#include "OpenSLESPlayMusic.h"
#include "FFmpegMusic.h"
#include "../CPlusLogUtil.h"

extern "C" {
#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}

size_t bufferSize = 0;
void *buffer;

//将pcm数据添加到缓冲区中
void getQueueCallBack(SLAndroidSimpleBufferQueueItf slBufferQueueItf, void *context) {
    bufferSize = 0;

    FFmpegMusic::getPcm(&buffer, &bufferSize);
    if (buffer != NULL && bufferSize != 0) {
        //将得到的数据加入到队列中
        (*slBufferQueueItf)->Enqueue(slBufferQueueItf, buffer, bufferSize);
    }
}

void OpenSLESPlayMusic::playMusic(const char *input) {
    createEngine();
    createMixVolume();
    createPlayer(input);
}

//创建引擎
void OpenSLESPlayMusic::createEngine() {
    //创建引擎
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    //实现engineObject接口对象
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    //通过引擎调用接口初始化SLEngineItf
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
}

//创建混音器
void OpenSLESPlayMusic::createMixVolume() {
    //用引擎对象创建混音器接口对象
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    //实现混音器接口对象
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    SLresult sLresult = (*outputMixObject)->GetInterface(outputMixObject,
                                                         SL_IID_ENVIRONMENTALREVERB,
                                                         &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == sLresult) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }
}

//创建
void OpenSLESPlayMusic::createPlayer(const char *input) {
    //初始化FFmpeg
    int rate;
    int channels;

    fFmpegMusic = new FFmpegMusic();
    fFmpegMusic->createFFmpeg(input, &rate, &channels);

    LOGI("rate::%d,,channels::%d", rate, channels);

    /*
     * typedef struct SLDataLocator_AndroidBufferQueue_ {
    SLuint32    locatorType;//缓冲区队列类型
    SLuint32    numBuffers;//buffer位数
    } */
    SLDataLocator_BufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    /*
    typedef struct SLDataFormat_PCM_ {
        SLuint32 		formatType;  pcm
        SLuint32 		numChannels;  通道数
        SLuint32 		samplesPerSec;  采样率
        SLuint32 		bitsPerSample;  采样位数
        SLuint32 		containerSize;  包含位数
        SLuint32 		channelMask;     立体声
        SLuint32		endianness;    end标志位
    } SLDataFormat_PCM;
     */
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM, static_cast<SLuint32>(channels), static_cast<SLuint32>(rate * 1000),
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN
    };

    /*
     * typedef struct SLDataSource_ {
	        void *pLocator;//缓冲区队列
	        void *pFormat;//数据样式,配置信息
        } SLDataSource;
     */
    SLDataSource dataSource = {&android_queue, &pcm};

    SLDataLocator_OutputMix slDataLocator_outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink slDataSink = {&slDataLocator_outputMix, NULL};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE};
    LOGI("%s", "执行到此处");

    (*engineEngine)->CreateAudioPlayer(engineEngine, &audioPlayer, &dataSource, &slDataSink, 3, ids,
                                       req);
    (*audioPlayer)->Realize(audioPlayer, SL_BOOLEAN_FALSE);
    LOGI("%s", "执行到此处2");

    //初始化播放器
    (*audioPlayer)->GetInterface(audioPlayer, SL_IID_PLAY, &slPlayItf);
    //注册缓冲区，通过缓冲区里面的数据进行播放
    (*audioPlayer)->GetInterface(audioPlayer, SL_IID_BUFFERQUEUE, &slBufferQueueItf);
    //设置回调接口
    (*slBufferQueueItf)->RegisterCallback(slBufferQueueItf, getQueueCallBack, NULL);

    //播放
    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
    //开始播放
    getQueueCallBack(slBufferQueueItf, NULL);
}

void OpenSLESPlayMusic::releaseResource() {
    if (audioPlayer != NULL) {
        (*audioPlayer)->Destroy(audioPlayer);
        audioPlayer = NULL;
        slBufferQueueItf = NULL;
        slPlayItf = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (fFmpegMusic != NULL) {
        fFmpegMusic->releaseFFmpeg();
    }
}




