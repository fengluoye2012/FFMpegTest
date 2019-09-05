////
//// Created by wjw on 2019-09-02.
////
//

#include <cstddef>
#include "../CPlusLogUtil.h"

#ifndef FFMPEGTEST_SINGLETONTEST_H
#define FFMPEGTEST_SINGLETONTEST_H

#endif //FFMPEGTEST_SINGLETONTEST_H

//C++ 单例设计模式 https://blog.csdn.net/zhanghuaichao/article/details/79459130

static const char *kTAG = "SingletonTest";

class SingletonTest {

private:

    SingletonTest() {}

public:

    //懒汉式
    static SingletonTest *getInstance() {
        static SingletonTest instance;
        return &instance;
    }

    void printStr() {
        LOGE("%s", "c++  懒汉式 单利模式");
    }
};