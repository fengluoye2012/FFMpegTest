//
// Created by wjw on 2019-09-06.
//

#ifndef FFMPEGTEST_OPENSLESPLAYMUSIC_H
#define FFMPEGTEST_OPENSLESPLAYMUSIC_H

#endif //FFMPEGTEST_OPENSLESPLAYMUSIC_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "iostream"
#include "FFmpegMusic.h"

using namespace std;
using std::string;

class OpenSLESPlayMusic {

private:
    SLObjectItf engineObject = NULL;//用SLObjectItf声明引擎接口对象
    SLEngineItf engineEngine = NULL;//声明具体的引擎对象
    SLObjectItf outputMixObject = NULL;//用SLObjectItf创建混音器接口对象
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;////具体的混音器对象实例

    SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;//默认情况
    SLObjectItf audioPlayer = NULL;//用SLObjectItf声明播放器接口对象

    SLPlayItf slPlayItf = NULL;//播放器接口
    SLAndroidSimpleBufferQueueItf slBufferQueueItf = NULL;//缓冲区队列接口

    FFmpegMusic *fFmpegMusic;


    void createEngine();

    void createMixVolume();

    void createPlayer(const char *input);

public:
    void playMusic(const char *input);

    void releaseResource();
};