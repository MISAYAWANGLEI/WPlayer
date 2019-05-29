#ifndef B_H
#define B_H

#include "SafeQueue.h"
#include "macro.h"
#include "CppCallJavaUtils.h"

extern "C"{
#include <libavcodec/avcodec.h>
}

class BaseChannel {
public:
    BaseChannel(int id,AVCodecContext *codecContext,AVRational timeBase,pthread_mutex_t seekMutex
                        ,CppCallJavaUtils *callJavaUtils,
                pthread_mutex_t mutex_pause,pthread_cond_t cond_pause):id(id),
                        codecContext(codecContext),timeBase(timeBase),seekMutex(seekMutex)
                        ,callJavaUtils(callJavaUtils),mutex_pause(mutex_pause),cond_pause(cond_pause){
        packets.setReleaseCallBack(releaseAVPacket);
        frames.setReleaseCallBack(releaseAVFrame);
    }

    virtual ~BaseChannel(){
        LOGE("~BaseChannel()");
        packets.clear();
        frames.clear();
        if(codecContext){
            LOGE("~BaseChannel() release codecContext");
            avcodec_close(codecContext);
            avcodec_free_context(&codecContext);
            codecContext = 0;
        }
        LOGE("释放channel:%d %d", packets.size(), frames.size());
    }

    static void releaseAVPacket(AVPacket **packet){
        if (packet){
            av_packet_free(packet);
            *packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame **frame){
        if (frame){
            av_frame_free(frame);
            *frame = 0;
        }
    }

    void clear(){
        packets.clear();
        frames.clear();
    }

    void startWork(){
        packets.setWork(1);
        frames.setWork(1);
    }

    void stopWork(){
        packets.setWork(0);
        frames.setWork(0);
    }

    virtual void pause() = 0;

    virtual void continuePlay() = 0;

    int id;
    bool isPlaying;
    bool isPause = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    AVRational timeBase;
    pthread_mutex_t seekMutex;
    CppCallJavaUtils *callJavaUtils;
    double clock;//音频或者视频相对时间：相对录制时的时间，用于音视频同步
    pthread_mutex_t mutex_pause;
    pthread_cond_t cond_pause;
};

#endif