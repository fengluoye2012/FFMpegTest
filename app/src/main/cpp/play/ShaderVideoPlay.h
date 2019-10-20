//
// Created by wjw on 2019-10-20.
//

#ifndef FFMPEGTEST_SHADERVIDEOPLAY_H
#define FFMPEGTEST_SHADERVIDEOPLAY_H

class ShaderVideoPlay {
public:
    void shader_play_video(JNIEnv *env, jstring inPath,jobject surface);
};

#endif //FFMPEGTEST_SHADERVIDEOPLAY_H
