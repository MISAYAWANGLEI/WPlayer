//
// Created by wanglei55 on 2019/5/8.
//
#include "SafeQuene.h"

extern "C"{
#include <libavcodec/avcodec.h>
}


class BaseChannel {
public:
    BaseChannel(int id,AVCodecContext *codecContext):id(id),codecContext(codecContext){}
    virtual ~BaseChannel(){
        packets.setReleaseCallBack(BaseChannel::releaseAVPacket);
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
    virtual void play();
    AVCodecContext *codecContext;
    SafeQuene<AVPacket*> packets;
    SafeQuene<AVFrame*> frames;//盛放解码出的视频或音频frame
};

