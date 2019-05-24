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
    char *dataSource = 0;
    CppCallJavaUtils *utils = 0;
    pthread_t pid_prepare;
    pthread_t pid_play;
    pthread_t pid_stop;
    AVFormatContext *formatContext = 0;
    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
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

