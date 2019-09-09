//
// Created by 风落叶 on 2019-08-29.
//

#ifndef FFMPEGTEST_CLOGUTIL_H
#define FFMPEGTEST_CLOGUTIL_H

#endif //FFMPEGTEST_CLOGUTIL_H

#define LOGI_TAG(TAG, FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,TAG,FORMAT,__VA_ARGS__);
#define LOGE_TAG(TAG, FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,__VA_ARGS__);
