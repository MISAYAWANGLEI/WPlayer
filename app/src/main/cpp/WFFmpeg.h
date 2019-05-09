#include <pthread.h>
#include "CppCallJavaUtils.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

extern "C"{
    #include "libavformat/avformat.h"
}

class WFFmpeg {
private:
    char *dataSource = nullptr;
    CppCallJavaUtils *utils = nullptr;
    pthread_t pid;
    pthread_t pid_play;
    AVFormatContext *formatContext = nullptr;
    AudioChannel *audioChannel = nullptr;
    VideoChannel *videoChannel = nullptr;
    renderFrameCallBack frameCallBack;
public:
    WFFmpeg(CppCallJavaUtils *utils, const char *dataSource);
    ~WFFmpeg();
    void prepare();
    void _prepare();
    void setRenderFrameCallback(renderFrameCallBack callback);
    void start();
    void _start();

    bool isPlaying;
};

