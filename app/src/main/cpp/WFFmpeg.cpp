//
// Created by wanglei55 on 2019/4/30.
//
#include "WFFmpeg.h"
#include "macro.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <string>
extern "C"{
#include <libavutil/time.h>
}

WFFmpeg::WFFmpeg(CppCallJavaUtils *utils, const char *dataSource) {
    this->utils = utils;
    this->dataSource = new char[strlen(dataSource)+1];
    strcpy(this->dataSource,dataSource);
    duration = 0;
    pthread_mutex_init(&seekMutex,0);
}

WFFmpeg::~WFFmpeg() {
    DELETE(dataSource);
    DELETE(utils);
    pthread_mutex_destroy(&seekMutex);
}

void *start_prepare(void* args){
    WFFmpeg *wfFmpeg = static_cast<WFFmpeg *>(args);
    wfFmpeg->_prepare();
    return 0;
}

void WFFmpeg::prepare() {
    pthread_create(&pid_prepare,0,start_prepare,this);
}

void WFFmpeg::_prepare() {
    av_register_all();
    //初始化网络
    avformat_network_init();
    //打开媒体地址，相关信息存入formatContext
    AVDictionary *options = 0;
    //设置超时时间 微妙 超时时间5秒
    av_dict_set(&options, "timeout", "50000000", 0);
    int ret = avformat_open_input(&formatContext,dataSource,0,&options);
    av_dict_free(&options);
    if (ret!=0){
        LOGE("打开媒体地址失败:%s",av_err2str(ret));
        if (utils){
            utils->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    //查找媒体中音视频流信息
    ret = avformat_find_stream_info(formatContext,0);
    if(ret<0){
        LOGE("查找媒体中音视频流信息失败:%s",av_err2str(ret));
        if (utils){
            utils->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }
    duration = (formatContext->duration)/1000000;
    LOGE("duration = %d",duration);
    //AVFormatContext主要存储视音频封装格式中包含的信息；
    LOGE("formatContext->nb_streams:%d",formatContext->nb_streams);
    //duration是以微秒为单位
    // 转换成hh:mm:ss形式
/**    int time, hh, mm, ss;
    time  = (formatContext->duration)/1000000;
    hh  = time / 3600;
    mm  = (time % 3600) / 60;
    ss  = (time % 60);
    LOGE("视频时长->%d : %d: %d",hh,mm,ss);
//获取原数据
    AVDictionaryEntry *m = NULL;
    m=av_dict_get(formatContext->metadata,"author",m,0);
    LOGE("作者：%s",m->value);
    m=av_dict_get(formatContext->metadata,"copyright",m,0);
    LOGE("版权：%s",m->value);
    m=av_dict_get(formatContext->metadata,"description",m,0);
    LOGE("描述：%s",m->value);
//使用循环读出
    AVDictionaryEntry *m2 = NULL;
//(需要读取的数据，字段名称，前一条字段（循环时使用），参数)
    while(av_dict_get(formatContext->metadata,"",m2,AV_DICT_IGNORE_SUFFIX)){
        char * mKey = m2->key;
        char * mValue = m2->value;
        LOGE("metadata key:%s",mKey);
        LOGE("metadata value:%s",mValue);
    }
**/
    // AVInputFormat存储输入视音频使用的封装格式。
    // 每种视音频封装格式都对应一个AVInputFormat 结构。
    //输出信息
    AVInputFormat* inputFormat = formatContext->iformat;
    LOGE("inputFormat->name:%s",inputFormat->name);
    LOGE("inputFormat->long_name:%s",inputFormat->long_name);
    //AVIOContext，URLProtocol，URLContext主要存储视音频使用的协议的类型以及状态。
    //URLProtocol存储输入视音频使用的封装格式。每种协议都对应一个URLProtocol结构。
    AVIOContext *avioContext = formatContext->pb;
//    unsigned char *buffer：缓存开始位置
//    int buffer_size：缓存大小（默认32768）
//    unsigned char *buf_ptr：当前指针读取到的位置
//    unsigned char *buf_end：缓存结束的位置
//    void *opaque：URLContext结构体
//    在解码的情况下，buffer用于存储ffmpeg读入的数据。例如打开一个视频文件的时候，
//    先把数据从硬盘读入buffer，然后在送给解码器用于解码。
//    其中opaque指向了URLContext。注意，这个结构体并不在FFMPEG提供的头文件中，
//    而是在FFMPEG的源代码中。URLContext结构体中还有一个结构体URLProtocol。
//    在这个结构体中，除了一些回调函数接口之外，有一个变量const char *name，
//    该变量存储了协议的名称。每一种输入协议都对应这样一个结构体。

    //遍历其中流：可能是音频，也可能是视频
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //每个AVStream存储一个视频/音频流的相关数据；
        // 每个AVStream对应一个AVCodecContext，存储该视频/音频流使用解码方式的相关数据；
        // 每个AVCodecContext中对应一个AVCodec，包含该视频/音频对应的解码器。
        // 每种解码器都对应一个AVCodec结构。
        //AVStream是存储每一个视频/音频流信息的结构体
        AVStream *avStream = formatContext->streams[i];
//        int index：标识该视频/音频流
//        AVCodecContext *codec：指向该视频/音频流的AVCodecContext（它们是一一对应的关系）
//        AVRational time_base：时基。通过该值可以把PTS，DTS转化为真正的时间。
//        FFMPEG其他结构体中也有这个字段，但是根据我的经验，只有AVStream中的time_base是可用的。
//        PTS*time_base=真正的时间
//        int64_t duration：该视频/音频流长度
//        AVDictionary *metadata：元数据信息
//        AVRational avg_frame_rate：帧率（注：对视频来说，这个挺重要的）
//        AVPacket attached_pic：附带的图片。比如说一些MP3，AAC音频文件附带的专辑封面。
        LOGE("avStream->index : %d",avStream->index);
        LOGE("avStream->duration : %lld",avStream->duration);
        AVRational avg_frame_rate = avStream->avg_frame_rate;
        //帧率：每秒显示的图像数
        int fps = av_q2d(avg_frame_rate);
        LOGE("fps : %d",fps);
        //包含解码流的各种信息
        AVCodecParameters *parameters = avStream->codecpar;
        //查找解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (codec==NULL){
            LOGE("查找解码器失败:%s",av_err2str(ret));
            if (utils) {
                utils->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        LOGE("codec->name:%s",codec->name);
//        const char *name：编解码器的名字，比较短
//        const char *long_name：编解码器的名字，全称，比较长
//        enum AVMediaType type：指明了类型，是视频，音频，还是字幕
//        enum AVCodecID id：ID，不重复
//        const AVRational *supported_framerates：支持的帧率（仅视频）
//        const enum AVPixelFormat *pix_fmts：支持的像素格式（仅视频）
//        const int *supported_samplerates：支持的采样率（仅音频）
//        const enum AVSampleFormat *sample_fmts：支持的采样格式（仅音频）
//        const uint64_t *channel_layouts：支持的声道数（仅音频）
//        int priv_data_size：私有数据的大小

        //创建解码器上下文
        codecContext = avcodec_alloc_context3(codec);
//        enum AVMediaType codec_type：编解码器的类型（视频，音频...）
//        struct AVCodec  *codec：采用的解码器AVCodec（H.264,MPEG2...）
//        int bit_rate：平均比特率
//        uint8_t *extradata; int extradata_size：针对特定编码器包含的附加信息（例如对于H.264解码器来说，存储SPS，PPS等）
//        AVRational time_base：根据该参数，可以把PTS转化为实际的时间（单位为秒s）
//        int width, height：如果是视频的话，代表宽和高
//        int refs：运动估计参考帧的个数（H.264的话会有多帧，MPEG2这类的一般就没有了）
//        int sample_rate：采样率（音频）
//        int channels：声道数（音频）
//        enum AVSampleFormat sample_fmt：采样格式
//        int profile：型（H.264里面就有，其他编码标准应该也有）
//        int level：级（和profile差不太多）
        if(codecContext == NULL){
            LOGE("创建解码器上下文失败:%s",av_err2str(ret));
            if (utils) {
                utils->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }
        //拷贝参数从parameters到codecContext
        ret = avcodec_parameters_to_context(codecContext,parameters);
        if(ret<0){
            LOGE("拷贝参数从parameters到codecContext失败:%s",av_err2str(ret));
            if (utils) {
                utils->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        //打开解码器
        ret = avcodec_open2(codecContext,codec,0);
        if (ret != 0){
            LOGE("打开解码器失败:%s",av_err2str(ret));
            if (utils) {
                utils->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }

        AVRational timeBase = avStream->time_base;
        if(parameters->codec_type == AVMEDIA_TYPE_AUDIO){//音频
            audioChannel = new AudioChannel(i,codecContext,timeBase,seekMutex,utils);
        } else if (parameters->codec_type == AVMEDIA_TYPE_VIDEO){//视频
            videoChannel = new VideoChannel(i,codecContext,timeBase,fps,seekMutex,utils);
            videoChannel->setRenderFrameCallBack(frameCallBack);
        }
    }

    if (!audioChannel && !videoChannel){
        LOGE("没有音视频");
        if (utils) {
            utils->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        }
        return;
    }
    if (utils) {
        //到这里prepare成功，回调java层
        utils->onPrepare(THREAD_CHILD);
    }
}

void * play(void* args){
    WFFmpeg *ffmpeg = static_cast<WFFmpeg *>(args);
    ffmpeg->_start();
    return 0;
}

void WFFmpeg::start() {
    isPlaying = 1;
    if(audioChannel){
        audioChannel->play();
    }

    if (videoChannel){
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }
    //开启线程读取未加压的数据
    pthread_create(&pid_play,0,play,this);
}

//AVFrame结构体一般用于存储原始数据（即非压缩数据，例如对视频来说是YUV，RGB，对音频来说是PCM），
//此外还包含了一些相关的信息。比如说，解码的时候存储了宏块类型表，QP表，运动矢量表等数据。
//编码的时候也存储了相关的数据。因此在使用FFMPEG进行码流分析的时候，AVFrame是一个很重要的结构体。
/**uint8_t *data[AV_NUM_DATA_POINTERS]：解码后原始数据（对视频来说是YUV，RGB，对音频来说是PCM）
 对于packed格式的数据（例如RGB24），会存到data[0]里面。
对于planar格式的数据（例如YUV420P），则会分开成data[0]，data[1]，data[2]...（YUV420P中data[0]存Y，data[1]存U，data[2]存V）
int linesize[AV_NUM_DATA_POINTERS]：data中“一行”数据的大小。注意：未必等于图像的宽，一般大于图像的宽。
int width, height：视频帧宽和高（1920x1080,1280x720...）
int nb_samples：音频的一个AVFrame中可能包含多个音频帧，在此标记包含了几个
int format：解码后原始数据类型（YUV420，YUV422，RGB24...）
int key_frame：是否是关键帧enum AVPictureType pict_type：帧类型（I,B,P...）
AVRational sample_aspect_ratio：宽高比（16:9，4:3...）
int64_t pts：显示时间戳
int coded_picture_number：编码帧序号
int display_picture_number：显示帧序号
int8_t *qscale_table：QP表
uint8_t *mbskip_table：跳过宏块表
int16_t (*motion_val[2])[2]：运动矢量表
uint32_t *mb_type：宏块类型表
short *dct_coeff：DCT系数，这个没有提取过
int8_t *ref_index[2]：运动估计参考帧列表（貌似H.264这种比较新的标准才会涉及到多参考帧）
int interlaced_frame：是否是隔行扫描
uint8_t motion_subsample_log2：一个宏块中的运动矢量采样个数，取log的**/


void WFFmpeg::_start() {
    int ret;
    while (isPlaying){
        //防止读取文件一下子读完了，导致oom
        //读一部分拿去播
        if (audioChannel && audioChannel->packets.size() > 100) {
            //5ms
            av_usleep(1000 *5);
            continue;
        }
        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(1000 * 5);
            continue;
        }
        //AVPacket是存储压缩编码数据相关信息的结构体
//        uint8_t *data：压缩编码的数据。
//        例如对于H.264来说。1个AVPacket的data通常对应一个NAL。
//        注意：在这里只是对应，而不是一模一样。
//        他们之间有微小的差别：https://blog.csdn.net/leixiaohua1020/article/details/11800877
//        因此在使用FFMPEG进行视音频处理的时候，常常可以将得到的AVPacket的data数据直接写成文件，
//        从而得到视音频的码流文件。
//        int   size：data的大小
//        int64_t pts：显示时间戳
//        int64_t dts：解码时间戳
//        int   stream_index：标识该AVPacket所属的视频/音频流。
        pthread_mutex_lock(&seekMutex);
        AVPacket* packet = av_packet_alloc();
        //0 if OK, < 0 on error or end of file
        ret = av_read_frame(formatContext,packet);
        pthread_mutex_unlock(&seekMutex);
        if (ret == 0){
            if (audioChannel && packet->stream_index == audioChannel->id){
                //音频
                audioChannel ->packets.push(packet);
            } else if(videoChannel && packet->stream_index == videoChannel->id){
                //视频
                videoChannel ->packets.push(packet);//将数据塞到视频队列中
            }
        } else if(ret == AVERROR_EOF){//读取完成
            if (packet){
                av_packet_free(&packet);
                packet = 0;
            }
            if (audioChannel->packets.empty() && audioChannel->frames.empty()
                && videoChannel->packets.empty() && videoChannel->frames.empty()) {
                break;
            }
        } else{
            if (packet){
                av_packet_free(&packet);
                packet = 0;
            }
            LOGE("av_read_frame......");
            break;
        }
    }
    isPlaying = 0;
    if (audioChannel){
        audioChannel->stop();
    }
    if (videoChannel){
        videoChannel->stop();
    }
    if(codecContext){
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
    }
}

void WFFmpeg::setRenderFrameCallback(renderFrameCallBack callback) {
    this->frameCallBack = callback;
}

void *syncStop(void *args){
    LOGE("syncStop");
    WFFmpeg *ffmpeg = static_cast<WFFmpeg *>(args);
    //等待prepare与play逻辑执行完在释放以下资源
    pthread_join(ffmpeg->pid_prepare,0);
    pthread_join(ffmpeg->pid_play,0);

    DELETE(ffmpeg->audioChannel);
    DELETE(ffmpeg->videoChannel);

    if (ffmpeg->formatContext){
        //先关闭读取流
        avformat_close_input(&ffmpeg->formatContext);
        avformat_free_context(ffmpeg->formatContext);
        ffmpeg->formatContext = 0;
    }
    DELETE(ffmpeg);
    return 0;
}

int WFFmpeg::getDuration() {
    return duration;
}

void WFFmpeg::seek(int progress) {
    //音视频都没有seek毛线啊
    if (!audioChannel && !videoChannel){
        return;
    }

    if(!formatContext){
        return;
    }
    pthread_mutex_lock(&seekMutex);

    int64_t seekToTime = progress* AV_TIME_BASE;//转为微妙
    /**
     * seek到请求的时间 之前最近的关键帧,只有从关键帧才能开始解码出完整图片
     * stream_index:可以控制快进音频还是视频，-1表示音视频均快进
     */
    av_seek_frame(formatContext,-1,seekToTime,AVSEEK_FLAG_BACKWARD);

    //清空解码器中缓存的数据
    if (codecContext){
        avcodec_flush_buffers(codecContext);
    }

    if (audioChannel){
        audioChannel->stopWork();
        audioChannel->clear();
        audioChannel->startWork();
    }

    if (videoChannel){
        videoChannel->stopWork();
        videoChannel->clear();
        videoChannel->startWork();
    }

    pthread_mutex_unlock(&seekMutex);
}

void WFFmpeg::stop() {
    isPlaying = 0;
    utils = 0;
    pthread_create(&pid_stop,0,syncStop,this);
}












