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

public:

    static SingletonTest1 *instance;

    static SingletonTest1 *getInstance();

    void printStr();

    SingletonTest1();
};








