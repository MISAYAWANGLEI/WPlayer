#include <jni.h>
#include <string>
#include "WFFmpeg.h"


extern "C"{
    #include "libavformat/avformat.h"
}

WFFmpeg *ffmpeg = nullptr;
JavaVM *javaVm = nullptr;
int JNI_OnLoad(JavaVM *vm, void *r) {
    javaVm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1prepare(JNIEnv *env, jobject instance,
                                                 jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    auto *cppCallJavaUtils = new CppCallJavaUtils(javaVm, env, instance);
    ffmpeg = new WFFmpeg(cppCallJavaUtils, dataSource);
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1start(JNIEnv *env, jobject instance) {

    ffmpeg->start();
}