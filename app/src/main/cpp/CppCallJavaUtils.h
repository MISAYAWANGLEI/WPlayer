#ifndef CPP_UTILS_H
#define CPP_UTILS_H

#include <jni.h>

class CppCallJavaUtils {
private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
    jmethodID onErrorMethodID;
    jmethodID onPrepareMethodID;
    jmethodID onProgressMethodID;
public:
    CppCallJavaUtils(JavaVM *vm,JNIEnv *env,jobject instance);
    ~CppCallJavaUtils();
    void onError(int threadID, int errorID);
    void onPrepare(int threadID);
    void onProgress(int threadID,int progress);
};

#endif
