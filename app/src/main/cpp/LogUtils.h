//
// Created by 风落叶 on 2019-08-14.
//

#ifndef FFMPEGTEST_LOGUTILS_H

#define FFMPEGTEST_LOGUTILS_H

#include "string"

using namespace std;
using std::string;


class LogUtils {
public:
    static void logInfo(string str);

    static void logWarn(string str);
};


#endif //FFMPEGTEST_LOGUTILS_H
