//
// Created by wanglei55 on 2019/4/30.
//

#include "WFFmpeg.h"
#include "macro.h"
#include <pthread.h>

WFFmpeg::WFFmpeg(CppCallJavaUtils *utils, const char *dataSource) {
    this->utils = utils;
    this->dataSource = new char[strlen(dataSource)+1];
    strcpy(this->dataSource,dataSource);
}

WFFmpeg::~WFFmpeg() {
    DELETE(dataSource);
    DELETE(utils);
}

void *start_prepare(void* args){
    WFFmpeg *wfFmpeg = static_cast<WFFmpeg *>(args);
    wfFmpeg->_prepare();
    return 0;
}

void WFFmpeg::prepare() {
    pthread_create(&pid,0,start_prepare,this);
}

void WFFmpeg::_prepare() {
    //初始化网络
    avformat_network_init();
    //打开媒体地址，相关信息存入formatContext
    int ret = avformat_open_input(&formatContext,dataSource,0,0);
    if (ret!=0){
        LOGE("打开媒体地址失败:%s",av_err2str(ret));
        utils->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }
    //查找媒体中音视频流信息
    ret = avformat_find_stream_info(formatContext,0);
    if(ret<0){
        LOGE("查找媒体中音视频流信息失败:%s",av_err2str(ret));
        utils->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }
    //遍历其中流：可能是音频，也可能是视频
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *avStream = formatContext->streams[i];
        //包含解码流的各种信息
        AVCodecParameters *parameters = avStream->codecpar;
        //查找解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (codec==NULL){
            LOGE("查找解码器失败:%s",av_err2str(ret));
            utils->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        //创建解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if(codecContext == NULL){
            LOGE("创建解码器上下文失败:%s",av_err2str(ret));
            utils->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //拷贝参数从parameters到codecContext
        ret = avcodec_parameters_to_context(codecContext,parameters);
        if(ret<0){
            LOGE("拷贝参数从parameters到codecContext失败:%s",av_err2str(ret));
            utils->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        //打开解码器
        ret = avcodec_open2(codecContext,codec,0);
        if (ret != 0){
            LOGE("打开解码器失败:%s",av_err2str(ret));
            utils->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        if(parameters->codec_type == AVMEDIA_TYPE_AUDIO){//音频
            audioChannel = new AudioChannel;
        } else if (parameters->codec_type == AVMEDIA_TYPE_VIDEO){//视频
            videoChannel = new VideoChannel;
        }
    }

    if (!audioChannel && !videoChannel){
        LOGE("没有音视频");
        utils->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        return;
    }
    //到这里prepare成功，回调java层
    utils->onPrepare(THREAD_CHILD);
}



















