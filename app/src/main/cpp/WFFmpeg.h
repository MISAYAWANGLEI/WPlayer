#include <pthread.h>
#include "CppCallJavaUtils.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

using namespace std;

extern "C"{
    #include "libavformat/avformat.h"
}

class WFFmpeg {
public:
    char *dataSource = nullptr;
    CppCallJavaUtils *utils = nullptr;
    pthread_t pid_prepare;
    pthread_t pid_play;
    pthread_t pid_stop;
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
    void stop();
    bool isPlaying;
};

