//
// Created by wanglei55 on 2019/4/30.
//
#include "BaseChannel.h"
#include "AudioChannel.h"
extern "C" {
#include <libswscale/swscale.h>
};

typedef void (*renderFrameCallBack)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id,AVCodecContext *codecContext,AVRational timeBase,int fps,pthread_mutex_t seekMutex,CppCallJavaUtils *callJavaUtils);
    ~VideoChannel();

    void play();//播放

    void setRenderFrameCallBack(renderFrameCallBack callBack);
    void decode();//解码
    void render();//播放
    void stop();
    void setAudioChannel(AudioChannel *audioChannel);
private:
    pthread_t decode_id;
    pthread_t render_id;
    renderFrameCallBack frameCallBack;
    SwsContext *swsContext = 0;
    AudioChannel *audioChannel = 0;
    int fps;
};

