#include <jni.h>
#include <string>
#include "WFFmpeg.h"
#include <android/native_window_jni.h>
#include "macro.h"

extern "C"{
    #include "libavformat/avformat.h"
}

ANativeWindow *nativeWindow = 0;
WFFmpeg *ffmpeg = nullptr;
JavaVM *javaVm = nullptr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

//展示画面
void frrameCallBack(uint8_t *data, int lineSize, int width, int height){
    pthread_mutex_lock(&mutex);
    if (!nativeWindow){
        pthread_mutex_unlock(&mutex);
        return;
    }
    //设置窗口属性
    int ret = ANativeWindow_setBuffersGeometry(nativeWindow,width,height,WINDOW_FORMAT_RGBA_8888);
    if (ret!=0){
        LOGE("ANativeWindow_setBuffersGeometry failed");
        return;
    }
    ANativeWindow_Buffer nativeWindow_buffer;
    ret = ANativeWindow_lock(nativeWindow,&nativeWindow_buffer,0);
    if (ret != 0){
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //将数据一行行拷贝到目标窗体buffer中
    uint8_t *dst_data = static_cast<uint8_t *>(nativeWindow_buffer.bits);
    //目标窗体一行数据*4(RGBA)
    //nativeWindow_buffer.stride表示每一行的像素数，每个像素由RGBA 4个字节组成
    int dst_linesize = nativeWindow_buffer.stride *4;
    for (int i = 0; i < nativeWindow_buffer.height; ++i) {
        memcpy(dst_data+i*dst_linesize,data+i*lineSize,dst_linesize);
    }
    ANativeWindow_unlockAndPost(nativeWindow);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1prepare(JNIEnv *env, jobject instance,
                                                 jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    auto *cppCallJavaUtils = new CppCallJavaUtils(javaVm, env, instance);
    ffmpeg = new WFFmpeg(cppCallJavaUtils, dataSource);
    ffmpeg->setRenderFrameCallback(frrameCallBack);
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1start(JNIEnv *env, jobject instance) {

    ffmpeg->start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1setSurface(JNIEnv *env, jobject instance,
                                                    jobject surface) {
    pthread_mutex_lock(&mutex);
    if (nativeWindow){
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }
    nativeWindow = ANativeWindow_fromSurface(env,surface);
    pthread_mutex_unlock(&mutex);
}