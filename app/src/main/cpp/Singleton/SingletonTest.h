////
//// Created by wjw on 2019-09-02.
////
//

#include <cstddef>
#include "../CPlusLogUtil.h"

#ifndef FFMPEGTEST_SINGLETONTEST_H
#define FFMPEGTEST_SINGLETONTEST_H

// 一些声明语句



#endif //FFMPEGTEST_SINGLETONTEST_H

//C++ 单例设计模式 https://blog.csdn.net/zhanghuaichao/article/details/79459130
//饿汉式 单例实际模式

//声明全局变量
#pragma once   //防止重复加载
static const char *kTAG = "SingletonTest";

class SingletonTest {

private:
    SingletonTest() {};
public:

    //懒汉式
    static SingletonTest *getInstance() {
        static SingletonTest instance;
        return &instance;
    };

    void printStr() {
        LOGE("%s", "c++  饿汉式 单利模式");
    }
};
