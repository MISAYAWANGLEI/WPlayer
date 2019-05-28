//
// Created by wanglei55 on 2019/4/30.
//

#include "CppCallJavaUtils.h"
#include "macro.h"

CppCallJavaUtils::CppCallJavaUtils(JavaVM *vm, JNIEnv *env, jobject instance) {
    this->vm = vm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);

    jclass jclazz = env->GetObjectClass(instance);
    onErrorMethodID = env->GetMethodID(jclazz,"onError","(I)V");
    onPrepareMethodID = env->GetMethodID(jclazz,"onPrepare","()V");
    onProgressMethodID = env->GetMethodID(jclazz,"onProgress","(I)V");
}

CppCallJavaUtils::~CppCallJavaUtils() {
    env->DeleteGlobalRef(instance);
}

void CppCallJavaUtils::onError(int threadID, int errorID) {
    if (threadID==THREAD_MAIN){
        env->CallVoidMethod(instance,onErrorMethodID,errorID);
    } else{
        JNIEnv *env;
        if (vm->AttachCurrentThread(&env,0)!=JNI_OK){
            return;
        }
        env->CallVoidMethod(instance,onErrorMethodID,errorID);
        vm->DetachCurrentThread();
    }
}

void CppCallJavaUtils::onPrepare(int threadID) {
    if (threadID==THREAD_MAIN){
        env->CallVoidMethod(instance,onPrepareMethodID);
    } else{
        JNIEnv *env;
        if (vm->AttachCurrentThread(&env,0)!=JNI_OK){
            return;
        }
        env->CallVoidMethod(instance,onPrepareMethodID);
        vm->DetachCurrentThread();
    }
}

void CppCallJavaUtils::onProgress(int threadID, int progress) {
    if (threadID==THREAD_MAIN){
        env->CallVoidMethod(instance,onProgressMethodID,progress);
    } else{
        JNIEnv *env;
        if (vm->AttachCurrentThread(&env,0)!=JNI_OK){
            return;
        }
        env->CallVoidMethod(instance,onProgressMethodID,progress);
        vm->DetachCurrentThread();
    }
}
