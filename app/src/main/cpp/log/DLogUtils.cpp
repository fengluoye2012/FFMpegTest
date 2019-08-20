//
// Created by 风落叶 on 2019-08-14.
//

#include <android/log.h>
#include "DLogUtils.h"

void DLogUtils::logInfo(string str) {
    __android_log_print(ANDROID_LOG_INFO, "LogUtils", "%s", str.c_str());
}

void DLogUtils::logWarn(string str) {
    __android_log_print(ANDROID_LOG_WARN, "LogUtils", "%s", str.c_str());

}

void DLogUtils::logError(string str) {
    __android_log_print(ANDROID_LOG_ERROR, "LogUtils", "%s", str.c_str());
}

void DLogUtils::logDebug(string str) {
    __android_log_print(ANDROID_LOG_DEBUG, "LogUtils", "%s", str.c_str());
}


