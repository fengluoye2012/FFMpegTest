//
// Created by wjw on 2019-09-05.
//

#include <cstddef>
#include <android/log.h>
#include "../CPlusLogUtil.h"
#include "SingletonTest.h"

#ifndef FFMPEGTEST_SINGLETONTEST1_H
#define FFMPEGTEST_SINGLETONTEST1_H

#endif //FFMPEGTEST_SINGLETONTEST1_H

class SingletonTest1 {

    //私有变量无法在源文件访问
private:
    SingletonTest1();

public:
    static SingletonTest1 *instance;

    static SingletonTest1 *getInstance();
    static SingletonTest1 *getInstanceLock();

    void printStr();

};








