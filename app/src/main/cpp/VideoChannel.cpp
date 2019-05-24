//
// Created by wanglei55 on 2019/4/30.
//
extern "C"{
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}
#include "VideoChannel.h"
#include "macro.h"


void syncAVFrame(queue<AVFrame *> &q){
    if (!q.empty()){
        AVFrame *frame = q.front();
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int id, AVCodecContext *codecContext,
        AVRational timeBase,int fps):BaseChannel(id,codecContext,timeBase) {
    this->fps = fps;
    frames.setSyncHandle(syncAVFrame);
}

VideoChannel::~VideoChannel() {

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

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
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
        releaseAVPacket(&packet);
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
    //每个画面显示的时间，也就是图片之间显示间隔，单位秒
    double frame_delay = 1.0/fps;
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
        //记录这一帧视频画面播放相对时间
        clock = frame->best_effort_timestamp * av_q2d(timeBase);
        /**
         * 计算额外需要延迟播放的时间
         * When decoding, this signals how much the picture must be delayed.
         * extra_delay = repeat_pict / (2*fps)
         * int repeat_pict;
        */
        double extra_delay = frame->repeat_pict / (2*fps);
        //真实需要的时间间隔
        double delay = frame_delay + extra_delay;
        if (!audioChannel){//没有音频
            av_usleep(delay * 1000000);
        } else{//有音频
            if (clock == 0){
                av_usleep(delay * 1000000);
            } else{
                //比较音频与视频相对时间差：慢就追快就歇一会
                double diff = clock - audioChannel->clock;//视频减音频
                if(diff > 0){//视频快
                    LOGE("视频快了：%lf",diff);
                    if (diff >1){
                        av_usleep((delay *2 ) * 1000000);//差的比较大，慢慢赶
                    } else{
                        av_usleep((delay+diff) * 1000000);//差不多，多睡一会
                    }
                } else{//音频快
                    LOGE("音频快了：%lf",diff);
                    if(fabs(diff) >= 0.05){//差距比较大，考虑视频丢包
                        releaseAVFrame(&frame);
                        frames.sync();
                        continue;
                    } else{//差距没那么大，视频不用丢包，播放不延时就可以了

                    }
                }
            }
        }
        frameCallBack(dst_data[0],dst_linesize[0],codecContext->width,codecContext->height);
        releaseAVFrame(&frame);
    }
    av_freep(&dst_data[0]);
    isPlaying = 0;
    releaseAVFrame(&frame);
    sws_freeContext(swsContext);
    swsContext = 0;
}

void VideoChannel::setRenderFrameCallBack(renderFrameCallBack callBack) {
    this->frameCallBack = callBack;
}

void VideoChannel::stop() {
    LOGE("VideoChannel::stop()");
    isPlaying = 0;
    frames.setWork(0);
    packets.setWork(0);
    pthread_join(decode_id, 0);
    pthread_join(render_id, 0);
}