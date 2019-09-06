//
// Created by wjw on 2019-09-05.
//

#include "SingletonTest1.h"

//类中声明的静态变量需要在类外边再定义一次
SingletonTest1 *SingletonTest1::instance = NULL;

SingletonTest1::SingletonTest1() {

}


SingletonTest1 *SingletonTest1::getInstance() {
    if (instance == NULL) {
        instance = new SingletonTest1();
    }
    return instance;
}

void SingletonTest1::printStr() {
    LOGI("%s", "懒汉式 单利设计模式");
}