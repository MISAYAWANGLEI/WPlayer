#ifndef B_H
#define B_H

#include "SafeQueue.h"
#include "macro.h"

extern "C"{
#include <libavcodec/avcodec.h>
}

class BaseChannel {
public:
    BaseChannel(int id,AVCodecContext *codecContext,AVRational timeBase):id(id),
                        codecContext(codecContext),timeBase(timeBase){
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
            LOGE("releaseAVPacket");
            av_packet_free(packet);
            *packet = 0;
        }
    }

    static void releaseAVFrame(AVFrame **frame){
        if (frame){
            LOGE("releaseAVFrame");
            av_frame_free(frame);
            *frame = 0;
        }
    }

    int id;
    bool isPlaying;
    virtual void play() = 0;
    virtual void stop() = 0;
    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    AVRational timeBase;
    double clock;//音频或者视频相对时间：相对录制时的时间，用于音视频同步
};

#endif