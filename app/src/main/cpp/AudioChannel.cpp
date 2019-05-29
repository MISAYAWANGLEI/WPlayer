//
// Created by wanglei55 on 2019/4/30.
//
#include "AudioChannel.h"
#include "macro.h"

AudioChannel::AudioChannel(int id,AVCodecContext *codecContext,AVRational timeBase,
        pthread_mutex_t seekMutex,CppCallJavaUtils *callJavaUtils):BaseChannel(id,codecContext,timeBase,seekMutex,callJavaUtils) {
    //声道数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //每个采样16位表示
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //采样率
    out_sample_rate = 44100;

    data = static_cast<uint8_t *>(malloc(out_sample_rate * out_channels * out_samplesize));
    memset(data,0,out_sample_rate * out_channels * out_samplesize);
}

AudioChannel::~AudioChannel() {
    if (data){//释放内存
        free(data);
        data = 0;
    }
}

void * audio_decode(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return 0;
}

void * audio_play(void *args){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->_play();
    return 0;
}

void AudioChannel::decode() {
    AVPacket *packet = 0;
    while (isPlaying){
        int ret = packets.pop(packet);
        if (!isPlaying){
            break;
        }
        if (!ret){
            continue;
        }
        //与seek逻辑清空codecContext解码器中缓存数据同步,否则多线程操作codecContext会有同步问题
        pthread_mutex_lock(&seekMutex);
        ret = avcodec_send_packet(codecContext,packet);
        releaseAVPacket(&packet);//释放数据所占内存
        if(ret!=0){
            break;
        }
        //解码后的数据
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext,frame);
        pthread_mutex_unlock(&seekMutex);
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

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context){
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    int dataSize = audioChannel->getPcm();
    if (dataSize > 0){
        (*bq)->Enqueue(bq,audioChannel->data,dataSize);
    }

}

int AudioChannel::getPcm() {
    int dataSize = 0;
    AVFrame *frame = 0;
    int ret = frames.pop(frame);
    if (!isPlaying){
        if (ret){
            releaseAVFrame(&frame);
        }
        return dataSize;
    }
    if (!ret){
        return dataSize;
    }
    //重采样
    //函数swr_get_delay得到输入sample和输出sample之间的延迟，
    // 并且其返回值的根据传入的第二个参数不同而不同。如果是输入的采样率，
    // 则返回值是输入sample个数；如果输入的是输出采样率，则返回值是输出sample个数。
    int64_t delay = swr_get_delay(swrContext,frame->sample_rate);
    //计算转换后的sample个数
    //转后后的sample个数的计算公式为：src_nb_samples * dst_sample_rate / src_sample_rate
    //delay+frame->nb_samples:这里写成这样是为了能实时处理。想想一下这个矛盾，这些音频数据，
    // 如果处理转换的时间大于产生的时间，那么就会造成生产的新数据堆积。
    // 写成这样，就计算了一个大的buffer size，用于转换。这样就不会产生数据堆积了。
    int64_t dstSamples = av_rescale_rnd(delay+frame->nb_samples,out_sample_rate,
            frame->sample_rate,AV_ROUND_UP);
    //返回每个声道采样的个数:dstSamples表示转换后缓冲区大小
    int samples = swr_convert(swrContext, &data, dstSamples,
            (const uint8_t **)frame->data, frame->nb_samples);
    if (samples < 0){
        return dataSize;
    }
    //算成双声道字节数：每个采样16位表示（2字节）
    dataSize = samples * out_channels * out_samplesize;
    //记录这一帧音频相对时间
    clock = frame->best_effort_timestamp * av_q2d(timeBase);
    if (callJavaUtils){
        callJavaUtils->onProgress(THREAD_CHILD,clock);
    }
    releaseAVFrame(&frame);
    return dataSize;
}

//利用 Android 提供的 AudioRecord 采集音频，利用 AudioTrack 播放音频，
// 利用 MediaCodec 来编解码，这些 API 均是 Android 提供的 Java 层 API，
// 无论是采集、播放还是编解码，这些 API 接口都需要将音频数据从 Java 拷贝到 native 层，
// 或者从 native 层拷贝到 Java，如果希望减少拷贝，开发更加高效的 Android 音频应用，
// 则建议使用 Android NDK 提供的 OpenSL ES API 接口，它支持在 native 层直接处理音频数据。
void AudioChannel::_play() {
    //OpenSL ES播放音频套路
    //1：创建引擎并获取引擎接口
    SLresult result;
    // 1.1 创建引擎 SLObjectItf engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 1.2 初始化引擎  init
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 1.3 获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,
                                           &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //2：设置混音器
    //2.1 创建混音器SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                                 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 2.2 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //3：创建播放器:
    //3.1 配置输入声音信息
    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    //设置播放器播放数据的信息
    //pcm+2(双声道)+44100(采样率)+ 16(采样位)+16(容器的大小)+LEFT|RIGHT(双声道)+小端数据
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};

    //3.2  配置音轨(输出)
    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&outputMix, NULL};
    //需要的接口  操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //3.3 创建播放器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource,
                                          &audioSink, 1,
                                          ids, req);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);

    /**
     * 4、设置播放回调函数
     */
    //获取播放器队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueueInterface);
    //设置回调
    (*bqPlayerBufferQueueInterface)->RegisterCallback(bqPlayerBufferQueueInterface,
                                                      bqPlayerCallback, this);
    /**
     * 5、设置播放状态
     */
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);
    /**
     * 6、手动激活一下这个回调
     */
    bqPlayerCallback(bqPlayerBufferQueueInterface, this);
}

void AudioChannel::play() {
    packets.setWork(1);
    frames.setWork(1);

    swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,out_sample_rate,
            codecContext->channel_layout,codecContext->sample_fmt,codecContext->sample_rate,
            0,0);
    if (!swrContext){
        LOGE("音频 swrContext init failed");
        return;
    }
    swr_init(swrContext);

    isPlaying = 1;
    pthread_create(&pid_decode,0,audio_decode,this);
    pthread_create(&pid_play,0,audio_play,this);
}

void AudioChannel::stop() {
    LOGE("AudioChannel::stop()");
    isPlaying = 0;
    packets.setWork(0);
    frames.setWork(0);
    pthread_join(pid_decode,0);
    pthread_join(pid_play,0);

    //设置停止状态
    if (bqPlayerInterface) {
        (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_STOPPED);
        bqPlayerInterface = 0;
    }

    //释放播放器
    if(bqPlayerObject){
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bqPlayerBufferQueueInterface = 0;
    }

    //释放混音器
    if(outputMixObject){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }

    //释放引擎
    if(engineObject){
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineInterface = 0;
    }

    if (swrContext){
        swr_free(&swrContext);
        swrContext = 0;
    }
}