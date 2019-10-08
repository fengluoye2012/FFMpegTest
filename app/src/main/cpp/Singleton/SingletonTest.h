////
//// Created by wjw on 2019-09-02.
////
//



#ifndef FFMPEGTEST_SINGLETONTEST_H
#define FFMPEGTEST_SINGLETONTEST_H

// 一些声明语句
//声明全局变量
#pragma once   //防止重复加载
static const char *kTAG = "SingletonTest";

#include <cstddef>
#include "../CPlusLogUtil.h"

#endif //FFMPEGTEST_SINGLETONTEST_H

//C++ 单例设计模式 https://blog.csdn.net/zhanghuaichao/article/details/79459130
//饿汉式 单例实际模式  一开始就加载了，以空间换时间；

/**
 * 饿汉 和 懒汉的区别：看定义的是静态成员对象变量还是静态成员对象指针变量，
 * 因为如果定义了静态成员对象变量，程序在运行之初已经分配了空间，就要调用构造函数了，而你在调用getinstance的时候，
 * 不会再调用构造函数了，因为之前已经调用过了，你就是用的现成的，就是所谓的饿汉模式
 */



class SingletonTest {

private:
    SingletonTest() {};
public:
    //懒汉式
    static SingletonTest *getInstance() {
        static SingletonTest instance;
        return &instance;
    };

    void printStr();
};
