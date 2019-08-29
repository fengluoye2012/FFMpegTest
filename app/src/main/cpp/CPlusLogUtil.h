//
// Created by 风落叶 on 2019-08-29.
//

#ifndef FFMPEGTEST_LOGUTIL_H
#define FFMPEGTEST_LOGUTIL_H

#endif //FFMPEGTEST_LOGUTIL_H

#include "stdio.h"
#include "string.h"


// 共同的Log日志
#define LOGI(...) \
  __android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__)
#define LOGW(...) \
 __android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__)
#define LOGE(...) \
  __android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__)
