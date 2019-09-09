//
// Created by 风落叶 on 2019-08-29.
//

#ifndef FFMPEGTEST_LOGUTIL_H
#define FFMPEGTEST_LOGUTIL_H

#endif //FFMPEGTEST_LOGUTIL_H

#include "stdio.h"
#include "string.h"

//日志工具头文件
#define LOGI_TAG(...) \
  __android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__)
#define LOGW_TAG(...) \
 __android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__)
#define LOGE_TAG(...) \
  __android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__)
