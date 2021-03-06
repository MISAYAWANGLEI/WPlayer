//
// Created by wanglei on 2019/4/30.
//
#ifndef _AUDIO_H
#define _AUDIO_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include "macro.h"
#include <SLES/OpenSLES_Android.h>
extern "C"{
#include <libswresample/swresample.h>
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int id,AVCodecContext *codecContext,AVRational timeBase,
            pthread_mutex_t seekMutex,CppCallJavaUtils *callJavaUtils,
            pthread_mutex_t mutex_pause,pthread_cond_t cond_pause);
    ~AudioChannel();
    void play();
    void decode();
    void _play();
    int getPcm();
    void stop();
    void pause();
    void continuePlay();
    uint8_t *data = 0;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;
private:

    //引擎与引擎接口
    SLObjectItf engineObject = 0;
    SLEngineItf engineInterface = 0;
    //混音器
    SLObjectItf outputMixObject = 0;
    //播放器
    SLObjectItf bqPlayerObject = 0;
    //播放器接口
    SLPlayItf bqPlayerInterface = 0;
    //数据缓冲区
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface =0;
    pthread_t pid_decode;
    pthread_t pid_play;
    //重采样
    SwrContext *swrContext = 0;
};

#endif

