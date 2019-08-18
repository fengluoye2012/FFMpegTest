//
// Created by 风落叶 on 2019-08-14.
//

#include <android/log.h>
#include "LogUtils.h"

void LogUtils::logInfo(string str) {
    __android_log_print(ANDROID_LOG_INFO, "LogUtils", "%s", str.c_str());
}

void LogUtils::logWarn(string str) {
    __android_log_print(ANDROID_LOG_WARN, "LogUtils", "%s", str.c_str());

}

void LogUtils::logError(string str) {
    __android_log_print(ANDROID_LOG_ERROR, "LogUtils", "%s", str.c_str());
}

void LogUtils::logDebug(string str) {
    __android_log_print(ANDROID_LOG_DEBUG, "LogUtils", "%s", str.c_str());
}


