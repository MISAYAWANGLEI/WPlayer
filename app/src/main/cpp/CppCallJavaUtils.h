
#include <jni.h>

class CppCallJavaUtils {
private:
    JavaVM *vm;
    JNIEnv *env;
    jobject instance;
    jmethodID onErrorMethodID;
    jmethodID onPrepareMethodID;
public:
    CppCallJavaUtils(JavaVM *vm,JNIEnv *env,jobject instance);
    ~CppCallJavaUtils();
    void onError(int threadID, int errorID);
    void onPrepare(int threadID);
};

