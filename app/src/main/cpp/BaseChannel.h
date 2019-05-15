#ifndef B_H
#define B_H

#include "SafeQueue.h"

extern "C"{
#include <libavcodec/avcodec.h>
}

class BaseChannel {
public:
    BaseChannel(int id,AVCodecContext *codecContext):id(id),codecContext(codecContext){
        packets.setReleaseCallBack(releaseAVPacket);
    }

    virtual ~BaseChannel(){
        packets.clear();
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

    int id;
    bool isPlaying;
    virtual void play() = 0;
    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
};

#endif