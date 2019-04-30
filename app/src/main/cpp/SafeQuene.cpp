//
// Created by wanglei55 on 2019/4/30.
//

#include "SafeQuene.h"

SafeQuene::SafeQuene() {
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
}

SafeQuene::~SafeQuene() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void SafeQuene::setWork(int work) {
    pthread_mutex_lock(&mutex);
    this->work = work;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

template <typename T> void SafeQuene::push(const T value) {
    pthread_mutex_lock(&mutex);
    if(work){
        queue.push(value);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
}

template <typename T> int SafeQuene::pop(T &value) {
    int ret = 0;
    pthread_mutex_lock(&mutex);
    while(work && queue.empty()){
        pthread_cond_wait(&cond,&mutex);
    }
    if (!queue.empty()){
        value = queue.front();
        queue.pop();
        ret = 1;
    }
    pthread_mutex_unlock(&mutex);
    return ret;
}

int SafeQuene::empty() {
    return queue.empty();
}

int SafeQuene::size() {
    return queue.size();
}

void SafeQuene::setReleaseCallBack(SafeQuene::releaseCallBack callBack) {
    mReleaseCallBack = callBack;
}

template <typename T> void SafeQuene::clear() {
    pthread_mutex_lock(&mutex);
    int size = queue.size();
    for (int i = 0; i < size; ++i) {
        T value = queue.front();
        mReleaseCallBack(value);
        queue.pop();
    }
    pthread_mutex_unlock(&mutex);
}