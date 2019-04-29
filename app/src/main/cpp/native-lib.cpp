#include <jni.h>
#include <string>

extern "C"{
#include "libavformat/avformat.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wanglei_wplayer_WPlayer_native_1prepare(JNIEnv *env, jobject instance,
                                                 jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);

    // TODO

    env->ReleaseStringUTFChars(dataSource_, dataSource);
}