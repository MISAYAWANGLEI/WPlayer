//
// Created by wanglei55 on 2019/4/30.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext):BaseChannel(id,codecContext) {
    frames.setReleaseCallBack(releaseAVFrame);
}

VideoChannel::~VideoChannel() {
    frames.clear();
}

/**
 * 解码线程
 * @param args
 * @return
 */
void * decode_task(void* args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
}

/**
 * 渲染线程
 * @param args
 * @return
 */
void * render_task(void* args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->render();
}

void VideoChannel::play() {
    isPlaying = 1;
    frames.setWork(1);
    packets.setWork(1);
    //解码线程
    pthread_create(&decode_id,0,decode_task,this);
    //播放线程
    pthread_create(&render_id,0,render_task,this);
}

//解码
void VideoChannel::decode() {

    while (isPlaying){

    }

}
//渲染
void VideoChannel::render() {

}

void VideoChannel::setRenderFrameCallBack(renderFrameCallBack callBack) {
    this->frameCallBack = callBack;
}