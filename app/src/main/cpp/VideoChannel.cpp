//
// Created by wanglei55 on 2019/4/30.
//
extern "C"{
#include <libavutil/imgutils.h>
}
#include "VideoChannel.h"
#include "macro.h"

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
    return 0;
}

/**
 * 渲染线程
 * @param args
 * @return
 */
void * render_task(void* args){
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->render();
    return 0;
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
    AVPacket *packet = 0;
    while (isPlaying){
        int ret = packets.pop(packet);//阻塞的队列
        if (!isPlaying){//队列阻塞期间，可能暂停应用
            break;
        }
        if (!ret){//取出失败，继续取数据
            continue;
        }
        //将packet发送到解码器
        ret = avcodec_send_packet(codecContext,packet);
        if(ret!=0){
            break;
        }
        //解码后的数据，代表一个画面
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext,frame);
        //需要更多的数据才能够进行解码
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if(ret != 0){
            break;
        }
        //解码后的数据放入队列frames中：播放线程会从frames中取出进行播放
        frames.push(frame);
    }
    releaseAVPacket(&packet);
}
//渲染:播放
void VideoChannel::render() {
    //将解码出的frame变为RGBA格式
    swsContext = sws_getContext(codecContext->width,codecContext->height,codecContext->pix_fmt,
                                codecContext->width,codecContext->height,AV_PIX_FMT_RGBA,
                                SWS_BILINEAR,NULL,NULL,NULL);
    if (!swsContext){
        LOGE("swsContext init failed");
        return;
    }
    AVFrame *frame = 0;
    uint8_t *dst_data[4];
    int dst_linesize[4];
    int ret;
    if ((ret = av_image_alloc(dst_data, dst_linesize,
                              codecContext->width,codecContext->height,AV_PIX_FMT_RGBA, 1)) < 0) {
        LOGE("av_image_alloc failed");
        return;
    }

    while (isPlaying){
        ret = frames.pop(frame);
        if (!isPlaying){
            break;
        }
        if (!ret){//取出失败，继续取数据
            continue;
        }
        //linesize:每一行存放的数据的字节数
        sws_scale(swsContext,frame->data,frame->linesize,
                0,codecContext->height,
                dst_data,dst_linesize);
        frameCallBack(dst_data[0],dst_linesize[0],codecContext->width,codecContext->height);
        releaseAVFrame(&frame);
    }
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
    releaseAVFrame(&frame);
}

void VideoChannel::setRenderFrameCallBack(renderFrameCallBack callBack) {
    this->frameCallBack = callBack;
}