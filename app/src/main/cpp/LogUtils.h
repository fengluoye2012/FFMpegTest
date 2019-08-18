//
// Created by 风落叶 on 2019-08-14.
//

#ifndef FFMPEGTEST_LOGUTILS_H

#define FFMPEGTEST_LOGUTILS_H

#include "string"

using namespace std;
using std::string;

//NDK 打印log
class LogUtils {
public:


    static void logInfo(string str);

    static void logWarn(string str);

    static void logError(string str);

    static void logDebug(string str);

};


#endif //FFMPEGTEST_LOGUTILS_H
