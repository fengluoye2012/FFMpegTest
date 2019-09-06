//
// Created by wjw on 2019-09-05.
//
#include <android/log.h>
#include "SingletonTest.h"
#include "../CPlusLogUtil.h"



void SingletonTest::printStr() {
    LOGE("%s", "c++  饿汉式 单利模式");
}
