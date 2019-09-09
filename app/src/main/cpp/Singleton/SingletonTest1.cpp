//
// Created by wjw on 2019-09-05.
//

#include "SingletonTest1.h"

//类中声明的静态变量需要在类外边再定义一次
SingletonTest1 *SingletonTest1::instance = NULL;

SingletonTest1::SingletonTest1() {

}


void SingletonTest1::printStr() {
    LOGI_TAG("%s", "懒汉式 单利设计模式");
}

//懒汉式单利模式   非线程安全；
SingletonTest1 *SingletonTest1::getInstance() {
    if (instance == NULL) {
        instance = new SingletonTest1();
    }
    return instance;
}

//懒汉 单利模式   线程安全
SingletonTest1 *SingletonTest1::getInstanceLock() {

    if(instance == NULL){

    }

    return nullptr;
}



